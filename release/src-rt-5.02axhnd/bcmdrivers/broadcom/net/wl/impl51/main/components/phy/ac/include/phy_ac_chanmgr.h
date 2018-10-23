/*
 * ACPHY Channel Manager module interface (to other PHY modules).
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
 * $Id: phy_ac_chanmgr.h 766664 2018-08-09 08:50:20Z $
 */

#ifndef _phy_ac_chanmgr_h_
#define _phy_ac_chanmgr_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_chanmgr.h>

#define NUM_CHANS_IN_CHAN_BONDING 2
#define FCBGA_43012 0
#define WLBGA_43012 1
/* forward declaration */
typedef struct phy_ac_chanmgr_info phy_ac_chanmgr_info_t;

typedef struct phy_ac_chanmgr_data {
	/* this data is shared between chanmgr and radio */
	int		bbmult_comp;
	uint16	vlinmask2g_from_nvram;
	uint16	vlinmask5g_from_nvram;
	uint8	phyrxchain_old;
	uint8	curr_band2g;
	uint8	vlin_txidx;
	uint8	fast_adc_en;
	uint8   core_freq_mapping[PHY_CORE_MAX];
	bool	init_done;
	bool	both_txchain_rxchain_eq_1;
	bool	rxchain_hw_notset;
} phy_ac_chanmgr_data_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_chanmgr_info_t *phy_ac_chanmgr_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_chanmgr_info_t *cmn_info);
void phy_ac_chanmgr_unregister_impl(phy_ac_chanmgr_info_t *ac_info);

/* inter-module data API */
phy_ac_chanmgr_data_t *phy_ac_chanmgr_get_data(phy_ac_chanmgr_info_t *chmgri);
void phy_ac_chanmgr_set_both_txchain_rxchain(phy_ac_chanmgr_info_t *chmgri,
	uint8 rxchain, uint8 txchain);

void wlc_phy_farrow_setup_tiny(phy_info_t *pi, chanspec_t chanspec);
void wlc_phy_nvram_avvmid_read(phy_info_t *pi);
void  wlc_phy_nvram_vlin_params_read(phy_info_t *pi);
void wlc_phy_mlua_adjust_acphy(phy_info_t *pi, bool btactive);
#ifdef WL_PROXDETECT
void phy_ac_chanmgr_save_smoothing(phy_ac_chanmgr_info_t *ci, uint8 *enable, uint8 *dump_mode);
#endif /* WL_PROXDETECT */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* SMTH MACROS */
#define SMTH_DISABLE                0x0
#define SMTH_ENABLE                 0x1
#define SMTH_ENABLE_NO_NW           0x2
#define SMTH_ENABLE_NO_NW_GD        0x3
#define SMTH_ENABLE_NO_NW_GD_MTE    0x4
#define DISABLE_SIGB_AND_SMTH       0x5
#define SMTH_FOR_TXBF               0x6
#define ALTCLKPLN_ENABLE            0x0
#define ALTCLKPLN_ENABLE_ROUTER4349 0x1

/* SMTH DUMP MODE */
#define SMTH_NODUMP			0
#define SMTH_FREQDUMP			2
#define SMTH_FREQDUMP_AFTER_NW		3
#define SMTH_FREQDUMP_AFTER_GD		4
#define SMTH_FREQDUMP_AFTER_MTE		5
#define SMTH_TIMEDUMP_AFTER_IFFT	6
#define SMTH_TIMEDUMP_AFTER_WIN		7
#define SMTH_FREQDUMP_AFTER_FFT		8

#define PHYBW_20 20
#define PHYBW_40 40
#define PHYBW_80 80
#define PHYBW_160 160

#define ADC_DIV_FAST 1
#define ADC_DIV_SLOW 2

#define SIPO_DIV_FAST 12
#define SIPO_DIV_SLOW 8

#define AFE_DIV_20   8
#define AFE_DIV_40   4
#define AFE_DIV_FAST 3

#define CCK_REF_VAR_NOR 0x809c
#define CCK_REF_VAR_ACI 0xffff

#define AFE_DIV_BW(bw) ((bw == PHYBW_20) ? AFE_DIV_20 : \
	((bw == PHYBW_40) ? AFE_DIV_40 : AFE_DIV_FAST))

/* 4335C0 LP Mode definitions */
/* "NORMAL_SETTINGS" --> VCO 2.5V + B0's tuning file changes */
/* "LOW_PWR_SETTINGS_1" --> VCO 2.5V + low power settings + tuning file changes */
/* "LOW_PWR_SETTINGS_2" --> VCO 1.35V + low power settings + tuning file changes */

/* Chip related low power flags (lpflags) */
#define LPFLAGS_PHY_GLOBAL_DISABLE		(1 << 16)
#define LPFLAGS_PHY_LP_DISABLE			(1 << 17)
#define LPFLAGS_PSM_PHY_CTL			(1 << 18)

typedef struct _chan_info_radio2069 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RF0_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_dsp1;
	uint16 RFP_pll_dsp2;
	uint16 RFP_pll_dsp3;
	uint16 RFP_pll_dsp4;
	uint16 RFP_pll_dsp6;
	uint16 RFP_pll_dsp7;
	uint16 RFP_pll_dsp8;
	uint16 RFP_pll_dsp9;
	uint16 RF0_logen2g_tune;
	uint16 RFX_lna2g_tune;
	uint16 RFX_txmix2g_cfg1;
	uint16 RFX_pga2g_cfg2;
	uint16 RFX_pad2g_tune;
	uint16 RF0_logen5g_tune1;
	uint16 RF0_logen5g_tune2;
	uint16 RFX_logen5g_rccr;
	uint16 RFX_lna5g_tune;
	uint16 RFX_txmix5g_cfg1;
	uint16 RFX_pga5g_cfg2;
	uint16 RFX_pad5g_tune;
	uint16 RFP_pll_cp5;
	uint16 RF0_afediv1;
	uint16 RF0_afediv2;
	uint16 RFX_adc_cfg5;

	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069_t;

typedef struct _chan_info_radio2069revGE16 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_logen2g_tune;
	uint16 RF0_lna2g_tune;
	uint16 RF0_txmix2g_cfg1;
	uint16 RF0_pga2g_cfg2;
	uint16 RF0_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RF0_logen5g_rccr;
	uint16 RF0_lna5g_tune;
	uint16 RF0_txmix5g_cfg1;
	uint16 RF0_pga5g_cfg2;
	uint16 RF0_pad5g_tune;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069revGE16_t;

typedef struct _chan_info_radio2069revGE25 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_cfg3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_logen2g_tune;
	uint16 RF0_lna2g_tune;
	uint16 RF0_txmix2g_cfg1;
	uint16 RF0_pga2g_cfg2;
	uint16 RF0_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RF0_logen5g_rccr;
	uint16 RF0_lna5g_tune;
	uint16 RF0_txmix5g_cfg1;
	uint16 RF0_pga5g_cfg2;
	uint16 RF0_pad5g_tune;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069revGE25_t;

typedef struct _chan_info_radio2069revGE32 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_pll_xtal4;
	uint16 RFP_logen2g_tune;
	uint16 RFX_lna2g_tune;
	uint16 RFX_txmix2g_cfg1;
	uint16 RFX_pga2g_cfg2;
	uint16 RFX_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RFP_logen5g_idac1;
	uint16 RFX_lna5g_tune;
	uint16 RFX_txmix5g_cfg1;
	uint16 RFX_pga5g_cfg2;
	uint16 RFX_pad5g_tune;
} chan_info_radio2069revGE32_t;

typedef struct _chan_info_radio2069revGE25_52MHz {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	uint16 RFP_pll_vcocal5;
	uint16 RFP_pll_vcocal6;
	uint16 RFP_pll_vcocal2;
	uint16 RFP_pll_vcocal1;
	uint16 RFP_pll_vcocal11;
	uint16 RFP_pll_vcocal12;
	uint16 RFP_pll_frct2;
	uint16 RFP_pll_frct3;
	uint16 RFP_pll_vcocal10;
	uint16 RFP_pll_xtal3;
	uint16 RFP_pll_vco2;
	uint16 RFP_logen5g_cfg1;
	uint16 RFP_pll_vco8;
	uint16 RFP_pll_vco6;
	uint16 RFP_pll_vco3;
	uint16 RFP_pll_xtalldo1;
	uint16 RFP_pll_hvldo1;
	uint16 RFP_pll_hvldo2;
	uint16 RFP_pll_vco5;
	uint16 RFP_pll_vco4;
	uint16 RFP_pll_lf4;
	uint16 RFP_pll_lf5;
	uint16 RFP_pll_lf7;
	uint16 RFP_pll_lf2;
	uint16 RFP_pll_lf3;
	uint16 RFP_pll_cp4;
	uint16 RFP_pll_lf6;
	uint16 RFP_logen2g_tune;
	uint16 RF0_lna2g_tune;
	uint16 RF0_txmix2g_cfg1;
	uint16 RF0_pga2g_cfg2;
	uint16 RF0_pad2g_tune;
	uint16 RFP_logen5g_tune1;
	uint16 RFP_logen5g_tune2;
	uint16 RF0_logen5g_rccr;
	uint16 RF0_lna5g_tune;
	uint16 RF0_txmix5g_cfg1;
	uint16 RF0_pga5g_cfg2;
	uint16 RF0_pad5g_tune;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio2069revGE25_52MHz_t;

typedef struct _chan_info_rx_farrow {
#ifndef ACPHY_1X1_ONLY
	uint8 chan;            /* channel number */
	uint16 freq;           /* in Mhz */
	uint16 deltaphase_lo;
	uint16 deltaphase_hi;
	uint16 drift_period;
	uint16 farrow_ctrl;
#else /* ACPHY_1X1_ONLY */
	uint8 chan;            /* channel number */
	uint16 farrow_ctrl_20_40;
	uint32 deltaphase_20_40;
	uint16 farrow_ctrl_80;
	uint32 deltaphase_80;
#endif /* ACPHY_1X1_ONLY */
} chan_info_rx_farrow;

typedef struct _chan_info_tx_farrow {
#ifdef ACPHY_1X1_ONLY
	uint8 chan;            /* channel number */
	uint32 dac_resamp_fcw;
#else /* ACPHY_1X1_ONLY */
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint16 MuDelta_l;
	uint16 MuDelta_u;
	uint16 MuDeltaInit_l;
	uint16 MuDeltaInit_u;
#endif /* ACPHY_1X1_ONLY */
} chan_info_tx_farrow;

typedef struct {
		uint8 idx;
		uint16 val;
} sparse_array_entry_t;

extern void wlc_phy_apply_default_edthresh_acphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_chanspec_set_acphy(phy_info_t *pi, chanspec_t chanspec);
extern void phy_ac_chanmgr_set_phymode(phy_info_t *pi, chanspec_t chanspec, chanspec_t chanspec_sc,
	uint16 phymode);
extern void wlc_phy_set_lowpwr_phy_reg_rev3(phy_info_t *pi);
extern void wlc_phy_set_lowpwr_phy_reg(phy_info_t *pi);
extern void wlc_phy_populate_recipcoeffs_acphy(phy_info_t *pi);
extern void  phy_ac_chanmgr_get_chan_freq_range_80p80(phy_info_t *pi,
		chanspec_t channel, uint8 *freq);
extern uint8 phy_ac_chanmgr_get_chan_freq_range(phy_info_t *pi, chanspec_t channel,
	uint8 core_segment_mapping);
extern uint8 phy_ac_chanmgr_chan2freq_range(phy_info_t *pi, chanspec_t chanspec, uint8 channel);
extern void  phy_ac_chanmgr_get_chan_freq_range_80p80_srom12(phy_info_t *pi,
		chanspec_t chanspec, uint8 *freq);
extern uint8 phy_ac_chanmgr_get_chan_freq_range_srom12(phy_info_t *pi, chanspec_t channel);
extern uint8 phy_ac_chanmgr_chan2freq_range_srom12(phy_info_t *pi,
		chanspec_t chanspec, uint8 channel);
extern void wlc_phy_smth(phy_info_t *pi, int8 enable_smth, int8 smth_dumpmode);
extern void wlc_phy_preemption_abort_during_timing_search(phy_info_t *pi, bool enable);
extern void wlc_phy_rxcore_setstate_acphy(wlc_phy_t *pih, uint8 rxcore_bitmask, uint8 phytxchain);
extern void wlc_phy_update_rxchains(wlc_phy_t *pih, uint8 *rxcore_bitmask, uint8 *txcore_bitmask,
		uint8 rxchain, uint8 txchain);
extern void wlc_phy_restore_rxchains(wlc_phy_t *pih, uint8 enRx, uint8 enTx);
extern uint8 wlc_phy_rxcore_getstate_acphy(wlc_phy_t *pih);
extern bool wlc_phy_is_scan_chan_acphy(phy_info_t *pi);
extern void wlc_phy_resetcca_acphy(phy_info_t *pi);
extern void wlc_phy_farrow_setup_28nm_ulp(phy_info_t *pi, uint16 ulp_tx_mode,
	uint16 ulp_adc_50mhz_mode);
extern void wlc_phy_tx_farrow_setup_28nm(phy_info_t *pi, uint16 dac_rate_mode);
extern void wlc_phy_rx_farrow_setup_28nm(phy_info_t *pi, chanspec_t chanspec_sc, uint8 sc_mode);

extern void wlc_phy_rx_farrow_setup_28nm_ulp(phy_info_t *pi, uint16 ulp_adc_50mhz_mode);

extern void wlc_phy_tx_farrow_setup_28nm_ulp(phy_info_t *pi, uint16 ulp_tx_mode);
extern void wlc_phy_farrow_setup_20694(phy_info_t *pi, uint16 ulp_tx_mode,
	uint16 ulp_adc_50mhz_mode);
extern void wlc_phy_farrow_setup_20697(phy_info_t *pi, uint16 ulp_tx_mode,
	uint16 ulp_adc_50mhz_mode);
extern void wlc_phy_farrow_setup_acphy(phy_info_t *pi, chanspec_t chanspec);
extern void phy_ac_chanmgr_write_rx_farrow_tiny(phy_info_t *pi, chanspec_t chanspec,
	chanspec_t chanspec_sc, int sc_mode);
extern void wlc_phy_enable_lna_dcc_comp_20691(phy_info_t *pi, bool on);
void wlc_phy_radio20693_sel_logen_mode(phy_info_t *pi);
void wlc_phy_radio20693_sel_logen_2g_mode(phy_info_t *pi, int mode);
void wlc_phy_radio20693_sel_logen_5g_mode(phy_info_t *pi, int mode);
void wlc_phy_radio20693_afe_clkdistribtion_mode(phy_info_t *pi, int mode);
void wlc_phy_radio20693_force_dacbuf_setting(phy_info_t *pi);
extern void wlc_phy_afeclkswitch_sifs_delay(phy_info_t *pi);
extern void wlc_phy_modify_txafediv_acphy(phy_info_t *pi, uint16 afediv);
extern void chanspec_get_operating_channels(phy_info_t *pi, uint8 *ch);
extern void wlc_phy_vlin_en_acphy(phy_info_t *pi);
#ifdef WL11ULB
extern void wlc_phy_ulb_mode(phy_info_t *pi, uint8 ulb_mode);
#endif /* WL11ULB */

extern void phy_ac_chanmgr_core2core_sync_setup(phy_ac_chanmgr_info_t *chanmgri, bool enable);
extern void phy_ac_chanmgr_hwobss(phy_ac_chanmgr_info_t *chanmgri, bool enable_hwobss);

void phy_ac_chanmgr_cal_init(phy_info_t *pi, uint8 *enULB);
void phy_ac_chanmgr_cal_reset(phy_info_t *pi);

int wlc_phy_femctrl_clb_prio_2g_acphy(phy_info_t *pi, bool set, uint32 val);
int wlc_phy_femctrl_clb_prio_5g_acphy(phy_info_t *pi, bool set, uint32 val);

/* IOVAR functions */
int phy_ac_chanmgr_force_td_sfo(phy_info_t *pi, bool set, uint16 val);
int phy_ac_chanmgr_force_tdcs_160m(phy_info_t *pi, bool set, int8 val);
int phy_ac_chanmgr_iovar_get_chanup_ovrd(phy_info_t *pi, int32 *ret_val);
int phy_ac_chanmgr_iovar_set_chanup_ovrd(phy_info_t *pi, int32 set_val);

int phy_ac_chanmgr_enable_mac_aided(phy_info_t *pi, bool set, uint16 val);
int phy_ac_chanmgr_enable_mac_aided_timing(phy_info_t *pi, bool set, uint16 val);

/* to help with 3+1 mode and radar scan core */
extern int phy_ac_chanmgr_set_val_sc_chspec(phy_ac_chanmgr_info_t *chanmgri, int32 set_val);
extern int phy_ac_chanmgr_get_val_sc_chspec(phy_ac_chanmgr_info_t *chanmgri, int32 *ret_val);
extern int phy_ac_chanmgr_set_val_phymode(phy_ac_chanmgr_info_t *chanmgri, int32 set_val);
extern int phy_ac_chanmgr_get_val_phymode(phy_ac_chanmgr_info_t *chanmgri, int32 *ret_val);
extern int phy_ac_chanmgr_get_val_phy_vcore(phy_ac_chanmgr_info_t *chanmgri, int32 *ret_val);
extern bool phy_ac_chanmgr_get_val_nonbf_logen_mode(phy_ac_chanmgr_info_t *chanmgri);

/* WAR */
extern void phy_ac_chanmgr_mutx_war(wlc_phy_t *pih, bool enable);
extern void impbf_radio_ovrs_4347(phy_info_t *pi, bool ovr);
#endif /* _phy_ac_chanmgr_h_ */
