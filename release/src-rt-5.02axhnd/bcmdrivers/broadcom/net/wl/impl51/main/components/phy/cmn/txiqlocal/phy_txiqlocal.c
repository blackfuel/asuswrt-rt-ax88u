/*
 * TXIQLO CAL module implementation.
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
 * $Id: phy_txiqlocal.c 635707 2016-05-05 00:32:31Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_txiqlocal.h"
#include <phy_rstr.h>
#include <phy_txiqlocal.h>
#include <phy_utils_var.h>

/* forward declaration */
typedef struct phy_txiqlocal_mem phy_txiqlocal_mem_t;

/* module private states */
struct phy_txiqlocal_priv_info {
	phy_info_t 					*pi;		/* PHY info ptr */
	phy_type_txiqlocal_fns_t	*fns;		/* Function ptr */
};

/* module private states memory layout */
struct phy_txiqlocal_mem {
	phy_txiqlocal_info_t		cmn_info;
	phy_type_txiqlocal_fns_t	fns;
	phy_txiqlocal_priv_info_t priv;
	phy_txiqlocal_data_t data;
};

/* local function declaration */

/* attach/detach */
phy_txiqlocal_info_t *
BCMATTACHFN(phy_txiqlocal_attach)(phy_info_t *pi)
{
	phy_txiqlocal_mem_t	*mem;
	phy_txiqlocal_info_t *cmn_info = NULL;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_txiqlocal_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	cmn_info = &(mem->cmn_info);
	cmn_info->priv = &(mem->priv);
	cmn_info->priv->pi = pi;
	cmn_info->priv->fns = &(mem->fns);
	cmn_info->data = &(mem->data);

	/* init the txiqlocal states */
	mem->data.txiqcalidx2g = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txiqcalidx2g, -1));
	mem->data.txiqcalidx5g = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txiqcalidx5g, -1));

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_txiqlocal_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_txiqlocal_detach)(phy_txiqlocal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	if (cmn_info == NULL) {
		PHY_INFORM(("%s: null txiqlocal module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->priv->pi, cmn_info, sizeof(phy_txiqlocal_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_txiqlocal_register_impl)(phy_txiqlocal_info_t *cmn_info,
	phy_type_txiqlocal_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	*cmn_info->priv->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_txiqlocal_unregister_impl)(phy_txiqlocal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->priv->fns = NULL;
}

/* Inter-module interfaces and downward interfaces to PHY type specific implemenation */
/* Intra-module API for IOVAR */
void phy_txiqlocal_txiqccget(phy_info_t *pi, void *a)
{
	phy_type_txiqlocal_fns_t *fns = pi->txiqlocali->priv->fns;

	if (fns->txiqccget != NULL)
		(fns->txiqccget)(fns->ctx, a);
}

void phy_txiqlocal_txiqccset(phy_info_t *pi, void *p)
{
	phy_type_txiqlocal_fns_t *fns = pi->txiqlocali->priv->fns;

	if (fns->txiqccset != NULL)
		(fns->txiqccset)(fns->ctx, p);
}

void phy_txiqlocal_txloccget(phy_info_t *pi, void *a)
{
	phy_type_txiqlocal_fns_t *fns = pi->txiqlocali->priv->fns;

	if (fns->txloccget != NULL)
		(fns->txloccget)(fns->ctx, a);
}

void phy_txiqlocal_txloccset(phy_info_t *pi, void *p)
{
	phy_type_txiqlocal_fns_t *fns = pi->txiqlocali->priv->fns;

	if (fns->txloccset != NULL)
		(fns->txloccset)(fns->ctx, p);
}

void phy_txiqlocal_scanroam_cache(phy_info_t *pi, bool set)
{
	phy_type_txiqlocal_fns_t *fns = pi->txiqlocali->priv->fns;

	if (fns->scanroam_cache != NULL)
		(fns->scanroam_cache)(fns->ctx, set);
}
