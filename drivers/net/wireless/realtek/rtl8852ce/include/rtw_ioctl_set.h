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
#ifndef __RTW_IOCTL_SET_H_
#define __RTW_IOCTL_SET_H_

u8 rtw_set_802_11_authentication_mode(_adapter *pdapter, NDIS_802_11_AUTHENTICATION_MODE authmode);
u8 rtw_set_802_11_bssid(_adapter *padapter, u8 *bssid);
u8 rtw_set_802_11_add_wep(_adapter *padapter, NDIS_802_11_WEP *wep);
u8 rtw_set_802_11_disassociate(_adapter *padapter);
u8 rtw_set_802_11_infrastructure_mode(_adapter *padapter, NDIS_802_11_NETWORK_INFRASTRUCTURE networktype, u8 flags);
u8 rtw_set_802_11_ssid(_adapter *padapter, NDIS_802_11_SSID *ssid);
u8 rtw_set_802_11_connect(_adapter *padapter, const u8 *bssid, NDIS_802_11_SSID *ssid,
			  u16 ch, enum band_type band);

u8 rtw_validate_bssid(const u8 *bssid);
u8 rtw_validate_ssid(NDIS_802_11_SSID *ssid);

u16 rtw_get_cur_max_rate(_adapter *adapter);
int rtw_set_channel_plan(_adapter *adapter, u8 channel_plan, u8 chplan_6g
	, enum rtw_env_t env, enum rtw_regd_inr inr);
int rtw_set_country(_adapter *adapter, const char *country_code
	, enum rtw_env_t env, enum rtw_regd_inr inr);
#if CONFIG_IEEE80211_BAND_6GHZ
int rtw_set_env(_adapter *adapter, enum rtw_env_t env, enum rtw_regd_inr inr);
#endif
int rtw_set_band(_adapter *adapter, u8 band);

#endif
