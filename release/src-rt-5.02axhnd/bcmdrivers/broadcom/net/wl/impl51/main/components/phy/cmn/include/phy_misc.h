/*
 * Miscellaneous module interface (to other PHY modules).
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
 * $Id: phy_misc.h 682910 2017-02-03 19:06:46Z $
 */

#ifndef _phy_misc_h_
#define _phy_misc_h_

#include <phy_api.h>
#include <phy_dbg.h>

/* WAR to address low/mid band RSSI bias for X14 after Rx gain error calibration */
#define X14_5G_LOWBAND_RSSI_OFFSET 3
#define X14_5G_MIDBAND_RSSI_OFFSET 3

/* forward declaration */
typedef struct phy_misc_info phy_misc_info_t;

/* attach/detach */
phy_misc_info_t *phy_misc_attach(phy_info_t *pi);
void phy_misc_detach(phy_misc_info_t *cmn_info);

/* up/down */
int phy_misc_init(phy_misc_info_t *cmn_info);
int phy_misc_down(phy_misc_info_t *cmn_info);

/* Inter-module interfaces and downward interfaces to PHY type specific implemenation */
#if defined(BCMDBG) || defined(WLTEST)
int wlc_phy_test_init(phy_info_t *pi, int channel, bool txpkt);
int wlc_phy_test_stop(phy_info_t *pi);
int wlc_phy_test_freq_accuracy(phy_info_t *pi, int channel);
#endif	/* defined(BCMDBG) || defined(WLTEST) */
uint32 wlc_phy_rx_iq_est(phy_info_t *pi, uint8 samples, uint8 antsel, uint8 resolution,
	uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
                                uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type);
#if defined(WLTEST) || defined(ATE_BUILD) || defined(DBG_PHY_IOV)
void wlc_phy_iovar_tx_tone(phy_info_t *pi, int32 int_val);
void wlc_phy_iovar_txlo_tone(phy_info_t *pi);
#endif // endif
int wlc_phy_iovar_get_rx_iq_est(phy_info_t *pi, int32 *ret_int_ptr, int32 int_val, int err);
int wlc_phy_iovar_set_rx_iq_est(phy_info_t *pi, void *p, int32 plen, int err);
int wlc_phy_iovar_get_rx_iq_est_sweep(phy_info_t *pi, void *p, int32 plen, void *a, int32 alen,
	struct wlc_if *wlcif, int err);
#ifdef PHY_DUMP_BINARY
int phy_misc_getlistandsize(phy_info_t *pi, phyradregs_list_t **phyreglist, uint16 *phyreglist_sz);
#endif // endif
bool wlc_phy_get_rxgainerr_phy(phy_info_t *pi, int16 *gainerr);
#endif /* _phy_misc_h_ */
