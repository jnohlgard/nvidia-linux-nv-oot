/*
*
*  Realtek Bluetooth USB driver
*
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/dcache.h>
#include <linux/version.h>
#include <net/sock.h>
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include <net/bluetooth/l2cap.h>

#include "rtk_coex.h"

#define DIRECT_A2DP_MEDIA_PACKET_PROCESSING

#define HCI_CONN_HANDLE_UNSET_START	(0x0eff + 1)
#define HCI_CONN_TYPE_BREDR_ACL		0x00
#define HCI_CONN_TYPE_SCO		0x01
#define HCI_CONN_TYPE_LE_ACL		0x02
#define HCI_CONN_TYPE_CIS		0x04
#define HCI_CONN_TYPE_BIS		0x05

/* Software coex message can be sent to and receive from WiFi driver by
 * UDP socket or exported symbol */
/* #define RTK_COEX_OVER_SYMBOL */

#if BTRTL_HCI_IF == BTRTL_HCIUSB
#include <linux/usb.h>
#include "rtk_bt.h"
#undef RTKBT_DBG
#undef RTKBT_INFO
#undef RTKBT_WARN
#undef RTKBT_ERR

#elif BTRTL_HCI_IF == BTRTL_HCIUART
/* #define HCI_VERSION_CODE KERNEL_VERSION(3, 14, 41) */
#define HCI_VERSION_CODE LINUX_VERSION_CODE

#else
#error "Please set type of HCI interface"
#endif

#define RTK_VERSION "1.2"

#define RTKBT_DBG(fmt, arg...) printk(KERN_DEBUG "rtk_btcoex: " fmt "\n" , ## arg)
#define RTKBT_INFO(fmt, arg...) printk(KERN_INFO "rtk_btcoex: " fmt "\n" , ## arg)
#define RTKBT_WARN(fmt, arg...) printk(KERN_WARNING "rtk_btcoex: " fmt "\n", ## arg)
#define RTKBT_ERR(fmt, arg...) printk(KERN_ERR "rtk_btcoex: " fmt "\n", ## arg)

#ifndef HCI_OP_LE_REMOVE_ISO_PATH
#define HCI_OP_LE_REMOVE_ISO_PATH	0x206f
struct hci_cp_le_remove_iso_path {
	u16 handle;
	u8  direction;
} __attribute__((packed));
#endif
#define HCI_DATA_PATH_INPUT	(1 << 0) /* Host to Controller */
#define HCI_DATA_PATH_OUTPUT	(1 << 1) /* Controller to Host */

static struct rtl_coex_struct btrtl_coex;

#ifdef RTB_SOFTWARE_MAILBOX
#ifdef RTK_COEX_OVER_SYMBOL
static struct sk_buff_head rtw_q;
static struct workqueue_struct *rtw_wq;
static struct work_struct rtw_work;
static u8 rtw_coex_on;
#endif
#endif

#define is_profile_connected(conn, profile)   ((conn->profile_bitmap & BIT(profile)) > 0)
#define is_profile_busy(conn, profile)        ((conn->profile_status & BIT(profile)) > 0)

#ifdef RTB_SOFTWARE_MAILBOX
static void rtk_handle_event_from_wifi(uint8_t * msg);
#endif

static void count_a2dp_packet_timeout(struct work_struct *work);
static void count_pan_packet_timeout(struct work_struct *work);
static void count_hogp_packet_timeout(struct work_struct *work);

static int rtl_alloc_buff(struct rtl_coex_struct *coex)
{
	struct rtl_hci_ev *ev;
	struct rtl_l2_buff *l2;
	int i;
	int order;
	unsigned long addr;
	unsigned long addr2;
	int ev_size;
	int l2_size;
	int n;

	spin_lock_init(&coex->buff_lock);

	INIT_LIST_HEAD(&coex->ev_free_list);
	INIT_LIST_HEAD(&coex->l2_free_list);
	INIT_LIST_HEAD(&coex->hci_pkt_list);

	n = NUM_RTL_HCI_EV * sizeof(struct rtl_hci_ev);
	ev_size = ALIGN(n, sizeof(unsigned long));

	n = L2_MAX_PKTS * sizeof(struct rtl_l2_buff);
	l2_size = ALIGN(n, sizeof(unsigned long));

	RTKBT_DBG("alloc buffers %d, %d for ev and l2", ev_size, l2_size);

	order = get_order(ev_size + l2_size);
	addr = __get_free_pages(GFP_KERNEL, order);
	if (!addr) {
		RTKBT_ERR("failed to alloc buffers for ev and l2.");
		return -ENOMEM;
	}
	memset((void *)addr, 0, ev_size + l2_size);

	coex->pages_addr = addr;
	coex->buff_size = ev_size + l2_size;

	ev = (struct rtl_hci_ev *)addr;
	for (i = 0; i < NUM_RTL_HCI_EV; i++) {
		list_add_tail(&ev->list, &coex->ev_free_list);
		ev++;
	}

	addr2 = addr + ev_size;
	l2 = (struct rtl_l2_buff *)addr2;
	for (i = 0; i < L2_MAX_PKTS; i++) {
		list_add_tail(&l2->list, &coex->l2_free_list);
		l2++;
	}

	return 0;
}

static void rtl_free_buff(struct rtl_coex_struct *coex)
{
	struct rtl_hci_ev *ev;
	struct rtl_l2_buff *l2;
	struct rtl_hci_hdr *hdr;
	unsigned long flags;

	spin_lock_irqsave(&coex->buff_lock, flags);

	while (!list_empty(&coex->hci_pkt_list)) {
		hdr = list_entry(coex->hci_pkt_list.next, struct rtl_hci_hdr,
				 list);
		list_del(&hdr->list);
	}

	while (!list_empty(&coex->ev_free_list)) {
		ev = list_entry(coex->ev_free_list.next, struct rtl_hci_ev,
				list);
		list_del(&ev->list);
	}

	while (!list_empty(&coex->l2_free_list)) {
		l2 = list_entry(coex->l2_free_list.next, struct rtl_l2_buff,
				list);
		list_del(&l2->list);
	}

	spin_unlock_irqrestore(&coex->buff_lock, flags);

	if (coex->buff_size > 0) {
		free_pages(coex->pages_addr, get_order(coex->buff_size));
		coex->pages_addr = 0;
		coex->buff_size = 0;
	}
}

static struct rtl_hci_ev *rtl_ev_node_get(struct rtl_coex_struct *coex)
{
	struct rtl_hci_ev *ev;
	unsigned long flags;

	if (!coex->buff_size)
		return NULL;

	spin_lock_irqsave(&coex->buff_lock, flags);
	if (!list_empty(&coex->ev_free_list)) {
		ev = list_entry(coex->ev_free_list.next, struct rtl_hci_ev,
				list);
		list_del(&ev->list);
	} else
		ev = NULL;
	spin_unlock_irqrestore(&coex->buff_lock, flags);
	return ev;
}

static int rtl_ev_node_to_used(struct rtl_coex_struct *coex,
		struct rtl_hci_ev *ev)
{
	unsigned long flags;

	ev->type = HCI_PT_EVT;
	spin_lock_irqsave(&coex->buff_lock, flags);
	list_add_tail(&ev->list, &coex->hci_pkt_list);
	spin_unlock_irqrestore(&coex->buff_lock, flags);

	return 0;
}

static struct rtl_l2_buff *rtl_l2_node_get(struct rtl_coex_struct *coex)
{
	struct rtl_l2_buff *l2;
	unsigned long flags;

	if (!coex->buff_size)
		return NULL;

	spin_lock_irqsave(&coex->buff_lock, flags);

	if(!list_empty(&coex->l2_free_list)) {
		l2 = list_entry(coex->l2_free_list.next, struct rtl_l2_buff,
				list);
		list_del(&l2->list);
	} else
		l2 = NULL;

	spin_unlock_irqrestore(&coex->buff_lock, flags);
	return l2;
}

static int rtl_l2_node_to_used(struct rtl_coex_struct *coex,
		struct rtl_l2_buff *l2)
{
	unsigned long flags;

	if (l2->out)
		l2->type = HCI_PT_L2SIG_TX;
	else
		l2->type = HCI_PT_L2SIG_RX;
	spin_lock_irqsave(&coex->buff_lock, flags);
	list_add_tail(&l2->list, &coex->hci_pkt_list);
	spin_unlock_irqrestore(&coex->buff_lock, flags);

	return 0;
}

static uint8_t psm_to_profile_index(uint16_t psm)
{
	switch (psm) {
	case PSM_AVCTP:
	case PSM_SDP:
		return 0xFF;	//ignore

	case PSM_HID:
	case PSM_HID_INT:
		return profile_hid;

	case PSM_AVDTP:
		return profile_a2dp;

	case PSM_PAN:
	case PSM_OPP:
	case PSM_FTP:
	case PSM_BIP:
	case PSM_RFCOMM:
		return profile_pan;

	default:
		return profile_pan;
	}
}

static rtk_prof_info *find_by_psm(u16 handle, u16 psm)
{
	struct list_head *head = &btrtl_coex.profile_list;
	struct list_head *iter = NULL;
	struct list_head *temp = NULL;
	rtk_prof_info *desc = NULL;

	list_for_each_safe(iter, temp, head) {
		desc = list_entry(iter, rtk_prof_info, list);
		if ((handle & 0xfff) == (desc->handle & 0xfff) &&
                    desc->psm == psm)
			return desc;
	}

	return NULL;
}

static void rtk_check_setup_timer(rtk_conn_prof * phci_conn, uint8_t profile_index)
{
	int delay = msecs_to_jiffies(1000);
	if (profile_index == profile_a2dp) {
		phci_conn->a2dp_packet_count = 0;
		queue_delayed_work(btrtl_coex.fw_wq,
				   &phci_conn->a2dp_count_work, delay);
	}

	if (profile_index == profile_pan) {
		phci_conn->pan_packet_count = 0;
		queue_delayed_work(btrtl_coex.fw_wq,
				   &phci_conn->pan_count_work, delay);
	}

	/* hogp & voice share one timer now */
	if ((profile_index == profile_hogp) || (profile_index == profile_voice)) {
		if ((0 == phci_conn->profile_refcount[profile_hogp])
		    && (0 == phci_conn->profile_refcount[profile_voice])) {
			phci_conn->hogp_packet_count = 0;
			phci_conn->voice_packet_count = 0;
			queue_delayed_work(btrtl_coex.fw_wq,
					   &phci_conn->hogp_count_work, delay);
		}
	}
}

static void rtk_check_del_timer(uint8_t profile_index, rtk_conn_prof * phci_conn)
{
	RTKBT_DBG("%s: handle 0x%04x", __func__, phci_conn->handle);
	if (profile_a2dp == profile_index) {
		phci_conn->a2dp_packet_count = 0;
		cancel_delayed_work_sync(&phci_conn->a2dp_count_work);
	}
	if (profile_pan == profile_index) {
		phci_conn->pan_packet_count = 0;
		cancel_delayed_work_sync(&phci_conn->pan_count_work);
	}
	if (profile_hogp == profile_index) {
		phci_conn->hogp_packet_count = 0;
		if (phci_conn->profile_refcount[profile_voice] == 0) {
			cancel_delayed_work_sync(&phci_conn->hogp_count_work);
		}
	}
	if (profile_voice == profile_index) {
		phci_conn->voice_packet_count = 0;
		if (phci_conn->profile_refcount[profile_hogp] == 0) {
			cancel_delayed_work_sync(&phci_conn->hogp_count_work);
		}
	}
}



static rtk_conn_prof *find_connection_by_handle(struct rtl_coex_struct * coex,
						uint16_t handle)
{
	struct list_head *head = &coex->conn_hash;
	struct list_head *iter = NULL, *temp = NULL;
	rtk_conn_prof *desc = NULL;

	list_for_each_safe(iter, temp, head) {
		desc = list_entry(iter, rtk_conn_prof, list);
		if ((handle & 0xFFF) == desc->handle) {
			return desc;
		}
	}
	return NULL;
}

static rtk_conn_prof *allocate_connection_by_handle(uint16_t handle)
{
	rtk_conn_prof *phci_conn = NULL;
	phci_conn = kzalloc(sizeof(rtk_conn_prof), GFP_ATOMIC);
	if (phci_conn) {
		phci_conn->handle = handle;
		phci_conn->direction = 0;
	}

	return phci_conn;
}

static void init_connection_hash(struct rtl_coex_struct * coex)
{
	struct list_head *head = &coex->conn_hash;
	INIT_LIST_HEAD(head);
}

static void add_connection_to_hash(struct rtl_coex_struct * coex,
				   rtk_conn_prof * desc)
{
	struct list_head *head = &coex->conn_hash;
	list_add_tail(&desc->list, head);
	INIT_DELAYED_WORK(&desc->a2dp_count_work,
			  count_a2dp_packet_timeout);
	INIT_DELAYED_WORK(&desc->pan_count_work,
			  count_pan_packet_timeout);
	INIT_DELAYED_WORK(&desc->hogp_count_work,
			  count_hogp_packet_timeout);
}

static void delete_connection_from_hash(rtk_conn_prof *desc)
{
	if (desc) {
		cancel_delayed_work_sync(&desc->a2dp_count_work);
		cancel_delayed_work_sync(&desc->pan_count_work);
		cancel_delayed_work_sync(&desc->hogp_count_work);

		set_bit(RTL_COEX_CONN_REMOVING, &btrtl_coex.flags);
		while (test_bit(RTL_COEX_PKT_COUNTING, &btrtl_coex.flags)) {
			RTKBT_INFO("%s: counting packets", __func__);
			schedule();
		}
		mutex_lock(&btrtl_coex.conn_mutex);
		list_del(&desc->list);
		mutex_unlock(&btrtl_coex.conn_mutex);
		clear_bit(RTL_COEX_CONN_REMOVING, &btrtl_coex.flags);

		kfree(desc);
	}
}

static void flush_connection_hash(struct rtl_coex_struct *coex)
{
	struct list_head *head = &coex->conn_hash;
	struct list_head *iter = NULL, *temp = NULL;
	rtk_conn_prof *desc = NULL;

	list_for_each_safe(iter, temp, head) {
		desc = list_entry(iter, rtk_conn_prof, list);
		if (desc) {
			cancel_delayed_work_sync(&desc->a2dp_count_work);
			cancel_delayed_work_sync(&desc->pan_count_work);
			cancel_delayed_work_sync(&desc->hogp_count_work);
			delete_connection_from_hash(desc);
		}
	}
	//INIT_LIST_HEAD(head);
}

static void init_profile_hash(struct rtl_coex_struct * coex)
{
	struct list_head *head = &coex->profile_list;
	INIT_LIST_HEAD(head);
}

static uint8_t list_allocate_add(uint16_t handle, uint16_t psm,
				 uint8_t profile_index, uint16_t dcid,
				 uint16_t scid)
{
	rtk_prof_info *pprof_info = NULL;

	pprof_info = kmalloc(sizeof(rtk_prof_info), GFP_ATOMIC);
	if (NULL == pprof_info) {
		RTKBT_ERR("list_allocate_add: allocate error");
		return FALSE;
	}

	/* Check if it is the second l2cap connection for a2dp
	 * a2dp signal channel will be created first than media channel.
	 */
	if (psm == PSM_AVDTP) {
		rtk_prof_info *pinfo = find_by_psm(handle, psm);
		if (!pinfo) {
			pprof_info->flags = A2DP_SIGNAL;
			RTKBT_INFO("%s: Add a2dp signal channel", __func__);
		} else {
			pprof_info->flags = A2DP_MEDIA;
			RTKBT_INFO("%s: Add a2dp media channel", __func__);
		}
	}

	pprof_info->handle = handle;
	pprof_info->psm = psm;
	pprof_info->scid = scid;
	pprof_info->dcid = dcid;
	pprof_info->profile_index = profile_index;
	list_add_tail(&(pprof_info->list), &(btrtl_coex.profile_list));

	return TRUE;
}

static void delete_profile_from_hash(rtk_prof_info *desc)
{
	if (desc) {
		RTKBT_DBG("Delete profile: hndl 0x%04x, psm 0x%04x, dcid 0x%04x, "
			"scid 0x%04x", desc->handle, desc->psm, desc->dcid,
			desc->scid);

		set_bit(RTL_COEX_CONN_REMOVING, &btrtl_coex.flags);
		while (test_bit(RTL_COEX_PKT_COUNTING, &btrtl_coex.flags)) {
			RTKBT_INFO("%s: counting packets", __func__);
			schedule();
		}
		list_del(&desc->list);
		clear_bit(RTL_COEX_CONN_REMOVING, &btrtl_coex.flags);

		kfree(desc);
	}
}

static void flush_profile_hash(struct rtl_coex_struct *coex)
{
	struct list_head *head = &coex->profile_list;
	struct list_head *iter = NULL, *temp = NULL;
	rtk_prof_info *desc = NULL;

	mutex_lock(&btrtl_coex.profile_mutex);
	list_for_each_safe(iter, temp, head) {
		desc = list_entry(iter, rtk_prof_info, list);
		if (desc) {
			RTKBT_DBG("Delete profile: hndl 0x%04x, psm 0x%04x, "
				"dcid 0x%04x, scid 0x%04x", desc->handle,
				desc->psm, desc->dcid, desc->scid);

			delete_profile_from_hash(desc);
		}
	}
	//INIT_LIST_HEAD(head);
	mutex_unlock(&btrtl_coex.profile_mutex);
}

static rtk_prof_info *find_profile_by_handle_scid(struct rtl_coex_struct *
						  coex, uint16_t handle,
						  uint16_t scid)
{
	struct list_head *head = &coex->profile_list;
	struct list_head *iter = NULL, *temp = NULL;
	rtk_prof_info *desc = NULL;

	list_for_each_safe(iter, temp, head) {
		desc = list_entry(iter, rtk_prof_info, list);
		if (((handle & 0xFFF) == desc->handle) && (scid == desc->scid)) {
			return desc;
		}
	}
	return NULL;
}

static rtk_prof_info *find_profile_by_handle_dcid(struct rtl_coex_struct *
						  coex, uint16_t handle,
						  uint16_t dcid)
{
	struct list_head *head = &coex->profile_list;
	struct list_head *iter = NULL, *temp = NULL;
	rtk_prof_info *desc = NULL;

	list_for_each_safe(iter, temp, head) {
		desc = list_entry(iter, rtk_prof_info, list);
		if (((handle & 0xFFF) == desc->handle) && (dcid == desc->dcid)) {
			return desc;
		}
	}
	return NULL;
}

static rtk_prof_info *find_profile_by_handle_dcid_scid(struct rtl_coex_struct
						       * coex, uint16_t handle,
						       uint16_t dcid,
						       uint16_t scid)
{
	struct list_head *head = &coex->profile_list;
	struct list_head *iter = NULL, *temp = NULL;
	rtk_prof_info *desc = NULL;

	list_for_each_safe(iter, temp, head) {
		desc = list_entry(iter, rtk_prof_info, list);
		if (((handle & 0xFFF) == desc->handle) && (dcid == desc->dcid)
		    && (scid == desc->scid)) {
			return desc;
		}
	}
	return NULL;
}

static void rtk_vendor_cmd_to_fw(uint16_t opcode, uint8_t parameter_len,
				 uint8_t * parameter)
{
	int len = HCI_CMD_PREAMBLE_SIZE + parameter_len;
	uint8_t *p;
	struct sk_buff *skb;
	struct hci_dev *hdev = btrtl_coex.hdev;

	if (!hdev) {
		RTKBT_ERR("No HCI device");
		return;
	} else if (!test_bit(HCI_UP, &hdev->flags)) {
		RTKBT_WARN("HCI device is down");
		return;
	}

	skb = bt_skb_alloc(len, GFP_ATOMIC);
	if (!skb) {
		RTKBT_DBG("there is no room for cmd 0x%x", opcode);
		return;
	}

	p = (uint8_t *) skb_put(skb, HCI_CMD_PREAMBLE_SIZE);
	UINT16_TO_STREAM(p, opcode);
	*p++ = parameter_len;

	if (parameter_len)
		memcpy(skb_put(skb, parameter_len), parameter, parameter_len);

	bt_cb(skb)->pkt_type = HCI_COMMAND_PKT;

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	bt_cb(skb)->opcode = opcode;
#else
	bt_cb(skb)->hci.opcode = opcode;
#endif
#endif

	/* Stand-alone HCI commands must be flagged as
	 * single-command requests.
	 */
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	bt_cb(skb)->req.start = true;
#else

#if HCI_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
	bt_cb(skb)->hci.req_start = true;
#else

	bt_cb(skb)->hci.req_flags |= HCI_REQ_START;
#endif

#endif /* 4.4.0 */
#endif /* 3.10.0 */
	RTKBT_DBG("%s: opcode 0x%x", __func__, opcode);

	/* It is harmless if set skb->dev twice. The dev will be used in
	 * btusb_send_frame() after or equal to kernel/hci 3.13.0,
	 * the hdev will not come from skb->dev. */
#if HCI_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
	skb->dev = (void *)btrtl_coex.hdev;
#endif
	/* Put the skb to the global hdev->cmd_q */
	skb_queue_tail(&hdev->cmd_q, skb);

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
	tasklet_schedule(&hdev->cmd_task);
#else
	queue_work(hdev->workqueue, &hdev->cmd_work);
#endif

	return;
}

static uint8_t profileinfo_cmd = 0;
static void rtk_notify_profileinfo_to_fw(void)
{
	struct list_head *head = NULL;
	struct list_head *iter = NULL;
	struct list_head *temp = NULL;
	rtk_conn_prof *hci_conn = NULL;
	uint8_t handle_number = 0;
	uint32_t buffer_size = 0;
	uint8_t *p_buf = NULL;
	uint8_t *p = NULL;

	head = &btrtl_coex.conn_hash;
	list_for_each_safe(iter, temp, head) {
		hci_conn = list_entry(iter, rtk_conn_prof, list);
		if (hci_conn && hci_conn->profile_bitmap)
			handle_number++;
	}

	if(!profileinfo_cmd) {
		buffer_size = 1 + handle_number * 3 + 1;
	} else {
		buffer_size = 1 + handle_number * 6;
	}

	p_buf = kmalloc(buffer_size, GFP_ATOMIC);

	if (NULL == p_buf) {
		RTKBT_ERR("%s: alloc error", __func__);
		return;
	}
	p = p_buf;
	*p++ = handle_number;

	RTKBT_DBG("%s: BufferSize %u", __func__, buffer_size);
	RTKBT_DBG("%s: NumberOfHandles %u", __func__, handle_number);
	head = &btrtl_coex.conn_hash;
	list_for_each(iter, head) {
		hci_conn = list_entry(iter, rtk_conn_prof, list);
		if (hci_conn && hci_conn->profile_bitmap) {
			if(!profileinfo_cmd) {
				UINT16_TO_STREAM(p, hci_conn->handle);
				RTKBT_DBG("%s: handle 0x%04x", __func__,
						hci_conn->handle);
				*p++ = hci_conn->profile_bitmap;
				btrtl_coex.profile_status |= hci_conn->profile_status;
			} else {
				UINT16_TO_STREAM(p, hci_conn->handle);
				UINT16_TO_STREAM(p, hci_conn->profile_bitmap);
				UINT16_TO_STREAM(p, hci_conn->profile_status);
				RTKBT_DBG("%s: profile_status 0x%04x", __func__,
						hci_conn->profile_status);
			}
			RTKBT_DBG("%s: profile_bitmap 0x%04x", __func__,
							hci_conn->profile_bitmap);
			handle_number--;
		}
		if (0 == handle_number)
			break;
	}

	if(!profileinfo_cmd) {
		*p++ = btrtl_coex.profile_status;
		btrtl_coex.profile_status = 0;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_SET_PROFILE_REPORT_LEGACY_COMMAND, buffer_size,
			     p_buf);
	} else {
		rtk_vendor_cmd_to_fw(HCI_VENDOR_SET_PROFILE_REPORT_COMMAND, buffer_size,
				p_buf);
	}

	kfree(p_buf);
	return;
}

static void update_profile_state(rtk_conn_prof * phci_conn,
		uint8_t profile_index, uint8_t is_busy)
{
	uint8_t need_update = FALSE;

	RTKBT_DBG("%s: is_busy %d, profile_index %x", __func__,
			is_busy, profile_index);
	if ((phci_conn->profile_bitmap & BIT(profile_index)) == 0) {
		RTKBT_ERR("%s: : ERROR!!! profile(Index: %x) does not exist",
				__func__, profile_index);
		return;
	}


	if (is_busy) {
		if ((phci_conn->profile_status & BIT(profile_index)) == 0) {
			need_update = TRUE;
			phci_conn->profile_status |= BIT(profile_index);

			if(profile_index == profile_sink)
				phci_conn->profile_status |= BIT(profile_a2dpsink);
		}
	} else {
		if ((phci_conn->profile_status & BIT(profile_index)) > 0) {
			need_update = TRUE;
			phci_conn->profile_status &= ~(BIT(profile_index));

			if(profile_index == profile_sink)
				phci_conn->profile_status &= ~(BIT(profile_a2dpsink));
		}
	}

	if (need_update) {
		RTKBT_DBG("%s: phci_conn->profile_status 0x%02x",
				__func__, phci_conn->profile_status);
		rtk_notify_profileinfo_to_fw();
	}
}

static void update_profile_connection(rtk_conn_prof * phci_conn,
				      uint8_t profile_index, uint8_t is_add)
{
	uint8_t need_update = FALSE;

	RTKBT_DBG("%s: is_add %d, profile_index %d", __func__, is_add,
		  profile_index);

	if (is_add) {

		if (0 == phci_conn->profile_refcount[profile_index]) {
			need_update = TRUE;
			phci_conn->profile_bitmap |= BIT(profile_index);
			/* SCO is always busy */
			if (profile_index == profile_sco)
				phci_conn->profile_status |=
				    BIT(profile_index);

			if (profile_index == profile_sink)
				phci_conn->profile_bitmap |= BIT(profile_a2dpsink);

			rtk_check_setup_timer(phci_conn, profile_index);
		}
		phci_conn->profile_refcount[profile_index]++;
	} else {
		if (!phci_conn->profile_refcount[profile_index]) {
			RTKBT_WARN("profile %u refcount is already zero",
				   profile_index);
			return;
		}

		phci_conn->profile_refcount[profile_index]--;

		if (profile_index == profile_a2dp &&
			phci_conn->profile_refcount[profile_index] == 1) {
			phci_conn->profile_status &= ~(BIT(profile_index));
			RTKBT_DBG("%s: clear a2dp status",__func__);
		}

		if (0 == phci_conn->profile_refcount[profile_index]) {
			need_update = TRUE;
			phci_conn->profile_bitmap &= ~(BIT(profile_index));

			phci_conn->profile_status &= ~(BIT(profile_index));

			if (profile_index == profile_sink) {
				phci_conn->profile_bitmap &= ~(BIT(profile_a2dpsink));
				phci_conn->profile_status &= ~(BIT(profile_a2dpsink));
			}
			rtk_check_del_timer(profile_index, phci_conn);
			/* clear profile_hid_interval if need */
			if ((profile_hid == profile_index)
			    && (phci_conn->
				profile_bitmap & (BIT(profile_hid_interval)))) {
				phci_conn->profile_bitmap &=
				    ~(BIT(profile_hid_interval));
			}
		}
	}

	RTKBT_DBG("%s: phci_conn->profile_bitmap 0x%02x", __func__,
			phci_conn->profile_bitmap);

	if (need_update)
		rtk_notify_profileinfo_to_fw();
}

static void update_hid_active_state(uint16_t handle, uint16_t interval)
{
	uint8_t need_update = 0;
	rtk_conn_prof *phci_conn =
	    find_connection_by_handle(&btrtl_coex, handle);

	if (phci_conn == NULL)
		return;

	RTKBT_DBG("%s: handle 0x%04x, interval %u", __func__, handle, interval);
	if (((phci_conn->profile_bitmap) & (BIT(profile_hid))) == 0) {
		RTKBT_DBG("HID not connected, nothing to be down");
		return;
	}

	if (interval < 60) {
		if ((phci_conn->profile_bitmap & (BIT(profile_hid_interval))) ==
		    0) {
			need_update = 1;
			phci_conn->profile_bitmap |= BIT(profile_hid_interval);

			phci_conn->profile_refcount[profile_hid_interval]++;
			if (phci_conn->
			    profile_refcount[profile_hid_interval] == 1)
				phci_conn->profile_status |=
				    BIT(profile_hid);
		}
	} else {
		if ((phci_conn->profile_bitmap & (BIT(profile_hid_interval)))) {
			need_update = 1;
			phci_conn->profile_bitmap &=
			    ~(BIT(profile_hid_interval));

			phci_conn->profile_refcount[profile_hid_interval]--;
			if (phci_conn->
			    profile_refcount[profile_hid_interval] == 0)
				phci_conn->profile_status &=
				    ~(BIT(profile_hid));
		}
	}

	if (need_update)
		rtk_notify_profileinfo_to_fw();
}

static uint8_t handle_l2cap_con_req(uint16_t handle, uint16_t psm,
				    uint16_t scid, uint8_t direction)
{
	uint8_t status = FALSE;
	rtk_prof_info *prof_info = NULL;
	uint8_t profile_index = psm_to_profile_index(psm);

	if (profile_index == 0xFF) {
		RTKBT_DBG("PSM(0x%04x) do not need parse", psm);
		return status;
	}

	mutex_lock(&btrtl_coex.profile_mutex);
	if (direction)		//1: out
		prof_info =
		    find_profile_by_handle_scid(&btrtl_coex, handle, scid);
	else			// 0:in
		prof_info =
		    find_profile_by_handle_dcid(&btrtl_coex, handle, scid);

	if (prof_info) {
		RTKBT_DBG("%s: this profile is already exist!", __func__);
		mutex_unlock(&btrtl_coex.profile_mutex);
		return status;
	}

	if (direction)		//1: out
		status = list_allocate_add(handle, psm, profile_index, 0, scid);
	else			// 0:in
		status = list_allocate_add(handle, psm, profile_index, scid, 0);

	mutex_unlock(&btrtl_coex.profile_mutex);

	if (!status)
		RTKBT_ERR("%s: list_allocate_add failed!", __func__);

	return status;
}

static uint8_t handle_l2cap_con_rsp(uint16_t handle, uint16_t dcid,
				    uint16_t scid, uint8_t direction,
				    uint8_t result)
{
	rtk_prof_info *prof_info = NULL;
	rtk_conn_prof *phci_conn = NULL;

	mutex_lock(&btrtl_coex.profile_mutex);
	if (!direction)		//0, in
		prof_info =
		    find_profile_by_handle_scid(&btrtl_coex, handle, scid);
	else			//1, out
		prof_info =
		    find_profile_by_handle_dcid(&btrtl_coex, handle, scid);

	if (!prof_info) {
		//RTKBT_DBG("handle_l2cap_con_rsp: prof_info Not Find!!");
		mutex_unlock(&btrtl_coex.profile_mutex);
		return FALSE;
	}

	if (!result) {		//success
		RTKBT_DBG("l2cap connection success, update connection");
		if (!direction)	//0, in
			prof_info->dcid = dcid;
		else		//1, out
			prof_info->scid = dcid;

		phci_conn = find_connection_by_handle(&btrtl_coex, handle);
		if (phci_conn)
			update_profile_connection(phci_conn,
						  prof_info->profile_index,
						  TRUE);
	} else if (result != 0x0001) {
		delete_profile_from_hash(prof_info);
	}

	mutex_unlock(&btrtl_coex.profile_mutex);
	return TRUE;
}

static uint8_t handle_l2cap_discon_req(uint16_t handle, uint16_t dcid,
				       uint16_t scid, uint8_t direction)
{
	rtk_prof_info *prof_info = NULL;
	rtk_conn_prof *phci_conn = NULL;
	RTKBT_DBG("%s: handle 0x%04x, dcid 0x%04x, scid 0x%04x, dir %u",
			__func__, handle, dcid, scid, direction);

	mutex_lock(&btrtl_coex.profile_mutex);
	if (!direction)		//0: in
		prof_info =
		    find_profile_by_handle_dcid_scid(&btrtl_coex, handle,
						     scid, dcid);
	else			//1: out
		prof_info =
		    find_profile_by_handle_dcid_scid(&btrtl_coex, handle,
						     dcid, scid);

	if (!prof_info) {
		//LogMsg("handle_l2cap_discon_req: prof_info Not Find!");
		mutex_unlock(&btrtl_coex.profile_mutex);
		return 0;
	}

	phci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (!phci_conn) {
		mutex_unlock(&btrtl_coex.profile_mutex);
		return 0;
	}

	update_profile_connection(phci_conn, prof_info->profile_index, FALSE);
	if (prof_info->profile_index == profile_a2dp &&
	    (phci_conn->profile_bitmap & BIT(profile_sink)))
		update_profile_connection(phci_conn, profile_sink, FALSE);

	delete_profile_from_hash(prof_info);
	mutex_unlock(&btrtl_coex.profile_mutex);

	return 1;
}

static const char sample_freqs[4][8] = {
	"16", "32", "44.1", "48"
};

static const uint8_t sbc_blocks[4] = { 4, 8, 12, 16 };

static const char chan_modes[4][16] = {
	"MONO", "DUAL_CHANNEL", "STEREO", "JOINT_STEREO"
};

static const char alloc_methods[2][12] = {
	"LOUDNESS", "SNR"
};

static const uint8_t subbands[2] = { 4, 8 };

static void print_sbc_header(struct sbc_frame_hdr *hdr)
{
	RTKBT_DBG("syncword: %02x", hdr->syncword);
	RTKBT_DBG("freq %skHz", sample_freqs[hdr->sampling_frequency]);
	RTKBT_DBG("blocks %u", sbc_blocks[hdr->blocks]);
	RTKBT_DBG("channel mode %s", chan_modes[hdr->channel_mode]);
	RTKBT_DBG("allocation method %s",
		  alloc_methods[hdr->allocation_method]);
	RTKBT_DBG("subbands %u", subbands[hdr->subbands]);
}

static void rtl_process_media_data(rtk_conn_prof *hci_conn, u8 *data, u16 len,
				   u8 out)
{
	u8 *p = NULL;
	struct sbc_frame_hdr *sbc_header;
	struct rtp_header *rtph;
	u8 bitpool;

	if (!hci_conn || !data || !len) {
		RTKBT_ERR("%s: invalid parameters", __func__);
		return;
	}

	/* avdtp media data */
	update_profile_state(hci_conn, profile_a2dp, TRUE);
	if (!out) {
		if (!(hci_conn->profile_bitmap & BIT(profile_sink))) {
			hci_conn->profile_bitmap |= BIT(profile_sink);
			update_profile_connection(hci_conn, profile_sink, TRUE);
		}
		update_profile_state(hci_conn, profile_sink, TRUE);
	}

	/* We assume it is SBC if the packet length
	 * is bigger than 100 bytes
	 */
	if (len > 100) {
		RTKBT_INFO("%s: Length %u", __func__, len);

		p = data + sizeof(struct hci_acl_hdr) +
			   sizeof(struct l2cap_hdr);
		rtph = (struct rtp_header *)p;

		RTKBT_INFO("rtp: v %u, cc %u, pt %u", rtph->v, rtph->cc,
			   rtph->pt);
		/* move forward */
		p += sizeof(struct rtp_header) + rtph->cc * 4 + 1;

		/* point to the sbc frame header */
		sbc_header = (struct sbc_frame_hdr *)p;
		bitpool = sbc_header->bitpool;

		print_sbc_header(sbc_header);

		RTKBT_INFO("%s: bitpool %u", __func__, bitpool);

		rtk_vendor_cmd_to_fw(HCI_VENDOR_SET_BITPOOL, 1, &bitpool);
	}
	hci_conn->a2dp_packet_count++;
}

static void packets_count(u16 handle, u16 scid, u8 out, u8 *data, u16 len)
{
	rtk_prof_info *prof = NULL;
	rtk_conn_prof *hci_conn = NULL;

	set_bit(RTL_COEX_PKT_COUNTING, &btrtl_coex.flags);
	if (test_bit(RTL_COEX_CONN_REMOVING, &btrtl_coex.flags)) {
		RTKBT_INFO("%s: conn/prof is being removed", __func__);
		goto done;
	}

	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (!hci_conn)
		goto done;
	if (hci_conn->type != HCI_CONN_TYPE_BREDR_ACL)
		goto done;

	if (!out)
		prof = find_profile_by_handle_scid(&btrtl_coex, handle, scid);
	else
		prof = find_profile_by_handle_dcid(&btrtl_coex, handle, scid);
	if (!prof)
		goto done;

	/* avdtp media data */
	if (prof->profile_index == profile_a2dp &&
	    prof->flags == A2DP_MEDIA) {
		if (!(hci_conn->profile_status & BIT(profile_a2dp))) {
#ifdef DIRECT_A2DP_MEDIA_PACKET_PROCESSING
			rtl_process_media_data(hci_conn, data, len, out);
#else
			u16 n;
			unsigned long flags;
			struct rtl_l2_buff *l2;

			l2 = rtl_l2_node_get(&btrtl_coex);
			if (!l2) {
				RTKBT_ERR("%s: No free l2 node", __func__);
				goto done;
			}

			n = min_t(uint, len, L2_MAX_SUBSEC_LEN);
			memcpy(l2->data, data, n);

			if (out)
				l2->type = HCI_PT_L2DATA_TX;
			else
				l2->type = HCI_PT_L2DATA_RX;
			spin_lock_irqsave(&btrtl_coex.buff_lock, flags);
			list_add_tail(&l2->list, &btrtl_coex.hci_pkt_list);
			spin_unlock_irqrestore(&btrtl_coex.buff_lock, flags);

			queue_delayed_work(btrtl_coex.fw_wq,
					   &btrtl_coex.fw_work, 0);
#endif
		} else {
			hci_conn->a2dp_packet_count++;
		}
	}
	if (prof->profile_index == profile_pan)
		hci_conn->pan_packet_count++;

	if (prof->profile_index == profile_pan)
		hci_conn->pan_packet_count++;
done:
	clear_bit(RTL_COEX_PKT_COUNTING, &btrtl_coex.flags);
	return;
}

static void count_a2dp_packet_timeout(struct work_struct *work)
{
	rtk_conn_prof *hci_conn = container_of(work, rtk_conn_prof,
						a2dp_count_work.work);
	if (hci_conn->a2dp_packet_count)
		RTKBT_DBG("%s: a2dp_packet_count %d", __func__,
			  hci_conn->a2dp_packet_count);
	if (hci_conn->a2dp_packet_count == 0) {
		if (is_profile_busy(hci_conn, profile_a2dp)) {
			RTKBT_DBG("%s: a2dp busy->idle!", __func__);
			/*
			 * We should prevent any conn from being deleted when
			 * update_profile_state() traverses the conn list.
			 */
			mutex_lock(&btrtl_coex.conn_mutex);
			update_profile_state(hci_conn, profile_a2dp, FALSE);
			if (hci_conn->profile_bitmap & BIT(profile_sink))
				update_profile_state(hci_conn, profile_sink, FALSE);
			mutex_unlock(&btrtl_coex.conn_mutex);
		}
	}
	hci_conn->a2dp_packet_count = 0;

	queue_delayed_work(btrtl_coex.fw_wq, &hci_conn->a2dp_count_work,
			   msecs_to_jiffies(1000));
}

static void count_pan_packet_timeout(struct work_struct *work)
{
	rtk_conn_prof *hci_conn = container_of(work, rtk_conn_prof,
						pan_count_work.work);
	if (hci_conn->pan_packet_count)
		RTKBT_DBG("%s: pan_packet_count %d", __func__,
			  hci_conn->pan_packet_count);
	if (hci_conn->pan_packet_count < PAN_PACKET_COUNT) {
		if (is_profile_busy(hci_conn, profile_pan)) {
			RTKBT_DBG("%s: pan busy->idle!", __func__);
			mutex_lock(&btrtl_coex.conn_mutex);
			update_profile_state(hci_conn, profile_pan, FALSE);
			mutex_unlock(&btrtl_coex.conn_mutex);
		}
	} else {
		if (!is_profile_busy(hci_conn, profile_pan)) {
			RTKBT_DBG("timeout_handler: pan idle->busy!");
			mutex_lock(&btrtl_coex.conn_mutex);
			update_profile_state(hci_conn, profile_pan, TRUE);
			mutex_unlock(&btrtl_coex.conn_mutex);
		}
	}
	hci_conn->pan_packet_count = 0;
	queue_delayed_work(btrtl_coex.fw_wq, &hci_conn->pan_count_work,
			   msecs_to_jiffies(1000));
}

static void count_hogp_packet_timeout(struct work_struct *work)
{
	rtk_conn_prof *hci_conn = container_of(work, rtk_conn_prof,
						hogp_count_work.work);
	if (hci_conn->hogp_packet_count)
		RTKBT_DBG("%s: hogp_packet_count %d", __func__,
			  hci_conn->hogp_packet_count);
	if (hci_conn->hogp_packet_count == 0) {
		if (is_profile_busy(hci_conn, profile_hogp)) {
			RTKBT_DBG("%s: hogp busy->idle!", __func__);
			mutex_lock(&btrtl_coex.conn_mutex);
			update_profile_state(hci_conn, profile_hogp, FALSE);
			mutex_unlock(&btrtl_coex.conn_mutex);
		}
	}
	hci_conn->hogp_packet_count = 0;

	if (hci_conn->voice_packet_count)
		RTKBT_DBG("%s: voice_packet_count %d", __func__,
			  hci_conn->voice_packet_count);
	if (hci_conn->voice_packet_count == 0) {
		if (is_profile_busy(hci_conn, profile_voice)) {
			RTKBT_DBG("%s: voice busy->idle!", __func__);
			mutex_lock(&btrtl_coex.conn_mutex);
			update_profile_state(hci_conn, profile_voice, FALSE);
			mutex_unlock(&btrtl_coex.conn_mutex);
		}
	}
	hci_conn->voice_packet_count = 0;
	queue_delayed_work(btrtl_coex.fw_wq, &hci_conn->hogp_count_work,
			   msecs_to_jiffies(1000));
}

#ifdef RTB_SOFTWARE_MAILBOX

#ifndef RTK_COEX_OVER_SYMBOL
static int udpsocket_send(char *tx_msg, int msg_size)
{
	u8 error = 0;
	struct msghdr udpmsg;
	mm_segment_t oldfs;
	struct iovec iov;

	RTKBT_DBG("send msg %s with len:%d", tx_msg, msg_size);

	if (btrtl_coex.sock_open) {
		iov.iov_base = (void *)tx_msg;
		iov.iov_len = msg_size;
		udpmsg.msg_name = &btrtl_coex.wifi_addr;
		udpmsg.msg_namelen = sizeof(struct sockaddr_in);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
		udpmsg.msg_iov = &iov;
		udpmsg.msg_iovlen = 1;
#else
		iov_iter_init(&udpmsg.msg_iter, WRITE, &iov, 1, msg_size);
#endif
		udpmsg.msg_control = NULL;
		udpmsg.msg_controllen = 0;
		udpmsg.msg_flags = MSG_DONTWAIT | MSG_NOSIGNAL;
		oldfs = get_fs();
		set_fs(KERNEL_DS);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)
		error = sock_sendmsg(btrtl_coex.udpsock, &udpmsg, msg_size);
#else
		error = sock_sendmsg(btrtl_coex.udpsock, &udpmsg);
#endif
		set_fs(oldfs);

		if (error < 0)
			RTKBT_DBG("Error when sendimg msg, error:%d", error);
	}

	return error;
}
#endif

#ifdef RTK_COEX_OVER_SYMBOL
/* Receive message from WiFi */
u8 rtw_btcoex_wifi_to_bt(u8 *msg, u8 msg_size)
{
	struct sk_buff *nskb;

	if (!rtw_coex_on) {
		RTKBT_WARN("Bluetooth is closed");
		return 0;
	}

	nskb = alloc_skb(msg_size, GFP_ATOMIC);
	if (!nskb) {
		RTKBT_ERR("Couldnt alloc skb for WiFi coex message");
		return 0;
	}

	memcpy(skb_put(nskb, msg_size), msg, msg_size);
	skb_queue_tail(&rtw_q, nskb);

	queue_work(rtw_wq, &rtw_work);

	return 1;
}
EXPORT_SYMBOL(rtw_btcoex_wifi_to_bt);

static int rtk_send_coexmsg2wifi(u8 *msg, u8 size)
{
	u8 result;
	u8 (*btmsg_to_wifi)(u8 *, u8);

	btmsg_to_wifi = __symbol_get(VMLINUX_SYMBOL_STR(rtw_btcoex_bt_to_wifi));

	if (!btmsg_to_wifi) {
		/* RTKBT_ERR("Couldnt get symbol"); */
		return -1;
	}

	result = btmsg_to_wifi(msg, size);
	__symbol_put(VMLINUX_SYMBOL_STR(rtw_btcoex_bt_to_wifi));
	if (!result) {
		RTKBT_ERR("Couldnt send coex msg to WiFi");
		return -1;
	} else if (result == 1){
		/* successful to send message */
		return 0;
	} else {
		RTKBT_ERR("Unknown result %d", result);
		return -1;
	}
}

static int rtkbt_process_coexskb(struct sk_buff *skb)
{
	rtk_handle_event_from_wifi(skb->data);
	return 0;
}

static void rtw_work_func(struct work_struct *work)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&rtw_q))) {
		rtkbt_process_coexskb(skb);
		kfree_skb(skb);
	}
}

#endif

static int rtkbt_coexmsg_send(char *tx_msg, int msg_size)
{
#ifdef RTK_COEX_OVER_SYMBOL
	return rtk_send_coexmsg2wifi((uint8_t *)tx_msg, (u8)msg_size);
#else
	return udpsocket_send(tx_msg, msg_size);
#endif
}

#ifndef RTK_COEX_OVER_SYMBOL
static void udpsocket_recv_data(void)
{
	u8 recv_data[512];
	u32 len = 0;
	u16 recv_length;
	struct sk_buff *skb;

	RTKBT_DBG("-");

	spin_lock(&btrtl_coex.spin_lock_sock);
	len = skb_queue_len(&btrtl_coex.sk->sk_receive_queue);

	while (len > 0) {
		skb = skb_dequeue(&btrtl_coex.sk->sk_receive_queue);

		/*important: cut the udp header from skb->data! header length is 8 byte */
		recv_length = skb->len - 8;
		memset(recv_data, 0, sizeof(recv_data));
		memcpy(recv_data, skb->data + 8, recv_length);
		//RTKBT_DBG("received data: %s :with len %u", recv_data, recv_length);

		rtk_handle_event_from_wifi(recv_data);

		len--;
		kfree_skb(skb);
	}

	spin_unlock(&btrtl_coex.spin_lock_sock);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0)
static void udpsocket_recv(struct sock *sk, int bytes)
#else
static void udpsocket_recv(struct sock *sk)
#endif
{
	spin_lock(&btrtl_coex.spin_lock_sock);
	btrtl_coex.sk = sk;
	spin_unlock(&btrtl_coex.spin_lock_sock);
	queue_delayed_work(btrtl_coex.sock_wq, &btrtl_coex.sock_work, 0);
}

static void create_udpsocket(void)
{
	int err;
	RTKBT_DBG("%s: connect_port: %d", __func__, CONNECT_PORT);
	btrtl_coex.sock_open = 0;

	err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP,
			&btrtl_coex.udpsock);
	if (err < 0) {
		RTKBT_ERR("%s: sock create error, err = %d", __func__, err);
		return;
	}

	memset(&btrtl_coex.addr, 0, sizeof(struct sockaddr_in));
	btrtl_coex.addr.sin_family = AF_INET;
	btrtl_coex.addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	btrtl_coex.addr.sin_port = htons(CONNECT_PORT);

	memset(&btrtl_coex.wifi_addr, 0, sizeof(struct sockaddr_in));
	btrtl_coex.wifi_addr.sin_family = AF_INET;
	btrtl_coex.wifi_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	btrtl_coex.wifi_addr.sin_port = htons(CONNECT_PORT_WIFI);

	err =
	    btrtl_coex.udpsock->ops->bind(btrtl_coex.udpsock,
					     (struct sockaddr *)&btrtl_coex.
					     addr, sizeof(struct sockaddr));
	if (err < 0) {
		sock_release(btrtl_coex.udpsock);
		RTKBT_ERR("%s: sock bind error, err = %d",__func__,  err);
		return;
	}

	btrtl_coex.sock_open = 1;
	btrtl_coex.udpsock->sk->sk_data_ready = udpsocket_recv;
}
#endif /* !RTK_COEX_OVER_SYMBOL */

static void rtk_notify_extension_version_to_wifi(void)
{
	uint8_t para_length = 2;
	char p_buf[2 + HCI_CMD_PREAMBLE_SIZE];
	char *p = p_buf;

	if (!btrtl_coex.wifi_on)
		return;

	UINT16_TO_STREAM(p, HCI_OP_HCI_EXTENSION_VERSION_NOTIFY);
	*p++ = para_length;
	UINT16_TO_STREAM(p, HCI_EXTENSION_VERSION);
	RTKBT_DBG("extension version is 0x%x", HCI_EXTENSION_VERSION);
	if (rtkbt_coexmsg_send(p_buf, para_length + HCI_CMD_PREAMBLE_SIZE) < 0)
		RTKBT_ERR("%s: sock send error", __func__);
}

static void rtk_notify_btpatch_version_to_wifi(void)
{
	uint8_t para_length = 4;
	char p_buf[para_length + HCI_CMD_PREAMBLE_SIZE];
	char *p = p_buf;

	if (!btrtl_coex.wifi_on)
		return;

	UINT16_TO_STREAM(p, HCI_OP_HCI_BT_PATCH_VER_NOTIFY);
	*p++ = para_length;
	UINT16_TO_STREAM(p, btrtl_coex.hci_reversion);
	UINT16_TO_STREAM(p, btrtl_coex.lmp_subversion);
	RTKBT_DBG("btpatch ver: len %u, hci_rev 0x%04x, lmp_subver 0x%04x",
			para_length, btrtl_coex.hci_reversion,
			btrtl_coex.lmp_subversion);

	if (rtkbt_coexmsg_send(p_buf, para_length + HCI_CMD_PREAMBLE_SIZE) < 0)
		RTKBT_ERR("%s: sock send error", __func__);
}

static void rtk_notify_afhmap_to_wifi(void)
{
	uint8_t para_length = 13;
	char p_buf[para_length + HCI_CMD_PREAMBLE_SIZE];
	char *p = p_buf;
	uint8_t kk = 0;

	if (!btrtl_coex.wifi_on)
		return;

	UINT16_TO_STREAM(p, HCI_OP_HCI_BT_AFH_MAP_NOTIFY);
	*p++ = para_length;
	*p++ = btrtl_coex.piconet_id;
	*p++ = btrtl_coex.mode;
	*p++ = 10;
	memcpy(p, btrtl_coex.afh_map, 10);

	RTKBT_DBG("afhmap, piconet_id is 0x%x, map type is 0x%x",
		  btrtl_coex.piconet_id, btrtl_coex.mode);
	for (kk = 0; kk < 10; kk++)
		RTKBT_DBG("afhmap data[%d] is 0x%x", kk,
			  btrtl_coex.afh_map[kk]);

	if (rtkbt_coexmsg_send(p_buf, para_length + HCI_CMD_PREAMBLE_SIZE) < 0)
		RTKBT_ERR("%s: sock send error", __func__);
}

static void rtk_notify_btcoex_to_wifi(uint8_t opcode, uint8_t status)
{
	uint8_t para_length = 2;
	char p_buf[para_length + HCI_CMD_PREAMBLE_SIZE];
	char *p = p_buf;

	if (!btrtl_coex.wifi_on)
		return;

	UINT16_TO_STREAM(p, HCI_OP_HCI_BT_COEX_NOTIFY);
	*p++ = para_length;
	*p++ = opcode;
	if (!status)
		*p++ = 0;
	else
		*p++ = 1;

	RTKBT_DBG("btcoex, opcode is 0x%x, status is 0x%x", opcode, status);

	if (rtkbt_coexmsg_send(p_buf, para_length + HCI_CMD_PREAMBLE_SIZE) < 0)
		RTKBT_ERR("%s: sock send error", __func__);
}

static void rtk_notify_btoperation_to_wifi(uint8_t operation,
					   uint8_t append_data_length,
					   uint8_t * append_data)
{
	uint8_t para_length = 3 + append_data_length;
	char p_buf[para_length + HCI_CMD_PREAMBLE_SIZE];
	char *p = p_buf;
	uint8_t kk = 0;

	if (!btrtl_coex.wifi_on)
		return;

	UINT16_TO_STREAM(p, HCI_OP_BT_OPERATION_NOTIFY);
	*p++ = para_length;
	*p++ = operation;
	*p++ = append_data_length;
	if (append_data_length)
		memcpy(p, append_data, append_data_length);

	RTKBT_DBG("btoperation: op 0x%02x, append_data_length %u",
		  operation, append_data_length);
	if (append_data_length) {
		for (kk = 0; kk < append_data_length; kk++)
			RTKBT_DBG("append data is 0x%x", *(append_data + kk));
	}

	if (rtkbt_coexmsg_send(p_buf, para_length + HCI_CMD_PREAMBLE_SIZE) < 0)
		RTKBT_ERR("%s: sock send error", __func__);
}

static void rtk_notify_info_to_wifi(uint8_t reason, uint8_t length,
				    uint8_t *report_info)
{
	uint8_t para_length = 4 + length;
	char buf[para_length + HCI_CMD_PREAMBLE_SIZE];
	char *p = buf;
	struct rtl_btinfo *report = (struct rtl_btinfo *)report_info;

	if (length) {
		RTKBT_DBG("bt info: cmd %2.2X", report->cmd);
		RTKBT_DBG("bt info: len %2.2X", report->len);
		RTKBT_DBG("bt info: data %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X",
			  report->data[0], report->data[1], report->data[2],
			  report->data[3], report->data[4], report->data[5]);
	}
	RTKBT_DBG("bt info: reason 0x%2x, length 0x%2x", reason, length);

	if (!btrtl_coex.wifi_on)
		return;

	UINT16_TO_STREAM(p, HCI_OP_HCI_BT_INFO_NOTIFY);
	*p++ = para_length;
	*p++ = btrtl_coex.polling_enable;
	*p++ = btrtl_coex.polling_interval;
	*p++ = reason;
	*p++ = length;

	if (length)
		memcpy(p, report_info, length);

	RTKBT_DBG("para length %2x, polling_enable %u, poiiling_interval %u",
	     para_length, btrtl_coex.polling_enable,
	     btrtl_coex.polling_interval);
	/* send BT INFO to Wi-Fi driver */
	if (rtkbt_coexmsg_send(buf, para_length + HCI_CMD_PREAMBLE_SIZE) < 0)
		RTKBT_ERR("%s: sock send error", __func__);
}

static void rtk_notify_regester_to_wifi(uint8_t * reg_value)
{
	uint8_t para_length = 9;
	char p_buf[para_length + HCI_CMD_PREAMBLE_SIZE];
	char *p = p_buf;
	hci_mailbox_register *reg = (hci_mailbox_register *) reg_value;

	if (!btrtl_coex.wifi_on)
		return;

	UINT16_TO_STREAM(p, HCI_OP_HCI_BT_REGISTER_VALUE_NOTIFY);
	*p++ = para_length;
	memcpy(p, reg_value, para_length);

	RTKBT_DBG("bt register, register type is %x", reg->type);
	RTKBT_DBG("bt register, register offset is %x", reg->offset);
	RTKBT_DBG("bt register, register value is %x", reg->value);

	if (rtkbt_coexmsg_send(p_buf, para_length + HCI_CMD_PREAMBLE_SIZE) < 0)
		RTKBT_ERR("%s: sock send error", __func__);
}

#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
static void rtk_btcoex_handle_cmd_le_create_cis(u8 *buffer, int count)
{
	struct hci_cp_le_create_cis *cp;
	struct hci_cis *cis;
	rtk_conn_prof *hci_conn = NULL;
	u16 cis_handle;
	u8 i;

	cp = (void *)(buffer + sizeof(struct hci_command_hdr));
	cis = cp->cis;
	for (i = 0; i < cp->num_cis; i++) {
		cis_handle = get_unaligned_le16(&cis[i].cis_handle);
		hci_conn = find_connection_by_handle(&btrtl_coex, cis_handle);
		if (hci_conn) {
			RTKBT_ERR("%s: conn %04x has existed", __func__,
				  cis_handle);
			continue;
		}
		hci_conn = allocate_connection_by_handle(cis_handle);
		if (!hci_conn) {
			RTKBT_ERR("cis (0x%04x) allocation failed", cis_handle);
			continue;
		}

		hci_conn->profile_bitmap = 0;
		hci_conn->profile_status = 0;
		memset(hci_conn->profile_refcount, 0, profile_max);
		hci_conn->type = HCI_CONN_TYPE_CIS;

		add_connection_to_hash(&btrtl_coex, hci_conn);
	}
}

static void rtk_btcoex_handle_cmd_le_setup_iso_path(u8 *buffer, int count)
{
	struct hci_cp_le_setup_iso_path *cp;
	rtk_conn_prof *hci_conn = NULL;
	u16 handle;

	cp = (void *)(buffer + sizeof(struct hci_command_hdr));
	handle = get_unaligned_le16(&cp->handle);

	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (!hci_conn)
		return;

	if (!cp->direction)
		hci_conn->direction |= HCI_DATA_PATH_INPUT;
	else
		hci_conn->direction |= HCI_DATA_PATH_OUTPUT;

	RTKBT_INFO("data path direction 0x%02x", hci_conn->direction);
}

static void rtk_btcoex_handle_cmd_le_remove_iso_path(u8 *buffer, int count)
{
	rtk_conn_prof *hci_conn = NULL;
	struct hci_cp_le_remove_iso_path *cp;
	u16 handle;

	cp = (void *)(buffer + sizeof(struct hci_command_hdr));
	handle = get_unaligned_le16(&cp->handle);

	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (!hci_conn)
		return;

	if (!cp->direction)
		hci_conn->remove_path = HCI_DATA_PATH_INPUT;
	else
		hci_conn->remove_path = HCI_DATA_PATH_OUTPUT;

	RTKBT_INFO("Remove iso path handle %04x, direction %02x", handle,
		   cp->direction);
}

static void rtk_btcoex_cmd_enqueue(u8 *buffer, int count)
{
	/* We use ev structure for cmd */
	struct rtl_hci_ev *cmd;
	unsigned long flags;

	cmd = rtl_ev_node_get(&btrtl_coex);
	if (!cmd) {
		RTKBT_ERR("%s: No mem for saving cmd", __func__);
		return;
	}

	if (count > MAX_LEN_OF_HCI_EV) {
		memcpy(cmd->data, buffer, MAX_LEN_OF_HCI_EV);
		cmd->len = MAX_LEN_OF_HCI_EV;
	} else {
		memcpy(cmd->data, buffer, count);
		cmd->len = count;
	}
	cmd->type = HCI_PT_CMD;
	spin_lock_irqsave(&btrtl_coex.buff_lock, flags);
	list_add_tail(&cmd->list, &btrtl_coex.hci_pkt_list);
	spin_unlock_irqrestore(&btrtl_coex.buff_lock, flags);

	queue_delayed_work(btrtl_coex.fw_wq, &btrtl_coex.fw_work, 0);
}

#endif

void rtk_btcoex_parse_cmd(uint8_t *buffer, int count)
{
	u16 opcode = (buffer[0]) + (buffer[1] << 8);

	if (!test_bit(RTL_COEX_RUNNING, &btrtl_coex.flags)) {
		RTKBT_INFO("%s: Coex is closed, ignore", __func__);
		return;
	}

	switch (opcode) {
	case HCI_OP_INQUIRY:
	case HCI_OP_PERIODIC_INQ:
		if (!btrtl_coex.isinquirying) {
			btrtl_coex.isinquirying = 1;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("hci (periodic)inq, notify wifi "
				  "inquiry start");
			rtk_notify_btoperation_to_wifi(BT_OPCODE_INQUIRY_START,
						       0, NULL);
#else
			RTKBT_INFO("hci (periodic)inq start");
#endif
		}
		break;
	case HCI_OP_INQUIRY_CANCEL:
	case HCI_OP_EXIT_PERIODIC_INQ:
		if (btrtl_coex.isinquirying) {
			btrtl_coex.isinquirying = 0;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("hci (periodic)inq cancel/exit, notify wifi "
				  "inquiry stop");
			rtk_notify_btoperation_to_wifi(BT_OPCODE_INQUIRY_END, 0,
						       NULL);
#else
			RTKBT_INFO("hci (periodic)inq cancel/exit");
#endif
		}
		break;
	case HCI_OP_ACCEPT_CONN_REQ:
		if (!btrtl_coex.ispaging) {
			btrtl_coex.ispaging = 1;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("hci accept connreq, notify wifi page start");
			rtk_notify_btoperation_to_wifi(BT_OPCODE_PAGE_START, 0,
						       NULL);
#else
			RTKBT_INFO("hci accept conn req");
#endif
		}
		break;
	case HCI_OP_DISCONNECT:
		RTKBT_INFO("HCI OP Disconnect, handle %04x, reason 0x%02x",
			   ((u16)buffer[4] << 8 | buffer[3]), buffer[5]);
		break;
#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	case HCI_OP_LE_CREATE_CIS:
	case HCI_OP_LE_SETUP_ISO_PATH:
	case HCI_OP_LE_REMOVE_ISO_PATH:
		rtk_btcoex_cmd_enqueue(buffer, count);
		break;
#endif
	default:
		break;
	}
}

static void rtk_handle_inquiry_complete(void)
{
	if (btrtl_coex.isinquirying) {
		btrtl_coex.isinquirying = 0;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("inq complete, notify wifi inquiry end");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_INQUIRY_END, 0, NULL);
#else
		RTKBT_INFO("inquiry complete");
#endif
	}
}

static void rtk_handle_pin_code_req(void)
{
	if (!btrtl_coex.ispairing) {
		btrtl_coex.ispairing = 1;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("pin code req, notify wifi pair start");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_PAIR_START, 0, NULL);
#else
		RTKBT_INFO("pin code request");
#endif
	}
}

static void rtk_handle_io_capa_req(void)
{
	if (!btrtl_coex.ispairing) {
		btrtl_coex.ispairing = 1;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("io cap req, notify wifi pair start");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_PAIR_START, 0, NULL);
#else
		RTKBT_INFO("io capability request");
#endif
	}
}

static void rtk_handle_auth_request(void)
{
	if (btrtl_coex.ispairing) {
		btrtl_coex.ispairing = 0;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("auth req, notify wifi pair end");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_PAIR_END, 0, NULL);
#else
		RTKBT_INFO("authentication request");
#endif
	}
}

static void rtk_handle_link_key_notify(void)
{
	if (btrtl_coex.ispairing) {
		btrtl_coex.ispairing = 0;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("link key notify, notify wifi pair end");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_PAIR_END, 0, NULL);
#else
		RTKBT_INFO("link key notify");
#endif
	}
}

static void rtk_handle_mode_change_evt(u8 * p)
{
	u16 mode_change_handle, mode_interval;

	p++;
	STREAM_TO_UINT16(mode_change_handle, p);
	p++;
	STREAM_TO_UINT16(mode_interval, p);
	update_hid_active_state(mode_change_handle, mode_interval);
}

#ifdef RTB_SOFTWARE_MAILBOX
static void rtk_parse_vendor_mailbox_cmd_evt(u8 * p, u8 total_len)
{
	u8 status, subcmd;
	u8 temp_cmd[10];

	status = *p++;
	if (total_len <= 4) {
		RTKBT_DBG("receive mailbox cmd from fw, total length <= 4");
		return;
	}
	subcmd = *p++;
	RTKBT_DBG("receive mailbox cmd from fw, subcmd is 0x%x, status is 0x%x",
		  subcmd, status);

	switch (subcmd) {
	case HCI_VENDOR_SUB_CMD_BT_REPORT_CONN_SCO_INQ_INFO:
		if (status == 0)	//success
			rtk_notify_info_to_wifi(POLLING_RESPONSE,
					RTL_BTINFO_LEN, (uint8_t *)p);
		break;

	case HCI_VENDOR_SUB_CMD_WIFI_CHANNEL_AND_BANDWIDTH_CMD:
		rtk_notify_btcoex_to_wifi(WIFI_BW_CHNL_NOTIFY, status);
		break;

	case HCI_VENDOR_SUB_CMD_WIFI_FORCE_TX_POWER_CMD:
		rtk_notify_btcoex_to_wifi(BT_POWER_DECREASE_CONTROL, status);
		break;

	case HCI_VENDOR_SUB_CMD_BT_ENABLE_IGNORE_WLAN_ACT_CMD:
		rtk_notify_btcoex_to_wifi(IGNORE_WLAN_ACTIVE_CONTROL, status);
		break;

	case HCI_VENDOR_SUB_CMD_SET_BT_PSD_MODE:
		rtk_notify_btcoex_to_wifi(BT_PSD_MODE_CONTROL, status);
		break;

	case HCI_VENDOR_SUB_CMD_SET_BT_LNA_CONSTRAINT:
		rtk_notify_btcoex_to_wifi(LNA_CONSTRAIN_CONTROL, status);
		break;

	case HCI_VENDOR_SUB_CMD_BT_AUTO_REPORT_ENABLE:
		break;

	case HCI_VENDOR_SUB_CMD_BT_SET_TXRETRY_REPORT_PARAM:
		break;

	case HCI_VENDOR_SUB_CMD_BT_SET_PTATABLE:
		break;

	case HCI_VENDOR_SUB_CMD_GET_AFH_MAP_L:
		if (status == 0) {
			memcpy(btrtl_coex.afh_map, p + 4, 4);	/* cmd_idx, length, piconet_id, mode */
			temp_cmd[0] = HCI_VENDOR_SUB_CMD_GET_AFH_MAP_M;
			temp_cmd[1] = 2;
			temp_cmd[2] = btrtl_coex.piconet_id;
			temp_cmd[3] = btrtl_coex.mode;
			rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 4,
					     temp_cmd);
		} else {
			memset(btrtl_coex.afh_map, 0, 10);
			rtk_notify_afhmap_to_wifi();
		}
		break;

	case HCI_VENDOR_SUB_CMD_GET_AFH_MAP_M:
		if (status == 0) {
			memcpy(btrtl_coex.afh_map + 4, p + 4, 4);
			temp_cmd[0] = HCI_VENDOR_SUB_CMD_GET_AFH_MAP_H;
			temp_cmd[1] = 2;
			temp_cmd[2] = btrtl_coex.piconet_id;
			temp_cmd[3] = btrtl_coex.mode;
			rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 4,
					     temp_cmd);
		} else {
			memset(btrtl_coex.afh_map, 0, 10);
			rtk_notify_afhmap_to_wifi();
		}
		break;

	case HCI_VENDOR_SUB_CMD_GET_AFH_MAP_H:
		if (status == 0)
			memcpy(btrtl_coex.afh_map + 8, p + 4, 2);
		else
			memset(btrtl_coex.afh_map, 0, 10);

		rtk_notify_afhmap_to_wifi();
		break;

	case HCI_VENDOR_SUB_CMD_RD_REG_REQ:
		if (status == 0)
			rtk_notify_regester_to_wifi(p + 3);	/* cmd_idx,length,regist type */
		break;

	case HCI_VENDOR_SUB_CMD_WR_REG_REQ:
		rtk_notify_btcoex_to_wifi(BT_REGISTER_ACCESS, status);
		break;

	default:
		break;
	}
}
#endif /* RTB_SOFTWARE_MAILBOX */

#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
static void big_conn_remove(u16 big_handle)
{
	struct list_head *head = &btrtl_coex.conn_hash;
	struct list_head *iter = NULL, *temp = NULL;
	rtk_conn_prof *hci_conn = NULL;

	list_for_each_safe(iter, temp, head) {
		hci_conn = list_entry(iter, rtk_conn_prof, list);
		if ((big_handle & 0xFFF) != hci_conn->big_handle)
			continue;
		if (hci_conn->profile_bitmap & BIT(profile_lea_src))
			update_profile_connection(hci_conn, profile_lea_src,
						  FALSE);
		else if (hci_conn->profile_bitmap & BIT(profile_lea_snk))
			update_profile_connection(hci_conn, profile_lea_snk,
						  FALSE);

		delete_connection_from_hash(hci_conn);
	}
}

static void rtk_handle_cc_le_remove_iso_data_path(u8 *p)
{
	rtk_conn_prof *hci_conn = NULL;
	u16 handle = 0;
	u8 status;

	status = *p++;
	if (status) {
		RTKBT_ERR("%s: status %02x", __func__, status);
		return;
	}

	handle = get_unaligned_le16(p);
	RTKBT_DBG("BTCOEX conn handle 0x%04x", handle);

	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (!hci_conn)
		return;

	if (hci_conn->remove_path == HCI_DATA_PATH_INPUT &&
	    is_profile_busy(hci_conn, profile_lea_src))
		update_profile_state(hci_conn, profile_lea_src, FALSE);
	else if (hci_conn->remove_path == HCI_DATA_PATH_OUTPUT &&
		 is_profile_busy(hci_conn, profile_lea_snk))
		update_profile_state(hci_conn, profile_lea_snk, FALSE);
}

static void rtk_handle_cc_le_setup_iso_path(u8 *p)
{
	rtk_conn_prof *hci_conn = NULL;
	u16 handle;
	u8 status = *p++;

	if (status) {
		RTKBT_ERR("setup iso path evt err, status 0x%02x", status);
		return;
	}

	handle = get_unaligned_le16(p);

	RTKBT_DBG("BTCOEX conn handle 0x%04x", handle);

	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (!hci_conn)
		return;

	if ((hci_conn->direction & HCI_DATA_PATH_INPUT) &&
	    !is_profile_busy(hci_conn, profile_lea_src))
		update_profile_state(hci_conn, profile_lea_src, TRUE);
	if ((hci_conn->direction & HCI_DATA_PATH_OUTPUT) &&
	    !is_profile_busy(hci_conn, profile_lea_snk))
		update_profile_state(hci_conn, profile_lea_snk, TRUE);
}

static void rtk_handle_cc_le_big_term_sync(u8 *data)
{
	u8 status;
	u8 big_handle;

	status = *data++;
	big_handle = *data++;

	if (!status) {
		u16 handle = big_handle;

		handle += HCI_CONN_HANDLE_UNSET_START;
		big_conn_remove(handle);
	}
}
#endif

static void rtk_handle_cmd_complete_evt(u8 total_len, u8 * p)
{
	u16 opcode;

	p++;
	STREAM_TO_UINT16(opcode, p);
	//RTKBT_DBG("cmd_complete, opcode is 0x%x", opcode);

	if (opcode == HCI_OP_PERIODIC_INQ) {
		if (*p++ && btrtl_coex.isinquirying) {
			btrtl_coex.isinquirying = 0;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("hci period inq, start error, notify wifi "
				  "inquiry stop");
			rtk_notify_btoperation_to_wifi(BT_OPCODE_INQUIRY_END, 0,
						       NULL);
#else
			RTKBT_INFO("hci period inquiry start error");
#endif
		}
	}

	if (opcode == HCI_OP_READ_LOCAL_VERSION) {
		if (!(*p++)) {
			p++;
			STREAM_TO_UINT16(btrtl_coex.hci_reversion, p);
			p += 3;
			STREAM_TO_UINT16(btrtl_coex.lmp_subversion, p);
			RTKBT_DBG("BTCOEX hci_rev 0x%04x",
				  btrtl_coex.hci_reversion);
			RTKBT_DBG("BTCOEX lmp_subver 0x%04x",
				  btrtl_coex.lmp_subversion);
		}
	}
#ifdef RTB_SOFTWARE_MAILBOX
	if (opcode == HCI_VENDOR_MAILBOX_CMD) {
		rtk_parse_vendor_mailbox_cmd_evt(p, total_len);
	}
#endif
	if (opcode == HCI_VENDOR_SET_PROFILE_REPORT_COMMAND) {
		//0x01-unknown hci command
		if ((*p++) == 0x01) {
			//RTKBT_DBG("unknown hci command");
			return;
		} else {
			profileinfo_cmd = 1;
		}
	}

#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	if (opcode == HCI_OP_LE_SETUP_ISO_PATH)
		rtk_handle_cc_le_setup_iso_path(p);

	if (opcode == HCI_OP_LE_REMOVE_ISO_PATH)
		rtk_handle_cc_le_remove_iso_data_path(p);

	if (opcode == HCI_OP_LE_BIG_TERM_SYNC)
		rtk_handle_cc_le_big_term_sync(p);
#endif
}

static void rtk_handle_cmd_status_evt(u8 * p)
{
	u16 opcode;
	u8 status;

	status = *p++;
	p++;
	STREAM_TO_UINT16(opcode, p);
	//RTKBT_DBG("cmd_status, opcode is 0x%x", opcode);
	if ((opcode == HCI_OP_INQUIRY) && (status)) {
		if (btrtl_coex.isinquirying) {
			btrtl_coex.isinquirying = 0;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("hci inq, start error, notify wifi inq stop");
			rtk_notify_btoperation_to_wifi(BT_OPCODE_INQUIRY_END, 0,
						       NULL);
#else
			RTKBT_INFO("hci inquiry start error");
#endif
		}
	}

	if (opcode == HCI_OP_CREATE_CONN) {
		if (!status && !btrtl_coex.ispaging) {
			btrtl_coex.ispaging = 1;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("hci create conn, notify wifi start page");
			rtk_notify_btoperation_to_wifi(BT_OPCODE_PAGE_START, 0,
						       NULL);
#else
			RTKBT_INFO("hci create connection, start paging");
#endif
		}
	}
#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	if (opcode == HCI_OP_LE_CREATE_CIS) {
		/* TODO: Should we remove the cis conn?
		 * Will an HCI_LE_CIS_Established event be generated for each
		 * CIS no matter the following status is 0 or non-zero?
		 */
		if (status) {
			RTKBT_ERR("cs of HCI_OP_LE_CREATE_CIS");
		}
	}
#endif
}

static void rtk_handle_connection_complete_evt(u8 * p)
{
	u16 handle;
	u8 status, link_type;
	rtk_conn_prof *hci_conn = NULL;

	status = *p++;
	STREAM_TO_UINT16(handle, p);
	p += 6;
	link_type = *p++;

	RTKBT_INFO("connected, handle %04x, status 0x%02x", handle, status);

	if (status == 0) {
		if (btrtl_coex.ispaging) {
			btrtl_coex.ispaging = 0;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("notify wifi page success end");
			rtk_notify_btoperation_to_wifi
			    (BT_OPCODE_PAGE_SUCCESS_END, 0, NULL);
#else
			RTKBT_INFO("Page success");
#endif
		}

		hci_conn = find_connection_by_handle(&btrtl_coex, handle);
		if (hci_conn == NULL) {
			hci_conn = allocate_connection_by_handle(handle);
			if (hci_conn) {
				hci_conn->profile_bitmap = 0;
				hci_conn->profile_status = 0;
				memset(hci_conn->profile_refcount, 0, 8);
				hci_conn->type = HCI_CONN_TYPE_BREDR_ACL;

				add_connection_to_hash(&btrtl_coex, hci_conn);
				if (0 == link_type || 2 == link_type) {
					/* sco or esco */
					hci_conn->type = HCI_CONN_TYPE_SCO;
					update_profile_connection(hci_conn,
								  profile_sco,
								  TRUE);
				}
			} else {
				RTKBT_ERR("hci connection allocate fail");
			}
		} else {
			RTKBT_DBG("hci conn handle 0x%04x already existed!",
				  handle);
			hci_conn->profile_bitmap = 0;
			memset(hci_conn->profile_refcount, 0, 8);
			if (0 == link_type || 2 == link_type) {
				/* sco or esco */
				hci_conn->type = HCI_CONN_TYPE_SCO;
				update_profile_connection(hci_conn, profile_sco,
							  TRUE);
			} else {
				hci_conn->type = HCI_CONN_TYPE_BREDR_ACL;
			}
		}
	} else if (btrtl_coex.ispaging) {
		btrtl_coex.ispaging = 0;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("notify wifi page unsuccess end");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_PAGE_UNSUCCESS_END, 0,
					       NULL);
#else
		RTKBT_INFO("Page failed");
#endif
	}
}

static void rtk_handle_le_connection_complete_evt(u8 enhanced, u8 * p)
{
	u16 handle, interval;
	u8 status;
	rtk_conn_prof *hci_conn = NULL;

	status = *p++;
	STREAM_TO_UINT16(handle, p);
	if (!enhanced)
		p += 8;	/* role, address type, address */
	else
		p += (8 + 12); /* plus two bluetooth addresses */
	STREAM_TO_UINT16(interval, p);

	RTKBT_INFO("LE connected, handle %04x, status 0x%02x, interval %u",
		   handle, status, interval);

	if (status == 0) {
		if (btrtl_coex.ispaging) {
			btrtl_coex.ispaging = 0;
#ifdef RTB_SOFTWARE_MAILBOX
			RTKBT_DBG("notify wifi page success end");
			rtk_notify_btoperation_to_wifi
			    (BT_OPCODE_PAGE_SUCCESS_END, 0, NULL);
#else
			RTKBT_INFO("Page success end");
#endif
		}

		hci_conn = find_connection_by_handle(&btrtl_coex, handle);
		if (hci_conn == NULL) {
			hci_conn = allocate_connection_by_handle(handle);
			if (hci_conn) {
				hci_conn->profile_bitmap = 0;
				hci_conn->profile_status = 0;
				memset(hci_conn->profile_refcount, 0, 8);
				hci_conn->type = HCI_CONN_TYPE_LE_ACL;

				add_connection_to_hash(&btrtl_coex, hci_conn);

				update_profile_connection(hci_conn, profile_hid, TRUE);	//for coex, le is the same as hid
				update_hid_active_state(handle, interval);
			} else {
				RTKBT_ERR("hci connection allocate fail");
			}
		} else {
			RTKBT_DBG("hci conn handle 0x%04x already existed!",
				  handle);
			hci_conn->profile_bitmap = 0;
			memset(hci_conn->profile_refcount, 0, 8);
			hci_conn->type = HCI_CONN_TYPE_LE_ACL;
			update_profile_connection(hci_conn, profile_hid, TRUE);
			update_hid_active_state(handle, interval);
		}
	} else if (btrtl_coex.ispaging) {
		btrtl_coex.ispaging = 0;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("notify wifi page unsuccess end");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_PAGE_UNSUCCESS_END, 0,
					       NULL);
#else
		RTKBT_INFO("Page failed");
#endif
	}
}

static void rtk_handle_le_connection_update_complete_evt(u8 * p)
{
	u16 handle, interval;
	/* u8 status; */

	/* status = *p++; */
	p++;

	STREAM_TO_UINT16(handle, p);
	STREAM_TO_UINT16(interval, p);
	update_hid_active_state(handle, interval);
}

#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
static void rtk_handle_le_cis_established_evt(void * p)
{
	struct hci_evt_le_cis_established *ev = p;
	u8 status;
	u16 c_to_p, p_to_c;
	rtk_conn_prof *hci_conn;
	u16 handle = __le16_to_cpu(ev->handle);
	u8 central = 1;;

	status = ev->status;
	c_to_p = le16_to_cpu(ev->c_mtu);
	p_to_c = le16_to_cpu(ev->p_mtu);

	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (status) {
		RTKBT_ERR("cis established evt err, status 0x%02x", status);
		if (hci_conn)
			delete_connection_from_hash(hci_conn);
		goto done;
	}

	if (!hci_conn) {
		/* We're peripheral */
		central = 0;
		hci_conn = allocate_connection_by_handle(handle);
		if (!hci_conn) {
			RTKBT_ERR("cis conn (0x%04x) allocation failed", handle);
			goto done;
		}
		hci_conn->profile_bitmap = 0;
		hci_conn->profile_status = 0;
		memset(hci_conn->profile_refcount, 0, profile_max);
		hci_conn->type = HCI_CONN_TYPE_CIS;
		add_connection_to_hash(&btrtl_coex, hci_conn);
	}

	RTKBT_DBG("central %u, c_to_p 0x%04x, p_to_c 0x%04x", central, c_to_p,
		  p_to_c);

	if ((central && c_to_p) || (!central && p_to_c))
		update_profile_connection(hci_conn, profile_lea_src, TRUE);

	if ((central && p_to_c) || (!central && c_to_p))
		update_profile_connection(hci_conn, profile_lea_snk, TRUE);

done:
	return;
}

static void big_conn_add(u16 big_handle, u16 handle, u8 profile_index)
{
	rtk_conn_prof *hci_conn = NULL;

	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (hci_conn) {
		RTKBT_DBG("bis handle 0x%04x has already existed!", handle);
		return;
	}
	RTKBT_DBG("bis handle 0x%04x", handle);
	hci_conn = allocate_connection_by_handle(handle);
	if (hci_conn) {
		hci_conn->profile_bitmap = 0;
		hci_conn->profile_status = 0;
		hci_conn->big_handle = big_handle;
		memset(hci_conn->profile_refcount, 0, profile_max);
		hci_conn->type = HCI_CONN_TYPE_BIS;

		add_connection_to_hash(&btrtl_coex, hci_conn);

		update_profile_connection(hci_conn, profile_index, TRUE);
	} else {
		RTKBT_DBG("bis conn allocation failed");
	}
}

static void rtk_handle_le_create_big_complete_evt(void *p)
{
	struct hci_evt_le_create_big_complete *ev = p;
	u16 big_handle;
	u8 status;

	status = ev->status;
	if (status) {
		RTKBT_ERR("big complete evt err, status 0x%02x", status);
		return;
	}
	big_handle = ev->handle;
	big_handle += HCI_CONN_HANDLE_UNSET_START;
	for (u8 i = 0; i < ev->num_bis; i++) {
		u16 handle = le16_to_cpu(ev->bis_handle[i]);
		big_conn_add(big_handle, handle, profile_lea_src);
	}
}

static void rtk_handle_le_terminate_big_complete_evt(u8 * p)
{
	u16 big_handle;

	big_handle = *p++;
	big_handle += HCI_CONN_HANDLE_UNSET_START;

	big_conn_remove(big_handle);
}

static void rtk_handle_le_big_sync_established_evt(void * p)
{
	struct hci_evt_le_big_sync_estabilished *ev = p;
	u8 status;
	u16 big_handle;
	u16 bis_handle;

	status = ev->status;
	big_handle = ev->handle;
	big_handle += HCI_CONN_HANDLE_UNSET_START;
	if (status) {
		RTKBT_ERR("big complete evt err, status 0x%02x", status);
		return;
	}

	for (u8 i = 0; i < ev->num_bis; i++) {
		bis_handle = le16_to_cpu(ev->bis[i]);
		big_conn_add(big_handle, bis_handle, profile_lea_snk);
	}
}

static void rtk_handle_le_big_sync_lost_evt(u8 * p)
{
	u16 big_handle;

	big_handle = *p++;
	big_handle += HCI_CONN_HANDLE_UNSET_START;

	big_conn_remove(big_handle);
}

#endif

static void rtk_handle_le_meta_evt(u8 * p)
{
	u8 sub_event = *p++;

	switch (sub_event) {
	case HCI_EV_LE_CONN_COMPLETE:
		rtk_handle_le_connection_complete_evt(0, p);
		break;
	case HCI_EV_LE_ENHANCED_CONN_COMPLETE:
		rtk_handle_le_connection_complete_evt(1, p);
		break;
	case HCI_EV_LE_CONN_UPDATE_COMPLETE:
		rtk_handle_le_connection_update_complete_evt(p);
		break;
#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	case HCI_EV_LE_CIS_EST:
		rtk_handle_le_cis_established_evt(p);
		break;
	case HCI_EV_LE_CREATE_BIG_CPL:
		rtk_handle_le_create_big_complete_evt(p);
		break;
	case HCI_EV_LE_TERM_BIG_CPL:
		rtk_handle_le_terminate_big_complete_evt(p);
		break;
	case HCI_EV_LE_BIG_SYNC_EST:
		rtk_handle_le_big_sync_established_evt(p);
		break;
	case HCI_EV_LE_BIG_SYNC_LOST:
		rtk_handle_le_big_sync_lost_evt(p);
		break;
#endif
	default:
		break;
	}
}

static u8 disconn_profile(struct rtl_hci_conn *conn, u8 pfe_index)
{
	u8 need_update = 0;

	if (!conn->profile_refcount[pfe_index]) {
		RTKBT_WARN("profile %u ref is 0", pfe_index);
		return 0;
	}

	RTKBT_INFO("%s: profile_ref[%u] %u", __func__, pfe_index,
		  conn->profile_refcount[pfe_index]);

	if (conn->profile_refcount[pfe_index])
		conn->profile_refcount[pfe_index]--;
	else
		RTKBT_INFO("%s: conn pfe ref[%u] is 0", __func__,
			   conn->profile_refcount[pfe_index]);
	if (!conn->profile_refcount[pfe_index]) {
		need_update = 1;
		conn->profile_bitmap &= ~(BIT(pfe_index));

		/* if profile does not exist, status is meaningless */
		conn->profile_status &= ~(BIT(pfe_index));
		rtk_check_del_timer(pfe_index, conn);
	}

	return need_update;
}

static void disconn_acl(u16 handle, struct rtl_hci_conn *conn)
{
	struct rtl_coex_struct *coex = &btrtl_coex;
	rtk_prof_info *prof_info = NULL;
	struct list_head *iter = NULL, *temp = NULL;
	u8 need_update = 0;

	mutex_lock(&coex->profile_mutex);

	list_for_each_safe(iter, temp, &coex->profile_list) {
		prof_info = list_entry(iter, rtk_prof_info, list);
		if (handle == prof_info->handle) {
			RTKBT_DBG("hci disconn, hndl %x, psm %x, dcid %x, "
				  "scid %x, profile %u", prof_info->handle,
				  prof_info->psm, prof_info->dcid,
				  prof_info->scid, prof_info->profile_index);
			//If both scid and dcid > 0, L2cap connection is exist.
			need_update |= disconn_profile(conn,
						      prof_info->profile_index);
			if ((prof_info->flags & A2DP_MEDIA) &&
			    (conn->profile_bitmap & BIT(profile_sink)))
				need_update |= disconn_profile(conn,
							       profile_sink);
			delete_profile_from_hash(prof_info);
		}
	}
	if (need_update)
		rtk_notify_profileinfo_to_fw();
	mutex_unlock(&coex->profile_mutex);
}

static void rtk_handle_disconnect_complete_evt(u8 * p)
{
	u16 handle;
	u8 status;
	u8 reason;
	rtk_conn_prof *hci_conn = NULL;

	if (btrtl_coex.ispairing) {	//for slave: connection will be disconnected if authentication fail
		btrtl_coex.ispairing = 0;
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("hci disc complete, notify wifi pair end");
		rtk_notify_btoperation_to_wifi(BT_OPCODE_PAIR_END, 0, NULL);
#else
		RTKBT_INFO("hci disconnection complete");
#endif
	}

	status = *p++;
	STREAM_TO_UINT16(handle, p);
	reason = *p;

	RTKBT_INFO("disconn cmpl evt: status %02x, handle %04x, reason %02x",
		   status, handle, reason);

	if (status == 0) {
		RTKBT_DBG("process disconn complete event.");
		hci_conn = find_connection_by_handle(&btrtl_coex, handle);
		if (!hci_conn) {
			RTKBT_ERR("hci conn handle 0x%04x not found", handle);
			return;
		}
		switch (hci_conn->type) {
		case HCI_CONN_TYPE_BREDR_ACL:
			/* FIXME: If this is interrupted by l2cap rx,
			 * there may be deadlock on profile_mutex */
			disconn_acl(handle, hci_conn);
			break;

		case HCI_CONN_TYPE_SCO:
			update_profile_connection(hci_conn, profile_sco, FALSE);
			break;

		case HCI_CONN_TYPE_LE_ACL:
			update_profile_connection(hci_conn, profile_hid, FALSE);
			break;

		case HCI_CONN_TYPE_CIS:
			if (hci_conn->direction & HCI_DATA_PATH_INPUT)
				update_profile_connection(hci_conn,
							  profile_lea_src,
							  FALSE);
			if (hci_conn->direction & HCI_DATA_PATH_OUTPUT)
				update_profile_connection(hci_conn,
							  profile_lea_snk,
							  FALSE);
			break;

		default:
			break;
		}
		delete_connection_from_hash(hci_conn);
	}
}

static void rtk_handle_specific_evt(u8 * p)
{
	u16 subcode;

	STREAM_TO_UINT16(subcode, p);
	if (subcode == HCI_VENDOR_PTA_AUTO_REPORT_EVENT) {
#ifdef RTB_SOFTWARE_MAILBOX
		RTKBT_DBG("notify wifi driver with autoreport data");
		rtk_notify_info_to_wifi(AUTO_REPORT, RTL_BTINFO_LEN,
			(uint8_t *)p);
#else
		RTKBT_INFO("auto report data");
#endif
	}
}

static void rtk_parse_event_data(struct rtl_coex_struct *coex,
		u8 *data, u16 len)
{
	u8 *p = data;
	u8 event_code = *p++;
	u8 total_len = *p++;

	(void)coex;
	(void)&len;

	switch (event_code) {
	case HCI_EV_INQUIRY_COMPLETE:
		rtk_handle_inquiry_complete();
		break;

	case HCI_EV_PIN_CODE_REQ:
		rtk_handle_pin_code_req();
		break;

	case HCI_EV_IO_CAPA_REQUEST:
		rtk_handle_io_capa_req();
		break;

	case HCI_EV_AUTH_COMPLETE:
		rtk_handle_auth_request();
		break;

	case HCI_EV_LINK_KEY_NOTIFY:
		rtk_handle_link_key_notify();
		break;

	case HCI_EV_MODE_CHANGE:
		rtk_handle_mode_change_evt(p);
		break;

	case HCI_EV_CMD_COMPLETE:
		rtk_handle_cmd_complete_evt(total_len, p);
		break;

	case HCI_EV_CMD_STATUS:
		rtk_handle_cmd_status_evt(p);
		break;

	case HCI_EV_CONN_COMPLETE:
	case HCI_EV_SYNC_CONN_COMPLETE:
		rtk_handle_connection_complete_evt(p);
		break;

	case HCI_EV_DISCONN_COMPLETE:
		rtk_handle_disconnect_complete_evt(p);
		break;

	case HCI_EV_LE_META:
		rtk_handle_le_meta_evt(p);
		break;

	case HCI_EV_VENDOR_SPECIFIC:
		rtk_handle_specific_evt(p);
		break;

	default:
		break;
	}
}

static const char l2_dir_str[][4] = {
	"RX", "TX",
};

static void rtl_process_l2_sig(struct rtl_l2_buff *l2)
{
	/* u8 flag; */
	u8 code;
	/* u8 identifier; */
	u16 handle;
	/* u16 total_len; */
	/* u16 pdu_len, channel_id; */
	/* u16 command_len; */
	u16 psm, scid, dcid, result;
	/* u16 status; */
	u8 *pp = l2->data;

	STREAM_TO_UINT16(handle, pp);
	/* flag = handle >> 12; */
	handle = handle & 0x0FFF;
	/* STREAM_TO_UINT16(total_len, pp); */
	pp += 2; /* data total length */

	/* STREAM_TO_UINT16(pdu_len, pp);
	 * STREAM_TO_UINT16(channel_id, pp); */
	pp += 4; /* l2 len and channel id */

	code = *pp++;
	switch (code) {
	case L2CAP_CONN_REQ:
		/* identifier = *pp++; */
		pp++;
		/* STREAM_TO_UINT16(command_len, pp); */
		pp += 2;
		STREAM_TO_UINT16(psm, pp);
		STREAM_TO_UINT16(scid, pp);
		RTKBT_DBG("%s l2cap conn req, hndl 0x%04x, PSM 0x%04x, "
			  "scid 0x%04x", l2_dir_str[l2->out], handle, psm,
			  scid);
		handle_l2cap_con_req(handle, psm, scid, l2->out);
		break;

	case L2CAP_CONN_RSP:
		/* identifier = *pp++; */
		pp++;
		/* STREAM_TO_UINT16(command_len, pp); */
		pp += 2;
		STREAM_TO_UINT16(dcid, pp);
		STREAM_TO_UINT16(scid, pp);
		STREAM_TO_UINT16(result, pp);
		/* STREAM_TO_UINT16(status, pp); */
		pp += 2;
		RTKBT_DBG("%s l2cap conn rsp, hndl 0x%04x, dcid 0x%04x, "
			  "scid 0x%04x, result 0x%04x", l2_dir_str[l2->out],
			  handle, dcid, scid, result);
		handle_l2cap_con_rsp(handle, dcid, scid, l2->out, result);
		break;

	case L2CAP_DISCONN_REQ:
		/* identifier = *pp++; */
		pp++;
		/* STREAM_TO_UINT16(command_len, pp); */
		pp += 2;
		STREAM_TO_UINT16(dcid, pp);
		STREAM_TO_UINT16(scid, pp);
		RTKBT_DBG("%s l2cap disconn req, hndl 0x%04x, dcid 0x%04x, "
			  "scid 0x%04x", l2_dir_str[l2->out], handle, dcid, scid);
		handle_l2cap_discon_req(handle, dcid, scid, l2->out);
		break;
	default:
		RTKBT_DBG("undesired l2 command %u", code);
		break;
	}
}

static void rtl_l2_data_process(u8 *pp, u16 len, int dir)
{
	u8 code;
	u8 flag;
	u16 handle, pdu_len, channel_id;
	/* u16 total_len; */
	struct rtl_l2_buff *l2 = NULL;
	u8 *hd = pp;

	/* RTKBT_DBG("l2 sig data %p, len %u, dir %d", pp, len, dir); */

	STREAM_TO_UINT16(handle, pp);
	flag = handle >> 12;
	handle = handle & 0x0FFF;
	/* STREAM_TO_UINT16(total_len, pp); */
	pp += 2; /* data total length */

	STREAM_TO_UINT16(pdu_len, pp);
	STREAM_TO_UINT16(channel_id, pp);

	if (channel_id == 0x0001) {
		code = *pp++;
		switch (code) {
		case L2CAP_CONN_REQ:
		case L2CAP_CONN_RSP:
		case L2CAP_DISCONN_REQ:
			RTKBT_DBG("l2cap op %u, len %u, out %d", code, len,
				  dir);
			l2 = rtl_l2_node_get(&btrtl_coex);
			if (l2) {
				u16 n;
				n = min_t(uint, len, L2_MAX_SUBSEC_LEN);
				memcpy(l2->data, hd, n);
				l2->out = dir;
				rtl_l2_node_to_used(&btrtl_coex, l2);
				queue_delayed_work(btrtl_coex.fw_wq,
						&btrtl_coex.fw_work, 0);
			} else
				RTKBT_ERR("%s: failed to get l2 node",
					  __func__);
			break;
		case L2CAP_DISCONN_RSP:
			break;
		default:
			break;
		}
	} else {
		//RTKBT_DBG("%s: handle:%x, flag:%x, pan:%d, a2dp:%d", __func__, handle, flag,
		//	is_profile_connected(profile_a2dp), is_profile_connected(profile_pan));
		if (flag != 0x01)
			packets_count(handle, channel_id, dir, hd, len);
	}
	return;
}

static void rtl_process_cmd(struct rtl_coex_struct *coex, u8 *buffer, int count)
{
	struct hci_command_hdr *hdr;
	u16 opcode;

	hdr = (void *)buffer;
	opcode = get_unaligned_le16(&hdr->opcode);
	switch (opcode) {
#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	case HCI_OP_LE_CREATE_CIS:
		rtk_btcoex_handle_cmd_le_create_cis(buffer, count);
		break;
	case HCI_OP_LE_SETUP_ISO_PATH:
		rtk_btcoex_handle_cmd_le_setup_iso_path(buffer, count);
		break;
	case HCI_OP_LE_REMOVE_ISO_PATH:
		rtk_btcoex_handle_cmd_le_remove_iso_path(buffer, count);
		break;
#endif
	default:
		break;
	}
}

static void rtl_process_l2_data(struct rtl_l2_buff *l2)
{
	rtk_prof_info *prof = NULL;
	rtk_conn_prof *hci_conn = NULL;
	struct hci_acl_hdr *hdr;
	u16 handle;
	struct l2cap_hdr *l2hdr;
	u16 scid;
	u8 out;
	u16 dlen;

	hdr = (void *)l2->data;
	handle = get_unaligned_le16(&hdr->handle);
	handle &= 0x0fff;
	dlen = get_unaligned_le16(&hdr->dlen);
	hci_conn = find_connection_by_handle(&btrtl_coex, handle);
	if (!hci_conn)
		goto done;
	if (hci_conn->type != HCI_CONN_TYPE_BREDR_ACL)
		goto done;

	if (l2->type == HCI_PT_L2DATA_TX)
		out = 1;
	else
		out = 0;

	l2hdr = (void *)(l2->data + sizeof(*hdr));
	scid = get_unaligned_le16(&l2hdr->cid);
	if (!out)
		prof = find_profile_by_handle_scid(&btrtl_coex, handle, scid);
	else
		prof = find_profile_by_handle_dcid(&btrtl_coex, handle, scid);
	if (!prof)
		goto done;

	if (prof->profile_index != profile_a2dp || prof->flags != A2DP_MEDIA ||
	    hci_conn->profile_status & BIT(profile_a2dp))
		goto done;

	rtl_process_media_data(hci_conn, l2->data, sizeof(*hdr) + dlen, out);

done:
	return;
}

static void rtl_hci_work_func(struct work_struct *work)
{
	struct rtl_coex_struct *coex;
	struct rtl_hci_ev *ev;
	struct rtl_l2_buff *l2;
	unsigned long flags;
	struct rtl_hci_hdr *hdr;
	u8 pkt_type;

	coex = container_of(work, struct rtl_coex_struct, fw_work.work);

	spin_lock_irqsave(&coex->buff_lock, flags);
	while (!list_empty(&coex->hci_pkt_list)) {
		hdr = list_entry(coex->hci_pkt_list.next, struct rtl_hci_hdr,
				 list);
		list_del(&hdr->list);
		pkt_type = hdr->type;

		spin_unlock_irqrestore(&coex->buff_lock, flags);

		switch (pkt_type) {
		case HCI_PT_EVT:
			ev = (void *)hdr;
			rtk_parse_event_data(coex, ev->data, ev->len);
			break;
		case HCI_PT_CMD:
			ev = (void *)hdr;
			rtl_process_cmd(coex, ev->data, ev->len);
			break;
		case HCI_PT_L2SIG_RX:
		case HCI_PT_L2SIG_TX:
			l2 = (void *)hdr;
			rtl_process_l2_sig(l2);
			break;
		case HCI_PT_L2DATA_RX:
		case HCI_PT_L2DATA_TX:
			l2 = (void *)hdr;
			rtl_process_l2_data(l2);
			break;
		default:
			break;
		}

		spin_lock_irqsave(&coex->buff_lock, flags);

		hdr->type = 0;
		switch (pkt_type) {
		case HCI_PT_EVT:
		case HCI_PT_CMD:
			list_add_tail(&hdr->list, &coex->ev_free_list);
			break;
		case HCI_PT_L2SIG_RX:
		case HCI_PT_L2SIG_TX:
		case HCI_PT_L2DATA_RX:
		case HCI_PT_L2DATA_TX:
			list_add_tail(&hdr->list, &coex->l2_free_list);
			break;
		default:
			RTKBT_ERR("%s: invalid pkt type %u", __func__,
				  hdr->type);
			list_add_tail(&hdr->list, &coex->ev_free_list);
			break;
		}
	}
	spin_unlock_irqrestore(&coex->buff_lock, flags);
}

static inline int cmd_cmplt_filter_out(u8 *buf)
{
	u16 opcode;

	opcode = buf[3] | (buf[4] << 8);
	switch (opcode) {
	case HCI_OP_PERIODIC_INQ:
	case HCI_OP_READ_LOCAL_VERSION:
#ifdef RTB_SOFTWARE_MAILBOX
	case HCI_VENDOR_MAILBOX_CMD:
#endif
	case HCI_VENDOR_SET_PROFILE_REPORT_COMMAND:
#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	case HCI_OP_LE_SETUP_ISO_PATH:
	case HCI_OP_LE_BIG_TERM_SYNC:
#endif
		return 0;
	default:
		return 1;
	}
}

static inline int cmd_status_filter_out(u8 *buf)
{
	u16 opcode;

	opcode = buf[4] | (buf[5] << 8);
	switch (opcode) {
	case HCI_OP_INQUIRY:
	case HCI_OP_CREATE_CONN:
#if HCI_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	case HCI_OP_LE_CREATE_CIS:
#endif
		return 0;
	default:
		return 1;
	}
}

static int ev_filter_out(u8 *buf)
{
	switch (buf[0]) {
	case HCI_EV_INQUIRY_COMPLETE:
	case HCI_EV_PIN_CODE_REQ:
	case HCI_EV_IO_CAPA_REQUEST:
	case HCI_EV_AUTH_COMPLETE:
	case HCI_EV_LINK_KEY_NOTIFY:
	case HCI_EV_MODE_CHANGE:
	case HCI_EV_CONN_COMPLETE:
	case HCI_EV_SYNC_CONN_COMPLETE:
	case HCI_EV_DISCONN_COMPLETE:
	case HCI_EV_VENDOR_SPECIFIC:
		return 0;
	case HCI_EV_LE_META:
		/* Ignore frequent but not useful events that result in
		 * costing too much space.
		 */
		switch (buf[2]) {
		case HCI_EV_LE_CONN_COMPLETE:
		case HCI_EV_LE_ENHANCED_CONN_COMPLETE:
		case HCI_EV_LE_CONN_UPDATE_COMPLETE:
		case HCI_EV_LE_CIS_EST:
		case HCI_EV_LE_CREATE_BIG_CPL:
		case HCI_EV_LE_TERM_BIG_CPL:
		case HCI_EV_LE_BIG_SYNC_EST:
		case HCI_EV_LE_BIG_SYNC_LOST:
			return 0;
		}
		return 1;
	case HCI_EV_CMD_COMPLETE:
		return cmd_cmplt_filter_out(buf);
	case HCI_EV_CMD_STATUS:
		return cmd_status_filter_out(buf);
	default:
		return 1;
	}
}

static void rtk_btcoex_evt_enqueue(__u8 *s, __u16 count)
{
	struct rtl_hci_ev *ev;
	if (ev_filter_out(s))
		return;

	ev = rtl_ev_node_get(&btrtl_coex);
	if (!ev) {
		RTKBT_ERR("%s: no free ev node.", __func__);
		return;
	}

	if (count > MAX_LEN_OF_HCI_EV) {
		memcpy(ev->data, s, MAX_LEN_OF_HCI_EV);
		ev->len = MAX_LEN_OF_HCI_EV;
	} else {
		memcpy(ev->data, s, count);
		ev->len = count;
	}

	rtl_ev_node_to_used(&btrtl_coex, ev);

	queue_delayed_work(btrtl_coex.fw_wq, &btrtl_coex.fw_work, 0);
}

/* Context: in_interrupt() */
void rtk_btcoex_parse_event(uint8_t *buffer, int count)
{
	struct rtl_coex_struct *coex = &btrtl_coex;
	__u8 *tbuff;
	__u16 elen = 0;

	/* RTKBT_DBG("%s: parse ev.", __func__); */
	if (!test_bit(RTL_COEX_RUNNING, &btrtl_coex.flags)) {
		/* RTKBT_INFO("%s: Coex is closed, ignore", __func__); */
		RTKBT_INFO("%s: Coex is closed, ignore %x, %x",
			   __func__, buffer[0], buffer[1]);
		return;
	}

	spin_lock(&coex->rxlock);

	/* coex->tbuff will be set to NULL when initializing or
	 * there is a complete frame or there is start of a frame */
	tbuff = coex->tbuff;

	while (count) {
		int len;

		/* Start of a frame */
		if (!tbuff) {
			tbuff = coex->back_buff;
			coex->tbuff = NULL;
			coex->elen = 0;

			coex->pkt_type = HCI_EVENT_PKT;
			coex->expect = HCI_EVENT_HDR_SIZE;
		}

		len = min_t(uint, coex->expect, count);
		memcpy(tbuff, buffer, len);
		tbuff += len;
		coex->elen += len;

		count -= len;
		buffer += len;
		coex->expect -= len;

		if (coex->elen == HCI_EVENT_HDR_SIZE) {
			/* Complete event header */
			coex->expect =
				((struct hci_event_hdr *)coex->back_buff)->plen;
			if (coex->expect > HCI_MAX_EVENT_SIZE - coex->elen) {
				tbuff = NULL;
				coex->elen = 0;
				RTKBT_ERR("tbuff room is not enough");
				break;
			}
		}

		if (coex->expect == 0) {
			/* Complete frame */
			elen = coex->elen;
			spin_unlock(&coex->rxlock);
			rtk_btcoex_evt_enqueue(coex->back_buff, elen);
			spin_lock(&coex->rxlock);

			tbuff = NULL;
			coex->elen = 0;
		}
	}

	/* coex->tbuff would be non-NULL if there isn't a complete frame
	 * And it will be updated next time */
	coex->tbuff = tbuff;
	spin_unlock(&coex->rxlock);
}


void rtk_btcoex_parse_l2cap_data_tx(uint8_t *buffer, int count)
{
	if (!test_bit(RTL_COEX_RUNNING, &btrtl_coex.flags)) {
		RTKBT_INFO("%s: Coex is closed, ignore", __func__);
		return;
	}

	rtl_l2_data_process(buffer, count, 1);
	//u16 handle, total_len, pdu_len, channel_ID, command_len, psm, scid,
	//    dcid, result, status;
	//u8 flag, code, identifier;
	//u8 *pp = (u8 *) (skb->data);
	//STREAM_TO_UINT16(handle, pp);
	//flag = handle >> 12;
	//handle = handle & 0x0FFF;
	//STREAM_TO_UINT16(total_len, pp);
	//STREAM_TO_UINT16(pdu_len, pp);
	//STREAM_TO_UINT16(channel_ID, pp);

	//if (channel_ID == 0x0001) {
	//	code = *pp++;
	//	switch (code) {
	//	case L2CAP_CONN_REQ:
	//		identifier = *pp++;
	//		STREAM_TO_UINT16(command_len, pp);
	//		STREAM_TO_UINT16(psm, pp);
	//		STREAM_TO_UINT16(scid, pp);
	//		RTKBT_DBG("TX l2cap conn req, hndl %x, PSM %x, scid=%x",
	//			  handle, psm, scid);
	//		handle_l2cap_con_req(handle, psm, scid, 1);
	//		break;

	//	case L2CAP_CONN_RSP:
	//		identifier = *pp++;
	//		STREAM_TO_UINT16(command_len, pp);
	//		STREAM_TO_UINT16(dcid, pp);
	//		STREAM_TO_UINT16(scid, pp);
	//		STREAM_TO_UINT16(result, pp);
	//		STREAM_TO_UINT16(status, pp);
	//		RTKBT_DBG("TX l2cap conn rsp, hndl %x, dcid %x, "
	//			  "scid %x, result %x",
	//			  handle, dcid, scid, result);
	//		handle_l2cap_con_rsp(handle, dcid, scid, 1, result);
	//		break;

	//	case L2CAP_DISCONN_REQ:
	//		identifier = *pp++;
	//		STREAM_TO_UINT16(command_len, pp);
	//		STREAM_TO_UINT16(dcid, pp);
	//		STREAM_TO_UINT16(scid, pp);
	//		RTKBT_DBG("TX l2cap disconn req, hndl %x, dcid %x, "
	//			  "scid %x", handle, dcid, scid);
	//		handle_l2cap_discon_req(handle, dcid, scid, 1);
	//		break;

	//	case L2CAP_DISCONN_RSP:
	//		break;

	//	default:
	//		break;
	//	}
	//} else {
	//	if ((flag != 0x01) && (is_profile_connected(profile_a2dp) || is_profile_connected(profile_pan)))	//Do not count the continuous packets
	//		packets_count(handle, channel_ID, pdu_len, 1, pp);
	//}
}

void rtk_btcoex_parse_l2cap_data_rx(uint8_t *buffer, int count)
{
	if (!test_bit(RTL_COEX_RUNNING, &btrtl_coex.flags)) {
		RTKBT_INFO("%s: Coex is closed, ignore", __func__);
		return;
	}

	rtl_l2_data_process(buffer, count, 0);
	//u16 handle, total_len, pdu_len, channel_ID, command_len, psm, scid,
	//    dcid, result, status;
	//u8 flag, code, identifier;
	//u8 *pp = urb->transfer_buffer;
	//STREAM_TO_UINT16(handle, pp);
	//flag = handle >> 12;
	//handle = handle & 0x0FFF;
	//STREAM_TO_UINT16(total_len, pp);
	//STREAM_TO_UINT16(pdu_len, pp);
	//STREAM_TO_UINT16(channel_ID, pp);

	//if (channel_ID == 0x0001) {
	//	code = *pp++;
	//	switch (code) {
	//	case L2CAP_CONN_REQ:
	//		identifier = *pp++;
	//		STREAM_TO_UINT16(command_len, pp);
	//		STREAM_TO_UINT16(psm, pp);
	//		STREAM_TO_UINT16(scid, pp);
	//		RTKBT_DBG("RX l2cap conn req, hndl %x, PSM %x, scid %x",
	//			  handle, psm, scid);
	//		handle_l2cap_con_req(handle, psm, scid, 0);
	//		break;

	//	case L2CAP_CONN_RSP:
	//		identifier = *pp++;
	//		STREAM_TO_UINT16(command_len, pp);
	//		STREAM_TO_UINT16(dcid, pp);
	//		STREAM_TO_UINT16(scid, pp);
	//		STREAM_TO_UINT16(result, pp);
	//		STREAM_TO_UINT16(status, pp);
	//		RTKBT_DBG("RX l2cap conn rsp, hndl %x, dcid %x, "
	//			  "scid %x, result %x",
	//			  handle, dcid, scid, result);
	//		handle_l2cap_con_rsp(handle, dcid, scid, 0, result);
	//		break;

	//	case L2CAP_DISCONN_REQ:
	//		identifier = *pp++;
	//		STREAM_TO_UINT16(command_len, pp);
	//		STREAM_TO_UINT16(dcid, pp);
	//		STREAM_TO_UINT16(scid, pp);
	//		RTKBT_DBG("RX l2cap disconn req, hndl %x, dcid %x, "
	//			  "scid %x", handle, dcid, scid);
	//		handle_l2cap_discon_req(handle, dcid, scid, 0);
	//		break;

	//	case L2CAP_DISCONN_RSP:
	//		break;

	//	default:
	//		break;
	//	}
	//} else {
	//	if ((flag != 0x01) && (is_profile_connected(profile_a2dp) || is_profile_connected(profile_pan)))	//Do not count the continuous packets
	//		packets_count(handle, channel_ID, pdu_len, 0, pp);
	//}
}

#ifdef RTB_SOFTWARE_MAILBOX

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
static void polling_bt_info(struct timer_list *unused)
#else
static void polling_bt_info(unsigned long data)
#endif
{
	uint8_t temp_cmd[1];
	RTKBT_DBG("polling timer");
	if (btrtl_coex.polling_enable) {
		//temp_cmd[0] = HCI_VENDOR_SUB_CMD_BT_REPORT_CONN_SCO_INQ_INFO;
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_BT_AUTO_REPORT_STATUS_INFO;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 1, temp_cmd);
	}
	mod_timer(&btrtl_coex.polling_timer,
		  jiffies + msecs_to_jiffies(1000 * btrtl_coex.polling_interval));
}

static void rtk_handle_bt_info_control(uint8_t *p)
{
	uint8_t temp_cmd[20];
	struct rtl_btinfo_ctl *ctl = (struct rtl_btinfo_ctl*)p;
	RTKBT_DBG("Received polling_enable %u, polling_time %u, "
		  "autoreport_enable %u", ctl->polling_enable,
		  ctl->polling_time, ctl->autoreport_enable);
	RTKBT_DBG("coex: original polling_enable %u",
		  btrtl_coex.polling_enable);

	if (ctl->polling_enable && !btrtl_coex.polling_enable) {
		/* setup polling timer for getting bt info from firmware */
		btrtl_coex.polling_timer.expires =
		    jiffies + msecs_to_jiffies(ctl->polling_time * 1000);
		mod_timer(&btrtl_coex.polling_timer,
			  btrtl_coex.polling_timer.expires);
	}

	/* Close bt info polling timer */
	if (!ctl->polling_enable && btrtl_coex.polling_enable)
		del_timer(&btrtl_coex.polling_timer);

	if (btrtl_coex.autoreport != ctl->autoreport_enable) {
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_BT_AUTO_REPORT_ENABLE;
		temp_cmd[1] = 1;
		temp_cmd[2] = ctl->autoreport_enable;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 3, temp_cmd);
	}

	btrtl_coex.polling_enable = ctl->polling_enable;
	btrtl_coex.polling_interval = ctl->polling_time;
	btrtl_coex.autoreport = ctl->autoreport_enable;

	rtk_notify_info_to_wifi(HOST_RESPONSE, 0, NULL);
}

static void rtk_handle_bt_coex_control(uint8_t * p)
{
	uint8_t temp_cmd[20];
	uint8_t opcode, opcode_len, value, power_decrease, psd_mode,
	    access_type;

	opcode = *p++;
	RTKBT_DBG("receive bt coex control event from wifi, op 0x%02x", opcode);

	switch (opcode) {
	case BT_PATCH_VERSION_QUERY:
		rtk_notify_btpatch_version_to_wifi();
		break;

	case IGNORE_WLAN_ACTIVE_CONTROL:
		opcode_len = *p++;
		value = *p++;
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_BT_ENABLE_IGNORE_WLAN_ACT_CMD;
		temp_cmd[1] = 1;
		temp_cmd[2] = value;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 3, temp_cmd);
		break;

	case LNA_CONSTRAIN_CONTROL:
		opcode_len = *p++;
		value = *p++;
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_SET_BT_LNA_CONSTRAINT;
		temp_cmd[1] = 1;
		temp_cmd[2] = value;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 3, temp_cmd);
		break;

	case BT_POWER_DECREASE_CONTROL:
		opcode_len = *p++;
		power_decrease = *p++;
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_WIFI_FORCE_TX_POWER_CMD;
		temp_cmd[1] = 1;
		temp_cmd[2] = power_decrease;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 3, temp_cmd);
		break;

	case BT_PSD_MODE_CONTROL:
		opcode_len = *p++;
		psd_mode = *p++;
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_SET_BT_PSD_MODE;
		temp_cmd[1] = 1;
		temp_cmd[2] = psd_mode;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 3, temp_cmd);
		break;

	case WIFI_BW_CHNL_NOTIFY:
		opcode_len = *p++;
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_WIFI_CHANNEL_AND_BANDWIDTH_CMD;
		temp_cmd[1] = 3;
		memcpy(temp_cmd + 2, p, 3);	//wifi_state, wifi_centralchannel, chnnels_btnotuse
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 5, temp_cmd);
		break;

	case QUERY_BT_AFH_MAP:
		opcode_len = *p++;
		btrtl_coex.piconet_id = *p++;
		btrtl_coex.mode = *p++;
		temp_cmd[0] = HCI_VENDOR_SUB_CMD_GET_AFH_MAP_L;
		temp_cmd[1] = 2;
		temp_cmd[2] = btrtl_coex.piconet_id;
		temp_cmd[3] = btrtl_coex.mode;
		rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 4, temp_cmd);
		break;

	case BT_REGISTER_ACCESS:
		opcode_len = *p++;
		access_type = *p++;
		if (access_type == 0) {	//read
			temp_cmd[0] = HCI_VENDOR_SUB_CMD_RD_REG_REQ;
			temp_cmd[1] = 5;
			temp_cmd[2] = *p++;
			memcpy(temp_cmd + 3, p, 4);
			rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 7,
					     temp_cmd);
		} else {	//write
			temp_cmd[0] = HCI_VENDOR_SUB_CMD_RD_REG_REQ;
			temp_cmd[1] = 5;
			temp_cmd[2] = *p++;
			memcpy(temp_cmd + 3, p, 8);
			rtk_vendor_cmd_to_fw(HCI_VENDOR_MAILBOX_CMD, 11,
					     temp_cmd);
		}
		break;

	default:
		break;
	}
}

static void rtk_handle_event_from_wifi(uint8_t * msg)
{
	uint8_t *p = msg;
	uint8_t event_code = *p++;
	uint8_t total_length;
	uint8_t extension_event;
	uint8_t operation;
	uint16_t wifi_opcode;
	uint8_t op_status;

	if (memcmp(msg, invite_rsp, sizeof(invite_rsp)) == 0) {
		RTKBT_DBG("receive invite rsp from wifi, wifi is already on");
		btrtl_coex.wifi_on = 1;
		rtk_notify_extension_version_to_wifi();
	}

	if (memcmp(msg, attend_req, sizeof(attend_req)) == 0) {
		RTKBT_DBG("receive attend req from wifi, wifi turn on");
		btrtl_coex.wifi_on = 1;
		rtkbt_coexmsg_send(attend_ack, sizeof(attend_ack));
		rtk_notify_extension_version_to_wifi();
	}

	if (memcmp(msg, wifi_leave, sizeof(wifi_leave)) == 0) {
		RTKBT_DBG("receive wifi leave from wifi, wifi turn off");
		btrtl_coex.wifi_on = 0;
		rtkbt_coexmsg_send(leave_ack, sizeof(leave_ack));
		if (btrtl_coex.polling_enable) {
			btrtl_coex.polling_enable = 0;
			del_timer(&btrtl_coex.polling_timer);
		}
	}

	if (memcmp(msg, leave_ack, sizeof(leave_ack)) == 0) {
		RTKBT_DBG("receive leave ack from wifi");
	}

	if (event_code == 0xFE) {
		total_length = *p++;
		extension_event = *p++;
		switch (extension_event) {
		case RTK_HS_EXTENSION_EVENT_WIFI_SCAN:
			operation = *p;
			RTKBT_DBG("Recv WiFi scan notify event from WiFi, "
				  "op 0x%02x", operation);
			break;

		case RTK_HS_EXTENSION_EVENT_HCI_BT_INFO_CONTROL:
			rtk_handle_bt_info_control(p);
			break;

		case RTK_HS_EXTENSION_EVENT_HCI_BT_COEX_CONTROL:
			rtk_handle_bt_coex_control(p);
			break;

		default:
			break;
		}
	}

	if (event_code == 0x0E) {
		p += 2;		//length, number of complete packets
		STREAM_TO_UINT16(wifi_opcode, p);
		op_status = *p;
		RTKBT_DBG("Recv cmd complete event from WiFi, op 0x%02x, "
			  "status 0x%02x", wifi_opcode, op_status);
	}
}
#endif /* RTB_SOFTWARE_MAILBOX */

static inline void rtl_free_frags(struct rtl_coex_struct *coex)
{
	unsigned long flags;

	spin_lock_irqsave(&coex->rxlock, flags);

	coex->elen = 0;
	coex->tbuff = NULL;

	spin_unlock_irqrestore(&coex->rxlock, flags);
}

static void check_profileinfo_cmd(void)
{
	//1 + 6 * handle_bumfer, handle_number = 0
	uint8_t profileinfo_buf[] = {0x00};
	rtk_vendor_cmd_to_fw(HCI_VENDOR_SET_PROFILE_REPORT_COMMAND, 1,
				profileinfo_buf);
}

static void rtl_cmd_work(struct work_struct *work)
{
	check_profileinfo_cmd();
}

void rtk_btcoex_open(struct hci_dev *hdev)
{
	unsigned long flags;

	if (test_and_set_bit(RTL_COEX_RUNNING, &btrtl_coex.flags)) {
		RTKBT_WARN("RTL COEX is already running.");
		return;
	}

	RTKBT_INFO("Open BTCOEX");

	/* Just for test */
	//struct rtl_btinfo_ctl ctl;

	INIT_DELAYED_WORK(&btrtl_coex.fw_work, (void *)rtl_hci_work_func);
	INIT_DELAYED_WORK(&btrtl_coex.cmd_work, rtl_cmd_work);
#ifdef RTB_SOFTWARE_MAILBOX
#ifdef RTK_COEX_OVER_SYMBOL
	INIT_WORK(&rtw_work, rtw_work_func);
	skb_queue_head_init(&rtw_q);
	rtw_coex_on = 1;
#else
	INIT_DELAYED_WORK(&btrtl_coex.sock_work,
			  (void *)udpsocket_recv_data);
#endif
#endif /* RTB_SOFTWARE_MAILBOX */

	btrtl_coex.hdev = hdev;
#ifdef RTB_SOFTWARE_MAILBOX
	btrtl_coex.wifi_on = 0;
#endif

	init_profile_hash(&btrtl_coex);
	init_connection_hash(&btrtl_coex);

	spin_lock_irqsave(&btrtl_coex.rxlock, flags);
	btrtl_coex.pkt_type = 0;
	btrtl_coex.expect = 0;
	btrtl_coex.elen = 0;
	btrtl_coex.tbuff = NULL;
	spin_unlock_irqrestore(&btrtl_coex.rxlock, flags);

#ifdef RTB_SOFTWARE_MAILBOX
#ifndef RTK_COEX_OVER_SYMBOL
	create_udpsocket();
#endif
	rtkbt_coexmsg_send(invite_req, sizeof(invite_req));
#endif
	queue_delayed_work(btrtl_coex.fw_wq, &btrtl_coex.cmd_work,
			msecs_to_jiffies(10));
	/* Just for test */
	//ctl.polling_enable = 1;
	//ctl.polling_time = 1;
	//ctl.autoreport_enable = 1;
	//rtk_handle_bt_info_control((u8 *)&ctl);
}

void rtk_btcoex_close(void)
{

	if (!test_and_clear_bit(RTL_COEX_RUNNING, &btrtl_coex.flags)) {
		RTKBT_WARN("RTL COEX is already closed.");
		return;
	}

	RTKBT_INFO("Close BTCOEX");

#ifdef RTB_SOFTWARE_MAILBOX
	/* Close coex socket */
	if (btrtl_coex.wifi_on)
		rtkbt_coexmsg_send(bt_leave, sizeof(bt_leave));
#ifdef RTK_COEX_OVER_SYMBOL
	rtw_coex_on = 0;
	skb_queue_purge(&rtw_q);
	cancel_work_sync(&rtw_work);
#else
	cancel_delayed_work_sync(&btrtl_coex.sock_work);
	if (btrtl_coex.sock_open) {
		btrtl_coex.sock_open = 0;
		RTKBT_DBG("release udp socket");
		sock_release(btrtl_coex.udpsock);
	}
#endif

	/* Delete all timers */
	if (btrtl_coex.polling_enable) {
		btrtl_coex.polling_enable = 0;
		del_timer_sync(&(btrtl_coex.polling_timer));
	}
#endif /* RTB_SOFTWARE_MAILBOX */

	cancel_delayed_work_sync(&btrtl_coex.fw_work);
	cancel_delayed_work_sync(&btrtl_coex.cmd_work);

	/* Process all the remaining pkts */
	rtl_hci_work_func(&btrtl_coex.fw_work.work);

	flush_connection_hash(&btrtl_coex);
	flush_profile_hash(&btrtl_coex);
	btrtl_coex.profile_bitmap = 0;
	btrtl_coex.profile_status = 0;

	rtl_free_frags(&btrtl_coex);
	profileinfo_cmd = 0;
	RTKBT_DBG("-x");
}

void rtk_btcoex_probe(struct hci_dev *hdev)
{
	btrtl_coex.hdev = hdev;
}

void rtk_btcoex_init(void)
{
	RTKBT_DBG("%s: version: %s", __func__, RTK_VERSION);
	RTKBT_DBG("create workqueue");
#ifdef RTB_SOFTWARE_MAILBOX
#ifdef RTK_COEX_OVER_SYMBOL
	RTKBT_INFO("Coex over Symbol");
	rtw_wq = create_workqueue("btcoexwork");
	skb_queue_head_init(&rtw_q);
#else
	RTKBT_INFO("Coex over UDP");
	btrtl_coex.sock_wq = create_workqueue("btudpwork");
#endif
#endif /* RTB_SOFTWARE_MAILBOX */
	btrtl_coex.fw_wq = create_workqueue("btfwwork");
	rtl_alloc_buff(&btrtl_coex);
	spin_lock_init(&btrtl_coex.rxlock);
	mutex_init(&btrtl_coex.conn_mutex);
	spin_lock_init(&btrtl_coex.spin_lock_sock);
	mutex_init(&btrtl_coex.profile_mutex);
}

void rtk_btcoex_exit(void)
{
	RTKBT_DBG("%s: destroy workqueue", __func__);
#ifdef RTB_SOFTWARE_MAILBOX
#ifdef RTK_COEX_OVER_SYMBOL
	flush_workqueue(rtw_wq);
	destroy_workqueue(rtw_wq);
#else
	flush_workqueue(btrtl_coex.sock_wq);
	destroy_workqueue(btrtl_coex.sock_wq);
#endif
#endif
	flush_workqueue(btrtl_coex.fw_wq);
	destroy_workqueue(btrtl_coex.fw_wq);
	rtl_free_buff(&btrtl_coex);
}
