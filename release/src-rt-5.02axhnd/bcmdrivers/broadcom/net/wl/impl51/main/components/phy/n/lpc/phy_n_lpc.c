/*
 * NPHY Link Power Control module implementation
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
 * $Id: phy_n_lpc.c $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_lpc.h"
#include <phy_n.h>
#include <phy_n_lpc.h>

/* module private states */
struct phy_n_lpc_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_lpc_info_t *cmn_info;
};

/* local functions */
static uint8 wlc_phy_lpc_getminidx_nphy(void);
static uint8 wlc_phy_lpc_getoffset_nphy(uint8 index);
static uint8 wlc_phy_lpc_get_txcpwrval_nphy(uint16 phytxctrlword);
static void wlc_phy_lpc_setmode_nphy(phy_type_lpc_ctx_t *ctx, bool enable);
static void wlc_phy_lpc_set_txcpwrval_nphy(uint16 *phytxctrlword, uint8 txcpwrval);

/* register phy type specific implementation */
phy_n_lpc_info_t *
BCMATTACHFN(phy_n_lpc_register_impl)(phy_info_t *pi, phy_n_info_t *ni, phy_lpc_info_t *cmn_info)
{
	phy_n_lpc_info_t *n_info;
	phy_type_lpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((n_info = phy_malloc(pi, sizeof(phy_n_lpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	n_info->pi = pi;
	n_info->ni = ni;
	n_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = n_info;
	fns.setmode = wlc_phy_lpc_setmode_nphy;
	fns.getpwros = wlc_phy_lpc_getoffset_nphy;
	fns.getminidx = wlc_phy_lpc_getminidx_nphy;
	fns.gettxcpwrval = wlc_phy_lpc_get_txcpwrval_nphy;
	fns.settxcpwrval = wlc_phy_lpc_set_txcpwrval_nphy;
	if (phy_lpc_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_lpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return n_info;

	/* error handling */
fail:
	if (n_info != NULL)
		phy_mfree(pi, n_info, sizeof(phy_n_lpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_lpc_unregister_impl)(phy_n_lpc_info_t *n_info)
{
	phy_info_t *pi;
	phy_lpc_info_t *cmn_info;

	ASSERT(n_info);
	pi = n_info->pi;
	cmn_info = n_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_lpc_unregister_impl(cmn_info);

	phy_mfree(pi, n_info, sizeof(phy_n_lpc_info_t));
}

/* ********************************************* */
/*				Callback Functions Table					*/
/* ********************************************* */
#define PWR_SEL_MIN_IDX 24
static uint8
wlc_phy_lpc_getminidx_nphy(void)
{
	return PWR_SEL_MIN_IDX;
}

static uint8
wlc_phy_lpc_getoffset_nphy(uint8 index)
{
	return index;
}

static uint8
wlc_phy_lpc_get_txcpwrval_nphy(uint16 phytxctrlword)
{
	return (phytxctrlword & PHY_TXC_PWR_MASK) >> PHY_TXC_PWR_SHIFT;
}

static void
wlc_phy_lpc_setmode_nphy(phy_type_lpc_ctx_t *ctx, bool enable)
{
	/* do nothing; just return */
	/* fn put to maintain modularity across PHYs */
	return;
}

static void
wlc_phy_lpc_set_txcpwrval_nphy(uint16 *phytxctrlword, uint8 txcpwrval)
{
	*phytxctrlword = (*phytxctrlword & ~PHY_TXC_PWR_MASK) |
		(txcpwrval << PHY_TXC_PWR_SHIFT);
	return;
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
