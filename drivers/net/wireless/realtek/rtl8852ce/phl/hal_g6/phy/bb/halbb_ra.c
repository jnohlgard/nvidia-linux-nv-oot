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
#include "halbb_precomp.h"
#ifdef HALBB_RA_SUPPORT

#ifdef PHL_USE_RA_BW_MODE
#define HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta) (phl_sta)->hal_sta->ra_info.ra_bw_mode
#else
#define HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta) (phl_sta)->chandef.bw
#endif

const u16 bb_phy_rate_table[] = {
	/*CCK*/
	1, 2, 5, 11,
	/*OFDM*/
	6, 9, 12, 18, 24, 36, 48, 54,
	/*HT/VHT-1ss LGI*/
	6, 13, 19, 26, 39, 52, 58, 65, 78, 87, 98, 108,
	/*HT/VHT-2ss LGI*/
	13, 26, 39, 52, 78, 104, 117, 130, 156, 173, 195, 217,
	/*HT/VHT-3ss LGI*/
	19, 39, 58, 78, 117, 156, 175, 195, 234, 260, 293, 325,
	/*HT/VHT-4ss LGI*/
	26, 52, 78, 104, 156, 208, 234, 260, 312, 347, 390, 433,
}; /*HE[3.2] = VHT[LGI] * 1.125*/

u8 halbb_get_max_mode(struct bb_info *bb)

{
	struct bb_link_info	*link = &bb->bb_link_i;

	if (link->wlan_mode_bitmap & BIT6)
		return 4;
	else if (link->wlan_mode_bitmap & WLAN_MD_11AX)
		return 3;
	else if (link->wlan_mode_bitmap & WLAN_MD_11AC)
		return 2;
	else if (link->wlan_mode_bitmap & WLAN_MD_11N)
		return 1;
	else
		return 0;
}

bool halbb_is_he_rate(struct bb_info *bb, u16 rate)
{
	return ((((rate & 0x1ff) >= BB_HE_1SS_MCS0) &&
		((rate & 0x1ff) <= BB_HE_1SS_MCS(11))) ||
		(((rate & 0x1ff) >= BB_HE_2SS_MCS0) &&
		((rate & 0x1ff) <= BB_HE_2SS_MCS(11))) ||
		(((rate & 0x1ff) >= BB_HE_3SS_MCS0) &&
		((rate & 0x1ff) <= BB_HE_3SS_MCS(11))) ||
		(((rate & 0x1ff) >= BB_HE_4SS_MCS0) &&
		((rate & 0x1ff) <= BB_HE_4SS_MCS(11)))) ? true : false;
}

bool halbb_is_vht_rate(struct bb_info *bb, u16 rate)
{
	return ((((rate & 0x1ff) >= BB_VHT_1SS_MCS0) &&
		((rate & 0x1ff) <= BB_VHT_1SS_MCS(9))) ||
		(((rate & 0x1ff) >= BB_VHT_2SS_MCS0) &&
		((rate & 0x1ff) <= BB_VHT_2SS_MCS(9))) ||
		(((rate & 0x1ff) >= BB_VHT_3SS_MCS0) &&
		((rate & 0x1ff) <= BB_VHT_3SS_MCS(9))) ||
		(((rate & 0x1ff) >= BB_VHT_4SS_MCS0) &&
		((rate & 0x1ff) <= BB_VHT_4SS_MCS(9)))) ? true : false;
}

bool halbb_is_ht_rate(struct bb_info *bb, u16 rate)
{
	return (((rate & 0x1ff) >= BB_HT_MCS0) &&
		((rate & 0x1ff) <= BB_HT_MCS(31))) ? true : false;
}

bool halbb_is_ofdm_rate(struct bb_info *bb, u16 rate)
{
	return (((rate & 0x1ff) >= BB_06M) &&
		((rate & 0x1ff) <= BB_54M)) ? true : false;
}

bool halbb_is_cck_rate(struct bb_info *bb, u16 rate)
{
	return ((rate & 0x1ff) <= BB_11M) ? true : false;
}

u8 halbb_legacy_rate_2_spec_rate(struct bb_info *bb, u16 rate)
{
	u8 rate_idx = 0x0;
	u8 legacy_spec_rate_t[8] = {BB_SPEC_RATE_6M, BB_SPEC_RATE_9M,
				    BB_SPEC_RATE_12M, BB_SPEC_RATE_18M,
				    BB_SPEC_RATE_24M, BB_SPEC_RATE_36M,
				    BB_SPEC_RATE_48M, BB_SPEC_RATE_54M};

	rate_idx = rate - BB_06M;
	return legacy_spec_rate_t[rate_idx];
}

u64 halbb_mgnt_2_rate_mask(u8 rate)
{
	u64 ret = 0;

	/*exclude BSS basic bit*/
	rate &= 0x7f;

	/*a/b/g mode only, there is no requirement for n/ac/ax mode*/
	if (rate > 108)
		return ret;

	/*unit:0.5Mbps*/
	switch (rate) {
	case 2:
		ret = BIT(BB_01M);
		break;
	case 4:
		ret = BIT(BB_02M);
		break;
	case 11:
		ret = BIT(BB_05M);
		break;
	case 22:
		ret = BIT(BB_11M);
		break;
	case 12:
		ret = BIT(BB_06M);
		break;
	case 18:
		ret = BIT(BB_09M);
		break;
	case 24:
		ret = BIT(BB_12M);
		break;
	case 36:
		ret = BIT(BB_18M);
		break;
	case 48:
		ret = BIT(BB_24M);
		break;
	case 72:
		ret = BIT(BB_36M);
		break;
	case 96:
		ret = BIT(BB_48M);
		break;
	case 108:
		ret = BIT(BB_54M);
		break;
	default:
		break;
	}

	return ret;
}

u8 halbb_mcs_ss_to_fw_rate_idx(u8 mode, u8 mcs, u8 ss)
{
	u8 fw_rate_idx = 0;

	if (mode == 3) {
		fw_rate_idx = RATE_HE1SS_MCS0 + (ss - 1) * 12; // MAX HE MCS is MCS11 (Totally, 12 MCS index)
	} else if (mode == 2) {
		fw_rate_idx = RATE_VHT1SS_MCS0 + (ss - 1) * 10; // MAX VHT MCS is MCS9 (Totally, 10 MCS index)
	} else if (mode == 1) { // HT
		fw_rate_idx = RATE_HT_MCS0 + (ss - 1) * 8;
	} else {
		fw_rate_idx = 0;
	}

	return fw_rate_idx;
}

void halbb_rate_idx_parsor(struct bb_info *bb, u16 rate_idx, enum rtw_gi_ltf gi_ltf, struct bb_rate_info *ra_i)
{
	ra_i->rate_idx_all = rate_idx | (((u16)gi_ltf & 0xf) << 12);
	ra_i->rate_idx = rate_idx;
	ra_i->gi_ltf = gi_ltf;
	if (bb->bb_80211spec == BB_AX_IC)
		ra_i->mode = (enum bb_mode_type)((rate_idx & 0x180) >> 7);
	else
		ra_i->mode = (enum bb_mode_type)((rate_idx & 0xf00) >> 8);

	if (ra_i->mode == BB_LEGACY_MODE) {
		ra_i->ss = 1;
		ra_i->idx = rate_idx & 0x1f;
	} else if (ra_i->mode == BB_HT_MODE) {
		ra_i->ss = ((rate_idx & 0x18) >> 3) + 1;
		ra_i->idx = rate_idx & 0x1f;
	} else if (ra_i->mode == BB_EHT_MODE) {
		ra_i->ss = ((rate_idx & 0xe0) >> 5) + 1;
		ra_i->idx = rate_idx & 0x1f;
	} else { /*VHT,HE*/
		if (bb->ic_type & BB_IC_AX_SERIES)
			ra_i->ss = ((rate_idx & 0x70) >> 4) + 1;
		else
			ra_i->ss = ((rate_idx & 0xe0) >> 5) + 1;

		ra_i->idx = rate_idx & 0xf;
	}

	/* Transfer to fw used rate_idx*/
	if (ra_i->mode == BB_LEGACY_MODE) {
		ra_i->fw_rate_idx = ra_i->idx;
		return;
	}

	ra_i->fw_rate_idx = halbb_mcs_ss_to_fw_rate_idx(ra_i->mode, ra_i->idx, ra_i->ss);

	return;
}

u8 halbb_rate_2_rate_digit(struct bb_info *bb, u16 rate)
{
	u8 legacy_table[12] = {1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54};
	u8 rate_digit = 0;

	if (rate >= BB_HE_8SS_MCS0)
		rate_digit = (rate - BB_HE_8SS_MCS0);
	else if (rate >= BB_HE_7SS_MCS0)
		rate_digit = (rate - BB_HE_7SS_MCS0);
	else if (rate >= BB_HE_6SS_MCS0)
		rate_digit = (rate - BB_HE_6SS_MCS0);
	else if (rate >= BB_HE_5SS_MCS0)
		rate_digit = (rate - BB_HE_5SS_MCS0);
	else if (rate >= BB_HE_4SS_MCS0)
		rate_digit = (rate - BB_HE_4SS_MCS0);
	else if (rate >= BB_HE_3SS_MCS0)
		rate_digit = (rate - BB_HE_3SS_MCS0);
	else if (rate >= BB_HE_2SS_MCS0)
		rate_digit = (rate - BB_HE_2SS_MCS0);
	else if (rate >= BB_HE_1SS_MCS0)
		rate_digit = (rate - BB_HE_1SS_MCS0);
	else if (rate >= BB_VHT_8SS_MCS0)
		rate_digit = (rate - BB_VHT_8SS_MCS0);
	else if (rate >= BB_VHT_7SS_MCS0)
		rate_digit = (rate - BB_VHT_7SS_MCS0);
	else if (rate >= BB_VHT_6SS_MCS0)
		rate_digit = (rate - BB_VHT_6SS_MCS0);
	else if (rate >= BB_VHT_5SS_MCS0)
		rate_digit = (rate - BB_VHT_5SS_MCS0);
	else if (rate >= BB_VHT_4SS_MCS0)
		rate_digit = (rate - BB_VHT_4SS_MCS0);
	else if (rate >= BB_VHT_3SS_MCS0)
		rate_digit = (rate - BB_VHT_3SS_MCS0);
	else if (rate >= BB_VHT_2SS_MCS0)
		rate_digit = (rate - BB_VHT_2SS_MCS0);
	else if (rate >= BB_VHT_1SS_MCS0)
		rate_digit = (rate - BB_VHT_1SS_MCS0);
	else if (rate >= BB_HT_MCS0)
		rate_digit = (rate - BB_HT_MCS0);
	else if (rate <= BB_54M)
		rate_digit = legacy_table[rate];

	return rate_digit;
}

/*u8 halbb_get_rx_stream_num(struct bb_info *bb, enum rf_type type)
{
	u8 rx_num = 1;

	if (type == RF_1T1R)
		rx_num = 1;
	else if (type == RF_2T2R || type == RF_1T2R)
		rx_num = 2;
	else if (type == RF_3T3R || type == RF_2T3R)
		rx_num = 3;
	else if (type == RF_4T4R || type == RF_3T4R || type == RF_2T4R)
		rx_num = 4;
	else
		BB_WARNING("%s\n", __func__);

	return rx_num;
}*/

u8 halbb_rate_type_2_num_ss(struct bb_info *bb, enum halbb_rate_type type)
{
	u8 num_ss = 1;

	switch (type) {
	case BB_CCK:
	case BB_OFDM:
	case BB_1SS:
		num_ss = 1;
		break;
	case BB_2SS:
		num_ss = 2;
		break;
	case BB_3SS:
		num_ss = 3;
		break;
	case BB_4SS:
		num_ss = 4;
		break;
	default:
		break;
	}

	return num_ss;
}

u8 halbb_rate_to_num_ss(struct bb_info *bb, u16 rate)
{
	u8 num_ss = 1;

	if (rate <= BB_54M)
		num_ss = 1;
	else if (rate <= BB_HT_MCS(31))
		num_ss = ((rate - BB_HT_MCS0) >> 3) + 1;
	else if (rate <= BB_VHT_1SS_MCS(9))
		num_ss = 1;
	else if (rate <= BB_VHT_2SS_MCS(9))
		num_ss = 2;
	else if (rate <= BB_VHT_3SS_MCS(9))
		num_ss = 3;
	else if (rate <= BB_VHT_4SS_MCS(9))
		num_ss = 4;
	else if (rate <= BB_VHT_5SS_MCS(9))
		num_ss = 5;
	else if (rate <= BB_VHT_6SS_MCS(9))
		num_ss = 6;
	else if (rate <= BB_VHT_7SS_MCS(9))
		num_ss = 7;
	else if (rate <= BB_VHT_8SS_MCS(9))
		num_ss = 8;
	else if (rate <= BB_HE_1SS_MCS(11))
		num_ss = 1;
	else if (rate <= BB_HE_2SS_MCS(11))
		num_ss = 2;
	else if (rate <= BB_HE_3SS_MCS(11))
		num_ss = 3;
	else if (rate <= BB_HE_4SS_MCS(11))
		num_ss = 4;
	else if (rate <= BB_HE_5SS_MCS(11))
		num_ss = 5;
	else if (rate <= BB_HE_6SS_MCS(11))
		num_ss = 6;
	else if (rate <= BB_HE_7SS_MCS(11))
		num_ss = 7;
	else if (rate <= BB_HE_8SS_MCS(11))
		num_ss = 8;

	return num_ss;
}

u8 halbb_ss_mcs_2_mcs_ss_idx(struct bb_info *bb, enum bb_mode_type mode, u8 ss, u8 mcs_idx)
{
	u8 mcs_ss_idx = 0;

	if (mode == BB_LEGACY_MODE) {
		mcs_ss_idx = mcs_idx;
	} else if (mode == BB_HT_MODE) {
		mcs_ss_idx = (mcs_idx & 0x7) | (ss << 3);
	} else if ((mode == BB_VHT_MODE) || (mode == BB_HE_MODE)) {
		if (bb->ic_type & BB_IC_AX_SERIES)
			mcs_ss_idx = (mcs_idx & 0xf) | (ss << 4);
		else
			mcs_ss_idx = (mcs_idx & 0xf) | (ss << 5);
	} else if (mode == BB_EHT_MODE) {
		mcs_ss_idx = (mcs_idx & 0x1f) | (ss << 5);
	}

	return mcs_ss_idx;
}

u16 halbb_gen_rate_idx(struct bb_info *bb, enum bb_mode_type mode, u8 ss, u8 idx)
{
	u16 rate_idx = 0;

	if (bb->bb_80211spec == BB_BE_IC) {
		if (mode == BB_EHT_MODE) {
			rate_idx = BB_BE_EHT_MCS(ss, idx);
		} else if (mode == BB_HE_MODE) {
			rate_idx = BB_BE_HE_MCS(ss, idx);
		} else if (mode == BB_VHT_MODE) {
			rate_idx = BB_BE_VHT_MCS(ss, idx);
		} else if (mode == BB_HT_MODE) {
			rate_idx = BB_BE_HT_MCS(idx);
		}
	} else {
		if (mode == BB_HE_MODE) {
			rate_idx = BB_HE_MCS(ss, idx);
		} else if (mode == BB_VHT_MODE) {
			rate_idx = BB_VHT_MCS(ss, idx);
		} else if (mode == BB_HT_MODE) {
			rate_idx = BB_HT_MCS(idx);
		} else {
			rate_idx = idx;
		}
	}
#if 0
	BB_DBG(bb, DBG_BIT17, "rate_idx=0x%x, mode=%d, ss=%d, idx=%d\n", rate_idx, mode, ss, idx);

	/*RX Rate*/
	halbb_print_rate_2_buff(bb, rate_idx, RTW_GILTF_LGI_4XHE32, bb->dbg_buf, HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_BIT17, "Rate:%s\n", bb->dbg_buf);
#endif
	return rate_idx;
}

void halbb_print_rate_2_buff(struct bb_info *bb, u16 rate_idx, enum rtw_gi_ltf gi_ltf, char *buf, u16 buf_size)
{
	struct bb_rate_info rate;
	char *ss = NULL;
	char *mode = NULL;
	char *gi = NULL;

	halbb_rate_idx_parsor(bb, rate_idx, gi_ltf, &rate);

	if (rate.mode == BB_HE_MODE)
		mode = "HE ";
	else if (rate.mode == BB_VHT_MODE)
		mode = "VHT ";
	else if (rate.mode == BB_HT_MODE)
		mode = "HT";
	else if (rate.mode == BB_EHT_MODE)
		mode = "EHT";
	else
		mode = "";

	if (rate.ss == 4)
		ss = "4";
	else if (rate.ss == 3)
		ss = "3";
	else if (rate.ss == 2)
		ss = "2";
	else
		ss = "1";

	if ((rate.mode == BB_HE_MODE) || (rate.mode == BB_EHT_MODE)) {
		if (rate.gi_ltf == RTW_GILTF_LGI_4XHE32)
			gi = "[4X32]";
		else if (rate.gi_ltf == RTW_GILTF_SGI_4XHE08)
			gi = "[4X08]";
		else if (rate.gi_ltf == RTW_GILTF_2XHE16)
			gi = "[2X16]";
		else if (rate.gi_ltf == RTW_GILTF_2XHE08)
			gi = "[2X08]";
		else if (rate.gi_ltf == RTW_GILTF_1XHE16)
			gi = "[1X16]";
		else
			gi = "[1X08]";
	} else if (rate.mode >= BB_HT_MODE) {
		if (rate.gi_ltf == RTW_GILTF_SGI_4XHE08)
			gi = "[sgi]";
		else
			gi = "";
	} else {
		gi = "";
	}

	/* BB_SNPRINTF wait driver porting */
	_os_snprintf(buf, buf_size, "(%s%s%s%s%d%s%s)",
		     mode,
		     (rate.mode >= BB_VHT_MODE) ? ss : "",
		     (rate.mode >= BB_VHT_MODE) ? "-ss " : "",
		     (rate.rate_idx >= BB_HT_MCS0) ? "MCS" : "",
		     (rate.rate_idx >= BB_HT_MCS0) ? rate.idx : bb_phy_rate_table[rate.idx],
		     gi,
		     (rate.rate_idx < BB_HT_MCS0) ? "M" : "");
}

enum bb_qam_type halbb_get_qam_order(struct bb_info *bb, u16 rate_idx)
{
	u16 tmp_idx = rate_idx;
	enum bb_qam_type qam_order = BB_QAM_BPSK;
	enum bb_qam_type qam[10] = {BB_QAM_BPSK, BB_QAM_QPSK,
				    BB_QAM_QPSK, BB_QAM_16QAM,
				    BB_QAM_16QAM, BB_QAM_64QAM,
				    BB_QAM_64QAM, BB_QAM_64QAM,
				    BB_QAM_256QAM, BB_QAM_256QAM};

	if (rate_idx <= BB_11M)
		return BB_QAM_CCK;

	if ((rate_idx >= BB_VHT_MCS(1, 0)) && (rate_idx <= BB_VHT_MCS(4, 9))) {
		if (rate_idx >= BB_VHT_MCS(4, 0))
			tmp_idx -= BB_VHT_MCS(4, 0);
		else if (rate_idx >= BB_VHT_MCS(3, 0))
			tmp_idx -= BB_VHT_MCS(3, 0);
		else if (rate_idx >= BB_VHT_MCS(2, 0))
			tmp_idx -= BB_VHT_MCS(2, 0);
		else
			tmp_idx -= BB_VHT_MCS(1, 0);

		if (tmp_idx < 10)
			qam_order = qam[tmp_idx];
	} else if ((rate_idx >= BB_HT_MCS(0)) && (rate_idx <= BB_HT_MCS(31))) {
		if (rate_idx >= BB_HT_MCS(24))
			tmp_idx -= BB_HT_MCS(24);
		else if (rate_idx >= BB_HT_MCS(16))
			tmp_idx -= BB_HT_MCS(16);
		else if (rate_idx >= BB_HT_MCS(8))
			tmp_idx -= BB_HT_MCS(8);
		else
			tmp_idx -= BB_HT_MCS(0);

		qam_order = qam[tmp_idx];
	} else {
		if ((rate_idx > BB_06M) && (rate_idx <= BB_54M)) {
			tmp_idx -= BB_06M;
			qam_order = qam[tmp_idx - 1];
		} else { /* OFDM 6M & all other undefine rate*/
			qam_order = BB_QAM_BPSK;
		}
	}
	return qam_order;
}

u8 halbb_rate_order_compute(struct bb_info *bb, u16 rate_idx)
{
	u16 rate_order = rate_idx & 0x7f;

	rate_idx &= 0x1FF; /* AX: 0x180 & 0x7F */

	if (rate_idx >= BB_VHT_MCS(4, 0))
		rate_order -= BB_VHT_MCS(4, 0);
	else if (rate_idx >= BB_VHT_MCS(3, 0))
		rate_order -= BB_VHT_MCS(3, 0);
	else if (rate_idx >= BB_VHT_MCS(2, 0))
		rate_order -= BB_VHT_MCS(2, 0);
	else if (rate_idx >= BB_VHT_MCS(1, 0))
		rate_order -= BB_VHT_MCS(1, 0);
	else if (rate_idx >= BB_HT_MCS(24))
		rate_order -= BB_HT_MCS(24);
	else if (rate_idx >= BB_HT_MCS(16))
		rate_order -= BB_HT_MCS(16);
	else if (rate_idx >= BB_HT_MCS(8))
		rate_order -= BB_HT_MCS(8);
	else if (rate_idx >= BB_HT_MCS(0))
		rate_order -= BB_HT_MCS(0);
	else if (rate_idx >= BB_06M)
		rate_order -= BB_06M;
	else
		rate_order -= BB_01M;

	if (rate_idx >= BB_HT_MCS(0))
		rate_order++;

	return (u8)rate_order;
}

u8 halbb_init_ra_by_rssi(struct bb_info *bb, u8 rssi_assoc)
{
	u8 init_ra_lv = 0;

	if (rssi_assoc > 50)
		init_ra_lv = 1;
	else if (rssi_assoc > 30)
		init_ra_lv = 2;
	else if (rssi_assoc > 1)
		init_ra_lv = 3;
	else
		init_ra_lv = 0;

	return init_ra_lv;
}

bool halbb_set_csi_rate(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	u8 macid;
	union bb_h2c_ra_cmn_info *ra_cmn;
	struct bb_h2c_ra_cfg_info *ra_cfg;
	struct rtw_hal_stainfo_t *hal_sta_i;
	struct rtw_ra_sta_info * ra_sta_i;

	if (!phl_sta_i || !bb)
		return false;
	macid = (u8)(phl_sta_i->macid);
	hal_sta_i = phl_sta_i->hal_sta;
	if (!hal_sta_i)
		return false;
	ra_cmn = &bb->bb_cmn_hooker->bb_ra_i[macid].ra_cfg;
	ra_cfg = &ra_cmn->bb_h2c_ra_info;
	ra_sta_i = &(hal_sta_i->ra_info);
	if (!ra_sta_i)
		return false;
	/*if ((!ra_sta_i->ra_csi_rate_en) && (!ra_sta_i->fixed_csi_rate_en))
		return false;*/
	/* Set csi rate ctrl enable */
	ra_cfg->ramask[7] |= BIT(7);
	ra_cfg->ra_csi_rate_en = ra_sta_i->ra_csi_rate_en;
	ra_cfg->fixed_csi_rate_en = ra_sta_i->fixed_csi_rate_en;
	ra_cfg->cr_tbl_sel = bb->hal_com->csi_para_ctrl_sel;
	ra_cfg->band_num = phl_sta_i->rlink->hw_band;
	ra_cfg->fixed_csi_rate_l = halbb_ss_mcs_2_mcs_ss_idx(bb,
							     ra_sta_i->csi_rate.mode,
							     ra_sta_i->csi_rate.ss,
							     ra_sta_i->csi_rate.mcs_idx);

	BB_DBG(bb, DBG_RA,
	       "ra_csi_rate_en=%d, fixed_csi_rate_en=%d, cr_tbl_sel=%d, band_num=%d, fixed_csi_rate_l=%x\n",
	       ra_cfg->ra_csi_rate_en, ra_cfg->fixed_csi_rate_en,
	       ra_cfg->cr_tbl_sel, ra_cfg->band_num, ra_cfg->fixed_csi_rate_l);

	return true;
}

u8 halbb_rssi_lv_dec(struct bb_info *bb, u8 rssi, u8 ratr_state)
{
	/*@MCS0 ~ MCS4 , VHT1SS MCS0 ~ MCS4 , G 6M~24M*/
	/*u8 rssi_lv_t[RA_FLOOR_TABLE_SIZE] = {20, 34, 38, 42, 46, 50, 100};*/
	u8 rssi_lv_t[RA_FLOOR_TABLE_SIZE] = {30, 44, 48, 52, 56, 60, 100};
	/*@ RSSI definition changed in AX*/
	u8 new_rssi_lv = 0;
	u8 i;

	BB_DBG(bb, DBG_RA,
	       "curr RA level=(%d), Table_ori=[%d, %d, %d, %d, %d, %d]\n",
	       ratr_state, rssi_lv_t[0], rssi_lv_t[1], rssi_lv_t[2],
	       rssi_lv_t[3], rssi_lv_t[4], rssi_lv_t[5]);
	for (i = 0; i < RA_FLOOR_TABLE_SIZE; i++) {
		if (i >= (ratr_state))
			rssi_lv_t[i] += RA_FLOOR_UP_GAP;
	}
	BB_DBG(bb, DBG_RA,
	       "RSSI=(%d), Table_mod=[%d, %d, %d, %d, %d, %d]\n", rssi,
	       rssi_lv_t[0], rssi_lv_t[1], rssi_lv_t[2], rssi_lv_t[3],
	       rssi_lv_t[4], rssi_lv_t[5]);
	for (i = 0; i < RA_FLOOR_TABLE_SIZE; i++) {
		if (rssi < rssi_lv_t[i]) {
			new_rssi_lv = i;
			break;
		}
	}
	return new_rssi_lv;
}

u64 halbb_ramask_by_rssi(struct bb_info *bb, u8 rssi_lv, u64 ramask)
{
	u64 ra_mask_bitmap = ramask;

	if (rssi_lv == 0)
		ra_mask_bitmap &= 0xffffffffffffffff;
	else if (rssi_lv == 1)
		ra_mask_bitmap &= 0xfffffffffffffff0;
	else if (rssi_lv == 2)
		ra_mask_bitmap &= 0xffffffffffffefe0;
	else if (rssi_lv == 3)
		ra_mask_bitmap &= 0xffffffffffffcfc0;
	else if (rssi_lv == 4)
		ra_mask_bitmap &= 0xffffffffffff8f80;
	else if (rssi_lv >= 5)
		ra_mask_bitmap &= 0xffffffffffff0f00;

	/*Avoid empty HT/VHT/HE ramask when HT/VHT/HE mode is enabled*/
	if ((ra_mask_bitmap >> 12) == 0x0) {
		ra_mask_bitmap |= (ramask & 0xfffffffffffff000);
		BB_DBG(bb, DBG_RA,
		       "Empty HT/VHT/HE ramask! Bypass HT/VHT/HE ramask_by_rssi\n");
	}

	/*Avoid empty legacy ramask after foolproof of HT/VHT/HE mode*/
	if (ra_mask_bitmap == 0x0) {
		ra_mask_bitmap |= (ramask & 0xfff);
		BB_DBG(bb, DBG_RA,
		       "Empty ramask! Bypass a/b/g ramask_by_rssi\n");
	}

	return ra_mask_bitmap;
}

void halbb_update_low_latency_macid(struct bb_info *bb, u16 macid,
				    bool is_low_latency)
{
	struct bb_ra_info *bb_ra = &bb->bb_cmn_hooker->bb_ra_i[macid];

	bb_ra->is_low_latency = is_low_latency;
}

u64 halbb_ramask_mod(struct bb_info *bb, u8 macid, u64 ramask, u8 rssi, u8 mode,
	u8 nss, u64 *ramask_h)
{
	struct bb_ra_info *bb_ra = &bb->bb_cmn_hooker->bb_ra_i[macid];
	u64 mod_mask = ramask;
	u64 mod_mask_rssi = ramask;
	u8 new_rssi_lv = 0;
	u8 wifi_mode = RA_non_ht;

	if (mode == CCK_SUPPORT) { /* B mode */
		mod_mask &= RAMASK_B;
		wifi_mode = RA_CCK;
		BB_DBG(bb, DBG_RA, "RA mask B mode\n");
	} else if (mode == OFDM_SUPPORT) { /* AG mode */
		mod_mask &= RAMASK_AG;
		wifi_mode = RA_non_ht;
		BB_DBG(bb, DBG_RA, "RA mask A mode\n");
	} else if (mode == (CCK_SUPPORT|OFDM_SUPPORT)) {
		/* BG mode */
		mod_mask &= RAMASK_BG;
		wifi_mode = RA_non_ht;
		BB_DBG(bb, DBG_RA, "RA mask 2.4G BG mode\n");
	} else if (mode == (CCK_SUPPORT|OFDM_SUPPORT|HT_SUPPORT)) {
		/* 2G N mode */
		mod_mask &= RAMASK_HT_2G;
		wifi_mode = RA_HT;
		BB_DBG(bb, DBG_RA, "RA mask 2.4G HT mode\n");
	} else if (mode == (OFDM_SUPPORT|HT_SUPPORT)) {
		/* 5G N mode */
		mod_mask &= RAMASK_HT_5G;
		wifi_mode = RA_HT;
		BB_DBG(bb, DBG_RA, "RA mask 5G HT mode\n");
	} else if (mode == (CCK_SUPPORT|OFDM_SUPPORT|VHT_SUPPORT_TX)) {
		/* 2G AC mode */
		mod_mask &= RAMASK_VHT_2G;
		wifi_mode = RA_VHT;
		BB_DBG(bb, DBG_RA, "RA mask 2.4G VHT mode\n");
	} else if (mode == (OFDM_SUPPORT|VHT_SUPPORT_TX)) {
		/* 5G AC mode */
		mod_mask &= RAMASK_VHT_5G;
		wifi_mode = RA_VHT;
		BB_DBG(bb, DBG_RA, "RA mask 5G VHT mode\n");
	} else if (mode == (CCK_SUPPORT|OFDM_SUPPORT|HE_SUPPORT)) {
		/* 2G AX mode */
		mod_mask &= RAMASK_HE_2G;
		wifi_mode = RA_HE;
		BB_DBG(bb, DBG_RA, "RA mask 2.4G HE mode\n");
	} else if (mode == (OFDM_SUPPORT|HE_SUPPORT)) {
		/* 5G AX mode */
		mod_mask &= RAMASK_HE_5G;
		wifi_mode = RA_HE;
		BB_DBG(bb, DBG_RA, "RA mask 5G HE mode\n");
	} else if (mode == (CCK_SUPPORT|OFDM_SUPPORT|EHT_SUPPORT)) {
		/* 2G BE mode */
		mod_mask &= RAMASK_EHT_2G_L;
		*ramask_h &= RAMASK_EHT_H;
		wifi_mode = RA_EHT;
		BB_DBG(bb, DBG_RA, "RA mask 2.4G BE mode\n");
	} else if (mode == (OFDM_SUPPORT|EHT_SUPPORT)) {
		/* 5G BE mode */
		mod_mask &= RAMASK_EHT_5G_L;
		*ramask_h &= RAMASK_EHT_H;
		wifi_mode = RA_EHT;
		BB_DBG(bb, DBG_RA, "RA mask 5G BE mode\n");
	} else {
		BB_WARNING("[%s] Mode=%x\n", __func__, mode);
	}
	BB_DBG(bb, DBG_RA, "RA mask SS NUM : %d\n", nss);
	if (wifi_mode == RA_HT) {
		switch (nss) {
		case RA_1SS_MODE:
			mod_mask &= RAMASK_1SS_HT;
			break;
		case RA_2SS_MODE:
			mod_mask &= RAMASK_2SS_HT;
			break;
		case RA_3SS_MODE:
			mod_mask &= RAMASK_3SS_HT;
			break;
		case RA_4SS_MODE:
			mod_mask &= RAMASK_4SS_HT;
			break;
		default:
			mod_mask &= RAMASK_1SS_HT;
			break;
		}
	} else if (wifi_mode == RA_VHT) {
		switch (nss) {
		case RA_1SS_MODE:
			mod_mask &= RAMASK_1SS_VHT;
			break;
		case RA_2SS_MODE:
			mod_mask &= RAMASK_2SS_VHT;
			break;
		case RA_3SS_MODE:
			mod_mask &= RAMASK_3SS_VHT;
			break;
		case RA_4SS_MODE:
			mod_mask &= RAMASK_4SS_VHT;
			break;
		default:
			mod_mask &= RAMASK_1SS_VHT;
			break;
		}
	} else if (wifi_mode == RA_HE) {
		switch (nss) {
		case RA_1SS_MODE:
			mod_mask &= RAMASK_1SS_HE;
			break;
		case RA_2SS_MODE:
			mod_mask &= RAMASK_2SS_HE;
			break;
		case RA_3SS_MODE:
			mod_mask &= RAMASK_3SS_HE;
			break;
		case RA_4SS_MODE:
			mod_mask &= RAMASK_4SS_HE;
			break;
		default:
			mod_mask &= RAMASK_1SS_HE;
			break;
		}
	} else if (wifi_mode == RA_EHT) {
		switch (nss) {
		case RA_1SS_MODE:
			mod_mask &= RAMASK_1SS_EHT;
			*ramask_h = 0;
			break;
		case RA_2SS_MODE:
			mod_mask &= RAMASK_2SS_EHT;
			*ramask_h = 0;
			break;
		case RA_3SS_MODE:
			mod_mask &= RAMASK_3SS_EHT;
			*ramask_h = 0;
			break;
		case RA_4SS_MODE:
			mod_mask &= RAMASK_4SS_EHT_L;
			*ramask_h &= RAMASK_4SS_EHT_H;
			break;
		default:
			mod_mask &= RAMASK_1SS_EHT;
			*ramask_h = 0;
			break;
		}
	} else {
		BB_DBG(bb, DBG_RA, "RA mask non-ht mode\n");
	}
	BB_DBG(bb, DBG_RA, "RA mask modify : %llx\n", mod_mask);
	new_rssi_lv = halbb_rssi_lv_dec(bb, rssi, bb_ra->rssi_lv);

	if (bb_ra->rssi_lv != new_rssi_lv)
		bb_ra->rssi_lv = new_rssi_lv;

	if (wifi_mode != RA_CCK) {
		mod_mask_rssi = halbb_ramask_by_rssi(bb, new_rssi_lv, mod_mask);
		//BB_DBG(bb, DBG_RA, "RA mask modify by rssi : 0x%016llx\n", mod_mask_rssi);
	} else {
		mod_mask_rssi = mod_mask;
	}

	/*low latency RAmask, no 2ss MCS4/1ss MCS6 above*/
	if (bb_ra->is_low_latency)
		mod_mask_rssi &= 0x000000000f03ffff;

	return mod_mask_rssi;
}

void rtw_halbb_mudbg(struct bb_info *bb, u8 type, u8 mu_entry, u8 macid, 
			   bool en_256q, bool en_1024q)
{
	struct bb_h2c_mu_cfg mucfg = {0};
	u32 *bb_h2c = (u32 *)&mucfg;
	u8 cmdlen = sizeof(mucfg);

	mucfg.cmd_type = type;
	mucfg.entry = mu_entry;
	mucfg.macid = macid;
	halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_MUCFG, HALBB_H2C_RA, bb_h2c);
}

u64 halbb_gen_abg_mask(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	u64 abg_mask = 0;
	u8 i;
	struct protocol_cap_t *asoc_cap_i;

	if (!phl_sta_i)
		return 0;

	asoc_cap_i = &phl_sta_i->asoc_cap;

	BB_DBG(bb, DBG_RA,
	       "supported rates(L->H) = [%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x]\n",
	       asoc_cap_i->supported_rates[0], asoc_cap_i->supported_rates[1],
	       asoc_cap_i->supported_rates[2], asoc_cap_i->supported_rates[3],
	       asoc_cap_i->supported_rates[4], asoc_cap_i->supported_rates[5],
	       asoc_cap_i->supported_rates[6], asoc_cap_i->supported_rates[7],
	       asoc_cap_i->supported_rates[8], asoc_cap_i->supported_rates[9],
	       asoc_cap_i->supported_rates[10], asoc_cap_i->supported_rates[11]);
	for (i = 0; i < MAX_ABG_RATE_NUM; i++) {
		if (asoc_cap_i->supported_rates[i] == 0x0)
			continue;

		abg_mask |= halbb_mgnt_2_rate_mask(asoc_cap_i->supported_rates[i]);
	}
	BB_DBG(bb, DBG_RA, "gen_abgmask: 0x%llx\n", abg_mask);

	return abg_mask;
}

u64 halbb_gen_vht_mask(struct bb_info *bb,
				struct rtw_phl_stainfo_t *phl_sta_i)
{
	u8 vht_cap[2] = {0};
	u8 tmp_cap = 0;
	u8 cap_ss;
	u64 tmp_mask_nss = 0;
	u8 i;
	struct protocol_cap_t *asoc_cap_i;

	/*@Becareful RA use our "Tx" capability which means the capability of their "Rx"*/
	if (!phl_sta_i)
		return 0;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	vht_cap[0] = asoc_cap_i->vht_rx_mcs[0];
	vht_cap[1] = asoc_cap_i->vht_rx_mcs[1];
	BB_DBG(bb, DBG_RA, "%s : vhtcap:%x %x\n", __func__, vht_cap[0], vht_cap[1]);
	for (i = 0; i < MAX_NSS_VHT; i++) {
		if (i == 0)
			tmp_cap = vht_cap[0];
#if MAX_NSS_VHT > 4
		else if (i == 4)
			tmp_cap = vht_cap[1];
#endif

		cap_ss = tmp_cap & 0x03;
		tmp_cap = tmp_cap >> 2;
		if (cap_ss == 0)
			tmp_mask_nss |= ((u64)0xff << (i * 12));
		else if (cap_ss == 1)
			tmp_mask_nss |= ((u64)0x1ff << (i * 12));
		else if (cap_ss == 2)
			tmp_mask_nss |= ((u64)0x3ff << (i * 12));

#ifdef HALBB_CONFIG_HT2VHT_SUPPORT
		if (phl_sta_i->vht_2g_supported) {
			tmp_mask_nss |= ((u64)0x3ff << (i * 12));
		}
#endif

		BB_DBG(bb, DBG_RA, "gen_vhtmask:cap%x, ss%x, hemask: 0x%llx\n",
		       cap_ss, i, tmp_mask_nss);
	}
	return tmp_mask_nss;
}

u64 halbb_gen_ht_mask(struct bb_info *bb,
				struct rtw_phl_stainfo_t *phl_sta_i)
{
	u8 ht_cap[4] = {0};
	u64 cap_ss;
	u64 tmp_mask_nss = 0;
	u8 i;
	struct protocol_cap_t *asoc_cap_i;

	/*@Becareful RA use our "Tx" capability which means the capability of their "Rx"*/
	if (!phl_sta_i)
		return 0;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	for (i = 0; i < MAX_NSS_HT; i++)/* can use pointer after merge code*/
		ht_cap[i] = asoc_cap_i->ht_rx_mcs[i];
	BB_DBG(bb, DBG_RA, "%s : htcap: %x %x\n", __func__, ht_cap[0], ht_cap[1]);
	for (i = 0; i < MAX_NSS_HT; i++) {
		cap_ss = (u64)ht_cap[i];
		tmp_mask_nss = tmp_mask_nss | (cap_ss << (i * 12));
		BB_DBG(bb, DBG_RA, "gen_htmask:cap%llx, ss%x, htmask: 0x%llx\n", cap_ss, i, tmp_mask_nss);
	}

	return tmp_mask_nss;
}

u64 halbb_gen_he_mask(struct bb_info *bb,
				struct rtw_phl_stainfo_t *phl_sta_i, enum channel_width bw)
{
	u8 he_cap[2] = {0};
	u8 tmp_cap = 0;
	u8 cap_ss;
	u64 tmp_mask_nss = 0;
	u8 i;
	struct protocol_cap_t *asoc_cap_i;
	/*@Becareful RA use our "Tx" capability which means the capability of their "Rx"*/
	/*@In HE cap, mcs is correspond to channel bw"*/

	if (!phl_sta_i)
		return 0;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	if (bw == CHANNEL_WIDTH_80_80) {
		he_cap[0] = asoc_cap_i->he_rx_mcs[4];
		he_cap[1] = asoc_cap_i->he_rx_mcs[5];
	} else if (bw == CHANNEL_WIDTH_160) {
		he_cap[0] = asoc_cap_i->he_rx_mcs[2];
		he_cap[1] = asoc_cap_i->he_rx_mcs[3];
	} else {
		he_cap[0] = asoc_cap_i->he_rx_mcs[0];
		he_cap[1] = asoc_cap_i->he_rx_mcs[1];
	}
	BB_DBG(bb, DBG_RA, "%s: hecap:%x %x\n", __func__, he_cap[0], he_cap[1]);
	for (i = 0; i < MAX_NSS_HE; i++) {
		if (i == 0)
			tmp_cap = he_cap[0];
#if MAX_NSS_HE > 4
		else if (i == 4)
			tmp_cap = he_cap[1];
#endif
		cap_ss = tmp_cap & 0x03;
		tmp_cap = tmp_cap >> 2;
		if (cap_ss == 0)
			tmp_mask_nss |= ((u64)0xff << (i * 12));
		else if (cap_ss == 1)
			tmp_mask_nss |= ((u64)0x3ff << (i * 12));
		else if (cap_ss == 2)
			tmp_mask_nss |= ((u64)0xfff << (i * 12));

		BB_DBG(bb, DBG_RA, "gen_hemask:cap%x, ss%x, hemask: 0x%llx\n",
		       cap_ss, i, tmp_mask_nss);
	}
	return tmp_mask_nss;
}

#ifdef BB_1115_DVLP_SPF
u64 halbb_gen_eht_mask(struct bb_info *bb,
				struct rtw_phl_stainfo_t *phl_sta_i, enum channel_width bw)
{
	u8 eht_cap[4] = {0};
	u8 tmp_cap = 0;
	u32 cap_mcs[4] = {0xff, 0x3ff, 0xfff, 0x3fff};
	u64 tmp_mask_nss = 0;
	enum halbb_drv_type drv_role = bb->bb_cmn_hooker->bb_drv_type;
	u8 i,j;
	struct protocol_cap_t *asoc_cap_i;
	/*@Becareful RA use our "Tx" capability which means the capability of their "Rx"*/
	/*@In EHT cap, mcs is correspond to channel bw"*/

	if (!phl_sta_i)
		return 0;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	if (drv_role == BB_AP_DRV && bw == CHANNEL_WIDTH_20) {
		eht_cap[0] = asoc_cap_i->eht_mcs[0];
		eht_cap[1] = asoc_cap_i->eht_mcs[1];
		eht_cap[2] = asoc_cap_i->eht_mcs[2];
		eht_cap[3] = asoc_cap_i->eht_mcs[3];
	} else if (bw <= CHANNEL_WIDTH_80) {
		eht_cap[1] = asoc_cap_i->eht_mcs[4];
		eht_cap[2] = asoc_cap_i->eht_mcs[5];
		eht_cap[3] = asoc_cap_i->eht_mcs[6];
	} else if (bw == CHANNEL_WIDTH_160 || bw == CHANNEL_WIDTH_80_80) {
		eht_cap[1] = asoc_cap_i->eht_mcs[7];
		eht_cap[2] = asoc_cap_i->eht_mcs[8];
		eht_cap[3] = asoc_cap_i->eht_mcs[9];
	} else {
		eht_cap[1] = asoc_cap_i->eht_mcs[10];
		eht_cap[2] = asoc_cap_i->eht_mcs[11];
		eht_cap[3] = asoc_cap_i->eht_mcs[12];
	}
	BB_DBG(bb, DBG_RA, "%s: ehtcap:%x %x %x %x\n", __func__,
		   eht_cap[0], eht_cap[1], eht_cap[2], eht_cap[3]);

	for (i = 0; i < MAX_CAP_EHT; i++) {
		tmp_cap = (eht_cap[i] >> 4) & 0xf;
		for (j = 0; j < tmp_cap; j++) {
			tmp_mask_nss |= (cap_mcs[i] << (j * 16));
			BB_DBG(bb, DBG_RA, "gen_ehtmask:cap: %x, ss: %x, ehtmask: 0x%llx\n",
				   cap_mcs[i], j, tmp_mask_nss);
		}
	}
	return tmp_mask_nss;
}

bool halbb_is_eht_rate_wifi7(struct bb_info *bb, u16 rate)
{
	return ((((rate & 0xfff) >= BE_BB_EHT_1SS_MCS0) &&
		((rate & 0xfff) <= BB_EHT_1SS_MCS(15))) ||
		(((rate & 0xfff) >= BE_BB_EHT_2SS_MCS0) &&
		((rate & 0xfff) <= BB_EHT_2SS_MCS(15))) ||
		(((rate & 0xfff) >= BE_BB_EHT_3SS_MCS0) &&
		((rate & 0xfff) <= BB_EHT_3SS_MCS(15))) ||
		(((rate & 0xfff) >= BE_BB_EHT_4SS_MCS0) &&
		((rate & 0xfff) <= BB_EHT_4SS_MCS(15)))) ? true : false;
}

bool halbb_is_he_rate_wifi7(struct bb_info *bb, u16 rate)
{
	return ((((rate & 0xfff) >= BE_BB_HE_1SS_MCS0) &&
		((rate & 0xfff) <= BB_HE_1SS_MCS(11))) ||
		(((rate & 0xfff) >= BE_BB_HE_2SS_MCS0) &&
		((rate & 0xfff) <= BB_HE_2SS_MCS(11))) ||
		(((rate & 0xfff) >= BE_BB_HE_3SS_MCS0) &&
		((rate & 0xfff) <= BB_HE_3SS_MCS(11))) ||
		(((rate & 0xfff) >= BE_BB_HE_4SS_MCS0) &&
		((rate & 0xfff) <= BB_HE_4SS_MCS(11)))) ? true : false;
}

bool halbb_is_vht_rate_wifi7(struct bb_info *bb, u16 rate)
{
	return ((((rate & 0xfff) >= BE_BB_VHT_1SS_MCS0) &&
		((rate & 0xfff) <= BB_VHT_1SS_MCS(9))) ||
		(((rate & 0xfff) >= BE_BB_VHT_2SS_MCS0) &&
		((rate & 0xfff) <= BB_VHT_2SS_MCS(9))) ||
		(((rate & 0xfff) >= BE_BB_VHT_3SS_MCS0) &&
		((rate & 0xfff) <= BB_VHT_3SS_MCS(9))) ||
		(((rate & 0xfff) >= BE_BB_VHT_4SS_MCS0) &&
		((rate & 0xfff) <= BB_VHT_4SS_MCS(9)))) ? true : false;
}

bool halbb_is_ht_rate_wifi7(struct bb_info *bb, u16 rate)
{
	return (((rate & 0xfff) >= BE_BB_HT_MCS0) &&
		((rate & 0xfff) <= BB_HT_MCS(31))) ? true : false;
}

#endif

bool halbb_chk_bw_under_20(struct bb_info *bb, u8 bw)
{
	bool ret_val = true;

	switch (bw) {
	case CHANNEL_WIDTH_5:
	case CHANNEL_WIDTH_10:
	case CHANNEL_WIDTH_20:
		ret_val = true;
		break;
	case CHANNEL_WIDTH_40:
	case CHANNEL_WIDTH_80:
	case CHANNEL_WIDTH_80_80:
	case CHANNEL_WIDTH_160:
		ret_val = false;
		break;
	default:
		ret_val = false;
		break;
	}
	return ret_val;
}

bool halbb_chk_bw_under_40(struct bb_info *bb, u8 bw)
{
	bool ret_val = true;

	switch (bw) {
	case CHANNEL_WIDTH_5:
	case CHANNEL_WIDTH_10:
	case CHANNEL_WIDTH_20:
	case CHANNEL_WIDTH_40:
		ret_val = true;
		break;
	case CHANNEL_WIDTH_80:
	case CHANNEL_WIDTH_80_80:
	case CHANNEL_WIDTH_160:
		ret_val = false;
		break;
	default:
		ret_val = false;
		break;
	}
	return ret_val;
}

bool halbb_hw_bw_mode_chk(struct bb_info *bb, u8 bw, u8 hw_mode)
{
	bool ret_val = true;

	switch (hw_mode) {
	case CCK_SUPPORT:
	case OFDM_SUPPORT:
	case (CCK_SUPPORT | OFDM_SUPPORT):
		ret_val = halbb_chk_bw_under_20(bb, bw);
		break;
	case HT_SUPPORT:
	case (HT_SUPPORT | CCK_SUPPORT):
	case (HT_SUPPORT | OFDM_SUPPORT):
	case (HT_SUPPORT | OFDM_SUPPORT | CCK_SUPPORT):
		ret_val = halbb_chk_bw_under_40(bb, bw);
		break;
	case VHT_SUPPORT_TX:
	case (VHT_SUPPORT_TX | CCK_SUPPORT):
	case (VHT_SUPPORT_TX | OFDM_SUPPORT):
	case (VHT_SUPPORT_TX | OFDM_SUPPORT | CCK_SUPPORT):
	case HE_SUPPORT:
	case (HE_SUPPORT | CCK_SUPPORT):
	case (HE_SUPPORT | OFDM_SUPPORT):
	case (HE_SUPPORT | OFDM_SUPPORT | CCK_SUPPORT):
		ret_val = true;
		break;
	default:
		ret_val = true;
		break;
	}
	if (!ret_val)
		BB_WARNING("[%s]\n", __func__);
	return ret_val;
}

u8 halbb_hw_bw_mapping(struct bb_info *bb, u8 bw, u8 hw_mode)
{
	u8 hw_bw_map = CHANNEL_WIDTH_20;
	bool ret_val;

	if (bw <= CHANNEL_WIDTH_80)
		hw_bw_map = bw;
	else if (bw == CHANNEL_WIDTH_160 || bw == CHANNEL_WIDTH_80_80)
		hw_bw_map = CHANNEL_WIDTH_160;
#ifdef BB_1115_DVLP_SPF
	else if (bw == CHANNEL_WIDTH_320)
		hw_bw_map = CHANNEL_WIDTH_320;
#endif
	else
		hw_bw_map = CHANNEL_WIDTH_20;
	ret_val = halbb_hw_bw_mode_chk(bb, bw, hw_mode);
	return hw_bw_map;
}

u8 halbb_hw_mode_mapping(struct bb_info *bb, u8 wifi_mode, struct rtw_phl_stainfo_t *phl_sta_i)
{
	u8 hw_mode_map = 0;
	/* Driver wifi mode mapping */

#ifdef HALBB_CONFIG_HT2VHT_SUPPORT
	if (phl_sta_i->vht_2g_supported) {
		wifi_mode |= WLAN_MD_11AC;
	}
#endif

	if (wifi_mode & WLAN_MD_11B) /*11B*/
		hw_mode_map |= CCK_SUPPORT;
	if ((wifi_mode & WLAN_MD_11A) || (wifi_mode & WLAN_MD_11G)) /*11G, 11A*/
		hw_mode_map |= OFDM_SUPPORT;

	/* To prevent unnecessary mode from driver, causing confusing ra mask selection after then*/
#ifdef BB_1115_DVLP_SPF
	if (wifi_mode & WLAN_MD_11BE) /*11BE*/
		hw_mode_map |= EHT_SUPPORT;
	else if (wifi_mode & WLAN_MD_11AX) /*11AX*/
#else
	if (wifi_mode & WLAN_MD_11AX) /*11AX*/
#endif
		hw_mode_map |= HE_SUPPORT;
	else if (wifi_mode & WLAN_MD_11AC) /*11AC*/
		hw_mode_map |= VHT_SUPPORT_TX;
	else if (wifi_mode & WLAN_MD_11N) /* 11_N*/
		hw_mode_map |= HT_SUPPORT;

	if (hw_mode_map == 0)
		BB_WARNING("[%s] hw_mode_map == 0\n", __func__);

	BB_DBG(bb, DBG_RA, "drv_wifi_mode(0x%x) -> bb_wifi_mode(0x%x)\n", wifi_mode, hw_mode_map);
	return hw_mode_map;
}

bool halbb_ac_n_sgi_chk(struct bb_info *bb, u8 hw_mode, bool en_sgi)
{
	bool sgi_chk = en_sgi;

	/* Driver wifi mode mapping */
	if (hw_mode & HE_SUPPORT) {
		sgi_chk = false;
		BB_DBG(bb, DBG_RA, "HE mode sgi is not used!\n");
	}
	return sgi_chk;
}

u8 halbb_ax_giltf_chk(struct bb_info *bb, u8 hw_mode, u8 giltf_cap)
{
	u8 giltf_chk = giltf_cap;
	
	/* Driver wifi mode mapping */
	if (!(hw_mode & HE_SUPPORT)) {
		giltf_chk = 0;
		BB_DBG(bb, DBG_RA, "In non-HE mode gi_ltf is not used!\n");
	}
	return giltf_chk;
}

bool halbb_ldpc_chk(struct bb_info *bb,  struct rtw_phl_stainfo_t *phl_sta_i, u8 hw_mode)
{
	struct protocol_cap_t *asoc_cap_i;
	bool ldpc_en = false;

	/* Driver wifi mode mapping */
	if (!phl_sta_i)
		return false;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	if (hw_mode & HE_SUPPORT) {
		ldpc_en |= asoc_cap_i->he_ldpc;
		BB_DBG(bb, DBG_RA, "Enable HE LDPC\n");
	} else if (hw_mode&VHT_SUPPORT_TX) {
		ldpc_en |= asoc_cap_i->vht_ldpc;
		BB_DBG(bb, DBG_RA, "Enable VHT LDPC\n");
	} else if  (hw_mode&HT_SUPPORT) {
		ldpc_en |= asoc_cap_i->ht_ldpc;
		BB_DBG(bb, DBG_RA, "Enable HT LDPC\n");
	}
	return ldpc_en;
}

u8 halbb_nss_mapping(struct bb_info *bb, u8 nss_cap, u8 nss_limit)
{
	u8 mapping_nss = 0;

	/* Driver tx_nss mapping */
	if (nss_cap != 0)
		mapping_nss = nss_cap - 1;

	/*nss_limit. e.g. BTC_1ss feature*/
	if (nss_limit != 0) { /*nss_limit = 0 means no limit*/
		if (mapping_nss > nss_limit - 1)
			mapping_nss = nss_limit - 1;
	}

	/*protection when AP/NIC is connected to a device whose rx_nss > 2*/
	if (mapping_nss > (bb->phl_com->phy_cap[bb->bb_phy_idx].tx_path_num - 1)) {
		mapping_nss = bb->phl_com->phy_cap[bb->bb_phy_idx].tx_path_num - 1;
	}

	BB_DBG(bb, DBG_RA, "nss_mapping:{nss_cap,nss_limit,rfpath_tx} = {%d,%d,%d}\n",
	       nss_cap, nss_limit, bb->phl_com->phy_cap[bb->bb_phy_idx].tx_path_num);


	return mapping_nss;
}

bool halbb_stbc_mapping(struct bb_info *bb,  struct rtw_phl_stainfo_t *phl_sta_i, u8 hw_mode)
{
	struct protocol_cap_t *asoc_cap_i;
	bool stbc_en = false;

	/* Driver wifi mode mapping */
	if (!phl_sta_i)
		return false;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	if (hw_mode & HE_SUPPORT) {
		BB_DBG(bb, DBG_RA, "Iot id = %x\n", bb->phl_com->id.id);
		if ((((bb->phl_com->id.id & 0xF8) >> 3) == 1) &&
			(((bb->phl_com->id.id & 0x1F0000)>> 16) == 4)) {
			stbc_en = false;
			BB_DBG(bb, DBG_RA, "HE STBC off by Aruba AP = %d\n", stbc_en);
			return stbc_en;
		}
		if (asoc_cap_i->stbc_he_rx != 0)
			stbc_en = true;
		BB_DBG(bb, DBG_RA, "HE STBC %d\n", stbc_en);
	} else if (hw_mode & VHT_SUPPORT_TX) {
		if (asoc_cap_i->stbc_vht_rx != 0)
			stbc_en = true;
		BB_DBG(bb, DBG_RA, "VHT STBC %d\n", stbc_en);
	} else if  (hw_mode & HT_SUPPORT) {
		if (asoc_cap_i->stbc_ht_rx != 0)
			stbc_en = true;
		BB_DBG(bb, DBG_RA, "HT STBC %d\n", stbc_en);
	}
	return stbc_en;
}

bool halbb_sgi_chk(struct bb_info *bb,  struct rtw_phl_stainfo_t *phl_sta_i, u8 hw_bw)
{
	struct protocol_cap_t *asoc_cap_i;
	bool sgi_en = false;

	/* Driver wifi mode mapping */
	if (!phl_sta_i)
		return false;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	if (hw_bw == CHANNEL_WIDTH_20) {
		sgi_en = asoc_cap_i->sgi_20;
		BB_DBG(bb, DBG_RA, "Enable 20M SGI\n");
	} else if (hw_bw == CHANNEL_WIDTH_40) {
		sgi_en = asoc_cap_i->sgi_40;
		BB_DBG(bb, DBG_RA, "Enable 40M SGI\n");
	} else if  (hw_bw == CHANNEL_WIDTH_80) {
		sgi_en = asoc_cap_i->sgi_80;
		BB_DBG(bb, DBG_RA, "Enable 80M SGI\n");
	} else if  (hw_bw == CHANNEL_WIDTH_160) {
		sgi_en = asoc_cap_i->sgi_160;
		BB_DBG(bb, DBG_RA, "Enable 160M SGI\n");
	}
	return sgi_en;
}

void halbb_ramask_trans(struct bb_info *bb, u8 macid, u64 mask, u64 mask_h)
{
	union bb_h2c_ra_cmn_info *ra_cmn = &bb->bb_cmn_hooker->bb_ra_i[macid].ra_cfg;
	struct bb_h2c_ra_cfg_info *ra_cfg = &ra_cmn->bb_h2c_ra_info;

	ra_cfg->ramask[0] = (u8)(mask & 0x00000000000000ff);
	ra_cfg->ramask[1] = (u8)((mask & 0x000000000000ff00)>>8);
	ra_cfg->ramask[2] = (u8)((mask & 0x0000000000ff0000)>>16);
	ra_cfg->ramask[3] = (u8)((mask & 0x00000000ff000000)>>24);
	ra_cfg->ramask[4] = (u8)((mask & 0x000000ff00000000)>>32);
	ra_cfg->ramask[5] = (u8)((mask & 0x0000ff0000000000)>>40);
	ra_cfg->ramask[6] = (u8)((mask & 0x00ff000000000000)>>48);
	ra_cfg->ramask[7] = (u8)((mask & 0xff00000000000000)>>56);
	if (bb->ic_type & BB_IC_BE_SERIES) {
		ra_cmn->bb_h2c_ra_info_wifi7.ramask[0] = (u8)(mask_h & 0x00000000000000ff);
		ra_cmn->bb_h2c_ra_info_wifi7.ramask[1] = (u8)((mask_h & 0x000000000000ff00)>>8);
		ra_cmn->bb_h2c_ra_info_wifi7.ramask[2] = (u8)((mask_h & 0x0000000000ff0000)>>16);
		ra_cmn->bb_h2c_ra_info_wifi7.ramask[3] = (u8)((mask_h & 0x00000000ff000000)>>24);
	}
}

u8 halbb_get_opt_giltf(struct bb_info *bb, u8 assoc_giltf)
{
	u8 i =0;
	u8 opt_gi_ltf = 0;

	if (assoc_giltf & BIT(1)) /* cap. for 4x0.8*/
		opt_gi_ltf |= BIT(BB_OPT_GILTF_4XHE08);
	else if (assoc_giltf & BIT(5)) /* cap. for 1x0.8*/
		opt_gi_ltf |= BIT(BB_OPT_GILTF_1XHE08);

	BB_DBG(bb, DBG_RA, "Ass GILTF=%x,opt GILTF=%x\n", assoc_giltf, opt_gi_ltf);
	return opt_gi_ltf;
}

u8 halbb_giltf_trans(struct bb_info *bb, u8 assoc_giltf, u8 cal_giltf)
{
	u8 i =0;

	BB_DBG(bb, DBG_RA, "Ass GILTF=%x,Cal GILTF=%x\n", assoc_giltf, cal_giltf);
	if (cal_giltf == RTW_GILTF_LGI_4XHE32 && (assoc_giltf & BIT(0)))
		return cal_giltf;
	else if (cal_giltf == RTW_GILTF_SGI_4XHE08 && (assoc_giltf & BIT(1)))
		return cal_giltf;
	else if (cal_giltf == RTW_GILTF_2XHE16 && (assoc_giltf & BIT(2)))
		return cal_giltf;
	else if (cal_giltf == RTW_GILTF_2XHE08 && (assoc_giltf & BIT(3)))
		return cal_giltf;
	else if (cal_giltf == RTW_GILTF_1XHE16 && (assoc_giltf & BIT(4)))
		return cal_giltf;
	else if (cal_giltf == RTW_GILTF_1XHE08 && (assoc_giltf & BIT(5)))
		return cal_giltf;
	if (assoc_giltf & BIT(3))
		return RTW_GILTF_2XHE08;
	else if (assoc_giltf & BIT(2))
		return RTW_GILTF_2XHE16;
	else if (assoc_giltf & BIT(1))
		return RTW_GILTF_SGI_4XHE08;
	else if (assoc_giltf & BIT(5))
		return RTW_GILTF_1XHE08;
	else if (assoc_giltf & BIT(4))
		return RTW_GILTF_1XHE16;
	else
		return RTW_GILTF_LGI_4XHE32;
}

bool rtw_halbb_dft_mask(struct bb_info *bb_0,
				struct rtw_phl_stainfo_t *phl_sta_i)
{
	struct bb_info *bb = bb_0;
	u8 mode = 0; /* connect to phl->assoc*/
	u8 hw_md;
	u64 init_mask = 0;
	u64 get_mask = 0;
	u32 mask0, mask1, mask2 = 0;
	enum channel_width bw;
	struct rtw_hal_stainfo_t *hal_sta_i;
	struct protocol_cap_t *asoc_cap_i;

#if 1
	if (!phl_sta_i || !bb) {
		BB_WARNING("[%s][0] phl_sta_i is NULL!!!\n", __func__);
		return false;
	}
	hal_sta_i = phl_sta_i->hal_sta;

	if (!hal_sta_i) {
		BB_WARNING("[%s][1] hal_sta_i is NULL!!!\n", __func__);
		return false;
	}

	if (phl_sta_i->rlink->hw_band == HW_BAND_0) {
		BB_DBG(bb, DBG_RA, "[0] MACID=%d, phy_idx=%d\n", phl_sta_i->macid, bb->bb_phy_idx);
	}
	#ifdef HALBB_DBCC_SUPPORT
	else if (phl_sta_i->rlink->hw_band == HW_BAND_1) {
		HALBB_GET_PHY_PTR(bb_0, bb, HW_PHY_1);
		BB_DBG(bb, DBG_RA, "[1] MACID=%d, phy_idx=%d\n", phl_sta_i->macid, bb->bb_phy_idx);
	}
	#endif
	else {
		BB_WARNING("[%s]MACID not exist\n", __func__);
		return false;
	}

	BB_DBG(bb, DBG_RA, "====>%s\n", __func__);
#else
	if (!phl_sta_i || !bb) {
		BB_WARNING("[%s][1]\n", __func__);
		return false;
	}
	hal_sta_i = phl_sta_i->hal_sta;
	if (!hal_sta_i) {
		BB_WARNING("[%s][2]\n", __func__);
		return false;
	}
#endif
	asoc_cap_i = &phl_sta_i->asoc_cap;
	mode = phl_sta_i->wmode;
	bw = HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i);
	hw_md = halbb_hw_mode_mapping(bb, mode, phl_sta_i);
	BB_DBG(bb, DBG_RA, "Gen Dftmask: mode = %x, hw_md = %x\n", mode, hw_md);

	if (hw_md & (CCK_SUPPORT | OFDM_SUPPORT))
		init_mask = halbb_gen_abg_mask(bb, phl_sta_i);

	if (init_mask == 0) {
		if (hw_md & CCK_SUPPORT) {
			init_mask |= 0x0000000f;
			BB_DBG(bb, DBG_RA,
			       "[%s]abg mask is null!, set b mask=0xf\n", __func__);
		}

		if (hw_md & OFDM_SUPPORT) {
			init_mask |= 0x00000ff0;
			BB_DBG(bb, DBG_RA,
			       "[%s]abg mask is null!, set ag mask=0xff\n", __func__);
		}
	}
#ifdef BB_1115_DVLP_SPF
	if (hw_md & EHT_SUPPORT) {
		get_mask = halbb_gen_eht_mask(bb, phl_sta_i, bw);
		mask2 = (get_mask >> 52) & 0xfff;
		hal_sta_i->ra_info.cur_ra_mask_h= mask2;
	} else if (hw_md & HE_SUPPORT)
#else
	if (hw_md & HE_SUPPORT)
#endif
		get_mask = halbb_gen_he_mask(bb, phl_sta_i, bw);
	else if (hw_md & VHT_SUPPORT_TX)
		get_mask = halbb_gen_vht_mask(bb, phl_sta_i);
	else if (hw_md & HT_SUPPORT)
		get_mask = halbb_gen_ht_mask(bb, phl_sta_i);
	else
		get_mask = 0;
	init_mask |= (get_mask << 12);
	mask0 = (u32)(init_mask & 0xffffffff);
	mask1 = (u32)((init_mask >> 32) & 0xffffffff);
	if (init_mask != 0) {
		hal_sta_i->ra_info.cur_ra_mask = init_mask;
#ifdef BB_1115_DVLP_SPF
		hal_sta_i->ra_info.cur_ra_mask_h = mask2;
#endif
		BB_DBG(bb, DBG_RA, "Default mask = %x %x %x\n", mask0, mask1, mask2);
		return true;
	} else {
		BB_WARNING("[%s]Error default mask, it should not zero\n", __func__);
		return false;
	}
}

u8 rtw_halbb_arfr_trans(struct bb_info *bb,
	struct rtw_phl_stainfo_t *phl_sta_i)
{
	struct rtw_hal_stainfo_t *hal_sta_i;
	u8 mode;
	u8 arfr_ret = 0x0;

	if (!phl_sta_i)
		return false;
	hal_sta_i = phl_sta_i->hal_sta;
	if (!hal_sta_i)
		return false;
	mode = phl_sta_i->wmode;
	if (mode & WLAN_MD_11B) /*11B*/
		arfr_ret |= CCK_SUPPORT;
	if ((mode & WLAN_MD_11A)||(mode & WLAN_MD_11G)) /*11G, 11A*/
		arfr_ret |= OFDM_SUPPORT;
	if (mode & WLAN_MD_11N) /* 11_N*/
		arfr_ret |= HT_SUPPORT;
	if (mode & WLAN_MD_11AC) /*11AC*/
		arfr_ret |= VHT_SUPPORT_TX;
	return arfr_ret;
	/*if (mode|WLAN_MD_11AX ) // 11AX usually can use arfr
		hw_mode_map |= HE_SUPPORT;*/
}

bool halbb_ra_cfg_chk(struct bb_info *bb, u8 *mode, u8 *tx_nss, bool *tx_stbc)
{
	bool rpt = true;

	if (bb->hal_com->dbcc_en && bb->num_rf_path == 2) {
		if (*tx_stbc) {
			BB_WARNING("[%s] stbc_en=%d", __func__, tx_stbc[0]);
			*tx_stbc = false;
			rpt = false;
		}

		if (*tx_nss == 1) { /*2SS*/
			BB_WARNING("[%s] mapping_nss=%d", __func__, tx_nss[0]);
			*tx_nss = 0;
			rpt = false;
		}

		if (*mode == 0) {
			BB_WARNING("[%s] mode=%d", __func__, mode[0]);
			rpt = false;
		}
	}
	return rpt;
}

bool rtw_halbb_raregistered(struct bb_info *bb_0, struct rtw_phl_stainfo_t *phl_sta_i)
{
	struct bb_info *bb = bb_0;
	/*struct rtw_phl_stainfo_t *phl_sta_i =  bb->phl_sta_info[macid];*/
	u8 macid;
	union bb_h2c_ra_cmn_info *ra_cmn;
	struct bb_h2c_ra_cfg_info *ra_cfg;
	struct rtw_hal_stainfo_t *hal_sta_i;
	struct rtw_wifi_role_link_t *rlink = phl_sta_i->rlink;
	struct rtw_rate_info *rt_i;
	bool tx_ldpc;
	bool tx_stbc;
	bool ret_val = false;
	u8 tx_nss;
	//u8 rssi;
	struct protocol_cap_t *asoc_cap_i;
	u8 rssi_assoc ;
	u8 mode;
	bool en_sgi = false;
	u8 giltf_cap = 0;
	u8 init_lv;
	/* Need to mapping with driver wifi mode*/
	u32 *bb_h2c;
	u8 cmdlen = sizeof(union bb_h2c_ra_cmn_info);
	u64 mod_mask, mod_mask_h = 0;
	enum halbb_drv_type drv_role = bb->bb_cmn_hooker->bb_drv_type;
	struct bb_env_mntr_info *env = NULL;
	struct bb_ra_info *bb_ra;

	macid = (u8)(phl_sta_i->macid);
	rt_i = &bb->phl_sta_info[macid]->hal_sta->ra_info.rpt_rt_i;
	hal_sta_i = phl_sta_i->hal_sta;

	if (!hal_sta_i) {
		BB_WARNING("[%s][1] hal_sta_i is NULL!!!\n", __func__);
		return false;
	}

	if (phl_sta_i->rlink->hw_band == HW_BAND_0) {
		BB_DBG(bb, DBG_RA, "[0] MACID=%d, phy_idx=%d\n", macid, bb->bb_phy_idx);
	}
	#ifdef HALBB_DBCC_SUPPORT
	else if (phl_sta_i->rlink->hw_band == HW_BAND_1) {
		HALBB_GET_PHY_PTR(bb_0, bb, HW_PHY_1);
		BB_DBG(bb, DBG_RA, "[1] MACID=%d, phy_idx=%d\n", macid, bb->bb_phy_idx);
	}
	#endif
	else {
		BB_WARNING("[%s]MACID not exist\n", __func__);
		return false;
	}

	env = &bb->bb_env_mntr_i;
	bb_ra = &bb->bb_cmn_hooker->bb_ra_i[macid];
	ra_cmn = &bb_ra->ra_cfg;
	ra_cfg = &ra_cmn->bb_h2c_ra_info;
	bb_h2c = ra_cmn->val;

	BB_DBG(bb, DBG_RA, "====>%s\n", __func__);

	asoc_cap_i = &phl_sta_i->asoc_cap;
	/*@ use assoc rssi to init ra, only use in ra register, it is an integer using U(8,0)*/
	rssi_assoc = (u8)(hal_sta_i->rssi_stat.assoc_rssi);
	//rssi = (u8)(hal_sta_i->rssi_stat.rssi) >> 1; //unused now
	//mode = phl_sta_i->wmode;
	init_lv = halbb_init_ra_by_rssi(bb, rssi_assoc);

	BB_DBG(bb, DBG_RA, "Phy_idx=%d, RSSI_LV=%d, rssi_assoc=%d\n", bb->bb_phy_idx, init_lv, rssi_assoc);

	/*@Becareful RA use our "Tx" capability which means the capability of their "Rx"*/
	tx_nss = halbb_nss_mapping(bb, asoc_cap_i->nss_rx, hal_sta_i->ra_info.ra_nss_limit);
	if (asoc_cap_i->dcm_max_const_rx)
		ra_cfg->dcm_cap = 1;
	else
		ra_cfg->dcm_cap = 0;

	BB_DBG(bb, DBG_RA,
	       "Mode=%s%s%s%s%s\n",
	       (phl_sta_i->wmode & WLAN_MD_11B) ? "B" : " ",
	       (phl_sta_i->wmode & (WLAN_MD_11G | WLAN_MD_11A)) ? "G" : " ",
	       (phl_sta_i->wmode & WLAN_MD_11N) ? "N" : " ",
	       (phl_sta_i->wmode & WLAN_MD_11AC) ? "AC" : " ",
	       (phl_sta_i->wmode & WLAN_MD_11AX) ? "AX" : " ");

	mode = halbb_hw_mode_mapping(bb, phl_sta_i->wmode, phl_sta_i);

	/* ONLY need to get the optional gi-ltf combination for H2C FW*/
	/* bit(0)=4x0.8, bit(1)=1x0.8 -> different definition from the drver giltf*/
	giltf_cap = halbb_get_opt_giltf(bb, asoc_cap_i->ltf_gi);
	ra_cfg->giltf_cap = halbb_ax_giltf_chk(bb, mode, giltf_cap);
	ra_cfg->bw_cap = halbb_hw_bw_mapping(bb, HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i), mode);

	/* giltf assigned by phl/halbb or trained by FW*/
	if (hal_sta_i->ra_info.fix_giltf_en == false) { /*giltf from halbb*/
		#if 0
		if (bb->ic_type & (BB_RTL8852A | BB_RTL8852B | BB_RTL8852C)) { /*Disable FW train GI_LTF*/
			ra_cfg->fix_giltf_en = true;
			ra_cfg->fix_giltf = RTW_GILTF_2XHE08;
			/* gi_ltf is decided by delay spread. This is not ready.
			if (is_giltf_decided_by_delay_sp)
				ra_cfg->fix_giltf = halbb_ra_giltf_ctrl(bb, macid, delay_sp, asoc_cap_i->ltf_gi);
			*/
		} else { /*Enable FW train GI_LTF*/
			ra_cfg->fix_giltf_en = false;
		}
		#endif
		/*Disable FW train GI_LTF*/
		ra_cfg->fix_giltf_en = true;
		if ((ra_cfg->giltf_cap & BIT(0)) && (bb->ic_type & BB_RTL8852C)
			&& (ra_cfg->bw_cap == CHANNEL_WIDTH_160)
			&& (drv_role == BB_NIC_DRV))
			ra_cfg->fix_giltf = RTW_GILTF_SGI_4XHE08;
		else
			ra_cfg->fix_giltf = RTW_GILTF_2XHE08;
	} else { /*giltf from phl*/
		ra_cfg->fix_giltf = halbb_giltf_trans(bb, asoc_cap_i->ltf_gi, hal_sta_i->ra_info.cal_giltf);
	}
	BB_DBG(bb, DBG_RA, "fix_giltf_en(phl)=%d, fix_giltf_en(Final)=%d, fix_giltf=%d\n",
	       hal_sta_i->ra_info.fix_giltf_en, ra_cfg->fix_giltf_en, ra_cfg->fix_giltf);

	ra_cfg->is_dis_ra = hal_sta_i->ra_info.dis_ra;
	mod_mask = hal_sta_i->ra_info.cur_ra_mask;
#ifdef BB_1115_DVLP_SPF
	mod_mask_h = hal_sta_i->ra_info.cur_ra_mask_h;
#endif
	BB_DBG(bb, DBG_RA, "ra mask from phl = %llx, %llx\n", mod_mask, mod_mask_h);
	ra_cfg->er_cap = asoc_cap_i->er_su;
	ra_cfg->partial_bw_su_er = asoc_cap_i->partial_bw_su_er;
	tx_stbc = halbb_stbc_mapping(bb, phl_sta_i, mode);
	ra_cfg->upd_all= true;
	ra_cfg->upd_bw_nss_mask= false;
	ra_cfg->upd_mask= false;

	if (!(bb->ic_type & BB_IC_BE_SERIES)
		&& !(halbb_ra_cfg_chk(bb, &mode, &tx_nss, &tx_stbc))) {
		BB_WARNING("[%s] mode=%d, tx_nss=%d, tx_stbc=%d\n", __func__, mode, tx_nss, tx_stbc);
		return false;
	}
	if (mode == 0) {
		BB_WARNING("[%s] mode=%d\n", __func__, mode);
		return false;
	}
	tx_ldpc = halbb_ldpc_chk(bb, phl_sta_i, mode);
	if (bb->ic_type & BB_IC_BE_SERIES) {
		ra_cmn->bb_h2c_ra_info_wifi7.mode_ctrl_eht= mode;
		ra_cmn->bb_h2c_ra_info_wifi7.bw_cap_eht= halbb_hw_bw_mapping(bb, HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i), mode);
		en_sgi = halbb_sgi_chk(bb, phl_sta_i, ra_cmn->bb_h2c_ra_info_wifi7.bw_cap_eht);
	} else
		en_sgi = halbb_sgi_chk(bb, phl_sta_i, ra_cfg->bw_cap);
	ra_cfg->mode_ctrl = mode;
	ra_cfg->macid = macid;
	ra_cfg->init_rate_lv = init_lv;
	ra_cfg->en_sgi = halbb_ac_n_sgi_chk(bb, mode, en_sgi);
	ra_cfg->ldpc_cap = tx_ldpc;
	ra_cfg->stbc_cap = tx_stbc;
	rt_i->en_stbc = tx_stbc;
	ra_cfg->ss_num = tx_nss;
	ra_cfg->band = rlink->chandef.band;
	if (bb->ic_type == BB_RTL8192XB)
		ra_cfg->is_new_bb_ra_dbgreg = true;
	else
		ra_cfg->is_new_bb_ra_dbgreg = false;

	bb_ra->is_low_latency = false;
	/*@ modify ra mask by assoc rssi*/
	mod_mask = halbb_ramask_mod(bb, macid, mod_mask, rssi_assoc, mode, tx_nss, &mod_mask_h);
	halbb_ramask_trans(bb, macid, mod_mask, mod_mask_h);

	if ((env->env_mntr_rpt_bg.nhm_ratio >= 1) && (env->env_mntr_rpt_bg.nhm_ratio != ENV_MNTR_FAIL_BYTE))
		ra_cfg->is_noisy = true;
	else
		ra_cfg->is_noisy = false;

	BB_DBG(bb, DBG_RA, "nhm_ratio=%d, is_noisy=%d\n",
	       env->env_mntr_rpt_bg.nhm_ratio, ra_cfg->is_noisy);

	ret_val = halbb_set_csi_rate(bb, phl_sta_i);
	if (!ret_val)
		return ret_val;
	BB_DBG(bb, DBG_RA, "RA Register=>In: Dis_ra=%x, MD=%x, BW=%x, macid=%x, band=%d\n",
		   hal_sta_i->ra_info.dis_ra, mode, HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i), macid, rlink->chandef.band);
	BB_DBG(bb, DBG_RA, "RA Register=>In: DCM =%x, ER=%x, Par_ER=%x, in_rt=%x, upd_a=%x, sgi=%x, ldpc=%x, stbc=%x\n",
		   ra_cfg->dcm_cap, ra_cfg->er_cap, ra_cfg->partial_bw_su_er, init_lv, ra_cfg->upd_all,
		   en_sgi, tx_ldpc, tx_stbc);
	BB_DBG(bb, DBG_RA, "RA Register=>In: SS=%x, GILTF_cap=%x, upd_bnm=%x, upd_m=%x, mask=%llx\n",
		   tx_nss, giltf_cap, ra_cfg->upd_bw_nss_mask, ra_cfg->upd_mask, mod_mask);
	BB_DBG(bb, DBG_RA, "RA Register=>Out racfg: dis%x bw%x md%x mid%x\n", ra_cfg->is_dis_ra,
		   ra_cfg->bw_cap, ra_cfg->mode_ctrl, ra_cfg->macid);
	BB_DBG(bb, DBG_RA, "RA Register=>Out h2cp: %x %x %x %x %x %x\n", bb_h2c[0], bb_h2c[1],
		   bb_h2c[2], bb_h2c[3], bb_h2c[4], bb_h2c[5]);
	ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_MACIDCFG, HALBB_H2C_RA, bb_h2c);
	return ret_val;
}

bool rtw_halbb_ra_deregistered(struct bb_info *bb,
	struct rtw_phl_stainfo_t *phl_sta_i)
{
	return true;
}

bool rtw_halbb_raupdate(struct bb_info *bb_0,
	struct rtw_phl_stainfo_t *phl_sta_i)
{
	/* Update only change bw, nss, ramask */
	struct bb_info *bb = bb_0;
	u8 macid;
	union bb_h2c_ra_cmn_info *ra_cmn;
	struct bb_h2c_ra_cfg_info *ra_cfg;
	struct rtw_hal_stainfo_t *hal_sta_i;
	struct protocol_cap_t *asoc_cap_i;
	u8 tx_nss;
	u8 rssi;
	u8 init_lv = 0;
	u8 mode = 0;
	u8 giltf_cap = 0;
	bool ret_val = false;
	/* Need to mapping with driver wifi mode*/
	u32 *bb_h2c;
	u8 cmdlen = sizeof(union bb_h2c_ra_cmn_info);
	u64 mod_mask, mod_mask_h = 0;
	enum halbb_drv_type drv_role = bb->bb_cmn_hooker->bb_drv_type;
	struct bb_env_mntr_info *env = NULL;

#if 1
	if (!phl_sta_i) {
		BB_WARNING("[%s][0] phl_sta_i is NULL!!!\n", __func__);
		return false;
	}
	macid = (u8)(phl_sta_i->macid);
	hal_sta_i = phl_sta_i->hal_sta;

	if (!hal_sta_i) {
		BB_WARNING("[%s][1] hal_sta_i is NULL!!!\n", __func__);
		return false;
	}

	if (phl_sta_i->rlink->hw_band == HW_BAND_0) {
		BB_DBG(bb, DBG_RA, "[0] MACID=%d, phy_idx=%d\n", macid, bb->bb_phy_idx);
	}
	#ifdef HALBB_DBCC_SUPPORT
	else if (phl_sta_i->rlink->hw_band == HW_BAND_1) {
		HALBB_GET_PHY_PTR(bb_0, bb, HW_PHY_1);
		BB_DBG(bb, DBG_RA, "[1] MACID=%d, phy_idx=%d\n", macid, bb->bb_phy_idx);
	}
	#endif
	else {
		BB_WARNING("[%s]MACID not exist\n", __func__);
		return false;
	}

	env = &bb->bb_env_mntr_i;

	ra_cmn = &bb->bb_cmn_hooker->bb_ra_i[macid].ra_cfg;
	ra_cfg = &ra_cmn->bb_h2c_ra_info;
	bb_h2c = ra_cmn->val;

	BB_DBG(bb, DBG_RA, "====>%s\n", __func__);
#else
	BB_DBG(bb, DBG_RA, "====>%s\n", __func__);
	if (!phl_sta_i || !bb) {
		BB_WARNING("[%s][1] Pointer is NULL!!!\n", __func__);
		return ret_val;
	}
	macid = (u8) (phl_sta_i->macid);
	hal_sta_i = phl_sta_i->hal_sta;
	if (!hal_sta_i) {
		BB_WARNING("[%s][2] Pointer is NULL!!!\n", __func__);
		return ret_val;
	}
	ra_cfg = &bb->bb_cmn_hooker->bb_ra_i[macid].ra_cfg;
	mode = ra_cfg->mode_ctrl;
	bb_h2c = (u32 *) ra_cfg;
#endif

	asoc_cap_i = &phl_sta_i->asoc_cap;
	rssi = hal_sta_i->rssi_stat.rssi >> 1;
	/*@Becareful RA use our "Tx" capability which means the capability of their "Rx"*/
	tx_nss = halbb_nss_mapping(bb, asoc_cap_i->nss_rx, hal_sta_i->ra_info.ra_nss_limit);
	ra_cfg->is_dis_ra = hal_sta_i->ra_info.dis_ra;
	mod_mask = hal_sta_i->ra_info.cur_ra_mask;
	ra_cfg->upd_all= false;
	ra_cfg->upd_bw_nss_mask= true;
	ra_cfg->upd_mask= false;
	mode = ra_cfg->mode_ctrl;
#ifdef BB_1115_DVLP_SPF
	mode = ra_cmn->bb_h2c_ra_info_wifi7.mode_ctrl_eht;
	mod_mask_h = hal_sta_i->ra_info.cur_ra_mask_h;
#endif
	BB_DBG(bb, DBG_RA, "mode=%x,ra mask from phl = %llx, %llx\n", mode, mod_mask, mod_mask_h);

	if (mode == 0) {
		BB_WARNING("[%s] mode=%d\n", __func__, mode);
		return false;
	}

	/* ONLY need to get the optional gi-ltf combination for H2C FW*/
	/* bit(0)=4x0.8, bit(1)=1x0.8 -> different definition from the drver giltf*/
	giltf_cap = halbb_get_opt_giltf(bb, asoc_cap_i->ltf_gi);
	ra_cfg->giltf_cap = halbb_ax_giltf_chk(bb, mode, giltf_cap);
	if (bb->ic_type & BB_IC_BE_SERIES)
		ra_cmn->bb_h2c_ra_info_wifi7.bw_cap_eht = halbb_hw_bw_mapping(bb, HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i), mode);
	else
		ra_cfg->bw_cap = halbb_hw_bw_mapping(bb, HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i), mode);

	/* giltf assigned by phl/halbb or trained by FW*/
	if (hal_sta_i->ra_info.fix_giltf_en == false) { /*giltf from halbb*/
		#if 0
		if (bb->ic_type & (BB_RTL8852A | BB_RTL8852B | BB_RTL8852C)) { /*Disable FW train GI_LTF*/
			ra_cfg->fix_giltf_en = true;
			ra_cfg->fix_giltf = RTW_GILTF_2XHE08;
			/* gi_ltf is decided by delay spread. This is not ready.
			if (is_giltf_decided_by_delay_sp)
				ra_cfg->fix_giltf = halbb_ra_giltf_ctrl(bb, macid, delay_sp, asoc_cap_i->ltf_gi);
			*/
		} else { /*Enable FW train GI_LTF*/
			ra_cfg->fix_giltf_en = false;
		}
		#endif
		/*Disable FW train GI_LTF*/
		ra_cfg->fix_giltf_en = true;
		if ((ra_cfg->giltf_cap & BIT(0)) && (bb->ic_type & BB_RTL8852C)
			&& (ra_cfg->bw_cap == CHANNEL_WIDTH_160)
			&& (drv_role == BB_NIC_DRV))
			ra_cfg->fix_giltf = RTW_GILTF_SGI_4XHE08;
		else
			ra_cfg->fix_giltf = RTW_GILTF_2XHE08;
	} else { /*giltf from phl*/
		ra_cfg->fix_giltf = halbb_giltf_trans(bb, asoc_cap_i->ltf_gi, hal_sta_i->ra_info.cal_giltf);
	}
	BB_DBG(bb, DBG_RA, "fix_giltf_en(phl)=%d, fix_giltf_en(Final)=%d, fix_giltf=%d\n",
	       hal_sta_i->ra_info.fix_giltf_en, ra_cfg->fix_giltf_en, ra_cfg->fix_giltf);

	ra_cfg->init_rate_lv = 0;
	ra_cfg->ss_num = tx_nss;
	mod_mask = halbb_ramask_mod(bb, macid, mod_mask, rssi, mode, tx_nss, &mod_mask_h);
	halbb_ramask_trans(bb, macid, mod_mask, mod_mask_h);

	if ((env->env_mntr_rpt_bg.nhm_ratio >= 1) && (env->env_mntr_rpt_bg.nhm_ratio != ENV_MNTR_FAIL_BYTE))
		ra_cfg->is_noisy = true;
	else
		ra_cfg->is_noisy = false;

	BB_DBG(bb, DBG_RA, "nhm_ratio=%d, is_noisy=%d\n",
	       env->env_mntr_rpt_bg.nhm_ratio, ra_cfg->is_noisy);

	BB_DBG(bb, DBG_RA, "RA Register=>Out h2cp: %x %x %x %x %x %x\n", bb_h2c[0], bb_h2c[1],
		   bb_h2c[2], bb_h2c[3], bb_h2c[4], bb_h2c[5]);
	ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_MACIDCFG, HALBB_H2C_RA, bb_h2c);
	return ret_val;
}

bool halbb_ra_update_mask_watchdog(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	u8 macid;
	union bb_h2c_ra_cmn_info *ra_cmn;
	struct bb_h2c_ra_cfg_info *ra_cfg;
	struct rtw_hal_stainfo_t *hal_sta_i;
	struct protocol_cap_t *asoc_cap_i;
	u8 tx_nss;
	u8 rssi;
	u8 init_lv = 0;
	u8 mode = 0;
	u8 giltf_cap = 0;
	bool ret_val = false;
	/* Need to mapping with driver wifi mode*/
	u32 *bb_h2c;
	u8 cmdlen = sizeof(union bb_h2c_ra_cmn_info);
	u64 mod_mask, mod_mask_h = 0;
	enum halbb_drv_type drv_role = bb->bb_cmn_hooker->bb_drv_type;
	struct bb_env_mntr_info *env = NULL;

	env = &bb->bb_env_mntr_i;

	macid = (u8)(phl_sta_i->macid);
	hal_sta_i = phl_sta_i->hal_sta;

	ra_cmn = &bb->bb_cmn_hooker->bb_ra_i[macid].ra_cfg;
	ra_cfg = &ra_cmn->bb_h2c_ra_info;
	mode = ra_cfg->mode_ctrl;
	bb_h2c = ra_cmn->val;
	asoc_cap_i = &phl_sta_i->asoc_cap;
	rssi = hal_sta_i->rssi_stat.rssi >> 1;
	/*@Becareful RA use our "Tx" capability which means the capability of their "Rx"*/
	tx_nss = halbb_nss_mapping(bb, asoc_cap_i->nss_rx, hal_sta_i->ra_info.ra_nss_limit);
	ra_cfg->is_dis_ra = hal_sta_i->ra_info.dis_ra;
	mod_mask = hal_sta_i->ra_info.cur_ra_mask;
#ifdef BB_1115_DVLP_SPF
	mode = ra_cmn->bb_h2c_ra_info_wifi7.mode_ctrl_eht;
	mod_mask_h = hal_sta_i->ra_info.cur_ra_mask_h;
#endif
	BB_DBG(bb, DBG_RA, "ra mask from phl = %llx, %llx\n", mod_mask, mod_mask_h);
	ra_cfg->upd_all= false;
	ra_cfg->upd_bw_nss_mask= false;
	ra_cfg->upd_mask= true;
	/* while ra mask is updated, gi_ltf can also be update */
	if (mode == 0) {
		BB_DBG(bb, DBG_RA, "mode = %d\n", mode);
		return ret_val;
	}
	ra_cfg->init_rate_lv = 0;
	/* ONLY need to get the optional gi-ltf combination for H2C FW*/
	/* bit(0)=4x0.8, bit(1)=1x0.8 -> different definition from the drver giltf*/
	giltf_cap = halbb_get_opt_giltf(bb, asoc_cap_i->ltf_gi);
	ra_cfg->giltf_cap = halbb_ax_giltf_chk(bb, mode, giltf_cap);
	if (bb->ic_type & BB_IC_BE_SERIES)
		ra_cmn->bb_h2c_ra_info_wifi7.bw_cap_eht = halbb_hw_bw_mapping(bb, HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i), mode);
	else
		ra_cfg->bw_cap = halbb_hw_bw_mapping(bb, HALBB_GET_PHL_STA_RA_BW_MODE(phl_sta_i), mode);

	/* giltf assigned by phl/halbb or trained by FW*/
	if (hal_sta_i->ra_info.fix_giltf_en == false) { /*giltf from halbb*/
		#if 0
		if (bb->ic_type & (BB_RTL8852A | BB_RTL8852B | BB_RTL8852C)) { /*Disable FW train GI_LTF*/
			ra_cfg->fix_giltf_en = true;
			ra_cfg->fix_giltf = RTW_GILTF_2XHE08;
			/* gi_ltf is decided by delay spread. This is not ready.
			if (is_giltf_decided_by_delay_sp)
				ra_cfg->fix_giltf = halbb_ra_giltf_ctrl(bb, macid, delay_sp, asoc_cap_i->ltf_gi);
			*/
		} else { /*Enable FW train GI_LTF*/
			ra_cfg->fix_giltf_en = false;
		}
		#endif
		/*Disable FW train GI_LTF*/
		ra_cfg->fix_giltf_en = true;
		if ((ra_cfg->giltf_cap & BIT(0)) && (bb->ic_type & BB_RTL8852C)
			&& (ra_cfg->bw_cap == CHANNEL_WIDTH_160)
			&& (drv_role == BB_NIC_DRV))
			ra_cfg->fix_giltf = RTW_GILTF_SGI_4XHE08;
		else
			ra_cfg->fix_giltf = RTW_GILTF_2XHE08;
	} else { /*giltf from phl*/
		ra_cfg->fix_giltf = halbb_giltf_trans(bb, asoc_cap_i->ltf_gi, hal_sta_i->ra_info.cal_giltf);
	}
	BB_DBG(bb, DBG_RA, "fix_giltf_en(phl)=%d, fix_giltf_en(Final)=%d, fix_giltf=%d\n",
	       hal_sta_i->ra_info.fix_giltf_en, ra_cfg->fix_giltf_en, ra_cfg->fix_giltf);

	mod_mask = halbb_ramask_mod(bb, macid, mod_mask, rssi, mode, tx_nss, &mod_mask_h);
	halbb_ramask_trans(bb, macid, mod_mask, mod_mask_h);

	if ((env->env_mntr_rpt_bg.nhm_ratio >= 1) && (env->env_mntr_rpt_bg.nhm_ratio != ENV_MNTR_FAIL_BYTE))
		ra_cfg->is_noisy = true;
	else
		ra_cfg->is_noisy = false;

	BB_DBG(bb, DBG_RA, "nhm_ratio=%d, is_noisy=%d\n",
	       env->env_mntr_rpt_bg.nhm_ratio, ra_cfg->is_noisy);

	BB_DBG(bb, DBG_RA, "RA Register=>Out h2cp: %x %x %x %x %x %x\n", bb_h2c[0], bb_h2c[1],
		   bb_h2c[2], bb_h2c[3], bb_h2c[4], bb_h2c[5]);
	ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_MACIDCFG, HALBB_H2C_RA, bb_h2c);
	return ret_val;
}

u32 halbb_get_fw_ra_rpt(struct bb_info *bb, u16 len, u8 *c2h)
{
#ifdef HALBB_DBCC_SUPPORT
	struct bb_info *bb_1 = NULL;
#endif
	u16 macid_rpt;
	struct rtw_hal_stainfo_t *hal_sta_i;
	struct rtw_phl_stainfo_t *phl_sta_i;
	struct rtw_rate_info *rt_i;
	struct halbb_ra_rpt_info *ra_rpt_i;
	struct bb_rate_info rate;
	u16 rate_idx = 0;
	u8 mcs_ss_idx;
	u8 mode = 0;
	u8 bw = 0;
	enum halbb_drv_type drv_role = bb->bb_cmn_hooker->bb_drv_type;
	#if 0 /*wait for phl mu ra declaration*/
	u8 u1_muidx = 0;
	u16 i = 0;
	#endif

	if (!c2h)
		return 0;
	ra_rpt_i = (struct halbb_ra_rpt_info *)c2h;
	macid_rpt = ra_rpt_i->rpt_macid_l + (ra_rpt_i->rpt_macid_m << 8);
	if (macid_rpt >= PHL_MAX_STA_NUM) {
		BB_WARNING("[%s][1]Error macid = %d!!\n", __func__, macid_rpt);
		return 0;
	}

	if (bb->sta_exist[macid_rpt]) {
		phl_sta_i = bb->phl_sta_info[(u8)macid_rpt];
	} else {
#ifdef HALBB_DBCC_SUPPORT
		HALBB_GET_PHY_PTR(bb, bb_1, HW_PHY_1);
		if (bb_1->sta_exist[macid_rpt]) {
			phl_sta_i = bb_1->phl_sta_info[(u8)macid_rpt];
		} else
#endif
		{
			BB_WARNING("[%s][2]Error macid = %d!!\n", __func__, macid_rpt);
			return 0;
		}
	}

	if (!phl_sta_i) {
		BB_WARNING("[%s][3] phl_sta==NULL, Wrong C2H RA macid !!\n", __func__);
		return 0;
	}
	hal_sta_i = phl_sta_i->hal_sta;
	if (!hal_sta_i) {
		BB_WARNING("[%s][4] hal_sta==NULL, Wrong C2H RA macid !!\n", __func__);
		return 0;
	}

	rt_i = &hal_sta_i->ra_info.rpt_rt_i;
	if (bb->ic_type & BB_IC_AX_SERIES) {
		rate_idx = (u16)(ra_rpt_i->rpt_mcs_nss & 0x7f) | ((u16)(ra_rpt_i->rpt_md_sel & 0x3) << 7);
		bw = ra_rpt_i->rpt_bw;
		mode = ra_rpt_i->rpt_md_sel;
		mcs_ss_idx = ra_rpt_i->rpt_mcs_nss & 0x7f;
	} else {
		rate_idx = (u16)((ra_rpt_i->rpt_mcs_nss & 0x7f) | ra_rpt_i->rpt_mcs_nss_M << 7)
					| ((u16)((ra_rpt_i->rpt_md_sel & 0x3) << 8) | (ra_rpt_i->rpt_md_sel_M << 10));
		bw = (ra_rpt_i->rpt_bw) | (ra_rpt_i->rpt_bw_M << 2);
		mode = (ra_rpt_i->rpt_md_sel) | (ra_rpt_i->rpt_md_sel_M << 2);
		mcs_ss_idx = ((ra_rpt_i->rpt_mcs_nss & 0x7f) | ra_rpt_i->rpt_mcs_nss_M << 7);
		BB_DBG(bb, DBG_RA, 
		       "Rpt:rate = %x,mcsss = %x,mcsss_M = %x,md = %x,md_M = %d,bw = %d,bw_M = %d\n",
		       rate_idx, ra_rpt_i->rpt_mcs_nss, ra_rpt_i->rpt_mcs_nss_M, ra_rpt_i->rpt_md_sel,
			   ra_rpt_i->rpt_md_sel_M, ra_rpt_i->rpt_bw, ra_rpt_i->rpt_bw_M);
	}
	halbb_rate_idx_parsor(bb, rate_idx, ra_rpt_i->rpt_gi_ltf, &rate);

	if (ra_rpt_i->is_mu) {
		#if 0 /*wait for phl mu ra declaration*/
		hal_sta_i->ra_info.mu_idx = ra_rpt_i->u0_muidx;

		u1_muidx = (ra_rpt_i->u1_muidx >= ra_rpt_i->u0_muidx) ?
			   ra_rpt_i->u1_muidx + 1 : ra_rpt_i->u1_muidx;

		if (hal_sta_i->ra_info.mu_active[ra_rpt_i->u1_muidx] == false) {
			for (i = 0; i < PHL_MAX_STA_NUM; i++) {
				if (i == (u8)macid_rpt)
					continue;
				if (!bb->sta_exist[i])
					continue;
				if (!is_sta_active(bb->phl_sta_info[i]))
					continue;

				if (bb->phl_sta_info[i]->hal_sta->ra_info.mu_idx == u1_muidx)
					hal_sta_i->ra_info.mu_active[ra_rpt_i->u1_muidx] = true;
			}
		}

		if (hal_sta_i->ra_info.mu_active[ra_rpt_i->u1_muidx]) {
			hal_sta_i->ra_info.mu_curr_retry_ratio[ra_rpt_i->u1_muidx] = ra_rpt_i->retry_ratio;
			rt_i->mu_mcs_ss_idx[ra_rpt_i->u1_muidx] = mcs_ss_idx;
			rt_i->mu_gi_ltf[ra_rpt_i->u1_muidx] = ra_rpt_i->rpt_gi_ltf;
			rt_i->mu_mode[ra_rpt_i->u1_muidx] = mode;
			rt_i->mu_mcs_idx[ra_rpt_i->u1_muidx] = rate.idx;
			rt_i->mu_ss[ra_rpt_i->u1_muidx] = rate.ss - 1;
			rt_i->mu_bw[ra_rpt_i->u1_muidx] = bw;

			BB_DBG(bb, DBG_RA,
			       "MU RA RPT: su_macid = %d, muidx{u0,u1} = {%d,%d}, mu_mode = %d, mu_giltf = %x, mu_mcs_nss = %x, mu_mcs_idx = %x, mu_ss = %x\n",
			       macid_rpt, ra_rpt_i->u0_muidx, u1_muidx,
			       rt_i->mu_mode[ra_rpt_i->u1_muidx],
			       rt_i->mu_gi_ltf[ra_rpt_i->u1_muidx],
			       rt_i->mu_mcs_ss_idx[ra_rpt_i->u1_muidx],
			       rt_i->mu_mcs_idx[ra_rpt_i->u1_muidx],
			       rt_i->mu_ss[ra_rpt_i->u1_muidx]);
		}
		#endif
	} else {
		hal_sta_i->ra_info.curr_retry_ratio = ra_rpt_i->retry_ratio;
		rt_i->mcs_ss_idx = mcs_ss_idx;
		rt_i->gi_ltf = ra_rpt_i->rpt_gi_ltf;
		rt_i->mode = mode;
		rt_i->mcs_idx = rate.idx;
		rt_i->ss = rate.ss - 1;
		rt_i->bw = bw;
		if ((enum bb_mode_type)mode >= BB_HE_MODE) /*HE & EHT*/
			rt_i->is_actrl = ((rt_i->mcs_ss_idx & 0xf) <= 2) ? false : true;
		else
			rt_i->is_actrl = false;
		/*Aruba AP STBC IOT WA: Dynamic turn off STBC in WD when HE 1ss per=100 */
		/* Jira: HALBB-211*/
		if (!(bb->ic_type & BB_IC_FW_CONTROL_STBC)
			&& (drv_role == BB_NIC_DRV))
			rt_i->en_stbc= ra_rpt_i->en_stbc;

		BB_DBG(bb, DBG_RA,
		       "SU RA RPT:macid = %d,mode = %d,giltf = %x,mcs_nss = %x,mcs_idx = %x,ss = %x,retry = %d,stbc = %d\n",
		       macid_rpt, rt_i->mode, rt_i->gi_ltf, rt_i->mcs_ss_idx, rt_i->mcs_idx, rt_i->ss,
		       ra_rpt_i->retry_ratio, rt_i->en_stbc);
	}

	return 0;
}

void halbb_get_fw_c2h_tx_hist(struct bb_info *bb, u16 len, u8 *c2h)
{
	struct bb_ra_tx_hist_c2h_rpt *c2h_rpt;
	u8 i = 0;

	if (!c2h)
		return;

	c2h_rpt = (struct bb_ra_tx_hist_c2h_rpt *)c2h;
#if 0
	BB_DBG(bb, DBG_RA, "ra_tbtt_cnt=%03d\n", c2h_rpt->ra_tbtt_cnt);

	BB_DBG(bb, DBG_RA, "=== [Legacy Rate]==============================\n");

	for (i = 0; i < 12; i++) {
		BB_DBG(bb, DBG_RA, "Legacy[%02d]: cnt: %03d\n", i, c2h_rpt->tx_rate_cnt_hist[i]);
	}

	BB_DBG(bb, DBG_RA, "=== [1ss Rate]=================================\n");

	for (i = 12; i < 26; i++) {
		BB_DBG(bb, DBG_RA, "1SS MCS[%02d]: cnt: %03d\n", i - 12, c2h_rpt->tx_rate_cnt_hist[i]);	
	}

	BB_DBG(bb, DBG_RA, "=== [2ss Rate]=================================\n");

	for (i = 26; i < 40; i++) {
		BB_DBG(bb, DBG_RA, "2SS MCS[%02d]: cnt: %03d\n", i - 26, c2h_rpt->tx_rate_cnt_hist[i]);	
	}
#endif
	halbb_mem_cpy(bb, &bb->bb_tx_hist_rpt_i, c2h_rpt, sizeof(struct bb_ra_tx_hist_c2h_rpt));
}

u32 halbb_get_fw_ra_dbgrpt_wifi7(struct bb_info *bb, u16 len, u8 *c2h)
{
	struct halbb_c2h_dbg_rpt_wifi7 *ra_rpt_i;
	struct bb_dbg_info *dbg = &bb->bb_dbg_i;
	struct bb_ra_dbgreg *ra_dbg_i = &dbg->ra_dbgreg_i;

	if (!c2h)
		return 0;
	ra_rpt_i = (struct halbb_c2h_dbg_rpt_wifi7 *)c2h;
	ra_dbg_i->macid = ra_rpt_i->macid;
	if (ra_dbg_i->macid >= PHL_MAX_STA_NUM) {
		BB_WARNING("[%s][1]Error macid = %d!!\n", __func__, ra_dbg_i->macid);
		return 0;
	}
	ra_dbg_i->cmac_tbl_id0 = ra_rpt_i->cmac_tbl;
	ra_dbg_i->per = ra_rpt_i->per;
	ra_dbg_i->rdr = ra_rpt_i->rdr;
	ra_dbg_i->r4 = ra_rpt_i->r4;
	ra_dbg_i->cls = ra_rpt_i->cls;
	ra_dbg_i->rate_up_lmt_cnt = ra_rpt_i->rate_up_lmt_cnt;
	ra_dbg_i->per_ma = ra_rpt_i->per_ma;
	ra_dbg_i->var = ra_rpt_i->var;
	ra_dbg_i->d_o_n = ra_rpt_i->d_o_n;
	ra_dbg_i->d_o_p = ra_rpt_i->d_o_p;
	ra_dbg_i->rd_th = ra_rpt_i->rd_th;
	ra_dbg_i->ru_th = ra_rpt_i->ru_th;
	ra_dbg_i->try_per = ra_rpt_i->try_per;
	ra_dbg_i->try_rdr = ra_rpt_i->try_rdr;
	ra_dbg_i->try_r4 = ra_rpt_i->try_r4;
	ra_dbg_i->txrpt_tot = ra_rpt_i->txrpt_tot;
	ra_dbg_i->ra_timer = ra_rpt_i->ra_timer;
	ra_dbg_i->tot_disra_trying_return = ra_rpt_i->tot_disra_trying_return;
	ra_dbg_i->r4_return = ra_rpt_i->r4_return;
	ra_dbg_i->ra_mask_h = ra_rpt_i->ra_mask_h;
	ra_dbg_i->ra_mask_l = ra_rpt_i->ra_mask_l;
	ra_dbg_i->highest_rate = ra_rpt_i->highest_rate;
	ra_dbg_i->lowest_rate = ra_rpt_i->lowest_rate;
	ra_dbg_i->upd_all_h2c_0 = ra_rpt_i->upd_all_h2c_0;
	ra_dbg_i->upd_all_h2c_1 = ra_rpt_i->upd_all_h2c_1;
	ra_dbg_i->upd_all_h2c_2 = ra_rpt_i->upd_all_h2c_2;
	ra_dbg_i->upd_all_h2c_3 = ra_rpt_i->upd_all_h2c_3;
	BB_DBG(bb, DBG_RA,
		   "RA DBG RPT:macid=%d, cmac=0x%x,per=%d, rdr=%d, r4=%d, highest_rate=0x%x, lowest_rate=0x%x\n",
		   ra_dbg_i->macid, ra_dbg_i->cmac_tbl_id0, ra_dbg_i->per,
		   ra_dbg_i->rdr, ra_dbg_i->r4, ra_dbg_i->highest_rate, ra_dbg_i->lowest_rate);

	return 0;
}

bool rtw_halbb_query_txsts(struct bb_info *bb, u16 macid0, u16 macid1)
{
	u8 content[4];
	u32 *bb_h2c = (u32 *)content;
	bool ret_val;
	u16 cmdlen = 4;

	BB_DBG(bb, DBG_RA, "====> QuerryTxSts : macid = %d %d\n", macid0, macid1);
	content[0] = (u8)(macid0& 0xff);
	content[1] = (u8)((macid0>>8)& 0xff);
	content[2] = (u8)(macid1& 0xff);
	content[3] = (u8)((macid1>>8)& 0xff);
	ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_GET_TXSTS, HALBB_H2C_RA, bb_h2c);
	if (!ret_val)
		BB_WARNING("[%s] Error H2C cmd in querry txsts !!\n", __func__);
	return ret_val;
}

void halbb_drv_cmac_rpt_parsing(struct bb_info *bb, u8 *rpt)
{
	struct bb_fw_cmac_rpt_info *c_rpt;
	u16 rpt_len, i;
	u8 *rpt_tmp = rpt;

	if (!rpt)
		return;
	c_rpt = (struct bb_fw_cmac_rpt_info *)rpt;
	rpt_len = sizeof(struct bb_fw_cmac_rpt_info);
	if (rpt_len % 4) {
		return;
	}
	BB_DBG(bb, DBG_RA,
	       "[%d] try_rate=%d, fix_rate=0x%x, final{rate, giltf}={0x%x,0x%x}, data_bw = 0x%x\n",
	       c_rpt->macid, c_rpt->try_rate, c_rpt->fix_rate,
	       c_rpt->final_rate, c_rpt->final_gi_ltf, c_rpt->data_bw);
	for (i = 0; i < rpt_len; i++) {
		BB_DBG(bb, DBG_RA, "[%d] 0x%x\n", i, ((u32*)rpt)[0]);
		rpt_tmp += 4;
	}
}

u32 halbb_get_txsts_rpt(struct bb_info *bb, u16 len, u8 *c2h)
{
#ifdef HALBB_DBCC_SUPPORT
	struct bb_info *bb_1 = NULL;
#endif
	u16 macid_rpt;
	u16 macid_rpt_1;
	u8 i = 0;
	struct rtw_hal_stainfo_t *hal_sta_i;
	struct rtw_phl_stainfo_t *phl_sta_i;
	struct rtw_ra_sta_info *ra_sta_i;
	struct halbb_txsts_info *txsts_i;
	u16 tx_total;
	u16 tx_ok[4];
	u16 tx_retry[4];
	u8 null_cnt = 0;
	u8 j = 0;

	if (!c2h)
		return 0;

	for (j = 0; j <= 1; j++) {
		c2h = (j) ? (c2h + 32) : c2h; /*txsts_info is 32 bytes*/

		txsts_i = (struct halbb_txsts_info *)c2h;
		macid_rpt = txsts_i->rpt_macid_l + (txsts_i->rpt_macid_m << 8);

		if (j == 0)
			macid_rpt_1 = macid_rpt;

		if (macid_rpt >= PHL_MAX_STA_NUM) {
			BB_DBG(bb, DBG_RA,
			       "[%s][1] macid = %d, Error macid !!\n",
			       __func__, macid_rpt);
			null_cnt++;
			continue;
		}

		if (bb->sta_exist[macid_rpt]) {
			phl_sta_i = bb->phl_sta_info[(u8)macid_rpt];
		} else {
#ifdef HALBB_DBCC_SUPPORT
			HALBB_GET_PHY_PTR(bb, bb_1, HW_PHY_1);
			if (bb_1->sta_exist[macid_rpt]) {
				phl_sta_i = bb_1->phl_sta_info[(u8)macid_rpt];
			} else
#endif
			{
				BB_DBG(bb, DBG_RA,
				       "[%s][2]Error macid = %d!!\n",
				       __func__, macid_rpt);
				null_cnt++;
				continue;
			}
		}

		if (phl_sta_i == NULL) {
			BB_DBG(bb, DBG_RA,
			       "[%s][3] phl_sta == NULL, Error macid = %d!!\n",
			       __func__, macid_rpt);
			null_cnt++;
			continue;
		}
		hal_sta_i = phl_sta_i->hal_sta;
		if (hal_sta_i == NULL) {
			BB_DBG(bb, DBG_RA,
			       "[%s][3] hal_sta == NULL, Error macid = %d!!\n",
			       __func__, macid_rpt);
			null_cnt++;
			continue;
		}
		ra_sta_i = &hal_sta_i->ra_info;
		tx_ok[0] = txsts_i->tx_ok_be_l + (txsts_i->tx_ok_be_m << 8);
		tx_ok[1] = txsts_i->tx_ok_bk_l + (txsts_i->tx_ok_bk_m << 8);
		tx_ok[2] = txsts_i->tx_ok_vi_l + (txsts_i->tx_ok_vi_m << 8);
		tx_ok[3] = txsts_i->tx_ok_vo_l + (txsts_i->tx_ok_vo_m << 8);
		tx_retry[0] = txsts_i->tx_retry_be_l + (txsts_i->tx_retry_be_m << 8);
		tx_retry[1] = txsts_i->tx_retry_bk_l + (txsts_i->tx_retry_bk_m << 8);
		tx_retry[2] = txsts_i->tx_retry_vi_l + (txsts_i->tx_retry_vi_m << 8);
		tx_retry[3] = txsts_i->tx_retry_vo_l + (txsts_i->tx_retry_vo_m << 8);
		for (i = 0; i <= 3; i++) {
			if ((0xffffffff - ra_sta_i->tx_ok_cnt[i]) > (tx_ok[i]))
				ra_sta_i->tx_ok_cnt[i] += tx_ok[i];
			if ((0xffffffff - ra_sta_i->tx_retry_cnt[i]) > (tx_retry[i]))
				ra_sta_i->tx_retry_cnt[i] += tx_retry[i];
			BB_DBG(bb, DBG_RA, "TxOk[%d] = %d, TxRty[%d] = %d\n", i, tx_ok[i], i, tx_retry[i]);
		}
		tx_total = txsts_i->tx_total_l + (txsts_i->tx_total_m << 8);
		if ((0xffffffff - ra_sta_i->tx_total_cnt) > tx_total)
			ra_sta_i->tx_total_cnt += tx_total;
		BB_DBG(bb, DBG_RA,
		       "====> GetTxSts : macid = %d, TxTotal = %d\n",
		       macid_rpt, tx_total);
	}

	if (null_cnt > 1)
		BB_WARNING("====> GetTxStS : both macid sta is null, macid = {%d,%d}\n",
			   macid_rpt_1, macid_rpt);

	return 0;
}

void halbb_ra_rssi_setting_watchdog(struct bb_info *bb)
{
	u8 macid = 0;
	u16 i = 0, sta_cnt = 0;
	u16 cmdlen = 0;
	u16 cmdlen_per_sta = 0;
	bool ret_val = true;
	struct rtw_ra_sta_info *bb_ra = NULL;
	struct rtw_hal_stainfo_t *hal_sta_i = NULL;
	union bb_h2c_ra_rssi_info *ra_rssi = NULL;
	struct bb_h2c_rssi_setting *rssi_i = NULL;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_dtp_info *dtp = NULL;
	u32 *buff_head = NULL, *buff_curr = NULL;
	u8 rssi_a = 0, rssi_b = 0, bcn_rssi_a = 0, bcn_rssi_b = 0;

	cmdlen_per_sta = sizeof(union bb_h2c_ra_rssi_info);
	cmdlen = cmdlen_per_sta * PHL_MAX_STA_NUM;
	buff_head = hal_mem_alloc(bb->hal_com, cmdlen);

	if (!buff_head) {
		BB_WARNING("[%s] Error RSSI allocat failed!!\n", __func__);
		return;
	}

	sta_cnt = 0;

	for (i = 0; i < PHL_MAX_STA_NUM; i++) {
		if (!bb->sta_exist[i] || !bb->phl_sta_info[i])
			continue;

		hal_sta_i = bb->phl_sta_info[i]->hal_sta;
		if (!hal_sta_i)
			continue;

		bb_ra = &hal_sta_i->ra_info;
		if (!(bb_ra->ra_registered))
			continue;

		buff_curr = buff_head + cmdlen_per_sta * sta_cnt;
		ra_rssi = (union bb_h2c_ra_rssi_info *)buff_curr;
		rssi_i = &ra_rssi->bb_h2c_ra_rssi;

		macid = (u8)(bb->phl_sta_info[i]->macid);
		dtp = &bb->bb_pwr_ctrl_i.dtp_i[macid];
		BB_DBG(bb, DBG_RA, "Add BB rssi info[%d], macid=%d\n", i, macid);
		/* Need modify for Nss > 2 */
		rssi_a = (hal_sta_i->rssi_stat.rssi_ma_path[0] >> 5) & 0x7f;
		bcn_rssi_a = (hal_sta_i->rssi_stat.rssi_bcn_ma_path[0] >> 5) & 0x7f;
		//if (!bb->hal_com->dbcc_en)
		{
			rssi_b = (hal_sta_i->rssi_stat.rssi_ma_path[1] >> 5) & 0x7f;
			bcn_rssi_b = (hal_sta_i->rssi_stat.rssi_bcn_ma_path[1] >> 5) & 0x7f;
		}
		rssi_i->macid = macid;
		rssi_i->rssi_a = rssi_a | BIT(7);
		rssi_i->rssi_b = rssi_b;
		rssi_i->bcn_rssi_a = bcn_rssi_a | BIT(7);
		rssi_i->bcn_rssi_b = bcn_rssi_b;
		rssi_i->is_fixed_rate = bb_ra->fixed_rt_en;
		rssi_i->fixed_rate_md = bb_ra->fixed_rt_i.mode;
		rssi_i->fixed_giltf = bb_ra->fixed_rt_i.gi_ltf;
		rssi_i->fixed_bw = bb_ra->fixed_rt_i.bw;

		if (bb->ic_type & BB_IC_AX_SERIES) {
			rssi_i->fixed_rate = bb_ra->fixed_rt_i.mcs_ss_idx & 0x3f;
			rssi_i->fixed_is_mu = false;
			if (bb->support_ability & BB_PWR_CTRL)
				rssi_i->dtp_lv = dtp->dyn_tx_pwr_lvl;
			else
				rssi_i->dtp_lv = 0;
		} else {
			ra_rssi->bb_h2c_ra_rssi_wifi7.fixed_rate = bb_ra->fixed_rt_i.mcs_ss_idx & 0x7f;
			ra_rssi->bb_h2c_ra_rssi_wifi7.fixed_rate_M = ((u8)bb_ra->fixed_rt_i.mcs_ss_idx >> 7) & 0x1;
			ra_rssi->bb_h2c_ra_rssi_wifi7.fixed_bw_M = ((u8)bb_ra->fixed_rt_i.bw >> 2) & 0x1;
			ra_rssi->bb_h2c_ra_rssi_wifi7.fixed_rate_md_M = ((u8)bb_ra->fixed_rt_i.mode >>2) & 0x1;
			ra_rssi->bb_h2c_ra_rssi_wifi7.fixed_is_mu = false;
		}

		BB_DBG(bb, DBG_RA, "bcn_rssi{a,b}={%d,%d}, data_rssi{a,b}={%d,%d}, dtp_lv=%d\n",
		       bcn_rssi_a, bcn_rssi_b, rssi_a, rssi_b, dtp->dyn_tx_pwr_lvl);
		BB_DBG(bb, DBG_RA, "RSSI=>h2c: 0x%04x%04x\n", ra_rssi->val[1], ra_rssi->val[0]);

		sta_cnt++;

		if (sta_cnt == bb_link->num_linked_client)
			break;
	}

	if ((sta_cnt > 0) && (sta_cnt <= STA_NUM_RSSI_CMD)) {
		/* Fill endcmd = 1 for last sta */
		rssi_i->endcmd = 1;
		ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RSSISETTING, HALBB_H2C_RA, buff_head);
		BB_DBG(bb, DBG_RA, "sta_cnt=%d, RSSI cmd end\n", sta_cnt);
	}

	if (!ret_val)
		BB_WARNING("[%s] H2C cmd error!!\n", __func__);

	if (buff_head)
		hal_mem_free(bb->hal_com, buff_head, cmdlen);
}

void halbb_ra_giltf_ctrl(struct bb_info *bb, u8 macid, u8 delay_sp, u8 assoc_giltf)
{
	//struct bb_ra_info *bb_ra = &bb->bb_cmn_hooker->bb_ra_i[macid];
	//struct bb_h2c_ra_cfg_info *ra_cfg = &bb->bb_cmn_hooker->bb_ra_i[macid].ra_cfg;
	//enum rtw_gi_ltf giltf = RTW_GILTF_LGI_4XHE32;

	//if (!bb)
	//	return;

	/*This dhould be decision before ra mask h2c*/
	/* GI LTF decision algorithm is needed*/
	/*if (delay_sp)
		giltf = RTW_GILTF_LGI_4XHE32;
	else
		giltf = RTW_GILTF_LGI_4XHE32;
	ra_cfg->giltf = giltf;*/
}

bool halbb_ra_bfer_chk(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	struct protocol_cap_t *asoc_cap = &phl_sta_i->asoc_cap;
	struct protocol_cap_t *self_cap = &phl_sta_i->rlink->protocol_cap;
	bool bfer_chk = false;

	BB_DBG(bb, DBG_RA, "====>%s\n", __func__);

	if ((phl_sta_i->wmode & WLAN_MD_11AX) &&
	    ((asoc_cap->he_su_bfme && self_cap->he_su_bfmr) ||
	     (asoc_cap->he_mu_bfme && self_cap->he_mu_bfmr))) {
		/*AX bfer chk*/
		bfer_chk = true;
	} else if ((phl_sta_i->wmode & WLAN_MD_11AC) &&
		   ((asoc_cap->vht_su_bfme && self_cap->vht_su_bfmr) ||
		    (asoc_cap->vht_mu_bfme && self_cap->vht_mu_bfmr))) {
		/*AC bfer chk*/
		bfer_chk = true;
	} else if ((phl_sta_i->wmode & WLAN_MD_11N) &&
		   (asoc_cap->ht_su_bfme && self_cap->ht_su_bfmr)) {
		/*N bfer chk*/
		bfer_chk = true;
	}

	BB_DBG(bb, DBG_RA,
		"assoc bfee : {he_su, he_mu, vht_su, vht_mu, ht_su}={%d,%d,%d,%d,%d}\n",
		asoc_cap->he_su_bfme, asoc_cap->he_mu_bfme,
		asoc_cap->vht_su_bfme, asoc_cap->vht_mu_bfme,
		asoc_cap->ht_su_bfme);

	BB_DBG(bb, DBG_RA,
		"self bfer : {he_su, he_mu, vht_su, vht_mu, ht_su}={%d,%d,%d,%d,%d}\n",
		self_cap->he_su_bfmr, self_cap->he_mu_bfmr,
		self_cap->vht_su_bfmr, self_cap->vht_mu_bfmr,
		self_cap->ht_su_bfmr);

	return bfer_chk;
}

bool halbb_ra_bfee_chk(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i)
{
	struct protocol_cap_t *asoc_cap = &phl_sta_i->asoc_cap;
	struct protocol_cap_t *self_cap = &phl_sta_i->rlink->protocol_cap;
	bool bfee_chk = false;

	BB_DBG(bb, DBG_RA, "====>%s\n", __func__);


	if ((phl_sta_i->wmode & WLAN_MD_11AX) &&
	    ((asoc_cap->he_su_bfmr && self_cap->he_su_bfme) ||
	     (asoc_cap->he_mu_bfmr && self_cap->he_mu_bfme))) {
		/*AX bfee chk*/
		bfee_chk = true;
	} else if ((phl_sta_i->wmode & WLAN_MD_11AC) &&
		   ((asoc_cap->vht_su_bfmr && self_cap->vht_su_bfme) ||
		    (asoc_cap->vht_mu_bfmr && self_cap->vht_mu_bfme))) {
		/*AC bfee chk*/
		bfee_chk = true;
	} else if ((phl_sta_i->wmode & WLAN_MD_11N) &&
		   (asoc_cap->ht_su_bfmr && self_cap->ht_su_bfme)) {
		/*N bfee chk*/
		bfee_chk = true;
	}

	BB_DBG(bb, DBG_RA,
		"assoc bfer : {he_su, he_mu, vht_su, vht_mu, ht_su}={%d,%d,%d,%d,%d}\n",
		asoc_cap->he_su_bfmr, asoc_cap->he_mu_bfmr,
		asoc_cap->vht_su_bfmr, asoc_cap->vht_mu_bfmr,
		asoc_cap->ht_su_bfmr);

	BB_DBG(bb, DBG_RA,
		"self bfee : {he_su, he_mu, vht_su, vht_mu, ht_su}={%d,%d,%d,%d,%d}\n",
		self_cap->he_su_bfme, self_cap->he_mu_bfme,
		self_cap->vht_su_bfme, self_cap->vht_mu_bfme,
		self_cap->ht_su_bfme);

	return bfee_chk;
}

void halbb_ra_watchdog(struct bb_info *bb)
{
	u16 i = 0, sta_cnt = 0, macid = 0;
	struct bb_link_info *bb_link = &bb->bb_link_i;

	halbb_show_cr_cnt(bb, BB_WD_RA);
#if 0
	if (!(bb->support_ability & BB_RA))
		return;
#endif
	halbb_ra_rssi_setting_watchdog(bb);

	bb_link->at_least_one_bfee = false;
	bb_link->at_least_one_bfer = false;

	for (i = 0; i < PHL_MAX_STA_NUM; i++) {
		if (!(bb->sta_exist[i]))
			continue;

		if (!(bb->phl_sta_info[i])) {
			BB_WARNING("[%s][1] Pointer is NULL!!!\n", __func__);
			continue;
		}

		if (!(bb->phl_sta_info[i]->hal_sta)) {
			BB_WARNING("[%s][2] Pointer is NULL!!!\n", __func__);
			continue;
		}

		if (!(bb->phl_sta_info[i]->hal_sta->ra_info.ra_registered))
			continue;

		macid = (u8)(bb->phl_sta_info[i]->macid);
		BB_DBG(bb, DBG_RA, "====>ra update mask[%d], macid=%d\n", i, macid);
		if (!halbb_ra_update_mask_watchdog(bb, bb->phl_sta_info[i])) {
			BB_DBG(bb, DBG_RA, "Update Fail\n");
		}

		BB_DBG(bb, DBG_RA,
		       "i=%d,macid=%d,Mode=%s%s%s%s%s\n", i, macid,
		       (bb->phl_sta_info[i]->wmode & WLAN_MD_11B) ? "B" : " ",
		       (bb->phl_sta_info[i]->wmode & (WLAN_MD_11G | WLAN_MD_11A)) ? "G" : " ",
		       (bb->phl_sta_info[i]->wmode & WLAN_MD_11N) ? "N" : " ",
		       (bb->phl_sta_info[i]->wmode & WLAN_MD_11AC) ? "AC" : " ",
		       (bb->phl_sta_info[i]->wmode & WLAN_MD_11AX) ? "AX" : " ");

		if (halbb_ra_bfee_chk(bb, bb->phl_sta_info[i]))
			bb_link->at_least_one_bfee = true;

		if (halbb_ra_bfer_chk(bb, bb->phl_sta_info[i]))
			bb_link->at_least_one_bfer = true;

		sta_cnt++;
		if (sta_cnt == bb_link->num_linked_client)
			break;
	}

	BB_DBG(bb, DBG_RA,
	       "at_least_one_bfee=%d, at_least_one_bfer=%d\n",
	       bb_link->at_least_one_bfee, bb_link->at_least_one_bfer);
}

void halbb_ra_init(struct bb_info *bb)
{
	struct bb_ra_info *bb_ra = NULL;
	u16 macid = 0;

	for (macid = 0; macid < PHL_MAX_STA_NUM; macid ++) {
		bb_ra = &bb->bb_cmn_hooker->bb_ra_i[macid];
		if (!bb_ra)
			halbb_mem_set(bb, bb_ra, 0, sizeof (struct bb_ra_info));
	}

	bb->bb_cmn_hooker->bb_ra_dbg_i.macid = 0xffff;
	bb->bb_cmn_hooker->bb_ra_dbg_i.per_ppdu = 0;
}

u8 rtw_halbb_rts_rate(struct bb_info *bb, u16 tx_rate, bool is_erp_prot)
{

	u8 rts_ini_rate = RTW_DATA_RATE_OFDM6;

	if (is_erp_prot) { /* use CCK rate as RTS */
		rts_ini_rate = RTW_DATA_RATE_CCK1;
	} else {
		switch (tx_rate) {
		case RTW_DATA_RATE_VHT_NSS4_MCS9:
		case RTW_DATA_RATE_VHT_NSS4_MCS8:
		case RTW_DATA_RATE_VHT_NSS4_MCS7:
		case RTW_DATA_RATE_VHT_NSS4_MCS6:
		case RTW_DATA_RATE_VHT_NSS4_MCS5:
		case RTW_DATA_RATE_VHT_NSS4_MCS4:
		case RTW_DATA_RATE_VHT_NSS4_MCS3:
		case RTW_DATA_RATE_VHT_NSS3_MCS9:
		case RTW_DATA_RATE_VHT_NSS3_MCS8:
		case RTW_DATA_RATE_VHT_NSS3_MCS7:
		case RTW_DATA_RATE_VHT_NSS3_MCS6:
		case RTW_DATA_RATE_VHT_NSS3_MCS5:
		case RTW_DATA_RATE_VHT_NSS3_MCS4:
		case RTW_DATA_RATE_VHT_NSS3_MCS3:
		case RTW_DATA_RATE_VHT_NSS2_MCS9:
		case RTW_DATA_RATE_VHT_NSS2_MCS8:
		case RTW_DATA_RATE_VHT_NSS2_MCS7:
		case RTW_DATA_RATE_VHT_NSS2_MCS6:
		case RTW_DATA_RATE_VHT_NSS2_MCS5:
		case RTW_DATA_RATE_VHT_NSS2_MCS4:
		case RTW_DATA_RATE_VHT_NSS2_MCS3:
		case RTW_DATA_RATE_VHT_NSS1_MCS9:
		case RTW_DATA_RATE_VHT_NSS1_MCS8:
		case RTW_DATA_RATE_VHT_NSS1_MCS7:
		case RTW_DATA_RATE_VHT_NSS1_MCS6:
		case RTW_DATA_RATE_VHT_NSS1_MCS5:
		case RTW_DATA_RATE_VHT_NSS1_MCS4:
		case RTW_DATA_RATE_VHT_NSS1_MCS3:
		case RTW_DATA_RATE_MCS31:
		case RTW_DATA_RATE_MCS30:
		case RTW_DATA_RATE_MCS29:
		case RTW_DATA_RATE_MCS28:
		case RTW_DATA_RATE_MCS27:
		case RTW_DATA_RATE_MCS23:
		case RTW_DATA_RATE_MCS22:
		case RTW_DATA_RATE_MCS21:
		case RTW_DATA_RATE_MCS20:
		case RTW_DATA_RATE_MCS19:
		case RTW_DATA_RATE_MCS15:
		case RTW_DATA_RATE_MCS14:
		case RTW_DATA_RATE_MCS13:
		case RTW_DATA_RATE_MCS12:
		case RTW_DATA_RATE_MCS11:
		case RTW_DATA_RATE_MCS7:
		case RTW_DATA_RATE_MCS6:
		case RTW_DATA_RATE_MCS5:
		case RTW_DATA_RATE_MCS4:
		case RTW_DATA_RATE_MCS3:
		case RTW_DATA_RATE_OFDM54:
		case RTW_DATA_RATE_OFDM48:
		case RTW_DATA_RATE_OFDM36:
		case RTW_DATA_RATE_OFDM24:
			rts_ini_rate = RTW_DATA_RATE_OFDM24;
			break;
		case RTW_DATA_RATE_VHT_NSS4_MCS2:
		case RTW_DATA_RATE_VHT_NSS4_MCS1:
		case RTW_DATA_RATE_VHT_NSS3_MCS2:
		case RTW_DATA_RATE_VHT_NSS3_MCS1:
		case RTW_DATA_RATE_VHT_NSS2_MCS2:
		case RTW_DATA_RATE_VHT_NSS2_MCS1:
		case RTW_DATA_RATE_VHT_NSS1_MCS2:
		case RTW_DATA_RATE_VHT_NSS1_MCS1:
		case RTW_DATA_RATE_MCS26:
		case RTW_DATA_RATE_MCS25:
		case RTW_DATA_RATE_MCS18:
		case RTW_DATA_RATE_MCS17:
		case RTW_DATA_RATE_MCS10:
		case RTW_DATA_RATE_MCS9:
		case RTW_DATA_RATE_MCS2:
		case RTW_DATA_RATE_MCS1:
		case RTW_DATA_RATE_OFDM18:
		case RTW_DATA_RATE_OFDM12:
			rts_ini_rate = RTW_DATA_RATE_OFDM12;
			break;
		case RTW_DATA_RATE_VHT_NSS4_MCS0:
		case RTW_DATA_RATE_VHT_NSS3_MCS0:
		case RTW_DATA_RATE_VHT_NSS2_MCS0:
		case RTW_DATA_RATE_VHT_NSS1_MCS0:
		case RTW_DATA_RATE_MCS24:
		case RTW_DATA_RATE_MCS16:
		case RTW_DATA_RATE_MCS8:
		case RTW_DATA_RATE_MCS0:
		case RTW_DATA_RATE_OFDM9:
		case RTW_DATA_RATE_OFDM6:
			rts_ini_rate = RTW_DATA_RATE_OFDM6;
			break;
		case RTW_DATA_RATE_CCK11:
		case RTW_DATA_RATE_CCK5_5:
		case RTW_DATA_RATE_CCK2:
		case RTW_DATA_RATE_CCK1:
			rts_ini_rate = RTW_DATA_RATE_CCK1;
			break;
		default:
			rts_ini_rate = RTW_DATA_RATE_OFDM6;
			break;
		}
	}

	if (bb->hal_com->band[0].cur_chandef.band == BAND_ON_5G) {
		if (rts_ini_rate < RTW_DATA_RATE_OFDM6)
			rts_ini_rate = RTW_DATA_RATE_OFDM6;
	}
	return rts_ini_rate;
}

void halbb_ra_shift_darf_tc(struct bb_info *bb, bool enable, u8 *init_fb_cnt)
{
	struct bb_h2c_ra_shift_dafc_tc darf_tc_h2c;
	u32 *bb_h2c = (u32 *) &darf_tc_h2c;
	u8 cmdlen = sizeof(struct bb_h2c_ra_shift_dafc_tc);
	bool ret_val = false;
	u8 i = 0;

	darf_tc_h2c.enable = (u8)enable;

	for (i = 0; i < 24; i++) { /*1ss MCS0 ~ 2ss MCS11*/
		if (init_fb_cnt[i] == 0) /*the range of init_fb_cnt is 1~8*/
			darf_tc_h2c.init_fb_cnt[i] = 4; /*default setting => 0x1123_1000*/
		else
			darf_tc_h2c.init_fb_cnt[i] = init_fb_cnt[i];
	}

	BB_DBG(bb, DBG_RA,
	       "RA shift darf_tc H2C: %x %x %x %x %x %x %x\n",
	       bb_h2c[0], bb_h2c[1], bb_h2c[2], bb_h2c[3], bb_h2c[4],
	       bb_h2c[5], bb_h2c[6]);

	ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RA_SHIFT_DARF_TC, HALBB_H2C_RA, bb_h2c);
}

void halbb_get_ra_dbgreg(struct bb_info *bb)
{
	struct rtw_phl_com_t *phl = bb->phl_com;
	struct dev_cap_t *dev = &phl->dev_cap;
	struct bb_dbg_info *dbg = &bb->bb_dbg_i;
	struct bb_ra_dbgreg *dbgreg = &dbg->ra_dbgreg_i;

	if (bb->bb_watchdog_mode != BB_WATCHDOG_NORMAL)
		return;

	dbgreg->cmac_tbl_id0 = halbb_get_reg(bb, 0x160, MASKDWORD);
	dbgreg->cmac_tbl_id1 = halbb_get_reg(bb, 0x164, MASKDWORD);
	dbgreg->per = halbb_get_reg(bb, 0x168, MASKBYTE3);
	dbgreg->rdr = halbb_get_reg(bb, 0x174, MASKBYTE0);
	dbgreg->r4 = halbb_get_reg(bb, 0x174, MASKBYTE1);
	if ((bb->ic_type != BB_RTL8852A) && (bb->ic_type != BB_RTL8852B) &&
	    (bb->ic_type != BB_RTL8851B)) {
		dbgreg->cls = halbb_get_reg(bb, 0x2444, MASKBYTE0);
	}
	dbgreg->rate_up_lmt_cnt = halbb_get_reg(bb, 0x168, MASKBYTE0);
	dbgreg->per_ma = halbb_get_reg(bb, 0x16c, MASKLWORD);
	dbgreg->var = halbb_get_reg(bb, 0x16c, MASKHWORD);
	dbgreg->d_o_n = halbb_get_reg(bb, 0x170, MASKLWORD);
	dbgreg->d_o_p = halbb_get_reg(bb, 0x170, MASKHWORD);
	dbgreg->rd_th = halbb_get_reg(bb, 0x168, MASKBYTE1);
	dbgreg->ru_th = halbb_get_reg(bb, 0x168, MASKBYTE2);
	dbgreg->try_per = halbb_get_reg(bb, 0x178, MASKBYTE2);
	dbgreg->try_rdr = halbb_get_reg(bb, 0x184, MASKBYTE3);
	dbgreg->try_r4 = halbb_get_reg(bb, 0x188, MASKBYTE0);
	dbgreg->txrpt_tot = halbb_get_reg(bb, 0x178, MASKBYTE3);
	dbgreg->ra_timer = halbb_get_reg(bb, 0x178, MASKBYTE1);
	dbgreg->tot_disra_trying_return = halbb_get_reg(bb, 0x174, MASKBYTE2);
	dbgreg->r4_return = halbb_get_reg(bb, 0x174, MASKBYTE3);
	dbgreg->ra_mask_h = halbb_get_reg(bb, 0x180, MASKDWORD);
	dbgreg->ra_mask_l = halbb_get_reg(bb, 0x17c, MASKDWORD);
	dbgreg->highest_rate = halbb_get_reg(bb, 0x184, MASKBYTE0);
	dbgreg->lowest_rate = halbb_get_reg(bb, 0x184, MASKBYTE1);
	dbgreg->upd_all_h2c_0 = halbb_get_reg(bb, 0x1d0, MASKDWORD);
	dbgreg->upd_all_h2c_1 = halbb_get_reg(bb, 0x1d4, MASKDWORD);
	dbgreg->upd_all_h2c_2 = halbb_get_reg(bb, 0x1d8, MASKDWORD);
	dbgreg->upd_all_h2c_3 = halbb_get_reg(bb, 0x1dc, MASKDWORD);
	if ((bb->ic_type == BB_RTL8852A) || (bb->ic_type == BB_RTL8852B)) {
		dbgreg->dyn_stbc = halbb_get_reg(bb, 0x1ec, MASKBYTE3);
	}
	/*MU MIMO RA*/
	if (dev->rfe_type >= 50) {
		dbgreg->mu_mcs = halbb_get_reg(bb, 0x1a0, MASKDWORD);
		dbgreg->mu_id_lowest_rate = halbb_get_reg(bb, 0x1a4, MASKDWORD);
		dbgreg->mu_rd_ru_th = halbb_get_reg(bb, 0x1a8, MASKDWORD);
		dbgreg->mu_per = halbb_get_reg(bb, 0x1ac, MASKDWORD);
	}
}

void halbb_ra_fix_rate_dbg(struct bb_info *bb, char input[][MAX_ARGV],
			   u32 *_used, char *output, u32 *_out_len)
{
	struct bb_ra_drv_info *ra_drv = &bb->bb_cmn_hooker->bb_ra_drv_i;
	struct bb_link_info *link = &bb->bb_link_i;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	u32 val[10] = {0};
	u8 i;
	bool ret_val = false;
	struct rtw_ra_sta_info *bb_ra = NULL;
	struct rtw_hal_stainfo_t *hal_sta_i = NULL;
	u8 rssi_a = 0, rssi_b = 0, bcn_rssi_a = 0, bcn_rssi_b = 0, cmdlen = 0;
	u16 rssi_len = 0;
	u32 *bb_h2c = NULL;
	union bb_h2c_ra_rssi_info ra_rssi = {0};
	struct bb_h2c_rssi_setting *rssi_i = &ra_rssi.bb_h2c_ra_rssi;
	u8 rate_ss_tmp = 1;
	u8 ra_macid = 0;
	enum bb_mode_type ra_mode = 0;
	u8 ra_giltf = 0;
	u8 ra_rate_idx = 0;
	u8 ra_bw = 0;
	bool fw_fix_rate_en = false;
	char *text_tmp = NULL;
	u16 l_phy_rate_tmp = 0;
	u8 ra_mcs_idx = 0;
	bool is_mu = false;

	for (i = 0; i < 8; i++) {
		HALBB_SCAN(input[i + 1], DCMD_DECIMAL, &val[i]);
	}

	if (_os_strcmp(input[1], "sel_macid") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		ra_drv->drv_fw_fix_rate_macid = (u8)val[0];
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "sel_macid= %d \n", ra_drv->drv_fw_fix_rate_macid);
		return;
	}

	if (_os_strcmp(input[1], "fix") == 0) {
		if (link->is_one_entry_only)
			ra_macid = (u8)link->one_entry_macid; /*target_macid*/
		else
			ra_macid = (u8)ra_drv->drv_fw_fix_rate_macid;

		phl_sta = bb->phl_sta_info[ra_macid];
		if (!phl_sta) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used,
				    *_out_len - *_used,
				    "phl_sta null, macid=%d\n", ra_macid);
			return;
		}
		hal_sta_i = phl_sta->hal_sta;
		bb_ra = &hal_sta_i->ra_info;
		if (!bb_ra->ra_registered) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used,
				*_out_len - *_used, "[Err] sta_registered = %d\n",
				bb_ra->ra_registered);
			return;
		}

		ra_mode = bb_ra->rpt_rt_i.mode;
		ra_giltf = bb_ra->rpt_rt_i.gi_ltf;
		ra_mcs_idx = bb_ra->rpt_rt_i.mcs_idx;
		rate_ss_tmp = bb_ra->rpt_rt_i.ss + 1;
		ra_bw = bb_ra->rpt_rt_i.bw;

		if (_os_strcmp(input[2], "auto") == 0) {
			fw_fix_rate_en = false;

			BB_DBG_CNSL(*_out_len, *_used, output + *_used,
					*_out_len - *_used, "AUTO rate\n");
		} else if (_os_strcmp(input[2], "giltf") == 0) {
			fw_fix_rate_en = true;
			HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);

			ra_giltf = (u8)val[0];

			if ((ra_mode <= BB_VHT_MODE) && (ra_giltf > 1))
				BB_DBG_CNSL(*_out_len, *_used, output + *_used,
					    *_out_len - *_used,
					    "Set Err, lgy/ht/vht gi=%d\n", ra_giltf);
			else if ((ra_mode <= BB_EHT_MODE) && (ra_giltf > RTW_GILTF_1XHE08))
				BB_DBG_CNSL(*_out_len, *_used, output + *_used,
					    *_out_len - *_used,
					    "Set Err, he/eht giltf=%d\n", ra_giltf);
		} else if (_os_strcmp(input[2], "bw") == 0) {
			fw_fix_rate_en = true;
			HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);

			ra_bw = (u8)val[0];

			if ((ra_mode == BB_HT_MODE) && (ra_bw > BB_CMAC_BW_40M))
				BB_DBG_CNSL(*_out_len, *_used, output + *_used,
					    *_out_len - *_used,
					    "Set Err, ht bw=%d\n", ra_bw);
			else if (((ra_mode == BB_VHT_MODE) || (ra_mode == BB_HE_MODE)) &&
				 (ra_bw > BB_CMAC_BW_160M))
				BB_DBG_CNSL(*_out_len, *_used, output + *_used,
					    *_out_len - *_used,
					    "Set Err, vht/he bw=%d\n", ra_bw);
			else if ((ra_mode == BB_EHT_MODE) && (ra_bw > BB_CMAC_BW_320M))
				BB_DBG_CNSL(*_out_len, *_used, output + *_used,
					    *_out_len - *_used,
					    "Set Err, eht bw=%d\n", ra_bw);
		} else if (_os_strcmp(input[2], "rate") == 0) {
			fw_fix_rate_en = true;
			HALBB_SCAN(input[4], DCMD_DECIMAL, &val[0]);

			if (_os_strcmp(input[3], "cck") == 0) {
				ra_mode = BB_LEGACY_MODE;
				ra_bw = (u8)BB_CMAC_BW_20M;
				rate_ss_tmp = 1;

				for (i = 0; i <= 4; i++) {
					l_phy_rate_tmp = (u8)bb_phy_rate_table[i];
					if (l_phy_rate_tmp == val[0]) {
						ra_mcs_idx = i;
						break;
					}
					if (i == 4) {
						BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
								"Set Err\n");
						return;
					}
				}
			} else if (_os_strcmp(input[3], "ofdm") == 0) {
				ra_mode = BB_LEGACY_MODE;
				ra_bw = (u8)BB_CMAC_BW_20M;
				rate_ss_tmp = 1;

				for (i = 4; i <= 12; i++) {
					l_phy_rate_tmp = bb_phy_rate_table[i];
					if (l_phy_rate_tmp == (u16)val[0]) {
						ra_mcs_idx = i;
						break;
					}
					if (i == 12) {
						BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
								"Set Err\n");
						return;
					}
				}
			} else if (_os_strcmp(input[3], "ht") == 0) {
				ra_mode = BB_HT_MODE;
				rate_ss_tmp = 1;
				ra_mcs_idx = (u8)val[0];
			} else if (_os_strcmp(input[3], "xht") == 0) {
				rate_ss_tmp = (u8)val[0];
				if ((rate_ss_tmp == 0) || (rate_ss_tmp > bb->num_rf_path) ||
					(ra_mode < BB_VHT_MODE)) {
					BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
							"Set Err, mode= %d, rate_ss = %d\n",
							ra_mode, rate_ss_tmp);
					return;
				}

				HALBB_SCAN(input[5], DCMD_DECIMAL, &val[1]);
				ra_mcs_idx = (u8)(val[1]);
			} else {
				BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
						"Set Err\n");
				return;
			}
		}
		if (bb->ic_type & BB_IC_AX_SERIES)
			ra_rate_idx = ((rate_ss_tmp - 1) << 4) | ra_mcs_idx;
		else
			ra_rate_idx = ((rate_ss_tmp - 1) << 5) | ra_mcs_idx;
	} else if (val[0] == 1) {
		ra_macid = (u8)val[1];
		phl_sta = bb->phl_sta_info[ra_macid];
		if (!phl_sta) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used,
				    *_out_len - *_used,
				    "phl_sta null, macid=%d\n", ra_macid);
			return;
		}
		hal_sta_i = phl_sta->hal_sta;
		bb_ra = &hal_sta_i->ra_info;
		if (!bb_ra->ra_registered) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used,
				*_out_len - *_used, "[Err] sta_registered = %d\n",
				bb_ra->ra_registered);
			return;
		}

		fw_fix_rate_en = true;
		ra_mode = (enum bb_mode_type)val[2];
		ra_giltf = (u8)val[3];
		if (bb->ic_type & BB_IC_AX_SERIES) {
			ra_rate_idx = (u8)(val[4] & 0x3f);
			is_mu = (bool)((val[4] >> 6) & 0x1);
			ra_bw = (u8)val[5];
		} else {
			rate_ss_tmp = (u8)val[4];
			ra_mcs_idx = (u8)val[5];
			ra_bw = (u8)val[6];
			is_mu = (bool)val[7];
			ra_rate_idx = ((rate_ss_tmp - 1) << 5) | ra_mcs_idx;
		}

		if ((rate_ss_tmp == 0) || (rate_ss_tmp > bb->num_rf_path)) {
			BB_DBG_CNSL(*_out_len, *_used, output +
				    *_used, *_out_len - *_used,
				    "Set Err, rate_ss = %d\n", rate_ss_tmp);
			return;
		}
	} else if (val[0] == 2) {
		ra_macid = (u8)val[1];
		phl_sta = bb->phl_sta_info[ra_macid];
		if (!phl_sta) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used,
				    *_out_len - *_used,
				    "phl_sta null, macid=%d\n", ra_macid);
			return;
		}
		hal_sta_i = phl_sta->hal_sta;
		bb_ra = &hal_sta_i->ra_info;
		if (!bb_ra->ra_registered) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used,
				*_out_len - *_used, "[Err] sta_registered = %d\n",
				bb_ra->ra_registered);
			return;
		}

		fw_fix_rate_en = false;
		is_mu = (bool)val[2];
	}

	ra_drv->is_fw_fix_rate[ra_macid] = fw_fix_rate_en;

	if (fw_fix_rate_en) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "=== [FW_fix_rate} ========\n");

		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d\n", "FW_fix_rate_en", fw_fix_rate_en);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d\n", "macid", ra_macid);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d\n", "mode", ra_mode);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d\n", "giltf", ra_giltf);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d\n", "Nss", rate_ss_tmp);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d\n", "mcs", ra_mcs_idx);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d M\n", "bw", 20 << ra_bw);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "%-20s: %d\n", "is_mu", is_mu);

		if (ra_mode == BB_EHT_MODE)
			text_tmp = "EHT ";
		else if (ra_mode == BB_HE_MODE)
			text_tmp = "HE ";
		else if (ra_mode == BB_VHT_MODE)
			text_tmp = "VHT ";
		else if (ra_mode == BB_HT_MODE)
			text_tmp = "HT";
		else
			text_tmp = "";

		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "=== [Summary} ============\n");
		if (ra_mode >= BB_VHT_MODE) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "FIX @ %s %dSS-MCS%d, BW:%dM\n",
				    text_tmp, rate_ss_tmp, ra_mcs_idx, 20 << ra_bw);
		} else if (ra_mode == BB_HT_MODE) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "FIX @ HT MCS%d, BW:%dM\n",
				    ra_mcs_idx, 20 << ra_bw);
		} else {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "FIX @ %d M, BW:%dM\n",
				    bb_phy_rate_table[ra_mcs_idx], 20 << ra_bw);
		}

		ra_drv->last_fw_fix_rate_macid = (u16)ra_macid;
	} else {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			"%-20s: %d\n", "FW_fix_rate_en", false);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			"%-20s: %d\n", "macid", ra_macid);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			"%-20s: %d\n", "is_mu", is_mu);
	}

	bb_h2c = ra_rssi.val;
	cmdlen = sizeof(union bb_h2c_ra_rssi_info);

	if (!hal_sta_i) {
		BB_WARNING("[%s] hal_sta_i = NULL\n", __func__);
		return;
	}

	/* Need modify for Nss > 2 */
	rssi_a = (hal_sta_i->rssi_stat.rssi_ma_path[0] >> 5) & 0x7f;
	bcn_rssi_a = (hal_sta_i->rssi_stat.rssi_bcn_ma_path[0] >> 5) & 0x7f;
	if (!bb->hal_com->dbcc_en) {
		rssi_b = (hal_sta_i->rssi_stat.rssi_ma_path[1] >> 5) & 0x7f;
		bcn_rssi_b = (hal_sta_i->rssi_stat.rssi_bcn_ma_path[1] >> 5) & 0x7f;
	}

	rssi_i->macid = ra_macid;
	rssi_i->rssi_a = rssi_a | BIT(7);
	rssi_i->rssi_b = rssi_b;
	rssi_i->bcn_rssi_a = bcn_rssi_a | BIT(7);
	rssi_i->bcn_rssi_b = bcn_rssi_b;
	rssi_i->is_fixed_rate = fw_fix_rate_en;
	rssi_i->fixed_giltf = ra_giltf;
	rssi_i->fixed_bw = (u8)(ra_bw & 0x3);
	rssi_i->fixed_rate_md = (u8)(ra_mode & 0x3);
	rssi_i->endcmd = 1;
	if (bb->ic_type & BB_IC_AX_SERIES) {
		rssi_i->fixed_rate = ra_rate_idx & 0x3f;
		rssi_i->fixed_is_mu = is_mu;
	} else {
		ra_rssi.bb_h2c_ra_rssi_wifi7.fixed_rate = ra_rate_idx & 0x7f;
		ra_rssi.bb_h2c_ra_rssi_wifi7.fixed_rate_M = (ra_rate_idx >> 7) & 0x1;
		ra_rssi.bb_h2c_ra_rssi_wifi7.fixed_bw_M = (ra_bw >> 2) & 0x1;
		ra_rssi.bb_h2c_ra_rssi_wifi7.fixed_rate_md_M = ((u8)ra_mode >> 2) & 0x1;
		ra_rssi.bb_h2c_ra_rssi_wifi7.fixed_is_mu = is_mu;
	}

	if (!bb_ra) {
		BB_WARNING("[%s] bb_ra = NULL\n", __func__);
		return;
	}

	if (is_mu == 0) { /*RSSI setting watchdog only works for SU*/
		bb_ra->fixed_rt_en = fw_fix_rate_en;
		bb_ra->fixed_rt_i.mcs_ss_idx = ra_rate_idx;
		bb_ra->fixed_rt_i.bw = ra_bw;
		bb_ra->fixed_rt_i.mode = ra_mode;
		bb_ra->fixed_rt_i.gi_ltf = ra_giltf;
	}

	BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
		    "RA RSSI setting H2C: %x %x\n", bb_h2c[0], bb_h2c[1]);

	ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RSSISETTING, HALBB_H2C_RA, bb_h2c);
}

void halbb_ra_dbg(struct bb_info *bb, char input[][MAX_ARGV], u32 *_used,
		  char *output, u32 *_out_len)
{
	struct bb_ra_drv_info *ra_drv = &bb->bb_cmn_hooker->bb_ra_drv_i;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	char help[] = "-h";
	char tx_hist_cnt_unit[][5] = {{"MPDU"}, {"PPDU"}};
	u32 val[10] = {0};
	u8 i;
	bool ret_val = false;
	struct rtw_ra_sta_info *bb_ra;
	struct rtw_hal_stainfo_t *hal_sta_i;
	u8 h2c_ra_cls[4];
	u8 cmdlen = 0;
	u32 *bb_h2c = NULL;
	struct bb_h2c_ra_adjust ra_th_i;
	struct bb_h2c_ra_mask ra_mask_i;
	struct bb_h2c_ra_d_o_timer ra_d_o_timer_i;
	struct bb_h2c_ra_tx_hist_info ra_tx_hist_i;

	if (_os_strcmp(input[1], help) == 0) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "==========Simple cmd=============\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "fix auto\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "fix rate cck {1/2/5/11}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "fix rate ofdm {6/9/12/18/24/36/48/54}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "fix rate ht {MCS}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "fix rate xht {SS:1~4} {MCS}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "fix bw {20/40/80/160/320: 0/1/2/3/4}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "fix giltf {giltf}\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "sel_macid {id: %d}\n", ra_drv->drv_fw_fix_rate_macid);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "=============Adv cmd=============\n");
		if (bb->ic_type & BB_IC_AX_SERIES)
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "{Fix rate} [ra] [1] [macid] [mode] [giltf] [is_mu/ss_mcs(dec)] [bw]\n");
		else
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "{Fix rate} [ra] [1] [macid] [mode] [giltf] [Nss] [mcs] [bw] [is_mu]\n");

		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "{Auto rate}: [ra] [2] [macid] [is_mu]\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "=============Notes=============>\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			     "[mode]: 0:(legacy), 1:(HT), 2:(VHT), 3:(HE) 4:(EHT)\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "[giltf]: 0: (4xHE-LTF 3.2usGI, LGI), 1: (4xHE-LTF 0.8usGI, SGI), 2: (2xHE-LTF 1.6usGI)\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "[giltf]: 3: (2xHE-LTF 0.8usGI), 4: (1xHE-LTF 1.6usGI), 5: (1xHE-LTF 0.8usGI)\n");
		if (bb->ic_type & BB_IC_AX_SERIES)
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "[is_mu/ss_mcs]: (Bitmap format) [6]: is_mu [5:4]:Nss(0=1SS, 1=2SS) [3:0]:MCS\n");
		else
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "[Nss]: 1~4=1SS~4SS, [MCS]: legacy=0~11, HT=0~15, VHT=0~9, HE=0~11, EHT=0~15\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			 "==============================>\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "{Drvier shift rate up/down threshold}: [ra] [3] [macid] [0: Increase th. (Tend to RU) 1: Decrease th (Tend to RD)] [percent]\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "{Manually adjust RA mask}: [ra] [4] [macid] [0: dis. manual adj. RA mask; 1: en. manual adj. RA mask] [0: mask; 1: reveal] [rate_mode] [ss_mcs]\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "{d_o_timer}: [ra] [5] [macid] [en] [timer (FW default 20)]\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "{cls head/tail}: [ra] [6] [en]\n");
		if (bb->ic_type & BB_IC_AX_SERIES) {
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "{debug bb reg}: [ra] [dbgreg]\n");
		} else {

			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
				    "{WIFI7 debug RA rpt}: [ra] [dbgreg] [macid]\n");
		}
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "=============Tx hist dbg=========\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "tx_hist macid {val} // All macid cnt for val == 0xffff\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "tx_hist {\"ppdu\" / \"mpdu\"}\n");
		//BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
		//	 "{Fix rate & ra mask}: ra (3 [macid] [mode] [giltf] [ss_mcs] [mask1] [mask0])}\n");
		return;
	}

	for (i = 0; i < 8; i++) {
		HALBB_SCAN(input[i + 1], DCMD_DECIMAL, &val[i]);
	}

	if ((_os_strcmp(input[1], "sel_macid") == 0) ||
	    (_os_strcmp(input[1], "fix") == 0) ||
	    (val[0] == 1) || (val[0] == 2)) {
		halbb_ra_fix_rate_dbg(bb, input, _used, output, _out_len);
	} else if (_os_strcmp(input[1], "tx_hist") == 0) {
		bb_h2c = (u32 *) &ra_tx_hist_i;
		cmdlen = sizeof(struct bb_h2c_ra_tx_hist_info);

		if (_os_strcmp(input[2], "macid") == 0) {
			HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
			bb->bb_cmn_hooker->bb_ra_dbg_i.macid = (u16)val[0];
		} else if (_os_strcmp(input[2], "ppdu") == 0) {
			bb->bb_cmn_hooker->bb_ra_dbg_i.per_ppdu = 1;
		} else if (_os_strcmp(input[2], "mpdu") == 0) {
			bb->bb_cmn_hooker->bb_ra_dbg_i.per_ppdu = 0;
		}

		ra_tx_hist_i.macid = bb->bb_cmn_hooker->bb_ra_dbg_i.macid;
		ra_tx_hist_i.per_ppdu = bb->bb_cmn_hooker->bb_ra_dbg_i.per_ppdu;

		ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RA_TX_HIST, HALBB_H2C_RA, bb_h2c);

		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
					"[Tx rate histogram info]\n");
		if (ra_tx_hist_i.macid == 0xffff)
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
					"macid: all\n");
		else
			BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
					"macid: %d\n", ra_tx_hist_i.macid);
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
					"Tx rate cnt unit: %s\n", tx_hist_cnt_unit[ra_tx_hist_i.per_ppdu]);
	} else if (val[0] == 3) {
		bb_h2c = (u32 *) &ra_th_i;
		cmdlen = sizeof(struct bb_h2c_ra_adjust);

		hal_sta_i = bb->phl_sta_info[(u8)val[1]]->hal_sta;
		bb_ra = &hal_sta_i->ra_info;
		if (bb_ra->ra_registered) {
			BB_DBG(bb, DBG_RA, "RA adjust th. macid=[%d]\n", (u8)val[1]);
			ra_th_i.macid = (u8)val[1];
			ra_th_i.drv_shift_en = (u8)val[2] & 0x01;
			ra_th_i.drv_shift_value= (u8)val[3] & 0x7f;

			BB_DBG(bb, DBG_RA, "RA adjust %s th =[%d]\n",
			ra_th_i.drv_shift_en == 0x1 ? "RD": "RU", ra_th_i.drv_shift_value);

			BB_DBG(bb, DBG_RA, "RA adjust th H2C: %x\n", bb_h2c[0]);

			ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RA_ADJUST, HALBB_H2C_RA, bb_h2c);
		} else {
			BB_DBG(bb, DBG_RA, "No Link ! RA rssi cmd fail!\n");
		}
	} else if (val[0] == 4) {
		bb_h2c = (u32 *) &ra_mask_i;
		cmdlen = sizeof(struct bb_h2c_ra_mask);

		hal_sta_i = bb->phl_sta_info[(u8)val[1]]->hal_sta;
		bb_ra = &hal_sta_i->ra_info;

		if (bb_ra->ra_registered) {
			BB_DBG(bb, DBG_RA, "macid=[%d]\n", (u8)val[1]);

			ra_mask_i.macid = (u8)val[1];
			ra_mask_i.is_manual_adjust_ra_mask = (u8)val[2] & 0x01;
			ra_mask_i.mask_or_reveal = (u8)val[3] & 0x01;
			ra_mask_i.mask_rate_md= (u8)val[4];
			ra_mask_i.mask_rate= (u8)val[5];

			BB_DBG(bb, DBG_RA, "Manual adjust RA mask = %d\n",
			       ra_mask_i.is_manual_adjust_ra_mask);
			BB_DBG(bb, DBG_RA, "Adjust mode=%d, rate(d')=%d\n",
			       ra_mask_i.mask_rate_md, ra_mask_i.mask_rate);
			BB_DBG(bb, DBG_RA, "[%s] this rate in RA mask\n",
			       ra_mask_i.mask_or_reveal ? "Reveal": "Mask");
			BB_DBG(bb, DBG_RA, "RA adjust RAmask H2C: %x\n",
			       bb_h2c[0]);

			ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_ADJUST_RA_MASK, HALBB_H2C_RA, bb_h2c);
		} else {
			BB_DBG(bb, DBG_RA, "No Link ! RA rssi cmd fail!\n");
		}
	} else if (val[0] == 5) {
		bb_h2c = (u32 *) &ra_d_o_timer_i;
		cmdlen = sizeof(struct bb_h2c_ra_d_o_timer);

		hal_sta_i = bb->phl_sta_info[(u8)val[1]]->hal_sta;
		bb_ra = &hal_sta_i->ra_info;
		if (bb_ra->ra_registered) {
			BB_DBG(bb, DBG_RA, "RA d_o_timer macid=[%d]\n", (u8)val[1]);
			ra_d_o_timer_i.macid = (u8)val[1];
			ra_d_o_timer_i.d_o_timer_en = (u8)val[2] & 0x01;
			ra_d_o_timer_i.d_o_timer_value= (u8)val[3] & 0x7f;

			BB_DBG(bb, DBG_RA, "RA d_o_timer {en,value} = {%d,%d}\n",
			ra_d_o_timer_i.d_o_timer_en , ra_d_o_timer_i.d_o_timer_value);

			BB_DBG(bb, DBG_RA, "RA d_o_timer H2C: %x\n", bb_h2c[0]);

			ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RA_D_O_TIMER, HALBB_H2C_RA, bb_h2c);
		} else {
			BB_DBG(bb, DBG_RA, "No Link ! RA d_o_timer cmd fail!\n");
		}
	}  else if (val[0] == 6) {
		bb_h2c = (u32 *)h2c_ra_cls;
		cmdlen = 1;

		h2c_ra_cls[0] = (u8)(val[1] & 0x01); /*ra_cls_en*/
		h2c_ra_cls[1] = 0; /*rsvd*/
		h2c_ra_cls[2] = 0; /*rsvd*/
		h2c_ra_cls[3] = 0; /*rsvd*/

		BB_DBG(bb, DBG_RA, "RA cls head/tail en=%d\n",
		h2c_ra_cls[0]);

		BB_DBG(bb, DBG_RA, "RA cls head/tail H2C: %x\n", bb_h2c[0]);

		ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RA_CLS, HALBB_H2C_RA, bb_h2c);
	} else if (_os_strcmp(input[1], "dbgreg") == 0) {
		if (bb->ic_type & BB_IC_BE_SERIES) {
			struct bb_h2c_ra_tx_info ra_tx_i;
			struct bb_dbg_info *dbg = &bb->bb_dbg_i;
			struct bb_ra_dbgreg *ra_dbg_i = &dbg->ra_dbgreg_i;
			bb_h2c = (u32 *) &ra_tx_i;
			cmdlen = sizeof(struct bb_h2c_ra_tx_info);
			phl_sta = bb->phl_sta_info[(u16)val[1]];
			if (!phl_sta) {
				BB_WARNING("[%s][1] macid=%d\n", __func__, val[1]);
				return;
			}
			ra_tx_i.macid = (u16)val[1];
			BB_DBG(bb, DBG_RA, "RA TX log macid = %d,h2c=%x\n", ra_tx_i.macid, bb_h2c[0]);
			ret_val = halbb_fill_h2c_cmd(bb, cmdlen, RA_H2C_RA_TX_INFO, HALBB_H2C_RA, bb_h2c);
			halbb_ra_dbgreg_cnsl(bb, _used, output, _out_len);
		} else {
			halbb_get_ra_dbgreg(bb);
			halbb_ra_dbgreg_cnsl(bb, _used, output, _out_len);
		}
	} else {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			 "Set Err\n");
	}
}

#endif


