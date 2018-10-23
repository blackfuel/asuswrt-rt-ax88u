/*
 * DeepSleepInit module implementation
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
 * $Id: phy_dsi.c 583048 2015-08-31 16:43:34Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <phy_mem.h>

#include <phy_dsi_api.h>
#include <phy_dsi.h>
#include "phy_dsi_st.h"
#include "phy_type_dsi.h"

/* module private states */
struct phy_dsi_info {
	phy_info_t *pi;
	phy_type_dsi_fns_t *fns;
	phy_dsi_state_t *st;
};

/* module private states memory layout */
typedef struct {
	phy_dsi_info_t info;
	phy_type_dsi_fns_t fns;
	phy_dsi_state_t st;
} phy_dsi_mem_t;

/* attach/detach */
phy_dsi_info_t *
BCMATTACHFN(phy_dsi_attach)(phy_info_t *pi)
{
	phy_dsi_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_dsi_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	info->pi = pi;
	info->fns = &((phy_dsi_mem_t *)info)->fns;
	info->st = &((phy_dsi_mem_t *)info)->st;

	return info;

	/* error */
fail:
	phy_dsi_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_dsi_detach)(phy_dsi_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null dsi module\n", __FUNCTION__));
		return;
	}
	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_dsi_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_dsi_register_impl)(phy_dsi_info_t *ri, phy_type_dsi_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ri->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_dsi_unregister_impl)(phy_dsi_info_t *ri)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/*
 * Query the states pointer.
 */
phy_dsi_state_t *
phy_dsi_get_st(phy_dsi_info_t *di)
{
	return di->st;
}

/* Returns which path to take for phy & radio initializations
 * DeepSleepInit (true) or Normal Init (false)
 */
bool
phy_get_dsi_trigger_st(phy_info_t *pi)
{
	phy_dsi_info_t *di = pi->dsii;
	phy_dsi_state_t *st = phy_dsi_get_st(di);

	return st->trigger;
}

void
BCMINITFN(phy_dsi_restore)(phy_info_t *pi)
{
	phy_dsi_info_t *info = pi->dsii;
	phy_type_dsi_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* redirect the request to PHY type specific implementation */
	ASSERT(fns->restore != NULL);
	(fns->restore)(fns->ctx);
}

int
phy_dsi_save(phy_info_t *pi)
{
	phy_dsi_info_t *info = pi->dsii;
	phy_type_dsi_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* redirect the request to PHY type specific implementation */
	ASSERT(fns->save != NULL);

	return (fns->save)(fns->ctx);
}
