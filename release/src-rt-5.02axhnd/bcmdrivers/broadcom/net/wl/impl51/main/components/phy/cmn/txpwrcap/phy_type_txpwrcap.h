/*
 * TxPowerCtrl module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_txpwrcap.h 687587 2017-03-01 20:11:10Z $
 */

#ifndef _phy_type_txpwrcap_h_
#define _phy_type_txpwrcap_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_txpwrcap.h>
#include <phy_txpwrcap_api.h>

typedef struct phy_txpwrcap_priv_info phy_txpwrcap_priv_info_t;

typedef struct phy_txpwrcap_data {
	/* Tx Power cap vars */
	wl_txpwrcap_tbl_t *txpwrcap_tbl;
	bool	txpwrcap_cellstatus;
	bool	_txpwrcap;	/* enable/disable Tx power cap feature */
} phy_txpwrcap_data_t;

struct phy_txpwrcap_info {
	phy_txpwrcap_priv_info_t *priv;
	phy_txpwrcap_data_t *data;
};

/* Tx Pwr Cap Support */
#ifdef WLC_TXPWRCAP
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define PHY_TXPWRCAP_ENAB(physhim)   (wlapi_txpwrcap_enab_check(physhim))
#elif defined(WLC_TXPWRCAP_DISABLED)
	#define PHY_TXPWRCAP_ENAB(physhim)   0
#else
	#define PHY_TXPWRCAP_ENAB(physhim)   (wlapi_txpwrcap_enab_check(physhim))
#endif // endif
#else
	#define PHY_TXPWRCAP_ENAB(physhim)   0
#endif	/* WLC_TXPWRCAP */

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_txpwrcap_ctx_t;

typedef int (*phy_type_txpwrcap_init_fn_t)(phy_type_txpwrcap_ctx_t *ctx);
typedef int (*phy_type_txpwrcap_tbl_set_fn_t)(phy_type_txpwrcap_ctx_t *ctx);
typedef void (*phy_type_txpwrcap_to_shm_fn_t)(phy_type_txpwrcap_ctx_t *ctx,
	uint16 tx_ant, uint16 rx_ant);
typedef uint32 (*phy_type_txpwrcap_in_use_fn_t)(phy_type_txpwrcap_ctx_t *ctx);
typedef void (*phy_type_txpwrcap_set_fn_t)(phy_type_txpwrcap_ctx_t *ctx);

typedef struct {
	/* init module including h/w */
	phy_type_txpwrcap_init_fn_t init;
	/* Txpwrcap Table Set */
	phy_type_txpwrcap_tbl_set_fn_t txpwrcap_tbl_set;
	/* Txpwrcap Set */
	phy_type_txpwrcap_set_fn_t txpwrcap_set;
	/* txpwrcap values in shm along with diversity */
	phy_type_txpwrcap_to_shm_fn_t txpwrcap_to_shm;
	/* txpwrcap values in shm along with diversity */
	phy_type_txpwrcap_in_use_fn_t txpwrcap_in_use;
	/* context */
	phy_type_txpwrcap_ctx_t *ctx;
} phy_type_txpwrcap_fns_t;

#if defined(WLC_TXPWRCAP)
/*
 * Register/unregister PHY type implementation to the TxPowerCtrl module.
 * It returns BCME_XXXX.
 */
int phy_txpwrcap_register_impl(phy_txpwrcap_info_t *mi, phy_type_txpwrcap_fns_t *fns);
void phy_txpwrcap_unregister_impl(phy_txpwrcap_info_t *mi);
#endif /* WLC_TXPWRCAP */

#endif /* _phy_type_txpwrcap_h_ */
