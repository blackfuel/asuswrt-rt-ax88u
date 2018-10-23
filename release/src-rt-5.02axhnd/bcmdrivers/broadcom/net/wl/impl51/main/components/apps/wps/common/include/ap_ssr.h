/*
 * Broadcom WPS Set Selected Registrar
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
 * $Id: ap_ssr.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef __AP_SSR_H__
#define __AP_SSR_H__

#ifdef __cplusplus
extern "C" {
#endif // endif

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif // endif

/* WSC 2.0 */
/* Find scb type */
#define WPS_SSR_SCB_FIND_ANY		0
#define WPS_SSR_SCB_FIND_IPADDR		1
#define WPS_SSR_SCB_FIND_AUTHORIED_MAC	2
#define WPS_SSR_SCB_FIND_UUID_R		3
#define WPS_SSR_SCB_FIND_VERSION1	4

#define WPS_SSR_SCB_SEARCH_ONLY		1
#define WPS_SSR_SCB_ENTER		2

typedef struct wps_ssr_scb_s {
	unsigned char version; /* V1 */
	unsigned char version2; /* V2 */
	unsigned char scState;
	unsigned long upd_time;
	char ipaddr[16]; /* string format */
	char wps_ifname[IFNAMSIZ];
	int authorizedMacs_len;
	char authorizedMacs[SIZE_MAC_ADDR * SIZE_AUTHORIZEDMACS_NUM]; /* <= 30 B */
	char uuid_R[SIZE_UUID]; /* unique identifier for registrar = 16 B */
	unsigned short selRegCfgMethods;
	unsigned short devPwdId;
	struct wps_ssr_scb_s *next;
} WPS_SSR_SCB;

static const char wildcard_authorizedMacs[SIZE_MAC_ADDR] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/*
 * UPnP command between WPS and WFA device
 */

/* Functions */
uint32 ap_ssr_init();
uint32 ap_ssr_deinit();
int ap_ssr_set_scb(char *ipaddr, CTlvSsrIE *ssrmsg, char *wps_ifname, unsigned long upd_time);
int ap_ssr_get_scb_num();
CTlvSsrIE *ap_ssr_get_scb_info(char *ipaddr, CTlvSsrIE *ssrmsg);
CTlvSsrIE *ap_ssr_get_recent_scb_info(CTlvSsrIE *ssrmsg);

int ap_ssr_get_authorizedMacs(char *authorizedMacs_buf);
int ap_ssr_get_union_attributes(CTlvSsrIE *ssrmsg, char *authorizedMacs_buf,
	int *authorizedMacs_len);
char *ap_ssr_get_ipaddr(char* uuid_r, char *enroll_mac);
char *ap_ssr_get_wps_ifname(char *ipaddr, char *wps_ifname);
void ap_ssr_free_all_scb();

#ifdef _TUDEBUGTRACE
void ap_ssr_dump_all_scb(char *title);
#endif // endif

#ifdef __cplusplus
}
#endif // endif

#endif	/* __AP_SSR_H__ */
