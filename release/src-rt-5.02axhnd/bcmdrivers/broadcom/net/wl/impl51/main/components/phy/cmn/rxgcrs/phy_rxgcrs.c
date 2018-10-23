/*
 * Rx Gain Control and Carrier Sense module implementation.
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
 * $Id: phy_rxgcrs.c 689894 2017-03-13 23:32:18Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_utils_reg.h>
#include <phy_rxgcrs_api.h>
#include "phy_type_rxgcrs.h"
#include <phy_rstr.h>
#include <phy_rxgcrs.h>
#include <phy_stf.h>

/* forward declaration */
typedef struct phy_rxgcrs_mem phy_rxgcrs_mem_t;

/* module private states */
struct phy_rxgcrs_info {
	phy_info_t 				*pi;	/* PHY info ptr */
	phy_type_rxgcrs_fns_t	*fns;	/* PHY specific function ptrs */
	phy_rxgcrs_mem_t			*mem;	/* Memory layout ptr */
	uint8 region_group;
};

/* module private states memory layout */
struct phy_rxgcrs_mem {
	phy_rxgcrs_info_t		cmn_info;
	phy_type_rxgcrs_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */
static int phy_rxgcrs_init(phy_init_ctx_t *ctx);

/* attach/detach */
phy_rxgcrs_info_t *
BCMATTACHFN(phy_rxgcrs_attach)(phy_info_t *pi)
{
	phy_rxgcrs_mem_t	*mem = NULL;
	phy_rxgcrs_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_rxgcrs_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize rxgcrs params */
	cmn_info->region_group = REGION_OTHER;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_rxgcrs_init, cmn_info, PHY_INIT_RXG) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG)
	phy_dbg_add_dump_fn(pi, "phychanest", phy_rxgcrs_dump_chanest, cmn_info);
#if defined(DBG_BCN_LOSS)
	phy_dbg_add_dump_fn(pi, "phycalrxmin", phy_rxgcrs_dump_phycal_rx_min, cmn_info);
#endif // endif
#endif // endif

#if defined(WLTEST)
	phy_dbg_add_dump_fn(pi, "phych4rpcal", phy_rxgcrs_dump_ch4rpcal, cmn_info);
#endif /* WLTEST */

	return cmn_info;

	/* error */
fail:
	phy_rxgcrs_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_rxgcrs_detach)(phy_rxgcrs_info_t *cmn_info)
{
	phy_rxgcrs_mem_t	*mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info)
		return;

	/* Freeup memory associated with cmn_info. */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null rxgcrs module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_rxgcrs_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_rxgcrs_register_impl)(phy_rxgcrs_info_t *mi, phy_type_rxgcrs_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*mi->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_rxgcrs_unregister_impl)(phy_rxgcrs_info_t *mi)
{
	BCM_REFERENCE(mi);
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* Rx Gain Control Carrier Sense init */
static int
WLBANDINITFN(phy_rxgcrs_init)(phy_init_ctx_t *ctx)
{
	phy_rxgcrs_info_t *rxgcrsi = (phy_rxgcrs_info_t *)ctx;
	phy_type_rxgcrs_fns_t *fns = rxgcrsi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->init_rxgcrs != NULL) {
		return (fns->init_rxgcrs)(fns->ctx);
	}
	return BCME_OK;
}

/* driver down processing */
int
phy_rxgcrs_down(phy_rxgcrs_info_t *mi)
{
	int callbacks = 0;
	BCM_REFERENCE(mi);
	PHY_TRACE(("%s\n", __FUNCTION__));

	return callbacks;
}

/* When multiple PHY supported, then need to walk thru all instances of pi */
void
wlc_phy_set_locale(phy_info_t *pi, uint8 region_group)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	info->region_group = region_group;

	if (fns->set_locale != NULL)
		(fns->set_locale)(fns->ctx, region_group);
}

uint8
wlc_phy_get_locale(phy_rxgcrs_info_t *info)
{
	return info->region_group;
}

int
wlc_phy_adjust_ed_thres(phy_info_t *pi, int32 *assert_thresh_dbm, bool set_threshold)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (NULL != fns->adjust_ed_thres)
	{
		fns->adjust_ed_thres(fns->ctx, assert_thresh_dbm, set_threshold);
		return BCME_OK;
	}

	return BCME_UNSUPPORTED;
}

/* Rx desense Module */
#if defined(RXDESENS_EN)
int
phy_rxgcrs_get_rxdesens(phy_info_t *pi, int32 *ret_int_ptr)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->get_rxdesens != NULL) {
		return (fns->get_rxdesens)(fns->ctx, ret_int_ptr);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_rxgcrs_set_rxdesens(phy_info_t *pi, int32 int_val)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->set_rxdesens != NULL) {
		return (fns->set_rxdesens)(fns->ctx, int_val);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* RXDESENS_EN */

int
wlc_phy_set_rx_gainindex(phy_info_t *pi, int32 gain_idx)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	if (!pi->sh->up) {
		return BCME_NOTUP;
	}

	PHY_INFORM(("wlc_phy_set_rx_gainindex Called\n"));
	if (fns->set_rxgainindex != NULL) {
		return (fns->set_rxgainindex)(fns->ctx, gain_idx);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
wlc_phy_get_rx_gainindex(phy_info_t *pi, int32 *gain_idx)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	if (!pi->sh->up) {
		return BCME_NOTUP;
	}

	PHY_INFORM(("wlc_phy_get_rx_gainindex Called\n"));
	if (fns->get_rxgainindex != NULL) {
		return (fns->get_rxgainindex)(fns->ctx, gain_idx);
	} else {
		return BCME_UNSUPPORTED;
	}
}

void
phy_rxgcrs_stay_in_carriersearch(void *ctx, bool enable)
{
	phy_rxgcrs_info_t *rxgcrsi = (phy_rxgcrs_info_t *)ctx;
	phy_type_rxgcrs_fns_t *fns = rxgcrsi->fns;

	if (fns->stay_in_carriersearch != NULL)
		(fns->stay_in_carriersearch)(fns->ctx, enable);
}

#if defined(BCMDBG)
int
phy_rxgcrs_dump_chanest(void *ctx, struct bcmstrbuf *b)
{
	phy_rxgcrs_info_t *rxgcrsi = (phy_rxgcrs_info_t *)ctx;
	phy_type_rxgcrs_fns_t *fns = rxgcrsi->fns;
	phy_info_t *pi = rxgcrsi->pi;

	if (fns->phydump_chanest == NULL)
		return BCME_UNSUPPORTED;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* Go deaf to prevent PHY channel writes while doing reads */
	phy_rxgcrs_stay_in_carriersearch(rxgcrsi, TRUE);

	if (fns->phydump_chanest != NULL) {
		(fns->phydump_chanest)(fns->ctx, b);
	}

	/* Return from deaf */
	phy_rxgcrs_stay_in_carriersearch(rxgcrsi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}
#endif // endif

#ifdef WLTEST
int
phy_rxgcrs_dump_ch4rpcal(void *ctx, struct bcmstrbuf *b)
{
	phy_rxgcrs_info_t *rxgcrsi = (phy_rxgcrs_info_t *)ctx;
	phy_type_rxgcrs_fns_t *fns = rxgcrsi->fns;
	phy_info_t *pi = rxgcrsi->pi;
	uint16 num_tones;
	uint16 k, r, fftk, fft_size;
	int16  ch_re, ch_im;
	uint16 *tone_idx_tbl;
	uint16 tone_idx_20[52]  = {
		4,  5,  6,  7,  8,  9, 10, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 31,
		33, 34, 35, 36, 37, 38, 40, 41, 42, 43, 44, 45, 46,
		47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60};

	uint16 tone_idx_40[108] = {
		6,  7,  8,  9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
		25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62,
		66, 67, 68, 69, 70, 71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 83, 84,
		85, 86, 87, 88, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
		103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
		118, 119, 120, 121, 122};

#ifdef CHSPEC_IS80
	uint16 tone_idx_80[234] = {
		6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		24, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61,
		62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 90, 91, 92, 93, 94, 95, 96, 97, 98,
		99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
		114, 115, 116, 118, 119, 120, 121, 122, 123, 124, 125, 126, 130, 131, 132,
		133, 134, 135, 136, 137, 138, 140, 141, 142, 143, 144, 145, 146, 147, 148,
		149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
		164, 165, 166, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
		180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
		195, 196, 197, 198, 199, 200, 201, 202, 204, 205, 206, 207, 208, 209, 210,
		211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225,
		226, 227, 228, 229, 230, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
		242, 243, 244, 245, 246, 247, 248, 249, 250
	};
#endif /* CHSPEC_IS80 */
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	if (fns->get_chanest == NULL)
		return BCME_UNSUPPORTED;

	if (!pi->sh->up)
		return BCME_NOTUP;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* Go deaf to prevent PHY channel writes while doing reads */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		num_tones = 108;
		fft_size = 128;
		tone_idx_tbl = tone_idx_40;
#ifdef CHSPEC_IS80
	} else if (CHSPEC_IS80(pi->radio_chanspec)) {
		num_tones = 234;
		fft_size = 256;
		tone_idx_tbl = tone_idx_80;
#endif /* CHSPEC_IS80 */
	} else {
		num_tones = 52;
		fft_size = 64;
		tone_idx_tbl = tone_idx_20;
	}

	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, r) {
		for (k = 0; k < num_tones; k++) {
			fftk = tone_idx_tbl[k];
			fftk = (fftk < fft_size/2) ? (fftk + fft_size/2) : (fftk - fft_size/2);
			(fns->get_chanest)(fns->ctx, fftk, r, &ch_re, &ch_im);
			bcm_bprintf(b, "%d\n%d\n", ch_re, ch_im);
		}
	}

	/* Return from deaf */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}
#endif /* WLTEST */

#if defined(DBG_BCN_LOSS)
int
phy_rxgcrs_dump_phycal_rx_min(void *ctx, struct bcmstrbuf *b)
{
	phy_rxgcrs_info_t *rxgcrsi = (phy_rxgcrs_info_t *)ctx;
	phy_type_rxgcrs_fns_t *fns = rxgcrsi->fns;
	phy_info_t *pi = rxgcrsi->pi;

	if (fns->phydump_phycal_rxmin == NULL) {
		return BCME_UNSUPPORTED;
	}

	if (!pi->sh->up) {
		PHY_ERROR(("wl%d: %s: Not up, cannot dump \n", pi->sh->unit, __FUNCTION__));
		return BCME_NOTUP;
	}

	return (fns->phydump_phycal_rxmin)(fns->ctx, b);
}
#endif /* DBG_BCN_LOSS */

#ifndef ATE_BUILD
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
int
wlc_phy_iovar_forcecal_noise(phy_info_t *pi, void *a, bool set)
{
	phy_rxgcrs_info_t *info = pi->rxgcrsi;
	phy_type_rxgcrs_fns_t *fns = info->fns;

	if (fns->forcecal_noise != NULL) {
		return (fns->forcecal_noise)(fns->ctx, a, set);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
}
#endif // endif
#endif /* !ATE_BUILD */

uint16 phy_rxgcrs_sel_classifier(phy_info_t *pi, uint16 class_mask)
{
	phy_type_rxgcrs_fns_t *fns = pi->rxgcrsi->fns;
	ASSERT(pi != NULL);
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->sel_classifier != NULL) {
		return (fns->sel_classifier)(fns->ctx, class_mask);
	}

	return BCME_OK;
}
