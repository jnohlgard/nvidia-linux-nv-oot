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
#ifndef __RTW_MLME_H_
#define __RTW_MLME_H_

#ifndef MAX_BSS_CNT
#define MAX_BSS_CNT 128
#endif
/* #define   MAX_JOIN_TIMEOUT	2000 */
/* #define   MAX_JOIN_TIMEOUT	2500 */
#define   MAX_JOIN_TIMEOUT	6500
#define   PS_ANNC_DRV_RETRY_INT_MS	30

/*	Commented by Albert 20101105
 *	Increase the scanning timeout because of increasing the SURVEY_TO value. */
#ifndef SCANQUEUE_LIFETIME
#define	SCANQUEUE_LIFETIME 20000 /* 20sec, unit:msec */
#endif /*SCANQUEUE_LIFETIME*/

#define MAX_UNASSOC_STA_CNT 128
#define UNASSOC_STA_LIFETIME_MS 60000

/*pmlmepriv->fw_state*/
#define WIFI_NULL_STATE			0x00000000
#define WIFI_ASOC_STATE			0x00000001 /* Linked */
#define WIFI_REASOC_STATE			0x00000002
#define WIFI_SLEEP_STATE			0x00000004
#define WIFI_STATION_STATE			0x00000008
#define WIFI_AP_STATE				0x00000010
#define WIFI_ADHOC_STATE			0x00000020
#define WIFI_ADHOC_MASTER_STATE		0x00000040
#define WIFI_UNDER_LINKING			0x00000080
#define WIFI_UNDER_WPS				0x00000100
#define WIFI_MESH_STATE			0x00000200
#define WIFI_STA_ALIVE_CHK_STATE		0x00000400
#define WIFI_UNDER_SURVEY			0x00000800 /* under site surveying */
#define WIFI_REGU_FORBID			0x00001000
/*#define WIFI_UNDEFINED_STATE			0x00002000*/
/*#define WIFI_UNDEFINED_STATE			0x00004000*/
/*#define WIFI_UNDEFINED_STATE			0x00008000*/
#define WIFI_MP_STATE				0x00010000
/*#define WIFI_UNDEFINED_STATE			0x00020000*/
/*#define WIFI_UNDEFINED_STATE			0x00040000*/
/*#define WIFI_UNDEFINED_STATE			0x00080000*/
/*#define WIFI_UNDEFINED_STATE			0x00100000*/
/*#define WIFI_UNDEFINED_STATE			0x00200000*/
/*#define WIFI_UNDEFINED_STATE			0x00400000*/
#define WIFI_OP_CH_SWITCHING			0x00800000
#define WIFI_UNDER_KEY_HANDSHAKE		0x01000000
/*#define WIFI_UNDEFINED_STATE			0x02000000*/
/*#define WIFI_UNDEFINED_STATE			0x04000000*/
/*#define WIFI_UNDEFINED_STATE			0x08000000*/
/*#define WIFI_UNDEFINED_STATE			0x10000000*/
#define WIFI_CSA_SKIP_CHECK_BEACON		0x20000000
#define WIFI_CSA_UPDATE_BEACON			0x40000000
#define WIFI_MONITOR_STATE			0x80000000


#define MIRACAST_DISABLED	0
#define MIRACAST_SOURCE		BIT0
#define MIRACAST_SINK		BIT1

#define MIRACAST_MODE_REVERSE(mode) \
	((((mode) & MIRACAST_SOURCE) ? MIRACAST_SINK : 0) | (((mode) & MIRACAST_SINK) ? MIRACAST_SOURCE : 0))

bool is_miracast_enabled(_adapter *adapter);
bool rtw_chk_miracast_mode(_adapter *adapter, u8 mode);
const char *get_miracast_mode_str(int mode);
void rtw_wfd_st_switch(struct sta_info *sta, bool on);

#define MLME_STATE(adapter) get_fwstate(&((adapter)->mlmepriv))
#define CHK_MLME_STATE(adapter, state) check_fwstate(&((adapter)->mlmepriv), (state))
#define SET_MLME_STATE(adapter, state) set_fwstate(&((adapter)->mlmepriv), (state))
#define CLR_MLME_STATE(adapter, state) clr_fwstate(&((adapter)->mlmepriv), (state))
#define _CLR_MLME_STATE_(adapter, state) _clr_fwstate_(&((adapter)->mlmepriv), (state))

/* this driver has only one adapter link */
#define LINK_MLME_STATE(adapter_link) MLME_STATE((adapter_link)->adapter)
#define CHK_LINK_MLME_STATE(adapter_link, state) CHK_MLME_STATE((adapter_link)->adapter, (state))
#define SET_LINK_MLME_STATE(adapter_link, state) SET_MLME_STATE((adapter_link)->adapter, (state))
#define CLR_LINK_MLME_STATE(adapter_link, state) CLR_MLME_STATE((adapter_link)->adapter, (state))
#define _CLR_LINK_MLME_STATE_(adapter_link, state) _CLR_MLME_STATE_((adapter_link)->adapter, (state))

#define MLME_IS_NULL(adapter) CHK_MLME_STATE(adapter, WIFI_NULL_STATE)
#define MLME_IS_STA(adapter) CHK_MLME_STATE(adapter, WIFI_STATION_STATE)
#define MLME_IS_AP(adapter) CHK_MLME_STATE(adapter, WIFI_AP_STATE)
#define MLME_IS_ADHOC(adapter) CHK_MLME_STATE(adapter, WIFI_ADHOC_STATE)
#define MLME_IS_ADHOC_MASTER(adapter) CHK_MLME_STATE(adapter, WIFI_ADHOC_MASTER_STATE)
#define MLME_IS_MESH(adapter) CHK_MLME_STATE(adapter, WIFI_MESH_STATE)
#define MLME_IS_MONITOR(adapter) CHK_MLME_STATE(adapter, WIFI_MONITOR_STATE)
#define MLME_IS_MP(adapter) CHK_MLME_STATE(adapter, WIFI_MP_STATE)
#ifdef CONFIG_P2P
	#define MLME_IS_PD(adapter) rtw_p2p_chk_role(&(adapter)->wdinfo, P2P_ROLE_DEVICE)
	#define MLME_IS_GC(adapter) rtw_p2p_chk_role(&(adapter)->wdinfo, P2P_ROLE_CLIENT)
	#define MLME_IS_GO(adapter) rtw_p2p_chk_role(&(adapter)->wdinfo, P2P_ROLE_GO)
#else /* !CONFIG_P2P */
	#define MLME_IS_PD(adapter) 0
	#define MLME_IS_GC(adapter) 0
	#define MLME_IS_GO(adapter) 0
#endif /* !CONFIG_P2P */

#define MLME_IS_MSRC(adapter) rtw_chk_miracast_mode((adapter), MIRACAST_SOURCE)
#define MLME_IS_MSINK(adapter) rtw_chk_miracast_mode((adapter), MIRACAST_SINK)

#define MLME_IS_SCAN(adapter) CHK_MLME_STATE(adapter, WIFI_UNDER_SURVEY)
#define MLME_IS_LINKING(adapter) CHK_MLME_STATE(adapter, WIFI_UNDER_LINKING)
#define MLME_IS_ASOC(adapter) CHK_MLME_STATE(adapter, WIFI_ASOC_STATE)
#define MLME_IS_OPCH_SW(adapter) CHK_MLME_STATE(adapter, WIFI_OP_CH_SWITCHING)
#define MLME_IS_WPS(adapter) CHK_MLME_STATE(adapter, WIFI_UNDER_WPS)

#define LINK_MLME_IS_SCAN(adapter_link) CHK_LINK_MLME_STATE(adapter_link, WIFI_UNDER_SURVEY)
#define LINK_MLME_IS_ASOC(adapter_link) CHK_LINK_MLME_STATE(adapter_link, WIFI_ASOC_STATE)
#define LINK_MLME_IS_OPCH_SW(adapter_link) CHK_LINK_MLME_STATE(adapter_link, WIFI_OP_CH_SWITCHING)
#define LINK_MLME_IS_CSA_UPBCN(adapter_link) CHK_LINK_MLME_STATE(adapter_link, WIFI_CSA_UPDATE_BEACON)

#if CONFIG_AP_REGU_FORBID
#define MLME_IS_REGU_FORBID(adapter) CHK_MLME_STATE(adapter, WIFI_REGU_FORBID)
#define LINK_MLME_IS_REGU_FORBID(adapter_link) CHK_LINK_MLME_STATE(adapter_link, WIFI_REGU_FORBID)
#else
#define MLME_IS_REGU_FORBID(adapter) false
#define LINK_MLME_IS_REGU_FORBID(adapter_link) false
#endif

#if defined(CONFIG_IOCTL_CFG80211) && defined(CONFIG_P2P)
#define MLME_IS_ROCH(adapter) (rtw_cfg80211_get_is_roch(adapter) == _TRUE)
#else
#define MLME_IS_ROCH(adapter) 0
#endif

#ifdef CONFIG_IOCTL_CFG80211
#define MLME_IS_MGMT_TX(adapter) rtw_cfg80211_get_is_mgmt_tx(adapter)
#else
#define MLME_IS_MGMT_TX(adapter) 0
#endif

#define MLME_STATE_FMT "%s%s%s%s%s%s%s%s%s%s%s%s%s"
#define MLME_STATE_ARG(adapter) \
	MLME_IS_STA((adapter)) ? (MLME_IS_GC((adapter)) ? " GC" : " STA") : \
	MLME_IS_AP((adapter)) ? (MLME_IS_GO((adapter)) ? " GO" : " AP") : \
	MLME_IS_ADHOC((adapter)) ? " ADHOC" : \
	MLME_IS_ADHOC_MASTER((adapter)) ? " ADHOC_M" : \
	MLME_IS_MESH((adapter)) ? " MESH" : \
	MLME_IS_MONITOR((adapter)) ? " MONITOR" : \
	MLME_IS_MP((adapter)) ? " MP" : "", \
	MLME_IS_PD((adapter)) ? " PD" : "", \
	MLME_IS_MSRC((adapter)) ? " MSRC" : "", \
	MLME_IS_MSINK((adapter)) ? " MSINK" : "", \
	MLME_IS_SCAN((adapter)) ? " SCAN" : "", \
	MLME_IS_LINKING((adapter)) ? " LINKING" : "", \
	MLME_IS_ASOC((adapter)) ? " ASOC" : "", \
	MLME_IS_OPCH_SW((adapter)) ? " OPCH_SW" : "", \
	MLME_IS_REGU_FORBID((adapter)) ? " REGU_FORBID" : "", \
	MLME_IS_WPS((adapter)) ? " WPS" : "", \
	MLME_IS_ROCH((adapter)) ? " ROCH" : "", \
	MLME_IS_MGMT_TX((adapter)) ? " MGMT_TX" : "", \
	(MLME_STATE((adapter)) & WIFI_SLEEP_STATE) ? " SLEEP" : ""

enum {
	MLME_ACTION_UNKNOWN,
	MLME_ACTION_NONE,
	MLME_SCAN_ENABLE, /* WIFI_UNDER_SURVEY */
	MLME_SCAN_ENTER, /* WIFI_UNDER_SURVEY && !SCAN_DISABLE && !SCAN_BACK_OP */
	MLME_SCAN_DONE, /*  WIFI_UNDER_SURVEY && (SCAN_DISABLE || SCAN_BACK_OP) */
	MLME_SCAN_DISABLE, /* WIFI_UNDER_SURVEY is going to be cleared */
	MLME_STA_CONNECTING,
	MLME_STA_CONNECTED,
	MLME_STA_DISCONNECTED,
	MLME_TDLS_LINKED,
	MLME_TDLS_NOLINK,
	MLME_AP_STARTED,
	MLME_AP_STOPPED,
	MLME_ADHOC_STARTED,
	MLME_ADHOC_STOPPED,
	MLME_MESH_STARTED,
	MLME_MESH_STOPPED,
	MLME_OPCH_SWITCH,
};

enum dot11AuthAlgrthmNum {
	dot11AuthAlgrthm_Open = 0,
	dot11AuthAlgrthm_Shared,
	dot11AuthAlgrthm_8021X,
	dot11AuthAlgrthm_Auto,
	dot11AuthAlgrthm_WAPI,
	dot11AuthAlgrthm_MaxNum
};

/**
 * enum mlme_auth_type - AuthenticationType
 *
 * @MLME_AUTHTYPE_OPEN_SYSTEM: Open System authentication
 * @MLME_AUTHTYPE_SHARED_KEY: Shared Key authentication (WEP only)
 * @MLME_AUTHTYPE_FT: Fast BSS Transition (IEEE 802.11r)
 * @MLME_AUTHTYPE_NETWORK_EAP: Network EAP (some Cisco APs and mainly LEAP)
 * @MLME_AUTHTYPE_SAE: Simultaneous authentication of equals
 * @MLME_AUTHTYPE_FILS_SK: Fast Initial Link Setup shared key
 * @MLME_AUTHTYPE_FILS_SK_PFS: Fast Initial Link Setup shared key with PFS
 * @MLME_AUTHTYPE_FILS_PK: Fast Initial Link Setup public key
 * @__MLME_AUTHTYPE_NUM: internal
 * @MLME_AUTHTYPE_MAX: maximum valid auth algorithm
 * @MLME_AUTHTYPE_AUTOMATIC: determine automatically (if necessary by trying
 *      multiple times); this is invalid in netlink -- leave out the attribute
 *      for this on CONNECT commands.
 */
enum mlme_auth_type {
	MLME_AUTHTYPE_OPEN_SYSTEM,
	MLME_AUTHTYPE_SHARED_KEY,
	MLME_AUTHTYPE_FT,
	MLME_AUTHTYPE_NETWORK_EAP,
	MLME_AUTHTYPE_SAE,
	MLME_AUTHTYPE_FILS_SK,
	MLME_AUTHTYPE_FILS_SK_PFS,
	MLME_AUTHTYPE_FILS_PK,

	/* keep last */
	__MLME_AUTHTYPE_NUM,
	MLME_AUTHTYPE_MAX = __MLME_AUTHTYPE_NUM - 1,
	MLME_AUTHTYPE_AUTOMATIC
};


#define WIFI_FREQUENCY_BAND_AUTO 0
#define WIFI_FREQUENCY_BAND_5GHZ 1
#define WIFI_FREQUENCY_BAND_2GHZ 2

#define rtw_band_valid(band) ((band) <= WIFI_FREQUENCY_BAND_2GHZ)
#define rtw_band_str(band) (band==BAND_ON_6G)?"6G":(band==BAND_ON_5G)?"5G":(band==BAND_ON_24G)?"2G":"unknown"

enum SCAN_RESULT_TYPE {
	SCAN_RESULT_P2P_ONLY = 0,		/*	Will return all the P2P devices. */
	SCAN_RESULT_ALL = 1,			/*	Will return all the scanned device, include AP. */
	SCAN_RESULT_WFD_TYPE = 2		/*	Will just return the correct WFD device. */
									/*	If this device is Miracast sink device, it will just return all the Miracast source devices. */
};

/*

there are several "locks" in mlme_priv,
since mlme_priv is a shared resource between many threads,
like ISR/Call-Back functions, the OID handlers, and even timer functions.


Each _queue has its own locks, already.
Other items are protected by mlme_priv.lock.

To avoid possible dead lock, any thread trying to modifiying mlme_priv
SHALL not lock up more than one locks at a time!

*/


#define traffic_threshold	10
#define	traffic_scan_period	500

typedef struct _RT_LINK_DETECT_T {
	u32				NumTxOkInPeriod;
	u32				NumRxOkInPeriod;
	u32				NumRxUnicastOkInPeriod;
	BOOLEAN			bBusyTraffic;
	BOOLEAN			bTxBusyTraffic;
	BOOLEAN			bRxBusyTraffic;
	BOOLEAN			bHigherBusyTraffic; /* For interrupt migration purpose. */
	BOOLEAN			bHigherBusyRxTraffic; /* We may disable Tx interrupt according as Rx traffic. */
	BOOLEAN			bHigherBusyTxTraffic; /* We may disable Tx interrupt according as Tx traffic. */
	/* u8 TrafficBusyState; */
	u8 TrafficTransitionCount;
	u32 LowPowerTransitionCount;
} RT_LINK_DETECT_T, *PRT_LINK_DETECT_T;

#ifdef CONFIG_WFD

struct wifi_display_info {
	u16							wfd_enable;			/*	Eanble/Disable the WFD function. */
	u16							init_rtsp_ctrlport;	/* init value of rtsp_ctrlport when WFD enable */
	u16							rtsp_ctrlport;		/* TCP port number at which the this WFD device listens for RTSP messages, 0 when WFD disable */
	u16							tdls_rtsp_ctrlport;	/* rtsp_ctrlport used by tdls, will sync when rtsp_ctrlport is changed by user */
	u16							peer_rtsp_ctrlport;	/*	TCP port number at which the peer WFD device listens for RTSP messages */
													/*	This filed should be filled when receiving the gropu negotiation request */

	u8							peer_session_avail;	/*	WFD session is available or not for the peer wfd device. */
													/*	This variable will be set when sending the provisioning discovery request to peer WFD device. */
													/*	And this variable will be reset when it is read by using the iwpriv p2p_get wfd_sa command. */
	u8							ip_address[4];
	u8							peer_ip_address[4];
	u8							wfd_pc;				/*	WFD preferred connection */
													/*	0 -> Prefer to use the P2P for WFD connection on peer side. */
													/*	1 -> Prefer to use the TDLS for WFD connection on peer side. */

	u8							wfd_device_type;	/*	WFD Device Type */
													/*	0 -> WFD Source Device */
													/*	1 -> WFD Primary Sink Device */
	enum	SCAN_RESULT_TYPE	scan_result_type;	/*	Used when P2P is enable. This parameter will impact the scan result. */
	u8 op_wfd_mode;
	u8 stack_wfd_mode;
};
#endif /* CONFIG_WFD */

#ifdef CONFIG_IOCTL_CFG80211
struct cfg80211_roch_info {
	u8						restore_channel;
	struct ieee80211_channel	remain_on_ch_channel;
	enum nl80211_channel_type	remain_on_ch_type;
	unsigned int duration;
	ATOMIC_T ro_ch_cookie_gen;
	u64 remain_on_ch_cookie;
	bool is_ro_ch;
	struct wireless_dev *ro_ch_wdev;
	systime last_ro_ch_time; /* this will be updated at the beginning and end of ro_ch */
};
#endif /* CONFIG_IOCTL_CFG80211 */

#ifdef CONFIG_P2P_WOWLAN

enum P2P_WOWLAN_RECV_FRAME_TYPE {
	P2P_WOWLAN_RECV_NEGO_REQ = 0,
	P2P_WOWLAN_RECV_INVITE_REQ = 1,
	P2P_WOWLAN_RECV_PROVISION_REQ = 2,
};

struct p2p_wowlan_info {

	u8						is_trigger;
	enum P2P_WOWLAN_RECV_FRAME_TYPE	wowlan_recv_frame_type;
	u8						wowlan_peer_addr[ETH_ALEN];
	u16						wowlan_peer_wpsconfig;
	u8						wowlan_peer_is_persistent;
	u8						wowlan_peer_invitation_type;
};

#endif /* CONFIG_P2P_WOWLAN */

struct wifidirect_info {
	_adapter				*padapter;

#ifdef CONFIG_WFD
	struct wifi_display_info		*wfd_info;
#endif

#ifdef CONFIG_P2P_WOWLAN
	struct p2p_wowlan_info		p2p_wow_info;
#endif /* CONFIG_P2P_WOWLAN */

	enum P2P_ROLE			role;
	u8						listen_channel;
	u8						support_rate[8];
	u8						p2p_wildcard_ssid[P2P_WILDCARD_SSID_LEN];
	u8						wfd_tdls_enable;			/*	Flag to enable or disable the TDLS by WFD Sigma */
														/*	0: disable */
														/*	1: enable */
	u8						wfd_tdls_weaksec;			/*	Flag to enable or disable the weak security function for TDLS by WFD Sigma */
														/*	0: disable */
														/*	In this case, the driver can't issue the tdsl setup request frame. */
														/*	1: enable */
														/*	In this case, the driver can issue the tdls setup request frame */
														/*	even the current security is weak security. */

#ifdef CONFIG_P2P_PS
	enum P2P_PS_MODE		p2p_ps_mode; /* indicate p2p ps mode */
	enum P2P_PS_STATE		p2p_ps_state; /* indicate p2p ps state */
	u8						noa_index; /* Identifies and instance of Notice of Absence timing. */
	u8						ctwindow; /* Client traffic window. A period of time in TU after TBTT. */
	u8						opp_ps; /* opportunistic power save. */
	u8						noa_num; /* number of NoA descriptor in P2P IE. */
	u8						noa_count[P2P_MAX_NOA_NUM]; /* Count for owner, Type of client. */
	u32						noa_duration[P2P_MAX_NOA_NUM]; /* Max duration for owner, preferred or min acceptable duration for client. */
	u32						noa_interval[P2P_MAX_NOA_NUM]; /* Length of interval for owner, preferred or max acceptable interval of client. */
	u32						noa_start_time[P2P_MAX_NOA_NUM]; /* schedule expressed in terms of the lower 4 bytes of the TSF timer. */
#endif /* CONFIG_P2P_PS */
#ifdef PRIVATE_R
	u8						remote_mac_address[48]; /* For Roku find remote function */
	u32						num_of_remote;
#endif
};

struct tdls_ss_record {	/* signal strength record */
	u8		macaddr[ETH_ALEN];
	u8		RxPWDBAll;
	u8		is_tdls_sta;	/* _TRUE: direct link sta, _FALSE: else */
};

struct tdls_temp_mgmt {
	u8	initiator;	/* 0: None, 1: we initiate, 2: peer initiate */
	u8	peer_addr[ETH_ALEN];
};

#ifdef CONFIG_TDLS_CH_SW
struct tdls_ch_switch {
	u32	ch_sw_state;
	ATOMIC_T	chsw_on;
	u8	addr[ETH_ALEN];
	u8	off_ch_num;
	u8	ch_offset;
	u32	cur_time;
	u8	delay_switch_back;
	u8	dump_stack;
	struct submit_ctx	chsw_sctx;
};
#endif

struct tdls_info {
	u8					ap_prohibited;
	u8					ch_switch_prohibited;
	u8					link_established;
	u8					sta_cnt;
	u8					sta_maximum;	/* 1:tdls sta is equal (NUM_STA-1), reach max direct link number; 0: else; */
	struct tdls_ss_record	ss_record;
#ifdef CONFIG_TDLS_CH_SW
	struct tdls_ch_switch	chsw_info;
#endif

	u8					ch_sensing;
	u8					cur_channel;
	u8					collect_pkt_num[MAX_CHANNEL_NUM];
	_lock				cmd_lock;
	_lock				hdl_lock;
	u8					watchdog_count;
	u8					dev_discovered;		/* WFD_TDLS: for sigma test */

	/* Let wpa_supplicant to setup*/
	u8					driver_setup;
#ifdef CONFIG_WFD
	struct wifi_display_info		*wfd_info;
#endif

	struct submit_ctx	*tdls_sctx;
};

struct tdls_txmgmt {
	u8 peer[ETH_ALEN];
	u8 action_code;
	u8 dialog_token;
	u16 status_code;
	u8 *buf;
	size_t len;
};

/* used for mlme_priv.roam_flags */
enum {
	RTW_ROAM_ON_EXPIRED = BIT0,
	RTW_ROAM_ON_RESUME = BIT1,
	RTW_ROAM_ACTIVE = BIT2,
	RTW_ROAM_QUICK_SCAN = BIT3,
	RTW_ROAM_ACTIVE_INTERVAL_EXT = BIT4,
	RTW_ROAM_BTM_IGNORE_DELTA = BIT5,
	RTW_ROAM_ACTIVE_IGNORE_CHK = BIT6,
	RTW_ROAM_BTM = BIT7
};

#define UNASOC_STA_SRC_RX_BMC		0
#define UNASOC_STA_SRC_RX_NMY_UC	1
#define UNASOC_STA_SRC_NUM			2

#define UNASOC_STA_MODE_DISABLED	0
#define UNASOC_STA_MODE_INTERESTED	1
#define UNASOC_STA_MODE_ALL			2
#define UNASOC_STA_MODE_NUM			3

#define UNASOC_STA_DEL_CHK_SKIP		0
#define UNASOC_STA_DEL_CHK_ALIVE	1
#define UNASOC_STA_DEL_CHK_DELETED	2

#ifdef CONFIG_RTW_MULTI_AP
struct unassoc_sta_info {
	_list list;
	u8 addr[ETH_ALEN];
	u8 interested;
	s8 recv_signal_power;
	systime time;
};
#endif

struct	wlan_network {
	_list	list;
	int	network_type;	/* refer to ieee80211.h for WIRELESS_11A/B/G */
	int	fixed;			/* set to fixed when not to be removed as site-surveying */
	systime last_scanned; /* timestamp for the network */
	systime last_non_hidden_ssid_ap;
#ifdef CONFIG_RTW_MESH
#if CONFIG_RTW_MESH_ACNODE_PREVENT
	systime acnode_stime;
	systime acnode_notify_etime;
#endif
#endif
	int	aid;			/* will only be valid when a BSS is joinned. */
	int	join_res;
	struct beacon_keys bcn_keys;
	bool bcn_keys_valid;
#ifdef CONFIG_80211D
	struct country_ie_slave_record cisr;
#endif
	WLAN_BSSID_EX	network; /* must be the last item */
};

#ifdef PRIVATE_R
#define MAX_VENDOR_IE_NUM 10
#define MAX_VENDOR_IE_LEN 255
#define MAX_VENDOR_IE_PARAM_LEN MAX_VENDOR_IE_LEN + 2	/* vendor ie filter index + content maximum length */

#define MAX_NUM_DIS_BCN_INFO 3
struct dis_bcn_key {
	_list	list;
	struct beacon_keys bcn_content;
};
#endif

struct link_mlme_priv {
	u8 	link_id;		/* valid for connectint to MLD network
					** is parsed from RN and ML IEs */
	u8 	is_accepted;	/* Accepted by AP or not */
	u8	to_join;	/* Desired link or not */
	struct wlan_network	cur_network;

	/* bcn check info */
	struct beacon_keys cur_beacon_keys; /* save current beacon keys */
#ifdef PRIVATE_R
	_queue idle_dis_bcn_queue;
	_queue busy_dis_bcn_queue;
	struct dis_bcn_key dis_bcn_key_info[MAX_NUM_DIS_BCN_INFO];
#endif
#ifdef CONFIG_80211N_HT

	/* Number of non-HT AP/stations */
	int num_sta_no_ht;

	/* Number of HT AP/stations 20 MHz */
	/* int num_sta_ht_20mhz; */


	int num_FortyMHzIntolerant;
	u8 sw_to_20mhz; /*switch to 20Mhz BW*/

	struct ht_priv	htpriv;
	struct ampdu_priv ampdu_priv;
#endif
	/* VHT or HE IE is configured by upper layer or not (hostapd or wpa_supplicant) */
	u8 upper_layer_setting;
#ifdef CONFIG_80211AC_VHT
	struct vht_priv	vhtpriv;
#ifdef PRIVATE_R
	/*infra mode, used to store AP's info*/
	struct vht_priv_infra_ap vhtpriv_infra_ap;
#endif /* PRIVATE_R */
#endif

#ifdef PRIVATE_R
	struct ht_priv_infra_ap htpriv_infra_ap;
#endif /* PRIVATE_R */

#ifdef CONFIG_80211AX_HE
	struct he_priv hepriv;
#endif
	struct qos_priv qospriv;
#if defined(CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	/* Number of associated Non-ERP stations (i.e., stations using 802.11b
	 * in 802.11g BSS) */
	int num_sta_non_erp;

	/* Number of associated stations that do not support Short Slot Time */
	int num_sta_no_short_slot_time;

	/* Number of associated stations that do not support Short Preamble */
	int num_sta_no_short_preamble;

	ATOMIC_T olbc; /* Overlapping Legacy BSS Condition (Legacy b/g)*/

	/* Number of HT associated stations that do not support greenfield */
	int num_sta_ht_no_gf;

	/* Number of associated non-HT stations */
	/* int num_sta_no_ht; */

	/* Number of HT associated stations 20 MHz */
	int num_sta_ht_20mhz;

	/* number of associated stations 40MHz intolerant */
	int num_sta_40mhz_intolerant;

	/* Overlapping BSS information */
	ATOMIC_T olbc_ht;

	_lock	bcn_update_lock;
	u8 update_bcn;
	struct rtw_chan_def ori_chandef;
#endif
	u8 ext_capab_ie_data[WLAN_EID_EXT_CAP_MAX_LEN];/*currently for ap mode only*/
	u8 ext_capab_ie_len;

#ifdef CONFIG_80211N_HT
	int ht_20mhz_width_req;
	int ht_intolerant_ch_reported;
	u16 ht_op_mode;
#endif /* CONFIG_80211N_HT */
};

struct mlme_priv {

	_lock	lock;
	sint	fw_state;	/* shall we protect this variable? maybe not necessarily... */
	u8	to_join; /* flag */
	u16 join_status;
#ifdef CONFIG_LAYER2_ROAMING
	u8 roam_scan_order[4];
	u8 to_roam; /* roaming trying times */
	struct wlan_network *roam_network; /* the target of active roam */
	u16 roam_flags;
#ifdef PRIVATE_N
	u8 roam_idle_rssi_delta; /* idle rssi delta for active scan candidate selection */
	u8 roam_busy_rssi_delta; /* busy rssi delta for active scan candidate selection */
	u8 roam_idle_rssi_th;
	u8 roam_busy_rssi_th;
#else
	u8 roam_rssi_delta; /* rssi delta for active scan candidate selection */
	u8 roam_rssi_th;
#endif
	u32 roam_scan_int; 		/* scan interval for active roam (Unit:2 second)*/
	u32 roam_scanr_exp_ms; /* scan result expire time in ms  for roam */
	u8 roam_tgt_addr[ETH_ALEN]; /* request to roam to speicific target without other consideration */
	u8 roam_from_addr[ETH_ALEN];
	u8 roam_scan_count;
	u8 roam_freeze_rssi;
	systime last_roaming;
	bool need_to_roam;
	bool roam_buf_pkt;
#endif

	u32 defs_lmt_sta;
	u32 defs_lmt_time;

	u8	*nic_hdl;
	u32	max_bss_cnt;		/*	The size of scan queue	*/
	_list	*pscanned;
	_queue	free_bss_pool;
	_queue	scanned_queue;
	u8		*free_bss_buf;
	u32		num_of_scanned;

	_queue	free_mld_bss_pool;
	_queue	scanned_mld_queue;
	u8		*free_mld_bss_buf;
	u32		num_of_scanned_mld;

	NDIS_802_11_SSID	assoc_ssid;
	u8	assoc_bssid[6];
	u16	assoc_ch;		/* 0 reserved for no specific channel */
	enum band_type	assoc_band;

	struct wlan_network	dev_cur_network;
	struct wlan_network *cur_network_scanned;

#if CONFIG_DFS
	u8 bcn_cnts_after_csa;
#endif

#ifdef CONFIG_ARP_KEEP_ALIVE_GW
	/* for arp offload keep alive */
	u8 bGetGateway;
	u8	GetGatewayTryCnt;
	u8	gw_mac_addr[ETH_ALEN];
	u8	gw_ip[4];
#endif

	/* uint wireless_mode; no used, remove it */

	_timer assoc_timer;

	uint assoc_by_bssid;
	uint assoc_by_rssi;

	systime scan_start_time; /* used to evaluate the time spent in scanning */

#ifdef CONFIG_SET_SCAN_DENY_TIMER
	_timer set_scan_deny_timer;
	ATOMIC_T set_scan_deny; /* 0: allowed, 1: deny */
#endif
	u8 wpa_phase;/*wpa_phase after wps finished*/

#ifdef CONFIG_80211N_HT
	struct ht_priv	dev_htpriv;
#endif

#ifdef CONFIG_80211AC_VHT
	struct vht_priv	dev_vhtpriv;
#endif

#ifdef CONFIG_80211AX_HE
	struct he_priv dev_hepriv;
#endif

#ifdef CONFIG_80211BE_EHT
	struct mlo_priv mlopriv;
#endif

#ifdef CONFIG_RTW_MBO
	struct mbo_priv mbopriv;
#endif

#ifdef CONFIG_RTW_80211R
	struct ft_roam_info ft_roam;
#endif
#if defined(CONFIG_RTW_WNM) || defined(CONFIG_RTW_80211K) || defined(CONFIG_RTW_FSM_RRM)
	struct roam_nb_info nb_info;
	u8 ch_cnt;
#endif

	RT_LINK_DETECT_T	LinkDetectInfo;

	u8	acm_mask; /* for wmm acm mask */

	u8 *wps_probe_req_ie;
	u32 wps_probe_req_ie_len;

#if defined(CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
#ifdef CONFIG_RTW_80211R
	u8 *auth_rsp;
	u32 auth_rsp_len;
#endif
	u8 *assoc_req;
	u32 assoc_req_len;

	u8 *assoc_rsp;
	u32 assoc_rsp_len;

	u8 *probe_rsp;
	u32 probe_rsp_len;

	/* u8 *wps_probe_req_ie; */
	/* u32 wps_probe_req_ie_len; */

	u8 *beacon_head_ie;
	u32 beacon_head_ie_len;
	u8 *beacon_tail_ie;
	u32 beacon_tail_ie_len;

	u8 *wps_beacon_ie;
	u32 wps_beacon_ie_len;

	u8 *wps_probe_resp_ie;
	u32 wps_probe_resp_ie_len;

	u8 *wps_assoc_resp_ie;
	u32 wps_assoc_resp_ie_len;

	u8 *p2p_beacon_ie;
	u32 p2p_beacon_ie_len;

	u8 *p2p_probe_req_ie;
	u32 p2p_probe_req_ie_len;

	u8 *p2p_probe_resp_ie;
	u32 p2p_probe_resp_ie_len;

	u8 *p2p_go_probe_resp_ie;		/* for GO */
	u32 p2p_go_probe_resp_ie_len;	/* for GO */

	u8 *p2p_assoc_req_ie;
	u32 p2p_assoc_req_ie_len;

	u8 *p2p_assoc_resp_ie;
	u32 p2p_assoc_resp_ie_len;

	_lock	bcn_update_lock;
	u8 update_bcn;

	struct rtw_chan_def ori_chandef;
	#ifdef CONFIG_80211AC_VHT
	u8 ori_vht_en;
	#endif
	u8 ap_isolate;
#endif /* #if defined (CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME) */

#if defined(CONFIG_WFD) && defined(CONFIG_IOCTL_CFG80211)
	u8 *wfd_beacon_ie;
	u32 wfd_beacon_ie_len;

	u8 *wfd_probe_req_ie;
	u32 wfd_probe_req_ie_len;

	u8 *wfd_probe_resp_ie;
	u32 wfd_probe_resp_ie_len;

	u8 *wfd_go_probe_resp_ie;		/* for GO */
	u32 wfd_go_probe_resp_ie_len;	/* for GO */

	u8 *wfd_assoc_req_ie;
	u32 wfd_assoc_req_ie_len;

	u8 *wfd_assoc_resp_ie;
	u32 wfd_assoc_resp_ie_len;
#endif

#ifdef CONFIG_RTW_MBO
	u8 *pcell_data_cap_ie;
	u32 cell_data_cap_len;
	struct mbo_attr_info mbo_attr;
#endif

#ifdef RTK_DMP_PLATFORM
	/* DMP kobject_hotplug function  signal need in passive level */
	_workitem	Linkup_workitem;
	_workitem	Linkdown_workitem;
#endif

#ifdef RTW_BUSY_DENY_SCAN
	systime lastscantime;
#endif

#ifdef CONFIG_CONCURRENT_MODE
	u8	scanning_via_buddy_intf;
#endif

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	u32 vendor_ie_mask[WLAN_MAX_VENDOR_IE_NUM];
	u8 vendor_ie[WLAN_MAX_VENDOR_IE_NUM][WLAN_MAX_VENDOR_IE_LEN];
	u32 vendor_ielen[WLAN_MAX_VENDOR_IE_NUM];
#endif
#ifdef CONFIG_RTW_MULTI_AP
	u8 unassoc_sta_mode_of_stype[UNASOC_STA_SRC_NUM];
	_queue unassoc_sta_queue;
	_queue free_unassoc_sta_queue;
	u8 *free_unassoc_sta_buf;
	u32 interested_unassoc_sta_cnt;
	u32 max_unassoc_sta_cnt;
#endif
#ifdef PRIVATE_R
	u8 vendor_ie_filter[MAX_VENDOR_IE_NUM][MAX_VENDOR_IE_LEN];
	u8 vendor_ie_len[MAX_VENDOR_IE_NUM];
	u8 vendor_ie_filter_enable;
#endif
};

#define set_assoc_timer(mlme, ms) \
	do { \
		/*RTW_INFO("%s set_assoc_timer(%p, %d)\n", __FUNCTION__, (mlme), (ms));*/ \
		_set_timer(&(mlme)->assoc_timer, (ms)); \
	} while (0)
#define cancel_assoc_timer(mlme) \
	do { \
		/*RTW_INFO("%s cancel_assoc_timer(%p)\n", __FUNCTION__, (mlme));*/ \
		_cancel_timer_ex(&(mlme)->assoc_timer); \
	} while (0)

#define RTW_AUTO_SCAN_REASON_UNSPECIFIED	0
#define RTW_AUTO_SCAN_REASON_2040_BSS		BIT0
#define RTW_AUTO_SCAN_REASON_ACS		BIT1
#define RTW_AUTO_SCAN_REASON_ROAM		BIT2
#define RTW_AUTO_SCAN_REASON_ROAM_ACTIVE	BIT3
#define RTW_AUTO_SCAN_REASON_MESH_OFFCH_CAND	BIT4
#define RTW_AUTO_SCAN_REASON_CIS_ENV_BSS	BIT5

/* special auto scan interval values */
#define RTW_ASCAN_INT_NONE	0	/* don't do auto scan */
#define RTW_ASCAN_INT_URGENT	1	/* will ignore busy status */
#define RTW_ASCAN_INT_ASAP	2	/* will blocked by busy status */

#ifdef CONFIG_AP_MODE

struct hostapd_priv {
	_adapter *padapter;

#ifdef CONFIG_HOSTAPD_MLME
	struct net_device *pmgnt_netdev;
	struct usb_anchor anchored;
#endif

};

extern int hostapd_mode_init(_adapter *padapter);
extern void hostapd_mode_unload(_adapter *padapter);
#endif


extern void rtw_joinbss_event_prehandle(_adapter *adapter, u8 *pbuf, u16 status);
extern void rtw_joinbss_event_callback(_adapter *adapter, u8 *pbuf);
extern void rtw_stassoc_event_callback(_adapter *adapter, u8 *pbuf);
extern void rtw_stadel_event_callback(_adapter *adapter, u8 *pbuf);
extern void rtw_wmm_event_callback(_adapter *padapter, u8 *pbuf);
#ifdef CONFIG_IEEE80211W
void rtw_sta_timeout_event_callback(_adapter *adapter, u8 *pbuf);
#endif /* CONFIG_IEEE80211W */

extern void rtw_free_network_queue(_adapter *adapter, u8 isfreeall);

#ifdef PRIVATE_R
struct dis_bcn_key *_rtw_alloc_dis_bcn_key(_queue *pqueue);
struct dis_bcn_key *rtw_alloc_dis_bcn_key(_queue *pidle_queue, _queue *pbusy_queue);
int rtw_enqueue_dis_bcn_key(struct  dis_bcn_key  *pdis_bcn_key, _queue *pbusy_queue);
#endif

extern int rtw_init_mlme_priv(_adapter *adapter);/* (struct mlme_priv *pmlmepriv); */

extern void rtw_free_mlme_priv(struct mlme_priv *pmlmepriv);

extern sint rtw_select_and_join_from_scanned_queue(struct mlme_priv *pmlmepriv);
extern sint rtw_set_key(_adapter *adapter, struct _ADAPTER_LINK *adapter_link,
			sint keyid, u8 set_tx, bool enqueue);
extern sint rtw_set_auth(_adapter *adapter, struct security_priv *psecuritypriv);

__inline static u8 *get_link_bssid(struct link_mlme_priv *pmlmepriv)
{
	/* if sta_mode:pmlmepriv->cur_network.network.MacAddress=> bssid */
	/* if adhoc_mode:pmlmepriv->cur_network.network.MacAddress=> ibss mac address */
	return pmlmepriv->cur_network.network.MacAddress;
}

__inline static u8 *get_mbssid(struct mlme_priv *pmlmepriv)
{

#ifdef CONFIG_STA_MULTIPLE_BSSID
	if (pmlmepriv->dev_cur_network.network.is_mbssid)
		return pmlmepriv->dev_cur_network.network.mbsMacAddress;
	else
#endif
		return pmlmepriv->dev_cur_network.network.MacAddress;
}

__inline static u8 *get_bssid(struct mlme_priv *pmlmepriv)
{
	/* if sta_mode:pmlmepriv->cur_network.network.MacAddress=> bssid */
	/* if adhoc_mode:pmlmepriv->cur_network.network.MacAddress=> ibss mac address */
	return pmlmepriv->dev_cur_network.network.MacAddress;
}

__inline static sint check_fwstate(struct mlme_priv *pmlmepriv, sint state)
{
	if ((state == WIFI_NULL_STATE) &&
		(pmlmepriv->fw_state == WIFI_NULL_STATE))
		return _TRUE;

	if (pmlmepriv->fw_state & state)
		return _TRUE;

	return _FALSE;
}

__inline static sint get_fwstate(struct mlme_priv *pmlmepriv)
{
	return pmlmepriv->fw_state;
}

/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 *
 * ### NOTE:#### (!!!!)
 * MUST TAKE CARE THAT BEFORE CALLING THIS FUNC, YOU SHOULD HAVE LOCKED pmlmepriv->lock
 */
extern void rtw_mi_update_iface_status(struct mlme_priv *pmlmepriv, sint state);

static inline void set_fwstate(struct mlme_priv *pmlmepriv, sint state)
{
	pmlmepriv->fw_state |= state;
	rtw_mi_update_iface_status(pmlmepriv, state);
}
static inline void init_fwstate(struct mlme_priv *pmlmepriv, sint state)
{
	pmlmepriv->fw_state = state;
	rtw_mi_update_iface_status(pmlmepriv, state);
}

static inline void _clr_fwstate_(struct mlme_priv *pmlmepriv, sint state)
{
	pmlmepriv->fw_state &= ~state;
	rtw_mi_update_iface_status(pmlmepriv, state);
}

/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 */
static inline void clr_fwstate(struct mlme_priv *pmlmepriv, sint state)
{
	_rtw_spinlock_bh(&pmlmepriv->lock);
	_clr_fwstate_(pmlmepriv, state);
	_rtw_spinunlock_bh(&pmlmepriv->lock);
}

static inline void up_scanned_network(struct mlme_priv *pmlmepriv)
{
	_rtw_spinlock_bh(&pmlmepriv->lock);
	pmlmepriv->num_of_scanned++;
	_rtw_spinunlock_bh(&pmlmepriv->lock);
}
u8 rtw_is_adapter_up(_adapter *padapter);

__inline static void down_scanned_network(struct mlme_priv *pmlmepriv)
{
	_rtw_spinlock_bh(&pmlmepriv->lock);
	pmlmepriv->num_of_scanned--;
	_rtw_spinunlock_bh(&pmlmepriv->lock);
}

__inline static void set_scanned_network_val(struct mlme_priv *pmlmepriv, sint val)
{
	_rtw_spinlock_bh(&pmlmepriv->lock);
	pmlmepriv->num_of_scanned = val;
	_rtw_spinunlock_bh(&pmlmepriv->lock);
}

__inline static void set_scanned_mld_network_val(struct mlme_priv *pmlmepriv, sint val)
{
	_rtw_spinlock_bh(&pmlmepriv->lock);
	pmlmepriv->num_of_scanned_mld = val;
	_rtw_spinunlock_bh(&pmlmepriv->lock);
}

extern u16 rtw_get_capability(WLAN_BSSID_EX *bss);
void dump_scanned_queue(void *sel, _adapter *adapter);
extern void rtw_disconnect_hdl_under_linked(_adapter *adapter, struct sta_info *psta, u8 free_assoc);
extern void rtw_generate_random_ibss(u8 *pibss);
struct wlan_network *_rtw_find_network(_queue *scanned_queue, const u8 *addr);
struct wlan_network *rtw_find_network(_queue *scanned_queue, const u8 *addr);
extern struct wlan_network *rtw_get_oldest_wlan_network(_queue *scanned_queue);
struct wlan_network *_rtw_find_same_network(_queue *scanned_queue, struct wlan_network *network);
struct wlan_network *rtw_find_same_network(_queue *scanned_queue, struct wlan_network *network);

extern void rtw_free_assoc_resources(_adapter *adapter, u8 lock_scanned_queue);
extern void rtw_indicate_disconnect(_adapter *adapter, u16 reason, u8 locally_generated);
extern void rtw_indicate_connect(_adapter *adapter);
void rtw_indicate_scan_done(_adapter *padapter, bool aborted);

u32 rtw_join_abort_timeout(_adapter *adapter, u32 timeout_ms);

int rtw_cached_pmkid(_adapter *adapter, u8 *bssid);
int rtw_pmkid_sync_rsn(_adapter *adapter, u8 *ie, uint ie_len, int i_ent);
void rtw_set_pmksa(_adapter *padapter, u8 *bssid, u8 *pmkid);

extern int rtw_restruct_sec_ie(_adapter *adapter, u8 *out_ie);
extern int rtw_restruct_wmm_ie(_adapter *adapter, struct _ADAPTER_LINK *adapter_link,
					u8 *in_ie, u8 *out_ie, uint in_len, uint initial_out_len);
extern void rtw_init_registrypriv_dev_network(_adapter *adapter);

extern void rtw_update_registrypriv_dev_network(_adapter *adapter);

extern void rtw_get_encrypt_decrypt_from_registrypriv(_adapter *adapter);

extern void rtw_join_timeout_handler(void *ctx);

extern void rtw_iface_dynamic_check_handlder(struct _ADAPTER *a);
#ifdef CONFIG_CMD_GENERAL
void rtw_core_watchdog_sw_post_hdlr(void *drv_priv);
void rtw_core_watchdog_sw_hdlr(void *drv_priv);
void rtw_core_watchdog_hw_hdlr(void *drv_priv);
#else
extern int rtw_dynamic_check_handlder(void *ctx, void* param, bool discard);
#endif
#if 0 /*#ifdef CONFIG_CORE_DM_CHK_TIMER*/
extern void rtw_dynamic_check_timer_handlder(void *ctx);
extern void rtw_iface_dynamic_check_timer_handlder(_adapter *adapter);
#endif


void rtw_free_mlme_priv_ie_data(struct mlme_priv *pmlmepriv);

#define MLME_BEACON_IE			0
#define MLME_PROBE_REQ_IE		1
#define MLME_PROBE_RESP_IE		2
#define MLME_GO_PROBE_RESP_IE	3
#define MLME_ASSOC_REQ_IE		4
#define MLME_ASSOC_RESP_IE		5

#if defined(CONFIG_WFD) && defined(CONFIG_IOCTL_CFG80211)
int rtw_mlme_update_wfd_ie_data(struct mlme_priv *mlme, u8 type, u8 *ie, u32 ie_len);
#endif


/* extern struct wlan_network* _rtw_dequeue_network(_queue *queue); */

extern struct wlan_network *_rtw_alloc_network(struct mlme_priv *pmlmepriv);


extern void _rtw_free_network(struct mlme_priv *pmlmepriv, struct wlan_network *pnetwork, u8 isfreeall);
extern void _rtw_free_network_nolock(struct mlme_priv *pmlmepriv, struct wlan_network *pnetwork);
extern int rtw_init_link_mlme_priv(struct _ADAPTER_LINK *padapter_link);
extern void rtw_free_mld_network_queue(_adapter *adapter, u8 isfreeall);
#ifdef CONFIG_80211BE_EHT
void rtw_link_mld_network(_adapter *padapter, struct wlan_mld_network *mld_network, struct wlan_network *network);
struct wlan_mld_network *rtw_clone_to_join_mld_network(_adapter *padapter, struct wlan_mld_network *pmld_network);
void rtw_free_cloned_mld_network(struct wlan_mld_network *mld_network);
extern struct wlan_network *rtw_get_link_network_by_linkid(struct wlan_mld_network *mld_network, u8 link_id);
#endif
struct wlan_network *rtw_clone_network(_adapter *padapter, WLAN_BSSID_EX *pnetwork);
struct wlan_mld_network *rtw_alloc_mld_network(_adapter *adapter, const u8 *mac_addr);
extern struct _ADAPTER_LINK *rtw_get_adapter_link_by_linkid(_adapter *padapter, u8 link_id);
struct _ADAPTER_LINK *rtw_get_adapter_link_by_hwband(_adapter *padapter, u8 band_idx);
u8 rtw_adapter_link_get_id(struct _ADAPTER_LINK *alink);
void rtw_update_link_ht_cap(_adapter *padapter, struct _ADAPTER_LINK *padapter_link, u8 *pie, uint ie_len, u8 channel);
void rtw_update_join_priv(_adapter *padapter, struct _ADAPTER_LINK *padapter_link,
				WLAN_BSSID_EX *network, WLAN_BSSID_EX *psecnetwork);
void rtw_clear_to_join_status(_adapter *padapter);
void rtw_clear_is_accepted_status(_adapter *padapter);

extern void _rtw_free_network_queue(_adapter *padapter, u8 isfreeall);

extern sint rtw_if_up(_adapter *padapter);

sint rtw_linked_check(_adapter *padapter);

u8 *rtw_get_capability_from_ie(u8 *ie);
u8 *rtw_get_timestampe_from_ie(u8 *ie);
u8 *rtw_get_beacon_interval_from_ie(u8 *ie);


void rtw_joinbss_reset(_adapter *padapter);

#ifdef CONFIG_80211N_HT
void rtw_ht_get_dft_setting(_adapter *padapter,
				struct protocol_cap_t *dft_proto_cap,
				struct role_link_cap_t *dft_cap);
void	rtw_ht_use_default_setting(_adapter *padapter, struct _ADAPTER_LINK *padapter_link);
void rtw_build_wmm_ie_ht(_adapter *padapter, u8 *out_ie, uint *pout_len);
unsigned int rtw_restructure_ht_ie(_adapter *padapter, struct _ADAPTER_LINK *padapter_link,
						u8 *in_ie, u8 *out_ie, uint in_len, uint *pout_len,
						u8 channel);
void rtw_issue_addbareq_cmd(_adapter *padapter, struct xmit_frame *pxmitframe, u8 issue_when_busy);
#endif

void rtw_append_extended_cap(_adapter *padapter, struct _ADAPTER_LINK *padapter_link,
					u8 *out_ie, uint *pout_len);

int rtw_is_same_ibss(_adapter *adapter, struct wlan_network *pnetwork);
int is_same_network(WLAN_BSSID_EX *src, WLAN_BSSID_EX *dst);
int rtw_check_join_candidate(struct mlme_priv *mlme,
	struct wlan_network **candidate, struct wlan_network *competitor);
u8 rtw_do_join(_adapter *padapter);

#ifdef CONFIG_LAYER2_ROAMING
#define rtw_roam_flags(adapter) ((adapter)->mlmepriv.roam_flags)
#define rtw_chk_roam_flags(adapter, flags) ((adapter)->mlmepriv.roam_flags & flags)
#define rtw_clr_roam_flags(adapter, flags) \
	do { \
		((adapter)->mlmepriv.roam_flags &= ~flags); \
	} while (0)

#define rtw_set_roam_flags(adapter, flags) \
	do { \
		((adapter)->mlmepriv.roam_flags |= flags); \
	} while (0)

#define rtw_assign_roam_flags(adapter, flags) \
	do { \
		((adapter)->mlmepriv.roam_flags = flags); \
	} while (0)

void _rtw_roaming(_adapter *adapter, struct wlan_network *tgt_network, u8 reason);
void rtw_roaming(_adapter *adapter, struct wlan_network *tgt_network, u8 reason);
void rtw_set_to_roam(_adapter *adapter, u8 to_roam);
u8 rtw_dec_to_roam(_adapter *adapter);
u8 rtw_to_roam(_adapter *adapter);
struct wlan_network *rtw_select_roaming_candidate(struct mlme_priv *pmlmepriv);
void rtw_wnm_candidate_info(struct wlan_network *pnetwork, struct wlan_network *cnetwork, u8 *reason);
int rtw_check_roaming_candidate(struct mlme_priv *mlme, struct wlan_network **candidate,
	struct wlan_network *competitor);
#else
#define rtw_roam_flags(adapter) 0
#define rtw_chk_roam_flags(adapter, flags) 0
#define rtw_clr_roam_flags(adapter, flags) do {} while (0)
#define rtw_set_roam_flags(adapter, flags) do {} while (0)
#define rtw_assign_roam_flags(adapter, flags) do {} while (0)
#define _rtw_roaming(adapter, tgt_network) do {} while (0)
#define rtw_roaming(adapter, tgt_network) do {} while (0)
#define rtw_set_to_roam(adapter, to_roam) do {} while (0)
#define rtw_dec_to_roam(adapter) 0
#define rtw_to_roam(adapter) 0
#define rtw_select_roaming_candidate(mlme) NULL
#endif /* CONFIG_LAYER2_ROAMING */

RTW_FUNC_2G_5G_ONLY bool rtw_adjust_chbw(_adapter *adapter, u8 req_ch, u8 *req_bw, u8 *req_offset);
bool rtw_adjust_bchbw(_adapter *adapter, enum band_type req_band, u8 req_ch, u8 *req_bw, u8 *req_offset);

#ifdef CONFIG_RTW_MULTI_AP
void rtw_map_config_monitor_act_non(_adapter *adapter);
void rtw_map_config_monitor(_adapter *adapter, u8 self_act);
void rtw_unassoc_sta_set_mode(_adapter *adapter, u8 stype, u8 mode);
bool rtw_unassoc_sta_src_chk(_adapter *adapter, u8 stype);
void dump_unassoc_sta(void *sel, _adapter *adapter);
void rtw_del_unassoc_sta_queue(_adapter *adapter);
void rtw_del_unassoc_sta(_adapter *adapter, u8 *addr);
void rtw_rx_add_unassoc_sta(_adapter *adapter, u8 stype, u8 *addr, s8 recv_signal_power);
void rtw_add_interested_unassoc_sta(_adapter *adapter, u8 *addr);
void rtw_undo_interested_unassoc_sta(_adapter *adapter, u8 *addr);
void rtw_undo_all_interested_unassoc_sta(_adapter *adapter);
u8 rtw_search_unassoc_sta(_adapter *adapter, u8 *addr, struct unassoc_sta_info *ret_sta);
#endif

void rtw_sta_traffic_info(void *sel, _adapter *adapter);

#define GET_ARP_HTYPE(_arp)	BE_BITS_TO_2BYTE(((u8 *)(_arp)) + 0, 0, 16)
#define GET_ARP_PTYPE(_arp)	BE_BITS_TO_2BYTE(((u8 *)(_arp)) + 2, 0, 16)
#define GET_ARP_HLEN(_arp)	BE_BITS_TO_1BYTE(((u8 *)(_arp)) + 4, 0, 8)
#define GET_ARP_PLEN(_arp)	BE_BITS_TO_1BYTE(((u8 *)(_arp)) + 5, 0, 8)
#define GET_ARP_OPER(_arp)	BE_BITS_TO_2BYTE(((u8 *)(_arp)) + 6, 0, 16)

#define SET_ARP_HTYPE(_arp, _val)	SET_BITS_TO_BE_2BYTE(((u8 *)(_arp)) + 0, 0, 16, _val)
#define SET_ARP_PTYPE(_arp, _val)	SET_BITS_TO_BE_2BYTE(((u8 *)(_arp)) + 2, 0, 16, _val)
#define SET_ARP_HLEN(_arp, _val)	SET_BITS_TO_BE_1BYTE(((u8 *)(_arp)) + 4, 0, 8, _val)
#define SET_ARP_PLEN(_arp, _val)	SET_BITS_TO_BE_1BYTE(((u8 *)(_arp)) + 5, 0, 8, _val)
#define SET_ARP_OPER(_arp, _val)	SET_BITS_TO_BE_2BYTE(((u8 *)(_arp)) + 6, 0, 16, _val)

#define ARP_SHA(_arp, _hlen, _plen)	(((u8 *)(_arp)) + 8)
#define ARP_SPA(_arp, _hlen, _plen)	(((u8 *)(_arp)) + 8 + (_hlen))
#define ARP_THA(_arp, _hlen, _plen)	(((u8 *)(_arp)) + 8 + (_hlen) + (_plen))
#define ARP_TPA(_arp, _hlen, _plen)	(((u8 *)(_arp)) + 8 + 2 * (_hlen) + (_plen))

#define ARP_SENDER_MAC_ADDR(_arp)	ARP_SHA(_arp, ETH_ALEN, RTW_IP_ADDR_LEN)
#define ARP_SENDER_IP_ADDR(_arp)	ARP_SPA(_arp, ETH_ALEN, RTW_IP_ADDR_LEN)
#define ARP_TARGET_MAC_ADDR(_arp)	ARP_THA(_arp, ETH_ALEN, RTW_IP_ADDR_LEN)
#define ARP_TARGET_IP_ADDR(_arp)	ARP_TPA(_arp, ETH_ALEN, RTW_IP_ADDR_LEN)

#define GET_ARP_SENDER_MAC_ADDR(_arp, _val)	_rtw_memcpy(_val, ARP_SENDER_MAC_ADDR(_arp), ETH_ALEN)
#define GET_ARP_SENDER_IP_ADDR(_arp, _val)	_rtw_memcpy(_val, ARP_SENDER_IP_ADDR(_arp), RTW_IP_ADDR_LEN)
#define GET_ARP_TARGET_MAC_ADDR(_arp, _val)	_rtw_memcpy(_val, ARP_TARGET_MAC_ADDR(_arp), ETH_ALEN)
#define GET_ARP_TARGET_IP_ADDR(_arp, _val)	_rtw_memcpy(_val, ARP_TARGET_IP_ADDR(_arp), RTW_IP_ADDR_LEN)

#define SET_ARP_SENDER_MAC_ADDR(_arp, _val)	_rtw_memcpy(ARP_SENDER_MAC_ADDR(_arp), _val, ETH_ALEN)
#define SET_ARP_SENDER_IP_ADDR(_arp, _val)	_rtw_memcpy(ARP_SENDER_IP_ADDR(_arp), _val, RTW_IP_ADDR_LEN)
#define SET_ARP_TARGET_MAC_ADDR(_arp, _val)	_rtw_memcpy(ARP_TARGET_MAC_ADDR(_arp), _val, ETH_ALEN)
#define SET_ARP_TARGET_IP_ADDR(_arp, _val)	_rtw_memcpy(ARP_TARGET_IP_ADDR(_arp), _val, RTW_IP_ADDR_LEN)

void dump_arp_pkt(void *sel, u8 *da, u8 *sa, u8 *arp, bool tx);

#define IPV4_SRC(_iphdr)			(((u8 *)(_iphdr)) + 12)
#define IPV4_DST(_iphdr)			(((u8 *)(_iphdr)) + 16)
#define GET_IPV4_IHL(_iphdr)		BE_BITS_TO_1BYTE(((u8 *)(_iphdr)) + 0, 0, 4)
#define GET_IPV4_PROTOCOL(_iphdr)	BE_BITS_TO_1BYTE(((u8 *)(_iphdr)) + 9, 0, 8)
#define GET_IPV4_SRC(_iphdr)		BE_BITS_TO_4BYTE(((u8 *)(_iphdr)) + 12, 0, 32)
#define GET_IPV4_DST(_iphdr)		BE_BITS_TO_4BYTE(((u8 *)(_iphdr)) + 16, 0, 32)

#define GET_UDP_SRC(_udphdr)			BE_BITS_TO_2BYTE(((u8 *)(_udphdr)) + 0, 0, 16)
#define GET_UDP_DST(_udphdr)			BE_BITS_TO_2BYTE(((u8 *)(_udphdr)) + 2, 0, 16)
#define GET_UDP_SIG1(_udphdr)			BE_BITS_TO_1BYTE(((u8 *)(_udphdr)) + 8, 0, 8)
#define GET_UDP_SIG2(_udphdr)			BE_BITS_TO_1BYTE(((u8 *)(_udphdr)) + 23, 0, 8)

#define TCP_SRC(_tcphdr)				(((u8 *)(_tcphdr)) + 0)
#define TCP_DST(_tcphdr)				(((u8 *)(_tcphdr)) + 2)
#define GET_TCP_SRC(_tcphdr)			BE_BITS_TO_2BYTE(((u8 *)(_tcphdr)) + 0, 0, 16)
#define GET_TCP_DST(_tcphdr)			BE_BITS_TO_2BYTE(((u8 *)(_tcphdr)) + 2, 0, 16)
#define GET_TCP_SEQ(_tcphdr)			BE_BITS_TO_4BYTE(((u8 *)(_tcphdr)) + 4, 0, 32)
#define GET_TCP_ACK_SEQ(_tcphdr)		BE_BITS_TO_4BYTE(((u8 *)(_tcphdr)) + 8, 0, 32)
#define GET_TCP_DOFF(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 12, 4, 4)
#define GET_TCP_FIN(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 0, 1)
#define GET_TCP_SYN(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 1, 1)
#define GET_TCP_RST(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 2, 1)
#define GET_TCP_PSH(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 3, 1)
#define GET_TCP_ACK(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 4, 1)
#define GET_TCP_URG(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 5, 1)
#define GET_TCP_ECE(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 6, 1)
#define GET_TCP_CWR(_tcphdr)			BE_BITS_TO_1BYTE(((u8 *)(_tcphdr)) + 13, 7, 1)

#ifdef CONFIG_STA_CMD_DISPR
enum rtw_phl_status rtw_connect_cmd(struct _ADAPTER *a,
				    struct _WLAN_BSSID_EX *network);
void rtw_connect_abort(struct _ADAPTER *a);
int rtw_connect_abort_wait(struct _ADAPTER *a);
void rtw_connect_req_free(struct _ADAPTER *a);
void rtw_connect_req_init(struct _ADAPTER *a);
enum rtw_phl_status rtw_connect_disconnect_prepare(struct _ADAPTER *a);

enum rtw_phl_status rtw_disconnect_cmd(struct _ADAPTER *a,
				       struct cmd_obj *pcmd);
bool rtw_disconnect_wait_complete(struct _ADAPTER *a, u32 timeout);
int rtw_disconnect_abort_wait(struct _ADAPTER *a);
void rtw_disconnect_req_free(struct _ADAPTER *a);
void rtw_disconnect_req_init(struct _ADAPTER *a);
#endif /* CONFIG_STA_CMD_DISPR */
#endif /* __RTL871X_MLME_H_ */
