/*
 * NPHY Debug modules implementation
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
 * $Id: phy_n_dbg.c $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_mem.h>
#include "phy_type_dbg.h"
#include <phy_n.h>
#include <phy_n_dbg.h>
#if defined(BCMDBG) || defined(WLTEST)
#include <bcmdevs.h>
#include <wlc_phyreg_n.h>
#include <phy_utils_reg.h>
#endif // endif

/* module private states */
struct phy_n_dbg_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_dbg_info_t *info;
};

/* local functions */
#if defined(BCMDBG) || defined(WLTEST)
static int phy_n_dbg_test_evm(phy_type_dbg_ctx_t *ctx, int channel, uint rate,
	int txpwr);
static int phy_n_dbg_test_carrier_suppress(phy_type_dbg_ctx_t *ctx, int channel);
#else
#define phy_n_dbg_test_evm NULL
#define phy_n_dbg_test_carrier_suppress NULL
#endif /* BCMDBG || WLTEST  */

/* register phy type specific implementation */
phy_n_dbg_info_t *
BCMATTACHFN(phy_n_dbg_register_impl)(phy_info_t *pi, phy_n_info_t *ni, phy_dbg_info_t *info)
{
	phy_n_dbg_info_t *di;
	phy_type_dbg_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((di = phy_malloc(pi, sizeof(phy_n_dbg_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	di->pi = pi;
	di->ni = ni;
	di->info = info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = di;
	fns.test_evm = phy_n_dbg_test_evm;
	fns.test_carrier_suppress = phy_n_dbg_test_carrier_suppress;

	if (phy_dbg_register_impl(info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_dbg_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return di;

	/* error handling */
fail:
	if (di != NULL)
		phy_mfree(pi, di, sizeof(phy_n_dbg_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_dbg_unregister_impl)(phy_n_dbg_info_t *di)
{
	phy_info_t *pi;
	phy_dbg_info_t *info;

	ASSERT(di);

	pi = di->pi;
	info = di->info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_dbg_unregister_impl(info);

	phy_mfree(pi, di, sizeof(phy_n_dbg_info_t));
}

#if defined(BCMDBG) || defined(WLTEST)
static int
phy_n_dbg_test_evm(phy_type_dbg_ctx_t *ctx, int channel, uint rate, int txpwr)
{
	phy_n_dbg_info_t *di = (phy_n_dbg_info_t *)ctx;
	phy_info_t *pi = di->pi;
	uint16 reg = 0;
	int bcmerror = 0;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), pi->evm_phytest);

		pi->evm_phytest = 0;

		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL) {
			W_REG(pi->sh->osh, D11_PSM_GPIOOUT(pi), pi->evm_o);
			W_REG(pi->sh->osh, D11_PSM_GPIOEN(pi), pi->evm_oe);
			OSL_DELAY(1000);
		}
		return 0;
	}

	phy_dbg_test_evm_init(pi);

	if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
		return bcmerror;

	reg = phy_dbg_test_evm_reg(rate);

	PHY_INFORM(("wlc_evm: rate = %d, reg = 0x%x\n", rate, reg));

	/* Save original contents */
	if (pi->evm_phytest == 0) {
		pi->evm_phytest = phy_utils_read_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST));
	}

	/* Set EVM test mode */
	phy_utils_and_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST),
		~(TST_TXTEST_ENABLE|TST_TXTEST_RATE|TST_TXTEST_PHASE));
	phy_utils_or_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), TST_TXTEST_ENABLE | reg);

	return BCME_OK;
}

static int
phy_n_dbg_test_carrier_suppress(phy_type_dbg_ctx_t *ctx, int channel)
{
	phy_n_dbg_info_t *di = (phy_n_dbg_info_t *)ctx;
	phy_info_t *pi = di->pi;
	int bcmerror = 0;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), pi->car_sup_phytest);

		pi->car_sup_phytest = 0;
		return 0;
	}

	if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
		return bcmerror;

	/* Save original contents */
	if (pi->car_sup_phytest == 0) {
		pi->car_sup_phytest = phy_utils_read_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST));
	}

	/* set carrier suppression test mode */
	PHY_REG_LIST_START
		PHY_REG_AND_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_TEST, 0xfc00)
		PHY_REG_OR_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_TEST, 0x0228)
	PHY_REG_LIST_EXECUTE(pi);

	return BCME_OK;
}
#endif /* BCMDBG || WLTEST  */
