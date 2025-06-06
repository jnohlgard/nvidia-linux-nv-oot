/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#include "halbb_precomp.h"

/*============================ [Tx Settings] =============================*/
void halbb_set_pmac_tx(struct bb_info *bb, struct halbb_pmac_info *tx_info,
		       enum phl_phy_idx phy_idx)
{

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);
	#ifdef HALBB_FW_OFLD_SUPPORT
	halbb_fwofld_bitmap_en(bb, true, FW_OFLD_BB_API);
	#endif

	if (bb->bb_dbg_i.cr_mp_recorder_en) {
		if (tx_info->en_pmac_tx)
			BB_TRACE("[MP] // <====== PMAC Tx start ======>\n");
		else
			BB_TRACE("[MP] // <====== PMAC Tx stop ======>\n");
	}

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		#if 0
			if (halbb_check_fw_ofld(bb))
				halbb_fwofld_set_pmac_tx_8852a_2(bb, tx_info, phy_idx);
			else
				halbb_set_pmac_tx_8852a_2(bb, tx_info, phy_idx);
		#else
			halbb_set_pmac_tx_8852a_2(bb, tx_info, phy_idx);
		#endif
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		#if 0
			if (halbb_check_fw_ofld(bb))
				halbb_fwofld_set_pmac_tx_8852b(bb, tx_info, phy_idx);
			else
				halbb_set_pmac_tx_8852b(bb, tx_info, phy_idx);
		#else
			halbb_set_pmac_tx_8852b(bb, tx_info, phy_idx);
		#endif	
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_set_pmac_tx_8852c(bb, tx_info, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		halbb_set_pmac_tx_8192xb(bb, tx_info, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		halbb_set_pmac_tx_8851b(bb, tx_info, phy_idx);
		break;
	#endif

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		halbb_set_pmac_tx_1115(bb, tx_info, phy_idx);
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		halbb_set_pmac_tx_8922a(bb, tx_info, phy_idx);
		break;
	#endif

	default:
		break;
	}

	if (bb->bb_dbg_i.cr_mp_recorder_en) {
		if (tx_info->en_pmac_tx)
			BB_TRACE("[MP] // <====== PMAC Tx start [End] ======>\n");
		else
			BB_TRACE("[MP] // <====== PMAC Tx stop [End] ======>\n");
	}

	#ifdef HALBB_FW_OFLD_SUPPORT
	halbb_fwofld_bitmap_en(bb, false, FW_OFLD_BB_API);
	#endif
}

void halbb_set_tmac_tx(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
#ifdef HALBB_FW_OFLD_SUPPORT
	halbb_fwofld_bitmap_en(bb, true, FW_OFLD_BB_API);
#endif
	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_set_tmac_tx_8852a_2(bb, phy_idx);
		break;
	#endif
	
	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		halbb_set_tmac_tx_8852b(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_set_tmac_tx_8852c(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		halbb_set_tmac_tx_8192xb(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		halbb_set_tmac_tx_8851b(bb, phy_idx);
		break;
	#endif

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		halbb_set_tmac_tx_1115(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		halbb_set_tmac_tx_8922a(bb, phy_idx);
		break;
	#endif

	default:
		break;
	}

#ifdef HALBB_FW_OFLD_SUPPORT
	halbb_fwofld_bitmap_en(bb, false, FW_OFLD_BB_API);
#endif

}
/*============================ [LBK Module] =============================*/
bool halbb_cfg_lbk(struct bb_info *bb, bool lbk_en, bool is_dgt_lbk,
		   enum rf_path tx_path, enum rf_path rx_path,
		   enum channel_width bw, enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	if (bb->bb_dbg_i.cr_mp_recorder_en) {
		if (lbk_en)
			BB_TRACE("[MP] // <====== LBK Enable ======>\n");
		else
			BB_TRACE("[MP] // <====== LBK disable ======>\n");
	}

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_cfg_lbk_8852b(bb, lbk_en, is_dgt_lbk, tx_path,
					  rx_path, bw, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_cfg_lbk_8852c(bb, lbk_en, is_dgt_lbk, tx_path,
					  rx_path, bw, phy_idx);
		break;
	#endif
	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_cfg_lbk_8192xb(bb, lbk_en, is_dgt_lbk, tx_path,
					  rx_path, bw, phy_idx);
		break;
	#endif

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		rpt = halbb_cfg_lbk_1115(bb, lbk_en, is_dgt_lbk, tx_path,
					  rx_path, bw, phy_idx);
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		rpt = halbb_cfg_lbk_8922a(bb, lbk_en, is_dgt_lbk, tx_path,
					  rx_path, bw, phy_idx);
		break;
	#endif

	default:
		break;
	}

	if (bb->bb_dbg_i.cr_mp_recorder_en) {
		if (lbk_en)
			BB_TRACE("[MP] // <====== LBK Enable [End] ======>\n");
		else
			BB_TRACE("[MP] // <====== LBK disable [End] ======>\n");
	}

	return rpt;
}

bool halbb_cfg_lbk_cck(struct bb_info *bb, bool lbk_en, bool is_dgt_lbk,
		       enum rf_path tx_path, enum rf_path rx_path,
		       enum channel_width bw, enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_cfg_lbk_cck_8852c(bb, lbk_en, is_dgt_lbk, tx_path,
					      rx_path, bw, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_cfg_lbk_cck_8192xb(bb, lbk_en, is_dgt_lbk, tx_path,
					      rx_path, bw, phy_idx);
		break;
	#endif
	default:
		break;
	}

	return rpt;
}

void halbb_tx_triangular_en(struct bb_info *bb, bool en, enum phl_phy_idx phy_idx)
{

	BB_DBG(bb, DBG_PHY_CONFIG, "<====== %s ======>\n", __func__);

	switch (bb->ic_type) {

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		halbb_tx_triangular_en_8922a(bb, en, phy_idx);
		break;
	#endif

	default:
		break;
	}

}

/*============================ [Power Module] =============================*/
bool halbb_set_txpwr_dbm(struct bb_info *bb, s16 pwr_dbm,
			 enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	if (phl_mp_is_tmac_mode(bb->phl_com)) {
		BB_WARNING("[%s] mode=%d\n", __func__, bb->phl_com->drv_mode);
		//return false;
	}

	if (bb->bb_dbg_i.cr_mp_recorder_en)
		BB_TRACE("[MP] // <====== Set PMAC Tx pwr ======>\n");

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_set_txpwr_dbm_8852a_2(bb, pwr_dbm, phy_idx);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_set_txpwr_dbm_8852b(bb, pwr_dbm, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_set_txpwr_dbm_8852c(bb, pwr_dbm, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_set_txpwr_dbm_8192xb(bb, pwr_dbm, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_set_txpwr_dbm_8851b(bb, pwr_dbm, phy_idx);
		break;
	#endif

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		rpt = halbb_set_txpwr_dbm_1115(bb, pwr_dbm, phy_idx);
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		rpt = halbb_set_txpwr_dbm_8922a(bb, pwr_dbm, phy_idx);
		break;
	#endif

	default:
		rpt = false;
		break;
	}

	if (bb->bb_dbg_i.cr_mp_recorder_en)
		BB_TRACE("[MP] // <====== Set PMAC Tx pwr [End] ======>\n");

	return rpt;
}

s16 halbb_get_txpwr_dbm(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	s16 rpt;

	if (phl_mp_is_tmac_mode(bb->phl_com)) {
		BB_WARNING("[%s] mode=%d\n", __func__, bb->phl_com->drv_mode);
		return false;
	}

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_get_txpwr_dbm_8852a_2(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_get_txpwr_dbm_8852b(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_get_txpwr_dbm_8852c(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_get_txpwr_dbm_8192xb(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_get_txpwr_dbm_8851b(bb, phy_idx);
		break;
	#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

s16 halbb_get_txinfo_txpwr_dbm(struct bb_info *bb)
{
	s16 rpt;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_get_txinfo_txpwr_dbm_8852a_2(bb);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_get_txinfo_txpwr_dbm_8852b(bb);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_get_txinfo_txpwr_dbm_8852c(bb);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_get_txinfo_txpwr_dbm_8192xb(bb);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_get_txinfo_txpwr_dbm_8851b(bb);
		break;
	#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_set_cck_txpwr_idx(struct bb_info *bb, u16 pwr_idx,
			     enum rf_path tx_path)
{
	bool rpt = true;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_set_cck_txpwr_idx_8852a_2(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_set_cck_txpwr_idx_8852b(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_set_cck_txpwr_idx_8852c(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_set_cck_txpwr_idx_8192xb(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_set_cck_txpwr_idx_8851b(bb, pwr_idx, tx_path);
		break;
	#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

u16 halbb_get_cck_txpwr_idx(struct bb_info *bb, enum rf_path tx_path)
{
	u16 rpt;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_get_cck_txpwr_idx_8852a_2(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_get_cck_txpwr_idx_8852b(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_get_cck_txpwr_idx_8852c(bb, tx_path);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_get_cck_txpwr_idx_8192xb(bb, tx_path);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_get_cck_txpwr_idx_8851b(bb, tx_path);
		break;
	#endif

	default:
		rpt= false;
		break;
	}
	return rpt;
}

s16 halbb_get_cck_ref_dbm(struct bb_info *bb, enum rf_path tx_path)
{
	s16 rpt;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_get_cck_ref_dbm_8852a_2(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_get_cck_ref_dbm_8852b(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_get_cck_ref_dbm_8852c(bb, tx_path);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_get_cck_ref_dbm_8192xb(bb, tx_path);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_get_cck_ref_dbm_8851b(bb, tx_path);
		break;
	#endif

	default:
		rpt= false;
		break;
	}
	return rpt;
}


bool halbb_set_vht_mu_user_idx(struct bb_info *bb, bool en, u8 idx,
			       enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	switch (bb->ic_type) {

#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_set_vht_mu_user_idx_8852c(bb, en, idx, phy_idx);
		break;
#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_set_ofdm_txpwr_idx(struct bb_info *bb, u16 pwr_idx,
			      enum rf_path tx_path)
{
	bool rpt = true;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_set_ofdm_txpwr_idx_8852a_2(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_set_ofdm_txpwr_idx_8852b(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_set_ofdm_txpwr_idx_8852c(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_set_ofdm_txpwr_idx_8192xb(bb, pwr_idx, tx_path);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_set_ofdm_txpwr_idx_8851b(bb, pwr_idx, tx_path);
		break;
	#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

u16 halbb_get_ofdm_txpwr_idx(struct bb_info *bb, enum rf_path tx_path)
{
	u16 rpt;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_get_ofdm_txpwr_idx_8852a_2(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_get_ofdm_txpwr_idx_8852b(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_get_ofdm_txpwr_idx_8852c(bb, tx_path);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_get_ofdm_txpwr_idx_8192xb(bb, tx_path);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_get_ofdm_txpwr_idx_8851b(bb, tx_path);
		break;
	#endif

	default:
		rpt= false;
		break;
	}
	return rpt;
}

s16 halbb_get_ofdm_ref_dbm(struct bb_info *bb, enum rf_path tx_path)
{
	s16 rpt;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_get_ofdm_ref_dbm_8852a_2(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_get_ofdm_ref_dbm_8852b(bb, tx_path);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_get_ofdm_ref_dbm_8852c(bb, tx_path);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_get_ofdm_ref_dbm_8192xb(bb, tx_path);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_get_ofdm_ref_dbm_8851b(bb, tx_path);
		break;
	#endif

	default:
		rpt= false;
		break;
	}
	return rpt;
}


/*============================ [Others] =============================*/
bool halbb_chk_tx_idle(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		rpt = halbb_chk_tx_idle_8852a_2(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		rpt = halbb_chk_tx_idle_8852b(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		rpt = halbb_chk_tx_idle_8852c(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		rpt = halbb_chk_tx_idle_8192xb(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		rpt = halbb_chk_tx_idle_8851b(bb, phy_idx);
		break;
	#endif

	default:
		#ifdef HALBB_COMPILE_BE_SERIES
		if (bb->bb_80211spec == BB_BE_IC) {
			rpt = true;
		} else
		#endif
		{
		rpt = false;
		}
		break;
	}

	return rpt;
}

void halbb_dpd_bypass(struct bb_info *bb, bool pdp_bypass,
		      enum phl_phy_idx phy_idx)
{
	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_dpd_bypass_8852a_2(bb, pdp_bypass, phy_idx);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		//halbb_dpd_bypass_8852b(bb, pdp_bypass, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_dpd_bypass_8852c(bb, pdp_bypass, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		halbb_dpd_bypass_8192xb(bb, pdp_bypass, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		halbb_dpd_bypass_8851b(bb, pdp_bypass, phy_idx);
		break;
	#endif
	
	default:
		break;
	}
}

void halbb_backup_info(struct bb_info *bb_0, enum phl_phy_idx phy_idx)
{
	struct bb_info *bb = bb_0;
		
#ifdef HALBB_DBCC_SUPPORT
	HALBB_GET_PHY_PTR(bb_0, bb, phy_idx);
#endif

	BB_DBG(bb, DBG_DBCC, "[%s] phy_idx=%d\n", __func__, phy_idx);

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_backup_info_8852a_2(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		halbb_backup_info_8852b(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_backup_info_8852c(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		halbb_backup_info_8192xb(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		halbb_backup_info_8851b(bb, phy_idx);
		break;
	#endif

	default:
		break;
	}
}

void halbb_restore_info(struct bb_info *bb_0, enum phl_phy_idx phy_idx)
{
	struct bb_info *bb = bb_0;

#ifdef HALBB_FW_OFLD_SUPPORT
	halbb_fwofld_bitmap_en(bb, true, FW_OFLD_BB_API);
#endif

#ifdef HALBB_DBCC_SUPPORT
	HALBB_GET_PHY_PTR(bb_0, bb, phy_idx);
#endif

	BB_DBG(bb, DBG_DBCC, "[%s] phy_idx=%d\n", __func__, phy_idx);

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_restore_info_8852a_2(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		halbb_restore_info_8852b(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		halbb_restore_info_8852c(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		halbb_restore_info_8192xb(bb, phy_idx);
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		halbb_restore_info_8851b(bb, phy_idx);
		break;
	#endif

	default:
		break;
	}

#ifdef HALBB_FW_OFLD_SUPPORT
	halbb_fwofld_bitmap_en(bb, false, FW_OFLD_BB_API);
#endif

}

enum rtw_hal_status halbb_set_txsc(struct bb_info *bb, u8 txsc,
				   enum phl_phy_idx phy_idx)
{
	enum rtw_hal_status rpt = RTW_HAL_STATUS_FAILURE;

	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		if (halbb_set_txsc_8852a_2(bb, txsc, phy_idx))
			rpt = RTW_HAL_STATUS_SUCCESS;
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		if (halbb_set_txsc_8852b(bb, txsc, phy_idx))
			rpt = RTW_HAL_STATUS_SUCCESS;
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		if (halbb_set_txsc_8852c(bb, txsc, phy_idx))
			rpt = RTW_HAL_STATUS_SUCCESS;
		break;
	#endif

	#ifdef BB_8192XB_SUPPORT
	case BB_RTL8192XB:
		if (halbb_set_txsc_8192xb(bb, txsc, phy_idx))
			rpt = RTW_HAL_STATUS_SUCCESS;
		break;
	#endif

	#ifdef BB_8851B_SUPPORT
	case BB_RTL8851B:
		if (halbb_set_txsc_8851b(bb, txsc, phy_idx))
			rpt = RTW_HAL_STATUS_SUCCESS;
		break;
	#endif

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		#ifdef BB_1115_DVLP_SPF
		rpt = RTW_HAL_STATUS_NOT_SUPPORT;
		#endif
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		rpt = RTW_HAL_STATUS_NOT_SUPPORT;
		break;
	#endif

	default:
		rpt = RTW_HAL_STATUS_FAILURE;
		break;
	}

	return rpt;
}

enum rtw_hal_status halbb_set_txsb(struct bb_info *bb, u8 txsb,
				   enum phl_phy_idx phy_idx)
{
	enum rtw_hal_status rpt = RTW_HAL_STATUS_FAILURE;

	switch (bb->ic_type) {

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		if (halbb_set_txsb_1115(bb, txsb, phy_idx))
			rpt = RTW_HAL_STATUS_SUCCESS;
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		if (halbb_set_txsb_8922a(bb, txsb, phy_idx))
			rpt = RTW_HAL_STATUS_SUCCESS;
		break;
	#endif

	default:
		rpt = RTW_HAL_STATUS_FAILURE;
		break;
	}

	return rpt;
}

bool halbb_set_bss_color(struct bb_info *bb, u8 bss_color, 
			 enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	switch (bb->ic_type) {
	
	#ifdef BB_8852A_2_SUPPORT
		case BB_RTL8852A:
			rpt = halbb_set_bss_color_8852a_2(bb, bss_color, phy_idx);
			break;
	#endif

	#ifdef BB_8852B_SUPPORT
		case BB_RTL8852B:
			rpt = halbb_set_bss_color_8852b(bb, bss_color, phy_idx);
			break;
	#endif

	#ifdef BB_8852C_SUPPORT
		case BB_RTL8852C:
			rpt = halbb_set_bss_color_8852c(bb, bss_color, phy_idx);
			break;
	#endif

	#ifdef BB_8192XB_SUPPORT
		case BB_RTL8192XB:
			rpt = halbb_set_bss_color_8192xb(bb, bss_color, phy_idx);
			break;
	#endif

	#ifdef BB_8851B_SUPPORT
		case BB_RTL8851B:
			rpt = halbb_set_bss_color_8851b(bb, bss_color, phy_idx);
			break;
	#endif

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		rpt = halbb_set_bss_color_1115(bb, bss_color, phy_idx);
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		rpt = halbb_set_bss_color_8922a(bb, bss_color, phy_idx);
		break;
	#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

bool halbb_set_sta_id(struct bb_info *bb, u16 sta_id, enum phl_phy_idx phy_idx)
{
	bool rpt = true;

	switch (bb->ic_type) {
	
	#ifdef BB_8852A_2_SUPPORT
		case BB_RTL8852A:
			rpt = halbb_set_sta_id_8852a_2(bb, sta_id, phy_idx);
			break;
	#endif

	#ifdef BB_8852B_SUPPORT
		case BB_RTL8852B:
			rpt = halbb_set_sta_id_8852b(bb, sta_id, phy_idx);
			break;
	#endif

	#ifdef BB_8852C_SUPPORT
		case BB_RTL8852C:
			rpt = halbb_set_sta_id_8852c(bb, sta_id, phy_idx);
			break;
	#endif

	#ifdef BB_8192XB_SUPPORT
		case BB_RTL8192XB:
			rpt = halbb_set_sta_id_8192xb(bb, sta_id, phy_idx);
			break;
	#endif

	#ifdef BB_8851B_SUPPORT
		case BB_RTL8851B:
			rpt = halbb_set_sta_id_8851b(bb, sta_id, phy_idx);
			break;
	#endif

	#ifdef BB_1115_SUPPORT
	case BB_RLE1115:
		rpt = halbb_set_sta_id_1115(bb, sta_id, phy_idx);
		break;
	#endif

	#ifdef BB_8922A_SUPPORT
	case BB_RTL8922A:
		rpt = halbb_set_sta_id_8922a(bb, sta_id, phy_idx);
		break;
	#endif

	default:
		rpt = false;
		break;
	}

	return rpt;
}

void halbb_pmac_cmd_set(struct bb_info *bb, struct halbb_pmac_info *txinfo,
			char input[][16], u32 *_used, char *output, u32 *_out_len)
{
	u32 val[10] = {0};

	if (_os_strcmp(input[2], "list") == 0 || _os_strcmp(input[2], "-h") == 0) {
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "en_pmac_tx,  is_cck, cck_lbk_en\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "    period, tx_time,     tx_cnt\n");
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "      mode\n");
		return;
	}

	if (_os_strcmp(input[2], "en_pmac_tx") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		txinfo->en_pmac_tx = (u8)val[0];
	} else if (_os_strcmp(input[2], "is_cck") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		txinfo->is_cck = (u8)val[0];
	} else if (_os_strcmp(input[2], "tx_cnt") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		txinfo->tx_cnt = (u16)val[0];
	} else if (_os_strcmp(input[2], "period") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		txinfo->period = (u16)val[0];
	} else if (_os_strcmp(input[2], "tx_time") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		txinfo->tx_time = (u8)val[0];
	} else if (_os_strcmp(input[2], "cck_lbk_en") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		txinfo->cck_lbk_en = (bool)val[0];
	} else if (_os_strcmp(input[2], "mode") == 0) {
		HALBB_SCAN(input[3], DCMD_DECIMAL, &val[0]);
		txinfo->mode = (u8)val[0];
	}
}
void halbb_pmac_cmd_show(struct bb_info *bb, struct halbb_pmac_info *txinfo,
			char input[][16], u32 *_used, char *output, u32 *_out_len)
{
	BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
		    "en_pmac_tx : %5d, is_cck : %5d\n", txinfo->en_pmac_tx, txinfo->is_cck);
	BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
		    "cck_lbk_en : %5d, period : %5d\n", txinfo->cck_lbk_en, txinfo->period);
	BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
		    "   tx_time : %5d, tx_cnt : %5d\n", txinfo->tx_time, txinfo->tx_cnt);
	BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
		    "      mode : %5d\n", txinfo->mode);
}

void halbb_pmac_cmd_default(struct bb_info *bb, struct halbb_pmac_info *txinfo,
			char input[][16], u32 *_used, char *output, u32 *_out_len)
{
	txinfo->en_pmac_tx = 0;
	txinfo->is_cck = 1;
	txinfo->tx_cnt = 0;
	txinfo->period = 200;
	txinfo->tx_time = 150;
	txinfo->cck_lbk_en = 0;
	txinfo->mode = 1;
}

void halbb_pmac_tx_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		       char *output, u32 *_out_len)
{
	u32 val[10] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	struct halbb_pmac_info *txinfo = &bb->pmac_in;

	if (_os_strcmp(input[1], "-h") == 0) {
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "{bw} {pri_ch bw} {phy_idx}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "dyn_pmac_tri {en}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "pmac_tri {en}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "pwr_comp_en {en}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "set : set input argument\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "show : show parameters setting\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "default : set parameters as default value\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "trig_tx {1/2}: start/stop tx, 1 : start, 2: stop\n");

	} else if (_os_strcmp(input[1], "tx_path") == 0) {
		HALBB_SCAN(input[1], DCMD_DECIMAL, &val[0]);

		//halbb_set_pmac_tx_path(bb, (enum bb_path)val[0]);

		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Cfg Tx Path API \n");
	} else if (_os_strcmp(input[1], "dyn_pmac_tri") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		bb->dyn_pmac_tri_en = (bool)val[0];
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "set dyn_pmac_tri_en = %d\n", bb->dyn_pmac_tri_en);
	} else if (_os_strcmp(input[1], "pmac_tri") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		bb->pmac_tri_en = (bool)val[0];
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "set pmac_tri_en = %d\n", bb->pmac_tri_en);
	} else if (_os_strcmp(input[1], "pwr_comp_en") == 0) {
		HALBB_SCAN(input[2], DCMD_DECIMAL, &val[0]);
		bb->pwr_comp_en = (bool)val[0];
		BB_DBG_CNSL(*_out_len, *_used, output + *_used, *_out_len - *_used,
			    "set pwr_comp_en = %d\n", bb->pwr_comp_en);
	} else if (_os_strcmp(input[1], "set") == 0) {
		halbb_pmac_cmd_set(bb, txinfo, input, _used, output, _out_len);
	} else if (_os_strcmp(input[1], "show") == 0) {
		halbb_pmac_cmd_show(bb, txinfo, input, _used, output, _out_len);
	} else if (_os_strcmp(input[1], "default") == 0) {
		halbb_pmac_cmd_default(bb, txinfo, input, _used, output, _out_len);
	} else if (_os_strcmp(input[1], "trig_tx") == 0) {
		if (_os_strcmp(input[2], "1") == 0)
			txinfo->en_pmac_tx = 1;
		else if (_os_strcmp(input[2], "2") == 0) 
			txinfo->en_pmac_tx = 0;
		halbb_set_pmac_tx(bb, txinfo, bb->bb_phy_idx);
	}
	*_used = used;
	*_out_len = out_len;
}

