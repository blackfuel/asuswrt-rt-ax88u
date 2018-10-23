/*
 * IOCV module implementation - ioctl/iovar table registry.
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
 * $Id: wlc_iocv_high.c 678210 2017-01-07 00:12:49Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>

#include "wlc_iocv_if.h"
#include <wlc_iocv_types.h>
#include <wlc_iocv_fwd.h>
#include <wlc_iocv_high.h>

/* invalid table id */
#define WLC_IOCV_TID_INV ((uint16)(~0))

/* 1. ioctl/iovar registry entry - wlc_io[cv]t_ent_t
 *
 * 'dispatch proc' - callback to process the iovar/ioctl params and return the result.
 *
 * 2. ioctl/iovar registry - io[cv]t_cnt/io[cv]t_max/io[cv]t_wlc/io[cv]t
 *
 * All entries are stored in the registry. The number of available entries is 'io[cv]t_max.
 * The number of used entries is 'io[cv]t_cnt (from index 0 in 'io[cv]t' array).
 *
 * The first N entries (N = 'io[cv]t_wlc') are dispatch proc of BMAC/PHY and
 * ioctl/iovar in the registered tables are forwarded to bmac.
 *
 * The all other entries (from 'io[cv]t_wlc' to 'io[cv]t_cnt' - 1) are
 * 'dispatch proc' of wlc modules.
 */

/* iovar handler entry */
typedef struct {
	/* table */
	const bcm_iovar_t *iovt;
	const bcm_iovar_t *patch_iovt;
	/* dispatch proc */
	wlc_iov_disp_fn_t fn;
	wlc_iov_disp_fn_t patch_fn;
} wlc_iovt_high_ent_t;

/* ioctl handler entry */
typedef struct {
	/* table */
	const wlc_ioctl_cmd_t *ioct;
	uint num_cmds;
	/* handlers */
	wlc_ioc_vld_fn_t vld_fn;	/* ioctl validation callback */
	/* dispatch proc */
	wlc_ioc_disp_fn_t fn;
	wlc_ioc_disp_fn_t ioc_patch_fn;
} wlc_ioct_high_ent_t;

struct wlc_iocv_ctx {
	void *ctx;
};
typedef struct wlc_iocv_ctx wlc_iovt_ent_ctx_t;
typedef struct wlc_iocv_ctx wlc_ioct_ent_ctx_t;

typedef struct wlc_iocv_high_cmn wlc_iocv_high_cmn_t;
struct wlc_iocv_high_cmn {
	wlc_iovt_high_ent_t *iovt;
	wlc_ioct_high_ent_t *ioct;
};

/* module private states */
typedef struct wlc_iocv_high wlc_iocv_high_t;
struct wlc_iocv_high {
	osl_t *osh;

	/* contexts */
	void *high_ctx;		/* for cmd/res/vld interfaces */
	void *low_ctx;		/* for fwd interface in wlc_iocv_fwd.h */

	/* iovar table registry */
	uint16 iovt_cnt;
	uint16 iovt_max;
	uint16 iovt_wlc;	/* the first wlc only table */
	wlc_iovt_ent_ctx_t *iovt_ctx;

	/* ioctl table registry */
	uint16 ioct_cnt;
	uint16 ioct_max;
	uint16 ioct_wlc;		/* the first wlc only table */
	wlc_ioct_ent_ctx_t *ioct_ctx;

	wlc_iocv_high_cmn_t *cmn;

	/* next instance */
	wlc_iocv_high_t *next;
};

/* start of the module info chain */
static wlc_iocv_high_t *g_iocv_high = NULL;

/* module private states memory layout - descriptive */
/*
 * typedef struct {
 *	wlc_iocv_high_t high;
 *	wlc_iocv_info_t ii;
 *	wlc_iovt_high_ent_t iovt[iovt_cnt];
 *	wlc_ioct_high_ent_t ioct[ioct_cnt];
 * } wlc_iocv_mem_t;
 */

/* helper macros */
#define WLC_IOCV_HIGH(ii) ((wlc_iocv_high_t *)(ii)->obj)

/* debug macros */
#ifdef BCMDBG
#define WL_IOCV_ERR(x) printf x
#define WL_IOCV_DBG(x)
#else
#define WL_IOCV_ERR(x)
#define WL_IOCV_DBG(x)
#endif // endif

/* local functions */
static int wlc_iocv_high_reg_iovt(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd);
static int wlc_iocv_high_reg_ioct(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd);

#define IOCV_ALLOC_SIZE(iovt_cnt, ioct_cnt) \
	sizeof(wlc_iocv_high_t) + \
	sizeof(wlc_iocv_info_t) + \
	sizeof(wlc_iovt_ent_ctx_t) * (iovt_cnt) + \
	sizeof(wlc_ioct_ent_ctx_t) * (ioct_cnt)

#define IOCV_CMN_ALLOC_SIZE(iovt_cnt, ioct_cnt) \
		sizeof(wlc_iocv_high_cmn_t) + \
		sizeof(wlc_iovt_high_ent_t) * (iovt_cnt) + \
		sizeof(wlc_ioct_high_ent_t) * (ioct_cnt)

/* attach/detach */
wlc_iocv_info_t *
BCMATTACHFN(wlc_iocv_high_attach)(osl_t *osh, uint16 iovt_cnt, uint16 ioct_cnt,
	void *high_ctx, void *low_ctx)
{
	wlc_iocv_high_t *high, *last;
	wlc_iovt_ent_ctx_t *iovt_ctx;
	wlc_ioct_ent_ctx_t *ioct_ctx;
	wlc_iocv_info_t *ii;
	wlc_iocv_high_cmn_t *cmn;
	uint sz;

	sz = IOCV_ALLOC_SIZE(iovt_cnt, ioct_cnt);
	/* allocate storage */
	if ((high = MALLOCZ(osh, sz)) == NULL) {
		WL_IOCV_ERR(("%s: MALLOC failed\n", __FUNCTION__));
		return NULL;
	}
	high->osh = osh;
	high->high_ctx = high_ctx;
	high->low_ctx = low_ctx;

	/* init the common info struct */
	ii = (wlc_iocv_info_t *)&high[1];
	/* for bmac and phy tables */
	ii->iovt_reg_fn = wlc_iocv_high_reg_iovt;
	ii->ioct_reg_fn = wlc_iocv_high_reg_ioct;
	ii->obj = high;

	/* init the iov registry */
	iovt_ctx = (wlc_iovt_ent_ctx_t *)&ii[1];
	high->iovt_max = iovt_cnt;
	high->iovt_ctx = iovt_ctx;

	/* init the ioc registry */
	ioct_ctx = (wlc_ioct_ent_ctx_t *)&iovt_ctx[iovt_cnt];
	high->ioct_max = ioct_cnt;
	high->ioct_ctx = ioct_ctx;

	sz = IOCV_CMN_ALLOC_SIZE(iovt_cnt, ioct_cnt);
	cmn = wlc_iovc_obj_registry(high_ctx, sz);
	if (cmn == NULL) {
		WL_IOCV_ERR(("%s: MALLOC failed for iovt\n", __FUNCTION__));
		return NULL;
	}
	high->cmn = cmn;
	if (cmn->iovt == NULL) {
		cmn->iovt = (wlc_iovt_high_ent_t *)&cmn[1];
		cmn->ioct = (wlc_ioct_high_ent_t *)&cmn->iovt[iovt_cnt];
	}

	/* chian the info together for visibility */
	for (last = g_iocv_high; last != NULL && last->next != NULL; last = last->next) {
		/* nothing to do */
	}
	if (last == NULL) {
		g_iocv_high = high;
	}
	else {
		last->next = high;
	}

	return ii;
}

void
BCMATTACHFN(wlc_iocv_high_detach)(wlc_iocv_info_t *ii)
{
	wlc_iocv_high_t *high;
	uint sz;

	ASSERT(ii != NULL);

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	sz = IOCV_CMN_ALLOC_SIZE(high->iovt_max, high->ioct_max);
	high->cmn = wlc_iovc_obj_registry_unref(high->high_ctx, high->cmn, sz);
	sz = IOCV_ALLOC_SIZE(high->iovt_max, high->ioct_max);

	MFREE(high->osh, high, sz);

	g_iocv_high = NULL;
}

/* register iovar table (for BMAC and PHY) */
static int
BCMATTACHFN(wlc_iocv_high_reg_iovt)(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->iovt_cnt == high->iovt_max) {
		WL_IOCV_ERR(("%s: too many iovar tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(iovd->iovt != NULL);

	idx = high->iovt_wlc;

	/* move wlc only tables down towards the end of the table */
	if (idx < high->iovt_cnt) {
		memmove(&high->cmn->iovt[idx + 1], &high->cmn->iovt[idx],
		        (high->iovt_cnt - idx) * sizeof(wlc_iovt_high_ent_t));
	}

	if (high->cmn->iovt[idx].iovt == NULL) {
		high->cmn->iovt[idx].iovt = iovd->iovt;
#ifdef WLC_PATCH_IOCTL
		high->cmn->iovt[idx].patch_iovt = iovd->patch_iovt;
#endif /* WLC_PATCH_IOCTL */
	} else if (high->cmn->iovt[idx].iovt != iovd->iovt ||
#ifdef WLC_PATCH_IOCTL
		high->cmn->iovt[idx].patch_iovt != iovd->patch_iovt ||
#endif /* WLC_PATCH_IOCTL */
		FALSE) {
		ASSERT(0);
		return BCME_ERROR;
	}

	high->iovt_wlc ++;
	high->iovt_cnt ++;

	return BCME_OK;
}

/* register ioctl table (for BMAC and PHY) */
static int
BCMATTACHFN(wlc_iocv_high_reg_ioct)(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->ioct_cnt == high->ioct_max) {
		WL_IOCV_ERR(("%s: too many ioctl tables\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(iocd->ioct != NULL);

	idx = high->ioct_wlc;

	/* move wlc only tables down towards the end of the table */
	if (idx < high->ioct_cnt) {
		memmove(&high->cmn->ioct[idx + 1], &high->cmn->ioct[idx],
		        (high->ioct_cnt - idx) * sizeof(wlc_ioct_high_ent_t));
	}

	if (high->cmn->ioct[idx].ioct == NULL) {
		high->cmn->ioct[idx].ioct = iocd->ioct;
		high->cmn->ioct[idx].num_cmds = iocd->num_cmds;
		high->cmn->ioct[idx].vld_fn = iocd->st_vld_fn;
	} else if (high->cmn->ioct[idx].ioct != iocd->ioct ||
		high->cmn->ioct[idx].num_cmds != iocd->num_cmds ||
		high->cmn->ioct[idx].vld_fn != iocd->st_vld_fn) {
		ASSERT(0);
		return BCME_ERROR;
	}

	high->ioct_wlc ++;
	high->ioct_cnt ++;

	return BCME_OK;
}

/* register iovar table (for WLC) */
int
BCMATTACHFN(wlc_iocv_high_register_iovt)(wlc_iocv_info_t *ii,
	const bcm_iovar_t *iovt, wlc_iov_disp_fn_t disp_fn,
#ifdef WLC_PATCH_IOCTL
	const bcm_iovar_t *patch_iovt, wlc_iov_disp_fn_t patch_fn,
#endif // endif
	void *ctx)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->iovt_cnt == high->iovt_max) {
		WL_IOCV_ERR(("%s: no available iovar table registry entry (total %d)\n",
		         __FUNCTION__, high->iovt_max));
		return BCME_NORESOURCE;
	}

	ASSERT(iovt != NULL);
	ASSERT(disp_fn != NULL);

	idx = high->iovt_cnt;

	if (high->cmn->iovt[idx].iovt == NULL) {
		high->cmn->iovt[idx].iovt = iovt;
		high->cmn->iovt[idx].fn = disp_fn;
#ifdef WLC_PATCH_IOCTL
		high->cmn->iovt[idx].patch_iovt = patch_iovt;
		high->cmn->iovt[idx].patch_fn = patch_fn;
#endif /* WLC_PATCH_IOCTL */
	} else if (high->cmn->iovt[idx].iovt != iovt ||
		high->cmn->iovt[idx].fn != disp_fn ||
#ifdef WLC_PATCH_IOCTL
		high->cmn->iovt[idx].patch_iovt != patch_iovt ||
		high->cmn->iovt[idx].patch_fn != patch_fn ||
#endif /* WLC_PATCH_IOCTL */
		FALSE) {
		 ASSERT(0);
		 return BCME_ERROR;
	}
	high->iovt_ctx[idx].ctx = ctx;

	high->iovt_cnt ++;

	return BCME_OK;
}

/* register ioctl table (for WLC) */
int
BCMATTACHFN(wlc_iocv_high_register_ioct)(wlc_iocv_info_t *ii,
	const wlc_ioctl_cmd_t *ioct, uint num_cmds, wlc_ioc_disp_fn_t disp_fn,
#ifdef WLC_PATCH_IOCTL
	wlc_ioc_disp_fn_t ioc_patch_fn,
#endif // endif
	void *ctx)
{
	wlc_iocv_high_t *high;
	uint16 idx;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	if (high->ioct_cnt == high->ioct_max) {
		WL_IOCV_ERR(("%s: no available ioctl table registry entry (total %d)\n",
		         __FUNCTION__, high->ioct_max));
		return BCME_NORESOURCE;
	}

	ASSERT(disp_fn != NULL);

	idx = high->ioct_cnt;

	if (high->cmn->ioct[idx].ioct == NULL) {
		high->cmn->ioct[idx].ioct = ioct;
		high->cmn->ioct[idx].num_cmds = num_cmds;
		high->cmn->ioct[idx].fn = disp_fn;
#ifdef WLC_PATCH_IOCTL
		high->cmn->ioct[idx].ioc_patch_fn = ioc_patch_fn;
#endif /* WLC_PATCH_IOCTL */
	} else if (high->cmn->ioct[idx].ioct != ioct ||
		high->cmn->ioct[idx].num_cmds != num_cmds ||
		high->cmn->ioct[idx].fn != disp_fn ||
#ifdef WLC_PATCH_IOCTL
		high->cmn->ioct[idx].ioc_patch_fn != ioc_patch_fn ||
#endif /* WLC_PATCH_IOCTL */
		FALSE) {
		ASSERT(0);
		return BCME_ERROR;
	}
	high->ioct_ctx[idx].ctx = ctx;

	high->ioct_cnt ++;

	return BCME_OK;
}

/* lookup iovar and return iovar entry and table id if found */
const bcm_iovar_t *
wlc_iocv_high_find_iov(wlc_iocv_info_t *ii, const char *name, uint16 *tid)
{
	wlc_iocv_high_t *high;
	const bcm_iovar_t *vi;
	uint16 i;
#ifdef WLC_PATCH_IOCTL
	bool iovar_removed = FALSE;
	BCM_REFERENCE(iovar_removed);
#endif // endif

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

#ifdef WLC_PATCH_IOCTL
	/* Search given IOVAR name only in patch tables */
	for (i = 0; i < high->iovt_cnt; i ++) {
		/* If an IOVAR is flagged as REMOVED, then keep
		 * searching in case it was actually moved to a different IOVAR table.
		 */
		if (high->cmn->iovt[i].patch_iovt != NULL) {
			if ((vi = bcm_iovar_lookup(high->cmn->iovt[i].patch_iovt, name))) {
				if (vi->flags != IOVF_REMOVED) {
					*tid = i;
					return vi;
				}
				iovar_removed = TRUE;
			}
		}
	}

	/*
	 * Search given IOVAR name which are not patched.
	 * Covers new IOVAR table added after tape-out.
	 */
	for (i = 0; i < high->iovt_cnt; i ++) {
		if ((high->cmn->iovt[i].patch_iovt == NULL) && (high->cmn->iovt[i].iovt != NULL)) {
			if ((vi = bcm_iovar_lookup(high->cmn->iovt[i].iovt, name))) {
				*tid = i;
				return vi;
			}
		}
	}

	/*
	 * Bail if the IOVAR has been removed since ROM tape-out.
	 * or compiled out for specific build targets
	 */
	if (iovar_removed) {
		*tid = WLC_IOCV_TID_INV;
		return NULL;
	}
#endif /* WLC_PATCH_IOCTL */

	/*
	 * Find the given IOVAR name only in ROM tables which are patched
	 */
	for (i = 0; i < high->iovt_cnt; i ++) {
		if (high->cmn->iovt[i].iovt == NULL)
			continue;
#ifdef WLC_PATCH_IOCTL
		/* Skip Non patched ROM tables */
		if (high->cmn->iovt[i].patch_iovt == NULL)
			continue;
#endif // endif
		if ((vi = bcm_iovar_lookup(high->cmn->iovt[i].iovt, name))) {
			/* found entry in the table */
			*tid = i;
			return vi;
		}
	}

	/* If IOVAR is not found */
	*tid = WLC_IOCV_TID_INV;
	return NULL;
}

/* forward iovar to registered module callbacks */
int
wlc_iocv_high_fwd_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid, const bcm_iovar_t *vi,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz, struct wlc_if *wlcif)
{
	wlc_iocv_high_t *high;
	int err;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->iovt_cnt);

	/* forward to BMAC */
	if (tid < high->iovt_wlc) {
		if ((err = wlc_iocv_fwd_disp_iov(high->low_ctx, tid, aid, vi->type,
				p, p_len, a, a_len, var_sz, wlcif)) != BCME_OK) {
			WL_IOCV_DBG(("%s: wlc_iocv_fwd_disp_iov failure, aid %u\n",
			             __FUNCTION__, aid));
			goto exit;
		}
	}
	/* forward to registered dispatch callback */
	else {
#ifdef WLC_PATCH_IOCTL
		if (high->cmn->iovt[tid].patch_fn) {
			if ((err = (high->cmn->iovt[tid].patch_fn)(high->iovt_ctx[tid].ctx, aid,
					p, p_len, a, a_len, var_sz, wlcif)) !=
					BCME_IOCTL_PATCH_UNSUPPORTED) {
				WL_IOCV_DBG(("%s: patch_fn %p failed, aid %u\n",
				         __FUNCTION__, high->cmn->iovt[tid].patch_fn, aid));
				goto exit;
			}
		}
#endif // endif
		if ((err = (high->cmn->iovt[tid].fn)(high->iovt_ctx[tid].ctx, aid,
				p, p_len, a, a_len, var_sz, wlcif)) != BCME_OK) {
			WL_IOCV_DBG(("%s: fn %p failed, aid %u\n",
			             __FUNCTION__, high->cmn->iovt[tid].fn, aid));
			goto exit;
		}
	}

exit:
	return err;
}

/* lookup ioctl and return ioctl entry and table id if found */
const wlc_ioctl_cmd_t *
wlc_iocv_high_find_ioc(wlc_iocv_info_t *ii, uint32 cid, uint16 *tid)
{
	wlc_iocv_high_t *high;
	uint16 i, j;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	for (i = 0; i < high->ioct_cnt; i ++) {
		for (j = 0; j < high->cmn->ioct[i].num_cmds; j++) {
			if (cid == high->cmn->ioct[i].ioct[j].cmd) {
				/* found command 'cid' in the table */
				*tid = i;
				return &high->cmn->ioct[i].ioct[j];
			}
		}
	}

	*tid = WLC_IOCV_TID_INV;
	return NULL;
}

/* forward ioctl to registered module callbacks */
int
wlc_iocv_high_fwd_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	uint8 *a, uint a_len, struct wlc_if *wlcif)
{
	wlc_iocv_high_t *high;
	uint32 cid;
	int err;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->ioct_cnt);

	cid = ci->cmd;

	if ((err = wlc_iocv_high_vld_ioc(ii, tid, ci, a, a_len)) != BCME_OK) {
		WL_IOCV_DBG(("%s: wlc_iocv_high_vld_ioc failed err %d tid %u cid %u\n",
		             __FUNCTION__, err, tid, cid));
		goto exit;
	}

	/* forward to BMAC */
	if (tid < high->ioct_wlc) {
		if ((err = wlc_iocv_fwd_disp_ioc(high->low_ctx, tid, cid, 0,
				a, a_len, wlcif)) != BCME_OK) {
			WL_IOCV_DBG(("%s: wlc_iocv_fwd_disp_ioc failure err %d cid %u\n",
			             __FUNCTION__, err, cid));
			goto exit;
		}
	}
	/* forward to registered dispatch callback */
	else {
#ifdef WLC_PATCH_IOCTL
		if (high->cmn->ioct[tid].ioc_patch_fn) {
			if ((err = (high->cmn->ioct[tid].ioc_patch_fn)(high->ioct_ctx[tid].ctx, cid,
					a, a_len, wlcif)) != BCME_IOCTL_PATCH_UNSUPPORTED) {
				goto exit;
			}
		}
#endif // endif
		if ((err = (high->cmn->ioct[tid].fn)(high->ioct_ctx[tid].ctx, cid,
				a, a_len, wlcif)) != BCME_OK) {
			WL_IOCV_DBG(("%s: fn %p failed err %d cid %u\n",
			             __FUNCTION__, high->cmn->ioct[tid].fn, err, cid));
			goto exit;
		}
	}

exit:
	return err;
}

/* dispatch ioctl that doesn't have a table registered */
int
wlc_iocv_high_proc_ioc(wlc_iocv_info_t *ii, uint32 cid,
	uint8 *a, uint a_len, struct wlc_if *wlcif)
{
	wlc_iocv_high_t *high;
	uint16 i;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	for (i = high->ioct_wlc; i < high->ioct_cnt; i ++) {
		int err;

		/* skip the entry with a table */
		if (high->cmn->ioct[i].num_cmds > 0) {
			continue;
		}

#ifdef WLC_PATCH_IOCTL
		if (high->cmn->ioct[i].ioc_patch_fn) {
			if ((err = (high->cmn->ioct[i].ioc_patch_fn)(high->ioct_ctx[i].ctx, cid,
					a, a_len, wlcif)) != BCME_IOCTL_PATCH_UNSUPPORTED) {
				return err;
			}
		}
#endif // endif
		if ((err = (high->cmn->ioct[i].fn)(high->ioct_ctx[i].ctx, cid,
				a, a_len, wlcif)) == BCME_UNSUPPORTED) {
			continue;
		}
		return err;
	}

	return BCME_UNSUPPORTED;
}

/**
 * Get a block of IO variable information including the name, type and module
 * Data is returned as triples (module-idx, type, string);
 *     module-idx is a 1 bytes, internal index, consistent with the information returned
 *         by wlc_iovar_modules_get.
 *     type is one byte type, one of the IOVT_ values in bcmutils.h
 *     the name is null terminated, up to a max length of the param name_bytes.
 * Params:
 *     name_bytes -- size of name buffer
 *     var_start -- Which variable index to start with in transfer
 *     out_max -- Max number of variables to transfer
 *     buf -- output buffer; held name_bytes, var_start and out_max
 *     len -- number of bytes available in buf
 */
int
wlc_iocv_high_iov_names(wlc_iocv_info_t *ii, uint name_bytes, uint var_start, uint out_max,
	uint8 *buf, uint len)
{
	wlc_iocv_high_t *high;
	uint16 i, iov_mod_idx = 0;
	const bcm_iovar_t *var = NULL;
	char *out_p;
	uint skip_count = 0;   /* Variables skipped to get to start */
	uint out_count = 0;    /* Variables transfered */
	uint iovar_bank;
	uint max_iovar_bank = 1;

#ifdef WLC_PATCH_IOCTL
	max_iovar_bank = 2;
#endif // endif

	/* Check there's enough space in the output buffer */
	if (len < out_max * (name_bytes + 2)) {
		return BCME_ERROR;
	}
	out_p = (char *)buf;

	for (high = g_iocv_high; high != NULL; high = high->next) {
		for (i = 0; i < high->iovt_cnt && out_count < out_max; i ++) {
			for (iovar_bank = 0; iovar_bank < max_iovar_bank; iovar_bank++) {
				if (iovar_bank == 0) {
					var = high->cmn->iovt[i].iovt;
				}
#ifdef WLC_PATCH_IOCTL
				else {
					var = high->cmn->iovt[i].patch_iovt;
				}
#endif // endif
				for (; (var != NULL) && (var->name != NULL); var ++) {
					/* Skip first "var_start" variables */
					if (skip_count++ < var_start) {
						continue;
					}
					/* Get module */
					*out_p++ = (char)iov_mod_idx;
					/* Get variable type */
					*out_p++ = (char)(var->type);
					/* Get variable name */
					strncpy(out_p, var->name, name_bytes);
					/* In case var->name too long */
					out_p[name_bytes - 1] = '\0';
					/* Advance to next place in output buffer */
					out_p += name_bytes;
					/* All done? */
					if (++out_count >= out_max) {
						break;
					}
				}
			}
			iov_mod_idx++;
		}
	}

	/* Set empty name if fewer than out_max variables returned */
	if (out_count < out_max) {
		out_p += 2;
		*out_p = '\0';
	}

	return BCME_OK;
}

/* validate ioctl */
int
wlc_iocv_high_vld_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	void *a, uint a_len)
{
	wlc_iocv_high_t *high;
	uint32 cid;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	ASSERT(tid < high->ioct_cnt);

	if (high->cmn->ioct[tid].vld_fn == NULL)
		return BCME_OK;

	cid = ci->cmd;

	return (high->cmn->ioct[tid].vld_fn)(high->high_ctx, cid, a, a_len);
}

int
wlc_iocv_high_dump(wlc_iocv_info_t *ii, struct bcmstrbuf *b)
{
	wlc_iocv_high_t *high;
	uint16 tid;

	high = WLC_IOCV_HIGH(ii);
	ASSERT(high != NULL);

	(void)wlc_iocv_fwd_dump(high->low_ctx, b);

	bcm_bprintf(b, "IOVAR Tables: cnt %u max %u\n", high->iovt_cnt, high->iovt_max);
	bcm_bprintf(b, "  Entries from LOW: cnt %u\n", high->iovt_wlc);
	for (tid = 0; tid < high->iovt_wlc; tid ++) {
		bcm_bprintf(b, "    tid %u: tbl %p", tid, high->cmn->iovt[tid].iovt);
		if (high->cmn->iovt[tid].iovt[0].name != NULL)
			bcm_bprintf(b, " (%s...)", high->cmn->iovt[tid].iovt[0].name);
		bcm_bprintf(b, "\n");
	}
	bcm_bprintf(b, "  Entries from HIGH: cnt %u\n", high->iovt_cnt - high->iovt_wlc);
	for (; tid < high->iovt_cnt; tid ++) {
		bcm_bprintf(b, "    tid %u: tbl %p", tid, high->cmn->iovt[tid].iovt);
		if (high->cmn->iovt[tid].iovt[0].name != NULL)
			bcm_bprintf(b, " (%s...)", high->cmn->iovt[tid].iovt[0].name);
		bcm_bprintf(b, " disp %p ctx %p\n",
		            high->cmn->iovt[tid].fn,
		            high->iovt_ctx[tid].ctx);
	}

	bcm_bprintf(b, "IOCTL Tables: cnt %u max %u\n", high->ioct_cnt, high->ioct_max);
	bcm_bprintf(b, "  Entries from LOW: cnt %u\n", high->ioct_wlc);
	for (tid = 0; tid < high->ioct_wlc; tid ++) {
		bcm_bprintf(b, "    tid %u: tbl %p cnt %u",
		            tid, high->cmn->ioct[tid].ioct, high->cmn->ioct[tid].num_cmds);
		if (high->cmn->ioct[tid].ioct != NULL)
			bcm_bprintf(b, " (%u...)", high->cmn->ioct[tid].ioct[0].cmd);
		bcm_bprintf(b, " vld %p", high->cmn->ioct[tid].vld_fn);
		bcm_bprintf(b, "\n");
	}
	bcm_bprintf(b, "  Entries from HIGH: cnt %u\n", high->ioct_cnt - high->ioct_wlc);
	for (; tid < high->ioct_cnt; tid ++) {
		bcm_bprintf(b, "    tid %u: tbl %p cnt %u",
		            tid, high->cmn->ioct[tid].ioct, high->cmn->ioct[tid].num_cmds);
		if (high->cmn->ioct[tid].ioct != NULL)
			bcm_bprintf(b, " (%u...)", high->cmn->ioct[tid].ioct[0].cmd);
		bcm_bprintf(b, " vld %p", high->cmn->ioct[tid].vld_fn);
		bcm_bprintf(b, " disp %p ctx %p\n",
		            high->cmn->ioct[tid].fn,
		            high->ioct_ctx[tid].ctx);
	}

	return BCME_OK;
}
