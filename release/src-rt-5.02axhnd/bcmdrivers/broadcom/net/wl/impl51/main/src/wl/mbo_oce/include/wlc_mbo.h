/*
 * MBO declarations/definitions for
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
 * WFA Multiband Operation (MBO) certification program certify features that facilitate
 * efficient use of multiple frequency bands/channels available to Access Points(APs)
 * and wireless devices(STAs) that may associates with them. The prerequisites of the
 * program is that AP and STAs have information which can help in making most effective
 * selection of the spectrum band in which the STA and AP should be communicating.
 * AP and STAs enable each other to make intelligent decisions collectively for more
 * efficient use of the available spectrum by exchanging this information.
 */

#ifndef _wlc_mbo_h_
#define _wlc_mbo_h_

#include <wlc_types.h>
typedef struct wlc_mbo_chan_pref {
	uint8 opclass;
	uint8 chan;
	uint8 pref;
	uint8 reason;
} wlc_mbo_chan_pref_t;

#ifdef MBO_STA
#define WL_MBO_CNT_INR(_m, _ctr) (++((_m)->cntrs->_ctr))
#define MBO_MAX_CHAN_PREF_ENTRIES 16

/* flags to mark MBO ap capability */
#define MBO_FLAG_AP_CELLULAR_AWARE  0x1
/* flag to association attempt even AP is not accepting connection */
#define MBO_FLAG_FORCE_ASSOC_TO_AP  0x2
/* flag to forcefully reject bss transition request from AP */
#define MBO_FLAG_FORCE_BSSTRANS_REJECT  0x4

wlc_mbo_info_t * wlc_mbo_attach(wlc_info_t *wlc);
void wlc_mbo_detach(wlc_mbo_info_t *mbo);
int
wlc_mbo_add_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref);
int
wlc_mbo_del_chan_pref(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	wlc_mbo_chan_pref_t *ch_pref);
int
wlc_mbo_set_cellular_data_cap(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 cell_data_cap);
int
wlc_mbo_get_cellular_data_cap(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *cell_data_cap);
#ifdef WL_MBO_WFA_CERT
int
wlc_mbo_set_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 enable, uint8 reason);
int
wlc_mbo_get_bsstrans_reject(wlc_mbo_info_t *mbo, wlc_bsscfg_t *bsscfg,
	uint8 *enable, uint8 *reason);
#endif /* WL_MBO_WFA_CERT */
#endif /* MBO_STA */

#ifdef MBO_AP
wlc_mbo_info_t * wlc_mbo_ap_attach(wlc_info_t *wlc);
void wlc_mbo_ap_detach(wlc_mbo_info_t *mbo);
#ifdef WL_MBO_OCE
wlc_mbo_oce_info_t *wlc_init_mbo_oce(wlc_info_t* wlc);
#endif /* WL_MBO_OCE */
int wlc_mbo_process_wnm_notif(wlc_info_t *wlc, struct scb *scb, uint8 *body, int body_len);
void wlc_mbo_update_scb_band_cap(wlc_info_t* wlc, struct scb* scb, uint8* data);
int wlc_mbo_process_bsstrans_resp(wlc_info_t* wlc, struct scb* scb, uint8* body, int body_len);
void wlc_mbo_add_mbo_ie_bsstrans_req(wlc_info_t* wlc, uint8* data, bool assoc_retry_attr,
	uint8 retry_delay, uint8 transition_reason);
int wlc_mbo_calc_len_mbo_ie_bsstrans_req(uint8 reqmode, bool* assoc_retry_attr);
bool wlc_mbo_reject_assoc_req(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
bool wlc_mbo_is_channel_non_preferred(wlc_info_t* wlc, struct scb* scb, uint8 channel,
	uint8 opclass);
int32 wlc_mbo_get_gas_support(wlc_info_t* wlc);
extern void wlc_mbo_update_gasi(wlc_info_t* wlc, void* gasi);
#endif /* MBO_AP */
#endif	/* _wlc_mbo_h_ */
