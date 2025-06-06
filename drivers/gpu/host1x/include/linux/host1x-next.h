/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2009-2023, NVIDIA Corporation. All rights reserved.
 */

#ifndef __LINUX_HOST1X_H
#define __LINUX_HOST1X_H

#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/dma-fence.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>
#include <linux/types.h>

enum host1x_class {
	HOST1X_CLASS_HOST1X = 0x1,
	HOST1X_CLASS_NVJPG1 = 0x7,
	HOST1X_CLASS_NVENC = 0x21,
	HOST1X_CLASS_NVENC1 = 0x22,
	HOST1X_CLASS_GR2D = 0x51,
	HOST1X_CLASS_GR2D_SB = 0x52,
	HOST1X_CLASS_VIC = 0x5D,
	HOST1X_CLASS_GR3D = 0x60,
	HOST1X_CLASS_NVJPG = 0xC0,
	HOST1X_CLASS_NVDEC = 0xF0,
	HOST1X_CLASS_NVDEC1 = 0xF5,
	HOST1X_CLASS_OFA = 0xF8,
};

enum host1x_actmon_wmark_event {
	HOST1X_ACTMON_AVG_WMARK_BELOW,
	HOST1X_ACTMON_AVG_WMARK_ABOVE,
	HOST1X_ACTMON_CONSEC_WMARK_BELOW,
	HOST1X_ACTMON_CONSEC_WMARK_ABOVE,
};

struct host1x;
struct host1x_client;
struct iommu_group;

u64 host1x_get_dma_mask(struct host1x *host1x);

/**
 * struct host1x_bo_cache - host1x buffer object cache
 * @mappings: list of mappings
 * @lock: synchronizes accesses to the list of mappings
 *
 * Note that entries are not periodically evicted from this cache and instead need to be
 * explicitly released. This is used primarily for DRM/KMS where the cache's reference is
 * released when the last reference to a buffer object represented by a mapping in this
 * cache is dropped.
 */
struct host1x_bo_cache {
	struct list_head mappings;
	struct mutex lock;
};

static inline void host1x_bo_cache_init(struct host1x_bo_cache *cache)
{
	INIT_LIST_HEAD(&cache->mappings);
	mutex_init(&cache->lock);
}

static inline void host1x_bo_cache_destroy(struct host1x_bo_cache *cache)
{
	/* XXX warn if not empty? */
	mutex_destroy(&cache->lock);
}

/**
 * struct host1x_client_ops - host1x client operations
 * @early_init: host1x client early initialization code
 * @init: host1x client initialization code
 * @exit: host1x client tear down code
 * @late_exit: host1x client late tear down code
 * @suspend: host1x client suspend code
 * @resume: host1x client resume code
 * @get_rate: host1x client get clock rate code
 * @actmon_event: host1x client actmon event handling code in threaded interrupt context
 */
struct host1x_client_ops {
	int (*early_init)(struct host1x_client *client);
	int (*init)(struct host1x_client *client);
	int (*exit)(struct host1x_client *client);
	int (*late_exit)(struct host1x_client *client);
	int (*suspend)(struct host1x_client *client);
	int (*resume)(struct host1x_client *client);
	unsigned long (*get_rate)(struct host1x_client *client);
	void (*actmon_event)(struct host1x_client *client,
			       enum host1x_actmon_wmark_event event);
};

struct host1x_actmon;

/**
 * struct host1x_client - host1x client structure
 * @list: list node for the host1x client
 * @host: pointer to struct device representing the host1x controller
 * @dev: pointer to struct device backing this host1x client
 * @group: IOMMU group that this client is a member of
 * @ops: host1x client operations
 * @class: host1x class represented by this client
 * @channel: host1x channel associated with this client
 * @syncpts: array of syncpoints requested for this client
 * @num_syncpts: number of syncpoints requested for this client
 * @parent: pointer to parent structure
 * @usecount: reference count for this structure
 * @lock: mutex for mutually exclusive concurrency
 * @cache: host1x buffer object cache
 * @actmon: unit actmon for this client
 */
struct host1x_client {
	struct list_head list;
	struct device *host;
	struct device *dev;
	struct iommu_group *group;

	const struct host1x_client_ops *ops;

	enum host1x_class class;
	struct host1x_channel *channel;

	struct host1x_syncpt **syncpts;
	unsigned int num_syncpts;

	struct host1x_client *parent;
	unsigned int usecount;
	struct mutex lock;

	struct host1x_bo_cache cache;

	struct host1x_actmon *actmon;
};

/*
 * host1x buffer objects
 */

struct host1x_bo;
struct sg_table;

struct host1x_bo_mapping {
	struct kref ref;
	struct dma_buf_attachment *attach;
	enum dma_data_direction direction;
	struct list_head list;
	struct host1x_bo *bo;
	struct sg_table *sgt;
	unsigned int chunks;
	struct device *dev;
	dma_addr_t phys;
	size_t size;

	struct host1x_bo_cache *cache;
	struct list_head entry;
};

static inline struct host1x_bo_mapping *to_host1x_bo_mapping(struct kref *ref)
{
	return container_of(ref, struct host1x_bo_mapping, ref);
}

struct host1x_bo_ops {
	struct host1x_bo *(*get)(struct host1x_bo *bo);
	void (*put)(struct host1x_bo *bo);
	struct host1x_bo_mapping *(*pin)(struct device *dev, struct host1x_bo *bo,
					 enum dma_data_direction dir);
	void (*unpin)(struct host1x_bo_mapping *map);
	void *(*mmap)(struct host1x_bo *bo);
	void (*munmap)(struct host1x_bo *bo, void *addr);
};

struct host1x_bo {
	const struct host1x_bo_ops *ops;
	struct list_head mappings;
	spinlock_t lock;
};

static inline void host1x_bo_init(struct host1x_bo *bo,
				  const struct host1x_bo_ops *ops)
{
	INIT_LIST_HEAD(&bo->mappings);
	spin_lock_init(&bo->lock);
	bo->ops = ops;
}

static inline struct host1x_bo *host1x_bo_get(struct host1x_bo *bo)
{
	return bo->ops->get(bo);
}

static inline void host1x_bo_put(struct host1x_bo *bo)
{
	bo->ops->put(bo);
}

struct host1x_bo_mapping *host1x_bo_pin(struct device *dev, struct host1x_bo *bo,
					enum dma_data_direction dir,
					struct host1x_bo_cache *cache);
void host1x_bo_unpin(struct host1x_bo_mapping *map);

static inline void *host1x_bo_mmap(struct host1x_bo *bo)
{
	return bo->ops->mmap(bo);
}

static inline void host1x_bo_munmap(struct host1x_bo *bo, void *addr)
{
	bo->ops->munmap(bo, addr);
}

/*
 * host1x syncpoints
 */

#define HOST1X_SYNCPT_CLIENT_MANAGED	(1 << 0)
#define HOST1X_SYNCPT_HAS_BASE		(1 << 1)
#define HOST1X_SYNCPT_GPU		(1 << 2)

struct host1x_syncpt_base;
struct host1x_syncpt;
struct host1x;

struct host1x_syncpt *host1x_syncpt_get_by_id(struct host1x *host, u32 id);
struct host1x_syncpt *host1x_syncpt_get_by_id_noref(struct host1x *host, u32 id);
struct host1x_syncpt *host1x_syncpt_get(struct host1x_syncpt *sp);
u32 host1x_syncpt_id(struct host1x_syncpt *sp);
u32 host1x_syncpt_read_min(struct host1x_syncpt *sp);
u32 host1x_syncpt_read_max(struct host1x_syncpt *sp);
u32 host1x_syncpt_read(struct host1x_syncpt *sp);
int host1x_syncpt_incr(struct host1x_syncpt *sp);
u32 host1x_syncpt_incr_max(struct host1x_syncpt *sp, u32 incrs);
int host1x_syncpt_wait_ts(struct host1x_syncpt *sp, u32 thresh, long timeout,
			  u32 *value, ktime_t *ts);
int host1x_syncpt_wait(struct host1x_syncpt *sp, u32 thresh, long timeout,
		       u32 *value);
struct host1x_syncpt *host1x_syncpt_request(struct host1x_client *client,
					    unsigned long flags);
void host1x_syncpt_put(struct host1x_syncpt *sp);
struct host1x_syncpt *host1x_syncpt_alloc(struct host1x *host,
					  unsigned long flags,
					  const char *name);

struct host1x_syncpt_base *host1x_syncpt_get_base(struct host1x_syncpt *sp);
u32 host1x_syncpt_base_id(struct host1x_syncpt_base *base);

void host1x_syncpt_release_vblank_reservation(struct host1x_client *client,
					      u32 syncpt_id);

struct dma_fence *host1x_fence_create(struct host1x_syncpt *sp, u32 threshold,
				      bool timeout);
int host1x_fence_extract(struct dma_fence *fence, u32 *id, u32 *threshold);
void host1x_fence_cancel(struct dma_fence *fence);

/*
 * host1x channel
 */

struct host1x_channel;
struct host1x_job;

struct host1x_channel *host1x_channel_request(struct host1x_client *client);
struct host1x_channel *host1x_channel_get(struct host1x_channel *channel);
void host1x_channel_stop(struct host1x_channel *channel);
void host1x_channel_put(struct host1x_channel *channel);
int host1x_job_submit(struct host1x_job *job);

/*
 * host1x job
 */

#define HOST1X_RELOC_READ	(1 << 0)
#define HOST1X_RELOC_WRITE	(1 << 1)

struct host1x_reloc {
	struct {
		struct host1x_bo *bo;
		unsigned long offset;
	} cmdbuf;
	struct {
		struct host1x_bo *bo;
		unsigned long offset;
	} target;
	unsigned long shift;
	unsigned long flags;
};

struct host1x_job {
	/* When refcount goes to zero, job can be freed */
	struct kref ref;

	/* List entry */
	struct list_head list;

	/* Channel where job is submitted to */
	struct host1x_channel *channel;

	/* client where the job originated */
	struct host1x_client *client;

	/* Gathers and their memory */
	struct host1x_job_cmd *cmds;
	unsigned int num_cmds;

	/* Array of handles to be pinned & unpinned */
	struct host1x_reloc *relocs;
	unsigned int num_relocs;
	struct host1x_job_unpin_data *unpins;
	unsigned int num_unpins;

	dma_addr_t *addr_phys;
	dma_addr_t *gather_addr_phys;
	dma_addr_t *reloc_addr_phys;

	/* Sync point id, number of increments and end related to the submit */
	struct host1x_syncpt *syncpt;
	u32 syncpt_incrs;
	u32 syncpt_end;

	/* Non-job tracking related syncpoint */
	struct host1x_syncpt *secondary_syncpt;

	/* Completion fence for job tracking */
	struct dma_fence *fence;
	struct dma_fence_cb fence_cb;
	struct completion fence_cb_done;

	/* Maximum time to wait for this job */
	unsigned int timeout;

	/* Job has timed out and should be released */
	bool cancelled;

	/* Index and number of slots used in the push buffer */
	unsigned int first_get;
	unsigned int num_slots;

	/* Copy of gathers */
	size_t gather_copy_size;
	dma_addr_t gather_copy;
	u8 *gather_copy_mapped;

	/* Check if register is marked as an address reg */
	int (*is_addr_reg)(struct device *dev, u32 class, u32 reg);

	/* Check if class belongs to the unit */
	int (*is_valid_class)(u32 class);

	/* Request a SETCLASS to this class */
	u32 class;

	/* Add a channel wait for previous ops to complete */
	bool serialize;

	/* Fast-forward syncpoint increments on job timeout */
	bool syncpt_recovery;

	/* Callback called when job is freed */
	void (*release)(struct host1x_job *job);
	void *user_data;

	/* Whether host1x-side firewall should be ran for this job or not */
	bool enable_firewall;

	/* Options for configuring engine data stream ID */
	/* Context device to use for job */
	struct host1x_memory_context *memory_context;
	/* Stream ID to use if context isolation is disabled (!memory_context) */
	u32 engine_fallback_streamid;
	/* Engine offset to program stream ID to */
	u32 engine_streamid_offset;
};

struct host1x_job *host1x_job_alloc(struct host1x_channel *ch,
				    u32 num_cmdbufs, u32 num_relocs,
				    bool skip_firewall);
void host1x_job_add_gather(struct host1x_job *job, struct host1x_bo *bo,
			   unsigned int words, unsigned int offset);
void host1x_job_add_wait(struct host1x_job *job, u32 id, u32 thresh,
			 bool relative, u32 next_class);
void host1x_job_add_reg_write(struct host1x_job *job, u32 reg, u32 value);
struct host1x_job *host1x_job_get(struct host1x_job *job);
void host1x_job_put(struct host1x_job *job);
int host1x_job_pin(struct host1x_job *job, struct device *dev);
void host1x_job_unpin(struct host1x_job *job);

/*
 * subdevice probe infrastructure
 */

struct host1x_device;

/**
 * struct host1x_driver - host1x logical device driver
 * @driver: core driver
 * @subdevs: table of OF device IDs matching subdevices for this driver
 * @list: list node for the driver
 * @probe: called when the host1x logical device is probed
 * @remove: called when the host1x logical device is removed
 * @shutdown: called when the host1x logical device is shut down
 */
struct host1x_driver {
	struct device_driver driver;

	const struct of_device_id *subdevs;
	struct list_head list;

	int (*probe)(struct host1x_device *device);
	int (*remove)(struct host1x_device *device);
	void (*shutdown)(struct host1x_device *device);
};

static inline struct host1x_driver *
to_host1x_driver(struct device_driver *driver)
{
	return container_of(driver, struct host1x_driver, driver);
}

int host1x_driver_register_full(struct host1x_driver *driver,
				struct module *owner);
void host1x_driver_unregister(struct host1x_driver *driver);

#define host1x_driver_register(driver) \
	host1x_driver_register_full(driver, THIS_MODULE)

struct host1x_device {
	struct host1x_driver *driver;
	struct list_head list;
	struct device dev;

	struct mutex subdevs_lock;
	struct list_head subdevs;
	struct list_head active;

	struct mutex clients_lock;
	struct list_head clients;

	bool registered;

	struct device_dma_parameters dma_parms;
};

static inline struct host1x_device *to_host1x_device(struct device *dev)
{
	return container_of(dev, struct host1x_device, dev);
}

int host1x_device_init(struct host1x_device *device);
int host1x_device_exit(struct host1x_device *device);

void __host1x_client_init(struct host1x_client *client, struct lock_class_key *key);
void host1x_client_exit(struct host1x_client *client);

#define host1x_client_init(client)			\
	({						\
		static struct lock_class_key __key;	\
		__host1x_client_init(client, &__key);	\
	})

int __host1x_client_register(struct host1x_client *client);

/*
 * Note that this wrapper calls __host1x_client_init() for compatibility
 * with existing callers. Callers that want to separately initialize and
 * register a host1x client must first initialize using either of the
 * __host1x_client_init() or host1x_client_init() functions and then use
 * the low-level __host1x_client_register() function to avoid the client
 * getting reinitialized.
 */
#define host1x_client_register(client)			\
	({						\
		static struct lock_class_key __key;	\
		__host1x_client_init(client, &__key);	\
		__host1x_client_register(client);	\
	})

void host1x_client_unregister(struct host1x_client *client);

int host1x_client_suspend(struct host1x_client *client);
int host1x_client_resume(struct host1x_client *client);

struct tegra_mipi_device;

struct tegra_mipi_device *tegra_mipi_request(struct device *device,
					     struct device_node *np);
void tegra_mipi_free(struct tegra_mipi_device *device);
int tegra_mipi_enable(struct tegra_mipi_device *device);
int tegra_mipi_disable(struct tegra_mipi_device *device);
int tegra_mipi_start_calibration(struct tegra_mipi_device *device);
int tegra_mipi_finish_calibration(struct tegra_mipi_device *device);

/* host1x memory contexts */

struct host1x_memory_context {
	struct host1x *host;

	struct device *dev; /* Owning engine */
	struct pid *pid;

	refcount_t ref;

	struct host1x_hw_memory_context *hw;
	struct device *context_dev; /* Context device */
	struct list_head entry; /* Entry in hw_memory_context's list */
	struct list_head mappings; /* List of mappings */
};

struct host1x_context_mapping {
	struct host1x *host;

	struct host1x_bo_mapping *mapping;

	struct host1x_bo *bo;
	enum dma_data_direction direction;

	struct list_head entry;
};

#ifdef CONFIG_IOMMU_API
struct host1x_memory_context *host1x_memory_context_alloc(struct host1x *host1x,
							  struct device *dev,
							  struct pid *pid);
void host1x_memory_context_get(struct host1x_memory_context *cd);
void host1x_memory_context_put(struct host1x_memory_context *cd);
int host1x_memory_context_active(struct host1x_memory_context *cd);
void host1x_memory_context_inactive(struct host1x_memory_context *cd);
struct host1x_context_mapping *host1x_memory_context_map(
	struct host1x_memory_context *ctx, struct host1x_bo *bo, enum dma_data_direction direction);
void host1x_memory_context_unmap(struct host1x_context_mapping *m);
#else
static inline struct host1x_memory_context *host1x_memory_context_alloc(struct host1x *host1x,
									struct pid *pid)
{
	return NULL;
}

static inline void host1x_memory_context_get(struct host1x_memory_context *cd)
{
}

static inline void host1x_memory_context_put(struct host1x_memory_context *cd)
{
}

static inline int host1x_memory_context_active(struct host1x_memory_context *cd)
{
	return -ENODEV;
}

static inline void host1x_memory_context_inactive(struct host1x_memory_context *cd)
{
}

static inline struct host1x_context_mapping *host1x_memory_context_map(
	struct host1x_memory_context *ctx, struct host1x_bo *bo, enum dma_data_direction direction)
{
	return ERR_PTR(-ENODEV);
}

static inline void host1x_memory_context_unmap(struct host1x_context_mapping *m)
{
}
#endif

int host1x_actmon_read_avg_count(struct host1x_client *client);
int host1x_actmon_register(struct host1x_client *client);
void host1x_actmon_unregister(struct host1x_client *client);
void host1x_actmon_enable(struct host1x_client *client);
void host1x_actmon_disable(struct host1x_client *client);
void host1x_actmon_update_client_rate(struct host1x_client *client,
				      unsigned long rate,
				      u32 *weight);
void host1x_actmon_read_active_norm(struct host1x_client *client, unsigned long *usage);
void host1x_actmon_update_active_wmark(struct host1x_client *client,
				       u32 avg_upper_wmark,
				       u32 avg_lower_wmark,
				       u32 consec_upper_wmark,
				       u32 consec_lower_wmark,
				       bool upper_wmark_enabled,
				       bool lower_wmark_enabled);

#endif
