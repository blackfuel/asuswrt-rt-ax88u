/*
 * lcn20PHY ANAcore contorl module implementation
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
 * $Id: phy_lcn20_ana.c 583048 2015-08-31 16:43:34Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_ana.h>
#include "phy_type_ana.h"
#include <phy_lcn20.h>
#include <phy_lcn20_ana.h>

#include <wlc_phyreg_lcn20.h>
#include <phy_utils_reg.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_lcn20_ana_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_ana_info_t *ani;
};

/* local functions */
static int phy_lcn20_ana_switch(phy_type_ana_ctx_t *ctx, bool on);

/* Register/unregister lcn20PHY specific implementation to common layer */
phy_lcn20_ana_info_t *
BCMATTACHFN(phy_lcn20_ana_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_ana_info_t *ani)
{
	phy_lcn20_ana_info_t *info;
	phy_type_ana_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn20_ana_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn20_ana_info_t));
	info->pi = pi;
	info->lcn20i = lcn20i;
	info->ani = ani;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_lcn20_ana_switch;
	fns.ctx = info;

	phy_ana_register_impl(ani, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn20_ana_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_ana_unregister_impl)(phy_lcn20_ana_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_ana_info_t *ani = info->ani;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_ana_unregister_impl(ani);

	phy_mfree(pi, info, sizeof(phy_lcn20_ana_info_t));
}

/* switch anacore on/off */
static int
phy_lcn20_ana_switch(phy_type_ana_ctx_t *ctx, bool on)
{
	phy_lcn20_ana_info_t *info = (phy_lcn20_ana_info_t *)ctx;
	phy_info_t *pi = info->pi;

	wlc_lcn20phy_anacore(pi, on);

	return BCME_OK;
}
