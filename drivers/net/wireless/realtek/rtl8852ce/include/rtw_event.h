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
#ifndef _RTW_EVENT_H_
#define _RTW_EVENT_H_

/*
Used to report a bss has been scanned

*/
struct survey_event	{
	WLAN_BSSID_EX bss;
};

/*
Used to report that the requested site survey has been done.

bss_cnt indicates the number of bss that has been reported.


*/
struct surveydone_event {
	unsigned int	bss_cnt;
	u8 activate_ch_cnt;
	u8 reason;
	bool complete; /* scan done w/o aborting */
	bool acs; /* aim to trigger channel selection */
};

/*
Used to report the link result of joinning the given bss


join_res:
-1: authentication fail
-2: association fail
> 0: TID

*/
struct joinbss_event {
	struct	wlan_network	network;
};

/*
Used to report a given STA has joinned the created BSS.
It is used in AP/Ad-HoC(M) mode.


*/
struct stassoc_event {
	unsigned char macaddr[6];
};

struct stadel_event {
	unsigned char macaddr[6];
	unsigned char rsvd[2]; /* for reason */
	unsigned char locally_generated;
	int mac_id;
};

struct wmm_event {
	unsigned char wmm;
};


#endif /* _WLANEVENT_H_ */
