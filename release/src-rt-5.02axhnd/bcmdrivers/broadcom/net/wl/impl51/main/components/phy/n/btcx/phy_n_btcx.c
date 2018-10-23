/*
 * NPHY BT Coex module implementation
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
 * $Id: phy_n_btcx.c 679944 2017-01-18 01:22:39Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_btcx.h"
#include <phy_n.h>
#include <phy_n_btcx.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phyreg_n.h>
#include <wlc_phy_int.h>

#include <phy_utils_reg.h>
#include <phy_n_info.h>

/* module private states */
struct phy_n_btcx_info {
	phy_info_t			*pi;
	phy_n_info_t		*ni;
	phy_btcx_info_t		*cmn_info;

/* add other variable size variables here at the end */
};

/* local functions */
static void wlc_nphy_btc_adjust(phy_type_btcx_ctx_t *ctx, bool btactive);
static void phy_n_btcx_override_enable(phy_type_btcx_ctx_t *ctx);
static void phy_n_btcx_override_disable(phy_type_btcx_ctx_t *ctx);
static int phy_n_btcx_set_restage_rxgain(phy_type_btcx_ctx_t *ctx, int32 set_val);

/* register phy type specific implementation */
phy_n_btcx_info_t *
BCMATTACHFN(phy_n_btcx_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_btcx_info_t *cmn_info)
{
	phy_n_btcx_info_t *n_info;
	phy_type_btcx_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((n_info = phy_malloc(pi, sizeof(phy_n_btcx_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	n_info->pi = pi;
	n_info->ni = ni;
	n_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.adjust = wlc_nphy_btc_adjust;
	fns.override_enable = phy_n_btcx_override_enable;
	fns.override_disable = phy_n_btcx_override_disable;
	fns.set_restage_rxgain = phy_n_btcx_set_restage_rxgain;
	fns.ctx = n_info;

	if (phy_btcx_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_btcx_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* PHY-Feature specific parameter initialization */

	return n_info;

	/* error handling */
fail:
	if (n_info != NULL)
		phy_mfree(pi, n_info, sizeof(phy_n_btcx_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_btcx_unregister_impl)(phy_n_btcx_info_t *n_info)
{
	phy_info_t *pi;
	phy_btcx_info_t *cmn_info;

	ASSERT(n_info);
	pi = n_info->pi;
	cmn_info = n_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_btcx_unregister_impl(cmn_info);

	phy_mfree(pi, n_info, sizeof(phy_n_btcx_info_t));
}

static void
wlc_nphy_btc_adjust(phy_type_btcx_ctx_t *ctx, bool btactive)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(btactive);
}

static void
phy_n_btcx_override_enable(phy_type_btcx_ctx_t *ctx)
{
	phy_n_btcx_info_t *info = (phy_n_btcx_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* This is required only for 2G operation. No BTCX in 5G */
	if ((pi->sh->machwcap & MCAP_BTCX_SUP(pi->sh->corerev)) &&
		CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));

		wlapi_coex_flush_a2dp_buffers(pi->sh->physhim);

		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, D11_BTCX_CTL(pi), BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force WLAN antenna and priority */
		OR_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
			BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL);
	}
}

static void
phy_n_btcx_override_disable(phy_type_btcx_ctx_t *ctx)
{
	phy_n_btcx_info_t *info = (phy_n_btcx_info_t *)ctx;
	phy_info_t *pi = info->pi;

	if ((pi->sh->machwcap & MCAP_BTCX_SUP(pi->sh->corerev)) &&
		CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));

		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, D11_BTCX_CTL(pi), BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force BT priority */
		AND_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
			~(BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL));
	}
}

static int
phy_n_btcx_set_restage_rxgain(phy_type_btcx_ctx_t *ctx, int32 set_val)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(set_val);

	return BCME_UNSUPPORTED;
}
