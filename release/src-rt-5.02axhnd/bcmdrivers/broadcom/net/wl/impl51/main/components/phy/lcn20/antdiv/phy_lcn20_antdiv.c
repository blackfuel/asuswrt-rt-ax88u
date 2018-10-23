/*
 * LCN20PHY ANTennaDIVersity module implementation
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
 * $Id: phy_lcn20_antdiv.c 629393 2016-04-05 06:55:25Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_antdiv.h"
#include <phy_lcn20.h>
#include <phy_lcn20_rstr.h>
#include <phy_lcn20_antdiv.h>
#include <phy_antdiv.h>
#include <phy_antdiv_cfg.h>

#include <wlc_phyreg_lcn20.h>

#include <phy_utils_reg.h>
#include <phy_utils_var.h>

/* forward declaration */
typedef struct phy_lcn20_antdiv_mem phy_lcn20_antdiv_mem_t;

/* module private states */
struct phy_lcn20_antdiv_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_antdiv_info_t *di;
	phy_lcn20_antdiv_mem_t *mem;
	phy_swdiv_t *swdiv;
};

/* module private states memory layout */
struct phy_lcn20_antdiv_mem {
	phy_lcn20_antdiv_info_t info;
	phy_swdiv_t swdiv;
};

/* local functions */
#ifdef WLC_SW_DIVERSITY
static void phy_lcn20_swdiv_init(phy_type_antdiv_ctx_t *ctx);
static uint8 phy_lcn20_swdiv_get_ant(phy_type_antdiv_ctx_t *ctx);
static void phy_lcn20_swdiv_set_ant(phy_type_antdiv_ctx_t *ctx, uint8 ant);
#endif // endif

/* register phy type specific implementation */
phy_lcn20_antdiv_info_t *
BCMATTACHFN(phy_lcn20_antdiv_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_antdiv_info_t *di)
{
	phy_lcn20_antdiv_mem_t  *mem;
	phy_lcn20_antdiv_info_t *info;
	phy_type_antdiv_fns_t fns;
#ifdef WLC_SW_DIVERSITY
	phy_swdiv_t *swdiv;
#endif // endif

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((mem = phy_malloc(pi, sizeof(phy_lcn20_antdiv_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(mem, sizeof(phy_lcn20_antdiv_mem_t));

	info = &(mem->info);
	info->pi = pi;
	info->lcn20i = lcn20i;
	info->di = di;
	info->mem = mem;

#ifdef WLC_SW_DIVERSITY
	if (PHYSWDIV_ENAB(pi)) {
		swdiv = &(mem->swdiv);
		info->swdiv = swdiv;
		phy_swdiv_read_srom(pi, swdiv);
	}
#endif /* WLC_SW_DIVERSITY */

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#ifdef WLC_SW_DIVERSITY
	fns.initswdiv = phy_lcn20_swdiv_init;
	fns.getswdiv = phy_lcn20_swdiv_get_ant;
	fns.setswdiv = phy_lcn20_swdiv_set_ant;
#endif // endif
	fns.setrx = (phy_type_antdiv_set_rx_fn_t)phy_lcn20_antdiv_set_rx;
	fns.ctx = info;

	phy_antdiv_register_impl(di, &fns);

	return info;

	/* error handling */
fail:
	if (mem != NULL)
		phy_mfree(pi, mem, sizeof(phy_lcn20_antdiv_mem_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_antdiv_unregister_impl)(phy_lcn20_antdiv_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_antdiv_info_t *di = info->di;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_antdiv_unregister_impl(di);

	phy_mfree(pi, info->mem, sizeof(phy_lcn20_antdiv_mem_t));
}

/* Setup */
void
phy_lcn20_antdiv_set_rx(phy_lcn20_antdiv_info_t *info, uint8 ant)
{
	PHY_TRACE(("%s: ant 0x%x\n", __FUNCTION__, ant));
}

#ifdef WLC_SW_DIVERSITY
static void
phy_lcn20_swdiv_init(phy_type_antdiv_ctx_t *ctx)
{
	PHY_TRACE(("%s \n", __FUNCTION__));
}

static void
phy_lcn20_swdiv_set_ant(phy_type_antdiv_ctx_t *ctx, uint8 new_ant)
{
	PHY_TRACE(("%s \n", __FUNCTION__));
}

static uint8
phy_lcn20_swdiv_get_ant(phy_type_antdiv_ctx_t *ctx)
{
	PHY_TRACE(("%s \n", __FUNCTION__));
	return 0;
}

void
phy_lcn20_swdiv_epa_pd(phy_lcn20_antdiv_info_t *di, bool disable)
{
	PHY_TRACE(("%s \n", __FUNCTION__));
}
#endif /* WLC_SW_DIVERSITY */
