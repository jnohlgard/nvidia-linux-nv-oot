// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2011-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
/*
 * User-space interface to nvmap
 */

#include <nvidia/conftest.h>

#include <linux/backing-dev.h>
#include <linux/bitmap.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/oom.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/nvmap.h>
#include <linux/module.h>
#include <linux/resource.h>
#include <linux/security.h>
#include <linux/stat.h>
#include <linux/kthread.h>
#include <linux/highmem.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/of.h>
#include <linux/iommu.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
#include <soc/tegra/chip-id.h>
#else
#include <soc/tegra/fuse.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/clock.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#include <linux/sched/mm.h>
#endif

#include <linux/backing-dev.h>
#include <asm/cputype.h>

#define CREATE_TRACE_POINTS
#include <trace/events/nvmap.h>

#include "nvmap_priv.h"
#include "nvmap_heap.h"
#include "nvmap_ioctl.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#include <linux/pagewalk.h>
#endif

#define NVMAP_CARVEOUT_KILLER_RETRY_TIME 100 /* msecs */

struct nvmap_device *nvmap_dev;
EXPORT_SYMBOL(nvmap_dev);
ulong nvmap_init_time;

static struct device_dma_parameters nvmap_dma_parameters = {
	.max_segment_size = UINT_MAX,
};

static int nvmap_open(struct inode *inode, struct file *filp);
static int nvmap_release(struct inode *inode, struct file *filp);
static long nvmap_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvmap_map(struct file *filp, struct vm_area_struct *vma);
#if !defined(CONFIG_MMU)
static unsigned nvmap_mmap_capabilities(struct file *filp);
#endif

static const struct file_operations nvmap_user_fops = {
	.owner		= THIS_MODULE,
	.open		= nvmap_open,
	.release	= nvmap_release,
	.unlocked_ioctl	= nvmap_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = nvmap_ioctl,
#endif
	.mmap		= nvmap_map,
#if !defined(CONFIG_MMU)
	.mmap_capabilities = nvmap_mmap_capabilities,
#endif
};

static const struct file_operations debug_handles_by_pid_fops;

struct nvmap_pid_data {
	struct rb_node node;
	pid_t pid;
	struct kref refcount;
	struct dentry *handles_file;
};

static void nvmap_pid_release_locked(struct kref *kref)
{
	struct nvmap_pid_data *p = container_of(kref, struct nvmap_pid_data,
			refcount);
	debugfs_remove(p->handles_file);
	rb_erase(&p->node, &nvmap_dev->pids);
	kfree(p);
}

static void nvmap_pid_get_locked(struct nvmap_device *dev, pid_t pid)
{
	struct rb_root *root = &dev->pids;
	struct rb_node **new = &(root->rb_node), *parent = NULL;
	struct nvmap_pid_data *p;
	char name[16];

	while (*new) {
		p = container_of(*new, struct nvmap_pid_data, node);
		parent = *new;

		if (p->pid > pid) {
			new = &((*new)->rb_left);
		} else if (p->pid < pid) {
			new = &((*new)->rb_right);
		} else {
			kref_get(&p->refcount);
			return;
		}
	}

	if (snprintf(name, sizeof(name), "%d", pid) < 0)
		return;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return;

	p->pid = pid;
	kref_init(&p->refcount);
	p->handles_file = debugfs_create_file(name, S_IRUGO,
			dev->handles_by_pid, p,
			&debug_handles_by_pid_fops);

	if (IS_ERR_OR_NULL(p->handles_file)) {
		kfree(p);
	} else {
		rb_link_node(&p->node, parent, new);
		rb_insert_color(&p->node, root);
	}
}

static struct nvmap_pid_data *nvmap_pid_find_locked(struct nvmap_device *dev,
		pid_t pid)
{
	struct rb_node *node = dev->pids.rb_node;

	while (node) {
		struct nvmap_pid_data *p = container_of(node,
				struct nvmap_pid_data, node);

		if (p->pid > pid)
			node = node->rb_left;
		else if (p->pid < pid)
			node = node->rb_right;
		else
			return p;
	}
	return NULL;
}

static void nvmap_pid_put_locked(struct nvmap_device *dev, pid_t pid)
{
	struct nvmap_pid_data *p = nvmap_pid_find_locked(dev, pid);
	if (p)
		kref_put(&p->refcount, nvmap_pid_release_locked);
}

struct nvmap_client *__nvmap_create_client(struct nvmap_device *dev,
					   const char *name)
{
	struct nvmap_client *client;
	struct task_struct *task;
	pid_t pid;
	bool is_existing_client = false;

	if (WARN_ON(!dev))
		return NULL;

	get_task_struct(current->group_leader);
	task_lock(current->group_leader);
	/* don't bother to store task struct for kernel threads,
	   they can't be killed anyway */
	if (current->flags & PF_KTHREAD) {
		put_task_struct(current->group_leader);
		task = NULL;
	} else {
		task = current->group_leader;
	}
	task_unlock(current->group_leader);

	pid = task ? task->pid : 0;

	mutex_lock(&dev->clients_lock);
	list_for_each_entry(client, &nvmap_dev->clients, list) {
		if (nvmap_client_pid(client) == pid) {
			/* Increment counter to track number of namespaces of a process */
			atomic_add(1, &client->count);
			put_task_struct(current->group_leader);
			is_existing_client = true;
			goto unlock;
		}
	}
unlock:
	if (is_existing_client) {
		mutex_unlock(&dev->clients_lock);
		return client;
	}

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client) {
		mutex_unlock(&dev->clients_lock);
		return NULL;
	}
	client->name = name;
	client->handle_refs = RB_ROOT;
	client->task = task;

	mutex_init(&client->ref_lock);
	atomic_set(&client->count, 1);
	client->kernel_client = false;
	nvmap_id_array_init(&client->id_array);

#ifdef NVMAP_CONFIG_HANDLE_AS_ID
	client->ida = &client->id_array;
#else
	client->ida = NULL;
#endif

	list_add(&client->list, &dev->clients);
	if (!IS_ERR_OR_NULL(dev->handles_by_pid)) {
		pid_t pid = nvmap_client_pid(client);
		nvmap_pid_get_locked(dev, pid);
	}
	mutex_unlock(&dev->clients_lock);
	return client;
}

static void destroy_client(struct nvmap_client *client)
{
	struct rb_node *n;

	if (!client)
		return;

	mutex_lock(&nvmap_dev->clients_lock);
	/*
	 * count field tracks the number of namespaces within a process.
	 * Destroy the client only after all namespaces close the /dev/nvmap node.
	 */
	if (atomic_dec_return(&client->count)) {
		mutex_unlock(&nvmap_dev->clients_lock);
		return;
	}

	nvmap_id_array_exit(&client->id_array);
#ifdef NVMAP_CONFIG_HANDLE_AS_ID
	client->ida = NULL;
#endif
	if (!IS_ERR_OR_NULL(nvmap_dev->handles_by_pid)) {
		pid_t pid = nvmap_client_pid(client);
		nvmap_pid_put_locked(nvmap_dev, pid);
	}
	list_del(&client->list);
	mutex_unlock(&nvmap_dev->clients_lock);

	while ((n = rb_first(&client->handle_refs))) {
		struct nvmap_handle_ref *ref;
		int dupes;

		ref = rb_entry(n, struct nvmap_handle_ref, node);
		smp_rmb();
		if (ref->handle->owner == client)
			ref->handle->owner = NULL;

		if (ref->is_ro)
			dma_buf_put(ref->handle->dmabuf_ro);
		else
			dma_buf_put(ref->handle->dmabuf);
		rb_erase(&ref->node, &client->handle_refs);
		atomic_dec(&ref->handle->share_count);

		dupes = atomic_read(&ref->dupes);
		while (dupes--)
			nvmap_handle_put(ref->handle);

		kfree(ref);
	}

	if (client->task)
		put_task_struct(client->task);

	kfree(client);
}

static int nvmap_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	struct nvmap_device *dev = dev_get_drvdata(miscdev->parent);
	struct nvmap_client *priv;
	int ret;
	__attribute__((unused)) struct rlimit old_rlim, new_rlim;

	ret = nonseekable_open(inode, filp);
	if (unlikely(ret))
		return ret;

	BUG_ON(dev != nvmap_dev);
	priv = __nvmap_create_client(dev, "user");
	if (!priv)
		return -ENOMEM;
	trace_nvmap_open(priv, priv->name);

	filp->private_data = priv;
	return 0;
}

static int nvmap_release(struct inode *inode, struct file *filp)
{
	struct nvmap_client *priv = filp->private_data;

	if(!priv)
		return 0;

	trace_nvmap_release(priv, priv->name);
	destroy_client(priv);
	return 0;
}

static int nvmap_map(struct file *filp, struct vm_area_struct *vma)
{
	char task_comm[TASK_COMM_LEN];

	get_task_comm(task_comm, current);
	pr_debug("error: mmap not supported on nvmap file, pid=%d, %s\n",
		  task_tgid_nr(current), task_comm);
	return -EPERM;
}

static long nvmap_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	void __user *uarg = (void __user *)arg;

	if (_IOC_TYPE(cmd) != NVMAP_IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > NVMAP_IOC_MAXNR)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !ACCESS_OK(VERIFY_WRITE, uarg, _IOC_SIZE(cmd));
	if (!err && (_IOC_DIR(cmd) & _IOC_WRITE))
		err = !ACCESS_OK(VERIFY_READ, uarg, _IOC_SIZE(cmd));

	if (err)
		return -EFAULT;

	err = -ENOTTY;

	switch (cmd) {
	case NVMAP_IOC_CREATE:
	case NVMAP_IOC_CREATE_64:
	case NVMAP_IOC_FROM_FD:
		err = nvmap_ioctl_create(filp, cmd, uarg);
		break;

	case NVMAP_IOC_FROM_VA:
		err = nvmap_ioctl_create_from_va(filp, uarg);
		break;

	case NVMAP_IOC_GET_FD:
		err = nvmap_ioctl_getfd(filp, uarg);
		break;

	case NVMAP_IOC_GET_IVM_HEAPS:
		err = nvmap_ioctl_get_ivc_heap(filp, uarg);
		break;

	case NVMAP_IOC_FROM_IVC_ID:
		err = nvmap_ioctl_create_from_ivc(filp, uarg);
		break;

	case NVMAP_IOC_GET_IVC_ID:
		err = nvmap_ioctl_get_ivcid(filp, uarg);
		break;

	case NVMAP_IOC_ALLOC:
		err = nvmap_ioctl_alloc(filp, uarg);
		break;

	case NVMAP_IOC_ALLOC_IVM:
		err = nvmap_ioctl_alloc_ivm(filp, uarg);
		break;

	case NVMAP_IOC_VPR_FLOOR_SIZE:
		err = nvmap_ioctl_vpr_floor_size(filp, uarg);
		break;

	case NVMAP_IOC_FREE:
		err = nvmap_ioctl_free(filp, arg);
		break;

	case NVMAP_IOC_DUP_HANDLE:
		err = nvmap_ioctl_dup_handle(filp, uarg);
		break;

#ifdef CONFIG_COMPAT
	case NVMAP_IOC_WRITE_32:
	case NVMAP_IOC_READ_32:
		err = nvmap_ioctl_rw_handle(filp, cmd == NVMAP_IOC_READ_32,
			uarg, sizeof(struct nvmap_rw_handle_32));
		break;
#endif

	case NVMAP_IOC_WRITE:
	case NVMAP_IOC_READ:
		err = nvmap_ioctl_rw_handle(filp, cmd == NVMAP_IOC_READ, uarg,
			sizeof(struct nvmap_rw_handle));
		break;

#ifdef CONFIG_COMPAT
	case NVMAP_IOC_CACHE_32:
		err = nvmap_ioctl_cache_maint(filp, uarg,
			sizeof(struct nvmap_cache_op_32));
		break;
#endif

	case NVMAP_IOC_CACHE:
		err = nvmap_ioctl_cache_maint(filp, uarg,
			sizeof(struct nvmap_cache_op));
		break;

	case NVMAP_IOC_CACHE_64:
		err = nvmap_ioctl_cache_maint(filp, uarg,
			sizeof(struct nvmap_cache_op_64));
		break;

	case NVMAP_IOC_CACHE_LIST:
		err = nvmap_ioctl_cache_maint_list(filp, uarg);
		break;

	case NVMAP_IOC_GUP_TEST:
		err = nvmap_ioctl_gup_test(filp, uarg);
		break;

	case NVMAP_IOC_FROM_ID:
	case NVMAP_IOC_GET_ID:
		pr_warn("NVMAP_IOC_GET_ID/FROM_ID pair is deprecated. "
			"Use the pair NVMAP_IOC_GET_FD/FROM_FD.\n");
		break;

	case NVMAP_IOC_SET_TAG_LABEL:
		err = nvmap_ioctl_set_tag_label(filp, uarg);
		break;

	case NVMAP_IOC_GET_AVAILABLE_HEAPS:
		err = nvmap_ioctl_get_available_heaps(filp, uarg);
		break;

	case NVMAP_IOC_GET_HEAP_SIZE:
		err = nvmap_ioctl_get_heap_size(filp, uarg);
		break;

	case NVMAP_IOC_PARAMETERS:
		err = nvmap_ioctl_get_handle_parameters(filp, uarg);
		break;

	case NVMAP_IOC_GET_SCIIPCID:
		err = nvmap_ioctl_get_sci_ipc_id(filp, uarg);
		break;

	case NVMAP_IOC_HANDLE_FROM_SCIIPCID:
		err = nvmap_ioctl_handle_from_sci_ipc_id(filp, uarg);
		break;

	case NVMAP_IOC_QUERY_HEAP_PARAMS:
		err = nvmap_ioctl_query_heap_params(filp, uarg);
		break;
	case NVMAP_IOC_GET_FD_FOR_RANGE_FROM_LIST:
		err = nvmap_ioctl_get_fd_from_list(filp, uarg);
		break;
	default:
		pr_warn("Unknown NVMAP_IOC = 0x%x\n", cmd);
	}
	return err;
}

#define DEBUGFS_OPEN_FOPS_STATIC(name) \
static int nvmap_debug_##name##_open(struct inode *inode, \
					    struct file *file) \
{ \
	return single_open(file, nvmap_debug_##name##_show, \
			    inode->i_private); \
} \
\
static const struct file_operations debug_##name##_fops = { \
	.open = nvmap_debug_##name##_open, \
	.read = seq_read, \
	.llseek = seq_lseek, \
	.release = single_release, \
}

#define DEBUGFS_OPEN_FOPS(name) \
static int nvmap_debug_##name##_open(struct inode *inode, \
					    struct file *file) \
{ \
	return single_open(file, nvmap_debug_##name##_show, \
			    inode->i_private); \
} \
\
const struct file_operations debug_##name##_fops = { \
	.open = nvmap_debug_##name##_open, \
	.read = seq_read, \
	.llseek = seq_lseek, \
	.release = single_release, \
}

#define K(x) (x >> 10)

static void client_stringify(struct nvmap_client *client, struct seq_file *s)
{
	char task_comm[TASK_COMM_LEN];
	if (!client->task) {
		seq_printf(s, "%-18s %18s %8u", client->name, "kernel", 0);
		return;
	}
	get_task_comm(task_comm, client->task);
	seq_printf(s, "%-18s %18s %8u", client->name, task_comm,
		   client->task->pid);
}

static void allocations_stringify(struct nvmap_client *client,
				  struct seq_file *s, u32 heap_type)
{
	struct rb_node *n;
	unsigned int pin_count = 0;
	struct nvmap_device *dev = nvmap_dev;

	nvmap_ref_lock(client);
	mutex_lock(&dev->tags_lock);
	n = rb_first(&client->handle_refs);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle_ref *ref =
			rb_entry(n, struct nvmap_handle_ref, node);
		struct nvmap_handle *handle = ref->handle;
		if (handle->alloc && handle->heap_type == heap_type) {
			phys_addr_t base = heap_type == NVMAP_HEAP_IOVMM ? 0 :
					   handle->heap_pgalloc ? 0 :
					   (handle->carveout->base);
			size_t size = K(handle->size);
			int i = 0;

next_page:
			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				base = page_to_phys(handle->pgalloc.pages[i++]);
				size = K(PAGE_SIZE);
			}

			seq_printf(s,
				"%-18s %-18s %8llx %10zuK %8x %6u %6u %6u %6u %6u %6u %8pK %s\n",
				"", "",
				(unsigned long long)base, size,
				handle->userflags,
				atomic_read(&handle->ref),
				atomic_read(&ref->dupes),
				pin_count,
				atomic_read(&handle->kmap_count),
				atomic_read(&handle->umap_count),
				atomic_read(&handle->share_count),
				handle,
				__nvmap_tag_name(dev, handle->userflags >> 16));

			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				i++;
				if (i < (handle->size >> PAGE_SHIFT))
					goto next_page;
			}
		}
	}
	mutex_unlock(&dev->tags_lock);
	nvmap_ref_unlock(client);
}

bool is_nvmap_memory_available(size_t size, uint32_t heap)
{
	unsigned int carveout_mask = NVMAP_HEAP_CARVEOUT_MASK;
	unsigned int iovmm_mask = NVMAP_HEAP_IOVMM;
	struct nvmap_device *dev = nvmap_dev;
	bool heap_present = false;
	int i;
	unsigned long free_mem = 0;

	if (!heap)
		return false;

	if (nvmap_convert_carveout_to_iovmm) {
		carveout_mask &= ~NVMAP_HEAP_CARVEOUT_GENERIC;
		iovmm_mask |= NVMAP_HEAP_CARVEOUT_GENERIC;
	} else if (nvmap_convert_iovmm_to_carveout) {
		if (heap & NVMAP_HEAP_IOVMM) {
			heap &= ~NVMAP_HEAP_IOVMM;
			heap |= NVMAP_HEAP_CARVEOUT_GENERIC;
		}
	}

	if (heap & iovmm_mask) {
		if (system_heap_free_mem(&free_mem)) {
			pr_debug("Call to system_heap_free_mem failed\n");
			return false;
		}

		if (size > (free_mem & PAGE_MASK)) {
			pr_debug("Requested size is more than available memory\n");
			pr_debug("Requested size : %lu B, Available memory : %lu B\n", size,
					free_mem & PAGE_MASK);
			return false;
		}
		return true;
	}

	for (i = 0; i < dev->nr_carveouts; i++) {
		struct nvmap_carveout_node *co_heap;
		struct nvmap_heap *h;

		co_heap = &dev->heaps[i];
		if (!(co_heap->heap_bit & heap))
			continue;

		heap_present = true;
		h = co_heap->carveout;
		if (size > (h->free_size & PAGE_MASK)) {
			pr_debug("Requested size is more than available memory");
			pr_debug("Requested size : %lu B, Available memory : %lu B\n", size,
					(h->free_size & PAGE_MASK));
                        return false;
                }
		break;
	}
	return heap_present;
}

/* compute the total amount of handle physical memory that is mapped
 * into client's virtual address space. Remember that vmas list is
 * sorted in ascending order of handle offsets.
 * NOTE: This function should be called while holding handle's lock mutex.
 */
static void nvmap_get_client_handle_mss(struct nvmap_client *client,
				struct nvmap_handle *handle, u64 *total)
{
	struct nvmap_vma_list *vma_list = NULL;
	struct vm_area_struct *vma = NULL;
	u64 end_offset = 0, vma_start_offset, vma_size;
	int64_t overlap_size;

	*total = 0;
	list_for_each_entry(vma_list, &handle->vmas, list) {

		if (client->task->pid == vma_list->pid) {
			vma = vma_list->vma;
			vma_size = vma->vm_end - vma->vm_start;

			vma_start_offset = vma->vm_pgoff << PAGE_SHIFT;
			if (end_offset < vma_start_offset + vma_size) {
				*total += vma_size;

				overlap_size = end_offset - vma_start_offset;
				if (overlap_size > 0)
					*total -= overlap_size;
				end_offset = vma_start_offset + vma_size;
			}
		}
	}
}

static void maps_stringify(struct nvmap_client *client,
				struct seq_file *s, u32 heap_type)
{
	struct rb_node *n;
	struct nvmap_vma_list *vma_list = NULL;
	struct vm_area_struct *vma = NULL;
	u64 total_mapped_size, vma_size;

	nvmap_ref_lock(client);
	n = rb_first(&client->handle_refs);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle_ref *ref =
			rb_entry(n, struct nvmap_handle_ref, node);
		struct nvmap_handle *handle = ref->handle;
		if (handle->alloc && handle->heap_type == heap_type) {
			phys_addr_t base = heap_type == NVMAP_HEAP_IOVMM ? 0 :
					   handle->heap_pgalloc ? 0 :
					   (handle->carveout->base);
			size_t size = K(handle->size);
			int i = 0;

next_page:
			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				base = page_to_phys(handle->pgalloc.pages[i++]);
				size = K(PAGE_SIZE);
			}

			seq_printf(s,
				"%-18s %-18s %8llx %10zuK %8x %6u %16pK "
				"%12s %12s ",
				"", "",
				(unsigned long long)base, K(handle->size),
				handle->userflags,
				atomic_read(&handle->share_count),
				handle, "", "");

			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				i++;
				if (i < (handle->size >> PAGE_SHIFT))
					goto next_page;
			}

			mutex_lock(&handle->lock);
			nvmap_get_client_handle_mss(client, handle,
							&total_mapped_size);
			seq_printf(s, "%6lluK\n", K(total_mapped_size));

			list_for_each_entry(vma_list, &handle->vmas, list) {

				if (vma_list->pid == client->task->pid) {
					vma = vma_list->vma;
					vma_size = vma->vm_end - vma->vm_start;
					seq_printf(s,
					  "%-18s %-18s %8s %11s %8s %6s %16s "
					  "%-12lx-%12lx %6lluK\n",
					  "", "", "", "", "", "", "",
					  vma->vm_start, vma->vm_end,
					  K(vma_size));
				}
			}
			mutex_unlock(&handle->lock);
		}
	}
	nvmap_ref_unlock(client);
}

static void nvmap_get_client_mss(struct nvmap_client *client,
				 u64 *total, u32 heap_type)
{
	struct rb_node *n;

	*total = 0;
	nvmap_ref_lock(client);
	n = rb_first(&client->handle_refs);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle_ref *ref =
			rb_entry(n, struct nvmap_handle_ref, node);
		struct nvmap_handle *handle = ref->handle;
		if (handle->alloc && handle->heap_type == heap_type)
			*total += handle->size /
				  atomic_read(&handle->share_count);
	}
	nvmap_ref_unlock(client);
}

static int nvmap_page_mapcount(struct page *page)
{
	int mapcount = atomic_read(&page->_mapcount) + 1;

	/* Handle page_has_type() pages */
	if (page_has_type(page))
		mapcount = 0;
	if (unlikely(PageCompound(page)))
#if defined(NV_FOLIO_ENTIRE_MAPCOUNT_PRESENT) /* Linux v5.18 */
		mapcount += folio_entire_mapcount(page_folio(page));
#else
		mapcount += compound_mapcount(page);
#endif

	return mapcount;
}

#define PSS_SHIFT 12
static void nvmap_get_total_mss(u64 *pss, u64 *total, u32 heap_type)
{
	int i;
	struct rb_node *n;
	struct nvmap_device *dev = nvmap_dev;

	*total = 0;
	if (pss)
		*pss = 0;
	if (!dev)
		return;
	spin_lock(&dev->handle_lock);
	n = rb_first(&dev->handles);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle *h =
			rb_entry(n, struct nvmap_handle, node);

		if (!h || !h->alloc || h->heap_type != heap_type)
			continue;

		*total += h->size;
		if (!pss)
			continue;

		for (i = 0; i < h->size >> PAGE_SHIFT; i++) {
			struct page *page = nvmap_to_page(h->pgalloc.pages[i]);

			if (nvmap_page_mapcount(page) > 0)
				*pss += PAGE_SIZE;
		}
	}
	spin_unlock(&dev->handle_lock);
}

static int nvmap_debug_allocations_show(struct seq_file *s, void *unused)
{
	u64 total;
	struct nvmap_client *client;
	u32 heap_type = (u32)(uintptr_t)s->private;

	mutex_lock(&nvmap_dev->clients_lock);
	seq_printf(s, "%-18s %18s %8s %11s\n",
		"CLIENT", "PROCESS", "PID", "SIZE");
	seq_printf(s, "%-18s %18s %8s %11s %8s %6s %6s %6s %6s %6s %6s %8s\n",
			"", "", "BASE", "SIZE", "FLAGS", "REFS",
			"DUPES", "PINS", "KMAPS", "UMAPS", "SHARE", "UID");
	list_for_each_entry(client, &nvmap_dev->clients, list) {
		u64 client_total;
		client_stringify(client, s);
		nvmap_get_client_mss(client, &client_total, heap_type);
		seq_printf(s, " %10lluK\n", K(client_total));
		allocations_stringify(client, s, heap_type);
		seq_printf(s, "\n");
	}
	mutex_unlock(&nvmap_dev->clients_lock);
	nvmap_get_total_mss(NULL, &total, heap_type);
	seq_printf(s, "%-18s %-18s %8s %10lluK\n", "total", "", "", K(total));
	return 0;
}

DEBUGFS_OPEN_FOPS(allocations);

static int nvmap_debug_free_size_show(struct seq_file *s, void *unused)
{
	unsigned long free_mem = 0;

	if (system_heap_free_mem(&free_mem))
		seq_printf(s, "Error while fetching free size of IOVMM memory\n");
	else
		seq_printf(s, "Max allocatable IOVMM memory: %lu bytes\n", free_mem);
	return 0;
}
DEBUGFS_OPEN_FOPS(free_size);

#ifdef NVMAP_CONFIG_DEBUG_MAPS
static int nvmap_debug_device_list_show(struct seq_file *s, void *unused)
{
	u32 heap_type = (u32)(uintptr_t)s->private;
	struct rb_node *n = NULL;
	struct nvmap_device_list *dl = NULL;
	int i;

	if (heap_type == NVMAP_HEAP_IOVMM) {
		n = rb_first(&nvmap_dev->device_names);
	} else {
		/* Iterate over all heaps to find the matching heap */
		for (i = 0; i < nvmap_dev->nr_carveouts; i++) {
			if (heap_type & nvmap_dev->heaps[i].heap_bit) {
				if (nvmap_dev->heaps[i].carveout) {
					n = rb_first(&nvmap_dev->heaps[i].carveout->device_names);
					break;
				}
			}
		}
	}
	if (n) {
		seq_printf(s, "Device list is\n");
		for (; n != NULL; n = rb_next(n)) {
			dl = rb_entry(n, struct nvmap_device_list, node);
			seq_printf(s, "%s %llu\n", dl->device_name, dl->dma_mask);
		}
	}
	return 0;
}
DEBUGFS_OPEN_FOPS(device_list);
#endif /* NVMAP_CONFIG_DEBUG_MAPS */

static int nvmap_debug_all_allocations_show(struct seq_file *s, void *unused)
{
	u32 heap_type = (u32)(uintptr_t)s->private;
	struct rb_node *n;


	spin_lock(&nvmap_dev->handle_lock);
	seq_printf(s, "%8s %11s %9s %6s %6s %6s %6s %8s\n",
			"BASE", "SIZE", "USERFLAGS", "REFS",
			"KMAPS", "UMAPS", "SHARE", "UID");

	/* for each handle */
	n = rb_first(&nvmap_dev->handles);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle *handle =
			rb_entry(n, struct nvmap_handle, node);
		int i = 0;

		if (handle->alloc && handle->heap_type == heap_type) {
			phys_addr_t base = heap_type == NVMAP_HEAP_IOVMM ? 0 :
					   handle->heap_pgalloc ? 0 :
					   (handle->carveout->base);
			size_t size = K(handle->size);

next_page:
			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				base = page_to_phys(handle->pgalloc.pages[i++]);
				size = K(PAGE_SIZE);
			}

			seq_printf(s,
				"%8llx %10zuK %9x %6u %6u %6u %6u %8p\n",
				(unsigned long long)base, K(handle->size),
				handle->userflags,
				atomic_read(&handle->ref),
				atomic_read(&handle->kmap_count),
				atomic_read(&handle->umap_count),
				atomic_read(&handle->share_count),
				handle);

			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				i++;
				if (i < (handle->size >> PAGE_SHIFT))
					goto next_page;
			}
		}
	}

	spin_unlock(&nvmap_dev->handle_lock);

	return 0;
}

DEBUGFS_OPEN_FOPS(all_allocations);

static int nvmap_debug_orphan_handles_show(struct seq_file *s, void *unused)
{
	u32 heap_type = (u32)(uintptr_t)s->private;
	struct rb_node *n;


	spin_lock(&nvmap_dev->handle_lock);
	seq_printf(s, "%8s %11s %9s %6s %6s %6s %8s\n",
			"BASE", "SIZE", "USERFLAGS", "REFS",
			"KMAPS", "UMAPS", "UID");

	/* for each handle */
	n = rb_first(&nvmap_dev->handles);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle *handle =
			rb_entry(n, struct nvmap_handle, node);
		int i = 0;

		if (handle->alloc && handle->heap_type == heap_type &&
			!atomic_read(&handle->share_count)) {
			phys_addr_t base = heap_type == NVMAP_HEAP_IOVMM ? 0 :
					   handle->heap_pgalloc ? 0 :
					   (handle->carveout->base);
			size_t size = K(handle->size);

next_page:
			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				base = page_to_phys(handle->pgalloc.pages[i++]);
				size = K(PAGE_SIZE);
			}

			seq_printf(s,
				"%8llx %10zuK %9x %6u %6u %6u %8p\n",
				(unsigned long long)base, K(handle->size),
				handle->userflags,
				atomic_read(&handle->ref),
				atomic_read(&handle->kmap_count),
				atomic_read(&handle->umap_count),
				handle);

			if ((heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
				i++;
				if (i < (handle->size >> PAGE_SHIFT))
					goto next_page;
			}
		}
	}

	spin_unlock(&nvmap_dev->handle_lock);

	return 0;
}

DEBUGFS_OPEN_FOPS(orphan_handles);

static int nvmap_debug_maps_show(struct seq_file *s, void *unused)
{
	u64 total;
	struct nvmap_client *client;
	u32 heap_type = (u32)(uintptr_t)s->private;

	mutex_lock(&nvmap_dev->clients_lock);
	seq_printf(s, "%-18s %18s %8s %11s\n",
		"CLIENT", "PROCESS", "PID", "SIZE");
	seq_printf(s, "%-18s %18s %8s %11s %8s %6s %9s %21s %18s\n",
		"", "", "BASE", "SIZE", "FLAGS", "SHARE", "UID",
		"MAPS", "MAPSIZE");

	list_for_each_entry(client, &nvmap_dev->clients, list) {
		u64 client_total;
		client_stringify(client, s);
		nvmap_get_client_mss(client, &client_total, heap_type);
		seq_printf(s, " %10lluK\n", K(client_total));
		maps_stringify(client, s, heap_type);
		seq_printf(s, "\n");
	}
	mutex_unlock(&nvmap_dev->clients_lock);

	nvmap_get_total_mss(NULL, &total, heap_type);
	seq_printf(s, "%-18s %-18s %8s %10lluK\n", "total", "", "", K(total));
	return 0;
}

DEBUGFS_OPEN_FOPS(maps);

static int nvmap_debug_clients_show(struct seq_file *s, void *unused)
{
	u64 total;
	struct nvmap_client *client;
	ulong heap_type = (ulong)s->private;

	mutex_lock(&nvmap_dev->clients_lock);
	seq_printf(s, "%-18s %18s %8s %11s\n",
		"CLIENT", "PROCESS", "PID", "SIZE");
	list_for_each_entry(client, &nvmap_dev->clients, list) {
		u64 client_total;
		client_stringify(client, s);
		nvmap_get_client_mss(client, &client_total, heap_type);
		seq_printf(s, " %10lluK\n", K(client_total));
	}
	mutex_unlock(&nvmap_dev->clients_lock);
	nvmap_get_total_mss(NULL, &total, heap_type);
	seq_printf(s, "%-18s %18s %8s %10lluK\n", "total", "", "", K(total));
	return 0;
}

DEBUGFS_OPEN_FOPS(clients);

static int nvmap_debug_handles_by_pid_show_client(struct seq_file *s,
		struct nvmap_client *client)
{
	struct rb_node *n;
	int ret = 0;

	nvmap_ref_lock(client);
	n = rb_first(&client->handle_refs);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle_ref *ref = rb_entry(n,
				struct nvmap_handle_ref, node);
		struct nvmap_handle *handle = ref->handle;
		struct nvmap_debugfs_handles_entry entry;
		u64 total_mapped_size;
		int i = 0;

		if (!handle->alloc)
			continue;

		mutex_lock(&handle->lock);
		nvmap_get_client_handle_mss(client, handle, &total_mapped_size);
		mutex_unlock(&handle->lock);

		entry.base = handle->heap_type == NVMAP_HEAP_IOVMM ? 0 :
			     handle->heap_pgalloc ? 0 :
			     (handle->carveout->base);
		entry.size = handle->size;
		entry.flags = handle->userflags;
		entry.share_count = atomic_read(&handle->share_count);
		entry.mapped_size = total_mapped_size;

next_page:
		if ((handle->heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
			entry.base = page_to_phys(handle->pgalloc.pages[i++]);
			entry.size = K(PAGE_SIZE);
		}

		seq_printf(s, "%llu %12llu %8u %8u %10llu\n", entry.base, entry.size,
				entry.flags, entry.share_count, entry.mapped_size);

		if ((handle->heap_type == NVMAP_HEAP_CARVEOUT_VPR) && handle->heap_pgalloc) {
			i++;
			if (i < (handle->size >> PAGE_SHIFT))
				goto next_page;
		}
	}
	nvmap_ref_unlock(client);

	return ret;
}

static int nvmap_debug_handles_by_pid_show(struct seq_file *s, void *unused)
{
	struct nvmap_pid_data *p = s->private;
	struct nvmap_client *client;
	struct nvmap_debugfs_handles_header header;
	int ret = 0;

	header.version = 1;
	seq_printf(s, "%s: %u\n", "header.version", header.version);
	seq_printf(s, "%s %8s %8s %12s %8s\n", "base",
		"size", "flags", "share_count", "mapped_size");
	mutex_lock(&nvmap_dev->clients_lock);

	list_for_each_entry(client, &nvmap_dev->clients, list) {
		if (nvmap_client_pid(client) != p->pid)
			continue;

		ret = nvmap_debug_handles_by_pid_show_client(s, client);
		if (ret < 0)
			break;
	}

	mutex_unlock(&nvmap_dev->clients_lock);
	return ret;
}

DEBUGFS_OPEN_FOPS_STATIC(handles_by_pid);

#define PRINT_MEM_STATS_NOTE(x) \
do { \
	seq_printf(s, "Note: total memory is precise account of pages " \
		"allocated by NvMap.\nIt doesn't match with all clients " \
		"\"%s\" accumulated as shared memory \nis accounted in " \
		"full in each clients \"%s\" that shared memory.\n", #x, #x); \
} while (0)

#ifdef NVMAP_CONFIG_PROCRANK
struct procrank_stats {
	struct vm_area_struct *vma;
	u64 pss;
};

static int procrank_pte_entry(pte_t *pte, unsigned long addr, unsigned long end,
		struct mm_walk *walk)
{
	struct procrank_stats *mss = walk->private;
	struct vm_area_struct *vma = mss->vma;
	struct page *page = NULL;
	int mapcount;

	if (pte_present(*pte))
		page = vm_normal_page(vma, addr, *pte);
	else if (is_swap_pte(*pte)) {
		swp_entry_t swpent = pte_to_swp_entry(*pte);

		if (is_migration_entry(swpent))
			page = migration_entry_to_page(swpent);
	}

	if (!page)
		return 0;

	mapcount = nvmap_page_mapcount(page);
	if (mapcount >= 2)
		mss->pss += (PAGE_SIZE << PSS_SHIFT) / mapcount;
	else
		mss->pss += (PAGE_SIZE << PSS_SHIFT);

	return 0;
}

#ifndef PTRACE_MODE_READ_FSCREDS
#define PTRACE_MODE_READ_FSCREDS PTRACE_MODE_READ
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
static void nvmap_iovmm_get_client_mss(struct nvmap_client *client, u64 *pss,
				   u64 *total)
{
	struct rb_node *n;
	struct nvmap_vma_list *tmp;
	struct procrank_stats mss;
	struct mm_walk procrank_walk = {
		.pte_entry = procrank_pte_entry,
		.private = &mss,
	};
	struct mm_struct *mm;

	memset(&mss, 0, sizeof(mss));
	*pss = *total = 0;

	mm = mm_access(client->task,
			PTRACE_MODE_READ_FSCREDS);
	if (!mm || IS_ERR(mm)) return;

	nvmap_acquire_mmap_read_lock(mm);
	procrank_walk.mm = mm;

	nvmap_ref_lock(client);
	n = rb_first(&client->handle_refs);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle_ref *ref =
			rb_entry(n, struct nvmap_handle_ref, node);
		struct nvmap_handle *h = ref->handle;

		if (!h || !h->alloc || !h->heap_pgalloc)
			continue;

		mutex_lock(&h->lock);
		list_for_each_entry(tmp, &h->vmas, list) {
			if (client->task->pid == tmp->pid) {
				mss.vma = tmp->vma;
				walk_page_range(tmp->vma->vm_start,
						tmp->vma->vm_end,
						&procrank_walk);
			}
		}
		mutex_unlock(&h->lock);
		*total += h->size / atomic_read(&h->share_count);
	}

	nvmap_release_mmap_read_lock(mm);
	mmput(mm);
	*pss = (mss.pss >> PSS_SHIFT);
	nvmap_ref_unlock(client);
}
#else
static void nvmap_iovmm_get_client_mss(struct nvmap_client *client, u64 *pss,
				   u64 *total)
{
	struct mm_walk_ops wk_ops = {
		.pte_entry = procrank_pte_entry,
	};
	struct rb_node *n;
	struct nvmap_vma_list *tmp;
	struct procrank_stats mss;
	struct mm_walk procrank_walk = {
		.ops = &wk_ops,
		.private = &mss,
	};
	struct mm_struct *mm;

	memset(&mss, 0, sizeof(mss));
	*pss = *total = 0;

	mm = mm_access(client->task,
			PTRACE_MODE_READ_FSCREDS);
	if (!mm || IS_ERR(mm)) return;

	nvmap_acquire_mmap_read_lock(mm);
	procrank_walk.mm = mm;

	nvmap_ref_lock(client);
	n = rb_first(&client->handle_refs);
	for (; n != NULL; n = rb_next(n)) {
		struct nvmap_handle_ref *ref =
			rb_entry(n, struct nvmap_handle_ref, node);
		struct nvmap_handle *h = ref->handle;

		if (!h || !h->alloc || !h->heap_pgalloc)
			continue;

		mutex_lock(&h->lock);
		list_for_each_entry(tmp, &h->vmas, list) {
			if (client->task->pid == tmp->pid) {
				mss.vma = tmp->vma;
					walk_page_range(procrank_walk.mm,
						tmp->vma->vm_start,
						tmp->vma->vm_end,
						procrank_walk.ops,
						procrank_walk.private);
			}
		}
		mutex_unlock(&h->lock);
		*total += h->size / atomic_read(&h->share_count);
	}

	nvmap_release_mmap_read_lock(mm);
	mmput(mm);
	*pss = (mss.pss >> PSS_SHIFT);
	nvmap_ref_unlock(client);
}
#endif

static int nvmap_debug_iovmm_procrank_show(struct seq_file *s, void *unused)
{
	u64 pss, total;
	struct nvmap_client *client;
	struct nvmap_device *dev = s->private;
	u64 total_memory, total_pss;

	mutex_lock(&dev->clients_lock);
	seq_printf(s, "%-18s %18s %8s %11s %11s\n",
		"CLIENT", "PROCESS", "PID", "PSS", "SIZE");
	list_for_each_entry(client, &dev->clients, list) {
		client_stringify(client, s);
		nvmap_iovmm_get_client_mss(client, &pss, &total);
		seq_printf(s, " %10lluK %10lluK\n", K(pss), K(total));
	}
	mutex_unlock(&dev->clients_lock);

	nvmap_get_total_mss(&total_pss, &total_memory, NVMAP_HEAP_IOVMM);
	seq_printf(s, "%-18s %18s %8s %10lluK %10lluK\n",
		"total", "", "", K(total_pss), K(total_memory));
	return 0;
}

DEBUGFS_OPEN_FOPS(iovmm_procrank);
#endif /* NVMAP_CONFIG_PROCRANK */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
ulong nvmap_iovmm_get_used_pages(void)
{
	u64 total;

	nvmap_get_total_mss(NULL, &total, NVMAP_HEAP_IOVMM);
	return total >> PAGE_SHIFT;
}
#endif

static void nvmap_iovmm_debugfs_init(void)
{
	if (!IS_ERR_OR_NULL(nvmap_dev->debug_root)) {
		struct dentry *iovmm_root =
			debugfs_create_dir("iovmm", nvmap_dev->debug_root);
		if (!IS_ERR_OR_NULL(iovmm_root)) {
			debugfs_create_file("clients", S_IRUGO, iovmm_root,
				(void *)(uintptr_t)NVMAP_HEAP_IOVMM,
				&debug_clients_fops);
			debugfs_create_file("allocations", S_IRUGO, iovmm_root,
				(void *)(uintptr_t)NVMAP_HEAP_IOVMM,
				&debug_allocations_fops);
			debugfs_create_file("all_allocations", S_IRUGO,
				iovmm_root, (void *)(uintptr_t)NVMAP_HEAP_IOVMM,
				&debug_all_allocations_fops);
			debugfs_create_file("orphan_handles", S_IRUGO,
				iovmm_root, (void *)(uintptr_t)NVMAP_HEAP_IOVMM,
				&debug_orphan_handles_fops);
			debugfs_create_file("maps", S_IRUGO, iovmm_root,
				(void *)(uintptr_t)NVMAP_HEAP_IOVMM,
				&debug_maps_fops);
			debugfs_create_file("free_size", S_IRUGO, iovmm_root,
				(void *)(uintptr_t)NVMAP_HEAP_IOVMM,
				&debug_free_size_fops);
#ifdef NVMAP_CONFIG_DEBUG_MAPS
			debugfs_create_file("device_list", S_IRUGO, iovmm_root,
				(void *)(uintptr_t)NVMAP_HEAP_IOVMM,
				&debug_device_list_fops);
#endif /* NVMAP_CONFIG_DEBUG_MAPS */

#ifdef NVMAP_CONFIG_PROCRANK
			debugfs_create_file("procrank", S_IRUGO, iovmm_root,
				nvmap_dev, &debug_iovmm_procrank_fops);
#endif
		}
	}
}

static bool nvmap_is_iommu_present(void)
{
	struct device_node *np;
	struct property *prop;

	np = of_find_node_by_name(NULL, "iommu");
	while (np) {
		prop = of_find_property(np, "status", NULL);
		if (prop && !strcmp(prop->value, "okay")) {
			of_node_put(np);
			return true;
		}
		of_node_put(np);
		np = of_find_node_by_name(np, "iommu");
	}

	return false;
}

int __init nvmap_probe(struct platform_device *pdev)
{
	struct nvmap_platform_data *plat;
	struct nvmap_device *dev;
	struct dentry *nvmap_debug_root;
	int i;
	int e;
	int generic_carveout_present = 0;
	ulong start_time = sched_clock();

	if (WARN_ON(nvmap_dev != NULL)) {
		dev_err(&pdev->dev, "only one nvmap device may be present\n");
		e = -ENODEV;
		goto finish;
	}

	dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&pdev->dev, "out of memory for device\n");
		e = -ENOMEM;
		goto finish;
	}

	nvmap_init(pdev);

	plat = pdev->dev.platform_data;
#ifndef NVMAP_LOADABLE_MODULE
	if (!plat) {
		dev_err(&pdev->dev, "no platform data?\n");
		e = -ENODEV;
		goto finish;
	}
#endif /* !NVMAP_LOADABLE_MODULE */

	nvmap_dev = dev;
	nvmap_dev->plat = plat;

	/*
	 * dma_parms need to be set with desired max_segment_size to avoid
	 * DMA map API returning multiple IOVA's for the buffer size > 64KB.
	 */
	pdev->dev.dma_parms = &nvmap_dma_parameters;
	dev->dev_user.minor = MISC_DYNAMIC_MINOR;
	dev->dev_user.name = "nvmap";
	dev->dev_user.fops = &nvmap_user_fops;
	dev->dev_user.parent = &pdev->dev;
	dev->handles = RB_ROOT;
	dev->serial_id_counter = 0;

#ifdef NVMAP_CONFIG_PAGE_POOLS
	e = nvmap_page_pool_init(dev);
	if (e)
		goto fail;
#endif

	spin_lock_init(&dev->handle_lock);
	INIT_LIST_HEAD(&dev->clients);
	dev->pids = RB_ROOT;
	mutex_init(&dev->clients_lock);
	INIT_LIST_HEAD(&dev->lru_handles);
	spin_lock_init(&dev->lru_lock);
	dev->tags = RB_ROOT;
	mutex_init(&dev->tags_lock);
	mutex_init(&dev->carveout_lock);

	nvmap_debug_root = debugfs_create_dir("nvmap", NULL);
	nvmap_dev->debug_root = nvmap_debug_root;
	if (IS_ERR_OR_NULL(nvmap_debug_root))
		dev_err(&pdev->dev, "couldn't create debug files\n");
	else {
		debugfs_create_u32("max_handle_count", S_IRUGO,
				   nvmap_debug_root, &nvmap_max_handle_count);
		nvmap_dev->handles_by_pid = debugfs_create_dir("handles_by_pid",
							nvmap_debug_root);
#if defined(CONFIG_DEBUG_FS)
		debugfs_create_ulong("nvmap_init_time", S_IRUGO | S_IWUSR,
				     nvmap_dev->debug_root, &nvmap_init_time);
#endif
	}
	nvmap_dev->dynamic_dma_map_mask = ~0U;
	nvmap_dev->cpu_access_mask = ~0U;
#ifdef NVMAP_CONFIG_CACHE_FLUSH_AT_ALLOC
	nvmap_dev->co_cache_flush_at_alloc = true;
#endif /* NVMAP_CONFIG_CACHE_FLUSH_AT_ALLOC */
	if (plat)
		for (i = 0; i < plat->nr_carveouts; i++)
			nvmap_create_carveout(&plat->carveouts[i]);
#ifdef NVMAP_CONFIG_DEBUG_MAPS
	nvmap_dev->device_names = RB_ROOT;
#endif /* NVMAP_CONFIG_DEBUG_MAPS */
	nvmap_iovmm_debugfs_init();
#ifdef NVMAP_CONFIG_PAGE_POOLS
	nvmap_page_pool_debugfs_init(nvmap_dev->debug_root);
#endif
	nvmap_stats_init(nvmap_debug_root);
	platform_set_drvdata(pdev, dev);

	e = nvmap_dmabuf_stash_init();
	if (e)
		goto fail_heaps;

	for (i = 0; i < dev->nr_carveouts; i++)
		if (dev->heaps[i].heap_bit & NVMAP_HEAP_CARVEOUT_GENERIC)
			generic_carveout_present = 1;

	if (generic_carveout_present) {
		if (!iommu_present(&platform_bus_type) &&
			!nvmap_is_iommu_present())
			nvmap_convert_iovmm_to_carveout = 1;
		else if (!of_property_read_bool(pdev->dev.of_node,
				"dont-convert-iovmm-to-carveout"))
			nvmap_convert_iovmm_to_carveout = 1;
	} else {
		nvmap_convert_carveout_to_iovmm = 1;
	}

#ifdef NVMAP_CONFIG_PAGE_POOLS
	if (nvmap_convert_iovmm_to_carveout)
		nvmap_page_pool_fini(dev);
#endif

	e = nvmap_sci_ipc_init();
	if (e)
		goto fail_heaps;

	e = misc_register(&dev->dev_user);
	if (e) {
		dev_err(&pdev->dev, "unable to register miscdevice %s\n",
			dev->dev_user.name);
		goto fail_sci_ipc;
	}
	goto finish;
fail_sci_ipc:
	nvmap_sci_ipc_exit();
fail_heaps:
	debugfs_remove_recursive(nvmap_dev->debug_root);
	for (i = 0; i < dev->nr_carveouts; i++) {
		struct nvmap_carveout_node *node = &dev->heaps[i];
		nvmap_heap_destroy(node->carveout);
	}
fail:
#ifdef NVMAP_CONFIG_PAGE_POOLS
	nvmap_page_pool_fini(nvmap_dev);
#endif
	kfree(dev->heaps);
	if (dev->dev_user.minor != MISC_DYNAMIC_MINOR)
		misc_deregister(&dev->dev_user);
	nvmap_dev = NULL;
finish:
	nvmap_init_time += sched_clock() - start_time;
	return e;
}

int nvmap_remove(struct platform_device *pdev)
{
	struct nvmap_device *dev = platform_get_drvdata(pdev);
	struct rb_node *n;
	struct nvmap_handle *h;
	int i;

#ifdef NVMAP_CONFIG_SCIIPC
	nvmap_sci_ipc_exit();
#endif
	nvmap_dmabuf_stash_deinit();
	debugfs_remove_recursive(dev->debug_root);
	misc_deregister(&dev->dev_user);
#ifdef NVMAP_CONFIG_PAGE_POOLS
	nvmap_page_pool_clear();
	nvmap_page_pool_fini(nvmap_dev);
#endif
	while ((n = rb_first(&dev->handles))) {
		h = rb_entry(n, struct nvmap_handle, node);
		rb_erase(&h->node, &dev->handles);
		kfree(h);
	}

	for (i = 0; i < dev->nr_carveouts; i++) {
		struct nvmap_carveout_node *node = &dev->heaps[i];
		nvmap_heap_destroy(node->carveout);
	}
	kfree(dev->heaps);

	nvmap_dev = NULL;
	return 0;
}
