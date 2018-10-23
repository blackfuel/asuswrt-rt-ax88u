/*
 * Napping module implementation.
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
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_nap.h"
#include <phy_nap.h>
#include <phy_nap_api.h>
#include <phy_utils_var.h>

/* forward declaration */
typedef struct phy_nap_mem phy_nap_mem_t;

/* module private states */
struct phy_nap_info {
	phy_info_t		*pi;	/* PHY info ptr */
	phy_type_nap_fns_t	*fns;	/* PHY specific function ptrs */
	phy_nap_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_nap_mem {
	phy_nap_info_t		cmn_info;
	phy_type_nap_fns_t	fns;
	/* add other variable size variables here at the end */
};

static const char BCMATTACHDATA(rstr_napen)[] = "ulpnap";

/* attach/detach */
phy_nap_info_t *
BCMATTACHFN(phy_nap_attach)(phy_info_t *pi)
{
	phy_nap_mem_t	*mem = NULL;
	phy_nap_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_nap_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	return cmn_info;

fail:
	phy_nap_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_nap_detach)(phy_nap_info_t *cmn_info)
{
	phy_nap_mem_t	*mem;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info) {
		return;
	}

	/* Freeup memory associated with cmn_info. */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null nap module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_nap_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_nap_register_impl)(phy_nap_info_t *mi, phy_type_nap_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*mi->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_nap_unregister_impl)(phy_nap_info_t *mi)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

#ifdef WL_NAP
void
wlc_phy_nap_status_get(wlc_phy_t *ppi, uint16 *bitmap, uint8 *hwstatus, uint32 *ms)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_nap_fns_t *fns;
	ASSERT(pi != NULL);
	fns = pi->napi->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->get_status != NULL) {
		(fns->get_status)(fns->ctx, bitmap, hwstatus, ms);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

void
wlc_phy_nap_disable_req_set(wlc_phy_t *ppi, uint16 req, bool disable,
	bool agc_reconfig, uint8 req_id)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_nap_fns_t *fns;
	ASSERT(pi != NULL);
	fns = pi->napi->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->set_disable_req != NULL) {
		(fns->set_disable_req)(fns->ctx, req, disable, agc_reconfig, req_id);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}
int8
phy_nap_get_energy_threshold(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_nap_fns_t *fns = pi->napi->fns;

	if (fns->get_energy_threshold) {
		return (fns->get_energy_threshold)(fns->ctx);
	} else {
		return BCME_UNSUPPORTED;
	}
}
void
phy_nap_set_energy_threshold(wlc_phy_t *ppi, int8 energy_threshold)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_nap_fns_t *fns = pi->napi->fns;
	if (fns->set_energy_threshold) {
		return (fns->set_energy_threshold)(fns->ctx, energy_threshold);
	}
}
#endif /* WL_NAP */
