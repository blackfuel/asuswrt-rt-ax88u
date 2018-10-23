/*
 * LCN20PHY module header file
 *
 * -----------------------------------------------------------------------------
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
 * -----------------------------------------------------------------------------
 *
 * $Id: wlc_phy_lcn20.h 661662 2016-09-27 00:14:43Z $
 */

#ifndef _wlc_phy_lcn20_h_
#define _wlc_phy_lcn20_h_

#include <typedefs.h>
#include <wlc_phy_int.h>
/* ********************************************************* */
#include "phy_api.h"
#include "phy_lcn20_ana.h"
#include "phy_lcn20_chanmgr.h"
#include "phy_lcn20_radio.h"
#include "phy_lcn20_tbl.h"
#include "phy_lcn20_tpc.h"
#include "phy_lcn20_antdiv.h"
#include "phy_lcn20_noise.h"
#include "phy_lcn20_rssi.h"
#include "phy_lcn20_cache.h"
#include "phy_lcn20_misc.h"
#include "phy_lcn20_txiqlocal.h"
#include "phy_lcn20_lpc.h"
#include "phy_lcn20_rxspur.h"
#include "phy_lcn20_tssical.h"
/* ********************************************************* */

#define LCN20_PAPD_ENABLE
#define PAPD_DEBUG
#define WL_RATESET_SZ	(WL_RATESET_SZ_DSSS + WL_RATESET_SZ_OFDM + WL_RATESET_SZ_HT_MCS)

#define LCN20PHY_OTP_RCAL_OFFSET 0x14
#define LCN20PHY_OTP_DATE_OFFSET 0x2

#define LCN20PHY_GAIN_DELTA_2G_PARAMS 3
#define LCN20PHY_GROUPS	5

/* swctrl map offsets */
#define LCN20PHY_I_WL_TX      0 /* PA1 PA0 Tx1 TX0 */
#define LCN20PHY_I_WL_RX      1 /* eLNARx1 eLNARx0 Rx1 RX0 */
#define LCN20PHY_I_WL_RX_ATTN 2 /* eLNAAttnRx1 eLNAAttnRx0 AttnRx1 AttnRx0 */
#define LCN20PHY_I_WL_RX_ATTN_MASK 0x3
#define LCN20PHY_I_WL_RX_ATTN_SHIFT 0
#define LCN20PHY_I_BT         3 /* Tx eLNARx Rx */
#define LCN20PHY_I_WL_MASK    4 /* ant(1 bit) ovr_en(1 bit) tdm(1 bit) mask(8 bits) */

#define LCN20PHY_MAX_SUPPORTED_2G_CHANNELS 14
#define LCN20PHY_NUM_RATE_OFFSETS         20

#define TEMPER_VBAT_TRIGGER_NEW_MEAS 1

typedef enum {
	LCN20PHY_TSSI_PRE_PA = 0,
	LCN20PHY_TSSI_POST_PA = 1,
	LCN20PHY_TSSI_EXT = 2,
	LCN20PHY_TSSI_EXT_POST_PAD = 3
} lcn20phy_tssi_mode_t;

typedef enum {
	LCN20PHY_PAPD_OFFSET_SRC_DEFAULT = 0,
	LCN20PHY_PAPD_OFFSET_SRC_NVRAM = 1,
	LCN20PHY_PAPD_OFFSET_SRC_IOVAR = 2
} lcn20phy_papad_offset_init_t;

typedef struct _lcn20phy_txiqcal_ladder_struct {
	uint8 percent;
	uint8 g_env;
} lcn20phy_txiqcal_ladder_t;

typedef struct _lcn20phy_rx_fam_struct {
	int8 freq;
	int32 angle;
	int32 mag;
} lcn20phy_rx_fam_t;

typedef struct _lcn20phy_rxiqcal_phyregs_struct {
	bool   is_orig;
	uint16 RFOverride0;
	uint16 RFOverrideVal0;
	uint16 rfoverride2;
	uint16 rfoverride2val;
	uint16 rfoverride3;
	uint16 rfoverride3_val;
	uint16 rfoverride4;
	uint16 rfoverride4val;
	uint16 rfoverride5;
	uint16 rfoverride5val;
	uint16 rfoverride7;
	uint16 rfoverride7val;
	uint16 rfoverride8;
	uint16 rfoverride8val;
	uint8 bbmult;
	uint8 txidx;
	uint16 SAVE_txpwrctrl_on;
	uint16 PapdEnable0;
	uint16 papr_ctrl;
	uint16 RxSdFeConfig1;
	uint16 RxSdFeConfig6;
	uint16 phyreg2dvga2;
	uint16 SAVE_Core1TxControl;
	uint16 sslpnCalibClkEnCtrl;
	uint16 DSSF_control_0;
	uint16 RxFeCtrl1;
	int16 ofdm_filt_type;
} lcn20phy_rxiqcal_phyregs_t;

typedef struct _lcn20phy_txiqlocal_phyregs_struct {
	bool   is_orig;
	uint8  bbmult;
	uint16 RxFeCtrl1;
	uint16 TxPwrCtrlCmd;
	uint16 RxSdFeConfig1;
	uint16 sslpnCalibClkEnCtrl;
	uint16 AfeCtrlOvr1Val;
	uint16 AfeCtrlOvr1;
	uint16 ClkEnCtrl;
	uint16 lpfbwlutreg3;
	uint16 RFOverride0;
	uint16 RFOverrideVal0;
	uint16 rfoverride4;
	uint16 rfoverride4val;
	uint16 TxPwrCtrlRfCtrlOvr0;
	uint16 PapdEnable;
} lcn20phy_txiqlocal_phyregs_t;

typedef struct _lcn20phy_txiqcal_radioregs_struct {
	bool   is_orig;
	uint16 iqcal_cfg1;
	uint16 tssi_iqcal_ovr1;
	uint16 auxpga_cfg1;
	uint16 iqcal_cfg3;
	uint16 adc_cfg10;
	uint16 AUX_RXPGA_ovr1;
	uint16 GPABuf_ovr1;
	uint16 pa2g_cfg1;
	uint16 rx_adc_ovr1;
	uint16 rx_top_2g_ovr1;
	uint16 tx_top_2g_ovr2;
	uint16 rx_bb_ovr1;
	uint16 rx_bb_ovr2;
	uint16 minipmu_ovr1;
} lcn20phy_txiqcal_radioregs_t;

typedef struct _lcn20phy_rxcal_rxgain_struct {
	int8 lna;
	uint8 tia;
	uint8 far;
	uint8 dvga;
} lcn20phy_rxcal_rxgain_t;

typedef struct _lcn20phy_iq_mismatch_struct {
	int32 angle;
	int32 mag;
	int32 sin_angle;
} lcn20phy_iq_mismatch_t;

#define LCN20PHY_CALBUFFER_MAX_SZ 1536

typedef struct _chan_info_20692_lcn20phy {
	uint8  chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint16 pll_vcocal18;
	uint16 pll_vcocal3;
	uint16 pll_vcocal4;
	uint16 pll_vcocal7;
	uint16 pll_vcocal8;
	uint16 pll_vcocal20;
	uint16 pll_vcocal1;
	uint16 pll_vcocal12;
	uint16 pll_vcocal13;
	uint16 pll_vcocal10;
	uint16 pll_vcocal11;
	uint16 pll_vcocal19;
	uint16 pll_vcocal6;
	uint16 pll_vcocal9;
	uint16 pll_vcocal17;
	uint16 pll_vcocal5;
	uint16 pll_vcocal15;
	uint16 pll_vcocal2;
	uint16 pll_mmd1;
	uint16 pll_mmd2;
	uint16 pll_vco2;
	uint16 pll_vco1;
	uint16 pll_lf4;
	uint16 pll_lf5;
	uint16 pll_lf2;
	uint16 pll_lf3;
	uint16 pll_cp1;
	uint16 pll_cp2;
	uint16 logen_cfg2;
	uint16 lna2g_tune;
	uint16 txmix2g_cfg5;
} chan_info_20692_lcn20phy_t;

/* papd structs, keep registers which are not part of iq cal */
typedef struct _lcn20phy_papd_radioregs_struct {
	bool   is_orig;
	uint16 wl_pa2g_cfg1;
	uint16 wl_pa2g_cfg3;
	uint16 wl_pa2g_idac1;
	uint16 wl_pa2g_incap;
	uint16 wl_rxmix2g_cfg1;
	uint16 wl_tx_top_2g_ovr2;
	uint16 wl_tx_dac_cfg5;
	uint16 wl_tia_cfg5;
	uint16 wl_adc_cfg10;
	uint16 wl_tia_cfg8;
	uint16 wl_trsw2g_cfg1;
	uint16 wl_lna2g_cfg1;
	uint16 wl_tx_logen2g_cfg1;
	uint16 wl_tx2g_cfg1;
	uint16 wl_rxrf2g_cfg1;
	uint16 wl_tia_cfg3;
	uint16 wl_pmu_op;
	uint16 wl_adc_cfg8;
	uint16 wl_adc_cfg1;
	phy_txgains_t gains;
	uint16 pa_gain;
	uint16 bbmult;
} lcn20phy_papd_radioregs_t;

typedef struct _lcn20phy_papd_phyregs_struct {
	bool   is_orig;
	uint16 AfeCtrlOvr1Val;
	uint16 AfeCtrlOvr1;
	uint16 RxFeCtrl1;
} lcn20phy_papd_phyregs_struct_t;

typedef struct _lcn20phy_rxiqcal_radioregs_struct {
	bool   is_orig;
	uint16 wl_rx_adc_ovr1;
	uint16 wl_tx_top_2g_ovr1;
	uint16 wl_tx_top_2g_ovr2;
	uint16 wl_rx_top_2g_ovr1;
	uint16 wl_rx_bb_ovr1;
	uint16 wl_minipmu_ovr1;
} lcn20phy_rxiqcal_radioregs_t;

typedef struct _lcn20phy_tempsense_vbat_radioregs_struct {
	bool	is_orig;
	uint16 wl_temp_sens_ovr1;
	uint16 wl_tempsense_cfg;
	uint16 wl_GPABuf_ovr1;
	uint16 wl_testbuf_cfg1;
	uint16 wl_AUX_RXPGA_ovr1;
	uint16 wl_auxpga_cfg1;
	uint16 wl_auxpga_vmid;
	uint16 wl_rx_adc_ovr1;
	uint16 wl_adc_cfg10;
	uint16 wl_tx_dac_cfg5;
	uint16 wl_rx_bb_ovr1;
	uint16 wl_tia_cfg7;
	uint16 wl_tia_cfg8;
	uint16 wl_minipmu_ovr1;
	uint16 wl_vbat_monitor_ovr1;
	uint16 wl_tia_cfg5;
	uint16 wl_tia_cfg6;
	uint16 wl_tia_cfg1;
	uint16 wl_pmu_op;
	uint16 wl_vbat_cfg;
} lcn20phy_tempsense_vbat_radioregs_t;

typedef struct _adc_tuning_array_20962_struct {
	uint16 gi;
	uint16 g21;
	uint16 g32;
	uint16 g43;
	uint16 r12;
	uint16 r34;
	uint16 gff1;
	uint16 gff2;
	uint16 gff3;
	uint16 gff4;
	uint16 g11;
} adc_tuning_array_20962_t;

typedef struct _lcn20phy_lna_params {
	uint8 gainrequestTRAttnOnEn;
	uint8 gainReqTrAttOnEnByCrs;
	uint8 wl_lna2g_dig_wrssi1_threshold;
	int16 trGainThresh;
	int16 gainrequestTRAttnOnOffset;
	int16 wl_gain_tbl_offset;
	int16 rssi_gain;
	int16 crs_gain_high_gain_db_40mhz;
	int16 max_gain_of_tbl;
	int16 meas_rxspur_gaintblidx;
	int16 rssi_clip_thresh_norm_9;
	int16 rssi_clip_thresh_norm_10;
	int16 rssi_clip_thresh_norm_11;
	int16 rssi_clip_thresh_aci_9;
	int16 rssi_clip_thresh_aci_10;
	int16 rssi_clip_thresh_aci_11;
	int16 clipCtrThresh;
	int8 dsss_threshold_offset[LCN20PHY_MAX_SUPPORTED_2G_CHANNELS];
} lcn20phy_lna_params_t;

typedef struct _lcn20phy_rssi_gain_params {
	uint8 elna_bypass;
	uint8 lna1_bypass;
	uint8 lna1_rout;
	uint8 lna1_gain;
	uint8 lna2_gain;
	uint8 tia_amp2_bypass;
	uint8 aci_tbl_ind;
	uint16 tia_R1_val;
	uint16 tia_R2_val;
	uint16 tia_R3_val;
	uint8 dvga1_val;
} lcn20phy_rssi_gain_params_t;

#ifdef WL_PROXDETECT
typedef struct proxd_loopback_gain {
	uint8 slna_byp;
	uint8 slna_rout;
	uint8 slna_gain;
	uint8 lna2_gain;
	uint8 tia;
	uint8 dvga1_gain;
	uint8 dvga2_gain;
} proxd_loopback_gain_t;
#endif /* WL_PROXDETECT */

struct phy_info_lcn20phy {
/* ********************************************************* */
	phy_info_t *pi;
	phy_lcn20_ana_info_t *anai;
	phy_lcn20_radio_info_t *radioi;
	phy_lcn20_tbl_info_t *tbli;
	phy_lcn20_tpc_info_t *tpci;
	phy_lcn20_antdiv_info_t *antdivi;
	phy_lcn20_noise_info_t *noisei;
	phy_lcn20_rssi_info_t *rssii;
	phy_lcn20_cache_info_t *cachei;
	phy_lcn20_misc_info_t *misci;
	phy_lcn20_chanmgr_info_t *chanmgri;
	phy_lcn20_txiqlocal_info_t *txiqlocali;
	phy_lcn20_lpc_info_t *lpci;
	phy_lcn20_rxspur_info_t *rxspuri;
	phy_lcn20_tssical_info_t	*tssicali;
/* ********************************************************* */
	bool	calbuffer_inuse;
	uint8	*calbuffer;
	uint16	tssi_idx;	/* Estimated index for target power */
	uint16	cck_tssi_idx;
	uint16	init_ccktxpwrindex;
	uint8	init_txpwrindex;
	uint16	tssi_npt;	/* NPT for TSSI averaging */
	uint16	tssi_tx_cnt; /* Tx frames at that level for NPT calculations */
	uint8	tssi_floor;
	int32	tssi_maxpwr_limit;
	int32	tssi_minpwr_limit;
	int8	tx_power_idx_override; /* Forced tx power index */
	uint8	current_index;
	uint8	txpwr_clamp_dis;
	uint8	txpwr_tssifloor_clamp_dis;
	uint8	txpwr_tssioffset_clamp_dis;
	int32	target_pwr_cck_max;
	int32	target_pwr_ofdm_max;
	int8	offset_targetpwr;
	int16	tempsenseCorr;
	int16	idletssi_corr;
	uint32	*rate_table;
	uint16 idletssi0_cache;
	uint8	auxpga_gain;
	uint8	auxpga_vmid;
	uint8	iqcal_auxpga_gain;
	uint8	iqcal_auxpga_vmid;
	/* LDPC related parameter used to change the ldpc_support register bit */
	bool lcn20_rxldpc_override;
	/* flags */
	bool    ePA;
	uint32	tssical_time;
	bool	uses_rate_offset_table;
	bool	btc_clamp;
	lcn20phy_rxiqcal_phyregs_t *rxiqcal_phyregs;
	lcn20phy_rxiqcal_radioregs_t *rxiqcal_radioregs;
/* #ifdef LCN20_PAPD_ENABLE */
	lcn20phy_papd_radioregs_t *papd_radioregs;
	lcn20phy_papd_phyregs_struct_t *papd_phyregs;
	int16	epsdelta2g;
	bool	papd_enable;
	bool	papd_mcs_comp;
	uint8	papd_end_idx;
	int		papd_valid_retest_num;
	int		papd_cal_idx;
	bool	papd_cal_res_valid;
	lcn20phy_papad_offset_init_t epsdelta2g_flag;
/* #endif LCN20_PAPD_ENABLE */
	int 	base_pwr_dbm;
	int 	target_pwr_dbm;
	bool	do_papd_calidx_est;
	uint8 dccalen_lpbkpath;
	uint8 rxiqcal_lpbkpath;
	int16 cckdigiftype;
	int16 ofdmdigiftype;
	int16 ofdmdigiftypebe;
	int8 cckscale_fctr_db;
	lcn20phy_txiqlocal_phyregs_t *txiqlocal_phyregs;
	lcn20phy_txiqcal_radioregs_t *txiqcal_radioregs;
	/* result of radio rccal */
	uint16 rccal_gmult;
	uint16 rccal_gmult_rc;
	uint8 rccal_dacbuf;
	uint16 rccal_adc_gmult;
	uint32 min_txpwrindex_2g;
	CONST lcn20phy_tx_gain_tbl_entry *txgaintable;
	/* Coefficients for Temperature Conversion to Centigrade */
	/* Temp in deg = (temp_add - (T2-T1)*temp_mult)>>temp_q;  */
	int32 temp_mult;
	int32 temp_add;
	int32 temp_q;
	/* Coefficients for Vbat Conversion to Volts */
	/* Voltage = (vbat_add - (vbat_reading)*vbat_mult)>>vbat_q;  */
	int32 vbat_mult;
	int32 vbat_add;
	int32 vbat_q;
	lcn20phy_tempsense_vbat_radioregs_t *tempsense_vbat_radioregs;
	/* ED thresholds */
	int16 edonthreshold20L;
	int16 edoffthreshold;
	int8 txpwroffset2g[14];
	/* Spur/DSSF modes/params */
	uint8 spurmode; /* from nvram */
	bool  spurmode_override; /* indicate iovar-override */
	uint8 forced_spurmode; /* iovar setting */
	uint8 dssfmode; /* from nvram */
	int16 dssf_thresh[3];
	int8  rssicorr_aci;
	chan_info_20692_lcn20phy_t *tuningtbl;
	/* Tx Gain tables */
	lcn20phy_tx_gain_tbl_entry *gain_table_tx_2g;
	uint8  qam256en;
	/* switch control */
	uint32 swctrlmap_2g[5];
	bool trsw_ctrl_etr;
	int16 ofdm_filt_type;
	/* eLNA bypass */
	uint8 elna_off_gain_idx_2g;
	uint8 tr_isolation;
	uint8 swctrl_gpios; /* enable rfswctrl in GCI gpio control */
	/* Rx gain tables */
	uint8 *gainvaltbl;
	uint32 *gaintbl;
	uint32 *gainidxtbl;
	lcn20phy_lna_params_t *lna_params;
	int8 pmin;    /* pwrMin_range2 */
	int8 pmax;    /* pwrMax_range2 */
	uint8 lcn20_twopwr_txpwrctrl_en;
	uint8 rcal;
	uint8 xtalpn_nv;
	uint8 xtal_pn[14];
	int16 rxgain_tempadj_2g;
	uint8 rssi_cal_freq_grp[14];
	uint8 rssi_cal_channel[LCN20PHY_GROUPS];
	int8 *rssi_corr_gain_delta_2g_sub;
	int8 rssi_channel_offset_rout0[14];
	int8 rssi_channel_offset_rout8[14];
	int8 rssi_channel_offset_elnabyp[14];
	uint8 rxpath_index;
	uint8 rxpath_elna;
	uint16 elna_on_val;
	uint16 elna_bypsss_val;
	int8  gainadj;
	uint8 noise_iqest_en;
	uint8 noise_log_nsamps;
	int8 noise_iqest_gain_adj_2g;
#ifdef WL11ULB
	uint8 ulb_mode;
#endif /* WL11ULB */
	bool logain_NBcal;
	uint8 papdcalidxoffset;
	int8 papdidx_offset_chan12;
	int8 papdidx_offset_chan13;
#ifdef WL_PROXDETECT
	proxd_loopback_gain_t proxd_rx_gain_override;
#endif /* WL_PROXDETECT */
	int8 rssi_rate_offset[LCN20PHY_NUM_RATE_OFFSETS];
};

/* TXIQLOcal tbl offsets */
#define LCN20PHY_TXIQLOCAL_IQCOEFF_OFFSET	64
#define LCN20PHY_TXIQLOCAL_DLOCOEFF_OFFSET	67
#define LCN20PHY_TXIQLOCAL_IQCOMP_OFFSET	96
#define LCN20PHY_TXIQLOCAL_DLOCOMP_OFFSET	98
#define LCN20PHY_TXIQLOCAL_BPHY_IQCOMP_OFFSET	112
#define LCN20PHY_TXIQLOCAL_BPHY_DLOCOMP_OFFSET	114
#define LCN20PHY_TXIQLOCAL_IQBESTCOEFF_OFFSET	128
#define LCN20PHY_TXIQLOCAL_DLOBESTCOEFF_OFFSET	131
#define LCN20PHY_TXIQLOCAL_RIPPLE_BIN_OFFSET	156
#define LCN20PHY_TXIQLOCAL_CORR_I_OFFSET		157
#define LCN20PHY_TXIQLOCAL_CORR_Q_OFFSET		158

/* Tx Pwr Ctrl table offsets */
#define LCN20PHY_TX_PWR_CTRL_GAIN_OFFSET	192
#define LCN20PHY_TX_PWR_CTRL_IQ_OFFSET		320
#define LCN20PHY_TX_PWR_CTRL_LO_OFFSET		448
#define LCN20PHY_TX_PWR_CTRL_PWR_OFFSET		576
#define LCN20PHY_TX_PWR_CTRL_EST_PWR_OFFSET	704
#define LCN20PHY_TX_GAIN_TABLE_SZ			128
#define LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ	128
#define LCN20PHY_TX_PAPD_LUT_SELECT_OFFSET	1024
#define LCN20PHY_INIT_ANCRPOINT	(LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ - 4)
#define LCN20PHY_FRAC_SHIFT	3
#define LCN20PHY_PWR_THRESHOLD	68
#define LCN20PHY_TX_PWR_CTRL_START_NPT		1
#define LCN20PHY_TX_PWR_CTRL_START_INDEX_2G	50
#define txpwrctrl_off(pi) (0x7 != ((phy_utils_read_phyreg(pi, \
	LCN20PHY_TxPwrCtrlCmd) & 0xE000) >> 13))

/* Rx gain table size */
#define LCN20PHY_RXGAINVAL_TBL_SZ			26
#define LCN20PHY_RXGAIN_TBL_SZ				96
#define LCN20PHY_RXGAINIDX_TBL_SZ			76
void *wlc_lcn20_malloc(phy_info_lcn20phy_t *pi_lcn20phy, uint16 size, uint32 line);
void wlc_lcn20_mfree(phy_info_lcn20phy_t *pi_lcn20phy, uint32 line);

/* Functions for fresh buffer access in wlc_phy_lcn20.c file */
#define LCN20PHY_MALLOC(pi, size) wlc_lcn20_malloc((pi)->u.pi_lcn20phy, size, __LINE__)
#define LCN20PHY_MFREE(pi, addr, size) wlc_lcn20_mfree((pi)->u.pi_lcn20phy, __LINE__)

#ifdef WL11ULB
extern bool wlc_phy_lcn20_ulb_10_capable(phy_info_t *pi);
extern bool wlc_phy_lcn20_ulb_5_capable(phy_info_t *pi);
#endif /* WL11ULB */

extern void wlc_lcn20phy_rx_power(phy_info_t *pi, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs, phy_iq_est_t* est, int16 *tot_gain);
extern int wlc_lcn20phy_idle_tssi_reg_iovar(phy_info_t *pi, int32 int_val, bool set, int *err);
extern int wlc_lcn20phy_avg_tssi_reg_iovar(phy_info_t *pi);
extern int32 wlc_lcn20phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr);
extern int32 wlc_lcn20phy_iovar_isenabled_tpc(phy_info_t *pi, int32 *is_enabled);
extern void wlc_lcn20phy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr);
extern int wlc_phy_sample_collect_lcn20phy(phy_info_t *pi, wl_samplecollect_args_t *collect,
	uint32 *buf);
extern int wlc_lcn20phy_aci_modes(phy_info_t *pi, int wanted_mode);
extern void wlc_lcn20phy_aci_init(phy_info_t *pi);
extern bool wlc_lcn20phy_acimode_valid(phy_info_t *pi, int wanted_mode);

extern void
wlc_phy_set_papd_offset_lcn20phy(phy_info_t *pi, int16 int_val);
extern int
wlc_phy_get_papd_offset_lcn20phy(phy_info_t *pi);

extern uint16
wlc_lcn20phy_adjusted_tssi(phy_info_t *pi);
extern uint8
wlc_phy_apply_pwr_tssi_tble_chan_lcn20phy(phy_info_t *pi);
extern int32
wlc_lcn20phy_tssi2dbm(int32 tssi, int32 a1, int32 b0, int32 b1);
extern int8
wlc_phy_estpwrlut_intpol_lcn20phy(phy_info_t *pi, uint8 channel,
       wl_txcal_power_tssi_t *pwr_tssi_lut_ch1, wl_txcal_power_tssi_t *pwr_tssi_lut_ch2);

#if defined(WLTEST)
extern int16
wlc_phy_test_tssi_lcn20phy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs);
extern int16
wlc_phy_test_idletssi_lcn20phy(phy_info_t *pi, int8 ctrl_type);
#endif // endif

extern int16
wlc_lcn20phy_rxpath_rssicorr(phy_info_t *pi, int16 rssi,
	lcn20phy_rssi_gain_params_t *rssi_gain_param, int frm_type, int rate);

extern int16 wlc_lcn20phy_rssi_tempcorr(phy_info_t *pi, bool mode);

extern int16
wlc_lcn20phy_tempsense(phy_info_t *pi, bool mode);

#if defined(WLTEST)
extern void wlc_lcn20phy_force_spurmode(phy_info_t *pi, int16 int_val);
#endif /* WLTEST */
void wlc_phy_init_lcn20phy(phy_info_t *pi);
void wlc_lcn20phy_anacore(phy_info_t *pi, bool on);
void wlc_lcn20phy_switch_radio(phy_info_t *pi, bool on);
void wlc_phy_txpower_recalc_target_lcn20phy(phy_info_t *pi);
bool wlc_lcn20phy_txpwr_srom_read(phy_info_t *pi);
void wlc_lcn20phy_read_table(phy_info_t *pi, phytbl_info_t *pti);
void wlc_lcn20phy_write_table(phy_info_t *pi, const phytbl_info_t *pti);
void wlc_phy_chanspec_set_lcn20phy(phy_info_t *pi, chanspec_t chanspec);

void wlc_lcn20phy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode);
void wlc_lcn20phy_calib_modes(phy_info_t *pi, uint mode);
void wlc_lcn20phy_tssi_setup(phy_info_t *pi);
void wlc_lcn20phy_set_estPwrLUT(phy_info_t *pi, int32 lut_num);
uint8 wlc_lcn20phy_get_bbmult(phy_info_t *pi);
void wlc_lcn20phy_set_bbmult(phy_info_t *pi, uint8 m0);
void wlc_lcn20phy_get_tx_gain(phy_info_t *pi, phy_txgains_t *gains);
void wlc_lcn20phy_set_tx_gain_override(phy_info_t *pi, bool bEnable);
void wlc_lcn20phy_set_tx_gain(phy_info_t *pi,  phy_txgains_t *target_gains);
#define wlc_lcn20phy_enable_tx_gain_override(pi) \
	wlc_lcn20phy_set_tx_gain_override(pi, TRUE)
#define wlc_lcn20phy_disable_tx_gain_override(pi) \
	wlc_lcn20phy_set_tx_gain_override(pi, FALSE)
#endif /* _wlc_phy_lcn20_h_ */
