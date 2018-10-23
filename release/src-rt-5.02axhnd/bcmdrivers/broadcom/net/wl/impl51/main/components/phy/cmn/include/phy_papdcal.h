/*
 * PAPD CAL module interface (to other PHY modules).
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
 * $Id: phy_papdcal.h 657373 2016-09-01 01:08:38Z $
 */

#ifndef _phy_papdcal_h_
#define _phy_papdcal_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_papdcal_info phy_papdcal_info_t;

/* attach/detach */
phy_papdcal_info_t *phy_papdcal_attach(phy_info_t *pi);
void phy_papdcal_detach(phy_papdcal_info_t *cmn_info);

bool phy_papdcal_epacal(phy_papdcal_info_t *papdcali);

/* Changed from IS_WFD_PHY_LL_ENABLE */
bool phy_papdcal_is_wfd_phy_ll_enable(phy_papdcal_info_t *papdcali);

/* EPAPD */
bool phy_papdcal_epapd(phy_papdcal_info_t *papdcali);

#ifndef WLC_DISABLE_PAPD_SUPPORT
#define PHY_PAPDEN(pi)	(PHY_IPA(pi) || phy_papdcal_epapd(pi->papdcali))
#else
#define PHY_PAPDEN(pi)	0
#endif // endif

#if defined(WFD_PHY_LL)
int phy_papdcal_set_wfd_ll_enable(phy_papdcal_info_t *papdcali, uint8 int_val);
int phy_papdcal_get_wfd_ll_enable(phy_papdcal_info_t *papdcali, int32 *ret_int_ptr);
#endif /* WFD_PHY_LL */
#if defined(WLTEST) || defined(BCMDBG)
void phy_papdcal_epa_dpd_set(phy_info_t *pi, uint8 enab_epa_dpd, bool in_2g_band);
#endif /* defined(WLTEST) || defined(BCMDBG) */
#if defined(WLTEST)
int phy_papdcal_get_lut_idx0(phy_info_t *pi, int32* idx);
int phy_papdcal_get_lut_idx1(phy_info_t *pi, int32* idx);
int phy_papdcal_set_idx(phy_info_t *pi, int8 idx);
#endif // endif
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
#ifndef ATE_BUILD
int phy_papdcal_set_skip(phy_info_t *pi, uint8 skip);
int phy_papdcal_get_skip(phy_info_t *pi, int32* skip);
#endif /* !ATE_BUILD */
#endif // endif
int phy_papdcal_decode_epsilon(uint32 epsilon, int32 *eps_real, int32 *eps_imag);
#endif /* _phy_papdcal_h_ */
