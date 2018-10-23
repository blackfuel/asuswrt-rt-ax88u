/*
 * TxPowerCapl module implementation.
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
 * $Id: phy_txpwrcap.c 686983 2017-02-26 13:46:48Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include <phy_rstr.h>
#include <phy_type_txpwrcap.h>
#include <phy_txpwrcap_api.h>
#include <phy_txpwrcap.h>
#include <phy_utils_channel.h>
#include <phy_utils_var.h>

#if defined(WLC_TXPWRCAP)

/* ******* Local Functions ************ */
static int phy_txpwrcap_init(phy_init_ctx_t *ctx);
static void phy_txpwrcap_set_cellstatus(phy_txpwrcap_info_t *info,
	int mask, int value);

/* ********************************** */

/* module private states */
struct phy_txpwrcap_priv_info {
	phy_info_t *pi;
	phy_type_txpwrcap_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_txpwrcap_info_t info;
	phy_type_txpwrcap_fns_t fns;
	phy_txpwrcap_priv_info_t priv;
	phy_txpwrcap_data_t data;
/* add other variable size variables here at the end */
} phy_txpwrcap_mem_t;

/* local function declaration */

/* attach/detach */
phy_txpwrcap_info_t *
BCMATTACHFN(phy_txpwrcap_attach)(phy_info_t *pi)
{
	phy_txpwrcap_info_t *info;
	uint8 i;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_txpwrcap_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	info->priv = &((phy_txpwrcap_mem_t *)info)->priv;
	info->priv->pi = pi;
	info->priv->fns = &((phy_txpwrcap_mem_t *)info)->fns;
	info->data = &((phy_txpwrcap_mem_t *)info)->data;

	if ((info->data->txpwrcap_tbl =
		phy_malloc(pi, sizeof(wl_txpwrcap_tbl_t))) == NULL) {
		PHY_ERROR(("%s: txpwrcap_tbl malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* By default TxPwerCap state has CellOn state */
	info->data->txpwrcap_cellstatus = TXPWRCAP_CELLSTATUS_ON;

	for (i = 0; i < TXPWRCAP_MAX_NUM_CORES; i++) {
		info->data->txpwrcap_tbl->num_antennas_per_core[i] = 2;
	}
	for (i = 0; i < TXPWRCAP_MAX_NUM_ANTENNAS; i++) {
		info->data->txpwrcap_tbl->pwrcap_cell_off[i] = TXPOWERCAP_MAX_QDB;
		info->data->txpwrcap_tbl->pwrcap_cell_on[i] = TXPOWERCAP_MAX_QDB;
	}

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_txpwrcap_init,
		info, PHY_INIT_TXPWRCAP) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_txpwrcap_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_txpwrcap_detach)(phy_txpwrcap_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null txpwrcap module\n", __FUNCTION__));
		return;
	}

	pi = info->priv->pi;

	if (info->data->txpwrcap_tbl != NULL) {
		phy_mfree(pi, info->data->txpwrcap_tbl, sizeof(wl_txpwrcap_tbl_t));
	}

	phy_mfree(pi, info, sizeof(phy_txpwrcap_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_txpwrcap_register_impl)(phy_txpwrcap_info_t *ti, phy_type_txpwrcap_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ti->priv->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_txpwrcap_unregister_impl)(phy_txpwrcap_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* init Txpwrcap */
static int
WLBANDINITFN(phy_txpwrcap_init)(phy_init_ctx_t *ctx)
{
	phy_txpwrcap_info_t *ii = (phy_txpwrcap_info_t *)ctx;
	phy_type_txpwrcap_fns_t *fns = ii->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fns->init != NULL);
	return (fns->init)(fns->ctx);
}

int
wlc_phy_txpwrcap_tbl_set(wlc_phy_t *pih, wl_txpwrcap_tbl_t *txpwrcap_tbl)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;
	phy_type_txpwrcap_fns_t *fns = ti->priv->fns;
	uint8 i, j = 0, k;
	uint8 num_antennas = 0, max_antennas = 0, min_antennas = 0;
	uint16 phy_mode;
	uint8 core;

	/* Some Error Checking */
	phy_mode = phy_get_phymode(pi);
	if (phy_mode == PHYMODE_MIMO) {
		for (i = 0; i < TXPWRCAP_MAX_NUM_CORES; i++) {
			num_antennas += txpwrcap_tbl->num_antennas_per_core[i];
		}
		FOREACH_CORE(pi, core) {
			max_antennas += 2;
			min_antennas++;
		}
	} else if (phy_mode == PHYMODE_RSDB) { /* SDB mode */
		num_antennas += txpwrcap_tbl->num_antennas_per_core[phy_get_current_core(pi)];
		max_antennas += 2;
		min_antennas++;
	} else
		return BCME_ERROR;

	if ((num_antennas > max_antennas) || (num_antennas < min_antennas))
		return BCME_ERROR;

	for (i = 0; i < TXPWRCAP_MAX_NUM_CORES; i++) {
		ti->data->txpwrcap_tbl->num_antennas_per_core[i] =
			txpwrcap_tbl->num_antennas_per_core[i];
		for (k = 0; k < ti->data->txpwrcap_tbl->num_antennas_per_core[i]; k++) {
			ti->data->txpwrcap_tbl->pwrcap_cell_off[2*i+k] =
				txpwrcap_tbl->pwrcap_cell_off[j];
			ti->data->txpwrcap_tbl->pwrcap_cell_on[2*i+k] =
				txpwrcap_tbl->pwrcap_cell_on[j];
			j++;
		}
	}

	if (fns->txpwrcap_tbl_set)
		return (fns->txpwrcap_tbl_set)(fns->ctx);
	else
		return BCME_UNSUPPORTED;

}

int
wlc_phy_txpwrcap_tbl_get(wlc_phy_t *pih, wl_txpwrcap_tbl_t *txpwrcap_tbl)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;
	uint8 i, j = 0, k;

	for (i = 0; i < TXPWRCAP_MAX_NUM_CORES; i++) {
		txpwrcap_tbl->num_antennas_per_core[i] =
			ti->data->txpwrcap_tbl->num_antennas_per_core[i];
		for (k = 0; k < ti->data->txpwrcap_tbl->num_antennas_per_core[i]; k++) {
			txpwrcap_tbl->pwrcap_cell_off[j] =
				ti->data->txpwrcap_tbl->pwrcap_cell_off[2*i+k];
			txpwrcap_tbl->pwrcap_cell_on[j] =
				ti->data->txpwrcap_tbl->pwrcap_cell_on[2*i+k];
			j++;
		}
	}
	return BCME_OK;
}

int
phy_txpwrcap_cellstatus_override_set(phy_info_t *pi, int value)
{
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;

	if (value == -1)
		/* Clear the Force bit allowing WCI2 updates to take effect */
		phy_txpwrcap_set_cellstatus(ti,
			(TXPWRCAP_CELLSTATUS_FORCE_MASK |
			TXPWRCAP_CELLSTATUS_FORCE_UPD_MASK),
			0);
	else
		phy_txpwrcap_set_cellstatus(ti,
			(TXPWRCAP_CELLSTATUS_FORCE_MASK |
			TXPWRCAP_CELLSTATUS_FORCE_UPD_MASK |
			TXPWRCAP_CELLSTATUS_MASK),
			(TXPWRCAP_CELLSTATUS_FORCE_MASK |
			TXPWRCAP_CELLSTATUS_FORCE_UPD_MASK |
			(value & TXPWRCAP_CELLSTATUS_MASK)));
	return BCME_OK;

}

int wlc_phyhal_txpwrcap_get_cellstatus(wlc_phy_t *pih, int32* cellstatus)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;
	*cellstatus = (ti->data->txpwrcap_cellstatus & TXPWRCAP_CELLSTATUS_MASK);
	return BCME_OK;
}

void
wlc_phyhal_txpwrcap_set_cellstatus(wlc_phy_t *pih, int status)
{
	phy_info_t *pi = (phy_info_t*)pih;
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;
	phy_txpwrcap_set_cellstatus(ti, TXPWRCAP_CELLSTATUS_WCI2_MASK,
		(status & 1) << TXPWRCAP_CELLSTATUS_WCI2_NBIT);
}

static void
phy_txpwrcap_set_cellstatus(phy_txpwrcap_info_t *info, int mask, int value)
{
	phy_txpwrcap_info_t *ti = info;

	phy_info_t *pi = ti->priv->pi;
	phy_type_txpwrcap_fns_t *fns = ti->priv->fns;

	int txpwrcap_upreqd = FALSE;
	int cellstatus_new, cellstatus_cur;

	ti->data->txpwrcap_cellstatus &= ~(mask);
	value &= mask;
	ti->data->txpwrcap_cellstatus |= value;

	/* CELLSTATUS_FORCE_UPD => Update forced, either by iovar or at init time
	 * CELLSTATUS_FORCE => value forced by iovar, ignore value from WCI2
	 * else compare value from WCI2 (cellstatus_new) and current value (cellstatus_cur)
	 * to determine if update needed
	 */
	if (ti->data->txpwrcap_cellstatus & TXPWRCAP_CELLSTATUS_FORCE_UPD_MASK) {
		ti->data->txpwrcap_cellstatus &= ~(TXPWRCAP_CELLSTATUS_FORCE_UPD_MASK);
		txpwrcap_upreqd = TRUE;
	}
	else if (!(ti->data->txpwrcap_cellstatus & TXPWRCAP_CELLSTATUS_FORCE_MASK)) {
		cellstatus_cur = (ti->data->txpwrcap_cellstatus & TXPWRCAP_CELLSTATUS_MASK)
			>> TXPWRCAP_CELLSTATUS_NBIT;
		cellstatus_new = (ti->data->txpwrcap_cellstatus & TXPWRCAP_CELLSTATUS_WCI2_MASK)
			>> TXPWRCAP_CELLSTATUS_WCI2_NBIT;
		/* Note if status change, as need to update PHY */
		if (cellstatus_cur != cellstatus_new) {
			ti->data->txpwrcap_cellstatus &= ~(1 << TXPWRCAP_CELLSTATUS_NBIT);
			ti->data->txpwrcap_cellstatus |=
				(cellstatus_new << TXPWRCAP_CELLSTATUS_NBIT);
			txpwrcap_upreqd = TRUE;
		}
	}

	if (txpwrcap_upreqd && pi->sh->clk) {
		/* PHY update required */
#ifdef WLC_SW_DIVERSITY
		if (PHYSWDIV_ENAB(pi)) {
			wlapi_swdiv_cellstatus_notif(pi->sh->physhim,
				(ti->data->txpwrcap_cellstatus & TXPWRCAP_CELLSTATUS_MASK));
		}
#endif // endif
		if (fns->txpwrcap_set) {
			(fns->txpwrcap_set)(fns->ctx);
		}
	}
}

uint32
phy_txpwrcap_get_caps_inuse(phy_info_t *pi)
{
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;
	phy_type_txpwrcap_fns_t *fns = ti->priv->fns;

	if (fns->txpwrcap_in_use) {
		return (fns->txpwrcap_in_use)(fns->ctx);
	} else
		return 0;
}

bool
wlc_phy_txpwrcap_get_enabflg(wlc_phy_t *pih)
{
	return (((phy_info_t *)pih)->txpwrcapi->data->_txpwrcap);
}

#ifdef WLC_SW_DIVERSITY
uint8
wlc_phy_get_current_core(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	return phy_get_current_core(pi);
}
void
wlc_phy_txpwrcap_to_shm(wlc_phy_t *pih, uint16 tx_ant, uint16 rx_ant)
{
	phy_info_t *pi = (phy_info_t*)pih;
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;
	phy_type_txpwrcap_fns_t *fns = ti->priv->fns;

	if (fns->txpwrcap_to_shm)
		(fns->txpwrcap_to_shm)(fns->ctx, tx_ant, rx_ant);
}
#endif /* WLC_SW_DIVERSITY */

int8
phy_txpwrcap_tbl_get_max_percore(phy_info_t *pi, uint8 core)
{
	phy_txpwrcap_info_t *ti = pi->txpwrcapi;

	uint8 k;
	int8 maxpwr = 127;

	if (!PHY_TXPWRCAP_ENAB(pi->sh->physhim))
		return maxpwr;

	for (k = 0; k < ti->data->txpwrcap_tbl->num_antennas_per_core[core]; k++) {
		/* Get the min of txpwrcap for all cores/cell on/off */
		if (ti->data->txpwrcap_tbl->pwrcap_cell_off[2*core+k] != -128) {
			maxpwr = MIN(maxpwr, ti->data->txpwrcap_tbl->pwrcap_cell_off[2*core+k]);
		}
		if (ti->data->txpwrcap_tbl->pwrcap_cell_on[2*core+k] != -128) {
			maxpwr = MIN(maxpwr, ti->data->txpwrcap_tbl->pwrcap_cell_on[2*core+k]);
		}
	}
	return maxpwr;
}

#endif /* WLC_TXPWRCAP */
