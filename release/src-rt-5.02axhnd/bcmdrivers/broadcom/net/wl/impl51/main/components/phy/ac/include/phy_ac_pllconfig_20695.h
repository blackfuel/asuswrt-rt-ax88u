/*
 * ACPHY 20695 Radio PLL configuration
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
 * $Id: phy_ac_pllconfig_20695.h 648459 2016-07-12 12:29:59Z $
 */

#ifndef _PHY_AC_20695_PLLCONFIG_H
#define _PHY_AC_20695_PLLCONFIG_H

/* Fixed point constant defines */
/* 2^-13 in 1.15.16 */
#define FIX_2POW_MIN13					0x8
/* 2.0 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYEND_FX			0x20000
/* 10.0 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYSTARTCOLD_FX			0xA0000
/* 0.5 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYSTARTWARM_FX			0x8000
/* 1.0 in 1.15.16 */
#define RFPLL_VCOCAL_PAUSECNT_FX			0x10000
/* 2.33 in 1.15.16 */
#define T_OFF_FX					0x2547B
/* 1.0 in 1.15.16 */
#define FX_ONE						0x10000
/* ((1000 * 2 * pi) / f3db_ovr_fn) in 1.23.8 */
#define WN_CONST_FX					0x9E31A
/* 800 in 1.15.16 */
#define CONST_800_FX					0x3200000
/* 400 in 1.15.16 */
#define CONST_400_FX					0x1900000
/* 3200 in 1.15.16 */
#define CONST_3200_FX					0xC800000
/* 1600 in 1.15.16 */
#define CONST_1600_FX					0x6400000
/* 5.4 in 1.15.16 */
#define CONST_5P4_FX					0x56666
/* 0.9 in 1.15.16 */
#define CONST_P9_FX					0xE666
/* 3.4 in 1.15.16 */
#define CONST_3P4_FX					0x36666
/* 0.6 in 1.15.16 */
#define CONST_P6_FX					0x999A
/* (CAP_MULTIPLIER_RATIO_PLUS_ONE / CAP_MULTIPLIER_RATIO) in 1.15.16 */
#define RF_CONST_FX					0x1199A

/* Band dependent constants */
#define VCO_CAL_CAP_BITS_5G				12
#define VCO_CAL_CAP_BITS_2G				9
/* damp = 1 */
/* f3db_ovr_fn = sqrt(1 + 2 * damp^2 + sqrt(2 + 4 * damp^2 + 4 * damp^4)); */
/* f3db_ovr_fn = 2.4823935345082 */
/* pi = 3.14159265358979 */
/* ((icp * kvco * f3db_ovr_fn^2) / (4 * pi^2)) * 1e9 in 1.32.16 */
#define C1_PASSIVE_LF_CONST_FX_5G			0x323D9C4DB8E95ULL
#define C1_PASSIVE_LF_CONST_FX_2G			0xDF4AB6AEC40DULL
/* (2 * damp) / (icp * kvco) in 0.0.32 */
#define R1_PASSIVE_LF_CONST_FX_5G			0x5ED097B
#define R1_PASSIVE_LF_CONST_FX_2G			0x15555555
/* (R_CONSTANT * damp) / (icp * kvco) in 1.0.32 */
#define RFPLL_LF_LF_R2_REF_CONST_FX_5G			0x1F9ADD4
#define RFPLL_LF_LF_R2_REF_CONST_FX_2G			0x71C71C7
#define KVCO_CODE_FX_5G					0x8
#define KVCO_CODE_FX_2G					0xb
#define TEMP_CODE_FX_5G					0x8
#define TEMP_CODE_FX_2G					0xb
/* 0.9 in 1.15.16 */
#define ICP_FX_5G					0xE666
/* 0.4 in 1.15.16 */
#define ICP_FX_2G					0x6666
/* (icp * 125) in 1.32.0 */
/* For 5G it is hardcoded to 0xFF */
#define RFPLL_CP_KPD_SCALE_DEC_FX_5G			0xFF
#define RFPLL_CP_KPD_SCALE_DEC_FX_2G			0x32

/* Constants */
#define VCO_CAL_AUX_CAP_BITS				7
#define UP_CONVERSION_RATIO				2
#define RFPLL_VCOCAL_TARGETCOUNTBASE			0
#define RFPLL_VCOCAL_OFFSETIN				0
#define RFPLL_VCOCAL_UPDATESEL_DEC			1
#define RFPLL_VCOCAL_CALCAPRBMODE_DEC			3
#define RFPLL_VCOCAL_TESTVCOCNT_DEC			0
#define RFPLL_VCOCAL_FORCE_CAPS_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_CAPS_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FAST_SETTLE_OVR_DEC		0
#define RFPLL_VCOCAL_FAST_SETTLE_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_VCTRL_OVR_DEC		0
#define RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL_DEC		0
#define RFPLL_VCOCAL_ENABLECOUPLING_DEC			0
#define RFPLL_VCOCAL_COUPLINGMODE_DEC			1
#define RFPLL_VCOCAL_SWAPVCO12_DEC			0
#define RFPLL_VCOCAL_COUPLTHRES_DEC			6
#define RFPLL_VCOCAL_COUPLTHRES2_DEC			12
#define RFPLL_VCOCAL_SECONDMESEN_DEC			3
#define RFPLL_VCOCAL_UPDATESELCOUP_DEC			1
#define RFPLL_VCOCAL_COUPLINGIN_DEC			0
#define RFPLL_VCOCAL_FORCE_CAPS2_OVR_DEC		0
#define RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_AUX1_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_AUX1_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_AUX2_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_AUX2_OVRVAL_DEC		0
#define C_CONSTANT					30
#define CAP_MULTIPLIER_RATIO				10
#define CAP_MULTIPLIER_RATIO_PLUS_ONE			11

#define LOOP_BW_5G					550
#define LOOP_BW_2G					550
/* zero will take default value */
#define LOOP_BW						0
#define USE_DOUBLER					1
#define LOGEN_MODE					0

/* No of fraction bits */
#define NF0	0
#define NF6	6
#define NF8	8
#define NF16	16
#define NF20	20
#define NF24	24
#define NF32	32

/* WAR Related constants defines */
#define RFPLL_LF_LF_C1_CH2412		0x1f
#define RFPLL_LF_LF_C2_CH2412		0x1f
#define RFPLL_LF_LF_C3_CH2412		0x1f
#define RFPLL_LF_LF_C4_CH2412		0x1f
#define RFPLL_LF_LF_R2_CH2412		0x1
#define RFPLL_LF_LF_R3_CH2412		0x1
#define RFPLL_LF_LF_RF_CM_CH2412	0x9
#define RFPLL_LF_LF_RS_CM_CH2412	0x1a

#ifdef BCMDBG_PLL
#define PRINT_PLL_CONFIG(struct_name, var_name, mask) \
		printf("%s = 0x%x\n", #var_name, (struct_name->var_name & mask))
#define PRINT_VAR_VALUE(var_name) printf("%s = %d\n", #var_name, var_name)

#define PRINT_PLL_CONFIG_FLAG 0
#endif // endif

#define PLL_CONFIG_20695_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val[IDX_20695_##offset] = val

#if defined(DBAND) || defined(USE_5G_PLL_FOR_2G)
#define PLL_CONFIG_20695_REG_INFO_ENTRY(pi, pll_str, offset, regpfx, reg2g, reg5g, fld2g, fld5g) \
	do { \
	pll_str->reg_addr_2g[IDX_20695_##offset] = RADIO_REG_20695(pi, RFP, reg2g, 0); \
	pll_str->reg_addr_5g[IDX_20695_##offset] = RADIO_REG_20695(pi, RFP, reg5g, 0); \
	pll_str->reg_field_mask_2g[IDX_20695_##offset] = \
			RF_##20695##_##reg2g##_##fld2g##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_mask_5g[IDX_20695_##offset] = \
			RF_##20695##_##reg5g##_##fld5g##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift_2g[IDX_20695_##offset] = \
			RF_##20695##_##reg2g##_##fld2g##_SHIFT(pi->pubpi->radiorev); \
	pll_str->reg_field_shift_5g[IDX_20695_##offset] = \
			RF_##20695##_##reg5g##_##fld5g##_SHIFT(pi->pubpi->radiorev); \
	} while (0)
#else
#define PLL_CONFIG_20695_REG_INFO_ENTRY(pi, pll_str, offset, regpfx, reg2g, reg5g, fld2g, fld5g) \
	do { \
	pll_str->reg_addr_2g[IDX_20695_##offset] = RADIO_REG_20695(pi, RFP, reg2g, 0); \
	pll_str->reg_field_mask_2g[IDX_20695_##offset] = \
			RF_##20695##_##reg2g##_##fld2g##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift_2g[IDX_20695_##offset] = \
			RF_##20695##_##reg2g##_##fld2g##_SHIFT(pi->pubpi->radiorev); \
	} while (0)
#endif /* DBAND */

/* structure to hold computed PLL config values */
typedef struct {
	uint32 xtal_fx;
	uint32 loop_bw;
	uint8 use_doubler;
	uint8 logen_mode;
	uint16 *reg_addr_2g;
	uint16 *reg_addr_5g;
	uint16 *reg_field_mask_2g;
	uint16 *reg_field_mask_5g;
	uint8 *reg_field_shift_2g;
	uint8 *reg_field_shift_5g;
	uint16 *reg_field_val;
} pll_config_20695_tbl_t;

typedef enum {
	IDX_20695_OVR_RFPLL_VCOCAL_OFFSETIN,
	IDX_20695_OVR_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20695_OVR_MUX_XT_DEL,
	IDX_20695_OVR_MUX_SEL_BAND,
	IDX_20695_XTALCNT_2G,
	IDX_20695_XTALCNT_5G,
	IDX_20695_RFPLL_VCOCAL_DELAYEND,
	IDX_20695_RFPLL_VCOCAL_DELAYSTARTCOLD,
	IDX_20695_RFPLL_VCOCAL_DELAYSTARTWARM,
	IDX_20695_RFPLL_VCOCAL_XTALCOUNT,
	IDX_20695_RFPLL_VCOCAL_CALCAPRBMODE,
	IDX_20695_RFPLL_VCOCAL_DELTAPLLVAL,
	IDX_20695_RFPLL_VCOCAL_ERRORTHRES,
	IDX_20695_RFPLL_VCOCAL_INITCAPA,
	IDX_20695_RFPLL_VCOCAL_INITCAPB,
	IDX_20695_RFPLL_VCOCAL_NORMCOUNTLEFT,
	IDX_20695_RFPLL_VCOCAL_NORMCOUNTRIGHT,
	IDX_20695_RFPLL_VCOCAL_PAUSECNT,
	IDX_20695_RFPLL_VCOCAL_ROUNDLSB,
	IDX_20695_RFPLL_VCOCAL_UPDATESEL,
	IDX_20695_RFPLL_VCOCAL_TESTVCOCNT,
	IDX_20695_RFPLL_VCOCAL_FAST_SETTLE_OVR,
	IDX_20695_RFPLL_VCOCAL_FAST_SETTLE_OVRVAL,
	IDX_20695_RFPLL_VCOCAL_FORCE_VCTRL_OVR,
	IDX_20695_RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL,
	IDX_20695_RFPLL_VCOCAL_TARGETCOUNTBASE,
	IDX_20695_RFPLL_VCOCAL_TARGETCOUNTCENTER,
	IDX_20695_RFPLL_VCOCAL_OFFSETIN,
	IDX_20695_RFPLL_VCOCAL_PLL_VAL,
	IDX_20695_RFPLL_VCOCAL_FORCE_CAPS_OVR,
	IDX_20695_RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
	IDX_20695_RFPLL_FRCT_WILD_BASE_HIGH,
	IDX_20695_RFPLL_FRCT_WILD_BASE_LOW,
	IDX_20695_RFPLL_VCOCAL_ENABLECOUPLING,
	IDX_20695_RFPLL_VCOCAL_COUPLINGMODE,
	IDX_20695_RFPLL_VCOCAL_SWAPVCO12,
	IDX_20695_RFPLL_VCOCAL_MIDCODESEL,
	IDX_20695_RFPLL_VCOCAL_SECONDMESEN,
	IDX_20695_RFPLL_VCOCAL_UPDATESELCOUP,
	IDX_20695_RFPLL_VCOCAL_COUPLTHRES,
	IDX_20695_RFPLL_VCOCAL_COUPLTHRES2,
	IDX_20695_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20695_RFPLL_VCOCAL_FORCE_CAPS2_OVR,
	IDX_20695_RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
	IDX_20695_RFPLL_VCOCAL_FORCE_AUX1_OVR,
	IDX_20695_RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
	IDX_20695_RFPLL_VCOCAL_FORCE_AUX2_OVR,
	IDX_20695_RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
	IDX_20695_RFPLL_LF_LF_R2,
	IDX_20695_RFPLL_LF_LF_R3,
	IDX_20695_RFPLL_LF_LF_RS_CM,
	IDX_20695_RFPLL_LF_LF_RF_CM,
	IDX_20695_RFPLL_LF_LF_C1,
	IDX_20695_RFPLL_LF_LF_C2,
	IDX_20695_RFPLL_LF_LF_C3,
	IDX_20695_RFPLL_LF_LF_C4,
	IDX_20695_RFPLL_CP_IOFF,
	IDX_20695_RFPLL_CP_KPD_SCALE,
	IDX_20695_RFPLL_RFVCO_KVCO_CODE,
	IDX_20695_RFPLL_RFVCO_TEMP_CODE,
	PLL_CONFIG_20695_ARRAY_SIZE
} pll_config_20695_offset_t;

#endif /* _PHY_AC_20695_PLLCONFIG_H */
