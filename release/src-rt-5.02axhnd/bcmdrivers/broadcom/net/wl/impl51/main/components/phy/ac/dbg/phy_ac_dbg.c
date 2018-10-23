/*
 * ACPHY Debug modules implementation
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
 * $Id: phy_ac_dbg.c 765492 2018-07-05 10:49:51Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_dbg.h"
#include <phy_ac.h>
#include <phy_ac_dbg.h>
#include <phy_ac_info.h>
#include <phy_utils_reg.h>
/* *************************** */
/* Modules used by this module */
/* *************************** */
#include <wlc_phyreg_ac.h>
#include <phy_ac_noise.h>
#include <phy_ac_tpc.h>

/* module private states */
struct phy_ac_dbg_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_dbg_info_t *info;
};

/* local functions */
#if defined(BCMDBG)
static void wlc_acphy_txerr_dump(phy_type_dbg_ctx_t *ctx, uint16 err);
#else
#define wlc_acphy_txerr_dump NULL
#endif // endif
#if defined(DNG_DBGDUMP)
static void wlc_acphy_print_phydbg_regs(phy_type_dbg_ctx_t *ctx);
#else
#define wlc_acphy_print_phydbg_regs NULL
#endif /* DNG_DBGDUMP */
#if defined(BCMDBG) || defined(WL_MACDBG)
static void phy_ac_dbg_gpio_out_enab(phy_type_dbg_ctx_t *ctx, bool enab);
#else
#define phy_ac_dbg_gpio_out_enab NULL
#endif // endif
#ifdef PHY_DUMP_BINARY
static uint16 phy_ac_dbg_get_phyreg_address(phy_type_dbg_ctx_t *ctx, uint16 addr);
#endif // endif
#ifdef AWD_EXT_TRAP
static void phy_ac_dbg_ext_trap(phy_type_dbg_ctx_t *ctx, phy_dbg_ext_trap_err_t *ext_trap_data);
#endif // endif

/* register phy type specific implementation */
phy_ac_dbg_info_t *
BCMATTACHFN(phy_ac_dbg_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_dbg_info_t *info)
{
	phy_ac_dbg_info_t *di;
	phy_type_dbg_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((di = phy_malloc(pi, sizeof(*di))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	di->pi = pi;
	di->aci = aci;
	di->info = info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = di;
	fns.txerr_dump = wlc_acphy_txerr_dump;
	fns.print_phydbg_regs = wlc_acphy_print_phydbg_regs;
	fns.gpio_out_enab = phy_ac_dbg_gpio_out_enab;
#ifdef PHY_DUMP_BINARY
	fns.phyregaddr = phy_ac_dbg_get_phyreg_address;
#endif // endif
#ifdef AWD_EXT_TRAP
	fns.ext_trap = phy_ac_dbg_ext_trap;
#endif // endif

	if (phy_dbg_register_impl(info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_dbg_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return di;

	/* error handling */
fail:
	if (di != NULL)
		phy_mfree(pi, di, sizeof(*di));
	return NULL;
}

void
BCMATTACHFN(phy_ac_dbg_unregister_impl)(phy_ac_dbg_info_t *di)
{
	phy_info_t *pi;
	phy_dbg_info_t *info;

	ASSERT(di);

	pi = di->pi;
	info = di->info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_dbg_unregister_impl(info);

	phy_mfree(pi, di, sizeof(*di));
}

#if defined(BCMDBG)
static const bcm_bit_desc_t attr_flags_phy_err[] = {
	{ACPHY_TxError_NDPError_MASK(0), "NDPError"},
	{ACPHY_TxError_RsvdBitError_MASK(0), "RsvdBitError"},
	{ACPHY_TxError_illegal_frame_type_MASK(0), "Illegal frame type"},
	{ACPHY_TxError_COMBUnsupport_MASK(0), "COMBUnsupport"},
	{ACPHY_TxError_BWUnsupport_MASK(0), "BWUnsupport"},
	{ACPHY_TxError_txInCal_MASK(0), "txInCal_MASK"},
	{ACPHY_TxError_send_frame_low_MASK(0), "send_frame_low"},
	{ACPHY_TxError_lengthmismatch_short_MASK(0), "lengthmismatch_short"},
	{ACPHY_TxError_lengthmismatch_long_MASK(0), "lengthmismatch_long"},
	{ACPHY_TxError_invalidRate_MASK(0), "invalidRate_MASK"},
	{ACPHY_TxError_unsupportedmcs_MASK(0), "unsupported mcs"},
	{ACPHY_TxError_send_frame_low_MASK(0), "send_frame_low"},
	{0, NULL}
};
static const bcm_bit_desc_t attr_flags_phy_err_ax[] = {
	{ACPHY_phytxerrorStatusReg0_txFrameErrorMask_MASK(0), "txFrameErrorMask"},
	{ACPHY_phytxerrorStatusReg0_txFrameError_MASK(0), "txFrameError"},
	{ACPHY_phytxerrorStatusReg0_legacyregs_MASK(0), "legacyregs"},
	{ACPHY_phytxerrorStatusReg0_transactionError_MASK(0), "transactionError"},
	{ACPHY_phytxerrorStatusReg0_sigAError_MASK(0), "sigAError"},
	{ACPHY_phytxerrorStatusReg0_macPhyXError_MASK(0), "macPhyXError"},
	{0, NULL}
};
static const char *frametypes[] = {
	"FT_CCK", "FT_OFDM", "FT_HT", "FT_VHT", "FT_HE", "FT_AH", "FT inv6", "FT inv7"
};

static void
wlc_acphy_txerr_dump(phy_type_dbg_ctx_t *ctx, uint16 err)
{
	phy_ac_dbg_info_t *di = (phy_ac_dbg_info_t *)ctx;
	phy_info_t *pi = di->pi;

	if (err != 0) {
		char flagstr[128];
		uint rev = pi->pubpi->phy_rev;
		BCM_REFERENCE(rev);

		if (ACMAJORREV_47_51(rev)) {
			uint ft = (err & ACPHY_phytxerrorStatusReg0_frameType_MASK(rev)) >>
				ACPHY_phytxerrorStatusReg0_frameType_SHIFT(rev);
			uint usridx = (err & ACPHY_phytxerrorStatusReg0_UsrIdx_MASK(rev)) >>
				ACPHY_phytxerrorStatusReg0_UsrIdx_SHIFT(rev);
			uint16 err_local = err & ~(ACPHY_phytxerrorStatusReg0_frameType_MASK(rev) |
				ACPHY_phytxerrorStatusReg0_UsrIdx_MASK(rev));
			int written = snprintf(flagstr, 22, "UsrIdx:0x%2x %s ",
				usridx, frametypes[ft]);
			bcm_format_flags(attr_flags_phy_err_ax, err_local, flagstr + written, 100);
		} else {
			bcm_format_flags(attr_flags_phy_err, err, flagstr, 128);
		}
		printf("Tx PhyErr 0x%04x (%s)\n", err, flagstr);
	}
}
#endif // endif

#if defined(DNG_DBGDUMP)
static void
wlc_acphy_print_phydbg_regs(phy_type_dbg_ctx_t *ctx)
{
	phy_ac_dbg_info_t *di = (phy_ac_dbg_info_t *)ctx;
	phy_info_t *pi = di->pi;
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		uint16 phymode = phy_get_phymode(pi);
		uint8 ct;
		PHY_ERROR(("*** [PHY_DBG] *** : wl%d:: PHYMODE: 0x%x\n", pi->sh->unit, phymode));

		if ((phymode == PHYMODE_RSDB) &&
			(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0)) {
			PHY_ERROR(("*** [PHY_DBG] *** : RSDB Cr 0: wl isup: %d\n", pi->sh->up));
		} else if ((phymode == PHYMODE_RSDB) &&
			(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1)) {
			PHY_ERROR(("*** [PHY_DBG] *** : RSDB Cr 1: wl isup: %d\n", pi->sh->up));
		} else {
			PHY_ERROR(("*** [PHY_DBG] *** : MIMO : wl isup: %d\n", pi->sh->up));
		}

		PHY_PRINT(("*** [PHY_DBG] *** : RxFeStatus: 0x%x\n",
			READ_PHYREG(pi, RxFeStatus)));
		PHY_PRINT(("*** [PHY_DBG] *** : TxFIFOStatus0/1: 0x%x 0x%x\n",
			READ_PHYREG(pi, TxFIFOStatus0), READ_PHYREG(pi, TxFIFOStatus1)));
		PHY_PRINT(("*** [PHY_DBG] *** : RfseqMode: 0x%x\n",
			READ_PHYREG(pi, RfseqMode)));
		PHY_PRINT(("*** [PHY_DBG] *** : RfseqStatus0/1: 0x%x 0x%x\n",
			READ_PHYREG(pi, RfseqStatus0), READ_PHYREG(pi, RfseqStatus1)));
		PHY_PRINT(("*** [PHY_DBG] *** : RfseqStatus_Ocl/1: 0x%x 0x%x\n",
			READ_PHYREG(pi, RfseqStatus_Ocl), READ_PHYREG(pi, RfseqStatus_Ocl1)));
		PHY_PRINT(("*** [PHY_DBG] *** : OCLControl1: 0x%x\n",
			READ_PHYREG(pi, OCLControl1)));
		PHY_PRINT(("*** [PHY_DBG] *** : TxError 0x%x bphy 0x%x cck5g 0x%x\n",
			READ_PHYREG(pi, TxError), READ_PHYREG(pi, bphyTxError),
			READ_PHYREG(pi, TxCCKError)));
		PHY_PRINT(("*** [PHY_DBG] *** : TxCtrlWrd0/1/2: 0x%x 0x%x 0x%x\n",
			READ_PHYREG(pi, TxCtrlWrd0), READ_PHYREG(pi, TxCtrlWrd1),
			READ_PHYREG(pi, TxCtrlWrd2)));
		PHY_PRINT(("*** [PHY_DBG] *** : TxLsig0/1: 0x%x 0x%x\n",
			READ_PHYREG(pi, TxLsig0), READ_PHYREG(pi, TxLsig1)));
		PHY_PRINT(("*** [PHY_DBG] *** : TxVhtSigA: 0x%x 0x%x 0x%x 0x%x\n",
			READ_PHYREG(pi, TxVhtSigA10), READ_PHYREG(pi, TxVhtSigA11),
			READ_PHYREG(pi, TxVhtSigA20), READ_PHYREG(pi, TxVhtSigA21)));
		for (ct = 0; ct < 10; ct++) {
			PHY_PRINT(("*** [PHY_DBG] *** : gpio(Hi/Lo)Out: 0x%x 0x%x\n",
				READ_PHYREG(pi, gpioHiOut), READ_PHYREG(pi, gpioLoOut)));
		}
	}
}
#endif /* DNG_DBGDUMP */

#if defined(BCMDBG) || defined(WL_MACDBG)
static void
phy_ac_dbg_gpio_out_enab(phy_type_dbg_ctx_t *ctx, bool enab)
{
	phy_ac_dbg_info_t *di = (phy_ac_dbg_info_t *)ctx;
	phy_info_t *pi = di->pi;

	if (D11REV_LT(pi->sh->corerev, 50)) {
		W_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi), ACPHY_gpioLoOutEn(pi->pubpi->phy_rev));
		W_REG(pi->sh->osh, D11_PHY_REG_DATA(pi), 0);
		W_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi), ACPHY_gpioHiOutEn(pi->pubpi->phy_rev));
		W_REG(pi->sh->osh, D11_PHY_REG_DATA(pi), 0);
	}
}
#endif // endif

#ifdef PHY_DUMP_BINARY
static uint16
phy_ac_dbg_get_phyreg_address(phy_type_dbg_ctx_t *ctx, uint16 addr)
{
	phy_ac_dbg_info_t *di = (phy_ac_dbg_info_t *)ctx;
	phy_info_t *pi = di->pi;
	if (addr == ACPHY_TableDataWide(pi->pubpi->phy_rev)) {
		return 0;
	} else if (addr == ACPHY_TableDataLo(pi->pubpi->phy_rev)) {
		return 0;
	} else if (addr == ACPHY_TableDataHi(pi->pubpi->phy_rev)) {
		return 0;
	} else {
		return phy_utils_read_phyreg(pi, addr);
	}
}
#endif /* PHY_DUMP_BINARY */

#ifdef AWD_EXT_TRAP
static void
phy_ac_dbg_ext_trap(phy_type_dbg_ctx_t *ctx, phy_dbg_ext_trap_err_t *ext_trap_data)
{
	phy_ac_dbg_info_t *di = (phy_ac_dbg_info_t *)ctx;
	phy_info_t *pi = di->pi;
	uint32 i, count, prev_out, gpio_temp;

	ext_trap_data->RxFeStatus = READ_PHYREG(pi, RxFeStatus);
	ext_trap_data->TxFIFOStatus0 = READ_PHYREG(pi, TxFIFOStatus0);
	ext_trap_data->TxFIFOStatus1 = READ_PHYREG(pi, TxFIFOStatus1);
	ext_trap_data->RfseqMode = READ_PHYREG(pi, RfseqMode);
	ext_trap_data->RfseqStatus0 = READ_PHYREG(pi, RfseqStatus0);
	ext_trap_data->RfseqStatus1 = READ_PHYREG(pi, RfseqStatus1);
	ext_trap_data->RfseqStatus_Ocl = READ_PHYREG(pi, RfseqStatus_Ocl);
	ext_trap_data->RfseqStatus_Ocl1 = READ_PHYREG(pi, RfseqStatus_Ocl1);
	ext_trap_data->OCLControl1 = READ_PHYREG(pi, OCLControl1);
	ext_trap_data->TxError = READ_PHYREG(pi, TxError);
	ext_trap_data->bphyTxError = READ_PHYREG(pi, bphyTxError);
	ext_trap_data->TxCCKError = READ_PHYREG(pi, TxCCKError);
	ext_trap_data->TxCtrlWrd0 = READ_PHYREG(pi, TxCtrlWrd0);
	ext_trap_data->TxCtrlWrd1 = READ_PHYREG(pi, TxCtrlWrd1);
	ext_trap_data->TxCtrlWrd2 = READ_PHYREG(pi, TxCtrlWrd2);
	ext_trap_data->TxLsig0 = READ_PHYREG(pi, TxLsig0);
	ext_trap_data->TxLsig1 = READ_PHYREG(pi, TxLsig1);
	ext_trap_data->TxVhtSigA10 = READ_PHYREG(pi, TxVhtSigA10);
	ext_trap_data->TxVhtSigA11 = READ_PHYREG(pi, TxVhtSigA11);
	ext_trap_data->TxVhtSigA20 = READ_PHYREG(pi, TxVhtSigA20);
	ext_trap_data->TxVhtSigA21 = READ_PHYREG(pi, TxVhtSigA21);
	ext_trap_data->txPktLength = READ_PHYREG(pi, txPktLength);
	ext_trap_data->txPsdulengthCtr = READ_PHYREG(pi, txPsdulengthCtr);
	ext_trap_data->gpioClkControl = READ_PHYREG(pi, gpioClkControl);
	ext_trap_data->gpioSel = READ_PHYREG(pi, gpioSel);
	ext_trap_data->pktprocdebug = READ_PHYREG(pi, pktprocdebug);

	gpio_temp = 0;
	prev_out = 0;
	count = 0;
	for (i = 0; i < 10; i++)
	{
		gpio_temp = READ_PHYREG(pi, gpioHiOut) << 16 | READ_PHYREG(pi, gpioLoOut);
		if (count < 3 && ext_trap_data->gpioOut[prev_out] != gpio_temp)
		{
			ext_trap_data->gpioOut[count] = gpio_temp;
			prev_out = count;
			count++;
		}
	}
}
#endif /* AWD_EXT_TRAP */
