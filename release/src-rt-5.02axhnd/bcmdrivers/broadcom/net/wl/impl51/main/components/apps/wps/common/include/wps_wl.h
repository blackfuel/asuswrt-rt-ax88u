/*
 * WPS wireless related
 *
 * Copyright 2018 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wps_wl.h 676790 2016-12-24 17:51:50Z $
 */
#ifndef _WPS_WL_H_
#define _WPS_WL_H_

#include <typedefs.h>
#include <ethernet.h>
#include <wpa.h>
#include <wlioctl.h>

#include <wpsheaders.h>
#include <wpscommon.h>
#include <wpserror.h>
#include <info.h>

#define WPS_IE_TYPE_SET_BEACON_IE		1
#define WPS_IE_TYPE_SET_PROBE_REQUEST_IE	2
#define WPS_IE_TYPE_SET_PROBE_RESPONSE_IE	3
#define WPS_IE_TYPE_SET_ASSOC_REQUEST_IE	4
#define WPS_IE_TYPE_SET_ASSOC_RESPONSE_IE	5

#define OUITYPE_WPS				4
#define OUITYPE_PROVISION_STATIC_WEP		5

#define WPS_WLAKM_BOTH(akm) ((akm & WPA_AUTH_PSK) && (akm & WPA2_AUTH_PSK))
#define WPS_WLAKM_PSK2(akm) ((akm & WPA2_AUTH_PSK))
#define WPS_WLAKM_PSK(akm) ((akm & WPA_AUTH_PSK))
#define WPS_WLAKM_NONE(akm) (!(WPS_WLAKM_BOTH(akm) | WPS_WLAKM_PSK2(akm) | WPS_WLAKM_PSK(akm)))

#define WPS_WLENCR_BOTH(wsec) ((wsec & TKIP_ENABLED) && (wsec & AES_ENABLED))
#define WPS_WLENCR_TKIP(wsec) (wsec & TKIP_ENABLED)
#define WPS_WLENCR_AES(wsec) (wsec & AES_ENABLED)

/*
 * implemented in wps_linux.c
 */
int wps_set_wsec(int ess_id, char *ifname, void *credential, int mode);
#ifndef WPS_ROUTER
int wps_set_wps_ie(void *bcmdev, unsigned char *p_data, int length, unsigned int cmdtype);
#endif /* !WPS_ROUTER */
int wps_ioctl(char *ifname, int cmd, void *buf, int len);

/* OS dependent EAP APIs */
int wps_set_ifname(char *ifname);
uint32 wps_eapol_send_data(char *dataBuffer, uint32 dataLen);
char* wps_eapol_parse_msg(char *msg, int msg_len, int *len);

/*
 * implemented in wps_wl.c
 */
int wps_deauthenticate(unsigned char *bssid, unsigned char *sta_mac, int reason);

int wps_wl_deauthenticate(char *wl_if, unsigned char *sta_mac, int reason);
int wps_wl_del_wps_ie(char *wl_if, unsigned int cmdtype, unsigned char ouitype);
int wps_wl_set_wps_ie(char *wl_if, unsigned char *p_data, int length, unsigned int cmdtype,
	unsigned char ouitype);
int wps_wl_open_wps_window(char *ifname);
int wps_wl_close_wps_window(char *ifname);
int wps_wl_get_maclist(char *ifname, char *buf, int count);
#ifdef BCMWPSAPSTA
int wps_wl_bss_config(char *ifname, int enabled);
#endif // endif
int wps_wl_channel(char *ifname);
extern char *ether_etoa(const unsigned char *e, char *a);

#endif /* _WPS_WL_H_ */
