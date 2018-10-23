/*
 * LCN20PHY Cache module implementation
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
 * $Id: phy_lcn20_cache.c 606042 2015-12-14 06:21:23Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_cache.h"
#include <phy_lcn20_cache.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <phy_utils_reg.h>

/* module private states */
struct phy_lcn20_cache_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_cache_info_t *ci;
};

/* local functions */
#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
/** dump calibration regs/info */
static void
wlc_phy_cal_dump_lcn20phy(phy_type_cache_ctx_t * cache_ctx, struct bcmstrbuf *b);
#endif // endif

/* register phy type specific implementation */
phy_lcn20_cache_info_t *
BCMATTACHFN(phy_lcn20_cache_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_cache_info_t *ci)
{
	phy_lcn20_cache_info_t *info;
	phy_type_cache_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_lcn20_cache_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn20_cache_info_t));
	info->pi = pi;
	info->lcn20i = lcn20i;
	info->ci = ci;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = info;
#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
	fns.dump_cal = wlc_phy_cal_dump_lcn20phy;
#endif // endif

	if (phy_cache_register_impl(ci, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_cache_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn20_cache_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_cache_unregister_impl)(phy_lcn20_cache_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_cache_info_t *ci = info->ci;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_cache_unregister_impl(ci);

	phy_mfree(pi, info, sizeof(phy_lcn20_cache_info_t));
}

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
/* dump calibration regs/info */
static void
wlc_phy_cal_dump_lcn20phy(phy_type_cache_ctx_t * cache_ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = (phy_info_t *)cache_ctx;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!pi->sh->up) {
		return;
	}
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
#ifdef LCN20_PAPD_ENABLE
	PHY_PAPD(("calling wlc_phy_papd_dump_eps_trace_lcn20\n"));
	wlc_phy_papd_dump_eps_trace_lcn20(pi, b);
	bcm_bprintf(b, "papdcalidx 000\n");
#endif /* LCN20_PAPD_ENABLE */

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif // endif
