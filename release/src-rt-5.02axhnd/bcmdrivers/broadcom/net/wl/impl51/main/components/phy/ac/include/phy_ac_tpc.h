/*
 * ACPHY TxPowerCtrl module interface (to other PHY modules).
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
 * $Id: phy_ac_tpc.h 714981 2017-08-08 23:19:37Z $
 */

#ifndef _phy_ac_tpc_h_
#define _phy_ac_tpc_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_tpc.h>

/* forward declaration */
typedef struct phy_ac_tpc_info phy_ac_tpc_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_tpc_info_t *phy_ac_tpc_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_tpc_info_t *ti);
void phy_ac_tpc_unregister_impl(phy_ac_tpc_info_t *info);

#define TSSI_DIVWAR_INDX (2)

void chanspec_setup_tpc(phy_info_t *pi);
extern uint8 wlc_phy_tssi2dbm_acphy(phy_info_t *pi, int32 tssi, int32 a1, int32 b0, int32 b1);
#if defined(WLTEST) || defined(ATE_BUILD)
extern void wlc_phy_tone_pwrctrl_loop(phy_info_t *pi, int8 targetpwr_dBm);
#endif // endif
extern void wlc_phy_get_paparams_for_band_acphy(phy_info_t *pi, int16 *a1, int16 *b0, int16 *b1);
extern void wlc_phy_read_txgain_acphy(phy_info_t *pi);
extern void wlc_phy_txpwr_by_index_acphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex);
extern void wlc_phy_get_txgain_settings_by_index_acphy(phy_info_t *pi,
	txgain_setting_t *txgain_settings, int8 txpwrindex);
extern void wlc_phy_get_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core);
extern void wlc_phy_set_tx_bbmult_acphy(phy_info_t *pi, uint16 *bb_mult, uint16 core);
extern uint32 wlc_phy_txpwr_idx_get_acphy(phy_info_t *pi);
extern void wlc_phy_txpwrctrl_enable_acphy(phy_info_t *pi, uint8 ctrl_type);
extern void wlc_phy_txpwr_fixpower_acphy(phy_info_t *pi);
extern void wlc_phy_txpwr_est_pwr_acphy(phy_info_t *pi, uint8 *Pout, uint8 *Pout_adj);
extern int8 wlc_phy_tone_pwrctrl(phy_info_t *pi, int8 tx_idx, uint8 core);

extern void wlc_phy_txpwrctrl_set_target_acphy(phy_info_t *pi, uint8 pwr_qtrdbm, uint8 core);
extern void wlc_phy_txpwrctrl_config_acphy(phy_info_t *pi);

#if defined(WLTEST)
void wlc_phy_iovar_patrim_acphy(phy_info_t *pi, int32 *ret_int_ptr);
void wlc_phy_txpwr_ovrinitbaseidx(phy_info_t *pi);
#endif // endif
int8 wlc_phy_txpwrctrl_update_minpwr_acphy(phy_info_t *pi);
void wlc_phy_txpwrctrl_set_baseindex(phy_info_t *pi, uint8 core, uint8 baseindex, bool frame_type);
#if (defined(WLOLPC) && !defined(WLOLPC_DISABLED)) || defined(BCMDBG) || \
	defined(WLTEST)
void chanspec_clr_olpc_dbg_mode(phy_ac_tpc_info_t *info);
#endif /* ((WLOLPC) && !(WLOLPC_DISABLED)) || (BCMDBG) || (WLTEST) */

#ifdef WLC_TXCAL
extern void wlc_phy_txcal_olpc_idx_recal_acphy(phy_info_t *pi, bool compute_idx);
#endif	/* WLC_TXCAL */
#ifdef PHYCAL_CACHING
void phy_ac_tpc_save_cache(phy_ac_tpc_info_t *ti, ch_calcache_t *ctx);
#endif /* PHYCAL_CACHING */
extern int8
wlc_phy_calc_ppr_pwr_cap_acphy(phy_info_t *pi, uint8 core, int8 maxpwr);
extern int16 wlc_phy_calc_adjusted_cap_rgstr_acphy(phy_info_t *pi, uint8 core);
extern void phy_ac_tpc_stf_chain_get_valid(phy_ac_tpc_info_t *tpci, uint8 *txchain, uint8 *rxchain);
extern uint16 wlc_phy_set_txpwr_by_index_acphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex);
#ifdef RADIO_HEALTH_CHECK
extern int phy_ac_tpc_force_fail_baseindex(phy_ac_tpc_info_t *tpci);
#endif /* RADIO_HEALTH_CHECK */

/* define number of MCSs for NDP PWR lookup table:
 * every two are packed into one 16-bit entry
 */
#define D11_MU_NDPPWR_MAXMCS	9

#ifdef TXPWRBACKOFF
#define TXPWR_BACKOFF_V_HI 0
#define TXPWR_BACKOFF_V_LO 1
#define TXPWR_BACKOFF_T_HI 1
#define TXPWR_BACKOFF_T_LO 0

#ifdef WL_TVPM
#define TXPWR_BACKOFF_T_NO_MITIGATION 1
#define TXPWR_BACKOFF_V_NO_MITIGATION 0
#else
#define TXPWR_BACKOFF_T_NO_MITIGATION TXPWR_BACKOFF_T_LO
#define TXPWR_BACKOFF_V_NO_MITIGATION TXPWR_BACKOFF_V_HI
#endif /* WL_TVPM */

extern void wlc_phy_calc_txpwr_backoff_chanset(phy_info_t *pi);
extern bool wlc_phy_calc_txpwr_backoff(phy_info_t *pi, bool force);
extern void wlc_phy_update_txpwr_backoff(phy_info_t *pi);
extern void wlc_phy_apply_txpwr_backoff_acphy(phy_ac_tpc_info_t *tpci, ppr_t *tx_pwr_target);
extern uint8 wlc_phy_get_max_srom_txpower_band_acphy(phy_info_t *pi, chanspec_t chanspec,
	int8* p_maxpwr, uint8 core, uint8 maxcore);
#endif /* TXPWRBACKOFF */
#endif /* _phy_ac_tpc_h_ */
