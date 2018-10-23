/*
 * TSSI Cal module implementation.
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
 * $Id: phy_tssical.c 739019 2018-01-04 20:00:32Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_utils_reg.h>
#include "phy_type_tssical.h"
#include <phy_rstr.h>
#include <phy_tssical_api.h>
#include <phy_utils_var.h>
#include <phy_misc_api.h>

#define PHY_TXFIFO_END_BLK_REV35	(0x7900 >> 2)

/* forward declaration */
typedef struct phy_tssical_mem phy_tssical_mem_t;

/* module private states */
struct phy_tssical_info {
	phy_info_t		*pi;	/* PHY info ptr */
	phy_type_tssical_fns_t	*fns;	/* PHY specific function ptrs */
	phy_tssical_mem_t	*mem;	/* Memory layout ptr */
	wl_txcal_power_tssi_t	*txcal_pwr_tssi;
	wl_txcal_meas_t		*txcal_meas;
	int8			txcal_pwr_tssi_tbl_count;
	txcal_root_pwr_tssi_t	*txcal_root_pwr_tssi_tbl;
	bool			txcal_pwr_tssi_tbl_in_use;
	uint8			txcal_status;
	int8			disable_olpc;
	int8			olpc_anchor2g;
	int8			olpc_anchor5g;
	int8			olpc_thresh;
	uint8			olpc_anchor_idx[PHY_CORE_MAX];
	uint8			olpc_idx_valid;
	uint8			olpc_idx_in_use;
	int16			olpc_tempsense[PHY_CORE_MAX];
	int16			olpc_tempslope2g[PHY_CORE_MAX];
	int16			olpc_tempslope5g[PHY_CORE_MAX];
	bool			olpc_thresh_iovar_ovr;
	int8			olpc_thresh2g;
	int8			olpc_thresh5g;
	int8			olpc_offset[5];
	int16			txcal_olpc_last_calc_temp;
};

/* module private states memory layout */
struct phy_tssical_mem {
	phy_tssical_info_t	cmn_info;
	phy_type_tssical_fns_t	fns;
#ifdef WLC_TXCAL
	wl_txcal_power_tssi_t	txcal_pwr_tssi;
	wl_txcal_meas_t		txcal_meas;
	/* obsolete member to keep ROM compatibility */
	txcal_root_pwr_tssi_old_t	txcal_root_pwr_tssi_tbl_unused;
	txcal_pwr_tssi_lut_t	root_pwr_tssi_lut_2G;
	txcal_pwr_tssi_lut_t	root_pwr_tssi_lut_5G20;
	txcal_pwr_tssi_lut_t	root_pwr_tssi_lut_5G40;
	txcal_pwr_tssi_lut_t	root_pwr_tssi_lut_5G80;
	wl_txcal_power_tssi_t	txcal_pwr_tssi_2G;
	wl_txcal_power_tssi_t	txcal_pwr_tssi_5G20;
	wl_txcal_power_tssi_t	txcal_pwr_tssi_5G40;
	wl_txcal_power_tssi_t	txcal_pwr_tssi_5G80;
	txcal_pwr_tssi_lut_t	root_pwr_tssi_lut_2G_bphy;
	wl_txcal_power_tssi_t	txcal_pwr_tssi_2G_bphy;
	txcal_root_pwr_tssi_t	txcal_root_pwr_tssi_tbl;

#endif /* WLC_TXCAL */
/* add other variable size variables here at the end */
};

/* local function declaration */
#ifdef WLC_TXCAL
static void
phy_tssical_read_olpc_params(phy_info_t *pi, phy_tssical_info_t *cmn_info);
#endif /* WLC_TXCAL */

/* attach/detach */
phy_tssical_info_t *
BCMATTACHFN(phy_tssical_attach)(phy_info_t *pi)
{
	phy_tssical_mem_t		*mem = NULL;
	phy_tssical_info_t		*cmn_info = NULL;

#ifdef WLC_TXCAL
	txcal_root_pwr_tssi_t *pi_txcal_tssi_tbl;
	txcal_pwr_tssi_lut_t *pwr_tssi_lut_5G80, *pwr_tssi_lut_5G40, *pwr_tssi_lut_5G20;
	txcal_pwr_tssi_lut_t *pwr_tssi_lut_2G_bphy, *pwr_tssi_lut_2G;
#endif /* WLC_TXCAL */

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_tssical_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

#ifdef WLC_TXCAL
	cmn_info->txcal_pwr_tssi = &(mem->txcal_pwr_tssi);
	cmn_info->txcal_meas = &(mem->txcal_meas);

	pi_txcal_tssi_tbl = cmn_info->txcal_root_pwr_tssi_tbl = &(mem->txcal_root_pwr_tssi_tbl);

	pwr_tssi_lut_2G_bphy = pi_txcal_tssi_tbl->root_pwr_tssi_lut_2G_bphy =
		&(mem->root_pwr_tssi_lut_2G_bphy);
	pwr_tssi_lut_2G = pi_txcal_tssi_tbl->root_pwr_tssi_lut_2G = &(mem->root_pwr_tssi_lut_2G);
	pwr_tssi_lut_5G20 = pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G20 =
			&(mem->root_pwr_tssi_lut_5G20);
	pwr_tssi_lut_5G40 = pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G40 =
			&(mem->root_pwr_tssi_lut_5G40);
	pwr_tssi_lut_5G80 = pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G80 =
			&(mem->root_pwr_tssi_lut_5G80);

	pwr_tssi_lut_2G_bphy->txcal_pwr_tssi = &(mem->txcal_pwr_tssi_2G_bphy);
	pwr_tssi_lut_2G->txcal_pwr_tssi = &(mem->txcal_pwr_tssi_2G);
	pwr_tssi_lut_5G20->txcal_pwr_tssi = &(mem->txcal_pwr_tssi_5G20);
	pwr_tssi_lut_5G40->txcal_pwr_tssi = &(mem->txcal_pwr_tssi_5G40);
	pwr_tssi_lut_5G80->txcal_pwr_tssi = &(mem->txcal_pwr_tssi_5G80);

	phy_tssical_read_olpc_params(pi, cmn_info);
#endif /* WLC_TXCAL */

	return cmn_info;

	/* error */
fail:
	phy_tssical_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_tssical_detach)(phy_tssical_info_t *cmn_info)
{
	phy_tssical_mem_t *mem;

#ifdef WLC_TXCAL
	phy_info_t *pi;
	txcal_root_pwr_tssi_t *pi_txcal_tssi_tbl;
	txcal_pwr_tssi_lut_t *pwr_tssi_lut_5G80, *pwr_tssi_lut_5G40, *pwr_tssi_lut_5G20;
	txcal_pwr_tssi_lut_t *pwr_tssi_lut_2G_bphy, *pwr_tssi_lut_2G;
	ASSERT(cmn_info);
	pi = cmn_info->pi;

#endif /* WLC_TXCAL */

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	if (!cmn_info)
		return;

	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null tssical module\n", __FUNCTION__));
		return;
	}

#ifdef WLC_TXCAL
	pi_txcal_tssi_tbl = cmn_info->txcal_root_pwr_tssi_tbl;
	if (pi_txcal_tssi_tbl != NULL) {
		pwr_tssi_lut_5G80 = pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G80;
		pwr_tssi_lut_5G40 = pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G40;
		pwr_tssi_lut_5G20 = pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G20;
		pwr_tssi_lut_2G_bphy = pi_txcal_tssi_tbl->root_pwr_tssi_lut_2G_bphy;
		pwr_tssi_lut_2G = pi_txcal_tssi_tbl->root_pwr_tssi_lut_2G;

		if (pwr_tssi_lut_5G80 != NULL) {
			if (pwr_tssi_lut_5G80->txcal_pwr_tssi != NULL) {
				phy_mfree(pi, pwr_tssi_lut_5G80->txcal_pwr_tssi,
						sizeof(*pwr_tssi_lut_5G80->txcal_pwr_tssi));
				pwr_tssi_lut_5G80->txcal_pwr_tssi = NULL;
			}
			phy_mfree(pi, pwr_tssi_lut_5G80,
					sizeof(*pwr_tssi_lut_5G80));
			 pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G80 = NULL;
		}
		if (pwr_tssi_lut_5G40 != NULL) {
			if (pwr_tssi_lut_5G40->txcal_pwr_tssi != NULL) {
				phy_mfree(pi, pwr_tssi_lut_5G40->txcal_pwr_tssi,
						sizeof(*pwr_tssi_lut_5G40->txcal_pwr_tssi));
				pwr_tssi_lut_5G40->txcal_pwr_tssi = NULL;
			}
			phy_mfree(pi, pwr_tssi_lut_5G40,
					sizeof(*pwr_tssi_lut_5G40));
			 pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G40 = NULL;
		}
		if (pwr_tssi_lut_5G20 != NULL) {
			if (pwr_tssi_lut_5G20->txcal_pwr_tssi != NULL) {
				phy_mfree(pi, pwr_tssi_lut_5G20->txcal_pwr_tssi,
						sizeof(*pwr_tssi_lut_5G20->txcal_pwr_tssi));
				pwr_tssi_lut_5G20->txcal_pwr_tssi = NULL;
			}
			phy_mfree(pi, pwr_tssi_lut_5G20,
					sizeof(*pwr_tssi_lut_5G20));
			 pi_txcal_tssi_tbl->root_pwr_tssi_lut_5G20 = NULL;
		}
		if (pwr_tssi_lut_2G_bphy != NULL) {
			if (pwr_tssi_lut_2G_bphy->txcal_pwr_tssi != NULL) {
				phy_mfree(pi, pwr_tssi_lut_2G_bphy->txcal_pwr_tssi,
						sizeof(*pwr_tssi_lut_2G_bphy->txcal_pwr_tssi));
				pwr_tssi_lut_2G_bphy->txcal_pwr_tssi = NULL;
			}
			phy_mfree(pi, pwr_tssi_lut_2G_bphy, sizeof(*pwr_tssi_lut_2G_bphy));
			pi_txcal_tssi_tbl->root_pwr_tssi_lut_2G_bphy = NULL;
		}
		if (pwr_tssi_lut_2G != NULL) {
			if (pwr_tssi_lut_2G->txcal_pwr_tssi != NULL) {
				phy_mfree(pi, pwr_tssi_lut_2G->txcal_pwr_tssi,
						sizeof(*pwr_tssi_lut_2G->txcal_pwr_tssi));
				pwr_tssi_lut_2G->txcal_pwr_tssi = NULL;
			}
			phy_mfree(pi, pwr_tssi_lut_2G, sizeof(*pwr_tssi_lut_2G));
			pi_txcal_tssi_tbl->root_pwr_tssi_lut_2G = NULL;
		}
		phy_mfree(pi, pi_txcal_tssi_tbl, sizeof(*pi_txcal_tssi_tbl));
		cmn_info->txcal_root_pwr_tssi_tbl = NULL;
	}
	if (cmn_info->txcal_meas != NULL) {
		phy_mfree(pi, cmn_info->txcal_meas, sizeof(*cmn_info->txcal_meas));
		cmn_info->txcal_meas = NULL;
	}
	if (cmn_info->txcal_pwr_tssi != NULL) {
		phy_mfree(pi, cmn_info->txcal_pwr_tssi,
				sizeof(*cmn_info->txcal_pwr_tssi));
		cmn_info->txcal_pwr_tssi = NULL;
	}
#endif /* WLC_TXCAL */

	phy_mfree(cmn_info->pi, mem, sizeof(phy_tssical_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_tssical_register_impl)(phy_tssical_info_t *cmn_info, phy_type_tssical_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*cmn_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_tssical_unregister_impl)(phy_tssical_info_t *cmn_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

int wlc_phy_tssivisible_thresh(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_tssical_info_t *ti = pi->tssicali;
	phy_type_tssical_fns_t *fns = ti->fns;
	int visi_thresh_qdbm = WL_RATE_DISABLED;

#if defined(WLOLPC)
	if (fns->visible_thresh != NULL) {
		visi_thresh_qdbm = (fns->visible_thresh)(fns->ctx);
	}
#endif /* WLOLPC */

	if (fns->visible_thresh == NULL) {
		if (ISLCN20PHY(pi))
			visi_thresh_qdbm = LCN20PHY_TXPWR_MIN * WLC_TXPWR_DB_FACTOR;
		else
			visi_thresh_qdbm = (PHY_TXPWR_MIN_LEGACYPHY * WLC_TXPWR_DB_FACTOR);
	}

	return visi_thresh_qdbm;
}

void wlc_phy_get_tssi_sens_min(wlc_phy_t *ppi, int8 *tssiSensMinPwr)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_tssical_info_t *ti = pi->tssicali;
	phy_type_tssical_fns_t *fns = ti->fns;
	if (fns->sens_min != NULL) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		(fns->sens_min)(fns->ctx, tssiSensMinPwr);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}
	else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

/* driver down processing */
int
phy_tssical_down(phy_tssical_info_t *cmn_info)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return callbacks;
}

int
phy_tssical_do_dummy_tx(phy_info_t *pi, bool ofdm, bool pa_on)
{
#define	DUMMY_PKT_LEN	20 /* Dummy packet's length */
	int	i, count;
	uint8	ofdmpkt[DUMMY_PKT_LEN] = {
		0xcc, 0x01, 0x02, 0x00, 0x00, 0x00, 0xd4, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
	};
	uint8	cckpkt[DUMMY_PKT_LEN] = {
		0x6e, 0x84, 0x0b, 0x00, 0x00, 0x00, 0xd4, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
	};
	uint32 *dummypkt;

	ASSERT((R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC) == 0);

	dummypkt = (uint32 *)(ofdm ? ofdmpkt : cckpkt);
	wlapi_bmac_write_template_ram(pi->sh->physhim, 0, DUMMY_PKT_LEN, dummypkt);

	/* set up the TXE transfer */

	W_REG(pi->sh->osh, D11_xmtsel(pi), 0);
	/* Assign the WEP to the transmit path */
	W_REG(pi->sh->osh, D11_WEP_CTL(pi), 0x100);

	/* Set/clear OFDM bit in PHY control word */
	W_REG(pi->sh->osh, D11_TXE_PHYCTL(pi), (ofdm ? 1 : 0) | PHY_TXC_ANT_0);

	ASSERT(ofdm);
	W_REG(pi->sh->osh, D11_TXE_PHYCTL_1(pi), 0x1A02);

	W_REG(pi->sh->osh, D11_TXE_WM_0(pi), 0);		/* No substitutions */
	W_REG(pi->sh->osh, D11_TXE_WM_1(pi), 0);

	W_REG(pi->sh->osh, D11_xmttplatetxptr(pi), 0);
	W_REG(pi->sh->osh, D11_xmttxcnt(pi), DUMMY_PKT_LEN);

	/* Set Template as source, length specified as a count and destination
	 * as Serializer also set "gen_eof"
	 */
	W_REG(pi->sh->osh, D11_xmtsel(pi), ((8 << 8) | (1 << 5) | (1 << 2) | 2));

	/* Instruct the MAC to not calculate FCS, we'll supply a bogus one */
	W_REG(pi->sh->osh, D11_TXE_CTL(pi), 0);

	/* Start transmission and wait until sendframe goes away */
	/* Set TX_NOW in AUX along with MK_CTLWRD */
	W_REG(pi->sh->osh, D11_TXE_AUX(pi), 0xD0);

	(void)R_REG(pi->sh->osh, D11_TXE_AUX(pi));

	/* Wait for 10 x ack time, enlarge it for vsim of QT */
	i = 0;
	count = ofdm ? 30 : 250;

#ifndef BCMQT_CPU
	if (ISSIM_ENAB(pi->sh->sih)) {
		count *= 100;
	}
#endif // endif
	/* wait for txframe to be zero */
	while ((i++ < count) && (R_REG(pi->sh->osh, D11_TXE_STATUS(pi)) & (1 << 7))) {
		OSL_DELAY(10);
	}
	if (i >= count)
		PHY_ERROR(("wl%d: %s: Waited %d uS for %s txframe\n",
		          pi->sh->unit, __FUNCTION__, 10 * i, (ofdm ? "ofdm" : "cck")));

	/* Wait for the mac to finish (this is 10x what is supposed to take) */
	i = 0;
	/* wait for txemend */
	while ((i++ < 10) && ((R_REG(pi->sh->osh, D11_TXE_STATUS(pi)) & (1 << 10)) == 0)) {
		OSL_DELAY(10);
	}
	if (i >= 10)
		PHY_ERROR(("wl%d: %s: Waited %d uS for txemend\n",
		          pi->sh->unit, __FUNCTION__, 10 * i));

	/* Wait for the phy to finish */
	i = 0;
	/* wait for txcrs */
	while ((i++ < 500) && ((R_REG(pi->sh->osh, D11_IFS_STAT(pi)) & (1 << 8)))) {
		OSL_DELAY(10);
	}
	if (i >= 500)
		PHY_ERROR(("wl%d: %s: Waited %d uS for txcrs\n",
		          pi->sh->unit, __FUNCTION__, 10 * i));

	return BCME_OK;
}

#ifdef WLC_TXCAL
static void
BCMATTACHFN(phy_tssical_read_olpc_params)(phy_info_t *pi, phy_tssical_info_t *info)
{
	uint8 i;
	info->disable_olpc = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_disable_olpc, 0));
	for (i = 0; i < PHY_CORE_MAX; i++) {
		/* Tempslope is in S0.10 format */
		info->olpc_tempslope2g[i] = (int16) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE
			(pi, rstr_olpc_tempslope2g, i, 0));
		info->olpc_tempslope5g[i] = (int16) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE
			(pi, rstr_olpc_tempslope5g, i, 0));
	}
	/* Both olpc_thresh and olpc_anchor are in qdb format */
	info->olpc_thresh = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_olpc_thresh, 0));
	/* If olpc_thresh2g/5g not present in nvram, just load them with olpc_thresh value */
	info->olpc_thresh2g = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_olpc_thresh2g,
		info->olpc_thresh));
	info->olpc_thresh5g = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_olpc_thresh5g,
		info->olpc_thresh));
	info->olpc_anchor2g = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_olpc_anchor2g, 0));
	info->olpc_anchor5g = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_olpc_anchor5g, 0));
	/* olpc_idx_in_use is the top level control for whether  */
	/* table based txcal anchor point txidx will be used for OLPC */
	info->olpc_idx_in_use = (uint8) (PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_olpc_idx_in_use, 0));
	for (i = 0; i < 5; i++) {
		/* 2G + 5G 4 subbands */
		info->olpc_offset[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE
			(pi, rstr_olpc_offset, i, 0));
	}
}

void
phy_tssical_compute_olpc_idx(phy_tssical_info_t *tssicali)
{
	phy_type_tssical_fns_t *fns = tssicali->fns;
	if (fns->compute_olpc_idx)
		(fns->compute_olpc_idx)(fns->ctx);
}

bool
phy_tssical_get_pwr_tssi_tbl_in_use(phy_tssical_info_t *tssicali)
{
	return (tssicali->txcal_pwr_tssi_tbl_in_use);
}

bool
phy_tssical_get_olpc_idx_valid_in_use(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_idx_valid && tssicali->olpc_idx_in_use);
}

uint8
phy_tssical_get_olpc_idx_valid(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_idx_valid);
}

void
phy_tssical_set_olpc_idx_valid(phy_tssical_info_t *tssicali, uint8 val)
{
	tssicali->olpc_idx_valid = val;
}

uint8
phy_tssical_get_olpc_idx_in_use(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_idx_in_use);
}

void
phy_tssical_set_olpc_idx_in_use(phy_tssical_info_t *tssicali, uint8 val)
{
	tssicali->olpc_idx_in_use = val;
}

int8 phy_tssical_get_olpc_anchor2g(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_anchor2g);
}

void phy_tssical_set_olpc_anchor2g(phy_tssical_info_t *tssicali, int8 anchor2g)
{
	tssicali->olpc_anchor2g = anchor2g;
}

int8 phy_tssical_get_olpc_anchor5g(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_anchor5g);
}

void phy_tssical_set_olpc_anchor5g(phy_tssical_info_t *tssicali, int8 anchor5g)
{
	tssicali->olpc_anchor5g = anchor5g;
}

int8 phy_tssical_get_olpc_anchor(phy_tssical_info_t *tssicali)
{
	phy_info_t *pi = tssicali->pi;
	if (CHSPEC_IS2G(pi->radio_chanspec))
		return (tssicali->olpc_anchor2g);
	else
		return (tssicali->olpc_anchor5g);
}

void phy_tssical_set_olpc_anchor_threshold(phy_tssical_info_t *tssicali)
{
	phy_info_t *pi = tssicali->pi;
	if (CHSPEC_IS2G(pi->radio_chanspec))
		tssicali->olpc_anchor2g = tssicali->olpc_thresh;
	else
		tssicali->olpc_anchor5g = tssicali->olpc_thresh;
}

int16 phy_tssical_get_olpc_tempslope(phy_tssical_info_t *tssicali, uint8 core)
{
	phy_info_t *pi = tssicali->pi;
	if (CHSPEC_IS2G(pi->radio_chanspec))
		return (tssicali->olpc_tempslope2g[core]);
	else
		return (tssicali->olpc_tempslope5g[core]);
}

int8 phy_tssical_get_olpc_offset(phy_tssical_info_t *tssicali, int8 chan_freq_range)
{
	int8 offset_idx;
	switch (chan_freq_range) {
		case WL_CHAN_FREQ_RANGE_2G:
			offset_idx = tssicali->olpc_offset[0];
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND0:
			offset_idx = tssicali->olpc_offset[1];
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND1:
			offset_idx = tssicali->olpc_offset[2];
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND2:
			offset_idx = tssicali->olpc_offset[3];
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND3:
			offset_idx = tssicali->olpc_offset[4];
			break;
		default:
			offset_idx = 0;
	}
	return offset_idx;
}

uint8 phy_tssical_get_olpc_anchor_idx(phy_tssical_info_t *tssicali, uint8 core)
{
	return (tssicali->olpc_anchor_idx[core]);
}

void phy_tssical_set_olpc_anchor_idx(phy_tssical_info_t *tssicali, uint8 core, uint8 val)
{
	tssicali->olpc_anchor_idx[core] = val;
}

int16 phy_tssical_get_olpc_tempsense(phy_tssical_info_t *tssicali, uint8 core)
{
	return (tssicali->olpc_tempsense[core]);
}

void phy_tssical_set_olpc_tempsense(phy_tssical_info_t *tssicali, uint8 core, int16 val)
{
	tssicali->olpc_tempsense[core] = val;
}

int8 phy_tssical_get_olpc_threshold(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_thresh);
}

void phy_tssical_set_olpc_threshold_val(phy_tssical_info_t *tssicali, int8 val)
{
	tssicali->olpc_thresh = val;
}

void phy_tssical_iov_set_olpc_threshold(phy_tssical_info_t *tssicali, int8 olpc_threshold)
{
	tssicali->olpc_thresh = olpc_threshold;
	tssicali->olpc_thresh_iovar_ovr = 1;
}

int16 phy_tssical_get_last_calc_temp(phy_tssical_info_t *tssicali)
{
	return tssicali->txcal_olpc_last_calc_temp;
}

void phy_tssical_set_last_calc_temp(phy_tssical_info_t *tssicali, int16 last_calc_temp)
{
	tssicali->txcal_olpc_last_calc_temp = last_calc_temp;
}

int8 phy_tssical_get_disable_olpc(phy_tssical_info_t *tssicali)
{
	return tssicali->disable_olpc;
}

void phy_tssical_set_disable_olpc(phy_tssical_info_t *tssicali, int8 disable_olpc)
{
	tssicali->disable_olpc = disable_olpc;
}

void phy_tssical_get_set_olpc_offset(phy_tssical_info_t *tssicali, int8 *olpc_offset, uint8 set)
{
	if (set) {
		bcopy(olpc_offset, tssicali->olpc_offset, 5*sizeof(int8));
	} else {
		bcopy(tssicali->olpc_offset, olpc_offset, 5*sizeof(int8));
	}
}

txcal_root_pwr_tssi_t *
phy_tssical_get_root_pwr_tssi_tbl(phy_tssical_info_t *tssicali)
{
	return tssicali->txcal_root_pwr_tssi_tbl;
}

int
phy_tssical_iovar_adjusted_tssi(phy_tssical_info_t *tssicali, int32 *ret_int_ptr, uint8 int_val)
{
	phy_type_tssical_fns_t *fns = tssicali->fns;
	phy_info_t *pi = tssicali->pi;
	if (fns->adjusted_tssi) {
		if (int_val >= PHY_CORE_MAX)
			*ret_int_ptr = 0;
		else
			*ret_int_ptr = (fns->adjusted_tssi)(pi, int_val);
	}
	return BCME_OK;
}

int
phy_tssical_copy_get_gainsweep_meas(phy_tssical_info_t *tssicali,
		wl_txcal_meas_ncore_t * txcal_meas)
{
	uint16 i, j;
	wl_txcal_meas_percore_t * per_core;
	txcal_meas->valid_cnt = tssicali->txcal_meas->valid_cnt;
	txcal_meas->num_core = PHY_CORE_MAX;
	txcal_meas->version = TXCAL_IOVAR_VERSION;
	/* fillup per core info */
	per_core = txcal_meas->txcal_percore;
	for (i = 0; i < PHY_CORE_MAX; i++) {
		for (j = 0; j < tssicali->txcal_meas->valid_cnt; j++) {
			per_core->tssi[j] = tssicali->txcal_meas->tssi[i][j];
			per_core->pwr[j] = tssicali->txcal_meas->pwr[i][j];
		}
		per_core++;
	}
	return BCME_OK;
}

void
phy_tssical_set_measured_pwr(phy_tssical_info_t *tssicali, wl_txcal_meas_percore_t *per_core)
{
	uint8 i, j;
	for (i = 0; i < PHY_CORE_MAX; i++) {
		for (j = 0; j < tssicali->txcal_meas->valid_cnt; j++) {
			tssicali->txcal_meas->pwr[i][j] = per_core->pwr[j];
		}
		per_core++;
	}
}

int
phy_tssical_gainsweep(phy_tssical_info_t *tssicali, wl_txcal_params_t *txcal_params)
{
	phy_info_t *pi = tssicali->pi;
	int16 gidx;
	uint16 adj_tssi = 0;
	bool more_steps;
	uint8 cnt = 0;
	uint16 save_TxPwrCtrlCmd = 0;
	uint8 tx_pwr_ctrl_state = pi->txpwrctrl;
	uint8 core;
	wl_txcal_meas_t *pi_txcal_meas = tssicali->txcal_meas;
	bool suspend = FALSE;
	phy_type_tssical_fns_t *fns = tssicali->fns;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	//suspend mac before accessing phy registers
	wlc_phy_conditional_suspend(pi, &suspend);
	if (fns->txpwr_ctrl_cmd)
		save_TxPwrCtrlCmd = (fns->txpwr_ctrl_cmd)(fns->ctx);
	gidx = txcal_params->gidx_start;
	do {
		if (fns->set_txpwrindex)
			(fns->set_txpwrindex)(fns->ctx, gidx, save_TxPwrCtrlCmd);
		wlc_phy_conditional_resume(pi, &suspend);
		wlapi_bmac_pkteng_txcal(pi->sh->physhim, 0, 0, &(txcal_params->pkteng));
		wlc_phy_conditional_suspend(pi, &suspend);
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			if (fns->adjusted_tssi)
				adj_tssi = (fns->adjusted_tssi)(fns->ctx, core);
			pi_txcal_meas->tssi[core][cnt] = adj_tssi;
		}
		cnt++;
		wlc_phy_conditional_resume(pi, &suspend);
		wlapi_bmac_pkteng_txcal(pi->sh->physhim, 0, 0, 0);
		wlapi_bmac_service_txstatus(pi->sh->physhim);
		wlc_phy_conditional_suspend(pi, &suspend);
		gidx = gidx + txcal_params->gidx_step;
		if ((gidx > 127) || (gidx < 0))
			more_steps = FALSE;
		else
			more_steps = (txcal_params->gidx_step > 0) ?
				(gidx <= txcal_params->gidx_stop) :
				(gidx >= txcal_params->gidx_stop);
	} while (more_steps);
	pi_txcal_meas->valid_cnt = cnt;
	if (fns->restore_txpwr_ctrl_cmd)
		(fns->restore_txpwr_ctrl_cmd)(fns->ctx, save_TxPwrCtrlCmd, tx_pwr_ctrl_state);
	wlc_phy_conditional_resume(pi, &suspend);
	return BCME_OK;
}

int
phy_tssical_generate_pwr_tssi_tbl(phy_tssical_info_t *tssicali)
{
	wl_txcal_power_tssi_t *pi_txcal_pwr_tssi = tssicali->txcal_pwr_tssi;
	wl_txcal_meas_t *pi_txcal_meas = tssicali->txcal_meas;
	uint8 core = pi_txcal_pwr_tssi->set_core;
	int16 pwr_start = pi_txcal_pwr_tssi->pwr_start[core];
	uint8 num_entries = pi_txcal_pwr_tssi->num_entries[core];
	int32 pwr_val = pwr_start;
	int8 i = 0;
	uint8 j, k;
	int16 *pwr = pi_txcal_meas->pwr[core];
	uint16 *tssi = pi_txcal_meas->tssi[core];
	uint8 valid_cnt = pi_txcal_meas->valid_cnt;
	int32 tssi_val;
	uint8 flag;
	int32 num, den;
	phy_type_tssical_fns_t *fns = tssicali->fns;
	for (j = 0; j < num_entries; j++) {
		flag = 0;
		for (k = 0; k < valid_cnt; k++) {
			if (pwr[k] <= pwr_val) {
				flag = 1;
				i = k-1;
				if (i < 0)
					i = 0;
				break;
			}
		}
		if (flag == 0) {
			i = valid_cnt - 2;
			if (i < 0) {
				return BCME_ERROR;
			}
		}
		/* Interpolate between i and i+1 to find tssi_val at known pwr */
		num = (pwr_val - pwr[i])*(tssi[i+1] - tssi[i]);
		den = pwr[i+1] - pwr[i];
		/* Limiting tssi_val if trying to find tssi for power higher than */
		/* that measured during gain sweep */
		if (den == 0 || i == 0)
			tssi_val = tssi[i];
		else {
			tssi_val = tssi[i] + (num + (num > 0 ? ABS(den) :
			-ABS(den))/2)/den;
		}
		/* Keep TSSI val in 8 bits to increase accuracy when */
		/* interpolated to 128 entries estpwrlut */
		if (fns->get_tssi_val) {
			tssi_val = (fns->get_tssi_val)(fns->ctx, tssi_val);
		} else {
			tssi_val = (tssi_val > 0 ? tssi_val+2 :	tssi_val-2)>>2;
		}
		/* Limiting tssi_val in range */
		if (tssi_val < 0)
			tssi_val = 0;
		else if (tssi_val > 255)
			tssi_val = 255;
		tssicali->txcal_pwr_tssi->tssi[core][j] = (uint8) tssi_val;
		pwr_val = pwr_val + 8;
	}
	tssicali->txcal_pwr_tssi->gen_tbl = 0;
	return BCME_OK;
}

int
phy_tssical_set_pwr_tssi_tbl(phy_tssical_info_t *tssicali, void *p)
{
	phy_info_t *pi = tssicali->pi;
	wl_txcal_power_tssi_t  *pi_txcal_pwr_tssi;
	wl_txcal_power_tssi_ncore_t * txcal_tssi;
	txcal_tssi = (wl_txcal_power_tssi_ncore_t *)p;
	/* check for txcal version */
	if (txcal_tssi->version != TXCAL_IOVAR_VERSION) {
		return BCME_VERSION;
	}
	/* FW copy */
	pi_txcal_pwr_tssi = tssicali->txcal_pwr_tssi;
	if (txcal_tssi->gen_tbl) {
		/* Generate table */
		phy_tssical_copy_set_pwr_tssi_tbl_gentbl(pi_txcal_pwr_tssi, txcal_tssi);
		phy_tssical_generate_pwr_tssi_tbl(tssicali);
	} else {
		phy_tssical_copy_set_pwr_tssi_tbl_storetbl(pi_txcal_pwr_tssi, txcal_tssi);
		/* 4355 supports both RSDB and MIMO modes.
		 * In RSDB mode they can use the tssi table generated (in MIMO mode)
		 * by copying core1 table to core0
		 * Currently for RSDB enabled builds
		 * tssi table of core1 will be copied to core0
		 * without checking the current mode,
		 * as for RSDB enabled builds default mode is MIMO
		 */
		if (ACREV_IS(pi->pubpi->phy_rev, 4) &&
				(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1)) {
			phy_tssical_copy_rsdb_pwr_tssi_tbl(pi->tssicali);
		}
	}
	return BCME_OK;
}

int
phy_tssical_get_pwr_tssi_tbl(phy_tssical_info_t *tssicali, uint8 channel, uint8 mode)
{
	/* Go over the list for inquiry of the pwr tssi tbl */
	txcal_pwr_tssi_lut_t *LUT_pt;
	txcal_root_pwr_tssi_t *pi_txcal_root_pwr_tssi_tbl = tssicali->txcal_root_pwr_tssi_tbl;
	if ((channel < 15) && !mode) {
		LUT_pt = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G_bphy;
		//LUT_pt = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G;
	} else if ((channel < 15) && mode) {
		LUT_pt = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G;
	} else if ((channel & 0x06) == 0x06) {
		LUT_pt = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G40;
	} else if ((channel & 0x0A) == 0x0A) {
		LUT_pt = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G80;
	} else {
		LUT_pt = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G20;
	}
	while (LUT_pt->next_chan != 0) {
		if (LUT_pt->txcal_pwr_tssi->channel == channel) {
			bcopy(LUT_pt->txcal_pwr_tssi, tssicali->txcal_pwr_tssi,
			        sizeof(wl_txcal_power_tssi_t));
			break;
		} else {
			LUT_pt = LUT_pt->next_chan;
		}
	}
	if (LUT_pt->txcal_pwr_tssi->channel == channel) {
		bcopy(LUT_pt->txcal_pwr_tssi, tssicali->txcal_pwr_tssi,
			sizeof(wl_txcal_power_tssi_t));
	} else {
		memset(tssicali->txcal_pwr_tssi, 0, sizeof(wl_txcal_power_tssi_t));
	}
	return BCME_OK;
}

int
phy_tssical_copy_get_pwr_tssi_tbl(phy_tssical_info_t *tssicali,
		wl_txcal_power_tssi_ncore_t * txcal_tssi)
{
	uint16 i;
	wl_txcal_power_tssi_percore_t * per_core;
	wl_txcal_power_tssi_t * in_buf = tssicali->txcal_pwr_tssi;
	txcal_tssi->set_core = in_buf->set_core;
	txcal_tssi->channel  = in_buf->channel;
	txcal_tssi->gen_tbl = in_buf->gen_tbl;
	txcal_tssi->num_core = PHY_CORE_MAX;
	txcal_tssi->version = TXCAL_IOVAR_VERSION;
	txcal_tssi->ofdm = in_buf->ofdm;
	/* per core info */
	per_core = txcal_tssi->tssi_percore;
	for (i = 0; i < PHY_CORE_MAX; i++) {
		per_core->tempsense = in_buf->tempsense[i];
		per_core->pwr_start = in_buf->pwr_start[i];
		per_core->pwr_start_idx = in_buf->pwr_start_idx[i];
		per_core->num_entries = in_buf->num_entries[i];
		bcopy(in_buf->tssi[i], per_core->tssi,
			MAX_NUM_PWR_STEP * sizeof(in_buf->tssi[0][0]));
		per_core++;
	}
	return BCME_OK;
}

int
phy_tssical_get_olpc_pwr(phy_tssical_info_t *tssicali, void *p, void *a)
{
	wl_olpc_pwr_t olpc_pwr;
	uint8 core;
	wl_txcal_power_tssi_t * in_buf = tssicali->txcal_pwr_tssi;
	bcopy(p, &olpc_pwr, sizeof(wl_olpc_pwr_t));
	/* Check for txcal version */
	if (olpc_pwr.version != TXCAL_IOVAR_VERSION) {
		PHY_ERROR(("wl%d: %s  version mismatch",
			tssicali->pi->sh->unit, __FUNCTION__));
		return BCME_VERSION;
	}
	core = olpc_pwr.core;
	phy_tssical_get_pwr_tssi_tbl(tssicali, olpc_pwr.channel, olpc_pwr.ofdm);
	olpc_pwr.olpc_idx = in_buf->pwr_start_idx[core];
	olpc_pwr.tempsense = in_buf->tempsense[core];
	olpc_pwr.version = TXCAL_IOVAR_VERSION;
	bcopy(&olpc_pwr, a, sizeof(wl_olpc_pwr_t));
	return BCME_OK;
}

int
phy_tssical_store_pwr_tssi_tbl(phy_tssical_info_t *tssicali)
{
	txcal_pwr_tssi_lut_t *LUT_pt;
	txcal_pwr_tssi_lut_t *LUT_tmp;
	txcal_pwr_tssi_lut_t *LUT_root;
	txcal_root_pwr_tssi_t *pi_txcal_root_pwr_tssi_tbl = tssicali->txcal_root_pwr_tssi_tbl;
	if ((tssicali->txcal_pwr_tssi->channel < 15) && !tssicali->txcal_pwr_tssi->ofdm) {
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G_bphy;
		//LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G;
	} else if ((tssicali->txcal_pwr_tssi->channel < 15) && tssicali->txcal_pwr_tssi->ofdm) {
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G;
	} else if ((tssicali->txcal_pwr_tssi->channel & 0x06) == 0x06) {
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G40;
	} else if ((tssicali->txcal_pwr_tssi->channel & 0x0A) == 0x0A) {
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G80;
	} else {
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G20;
	}
	if (LUT_root->txcal_pwr_tssi->channel == 0) {
		/* First valid entry of the linked list */
		bcopy(tssicali->txcal_pwr_tssi, LUT_root->txcal_pwr_tssi,
		      sizeof(wl_txcal_power_tssi_t));
		LUT_root->next_chan = 0;
	} else {
		LUT_pt = LUT_root;
		if (LUT_pt->txcal_pwr_tssi->channel > tssicali->txcal_pwr_tssi->channel) {
			/* Add entry at the beginning of the linked list */
			phy_tssical_alloc_pwr_tssi_lut(tssicali, &LUT_tmp);
			bcopy(LUT_root->txcal_pwr_tssi, LUT_tmp->txcal_pwr_tssi,
			        sizeof(wl_txcal_power_tssi_t));
			LUT_tmp->next_chan = LUT_root->next_chan;
			bcopy(tssicali->txcal_pwr_tssi, LUT_root->txcal_pwr_tssi,
			        sizeof(wl_txcal_power_tssi_t));
			LUT_root->next_chan = LUT_tmp;
			tssicali->txcal_pwr_tssi_tbl_count++;
			return BCME_OK;
		}
		while (LUT_pt->next_chan != 0) {
			/* Go over all the entries in the list */
			if (LUT_pt->txcal_pwr_tssi->channel == tssicali->txcal_pwr_tssi->channel)
				break;
			if ((LUT_pt->txcal_pwr_tssi->channel <
				tssicali->txcal_pwr_tssi->channel) &&
				(LUT_pt->next_chan->txcal_pwr_tssi->channel >
				tssicali->txcal_pwr_tssi->channel))
				break;
			LUT_pt = LUT_pt->next_chan;
		}
		if (LUT_pt->txcal_pwr_tssi->channel == tssicali->txcal_pwr_tssi->channel) {
			/* Channel found, override */
			bcopy(tssicali->txcal_pwr_tssi, LUT_pt->txcal_pwr_tssi,
			        sizeof(wl_txcal_power_tssi_t));
		} else {
			if (tssicali->txcal_pwr_tssi_tbl_count > 31)
				return BCME_NOMEM;
			if (LUT_pt->next_chan == 0) {
				/* Add to the end of the linked list */
				phy_tssical_alloc_pwr_tssi_lut(tssicali, &LUT_pt->next_chan);
				LUT_pt = LUT_pt->next_chan;
				bcopy(tssicali->txcal_pwr_tssi, LUT_pt->txcal_pwr_tssi,
				        sizeof(wl_txcal_power_tssi_t));
				LUT_pt->next_chan = 0;
			} else {
				/* Insert into the linked list */
				phy_tssical_alloc_pwr_tssi_lut(tssicali, &LUT_tmp);
				bcopy(tssicali->txcal_pwr_tssi, LUT_tmp->txcal_pwr_tssi,
				        sizeof(wl_txcal_power_tssi_t));
				LUT_tmp->next_chan = LUT_pt->next_chan;
				LUT_pt->next_chan = LUT_tmp;
			}
			tssicali->txcal_pwr_tssi_tbl_count++;
		}
	}
	return BCME_OK;
}

int
phy_tssical_set_olpc_pwr(phy_tssical_info_t *tssicali, void *p, void *a)
{
	wl_olpc_pwr_t olpc_pwr;
	uint8 core;
	phy_type_tssical_fns_t *fns = tssicali->fns;
	int err;
	wl_txcal_power_tssi_t * in_buf = tssicali->txcal_pwr_tssi;
	bcopy(p, &olpc_pwr, sizeof(wl_olpc_pwr_t));
	/* Get the structure for the particular channel */
	phy_tssical_get_pwr_tssi_tbl(tssicali, olpc_pwr.channel, olpc_pwr.ofdm);
	/* Check for txcal version */
	if (olpc_pwr.version != TXCAL_IOVAR_VERSION) {
		return BCME_VERSION;
	}
	core = olpc_pwr.core;
	in_buf->channel = olpc_pwr.channel;
	in_buf->pwr_start_idx[core] = olpc_pwr.olpc_idx;
	in_buf->tempsense[core] = olpc_pwr.tempsense;
	err = phy_tssical_store_pwr_tssi_tbl(tssicali);
	if (fns->set_olpc_anchor)
		(fns->set_olpc_anchor)(fns->ctx);
	return err;
}

void
phy_tssical_set_pwr_tssi_tbl_in_use(phy_tssical_info_t *tssicali, bool val)
{
	tssicali->txcal_pwr_tssi_tbl_in_use = val;
}

uint8
phy_tssical_get_txcal_status(phy_tssical_info_t *tssicali)
{
	return tssicali->txcal_status;
}

int
phy_tssical_apply_pa_params(phy_tssical_info_t *tssicali)
{
	phy_type_tssical_fns_t *fns = tssicali->fns;
	if (fns->apply_paparams)
		(fns->apply_paparams)(fns->ctx);
	return BCME_OK;
}

void
phy_tssical_set_txcal_status(phy_tssical_info_t *tssicali, uint8 val)
{
	tssicali->txcal_status = val;
}

int
phy_tssical_apply_pwr_tssi_tbl(phy_tssical_info_t *tssicali,
		wl_txcal_power_tssi_t *txcal_pwr_tssi, uint8 bphy_tbl)
{
	phy_type_tssical_fns_t *fns = tssicali->fns;
	if (fns->apply_pwr_tssi_tbl)
		(fns->apply_pwr_tssi_tbl)(fns->ctx, txcal_pwr_tssi, bphy_tbl);
	return BCME_OK;
}

int
phy_tssical_generate_estpwr_lut(wl_txcal_power_tssi_t *txcal_pwr_tssi, uint16 *estpwr, uint8 core)
{
	uint8 tssi_val_idx;
	uint8 tssi_val;
	int8 i = 0, k;
	int16 pwr_start = txcal_pwr_tssi->pwr_start[core];
	uint8 num_entries = txcal_pwr_tssi->num_entries[core];
	uint8 *tssi = txcal_pwr_tssi->tssi[core];
	int16 pwr_i, pwr_i_1;
	int32 est_pwr_calc;
	int32 num, den;

	for (tssi_val_idx = 0; tssi_val_idx < 128; tssi_val_idx++) {
		tssi_val = tssi_val_idx * 2;
		for (k = num_entries-1; k >= 0; k--) {
			if (tssi[k] >= tssi_val) {
				i = k+1;
				if (i >= num_entries)
					i = num_entries-1;
				break;
			}
		}
		pwr_i = pwr_start + 8*i;
		pwr_i_1 = pwr_start + 8*(i-1);
		if (tssi[i-1] - tssi[i] == 0) {
			est_pwr_calc = pwr_i;
		} else {
			num = (tssi_val - tssi[i])*(pwr_i_1 - pwr_i);
			den = tssi[i-1] - tssi[i];
			est_pwr_calc = pwr_i + (num + (num > 0 ? ABS(den) :
				-ABS(den))/2)/den;
		}
		/* Convert to qdBm for writing to LUT */
		est_pwr_calc = (est_pwr_calc + 1) >> 1;
		if (est_pwr_calc > 126)
			est_pwr_calc = 126;
		else if (est_pwr_calc < -128)
			est_pwr_calc = -128;
		estpwr[tssi_val_idx] = (uint16) (est_pwr_calc & 0xFF);
	}
	return BCME_OK;
}

int
phy_tssical_copy_set_pwr_tssi_tbl_gentbl(wl_txcal_power_tssi_t *pi_txcal_pwr_tssi,
		wl_txcal_power_tssi_ncore_t *txcal_tssi)
{
	uint16 i;
	wl_txcal_power_tssi_percore_t * per_core;
	per_core = txcal_tssi->tssi_percore;
	pi_txcal_pwr_tssi->gen_tbl = txcal_tssi->gen_tbl;
	pi_txcal_pwr_tssi->channel = txcal_tssi->channel;
	pi_txcal_pwr_tssi->ofdm = txcal_tssi->ofdm;
	for (i = 0; i < PHY_CORE_MAX; i++) {
		pi_txcal_pwr_tssi->pwr_start[i] = per_core->pwr_start;
		pi_txcal_pwr_tssi->num_entries[i] = per_core->num_entries;
		per_core++;
	}
	pi_txcal_pwr_tssi->set_core = txcal_tssi->set_core;
	return BCME_OK;
}

int
phy_tssical_copy_set_pwr_tssi_tbl_storetbl(wl_txcal_power_tssi_t *pi_txcal_pwr_tssi,
		wl_txcal_power_tssi_ncore_t *txcal_tssi)
{
	uint16 i;
	wl_txcal_power_tssi_percore_t * per_core;
	per_core = txcal_tssi->tssi_percore;
	for (i = 0; i < PHY_CORE_MAX; i++) {
		pi_txcal_pwr_tssi->set_core = txcal_tssi->set_core;
		pi_txcal_pwr_tssi->channel = txcal_tssi->channel;
		pi_txcal_pwr_tssi->gen_tbl = txcal_tssi->gen_tbl;
		pi_txcal_pwr_tssi->tempsense[i] = per_core->tempsense;
		pi_txcal_pwr_tssi->pwr_start[i] = per_core->pwr_start;
		pi_txcal_pwr_tssi->pwr_start_idx[i] = per_core->pwr_start_idx;
		pi_txcal_pwr_tssi->num_entries[i] = per_core->num_entries;
		pi_txcal_pwr_tssi->tempsense[i] = per_core->tempsense;
		pi_txcal_pwr_tssi->ofdm = txcal_tssi->ofdm;
		bcopy(per_core->tssi, pi_txcal_pwr_tssi->tssi[i],
			MAX_NUM_PWR_STEP * sizeof(per_core->tssi[0]));
		per_core++;
	}

	return BCME_OK;
}

int
phy_tssical_alloc_pwr_tssi_lut(phy_tssical_info_t *tssicali, txcal_pwr_tssi_lut_t** LUT)
{
	phy_info_t *pi = tssicali->pi;
	*LUT = phy_malloc_fatal(pi, sizeof(**LUT));
	(*LUT)->txcal_pwr_tssi = phy_malloc_fatal(pi, sizeof(*(*LUT)->txcal_pwr_tssi));
	return BCME_OK;
}

void
phy_tssical_copy_rsdb_pwr_tssi_tbl(phy_tssical_info_t *tssicali)
{
	int tssi_lp_cnt;
	tssicali->txcal_pwr_tssi->tempsense[PHY_RSBD_PI_IDX_CORE0] =
			tssicali->txcal_pwr_tssi->tempsense[PHY_RSBD_PI_IDX_CORE1];
	tssicali->txcal_pwr_tssi->pwr_start[PHY_RSBD_PI_IDX_CORE0] =
			tssicali->txcal_pwr_tssi->pwr_start[PHY_RSBD_PI_IDX_CORE1];
	tssicali->txcal_pwr_tssi->pwr_start_idx[PHY_RSBD_PI_IDX_CORE0] =
			tssicali->txcal_pwr_tssi->pwr_start_idx[PHY_RSBD_PI_IDX_CORE1];
	tssicali->txcal_pwr_tssi->num_entries[PHY_RSBD_PI_IDX_CORE0] =
			tssicali->txcal_pwr_tssi->num_entries[PHY_RSBD_PI_IDX_CORE1];
	for (tssi_lp_cnt = 0; tssi_lp_cnt < MAX_NUM_PWR_STEP; tssi_lp_cnt++)
		tssicali->txcal_pwr_tssi->tssi[PHY_RSBD_PI_IDX_CORE0][tssi_lp_cnt] =
			tssicali->txcal_pwr_tssi->tssi[PHY_RSBD_PI_IDX_CORE1][tssi_lp_cnt];
	return;
}

int
phy_tssical_iov_apply_pwr_tssi_tbl(phy_tssical_info_t *tssicali, int int_val)
{
	phy_type_tssical_fns_t *fns = tssicali->fns;
	if (fns->iov_apply_pwr_tssi_tbl) {
		(fns->iov_apply_pwr_tssi_tbl)(fns->ctx, int_val);
		return BCME_OK;
	}
	else
		return BCME_UNSUPPORTED;
}

int
phy_tssical_iov_read_est_pwr_lut(phy_tssical_info_t *tssicali, void *a, int int_val)
{
	phy_type_tssical_fns_t *fns = tssicali->fns;
	if (fns->read_est_pwr_lut) {
		(fns->read_est_pwr_lut)(fns->ctx, a, (uint8) int_val);
		return BCME_OK;
	}
	else
		return BCME_UNSUPPORTED;
}

void
phy_tssical_set_olpc_threshold(phy_tssical_info_t *tssicali)
{
	phy_info_t *pi = tssicali->pi;
	if (tssicali->olpc_thresh_iovar_ovr != 1) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (tssicali->olpc_thresh2g != 0)
				tssicali->olpc_thresh = tssicali->olpc_thresh2g;
		} else {
			if (tssicali->olpc_thresh5g != 0)
				tssicali->olpc_thresh = tssicali->olpc_thresh5g;
		}
	}
}
#endif /* WLC_TXCAL */

#ifdef WL_EAP_OLPC
int8 phy_tssical_get_olpc_threshold2g(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_thresh2g);
}

int8 phy_tssical_get_olpc_threshold5g(phy_tssical_info_t *tssicali)
{
	return (tssicali->olpc_thresh5g);
}
#endif /* WL_EAP_OLPC */
