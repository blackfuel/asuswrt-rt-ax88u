/*
 * Noise module public interface (to MAC driver).
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
 * $Id: phy_noise_api.h 753957 2018-03-23 11:56:39Z $
 */

#ifndef _phy_noise_api_h_
#define _phy_noise_api_h_

#include <typedefs.h>
#include <phy_api.h>

/* Reasons for changing the PHY BG NOISE scheduling */
typedef enum  phy_bgnoise_schd_mode {
	PHY_LPAS_MODE = 0,
	PHY_CAL_MODE = 1,
	PHY_FORCEMEASURE_MODE = 2
} phy_bgnoise_schd_mode_t;

/* Fixed Noise for APHY/BPHY/GPHY */
#define PHY_NOISE_FIXED_VAL				(-101)	/* reported fixed noise */
#define PHY_NOISE_FIXED_VAL_NPHY		(-101)	/* reported fixed noise */
#define PHY_NOISE_INVALID				(0)

/* phy_mode bit defs for high level phy mode state information */
#define PHY_MODE_ACI		0x0001	/* set if phy is in ACI mitigation mode */
#define PHY_MODE_CAL		0x0002	/* set if phy is in Calibration mode */
#define PHY_MODE_NOISEM		0x0004	/* set if phy is in Noise Measurement mode */

#ifndef WLC_DISABLE_ACI
void wlc_phy_interf_rssi_update(wlc_phy_t *pi, chanspec_t chanNum, int8 leastRSSI);
#endif // endif
/* HWACI Interrupt Handler */
void wlc_phy_hwaci_mitigate_intr(wlc_phy_t *pih);

/* Interference and Mitigation (ACI, Noise) Module */
int phy_noise_interf_chan_stats_upd(wlc_phy_t *pi, chanspec_t chanspec, uint32 crsglitch,
	uint32 bphy_crsglitch, uint32 badplcp, uint32 bphy_badplcp, uint8 txop, uint32 mbsstime);
int phy_noise_sched_set(wlc_phy_t *pih, phy_bgnoise_schd_mode_t reason, bool upd);
bool phy_noise_sched_get(wlc_phy_t *pih);
int phy_noise_pmstate_set(wlc_phy_t *pih, bool pmstate);
int8 phy_noise_avg_per_antenna(wlc_phy_t *pih, int coreidx);
int8 phy_noise_avg(wlc_phy_t *wpi);
int8 phy_noise_lte_avg(wlc_phy_t *wpi);
int phy_noise_sample_request_external(wlc_phy_t *ppi);
int phy_noise_sample_request_crsmincal(wlc_phy_t *ppi);
int phy_noise_sample_intr(wlc_phy_t *ppi);
#endif /* _phy_noise_api_h_ */
