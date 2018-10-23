/*
 * ACPHY STF module implementation
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
 * $Id: phy_ac_stf.c 742511 2018-01-22 14:14:24Z $
 */
#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_stf.h>
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <phy_ac_chanmgr.h>
#include <phy_ac_stf.h>
#include <wlc_radioreg_20693.h>
#include <phy_rstr.h>
#include <phy_utils_var.h>
#include <wlioctl.h>
#include <phy_stf.h>

/* module private states */
struct phy_ac_stf_info {
	phy_info_t *pi;
	phy_ac_info_t *pi_ac;
	phy_stf_info_t *stf_info;
};

/* locally used functions */

/* Functions used by common layer as callbacks */
static int phy_ac_stf_set_stf_chain(phy_type_stf_ctx_t *ctx,
		uint8 txchain, uint8 rxchain);
static int phy_ac_stf_chain_init(phy_type_stf_ctx_t *ctx, bool txrxchain_mask,
		uint8 txchain, uint8 rxchain);

/* register phy type specific implementation */
phy_ac_stf_info_t*
BCMATTACHFN(phy_ac_stf_register_impl)(phy_info_t *pi,
	phy_ac_info_t *pi_ac, phy_stf_info_t *stf_info)
{
	phy_ac_stf_info_t *ac_stf_info;
	phy_type_stf_fns_t fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_stf_info = phy_malloc(pi, sizeof(phy_ac_stf_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	ac_stf_info->pi = pi;
	ac_stf_info->pi_ac = pi_ac;
	ac_stf_info->stf_info = stf_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.set_stf_chain = phy_ac_stf_set_stf_chain;
	fns.chain_init = phy_ac_stf_chain_init;
	fns.ctx = ac_stf_info;

	if (phy_stf_register_impl(stf_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_stf_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_stf_info;
	/* error handling */
fail:
	if (ac_stf_info != NULL)
		phy_mfree(pi, ac_stf_info, sizeof(phy_ac_stf_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_stf_unregister_impl)(phy_ac_stf_info_t *ac_stf_info)
{
	phy_info_t *pi = ac_stf_info->pi;
	phy_stf_info_t *stf_info = ac_stf_info->stf_info;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_stf_unregister_impl(stf_info);
	phy_mfree(pi, ac_stf_info, sizeof(phy_ac_stf_info_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/* ****       Functions used by other AC modules     **** */

int phy_ac_stf_force_fail_txchain(phy_ac_stf_info_t *stfi)
{
	phy_stf_info_t *cmn_stf_info = NULL;

	ASSERT(stfi);

	cmn_stf_info = (phy_stf_info_t *) stfi->stf_info;

	phy_stf_set_phytxchain(cmn_stf_info, 0x3);
	phy_stf_set_phyrxchain(cmn_stf_info, 0x1);

	return BCME_OK;
}

/* ********************************************* */
/*	Callback Functions                                                   */
/* ********************************************* */
static int
phy_ac_stf_set_stf_chain(phy_type_stf_ctx_t *ctx, uint8 txchain, uint8 rxchain)
{
	phy_ac_stf_info_t *ac_stf_info = NULL;
	phy_stf_info_t *cmn_stf_info = NULL;
	phy_info_t *pi = NULL;

	ASSERT(ctx);
	ac_stf_info = (phy_ac_stf_info_t *) ctx;

	ASSERT(ac_stf_info->stf_info);
	cmn_stf_info = (phy_stf_info_t *) ac_stf_info->stf_info;
	pi = ac_stf_info->pi;

	PHY_TRACE(("phy_ac_stf_set_stf_chain, new phy chain tx %d, rx %d", txchain, rxchain));

	phy_stf_set_phytxchain(cmn_stf_info, txchain);
	if (!(ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
		phy_ac_chanmgr_set_both_txchain_rxchain(pi->u.pi_acphy->chanmgri, rxchain, txchain);
	}

#ifdef STA
	if (pi->sh->up) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
#endif // endif
	wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi, rxchain,
		phy_stf_get_data(cmn_stf_info)->phytxchain);

#ifdef STA
	if (pi->sh->up) {
		wlapi_mimops_pmbcnrx(pi->sh->physhim);
		wlapi_enable_mac(pi->sh->physhim);
	}
#endif // endif
	return BCME_OK;
}

static int
phy_ac_stf_chain_init(phy_type_stf_ctx_t *ctx, bool txrxchain_mask,
		uint8 txchain, uint8 rxchain)
{
	phy_ac_stf_info_t *ac_stf_info = NULL;
	phy_stf_info_t *cmn_stf_info = NULL;
	phy_info_t *pi = NULL;

	ASSERT(ctx);
	ac_stf_info = (phy_ac_stf_info_t *) ctx;

	ASSERT(ac_stf_info->stf_info);
	cmn_stf_info = (phy_stf_info_t *) ac_stf_info->stf_info;
	pi = ac_stf_info->pi;

	if ((txrxchain_mask) &&
		(ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
		phy_stf_set_phytxchain(cmn_stf_info, txchain & pi->sromi->sw_txchain_mask);
		phy_stf_set_phyrxchain(cmn_stf_info, rxchain & pi->sromi->sw_rxchain_mask);
	} else {
		phy_stf_set_phytxchain(cmn_stf_info, txchain);
		phy_stf_set_phyrxchain(cmn_stf_info, rxchain);
	}

	return BCME_OK;
}
