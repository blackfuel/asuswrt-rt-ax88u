/*
 * Health check module.
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
 * $Id: phy_ac_hc.c 680712 2017-01-23 03:31:43Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_hc.h"
#include <phy_ac.h>
#include <phy_ac_hc.h>
#include <phy_ac_info.h>

/* module private states */
struct phy_ac_hc_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_hc_info_t *info;
};

#ifdef RADIO_HEALTH_CHECK

/* local functions */
static int phy_ac_hc_debugcrash_forcefail(phy_type_hc_ctx_t *ctx, phy_crash_reason_t dctype);

/* register phy type specific implementation */
phy_ac_hc_info_t *
BCMATTACHFN(phy_ac_hc_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_hc_info_t *info)
{
	phy_ac_hc_info_t *hci;
	phy_type_hc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((hci = phy_malloc(pi, sizeof(*hci))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	hci->pi = pi;
	hci->aci = aci;
	hci->info = info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = hci;
	fns.force_fail = phy_ac_hc_debugcrash_forcefail;
	if (phy_hc_register_impl(info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_hc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return hci;

	/* error handling */
fail:
	if (hci != NULL)
		phy_mfree(pi, hci, sizeof(*hci));
	return NULL;
}

void
BCMATTACHFN(phy_ac_hc_unregister_impl)(phy_ac_hc_info_t *hci)
{
	phy_info_t *pi;
	phy_hc_info_t *info;

	ASSERT(hci);

	pi = hci->pi;
	info = hci->info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_hc_unregister_impl(info);

	phy_mfree(pi, hci, sizeof(*hci));
}

static int
phy_ac_hc_debugcrash_forcefail(phy_type_hc_ctx_t *ctx, phy_crash_reason_t dctype)
{
	phy_ac_hc_info_t *hci = (phy_ac_hc_info_t *)ctx;
	phy_info_t *pi = hci->pi;

	PHY_TRACE(("\n %s : dctype : %d \n", __FUNCTION__, dctype));
	switch (dctype) {
		case PHY_RC_DESENSE_LIMITS:
			if (phy_ac_noise_force_fail_desense(pi->u.pi_acphy->noisei) !=
					BCME_OK)
				return BCME_UNSUPPORTED;
			break;
		case PHY_RC_BASEINDEX_LIMITS:
			if (phy_ac_tpc_force_fail_baseindex(pi->u.pi_acphy->tpci) !=
					BCME_OK)
				return BCME_UNSUPPORTED;
			break;
		case PHY_RC_TXCHAIN_INVALID:
			if (phy_ac_stf_force_fail_txchain(pi->u.pi_acphy->stfi) !=
					BCME_OK)
				return BCME_UNSUPPORTED;
			break;
		default:
			ASSERT(0);
			return BCME_UNSUPPORTED;
	}
	return BCME_OK;
}
#endif /* RADIO_HEALTH_CHECK */
