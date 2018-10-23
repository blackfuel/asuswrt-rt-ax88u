/*
 * ACPHY 20694 Radio PLL configuration
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
 * $Id: phy_ac_pllconfig_20694.h 587569 2015-09-21 12:59:29Z $
 */

#ifndef _PHY_AC_20694_PLLCONFIG_H
#define _PHY_AC_20694_PLLCONFIG_H

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
/* 303 in 1.15.16 */
#define CONST_303_FX					0x12F0000
/* 280 in 1.15.16 */
#define CONST_280_FX					0x1180000
/* 400 in 1.15.16 */
#define CONST_400_FX					0x1900000
/* 3200 in 1.15.16 */
#define CONST_3200_FX					0xC800000
/* 1600 in 1.15.16 */
#define CONST_1600_FX					0x6400000
/* 5.4 in 1.15.16 */
#define CONST_5P4_FX					0x56666
/* 0.52 in 1.15.16 */
#define CONST_P52_FX					0x851F
/* 3 in 1.15.16 */
#define CONST_3_FX						0x30000
/* 1.32 in 1.15.16 */
#define CONST_1P32_FX					0x151EC
/* 0.25 in 1.15.16 */
#define CONST_P25_FX					0x4000
/* 0.663 in 1.15.16 */
#define CONST_P663_FX					0xA9BA
/* 0.125 in 1.15.16 */
#define CONST_P125_FX					0x2000
/* 0.317 in 1.15.16 */
#define CONST_P317_FX					0x5127
/* 0.0061 in 1.15.16 */
#define CONST_P061_FX					0xF9DB22D
/* 0.0384 in 1.15.16 */
#define CONST_P0384_FX					0x9D5
/* 0.032 in 1.15.16 */
#define CONST_P032_FX					0x831
/* 0.9 in 1.15.16 */
#define CONST_P9_FX						0xE666
/* 3.4 in 1.15.16 */
#define CONST_3P4_FX					0x36666
/* 0.6 in 1.15.16 */
#define CONST_P6_FX						0x999A

/* (CAP_MULTIPLIER_RATIO_PLUS_ONE / CAP_MULTIPLIER_RATIO) in 1.15.16 */
#define RF_CONST1_FX					0x12000

/* Band dependent constants */
/* #define VCO_CAL_CAP_BITS_5G				11 */
/* #define VCO_CAL_CAP_BITS_2G				11 */
#define VCO_CAL_CAP_BITS					11
/* damp = 1 */
/* f3db_ovr_fn = sqrt(1 + 2 * damp^2 + sqrt(2 + 4 * damp^2 + 4 * damp^4)); */
/* f3db_ovr_fn = 2.4823935345082 */
/* pi = 3.14159265358979 */
/* ((icp * kvco * f3db_ovr_fn^2 * 1e-3) / (4 * pi^2)) * 1e12 in 1.32.16; */
/* 1e12 used for Cap value in pF */

/* ((kvco * f3db_ovr_fn^2) / (4 * pi^2)) * 1e9 in 1.32.16; Kvco = 22 */
#define C1_PASSIVE_LF_CONST_FX				0xCCAF27758915ULL
/* (2 * damp) / (kvco) in 0.0.32; kvco=22 */
#define R1_PASSIVE_LF_CONST_FX			0x1745D174
/* (R_CONSTANT * damp) / (kvco) in 1.0.32; R_CONSTANT=2, kvco=11 */
#define RFPLL_LF_LF_R2_REF_CONST_FX			0x1745D174

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
#define RFPLL_VCOCAL_UPDATESELCOUP_DEC_20694	0
#define RFPLL_VCOCAL_COUPLINGIN_DEC				0
#define RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_AUX1_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_AUX1_OVRVAL_DEC		0
#define RFPLL_VCOCAL_FORCE_AUX2_OVR_DEC			0
#define RFPLL_VCOCAL_FORCE_AUX2_OVRVAL_DEC		0
#define RFPLL_VCO_CAP_MODE_DEC					0
#define C_CONSTANT								30
#define CAP_MULTIPLIER_RATIO_20694				8
#define CAP_MULTIPLIER_RATIO_PLUS_ONE_20694		9

/* zero will take default value */
// #define LOOP_BW						0
#define USE_DOUBLER					1
#define LOGEN_MODE					0

#define XTAL_FREQ					37400000

/* No of fraction bits */
#define NF0	0
#define NF6	6
#define NF8	8
#define NF16	16
#define NF20	20
#define NF24	24
#define NF32	32

#ifdef BCMDBG_PLL
#define PRINT_PLL_CONFIG_20694(pll_struct, offset) \
	printf("\n%s = %u\n", #offset, (pll_struct->reg_field_val[IDX_20694_##offset]))
#define PRINT_PLL_CONFIG_20694_MAIN(pll_struct, offset) \
	printf("\n%s = %u\n", #offset, (pll_struct->reg_field_val_main[IDX_20694_MAIN_##offset]))

#define PRINT_PLL_CONFIG(struct_name, var_name, mask) \
	printf("%s = 0x%x\n", #var_name, (struct_name->var_name & mask))

#define PRINT_VAR_VALUE(var_name) printf("%s = %d\n", #var_name, var_name)

#define PRINT_PLL_CONFIG_FLAG_20694 0
#else
#define PRINT_PLL_CONFIG_20694(pll_struct, offset)
#define PRINT_PLL_CONFIG_20694_MAIN(pll_struct, offset)
#define PRINT_PLL_CONFIG(struct_name, var_name, mask)
#define PRINT_VAR_VALUE(var_name)
#endif /* BCMDBG_PLL */

/* applies to both MAIN and AUX cores */
#define PLL_CONFIG_20694_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val[IDX_20694_##offset] = val
/* applies to only MAIN core */
#define PLL_CONFIG_20694_MAIN_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val_main[IDX_20694_MAIN_##offset] = val

/* applies to both MAIN and AUX cores */
#define PLL_CONFIG_20694_REG_INFO_ENTRY(pi, pll_str, offset, regpfx, reg, fld) \
	pll_str->reg_addr[IDX_20694_##offset] = RADIO_REG_20694(pi, RFP, reg, 0); \
	pll_str->reg_field_mask[IDX_20694_##offset] = \
			RF_##20694##_##reg##_##fld##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift[IDX_20694_##offset] = \
			RF_##20694##_##reg##_##fld##_SHIFT(pi->pubpi->radiorev); \
/* Note: above in RADIO_REG_20694(pi, regpfx, regnm, core), */
/* core is set 0, since in MIMO mode only PLL0(RFP0) is the valid PLL */
/* for both main and aux cores */

/* applies to only MAIN core */
#define PLL_CONFIG_20694_MAIN_REG_INFO_ENTRY(pi, pll_str, offset, regpfx, reg, fld) \
	pll_str->reg_addr_main[IDX_20694_MAIN_##offset] = RADIO_REG_20694(pi, RFP, reg, 0); \
	pll_str->reg_field_mask_main[IDX_20694_MAIN_##offset] = \
			RF_##20694##_##reg##_##fld##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift_main[IDX_20694_MAIN_##offset] = \
			RF_##20694##_##reg##_##fld##_SHIFT(pi->pubpi->radiorev); \

/* structure to hold computed PLL config values */
typedef struct {
	uint32 xtal_fx;
	uint32 loop_bw;
	uint8 use_doubler;
	uint8 logen_mode;
	uint16 *reg_addr;
	uint16 *reg_field_mask;
	uint8 *reg_field_shift;
	uint16 *reg_field_val;
	uint16 *reg_addr_main;
	uint16 *reg_field_mask_main;
	uint8 *reg_field_shift_main;
	uint16 *reg_field_val_main;
} pll_config_20694_tbl_t;

/* applies to both MAIN and AUX cores */
typedef enum {
	IDX_20694_OVR_RFPLL_VCOCAL_SLOPEIN,
	IDX_20694_OVR_RFPLL_VCOCAL_OFFSETIN,
	IDX_20694_OVR_RFPLL_VCOCAL_RST_N,
	IDX_20694_OVR_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20694_SEL_BAND,
	IDX_20694_OVR_MUX_XT_DEL,
	IDX_20694_OVR_MUX_SEL_BAND,
	IDX_20694_RFPLL_VCOCAL_DELAYEND,
	IDX_20694_RFPLL_VCOCAL_DELAYSTARTCOLD,
	IDX_20694_RFPLL_VCOCAL_DELAYSTARTWARM,
	IDX_20694_RFPLL_VCOCAL_XTALCOUNT,
	IDX_20694_RFPLL_VCOCAL_CALCAPRBMODE,
	IDX_20694_RFPLL_VCOCAL_DELTAPLLVAL,
	IDX_20694_RFPLL_VCOCAL_ERRORTHRES,
	IDX_20694_RFPLL_VCOCAL_INITCAPA,
	IDX_20694_RFPLL_VCOCAL_INITCAPB,
	IDX_20694_RFPLL_VCOCAL_NORMCOUNTLEFT,
	IDX_20694_RFPLL_VCOCAL_NORMCOUNTRIGHT,
	IDX_20694_RFPLL_VCOCAL_PAUSECNT,
	IDX_20694_RFPLL_VCOCAL_ROUNDLSB,
	IDX_20694_RFPLL_VCOCAL_UPDATESEL,
	IDX_20694_RFPLL_VCOCAL_TESTVCOCNT,
	IDX_20694_RFPLL_VCOCAL_FAST_SETTLE_OVR,
	IDX_20694_RFPLL_VCOCAL_FAST_SETTLE_OVRVAL,
	IDX_20694_RFPLL_VCOCAL_FORCE_VCTRL_OVR,
	IDX_20694_RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL,
	IDX_20694_RFPLL_VCOCAL_TARGETCOUNTBASE,
	IDX_20694_RFPLL_VCOCAL_TARGETCOUNTCENTER,
	IDX_20694_RFPLL_VCOCAL_OFFSETIN,
	IDX_20694_RFPLL_VCOCAL_SLOPEIN,
	IDX_20694_RFPLL_VCOCAL_FASTSWITCH,
	IDX_20694_RFPLL_VCOCAL_FIXMSB,
	IDX_20694_RFPLL_VCOCAL_FOURTHMESEN,
	IDX_20694_RFPLL_VCO_CAP_MODE,
	IDX_20694_RFPLL_VCOCAL_PLL_VAL,
	IDX_20694_RFPLL_VCOCAL_FORCE_CAPS_OVR,
	IDX_20694_RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
	IDX_20694_RFPLL_FRCT_WILD_BASE_HIGH,
	IDX_20694_RFPLL_FRCT_WILD_BASE_LOW,
	IDX_20694_RFPLL_VCO_CVAR,
	IDX_20694_RFPLL_VCO_ALC_REF_CTRL,
	IDX_20694_RFPLL_VCO_BIAS_MODE,
	IDX_20694_RFPLL_VCO_TEMPCO_DCADJ,
	IDX_20694_RFPLL_VCO_TEMPCO,
	IDX_20694_RFPLL_LF_LF_R2,
	IDX_20694_RFPLL_LF_LF_R3,
	IDX_20694_RFPLL_LF_LF_RS_CM,
	IDX_20694_RFPLL_LF_LF_RF_CM,
	IDX_20694_RFPLL_LF_LF_C1,
	IDX_20694_RFPLL_LF_LF_C2,
	IDX_20694_RFPLL_LF_LF_C3,
	IDX_20694_RFPLL_LF_LF_C4,
	IDX_20694_RFPLL_CP_IOFF,
	IDX_20694_RFPLL_CP_KPD_SCALE,
	PLL_CONFIG_20694_ARRAY_SIZE
} pll_config_20694_offset_t;

/* applies to MAIN core only */
typedef enum {
	IDX_20694_MAIN_RFPLL_VCO_CORE1_EN,
	IDX_20694_MAIN_RFPLL_VCO_CORE2_EN,
	IDX_20694_MAIN_RFPLL_VCOCAL_ENABLECOUPLING,
	IDX_20694_MAIN_RFPLL_VCOCAL_COUPLINGMODE,
	IDX_20694_MAIN_RFPLL_VCOCAL_SWAPVCO12,
	IDX_20694_MAIN_RFPLL_VCOCAL_MIDCODESEL,
	IDX_20694_MAIN_RFPLL_VCOCAL_SECONDMESEN,
	IDX_20694_MAIN_RFPLL_VCOCAL_UPDATESELCOUP,
	IDX_20694_MAIN_RFPLL_VCOCAL_COUPLTHRES,
	IDX_20694_MAIN_RFPLL_VCOCAL_COUPLTHRES2,
	IDX_20694_MAIN_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20694_MAIN_RFPLL_VCOCAL_FORCE_CAPS2_OVR,
	IDX_20694_MAIN_RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
	IDX_20694_MAIN_RFPLL_VCOCAL_FORCE_AUX1_OVR,
	IDX_20694_MAIN_RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
	IDX_20694_MAIN_RFPLL_VCOCAL_FORCE_AUX2_OVR,
	IDX_20694_MAIN_RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
	PLL_CONFIG_20694_MAIN_ARRAY_SIZE
} pll_config_20694_main_offset_t;

#endif /* _PHY_AC_20694_PLLCONFIG_H */
