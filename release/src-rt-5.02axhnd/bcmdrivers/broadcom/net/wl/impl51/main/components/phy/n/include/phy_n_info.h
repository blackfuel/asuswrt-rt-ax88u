/*
 * nPHY module internal interface (to other PHY modules).
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
 * $Id: phy_n_info.h 689894 2017-03-13 23:32:18Z $
 */

#ifndef _phy_n_info_h_
#define _phy_n_info_h_

#include <typedefs.h>

#define NPHY_CORE_NUM	2	/* N-PHY chips have exactly 2 PHY cores */

/* interference mitigation rssi values */
#define NPHY_NOISE_PWR_FIFO_DEPTH 6
#define PHY_RSSI_WINDOW_SZ 16
#define PHY_INTF_RSSI_INIT_VAL -60 /* -60: any number between -30 and -90 */
#define PHY_CRSMIN_ARRAY_MAX 3
#define PHY_CRSMIN_IDX_MAX (PHY_CRSMIN_ARRAY_MAX - 1)
#define PHY_CRSMIN_RANGE 3

#define PHY_CRSMIN_GE7_ACIOFF_2G -86
#define PHY_CRSMIN_GE7_ACION_2G -79
#define PHY_CRSMIN_LT7_ACIOFF_2G -82
#define PHY_CRSMIN_LT7_ACION_2G -74

#define PHY_CRSMIN_ACI2G_PWR_0 6
#define PHY_CRSMIN_ACI2G_PWR_1 18
#define PHY_CRSMIN_ACI2G_PWR_2 30

#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_2G -77
#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_2G_MAX -62
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_2G -70
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_2G_MAX -46

#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_5G -77
#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_5G_MAX -62
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_5G -77
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_5G_MAX -62

#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_2G -73
#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_2G_MAX -61
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_2G -65
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_2G_MAX -44

#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_5G -73
#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_5G_MAX -61
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_5G -73
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_5G_MAX -61

#define NPHY_GAIN_VS_TEMP_SLOPE_2G 6   /* units: db/100C */
#define NPHY_GAIN_VS_TEMP_SLOPE_5G 6   /* units: db/100C */

#define IS_ELNA_BRD ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &\
		BFL_ELNA_GAINDEF) && ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &\
		BFL_EXTLNA) || (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz)))

#define NPHY_GAIN_DELTA_2G_PARAMS 18
#define NPHY_GAIN_DELTA_5G_PARAMS 8

#define NPHY_RFSEQ_RESET2RX_MASK 0x20
/*
 * Spin at most 'us' microseconds while 'exp' is true.
 * Caller should explicitly test 'exp' when this completes
 * and take appropriate error action if 'exp' is still true.
 */
#define SPINPHYWAIT(exp, us) { \
	uint countdown = (us); \
	while ((exp) && (countdown >= 1)) {\
		OSL_DELAY(1); \
		countdown -= 1; \
	} \
}

typedef struct _nphy_txpwrindex {
	int8   index;
	int8   index_internal;     /* store initial or user specified txpwr index */
	int8   index_internal_save;
	uint16 AfectrlOverride;
	uint16 AfeCtrlDacGain;
	uint16 rad_gain;
	uint8  bbmult;
	uint16 iqcomp_a;
	uint16 iqcomp_b;
	uint16 locomp;
} phy_txpwrindex_t;

typedef struct _nphy_pwrctrl {
	int8	idle_targ_2g;
	int8	idle_targ_5g[PHY_MAXNUM_5GSUBBANDS];
	int16	idle_tssi_2g;
	int16	idle_tssi_5g;
	int16	idle_tssi;
	int16	a1;
	int16	b0;
	int16	b1;
} phy_pwrctrl_t;

/* ********************************************************* */
#include "phy_api.h"
#include "phy_n_ana.h"
#include "phy_n_chanmgr.h"
#include "phy_n_radio.h"
#include "phy_n_tbl.h"
#include "phy_n_tpc.h"
#include "phy_n_radar.h"
#include "phy_n_noise.h"
#include "phy_n_antdiv.h"
#include "phy_n_temp.h"
#include "phy_n_rssi.h"
#include "phy_n_rxiqcal.h"
#include "phy_n_txiqlocal.h"
#include "phy_n_papdcal.h"
#include "phy_n_vcocal.h"
#include "phy_n_calmgr.h"
#include "phy_n_cache.h"
#include "phy_n_misc.h"
#include "phy_n_rxgcrs.h"
#include "phy_n_btcx.h"
#include "phy_n_lpc.h"
#include "phy_n_rxspur.h"
#include "phy_n_dbg.h"
#include "phy_n_stf.h"
/* ********************************************************* */

struct phy_info_nphy {
/* ********************************************************* */
	phy_info_t *pi;
	phy_n_ana_info_t		*anai;
	phy_n_radio_info_t		*radioi;
	phy_n_tbl_info_t		*tbli;
	phy_n_tpc_info_t		*tpci;
	phy_n_radar_info_t		*radari;
	phy_n_noise_info_t		*noisei;
	phy_n_antdiv_info_t		*antdivi;
	phy_n_temp_info_t		*tempi;
	phy_n_rssi_info_t		*rssii;
	phy_n_rxiqcal_info_t	*rxiqcali;
	phy_n_txiqlocal_info_t	*txiqlocali;
	phy_n_papdcal_info_t	*papdcali;
	phy_n_vcocal_info_t		*vcocali;
	phy_n_calmgr_info_t		*calmgri;
	phy_n_cache_info_t		*cachei;
	phy_n_misc_info_t		*misci;
	phy_n_rxgcrs_info_t		*rxgcrsi;
	phy_n_chanmgr_info_t	*chanmgri;
	phy_n_btcx_info_t		*btcxi;
	phy_n_lpc_info_t		*lpci;
	phy_n_rxspur_info_t		*rxspuri;
	phy_n_dbg_info_t		*dbgi;
	phy_n_stf_info_t		*stfi;
/* ********************************************************* */
	uint32 pstart; /* sample collect fifo begins */
	uint32 pstop;  /* sample collect fifo ends */
	uint32 pfirst; /* sample collect trigger begins */
	uint32 plast;  /* sample collect trigger ends */
	uint16  rfctrlIntc1_save;
	uint16  rfctrlIntc2_save;
	bool    phyhang_avoid;  /* nphy rev 3, make PHY receiver deaf before accessing tables */

	/* Entry Id in Rx2Tx seq table that has the CLR_RXTX_BIAS opcode */

	uint8   adj_pwr_tbl_nphy[ADJ_PWR_TBL_LEN];      /* Adjusted power table of NPHY */
	bool    nphy_papd_kill_switch_en; /* flag to indicate if lna kill switch is enabled */
	uint8	nphy_txGainTable_mode; /* 0 : .25dB step, 1 : .5dB step */

	bool	nphy_rxiqcal_fw_war_en;
	bool	dynamic_rflo_war_en;
	uint16  TX_logen5g_idac1_core0;
	uint16  TX_logen5g_idac1_core1;
	uint16  TX_logen5g_idac2_core0;
	uint16  TX_logen5g_idac2_core1;
	uint16  TX_logen5g_tune_core0;
	uint16  TX_logen5g_tune_core1;
	uint16  RX_logen5g_idac1_core0;
	uint16  RX_logen5g_idac1_core1;
	uint16  RX_logen5g_idac2_core0;
	uint16  RX_logen5g_idac2_core1;
	uint16  RX_logen5g_tune_core0;
	uint16  RX_logen5g_tune_core1;
	bool    txiqlo_cal_twice;
	bool	conseq_clips;
	uint8	clip_counts;
	bool	conseq_noclips;
	uint8	no_clip_counts;

	bool	firstTime;
	bool	save_cmds;
	uint16	save_cmdgctl;
	uint16	save_cmdgctl_8;

	uint16  saved_bbmult0;

	bool    nphy_anarxlpf_adjusted;
	uint16  nphy_rccal_value;
	uint16  nphy_crsminpwr[3];

	bool    nphy_crsminpwr_adjusted;
	bool    nphy_noisevars_adjusted;
	bool    nphy_base_nvars_adjusted;

#if defined(RXDESENS_EN)
	bool	ntd_crs_adjusted;
	uint16	ntd_crsminpwr[3];
	uint16	ntd_initgain;
	uint16	ntd_current_rxdesens;
	uint16	ntd_save_current_rxdesens;
	uint16	ntd_save_current_rxdesens_channel;
	bool	ntd_rxdesens_active;
#endif // endif
	bool	ntd_papdcal_dcs;

	bool    nphy_sample_play_lpf_bw_ctl_ovr;
	uint8	nphy_disable_stalls;
	uint32  nphy_bb_mult_save;
	uint16  tx_rx_cal_radio_saveregs[24];
	uint16  tx_rx_cal_radio_saveregs_rev19[NPHY_CORE_NUM];
	bool	tx_rx_radio_reg_save;
	/* new flag to signal periodic_cal is running to blank radar */
	bool    nphy_rxcal_active;
	uint32  nphy_rxcalparams;
	bool    nphy_force_papd_cal;
	uint8  nphy_current_tx_gain[NPHY_CORE_NUM];
	uint8  nphy_papd_tx_gain_at_last_cal[NPHY_CORE_NUM]; /* Tx gain index at last papd cal */
	uint    nphy_papd_last_cal;     /* time of last papd cal */
	uint32  nphy_papd_recal_counter;
	/*
	 * Tx gain pga index used during last papd cal.
	 * For REVs>=7, the PAD index is stored in the 2G band and the PGA index is stored in
	 * the 5G band
	 */
	uint8   nphy_papd_cal_gain_index[NPHY_CORE_NUM];
	bool    nphy_papdcomp;

	uint8   nphy_txpid2g[NPHY_CORE_NUM];
	uint8   nphy_txpid5g[NPHY_CORE_NUM][PHY_MAXNUM_5GSUBBANDS];
	uint8   tx_precal_tssi_radio_saveregs[NPHY_CORE_NUM][4];

	/* Tx power indices/gains during nphy cal */
	uint8   nphy_cal_orig_pwr_idx[NPHY_CORE_NUM];
	uint8   nphy_txcal_pwr_idx[NPHY_CORE_NUM];
	uint8   nphy_rxcal_pwr_idx[NPHY_CORE_NUM];
	uint16  nphy_cal_orig_tx_gain[NPHY_CORE_NUM];
	nphy_txgains_t  nphy_cal_target_gain;
	uint16  nphy_txcal_bbmult;
	uint16  nphy_gmval;

	uint16  nphy_saved_bbconf;
	uint    nphy_deaf_count;
	/* Variable to store the value of phyreg NPHY_fineclockgatecontrol */
	uint16  nphy_fineclockgatecontrol;
	uint8   nphy_pabias;        /* override PA bias value, 0: no override */
	uint8   nphy_txpwr_idx_2G[3];    /* to store the power control txpwr index for 2G band */
	uint8   nphy_txpwr_idx_5G[3];    /* to store the power control txpwr index for 5G band */

	uint16   init_txpwr_idx_2G[NPHY_CORE_NUM];    /* to store the txpwr baseindex for 2G band */
	uint16   init_txpwr_idx_5G[NPHY_CORE_NUM];    /* to store the txpwr baseindex for 5G band */
	uint16   nphy_txpwr_baseidx[NPHY_CORE_NUM];   /* to store HW pwr control txpwr baseindex */

	int16   nphy_papd_epsilon_offset[NPHY_CORE_NUM];
	phy_txpwrindex_t nphy_txpwrindex[NPHY_CORE_NUM]; /* independent override per core */
	phy_pwrctrl_t nphy_pwrctrl_info[NPHY_CORE_NUM]; /* Tx pwr ctrl info per Tx chain */

	/* Draconian Power Limits for Sulley */
	int16 tssi_maxpwr_limit;
	int16 tssi_minpwr_limit;
	uint8 tssi_ladder_offset_maxpwr;
	uint8 tssi_ladder_offset_minpwr;

	chanspec_t      nphy_rssical_chanspec_2G; /* 0: invalid, other: last valid cal chanspec */
	chanspec_t      nphy_rssical_chanspec_5G; /* 0: invalid, other: last valid cal chanspec */
	bool nphy_use_int_tx_iqlo_cal; /* Flag to determine if the Tx IQ/LO cal should be performed
					* using the radio's internal envelope detectors.
					*/
	bool nphy_int_tx_iqlo_cal_tapoff_intpa; /* Flag to determine whether the internal Tx
						* IQ/LO cal would be use the signal from the
						* radio's internal envelope detector at the
						* PAD tapoff or the intPA tapoff point.
						*/
	/* interference mitigation rssi vars */
	int16 intf_rssi_vals[PHY_RSSI_WINDOW_SZ];
	int16 intf_rssi_window_idx;
	int16 intf_rssi_avg;

	int16 crsmin_rssi_avg_acioff_2G;
	int16 crsmin_rssi_avg_acion_2G;
	int16 crsmin_pwr_aci2g[PHY_CRSMIN_ARRAY_MAX];

	int16 rfgain_rssi_avg_acioff_2G;
	int16 rfgain_rssi_avg_acioff_2G_max;
	int16 rfgain_rssi_avg_acioff_5G;
	int16 rfgain_rssi_avg_acioff_5G_max;

	int16 rfgain_rssi_avg_acion_2G;
	int16 rfgain_rssi_avg_acion_2G_max;
	int16 rfgain_rssi_avg_acion_5G;
	int16 rfgain_rssi_avg_acion_5G_max;

	uint8	elna2g;
	uint8	elna5g;

	bool	phy_isspuravoid;	/* TRUE if spur avoidance is ON for the current channel,
					 * else FALSE
					 */

	txiqcal_cache_t 		nphy_calibration_cache;
	rssical_cache_t			nphy_rssical_cache;

	uint16	tx_rx_cal_phy_saveregs[17];
	uint16	tx_rx_cal_phy_saveregs_rev19[20];
	bool	do_initcal;		/* to enable/disable phy init cal */
	uint8	cal_type_override;      /* cal override set from command line */
	uint8	nphy_papd_skip;     /* skip papd calibration for IPA case */
	chanspec_t	nphy_iqcal_chanspec_2G;	/* 0: invalid, other: last valid cal chanspec */
	chanspec_t	nphy_iqcal_chanspec_5G;	/* 0: invalid, other: last valid cal chanspec */

	uint8	nphy_papd_cal_type;
	bool	nphy_gband_spurwar2_en;
	bool	nphy_aband_spurwar_en;
	phy_noisevar_buf_t	nphy_saved_noisevars;

	/* Entry Id in Rx2Tx seq table that has the CLR_RXTX_BIAS opcode */
	int8	rx2tx_biasentry;

	uint16 orig_rfctrloverride[2];
	uint16 orig_rfctrlauxreg[2];
	uint16 orig_rxlpf_rccal_hpc_ovr_val;
	bool is_orig;
	nphy_txgains_t nphy_ipa_pref_gain;

	uint16 rccal_capval[6];
	bool ncb_triso_comp_done;
	uint8 ppr_offsets_copied;
	int16 noisecal_rssi_offset[NPHY_CORE_NUM];
	int8 sample_collect_gainadj;
	uint16  lowpwrDiq[2];           /* Digital LOFT coeffs for low power */
	int8 rssi_gain_delta_2g[NPHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_gain_delta_2gh[NPHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_gain_delta_2ghh[NPHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_gain_delta_5gl[NPHY_GAIN_DELTA_5G_PARAMS];
	int8 rssi_gain_delta_5gml[NPHY_GAIN_DELTA_5G_PARAMS];
	int8 rssi_gain_delta_5gmu[NPHY_GAIN_DELTA_5G_PARAMS];
	int8 rssi_gain_delta_5gh[NPHY_GAIN_DELTA_5G_PARAMS];
	int8 gain_cal_temp;
	uint16 tx_iqcal_rfphy_saveregs[12]; /* To save RF-PHY registers during TX-IQ cal */

	uint16 save_OVR19;
	uint16 save_OVR3; /* save Rout overrides during rxiqcal & restore back when done.
			   * Takes care of removing ACI override of gain during rxiqcal &
			   * restore after cal done.
			   */
	int32 last_valid_temp; /* For Tx Index Cap based on FSM tempsense */
	int32	ed_assert_thresh_dbm; /* nvram tunable for programming ed assert thresh */
	bool txidxcap_hi_inuse;
	bool txidxcap_lo_inuse;

#if defined(BCMDBG)
	uint16 nphy_papd_mix_ovr[NPHY_CORE_NUM];
	uint16 nphy_papd_attn_ovr[NPHY_CORE_NUM];
	uint16 nphy_papd_pga_settled[NPHY_CORE_NUM];
#endif // endif
};

enum {
	NPHY_FILT_OFDM20 = 0,
	NPHY_FILT_OFDM40 = 1,
	NPHY_FILT_CCK_GAUSS2_2 = 2,
	NPHY_FILT_4322_20MHZ = 3,
	NPHY_FILT_4322_40MHZ = 4,
	NPHY_FILT_ABAND_OFDM20 = 5,
	NPHY_FILT_CCK_JAPAN_CH14 = 6,
	NPHY_FILT_OFDM22 = 7,
	NPHY_FILT_53572_OFDM40 = 8,
	NPHY_FILT_43217_OFDM26 = 9,
	NPHY_FILT_43217_OFDM5 = 10,
	NPHY_FILT_43217_OFDM20 = 11,
	NPHY_FILT_OFDM3 = 12,
	NPHY_FILT_OFDM26_40MHZ = 13,
	NPHY_FILT_OFDM6_40MHZ = 14,
	NPHY_FILT_OFDM8_40MHZ = 15,
	NPHY_FILT_OFDM1_20MHZ = 16
};

#endif /* _phy_n_info_h_ */
