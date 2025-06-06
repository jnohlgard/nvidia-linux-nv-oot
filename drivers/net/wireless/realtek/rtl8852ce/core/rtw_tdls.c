/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#define _RTW_TDLS_C_

#include <drv_types.h>

#ifdef CONFIG_TDLS
#define ONE_SEC 	1000 /* 1000 ms */

extern unsigned char MCS_rate_2R[16];
extern unsigned char MCS_rate_1R[16];

bool rtw_check_tdls_frame(void *priv, struct rtw_phl_stainfo_t *phl_sta,
		struct rtw_recv_pkt *phlrx)
{
	struct dvobj_priv *d = (struct dvobj_priv *)priv;
	struct rtw_wifi_role_t *wrole = phl_sta->wrole;
	struct _ADAPTER *a = d->padapters[wrole->id];
	struct security_priv *psecuritypriv = &a->securitypriv;
	struct sta_info *psta = rtw_get_stainfo(&a->stapriv, phl_sta->mac_addr);
	struct rtw_r_meta_data *mdata = &phlrx->mdata;
	struct rtw_pkt_buf_list *pktbuf;
	bool is_tdls = _FALSE;
	u8 a4_shift, privacy, iv_len, icv_len, hdrlen, encrypt;
	u8 *psnap_type, *pcategory;
	static u8 SNAP_ETH_TYPE_TDLS[2] = {0x89, 0x0d};

	if (wrole->type != PHL_RTYPE_STATION && wrole->type != PHL_RTYPE_TDLS)
		goto _exit;

	/* check TDLS frame */
	if (mdata->frame_type != RTW_FRAME_TYPE_DATA)
		goto _exit;

	rtw_warn_on(phlrx->pkt_cnt > 1);
	pktbuf = phlrx->pkt_list; /* &phlrx->pkt_list[0] */

	a4_shift = mdata->a4_frame ? ETH_ALEN : 0;
	privacy = GetPrivacy(pktbuf->vir_addr);

	if (mdata->qos)
		hdrlen = WLAN_HDR_A3_QOS_LEN + a4_shift;
	else
		hdrlen = WLAN_HDR_A3_LEN + a4_shift;

	if (privacy) {
		if ((psta->tdls_sta_state & TDLS_LINKED_STATE) && (psta->dot118021XPrivacy == _AES_))
			encrypt = psta->dot118021XPrivacy;
		else
			GET_ENCRY_ALGO(psecuritypriv, psta, encrypt, IS_MCAST(GetAddr1Ptr(pktbuf->vir_addr)));
		SET_ICE_IV_LEN(iv_len, icv_len, encrypt);
	} else {
		iv_len = icv_len = 0;
	}

	psnap_type = pktbuf->vir_addr + hdrlen + iv_len + SNAP_SIZE;
	pcategory = psnap_type + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	if ((_rtw_memcmp(psnap_type, SNAP_ETH_TYPE_TDLS, ETH_TYPE_LEN)) &&
		(*pcategory == RTW_WLAN_CATEGORY_TDLS))
		is_tdls = _TRUE;

_exit:

	return is_tdls;
}

inline void rtw_tdls_set_link_established(_adapter *adapter, bool en)
{
	adapter->tdlsinfo.link_established = en;
	rtw_mi_update_iface_status(&(adapter->mlmepriv), 0);
}

void rtw_reset_tdls_info(_adapter *padapter)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	ptdlsinfo->ap_prohibited = _FALSE;

	/* For TDLS channel switch, currently we only allow it to work in wifi logo test mode */
	if (padapter->registrypriv.wifi_spec == 1)
		ptdlsinfo->ch_switch_prohibited = _FALSE;
	else
		ptdlsinfo->ch_switch_prohibited = _TRUE;

	rtw_tdls_set_link_established(padapter, _FALSE);
	ptdlsinfo->sta_cnt = 0;
	ptdlsinfo->sta_maximum = _FALSE;

#ifdef CONFIG_TDLS_CH_SW
	ptdlsinfo->chsw_info.ch_sw_state = TDLS_STATE_NONE;
	ATOMIC_SET(&ptdlsinfo->chsw_info.chsw_on, _FALSE);
	ptdlsinfo->chsw_info.off_ch_num = 0;
	ptdlsinfo->chsw_info.ch_offset = CHAN_OFFSET_NO_EXT;
	ptdlsinfo->chsw_info.cur_time = 0;
	ptdlsinfo->chsw_info.delay_switch_back = _FALSE;
	ptdlsinfo->chsw_info.dump_stack = _FALSE;
#endif

	ptdlsinfo->ch_sensing = 0;
	ptdlsinfo->watchdog_count = 0;
	ptdlsinfo->dev_discovered = _FALSE;

#ifdef CONFIG_WFD
	ptdlsinfo->wfd_info = &padapter->wfd_info;
#endif

	ptdlsinfo->tdls_sctx = NULL;
}

int rtw_init_tdls_info(_adapter *padapter)
{
	int	res = _SUCCESS;
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	rtw_reset_tdls_info(padapter);

#ifdef CONFIG_TDLS_DRIVER_SETUP
	ptdlsinfo->driver_setup = _TRUE;
#else
	ptdlsinfo->driver_setup = _FALSE;
#endif /* CONFIG_TDLS_DRIVER_SETUP */

	_rtw_spinlock_init(&ptdlsinfo->cmd_lock);
	_rtw_spinlock_init(&ptdlsinfo->hdl_lock);

	return res;

}

void rtw_free_tdls_info(struct tdls_info *ptdlsinfo)
{
	_rtw_spinlock_free(&ptdlsinfo->cmd_lock);
	_rtw_spinlock_free(&ptdlsinfo->hdl_lock);

	_rtw_memset(ptdlsinfo, 0, sizeof(struct tdls_info));

}

void rtw_free_all_tdls_sta(_adapter *padapter, u8 enqueue_cmd)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	_list	*plist, *phead;
	s32	index;
	struct sta_info *psta = NULL;
	struct sta_info *ptdls_sta[NUM_STA];
	u8 empty_hwaddr[ETH_ALEN] = { 0x00 };

	_rtw_memset(ptdls_sta, 0x00, sizeof(ptdls_sta));

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
	for (index = 0; index < NUM_STA; index++) {
		phead = &(pstapriv->sta_hash[index]);
		plist = get_next(phead);

		while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

			plist = get_next(plist);

			if (psta->tdls_sta_state != TDLS_STATE_NONE)
				ptdls_sta[index] = psta;
		}
	}
	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	for (index = 0; index < NUM_STA; index++) {
		if (ptdls_sta[index]) {
			struct TDLSoption_param tdls_param;

			psta = ptdls_sta[index];

			RTW_INFO("Do tear down to "MAC_FMT" by enqueue_cmd = %d\n", MAC_ARG(psta->phl_sta->mac_addr), enqueue_cmd);

			_rtw_memcpy(&(tdls_param.addr), psta->phl_sta->mac_addr, ETH_ALEN);
			tdls_param.option = TDLS_TEARDOWN_STA_NO_WAIT;
			tdls_hdl(padapter, (unsigned char *)&(tdls_param));
		}
	}
}

int check_ap_tdls_prohibited(u8 *pframe, u8 pkt_len)
{
	u8 tdls_prohibited_bit = 0x40; /* bit(38); TDLS_prohibited */

	if (pkt_len < 5)
		return _FALSE;

	pframe += 4;
	if ((*pframe) & tdls_prohibited_bit)
		return _TRUE;

	return _FALSE;
}

int check_ap_tdls_ch_switching_prohibited(u8 *pframe, u8 pkt_len)
{
	u8 tdls_ch_swithcing_prohibited_bit = 0x80; /* bit(39); TDLS_channel_switching prohibited */

	if (pkt_len < 5)
		return _FALSE;

	pframe += 4;
	if ((*pframe) & tdls_ch_swithcing_prohibited_bit)
		return _TRUE;

	return _FALSE;
}

u8 rtw_is_tdls_enabled(_adapter *padapter)
{
	return padapter->registrypriv.en_tdls;
}

void rtw_set_tdls_enable(_adapter *padapter, u8 enable)
{
	padapter->registrypriv.en_tdls = enable;
	RTW_INFO("%s: en_tdls = %d\n", __func__, rtw_is_tdls_enabled(padapter));
}

void rtw_enable_tdls_func(_adapter *padapter)
{
	if (rtw_is_tdls_enabled(padapter) == _TRUE)
		return;
	rtw_set_tdls_enable(padapter, _TRUE);
}

void rtw_disable_tdls_func(_adapter *padapter, u8 enqueue_cmd)
{
	if (rtw_is_tdls_enabled(padapter) == _FALSE)
		return;

	rtw_free_all_tdls_sta(padapter, enqueue_cmd);
	rtw_reset_tdls_info(padapter);

	rtw_set_tdls_enable(padapter, _FALSE);
}

u8 rtw_is_tdls_sta_existed(_adapter *padapter)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta;
	int i = 0;
	_list	*plist, *phead;
	u8 ret = _FALSE;

	if (rtw_is_tdls_enabled(padapter) == _FALSE)
		return _FALSE;

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);
			if (psta->tdls_sta_state != TDLS_STATE_NONE) {
				ret = _TRUE;
				goto Exit;
			}
		}
	}

Exit:

	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	return ret;
}

u8 rtw_tdls_is_setup_allowed(_adapter *padapter)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	if (is_client_associated_to_ap(padapter) == _FALSE)
		return _FALSE;

	if (ptdlsinfo->ap_prohibited == _TRUE)
		return _FALSE;

	return _TRUE;
}

#ifdef CONFIG_TDLS_CH_SW
u8 rtw_tdls_is_chsw_allowed(_adapter *padapter)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;

	if (ptdlsinfo->ch_switch_prohibited == _TRUE)
		return _FALSE;

	if (padapter->registrypriv.wifi_spec == 0)
		return _FALSE;

	return _TRUE;
}
#endif

void _update_tdls_mgntframe_attrib(_adapter *padapter, struct sta_info *sta,
				struct pkt_attrib *pattrib)
{
	u8	wireless_mode;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct xmit_priv		*pxmitpriv = &padapter->xmitpriv;
	struct sta_info		*bmc_sta = NULL;
	struct _ADAPTER_LINK *padapter_link = sta->padapter_link;

	pattrib->type = WIFI_MGT_TYPE;
	pattrib->hdrlen = 24;
	pattrib->nr_frags = 1;
	pattrib->priority = 8;
	pattrib->mac_id = sta->phl_sta->macid;
	pattrib->adapter_link = padapter_link;
	pattrib->pktlen = 0;

	if (IS_CCK_RATE(padapter_link->mlmeextpriv.tx_rate))
		wireless_mode = WLAN_MD_11B;
	else
		wireless_mode = WLAN_MD_11G;
	pattrib->rate = padapter_link->mlmeextpriv.tx_rate;

	pattrib->encrypt = _NO_PRIVACY_;
	pattrib->bswenc = _FALSE;

	pattrib->qos_en = _FALSE;
	pattrib->ht_en = _FALSE;
	pattrib->bwmode = CHANNEL_WIDTH_20;
	pattrib->ch_offset = CHAN_OFFSET_NO_EXT;
	pattrib->sgi = _FALSE;

	pattrib->seqnum = pmlmeext->mgnt_seq;

	pattrib->retry_ctrl = _TRUE;

	pattrib->mbssid = 0;
	pattrib->hw_ssn_sel = pxmitpriv->hw_ssn_seq_no;
}

int _issue_nulldata_to_TDLS_peer_STA(_adapter *padapter, struct sta_info *sta,
		unsigned int power_mode, int wait_ms)
{
	int ret = _FAIL;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl, *qc;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct _ADAPTER_LINK *padapter_link = sta->padapter_link;
	struct link_mlme_ext_info	*lmlmeinfo = &(padapter_link->mlmeextpriv.mlmext_info);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;
	_update_tdls_mgntframe_attrib(padapter, sta, pattrib);

	pattrib->hdrlen += 2;
	pattrib->qos_en = _TRUE;
	pattrib->eosp = 1;
	pattrib->ack_policy = 0;
	pattrib->mdata = 0;
	pattrib->retry_ctrl = _FALSE;

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if (power_mode)
		SetPwrMgt(fctrl);

	qc = (unsigned short *)(pframe + pattrib->hdrlen - 2);

	SetPriority(qc, 7);	/* Set priority to VO */

	SetEOSP(qc, pattrib->eosp);

	SetAckpolicy(qc, pattrib->ack_policy);

	_rtw_memcpy(pwlanhdr->addr1, sta->phl_sta->mac_addr, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, padapter_link->mac_addr, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, get_my_bssid(&(lmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_QOS_DATA_NULL);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr_qos);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr_qos);

	pattrib->last_txcmdsz = pattrib->pktlen;

	if (wait_ms)
		ret = dump_mgntframe_and_wait_ack_timeout(padapter, pmgntframe, wait_ms);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;

}

/*
 *wait_ms == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 *wait_ms > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 *try_cnt means the maximal TX count to try
 */
int issue_nulldata_to_TDLS_peer_STA(_adapter *padapter, struct sta_info *sta,
		unsigned int power_mode, int try_cnt, int wait_ms)
{
	int ret;
	int i = 0;
	systime start = rtw_get_current_time();
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

#if 0
	psta = rtw_get_stainfo(&padapter->stapriv, da);
	if (psta) {
		if (power_mode)
			rtw_hal_macid_sleep(padapter, psta->phl_sta->macid);
		else
			rtw_hal_macid_wakeup(padapter, psta->phl_sta->macid);
	} else {
		RTW_INFO(FUNC_ADPT_FMT ": Can't find sta info for " MAC_FMT ", skip macid %s!!\n",
			FUNC_ADPT_ARG(padapter), MAC_ARG(da), power_mode ? "sleep" : "wakeup");
		rtw_warn_on(1);
	}
#endif

	do {
		ret = _issue_nulldata_to_TDLS_peer_STA(padapter, sta, power_mode, wait_ms);

		i++;

		if (RTW_CANNOT_RUN(adapter_to_dvobj(padapter)))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && (ret == _FAIL || wait_ms == 0));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		goto exit;
#endif
	}

	if (try_cnt && wait_ms) {
		if (sta)
			RTW_INFO(FUNC_ADPT_FMT" to "MAC_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), MAC_ARG(sta->phl_sta->mac_addr), rtw_get_oper_ch(padapter, sta->padapter_link),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
		else
			RTW_INFO(FUNC_ADPT_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), rtw_get_oper_ch(padapter, sta->padapter_link),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:
	return ret;
}

/* TDLS encryption(if needed) will always be CCMP */
void rtw_tdls_set_key(_adapter *padapter, struct sta_info *ptdls_sta)
{
	ptdls_sta->dot118021XPrivacy = _AES_;
	rtw_setstakey_cmd(padapter, ptdls_sta, TDLS_KEY, _TRUE);
}

#ifdef CONFIG_80211N_HT
void rtw_tdls_process_ht_cap(_adapter *padapter, struct sta_info *ptdls_sta, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct _ADAPTER_LINK	*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &padapter_link->mlmeextpriv;
	struct link_mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct link_mlme_priv		*pmlmepriv = &padapter_link->mlmepriv;
	struct ht_priv			*phtpriv = &pmlmepriv->htpriv;
	u8	max_AMPDU_len, min_MPDU_spacing;
	u8	cur_ldpc_cap = 0, cur_stbc_cap = 0, cur_beamform_cap = 0;

	/* Save HT capabilities in the sta object */
	_rtw_memset(&ptdls_sta->htpriv.ht_cap, 0, sizeof(struct rtw_ieee80211_ht_cap));
	if (pIE->data && pIE->Length >= sizeof(struct rtw_ieee80211_ht_cap)) {
		ptdls_sta->flags |= WLAN_STA_HT;
		ptdls_sta->flags |= WLAN_STA_WME;

		_rtw_memcpy(&ptdls_sta->htpriv.ht_cap, pIE->data, sizeof(struct rtw_ieee80211_ht_cap));
	} else {
		ptdls_sta->flags &= ~WLAN_STA_HT;
		return;
	}

	if (ptdls_sta->flags & WLAN_STA_HT) {
		if (padapter->registrypriv.ht_enable == _TRUE && is_supported_ht(padapter->registrypriv.wireless_mode) ) {
			ptdls_sta->htpriv.ht_option = _TRUE;
			ptdls_sta->qos_option = _TRUE;
		} else {
			ptdls_sta->htpriv.ht_option = _FALSE;
			ptdls_sta->qos_option = _FALSE;
		}
	}

	/* HT related cap */
	if (ptdls_sta->htpriv.ht_option) {
		/* Check if sta supports rx ampdu */
		if (padapter->registrypriv.ampdu_enable == 1)
			ptdls_sta->ampdu_priv.ampdu_enable = _TRUE;

		/* AMPDU Parameters field */

		/* Get MIN of MAX AMPDU Length Exp */
		max_AMPDU_len = GET_HT_CAP_ELE_MAX_AMPDU_LEN_EXP(pIE->data);
		max_AMPDU_len = rtw_min((pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x3), max_AMPDU_len);

		/* Get MAX of MIN MPDU Start Spacing */
		min_MPDU_spacing = GET_HT_CAP_ELE_MIN_MPDU_S_SPACE(pIE->data);
		min_MPDU_spacing = rtw_max((pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & 0x1c), min_MPDU_spacing);
		ptdls_sta->ampdu_priv.rx_ampdu_min_spacing = max_AMPDU_len | min_MPDU_spacing;

		/* Check if sta support s Short GI 20M */
		if ((phtpriv->sgi_20m == _TRUE) && (ptdls_sta->htpriv.ht_cap.cap_info & cpu_to_le16(IEEE80211_HT_CAP_SGI_20)))
			ptdls_sta->htpriv.sgi_20m = _TRUE;

		/* Check if sta support s Short GI 40M */
		if ((phtpriv->sgi_40m == _TRUE) && (ptdls_sta->htpriv.ht_cap.cap_info & cpu_to_le16(IEEE80211_HT_CAP_SGI_40)))
			ptdls_sta->htpriv.sgi_40m = _TRUE;

		/* Bwmode would still followed AP's setting */
		if (ptdls_sta->htpriv.ht_cap.cap_info & cpu_to_le16(IEEE80211_HT_CAP_SUP_WIDTH)) {
			if (pmlmeext->chandef.bw >= CHANNEL_WIDTH_40)
				ptdls_sta->phl_sta->chandef.bw = CHANNEL_WIDTH_40;
			ptdls_sta->htpriv.ch_offset = pmlmeext->chandef.offset;
		}

		/* Config LDPC Coding Capability */
		if (TEST_FLAG(phtpriv->ldpc_cap, LDPC_HT_ENABLE_TX) && GET_HT_CAP_ELE_LDPC_CAP(pIE->data)) {
			SET_FLAG(cur_ldpc_cap, (LDPC_HT_ENABLE_TX | LDPC_HT_CAP_TX));
			RTW_INFO("Enable HT Tx LDPC!\n");
		}
		ptdls_sta->htpriv.ldpc_cap = cur_ldpc_cap;

		/* Config STBC setting */
		if (TEST_FLAG(phtpriv->stbc_cap, STBC_HT_ENABLE_TX) && GET_HT_CAP_ELE_RX_STBC(pIE->data)) {
			SET_FLAG(cur_stbc_cap, (STBC_HT_ENABLE_TX | STBC_HT_CAP_TX));
			RTW_INFO("Enable HT Tx STBC!\n");
		}
		ptdls_sta->htpriv.stbc_cap = cur_stbc_cap;

#ifdef CONFIG_BEAMFORMING
		/* Config Tx beamforming setting */
		if (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE) &&
			GET_HT_CAP_TXBF_EXPLICIT_COMP_STEERING_CAP(pIE->data))
			SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE);

		if (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE) &&
			GET_HT_CAP_TXBF_EXPLICIT_COMP_FEEDBACK_CAP(pIE->data))
			SET_FLAG(cur_beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE);
		ptdls_sta->htpriv.beamform_cap = cur_beamform_cap;
		if (cur_beamform_cap)
			RTW_INFO("Client HT Beamforming Cap = 0x%02X\n", cur_beamform_cap);
#endif /* CONFIG_BEAMFORMING */
	}

}

u8 *rtw_tdls_set_ht_cap(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	struct _ADAPTER_LINK *padapter_link = pattrib->adapter_link;
	rtw_ht_use_default_setting(padapter, padapter_link);

	if (padapter->registrypriv.wifi_spec == 1) {
		padapter_link->mlmepriv.htpriv.sgi_20m = _FALSE;
		padapter_link->mlmepriv.htpriv.sgi_40m = _FALSE;
	}

	rtw_restructure_ht_ie(padapter, padapter_link, NULL, pframe, 0, &(pattrib->pktlen), padapter_link->mlmeextpriv.chandef.chan);

	return pframe + pattrib->pktlen;
}
#endif

#ifdef CONFIG_80211AC_VHT
void rtw_tdls_process_vht_cap(_adapter *padapter, struct sta_info *ptdls_sta, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct _ADAPTER_LINK	*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_priv		*pmlmepriv = &padapter_link->mlmepriv;
	struct vht_priv			*pvhtpriv = &pmlmepriv->vhtpriv;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	u8	cur_ldpc_cap = 0, cur_stbc_cap = 0, tx_nss = 0;
	u16 cur_beamform_cap = 0;
	u8	*pcap_mcs;

	if (pIE == NULL)
		return;

	_rtw_memset(&ptdls_sta->vhtpriv, 0, sizeof(struct vht_priv));
	if (pIE->data && pIE->Length == 12) {
		ptdls_sta->flags |= WLAN_STA_VHT;

		_rtw_memcpy(ptdls_sta->vhtpriv.vht_cap, pIE->data, 12);

#if 0
		if (elems.vht_op_mode_notify && elems.vht_op_mode_notify_len == 1)
			_rtw_memcpy(&pstat->vhtpriv.vht_op_mode_notify, elems.vht_op_mode_notify, 1);
		else /* for Frame without Operating Mode notify ie; default: 80M */
			pstat->vhtpriv.vht_op_mode_notify = CHANNEL_WIDTH_80;
#else
		ptdls_sta->vhtpriv.vht_op_mode_notify = CHANNEL_WIDTH_80;
#endif
	} else {
		ptdls_sta->flags &= ~WLAN_STA_VHT;
		return;
	}

	if (ptdls_sta->flags & WLAN_STA_VHT) {
		if (REGSTY_IS_11AC_ENABLE(&padapter->registrypriv)
		    && is_supported_vht(padapter->registrypriv.wireless_mode)
		    && RFCTL_REG_EN_11AC(rfctl)
		) {
			ptdls_sta->vhtpriv.vht_option = _TRUE;
			/* ToDo: need to API to inform hal_sta->ra_info.is_vht_enable  */
			#if 0
			ptdls_sta->phl_sta->ra_info.is_vht_enable = _TRUE;
			#endif
		}
		else
			ptdls_sta->vhtpriv.vht_option = _FALSE;
	}

	/* Follows VHT_caps_handler */
	if (ptdls_sta->vhtpriv.vht_option == _FALSE)
		return;

	if (ptdls_sta->phl_sta == NULL)
		return;

	phl_sta = ptdls_sta->phl_sta;

	/* B4 Rx LDPC */
	if (TEST_FLAG(pvhtpriv->ldpc_cap, LDPC_VHT_ENABLE_TX) &&
	    GET_VHT_CAPABILITY_ELE_RX_LDPC(pIE->data)) {
		SET_FLAG(cur_ldpc_cap, (LDPC_VHT_ENABLE_TX | LDPC_VHT_CAP_TX));
		RTW_INFO("Current VHT LDPC Setting = %02X\n", cur_ldpc_cap);
	}
	ptdls_sta->vhtpriv.ldpc_cap = cur_ldpc_cap;

	/* B5 Short GI for 80 MHz */
	ptdls_sta->vhtpriv.sgi_80m = (GET_VHT_CAPABILITY_ELE_SHORT_GI80M(pIE->data) & pvhtpriv->sgi_80m) ? _TRUE : _FALSE;
	/*RTW_INFO("Current ShortGI80MHz = %d\n", pvhtpriv->sgi_80m);*/

	/* B8 B9 B10 Rx STBC */
	if (TEST_FLAG(pvhtpriv->stbc_cap, STBC_VHT_ENABLE_TX) &&
		GET_VHT_CAPABILITY_ELE_RX_STBC(pIE->data)) {
		SET_FLAG(cur_stbc_cap, (STBC_VHT_ENABLE_TX | STBC_VHT_CAP_TX));
		RTW_INFO("Current VHT STBC Setting = %02X\n", cur_stbc_cap);
	}
	ptdls_sta->vhtpriv.stbc_cap = cur_stbc_cap;

	phl_sta->asoc_cap.vht_su_bfmr = GET_VHT_CAPABILITY_ELE_SU_BFER(pIE->data);
	phl_sta->asoc_cap.vht_su_bfme = GET_VHT_CAPABILITY_ELE_SU_BFEE(pIE->data);
	phl_sta->asoc_cap.vht_mu_bfmr = GET_VHT_CAPABILITY_ELE_MU_BFER(pIE->data);
	phl_sta->asoc_cap.bfme_sts = GET_VHT_CAPABILITY_ELE_SU_BFEE_STS_CAP(pIE->data);
	phl_sta->asoc_cap.num_snd_dim = GET_VHT_CAPABILITY_ELE_SU_BFER_SOUND_DIM_NUM(pIE->data);

	RTW_INFO("%s: VHT STA assoc_cap:\n", __func__);
	RTW_INFO("- SU BFer: %d\n", phl_sta->asoc_cap.vht_su_bfmr);
	RTW_INFO("- SU BFee: %d\n", phl_sta->asoc_cap.vht_su_bfme);
	RTW_INFO("- MU BFer: %d\n", phl_sta->asoc_cap.vht_mu_bfmr);
	RTW_INFO("- BFee STS: %d\n", phl_sta->asoc_cap.bfme_sts);
	RTW_INFO("- BFer SND DIM number: %d\n", phl_sta->asoc_cap.num_snd_dim);

#ifdef CONFIG_BEAMFORMING
	/*
	 * B11 SU Beamformer Capable,
	 * the target supports Beamformer and we are Beamformee
	 */
	if (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_BEAMFORMEE_ENABLE)
		&& GET_VHT_CAPABILITY_ELE_SU_BFER(pIE->data)) {
		SET_FLAG(cur_beamform_cap, BEAMFORMING_VHT_BEAMFORMEE_ENABLE);

		/* Shift to BEAMFORMING_VHT_BEAMFORMEE_STS_CAP */
		SET_FLAG(cur_beamform_cap, GET_VHT_CAPABILITY_ELE_SU_BFEE_STS_CAP(pIE->data) << 8);

		/*
		 * B19 MU Beamformer Capable,
		 * the target supports Beamformer and we are Beamformee
		 */
		if (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_MU_MIMO_STA_ENABLE)
			&& GET_VHT_CAPABILITY_ELE_MU_BFER(pIE->data))
			SET_FLAG(cur_beamform_cap, BEAMFORMING_VHT_MU_MIMO_STA_ENABLE);
	}

	/*
	 * B12 SU Beamformee Capable,
	 * the target supports Beamformee and we are Beamformer
	 */
	if (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_BEAMFORMER_ENABLE)
		&& GET_VHT_CAPABILITY_ELE_SU_BFEE(pIE->data)) {
		SET_FLAG(cur_beamform_cap, BEAMFORMING_VHT_BEAMFORMER_ENABLE);

		/* Shit to BEAMFORMING_VHT_BEAMFORMER_SOUND_DIM */
		SET_FLAG(cur_beamform_cap, GET_VHT_CAPABILITY_ELE_SU_BFER_SOUND_DIM_NUM(pIE->data) << 12);

		/*
		 * B20 MU Beamformee Capable,
		 * the target supports Beamformee and we are Beamformer
		 */
		if (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_MU_MIMO_AP_ENABLE)
			&& GET_VHT_CAPABILITY_ELE_MU_BFEE(pIE->data))
			SET_FLAG(cur_beamform_cap, BEAMFORMING_VHT_MU_MIMO_AP_ENABLE);
	}

	ptdls_sta->vhtpriv.beamform_cap = cur_beamform_cap;
	RTW_INFO("Current VHT Beamforming Setting=0x%04X\n", cur_beamform_cap);
	/* ToDo: need to API to inform hal_sta->bf_info.vht_beamform_cap  */
	#if 0
	ptdls_sta->phl_sta->bf_info.vht_beamform_cap = cur_beamform_cap;
	#endif
#endif /* CONFIG_BEAMFORMING */

	/* B0 B1 Maximum MPDU Length */
	ptdls_sta->vhtpriv.max_mpdu_len = GET_VHT_CAPABILITY_ELE_MAX_MPDU_LENGTH(pIE->data);
	/* B23 B24 B25 Maximum A-MPDU Length Exponent */
	ptdls_sta->vhtpriv.ampdu_len = GET_VHT_CAPABILITY_ELE_MAX_RXAMPDU_FACTOR(pIE->data); 

	pcap_mcs = GET_VHT_CAPABILITY_ELE_RX_MCS(pIE->data);
	tx_nss = get_phy_tx_nss(padapter, padapter_link);
	rtw_vht_nss_to_mcsmap(tx_nss, ptdls_sta->vhtpriv.vht_mcs_map, pcap_mcs);
	ptdls_sta->vhtpriv.vht_highest_rate = rtw_get_vht_highest_rate(ptdls_sta->vhtpriv.vht_mcs_map);
}

void rtw_tdls_process_vht_operation(_adapter *padapter, struct sta_info *ptdls_sta, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct registry_priv *regsty = &padapter->registrypriv;
	struct _ADAPTER_LINK	*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_priv		*pmlmepriv = &padapter_link->mlmepriv;
	struct link_mlme_ext_priv	*pmlmeext = &padapter_link->mlmeextpriv;
	u8 operation_bw = 0;

	if (GET_VHT_OPERATION_ELE_CHL_WIDTH(pIE->data) >= CH_WIDTH_80_160M) {

		operation_bw = CHANNEL_WIDTH_80;

		if (rtw_hw_is_bw_support(adapter_to_dvobj(padapter), operation_bw) && REGSTY_IS_BW_5G_SUPPORT(regsty, operation_bw)
			&& (operation_bw <= pmlmeext->chandef.bw))
			ptdls_sta->phl_sta->chandef.bw = operation_bw;
		else
			ptdls_sta->phl_sta->chandef.bw = pmlmeext->chandef.bw;
	} else
		ptdls_sta->phl_sta->chandef.bw = pmlmeext->chandef.bw;
	_rtw_memcpy(ptdls_sta->vhtpriv.vht_op, pIE->data,
			pIE->Length > VHT_OP_IE_LEN ? VHT_OP_IE_LEN : pIE->Length);
}

void rtw_tdls_process_vht_op_mode_notify(_adapter *padapter, struct sta_info *ptdls_sta, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct registry_priv *regsty = &padapter->registrypriv;
	struct _ADAPTER_LINK	*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_priv		*pmlmepriv = &padapter_link->mlmepriv;
	struct vht_priv		*pvhtpriv = &pmlmepriv->vhtpriv;
	struct link_mlme_ext_priv	*pmlmeext = &padapter_link->mlmeextpriv;
	u8	target_bw;
	u8	target_rxss, current_rxss;

	if (pvhtpriv->vht_option == _FALSE)
		return;

	target_bw = GET_VHT_OPERATING_MODE_FIELD_CHNL_WIDTH(pIE->data);
	target_rxss = (GET_VHT_OPERATING_MODE_FIELD_RX_NSS(pIE->data) + 1);

	if (rtw_hw_is_bw_support(adapter_to_dvobj(padapter), target_bw) && REGSTY_IS_BW_5G_SUPPORT(regsty, target_bw)
		&& (target_bw <= pmlmeext->chandef.bw))
		ptdls_sta->phl_sta->chandef.bw = target_bw;
	else
		ptdls_sta->phl_sta->chandef.bw = pmlmeext->chandef.bw;

	current_rxss = rtw_vht_mcsmap_to_nss(ptdls_sta->vhtpriv.vht_mcs_map);
	if (target_rxss != current_rxss) {
		u8	vht_mcs_map[2] = {};

		rtw_vht_nss_to_mcsmap(target_rxss, vht_mcs_map, ptdls_sta->vhtpriv.vht_mcs_map);
		_rtw_memcpy(ptdls_sta->vhtpriv.vht_mcs_map, vht_mcs_map, 2);
	}
}

u8 *rtw_tdls_set_aid(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	return rtw_set_ie(pframe, EID_AID, 2, (u8 *)&(padapter->mlmepriv.dev_cur_network.aid), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_vht_cap(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	u32 ie_len = 0;

	rtw_vht_get_real_setting(padapter, pattrib->adapter_link);

	ie_len = rtw_build_vht_cap_ie(padapter, pattrib->adapter_link, pframe);
	pattrib->pktlen += ie_len;

	return pframe + ie_len;
}

u8 *rtw_tdls_set_vht_operation(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib, enum band_type band, u8 channel)
{
	u32 ie_len = 0;

	ie_len = rtw_build_vht_operation_ie(padapter, pattrib->adapter_link, pframe, band, channel);
	pattrib->pktlen += ie_len;

	return pframe + ie_len;
}

u8 *rtw_tdls_set_vht_op_mode_notify(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib, u8 bw)
{
	u32 ie_len = 0;

	ie_len = rtw_build_vht_op_mode_notify_ie(padapter, pattrib->adapter_link, pframe, bw);
	pattrib->pktlen += ie_len;

	return pframe + ie_len;
}
#endif

#ifdef CONFIG_80211AX_HE
u8 *rtw_tdls_set_he_cap(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	u32 ie_len = 0;
	rtw_he_use_default_setting(padapter, pattrib->adapter_link);
	ie_len = rtw_build_he_cap_ie(padapter, pattrib->adapter_link, pframe,
				     pattrib->adapter_link->mlmeextpriv.chandef.band);
	pattrib->pktlen += ie_len;

	return pframe + pattrib->pktlen;
}

u8 *rtw_tdls_set_he_operation(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	u32 ie_len = 0;

	ie_len = rtw_build_he_operation_ie(padapter, pattrib->adapter_link,
			pframe, &pattrib->adapter_link->mlmeextpriv.chandef, 0);
	pattrib->pktlen += ie_len;

	return pframe + ie_len;
}

/* Follows HE_caps_handler */
void rtw_tdls_process_he_cap(_adapter *padapter, struct sta_info *ptdls_sta, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct _ADAPTER_LINK *padapter_link = ptdls_sta->padapter_link;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	u8 supp_mcs_len = 4;
	u8 *ele_start = (&(pIE->data[0]) + 1);

	if (pIE == NULL)
		return;

	_rtw_memset(&ptdls_sta->hepriv, 0, sizeof(struct he_priv));
	if (pIE->data && pIE->Length >= 22 && pIE->Length <= HE_CAP_ELE_MAX_LEN) {
		ptdls_sta->flags |= WLAN_STA_HE;
		_rtw_memcpy(ptdls_sta->hepriv.he_cap, pIE->data, pIE->Length);

	} else {
		ptdls_sta->flags &= ~WLAN_STA_HE;
		return;
	}

	if (ptdls_sta->flags & WLAN_STA_HE) {
		if (REGSTY_IS_11AX_ENABLE(&padapter->registrypriv)
			&& is_supported_he(padapter->registrypriv.wireless_mode)
			/* CONFIG_80211AX_HE_TODO
			&& (!rfctl->country_ent || COUNTRY_CHPLAN_EN_11AX(rfctl->country_ent)) */
			) {
			ptdls_sta->hepriv.he_option = _TRUE;
			/* ToDo: need to API to inform hal_sta->ra_info.is_he_enable  */
			#if 0
			ptdls_sta->phl_sta->ra_info.is_he_enable = _TRUE;
			#endif
		}
		else
			ptdls_sta->hepriv.he_option = _FALSE;
	}

	if (ptdls_sta->hepriv.he_option == _FALSE)
		return;

	if (ptdls_sta->phl_sta == NULL)
		return;

	phl_sta = ptdls_sta->phl_sta;

	/* HE MAC Caps */
	HE_mac_caps_handler(padapter, phl_sta, ele_start);
	ele_start += HE_CAP_ELE_MAC_CAP_LEN;

	/* HE PHY Caps */
	HE_phy_caps_handler(padapter, phl_sta, ele_start, &supp_mcs_len);
	ele_start += HE_CAP_ELE_PHY_CAP_LEN;

	/* HE Supp MCS Set */
	rtw_he_set_asoc_cap_supp_mcs(padapter, phl_sta, ele_start, supp_mcs_len);
	ptdls_sta->hepriv.he_highest_rate = rtw_he_get_highest_rate(phl_sta->asoc_cap.he_rx_mcs);
	/* HE_supp_mcs_handler(padapter, phl_sta, ele_start, supp_mcs_len); */
	ele_start += supp_mcs_len;

	/* HE PPE Thresholds */
	HE_ppe_thre_handler(padapter, phl_sta, ele_start);

}

/* Follows HE_operation_handler */
void rtw_tdls_process_he_operation(_adapter *padapter, struct sta_info *ptdls_sta, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct _ADAPTER_LINK	*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_priv	*pmlmepriv = &padapter_link->mlmepriv;
	struct he_priv		*phepriv = &pmlmepriv->hepriv;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	u8 *ele_start = (&(pIE->data[0]) + 1);

	if (phepriv->he_option == _FALSE)
		return;

	if (ptdls_sta->phl_sta == NULL)
		return;

	if (pIE->data && pIE->Length >= 7 && pIE->Length <= HE_OPER_ELE_MAX_LEN)
		_rtw_memcpy(ptdls_sta->hepriv.he_op, pIE->data, pIE->Length);

	phl_sta = ptdls_sta->phl_sta;

	phl_sta->asoc_cap.er_su = !GET_HE_OP_PARA_ER_SU_DISABLE(ele_start);
	phl_sta->asoc_cap.bsscolor = GET_HE_OP_BSS_COLOR_INFO_BSS_COLOR(ele_start);
	phl_sta->tf_trs = _TRUE;
}
#endif

u8 *rtw_tdls_set_sup_ch(_adapter *adapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	return rtw_chset_set_spt_chs_ie(adapter_to_chset(adapter), pframe, &(pattrib->pktlen));
}

u8 *rtw_tdls_set_rsnie(struct tdls_txmgmt *ptxmgmt, u8 *pframe, struct pkt_attrib *pattrib,  int init, struct sta_info *ptdls_sta)
{
	u8 *p = NULL;
	int len = 0;

	if (ptxmgmt->len > 0)
		p = rtw_get_ie(ptxmgmt->buf, _RSN_IE_2_, &len, ptxmgmt->len);

	if (p != NULL)
		return rtw_set_ie(pframe, _RSN_IE_2_, len, p + 2, &(pattrib->pktlen));
	else if (init == _TRUE)
		return rtw_set_ie(pframe, _RSN_IE_2_, sizeof(TDLS_RSNIE), TDLS_RSNIE, &(pattrib->pktlen));
	else
		return rtw_set_ie(pframe, _RSN_IE_2_, sizeof(ptdls_sta->TDLS_RSNIE), ptdls_sta->TDLS_RSNIE, &(pattrib->pktlen));
}

u8 *rtw_tdls_set_ext_cap(u8 *pframe, struct pkt_attrib *pattrib)
{
	return rtw_set_ie(pframe, WLAN_EID_EXT_CAP , sizeof(TDLS_EXT_CAPIE), TDLS_EXT_CAPIE, &(pattrib->pktlen));
}

u8 *rtw_tdls_set_qos_cap(u8 *pframe, struct pkt_attrib *pattrib)
{
	return rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, sizeof(TDLS_WMMIE), TDLS_WMMIE,  &(pattrib->pktlen));
}

u8 *rtw_tdls_set_ftie(struct tdls_txmgmt *ptxmgmt, u8 *pframe, struct pkt_attrib *pattrib, u8 *ANonce, u8 *SNonce)
{
	struct wpa_tdls_ftie FTIE = {0};
	u8 *p = NULL;
	int len = 0;

	if (ptxmgmt->len > 0)
		p = rtw_get_ie(ptxmgmt->buf, _FTIE_, &len, ptxmgmt->len);

	if (p != NULL)
		return rtw_set_ie(pframe, _FTIE_, len, p + 2, &(pattrib->pktlen));
	else {
		if (ANonce != NULL)
			_rtw_memcpy(FTIE.Anonce, ANonce, WPA_NONCE_LEN);
		if (SNonce != NULL)
			_rtw_memcpy(FTIE.Snonce, SNonce, WPA_NONCE_LEN);

		return rtw_set_ie(pframe, _FTIE_, TDLS_FTIE_DATA_LEN,
						  (u8 *)FTIE.data, &(pattrib->pktlen));
	}
}

u8 *rtw_tdls_set_timeout_interval(struct tdls_txmgmt *ptxmgmt, u8 *pframe, struct pkt_attrib *pattrib, int init, struct sta_info *ptdls_sta)
{
	u8 timeout_itvl[5];	/* set timeout interval to maximum value */
	u32 timeout_interval = TDLS_TPK_RESEND_COUNT;
	u8 *p = NULL;
	int len = 0;

	if (ptxmgmt->len > 0)
		p = rtw_get_ie(ptxmgmt->buf, _TIMEOUT_ITVL_IE_, &len, ptxmgmt->len);

	if (p != NULL)
		return rtw_set_ie(pframe, _TIMEOUT_ITVL_IE_, len, p + 2, &(pattrib->pktlen));
	else {
		/* Timeout interval */
		timeout_itvl[0] = 0x02;
		if (init == _TRUE)
			_rtw_memcpy(timeout_itvl + 1, &timeout_interval, 4);
		else
			_rtw_memcpy(timeout_itvl + 1, (u8 *)(&ptdls_sta->TDLS_PeerKey_Lifetime), 4);

		return rtw_set_ie(pframe, _TIMEOUT_ITVL_IE_, 5, timeout_itvl, &(pattrib->pktlen));
	}
}

u8 *rtw_tdls_set_bss_coexist(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	u8 iedata = 0;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);

	if (padapter_link->mlmepriv.num_FortyMHzIntolerant > 0)
		iedata |= BIT(2);	/* 20 MHz BSS Width Request */

	/* Information Bit should be set by TDLS test plan 5.9 */
	iedata |= BIT(0);
	return rtw_set_ie(pframe, WLAN_EID_20_40_BSS_COEXISTENCE, 1, &iedata, &(pattrib->pktlen));
}

u8 *rtw_tdls_set_payload_type(u8 *pframe, struct pkt_attrib *pattrib)
{
	u8 payload_type = 0x02;
	return rtw_set_fixed_ie(pframe, 1, &(payload_type), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_category(u8 *pframe, struct pkt_attrib *pattrib, u8 category)
{
	return rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_action(u8 *pframe, struct pkt_attrib *pattrib, struct tdls_txmgmt *ptxmgmt)
{
	return rtw_set_fixed_ie(pframe, 1, &(ptxmgmt->action_code), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_status_code(u8 *pframe, struct pkt_attrib *pattrib, struct tdls_txmgmt *ptxmgmt)
{
	return rtw_set_fixed_ie(pframe, 2, (u8 *)&(ptxmgmt->status_code), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_dialog(u8 *pframe, struct pkt_attrib *pattrib, struct tdls_txmgmt *ptxmgmt)
{
	u8 dialogtoken = 1;
	if (ptxmgmt->dialog_token)
		return rtw_set_fixed_ie(pframe, 1, &(ptxmgmt->dialog_token), &(pattrib->pktlen));
	else
		return rtw_set_fixed_ie(pframe, 1, &(dialogtoken), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_reg_class(u8 *pframe, struct pkt_attrib *pattrib, struct sta_info *ptdls_sta)
{
	u8 reg_class = 22;
	return rtw_set_fixed_ie(pframe, 1, &(reg_class), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_second_channel_offset(u8 *pframe, struct pkt_attrib *pattrib, u8 ch_offset)
{
	return rtw_set_ie(pframe, WLAN_EID_SECONDARY_CHANNEL_OFFSET, 1, &ch_offset, &(pattrib->pktlen));
}

u8 *rtw_tdls_set_capability(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	struct _ADAPTER_LINK	*padapter_link = pattrib->adapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &padapter_link->mlmeextpriv;
	struct link_mlme_ext_info	*pmlmeinfo = &pmlmeext->mlmext_info;
	u8 cap_from_ie[2] = {0};

	_rtw_memcpy(cap_from_ie, rtw_get_capability_from_ie(pmlmeinfo->network.IEs), 2);

	return rtw_set_fixed_ie(pframe, 2, cap_from_ie, &(pattrib->pktlen));
}

u8 *rtw_tdls_set_supported_rate(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	u8 bssrate[NDIS_802_11_LENGTH_RATES_EX];
	int bssrate_len = 0;
	u8 more_supportedrates = 0;
	struct _ADAPTER_LINK *padapter_link = pattrib->adapter_link;

	rtw_set_supported_rate(bssrate,
		(padapter->registrypriv.wireless_mode == WLAN_MD_MAX) ? padapter_link->mlmeextpriv.cur_wireless_mode : padapter->registrypriv.wireless_mode,
			GET_WIFI_ROLE_LINK_CURRENT_CH(padapter_link),
			GET_WIFI_ROLE_LINK_CURRENT_BAND(padapter_link));
	bssrate_len = rtw_get_rateset_len(bssrate);

	if (bssrate_len > 8) {
		pframe = rtw_set_ie(pframe, WLAN_EID_SUPP_RATES , 8, bssrate, &(pattrib->pktlen));
		more_supportedrates = 1;
	} else
		pframe = rtw_set_ie(pframe, WLAN_EID_SUPP_RATES , bssrate_len , bssrate, &(pattrib->pktlen));

	/* extended supported rates */
	if (more_supportedrates == 1)
		pframe = rtw_set_ie(pframe, WLAN_EID_EXT_SUPP_RATES , (bssrate_len - 8), (bssrate + 8), &(pattrib->pktlen));

	return pframe;
}

u8 *rtw_tdls_set_sup_reg_class(u8 *pframe, struct pkt_attrib *pattrib)
{
	return rtw_set_ie(pframe, _SRC_IE_ , sizeof(TDLS_SRC), TDLS_SRC, &(pattrib->pktlen));
}

u8 *rtw_tdls_set_linkid(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib, u8 init)
{
	struct _ADAPTER_LINK	*padapter_link = pattrib->adapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &(padapter_link->mlmeextpriv);
	struct link_mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	u8 link_id_addr[18] = {0};

	_rtw_memcpy(link_id_addr, get_my_bssid(&(pmlmeinfo->network)), 6);

	if (init == _TRUE) {
		_rtw_memcpy((link_id_addr + 6), pattrib->src, 6);
		_rtw_memcpy((link_id_addr + 12), pattrib->dst, 6);
	} else {
		_rtw_memcpy((link_id_addr + 6), pattrib->dst, 6);
		_rtw_memcpy((link_id_addr + 12), pattrib->src, 6);
	}
	return rtw_set_ie(pframe, _LINK_ID_IE_, 18, link_id_addr, &(pattrib->pktlen));
}

#ifdef CONFIG_TDLS_CH_SW
u8 *rtw_tdls_set_target_ch(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	u8 target_ch = 1;
	if (padapter->tdlsinfo.chsw_info.off_ch_num)
		return rtw_set_fixed_ie(pframe, 1, &(padapter->tdlsinfo.chsw_info.off_ch_num), &(pattrib->pktlen));
	else
		return rtw_set_fixed_ie(pframe, 1, &(target_ch), &(pattrib->pktlen));
}

u8 *rtw_tdls_set_ch_sw(u8 *pframe, struct pkt_attrib *pattrib, struct sta_info *ptdls_sta)
{
	u8 ch_switch_timing[4] = {0};
	u16 switch_time = (ptdls_sta->ch_switch_time >= TDLS_CH_SWITCH_TIME * 1000) ?
			  ptdls_sta->ch_switch_time : TDLS_CH_SWITCH_TIME;
	u16 switch_timeout = (ptdls_sta->ch_switch_timeout >= TDLS_CH_SWITCH_TIMEOUT * 1000) ?
		     ptdls_sta->ch_switch_timeout : TDLS_CH_SWITCH_TIMEOUT;

	_rtw_memcpy(ch_switch_timing, &switch_time, 2);
	_rtw_memcpy(ch_switch_timing + 2, &switch_timeout, 2);

	return rtw_set_ie(pframe, _CH_SWITCH_TIMING_,  4, ch_switch_timing, &(pattrib->pktlen));
}

void rtw_tdls_set_ch_sw_oper_control(_adapter *padapter, u8 enable)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct phl_info_t *phl_info = GET_PHL_INFO(dvobj);
	struct rtw_phl_com_t *phl_com = GET_PHL_COM(dvobj);
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);
	enum rtw_phl_status status;

	if (enable == _TRUE)
		rtw_phl_set_chsw_ofld_info(phl_com, _TRUE, _TRUE, _TRUE);
	else
		rtw_phl_set_chsw_ofld_info(phl_com, _FALSE, _FALSE, _FALSE);


	if (ATOMIC_READ(&padapter->tdlsinfo.chsw_info.chsw_on) != enable)
		ATOMIC_SET(&padapter->tdlsinfo.chsw_info.chsw_on, enable);

	/* Enable bcn erly report */
	RTW_INFO("[TDLS] %s Bcn Early C2H Report\n", (enable == _TRUE) ? "Start" : "Stop");
	status = rtw_phl_cmd_wrole_change(phl_info, padapter->phl_role, padapter_link->wrlink,
		WR_CHG_BCN_EARLY_RPT_CFG, (u8 *)&enable, sizeof(enable),
		PHL_CMD_DIRECTLY, 0);
}

void rtw_tdls_ch_sw_back_to_base_chnl(_adapter *padapter)
{
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);

	if ((ATOMIC_READ(&pchsw_info->chsw_on) == _TRUE) &&
	    (padapter_link->mlmeextpriv.chandef.chan != rtw_get_oper_ch(padapter, padapter_link)))
		rtw_tdls_cmd(padapter, pchsw_info->addr, TDLS_CH_SW_TO_BASE_CHNL_UNSOLICITED, 0);
}

static void rtw_tdls_chsw_oper_init(_adapter *padapter, u32 timeout_ms)
{
	struct submit_ctx	*chsw_sctx = &padapter->tdlsinfo.chsw_info.chsw_sctx;

	rtw_sctx_init(chsw_sctx, timeout_ms);
}

static int rtw_tdls_chsw_oper_wait(_adapter *padapter)
{
	struct submit_ctx	*chsw_sctx = &padapter->tdlsinfo.chsw_info.chsw_sctx;

	return rtw_sctx_wait(chsw_sctx, __func__);
}

void rtw_tdls_chsw_oper_done(_adapter *padapter)
{
	struct submit_ctx	*chsw_sctx = &padapter->tdlsinfo.chsw_info.chsw_sctx;

	rtw_sctx_done(&chsw_sctx);
}

s32 rtw_tdls_do_ch_sw(_adapter *padapter, struct sta_info *ptdls_sta, u8 chnl_type, u8 channel, u8 channel_offset, u16 bwmode, u16 ch_switch_time)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	u32 ch_sw_time_start, ch_sw_time_spent, wait_time;
	s32 ret = _SUCCESS;

	ch_sw_time_start = rtw_systime_to_ms(rtw_get_current_time());

	set_channel_bwmode(padapter, ptdls_sta->padapter_link, channel, channel_offset, bwmode, RFK_TYPE_FORCE_NOT_DO);
	ch_sw_time_spent = rtw_systime_to_ms(rtw_get_current_time()) - ch_sw_time_start;
	if (chnl_type == TDLS_CH_SW_OFF_CHNL) {
		if ((u32)ch_switch_time / 1000 > ch_sw_time_spent)
			wait_time = (u32)ch_switch_time / 1000 - ch_sw_time_spent;
		else
			wait_time = 0;
		if (wait_time > 0)
			rtw_msleep_os(wait_time);
	}

	return ret;
}
#endif

u8 *rtw_tdls_set_wmm_params(_adapter *padapter, u8 *pframe, struct pkt_attrib *pattrib)
{
	struct _ADAPTER_LINK	*padapter_link = pattrib->adapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &(padapter_link->mlmeextpriv);
	struct link_mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 wmm_param_ele[24] = {0};

	if (&pmlmeinfo->WMM_param) {
		_rtw_memcpy(wmm_param_ele, WMM_PARA_OUI, 6);
		if (_rtw_memcmp(&pmlmeinfo->WMM_param, &wmm_param_ele[6], 18) == _TRUE)
			/* Use default WMM Param */
			_rtw_memcpy(wmm_param_ele + 6, (u8 *)&TDLS_WMM_PARAM_IE, sizeof(TDLS_WMM_PARAM_IE));
		else
			_rtw_memcpy(wmm_param_ele + 6, (u8 *)&pmlmeinfo->WMM_param, sizeof(pmlmeinfo->WMM_param));
		return rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_,  24, wmm_param_ele, &(pattrib->pktlen));
	} else
		return pframe;
}

#ifdef CONFIG_WFD
void rtw_tdls_process_wfd_ie(struct tdls_info *ptdlsinfo, u8 *ptr, u8 length)
{
	u8 *wfd_ie;
	u32 wfd_ielen = 0;
	_adapter *adapter = tdls_info_to_adapter(ptdlsinfo);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	if (!rtw_hw_chk_wl_func(dvobj, WL_FUNC_MIRACAST))
		return;

	/* Try to get the TCP port information when receiving the negotiation response. */

	wfd_ie = rtw_get_wfd_ie(ptr, length, NULL, &wfd_ielen);
	while (wfd_ie) {
		u8 *attr_content;
		u32	attr_contentlen = 0;
		int	i;

		RTW_INFO("[%s] WFD IE Found!!\n", __FUNCTION__);
		attr_content = rtw_get_wfd_attr_content(wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, NULL, &attr_contentlen);
		if (attr_content && attr_contentlen) {
			ptdlsinfo->wfd_info->peer_rtsp_ctrlport = RTW_GET_BE16(attr_content + 2);
			RTW_INFO("[%s] Peer PORT NUM = %d\n", __FUNCTION__, ptdlsinfo->wfd_info->peer_rtsp_ctrlport);
		}

		attr_content = rtw_get_wfd_attr_content(wfd_ie, wfd_ielen, WFD_ATTR_LOCAL_IP_ADDR, NULL, &attr_contentlen);
		if (attr_content && attr_contentlen) {
			_rtw_memcpy(ptdlsinfo->wfd_info->peer_ip_address, (attr_content + 1), 4);
			RTW_INFO("[%s] Peer IP = %02u.%02u.%02u.%02u\n", __FUNCTION__,
				ptdlsinfo->wfd_info->peer_ip_address[0], ptdlsinfo->wfd_info->peer_ip_address[1],
				ptdlsinfo->wfd_info->peer_ip_address[2], ptdlsinfo->wfd_info->peer_ip_address[3]);
		}

		wfd_ie = rtw_get_wfd_ie(wfd_ie + wfd_ielen, (ptr + length) - (wfd_ie + wfd_ielen), NULL, &wfd_ielen);
	}
}

int issue_tunneled_probe_req(_adapter *padapter)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	u8 baddr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct tdls_txmgmt txmgmt;
	int ret = _FAIL;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);

	RTW_INFO("[%s]\n", __FUNCTION__);

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	txmgmt.action_code = TUNNELED_PROBE_REQ;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;

	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, baddr, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, &txmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}
	dump_mgntframe(padapter, pmgntframe);
	ret = _SUCCESS;
exit:

	return ret;
}

int issue_tunneled_probe_rsp(_adapter *padapter, union recv_frame *precv_frame)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct tdls_txmgmt txmgmt;
	int ret = _FAIL;
	struct _ADAPTER_LINK *padapter_link = precv_frame->u.hdr.adapter_link;

	RTW_INFO("[%s]\n", __FUNCTION__);

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	txmgmt.action_code = TUNNELED_PROBE_RSP;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;

	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, precv_frame->u.hdr.attrib.src, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, &txmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}
	dump_mgntframe(padapter, pmgntframe);
	ret = _SUCCESS;
exit:

	return ret;
}
#endif /* CONFIG_WFD */

int issue_tdls_setup_req(_adapter *padapter, struct tdls_txmgmt *ptxmgmt, int wait_ack)
{
	struct tdls_info	*ptdlsinfo = &padapter->tdlsinfo;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *ptdls_sta = NULL;
	int ret = _FAIL;
	void *phl = GET_PHL_INFO(adapter_to_dvobj(padapter));
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	enum role_type rtype = PHL_RTYPE_TDLS;
	enum rtw_phl_status status;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);
	u16 main_id = rtw_phl_get_macid_max_num(phl);
	struct rtw_phl_mld_t *pmld = NULL;

	/* Retry timer should be set at least 301 sec, using TPK_count counting 301 times. */
	u32 timeout_interval = TDLS_TPK_RESEND_COUNT;

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	if (rtw_tdls_is_setup_allowed(padapter) == _FALSE)
		goto exit;

	if (IS_MCAST(ptxmgmt->peer))
		goto exit;

	ptdls_sta = rtw_get_stainfo(pstapriv, ptxmgmt->peer);
	if (ptdlsinfo->sta_maximum == _TRUE) {
		if (ptdls_sta == NULL)
			goto exit;
		else if (!(ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE))
			goto exit;
	}

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	if (wrole->type != PHL_RTYPE_TDLS) {
		status = rtw_phl_cmd_wrole_change(phl, wrole, NULL,
			WR_CHG_TYPE, (u8*)&rtype, sizeof(enum role_type), PHL_CMD_DIRECTLY, 0);
		if (status != RTW_PHL_STATUS_SUCCESS) {
			RTW_ERR("%s - change to phl role type = %d fail with error = %d\n", __func__, rtype, status);
			rtw_warn_on(1);
		}
	}

	if (ptdls_sta == NULL) {
		pmld = rtw_phl_alloc_mld(phl, padapter->phl_role, ptxmgmt->peer, DTYPE);
		if (pmld == NULL) {
				RTW_INFO("[%s] rtw_phl_alloc_mld fail\n", __FUNCTION__);
				goto exit;
		}
		ptdls_sta = rtw_alloc_stainfo_sw(pstapriv, DTYPE, main_id, padapter_link->wrlink->id, ptxmgmt->peer);
		if (ptdls_sta == NULL) {
			RTW_INFO("[%s] rtw_alloc_stainfo fail\n", __FUNCTION__);
			#if 0 /*CONFIG_CORE_XMITBUF*/
			rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
			#endif
			rtw_free_xmitframe(pxmitpriv, pmgntframe);
			goto exit;
		}
		rtw_phl_link_mld_stainfo(pmld, ptdls_sta->phl_sta);
		ptdlsinfo->sta_cnt++;
	}

	ptxmgmt->action_code = TDLS_SETUP_REQUEST;

	pattrib = &pmgntframe->attrib;
	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, padapter_link, pattrib);

	if (ptdlsinfo->sta_cnt == MAX_ALLOWED_TDLS_STA_NUM)
		ptdlsinfo->sta_maximum  = _TRUE;

	ptdls_sta->tdls_sta_state |= TDLS_RESPONDER_STATE;

	if (rtw_tdls_is_driver_setup(padapter) == _TRUE) {
		ptdls_sta->TDLS_PeerKey_Lifetime = timeout_interval;
		_set_timer(&ptdls_sta->handshake_timer, TDLS_HANDSHAKE_TIME);
	}

	/*pattrib->qsel = pattrib->priority;*/

	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, ptxmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:

	return ret;
}

int _issue_tdls_teardown(_adapter *padapter, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta, u8 wait_ack)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	int ret = _FAIL;

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	ptxmgmt->action_code = TDLS_TEARDOWN;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	rtw_mi_set_scan_deny(padapter, 550);
	rtw_mi_scan_abort(padapter, _TRUE);

	pattrib = &pmgntframe->attrib;

	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);

	if (ptxmgmt->status_code == _RSON_TDLS_TEAR_UN_RSN_)
		_rtw_memcpy(pattrib->ra, ptxmgmt->peer, ETH_ALEN);
	else
		_rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);

	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, ptdls_sta->padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, ptxmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	if (rtw_tdls_is_driver_setup(padapter) == _TRUE)
		if (ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE)
			if (pattrib->encrypt)
				_cancel_timer_ex(&ptdls_sta->TPK_timer);

	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:

	return ret;
}

int issue_tdls_teardown(_adapter *padapter, struct tdls_txmgmt *ptxmgmt, u8 wait_ack)
{
	struct sta_info *ptdls_sta = NULL;
	int ret = _FAIL;

	ptdls_sta = rtw_get_stainfo(&(padapter->stapriv), ptxmgmt->peer);
	if (ptdls_sta == NULL) {
		RTW_INFO("No tdls_sta for tearing down\n");
		goto exit;
	}

	ret = _issue_tdls_teardown(padapter, ptxmgmt, ptdls_sta, wait_ack);
	if ((ptxmgmt->status_code == _RSON_TDLS_TEAR_UN_RSN_) && (ret == _FAIL)) {
		/* Change status code and send teardown again via AP */
		ptxmgmt->status_code = _RSON_TDLS_TEAR_TOOFAR_;
		ret = _issue_tdls_teardown(padapter, ptxmgmt, ptdls_sta, wait_ack);
	}

	if (rtw_tdls_is_driver_setup(padapter)) {
		rtw_tdls_teardown_pre_hdl(padapter, ptdls_sta);
		rtw_tdls_cmd(padapter, ptxmgmt->peer, TDLS_TEARDOWN_STA_LOCALLY_POST, 0);
	}

exit:
	return ret;
}

int issue_tdls_dis_req(_adapter *padapter, struct tdls_txmgmt *ptxmgmt)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	int ret = _FAIL;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	ptxmgmt->action_code = TDLS_DISCOVERY_REQUEST;
	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;
	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, ptxmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}
	dump_mgntframe(padapter, pmgntframe);
	RTW_INFO("issue tdls dis req\n");

	ret = _SUCCESS;
exit:

	return ret;
}

int issue_tdls_setup_rsp(_adapter *padapter, struct tdls_txmgmt *ptxmgmt)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	int ret = _FAIL;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	ptxmgmt->action_code = TDLS_SETUP_RESPONSE;
	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;
	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(&(padapter->mlmepriv)), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, ptxmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	dump_mgntframe(padapter, pmgntframe);

	ret = _SUCCESS;
exit:

	return ret;

}

int issue_tdls_setup_cfm(_adapter *padapter, struct tdls_txmgmt *ptxmgmt)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	int ret = _FAIL;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK *padapter_link = GET_PRIMARY_LINK(padapter);

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	ptxmgmt->action_code = TDLS_SETUP_CONFIRM;
	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;
	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(&padapter->mlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, ptxmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	dump_mgntframe(padapter, pmgntframe);

	ret = _SUCCESS;
exit:

	return ret;

}

/* TDLS Discovery Response frame is a management action frame */
int issue_tdls_dis_rsp(_adapter *padapter, struct tdls_txmgmt *ptxmgmt, u8 privacy)
{
	struct xmit_frame		*pmgntframe;
	struct pkt_attrib		*pattrib;
	unsigned char			*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	unsigned short		*fctrl;
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	int ret = _FAIL;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only] */
	struct _ADAPTER_LINK	*padapter_link = GET_PRIMARY_LINK(padapter);

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, padapter_link, pattrib);

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	/* unicast probe request frame */
	_rtw_memcpy(pwlanhdr->addr1, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->dst, pwlanhdr->addr1, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->src, pwlanhdr->addr2, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, get_bssid(&padapter->mlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, pwlanhdr->addr3, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	rtw_build_tdls_dis_rsp_ies(padapter, pmgntframe, pframe, ptxmgmt, privacy);

	pattrib->nr_frags = 1;
	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);
	ret = _SUCCESS;

exit:
	return ret;
}

int issue_tdls_peer_traffic_rsp(_adapter *padapter, struct sta_info *ptdls_sta, struct tdls_txmgmt *ptxmgmt)
{
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	int ret = _FAIL;

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	ptxmgmt->action_code = TDLS_PEER_TRAFFIC_RESPONSE;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;

	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptdls_sta->phl_sta->mac_addr, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, ptdls_sta->phl_sta->mac_addr, ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, ptdls_sta->padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/

	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, ptxmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	dump_mgntframe(padapter, pmgntframe);
	ret = _SUCCESS;

exit:

	return ret;
}

int issue_tdls_peer_traffic_indication(_adapter *padapter, struct sta_info *ptdls_sta)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct tdls_txmgmt txmgmt;
	int ret = _FAIL;

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	txmgmt.action_code = TDLS_PEER_TRAFFIC_INDICATION;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;

	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptdls_sta->phl_sta->mac_addr, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	/* PTI frame's priority should be AC_VO */
	pattrib->priority = 7;

	update_tdls_attrib(padapter, ptdls_sta->padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, &txmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	dump_mgntframe(padapter, pmgntframe);
	ret = _SUCCESS;

exit:

	return ret;
}

#ifdef CONFIG_TDLS_CH_SW
int issue_tdls_ch_switch_req(_adapter *padapter, struct sta_info *ptdls_sta)
{
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	struct tdls_txmgmt txmgmt;
	int ret = _FAIL;

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	if (rtw_tdls_is_chsw_allowed(padapter) == _FALSE) {
		RTW_INFO("[TDLS] Ignore %s since channel switch is not allowed\n", __func__);
		goto exit;
	}

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	txmgmt.action_code = TDLS_CHANNEL_SWITCH_REQUEST;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;

	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptdls_sta->phl_sta->mac_addr, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, ptdls_sta->phl_sta->mac_addr, ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, ptdls_sta->padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, &txmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	dump_mgntframe(padapter, pmgntframe);
	ret = _SUCCESS;
exit:

	return ret;
}

int issue_tdls_ch_switch_rsp(_adapter *padapter, struct tdls_txmgmt *ptxmgmt, int wait_ack)
{
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	int ret = _FAIL;
	/* ToDo CONFIG_RTW_MLD: [currently primary link only]*/
	struct _ADAPTER_LINK	*padapter_link = GET_PRIMARY_LINK(padapter);

	RTW_INFO("[TDLS] %s\n", __FUNCTION__);

	if (rtw_tdls_is_chsw_allowed(padapter) == _FALSE) {
		RTW_INFO("[TDLS] Ignore %s since channel switch is not allowed\n", __func__);
		goto exit;
	}

	ptxmgmt->action_code = TDLS_CHANNEL_SWITCH_RESPONSE;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	pattrib = &pmgntframe->attrib;

	pmgntframe->frame_tag = DATA_FRAMETAG;
	pattrib->ether_type = 0x890d;

	_rtw_memcpy(pattrib->dst, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, ptxmgmt->peer, ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);

	update_tdls_attrib(padapter, padapter_link, pattrib);
	/*pattrib->qsel = pattrib->priority;*/
	/*
		_rtw_spinlock_bh(&pxmitpriv->lock);
		if(xmitframe_enqueue_for_tdls_sleeping_sta(padapter, pmgntframe)==_TRUE){
			_rtw_spinunlock_bh(&pxmitpriv->lock);
			return _FALSE;
		}
	*/
	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, ptxmgmt) != _SUCCESS) {
		#if 0 /*CONFIG_CORE_XMITBUF*/
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		#endif
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}

	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack_timeout(padapter, pmgntframe, 10);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}
exit:

	return ret;
}
#endif

int On_TDLS_Dis_Rsp(_adapter *padapter, union recv_frame *precv_frame)
{
	struct sta_info *ptdls_sta = NULL, *psta = rtw_get_stainfo(&(padapter->stapriv), get_bssid(&(padapter->mlmepriv)));
	u8 *ptr = precv_frame->u.hdr.rx_data, *psa;
	struct rx_pkt_attrib *pattrib = &(precv_frame->u.hdr.attrib);
	struct tdls_info *ptdlsinfo = &(padapter->tdlsinfo);
	u8 empty_addr[ETH_ALEN] = { 0x00 };
	int rssi = 0;
	struct tdls_txmgmt txmgmt;
	int ret = _SUCCESS;

	/* ToDo: need API to query hal_sta->rssi_stat.rssi */
	/*
	if (psta)
		rssi = psta->phl_sta->hal_sta->rssi_stat.rssi;
	*/

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	/* WFDTDLS: for sigma test, not to setup direct link automatically */
	ptdlsinfo->dev_discovered = _TRUE;

	psa = get_sa(ptr);
	ptdls_sta = rtw_get_stainfo(&(padapter->stapriv), psa);
	if (ptdls_sta != NULL)
		ptdls_sta->sta_stats.rx_tdls_disc_rsp_pkts++;

#ifdef CONFIG_TDLS_AUTOSETUP
	if (ptdls_sta != NULL) {
		/* Record the tdls sta with lowest signal strength */
		if (ptdlsinfo->sta_maximum == _TRUE && ptdls_sta->alive_count >= 1) {
			if (_rtw_memcmp(ptdlsinfo->ss_record.macaddr, empty_addr, ETH_ALEN)) {
				_rtw_memcpy(ptdlsinfo->ss_record.macaddr, psa, ETH_ALEN);
				ptdlsinfo->ss_record.RxPWDBAll = pattrib->phy_info.rx_pwdb_all;
			} else {
				if (ptdlsinfo->ss_record.RxPWDBAll < pattrib->phy_info.rx_pwdb_all) {
					_rtw_memcpy(ptdlsinfo->ss_record.macaddr, psa, ETH_ALEN);
					ptdlsinfo->ss_record.RxPWDBAll = pattrib->phy_info.rx_pwdb_all;
				}
			}
		}
	} else {
		if (ptdlsinfo->sta_maximum == _TRUE) {
			if (_rtw_memcmp(ptdlsinfo->ss_record.macaddr, empty_addr, ETH_ALEN)) {
				/* All traffics are busy, do not set up another direct link. */
				ret = _FAIL;
				goto exit;
			} else {
				if (pattrib->phy_info.rx_pwdb_all > ptdlsinfo->ss_record.RxPWDBAll) {
					_rtw_memcpy(txmgmt.peer, ptdlsinfo->ss_record.macaddr, ETH_ALEN);
					/* issue_tdls_teardown(padapter, ptdlsinfo->ss_record.macaddr, _FALSE); */
				} else {
					ret = _FAIL;
					goto exit;
				}
			}
		}


		if (pattrib->phy_info.rx_pwdb_all + TDLS_SIGNAL_THRESH >= rssi) {
			RTW_INFO("pattrib->RxPWDBAll=%d, pdmpriv->undecorated_smoothed_pwdb=%d\n", pattrib->phy_info.rx_pwdb_all, rssi);
			_rtw_memcpy(txmgmt.peer, psa, ETH_ALEN);
			issue_tdls_setup_req(padapter, &txmgmt, _FALSE);
		}
	}
exit:
#endif /* CONFIG_TDLS_AUTOSETUP */

	return ret;

}

sint On_TDLS_Setup_Req(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	u8 *psa, *pmyid;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct rx_pkt_attrib	*prx_pkt_attrib = &precv_frame->u.hdr.attrib;
	u8 *prsnie, *ppairwise_cipher;
	u8 i, k;
	u8 ccmp_included = 0, rsnie_included = 0;
	u16 j, pairwise_count;
	u8 SNonce[32];
	u32 timeout_interval = TDLS_TPK_RESEND_COUNT;
	sint parsing_length;	/* Frame body length, without icv_len */
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 FIXED_IE = 5;
	unsigned char		supportRate[16];
	int				supportRateNum = 0;
	struct tdls_txmgmt txmgmt;
	void *phl = GET_PHL_INFO(adapter_to_dvobj(padapter));
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	enum role_type rtype = PHL_RTYPE_TDLS;
	enum rtw_phl_status status;
	struct _ADAPTER_LINK *padapter_link = precv_frame->u.hdr.adapter_link;
	u16 main_id = rtw_phl_get_macid_max_num(phl);
	struct rtw_phl_mld_t *pmld = NULL;

	if (rtw_tdls_is_setup_allowed(padapter) == _FALSE)
		goto exit;

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	psa = get_sa(ptr);

	if (ptdlsinfo->sta_maximum == _TRUE) {
		if (ptdls_sta == NULL)
			goto exit;
		else if (!(ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE))
			goto exit;
	}

	pmyid = adapter_mac_addr(padapter);
	ptr += prx_pkt_attrib->hdrlen + prx_pkt_attrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	parsing_length = ((union recv_frame *)precv_frame)->u.hdr.len
			 - prx_pkt_attrib->hdrlen
			 - prx_pkt_attrib->iv_len
			 - prx_pkt_attrib->icv_len
			 - LLC_HEADER_SIZE
			 - ETH_TYPE_LEN
			 - PAYLOAD_TYPE_LEN;

	if (wrole->type != PHL_RTYPE_TDLS) {
		status = rtw_phl_cmd_wrole_change(phl, wrole, NULL,
			WR_CHG_TYPE, (u8*)&rtype, sizeof(enum role_type), PHL_CMD_DIRECTLY, 0);
		if (status != RTW_PHL_STATUS_SUCCESS) {
			RTW_ERR("%s - change to phl role type = %d fail with error = %d\n", __func__, rtype, status);
			rtw_warn_on(1);
		}
        }

	if (ptdls_sta == NULL) {
		pmld = rtw_phl_alloc_mld(phl, padapter->phl_role, psa, DTYPE);
		if (pmld == NULL) {
				RTW_INFO("[%s] rtw_phl_alloc_mld fail\n", __FUNCTION__);
				goto exit;
		}
		ptdls_sta = rtw_alloc_stainfo_sw(pstapriv, DTYPE, main_id, padapter_link->wrlink->id, psa);
		if (ptdls_sta == NULL) {
			RTW_INFO("[%s] rtw_alloc_stainfo fail\n", __FUNCTION__);
			goto exit;
		}
		rtw_phl_link_mld_stainfo(pmld, ptdls_sta->phl_sta);
		ptdlsinfo->sta_cnt++;
	}
	else {
		if (ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE) {
			/* If the direct link is already set up */
			/* Process as re-setup after tear down */
			RTW_INFO("re-setup a direct link\n");
			if (rtw_tdls_is_driver_setup(padapter) == _FALSE)
				ptdls_sta->tdls_sta_state |= TDLS_RESETUP_STATE;
		}
		/* Already receiving TDLS setup request */
		else if (ptdls_sta->tdls_sta_state & TDLS_INITIATOR_STATE) {
			RTW_INFO("receive duplicated TDLS setup request frame in handshaking\n");
			goto exit;
		}
		/* When receiving and sending setup_req to the same link at the same time */
		/* STA with higher MAC_addr would be initiator */
		else if (ptdls_sta->tdls_sta_state & TDLS_RESPONDER_STATE) {
			RTW_INFO("receive setup_req after sending setup_req\n");
			for (i = 0; i < 6; i++) {
				if (*(pmyid + i) == *(psa + i)) {
				} else if (*(pmyid + i) > *(psa + i)) {
					ptdls_sta->tdls_sta_state = TDLS_INITIATOR_STATE;
					break;
				} else if (*(pmyid + i) < *(psa + i))
					goto exit;
			}
		}
	}

	if (ptdls_sta) {
		txmgmt.dialog_token = *(ptr + 2);	/* Copy dialog token */
		txmgmt.status_code = _STATS_SUCCESSFUL_;

		/* Parsing information element */
		for (j = FIXED_IE; j < parsing_length;) {

			pIE = (PNDIS_802_11_VARIABLE_IEs)(ptr + j);

			switch (pIE->ElementID) {
			case WLAN_EID_SUPP_RATES:
				if (pIE->Length <= sizeof(supportRate)) {
					_rtw_memcpy(supportRate, pIE->data, pIE->Length);
					supportRateNum = pIE->Length;
				}
				break;
			case WLAN_EID_COUNTRY:
				break;
			case WLAN_EID_EXT_SUPP_RATES:
				if ((supportRateNum + pIE->Length) <= sizeof(supportRate)) {
					_rtw_memcpy(supportRate + supportRateNum, pIE->data, pIE->Length);
					supportRateNum += pIE->Length;
				}
				break;
			case WLAN_EID_SUPPORTED_CHANNELS:
				break;
			case WLAN_EID_RSN:
				rsnie_included = 1;
				if (prx_pkt_attrib->encrypt) {
					prsnie = (u8 *)pIE;
					/* Check CCMP pairwise_cipher presence. */
					ppairwise_cipher = prsnie + 10;
					_rtw_memcpy(ptdls_sta->TDLS_RSNIE, pIE->data,
						(pIE->Length <= sizeof(ptdls_sta->TDLS_RSNIE) ? pIE->Length : sizeof(ptdls_sta->TDLS_RSNIE)));
					pairwise_count = *(u16 *)(ppairwise_cipher - 2);
					for (k = 0; k < pairwise_count; k++) {
						if (_rtw_memcmp(ppairwise_cipher + 4 * k, RSN_CIPHER_SUITE_CCMP, 4) == _TRUE)
							ccmp_included = 1;
					}

					if (ccmp_included == 0)
						txmgmt.status_code = _STATS_INVALID_RSNIE_;
				}
				break;
			case WLAN_EID_EXT_CAP:
				break;
			case WLAN_EID_VENDOR_SPECIFIC:
				break;
			case WLAN_EID_FAST_BSS_TRANSITION:
				if (prx_pkt_attrib->encrypt)
					_rtw_memcpy(SNonce, (ptr + j + 52), 32);
				break;
			case WLAN_EID_TIMEOUT_INTERVAL:
				if (prx_pkt_attrib->encrypt)
					timeout_interval = cpu_to_le32(*(u32 *)(ptr + j + 3));
				break;
			case _RIC_Descriptor_IE_:
				break;
#ifdef CONFIG_80211N_HT
			case WLAN_EID_HT_CAP:
				rtw_tdls_process_ht_cap(padapter, ptdls_sta, pIE);
				break;
#endif
#ifdef CONFIG_80211AC_VHT
			case EID_AID:
				break;
			case WLAN_EID_VHT_CAPABILITY:
				rtw_tdls_process_vht_cap(padapter, ptdls_sta, pIE);
				break;
#endif
#ifdef CONFIG_80211AX_HE
			case WLAN_EID_EXTENSION:
				if (pIE->data[0] == WLAN_EID_EXTENSION_HE_CAPABILITY)
					rtw_tdls_process_he_cap(padapter, ptdls_sta, pIE);
				break;
#endif
			case WLAN_EID_20_40_BSS_COEXISTENCE:
				break;
			case _LINK_ID_IE_:
				if (_rtw_memcmp(get_bssid(pmlmepriv), pIE->data, 6) == _FALSE)
					txmgmt.status_code = _STATS_NOT_IN_SAME_BSS_;
				break;
			default:
				break;
			}

			j += (pIE->Length + 2);

		}

		/* Check status code */
		/* If responder STA has/hasn't security on AP, but request hasn't/has RSNIE, it should reject */
		if (txmgmt.status_code == _STATS_SUCCESSFUL_) {
			if (rsnie_included && prx_pkt_attrib->encrypt == 0)
				txmgmt.status_code = _STATS_SEC_DISABLED_;
			else if (rsnie_included == 0 && prx_pkt_attrib->encrypt)
				txmgmt.status_code = _STATS_INVALID_PARAMETERS_;

#ifdef CONFIG_WFD
			/* WFD test plan version 0.18.2 test item 5.1.5 */
			/* SoUT does not use TDLS if AP uses weak security */
			if (padapter->wdinfo.wfd_tdls_enable && (rsnie_included && prx_pkt_attrib->encrypt != _AES_))
				txmgmt.status_code = _STATS_SEC_DISABLED_;
#endif /* CONFIG_WFD */
		}

		ptdls_sta->tdls_sta_state |= TDLS_INITIATOR_STATE;
		if (prx_pkt_attrib->encrypt) {
			_rtw_memcpy(ptdls_sta->SNonce, SNonce, 32);

			if (timeout_interval <= 300)
				ptdls_sta->TDLS_PeerKey_Lifetime = TDLS_TPK_RESEND_COUNT;
			else
				ptdls_sta->TDLS_PeerKey_Lifetime = timeout_interval;
		}

		/* Update station supportRate */
		ptdls_sta->bssratelen = supportRateNum;
		_rtw_memcpy(ptdls_sta->bssrateset, supportRate, supportRateNum);

		/* -2: AP + BC/MC sta, -4: default key */
		if (ptdlsinfo->sta_cnt == MAX_ALLOWED_TDLS_STA_NUM)
			ptdlsinfo->sta_maximum = _TRUE;

#ifdef CONFIG_WFD
		rtw_tdls_process_wfd_ie(ptdlsinfo, ptr + FIXED_IE, parsing_length);
#endif

	} else
		goto exit;

	_rtw_memcpy(txmgmt.peer, prx_pkt_attrib->src, ETH_ALEN);

	if (rtw_tdls_is_driver_setup(padapter)) {
		issue_tdls_setup_rsp(padapter, &txmgmt);

		if (txmgmt.status_code == _STATS_SUCCESSFUL_)
			_set_timer(&ptdls_sta->handshake_timer, TDLS_HANDSHAKE_TIME);
		else {
			rtw_tdls_teardown_pre_hdl(padapter, ptdls_sta);
			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_LOCALLY_POST, 0);
		}
	}

exit:

	return _SUCCESS;
}

int On_TDLS_Setup_Rsp(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib	*prx_pkt_attrib = &precv_frame->u.hdr.attrib;
	u8 *psa;
	u16 status_code = 0;
	sint parsing_length;	/* Frame body length, without icv_len */
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 FIXED_IE = 7;
	u8 ANonce[32];
	u8  *pftie = NULL, *ptimeout_ie = NULL, *plinkid_ie = NULL, *prsnie = NULL, *pftie_mic = NULL, *ppairwise_cipher = NULL;
	u16 pairwise_count, j, k;
	u8 verify_ccmp = 0;
	unsigned char		supportRate[16];
	int				supportRateNum = 0;
	struct tdls_txmgmt txmgmt;
	int ret = _SUCCESS;
	u32 timeout_interval = TDLS_TPK_RESEND_COUNT;
	struct _ADAPTER_LINK *padapter_link = ptdls_sta->padapter_link;

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	psa = get_sa(ptr);

	ptr += prx_pkt_attrib->hdrlen + prx_pkt_attrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	parsing_length = ((union recv_frame *)precv_frame)->u.hdr.len
			 - prx_pkt_attrib->hdrlen
			 - prx_pkt_attrib->iv_len
			 - prx_pkt_attrib->icv_len
			 - LLC_HEADER_SIZE
			 - ETH_TYPE_LEN
			 - PAYLOAD_TYPE_LEN;

	_rtw_memcpy(&status_code, ptr + 2, 2);

	if (status_code != 0) {
		RTW_INFO("[TDLS] %s status_code = %d, free_tdls_sta\n", __FUNCTION__, status_code);
		rtw_tdls_teardown_pre_hdl(padapter, ptdls_sta);
		rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_LOCALLY_POST, 0);
		ret = _FAIL;
		goto exit;
	}

	status_code = 0;

	/* parsing information element */
	for (j = FIXED_IE; j < parsing_length;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(ptr + j);

		switch (pIE->ElementID) {
		case WLAN_EID_SUPP_RATES:
			if (pIE->Length <= sizeof(supportRate)) {
				_rtw_memcpy(supportRate, pIE->data, pIE->Length);
				supportRateNum = pIE->Length;
			}
			break;
		case WLAN_EID_COUNTRY:
			break;
		case WLAN_EID_EXT_SUPP_RATES:
			if ((supportRateNum + pIE->Length) <= sizeof(supportRate)) {
				_rtw_memcpy(supportRate + supportRateNum, pIE->data, pIE->Length);
				supportRateNum += pIE->Length;
			}
			break;
		case WLAN_EID_SUPPORTED_CHANNELS:
			break;
		case WLAN_EID_RSN:
			prsnie = (u8 *)pIE;
			/* Check CCMP pairwise_cipher presence. */
			ppairwise_cipher = prsnie + 10;
			_rtw_memcpy(&pairwise_count, (u16 *)(ppairwise_cipher - 2), 2);
			for (k = 0; k < pairwise_count; k++) {
				if (_rtw_memcmp(ppairwise_cipher + 4 * k, RSN_CIPHER_SUITE_CCMP, 4) == _TRUE)
					verify_ccmp = 1;
			}
		case WLAN_EID_EXT_CAP:
			break;
		case WLAN_EID_VENDOR_SPECIFIC:
			if (_rtw_memcmp((u8 *)pIE + 2, WMM_INFO_OUI, 6) == _TRUE) {
				/* WMM Info ID and OUI */
				if ((pregistrypriv->wmm_enable == _TRUE) || (padapter_link->mlmepriv.htpriv.ht_option == _TRUE))
					ptdls_sta->qos_option = _TRUE;
			}
			break;
		case WLAN_EID_FAST_BSS_TRANSITION:
			pftie = (u8 *)pIE;
			_rtw_memcpy(ANonce, (ptr + j + 20), 32);
			break;
		case WLAN_EID_TIMEOUT_INTERVAL:
			ptimeout_ie = (u8 *)pIE;
			timeout_interval = cpu_to_le32(*(u32 *)(ptimeout_ie + 3));
			break;
		case _RIC_Descriptor_IE_:
			break;
#ifdef CONFIG_80211N_HT
		case WLAN_EID_HT_CAP:
			rtw_tdls_process_ht_cap(padapter, ptdls_sta, pIE);
			break;
#endif
#ifdef CONFIG_80211AC_VHT
		case EID_AID:
			/* todo in the future if necessary */
			break;
		case WLAN_EID_VHT_CAPABILITY:
			rtw_tdls_process_vht_cap(padapter, ptdls_sta, pIE);
			break;
		case WLAN_EID_VHT_OP_MODE_NOTIFY:
			rtw_tdls_process_vht_op_mode_notify(padapter, ptdls_sta, pIE);
			break;
#endif
#ifdef CONFIG_80211AX_HE
		case WLAN_EID_EXTENSION:
			if (pIE->data[0] == WLAN_EID_EXTENSION_HE_CAPABILITY)
				rtw_tdls_process_he_cap(padapter, ptdls_sta, pIE);
			break;
#endif
		case WLAN_EID_20_40_BSS_COEXISTENCE:
			break;
		case _LINK_ID_IE_:
			plinkid_ie = (u8 *)pIE;
			break;
		default:
			break;
		}

		j += (pIE->Length + 2);

	}

	ptdls_sta->bssratelen = supportRateNum;
	_rtw_memcpy(ptdls_sta->bssrateset, supportRate, supportRateNum);
	_rtw_memcpy(ptdls_sta->ANonce, ANonce, 32);

#ifdef CONFIG_WFD
	rtw_tdls_process_wfd_ie(ptdlsinfo, ptr + FIXED_IE, parsing_length);
#endif

	if (prx_pkt_attrib->encrypt) {
		if (verify_ccmp == 1) {
			txmgmt.status_code = _STATS_SUCCESSFUL_;
			if (rtw_tdls_is_driver_setup(padapter) == _TRUE) {
				wpa_tdls_generate_tpk(padapter, ptdls_sta);
				if (tdls_verify_mic(ptdls_sta->tpk.kck, 2, plinkid_ie, prsnie, ptimeout_ie, pftie) == _FAIL) {
					RTW_INFO("[TDLS] %s tdls_verify_mic fail, free_tdls_sta\n", __FUNCTION__);
					rtw_tdls_teardown_pre_hdl(padapter, ptdls_sta);
					rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_LOCALLY_POST, 0);
					ret = _FAIL;
					goto exit;
				}
				ptdls_sta->TDLS_PeerKey_Lifetime = timeout_interval;
			}
		} else
			txmgmt.status_code = _STATS_INVALID_RSNIE_;
	} else
		txmgmt.status_code = _STATS_SUCCESSFUL_;

	if (rtw_tdls_is_driver_setup(padapter) == _TRUE) {
		_rtw_memcpy(txmgmt.peer, prx_pkt_attrib->src, ETH_ALEN);
		issue_tdls_setup_cfm(padapter, &txmgmt);

		if (txmgmt.status_code == _STATS_SUCCESSFUL_) {
			rtw_tdls_set_link_established(padapter, _TRUE);

			if (ptdls_sta->tdls_sta_state & TDLS_RESPONDER_STATE) {
				ptdls_sta->tdls_sta_state |= TDLS_LINKED_STATE;
				ptdls_sta->state |= WIFI_ASOC_STATE;
				_cancel_timer_ex(&ptdls_sta->handshake_timer);
			}

			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_ESTABLISHED, 0);

		}
	}

exit:
	if (rtw_tdls_is_driver_setup(padapter) == _TRUE)
		return ret;
	else
		return _SUCCESS;

}

int On_TDLS_Setup_Cfm(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib	*prx_pkt_attrib = &precv_frame->u.hdr.attrib;
	u8 *psa;
	u16 status_code = 0;
	sint parsing_length;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 FIXED_IE = 5;
	u8  *pftie = NULL, *ptimeout_ie = NULL, *plinkid_ie = NULL, *prsnie = NULL, *pftie_mic = NULL, *ppairwise_cipher = NULL;
	u16 j, pairwise_count;
	int ret = _SUCCESS;

	psa = get_sa(ptr);

	ptr += prx_pkt_attrib->hdrlen + prx_pkt_attrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	parsing_length = ((union recv_frame *)precv_frame)->u.hdr.len
			 - prx_pkt_attrib->hdrlen
			 - prx_pkt_attrib->iv_len
			 - prx_pkt_attrib->icv_len
			 - LLC_HEADER_SIZE
			 - ETH_TYPE_LEN
			 - PAYLOAD_TYPE_LEN;

	_rtw_memcpy(&status_code, ptr + 2, 2);

	if (status_code != 0) {
		RTW_INFO("[%s] status_code = %d\n, free_tdls_sta", __FUNCTION__, status_code);
		rtw_tdls_teardown_pre_hdl(padapter, ptdls_sta);
		rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_LOCALLY_POST, 0);
		ret = _FAIL;
		goto exit;
	}

	/* Parsing information element */
	for (j = FIXED_IE; j < parsing_length;) {

		pIE = (PNDIS_802_11_VARIABLE_IEs)(ptr + j);

		switch (pIE->ElementID) {
		case WLAN_EID_RSN:
			prsnie = (u8 *)pIE;
			break;
		case WLAN_EID_VENDOR_SPECIFIC:
			if (_rtw_memcmp((u8 *)pIE + 2, WMM_PARA_OUI, 6) == _TRUE) {
				/* WMM Parameter ID and OUI */
				ptdls_sta->qos_option = _TRUE;
			}
			break;
		case WLAN_EID_FAST_BSS_TRANSITION:
			pftie = (u8 *)pIE;
			break;
		case WLAN_EID_TIMEOUT_INTERVAL:
			ptimeout_ie = (u8 *)pIE;
			break;
#ifdef CONFIG_80211N_HT
		case WLAN_EID_HT_OPERATION:
			break;
#endif
#ifdef CONFIG_80211AC_VHT
		case WLAN_EID_VHT_OPERATION:
			rtw_tdls_process_vht_operation(padapter, ptdls_sta, pIE);
			break;
		case WLAN_EID_VHT_OP_MODE_NOTIFY:
			rtw_tdls_process_vht_op_mode_notify(padapter, ptdls_sta, pIE);
			break;
#endif
#ifdef CONFIG_80211AX_HE
		case WLAN_EID_EXTENSION:
			if (pIE->data[0] == WLAN_EID_EXTENSION_HE_OPERATION)
				rtw_tdls_process_he_operation(padapter, ptdls_sta, pIE);
			break;
#endif
		case _LINK_ID_IE_:
			plinkid_ie = (u8 *)pIE;
			break;
		default:
			break;
		}

		j += (pIE->Length + 2);

	}

	if (prx_pkt_attrib->encrypt) {
		/* Verify mic in FTIE MIC field */
		if (rtw_tdls_is_driver_setup(padapter) &&
		    (tdls_verify_mic(ptdls_sta->tpk.kck, 3, plinkid_ie, prsnie, ptimeout_ie, pftie) == _FAIL)) {
			rtw_tdls_teardown_pre_hdl(padapter, ptdls_sta);
			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_LOCALLY_POST, 0);
			ret = _FAIL;
			goto exit;
		}
	}

	if (rtw_tdls_is_driver_setup(padapter)) {
		rtw_tdls_set_link_established(padapter, _TRUE);

		if (ptdls_sta->tdls_sta_state & TDLS_INITIATOR_STATE) {
			ptdls_sta->tdls_sta_state |= TDLS_LINKED_STATE;
			ptdls_sta->state |= WIFI_ASOC_STATE;
			_cancel_timer_ex(&ptdls_sta->handshake_timer);
		}

		if (prx_pkt_attrib->encrypt) {
			/* Start  TPK timer */
			ptdls_sta->TPK_count = 0;
			_set_timer(&ptdls_sta->TPK_timer, ONE_SEC);
		}

		rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_ESTABLISHED, 0);
	}

exit:
	return ret;

}

int On_TDLS_Dis_Req(_adapter *padapter, union recv_frame *precv_frame)
{
	struct rx_pkt_attrib	*prx_pkt_attrib = &precv_frame->u.hdr.attrib;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta_ap;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	sint parsing_length;	/* Frame body length, without icv_len */
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 FIXED_IE = 3, *dst;
	u16 j;
	struct tdls_txmgmt txmgmt;
	int ret = _SUCCESS;

	if (rtw_tdls_is_driver_setup(padapter) == _FALSE)
		goto exit;

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	ptr += prx_pkt_attrib->hdrlen + prx_pkt_attrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	txmgmt.dialog_token = *(ptr + 2);
	_rtw_memcpy(&txmgmt.peer, precv_frame->u.hdr.attrib.src, ETH_ALEN);
	txmgmt.action_code = TDLS_DISCOVERY_RESPONSE;
	parsing_length = ((union recv_frame *)precv_frame)->u.hdr.len
			 - prx_pkt_attrib->hdrlen
			 - prx_pkt_attrib->iv_len
			 - prx_pkt_attrib->icv_len
			 - LLC_HEADER_SIZE
			 - ETH_TYPE_LEN
			 - PAYLOAD_TYPE_LEN;

	/* Parsing information element */
	for (j = FIXED_IE; j < parsing_length;) {

		pIE = (PNDIS_802_11_VARIABLE_IEs)(ptr + j);

		switch (pIE->ElementID) {
		case _LINK_ID_IE_:
			psta_ap = rtw_get_stainfo(pstapriv, pIE->data);
			if (psta_ap == NULL)
				goto exit;
			dst = pIE->data + 12;
			if (MacAddr_isBcst(dst) == _FALSE && (_rtw_memcmp(adapter_mac_addr(padapter), dst, ETH_ALEN) == _FALSE))
				goto exit;
			break;
		default:
			break;
		}

		j += (pIE->Length + 2);

	}

	issue_tdls_dis_rsp(padapter, &txmgmt, prx_pkt_attrib->privacy);

exit:
	return ret;

}

int On_TDLS_Teardown(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib	*prx_pkt_attrib = &precv_frame->u.hdr.attrib;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv	*pstapriv = &padapter->stapriv;
	u8 reason;

	reason = *(ptr + prx_pkt_attrib->hdrlen + prx_pkt_attrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN + 2);
	RTW_INFO("[TDLS] %s Reason code(%d)\n", __FUNCTION__, reason);

	if (rtw_tdls_is_driver_setup(padapter)) {
		rtw_tdls_teardown_pre_hdl(padapter, ptdls_sta);
		rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_LOCALLY_POST, 0);
	}

	return _SUCCESS;

}

#if 0
u8 TDLS_check_ch_state(uint state)
{
	if (state & TDLS_CH_SWITCH_ON_STATE &&
	    state & TDLS_PEER_AT_OFF_STATE) {
		if (state & TDLS_PEER_SLEEP_STATE)
			return 2;	/* U-APSD + ch. switch */
		else
			return 1;	/* ch. switch */
	} else
		return 0;
}
#endif

int On_TDLS_Peer_Traffic_Indication(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	struct rx_pkt_attrib	*pattrib = &precv_frame->u.hdr.attrib;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct tdls_txmgmt txmgmt;

	ptr += pattrib->hdrlen + pattrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));

		txmgmt.dialog_token = *(ptr + 2);
		issue_tdls_peer_traffic_rsp(padapter, ptdls_sta, &txmgmt);
		/* issue_nulldata_to_TDLS_peer_STA(padapter, ptdls_sta->phl_sta->mac_addr, 0, 0, 0); */

	return _SUCCESS;
}

/* We process buffered data for 1. U-APSD, 2. ch. switch, 3. U-APSD + ch. switch here */
int On_TDLS_Peer_Traffic_Rsp(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct rx_pkt_attrib	*pattrib = &precv_frame->u.hdr.attrib;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 wmmps_ac = 0;
	/* u8 state=TDLS_check_ch_state(ptdls_sta->tdls_sta_state); */
	int i;

	ptdls_sta->sta_stats.rx_data_pkts++;

	ptdls_sta->tdls_sta_state &= ~(TDLS_WAIT_PTR_STATE);

	/* Check 4-AC queue bit */
	if (ptdls_sta->uapsd_vo || ptdls_sta->uapsd_vi || ptdls_sta->uapsd_be || ptdls_sta->uapsd_bk)
		wmmps_ac = 1;

	/* If it's a direct link and have buffered frame */
	if (ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE) {
		if (wmmps_ac) {
			_list	*xmitframe_plist, *xmitframe_phead;
			struct xmit_frame *pxmitframe = NULL;

			_rtw_spinlock_bh(&ptdls_sta->sleep_q.lock);

			xmitframe_phead = get_list_head(&ptdls_sta->sleep_q);
			xmitframe_plist = get_next(xmitframe_phead);

			/* transmit buffered frames */
			while (rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist) == _FALSE) {
				pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);
				xmitframe_plist = get_next(xmitframe_plist);
				rtw_list_delete(&pxmitframe->list);

				ptdls_sta->sleepq_len--;
				ptdls_sta->sleepq_ac_len--;
				if (ptdls_sta->sleepq_len > 0) {
					pxmitframe->attrib.mdata = 1;
					pxmitframe->attrib.eosp = 0;
				} else {
					pxmitframe->attrib.mdata = 0;
					pxmitframe->attrib.eosp = 1;
				}
				pxmitframe->attrib.triggered = 1;

				rtw_intf_xmitframe_enqueue(padapter, pxmitframe);
			}

			if (ptdls_sta->sleepq_len == 0)
				RTW_INFO("no buffered packets for tdls to xmit\n");
			else {
				RTW_INFO("error!psta->sleepq_len=%d\n", ptdls_sta->sleepq_len);
				ptdls_sta->sleepq_len = 0;
			}

			_rtw_spinunlock_bh(&ptdls_sta->sleep_q.lock);

		}

	}

	return _SUCCESS;
}

#ifdef CONFIG_TDLS_CH_SW
sint On_TDLS_Ch_Switch_Req(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib	*prx_pkt_attrib = &precv_frame->u.hdr.attrib;
	sint parsing_length;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 FIXED_IE = 4;
	u16 j;
	u8 zaddr[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u16 switch_time = TDLS_CH_SWITCH_TIME * 1000, switch_timeout = TDLS_CH_SWITCH_TIMEOUT * 1000;
	struct _ADAPTER_LINK *padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_ext_priv *pmlmeext = &padapter_link->mlmeextpriv;
	struct rtw_chan_def chdef = {0};

	if (rtw_tdls_is_chsw_allowed(padapter) == _FALSE) {
		RTW_INFO("[TDLS] Ignore %s since channel switch is not allowed\n", __func__);
		return _FAIL;
	}

	ptdls_sta->ch_switch_time = switch_time;
	ptdls_sta->ch_switch_timeout = switch_timeout;

	ptr += prx_pkt_attrib->hdrlen + prx_pkt_attrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	parsing_length = ((union recv_frame *)precv_frame)->u.hdr.len
			 - prx_pkt_attrib->hdrlen
			 - prx_pkt_attrib->iv_len
			 - prx_pkt_attrib->icv_len
			 - LLC_HEADER_SIZE
			 - ETH_TYPE_LEN
			 - PAYLOAD_TYPE_LEN;

	pchsw_info->off_ch_num = *(ptr + 2);

	if ((*(ptr + 2) == 2) && (rtw_hw_is_band_support(adapter_to_dvobj(padapter), BAND_ON_5G)))
		pchsw_info->off_ch_num = 44;

	if (pchsw_info->off_ch_num != pmlmeext->chandef.chan)
		pchsw_info->delay_switch_back = _FALSE;

	/* Parsing information element */
	for (j = FIXED_IE; j < parsing_length;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(ptr + j);

		switch (pIE->ElementID) {
		case WLAN_EID_SECONDARY_CHANNEL_OFFSET:
			switch (*(pIE->data)) {
			case IEEE80211_SCA:
				pchsw_info->ch_offset = CHAN_OFFSET_UPPER;
				break;

			case IEEE80211_SCB:
				pchsw_info->ch_offset = CHAN_OFFSET_LOWER;
				break;

			default:/*IEEE80211_SCN*/
				pchsw_info->ch_offset = CHAN_OFFSET_NO_EXT;
				break;
			}
			break;
		case _LINK_ID_IE_:
			break;
		case _CH_SWITCH_TIMING_:
			ptdls_sta->ch_switch_time = (RTW_GET_LE16(pIE->data) >= TDLS_CH_SWITCH_TIME * 1000) ?
				RTW_GET_LE16(pIE->data) : TDLS_CH_SWITCH_TIME * 1000;
			ptdls_sta->ch_switch_timeout = (RTW_GET_LE16(pIE->data + 2) >= TDLS_CH_SWITCH_TIMEOUT * 1000) ?
				RTW_GET_LE16(pIE->data + 2) : TDLS_CH_SWITCH_TIMEOUT * 1000;
			RTW_INFO("[TDLS] %s ch_switch_time:%d, ch_switch_timeout:%d\n"
				, __FUNCTION__, RTW_GET_LE16(pIE->data), RTW_GET_LE16(pIE->data + 2));
			break;
		default:
			break;
		}

		j += (pIE->Length + 2);
	}

	chdef.chan = pchsw_info->off_ch_num;
	chdef.bw = (pchsw_info->ch_offset) ? CHANNEL_WIDTH_40 : CHANNEL_WIDTH_20;
	chdef.offset = pchsw_info->ch_offset;
	chdef.band = rtw_get_band_type(chdef.chan);

	if (!(pchsw_info->ch_sw_state & TDLS_CH_SWITCH_PREPARE_STATE)) {
		rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_PREPARE, 0);
		return _FAIL;
	}

	/* cancel ch sw monitor timer for responder */
	if (!(pchsw_info->ch_sw_state & TDLS_CH_SW_INITIATOR_STATE))
		_cancel_timer_ex(&ptdls_sta->ch_sw_monitor_timer);

	if (_rtw_memcmp(pchsw_info->addr, zaddr, ETH_ALEN) == _TRUE)
		_rtw_memcpy(pchsw_info->addr, ptdls_sta->phl_sta->mac_addr, ETH_ALEN);

	if (ATOMIC_READ(&pchsw_info->chsw_on) == _FALSE)
		rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_START, 0);

	rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_RESP, 0);

	return _SUCCESS;
}

sint On_TDLS_Ch_Switch_Rsp(_adapter *padapter, union recv_frame *precv_frame, struct sta_info *ptdls_sta)
{
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib	*prx_pkt_attrib = &precv_frame->u.hdr.attrib;
	sint parsing_length;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 FIXED_IE = 4;
	u16 status_code, j, switch_time, switch_timeout;
	struct _ADAPTER_LINK *padapter_link = precv_frame->u.hdr.adapter_link;
	struct link_mlme_ext_priv *pmlmeext = &padapter_link->mlmeextpriv;
	int ret = _SUCCESS;

	if (rtw_tdls_is_chsw_allowed(padapter) == _FALSE) {
		RTW_INFO("[TDLS] Ignore %s since channel switch is not allowed\n", __func__);
		return _SUCCESS;
	}

	/* If we receive Unsolicited TDLS Channel Switch Response when channel switch is running, */
	/* we will go back to base channel and terminate this channel switch procedure */
	if (ATOMIC_READ(&pchsw_info->chsw_on) == _TRUE) {
		if (pmlmeext->chandef.chan != rtw_get_oper_ch(padapter, padapter_link)) {
			RTW_INFO("[TDLS] Rx unsolicited channel switch response\n");
			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_TO_BASE_CHNL, 0);
			goto exit;
		}
	}

	ptr += prx_pkt_attrib->hdrlen + prx_pkt_attrib->iv_len + LLC_HEADER_SIZE + ETH_TYPE_LEN + PAYLOAD_TYPE_LEN;
	parsing_length = ((union recv_frame *)precv_frame)->u.hdr.len
			 - prx_pkt_attrib->hdrlen
			 - prx_pkt_attrib->iv_len
			 - prx_pkt_attrib->icv_len
			 - LLC_HEADER_SIZE
			 - ETH_TYPE_LEN
			 - PAYLOAD_TYPE_LEN;

	_rtw_memcpy(&status_code, ptr + 2, 2);

	if (status_code != 0) {
		RTW_INFO("[TDLS] %s status_code:%d\n", __func__, status_code);
		pchsw_info->ch_sw_state &= ~(TDLS_CH_SW_INITIATOR_STATE);
		rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_END, 0);
		ret = _FAIL;
		goto exit;
	}

	/* Parsing information element */
	for (j = FIXED_IE; j < parsing_length;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(ptr + j);

		switch (pIE->ElementID) {
		case _LINK_ID_IE_:
			break;
		case _CH_SWITCH_TIMING_:
			_rtw_memcpy(&switch_time, pIE->data, 2);
			if (switch_time > ptdls_sta->ch_switch_time)
				_rtw_memcpy(&ptdls_sta->ch_switch_time, &switch_time, 2);

			_rtw_memcpy(&switch_timeout, pIE->data + 2, 2);
			if (switch_timeout > ptdls_sta->ch_switch_timeout)
				_rtw_memcpy(&ptdls_sta->ch_switch_timeout, &switch_timeout, 2);
			break;
		default:
			break;
		}

		j += (pIE->Length + 2);
	}

	if ((pmlmeext->chandef.chan == rtw_get_oper_ch(padapter, padapter_link)) &&
	    (pchsw_info->ch_sw_state & TDLS_WAIT_CH_RSP_STATE)) {
		if (ATOMIC_READ(&pchsw_info->chsw_on) == _TRUE)
			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_TO_OFF_CHNL, 0);
	}

exit:
	return ret;
}
#endif /* CONFIG_TDLS_CH_SW */

#ifdef CONFIG_WFD
void wfd_ie_tdls(_adapter *padapter, u8 *pframe, u32 *pktlen)
{
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct wifi_display_info	*pwfd_info = padapter->tdlsinfo.wfd_info;
	u8 wfdie[MAX_WFD_IE_LEN] = { 0x00 };
	u32 wfdielen = 0;
	u16 v16 = 0;

	if (!rtw_hw_chk_wl_func(adapter_to_dvobj(padapter), WL_FUNC_MIRACAST))
		return;

	/* WFD OUI */
	wfdielen = 0;
	wfdie[wfdielen++] = 0x50;
	wfdie[wfdielen++] = 0x6F;
	wfdie[wfdielen++] = 0x9A;
	wfdie[wfdielen++] = 0x0A;	/* WFA WFD v1.0 */

	/*
	 *	Commented by Albert 20110825
	 *	According to the WFD Specification, the negotiation request frame should contain 3 WFD attributes
	 *	1. WFD Device Information
	 *	2. Associated BSSID ( Optional )
	 *	3. Local IP Adress ( Optional )
	 */

	/* WFD Device Information ATTR */
	/* Type: */
	wfdie[wfdielen++] = WFD_ATTR_DEVICE_INFO;

	/* Length: */
	/* Note: In the WFD specification, the size of length field is 2. */
	RTW_PUT_BE16(wfdie + wfdielen, 0x0006);
	wfdielen += 2;

	/* Value1: */
	/* WFD device information */
	/* available for WFD session + Preferred TDLS + WSD ( WFD Service Discovery ) */
	v16 = pwfd_info->wfd_device_type | WFD_DEVINFO_SESSION_AVAIL
		| WFD_DEVINFO_PC_TDLS | WFD_DEVINFO_WSD;
	RTW_PUT_BE16(wfdie + wfdielen, v16);
	wfdielen += 2;

	/* Value2: */
	/* Session Management Control Port */
	/* Default TCP port for RTSP messages is 554 */
	RTW_PUT_BE16(wfdie + wfdielen, pwfd_info->tdls_rtsp_ctrlport);
	wfdielen += 2;

	/* Value3: */
	/* WFD Device Maximum Throughput */
	/* 300Mbps is the maximum throughput */
	RTW_PUT_BE16(wfdie + wfdielen, 300);
	wfdielen += 2;

	/* Associated BSSID ATTR */
	/* Type: */
	wfdie[wfdielen++] = WFD_ATTR_ASSOC_BSSID;

	/* Length: */
	/* Note: In the WFD specification, the size of length field is 2. */
	RTW_PUT_BE16(wfdie + wfdielen, 0x0006);
	wfdielen += 2;

	/* Value: */
	/* Associated BSSID */
	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)
		_rtw_memcpy(wfdie + wfdielen, &pmlmepriv->assoc_bssid[0], ETH_ALEN);
	else
		_rtw_memset(wfdie + wfdielen, 0x00, ETH_ALEN);

	/* Local IP Address ATTR */
	wfdie[wfdielen++] = WFD_ATTR_LOCAL_IP_ADDR;

	/* Length: */
	/* Note: In the WFD specification, the size of length field is 2. */
	RTW_PUT_BE16(wfdie + wfdielen, 0x0005);
	wfdielen += 2;

	/* Version: */
	/* 0x01: Version1;IPv4 */
	wfdie[wfdielen++] = 0x01;

	/* IPv4 Address */
	_rtw_memcpy(wfdie + wfdielen, pwfd_info->ip_address, 4);
	wfdielen += 4;

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, wfdielen, (unsigned char *) wfdie, pktlen);

}
#endif /* CONFIG_WFD */

void rtw_build_tdls_setup_req_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	struct _ADAPTER_LINK 	*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &(padapter_link->mlmeextpriv);
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	int i = 0 ;
	u32 time;
	u8 *pframe_head;

	/* SNonce */
	if (pattrib->encrypt) {
		for (i = 0; i < 8; i++) {
			time = rtw_get_current_time();
			_rtw_memcpy(&ptdls_sta->SNonce[4 * i], (u8 *)&time, 4);
		}
	}

	pframe_head = pframe;	/* For rtw_tdls_set_ht_cap() */

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_dialog(pframe, pattrib, ptxmgmt);

	pframe = rtw_tdls_set_capability(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_supported_rate(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_sup_ch(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_sup_reg_class(pframe, pattrib);

	if (pattrib->encrypt)
		pframe = rtw_tdls_set_rsnie(ptxmgmt, pframe, pattrib,  _TRUE, ptdls_sta);

	pframe = rtw_tdls_set_ext_cap(pframe, pattrib);

	if (pattrib->encrypt) {
		pframe = rtw_tdls_set_ftie(ptxmgmt
					   , pframe
					   , pattrib
					   , NULL
					   , ptdls_sta->SNonce);

		pframe = rtw_tdls_set_timeout_interval(ptxmgmt, pframe, pattrib, _TRUE, ptdls_sta);
	}

#ifdef CONFIG_80211N_HT
	/* Sup_reg_classes(optional) */
	if (pregistrypriv->ht_enable == _TRUE)
		pframe = rtw_tdls_set_ht_cap(padapter, pframe_head, pattrib);
#endif

	pframe = rtw_tdls_set_bss_coexist(padapter, pframe, pattrib);

	pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);

	if ((padapter->registrypriv.wmm_enable == _TRUE) || (padapter_link->mlmepriv.htpriv.ht_option == _TRUE))
		pframe = rtw_tdls_set_qos_cap(pframe, pattrib);

#ifdef CONFIG_80211AC_VHT
	if ((pregistrypriv->ht_enable == _TRUE) && (pmlmeext->chandef.chan > 14)
		&& REGSTY_IS_11AC_ENABLE(pregistrypriv)
		&& is_supported_vht(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AC(rfctl)
		) {
		pframe = rtw_tdls_set_aid(padapter, pframe, pattrib);
		pframe = rtw_tdls_set_vht_cap(padapter, pframe, pattrib);
	}
#endif

#ifdef CONFIG_80211AX_HE
	if ((((pregistrypriv->ht_enable == _TRUE) && (pmlmeext->chandef.band == BAND_ON_24G))
#ifdef CONFIG_80211AC_VHT
		|| (REGSTY_IS_11AC_ENABLE(pregistrypriv) && (pmlmeext->chandef.band == BAND_ON_5G))
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
		|| (pmlmeext->chandef.band == BAND_ON_6G)
#endif
		)
		&& REGSTY_IS_11AX_ENABLE(pregistrypriv)
		&& is_supported_he(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AX(rfctl)
	) {
		pframe = rtw_tdls_set_he_cap(padapter, pframe, pattrib);
	}
#endif

#ifdef CONFIG_WFD
	if (padapter->wdinfo.wfd_tdls_enable == 1)
		wfd_ie_tdls(padapter, pframe, &(pattrib->pktlen));
#endif

}

void rtw_build_tdls_setup_rsp_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	struct _ADAPTER_LINK		*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &(padapter_link->mlmeextpriv);
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	u8 k; /* for random ANonce */
	u8  *pftie = NULL, *ptimeout_ie = NULL, *plinkid_ie = NULL, *prsnie = NULL, *pftie_mic = NULL;
	u32 time;
	u8 *pframe_head;

	if (pattrib->encrypt) {
		for (k = 0; k < 8; k++) {
			time = rtw_get_current_time();
			_rtw_memcpy(&ptdls_sta->ANonce[4 * k], (u8 *)&time, 4);
		}
	}

	pframe_head = pframe;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_status_code(pframe, pattrib, ptxmgmt);

	if (ptxmgmt->status_code != 0) {
		RTW_INFO("[%s] status_code:%04x\n", __FUNCTION__, ptxmgmt->status_code);
		return;
	}

	pframe = rtw_tdls_set_dialog(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_capability(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_supported_rate(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_sup_ch(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_sup_reg_class(pframe, pattrib);

	if (pattrib->encrypt) {
		prsnie = pframe;
		pframe = rtw_tdls_set_rsnie(ptxmgmt, pframe, pattrib,  _FALSE, ptdls_sta);
	}

	pframe = rtw_tdls_set_ext_cap(pframe, pattrib);

	if (pattrib->encrypt) {
		if (rtw_tdls_is_driver_setup(padapter) == _TRUE)
			wpa_tdls_generate_tpk(padapter, ptdls_sta);

		pftie = pframe;
		pftie_mic = pframe + 4;
		pframe = rtw_tdls_set_ftie(ptxmgmt
					   , pframe
					   , pattrib
					   , ptdls_sta->ANonce
					   , ptdls_sta->SNonce);

		ptimeout_ie = pframe;
		pframe = rtw_tdls_set_timeout_interval(ptxmgmt, pframe, pattrib, _FALSE, ptdls_sta);
	}

#ifdef CONFIG_80211N_HT
	/* Sup_reg_classes(optional) */
	if (pregistrypriv->ht_enable == _TRUE)
		pframe = rtw_tdls_set_ht_cap(padapter, pframe_head, pattrib);
#endif

	pframe = rtw_tdls_set_bss_coexist(padapter, pframe, pattrib);

	plinkid_ie = pframe;
	pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _FALSE);

	/* Fill FTIE mic */
	if (pattrib->encrypt && rtw_tdls_is_driver_setup(padapter) == _TRUE)
		wpa_tdls_ftie_mic(ptdls_sta->tpk.kck, 2, plinkid_ie, prsnie, ptimeout_ie, pftie, pftie_mic);

	if ((padapter->registrypriv.wmm_enable == _TRUE) || (padapter_link->mlmepriv.htpriv.ht_option == _TRUE))
		pframe = rtw_tdls_set_qos_cap(pframe, pattrib);

#ifdef CONFIG_80211AC_VHT
	if ((pregistrypriv->ht_enable == _TRUE) && (pmlmeext->chandef.chan > 14)
		&& REGSTY_IS_11AC_ENABLE(pregistrypriv)
		&& is_supported_vht(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AC(rfctl)
		) {
		pframe = rtw_tdls_set_aid(padapter, pframe, pattrib);
		pframe = rtw_tdls_set_vht_cap(padapter, pframe, pattrib);
		pframe = rtw_tdls_set_vht_op_mode_notify(padapter, pframe, pattrib, pmlmeext->chandef.bw);
	}
#endif

#ifdef CONFIG_80211AX_HE
	if ((((pregistrypriv->ht_enable == _TRUE) && (pmlmeext->chandef.band == BAND_ON_24G))
#ifdef CONFIG_80211AC_VHT
		|| (REGSTY_IS_11AC_ENABLE(pregistrypriv) && (pmlmeext->chandef.band == BAND_ON_5G))
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
		|| (pmlmeext->chandef.band == BAND_ON_6G)
#endif
		)
		&& REGSTY_IS_11AX_ENABLE(pregistrypriv)
		&& is_supported_he(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AX(rfctl)
	) {
		pframe = rtw_tdls_set_he_cap(padapter, pframe, pattrib);
	}
#endif

#ifdef CONFIG_WFD
	if (padapter->wdinfo.wfd_tdls_enable)
		wfd_ie_tdls(padapter, pframe, &(pattrib->pktlen));
#endif

}

void rtw_build_tdls_setup_cfm_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	struct _ADAPTER_LINK	*padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &(padapter_link->mlmeextpriv);
	struct link_mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;

	unsigned int ie_len;
	unsigned char *p;
	u8 wmm_param_ele[24] = {0};
	u8  *pftie = NULL, *ptimeout_ie = NULL, *plinkid_ie = NULL, *prsnie = NULL, *pftie_mic = NULL;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_status_code(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_dialog(pframe, pattrib, ptxmgmt);

	if (ptxmgmt->status_code != 0)
		return;

	if (pattrib->encrypt) {
		prsnie = pframe;
		pframe = rtw_tdls_set_rsnie(ptxmgmt, pframe, pattrib, _TRUE, ptdls_sta);
	}

	if (pattrib->encrypt) {
		pftie = pframe;
		pftie_mic = pframe + 4;
		pframe = rtw_tdls_set_ftie(ptxmgmt
					   , pframe
					   , pattrib
					   , ptdls_sta->ANonce
					   , ptdls_sta->SNonce);

		ptimeout_ie = pframe;
		pframe = rtw_tdls_set_timeout_interval(ptxmgmt, pframe, pattrib, _TRUE, ptdls_sta);

		if (rtw_tdls_is_driver_setup(padapter) == _TRUE) {
			/* Start TPK timer */
			ptdls_sta->TPK_count = 0;
			_set_timer(&ptdls_sta->TPK_timer, ONE_SEC);
		}
	}

	/* HT operation; todo */

	plinkid_ie = pframe;
	pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);

	if (pattrib->encrypt && (rtw_tdls_is_driver_setup(padapter) == _TRUE))
		wpa_tdls_ftie_mic(ptdls_sta->tpk.kck, 3, plinkid_ie, prsnie, ptimeout_ie, pftie, pftie_mic);

	if (ptdls_sta->qos_option == _TRUE)
		pframe = rtw_tdls_set_wmm_params(padapter, pframe, pattrib);

#ifdef CONFIG_80211AC_VHT
	if ((pregistrypriv->ht_enable == _TRUE)
		&& (ptdls_sta->vhtpriv.vht_option == _TRUE) && (pmlmeext->chandef.chan > 14)
		&& REGSTY_IS_11AC_ENABLE(pregistrypriv)
		&& is_supported_vht(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AC(rfctl)
		) {
		pframe = rtw_tdls_set_vht_operation(padapter, pframe, pattrib, pmlmeext->chandef.band, pmlmeext->chandef.chan);
		pframe = rtw_tdls_set_vht_op_mode_notify(padapter, pframe, pattrib, pmlmeext->chandef.bw);
	}
#endif

#ifdef CONFIG_80211AX_HE
	if ((((pregistrypriv->ht_enable == _TRUE) && (pmlmeext->chandef.band == BAND_ON_24G))
#ifdef CONFIG_80211AC_VHT
		|| (REGSTY_IS_11AC_ENABLE(pregistrypriv) && (pmlmeext->chandef.band == BAND_ON_5G))
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
		|| (pmlmeext->chandef.band == BAND_ON_6G)
#endif
		)
		&& (ptdls_sta->hepriv.he_option == _TRUE)
		&& REGSTY_IS_11AX_ENABLE(pregistrypriv)
		&& is_supported_he(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AX(rfctl)
		) {
		pframe = rtw_tdls_set_he_operation(padapter, pframe, pattrib);
	}
#endif
}

void rtw_build_tdls_teardown_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	u8  *pftie = NULL, *pftie_mic = NULL, *plinkid_ie = NULL;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_status_code(pframe, pattrib, ptxmgmt);

	if (pattrib->encrypt) {
		pftie = pframe;
		pftie_mic = pframe + 4;
		pframe = rtw_tdls_set_ftie(ptxmgmt
					   , pframe
					   , pattrib
					   , ptdls_sta->ANonce
					   , ptdls_sta->SNonce);
	}

	plinkid_ie = pframe;
	if (ptdls_sta->tdls_sta_state & TDLS_INITIATOR_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _FALSE);
	else if (ptdls_sta->tdls_sta_state & TDLS_RESPONDER_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);

	if (pattrib->encrypt && (rtw_tdls_is_driver_setup(padapter) == _TRUE))
		wpa_tdls_teardown_ftie_mic(ptdls_sta->tpk.kck, plinkid_ie, ptxmgmt->status_code, 1, 4, pftie, pftie_mic);
}

void rtw_build_tdls_dis_req_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt)
{
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_dialog(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);

}

void rtw_build_tdls_dis_rsp_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, u8 privacy)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	u8 *pframe_head, pktlen_index;
	struct _ADAPTER_LINK	*padapter_link = pattrib->adapter_link;
	struct link_mlme_ext_priv	*pmlmeext = &padapter_link->mlmeextpriv;

	pktlen_index = pattrib->pktlen;
	pframe_head = pframe;

	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_PUBLIC);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_dialog(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_capability(padapter, pframe, pattrib);

	pframe = rtw_tdls_set_supported_rate(padapter, pframe, pattrib);

	pframe = rtw_tdls_set_sup_ch(padapter, pframe, pattrib);

	if (privacy)
		pframe = rtw_tdls_set_rsnie(ptxmgmt, pframe, pattrib, _TRUE, NULL);

	pframe = rtw_tdls_set_ext_cap(pframe, pattrib);

	if (privacy) {
		pframe = rtw_tdls_set_ftie(ptxmgmt, pframe, pattrib, NULL, NULL);
		pframe = rtw_tdls_set_timeout_interval(ptxmgmt, pframe, pattrib,  _TRUE, NULL);
	}

#ifdef CONFIG_80211N_HT
	if (pregistrypriv->ht_enable == _TRUE)
		pframe = rtw_tdls_set_ht_cap(padapter, pframe_head - pktlen_index, pattrib);
#endif

	pframe = rtw_tdls_set_bss_coexist(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _FALSE);

#ifdef CONFIG_80211AC_VHT
	if ((pregistrypriv->ht_enable == _TRUE) && (pmlmeext->chandef.chan > 14)
		&& REGSTY_IS_11AC_ENABLE(pregistrypriv)
		&& is_supported_vht(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AC(rfctl)
		) {
		pframe = rtw_tdls_set_vht_cap(padapter, pframe, pattrib);
	}
#endif

#ifdef CONFIG_80211AX_HE
	if ((((pregistrypriv->ht_enable == _TRUE) && (pmlmeext->chandef.band == BAND_ON_24G))
#ifdef CONFIG_80211AC_VHT
		|| (REGSTY_IS_11AC_ENABLE(pregistrypriv) && (pmlmeext->chandef.band == BAND_ON_5G))
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
		|| (pmlmeext->chandef.band == BAND_ON_6G)
#endif
		)
		&& REGSTY_IS_11AX_ENABLE(pregistrypriv)
		&& is_supported_he(pregistrypriv->wireless_mode)
		&& RFCTL_REG_EN_11AX(rfctl)
		) {
		pframe = rtw_tdls_set_he_cap(padapter, pframe, pattrib);
	}
#endif

}


void rtw_build_tdls_peer_traffic_indication_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{

	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	u8 AC_queue = 0;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_dialog(pframe, pattrib, ptxmgmt);

	if (ptdls_sta->tdls_sta_state & TDLS_INITIATOR_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _FALSE);
	else if (ptdls_sta->tdls_sta_state & TDLS_RESPONDER_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);

	/* PTI control */
	/* PU buffer status */
	if (ptdls_sta->uapsd_bk & BIT(1))
		AC_queue = BIT(0);
	if (ptdls_sta->uapsd_be & BIT(1))
		AC_queue = BIT(1);
	if (ptdls_sta->uapsd_vi & BIT(1))
		AC_queue = BIT(2);
	if (ptdls_sta->uapsd_vo & BIT(1))
		AC_queue = BIT(3);
	pframe = rtw_set_ie(pframe, _PTI_BUFFER_STATUS_, 1, &AC_queue, &(pattrib->pktlen));

}

void rtw_build_tdls_peer_traffic_rsp_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{

	struct pkt_attrib	*pattrib = &pxmitframe->attrib;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_dialog(pframe, pattrib, ptxmgmt);

	if (ptdls_sta->tdls_sta_state & TDLS_INITIATOR_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _FALSE);
	else if (ptdls_sta->tdls_sta_state & TDLS_RESPONDER_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);
}

#ifdef CONFIG_TDLS_CH_SW
void rtw_build_tdls_ch_switch_req_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	struct sta_priv	*pstapriv = &padapter->stapriv;
	u16 switch_time = TDLS_CH_SWITCH_TIME * 1000, switch_timeout = TDLS_CH_SWITCH_TIMEOUT * 1000;

	ptdls_sta->ch_switch_time = switch_time;
	ptdls_sta->ch_switch_timeout = switch_timeout;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_target_ch(padapter, pframe, pattrib);
	pframe = rtw_tdls_set_reg_class(pframe, pattrib, ptdls_sta);

	if (ptdlsinfo->chsw_info.ch_offset != CHAN_OFFSET_NO_EXT) {
		switch (ptdlsinfo->chsw_info.ch_offset) {
		case CHAN_OFFSET_UPPER:
			pframe = rtw_tdls_set_second_channel_offset(pframe, pattrib, IEEE80211_SCA);
			break;
		case CHAN_OFFSET_LOWER:
			pframe = rtw_tdls_set_second_channel_offset(pframe, pattrib, IEEE80211_SCB);
			break;
		}
	}

	if (ptdls_sta->tdls_sta_state & TDLS_INITIATOR_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _FALSE);
	else if (ptdls_sta->tdls_sta_state & TDLS_RESPONDER_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);

	pframe = rtw_tdls_set_ch_sw(pframe, pattrib, ptdls_sta);

}

void rtw_build_tdls_ch_switch_rsp_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe, struct tdls_txmgmt *ptxmgmt, struct sta_info *ptdls_sta)
{

	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	struct sta_priv	*pstapriv = &padapter->stapriv;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_tdls_set_category(pframe, pattrib, RTW_WLAN_CATEGORY_TDLS);
	pframe = rtw_tdls_set_action(pframe, pattrib, ptxmgmt);
	pframe = rtw_tdls_set_status_code(pframe, pattrib, ptxmgmt);

	if (ptdls_sta->tdls_sta_state & TDLS_INITIATOR_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _FALSE);
	else if (ptdls_sta->tdls_sta_state & TDLS_RESPONDER_STATE)
		pframe = rtw_tdls_set_linkid(padapter, pframe, pattrib, _TRUE);

	pframe = rtw_tdls_set_ch_sw(pframe, pattrib, ptdls_sta);
}
#endif

#ifdef CONFIG_WFD
void rtw_build_tunneled_probe_req_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe)
{
	u8 i;
	_adapter *iface = NULL;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct wifidirect_info *pwdinfo;

	u8 category = RTW_WLAN_CATEGORY_P2P;
	u8 WFA_OUI[3] = { 0x50, 0x6f, 0x9a};
	u8 probe_req = 4;
	u8 wfdielen = 0;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 3, WFA_OUI, &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(probe_req), &(pattrib->pktlen));

	for (i = 0; i < dvobj->iface_nums; i++) {
		if ((iface) && rtw_is_adapter_up(iface) &&
			rtw_iface_at_same_hwband(padapter, iface)) {

			pwdinfo = &iface->wdinfo;
			if (pwdinfo->wfd_tdls_enable) {
				wfdielen = build_probe_req_wfd_ie(pwdinfo, pframe);
				pframe += wfdielen;
				pattrib->pktlen += wfdielen;
			}
		}
	}
}

void rtw_build_tunneled_probe_rsp_ies(_adapter *padapter, struct xmit_frame *pxmitframe, u8 *pframe)
{
	u8 i;
	_adapter *iface = NULL;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	struct wifidirect_info *pwdinfo;
	u8 category = RTW_WLAN_CATEGORY_P2P;
	u8 WFA_OUI[3] = { 0x50, 0x6f, 0x9a};
	u8 probe_rsp = 5;
	u8 wfdielen = 0;

	pframe = rtw_tdls_set_payload_type(pframe, pattrib);
	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 3, WFA_OUI, &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(probe_rsp), &(pattrib->pktlen));

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface) &&
			rtw_iface_at_same_hwband(padapter, iface)) {

			pwdinfo = &iface->wdinfo;
			if (pwdinfo->wfd_tdls_enable) {
				wfdielen = build_probe_resp_wfd_ie(pwdinfo, pframe, 1);
				pframe += wfdielen;
				pattrib->pktlen += wfdielen;
			}
		}
	}
}
#endif /* CONFIG_WFD */

void _tdls_tpk_timer_hdl(void *FunctionContext)
{
	struct sta_info *ptdls_sta = (struct sta_info *)FunctionContext;
	struct tdls_txmgmt txmgmt;

	_rtw_memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	ptdls_sta->TPK_count++;
	/* TPK_timer expired in a second */
	/* Retry timer should set at least 301 sec. */
	if (ptdls_sta->TPK_count >= (ptdls_sta->TDLS_PeerKey_Lifetime - 3)) {
		RTW_INFO("[TDLS] %s, Re-Setup TDLS link with "MAC_FMT" since TPK lifetime expires!\n",
			__FUNCTION__, MAC_ARG(ptdls_sta->phl_sta->mac_addr));
		ptdls_sta->TPK_count = 0;
		_rtw_memcpy(txmgmt.peer, ptdls_sta->phl_sta->mac_addr, ETH_ALEN);
		issue_tdls_setup_req(ptdls_sta->padapter, &txmgmt, _FALSE);
	}

	_set_timer(&ptdls_sta->TPK_timer, ONE_SEC);
}

#ifdef CONFIG_TDLS_CH_SW
void _tdls_ch_switch_timer_hdl(void *FunctionContext)
{
	struct sta_info *ptdls_sta = (struct sta_info *)FunctionContext;
	_adapter *padapter = ptdls_sta->padapter;
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;

	rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_END_TO_BASE_CHNL, 0);
	RTW_INFO("[TDLS] %s, can't get traffic from op_ch:%d\n", __func__,
		 rtw_get_oper_ch(padapter, ptdls_sta->padapter_link));
}

void _tdls_delay_timer_hdl(void *FunctionContext)
{
	struct sta_info *ptdls_sta = (struct sta_info *)FunctionContext;
	_adapter *padapter = ptdls_sta->padapter;
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;

	RTW_INFO("[TDLS] %s, op_ch:%d, tdls_state:0x%08x\n", __func__,
		 rtw_get_oper_ch(padapter, ptdls_sta->padapter_link), ptdls_sta->tdls_sta_state);
	pchsw_info->delay_switch_back = _TRUE;
}

void _tdls_stay_on_base_chnl_timer_hdl(void *FunctionContext)
{
	struct sta_info *ptdls_sta = (struct sta_info *)FunctionContext;
	_adapter *padapter = ptdls_sta->padapter;
	struct _ADAPTER_LINK *padapter_link = ptdls_sta->padapter_link;
	struct link_mlme_ext_priv *pmlmeext = &padapter_link->mlmeextpriv;
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;

	if (ptdls_sta != NULL) {
		if (pmlmeext->chandef.chan == rtw_get_oper_ch(padapter, padapter_link)) {
			issue_tdls_ch_switch_req(padapter, ptdls_sta);
			pchsw_info->ch_sw_state |= TDLS_WAIT_CH_RSP_STATE;
		}
	}
}

void _tdls_ch_switch_monitor_timer_hdl(void *FunctionContext)
{
	struct sta_info *ptdls_sta = (struct sta_info *)FunctionContext;
	_adapter *padapter = ptdls_sta->padapter;
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;

	rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_CH_SW_END, 0);
	RTW_INFO("[TDLS] %s, does not receive ch sw req\n", __func__);
}

#endif

void _tdls_handshake_timer_hdl(void *FunctionContext)
{
	struct sta_info *ptdls_sta = (struct sta_info *)FunctionContext;
	_adapter *padapter = NULL;

	if (ptdls_sta != NULL) {
		padapter = ptdls_sta->padapter;

		RTW_INFO("[TDLS] Handshake time out\n");
		if (ptdls_sta->tdls_sta_state & TDLS_LINKED_STATE)
			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA, 0);
		else
			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_LOCALLY, 0);
	}
}

void _tdls_pti_timer_hdl(void *FunctionContext)
{
	struct sta_info *ptdls_sta = (struct sta_info *)FunctionContext;
	_adapter *padapter = NULL;

	if (ptdls_sta != NULL) {
		padapter = ptdls_sta->padapter;

		if (ptdls_sta->tdls_sta_state & TDLS_WAIT_PTR_STATE) {
			RTW_INFO("[TDLS] Doesn't receive PTR from peer dev:"MAC_FMT"; "
				"Send TDLS Tear Down\n", MAC_ARG(ptdls_sta->phl_sta->mac_addr));
			rtw_tdls_cmd(padapter, ptdls_sta->phl_sta->mac_addr, TDLS_TEARDOWN_STA_TOOFAR, 0);
		}
	}
}

void rtw_init_tdls_timer(_adapter *padapter, struct sta_info *psta)
{
	psta->padapter = padapter;
	rtw_init_timer(&psta->TPK_timer, _tdls_tpk_timer_hdl, psta);
#ifdef CONFIG_TDLS_CH_SW
	rtw_init_timer(&psta->ch_sw_timer, _tdls_ch_switch_timer_hdl, psta);
	rtw_init_timer(&psta->delay_timer, _tdls_delay_timer_hdl, psta);
	rtw_init_timer(&psta->stay_on_base_chnl_timer, _tdls_stay_on_base_chnl_timer_hdl, psta);
	rtw_init_timer(&psta->ch_sw_monitor_timer, _tdls_ch_switch_monitor_timer_hdl, psta);
#endif
	rtw_init_timer(&psta->handshake_timer, _tdls_handshake_timer_hdl, psta);
	rtw_init_timer(&psta->pti_timer, _tdls_pti_timer_hdl, psta);
}

void rtw_cancel_tdls_timer(struct sta_info *psta)
{
	_cancel_timer_ex(&psta->TPK_timer);
#ifdef CONFIG_TDLS_CH_SW
	_cancel_timer_ex(&psta->ch_sw_timer);
	_cancel_timer_ex(&psta->delay_timer);
	_cancel_timer_ex(&psta->stay_on_base_chnl_timer);
	_cancel_timer_ex(&psta->ch_sw_monitor_timer);
#endif
	_cancel_timer_ex(&psta->handshake_timer);
	_cancel_timer_ex(&psta->pti_timer);
}

void rtw_tdls_teardown_pre_hdl(_adapter *padapter, struct sta_info *psta)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct sta_priv *pstapriv = &padapter->stapriv;

	rtw_cancel_tdls_timer(psta);

	_rtw_spinlock_bh(&(pstapriv->sta_hash_lock));
	if (ptdlsinfo->sta_cnt != 0)
		ptdlsinfo->sta_cnt--;
	_rtw_spinunlock_bh(&(pstapriv->sta_hash_lock));

	if (ptdlsinfo->sta_cnt < MAX_ALLOWED_TDLS_STA_NUM) {
		ptdlsinfo->sta_maximum = _FALSE;
		_rtw_memset(&ptdlsinfo->ss_record, 0x00, sizeof(struct tdls_ss_record));
	}

	if (ptdlsinfo->sta_cnt == 0)
		rtw_tdls_set_link_established(padapter, _FALSE);
	else
		RTW_INFO("Remain tdls sta:%02x\n", ptdlsinfo->sta_cnt);
}

void rtw_tdls_teardown_post_hdl(_adapter *padapter, struct sta_info *psta, u8 enqueue_cmd)
{
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	void *phl = GET_PHL_INFO(adapter_to_dvobj(padapter));
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	enum role_type rtype = PHL_RTYPE_STATION;
	enum rtw_phl_status status;
	int tid;

	/* Clear cam */
	rtw_clearstakey_cmd(padapter, psta, enqueue_cmd);

	for (tid = 0; tid < TID_NUM; tid++) {
		if (psta->recvreorder_ctrl[tid].enable == _TRUE) {
			psta->recvreorder_ctrl[tid].enable =_FALSE;
			rtw_phl_stop_rx_ba_session(phl, psta->phl_sta, tid);
			RTW_INFO(FUNC_ADPT_FMT"stop process tid %d \n",
				FUNC_ADPT_ARG(padapter), tid);
		}
	}

	rtw_sta_hal_media_status_rpt_cmd(padapter, psta, false, RTW_CMDF_DIRECTLY);

	/* Free tdls sta info */
	rtw_free_mld_stainfo(padapter, psta->phl_sta->mld);

	if (ptdlsinfo->sta_cnt == 0) {
		status = rtw_phl_cmd_wrole_change(phl, wrole, NULL,
			WR_CHG_TYPE, (u8*)&rtype, sizeof(enum role_type), PHL_CMD_DIRECTLY, 0);
		if (status != RTW_PHL_STATUS_SUCCESS) {
			RTW_ERR("%s - change to phl role type = %d fail with error = %d\n", __func__, rtype, status);
			rtw_warn_on(1);
		}
		/* Inform PHL for phl_mr_info_upt */
		rtw_send_tdls_sync_msg(padapter);
	}
}

int rtw_tdls_is_driver_setup(_adapter *padapter)
{
	return padapter->tdlsinfo.driver_setup;
}

const char *rtw_tdls_action_txt(enum TDLS_ACTION_FIELD action)
{
	switch (action) {
	case TDLS_SETUP_REQUEST:
		return "TDLS_SETUP_REQUEST";
	case TDLS_SETUP_RESPONSE:
		return "TDLS_SETUP_RESPONSE";
	case TDLS_SETUP_CONFIRM:
		return "TDLS_SETUP_CONFIRM";
	case TDLS_TEARDOWN:
		return "TDLS_TEARDOWN";
	case TDLS_PEER_TRAFFIC_INDICATION:
		return "TDLS_PEER_TRAFFIC_INDICATION";
	case TDLS_CHANNEL_SWITCH_REQUEST:
		return "TDLS_CHANNEL_SWITCH_REQUEST";
	case TDLS_CHANNEL_SWITCH_RESPONSE:
		return "TDLS_CHANNEL_SWITCH_RESPONSE";
	case TDLS_PEER_PSM_REQUEST:
		return "TDLS_PEER_PSM_REQUEST";
	case TDLS_PEER_PSM_RESPONSE:
		return "TDLS_PEER_PSM_RESPONSE";
	case TDLS_PEER_TRAFFIC_RESPONSE:
		return "TDLS_PEER_TRAFFIC_RESPONSE";
	case TDLS_DISCOVERY_REQUEST:
		return "TDLS_DISCOVERY_REQUEST";
	case TDLS_DISCOVERY_RESPONSE:
		return "TDLS_DISCOVERY_RESPONSE";
	default:
		return "UNKNOWN";
	}
}

#endif /* CONFIG_TDLS */
