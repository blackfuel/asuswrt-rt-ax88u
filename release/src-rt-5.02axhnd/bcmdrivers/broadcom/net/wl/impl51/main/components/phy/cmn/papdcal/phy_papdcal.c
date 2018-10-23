/*
 * PAPD CAL module implementation.
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
 * $Id: phy_papdcal.c 657373 2016-09-01 01:08:38Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_papdcal.h"
#include <phy_rstr.h>
#include <phy_papdcal.h>
#include <phy_utils_var.h>
#include <phy_papdcal_api.h>

/* module private states */
struct phy_papdcal_priv_info {
	phy_info_t				*pi;		/* PHY info ptr */
	phy_type_papdcal_fns_t	*fns;		/* Function ptr */
};

/* module private states memory layout */
typedef struct phy_papdcal_mem {
	phy_papdcal_info_t		cmn_info;
	phy_type_papdcal_fns_t	fns;
	phy_papdcal_priv_info_t priv;
	phy_papdcal_data_t data;
} phy_papdcal_mem_t;

/* local function declaration */
static int phy_papdcal_init_cb(phy_init_ctx_t *ctx);
static bool phy_wd_wfd_ll(phy_wd_ctx_t *ctx);

#if defined(BCMDBG)
static int phy_papdcal_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

static const char BCMATTACHDATA(rstr_txwbpapden)[] = "txwbpapden";

/* attach/detach */
phy_papdcal_info_t *
BCMATTACHFN(phy_papdcal_attach)(phy_info_t *pi)
{
	phy_papdcal_info_t *cmn_info = NULL;
	BCM_REFERENCE(rstr_txwbpapden);

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((cmn_info = phy_malloc(pi, sizeof(phy_papdcal_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	cmn_info->priv = &((phy_papdcal_mem_t *)cmn_info)->priv;
	cmn_info->priv->pi = pi;
	cmn_info->priv->fns = &((phy_papdcal_mem_t *)cmn_info)->fns;
	cmn_info->data = &((phy_papdcal_mem_t *)cmn_info)->data;

	/* init the papdcal states */
#if defined(WL_WBPAPD) && !defined(WL_WBPAPD_DISABLED)
	pi->ff->_wbpapd = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_txwbpapden, 0);
#else
	pi->ff->_wbpapd = FALSE;
#endif /* WL_WBPAPD */

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_papdcal_init_cb, cmn_info,
		PHY_INIT_PAPDCAL) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_wd_wfd_ll, cmn_info,
		PHY_WD_PRD_1TICK, PHY_WD_1TICK_PAPDCAL_WFDLL,
		PHY_WD_FLAG_DEF_DEFER) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "papdcal", phy_papdcal_dump, cmn_info);
#endif // endif

	return cmn_info;

	/* error */
fail:
	phy_papdcal_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_papdcal_detach)(phy_papdcal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info) {
		PHY_INFORM(("%s: null papdcal module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->priv->pi, cmn_info, sizeof(phy_papdcal_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_papdcal_register_impl)(phy_papdcal_info_t *cmn_info, phy_type_papdcal_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	*cmn_info->priv->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_papdcal_unregister_impl)(phy_papdcal_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->priv->fns = NULL;
}

/* papdcal init processing */
static int
WLBANDINITFN(phy_papdcal_init_cb)(phy_init_ctx_t *ctx)
{
	PHY_CAL(("%s\n", __FUNCTION__));

		return BCME_OK;
}

static bool
phy_wd_wfd_ll(phy_wd_ctx_t *ctx)
{
	phy_papdcal_info_t *papdcali = (phy_papdcal_info_t *)ctx;
	phy_type_papdcal_fns_t *fns = papdcali->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->wd_wfd_ll != NULL)
		(fns->wd_wfd_ll)(fns->ctx);
	return TRUE;
}

bool
phy_papdcal_epacal(phy_papdcal_info_t *papdcali)
{
	return papdcali->data->epacal2g || papdcali->data->epacal5g;
}

bool
phy_papdcal_is_wfd_phy_ll_enable(phy_papdcal_info_t *papdcali)
{
#ifdef WFD_PHY_LL
	phy_papdcal_data_t *dt = papdcali->data;
	return (dt->wfd_ll_enable && (dt->wfd_ll_chan_active || dt->wfd_ll_chan_active_force));
#else
	return 0;
#endif /* WFD_PHY_LL */
}

bool
phy_papdcal_epapd(phy_papdcal_info_t *papdcali)
{
	return PHY_EPAPD(papdcali->priv->pi);
}

#if defined(WLTEST) || defined(BCMDBG)
void
phy_papdcal_epa_dpd_set(phy_info_t *pi, uint8 enab_epa_dpd, bool in_2g_band)
{
	phy_papdcal_info_t *papdi = pi->papdcali;
	phy_type_papdcal_fns_t *fns = papdi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->epa_dpd_set != NULL) {
		(fns->epa_dpd_set)(fns->ctx, enab_epa_dpd, in_2g_band);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}
#endif /* defined(WLTEST) || defined(BCMDBG) */

#if defined(BCMDBG)
static int
phy_papdcal_dump(void *ctx, struct bcmstrbuf *b)
{
	return BCME_OK;
}
#endif // endif

#if (defined(WLTEST) || defined(WLPKTENG))
bool
wlc_phy_isperratedpden(wlc_phy_t *ppi)
{
	phy_papdcal_info_t *info;
	phy_type_papdcal_fns_t *fns;

	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	info = pi->papdcali;
	fns = info->priv->fns;

	if (fns->isperratedpden != NULL) {
		return (fns->isperratedpden)(fns->ctx);
	}
	else {
		return FALSE;
	}
}

void
wlc_phy_perratedpdset(wlc_phy_t *ppi, bool enable)
{
	phy_papdcal_info_t *info;
	phy_type_papdcal_fns_t *fns;

	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	info = pi->papdcali;
	fns = info->priv->fns;

	if (fns->perratedpdset != NULL)
		(fns->perratedpdset)(fns->ctx, enable);
}
#endif // endif

#if defined(WLTEST)
int phy_papdcal_get_lut_idx0(phy_info_t *pi, int32* idx)
{
	phy_type_papdcal_fns_t *fns = pi->papdcali->priv->fns;
	if (fns->get_idx0 != NULL) {
		return (fns->get_idx0)(fns->ctx, idx);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int phy_papdcal_get_lut_idx1(phy_info_t *pi, int32* idx)
{
	phy_type_papdcal_fns_t *fns = pi->papdcali->priv->fns;
	if (fns->get_idx1 != NULL) {
		return (fns->get_idx1)(fns->ctx, idx);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_papdcal_set_idx(phy_info_t *pi, int8 idx)
{
	phy_type_papdcal_fns_t *fns = pi->papdcali->priv->fns;

	if (fns->set_idx != NULL) {
		return (fns->set_idx)(fns->ctx, idx);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif // endif

#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
#ifndef ATE_BUILD
int
phy_papdcal_set_skip(phy_info_t *pi, uint8 skip)
{
	phy_type_papdcal_fns_t *fns = pi->papdcali->priv->fns;
	if (fns->set_skip != NULL) {
		return (fns->set_skip)(fns->ctx, skip);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_papdcal_get_skip(phy_info_t *pi, int32* skip)
{
	phy_type_papdcal_fns_t *fns = pi->papdcali->priv->fns;
	if (fns->get_skip != NULL) {
		return (fns->get_skip)(fns->ctx, skip);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* !ATE_BUILD */
#endif // endif

#if defined(WFD_PHY_LL)
int
phy_papdcal_set_wfd_ll_enable(phy_papdcal_info_t *papdcali, uint8 int_val)
{
	phy_type_papdcal_fns_t *fns = papdcali->priv->fns;
	if (fns->set_wfd_ll_enable != NULL) {
		return (fns->set_wfd_ll_enable)(fns->ctx, int_val);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_papdcal_get_wfd_ll_enable(phy_papdcal_info_t *papdcali, int32 *ret_int_ptr)
{
	phy_type_papdcal_fns_t *fns = papdcali->priv->fns;
	if (fns->get_wfd_ll_enable != NULL) {
		return (fns->get_wfd_ll_enable)(fns->ctx, ret_int_ptr);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* WFD_PHY_LL */

/* Convert epsilon table value to complex number */
int
phy_papdcal_decode_epsilon(uint32 epsilon, int32 *eps_real, int32 *eps_imag)
{
	if ((*eps_imag = (epsilon>>13)) > 0xfff)
		*eps_imag -= 0x2000; /* Sign extend */
	if ((*eps_real = (epsilon & 0x1fff)) > 0xfff)
		*eps_real -= 0x2000; /* Sign extend */
	return BCME_OK;
}
