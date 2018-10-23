/*
 * NPHY STF modules implementation
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
 * $Id: phy_n_stf.c 656697 2016-08-29 19:19:37Z $
 */
#include <typedefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_btcx.h>
#include <phy_stf.h>
#include <phy_type_stf.h>
#include <phy_n_stf.h>
#include <phy_n_rxiqcal.h>
#include <wlc_phy_int.h>
#include <wlc_phyreg_n.h>
#include <phy_utils_reg.h>
#include <phy_stf.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_n.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_n_stf_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_stf_info_t *stf_info;
};

/* Functions used by common layer as callbacks */
static int phy_n_stf_set_stf_chain(phy_type_stf_ctx_t *ctx,
		uint8 txchain, uint8 rxchain);

/* register phy type specific implementation */
phy_n_stf_info_t *
BCMATTACHFN(phy_n_stf_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_stf_info_t *stf_info)
{
	phy_n_stf_info_t *n_stf_info;
	phy_type_stf_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((n_stf_info = phy_malloc(pi, sizeof(phy_n_stf_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	n_stf_info->pi = pi;
	n_stf_info->ni = ni;
	n_stf_info->stf_info = stf_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.set_stf_chain = phy_n_stf_set_stf_chain;
	fns.chain_init = NULL;
	fns.ctx = n_stf_info;

	if (phy_stf_register_impl(stf_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_stf_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return n_stf_info;

	/* error handling */
fail:
	if (n_stf_info != NULL)
		phy_mfree(pi, n_stf_info, sizeof(phy_n_stf_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_stf_unregister_impl)(phy_n_stf_info_t *n_stf_info)
{
	phy_info_t *pi = n_stf_info->pi;
	phy_stf_info_t *stf_info = n_stf_info->stf_info;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_stf_unregister_impl(stf_info);
	phy_mfree(pi, n_stf_info, sizeof(phy_n_stf_info_t));
}

/* ********************************************** */
/* Function table registered function                                  */
/* ********************************************** */
static int
phy_n_stf_set_stf_chain(phy_type_stf_ctx_t *ctx, uint8 txchain, uint8 rxchain)
{
	phy_n_stf_info_t *stf_info = (phy_n_stf_info_t *)ctx;
	phy_stf_info_t *cmn_stf_info = (phy_stf_info_t *) stf_info->stf_info;
	phy_info_t *pi = stf_info->pi;

	PHY_TRACE(("phy_n_stf_set_stf_chain, new phy chain tx %d, rx %d", txchain, rxchain));

	phy_stf_set_phytxchain(cmn_stf_info, txchain);

	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, rxchain, 1);
	} else {
		wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, rxchain, 0);
	}

	return BCME_OK;
}
