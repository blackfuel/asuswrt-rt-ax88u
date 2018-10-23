/*
 * STF phy module implementation
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: phy_stf.c 659995 2016-09-16 22:23:48Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_stf.h>
#include <phy_stf.h>
#include <phy_stf_api.h>

/* module private states */
struct phy_stf_info {
	phy_info_t *pi;
	phy_type_stf_fns_t *fns;
	phy_stf_data_t	*data; /* shared data */
};

/* module private states memory layout */
typedef struct {
	phy_stf_info_t stf_info;
	phy_type_stf_fns_t fns;
	phy_stf_data_t	data;
/* add other variable size variables here at the end */
} phy_stf_mem_t;

/* function prototypes */

/* attach/detach */
phy_stf_info_t *
BCMATTACHFN(phy_stf_attach)(phy_info_t *pi)
{
	phy_stf_info_t *stf_info;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((stf_info = phy_malloc(pi, sizeof(phy_stf_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	stf_info->pi = pi;
	stf_info->fns = &((phy_stf_mem_t *)stf_info)->fns;
	stf_info->data = &((phy_stf_mem_t *)stf_info)->data;

	return stf_info;

	/* error */
fail:
	phy_stf_detach(stf_info);
	return NULL;
}

void

BCMATTACHFN(phy_stf_detach)(phy_stf_info_t *stf_info)
{
	phy_info_t *pi;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (stf_info == NULL) {
		PHY_INFORM(("%s: null stf module\n", __FUNCTION__));
		return;
	}

	pi = stf_info->pi;
	phy_mfree(pi, stf_info, sizeof(phy_stf_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_stf_register_impl)(phy_stf_info_t *stf_info,
	phy_type_stf_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*stf_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_stf_unregister_impl)(phy_stf_info_t *stf_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/* ****       Functions used by other PHY modules    **** */
phy_stf_data_t *
phy_stf_get_data(phy_stf_info_t *stfi)
{
	ASSERT(stfi);
	ASSERT(stfi->data);

	return stfi->data;
}

int
phy_stf_set_phyrxchain(phy_stf_info_t *stfi, uint8 phyrxchain)
{
	ASSERT(stfi);
	ASSERT(stfi->data);

	stfi->data->phyrxchain = phyrxchain;
	return BCME_OK;
}

int
phy_stf_set_phytxchain(phy_stf_info_t *stfi, uint8 phytxchain)
{
	ASSERT(stfi);
	ASSERT(stfi->data);

	stfi->data->phytxchain = phytxchain;
	return BCME_OK;
}

int
phy_stf_set_hwphyrxchain(phy_stf_info_t *stfi, uint8 hwphyrxchain)
{
	ASSERT(stfi);
	ASSERT(stfi->data);

	stfi->data->hw_phyrxchain = hwphyrxchain;
	return BCME_OK;
}

int
phy_stf_set_hwphytxchain(phy_stf_info_t *stfi, uint8 hwphytxchain)
{
	ASSERT(stfi);
	ASSERT(stfi->data);

	stfi->data->hw_phytxchain = hwphytxchain;
	return BCME_OK;
}

/* ********************************************* */
/*	                      PHY HAL functions                             */
/* ********************************************* */
int
phy_stf_chain_set(wlc_phy_t *pih, uint8 txchain, uint8 rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;
	phy_stf_info_t *stfi = NULL;
	phy_type_stf_fns_t *fns = NULL;
	phy_type_stf_ctx_t *ctx = NULL;

	ASSERT(pi->stfi);
	ASSERT(pi->stfi->fns);
	ASSERT(pi->stfi->fns->ctx);

	stfi = pi->stfi;
	fns = stfi->fns;
	ctx = fns->ctx;

	PHY_TRACE(("phy_stf_chain_set, new phy chain tx %d, rx %d", txchain, rxchain));

	if (fns->set_stf_chain) {
		return fns->set_stf_chain(ctx, txchain, rxchain);
	} else {
		PHY_ERROR(("phy_stf_chain_set called for unsupported phy."));
		ASSERT(0);
		return BCME_UNSUPPORTED;
	}
	return BCME_OK;
}

int
phy_stf_chain_init(wlc_phy_t *pih, uint8 txchain, uint8 rxchain)
{
	int err = BCME_OK;
	phy_info_t *pi = (phy_info_t*)pih;
	phy_stf_info_t *stfi = NULL;
	phy_type_stf_fns_t *fns = NULL;
	phy_stf_data_t	*data = NULL;
	phy_type_stf_ctx_t *ctx = NULL;

	ASSERT(pi->stfi);
	ASSERT(pi->stfi->fns);
	ASSERT(pi->stfi->data);
	ASSERT(pi->stfi->fns->ctx);

	stfi = pi->stfi;
	fns = stfi->fns;
	data = stfi->data;
	ctx = fns->ctx;

	data->hw_phytxchain = txchain;
	data->hw_phyrxchain = rxchain;

	if (fns->chain_init) {
		err = fns->chain_init(ctx, pi->sromi->sr13_en_sw_txrxchain_mask,
				txchain, rxchain);
	} else {
		data->phytxchain = txchain;
		data->phyrxchain = rxchain;
	}
	return err;
}

int
phy_stf_chain_get(wlc_phy_t *pih, uint8 *txchain, uint8 *rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;
	phy_stf_data_t	*data = NULL;

	ASSERT(pi->stfi);
	ASSERT(pi->stfi->data);

	data = pi->stfi->data;

	*txchain = data->phytxchain;
	*rxchain = data->phyrxchain;

	return BCME_OK;
}

uint16
phy_stf_duty_cycle_chain_active_get(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	return phy_temp_throttle(pi->tempi);
}
