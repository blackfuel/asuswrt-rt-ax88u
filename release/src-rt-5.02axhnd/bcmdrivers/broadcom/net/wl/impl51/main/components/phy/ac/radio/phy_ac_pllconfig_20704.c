/*
 * ACPHY 20704 Radio PLL configuration
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
#include <wlc_radioreg_20704.h>
#include <phy_ac_pllconfig_20704.h>

#define DBG_PLL 0

/* Fixed point constant defines */
/* 2^-13 in 1.15.16 */
#define FIX_2POW_MIN13                  0x8
/* 2.0 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYEND_FX        0x20000
/* 10.0 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYSTARTCOLD_FX  0xA0000
/* 0.5 in 1.15.16 */
#define RFPLL_VCOCAL_DELAYSTARTWARM_FX  0x8000
/* 1.0 in 1.15.16 */
#define RFPLL_VCOCAL_PAUSECNT_FX        0x10000
/* 1.7 in 1.15.16 */
#define T_OFF_FX                        0x1b333
/* 1.0 in 1.15.16 */
#define FX_ONE                          0x10000
/* ((1000 * 2 * pi) / f3db_ovr_fn) in 1.23.8 */
#define CONST_257_18_FX                 0x1012e14
/* 281.21 in 1.15.16 */
#define CONST_281_21_FX                 0x11935c3
/* 3.2619 in 1.15.16 */
#define CONST_3P2619_FX                 0x3430c
/* 0.5041 in 1.15.16 */
#define CONST_P5041_FX                  0x810d
/* 7.8202 in 1.15.16 */
#define CONST_7P8202_FX                 0x7d1f9
/* 0.21758 in 1.15.16 */
#define CONST_P21758_FX                 0x37b3
/* 3.8879 in 1.15.16 */
#define CONST_3P8879_FX                 0x3e34d
/* 0.11059 in 1.15.16 */
#define CONST_P11059_FX                 0x1c50
/* 1.7649 in 1.15.16 */
#define CONST_1P7649_FX                 0x1c3d0
/* 0.055381 in 1.15.16 */
#define CONST_P055381_FX                0xe2d
/* 0.03901 in 1.15.16 */
#define CONST_P03901_FX                 0x9fd
/* 1e9 in (0.32.0) */
#define CONST_1E9_FX                    0x3b9aca00
/* 0.134068 in (0.0.32) */
#define CONST_P134068_FX                0x225247cb
/* 0.27314 in (0.0.32) */
#define CONST_P27314_FX                 0x45ec80c7
/* 3/2 in (0.1.31) */
#define CONST_3_OVER_2_FX               0xc0000000
/* 2/3 in (0.1.31) */
#define CONST_2_OVER_3_FX               0x55555555
/* 13.151136 in (0.16.16) */
#define CONST_13P151136_FX              0x000d26b1
/* 1607.45  in (0.16.16) */
#define CONST_1607P45_FX                0x06477333
/* 28.989 in (0.16.16) */
#define CONST_28P989_FX                 0x001cfd2f

/* (CAP_MULTIPLIER_RATIO_PLUS_ONE / CAP_MULTIPLIER_RATIO) in 1.15.16 */
#define RF_CONST1_FX                    0x12000

/* Band dependent constants */
#define VCO_CAL_CAP_BITS 11
/* damp = 1 */
/* f3db_ovr_fn = sqrt(1 + 2 * damp^2 + sqrt(2 + 4 * damp^2 + 4 * damp^4)); */
/* f3db_ovr_fn = 2.4823935345082537 */
/* pi = 3.14159265358979 */
/* ((kvco * f3db_ovr_fn^2) / (4 * pi^2)) * 1e9 in 1.32.16; Kvco = 11.2 */
/* NOTE: All C values expressed here in pF (while F in Tcl) */
#define C1_PASSIVE_LF_CONST_FX          0x6833eeda173eULL
/* (2 * damp) / (kvco) in 0.0.32; kvco=11.2 */
#define R1_PASSIVE_LF_CONST_FX          0x2db6db6e
/* 5 * 256 * 1e-3 in 0.16.16 */
#define IOFF_REF_CONST_FX               0x147ae
/* max icp is 2.5mA in 0.16.16 */
#define ICP_MAX_FX                      0x28000

/* const for wn calculation
 * damp = 1
 * f3db_ovr_fn = sqrt(1+2* damp**2+sqrt(2+4*damp**2+4*damp**4))
 * wn = 2*pi * loop_bw / f3db_ovr_fn
 * wn_const = 2*pi / f3db_ovr_fn
 * wn = wn_const * loop_bw
 */
#define WN_CONST_FX  0xa1fd8938  /* 2.5310996100480279 in (0.2.30) */

/* const for Kvco calculation
 * Kvco = 2 * pi**2 * 455 * Fvco**3 * dCv * 0.041846 * 1e-12
 *      = 2 * pi**2 * 455 * 0.041846 = 375.83315384886663
 *      = ((2 * pi**2 * 455 * 0.041846 * 1e-12)**(1./3) * Fvco)**3 * dCv
 * Kvco_const = (2 * pi**2 * 455 * 0.041846 * 1e-12)**(1./3)
 * Kvco = (Kvco_const * VCO_frequency)**3 * dCv
 */
#define KVCO_CONST_FX 0x2f4b6b /* 0.0007216584415907027 in (0.0.32) */

/* const for icp calculation
 * Zlpf = 1999.0
 * icp_const = (2*pi) / (1.31*Zlpf) = 0.00239936201199057
 * icp = icp_const * loop_bw * divide_ratio / Kvco
 */
#define ICP_CONST_FX 0x9d3e9d    /* 0.00239936201199057 in (0.0.32) */

/* Constants */
#define VCO_CAL_AUX_CAP_BITS                    7
#define UP_CONVERSION_RATIO                     2
#define RFPLL_VCOCAL_TARGETCOUNTBASE            0
#define RFPLL_VCOCAL_OFFSETIN                   0
#define RFPLL_VCOCAL_SLOPEIN_DEC                0
#define RFPLL_VCOCAL_UPDATESEL_DEC              1
#define RFPLL_VCOCAL_CALCAPRBMODE_DEC           3
#define RFPLL_VCOCAL_FASTSWITCH_DEC             3
#define RFPLL_VCOCAL_FIXMSB_DEC                 3
#define RFPLL_VCOCAL_FOURTHMESEN_DEC            1
#define RFPLL_VCOCAL_TESTVCOCNT_DEC             0
#define RFPLL_VCOCAL_FAST_SETTLE_OVR_DEC        0
#define RFPLL_VCOCAL_FAST_SETTLE_OVRVAL_DEC     0
#define RFPLL_VCOCAL_FORCE_VCTRL_OVR_DEC        0
#define RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL_DEC     0
#define RFPLL_VCOCAL_TARGETCOUNTBASE_DEC        0
#define RFPLL_VCOCAL_COUPLINGMODE_DEC           1
#define RFPLL_VCOCAL_SWAPVCO12_DEC              0
#define RFPLL_VCOCAL_COUPLTHRES_DEC             6
#define RFPLL_VCOCAL_SECONDMESEN_DEC            3
#define RFPLL_VCOCAL_UPDATESELCOUP_DEC          0
#define RFPLL_VCOCAL_COUPLINGIN_DEC             0
#define RFPLL_VCOCAL_FORCE_AUX1_OVR_DEC         0
#define RFPLL_VCOCAL_FORCE_AUX1_OVRVAL_DEC      0
#define RFPLL_VCOCAL_FORCE_AUX2_OVR_DEC         0
#define RFPLL_VCOCAL_FORCE_AUX2_OVRVAL_DEC      0
#define RFPLL_VCO_CAP_MODE_DEC                  0
#define CAP_MULTIPLIER_RATIO                    8
#define CAP_MULTIPLIER_RATIO_PLUS_ONE           9

/* PLL loop bw in kHz */
#define LOOP_BW                     560   /* default PLL BW */
#define USE_DOUBLER                 1
/* VCO_SELECT: 0->VCO1; 1->VCO2; 2->VCO1+VCO2 */
#define VCO_SELECT                  2

/* No of fraction bits */
#define NF0      0
#define NF6      6
#define NF8      8
#define NF16    16
#define NF20    20
#define NF21    21
#define NF22    22
#define NF24    24
#define NF26    26
#define NF30    30
#define NF31    31
#define NF32    32

#define PRINT_PLL_CONFIG_20704(pll_struct, offset) \
	printf("%s = %u\n", #offset, (pll_struct->reg_field_val[IDX_20704_##offset]))

#define PLL_CONFIG_20704_VAL_ENTRY(pll_struct, offset, val) \
	pll_struct->reg_field_val[IDX_20704_##offset] = val

#define PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll_struct, pll_num, offset, reg, fld) \
	pll_struct->reg_addr[IDX_20704_##offset] = RADIO_PLLREG_20704(pi, reg, pll_num); \
	pll_struct->reg_field_mask[IDX_20704_##offset] = \
			RF_##20704##_##reg##_##fld##_MASK(pi->pubpi->radiorev); \
	pll_struct->reg_field_shift[IDX_20704_##offset] = \
			RF_##20704##_##reg##_##fld##_SHIFT(pi->pubpi->radiorev);

/* structure to hold computed PLL config values */
typedef struct {
	uint32 xtal_fx;
	uint32 loop_bw;
	uint8 use_doubler;
	uint8 vco_select;
	uint16 *reg_addr;
	uint16 *reg_field_mask;
	uint8 *reg_field_shift;
	uint16 *reg_field_val;
} pll_config_20704_tbl_t;

typedef enum {
	IDX_20704_OVR_RFPLL_VCOCAL_SLOPEIN,
	IDX_20704_OVR_RFPLL_VCOCAL_OFFSETIN,
	IDX_20704_OVR_RFPLL_VCOCAL_RST_N,
	IDX_20704_OVR_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20704_OVR_RFPLL_VCOCAL_XTALCOUNT,
	IDX_20704_OVR_RFPLL_VCOCAL_DELTAPLLVAL,
	IDX_20704_OVR_RFPLL_FRCT_WILD_BASE,
	IDX_20704_SEL_BAND,
	IDX_20704_RFPLL_VCOCAL_DELAYEND,
	IDX_20704_RFPLL_VCOCAL_DELAYSTARTCOLD,
	IDX_20704_RFPLL_VCOCAL_DELAYSTARTWARM,
	IDX_20704_RFPLL_VCOCAL_XTALCOUNT,
	IDX_20704_RFPLL_VCOCAL_CALCAPRBMODE,
	IDX_20704_RFPLL_VCOCAL_DELTAPLLVAL,
	IDX_20704_RFPLL_VCOCAL_ERRORTHRES,
	IDX_20704_RFPLL_VCOCAL_INITCAPA,
	IDX_20704_RFPLL_VCOCAL_INITCAPB,
	IDX_20704_RFPLL_VCOCAL_NORMCOUNTLEFT,
	IDX_20704_RFPLL_VCOCAL_NORMCOUNTRIGHT,
	IDX_20704_RFPLL_VCOCAL_PAUSECNT,
	IDX_20704_RFPLL_VCOCAL_ROUNDLSB,
	IDX_20704_RFPLL_VCOCAL_UPDATESEL,
	IDX_20704_RFPLL_VCOCAL_TESTVCOCNT,
	IDX_20704_RFPLL_VCOCAL_FAST_SETTLE_OVR,
	IDX_20704_RFPLL_VCOCAL_FAST_SETTLE_OVRVAL,
	IDX_20704_RFPLL_VCOCAL_FORCE_VCTRL_OVR,
	IDX_20704_RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL,
	IDX_20704_RFPLL_VCOCAL_TARGETCOUNTBASE,
	IDX_20704_RFPLL_VCOCAL_TARGETCOUNTCENTER,
	IDX_20704_RFPLL_VCOCAL_OFFSETIN,
	IDX_20704_RFPLL_VCOCAL_SLOPEIN,
	IDX_20704_RFPLL_VCOCAL_FASTSWITCH,
	IDX_20704_RFPLL_VCOCAL_FIXMSB,
	IDX_20704_RFPLL_VCOCAL_FOURTHMESEN,
	IDX_20704_RFPLL_VCO_CAP_MODE,
	IDX_20704_RFPLL_VCOCAL_PLL_VAL,
	IDX_20704_RFPLL_VCOCAL_FORCE_CAPS_OVR,
	IDX_20704_RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
	IDX_20704_RFPLL_FRCT_WILD_BASE_HIGH,
	IDX_20704_RFPLL_FRCT_WILD_BASE_LOW,
	IDX_20704_RFPLL_VCO_CVAR,
	IDX_20704_RFPLL_VCO_ALC_REF_CTRL,
	IDX_20704_RFPLL_VCO_BIAS_MODE,
	IDX_20704_RFPLL_VCO_TEMPCO_DCADJ,
	IDX_20704_RFPLL_VCO_TEMPCO,
	IDX_20704_RFPLL_LF_LF_R2,
	IDX_20704_RFPLL_LF_LF_R3,
	IDX_20704_RFPLL_LF_LF_RS_CM,
	IDX_20704_RFPLL_LF_LF_RF_CM,
	IDX_20704_RFPLL_LF_LF_C1,
	IDX_20704_RFPLL_LF_LF_C2,
	IDX_20704_RFPLL_LF_LF_C3,
	IDX_20704_RFPLL_LF_LF_C4,
	IDX_20704_RFPLL_CP_IOFF,
	IDX_20704_RFPLL_CP_KPD_SCALE,
	IDX_20704_RFPLL_VCO_PREFER,
	IDX_20704_RFPLL_VCO_CORE1_EN,
	IDX_20704_RFPLL_VCO_CORE2_EN,
	IDX_20704_RFPLL_VCOCAL_ENABLECOUPLING,
	IDX_20704_RFPLL_VCOCAL_COUPLINGMODE,
	IDX_20704_RFPLL_VCOCAL_SWAPVCO12,
	IDX_20704_RFPLL_VCOCAL_MIDCODESEL,
	IDX_20704_RFPLL_VCOCAL_SECONDMESEN,
	IDX_20704_RFPLL_VCOCAL_UPDATESELCOUP,
	IDX_20704_RFPLL_VCOCAL_COUPLTHRES,
	IDX_20704_RFPLL_VCOCAL_COUPLTHRES2,
	IDX_20704_RFPLL_VCOCAL_COUPLINGIN,
	IDX_20704_RFPLL_VCOCAL_FORCE_CAPS2_OVR,
	IDX_20704_RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
	IDX_20704_RFPLL_VCOCAL_FORCE_AUX1_OVR,
	IDX_20704_RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
	IDX_20704_RFPLL_VCOCAL_FORCE_AUX2_OVR,
	IDX_20704_RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
	PLL_CONFIG_20704_ARRAY_SIZE
} pll_config_20704_offset_t;

static void phy_ac_radio20704_write_pll_config(phy_info_t *pi, pll_config_20704_tbl_t *pll);
static void BCMATTACHFN(phy_ac_radio20704_pll_config_const_calc)(phy_info_t *pi,
		pll_config_20704_tbl_t *pll);
static void phy_ac_radio20704_pll_config_ch_dep_calc(phy_info_t *pi, uint32 lo_freq,
		uint8 ac_mode, pll_config_20704_tbl_t *pll);
#if DBG_PLL != 0
static void print_pll_config_20704(pll_config_20704_tbl_t *pll, uint32 lo_freq,
	uint32 loop_bw, uint32 icp_fx);
#endif // endif

/* Structure to hold 20704 PLL config values */
static pll_config_20704_tbl_t pll_conf_20704;

static pll_config_20704_tbl_t *
BCMRAMFN(wlc_phy_ac_get_20704_pll_config)(phy_info_t *pi)
{
	return &pll_conf_20704;
}

void
wlc_phy_radio20704_pll_tune(phy_info_t *pi, uint32 chan_freq)
{
	uint8 ac_mode;
	pll_config_20704_tbl_t *pll = wlc_phy_ac_get_20704_pll_config(pi);

	ac_mode = 1;
	phy_ac_radio20704_pll_config_ch_dep_calc(pi, chan_freq, ac_mode, pll);

	/* Write computed values to PLL registers */
	phy_ac_radio20704_write_pll_config(pi, pll);

	/* Turn on logen_top and logen_vcobuf */
	MOD_RADIO_PLLREG_20704(pi, LOGEN_REG0, 0, logen_pu, 1);
	MOD_RADIO_PLLREG_20704(pi, LOGEN_REG0, 0, logen_vcobuf_pu, 1);

}

void
BCMATTACHFN(phy_ac_radio20704_populate_pll_config_mfree)(phy_info_t *pi)
{
	pll_config_20704_tbl_t *pll = wlc_phy_ac_get_20704_pll_config(pi);

	if (pll->reg_addr != NULL) {
		phy_mfree(pi, pll->reg_addr, (sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE));
	}

	if (pll->reg_field_mask != NULL) {
		phy_mfree(pi, pll->reg_field_mask,
				(sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE));
	}

	if (pll->reg_field_shift != NULL) {
		phy_mfree(pi, pll->reg_field_shift,
				(sizeof(uint8) * PLL_CONFIG_20704_ARRAY_SIZE));
	}

	if (pll->reg_field_val != NULL) {
		phy_mfree(pi, pll->reg_field_val, (sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE));
	}
}

int
BCMATTACHFN(phy_ac_radio20704_populate_pll_config_tbl)(phy_info_t *pi)
{
	pll_config_20704_tbl_t *pll = wlc_phy_ac_get_20704_pll_config(pi);

	if ((pll->reg_addr =
		phy_malloc(pi, (sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_addr failed\n", __FUNCTION__));
		goto fail;
	}

	if ((pll->reg_field_mask =
		phy_malloc(pi, (sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_field_mask failed\n", __FUNCTION__));
		goto fail;
	}

	if ((pll->reg_field_shift =
		phy_malloc(pi, (sizeof(uint8) * PLL_CONFIG_20704_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_field_shift failed\n", __FUNCTION__));
		goto fail;
	}

	if ((pll->reg_field_val =
		phy_malloc(pi, (sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE))) == NULL) {
		PHY_ERROR(("%s: phy_malloc reg_field_val failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register addresses */
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, OVR_RFPLL_VCOCAL_SLOPEIN, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_slopeIn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, OVR_RFPLL_VCOCAL_OFFSETIN, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_offsetIn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, OVR_RFPLL_VCOCAL_RST_N, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_rst_n);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, OVR_RFPLL_VCOCAL_COUPLINGIN, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_couplingIn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, OVR_RFPLL_VCOCAL_XTALCOUNT, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_XtalCount);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, OVR_RFPLL_VCOCAL_DELTAPLLVAL, PLL_VCOCAL_OVR1,
			ovr_rfpll_vcocal_DeltaPllVal);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, OVR_RFPLL_FRCT_WILD_BASE, PLL_OVR1,
			ovr_rfpll_frct_wild_base);

	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, SEL_BAND, PLL_MUXSELECT_LINE,
			sel_band);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_DELAYEND, PLL_VCOCAL18,
			rfpll_vcocal_DelayEnd);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_DELAYSTARTCOLD, PLL_VCOCAL3,
			rfpll_vcocal_DelayStartCold);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_DELAYSTARTWARM, PLL_VCOCAL4,
			rfpll_vcocal_DelayStartWarm);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_XTALCOUNT, PLL_VCOCAL7,
			rfpll_vcocal_XtalCount);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_CALCAPRBMODE, PLL_VCOCAL7,
			rfpll_vcocal_CalCapRBMode);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_DELTAPLLVAL, PLL_VCOCAL8,
			rfpll_vcocal_DeltaPllVal);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_ERRORTHRES, PLL_VCOCAL20,
			rfpll_vcocal_ErrorThres);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_INITCAPA, PLL_VCOCAL12,
			rfpll_vcocal_InitCapA);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_INITCAPB, PLL_VCOCAL13,
			rfpll_vcocal_InitCapB);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_NORMCOUNTLEFT, PLL_VCOCAL10,
			rfpll_vcocal_NormCountLeft);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_NORMCOUNTRIGHT, PLL_VCOCAL11,
			rfpll_vcocal_NormCountRight);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_PAUSECNT, PLL_VCOCAL19,
			rfpll_vcocal_PauseCnt);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_ROUNDLSB, PLL_VCOCAL1,
			rfpll_vcocal_RoundLSB);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_UPDATESEL, PLL_VCOCAL1,
			rfpll_vcocal_UpdateSel);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_TESTVCOCNT, PLL_VCOCAL1,
			rfpll_vcocal_testVcoCnt);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FAST_SETTLE_OVR, PLL_VCOCAL1,
			rfpll_vcocal_fast_settle_ovr);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FAST_SETTLE_OVRVAL, PLL_VCOCAL1,
			rfpll_vcocal_fast_settle_ovrVal);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_VCTRL_OVR, PLL_VCOCAL1,
			rfpll_vcocal_force_vctrl_ovr);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL, PLL_VCOCAL1,
			rfpll_vcocal_force_vctrl_ovrVal);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_TARGETCOUNTBASE, PLL_VCOCAL6,
			rfpll_vcocal_TargetCountBase);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_TARGETCOUNTCENTER, PLL_VCOCAL9,
			rfpll_vcocal_TargetCountCenter);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_OFFSETIN, PLL_VCOCAL17,
			rfpll_vcocal_offsetIn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_SLOPEIN, PLL_VCOCAL15,
			rfpll_vcocal_slopeIn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FASTSWITCH, PLL_VCOCAL1,
			rfpll_vcocal_FastSwitch);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FIXMSB, PLL_VCOCAL1,
			rfpll_vcocal_FixMSB);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FOURTHMESEN, PLL_VCOCAL1,
			rfpll_vcocal_FourthMesEn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_CAP_MODE, PLL_VCO2,
			rfpll_vco_cap_mode);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_PLL_VAL, PLL_VCOCAL5,
			rfpll_vcocal_pll_val);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_CAPS_OVR, PLL_VCOCAL2,
			rfpll_vcocal_force_caps_ovr);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_CAPS_OVRVAL, PLL_VCOCAL2,
			rfpll_vcocal_force_caps_ovrVal);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_FRCT_WILD_BASE_HIGH, PLL_FRCT2,
			rfpll_frct_wild_base_high);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_FRCT_WILD_BASE_LOW, PLL_FRCT3,
			rfpll_frct_wild_base_low);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_CVAR, PLL_VCO2,
			rfpll_vco_cvar);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_ALC_REF_CTRL, PLL_VCO6,
			rfpll_vco_ALC_ref_ctrl);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_BIAS_MODE, PLL_VCO6,
			rfpll_vco_bias_mode);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_TEMPCO_DCADJ, PLL_VCO5,
			rfpll_vco_tempco_dcadj);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_TEMPCO, PLL_VCO4,
			rfpll_vco_tempco);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_R2,
		PLL_LF4, rfpll_lf_lf_r2);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_R3,
		PLL_LF5, rfpll_lf_lf_r3);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_RS_CM,
		PLL_LF7, rfpll_lf_lf_rs_cm);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_RF_CM,
		PLL_LF7, rfpll_lf_lf_rf_cm);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_C1,
		PLL_LF2, rfpll_lf_lf_c1);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_C2,
		PLL_LF2, rfpll_lf_lf_c2);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_C3,
		PLL_LF3, rfpll_lf_lf_c3);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_LF_LF_C4,
		PLL_LF3, rfpll_lf_lf_c4);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_CP_IOFF,
		PLL_CP4, rfpll_cp_ioff);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_CP_KPD_SCALE,
		PLL_CP4, rfpll_cp_kpd_scale);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_PREFER, PLL_VCO7,
		rfpll_vco_prefer);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_CORE1_EN, PLL_VCO7,
		rfpll_vco_core1_en);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCO_CORE2_EN, PLL_VCO7,
		rfpll_vco_core2_en);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_ENABLECOUPLING,
		PLL_VCOCAL24, rfpll_vcocal_enableCoupling);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_COUPLINGMODE,
		PLL_VCOCAL24, rfpll_vcocal_couplingMode);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_SWAPVCO12,
		PLL_VCOCAL24, rfpll_vcocal_swapVco12);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_MIDCODESEL,
		PLL_VCOCAL24, rfpll_vcocal_MidCodeSel);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_SECONDMESEN,
		PLL_VCOCAL24, rfpll_vcocal_SecondMesEn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_UPDATESELCOUP,
		PLL_VCOCAL24, rfpll_vcocal_UpdateSelCoup);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_COUPLTHRES,
		PLL_VCOCAL26, rfpll_vcocal_CouplThres);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_COUPLTHRES2,
		PLL_VCOCAL26, rfpll_vcocal_CouplThres2);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_COUPLINGIN,
		PLL_VCOCAL25, rfpll_vcocal_couplingIn);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_CAPS2_OVR,
		PLL_VCOCAL21, rfpll_vcocal_force_caps2_ovr);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
		PLL_VCOCAL21, rfpll_vcocal_force_caps2_ovrVal);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_AUX1_OVR,
		PLL_VCOCAL22, rfpll_vcocal_force_aux1_ovr);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
		PLL_VCOCAL22, rfpll_vcocal_force_aux1_ovrVal);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_AUX2_OVR,
		PLL_VCOCAL23, rfpll_vcocal_force_aux2_ovr);
	PLL_CONFIG_20704_REG_INFO_ENTRY(pi, pll, 0, RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
		PLL_VCOCAL23, rfpll_vcocal_force_aux2_ovrVal);

	/* Add frequency independent data */
	phy_ac_radio20704_pll_config_const_calc(pi, pll);

	return BCME_OK;

fail:
	if (pll->reg_addr != NULL) {
		phy_mfree(pi, pll->reg_addr, (sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE));
	}

	if (pll->reg_field_mask != NULL) {
		phy_mfree(pi, pll->reg_field_mask,
				(sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE));
	}

	if (pll->reg_field_shift != NULL) {
		phy_mfree(pi, pll->reg_field_shift,
				(sizeof(uint8) * PLL_CONFIG_20704_ARRAY_SIZE));
	}

	if (pll->reg_field_val != NULL) {
		phy_mfree(pi, pll->reg_field_val, (sizeof(uint16) * PLL_CONFIG_20704_ARRAY_SIZE));
	}

	return BCME_NOMEM;
}

static void
phy_ac_radio20704_write_pll_config(phy_info_t *pi, pll_config_20704_tbl_t *pll)
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
	for (i = 0; i < PLL_CONFIG_20704_ARRAY_SIZE; i++) {
		phy_utils_mod_radioreg(pi, reg_addr[i], reg_mask[i], (reg_val[i] << reg_shift[i]));
	}
}

static void
BCMATTACHFN(phy_ac_radio20704_pll_config_const_calc)(phy_info_t *pi, pll_config_20704_tbl_t *pll)
{
	/* 20704_procs.tcl r710814: 20704_pll_config */
	uint32 xtal_freq;
	uint32 xtal_fx;
	uint64 temp_64;
	uint8 nf;

	/* Default parameters to be used in chan dependent computation */
	pll->use_doubler = USE_DOUBLER;
	pll->loop_bw = LOOP_BW;
	pll->vco_select = VCO_SELECT;

	/* ------------------------------------------------------------------- */
	/* XTAL frequency in 0.8.24 */
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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_DELAYEND,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* <0.8.24> * <0.16.16> -> <0.16.16> */
	temp_64 = math_fp_mult_64(xtal_fx, RFPLL_VCOCAL_DELAYSTARTCOLD_FX, NF24, NF16, NF16);
	/* round(0.16.16, 16) -> <0.32.0> */
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_DELAYSTARTCOLD,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* <0.8.24> * <0.16.16> -> <0.16.16> */
	temp_64 = math_fp_mult_64(xtal_fx, RFPLL_VCOCAL_DELAYSTARTWARM_FX, NF24, NF16, NF16);
	/* round(0.16.16, 16) -> <0.32.0> */
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_DELAYSTARTWARM,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* <0.8.24> * <0.16.16> -> <0.16.16> */
	temp_64 = math_fp_mult_64(xtal_fx, RFPLL_VCOCAL_PAUSECNT_FX, NF24, NF16, NF16);
	/* round(0.16.16, 16) -> <0.32.0> */
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_PAUSECNT,
			(uint16)math_fp_round_64(temp_64, NF16));

	/* ------------------------------------------------------------------- */
	/* Put other register values to structure */
	PLL_CONFIG_20704_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_SLOPEIN, 1);
	PLL_CONFIG_20704_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_OFFSETIN, 1);
	PLL_CONFIG_20704_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_RST_N, 1);
	PLL_CONFIG_20704_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_COUPLINGIN, 1);
	PLL_CONFIG_20704_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_XTALCOUNT, 1);
	PLL_CONFIG_20704_VAL_ENTRY(pll, OVR_RFPLL_VCOCAL_DELTAPLLVAL, 1);
	PLL_CONFIG_20704_VAL_ENTRY(pll, OVR_RFPLL_FRCT_WILD_BASE, 1);
	PLL_CONFIG_20704_VAL_ENTRY(pll, SEL_BAND, 3);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_TARGETCOUNTBASE, RFPLL_VCOCAL_TARGETCOUNTBASE);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_OFFSETIN, RFPLL_VCOCAL_OFFSETIN);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_SLOPEIN, RFPLL_VCOCAL_SLOPEIN_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_UPDATESEL, RFPLL_VCOCAL_UPDATESEL_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_CALCAPRBMODE, RFPLL_VCOCAL_CALCAPRBMODE_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FASTSWITCH, RFPLL_VCOCAL_FASTSWITCH_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FIXMSB, RFPLL_VCOCAL_FIXMSB_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FOURTHMESEN, RFPLL_VCOCAL_FOURTHMESEN_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_TESTVCOCNT, RFPLL_VCOCAL_TESTVCOCNT_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FAST_SETTLE_OVR,
		RFPLL_VCOCAL_FAST_SETTLE_OVR_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FAST_SETTLE_OVRVAL,
		RFPLL_VCOCAL_FAST_SETTLE_OVRVAL_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVR,
		RFPLL_VCOCAL_FORCE_VCTRL_OVR_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL,
		RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_TARGETCOUNTBASE,
		RFPLL_VCOCAL_TARGETCOUNTBASE_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLINGMODE,
		RFPLL_VCOCAL_COUPLINGMODE_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_SWAPVCO12,
		RFPLL_VCOCAL_SWAPVCO12_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLTHRES,
		RFPLL_VCOCAL_COUPLTHRES_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_SECONDMESEN,
		RFPLL_VCOCAL_SECONDMESEN_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_UPDATESELCOUP,
		RFPLL_VCOCAL_UPDATESELCOUP_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLINGIN,
		RFPLL_VCOCAL_COUPLINGIN_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX1_OVR,
		RFPLL_VCOCAL_FORCE_AUX1_OVR_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX1_OVRVAL,
		RFPLL_VCOCAL_FORCE_AUX1_OVRVAL_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX2_OVR,
		RFPLL_VCOCAL_FORCE_AUX2_OVR_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_AUX2_OVRVAL,
		RFPLL_VCOCAL_FORCE_AUX2_OVRVAL_DEC);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_CAP_MODE, RFPLL_VCO_CAP_MODE_DEC);
}

static void
phy_ac_radio20704_pll_config_ch_dep_calc(phy_info_t *pi, uint32 lo_freq,
	uint8 ac_mode, pll_config_20704_tbl_t *pll)
{
	/* 20704_procs.tcl r710814: 20704_pll_config */
	uint32 icp_fx;
	uint32 kvco_fx;
	uint32 pfd_ref_freq_fx;
	uint8 nf;
	uint64 temp_64;
	uint32 temp_32;
	uint32 temp_32_1;
	uint32 dcv_fx;
	uint32 vco_freq_fx;
	uint32 divide_ratio_fx;
	uint32 ndiv_over_kvco_fx;
	uint8 enable_coupled_VCO;
	uint8 select_VCO;
	uint8 rfpll_vcocal_enablecoupling_dec;
	uint32 lo_div_vco_ratio_fx;
	uint32 rfpll_vcocal_XtalCount_raw_fx;
	uint32 rfpll_vcocal_TargetCountCenter = 0;
	uint32 rfpll_frct_wild_base_dec_fx;
	uint32 wn_fx;
	uint32 loop_band;
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
	uint16 rfpll_vcocal_force_caps_ovr_dec = 0;
	uint16 rfpll_vcocal_force_caps_ovrval_dec = 0;
	uint16 rfpll_vcocal_force_caps2_ovr_dec = 0;
	uint16 rfpll_vcocal_force_caps2_ovrval_dec = 0;

	ASSERT(ac_mode == 1);  /* Other mode no longer supported */

	/* ------------------------------------------------------------------- */
	/* PFD Reference Frequency */
	/* <0.8.24> * <0.1.0> --> <0.8.24> */
	pfd_ref_freq_fx = pll->xtal_fx << pll->use_doubler;
	/* ------------------------------------------------------------------- */

	/* ------------------------------------------------------------------- */
	/* VCO Frequency: 3453 < f < 3883 in (0,12,20) */
	/* In 2G: VCO = LO x 3/2 */
	/* In 5G: VCO = LO x 2/3  */
	/* <0.13.0> * <0.1.31> --> <0.12,20> */
	vco_freq_fx = (uint32)math_fp_mult_64(
		lo_freq,
		(lo_freq <= 3000)? CONST_3_OVER_2_FX : CONST_2_OVER_3_FX,
		NF0, NF31, NF20);
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
	/* <0.12.20> / <0.8.24> --> <0.(32 - nf).nf> */
	nf = math_fp_div_64(vco_freq_fx, pfd_ref_freq_fx, NF20, NF24, &divide_ratio_fx);
	/* floor(<0.(32-nf).nf>, (nf-21)) -> 0.11.21 */
	divide_ratio_fx = math_fp_round_32(divide_ratio_fx, (nf - NF21));

	/* <0.32.0> / <0.12.20> --> <0.(32 - nf).nf> */
	nf = math_fp_div_64(lo_freq, vco_freq_fx, NF0, NF20, &lo_div_vco_ratio_fx);
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

	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_XTALCOUNT,
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

	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_DELTAPLLVAL,
		rfpll_vcocal_DeltaPllVal_dec);

	/* Frequency range dependent constants */
	if (lo_freq >= 2000 && lo_freq <= 3000) {
		rfpll_vcocal_InitCapA = 1536;
		rfpll_vcocal_InitCapB = 1792;
		rfpll_vcocal_ErrorThres  = 3;
		rfpll_vcocal_TargetCountCenter  = 2442;
	} else if  (lo_freq < 5250) {
		rfpll_vcocal_InitCapA  = 2432;
		rfpll_vcocal_InitCapB  = 2688;
		rfpll_vcocal_ErrorThres = 5;
		rfpll_vcocal_TargetCountCenter = 5070;
	} else if (lo_freq >= 5250 && lo_freq < 5500) {
		rfpll_vcocal_InitCapA = 1792;
		rfpll_vcocal_InitCapB = 2048;
		rfpll_vcocal_ErrorThres = 5;
		rfpll_vcocal_TargetCountCenter = 5370;
	} else if (lo_freq >= 5500) {
		rfpll_vcocal_InitCapA = 1088;
		rfpll_vcocal_InitCapB = 1344;
		rfpll_vcocal_ErrorThres = 8;
		rfpll_vcocal_TargetCountCenter = 5700;
	}
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_ERRORTHRES, rfpll_vcocal_ErrorThres);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_INITCAPA, rfpll_vcocal_InitCapA);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_INITCAPB, rfpll_vcocal_InitCapB);

	if (lo_freq >= 2000 && lo_freq <= 3000) {
		rfpll_vcocal_NormCountLeft = -39;
		rfpll_vcocal_NormCountRight = 39;
		rfpll_vcocal_CouplThres2_dec = 25;
	} else {
		rfpll_vcocal_NormCountLeft = -50;
		rfpll_vcocal_NormCountRight = 50;
		rfpll_vcocal_CouplThres2_dec = 40;
	}

	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_NORMCOUNTLEFT,
			rfpll_vcocal_NormCountLeft);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_NORMCOUNTRIGHT,
			rfpll_vcocal_NormCountRight);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_COUPLTHRES2,
			rfpll_vcocal_CouplThres2_dec);

	/* <0.32.0> - <0.32.0> -> <0.32.0> */
	// pll->rfpll_vcocal_RoundLSB = 12 - VCO_CAL_CAP_BITS;
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_ROUNDLSB, (12 - VCO_CAL_CAP_BITS));

	/* <0.32.0> * <0.32.0> -> <0.32.0> */
	temp_64 = rfpll_vcocal_TargetCountCenter * rfpll_vcocal_DeltaPllVal_dec;
	/* <0.32.0> + round(<0.32.0> * <0.16.16>, 16) -> <0.32.0> */
	rfpll_vcocal_TargetCountCenter_dec = (uint16)(rfpll_vcocal_TargetCountCenter +
			math_fp_round_64((temp_64 * FIX_2POW_MIN13), NF16));
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_TARGETCOUNTCENTER,
			rfpll_vcocal_TargetCountCenter_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_PLL_VAL, (uint16)lo_freq);

	if (enable_coupled_VCO == 1) {
		rfpll_vcocal_enablecoupling_dec = 1;
	} else {
		rfpll_vcocal_enablecoupling_dec = 0;
	}
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_ENABLECOUPLING,
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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_MIDCODESEL, rfpll_vcocal_MidCodeSel_dec);
	/* ------------------------------------------------------------------- */
	/* VCO register */
	/* ------------------------------------------------------------------- */
	if (enable_coupled_VCO == 1) {
		rfpll_vco_core1_en_dec = 1;
		rfpll_vco_core2_en_dec = 1;
		rfpll_vco_prefer_dec = select_VCO;
		rfpll_vcocal_force_caps_ovr_dec = 0;
		rfpll_vcocal_force_caps_ovrval_dec = 0;
		rfpll_vcocal_force_caps2_ovr_dec = 0;
		rfpll_vcocal_force_caps2_ovrval_dec = 0;
	} else if (select_VCO == 0) {
		rfpll_vco_core1_en_dec = 1;
		rfpll_vco_core2_en_dec = 0;
		rfpll_vco_prefer_dec = 0;
		rfpll_vcocal_force_caps_ovr_dec = 0;
		rfpll_vcocal_force_caps_ovrval_dec = 0;
		rfpll_vcocal_force_caps2_ovr_dec = 1;
		rfpll_vcocal_force_caps2_ovrval_dec = 4095;
	} else if (select_VCO == 1) {
		rfpll_vco_core1_en_dec = 0;
		rfpll_vco_core2_en_dec = 1;
		rfpll_vco_prefer_dec = 0;
		rfpll_vcocal_force_caps_ovr_dec = 1;
		rfpll_vcocal_force_caps_ovrval_dec = 4095;
		rfpll_vcocal_force_caps2_ovr_dec = 0;
		rfpll_vcocal_force_caps2_ovrval_dec = 0;
	}
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_PREFER,
		rfpll_vco_prefer_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_CORE1_EN,
		rfpll_vco_core1_en_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_CORE2_EN,
		rfpll_vco_core2_en_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS_OVR,
		rfpll_vcocal_force_caps_ovr_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS_OVRVAL,
		rfpll_vcocal_force_caps_ovrval_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVR,
		rfpll_vcocal_force_caps2_ovr_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL,
		rfpll_vcocal_force_caps2_ovrval_dec);

	/* Fract-N (Sigma-Delta) register */
	rfpll_frct_wild_base_dec_fx = divide_ratio_fx;
	temp_32 = rfpll_frct_wild_base_dec_fx >> 16;
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_FRCT_WILD_BASE_HIGH,
			(uint16)(temp_32 & 0xFFFF));
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_FRCT_WILD_BASE_LOW,
			(uint16)(rfpll_frct_wild_base_dec_fx & 0xFFFF));

	/* ------------------------------------------------------------------- */
	/* VCO register (11n vs 11ac mode) */
	/* ------------------------------------------------------------------- */
	/* There is no Class-C VCO anymore. Always CMOS VCO */
	/* 2 VCO mode: 11abgn, 11ac */
	loop_band = LOOP_BW;
	rfpll_vco_cvar_dec = 9;
	rfpll_vco_ALC_ref_ctrl_dec = 9;
	rfpll_vco_bias_mode_dec = 1;
	rfpll_vco_tempco_dcadj_dec = 3;
	rfpll_vco_tempco_dec = 5;

	/* Special settings for spur affected channels */
	if (lo_freq == 5510) {
		loop_band = 340;
		rfpll_vco_cvar_dec = 8;
	}
	else if (lo_freq == 5590) {
		loop_band = 300;
		rfpll_vco_cvar_dec = 6;
	}
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_CVAR, rfpll_vco_cvar_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_ALC_REF_CTRL, rfpll_vco_ALC_ref_ctrl_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_BIAS_MODE, rfpll_vco_bias_mode_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_TEMPCO_DCADJ, rfpll_vco_tempco_dcadj_dec);
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_VCO_TEMPCO, rfpll_vco_tempco_dec);

	/* ----------------------------- */
	/* CP and Loop Filter Registers  */
	/* ----------------------------- */
	/* wn = wn_const * loop_bw
	 * wn in <0.12.20> (=1417.416 for loop_band=560)
	 */
	wn_fx = (uint32)math_fp_mult_64(WN_CONST_FX, loop_band, NF30, NF0, NF20);

	/* dCv = rfpll_vco_cvar*0.134068 + floor(vco_cvar_dec/8.0)*0.27314
	 * dCv in <0.6.26>
	 */
	temp_32 = (uint32)math_fp_mult_64(CONST_P134068_FX, rfpll_vco_cvar_dec,
		NF32, NF0, NF26);
	temp_32_1 = (uint32)math_fp_mult_64(CONST_P27314_FX, rfpll_vco_cvar_dec>>3,
		NF32, NF0, NF26);
	dcv_fx = temp_32 + temp_32_1;

	/* VCO Frequency: 3453 < f < 3883 in (0,12,20)
	 * Kvco_const = 0.000722
	 * Kvco = (Kvco_const * VCO_frequency)**3 * dCv
	 * Kvco in <0.6.26> ( 31.97 < Kvco < 35.96 )
	 */
	temp_32 = (uint32)math_fp_mult_64(KVCO_CONST_FX, vco_freq_fx, NF32, NF20, NF26);
	temp_32_1 = (uint32)math_fp_mult_64(temp_32, temp_32, NF26, NF26, NF26);
	temp_32 = (uint32)math_fp_mult_64(temp_32, temp_32_1, NF26, NF26, NF26);
	kvco_fx = (uint32)math_fp_mult_64(temp_32, dcv_fx, NF26, NF26, NF26);

	/* ndiv_over_kvco = divide_ratio / kvco
	 * ndiv_over_kvco in <0.16.16>  ( 1.1 < ndiv_over_kvco < 1.4 )
	 */
	nf = math_fp_div_64(divide_ratio_fx, kvco_fx, 21, 26, &ndiv_over_kvco_fx);
	ndiv_over_kvco_fx = math_fp_round_32(ndiv_over_kvco_fx, (nf - NF16));

	/* icp = icp_const * loop_bw * divide_ratio / Kvco
	 * icp in <0.16.16>  ( 1.48 < icp < 1.88 )
	 */
	temp_32 = (uint32)math_fp_mult_64(ndiv_over_kvco_fx, loop_band, NF16, NF0, NF16);
	icp_fx  = (uint32)math_fp_mult_64(temp_32, ICP_CONST_FX, NF16, NF32, NF16);
	if (icp_fx > ICP_MAX_FX)
		icp_fx = ICP_MAX_FX;

	/* r1_passive_lf = 2 * damp * wn * ndiv_over_kvco / icp
	 * r1_passive_lf in <0.16.16>
	 */
	nf = math_fp_div_64(wn_fx*2, icp_fx, NF20, NF16, &temp_32);
	temp_32 = math_fp_round_32(temp_32, (nf - NF16));
	r1_passive_lf_fx = (uint32)math_fp_mult_64(temp_32, ndiv_over_kvco_fx, NF16, NF16, NF16);

	/* c1_passive_lf = icp / (wn**2 * ndiv_over_kvco) * 1e9
	 * c1_passive_lf in <0.16.16>
	 */
	temp_32 = (uint32)math_fp_mult_64(wn_fx, wn_fx, NF20, NF20, NF8);
	temp_32 = (uint32)math_fp_mult_64(ndiv_over_kvco_fx, temp_32, NF16, NF8, NF8);
	temp_32_1 = (uint32)math_fp_mult_64(icp_fx, CONST_1E9_FX, NF16, NF0, NF0);
	nf = math_fp_div_64(temp_32_1, temp_32, NF0, NF8, &c1_passive_lf_fx);
	c1_passive_lf_fx = math_fp_round_32(c1_passive_lf_fx, (nf - NF16));

	nf = math_fp_div_64(c1_passive_lf_fx, CAP_MULTIPLIER_RATIO_PLUS_ONE, NF16, NF0,
			&c1_cap_multiplier_mode_fx);
	/* round(<0.(32-nf).nf>, (nf-16)) -> <0.16.16> */
	c1_cap_multiplier_mode_fx = math_fp_round_32(c1_cap_multiplier_mode_fx, (nf - NF16));

	rf_fx = math_fp_mult_64(r1_passive_lf_fx, RF_CONST1_FX, NF16, NF16, NF16);
	/* <0.32.0> * <0.16.16> -> <0.16.16> */
	rs_fx = CAP_MULTIPLIER_RATIO * rf_fx;
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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_R2, rfpll_lf_lf_r2_ideal);

	/* rfpll_lf_lf_r3 */
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_R3, rfpll_lf_lf_r2_ideal);

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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_RS_CM, rfpll_lf_lf_rs_cm_ideal);

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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_RF_CM, rfpll_lf_lf_rf_cm_ideal);

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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_C1, rfpll_lf_lf_c1_ideal);

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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_C2, rfpll_lf_lf_c2_ideal);

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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_C3, rfpll_lf_lf_c3_ideal);

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
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_LF_LF_C4, rfpll_lf_lf_c4_ideal);

	/* cp_kpd_scale_ref = i_cp
	 * cp_kpd_scale_dec = (cp_kpd_scale_ref**2 * 28.989 +
	 *                     cp_kpd_scale_ref*1607.45 -
	 *                     13.151136) / 64
	 * cp_kpd_scale_dec in <0.32.0>
	 */
	rfpll_cp_kpd_scale_ref_fx = icp_fx;
	temp_32 = (uint32)math_fp_mult_64(rfpll_cp_kpd_scale_ref_fx, rfpll_cp_kpd_scale_ref_fx,
		NF16, NF16, NF16);
	temp_32 = (uint32)math_fp_mult_64(CONST_28P989_FX, temp_32,
		NF16, NF16, NF16);
	temp_32_1 = (uint32)math_fp_mult_64(CONST_1607P45_FX, rfpll_cp_kpd_scale_ref_fx,
		NF16, NF16, NF16);
	temp_32 = temp_32 + temp_32_1 - CONST_13P151136_FX;
	rfpll_cp_kpd_scale_dec_fx = math_fp_round_32(temp_32, NF22);
	rfpll_cp_kpd_scale_ideal = (uint16)(LIMIT((int32)rfpll_cp_kpd_scale_dec_fx,
			0, 127));
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_CP_KPD_SCALE, rfpll_cp_kpd_scale_ideal);

	/* rfpll_cp_ioff */
	/* <0.16.16> * <0.8.24> -> <0.16.16> */
	temp_32 = (uint32)math_fp_mult_64(T_OFF_FX, pfd_ref_freq_fx, NF16, NF24, NF16);
	/* <0.16.16> * <0.16.16> -> <0.16.16> */
	i_off_fx = (uint32)math_fp_mult_64(temp_32, IOFF_REF_CONST_FX, NF16, NF16, NF16);
	/* round(<0.16.16>) -> <0.32.0>  */
	rfpll_cp_ioff_dec_fx = math_fp_round_32(i_off_fx, NF16);
	rfpll_cp_ioff_ideal = (uint16)(LIMIT((int32)rfpll_cp_ioff_dec_fx, 0, 255));
	PLL_CONFIG_20704_VAL_ENTRY(pll, RFPLL_CP_IOFF, rfpll_cp_ioff_ideal);
	/* ------------------------------------------------------------------- */

#if DBG_PLL != 0
		print_pll_config_20704(pll, lo_freq, loop_band, icp_fx);
#endif /* DBG_PLL */
}

#if DBG_PLL != 0
static void
print_pll_config_20704(pll_config_20704_tbl_t *pll, uint32 lo_freq, uint32 loop_bw, uint32 icp_fx)
{
	printf("------------------------------------------------------------\n");
	printf("\nLO_Freq = %d MHz\n", lo_freq);
	printf("\nLoopFilter_bandwidth = %d kHz\n", loop_bw);
	printf("\nIcp_fx = %u (0.16.16 format)\n", icp_fx);
	printf("\nRegister Definition (upper):\n");
	printf("------------------------------------------------------------\n");

	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_DELAYEND);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_DELAYSTARTCOLD);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_DELAYSTARTWARM);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_XTALCOUNT);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_DELTAPLLVAL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_ERRORTHRES);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FASTSWITCH);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FIXMSB);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FOURTHMESEN);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_INITCAPA);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_INITCAPB);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_NORMCOUNTLEFT);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_NORMCOUNTRIGHT);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_PAUSECNT);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_ROUNDLSB);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_TARGETCOUNTBASE);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_TARGETCOUNTCENTER);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_OFFSETIN);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_PLL_VAL);

	printf("\nother VCOCAL register (not channel/XTAL dependent):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_SLOPEIN);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_UPDATESEL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_CALCAPRBMODE);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_TESTVCOCNT);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_CAPS_OVR);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_CAPS_OVRVAL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FAST_SETTLE_OVR);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FAST_SETTLE_OVRVAL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVR);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_VCTRL_OVRVAL);

	printf("\nVCOCAL register (required by coupled VCOs):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_ENABLECOUPLING);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_COUPLINGMODE);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_SWAPVCO12);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_MIDCODESEL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_COUPLTHRES);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_COUPLTHRES2);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_SECONDMESEN);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_UPDATESELCOUP);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_COUPLINGIN);

	printf("\nVCOCAL register (optional for coupled VCOs):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVR);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_CAPS2_OVRVAL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_AUX1_OVR);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_AUX1_OVRVAL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_AUX2_OVR);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCOCAL_FORCE_AUX2_OVRVAL);

	printf("\nVCO register:\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_CORE1_EN);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_CORE2_EN);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_PREFER);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_CAP_MODE);

	printf("\nFract-N (Sigma-Delta) register:\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20704(pll, RFPLL_FRCT_WILD_BASE_HIGH);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_FRCT_WILD_BASE_LOW);

	printf("\nVCO Register (11n vs 11ac mode):\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_CVAR);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_ALC_REF_CTRL);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_BIAS_MODE);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_TEMPCO_DCADJ);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_VCO_TEMPCO);

	printf("\nCP and Loop Filter register:\n");
	printf("------------------------------------------------------------\n");
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_R2);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_R3);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_RS_CM);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_RF_CM);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_C1);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_C2);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_C3);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_LF_LF_C4);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_CP_KPD_SCALE);
	PRINT_PLL_CONFIG_20704(pll, RFPLL_CP_IOFF);
}
#endif /* DBG_PLL */
