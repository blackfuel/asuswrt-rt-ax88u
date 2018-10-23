/*
 * TxPowerCtrl module internal interface (to other PHY modules).
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
 * $Id: phy_tpc.h 691388 2017-03-22 02:17:00Z $
 */

#ifndef _phy_tpc_h_
#define _phy_tpc_h_

#include <typedefs.h>
#include <phy_api.h>
#include <wlc_ppr.h>
#include <phy_dbg_api.h>

/* forward declaration */
typedef struct phy_tpc_info phy_tpc_info_t;

/* attach/detach */
phy_tpc_info_t *phy_tpc_attach(phy_info_t *pi);
void phy_tpc_detach(phy_tpc_info_t *ri);

/* ******** interface for TPC module ******** */
/* tx gain settings */
typedef struct {
	uint16 rad_gain; /* Radio gains */
	uint16 rad_gain_mi; /* Radio gains [16:31] */
	uint16 rad_gain_hi; /* Radio gains [32:47] */
	uint16 dac_gain; /* DAC attenuation */
	uint16 bbmult;   /* BBmult */
} txgain_setting_t;

#ifdef WL_SAR_SIMPLE_CONTROL
#define SAR_VAL_LENG        (8) /* Number of bit for SAR target pwr val */
#define SAR_ACTIVEFLAG_MASK (0x80)  /* Bitmask of SAR limit active flag */
#define SAR_VAL_MASK        (0x7f)  /* Bitmask of SAR limit target */
#endif /* WL_SAR_SIMPLE_CONTROL */

#ifdef POWPERCHANNL
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define PWRPERCHAN_ENAB(pi)	(pi->_powerperchan)
	#elif defined(POWPERCHANNL_DISABLED)
		#define PWRPERCHAN_ENAB(pi)	(0)
	#else
		#define PWRPERCHAN_ENAB(pi)	(pi->_powerperchan)
	#endif
#else
	#define PWRPERCHAN_ENAB(pi)	(0)
#endif /* POWPERCHANNL */

/* recalc target txpwr and apply to h/w */
void phy_tpc_recalc_tgt(phy_tpc_info_t *ti);

/* srom9 : Used by HT and N PHY */
void wlc_phy_txpwr_apply_srom9(phy_info_t *pi, uint8 band_num, chanspec_t chanspec,
	uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr);

/* %%%%%% srom accessory functions */
void wlc_phy_txpwr_srom_convert_cck(uint16 po, uint8 max_pwr, ppr_dsss_rateset_t *dsss);
void wlc_phy_txpwr_srom_convert_ofdm(uint32 po, uint8 max_pwr, ppr_ofdm_rateset_t *ofdm);
void wlc_phy_ppr_set_dsss(ppr_t* tx_srom_max_pwr, uint8 bwtype, ppr_dsss_rateset_t* pwr_offsets,
    phy_info_t *pi);
void wlc_phy_ppr_set_ofdm(ppr_t* tx_srom_max_pwr, uint8 bwtype, ppr_ofdm_rateset_t* pwr_offsets,
    phy_info_t *pi);

void wlc_phy_txpwr_srom_convert_mcs(uint32 po, uint8 max_pwr, ppr_ht_mcs_rateset_t *mcs);
void wlc_phy_txpwr_srom_convert_mcs_offset(uint32 po, uint8 offset, uint8 max_pwr,
	ppr_ht_mcs_rateset_t* mcs, int8 mcs7_15_offset);

/* CCK Pwr Index Convergence Correction */
void phy_tpc_cck_corr(phy_info_t *pi);

/* check limit */
void phy_tpc_check_limit(phy_info_t *pi);

//#ifdef PREASSOC_PWRCTRL
void phy_preassoc_pwrctrl_upd(phy_info_t *pi, chanspec_t chspec);
//#endif /* PREASSOC_PWRCTRL */

void wlc_phy_txpwr_srom11_read_ppr(phy_info_t *pi);
void wlc_phy_txpwr_srom12_read_ppr(phy_info_t *pi);
/* two range tssi */
bool phy_tpc_get_tworangetssi2g(phy_tpc_info_t *tpci);
bool phy_tpc_get_tworangetssi5g(phy_tpc_info_t *tpci);
/* pdet_range_id */
uint8 phy_tpc_get_2g_pdrange_id(phy_tpc_info_t *tpci);
uint8 phy_tpc_get_5g_pdrange_id(phy_tpc_info_t *tpci);

uint8 phy_tpc_get_band_from_channel(phy_tpc_info_t *tpci, uint channel);

#ifdef NO_PROPRIETARY_VHT_RATES
#else
void wlc_phy_txpwr_read_1024qam_ppr(phy_info_t *pi);
void wlc_phy_txpwr_srom11_ext_1024qam_convert_mcs_5g(uint32 po,
		chanspec_t chanspec, uint8 tmp_max_pwr,
		ppr_vht_mcs_rateset_t* vht);
void wlc_phy_txpwr_srom11_ext_1024qam_convert_mcs_2g(uint16 po,
		chanspec_t chanspec, uint8 tmp_max_pwr,
		ppr_vht_mcs_rateset_t* vht);
#endif // endif
void phy_tpc_ipa_upd(phy_tpc_info_t *tpci);
#if defined(WLTEST)
int phy_tpc_set_pavars(phy_tpc_info_t *tpci, void* a, void* p);
int phy_tpc_get_pavars(phy_tpc_info_t *tpci, void* a, void* p);
#endif // endif

#ifdef RADIO_HEALTH_CHECK
phy_crash_reason_t phy_radio_health_check_baseindex(phy_info_t *pi);
#endif /* RADIO_HEALTH_CHECK */

bool wlc_phy_txpwr_srom9_read(phy_info_t *pi);

void phy_tpc_get_paparams_for_band(phy_info_t *pi, int32 *a1, int32 *b0, int32 *b1);

extern int
phy_tpc_get_vt_pwrbackoff(phy_tpc_info_t *tpci, int32 *ret_int_ptr);

#endif /* _phy_tpc_h_ */
