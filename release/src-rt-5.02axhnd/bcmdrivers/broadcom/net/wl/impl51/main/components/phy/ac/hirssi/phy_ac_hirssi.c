/*
 * ACPHY HiRSSI eLNA Bypass module implementation
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
 * $Id: phy_ac_hirssi.c 655464 2016-08-19 23:20:17Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_hirssi.h>
#include <phy_ac.h>
#include <phy_ac_chanmgr.h>
#include <phy_ac_hirssi.h>
#include <phy_wd.h>
#include <phy_ac_info.h>
#include <d11.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phyreg_ac.h>
#include <wlc_phy_int.h>
#endif // endif

/* module private states */
struct phy_ac_hirssi_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_hirssi_info_t *hirssii;
	/* hirssi elna bypass */
	bool	hirssi_en;
	uint16	hirssi_period;
	uint16	hirssi_byp_cnt;
	uint16	hirssi_res_cnt;
	int8	hirssi_byp_rssi;
	int8	hirssi_res_rssi;
	bool	hirssi_elnabyp2g_en;
	bool	hirssi_elnabyp5g_en;
	int16	hirssi_timer2g;
	int16	hirssi_timer5g;
};

/* local functions */
static void phy_ac_hirssi_std_params_attach(phy_ac_hirssi_info_t *info);
static int phy_ac_hirssi_init(phy_type_hirssi_ctx_t *ctx);

static bool phy_ac_wd_hirssi_engine(phy_wd_ctx_t *ctx);
static void phy_ac_hirssi_engine(phy_ac_hirssi_info_t *info);
static void phy_ac_hirssi_apply(phy_ac_hirssi_info_t *info);
static bool phy_ac_hirssi_status(phy_ac_hirssi_info_t *info);

static int phy_ac_hirssi_get_period(phy_type_hirssi_ctx_t *ctx, int32 *period);
static int phy_ac_hirssi_set_period(phy_type_hirssi_ctx_t *ctx, int32 period);
static int phy_ac_hirssi_get_en(phy_type_hirssi_ctx_t *ctx, int32 *enable);
static int phy_ac_hirssi_set_en(phy_type_hirssi_ctx_t *ctx, int32 enable);
static int phy_ac_hirssi_get_rssi(phy_type_hirssi_ctx_t *ctx, int32 *rssi, phy_hirssi_t op);
static int phy_ac_hirssi_set_rssi(phy_type_hirssi_ctx_t *ctx, int32 rssi, phy_hirssi_t op);
static int phy_ac_hirssi_get_cnt(phy_type_hirssi_ctx_t *ctx, int32 *cnt, phy_hirssi_t op);
static int phy_ac_hirssi_set_cnt(phy_type_hirssi_ctx_t *ctx, int32 cnt, phy_hirssi_t op);
static int phy_ac_hirssi_get_status(phy_type_hirssi_ctx_t *ctx, int32 *status);

/* register phy type specific implementation */
phy_ac_hirssi_info_t *
BCMATTACHFN(phy_ac_hirssi_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_hirssi_info_t *hirssii)
{
	phy_ac_hirssi_info_t *info;
	phy_type_hirssi_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_ac_hirssi_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->hirssii = hirssii;

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_ac_wd_hirssi_engine, info,
		PHY_WD_PRD_1TICK, PHY_WD_1TICK_AC_HIRSSI_ELNABYPASS,
		PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	if (!ACMAJORREV_3(pi->pubpi->phy_rev)) {
		/* Only supported in ucode for mac revid 40 and 42 */
		/* ucode hirssi detect - bypass lna1 to save it */
		fns.init_hirssi = phy_ac_hirssi_init;
	}
	fns.getperiod = phy_ac_hirssi_get_period;
	fns.setperiod = phy_ac_hirssi_set_period;
	fns.geten = phy_ac_hirssi_get_en;
	fns.seten = phy_ac_hirssi_set_en;
	fns.getrssi = phy_ac_hirssi_get_rssi;
	fns.setrssi = phy_ac_hirssi_set_rssi;
	fns.getcnt = phy_ac_hirssi_get_cnt;
	fns.setcnt = phy_ac_hirssi_set_cnt;
	fns.getstatus = phy_ac_hirssi_get_status;
	fns.ctx = info;

	phy_ac_hirssi_std_params_attach(info);

	phy_hirssi_register_impl(hirssii, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_hirssi_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_hirssi_unregister_impl)(phy_ac_hirssi_info_t *info)
{
	phy_info_t *pi;
	phy_hirssi_info_t *hirssii;

	ASSERT(info);
	pi = info->pi;
	hirssii = info->hirssii;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_hirssi_unregister_impl(hirssii);

	phy_mfree(pi, info, sizeof(phy_ac_hirssi_info_t));
}

static void
BCMATTACHFN(phy_ac_hirssi_std_params_attach)(phy_ac_hirssi_info_t *pi_ac)
{
	/* ucode hirssi detect - bypass lna1 to save it */
	pi_ac->hirssi_en = 0; /* ACMAJORREV_0(pi->pubpi->phy_rev); */
	pi_ac->hirssi_period = PHY_SW_HIRSSI_PERIOD;
	pi_ac->hirssi_byp_rssi = PHY_SW_HIRSSI_BYP_THR;
	pi_ac->hirssi_res_rssi = PHY_SW_HIRSSI_RES_THR;
	pi_ac->hirssi_byp_cnt = PHY_SW_HIRSSI_W1_BYP_CNT;
	pi_ac->hirssi_res_cnt = PHY_SW_HIRSSI_W1_RES_CNT;

	/* J28 have attenuators, so don't use hirssi feature there */
	if (pi_ac->pi->sh->boardvendor == VENDOR_APPLE &&
		((pi_ac->pi->sh->boardtype == BCM94360J28_D11AC2G) ||
		(pi_ac->pi->sh->boardtype == BCM94360J28_D11AC5G))) {
		pi_ac->hirssi_en = FALSE;
	}

	/* Only supported in ucode for mac revid 40 and 42 */
	phy_ac_hirssi_init(pi_ac);
}

/* ******************  HIRSSI ELNABYPASS (uCode supported). Begin ****************** */
static bool
phy_ac_wd_hirssi_engine(phy_wd_ctx_t *ctx)
{
	phy_ac_hirssi_info_t *pi_ac = (phy_ac_hirssi_info_t *)ctx;
	phy_info_t *pi = pi_ac->pi;
	BCM_REFERENCE(pi);
	/* Bypass elna if hirssi is detected to protect lna1.
	 * Applicable only for 4360 A0/B0 designs
	 */
	if (PHY_SW_HIRSSI_UCODE_CAP(pi))
		phy_ac_hirssi_engine(pi_ac);
	return TRUE;
}

static void
phy_ac_hirssi_engine(phy_ac_hirssi_info_t *pi_ac)
{
	int16 timer;
	phy_info_t *pi = pi_ac->pi;
	bool ucode_hirssi, upd = FALSE;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		timer = pi_ac->hirssi_timer2g;
		pi_ac->hirssi_timer5g = MAX(pi_ac->hirssi_timer5g - 1, PHY_SW_HIRSSI_OFF);
		if (!pi_ac->hirssi_elnabyp2g_en) return;
	} else {
		timer = pi_ac->hirssi_timer5g;
		pi_ac->hirssi_timer2g = MAX(pi_ac->hirssi_timer2g - 1, PHY_SW_HIRSSI_OFF);
		if (!pi_ac->hirssi_elnabyp5g_en) return;
	}

	/* Logic */
	if (timer > PHY_SW_HIRSSI_OFF) {
		timer--;
		if (timer == PHY_SW_HIRSSI_OFF) {
			ucode_hirssi = phy_ac_hirssi_shmem_read_clear(pi);
			if (ucode_hirssi) {
				PHY_ERROR(("wl%d: %s state:Already ON\n", pi->sh->unit,
				           __FUNCTION__));
				timer = pi_ac->hirssi_period;
			} else  {
				PHY_ERROR(("wl%d: %s state:OFF\n", pi->sh->unit, __FUNCTION__));
				timer = PHY_SW_HIRSSI_OFF;
				upd = TRUE;

			}
		}
	} else {
		ucode_hirssi = phy_ac_hirssi_shmem_read_clear(pi);
		if (ucode_hirssi) {
			PHY_ERROR(("wl%d: %s state:ON\n", pi->sh->unit, __FUNCTION__));
			timer = pi_ac->hirssi_period;
			upd = TRUE;
		}
	}

	/* Update Timer */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		pi_ac->hirssi_timer2g = timer;
	} else {
		pi_ac->hirssi_timer5g = timer;
	}

	/* Update uCode & apply gainctrl changes */
	if (upd) {
		phy_ac_hirssi_set_ucode_params(pi);
		phy_ac_hirssi_apply(pi_ac);
	}
}

static void
phy_ac_hirssi_apply(phy_ac_hirssi_info_t *pi_ac)
{
	phy_info_t *pi = pi_ac->pi;
	if (!(pi->sh->clk))
		return;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifndef WLC_DISABLE_ACI
	if (!ACPHY_ENABLE_FCBS_HWACI(pi)) {
		/* ACI - reset aci for current band & restore defaults */
		wlc_phy_desense_aci_reset_params_acphy(pi, FALSE, CHSPEC_IS2G(pi->radio_chanspec),
		                                       CHSPEC_IS5G(pi->radio_chanspec));
		wlc_phy_desense_calc_total_acphy(pi->u.pi_acphy->rxgcrsi);
		wlc_phy_desense_apply_acphy(pi, FALSE);
	}
#endif /* !WLC_DISABLE_ACI */

	/* Set new gainctrl with current aci_off/elna_bypass settings */
	wlc_phy_rxgainctrl_set_gaintbls_acphy(pi, TRUE, TRUE, TRUE);
	wlc_phy_rxgainctrl_gainctrl_acphy(pi);

	wlc_phy_resetcca_acphy(pi);
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RESET2RX);

	wlapi_enable_mac(pi->sh->physhim);
}

static bool
phy_ac_hirssi_status(phy_ac_hirssi_info_t *pi_ac)
{
	bool status;
	phy_info_t *pi = pi_ac->pi;
	if (CHSPEC_IS2G(pi->radio_chanspec))
		status = pi_ac->hirssi_timer2g > PHY_SW_HIRSSI_OFF;
	else
		status = pi_ac->hirssi_timer5g > PHY_SW_HIRSSI_OFF;
	if (!pi->sh->clk)
		return status;
	else
		return (status ||
			(wlapi_bmac_read_shm(pi->sh->physhim, M_SLP_RDY_INT(pi)) == 0xdead));
}

static int
phy_ac_hirssi_init(phy_type_hirssi_ctx_t *ctx)
{
	phy_ac_hirssi_info_t  *pi_ac = (phy_ac_hirssi_info_t *)ctx;
	pi_ac->hirssi_timer2g = PHY_SW_HIRSSI_OFF;
	pi_ac->hirssi_timer5g = PHY_SW_HIRSSI_OFF;

	if (PHY_SW_HIRSSI_UCODE_CAP(pi_ac->pi)) {
		pi_ac->hirssi_elnabyp2g_en = pi_ac->hirssi_en;
		pi_ac->hirssi_elnabyp5g_en = pi_ac->hirssi_en;
		if (pi_ac->pi->sh->clk) {
			phy_ac_hirssi_set_ucode_params(pi_ac->pi);
			wlapi_bmac_write_shm(pi_ac->pi->sh->physhim, M_SLP_RDY_INT(pi_ac->pi), 0);
		}
	} else {
		pi_ac->hirssi_elnabyp2g_en = FALSE;
		pi_ac->hirssi_elnabyp5g_en = FALSE;
	}
	return BCME_OK;
}

bool
phy_ac_hirssi_shmem_read_clear(phy_info_t *pi)
{
	bool hirssi = FALSE;
	if (PHY_SW_HIRSSI_UCODE_CAP(pi)) {
		hirssi = (wlapi_bmac_read_shm(pi->sh->physhim, M_SLP_RDY_INT(pi)) == 0xdead);
		if (hirssi)
			wlapi_bmac_write_shm(pi->sh->physhim, M_SLP_RDY_INT(pi), 0);
	}
	return hirssi;
}

void
phy_ac_hirssi_set_ucode_params(phy_info_t *pi)
{
	int16 hirssi_rssi = 50;
	uint16 hirssi_w1_reg =  PHY_SW_HIRSSI_W1_BYP_REG;
	uint16 hirssi_w1_cnt = 500;
	bool res, en;
	uint8 factor;
	phy_ac_hirssi_info_t  *pi_ac = pi->u.pi_acphy->hirssii;

	if (!PHY_SW_HIRSSI_UCODE_CAP(pi)) return;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		en = pi_ac->hirssi_elnabyp2g_en;
		res = pi_ac->hirssi_timer2g > PHY_SW_HIRSSI_OFF;
	} else {
		en = pi_ac->hirssi_elnabyp5g_en;
		res = pi_ac->hirssi_timer5g > PHY_SW_HIRSSI_OFF;
	}

	if (en) {
		hirssi_rssi = (res) ? pi_ac->hirssi_res_rssi : pi_ac->hirssi_byp_rssi;
		hirssi_w1_reg = (res) ? PHY_SW_HIRSSI_W1_RES_REG : PHY_SW_HIRSSI_W1_BYP_REG;
		hirssi_w1_cnt = (res) ? pi_ac->hirssi_res_cnt : pi_ac->hirssi_byp_cnt;

		factor = CHSPEC_BW_LE20(pi->radio_chanspec) ? 1 :
		        (CHSPEC_IS40(pi->radio_chanspec) ? 2 : 4);
		hirssi_w1_cnt *= factor;
	}

	wlapi_bmac_write_shm(pi->sh->physhim, M_HOST_FLAGS6(pi), hirssi_rssi);
	wlapi_bmac_write_shm(pi->sh->physhim, M_USEQ_PWRUP_PTR(pi), hirssi_w1_reg);
	wlapi_bmac_write_shm(pi->sh->physhim, M_USEQ_PWRDN_PTR(pi), hirssi_w1_cnt);
}

void phy_ac_hirssi_set_timer(phy_info_t *pi)
{
	phy_ac_hirssi_info_t  *pi_ac = pi->u.pi_acphy->hirssii;
	phy_info_acphy_t *acphy = pi->u.pi_acphy;
	if (phy_ac_chanmgr_get_data(acphy->chanmgri)->curr_band2g) {
		if (pi_ac->hirssi_elnabyp2g_en)
			pi_ac->hirssi_timer2g = pi_ac->hirssi_period;
	} else {
		if (pi_ac->hirssi_elnabyp5g_en)
			pi_ac->hirssi_timer5g = pi_ac->hirssi_period;
	}
}

bool phy_ac_hirssi_set(phy_info_t *pi)
{
	phy_ac_hirssi_info_t  *pi_ac = pi->u.pi_acphy->hirssii;
	bool elna_bypass = FALSE;
	if ((CHSPEC_IS2G(pi->radio_chanspec) && (pi_ac->hirssi_timer2g > PHY_SW_HIRSSI_OFF)) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && (pi_ac->hirssi_timer5g > PHY_SW_HIRSSI_OFF))) {
		elna_bypass = TRUE;
	}
	return elna_bypass;
}

bool phy_ac_hirssi_get(phy_info_t *pi)
{
	phy_ac_hirssi_info_t  *pi_ac = pi->u.pi_acphy->hirssii;
	return (CHSPEC_IS2G(pi->radio_chanspec) & pi_ac->hirssi_elnabyp2g_en) ||
		(CHSPEC_IS5G(pi->radio_chanspec) & pi_ac->hirssi_elnabyp5g_en);
}
/* ******************  HIRSSI ELNABYPASS (uCode supported). End  ****************** */

static int
phy_ac_hirssi_get_period(phy_type_hirssi_ctx_t *ctx, int32 *period)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	*period = (int32)aci->hirssi_period;
	return BCME_OK;
}

static int
phy_ac_hirssi_set_period(phy_type_hirssi_ctx_t *ctx, int32 period)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	aci->hirssi_period = (period <= 0) ? PHY_SW_HIRSSI_PERIOD  : (uint16)period;
	return BCME_OK;
}

static int
phy_ac_hirssi_get_en(phy_type_hirssi_ctx_t *ctx, int32 *enable)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	*enable = (int32)aci->hirssi_en;
	return BCME_OK;
}

static int
phy_ac_hirssi_set_en(phy_type_hirssi_ctx_t *ctx, int32 enable)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	aci->hirssi_en = (enable == 0) ? FALSE : TRUE;
	if (PHY_SW_HIRSSI_UCODE_CAP(aci->pi)) {
		phy_ac_hirssi_init(aci);
		if (!aci->hirssi_en) {
			phy_ac_hirssi_apply(aci);
		}
	}
	return BCME_OK;
}

static int
phy_ac_hirssi_get_rssi(phy_type_hirssi_ctx_t *ctx, int32 *rssi, phy_hirssi_t opt)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	switch (opt) {
		case PHY_HIRSSI_BYP:
			*rssi = (int32)aci->hirssi_byp_rssi;
			break;
		case PHY_HIRSSI_RES:
			*rssi = (int32)aci->hirssi_res_rssi;
			break;
		default:
			return BCME_BADARG;
	}
	return BCME_OK;
}

static int
phy_ac_hirssi_set_rssi(phy_type_hirssi_ctx_t *ctx, int32 rssi, phy_hirssi_t opt)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	switch (opt) {
		case PHY_HIRSSI_BYP:
			aci->hirssi_byp_rssi = (int8)rssi;
			break;
		case PHY_HIRSSI_RES:
			aci->hirssi_res_rssi = (int8)rssi;
			break;
		default:
			return BCME_BADARG;
	}
	if (PHY_SW_HIRSSI_UCODE_CAP(aci->pi)) {
		phy_ac_hirssi_init(aci);
	}
	return BCME_OK;
}

static int
phy_ac_hirssi_get_cnt(phy_type_hirssi_ctx_t *ctx, int32 *cnt, phy_hirssi_t opt)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	switch (opt) {
		case PHY_HIRSSI_BYP:
			*cnt = (int32)aci->hirssi_byp_cnt;
			break;
		case PHY_HIRSSI_RES:
			*cnt = (int32)aci->hirssi_res_cnt;
			break;
		default:
			return BCME_BADARG;
	}
	return BCME_OK;
}

static int
phy_ac_hirssi_set_cnt(phy_type_hirssi_ctx_t *ctx, int32 cnt, phy_hirssi_t opt)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	switch (opt) {
		case PHY_HIRSSI_BYP:
			aci->hirssi_byp_cnt = (cnt == -1) ?
				PHY_SW_HIRSSI_W1_BYP_CNT : (uint16)cnt;
			break;
		case PHY_HIRSSI_RES:
			aci->hirssi_res_cnt = (cnt == -1) ?
				PHY_SW_HIRSSI_W1_RES_CNT : (uint16)cnt;
			break;
		default:
			return BCME_BADARG;
	}
	if (PHY_SW_HIRSSI_UCODE_CAP(aci->pi)) {
		phy_ac_hirssi_init(aci);
	}
	return BCME_OK;
}

static int
phy_ac_hirssi_get_status(phy_type_hirssi_ctx_t *ctx, int32 *status)
{
	phy_ac_hirssi_info_t *aci = (phy_ac_hirssi_info_t *) ctx;
	phy_info_t *pi = aci->pi;
	BCM_REFERENCE(pi);
	*status = 0;
	if (PHY_SW_HIRSSI_UCODE_CAP(pi)) {
		*status = (int32) phy_ac_hirssi_status(aci);
	}
	return BCME_OK;
}
