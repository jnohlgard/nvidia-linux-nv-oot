/******************************************************************************
 *
 * Copyright(c) 2007 - 2023 Realtek Corporation.
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
#ifndef _RTW_XMIT_H_
#define _RTW_XMIT_H_

#if 0 /*CONFIG_CORE_XMITBUF*/
#ifdef CONFIG_PCI_HCI
	#define XMITBUF_ALIGN_SZ 4
#else
	#ifdef USB_XMITBUF_ALIGN_SZ
		#define XMITBUF_ALIGN_SZ (USB_XMITBUF_ALIGN_SZ)
	#else
		#define XMITBUF_ALIGN_SZ 512
	#endif
#endif

#define MAX_CMDBUF_SZ	(5120)	/* (4096) */
#endif

#define MAX_BEACON_LEN	512

#define MAX_NUMBLKS		(1)

#define XMIT_VO_QUEUE (0)
#define XMIT_VI_QUEUE (1)
#define XMIT_BE_QUEUE (2)
#define XMIT_BK_QUEUE (3)

#define VO_QUEUE_INX		0
#define VI_QUEUE_INX		1
#define BE_QUEUE_INX		2
#define BK_QUEUE_INX		3
#define BCN_QUEUE_INX		4
#define MGT_QUEUE_INX		5
#define HIGH_QUEUE_INX		6
#define TXCMD_QUEUE_INX	7

#define HW_QUEUE_ENTRY	8


#ifdef RTW_PHL_TX
#ifndef RTW_MAX_FRAG_NUM
#define RTW_MAX_FRAG_NUM 10 //max scatter number of a packet to xmit
#endif /*RTW_MAX_FRAG_NUM*/
#define RTW_MAX_WL_HEAD	100
#define RTW_MAX_WL_TAIL 100
#define RTW_SZ_LLC	(SNAP_SIZE + sizeof(u16))
#define RTW_SZ_FCS	4
#endif

#define WEP_IV(pattrib_iv, dot11txpn, keyidx)\
	do {\
		dot11txpn.val = (dot11txpn.val == 0xffffff) ? 0 : (dot11txpn.val + 1);\
		pattrib_iv[0] = dot11txpn._byte_.TSC0;\
		pattrib_iv[1] = dot11txpn._byte_.TSC1;\
		pattrib_iv[2] = dot11txpn._byte_.TSC2;\
		pattrib_iv[3] = ((keyidx & 0x3)<<6);\
	} while (0)


#define TKIP_IV(pattrib_iv, dot11txpn, keyidx)\
	do {\
		dot11txpn.val = dot11txpn.val == 0xffffffffffffULL ? 0 : (dot11txpn.val + 1);\
		pattrib_iv[0] = dot11txpn._byte_.TSC1;\
		pattrib_iv[1] = (dot11txpn._byte_.TSC1 | 0x20) & 0x7f;\
		pattrib_iv[2] = dot11txpn._byte_.TSC0;\
		pattrib_iv[3] = BIT(5) | ((keyidx & 0x3)<<6);\
		pattrib_iv[4] = dot11txpn._byte_.TSC2;\
		pattrib_iv[5] = dot11txpn._byte_.TSC3;\
		pattrib_iv[6] = dot11txpn._byte_.TSC4;\
		pattrib_iv[7] = dot11txpn._byte_.TSC5;\
	} while (0)

#define AES_IV(pattrib_iv, dot11txpn, keyidx)\
	do {\
		dot11txpn.val = dot11txpn.val == 0xffffffffffffULL ? 0 : (dot11txpn.val + 1);\
		pattrib_iv[0] = dot11txpn._byte_.TSC0;\
		pattrib_iv[1] = dot11txpn._byte_.TSC1;\
		pattrib_iv[2] = 0;\
		pattrib_iv[3] = BIT(5) | ((keyidx & 0x3)<<6);\
		pattrib_iv[4] = dot11txpn._byte_.TSC2;\
		pattrib_iv[5] = dot11txpn._byte_.TSC3;\
		pattrib_iv[6] = dot11txpn._byte_.TSC4;\
		pattrib_iv[7] = dot11txpn._byte_.TSC5;\
	} while (0)

#define GCMP_IV(a, b, c) AES_IV(a, b, c)

/* Check if AMPDU Tx is supported or not. If it is supported,
* it need to check "amsdu in ampdu" is supported or not.
* (ampdu_en, amsdu_ampdu_en) =
* (0, x) : AMPDU is not enable, but AMSDU is valid to send.
* (1, 0) : AMPDU is enable, AMSDU in AMPDU is not enable. So, AMSDU is not valid to send.
* (1, 1) : AMPDU and AMSDU in AMPDU are enable. So, AMSDU is valid to send.
*/
#define IS_AMSDU_AMPDU_NOT_VALID(pattrib)\
	 ((pattrib->ampdu_en == _TRUE) && (pattrib->amsdu_ampdu_en == _FALSE))

#define IS_AMSDU_AMPDU_VALID(pattrib)\
	 !((pattrib->ampdu_en == _TRUE) && (pattrib->amsdu_ampdu_en == _FALSE))

#define HWXMIT_ENTRY	4

/* For Buffer Descriptor ring architecture */
#if defined(BUF_DESC_ARCH) || defined(CONFIG_TRX_BD_ARCH)
	#define TX_BUFFER_SEG_NUM	1 /* 0:2 seg, 1: 4 seg, 2: 8 seg. */
#endif

/*GEORGIA_TODO_FIXIT_MOVE_TO_HAL*/
#if defined(CONFIG_RTL8822B) || defined(CONFIG_RTL8822C)
	#define TXDESC_SIZE 48		/* HALMAC_TX_DESC_SIZE_8822B */
#elif defined(CONFIG_RTL8821C)
	#define TXDESC_SIZE 48		/* HALMAC_TX_DESC_SIZE_8821C */
#elif defined(CONFIG_RTL8814B)
	#define TXDESC_SIZE (16 + 32)
#else
	#define TXDESC_SIZE 32 /* old IC (ex: 8188E) */
#endif

#ifdef CONFIG_TX_EARLY_MODE
	#define EARLY_MODE_INFO_SIZE	8
#endif

#ifdef CONFIG_USB_HCI
	#ifdef USB_PACKET_OFFSET_SZ
		#define PACKET_OFFSET_SZ (USB_PACKET_OFFSET_SZ)
	#else
		#define PACKET_OFFSET_SZ (8)
	#endif
	#define TXDESC_OFFSET (TXDESC_SIZE + PACKET_OFFSET_SZ)
#endif

#ifdef CONFIG_PCI_HCI
	#if defined(CONFIG_RTL8822B) || defined(CONFIG_RTL8821C) || defined(CONFIG_RTL8822C) || defined(CONFIG_TRX_BD_ARCH)
		/* this section is defined for buffer descriptor ring architecture */
		#define TX_WIFI_INFO_SIZE (TXDESC_SIZE) /* it may add 802.11 hdr or others... */
		/* tx desc and payload are in the same buf */
		#define TXDESC_OFFSET (TX_WIFI_INFO_SIZE)
	#else
		/* tx desc and payload are NOT in the same buf */
		#define TXDESC_OFFSET (0)
		/* 8188ee/8723be/8812ae/8821ae has extra PCI DMA info in tx desc */
	#endif
#endif /* CONFIG_PCI_HCI */

#ifdef RTW_PHL_TX
#ifdef TXDESC_OFFSET
#undef TXDESC_OFFSET
#endif
#define TXDESC_OFFSET (0)
#endif

#ifdef RTW_PHL_TX
enum CORE_TX_TYPE {
	RTW_TX_OS = 0,
	RTW_TX_OS_MAC80211,
	RTW_TX_DRV_MGMT,
};
#endif

enum TXDESC_SC {
	SC_DONT_CARE = 0x00,
	SC_UPPER = 0x01,
	SC_LOWER = 0x02,
	SC_DUPLICATE = 0x03
};

#ifdef CONFIG_PCI_HCI
	#ifndef CONFIG_TRX_BD_ARCH	/* CONFIG_TRX_BD_ARCH doesn't need this */
		#define TXDESC_64_BYTES
	#endif
#endif

/*GEORGIA_TODO_FIXIT_IC_DEPENDENCE*/
#ifdef CONFIG_TRX_BD_ARCH
struct tx_buf_desc {
#ifdef CONFIG_64BIT_DMA
#define TX_BUFFER_SEG_SIZE	4	/* in unit of DWORD */
#else
#define TX_BUFFER_SEG_SIZE	2	/* in unit of DWORD */
#endif
	unsigned int dword[TX_BUFFER_SEG_SIZE * (2 << TX_BUFFER_SEG_NUM)];
} __packed;
#elif (defined(CONFIG_RTL8822B) || defined(CONFIG_RTL8822C)) && defined(CONFIG_PCI_HCI) /* 8192ee or 8814ae */
/* 8192EE_TODO */
struct tx_desc {
	unsigned int txdw0;
	unsigned int txdw1;
	unsigned int txdw2;
	unsigned int txdw3;
	unsigned int txdw4;
	unsigned int txdw5;
	unsigned int txdw6;
	unsigned int txdw7;
};
#else
struct tx_desc {
	unsigned int txdw0;
	unsigned int txdw1;
	unsigned int txdw2;
	unsigned int txdw3;
	unsigned int txdw4;
	unsigned int txdw5;
	unsigned int txdw6;
	unsigned int txdw7;

#if defined(TXDESC_40_BYTES) || defined(TXDESC_64_BYTES)
	unsigned int txdw8;
	unsigned int txdw9;
#endif /* TXDESC_40_BYTES */

#ifdef TXDESC_64_BYTES
	unsigned int txdw10;
	unsigned int txdw11;

	/* 2008/05/15 MH Because PCIE HW memory R/W 4K limit. And now,  our descriptor */
	/* size is 40 bytes. If you use more than 102 descriptor( 103*40>4096), HW will execute */
	/* memoryR/W CRC error. And then all DMA fetch will fail. We must decrease descriptor */
	/* number or enlarge descriptor size as 64 bytes. */
	unsigned int txdw12;
	unsigned int txdw13;
	unsigned int txdw14;
	unsigned int txdw15;
#endif
};
#endif

#ifndef CONFIG_TRX_BD_ARCH
union txdesc {
	struct tx_desc txdesc;
	unsigned int value[TXDESC_SIZE >> 2];
};
#endif

#ifdef CONFIG_PCI_HCI
#define PCI_MAX_TX_QUEUE_COUNT	8	/* == HW_QUEUE_ENTRY */

struct rtw_tx_ring {
	unsigned char	qid;
#ifdef CONFIG_TRX_BD_ARCH
	struct tx_buf_desc	*buf_desc;
#else
	struct tx_desc	*desc;
#endif
	dma_addr_t	dma;
	unsigned int	idx;
	unsigned int	entries;
	_queue		queue;
	u32		qlen;
#ifdef CONFIG_TRX_BD_ARCH
	u16		hw_rp_cache;
#endif
};

#ifdef DBG_TXBD_DESC_DUMP

#define TX_BAK_FRMAE_CNT	10
#define TX_BAK_DESC_LEN	48	/* byte */
#define TX_BAK_DATA_LEN		30	/* byte */

struct rtw_tx_desc_backup {
	int tx_bak_rp;
	int tx_bak_wp;
	u8 tx_bak_desc[TX_BAK_DESC_LEN];
	u8 tx_bak_data_hdr[TX_BAK_DATA_LEN];
	u8 tx_desc_size;
};
#endif
#endif

struct	hw_xmit	{
	/* _lock xmit_lock; */
	/* _list	pending; */
	_queue *sta_queue;
	/* struct hw_txqueue *phwtxqueue; */
	/* sint	txcmdcnt; */
	int	accnt;
};


#if 1 //def RTW_PHL_TX
struct pkt_attrib {
//updated by rtw_core_update_xmitframe
	u32 sz_payload_per_frag;

	u32 sz_wlan_head;
	u32 sz_wlan_tail;

	u32 sz_phl_head;
	u32 sz_phl_tail;

	u8	nr_frags;
	u32 frag_len;
	u32 frag_datalen;
#ifdef CONFIG_CORE_TXSC
	u32 frag_len_txsc;
#endif

//updated by
	u16	ether_type;

	u8	src[ETH_ALEN];
	u8	dst[ETH_ALEN];
	u8	ta[ETH_ALEN];
	u8	ra[ETH_ALEN];

	u16	pkt_hdrlen;	/* the original 802.3 pkt header len */
	u32 sz_payload;

	u8	dhcp_pkt;
	u8	icmp_pkt;
	u8	hipriority_pkt; /* high priority packet */

//WLAN HDR
	u16	hdrlen;		/* the WLAN Header Len */
	u8	a4_hdr;
	u8	type;
	u8	subtype;
	u8	qos_en;
	u16	seqnum;
	u8	ampdu_en;/* tx ampdu enable */
	u8	ack_policy;
	u8	amsdu;
	u8	mdata;/* more data bit */
	u8	eosp;
	u8	priority;

//Security
	u8	bswenc;
	/*
	 * encrypt
	 * indicate the encrypt algorithm, ref: enum security_type.
	 * 0: indicate no encrypt.
	 */
	u8	encrypt;
	u8	iv_len;
	u8	icv_len;
	u8	iv[18];
	u8	icv[16];
	u8	key_idx;
	union Keytype	dot11tkiptxmickey;
	/* union Keytype	dot11tkiprxmickey; */
	union Keytype	dot118021x_UncstKey;

//updated by rtw_core_update_xmitframe
	u8	hw_ssn_sel;	/* for HW_SEQ0,1,2,3 */
	u32	pktlen;		/* the original 802.3 pkt raw_data len (not include ether_hdr data) */
	u32	last_txcmdsz;

#if defined(CONFIG_CONCURRENT_MODE)
	u8	bmc_camid;
#endif



	u8	mac_id;
	u8	vcs_mode;	/* virtual carrier sense method */
#ifdef CONFIG_RTW_WDS
	u8	wds;
#endif
#ifdef CONFIG_RTW_MESH
	u8	mda[ETH_ALEN];	/* mesh da */
	u8	msa[ETH_ALEN];	/* mesh sa */
	u8	meshctrl_len;	/* Length of Mesh Control field */
	u8	mesh_frame_mode;
	#if CONFIG_RTW_MESH_DATA_BMC_TO_UC
	u8 mb2u;
	#endif
	u8 mfwd_ttl;
	u32 mseq;
#endif
#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	u8	hw_csum;
#endif


	u8	ht_en;
	u8	raid;/* rate adpative id */
	u8	bwmode;
	u8	ch_offset;/* PRIME_CHNL_OFFSET */
	u8	sgi;/* short GI */
	u8	ampdu_spacing; /* ampdu_min_spacing for peer sta's rx */
	u8	amsdu_ampdu_en;/* tx amsdu in ampdu enable */
#ifdef CONFIG_TX_AMSDU
	u8	tx_amsdu_en;/* tx amsdu enable */
#endif
	u8	pctrl;/* per packet txdesc control enable */
	u8	triggered;/* for ap mode handling Power Saving sta */
	/*u8	qsel;*/
	u8	order;/* order bit */
	u8	rate;
	u8	intel_proxim;
	u8	retry_ctrl;
	u8   mbssid;
	u8	ldpc;
	u8	stbc;

	struct sta_info *psta;

	struct _ADAPTER_LINK *adapter_link;

	u8 rtsen;
	u8 cts2self;
	u8 hw_rts_en;

#ifdef CONFIG_TDLS
	u8 direct_link;
	struct sta_info *ptdls_sta;
#endif /* CONFIG_TDLS */
	u8 key_type;



#ifdef CONFIG_BEAMFORMING
	u16 txbf_p_aid;/*beamforming Partial_AID*/
	u16 txbf_g_id;/*beamforming Group ID*/

	/*
	 * 2'b00: Unicast NDPA
	 * 2'b01: Broadcast NDPA
	 * 2'b10: Beamforming Report Poll
	 * 2'b11: Final Beamforming Report Poll
	 */
	u8 bf_pkt_type;
#endif
	u8 wdinfo_en;/*FPGA_test*/
	u8 dma_ch;/*FPGA_test*/
};
#endif

#if 0 //ndef RTW_PHL_TX
/* reduce size */
struct pkt_attrib {
	u8	type;
	u8	subtype;
	u8	bswenc;
	u8	dhcp_pkt;
	u16	ether_type;
	u16	seqnum;
	u8	hw_ssn_sel;	/* for HW_SEQ0,1,2,3 */
	u16	pkt_hdrlen;	/* the original 802.3 pkt header len */
	u16	hdrlen;		/* the WLAN Header Len */
	u32	pktlen;		/* the original 802.3 pkt raw_data len (not include ether_hdr data) */
	u32	last_txcmdsz;
	u8	nr_frags;
	u8	encrypt;	/* when 0 indicate no encrypt. when non-zero, indicate the encrypt algorith */
#if defined(CONFIG_CONCURRENT_MODE)
	u8	bmc_camid;
#endif
	u8	iv_len;
	u8	icv_len;
	u8	iv[18];
	u8	icv[16];
	u8	priority;
	u8	ack_policy;
	u8	mac_id;
	u8	vcs_mode;	/* virtual carrier sense method */
	u8	dst[ETH_ALEN];
	u8	src[ETH_ALEN];
	u8	ta[ETH_ALEN];
	u8	ra[ETH_ALEN];
#ifdef CONFIG_RTW_MESH
	u8	mda[ETH_ALEN];	/* mesh da */
	u8	msa[ETH_ALEN];	/* mesh sa */
	u8	meshctrl_len;	/* Length of Mesh Control field */
	u8	mesh_frame_mode;
	#if CONFIG_RTW_MESH_DATA_BMC_TO_UC
	u8 mb2u;
	#endif
	u8 mfwd_ttl;
	u32 mseq;
#endif
#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	u8	hw_csum;
#endif
	u8	key_idx;
	u8	qos_en;
	u8	ht_en;
	u8	raid;/* rate adpative id */
	u8	bwmode;
	u8	ch_offset;/* PRIME_CHNL_OFFSET */
	u8	sgi;/* short GI */
	u8	ampdu_en;/* tx ampdu enable */
	u8	ampdu_spacing; /* ampdu_min_spacing for peer sta's rx */
	u8	amsdu;
	u8	amsdu_ampdu_en;/* tx amsdu in ampdu enable */
	u8	mdata;/* more data bit */
	u8	pctrl;/* per packet txdesc control enable */
	u8	triggered;/* for ap mode handling Power Saving sta */
	u8	qsel;
	u8	order;/* order bit */
	u8	eosp;
	u8	rate;
	u8	intel_proxim;
	u8	retry_ctrl;
	u8   mbssid;
	u8	ldpc;
	u8	stbc;

	struct sta_info *psta;

	u8 rtsen;
	u8 cts2self;
	union Keytype	dot11tkiptxmickey;
	/* union Keytype	dot11tkiprxmickey; */
	union Keytype	dot118021x_UncstKey;

#ifdef CONFIG_TDLS
	u8 direct_link;
	struct sta_info *ptdls_sta;
#endif /* CONFIG_TDLS */
	u8 key_type;

	u8 icmp_pkt;
	u8 hipriority_pkt; /* high priority packet */

#ifdef CONFIG_BEAMFORMING
	u16 txbf_p_aid;/*beamforming Partial_AID*/
	u16 txbf_g_id;/*beamforming Group ID*/

	/*
	 * 2'b00: Unicast NDPA
	 * 2'b01: Broadcast NDPA
	 * 2'b10: Beamforming Report Poll
	 * 2'b11: Final Beamforming Report Poll
	 */
	u8 bf_pkt_type;
#endif
	u8 wdinfo_en;/*FPGA_test*/
	u8 dma_ch;/*FPGA_test*/
};
#endif

#ifdef CONFIG_RTW_WDS
#define XATTRIB_GET_WDS(xattrib) ((xattrib)->wds)
#else
#define XATTRIB_GET_WDS(xattrib) 0
#endif

#ifdef CONFIG_RTW_MESH
#define XATTRIB_GET_MCTRL_LEN(xattrib) ((xattrib)->meshctrl_len)
#else
#define XATTRIB_GET_MCTRL_LEN(xattrib) 0
#endif

#ifdef CONFIG_TX_AMSDU
enum {
	RTW_AMSDU_TIMER_UNSET = 0,
	RTW_AMSDU_TIMER_SETTING,
	RTW_AMSDU_TIMER_TIMEOUT,
};
#endif

#define WLANHDR_OFFSET	64

#define NULL_FRAMETAG		(0x0)
#define DATA_FRAMETAG		0x01
#define L2_FRAMETAG		0x02
#define MGNT_FRAMETAG		0x03
#define AMSDU_FRAMETAG	0x04

#define EII_FRAMETAG		0x05
#define IEEE8023_FRAMETAG  0x06

#define MP_FRAMETAG		0x07

#define TXAGG_FRAMETAG	0x08

enum {
	XMITBUF_DATA = 0,
	XMITBUF_MGNT = 1,
	XMITBUF_CMD = 2,
};

bool rtw_xmit_ac_blocked(_adapter *adapter);

struct  submit_ctx {
	systime submit_time; /* */
	u32 timeout_ms; /* <0: not synchronous, 0: wait forever, >0: up to ms waiting */
	int status; /* status for operation */
	void *rsp; /* rsp buffer allocated by handler */
	_completion done;
};

enum {
	RTW_SCTX_SUBMITTED = -1,
	RTW_SCTX_DONE_SUCCESS = 0,
	RTW_SCTX_DONE_UNKNOWN,
	RTW_SCTX_DONE_TIMEOUT,
	RTW_SCTX_DONE_BUF_ALLOC,
	RTW_SCTX_DONE_BUF_FREE,
	RTW_SCTX_DONE_WRITE_PORT_ERR,
	RTW_SCTX_DONE_TX_DESC_NA,
	RTW_SCTX_DONE_TX_DENY,
	RTW_SCTX_DONE_CCX_PKT_FAIL,
	RTW_SCTX_DONE_DRV_STOP,
	RTW_SCTX_DONE_DEV_REMOVE,
	RTW_SCTX_DONE_CMD_ERROR,
	RTW_SCTX_DONE_CMD_DROP,
	RTX_SCTX_CSTR_WAIT_RPT2,
};


void rtw_sctx_init(struct submit_ctx *sctx, int timeout_ms);
int rtw_sctx_wait(struct submit_ctx *sctx, const char *msg);
void rtw_sctx_done_err(struct submit_ctx **sctx, int status);
void rtw_sctx_done(struct submit_ctx **sctx);

#if 0 /*CONFIG_CORE_XMITBUF*/
struct xmit_buf {
	_list	list;

	_adapter *padapter;

	u8 *pallocated_buf;

	u8 *pbuf;

	void *priv_data;

	u16 buf_tag; /* 0: Normal xmitbuf, 1: extension xmitbuf, 2:cmd xmitbuf */
	u16 flags;
	u32 alloc_sz;

	u32  len;

	struct submit_ctx *sctx;

#ifdef CONFIG_USB_HCI

	/* u32 sz[8]; */
	u32	ff_hwaddr;
	u8 bulkout_id; /* for halmac */

	PURB	pxmit_urb[8];
	dma_addr_t dma_transfer_addr;	/* (in) dma addr for transfer_buffer */

	u8 bpending[8];

	sint last[8];

#endif

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
	u8 *phead;
	u8 *pdata;
	u8 *ptail;
	u8 *pend;
	u32 ff_hwaddr;
	u8	pg_num;
	u8	agg_num;
#endif

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_TRX_BD_ARCH
	/*struct tx_buf_desc *buf_desc;*/
#else
	struct tx_desc *desc;
#endif
#endif

#if defined(DBG_XMIT_BUF) || defined(DBG_XMIT_BUF_EXT)
	u8 no;
#endif

};
#endif

#ifdef CONFIG_CORE_TXSC
#define MAX_TXSC_SKB_NUM 6
#endif

struct xmit_txreq_buf {
	_list	list;
	u8 *txreq;
	u8 *head;
	u8 *tail;
	u8 *pkt_list;
#ifdef CONFIG_CORE_TXSC
	u8 *pkt[MAX_TXSC_SKB_NUM];
	u8 pkt_cnt;
	_adapter *adapter;
	u16 macid;
	u16 txsc_id;
#endif
};

struct xmit_frame {
	_list	list;

	struct pkt_attrib attrib;

	u16 os_qid;

	struct sk_buff *pkt;

	int	frame_tag;

	_adapter *padapter;

	/*Only for MGNT Frame*/
	u8 *prealloc_buf_addr;
	#ifdef CONFIG_USB_HCI
	dma_addr_t dma_transfer_addr;
	#endif
	u8 *buf_addr;
	#if 0 /*CONFIG_CORE_XMITBUF*/
	struct xmit_buf *pxmitbuf;
	#endif

#ifdef CONFIG_USB_HCI
#ifdef CONFIG_USB_TX_AGGREGATION
	u8	agg_num;
#endif
	s8	pkt_offset;
#endif

#ifdef CONFIG_XMIT_ACK
	u8 ack_report;
#endif

	u8 *alloc_addr; /* the actual address this xmitframe allocated */
	u8 ext_tag; /* 0:data, 1:mgmt */

#ifdef RTW_PHL_TX
	u8 xftype;

	//struct sk_buff		*skb;
	//struct sta_info 		*psta;
	//struct pkt_attrib	tx_attrib;

	u8 alloc_hdr;
	u8 alloc_tail;
	u8 *wlhdr[RTW_MAX_FRAG_NUM];
	u8 *wltail[RTW_MAX_FRAG_NUM];

	u32 txring_idx;
	u32 txreq_cnt;
	struct rtw_xmit_req 	*phl_txreq;
	u32 txfree_cnt;

	struct xmit_txreq_buf	*ptxreq_buf;/* TXREQ_QMGT for recycle*/

	u16 buf_need_free; /* size is realted to RTW_MAX_FRAG_NUM */
#endif

};

struct tx_servq {
	_list	tx_pending;
	_queue	sta_pending;
	int qcnt;
};


struct sta_xmit_priv {
	_lock	lock;
	sint	option;
	sint	apsd_setting;	/* When bit mask is on, the associated edca queue supports APSD. */


	/* struct tx_servq blk_q[MAX_NUMBLKS]; */
	struct tx_servq	be_q;			/* priority == 0,3 */
	struct tx_servq	bk_q;			/* priority == 1,2 */
	struct tx_servq	vi_q;			/* priority == 4,5 */
	struct tx_servq	vo_q;			/* priority == 6,7 */
	_list	legacy_dz;
	_list  apsd;

	u16 txseq_tid[16];

	/* uint	sta_tx_bytes; */
	/* u64	sta_tx_pkts; */
	/* uint	sta_tx_fail; */


};


struct	hw_txqueue	{
	volatile sint	head;
	volatile sint	tail;
	volatile sint 	free_sz;	/* in units of 64 bytes */
	volatile sint      free_cmdsz;
	volatile sint	 txsz[8];
	uint	ff_hwaddr;
	uint	cmd_hwaddr;
	sint	ac_tag;
};

struct agg_pkt_info {
	u16 offset;
	u16 pkt_len;
};
#if 0 /*CONFIG_CORE_XMITBUF*/
enum cmdbuf_type {
	CMDBUF_BEACON = 0x00,
	CMDBUF_RSVD,
	CMDBUF_MAX
};
#endif
struct	xmit_priv	{

	_lock	lock;
	#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
	_sema	xmit_sema;
	#endif

	/* _queue	blk_strms[MAX_NUMBLKS]; */
	_queue	be_pending;
	_queue	bk_pending;
	_queue	vi_pending;
	_queue	vo_pending;
	_queue	bm_pending;

	/* _queue	legacy_dz_queue; */
	/* _queue	apsd_queue; */

	u8 *pallocated_frame_buf;
	u8 *pxmit_frame_buf;
	uint free_xmitframe_cnt;
	_queue	free_xmit_queue;
	uint full_xmitframe_cnt;

	/* uint mapping_addr; */
	/* uint pkt_sz; */

	u8 *xframe_ext_alloc_addr;
	u8 *xframe_ext;
	uint free_xframe_ext_cnt;
	_queue free_xframe_ext_queue;

	/* MGT_TXREQ_QMGT */
	u8 *xframe_ext_txreq_alloc_addr;
	u8 *xframe_ext_txreq;

	/* struct	hw_txqueue	be_txqueue; */
	/* struct	hw_txqueue	bk_txqueue; */
	/* struct	hw_txqueue	vi_txqueue; */
	/* struct	hw_txqueue	vo_txqueue; */
	/* struct	hw_txqueue	bmc_txqueue; */

	uint	frag_len;

	_adapter	*adapter;

	u8   vcs_setting;
	u8	vcs;
	u8	vcs_type;
	/* u16  rts_thresh; */

	u64	tx_bytes;
	u64	tx_pkts;
	u64	tx_drop;
	u64	last_tx_pkts;

	struct hw_xmit *hwxmits;
	u8	hwxmit_entry;

	u8	wmm_para_seq[4];/* sequence for wmm ac parameter strength from large to small. it's value is 0->vo, 1->vi, 2->be, 3->bk. */

#ifdef CONFIG_USB_HCI
	_sema	tx_retevt;/* all tx return event; */
	u8		txirp_cnt;

	_tasklet xmit_tasklet;

	/* per AC pending irp */
	int beq_cnt;
	int bkq_cnt;
	int viq_cnt;
	int voq_cnt;
#endif

#ifdef PRIVATE_R
        u64 tx_be_drop_cnt;
        u64 tx_bk_drop_cnt;
        u64 tx_vi_drop_cnt;
        u64 tx_vo_drop_cnt;
#endif

#ifdef CONFIG_PCI_HCI
	/* Tx */
	struct rtw_tx_ring	tx_ring[PCI_MAX_TX_QUEUE_COUNT];
	int	txringcount[PCI_MAX_TX_QUEUE_COUNT];
	u8 	beaconDMAing;		/* flag of indicating beacon is transmiting to HW by DMA */
#ifdef CONFIG_RTW_TX_AMSDU_USE_WQ
	_workitem_cpu xmit_workitem;
#else
	_tasklet xmit_tasklet;
#endif
#endif

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
#ifdef CONFIG_TX_AMSDU_SW_MODE
	_tasklet xmit_tasklet;
#endif
#endif /* CONFIG_SDIO_HCI */

#if 0 /*CONFIG_CORE_XMITBUF*/
	_queue free_xmitbuf_queue;
	_queue pending_xmitbuf_queue;
	u8 *pallocated_xmitbuf;
	u8 *pxmitbuf;
	uint free_xmitbuf_cnt;

	_queue free_xmit_extbuf_queue;
	u8 *pallocated_xmit_extbuf;
	u8 *pxmit_extbuf;
	uint free_xmit_extbuf_cnt;

	struct xmit_buf	pcmd_xmitbuf[CMDBUF_MAX];
#endif
	u8   hw_ssn_seq_no;/* mapping to REG_HW_SEQ 0,1,2,3 */
	u16	nqos_ssn;
#ifdef CONFIG_TX_EARLY_MODE

#ifdef CONFIG_SDIO_HCI
#define MAX_AGG_PKT_NUM 20
#else
#define MAX_AGG_PKT_NUM 256 /* Max tx ampdu coounts		 */
#endif

	struct agg_pkt_info agg_pkt[MAX_AGG_PKT_NUM];
#endif

#ifdef CONFIG_XMIT_ACK
	int	ack_tx;
	_mutex ack_tx_mutex;
	struct submit_ctx ack_tx_ops;
	u8 ack_tx_seq_no;
	#ifdef CONFIG_XMIT_ACK_BY_REL_RPT
	struct rtw_txfb_t ack_txfb;
	#endif
#endif

#ifdef CONFIG_TX_AMSDU
	_timer amsdu_vo_timer;
	u8 amsdu_vo_timeout;

	_timer amsdu_vi_timer;
	u8 amsdu_vi_timeout;

	_timer amsdu_be_timer;
	u8 amsdu_be_timeout;

	_timer amsdu_bk_timer;
	u8 amsdu_bk_timeout;

	u32 amsdu_debug_set_timer;
	u32 amsdu_debug_timeout;

#ifndef AMSDU_DEBUG_MAX_COUNT
#define AMSDU_DEBUG_MAX_COUNT 5
#endif
	u32 amsdu_debug_coalesce[AMSDU_DEBUG_MAX_COUNT];
	u32 amsdu_debug_tasklet;
	u32 amsdu_debug_enqueue;
	u32 amsdu_debug_dequeue;
#endif
#ifdef DBG_TXBD_DESC_DUMP
	BOOLEAN	 dump_txbd_desc;
#endif
#ifdef CONFIG_PCI_TX_POLLING
	_timer tx_poll_timer;
#endif
#ifdef CONFIG_LAYER2_ROAMING
#ifndef CONFIG_RTW_ROAM_STOP_NETIF_QUEUE
	_queue	rpkt_queue;
#endif
#endif
	_lock lock_sctx;
#ifdef CONFIG_CORE_TXSC
	_lock txsc_lock;
	u8 txsc_enable;
	struct sta_info *ptxsc_sta_cached;
#ifdef CONFIG_TXSC_AMSDU
	u8 txsc_amsdu_enable;
	u8 txsc_amsdu_force_num;
	u64 cnt_txsc_amsdu_enq[4];
	u64 cnt_txsc_amsdu_enq_abort[4];
	u64 cnt_txsc_amsdu_deq[4];
	u64 cnt_txsc_amsdu_dump[MAX_TXSC_SKB_NUM + 1];
	u64 cnt_txsc_amsdu_deq_empty;

	u64 cnt_txsc_amsdu_enq_ps;
	u64 cnt_txsc_amsdu_deq_ps;

	u64 cnt_txsc_amsdu_timeout_dump[MAX_TXSC_SKB_NUM + 1];
	u64 cnt_txsc_amsdu_timeout_deq_empty;
	u64 cnt_txsc_amsdu_timeout_ok[4];
	u64 cnt_txsc_amsdu_timeout_fail[4];
#endif /* CONFIG_TXSC_AMSDU */
#endif /* CONFIG_CORE_TXSC */

	u16 max_agg_time;
};

#if 0 /*CONFIG_CORE_XMITBUF*/
extern struct xmit_frame *__rtw_alloc_cmdxmitframe(struct xmit_priv *pxmitpriv,
		enum cmdbuf_type buf_type);
#define rtw_alloc_cmdxmitframe(p) __rtw_alloc_cmdxmitframe(p, CMDBUF_RSVD)
#define rtw_alloc_bcnxmitframe(p) __rtw_alloc_cmdxmitframe(p, CMDBUF_BEACON)

extern struct xmit_buf *rtw_alloc_xmitbuf_ext(struct xmit_priv *pxmitpriv);
extern s32 rtw_free_xmitbuf_ext(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);

extern struct xmit_buf *rtw_alloc_xmitbuf(struct xmit_priv *pxmitpriv);
extern s32 rtw_free_xmitbuf(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);
#endif
enum rtw_data_rate _rate_mrate2phl(enum MGN_RATE mrate);
void rtw_count_tx_stats(_adapter *padapter, struct xmit_frame *pxmitframe, int sz);
extern void rtw_update_protection(_adapter *padapter, u8 *ie, uint ie_len);

extern s32 rtw_make_wlanhdr(_adapter *padapter, u8 *hdr, struct pkt_attrib *pattrib);
extern s32 rtw_put_snap(u8 *data, u16 h_proto);

extern struct xmit_frame *rtw_alloc_xmitframe(struct xmit_priv *pxmitpriv, u16 os_qid);
struct xmit_frame *rtw_alloc_xmitframe_ext(struct xmit_priv *pxmitpriv);
struct xmit_frame *rtw_alloc_xmitframe_once(struct xmit_priv *pxmitpriv);
extern s32 rtw_free_xmitframe(struct xmit_priv *pxmitpriv, struct xmit_frame *pxmitframe);
extern void rtw_free_xmitframe_queue(struct xmit_priv *pxmitpriv, _queue *pframequeue);
s32 core_tx_free_xmitframe(_adapter *padapter, struct xmit_frame *pxframe);
struct tx_servq *rtw_get_sta_pending(_adapter *padapter, struct sta_info *psta, sint up, u8 *ac);
extern s32 rtw_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
extern struct xmit_frame *rtw_dequeue_xframe(struct xmit_priv *pxmitpriv, struct hw_xmit *phwxmit_i, sint entry);

extern s32 rtw_xmit_classifier(_adapter *padapter, struct xmit_frame *pxmitframe);
extern u32 rtw_calculate_wlan_pkt_size_by_attribue(struct pkt_attrib *pattrib);
#define rtw_wlan_pkt_size(f) rtw_calculate_wlan_pkt_size_by_attribue(&f->attrib)
extern s32 rtw_xmitframe_coalesce(_adapter *padapter, struct sk_buff *pkt,
						struct xmit_frame *pxmitframe);
#if defined(CONFIG_IEEE80211W) || defined(CONFIG_RTW_MESH)
extern s32 rtw_mgmt_xmitframe_coalesce(_adapter *padapter,
			struct sk_buff *pkt, struct xmit_frame *pxmitframe);
#endif
#ifdef CONFIG_TDLS
extern struct tdls_txmgmt *ptxmgmt;
s32 rtw_xmit_tdls_coalesce(_adapter *padapter, struct xmit_frame *pxmitframe, struct tdls_txmgmt *ptxmgmt);
s32 update_tdls_attrib(_adapter *padapter, struct _ADAPTER_LINK *padapter_link, struct pkt_attrib *pattrib);
#endif
s32 _rtw_init_hw_txqueue(struct hw_txqueue *phw_txqueue, u8 ac_tag);
void _rtw_init_sta_xmit_priv(struct sta_xmit_priv *psta_xmitpriv);


s32 rtw_txframes_pending(_adapter *padapter);
s32 rtw_txframes_sta_ac_pending(_adapter *padapter, struct pkt_attrib *pattrib);
void rtw_init_hwxmits(struct hw_xmit *phwxmit, sint entry);


s32 _rtw_init_xmit_priv(struct xmit_priv *pxmitpriv, _adapter *padapter);
void _rtw_free_xmit_priv(struct xmit_priv *pxmitpriv);

u8 rtw_init_lite_xmit_resource(struct dvobj_priv *dvobj);
void rtw_free_lite_xmit_resource(struct dvobj_priv *dvobj);

void rtw_alloc_hwxmits(_adapter *padapter);
void rtw_free_hwxmits(_adapter *padapter);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
s32 rtw_monitor_xmit_entry(struct sk_buff *skb, struct net_device *ndev);
#endif
s32 rtw_xmit_posthandle(_adapter *padapter, struct xmit_frame *pxmitframe, struct sk_buff *pkt);
s32 rtw_xmit(_adapter *padapter, struct sk_buff **pkt, u16 os_qid);
bool xmitframe_hiq_filter(struct xmit_frame *xmitframe);
#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
sint xmitframe_enqueue_for_sleeping_sta(_adapter *padapter, struct xmit_frame *pxmitframe);
void stop_sta_xmit(_adapter *padapter, struct sta_info *psta);
void wakeup_sta_to_xmit(_adapter *padapter, struct sta_info *psta);
void xmit_delivery_enabled_frames(_adapter *padapter, struct sta_info *psta);
#endif

#ifdef RTW_PHL_TX
void dbg_dump_txreq_mdata(struct rtw_t_meta_data *mdata, const char *func);
s32 core_tx_prepare_phl(_adapter *padapter, struct xmit_frame *pxframe);
s32 core_tx_call_phl(_adapter *padapter, struct xmit_frame *pxframe, void *txsc_pkt);
#ifdef CONFIG_CORE_TXSC
struct txsc_pkt_entry;
s32 core_tx_per_packet_sc(_adapter *padapter, struct xmit_frame *pxframe,
		       struct sk_buff **pskb, struct sta_info *psta,
		       struct txsc_pkt_entry *ptxsc_pkt);
#define core_tx_per_packet(a,f,sk,s) core_tx_per_packet_sc(a,f,sk,s,NULL)
#else
s32 core_tx_per_packet(_adapter *padapter, struct xmit_frame *pxframe,
		       struct sk_buff **pskb, struct sta_info *psta);
#endif
s32 rtw_core_tx(_adapter *padapter, struct sk_buff **ppkt, struct sta_info *psta, u16 os_qid);
enum rtw_phl_status rtw_core_tx_recycle(void *drv_priv, struct rtw_xmit_req *txreq);
s32 core_tx_alloc_xmitframe(_adapter *padapter, struct xmit_frame **pxmitframe, u16 os_qid);
#ifdef CONFIG_PCI_HCI
struct tx_local_buf;
void rtw_os_query_local_buf(void *priv, struct tx_local_buf *buf);
void rtw_os_return_local_buf(void *priv, struct tx_local_buf *buf);
#endif
#ifdef CONFIG_CORE_TXSC
void core_recycle_txreq_phyaddr(_adapter *padapter, struct rtw_xmit_req *txreq);
s32 core_tx_free_xmitframe(_adapter *padapter, struct xmit_frame *pxframe);
u8 *get_txreq_buffer(_adapter *padapter, u8 **txreq, u8 **pkt_list, u8 **head, u8 **tail);
u8 tos_to_up(u8 tos);
#endif
#endif

void core_tx_amsdu_handler(unsigned long priv);

u8 rtw_get_tx_bw_mode(_adapter *adapter, struct sta_info *sta);

void rtw_update_tx_rate_bmp(struct dvobj_priv *dvobj);
u8 rtw_get_tx_bw_bmp_of_ht_rate(struct dvobj_priv *dvobj, u8 rate, u8 max_bw);
u8 rtw_get_tx_bw_bmp_of_vht_rate(struct dvobj_priv *dvobj, u8 rate, u8 max_bw);
s16 rtw_rfctl_get_oper_txpwr_max_mbm(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset, u8 ifbmp_mod, u8 if_op, bool eirp);
s16 rtw_rfctl_get_reg_max_txpwr_mbm(struct rf_ctl_t *rfctl, enum band_type band, u8 ch, u8 bw, u8 offset, bool eirp);

u8 query_ra_short_GI(struct sta_info *psta, u8 bw);

u8	qos_acm(u8 acm_mask, u8 priority);

#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
void	enqueue_pending_xmitbuf(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);
void enqueue_pending_xmitbuf_to_head(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);
struct xmit_buf	*dequeue_pending_xmitbuf(struct xmit_priv *pxmitpriv);
struct xmit_buf	*select_and_dequeue_pending_xmitbuf(_adapter *padapter);
sint	check_pending_xmitbuf(struct xmit_priv *pxmitpriv);
thread_return	rtw_xmit_thread(thread_context context);
#endif

#ifdef CONFIG_TX_AMSDU
extern void rtw_amsdu_vo_timeout_handler(void *FunctionContext);
extern void rtw_amsdu_vi_timeout_handler(void *FunctionContext);
extern void rtw_amsdu_be_timeout_handler(void *FunctionContext);
extern void rtw_amsdu_bk_timeout_handler(void *FunctionContext);

extern u8 rtw_amsdu_get_timer_status(_adapter *padapter, u8 priority);
extern void rtw_amsdu_set_timer_status(_adapter *padapter, u8 priority, u8 status);
extern void rtw_amsdu_set_timer(_adapter *padapter, u8 priority);
extern void rtw_amsdu_cancel_timer(_adapter *padapter, u8 priority);

extern s32 rtw_xmitframe_coalesce_amsdu(_adapter *padapter, struct xmit_frame *pxmitframe, struct xmit_frame *pxmitframe_queue);
extern s32 check_amsdu(struct xmit_frame *pxmitframe);
extern s32 check_amsdu_tx_support(_adapter *padapter, struct pkt_attrib *pattrib);
extern struct xmit_frame *rtw_get_xframe(struct xmit_priv *pxmitpriv, int *num_frame);
#endif

void fill_txreq_list_skb(_adapter *padapter,
		struct rtw_xmit_req *txreq, struct rtw_pkt_buf_list **pkt_list,
		struct sk_buff *head_skb, u32 req_sz, s32 offset);

#ifdef DBG_TXBD_DESC_DUMP
void rtw_tx_desc_backup(_adapter *padapter, struct xmit_frame *pxmitframe, u8 desc_size, u8 hwq);
void rtw_tx_desc_backup_reset(void);
u8 rtw_get_tx_desc_backup(_adapter *padapter, u8 hwq, struct rtw_tx_desc_backup **pbak);
#endif

#ifdef CONFIG_PCI_TX_POLLING
void rtw_tx_poll_init(_adapter *padapter);
void rtw_tx_poll_timeout_handler(void *FunctionContext);
void rtw_tx_poll_timer_set(_adapter *padapter, u32 delay);
void rtw_tx_poll_timer_cancel(_adapter *padapter);
#endif

#ifdef CONFIG_XMIT_ACK
int rtw_ack_tx_wait(struct xmit_priv *pxmitpriv, u32 timeout_ms);
void rtw_ack_tx_done(struct xmit_priv *pxmitpriv, int status);

#ifdef CONFIG_XMIT_ACK_BY_REL_RPT
void rtw_ack_txfb_init(_adapter *padapter, struct rtw_txfb_t *txfb);
#endif

#endif /* CONFIG_XMIT_ACK */

enum XMIT_BLOCK_REASON {
	XMIT_BLOCK_NONE = 0,
	XMIT_BLOCK_REDLMEM = BIT0, /*LPS-PG*/
	XMIT_BLOCK_SUSPEND = BIT1, /*WOW*/
	XMIT_BLOCK_MAX = 0xFF,
};
void rtw_init_xmit_block(_adapter *padapter);
void rtw_deinit_xmit_block(_adapter *padapter);

#ifdef DBG_XMIT_BLOCK
void dump_xmit_block(void *sel, _adapter *padapter);
#endif
void rtw_set_xmit_block(_adapter *padapter, enum XMIT_BLOCK_REASON reason);
void rtw_clr_xmit_block(_adapter *padapter, enum XMIT_BLOCK_REASON reason);
bool rtw_is_xmit_blocked(_adapter *padapter);
#ifdef CONFIG_LAYER2_ROAMING
void dequeuq_roam_pkt(_adapter *padapter, bool drop);
#endif
/* include after declaring struct xmit_buf, in order to avoid warning */
#include <xmit_osdep.h>

#endif /* _RTL871X_XMIT_H_ */
