/*
 * LCN20PHY Link Power Control module implementation
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
 * $Id: phy_lcn20_lpc.c $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_lpc.h"
#include <phy_lcn20.h>
#include <phy_lcn20_lpc.h>

#define LPC_MIN_IDX 19

/* module private states */
struct phy_lcn20_lpc_info {
	phy_info_t *pi;
	phy_lcn20_info_t *ni;
	phy_lpc_info_t *cmn_info;
};

/* local functions */
static uint8 wlc_lcn20phy_lpc_getminidx(void);
static void wlc_lcn20phy_lpc_setmode(phy_type_lpc_ctx_t *ctx, bool enable);
static uint8 wlc_lcn20phy_lpc_getoffset(uint8 index);
static uint8 wlc_lcn20phy_lpc_get_txcpwrval(uint16 phytxctrlword);
static void wlc_lcn20phy_lpc_set_txcpwrval(uint16 *phytxctrlword, uint8 txcpwrval);
#ifdef WL_LPC_DEBUG
static uint8 * wlc_lcn20phy_lpc_get_pwrlevelptr(void);
#endif /* WL_LPC_DEBUG */

/* register phy type specific implementation */
phy_lcn20_lpc_info_t *
BCMATTACHFN(phy_lcn20_lpc_register_impl)(phy_info_t *pi, phy_lcn20_info_t *ni,
	phy_lpc_info_t *cmn_info)
{
	phy_lcn20_lpc_info_t *lcn20_info;
	phy_type_lpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((lcn20_info = phy_malloc(pi, sizeof(phy_lcn20_lpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	lcn20_info->pi = pi;
	lcn20_info->ni = ni;
	lcn20_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = lcn20_info;
	fns.getminidx = wlc_lcn20phy_lpc_getminidx;
	fns.getpwros = wlc_lcn20phy_lpc_getoffset;
	fns.gettxcpwrval = wlc_lcn20phy_lpc_get_txcpwrval;
	fns.settxcpwrval = wlc_lcn20phy_lpc_set_txcpwrval;
	fns.setmode = wlc_lcn20phy_lpc_setmode;
#ifdef WL_LPC_DEBUG
	fns.getpwrlevelptr = wlc_lcn20phy_lpc_get_pwrlevelptr;
#endif // endif

	if (phy_lpc_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_lpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return lcn20_info;

	/* error handling */
fail:
	if (lcn20_info != NULL)
		phy_mfree(pi, lcn20_info, sizeof(phy_lcn20_lpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_lpc_unregister_impl)(phy_lcn20_lpc_info_t *lcn20_info)
{
	phy_info_t *pi;
	phy_lpc_info_t *cmn_info;

	ASSERT(lcn20_info);
	pi = lcn20_info->pi;
	cmn_info = lcn20_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_lpc_unregister_impl(cmn_info);

	phy_mfree(pi, lcn20_info, sizeof(phy_lcn20_lpc_info_t));
}

/* ********************************************* */
/*				Callback Functions Table					*/
/* ********************************************* */
uint8
wlc_lcn20phy_lpc_getminidx(void)
{
	return LPC_MIN_IDX;
}

void
wlc_lcn20phy_lpc_setmode(phy_type_lpc_ctx_t *ctx, bool enable)
{
	PHY_INFORM(("\n lpc mode set to : %d\n", enable));
}

uint8
wlc_lcn20phy_lpc_getoffset(uint8 index)
{
	return index;
}

uint8
wlc_lcn20phy_lpc_get_txcpwrval(uint16 phytxctrlword)
{
	return (phytxctrlword & PHY_TXC_PWR_MASK) >> PHY_TXC_PWR_SHIFT;
}

void
wlc_lcn20phy_lpc_set_txcpwrval(uint16 *phytxctrlword, uint8 txcpwrval)
{
	*phytxctrlword = (*phytxctrlword & ~PHY_TXC_PWR_MASK) |
		(txcpwrval << PHY_TXC_PWR_SHIFT);
	return;
}
#ifdef WL_LPC_DEBUG
uint8 *
wlc_lcn20phy_lpc_get_pwrlevelptr(void)
{
	return lpc_pwr_level;
}
#endif // endif

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
