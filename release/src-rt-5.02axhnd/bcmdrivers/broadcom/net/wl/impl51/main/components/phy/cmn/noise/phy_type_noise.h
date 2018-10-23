/*
 * NOISEmeasure module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_noise.h 721745 2017-09-15 18:03:27Z $
 */

#ifndef _phy_type_noise_h_
#define _phy_type_noise_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_noise.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_noise_ctx_t;

typedef void (*phy_type_noise_mode_fn_t)(phy_type_noise_ctx_t *ctx, int mode, bool init);
typedef int (*phy_type_noise_reset_fn_t)(phy_type_noise_ctx_t *ctx);
typedef void (*phy_type_calc_fn_t)(phy_type_noise_ctx_t *ctx, int8 cmplx_pwr_dbm[],
		uint8 extra_gain_1dB);
typedef void (*phy_type_calc_fine_resln_fn_t)(phy_type_noise_ctx_t *ctx, int16 cmplx_pwr_dbm[],
		uint8 extra_gain_1dB, int16 *tot_gain);
typedef int (*phy_type_noise_get_fixed_fn_t)(phy_type_noise_ctx_t *ctx);
typedef bool (*phy_type_noise_start_fn_t)(phy_type_noise_ctx_t *ctx, uint8 reason);
typedef void (*phy_type_noise_stop_fn_t)(phy_type_noise_ctx_t *ctx);
typedef void (*phy_type_noise_interference_rssi_update_fn_t)(phy_type_noise_ctx_t *ctx,
		chanspec_t chanspec, int8 leastRSSI);
typedef void (*phy_type_noise_interf_mode_set_fn_t)(phy_type_noise_ctx_t *ctx, int val);
typedef void (*phy_type_noise_aci_mitigate_fn_t)(phy_type_noise_ctx_t *ctx);
typedef int (*phy_type_noise_dump_fn_t)(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b);
typedef void (*phy_type_noise_upd_fn_t)(phy_type_noise_ctx_t *ctx);
typedef phy_crash_reason_t (*phy_type_noise_health_check_desense_fn_t)(phy_type_noise_ctx_t *ctx);
typedef void (*phy_type_noise_request_sample_fn_t)(phy_type_noise_ctx_t *ctx,
	uint8 reason, uint8 ch);
typedef int8 (*phy_type_noise_avg_fn_t)(phy_type_noise_ctx_t *ctx);
typedef void (*phy_type_noise_sample_intr_fns_t)(phy_type_noise_ctx_t *ctx);
typedef void (*phy_type_noise_cb_fns_t)(phy_type_noise_ctx_t *ctx);
typedef int (*phy_type_noise_get_fn_t)(phy_type_noise_ctx_t *ctx, int32 *ret_int_ptr);
typedef int8 (*phy_type_noise_read_shmem_fns_t)(phy_type_noise_ctx_t *ctx,
	uint8 *lte_on, uint8 *crs_high);

typedef struct {
	phy_type_noise_mode_fn_t mode;	/* From Interference */
	phy_type_noise_reset_fn_t reset; /* From Interference */
	/* calculate noise */
	phy_type_calc_fn_t calc;
	/* calculate noise fine resolution */
	phy_type_calc_fine_resln_fn_t calc_fine;
	/* get fixed noise */
	phy_type_noise_get_fixed_fn_t get_fixed;
	/* start measure request */
	phy_type_noise_start_fn_t start;
	/* stop measure */
	phy_type_noise_stop_fn_t stop;
	/* interference rssi update */
	phy_type_noise_interference_rssi_update_fn_t interf_rssi_update;
	/* interference rssi update */
	phy_type_noise_interf_mode_set_fn_t interf_mode_set;
	/* hw aci mitigate */
	phy_type_noise_aci_mitigate_fn_t aci_mitigate;
	/* radio health check desense */
	phy_type_noise_health_check_desense_fn_t health_check_desense;
	/* context */
	phy_type_noise_ctx_t *ctx;
	/* dump */
	phy_type_noise_dump_fn_t dump;
	/* aci_upd */
	phy_type_noise_upd_fn_t aci_upd;
	/* aci_ma_upd */
	phy_type_noise_upd_fn_t ma_upd;
	/* request noise sample */
	phy_type_noise_request_sample_fn_t request_noise_sample;
	/* avg */
	phy_type_noise_avg_fn_t avg;
	/* sample_intr */
	phy_type_noise_sample_intr_fns_t sample_intr;
	/* cb */
	phy_type_noise_cb_fns_t cb;
	/* lte_avg */
	phy_type_noise_avg_fn_t lte_avg;
	/* get srom noise level */
	phy_type_noise_get_fn_t get_srom_level;
	/* read shm */
	phy_type_noise_read_shmem_fns_t read_shm;
} phy_type_noise_fns_t;

/*
 * Register/unregister PHY type implementation to the NOISEmeasure module.
 * It returns BCME_XXXX.
 */
int phy_noise_register_impl(phy_noise_info_t *mi, phy_type_noise_fns_t *fns);
void phy_noise_unregister_impl(phy_noise_info_t *mi);

#endif /* _phy_type_noise_h_ */
