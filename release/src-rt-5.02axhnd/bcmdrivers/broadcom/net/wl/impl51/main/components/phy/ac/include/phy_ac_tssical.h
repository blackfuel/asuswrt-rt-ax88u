/*
 * ACPHY TSSI Cal module interface (to other PHY modules).
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
 * $Id: phy_ac_tssical.h 742511 2018-01-22 14:14:24Z $
 */

#ifndef _phy_ac_tssical_h_
#define _phy_ac_tssical_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_tssical.h>

#define LOOPBACK_FOR_TSSICAL 0
#define LOOPBACK_FOR_IQCAL 1

/* forward declaration */
typedef struct phy_ac_tssical_info phy_ac_tssical_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_tssical_info_t *phy_ac_tssical_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_tssical_info_t *cmn_info);
void phy_ac_tssical_unregister_impl(phy_ac_tssical_info_t *ac_info);
extern void wlc_phy_tssi_radio_setup_acphy_tiny(phy_info_t *pi, uint8 core_mask, uint8 for_iqcal);
extern void wlc_phy_tssi_radio_setup_acphy_28nm(phy_info_t *pi, uint8 for_iqcal);
extern void wlc_phy_tssi_radio_setup_acphy_20694(phy_info_t *pi, uint8 for_iqcal);
extern void wlc_phy_tssi_radio_setup_acphy_20696(phy_info_t *pi, uint8 for_iqcal);
extern void wlc_phy_tssi_radio_setup_acphy_20697(phy_info_t *pi);
extern void wlc_phy_tssi_radio_setup_acphy_20698(phy_info_t *pi, uint8 for_iqcal);
extern void wlc_phy_tssi_radio_setup_acphy_20704(phy_info_t *pi, uint8 for_iqcal);
extern int8 wlc_phy_tssivisible_thresh_acphy(phy_info_t *pi);
extern void wlc_phy_txpwrctrl_idle_tssi_meas_acphy(phy_info_t *pi);
extern void wlc_phy_tssi_phy_setup_acphy(phy_info_t *pi, uint8 for_iqcal);
extern void wlc_phy_tssi_radio_setup_acphy(phy_info_t *pi, uint8 core_mask, uint8 for_iqcal);
extern void wlc_phy_txpwrctrl_set_idle_tssi_acphy(phy_info_t *pi, int16 idle_tssi, uint8 core);
extern void phy_ac_tssi_loopback_path_setup(phy_info_t *pi, uint8 for_iqcal);
extern uint8 wlc_phy_ac_set_tssi_params_maj36(phy_info_t *pi);

#if defined(WLTEST)
extern int16 wlc_phy_test_tssi_acphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs);
extern int16 wlc_phy_test_idletssi_acphy(phy_info_t *pi, int8 ctrl_type);
#endif // endif

#ifdef WLC_TXCAL
uint16 wlc_phy_adjusted_tssi_acphy(phy_info_t *pi, uint8 core_num);
uint8 wlc_phy_set_olpc_anchor_acphy(phy_info_t *pi);
uint8 wlc_phy_apply_pwr_tssi_tble_chan_acphy(phy_info_t *pi);
uint8 wlc_phy_estpwrlut_intpol_acphy(phy_info_t *pi, uint8 channel,
	wl_txcal_power_tssi_t *pwr_tssi_lut_ch1, wl_txcal_power_tssi_t *pwr_tssi_lut_ch2,
	uint8 bphy_tbl);
uint8 wlc_phy_olpc_idx_tempsense_comp_acphy(phy_info_t *pi, uint8 *iidx, uint8 core);
void phy_ac_tssical_set_olpc_threshold(phy_info_t *pi);
void wlc_phy_populate_pwr_tssi_tble_chan_acphy(phy_info_t *pi,
	txcal_pwr_tssi_lut_t *LUT_pt, uint8 bphy_tbl);
#endif /* WLC_TXCAL */

extern void
wlc_phy_set_tssisens_lim_acphy(phy_info_t *pi, uint8 override);

void phy_ac_tssical_idle(phy_info_t *pi);
#ifdef PHYCAL_CACHING
void phy_ac_tssical_idle_save_cache(phy_ac_tssical_info_t *ti, ch_calcache_t *ctx);
#endif /* PHYCAL_CACHING */

#endif /* _phy_ac_tssical_h_ */
