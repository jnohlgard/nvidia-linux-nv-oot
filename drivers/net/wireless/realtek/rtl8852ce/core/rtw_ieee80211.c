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
#define _IEEE80211_C

#include <drv_types.h>

u8 RTW_WPA_OUI_TYPE[] = { 0x00, 0x50, 0xf2, 1 };
u16 RTW_WPA_VERSION = 1;
u8 WPA_AUTH_KEY_MGMT_NONE[] = { 0x00, 0x50, 0xf2, 0 };
u8 WPA_AUTH_KEY_MGMT_UNSPEC_802_1X[] = { 0x00, 0x50, 0xf2, 1 };
u8 WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X[] = { 0x00, 0x50, 0xf2, 2 };
u8 WPA_CIPHER_SUITE_NONE[] = { 0x00, 0x50, 0xf2, 0 };
u8 WPA_CIPHER_SUITE_WEP40[] = { 0x00, 0x50, 0xf2, 1 };
u8 WPA_CIPHER_SUITE_TKIP[] = { 0x00, 0x50, 0xf2, 2 };
u8 WPA_CIPHER_SUITE_WRAP[] = { 0x00, 0x50, 0xf2, 3 };
u8 WPA_CIPHER_SUITE_CCMP[] = { 0x00, 0x50, 0xf2, 4 };
u8 WPA_CIPHER_SUITE_WEP104[] = { 0x00, 0x50, 0xf2, 5 };
u8 WPA_CIPHER_SUITE_AES_CMAC[] = { 0x00, 0x50, 0xf2, 6 };
u8 WPA_CIPHER_SUITE_GCMP[] = { 0x00, 0x50, 0xf2, 8 };
u8 WPA_CIPHER_SUITE_GCMP_256[] = { 0x00, 0x50, 0xf2, 9 };
u8 WPA_CIPHER_SUITE_CCMP_256[] = { 0x00, 0x50, 0xf2, 10 };

u16 RSN_VERSION_BSD = 1;
u8 RSN_CIPHER_SUITE_NONE[] = { 0x00, 0x0f, 0xac, 0 };
u8 RSN_CIPHER_SUITE_WEP40[] = { 0x00, 0x0f, 0xac, 1 };
u8 RSN_CIPHER_SUITE_TKIP[] = { 0x00, 0x0f, 0xac, 2 };
/*u8 RSN_CIPHER_SUITE_WRAP[] = { 0x00, 0x0f, 0xac, 3 };*/
u8 RSN_CIPHER_SUITE_CCMP[] = { 0x00, 0x0f, 0xac, 4 };
u8 RSN_CIPHER_SUITE_AES_128_CMAC[] = { 0x00, 0x0f, 0xac, 6 };
u8 RSN_CIPHER_SUITE_GCMP[] = { 0x00, 0x0f, 0xac, 8 };
u8 RSN_CIPHER_SUITE_GCMP_256[] = { 0x00, 0x0f, 0xac, 9 };
u8 RSN_CIPHER_SUITE_CCMP_256[] = { 0x00, 0x0f, 0xac, 10 };
u8 RSN_CIPHER_SUITE_BIP_GMAC_128[] = { 0x00, 0x0f, 0xac, 11 };
u8 RSN_CIPHER_SUITE_BIP_GMAC_256[] = { 0x00, 0x0f, 0xac, 12 };
u8 RSN_CIPHER_SUITE_BIP_CMAC_256[] = { 0x00, 0x0f, 0xac, 13 };
u8 RSN_CIPHER_SUITE_WEP104[] = { 0x00, 0x0f, 0xac, 5 };

u8 WLAN_AKM_8021X[] = {0x00, 0x0f, 0xac, 1};
u8 WLAN_AKM_PSK[] = {0x00, 0x0f, 0xac, 2};
u8 WLAN_AKM_FT_8021X[] = {0x00, 0x0f, 0xac, 3};
u8 WLAN_AKM_FT_PSK[] = {0x00, 0x0f, 0xac, 4};
u8 WLAN_AKM_8021X_SHA256[] = {0x00, 0x0f, 0xac, 5};
u8 WLAN_AKM_PSK_SHA256[] = {0x00, 0x0f, 0xac, 6};
u8 WLAN_AKM_TDLS[] = {0x00, 0x0f, 0xac, 7};
u8 WLAN_AKM_SAE[] = {0x00, 0x0f, 0xac, 8};
u8 WLAN_AKM_FT_OVER_SAE[] = {0x00, 0x0f, 0xac, 9};
u8 WLAN_AKM_8021X_SUITE_B[] = {0x00, 0x0f, 0xac, 11};
u8 WLAN_AKM_8021X_SUITE_B_192[] = {0x00, 0x0f, 0xac, 12};
u8 WLAN_AKM_FILS_SHA256[] = {0x00, 0x0f, 0xac, 14};
u8 WLAN_AKM_FILS_SHA384[] = {0x00, 0x0f, 0xac, 15};
u8 WLAN_AKM_FT_FILS_SHA256[] = {0x00, 0x0f, 0xac, 16};
u8 WLAN_AKM_FT_FILS_SHA384[] = {0x00, 0x0f, 0xac, 17};

/* -----------------------------------------------------------
 * for adhoc-master to generate ie and provide supported-rate to fw
 * ----------------------------------------------------------- */

u8	WIFI_CCKRATES[] = {
	(IEEE80211_CCK_RATE_1MB | IEEE80211_BASIC_RATE_MASK),
	(IEEE80211_CCK_RATE_2MB | IEEE80211_BASIC_RATE_MASK),
	(IEEE80211_CCK_RATE_5MB | IEEE80211_BASIC_RATE_MASK),
	(IEEE80211_CCK_RATE_11MB | IEEE80211_BASIC_RATE_MASK)
};

u8	WIFI_OFDMRATES[] = {
	(IEEE80211_OFDM_RATE_6MB),
	(IEEE80211_OFDM_RATE_9MB),
	(IEEE80211_OFDM_RATE_12MB),
	(IEEE80211_OFDM_RATE_18MB),
	(IEEE80211_OFDM_RATE_24MB),
	IEEE80211_OFDM_RATE_36MB,
	IEEE80211_OFDM_RATE_48MB,
	IEEE80211_OFDM_RATE_54MB
};

u8 mgn_rates_cck[4] = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M};
u8 mgn_rates_ofdm[8] = {MGN_6M, MGN_9M, MGN_12M, MGN_18M, MGN_24M, MGN_36M, MGN_48M, MGN_54M};
u8 mgn_rates_mcs0_7[8] = {MGN_MCS0, MGN_MCS1, MGN_MCS2, MGN_MCS3, MGN_MCS4, MGN_MCS5, MGN_MCS6, MGN_MCS7};
u8 mgn_rates_mcs8_15[8] = {MGN_MCS8, MGN_MCS9, MGN_MCS10, MGN_MCS11, MGN_MCS12, MGN_MCS13, MGN_MCS14, MGN_MCS15};
u8 mgn_rates_mcs16_23[8] = {MGN_MCS16, MGN_MCS17, MGN_MCS18, MGN_MCS19, MGN_MCS20, MGN_MCS21, MGN_MCS22, MGN_MCS23};
u8 mgn_rates_mcs24_31[8] = {MGN_MCS24, MGN_MCS25, MGN_MCS26, MGN_MCS27, MGN_MCS28, MGN_MCS29, MGN_MCS30, MGN_MCS31};
u8 mgn_rates_vht1ss[10] = {MGN_VHT1SS_MCS0, MGN_VHT1SS_MCS1, MGN_VHT1SS_MCS2, MGN_VHT1SS_MCS3, MGN_VHT1SS_MCS4
	, MGN_VHT1SS_MCS5, MGN_VHT1SS_MCS6, MGN_VHT1SS_MCS7, MGN_VHT1SS_MCS8, MGN_VHT1SS_MCS9
			  };
u8 mgn_rates_vht2ss[10] = {MGN_VHT2SS_MCS0, MGN_VHT2SS_MCS1, MGN_VHT2SS_MCS2, MGN_VHT2SS_MCS3, MGN_VHT2SS_MCS4
	, MGN_VHT2SS_MCS5, MGN_VHT2SS_MCS6, MGN_VHT2SS_MCS7, MGN_VHT2SS_MCS8, MGN_VHT2SS_MCS9
			  };
u8 mgn_rates_vht3ss[10] = {MGN_VHT3SS_MCS0, MGN_VHT3SS_MCS1, MGN_VHT3SS_MCS2, MGN_VHT3SS_MCS3, MGN_VHT3SS_MCS4
	, MGN_VHT3SS_MCS5, MGN_VHT3SS_MCS6, MGN_VHT3SS_MCS7, MGN_VHT3SS_MCS8, MGN_VHT3SS_MCS9
			  };
u8 mgn_rates_vht4ss[10] = {MGN_VHT4SS_MCS0, MGN_VHT4SS_MCS1, MGN_VHT4SS_MCS2, MGN_VHT4SS_MCS3, MGN_VHT4SS_MCS4
	, MGN_VHT4SS_MCS5, MGN_VHT4SS_MCS6, MGN_VHT4SS_MCS7, MGN_VHT4SS_MCS8, MGN_VHT4SS_MCS9
			  };
u8 mgn_rates_he1ss[12] = {MGN_HE1SS_MCS0, MGN_HE1SS_MCS1, MGN_HE1SS_MCS2, MGN_HE1SS_MCS3, MGN_HE1SS_MCS4
	, MGN_HE1SS_MCS5, MGN_HE1SS_MCS6, MGN_HE1SS_MCS7, MGN_HE1SS_MCS8, MGN_HE1SS_MCS9, MGN_HE1SS_MCS10, MGN_HE1SS_MCS11
			  };
u8 mgn_rates_he2ss[12] = {MGN_HE2SS_MCS0, MGN_HE2SS_MCS1, MGN_HE2SS_MCS2, MGN_HE2SS_MCS3, MGN_HE2SS_MCS4
	, MGN_HE2SS_MCS5, MGN_HE2SS_MCS6, MGN_HE2SS_MCS7, MGN_HE2SS_MCS8, MGN_HE2SS_MCS9, MGN_HE2SS_MCS10, MGN_HE2SS_MCS11
			  };
u8 mgn_rates_he3ss[12] = {MGN_HE3SS_MCS0, MGN_HE3SS_MCS1, MGN_HE3SS_MCS2, MGN_HE3SS_MCS3, MGN_HE3SS_MCS4
	, MGN_HE3SS_MCS5, MGN_HE3SS_MCS6, MGN_HE3SS_MCS7, MGN_HE3SS_MCS8, MGN_HE3SS_MCS9, MGN_HE3SS_MCS10, MGN_HE3SS_MCS11
			  };
u8 mgn_rates_he4ss[12] = {MGN_HE4SS_MCS0, MGN_HE4SS_MCS1, MGN_HE4SS_MCS2, MGN_HE4SS_MCS3, MGN_HE4SS_MCS4
	, MGN_HE4SS_MCS5, MGN_HE4SS_MCS6, MGN_HE4SS_MCS7, MGN_HE4SS_MCS8, MGN_HE4SS_MCS9, MGN_HE4SS_MCS10, MGN_HE4SS_MCS11
			  };
u8 mgn_rates_dcm1ss[4] = {MGN_HE1SS_MCS0, MGN_HE1SS_MCS1, MGN_HE1SS_MCS3, MGN_HE1SS_MCS4};
u8 mgn_rates_dcm2ss[4] = {MGN_HE2SS_MCS0, MGN_HE2SS_MCS1, MGN_HE2SS_MCS3, MGN_HE2SS_MCS4};
u8 mgn_rates_dcm3ss[4] = {MGN_HE3SS_MCS0, MGN_HE3SS_MCS1, MGN_HE3SS_MCS3, MGN_HE3SS_MCS4};
u8 mgn_rates_dcm4ss[4] = {MGN_HE4SS_MCS0, MGN_HE4SS_MCS1, MGN_HE4SS_MCS3, MGN_HE4SS_MCS4};
u8 mgn_rates_offset[5] = {MGN_1M, MGN_6M, MGN_MCS0, MGN_VHT1SS_MCS0, MGN_HE1SS_MCS0};

RATE_SECTION mgn_rate_to_rs(enum MGN_RATE rate)
{
	RATE_SECTION rs = RATE_SECTION_NUM;

	if (IS_CCK_RATE(rate))
		rs = CCK;
	else if (IS_OFDM_RATE(rate))
		rs = OFDM;
	else if (IS_HT1SS_RATE(rate))
		rs = HT_1SS;
	else if (IS_HT2SS_RATE(rate))
		rs = HT_2SS;
	else if (IS_HT3SS_RATE(rate))
		rs = HT_3SS;
	else if (IS_HT4SS_RATE(rate))
		rs = HT_4SS;
	else if (IS_VHT1SS_RATE(rate))
		rs = VHT_1SS;
	else if (IS_VHT2SS_RATE(rate))
		rs = VHT_2SS;
	else if (IS_VHT3SS_RATE(rate))
		rs = VHT_3SS;
	else if (IS_VHT4SS_RATE(rate))
		rs = VHT_4SS;

	return rs;
}

static const char *const _rate_section_str[] = {
	"CCK",
	"OFDM",
	"HT_1SS",
	"HT_2SS",
	"HT_3SS",
	"HT_4SS",
	"VHT_1SS",
	"VHT_2SS",
	"VHT_3SS",
	"VHT_4SS",
	"HE_1SS",
	"HE_2SS",
	"HE_3SS",
	"HE_4SS",
	"DCM_1SS",
	"DCM_2SS",
	"DCM_3SS",
	"DCM_4SS",
	"RATE_SECTION_UNKNOWN",
};

const char *rate_section_str(u8 section)
{
	section = (section >= RATE_SECTION_NUM) ? RATE_SECTION_NUM : section;
	return _rate_section_str[section];
}

struct rate_section_ent rates_by_sections[RATE_SECTION_NUM] = {
	{RF_1TX, 4, mgn_rates_cck},
	{RF_1TX, 8, mgn_rates_ofdm},
	{RF_1TX, 8, mgn_rates_mcs0_7},
	{RF_2TX, 8, mgn_rates_mcs8_15},
	{RF_3TX, 8, mgn_rates_mcs16_23},
	{RF_4TX, 8, mgn_rates_mcs24_31},
	{RF_1TX, 10, mgn_rates_vht1ss},
	{RF_2TX, 10, mgn_rates_vht2ss},
	{RF_3TX, 10, mgn_rates_vht3ss},
	{RF_4TX, 10, mgn_rates_vht4ss},
	{RF_1TX, 12, mgn_rates_he1ss},
	{RF_2TX, 12, mgn_rates_he2ss},
	{RF_3TX, 12, mgn_rates_he3ss},
	{RF_4TX, 12, mgn_rates_he4ss},
	{RF_1TX, 4, mgn_rates_dcm1ss},
	{RF_2TX, 4, mgn_rates_dcm2ss},
	{RF_3TX, 4, mgn_rates_dcm3ss},
	{RF_4TX, 4, mgn_rates_dcm4ss},
};

int rtw_get_bit_value_from_ieee_value(u8 val)
{
	unsigned char dot11_rate_table[] = {2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 0}; /* last element must be zero!! */

	int i = 0;
	while (dot11_rate_table[i] != 0) {
		if (dot11_rate_table[i] == val)
			return BIT(i);
		i++;
	}
	return 0;
}
uint rtw_get_cckrate_size(u8 *rate, u32 rate_length)
{
	int i = 0;
	while(i < rate_length){
		RTW_DBG("%s, rate[%d]=%u\n", __FUNCTION__, i, rate[i]);
		if (((rate[i] & 0x7f) == 2) || ((rate[i] & 0x7f) == 4) ||
			((rate[i] & 0x7f) == 11)  || ((rate[i] & 0x7f) == 22))
			i++;
		else
			break;
	}
	return i;
}

uint	rtw_is_cckrates_included(u8 *rate)
{
	u32	i = 0;

	while (rate[i] != 0) {
		if ((((rate[i]) & 0x7f) == 2)	|| (((rate[i]) & 0x7f) == 4) ||
		    (((rate[i]) & 0x7f) == 11)  || (((rate[i]) & 0x7f) == 22))
			return _TRUE;
		i++;
	}

	return _FALSE;
}

uint	rtw_is_cckratesonly_included(u8 *rate)
{
	u32 i = 0;


	while (rate[i] != 0) {
		if ((((rate[i]) & 0x7f) != 2) && (((rate[i]) & 0x7f) != 4) &&
		    (((rate[i]) & 0x7f) != 11)  && (((rate[i]) & 0x7f) != 22))
			return _FALSE;

		i++;
	}

	return _TRUE;

}

int rtw_check_network_type(unsigned char *rate, int ratelen, int channel)
{
	if (channel > 14) {
		if ((rtw_is_cckrates_included(rate)) == _TRUE)
			return WLAN_MD_INVALID;
		else
			return WLAN_MD_11A;
	} else { /* could be pure B, pure G, or B/G */
		if ((rtw_is_cckratesonly_included(rate)) == _TRUE)
			return WLAN_MD_11B;
		else if ((rtw_is_cckrates_included(rate)) == _TRUE)
			return	WLAN_MD_11BG;
		else
			return WLAN_MD_11G;
	}

}

u8 *rtw_set_fixed_ie(unsigned char *pbuf, unsigned int len, unsigned char *source,
		     unsigned int *frlen)
{
	_rtw_memcpy((void *)pbuf, (void *)source, len);
	*frlen = *frlen + len;
	return pbuf + len;
}

/* rtw_set_ie will update frame length */
u8 *rtw_set_ie
(
	u8 *pbuf,
	sint index,
	uint len, /* IE content length, not entire IE length */
	const u8 *source,
	uint *frlen /* frame length */
)
{
	*pbuf = (u8)index;

	*(pbuf + 1) = (u8)len;

	if (len > 0)
		_rtw_memcpy((void *)(pbuf + 2), (void *)source, len);

	if (frlen)
		*frlen = *frlen + (len + 2);

	return pbuf + len + 2;
}

inline u8 *rtw_set_ie_ch_switch(u8 *buf, u32 *buf_len, u8 ch_switch_mode,
				u8 new_ch, u8 ch_switch_cnt)
{
	u8 ie_data[3];

	ie_data[0] = ch_switch_mode;
	ie_data[1] = new_ch;
	ie_data[2] = ch_switch_cnt;
	return rtw_set_ie(buf, WLAN_EID_CHANNEL_SWITCH,  3, ie_data, buf_len);
}

inline u8 secondary_ch_offset_to_hal_ch_offset(u8 ch_offset)
{
	if (ch_offset == IEEE80211_SCN)
		return CHAN_OFFSET_NO_EXT;
	else if (ch_offset == IEEE80211_SCA)
		return CHAN_OFFSET_UPPER;
	else if (ch_offset == IEEE80211_SCB)
		return CHAN_OFFSET_LOWER;

	return CHAN_OFFSET_NO_EXT;
}

inline u8 hal_ch_offset_to_secondary_ch_offset(u8 ch_offset)
{
	if (ch_offset == CHAN_OFFSET_NO_EXT)
		return IEEE80211_SCN;
	else if (ch_offset == CHAN_OFFSET_UPPER)
		return IEEE80211_SCA;
	else if (ch_offset == CHAN_OFFSET_LOWER)
		return IEEE80211_SCB;

	return IEEE80211_SCN;
}

inline u8 *rtw_set_ie_secondary_ch_offset(u8 *buf, u32 *buf_len, u8 secondary_ch_offset)
{
	return rtw_set_ie(buf, WLAN_EID_SECONDARY_CHANNEL_OFFSET,  1, &secondary_ch_offset, buf_len);
}

inline u8 *rtw_set_ie_wide_bw_ch_switch(u8 *buf, u32 *buf_len,
	u8 ch_width, u8 seg_0, u8 seg_1)
{
	u8 csw_data[3] = {0};

	/*
	* [0] : New Channel Width
	* [1] : New Channel Center Frequency Segment 0
	* [2] : New Channel Center Frequency Segment 1
	*/
	csw_data[0] = ch_width;
	csw_data[1] = seg_0;
	csw_data[2] = seg_1;

	return rtw_set_ie(buf, WLAN_EID_VHT_WIDE_BW_CHSWITCH, 3, csw_data, buf_len);
}

inline u8 *rtw_set_ie_mesh_ch_switch_parm(u8 *buf, u32 *buf_len, u8 ttl,
		u8 flags, u16 reason, u16 precedence)
{
	u8 ie_data[6];

	ie_data[0] = ttl;
	ie_data[1] = flags;
	RTW_PUT_LE16((u8 *)&ie_data[2], reason);
	RTW_PUT_LE16((u8 *)&ie_data[4], precedence);

	return rtw_set_ie(buf, 0x118,  6, ie_data, buf_len);
}

/*----------------------------------------------------------------------------
index: the information element id index, limit is the limit for search
-----------------------------------------------------------------------------*/
u8 *rtw_get_ie(const u8 *pbuf, sint index, sint *len, sint limit)
{
	sint tmp, i;
	const u8 *p;
	if (limit < 1) {
		return NULL;
	}

	p = pbuf;
	i = 0;
	*len = 0;
	while (1) {
		if (*p == index) {
			*len = *(p + 1);
			return (u8 *)p;
		} else {
			tmp = *(p + 1);
			p += (tmp + 2);
			i += (tmp + 2);
		}
		if (i >= limit)
			break;
	}
	return NULL;
}

u8 *rtw_get_ext_ie(const u8 *pbuf, sint ext_id, sint *len, sint limit)
{
	sint tmp, i;
	const u8 *p;

	if (limit < 3)
		return NULL;

	p = pbuf;
	i = 0;
	*len = 0;
	do {
		if (*p == WLAN_EID_EXTENSION && *(p + 2) == ext_id) {
			*len = *(p + 1);
			return (u8 *)p;
		} else {
			tmp = *(p + 1);
			p += (tmp + 2);
			i += (tmp + 2);
		}
	} while (i + 3 <= limit);

	return NULL;
}

/**
 * rtw_get_ie_ex - Search specific IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @eid: Element ID to match
 * @oui: OUI to match
 * @oui_len: OUI length
 * @ie: If not NULL and the specific IE is found, the IE will be copied to the buf starting from the specific IE
 * @ielen: If not NULL and the specific IE is found, will set to the length of the entire IE
 *
 * Returns: The address of the specific IE found, or NULL
 */
u8 *rtw_get_ie_ex(const u8 *in_ie, uint in_len, u8 eid, const u8 *oui, u8 oui_len, u8 *ie, uint *ielen)
{
	uint cnt;
	const u8 *target_ie = NULL;


	if (ielen)
		*ielen = 0;

	if (!in_ie || in_len <= 0)
		return (u8 *)target_ie;

	cnt = 0;

	while (cnt < in_len) {
		if (eid == in_ie[cnt]
		    && (!oui || _rtw_memcmp(&in_ie[cnt + 2], oui, oui_len) == _TRUE)) {
			target_ie = &in_ie[cnt];

			if (ie)
				_rtw_memcpy(ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			if (ielen)
				*ielen = in_ie[cnt + 1] + 2;

			break;
		} else {
			cnt += in_ie[cnt + 1] + 2; /* goto next	 */
		}

	}

	return (u8 *)target_ie;
}

/**
 * rtw_ies_update_ie - Find matching IEs and update it
 *
 * @ies: address of IEs to search
 * @ies_len: address of length of ies, will update to new length
 * @offset: the offset to start scarch
 * @eid: element ID to match
 * @content: new content will update to matching element
 * @content_len: length of new content
 * Returns: _SUCCESS: ies is updated, _FAIL: not updated
 */
u8 rtw_ies_update_ie(u8 *ies, uint *ies_len, uint ies_offset, u8 eid, const u8 *content, u8 content_len)
{
	u8 ret = _FAIL;
	u8 *target_ie;
	u32 target_ielen;
	u8 *start, *remain_ies = NULL, *backup_ies = NULL;
	uint search_len, remain_len = 0;
	sint offset;

	if (ies == NULL || *ies_len == 0 || *ies_len <= ies_offset)
		goto exit;

	start = ies + ies_offset;
	search_len = *ies_len - ies_offset;

	target_ie = rtw_get_ie(start, eid, &target_ielen, search_len);
	if (target_ie && target_ielen) {
		if (target_ielen != content_len) {
			remain_ies = target_ie + 2 + target_ielen;
			remain_len = search_len - (remain_ies - start);

			backup_ies = rtw_malloc(remain_len);
			if (!backup_ies)
				goto exit;

			_rtw_memcpy(backup_ies, remain_ies, remain_len);
		}

		_rtw_memcpy(target_ie + 2, content, content_len);
		*(target_ie + 1) = content_len;
		ret = _SUCCESS;

		if (target_ielen != content_len) {
			remain_ies = target_ie + 2 + content_len;
			_rtw_memcpy(remain_ies, backup_ies, remain_len);
			rtw_mfree(backup_ies, remain_len);
			offset = content_len - target_ielen;
			*ies_len = *ies_len + offset;
		}
	}
exit:
	return ret;
}

u8 rtw_ies_update_ie_ex(u8 *ies,
		uint *ies_len,
		uint ies_offset,
		u8 eid_ex,
		const u8 *content,
		u8 content_len)
{
	u8 ret = _FAIL;
	u8 *target_ie;
	u32 target_ielen;
	u8 *start, *remain_ies = NULL, *backup_ies = NULL;
	uint search_len, remain_len = 0;
	sint offset;

	if (ies == NULL || *ies_len == 0 || *ies_len <= ies_offset)
		goto exit;

	start = ies + ies_offset;
	search_len = *ies_len - ies_offset;

	/* | Element ID extension | Ext. tag lendth | Ext. tag number | ... | */
	/* | target_ie | (target_ielen-2) | eid_ex | ... | */
	target_ie = rtw_get_ie_ex(start, search_len, WLAN_EID_EXTENSION,
							&eid_ex, 1, NULL, &target_ielen);
	if (target_ie && target_ielen) {
		if (target_ielen != content_len) {
			remain_ies = target_ie + target_ielen;
			remain_len = search_len - (remain_ies - start);

			backup_ies = rtw_malloc(remain_len);
			if (!backup_ies)
				goto exit;

			_rtw_memcpy(backup_ies, remain_ies, remain_len);
		}

		_rtw_memcpy(target_ie, content, content_len);
		*(target_ie + 1) = content_len - 2;
		ret = _SUCCESS;

		if (target_ielen != content_len) {
			remain_ies = target_ie + content_len;
			_rtw_memcpy(remain_ies, backup_ies, remain_len);
			rtw_mfree(backup_ies, remain_len);
			offset = content_len - target_ielen;
			*ies_len = *ies_len + offset;
		}
	}
exit:
	return ret;
}

/**
 * rtw_ies_add_ie - append IE to existing IEs
 *
 * @ies: address of IEs to search
 * @ies_len: address of length of ies, will update to new length
 * @offset: the offset to start scarch
 * @eid: element ID to add
 * @content: content will be appended before the first element ID that is
 * greater than the @eid or overwrite the existing same element ID.
 * @content_len: length of content
 * Returns: _SUCCESS: ies is appended, _FAIL: not appended
 *
 * Notes: this api does not consider the element ID like
 * WLAN_EID_VENDOR_SPECIFIC(221), which contains different OUIs with the same
 * element ID or like WLAN_EID_EXTENSION(255), which contains different
 * extention tag number with the same IE.
 */
u8 rtw_ies_add_ie(u8 *ies, uint *ies_len, uint ies_offset, u8 eid,
		     const u8 *content, u8 content_len)
{
	u8 ret = _FAIL;
	PNDIS_802_11_VARIABLE_IEs pIE = {0};
	u8 *pstart = NULL, *pcurrent = NULL, *premainder_id = NULL;
	u8 *pbackup_remainder_ie = NULL, *dst_ie = NULL;
	uint search_len = 0, remainder_ielen = 0;
	u32 offset = 0;

	if (ies == NULL || *ies_len == 0 || *ies_len <= ies_offset)
		goto exit;

	pstart = ies + ies_offset;
	search_len = *ies_len - ies_offset;

	for (pcurrent = pstart; pcurrent - pstart <= search_len;
	     pcurrent += (pIE->Length + 2)) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)pcurrent;
		dst_ie = pcurrent;

		if (pcurrent - pstart == search_len)
			break;

		if (pIE->ElementID > eid)
			break;
		else if (pIE->ElementID == eid) {
			pcurrent += (pIE->Length + 2);
			break;
		}
	}

	if (dst_ie == NULL)
		goto exit;

	remainder_ielen = *ies_len - (pcurrent - ies);
	if (remainder_ielen > 0) {
		pbackup_remainder_ie = rtw_malloc(remainder_ielen);
		_rtw_memcpy(pbackup_remainder_ie, pcurrent, remainder_ielen);
	}

	*dst_ie++ = eid;
	*dst_ie++ = content_len;

	_rtw_memcpy(dst_ie, content, content_len);
	dst_ie += content_len;

	/* Append remainder IE */
	if (pbackup_remainder_ie) {
		_rtw_memcpy(dst_ie, pbackup_remainder_ie, remainder_ielen);
		rtw_mfree(pbackup_remainder_ie, remainder_ielen);
	}

	offset = (uint)(dst_ie - ies);
	*ies_len = offset + remainder_ielen;
	ret = _SUCCESS;
exit:
	return ret;
}
/**
 * rtw_ies_remove_ie - Find matching IEs and remove
 * @ies: Address of IEs to search
 * @ies_len: Pointer of length of ies, will update to new length
 * @offset: The offset to start scarch
 * @eid: Element ID to match
 * @oui: OUI to match
 * @oui_len: OUI length
 *
 * Returns: _SUCCESS: ies is updated, _FAIL: not updated
 */
int rtw_ies_remove_ie(u8 *ies, uint *ies_len, uint offset, u8 eid, u8 *oui, u8 oui_len)
{
	int ret = _FAIL;
	u8 *target_ie;
	u32 target_ielen;
	u8 *start;
	uint search_len;

	if (!ies || !ies_len || *ies_len <= offset)
		goto exit;

	start = ies + offset;
	search_len = *ies_len - offset;

	while (1) {
		target_ie = rtw_get_ie_ex(start, search_len, eid, oui, oui_len, NULL, &target_ielen);
		if (target_ie && target_ielen) {
			u8 *remain_ies = target_ie + target_ielen;
			uint remain_len = search_len - (remain_ies - start);

			_rtw_memmove(target_ie, remain_ies, remain_len);
			*ies_len = *ies_len - target_ielen;
			ret = _SUCCESS;

			start = target_ie;
			search_len = remain_len;
		} else
			break;
	}
exit:
	return ret;
}

void rtw_set_supported_rate(u8 *SupportedRates, uint mode, u8 ch, enum band_type band)
{
	int is_2G_band = 1;
	int cck_len = 0;

	if (ch > 14 || band == BAND_ON_6G)
		is_2G_band = 0;

	_rtw_memset(SupportedRates, 0, NDIS_802_11_LENGTH_RATES_EX);

	if (is_2G_band) {
		if (mode & WLAN_MD_11B) {
			_rtw_memcpy(SupportedRates, WIFI_CCKRATES, IEEE80211_CCK_RATE_LEN);
			cck_len = IEEE80211_CCK_RATE_LEN;
		}

		if (mode & ~WLAN_MD_11B) /* NOT B only */
			_rtw_memcpy(SupportedRates + cck_len,
				WIFI_OFDMRATES, IEEE80211_NUM_OFDM_RATESLEN);
	} else {/* 5G and 6G */
		_rtw_memcpy(SupportedRates, WIFI_OFDMRATES, IEEE80211_NUM_OFDM_RATESLEN);
	}
}

void rtw_filter_suppport_rateie(WLAN_BSSID_EX *pbss_network, u8 keep)
{
	u8 i, idx = 0, new_rate[NDIS_802_11_LENGTH_RATES_EX], *p;
	uint iscck, isofdm, ie_orilen = 0, remain_len;
	u8 *remain_ies;

	p = rtw_get_ie(pbss_network->IEs + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &ie_orilen, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if (!p)
		return;

	_rtw_memset(new_rate, 0, NDIS_802_11_LENGTH_RATES_EX);
	for (i=0; i < ie_orilen; i++) {
		iscck = rtw_is_cck_rate(p[i+2]);
		isofdm= rtw_is_ofdm_rate(p[i+2]);
		if (((keep == CCK) && iscck)
			|| ((keep == OFDM) && isofdm))
			new_rate[idx++]= rtw_is_basic_rate_ofdm(p[i+2]) ? p[i+2]|IEEE80211_BASIC_RATE_MASK : p[i+2];
	}
	/*	update rate ie	*/
	p[1] = idx;
	_rtw_memcpy(p+2, new_rate, idx);
	/*	update remain ie & IELength*/
	remain_ies = p + 2 + ie_orilen;
	remain_len = pbss_network->IELength - (remain_ies - pbss_network->IEs);
	_rtw_memmove(p+2+idx, remain_ies, remain_len);
	pbss_network->IELength -= (ie_orilen - idx);
}
 

/*
	Adjust those items by given wireless_mode
		1. pbss_network->IELength
		2. pbss_network->IE (SUPPORTRATE & EXT_SUPPORTRATE)
		3. pbss_network->SupportedRates
*/

u8 rtw_update_rate_bymode(WLAN_BSSID_EX *pbss_network, u32 mode)
{
	u8 network_type, *p, *ie = pbss_network->IEs;
	sint ie_len;
	uint network_ielen = pbss_network->IELength;

	if (mode == WLAN_MD_11B) {
		/*only keep CCK in support_rate IE and remove whole ext_support_rate IE*/
		rtw_filter_suppport_rateie(pbss_network, CCK);
		p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &ie_len, pbss_network->IELength - _BEACON_IE_OFFSET_);
		if (p) {
			rtw_ies_remove_ie(ie , &network_ielen, _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, NULL, 0);
			pbss_network->IELength -= ie_len;
		}
		network_type = WLAN_MD_11B;
	} else {
		if (BSS_EX_OP_BAND(pbss_network) != BAND_ON_24G){
			/* Remove CCK in support_rate IE */
			rtw_filter_suppport_rateie(pbss_network, OFDM);
			network_type = WLAN_MD_11A;
		} else {
			if ((mode & WLAN_MD_11B) == 0) {
				/* Remove CCK in support_rate IE */
				rtw_filter_suppport_rateie(pbss_network, OFDM);
				network_type = WLAN_MD_11G;
			} else {
				network_type = WLAN_MD_11BG;
			}
		}
	}

	rtw_set_supported_rate(pbss_network->SupportedRates, network_type,
			       pbss_network->Configuration.DSConfig,
			       pbss_network->Configuration.Band);

	return network_type;
}

uint	rtw_get_rateset_len(u8	*rateset)
{
	uint i = 0;
	while (1) {
		if ((rateset[i]) == 0)
			break;

		if (i > 12)
			break;

		i++;
	}
	return i;
}

int rtw_generate_ie(struct registry_priv *pregistrypriv)
{
	u8	wireless_mode;
	int	sz = 0, rateLen;
	WLAN_BSSID_EX	*pdev_network = &pregistrypriv->dev_network;
	u8	*ie = pdev_network->IEs;


	/* timestamp will be inserted by hardware */
	sz += 8;
	ie += sz;

	/* beacon interval : 2bytes */
	*(u16 *)ie = cpu_to_le16((u16)pdev_network->Configuration.BeaconPeriod); /* BCN_INTERVAL; */
	sz += 2;
	ie += 2;

	/* capability info */
	*(u16 *)ie = 0;

	*(u16 *)ie |= cpu_to_le16(cap_IBSS);

	if (pregistrypriv->preamble == PREAMBLE_SHORT)
		*(u16 *)ie |= cpu_to_le16(cap_ShortPremble);

	if (pdev_network->Privacy)
		*(u16 *)ie |= cpu_to_le16(cap_Privacy);

	sz += 2;
	ie += 2;

	/* SSID */
	ie = rtw_set_ie(ie, _SSID_IE_, pdev_network->Ssid.SsidLength, pdev_network->Ssid.Ssid, &sz);

	/* supported rates */
	if (pregistrypriv->wireless_mode == WLAN_MD_11ABGN) {
		if (pdev_network->Configuration.DSConfig > 14)
			wireless_mode = WLAN_MD_11AN;
		else
			wireless_mode = WLAN_MD_11BGN;
	} else if (pregistrypriv->wireless_mode == WLAN_MD_MAX) { /* WIRELESS_11ABGN | WIRELESS_11AC */
		if (pdev_network->Configuration.DSConfig > 14)
			wireless_mode = WLAN_MD_5G_MIX;
		else
			wireless_mode = WLAN_MD_24G_MIX;
	} else
		wireless_mode = pregistrypriv->wireless_mode;

	rtw_set_supported_rate(pdev_network->SupportedRates, wireless_mode,
			       pdev_network->Configuration.DSConfig,
			       pdev_network->Configuration.Band) ;

	rateLen = rtw_get_rateset_len(pdev_network->SupportedRates);

	if (rateLen > 8) {
		ie = rtw_set_ie(ie, _SUPPORTEDRATES_IE_, 8, pdev_network->SupportedRates, &sz);
		/* ie = rtw_set_ie(ie, _EXT_SUPPORTEDRATES_IE_, (rateLen - 8), (pdev_network->SupportedRates + 8), &sz); */
	} else
		ie = rtw_set_ie(ie, _SUPPORTEDRATES_IE_, rateLen, pdev_network->SupportedRates, &sz);

	/* DS parameter set */
	ie = rtw_set_ie(ie, _DSSET_IE_, 1, (u8 *)&(pdev_network->Configuration.DSConfig), &sz);


	/* IBSS Parameter Set */

	ie = rtw_set_ie(ie, _IBSS_PARA_IE_, 2, (u8 *)&(pdev_network->Configuration.ATIMWindow), &sz);

	if (rateLen > 8)
		ie = rtw_set_ie(ie, _EXT_SUPPORTEDRATES_IE_, (rateLen - 8), (pdev_network->SupportedRates + 8), &sz);

#ifdef CONFIG_80211N_HT
	/* HT Cap. */
	if (is_supported_ht(pregistrypriv->wireless_mode)
	    && (pregistrypriv->ht_enable == _TRUE)) {
		/* todo: */
	}
#endif /* CONFIG_80211N_HT */

	/* pdev_network->IELength =  sz; */ /* update IELength */


	/* return _SUCCESS; */

	return sz;

}

unsigned char *rtw_get_wpa_ie(unsigned char *pie, int *wpa_ie_len, int limit)
{
	int len;
	u16 val16;
	unsigned char wpa_oui_type[] = {0x00, 0x50, 0xf2, 0x01};
	u8 *pbuf = pie;
	int limit_new = limit;

	while (1) {
		pbuf = rtw_get_ie(pbuf, _WPA_IE_ID_, &len, limit_new);

		if (pbuf) {

			/* check if oui matches... */
			if (_rtw_memcmp((pbuf + 2), wpa_oui_type, sizeof(wpa_oui_type)) == _FALSE)

				goto check_next_ie;

			/* check version... */
			_rtw_memcpy((u8 *)&val16, (pbuf + 6), sizeof(val16));

			val16 = le16_to_cpu(val16);
			if (val16 != 0x0001)
				goto check_next_ie;

			*wpa_ie_len = *(pbuf + 1);

			return pbuf;

		} else {

			*wpa_ie_len = 0;
			return NULL;
		}

check_next_ie:

		limit_new = limit - (pbuf - pie) - 2 - len;

		if (limit_new <= 0)
			break;

		pbuf += (2 + len);

	}

	*wpa_ie_len = 0;

	return NULL;

}

unsigned char *rtw_get_wpa2_ie(unsigned char *pie, int *rsn_ie_len, int limit)
{

	return rtw_get_ie(pie, _WPA2_IE_ID_, rsn_ie_len, limit);

}

int rtw_get_wpa_cipher_suite(u8 *s)
{
	if (_rtw_memcmp(s, WPA_CIPHER_SUITE_NONE, WPA_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_NONE;
	if (_rtw_memcmp(s, WPA_CIPHER_SUITE_WEP40, WPA_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_WEP40;
	if (_rtw_memcmp(s, WPA_CIPHER_SUITE_TKIP, WPA_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_TKIP;
	if (_rtw_memcmp(s, WPA_CIPHER_SUITE_CCMP, WPA_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_CCMP;
	if (_rtw_memcmp(s, WPA_CIPHER_SUITE_WEP104, WPA_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_WEP104;

	return 0;
}

int rtw_get_rsn_cipher_suite(u8 *s)
{
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_NONE, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_NONE;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_WEP40, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_WEP40;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_TKIP, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_TKIP;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_CCMP, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_CCMP;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_GCMP, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_GCMP;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_GCMP_256, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_GCMP_256;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_CCMP_256, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_CCMP_256;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_WEP104, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_WEP104;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_AES_128_CMAC, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_BIP_CMAC_128;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_BIP_GMAC_128, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_BIP_GMAC_128;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_BIP_GMAC_256, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_BIP_GMAC_256;
	if (_rtw_memcmp(s, RSN_CIPHER_SUITE_BIP_CMAC_256, RSN_SELECTOR_LEN) == _TRUE)
		return WPA_CIPHER_BIP_CMAC_256;
	return 0;
}

u32 rtw_get_akm_suite_bitmap(u8 *s)
{
	if (_rtw_memcmp(s, WLAN_AKM_8021X, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_8021X;
	if (_rtw_memcmp(s, WLAN_AKM_PSK, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_PSK;
	if (_rtw_memcmp(s, WLAN_AKM_FT_8021X, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_FT_8021X;
	if (_rtw_memcmp(s, WLAN_AKM_FT_PSK, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_FT_PSK;
	if (_rtw_memcmp(s, WLAN_AKM_8021X_SHA256, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_8021X_SHA256;
	if (_rtw_memcmp(s, WLAN_AKM_PSK_SHA256, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_PSK_SHA256;
	if (_rtw_memcmp(s, WLAN_AKM_TDLS, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_TDLS;
	if (_rtw_memcmp(s, WLAN_AKM_SAE, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_SAE;
	if (_rtw_memcmp(s, WLAN_AKM_FT_OVER_SAE, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_FT_OVER_SAE;
	if (_rtw_memcmp(s, WLAN_AKM_8021X_SUITE_B, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_8021X_SUITE_B;
	if (_rtw_memcmp(s, WLAN_AKM_8021X_SUITE_B_192, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_8021X_SUITE_B_192;
	if (_rtw_memcmp(s, WLAN_AKM_FILS_SHA256, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_FILS_SHA256;
	if (_rtw_memcmp(s, WLAN_AKM_FILS_SHA384, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_FILS_SHA384;
	if (_rtw_memcmp(s, WLAN_AKM_FT_FILS_SHA256, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_FT_FILS_SHA256;
	if (_rtw_memcmp(s, WLAN_AKM_FT_FILS_SHA384, RSN_SELECTOR_LEN) == _TRUE)
		return WLAN_AKM_TYPE_FT_FILS_SHA384;

	return 0;
}

int rtw_parse_wpa_ie(u8 *wpa_ie, int wpa_ie_len, int *group_cipher,
	int *pairwise_cipher, u32 *akm)
{
	int i, ret = _SUCCESS;
	int left, count;
	u8 *pos;
	u8 SUITE_1X[4] = {0x00, 0x50, 0xf2, 1};

	if (wpa_ie_len <= 0) {
		/* No WPA IE - fail silently */
		return _FAIL;
	}


	if ((*wpa_ie != _WPA_IE_ID_) || (*(wpa_ie + 1) != (u8)(wpa_ie_len - 2)) ||
	    (_rtw_memcmp(wpa_ie + 2, RTW_WPA_OUI_TYPE, WPA_SELECTOR_LEN) != _TRUE))
		return _FAIL;

	pos = wpa_ie;

	pos += 8;
	left = wpa_ie_len - 8;


	/* group_cipher */
	if (left >= WPA_SELECTOR_LEN) {

		*group_cipher = rtw_get_wpa_cipher_suite(pos);

		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;

	} else if (left > 0) {

		return _FAIL;
	}


	/* pairwise_cipher */
	if (left >= 2) {
		/* count = le16_to_cpu(*(u16*)pos);	 */
		count = RTW_GET_LE16(pos);
		pos += 2;
		left -= 2;

		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			return _FAIL;
		}

		for (i = 0; i < count; i++) {
			*pairwise_cipher |= rtw_get_wpa_cipher_suite(pos);

			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}

	} else if (left == 1) {
		return _FAIL;
	}

	if (akm) {
		if (left >= 6) {
			pos += 2;
			if (_rtw_memcmp(pos, SUITE_1X, 4) == 1) {
				*akm = WLAN_AKM_TYPE_8021X;
			}
		}
	}

	return ret;

}

int rtw_rsne_info_parse(const u8 *ie, uint ie_len, struct rsne_info *info)
{
	const u8 *pos = ie;
	u16 cnt;

	_rtw_memset(info, 0, sizeof(struct rsne_info));

	/* wpa2-imp 4.2.2 F */
	if (ie + ie_len < pos + RSN_HEADER_LEN) {
		RTW_INFO("%s: Invalid RSN IE length\n", __func__);
		goto err;
	}

	if (*ie != WLAN_EID_RSN || *(ie + 1) != ie_len - 2)
		goto err;

	/* wpa2-imp 4.2.2 A */
	if ((u16)(*(ie + 2)) != RSN_VERSION_BSD) {
		RTW_INFO("%s: Invalid RSN version\n", __func__);
		goto err;
	}
	pos += RSN_HEADER_LEN;

	/* ------------------- */
	/* Group CS processing */
	/* ------------------- */
	/* wpa2-imp 4.2.2 G */
	if (ie + ie_len < pos + RSN_SELECTOR_LEN) {
		if (ie + ie_len != pos) {
			RTW_INFO("%s: Group Cipher Suite truncated\n", __func__);
			goto err;
		}
		goto exit;
	}
	info->gcs = (u8 *)pos;
	pos += RSN_SELECTOR_LEN;

	/* ---------------------- */
	/* Pairwise CS processing */
	/* ---------------------- */
	if (ie + ie_len < pos + 2) {
		if (ie + ie_len != pos)
			goto err;
		goto exit;
	}
	cnt = RTW_GET_LE16(pos);
	pos += 2;

	/* wpa-imp 4.2.2 B D */
	if (cnt == 0) {
		RTW_INFO("%s: Pairwise Suite Count Invalid\n", __func__);
		goto err;
	}

	/* wpa2-imp 4.2.2 H L */
	if (ie + ie_len < pos + RSN_SELECTOR_LEN * cnt) {
		if (ie + ie_len != pos) {
			RTW_INFO("%s: Pairwise Suite truncated\n", __func__);
			goto err;
		}
		goto exit;
	}
	info->pcs_cnt = cnt;
	info->pcs_list = (u8 *)pos;
	pos += RSN_SELECTOR_LEN * cnt;

	/* -------------- */
	/* AKM processing */
	/* -------------- */
	if (ie + ie_len < pos + 2) {
		if (ie + ie_len != pos)
			goto err;
		goto exit;
	}
	cnt = RTW_GET_LE16(pos);
	pos += 2;

	/* wpa-imp 4.2.2 C D */
	if (cnt == 0) {
		RTW_INFO("%s: AKM Count Invalid\n", __func__);
		goto err;
	}

	/* wpa2-imp 4.2.2 I M */
	if (ie + ie_len < pos + RSN_SELECTOR_LEN * cnt) {
		if (ie + ie_len != pos) {
			RTW_INFO("%s: AKM truncated\n", __func__);
			goto err;
		}
		goto exit;
	}
	info->akm_cnt = cnt;
	info->akm_list = (u8 *)pos;
	pos += RSN_SELECTOR_LEN * cnt;

	/* ------------------------- */
	/* RSN Capability processing */
	/* ------------------------- */
	/* wpa2-imp 4.2.2 J */
	if (ie + ie_len < pos + 2) {
		if (ie + ie_len != pos) {
			RTW_INFO("%s: RSN Capability truncated\n", __func__);
			goto err;
		}
		goto exit;
	}
	info->cap = (u8 *)pos;
	pos += 2;

	/* ---------------- */
	/* PMKID processing */
	/* ---------------- */
	/* wpa2-imp 4.2.2 K */
	if (ie + ie_len < pos + 2) {
		if (ie + ie_len != pos) {
			RTW_INFO("%s: PMKID count truncated\n", __func__);
			goto err;
		}
		goto exit;
	}
	cnt = RTW_GET_LE16(pos);
	pos += 2;

	/* wpa2-imp 4.2.2 N */
	if (ie + ie_len < pos + RSN_PMKID_LEN * cnt) {
		if (ie + ie_len != pos || cnt) {
			RTW_INFO("%s: PMKID list truncated\n", __func__);
			goto err;
		}
		goto exit;
	}
	info->pmkid_cnt = cnt;
	info->pmkid_list = (u8 *)pos;
	pos += RSN_PMKID_LEN * cnt;

	/* --------------------------------------- */
	/* Group Management Cipher Suit processing */
	/* --------------------------------------- */
	if (ie + ie_len < pos + RSN_SELECTOR_LEN) {
		if (ie + ie_len != pos)
			goto err;
		goto exit;
	}
	info->gmcs = (u8 *)pos;

exit:
	return _SUCCESS;

err:
	info->err = 1;
	return _FAIL;
}

int rtw_parse_wpa2_ie(u8 *rsn_ie, int rsn_ie_len, int *group_cipher,
	int *pairwise_cipher, int *gmcs, u32 *akm, u8 *mfp_opt, u8 *spp_opt)
{
	struct rsne_info info;
	int i, ret = _SUCCESS;

	ret = rtw_rsne_info_parse(rsn_ie, rsn_ie_len, &info);
	if (ret != _SUCCESS)
		goto exit;

	if (group_cipher) {
		if (info.gcs)
			*group_cipher = rtw_get_rsn_cipher_suite(info.gcs);
		else
			*group_cipher = 0;
	}

	if (pairwise_cipher) {
		*pairwise_cipher = 0;
		for (i = 0; i < info.pcs_cnt; i++)
			*pairwise_cipher |= rtw_get_rsn_cipher_suite(info.pcs_list + 4 * i);
	}

	if (gmcs) {
		if (info.gmcs)
			*gmcs = rtw_get_rsn_cipher_suite(info.gmcs);
		else
			*gmcs = WPA_CIPHER_BIP_CMAC_128; /* default value when absent */
	}

	if (akm) {
		*akm = 0;
		for (i = 0; i < info.akm_cnt; i++)
			*akm |= rtw_get_akm_suite_bitmap(info.akm_list + 4 * i);
	}

	if (mfp_opt) {
		*mfp_opt = MFP_NO;
		if (info.cap)
			*mfp_opt = GET_RSN_CAP_MFP_OPTION(info.cap);
	}
	if (spp_opt) {
		*spp_opt = 0;
		if (info.cap)
			*spp_opt = GET_RSN_CAP_SPP_OPT(info.cap);
		}
exit:
	return ret;
}

/* #ifdef CONFIG_WAPI_SUPPORT */
int rtw_get_wapi_ie(u8 *in_ie, uint in_len, u8 *wapi_ie, u16 *wapi_len)
{
	int len = 0;
	u8 authmode;
	uint	cnt;
	u8 wapi_oui1[4] = {0x0, 0x14, 0x72, 0x01};
	u8 wapi_oui2[4] = {0x0, 0x14, 0x72, 0x02};


	if (wapi_len)
		*wapi_len = 0;

	if (!in_ie || in_len <= 0)
		return len;

	cnt = (_TIMESTAMP_ + _BEACON_ITERVAL_ + _CAPABILITY_);

	while (cnt < in_len) {
		authmode = in_ie[cnt];

		/* if(authmode==_WAPI_IE_) */
		if (authmode == _WAPI_IE_ && (_rtw_memcmp(&in_ie[cnt + 6], wapi_oui1, 4) == _TRUE ||
			_rtw_memcmp(&in_ie[cnt + 6], wapi_oui2, 4) == _TRUE)) {
			if (wapi_ie)
				_rtw_memcpy(wapi_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			if (wapi_len)
				*wapi_len = in_ie[cnt + 1] + 2;

			cnt += in_ie[cnt + 1] + 2; /* get next */
		} else {
			cnt += in_ie[cnt + 1] + 2; /* get next */
		}
	}

	if (wapi_len)
		len = *wapi_len;


	return len;

}
/* #endif */

int rtw_get_sec_ie(u8 *in_ie, uint in_len, u8 *rsn_ie, u16 *rsn_len, u8 *wpa_ie, u16 *wpa_len)
{
	u8 authmode, sec_idx;
	u8 wpa_oui[4] = {0x0, 0x50, 0xf2, 0x01};
	uint	cnt;


	/* Search required WPA or WPA2 IE and copy to sec_ie[ ] */

	cnt = (_TIMESTAMP_ + _BEACON_ITERVAL_ + _CAPABILITY_);

	sec_idx = 0;

	while (cnt < in_len) {
		authmode = in_ie[cnt];

		if ((authmode == _WPA_IE_ID_) && (_rtw_memcmp(&in_ie[cnt + 2], &wpa_oui[0], 4) == _TRUE)) {

			if (wpa_ie)
				_rtw_memcpy(wpa_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			*wpa_len = in_ie[cnt + 1] + 2;
			cnt += in_ie[cnt + 1] + 2; /* get next */
		} else {
			if (authmode == _WPA2_IE_ID_) {

				if (rsn_ie)
					_rtw_memcpy(rsn_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

				*rsn_len = in_ie[cnt + 1] + 2;
				cnt += in_ie[cnt + 1] + 2; /* get next */
			} else {
				cnt += in_ie[cnt + 1] + 2; /* get next */
			}
		}

	}


	return *rsn_len + *wpa_len;

}

u8 rtw_is_wps_ie(u8 *ie_ptr, uint *wps_ielen)
{
	u8 match = _FALSE;
	u8 eid, wps_oui[4] = {0x0, 0x50, 0xf2, 0x04};

	if (ie_ptr == NULL)
		return match;

	eid = ie_ptr[0];

	if ((eid == _WPA_IE_ID_) && (_rtw_memcmp(&ie_ptr[2], wps_oui, 4) == _TRUE)) {
		/* RTW_INFO("==> found WPS_IE.....\n"); */
		*wps_ielen = ie_ptr[1] + 2;
		match = _TRUE;
	}
	return match;
}

u8 *rtw_get_wps_ie_from_scan_queue(u8 *in_ie, uint in_len, u8 *wps_ie, uint *wps_ielen, enum bss_type frame_type)
{
	u8	*wps = NULL;

	RTW_INFO("[%s] frame_type = %d\n", __FUNCTION__, frame_type);
	switch (frame_type) {
	case BSS_TYPE_BCN:
	case BSS_TYPE_PROB_RSP: {
		/*	Beacon or Probe Response */
		wps = rtw_get_wps_ie(in_ie + _PROBERSP_IE_OFFSET_, in_len - _PROBERSP_IE_OFFSET_, wps_ie, wps_ielen);
		break;
	}
	case BSS_TYPE_PROB_REQ: {
		/*	Probe Request */
		wps = rtw_get_wps_ie(in_ie + _PROBEREQ_IE_OFFSET_ , in_len - _PROBEREQ_IE_OFFSET_ , wps_ie, wps_ielen);
		break;
	}
	default:
	case BSS_TYPE_UNDEF:
		break;
	}
	return wps;
}

/**
 * rtw_get_wps_ie - Search WPS IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @wps_ie: If not NULL and WPS IE is found, WPS IE will be copied to the buf starting from wps_ie
 * @wps_ielen: If not NULL and WPS IE is found, will set to the length of the entire WPS IE
 *
 * Returns: The address of the WPS IE found, or NULL
 */
u8 *rtw_get_wps_ie(const u8 *in_ie, uint in_len, u8 *wps_ie, uint *wps_ielen)
{
	uint cnt;
	const u8 *wpsie_ptr = NULL;
	u8 eid, wps_oui[4] = {0x00, 0x50, 0xf2, 0x04};

	if (wps_ielen)
		*wps_ielen = 0;

	if (!in_ie) {
		rtw_warn_on(1);
		return (u8 *)wpsie_ptr;
	}

	if (in_len <= 0)
		return (u8 *)wpsie_ptr;

	cnt = 0;

	while (cnt + 1 + 4 < in_len) {
		eid = in_ie[cnt];

		if (cnt + 1 + 4 >= MAX_IE_SZ) {
			rtw_warn_on(1);
			return NULL;
		}

		if (eid == WLAN_EID_VENDOR_SPECIFIC && _rtw_memcmp(&in_ie[cnt + 2], wps_oui, 4) == _TRUE) {
			wpsie_ptr = in_ie + cnt;

			if (wps_ie)
				_rtw_memcpy(wps_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			if (wps_ielen)
				*wps_ielen = in_ie[cnt + 1] + 2;

			break;
		} else
			cnt += in_ie[cnt + 1] + 2;

	}

	return (u8 *)wpsie_ptr;
}

/**
 * rtw_get_wps_attr - Search a specific WPS attribute from a given WPS IE
 * @wps_ie: Address of WPS IE to search
 * @wps_ielen: Length limit from wps_ie
 * @target_attr_id: The attribute ID of WPS attribute to search
 * @buf_attr: If not NULL and the WPS attribute is found, WPS attribute will be copied to the buf starting from buf_attr
 * @len_attr: If not NULL and the WPS attribute is found, will set to the length of the entire WPS attribute
 *
 * Returns: the address of the specific WPS attribute found, or NULL
 */
u8 *rtw_get_wps_attr(u8 *wps_ie, uint wps_ielen, u16 target_attr_id , u8 *buf_attr, u32 *len_attr)
{
	u8 *attr_ptr = NULL;
	u8 *target_attr_ptr = NULL;
	u8 wps_oui[4] = {0x00, 0x50, 0xF2, 0x04};

	if (len_attr)
		*len_attr = 0;

	if ((wps_ie[0] != _VENDOR_SPECIFIC_IE_) ||
	    (_rtw_memcmp(wps_ie + 2, wps_oui , 4) != _TRUE))
		return attr_ptr;

	/* 6 = 1(Element ID) + 1(Length) + 4(WPS OUI) */
	attr_ptr = wps_ie + 6; /* goto first attr */

	while (attr_ptr - wps_ie < wps_ielen) {
		/* 4 = 2(Attribute ID) + 2(Length) */
		u16 attr_id = RTW_GET_BE16(attr_ptr);
		u16 attr_data_len = RTW_GET_BE16(attr_ptr + 2);
		u16 attr_len = attr_data_len + 4;

		/* RTW_INFO("%s attr_ptr:%p, id:%u, length:%u\n", __FUNCTION__, attr_ptr, attr_id, attr_data_len); */
		if (attr_id == target_attr_id) {
			target_attr_ptr = attr_ptr;

			if (buf_attr)
				_rtw_memcpy(buf_attr, attr_ptr, attr_len);

			if (len_attr)
				*len_attr = attr_len;

			break;
		} else {
			attr_ptr += attr_len; /* goto next */
		}

	}

	return target_attr_ptr;
}

/**
 * rtw_get_wps_attr_content - Search a specific WPS attribute content from a given WPS IE
 * @wps_ie: Address of WPS IE to search
 * @wps_ielen: Length limit from wps_ie
 * @target_attr_id: The attribute ID of WPS attribute to search
 * @buf_content: If not NULL and the WPS attribute is found, WPS attribute content will be copied to the buf starting from buf_content
 *               If len_content is NULL, only copy one byte.
 * @len_content: If not NULL and the WPS attribute is found, will set to the length of the WPS attribute content
 *
 * Returns: the address of the specific WPS attribute content found, or NULL
 */
u8 *rtw_get_wps_attr_content(u8 *wps_ie, uint wps_ielen, u16 target_attr_id , u8 *buf_content, uint *len_content)
{
	u8 *attr_ptr;
	u32 attr_len;

	attr_ptr = rtw_get_wps_attr(wps_ie, wps_ielen, target_attr_id, NULL, &attr_len);

	if (attr_ptr && attr_len) {
		if (len_content) {
			if ((buf_content && (*len_content > (attr_len - 4))) || !buf_content)
				*len_content = attr_len - 4;
		}

		if (len_content && buf_content) {
			_rtw_memcpy(buf_content, attr_ptr + 4, *len_content);
		} else if (buf_content) {
			_rtw_memcpy(buf_content, attr_ptr + 4, 1);
		}

		return attr_ptr + 4;
	}

	if (len_content)
		*len_content = 0;

	return NULL;
}

/* OWE */

/**
 * rtw_get_OWE_ie - Search OWE IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @wps_ie: If not NULL and OWE IE is found, OWE IE will be copied to the buf starting from owe_ie
 * @wps_ielen: If not NULL and OWE IE is found, will set to the length of the entire OWE IE
 *
 * Returns: The address of the OWE IE found, or NULL
 */
u8 *rtw_get_owe_ie(const u8 *in_ie, uint in_len, u8 *owe_ie, uint *owe_ielen)
{
	uint cnt;
	const u8 *oweie_ptr = NULL;
	u8 eid;

	if (owe_ielen)
		*owe_ielen = 0;

	if (!in_ie) {
		rtw_warn_on(1);
		return (u8 *)oweie_ptr;
	}

	if (in_len <= 0)
		return (u8 *)oweie_ptr;

	cnt = 0;

	while (cnt + 1 + 4 < in_len) {
		eid = in_ie[cnt];

		if (cnt + 1 + 4 >= MAX_IE_SZ) {
			rtw_warn_on(1);
			return NULL;
		}

		if ((eid == WLAN_EID_EXTENSION) && (in_ie[cnt + 2] == WLAN_EID_EXT_OWE_DH_PARAM)) {
			oweie_ptr = in_ie + cnt;

			if (owe_ie)
				_rtw_memcpy(owe_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			if (owe_ielen)
				*owe_ielen = in_ie[cnt + 1] + 2;

			break;
		} else
			cnt += in_ie[cnt + 1] + 2;

	}

	return (u8 *)oweie_ptr;
}

/* Add extended capabilities element infomation into ext_cap_data of driver */
void rtw_add_ext_cap_info(u8 *ext_cap_data, u8 *ext_cap_data_len, u8 cap_info)
{
	u8 byte_offset = cap_info >> 3;
	u8 bit_offset = cap_info % 8;

	ext_cap_data[byte_offset] |= BIT(bit_offset);

	/* Enlarge the length of EXT_CAP_IE */
	if (byte_offset + 1 > *ext_cap_data_len)
		*ext_cap_data_len = byte_offset + 1;

	#ifdef DBG_EXT_CAP_IE
	RTW_INFO("%s : cap_info = %u, byte_offset = %u, bit_offset = %u, ext_cap_data_len = %u\n", \
			__func__, cap_info, byte_offset, bit_offset, *ext_cap_data_len);
	#endif
}

/* Remvoe extended capabilities element infomation from ext_cap_data of driver */
void rtw_remove_ext_cap_info(u8 *ext_cap_data, u8 *ext_cap_data_len, u8 cap_info)
{
	u8 byte_offset = cap_info >> 3;
	u8 bit_offset = cap_info % 8;
	u8 i, max_len = 0;

	ext_cap_data[byte_offset] &= (~BIT(bit_offset));

	/* Reduce the length of EXT_CAP_IE */
	for (i = 0; i < WLAN_EID_EXT_CAP_MAX_LEN; i++) {
		if (ext_cap_data[i] != 0x0)
			max_len = i + 1;
	}
	*ext_cap_data_len = max_len;

	#ifdef DBG_EXT_CAP_IE
	RTW_INFO("%s : cap_info = %u, byte_offset = %u, bit_offset = %u, ext_cap_data_len = %u\n", \
			__func__, cap_info, byte_offset, bit_offset, *ext_cap_data_len);
	#endif
}

/**
 * rtw_update_ext_cap_ie - add/update/remove the extended capabilities element of frame
 *
 * @ext_cap_data: from &(mlme_priv->ext_capab_ie_data)
 * @ext_cap_data_len: length of ext_cap_data
 * @ies: address of ies, e.g. pnetwork->IEs
 * @ies_len: address of length of ies, e.g. &(pnetwork->IELength)
 * @ies_offset: offset of ies, e.g. _BEACON_IE_OFFSET_
 */
u8 rtw_update_ext_cap_ie(u8 *ext_cap_data, u8 ext_cap_data_len, u8 *ies, u32 *ies_len, u8 ies_offset)
{
	u8 *extcap_ie;
	uint extcap_len_field = 0;
	uint ie_len = 0;

	if (ext_cap_data_len != 0) {
		extcap_ie = rtw_get_ie(ies + ies_offset, WLAN_EID_EXT_CAP, &extcap_len_field, *ies_len - ies_offset);

		if (extcap_ie == NULL) {
			rtw_set_ie(ies + *ies_len, WLAN_EID_EXT_CAP, ext_cap_data_len, ext_cap_data, &ie_len);
			*ies_len += ie_len;
		} else {
			rtw_ies_update_ie(ies, ies_len, ies_offset, WLAN_EID_EXT_CAP, ext_cap_data, ext_cap_data_len);
		}
	} else {
		rtw_ies_remove_ie(ies, ies_len, ies_offset, WLAN_EID_EXT_CAP, NULL, 0);
	}

	return _SUCCESS;
}

void rtw_parse_ext_cap_ie(u8 *ext_cap_data, u8 *ext_cap_data_len, u8 *ies, u32 ies_len, u8 ies_offset)
{
	u8 *extcap_ie;
	uint extcap_len_field = 0;
	u8 i;

	extcap_ie = rtw_get_ie(ies + ies_offset, WLAN_EID_EXT_CAP, &extcap_len_field, ies_len - ies_offset);

	if (extcap_ie != NULL) {
		extcap_ie = extcap_ie + 2; /* element id and length filed */
		if (*ext_cap_data_len == 0) {
			_rtw_memcpy(ext_cap_data, extcap_ie, extcap_len_field);
			*ext_cap_data_len = extcap_len_field;
		} else {
			for (i = 0; i < extcap_len_field; i++)
				ext_cap_data[i] |= extcap_ie[i];
		}

		#ifdef DBG_EXT_CAP_IE
		for (i = 0; i < extcap_len_field; i++)
			RTW_INFO("%s : Parse extended capabilties[%u] = 0x%x\n", __func__, i, extcap_ie[i]);
		#endif
	}
}

static int rtw_ieee802_11_parse_vendor_specific(u8 *pos, uint elen,
		struct rtw_ieee802_11_elems *elems,
		int show_errors)
{
	unsigned int oui;

	/* first 3 bytes in vendor specific information element are the IEEE
	 * OUI of the vendor. The following byte is used a vendor specific
	 * sub-type. */
	if (elen < 4) {
		if (show_errors) {
			RTW_INFO("short vendor specific "
				 "information element ignored (len=%lu)\n",
				 (unsigned long) elen);
		}
		return -1;
	}

	oui = RTW_GET_BE24(pos);
	switch (oui) {
	case OUI_MICROSOFT:
		/* Microsoft/Wi-Fi information elements are further typed and
		 * subtyped */
		switch (pos[3]) {
		case 1:
			/* Microsoft OUI (00:50:F2) with OUI Type 1:
			 * real WPA information element */
			elems->wpa_ie = pos;
			elems->wpa_ie_len = elen;
			break;
		case WME_OUI_TYPE: /* this is a Wi-Fi WME info. element */
			if (elen < 5) {
				RTW_DBG("short WME "
					"information element ignored "
					"(len=%lu)\n",
					(unsigned long) elen);
				return -1;
			}
			switch (pos[4]) {
			case WME_OUI_SUBTYPE_INFORMATION_ELEMENT:
			case WME_OUI_SUBTYPE_PARAMETER_ELEMENT:
				elems->wme = pos;
				elems->wme_len = elen;
				break;
			case WME_OUI_SUBTYPE_TSPEC_ELEMENT:
				elems->wme_tspec = pos;
				elems->wme_tspec_len = elen;
				break;
			default:
				RTW_DBG("unknown WME "
					"information element ignored "
					"(subtype=%d len=%lu)\n",
					pos[4], (unsigned long) elen);
				return -1;
			}
			break;
		case 4:
			/* Wi-Fi Protected Setup (WPS) IE */
			elems->wps_ie = pos;
			elems->wps_ie_len = elen;
			break;
		default:
			RTW_DBG("Unknown Microsoft "
				"information element ignored "
				"(type=%d len=%lu)\n",
				pos[3], (unsigned long) elen);
			return -1;
		}
		break;

	case OUI_BROADCOM:
		switch (pos[3]) {
		case VENDOR_HT_CAPAB_OUI_TYPE:
			elems->vendor_ht_cap = pos;
			elems->vendor_ht_cap_len = elen;
			break;
		default:
			RTW_DBG("Unknown Broadcom "
				"information element ignored "
				"(type=%d len=%lu)\n",
				pos[3], (unsigned long) elen);
			return -1;
		}
		break;
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	case OUI_REALTEK:
		if (elen == 8) {  // TBTX capable IE length is 8
			elems->tbtx_cap = pos;
			elems->tbtx_cap_len = elen;
		}
		break;
#endif
	default:
		RTW_DBG("unknown vendor specific information "
			"element ignored (vendor OUI %02x:%02x:%02x "
			"len=%lu)\n",
			pos[0], pos[1], pos[2], (unsigned long) elen);
		return -1;
	}

	return 0;

}

static int rtw_ieee802_11_parse_ext_elems(u8 *start, uint elen, struct rtw_ieee802_11_elems *elems)
{
	u8 *pos = start;
	u8 id = *pos;

	switch (id) {
	case WLAN_EID_EXTENSION_HE_CAPABILITY:
		elems->he_capabilities = pos;
		elems->he_capabilities_len = elen;
		return 0;
	case WLAN_EID_EXTENSION_HE_OPERATION:
		elems->he_operation = pos;
		elems->he_operation_len = elen;
		return 0;
#if CONFIG_IEEE80211_BAND_6GHZ
	case WLAN_EID_EXT_HE_6G_CAP:
		elems->he_6g_band_cap = pos;
		elems->he_6g_band_cap_len = elen;
		return 0;
#endif
	default:
		return -1;
	}
}

static ParseRes _rtw_ieee802_11_parse_elems(u8 *start, uint len,
				    struct rtw_ieee802_11_elems *elems,
				    int show_errors, bool reset)
{
	uint left = len;
	u8 *pos = start;
	int unknown = 0;

	if (reset)
		_rtw_memset(elems, 0, sizeof(*elems));

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			if (show_errors) {
				RTW_INFO("IEEE 802.11 element "
					 "parse failed (id=%d elen=%d "
					 "left=%lu)\n",
					 id, elen, (unsigned long) left);
			}
			return ParseFailed;
		}

		switch (id) {
		case WLAN_EID_SSID:
			elems->ssid = pos;
			elems->ssid_len = elen;
			break;
		case WLAN_EID_SUPP_RATES:
			elems->supp_rates = pos;
			elems->supp_rates_len = elen;
			break;
		case WLAN_EID_FH_PARAMS:
			elems->fh_params = pos;
			elems->fh_params_len = elen;
			break;
		case WLAN_EID_DS_PARAMS:
			elems->ds_params = pos;
			elems->ds_params_len = elen;
			break;
		case WLAN_EID_CF_PARAMS:
			elems->cf_params = pos;
			elems->cf_params_len = elen;
			break;
		case WLAN_EID_TIM:
			elems->tim = pos;
			elems->tim_len = elen;
			break;
		case WLAN_EID_IBSS_PARAMS:
			elems->ibss_params = pos;
			elems->ibss_params_len = elen;
			break;
		case WLAN_EID_CHALLENGE:
			elems->challenge = pos;
			elems->challenge_len = elen;
			break;
		case WLAN_EID_ERP_INFO:
			elems->erp_info = pos;
			elems->erp_info_len = elen;
			break;
		case WLAN_EID_EXT_SUPP_RATES:
			elems->ext_supp_rates = pos;
			elems->ext_supp_rates_len = elen;
			break;
		case WLAN_EID_VENDOR_SPECIFIC:
			if (rtw_ieee802_11_parse_vendor_specific(pos, elen,
					elems,
					show_errors))
				unknown++;
			break;
		case WLAN_EID_RSN:
			elems->rsn_ie = pos;
			elems->rsn_ie_len = elen;
			break;
		case WLAN_EID_PWR_CAPABILITY:
			elems->power_cap = pos;
			elems->power_cap_len = elen;
			break;
		case WLAN_EID_SUPPORTED_CHANNELS:
			elems->supp_channels = pos;
			elems->supp_channels_len = elen;
			break;
		case WLAN_EID_MOBILITY_DOMAIN:
			elems->mdie = pos;
			elems->mdie_len = elen;
			break;
		case WLAN_EID_FAST_BSS_TRANSITION:
			elems->ftie = pos;
			elems->ftie_len = elen;
			break;
		case WLAN_EID_TIMEOUT_INTERVAL:
			elems->timeout_int = pos;
			elems->timeout_int_len = elen;
			break;
		case WLAN_EID_HT_CAP:
			elems->ht_capabilities = pos;
			elems->ht_capabilities_len = elen;
			break;
		case WLAN_EID_HT_OPERATION:
			elems->ht_operation = pos;
			elems->ht_operation_len = elen;
			break;
		case WLAN_EID_VHT_CAPABILITY:
			elems->vht_capabilities = pos;
			elems->vht_capabilities_len = elen;
			break;
		case WLAN_EID_VHT_OPERATION:
			elems->vht_operation = pos;
			elems->vht_operation_len = elen;
			break;
		case WLAN_EID_VHT_OP_MODE_NOTIFY:
			elems->vht_op_mode_notify = pos;
			elems->vht_op_mode_notify_len = elen;
			break;
		case _EID_RRM_EN_CAP_IE_:
			elems->rm_en_cap = pos;
			elems->rm_en_cap_len = elen;
			break;
		case WLAN_EID_AP_CHANNEL_RPT:
			elems->ap_channel_rpt= pos;
			elems->ap_channel_rpt_len = elen;
			break;
		case WLAN_EID_COUNTRY:
			elems->country_info= pos;
			elems->country_info_len = elen;
			break;
#ifdef CONFIG_RTW_MESH
		case WLAN_EID_PREQ:
			elems->preq = pos;
			elems->preq_len = elen;
			break;
		case WLAN_EID_PREP:
			elems->prep = pos;
			elems->prep_len = elen;
			break;
		case WLAN_EID_PERR:
			elems->perr = pos;
			elems->perr_len = elen;
			break;
		case WLAN_EID_RANN:
			elems->rann = pos;
			elems->rann_len = elen;
			break;
#endif
#ifdef CONFIG_STA_MULTIPLE_BSSID
		case WLAN_EID_MULTIPLE_BSSID:
			elems->mbssid = pos;
			elems->mbssid_len = elen;
			break;
		case WLAN_EID_NON_TX_BSSID_CAP:
			elems->non_tx_bssid_cap = pos;
			elems->non_tx_bssid_cap_len = elen;
			break;
#endif
		case WLAN_EID_EXTENSION:
			rtw_ieee802_11_parse_ext_elems(pos, elen, elems);
			break;
		default:
			unknown++;
			if (!show_errors)
				break;
			RTW_DBG("IEEE 802.11 element parse "
				"ignored unknown element (id=%d elen=%d)\n",
				id, elen);
			break;
		}

		left -= elen;
		pos += elen;
	}

	if (left)
		return ParseFailed;

	return unknown ? ParseUnknown : ParseOK;

}

/**
 * ieee802_11_parse_elems - Parse information elements in management frames
 * @start: Pointer to the start of IEs
 * @len: Length of IE buffer in octets
 * @elems: Data structure for parsed elements
 * @show_errors: Whether to show parsing errors in debug log
 * Returns: Parsing result
 */
ParseRes rtw_ieee802_11_parse_elems(u8 *start, uint len,
				    struct rtw_ieee802_11_elems *elems,
				    int show_errors)
{
	return _rtw_ieee802_11_parse_elems(start, len, elems, show_errors, true);
}

#ifdef CONFIG_STA_MULTIPLE_BSSID
static bool rtw_mbssid_ntbssid_profile_match_id(u8 *profile, uint len, u8 mbssid_idx)
{
	uint left = len;
	u8 *pos = profile;

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left)
			return false;

		switch (id) {
		case WLAN_EID_MULTI_BSSID_IDX:
			if (GET_MULTIPLE_BSSID_IDX_INDEX(pos - 2) == mbssid_idx)
				return true;
			break;
		default:
			break;
		}

		left -= elen;
		pos += elen;
	}

	return false;
}

/**
 * rtw_ieee802_11_override_elems_by_mbssid - override information elements in management frames
 * @mbssid_ie: Pointer to the start of mbssid IE
 * @mbssid_ie_len: Length of IE buffer in octets
 * @mbssid_idx: the specific mbssid index to get for override
 * @elems: Data structure for parsed elements
 * @show_errors: Whether to show parsing errors in debug log
 * Returns: Parsing result
 */
ParseRes rtw_ieee802_11_override_elems_by_mbssid(
	u8 *mbssid_ie, uint mbssid_ie_len, u8 mbssid_idx, struct rtw_ieee802_11_elems *elems
	, int show_errors)
{
	uint left = mbssid_ie_len;
	u8 *pos = mbssid_ie;
	u8 max_bssid_indicator;
	ParseRes ret = ParseOK;

	if (left < 3) {
		RTW_WARN("%s mbssid_ie_len < 3\n", __func__);
		return ParseFailed;
	}

	max_bssid_indicator = GET_MBSSID_MAX_BSSID_INDOCATOR(pos);
	if (mbssid_idx >= (1 << max_bssid_indicator)) {
		RTW_WARN("%s mbssid_idx >= max_bssid_indicator(%u)\n"
			, __func__, 1 << max_bssid_indicator);
		return ParseFailed;
	}

	pos += MBSSID_MAX_BSSID_INDICATOR_OFFSET;
	left -= MBSSID_MAX_BSSID_INDICATOR_OFFSET;

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			if (show_errors) {
				RTW_INFO("%s parse failed (id=%d elen=%d left=%lu)\n"
					, __func__, id, elen, (unsigned long) left);
			}
			return ParseFailed;
		}

		switch (id) {
		case MBSSID_NONTRANSMITTED_BSSID_PROFILE_ID:
			if (rtw_mbssid_ntbssid_profile_match_id(pos, elen, mbssid_idx))
				ret = _rtw_ieee802_11_parse_elems(pos, elen, elems, show_errors, false);
			break;
		default:
			break;
		}
		left -= elen;
		pos += elen;
	}

	if (left)
		return ParseFailed;

	return ret;

}
#endif /* CONFIG_STA_MULTIPLE_BSSID */

static u8 key_char2num(u8 ch);
static u8 key_char2num(u8 ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	else if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	else if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	else
		return 0xff;
}

u8 str_2char2num(u8 hch, u8 lch);
u8 str_2char2num(u8 hch, u8 lch)
{
	return (key_char2num(hch) * 10) + key_char2num(lch);
}

u8 key_2char2num(u8 hch, u8 lch);
u8 key_2char2num(u8 hch, u8 lch)
{
	return (key_char2num(hch) << 4) | key_char2num(lch);
}

void macstr2num(u8 *dst, u8 *src);
void macstr2num(u8 *dst, u8 *src)
{
	int	jj, kk;
	for (jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3)
		dst[jj] = key_2char2num(src[kk], src[kk + 1]);
}

u8 convert_ip_addr(u8 hch, u8 mch, u8 lch)
{
	return (key_char2num(hch) * 100) + (key_char2num(mch) * 10) + key_char2num(lch);
}


/*
 * Description:
 * rtw_check_invalid_mac_address:
 * This is only used for checking mac address valid or not.
 *
 * Input:
 * adapter: mac_address pointer.
 * check_local_bit: check locally bit or not.
 *
 * Output:
 * _TRUE: The mac address is invalid.
 * _FALSE: The mac address is valid.
 *
 * Auther: Isaac.Li
 */
u8 rtw_check_invalid_mac_address(u8 *mac_addr, u8 check_local_bit)
{
	u8 null_mac_addr[ETH_ALEN] = {0, 0, 0, 0, 0, 0};
	u8 multi_mac_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 res = _FALSE;

	if (_rtw_memcmp(mac_addr, null_mac_addr, ETH_ALEN)) {
		res = _TRUE;
		goto func_exit;
	}

	if (_rtw_memcmp(mac_addr, multi_mac_addr, ETH_ALEN)) {
		res = _TRUE;
		goto func_exit;
	}

	if (mac_addr[0] & BIT0) {
		res = _TRUE;
		goto func_exit;
	}

	if (check_local_bit == _TRUE) {
		if (mac_addr[0] & BIT1) {
			res = _TRUE;
			goto func_exit;
		}
	}

func_exit:
	return res;
}

extern char *rtw_initmac;
/**
 * rtw_macaddr_cfg - Decide the mac address used
 * @out: buf to store mac address decided
 * @hw_mac_addr: mac address from efuse/epprom
 */
void rtw_macaddr_cfg(u8 *out, const u8 *hw_mac_addr)
{
#define DEFAULT_RANDOM_MACADDR 1
	u8 mac[ETH_ALEN] = {0};

	if (out == NULL) {
		rtw_warn_on(1);
		return;
	}

	/* Users specify the mac address */
	if (rtw_initmac) {
		int jj, kk;

		for (jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3)
			mac[jj] = key_2char2num(rtw_initmac[kk], rtw_initmac[kk + 1]);

		goto err_chk;
	}


	/* Use the mac address stored in the Efuse */
	if (hw_mac_addr) {
		_rtw_memcpy(mac, hw_mac_addr, ETH_ALEN);
		goto err_chk;
	}

err_chk:
	platform_wifi_mac_addr(mac);

	if (rtw_check_invalid_mac_address(mac, _TRUE) == _TRUE) {
#if DEFAULT_RANDOM_MACADDR
		RTW_ERR("invalid mac addr:"MAC_FMT", assign random MAC\n", MAC_ARG(mac));
		*((u32 *)(&mac[2])) = rtw_random32();
		mac[0] = 0x00;
		mac[1] = 0xe0;
		mac[2] = 0x4c;
#else
		RTW_ERR("invalid mac addr:"MAC_FMT", assign default one\n", MAC_ARG(mac));
		mac[0] = 0x00;
		mac[1] = 0xe0;
		mac[2] = 0x4c;
		mac[3] = 0x87;
		mac[4] = 0x00;
		mac[5] = 0x00;
#endif
	}

	_rtw_memcpy(out, mac, ETH_ALEN);
	RTW_INFO("%s mac addr:"MAC_FMT"\n", __func__, MAC_ARG(out));
}

#ifdef CONFIG_80211N_HT
void dump_ht_cap_ie_content(void *sel, const u8 *buf, u32 buf_len)
{
	if (buf_len != HT_CAP_IE_LEN) {
		RTW_PRINT_SEL(sel, "Invalid HT capability IE len:%d != %d\n", buf_len, HT_CAP_IE_LEN);
		return;
	}

	RTW_PRINT_SEL(sel, "cap_info:%02x%02x:%s\n", *(buf), *(buf + 1)
		, GET_HT_CAP_ELE_CHL_WIDTH(buf) ? " 40MHz" : " 20MHz");
	RTW_PRINT_SEL(sel, "A-MPDU Parameters:"HT_AMPDU_PARA_FMT"\n"
		      , HT_AMPDU_PARA_ARG(HT_CAP_ELE_AMPDU_PARA(buf)));
	RTW_PRINT_SEL(sel, "Supported MCS Set:"HT_SUP_MCS_SET_FMT"\n"
		      , HT_SUP_MCS_SET_ARG(HT_CAP_ELE_SUP_MCS_SET(buf)));
}

void dump_ht_cap_ie(void *sel, const u8 *ie, u32 ie_len)
{
	const u8 *ht_cap_ie;
	sint ht_cap_ielen;

	ht_cap_ie = rtw_get_ie(ie, WLAN_EID_HT_CAP, &ht_cap_ielen, ie_len);
	if (!ie || ht_cap_ie != ie)
		return;

	dump_ht_cap_ie_content(sel, ht_cap_ie + 2, ht_cap_ielen);
}

const char *const _ht_sc_offset_str[] = {
	"SCN",
	"SCA",
	"SC-RSVD",
	"SCB",
};

void dump_ht_op_ie_content(void *sel, const u8 *buf, u32 buf_len)
{
	if (buf_len != HT_OP_IE_LEN) {
		RTW_PRINT_SEL(sel, "Invalid HT operation IE len:%d != %d\n", buf_len, HT_OP_IE_LEN);
		return;
	}

	RTW_PRINT_SEL(sel, "ch:%u%s %s\n"
		, GET_HT_OP_ELE_PRI_CHL(buf)
		, GET_HT_OP_ELE_STA_CHL_WIDTH(buf) ? "" : " 20MHz only"
		, ht_sc_offset_str(GET_HT_OP_ELE_2ND_CHL_OFFSET(buf))
	);
}

void dump_ht_op_ie(void *sel, const u8 *ie, u32 ie_len)
{
	const u8 *ht_op_ie;
	sint ht_op_ielen;

	ht_op_ie = rtw_get_ie(ie, WLAN_EID_HT_OPERATION, &ht_op_ielen, ie_len);
	if (!ie || ht_op_ie != ie)
		return;

	dump_ht_op_ie_content(sel, ht_op_ie + 2, ht_op_ielen);
}
#endif /* CONFIG_80211N_HT */

void dump_ies(void *sel, const u8 *buf, u32 buf_len)
{
	const u8 *pos = buf;
	u8 id, len;

	while (pos - buf + 1 < buf_len) {
		id = *pos;
		len = *(pos + 1);

		RTW_PRINT_SEL(sel, "%s ID:%u, LEN:%u\n", __FUNCTION__, id, len);
#ifdef CONFIG_80211N_HT
		dump_ht_cap_ie(sel, pos, len + 2);
		dump_ht_op_ie(sel, pos, len + 2);
#endif
#ifdef CONFIG_80211AC_VHT
		dump_vht_cap_ie(sel, pos, len + 2);
		dump_vht_op_ie(sel, pos, len + 2);
#endif
#ifdef CONFIG_80211AX_HE
		/* CONFIG_80211AX_HE_TODO */
#endif
		dump_wps_ie(sel, pos, len + 2);
#ifdef CONFIG_P2P
		dump_p2p_ie(sel, pos, len + 2);
#ifdef CONFIG_WFD
		dump_wfd_ie(sel, pos, len + 2);
#endif
#endif
#ifdef CONFIG_RTW_MULTI_AP
		dump_multi_ap_ie(sel, pos, len + 2);
#endif

		pos += (2 + len);
	}
}

void dump_wps_ie(void *sel, const u8 *ie, u32 ie_len)
{
	const u8 *pos = ie;
	u16 id;
	u16 len;

	const u8 *wps_ie;
	uint wps_ielen;

	wps_ie = rtw_get_wps_ie(ie, ie_len, NULL, &wps_ielen);
	if (wps_ie != ie || wps_ielen == 0)
		return;

	pos += 6;
	while (pos - ie + 4 <= ie_len) {
		id = RTW_GET_BE16(pos);
		len = RTW_GET_BE16(pos + 2);

		RTW_PRINT_SEL(sel, "%s ID:0x%04x, LEN:%u%s\n", __func__, id, len
			, ((pos - ie + 4 + len) <= ie_len) ? "" : "(exceed ie_len)");

		pos += (4 + len);
	}
}

#if defined(CONFIG_80211AX_HE) && defined(CONFIG_IEEE80211_BAND_6GHZ)
u8 *rtw_ies_get_he_6g_op_info_ie(u8 *ies, int ies_len)
{
	u32 ie_len;
	u8 he_op_eid_ext = WLAN_EID_EXTENSION_HE_OPERATION;
	u8 *he_op_ie, *_6g_op_info_ie = NULL;

	he_op_ie = rtw_get_ie_ex(ies, ies_len,
		WLAN_EID_EXTENSION, &he_op_eid_ext, 1, NULL, &ie_len);

	if (he_op_ie) {
		if (GET_HE_OP_PARA_6GHZ_OP_INFO_PRESENT(he_op_ie + 3)) {
			u32 len = HE_OPER_PARAMS_LEN +
				HE_OPER_BSS_COLOR_INFO_LEN + HE_OPER_BASIC_MCS_LEN;

			if (GET_HE_OP_PARA_VHT_OP_INFO_PRESENT(he_op_ie + 3))
				len += HE_OPER_VHT_OPER_INFO_LEN;
			if (GET_HE_OP_PARA_CO_HOSTED_BSS(he_op_ie + 3))
				len += HE_OPER_MAX_COHOST_BSSID_LEN;

			_6g_op_info_ie = he_op_ie + 3 + len;
		}
	}
	return _6g_op_info_ie;
}
#endif /* CONFIG_80211AX_HE && CONFIG_IEEE80211_BAND_6GHZ */

/**
 * rtw_ies_get_chbw - get operation ch, bw, offset from IEs of BSS.
 * @ies: pointer of the first tlv IE
 * @ies_len: length of @ies
 * @ch: pointer of ch, used as output
 * @bw: pointer of bw, used as output
 * @offset: pointer of offset, used as output
 * @ht: check HT IEs
 * @vht: check VHT IEs, if true imply ht is true
 */
#if CONFIG_ALLOW_FUNC_2G_5G_ONLY
RTW_FUNC_2G_5G_ONLY inline void rtw_ies_get_chbw(u8 *ies, int ies_len, u8 *ch, u8 *bw, u8 *offset, u8 ht, u8 vht)
{
	rtw_ies_get_bchbw(ies, ies_len, NULL, ch, bw, offset, NULL, NULL, ht, vht, 0);
}
#endif

void rtw_ies_get_bchbw(u8 *ies, int ies_len, enum band_type *band, u8 *chan, u8 *bw,
	u8 *offset, u8 *freq0, u8 *freq1, u8 ht, u8 vht, u8 he)
{
	u8 *p;
	int	ie_len;

	*chan = 0;
	*bw = CHANNEL_WIDTH_20;
	*offset = CHAN_OFFSET_NO_EXT;
	if (freq0)
		*freq0 = 0;
	if (freq1)
		*freq1 = 0;

	p = rtw_get_ie(ies, _DSSET_IE_, &ie_len, ies_len);
	if (p && ie_len > 0) {
		*chan = *(p + 2);
		if (band)
			*band = *chan > 14 ? BAND_ON_5G : BAND_ON_24G;
	}

#ifdef CONFIG_80211N_HT
	if (ht || vht) {
		u8 *ht_cap_ie, *ht_op_ie;
		int ht_cap_ielen, ht_op_ielen;

		ht_cap_ie = rtw_get_ie(ies, EID_HTCapability, &ht_cap_ielen, ies_len);
		if (ht_cap_ie && ht_cap_ielen) {
			if (GET_HT_CAP_ELE_CHL_WIDTH(ht_cap_ie + 2))
				*bw = CHANNEL_WIDTH_40;
		}

		ht_op_ie = rtw_get_ie(ies, EID_HTInfo, &ht_op_ielen, ies_len);
		if (ht_op_ie && ht_op_ielen) {
			if (*chan == 0) {
				*chan = GET_HT_OP_ELE_PRI_CHL(ht_op_ie + 2);
				if(band)
					*band = (*chan > 14) ? BAND_ON_5G : BAND_ON_24G;
			} else if (*chan != 0 && *chan != GET_HT_OP_ELE_PRI_CHL(ht_op_ie + 2)) {
				RTW_INFO("%s ch inconsistent, DSSS:%u, HT primary:%u\n"
					, __func__, *chan, GET_HT_OP_ELE_PRI_CHL(ht_op_ie + 2));
			}

			if (!GET_HT_OP_ELE_STA_CHL_WIDTH(ht_op_ie + 2))
				*bw = CHANNEL_WIDTH_20;

			if (*bw == CHANNEL_WIDTH_40) {
				switch (GET_HT_OP_ELE_2ND_CHL_OFFSET(ht_op_ie + 2)) {
				case IEEE80211_SCA:
					*offset = CHAN_OFFSET_UPPER;
					break;
				case IEEE80211_SCB:
					*offset = CHAN_OFFSET_LOWER;
					break;
				}
			}
		}

#ifdef CONFIG_80211AC_VHT
		if (vht) {
			u8 *vht_op_ie;
			int vht_op_ielen;
			u8 *vht_cap_ie;
			int vht_cap_ielen;

			vht_cap_ie = rtw_get_ie(ies, EID_VHTCapability, &vht_cap_ielen, ies_len);
			if (vht_cap_ie && vht_cap_ielen) {

				vht_op_ie = rtw_get_ie(ies, EID_VHTOperation, &vht_op_ielen, ies_len);
				if (vht_op_ie) {
					*bw = rtw_vht_get_oper_bw(ht_op_ie,
								  vht_cap_ie,
								  vht_op_ie,
								  *bw);
				}
			}
		}
#endif /* CONFIG_80211AC_VHT */
	}
#endif /* CONFIG_80211N_HT */

#ifdef CONFIG_80211AX_HE
#if CONFIG_IEEE80211_BAND_6GHZ
	if (he) {
		u8 *_6g_op_info_ie = NULL;

		_6g_op_info_ie = rtw_ies_get_he_6g_op_info_ie(ies, ies_len);

		if (_6g_op_info_ie) {
			*chan = GET_HE_OP_INFO_PRIMARY_CHAN(_6g_op_info_ie);
			*bw = GET_HE_OP_INFO_CHAN_WIDTH(_6g_op_info_ie);
			if (freq0)
				*freq0 = GET_HE_OP_INFO_CHAN_CTR_FREQ_SEG0(_6g_op_info_ie);
			if (freq1)
				*freq1 = GET_HE_OP_INFO_CHAN_CTR_FREQ_SEG1(_6g_op_info_ie);
			if (band)
				*band = BAND_ON_6G;
			if (!rtw_get_offset_by_bchbw(BAND_ON_6G, *chan, *bw, offset)) {
				RTW_INFO("%s get 6ghz channel offset fail, chan=%u, bw=%u\n",
					__func__, *chan, *bw);
			}
		}
	}
#endif
#endif /* CONFIG_80211AX_HE */
}

void rtw_bss_get_chbw(WLAN_BSSID_EX *bss, enum band_type *band, u8 *ch, u8 *bw, u8 *offset, u8 ht, u8 vht, u8 he)
{
	rtw_ies_get_bchbw(bss->IEs + _FIXED_IE_LENGTH_
		, bss->IELength - _FIXED_IE_LENGTH_
		, band, ch, bw, offset, NULL, NULL, ht, vht, he);

	if (*ch == 0) {
		*ch = bss->Configuration.DSConfig;
		*band = bss->Configuration.Band;
	}
	else if (*ch != bss->Configuration.DSConfig) {
		RTW_INFO("inconsistent ch - ies:%u bss->Configuration.DSConfig:%u\n"
			 , *ch, bss->Configuration.DSConfig);
		*ch = bss->Configuration.DSConfig;
		*band = bss->Configuration.Band;
		rtw_warn_on(1);
	}
}

/**
 * rtw_get_p2p_merged_len - Get merged ie length from muitiple p2p ies.
 * @in_ie: Pointer of the first p2p ie
 * @in_len: Total len of muiltiple p2p ies
 * Returns: Length of merged p2p ie length
 */
u32 rtw_get_p2p_merged_ies_len(u8 *in_ie, u32 in_len)
{
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 OUI[4] = { 0x50, 0x6f, 0x9a, 0x09 };
	int i = 0;
	int len = 0;

	while (i < in_len) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(in_ie + i);

		if (pIE->ElementID == _VENDOR_SPECIFIC_IE_ && _rtw_memcmp(pIE->data, OUI, 4)) {
			len += pIE->Length - 4; /* 4 is P2P OUI length, don't count it in this loop */
		}

		i += (pIE->Length + 2);
	}

	return len + 4;	/* Append P2P OUI length at last. */
}

/**
 * rtw_p2p_merge_ies - Merge muitiple p2p ies into one
 * @in_ie: Pointer of the first p2p ie
 * @in_len: Total len of muiltiple p2p ies
 * @merge_ie: Pointer of merged ie
 * Returns: Length of merged p2p ie
 */
int rtw_p2p_merge_ies(u8 *in_ie, u32 in_len, u8 *merge_ie)
{
	PNDIS_802_11_VARIABLE_IEs	pIE;
	u8 len = 0;
	u8 OUI[4] = { 0x50, 0x6f, 0x9a, 0x09 };
	u8 ELOUI[6] = { 0xDD, 0x00, 0x50, 0x6f, 0x9a, 0x09 };	/* EID;Len;OUI, Len would copy at the end of function */
	int i = 0;

	if (merge_ie != NULL) {
		/* Set first P2P OUI */
		_rtw_memcpy(merge_ie, ELOUI, 6);
		merge_ie += 6;

		while (i < in_len) {
			pIE = (PNDIS_802_11_VARIABLE_IEs)(in_ie + i);

			/* Take out the rest of P2P OUIs */
			if (pIE->ElementID == _VENDOR_SPECIFIC_IE_ && _rtw_memcmp(pIE->data, OUI, 4)) {
				_rtw_memcpy(merge_ie, pIE->data + 4, pIE->Length - 4);
				len += pIE->Length - 4;
				merge_ie += pIE->Length - 4;
			}

			i += (pIE->Length + 2);
		}

		return len + 4;	/* 4 is for P2P OUI */

	}

	return 0;
}

void dump_p2p_ie(void *sel, const u8 *ie, u32 ie_len)
{
	const u8 *pos = ie;
	u8 id;
	u16 len;

	const u8 *p2p_ie;
	uint p2p_ielen;

	p2p_ie = rtw_get_p2p_ie(ie, ie_len, NULL, &p2p_ielen);
	if (p2p_ie != ie || p2p_ielen == 0)
		return;

	pos += 6;
	while (pos - ie + 3 <= ie_len) {
		id = *pos;
		len = RTW_GET_LE16(pos + 1);

		RTW_PRINT_SEL(sel, "%s ID:%u, LEN:%u%s\n", __func__, id, len
			, ((pos - ie + 3 + len) <= ie_len) ? "" : "(exceed ie_len)");

		pos += (3 + len);
	}
}

/**
 * rtw_get_p2p_ie - Search P2P IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @p2p_ie: If not NULL and P2P IE is found, P2P IE will be copied to the buf starting from p2p_ie
 * @p2p_ielen: If not NULL and P2P IE is found, will set to the length of the entire P2P IE
 *
 * Returns: The address of the P2P IE found, or NULL
 */
u8 *rtw_get_p2p_ie(const u8 *in_ie, int in_len, u8 *p2p_ie, uint *p2p_ielen)
{
	uint cnt;
	const u8 *p2p_ie_ptr = NULL;
	u8 eid, p2p_oui[4] = {0x50, 0x6F, 0x9A, 0x09};

	if (p2p_ielen)
		*p2p_ielen = 0;

	if (!in_ie || in_len < 0) {
		rtw_warn_on(1);
		return (u8 *)p2p_ie_ptr;
	}

	if (in_len <= 0)
		return (u8 *)p2p_ie_ptr;

	cnt = 0;

	while (cnt + 1 + 4 < in_len) {
		eid = in_ie[cnt];

		if (cnt + 1 + 4 >= MAX_IE_SZ) {
			rtw_warn_on(1);
			return NULL;
		}

		if (eid == WLAN_EID_VENDOR_SPECIFIC && _rtw_memcmp(&in_ie[cnt + 2], p2p_oui, 4) == _TRUE) {
			p2p_ie_ptr = in_ie + cnt;

			if (p2p_ie)
				_rtw_memcpy(p2p_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			if (p2p_ielen)
				*p2p_ielen = in_ie[cnt + 1] + 2;

			break;
		} else
			cnt += in_ie[cnt + 1] + 2;

	}

	return (u8 *)p2p_ie_ptr;
}

u8 *rtw_get_vendor_ie(const u8 *in_ie, int in_len, u8 *vendor_ie, uint *vendor_ielen)
{
	uint cnt;
	const u8 *vendor_ie_ptr = NULL;
	u8 eid, vendor_oui[4] = {0xC8, 0x3A, 0x6B, 0x01};

	if (vendor_ielen)
		*vendor_ielen = 0;

	if (!in_ie || in_len < 0) {
		rtw_warn_on(1);
		return (u8 *)vendor_ie_ptr;
	}

	if (in_len <= 0)
		return (u8 *)vendor_ie_ptr;

	cnt = 0;

	while (cnt + 1 + 4 < in_len) {
		eid = in_ie[cnt];

		if (cnt + 1 + 4 >= MAX_IE_SZ) {
			rtw_warn_on(1);
			return NULL;
		}

		if (eid == WLAN_EID_VENDOR_SPECIFIC && _rtw_memcmp(&in_ie[cnt + 2], vendor_oui, 4) == _TRUE) {
			vendor_ie_ptr = in_ie + cnt;

			if (vendor_ie)
				_rtw_memcpy(vendor_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			if (vendor_ielen)
				*vendor_ielen = in_ie[cnt + 1] + 2;

			break;
		} else
			cnt += in_ie[cnt + 1] + 2;

	}

	return (u8 *)vendor_ie_ptr;
}

/**
 * rtw_get_p2p_attr - Search a specific P2P attribute from a given P2P IE
 * @p2p_ie: Address of P2P IE to search
 * @p2p_ielen: Length limit from p2p_ie
 * @target_attr_id: The attribute ID of P2P attribute to search
 * @buf_attr: If not NULL and the P2P attribute is found, P2P attribute will be copied to the buf starting from buf_attr
 * @len_attr: If not NULL and the P2P attribute is found, will set to the length of the entire P2P attribute
 *
 * Returns: the address of the specific WPS attribute found, or NULL
 */
u8 *rtw_get_p2p_attr(u8 *p2p_ie, uint p2p_ielen, u8 target_attr_id , u8 *buf_attr, u32 *len_attr)
{
	u8 *attr_ptr = NULL;
	u8 *target_attr_ptr = NULL;
	u8 p2p_oui[4] = {0x50, 0x6F, 0x9A, 0x09};

	if (len_attr)
		*len_attr = 0;

	if (!p2p_ie
	    || p2p_ielen <= 6
	    || (p2p_ie[0] != WLAN_EID_VENDOR_SPECIFIC)
	    || (_rtw_memcmp(p2p_ie + 2, p2p_oui, 4) != _TRUE))
		return attr_ptr;

	/* 6 = 1(Element ID) + 1(Length) + 3 (OUI) + 1(OUI Type) */
	attr_ptr = p2p_ie + 6; /* goto first attr */

	while ((attr_ptr - p2p_ie + 3) <= p2p_ielen) {
		/* 3 = 1(Attribute ID) + 2(Length) */
		u8 attr_id = *attr_ptr;
		u16 attr_data_len = RTW_GET_LE16(attr_ptr + 1);
		u16 attr_len = attr_data_len + 3;

		if (0)
			RTW_INFO("%s attr_ptr:%p, id:%u, length:%u\n", __func__, attr_ptr, attr_id, attr_data_len);

		if ((attr_ptr - p2p_ie + attr_len) > p2p_ielen)
			break;

		if (attr_id == target_attr_id) {
			target_attr_ptr = attr_ptr;

			if (buf_attr)
				_rtw_memcpy(buf_attr, attr_ptr, attr_len);

			if (len_attr)
				*len_attr = attr_len;

			break;
		} else
			attr_ptr += attr_len;
	}

	return target_attr_ptr;
}

/**
 * rtw_get_p2p_attr_content - Search a specific P2P attribute content from a given P2P IE
 * @p2p_ie: Address of P2P IE to search
 * @p2p_ielen: Length limit from p2p_ie
 * @target_attr_id: The attribute ID of P2P attribute to search
 * @buf_content: If not NULL and the P2P attribute is found, P2P attribute content will be copied to the buf starting from buf_content
 *               If len_content is NULL, only copy one byte.
 * @len_content: If not NULL and the P2P attribute is found, will set to the length of the P2P attribute content
 *
 * Returns: the address of the specific P2P attribute content found, or NULL
 */
u8 *rtw_get_p2p_attr_content(u8 *p2p_ie, uint p2p_ielen, u8 target_attr_id , u8 *buf_content, uint *len_content)
{
	u8 *attr_ptr;
	u32 attr_len;

	attr_ptr = rtw_get_p2p_attr(p2p_ie, p2p_ielen, target_attr_id, NULL, &attr_len);

	if (attr_ptr && attr_len) {
		if (len_content) {
			if ((buf_content && (*len_content > (attr_len - 3))) || !buf_content)
				*len_content = attr_len - 3;
		}

		if (len_content && buf_content) {
			_rtw_memcpy(buf_content, attr_ptr + 3, *len_content);
		} else if (buf_content) {
			_rtw_memcpy(buf_content, attr_ptr + 3, 1);
		}

		return attr_ptr + 3;
	}

	if (len_content)
		*len_content = 0;

	return NULL;
}

u32 rtw_set_p2p_attr_content(u8 *pbuf, u8 attr_id, u16 attr_len, u8 *pdata_attr)
{
	u32 a_len;

	*pbuf = attr_id;

	/* *(u16*)(pbuf + 1) = cpu_to_le16(attr_len); */
	RTW_PUT_LE16(pbuf + 1, attr_len);

	if (pdata_attr)
		_rtw_memcpy(pbuf + 3, pdata_attr, attr_len);

	a_len = attr_len + 3;

	return a_len;
}

uint rtw_del_p2p_ie(u8 *ies, uint ies_len_ori, const char *msg)
{
#define DBG_DEL_P2P_IE 0

	u8 *target_ie;
	u32 target_ie_len;
	uint ies_len = ies_len_ori;
	int index = 0;

	while (1) {
		target_ie = rtw_get_p2p_ie(ies, ies_len, NULL, &target_ie_len);
		if (target_ie && target_ie_len) {
			u8 *next_ie = target_ie + target_ie_len;
			uint remain_len = ies_len - (next_ie - ies);

			if (DBG_DEL_P2P_IE && msg) {
				RTW_INFO("%s %d before\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ies, ies_len);

				RTW_INFO("ies:%p, ies_len:%u\n", ies, ies_len);
				RTW_INFO("target_ie:%p, target_ie_len:%u\n", target_ie, target_ie_len);
				RTW_INFO("next_ie:%p, remain_len:%u\n", next_ie, remain_len);
			}

			_rtw_memmove(target_ie, next_ie, remain_len);
			_rtw_memset(target_ie + remain_len, 0, target_ie_len);
			ies_len -= target_ie_len;

			if (DBG_DEL_P2P_IE && msg) {
				RTW_INFO("%s %d after\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ies, ies_len);
			}

			index++;
		} else
			break;
	}

	return ies_len;
}

uint rtw_del_p2p_attr(u8 *ie, uint ielen_ori, u8 attr_id)
{
#define DBG_DEL_P2P_ATTR 0

	u8 *target_attr;
	u32 target_attr_len;
	uint ielen = ielen_ori;
	int index = 0;

	while (1) {
		target_attr = rtw_get_p2p_attr(ie, ielen, attr_id, NULL, &target_attr_len);
		if (target_attr && target_attr_len) {
			u8 *next_attr = target_attr + target_attr_len;
			uint remain_len = ielen - (next_attr - ie);

			if (DBG_DEL_P2P_ATTR) {
				RTW_INFO("%s %d before\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ie, ielen);

				RTW_INFO("ie:%p, ielen:%u\n", ie, ielen);
				RTW_INFO("target_attr:%p, target_attr_len:%u\n", target_attr, target_attr_len);
				RTW_INFO("next_attr:%p, remain_len:%u\n", next_attr, remain_len);
			}

			_rtw_memmove(target_attr, next_attr, remain_len);
			_rtw_memset(target_attr + remain_len, 0, target_attr_len);
			*(ie + 1) -= target_attr_len;
			ielen -= target_attr_len;

			if (DBG_DEL_P2P_ATTR) {
				RTW_INFO("%s %d after\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ie, ielen);
			}

			index++;
		} else
			break;
	}

	return ielen;
}

inline u8 *rtw_bss_ex_get_p2p_ie(WLAN_BSSID_EX *bss_ex, u8 *p2p_ie, uint *p2p_ielen)
{
	return rtw_get_p2p_ie(BSS_EX_TLV_IES(bss_ex), BSS_EX_TLV_IES_LEN(bss_ex), p2p_ie, p2p_ielen);
}

void rtw_bss_ex_del_p2p_ie(WLAN_BSSID_EX *bss_ex)
{
#define DBG_BSS_EX_DEL_P2P_IE 0

	u8 *ies = BSS_EX_TLV_IES(bss_ex);
	uint ies_len_ori = BSS_EX_TLV_IES_LEN(bss_ex);
	uint ies_len;

	ies_len = rtw_del_p2p_ie(ies, ies_len_ori, DBG_BSS_EX_DEL_P2P_IE ? __func__ : NULL);
	bss_ex->IELength -= ies_len_ori - ies_len;
}

void rtw_bss_ex_del_p2p_attr(WLAN_BSSID_EX *bss_ex, u8 attr_id)
{
#define DBG_BSS_EX_DEL_P2P_ATTR 0

	u8 *ies = BSS_EX_TLV_IES(bss_ex);
	uint ies_len = BSS_EX_TLV_IES_LEN(bss_ex);

	u8 *ie;
	uint ie_len, ie_len_ori;

	int index = 0;

	while (1) {
		ie = rtw_get_p2p_ie(ies, ies_len, NULL, &ie_len_ori);
		if (ie) {
			u8 *next_ie_ori = ie + ie_len_ori;
			uint remain_len = bss_ex->IELength - (next_ie_ori - bss_ex->IEs);
			u8 has_target_attr = 0;

			if (DBG_BSS_EX_DEL_P2P_ATTR) {
				if (rtw_get_p2p_attr(ie, ie_len_ori, attr_id, NULL, NULL)) {
					RTW_INFO("%s %d before\n", __func__, index);
					dump_ies(RTW_DBGDUMP, BSS_EX_TLV_IES(bss_ex), BSS_EX_TLV_IES_LEN(bss_ex));

					RTW_INFO("ies:%p, ies_len:%u\n", ies, ies_len);
					RTW_INFO("ie:%p, ie_len_ori:%u\n", ie, ie_len_ori);
					RTW_INFO("next_ie_ori:%p, remain_len:%u\n", next_ie_ori, remain_len);
					has_target_attr = 1;
				}
			}

			ie_len = rtw_del_p2p_attr(ie, ie_len_ori, attr_id);
			if (ie_len != ie_len_ori) {
				u8 *next_ie = ie + ie_len;

				_rtw_memmove(next_ie, next_ie_ori, remain_len);
				_rtw_memset(next_ie + remain_len, 0, ie_len_ori - ie_len);
				bss_ex->IELength -= ie_len_ori - ie_len;

				ies = next_ie;
			} else
				ies = next_ie_ori;

			if (DBG_BSS_EX_DEL_P2P_ATTR) {
				if (has_target_attr) {
					RTW_INFO("%s %d after\n", __func__, index);
					dump_ies(RTW_DBGDUMP, BSS_EX_TLV_IES(bss_ex), BSS_EX_TLV_IES_LEN(bss_ex));
				}
			}

			ies_len = remain_len;

			index++;
		} else
			break;
	}
}

#ifdef CONFIG_IGNORE_GO_AND_LOW_RSSI_IN_SCAN_LIST
int rtw_chk_p2p_wildcard_ssid(WLAN_BSSID_EX *bss_ex)
{
	if (_rtw_memcmp(bss_ex->Ssid.Ssid, "DIRECT-", P2P_WILDCARD_SSID_LEN))
		return _SUCCESS;
	else
		return _FAIL;
}

int rtw_chk_p2p_ie(WLAN_BSSID_EX *bss_ex)
{
	u8 *ies = BSS_EX_TLV_IES(bss_ex);
	uint ies_len = BSS_EX_TLV_IES_LEN(bss_ex);

	u8 *ie;
	uint ie_len_ori;

	ie = rtw_get_p2p_ie(ies, ies_len, NULL, &ie_len_ori);
	if(ie)
		return _SUCCESS;
	else
		return _FAIL;
}
#endif /*CONFIG_IGNORE_GO_AND_LOW_RSSI_IN_SCAN_LIST*/

void dump_wfd_ie(void *sel, const u8 *ie, u32 ie_len)
{
	const u8 *pos = ie;
	u8 id;
	u16 len;

	const u8 *wfd_ie;
	uint wfd_ielen;

	wfd_ie = rtw_get_wfd_ie(ie, ie_len, NULL, &wfd_ielen);
	if (wfd_ie != ie || wfd_ielen == 0)
		return;

	pos += 6;
	while (pos - ie + 3 <= ie_len) {
		id = *pos;
		len = RTW_GET_BE16(pos + 1);

		RTW_PRINT_SEL(sel, "%s ID:%u, LEN:%u%s\n", __func__, id, len
			, ((pos - ie + 3 + len) <= ie_len) ? "" : "(exceed ie_len)");

		pos += (3 + len);
	}
}

/**
 * rtw_get_wfd_ie - Search WFD IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @wfd_ie: If not NULL and WFD IE is found, WFD IE will be copied to the buf starting from wfd_ie
 * @wfd_ielen: If not NULL and WFD IE is found, will set to the length of the entire WFD IE
 *
 * Returns: The address of the P2P IE found, or NULL
 */
u8 *rtw_get_wfd_ie(const u8 *in_ie, int in_len, u8 *wfd_ie, uint *wfd_ielen)
{
	uint cnt;
	const u8 *wfd_ie_ptr = NULL;
	u8 eid, wfd_oui[4] = {0x50, 0x6F, 0x9A, 0x0A};

	if (wfd_ielen)
		*wfd_ielen = 0;

	if (!in_ie || in_len < 0) {
		rtw_warn_on(1);
		return (u8 *)wfd_ie_ptr;
	}

	if (in_len <= 0)
		return (u8 *)wfd_ie_ptr;

	cnt = 0;

	while (cnt + 1 + 4 < in_len) {
		eid = in_ie[cnt];

		if (cnt + 1 + 4 >= MAX_IE_SZ) {
			rtw_warn_on(1);
			return NULL;
		}

		if (eid == WLAN_EID_VENDOR_SPECIFIC && _rtw_memcmp(&in_ie[cnt + 2], wfd_oui, 4) == _TRUE) {
			wfd_ie_ptr = in_ie + cnt;

			if (wfd_ie)
				_rtw_memcpy(wfd_ie, &in_ie[cnt], in_ie[cnt + 1] + 2);

			if (wfd_ielen)
				*wfd_ielen = in_ie[cnt + 1] + 2;

			break;
		} else
			cnt += in_ie[cnt + 1] + 2;

	}

	return (u8 *)wfd_ie_ptr;
}

/**
 * rtw_get_wfd_attr - Search a specific WFD attribute from a given WFD IE
 * @wfd_ie: Address of WFD IE to search
 * @wfd_ielen: Length limit from wfd_ie
 * @target_attr_id: The attribute ID of WFD attribute to search
 * @buf_attr: If not NULL and the WFD attribute is found, WFD attribute will be copied to the buf starting from buf_attr
 * @len_attr: If not NULL and the WFD attribute is found, will set to the length of the entire WFD attribute
 *
 * Returns: the address of the specific WPS attribute found, or NULL
 */
u8 *rtw_get_wfd_attr(u8 *wfd_ie, uint wfd_ielen, u8 target_attr_id, u8 *buf_attr, u32 *len_attr)
{
	u8 *attr_ptr = NULL;
	u8 *target_attr_ptr = NULL;
	u8 wfd_oui[4] = {0x50, 0x6F, 0x9A, 0x0A};

	if (len_attr)
		*len_attr = 0;

	if (!wfd_ie
	    || wfd_ielen <= 6
	    || (wfd_ie[0] != WLAN_EID_VENDOR_SPECIFIC)
	    || (_rtw_memcmp(wfd_ie + 2, wfd_oui, 4) != _TRUE))
		return attr_ptr;

	/* 6 = 1(Element ID) + 1(Length) + 3 (OUI) + 1(OUI Type) */
	attr_ptr = wfd_ie + 6; /* goto first attr */

	while ((attr_ptr - wfd_ie + 3) <= wfd_ielen) {
		/* 3 = 1(Attribute ID) + 2(Length) */
		u8 attr_id = *attr_ptr;
		u16 attr_data_len = RTW_GET_BE16(attr_ptr + 1);
		u16 attr_len = attr_data_len + 3;

		if (0)
			RTW_INFO("%s attr_ptr:%p, id:%u, length:%u\n", __func__, attr_ptr, attr_id, attr_data_len);

		if ((attr_ptr - wfd_ie + attr_len) > wfd_ielen)
			break;

		if (attr_id == target_attr_id) {
			target_attr_ptr = attr_ptr;

			if (buf_attr)
				_rtw_memcpy(buf_attr, attr_ptr, attr_len);

			if (len_attr)
				*len_attr = attr_len;

			break;
		} else
			attr_ptr += attr_len;
	}

	return target_attr_ptr;
}

/**
 * rtw_get_wfd_attr_content - Search a specific WFD attribute content from a given WFD IE
 * @wfd_ie: Address of WFD IE to search
 * @wfd_ielen: Length limit from wfd_ie
 * @target_attr_id: The attribute ID of WFD attribute to search
 * @buf_content: If not NULL and the WFD attribute is found, WFD attribute content will be copied to the buf starting from buf_content
 * @len_content: If not NULL and the WFD attribute is found, will set to the length of the WFD attribute content
 *
 * Returns: the address of the specific WFD attribute content found, or NULL
 */
u8 *rtw_get_wfd_attr_content(u8 *wfd_ie, uint wfd_ielen, u8 target_attr_id, u8 *buf_content, uint *len_content)
{
	u8 *attr_ptr;
	u32 attr_len;

	if (len_content)
		*len_content = 0;

	attr_ptr = rtw_get_wfd_attr(wfd_ie, wfd_ielen, target_attr_id, NULL, &attr_len);

	if (attr_ptr && attr_len) {
		if (buf_content)
			_rtw_memcpy(buf_content, attr_ptr + 3, attr_len - 3);

		if (len_content)
			*len_content = attr_len - 3;

		return attr_ptr + 3;
	}

	return NULL;
}

uint rtw_del_wfd_ie(u8 *ies, uint ies_len_ori, const char *msg)
{
#define DBG_DEL_WFD_IE 0

	u8 *target_ie;
	u32 target_ie_len;
	uint ies_len = ies_len_ori;
	int index = 0;

	while (1) {
		target_ie = rtw_get_wfd_ie(ies, ies_len, NULL, &target_ie_len);
		if (target_ie && target_ie_len) {
			u8 *next_ie = target_ie + target_ie_len;
			uint remain_len = ies_len - (next_ie - ies);

			if (DBG_DEL_WFD_IE && msg) {
				RTW_INFO("%s %d before\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ies, ies_len);

				RTW_INFO("ies:%p, ies_len:%u\n", ies, ies_len);
				RTW_INFO("target_ie:%p, target_ie_len:%u\n", target_ie, target_ie_len);
				RTW_INFO("next_ie:%p, remain_len:%u\n", next_ie, remain_len);
			}

			_rtw_memmove(target_ie, next_ie, remain_len);
			_rtw_memset(target_ie + remain_len, 0, target_ie_len);
			ies_len -= target_ie_len;

			if (DBG_DEL_WFD_IE && msg) {
				RTW_INFO("%s %d after\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ies, ies_len);
			}

			index++;
		} else
			break;
	}

	return ies_len;
}

uint rtw_del_wfd_attr(u8 *ie, uint ielen_ori, u8 attr_id)
{
#define DBG_DEL_WFD_ATTR 0

	u8 *target_attr;
	u32 target_attr_len;
	uint ielen = ielen_ori;
	int index = 0;

	while (1) {
		target_attr = rtw_get_wfd_attr(ie, ielen, attr_id, NULL, &target_attr_len);
		if (target_attr && target_attr_len) {
			u8 *next_attr = target_attr + target_attr_len;
			uint remain_len = ielen - (next_attr - ie);

			if (DBG_DEL_WFD_ATTR) {
				RTW_INFO("%s %d before\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ie, ielen);

				RTW_INFO("ie:%p, ielen:%u\n", ie, ielen);
				RTW_INFO("target_attr:%p, target_attr_len:%u\n", target_attr, target_attr_len);
				RTW_INFO("next_attr:%p, remain_len:%u\n", next_attr, remain_len);
			}

			_rtw_memmove(target_attr, next_attr, remain_len);
			_rtw_memset(target_attr + remain_len, 0, target_attr_len);
			*(ie + 1) -= target_attr_len;
			ielen -= target_attr_len;

			if (DBG_DEL_WFD_ATTR) {
				RTW_INFO("%s %d after\n", __func__, index);
				dump_ies(RTW_DBGDUMP, ie, ielen);
			}

			index++;
		} else
			break;
	}

	return ielen;
}

inline u8 *rtw_bss_ex_get_wfd_ie(WLAN_BSSID_EX *bss_ex, u8 *wfd_ie, uint *wfd_ielen)
{
	return rtw_get_wfd_ie(BSS_EX_TLV_IES(bss_ex), BSS_EX_TLV_IES_LEN(bss_ex), wfd_ie, wfd_ielen);
}

void rtw_bss_ex_del_wfd_ie(WLAN_BSSID_EX *bss_ex)
{
#define DBG_BSS_EX_DEL_WFD_IE 0
	u8 *ies = BSS_EX_TLV_IES(bss_ex);
	uint ies_len_ori = BSS_EX_TLV_IES_LEN(bss_ex);
	uint ies_len;

	ies_len = rtw_del_wfd_ie(ies, ies_len_ori, DBG_BSS_EX_DEL_WFD_IE ? __func__ : NULL);
	bss_ex->IELength -= ies_len_ori - ies_len;
}

void rtw_bss_ex_del_wfd_attr(WLAN_BSSID_EX *bss_ex, u8 attr_id)
{
#define DBG_BSS_EX_DEL_WFD_ATTR 0

	u8 *ies = BSS_EX_TLV_IES(bss_ex);
	uint ies_len = BSS_EX_TLV_IES_LEN(bss_ex);

	u8 *ie;
	uint ie_len, ie_len_ori;

	int index = 0;

	while (1) {
		ie = rtw_get_wfd_ie(ies, ies_len, NULL, &ie_len_ori);
		if (ie) {
			u8 *next_ie_ori = ie + ie_len_ori;
			uint remain_len = bss_ex->IELength - (next_ie_ori - bss_ex->IEs);
			u8 has_target_attr = 0;

			if (DBG_BSS_EX_DEL_WFD_ATTR) {
				if (rtw_get_wfd_attr(ie, ie_len_ori, attr_id, NULL, NULL)) {
					RTW_INFO("%s %d before\n", __func__, index);
					dump_ies(RTW_DBGDUMP, BSS_EX_TLV_IES(bss_ex), BSS_EX_TLV_IES_LEN(bss_ex));

					RTW_INFO("ies:%p, ies_len:%u\n", ies, ies_len);
					RTW_INFO("ie:%p, ie_len_ori:%u\n", ie, ie_len_ori);
					RTW_INFO("next_ie_ori:%p, remain_len:%u\n", next_ie_ori, remain_len);
					has_target_attr = 1;
				}
			}

			ie_len = rtw_del_wfd_attr(ie, ie_len_ori, attr_id);
			if (ie_len != ie_len_ori) {
				u8 *next_ie = ie + ie_len;

				_rtw_memmove(next_ie, next_ie_ori, remain_len);
				_rtw_memset(next_ie + remain_len, 0, ie_len_ori - ie_len);
				bss_ex->IELength -= ie_len_ori - ie_len;

				ies = next_ie;
			} else
				ies = next_ie_ori;

			if (DBG_BSS_EX_DEL_WFD_ATTR) {
				if (has_target_attr) {
					RTW_INFO("%s %d after\n", __func__, index);
					dump_ies(RTW_DBGDUMP, BSS_EX_TLV_IES(bss_ex), BSS_EX_TLV_IES_LEN(bss_ex));
				}
			}

			ies_len = remain_len;

			index++;
		} else
			break;
	}
}

#ifdef CONFIG_RTW_MULTI_AP
void dump_multi_ap_ie(void *sel, const u8 *ie, u32 ie_len)
{
	const u8 *pos = ie;
	u8 id;
	u8 len;

	const u8 *multi_ap_ie;
	uint multi_ap_ielen;

	multi_ap_ie = rtw_get_ie_ex(ie, ie_len, WLAN_EID_VENDOR_SPECIFIC, MULTI_AP_OUI, 4, NULL, &multi_ap_ielen);
	if (multi_ap_ie != ie || multi_ap_ielen == 0)
		return;

	pos += 6;
	while (pos - ie + 2 <= ie_len) {
		id = *pos;
		len = *(pos + 1);

		RTW_PRINT_SEL(sel, "%s ID:%u, LEN:%u%s\n", __func__, id, len
			, ((pos - ie + 2 + len) <= ie_len) ? "" : "(exceed ie_len)");
		RTW_DUMP_SEL(sel, pos + 2, len);

		pos += (2 + len);
	}
}

/**
 * rtw_get_multi_ap_ext - Search Multi-AP IE from a series of IEs and return extension subelement value
 * @ies: Address of IEs to search
 * @ies_len: Length limit from in_ie
 *
 * Returns: The address of the target IE found, or NULL
 */
u8 rtw_get_multi_ap_ie_ext(const u8 *ies, int ies_len)
{
	u8 *ie;
	uint ielen;
	u8 val = 0;

	ie = rtw_get_ie_ex(ies, ies_len, WLAN_EID_VENDOR_SPECIFIC, MULTI_AP_OUI, 4, NULL, &ielen);
	if (ielen < 9)
		goto exit;

	if (ie[6] != MULTI_AP_SUB_ELEM_TYPE)
		goto exit;

	val = ie[8];

exit:
	return val;
}

u8 *rtw_set_multi_ap_ie_ext(u8 *pbuf, uint *frlen, u8 val)
{
	u8 cont_len = 7;

	*pbuf++ = WLAN_EID_VENDOR_SPECIFIC;
	*pbuf++ = cont_len;
	_rtw_memcpy(pbuf, MULTI_AP_OUI, 4);
	pbuf += 4;
	*pbuf++ = MULTI_AP_SUB_ELEM_TYPE;
	*pbuf++ = 1; /* len */
	*pbuf++ = val;

	if (frlen)
		*frlen = *frlen + (cont_len + 2);

	return pbuf;
}
#endif /* CONFIG_RTW_MULTI_AP */

/* Baron adds to avoid FreeBSD warning */
int ieee80211_is_empty_essid(const char *essid, int essid_len)
{
	/* Single white space is for Linksys APs */
	if (essid_len == 1 && essid[0] == ' ')
		return 1;

	/* Otherwise, if the entire essid is 0, we assume it is hidden */
	while (essid_len) {
		essid_len--;
		if (essid[essid_len] != '\0')
			return 0;
	}

	return 1;
}

int ieee80211_get_hdrlen(u16 fc)
{
	int hdrlen = 24;

	switch (WLAN_FC_GET_TYPE(fc)) {
	case RTW_IEEE80211_FTYPE_DATA:
		if (fc & RTW_IEEE80211_STYPE_QOS_DATA)
			hdrlen += 2;
		if ((fc & RTW_IEEE80211_FCTL_FROMDS) && (fc & RTW_IEEE80211_FCTL_TODS))
			hdrlen += 6; /* Addr4 */
		break;
	case RTW_IEEE80211_FTYPE_CTL:
		switch (WLAN_FC_GET_STYPE(fc)) {
		case RTW_IEEE80211_STYPE_CTS:
		case RTW_IEEE80211_STYPE_ACK:
			hdrlen = 10;
			break;
		default:
			hdrlen = 16;
			break;
		}
		break;
	}

	return hdrlen;
}

u8	rtw_ht_mcsset_to_nss(u8 *supp_mcs_set)
{
	u8 nss = 1;

	if (supp_mcs_set[3])
		nss = 4;
	else if (supp_mcs_set[2])
		nss = 3;
	else if (supp_mcs_set[1])
		nss = 2;
	else if (supp_mcs_set[0])
		nss = 1;
	else
		RTW_INFO("%s,%d, warning! supp_mcs_set is zero\n", __func__, __LINE__);
	/* RTW_INFO("%s HT: %dSS\n", __FUNCTION__, nss); */
	return nss;
}

u32	rtw_ht_mcs_set_to_bitmap(u8 *mcs_set, u8 nss)
{
	u8 i;
	u32 bitmap = 0;

	for (i = 0; i < nss; i++)
		bitmap |= mcs_set[i] << (i * 8);

	RTW_INFO("ht_mcs_set=%02x %02x %02x %02x, nss=%u, bitmap=%08x\n"
		, mcs_set[0], mcs_set[1], mcs_set[2], mcs_set[3], nss, bitmap);

	return bitmap;
}

/* show MCS rate, unit: 100Kbps */
u16 rtw_ht_mcs_rate(u8 bw_40MHz, u8 short_GI, unsigned char *MCS_rate)
{
	u16 max_rate = 0;

	if (MCS_rate[3]) {
		if (MCS_rate[3] & BIT(7))
			max_rate = (bw_40MHz) ? ((short_GI) ? 6000 : 5400) : ((short_GI) ? 2889 : 2600);
		else if (MCS_rate[3] & BIT(6))
			max_rate = (bw_40MHz) ? ((short_GI) ? 5400 : 4860) : ((short_GI) ? 2600 : 2340);
		else if (MCS_rate[3] & BIT(5))
			max_rate = (bw_40MHz) ? ((short_GI) ? 4800 : 4320) : ((short_GI) ? 2311 : 2080);
		else if (MCS_rate[3] & BIT(4))
			max_rate = (bw_40MHz) ? ((short_GI) ? 3600 : 3240) : ((short_GI) ? 1733 : 1560);
		else if (MCS_rate[3] & BIT(3))
			max_rate = (bw_40MHz) ? ((short_GI) ? 2400 : 2160) : ((short_GI) ? 1156 : 1040);
		else if (MCS_rate[3] & BIT(2))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1800 : 1620) : ((short_GI) ? 867 : 780);
		else if (MCS_rate[3] & BIT(1))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1200 : 1080) : ((short_GI) ? 578 : 520);
		else if (MCS_rate[3] & BIT(0))
			max_rate = (bw_40MHz) ? ((short_GI) ? 600 : 540) : ((short_GI) ? 289 : 260);
	} else if (MCS_rate[2]) {
		if (MCS_rate[2] & BIT(7))
			max_rate = (bw_40MHz) ? ((short_GI) ? 4500 : 4050) : ((short_GI) ? 2167 : 1950);
		else if (MCS_rate[2] & BIT(6))
			max_rate = (bw_40MHz) ? ((short_GI) ? 4050 : 3645) : ((short_GI) ? 1950 : 1750);
		else if (MCS_rate[2] & BIT(5))
			max_rate = (bw_40MHz) ? ((short_GI) ? 3600 : 3240) : ((short_GI) ? 1733 : 1560);
		else if (MCS_rate[2] & BIT(4))
			max_rate = (bw_40MHz) ? ((short_GI) ? 2700 : 2430) : ((short_GI) ? 1300 : 1170);
		else if (MCS_rate[2] & BIT(3))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1800 : 1620) : ((short_GI) ? 867 : 780);
		else if (MCS_rate[2] & BIT(2))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1350 : 1215) : ((short_GI) ? 650 : 585);
		else if (MCS_rate[2] & BIT(1))
			max_rate = (bw_40MHz) ? ((short_GI) ? 900 : 810) : ((short_GI) ? 433 : 390);
		else if (MCS_rate[2] & BIT(0))
			max_rate = (bw_40MHz) ? ((short_GI) ? 450 : 405) : ((short_GI) ? 217 : 195);
	} else if (MCS_rate[1]) {
		if (MCS_rate[1] & BIT(7))
			max_rate = (bw_40MHz) ? ((short_GI) ? 3000 : 2700) : ((short_GI) ? 1444 : 1300);
		else if (MCS_rate[1] & BIT(6))
			max_rate = (bw_40MHz) ? ((short_GI) ? 2700 : 2430) : ((short_GI) ? 1300 : 1170);
		else if (MCS_rate[1] & BIT(5))
			max_rate = (bw_40MHz) ? ((short_GI) ? 2400 : 2160) : ((short_GI) ? 1156 : 1040);
		else if (MCS_rate[1] & BIT(4))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1800 : 1620) : ((short_GI) ? 867 : 780);
		else if (MCS_rate[1] & BIT(3))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1200 : 1080) : ((short_GI) ? 578 : 520);
		else if (MCS_rate[1] & BIT(2))
			max_rate = (bw_40MHz) ? ((short_GI) ? 900 : 810) : ((short_GI) ? 433 : 390);
		else if (MCS_rate[1] & BIT(1))
			max_rate = (bw_40MHz) ? ((short_GI) ? 600 : 540) : ((short_GI) ? 289 : 260);
		else if (MCS_rate[1] & BIT(0))
			max_rate = (bw_40MHz) ? ((short_GI) ? 300 : 270) : ((short_GI) ? 144 : 130);
	} else {
		if (MCS_rate[0] & BIT(7))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1500 : 1350) : ((short_GI) ? 722 : 650);
		else if (MCS_rate[0] & BIT(6))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1350 : 1215) : ((short_GI) ? 650 : 585);
		else if (MCS_rate[0] & BIT(5))
			max_rate = (bw_40MHz) ? ((short_GI) ? 1200 : 1080) : ((short_GI) ? 578 : 520);
		else if (MCS_rate[0] & BIT(4))
			max_rate = (bw_40MHz) ? ((short_GI) ? 900 : 810) : ((short_GI) ? 433 : 390);
		else if (MCS_rate[0] & BIT(3))
			max_rate = (bw_40MHz) ? ((short_GI) ? 600 : 540) : ((short_GI) ? 289 : 260);
		else if (MCS_rate[0] & BIT(2))
			max_rate = (bw_40MHz) ? ((short_GI) ? 450 : 405) : ((short_GI) ? 217 : 195);
		else if (MCS_rate[0] & BIT(1))
			max_rate = (bw_40MHz) ? ((short_GI) ? 300 : 270) : ((short_GI) ? 144 : 130);
		else if (MCS_rate[0] & BIT(0))
			max_rate = (bw_40MHz) ? ((short_GI) ? 150 : 135) : ((short_GI) ? 72 : 65);
	}

	return max_rate;
}

int rtw_action_frame_parse(const u8 *frame, u32 frame_len, u8 *category, u8 *action)
{
	const u8 *frame_body = frame + sizeof(struct rtw_ieee80211_hdr_3addr);
	u16 fc;
	u8 c;
	u8 a = ACT_PUBLIC_MAX;

	fc = le16_to_cpu(((struct rtw_ieee80211_hdr_3addr *)frame)->frame_ctl);

	if ((fc & (RTW_IEEE80211_FCTL_FTYPE | RTW_IEEE80211_FCTL_STYPE))
	    != (RTW_IEEE80211_FTYPE_MGMT | RTW_IEEE80211_STYPE_ACTION)
	   )
		return _FALSE;

	c = frame_body[0];

	switch (c) {
	case RTW_WLAN_CATEGORY_P2P: /* vendor-specific */
		break;
	default:
		a = frame_body[1];
	}

	if (category)
		*category = c;
	if (action)
		*action = a;

	return _TRUE;
}

static const char *_action_public_str[] = {
	"ACT_PUB_BSSCOEXIST",
	"ACT_PUB_DSE_ENABLE",
	"ACT_PUB_DSE_DEENABLE",
	"ACT_PUB_DSE_REG_LOCATION",
	"ACT_PUB_EXT_CHL_SWITCH",
	"ACT_PUB_DSE_MSR_REQ",
	"ACT_PUB_DSE_MSR_RPRT",
	"ACT_PUB_MP",
	"ACT_PUB_DSE_PWR_CONSTRAINT",
	"ACT_PUB_VENDOR",
	"ACT_PUB_GAS_INITIAL_REQ",
	"ACT_PUB_GAS_INITIAL_RSP",
	"ACT_PUB_GAS_COMEBACK_REQ",
	"ACT_PUB_GAS_COMEBACK_RSP",
	"ACT_PUB_TDLS_DISCOVERY_RSP",
	"ACT_PUB_LOCATION_TRACK",
	"ACT_PUB_RSVD",
};

const char *action_public_str(u8 action)
{
	action = (action >= ACT_PUBLIC_MAX) ? ACT_PUBLIC_MAX : action;
	return _action_public_str[action];
}

#if 0
/*tmp for sta mode, root cause have to wait supplicant's update.*/
void rtw_set_spp_amsdu_mode(u8 mode, u8 *rsn_ie, int rsn_ie_len)
{
	struct rsne_info info;
	int i, ret = _SUCCESS;
	u8 spp_req_cap = 0;

	ret = rtw_rsne_info_parse(rsn_ie, rsn_ie_len, &info);
	if (ret != _SUCCESS)
		return;

	if (mode == RTW_AMSDU_MODE_NON_SPP ) {
		spp_req_cap = 0;
	} else if (mode == RTW_AMSDU_MODE_SPP) {
		spp_req_cap = SPP_CAP|SPP_REQ;
	} else if (mode == RTW_AMSDU_MODE_ALL_DROP) {
		spp_req_cap = SPP_REQ;
	} else {
		RTW_INFO("%s unexpected mode = %d, please check the config\n", __func__, mode);
		return;
	}

	SET_RSN_CAP_SPP(info.cap, spp_req_cap);
	RTW_INFO("%s set spp opt = %d\n", __func__, GET_RSN_CAP_SPP_OPT(info.cap));
}
#endif

/*	Returns:
	_TRUE	-- 	Disable AMSDU
	_FALSE	--	Enable AMSDU
*/
u8 rtw_check_amsdu_disable(u8 mode, u8 spp_opt)
{
	u8 ret = _FALSE;

	/* pp amsdu: peer's required has to be 0, or disable */
	if ((mode == RTW_AMSDU_MODE_NON_SPP) && (spp_opt & SPP_REQ))
		ret = _TRUE;
	/* spp amsdu: peer's cap has to be 1, or disable */
	else if ((mode == RTW_AMSDU_MODE_SPP) && (!(spp_opt & SPP_CAP)))
		ret = _TRUE;
	/* mode = all drop */
	else if (mode == RTW_AMSDU_MODE_ALL_DROP)
		ret = _TRUE;
	else
		ret = _FALSE;
	return ret;
}

