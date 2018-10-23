/*
 * ACPHY MU-MIMO module implementation
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
 * $Id: phy_ac_mu.c 668212 2016-11-02 09:00:52Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <phy_ac_smc.h>

/* module private states */
struct phy_ac_smc_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_smc_info_t *smc_info;
};

/* local functions */
static int phy_ac_smc_reset(phy_type_smc_ctx_t *ctx, bool val);
static int phy_ac_smc_download(phy_type_smc_ctx_t *ctx, uint16 tblLen, const uint32 *tblData);

/* register phy type specific implementation */
phy_ac_smc_info_t *
BCMATTACHFN(phy_ac_smc_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_smc_info_t *smci)
{
	phy_ac_smc_info_t *ac_smc_info;
	phy_type_smc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_smc_info = phy_malloc(pi, sizeof(phy_ac_smc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	ac_smc_info->pi = pi;
	ac_smc_info->aci = aci;
	ac_smc_info->smc_info = smci;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.smc_reset = phy_ac_smc_reset;
	fns.smc_download = phy_ac_smc_download;
	fns.ctx = ac_smc_info;

	phy_smc_register_impl(smci, &fns);

	return ac_smc_info;

	/* error handling */
fail:
	if (ac_smc_info != NULL)
		phy_mfree(pi, ac_smc_info, sizeof(phy_ac_smc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_smc_unregister_impl)(phy_ac_smc_info_t *ac_smc_info)
{
	phy_info_t *pi = ac_smc_info->pi;
	phy_smc_info_t *mi = ac_smc_info->smc_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_smc_unregister_impl(mi);

	phy_mfree(pi, ac_smc_info, sizeof(phy_ac_smc_info_t));
}

static int
phy_ac_smc_reset(phy_type_smc_ctx_t *ctx, bool val)
{
	phy_ac_smc_info_t *info = (phy_ac_smc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	if (val) {
		/* we need to follow this sequence */
		MOD_PHYREG(pi, smc_rxctrl_buffer_start_addr, mac2smc_reset_buffer, 1);
		MOD_PHYREG(pi, SMCCoreControl, smc_ULofdma_enable_RU_demod_ordering, 1);
		MOD_PHYREG(pi, SMCClkResetControl, SMCclkEn, 1);
		MOD_PHYREG(pi, SMCClkResetControl, SMC_PCRESET, 1);
	} else {
		WRITE_PHYREG(pi, SMCClkResetControl, 0);
	}

	return BCME_OK;
}

static int
phy_ac_smc_download(phy_type_smc_ctx_t *ctx, uint16 tblLen, const uint32 *tblData)
{
	phy_ac_smc_info_t *info = (phy_ac_smc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SMCPMTBL, tblLen, 0, 48, tblData);

	pi->smci->download = TRUE;

	return BCME_OK;
}
