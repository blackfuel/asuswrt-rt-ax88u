/*
 * lcn20PHY Core module implementation
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
 * $Id: phy_lcn20.c 662291 2016-09-29 02:58:11Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_rstr.h>
#include <phy_lcn20_ana.h>
#include <phy_lcn20_chanmgr.h>
#include <phy_lcn20_radio.h>
#include <phy_lcn20_tbl.h>
#include <phy_lcn20_tpc.h>
#include <phy_lcn20_noise.h>
#include <phy_lcn20_antdiv.h>
#include <phy_lcn20_rssi.h>
#include <phy_lcn20_cache.h>
#include <phy_lcn20_misc.h>
#include <phy_lcn20_rxspur.h>
#include <phy_lcn20_txiqlocal.h>
#include <phy_lcn20_lpc.h>

#include "phy_type.h"
#include "phy_type_lcn20.h"
#include "phy_type_lcn20_iovt.h"
#include "phy_type_lcn20_ioct.h"
#include "phy_shared.h"
#include <phy_lcn20.h>

#include <phy_utils_radio.h>
#include <phy_utils_var.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
/* TODO: all these are going away... > */
#endif // endif

#include <phy_lcn20_tssical.h>

/* local functions */
static int phy_lcn20_attach_ext(phy_info_t *pi, int bandtype);
static int phy_lcn20_register_impl(phy_info_t *pi, phy_type_info_t *ti, int bandtype);
static void phy_lcn20_unregister_impl(phy_info_t *pi, phy_type_info_t *ti);
#if defined(BCMDBG) && defined(DBG_PHY_IOV)
static int phy_lcn20_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b);
#else
#define	phy_lcn20_dump_phyregs	NULL
#endif // endif

/* attach/detach */
phy_type_info_t *
BCMATTACHFN(phy_lcn20_attach)(phy_info_t *pi, int bandtype)
{
	phy_lcn20_info_t *lcn20i;
	phy_type_fns_t fns;
	uint32 idcode;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* Extend phy_attach() here to initialize lcn20PHY specific stuff */
	if (phy_lcn20_attach_ext(pi, bandtype) != BCME_OK) {
		PHY_ERROR(("%s: phy_lcn20_attach_ext failed\n", __FUNCTION__));
		return NULL;
	}

	/* read idcode */
	idcode = phy_lcn20_radio_query_idcode(pi);
	PHY_TRACE(("%s: idcode 0x%08x\n", __FUNCTION__, idcode));
	/* parse idcode */
	phy_lcn20_radio_parse_idcode(pi, idcode);
	/* validate radio id */
	if (phy_utils_valid_radio(pi) != BCME_OK) {
		PHY_ERROR(("%s: phy_utils_valid_radio failed\n", __FUNCTION__));
		return NULL;
	}

	/* TODO: move the acphy attach code to here... */
	if (wlc_phy_attach_lcn20phy(pi) == FALSE) {
		PHY_ERROR(("%s: wlc_phy_attach_lcn20phy failed\n", __FUNCTION__));
		return NULL;
	}
	lcn20i = pi->u.pi_lcn20phy;
	lcn20i->pi = pi;

	/* register PHY type implementation entry points */
	bzero(&fns, sizeof(fns));
	fns.reg_impl = phy_lcn20_register_impl;
	fns.unreg_impl = phy_lcn20_unregister_impl;
	fns.reg_iovt = phy_lcn20_register_iovt;
	fns.reg_ioct = phy_lcn20_register_ioct;
	fns.dump_phyregs = phy_lcn20_dump_phyregs;
	fns.ti = (phy_type_info_t *)lcn20i;

	phy_register_impl(pi, &fns);

	return (phy_type_info_t *)lcn20i;
}

void
BCMATTACHFN(phy_lcn20_detach)(phy_type_info_t *ti)
{
	phy_lcn20_info_t *lcn20i = (phy_lcn20_info_t *)ti;
	phy_info_t *pi = lcn20i->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_detach_lcn20phy(pi);

	phy_mfree(pi, lcn20i, sizeof(phy_lcn20_info_t));
}

static int
BCMATTACHFN(phy_lcn20_attach_ext)(phy_info_t *pi, int bandtype)
{
	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	pi->min_txpower = LCN20PHY_TXPWR_MIN;
	pi->tx_pwr_backoff = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_txpwrbckof, 4);
	pi->rssi_corr_boardatten = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssicorratten, 0);

	if (CHIPREV(pi->sh->chiprev) == 0)
			pi->ldpc_en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_ldpc, 0);
		else
			pi->ldpc_en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_ldpc, 1);

	pi->phynoise_polling = FALSE;

	return BCME_OK;
}

/* Register/unregister lcn20PHY specific implementations to their commons.
 * Used to configure features/modules implemented for lcn20PHY.
 */
static int
BCMATTACHFN(phy_lcn20_register_impl)(phy_info_t *pi, phy_type_info_t *ti, int bandtype)
{

	phy_lcn20_info_t *lcn20i = (phy_lcn20_info_t *)ti;

	PHY_TRACE(("%s: band %d\n", __FUNCTION__, bandtype));

	/* Register with ANAcore control module */
	if (pi->anai != NULL &&
	    (lcn20i->anai = phy_lcn20_ana_register_impl(pi, lcn20i, pi->anai)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_ana_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
	/* Register with RADIO control module */
	if (pi->radioi != NULL &&
	    (lcn20i->radioi = phy_lcn20_radio_register_impl(pi, lcn20i, pi->radioi)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_radio_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
	/* Register with PHYTableInit module */
	if (pi->tbli != NULL &&
	    (lcn20i->tbli = phy_lcn20_tbl_register_impl(pi, lcn20i, pi->tbli)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_tbl_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with TxPowerCtrl module */
	if (pi->tpci != NULL &&
		(lcn20i->tpci = phy_lcn20_tpc_register_impl(pi, lcn20i, pi->tpci)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_tpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with ANTennaDIVersity module */
	if (pi->antdivi != NULL &&
		(lcn20i->antdivi =
			phy_lcn20_antdiv_register_impl(pi, lcn20i, pi->antdivi)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_antdiv_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

#ifndef WLC_DISABLE_ACI
	/* Register with INTerFerence module */
	if (pi->noisei != NULL &&
		(lcn20i->noisei = phy_lcn20_noise_register_impl(pi, lcn20i, pi->noisei)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_noise_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register with RSSICompute module */
	if (pi->rssii != NULL &&
		(lcn20i->rssii = phy_lcn20_rssi_register_impl(pi, lcn20i, pi->rssii)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_rssi_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with CACHE Compute module */
	if (pi->cachei != NULL &&
		(lcn20i->cachei = phy_lcn20_cache_register_impl(pi, lcn20i, pi->cachei)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_cache_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with MISC module */
	if (pi->misci != NULL &&
		(lcn20i->misci = phy_lcn20_misc_register_impl(pi, lcn20i, pi->misci)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_misc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with Channel Manager module */
	if (pi->chanmgri != NULL &&
		(lcn20i->chanmgri = phy_lcn20_chanmgr_register_impl(pi, lcn20i, pi->chanmgri)) ==
		NULL) {
		PHY_ERROR(("%s: phy_lcn20_chanmgr_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with TXIQLO CAL module */
	if (pi->txiqlocali != NULL &&
		(lcn20i->txiqlocali = phy_lcn20_txiqlocal_register_impl(pi, lcn20i,
		pi->txiqlocali)) ==	NULL) {
		PHY_ERROR(("%s: phy_lcn20_txiqlocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with Link Power Control module */
	if (pi->lpci != NULL &&
		(lcn20i->lpci = phy_lcn20_lpc_register_impl(pi, lcn20i, pi->lpci)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_lpc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register with Rx Spur canceller control module */
	if (pi->rxspuri != NULL &&
	    (lcn20i->rxspuri = phy_lcn20_rxspur_register_impl(pi, lcn20i, pi->rxspuri)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_rxspur_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register the tssi cal module. */
	if (pi->tssicali != NULL &&
			(lcn20i->tssicali = phy_lcn20_tssical_register_impl(pi, lcn20i,
			pi->tssicali)) == NULL) {
		PHY_ERROR(("%s: phy_lcn20_tssical_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* ...Add your module registration here... */

	return BCME_OK;

fail:
	return BCME_ERROR;
}

static void
BCMATTACHFN(phy_lcn20_unregister_impl)(phy_info_t *pi, phy_type_info_t *ti)
{
	phy_lcn20_info_t *lcn20i = (phy_lcn20_info_t *)ti;
	BCM_REFERENCE(pi);

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* ...Add your module registration here... */

	/* Unregister from tssical module */
	if (lcn20i->tssicali != NULL)
		phy_lcn20_tssical_unregister_impl(lcn20i->tssicali);

	/* Unregister from Rx Spur canceller module */
	if (lcn20i->rxspuri != NULL)
		phy_lcn20_rxspur_unregister_impl(lcn20i->rxspuri);

	/* Unregister from Link Power Control module */
	if (lcn20i->lpci != NULL)
		phy_lcn20_lpc_unregister_impl(lcn20i->lpci);

	/* Unregister from TXIQLO CAL module */
	if (lcn20i->txiqlocali != NULL)
		phy_lcn20_txiqlocal_unregister_impl(lcn20i->txiqlocali);

	/* Unregister from Channel Manager module */
	if (lcn20i->chanmgri != NULL)
		phy_lcn20_chanmgr_unregister_impl(lcn20i->chanmgri);

	/* Unregister from MISC module */
	if (lcn20i->misci != NULL)
		phy_lcn20_misc_unregister_impl(lcn20i->misci);

	/* Unregister from CACHE module */
	if (lcn20i->cachei != NULL)
		phy_lcn20_cache_unregister_impl(lcn20i->cachei);

	/* Unregister from ANAcore control module */
	if (lcn20i->anai != NULL)
		phy_lcn20_ana_unregister_impl(lcn20i->anai);

	/* Unregister from RADIO control module */
	if (lcn20i->radioi != NULL)
		phy_lcn20_radio_unregister_impl(lcn20i->radioi);

	/* Unregister from PHYTableInit module */
	if (lcn20i->tbli != NULL)
		phy_lcn20_tbl_unregister_impl(lcn20i->tbli);

	/* Unregister from TxPowerCtrl module */
	if (lcn20i->tpci != NULL)
		phy_lcn20_tpc_unregister_impl(lcn20i->tpci);

	/* Unregister from ANTennaDIVersity module */
	if (lcn20i->antdivi != NULL)
		phy_lcn20_antdiv_unregister_impl(lcn20i->antdivi);

#ifndef WLC_DISABLE_ACI
	/* Unregister from INTerFerence module */
	if (lcn20i->noisei != NULL)
		phy_lcn20_noise_unregister_impl(lcn20i->noisei);
#endif // endif

	/* Unregister from RSSICompute module */
	if (lcn20i->rssii != NULL)
		phy_lcn20_rssi_unregister_impl(lcn20i->rssii);

}

#if defined(BCMDBG)
#if defined(DBG_PHY_IOV)
static phy_regs_t lcn20phy3_regs[] = {
	{ 0x000,	0x002 },	/* 0x000 - 0x001 */
	{ 0x004,	0x002 },	/* 0x004 - 0x005 */
	{ 0x007,	0x001 },	/* 0x007 */
	{ 0x007,	0x004 },	/* 0x009 - 0x00c */
	{ 0x010,	0x001 },	/* 0x010 */
	{ 0x012,	0x002 },	/* 0x012 - 0x013 */
	{ 0x018,	0x002 },	/* 0x018 - 0x019 */
	{ 0x027,	0x001 },	/* 0x027 */
	{ 0x030,	0x002 },	/* 0x030 - 0x031 */
	{ 0x033,	0x003 },	/* 0x033 - 0x035 */
	{ 0x038,	0x002 },	/* 0x038 - 0x039 */
	{ 0x03d,	0x012 },	/* 0x03d - 0x04d */
	{ 0x04f,	0x001 },	/* 0x04f */
	{ 0x052,	0x002 },	/* 0x052 - 0x053 */
	{ 0x05d,	0x003 },	/* 0x05d - 0x05f */
	{ 0x068,	0x006 },	/* 0x068 - 0x006 */
	{ 0x070,	0x003 },	/* 0x070 - 0x072 */
	{ 0x0d9,	0x005 },	/* 0x0d9 - 0x0dd */
	{ 0x400,	0x00f },	/* 0x400 - 0x40e */
	{ 0x410,	0x02a },	/* 0x410 - 0x439 */
	{ 0x43b,	0x001 },	/* 0x43b */
	{ 0x440,	0x028 },	/* 0x440 - 0x467 */
	{ 0x469,	0x006 },	/* 0x469 - 0x46e */
	{ 0x470,	0x00b },	/* 0x470 - 0x47a */
	{ 0x47f,	0x017 },	/* 0x47f - 0x495 */
	{ 0x498,	0x001 },	/* 0x498 */
	{ 0x49a,	0x004 },	/* 0x49a - 0x49d */
	{ 0x4a2,	0x00c },	/* 0x4a2 - 0x4ad */
	{ 0x4b0,	0x002 },	/* 0x4b0 - 0x4b1 */
	{ 0x4b5,	0x001 },	/* 0x4b5 */
	{ 0x4b9,	0x008 },	/* 0x4b9 - 0x4c0 */
	{ 0x4d7,	0x00c },	/* 0x4d7 - 0x4e2 */
	{ 0x4e4,	0x008 },	/* 0x4e4 - 0x4eb */
	{ 0x4f0,	0x003 },	/* 0x4f0 - 0x4f2 */
	{ 0x4f9,	0x003 },	/* 0x4f9 - 0x4fb */
	{ 0x4fe,	0x004 },	/* 0x4fe - 0x501 */
	{ 0x503,	0x001 },	/* 0x503 */
	{ 0x506,	0x005 },	/* 0x506 - 0x50a */
	{ 0x50c,	0x003 },	/* 0x50c - 0x50e */
	{ 0x512,	0x001 },	/* 0x512 */
	{ 0x514,	0x035 },	/* 0x514 - 0x547 */
	{ 0x54b,	0x01e },	/* 0x54b - 0x568 */
	{ 0x570,	0x00b },	/* 0x570 - 0x580 */
	{ 0x583,	0x004 },	/* 0x583 - 0x586 */
	{ 0x589,	0x001 },	/* 0x589 */
	{ 0x591,	0x001 },	/* 0x591 */
	{ 0x593,	0x00b },	/* 0x593 - 0x59d */
	{ 0x5a1,	0x019 },	/* 0x5a1 - 0x5b9 */
	{ 0x5bb,	0x00f },	/* 0x5bb - 0x5c9 */
	{ 0x5cf,	0x006 },	/* 0x5cf - 0x5d4 */
	{ 0x5e0,	0x00e },	/* 0x5e0 - 0x5ed */
	{ 0x5f0,	0x014 },	/* 0x5f0 - 0x603 */
	{ 0x606,	0x023 },	/* 0x606 - 0x628 */
	{ 0x62a,	0x006 },	/* 0x62a - 0x62f */
	{ 0x631,	0x001 },	/* 0x631 */
	{ 0x634,	0x005 },	/* 0x634 - 0x638 */
	{ 0x63a,	0x050 },	/* 0x63a - 0x689 */
	{ 0x68b,	0x002 },	/* 0x68b - 0x68c */
	{ 0x690,	0x009 },	/* 0x690 - 0x698 */
	{ 0x69d,	0x001 },	/* 0x69d - 0x4eb */
	{ 0x6a1,	0x007 },	/* 0x6a1 - 0x6a7 */
	{ 0x6b2,	0x006 },	/* 0x6b2 - 0x6b7 */
	{ 0x6ba,	0x004 },	/* 0x6ba - 0x6bd */
	{ 0x6c2,	0x004 },	/* 0x6c2 - 0x6c5 */
	{ 0x6c8,	0x00f },	/* 0x6c8 - 0x6d6 */
	{ 0x6d8,	0x004 },	/* 0x6d8 - 0x6db */
	{ 0x6e1,	0x007 },	/* 0x6e1 - 0x6e7 */
	{ 0x6e2,	0x003 },	/* 0x6f0 - 0x6f2 */
	{ 0x775,	0x002 },	/* 0x775 - 0x776 */
	{ 0x77a,	0x003 },	/* 0x77a - 0x77c */
	{ 0x780,	0x00a },	/* 0x780 - 0x789 */
	{ 0x790,	0x00a },	/* 0x790 - 0x799 */
	{ 0x7c2,	0x005 },	/* 0x7c2 - 0x7c6 */
	{ 0x7d1,	0x006 },	/* 0x7d1 - 0x7d6 */
	{ 0x800,	0x001 },	/* 0x800 */
	{ 0x803,	0x007 },	/* 0x803 - 0x809 */
	{ 0x810,	0x003 },	/* 0x810 - 0x812 */
	{ 0x820,	0x00a },	/* 0x820 - 0x829 */
	{ 0x830,	0x003 },	/* 0x830 - 0x832 */
	{ 0x86c,	0x002 },	/* 0x86c - 0x86d */
	{ 0x87a,	0x001 },	/* 0x87a */
	{ 0x880,	0x002 },	/* 0x880 - 0x881 */
	{ 0x890,	0x001 },	/* 0x890 */
	{ 0x900,	0x013 },	/* 0x900 - 0x912 */
	{ 0x91e,	0x00c },	/* 0x91e - 0x929 */
	{ 0x930,	0x00a },	/* 0x930 - 0x939 */
	{ 0x93d,	0x005 },	/* 0x93d - 0x941 */
	{ 0x944,	0x006 },	/* 0x944 - 0x949 */
	{ 0x950,	0x008 },	/* 0x950 - 0x957 */
	{ 0x960,	0x004 },	/* 0x960 - 0x963 */
	{ 0xa00,	0x001 },	/* 0xa00 */
	{ 0xa04,	0x00d },	/* 0xa04 - 0xa10 */
	{ 0xa13,	0x07b },	/* 0xa13 - 0xa8d */
};

static int
phy_lcn20_dump_phyregs(phy_info_t *pi, phy_type_info_t *ti, struct bcmstrbuf *b)
{
	phy_regs_t *rl;
	BCM_REFERENCE(ti);

	rl = lcn20phy3_regs;
	phy_dump_phyregs(pi, "lcn20phy", rl, 0, b);

	return BCME_OK;
}
#endif // endif
#endif // endif
