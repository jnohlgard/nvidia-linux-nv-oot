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
#define _PHL_DEBUG_C_
#include "phl_headers.h"

#ifdef CONFIG_RTW_DEBUG
enum _phl_dbg_comp_mode {
	PHL_SET_COMP_BIT = 0x01,
	PHL_CLEAR_COMP_BIT = 0x2,
	PHL_SHOW_COMP = 0x3,
	PHL_SET_COMP_VALUE = 0x4,
};


struct phl_dbg_comp_list _phl_dbg_comp_list[] = {
	{"COMP_PHL_DBG", 	_PHL_DBG, 	BIT0},
	{"COMP_PHL_RECV", 	_PHL_RECV,	BIT1},
	{"COMP_PHL_XMIT", 	_PHL_XMIT, 	BIT2},
	{"COMP_PHL_MAC", 	_PHL_MAC,	BIT3},
	{"COMP_PHL_SOUND", 	_PHL_SOUND,	BIT4},
	{"COMP_PHL_WOW",	_PHL_WOW,	BIT5},
	{"COMP_PHL_TRIG",	_PHL_TRIG,	BIT6},
	{"COMP_PHL_PKTOFLD",	_PHL_PKTOFLD,	BIT7},
	{"COMP_PHL_FSM",	_PHL_FSM,	BIT8},
	{"COMP_PHL_PS",  	_PHL_PS,	BIT9},
	{"COMP_PHL_PSTS",  	_PHL_PSTS,	BIT10},
	{"COMP_TWT",  		_PHL_TWT,		BIT11},
	{"COMP_PHL_OFDMA",  _PHL_OFDMA,	BIT12},
	{"COMP_PHL_GROUP",  _PHL_GROUP,	BIT13},
};

#ifdef CONFIG_RTW_DISABLE_PHL_LOG
u32 phl_log_components = 0;
u8 phl_log_level = _PHL_NONE_;
#else
u32 phl_log_components = COMP_PHL_XMIT |
			 COMP_PHL_WOW |
			 COMP_PHL_PKTOFLD |
			/* COMP_PHL_CMDDISP |*/
			 COMP_PHL_RECV |
			 COMP_PHL_MAC |
			 #ifdef CONFIG_POWER_SAVE
			 COMP_PHL_PS |
			 #endif
			 COMP_PHL_DBG | 0;
u8 phl_log_level = _PHL_INFO_;
#endif /*CONFIG_RTW_DISABLE_PHL_LOG*/

struct dbg_mem_ctx debug_memory_ctx;

void rtw_phl_log_level_cfg(u8 dbg_level)
{
	if (dbg_level >= _PHL_MAX_)
		return;

	phl_log_level = dbg_level;
}

void debug_dump_mac_address(u8 *mac_addr)
{
	if (mac_addr == NULL) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[dump mac addr] mac_addr is NULL\n");
		return;
	}

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[MAC ADDRESS]\n");
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
		 "%02X-%02X-%02X-%02X-%02X-%02X\n",
		 mac_addr[0], mac_addr[1], mac_addr[2],
		 mac_addr[3], mac_addr[4], mac_addr[5]);
}

void debug_dump_buf(u8 *buf, u16 buf_len, const char *prefix)
{
	int i = 0, j;

	if (buf == NULL) {
		PHL_WARN("%s: NULL buffer!\n", __FUNCTION__);
		return;
	}

	PHL_PRINT("[%s (%uB)]@%px:\n", prefix, buf_len, buf);

	while (i < buf_len) {
		PHL_PRINT(" %04X -", i);
		for (j = 0; (j < 4) && (i < buf_len); j++, i += 4) {
			PHL_DATA(COMP_PHL_DBG, _PHL_ALWAYS_,
				 "  %02X %02X %02X %02X",
				 buf[i], buf[i+1], buf[i+2], buf[i+3]);
		}
		PHL_DATA(COMP_PHL_DBG, _PHL_ALWAYS_, "\n");
	}
}

void debug_dump_data(u8 *buf, u32 buf_len, const char *prefix)
{
	u32 i;

	if (buf == NULL) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "[debug dump] buf is NULL\n");
		return;
	}

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "[debug dump] %s\n", prefix);
	for (i = 0; i < buf_len; i++) {
		if (!(i % 8))
			PHL_DATA(COMP_PHL_DBG, _PHL_DEBUG_, "\n");
		PHL_DATA(COMP_PHL_DBG, _PHL_DEBUG_, "%02X ", buf[i]);
		}
	PHL_DATA(COMP_PHL_DBG, _PHL_DEBUG_, "\n");
}

void rt_alloc_dbg_buf(void *phl, u8 *buf_ptr, u32 buf_size,
		const u8 *file_name, u32 line_num, const u8 *func_name)
{
	_os_list *list = &debug_memory_ctx.alloc_buf_list;
	_os_lock *lock = &debug_memory_ctx.alloc_buf_list_lock;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct dbg_alloc_buf *dbg_buf = NULL;
	u32 name_size = 0;

	FUNCIN();

	dbg_buf = _os_mem_alloc(phl_to_drvpriv(phl_info), sizeof(*dbg_buf));

	dbg_buf->buf_ptr = buf_ptr;
		dbg_buf->buf_size = buf_size;

	name_size = (_os_strlen((u8 *)file_name) > DEBUG_PHL_MAX_NAME_LEN) ?
		DEBUG_PHL_MAX_NAME_LEN : _os_strlen((u8 *)file_name);
	_os_mem_cpy(phl_to_drvpriv(phl_info), dbg_buf->file_name, (u8 *)file_name, name_size);

	dbg_buf->line_num = line_num;

	name_size = (_os_strlen((u8 *)func_name) > DEBUG_PHL_MAX_NAME_LEN) ?
		DEBUG_PHL_MAX_NAME_LEN : _os_strlen((u8 *)func_name);
	_os_mem_cpy(phl_to_drvpriv(phl_info), dbg_buf->func_name, (u8 *)func_name, name_size);

	_os_spinlock(phl_to_drvpriv(phl_info), lock, _bh, NULL);

	list_add_tail(&(dbg_buf->list), list);
	debug_memory_ctx.alloc_buf_cnt++;

	_os_spinunlock(phl_to_drvpriv(phl_info), lock, _bh, NULL);

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "Allocate Memory: %p, size: %d, File: %s, Line: %d, Function: %s\n",
		 dbg_buf->buf_ptr, dbg_buf->buf_size, dbg_buf->file_name,
		 dbg_buf->line_num, dbg_buf->func_name);

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "Current allocated buffer count: %d",
		 debug_memory_ctx.alloc_buf_cnt);

	FUNCOUT();
}


void rt_free_dbg_buf(void *phl, u8 *buf_ptr, u32 buf_size,
		const u8 *file_name, u32 line_num, const u8 *func_name)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	_os_list *list = &debug_memory_ctx.alloc_buf_list;
	_os_lock *lock = &debug_memory_ctx.alloc_buf_list_lock;
	struct dbg_alloc_buf *dbg_buf = NULL , *t;
	u8 found = false;

	FUNCIN();

	_os_spinlock(phl_to_drvpriv(phl_info), lock, _bh, NULL);

	phl_list_for_loop_safe(dbg_buf, t, struct dbg_alloc_buf, list, list) {
		if (dbg_buf->buf_ptr == buf_ptr) {
			list_del(&dbg_buf->list);
			found = true;
			break;
		}
	}
	_os_spinunlock(phl_to_drvpriv(phl_info), lock, _bh, NULL);

	if (true == found) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "Free Memory: %p, size: %d, File: %s, Line: %d, Function: %s\n",
			 dbg_buf->buf_ptr, dbg_buf->buf_size, file_name,
			 line_num, func_name);
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "Current allocated buffer count: %d",
			 debug_memory_ctx.alloc_buf_cnt);
		_os_mem_free(phl_to_drvpriv(phl_info), dbg_buf, sizeof(*dbg_buf));
		debug_memory_ctx.alloc_buf_cnt--;
	} else {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			"WARNING, can not find allocated buffer in list\n");
	}

	FUNCOUT();
}


void rt_mem_dbg_init(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	_os_list *list = &debug_memory_ctx.alloc_buf_list;
	_os_lock *lock = &debug_memory_ctx.alloc_buf_list_lock;

	FUNCIN();
	debug_memory_ctx.alloc_buf_cnt = 0;
	INIT_LIST_HEAD(list);
	_os_spinlock_init(phl_to_drvpriv(phl_info), lock);
	FUNCOUT();
}

void rt_mem_dbg_deinit(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	_os_list *list = &debug_memory_ctx.alloc_buf_list;
	_os_lock *lock = &debug_memory_ctx.alloc_buf_list_lock;
	struct dbg_alloc_buf *dbg_buf = NULL;

	FUNCIN();
	while (true != list_empty(list) ||
		0 != debug_memory_ctx.alloc_buf_cnt) {

		_os_spinlock(phl_to_drvpriv(phl_info), lock, _bh, NULL);
		dbg_buf = list_first_entry(list, struct dbg_alloc_buf, list);
		list_del(&dbg_buf->list);
		_os_spinunlock(phl_to_drvpriv(phl_info), lock, _bh, NULL);

		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "FOUND non-freed Memory: %p, size: %d, File: %s, Line: %d, Function: %s\n",
			dbg_buf->buf_ptr, dbg_buf->buf_size,
			dbg_buf->file_name, dbg_buf->line_num,
			dbg_buf->func_name);
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "Current allocated buffer count: %d",
			debug_memory_ctx.alloc_buf_cnt);
		_os_mem_free(phl_to_drvpriv(phl_info), dbg_buf, sizeof(*dbg_buf));
		debug_memory_ctx.alloc_buf_cnt--;
	}
	_os_spinlock_init(phl_to_drvpriv(phl_info), lock);

	FUNCOUT();
}


void phl_dbg_show_log_comp(u32 para_dbg)
{
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[DBG] phl_log_components = 0x%x \n", phl_log_components);
}

void phl_dbg_set_log_comp(u32 para_dbg)
{
	phl_log_components |= BIT(para_dbg);
}

void phl_dbg_clear_log_comp(u32 para_dbg)
{
	phl_log_components &= (~BIT(para_dbg));
}

void phl_dbg_set_log_comp_value(u32 para_dbg)
{
	phl_log_components = para_dbg;
}

void phl_dbg_dump_log_comp(void)
{
	struct phl_dbg_comp_list *cmd = _phl_dbg_comp_list;
	u8 array_size = ARRAY_SIZE(_phl_dbg_comp_list);
	u8 i;

	for (i = 0; i < array_size; i++, cmd++) {
		PHL_PRINT("(%s)  [BIT%d] %s\n", (cmd->level_comp & phl_log_components)? "O" : "X", cmd->level, cmd->name);
	}
	return;
}

u32 rtw_phl_dbg_ctrl_comp(u8 ctrl, u32 para_dbg)
{

	switch(ctrl){
	case PHL_SET_COMP_BIT:
		if (para_dbg > 31)
			PHL_PRINT( "[DBG] bit num is illegal\n");
		else
			phl_dbg_set_log_comp(para_dbg);
		break;
	case PHL_CLEAR_COMP_BIT:
		if (para_dbg > 31)
			PHL_PRINT("[DBG] bit num is illegal\n");
		else
			phl_dbg_clear_log_comp(para_dbg);
		break;
	case PHL_SHOW_COMP:
		phl_dbg_dump_log_comp();
		break;
	case PHL_SET_COMP_VALUE:
		phl_dbg_set_log_comp_value(para_dbg);
		break;
	default:
		break;
	}

	phl_dbg_show_log_comp(para_dbg);

	return phl_log_components;
}
#endif	/* #ifdef CONFIG_PHL_DEBUG */

#ifdef CONFIG_RTW_MIRROR_DUMP
#define MD_HDR_LEN (ETH_HLEN + 4)
static void _phl_mirror_dump_out(void *phl, u8 cat, u8 *buf, u32 sz)
{
	u8 md_hdr[MD_HDR_LEN] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
		0x88, 0x9a, 0x00, 0x00, 0x00, 0x00
	};
	u8 *cat_loc = md_hdr + ETH_HLEN;
	u16 *len_loc = (u16 *)(md_hdr + ETH_HLEN + 2);

	*cat_loc = cat;
	*len_loc = ntohs(sz);

	_os_mirror_dump(md_hdr, MD_HDR_LEN, buf, sz);
}

void phl_mirror_dump_h2c(void *phl, struct rtw_h2c_pkt *pkt)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	u8 *buf = pkt->vir_data + 24;
	u32 len = pkt->data_len - 24;

	if (phl_info->phl_com->mirror_cfg & BIT(PHL_MIR_CFG_H2C))
		_phl_mirror_dump_out(phl, _PHL_MIR_CAT_H2C, buf, len);
}

void phl_mirror_dump_c2h(void *phl, struct rtw_pkt_buf_list *pkt)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	u8 *buf = pkt->vir_addr;
	u32 len = pkt->length;

	if (phl_info->phl_com->mirror_cfg & BIT(PHL_MIR_CFG_C2H))
		_phl_mirror_dump_out(phl, _PHL_MIR_CAT_C2H, buf, len);
}

void phl_mirror_dump_wd(void *phl, u8 txch, u8 *wd, u32 wd_len)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	u8 *buf = wd;
	u32 len = wd_len;

	if (phl_info->phl_com->mirror_cfg & BIT(PHL_MIR_CFG_TXD)) {
		if (phl_info->phl_com->mirror_txch == txch)
			_phl_mirror_dump_out(phl, _PHL_MIR_CAT_TXD, buf, len);
	}
}

void phl_mirror_dump_rxd(void *phl, u8 *rxd, u32 rxd_len)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	u8 *buf = rxd;
	u32 len = rxd_len;

	if (phl_info->phl_com->mirror_cfg & BIT(PHL_MIR_CFG_RXD)) {
		_phl_mirror_dump_out(phl, _PHL_MIR_CAT_RXD, buf, len);
	}
}

void rtw_phl_mirror_dump_buf(void *phl, u8 cat, u8 *buf, u32 sz)
{
	if (cat >= _PHL_MIR_CAT_MAX) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "mirror cat %d unrecognized\n", cat);
		return;
	}

	_phl_mirror_dump_out(phl, cat, buf, sz);
}

void rtw_phl_set_mirror_dump_config(void *phl, u32 config)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	phl_info->phl_com->mirror_cfg = config;
}

void rtw_phl_set_mirror_dump_txch(void *phl, u8 txch)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	phl_info->phl_com->mirror_txch = txch;
}
#endif /* CONFIG_RTW_MIRROR_DUMP */

void rtw_phl_get_halmac_ver(char *buf, u16 buf_len)
{
	rtw_hal_get_mac_version(buf, buf_len);
}

void rtw_phl_update_io_dump_allow(void *phl, bool io_dump_allow)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	if (io_dump_allow) {
		SET_IO_DUMP_ALLOWED(phl_info->phl_com);
	} else {
		CLR_IO_DUMP_ALLOWED(phl_info->phl_com);
	}

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[DBG] update io_dump_allow = %d, dump_cfg = 0x%x \n",
				(IS_IO_DUMP_ALLOWED(phl_info->phl_com) > 0),
				phl_info->phl_com->dbg_cfg.dump_cfg);
}

void rtw_phl_update_fw_log_dump_allow(void *phl, bool fw_log_dump_allow)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_dbg_cfg_t *dbg_cfg = &phl_info->phl_com->dbg_cfg;

	if (fw_log_dump_allow) {
		SET_FW_LOG_DUMP_ALLOWED(phl_info->phl_com);
		rtw_hal_set_fw_log_lvl(phl_info->hal, dbg_cfg->fw_log_info.level);
	} else {
		CLR_FW_LOG_DUMP_ALLOWED(phl_info->phl_com);
		rtw_hal_set_fw_log_lvl(phl_info->hal, FL_LV_OFF);
	}

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[DBG] update fw_log_dump_allow = %d, dump_cfg = 0x%x\n",
				(IS_FW_LOG_DUMP_ALLOWED(phl_info->phl_com) > 0),
				phl_info->phl_com->dbg_cfg.dump_cfg);
}

#ifdef DBG_MONITOR_TIME
void phl_fun_monitor_start(u32 *start_t, bool show_caller, const char *caller)
{
	*start_t = _os_get_cur_time_ms();
	if (show_caller)
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, ">> %s:\n", caller);
}

void phl_fun_monitor_end(struct rtw_phl_com_t *phl_com,
				   u32 *start_t, enum phl_time_flags flag, const char *caller)
{
	u32 pass_t = phl_get_passing_time_ms(*start_t);

	if (flag == TIME_HAL_SET_CHAN || flag == TIME_HAL_RFK) {
		u32 tmp_t = phl_com->func_latency[flag];

		if (pass_t > tmp_t)
			phl_com->func_latency[flag] = pass_t;
	} else {
		phl_com->func_latency[flag] = pass_t;
	}

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, " << %s: Process time(ms): %d\n",
		caller, pass_t);
}
void phl_dump_func_latency(struct rtw_phl_com_t *phl_com)
{
	PHL_INFO("========== PHL FUNC LATENCY ==========\n");
	PHL_INFO("\t rtw_phl_init : %d ms\n", phl_com->func_latency[TIME_PHL_INIT]);

	PHL_INFO("\t rtw_phl_preload : %d ms\n", phl_com->func_latency[TIME_PHL_PRELOAD]);
	PHL_INFO("\t\t hal_fast_start : %d ms\n", phl_com->func_latency[TIME_HAL_FAST_START]);
	PHL_INFO("\t\t hal_get_efuse : %d ms\n", phl_com->func_latency[TIME_HAL_GET_EFUSE]);
	PHL_INFO("\t\t hal_fast_stop : %d ms\n", phl_com->func_latency[TIME_HAL_FAST_STOP]);

	PHL_INFO("\t rtw_phl_star : %d ms\n", phl_com->func_latency[TIME_PHL_START]);
	PHL_INFO("\t\t rtw_hal_star : %d ms\n", phl_com->func_latency[TIME_HAL_START]);
	PHL_INFO("\t\t\t rtw_hal_mac_hal_init : %d ms\n", phl_com->func_latency[TIME_HAL_MAC_HAL_INIT]);
	PHL_INFO("\t\t\t rtw_hal_efuse_process : %d ms\n", phl_com->func_latency[TIME_HAL_EFUSE_PROC]);
	PHL_INFO("\t\t\t rtw_hal_init_bb_early_init : %d ms\n", phl_com->func_latency[TIME_HAL_INIT_BB_REG1]);
	PHL_INFO("\t\t\t rtw_hal_init_bb_reg : %d ms\n", phl_com->func_latency[TIME_HAL_INIT_BB_REG2]);
	PHL_INFO("\t\t\t rtw_hal_init_rf_reg : %d ms\n", phl_com->func_latency[TIME_HAL_INIT_RF_REG]);
	PHL_INFO("\t\t\t rtw_hal_btc_init_coex_cfg_ntfy : %d ms\n", phl_com->func_latency[TIME_HAL_INIT_BTC]);
	PHL_INFO("\t\t\t rtw_hal_bb_dm_init : %d ms\n", phl_com->func_latency[TIME_HAL_BB_DM_INIT]);
	PHL_INFO("\t\t\t rtw_hal_rf_dm_init : %d ms\n", phl_com->func_latency[TIME_HAL_RF_DM_INIT]);

	PHL_INFO("\t\t rtw_hal_set_ch_bw : %d ms\n", phl_com->func_latency[TIME_HAL_SET_CHAN]);
	PHL_INFO("\t\t rtw_hal_rf_chl_rfk_trigger : %d ms\n", phl_com->func_latency[TIME_HAL_RFK]);

	PHL_INFO("\t rtw_phl_wifi_role_alloc : %d ms\n", phl_com->func_latency[TIME_PHL_ROLE_ALLOC]);
	PHL_INFO("\t rtw_phl_wifi_role_free : %d ms\n", phl_com->func_latency[TIME_PHL_ROLE_FREE]);

	PHL_INFO("\t rtw_phl_suspend : %d ms\n", phl_com->func_latency[TIME_PHL_SUSPEND]);
	PHL_INFO("\t\t phl_wow_start : %d ms\n", phl_com->func_latency[TIME_PHL_WOW_START]);
	PHL_INFO("\t\t phl_cmd_role_suspend : %d ms\n", phl_com->func_latency[TIME_PHL_ROLE_SUSPEND]);

	PHL_INFO("\t rtw_phl_resume : %d ms\n", phl_com->func_latency[TIME_PHL_RESUME]);
	PHL_INFO("\t\t phl_wow_stop : %d ms\n", phl_com->func_latency[TIME_PHL_WOW_STOP]);
	PHL_INFO("\t\t phl_cmd_role_recover : %d ms\n", phl_com->func_latency[TIME_PHL_ROLE_RECOVER]);


	PHL_INFO("\t rtw_phl_stop : %d ms\n", phl_com->func_latency[TIME_PHL_STOP]);
	PHL_INFO("\t rtw_phl_deinit : %d ms\n", phl_com->func_latency[TIME_PHL_DEINIT]);

	PHL_INFO("======================================\n");
}
#endif /* DBG_MONITOR_TIME */

