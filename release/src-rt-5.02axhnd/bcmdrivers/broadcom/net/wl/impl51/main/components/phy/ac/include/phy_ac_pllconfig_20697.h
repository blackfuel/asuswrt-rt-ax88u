/*
 * ACPHY 20697 Radio PLL configuration
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
 * $Id: phy_ac_pllconfig_20697.h 587569 2015-09-21 12:59:29Z $
 */

#ifndef _PHY_AC_20697_PLLCONFIG_H
#define _PHY_AC_20697_PLLCONFIG_H

/* <0.16.16> */
#define CONST_770P5							0x3028000
/* <0.16.16> */
#define CONST_45P2							0x2D3333

#define PLL20697_DOUBLER_MAIN					2
#define PLL20697_DOUBLER_AUX					2

#define PLL20697_LOGEN_MODE0					0
#define PLL20697_LOGEN_MODE1					1
#define PLL20697_PLL_NO                     0
#define PLL20697_VCO_MODE0					0
#define PLL20697_VCO_MODE1					1
#define PLL20697_LOOP_BW					340

#define PLL20697_COUPLEDVCO_0					0
#define PLL20697_COUPLEDVCO_1					1

/*  default = 2 mA; 0.16.16 format -> (2.0*pow(2,16)) */
#define PLL20697_ICP_MAIN						0x20000
/*  default = 0.6 mA; 0.16.16 format -> (0.6*pow(2,16)) */
#define PLL20697_ICP_AUX_5G						0x999A
/*  default = 0.6 mA; 0.16.16 format -> (0.6*pow(2,16)) */
#define PLL20697_ICP_AUX_2G						0x999A

#define PLL20697_KVCO_MAIN					22
#define PLL20697_KVCO_AUX_2G					33
#define PLL20697_KVCO_AUX_5G					33
#define PLL20697_ACMODE_MAIN				1
#define PLL20697_ACMODE_AUX					0
#define PLL20697_AUX_VCOCAL_RATE			0
#define PLL20697_OFFSET_P_MAIN					0
#define PLL20697_OFFSET_N_MAIN					0
#define PLL20697_OFFSET_P_AUX					8
#define PLL20697_OFFSET_N_AUX					0

#define PLL20697_RFPLL_START_DEL_DEC		9

#define RFPLL20697_CAL_CAPS_LIM_H_DEC 	4095
#define RFPLL20697_CAL_CAPS_LIM_L_DEC 	0
#define RFPLL20697_CAL_CAPS_SEL_DEC 	5

#define CAP_MULTIPLIER_RATIO_20697_MAIN_PLL0				8
#define CAP_MULTIPLIER_RATIO_20697_MAIN_PLL1				10
#define CAP_MULTIPLIER_RATIO_PLUS_ONE_20697_MAIN_PLL0	(CAP_MULTIPLIER_RATIO_20697_MAIN_PLL0+1)
#define CAP_MULTIPLIER_RATIO_PLUS_ONE_20697_MAIN_PLL1	(CAP_MULTIPLIER_RATIO_20697_MAIN_PLL1+1)

/* damp = 1 */
/* f3db_ovr_fn = sqrt(1 + 2 * damp^2 + sqrt(2 + 4 * damp^2 + 4 * damp^4)); */
/* f3db_ovr_fn = 2.4823935345082 */
/* pi = 3.14159265358979 */

/* ((kvco * f3db_ovr_fn^2) / (4 * pi^2)) * 1e9 in 1.32.16; Kvco = 22 */
#define C1_PASSIVE_LF_CONST_FX_20697_MAIN				0xCCAF27758915ULL

/* (2 * damp) / (kvco) in 0.0.32; kvco=22 */
#define R1_PASSIVE_LF_CONST_FX_20697_MAIN			0x1745D174

/* ((kvco * f3db_ovr_fn^2) / (4 * pi^2)) * 1e9 in 1.32.16; Kvco = 33 */
#define C1_PASSIVE_LF_CONST_FX_20697_AUX				0x13306BB304DA0ULL

/* (2 * damp) / (kvco) in 0.0.32; kvco=33 */
#define R1_PASSIVE_LF_CONST_FX_20697_AUX			0xF83E0F8

/* (CAP_MULTIPLIER_RATIO_PLUS_ONE / CAP_MULTIPLIER_RATIO) in 1.15.16 */
#define RF_CONST1_FX_20697_MAIN_PLL0					0x12000
#define RF_CONST1_FX_20697_MAIN_PLL1					0x1199A

/* below are defined 37.4MHz */
/*	set rfpll_vcocal_cal_pause_timeout_dec [expr {round($cal_xtal_freq * 0.2)}] */
/*	set rfpll_vcocal_cal_ref_timeout_dec [expr {ceil($cal_xtal_freq * 2.0 - 1)}] */
/*	set rfpll_vcocal_cal_count_raw
	[expr ($rfpll_vcocal_cal_ref_timeout_dec + 1)/$cal_xtal_freq ]
 */
#define RFPLL_20697_VCOCAL_CAL_PAUSE_TIMEOUT_DEC	7
#define RFPLL_20697_VCOCAL_CAL_REF_TIMEOUT_DEC		74
/* <0.16.16> */
#define RFPLL_20697_VCOCAL_CAL_COUNT_RAW			0x2015E

/* [expr {round($cal_xtal_freq * 0.25)}] */
#define PLL20697_VCOCAL_CAL_START_DEL_DEC				9
/* [expr {ceil($cal_xtal_freq * 5.0)}] */
#define PLL20697_VCOCAL_CAL_START_DEL_FAST_SETTLE_DEC	187
#define PLL20697_CAL_CAPS_LIM_H_DEC						4095
#define PLL20697_CAL_CAPS_LIM_L_DEC						0
#define PLL20697_CAL_CAPS_SEL_DEC						5
#define PLL20697_RFPLL_START_DEL_DEC					9
#define PLL20697_VCO_CAL_AUX_CAP_BITS					7
#define RFPLL_VCOCAL_UPDATESELCOUP_DEC_20697			0

/* 2.33 in 1.15.16 */
#define T_OFF_FX_2P33					0x2547B
/* 1.6 in 1.15.16 */
#define T_OFF_FX_1P60					0x1999A
/* 0.008 in 1.15.16 */
#define CONST_P008_FX					0x20C

typedef enum {
	VCO_CAL_LEGACY,
	VCO_CAL_NONLEGACY
} vco_cal_types_t;

#ifdef BCMDBG_PLL
#define PRINT_PLL_CONFIG_20697(pll_struct, offset) \
	printf("%s = %u\n", #offset, (pll_struct->reg_field_val[IDX_20697_##offset]))
#define PRINT_PLL_CONFIG_20697_MAIN(pll_struct, offset) \
	printf("%s = %u\n", #offset, (pll_struct->reg_field_val_main[IDX_20697_MAIN_##offset]))
#define PRINT_PLL_CONFIG_20697_AUX(pll_struct, offset) \
	printf("%s = %u\n", #offset, (pll_struct->reg_field_val_aux[IDX_20697_AUX_##offset]))

#else
#define PRINT_PLL_CONFIG_20697(pll_struct, offset)
#define PRINT_PLL_CONFIG_20697_MAIN(pll_struct, offset)
#define PRINT_PLL_CONFIG_20697_AUX(pll_struct, offset)
#define PRINT_VAR_VALUE(var_name)
#endif /* BCMDBG_PLL */

/* applies to both MAIN and AUX cores */
#define PLL_CONFIG_20697_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val[IDX_20697_##offset] = val
/* applies to only MAIN core */
#define PLL_CONFIG_20697_MAIN_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val_main[IDX_20697_MAIN_##offset] = val
/* applies to AUX core */
#define PLL_CONFIG_20697_AUX_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val_aux[IDX_20697_AUX_##offset] = val

/* applies to both MAIN and AUX cores */
#define PLL_20697_INFO_ENTRY(pi, pll_str, offset, regpfx, core, reg, fld) \
	pll_str->reg_addr[IDX_20697_##offset] = RADIO_REG_20697(pi, RFP, reg, core); \
	pll_str->reg_field_mask[IDX_20697_##offset] = \
			RF_##20697##_##reg##_##fld##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift[IDX_20697_##offset] = \
			RF_##20697##_##reg##_##fld##_SHIFT(pi->pubpi->radiorev); \
	printf("[Ln%d] addr: 0x%x\n", __LINE__,	pll_str->reg_addr[IDX_20697_##offset]); \

/* applies to only MAIN core */
#define PLL_20697_MN_INFO_ENTRY(pi, pll_str, offset, regpfx, core, reg, fld) \
	pll_str->reg_addr_main[IDX_20697_MAIN_##offset] = \
		((core == 0) ? RFP0_20697_SLICE0_##reg((pi)->pubpi->radiorev) : \
		((core == 1) ? RFP1_20697_SLICE0_##reg((pi)->pubpi->radiorev) : \
		INVALID_ADDRESS)); \
	pll_str->reg_field_mask_main[IDX_20697_MAIN_##offset] = \
			RF_20697_SLICE0_##reg##_##fld##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift_main[IDX_20697_MAIN_##offset] = \
			RF_20697_SLICE0_##reg##_##fld##_SHIFT(pi->pubpi->radiorev);

/* applies to only AUX core */
#define PLL_20697_AX_INFO_ENTRY(pi, pll_str, offset, regpfx, core, reg, fld) \
	pll_str->reg_addr_aux[IDX_20697_AUX_##offset] = \
		((core == 0) ? RFP0_20697_SLICE1_##reg((pi)->pubpi->radiorev) : \
		((core == 1) ? RFP1_20697_SLICE1_##reg((pi)->pubpi->radiorev) : \
		INVALID_ADDRESS)); \
	pll_str->reg_field_mask_aux[IDX_20697_AUX_##offset] = \
			RF_20697_SLICE1_##reg##_##fld##_MASK(pi->pubpi->radiorev); \
	pll_str->reg_field_shift_aux[IDX_20697_AUX_##offset] = \
			RF_20697_SLICE1_##reg##_##fld##_SHIFT(pi->pubpi->radiorev);

/* structure to hold computed PLL config values */
typedef struct {
	uint8 logen_mode;
	uint8 vcomode;
	uint8 pll_num;
	uint8 doubler;
	uint8 coupledvco;
	uint8 kvco;
	uint8 acmode;
	int8 offset_p;
	int8 offset_n;
	uint8 vcocaltype;
	uint8 aux_vcocal_rate;
	uint8 qam_mode;
	uint16 loop_bw;
	uint32 icp;
	uint32 xtal_fx;
	uint32 lbw;
	uint16 *reg_addr;
	uint16 *reg_field_mask;
	uint8 *reg_field_shift;
	uint16 *reg_field_val;
	uint16 *reg_addr_main;
	uint16 *reg_field_mask_main;
	uint8 *reg_field_shift_main;
	uint16 *reg_field_val_main;
	uint16 *reg_addr_aux;
	uint16 *reg_field_mask_aux;
	uint8 *reg_field_shift_aux;
	uint16 *reg_field_val_aux;
} pll_config_20697_tbl_t;

/* structure to hold vals for writing to structure */
typedef struct {
	uint16 rfpll_lf_lf_r2_ideal;
	uint16 rfpll_lf_lf_rs_cm_ideal;
	uint16 rfpll_lf_lf_rf_cm_ideal;
	uint16 rfpll_lf_lf_c1_ideal;
	uint16 rfpll_lf_lf_c2_ideal;
	uint16 rfpll_lf_lf_c3_ideal;
	uint16 rfpll_lf_lf_c4_ideal;
	uint16 rfpll_cp_kpd_scale_ideal;
	uint16 rfpll_cp_ioff_ideal;
} pll_vals_20697_t;

/* applies to both MAIN and AUX cores */
typedef enum {
	IDX_20697_RFPLL_VCOCAL_FORCE_CAPS_OVR,
	IDX_20697_RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
	PLL_CONFIG_20697_ARRAY_SIZE
} pll_config_20697_offset_t;

/* applies to MAIN core only */
typedef enum {
	IDX_20697_MAIN_RFPLL_VCOCAL_FORCE_CAPS_OVR,
	IDX_20697_MAIN_RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
	IDX_20697_MAIN_RFPLL_VCOCAL_XTALCOUNT,
	IDX_20697_MAIN_RFPLL_VCOCAL_CALCAPRBMODE,
	IDX_20697_MAIN_RFPLL_VCOCAL_SECONDMESEN,
	IDX_20697_MAIN_RFPLL_VCOCAL_MIDCODESEL,
	IDX_20697_MAIN_RFPLL_VCOCAL_SWAPVCO12,
	IDX_20697_MAIN_RFPLL_VCOCAL_COUPLINGMODE,
	IDX_20697_MAIN_RFPLL_VCOCAL_FORCE_VCTRL_OVR,
	IDX_20697_MAIN_RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL,
	IDX_20697_MAIN_RFPLL_VCOCAL_FAST_SETTLE_OVR,
	IDX_20697_MAIN_RFPLL_VCOCAL_FAST_SETTLE_OVRVAL,
	IDX_20697_MAIN_RFPLL_VCOCAL_FORCE_CAPS2_OVR,
	IDX_20697_MAIN_RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
	IDX_20697_MAIN_RFPLL_VCOCAL_TESTVCOCNT,
	IDX_20697_MAIN_RFPLL_VCOCAL_PAUSECNT,
	IDX_20697_MAIN_RFPLL_VCOCAL_ROUNDLSB,
	IDX_20697_MAIN_RFPLL_VCOCAL_OFFSETIN,
	IDX_20697_MAIN_RFPLL_VCOCAL_SLOPEIN,
	IDX_20697_MAIN_RFPLL_VCOCAL_UPDATESEL,
	IDX_20697_MAIN_RFPLL_VCOCAL_DELTAPLLVAL,
	IDX_20697_MAIN_RFPLL_VCOCAL_FASTSWITCH,
	IDX_20697_MAIN_RFPLL_VCOCAL_FIXMSB,
	IDX_20697_MAIN_RFPLL_VCOCAL_FOURTHMESEN,
	IDX_20697_MAIN_RFPLL_VCOCAL_NORMCOUNTLEFT,
	IDX_20697_MAIN_RFPLL_VCOCAL_NORMCOUNTRIGHT,
	IDX_20697_MAIN_RFPLL_VCOCAL_DELAYEND,
	IDX_20697_MAIN_RFPLL_VCOCAL_DELAYSTARTCOLD,
	IDX_20697_MAIN_RFPLL_VCOCAL_DELAYSTARTWARM,
	IDX_20697_MAIN_RFPLL_VCOCAL_UPDATESELCOUP,
	IDX_20697_MAIN_RFPLL_VCOCAL_TARGETCOUNTBASE,
	IDX_20697_MAIN_RFPLL_VCOCAL_TARGETCOUNTCENTER,
	IDX_20697_MAIN_RFPLL_VCOCAL_ENABLECOUPLING,
	IDX_20697_MAIN_RFPLL_VCOCAL_ERRORTHRES,
	IDX_20697_MAIN_RFPLL_VCOCAL_COUPLTHRES,
	IDX_20697_MAIN_RFPLL_VCOCAL_COUPLTHRES2,
	IDX_20697_MAIN_RFPLL_VCOCAL_INITCAPA,
	IDX_20697_MAIN_RFPLL_VCOCAL_INITCAPB,
	IDX_20697_MAIN_RFPLL_VCOCAL_PLL_VAL,
	IDX_20697_MAIN_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20697_MAIN_RFPLL_VCO_CORE1_EN,
	IDX_20697_MAIN_RFPLL_VCO_CORE2_EN,
	IDX_20697_MAIN_RFPLL_PFD_RISING_EDGE_EN,
	IDX_20697_MAIN_RFPLL_PFD_FALLING_EDGE_EN,
	IDX_20697_MAIN_RFPLL_LF_LF_R2,
	IDX_20697_MAIN_RFPLL_LF_LF_R3,
	IDX_20697_MAIN_RFPLL_LF_LF_RS_CM,
	IDX_20697_MAIN_RFPLL_LF_LF_RF_CM,
	IDX_20697_MAIN_RFPLL_LF_LF_C1,
	IDX_20697_MAIN_RFPLL_LF_LF_C2,
	IDX_20697_MAIN_RFPLL_LF_LF_C3,
	IDX_20697_MAIN_RFPLL_LF_LF_C4,
	IDX_20697_MAIN_RFPLL_CP_KPD_SCALE,
	IDX_20697_MAIN_RFPLL_CP_IOFF,
	IDX_20697_MAIN_RFPLL_FRCT_WILD_BASE_HIGH,
	IDX_20697_MAIN_RFPLL_FRCT_WILD_BASE_LOW,
	IDX_20697_MAIN_RFPLL_VCOCAL_CLKXTAL_DIV_RATIO,
	IDX_20697_MAIN_RFPLL_RFVCO_VTEMP_DC,
	IDX_20697_MAIN_RFPLL_RFVCO_VTEMP_CODE,
	IDX_20697_MAIN_RFPLL_LOGEN2G_DIV3OVR2,
	IDX_20697_MAIN_RFPLL_VCO_ALC_REF_CTRL,
	IDX_20697_MAIN_RFPLL_VCO_BIAS_MODE,
	IDX_20697_MAIN_RFPLL_VCO_TEMPCO_DCADJ,
	IDX_20697_MAIN_RFPLL_VCO_TEMPCO,
	IDX_20697_MAIN_RFPLL_VCO_VREG_IB,
	IDX_20697_MAIN_RFPLL_VCO_VREG_IB_CTRL,
	IDX_20697_MAIN_RFPLL_VCLDO1V_CCOPM,
	IDX_20697_MAIN_RFPLL_VCO_CTAIL_BOT,
	IDX_20697_MAIN_RFPLL_KVCO_CTRL,
	IDX_20697_MAIN_XTAL_DOUBLER_PU,
	PLL_CONFIG_20697_MAIN_ARRAY_SIZE
} pll_config_20697_main_offset_t;

/* applies to AUX core only */
typedef enum {
	IDX_20697_AUX_RFPLL_VCOCAL_FORCE_CAPS_OVR,
	IDX_20697_AUX_RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
	IDX_20697_AUX_RFPLL_LOCORE_DIV3EN,
	IDX_20697_AUX_RFPLL_PFD_RISING_EDGE_EN,
	IDX_20697_AUX_RFPLL_PFD_FALLING_EDGE_EN,
	IDX_20697_AUX_RFPLL_LF_LF_R2,
	IDX_20697_AUX_RFPLL_LF_LF_R3,
	IDX_20697_AUX_RFPLL_LF_LF_RS_CM,
	IDX_20697_AUX_RFPLL_LF_LF_RF_CM,
	IDX_20697_AUX_RFPLL_LF_LF_C1,
	IDX_20697_AUX_RFPLL_LF_LF_C2,
	IDX_20697_AUX_RFPLL_LF_LF_C3,
	IDX_20697_AUX_RFPLL_LF_LF_C4,
	IDX_20697_AUX_RFPLL_CP_KPD_SCALE,
	IDX_20697_AUX_RFPLL_CP_IOFF,
	IDX_20697_AUX_RFPLL_FRCT_WILD_BASE_HIGH,
	IDX_20697_AUX_RFPLL_FRCT_WILD_BASE_LOW,
	IDX_20697_AUX_RFPLL_VCOCAL_CLKXTAL_DIV_RATIO,
	IDX_20697_AUX_RFPLL_RFVCO_VTEMP_DC,
	IDX_20697_AUX_RFPLL_RFVCO_VTEMP_CODE,
	IDX_20697_AUX_RFPLL_5G_RFVCO_VTHM_REF,
	IDX_20697_AUX_RFPLL_RFVCO_CS_M,
	IDX_20697_AUX_RFPLL_RFVCO_CS_N,
	IDX_20697_AUX_RFPLL_VCOCAL_FVCO_BY_2,
	IDX_20697_AUX_RFPLL_VCOCAL_CAL_PAUSE_TIMEOUT_DEC,
	IDX_20697_AUX_RFPLL_VCOCAL_CAL_REF_TIMEOUT_DEC,
	IDX_20697_AUX_RFPLL_VCOCAL_CAL_COUNT_DEC,
	IDX_20697_AUX_RFPLL_VCOCAL_CAL_START_DEL_FAST_SETTLE_DEC,
	IDX_20697_AUX_RFPLL_CAL_CAPS_LIM_H_DEC,
	IDX_20697_AUX_RFPLL_CAL_CAPS_LIM_L_DEC,
	IDX_20697_AUX_RFPLL_CAL_CAPS_SEL_DEC,
	IDX_20697_AUX_RFPLL_EN_HIGH_BAND_TABLE_DEC,
	IDX_20697_AUX_RFPLL_CODE_OFFSET_P_DEC,
	IDX_20697_AUX_RFPLL_CODE_OFFSET_N_DEC,
	IDX_20697_AUX_RFPLL_START_DEL_DEC,
	IDX_20697_AUX_XTAL_DOUBLER_PU,
	PLL_CONFIG_20697_AUX_ARRAY_SIZE
} pll_config_20697_aux_offset_t;

#endif /* _PHY_AC_20697_PLLCONFIG_H */
