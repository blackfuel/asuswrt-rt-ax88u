/*
 * VCO CAL module implementation.
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
 * $Id: phy_vcocal.c 656120 2016-08-25 08:17:57Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_vcocal.h"
#include <phy_rstr.h>
#include <phy_vcocal.h>

/* forward declaration */
typedef struct phy_vcocal_mem phy_vcocal_mem_t;

/* module private states */
struct phy_vcocal_info {
	phy_info_t 				*pi;	/* PHY info ptr */
	phy_type_vcocal_fns_t	*fns;	/* Function ptr */
	phy_vcocal_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_vcocal_mem {
	phy_vcocal_info_t		cmn_info;
	phy_type_vcocal_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_vcocal_info_t *
BCMATTACHFN(phy_vcocal_attach)(phy_info_t *pi)
{
	phy_vcocal_mem_t	*mem;
	phy_vcocal_info_t *cmn_info = NULL;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_vcocal_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Init vcocal params */

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_vcocal_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_vcocal_detach)(phy_vcocal_info_t *cmn_info)
{
	phy_vcocal_mem_t	*mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	if (!cmn_info)
		return;

	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null vcocal module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_vcocal_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_vcocal_register_impl)(phy_vcocal_info_t *cmn_info, phy_type_vcocal_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	*cmn_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_vcocal_unregister_impl)(phy_vcocal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->fns = NULL;
}

#if defined(WLTEST) || defined(RADIO_HEALTH_CHECK)
void
phy_vcocal_force(phy_info_t *pi)
{
	phy_vcocal_info_t *vi = pi->vcocali;
	phy_type_vcocal_fns_t *fns = vi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->force != NULL) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		(fns->force)(fns->ctx);
		wlapi_enable_mac(pi->sh->physhim);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}
#endif // endif
int phy_vcocal_status(phy_vcocal_info_t *vcocali)
{
	phy_type_vcocal_fns_t *fns = vcocali->fns;
	if (fns->status)
		return (fns->status)(fns->ctx);
	else
		return RADIO2069X_VCOCAL_IS_DONE;
}
