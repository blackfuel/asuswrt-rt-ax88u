/*
 * High Efficiency (802.11ax) (HE) phy module implementation
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: $
 */
#ifdef WL11AX

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_hecap.h>
#include <phy_hecap.h>
#include <phy_hecap_api.h>

/* module private states */
struct phy_hecap_info {
	phy_info_t *pi;
	phy_type_hecap_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_hecap_info_t he_info;
	phy_type_hecap_fns_t fns;
/* add other variable size variables here at the end */
} phy_hecap_mem_t;

/* function prototypes */

/* attach/detach */
phy_hecap_info_t *
BCMATTACHFN(phy_hecap_attach)(phy_info_t *pi)
{
	phy_hecap_info_t *he_info;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((he_info = phy_malloc(pi, sizeof(phy_hecap_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	he_info->pi = pi;
	he_info->fns = &((phy_hecap_mem_t *)he_info)->fns;

	return he_info;

	/* error */
fail:
	phy_hecap_detach(he_info);
	return NULL;
}

void

BCMATTACHFN(phy_hecap_detach)(phy_hecap_info_t *he_info)
{
	phy_info_t *pi;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (he_info == NULL) {
		PHY_INFORM(("%s: null HE module\n", __FUNCTION__));
		return;
	}

	pi = he_info->pi;
	phy_mfree(pi, he_info, sizeof(phy_hecap_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_hecap_register_impl)(phy_hecap_info_t *he_info,
	phy_type_hecap_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
	*he_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_hecap_unregister_impl)(phy_hecap_info_t *he_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

void
phy_hecap_fill_phycap_info(phy_info_t *pi, he_phy_cap_t *phycap)
{
	phy_hecap_info_t *hecap_info = pi->hecapi;
	phy_type_hecap_fns_t *fns = hecap_info->fns;

	bzero(phycap, sizeof(*phycap));

	if (fns->hecap_get != NULL) {
		(fns->hecap_get)(fns->ctx, phycap);
	}
}

void
phy_hecap_write_bsscolor(phy_info_t *pi, wlc_bsscolor_t *bsscolor)
{
	phy_hecap_info_t *hecap_info = pi->hecapi;
	phy_type_hecap_fns_t *fns = hecap_info->fns;

	if (fns->bsscolor_write != NULL) {
		(fns->bsscolor_write)(fns->ctx, bsscolor);
	}
}

void
phy_hecap_write_pe_dur(phy_info_t *pi, uint8 pe_dur)
{
	phy_hecap_info_t *hecap_info = pi->hecapi;
	phy_type_hecap_fns_t *fns = hecap_info->fns;

	if (fns->pe_dur_write != NULL) {
		(fns->pe_dur_write)(fns->ctx, pe_dur);
	}
}

void
phy_hecap_get_rateset(phy_info_t *pi, wlc_he_rateset_t *he_rateset)
{
	phy_hecap_info_t *hecap_info = pi->hecapi;
	phy_type_hecap_fns_t *fns = hecap_info->fns;

	if (fns->hecap_get_rateset != NULL) {
		(fns->hecap_get_rateset)(fns->ctx, he_rateset);
	}
}

uint8
phy_hecap_get_ppet(phy_info_t *pi)
{
	phy_hecap_info_t *hecap_info = pi->hecapi;
	phy_type_hecap_fns_t *fns = hecap_info->fns;

	if (fns->hecap_get_ppet != NULL) {
		return ((fns->hecap_get_ppet)(fns->ctx));
	}

	/* return worst case possible */
	return WL_HE_PPET_16US;
}

#endif /* WL11AX */
