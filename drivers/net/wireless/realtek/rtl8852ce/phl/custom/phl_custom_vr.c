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
#define _PHL_CUSTOMIZE_FEATURE_C_
#include "../phl_headers.h"

#ifdef CONFIG_PHL_CUSTOM_FEATURE_VR
#include "phl_custom_vr.h"
#include "phl_custom_vr_csi.h"

enum phl_mdl_ret_code
_is_vr_mode_valid(void* custom_ctx,
                  struct _custom_vr_ctx* vr_ctx,
                  struct phl_msg* msg, u32 size)
{
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;
	if (!vr_ctx->init.enable || cmd->len < size || vr_ctx->init.wifi_role == NULL) {
		PHL_INFO(" %s, evt_id(%d) not accepted\n",
		         __FUNCTION__,
		         MSG_EVT_ID_FIELD(msg->msg_id));
		ret = MDL_RET_FAIL;
		return ret;
	}

	return ret;
}

bool
_feature_vr_enable_init_setting(void* custom_ctx,
                             struct _custom_vr_ctx* vr_ctx)
{
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl->hal);

	vr_ctx->init.phl = phl;
#ifdef CONFIG_USB_HCI
	/* To Do: to offload gpio number in halmac for different ICs */
	rtw_hal_set_sw_gpio_mode(phl->phl_com, phl->hal
		, RTW_AX_SW_IO_MODE_OUTPUT_OD, 12);

	rtw_hal_set_sw_gpio_mode(phl->phl_com, phl->hal
		, RTW_AX_SW_IO_MODE_OUTPUT_OD, 14);
#endif
	rtw_hal_auto_debug_en_phy_util(hal_com, true);

	return true;
}

void
_feature_vr_enable_deinit_setting(void* custom_ctx,
                               struct _custom_vr_ctx* vr_ctx)
{
	/* TBD: merge codes from custom branch for the deinit setting*/
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl->hal);

	rtw_hal_auto_debug_en_phy_util(hal_com, false);

	return;
}

enum phl_mdl_ret_code
_phl_custom_vr_feature_enable(void* custom_ctx,
                          struct _custom_vr_ctx* vr_ctx,
                          struct phl_msg* msg)
{
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);

	if (cmd->len < sizeof(u32))
		return MDL_RET_FAIL;

	vr_ctx->init.enable = *(u32*)(cmd->data);
	vr_ctx->init.test_mode = 0;
	if (vr_ctx->init.enable){
		_feature_vr_enable_init_setting(custom_ctx, vr_ctx);
	} else {
		_feature_vr_enable_deinit_setting(custom_ctx, vr_ctx);
	}

	PHL_INFO(" %s, vr feature enable(%d)\n", __FUNCTION__, vr_ctx->init.enable);
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&(vr_ctx->init.enable),
	                           sizeof(u32));

	return MDL_RET_SUCCESS;
}

enum phl_mdl_ret_code
_phl_custom_vr_feature_query(void* custom_ctx,
                             struct _custom_vr_ctx* vr_ctx,
                             struct phl_msg* msg)
{
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	if (cmd->len < sizeof(u32))
		return ret;
	PHL_INFO("%s, vr query feature enable(%d)\n",
	         __FUNCTION__,
	         vr_ctx->init.enable);
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&(vr_ctx->init.enable),
	                           sizeof(u32));
	ret = MDL_RET_SUCCESS;
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_testmode_param(void* custom_ctx,
                              struct _custom_vr_ctx* vr_ctx,
                              struct phl_msg* msg)
{
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;

	if (cmd->len < sizeof(u32))
		return ret;
	vr_ctx->init.test_mode = *(u32*)(cmd->data);
	PHL_INFO("%s, test mode(0x%x)\n", __FUNCTION__,
	         vr_ctx->init.test_mode);
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));

	ret = MDL_RET_SUCCESS;
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_ampdu_cfg(void* custom_ctx,
                         struct _custom_vr_ctx* vr_ctx,
                         struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	void *d = phl_to_drvpriv(phl);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct rtw_phl_custom_ampdu_cfg custom_vr_ampdu_cfg = {0};
	u32 size = sizeof(struct rtw_phl_custom_ampdu_cfg);
	u8* val = cmd->data;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	u8 idx = 0;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	wrole = vr_ctx->init.wifi_role;
	_os_mem_cpy(d, &custom_vr_ampdu_cfg, val, size);

	PHL_INFO("%s, halsta(%d) ampdu dur(%d) num(%d)\n",
	         __FUNCTION__,
	         hal_status,
	         custom_vr_ampdu_cfg.max_agg_time_32us,
	         custom_vr_ampdu_cfg.max_agg_num);

	for (idx = 0; idx < wrole->rlink_num; idx++) {
		rlink = get_rlink(wrole, idx);

		hal_status = rtw_hal_custom_cfg_tx_ampdu(phl->hal,
		                                         rlink,
		                                         &custom_vr_ampdu_cfg);
		if (hal_status != RTW_HAL_STATUS_SUCCESS)
			ret = MDL_RET_FAIL;
	}

exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_ampdu_query(void* custom_ctx,
                           struct _custom_vr_ctx* vr_ctx,
                           struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct rtw_phl_custom_ampdu_cfg custom_vr_ampdu_cfg = {0};
	u32 size = sizeof(struct rtw_phl_custom_ampdu_cfg);
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	u8 idx = 0;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	wrole = vr_ctx->init.wifi_role;

	for (idx = 0; idx < wrole->rlink_num; idx++) {
		rlink = get_rlink(wrole, idx);

		hal_status = rtw_hal_get_ampdu_cfg(phl->hal,
		                                   rlink,
		                                   &custom_vr_ampdu_cfg);
		if (hal_status != RTW_HAL_STATUS_SUCCESS)
			ret = MDL_RET_FAIL;
	}

	PHL_INFO(" %s, ampdu dur(%d) time(%d)\n",
	         __FUNCTION__,
	         custom_vr_ampdu_cfg.max_agg_time_32us,
	         custom_vr_ampdu_cfg.max_agg_num);
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&custom_vr_ampdu_cfg,
	                           sizeof(struct rtw_phl_custom_ampdu_cfg));
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_pdthr_cfg(void* custom_ctx,
                         struct _custom_vr_ctx* vr_ctx,
                         struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	int pd_thr = 0xff;
	u32 size = sizeof(int);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	wrole = vr_ctx->init.wifi_role;
	rlink = get_rlink(wrole, RTW_RLINK_PRIMARY);

	pd_thr = *(int*)(cmd->data);
	PHL_INFO("%s, pd_thr(%d)\n", __FUNCTION__, pd_thr);

	hal_status = rtw_hal_set_pkt_detect_thold(phl->hal,
						rlink->hw_band,
						(u32)pd_thr);
	PHL_INFO("%s, hal_status(%d)\n", __FUNCTION__, hal_status);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		ret = MDL_RET_FAIL;
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_pdthr_query(void* custom_ctx,
                           struct _custom_vr_ctx* vr_ctx,
                           struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	int pd_thr = 0xff;
	u32 size = sizeof(int);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	wrole = vr_ctx->init.wifi_role;
	rlink = get_rlink(wrole, RTW_RLINK_PRIMARY);

	/*confirm whether pd thr is enabling or not*/
	pd_thr = rtw_hal_query_pkt_detect_thold(phl->hal,
	                                        true,
	                                        rlink->hw_band);
	if (pd_thr == 0) {
		PHL_INFO("%s, disable! pd_thr(%d)\n", __FUNCTION__, pd_thr);
	} else {
		pd_thr = rtw_hal_query_pkt_detect_thold(phl->hal,
		                                        false,
		                                        rlink->hw_band);
		PHL_INFO("%s, pd_thr(%d)\n", __FUNCTION__, pd_thr);
	}
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&pd_thr,
	                           sizeof(int));

	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_pop_cfg(void* custom_ctx,
                       struct _custom_vr_ctx* vr_ctx,
                       struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	u32 size = sizeof(u32);
	u32 pop_enable = 0xff;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	wrole = vr_ctx->init.wifi_role;
	rlink = get_rlink(wrole, RTW_RLINK_PRIMARY);

	pop_enable = *(u32*)(cmd->data);
	PHL_INFO("%s, pop_enable(%d)\n", __FUNCTION__, pop_enable);
	if (pop_enable != 0xff) {
		hal_status = rtw_hal_set_pop_en(phl->hal,
		                                (bool)pop_enable,
		                                rlink->hw_band);
		if (hal_status != RTW_HAL_STATUS_SUCCESS)
			ret = MDL_RET_FAIL;
	}
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));

	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_pop_query(void* custom_ctx,
                         struct _custom_vr_ctx* vr_ctx,
                         struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	u32 size = sizeof(u32);
	u32 pop_enable = 0xff;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS){
		goto exit;
	}

	wrole = vr_ctx->init.wifi_role;
	rlink = get_rlink(wrole, RTW_RLINK_PRIMARY);

	pop_enable = rtw_hal_query_pop_en(phl->hal, rlink->hw_band);
	PHL_INFO("%s, pop_en(%d)\n", __FUNCTION__, pop_enable);
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&pop_enable,
	                            sizeof(u32));

	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_set_tx_rate_masking(void* custom_ctx,
                                   struct _custom_vr_ctx* vr_ctx,
                                   struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	void *d = phl_to_drvpriv(phl);
	struct rtw_phl_stainfo_t *sta = NULL;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	_os_spinlockfg sp_flags;
	u64 tx_mask = (u64)(*(u32*)(cmd->data));
	u64 tmp_mask = 0;
	u8 idx = 0;


	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, sizeof(u32));
	if( ret != MDL_RET_SUCCESS){
		goto exit;
	}
	wrole = vr_ctx->init.wifi_role;

	/* Just only change the 1ss/2ss MCS rate*/
	tx_mask = (tx_mask << 12);

	/* search the stations in the queue*/

	for (idx = 0; idx < wrole->rlink_num; idx++) {
		rlink = get_rlink(wrole, idx);

		_os_spinlock(d, &rlink->assoc_sta_queue.lock, _irq, &sp_flags);

		phl_list_for_loop(sta, struct rtw_phl_stainfo_t,
					&rlink->assoc_sta_queue.queue, list) {
			if (sta == list_first_entry(&rlink->assoc_sta_queue.queue,
						struct rtw_phl_stainfo_t, list))
				continue;
			if (sta) {
				tmp_mask = sta->hal_sta->ra_info.cur_ra_mask & 0xfff;
				tx_mask |= tmp_mask;
				sta->hal_sta->ra_info.cur_ra_mask = tx_mask;
				PHL_INFO("%s: tx mask(0x%016llx) macid(%d)\n",
				         __FUNCTION__, tx_mask, sta->macid);
			}
		}
		_os_spinunlock(d, &rlink->assoc_sta_queue.lock, _irq, &sp_flags);
	}
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_get_tx_rate_masking(void* custom_ctx,
                                   struct _custom_vr_ctx* vr_ctx,
                                   struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	void *d = phl_to_drvpriv(phl);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct rtw_phl_stainfo_t *sta = NULL;
	u32 tx_mask = 0;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	_os_spinlockfg sp_flags;
	u8 idx = 0;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, sizeof(u32));
	if( ret != MDL_RET_SUCCESS){
		goto exit;
	}

	wrole = vr_ctx->init.wifi_role;

	/** Search the stations in the queue
	 *  We just need to query the first sta because all the masking
	 *  setting are the same for all sta.
	 */

	for (idx = 0; idx < wrole->rlink_num; idx++) {
		rlink = get_rlink(wrole, idx);

		_os_spinlock(d, &rlink->assoc_sta_queue.lock, _irq, &sp_flags);
		phl_list_for_loop(sta, struct rtw_phl_stainfo_t,
					&rlink->assoc_sta_queue.queue, list) {
			if (sta == list_first_entry(&rlink->assoc_sta_queue.queue,
						struct rtw_phl_stainfo_t, list))
				continue;
			if (sta) {
				tx_mask = (u32)(sta->hal_sta->ra_info.cur_ra_mask >> 12);
				break;
			}
		}

		_os_spinunlock(d, &rlink->assoc_sta_queue.lock, _irq, &sp_flags);
	}

	PHL_INFO("%s: tx mask(%x)\n", __FUNCTION__, tx_mask);

exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&tx_mask,
	                           sizeof(u32));
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_set_tx_rate_rty_tbl(void* custom_ctx,
                                   struct _custom_vr_ctx* vr_ctx,
                                   struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	void *d = phl_to_drvpriv(phl);
	u8* val = (u8*)(cmd->data);
	u8 i = 0;
	u32 size = sizeof(struct _vr_tx_rty_param);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if( ret != MDL_RET_SUCCESS){
		goto exit;
	}

	_os_mem_cpy(d, &vr_ctx->tx_rty_param, val, size);

	PHL_INFO("%s retry setting %s\n",
	          __func__,
	          vr_ctx->tx_rty_param.enable ? "enable" : "disable");

	for(i = 0; i < 24 ; i++){
	PHL_INFO("%s retry setting index(%d) rty_count(%d)\n",
	          __func__, i, vr_ctx->tx_rty_param.rty_rate_tbl[i]);
	}

	rtw_hal_set_tx_rate_rty_tbl(phl->hal,
	                            (bool)vr_ctx->tx_rty_param.enable,
	                            vr_ctx->tx_rty_param.rty_rate_tbl);

exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_get_tx_rate_rty_tbl(void* custom_ctx,
                                   struct _custom_vr_ctx* vr_ctx,
                                   struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	u8 i = 0;
	u32 size = sizeof(struct _vr_tx_rty_param);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if( ret != MDL_RET_SUCCESS){
		goto exit;
	}

	PHL_INFO("%s retry setting %s\n",
	          __func__,
	          vr_ctx->tx_rty_param.enable ? "enable" : "disable");

	for(i = 0; i < 24 ; i++){
	PHL_INFO("%s retry setting index(%d) rty_count(%d)\n",
	          __func__, i, vr_ctx->tx_rty_param.rty_rate_tbl[i]);
	}
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&vr_ctx->tx_rty_param,
	                           size);
	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_edca_query(void* custom_ctx,
                          struct _custom_vr_ctx* vr_ctx,
                          struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct rtw_edca_param edca_param = {0};
	struct rtw_wifi_role_t *wifi_role = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	u8 idx = 0;

	edca_param.ac = 0xFF;
	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, sizeof(u8));
	if (ret != MDL_RET_SUCCESS)
		goto exit;

	wifi_role = vr_ctx->init.wifi_role;
	edca_param.ac = *((u8*)(cmd->data));

	for (idx = 0; idx < wifi_role->rlink_num; idx++) {
		rlink = get_rlink(wifi_role, idx);

		hal_status = rtw_hal_get_edca(phl->hal,
		                              rlink,
		                              &edca_param);
		if (hal_status != RTW_HAL_STATUS_SUCCESS)
			ret = MDL_RET_FAIL;
	}

	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		edca_param.ac = 0xFF;

	PHL_INFO("%s, custom_vr_edca_param rpt: ac(%d), param(0x%x)\n",
	         __FUNCTION__,
	         edca_param.ac,
	         edca_param.param);
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&edca_param,
	                           sizeof(struct rtw_edca_param));

	return MDL_RET_SUCCESS;
}

enum phl_mdl_ret_code
_phl_custom_vr_sr_cfg(void* custom_ctx,
                       struct _custom_vr_ctx* vr_ctx,
                       struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	u32 size = sizeof(u8);
	u8 sr_enable = false;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	sr_enable = *(u8*)(cmd->data);
	PHL_INFO("%s, sr_enable(%d)\n", __FUNCTION__, sr_enable);

	hal_status = rtw_hal_set_spatial_reuse_en(phl->hal, (bool)sr_enable);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		ret = MDL_RET_FAIL;

exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));

	return ret;
}

enum phl_mdl_ret_code
_phl_custom_vr_sr_query(void* custom_ctx,
                         struct _custom_vr_ctx* vr_ctx,
                         struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	u32 size = sizeof(u8);
	u8 sr_enable = 0xff;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS){
		goto exit;
	}

	sr_enable = rtw_hal_is_spatial_reuse_en(phl->hal);
	PHL_INFO("%s, sr_en(%d)\n", __FUNCTION__, sr_enable);
exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           &sr_enable,
	                            sizeof(u8));

	return ret;
}

enum phl_mdl_ret_code _phl_custom_vr_rf_scramble(void *custom_ctx,
						 struct _custom_vr_ctx *vr_ctx,
						 struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	void *d = phl_to_drvpriv(phl);
	struct rtw_custom_decrpt *cmd =
	    (struct rtw_custom_decrpt *)(msg->inbuf);
	struct _vr_rf_scrmb *val = (struct _vr_rf_scrmb *)(cmd->data);
	u32 size = sizeof(struct _vr_rf_scrmb);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		val->status = RTW_HAL_STATUS_FAILURE;
		goto exit;
	}

	PHL_INFO("%s: scramble cmd(%s)\n", __func__,
		 (val->is_set) ? "Set_scrmb" : "Get_scrmb");
	if (val->is_set) {
		hal_status = rtw_hal_set_usr_frame_to_act(
		    phl->hal, val->mode, val->to_thr, val->trigger_cnt,
		    val->sw_def_bmp);
		val->status = (u8)hal_status;
		if (hal_status == RTW_HAL_STATUS_SUCCESS)
			_os_mem_cpy(d, &vr_ctx->cur_scrmb_param, val, size);
	} else {
		_os_mem_cpy(d, val, &vr_ctx->cur_scrmb_param, size);
	}

exit:
	phl_custom_prepare_evt_rpt(custom_ctx, cmd->evt_id, cmd->customer_id,
				   (u8 *)val, sizeof(struct _vr_rf_scrmb));

	return ret;
}

struct custom_vr_perf_metrics_rlink {
	u8 mac_addr[MAC_ALEN];
	u8 rssi;
	char tx_rate_str[32];
	char rx_rate_str[32];
	u8 clm_ratio;
	u8 nhm_ratio;
	u8 snr_avg;
};

struct custom_vr_perf_metrics {
	u8 rlink_num;
	struct custom_vr_perf_metrics_rlink perf_metrics_rlink[RTW_RLINK_MAX];
};

enum phl_mdl_ret_code
_phl_custom_vr_perf_mertics_query(void* custom_ctx,
                                 struct _custom_vr_ctx* vr_ctx,
                                 struct phl_msg* msg) {
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	void *drv = phl_to_drvpriv(phl);
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct custom_vr_perf_metrics perf_metrics = {0};
	struct custom_vr_perf_metrics_rlink *curr = NULL;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_wifi_role_link_t *rlink = NULL;
	struct rtw_phl_stainfo_t *n, *psta;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl->hal);
	struct rtw_env_report env_rpt = {0};
	u8 idx = 0;

	PHL_INFO("Enter %s\n", __func__);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, sizeof(u8));
	if (ret != MDL_RET_SUCCESS)
		goto exit;

	wrole = vr_ctx->init.wifi_role;
	perf_metrics.rlink_num = wrole->rlink_num;
	PHL_INFO("%s(): rlink_num = %d\n", __func__, perf_metrics.rlink_num);
	for (idx = 0; idx < wrole->rlink_num; idx++) {
		rlink = get_rlink(wrole, idx);
		curr = &perf_metrics.perf_metrics_rlink[idx];
		_os_spinlock(drv, &rlink->assoc_sta_queue.lock, _bh, NULL);
		phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
		       &rlink->assoc_sta_queue.queue, list) {
			_os_mem_cpy(drv, curr->mac_addr, psta->mac_addr, MAC_ALEN);
			PHL_INFO("mac addr %02x-%02x-%02x-%02x-%02x-%02x\n",
			         curr->mac_addr[0], curr->mac_addr[1],
			         curr->mac_addr[2], curr->mac_addr[3],
			         curr->mac_addr[4], curr->mac_addr[5]);

			PHL_INFO("WROLE-IDX:%d RLINK-IDX:%d wlan_mode:0x%02x, chan:%d, bw:%d, rlink_state:%s\n",
			         psta->wrole->id,
			         psta->rlink->id,
			         psta->wmode,
			         psta->chandef.chan,
			         psta->chandef.bw,
			         rlink->mstate?((rlink->mstate & MLME_LINKING)?"Linking":"Linked Up"):"No Link");

			curr->rssi = psta->hal_sta->rssi_stat.ma_rssi;
			PHL_INFO("[Stats] MA RSSI:%d(dBm)\n",
			         curr->rssi - PHL_MAX_RSSI);

			convert_tx_rate(psta->hal_sta->ra_info.rpt_rt_i.mode,
			                 psta->hal_sta->ra_info.rpt_rt_i.mcs_ss_idx,
			                 curr->tx_rate_str, 32);
			PHL_INFO("[Stats] Tx Rate:%s\n", curr->tx_rate_str);

			convert_rx_rate(psta->stats.rx_rate, curr->rx_rate_str,
			                 32);
			PHL_INFO("[Stats] Rx Rate:%s\n", curr->rx_rate_str);
		}
		_os_spinunlock(drv, &rlink->assoc_sta_queue.lock, _bh, NULL);

		rtw_hal_env_rpt(hal_com, &env_rpt, rlink->hw_band);
		curr->clm_ratio = env_rpt.clm_ratio;
		curr->nhm_ratio = env_rpt.nhm_ratio;
		PHL_INFO("[Stats] clm_ratio = %d\n", curr->clm_ratio);
		PHL_INFO("[Stats] nhm_ratio = %d\n", curr->nhm_ratio);

		rtw_hal_query_snr_avg(hal_com, &curr->snr_avg, rlink->hw_band);
		PHL_INFO("[Stats] snr_avg = %d\n", curr->snr_avg);
	}

exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&perf_metrics,
	                           sizeof(struct custom_vr_perf_metrics));

	PHL_INFO("Leave %s\n", __func__);
	return MDL_RET_SUCCESS;
}

enum phl_mdl_ret_code _phl_custom_vr_set_ant_switch(
    void *custom_ctx, struct _custom_vr_ctx *vr_ctx, struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_info_t *phl = phl_custom_get_phl_info(custom_ctx);
	struct rtw_custom_decrpt *cmd =
	    (struct rtw_custom_decrpt *)(msg->inbuf);
	u32 path = *(u32 *)(cmd->data);
	u32 size = sizeof(u32);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	PHL_INFO("%s: set rf path(%d)\n", __func__, path);

	switch (path) {
	case RF_PATH_AC:
		if ((rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 0, 12) !=
		     RTW_HAL_STATUS_SUCCESS) ||
		    (rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 0, 14) !=
		     RTW_HAL_STATUS_SUCCESS))
			ret = MDL_RET_FAIL;
		PHL_INFO("%s: set path to AC\n", __func__);
		break;
	case RF_PATH_AD:
		if ((rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 0, 12) !=
		     RTW_HAL_STATUS_SUCCESS) ||
		    (rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 1, 14) !=
		     RTW_HAL_STATUS_SUCCESS))
			ret = MDL_RET_FAIL;
		PHL_INFO("%s: set path to AD\n", __func__);
		break;
	case RF_PATH_BC:
		if ((rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 1, 12) !=
		     RTW_HAL_STATUS_SUCCESS) ||
		    (rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 0, 14) !=
		     RTW_HAL_STATUS_SUCCESS))
			ret = MDL_RET_FAIL;
		PHL_INFO("%s: set path to BC\n", __func__);
		break;
	case RF_PATH_BD:
		if ((rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 1, 12) !=
		     RTW_HAL_STATUS_SUCCESS) ||
		    (rtw_hal_sw_gpio_ctrl(phl->phl_com, phl->hal, 1, 14) !=
		     RTW_HAL_STATUS_SUCCESS))
			ret = MDL_RET_FAIL;
		PHL_INFO("%s: set path to BD\n", __func__);
		break;
	default:
		PHL_INFO("%s: unexpected rf path!\n", __func__);
		break;
	}

exit:
	phl_custom_prepare_evt_rpt(custom_ctx, cmd->evt_id, cmd->customer_id,
				   (u8 *)&ret, sizeof(u32));

	return ret;
}

#ifdef CONFIG_PHL_CHANNEL_INFO_VR
enum phl_mdl_ret_code
_phl_custom_vr_csi_cfg(void* custom_ctx,
                       struct _custom_vr_ctx* vr_ctx,
                       struct phl_msg* msg) {
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	u32 size = sizeof(u8);
	u8 csi_enable = false;

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS) {
		goto exit;
	}

	csi_enable = *(u8*)(cmd->data);
	PHL_INFO("%s, csi_enable(%d)\n", __FUNCTION__, csi_enable);

	if (csi_enable) {
		struct rtw_wifi_role_t *wrole = NULL;
		struct rtw_wifi_role_link_t *rlink = NULL;

		wrole = vr_ctx->init.wifi_role;
		rlink = get_rlink(wrole, RTW_RLINK_PRIMARY);
		ret = rtw_phl_custom_csi_start(custom_ctx,
		                               &vr_ctx->csi_ctrl,
		                               rlink);
	} else {
		ret = rtw_phl_custom_csi_stop(custom_ctx, &vr_ctx->csi_ctrl);
	}

exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&ret,
	                           sizeof(u8));

	return ret;
}

/*
 * custom_vr_csi - channel state information,
 * @len: length of channel info buffer
 * @buf: channel info buffer
 *       buf size is limited by minumim of MAX_DATA_SIZE (VR command data size)
 *       and CHAN_INFO_MAX_SIZE.
 */
struct custom_vr_csi {
	u32 len;
	u8 buf[MAX_DATA_SIZE - sizeof(u32)];
	/* u8 buf[CHAN_INFO_MAX_SIZE]; */
};

enum phl_mdl_ret_code
_phl_custom_vr_csi_query(void* custom_ctx,
                         struct _custom_vr_ctx* vr_ctx,
                         struct phl_msg* msg) {
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct rtw_custom_decrpt *cmd = (struct rtw_custom_decrpt *)(msg->inbuf);
	struct custom_vr_csi vr_csi = {0};
	u32 size = sizeof(struct custom_vr_csi);
	u32 i = 0, print_len = 0;
	u64 *buff_tmp = NULL;

	PHL_INFO("%s\n", __func__);

	ret = _is_vr_mode_valid(custom_ctx, vr_ctx, msg, size);
	if (ret != MDL_RET_SUCCESS)
		goto exit;

	ret = rtw_phl_custom_csi_rslt_query(custom_ctx,
	                                    (u8 *)vr_csi.buf,
	                                    &vr_csi.len);

	print_len = vr_csi.len >> 3;
	if (vr_csi.len % 8)
		print_len++;
	buff_tmp = (u64 *)vr_csi.buf;
	PHL_TRACE(COMP_PHL_CHINFO, _PHL_INFO_, "%s, CSI raw data: len = %d\n",
	          __func__, vr_csi.len);
	for (i = 0; i < print_len; i++)
		PHL_TRACE(COMP_PHL_CHINFO, _PHL_INFO_, "[%02d]0x%016llx\n",
		          i, buff_tmp[i]);

exit:
	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&vr_csi,
	                           sizeof(struct custom_vr_csi));

	return MDL_RET_SUCCESS;
}
#endif /* CONFIG_PHL_CHANNEL_INFO_VR */

enum phl_mdl_ret_code
phl_custom_hdl_vr_evt(void* dispr,
                      void* custom_ctx,
                      struct _custom_vr_ctx* vr_ctx,
                      struct phl_msg* msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	u8 prephase = (IS_MSG_IN_PRE_PHASE(msg->msg_id)) ? (true) : (false);

	if (prephase == true)
		return MDL_RET_SUCCESS;

	switch (MSG_EVT_ID_FIELD(msg->msg_id)) {
		case MSG_EVT_CUSTOME_FEATURE_ENABLE:
			ret = _phl_custom_vr_feature_enable(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_CUSTOME_FEATURE_QUERY:
			ret = _phl_custom_vr_feature_query(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_CUSTOME_TESTMODE_PARAM:
			ret = _phl_custom_vr_testmode_param(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_EDCA_QUERY:
			ret = _phl_custom_vr_edca_query(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_AMPDU_CFG:
			ret = _phl_custom_vr_ampdu_cfg(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_AMPDU_QUERY:
			ret = _phl_custom_vr_ampdu_query(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_PDTHR_CFG:
			ret = _phl_custom_vr_pdthr_cfg(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_PDTHR_QUERY:
			ret = _phl_custom_vr_pdthr_query(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_POP_CFG:
			ret = _phl_custom_vr_pop_cfg(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_POP_QUERY:
			ret = _phl_custom_vr_pop_query(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_SET_TX_RATE_MASKING:
			ret = _phl_custom_vr_set_tx_rate_masking(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_GET_TX_RATE_MASKING:
			ret = _phl_custom_vr_get_tx_rate_masking(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_SET_TX_RATE_RTY_TBL:
			ret = _phl_custom_vr_set_tx_rate_rty_tbl(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_GET_TX_RATE_RTY_TBL:
			ret = _phl_custom_vr_get_tx_rate_rty_tbl(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_GET_PERF_METRICS:
			ret = _phl_custom_vr_perf_mertics_query(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_SET_SPATIAL_REUSE:
			ret = _phl_custom_vr_sr_cfg(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_GET_SPATIAL_REUSE:
			ret = _phl_custom_vr_sr_query(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_RF_SCRAMBLE:
			ret = _phl_custom_vr_rf_scramble(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_SET_ANT_SWITCH:
			ret = _phl_custom_vr_set_ant_switch(custom_ctx, vr_ctx,
							    msg);
			break;
#ifdef CONFIG_PHL_CHANNEL_INFO_VR
		case MSG_EVT_SET_CSI:
			ret = _phl_custom_vr_csi_cfg(custom_ctx, vr_ctx, msg);
			break;
		case MSG_EVT_GET_CSI:
			ret = _phl_custom_vr_csi_query(custom_ctx, vr_ctx, msg);
			break;
#endif /* CONFIG_PHL_CHANNEL_INFO_VR */
		default:
			ret = MDL_RET_SUCCESS;
			break;
	}
	PHL_INFO("%s, evt(%d), ret(%d)\n", __FUNCTION__,
	         MSG_EVT_ID_FIELD(msg->msg_id),
	         ret);
	return ret;
}
enum phl_mdl_ret_code
phl_custom_hdl_vr_fail_evt(void* dispr,
                           void* custom_ctx,
                           struct _custom_vr_ctx* vr_ctx,
                           struct phl_msg* msg)
{
	return MDL_RET_IGNORE;
}

enum phl_mdl_ret_code
_phl_custom_vr_set_wifi_role(void* dispr,
                             void* custom_ctx,
                             struct _custom_vr_ctx* vr_ctx,
                             struct rtw_custom_decrpt *cmd)
{
	enum phl_mdl_ret_code status = MDL_RET_SUCCESS;
	u32 size = sizeof(struct rtw_wifi_role_t *);

	PHL_INFO("%s, start\n", __FUNCTION__);

	if (cmd->len < size) {
		PHL_INFO("%s: illegal info len\n", __FUNCTION__);
		status = MDL_RET_FAIL;
		return status;
	}

	vr_ctx->init.wifi_role = *(struct rtw_wifi_role_t **)(cmd->data);

	phl_custom_prepare_evt_rpt(custom_ctx,
	                           cmd->evt_id,
	                           cmd->customer_id,
	                           (u8*)&status,
	                           sizeof(u8));

	PHL_INFO("%s, status(%d).\n", __FUNCTION__, status);

	return status;
}

enum phl_mdl_ret_code
phl_custom_vr_feature_set_hdlr(void* dispr,
                               void* custom_ctx,
                               struct _custom_vr_ctx* vr_ctx,
                               struct rtw_custom_decrpt *cmd)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;

	switch (cmd->evt_id) {
		case MSG_EVT_CUSTOME_SET_WIFI_ROLE:
			ret = _phl_custom_vr_set_wifi_role(dispr, custom_ctx, vr_ctx, cmd);
			break;
		default:
			ret = MDL_RET_SUCCESS;
			break;
	}
	PHL_INFO("%s, evt(%d), ret(%d)\n", __FUNCTION__, cmd->evt_id, ret);
	return ret;
}

enum phl_mdl_ret_code
phl_custom_vr_feature_query_hdlr(void* dispr,
                                 void* custom_ctx,
                                 struct _custom_vr_ctx* vr_ctx,
                                 struct rtw_custom_decrpt *cmd)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;

	PHL_INFO("%s, evt(%d), ret(%d)\n", __FUNCTION__, cmd->evt_id, ret);
	return ret;
}
#endif
