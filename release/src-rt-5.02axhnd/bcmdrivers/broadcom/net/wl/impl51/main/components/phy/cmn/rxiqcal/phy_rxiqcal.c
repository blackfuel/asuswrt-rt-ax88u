/*
 * RXIQ CAL module implementation.
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
 * $Id: phy_rxiqcal.c 742511 2018-01-22 14:14:24Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rxiqcal.h"
#include <phy_rstr.h>
#include <phy_rxiqcal.h>

/* forward declaration */
typedef struct phy_rxiqcal_mem phy_rxiqcal_mem_t;

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
static int phy_rxiq_mismatch_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* module private states */
struct phy_rxiqcal_priv_info {
	phy_info_t 				*pi;		/* PHY info ptr */
	phy_type_rxiqcal_fns_t	*fns;		/* Function ptr */
};

/* module private states memory layout */
struct phy_rxiqcal_mem {
	phy_rxiqcal_info_t		cmn_info;
	phy_type_rxiqcal_fns_t	fns;
	phy_rxiqcal_priv_info_t priv;
	phy_rxiqcal_data_t data;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_rxiqcal_info_t *
BCMATTACHFN(phy_rxiqcal_attach)(phy_info_t *pi)
{
	phy_rxiqcal_info_t	*cmn_info = NULL;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((cmn_info = phy_malloc(pi, sizeof(phy_rxiqcal_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	cmn_info->priv = &((phy_rxiqcal_mem_t *)cmn_info)->priv;
	cmn_info->priv->pi = pi;
	cmn_info->priv->fns = &((phy_rxiqcal_mem_t *)cmn_info)->fns;
	cmn_info->data = &((phy_rxiqcal_mem_t *)cmn_info)->data;

	/* init the rxiqcal states */
	/* pi->phy_rx_diglpf_default_coeffs are not set yet */
	cmn_info->data->phy_rx_diglpf_default_coeffs_valid = FALSE;

	/* Register callbacks */

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "rxiq_mismatch", phy_rxiq_mismatch_dump, pi);
#endif // endif

	return cmn_info;

	/* error */
fail:
	phy_rxiqcal_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_rxiqcal_detach)(phy_rxiqcal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	if (cmn_info == NULL) {
		PHY_INFORM(("%s: null rxiqcal module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->priv->pi, cmn_info, sizeof(phy_rxiqcal_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_rxiqcal_register_impl)(phy_rxiqcal_info_t *cmn_info, phy_type_rxiqcal_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	*cmn_info->priv->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_rxiqcal_unregister_impl)(phy_rxiqcal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->priv->fns = NULL;
}

void phy_rxiqcal_scanroam_cache(phy_info_t *pi, bool set)
{
	phy_type_rxiqcal_fns_t *fns = pi->rxiqcali->priv->fns;

	if (fns->scanroam_cache != NULL)
		(fns->scanroam_cache)(fns->ctx, set);
}

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
static int
phy_rxiq_mismatch_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;
	phy_rxiqcal_info_t *rxiqcali = pi->rxiqcali;
	phy_type_rxiqcal_fns_t *fns = rxiqcali->priv->fns;
	int ret = BCME_UNSUPPORTED;

	if (!pi->sh->clk) {
		return BCME_NOCLK;
	}

	if (fns->rxiq_mismatch_dump) {
		ret = (fns->rxiq_mismatch_dump)(fns->ctx, b);
	}

	return ret;
}
#endif // endif
