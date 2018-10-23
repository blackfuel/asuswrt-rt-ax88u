/*
 * lcn20PHY Rx Spur canceller module implementation
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
 * $Id: $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_mem.h>
#include <phy_rxspur.h>
#include "phy_type_rxspur.h"
#include <phy_lcn20.h>
#include <phy_lcn20_rxspur.h>
#include <hndpmu.h>
#include <wlc_phy_shim.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
/* TODO: all these are going away... > */
#endif // endif

/* The default fvco MUST match the setting in pmu-pll-init */
#define LCN20PHY_SPURMODE_FVCO_DEFAULT  LCN20PHY_SPURMODE_FVCO_972

/* module private states */
struct phy_lcn20_rxspur_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_rxspur_info_t *rxspuri;
};

/* local functions */
#if defined(WLTEST)
static int phy_lcn20_rxspur_set_force_spurmode(phy_type_rxspur_ctx_t *ctx, int16 int_val);
static int phy_lcn20_rxspur_get_force_spurmode(phy_type_rxspur_ctx_t *ctx, int32 *ret_int_ptr);
#endif /* WLTEST */

/* Register/unregister lcn20PHY specific implementation to common layer */
phy_lcn20_rxspur_info_t *
BCMATTACHFN(phy_lcn20_rxspur_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_rxspur_info_t *rxspuri)
{
	phy_lcn20_rxspur_info_t *info;
	phy_type_rxspur_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn20_rxspur_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn20_rxspur_info_t));
	info->pi = pi;
	info->lcn20i = lcn20i;
	info->rxspuri = rxspuri;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#if defined(WLTEST)
	fns.set_force_spurmode = phy_lcn20_rxspur_set_force_spurmode;
	fns.get_force_spurmode  = phy_lcn20_rxspur_get_force_spurmode;
#endif /* WLTEST */
	fns.ctx = info;

	phy_rxspur_register_impl(rxspuri, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn20_rxspur_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_rxspur_unregister_impl)(phy_lcn20_rxspur_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_rxspur_info_t *rxspuri = info->rxspuri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_rxspur_unregister_impl(rxspuri);

	phy_mfree(pi, info, sizeof(phy_lcn20_rxspur_info_t));
}

#if defined(WLTEST)
static void
wlc_lcn20phy_set_spurmode(phy_info_t *pi, uint8 channel)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint8 spurmode = LCN20PHY_SPURMODE_FVCO_DEFAULT;

	PHY_TRACE(("%s:\n", __FUNCTION__));

	if (pi_lcn20->spurmode_override) {
		/*
		  spurmode decided by iovar
		*/
		spurmode = pi_lcn20->forced_spurmode;
	} else {
		/*
		  spurmode decided by nvram config
		*/
		spurmode = pi_lcn20->spurmode;
	}

	si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, spurmode);
	wlapi_switch_macfreq(pi->sh->physhim, WL_SPURAVOID_ON1);
}

static int
phy_lcn20_rxspur_set_force_spurmode(phy_type_rxspur_ctx_t *ctx, int16 int_val)
{
	phy_lcn20_rxspur_info_t *rxspuri = (phy_lcn20_rxspur_info_t *) ctx;
	phy_info_t *pi = rxspuri->pi;
	phy_info_lcn20phy_t *pi_lcn20 = rxspuri->lcn20i;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec); /* see wlioctl.h */

	if (int_val == -1) {
		pi_lcn20->spurmode_override = FALSE;
	} else {
		pi_lcn20->spurmode_override = TRUE;
	}

	switch (int_val) {
	case -1:
		PHY_TRACE(("Spurmode override is off; default spurmode restored; %s \n",
		__FUNCTION__));
		break;
	case LCN20PHY_SPURMODE_FVCO_972:
		PHY_TRACE(("Force spurmode to Fvco 972 MHz; %s \n",
		__FUNCTION__));
		pi_lcn20->forced_spurmode = LCN20PHY_SPURMODE_FVCO_972;
		break;
	case LCN20PHY_SPURMODE_FVCO_980:
		PHY_TRACE(("Force spurmode to Fvco 980 MHz; %s \n",
		__FUNCTION__));
		pi_lcn20->forced_spurmode = LCN20PHY_SPURMODE_FVCO_980;
		break;
	case LCN20PHY_SPURMODE_FVCO_984:
		PHY_TRACE(("Force spurmode to Fvco 984 MHz; %s \n",
		__FUNCTION__));
		pi_lcn20->forced_spurmode = LCN20PHY_SPURMODE_FVCO_984;
		break;
	case LCN20PHY_SPURMODE_FVCO_326P4:
		PHY_TRACE(("Force spurmode to Fvco 326.4 MHz; %s \n",
		__FUNCTION__));
		pi_lcn20->forced_spurmode = LCN20PHY_SPURMODE_FVCO_326P4;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unsupported spurmode %d.\n",
			pi->sh->unit, __FUNCTION__, int_val));
		ASSERT(FALSE);
		pi_lcn20->forced_spurmode = LCN20PHY_SPURMODE_FVCO_DEFAULT;
		return BCME_ERROR;
	}

	wlc_lcn20phy_set_spurmode(pi, channel);
	return BCME_OK;
}

static int
phy_lcn20_rxspur_get_force_spurmode(phy_type_rxspur_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_lcn20_rxspur_info_t *rxspuri = (phy_lcn20_rxspur_info_t *) ctx;

	if (rxspuri->lcn20i->spurmode_override) {
		*ret_int_ptr = rxspuri->lcn20i->forced_spurmode;
	} else {
		*ret_int_ptr = -1;
	}
	return BCME_OK;
}
#endif /* defined(WLTEST) */
