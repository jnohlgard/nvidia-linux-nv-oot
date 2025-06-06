/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _OSDEP_SERVICE_LINUX_C_
#include <drv_types.h>

#ifdef DBG_MEMORY_LEAK
ATOMIC_T _malloc_cnt = ATOMIC_INIT(0);
ATOMIC_T _malloc_size = ATOMIC_INIT(0);
#endif /* DBG_MEMORY_LEAK */

/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
inline int RTW_STATUS_CODE(int error_code)
{
	if (error_code >= 0)
		return _SUCCESS;

	switch (error_code) {
	/* case -ETIMEDOUT: */
	/*	return RTW_STATUS_TIMEDOUT; */
	default:
		return _FAIL;
	}
}

void _rtw_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		_rtw_skb_free(skb);
}

#ifdef CONFIG_RTW_NEON_MODE
void _rtw_neon_memcpy(volatile void *dst, volatile const void *src, u32 sz)
{
    if (sz & 63) {
        sz = (sz & -64) + 64;
    }
    asm volatile (
        "NEONCopyPLD:                          \n"
        "    VLDM %[src]!,{d0-d7}                 \n"
        "    VSTM %[dst]!,{d0-d7}                 \n"
        "    SUBS %[sz],%[sz],#0x40                 \n"
        "    BGT NEONCopyPLD                  \n"
        : [dst]"+r"(dst), [src]"+r"(src), [sz]"+r"(sz) : : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
}
#endif

void _rtw_memcpy(void *dst, const void *src, u32 sz)
{
	memcpy(dst, src, sz);
}

inline void _rtw_memmove(void *dst, const void *src, u32 sz)
{
	memmove(dst, src, sz);
}

int _rtw_memcmp(const void *dst, const void *src, u32 sz)
{
	/* under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0 */
	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
}

void _rtw_memset(void *pbuf, int c, u32 sz)
{
	memset(pbuf, c, sz);
}

void _rtw_init_listhead(_list *list)
{
	INIT_LIST_HEAD(list);
}
/*
For the following list_xxx operations,
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32 rtw_is_list_empty(_list *phead)
{
	if (list_empty(phead))
		return _TRUE;
	else
		return _FALSE;
}

void rtw_list_insert_head(_list *plist, _list *phead)
{
	list_add(plist, phead);
}

void rtw_list_insert_tail(_list *plist, _list *phead)
{
	list_add_tail(plist, phead);
}

inline void rtw_list_splice(_list *list, _list *head)
{
	list_splice(list, head);
}

inline void rtw_list_splice_init(_list *list, _list *head)
{
	list_splice_init(list, head);
}

inline void rtw_list_splice_tail(_list *list, _list *head)
{
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27))
	if (!list_empty(list))
		__list_splice(list, head);
	#else
	list_splice_tail(list, head);
	#endif
}

inline void rtw_hlist_head_init(rtw_hlist_head *h)
{
	INIT_HLIST_HEAD(h);
}

inline void rtw_hlist_add_head(rtw_hlist_node *n, rtw_hlist_head *h)
{
	hlist_add_head(n, h);
}

inline void rtw_hlist_del(rtw_hlist_node *n)
{
	hlist_del(n);
}

inline void rtw_hlist_add_head_rcu(rtw_hlist_node *n, rtw_hlist_head *h)
{
	hlist_add_head_rcu(n, h);
}

inline void rtw_hlist_del_rcu(rtw_hlist_node *n)
{
	hlist_del_rcu(n);
}

void rtw_init_timer(_timer *ptimer, void *pfunc, void *ctx)
{
	_init_timer(ptimer, pfunc, ctx);
}

systime _rtw_get_current_time(void)
{
	return jiffies;
}

inline u32 _rtw_systime_to_ms(systime stime)
{
	return jiffies_to_msecs(stime);
}

inline u32 _rtw_systime_to_us(systime stime)
{
	return jiffies_to_usecs(stime);
}

inline systime _rtw_ms_to_systime(u32 ms)
{
	return msecs_to_jiffies(ms);
}

inline systime _rtw_us_to_systime(u32 us)
{
	return usecs_to_jiffies(us);
}

inline bool _rtw_time_after(systime a, systime b)
{
	return time_after(a, b);
}

inline bool _rtw_time_after_eq(systime a, systime b)
{
	return time_after_eq(a, b);
}

sysptime rtw_sptime_get(void)
{
	return ktime_get(); /* CLOCK_MONOTONIC */
}

sysptime rtw_sptime_get_raw(void)
{
	return ktime_get_raw(); /* CLOCK_MONOTONIC_RAW */
}

sysptime rtw_sptime_set(s64 secs, const u32 nsecs)
{
	return ktime_set(secs, nsecs);
}

sysptime rtw_sptime_zero(void)
{
	return ktime_set(0, 0);
}

/*
 *   cmp1  < cmp2: return <0
 *   cmp1 == cmp2: return 0
 *   cmp1  > cmp2: return >0
 */
int rtw_sptime_cmp(const sysptime cmp1, const sysptime cmp2)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
	return ktime_compare(cmp1, cmp2);
#else
	if (cmp1.tv64 < cmp2.tv64)
		return -1;
	if (cmp1.tv64 > cmp2.tv64)
		return 1;
	return 0;
#endif
}

bool rtw_sptime_eql(const sysptime cmp1, const sysptime cmp2)
{
	return rtw_sptime_cmp(cmp1, cmp2) == 0;
}

bool rtw_sptime_is_zero(const sysptime sptime)
{
	return rtw_sptime_cmp(sptime, rtw_sptime_zero()) == 0;
}

/*
 * sub = lhs - rhs, in normalized form
 */
sysptime rtw_sptime_sub(const sysptime lhs, const sysptime rhs)
{
	return ktime_sub(lhs, rhs);
}

/*
 * add = lhs + rhs, in normalized form
 */
sysptime rtw_sptime_add(const sysptime lhs, const sysptime rhs)
{
	return ktime_add(lhs, rhs);
}

s64 rtw_sptime_to_ms(const sysptime sptime)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	return ktime_to_ms(sptime);
#else
	struct timeval tv = ktime_to_timeval(sptime);

	return (s64) tv.tv_sec * MSEC_PER_SEC + tv.tv_usec / USEC_PER_MSEC;
#endif
}

sysptime rtw_ms_to_sptime(u64 ms)
{
	return ns_to_ktime(ms * NSEC_PER_MSEC);
}

s64 rtw_sptime_to_us(const sysptime sptime)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22))
	return ktime_to_us(sptime);
#else
	struct timeval tv = ktime_to_timeval(sptime);

	return (s64) tv.tv_sec * USEC_PER_SEC + tv.tv_usec;
#endif
}

sysptime rtw_us_to_sptime(u64 us)
{
	return ns_to_ktime(us * NSEC_PER_USEC);
}

s64 rtw_sptime_to_ns(const sysptime sptime)
{
	return ktime_to_ns(sptime);
}

sysptime rtw_ns_to_sptime(u64 ns)
{
	return ns_to_ktime(ns);
}

s64 rtw_sptime_diff_ms(const sysptime start, const sysptime end)
{
	sysptime diff;

	diff = rtw_sptime_sub(end, start);

	return rtw_sptime_to_ms(diff);
}

s64 rtw_sptime_pass_ms(const sysptime start)
{
	sysptime cur, diff;

	cur = rtw_sptime_get();
	diff = rtw_sptime_sub(cur, start);

	return rtw_sptime_to_ms(diff);
}

s64 rtw_sptime_diff_us(const sysptime start, const sysptime end)
{
	sysptime diff;

	diff = rtw_sptime_sub(end, start);

	return rtw_sptime_to_us(diff);
}

s64 rtw_sptime_pass_us(const sysptime start)
{
	sysptime cur, diff;

	cur = rtw_sptime_get();
	diff = rtw_sptime_sub(cur, start);

	return rtw_sptime_to_us(diff);
}

s64 rtw_sptime_diff_ns(const sysptime start, const sysptime end)
{
	sysptime diff;

	diff = rtw_sptime_sub(end, start);

	return rtw_sptime_to_ns(diff);
}

s64 rtw_sptime_pass_ns(const sysptime start)
{
	sysptime cur, diff;

	cur = rtw_sptime_get();
	diff = rtw_sptime_sub(cur, start);

	return rtw_sptime_to_ns(diff);
}

void rtw_sleep_schedulable(int ms)
{
	u32 delta;

	delta = (ms * HZ) / 1000; /* (ms) */
	if (delta == 0) {
		delta = 1;/* 1 ms */
	}
	set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(delta);
	return;
}

void rtw_msleep_os(int ms)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	if (ms < 20) {
		unsigned long us = ms * 1000UL;
		usleep_range(us, us + 1000UL);
	} else
#endif
		msleep((unsigned int)ms);

}
void rtw_usleep_os(int us)
{
	/* msleep((unsigned int)us); */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	usleep_range(us, us + 1);
#else
	if (1 < (us / 1000))
		msleep(1);
	else
		msleep((us / 1000) + 1);
#endif
}


#ifdef DBG_DELAY_OS
void _rtw_mdelay_os(int ms, const char *func, const int line)
{
	RTW_INFO("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);
	mdelay((unsigned long)ms);
}
void _rtw_udelay_os(int us, const char *func, const int line)
{
	RTW_INFO("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);
	udelay((unsigned long)us);
}
#else
void rtw_mdelay_os(int ms)
{
	mdelay((unsigned long)ms);
}
void rtw_udelay_os(int us)
{
	udelay((unsigned long)us);
}
#endif

void rtw_yield_os(void)
{
	yield();
}


#define RTW_SUSPEND_LOCK_NAME "rtw_wifi"
#define RTW_SUSPEND_TRAFFIC_LOCK_NAME "rtw_wifi_traffic"
#define RTW_SUSPEND_RESUME_LOCK_NAME "rtw_wifi_resume"

#ifdef CONFIG_WAKELOCK
static struct wake_lock rtw_suspend_lock;
static struct wake_lock rtw_suspend_traffic_lock;
static struct wake_lock rtw_suspend_resume_lock;
#elif defined(CONFIG_ANDROID_POWER)
static android_suspend_lock_t rtw_suspend_lock = {
	.name = RTW_SUSPEND_LOCK_NAME
};
static android_suspend_lock_t rtw_suspend_traffic_lock = {
	.name = RTW_SUSPEND_TRAFFIC_LOCK_NAME
};
static android_suspend_lock_t rtw_suspend_resume_lock = {
	.name = RTW_SUSPEND_RESUME_LOCK_NAME
};
#endif

inline void rtw_suspend_lock_init(void)
{
#ifdef CONFIG_WAKELOCK
	wake_lock_init(&rtw_suspend_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_LOCK_NAME);
	wake_lock_init(&rtw_suspend_traffic_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_TRAFFIC_LOCK_NAME);
	wake_lock_init(&rtw_suspend_resume_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_RESUME_LOCK_NAME);
#elif defined(CONFIG_ANDROID_POWER)
	android_init_suspend_lock(&rtw_suspend_lock);
	android_init_suspend_lock(&rtw_suspend_traffic_lock);
	android_init_suspend_lock(&rtw_suspend_resume_lock);
#endif
}

inline void rtw_suspend_lock_uninit(void)
{
#ifdef CONFIG_WAKELOCK
	wake_lock_destroy(&rtw_suspend_lock);
	wake_lock_destroy(&rtw_suspend_traffic_lock);
	wake_lock_destroy(&rtw_suspend_resume_lock);
#elif defined(CONFIG_ANDROID_POWER)
	android_uninit_suspend_lock(&rtw_suspend_lock);
	android_uninit_suspend_lock(&rtw_suspend_traffic_lock);
	android_uninit_suspend_lock(&rtw_suspend_resume_lock);
#endif
}

inline void rtw_lock_suspend(void)
{
#ifdef CONFIG_WAKELOCK
	wake_lock(&rtw_suspend_lock);
#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend(&rtw_suspend_lock);
#endif

#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	/* RTW_INFO("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count); */
#endif
}

inline void rtw_unlock_suspend(void)
{
#ifdef CONFIG_WAKELOCK
	wake_unlock(&rtw_suspend_lock);
#elif defined(CONFIG_ANDROID_POWER)
	android_unlock_suspend(&rtw_suspend_lock);
#endif

#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	/* RTW_INFO("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count); */
#endif
}

inline void rtw_resume_lock_suspend(void)
{
#ifdef CONFIG_WAKELOCK
	wake_lock(&rtw_suspend_resume_lock);
#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend(&rtw_suspend_resume_lock);
#endif

#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	/* RTW_INFO("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count); */
#endif
}

inline void rtw_resume_unlock_suspend(void)
{
#ifdef CONFIG_WAKELOCK
	wake_unlock(&rtw_suspend_resume_lock);
#elif defined(CONFIG_ANDROID_POWER)
	android_unlock_suspend(&rtw_suspend_resume_lock);
#endif

#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	/* RTW_INFO("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count); */
#endif
}

inline void rtw_lock_suspend_timeout(u32 timeout_ms)
{
#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_suspend_lock, rtw_ms_to_systime(timeout_ms));
#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_suspend_lock, rtw_ms_to_systime(timeout_ms));
#endif
}

inline void rtw_lock_traffic_suspend_timeout(u32 timeout_ms)
{
#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_suspend_traffic_lock, rtw_ms_to_systime(timeout_ms));
#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_suspend_traffic_lock, rtw_ms_to_systime(timeout_ms));
#endif
	/* RTW_INFO("traffic lock timeout:%d\n", timeout_ms); */
}

inline void rtw_set_bit(int nr, unsigned long *addr)
{
	set_bit(nr, addr);
}

inline void rtw_clear_bit(int nr, unsigned long *addr)
{
	clear_bit(nr, addr);
}

inline int rtw_test_and_clear_bit(int nr, unsigned long *addr)
{
	return test_and_clear_bit(nr, addr);
}
inline int rtw_test_and_set_bit(int nr, unsigned long *addr)
{
	return test_and_set_bit(nr, addr);
}

#if defined(CONFIG_RTW_ANDROID_GKI) && !defined(CONFIG_LOAD_FILE_BY_REQ_FW_API)
#define CONFIG_LOAD_FILE_BY_REQ_FW_API
#endif

#ifdef CONFIG_LOAD_FILE_BY_REQ_FW_API
#include <linux/firmware.h>

static const char *get_file_name_from_path(const char *path)
{
	char *ret;
	size_t path_len;

	if (!path)
		return NULL;

	path_len = strlen(path);
	if (path_len == 0)
		return NULL;

	ret = strrchr(path, '/');
	if (ret && ret - path < path_len)
		return ret + 1;
	return NULL;
}
#endif /* CONFIG_LOAD_FILE_BY_REQ_FW_API */

#if !defined(CONFIG_RTW_ANDROID_GKI)
/*
* Open a file with the specific @param path, @param flag, @param mode
* @param fpp the pointer of struct file pointer to get struct file pointer while file opening is success
* @param path the path of the file to open
* @param flag file operation flags, please refer to linux document
* @param mode please refer to linux document
* @return Linux specific error code
*/
static int openFile(struct file **fpp, const char *path, int flag, int mode)
{
	struct file *fp;

	fp = filp_open(path, flag, mode);
	if (IS_ERR(fp)) {
		*fpp = NULL;
		return PTR_ERR(fp);
	} else {
		*fpp = fp;
		return 0;
	}
}

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp)
{
	filp_close(fp, NULL);
	return 0;
}

static int readFile(struct file *fp, char *buf, int len)
{
	int rlen = 0, sum = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	if (!(fp->f_mode & FMODE_CAN_READ))
#else
	if (!fp->f_op || !fp->f_op->read)
#endif
		return -EPERM;

	while (sum < len) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		rlen = kernel_read(fp, buf + sum, len - sum, &fp->f_pos);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
		rlen = __vfs_read(fp, buf + sum, len - sum, &fp->f_pos);
#else
		rlen = fp->f_op->read(fp, buf + sum, len - sum, &fp->f_pos);
#endif
		if (rlen > 0)
			sum += rlen;
		else if (0 != rlen)
			return rlen;
		else
			break;
	}

	return  sum;

}

static int writeFile(struct file *fp, char *buf, int len)
{
	int wlen = 0, sum = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	if (!(fp->f_mode & FMODE_CAN_WRITE))
#else
	if (!fp->f_op || !fp->f_op->write)
#endif
		return -EPERM;

	while (sum < len) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		wlen = kernel_write(fp, buf + sum, len - sum, &fp->f_pos);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
		wlen = __vfs_write(fp, buf + sum, len - sum, &fp->f_pos);
#else
		wlen = fp->f_op->write(fp, buf + sum, len - sum, &fp->f_pos);
#endif
		if (wlen > 0)
			sum += wlen;
		else if (0 != wlen)
			return wlen;
		else
			break;
	}

	return sum;

}

/*
* Test if the specifi @param pathname is a direct and readable
* If readable, @param sz is not used
* @param pathname the name of the path to test
* @return Linux specific error code
*/
static int isDirReadable(const char *pathname, u32 *sz)
{
	struct path path;
	int error = 0;

	return kern_path(pathname, LOOKUP_FOLLOW, &path);
}
#endif /* !defined(CONFIG_RTW_ANDROID_GKI)*/

/*
* Test if the specifi @param path is a file and readable
* If readable, @param sz is got
* @param path the path of the file to test
* @return Linux specific error code
*/
static int isFileReadable(const char *path, u32 *sz)
{
#if defined(CONFIG_LOAD_FILE_BY_REQ_FW_API)
	int ret = -EINVAL;
	const struct firmware *fw = NULL;
	const char *name;

	if (path == NULL) {
		RTW_ERR("%s() NULL pointer\n", __func__);
		goto exit;
	}

	name = get_file_name_from_path(path);
	if (name == NULL) {
		RTW_ERR("%s() parsing file name fail\n", __func__);
		goto exit;
	}

	/* request_firmware() will find file in /vendor/firmware but not in path */
	ret = request_firmware(&fw, name, NULL);
	if (ret != 0) {
		RTW_ERR("%s() request_firmware file : %s, error : %d\n", __func__, name, ret);
		goto exit;
	}

	if (sz)
		*sz = (u32)fw->size;

exit:
	if (fw)
		release_firmware(fw);

	return ret;
#else /* !defined(CONFIG_LOAD_FILE_BY_REQ_FW_API) */
	struct file *fp;
	int ret = 0;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	mm_segment_t oldfs;
	#endif
	char buf;

	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(fp))
		ret = PTR_ERR(fp);
	else {
		#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		oldfs = get_fs();
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
		set_fs(KERNEL_DS);
		#else
		set_fs(get_ds());
		#endif
		#endif

		if (1 != readFile(fp, &buf, 1))
			ret = PTR_ERR(fp);

		if (ret == 0 && sz) {
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
			*sz = i_size_read(fp->f_path.dentry->d_inode);
			#else
			*sz = i_size_read(fp->f_dentry->d_inode);
			#endif
		}

		#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		set_fs(oldfs);
		#endif
		filp_close(fp, NULL);
	}
	return ret;
#endif /* defined(CONFIG_LOAD_FILE_BY_REQ_FW_API) */
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(const char *path, u8 *buf, u32 sz)
{
#if defined(CONFIG_LOAD_FILE_BY_REQ_FW_API)
	int ret = -EINVAL;
	const struct firmware *fw = NULL;
	const char *name;

	if (path == NULL || buf == NULL) {
		RTW_ERR("%s() NULL pointer\n", __func__);
		goto err;
	}

	name = get_file_name_from_path(path);
	if (name == NULL) {
		RTW_ERR("%s() parsing file name fail\n", __func__);
		goto err;
	}

	/* request_firmware() will find file in /vendor/firmware but not in path */
	ret = request_firmware(&fw, name, NULL);
	if (ret == 0) {
		RTW_INFO("%s() Success. retrieve file : %s, file size : %zu\n", __func__, name, fw->size);

		if ((u32)fw->size <= sz) {
			_rtw_memcpy(buf, fw->data, (u32)fw->size);
			ret = (u32)fw->size;
			goto exit;
		} else {
			RTW_ERR("%s() file size : %zu exceed buf size : %u\n", __func__, fw->size, sz);
			ret = -EFBIG;
			goto err;
		}
	} else {
		RTW_ERR("%s() Fail. retrieve file : %s, error : %d\n", __func__, name, ret);
		goto err;
	}



err:
	RTW_ERR("%s() Fail. retrieve file : %s, error : %d\n", __func__, path, ret);
exit:
	if (fw)
		release_firmware(fw);
	return ret;
#else /* !defined(CONFIG_LOAD_FILE_BY_REQ_FW_API) */
	int ret = -1;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	mm_segment_t oldfs;
	#endif
	struct file *fp;

	if (path && buf) {
		ret = openFile(&fp, path, O_RDONLY, 0);
		if (0 == ret) {
			RTW_INFO("%s openFile path:%s fp=%p\n", __FUNCTION__, path , fp);

			#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
			oldfs = get_fs();
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
			set_fs(KERNEL_DS);
			#else
			set_fs(get_ds());
			#endif
			#endif

			ret = readFile(fp, buf, sz);

			#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
			set_fs(oldfs);
			#endif
			closeFile(fp);

			RTW_INFO("%s readFile, ret:%d\n", __FUNCTION__, ret);

		} else
			RTW_INFO("%s openFile path:%s Fail, ret:%d\n", __FUNCTION__, path, ret);
	} else {
		RTW_INFO("%s NULL pointer\n", __FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
#endif /* defined(CONFIG_LOAD_FILE_BY_REQ_FW_API) */
}

#if !defined(CONFIG_RTW_ANDROID_GKI)
/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(const char *path, u8 *buf, u32 sz)
{
	int ret = 0;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	mm_segment_t oldfs;
	#endif
	struct file *fp;

	if (path && buf) {
		ret = openFile(&fp, path, O_CREAT | O_WRONLY, 0666);
		if (0 == ret) {
			RTW_INFO("%s openFile path:%s fp=%p\n", __FUNCTION__, path , fp);

			#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
			oldfs = get_fs();
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
			set_fs(KERNEL_DS);
			#else
			set_fs(get_ds());
			#endif
			#endif

			ret = writeFile(fp, buf, sz);

			#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
			set_fs(oldfs);
			#endif
			closeFile(fp);

			RTW_INFO("%s writeFile, ret:%d\n", __FUNCTION__, ret);

		} else
			RTW_INFO("%s openFile path:%s Fail, ret:%d\n", __FUNCTION__, path, ret);
	} else {
		RTW_INFO("%s NULL pointer\n", __FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Test if the specifi @param path is a direct and readable
* @param path the path of the direct to test
* @return _TRUE or _FALSE
*/
int rtw_is_dir_readable(const char *path)
{
	if (isDirReadable(path, NULL) == 0)
		return _TRUE;
	else
		return _FALSE;
}
#endif /* !defined(CONFIG_RTW_ANDROID_GKI)*/

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable(const char *path)
{
	if (isFileReadable(path, NULL) == 0)
		return _TRUE;
	else
		return _FALSE;
}

/*
* Test if the specifi @param path is a file and readable.
* If readable, @param sz is got
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable_with_size(const char *path, u32 *sz)
{
	if (isFileReadable(path, sz) == 0)
		return _TRUE;
	else
		return _FALSE;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrieve_from_file(const char *path, u8 *buf, u32 sz)
{
	int ret = retriveFromFile(path, buf, sz);
	return ret >= 0 ? ret : 0;
}

#if !defined(CONFIG_RTW_ANDROID_GKI)
/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(const char *path, u8 *buf, u32 sz)
{
	int ret = storeToFile(path, buf, sz);
	return ret >= 0 ? ret : 0;
}
#endif /* !defined(CONFIG_RTW_ANDROID_GKI) */

struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 5);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);
	pnpi->priv = old_priv;
	pnpi->sizeof_priv = sizeof_priv;

RETURN:
	return pnetdev;
}

struct net_device *rtw_alloc_etherdev(int sizeof_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);

	pnpi->priv = rtw_zvmalloc(sizeof_priv);
	if (!pnpi->priv) {
		free_netdev(pnetdev);
		pnetdev = NULL;
		goto RETURN;
	}

	pnpi->sizeof_priv = sizeof_priv;
RETURN:
	return pnetdev;
}

void rtw_free_netdev(struct net_device *netdev)
{
	struct rtw_netdev_priv_indicator *pnpi;

	if (!netdev)
		goto RETURN;

	pnpi = netdev_priv(netdev);

	if (!pnpi->priv)
		goto RETURN;

	free_netdev(netdev);

RETURN:
	return;
}

#ifdef CONFIG_PLATFORM_SPRD
#ifdef do_div
	#undef do_div
#endif
	#include <asm-generic/div64.h>
#endif

u64 rtw_modular64(u64 x, u64 y)
{
	return do_div(x, y);
}

u64 rtw_division64(u64 x, u64 y)
{
	do_div(x, y);
	return x;
}

inline u32 rtw_random32(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
	return get_random_u32();
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
	return prandom_u32();
#elif (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18))
	u32 random_int;
	get_random_bytes(&random_int , 4);
	return random_int;
#else
	return random32();
#endif
}
void rtw_wiphy_rfkill_set_hw_state(struct wiphy *wiphy, bool blocked)
{
	wiphy_rfkill_set_hw_state(wiphy, blocked);
}

ATOMIC_T rtw_warn_on_cnt;

