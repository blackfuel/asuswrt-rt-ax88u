/*
 * CACHE module implementation
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
 * $Id: phy_cache.c 743056 2018-01-24 15:24:31Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <phy_dbg.h>
#include <phy_dbg_api.h>
#include <phy_init.h>
#include <phy_mem.h>
#include <phy_api.h>
#include <phy_chanmgr_notif.h>
#include "phy_cache_cfg.h"
#include <phy_type_cache.h>
#include <phy_cache_api.h>
#include <wlc_chctx_reg.h>
#include <phy_init.h>
#include <phy_cmn.h>

/* This is temporary. Need to modularize the members of calcache */
typedef struct {
	ch_calcache_t phy_calcache;
	acphy_ram_calcache_t ram_cache;
} phy_calcache_mem_t;

/* cache control entry */
typedef struct {
	chanspec_t chanspec;
	uint8 state;
	uint8 ctxid;	/* ctx id */
	uint8 *bufp;	/* storage */
} phy_cache_ctl_t;

/* state */
#define CTL_ST_USED	(1<<0)

typedef struct {
	/* Calibration cache buffer vars */
	uint8		*cal_cache_buf;
	uint32		cal_cache_buf_sz;
	uint16		cal_cache_buf_bmp;
} calcachebuf_param_t;

/* module private states */
struct phy_cache_info {
	phy_info_t *pi;
	phy_type_cache_fns_t *fns;	/* PHY specific function ptrs */

	wlc_chctx_reg_t *chctx;		/* channel context management module handle */

	/* global states */
	phy_cache_cubby_id_t ccid;	/* cache cubby ID */

	/* context size */
	uint16 bufsz;

	/* cache control */
	uint8 ctl_sz;
	phy_cache_ctl_t *ctl;

	/* common scratch buffer */
	uint16	reuse_buffer_size;
	void*	reuse_buffer;
	int8	reuse_buffer_lock; /* -1 Uninitialized, 0 Free, 1 Locked */

	/* Calibration cache related parameters */
	calcachebuf_param_t *calcachebuf_param;
};

/* module private states memory layout */
typedef struct {
	phy_cache_info_t info;
	phy_cache_ctl_t ctl[PHY_CACHE_SZ];
	phy_type_cache_fns_t fns;
} phy_cache_mem_t;

/* local function declaration */
static int phy_cache_init(phy_init_ctx_t *ctx);
static int phy_cache_down(phy_init_ctx_t *ctx);

#if defined(PHYCAL_CACHING)
static int phy_cache_chspec_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data);
static int phy_cache_restore(phy_cache_info_t *ci, chanspec_t chanspec);
static void phy_cache_free(phy_cache_info_t *ci);
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG)
static int phy_cache_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
static int wlc_phydump_phycal(void *ctx, struct bcmstrbuf *b);
#endif // endif

#if defined(PHYCAL_CACHING)
static void phy_cache_init_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf);
static int phy_cache_save_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf);
static int phy_cache_restore_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf);
#endif /* PHYCAL_CACHING */

#if PHY_CACHE_PREALLOC
static int phy_set_calcachebufbmp(phy_cache_info_t *ci, uint8 calbufidx, bool set);
#endif /* PHY_CACHE_PREALLOC */

//#undef PHY_TRACE
//#define PHY_TRACE(x) printf x
//#undef PHY_CAL
//#define PHY_CAL(x) printf x

/* attach/detach */
phy_cache_info_t *
BCMATTACHFN(phy_cache_attach)(phy_info_t *pi)
{
	phy_cache_info_t *info;
#if defined(PHYCAL_CACHING)
	uint16 events = (PHY_CHANMGR_NOTIF_OPCHCTX_OPEN | PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE |
	                 PHY_CHANMGR_NOTIF_OPCH_CHG);
#endif /* PHYCAL_CACHING */
	shared_phy_t *sh = pi->sh;
#if PHY_CACHE_PREALLOC
	int ref_count = 0;
	BCM_REFERENCE(ref_count);
#endif // endif

	PHY_TRACE(("%s\n", "phy_cache_attach"));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_cache_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	info->ctl = ((phy_cache_mem_t *)info)->ctl;
	info->ctl_sz = PHY_CACHE_SZ;

	info->fns = &((phy_cache_mem_t *)info)->fns;
	info->reuse_buffer_lock = -1; /* Uninitialized */
	/* Scratch buffer user should register the required size during attach time */
	info->reuse_buffer_size = 0;

#if PHY_CACHE_PREALLOC
	/* OBJECT REGISTRY: check if shared key has value already stored */
	info->calcachebuf_param = (calcachebuf_param_t *) wlapi_obj_registry_get(sh->physhim,
			OBJR_PHY_CALCH_CMN_INFO);

	if (info->calcachebuf_param == NULL) {
		if ((info->calcachebuf_param =
				phy_malloc(pi, sizeof(calcachebuf_param_t))) == NULL) {
			PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", sh->unit,
			          __FUNCTION__, (int) sizeof(calcachebuf_param_t)));
			return NULL;
		}

		/* OBJECT REGISTRY: We are the first instance, store value for key */
		wlapi_obj_registry_set(sh->physhim,
				OBJR_PHY_CALCH_CMN_INFO, info->calcachebuf_param);
	}

	/* OBJECT REGISTRY: Reference the stored value in both instances */
	ref_count = wlapi_obj_registry_ref(sh->physhim, OBJR_PHY_CALCH_CMN_INFO);
	ASSERT(ref_count <= MAX_RSDB_MAC_NUM);
#endif /* PHY_CACHE_PREALLOC */

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_cache_init, info, PHY_INIT_CACHE) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register down fn */
	if (phy_init_add_down_fn(pi->initi, phy_cache_down, info, PHY_DOWN_CACHE) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_down_fn failed\n", __FUNCTION__));
		goto fail;
	}

#ifdef PHYCAL_CACHING
	if ((info->chctx = wlc_chctx_reg_attach(sh->osh, 0, WLC_CHCTX_REG_V_TYPE,
			PHY_CACHE_REG_SZ, PHY_CACHE_REG_SZ, PHY_CACHE_SZ)) == NULL) {
		PHY_ERROR(("%s: wlc_chctx_reg_attach failed\n", __FUNCTION__));
		goto fail;
	}

	/* Reserve some space in cache: This is temporary.
	 * We need to modularize the members of calcache to use the new calmgr.
	 * It is Important to note that this cache cubby is reserved first.
	 * The offset 0 is thus used to index into the buffer for calcache.
	 */
	if (phy_cache_reserve_cubby(info, phy_cache_init_ctx,
	                phy_cache_save_ctx, phy_cache_restore_ctx, NULL, info,
	                sizeof(phy_calcache_mem_t), &info->ccid) != BCME_OK) {
		PHY_ERROR(("%s: phy_cache_reserve_cubby failed\n", __FUNCTION__));
		goto fail;
	}

	/* register chspec notification callback */
	if (phy_chanmgr_notif_add_interest(pi->chanmgr_notifi,
	                phy_cache_chspec_notif, info, PHY_CHANMGR_NOTIF_ORDER_CACHE,
	                events) != BCME_OK) {
		PHY_ERROR(("%s: phy_chanmgr_notif_add_interest failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "phycache", phy_cache_dump, info);
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
	phy_dbg_add_dump_fn(pi, "phycal", wlc_phydump_phycal, info);
#endif // endif

	return info;

	/* error */
fail:
	phy_cache_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_cache_detach)(phy_cache_info_t *info)
{
	phy_info_t *pi;
	shared_phy_t *sh;
#if PHY_CACHE_PREALLOC
	int ref_count = 0;
#endif // endif
	BCM_REFERENCE(sh);

	PHY_TRACE(("%s\n", "phy_cache_detach"));

	if (info == NULL) {
		PHY_INFORM(("%s: null cache module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;
	sh = pi->sh;

#ifdef PHYCAL_CACHING
	/* free cache entries */
	phy_cache_free(info);

	if (info->chctx != NULL) {
		wlc_chctx_reg_detach(info->chctx);
	}
#endif // endif

#if PHY_CACHE_PREALLOC
		/* OBJECT REGISTRY: if we are the last to refer to the shared key */
		ref_count = wlapi_obj_registry_unref(sh->physhim, OBJR_PHY_CALCH_CMN_INFO);

		if (ref_count == 0) {
			/* dealloc the buffer, if in use */
			if (info->calcachebuf_param) {
				wlapi_obj_registry_set(sh->physhim, OBJR_PHY_CALCH_CMN_INFO, NULL);
				phy_mfree(pi, info->calcachebuf_param, sizeof(calcachebuf_param_t));
				info->calcachebuf_param = NULL;
			}
		}
#endif /* PHY_CACHE_PREALLOC */

	phy_mfree(pi, info, sizeof(phy_cache_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_cache_register_impl)(phy_cache_info_t *ci, phy_type_cache_fns_t *fns)
{
	PHY_TRACE(("%s\n", "phy_cache_register_impl"));

	*ci->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_cache_unregister_impl)(phy_cache_info_t *ci)
{
	PHY_TRACE(("%s\n", "phy_cache_unregister_impl"));
}

/* Cache Init */
static int
WLBANDINITFN(phy_cache_init)(phy_init_ctx_t *ctx)
{
	phy_cache_info_t *cachei = (phy_cache_info_t *)ctx;
#if PHY_CACHE_PREALLOC
	calcachebuf_param_t *calbuf = cachei->calcachebuf_param;
	phy_info_t *pi = cachei->pi;
#endif // endif

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (cachei->reuse_buffer_lock == -1 && cachei->reuse_buffer_size > 0) {
		cachei->reuse_buffer = phy_malloc_fatal(cachei->pi, cachei->reuse_buffer_size);
		cachei->reuse_buffer_lock = 0; /* Initialized and free */
	}

#if PHY_CACHE_PREALLOC
	/* PHY Calibration cache related inits */

	/* If buffer already allocated by the other PHY, exit */
	if (calbuf->cal_cache_buf) {
		return BCME_OK;
	}

	/* Allocate Cal chache buffer block and store pointer */
	calbuf->cal_cache_buf_sz = PHY_CACHE_SZ * cachei->bufsz;
	calbuf->cal_cache_buf = (uint8 *) phy_malloc_fatal(pi,
			calbuf->cal_cache_buf_sz);
	calbuf->cal_cache_buf_bmp = 0;
#endif /* PHY_CACHE_PREALLOC */

	return BCME_OK;
}

/* Cache Down */
static int
BCMUNINITFN(phy_cache_down)(phy_init_ctx_t *ctx)
{
	phy_cache_info_t *cachei = (phy_cache_info_t *)ctx;
	phy_info_t *pi = cachei->pi;
	uint sliceidx = pi->sh->unit;
#if PHY_CACHE_PREALLOC
	calcachebuf_param_t *calbuf = cachei->calcachebuf_param;
#endif // endif
	bool moreslicesup = phy_get_sliceupstate(pi->cmni) ^ (1 << sliceidx);

	BCM_REFERENCE(moreslicesup);

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (cachei->reuse_buffer) {
		phy_mfree(cachei->pi, cachei->reuse_buffer, cachei->reuse_buffer_size);
		cachei->reuse_buffer_lock = -1;
	}

#if PHY_CACHE_PREALLOC
	/* PHY Calibration cache related deinits */

	/* Delete the cache control structure as they are not relevant for this pi */
#ifdef PHYCAL_CACHING
		/* free cache entries */
	phy_cache_free(cachei);
#endif // endif

	/* Free the buffer block if last slice to go down */
	if (calbuf->cal_cache_buf && !moreslicesup && wlapi_obj_registry_islast(pi->sh->physhim)) {
	    printf("============ [%s][%d] I am here ==============\n", __FUNCTION__, __LINE__);
		phy_mfree(pi, calbuf->cal_cache_buf,
				calbuf->cal_cache_buf_sz);
		calbuf->cal_cache_buf = NULL;
		calbuf->cal_cache_buf_sz = 0;
		calbuf->cal_cache_buf_bmp = 0;
	}
#endif /* PHY_CACHE_PREALLOC */

	return BCME_OK;
}

/* API to set the common buffer size */
void
BCMATTACHFN(phy_cache_register_reuse_size)(phy_cache_info_t *cachei, uint16 size)
{
	ASSERT(cachei != NULL);
	if (cachei->reuse_buffer_size < size) {
		cachei->reuse_buffer_size = size;
	}
}

/* API to acquire common buffer */
void *
phy_cache_acquire_reuse_buffer(phy_cache_info_t *cachei, uint size)
{
	void * ret = NULL;
	if (cachei->reuse_buffer_lock == 0 && size <= cachei->reuse_buffer_size) {
		cachei->reuse_buffer_lock = 1;
		ret = cachei->reuse_buffer;
	} else if (cachei->reuse_buffer_lock != 0) {
		PHY_FATAL_ERROR(cachei->pi, PHY_RC_REUSE_BUFFER_LOCK_ERROR);
	} else {
		PHY_FATAL_ERROR(cachei->pi, PHY_RC_REUSE_BUFFER_SIZE_ERROR);
	}
	return ret;
}

/* API to release common buffer */
void
phy_cache_release_reuse_buffer(phy_cache_info_t *cachei)
{
	if (cachei->reuse_buffer_lock == 1) {
		cachei->reuse_buffer_lock = 0;
		bzero(cachei->reuse_buffer, cachei->reuse_buffer_size);
	} else {
		PHY_FATAL_ERROR(cachei->pi, PHY_RC_REUSE_BUFFER_LOCK_ERROR);
	}
}

#if PHY_CACHE_PREALLOC
uint8 *
phy_cache_acquire_calcachebuf(phy_cache_info_t *cachei)
{
	calcachebuf_param_t *calbuf = cachei->calcachebuf_param;
	uint32 calcachesz = calbuf->cal_cache_buf_sz / PHY_CACHE_SZ;
	uint8 i;

	ASSERT(calbuf->cal_cache_buf);

	for (i = 0; i < PHY_CACHE_SZ; i++) {
		if (!(calbuf->cal_cache_buf_bmp & (1 << i))) {
			/* If available (0), use it */
			break;
		}
	}

	if (i == PHY_CACHE_SZ) {
		/* Nothing available. Can not happen */
		PHY_FATAL_ERROR(cachei->pi,
				PHY_RC_CALCACHEBUFACQ_FAILED);
	}

	/* Mark this buf in use */
	phy_set_calcachebufbmp(cachei, i, TRUE);

	return (calbuf->cal_cache_buf + (i * calcachesz));
}

int
phy_cache_release_calcachebuf(phy_cache_info_t *cachei, uint8 *bufptr)
{
	calcachebuf_param_t *calbuf = cachei->calcachebuf_param;
	uint32 calcachesz =	calbuf->cal_cache_buf_sz / PHY_CACHE_SZ;
	uint8 i;

	if (!calbuf->cal_cache_buf) {
		/* In case the buffer has already been freed (with down), exit */
		return BCME_OK;
	}

	for (i = 0; i < PHY_CACHE_SZ; i++) {
		if (bufptr == (calbuf->cal_cache_buf + (i * calcachesz))) {
			/* Found the cache in the cal cache buffer */
			break;
		}
	}

	if (i == PHY_CACHE_SZ) {
		/* Could not find. Can not happen */
		PHY_FATAL_ERROR(cachei->pi, PHY_RC_CALCACHEBUFREL_FAILED);
	}

	/* Note down the release of the cal cache buf and zero out the cal cache entry */
	phy_set_calcachebufbmp(cachei, i, FALSE);
	bzero(bufptr, calcachesz);

	return BCME_OK;
}
#endif /* PHY_CACHE_PREALLOC */

#ifdef PHYCAL_CACHING
/*
 * Reserve cubby in cache entry and register operation callbacks for the cubby.
 */
int
BCMATTACHFN(phy_cache_reserve_cubby)(phy_cache_info_t *ci, phy_cache_init_fn_t init,
	phy_cache_save_fn_t save, phy_cache_restore_fn_t restore, phy_cache_dump_fn_t dump,
	phy_cache_ctx_t *ctx, uint16 size, phy_cache_cubby_id_t *ccid)
{
	wlc_chctx_client_fn_t fns;
	int clntid;
	int cid;
	int err;

	PHY_TRACE(("%s: size %u\n", "phy_cache_reserve_cubby", size));

	fns.init = init;
	fns.save = save;
	fns.restore = restore;
	fns.dump = dump;

	if ((clntid = wlc_chctx_reg_add_client(ci->chctx, &fns, ctx)) < 0) {
		PHY_ERROR(("%s: wlc_chctx_reg_add_client failed %d\n", __FUNCTION__, clntid));
		err = clntid;
		goto fail;
	}
	if ((cid = wlc_chctx_reg_add_cubby(ci->chctx, clntid, NULL, size, NULL)) < 0) {
		PHY_ERROR(("%s: wlc_chctx_reg_add_cubby failed %d\n", __FUNCTION__, cid));
		err = cid;
		goto fail;
	}

	/* sanity check */
	ASSERT(init != NULL);
	/* ASSERT(save != NULL); Is this reqd? */
	ASSERT(restore != NULL);
	ASSERT(size > 0);

	/* account for the size and round it up to the next pointer */
	ci->bufsz += ROUNDUP(size, sizeof(void *));

	/* use the registry index as the cubby ID */
	*ccid = (phy_cache_cubby_id_t)cid;

	return BCME_OK;

fail:
	return err;
}

/* Find the control struct index (same as cache entry index) for 'chanspec' */
static int
phy_cache_find_ctl(phy_cache_info_t *ci, chanspec_t chanspec)
{
	int ctl;

	/* TODO: change to some faster search when necessary */

	for (ctl = 0; ctl < (int)ci->ctl_sz; ctl ++) {
		if ((ci->ctl[ctl].state & CTL_ST_USED) && (ci->ctl[ctl].chanspec == chanspec)) {
			return ctl;
		}
	}

	return BCME_NOTFOUND;
}

/*
 * Save the client states through the registered 'save' callbacks.
 */
static int
_phy_cache_save(phy_cache_info_t *ci)
{
	wlc_chctx_notif_t notif;

	PHY_TRACE(("_phy_cache_save:\n"));

	notif.event = WLC_CHCTX_LEAVE_CHAN;
	notif.chanspec = 0;

	return wlc_chctx_reg_notif(ci->chctx, &notif);
}

/*
 * Create a cache entry for the 'chanspec' if one doesn't exist.
 */
static int
phy_cache_add_entry(phy_cache_info_t *ci, chanspec_t chanspec)
{
	phy_info_t *pi = ci->pi;
	int ctl;
	int ctxid;
	wlc_chctx_notif_t notif;

	PHY_CAL(("phy_cache_add_entry: chanspec 0x%x, curr 0x%x, num %d\n", chanspec,
		pi->radio_chanspec, pi->phy_calcache_num));

	if ((ctl = phy_cache_find_ctl(ci, chanspec)) >= 0) {
		ASSERT(ci->ctl[ctl].bufp != NULL);
		return BCME_OK;
	}

	/* TODO: change to some faster search when necessary */

	/* find an empty entry */
	for (ctl = 0; ctl < (int)ci->ctl_sz; ctl ++) {
		if (ci->ctl[ctl].state & CTL_ST_USED)
			continue;
		goto init;
	}
	PHY_ERROR(("No cache available\n"));
	ASSERT(0);

	return BCME_NORESOURCE;

init:
	ASSERT(ctl >= 0 && ctl < (int)ci->ctl_sz);

#if PHY_CACHE_PREALLOC
	/* Acquire a Cal cache from the preallocated buffer */
	ci->ctl[ctl].bufp = phy_cache_acquire_calcachebuf(ci);
#else
	/* phy_malloc_fatal() never returns if fail */
	ci->ctl[ctl].bufp = phy_malloc_fatal(pi, ci->bufsz);
#endif // endif

	ASSERT(ci->ctl[ctl].bufp != NULL);

	if ((ctxid =
	     wlc_chctx_reg_add_entry(ci->chctx, chanspec, ci->ctl[ctl].bufp, ci->bufsz)) < 0) {
		PHY_ERROR(("%s: wlc_chctx_reg_add_entry failed\n", __FUNCTION__));
#if PHY_CACHE_PREALLOC
		/* Release the Cal cache into the preallocated buffer */
		phy_cache_release_calcachebuf(ci, ci->ctl[ctl].bufp);
#else
		phy_mfree(pi, ci->ctl[ctl].bufp, ci->bufsz);
#endif // endif
		ci->ctl[ctl].bufp = NULL;
		return ctxid;
	}
	ci->ctl[ctl].ctxid = (uint8)ctxid;
	ci->ctl[ctl].chanspec = chanspec;
	ci->ctl[ctl].state |= CTL_ST_USED;

	pi->phy_calcache_num++; /* Could be moved to cache info */

	notif.event = WLC_CHCTX_OPEN_CHAN;
	notif.chanspec = chanspec;

	wlc_chctx_reg_notif(ci->chctx, &notif);

	if (chanspec == pi->radio_chanspec) {
		/* Take care of condition where cache is created after channel switch */
		phy_cache_restore(ci, chanspec);
	}

	PHY_INFORM(("wl%d: %s ctx %d created for Ch %d\n",
		PI_INSTANCE(pi), __FUNCTION__,
		pi->phy_calcache_num,
		CHSPEC_CHANNEL(chanspec)));

	return BCME_OK;
}

/*
 * Delete the cache entry for 'chanspec' if one exists.
 */
static int
phy_cache_del_entry(phy_cache_info_t *ci, chanspec_t chanspec)
{
	phy_info_t *pi = ci->pi;
	int ctl;
	wlc_chctx_notif_t notif;

	PHY_CAL(("phy_cache_del_entry: chanspec 0x%x, curr 0x%x, num %d\n", chanspec,
		pi->radio_chanspec, pi->phy_calcache_num));

	if ((ctl = phy_cache_find_ctl(ci, chanspec)) < 0) {
		PHY_TRACE(("Cache Not Found for chanspec 0x%x\n", chanspec));
		return ctl;
	}
	ASSERT(ci->ctl[ctl].bufp != NULL);

	if (chanspec == pi->radio_chanspec) {
		_phy_cache_save(ci);
	}

	notif.event = WLC_CHCTX_CLOSE_CHAN;
	notif.chanspec = chanspec;

	wlc_chctx_reg_notif(ci->chctx, &notif);

	wlc_chctx_reg_del_entry(ci->chctx, ci->ctl[ctl].ctxid);

#if PHY_CACHE_PREALLOC
	/* Release the Cal cache into the preallocated buffer */
	phy_cache_release_calcachebuf(ci, ci->ctl[ctl].bufp);
#else
	phy_mfree(pi, ci->ctl[ctl].bufp, ci->bufsz);
#endif // endif

	pi->phy_calcache_num--;

	bzero(&ci->ctl[ctl], sizeof(ci->ctl[ctl]));

	return BCME_OK;
}

/*
 * Set the cache entry associated with 'chanspec' as the current and
 * restore client states through the registered 'restore' callbacks.
 */
static int
phy_cache_restore(phy_cache_info_t *ci, chanspec_t chanspec)
{
	wlc_chctx_notif_t notif;

	PHY_TRACE(("phy_cache_restore: 0x%x Current: 0x%x\n", chanspec,
		ci->pi->radio_chanspec));

	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = chanspec;

	return wlc_chctx_reg_notif(ci->chctx, &notif);
}

/* init cache states */
static void
phy_cache_init_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf)
{
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	phy_info_t *pi = ci->pi;

	phy_calcache_mem_t *cache_mem = (phy_calcache_mem_t *) buf;
	ch_calcache_t *cache = &(cache_mem->phy_calcache);

	PHY_CAL(("phy_cache_init_ctx\n"));

	/* Initialize ram cache pointer */
	cache->u_ram_cache.acphy_ram_calcache = &(cache_mem->ram_cache);

	cache->creation_time = pi->sh->now;
	cache->cal_info.last_cal_temp = -50;
	cache->cal_info.txcal_numcmds = pi->def_cal_info->txcal_numcmds;
	cache->in_use = TRUE;

	pi->cal_info = &(((ch_calcache_t *) buf)->cal_info);
}

static int
phy_cache_save_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf)
{
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	phy_info_t *pi = ci->pi;

	PHY_CAL(("phy_cache_save_ctx\n"));

	pi->cal_info = pi->def_cal_info;
	/* De-initialize cache pointer */

	return BCME_OK;
}

static int
phy_cache_restore_ctx(phy_cache_ctx_t *ctx, void *unused, uint8 *buf)
{
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	phy_info_t *pi = ci->pi;

	PHY_CAL(("phy_cache_restore_ctx: phy_cache_restore_cal\n"));

	/* we're on a channel we don't know about */
	if (buf == NULL) {
		pi->cal_info = pi->def_cal_info;
		return BCME_OK;
	}

	/* Retrieve the multi-phase info from the context */
	pi->cal_info = &(((ch_calcache_t *) buf)->cal_info);

	/* Switched the context so restart a pending MPHASE cal, else clear the state */
	if (phy_cache_restore_cal(pi) == BCME_ERROR) {
		PHY_CAL(("%s cal cache restore on chanspec 0x%x Failed\n",
			__FUNCTION__, pi->radio_chanspec));
	}

	return BCME_OK;
}

/* chspec notification callback */
static int
phy_cache_chspec_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data)
{
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	int status;

	PHY_TRACE(("%s: event = %d chanspec = 0x%x\n", "phy_cache_chspec_notif",
	           data->event, data->new));

	if (wf_chspec_malformed(data->new)) {
		return BCME_BADARG;
	}

	switch (data->event) {
	case PHY_CHANMGR_NOTIF_OPCHCTX_OPEN:
		status = phy_cache_add_entry(ci, data->new);
		break;
	case PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE:
		status = phy_cache_del_entry(ci, data->new);
		break;
	case PHY_CHANMGR_NOTIF_OPCH_CHG:
		status = phy_cache_restore(ci, data->new);
		break;
	default:
		status = BCME_ERROR;
		ASSERT(0);
		break;
	}

	return status;
}

static void
phy_cache_free(phy_cache_info_t *ci)
{
	uint i;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ci == NULL) {
		PHY_INFORM(("%s: null cache module\n", __FUNCTION__));
		return;
	}

	/* free cache entries */
	for (i = 0; i < ci->ctl_sz; i ++) {
		if (ci->ctl[i].state & CTL_ST_USED) {
			phy_cache_del_entry(ci, ci->ctl[i].chanspec);
		}
	}
}
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG)
static int
phy_cache_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	uint i;

	bcm_bprintf(b, "bufsz %u\n", ci->bufsz);

	bcm_bprintf(b, "ctl: sz %u\n", ci->ctl_sz);
	for (i = 0; i < ci->ctl_sz; i ++) {
		bcm_bprintf(b, "  idx %u: state 0x%x chan %u buf %p\n",
		            i, ci->ctl[i].state, ci->ctl[i].chanspec, ci->ctl[i].bufp);
	}

	return wlc_chctx_reg_dump(ci->chctx, b);
}
#endif // endif

#if defined(PHYCAL_CACHING)
int
wlc_phy_cal_cache_init(wlc_phy_t *ppi)
{
	return 0;
}

void
wlc_phy_cal_cache_deinit(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	phy_cache_free(pi->cachei);

	/* No more per-channel contexts, switch in the default one */
	pi->cal_info = pi->def_cal_info;
	ASSERT(pi->cal_info != NULL);
	if (pi->cal_info != NULL) {
		/* Reset the parameters */
		pi->cal_info->last_cal_temp = -50;
		pi->cal_info->last_cal_time = 0;
		pi->cal_info->last_temp_cal_time = 0;
	}
}

void
wlc_phy_cal_cache_set(wlc_phy_t *ppi, bool state)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->phy_calcache_on = state;
	if (!state)
		wlc_phy_cal_cache_deinit(ppi);
}

bool
wlc_phy_cal_cache_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->phy_calcache_on;
}

void
phy_cache_cal(phy_info_t *pi)
{
	phy_type_cache_fns_t * fns = pi->cachei->fns;
	phy_type_cache_ctx_t * cache_ctx = (phy_type_cache_ctx_t *)pi;

	ASSERT(pi != NULL);

	if (fns->cal_cache != NULL) {
		fns->cal_cache(cache_ctx);
	}
}

int
phy_cache_restore_cal(phy_info_t *pi)
{
	phy_type_cache_fns_t *fns = pi->cachei->fns;
	phy_type_cache_ctx_t * cache_ctx = (phy_type_cache_ctx_t *)pi;

	ASSERT(pi != NULL);

	/* TODO: Implement other phy types after ac_phy */
	if (fns->restore != NULL) {
		return fns->restore(cache_ctx);
	}
	else
		return BCME_UNSUPPORTED;
}

#if defined(BCMDBG) || defined(WLTEST)
static void
wlc_phydump_chanctx(phy_info_t *phi, struct bcmstrbuf *b)
{
	phy_cache_info_t * cachei = phi->cachei;
	phy_type_cache_fns_t *fns = cachei->fns;
	phy_type_cache_ctx_t * cache_ctx = (phy_type_cache_ctx_t *)phi;
	int ctl;

	ASSERT(phi != NULL);

	if (phi->HW_FCBS) {
		return;
	}

	bcm_bprintf(b, "\n   Current chanspec: 0x%x\n", phi->radio_chanspec);

	for (ctl = 0; ctl < (int)cachei->ctl_sz; ctl ++) {
		if (cachei->ctl[ctl].state & CTL_ST_USED) {
			if (fns->dump_chanctx != NULL) {
				fns->dump_chanctx(cache_ctx,
					(ch_calcache_t *) cachei->ctl[ctl].bufp, b);
			}
		}
	}
}
#endif /* BCMDBG || WLTEST */
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG) || defined(WLTEST)
static int
wlc_phydump_phycal(void *ctx, struct bcmstrbuf *b)
{
	phy_cache_info_t *cachei = (phy_cache_info_t *)ctx;
	phy_info_t *pi = cachei->pi;
	phy_type_cache_fns_t *fns = cachei->fns;
	phy_type_cache_ctx_t * cache_ctx = (phy_type_cache_ctx_t *)pi;

	ASSERT(pi != NULL);

	if (!pi->sh->up)
		return BCME_NOTUP;

	if (fns->dump_cal != NULL) {
		fns->dump_cal(cache_ctx, b);
	}
	else
		return BCME_UNSUPPORTED;

#if defined(PHYCAL_CACHING) && (defined(BCMDBG) || defined(WLTEST))
	wlc_phydump_chanctx(pi, b);
#endif /* defined(PHYCAL_CACHING) && (defined(BCMDBG) || defined(WLTEST)) */

	return BCME_OK;
}
#endif // endif

#if defined(PHYCAL_CACHING)
int
wlc_phy_invalidate_chanctx(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	int ctl;
	phy_cache_info_t *ci = pi->cachei;

	if ((ctl = phy_cache_find_ctl(ci, chanspec)) >= 0) {
		ch_calcache_t *ctx = (ch_calcache_t *) ci->ctl[ctl].bufp;
		ctx->valid = FALSE;
		ctx->cal_info.last_cal_time = 0;
		ctx->cal_info.last_cal_temp = -50;
		ctx->cal_info.last_temp_cal_time = 0;
	}
	return 0;
}

/*   This function will try and reuse the existing ctx:
	return 0 --> couldn't find any ctx
	return 1 --> channel ctx exist
	return 2 --> grabbed an invalid ctx // No longer valid
*/
int
wlc_phy_reuse_chanctx(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	PHY_CAL(("wlc_phy_reuse_chanctx: wlc_phy_get_chanctx\n"));

	/* Check for existing */
	if (wlc_phy_get_chanctx(pi, chanspec)) {
		PHY_INFORM(("wl%d: %s | using existing chanctx for Ch %d\n",
			PI_INSTANCE(pi), __FUNCTION__,
			CHSPEC_CHANNEL(chanspec)));
		return 1;
	}

	PHY_INFORM(("wl%d: %s | couldn't find any ctx for Ch %d\n",
		PI_INSTANCE(pi), __FUNCTION__,
		CHSPEC_CHANNEL(chanspec)));

	return BCME_OK;
}

void
wlc_phy_update_chctx_glacial_time(wlc_phy_t *ppi, chanspec_t chanspec)
{
	ch_calcache_t *ctx;
	phy_info_t *pi = (phy_info_t *)ppi;

	PHY_CAL(("wlc_phy_update_chctx_glacial_time: wlc_phy_get_chanctx\n"));

	if ((ctx = wlc_phy_get_chanctx((phy_info_t *)ppi, chanspec)))
		ctx->cal_info.last_cal_time = pi->sh->now - pi->sh->glacial_timer;
}

ch_calcache_t *
wlc_phy_get_chanctx(phy_info_t *pi, chanspec_t chanspec)
{
	phy_cache_info_t *ci = pi->cachei;
	int ctl = phy_cache_find_ctl(ci, chanspec);

	PHY_TRACE(("wlc_phy_get_chanctx: 0x%x, Current 0x%x\n",
		chanspec, pi->radio_chanspec));

	if (ctl >= 0) {
		/* We require non-current access for updating glacial timer */
		/* It is Important to note that this cache cubby is reserved first.
		 * The offset 0 is thus used to index into the buffer for calcache.
		 */
		return (ch_calcache_t *) ci->ctl[ctl].bufp;
	} else {
		return NULL;
	}

}

void
wlc_phy_get_all_cached_ctx(wlc_phy_t *ppi, chanspec_t *chanlist)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_cache_info_t *ci = pi->cachei;
	int ctl;
	int i = 0;

	/* TODO: change to some faster search when necessary */

	for (ctl = 0; ctl < (int)ci->ctl_sz; ctl ++) {
		if (ci->ctl[ctl].state & CTL_ST_USED) {
			*(chanlist+i) = ci->ctl[ctl].chanspec;
			i ++;
		}
	}
}

uint32
wlc_phy_get_current_cachedchans(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (pi) {
#if defined(PHYCAL_CACHING)|| defined(WL_MODESW)
		return pi->phy_calcache_num;
#endif // endif
	}
	return 0;
}

#endif /* PHYCAL_CACHING */

#if PHY_CACHE_PREALLOC
static int
phy_set_calcachebufbmp(phy_cache_info_t *ci, uint8 calbufidx, bool set)
{
	if (set) {
		ci->calcachebuf_param->cal_cache_buf_bmp |= (1 << calbufidx);
	} else {
		ci->calcachebuf_param->cal_cache_buf_bmp &= ~(1 << calbufidx);
	}
	return BCME_OK;
}
#endif /* PHY_CACHE_PREALLOC */
