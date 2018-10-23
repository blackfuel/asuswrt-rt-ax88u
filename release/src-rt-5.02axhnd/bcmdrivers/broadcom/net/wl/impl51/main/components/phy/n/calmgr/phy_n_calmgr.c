/*
 * NPHY Calibration Manager module implementation
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
 * $Id: phy_n_calmgr.c 659967 2016-09-16 19:35:14Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_calmgr.h"
#include <phy_n.h>
#include <phy_n_calmgr.h>
#include <phy_n_info.h>
#include <phy_calmgr.h>

/* module private states */
struct phy_n_calmgr_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_calmgr_info_t *ci;
};

/* local functions */
static int phy_n_calmgr_init(phy_type_calmgr_ctx_t *ctx);
static int phy_n_calmgr_enable_initcal(phy_type_calmgr_ctx_t *ctx, bool initcal);
static int phy_n_calmgr_prepare(phy_type_calmgr_ctx_t *ctx);
static void phy_n_calmgr_cleanup(phy_type_calmgr_ctx_t *ctx);
static void phy_n_calmgr_cals(phy_type_calmgr_ctx_t *ctx, uint8 legacy_caltype, uint8 searchmode);
static void phy_n_calmgr_add_timer_special(phy_type_calmgr_ctx_t *ctx, uint delay_val);
static void phy_n_calmgr_set_override(phy_type_calmgr_ctx_t *ctx, uint8 cal_type_override);

/* register phy type specific implementation */
phy_n_calmgr_info_t *
BCMATTACHFN(phy_n_calmgr_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_calmgr_info_t *ci)
{
	phy_n_calmgr_info_t *info;
	phy_type_calmgr_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_n_calmgr_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_calmgr_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ci = ci;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_n_calmgr_init;
	fns.enable_initcal = phy_n_calmgr_enable_initcal;
	fns.prepare = phy_n_calmgr_prepare;
	fns.cleanup = phy_n_calmgr_cleanup;
	fns.cals = phy_n_calmgr_cals;
	fns.add_timer_special = phy_n_calmgr_add_timer_special;
	fns.set_override = phy_n_calmgr_set_override;
	fns.ctx = info;

	if (phy_calmgr_register_impl(ci, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_calmgr_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_calmgr_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_calmgr_unregister_impl)(phy_n_calmgr_info_t *info)
{
	phy_info_t *pi;

	ASSERT(info);
	pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_calmgr_unregister_impl(info->ci);

	phy_mfree(pi, info, sizeof(phy_n_calmgr_info_t));
}

static int
phy_n_calmgr_init(phy_type_calmgr_ctx_t *ctx)
{
	phy_n_calmgr_info_t * calmgri = (phy_n_calmgr_info_t *) ctx;
	phy_info_t *pi = calmgri->pi;
	if (NREV_LE(pi->pubpi->phy_rev, 15)) {
		pi->interf->aci.ma_total = PHY_NOISE_MA_WINDOW_SZ * ACI_INIT_MA;
		pi->interf->badplcp_ma_total = PHY_NOISE_GLITCH_INIT_MA_BADPlCP *
			PHY_NOISE_MA_WINDOW_SZ;
	} else {
		pi->interf->aci.ma_total = MA_WINDOW_SZ * ACI_INIT_MA;
		pi->interf->badplcp_ma_total = PHY_NOISE_GLITCH_INIT_MA_BADPlCP *
			MA_WINDOW_SZ;
	}
#ifndef WLC_DISABLE_ACI
	wlc_phy_aci_init_nphy(pi);
	wlc_phy_aci_sw_reset_nphy(pi);
#endif // endif
	return BCME_OK;
}

static int
phy_n_calmgr_enable_initcal(phy_type_calmgr_ctx_t *ctx, bool initcal)
{
	phy_n_calmgr_info_t * calmgri = (phy_n_calmgr_info_t *) ctx;
	calmgri->ni->do_initcal = initcal;
	return BCME_OK;
}

static int
phy_n_calmgr_prepare(phy_type_calmgr_ctx_t *ctx)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	return BCME_OK;
}

static void
phy_n_calmgr_cleanup(phy_type_calmgr_ctx_t *ctx)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

static void
phy_n_calmgr_cals(phy_type_calmgr_ctx_t *ctx, uint8 legacy_caltype, uint8 searchmode)
{
	phy_n_calmgr_info_t *info = (phy_n_calmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	wlc_phy_cal_perical_nphy_run(pi, legacy_caltype);
}

static void
phy_n_calmgr_add_timer_special(phy_type_calmgr_ctx_t *ctx, uint delay_val)
{
	phy_n_calmgr_info_t *info = (phy_n_calmgr_info_t *)ctx;
	phy_n_info_t *pi_nphy = info->ni;
	phy_info_t *pi = info->pi;

	if (!(pi_nphy->ntd_papdcal_dcs == TRUE &&
		pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_RXCAL))
		wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, delay_val, 0);
	else {
		pi_nphy->ntd_papdcal_dcs = FALSE;
		phy_calmgr_mphase_reset(pi->calmgri);
	}
}

static void
phy_n_calmgr_set_override(phy_type_calmgr_ctx_t *ctx, uint8 cal_type_override)
{
	phy_n_calmgr_info_t *info = (phy_n_calmgr_info_t *)ctx;
	info->ni->cal_type_override = cal_type_override;
}
