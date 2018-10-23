/*
 * TOF module internal interface (to PHY specific implementation).
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
 * $Id: phy_type_tof.h 688501 2017-03-06 20:58:21Z $
 */

#ifndef _phy_type_tof_h_
#define _phy_type_tof_h_

#include <phy_tof.h>
#include <phy_tof_api.h>
#include <bcm_math.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_tof_ctx_t;

typedef int (*phy_type_tof_init_fn_t)(phy_type_tof_ctx_t *ctx);
typedef int (*phy_type_tof_fn_t)(phy_type_tof_ctx_t *ctx, bool enter, bool tx, bool hw_adj,
	bool seq_en, int core, int emu_delay);
typedef void (*phy_type_tof_cmd_fn_t)(phy_type_tof_ctx_t *ctx, bool seq, int emu_delay);
typedef int (*phy_type_tof_info_fn_t)(phy_type_tof_ctx_t *ctx, wlc_phy_tof_info_t *tof_info,
	wlc_phy_tof_info_type_t tof_info_mask, uint8  core);
typedef int (*phy_type_tof_kvalue_fn_t)(phy_type_tof_ctx_t *ctx, chanspec_t chanspec,
	uint32 rspecidx, uint32 *kip, uint32 *ktp, uint8 flag);
typedef int (*phy_type_tof_seq_params_fn_t)(phy_type_tof_ctx_t *ctx, bool assign_buffer);
typedef void (*phy_type_tof_seq_upd_dly_fn_t)(phy_type_tof_ctx_t *ctx, bool tx, uint8 core,
	bool mac_suspend);
typedef int (*phy_type_chan_freq_response_fn_t)(phy_type_tof_ctx_t *ctx, int len, int nbits,
	bool swap_pn_half, uint32 offset, cint32* H, uint32* Hraw, uint8 core, uint32 sts_offset,
	const bool single_core);
typedef int (*phy_type_chan_mag_sqr_impulse_response_fn_t)(phy_type_tof_ctx_t *ctx,
	int frame_type, int len, int offset, int nbits, int32* h, int* pgd, uint32* hraw,
	uint16 tof_shm_ptr);
typedef int (*phy_type_seq_ts_fn_t)(phy_type_tof_ctx_t *ctx, int n, cint32* p_buffer, int tx,
	int cfo, int adj, void* pparams, int32* p_ts, int32* p_seq_len, uint32* p_raw,
	uint8* ri_rr, const uint8 smooth_win_en);
typedef int (*phy_type_tof_dbg_fn_t)(phy_type_tof_ctx_t *ctx, int arg);
typedef int (*phy_type_tof_set_ri_rr_fn_t)(phy_type_tof_ctx_t *ctx, const uint8 *ri_rr,
	const uint16 len, const uint8 core, const bool isInitiator, const bool isSecure,
	wlc_phy_tof_secure_2_0_t  tof_sec_params);
typedef int (*phy_type_tof_seq_params_get_set_acphy_fn_t)(phy_type_tof_ctx_t *ctx, uint8 *delays,
	bool set, bool tx, int size);
typedef void (*phy_type_tof_setup_ack_core_fn_t)(phy_type_tof_ctx_t *ctx, const int core);
typedef void (*phy_type_tof_core_select_fn_t)(phy_type_tof_ctx_t *ctx, const uint32 gdv_th,
		const int32 gdmm_th, const int8 rssi_th, const int8 delta_rssi_th,
		uint8* core, uint8 core_mask);
typedef void (*phy_type_tof_init_gdmm_th_fn_t)(phy_type_tof_ctx_t *ctx, int32 *gdmm_th);
typedef void (*phy_type_tof_init_gdv_th_fn_t)(phy_type_tof_ctx_t *ctx, uint32 *gdv_th);
typedef int (*phy_type_tof_calc_snr_bitflips_fn_t)(phy_type_tof_ctx_t *ctx, void *In,
	wl_proxd_bitflips_t *bit_flips, wl_proxd_snr_t *snr);

typedef struct {
	phy_type_tof_init_fn_t init_tof;
	phy_type_tof_fn_t tof;
	phy_type_tof_cmd_fn_t cmd;
	phy_type_tof_info_fn_t info;
	phy_type_tof_kvalue_fn_t kvalue;
	phy_type_tof_seq_params_fn_t seq_params;
	phy_type_tof_seq_upd_dly_fn_t seq_upd_dly;
	phy_type_chan_freq_response_fn_t chan_freq_response;
	phy_type_chan_mag_sqr_impulse_response_fn_t chan_mag_sqr_impulse_response;
	phy_type_seq_ts_fn_t seq_ts;
	phy_type_tof_dbg_fn_t dbg;
	phy_type_tof_set_ri_rr_fn_t set_ri_rr;
	phy_type_tof_seq_params_get_set_acphy_fn_t seq_params_get_set_acphy;
	phy_type_tof_setup_ack_core_fn_t setup_ack_core;
	phy_type_tof_core_select_fn_t core_select;
	phy_type_tof_init_gdmm_th_fn_t init_gdmm_th;
	phy_type_tof_init_gdv_th_fn_t init_gdv_th;
	phy_type_tof_ctx_t *ctx;
	phy_type_tof_calc_snr_bitflips_fn_t calc_snr_bitflips;
} phy_type_tof_fns_t;

/*
 * Register/unregister PHY type implementation to the common of the TOF module.
 * It returns BCME_XXXX.
 */
int phy_tof_register_impl(phy_tof_info_t *ti, phy_type_tof_fns_t *fns);
void phy_tof_unregister_impl(phy_tof_info_t *ti);

#endif /* _phy_type_tof_h_ */
