/*
 * MBO+OCE IE management implementation for
 * Broadcom 802.11bang Networking Device Driver
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
 * $Id$
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 */

/**
 * @file
 * @brief
 * This file co-ordinates WFA MBO OCE IE management
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <wl_dbg.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>

#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_ie_mgmt_types.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_vs.h>
#include <mbo_oce.h>
#include <wlc_mbo_oce.h>
#include "wlc_mbo_oce_priv.h"
#include <bcmiov.h>
#include <wlc_ie_mgmt_ft.h>

#ifndef MBO_OCE_MAX_BUILD_CBS
	#define MBO_OCE_MAX_BUILD_CBS 8
#endif /* MBO_OCE_MAX_BUILD_CBS */
#ifndef MBO_OCE_MAX_PARSE_CBS
	#define MBO_OCE_MAX_PARSE_CBS 8
#endif /* MBO_OCE_MAX_PARSE_CBS */

uint16 ie_build_fstbmp = FT2BMP(FC_ASSOC_REQ) |
	FT2BMP(FC_REASSOC_REQ) |
	FT2BMP(FC_ASSOC_RESP) |
	FT2BMP(FC_REASSOC_RESP) |
	FT2BMP(FC_PROBE_RESP) |
#ifdef WL_OCE_AP
	FT2BMP(FC_BEACON) |
#endif // endif
	FT2BMP(FC_PROBE_REQ);
uint16 ie_parse_fstbmp = FT2BMP(FC_BEACON) |
	FT2BMP(FC_PROBE_RESP) |
	FT2BMP(FC_ASSOC_RESP) |
	FT2BMP(FC_REASSOC_RESP) |
#ifdef WL_OCE_AP
	FT2BMP(FC_ASSOC_REQ) |
	FT2BMP(FC_REASSOC_REQ) |
#endif // endif
	FT2BMP(WLC_IEM_FC_SCAN_BCN) |
	FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);

typedef struct wlc_mbo_oce_ie_build_entry {
	void *ctx;
	uint16 fstbmp;
	wlc_mbo_oce_attr_build_fn_t build_fn;
} wlc_mbo_oce_ie_build_entry_t;

typedef struct wlc_mbo_oce_ie_parse_entry {
	void *ctx;
	uint16 fstbmp;
	wlc_mbo_oce_attr_parse_fn_t parse_fn;
} wlc_mbo_oce_ie_parse_entry_t;

typedef struct wlc_mbo_oce_data wlc_mbo_oce_data_t;
struct wlc_mbo_oce_data {
};

struct wlc_mbo_oce_info {
	osl_t    *osh;
	wlc_cmn_info_t *cmn;
	wlc_pub_t *pub;  /* update before calling module detach */
	wlc_pub_cmn_t *pub_cmn;
	wlc_obj_registry_t *objr;
	uint16 ie_build_fstbmp;
	uint16 ie_parse_fstbmp;
	uint8 max_build_cbs;
	uint8 count_build_cbs;
	uint8 max_parse_cbs;
	uint8 count_parse_cbs;
	wlc_mbo_oce_ie_build_entry_t *build_cbs;
	wlc_mbo_oce_ie_parse_entry_t *parse_cbs;
};

static const bcm_iovar_t mbo_oce_iovars[] = {
	{NULL, 0, 0, 0, 0, 0}
};

static void wlc_mbo_oce_watchdog(void *ctx);
static int wlc_mbo_oce_wlc_up(void *ctx);
static int wlc_mbo_oce_wlc_down(void *ctx);
static int wlc_mbo_oce_ie_build_fn(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_mbo_oce_ie_calc_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_mbo_oce_ie_parse_fn(void *ctx, wlc_iem_parse_data_t *data);

wlc_mbo_oce_info_t *
BCMATTACHFN(wlc_mbo_oce_attach)(wlc_info_t *wlc)
{
	wlc_mbo_oce_info_t *mbo_oce = NULL;
	int ret = BCME_OK;
	uint16 alloc_size = 0;

	mbo_oce = obj_registry_get(wlc->objr, OBJR_MBO_OCE_INFO);
	if (mbo_oce == NULL) {
		mbo_oce = (wlc_mbo_oce_info_t *)MALLOCZ(wlc->osh, sizeof(*mbo_oce));
		if (mbo_oce == NULL) {
			WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
				wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_MBO_OCE_INFO, mbo_oce);
		mbo_oce->cmn = wlc->cmn;
		mbo_oce->osh = wlc->osh;
		mbo_oce->objr = wlc->objr;
		mbo_oce->pub = wlc->pub;
		mbo_oce->pub_cmn = wlc->pub->cmn;
		mbo_oce->ie_build_fstbmp = ie_build_fstbmp;
		mbo_oce->ie_parse_fstbmp = ie_parse_fstbmp;
		mbo_oce->max_build_cbs = MBO_OCE_MAX_BUILD_CBS;
		mbo_oce->max_parse_cbs = MBO_OCE_MAX_PARSE_CBS;

		alloc_size = mbo_oce->max_build_cbs * sizeof(*mbo_oce->build_cbs);
		mbo_oce->build_cbs = MALLOCZ(wlc->osh, alloc_size);
		if (mbo_oce->build_cbs == NULL) {
			WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
				wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
			goto fail;
		}

		alloc_size = mbo_oce->max_parse_cbs * sizeof(*mbo_oce->parse_cbs);
		mbo_oce->parse_cbs = MALLOCZ(wlc->osh, alloc_size);
		if (mbo_oce->parse_cbs == NULL) {
			WL_ERROR(("wl%d: %s:out of mem. alloced %u bytes\n",
				wlc->pub->unit, __FUNCTION__,  MALLOCED(wlc->osh)));
			goto fail;
		}
	}
	(void)obj_registry_ref(wlc->objr, OBJR_MBO_OCE_INFO);
	/* register module */
	ret = wlc_module_register(wlc->pub, mbo_oce_iovars, "MBO_OCE", mbo_oce,
		NULL, wlc_mbo_oce_watchdog,
		wlc_mbo_oce_wlc_up, wlc_mbo_oce_wlc_down);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register MBO-OCE IE build callback fn */
	ret = wlc_iem_vs_add_build_fn_mft(wlc->iemi, mbo_oce->ie_build_fstbmp,
		WLC_IEM_VS_IE_PRIO_MBO_OCE, wlc_mbo_oce_ie_calc_len,
		wlc_mbo_oce_ie_build_fn, mbo_oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_build_fn_mft failed %d, \n",
			wlc->pub->unit, __FUNCTION__, ret));
		goto fail;
	}

	/* register MBO-OCE IE parse callback fn */
	ret = wlc_iem_vs_add_parse_fn_mft(wlc->iemi, mbo_oce->ie_parse_fstbmp,
		WLC_IEM_VS_IE_PRIO_MBO_OCE, wlc_mbo_oce_ie_parse_fn, mbo_oce);
	if (ret != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vs_add_parse_fn failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	wlc->pub->cmn->_mbo_oce = TRUE;
	return mbo_oce;
fail:
	MODULE_DETACH(mbo_oce, wlc_mbo_oce_detach);
	return NULL;
}

void
BCMATTACHFN(wlc_mbo_oce_detach)(wlc_mbo_oce_info_t* mbo_oce)
{
	uint16 alloc_size = 0;
	if (mbo_oce && obj_registry_unref(mbo_oce->objr, OBJR_MBO_OCE_INFO) == 0) {
		obj_registry_set(mbo_oce->objr, OBJR_MBO_OCE_INFO, NULL);
		mbo_oce->pub_cmn->_mbo_oce = FALSE;
		if (mbo_oce->build_cbs) {
			alloc_size = mbo_oce->max_build_cbs * sizeof(*mbo_oce->build_cbs);
			MFREE(mbo_oce->osh, mbo_oce->build_cbs, alloc_size);
		}
		if (mbo_oce->parse_cbs) {
			alloc_size = mbo_oce->max_parse_cbs * sizeof(*mbo_oce->parse_cbs);
			MFREE(mbo_oce->osh, mbo_oce->parse_cbs, alloc_size);
		}
		wlc_module_unregister(mbo_oce->pub, "MBO_OCE", mbo_oce);
		MFREE(mbo_oce->osh, mbo_oce, sizeof(*mbo_oce));
		mbo_oce = NULL;
	}
}

static void
wlc_mbo_oce_watchdog(void *ctx)
{

}

static int
wlc_mbo_oce_wlc_up(void *ctx)
{
	return BCME_OK;
}

static int
wlc_mbo_oce_wlc_down(void *ctx)
{
	return BCME_OK;
}

static int
wlc_mbo_oce_ie_build_fn(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_mbo_oce_info_t *mbo_oce = (wlc_mbo_oce_info_t *)ctx;
	uint8 *cp = NULL;
	wifi_mbo_oce_ie_t *ie_hdr = NULL;
	uint total_len = 0; /* len to be put in IE header */
	int len = 0;
	wlc_mbo_oce_ie_build_entry_t *entry = NULL;
	wlc_mbo_oce_attr_build_data_t attr_data;
	int i = 0;

	ASSERT(mbo_oce != NULL);
	ASSERT(data != NULL);
	/* fill local build data */
	memset(&attr_data, 0, sizeof(attr_data));
	attr_data.cbparm = data->cbparm;
	attr_data.cfg = data->cfg;
	attr_data.ft = data->ft;
	attr_data.tag = data->tag;
	attr_data.buf = data->buf;
	attr_data.buf_len = data->buf_len;

	if (mbo_oce->build_cbs && mbo_oce->count_build_cbs) {
		cp = attr_data.buf;
		ie_hdr = (wifi_mbo_oce_ie_t *)cp;

		/* fill in MBO-OCE IE header */
		ie_hdr->id = MBO_OCE_IE_ID;
		memcpy(ie_hdr->oui, MBO_OCE_OUI, WFA_OUI_LEN);
		ie_hdr->oui_type = MBO_OCE_OUI_TYPE;
		len = MBO_OCE_IE_HDR_SIZE;
		cp += len;
		total_len = MBO_OCE_IE_NO_ATTR_LEN;

		attr_data.buf_len -= len;
		attr_data.buf = cp;
		for (i = 0; i < mbo_oce->max_build_cbs; i++) {
			entry = &mbo_oce->build_cbs[i];
			if ((entry->fstbmp & FT2BMP(data->ft)) && entry->build_fn) {
				len = entry->build_fn(entry->ctx, &attr_data);
				if (len < 0) {
					return BCME_ERROR;
				}
				cp += len;
				attr_data.buf = cp;
				attr_data.buf_len -= len;
				total_len += len;
			}
		}
		/* update MBO IE len */
		ie_hdr->len = total_len;
	}

	return BCME_OK;
}

static uint
wlc_mbo_oce_ie_calc_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_mbo_oce_info_t *mbo_oce = (wlc_mbo_oce_info_t *)ctx;
	wlc_mbo_oce_ie_build_entry_t *entry = NULL;
	wlc_mbo_oce_attr_build_data_t attr_data;
	uint total_len = 0;
	int i = 0;

	ASSERT(mbo_oce != NULL);
	ASSERT(data != NULL);

	/* fill local build data */
	memset(&attr_data, 0, sizeof(attr_data));
	attr_data.cbparm = data->cbparm;
	attr_data.cfg = data->cfg;
	attr_data.ft = data->ft;
	attr_data.tag = data->tag;
	attr_data.buf = NULL;
	attr_data.buf_len = 0;

	if (mbo_oce->build_cbs && mbo_oce->count_build_cbs) {
		total_len = MBO_OCE_IE_HDR_SIZE;
		for (i = 0; i < mbo_oce->max_build_cbs; i++) {
			entry = &mbo_oce->build_cbs[i];
			if ((entry->fstbmp & FT2BMP(data->ft)) && entry->build_fn) {
				total_len += entry->build_fn(entry->ctx, &attr_data);
			}
		}
	}
	return total_len;
}

static int
wlc_mbo_oce_ie_parse_fn(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_mbo_oce_info_t *mbo_oce = (wlc_mbo_oce_info_t *)ctx;
	wlc_mbo_oce_ie_parse_entry_t *entry = NULL;
	wlc_mbo_oce_attr_parse_data_t attr_data;
	int ret = BCME_OK;
	int i = 0;

	/* validate minimum IE length */
	if (data->ie_len <= MBO_OCE_IE_HDR_SIZE) {
		return BCME_OK;
	}
	ASSERT(data->ie);

	/* fill local parse data */
	memset(&attr_data, 0, sizeof(attr_data));
	attr_data.pparm = data->pparm;
	attr_data.cfg = data->cfg;
	attr_data.ft = data->ft;
	attr_data.ie = data->ie + MBO_OCE_IE_HDR_SIZE;
	attr_data.ie_len = data->ie_len - MBO_OCE_IE_HDR_SIZE;
	if (mbo_oce->parse_cbs && mbo_oce->count_parse_cbs) {
		for (i = 0; i < mbo_oce->max_parse_cbs; i++) {
			entry = &mbo_oce->parse_cbs[i];
			if (entry->fstbmp & FT2BMP(data->ft) && entry->parse_fn) {
				ret = entry->parse_fn(entry->ctx, &attr_data);
				if (ret != BCME_OK) {
					return ret;
				}
			}
		}
	}
	return BCME_OK;
}

wlc_mbo_oce_ie_build_hndl_t
wlc_mbo_oce_register_ie_build_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_build_data_t *build_data)
{
	wlc_mbo_oce_ie_build_entry_t *entry = NULL;
	int i = 0;

	ASSERT(mbo_oce);

	if (mbo_oce->count_build_cbs >= mbo_oce->max_build_cbs) {
		return NULL;
	}
	for (i = 0; i < mbo_oce->max_build_cbs; i++) {
		entry = &mbo_oce->build_cbs[i];
		if (!entry->ctx && !entry->build_fn) {
			/* fill in data */
			entry->ctx = build_data->ctx;
			entry->fstbmp = build_data->fstbmp;
			entry->build_fn = build_data->build_fn;
			mbo_oce->count_build_cbs++;
			break;
		}
	}
	return (wlc_mbo_oce_ie_build_hndl_t)entry;
}

int
wlc_mbo_oce_unregister_ie_build_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_build_hndl_t hndl)
{
	wlc_mbo_oce_ie_build_entry_t *entry = NULL;
	int i = 0;

	for (i = 0; i < mbo_oce->max_build_cbs; i++) {
		entry = &mbo_oce->build_cbs[i];
		if (entry == hndl) {
			memset(entry, 0, sizeof(*entry));
			mbo_oce->count_build_cbs--;
			return BCME_OK;
		}
	}
	hndl = NULL;
	return BCME_NOTFOUND;
}

wlc_mbo_oce_ie_parse_hndl_t
wlc_mbo_oce_register_ie_parse_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_parse_data_t *parse_data)
{
	wlc_mbo_oce_ie_parse_entry_t *entry = NULL;
	int i = 0;

	ASSERT(mbo_oce);

	if (mbo_oce->count_parse_cbs >= mbo_oce->max_parse_cbs) {
		return NULL;
	}
	for (i = 0; i < mbo_oce->max_parse_cbs; i++) {
		entry = &mbo_oce->parse_cbs[i];
		if (!entry->ctx && !entry->parse_fn) {
			/* fill in data */
			entry->ctx = parse_data->ctx;
			entry->fstbmp = parse_data->fstbmp;
			entry->parse_fn = parse_data->parse_fn;
			mbo_oce->count_parse_cbs++;
			break;
		}
	}
	return (wlc_mbo_oce_ie_parse_hndl_t)entry;
}

int
wlc_mbo_oce_unregister_ie_parse_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_parse_hndl_t hndl)
{
	wlc_mbo_oce_ie_parse_entry_t *entry = NULL;
	int i = 0;

	for (i = 0; i < mbo_oce->max_parse_cbs; i++) {
		entry = &mbo_oce->parse_cbs[i];
		if (entry == hndl) {
			memset(entry, 0, sizeof(*entry));
			mbo_oce->count_parse_cbs--;
			return BCME_OK;
		}
	}
	hndl = NULL;
	return BCME_NOTFOUND;
}
