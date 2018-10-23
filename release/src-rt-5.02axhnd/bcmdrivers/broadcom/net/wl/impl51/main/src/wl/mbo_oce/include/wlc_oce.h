/*
 * OCE declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

/**
 * WFA Optimized Connectivity Experience (OCE) certification program certifies features
 * that deliver a better overall connectivity experience by taking advantage of systemic
 * information available within planned and managed networks (e.g. hotspot, workplace,
 * operator-deployed networks). The program is intended to address issues identified by
 * operators including very long connection setup times, poor wireless local area network (WLAN)
 * connectivity, and spectrum consumed by management frames.
 */

#ifndef _wlc_oce_h_
#define _wlc_oce_h_

#include <mbo_oce.h>
#include <oce.h>

#ifdef WL_OCE_AP
/* According to WFA OCE TechSpec The RSSI min threshold value shall be
 * within the range of -60 to -90 dBm.
 */
#define OCE_ASS_REJ_DEF_RSSI_THD		(-70)
#define OCE_ASSOC_REJECT_DEF_RETRY_DELAY	30
#define OCE_ASSOC_REJECT_DEF_RSSI_DELTA		2

/* Set default values to max for Uplink and Downlink from Reduced WAN Metrics */
#define OCE_REDUCED_WAN_METR_DEF_UPLINK_CAP	15
#define OCE_REDUCED_WAN_METR_DEF_DOWNLINK_CAP	15
#endif /* WL_OCE_AP */

wlc_oce_info_t * wlc_oce_attach(wlc_info_t *wlc);
void wlc_oce_detach(wlc_oce_info_t *oce);
bool wlc_oce_send_probe(wlc_oce_info_t *oce, void *p);
uint8 wlc_oce_get_probe_defer_time(wlc_oce_info_t *oce);
void wlc_oce_flush_prb_suppress_bss_list(wlc_oce_info_t *oce);
bool wlc_oce_is_oce_environment(wlc_oce_info_t *oce);
uint8 wlc_oce_get_pref_channels(chanspec_t *chanspec_list);
bool wlc_oce_is_pref_channel(chanspec_t chanspec);
void wlc_oce_set_max_channel_time(wlc_oce_info_t *oce, uint16 time);
int wlc_oce_recv_fils(wlc_oce_info_t *oce, wlc_bsscfg_t *bsscfg,
	uint action_id, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
bool wlc_oce_is_fils_discovery(struct dot11_management_header *hdr);
int wlc_oce_parse_fils_discovery(wlc_oce_info_t *oce, wlc_d11rxhdr_t *wrxh,
	struct ether_addr *bssid, uint8 *body, uint body_len, wlc_bss_info_t *bi);
void wlc_oce_process_assoc_reject(wlc_bsscfg_t *cfg, struct scb *scb,
	uint16 fk, uint8 *body, uint body_len);
void oce_calc_join_pref(wlc_bsscfg_t *cfg, wlc_bss_info_t **bip, uint bss_cnt,
	join_pref_t *join_pref);
uint16 wlc_oce_if_valid_assoc(wlc_oce_info_t *oce, int8 *rssi, uint8 *rej_rssi_delta);
void wlc_oce_reset_rwan_statcs(wlc_bsscfg_t *cfg);
bool wlc_oce_get_trigger_rwan_roam(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
wifi_oce_cap_ind_attr_t* wlc_oce_find_cap_ind_attr(uint8 *parse, uint16 buf_len);
wifi_oce_probe_suppress_bssid_attr_t*
	wlc_oce_get_prb_suppr_bssid_attr(uint8 *parse, uint16 buf_len);
wifi_oce_probe_suppress_ssid_attr_t*
wlc_oce_get_prb_suppr_ssid_attr(uint8 *parse, uint16 buf_len);

#if defined(WL_OCE_AP) && !defined(WLMCNX)
void wlc_oce_pretbtt_fd_callback(wlc_oce_info_t *oce);
#endif // endif
#endif	/* _wlc_oce_h_ */
