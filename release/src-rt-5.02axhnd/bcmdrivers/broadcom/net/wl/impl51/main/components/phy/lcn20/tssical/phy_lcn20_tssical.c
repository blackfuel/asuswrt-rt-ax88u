/*
 * Tssical module.
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
 * $Id: phy_lcn20_tssical.c 685812 2017-02-18 02:44:08Z $
 */
#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_lcn20.h>
#include <wlc_phy_lcn20.h>
#include <phy_utils_reg.h>
#include <wlc_phyreg_lcn20.h>
#include <wlc_phytbl_lcn20.h>
#include <phy_utils_var.h>
#include <phy_tssical.h>
#include <phy_lcn20_tssical.h>
#include "wlc_phy_iovar.h"
#include <phy_type_tssical.h>

/* module private states */
struct phy_lcn20_tssical_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_tssical_info_t *info;
};

#ifdef WLC_TXCAL
/* ******************************************* */
/* Functions used by common layer as callbacks */
/* ******************************************* */

static uint16 phy_lcn20_tssical_adjusted_tssi(phy_type_tssical_ctx_t *ctx, uint8 core_num);
static uint16 phy_lcn20_tssical_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx);
static void phy_lcn20_tssical_set_txpwrindex(phy_type_tssical_ctx_t *ctx, int16 gain_idx,
		uint16 save_TxPwrCtrlCmd);
static void phy_lcn20_tssical_restore_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx,
		uint16 save_TxPwrCtrlCmd, uint8 txpwr_ctrl_state);
static void phy_lcn20_tssical_iov_apply_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx, int int_val);
static void phy_lcn20_tssical_read_est_pwr_lut(phy_type_tssical_ctx_t *ctx,
		void *output_buff, uint8 core);
static void phy_lcn20_tssical_apply_paparams(phy_type_tssical_ctx_t *ctx);
static int32 phy_lcn20_tssical_get_tssi_cal(phy_type_tssical_ctx_t *ctx, int32 tssi_val);
static void phy_lcn20_tssical_apply_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx,
		wl_txcal_power_tssi_t *txcal_pwr_tssi, uint8 bphy_tbl);
#endif /* WLC_TXCAL */

/* register phy type specific implementation */
phy_lcn20_tssical_info_t *
BCMATTACHFN(phy_lcn20_tssical_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_tssical_info_t *info)
{
	phy_lcn20_tssical_info_t *tssicali;
	phy_type_tssical_fns_t fns;
	PHY_TRACE(("%s\n", __FUNCTION__));
	/* allocate all storage together */
	if ((tssicali = phy_malloc(pi, sizeof(*tssicali))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	tssicali->pi = pi;
	tssicali->lcn20i = lcn20i;
	tssicali->info = info;
	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = tssicali;
#ifdef WLC_TXCAL
	fns.adjusted_tssi = phy_lcn20_tssical_adjusted_tssi;
	fns.txpwr_ctrl_cmd = phy_lcn20_tssical_txpwr_ctrl_cmd;
	fns.set_txpwrindex = phy_lcn20_tssical_set_txpwrindex;
	fns.restore_txpwr_ctrl_cmd = phy_lcn20_tssical_restore_txpwr_ctrl_cmd;
	fns.iov_apply_pwr_tssi_tbl = phy_lcn20_tssical_iov_apply_pwr_tssi_tbl;
	fns.read_est_pwr_lut = phy_lcn20_tssical_read_est_pwr_lut;
	fns.apply_paparams = phy_lcn20_tssical_apply_paparams;
	fns.get_tssi_val = phy_lcn20_tssical_get_tssi_cal;
	fns.apply_pwr_tssi_tbl = phy_lcn20_tssical_apply_pwr_tssi_tbl;
#endif /* WLC_TXCAL */
	if (phy_tssical_register_impl(info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_txcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
	return tssicali;
	/* error handling */
fail:
	if (tssicali != NULL)
		phy_mfree(pi, tssicali, sizeof(*tssicali));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_tssical_unregister_impl)(phy_lcn20_tssical_info_t *tssicali)
{
	phy_info_t *pi;
	phy_tssical_info_t *info;
	ASSERT(tssicali);
	pi = tssicali->pi;
	info = tssicali->info;
	PHY_TRACE(("%s\n", __FUNCTION__));
	/* unregister from common */
	phy_tssical_unregister_impl(info);
	phy_mfree(pi, tssicali, sizeof(*tssicali));
}

#ifdef WLC_TXCAL
int8
wlc_phy_estpwrlut_intpol_lcn20phy(phy_info_t *pi, uint8 channel,
       wl_txcal_power_tssi_t *pwr_tssi_lut_ch1, wl_txcal_power_tssi_t *pwr_tssi_lut_ch2)
{
	uint32 *estpwr = NULL;
	uint32 *estpwr1 = NULL;
	uint32 *estpwr2 = NULL;
	int32 est_pwr_calc, est_pwr_calc1, est_pwr_calc2, est_pwr_intpol1, est_pwr_intpol2;
	uint8 core = 0, i;
	phytbl_info_t tab;
	/* Allocating all estpwr interpol buffers */
	if ((estpwr1 = (uint32*) LCN20PHY_MALLOC(pi, LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ
		* sizeof(*estpwr1)*3)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return BCME_NOMEM;
	}
	estpwr2 = estpwr1 + (LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ);
	estpwr = estpwr2 + (LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ);
	/* Interpolate between estpwrlut */
	wlc_phy_txcal_generate_estpwr_lut_lcn20phy(pwr_tssi_lut_ch1, estpwr1, core);
	wlc_phy_txcal_generate_estpwr_lut_lcn20phy(pwr_tssi_lut_ch2, estpwr2, core);
	for (i = 0; i < LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ; i++) {
		est_pwr_calc1 = *(estpwr1 + i) > 0xFF ?
			(int32) (*(estpwr1 + i) - 0x200) : *(estpwr1 + i);
		est_pwr_calc2 = *(estpwr2 + i) > 0xFF ?
			(int32) (*(estpwr2 + i) - 0x200) : *(estpwr2 + i);
		/* round to the nearest integer */
		est_pwr_intpol1 = 2*(channel - pwr_tssi_lut_ch1->channel)*(est_pwr_calc2 -
			est_pwr_calc1)/(pwr_tssi_lut_ch2->channel -
			pwr_tssi_lut_ch1->channel);
		est_pwr_intpol2 = (channel - pwr_tssi_lut_ch1->channel)*(est_pwr_calc2 -
		        est_pwr_calc1)/(pwr_tssi_lut_ch2->channel -
				pwr_tssi_lut_ch1->channel);
		est_pwr_calc = est_pwr_calc1 + est_pwr_intpol1 - est_pwr_intpol2;
		*(estpwr + i) = (uint32) (est_pwr_calc & 0x1FF);
	}
	tab.tbl_offset = 0;
	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;
	tab.tbl_ptr = estpwr;
	tab.tbl_len = LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ;
	wlc_lcn20phy_write_table(pi,  &tab);
	/* Free all estpwrlut interpol buffers */
	if (estpwr1) {
		LCN20PHY_MFREE(pi, estpwr1, LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ
			* sizeof(*estpwr1)*3);
	}
	return BCME_OK;
}

int
wlc_phy_txcal_generate_estpwr_lut_lcn20phy(wl_txcal_power_tssi_t
		*txcal_pwr_tssi, uint32 *estpwr, uint8 core)
{
	uint8 tssi_val, tssi_val_idx;
	int8 i = 0, k;
	int16 pwr_start = txcal_pwr_tssi->pwr_start[core];
	uint8 num_entries = txcal_pwr_tssi->num_entries[core];
	uint8 *tssi = txcal_pwr_tssi->tssi[core];
	int16 pwr_i;
	int32 est_pwr_calc;
	int32 num, den;
	for (tssi_val_idx = 0; tssi_val_idx < LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ; tssi_val_idx++) {
		tssi_val = tssi_val_idx * 2;
		for (k = num_entries-1; k >= 0; k--) {
			if (tssi[k] >= tssi_val) {
				i = k+1;
				if (i >= num_entries)
					i = num_entries-1;
				break;
			}
		}
		/* power in dot 3 format */
		pwr_i = pwr_start + 8*i;
		if (tssi[i-1] - tssi[i] == 0) {
			est_pwr_calc = pwr_i;
		} else {
			/* Using actual value for slope or pwr difference */
			/* Power difference is always 1dB = 8 in dot 3 format */
			num = (tssi_val - tssi[i])*(-8);
			den = tssi[i-1] - tssi[i];
			est_pwr_calc = pwr_i + (num + (num > 0 ? ABS(den) :
				-ABS(den))/2)/den;
		}
		/* Convert to qdBm for writing to LUT */
		if (est_pwr_calc > 254)
			est_pwr_calc = 254;
		else if (est_pwr_calc < -256)
			est_pwr_calc = -256;
		estpwr[tssi_val_idx] = (uint32) (est_pwr_calc & 0x1FF);
	}
	return BCME_OK;
}

uint8
wlc_phy_apply_pwr_tssi_tble_chan_lcn20phy(phy_info_t *pi)
{
	txcal_pwr_tssi_lut_t *LUT_pt;
	txcal_pwr_tssi_lut_t *LUT_root;
	uint8 chan_num = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint8 flag_chan_found = 0;
	txcal_root_pwr_tssi_t *pi_txcal_root_pwr_tssi_tbl =
			phy_tssical_get_root_pwr_tssi_tbl(pi->tssicali);
	LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G;
	if (LUT_root->txcal_pwr_tssi->channel == 0) {
		/* if no txcal table is present for 2G, apply paparam */
		phy_tssical_apply_pa_params(pi->tssicali);
		return BCME_OK;
	}
	LUT_pt = LUT_root;
	while (LUT_pt->next_chan != 0) {
		/* Go over all the entries in the list */
		if (LUT_pt->txcal_pwr_tssi->channel == chan_num) {
			flag_chan_found = 1;
			break;
		}
		if ((LUT_pt->txcal_pwr_tssi->channel < chan_num) &&
		    (LUT_pt->next_chan->txcal_pwr_tssi->channel > chan_num)) {
			flag_chan_found = 2;
			break;
		}
		LUT_pt = LUT_pt->next_chan;
	}
	if (LUT_pt->txcal_pwr_tssi->channel == chan_num) {
		/* In case only one entry is in the list */
		flag_chan_found = 1;
	}
	switch (flag_chan_found) {
	case 0:
		/* Channel not found in linked list or not between two channels */
		/* Then pick the closest one */
		if (chan_num < LUT_root->txcal_pwr_tssi->channel)
			LUT_pt = LUT_root;
		if (phy_tssical_apply_pwr_tssi_tbl(pi->tssicali, LUT_pt->txcal_pwr_tssi, 0) !=
				BCME_OK)
			return BCME_ERROR;
		phy_tssical_set_txcal_status(pi->tssicali, 2);
		break;
	case 1:
		/* Channel found */
		if (phy_tssical_apply_pwr_tssi_tbl(pi->tssicali, LUT_pt->txcal_pwr_tssi, 0) !=
				BCME_OK)
			return BCME_ERROR;
		phy_tssical_set_txcal_status(pi->tssicali, 1);
		break;
	case 2:
		/* Channel is in between two channels, do interpolation */
		/* ---- need to verify goodness of interpolation */
		if (wlc_phy_estpwrlut_intpol_lcn20phy(pi, chan_num,
			LUT_pt->txcal_pwr_tssi, LUT_pt->next_chan->txcal_pwr_tssi) != BCME_OK)
			return BCME_ERROR;
		phy_tssical_set_txcal_status(pi->tssicali, 2);
		break;
	}
	return BCME_OK;
}

static void
phy_lcn20_tssical_read_est_pwr_lut(phy_type_tssical_ctx_t *ctx, void *output_buff, uint8 core)
{
	phy_lcn20_tssical_info_t *tssicali = (phy_lcn20_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	uint32 estpwr[128];
	phytbl_info_t tab;
	BCM_REFERENCE(core);
	tab.tbl_offset = 0;
	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;
	tab.tbl_ptr = estpwr;
	tab.tbl_len = 128;
	wlc_lcn20phy_read_table(pi,  &tab);
	bcopy(estpwr, output_buff, 128*sizeof(uint32));
}

uint16
wlc_lcn20phy_adjusted_tssi(phy_info_t *pi)
{
	uint16 adj_tssi = 0;
	int16 tssi_OB, idletssi_OB;
	int16 tssi_reg, idletssi_reg;
	tssi_reg = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew4, avgTssi) & 0x1ff;
	if (tssi_reg >= 256)
		tssi_OB = tssi_reg - 256;
	else
		tssi_OB = tssi_reg + 256;
	idletssi_reg = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0) & 0x1ff;
	if (idletssi_reg >= 256)
		idletssi_OB = idletssi_reg - 256;
	else
		idletssi_OB = idletssi_reg + 256;
	adj_tssi = tssi_OB - idletssi_OB + ((1 << 9)-1);
	return adj_tssi;
}

static uint16
phy_lcn20_tssical_adjusted_tssi(phy_type_tssical_ctx_t *ctx, uint8 core_num)
{
	phy_lcn20_tssical_info_t *tssicali = (phy_lcn20_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	BCM_REFERENCE(core_num);
	return wlc_lcn20phy_adjusted_tssi(pi);
}

static uint16
phy_lcn20_tssical_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx)
{
	phy_lcn20_tssical_info_t *tssicali = (phy_lcn20_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	return phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlCmd);
}

static void
phy_lcn20_tssical_set_txpwrindex(phy_type_tssical_ctx_t *ctx, int16 gain_idx,
		uint16 save_TxPwrCtrlCmd)
{
	phy_lcn20_tssical_info_t *tssicali = (phy_lcn20_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	/* Set txpwrindex */
	wlc_phy_iovar_txpwrindex_set(pi, &gain_idx);
	/* Setting index, disables both Txpwrctrl and HWTXPwrCtrl */
	/* Enabling only Txpwrctrl and leaving HWTxPwrctrl disabled */
	phy_utils_write_phyreg(pi, LCN20PHY_TxPwrCtrlCmd,
		(save_TxPwrCtrlCmd & (~LCN20PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK)));
}

static void
phy_lcn20_tssical_restore_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx, uint16 save_TxPwrCtrlCmd,
		uint8 txpwr_ctrl_state)
{
	phy_lcn20_tssical_info_t *tssicali = (phy_lcn20_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	BCM_REFERENCE(txpwr_ctrl_state);
	phy_utils_write_phyreg(pi, LCN20PHY_TxPwrCtrlCmd, save_TxPwrCtrlCmd);
}

static void
phy_lcn20_tssical_apply_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx,
		wl_txcal_power_tssi_t *txcal_pwr_tssi, uint8 bphy_tbl)
{
	phy_lcn20_tssical_info_t *tssicali = (phy_lcn20_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	uint8 core = 0;
	uint32 estpwr_lcn20[128];
	phytbl_info_t tab;
	if (txcal_pwr_tssi->num_entries[core] == 0) {
		/* sanity check */
		phy_tssical_apply_pa_params(pi->tssicali);
	} else {
		wlc_phy_txcal_generate_estpwr_lut_lcn20phy(txcal_pwr_tssi,
			estpwr_lcn20, core);
		tab.tbl_offset = 0;
		tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;
		tab.tbl_ptr = estpwr_lcn20;
		tab.tbl_len = 128;
		wlc_lcn20phy_write_table(pi,  &tab);
	}
}

static void
phy_lcn20_tssical_apply_paparams(phy_type_tssical_ctx_t *ctx)
{
	phy_lcn20_tssical_info_t *tssicali = (phy_lcn20_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	uint32 tbl_len = 128;
	uint32 tbl_offset = 0;
	uint8 tssi_val;
	uint32 estpwr[128];
	int32 x1 = 0, y0 = 0, y1 = 0;
	phytbl_info_t tab;
	phy_tpc_get_paparams_for_band(pi, &x1, &y0, &y1);
	for (tssi_val = 0; tssi_val < 128; tssi_val++) {
		estpwr[tssi_val] = wlc_lcn20phy_tssi2dbm(tssi_val, x1, y0, y1);
	}
	tab.tbl_offset = tbl_offset;
	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;
	tab.tbl_ptr = estpwr;
	tab.tbl_len = tbl_len;
	wlc_lcn20phy_write_table(pi,  &tab);
}

static int32
phy_lcn20_tssical_get_tssi_cal(phy_type_tssical_ctx_t *ctx, int32 tssi_val)
{
	return ((tssi_val > 0 ? tssi_val+2 :	tssi_val-2)>>1);
}

static void
phy_lcn20_tssical_iov_apply_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx, int int_val)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	if (pi->sh->clk) {
		if (int_val) {
			/* Use Tx cal based pwr_tssi_tbl */
			wlc_phy_apply_pwr_tssi_tble_chan_lcn20phy(pi);
		} else {
			/* Use PA Params */
			phy_tssical_apply_pa_params(pi->tssicali);
		}
	}
	phy_tssical_set_pwr_tssi_tbl_in_use(pi->tssicali, int_val);
}
#endif /* WLC_TXCAL */
