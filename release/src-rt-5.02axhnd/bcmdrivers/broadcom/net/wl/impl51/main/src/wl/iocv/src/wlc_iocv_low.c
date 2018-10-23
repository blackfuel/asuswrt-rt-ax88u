/*
 * IOCV module implementation - ioctl/iovar dispatcher registry
 * For BMAC/PHY.
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
 * $Id: wlc_iocv_low.c 651348 2016-07-26 17:13:07Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>

#include "wlc_iocv_if.h"
#include <wlc_iocv_types.h>
#include <wlc_iocv_low.h>

/* iovar handler entry */
typedef struct {
	wlc_iov_disp_fn_t disp_fn;	/* iovar dispatch callback */
	wlc_iov_disp_fn_t patch_disp_fn;	/* patch iovar dispatch callback */
	void *ctx;		/* context passed to callback */
} wlc_iovt_ent_t;

/* ioctl handler entry */
typedef struct {
	wlc_ioc_disp_fn_t disp_fn;	/* ioctl dispatch callback */
	void *ctx;		/* context passed to callback */
} wlc_ioct_ent_t;

/* module private states */
typedef struct wlc_iocv_low wlc_iocv_low_t;
struct wlc_iocv_low {
	osl_t *osh;

	/* iovar table registry */
	uint16 iovt_cnt;
	uint16 iovt_max;
	wlc_iovt_ent_t *iovt;

	/* ioctl table registry */
	uint16 ioct_cnt;
	uint16 ioct_max;
	wlc_ioct_ent_t *ioct;

	/* next instance */
	wlc_iocv_low_t *next;
};

/* start of the module info chain */
static wlc_iocv_low_t *g_iocv_low = NULL;

/* module private states memory layout - descriptive */
/*
 * typedef struct {
 *	wlc_iocv_low_t low;
 *	wlc_iocv_info_t ii;
 *	wlc_iovt_ent_t iovt[iovt_cnt];
 *	wlc_ioct_ent_t ioct[ioct_cnt];
 * } wlc_iocv_mem_t;
 */
/* helper macros */
#define WLC_IOCV_LOW(ii) ((wlc_iocv_low_t *)(ii)->obj)

/* debug */
#ifdef BCMDBG
#define WL_IOCV_ERR(x) printf x
#else
#define WL_IOCV_ERR(x)
#endif // endif

/* local functions */
static int wlc_iocv_low_reg_iovt(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd);
static int wlc_iocv_low_reg_ioct(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd);

#define IOCV_ALLOC_SIZE(iovt_cnt, ioct_cnt) \
	sizeof(wlc_iocv_low_t) + \
	sizeof(wlc_iocv_info_t) + \
	sizeof(wlc_iovt_ent_t) * (iovt_cnt) + \
	sizeof(wlc_ioct_ent_t) * (ioct_cnt)

/* attach/detach */
wlc_iocv_info_t *
BCMATTACHFN(wlc_iocv_low_attach)(osl_t *osh, uint16 iovt_cnt, uint16 ioct_cnt)
{
	wlc_iocv_low_t *low, *last;
	wlc_iovt_ent_t *iovt;
	wlc_ioct_ent_t *ioct;
	wlc_iocv_info_t *ii;
	uint sz;

	sz = IOCV_ALLOC_SIZE(iovt_cnt, ioct_cnt);

	/* allocate storage */
	if ((low = MALLOCZ(osh, sz)) == NULL) {
		WL_IOCV_ERR(("%s: MALLOC failed\n", __FUNCTION__));
		return NULL;
	}
	low->osh = osh;

	/* init the common info struct */
	ii = (wlc_iocv_info_t *)&low[1];
	ii->iovt_reg_fn = wlc_iocv_low_reg_iovt;
	ii->ioct_reg_fn = wlc_iocv_low_reg_ioct;
	ii->obj = low;

	/* init the iovt registry */
	iovt = (wlc_iovt_ent_t *)&ii[1];
	low->iovt_max = iovt_cnt;
	low->iovt = iovt;

	/* init the ioct registry */
	ioct = (wlc_ioct_ent_t *)&iovt[iovt_cnt];
	low->ioct_max = ioct_cnt;
	low->ioct = ioct;

	/* chian the info together for visibility */
	for (last = g_iocv_low; last != NULL && last->next != NULL; last = last->next) {
		/* nothing to do */
	}
	if (last == NULL) {
		g_iocv_low = low;
	}
	else {
		last->next = low;
	}

	return ii;
}

void
BCMATTACHFN(wlc_iocv_low_detach)(wlc_iocv_info_t *ii)
{
	wlc_iocv_low_t *low;
	uint sz;

	ASSERT(ii != NULL);

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	sz = IOCV_ALLOC_SIZE(low->iovt_max, low->ioct_max);

	MFREE(low->osh, low, sz);

	g_iocv_low = NULL;
}

/* register a single iovar table & callbacks */
static int
BCMATTACHFN(wlc_iocv_low_reg_iovt)(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	if (low->iovt_cnt == low->iovt_max) {
		WL_IOCV_ERR(("%s: no available iovar table registry entry (total %d)\n",
		             __FUNCTION__, low->iovt_max));
		return BCME_NORESOURCE;
	}

	low->iovt[low->iovt_cnt].disp_fn = iovd->disp_fn;
#ifdef WLC_PATCH_IOCTL
	low->iovt[low->iovt_cnt].patch_disp_fn = iovd->patch_disp_fn;
#endif // endif
	low->iovt[low->iovt_cnt].ctx = iovd->ctx;

	low->iovt_cnt ++;

	return BCME_OK;
}

/* register a single ioctl table & callbacks */
static int
BCMATTACHFN(wlc_iocv_low_reg_ioct)(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	if (low->ioct_cnt == low->ioct_max) {
		WL_IOCV_ERR(("%s: no available ioctl table registry entry (total %d)\n",
		             __FUNCTION__, low->ioct_max));
		return BCME_NORESOURCE;
	}

	low->ioct[low->ioct_cnt].disp_fn = iocd->disp_fn;
	low->ioct[low->ioct_cnt].ctx = iocd->ctx;

	low->ioct_cnt ++;

	return BCME_OK;
}

/* dispatch iovar to registered module dispatch callback */
int
wlc_iocv_low_dispatch_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz, struct wlc_if *wlcif)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	ASSERT(tid < low->iovt_cnt);
	ASSERT(low->iovt[tid].disp_fn != NULL);

#ifdef WLC_PATCH_IOCTL
	if (low->iovt[tid].patch_disp_fn) {
		int err = (low->iovt[tid].patch_disp_fn)
		        (low->iovt[tid].ctx, aid, p, p_len, a, a_len, var_sz, wlcif);
		if (err != BCME_IOCTL_PATCH_UNSUPPORTED) {
			return err;
		}
	}
#endif // endif
	return (low->iovt[tid].disp_fn)(low->iovt[tid].ctx, aid, p, p_len, a, a_len, var_sz, wlcif);
}

/* dispatch ioctl to registered module dispatch callback */
int
wlc_iocv_low_dispatch_ioc(wlc_iocv_info_t *ii, uint16 tid, uint32 cid, uint8 *a, uint a_len,
	struct wlc_if *wlcif)
{
	wlc_iocv_low_t *low;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	ASSERT(tid < low->ioct_cnt);
	ASSERT(low->ioct[tid].disp_fn != NULL);

	return (low->ioct[tid].disp_fn)(low->ioct[tid].ctx, cid, a, a_len, wlcif);
}

int
wlc_iocv_low_dump(wlc_iocv_info_t *ii, struct bcmstrbuf *b)
{
	wlc_iocv_low_t *low;
	uint16 tid;

	low = WLC_IOCV_LOW(ii);
	ASSERT(low != NULL);

	bcm_bprintf(b, "IOVAR Dispatchers: cnt %u max %u\n", low->iovt_cnt, low->iovt_max);
	for (tid = 0; tid < low->iovt_cnt; tid ++) {
		bcm_bprintf(b, "    tid %u: disp %p ctx %p\n", tid,
		            low->iovt[tid].disp_fn, low->iovt[tid].ctx);
	}
	bcm_bprintf(b, "IOCTL Dispatchers: cnt %u max %u\n", low->ioct_cnt, low->ioct_max);
	for (tid = 0; tid < low->ioct_cnt; tid ++) {
		bcm_bprintf(b, "    tid %u: disp %p ctx %p\n", tid,
		            low->ioct[tid].disp_fn, low->ioct[tid].ctx);
	}

	return BCME_OK;
}
