/*
 * RadarDetect module implementation
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
 * $Id: phy_radar.c 752845 2018-03-19 07:24:28Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_api.h>
#include <phy_init.h>
#include <phy_radar_api.h>
#include "phy_type_radar.h"
#include "phy_radar_st.h"
#include "phy_radar_shared.h"
#include <phy_radar.h>
#include <phy_ac_info.h>

/* module private states */
struct phy_radar_info {
	phy_info_t *pi;
	phy_radar_st_t *st;		/* states pointer, also as "enable" flag */
	phy_type_radar_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_radar_info_t info;
	phy_type_radar_fns_t fns;
/* add other variable size variables here at the end */
} phy_radar_mem_t;

/* local function declaration */
static int _phy_radar_init(phy_init_ctx_t *ctx);

/* attach/detach */
phy_radar_info_t *
BCMATTACHFN(phy_radar_attach)(phy_info_t *pi)
{
	phy_radar_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_radar_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_radar_mem_t *)info)->fns;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, _phy_radar_init, info, PHY_INIT_RADAR) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	phy_radar_shared_attach(pi);

	return info;

	/* error */
fail:
	phy_radar_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_radar_detach)(phy_radar_info_t *ri)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ri == NULL) {
		PHY_INFORM(("%s: null radar module\n", __FUNCTION__));
		return;
	}

	pi = ri->pi;

	phy_mfree(pi, ri, sizeof(phy_radar_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_radar_register_impl)(phy_radar_info_t *ri, phy_type_radar_fns_t *fns)
{
	phy_info_t *pi = ri->pi;
	int ref_count = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* ===================== */
	/* INIT THE RADAR STATES */
	/* ===================== */

	/* OBJECT REGISTRY: check if shared key has value already stored */
	ri->st = (phy_radar_st_t *)wlapi_obj_registry_get(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO);

	if (ri->st == NULL) {
		if ((ri->st = phy_malloc(pi, sizeof(phy_radar_st_t))) == NULL) {
			PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
			return BCME_NOMEM;
		}

		/* OBJECT REGISTRY: We are the first instance, store value for key */
		wlapi_obj_registry_set(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO, ri->st);

		PHY_INFORM(("wl%d: %s: ptr(ri->st) = %p | sizeof(phy_radar_st_t) = %d\n",
			pi->sh->unit, __FUNCTION__, ri->st, (int)sizeof(phy_radar_st_t)));
	}
	/* OBJECT REGISTRY: Reference the stored value in both instances */
	ref_count = wlapi_obj_registry_ref(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO);
	ASSERT(ref_count <= MAX_RSDB_MAC_NUM);
	BCM_REFERENCE(ref_count);

	*ri->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_radar_unregister_impl)(phy_radar_info_t *ri)
{
	phy_info_t *pi = ri->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ri->st == NULL)
		return;

	if (wlapi_obj_registry_unref(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO) == 0) {
		wlapi_obj_registry_set(pi->sh->physhim, OBJR_PHY_CMN_RADAR_INFO, NULL);
		phy_mfree(pi, ri->st, sizeof(phy_radar_st_t));
		ri->st = NULL;
	}
}

/* return radar state structure to PHY type specific implementation */
phy_radar_st_t *
phy_radar_get_st(phy_radar_info_t *ri)
{
	return ri->st;
}

/* init h/w */
static int
phy_radar_init(phy_radar_info_t *ri, bool init)
{
#ifdef BAND5G
	phy_type_radar_fns_t *fns = ri->fns;
	phy_info_t *pi = ri->pi;
#endif // endif

	PHY_TRACE(("%s: init %d\n", __FUNCTION__, init));

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (fns->init != NULL)
			return (fns->init)(fns->ctx, init);
	}
#endif // endif

	return BCME_OK;
}

static int
WLBANDINITFN(_phy_radar_init)(phy_init_ctx_t *ctx)
{
	phy_radar_info_t *ri = (phy_radar_info_t *)ctx;
	phy_info_t *pi = ri->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return phy_radar_init(ri, pi->sh->radar);
}

/* update h/w */
void
phy_radar_upd(phy_radar_info_t *ri)
{
	phy_type_radar_fns_t *fns = ri->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->update == NULL)
		return;

	(fns->update)(fns->ctx);
}

/* public APIs */
void
phy_radar_detect_enable(phy_info_t *pi, bool enab)
{
	phy_radar_info_t *ri = pi->radari;

	PHY_TRACE(("%s: enable %d\n", __FUNCTION__, enab));

	pi->sh->radar = enab;

	if (!pi->sh->up)
		return;

	phy_radar_init(ri, enab);
}

uint8
phy_radar_detect(phy_info_t *pi, radar_detected_info_t *radar_detected,
bool sec_pll, bool bw80_80_mode)
{
	phy_radar_info_t *ri = pi->radari;
	phy_type_radar_fns_t *fns = ri->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->run == NULL)
		return BCME_OK;

	return (fns->run)(fns->ctx, radar_detected, sec_pll, bw80_80_mode);
}

void
phy_radar_detect_mode_set(phy_info_t *pi, phy_radar_detect_mode_t mode)
{
	phy_radar_info_t *ri = pi->radari;
	phy_type_radar_fns_t *fns = ri->fns;
	phy_radar_st_t *st = ri->st;

	PHY_TRACE(("wl%d: %s, Radar detect mode set done\n", pi->sh->unit, __FUNCTION__));

	if (mode != RADAR_DETECT_MODE_FCC && mode != RADAR_DETECT_MODE_EU &&
		mode != RADAR_DETECT_MODE_UK) {
		PHY_TRACE(("wl%d: bogus radar detect mode: %d\n", pi->sh->unit, mode));
		return;
	}

	if (st == NULL) {
		PHY_ERROR(("%s: not supported\n", __FUNCTION__));
		return;
	}

	if (st->rdm == mode) {
		PHY_TRACE(("wl%d: radar detect mode unchanged: %d\n", pi->sh->unit, mode));
		return;
	}

	st->rdm = mode;

	/* Change radar params based on radar detect mode for
	 * both 20Mhz (index 0) and 40Mhz (index 1) aptly
	 * feature_mask bit-8 is UK-enable
	 * feature_mask bit-11 is FCC-enable
	 * feature_mask bit-12 is EU-enable
	 */
	if (mode == RADAR_DETECT_MODE_FCC) {
		st->rparams.radar_args.feature_mask =
			((st->rparams.radar_args.feature_mask & ~RADAR_FEATURE_ETSI_DETECT) &
			~RADAR_FEATURE_UK_DETECT) |
			RADAR_FEATURE_FCC_DETECT;
	}
	else if (mode == RADAR_DETECT_MODE_UK) {
		st->rparams.radar_args.feature_mask =
			((st->rparams.radar_args.feature_mask & ~RADAR_FEATURE_FCC_DETECT) |
			RADAR_FEATURE_ETSI_DETECT) |
			RADAR_FEATURE_UK_DETECT;
	}
	else if (mode == RADAR_DETECT_MODE_EU) {
		st->rparams.radar_args.feature_mask =
			((st->rparams.radar_args.feature_mask & ~RADAR_FEATURE_FCC_DETECT) &
			~ RADAR_FEATURE_UK_DETECT) |
			RADAR_FEATURE_ETSI_DETECT;
	}

	if (fns->mode == NULL)
		return;

	(fns->mode)(fns->ctx, mode);
}

void
phy_radar_first_indicator_set(phy_info_t *pi)
{
	phy_radar_info_t *ri = pi->radari;
	phy_radar_st_t *st = ri->st;

	if (st == NULL)
		return;

	/* indicate first time radar detection */
	st->first_radar_indicator = 1;
	if (PHY_SUPPORT_SCANCORE(pi) ||
		PHY_SUPPORT_BW80P80(pi)) {
		st->first_radar_indicator_sc = 1;
	}
}

int
phy_radar_set_thresholds(phy_radar_info_t *radari, wl_radar_thr_t *thresholds)
{
	int err = BCME_OK;
	phy_radar_st_t *st = phy_radar_get_st(radari);
	phy_type_radar_fns_t *fns = radari->fns;
	if (thresholds->version != WL_RADAR_THR_VERSION) {
		err = BCME_VERSION;
		goto end;
	}
	st->rparams.radar_thrs.thresh0_20_lo = thresholds->thresh0_20_lo;
	st->rparams.radar_thrs.thresh1_20_lo = thresholds->thresh1_20_lo;
	st->rparams.radar_thrs.thresh0_20_hi = thresholds->thresh0_20_hi;
	st->rparams.radar_thrs.thresh1_20_hi = thresholds->thresh1_20_hi;
	if (fns->set_thresholds != NULL) {
		err = (fns->set_thresholds)(fns->ctx, thresholds);
	}
	phy_radar_detect_enable(radari->pi, radari->pi->sh->radar);
end:
	return err;
}
