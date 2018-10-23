/*
 * ACPHY Envelope Tracking module implementation
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
 * $Id$
 */
#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_et.h"
#include <phy_et.h>
#include <phy_utils_reg.h>
#include <phy_utils_var.h>

#include <phy_ac.h>
#include <phy_ac_info.h>
#include <phy_ac_et.h>

#include <wlc_phy_radio.h>
#include <wlc_radioreg_20695.h>
#include <wlc_phyreg_ac.h>

#include <hndd11.h>
#include <hndd11.h>

#include <phy_ac_papdcal.h>
#include "phy_ac_et_data.h"
#include <phy_rstr.h>

/* module private states */
struct phy_ac_et_info {
	phy_info_t		*pi;
	phy_ac_info_t	*aci;
	phy_et_info_t	*cmn_info;
};

/* Local Function Declarations */
#ifdef WL_ETMODE
static void phy_ac_et_write_radiodig_LUT(phy_info_t *pi);
static void phy_ac_et_write_radioreg_cmn(phy_info_t *pi);
static bool wlc_phy_et_attach(phy_ac_et_info_t *eti);
#endif /* WL_ETMODE */

/* register phy type specific implementation */
phy_ac_et_info_t *
BCMATTACHFN(phy_ac_et_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_et_info_t *cmn_info)
{
	phy_ac_et_info_t *eti;
	phy_type_et_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((eti = phy_malloc(pi, sizeof(phy_ac_et_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	eti->pi = pi;
	eti->aci = aci;
	eti->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = eti;

	if (phy_et_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_et_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* PHY-Feature specific parameter initialization */
#ifdef WL_ETMODE
	if (!wlc_phy_et_attach(eti))
		goto fail;
#endif /* WL_ETMODE */

	return eti;

	/* error handling */
fail:
	if (eti != NULL)
		phy_mfree(pi, eti, sizeof(phy_ac_et_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_et_unregister_impl)(phy_ac_et_info_t *eti)
{
	phy_info_t *pi;
	phy_et_info_t *cmn_info;

	ASSERT(eti);
	pi = eti->pi;
	cmn_info = eti->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_et_unregister_impl(cmn_info);

	phy_mfree(pi, eti, sizeof(phy_ac_et_info_t));
}

#ifdef WL_ETMODE
static bool
BCMATTACHFN(wlc_phy_et_attach)(phy_ac_et_info_t *eti)
{
	eti->pi->ff->_et = (uint16) (PHY_GETINTVAR_DEFAULT(eti->pi, rstr_et_mode, 0));
	return TRUE;
}

/* ET calibration module/function */
int
phy_ac_et(phy_info_t *pi)
{
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	int err = BCME_OK;
	bool suspend = FALSE;
	uint8 core = 0;
	int etflag;
	uint16 val16;

	/* Currently in Driver we assume only ET or LDO mode */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* The first nibble of the etmode is for 2G */
		etflag = (int8) (pi->ff->_et & 0x0f);
	} else {
		/* The second nibble of the etmode is for 5G */
		etflag = (int8) ((pi->ff->_et & 0xf0) >> 4);
	}

	/* Suspend MAC if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, rf_pwr_cnvr_fctr, core, rf_pwr_cnvr_fctr, 26);
		MOD_PHYREGCE(pi, V0, core, V0, 26);
		MOD_PHYREGCE(pi, et_en, core, et_en, 1);
		MOD_PHYREGCE(pi, et_delay_offset, core, et_delay_offset_en, 0);
		MOD_PHYREGCE(pi, et_delay_offset, core, et_delay_offset, 1);
		MOD_PHYREGCE(pi, et_delay_offset, core, comp_delay_offset_en, 1);
		MOD_PHYREGCE(pi, et_delay_offset, core, comp_delay_offset, 0xf);
		MOD_PHYREG(pi, log_step, lut_step, 2600);
		MOD_PHYREG(pi, cordicscl, cordicscl, 0x1d5); /* 0x1bb+26 */

		/* Shaping table */
		ACPHY_DISABLE_STALL(pi);
		wlc_phy_table_write_acphy_dac_war(pi, ACPHY_TBL_ID_ET_SHAPING_LUT, 256, 0, 16,
			&acphy_et_shaping_lut, core);
		ACPHY_ENABLE_STALL(pi, stall_val);

		/* etflag = 1 (LDO mode, i.e., fixed Vdd. No Env tracking)
		 * etflag = 2 (Envelope Tracking mode) if (etflag >= 2 is ET)
		 */
		/* Currently only RF LUT mode is ported to Hornet */
		/* Offset 0x4D0 in RFSeq table is rx2tx_entx_et_lut(0) */
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x4D0, 16, &val16);
		/* The values to be anded and ored are taken directly from TCL as of now */
		val16 = val16 & 0xf0fb;
		val16 = val16 | 0x0a00;

		if (etflag == 1) {
			/* ET LDO mode. Fixed VDDD and no envelope tracking */
			val16 = val16 | 0x4;

		} else if (etflag == 2) {
			/* Envelope tracking mode. */
			MOD_PHYREG(pi, fdf_delay0, fdf_delay, 0x21f);
		} else {
			/* ET Not enabled or Error condition */
			/* Currently defaulting to LDO mode */
			val16 = val16 | 0x4;
		}

		/* Write to RF Seq table */
		/* Offset 0x4D0 in RFSeq table is rx2tx_entx_et_lut(0) */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x4D0, 16, &val16);

		/* Write RF LUT */
		phy_ac_et_write_radiodig_LUT(pi);

		/* Write Radio Reg common registers */
		phy_ac_et_write_radioreg_cmn(pi);
	}

	/* Resume MAC */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
return err;
}

static void phy_ac_et_write_radiodig_LUT(phy_info_t *pi)
{
	/* The values for each LUT reg is taken from TCL */
	RADIO_REG_LIST_START
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_0, 0, 0x01ea)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_4, 0, 0x0777)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_5, 0, 0x01c7)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_6, 0, 0x0007)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_7, 0, 0x03f3)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_8, 0, 0x03f3)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_9, 0, 0x0007)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_10, 0, 0x0007)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_11, 0, 0x015a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_12, 0, 0x001a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_13, 0, 0x015a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_14, 0, 0x001a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_15, 0, 0x000a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_16, 0, 0x000a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_17, 0, 0x0189)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_18, 0, 0x0009)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_19, 0, 0x0084)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_20, 0, 0x0004)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_21, 0, 0x0186)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_22, 0, 0x0006)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_23, 0, 0x00ca)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_24, 0, 0x000a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_25, 0, 0x00ca)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_26, 0, 0x000a)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_27, 0, 0x0514)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_28, 0, 0x0014)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_29, 0, 0x0fff)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_30, 0, 0x0514)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_31, 0, 0x0014)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_32, 0, 0x0004)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_33, 0, 0x0004)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_34, 0, 0x0006)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_35, 0, 0x0006)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_36, 0, 0x0004)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_37, 0, 0x0004)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_38, 0, 0x0006)
		WRITE_RADIO_REG_28NM_ENTRY(pi, RF, ET_LUT_39, 0, 0x0006)
	RADIO_REG_LIST_EXECUTE(pi, 0);
}

static void phy_ac_et_write_radioreg_cmn(phy_info_t *pi)
{
	WRITE_RADIO_REG_28NM(pi, RF, ET_SPARE0, 0, 0x0);
	WRITE_RADIO_REG_28NM(pi, RF, ET_SPARE1, 0, 0x0);

	if (RADIOMAJORREV(pi) >= 2) {
		MOD_RADIO_REG_28NM(pi, RF, ET_LOCALVREF, 0, ET_localvref_res_MSB, 0xf);
	}

	ACPHY_REG_LIST_START
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_RC_FILT_CFG, 0, ET_bypass_RC_linregin, 0x1)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_RC_FILT_CFG, 0, ET_sel_res_RC_linregin, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_RC_FILT_CFG, 0, ET_sel_cap_RC_linregin, 0x0)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CBUCK_PWSW, 0, ET_Cbuck_pwsw_ctrl, 0x2)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS2, 0, ibias_20uA_linreg_stage1_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS2, 0, ibias_20uA_linreg_ClassAB_nmos_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS2, 0, ibias_20uA_linreg_vmid_gen_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS2, 0, ibias_20uA_linreg_vmid_gen_ClassAB_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS4, 0, ibias_20uA_LDO_1P8V_mag, 0x6)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS5, 0, ibias_20uA_linreg_predriver_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS5, 0, ibias_20uA_LDO_1P8V_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS5, 0, isrc_20uA_hystcomp_thresh_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS5, 0, isink_20uA_hystcomp_thresh_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS8, 0, ibias_20uA_LDO_1P8V_swreg_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS8, 0, ibias_20uA_LDO_1P8V_swreg_classAB_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS8, 0, ibias_20uA_swreg_vmid_gen_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS8, 0, ibias_20uA_swreg_vmid_gen_ClassAB_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS10, 0, ibias_uncal_20uA_LDO_1P8V_swreg_mag, 0x14)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS10, 0, ibias_uncal_20uA_LDO_1P8V_linreg_mag, 0x14)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS11, 0, ibias_uncal_20uA_LDO_1P8V_swreg_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS11, 0, ibias_uncal_20uA_LDO_1P8V_linreg_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS11, 0, ibias_uncal_20uA_swreg_vmid_gen_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS11, 0, ibias_uncal_20uA_linreg_vmid_gen_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS12, 0, ibias_uncal_20uA_swreg_vmid_gen_mag, 0x14)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS13, 0,
			ibias_uncal_24uA_Cbuck_pwsw_vref_gen_mag, 0x18)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS13, 0,
			ibias_uncal_24uA_Cbuck_pwsw_vref_gen_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_BIAS14, 0, ibias_20uA_linreg_ClassAB_pmos_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_LDO, 0, ETLDO_lowcurrent_RX_enable, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_LDO, 0, ETLDO_ovrprotect_enable, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_MONITOR, 0, ET_1p8V_LDO_linreg_gpaio_test_en, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_MONITOR, 0, ET_1p8V_LDO_swreg_gpaio_test_en, 0x0)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_PU_MODES, 0, pa_cas_ref_pu, 0x1)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_PU_MODES, 0, ET_PA_casbias_pu, 0x1)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_PU_MODES, 0, ET_midcasc_bias_sel, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_PU_MODES, 0, ET_midcasc_bias_sel_ovr, 0x0)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SHAPE, 0, ET_PA_casbias_vcas_dc_up_down, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SHAPE, 0, ET_PA_vcas_slope, 0xa)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SHAPE, 0, ET_PA_casbias_vcas_dc_idac, 0xa)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_PREDRIVER, 0, ET_cfb_casc_predriver_tune, 0x2)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_PREDRIVER, 0, ET_casc_predriver_capmiller, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_PREDRIVER, 0, ET_casc_predriver_fb_R2tune, 0x5)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_1, 0,
			ET_1p8V_LDO_casc_gpaio_test_en, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_1, 0,
			ETcasc_ibias_20uA_LDO_1P8V_mag, 0x16)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_1, 0,
			ETcasc_ibias_20uA_LDO_1P8V_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_2, 0,
			ibias_uncal_20uA_LDO_1P8V_casc_mag, 0x14)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_2, 0,
			ibias_uncal_20uA_LDO_1P8V_casc_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_3, 0,
			ET_2p5V_LDO_casc_gpaio_test_en, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_3, 0,
			ETcasc_ibias_20uA_LDO_2P5V_mag, 0x16)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_3, 0,
			ETcasc_ibias_20uA_LDO_2P5V_slope, 0x14)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_4, 0,
			ibias_uncal_20uA_LDO_2P5V_casc_mag, 0x14)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_LDO_BIAS_4, 0,
			ibias_uncal_20uA_LDO_2P5V_casc_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_1, 0,
			ETcasc_ibias_20uA_casc_predriver_mag, 0x16)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_1, 0,
			ETcasc_ibias_20uA_casc_predriver_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_1, 0,
			ETcasc_ibias_20uA_casc_small_ota_mag, 0x16)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_2, 0,
			ETcasc_ibias_20uA_casc_small_ota_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_2, 0,
			ETcasc_ibias_20uA_casc_bias_gen_mag, 0x16)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_2, 0,
			ETcasc_ibias_20uA_casc_bias_gen_slope, 0x0)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_3, 0,
			ETcasc_ibias_highside_clampOTA_midcasc_mag, 0x16)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_3, 0,
			ETcasc_ibias_highside_clampOTA_midcasc_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_3, 0,
			ETcasc_ibias_lowside_clampOTA_midcasc_mag, 0x16)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_4, 0,
			ETcasc_ibias_lowside_clampOTA_midcasc_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_4, 0, ETcasc_ibias_20uA_dc_up_down_mag, 0x16)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_4, 0, ETcasc_ibias_20uA_dc_up_down_slope, 0x7)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_5, 0,
			ETcasc_iref_uncal_highside_clamp_midcasc_mag, 0x16)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_5, 0,
			ETcasc_iref_uncal_highside_clamp_midcasc_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_5, 0,
			ETcasc_iref_uncal_lowside_clamp_midcasc_mag, 0x16)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_6, 0,
			ETcasc_iref_uncal_lowside_clamp_midcasc_slope, 0x7)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_6, 0, PAcasref_uncal_mag, 0x29)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_BIAS_6, 0, PAcasref_uncal_slope, 0x0)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SPARE_1, 0, ETcascmod_spare0, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SPARE_1, 0, ETcascmod_spare1, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SPARE_1, 0, ETcascmod_spare2, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SPARE_1, 0, ETcascmod_spare3, 0x0)

	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SPARE_2, 0, ETcascmod_spare4, 0x0)
	MOD_RADIO_REG_28NM_ENTRY(pi, RF, ET_CASC_SPARE_2, 0, ETcascmod_spare5, 0x0)
	ACPHY_REG_LIST_EXECUTE(pi);
}
#endif /* WL_ETMODE */
