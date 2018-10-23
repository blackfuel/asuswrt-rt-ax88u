/*
 * MU-MIMO phy module implementation
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
 * $Id: phy_mu.c 612466 2016-01-14 02:49:29Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_smc.h>
#include <phy_smc_api.h>

/* module private states */
struct phy_smc_info {
	phy_info_t *pi;
	phy_type_smc_fns_t *fns;
	bool download;
};

/* module private states memory layout */
typedef struct {
	phy_smc_info_t smc_info;
	phy_type_smc_fns_t fns;
/* add other variable size variables here at the end */
} phy_smc_mem_t;

/* attach/detach */
phy_smc_info_t *
BCMATTACHFN(phy_smc_attach)(phy_info_t *pi)
{
	phy_smc_info_t *smc_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((smc_info = phy_malloc(pi, sizeof(phy_smc_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	smc_info->pi = pi;
	smc_info->fns = &((phy_smc_mem_t *)smc_info)->fns;

	return smc_info;

	/* error */
fail:
	phy_smc_detach(smc_info);
	return NULL;
}

void
BCMATTACHFN(phy_smc_detach)(phy_smc_info_t *smc_info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (smc_info == NULL) {
		PHY_INFORM(("%s: null smc module\n", __FUNCTION__));
		return;
	}

	pi = smc_info->pi;

	phy_mfree(pi, smc_info, sizeof(phy_smc_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_smc_register_impl)(phy_smc_info_t *smc_info, phy_type_smc_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*smc_info->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_smc_unregister_impl)(phy_smc_info_t *smc_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

int
phy_smc_reset(phy_info_t *pi, bool val)
{
	phy_type_smc_fns_t *fns = pi->smci->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	/* redirect the request to PHY type specific implementation */
	return (fns->smc_reset)(fns->ctx, val);
}

int
phy_smc_download(phy_info_t *pi, uint16 tblLen, const uint32 *tblData)
{
	phy_type_smc_fns_t *fns = pi->smci->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	/* redirect the request to PHY type specific implementation */
	return (fns->smc_download)(fns->ctx, tblLen, tblData);
}
