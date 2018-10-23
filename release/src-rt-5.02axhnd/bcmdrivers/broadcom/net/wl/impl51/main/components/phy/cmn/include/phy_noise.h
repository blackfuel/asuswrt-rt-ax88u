/*
 * NOISEmeasure module internal interface (to other PHY modules).
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
 * $Id: phy_noise.h 685853 2017-02-19 06:48:41Z $
 */

#ifndef _phy_noise_h_
#define _phy_noise_h_

#include <typedefs.h>
#include <phy_api.h>
#include <phy_dbg_api.h>

/* forward declaration */
typedef struct phy_noise_info phy_noise_info_t;

/* attach/detach */
phy_noise_info_t *phy_noise_attach(phy_info_t *pi, int bandtype);
void phy_noise_detach(phy_noise_info_t *nxi);

int phy_noise_bss_init(phy_noise_info_t *noisei, int noise);

/* noise calculation */
void wlc_phy_noise_calc(phy_info_t *pi, uint32 *cmplx_pwr, int8 *pwr_ant,
                               uint8 extra_gain_1dB);
void wlc_phy_noise_calc_fine_resln(phy_info_t *pi, uint32 *cmplx_pwr, uint16 *crsmin_pwr,
		int16 *pwr_ant, uint8 extra_gain_1dB, int16 *tot_gain);

/* set mode */
int phy_noise_set_mode(phy_noise_info_t *ii, int mode, bool init);

/* set interference over-ride mode */
int wlc_phy_set_interference_override_mode(phy_info_t *pi, int val);

/* common dump functions for non-ac phy */
int phy_noise_dump_common(phy_info_t *pi, struct bcmstrbuf *b);

#ifdef RADIO_HEALTH_CHECK
phy_crash_reason_t
phy_noise_healthcheck_desense(phy_noise_info_t *noisei);
#endif /* RADIO_HEALTH_CHECK */

void wlc_phy_noise_save(phy_info_t *pi, int8 *noise_dbm_ant, int8 *max_noise_dbm);

void wlc_phy_aci_upd(phy_noise_info_t *ii);

void phy_noise_invoke_callbacks(phy_noise_info_t *noisei, uint8 channel, int8 noise_dbm);

/* Returns noise level (read from srom) for current channel */
int phy_noise_get_srom_level(phy_info_t *pi, int32 *ret_int_ptr);

int8 phy_noise_read_shmem(phy_noise_info_t *noisei, uint8 *lte_on, uint8 *crs_high);

int8 phy_noise_abort_shmem_read(phy_noise_info_t *noisei);
#ifndef WLC_DISABLE_ACI
#if defined(WLTEST)
int  phy_noise_aci_args(phy_info_t *pi, wl_aci_args_t *params, bool get, int len);
#endif // endif
#endif /* Compiling out ACI code */
bool phy_noise_pmstate_get(phy_info_t *pi);
#endif /* _phy_noise_h_ */
