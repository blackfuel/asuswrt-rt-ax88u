/*
 * Miscellaneous modules interface (to other PHY modules).
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
 * $Id: phy_ac_misc.h 735703 2017-12-12 05:40:37Z $
 */

#ifndef _phy_ac_misc_h_
#define _phy_ac_misc_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_misc.h>
#include <bcm_math.h>

/* forward declaration */
typedef struct phy_ac_misc_info phy_ac_misc_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_misc_info_t *phy_ac_misc_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_misc_info_t *cmn_info);
void phy_ac_misc_unregister_impl(phy_ac_misc_info_t *ac_info);

extern void wlc_phy_force_rfseq_acphy(phy_info_t *pi, uint8 cmd);
extern void wlc_phy_deaf_acphy(phy_info_t *pi, bool mode);
extern bool wlc_phy_get_deaf_acphy(phy_info_t *pi);
extern void wlc_phy_gpiosel_acphy(phy_info_t *pi, uint16 sel, uint8 word_swap);
#if defined(WLTEST)
extern void wlc_phy_test_scraminit_acphy(phy_info_t *pi, int8 init);
#endif // endif
#ifdef ATE_BUILD
extern void wlc_phy_gpaio_acphy(phy_info_t *pi, wl_gpaio_option_t option, int core);
#endif /* ATE_BUILD */
void wlc_phy_cts2self(phy_info_t *pi, uint16 duration);
void wlc_phy_cals_mac_susp_en_other_cr(phy_info_t *pi, bool suspend);

/* !!! This has been redeclared in wlc_phy_hal.h. Shoul dbe removed from there. !!! */
extern void wlc_acphy_set_scramb_dyn_bw_en(wlc_phy_t *pi, bool enable);
extern void wlc_phy_force_rfseq_noLoleakage_acphy(phy_info_t *pi);
extern void wlc_phy_force_femreset_acphy(phy_info_t *pi, bool ovr);

void wlc_phy_loadsampletable_acphy(phy_info_t *pi, math_cint32 *tone_buf, const uint16 num_samps,
        bool conj);

#define TX_TONE_IQCAL_MODE_OFF	0
#define TX_TONE_IQCAL_MODE_ON	1
#define STOPPLAYBACK_W_CCA_RESET 0
#define STOPPLAYBACK_WO_CCA_RESET 1
int wlc_phy_tx_tone_acphy(phy_info_t *pi, int32 f_kHz, uint16 max_val, uint8 mode,
	bool modify_bbmult);
int wlc_phy_tx_tone_acphy_mac_based(phy_info_t *pi, int32 f_kHz, uint16 max_val, uint8 mode,
	bool modify_bbmult);

extern void wlc_phy_stopplayback_acphy(phy_info_t *pi, bool no_reset);

int phy_ac_misc_set_rud_agc_enable(phy_ac_misc_info_t *misci, int32 int_val);
int phy_ac_misc_get_rud_agc_enable(phy_ac_misc_info_t *misci, int32 *ret_int_ptr);
#endif /* _phy_ac_misc_h_ */
