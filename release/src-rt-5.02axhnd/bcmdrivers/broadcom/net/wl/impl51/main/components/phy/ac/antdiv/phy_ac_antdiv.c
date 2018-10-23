/*
 * ACPHY ANTennaDIVersity module implementation
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
 * $Id: phy_ac_antdiv.c 702070 2017-05-30 15:43:06Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_antdiv.h"
#include <phy_ac.h>
#include <phy_antdiv.h>
#include <phy_antdiv_cfg.h>
#include <phy_ac_antdiv.h>
#include <phy_ac_info.h>
#include <phy_utils_reg.h>
#include <sbchipc.h>
#include <sbgci.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_var.h>
#include <phy_rstr.h>
#ifdef WLC_TXPWRCAP
#include <phy_ac_txpwrcap.h>
#include <phy_txpwrcap_api.h>
#include <phy_type_txpwrcap.h>
#endif // endif
#include <phy_antdiv_api.h>

/* private definitions */
#define ACPHY_TXPOWERCAP_SWDIV_SHM_OFFSET	(8)	/* txpwrcap offset in shmem */
#define ACPHY_SWDIV_PREFANT_SHM_MASK	(0xFF)

/* forward declaration */
typedef struct phy_ac_antdiv_mem phy_ac_antdiv_mem_t;

/* module private states */
struct phy_ac_antdiv_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_antdiv_info_t *di;
	phy_ac_antdiv_mem_t *mem;
	phy_swdiv_t *swdiv;
	uint8	ant_swOvr_state_core0;
	uint8	ant_swOvr_state_core1;
	uint8	antdiv_rfswctrlpin_a0;
	uint8	antdiv_rfswctrlpin_a1;
	int8	pa_mode; /* Modes: High Efficiency, High Linearity */
};

/* module private states memory layout */
struct phy_ac_antdiv_mem {
	phy_ac_antdiv_info_t info;
	phy_swdiv_t swdiv;
};

/* local functions */
static void phy_ac_antdiv_nvram_attach(phy_ac_antdiv_info_t *info, phy_info_t * pi);
static void phy_ac_antdiv_std_params(phy_ac_antdiv_info_t *info);
static void phy_ac_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant);
static void phy_ac_antdiv_set_sw_control(phy_type_antdiv_ctx_t *ctx, int8 divOvrride, int core);
static void phy_ac_antdiv_get_sw_control(phy_type_antdiv_ctx_t *ctx, int32 *ret_int_ptr, int core);
static uint32 si_gci_chipstatus_acphy(si_t *sih, uint reg);
static int phy_ac_antdiv_set_txswctrlmap(phy_type_antdiv_ctx_t *ctx, int32 int_val);
static int phy_ac_antdiv_get_txswctrlmap(phy_type_antdiv_ctx_t *ctx, int32 *ret_int_ptr);

#ifdef WLC_SW_DIVERSITY
#ifndef WLC_SW_DIVERSITY_DISABLED
static void wlc_phy_swdiv_attach_acphy(phy_info_t *pi);
#endif // endif
static void phy_acphy_swdiv_init(phy_type_antdiv_ctx_t *ctx);
static void phy_acphy_swdiv_set_ant(phy_type_antdiv_ctx_t *ctx, uint8 new_ant);
static uint8 phy_acphy_swdiv_get_ant(phy_type_antdiv_ctx_t *ctx);
static void wlc_ant_div_sw_control(phy_type_antdiv_ctx_t *ctx, uint8 divOvrride, int core);
void wlc_phy_set_femctrl_control_reg(phy_type_antdiv_ctx_t *ctx);
static int32 wlc_ant_div_sw_control_ovr(phy_type_antdiv_ctx_t *ctx, uint8 divOvrride, int core);
static uint16 wlc_phy_swdiv_get_rxantmap(phy_type_antdiv_ctx_t *ctx);
static uint16 phy_acphy_antmap_gpio_conv(phy_info_t *pi, phy_swdiv_t *swdiv, uint16 map);
#endif /* WLC_SW_DIVERSITY */

/* register phy type specific implementation */
phy_ac_antdiv_info_t *
BCMATTACHFN(phy_ac_antdiv_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_antdiv_info_t *di)
{
	phy_ac_antdiv_mem_t  *mem;
	phy_ac_antdiv_info_t *info;
	phy_type_antdiv_fns_t fns;
#ifdef WLC_SW_DIVERSITY
	phy_swdiv_t *swdiv;
#endif // endif

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((mem = phy_malloc(pi, sizeof(phy_ac_antdiv_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(mem, sizeof(phy_ac_antdiv_mem_t));

	info = &(mem->info);
	info->pi = pi;
	info->aci = aci;
	info->di = di;
	info->mem = mem;

#if defined(WLC_SW_DIVERSITY) && !defined(WLC_SW_DIVERSITY_DISABLED)
	wlc_phy_swdiv_attach_acphy(pi);
#endif /* WLC_SW_DIVERSITY */

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#ifdef WLC_SW_DIVERSITY
	if (PHYSWDIV_ENAB(pi)) {
		fns.initswdiv = phy_acphy_swdiv_init;
		fns.getswdiv = phy_acphy_swdiv_get_ant;
		fns.setswdiv = phy_acphy_swdiv_set_ant;
	}
#endif // endif
	fns.setrx = phy_ac_antdiv_set_rx;
	fns.set_sw_control = phy_ac_antdiv_set_sw_control;
	fns.get_sw_control = phy_ac_antdiv_get_sw_control;
	fns.set_txswctrlmap = phy_ac_antdiv_set_txswctrlmap;
	fns.get_txswctrlmap = phy_ac_antdiv_get_txswctrlmap;
	fns.ctx = info;

	phy_ac_antdiv_nvram_attach(info, pi);
	phy_ac_antdiv_std_params(info);
#ifdef WLC_SW_DIVERSITY
	if (PHYSWDIV_ENAB(pi)) {
		swdiv = &(mem->swdiv);
		info->swdiv = swdiv;
		phy_swdiv_read_srom(pi, swdiv);
	}
#endif /* WLC_SW_DIVERSITY */

	phy_antdiv_register_impl(di, &fns);

	return info;

	/* error handling */
fail:
	if (mem != NULL)
		phy_mfree(pi, mem, sizeof(phy_ac_antdiv_mem_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_antdiv_unregister_impl)(phy_ac_antdiv_info_t *info)
{
	phy_info_t *pi;
	phy_antdiv_info_t *di;

	ASSERT(info);
	pi = info->pi;
	di = info->di;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_antdiv_unregister_impl(di);

	phy_mfree(pi, info->mem, sizeof(phy_ac_antdiv_mem_t));
}

static void
BCMATTACHFN(phy_ac_antdiv_nvram_attach)(phy_ac_antdiv_info_t *antdivi, phy_info_t * pi)
{
	if ((PHY_GETVAR_SLICE(pi, rstr_antdiv_rfswctrlpin_a0)) != NULL) {
		antdivi->antdiv_rfswctrlpin_a0 = (uint8)PHY_GETINTVAR_SLICE(pi,
			rstr_antdiv_rfswctrlpin_a0);
	} else {
		antdivi->antdiv_rfswctrlpin_a0 = (uint8)ANTDIV_RFSWCTRLPIN_UNDEFINED;
	}
	if ((PHY_GETVAR_SLICE(pi, rstr_antdiv_rfswctrlpin_a1)) != NULL) {
		antdivi->antdiv_rfswctrlpin_a1 = (uint8)PHY_GETINTVAR_SLICE(pi,
			rstr_antdiv_rfswctrlpin_a1);
	} else {
		antdivi->antdiv_rfswctrlpin_a1 = (uint8)ANTDIV_RFSWCTRLPIN_UNDEFINED;
	}
	/* PA Mode is set so that NVRAM values are used by default */
	antdivi->pa_mode = AUTO;
}

static void
BCMATTACHFN(phy_ac_antdiv_std_params)(phy_ac_antdiv_info_t *antdivi)
{
	antdivi->ant_swOvr_state_core0 = 2;
	antdivi->ant_swOvr_state_core1 = 2;
}

/* Setup */
static void
phy_ac_antdiv_set_rx(phy_type_antdiv_ctx_t *ctx, uint8 ant)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: ant 0x%x\n", __FUNCTION__, ant));

	if (!wlc_phy_check_antdiv_enable_acphy(pi))
		return;

	if (ant > ANT_RX_DIV_FORCE_1)
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, MHF1_ANTDIV, WLC_BAND_ALL);
	else
		wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, 0, WLC_BAND_ALL);

	wlc_phy_antdiv_acphy(pi, ant);
}

static void
wlc_ant_div_sw_control(phy_type_antdiv_ctx_t *ctx, uint8 divOvrride, int core)
{
#ifdef WLC_SW_DIVERSITY
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8 antmap = divOvrride;

	if (!(IS_4364_3x3(pi) || IS_4364_1x1(pi))) {
		uint16 femctrl_val = READ_PHYREG(pi, femctrl_override_reg);
		/* convert to the designated core ant bit */
		antmap = (antmap & (1 << core)) ? 1 : 0;
		/* pin position 255 implies the NVRAM does not have antdiv_rfswctrlpin_aX entry */
		/* Diversity override can be 0 or 1 to select the antenna */
		/*  and 2 to restore original FEMCTRL table */
		if (core == 0 && (info->antdiv_rfswctrlpin_a0 != ANTDIV_RFSWCTRLPIN_UNDEFINED)) {
			info->ant_swOvr_state_core0 = antmap;
			if (antmap == 1) {
				femctrl_val = (femctrl_val |
					(1 << info->antdiv_rfswctrlpin_a0));
			} else {
				femctrl_val = (femctrl_val &
					(~(1 << info->antdiv_rfswctrlpin_a0)));
			}
		} else if (core == 1 &&
			(info->antdiv_rfswctrlpin_a1 != ANTDIV_RFSWCTRLPIN_UNDEFINED)) {
			info->ant_swOvr_state_core1 = antmap;
			if (antmap == 1) {
				femctrl_val = (femctrl_val | (1 <<
					(info->antdiv_rfswctrlpin_a1 - 10)));
			} else {
				femctrl_val = (femctrl_val & (~(1 <<
					(info->antdiv_rfswctrlpin_a1 - 10))));
			}
		} else {
			return;
		}
		WRITE_PHYREG(pi, femctrl_override_reg, femctrl_val);
#if defined(WLC_TXPWRCAP)
		/* Tx Pwr Cap update */
		if (PHY_TXPWRCAP_ENAB(pi->sh->physhim)) {
			phy_ac_txpwrcap_set(info->aci->txpwrcapi);
		}
#endif // endif
	}
#endif /* WLC_SW_DIVERSITY */
}

static int32
wlc_ant_div_sw_control_ovr(phy_type_antdiv_ctx_t *ctx, uint8 divOvrride, int core)
{
#ifdef WLC_SW_DIVERSITY
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8 antmap = divOvrride;

	/* If iovars ant_diversity_sw_core0 or ant_diversity_sw_core1 are
	 * used when Diversity is enabled, overwrite Diversity policies
	 */
	if (PHYSWDIV_ENAB(pi)) {
		/* convert to the designated core ant bit */
		antmap = (antmap & (1 << core)) ? 1 : 0;
		/* pin position 255 implies the NVRAM does not have antdiv_rfswctrlpin_aX entry
		 * Diversity override can be 0 or 1 to select the antenna
		 * and 2 to restore original FEMCTRL table
		 */
		if (core == 0 && (info->antdiv_rfswctrlpin_a0 != ANTDIV_RFSWCTRLPIN_UNDEFINED)) {
			info->ant_swOvr_state_core0 = antmap;
		} else if (core == 1 &&
			(info->antdiv_rfswctrlpin_a1 != ANTDIV_RFSWCTRLPIN_UNDEFINED)) {
				info->ant_swOvr_state_core1 = antmap;
		} else {
			return BCME_UNSUPPORTED;
		}
		return wlapi_swdiv_ant_plcy_override(pi->sh->physhim, core, divOvrride, 0, 0, 0);
	} else
#endif /* WLC_SW_DIVERSITY */
	{
		wlc_ant_div_sw_control(ctx, divOvrride, core);
		return BCME_OK;
	}
}

static void
phy_ac_antdiv_set_sw_control(phy_type_antdiv_ctx_t *ctx, int8 divOvrride, int core)
{
	if (!ACMAJORREV_3(((phy_ac_antdiv_info_t *)ctx)->pi->pubpi->phy_rev)) {
		if ((divOvrride > 2) || (divOvrride < 0)) {
			PHY_ERROR(("Value %d is not supported \n", (uint16)divOvrride));
		} else {
			wlc_ant_div_sw_control_ovr(ctx, divOvrride, core);
		}
	} else {
		PHY_ERROR(("IOVAR is not supported for this chip \n"));
	}
}

static void
phy_ac_antdiv_get_sw_control(phy_type_antdiv_ctx_t *ctx, int32 *ret_int_ptr, int core)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;

	if (!ACMAJORREV_3(info->pi->pubpi->phy_rev)) {
		switch (core) {
		case 0:
		{
			*ret_int_ptr = info->ant_swOvr_state_core0;
			break;
		}
		case 1:
		{
			*ret_int_ptr = info->ant_swOvr_state_core1;
			break;
		}
		default:
			PHY_ERROR(("Core %d is not supported \n", core));
			break;
		}
	} else {
		PHY_ERROR(("IOVAR is not supported for this chip \n"));
	}
}

/* Read the gci chip status register indexed by 'reg' */
static uint32
si_gci_chipstatus_acphy(si_t *sih, uint reg)
{
	/* because NFLASH and GCI clashes in 0xC00 */
	if ((CCREV(sih->ccrev) == 38) && ((sih->chipst & (1 << 4)) != 0)) {
		/* CC NFLASH exist, prohibit to manipulate gci register */
		ASSERT(0);
		return 0xFFFFFFFF;
	}

	si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_indirect_addr), ~0, reg);
	/* setting mask and value to '0' to use si_corereg for read only purpose */
	return si_corereg(sih, GCI_CORE_IDX(sih), GCI_OFFSETOF(sih, gci_chipsts), 0, 0);
}

bool
wlc_phy_check_antdiv_enable_acphy(phy_info_t *pi)
{
	if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi))) {
		return ((si_gci_chipstatus_acphy(pi->sh->sih, 8) >> 2) & 0x1);
	} else {
		return (ACMAJORREV_3(pi->pubpi->phy_rev));
	}
}

void
wlc_phy_antdiv_acphy(phy_info_t *pi, uint8 val)
{
	if (val > ANT_RX_DIV_FORCE_1) {
		MOD_PHYREG_2(pi, AntDivConfig2059, board_switch_div0, 1, /* enable diversity */
			CoreStartAntPos0, (val == ANT_RX_DIV_START_1) ? 1 : 0);

		ACPHY_REG_LIST_START
			/* 1p6us dwell time */
			MOD_PHYREG_ENTRY(pi, AntennaDivDwellTime, DivDwellTime, 64)

			MOD_PHYREG_ENTRY(pi, DivEnableClipGain, AntDivEnClipGains_Md, 0)
			MOD_PHYREG_ENTRY(pi, DivEnableClipGain, AntDivEnClipGains_Lo, 0)
			MOD_PHYREG_ENTRY(pi, DivEnableClipGain, AntDivEnClipGains_Clip2, 0)
			MOD_PHYREG_ENTRY(pi, DivEnableClipGain, AntDivEnClipGainBphy, 0)
			MOD_PHYREG_ENTRY(pi, DivGainThreshold_OFDM, Div_GainThresh_OFDM, 85)
			MOD_PHYREG_ENTRY(pi, DivGainThreshold_BPHY, Div_GainThresh_BPHY, 95)
			MOD_PHYREG_ENTRY(pi, AntennaDivBackOffGain, BackoffGain, 6)
			MOD_PHYREG_ENTRY(pi, AntennaDivMinGain, cckBackoffGain, 0)
			/* MOD_PHYREG_ENTRY(pi, defer_setClip1_CtrLen,
			 *	defer_setclip1gain_len,	20);
			 */
		ACPHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
				MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Hi, 1);
			} else if (CHSPEC_IS160(pi->radio_chanspec)) {
				ASSERT(0);
			} else {
				MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Hi, 0);
			}
		} else {
			MOD_PHYREG(pi, DivEnableClipGain, AntDivEnClipGains_Hi, 0);
		}
	} else {
		MOD_PHYREG_2(pi, AntDivConfig2059, board_switch_div0, 0, /* disable HW antsel */
			CoreStartAntPos0, (val == ANT_RX_DIV_FORCE_1) ? 1 : 0);
	}
}

#ifdef WLC_SW_DIVERSITY
static void
wlc_phy_set_txant_via_antsel_acphy(phy_info_t *pi, uint8 txant)
{
	if (txant == 1) {
		MOD_PHYREG_2(pi, AntSelConfig, AntCfg_Override, 1, AntCfg_OverrideEn, 1);
	} else if (txant == 0) {
		MOD_PHYREG_2(pi, AntSelConfig, AntCfg_Override, 0, AntCfg_OverrideEn, 1);
	} else {
		MOD_PHYREG_2(pi, AntSelConfig, AntCfg_Override, 0, AntCfg_OverrideEn, 0);
	}
}

#ifndef WLC_SW_DIVERSITY_DISABLED
static void
BCMATTACHFN(wlc_phy_swdiv_attach_acphy)(phy_info_t *pi)
{
	bool swdiv_en = (bool)PHY_GETINTVAR(pi, rstr_swdiv_en);
	if (swdiv_en) {
		pi->_swdiv = TRUE;
	}
	else {
		pi->_swdiv = FALSE;
	}
}
#endif // endif

static void
phy_acphy_swdiv_init(phy_type_antdiv_ctx_t *ctx)
{
	bool suspend;
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;

	if (PHYSWDIV_ENAB(pi)) {
		if (swdiv->swdiv_swctrl_en == SWDIV_SWCTRL_0)
			return;

		suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

		switch (swdiv->swdiv_swctrl_en) {
			case SWDIV_SWCTRL_2:
				MOD_PHYREG(pi, AntDivConfig2059, board_switch_div0, 0);
				break;

			case SWDIV_SWCTRL_0:
			case SWDIV_SWCTRL_1:
				/* 4357 family use txvlin overriding */
				if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
					ACPHY_REG_LIST_START
					/* override RF_SW_CTRL_3 */
					MOD_PHYREG_ENTRY(pi, RfctrlCoreGlobalPus,
						muxTxVlinOnFemCtrl, 1 << 3)
					/* set '0' on core0 as a default value */
					MOD_PHYREG_ENTRY(pi, RfctrlCoreAuxTssi10, tx_vlin, 0)
					MOD_PHYREG_ENTRY(pi, RfctrlOverrideAuxTssi0, tx_vlin_ovr, 1)
					/* to keep routing the core1 tx path to AUX2G */
					MOD_PHYREG_ENTRY(pi, RfctrlCoreAuxTssi11, tx_vlin, 1)
					MOD_PHYREG_ENTRY(pi, RfctrlOverrideAuxTssi1, tx_vlin_ovr, 1)
					ACPHY_REG_LIST_EXECUTE(pi);
				}
				break;
			default:
				ASSERT(0);
		}

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);

		wlc_phy_swdiv_antmap_init(ctx);
#if defined(WLC_TXPWRCAP)
		if (PHY_TXPWRCAP_ENAB(pi->sh->physhim)) {
			phy_ac_txpwrcap_set(info->aci->txpwrcapi);
		}
#endif /* WLC_TXPWRCAP */
	}
}

static uint16
phy_acphy_antmap_gpio_conv(phy_info_t *pi, phy_swdiv_t *swdiv, uint16 map)
{
	uint16 val = 0;
	int i;

	val = R_REG(pi->sh->osh, D11_PSM_GPIOOUT(pi));
	for (i = 0; i < PHYCORENUM(pi->pubpi->phy_corenum); i++) {
		if (swdiv->swdiv_gpio_ctrl & (1 << i)) {
			if ((map & (1 << i))) {
				val |= 1 << (i + swdiv->swdiv_gpio_num);
			} else {
				val &= ~(1 << (i + swdiv->swdiv_gpio_num));
			}
		}
	}
	return val;
}

/* Set the Antenna Suggested by the SW Diversity Algorithm */
static void
phy_acphy_swdiv_set_ant(phy_type_antdiv_ctx_t *ctx, uint8 new_ant)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;
	bool suspend;
	uint16 val;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	switch (swdiv->swdiv_swctrl_en) {
		case SWDIV_SWCTRL_0:
		{
			/* support multi-core in the GPIO case */
			W_REG(pi->sh->osh, D11_PSM_GPIOOUT(pi),
				phy_acphy_antmap_gpio_conv(pi, swdiv, (uint16)new_ant));
		}
			break;

		case SWDIV_SWCTRL_1:
		{
			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
#ifndef WLC_SWDIV_MULTI_CORE_ENABLE
			/* only support core0 */
			wlc_ant_div_sw_control(ctx, new_ant, 0);
#endif /* !WLC_SWDIV_MULTI_CORE_ENABLE */
			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);
		}
			break;

		case SWDIV_SWCTRL_2:
		{
			val = 1 << swdiv->swdiv_gpio_num;
			if (!suspend) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			}
#ifndef WLC_SWDIV_MULTI_CORE_ENABLE
			/* only support core0 */
			if ((new_ant & 0x1) == 1) {
				phy_utils_or_phyreg(pi, ACPHY_AntDivConfig2059(pi->pubpi->phy_rev),
						val);
			} else {
				phy_utils_and_phyreg(pi, ACPHY_AntDivConfig2059(pi->pubpi->phy_rev),
						~val);
			}
			wlc_phy_set_txant_via_antsel_acphy(pi, (new_ant & 0x1)? 1 : 0);
#endif /* !WLC_SWDIV_MULTI_CORE_ENABLE */
			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);
		}
			break;

		default:
			ASSERT(0);
	}
}

/* Get the Antenna currently in use for use by the SW Diversity Algorithm */
static uint8
phy_acphy_swdiv_get_ant(phy_type_antdiv_ctx_t *ctx)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;
	uint16 val = 0;
	uint16 mask;
	bool suspend;
	uint8 cur_ant = 0;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	switch (swdiv->swdiv_swctrl_en) {
		case SWDIV_SWCTRL_0:
			val = R_REG(pi->sh->osh, D11_PSM_GPIOOUT(pi));
			mask = swdiv->swdiv_gpio_ctrl << swdiv->swdiv_gpio_num;
			cur_ant = (val & mask) >> swdiv->swdiv_gpio_num;
			break;

		case SWDIV_SWCTRL_1:
			cur_ant = (uint8)wlc_phy_swdiv_get_rxantmap(ctx);
			break;

		case SWDIV_SWCTRL_2:
			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			val = READ_PHYREG(pi, AntDivConfig2059);
			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);
			mask = 1 << swdiv->swdiv_gpio_num;
			cur_ant = (val & mask) ? 1:0;
			break;

		default:
			ASSERT(0);
	}

	return cur_ant;
}

static uint16
wlc_phy_swdiv_get_rxantmap(phy_type_antdiv_ctx_t *ctx)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint16 swdiv_shm_addr;
	uint16 rx_pref_ant_addr;
	uint16 rxantmap;

	swdiv_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_ACPHY_SWDIV_BLK_PTR);
	rx_pref_ant_addr = swdiv_shm_addr + M_ACPHY_SWDIV_PREF_ANT;
	rxantmap = wlapi_bmac_read_shm(pi->sh->physhim, rx_pref_ant_addr);
	return (rxantmap & ACPHY_SWDIV_PREFANT_SHM_MASK);
}

void
wlc_phy_swdiv_antmap_init(phy_type_antdiv_ctx_t *ctx)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;
	wlapi_swdiv_antmap_init(pi->sh->physhim);
}

/* write femctrl control regs with corresponding swdiv switch control lines */
void
wlc_phy_set_femctrl_control_reg(phy_type_antdiv_ctx_t *ctx)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint16 femctrl_ovr;
	uint8 core0_sw = info->antdiv_rfswctrlpin_a0;
	uint8 core1_sw = info->antdiv_rfswctrlpin_a1 - 10;
	bool core0_support, core1_support;

	if (!ACMAJORREV_4(pi->pubpi->phy_rev))
		return;

	core0_support = phy_ac_swdiv_is_supported(ctx, 0, TRUE);
	core1_support = phy_ac_swdiv_is_supported(ctx, 1, TRUE);
	if (phy_get_phymode(pi) == PHYMODE_MIMO) {
		if (core0_support) {
			wlapi_exclusive_reg_access_core0(pi->sh->physhim, 1);
			femctrl_ovr = READ_PHYREG(pi, femctrl_override_control_reg);
			femctrl_ovr = (femctrl_ovr | (1 << core0_sw));
			WRITE_PHYREG(pi, femctrl_override_control_reg, femctrl_ovr);
			wlapi_exclusive_reg_access_core0(pi->sh->physhim, 0);
		}
		if (core1_support) {
			wlapi_exclusive_reg_access_core1(pi->sh->physhim, 1);
			femctrl_ovr = READ_PHYREG(pi, femctrl_override_control_reg);
			femctrl_ovr = (femctrl_ovr | (1 << core1_sw));
			WRITE_PHYREG(pi, femctrl_override_control_reg, femctrl_ovr);
			wlapi_exclusive_reg_access_core1(pi->sh->physhim, 0);
		}
	} else {
		femctrl_ovr = READ_PHYREG(pi, femctrl_override_control_reg);
		if ((phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0) && core0_support) {
			femctrl_ovr = (femctrl_ovr | (1 << core0_sw));
		} else if ((phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1) && core1_support) {
			femctrl_ovr = (femctrl_ovr | (1 << core1_sw));
		}
		WRITE_PHYREG(pi, femctrl_override_control_reg, femctrl_ovr);
	}
}

bool
phy_ac_swdiv_is_supported(phy_type_antdiv_ctx_t *ctx, uint8 core, bool inanyband)
{
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_swdiv_t *swdiv = info->swdiv;
	phy_info_t *pi = info->pi;
	bool swdiv_supported = 0;
	uint8 map2g, map5g, coremap;
	uint8 bitstamp = 0;
	int cnt;

	if ((core != ACPHY_PHYCORE_ANY) &&
		(core >= PHYCORENUM(pi->pubpi->phy_corenum))) {
		PHY_ERROR(("%s: core %d idx is out of the range\n", __FUNCTION__, core));
		return FALSE;
	}
	/* core indicates phycore id */
	if (core == ACPHY_PHYCORE_ANY) {
		for (cnt = 0; cnt < PHYCORENUM(pi->pubpi->phy_corenum); cnt++) {
			bitstamp |= 1 << cnt;
		}
	} else {
		bitstamp = (1 << core);
	}
	/* PHY_RSBD_PI_IDX_CORE0 for main slice / PHY_RSBD_PI_IDX_CORE1 for aux slice */
	if (phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0) {
		map2g = swdiv->swdiv_antmap2g_main;
		map5g = swdiv->swdiv_antmap5g_main;
	} else {
		map2g = swdiv->swdiv_antmap2g_aux;
		map5g = swdiv->swdiv_antmap5g_aux;
	}
	if (inanyband) {
		coremap = map2g | map5g;
	} else if (CHSPEC_IS2G(pi->radio_chanspec)) {
		coremap = map2g;
	} else {
		coremap = map5g;
	}
	if (core == ACPHY_PHYCORE_ANY) {
		swdiv_supported = ((coremap & bitstamp) ? TRUE : FALSE);
	} else {
		swdiv_supported = ((coremap & bitstamp) >> core);
	}
	return swdiv_supported;
}

uint8
phy_ac_swdiv_get_rxant_bycoreidx(phy_type_antdiv_ctx_t *ctx, uint core)
{
	uint16 antmap;
	/* convert the antmap to bits started from the offset */
	antmap = wlc_phy_swdiv_get_rxantmap(ctx);
	return (uint8)((antmap & (1 << core)) ? 1 : 0);
}

#ifdef WLC_TXPWRCAP
/* common txpwrcap info update func. if no antmap update
 * read it back from shmem location
 */
void
phy_ac_swdiv_txpwrcap_shmem_set(phy_type_antdiv_ctx_t *ctx,
	uint core, uint8 cap_tx, uint8 cap_rx, uint16 txantmap, uint16 rxantmap)
{
	uint16 swdiv_shm_addr, rx_pref_ant_addr, tx_pref_ant_addr;
	uint16 rxant, txant;
	phy_ac_antdiv_info_t *info = (phy_ac_antdiv_info_t *)ctx;
	phy_info_t *pi = info->pi;

	if (core >= PHYCORENUM(pi->pubpi->phy_corenum)) {
		PHY_ERROR(("%s: core %d idx is out of the range\n", __FUNCTION__, core));
		return;
	}
	swdiv_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_ACPHY_SWDIV_BLK_PTR);
	rx_pref_ant_addr = swdiv_shm_addr + M_ACPHY_SWDIV_PREF_ANT;
	tx_pref_ant_addr = swdiv_shm_addr + M_ACPHY_SWDIV_TX_PREF_ANT;
	if (rxantmap == ANTDIV_ANTMAP_NOCHANGE) {
		rxant = wlapi_bmac_read_shm(pi->sh->physhim, rx_pref_ant_addr);
	} else {
#ifdef WLC_SWDIV_MULTI_CORE_ENABLE
		rxant = rxantmap;
#else
		rxant = rxantmap & 0x1;
#endif /* WLC_SWDIV_MULTI_CORE_ENABLE */
	}
	if (txantmap == ANTDIV_ANTMAP_NOCHANGE) {
		txant = wlapi_bmac_read_shm(pi->sh->physhim, tx_pref_ant_addr);
	} else {
#ifdef WLC_SWDIV_MULTI_CORE_ENABLE
		txant = txantmap;
#else
		txant = txantmap & 0x1;
#endif /* WLC_SWDIV_MULTI_CORE_ENABLE */
	}
	/* when ucode is ready to support multi-core based txpwrcap
	 * this need to be extended.
	 */
	wlapi_bmac_write_shm(pi->sh->physhim, rx_pref_ant_addr,
		(cap_rx << ACPHY_TXPOWERCAP_SWDIV_SHM_OFFSET) |
			(rxant & ACPHY_SWDIV_PREFANT_SHM_MASK));
	wlapi_bmac_write_shm(pi->sh->physhim, tx_pref_ant_addr,
		(cap_tx << ACPHY_TXPOWERCAP_SWDIV_SHM_OFFSET) |
			(txant & ACPHY_SWDIV_PREFANT_SHM_MASK));
}
#endif /* WLC_TXPWRCAP */
#endif /* WLC_SW_DIVERSITY */

static void
wlc_phy_wltx_word_get(phy_info_t *pi, uint8 band, uint32 swctrlmap_wltx,
	uint32 swctrlmap_wltx_ext, uint32 *swctrlword,	uint32 *swctrlwordext)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	BCM_REFERENCE(band);

	/* If linear, use the lower 16 bits */
	if (pi_ac->antdivi->pa_mode == PAMODE_HI_LIN) {
		*swctrlword = swctrlmap_wltx & PAMODE_HI_LIN_MASK;
		*swctrlwordext = swctrlmap_wltx_ext & PAMODE_HI_LIN_MASK;
	} else {

		/* Otherwise use the upper 16 bits. */
		*swctrlword = (swctrlmap_wltx & PAMODE_HI_EFF_MASK) >> 16;
		*swctrlwordext = (swctrlmap_wltx_ext & PAMODE_HI_EFF_MASK) >> 16;
	}
}

static uint16 wlc_phy_femctrlout_get_val(uint32 val_ext, uint32 val, uint32 MASK)
{
	uint32 value = 0;
	value =  ((val_ext>>(MASK)) & 0x3)<<8 | ((val>>(MASK)) & 0xff);
	return (uint16) value;
}

#define ACPHY_MASK_TDM	                0x100
#define ACPHY_MASK_OVR_EN	        0x200
#define ACPHY_MASK_OVR_ANT              0x400
#define ACPHY_MAP_BT_TX	                0xc00
#define ACPHY_MASK_BT_TX	        0xc90
#define ACPHY_MAP_BT_RX	                0x800
#define ACPHY_MASK_BT_RX                0x810
#define ACPHY_MAP_WLAN_RX	        0x002
#define ACPHY_MASK_WLAN_RX	        0xbcf
#define ACPHY_MAP_WLAN_LOW_GAIN_RX	0x003
#define ACPHY_MASK_WLAN_LOW_GAIN_RX	0xbcf
#define ACPHY_MAP_WLAN_TX	        0x009
#define ACPHY_MASK_WLAN_TX	        0xbcf
#define ACPHY_MASK_ANT                  0x020
#define ACPHY_MASK_BAND                 0x010

void wlc_phy_write_regtbl_fc_from_nvram(phy_info_t *pi)
{
	uint8 band, ovr_en, ovr_ant, elna, ant, nswctrl;
	uint8 inv_btcx_prisel, muxErcxPriSel;
	/* uint8 tdm, ercx_prisel; */
	uint8 BT_priority, BT_TX, MUX_CTRL;
	uint8 WL_ANT_SEL, BT_AoA, WL_ePA_PU;
	uint8 WL_eLNA_Gain, BT_eLNA_Gain, WL_eLNA_PU;
	uint8 BT_RX, WL_TRSW, BT_rx_attn;
	uint16 index, indx, femctrlout;
	uint32 *swctrlmap, *swctrlmapext, decoded_address;
	uint8 core;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

#ifdef WLC_SW_DIVERSITY
	if (PHYSWDIV_ENAB(pi)) {
		wlc_phy_set_femctrl_control_reg(pi->u.pi_acphy->antdivi);
	}
#endif // endif

	/* BT_prisel and ErcxPriSel polarity info */
	/* No Need of forcing inv_btcx_prisel below */
	/* should be already taken care of in reg_on_init function */
	/*
	   MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);
	 */
	inv_btcx_prisel = READ_PHYREGFLD(pi, BT_SwControl, inv_btcx_prisel);
	muxErcxPriSel = READ_PHYREGFLD(pi, FemCtrl, muxErcxPriSel);

	nswctrl = (ACMAJORREV_4(pi->pubpi->phy_rev)) ? 10 : 8;

	for (band = 0; band <= 1; band++) {
		swctrlmap = ((band == 0) ? &(pi_ac->sromi->nvram_femctrl.swctrlmap_2g[0]) :
			&(pi_ac->sromi->nvram_femctrl.swctrlmap_5g[0]));
		swctrlmapext = ((band == 0) ? &(pi_ac->sromi->nvram_femctrl.swctrlmapext_2g[0]) :
			&(pi_ac->sromi->nvram_femctrl.swctrlmapext_5g[0]));

		if (band) {
			pi_ac->sromi->femctrlmask_2g =
			        ((pi_ac->sromi->nvram_femctrl.swctrlmapext_2g[4] & 0x3)<<8
			         | (pi_ac->sromi->nvram_femctrl.swctrlmap_2g[4] & 0xff));
		} else {
			pi_ac->sromi->femctrlmask_5g =
				((pi_ac->sromi->nvram_femctrl.swctrlmapext_5g[4] & 0x3)<<8
			         | (pi_ac->sromi->nvram_femctrl.swctrlmap_5g[4] & 0xff));
		}

		elna =  ((band == 0) ? BF_ELNA_2G(pi_ac) : BF_ELNA_5G(pi_ac));
		/* tdm = ((swctrlmap[4] & ACPHY_MASK_TDM) == ACPHY_MASK_TDM); */
		ovr_en = ((swctrlmap[4] & ACPHY_MASK_OVR_EN) == ACPHY_MASK_OVR_EN);
		ovr_ant = ((swctrlmap[4] & ACPHY_MASK_OVR_ANT) == ACPHY_MASK_OVR_ANT);

		FOREACH_CORE(pi, core) {
			/* Core 0 and 1 respectively have 8 and 6 inputs to the FEM ctrl LUT */
			for (index = 0; index < ((core == 0) ? 128 : 32); index++) {

				/* generate a femctrl index which includes band bit as well */
				indx = (index & 0xf) | (band<<4) | ((index>>4) <<5);

				/* BT_priority and BT_TX */
				BT_priority =  (indx & (1 << 7)) >> 7;
				BT_TX = (indx & (1 << 6)) >> 6;

				if (inv_btcx_prisel == 1) {
					MUX_CTRL  =  BT_priority & (band == 0);
				} else {
					MUX_CTRL  =  (BT_priority == 0) & (band == 0);
				}

				if ((MUX_CTRL == 0) || (core == 1)) {
					/* BT_AoA and ANT_SEL */
					WL_ANT_SEL = (indx & (1 << 5)) >> 5;
					BT_AoA  =   0;

					/* eLNA_gain */
					WL_eLNA_Gain = (indx & (1 << 2)) >> 2;
					BT_eLNA_Gain =   0;

					/* RX_PU */
					WL_eLNA_PU  = (indx & (1 << 1)) >> 1;
					BT_RX   =  0;

					/* TR switch related */
					WL_TRSW = (indx & (1 << 0)) >> 0;
					BT_rx_attn   =  0;
				} else {
					/* BT_AoA and ANT_SEL */
					WL_ANT_SEL =  0;
					BT_AoA     =  (indx & (1 << 5)) >> 5;

					/* eLNA_gain */
					WL_eLNA_Gain = 0;
					BT_eLNA_Gain = (indx & (1 << 2)) >> 2;

					/* RX_PU */
					WL_eLNA_PU = 0;
					BT_RX  = (indx & (1 << 1)) >> 1;

					/* TR switch related */
					WL_TRSW = 0;
					BT_rx_attn = (indx & (1 << 0)) >> 0;
				}

				/* ercx_prisel and WL_ePA_PU */
				if ((muxErcxPriSel == 0) || (core == 1)) {
					WL_ePA_PU = (indx & (1 << 3)) >> 3;
					/* ercx_prisel = 0; */
				} else {
					WL_ePA_PU = 0;
					/* ercx_prisel = (indx & (1 << 3)) >> 3; */
				}

				/* now classify which case this address corresponds to */
				decoded_address =
					(WL_TRSW << 0) +
					(WL_eLNA_PU << 1) +
					(WL_eLNA_Gain << 2) +
					(WL_ePA_PU << 3) +
					(band << 4) +
					(WL_ANT_SEL << 5) +
					(BT_rx_attn << 6) +
					(BT_RX << 7) +
					(BT_eLNA_Gain << 8) +
					(BT_AoA << 9) +
					(BT_TX << 10) +
					(BT_priority << 11);

				/* depending on the case decoded and if elna and ant overrdide */
				/* read appropriate byte from swctrlmap */
				ant =  (ovr_en == 1) ? ovr_ant : WL_ANT_SEL;

				if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
					ant = phy_get_rsdbbrd_corenum(pi, core);
				} else if (PHYCORENUM(pi->pubpi->phy_corenum) == 2) {
					/* No 2o3 antenna selection for 2x2 for now */
					ant = core;
				}

				if ((decoded_address & ACPHY_MASK_WLAN_TX) ==
						ACPHY_MAP_WLAN_TX) {
					uint32 swctrlwordext, swctrlword;
					wlc_phy_wltx_word_get(pi, band, swctrlmap[0],
						swctrlmapext[0], &swctrlword, &swctrlwordext);
					femctrlout = wlc_phy_femctrlout_get_val(
						swctrlwordext, swctrlword, 8*ant);
				} else if ((decoded_address & ACPHY_MASK_WLAN_RX) ==
						ACPHY_MAP_WLAN_RX) {
					femctrlout = wlc_phy_femctrlout_get_val(
						swctrlmapext[1], swctrlmap[1], 8*ant+16*elna);
				} else if ((decoded_address & ACPHY_MASK_WLAN_LOW_GAIN_RX) ==
						ACPHY_MAP_WLAN_LOW_GAIN_RX) {
					femctrlout = wlc_phy_femctrlout_get_val(
						swctrlmapext[2], swctrlmap[2], 8*ant+16*elna);
				} else if ((decoded_address & ACPHY_MASK_BT_TX) ==
						ACPHY_MAP_BT_TX) {
					femctrlout = wlc_phy_femctrlout_get_val(
						swctrlmapext[3], swctrlmap[3], 16);
				} else if ((decoded_address & ACPHY_MASK_BT_RX) ==
						ACPHY_MAP_BT_RX) {
					femctrlout = wlc_phy_femctrlout_get_val(
						swctrlmapext[3], swctrlmap[3],
						8*elna*!BT_eLNA_Gain);
				} else {
				/* Reset state */
				/* Init entire FEM table with a predefined value */
					uint32 swctrlword;
					swctrlword = ((band == 0) ?
						pi_ac->sromi->femctrl_init_val_2g :
						pi_ac->sromi->femctrl_init_val_5g);
					femctrlout = wlc_phy_femctrlout_get_val(
						0, swctrlword, 8*ant+16*elna);
				}

				if (core == 1)
					indx += 256;

				/* antdiv_rfswctrlpin_aX is the rfswctrl bit position to override */
				/* For stella this is USI ES2.0 it is 7 and 9 respectively */
				if (pi_ac->antdivi->ant_swOvr_state_core0 != 2 && core == 0) {
					if (pi_ac->antdivi->ant_swOvr_state_core0 == 1) {
						femctrlout = ((femctrlout &
						(~(1<<pi_ac->antdivi->antdiv_rfswctrlpin_a0))) |
						(1<<pi_ac->antdivi->antdiv_rfswctrlpin_a0));
					} else {
						femctrlout = (femctrlout &
						(~(1<<pi_ac->antdivi->antdiv_rfswctrlpin_a0)));
					}
				}

				/* antdiv_rfswctrlpin_a1  - nswctrl maps to the LUT bit position */
				if (pi_ac->antdivi->ant_swOvr_state_core1 != 2 && core == 1) {
				  uint8 swctrl = (pi_ac->antdivi->antdiv_rfswctrlpin_a1-nswctrl);
				  if (pi_ac->antdivi->ant_swOvr_state_core1 == 1) {
				    femctrlout =
				      ((femctrlout & (~(1<<swctrl)))
				       | (1<<swctrl));
				  } else {
				    femctrlout = (femctrlout & (~(1<<swctrl)));
				  }
				}
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, indx,
					16, &femctrlout);

			}
		}
	}
}

void
phy_ac_antdiv_chanspec(phy_ac_antdiv_info_t * antdivi)
{
	phy_ac_info_t *pi_ac = antdivi->aci;
	if (CHSPEC_IS2G(antdivi->pi->radio_chanspec)) {
		if (pi_ac->sromi->nvram_femctrl.txswctrlmap_2g) {
			antdivi->pa_mode = (pi_ac->sromi->nvram_femctrl.txswctrlmap_2g_mask >>
				(CHSPEC_CHANNEL(antdivi->pi->radio_chanspec) - 1)) & 1;
		} else {
			antdivi->pa_mode = 0;
		}
	} else {
		antdivi->pa_mode = pi_ac->sromi->nvram_femctrl.txswctrlmap_5g;
	}
}

/* Former name chanspec_regtbl_fc_from_nvram */
void
phy_ac_antdiv_regtbl_fc_from_nvram(phy_ac_antdiv_info_t *antdivi)
{
	phy_info_t *pi = antdivi->pi;
	phy_info_acphy_t *pi_ac = antdivi->aci;
	if (!CCT_INIT(pi_ac) && CHSPEC_IS2G(pi->radio_chanspec) &&
		pi_ac->sromi->nvram_femctrl.txswctrlmap_2g &&
		(antdivi->pa_mode ^ ((pi_ac->sromi->nvram_femctrl.txswctrlmap_2g_mask >>
		(CHSPEC_CHANNEL(pi->radio_chanspec) - 1)) & 1)) &&
		!ACPHY_FEMCTRL_ACTIVE(pi)) {
		antdivi->pa_mode = (pi_ac->sromi->nvram_femctrl.txswctrlmap_2g_mask >>
			(CHSPEC_CHANNEL(pi->radio_chanspec) - 1)) & 1;
		wlc_phy_write_regtbl_fc_from_nvram(pi);
	}
}

static int
phy_ac_antdiv_set_txswctrlmap(phy_type_antdiv_ctx_t *ctx, int32 int_val)
{
	phy_ac_antdiv_info_t *antdivi = (phy_ac_antdiv_info_t *)ctx;

	if (!((int_val >= AUTO) && (int_val <= PAMODE_HI_EFF))) {
		PHY_ERROR(("Value out of range\n"));
		return BCME_RANGE;
	}

	/* Setter mode, sets the value.
	 * Populate the right swctrlmap only if the pa_mode requested is different
	 * from the current setting
	 */
	if (antdivi->pa_mode != (int8) int_val) {
		/* Note the new state */
		antdivi->pa_mode = (int8) int_val;
		/* Call this function again to repopulate the switch control table. */
		wlc_phy_write_regtbl_fc_from_nvram(antdivi->pi);
	}
	return BCME_OK;
}

static int
phy_ac_antdiv_get_txswctrlmap(phy_type_antdiv_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_ac_antdiv_info_t *antdivi = (phy_ac_antdiv_info_t *)ctx;

	/* Getter mode, return the previously set value. */
	*ret_int_ptr = (int32) antdivi->pa_mode;
	return BCME_OK;
}
