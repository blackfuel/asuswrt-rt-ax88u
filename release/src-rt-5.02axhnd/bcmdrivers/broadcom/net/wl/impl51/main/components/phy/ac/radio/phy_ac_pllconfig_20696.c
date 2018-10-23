/*
 * ACPHY 20696 Radio PLL configuration
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
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#include <wlc_radioreg_20696.h>
#include <phy_ac_pllconfig_20696.h>

#define DBG_PLL 0

/* Fixed point constant defines */
/* 2^-13 in 1.15.16 */
#define FIX_2POW_MIN13					0x8
/* 2.0 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYEND_FX		0x20000
/* 10.0 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYSTARTCOLD_FX	0xA0000
/* 0.5 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYSTARTWARM_FX	0x8000
/* 1.0 in 1.15.16 */
#define RFPLL_VCOCAL_PAUSECNT_FX		0x10000
/* 1.7 in 1.15.16 */
#define T_OFF_FX						0x1b333
/* 1.0 in 1.15.16 */
#define FX_ONE							0x10000
/* ac_mode=0: 0.865 mA in 0.16.16 */
#define ICP_FX_AC0						0xDD71
/* ac_mode=1: 1.382 mA in 0.16.16 */
#define ICP_FX_AC1						0x161CB
/* ((1000 * 2 * pi) / f3db_ovr_fn) in 1.23.8 */
#define WN_CONST_FX						0x9E31A
/* 257.18 in 1.15.16 */
#define CONST_257_18_FX					0x1012e14
/* 281.21 in 1.15.16 */
#define CONST_281_21_FX					0x11935c3
/* 3.2619 in 1.15.16 */
#define CONST_3P2619_FX					0x3430c
/* 0.5041 in 1.15.16 */
#define CONST_P5041_FX					0x810d
/* 7.8202 in 1.15.16 */
#define CONST_7P8202_FX					0x7d1f9
/* 0.21758 in 1.15.16 */
#define CONST_P21758_FX					0x37b3
/* 3.8879 in 1.15.16 */
#define CONST_3P8879_FX					0x3e34d
/* 0.11059 in 1.15.16 */
#define CONST_P11059_FX					0x1c50
/* 1.7649 in 1.15.16 */
#define CONST_1P7649_FX					0x1c3d0
/* 0.055381 in 1.15.16 */
#define CONST_P055381_FX				0xe2d
/* 0.0384 in 1.15.16 */
#define CONST_P0384_FX					0x9D5

/* (CAP_MULTIPLIER_RATIO_PLUS_ONE / CAP_MULTIPLIER_RATIO) in 1.15.16 */
#define RF_CONST1_FX					0x12000

/* Band dependent constants */
#define VCO_CAL_CAP_BITS					11
/* damp = 1 */
/* f3db_ovr_fn = sqrt(1 + 2 * damp^2 + sqrt(2 + 4 * damp^2 + 4 * damp^4)); */
/* f3db_ovr_fn = 2.4823935345082537 */
/* pi = 3.14159265358979 */
/* ((kvco * f3db_ovr_fn^2) / (4 * pi^2)) * 1e9 in 1.32.16; Kvco = 20 */
#define C1_PASSIVE_LF_CONST_FX			0xba13983c4e13ULL
/* (2 * damp) / (kvco) in 0.0.32; kvco=20 */
#define R1_PASSIVE_LF_CONST_FX			0x1999999a
/* 5 * 256 * 1e-3  in 0.16.16 */
#define IOFF_REF_CONST_FX				0x147ae

/* Constants */
#define VCO_CAL_AUX_CAP_BITS				7
#define UP_CONVERSION_RATIO					2
#define RFPLL_VCOCAL_TARGETCOUNTBASE		0
#define RFPLL_VCOCAL_OFFSETIN				0
#define RFPLL_VCOCAL_SLOPEIN_DEC			0
#define RFPLL_VCOCAL_UPDATESEL_DEC			1
#define RFPLL_VCOCAL_CALCAPRBMODE_DEC			3
#define RFPLL_VCOCAL_FASTSWITCH_DEC				3
#define RFPLL_VCOCAL_FIXMSB_DEC					3
#define RFPLL_VCOCAL_FOURTHMESEN_DEC			1
#define RFPLL_VCOCAL_TESTVCOCNT_DEC				0
#define RFPLL_VCOCAL_FORCE_CAPS_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_CAPS_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FAST_SETTLE_OVR_DEC		0
#define RFPLL_VCOCAL_FAST_SETTLE_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_VCTRL_OVR_DEC		0
#define RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL_DEC		0
#define RFPLL_VCOCAL_TARGETCOUNTBASE_DEC		0
#define RFPLL_VCOCAL_COUPLINGMODE_DEC			1
#define RFPLL_VCOCAL_SWAPVCO12_DEC				0
#define RFPLL_VCOCAL_COUPLTHRES_DEC				6
#define RFPLL_VCOCAL_SECONDMESEN_DEC			3
#define RFPLL_VCOCAL_UPDATESELCOUP_DEC_20696	0
#define RFPLL_VCOCAL_COUPLINGIN_DEC				0
#define RFPLL_VCOCAL_FORCE_CAPS2_OVR_DEC		0
#define RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_AUX1_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_AUX1_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_AUX2_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_AUX2_OVRVAL_DEC		0
#define RFPLL_VCO_CAP_MODE_DEC					0
#define CAP_MULTIPLIER_RATIO_20696				8
#define CAP_MULTIPLIER_RATIO_PLUS_ONE_20696		9

/* PLL loop bw in kHz */
#define LOOP_BW						340
#define USE_DOUBLER					1
#define LOGEN_MODE					0
/* VCO_SELECT: 0->VCO1; 1->VCO2; 2->VCO1+VCO2 */
#define VCO_SELECT					2

/* No of fraction bits */
#define NF0		0
#define NF6		6
#define NF8		8
#define NF16	16
#define NF20	20
#define NF24	24
#define NF32	32

#define PRINT_PLL_CONFIG_20696(pll_struct, offset) \
	printf("%s = %u\n", #offset, (pll_struct->reg_field_val[IDX_20696_##offset]))

#define PLL_CONFIG_20696_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val[IDX_20696_##offset] = val

#define PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll_str, offset, regpfx, reg, fld) \
	pll_str->reg_addr[IDX_20696_##offset] = RADIO_PLLREG_20696(pi, reg); \
	pll_str->reg_field_mask[IDX_20696_##offset] = \
			RF_##20696##_##reg##_##fld##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift[IDX_20696_##offset] = \
			RF_##20696##_##reg##_##fld##_SHIFT(pi->pubpi->radiorev);

/* structure to hold computed PLL config values */
typedef struct {
	uint32 xtal_fx;
	uint32 loop_bw;
	uint8 use_doubler;
	uint8 logen_mode;
	uint8 vco_select;
	uint16 *reg_addr;
	uint16 *reg_field_mask;
	uint8 *reg_field_shift;
	uint16 *reg_field_val;
} pll_config_20696_tbl_t;

typedef enum {
	IDX_20696_OVR_RFPLL_VCOCAL_SLOPEIN,
	IDX_20696_OVR_RFPLL_VCOCAL_OFFSETIN,
	IDX_20696_OVR_RFPLL_VCOCAL_RST_N,
	IDX_20696_OVR_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20696_OVR_RFPLL_VCOCAL_XTALCOUNT,
	IDX_20696_OVR_RFPLL_VCOCAL_DELTAPLLVAL,
	IDX_20696_SEL_BAND,
	IDX_20696_OVR_MUX_XT_DEL,
	IDX_20696_OVR_MUX_SEL_BAND,
	IDX_20696_RFPLL_VCOCAL_DELAYEND,
	IDX_20696_RFPLL_VCOCAL_DELAYSTARTCOLD,
	IDX_20696_RFPLL_VCOCAL_DELAYSTARTWARM,
	IDX_20696_RFPLL_VCOCAL_XTALCOUNT,
	IDX_20696_RFPLL_VCOCAL_CALCAPRBMODE,
	IDX_20696_RFPLL_VCOCAL_DELTAPLLVAL,
	IDX_20696_RFPLL_VCOCAL_ERRORTHRES,
	IDX_20696_RFPLL_VCOCAL_INITCAPA,
	IDX_20696_RFPLL_VCOCAL_INITCAPB,
	IDX_20696_RFPLL_VCOCAL_NORMCOUNTLEFT,
	IDX_20696_RFPLL_VCOCAL_NORMCOUNTRIGHT,
	IDX_20696_RFPLL_VCOCAL_PAUSECNT,
	IDX_20696_RFPLL_VCOCAL_ROUNDLSB,
	IDX_20696_RFPLL_VCOCAL_UPDATESEL,
	IDX_20696_RFPLL_VCOCAL_TESTVCOCNT,
	IDX_20696_RFPLL_VCOCAL_FAST_SETTLE_OVR,
	IDX_20696_RFPLL_VCOCAL_FAST_SETTLE_OVRVAL,
	IDX_20696_RFPLL_VCOCAL_FORCE_VCTRL_OVR,
	IDX_20696_RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL,
	IDX_20696_RFPLL_VCOCAL_TARGETCOUNTBASE,
	IDX_20696_RFPLL_VCOCAL_TARGETCOUNTCENTER,
	IDX_20696_RFPLL_VCOCAL_OFFSETIN,
	IDX_20696_RFPLL_VCOCAL_SLOPEIN,
	IDX_20696_RFPLL_VCOCAL_FASTSWITCH,
	IDX_20696_RFPLL_VCOCAL_FIXMSB,
	IDX_20696_RFPLL_VCOCAL_FOURTHMESEN,
	IDX_20696_RFPLL_VCO_CAP_MODE,
	IDX_20696_RFPLL_VCOCAL_PLL_VAL,
	IDX_20696_RFPLL_VCOCAL_FORCE_CAPS_OVR,
	IDX_20696_RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
	IDX_20696_RFPLL_FRCT_WILD_BASE_HIGH,
	IDX_20696_RFPLL_FRCT_WILD_BASE_LOW,
	IDX_20696_RFPLL_VCO_CVAR,
	IDX_20696_RFPLL_VCO_ALC_REF_CTRL,
	IDX_20696_RFPLL_VCO_BIAS_MODE,
	IDX_20696_RFPLL_VCO_TEMPCO_DCADJ,
	IDX_20696_RFPLL_VCO_TEMPCO,
	IDX_20696_RFPLL_LF_LF_R2,
	IDX_20696_RFPLL_LF_LF_R3,
	IDX_20696_RFPLL_LF_LF_RS_CM,
	IDX_20696_RFPLL_LF_LF_RF_CM,
	IDX_20696_RFPLL_LF_LF_C1,
	IDX_20696_RFPLL_LF_LF_C2,
	IDX_20696_RFPLL_LF_LF_C3,
	IDX_20696_RFPLL_LF_LF_C4,
	IDX_20696_RFPLL_CP_IOFF,
	IDX_20696_RFPLL_CP_KPD_SCALE,
	IDX_20696_RFPLL_VCO_PREFER,
	IDX_20696_RFPLL_VCO_CORE1_EN,
	IDX_20696_RFPLL_VCO_CORE2_EN,
	IDX_20696_RFPLL_VCOCAL_ENABLECOUPLING,
	IDX_20696_RFPLL_VCOCAL_COUPLINGMODE,
	IDX_20696_RFPLL_VCOCAL_SWAPVCO12,
	IDX_20696_RFPLL_VCOCAL_MIDCODESEL,
	IDX_20696_RFPLL_VCOCAL_SECONDMESEN,
	IDX_20696_RFPLL_VCOCAL_UPDATESELCOUP,
	IDX_20696_RFPLL_VCOCAL_COUPLTHRES,
	IDX_20696_RFPLL_VCOCAL_COUPLTHRES2,
	IDX_20696_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20696_RFPLL_VCOCAL_FORCE_CAPS2_OVR,
	IDX_20696_RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
	IDX_20696_RFPLL_VCOCAL_FORCE_AUX1_OVR,
	IDX_20696_RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
	IDX_20696_RFPLL_VCOCAL_FORCE_AUX2_OVR,
	IDX_20696_RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
	PLL_CONFIG_20696_ARRAY_SIZE
} pll_config_20696_offset_t;

static void phy_ac_radio20696_write_pll_config(phy_info_t *pi, pll_config_20696_tbl_t *pll);
static void BCMATTACHFN(phy_ac_radio20696_pll_config_const_calc)(phy_info_t *pi,
		pll_config_20696_tbl_t *pll);
static void phy_ac_radio20696_pll_config_ch_dep_calc(phy_info_t *pi, uint32 lo_freq,
		uint8 ac_mode, pll_config_20696_tbl_t *pll);
#if DBG_PLL != 0
static void print_pll_config_20696(pll_config_20696_tbl_t *pll, uint32 lo_freq,
	uint32 loop_bw, uint32 icp_fx);
#endif // endif

/* Structure to hold 20696 PLL config values */
static pll_config_20696_tbl_t pll_conf_20696;

static pll_config_20696_tbl_t *
BCMRAMFN(wlc_phy_ac_get_20696_pll_config)(phy_info_t *pi)
{
	return &pll_conf_20696;
}

void
wlc_phy_radio20696_pll_tune(phy_info_t *pi, uint32 chan_freq)
{
	uint8 ac_mode;
	pll_config_20696_tbl_t *pll = wlc_phy_ac_get_20696_pll_config(pi);
	ac_mode = 1;
	phy_ac_radio20696_pll_config_ch_dep_calc(pi, chan_freq, ac_mode, pll);

	/* Write computed values to PLL registers */
	phy_ac_radio20696_write_pll_config(pi, pll);

}

void
BCMATTACHFN(phy_ac_radio20696_populate_pll_config_mfree)(phy_info_t *pi)
{
	pll_config_20696_tbl_t *pll = wlc_phy_ac_get_20696_pll_config(pi);

	if (pll->reg_addr != NULL) {
		phy_mfree(pi, pll->reg_addr, (sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE));
	}

	if (pll->reg_field_mask != NULL) {
		phy_mfree(pi, pll->reg_field_mask,
				(sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE));
	}

	if (pll->reg_field_shift != NULL) {
		phy_mfree(pi, pll->reg_field_shift,
				(sizeof(uint8) * PLL_CONFIG_20696_ARRAY_SIZE));
	}

	if (pll->reg_field_val != NULL) {
		phy_mfree(pi, pll->reg_field_val, (sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE));
	}
}

int
BCMATTACHFN(phy_ac_radio20696_populate_pll_config_tbl)(phy_info_t *pi)
{
	pll_config_20696_tbl_t *pll = wlc_phy_ac_get_20696_pll_config(pi);

	if ((pll->reg_addr =
		phy_malloc(pi, (sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_addr failed\n", __FUNCTION__));
		goto fail;
	}

	if ((pll->reg_field_mask =
		phy_malloc(pi, (sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_field_mask failed\n", __FUNCTION__));
		goto fail;
	}

	if ((pll->reg_field_shift =
		phy_malloc(pi, (sizeof(uint8) * PLL_CONFIG_20696_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_field_shift failed\n", __FUNCTION__));
		goto fail;
	}

	if ((pll->reg_field_val =
		phy_malloc(pi, (sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_field_val failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register addresses */
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_RFPLL_VCOCAL_SLOPEIN, RFP, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_slopeIn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_RFPLL_VCOCAL_OFFSETIN, RFP, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_offsetIn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_RFPLL_VCOCAL_RST_N, RFP, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_rst_n);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_RFPLL_VCOCAL_COUPLINGIN, RFP, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_couplingIn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_RFPLL_VCOCAL_XTALCOUNT, RFP, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_XtalCount);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_RFPLL_VCOCAL_DELTAPLLVAL, RFP, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_DeltaPllVal);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, SEL_BAND, RFP, PLL_MUXSELECT_LINE,
			sel_band);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_MUX_XT_DEL, RFP, PLL_MUXSELECT_OVR,
			ovr_mux_xt_del);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, OVR_MUX_SEL_BAND, RFP, PLL_MUXSELECT_OVR,
			ovr_mux_sel_band);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_DELAYEND, RFP, PLL_VCOCAL18,
			rfpll_vcocal_DelayEnd);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_DELAYSTARTCOLD, RFP, PLL_VCOCAL3,
			rfpll_vcocal_DelayStartCold);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_DELAYSTARTWARM, RFP, PLL_VCOCAL4,
			rfpll_vcocal_DelayStartWarm);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_XTALCOUNT, RFP, PLL_VCOCAL7,
			rfpll_vcocal_XtalCount);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_CALCAPRBMODE, RFP, PLL_VCOCAL7,
			rfpll_vcocal_CalCapRBMode);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_DELTAPLLVAL, RFP, PLL_VCOCAL8,
			rfpll_vcocal_DeltaPllVal);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_ERRORTHRES, RFP, PLL_VCOCAL20,
			rfpll_vcocal_ErrorThres);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_INITCAPA, RFP, PLL_VCOCAL12,
			rfpll_vcocal_InitCapA);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_INITCAPB, RFP, PLL_VCOCAL13,
			rfpll_vcocal_InitCapB);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_NORMCOUNTLEFT, RFP, PLL_VCOCAL10,
			rfpll_vcocal_NormCountLeft);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_NORMCOUNTRIGHT, RFP, PLL_VCOCAL11,
			rfpll_vcocal_NormCountRight);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_PAUSECNT, RFP, PLL_VCOCAL19,
			rfpll_vcocal_PauseCnt);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_ROUNDLSB, RFP, PLL_VCOCAL1,
			rfpll_vcocal_RoundLSB);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_UPDATESEL, RFP, PLL_VCOCAL1,
			rfpll_vcocal_UpdateSel);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_TESTVCOCNT, RFP, PLL_VCOCAL1,
			rfpll_vcocal_testVcoCnt);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FAST_SETTLE_OVR, RFP, PLL_VCOCAL1,
			rfpll_vcocal_fast_settle_ovr);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FAST_SETTLE_OVRVAL, RFP, PLL_VCOCAL1,
			rfpll_vcocal_fast_settle_ovrVal);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_VCTRL_OVR, RFP, PLL_VCOCAL1,
			rfpll_vcocal_force_vctrl_ovr);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL, RFP, PLL_VCOCAL1,
			rfpll_vcocal_force_vctrl_ovrVal);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_TARGETCOUNTBASE, RFP, PLL_VCOCAL6,
			rfpll_vcocal_TargetCountBase);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_TARGETCOUNTCENTER, RFP, PLL_VCOCAL9,
			rfpll_vcocal_TargetCountCenter);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_OFFSETIN, RFP, PLL_VCOCAL17,
			rfpll_vcocal_offsetIn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_SLOPEIN, RFP, PLL_VCOCAL15,
			rfpll_vcocal_slopeIn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FASTSWITCH, RFP, PLL_VCOCAL1,
			rfpll_vcocal_FastSwitch);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FIXMSB, RFP, PLL_VCOCAL1,
			rfpll_vcocal_FixMSB);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FOURTHMESEN, RFP, PLL_VCOCAL1,
			rfpll_vcocal_FourthMesEn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_CAP_MODE, RFP, PLL_VCO2,
			rfpll_vco_cap_mode);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_PLL_VAL, RFP, PLL_VCOCAL5,
			rfpll_vcocal_pll_val);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_CAPS_OVR, RFP, PLL_VCOCAL2,
			rfpll_vcocal_force_caps_ovr);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_CAPS_OVRVAL, RFP, PLL_VCOCAL2,
			rfpll_vcocal_force_caps_ovrVal);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_FRCT_WILD_BASE_HIGH, RFP, PLL_FRCT2,
			rfpll_frct_wild_base_high);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_FRCT_WILD_BASE_LOW, RFP, PLL_FRCT3,
			rfpll_frct_wild_base_low);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_CVAR, RFP, PLL_VCO2,
			rfpll_vco_cvar);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_ALC_REF_CTRL, RFP, PLL_VCO6,
			rfpll_vco_ALC_ref_ctrl);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_BIAS_MODE, RFP, PLL_VCO6,
			rfpll_vco_bias_mode);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_TEMPCO_DCADJ, RFP, PLL_VCO5,
			rfpll_vco_tempco_dcadj);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_TEMPCO, RFP, PLL_VCO4,
			rfpll_vco_tempco);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_R2, RFP,
		PLL_LF4, rfpll_lf_lf_r2);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_R3, RFP,
		PLL_LF5, rfpll_lf_lf_r3);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_RS_CM, RFP,
		PLL_LF7, rfpll_lf_lf_rs_cm);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_RF_CM, RFP,
		PLL_LF7, rfpll_lf_lf_rf_cm);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_C1, RFP,
		PLL_LF2, rfpll_lf_lf_c1);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_C2, RFP,
		PLL_LF2, rfpll_lf_lf_c2);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_C3, RFP,
		PLL_LF3, rfpll_lf_lf_c3);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_LF_LF_C4, RFP,
		PLL_LF3, rfpll_lf_lf_c4);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_CP_IOFF, RFP,
		PLL_CP4, rfpll_cp_ioff);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_CP_KPD_SCALE, RFP,
		PLL_CP4, rfpll_cp_kpd_scale);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_PREFER, RFP, PLL_VCO7,
		rfpll_vco_prefer);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_CORE1_EN, RFP, PLL_VCO7,
		rfpll_vco_core1_en);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCO_CORE2_EN, RFP, PLL_VCO7,
		rfpll_vco_core2_en);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_ENABLECOUPLING,
		RFP, PLL_VCOCAL24, rfpll_vcocal_enableCoupling);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_COUPLINGMODE,
		RFP, PLL_VCOCAL24, rfpll_vcocal_couplingMode);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_SWAPVCO12,
		RFP, PLL_VCOCAL24, rfpll_vcocal_swapVco12);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_MIDCODESEL,
		RFP, PLL_VCOCAL24, rfpll_vcocal_MidCodeSel);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_SECONDMESEN,
		RFP, PLL_VCOCAL24, rfpll_vcocal_SecondMesEn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_UPDATESELCOUP,
		RFP, PLL_VCOCAL24, rfpll_vcocal_UpdateSelCoup);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_COUPLTHRES,
		RFP, PLL_VCOCAL26, rfpll_vcocal_CouplThres);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_COUPLTHRES2,
		RFP, PLL_VCOCAL26, rfpll_vcocal_CouplThres2);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_COUPLINGIN,
		RFP, PLL_VCOCAL25, rfpll_vcocal_couplingIn);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_CAPS2_OVR,
		RFP, PLL_VCOCAL21, rfpll_vcocal_force_caps2_ovr);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
		RFP, PLL_VCOCAL21, rfpll_vcocal_force_caps2_ovrVal);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_AUX1_OVR,
		RFP, PLL_VCOCAL22, rfpll_vcocal_force_aux1_ovr);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
		RFP, PLL_VCOCAL22, rfpll_vcocal_force_aux1_ovrVal);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_AUX2_OVR,
		RFP, PLL_VCOCAL23, rfpll_vcocal_force_aux2_ovr);
	PLL_CONFIG_20696_REG_INFO_ENTRY(pi, pll, RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
		RFP, PLL_VCOCAL23, rfpll_vcocal_force_aux2_ovrVal);

	/* Add frequency independent data */
	phy_ac_radio20696_pll_config_const_calc(pi, pll);

	return BCME_OK;

fail:
	if (pll->reg_addr != NULL) {
		phy_mfree(pi, pll->reg_addr, (sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE));
	}

	if (pll->reg_field_mask != NULL) {
		phy_mfree(pi, pll->reg_field_mask,
				(sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE));
	}

	if (pll->reg_field_shift != NULL) {
		phy_mfree(pi, pll->reg_field_shift,
				(sizeof(uint8) * PLL_CONFIG_20696_ARRAY_SIZE));
	}

	if (pll->reg_field_val != NULL) {
		phy_mfree(pi, pll->reg_field_val, (sizeof(uint16) * PLL_CONFIG_20696_ARRAY_SIZE));
	}

	return BCME_NOMEM;
}

static void
phy_ac_radio20696_write_pll_config(phy_info_t *pi, pll_config_20696_tbl_t *pll)
{
	uint16 *reg_val;
	uint16 *reg_addr;
	uint16 *reg_mask;
	uint8 *reg_shift;
	uint8 i;

	reg_val = pll->reg_field_val;
	reg_addr = pll->reg_addr;
	reg_mask = pll->reg_field_mask;
	reg_shift = pll->reg_field_shift;

	/* Write to register fields */
	for (i = 0; i < PLL_CONFIG_20696_ARRAY_SIZE; i++) {
	  phy_utils_mod_radioreg(pi, reg_addr[i], reg_mask[i], (reg_val[i] << reg_shift[i]));
	}
}

static void
BCMATTACHFN(phy_ac_radio20696_pll_config_const_calc)(phy_info_t *pi, pll_config_20696_tbl_t *pll)
{
	uint32 xtal_freq;
	uint32 xtal_fx;
	uint64 temp_64;
	uint8 nf;

	/* Default parameters to be used in chan dependent computation */
	pll->use_doubler = USE_DOUBLER;
	pll->loop_bw = LOOP_BW;
	pll->logen_mode = LOGEN_MODE;
	pll->vco_select = VCO_SELECT;

	/* ------------------------------------------------------------------- */
	/* XTAL frequency in 1.7.24 */
	xtal_freq = pi->xtalfreq;
	/* <0.32.0> / <0.32.0> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(xtal_freq, 1000000, 0, 0, &xtal_fx);
	/* round(<0.(32-nf).nf>, (nf-24)) -> <0.8.24> */
	pll->xtal_fx = math_fp_round_32(xtal_fx, (nf - NF24));
	xtal_fx = pll->xtal_fx;
	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	/* XTAL dependent */
	/* <0.8.24> * <0.16.16> -> <0.16.16> */
	temp_64 = math_fp_mult_64(xtal_fx, RFPLL_VCOCAL_DELAYEND_FX, NF24, NF16, NF16);
	/* round(0.16.16, 16) -> <0.32.0> */
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_DELAYEND,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* <0.8.24> * <0.16.16> -> <0.16.16> */
	temp_64 = math_fp_mult_64(xtal_fx, RFPLL_VCOCAL_DELAYSTARTCOLD_FX, NF24, NF16, NF16);
	/* round(0.16.16, 16) -> <0.32.0> */
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_DELAYSTARTCOLD,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* <0.8.24> * <0.16.16> -> <0.16.16> */
	temp_64 = math_fp_mult_64(xtal_fx, RFPLL_VCOCAL_DELAYSTARTWARM_FX, NF24, NF16, NF16);
	/* round(0.16.16, 16) -> <0.32.0> */
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_DELAYSTARTWARM,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* <0.8.24> * <0.16.16> -> <0.16.16> */
	temp_64 = math_fp_mult_64(xtal_fx, RFPLL_VCOCAL_PAUSECNT_FX, NF24, NF16, NF16);
	/* round(0.16.16, 16) -> <0.32.0> */
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_PAUSECNT,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* ------------------------------------------------------------------- */
	/* Put other register values to structure */
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_SLOPEIN, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_OFFSETIN, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_RST_N, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_COUPLINGIN, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_XTALCOUNT, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_DELTAPLLVAL, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, SEL_BAND, 3);
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_MUX_XT_DEL, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, OVR_MUX_SEL_BAND, 1);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_TARGETCOUNTBASE, RFPLL_VCOCAL_TARGETCOUNTBASE);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_OFFSETIN, RFPLL_VCOCAL_OFFSETIN);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_SLOPEIN, RFPLL_VCOCAL_SLOPEIN_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_UPDATESEL, RFPLL_VCOCAL_UPDATESEL_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_CALCAPRBMODE, RFPLL_VCOCAL_CALCAPRBMODE_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FASTSWITCH, RFPLL_VCOCAL_FASTSWITCH_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FIXMSB, RFPLL_VCOCAL_FIXMSB_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FOURTHMESEN, RFPLL_VCOCAL_FOURTHMESEN_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_TESTVCOCNT, RFPLL_VCOCAL_TESTVCOCNT_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS_OVR,
		RFPLL_VCOCAL_FORCE_CAPS_OVR_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
		RFPLL_VCOCAL_FORCE_CAPS_OVRVAL_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FAST_SETTLE_OVR,
		RFPLL_VCOCAL_FAST_SETTLE_OVR_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FAST_SETTLE_OVRVAL,
		RFPLL_VCOCAL_FAST_SETTLE_OVRVAL_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVR,
		RFPLL_VCOCAL_FORCE_VCTRL_OVR_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL,
		RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_TARGETCOUNTBASE,
		RFPLL_VCOCAL_TARGETCOUNTBASE_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLINGMODE,
		RFPLL_VCOCAL_COUPLINGMODE_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_SWAPVCO12,
		RFPLL_VCOCAL_SWAPVCO12_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLTHRES,
		RFPLL_VCOCAL_COUPLTHRES_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_SECONDMESEN,
		RFPLL_VCOCAL_SECONDMESEN_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_UPDATESELCOUP,
		RFPLL_VCOCAL_UPDATESELCOUP_DEC_20696);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLINGIN,
		RFPLL_VCOCAL_COUPLINGIN_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVR,
		RFPLL_VCOCAL_FORCE_CAPS2_OVR_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
		RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX1_OVR,
		RFPLL_VCOCAL_FORCE_AUX1_OVR_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
		RFPLL_VCOCAL_FORCE_AUX1_OVRVAL_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX2_OVR,
		RFPLL_VCOCAL_FORCE_AUX2_OVR_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
		RFPLL_VCOCAL_FORCE_AUX2_OVRVAL_DEC);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_CAP_MODE, RFPLL_VCO_CAP_MODE_DEC);
}

static void
phy_ac_radio20696_pll_config_ch_dep_calc(phy_info_t *pi, uint32 lo_freq,
	uint8 ac_mode, pll_config_20696_tbl_t *pll)
{
	uint32 icp_fx = ICP_FX_AC0;
	uint32 pfd_ref_freq_fx;
	uint8 nf;
	uint64 temp_64;
	uint32 temp_32;
	uint32 temp_32_1;
	uint32 vco_freq;
	uint32 divide_ratio_fx;
	uint8 enable_coupled_VCO;
	uint8 select_VCO;
	uint8 rfpll_vcocal_enablecoupling_dec;
	uint32 lo_div_vco_ratio_fx;
	uint32 rfpll_vcocal_XtalCount_raw_fx;
	uint32 rfpll_vcocal_TargetCountCenter = 0;
	uint32 rfpll_frct_wild_base_dec_fx;
	uint32 wn_fx;
	uint32 loop_band;
	uint32 loop_band_square_fx;
	uint32 c1_passive_lf_fx;
	uint32 c1_cap_multiplier_mode_fx;
	uint32 i_off_fx;
	uint64 r1_passive_lf_fx;
	uint64 rf_fx;
	uint64 rs_fx;
	uint32 rfpll_lf_lf_r2_dec_fx;
	uint64 rfpll_lf_lf_rs_cm_ref_fx;
	uint32 rfpll_lf_lf_rs_cm_dec_fx;
	uint64 rfpll_lf_lf_rf_cm_ref_fx;
	uint32 rfpll_lf_lf_rf_cm_dec_fx;
	uint32 rfpll_lf_lf_c1_ref_fx;
	uint32 rfpll_lf_lf_c1_dec_fx;
	uint32 rfpll_lf_lf_c2_dec_fx;
	uint32 rfpll_lf_lf_c2_ref_fx;
	uint32 rfpll_lf_lf_c3_ref_fx;
	uint32 rfpll_lf_lf_c3_dec_fx;
	uint32 rfpll_lf_lf_c4_ref_fx;
	uint32 rfpll_lf_lf_c4_dec_fx;
	uint32 rfpll_cp_kpd_scale_ref_fx;
	uint32 rfpll_cp_kpd_scale_dec_fx;
	uint32 rfpll_cp_ioff_dec_fx;
	int64  temp_sign;
	uint16 rfpll_vcocal_CouplThres2_dec = 0;
	uint16 rfpll_vco_prefer_dec = 0;
	uint16 rfpll_vco_core1_en_dec = 0;
	uint16 rfpll_vco_core2_en_dec = 0;
	uint16 rfpll_vco_cvar_dec = 0;
	uint16 rfpll_vco_ALC_ref_ctrl_dec = 0;
	uint16 rfpll_vco_bias_mode_dec = 0;
	uint16 rfpll_vco_tempco_dcadj_dec = 0;
	uint16 rfpll_vco_tempco_dec = 0;
	uint16 rfpll_vcocal_XtalCount_dec;
	uint16 rfpll_vcocal_DeltaPllVal_dec;
	uint16 rfpll_vcocal_ErrorThres = 0;
	uint16 rfpll_vcocal_InitCapA = 0;
	uint16 rfpll_vcocal_InitCapB = 0;
	uint16 rfpll_vcocal_NormCountLeft;
	uint16 rfpll_vcocal_NormCountRight;
	uint16 rfpll_vcocal_TargetCountCenter_dec;
	uint16 rfpll_vcocal_MidCodeSel_dec;
	uint16 rfpll_lf_lf_r2_ideal = 0;
	uint16 rfpll_lf_lf_rs_cm_ideal = 0;
	uint16 rfpll_lf_lf_rf_cm_ideal = 0;
	uint16 rfpll_lf_lf_c1_ideal = 0;
	uint16 rfpll_lf_lf_c2_ideal = 0;
	uint16 rfpll_lf_lf_c3_ideal = 0;
	uint16 rfpll_lf_lf_c4_ideal = 0;
	uint16 rfpll_cp_kpd_scale_ideal = 0;
	uint16 rfpll_cp_ioff_ideal = 0;

	/* ------------------------------------------------------------------- */
	/* PFD Reference Frequency */
	/* <0.8.24> * <0.32.0> --> <0.8.24> */
	pfd_ref_freq_fx = pll->xtal_fx << pll->use_doubler;
	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	/* VCO Frequency */
	/* In 2G regular mode. VCO = LO x 3/2, In 2G RSDB mode(some channels) */
	/* VCO = LO x 4/3 to avoid VCO pulling #C14 */
	/* In 5G, VCO  = LO x 2/3  #C15 */
	/* lo_freq #C9,	## logen_mode = C14  */
	if (lo_freq <= 3000 && pll->logen_mode == 0) {
		/* <0.32.0> * <0.32.0> --> <0.32.0> */
		vco_freq = (uint32)math_fp_mult_64(lo_freq, 3, NF0, NF0, NF0);
		/* <0.32.0> / <0.32.0> --> <0.(32 - nf).nf> */
		nf = math_fp_div_64(vco_freq, 2, NF0, NF0, &vco_freq);
		/* floor(<0.(32-nf).nf>, (nf-16)) -> 0.16.16 */
		vco_freq = math_fp_floor_32(vco_freq, (nf - NF16));
	} else if (lo_freq <= 3000 && pll->logen_mode == 1) {
		/* <0.32.0> * <0.32.0> --> <0.32.0> */
		vco_freq = (uint32)math_fp_mult_64(lo_freq, 4, NF0, NF0, NF0);
		/* <0.32.0> / <0.32.0> --> <0.(32 - nf).nf> */
		nf = math_fp_div_64(vco_freq, 3, NF0, NF0, &vco_freq);
		/* floor(<0.(32-nf).nf>, (nf-16)) -> 0.16.16 */
		vco_freq = math_fp_floor_32(vco_freq, (nf - NF16));
	} else {
		/* <0.32.0> / <0.32.0> --> <0.(32 - nf).nf> */
		nf = math_fp_div_64((lo_freq << 1), 3, NF0, NF0, &vco_freq);
		/* floor(<0.(32-nf).nf>, (nf-16)) -> 0.16.16 */
		vco_freq = math_fp_floor_32(vco_freq, (nf - NF16));
	}
	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	if (pll->vco_select == 0) {
		enable_coupled_VCO = 0;
		select_VCO = 0;
	} else if (pll->vco_select == 1) {
		enable_coupled_VCO = 0;
		select_VCO = 1;
	} else if (pll->vco_select == 2) {
		enable_coupled_VCO = 1;
		select_VCO = 0;
	} else {
		PHY_ERROR(("wl%d: %s: vco_select arg must be 0(VCO1),1(VCO2) or 2(VCO1+VCO2)\n",
			pi->sh->unit, __FUNCTION__));
		ASSERT(0);
		return;
	}
	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	/* VCO Frequency dependent calculations */
	/* <0.16.16> / <0.8.24> --> <0.(32 - nf).nf> */
	nf = math_fp_div_64(vco_freq, pfd_ref_freq_fx, NF16, NF24, &divide_ratio_fx);
	/* floor(<0.(32-nf).nf>, (nf-20)) -> 0.12.20 */
	divide_ratio_fx = math_fp_floor_32(divide_ratio_fx, (nf - NF20));

	/* <0.16.16> / <0.16.16> --> <0.(32 - nf).nf> */
	nf = math_fp_div_64(lo_freq, vco_freq, NF0, NF16, &lo_div_vco_ratio_fx);
	/* floor(<0.(32-nf).nf>, (nf-16)) -> 0.16.16 */
	lo_div_vco_ratio_fx = math_fp_floor_32(lo_div_vco_ratio_fx, (nf - NF16));

	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	/* Defines the duration of time that vcocal should wait after it applied a
	   new calibration bit to to vco before it starts to measure vco frequency
	   to calculate the next bit. This time is required by vco to settle.
	*/
	/* <0.16.16> * <0.8.24> --> <0.8.16> */
	rfpll_vcocal_XtalCount_raw_fx = (uint32)math_fp_mult_64(lo_div_vco_ratio_fx << 1,
		pll->xtal_fx, NF16, NF24, NF16);

	/* ceil(<0.8.16>, 16) -> <0.8.0> */
	rfpll_vcocal_XtalCount_dec = (uint16)math_fp_ceil_64(rfpll_vcocal_XtalCount_raw_fx, NF16);

	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_XTALCOUNT,
		rfpll_vcocal_XtalCount_dec);

	/* <0.32.0> / <0.16.16> --> <0.(32 - nf).nf> */
	nf = math_fp_div_64(rfpll_vcocal_XtalCount_dec, rfpll_vcocal_XtalCount_raw_fx,
			NF0, NF16, &temp_32);
	/* round(<0.(32-nf).nf>, (nf-16)) -> 0.16.16 */
	temp_32 = math_fp_round_32(temp_32, (nf - NF16));
	/* <0.16.16> - <0.16.16> -> <0.16.16> */

	temp_32	= temp_32 - FX_ONE;
	/* round(<0.16.16> * <0.32.0> ,16) -> <0.32.0> */
	rfpll_vcocal_DeltaPllVal_dec = (uint16)math_fp_round_64((temp_32 * (1 << 13)), 16);

	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_DELTAPLLVAL,
		rfpll_vcocal_DeltaPllVal_dec);

	/* Frequency range dependent constants */
	if (lo_freq >= 2000 && lo_freq <= 3000) {
		if (pll->logen_mode == 0) {
			rfpll_vcocal_InitCapA = 1536;
			rfpll_vcocal_InitCapB = 1792;
		} else {
			rfpll_vcocal_InitCapA = 2944;
			rfpll_vcocal_InitCapB = 3200;
		}
		rfpll_vcocal_ErrorThres  = 3;
		rfpll_vcocal_TargetCountCenter  = 2442;
	} else if  (lo_freq < 5250) {
		rfpll_vcocal_InitCapA  = 2432;
		rfpll_vcocal_InitCapB  = 2688;
		rfpll_vcocal_ErrorThres = 6;
		rfpll_vcocal_TargetCountCenter = 5070;
	} else if (lo_freq >= 5250 && lo_freq < 5500) {
		rfpll_vcocal_InitCapA = 1792;
		rfpll_vcocal_InitCapB = 2048;
		rfpll_vcocal_ErrorThres = 5;
		rfpll_vcocal_TargetCountCenter = 5370;
	} else if (lo_freq >= 5500) {
		rfpll_vcocal_InitCapA = 1088;
		rfpll_vcocal_InitCapB = 1344;
		rfpll_vcocal_ErrorThres = 9;
		rfpll_vcocal_TargetCountCenter = 5700;
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_ERRORTHRES, rfpll_vcocal_ErrorThres);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_INITCAPA, rfpll_vcocal_InitCapA);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_INITCAPB, rfpll_vcocal_InitCapB);

	if (lo_freq >= 2000 && lo_freq <= 3000) {
		rfpll_vcocal_NormCountLeft = -39;
		rfpll_vcocal_NormCountRight = 39;
		rfpll_vcocal_CouplThres2_dec = 25;
	} else {
		rfpll_vcocal_NormCountLeft = -50;
		rfpll_vcocal_NormCountRight = 50;
		rfpll_vcocal_CouplThres2_dec = 50;
	}

	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_NORMCOUNTLEFT,
			rfpll_vcocal_NormCountLeft);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_NORMCOUNTRIGHT,
			rfpll_vcocal_NormCountRight);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLTHRES2,
			rfpll_vcocal_CouplThres2_dec);

	/* <0.32.0> - <0.32.0> -> <0.32.0> */
	// pll->rfpll_vcocal_RoundLSB = 12 - VCO_CAL_CAP_BITS;
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_ROUNDLSB, (12 - VCO_CAL_CAP_BITS));

	/* <0.32.0> * <0.32.0> -> <0.32.0> */
	temp_64 = rfpll_vcocal_TargetCountCenter * rfpll_vcocal_DeltaPllVal_dec;
	/* <0.32.0> + round(<0.32.0> * <0.16.16>, 16) -> <0.32.0> */
	rfpll_vcocal_TargetCountCenter_dec = (uint16)(rfpll_vcocal_TargetCountCenter +
			math_fp_round_64((temp_64 * FIX_2POW_MIN13), NF16));
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_TARGETCOUNTCENTER,
			rfpll_vcocal_TargetCountCenter_dec);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_PLL_VAL, (uint16)lo_freq);

	if (enable_coupled_VCO == 1) {
		rfpll_vcocal_enablecoupling_dec = 1;
	} else {
		rfpll_vcocal_enablecoupling_dec = 0;
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_ENABLECOUPLING,
		rfpll_vcocal_enablecoupling_dec);

	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	/* Other VCOCAL register (not channel/XTAL dependent) */
	if (RFPLL_VCOCAL_COUPLINGMODE_DEC == 0) {
		if ((VCO_CAL_AUX_CAP_BITS < 6) || (VCO_CAL_AUX_CAP_BITS > 9)) {
			PHY_ERROR(("wl%d: %s: Unsupported value VCO_cal_aux_cap_bits\n",
				pi->sh->unit, __FUNCTION__));
			ASSERT(0);
			return;
		}

		rfpll_vcocal_MidCodeSel_dec = VCO_CAL_AUX_CAP_BITS - 6;
	} else {
		if ((VCO_CAL_AUX_CAP_BITS < 4) || (VCO_CAL_AUX_CAP_BITS > 7)) {
			PHY_ERROR(("wl%d: %s: Unsupported value VCO_cal_aux_cap_bits\n",
				pi->sh->unit, __FUNCTION__));
			ASSERT(0);
			return;
		}

		rfpll_vcocal_MidCodeSel_dec = VCO_CAL_AUX_CAP_BITS - 4;
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCOCAL_MIDCODESEL, rfpll_vcocal_MidCodeSel_dec);
	/* ------------------------------------------------------------------- */
	/* VCO register */
	/* ------------------------------------------------------------------- */
	if (enable_coupled_VCO == 1) {
		rfpll_vco_core1_en_dec = 1;
		rfpll_vco_core2_en_dec = 1;
		rfpll_vco_prefer_dec = select_VCO;
	} else if (select_VCO == 0) {
		rfpll_vco_core1_en_dec = 1;
		rfpll_vco_core2_en_dec = 0;
		rfpll_vco_prefer_dec = 0;
	} else if (select_VCO == 1) {
		rfpll_vco_core1_en_dec = 0;
		rfpll_vco_core2_en_dec = 1;
		rfpll_vco_prefer_dec = 0;
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_PREFER, rfpll_vco_prefer_dec);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_CORE1_EN, rfpll_vco_core1_en_dec);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_CORE2_EN, rfpll_vco_core2_en_dec);

	/* Fract-N (Sigma-Delta) register */
	rfpll_frct_wild_base_dec_fx = divide_ratio_fx;
	temp_32 = rfpll_frct_wild_base_dec_fx >> 16;
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_FRCT_WILD_BASE_HIGH,
			(uint16)(temp_32 & 0xFFFF));
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_FRCT_WILD_BASE_LOW,
			(uint16)(rfpll_frct_wild_base_dec_fx & 0xFFFF));

	/* ------------------------------------------------------------------- */
	/* VCO register (11n vs 11ac mode) */
	/* ------------------------------------------------------------------- */
	/* There is no Class-C VCO anymore. Always CMOS VCO */
	/* 2 VCO mode: 11abgn, 11ac */
	if (ac_mode == 0) {
		rfpll_vco_cvar_dec = 8;
		rfpll_vco_ALC_ref_ctrl_dec = 8;
		rfpll_vco_bias_mode_dec = 0;
		rfpll_vco_tempco_dcadj_dec = 1;
		rfpll_vco_tempco_dec = 3;
		icp_fx = ICP_FX_AC0;
	} else if (ac_mode == 1) {
		rfpll_vco_cvar_dec = 8;
		rfpll_vco_ALC_ref_ctrl_dec = 9;
		rfpll_vco_bias_mode_dec = 1;
		rfpll_vco_tempco_dcadj_dec = 3;
		rfpll_vco_tempco_dec = 5;
		icp_fx = ICP_FX_AC1;
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_CVAR, rfpll_vco_cvar_dec);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_ALC_REF_CTRL, rfpll_vco_ALC_ref_ctrl_dec);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_BIAS_MODE, rfpll_vco_bias_mode_dec);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_TEMPCO_DCADJ, rfpll_vco_tempco_dcadj_dec);
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_VCO_TEMPCO, rfpll_vco_tempco_dec);

	/* ----------------------------- */
	/* CP and Loop Filter Registers  */
	/* ----------------------------- */
	loop_band = LOOP_BW;
	/* <0.24.8> * <0.32.0> -> <0.26.6> */
	wn_fx = (uint32)math_fp_mult_64(WN_CONST_FX, loop_band, NF8, NF0, NF6);
	/* <0.32.0> * <0.32.0> -> <0.32.0> */
	loop_band_square_fx = loop_band * loop_band;

	/* <0.12.20> * <0.32.0> -> <0.26.6> */
	temp_32 = (uint32)math_fp_mult_64(divide_ratio_fx, loop_band_square_fx, NF20, NF0, NF6);
	/* <0.32.32> / <0.26.6> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(C1_PASSIVE_LF_CONST_FX, temp_32, NF16, NF6, &temp_32_1);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	temp_32_1 = math_fp_round_32(temp_32_1, (nf - NF16));
	c1_passive_lf_fx = (uint32)math_fp_mult_64(temp_32_1, icp_fx, NF16, NF16, NF16);
	/* <0.16.16> / <0.32.0> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(c1_passive_lf_fx, CAP_MULTIPLIER_RATIO_PLUS_ONE_20696, NF16, NF0,
			&c1_cap_multiplier_mode_fx);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	c1_cap_multiplier_mode_fx = math_fp_round_32(c1_cap_multiplier_mode_fx, (nf - NF16));

	/* <0.16.16> * <0.8.24> -> <0.16.16> */
	temp_32 = (uint32)math_fp_mult_64(T_OFF_FX, pfd_ref_freq_fx, NF16, NF24, NF16);
	/* <0.16.16> * <0.16.16> -> <0.16.16> */
	i_off_fx = (uint32)math_fp_mult_64(temp_32, IOFF_REF_CONST_FX, NF16, NF16, NF16);

	/* <0.26.6> * <0.0.32> -> <0.16.16> */
	temp_64 = math_fp_mult_64(wn_fx, R1_PASSIVE_LF_CONST_FX, NF6, NF32, NF16);
	/* <0.12.20> / <0.16.16> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(divide_ratio_fx, icp_fx, NF20, NF16, &temp_32);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	temp_32 = math_fp_round_32(temp_32, (nf - NF16));
	/* <0.16.16> / <0.32.0> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(temp_32, 1000, NF16, NF0, &temp_32_1);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	temp_32_1 = math_fp_round_32(temp_32_1, (nf - NF16));
	/* <0.16.16> * <0.16.16> -> <0.16.16> */
	r1_passive_lf_fx = math_fp_mult_64(temp_64, temp_32_1, NF16, NF16, NF16);
	/* <0.16.16> * <0.16.16> -> <0.16.16> */
	rf_fx = math_fp_mult_64(r1_passive_lf_fx, RF_CONST1_FX, NF16, NF16, NF16);
	/* <0.32.0> * <0.16.16> -> <0.16.16> */
	rs_fx = CAP_MULTIPLIER_RATIO_20696 * rf_fx;
	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	/* Register Definitions (lower) */
	/* rfpll_lf_lf_r2 */
	/* <0.16.16> - <0.16.16> -> <0.16.16> */
	temp_sign = (int64)(r1_passive_lf_fx - (int64)CONST_257_18_FX);
	if (temp_sign > 0) {
		/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
		nf = math_fp_div_64((uint64)temp_sign, CONST_281_21_FX, NF16, NF16, &temp_32);
		/* round(<0.(32-nf).nf>, nf) -> <0.32.0> */
		rfpll_lf_lf_r2_dec_fx = math_fp_round_32(temp_32, nf);
		rfpll_lf_lf_r2_ideal = LIMIT((int32)rfpll_lf_lf_r2_dec_fx, 0, 255);
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_R2, rfpll_lf_lf_r2_ideal);

	/* rfpll_lf_lf_r3 */
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_R3, rfpll_lf_lf_r2_ideal);

	/* rfpll_lf_lf_rs_cm */
	rfpll_lf_lf_rs_cm_ref_fx = rs_fx;   /* cap_multiplier_PU = 1 */
	/* <0.16.16> - <0.16.16> -> <0.16.16> */
	temp_sign = (int64)((int64)rfpll_lf_lf_rs_cm_ref_fx - (int64)CONST_257_18_FX);
	if (temp_sign > 0) {
		/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
		nf = math_fp_div_64((uint64)temp_sign, CONST_281_21_FX, NF16, NF16, &temp_32);
		/* round(<0.(32-nf).nf>, nf) -> <0.32.0> */
		rfpll_lf_lf_rs_cm_dec_fx = math_fp_round_32(temp_32, nf);
		rfpll_lf_lf_rs_cm_ideal = (uint16)(LIMIT((int32)rfpll_lf_lf_rs_cm_dec_fx,
			0, 255));
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_RS_CM, rfpll_lf_lf_rs_cm_ideal);

	/* rfpll_lf_lf_rf_cm */
	rfpll_lf_lf_rf_cm_ref_fx = rf_fx;  /* cap_multiplier_PU = 1 */
	/* <0.16.16> - <0.16.16> -> <0.16.16> */
	temp_sign = (int64)((int64)rfpll_lf_lf_rf_cm_ref_fx - (int64)CONST_257_18_FX);
	if (temp_sign > 0) {
		/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
		nf = math_fp_div_64((uint64)temp_sign, CONST_281_21_FX, NF16, NF16, &temp_32);
		/* round(<0.(32-nf).nf>, nf) -> <0.32.0> */
		rfpll_lf_lf_rf_cm_dec_fx = math_fp_round_32(temp_32, nf);
		rfpll_lf_lf_rf_cm_ideal = (uint16)(LIMIT((int32)rfpll_lf_lf_rf_cm_dec_fx,
				0, 255));
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_RF_CM, rfpll_lf_lf_rf_cm_ideal);

	/* rfpll_lf_lf_c1 */
	rfpll_lf_lf_c1_ref_fx  = c1_cap_multiplier_mode_fx;
	/* <0.16.16> - <0.16.16> -> <0.16.16> */
	temp_sign = (int64)((int64)rfpll_lf_lf_c1_ref_fx - (int64)CONST_3P2619_FX);
	if (temp_sign > 0) {
		/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
		nf = math_fp_div_64((uint64)temp_sign, CONST_P5041_FX, NF16, NF16, &temp_32);
		/* round(<0.(32-nf).nf>, nf) -> <0.32.0> */
		rfpll_lf_lf_c1_dec_fx = math_fp_round_32(temp_32, nf);
		rfpll_lf_lf_c1_ideal = (uint16)(LIMIT((int32)rfpll_lf_lf_c1_dec_fx, 0, 255));
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_C1, rfpll_lf_lf_c1_ideal);

	/* rfpll_lf_lf_c2 */
	/* <0.16.16> / <0.32.0> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(c1_passive_lf_fx, 20, NF16, NF0, &rfpll_lf_lf_c2_ref_fx);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	rfpll_lf_lf_c2_ref_fx = math_fp_round_32(rfpll_lf_lf_c2_ref_fx, (nf - NF16));
	/* <0.16.16> - <0.16.16> -> <0.16.16> */
	temp_sign = (int64)((int64)rfpll_lf_lf_c2_ref_fx - (int64)CONST_7P8202_FX);
	if (temp_sign > 0) {
		/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
		nf = math_fp_div_64((uint64)temp_sign, CONST_P21758_FX, NF16, NF16, &temp_32);
		/* round(<0.(32-nf).nf>, nf) -> <0.32.0> */
		rfpll_lf_lf_c2_dec_fx = math_fp_round_32(temp_32, nf);
		rfpll_lf_lf_c2_ideal = (uint16)(LIMIT((int32)rfpll_lf_lf_c2_dec_fx, 0, 255));
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_C2, rfpll_lf_lf_c2_ideal);

	/* rfpll_lf_lf_c3 */
	/* <0.16.16> / <0.32.0> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(c1_passive_lf_fx, 40, NF16, NF0, &rfpll_lf_lf_c3_ref_fx);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	rfpll_lf_lf_c3_ref_fx = math_fp_round_32(rfpll_lf_lf_c3_ref_fx, (nf - NF16));
	/* <0.16.16> - <0.16.16> -> <0.16.16> */
	temp_sign = (int64)((int64)rfpll_lf_lf_c3_ref_fx - (int64)CONST_3P8879_FX);
	if (temp_sign > 0) {
		/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
		nf = math_fp_div_64((uint64)temp_sign, CONST_P11059_FX, NF16, NF16, &temp_32);
		/* round(<0.(32-nf).nf>, nf) -> <0.32.0> */
		rfpll_lf_lf_c3_dec_fx = math_fp_round_32(temp_32, nf);
		rfpll_lf_lf_c3_ideal = (uint16)(LIMIT((int32)rfpll_lf_lf_c3_dec_fx, 0, 255));
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_C3, rfpll_lf_lf_c3_ideal);

	/* rfpll_lf_lf_c4 */
	rfpll_lf_lf_c4_ref_fx = rfpll_lf_lf_c3_ref_fx >> 1;
	/* <0.16.16> - <0.16.16> -> <0.16.16> */
	temp_sign = (int64)((int64)rfpll_lf_lf_c4_ref_fx - (int64)CONST_1P7649_FX);

	if (temp_sign > 0) {
		/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
		nf = math_fp_div_64((uint64)temp_sign, CONST_P055381_FX, NF16, NF16, &temp_32);
		/* round(<0.(32-nf).nf>, nf) -> <0.32.0> */
		rfpll_lf_lf_c4_dec_fx = math_fp_round_32(temp_32, nf);
		rfpll_lf_lf_c4_ideal = (uint16)(LIMIT((int32)rfpll_lf_lf_c4_dec_fx, 0, 255));
	}
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_LF_LF_C4, rfpll_lf_lf_c4_ideal);

	/* cp_kpd_scale */
	/* <0.16.16> / <0.16.16> -> <0.(32-nf).nf> */
	nf = math_fp_div_64(icp_fx, CONST_P0384_FX, NF16, NF0, &rfpll_cp_kpd_scale_ref_fx);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	rfpll_cp_kpd_scale_dec_fx = math_fp_round_32(rfpll_cp_kpd_scale_ref_fx, (nf - NF16));
	rfpll_cp_kpd_scale_ideal = (uint16)(LIMIT((int32)rfpll_cp_kpd_scale_dec_fx,
			0, 127));
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_CP_KPD_SCALE, rfpll_cp_kpd_scale_ideal);

	/* rfpll_cp_ioff */
	/* round(<0.16.16>) -> <0.32.0>  */
	rfpll_cp_ioff_dec_fx = math_fp_round_32(i_off_fx, NF16);
	rfpll_cp_ioff_ideal = (uint16)(LIMIT((int32)rfpll_cp_ioff_dec_fx, 0, 255));
	PLL_CONFIG_20696_VAL_ENTRY(pll, RFPLL_CP_IOFF, rfpll_cp_ioff_ideal);
	/* ------------------------------------------------------------------- */

#if DBG_PLL != 0
		print_pll_config_20696(pll, lo_freq, loop_band, icp_fx);
#endif /* DBG_PLL */
}

#if DBG_PLL != 0
static void
print_pll_config_20696(pll_config_20696_tbl_t *pll, uint32 lo_freq, uint32 loop_bw, uint32 icp_fx)
{
	printf("------------------------------------------------------------\n");
	printf("\nLO_Freq = %d MHz\n", lo_freq);
	printf("\nLoopFilter_bandwidth = %d kHz\n", loop_bw);
	printf("\nIcp_fx = %u (0.16.16 format)\n", icp_fx);
	printf("\nRegister Definition (upper):\n");
	printf("------------------------------------------------------------\n");

	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_DELAYEND);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_DELAYSTARTCOLD);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_DELAYSTARTWARM);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_XTALCOUNT);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_DELTAPLLVAL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_ERRORTHRES);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FASTSWITCH);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FIXMSB);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FOURTHMESEN);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_INITCAPA);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_INITCAPB);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_NORMCOUNTLEFT);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_NORMCOUNTRIGHT);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_PAUSECNT);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_ROUNDLSB);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_TARGETCOUNTBASE);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_TARGETCOUNTCENTER);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_OFFSETIN);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_PLL_VAL);

	printf("\nother VCOCAL register (not channel/XTAL dependent):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_SLOPEIN);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_UPDATESEL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_CALCAPRBMODE);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_TESTVCOCNT);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_CAPS_OVR);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_CAPS_OVRVAL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FAST_SETTLE_OVR);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FAST_SETTLE_OVRVAL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVR);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL);

	printf("\nVCOCAL register (required by coupled VCOs):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_ENABLECOUPLING);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_COUPLINGMODE);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_SWAPVCO12);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_MIDCODESEL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_COUPLTHRES);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_COUPLTHRES2);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_SECONDMESEN);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_UPDATESELCOUP);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_COUPLINGIN);

	printf("\nVCOCAL register (optional for coupled VCOs):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVR);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_AUX1_OVR);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_AUX1_OVRVAL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_AUX2_OVR);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCOCAL_FORCE_AUX2_OVRVAL);

	printf("\nVCO register:\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_PREFER);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_CORE1_EN);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_CORE2_EN);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_CAP_MODE);

	printf("\nFract-N (Sigma-Delta) register:\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20696(pll, RFPLL_FRCT_WILD_BASE_HIGH);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_FRCT_WILD_BASE_LOW);

	printf("\nVCO Register (11n vs 11ac mode):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_CVAR);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_ALC_REF_CTRL);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_BIAS_MODE);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_TEMPCO_DCADJ);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_VCO_TEMPCO);

	printf("\nCP and Loop Filter register:\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_R2);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_R3);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_RS_CM);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_RF_CM);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_C1);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_C2);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_C3);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_LF_LF_C4);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_CP_IOFF);
	PRINT_PLL_CONFIG_20696(pll, RFPLL_CP_KPD_SCALE);
}
#endif /* DBG_PLL */
