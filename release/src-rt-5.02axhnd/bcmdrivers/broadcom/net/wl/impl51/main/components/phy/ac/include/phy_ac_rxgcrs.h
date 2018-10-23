/*
 * ACPHY Rx Gain Control and Carrier Sense module interface (to other PHY modules).
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
 * $Id: phy_ac_rxgcrs.h 759965 2018-04-27 08:01:39Z $
 */

#ifndef _phy_ac_rxgcrs_h_
#define _phy_ac_rxgcrs_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_rxgcrs.h>
#include <phy_ac_noise.h>

/* forward declaration */
typedef struct phy_ac_rxgcrs_info phy_ac_rxgcrs_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_rxgcrs_info_t *phy_ac_rxgcrs_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_rxgcrs_info_t *cmn_info);
void phy_ac_rxgcrs_unregister_impl(phy_ac_rxgcrs_info_t *ac_info);

/* inter-module data API */
bool phy_ac_rxgcrs_get_md_trtx(phy_ac_rxgcrs_info_t *rxgcrsi);
uint16 phy_ac_rxgcrs_get_deaf_count(phy_ac_rxgcrs_info_t *rxgcrsi);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

/* ********************************* */
/*		Rx gain related definitions		*/
/* ********************************* */
/* gainctrl */
/* stages: elna, lna1, lna2, mix, bq0, bq1, dvga (tiny only) */
#define ACPHY_MAX_RX_GAIN_STAGES 7
#define ACPHY_INIT_GAIN 69
#define ACPHY_HI_GAIN 48
#define ACPHY_CRSMIN_DEFAULT_NON_TINY 54
#define CRSMIN_DELTA_GAIN_DIFF   (3 * (TINY_RADIO(pi)) *			\
	(ACPHY_INIT_GAIN - ACPHY_INIT_GAIN_TINY))
#define CRSMIN_DELTA_BW (3 * (CHSPEC_IS10(pi->radio_chanspec)) +		\
	6 * (CHSPEC_IS5(pi->radio_chanspec)) + 9 * (CHSPEC_IS2P5(pi->radio_chanspec)))
#define ACPHY_CRSMIN_DEFAULT ((ACMAJORREV_32(pi->pubpi->phy_rev) || \
	ACMAJORREV_33(pi->pubpi->phy_rev)) ? 54 : (ACPHY_CRSMIN_DEFAULT_NON_TINY - \
	CRSMIN_DELTA_GAIN_DIFF - CRSMIN_DELTA_BW))
#define ACPHY_BPHYCRSMIN 0x6a
#define ACPHY_CRSMIN_GAINHI  0x20
#define ACPHY_MAX_LNA1_IDX 5
#define ACPHY_4365_MAX_LNA1_IDX 4
#define ACPHY_ILNA2G_MAX_LNA2_IDX 6
#define ACPHY_ELNA2G_MAX_LNA2_IDX 4
#define ACPHY_ELNA2G_MAX_LNA2_IDX_L 5
#define ACPHY_ILNA5G_MAX_LNA2_IDX 6
#define ACPHY_ELNA5G_MAX_LNA2_IDX 6
#define ACPHY_28nm_MAX_LNA1_IDX 5
#define ACPHY_LESISCALE_DEFAULT 64
#define ACPHY_LO_NF_MODE_ELNA_TINY(pi) (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID) && \
	((CHSPEC_IS2G(pi->radio_chanspec)) ? BF_ELNA_2G(pi->u.pi_acphy) :              \
	BF_ELNA_5G(pi->u.pi_acphy)) && !(ACMAJORREV_32(pi->pubpi->phy_rev) || \
	ACMAJORREV_33(pi->pubpi->phy_rev)))
#define ACPHY_LO_NF_MODE_ELNA_28NM(pi) (IS_28NM_RADIO(pi) && \
		((CHSPEC_IS2G(pi->radio_chanspec)) ? BF_ELNA_2G(pi->u.pi_acphy) : \
		BF_ELNA_5G(pi->u.pi_acphy)) && pi->u.pi_acphy->rxgcrsi->lonf_elna_mode)
/* Following MACROS will be used by GBD only.
 * Tiny gain control has a local array of INIT and clip gains.
 */
#define ACPHY_INIT_GAIN_TINY ((RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) ? \
	(ACPHY_INIT_GAIN) : (ACPHY_LO_NF_MODE_ELNA_TINY(pi) ? 64 : 61))
#define ACPHY_HI_GAIN_TINY ((RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) ? \
	(ACPHY_HI_GAIN) : (ACPHY_LO_NF_MODE_ELNA_TINY(pi) ? 43 : 37))
/* End of GBD used MACRO
 */
#define ACPHY_INIT_GAIN_28NM_ULP 65
#define ACPHY_HI_GAIN_28NM_ULP 44
#define ACPHY_INIT_GAIN_28NM (\
	(ACMAJORREV_40(pi->pubpi->phy_rev) && CHSPEC_IS2G(pi->radio_chanspec)) ? 64 : \
	(ACMAJORREV_44(pi->pubpi->phy_rev) && (pi->pubpi->slice == DUALMAC_AUX)) ? 66 : \
	(ACMAJORREV_47(pi->pubpi->phy_rev) && (RADIOREV(pi->pubpi->radiorev) > 0)) ? 64 : \
	67)

#define ACPHY_INIT_GAIN_4365_2G 67
#define ACPHY_INIT_GAIN_4365_5G 64

#define ACPHY_20693_MAX_LNA2_IDX (ACPHY_LO_NF_MODE_ELNA_TINY(pi) ? 2 : 3)
#define ACPHY_20695_MAX_LNA2_IDX 2
#define ACPHY_4365_MAX_LNA2_IDX 2
#define MAX_ANALOG_RX_GAIN_TINY (ACPHY_LO_NF_MODE_ELNA_TINY(pi) ? 61 : 55)
#define MAX_ANALOG_RX_GAIN_28NM_ULP 65
#define MAX_ANALOG_RX_GAIN_28NM_ULP_LONF 65

/* Programming this value into gainlimittable for an index prevents AGC from using it */
#define GAINLIMIT_MAX_OUT 127

#define ELNA_ID 0
#define LNA1_ID 1
#define LNA2_ID 2
#define TIA_ID	3
#define BQ0_ID 4
#define BQ1_ID 5
#define DVGA_ID 6

#define ELNA_OFFSET         0
#define EXT_LNA_OFFSET		0
#define LNA1_OFFSET		8
#define LNA2_OFFSET		16
#define TIA_OFFSET		32
#define BQ0_OFFSET      96
#define BQ1_OFFSET          112
#define DVGA1_OFFSET		112
#define BIQ0_OFFSET		96

#define INIT_GAIN 0
#define HIGH_GAIN 1
#define MID_GAIN 2
#define LOW_GAIN 3
#define CLIP2_GAIN 4
#define SSAGC_CLIP_GAIN 5
#define GAINLMT_TBL_BAND_OFFSET 64

/* LNA1 bypass settings for 43012 */
#define LNA1_BYPASS_GAIN_2G_20695	-10
#define LNA1_BYPASS_GAIN_5G_20695	-7
#define LNA1_BYPASS_ROUT_2G_20695	15
#define LNA1_BYPASS_ROUT_5G_20695	8
#define LNA1_BYPASS_INDEX	6

/* RX GAIN PARAMS */
#define LNA1_TBL_IND		0
#define LNA2_TBL_IND		1
#define TIA_TBL_IND		2
#define BIQ0_TBL_IND		3
#define BIQ1_TBL_IND		4
#define RXGAIN_CONF_ELEMENTS	5
#define MAX_RX_GAINS_PER_ELEM	12
#define N_LNA12_GAINS		7
#define N_TIA_GAINS		12
#define N_BIQ01_GAINS		6

/* SSAGC clip1 gains count */
#define SSAGC_CLIP1_GAINS_CNT 19

/* SSAGC clip2 index count */
#define SSAGC_CLIP2_IDX_CNT 13

/* SSAGC clip2 table entries */
#define SSAGC_CLIP2_TBL_IDX_CNT	(SSAGC_CLIP1_GAINS_CNT * SSAGC_CLIP2_IDX_CNT)

#define SSAGC_N_CLIPCNT_THRESH 6
#define SSAGC_N_RSSI_THRESH 7

typedef struct {
	uint8 elna;
	uint8 trloss;
	uint8 elna_bypass_tr;
	uint8 lna1byp;
} acphy_fem_rxgains_t;

typedef struct acphy_rxgainctrl_params {
	int8  gaintbl[ACPHY_MAX_RX_GAIN_STAGES][15];
	uint8 gainbitstbl[ACPHY_MAX_RX_GAIN_STAGES][15];
} acphy_rxgainctrl_t;

typedef enum {
	GET_LNA_GAINCODE,
	GET_LNA_ROUT
} acphy_lna_gain_rout_t;

/* Different AGC types */
typedef enum {
	FAST_AGC,
	SINGLE_SHOT_AGC
} acphy_agc_type_t;

/* Different densense types */
typedef enum {
	LTE_DESENSE,
	BT_DESENSE,
	CURR_DESENSE,
	ZERO_DESENSE,
	TOTAL_DESENSE
} phy_ac_desense_type_t;

extern uint8 wlc_phy_rxgainctrl_encode_gain_acphy(phy_info_t *pi, uint8 core, int8 gain_dB,
	bool trloss, bool lna1byp, uint8 clipgain, uint8 *gidx);
extern uint8 wlc_phy_get_lna_gain_rout(phy_info_t *pi, uint8 idx,
	acphy_lna_gain_rout_t type);
void wlc_phy_get_rxgain_acphy(phy_info_t *pi, rxgain_t rxgain[], int16 *tot_gain,
                              uint8 force_gain_type);

#ifndef WLC_DISABLE_ACI
extern void wlc_phy_desense_apply_acphy(phy_info_t *pi, bool apply_desense);
extern void wlc_phy_desense_calc_total_acphy(phy_ac_rxgcrs_info_t *rxgcrs_info);
#endif /* WLC_DISABLE_ACI */

extern void wlc_phy_rxgainctrl_gainctrl_acphy(phy_info_t *pi);
extern void wlc_phy_upd_lna1_lna2_gains_acphy(phy_info_t *pi);
extern void wlc_phy_upd_lna1_lna2_gaintbls_acphy(phy_info_t *pi, uint8 lna12);
extern void wlc_phy_upd_lna1_lna2_gainlimittbls_acphy(phy_info_t *pi, uint8 lna12);
extern void wlc_phy_upd_lna1_lna2_gainlimittbls_28nm(phy_info_t *pi, uint8 lna12,
		uint16 gainlimitid, uint8 core);
extern void wlc_phy_upd_lna1_bypass_acphy(phy_info_t *pi, uint8 core);
void phy_ac_rxgcrs_upd_mix_gains(phy_ac_rxgcrs_info_t *rxgcrsi);
extern void wlc_phy_rfctrl_override_rxgain_acphy(phy_info_t *pi, uint8 restore,
	rxgain_t rxgain[], rxgain_ovrd_t rxgain_ovrd[]);
extern uint8 wlc_phy_calc_extra_init_gain_acphy(phy_info_t *pi, uint8 extra_gain_3dB,
	rxgain_t rxgain[]);
extern uint8 wlc_phy_get_max_lna_index_acphy(phy_info_t *pi, uint8 lna);
extern void wlc_phy_rxgainctrl_gainctrl_acphy_tiny(phy_info_t *pi, uint8 init_desense);
extern void wlc_phy_rxgainctrl_gainctrl_acphy_28nm_ulp(phy_info_t *pi);
extern void wlc_phy_rxgainctrl_gainctrl_acphy_28nm(phy_info_t *pi, uint8 init_desense);

extern void wlc_phy_rxgainctrl_set_gaintbls_acphy(phy_info_t *pi, bool init,
	bool band_change, bool bw_change);
extern void wlc_phy_rxgainctrl_set_gaintbls_acphy_wave2(phy_info_t *pi, uint8 core,
	uint16 gain_tblid, uint16  gainbits_tblid);
extern void wlc_phy_rxgainctrl_set_gaintbls_acphy_tiny(phy_info_t *pi, uint8 core,
	uint16 gain_tblid, uint16 gainbits_tblid);
extern void wlc_phy_rxgainctrl_set_gaintbls_acphy_28nm_ulp(phy_info_t *pi, uint8 core,
	uint16 gain_tblid, uint16 gainbits_tblid);
extern void wlc_phy_rxgainctrl_set_gaintbls_acphy_28nm(phy_info_t *pi, uint8 core,
	uint16 gain_tblid, uint16 gainbits_tblid);
extern void wlc_phy_set_trloss_reg_acphy(phy_info_t *pi, int8 core);
extern void wlc_phy_set_lna1byp_reg_acphy(phy_info_t *pi, int8 core);
extern void wlc_phy_rxgain_adj_forrssi_acphy(phy_info_t *pi, int8 core, uint8 force_gain_type);
extern void wlc_phy_set_agc_gaintbls_acphy(phy_info_t *pi, uint32 gain_tblid, const void *gain,
	uint32 gainbits_tblid, const void *gainbits, uint32 gainlimits_tblid,
	const void *gainlimits_ofdm, const void *gainlimits_cck, uint32 offset, uint32 len);

extern void phy_ac_subband_cust_28nm_ulp(phy_info_t *pi);
extern void phy_ac_agc_config(phy_info_t *pi, uint8 agc_type);
/* ************************************* */
/*		Carrier Sense related definitions		*/
/* ************************************* */

extern void wlc_phy_force_crsmin_acphy(phy_info_t *pi, void *p);
void phy_ac_rxgcrs_force_lesiscale(phy_ac_rxgcrs_info_t *rxgcrsi, void *p);
extern int phy_ac_rxgcrs_min_pwr_cal(phy_ac_rxgcrs_info_t *rxgcrsi, uint8 crsmin_cal_mode);
void phy_ac_rxgcrs_set_lesiscale(phy_ac_rxgcrs_info_t *rxgcrsi, int8 *lesi_scale);
extern void wlc_phy_force_gainlevel_acphy(phy_info_t *pi, int16 int_val);
extern void wlc_phy_set_srom_eu_edthresh_acphy(phy_info_t *pi);
void phy_ac_rxgcrs_lesi(phy_ac_rxgcrs_info_t *rxgcrsi, bool on, uint8 delta_halfdB);
int phy_ac_rxgcrs_iovar_get_lesi(phy_ac_rxgcrs_info_t *rxgcrsi, int32 *ret_val);
int phy_ac_rxgcrs_iovar_get_lesi_cap(phy_ac_rxgcrs_info_t *rxgcrsi, int32 *ret_val);
int phy_ac_rxgcrs_iovar_get_lesi_ovrd(phy_ac_rxgcrs_info_t *rxgcrsi, int32 *ret_val);
int phy_ac_rxgcrs_iovar_set_lesi_ovrd(phy_ac_rxgcrs_info_t *rxgcrsi, int32 set_val);
extern void phy_ac_get_fem_rxgains(phy_info_t *pi, int8 *rx_gains);
extern void phy_ac_get_rxgains_ctrl(phy_info_t *pi, int8 *rx_gains, int8 *input_param);
extern void chanspec_setup_rxgcrs(phy_info_t *pi);
extern void phy_ac_rxgcrs_set_mclip_agc_en(phy_info_t *pi, bool en);
extern bool phy_ac_rxgcrs_get_cap_lesi(phy_info_t *pi);
bool phy_ac_rxgcrs_get_cap_mclip_agc_en(phy_info_t *pi);
bool phy_ac_rxgcrs_get_cap_lesi(phy_info_t *pi);
#ifdef WL11ULB
void phy_ac_rxgcrs_ulb_cal(phy_ac_rxgcrs_info_t *rxgcrsi, uint8 enULB);
#endif /* WL11ULB */
void phy_ac_rxgcrs_cal(phy_ac_rxgcrs_info_t *rxgcrsi, uint8 enULB);
void phy_ac_ovr_rxgain_noise_sample_request(phy_ac_rxgcrs_info_t *rxgcrsi, bool set_ovr);
#if defined(BCMDBG) || defined(WLTEST)
void phy_ac_rxgcrs_cal_dump(phy_ac_rxgcrs_info_t* rxgcrsi, struct bcmstrbuf *b);
#endif // endif
void phy_ac_rxgcrs_set_desense(phy_ac_rxgcrs_info_t *rxgcrs_info, acphy_desense_values_t *desense,
	phy_ac_desense_type_t type);
acphy_desense_values_t phy_ac_rxgcrs_get_desense(phy_ac_rxgcrs_info_t *rxgcrs_info,
	phy_ac_desense_type_t type);
void phy_ac_rxgcrs_clean_noise_array(phy_ac_rxgcrs_info_t *rxgcrsi);
extern void wlc_phy_set_crs_min_offsets_acphy(phy_info_t *pi, uint8 core,
	int8 ac_offset, int8 mf_offset);
#endif /* _phy_ac_rxgcrs_h_ */
