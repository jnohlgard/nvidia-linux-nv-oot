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
#define _HAL_BTC_DEF_C_
#include "../hal_headers_le.h"
#include "hal_btc.h"
#include "halbtc_fw.h"
#include "halbtc_action.h"

#ifdef CONFIG_BTCOEX

#define case_role(src) \
	case PHL_RTYPE_##src: return #src
#define case_mrole(src) \
	case BTC_WLMROLE_##src: return #src
#define case_slot(src) \
	case CXST_##src: return #src
#define case_reg(src) \
	case REG_##src: return #src
#define case_tdma(src) \
	case CXTDMA_##src: return #src
#define case_trace(src) \
	case CXSTEP_##src: return #src
#define case_branch(src) \
	case BTC_BRANCH_##src: return #src
#define case_rxflctrl_(src) \
	case CXFLC_##src: return #src
#define case_wllink(src) \
	case BTC_WLINK_##src: return #src
#define case_antpath(src) \
	case BTC_ANT_##src: return #src
#define case_gdbg(src) \
	case BTC_DBG_##src: return #src
#define case_chip(src) \
	case CHIP_##src: return #src
#define case_event(src) \
	case CXEVNT_##src: return #src
#define case_wlmode(src) \
	case WLAN_MD_##src: return #src
#define case_wlbw(src) \
	case CHANNEL_WIDTH_##src: return #src
#define case_cxp(src) \
	case BTC_CXP_##src: return #src
#define case_rftype(src) \
	case RF_##src: return #src
#define case_mstate(src) \
	case MLME_##src: return #src
#define case_rate(src) \
	case RTW_DATA_RATE_##src: return #src
#define case_polut(src) \
	case BTC_PLT_##src: return #src
#define case_band(src) \
	case BAND_ON_##src: return #src
#define case_cxstate(src) \
	case BTC_##src: return #src
#define case_fddt_state(src) \
	case BTC_FDDT_STATE_##src: return #src
#define case_fddt_type(src) \
	case BTC_FDDT_TYPE_##src: return #src
#define case_fddt_norun(src) \
	case BTC_NFRSN_##src: return #src
#define case_fddt_tstate(src) \
	case CX_FDDT_STATE_##src: return #src
#define case_fddt_tphase(src) \
	case CX_FDDT_PHASE_##src: return #src
#define case_fddt_chkmap(src) \
	case CX_FDDT_CHKMAP_##src: return #src
#define case_dmerr(src) \
	case BTC_DMERR_##src: return #src
#define case_fwerr(src) \
	case BTC_FWERR_##src: return #src
#define case_rptmatch(src) \
	case BTC_RPT_TYPE_##src: return #src
#define case_h2cerr(src) \
	case SET_##src: return #src
#define case_vendorid(src) \
	case BTC_VID_##src: return #src
#define case_path(src) \
	case RF_PATH_##src: return #src

const char *id_to_str(u8 type, u32 id)
{
	switch(type) {
	case BTC_STR_ROLE:
		switch (id) {
		case_role(NONE);
		case_role(STATION);
		case_role(AP);
		case_role(VAP);
		case_role(ADHOC);
		case_role(ADHOC_MASTER);
		case_role(MESH);
		case_role(MONITOR);
		case_role(P2P_DEVICE);
		case_role(P2P_GC);
		case_role(P2P_GO);
		case_role(NAN);
		}
		break;
	case BTC_STR_MROLE:
		switch (id) {
		case_mrole(NONE);
		case_mrole(STA_GC);
		case_mrole(STA_GC_NOA);
		case_mrole(STA_GO);
		case_mrole(STA_GO_NOA);
		case_mrole(STA_STA);
		}
		break;
	case BTC_STR_SLOT:
		switch (id) {
		case_slot(OFF);
		case_slot(B2W);
		case_slot(W1);
		case_slot(W2);
		case_slot(W2B);
		case_slot(B1);
		case_slot(B2);
		case_slot(B3);
		case_slot(B4);
		case_slot(LK);
		case_slot(BLK);
		case_slot(E2G);
		case_slot(E5G);
		case_slot(EBT);
		case_slot(ENULL);
		case_slot(WLK);
		case_slot(W1FDD);
		case_slot(B1FDD);
		}
		break;
	case BTC_STR_REG:
		switch (id) {
		case_reg(MAC);
		case_reg(BB);
		case_reg(RF);
		case_reg(BT_RF);
		case_reg(BT_MODEM);
		case_reg(BT_BLUEWIZE);
		case_reg(BT_VENDOR);
		case_reg(BT_LE);
		}
		break;
	case BTC_STR_TDMA:
		switch (id) {
		case_tdma(OFF);
		case_tdma(FIX);
		case_tdma(AUTO);
		case_tdma(AUTO2);
		}
		break;
	case BTC_STR_TRACE:
		switch (id) {
		case_trace(NONE);
		case_trace(EVNT);
		case_trace(SLOT);
		}
		break;
	case BTC_STR_BRANCH:
		switch (id) {
		case_branch(FORMAL);
		case_branch(UPSTREAM);
		case_branch(HP);
		case_branch(LENOVO);
		case_branch(XBOX);
		case_branch(HW);
		case_branch(ROKU);
		case_branch(LG);
		case_branch(SAMSUNG);
		case_branch(AMAZON);
		}
		break;
	case BTC_STR_RXFLCTRL:
		switch (id) {
		case_rxflctrl_(OFF);
		case_rxflctrl_(NULLP);
		case_rxflctrl_(QOSNULL);
		case_rxflctrl_(CTS);
		}
		break;
	case BTC_STR_WLLINK:
		switch (id) {
		case_wllink(NOLINK);
		case_wllink(2G_STA);
		case_wllink(2G_GO);
		case_wllink(2G_GC);
		case_wllink(2G_AP);
		case_wllink(2G_SCC);
		case_wllink(2G_MCC);
		case_wllink(25G_MCC);
		case_wllink(25G_DBCC);
		case_wllink(5G);
		case_wllink(2G_NAN);
		case_wllink(OTHER);
		}
		break;
	case BTC_STR_ANTPATH:
		switch (id) {
		case_antpath(WPOWERON);
		case_antpath(WINIT);
		case_antpath(WONLY);
		case_antpath(WOFF);
		case_antpath(W2G);
		case_antpath(W5G);
		case_antpath(W25G);
		case_antpath(FREERUN);
		case_antpath(FDDTRAIN);
		case_antpath(WRFK);
		case_antpath(WRFK2);
		case_antpath(BRFK);
		}
		break;
	case BTC_STR_GDBG:
		switch (id) {
		case_gdbg(GNT_BT);
		case_gdbg(GNT_WL);
		case_gdbg(BCN_EARLY);
		case_gdbg(WL_NULL0);
		case_gdbg(WL_NULL1);
		case_gdbg(WL_RXISR);
		case_gdbg(TDMA_ENTRY);
		case_gdbg(A2DP_EMPTY);
		case_gdbg(BT_RETRY);
		case_gdbg(BT_RELINK);
		case_gdbg(SLOT_WL);
		case_gdbg(SLOT_BT);
		case_gdbg(WL_ERR);
		case_gdbg(WL_OK);
		case_gdbg(SLOT_B2W);
		case_gdbg(SLOT_W1);
		case_gdbg(SLOT_W2);
		case_gdbg(SLOT_W2B);
		case_gdbg(SLOT_B1);
		case_gdbg(SLOT_B2);
		case_gdbg(SLOT_B3);
		case_gdbg(SLOT_B4);
		case_gdbg(SLOT_LK);
		case_gdbg(SLOT_E2G);
		case_gdbg(SLOT_E5G);
		case_gdbg(SLOT_EBT);
		case_gdbg(SLOT_WLK);
		case_gdbg(SLOT_B1FDD);
		case_gdbg(BT_CHANGE);
		case_gdbg(WL_CCA);
		case_gdbg(BT_LEAUDIO);
		case_gdbg(USER_DEF);
		}
		break;
	case BTC_STR_CHIPID:
		switch (id) {
		case_chip(WIFI6_8852A);
		case_chip(WIFI6_8852B);
		case_chip(WIFI6_8852C);
		case_chip(WIFI6_8842A);
#ifdef BTC_8852D_SUPPORT
		case_chip(WIFI6_8852D);
#endif
		case_chip(WIFI6_8852BP);
		case_chip(WIFI6_8851B);
#ifdef BTC_8852BT_SUPPORT
		case_chip(WIFI6_8852BT);
#endif
#ifdef BTC_8852BT_SUPPORT
		case_chip(WIFI6_8852BPT);
#endif
		}
		break;
	case BTC_STR_POLUT:
		switch (id) {
		case_polut(NONE);
		case_polut(GNT_BT_TX);
		case_polut(GNT_BT_RX);
		case_polut(GNT_WL);
		case_polut(BT);
		case_polut(ALL);
		}
		break;
	case BTC_STR_EVENT:
		switch(id) {
		case_event(TDMA_ENTRY);
		case_event(WL_TMR);
		case_event(B1_TMR);
		case_event(B2_TMR);
		case_event(B3_TMR);
		case_event(B4_TMR);
		case_event(W2B_TMR);
		case_event(B2W_TMR);
		case_event(BCN_EARLY);
		case_event(A2DP_EMPTY);
		case_event(LK_END);
		case_event(RX_ISR);
		case_event(RX_FC0);
		case_event(RX_FC1);
		case_event(BT_RELINK);
		case_event(BT_RETRY);
		case_event(E2G);
		case_event(E5G);
		case_event(EBT);
		case_event(ENULL);
		case_event(DRV_WLK);
		case_event(BCN_OK);
		case_event(BT_CHANGE);
		case_event(EBT_EXTEND);
		case_event(E2G_NULL1);
		case_event(B1FDD_TMR);
#ifdef BTC_LE_INIT_TO_WL_SLOT
		case_event(LE_INIT_END);
#endif
		}
		break;
	case BTC_STR_WLMODE:
		switch(id) {
		case_wlmode(INVALID);
		case_wlmode(11B);
		case_wlmode(11A);
		case_wlmode(11G);
		case_wlmode(11N);
		case_wlmode(11AC);
		case_wlmode(11AX);
		case_wlmode(11BG);
		case_wlmode(11GN);
		case_wlmode(11AN);
		case_wlmode(11BN);
		case_wlmode(11BGN);
		case_wlmode(11BGAC);
		case_wlmode(11BGAX);
		case_wlmode(11A_AC);
		case_wlmode(11A_AX);
		case_wlmode(11AGN);
		case_wlmode(11ABGN);
		case_wlmode(24G_MIX);
		case_wlmode(5G_MIX);
		case_wlmode(MAX);
		}
		break;
	case BTC_STR_WLBW:
		switch(id) {
		case_wlbw(20);
		case_wlbw(40);
		case_wlbw(80);
		case_wlbw(160);
		case_wlbw(80_80);
		case_wlbw(5);
		case_wlbw(10);
		case_wlbw(MAX);
		}
		break;
	case BTC_STR_RFTYPE:
		switch(id) {
		case_rftype(1T1R);
		case_rftype(1T2R);
		case_rftype(2T3R);
		case_rftype(2T4R);
		case_rftype(3T3R);
		case_rftype(3T4R);
		case_rftype(4T4R);
		}
		break;
	case BTC_STR_POLICY:
		switch(id) {
		case_cxp(OFF_BT);
		case_cxp(OFF_WL);
		case_cxp(OFF_EQ0);
		case_cxp(OFF_EQ1);
		case_cxp(OFF_EQ2);
		case_cxp(OFF_EQ3);
		case_cxp(OFF_EQ4);
		case_cxp(OFF_EQ5);
		case_cxp(OFF_BWB0);
		case_cxp(OFF_BWB1);
		case_cxp(OFF_BWB2);
		case_cxp(OFF_BWB3);
		case_cxp(OFF_WL2);
		case_cxp(OFFB_BWB0);
		case_cxp(OFFE_2GBWISOB);
		case_cxp(OFFE_2GISOB);
		case_cxp(OFFE_2GBWMIXB);
		case_cxp(OFFE_2GBWMIXB2);
		case_cxp(OFFE_WL);
		case_cxp(FIX_TD3030);
		case_cxp(FIX_TD5050);
		case_cxp(FIX_TD2030);
		case_cxp(FIX_TD4010);
		case_cxp(FIX_TD7010);
		case_cxp(FIX_TD2060);
		case_cxp(FIX_TD3060);
		case_cxp(FIX_TD2080);
		case_cxp(FIX_TDW1B1);
		case_cxp(FIX_TD4010ISO);
		case_cxp(FIX_TD4010ISO_DL);
		case_cxp(FIX_TD4010ISO_UL);
		case_cxp(PFIX_TD3030);
		case_cxp(PFIX_TD5050);
		case_cxp(PFIX_TD2030);
		case_cxp(PFIX_TD2060);
		case_cxp(PFIX_TD3070);
		case_cxp(PFIX_TD2080);
		case_cxp(PFIX_TDW1B1);
		case_cxp(AUTO_TD50B1);
		case_cxp(AUTO_TD60B1);
		case_cxp(AUTO_TD20B1);
		case_cxp(AUTO_TDW1B1);
		case_cxp(AUTO_TDW1B1_CXTPS);
		case_cxp(PAUTO_TD50B1);
		case_cxp(PAUTO_TD60B1);
		case_cxp(PAUTO_TD20B1);
		case_cxp(PAUTO_TDW1B1);
		case_cxp(PAUTO_FDDT1);
		case_cxp(AUTO2_TD3050);
		case_cxp(AUTO2_TD3070);
		case_cxp(AUTO2_TD5050);
		case_cxp(AUTO2_TD6060);
		case_cxp(AUTO2_TD2080);
		case_cxp(AUTO2_TDW1B4);
		case_cxp(PAUTO2_TD3050);
		case_cxp(PAUTO2_TD3070);
		case_cxp(PAUTO2_TD5050);
		case_cxp(PAUTO2_TD6060);
		case_cxp(PAUTO2_TD2080);
		case_cxp(PAUTO2_TDW1B4);
		}
		break;
	case BTC_STR_MSTATE:
		switch(id) {
		case_mstate(NO_LINK);
		case_mstate(LINKING);
		case_mstate(LINKED);
		}
		break;
	case BTC_STR_RATE:
		switch(id) {
		case_rate(CCK1);
		case_rate(CCK2);
		case_rate(CCK5_5);
		case_rate(CCK11);
		case_rate(OFDM6);
		case_rate(OFDM9);
		case_rate(OFDM12);
		case_rate(OFDM18);
		case_rate(OFDM24);
		case_rate(OFDM36);
		case_rate(OFDM48);
		case_rate(OFDM54);
		case_rate(MCS0);
		case_rate(MCS1);
		case_rate(MCS2);
		case_rate(MCS3);
		case_rate(MCS4);
		case_rate(MCS5);
		case_rate(MCS6);
		case_rate(MCS7);
		case_rate(MCS8);
		case_rate(MCS9);
		case_rate(MCS10);
		case_rate(MCS11);
		case_rate(MCS12);
		case_rate(MCS13);
		case_rate(MCS14);
		case_rate(MCS15);
		case_rate(MCS16);
		case_rate(MCS17);
		case_rate(MCS18);
		case_rate(MCS19);
		case_rate(MCS20);
		case_rate(MCS21);
		case_rate(MCS22);
		case_rate(MCS23);
		case_rate(MCS24);
		case_rate(MCS25);
		case_rate(MCS26);
		case_rate(MCS27);
		case_rate(MCS28);
		case_rate(MCS29);
		case_rate(MCS30);
		case_rate(MCS31);
		case_rate(VHT_NSS1_MCS0);
		case_rate(VHT_NSS1_MCS1);
		case_rate(VHT_NSS1_MCS2);
		case_rate(VHT_NSS1_MCS3);
		case_rate(VHT_NSS1_MCS4);
		case_rate(VHT_NSS1_MCS5);
		case_rate(VHT_NSS1_MCS6);
		case_rate(VHT_NSS1_MCS7);
		case_rate(VHT_NSS1_MCS8);
		case_rate(VHT_NSS1_MCS9);
		case_rate(VHT_NSS2_MCS0);
		case_rate(VHT_NSS2_MCS1);
		case_rate(VHT_NSS2_MCS2);
		case_rate(VHT_NSS2_MCS3);
		case_rate(VHT_NSS2_MCS4);
		case_rate(VHT_NSS2_MCS5);
		case_rate(VHT_NSS2_MCS6);
		case_rate(VHT_NSS2_MCS7);
		case_rate(VHT_NSS2_MCS8);
		case_rate(VHT_NSS2_MCS9);
		case_rate(VHT_NSS3_MCS0);
		case_rate(VHT_NSS3_MCS1);
		case_rate(VHT_NSS3_MCS2);
		case_rate(VHT_NSS3_MCS3);
		case_rate(VHT_NSS3_MCS4);
		case_rate(VHT_NSS3_MCS5);
		case_rate(VHT_NSS3_MCS6);
		case_rate(VHT_NSS3_MCS7);
		case_rate(VHT_NSS3_MCS8);
		case_rate(VHT_NSS3_MCS9);
		case_rate(VHT_NSS4_MCS0);
		case_rate(VHT_NSS4_MCS1);
		case_rate(VHT_NSS4_MCS2);
		case_rate(VHT_NSS4_MCS3);
		case_rate(VHT_NSS4_MCS4);
		case_rate(VHT_NSS4_MCS5);
		case_rate(VHT_NSS4_MCS6);
		case_rate(VHT_NSS4_MCS7);
		case_rate(VHT_NSS4_MCS8);
		case_rate(VHT_NSS4_MCS9);
		case_rate(HE_NSS1_MCS0);
		case_rate(HE_NSS1_MCS1);
		case_rate(HE_NSS1_MCS2);
		case_rate(HE_NSS1_MCS3);
		case_rate(HE_NSS1_MCS4);
		case_rate(HE_NSS1_MCS5);
		case_rate(HE_NSS1_MCS6);
		case_rate(HE_NSS1_MCS7);
		case_rate(HE_NSS1_MCS8);
		case_rate(HE_NSS1_MCS9);
		case_rate(HE_NSS1_MCS10);
		case_rate(HE_NSS1_MCS11);
		case_rate(HE_NSS2_MCS0);
		case_rate(HE_NSS2_MCS1);
		case_rate(HE_NSS2_MCS2);
		case_rate(HE_NSS2_MCS3);
		case_rate(HE_NSS2_MCS4);
		case_rate(HE_NSS2_MCS5);
		case_rate(HE_NSS2_MCS6);
		case_rate(HE_NSS2_MCS7);
		case_rate(HE_NSS2_MCS8);
		case_rate(HE_NSS2_MCS9);
		case_rate(HE_NSS2_MCS10);
		case_rate(HE_NSS2_MCS11);
		case_rate(HE_NSS3_MCS0);
		case_rate(HE_NSS3_MCS1);
		case_rate(HE_NSS3_MCS2);
		case_rate(HE_NSS3_MCS3);
		case_rate(HE_NSS3_MCS4);
		case_rate(HE_NSS3_MCS5);
		case_rate(HE_NSS3_MCS6);
		case_rate(HE_NSS3_MCS7);
		case_rate(HE_NSS3_MCS8);
		case_rate(HE_NSS3_MCS9);
		case_rate(HE_NSS3_MCS10);
		case_rate(HE_NSS3_MCS11);
		case_rate(HE_NSS4_MCS0);
		case_rate(HE_NSS4_MCS1);
		case_rate(HE_NSS4_MCS2);
		case_rate(HE_NSS4_MCS3);
		case_rate(HE_NSS4_MCS4);
		case_rate(HE_NSS4_MCS5);
		case_rate(HE_NSS4_MCS6);
		case_rate(HE_NSS4_MCS7);
		case_rate(HE_NSS4_MCS8);
		case_rate(HE_NSS4_MCS9);
		case_rate(HE_NSS4_MCS10);
		case_rate(HE_NSS4_MCS11);
		case_rate(MAX);
		}
		break;
	case BTC_STR_BAND:
		switch(id) {
		case_band(24G);
		case_band(5G);
		case_band(6G);
		}
		break;
	case BTC_STR_CXSTATE:
		switch(id) {
		case_cxstate(WIDLE);
		case_cxstate(WBUSY_BNOSCAN);
		case_cxstate(WBUSY_BSCAN);
		case_cxstate(WSCAN_BNOSCAN);
		case_cxstate(WSCAN_BSCAN);
		case_cxstate(WLINKING);
		case_cxstate(WIDLE_BSCAN);
		}
		break;
	case BTC_STR_FDDT_TYPE:
		switch(id) {
		case_fddt_type(STOP);
		case_fddt_type(AUTO);
		case_fddt_type(FIX_TDD);
		case_fddt_type(FIX_FULL_FDD);
		}
		break;
	case BTC_STR_FDDT_STATE:
		switch(id) {
		case_fddt_state(STOP);
		case_fddt_state(RUN);
		case_fddt_state(PENDING);
		case_fddt_state(DEBUG);
		}
		break;
	case BTC_STR_FDDT_NORUN:
		switch(id) {
		case_fddt_norun(SUPPORT);
		case_fddt_norun(FORCE_STOP);
		case_fddt_norun(DEDICATED_ANT);
		case_fddt_norun(ANT_ISO_LOW);
		case_fddt_norun(ANT_ISO_HI);
		case_fddt_norun(WL_2GSTA);
		case_fddt_norun(WL_BUSY);
		case_fddt_norun(WL_NOSCAN);
		case_fddt_norun(WL_NORFK);
		case_fddt_norun(WB_RSSI);
		case_fddt_norun(BT_PROFILE);
		case_fddt_norun(BT_A2DP_BUSY);
		case_fddt_norun(BT_NOINQ);
		case_fddt_norun(COND_NUM);
		case_fddt_norun(NHM);
		case_fddt_norun(RETRY_PERIOD);
		}
		break;
	case BTC_STR_FDDT_TSTATE:
		switch(id) {
		case_fddt_tstate(STOP);
		case_fddt_tstate(RUN);
		case_fddt_tstate(OK);
		case_fddt_tstate(FAIL);
		case_fddt_tstate(NA);
		case_fddt_tstate(DEBUG);
		case_fddt_tstate(MAX);
		}
		break;
	case BTC_STR_FDDT_TPHASE:
		switch(id) {
		case_fddt_tphase(STOP);
		case_fddt_tphase(DEBUG);
		case_fddt_tphase(TDD);
		case_fddt_tphase(FDD_A);
		case_fddt_tphase(FDD_B);
		case_fddt_tphase(TDD_STAY);
		case_fddt_tphase(FDD_STAY);
		case_fddt_tphase(TDD_FAIL);
		case_fddt_tphase(MAX);
		}
		break;
	case BTC_STR_FDDT_CHKMAP:
		switch(id) {
		case_fddt_chkmap(NOEMPTY);
		case_fddt_chkmap(TPRATIO);
		case_fddt_chkmap(TPLOW);
		case_fddt_chkmap(CN);
		case_fddt_chkmap(CELLCHG);
		case_fddt_chkmap(NHM);
		case_fddt_chkmap(MAX);
		}
		break;
	case BTC_STR_DMERROR:
		switch(id) {
		case_dmerr(INIT);
		case_dmerr(PTA_OWNER);
		case_dmerr(WL_RFK_TIMEOUT);
		case_dmerr(BT_RFK_TIMEOUT);
		case_dmerr(WL_FW_HANG);
		case_dmerr(CYCLE_HANG);
		case_dmerr(W1_HANG);
		case_dmerr(B1_HANG);
		case_dmerr(TDMA_NO_SYNC);
		case_dmerr(SLOT_NO_SYNC);
		case_dmerr(WL_SLOT_DRIFT);
		case_dmerr(BT_SLOT_DRIFT);
		case_dmerr(ROLE_NUM_MISMATCH);
		case_dmerr(NULL1_TX_LATE);
		case_dmerr(AFH_CONFLICT);
		case_dmerr(AFH_LE_CONFLICT);
		case_dmerr(BT_SLOT_FLOOD);
		case_dmerr(E2G_HANG);
		case_dmerr(WL_VER_MISMATCH);
		case_dmerr(BT_VER_MISMATCH);
		case_dmerr(RFE_TYPE0);
		case_dmerr(H2C_BUF_OVER);
		case_dmerr(BTTX_HANG);
		case_dmerr(WL_NO_STA_NTFY);
		case_dmerr(H2C_BMAP_MISMATCH);
		case_dmerr(C2H_BMAP_MISMATCH);
		case_dmerr(H2C_STRUCT_INVALID);
		case_dmerr(C2H_STRUCT_INVALID);
		}
		break;
	case BTC_STR_FWERROR:
		switch(id) {
		case_fwerr(NULL_RESULT);
		case_fwerr(NULL_EXCEPTION);
		case_fwerr(B1_EXCEPTION);
		case_fwerr(W1_EXCEPTION);
		case_fwerr(B2_EXCEPTION);
		case_fwerr(B3_EXCEPTION);
		case_fwerr(B4_EXCEPTION);
		case_fwerr(LPS_TIMER_START);
		case_fwerr(EXAUTO_EXCEPTION);
		case_fwerr(BUF_OVERFLOW);
		case_fwerr(BRLY_EXCEPTION);
		case_fwerr(SM);
		case_fwerr(B2_AFTER);
		case_fwerr(LK_END);
		case_fwerr(TMR_FAIL);
		case_fwerr(H2C_SET_EXCEPTION);
		}
		break;
	case BTC_STR_RPTMATCH:
		switch(id) {
		case_rptmatch(CTRL);
		case_rptmatch(TDMA);
		case_rptmatch(SLOT);
		case_rptmatch(CYSTA);
		case_rptmatch(STEP);
		case_rptmatch(NULLSTA);
		case_rptmatch(MREG);
		case_rptmatch(GPIO_DBG);
		case_rptmatch(BT_VER);
		case_rptmatch(BT_SCAN);
		case_rptmatch(BT_AFH);
		case_rptmatch(BT_DEVICE);
		case_rptmatch(TEST);
		}
		break;
	case BTC_STR_H2CERROR:
		switch(id) {
		case_h2cerr(REPORT_EN);
		case_h2cerr(SLOT_TABLE);
		case_h2cerr(MREG_TABLE);
		case_h2cerr(CX_POLICY);
		case_h2cerr(GPIO_DBG);
		case_h2cerr(DRV_INFO);
		case_h2cerr(DRV_EVENT);
		case_h2cerr(BT_WREG_ADDR);
		case_h2cerr(BT_WREG_VAL);
		case_h2cerr(BT_RREG_ADDR);
		case_h2cerr(BT_WL_CH_INFO);
		case_h2cerr(BT_INFO_REPORT);
		case_h2cerr(BT_IGNORE_WLAN_ACT);
		case_h2cerr(BT_TX_PWR);
		case_h2cerr(BT_LNA_CONSTRAIN);
		case_h2cerr(BT_QUERY_DEV_LIST);
		case_h2cerr(BT_QUERY_DEV_INFO);
		case_h2cerr(BT_PSD_REPORT);
		case_h2cerr(H2C_TEST);
		}
		break;
	case BTC_STR_VENDORID:
		switch(id) {
		case_vendorid(NONE);
		case_vendorid(HP);
		case_vendorid(ASUS);
		case_vendorid(ACER);
		case_vendorid(LNV);
		case_vendorid(NEC);
		case_vendorid(AMD);
		case_vendorid(FUJITSU);
		case_vendorid(DELL);
		}
		break;
	case BTC_STR_PATH:
		switch(id) {
		case_path(A);
		case_path(B);
		case_path(C);
		case_path(D);
		case_path(AB);
		case_path(AC);
		case_path(AD);
		case_path(BC);
		case_path(BD);
		case_path(CD);
		case_path(ABC);
		case_path(ABD);
		case_path(ACD);
		case_path(BCD);
		case_path(ABCD);
		}
		break;
	}

	return "Undefine";
}


#endif
