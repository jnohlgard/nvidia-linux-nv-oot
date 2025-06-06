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

#ifdef CONFIG_GPIO_WAKEUP
#include <linux/gpio.h>
#endif

#include <drv_types.h>

#if defined(RTW_ENABLE_WIFI_CONTROL_FUNC)
#include <linux/platform_device.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	#include <linux/wlan_plat.h>
#else
	#include <linux/wifi_tiwlan.h>
#endif
#endif /* defined(RTW_ENABLE_WIFI_CONTROL_FUNC) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#define strnicmp	strncasecmp
#endif /* Linux kernel >= 4.0.0 */

#ifdef CONFIG_GPIO_WAKEUP
#include <linux/interrupt.h>
#include <linux/irq.h>
#endif

#include "rtw_version.h"

extern void macstr2num(u8 *dst, u8 *src);

const char *android_wifi_cmd_str[ANDROID_WIFI_CMD_MAX] = {
	"START",
	"STOP",
	"SCAN-ACTIVE",
	"SCAN-PASSIVE",
	"RSSI",
	"LINKSPEED",
	"RXFILTER-START",
	"RXFILTER-STOP",
	"RXFILTER-ADD",
	"RXFILTER-REMOVE",
	"BTCOEXSCAN-START",
	"BTCOEXSCAN-STOP",
	"BTCOEXMODE",
	"SETSUSPENDMODE",
	"SETSUSPENDOPT",
	"P2P_DEV_ADDR",
	"SETFWPATH",
	"SETBAND",
	"GETBAND",
	"COUNTRY",
	"P2P_SET_NOA",
	"P2P_GET_NOA",
	"P2P_SET_PS",
	"SET_AP_WPS_P2P_IE",

	"MIRACAST",

#ifdef CONFIG_PNO_SUPPORT
	"PNOSSIDCLR",
	"PNOSETUP",
	"PNOFORCE",
	"PNODEBUG",
#endif

	"MACADDR",

	"BLOCK_SCAN",
	"BLOCK",
	"WFD-ENABLE",
	"WFD-DISABLE",
	"WFD-SET-TCPPORT",
	"WFD-SET-MAXTPUT",
	"WFD-SET-DEVTYPE",
	"HOSTAPD_SET_MACADDR_ACL",
	"HOSTAPD_ACL_ADD_STA",
	"HOSTAPD_ACL_REMOVE_STA",
#if defined(CONFIG_GTK_OL) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
	"GTK_REKEY_OFFLOAD",
#endif /* CONFIG_GTK_OL */
/*	Private command for	P2P disable*/
	"P2P_DISABLE",
	"SET_AEK",
	"EXT_AUTH_STATUS",
	"DRIVER_VERSION"
#ifdef PRIVATE_R
	,"ROKU_FIND_REMOTE"
#endif
};

typedef struct android_wifi_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} android_wifi_priv_cmd;

#ifdef CONFIG_COMPAT
typedef struct compat_android_wifi_priv_cmd {
	compat_uptr_t buf;
	int used_len;
	int total_len;
} compat_android_wifi_priv_cmd;
#endif /* CONFIG_COMPAT */

/**
 * Local (static) functions and variables
 */

/* Initialize g_wifi_on to 1 so dhd_bus_start will be called for the first
 * time (only) in dhd_open, subsequential wifi on will be handled by
 * wl_android_wifi_on
 */
static int g_wifi_on = _TRUE;

unsigned int oob_irq = 0;
unsigned int oob_gpio = 0;

int rtw_android_cmdstr_to_num(char *cmdstr)
{
	int cmd_num;
	for (cmd_num = 0 ; cmd_num < ANDROID_WIFI_CMD_MAX; cmd_num++)
		if (0 == strnicmp(cmdstr , android_wifi_cmd_str[cmd_num], strlen(android_wifi_cmd_str[cmd_num])))
			break;

	return cmd_num;
}

int rtw_android_get_rssi(struct net_device *net, char *command, int total_len)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(net);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct	wlan_network	*pcur_network = &pmlmepriv->dev_cur_network;
	int bytes_written = 0;

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE) {
		bytes_written += snprintf(&command[bytes_written], total_len, "%s rssi %d",
			pcur_network->network.Ssid.Ssid, padapter->recvinfo.rssi);
	}

	return bytes_written;
}

int rtw_android_get_link_speed(struct net_device *net, char *command, int total_len)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(net);
	int bytes_written = 0;
	u16 link_speed = 0;

	link_speed = rtw_get_cur_max_rate(padapter) / 10;
	bytes_written = snprintf(command, total_len, "LinkSpeed %d", link_speed);

	return bytes_written;
}

int rtw_android_get_macaddr(struct net_device *net, char *command, int total_len)
{
	int bytes_written = 0;

	bytes_written = snprintf(command, total_len, "Macaddr = "MAC_FMT, MAC_ARG(net->dev_addr));
	return bytes_written;
}

int rtw_android_set_country(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *country_code = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_COUNTRY]) + 1;
	int ret = _FAIL;

	ret = rtw_set_country(adapter, country_code, RTW_ENV_NUM, RTW_REGD_SET_BY_USER);

	return (ret == _SUCCESS) ? 0 : -1;
}

int rtw_android_get_p2p_dev_addr(struct net_device *net, char *command, int total_len)
{
	int bytes_written = 0;

	/* We use the same address as our HW MAC address */
	_rtw_memcpy(command, net->dev_addr, ETH_ALEN);

	bytes_written = ETH_ALEN;
	return bytes_written;
}

int rtw_android_set_block_scan(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *block_value = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_BLOCK_SCAN]) + 1;

#ifdef CONFIG_IOCTL_CFG80211
	adapter_wdev_data(adapter)->block_scan = (*block_value == '0') ? _FALSE : _TRUE;
#endif

	return 0;
}

int rtw_android_set_block(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *block_value = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_BLOCK]) + 1;

#ifdef CONFIG_IOCTL_CFG80211
	adapter_wdev_data(adapter)->block = (*block_value == '0') ? _FALSE : _TRUE;
#endif

	return 0;
}

int rtw_android_setband(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *arg = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_SETBAND]) + 1;
	u32 band = WIFI_FREQUENCY_BAND_AUTO;
	int ret = _FAIL;

	if (sscanf(arg, "%u", &band) >= 1)
		ret = rtw_set_band(adapter, band);

	return (ret == _SUCCESS) ? 0 : -1;
}

int rtw_android_getband(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	int bytes_written = 0;

	bytes_written = snprintf(command, total_len, "%u", adapter->setband);

	return bytes_written;
}

#ifdef CONFIG_WFD
int rtw_android_set_miracast_mode(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	struct wifi_display_info *wfd_info = &adapter->wfd_info;
	char *arg = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_MIRACAST]) + 1;
	u8 mode;
	int num;
	int ret = _FAIL;

	num = sscanf(arg, "%hhu", &mode);

	if (num < 1)
		goto exit;

	switch (mode) {
	case 1: /* soruce */
		mode = MIRACAST_SOURCE;
		break;
	case 2: /* sink */
		mode = MIRACAST_SINK;
		break;
	case 0: /* disabled */
	default:
		mode = MIRACAST_DISABLED;
		break;
	}
	wfd_info->stack_wfd_mode = mode;
	RTW_INFO("stack miracast mode: %s\n", get_miracast_mode_str(wfd_info->stack_wfd_mode));

	ret = _SUCCESS;

exit:
	return (ret == _SUCCESS) ? 0 : -1;
}
#endif /* CONFIG_WFD */

int get_int_from_command(char *pcmd)
{
	int i = 0;

	for (i = 0; i < strlen(pcmd); i++) {
		if (pcmd[i] == '=') {
			/*	Skip the '=' and space characters. */
			i += 2;
			break;
		}
	}
	return rtw_atoi(pcmd + i) ;
}

#if defined(CONFIG_GTK_OL) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
int rtw_gtk_offload(struct net_device *net, u8 *cmd_ptr)
{
	int i;
	/* u8 *cmd_ptr = priv_cmd.buf; */
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(net);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct security_priv *psecuritypriv = &(padapter->securitypriv);
	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));


	if (psta == NULL)
		RTW_INFO("%s, : Obtain Sta_info fail\n", __func__);
	else {
		/* string command length of "GTK_REKEY_OFFLOAD" */
		cmd_ptr += 18;

		_rtw_memcpy(psta->kek, cmd_ptr, RTW_KEK_LEN);
		cmd_ptr += RTW_KEK_LEN;
		/*
		printk("supplicant KEK: ");
		for(i=0;i<RTW_KEK_LEN; i++)
			printk(" %02x ", psta->kek[i]);
		printk("\n supplicant KCK: ");
		*/
		_rtw_memcpy(psta->kck, cmd_ptr, RTW_KCK_LEN);
		cmd_ptr += RTW_KCK_LEN;
		/*
		for(i=0;i<RTW_KEK_LEN; i++)
			printk(" %02x ", psta->kck[i]);
		*/
		_rtw_memcpy(psta->replay_ctr, cmd_ptr, RTW_REPLAY_CTR_LEN);
		psecuritypriv->binstallKCK_KEK = _TRUE;

		/* printk("\nREPLAY_CTR: "); */
		/* for(i=0;i<RTW_REPLAY_CTR_LEN; i++) */
		/* printk(" %02x ", psta->replay_ctr[i]); */
	}

	return _SUCCESS;
}
#endif /* CONFIG_GTK_OL */

#ifdef CONFIG_RTW_MESH_AEK
static int rtw_android_set_aek(struct net_device *ndev, char *command, int total_len)
{
#define SET_AEK_DATA_LEN (ETH_ALEN + 32)

	_adapter *adapter = (_adapter *)rtw_netdev_priv(ndev);
	u8 *addr;
	u8 *aek;
	int err = 0;

	if (total_len - strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_SET_AEK]) - 1 != SET_AEK_DATA_LEN) {
		err = -EINVAL;
		goto exit;
	}

	addr = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_SET_AEK]) + 1;
	aek = addr + ETH_ALEN;

	RTW_PRINT(FUNC_NDEV_FMT" addr="MAC_FMT"\n"
		, FUNC_NDEV_ARG(ndev), MAC_ARG(addr));
	if (0)
		RTW_PRINT(FUNC_NDEV_FMT" aek="KEY_FMT KEY_FMT"\n"
			, FUNC_NDEV_ARG(ndev), KEY_ARG(aek), KEY_ARG(aek + 16));

	if (rtw_mesh_plink_set_aek(adapter, addr, aek) != _SUCCESS)
		err = -ENOENT;

exit:
	return err;
}
#endif /* CONFIG_RTW_MESH_AEK */

int rtw_android_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd)
{
	#define PRIVATE_COMMAND_MAX_LEN		65536
	int ret = 0;
	char *command = NULL;
	int cmd_num;
	int bytes_written = 0;
	android_wifi_priv_cmd priv_cmd;
	_adapter	*padapter = (_adapter *) rtw_netdev_priv(net);
#ifdef CONFIG_WFD
	struct wifi_display_info		*pwfd_info;
#endif

	rtw_lock_suspend();

	if (!ifr->ifr_data) {
		ret = -EINVAL;
		goto exit;
	}
	if (padapter->registrypriv.mp_mode == 1) {
		ret = -EINVAL;
		goto exit;
	}
#ifdef CONFIG_COMPAT
#if (KERNEL_VERSION(4, 6, 0) > LINUX_VERSION_CODE)
	if (is_compat_task()) {
#else
	if (in_compat_syscall()) {
#endif
		/* User space is 32-bit, use compat ioctl */
		compat_android_wifi_priv_cmd compat_priv_cmd;

		if (copy_from_user(&compat_priv_cmd, ifr->ifr_data, sizeof(compat_android_wifi_priv_cmd))) {
			ret = -EFAULT;
			goto exit;
		}
		priv_cmd.buf = compat_ptr(compat_priv_cmd.buf);
		priv_cmd.used_len = compat_priv_cmd.used_len;
		priv_cmd.total_len = compat_priv_cmd.total_len;
	} else
#endif /* CONFIG_COMPAT */
		if (copy_from_user(&priv_cmd, ifr->ifr_data, sizeof(android_wifi_priv_cmd))) {
			ret = -EFAULT;
			goto exit;
		}
	if (padapter->registrypriv.mp_mode == 1) {
		ret = -EFAULT;
		goto exit;
	}
	/*RTW_INFO("%s priv_cmd.buf=%p priv_cmd.total_len=%d  priv_cmd.used_len=%d\n",__func__,priv_cmd.buf,priv_cmd.total_len,priv_cmd.used_len);*/
	if (priv_cmd.total_len > PRIVATE_COMMAND_MAX_LEN || priv_cmd.total_len < 0) {
		RTW_WARN("%s: invalid private command (%d)\n", __FUNCTION__,
			priv_cmd.total_len);
		ret = -EFAULT;
		goto exit;
	}
	
	command = rtw_zmalloc(priv_cmd.total_len+1);
	if (!command) {
		RTW_INFO("%s: failed to allocate memory\n", __FUNCTION__);
		ret = -ENOMEM;
		goto exit;
	}
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
	if (!access_ok(priv_cmd.buf, priv_cmd.total_len)) {
	#else
	if (!access_ok(VERIFY_READ, priv_cmd.buf, priv_cmd.total_len)) {
	#endif
		RTW_INFO("%s: failed to access memory\n", __FUNCTION__);
		ret = -EFAULT;
		goto exit;
	}
	if (copy_from_user(command, (void *)priv_cmd.buf, priv_cmd.total_len)) {
		ret = -EFAULT;
		goto exit;
	}
	command[priv_cmd.total_len] = '\0';
	RTW_INFO("%s: Android private cmd \"%s\" on %s\n"
		 , __FUNCTION__, command, ifr->ifr_name);

	cmd_num = rtw_android_cmdstr_to_num(command);

	switch (cmd_num) {
	case ANDROID_WIFI_CMD_START:
		/* bytes_written = wl_android_wifi_on(net); */
		goto response;
	case ANDROID_WIFI_CMD_SETFWPATH:
		goto response;
	}

	if (!g_wifi_on) {
		RTW_INFO("%s: Ignore private cmd \"%s\" - iface %s is down\n"
			 , __FUNCTION__, command, ifr->ifr_name);
		ret = 0;
		goto exit;
	}

	if (!rtw_hw_chk_wl_func(adapter_to_dvobj(padapter), WL_FUNC_MIRACAST)) {
		switch (cmd_num) {
		case ANDROID_WIFI_CMD_WFD_ENABLE:
		case ANDROID_WIFI_CMD_WFD_DISABLE:
		case ANDROID_WIFI_CMD_WFD_SET_TCPPORT:
		case ANDROID_WIFI_CMD_WFD_SET_MAX_TPUT:
		case ANDROID_WIFI_CMD_WFD_SET_DEVTYPE:
			goto response;
		}
	}

	switch (cmd_num) {

	case ANDROID_WIFI_CMD_STOP:
		/* bytes_written = wl_android_wifi_off(net); */
		break;

	case ANDROID_WIFI_CMD_RSSI:
		bytes_written = rtw_android_get_rssi(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_LINKSPEED:
		bytes_written = rtw_android_get_link_speed(net, command, priv_cmd.total_len);
		break;

	case ANDROID_WIFI_CMD_MACADDR:
		bytes_written = rtw_android_get_macaddr(net, command, priv_cmd.total_len);
		break;

	case ANDROID_WIFI_CMD_BLOCK_SCAN:
		bytes_written = rtw_android_set_block_scan(net, command, priv_cmd.total_len);
		break;

	case ANDROID_WIFI_CMD_BLOCK:
		bytes_written = rtw_android_set_block(net, command, priv_cmd.total_len);
		break;

	case ANDROID_WIFI_CMD_RXFILTER_START:
		/* bytes_written = net_os_set_packet_filter(net, 1); */
		break;
	case ANDROID_WIFI_CMD_RXFILTER_STOP:
		/* bytes_written = net_os_set_packet_filter(net, 0); */
		break;
	case ANDROID_WIFI_CMD_RXFILTER_ADD:
		/* int filter_num = *(command + strlen(CMD_RXFILTER_ADD) + 1) - '0'; */
		/* bytes_written = net_os_rxfilter_add_remove(net, TRUE, filter_num); */
		break;
	case ANDROID_WIFI_CMD_RXFILTER_REMOVE:
		/* int filter_num = *(command + strlen(CMD_RXFILTER_REMOVE) + 1) - '0'; */
		/* bytes_written = net_os_rxfilter_add_remove(net, FALSE, filter_num); */
		break;

	case ANDROID_WIFI_CMD_BTCOEXSCAN_START:
		/* TBD: BTCOEXSCAN-START */
		break;
	case ANDROID_WIFI_CMD_BTCOEXSCAN_STOP:
		/* TBD: BTCOEXSCAN-STOP */
		break;
	case ANDROID_WIFI_CMD_BTCOEXMODE:
#if 0
		uint mode = *(command + strlen(CMD_BTCOEXMODE) + 1) - '0';
		if (mode == 1)
			net_os_set_packet_filter(net, 0); /* DHCP starts */
		else
			net_os_set_packet_filter(net, 1); /* DHCP ends */
#ifdef WL_CFG80211
		bytes_written = wl_cfg80211_set_btcoex_dhcp(net, command);
#endif
#endif
		break;

	case ANDROID_WIFI_CMD_SETSUSPENDMODE:
		break;

	case ANDROID_WIFI_CMD_SETSUSPENDOPT:
		/* bytes_written = wl_android_set_suspendopt(net, command, priv_cmd.total_len); */
		break;

	case ANDROID_WIFI_CMD_SETBAND:
		bytes_written = rtw_android_setband(net, command, priv_cmd.total_len);
		break;

	case ANDROID_WIFI_CMD_GETBAND:
		bytes_written = rtw_android_getband(net, command, priv_cmd.total_len);
		break;

	case ANDROID_WIFI_CMD_COUNTRY:
		bytes_written = rtw_android_set_country(net, command, priv_cmd.total_len);
		break;

#ifdef CONFIG_PNO_SUPPORT
	case ANDROID_WIFI_CMD_PNOSSIDCLR_SET:
		break;
	case ANDROID_WIFI_CMD_PNOSETUP_SET:
		break;
	case ANDROID_WIFI_CMD_PNOENABLE_SET:
		break;
#endif

	case ANDROID_WIFI_CMD_P2P_DEV_ADDR:
		bytes_written = rtw_android_get_p2p_dev_addr(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_P2P_SET_NOA:
		/* int skip = strlen(CMD_P2P_SET_NOA) + 1; */
		/* bytes_written = wl_cfg80211_set_p2p_noa(net, command + skip, priv_cmd.total_len - skip); */
		break;
	case ANDROID_WIFI_CMD_P2P_GET_NOA:
		/* bytes_written = wl_cfg80211_get_p2p_noa(net, command, priv_cmd.total_len); */
		break;
	case ANDROID_WIFI_CMD_P2P_SET_PS:
		/* int skip = strlen(CMD_P2P_SET_PS) + 1; */
		/* bytes_written = wl_cfg80211_set_p2p_ps(net, command + skip, priv_cmd.total_len - skip); */
		break;

#ifdef CONFIG_IOCTL_CFG80211
	case ANDROID_WIFI_CMD_SET_AP_WPS_P2P_IE: {
		int skip = strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_SET_AP_WPS_P2P_IE]) + 3;
		bytes_written = rtw_cfg80211_set_mgnt_wpsp2pie(net, command + skip, priv_cmd.total_len - skip, *(command + skip - 2) - '0');

		adapter_to_dvobj(padapter)->wpas_type = RTW_WPAS_ANDROID;
		break;
	}
#endif /* CONFIG_IOCTL_CFG80211 */

#ifdef CONFIG_WFD

	case ANDROID_WIFI_CMD_MIRACAST:
		bytes_written = rtw_android_set_miracast_mode(net, command, priv_cmd.total_len);
		break;

	case ANDROID_WIFI_CMD_WFD_ENABLE: {
		/*	Commented by Albert 2012/07/24 */
		/*	We can enable the WFD function by using the following command: */
		/*	wpa_cli driver wfd-enable */

		rtw_wfd_enable(padapter, 1);
		break;
	}

	case ANDROID_WIFI_CMD_WFD_DISABLE: {
		/*	Commented by Albert 2012/07/24 */
		/*	We can disable the WFD function by using the following command: */
		/*	wpa_cli driver wfd-disable */

		rtw_wfd_enable(padapter, 0);
		break;
	}
	case ANDROID_WIFI_CMD_WFD_SET_TCPPORT: {
		/*	Commented by Albert 2012/07/24 */
		/*	We can set the tcp port number by using the following command: */
		/*	wpa_cli driver wfd-set-tcpport = 554 */

		rtw_wfd_set_ctrl_port(padapter, (u16)get_int_from_command(command));
		break;
	}
	case ANDROID_WIFI_CMD_WFD_SET_MAX_TPUT: {
		break;
	}
	case ANDROID_WIFI_CMD_WFD_SET_DEVTYPE: {
		/*	Commented by Albert 2012/08/28 */
		/*	Specify the WFD device type ( WFD source/primary sink ) */

		pwfd_info = &padapter->wfd_info;

		pwfd_info->wfd_device_type = (u8) get_int_from_command(command);
		pwfd_info->wfd_device_type &= WFD_DEVINFO_DUAL;
		break;
	}
#endif
#if CONFIG_RTW_MACADDR_ACL
	case ANDROID_WIFI_CMD_HOSTAPD_SET_MACADDR_ACL: {
		rtw_set_macaddr_acl(padapter, RTW_ACL_PERIOD_BSS, get_int_from_command(command));
		break;
	}
	case ANDROID_WIFI_CMD_HOSTAPD_ACL_ADD_STA: {
		u8 addr[ETH_ALEN] = {0x00};
		macstr2num(addr, command + strlen("HOSTAPD_ACL_ADD_STA") + 3);	/* 3 is space bar + "=" + space bar these 3 chars */
		rtw_acl_add_sta(padapter, RTW_ACL_PERIOD_BSS, addr);
		break;
	}
	case ANDROID_WIFI_CMD_HOSTAPD_ACL_REMOVE_STA: {
		u8 addr[ETH_ALEN] = {0x00};
		macstr2num(addr, command + strlen("HOSTAPD_ACL_REMOVE_STA") + 3);	/* 3 is space bar + "=" + space bar these 3 chars */
		rtw_acl_remove_sta(padapter, RTW_ACL_PERIOD_BSS, addr);
		break;
	}
#endif /* CONFIG_RTW_MACADDR_ACL */
#if defined(CONFIG_GTK_OL) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
	case ANDROID_WIFI_CMD_GTK_REKEY_OFFLOAD:
		rtw_gtk_offload(net, (u8 *)command);
		break;
#endif /* CONFIG_GTK_OL		 */
	case ANDROID_WIFI_CMD_P2P_DISABLE: {
#ifdef CONFIG_P2P
		/* This Android private command is no longer in use */
		/* rtw_p2p_enable(padapter, P2P_ROLE_DISABLE); */
#endif /* CONFIG_P2P */
		break;
	}

#ifdef CONFIG_RTW_MESH_AEK
	case ANDROID_WIFI_CMD_SET_AEK:
		bytes_written = rtw_android_set_aek(net, command, priv_cmd.total_len);
		break;
#endif
	
	case ANDROID_WIFI_CMD_EXT_AUTH_STATUS: {
		rtw_set_external_auth_status(padapter,
			command + strlen("EXT_AUTH_STATUS "),
			priv_cmd.total_len - strlen("EXT_AUTH_STATUS "));
		break;
	}
	case ANDROID_WIFI_CMD_DRIVERVERSION: {
		bytes_written = strlen(DRIVERVERSION);
		snprintf(command, bytes_written + 1, DRIVERVERSION);
		break;
	}
#ifdef PRIVATE_R
	case ANDROID_WIFI_CMD_ROKU_FIND_REMOTE: {
		struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
		struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
		u8 	num, cmdlen;
		u8	*ptr,*remote_mac_addr;
		int 	i;

		cmdlen = strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_ROKU_FIND_REMOTE]);
		num = *(command + cmdlen + 1) - '0';

		if (num != 0) {
			pwdinfo->num_of_remote = num;
			remote_mac_addr = pwdinfo->remote_mac_address;
			ptr = command + cmdlen + 3;

			for (i = 0; i < num; i++) {
				macstr2num(remote_mac_addr, ptr);
				ptr += 18; /* skip space and go to next mac addr */
				remote_mac_addr += 6;
			}

			set_find_remote_timer(pmlmeext, 1);
		}
		else
			_cancel_timer_ex(&pmlmeext->find_remote_timer);
		break;
	}
#endif
	default:
		RTW_INFO("Unknown PRIVATE command %s - ignored\n", command);
		snprintf(command, 3, "OK");
		bytes_written = strlen("OK");
	}

response:
	if (bytes_written >= 0) {
		if ((bytes_written == 0) && (priv_cmd.total_len > 0))
			command[0] = '\0';
		if (bytes_written >= priv_cmd.total_len) {
			RTW_INFO("%s: bytes_written = %d\n", __FUNCTION__, bytes_written);
			bytes_written = priv_cmd.total_len;
		} else
			bytes_written++;
		priv_cmd.used_len = bytes_written;
		if (copy_to_user((void *)priv_cmd.buf, command, bytes_written)) {
			RTW_INFO("%s: failed to copy data to user buffer\n", __FUNCTION__);
			ret = -EFAULT;
		}
	} else
		ret = bytes_written;

exit:
	rtw_unlock_suspend();
	if (command)
		rtw_mfree(command, priv_cmd.total_len + 1);

	return ret;
}


/**
 * Functions for Android WiFi card detection
 */
#if defined(RTW_ENABLE_WIFI_CONTROL_FUNC)

static int g_wifidev_registered = 0;
static struct semaphore wifi_control_sem;
static struct wifi_platform_data *wifi_control_data = NULL;
static struct resource *wifi_irqres = NULL;

static int wifi_add_dev(void);
static void wifi_del_dev(void);

int rtw_android_wifictrl_func_add(void)
{
	int ret = 0;
	sema_init(&wifi_control_sem, 0);

	ret = wifi_add_dev();
	if (ret) {
		RTW_INFO("%s: platform_driver_register failed\n", __FUNCTION__);
		return ret;
	}
	g_wifidev_registered = 1;

	/* Waiting callback after platform_driver_register is done or exit with error */
	if (down_timeout(&wifi_control_sem,  msecs_to_jiffies(1000)) != 0) {
		ret = -EINVAL;
		RTW_INFO("%s: platform_driver_register timeout\n", __FUNCTION__);
	}

	return ret;
}

void rtw_android_wifictrl_func_del(void)
{
	if (g_wifidev_registered) {
		wifi_del_dev();
		g_wifidev_registered = 0;
	}
}

void *wl_android_prealloc(int section, unsigned long size)
{
	void *alloc_ptr = NULL;
	if (wifi_control_data && wifi_control_data->mem_prealloc) {
		alloc_ptr = wifi_control_data->mem_prealloc(section, size);
		if (alloc_ptr) {
			RTW_INFO("success alloc section %d\n", section);
			if (size != 0L)
				memset(alloc_ptr, 0, size);
			return alloc_ptr;
		}
	}

	RTW_INFO("can't alloc section %d\n", section);
	return NULL;
}

int wifi_get_irq_number(unsigned long *irq_flags_ptr)
{
	if (wifi_irqres) {
		*irq_flags_ptr = wifi_irqres->flags & IRQF_TRIGGER_MASK;
		return (int)wifi_irqres->start;
	}
#ifdef CUSTOM_OOB_GPIO_NUM
	return CUSTOM_OOB_GPIO_NUM;
#else
	return -1;
#endif
}

int wifi_set_power(int on, unsigned long msec)
{
	RTW_INFO("%s = %d\n", __FUNCTION__, on);
	if (wifi_control_data && wifi_control_data->set_power)
		wifi_control_data->set_power(on);
	if (msec)
		msleep(msec);
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
int wifi_get_mac_addr(unsigned char *buf)
{
	RTW_INFO("%s\n", __FUNCTION__);
	if (!buf)
		return -EINVAL;
	if (wifi_control_data && wifi_control_data->get_mac_addr)
		return wifi_control_data->get_mac_addr(buf);
	return -EOPNOTSUPP;
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)) || defined(COMPAT_KERNEL_RELEASE)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
void *wifi_get_country_code(char *ccode, u32 flags)
#else /* Linux kernel < 3.18 */
void *wifi_get_country_code(char *ccode)
#endif /* Linux kernel < 3.18 */
{
	RTW_INFO("%s\n", __FUNCTION__);
	if (!ccode)
		return NULL;
	if (wifi_control_data && wifi_control_data->get_country_code)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		return wifi_control_data->get_country_code(ccode, flags);
#else /* Linux kernel < 3.18 */
		return wifi_control_data->get_country_code(ccode);
#endif /* Linux kernel < 3.18 */
	return NULL;
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)) */

static int wifi_set_carddetect(int on)
{
	RTW_INFO("%s = %d\n", __FUNCTION__, on);
	if (wifi_control_data && wifi_control_data->set_carddetect)
		wifi_control_data->set_carddetect(on);
	return 0;
}

static int wifi_probe(struct platform_device *pdev)
{
	struct wifi_platform_data *wifi_ctrl =
		(struct wifi_platform_data *)(pdev->dev.platform_data);
	int wifi_wake_gpio = 0;

	RTW_INFO("## %s\n", __FUNCTION__);
	wifi_irqres = platform_get_resource_byname(pdev, IORESOURCE_IRQ, "bcmdhd_wlan_irq");

	if (wifi_irqres == NULL)
		wifi_irqres = platform_get_resource_byname(pdev,
				IORESOURCE_IRQ, "bcm4329_wlan_irq");
	else
		wifi_wake_gpio = wifi_irqres->start;

#ifdef CONFIG_GPIO_WAKEUP
	RTW_INFO("%s: gpio:%d wifi_wake_gpio:%d\n", __func__,
	       (int)wifi_irqres->start, wifi_wake_gpio);

	if (wifi_wake_gpio > 0) {
		gpio_request(wifi_wake_gpio, "oob_irq");
		gpio_direction_input(wifi_wake_gpio);
		oob_irq = gpio_to_irq(wifi_wake_gpio);
		RTW_INFO("%s oob_irq:%d\n", __func__, oob_irq);
	} else if (wifi_irqres) {
		oob_irq = wifi_irqres->start;
		RTW_INFO("%s oob_irq:%d\n", __func__, oob_irq);
	}

	platform_wifi_get_oob_irq(&oob_irq);
#endif
	wifi_control_data = wifi_ctrl;

	wifi_set_power(1, 0);	/* Power On */
	wifi_set_carddetect(1);	/* CardDetect (0->1) */

	up(&wifi_control_sem);
	return 0;
}

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
extern _adapter * g_test_adapter;

static void shutdown_card(void)
{
	u32 addr;
	u8 tmp8, cnt = 0;

	if (NULL == g_test_adapter) {
		RTW_INFO("%s: padapter==NULL\n", __FUNCTION__);
		return;
	}

#ifdef CONFIG_WOWLAN
#ifdef CONFIG_GPIO_WAKEUP
	/*default wake up pin change to BT*/
	RTW_INFO("%s:default wake up pin change to BT\n", __FUNCTION__);
	/* ToDo: clear pin mux code is not ready
	rtw_hal_switch_gpio_wl_ctrl(g_test_adapter, WAKEUP_GPIO_IDX, _FALSE); */
#endif /* CONFIG_GPIO_WAKEUP */
#endif /* CONFIG_WOWLAN */

	/* Leave SDIO HCI Suspend */
	#if 0 /*GEORGIA_TODO_REDEFINE_IO*/
	addr = 0x10250086;
	rtw_write8(g_test_adapter, addr, 0);
	do {

		tmp8 = rtw_read8(g_test_adapter, addr);
		cnt++;
		RTW_INFO(FUNC_ADPT_FMT ": polling SDIO_HSUS_CTRL(0x%x)=0x%x, cnt=%d\n",
			 FUNC_ADPT_ARG(g_test_adapter), addr, tmp8, cnt);

		if (tmp8 & BIT(1))
			break;

		if (cnt >= 100) {
			RTW_INFO(FUNC_ADPT_FMT ": polling 0x%x[1]==1 FAIL!!\n",
				 FUNC_ADPT_ARG(g_test_adapter), addr);
			break;
		}

		rtw_mdelay_os(10);
	} while (1);

	/* unlock register I/O */
	rtw_write8(g_test_adapter, 0x1C, 0);

	/* enable power down function */
	/* 0x04[4] = 1 */
	/* 0x05[7] = 1 */
	addr = 0x04;
	tmp8 = rtw_read8(g_test_adapter, addr);
	tmp8 |= BIT(4);
	rtw_write8(g_test_adapter, addr, tmp8);
	RTW_INFO(FUNC_ADPT_FMT ": read after write 0x%x=0x%x\n",
		FUNC_ADPT_ARG(g_test_adapter), addr, rtw_read8(g_test_adapter, addr));

	addr = 0x05;
	tmp8 = rtw_read8(g_test_adapter, addr);
	tmp8 |= BIT(7);
	rtw_write8(g_test_adapter, addr, tmp8);
	RTW_INFO(FUNC_ADPT_FMT ": read after write 0x%x=0x%x\n",
		FUNC_ADPT_ARG(g_test_adapter), addr, rtw_read8(g_test_adapter, addr));

	/* lock register page0 0x0~0xB read/write */
	rtw_write8(g_test_adapter, 0x1C, 0x0E);
#else
	rtw_hal_sdio_leave_suspend(g_test_adapter);
#endif
	dev_set_surprise_removed(adapter_to_dvobj(g_test_adapter));
	RTW_INFO(FUNC_ADPT_FMT ": bSurpriseRemoved=%s\n",
		FUNC_ADPT_ARG(g_test_adapter),
		dev_is_surprise_removed(adapter_to_dvobj(g_test_adapter)) ? "True" : "False");
}
#endif /* RTW_SUPPORT_PLATFORM_SHUTDOWN */

static int wifi_remove(struct platform_device *pdev)
{
	struct wifi_platform_data *wifi_ctrl =
		(struct wifi_platform_data *)(pdev->dev.platform_data);

	RTW_INFO("## %s\n", __FUNCTION__);
	wifi_control_data = wifi_ctrl;

	wifi_set_power(0, 0);	/* Power Off */
	wifi_set_carddetect(0);	/* CardDetect (1->0) */

	up(&wifi_control_sem);
	return 0;
}

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
static void wifi_shutdown(struct platform_device *pdev)
{
	struct wifi_platform_data *wifi_ctrl =
		(struct wifi_platform_data *)(pdev->dev.platform_data);


	RTW_INFO("## %s\n", __FUNCTION__);

	wifi_control_data = wifi_ctrl;

	shutdown_card();
	wifi_set_power(0, 0);	/* Power Off */
	wifi_set_carddetect(0);	/* CardDetect (1->0) */
}
#endif /* RTW_SUPPORT_PLATFORM_SHUTDOWN */

static int wifi_suspend(struct platform_device *pdev, pm_message_t state)
{
	RTW_INFO("##> %s\n", __FUNCTION__);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)) && defined(OOB_INTR_ONLY)
	bcmsdh_oob_intr_set(0);
#endif
	return 0;
}

static int wifi_resume(struct platform_device *pdev)
{
	RTW_INFO("##> %s\n", __FUNCTION__);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)) && defined(OOB_INTR_ONLY)
	if (dhd_os_check_if_up(bcmsdh_get_drvdata()))
		bcmsdh_oob_intr_set(1);
#endif
	return 0;
}

/* temporarily use these two */
static struct platform_driver wifi_device = {
	.probe          = wifi_probe,
	.remove         = wifi_remove,
	.suspend        = wifi_suspend,
	.resume         = wifi_resume,
#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
	.shutdown       = wifi_shutdown,
#endif /* RTW_SUPPORT_PLATFORM_SHUTDOWN */
	.driver         = {
		.name   = "bcmdhd_wlan",
	}
};

static struct platform_driver wifi_device_legacy = {
	.probe          = wifi_probe,
	.remove         = wifi_remove,
	.suspend        = wifi_suspend,
	.resume         = wifi_resume,
	.driver         = {
		.name   = "bcm4329_wlan",
	}
};

static int wifi_add_dev(void)
{
	RTW_INFO("## Calling platform_driver_register\n");
	platform_driver_register(&wifi_device);
	platform_driver_register(&wifi_device_legacy);
	return 0;
}

static void wifi_del_dev(void)
{
	RTW_INFO("## Unregister platform_driver_register\n");
	platform_driver_unregister(&wifi_device);
	platform_driver_unregister(&wifi_device_legacy);
}
#endif /* defined(RTW_ENABLE_WIFI_CONTROL_FUNC) */


