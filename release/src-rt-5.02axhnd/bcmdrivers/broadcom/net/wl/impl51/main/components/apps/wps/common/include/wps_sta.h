/*
 * WPS station
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
 * $Id: wps_sta.h 749115 2018-02-27 20:25:46Z $
 */
#ifndef __WPS_STA_H__
#define __WPS_STA_H__

#define WPS_MAX_AP_SCAN_LIST_LEN 50
#define PBC_WALK_TIME 120
#include <wlioctl.h>
wps_ap_list_info_t *wps_get_ap_list(void);
wps_ap_list_info_t *create_aplist(void);
int add_wps_ie(unsigned char *p_data, int length, bool pbc, bool b_wps_version2);
int rem_wps_ie(unsigned char *p_data, int length, unsigned int pktflag);
int join_network(char* ssid, uint32 wsec);
int leave_network(void);
int wps_get_bssid(char *bssid);
int wps_get_ssid(char *ssid, int *len);
int wps_get_bands(uint *band_num, uint *active_band);
int do_wpa_psk(WpsEnrCred* credential);
int join_network_with_bssid(char* ssid, uint32 wsec, char *bssid);
int do_wps_scan(void);
char* get_wps_scan_results(void);
void wpssta_display_aplist_set(uint8 *aplist_diplay, bool set);
/* BEGIN escan patch */
int do_wps_escan(void);
char* get_wps_escan_results(void);
wps_ap_list_info_t *create_aplist_escan(void);
/* END escan patch */

#ifdef WFA_WPS_20_TESTBED
int set_wps_ie_frag_threshold(int threshold);
int set_update_partial_ie(uint8 *updie_str, unsigned int pktflag);
#endif /* WFA_WPS_20_TESTBED */

bool wps_wl_init(void *caller_ctx, void *callback);
void wps_wl_deinit();
wps_ap_list_info_t *wps_wl_surveying(bool b_pbc, bool b_v2, bool b_add_wpsie);
bool wps_wl_join(uint8 *bssid, char *ssid, uint8 wep);

int wps_escan_timeout_handler(uint32 timout_state);
uint32 wps_eap_reset_scan_result();
uint32 get_wps_escan_state(void);
int wpssta_display_aplist(wps_ap_list_info_t *ap);
/* Escan timeout value in seconds */
#define ESCAN_TIMER_INTERVAL_S 10
/* wps escan States */
typedef enum wps_escan_state_t {
	WPS_ESCAN_NOT_STARTED = 0,	/* default */
	WPS_ESCAN_INPROGRESS = 1,	/* The wps Escan is in Progress */
	WPS_ESCAN_DONE = 2,	/* The wps Escan Done */
	WPS_ESCAN_TIMEDOUT = 3
} wps_escan_state_t;
/* wps escan Timeout set variables */
typedef enum wps_escan_timeout_state_t {
	WPS_ESCAN_STARTED = 0,        /**< default */
	WPS_ESCAN_CHECK_TIMEOUT = 1    /**< The wps Escan is in Progress */
} wps_escan_timeout_state_t;

struct escan_bss {
	struct escan_bss *next;
	wl_bss_info_t bss[1];
};

#endif /* __WPS_STA_H__ */
