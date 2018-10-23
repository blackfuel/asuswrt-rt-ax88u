
/*
 * ACPHY TxPowerCap module implementation
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
 * $Id: phy_ac_txpwrcap.c 691048 2017-03-20 16:47:17Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <phy_type_txpwrcap.h>
#include <phy_txpwrcap_api.h>
#include <phy_ac_txpwrcap.h>
#include <phy_txpwrcap.h>
#include <phy_utils_reg.h>
#include <wlc_phyreg_ac.h>
#ifdef WLC_SW_DIVERSITY
#include "phy_ac_antdiv.h"
#endif // endif
#include <phy_stf.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_hal.h>
/* TODO: all these are going away... > */
#endif // endif

#include <bcmdevs.h>

#ifdef WLC_TXCAL
#include <phy_tssical.h>
#endif /* WLC_TXCAL */

#define ACPHY_TXPOWERCAP_PARAMS	(3)

/* module private states */
struct phy_ac_txpwrcap_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_txpwrcap_info_t *ti;
	/* std params */
	/* add other variable size variables here at the end */
};

#if defined(WLC_TXPWRCAP)

/* local functions */
static bool wlc_phy_txpwrcap_attach_acphy(phy_ac_txpwrcap_info_t *ti);
static int wlc_phy_txpwrcap_init_acphy(phy_type_txpwrcap_ctx_t *ctx);
static int wlc_phy_txpwrcap_tbl_set_acphy(phy_type_txpwrcap_ctx_t *ctx);

#ifdef WLC_SW_DIVERSITY
static void wlc_phy_txpwrcap_to_shm_acphy(phy_type_txpwrcap_ctx_t *ctx,
	uint16 tx_ant, uint16 rx_ant);
#endif // endif
static void wlc_phy_txpwrcap_set_acphy_sdb(phy_info_t *pi, int8* txcap,
	uint16 tx_ant, uint16 rx_ant);
static uint32 wlc_phy_get_txpwrcap_inuse_acphy(phy_type_txpwrcap_ctx_t *ctx);
static int wlc_phy_txpwrcap_check_in_limits_acphy(phy_info_t *pi,
	int8 txpwrcaptbl[WLC_TXCORE_MAX], uint8 core);
static void _phy_ac_txpwrcap_set(phy_type_txpwrcap_ctx_t *ctx);

/* External functions */

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_txpwrcap_info_t *
BCMATTACHFN(phy_ac_txpwrcap_register_impl)(phy_info_t *pi,
	phy_ac_info_t *aci, phy_txpwrcap_info_t *ti)
{
	phy_ac_txpwrcap_info_t *info;
	phy_type_txpwrcap_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_txpwrcap_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = wlc_phy_txpwrcap_init_acphy;
	fns.txpwrcap_tbl_set = wlc_phy_txpwrcap_tbl_set_acphy;
	fns.txpwrcap_set = _phy_ac_txpwrcap_set;
#ifdef WLC_SW_DIVERSITY
	fns.txpwrcap_to_shm = wlc_phy_txpwrcap_to_shm_acphy;
#endif // endif
	fns.txpwrcap_in_use = wlc_phy_get_txpwrcap_inuse_acphy;
	fns.ctx = info;

	if (!wlc_phy_txpwrcap_attach_acphy(info))
		goto fail;

	phy_txpwrcap_register_impl(ti, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_txpwrcap_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_txpwrcap_unregister_impl)(phy_ac_txpwrcap_info_t *info)
{
	phy_info_t *pi;
	phy_txpwrcap_info_t *ti;

	ASSERT(info);
	pi = info->pi;
	ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_txpwrcap_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_ac_txpwrcap_info_t));
}

static bool
BCMATTACHFN(wlc_phy_txpwrcap_attach_acphy)(phy_ac_txpwrcap_info_t *ac_ti)
{
	ac_ti->ti->data->_txpwrcap = TRUE;
	return TRUE;
}

static int
wlc_phy_txpwrcap_init_acphy(phy_type_txpwrcap_ctx_t *ctx)
{
	phy_ac_txpwrcap_info_t *ac_ti = (phy_ac_txpwrcap_info_t *)ctx;
	phy_txpwrcap_info_t *ti = ac_ti->ti;
	phy_info_t *pi = ac_ti->pi;
	/* Write failsafe caps in shms */
	/* Choose pwrcap value for Ant 0 on core 0 for failsafe */
	wlapi_bmac_write_shm(pi->sh->physhim,
		M_TXPWRCAP_C0_FS(pi),
		ti->data->txpwrcap_tbl->pwrcap_cell_on[0] - pi->tx_pwr_backoff);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		wlapi_bmac_write_shm(pi->sh->physhim,
			M_TXPWRCAP_C1_FS(pi),
			ti->data->txpwrcap_tbl->pwrcap_cell_on[2] - pi->tx_pwr_backoff);
	return BCME_OK;
}

static void
_phy_ac_txpwrcap_set(phy_type_txpwrcap_ctx_t *ctx)
{
	phy_ac_txpwrcap_set((phy_ac_txpwrcap_info_t *)ctx);
}

void
phy_ac_txpwrcap_set(phy_ac_txpwrcap_info_t *ac_ti)
{
	phy_txpwrcap_info_t *ti = ac_ti->ti;
	phy_info_t *pi = ac_ti->pi;
	uint core;
	int8 txpwrcap[WLC_TXCORE_MAX] = {TXPOWERCAP_MAX_QDB,
		TXPOWERCAP_MAX_QDB,
		TXPOWERCAP_MAX_QDB,
		TXPOWERCAP_MAX_QDB};
	int8 *txcap = NULL;
	uint16 cap_core0, cap_core1;
	uint16 phy_mode;
	uint16 tx_ant = 0, rx_ant = 0;
#ifdef WLC_SW_DIVERSITY
	uint8 ant = 0;
	tx_ant = ANTDIV_ANTMAP_NOCHANGE;
	rx_ant = ANTDIV_ANTMAP_NOCHANGE;
#endif // endif
	int32 cellstatus;

	if (!PHY_TXPWRCAP_ENAB(pi->sh->physhim))
		return;

	ASSERT(pi->sh->clk);
	phy_mode = phy_get_phymode(pi);

	(void)wlc_phyhal_txpwrcap_get_cellstatus((wlc_phy_t *)pi, &cellstatus);

	if (cellstatus)
		txcap = ti->data->txpwrcap_tbl->pwrcap_cell_on;
	else
		txcap = ti->data->txpwrcap_tbl->pwrcap_cell_off;

	if (phy_mode == PHYMODE_MIMO) {
		FOREACH_CORE(pi, core) {
			if (!ti->data->txpwrcap_tbl->num_antennas_per_core[core]) {
				PHY_ERROR(("%s: Tx Power Caps not provided for core %d antennas\n",
					__FUNCTION__, core));
			  break;
			}
			else if	(ti->data->txpwrcap_tbl->num_antennas_per_core[core] == 1)
				txpwrcap[core] = txcap[core*2];
#ifdef WLC_SW_DIVERSITY
			else {
				if (PHYSWDIV_ENAB(pi)) {
					if (phy_ac_swdiv_is_supported(pi->u.pi_acphy->antdivi,
							core, TRUE)) {
						/* Based on swOvr, choose either 0 or 1 antennae
						 * for core 0
						 */
						ant = phy_ac_swdiv_get_rxant_bycoreidx(
							pi->u.pi_acphy->antdivi, core);
						txpwrcap[core] = txcap[(core *
							TXPOWERCAP_MAX_ANT_PER_CORE) +
							ant ? 1 : 0];
					} else {
						/* Currently support just one antenna for all
						 * cores other than 0
						 */
						txpwrcap[core] = txcap[core *
							TXPOWERCAP_MAX_ANT_PER_CORE];
					}
				}
			}
#endif /* WLC_SW_DIVERSITY */
		}
		cap_core0 = (uint16)(txpwrcap[0] - pi->tx_pwr_backoff);
		cap_core1 = (uint16)(txpwrcap[1] - pi->tx_pwr_backoff);
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_C0(pi),
			cap_core0);
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
			wlapi_bmac_write_shm(pi->sh->physhim,
					M_TXPWRCAP_C1(pi), cap_core1);
#ifdef WL_UCM
		phy_btcx_ucm_set_txpwrcap_orig(pi->btcxi, txpwrcap);
#endif /* WL_UCM */

#ifdef WLC_SW_DIVERSITY
		/* Add another shared memory here if sw div is available on other cores */
		if (PHYSWDIV_ENAB(pi)) {
			phy_ac_swdiv_txpwrcap_shmem_set(pi->u.pi_acphy->antdivi,
				0, cap_core0, cap_core0, tx_ant, rx_ant);
		}
#endif // endif
	} else if (phy_mode == PHYMODE_RSDB) {
		wlc_phy_txpwrcap_set_acphy_sdb(pi, txcap, tx_ant, rx_ant);
	}
}

/* This function is called in SDB mode to program TxCap values independently for each SDB core */
static void
wlc_phy_txpwrcap_set_acphy_sdb(phy_info_t *pi, int8* txcap,
	uint16 tx_ant, uint16 rx_ant)
{
	uint8 core; uint8 chain;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_txpwrcap_info_t *ac_ti = pi_ac->txpwrcapi;
	phy_txpwrcap_info_t *ti = ac_ti->ti;
	uint16 cap0, cap1;
	int8 txpwrcap[WLC_TXCORE_MAX] = {TXPOWERCAP_MAX_QDB,
		TXPOWERCAP_MAX_QDB,
		TXPOWERCAP_MAX_QDB,
		TXPOWERCAP_MAX_QDB};
#ifdef WLC_SW_DIVERSITY
	bool swdiv_supported = FALSE;
	uint8 ant = 0;
#endif /* WLC_SW_DIVERSITY */
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	core = phy_get_current_core(pi);

	FOREACH_CORE(pi, chain) {
#ifdef WLC_SW_DIVERSITY
	swdiv_supported = phy_ac_swdiv_is_supported(pi->u.pi_acphy->antdivi,
		ACPHY_PHYCORE_ANY, TRUE);
#endif /* WLC_SW_DIVERSITY */
	if (!ti->data->txpwrcap_tbl->num_antennas_per_core[core * 2 + chain]) {
		PHY_ERROR(("%s: Tx Power Caps not provided for core %d chain %d antennas\n",
			__FUNCTION__, core, chain));
	} else if (ti->data->txpwrcap_tbl->num_antennas_per_core[core * 2 + chain] == 1)
		txpwrcap[chain] = txcap[(core * 2 + chain) * TXPOWERCAP_MAX_ANT_PER_CORE];
#ifdef WLC_SW_DIVERSITY
	else {
		if (swdiv_supported) {
			/* Based on swOvr, choose either 0 or 1 antennae for core */
			ant = phy_ac_swdiv_get_rxant_bycoreidx(pi->u.pi_acphy->antdivi, core);
			txpwrcap[core] = txcap[(core * TXPOWERCAP_MAX_ANT_PER_CORE) +
					(ant ? 1 : 0)];
		} else {
			/* Currently support just one antenna for all cores other than 0 */
			txpwrcap[core] = txcap[core * TXPOWERCAP_MAX_ANT_PER_CORE];
		}
	}
#endif /* WLC_SW_DIVERSITY */
	}

	cap0 = (uint16)(txpwrcap[0] - pi->tx_pwr_backoff);
	cap1 = (uint16)(txpwrcap[1] - pi->tx_pwr_backoff);
	wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_C0(pi), cap0);
	wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_C1(pi), cap1);

#ifdef WL_UCM
	phy_btcx_ucm_set_txpwrcap_orig(pi->btcxi, txpwrcap);
#endif /* WL_UCM */

#ifdef WLC_SW_DIVERSITY
	if (swdiv_supported) {
		phy_ac_swdiv_txpwrcap_shmem_set(pi->u.pi_acphy->antdivi, core, cap0, cap0,
			tx_ant, rx_ant);
	}
#endif /* WLC_SW_DIVERSITY */
}

uint32
wlc_phy_get_txpwrcap_inuse_acphy(phy_type_txpwrcap_ctx_t *ctx)
{
	phy_ac_txpwrcap_info_t *info = (phy_ac_txpwrcap_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 cap_in_use = 0;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	ASSERT(pi->sh->clk);

	IF_ACTV_CORE(pi, stf_shdata->phytxchain, 0)
		cap_in_use = READ_PHYREGFLD(pi, TxPwrCapping_path0, maxTxPwrCap_path0);

	IF_ACTV_CORE(pi, stf_shdata->phytxchain, 1)
		cap_in_use = cap_in_use | (READ_PHYREGFLD(pi, TxPwrCapping_path1,
			maxTxPwrCap_path1) << 8);

	IF_ACTV_CORE(pi, stf_shdata->phytxchain, 2)
		cap_in_use = cap_in_use | (READ_PHYREGFLD(pi, TxPwrCapping_path2,
			maxTxPwrCap_path2) << 16);

	return cap_in_use;
}

static int
wlc_phy_txpwrcap_tbl_set_acphy(phy_type_txpwrcap_ctx_t *ctx)
{

	phy_ac_txpwrcap_info_t *ac_ti = (phy_ac_txpwrcap_info_t *)ctx;
	phy_txpwrcap_info_t *ti = ac_ti->ti;
	phy_info_t *pi = ac_ti->pi;
	uint8 default_ant_core0 = 0;	/* no swdiv default */
	uint16 failsafe_pwrcap_ant;
	uint core;
	uint16 phy_mode;
	int err = BCME_OK;

	phy_mode = phy_get_phymode(pi);

	if (phy_mode == PHYMODE_MIMO) {
		FOREACH_CORE(pi, core) {
			err = wlc_phy_txpwrcap_check_in_limits_acphy(pi,
					ti->data->txpwrcap_tbl->pwrcap_cell_on, core);
			if (err)
				return BCME_ERROR;
			err = wlc_phy_txpwrcap_check_in_limits_acphy(pi,
					ti->data->txpwrcap_tbl->pwrcap_cell_off, core);
			if (err)
				return BCME_ERROR;
		}
	} else if (phy_mode == PHYMODE_RSDB) {
		core = phy_get_current_core(pi);
		err = wlc_phy_txpwrcap_check_in_limits_acphy(pi,
				ti->data->txpwrcap_tbl->pwrcap_cell_on, core);
		if (err)
			return BCME_ERROR;
		err = wlc_phy_txpwrcap_check_in_limits_acphy(pi,
				ti->data->txpwrcap_tbl->pwrcap_cell_off, core);
		if (err)
			return BCME_ERROR;
	}

	/* Apply the new caps */
	if (pi->sh->clk) {
		phy_ac_txpwrcap_set(ctx);

		/* Write failsafe caps in shms */
#ifdef WLC_SW_DIVERSITY
		if (PHYSWDIV_ENAB(pi)) {
			default_ant_core0 = wlapi_swdiv_get_default_ant(pi->sh->physhim, 0);
		}
#endif // endif
		failsafe_pwrcap_ant =
			((ti->data->txpwrcap_tbl->pwrcap_cell_on[default_ant_core0]
			- pi->tx_pwr_backoff) << 8) | default_ant_core0;
		wlapi_bmac_write_shm(pi->sh->physhim,
				M_TXPWRCAP_C0_FSANT(pi),
				failsafe_pwrcap_ant);
		wlapi_bmac_write_shm(pi->sh->physhim,
				M_TXPWRCAP_C0_FS(pi),
				ti->data->txpwrcap_tbl->pwrcap_cell_on[default_ant_core0] -
				pi->tx_pwr_backoff);
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
			wlapi_bmac_write_shm(pi->sh->physhim,
				M_TXPWRCAP_C1_FS(pi),
				ti->data->txpwrcap_tbl->pwrcap_cell_on[2] - pi->tx_pwr_backoff);
	}
	return BCME_OK;
}

static int
wlc_phy_txpwrcap_check_in_limits_acphy(phy_info_t *pi,
	int8 txpwrcaptbl[WLC_TXCORE_MAX], uint8 core)
{
	int8 txpwrcap[WLC_TXCORE_MAX] = { TXPOWERCAP_MAX_QDB, TXPOWERCAP_MAX_QDB,
			TXPOWERCAP_MAX_QDB, TXPOWERCAP_MAX_QDB};
	uint8 loop_cnt = 0, i = 0;
	bool swdiv_supported = FALSE;
	BCM_REFERENCE(txpwrcap);
	BCM_REFERENCE(swdiv_supported);
	BCM_REFERENCE(loop_cnt);
	BCM_REFERENCE(i);
#ifdef WLC_SW_DIVERSITY
	if (PHYSWDIV_ENAB(pi)) {
		swdiv_supported = phy_ac_swdiv_is_supported(pi->u.pi_acphy->antdivi, core, TRUE);
	}
#endif /* WLC_SW_DIVERSITY */
	txpwrcap[loop_cnt++] = txpwrcaptbl[(core * TXPOWERCAP_MAX_ANT_PER_CORE)];
	if (swdiv_supported) {
		txpwrcap[loop_cnt++] = txpwrcaptbl[(core * TXPOWERCAP_MAX_ANT_PER_CORE) + 1];
	}
#ifdef WLC_TXCAL
	/* Check if tx powercap is lower than the limit with OLPC on */
	if (!phy_tssical_get_disable_olpc(pi->tssicali)) {
		/* TXCAL Data based OLPC */
		if (phy_tssical_get_olpc_idx_valid_in_use(pi->tssicali)) {
			for (i = 0; i < loop_cnt; ++i) {
				if (txpwrcap[i] < ACPHY_MIN_POWER_SUPPORTED_WITH_OLPC_QDBM) {
					PHY_FATAL_ERROR_MESG(("core%d, txcap = %d\n",
						core, txpwrcap[i]));
					PHY_FATAL_ERROR(pi, PHY_RC_TXPOWER_LIMITS);
					return BCME_ERROR;
				}
			}
		}
	}
#endif /* WLC_TXCAL */
	return BCME_OK;
}

#ifdef WLC_SW_DIVERSITY
static void
wlc_phy_txpwrcap_to_shm_acphy(phy_type_txpwrcap_ctx_t *ctx, uint16 tx_ant, uint16 rx_ant)
{
	phy_ac_txpwrcap_info_t *ac_ti = (phy_ac_txpwrcap_info_t *)ctx;
	phy_txpwrcap_info_t *ti = ac_ti->ti;
	phy_info_t *pi = ac_ti->pi;
	int8 *txcap = NULL;
	uint8 core;
	uint8 cap_tx, cap_rx;
	uint16 cap_core0, cap_core1;
	uint16 phy_mode;
	int32 cellstatus;

	(void)wlc_phyhal_txpwrcap_get_cellstatus((wlc_phy_t *)pi, &cellstatus);

	if (cellstatus)
		txcap = ti->data->txpwrcap_tbl->pwrcap_cell_on;
	else
		txcap = ti->data->txpwrcap_tbl->pwrcap_cell_off;

	phy_mode = phy_get_phymode(pi);
	if (phy_mode == PHYMODE_MIMO) {
		/* need to extend when uCode support multi-core */
		core = 0;
		/* Use cap corresponding to RX Ant for M_LTECX_PWRCP_C0_OFFSET */
		cap_core0 = (uint16)(txcap[rx_ant & (1 << core) ? 1 : 0]
			- pi->tx_pwr_backoff);
		cap_core1 = (uint16)(txcap[2] - pi->tx_pwr_backoff);

		wlapi_bmac_write_shm(pi->sh->physhim,
			M_TXPWRCAP_C0(pi), cap_core0);
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
			wlapi_bmac_write_shm(pi->sh->physhim,
					M_TXPWRCAP_C1(pi), cap_core1);
		}
		cap_tx = (uint8) (txcap[(tx_ant & (1 << core)) ? 1 : 0]
			- pi->tx_pwr_backoff);
		cap_rx = (uint8) (txcap[(rx_ant & (1 << core)) ? 1 : 0]
			- pi->tx_pwr_backoff);
		/* multi core support will be limited by ucode impl.
		 * currently core0 only
		 */
		phy_ac_swdiv_txpwrcap_shmem_set(pi->u.pi_acphy->antdivi,
			core, cap_tx, cap_rx, tx_ant, rx_ant);
	} else if (phy_mode == PHYMODE_RSDB) {
		wlc_phy_txpwrcap_set_acphy_sdb(pi, txcap, tx_ant, rx_ant);
	}
}
#endif /* WLC_SW_DIVERSITY */

#endif /* WLC_TXPWRCAP */
