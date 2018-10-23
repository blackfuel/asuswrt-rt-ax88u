/*
 * TOF module implementation
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
 * $Id: phy_tof.c 688501 2017-03-06 20:58:21Z $
 */

#ifdef WL_PROXDETECT
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_tof_api.h>
#include "phy_type_tof.h"
#include <phy_tof.h>
#include <phy_misc_api.h>

/* module private states */
struct phy_tof_info {
	phy_info_t *pi;
	phy_type_tof_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_tof_info_t info;
	phy_type_tof_fns_t fns;
} phy_tof_mem_t;

/* local function declaration */
static int phy_tof_init(phy_init_ctx_t *ctx);

/* attach/detach */
phy_tof_info_t *
BCMATTACHFN(phy_tof_attach)(phy_info_t *pi)
{
	phy_tof_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_tof_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_tof_mem_t *)info)->fns;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_tof_init, info, PHY_INIT_TOF) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error */
fail:
	phy_tof_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_tof_detach)(phy_tof_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null tof module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;
	phy_mfree(pi, info, sizeof(phy_tof_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_tof_register_impl)(phy_tof_info_t *ti, phy_type_tof_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ti->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_tof_unregister_impl)(phy_tof_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* TOF init */
static int
WLBANDINITFN(phy_tof_init)(phy_init_ctx_t *ctx)
{
	phy_tof_info_t *tofi = (phy_tof_info_t *)ctx;
	phy_type_tof_fns_t *fns = tofi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->init_tof != NULL) {
		return (fns->init_tof)(fns->ctx);
	}
	return BCME_OK;
}

#ifdef WL_PROXD_SEQ
/* Configure phy appropiately for RTT measurements */
int
wlc_phy_tof(wlc_phy_t *ppi, bool enter, bool tx, bool hw_adj, bool seq_en, int core, int emu_delay)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	int err;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->tof != NULL) {
		if (enter) {
			wlc_phy_hold_upd(ppi, PHY_HOLD_FOR_TOF, enter);
		}
		err = (fns->tof)(fns->ctx,  enter, tx, hw_adj, seq_en, core, emu_delay);
		if (!enter) {
			wlc_phy_hold_upd(ppi, PHY_HOLD_FOR_TOF, enter);
		}
		return err;
	} else {
		return BCME_UNSUPPORTED;
	}
}

/* Setup delays for trigger in RTT Sequence measurements */
int
phy_tof_seq_params(wlc_phy_t *ppi, bool assign_buffer)
{
	/*
	In case we are running secure ranging (ranging sequence is genrated out of ri_rr every
	session), this function should be called with assign_buffer = TRUE only before FTM1.
	After FTM1 when we have finished set_ri_rr function call, this should never be called
	with assign_buffer = TRUE otherwise ri_rr will never go in effect.
	*/

	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->seq_params != NULL) {
	  return (fns->seq_params)(fns->ctx, assign_buffer);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_tof_set_ri_rr(wlc_phy_t *ppi, const uint8* ri_rr, const uint16 len,
	const uint8 core, const bool isInitiator, const bool isSecure,
	wlc_phy_tof_secure_2_0_t secure_params)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->set_ri_rr != NULL) {
		return (fns->set_ri_rr)(fns->ctx, ri_rr, len, core, isInitiator,
			isSecure, secure_params);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_tof_seq_upd_dly(wlc_phy_t *ppi, bool tx, uint8 core, bool mac_suspend)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->seq_upd_dly != NULL) {
		(fns->seq_upd_dly)(fns->ctx, tx, core, mac_suspend);
		return BCME_OK;
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_tof_seq_params_get_set(wlc_phy_t *ppi, uint8 *delays, bool set, bool tx, int size)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->seq_params_get_set_acphy != NULL) {
	  return (fns->seq_params_get_set_acphy)(fns->ctx, delays, set, tx, size);
	} else {
		return BCME_UNSUPPORTED;
	}
}

#ifdef TOF_DBG_SEQ
int
phy_tof_dbg(wlc_phy_t *ppi, int arg)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->dbg != NULL) {
		return (fns->dbg)(fns->ctx, arg);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* TOF_DBG_SEQ */

#else
/* Configure phy appropiately for RTT measurements */
int
wlc_phy_tof(wlc_phy_t *ppi, bool enter, bool hw_adj)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	int err;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->tof != NULL) {
		if (enter) {
			wlc_phy_hold_upd(ppi, PHY_HOLD_FOR_TOF, enter);
		}
		err = (fns->tof)(fns->ctx,  enter, FALSE, hw_adj, FALSE, 0);
		if (!enter) {
			wlc_phy_hold_upd(ppi, PHY_HOLD_FOR_TOF, enter);
		}
		return err;
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* WL_PROXD_SEQ */

int
wlc_phy_tof_calc_snr_bitflips(wlc_phy_t *ppi, void *In,
	wl_proxd_bitflips_t *bit_flips, wl_proxd_snr_t *snr)
{
	int err = BCME_OK;
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->calc_snr_bitflips != NULL) {
		err = (fns->calc_snr_bitflips)(fns->ctx, In, bit_flips, snr);
	} else {
		return BCME_UNSUPPORTED;
	}
	return err;
}

void phy_tof_setup_ack_core(wlc_phy_t *ppi, int core)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ASSERT(pi != NULL);
	phy_tof_info_t *tofi = pi->tofi;
	phy_type_tof_fns_t *fns = tofi->fns;

	if (fns->setup_ack_core != NULL) {
		(fns->setup_ack_core)(pi, core);
	}

}

uint8 phy_tof_num_cores(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return (uint8)pi->pubpi->phy_corenum;
}

void phy_tof_core_select(wlc_phy_t *ppi, const uint32 gdv_th, const int32 gdmm_th,
		const int8 rssi_th, const int8 delta_rssi_th, uint8* core, uint8 core_mask)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ASSERT(pi != NULL);
	phy_tof_info_t *tofi = pi->tofi;
	phy_type_tof_fns_t *fns = tofi->fns;

	if (fns->core_select != NULL) {
		(fns->core_select)(pi, gdv_th, gdmm_th, rssi_th, delta_rssi_th, core, core_mask);
	}
}

/* Get channel frequency response for deriving 11v rx timestamp */
int
phy_tof_chan_freq_response(wlc_phy_t *ppi, int len, int nbits, int32* Hr, int32* Hi, uint32* Hraw,
		const bool single_core, uint8 num_sts, bool collect_offset)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	bool swap_pn_half = (Hr != NULL) ? TRUE : FALSE;
	int ret_val = 0;
	uint8 core, sts;
	uint32 ch_table_sts_offset, offset;
	uint32* Hraw_tmp;

	const uint8 num_cores = PHYCORENUM((pi)->pubpi->phy_corenum);

	uint8 core_max = (single_core) ? 1 : num_cores;
	uint8 sts_max = (single_core) ? 1 : num_sts;

	ASSERT(pi != NULL);
	phy_tof_info_t *tofi = pi->tofi;
	phy_type_tof_fns_t *fns = tofi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->chan_freq_response != NULL) {
		ch_table_sts_offset = 256;
		for (core = 0; core < core_max; core++) {
			for (sts = 0; ((sts < sts_max) && (ret_val == 0)); sts++) {
				offset = len*(single_core ? (collect_offset ? num_cores : 0)
					       : (sts + core*num_sts));
				Hraw_tmp = Hraw ? (Hraw + offset) : Hraw;
				ret_val += (fns->chan_freq_response)(fns->ctx, len, nbits,
						swap_pn_half, offset, ((cint32 *)Hr),
						Hraw_tmp, core, ch_table_sts_offset*sts,
						single_core);
			}
		}
	} else {
		ret_val = BCME_UNSUPPORTED;
	}
	return ret_val;
}

/* Get mag sqrd channel impulse response(from channel smoothing hw) to derive 11v rx timestamp */
int
wlc_phy_chan_mag_sqr_impulse_response(wlc_phy_t *ppi, int frame_type,
	int len, int offset, int nbits, int32* h, int* pgd, uint32* hraw, uint16 tof_shm_ptr)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->chan_mag_sqr_impulse_response != NULL)
		return (fns->chan_mag_sqr_impulse_response)(fns->ctx,
			frame_type, len, offset, nbits, h, pgd, hraw, tof_shm_ptr);

	return BCME_UNSUPPORTED;
}

#ifdef WL_PROXD_SEQ
/* Extract information from status bytes of last rxd frame */
int wlc_phy_tof_info(wlc_phy_t *ppi, wlc_phy_tof_info_t *tof_info,
	wlc_phy_tof_info_type_t tof_info_mask, uint8 core)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->info != NULL) {
		return (fns->info)(fns->ctx, tof_info, tof_info_mask, core);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#else
/* Extract information from status bytes of last rxd frame */
int wlc_phy_tof_info(wlc_phy_t *ppi, int* p_frame_type, int* p_frame_bw, int8* p_rssi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	int cfo;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->info != NULL) {
		return (fns->info)(fns->ctx,  p_frame_type, p_frame_bw, &cfo, p_rssi);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* WL_PROXD_SEQ */

/* Get timestamps from ranging sequence */
int
wlc_phy_seq_ts(wlc_phy_t *ppi, int n, void* p_buffer, int tx, int cfo, int adj, void* pparams,
	int32* p_ts, int32* p_seq_len, uint32* p_raw, uint8* ri_rr, const uint8 smooth_win_en)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->seq_ts != NULL) {
		return (fns->seq_ts)(fns->ctx, n, (cint32*)p_buffer, tx, cfo, adj, pparams, p_ts,
			p_seq_len, p_raw, ri_rr, smooth_win_en);
	}
	return BCME_UNSUPPORTED;
}

/* Do any phy specific setup needed for each command */
void phy_tof_cmd(wlc_phy_t *ppi, bool seq, int emu_delay)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->cmd != NULL) {
	  return (fns->cmd)(fns->ctx, seq, emu_delay);
	}
}

#ifdef WL_PROXD_SEQ
/* Get TOF K value for initiator and target */
int
wlc_phy_tof_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 *kip, uint32 *ktp, uint8 seq_en)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->kvalue != NULL) {
		return (fns->kvalue)(fns->ctx,  chanspec, 0, kip, ktp, seq_en);
	}

	return -1;
}

/* Get TOF K value for initiator and target */
int
wlc_phy_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 rspecidx, uint32 *kip,
	uint32 *ktp, uint8 seq_en)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->kvalue != NULL) {
		return (fns->kvalue)(fns->ctx,  chanspec, rspecidx, kip, ktp, seq_en);
	}

	return -1;
}

#else
/* Get TOF K value for initiator and target */
int
wlc_phy_tof_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 *kip, uint32 *ktp)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->kvalue != NULL) {
		return (fns->kvalue)(fns->ctx,  chanspec, kip, ktp, FALSE);
	}

	return -1;
}
#endif /* WL_PROXD_SEQ */

void
phy_tof_init_gdmm_th(wlc_phy_t *ppi, int32 *gdmm_th)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->init_gdmm_th != NULL) {
		(fns->init_gdmm_th)(fns->ctx, gdmm_th);
	} else {
		*gdmm_th = 0;
	}
}

void
phy_tof_init_gdv_th(wlc_phy_t *ppi, uint32 *gdv_th)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->init_gdv_th != NULL) {
		(fns->init_gdv_th)(fns->ctx, gdv_th);
	} else {
		*gdv_th = 0;
	}
}
/* DEBUG */
#ifdef TOF_DBG
int phy_tof_dbg(wlc_phy_t *ppi, int arg)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(pi != NULL);
	phy_tof_info_t *info = pi->tofi;
	phy_type_tof_fns_t *fns = info->fns;

	if (fns->dbg != NULL) {
		return (fns->dbg)(fns->ctx,  arg);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* TOF_DBG */

#endif /* WL_PROXDETECT */
