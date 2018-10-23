/*
 * ACPHY VASIP modules implementation
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
 * $Id: phy_ac_vasip.c 748133 2018-02-21 19:19:22Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_mem.h>
#include <phy_vasip.h>
#include <phy_vasip_api.h>
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <phy_ac_vasip.h>
#include <wlc_phy_int.h>
/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <phy_utils_var.h>

/* module private states */
struct phy_ac_vasip_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_vasip_info_t *cmn_info;
	uint8	vasipver;
};

/*
 * Return vasip version, -1 if not present.
 */
static uint8 phy_ac_vasip_get_ver(phy_type_vasip_ctx_t *ctx);
/*
 * reset/activate vasip.
 */
static void phy_ac_vasip_reset_proc(phy_type_vasip_ctx_t *ctx, int reset);
static void phy_ac_vasip_set_clk(phy_type_vasip_ctx_t *ctx, bool val);
static void phy_ac_vasip_write_bin(phy_type_vasip_ctx_t *ctx, const uint32 vasip_code[],
	const uint nbytes);
#ifdef VASIP_SPECTRUM_ANALYSIS
static void phy_ac_vasip_write_spectrum_tbl(phy_type_vasip_ctx_t *ctx,
	const uint32 vasip_spectrum_tbl[], const uint nbytes_tbl);
#endif // endif
static void phy_ac_vasip_write_svmp(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 val);
static void phy_ac_vasip_read_svmp(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 *val);
static void phy_ac_vasip_write_svmp_blk(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 len,
	uint16 *val);
static void phy_ac_vasip_read_svmp_blk(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 len,
	uint16 *val);
#if defined(BCMDBG)
static void phy_ac_dump_bfd_status(phy_type_vasip_ctx_t *ctx, struct bcmstrbuf *b);
#endif // endif

/* register phy type specific implementation */
phy_ac_vasip_info_t *
BCMATTACHFN(phy_ac_vasip_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_vasip_info_t *cmn_info)
{
	phy_ac_vasip_info_t *vasip_info;
	phy_type_vasip_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((vasip_info = phy_malloc(pi, sizeof(phy_ac_vasip_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	vasip_info->pi = pi;
	vasip_info->aci = aci;
	vasip_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = vasip_info;
	fns.get_ver = phy_ac_vasip_get_ver;
	fns.reset_proc = phy_ac_vasip_reset_proc;
	fns.set_clk = phy_ac_vasip_set_clk;
	fns.write_bin = phy_ac_vasip_write_bin;
#ifdef VASIP_SPECTRUM_ANALYSIS
	fns.write_spectrum_tbl = phy_ac_vasip_write_spectrum_tbl;
#endif // endif
	fns.write_svmp = phy_ac_vasip_write_svmp;
	fns.read_svmp = phy_ac_vasip_read_svmp;
	fns.write_svmp_blk = phy_ac_vasip_write_svmp_blk;
	fns.read_svmp_blk = phy_ac_vasip_read_svmp_blk;
#if defined(BCMDBG)
	fns.dump_bfd_status = phy_ac_dump_bfd_status;
#endif // endif

	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		vasip_info->vasipver =
			READ_PHYREGFLD(pi, MinorVersion, vasipversion);
	} else if (ACMAJORREV_GE32(pi->pubpi->phy_rev)) {
		vasip_info->vasipver = READ_PHYREGFLD(pi, PhyCapability2, vasipPresent) ?
			READ_PHYREGFLD(pi, MinorVersion, vasipversion) : VASIP_NOVERSION;
	} else {
		vasip_info->vasipver = VASIP_NOVERSION;
	}

	phy_vasip_register_impl(cmn_info, &fns);

	return vasip_info;

	/* error handling */
fail:
	if (vasip_info != NULL)
		phy_mfree(pi, vasip_info, sizeof(phy_ac_vasip_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_vasip_unregister_impl)(phy_ac_vasip_info_t *vasip_info)
{
	phy_info_t *pi;
	phy_vasip_info_t *cmn_info;

	ASSERT(vasip_info);
	pi = vasip_info->pi;
	cmn_info = vasip_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_vasip_unregister_impl(cmn_info);

	phy_mfree(pi, vasip_info, sizeof(phy_ac_vasip_info_t));
}

/*
 * Return vasip version, -1 if not present.
 */
static uint8
phy_ac_vasip_get_ver(phy_type_vasip_ctx_t *ctx)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;

	return info->vasipver;
}

/*
 * reset/activate vasip.
 */
static void
phy_ac_vasip_reset_proc(phy_type_vasip_ctx_t *ctx, int reset)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 reset_val = 1;
	uint8  stall_val;
	uint16 vasipregisters_reset_offset, vasipregisters_set_offset;

	vasipregisters_reset_offset = VASIPREGISTERS_RESET;
	vasipregisters_set_offset = VASIPREGISTERS_SET;

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		vasipregisters_reset_offset >>= 2;
		vasipregisters_set_offset >>= 2;
	}

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	if (reset) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS,
			1, vasipregisters_reset_offset, 32, &reset_val);
	} else {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS,
			1, vasipregisters_set_offset, 32, &reset_val);
		pi->vasipi->active = 1;
	}

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

void
phy_ac_vasip_clk_enable(si_t *sih, bool val)
{
	uint32 acphyidx = si_findcoreidx(sih, ACPHY_CORE_ID, 0);

	if (val) {
		/* enter reset */
		si_core_wrapperreg(sih, acphyidx,
			OFFSETOF(aidmp_t, resetctrl), ~0, 0x1);
		/* enable vasip clock */
		si_core_wrapperreg(sih, acphyidx,
			OFFSETOF(aidmp_t, ioctrl), ~0, 0x1);
		/* exit reset */
		si_core_wrapperreg(sih, acphyidx,
			OFFSETOF(aidmp_t, resetctrl), ~0, 0x0);
	} else {
		/* disable vasip clock */
		si_core_wrapperreg(sih, acphyidx,
			OFFSETOF(aidmp_t, ioctrl), ~0, 0x0);
	}
}

static void
phy_ac_vasip_set_clk(phy_type_vasip_ctx_t *ctx, bool val)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;

	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		phy_ac_vasip_clk_enable(pi->sh->sih, val);
	} else {
		MOD_PHYREG(pi, dacClkCtrl, vasipClkEn, val);
	}
}

static void
phy_ac_vasip_write_bin(phy_type_vasip_ctx_t *ctx, const uint32 vasip_code[], const uint nbytes)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8	stall_val, mem_id;
	uint32	count;
	uint32 svmp_addr = 0x0;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	mem_id = 0;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);

	count = (nbytes/sizeof(uint32));
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, count, svmp_addr, 32, &vasip_code[0]);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

#ifdef VASIP_SPECTRUM_ANALYSIS
static void
phy_ac_vasip_write_spectrum_tbl(phy_type_vasip_ctx_t *ctx,
        const uint32 vasip_tbl_code[], const uint nbytes_tbl)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8  stall_val, mem_id_tbl;
	uint32 count_tbl;
	uint32 svmp_tbl_addr = 0x3400; // (0x26800-0x8000*4)>>1

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	mem_id_tbl = 4;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id_tbl);

	count_tbl = (nbytes_tbl/sizeof(uint32));
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, count_tbl, svmp_tbl_addr, 32,
		&vasip_tbl_code[0]);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* VASIP_SPECTRUM_ANALYSIS */

static void
phy_ac_vasip_write_svmp(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 val)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint8  stall_val, mem_id, odd_even;

	mem_id = offset/0x8000;
	offset = offset%0x8000;

	odd_even = offset%2;
	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
	if (odd_even) {
		tbl_val = tbl_val & 0xffff;
		tbl_val = tbl_val | (uint32) (val << NBITS(uint16));
	} else {
		tbl_val = tbl_val & (0xffff << NBITS(uint16));
		tbl_val = tbl_val | (uint32) (val);
	}
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

static void
phy_ac_vasip_read_svmp(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 *val)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint8 stall_val, mem_id, odd_even;

	mem_id = offset/0x8000;
	offset = offset%0x8000;

	odd_even = offset%2;
	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	*val = odd_even ? ((tbl_val>> NBITS(uint16)) & 0xffff): (tbl_val & 0xffff);
}

static void
phy_ac_vasip_write_svmp_blk(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 len, uint16 *val)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint8  stall_val, mem_id;
	uint16 n, odd_start, odd_end;

	mem_id = offset / 0x8000;
	offset = offset % 0x8000;

	odd_start = offset % 2;
	odd_end = (offset + len) % 2;

	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);

	if (odd_start == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		tbl_val &= 0xffff;
		tbl_val |= ((uint32)val[0] << NBITS(uint16));
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		offset += 1;
	}
	for (n = odd_start; n < (len-odd_start-odd_end); n += 2) {
		tbl_val  = ((uint32)val[n+1] << NBITS(uint16));
		tbl_val |= ((uint32)val[n]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		offset += 1;
	}
	if (odd_end == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		tbl_val &= (0xffff << NBITS(uint16));
		tbl_val |= ((uint32)val[len-1]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
	}

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

static void
phy_ac_vasip_read_svmp_blk(phy_type_vasip_ctx_t *ctx, uint32 offset, uint16 len, uint16 *val)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint8 stall_val, mem_id;
	uint16 n, odd_start, odd_end;

	mem_id = offset / 0x8000;
	offset = offset % 0x8000;

	odd_start = offset % 2;
	odd_end = (offset + len) % 2;

	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);

	if (odd_start == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		val[0] = ((tbl_val >> NBITS(uint16)) & 0xffff);
		offset += 1;
	}
	for (n = odd_start; n < (len-odd_start-odd_end); n += 2) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		val[n]   = (tbl_val & 0xffff);
		val[n+1] = ((tbl_val >> NBITS(uint16)) & 0xffff);
		offset += 1;
	}
	if (odd_end == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		val[len-1] = (tbl_val & 0xffff);
	}

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

#if defined(BCMDBG)
static void phy_ac_dump_bfd_status(phy_type_vasip_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ac_vasip_info_t *info = (phy_ac_vasip_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8 stall_val;
	uint32 commandIndex, command, tmp_val, tbl_val[4];
	uint16 bfdrRunning, bfdrDone, bfdrFailed, rptDestIndex, bfdsLogAddr, bfdrCommandIndex;
	uint16 val0, val1, rptFailed, m, n, usedSlot, slot_count, bfdsCommandIndex;
	uint16 frameDone, steeringDone, steeringRunning, steeringCommandBusy, badReport, badCommand;
	int local_commandIndex;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	bfdsLogAddr = READ_PHYREG(pi, bfdsLogAddr);
	/* BFDR STATUS */
	rptDestIndex = READ_PHYREGFLD(pi, bfdrStatus1, rptDestIndex);
	bfdrCommandIndex = READ_PHYREGFLD(pi, bfdrStatus1, rptCommandIndex);
	bfdrRunning = READ_PHYREG(pi, bfdrRunning);
	bfdrDone = READ_PHYREG(pi, bfdrDone);
	bfdrFailed = READ_PHYREG(pi, bfdrFailed);
	bcm_bprintf(b, "===== BFDR STATUS =====\n");
	bcm_bprintf(b, "Destination index of most recently loaded report = %d \n", rptDestIndex);
	bcm_bprintf(b, "In below, each bit repsents a fifo:\n");
	bcm_bprintf(b, "bitmapping of BFDR Running     = 0x%04x \n", bfdrRunning);
	bcm_bprintf(b, "bitmapping of BFDR Done        = 0x%04x \n", bfdrDone);
	bcm_bprintf(b, "bitmapping of BFDR Failed is   = 0x%04x \n", bfdrFailed);

	rptFailed = READ_PHYREGFLD(pi, bfdrStatus0, rptFailed);
	if (rptFailed) {
		commandIndex = READ_PHYREGFLD(pi, bfdrStatus2, rptFailedCommandIndex);
		commandIndex = commandIndex*2+ 512 + bfdsLogAddr*16;
		phy_ac_vasip_read_svmp(ctx, commandIndex, &val0);
		phy_ac_vasip_read_svmp(ctx, commandIndex+1, &val1);
		command = val0 + (val1 << 16);
		bcm_bprintf(b, "command of the failed  report  = 0x%08x \n", command);
	}

	/* BFDS STATUS */
	frameDone = READ_PHYREGFLD(pi, bfdsStatus0, frameDone);
	steeringDone = READ_PHYREGFLD(pi, bfdsStatus0, steeringDone);
	steeringRunning = READ_PHYREGFLD(pi, bfdsStatus0, steeringRunning);
	steeringCommandBusy = READ_PHYREGFLD(pi, bfdsStatus0, steeringCommandBusy);
	bfdsCommandIndex = READ_PHYREGFLD(pi, bfdsStatus0, steeringCommandIndex);
	bcm_bprintf(b, "\n===== BFDS STATUS =====\n");
	bcm_bprintf(b, "frameDone                      = %d \n", frameDone);
	bcm_bprintf(b, "steeringDone                   = %d \n", steeringDone);
	bcm_bprintf(b, "steeringrunning                = %d \n", steeringRunning);
	bcm_bprintf(b, "steeringCommandBusy            = %d \n", steeringCommandBusy);

	badReport = READ_PHYREGFLD(pi, bfdsStatus0, illegalRecord);
	badCommand = READ_PHYREGFLD(pi, bfdsStatus0, illegalCommand);
	if (badReport) {
		commandIndex = READ_PHYREGFLD(pi, bfdsStatus1, illegalRecordIndex);
		commandIndex = commandIndex*2 + bfdsLogAddr*16;
		phy_ac_vasip_read_svmp(ctx, commandIndex, &val0);
		phy_ac_vasip_read_svmp(ctx, commandIndex+1, &val1);
		command = val0 + (val1 << 16);
		bcm_bprintf(b, "command containing bad report = 0x%08x \n", command);
	}

	if (badCommand) {
		commandIndex = READ_PHYREGFLD(pi, bfdsStatus1, illegalCommandIndex);
		commandIndex = commandIndex*2 + bfdsLogAddr*16;
		phy_ac_vasip_read_svmp(ctx, commandIndex, &val0);
		phy_ac_vasip_read_svmp(ctx, commandIndex+1, &val1);
		command = val0 + (val1 << 16);
		bcm_bprintf(b, "the last illegal bfds command  = 0x%08x \n", command);
	}

	/* Dump BFDR/BFDS Commands */
	bcm_bprintf(b, "\n===== DUMPING BFD COMMANDS =====\n");
	bcm_bprintf(b, "Last 16 BFDR commands are: \n");
	for (m = 0; m < 16; m++) {
		local_commandIndex = bfdrCommandIndex -m;
		local_commandIndex = (local_commandIndex < 0) ? (local_commandIndex + 256)
			: local_commandIndex;
		commandIndex = local_commandIndex*2+ 512 + bfdsLogAddr*16;
		phy_ac_vasip_read_svmp(ctx, commandIndex, &val0);
		phy_ac_vasip_read_svmp(ctx, commandIndex+1, &val1);
		command = val0 + (val1 << 16);
		bcm_bprintf(b, "0x%08x \n", command);
	}

	bcm_bprintf(b, "Last 16 BFDs commands are: \n");
	for (m = 0; m < 16; m++) {
		local_commandIndex = bfdsCommandIndex -m;
		local_commandIndex = (local_commandIndex < 0) ? (local_commandIndex + 256)
			: local_commandIndex;
		commandIndex = local_commandIndex*2 + bfdsLogAddr*16;
		phy_ac_vasip_read_svmp(ctx, commandIndex, &val0);
		phy_ac_vasip_read_svmp(ctx, commandIndex+1, &val1);
		command = val0 + (val1 << 16);
		bcm_bprintf(b, "0x%08x \n", command);
	}

	/* Print TXV status */
	bcm_bprintf(b, "\n===== TXV MEMORY USAGE =====\n");
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_BFDINDEXLUT, 4, 224, 32, &tbl_val[0]);
	usedSlot = 0;
	for (m = 0; m < 4; m++) {
		tmp_val = tbl_val[m];
		while (tmp_val > 0) {
			if ((tmp_val & 1) == 0)
				usedSlot ++;
			tmp_val  = tmp_val >> 1;
		}
	}
	bcm_bprintf(b, "-----%d slots are used----- \n", usedSlot);
	for (m = 0; m < 64; m++) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_BFDINDEXLUT, 2, 2*m, 32, &tbl_val[0]);
		slot_count = tbl_val[0] & 0x7;
		if (slot_count > 0) {
			bcm_bprintf(b, "txv index %d has %d slots:   ", m, slot_count);
			for (n = 0; n < slot_count; n++) {
				if (n < 4)
					tmp_val = (tbl_val[0] >> (3+ n*7)) & 0x7f;
				if (n == 4)
					tmp_val = ((tbl_val[0] >> 31) & 0x1)
						+ (tbl_val[1] & 0x3f) * 2;
				if (n > 4)
					tmp_val = (tbl_val[1] >> (6+ (n-5)*7)) & 0x7f;
				bcm_bprintf(b, "%03d ", tmp_val);
			}
			bcm_bprintf(b, "\n");
		}
	}

	/* clear BFDr/BFDs status */
	WRITE_PHYREG(pi, bfdrCtrl1, 0xffff);
	MOD_PHYREG(pi, bfdsConfig, clrBfdsStatus, 1);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif // endif
