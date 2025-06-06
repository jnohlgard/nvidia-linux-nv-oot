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
#define _HAL_BTC_ACTION_C_
#include "../hal_headers_le.h"
#include "hal_btc.h"
#include "halbtc_fw.h"
#include "halbtc_fwdef.h"
#include "halbtc_action.h"
#include "halbtc_def.h"

#ifdef CONFIG_BTCOEX

#define _set_cx_ctrl(btc, val) rtw_hal_mac_set_coex_ctrl(btc->hal, val)

/* tdma policy template */
struct fbtc_tdma t_def[] = {
	{ CXTDMA_OFF,    CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /*CXTD_OFF*/
	{ CXTDMA_OFF,    CXFLC_OFF, CXTPS_OFF, 0, 0, 1, 0, 0}, /*CXTD_B2*/
	{ CXTDMA_OFF,    CXFLC_OFF, CXTPS_OFF, 0, 0, 2, 0, 0}, /*CXTD_OFF_EXT*/
	{ CXTDMA_FIX,    CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /* CXTD_FIX */
	{ CXTDMA_FIX,  CXFLC_NULLP,  CXTPS_ON, 0, 5, 0, 0, 0}, /* CXTD_PFIX */
	{ CXTDMA_AUTO,   CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /* CXTD_AUTO */
	{ CXTDMA_AUTO, CXFLC_NULLP,  CXTPS_ON, 0, 5, 0, 0, 0}, /* CXTD_PAUTO */
	{CXTDMA_AUTO2,   CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /* CXTD_AUTO2 */
	{CXTDMA_AUTO2, CXFLC_NULLP,  CXTPS_ON, 0, 5, 0, 0, 0} /* CXTD_PAUTO2 */
};

/* slot policy template */
struct fbtc_slot s_def[] = {
	{100, SLOT_MIX, 0x55555555}, /* CXST_OFF */
	{  5, SLOT_ISO, 0xea5a5a5a}, /* CXST_B2W */
	{ 70, SLOT_ISO, 0xea5a5a5a}, /* CXST_W1 */
	{ 15, SLOT_ISO, 0xea5a5a5a}, /* CXST_W2 */
	{ 15, SLOT_ISO, 0xea5a5a5a}, /* CXST_W2B */
	{250, SLOT_MIX, 0xe5555555}, /* CXST_B1 */
	{  7, SLOT_ISO, 0xea5a5a5a}, /* CXST_B2 */
	{  5, SLOT_MIX, 0xe5555555}, /* CXST_B3 */
	{ 50, SLOT_MIX, 0xe5555555}, /* CXST_B4 */
	{ 20, SLOT_ISO, 0xea5a5a5a}, /* CXST_LK */
	{500, SLOT_MIX, 0x55555555}, /* CXST_BLK */
	{  5, SLOT_MIX, 0xea5a5a5a}, /* CXST_E2G */
	{  5, SLOT_ISO, 0xffffffff}, /* CXST_E5G */
	{  5, SLOT_MIX, 0xea555555}, /* CXST_EBT */
	{  5, SLOT_MIX, 0x55555555}, /* CXST_ENULL */
	{250, SLOT_MIX, 0xea5a6a5a}, /* CXST_WLK */
	{ 50, SLOT_ISO, 0xffffffff}, /* CXST_W1FDD */
	{ 50, SLOT_ISO, 0xffffdfff}  /* CXST_B1FDD */
};


const u32 cxtbl[] = {
	0xffffffff, /* 0 */
	0xaaaaaaaa, /* 1 */
	0xe5555555, /* 2 */
	0xee555555, /* 3 */
	0xd5555555, /* 4 */
	0x5a5a5a5a, /* 5 */
	0xfa5a5a5a, /* 6 */
	0xda5a5a5a, /* 7 */
	0xea5a5a5a, /* 8 */
	0x6a5a5aaa, /* 9 */
	0x6a5a6a5a, /* 10 */
	0x6a5a6aaa, /* 11 */
	0x6afa5afa, /* 12 */
	0xaaaa5aaa, /* 13 */
	0xaaffffaa, /* 14 */
	0xaa5555aa, /* 15 */
	0xfafafafa, /* 16 */
	0xffffddff, /* 17 */
	0xdaffdaff, /* 18 */
	0xfafa5afa, /* 19 */
	0xea6a6a6a, /* 20 */
	0xea55556a, /* 21 */
	0xaafafafa, /* 22 */
	0xfafaaafa, /* 23 */
	0xfafffaff, /* 24 */
	0xea6a5a5a, /* 25 */
	0xfaff5aff, /* 26 */
	0xffffdfff  /* 27 */
};

/* fdd train control parameters  */
struct btc_fddt_time_ctrl ft_tctrl_def = {20, 20, 40, 0};
struct btc_fddt_break_check ft_bchk_def = {0, 2, 80, 20, 8, 8, -85, 5};
struct btc_fddt_fail_check ft_fchk_def = {0, 2, 115, 85};

struct btc_fddt_cell cell_ul_def[5][5] = { /* BT-RSSI 0~4, WL-RSSI 0~4 */
	{{0,0,0,0},    {0,0,0,0},   {0,0,0,0},   {0,0,0,0},   {0,0,0,0}},
	{{0,0,0,0},    {5,15,10,4}, {3,13,10,4}, {-2,8,10,4}, {-5,5,10,4}},
	{{0,0,0,0},    {5,15,10,4}, {3,13,10,4}, {-2,8,10,4}, {-5,5,10,4}},
	{{15,15,10,4}, {5,15,10,4}, {3,13,10,4}, {-2,8,10,4}, {-5,5,10,4}},
	{{15,15,10,4}, {5,15,10,4}, {3,13,10,4}, {-2,8,10,4}, {-5,5,10,4}}
};

struct btc_fddt_cell cell_dl_def[5][5] = { /* BT-RSSI 0~4, WL-RSSI 0~4 */
	{{0,0,0,0},    {0,0,0,0},    {0,0,0,0},   {0,0,0,0},   {-5,5,15,6}},
	{{10,15,20,5}, {10,15,20,5}, {5,15,20,5}, {0,10,20,5}, {-5,5,20,5}},
	{{10,15,25,5}, {10,15,25,5}, {5,15,25,5}, {0,10,25,5}, {-5,5,25,5}},
	{{10,15,30,4}, {10,15,30,4}, {5,15,30,5}, {0,10,30,5}, {-5,5,30,5}},
	{{10,15,40,4}, {10,15,40,4}, {5,15,40,4}, {0,10,40,5}, {-5,5,40,5}}
};

void _set_bt_ignore_wl_act(struct btc_t *btc, u8 enable)
{
	u8 buf = 0;

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s(): %s wlan_act\n",
		  __func__, (enable? "ignore" : "do not ignore"));

	buf = enable;
	_send_fw_cmd(btc, SET_BT_IGNORE_WLAN_ACT, &buf, 1);
}

void _set_wl_tx_power(struct btc_t *btc, u32 level)
{
	struct btc_wl_info *wl = &btc->cx.wl;

	if (wl->rf_para.tx_pwr_freerun == level || wl->status.map.dbccing)
		return;

	wl->rf_para.tx_pwr_freerun = level;
	btc->dm.rf_trx_para.wl_tx_power = level;
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s(): level = %d\n",
		  __func__, level);

#ifdef BTC_WL_TX_PWR_CTRL_OFLD
	hal_btc_fw_set_drv_info(btc, CXDRVINFO_TXPWR);
#else
	btc->chip->ops->wl_tx_power(btc, level);
#endif
}

void _set_wl_rx_gain(struct btc_t *btc, u32 level)
{
	struct btc_wl_info *wl = &btc->cx.wl;

	if (wl->rf_para.rx_gain_freerun == level)
		return;

	wl->rf_para.rx_gain_freerun = level;
	btc->dm.rf_trx_para.wl_rx_gain = level;
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s(): level = %d\n",
		  __func__, level);

	btc->chip->ops->wl_rx_gain(btc, level);
}

void _set_bt_tx_power(struct btc_t *btc, u32 level)
{
	struct btc_bt_info *bt = &btc->cx.bt;
	u8 buf = 0;

	if (btc->cx.cnt_bt[BTC_BCNT_INFOUPDATE] == 0)
		return;

	if (bt->rf_para.tx_pwr_freerun == level)
		return;

	buf = level & bMASKB0;

	if (_send_fw_cmd(btc, SET_BT_TX_PWR, &buf, 1)) {
		bt->rf_para.tx_pwr_freerun = level;
		btc->dm.rf_trx_para.bt_tx_power = level;
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s(): level = %d\n",
			  __func__, level);
	}
}

void _set_bt_rx_gain(struct btc_t *btc, u32 level)
{
	struct btc_bt_info *bt = &btc->cx.bt;
	u8 buf = 0;

	if (btc->cx.cnt_bt[BTC_BCNT_INFOUPDATE] == 0)
		return;

	if ((bt->rf_para.rx_gain_freerun == level ||
	     level > BTC_BT_RX_NORMAL_LVL) &&
	     (!btc->chip->scbd || bt->lna_constrain == level))
		return;

	buf = level & bMASKB0;

	if (!_send_fw_cmd(btc, SET_BT_LNA_CONSTRAIN, &buf, 1)) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
			 "[BTC], %s(): setup fail!!\n", __func__);
		return;
	}

	bt->rf_para.rx_gain_freerun = level;
	btc->dm.rf_trx_para.bt_rx_gain = level;
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s(): level = %d\n",
		  __func__, level);

	_os_delay_us(btc->hal, BTC_SCBD_REWRITE_DELAY * 2);

	if (buf == BTC_BT_RX_NORMAL_LVL)
		_write_scbd(btc, BTC_WSCB_RXGAIN, false);
	else
		_write_scbd(btc, BTC_WSCB_RXGAIN, true);
}

static void _set_rf_trx_para(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_wl_smap *wl_smap = &wl->status.map;
	struct btc_bt_link_info *b = &bt->link_info;
	struct btc_rf_trx_para para;
	u32 level_id = 0;
	u32 wl_stb_chg = 0;

	/* decide trx_para_level */
	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) {
		/* fix LNA2 + TIA gain not change by GNT_BT */
		if ((btc->dm.wl_btg_rx &&
		     b->profile_cnt.now != 0) || dm->bt_only == 1)
			dm->trx_para_level = 1; /* for better BT ACI issue */
		else
			dm->trx_para_level = 0;
	} else { /* non-shared antenna  */
		dm->trx_para_level = 5;
		if (wl_rinfo->link_mode == BTC_WLINK_2G_STA &&
		    b->leaudio_desc.bis_cnt)
			dm->trx_para_level = 8;
		/* modify trx_para if WK 2.4G-STA-DL + bt link */
		if (dm->freerun && b->profile_cnt.now != 0 &&
		    wl_rinfo->link_mode == BTC_WLINK_2G_STA &&
		    wl_smap->traffic_dir & BIT(TRAFFIC_UL)) { /* uplink */
			if (wl->rssi_level == 4 && bt->rssi_level > 2)
				dm->trx_para_level = 6;
			else if (wl->rssi_level == 3 && bt->rssi_level > 3)
				dm->trx_para_level = 7;
		}
	}

	level_id = dm->trx_para_level;

	if (level_id >= btc->chip->rf_para_dlink_num ||
	    level_id >= btc->chip->rf_para_ulink_num) {
	    PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
	    	      "[BTC], %s(): invalid level_id:%d\n", __func__, level_id);
	    return;
	}

	if (wl_smap->traffic_dir & BIT(TRAFFIC_UL))
		para = btc->chip->rf_para_ulink[level_id];
	else
		para = btc->chip->rf_para_dlink[level_id];

	if (dm->fddt_train) {
		_set_wl_rx_gain(btc, 1);
		_write_scbd(btc, BTC_WSCB_RXGAIN, true);
	} else {
		_set_wl_tx_power(btc, para.wl_tx_power);
		_set_wl_rx_gain(btc, para.wl_rx_gain);
		_set_bt_tx_power(btc, para.bt_tx_power);
		_set_bt_rx_gain(btc, para.bt_rx_gain);
	}

	if (!bt->enable.now || dm->wl_only || wl_smap->rf_off ||
	    wl_smap->lps == BTC_LPS_RF_OFF ||
	    wl_rinfo->link_mode == BTC_WLINK_5G ||
	    wl_rinfo->link_mode == BTC_WLINK_NOLINK ||
	    (wl_rinfo->dbcc_en && wl_rinfo->dbcc_2g_phy != HW_PHY_1))
	    wl_stb_chg = 0;
	else
	    wl_stb_chg = 1;

	if (wl_stb_chg != dm->wl_stb_chg) {
		dm->wl_stb_chg = wl_stb_chg;
		btc->chip->ops->wl_btg_standby(btc, dm->wl_stb_chg);
	}
}

static void _set_wl_ch_map(struct btc_t *btc)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_wl_info *wl = &btc->cx.wl;
	u16 ch_start = 0, ch_end = 0, i;
	u8 b_group, b_pos; /* byte-group, bit-position */

	hal_mem_set(h, wl->ch_map, 0x0, sizeof(wl->ch_map));
	hal_mem_set(h, wl->ch_map_le, 0x0, sizeof(wl->ch_map_le));

	if (!wl->afh_info.en || wl->afh_info.ch > 13)
		return;

	ch_start = wl->afh_info.ch * 5 + 2407 - wl->afh_info.bw/2;
	ch_end = ch_start + wl->afh_info.bw;

	for (i = ch_start; i < ch_end; i++) {
		if (i < 2402 || i > 2480)
			continue;
		b_group = (i - 2402) / 8;
		b_pos = 7 - ((i - 2402) % 8);
		wl->ch_map[b_group] |= BIT(b_pos);

		if (i % 2)  /* BLE ch = even-freq (ex:2402,2404) */
			continue;

		b_group = (i - 2402) / 16;
		b_pos = 7 - (((i - 2402)/2) % 8);
		wl->ch_map_le[b_group] |= BIT(b_pos);
	}
}

static void _set_bt_afh_info(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *b = &bt->link_info;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_wl_afh_info *wl_afh = &wl->afh_info;
	struct btc_wl_active_role *act_role = NULL;
	struct btc_wl_smap *wl_smap = &wl->status.map;
	/*struct btc_module *module = &btc->mdinfo;*/
	u8 en = 0, i, ch = 0, bw = CHANNEL_WIDTH_MAX;
	u8 buf[3] = {0};

	if (btc->ctrl.manual || wl->status.map.scan)
		return;

	/* TBD if include module->ant.type == BTC_ANT_SHARED */
	if (wl->status.map.rf_off || bt->whql_test ||
	    wl_rinfo->link_mode == BTC_WLINK_NOLINK ||
	    wl_rinfo->link_mode == BTC_WLINK_5G) {
		en = false;
		ch = 0;
		bw = 0;
	} else {
		en = true;
		for (i = 0; i < BTC_WL_MAX_ROLE_NUMBER; i++) {
			act_role = &wl_rinfo->active_role[i];

			/* not care no-connected/non-2G-band role */
			if (!act_role->connected ||
			    act_role->band != BAND_ON_24G)
				continue;

			ch = act_role->ch;
			bw = act_role->bw;

			if (wl_rinfo->link_mode == BTC_WLINK_2G_MCC &&
			    (act_role->role == PHL_RTYPE_AP ||
			     act_role->role == PHL_RTYPE_P2P_GO ||
			     act_role->role == PHL_RTYPE_P2P_GC)) {
			     /* for 2.4G MCC, take role = ap/go/gc*/
				break;
			} else if (wl_rinfo->link_mode != BTC_WLINK_2G_SCC ||
				act_role->bw == CHANNEL_WIDTH_40) {
				/* for 2.4G scc, take bw = 40M  */
				break;
			}
		}
	}

	if (!en || ch > 14 || ch == 0)
		bw = CHANNEL_WIDTH_MAX;

	/* default AFH channel sapn = center-ch +- 6MHz  */
	switch (bw) {
	case CHANNEL_WIDTH_20:
		if (!wl_smap->busy && btc->mdinfo.ant.num == 3)
			bw = 10;
		else if (btc->dm.freerun || btc->dm.fddt_train)
			bw = 48;
		else
			bw = 20 + btc->chip->afh_guard_ch * 2;
		break;
	case CHANNEL_WIDTH_40:
		if (!wl_smap->busy && btc->mdinfo.ant.num == 3)
			bw = 10;
		else if (btc->dm.freerun || btc->dm.fddt_train)
			bw = 40 + btc->chip->afh_guard_ch * 2;
		else
			bw = 40;
		break;
	case CHANNEL_WIDTH_5:
		bw = 5 + btc->chip->afh_guard_ch * 2;
		break;
	case CHANNEL_WIDTH_10:
		bw = 10 + btc->chip->afh_guard_ch * 2;
		break;
	default:
	case CHANNEL_WIDTH_MAX:
		en = false; /* turn off AFH info if invalid BW */
		bw = 0;
		ch = 0;
		break;
	}

	if (wl_afh->en == en &&
	    wl_afh->ch == ch &&
	    wl_afh->bw == bw &&
	    b->profile_cnt.last == b->profile_cnt.now)
		return;

	buf[0] = en;
	buf[1] = ch;
	buf[2] = bw;

	if (_send_fw_cmd(btc, SET_BT_WL_CH_INFO, buf, 3)) {
		wl_afh->en = buf[0];
		wl_afh->ch = buf[1];
		wl_afh->bw = buf[2];
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
			  "[BTC], %s(): en=%d, ch=%d, bw=%d\n",
			  __func__, en, ch, bw);

		_set_wl_ch_map(btc);
		btc->cx.cnt_wl[BTC_WCNT_CH_UPDATE]++;
	}
}

static void _set_halbb_preagc_ctrl(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_chip_ops *ops = btc->chip->ops;
	struct btc_bt_link_info *bt_linfo = &btc->cx.bt.link_info;
	u8 is_preagc = 0, val = 0;

	if (btc->ctrl.manual)
		return;

	/* notify halbb ignore GNT_BT or not for WL BB Rx-AGC control */
	if (wl_rinfo->link_mode == BTC_WLINK_25G_MCC)
		is_preagc = 2; /* bb switch in WL FW by itself */
	else if (!(bt->run_patch_code && bt->enable.now))
		is_preagc = 0;
	else if (wl_rinfo->link_mode == BTC_WLINK_5G) /* always 0 if 5G */
		is_preagc = 0;
	else if (wl_rinfo->link_mode == BTC_WLINK_NOLINK ||
		 btc->cx.bt.link_info.profile_cnt.now == 0)
		is_preagc = 0;
	else if (dm->tdma_now.type != CXTDMA_OFF &&
		 !bt_linfo->hfp_desc.exist &&
		 !bt_linfo->hid_desc.exist &&
		 dm->fddt_train == BTC_FDDT_DISABLE)
		is_preagc = 0;
	else if (wl_rinfo->dbcc_en && wl_rinfo->dbcc_2g_phy != HW_PHY_1)
		is_preagc = 0;
	else if (btc->mdinfo.ant.type == BTC_ANT_SHARED)
		is_preagc = 0;
	else
		is_preagc = 1;

	/* check pre-agc only if not-sync between wl_pre_agc_rb and wl_pre_agc*/
	if (dm->wl_pre_agc_rb != dm->wl_pre_agc &&
	    dm->wl_pre_agc_rb != BTC_BB_PRE_AGC_NOTFOUND &&
	    (ops && ops->get_reg_status)) {
		ops->get_reg_status(btc, BTC_CSTATUS_BB_PRE_AGC, (void*)&val);
		dm->wl_pre_agc_rb = val;
	}

	if ((wl->coex_mode == BTC_MODE_NORMAL &&
	    (run_rsn("_ntfy_init_coex") ||
	    run_rsn("_ntfy_switch_band") ||
	    dm->wl_pre_agc_rb != dm->wl_pre_agc)) ||
	    is_preagc != dm->wl_pre_agc) {

		dm->wl_pre_agc = is_preagc;

		if (is_preagc > 1)
			return;
		rtw_hal_bb_ctrl_btc_preagc(btc->hal, dm->wl_pre_agc);
	}
}

static void _set_halbb_btg_ctrl(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_chip_ops *ops = btc->chip->ops;
	u32 is_btg = 0, val = 0;

	/* rtw_hal_bb_ctrl_btg() control bb internal GNT_MUX:
	 * if btc->dm.wl_btg_rx = 0 (gnt_wl always = 1, gnt_bt always = 0)
	 * Lte_rx:    0x10980[17]=1, 0x10980[29]=0
	 * Gnt_wl:    0x10980[18]=1, 0x10980[28]=1
	 * Gnt_bt_tx: 0x10980[19]=1, 0x10980[27]=0
	 * Gnt_bt:    0x10980[20]=1, 0x10980[26]=0   0x101e2250
	 *
	 * if btc->dm.wl_btg_rx = 1 (gnt from MAC)
	 * Lte_rx:    0x10980[17]=0, 0x10980[29]=0
	 * Gnt_wl:    0x10980[18]=0, 0x10980[28]=1
	 * Gnt_bt_tx: 0x10980[19]=0, 0x10980[27]=0
	 * Gnt_bt:    0x10980[20]=0, 0x10980[26]=0   0x10002250
	 */

	if (btc->ctrl.manual)
		return;

	/* notify halbb ignore GNT_BT or not for WL BB Rx-AGC control */
	if (wl_rinfo->link_mode == BTC_WLINK_25G_MCC)
		is_btg = 2; /* bb call ctrl_btg() in WL FW by slot */
	else if (!(bt->run_patch_code && bt->enable.now))
		is_btg = 0;
	else if (wl_rinfo->link_mode == BTC_WLINK_5G) /* always 0 if 5G */
		is_btg = 0;
	else if (dm->freerun)
		is_btg = 0;
	else if (wl_rinfo->dbcc_en && wl_rinfo->dbcc_2g_phy != HW_PHY_1)
		is_btg = 0;
	else if (btc->mdinfo.ant.type == BTC_ANT_DEDICATED)
		is_btg = 0;
	else
		is_btg = 1;

	/* check 0x10980 only if not-sync between wl_btg_rx_rb and wl_btg_rx */
	if (dm->wl_btg_rx_rb != dm->wl_btg_rx &&
	    dm->wl_btg_rx_rb != BTC_BB_GNT_NOTFOUND &&
	    (ops && ops->get_reg_status)) {
		ops->get_reg_status(btc, BTC_CSTATUS_BB_GNT_MUX, (void*)&val);
		dm->wl_btg_rx_rb = val;
	}

	/* always run --> _ntfy_init_coex, _ntfy_switch_band, reg-RB no match */
	if ((wl->coex_mode == BTC_MODE_NORMAL &&
	    (run_rsn("_ntfy_init_coex") ||
	    run_rsn("_ntfy_switch_band") ||
	    dm->wl_btg_rx_rb != dm->wl_btg_rx)) ||
	    is_btg != dm->wl_btg_rx) {

		dm->wl_btg_rx = is_btg;

		/* skip setup if btg_ctrl set by wl fw */
		if (is_btg > 1)
			return;

		rtw_hal_bb_ctrl_btg(btc->hal, (bool)(!!is_btg));
	}
}

static void _set_phl_bt_slot_req(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_link_info *b = &btc->cx.bt.link_info;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_bt_a2dp_desc a2dp = b->a2dp_desc;
	struct btc_dm *dm = &btc->dm;
	u8 len = 0;

	/*  don't change bt slot req state during RFK for p2p/mcc case*/
	if (run_rsn("_ntfy_wl_rfk"))
		return;

	/* enable bt-slot req if ext-slot-control  */
	if (btc->dm.tdma_now.type == CXTDMA_OFF &&
	    btc->dm.tdma_now.ext_ctrl == CXECTL_EXT) {
		if (wl->status.val & BTC_WL_LINK_SCAN)
			btc->bt_req_en = false;
		else
			btc->bt_req_en = true;
	} else {
		btc->bt_req_en = false;
	}

	if (!btc->bt_req_en) {
		len = 0;
	} else if (a2dp.exist || a2dp.active ||
		   (b->profile_cnt.now == 1 && b->hid_desc.pair_cnt == 1 &&
		    b->multi_link.now)) {
		/*a2dp, hid + acl-idle  */
		if (b->profile_cnt.now >= 2 || a2dp.vendor_id == 0x4c ||
		    dm->slot_req_more == 1)
			len = BTC_BSLOT_A2DP_HID;
		else if (wl_rinfo->p2p_2g == 1)
			len = BTC_BSLOT_A2DP_2;
		else
			len = BTC_BSLOT_A2DP;
	} else if (b->pan_desc.exist || b->status.map.inq_pag) {
		len = BTC_BSLOT_INQ;
	} else {
		len = BTC_BSLOT_IDLE;
	}

	if (len == btc->bt_req_len[wl->pta_req_mac] && !wl->pta_reg_mac_chg)
		return;

	btc->bt_req_len[wl->pta_req_mac] = len;
	if (wl->pta_req_mac == HW_PHY_0)
		btc->bt_req_len[HW_PHY_1] = 0;
	else
		btc->bt_req_len[HW_PHY_0] = 0;

	_send_phl_evnt(btc, (u8*)&btc->bt_req_len[wl->pta_req_mac], 4,
		       BTC_HMSG_SET_BT_REQ_SLOT);

	PHL_INFO("[BTC], %s(): bt_req_len= PHY0:%d, PHY1:%d\n", __func__,
		 btc->bt_req_len[HW_PHY_0], btc->bt_req_len[HW_PHY_1]);
}

static void _set_phl_stbc_req(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	u8 req_stbc = 1;

	if (btc->dm.freerun || btc->dm.wl_only || wl->role_info.dbcc_en)
		req_stbc = 0;

	if (req_stbc == btc->bt_req_stbc && !wl->pta_reg_mac_chg)
		return;

	btc->bt_req_stbc = req_stbc;
	_send_phl_evnt(btc, (u8*)&btc->bt_req_stbc, 1,
		       BTC_HMSG_SET_BT_REQ_STBC);

	PHL_INFO("[BTC], %s(): bt_req_stbc=%d\n", __func__, btc->bt_req_stbc);
}

static void _set_bt_rx_agc(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_bt_info *bt = &btc->cx.bt;
	bool bt_hi_lna_rx = false;

	if (wl_rinfo->dbcc_en && wl_rinfo->dbcc_2g_phy != HW_PHY_1)
		bt_hi_lna_rx = false;
	else if (wl_rinfo->link_mode != BTC_WLINK_NOLINK &&
		 btc->dm.wl_btg_rx && btc->mdinfo.ant.type == BTC_ANT_SHARED)
		bt_hi_lna_rx = true;

	if (bt_hi_lna_rx == bt->hi_lna_rx)
		return;

	/* true: bt use Hi-LNA rx gain table (f/e/3/2) in -3x~-9xdBm for co-rx
	 * false: bt use original rx gain table (f/b/7/3/2)
	 */

	_write_scbd(btc, BTC_WSCB_BT_HILNA, bt_hi_lna_rx);
}

static void _set_halmac_tx_limit(struct btc_t *btc)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_dm *dm = &btc->dm;
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_bt_link_info *b = &bt->link_info;
	struct btc_bt_hfp_desc *hfp = &b->hfp_desc;
	struct btc_bt_hid_desc *hid = &b->hid_desc;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_wl_link_info *plink = NULL;
	u8 tx_1ss_limit = 0;
	u8 mode = wl_rinfo->link_mode, i;
	u8 enable = 0, id = 0, resume = 0;
	u32 tx_time = BTC_MAX_TX_TIME_DEF;

	if (btc->ctrl.manual || wl->status.map.dbccing)
		return;

	if (btc->dm.freerun || btc->ctrl.igno_bt || b->profile_cnt.now == 0 ||
	    mode == BTC_WLINK_5G || mode == BTC_WLINK_NOLINK ||
	    dm->wl_trx_nss.tx_limit) {
		enable = 0;
		tx_time = BTC_MAX_TX_TIME_DEF;
		tx_1ss_limit = 0;
	} else if (hfp->exist || hid->exist) {
		enable = 1;
		tx_time = BTC_MAX_TX_TIME_L3;
	}

	if (dm->wl_tx_limit.en == enable &&
	    dm->wl_tx_limit.tx_time == tx_time &&
	    !wl_rinfo->link_mode_chg)
		return;

	resume = !enable;

	for (i = 0; i < BTC_WL_MAX_ROLE_NUMBER; i++) {
		plink = &wl->link_info[i];
		id = (u8)plink->mac_id;

		if (!plink->connected)
			continue;

		/* Skip non-2G-PHY setup for dbcc case */
		if (wl_rinfo->dbcc_en &&
		    (plink->phy != (u8)wl_rinfo->dbcc_2g_phy))
			continue;

		/* backup the original tx-limit-setup if tx-limit off */
		if (!dm->wl_tx_limit.en)
			rtw_hal_mac_get_tx_time(h, id, &plink->tx_time);

		/* restore the original tx-limit-setup if no tx-limit */
		if (resume)
			tx_time = plink->tx_time;

		rtw_hal_mac_set_tx_time(h, 1, resume, id, tx_time);
	}

	dm->wl_tx_limit.en = enable;
	dm->wl_tx_limit.tx_time = tx_time;
}

static void _set_trx_nss(struct btc_t *btc)
{
	struct btc_wl_trx_nss_para *trx_nss = &btc->dm.wl_trx_nss;
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_bt_link_info *b = &bt->link_info;
	struct btc_bt_hfp_desc *hfp = &b->hfp_desc;
	struct btc_bt_hid_desc *hid = &b->hid_desc;
	struct btc_bt_a2dp_desc *a2dp = &b->a2dp_desc;
	struct btc_bt_leaudio_desc *le_audio = &b->leaudio_desc;
	struct btc_chip_ops *ops = btc->chip->ops;
	u8 tx_en = 0, tx_re = 0, rx_en = 0, rx_re = 0;
	u8 tx_limit = 0, rx_limit = 0;
	u8 tx_ss, rx_ss, tx_path, rx_path;
	u8 tx_ss_ori, rx_ss_ori, tx_path_ori, rx_path_ori;

	/* tx/rx_limit = 1 ----> 1ss
	 * tx/rx_limit = 0 ----> default
	 */

	if (btc->ctrl.manual ||
	    wl->status.map.dbccing ||
	    !dm->wl_trx_nss_en)
		return;

	if (wl_rinfo->link_mode == BTC_WLINK_NOLINK ||
	    wl_rinfo->link_mode == BTC_WLINK_5G ||
	    wl_rinfo->dbcc_en) {
		tx_limit = 0;
		rx_limit = 0;
	} else if (bt->hiduty_dev) {
		tx_limit = 1;
		rx_limit = 1;
	} else if (le_audio->bis_cnt) {
		tx_limit = 1;
		rx_limit = 1;
	} else if (le_audio->cis_cnt &&
	           (btc->mdinfo.wa_type & BTC_WA_CO_RX)) {
		if (wl->he_mode) {
			tx_limit = 1;
			rx_limit = 1;
		}
	} else if (a2dp->exist && a2dp->sink) {
		tx_limit = 1;
		rx_limit = 1;
	} else if (wl_rinfo->link_mode == BTC_WLINK_2G_AP ||
		   wl_rinfo->link_mode == BTC_WLINK_2G_GO) {
		if (b->profile_cnt.now) {
			tx_limit = 1;
			rx_limit = 1;
		}
	} else if (hid->exist || hfp->exist) {
		if (wl_rinfo->p2p_2g) {
			tx_limit = 1;
			rx_limit = 0;
		}
	}

	if (trx_nss->tx_limit == tx_limit &&
	    trx_nss->rx_limit == rx_limit &&
	    !wl->client_cnt_inc_2g)
		return;

	tx_ss_ori = (btc->mdinfo.ant.stream_cnt & 0xf0) >> 4;
	rx_ss_ori = btc->mdinfo.ant.stream_cnt & 0xf;
	tx_path_ori = (btc->mdinfo.ant.path_pos & 0xf0) >> 4;
	rx_path_ori = btc->mdinfo.ant.path_pos & 0xf;

	if (tx_ss_ori == 2 && tx_limit) {
		tx_en = 1;
		tx_re = 0;
		tx_ss = 1;
		tx_path = RF_PATH_A;
	} else {
		tx_en = 0;
		tx_re = 1;
		tx_ss = tx_ss_ori;
		tx_path = tx_path_ori;
	}

	if (rx_ss_ori == 2 && rx_limit) {
		rx_en = 1;
		rx_re = 0;
		rx_ss = 1;
		rx_path = RF_PATH_A;
	} else {
		rx_en = 0;
		rx_re = 1;
		rx_ss = rx_ss_ori;
		rx_path = rx_path_ori;
	}

	rtw_hal_btc_cfg_1ss(btc->hal, btc->phl, BAND_ON_24G,
			    tx_en, rx_en, tx_re, rx_re);
	rtw_hal_btc_cfg_trx_path(btc->hal, tx_path, tx_ss, rx_path, rx_ss);

	trx_nss->tx_limit = tx_limit;
	trx_nss->rx_limit = rx_limit;
	trx_nss->tx_ss = tx_ss;
	trx_nss->rx_ss = rx_ss;
	trx_nss->tx_path = tx_path;
	trx_nss->rx_path = rx_path;

	/* if enable MIMO-PS (to 1T1R), change ant shared-type */
	if (dm->wl_trx_nss.tx_limit &&
	    dm->wl_trx_nss.rx_limit)
		btc->mdinfo.ant.type = BTC_ANT_DEDICATED;
	else
		btc->mdinfo.ant.type = BTC_ANT_SHARED;

	if (ops && ops->init_cfg)
		ops->init_cfg(btc);
}

void _set_fddt_ctrl(struct btc_t *btc, bool force_exec)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_fddt_train_info *train = &dm->fddt_info.train;
	struct btc_fddt_train_info *train_now = &dm->fddt_info.train_now;

	if (!force_exec && !_fddt_cmp(train, train_now)) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
			  "[BTC], %s(): fddt control no change!\n", __func__);
		return;
	}

	hal_btc_fw_set_drv_info(btc, CXDRVINFO_FDDT);

	_fddt_cpy(train_now, train);
}

static void _set_fddt_status(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	bool is_stop = false;

	if (dm->fddt_train == BTC_FDDT_DISABLE) {
		if ((dm->fddt_info.nrsn_map & BIT(BTC_NFRSN_FORCE_STOP)) ||
		     dm->fddt_info.result == BTC_FDDT_RESULT_FAIL)
		     is_stop = true;
	}

	/* fddt_train state change */
	switch (dm->fddt_train) {
	case BTC_FDDT_DISABLE: /* fddt_train enable->disable */
		if (is_stop)
			dm->fddt_info.state = BTC_FDDT_STATE_STOP;
		else
			dm->fddt_info.state = BTC_FDDT_STATE_PENDING;
		break;
	case BTC_FDDT_ENABLE:  /* fddt_train disable->enable */
		if (dm->fddt_info.state == BTC_FDDT_STATE_STOP)
			dm->cnt_dm[BTC_DCNT_FDDT_TRIG]++;

		if (dm->fddt_info.type == BTC_FDDT_TYPE_AUTO)
			dm->fddt_info.state = BTC_FDDT_STATE_RUN;
		else
			dm->fddt_info.state = BTC_FDDT_STATE_DEBUG;
		break;
	}
}

static void _set_policy(struct btc_t *btc, u16 policy_type, const char* action)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct fbtc_tdma *t = &dm->tdma;
	struct fbtc_slot *s = dm->slot;
	struct btc_wl_role_info *wl_rinfo = &btc->cx.wl.role_info;
	struct btc_bt_hid_desc *hid = &btc->cx.bt.link_info.hid_desc;
	struct btc_bt_hfp_desc *hfp = &btc->cx.bt.link_info.hfp_desc;
	struct btc_bt_a2dp_desc *a2dp = &btc->cx.bt.link_info.a2dp_desc;
	struct rtw_phl_com_t *p = btc->phl;
	u8 type, null_role = 0;
	u16 dur_1 = 0, dur_2 = 0;
	u32 tbl_w1, tbl_b1, tbl_b4;
	bool mode = NM_EXEC;

	type = (u8)((policy_type & bMASKB1) >> 8);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) {
		if (wl->status.map._4way)
			tbl_w1 = cxtbl[1];
		else if (hid->exist && hid->type == BTC_HID_218)
			tbl_w1 = cxtbl[7]; /* Ack/BA no break bt Hi-Pri-rx */
		else
			tbl_w1 = cxtbl[8];

		if (dm->leak_ap &&
		    (type == BTC_CXP_PFIX || type == BTC_CXP_PAUTO2)) {
			tbl_b1 = cxtbl[3];
			tbl_b4 = cxtbl[3];
		} else if (hid->exist && hid->type == BTC_HID_218) {
			tbl_b1 = cxtbl[4]; /* Ack/BA no break bt Hi-Pri-rx */
			tbl_b4 = cxtbl[4];
		} else {
			tbl_b1 = cxtbl[2];
			tbl_b4 = cxtbl[2];
		}
	} else {
		if (wl->legacy_mode)
			tbl_w1 = cxtbl[8];
		else if ((wl->status.map.traffic_dir & BIT(TRAFFIC_UL)) &&
			 hid->exist)
			tbl_w1 = cxtbl[19];
		else
			tbl_w1 = cxtbl[16];
		tbl_b1 = cxtbl[17];
		tbl_b4 = cxtbl[17];
	}

	switch(type) {
	case BTC_CXP_USERDEF0:
		mode = FC_EXEC;
		_tdma_cpy(t, &t_def[CXTD_OFF]);
		_slot_cpy(&s[CXST_OFF], &s_def[CXST_OFF]);
		_slot_set_tbl(CXST_OFF, cxtbl[2]);
		break;
	case BTC_CXP_OFF: /* TDMA off */
		_write_scbd(btc, BTC_WSCB_TDMA, false);
		_tdma_cpy(t, &t_def[CXTD_OFF]);
		_slot_cpy(&s[CXST_OFF], &s_def[CXST_OFF]);

		switch (policy_type) {
		case BTC_CXP_OFF_BT:
			_slot_set_tbl(CXST_OFF, cxtbl[2]);
			break;
		case BTC_CXP_OFF_WL:
			_slot_set_tbl(CXST_OFF, cxtbl[1]);
			break;
		case BTC_CXP_OFF_WL2:
			_slot_set_tbl(CXST_OFF, cxtbl[1]);
			_slot_set_type(CXST_OFF, SLOT_ISO);
			break;
		case BTC_CXP_OFF_EQ0:
			_slot_set_tbl(CXST_OFF, cxtbl[0]);
			_slot_set_type(CXST_OFF, SLOT_ISO);
			break;
		case BTC_CXP_OFF_EQ1:
			_slot_set_tbl(CXST_OFF, cxtbl[16]);
			break;
		case BTC_CXP_OFF_EQ2:
			_slot_set_tbl(CXST_OFF, cxtbl[0]);
			break;
		case BTC_CXP_OFF_EQ3:
			_slot_set_tbl(CXST_OFF, cxtbl[24]);
			break;
		case BTC_CXP_OFF_EQ4:
			_slot_set_tbl(CXST_OFF, cxtbl[26]);
			break;
		case BTC_CXP_OFF_EQ5:
			_slot_set_tbl(CXST_OFF, cxtbl[27]);
			break;
		case BTC_CXP_OFF_BWB0:
			_slot_set_tbl(CXST_OFF, cxtbl[5]);
			break;
		case BTC_CXP_OFF_BWB1:
			_slot_set_tbl(CXST_OFF, cxtbl[8]);
			break;
		case BTC_CXP_OFF_BWB2:
			_slot_set_tbl(CXST_OFF, cxtbl[7]);
			break;
		case BTC_CXP_OFF_BWB3:
			_slot_set_tbl(CXST_OFF, cxtbl[6]);
			break;
		}
		break;
	case BTC_CXP_OFFB: /* TDMA off + beacon protect */
		_write_scbd(btc, BTC_WSCB_TDMA, false);
		_tdma_cpy(t, &t_def[CXTD_OFF_B2]);
		_slot_cpy(&s[CXST_OFF], &s_def[CXST_OFF]);

		switch (policy_type) {
		case BTC_CXP_OFFB_BWB0:
			_slot_set_tbl(CXST_OFF, cxtbl[8]);
			break;
		}
		break;
	case BTC_CXP_OFFE: /* TDMA off + beacon protect + Ext_control */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_OFF_EXT]);

		/* To avoid wl-s0 tx break by hid/hfp tx */
		if (btc->cx.bt.hiduty_dev || hid->exist || hfp->exist) {
			if (dm->wl_trx_nss.tx_limit)
				tbl_w1 = cxtbl[16];
			else if (btc->mdinfo.ant.type == BTC_ANT_SHARED &&
			    (wl_rinfo->role_map & BIT(PHL_RTYPE_P2P_GO)))
				tbl_w1 = cxtbl[22];
			else if (btc->mdinfo.ant.type == BTC_ANT_SHARED &&
			    p->phy_cap[0].rx_path_num == 1 &&
			    btc->mdinfo.ant.type == BTC_ANT_SHARED &&
			    (wl_rinfo->role_map & BIT(PHL_RTYPE_P2P_GC)))
				tbl_w1 = cxtbl[23];
			else
				tbl_w1 = cxtbl[16];
		}
		if (wl_rinfo->link_mode == BTC_WLINK_2G_SCC &&
		    dm->wl_scc.e2g_null) {
			_tdma_set_rxflctrl(1);
			_tdma_set_txflctrl(1);
			/* Set Null-Tx time-tick by E2G duration */
			dur_1 = (u16)dm->e2g_slot_nulltx_time;
		}

		/* To avoid A2DP pop by the long E2G duration isolated
		 * Set E2G max-limit by EBT duration
		 */
		dur_2 = (u16)dm->e2g_slot_limit;

		switch (policy_type) {
		case BTC_CXP_OFFE_2GBWISOB: /* for normal-case */
			_slot_set(CXST_E2G, dur_1, tbl_w1, SLOT_ISO);
			_slot_cpy(&s[CXST_EBT], &s_def[CXST_EBT]);
			_slot_set_dur(CXST_EBT, dur_2);
			break;
		case BTC_CXP_OFFE_2GISOB: /* for bt no-link */
			_slot_set(CXST_E2G, dur_1, cxtbl[1], SLOT_ISO);
			_slot_cpy(&s[CXST_EBT], &s_def[CXST_EBT]);
			_slot_set_dur(CXST_EBT, dur_2);
			break;
		case BTC_CXP_OFFE_2GBWMIXB: /* for GC-only NOA < bt_slot_req */
			_slot_set(CXST_E2G, dur_1, tbl_w1, SLOT_MIX);
			_slot_cpy(&s[CXST_EBT], &s_def[CXST_EBT]);
			break;
		case BTC_CXP_OFFE_2GBWMIXB2: /* for GC+STA NOA < bt_slot_req */
			_slot_set(CXST_E2G, dur_1, tbl_w1, SLOT_MIX);
			_slot_cpy(&s[CXST_EBT], &s_def[CXST_EBT]);
			break;
		case BTC_CXP_OFFE_WL: /*for 4-way  */
			_slot_set(CXST_E2G, dur_1, cxtbl[1], SLOT_MIX);
			_slot_set(CXST_EBT, 0, cxtbl[1], SLOT_MIX);
			break;
		}

		_slot_cpy(&s[CXST_E5G], &s_def[CXST_E5G]);
		/* for p2p-on transient time */
		_slot_cpy(&s[CXST_OFF], &s_def[CXST_OFF]);
		break;
	case BTC_CXP_FIX: /* TDMA Fix-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_FIX]);

		if (dm->fddt_train) {
			tbl_w1 = cxtbl[0];
			tbl_b1 = cxtbl[0];
		}

		switch (policy_type) {
		case BTC_CXP_FIX_TD3030: /* W1:B1 = 30:30 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD5050: /* W1:B1 = 50:50 */
			_slot_set(CXST_W1, 50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 50, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD2030: /* W1:B1 = 20:30 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD4010: /* W1:B1 = 40:10 */
			_slot_set(CXST_W1, 40, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 10, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD4010ISO: /* W1:B1 = 40:10 for WL busy/scan */
			_slot_set(CXST_W1, 40, cxtbl[1], SLOT_ISO);
			_slot_set(CXST_B1, 10, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD4010ISO_DL: /* W1:B1 = 40:10 for WL DL/scan*/
			_slot_set(CXST_W1, 40, cxtbl[25], SLOT_ISO);
			_slot_set(CXST_B1, 10, cxtbl[25], SLOT_ISO);
			break;
		case BTC_CXP_FIX_TD4010ISO_UL: /* W1:B1 = 40:10 for WL UL/scan*/
			_slot_set(CXST_W1, 40, cxtbl[20], SLOT_ISO);
			_slot_set(CXST_B1, 10, cxtbl[25], SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD7010: /* W1:B1 = 70:10 */
			_slot_set(CXST_W1, 70, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 10, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD2060: /* W1:B1 = 20:60 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD3060: /* W1:B1 = 30:60 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD2080: /* W1:B1 = 20:80 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 80, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TDW1B1: /* W1:B1 = user-define */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_PFIX: /* PS-TDMA Fix-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_PFIX]);

		switch (policy_type) {
		case BTC_CXP_PFIX_TD3030: /* W1:B1 = 30:30 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD5050: /* W1:B1 = 50:50 */
			_slot_set(CXST_W1, 50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 50, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD2030: /* W1:B1 = 20:30 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD2060: /* W1:B1 = 20:60 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD3070: /* W1:B1 = 30:60 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD2080: /* W1:B1 = 20:80 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 80, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TDW1B1: /* W1:B1 = user-define */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_AUTO: /* TDMA Auto-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_AUTO]);

		if (dm->fddt_train) {
			tbl_w1 = cxtbl[8];
			tbl_b1 = cxtbl[0];
		}

		switch (policy_type) {
		case BTC_CXP_AUTO_TD50B1: /* W1:B1 = 50:BTC_B1_MAX */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_AUTO_TD60B1: /* W1:B1 = 60:BTC_B1_MAX */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_AUTO_TD20B1:  /* W1:B1 = 20:BTC_B1_MAX */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_AUTO_TDW1B1: /* W1:B1 = user-define */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_AUTO_TDW1B1_CXTPS: /* W1:B1 = user-define */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			t->txflctrl= CXTPS_ON;
			break;
		}
		break;
	case BTC_CXP_PAUTO: /* PS-TDMA Auto-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_PAUTO]);

		if (dm->fddt_train) {
			tbl_w1 = cxtbl[8];
			tbl_b1 = cxtbl[2];
		}

		switch (policy_type) {
		case BTC_CXP_PAUTO_TD50B1: /* W1:B1 = 50:BTC_B1_MAX */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO_TD60B1: /* W1:B1 = 60:BTC_B1_MAX */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO_TD20B1:  /* W1:B1 = 20:BTC_B1_MAX */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO_TDW1B1: /* W1:B1 = user-define */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO_FDDT1: /* for FDD-Train Default */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_AUTO2: /* TDMA Auto-Slot2 */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_AUTO2]);

		switch (policy_type) {
		case BTC_CXP_AUTO2_TD3050: /* W1:B4 = 30:50 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD3070: /* W1:B4 = 30:70 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  70, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD5050: /* W1:B4 = 50:50 */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD6060: /* W1:B4 = 60:60 */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  60, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD2080: /* W1:B4 = 20:80 */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  80, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TDW1B4: /* W1:B1 = user-define */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4, dm->slot_dur[CXST_B4],
				  tbl_b4, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_PAUTO2: /* PS-TDMA Auto-Slot2 */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_PAUTO2]);

		switch (policy_type) {
		case BTC_CXP_PAUTO2_TD3050: /* W1:B4 = 30:50 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD3070: /* W1:B4 = 30:70 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, cxtbl[3], SLOT_MIX);
			_slot_set(CXST_B4,  70, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD5050: /* W1:B4 = 50:50 */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD6060: /* W1:B4 = 60:60 */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  60, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD2080: /* W1:B4 = 20:80 */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, BTC_B1_MAX, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  80, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TDW1B4: /* W1:B1 = user-define */
			_slot_set(CXST_W1, dm->slot_dur[CXST_W1],
				  tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, dm->slot_dur[CXST_B1],
				  tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4, dm->slot_dur[CXST_B4],
				  tbl_b4, SLOT_MIX);
			break;
		}
		break;
	}

	if (wl_rinfo->link_mode == BTC_WLINK_2G_SCC && dm->tdma.rxflctrl) {
		null_role = (dm->wl_scc.null_role1 & 0xf) +
			    ((dm->wl_scc.null_role2 & 0xf) << 4);
		_tdma_set_flctrl_role(null_role);
	}

	/* enter leak_slot after each null-1 */
	if (dm->leak_ap && dm->tdma.leak_n > 1)
		_tdma_set_lek(1);

	if (dm->tdma_instant_excute ||
	    (dm->error & BTC_DMERR_TDMA_NO_SYNC) ||
	    (dm->error & BTC_DMERR_SLOT_NO_SYNC)) {
		_tdma_set_instant();
		mode = FC_EXEC;
		PHL_INFO("[BTC], %s(): Set TDMA instant!!\n", __func__);
	}

	if (dm->fddt_train) {
		_tdma_set_fddt_en();

		/* stop -> run/debug, different AP/BT  */
		if (dm->fddt_info.state == BTC_FDDT_STATE_STOP ||
		    hal_mem_cmp(btc->hal, dm->fddt_info.wl_iot, wl->bssid, 6) ||
		    dm->fddt_info.bt_iot != a2dp->vendor_id)
			_tdma_set_fddt_renew();

		if (dm->fddt_info.type > BTC_FDDT_TYPE_AUTO)
			_tdma_set_fddt_dbg();

		hal_mem_cpy(btc->hal, dm->fddt_info.wl_iot, wl->bssid, 6);
		dm->fddt_info.bt_iot = a2dp->vendor_id;
	}

#ifdef BTC_LE_INIT_TO_WL_SLOT
	if (dm->le_init_en && t->type == CXTDMA_AUTO)
		_tdma_set_le_init_end();
#endif

	if (run_rsn("_cmd_set_coex"))
		mode = FC_EXEC;

	_update_poicy(btc, mode, policy_type, action);
}

void _set_gnt(struct btc_t *btc, u8 phy_map, u8 wl_state, u8 bt_state,
	      u8 wlact_state)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_dm *dm = &btc->dm;
	u8 bt_idx;
	u8 val[12], i;
	u8 sz1 = sizeof(dm->gnt_set);
	u8 sz2 = sizeof(dm->wlact_set);

	if (phy_map > BTC_PHY_ALL || ARRAY_SIZE(val) != (sz1 + sz2))
		return;

	bt_idx = dm->bt_select + 1;  /* translate index to bit-map */

	for (i = 0; i < BTC_PHY_MAX; i++) {
		if (!(phy_map & BIT(i)))
			continue;

		switch (wl_state) {
		case BTC_GNT_HW:
			dm->gnt_set[i].gnt_wl_sw_en = 0;
			dm->gnt_set[i].gnt_wl = 0;
			break;
		case BTC_GNT_SW_LO:
			dm->gnt_set[i].gnt_wl_sw_en = 1;
			dm->gnt_set[i].gnt_wl = 0;
			break;
		case BTC_GNT_SW_HI:
			dm->gnt_set[i].gnt_wl_sw_en = 1;
			dm->gnt_set[i].gnt_wl = 1;
			break;
		case BTC_GNT_SET_SKIP:
			break;
		}

		switch (bt_state) {
		case BTC_GNT_HW:
			dm->gnt_set[i].gnt_bt_sw_en = 0;
			dm->gnt_set[i].gnt_bt = 0;
			break;
		case BTC_GNT_SW_LO:
			dm->gnt_set[i].gnt_bt_sw_en = 1;
			dm->gnt_set[i].gnt_bt = 0;
			break;
		case BTC_GNT_SW_HI:
			dm->gnt_set[i].gnt_bt_sw_en = 1;
			dm->gnt_set[i].gnt_bt = 1;
			break;
		case BTC_GNT_SET_SKIP:
			break;
		}
	}

#if 0
	rtw_hal_mac_set_grant(h, (u8*)dm->gnt_set);
#else
	if (btc->chip->hw & BTC_FEAT_WLAN_ACT_MUX) {

		for (i = 0; i < 2; i++) {

			if (!(bt_idx & BIT(i)))
				continue;

			switch (wlact_state) {
				case BTC_WLACT_HW:
					dm->wlact_set[i].sw_en = 0;
					dm->wlact_set[i].sw_val = 0;
					break;
				case BTC_WLACT_SW_LO:
					dm->wlact_set[i].sw_en = 1;
					dm->wlact_set[i].sw_val = 0;
					break;
				case BTC_WLACT_SW_HI:
					dm->wlact_set[i].sw_en = 1;
					dm->wlact_set[i].sw_val = 1;
					break;
			}
		}
	}

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
		 "[BTC], %s(): phy_map=0x%x, gnt_wl:%d, gnt_bt:%d, wl_act:%d\n",
		 __func__, phy_map, wl_state, bt_state, wlact_state);

	hal_mem_cpy(h, &val[0], (u8*)dm->gnt_set, sz1);
	hal_mem_cpy(h, &val[sz1], (u8*)dm->wlact_set, sz2);

	rtw_hal_mac_set_grant_act(h, val);
#endif
}

static void _set_bt_plut(struct btc_t *btc, u8 val)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	u8 band_id;

	/* Set BT-polluted from PTA request-MAC */
	band_id = wl->pta_req_mac;
	if (wl->bt_polut_type[band_id] == val)
		return;

	wl->bt_polut_type[band_id] = val;
	rtw_hal_mac_set_polluted(btc->hal, band_id, val, val);
}

static void _set_dbcc_ant_ctrl(struct btc_t *btc)
{
	struct btc_wl_dbcc_info *wl_dinfo = &btc->cx.wl.dbcc_info;
	u8 gnt_wl_ctrl, plt_ctrl, i, b2g = 0;
	bool is_dbcc_all_2g = true;

	for (i = 0; i < HW_PHY_MAX; i++) {
		if (wl_dinfo->real_band[i] != BAND_ON_24G) {
			is_dbcc_all_2g = false;
			break;
		}
	}

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
		  "[BTC], %s(): is_dbcc_all_2g=%d\n", __func__, is_dbcc_all_2g);

	/* if dbcc 2-phy are 2G */
	if (is_dbcc_all_2g) {
		/* Set PHY-0 SW control  */
		_set_gnt(btc, BIT(HW_PHY_0), BTC_GNT_SW_HI, BTC_GNT_SW_LO,
			 BTC_WLACT_HW);

		/* Set PHY-1(BTG) HW control */
		_set_gnt(btc, BIT(HW_PHY_1), BTC_GNT_HW, BTC_GNT_HW,
			 BTC_WLACT_HW);

		_set_bt_plut(btc, BTC_PLT_GNT_WL);

		btc->cx.cnt_wl[BTC_WCNT_DBCC_ALL_2G]++;
		return;
	}

	for (i = 0; i < HW_PHY_MAX; i++) {
		b2g = (wl_dinfo->real_band[i] == BAND_ON_24G);

		gnt_wl_ctrl = (b2g? BTC_GNT_HW : BTC_GNT_SW_HI);
		_set_gnt(btc, BIT(i), gnt_wl_ctrl, BTC_GNT_HW, BTC_WLACT_HW);

		plt_ctrl = (b2g? BTC_PLT_BT : BTC_PLT_NONE);
		if (b2g)
			_set_bt_plut(btc, BTC_PLT_GNT_WL);
	}
}

static void _set_ant(struct btc_t *btc, bool force_exec, u8 phy_map, u8 type)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	u32 ant_path_type;

	ant_path_type = ((phy_map << 8) + type);

	if (run_rsn("_cmd_set_coex") || run_rsn("_ntfy_power_off") ||
	    run_rsn("_ntfy_radio_state") || wl->role_info.dbcc_chg)
		force_exec = FC_EXEC;

	if (wl_rinfo->link_mode != BTC_WLINK_25G_MCC &&
	    btc->dm.wl_btg_rx == 2)
		force_exec = FC_EXEC;

	if (!force_exec && (ant_path_type == dm->set_ant_path)) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
			  "[BTC], %s(): return by no change!!\n", __func__);
		return;
	} else if (bt->rfk_info.map.run) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
			  "[BTC], %s(): return by bt rfk!!\n", __func__);
		return;
	} else if (!run_rsn("_ntfy_wl_rfk") &&
		   wl->rfk_info.state != BTC_WRFK_STOP) {
		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
			  "[BTC], %s(): return by wl rfk!!\n", __func__);
		return;
	}

	dm->set_ant_path = ant_path_type;

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
		  "[BTC], %s(): path=0x%x, set_type=%s\n", __func__,
		  phy_map, id_to_str(BTC_STR_ANTPATH, dm->set_ant_path & 0xff));

	switch (type){
	case BTC_ANT_WPOWERON:
		_set_cx_ctrl(btc, BTC_CTRL_BY_BT);
		break;
	case BTC_ANT_WINIT:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		/* To avoid BT MP driver case (bt_enable but no mailbox) */
		if (bt->enable.now && bt->run_patch_code) {
			_set_gnt(btc, phy_map, BTC_GNT_SW_LO, BTC_GNT_SW_HI,
				 BTC_WLACT_SW_LO);
		} else {
			_set_gnt(btc, phy_map, BTC_GNT_SW_HI, BTC_GNT_SW_LO,
				 BTC_WLACT_SW_HI);
		}
		break;
	case BTC_ANT_WONLY:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt(btc, phy_map, BTC_GNT_SW_HI, BTC_GNT_SW_LO,
			 BTC_WLACT_SW_HI);
		break;
	case BTC_ANT_WOFF:
		_set_cx_ctrl(btc, BTC_CTRL_BY_BT);
		break;
	case BTC_ANT_W2G:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		if (wl_rinfo->dbcc_en) {
			_set_dbcc_ant_ctrl(btc);
			return;
		}
		_set_gnt(btc, phy_map, BTC_GNT_HW, BTC_GNT_HW, BTC_WLACT_HW);
		break;
	case BTC_ANT_W5G:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt(btc, phy_map, BTC_GNT_SW_HI, BTC_GNT_HW, BTC_WLACT_HW);
		break;
	case BTC_ANT_W25G:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		if (wl_rinfo->dbcc_en) {
			_set_dbcc_ant_ctrl(btc);
			return;
		}
		_set_gnt(btc, phy_map, BTC_GNT_HW, BTC_GNT_HW, BTC_WLACT_HW);
		break;
	case BTC_ANT_FREERUN:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt(btc, phy_map, BTC_GNT_SW_HI, BTC_GNT_SW_HI,
			 BTC_WLACT_SW_LO);
		break;
	case BTC_ANT_FDDTRAIN:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt(btc, phy_map, BTC_GNT_SW_HI, BTC_GNT_HW, BTC_WLACT_HW);
		break;
	case BTC_ANT_WRFK:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt(btc, phy_map, BTC_GNT_SW_HI, BTC_GNT_SW_LO,
			 BTC_WLACT_HW);
		break;
	case BTC_ANT_WRFK2:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt(btc, phy_map, BTC_GNT_SW_HI, BTC_GNT_SW_LO,
			 BTC_WLACT_SW_HI); /* no BT-Tx */
		break;
	case BTC_ANT_BRFK:
		_set_cx_ctrl(btc, BTC_CTRL_BY_BT);
		_set_gnt(btc, phy_map, BTC_GNT_SW_LO, BTC_GNT_SW_HI,
			 BTC_WLACT_SW_LO);
		break;
	}

	_set_bt_plut(btc, BTC_PLT_GNT_WL);
}

void _action_wl_only(struct btc_t *btc)
{
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s !!\n", __func__);

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_WONLY);
	_set_policy(btc, BTC_CXP_OFF_WL, __func__);
}

void _action_wl_init(struct btc_t *btc)
{

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WINIT);

	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_wl_off(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s !!\n", __func__);

	/* don't set PTA control if LPS-PG/LPS-CG */
	if (wl->status.map.rf_off || btc->dm.bt_only) {
		_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_WOFF);
	} else if (wl->status.map.lps == BTC_LPS_RF_ON) {
		if (wl->role_info.link_mode == BTC_WLINK_5G)
			_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_W5G);
		else
			_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	}

	if (wl->role_info.link_mode == BTC_WLINK_5G) {
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	} else if (wl->status.map.lps == BTC_LPS_RF_ON) {
		if (btc->cx.bt.link_info.a2dp_desc.active)
			_set_policy(btc, BTC_CXP_OFF_BT, __func__);
		else
			_set_policy(btc, BTC_CXP_OFF_BWB1, __func__);
	} else {
		_set_policy(btc, BTC_CXP_OFF_BT, __func__);
	}
}

void _action_freerun(struct btc_t *btc)
{
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s !!\n", __func__);
	btc->dm.freerun = true;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_fddtrain(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_bt_info *bt = &btc->cx.bt;
	u16 policy_type = BTC_CXP_OFF_BT;

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
		  "[BTC], %s() type=%d !!\n", __func__, dm->fddt_info.type);

	bt->scan_rx_low_pri = true;

	switch (dm->fddt_info.type) {
	case BTC_FDDT_TYPE_AUTO:
		dm->slot_dur[CXST_W1] = 50;
		dm->slot_dur[CXST_B1] = BTC_B1_MAX;
		policy_type = BTC_CXP_PAUTO_FDDT1;
		break;
	case BTC_FDDT_TYPE_FIX_TDD:
		dm->slot_dur[CXST_W1] = 50;
		dm->slot_dur[CXST_B1] = BTC_B1_MAX;
		policy_type = BTC_CXP_PAUTO_TDW1B1;
		break;
	case BTC_FDDT_TYPE_FIX_FULL_FDD:
		dm->slot_dur[CXST_W1] = 50;
		dm->slot_dur[CXST_B1] = 50;
		policy_type = BTC_CXP_FIX_TDW1B1;
		break;
	}

	dm->fddt_train = BTC_FDDT_ENABLE;

	_set_fddt_ctrl(btc, NM_EXEC);
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

void _action_bt_whql(struct btc_t *btc)
{
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_bt_rfk(struct btc_t *btc)
{
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s !!\n", __func__);

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_BRFK);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_bt_off(struct btc_t *btc)
{
	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_, "[BTC], %s !!\n", __func__);

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_WONLY);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

static void _action_bt_idle(struct btc_t *btc)
{
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *b = &btc->cx.bt.link_info;
	struct btc_wl_info *wl = &btc->cx.wl;
	u16 policy_type = BTC_CXP_OFF_BT;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		switch(btc->cx.state_map) {
		case BTC_WBUSY_BNOSCAN: /* wl-busy + bt idle*/
		case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-idle */
			if (b->status.map.connect)
				policy_type = BTC_CXP_FIX_TD4010;
			else if (wl->status.map.traffic_dir & BIT(TRAFFIC_DL))
				policy_type = BTC_CXP_FIX_TD4010ISO_DL;
			else
				policy_type = BTC_CXP_FIX_TD4010ISO_UL;
			break;
		case BTC_WBUSY_BSCAN: /*wl-busy + bt-inq */
			if (bt->pag)
				policy_type = BTC_CXP_PFIX_TD2080;
			else
				policy_type = BTC_CXP_PFIX_TD5050;
			break;
		case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq */
		case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq */
			if (bt->pag)
				policy_type = BTC_CXP_FIX_TD2080;
			else
				policy_type = BTC_CXP_FIX_TD5050;
			break;
		case BTC_WLINKING: /* wl-connecting + bt-inq or bt-idle */
			if (bt->pag)
				policy_type = BTC_CXP_FIX_TD2080;
			else
				policy_type = BTC_CXP_FIX_TD7010;
			break;
		case BTC_WIDLE:       /* wl-idle + bt-idle */
			if (b->status.map.connect)
				policy_type = BTC_CXP_OFF_BWB1;
			else
				policy_type = BTC_CXP_FIX_TD4010ISO_DL;
			break;
		}
	} else { /* dedicated-antenna */
		policy_type = BTC_CXP_OFF_EQ0;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_hfp(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_a2dp_desc *a2dp = &btc->cx.bt.link_info.a2dp_desc;
	u16 policy_type = BTC_CXP_OFF_BT;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (btc->cx.wl.status.map._4way) {
			policy_type = BTC_CXP_OFF_WL;
		} else if (a2dp->active) {
			btc->dm.slot_dur[CXST_W1] = 80;
			btc->dm.slot_dur[CXST_B1] = 20;
			policy_type = BTC_CXP_PFIX_TDW1B1;
		} else if (wl->status.map.traffic_dir & BIT(TRAFFIC_UL)) {
			btc->cx.bt.scan_rx_low_pri = true;
			policy_type = BTC_CXP_OFF_BWB2;
		} else if (btc->mdinfo.wa_type & BTC_WA_HFP_LAG) {
			policy_type = BTC_CXP_OFF_BWB2;
		}else {
			policy_type = BTC_CXP_OFF_BWB1;
		}
	} else { /* dedicated-antenna */
		if (wl->legacy_mode)
			policy_type = BTC_CXP_OFF_BWB1;
		else if (wl->status.map.traffic_dir & BIT(TRAFFIC_UL))
			policy_type = BTC_CXP_OFF_EQ5;
		else
			policy_type = BTC_CXP_OFF_EQ2;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_hid(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_hid_desc *hid = &bt->link_info.hid_desc;
	struct btc_bt_a2dp_desc *a2dp = &bt->link_info.a2dp_desc;
	struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (wl->status.map._4way) {
			policy_type = BTC_CXP_OFF_WL;
		} else if (a2dp->active || bt->hiduty_dev) {
			btc->dm.slot_dur[CXST_W1] = 80;
			btc->dm.slot_dur[CXST_B1] = 20;
			policy_type = BTC_CXP_PFIX_TDW1B1;
		} else if (wl->status.map.traffic_dir & BIT(TRAFFIC_UL)) {
			btc->cx.bt.scan_rx_low_pri = true;
			if (hid->type & BTC_HID_BLE)
				policy_type = BTC_CXP_OFF_BWB0;
			else
				policy_type = BTC_CXP_OFF_BWB2;
		} else if (hid->type == BTC_HID_218) {
			bt->scan_rx_low_pri = true;
			policy_type = BTC_CXP_OFF_BWB2;
		} else if (btc->chip->hw & BTC_FEAT_NONBTG_GWL_THRU) {
			policy_type = BTC_CXP_OFF_BWB3;
		} else if (btc->mdinfo.wa_type & BTC_WA_HFP_LAG &&
			   bt->link_info.hfp_desc.exist) {
			policy_type = BTC_CXP_OFF_BWB2;
		}else {
			policy_type = BTC_CXP_OFF_BWB1;
		}
	} else { /* dedicated-antenna */
		if (wl->legacy_mode)
			policy_type = BTC_CXP_OFF_BWB1;
		else if (wl->status.map.traffic_dir & BIT(TRAFFIC_UL))
			policy_type = BTC_CXP_OFF_EQ4;
		else
			policy_type = BTC_CXP_OFF_EQ3;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_a2dp(struct btc_t *btc)
{
	struct btc_bt_link_info *bt_linfo = &btc->cx.bt.link_info;
	struct btc_bt_a2dp_desc a2dp = bt_linfo->a2dp_desc;
	struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT, w1_slot_dur = 0;

	if (a2dp.vendor_id == 0x4c || dm->leak_ap || bt_linfo->slave_role) {
	    if (bt_linfo->slave_role)
	    	w1_slot_dur = 20;
	    else
	    	w1_slot_dur = 40;
	} else {
		w1_slot_dur = 50;
	}

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP */
		if (btc->mdinfo.wa_type & BTC_WA_NULL_AP) { // for sporton Tx null issue
			dm->slot_dur[CXST_W1] = w1_slot_dur;
			dm->slot_dur[CXST_B1] = BTC_B1_MAX;
			policy_type = BTC_CXP_AUTO_TDW1B1_CXTPS;
		} else {
			dm->slot_dur[CXST_W1] = w1_slot_dur;
			dm->slot_dur[CXST_B1] = BTC_B1_MAX;
			policy_type = BTC_CXP_PAUTO_TDW1B1;
		}
		break;
	case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq + bt-A2DP */
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2DP */
		policy_type = BTC_CXP_PAUTO2_TD3070;
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2DP */
		policy_type = BTC_CXP_AUTO2_TD3050;
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP */
	case BTC_WLINKING:      /* wl-connecting + bt-A2DP */
		dm->slot_dur[CXST_W1] = w1_slot_dur;
		dm->slot_dur[CXST_B1] = BTC_B1_MAX;
		policy_type = BTC_CXP_AUTO_TDW1B1;
		break;
	case BTC_WIDLE:  /* wl-idle + bt-A2DP */
		policy_type = BTC_CXP_AUTO_TD20B1;
		break;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_a2dpsink(struct btc_t *btc)
{
	u16 policy_type = BTC_CXP_OFF_BT;

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2dp_Sink */
		policy_type = BTC_CXP_PFIX_TD3070;
		break;
	case BTC_WBUSY_BSCAN:   /* wl-busy + bt-inq + bt-A2dp_Sink */
		policy_type = BTC_CXP_PFIX_TD2060;
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2dp_Sink */
		policy_type = BTC_CXP_FIX_TD2030;
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2dp_Sink */
		policy_type = BTC_CXP_FIX_TD2060;
		break;
	case BTC_WLINKING:    /* wl-connecting + bt-A2dp_Sink */
		policy_type = BTC_CXP_FIX_TD3030;
		break;
	case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq + bt-A2dp_Sink */
	case BTC_WIDLE:       /* wl-idle + bt-A2dp_Sink */
		policy_type = BTC_CXP_FIX_TD2080;
		break;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_leaudio(struct btc_t *btc)
{
	if (btc->mdinfo.ant.type == BTC_ANT_DEDICATED)
		_action_freerun(btc);
}

static void _action_bt_pan(struct btc_t *btc)
{
	struct btc_bt_a2dp_desc *a2dp = &btc->cx.bt.link_info.a2dp_desc;
	u16 policy_type = BTC_CXP_OFF_BT;

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-PAN */
		if (a2dp->active) {
			btc->dm.slot_dur[CXST_W1] = 80;
			btc->dm.slot_dur[CXST_B1] = 20;
			policy_type = BTC_CXP_PFIX_TDW1B1;
		} else {
			policy_type = BTC_CXP_PFIX_TD5050;
		}
		break;
	case BTC_WBUSY_BSCAN:   /* wl-busy + bt-inq + bt-PAN */
		policy_type = BTC_CXP_PFIX_TD3070;
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-PAN */
		policy_type = BTC_CXP_FIX_TD3030;
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-PAN */
		policy_type = BTC_CXP_FIX_TD3060;
		break;
	case BTC_WLINKING:    /* wl-connecting + bt-PAN */
		policy_type = BTC_CXP_FIX_TD4010ISO;
		break;
	case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq + bt-pan */
	case BTC_WIDLE:       /* wl-idle + bt-pan */
		policy_type = BTC_CXP_PFIX_TD2080;
		break;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_a2dp_hid(struct btc_t *btc)
{
	struct btc_bt_link_info *bt_linfo = &btc->cx.bt.link_info;
	struct btc_bt_a2dp_desc a2dp = bt_linfo->a2dp_desc;
	struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT, w1_slot_dur = 0;

	if (a2dp.vendor_id == 0x4c || dm->leak_ap || bt_linfo->slave_role) {
	    if (bt_linfo->slave_role)
	    	w1_slot_dur = 20;
	    else
	    	w1_slot_dur = 40;
	} else {
		w1_slot_dur = 50;
	}

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP + HID */
	case BTC_WIDLE:  /* wl-idle + bt-A2DP + HID */
		dm->slot_dur[CXST_W1] = w1_slot_dur;
		dm->slot_dur[CXST_B1] = BTC_B1_MAX;
		policy_type = BTC_CXP_PAUTO_TDW1B1;
		break;
	case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq + bt-A2DP + HID */
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2DP + HID */
		policy_type = BTC_CXP_PAUTO2_TD3070;
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2DP + HID */
		policy_type = BTC_CXP_AUTO2_TD3050;
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP + HID */
	case BTC_WLINKING: /* wl-connecting + bt-A2DP + HID */
		dm->slot_dur[CXST_W1] = w1_slot_dur;
		dm->slot_dur[CXST_B1] = BTC_B1_MAX;
		policy_type = BTC_CXP_AUTO_TDW1B1;
		break;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_a2dp_pan(struct btc_t *btc)
{
	struct btc_bt_link_info *bt_linfo = &btc->cx.bt.link_info;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT;
	bool wl_cpt_test = false;

	if (dm->vid == BTC_VID_LNV && dm->tpl_ap &&
	    bt_linfo->slave_role && wl->rssi_level <= 2)
		wl_cpt_test = true;

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP+PAN */
		if (wl_cpt_test)
			policy_type = BTC_CXP_PAUTO2_TD5050;
		else
			policy_type = BTC_CXP_PAUTO2_TD3070;
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2DP+PAN */
		if (wl_cpt_test)
			policy_type = BTC_CXP_PAUTO2_TD5050;
		else
			policy_type = BTC_CXP_PAUTO2_TD3070;
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP+PAN */
		policy_type = BTC_CXP_AUTO2_TD5050;
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2DP+PAN */
		policy_type = BTC_CXP_AUTO2_TD3070;
		break;
	case BTC_WLINKING:    /* wl-connecting + bt-A2DP+PAN */
		policy_type = BTC_CXP_AUTO2_TD3050;
		break;
	case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq + bt-A2DP+PAN */
	case BTC_WIDLE:       /* wl-idle + bt-A2DP+PAN */
		policy_type = BTC_CXP_PAUTO2_TD2080;
		break;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_pan_hid(struct btc_t *btc)
{
	u16 policy_type = BTC_CXP_OFF_BT;

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-PAN+HID */
		policy_type = BTC_CXP_PFIX_TD3030;
		break;
	case BTC_WBUSY_BSCAN:  /* wl-busy + bt-inq + bt-PAN+HID */
		policy_type = BTC_CXP_PFIX_TD3070;
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-PAN+HID */
		policy_type = BTC_CXP_FIX_TD3030;
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-PAN+HID */
		policy_type = BTC_CXP_FIX_TD3060;
		break;
	case BTC_WLINKING:   /* wl-connecting + bt-PAN+HID */
		policy_type = BTC_CXP_FIX_TD4010;
		break;
	case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq + bt-PAN+HID */
	case BTC_WIDLE:       /* wl-idle + bt-PAN+HID */
		policy_type = BTC_CXP_PFIX_TD2080;
		break;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

static void _action_bt_a2dp_pan_hid(struct btc_t *btc)
{
	u16 policy_type = BTC_CXP_OFF_BT;

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP+PAN+HID */
		policy_type = BTC_CXP_PAUTO2_TD3070;
		break;
	case BTC_WBUSY_BSCAN:   /* wl-busy + bt-inq + bt-A2DP+PAN+HID */
		policy_type = BTC_CXP_PAUTO2_TD3070;
		break;
	case BTC_WSCAN_BSCAN:   /* wl-scan + bt-inq + bt-A2DP+PAN+HID */
		policy_type = BTC_CXP_AUTO2_TD3070;
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP+PAN+HID */
	case BTC_WLINKING:      /* wl-connecting + bt-A2DP+PAN+HID */
		policy_type = BTC_CXP_AUTO2_TD3050;
		break;
	case BTC_WIDLE_BSCAN: /* wl-idle + bt-inq + bt-A2DP+PAN+HID */
	case BTC_WIDLE:       /* wl-idle + bt-A2DP+PAN+HID */
		policy_type = BTC_CXP_PAUTO2_TD2080;
		break;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

void _action_wl_5g(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W5G);
	_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_wl_other(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED)
		_set_policy(btc, BTC_CXP_OFFB_BWB0, __func__);
	else
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_wl_idle(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED)
		_set_policy(btc, BTC_CXP_OFFB_BWB0, __func__);
	else
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_wl_nc(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_wl_rfk(struct btc_t *btc)
{
	struct btc_wl_rfk_info rfk = btc->cx.wl.rfk_info;

	if (rfk.state != BTC_WRFK_START && rfk.type != BTC_RF_RXDCK)
		return;

	PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
		  "[BTC], %s(): band=%d, type=%d, state=%d\n",
		  __func__, rfk.band, rfk.type, rfk.state);

	btc->dm.tdma_instant_excute = 1;

	if ((rfk.type == BTC_RF_RXDCK && rfk.state == BTC_WRFK_ONESHOT_START) ||
	    btc->mdinfo.ant.type == BTC_ANT_SHARED) {
		_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WRFK2);
		_set_policy(btc, BTC_CXP_OFF_WL2, __func__);
	} else {
		_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WRFK);
		_set_policy(btc, BTC_CXP_OFF_WL, __func__);
	}
}

void _action_common(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	u32 bt_rom_code_id = 0, bt_fw_ver = 0;

#if BTC_BB_TX_1SS_LIMIT
	_set_trx_nss(btc);
#endif
	_set_halbb_btg_ctrl(btc);
	_set_halbb_preagc_ctrl(btc);
	_set_halmac_tx_limit(btc);
	_set_phl_bt_slot_req(btc);
	_set_phl_stbc_req(btc); /* STBC flag*/
	_set_bt_afh_info(btc);
	_set_bt_rx_agc(btc); /* must call after _set_halbb_btg_ctrl() */
	_set_rf_trx_para(btc);
	_set_fddt_status(btc);

	_write_scbd(btc, BTC_WSCB_RXSCAN_PRI, (bool)bt->scan_rx_low_pri);

	if (wl->scbd_change) {
		rtw_hal_mac_set_scoreboard(btc->hal, &wl->scbd);
		/* Add delay to avoid BT FW loss information */
		_os_delay_us(btc->hal, BTC_SCBD_REWRITE_DELAY);
		wl->scbd_change = 0;

		PHL_TRACE(COMP_PHL_BTC, _PHL_DEBUG_,
			  "[BTC], write scbd : 0x%08x \n", wl->scbd);
		btc->cx.cnt_wl[BTC_WCNT_SCBDUPDATE]++;
	}

	bt_rom_code_id = (btc->chip->chip_id & 0xffff0) >> 4;
	bt_fw_ver = bt->ver_info.fw & 0xffff;
	if (bt->enable.now && (bt_fw_ver == 0 ||
	    (bt_fw_ver == bt_rom_code_id &&
	    bt->run_patch_code && btc->chip->scbd)))
		_set_fw_rpt(btc, RPT_EN_BT_VER_INFO, 1);
	else
		_set_fw_rpt(btc, RPT_EN_BT_VER_INFO, 0);


	btc->dm.tdma_instant_excute = 0;
}

void _action_wl_2g_sta(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;
	struct btc_bt_hid_desc hid = bt_linfo->hid_desc;
	struct btc_bt_a2dp_desc a2dp = bt_linfo->a2dp_desc;
	struct btc_bt_pan_desc pan = bt_linfo->pan_desc;
	u8 profile_map = 0;

	if (dm->freerun_chk && !dm->fddt_train_chk) {
		_action_freerun(btc);
		return;
	}

	if (bt_linfo->hfp_desc.exist)
		profile_map |= BTC_BT_HFP;

	if (bt_linfo->hid_desc.exist || bt->hiduty_dev)
		profile_map |= BTC_BT_HID;

	if (bt_linfo->a2dp_desc.exist)
		profile_map |= BTC_BT_A2DP;

	if (bt_linfo->pan_desc.exist)
		profile_map |= BTC_BT_PAN;

	if (bt_linfo->leaudio_desc.exist) {
		profile_map |= BTC_BT_LEAUDIO;
		_action_bt_leaudio(btc);
		return;
	}

	switch (profile_map) {
	case 0:
		if (a2dp.active || pan.active)
			_action_bt_pan(btc);
		else
			_action_bt_idle(btc);
		break;
	case BTC_BT_HFP:
			_action_bt_hfp(btc);
		break;
	case BTC_BT_HFP | BTC_BT_HID:
	case BTC_BT_HID:
			_action_bt_hid(btc);
		break;
	case BTC_BT_A2DP:
		if (dm->fddt_train_chk)
			_action_fddtrain(btc);
		else if (a2dp.sink)
			_action_bt_a2dpsink(btc);
		else if (bt_linfo->multi_link.now && !hid.pair_cnt)
			 _action_bt_a2dp_pan(btc);
		else
			_action_bt_a2dp(btc);
		break;
	case BTC_BT_PAN:
		_action_bt_pan(btc);
		break;
	case BTC_BT_A2DP | BTC_BT_HFP:
	case BTC_BT_A2DP | BTC_BT_HID:
	case BTC_BT_A2DP | BTC_BT_HFP | BTC_BT_HID:
		if (dm->fddt_train_chk)
			_action_fddtrain(btc);
		else if (a2dp.sink)
			_action_bt_a2dpsink(btc);
		else if (pan.active)
			_action_bt_a2dp_pan_hid(btc);
		else
			_action_bt_a2dp_hid(btc);
		break;
	case BTC_BT_A2DP | BTC_BT_PAN:
		if (a2dp.sink)
			_action_bt_a2dpsink(btc);
		else
			_action_bt_a2dp_pan(btc);
		break;
	case BTC_BT_PAN | BTC_BT_HFP:
	case BTC_BT_PAN | BTC_BT_HID:
	case BTC_BT_PAN | BTC_BT_HFP | BTC_BT_HID:
		_action_bt_pan_hid(btc);
		break;
	case BTC_BT_A2DP | BTC_BT_PAN | BTC_BT_HID:
	case BTC_BT_A2DP | BTC_BT_PAN | BTC_BT_HFP:
	default:
		if (a2dp.sink)
			_action_bt_a2dpsink(btc);
		else
			_action_bt_a2dp_pan_hid(btc);
		break;
	}
}

void _action_wl_scan(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_dbcc_info *wl_dinfo = &wl->dbcc_info;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;

	if (btc->hal->scanofld_en) {
		_action_wl_25g_mcc(btc);
	} else if (wl_rinfo->dbcc_en) {
		if (wl_dinfo->real_band[HW_PHY_0] != BAND_ON_24G &&
		    wl_dinfo->real_band[HW_PHY_1] != BAND_ON_24G)
			_action_wl_5g(btc);
		else
			_action_wl_2g_sta(btc);
	} else {
		if (wl->scan_info.band[HW_PHY_0] != BAND_ON_24G)
			_action_wl_5g(btc);
		else
			_action_wl_2g_sta(btc);
	}
}

void _action_wl_25g_mcc(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
    struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) {
		if (wl->status.map._4way)
			policy_type = BTC_CXP_OFFE_WL;
		else if (wl->status.val & BTC_WL_LINK_SCAN)
			policy_type = BTC_CXP_OFFE_2GBWMIXB;
		else if (bt->link_info.status.map.connect == 0)
			policy_type = BTC_CXP_OFFE_2GISOB;
		else
			policy_type = BTC_CXP_OFFE_2GBWISOB;
	} else { /* dedicated-antenna */
		policy_type = BTC_CXP_OFF_EQ0;
	}

	dm->e2g_slot_limit = BTC_E2G_LIMIT_DEF;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W25G);
	_set_policy(btc, policy_type, __func__);
}

void _action_wl_2g_mcc(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (wl->status.map._4way)
			policy_type = BTC_CXP_OFFE_WL;
		else if (bt->link_info.status.map.connect == 0)
			policy_type = BTC_CXP_OFFE_2GISOB;
		else
			policy_type = BTC_CXP_OFFE_2GBWISOB;
	} else { /* dedicated-antenna */
		policy_type = BTC_CXP_OFF_EQ0;
	}

	dm->e2g_slot_limit = BTC_E2G_LIMIT_DEF;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

void _action_wl_2g_scc(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	u16 policy_type = BTC_CXP_OFF_BT;
	u32 dur = 0, tnull1 = 0;

	if (btc->mdinfo.ant.type == BTC_ANT_DEDICATED) {
		_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
		return;
	}

	/* In 2G-SCC + ext-slot mode, tx/rx flow ctrl is BTC's job.
	 * But WL FW should notify BTC of E2G/EBT slot switch if Go-NOA is on
	 * (From P2P-Power-Save function).
	 */

	/* shared-antenna */
	switch(wl_rinfo->mrole_type) {
	default:
		break;
	case BTC_WLMROLE_STA_GC:
		dm->wl_scc.null_role1 = PHL_RTYPE_STATION;
		dm->wl_scc.null_role2 = PHL_RTYPE_P2P_GC;
		dm->wl_scc.e2g_null = 0; /* no ext-slot-control */
		_action_wl_2g_sta(btc);
		return;
	case BTC_WLMROLE_STA_STA:
		dm->wl_scc.null_role1 = PHL_RTYPE_STATION;
		dm->wl_scc.null_role2 = PHL_RTYPE_STATION;
		dm->wl_scc.e2g_null = 0; /* no ext-slot-control */
		_action_wl_2g_sta(btc);
		return;
	case BTC_WLMROLE_STA_GC_NOA:
	case BTC_WLMROLE_STA_GO:
	case BTC_WLMROLE_STA_GO_NOA:
		dm->wl_scc.null_role1 = PHL_RTYPE_STATION;
		dm->wl_scc.null_role2 = PHL_RTYPE_NONE;
		dur = wl_rinfo->mrole_noa_duration;

		if (wl->status.map._4way) {
			dm->wl_scc.e2g_null = 0;
			policy_type = BTC_CXP_OFFE_WL;
		} else if (bt->link_info.status.map.connect == 0) {
			dm->wl_scc.e2g_null = 0;
			policy_type = BTC_CXP_OFFE_2GISOB;
		} else if (bt->link_info.a2dp_desc.exist ||
 			   bt->link_info.pan_desc.exist) {
 			dm->wl_scc.e2g_null = 1; /* tx null at E2G before EBT */
			policy_type = BTC_CXP_OFFE_2GBWISOB;
		} else {
			dm->wl_scc.e2g_null = 0;
			policy_type = BTC_CXP_OFFE_2GBWISOB;
		}

		break;
	}

	/* use E2G dur as timer to Tx-null-1 before EBT
	 * E2G --> Null1-Tx --> 10ms --> EBT
	 * tnull1 = from E2G to Tx-Null1
	 */
	if (dm->wl_scc.e2g_null &&
	    dur <= BTC_P2P_CYCLE - BTC_NULL1_EBT_EARLY_T && dur > 0)
	    tnull1 = BTC_P2P_CYCLE - BTC_NULL1_EBT_EARLY_T - dur;

	if (tnull1 > BTC_NULL1_EBT_EARLY_T)
		dm->e2g_slot_nulltx_time = tnull1;
	else
		dm->e2g_slot_nulltx_time = 0;

	dm->e2g_slot_limit = BTC_E2G_LIMIT_DEF;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

void _action_wl_2g_ap(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT;

	/* Todo:if client issue Null-P, ap should follow Null/Null-P slot */
	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (wl->status.map._4way)
			policy_type = BTC_CXP_OFFE_WL;
		else if (bt->link_info.status.map.connect == 0)
			policy_type = BTC_CXP_OFFE_2GISOB;
		else
			policy_type = BTC_CXP_OFFE_2GBWISOB;
	} else {/* dedicated-antenna */
		policy_type = BTC_CXP_OFF_EQ0;
	}

	dm->e2g_slot_limit = BTC_E2G_LIMIT_DEF;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

void _action_wl_2g_go(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_a2dp_desc a2dp = bt->link_info.a2dp_desc;
	struct btc_dm *dm = &btc->dm;
	u16 policy_type = BTC_CXP_OFF_BT;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (wl->status.map._4way)
			policy_type = BTC_CXP_OFFE_WL;
		else if (a2dp.exist && !a2dp.sink)
			policy_type = BTC_CXP_OFFE_2GBWISOB;
		else if (dm->client_ps_tdma_on == 1 && a2dp.sink)
			policy_type = BTC_CXP_OFF_BT;
		else if (bt->link_info.status.map.connect == 0)
			policy_type = BTC_CXP_OFFE_2GISOB;
		else
			policy_type = BTC_CXP_OFFE_2GBWISOB;
	} else { /* dedicated-antenna */
		policy_type = BTC_CXP_OFF_EQ0;
	}

	dm->e2g_slot_limit = BTC_E2G_LIMIT_DEF;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

void _action_wl_2g_gc(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	u8 noa = 0, rid = 0;
	u16 policy_type = BTC_CXP_OFF_BT;
	u32 noa_dur = 0;


	rid = _get_wl_role_idx(btc, PHL_RTYPE_P2P_GC);

	if (rid < BTC_WL_MAX_ROLE_NUMBER) {
		noa = wl_rinfo->active_role[rid].noa;
		noa_dur = wl_rinfo->active_role[rid].noa_dur;
	}
	/* Check GC, GC+NOA */
	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (!noa) { /* Gc without NOA */
			_action_wl_2g_sta(btc);
			return;
		}

		/* Gc with NOA */
		if (wl->status.map._4way)
			policy_type = BTC_CXP_OFFE_WL;
		else if (bt->link_info.status.map.connect == 0)
			policy_type = BTC_CXP_OFFE_2GISOB;
		else
			policy_type = BTC_CXP_OFFE_2GBWISOB;

	} else {/* dedicated-antenna */
		policy_type = BTC_CXP_OFF_EQ0;
	}

	/* Adjust E2G limit duration to avoid A2DP pop
	 * If reach this duration from E2G start,
	 * change slot-table form E2G to ENULL (allow BT)
	 */
	if (btc->bt_req_len[wl->pta_req_mac] != 0)
		dm->e2g_slot_limit = (BTC_P2P_CYCLE * noa_dur) /btc->bt_req_len[wl->pta_req_mac];

#if 0
	dm->e2g_slot_limit += BTC_E2G_OFFSET;
#endif

	if (dm->e2g_slot_limit < BTC_E2G_LIMIT_MIN)
		dm->e2g_slot_limit = BTC_E2G_LIMIT_MIN;
	else if (dm->e2g_slot_limit > BTC_E2G_LIMIT_MAX)
		dm->e2g_slot_limit = BTC_E2G_LIMIT_MAX;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

void _action_wl_2g_nan(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	u16 policy_type = BTC_CXP_OFF_BT;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (wl->status.map._4way)
			policy_type = BTC_CXP_OFFE_WL;
		else if (bt->link_info.status.map.connect == 0)
			policy_type = BTC_CXP_OFFE_2GISOB;
		else
			policy_type = BTC_CXP_OFFE_2GBWISOB;
	} else { /* dedicated-antenna */
		policy_type = BTC_CXP_OFF_EQ0;
	}

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, policy_type, __func__);
}

#endif
