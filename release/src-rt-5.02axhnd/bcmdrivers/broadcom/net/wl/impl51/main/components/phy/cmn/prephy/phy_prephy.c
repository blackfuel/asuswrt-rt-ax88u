/*
 * Prephy module implementation.
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
 * $Id: phy_prephy.c 658443 2016-09-08 01:19:02Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_prephy.h"
#include <phy_rstr.h>
#include <phy_prephy.h>
#include <phy_prephy_api.h>
#include <phy_utils_var.h>
#include <phy_ac_prephy.h>

/* forward declaration */
typedef struct phy_prephy_mem phy_prephy_mem_t;

/* module private states */
struct phy_prephy_info {
	prephy_info_t			*pi;	/* PHY info ptr */
	phy_type_prephy_fns_t	*fns;	/* PHY specific function ptrs */
	phy_prephy_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_prephy_mem {
	phy_prephy_info_t cmn_info;
	phy_type_prephy_fns_t fns;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_prephy_info_t *
BCMATTACHFN(phy_prephy_attach)(prephy_info_t *pi)
{
	phy_prephy_mem_t	*mem = NULL;
	phy_prephy_info_t	*cmn_info = NULL;
	uint phy_type = pi->pubpi->phy_type;
	phy_type_prephy_fns_t	*fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	BCM_REFERENCE(fns);

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_prephy_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	fns = cmn_info->fns;

	switch (phy_type) {
#if ACCONF || ACCONF2
		case PHY_TYPE_AC:
			fns->phy_type_prephy_vasip_ver_get = phy_ac_prephy_vasip_ver_get;
			fns->phy_type_prephy_vasip_proc_reset = phy_ac_prephy_vasip_proc_reset;
			fns->phy_type_prephy_vasip_clk_set = phy_ac_prephy_vasip_clk_set;
			fns->phy_type_prephy_caps = phy_ac_prephy_caps;
			break;
#endif /* ACCONF || ACCONF2 */
		default:
			PHY_ERROR(("%s: prephy only support PHY_TYPE_AC\n", __FUNCTION__));
			break;
	}

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_prephy_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_prephy_detach)(phy_prephy_info_t *cmn_info)
{
	phy_prephy_mem_t	*mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info)
		return;

	/* Freeup memory associated with cmn_info */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null misc module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_prephy_mem_t));
}

/* vasip version read without pi */
void
phy_prephy_vasip_ver_get(prephy_info_t *pi, d11regs_t *regs, uint32 *vasipver)
{
	phy_type_prephy_fns_t *fns = pi->prephyi->fns;

	if (fns->phy_type_prephy_vasip_ver_get != NULL)
		fns->phy_type_prephy_vasip_ver_get(pi, regs, vasipver);
}

/* vasip reset without pi */
void
phy_prephy_vasip_proc_reset(prephy_info_t *pi, d11regs_t *regs, bool reset)
{
	phy_type_prephy_fns_t *fns = pi->prephyi->fns;

	if (fns->phy_type_prephy_vasip_proc_reset != NULL)
		fns->phy_type_prephy_vasip_proc_reset(pi, regs, reset);
}

uint32
phy_prephy_phy_caps(prephy_info_t *pi, uint32 *caps)
{
	int ret = BCME_OK;
	phy_type_prephy_fns_t *fns = pi->prephyi->fns;
	uint32 sflags = 0;

	ASSERT(caps);
	if (fns->phy_type_prephy_caps != NULL) {
		ret = fns->phy_type_prephy_caps(pi, caps);
		if (ret) {
			/* possibly ac phy, but not having 2g/5g support in cap regs */
			goto recompute;
		}
		goto done;
	}

recompute:

	sflags = si_core_sflags(pi->sh->sih, 0, 0);
	if ((sflags & (SISF_5G_PHY | SISF_DB_PHY)) != 0) {
		*caps = PHY_PREATTACH_CAP_SUP_5G;
	}
	if ((sflags & (SISF_2G_PHY | SISF_DB_PHY)) != 0) {
		*caps |= PHY_PREATTACH_CAP_SUP_2G;
	}

	ret = BCME_OK;
done:
	return ret;
}

/* vasip clock set without pi */
void
phy_prephy_vasip_clk_set(prephy_info_t *pi, d11regs_t *regs, bool set)
{
	phy_type_prephy_fns_t *fns = pi->prephyi->fns;

	if (fns->phy_type_prephy_vasip_clk_set != NULL)
		fns->phy_type_prephy_vasip_clk_set(pi, regs, set);
}
