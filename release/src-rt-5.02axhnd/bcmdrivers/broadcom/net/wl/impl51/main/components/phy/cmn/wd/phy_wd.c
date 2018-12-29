/*
 * WatchDog callback function registration module implementation.
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
 * $Id: phy_wd.c 767086 2018-08-28 05:28:44Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_cache.h>
#include <phy_wd_api.h>
#include "phy_wd_cfg.h"
#include <phy_wd.h>
#include <phy_chanmgr_notif.h>

/* callback entry */
typedef struct {
	phy_wd_fn_t fn;
	phy_wd_ctx_t *ctx;
	uint16 prd;
	uint16 flags;
	uint8 order;
	uint8 states;
} phy_wd_cb_reg_t;

/* callback states */
#define CB_STATE_DEFER 0x1

/* multiple channel watchdog simulation */
typedef struct {
	uint32 last_wd;
} phy_wd_cache_t;

/* module private states */
struct phy_wd_info {
	phy_info_t *pi;
	uint32 opch_chg;
	uint8 cb_cnt;
	uint8 cb_sz;
	phy_wd_cb_reg_t *cb_tbl;
	phy_wd_cache_t *ctx;
	phy_cache_cubby_id_t ccid;
	uint8 phywatchdog_override_ctr;
};

/* module private states memory layout */
typedef struct {
	phy_wd_info_t info;
	phy_wd_cb_reg_t cb[PHY_WD_CB_REG_SZ];
/* add other variable size variables here at the end */
} phy_wd_mem_t;

/* debug */
#ifdef BCMDBG
#define WDMGR_DBGMSG_VERBOSE (1<<0)
static uint32 wdmgr_dbgmsg = WDMGR_DBGMSG_VERBOSE;
#define PHY_WDMGR(x) do {if (wdmgr_dbgmsg & WDMGR_DBGMSG_VERBOSE) PHY_WD(x);} while (0)
#else
#define PHY_WDMGR(x)
#endif // endif

/* local function declaration */
static int phy_wd_dowd(phy_wd_info_t *wi, bool mchan, bool non_mchan);
#if defined(PHYCAL_CACHING)
static void phy_wd_init_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf);
static int phy_wd_save_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf);
static int phy_wd_restore_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf);
#if defined(BCMDBG)
static int phy_wd_dump_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf, struct bcmstrbuf *b);
#else
#define phy_wd_dump_ctx NULL
#endif // endif

static int phy_wd_chanmgr_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data);
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG)
static int phy_wd_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* attach/detach */
phy_wd_info_t *
BCMATTACHFN(phy_wd_attach)(phy_info_t *pi)
{
	phy_wd_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_wd_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	info->cb_sz = PHY_WD_CB_REG_SZ;
	info->cb_tbl = ((phy_wd_mem_t *)info)->cb;
#if defined(PHYCAL_CACHING)
	/* reserve some space in cache */
	if (phy_cache_reserve_cubby(pi->cachei, phy_wd_init_ctx,
	                phy_wd_save_ctx, phy_wd_restore_ctx, phy_wd_dump_ctx, info,
	                sizeof(phy_wd_cache_t), &info->ccid) != BCME_OK) {
		PHY_ERROR(("%s: phy_cache_reserve_cubby failed\n", __FUNCTION__));
		goto fail;
	}

	/* register chanmgr notification callback */
	if (phy_chanmgr_notif_add_interest(pi->chanmgr_notifi,
	                phy_wd_chanmgr_notif, info, PHY_CHANMGR_NOTIF_ORDER_WD,
	                PHY_CHANMGR_NOTIF_OPCH_CHG) != BCME_OK) {
		PHY_ERROR(("%s: phy_chanmgr_notif_add_interest failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "phywd", phy_wd_dump, info);
#endif // endif
	info->phywatchdog_override_ctr = 0;
	return info;

	/* error */
fail:
	phy_wd_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_wd_detach)(phy_wd_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null wd module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_wd_mem_t));
}

/* Add a watchdog callback fn. Return BCME_XXXX. */
int
BCMATTACHFN(phy_wd_add_fn)(phy_wd_info_t *wi, phy_wd_fn_t fn, phy_wd_ctx_t *ctx,
	phy_wd_prd_t prd, phy_wd_order_t order, phy_wd_flag_t flags)
{
	uint16 j;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fn != NULL);
	ASSERT(prd != 0);

	if (wi->cb_cnt == wi->cb_sz) {
		PHY_ERROR(("%s: too many watchdog callbacks\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	/* insert callback entry in ascending order */
	for (j = 0; j < wi->cb_cnt; j ++) {
		/* insert a new callback right at here */
		if (order == wi->cb_tbl[j].order ||
		    ((j == 0 || order > wi->cb_tbl[j - 1].order) && order <= wi->cb_tbl[j].order)) {
			PHY_WDMGR(("%s: insert %u\n", __FUNCTION__, j));
			break;
		}
	}
	/* insert a new callback at after all existing entries when j == wi->cb_cnt */
	if (j == wi->cb_cnt) {
		PHY_WDMGR(("%s: append %u\n", __FUNCTION__, j));
	}

	/* move callbacks down by 1 entry */
	if (j < wi->cb_cnt) {
		memmove(&wi->cb_tbl[j + 1], &wi->cb_tbl[j],
		        (wi->cb_cnt - j) * sizeof(*wi->cb_tbl));
	}

	/* insert the new callback */
	wi->cb_tbl[j].fn = fn;
	wi->cb_tbl[j].ctx = ctx;
	wi->cb_tbl[j].flags = (uint16)flags;
	wi->cb_tbl[j].prd = (uint16)prd;
	wi->cb_tbl[j].order = (uint8)order;
	wi->cb_tbl[j].states = 0;

	wi->cb_cnt ++;

#if defined(BCMDBG)
#endif // endif

	return BCME_OK;
}

void
phy_wd_override(phy_info_t *pi, bool val)
{
	pi->phywatchdog_override = val;
}

static bool
phy_wd_detect_mchan(phy_wd_info_t *wi)
{
	return wi->opch_chg > 0;
}

void
phy_watchdog_suspend(phy_info_t *pi)
{
	phy_wd_info_t *wi = pi->wdi;
	wi->phywatchdog_override_ctr++;
}

void
phy_watchdog_resume(phy_info_t *pi)
{
	phy_wd_info_t *wi = pi->wdi;
	ASSERT(wi->phywatchdog_override_ctr > 0);
	if (wi->phywatchdog_override_ctr > 0) {
		wi->phywatchdog_override_ctr--;
	}
}

/* watchdog */
int
phy_watchdog(phy_info_t *pi)
{
	phy_wd_info_t *wi = pi->wdi;
	int ret;

	pi->sh->now++;

	PHY_WDMGR(("%s: now %u\n", __FUNCTION__, pi->sh->now));

	/* for QT, abort if no radio */
	if (ISSIM_ENAB(pi->sh->sih))
		return BCME_OK;

	if ((!pi->phywatchdog_override) || (wi->phywatchdog_override_ctr != 0))
		return BCME_OK;

	ret = phy_wd_dowd(wi, !phy_wd_detect_mchan(wi), TRUE);
	wi->opch_chg = 0;

	return ret;
}

static int
phy_wd_dowd(phy_wd_info_t *wi, bool mchan, bool non_mchan)
{
	phy_info_t *pi = wi->pi;
	uint16 j;

	PHY_WDMGR(("%s: mchan %d non-mchan %d now %u chanspec %d\n",
	           "phy_wd_dowd", mchan, non_mchan, pi->sh->now, pi->radio_chanspec));

	for (j = 0; j < wi->cb_cnt; j ++) {
		bool defer = (wi->cb_tbl[j].states & CB_STATE_DEFER) != 0;
		bool tick = (pi->sh->now % wi->cb_tbl[j].prd) == 0;
		bool doit = (mchan && (wi->cb_tbl[j].flags & PHY_WD_FLAG_MCHAN_AWARE) != 0) ||
			(non_mchan && (wi->cb_tbl[j].flags & PHY_WD_FLAG_MCHAN_AWARE) == 0);

		/* invoke all regardless and save the flag */
		if (tick) {
			if ((SCAN_RM_IN_PROGRESS(pi) &&
				(wi->cb_tbl[j].flags & PHY_WD_FLAG_SCAN_SKIP)) ||
				(PLT_INPROG_PHY(pi) &&
				(wi->cb_tbl[j].flags & PHY_WD_FLAG_PLT_SKIP)) ||
				(ASSOC_INPROG_PHY(pi) &&
				(wi->cb_tbl[j].flags & PHY_WD_FLAG_AS_SKIP)))
				continue;

			if ((SCAN_RM_IN_PROGRESS(pi) &&
				(wi->cb_tbl[j].flags & PHY_WD_FLAG_SCAN_DEFER)) ||
				(PLT_INPROG_PHY(pi) &&
				(wi->cb_tbl[j].flags & PHY_WD_FLAG_PLT_DEFER)) ||
				(ASSOC_INPROG_PHY(pi) &&
				(wi->cb_tbl[j].flags & PHY_WD_FLAG_AS_DEFER)) ||
				(doit && !(wi->cb_tbl[j].fn)(wi->cb_tbl[j].ctx))) {
				PHY_WDMGR(("%s: callback %p deferred at %u\n",
					__FUNCTION__, wi->cb_tbl[j].fn, pi->sh->now));
				wi->cb_tbl[j].states |= CB_STATE_DEFER;
			} else {
				wi->cb_tbl[j].states &= ~CB_STATE_DEFER;
			}
		}
		/* invoke those with 'deferred' flag */
		else if (defer) {
			if ((wi->cb_tbl[j].states & CB_STATE_DEFER) &&
				doit &&	!(wi->cb_tbl[j].fn)(wi->cb_tbl[j].ctx)) {
				PHY_WDMGR(("%s: callback %p deferred again at %u\n",
					__FUNCTION__, wi->cb_tbl[j].fn, pi->sh->now));
			} else {
				wi->cb_tbl[j].states &= ~CB_STATE_DEFER;
			}
		}
	}

	return BCME_OK;
}

#if defined(PHYCAL_CACHING)
static int
phy_wd_dosim(phy_wd_info_t *wi)
{
	phy_info_t *pi = wi->pi;

	PHY_TRACE(("phy_wd_dosim\n"));

	if (wi->ctx != NULL &&
	    pi->sh->now - wi->ctx->last_wd > 0) {
		wi->ctx->last_wd = pi->sh->now;
		return phy_wd_dowd(wi, TRUE, FALSE);
	}

	return BCME_OK;
}

/* chanmgr notification callback */
static int
phy_wd_chanmgr_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data)
{
	phy_wd_info_t *wi = (phy_wd_info_t *)ctx;
	phy_info_t *pi = wi->pi;

	PHY_TRACE(("phy_wd_chanmgr_notif\n"));

	if (wi->ctx) {
		wi->opch_chg ++;
		/* Calibrate if now > last_cal_time + glacial */
		if (PHY_PERICAL_MPHASE_PENDING(pi)) {
			PHY_CAL(("%s: Restarting calibration for 0x%x phase %d\n",
			    __FUNCTION__, pi->radio_chanspec, pi->cal_info->cal_phase_id));
			/* Delete any existing timer just in case */
			wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
			wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
		}
		return phy_wd_dosim(wi);
	} else {
		PHY_TRACE(("%s Don't do anything\n", "phy_wd_chanmgr_notif"));
		return BCME_OK;
	}
}

/* init wd states */
static void
phy_wd_init_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf)
{
	PHY_TRACE(("phy_wd_init_ctx\n"));

	((phy_wd_cache_t *) buf)->last_wd = ~0;
}

/* save wd states */
static int
phy_wd_save_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf)
{
	phy_wd_info_t *wi = (phy_wd_info_t *)ctx;

	PHY_TRACE(("phy_wd_save_ctx\n"));

	wi->ctx = NULL;
	return BCME_OK;
}

/* restore wd states */
static int
phy_wd_restore_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf)
{
	phy_wd_info_t *wi = (phy_wd_info_t *)ctx;

	PHY_TRACE(("phy_wd_restore_ctx: buf %p\n", buf));

	wi->ctx = (phy_wd_cache_t *) buf;

	return BCME_OK;
}

#if defined(BCMDBG)
static int
phy_wd_dump_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf, struct bcmstrbuf *b)
{
	phy_wd_info_t *wi = (phy_wd_info_t *)ctx;

	if (wi->ctx == NULL) {
		return BCME_OK;
	}

	bcm_bprintf(b, "wd:\n");
	bcm_bprintf(b, "  last_wd %u\n", wi->ctx->last_wd);

	return BCME_OK;
}
#endif // endif
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG)
static int
phy_wd_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_wd_info_t *wi = (phy_wd_info_t *)ctx;
	phy_info_t *pi = wi->pi;
	uint16 j;

	bcm_bprintf(b, "now: %u\n", pi->sh->now);
	bcm_bprintf(b, "cb: max %u cnt %u\n", wi->cb_sz, wi->cb_cnt);
	for (j = 0; j < wi->cb_cnt; j ++) {
		bcm_bprintf(b, "  idx %u: order %u cb %p ctx %p states 0x%x flags 0x%x prd %u\n",
		            j, wi->cb_tbl[j].order, wi->cb_tbl[j].fn, wi->cb_tbl[j].ctx,
		            wi->cb_tbl[j].states, wi->cb_tbl[j].flags, wi->cb_tbl[j].prd);
	}

	return BCME_OK;
}
#endif // endif
