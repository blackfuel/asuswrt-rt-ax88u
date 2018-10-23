/*
 * RadarDetect module internal interface (functions sharde by PHY type specific implementations).
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
 * $Id: phy_radar_shared.h 752845 2018-03-19 07:24:28Z $
 */

#ifndef _phy_radar_shared_h_
#define _phy_radar_shared_h_

#include <typedefs.h>
#include <phy_api.h>
#include <phy_radar_api.h>

/* radar_args.feature_mask bitfields */
#define RADAR_FEATURE_DEBUG_SHORT_PULSE		(1 << 0) /* when bit-3 is on,
						          * 0=>bin5 data, 1=>short pulse data
						          */
#define RADAR_FEATURE_DEBUG_INPUT_PULSE_DATA	(1 << 1) /* output before-fitlered pulse data */
#define RADAR_FEATURE_DEBUG_PULSES_PER_ANT	(1 << 2) /* output # pulses at each antenna
						          * (if # pulse > 5)
						          */
#define RADAR_FEATURE_DEBUG_PULSE_DATA		(1 << 3) /* output radar pulses data */
#define RADAR_FEATURE_DEBUG_PW_CHECK_INFO	(1 << 4) /* output PW checking debug messages */
#define RADAR_FEATURE_DEBUG_STAGGERED_RESET	(1 << 5) /* output staggered reset */
#define RADAR_FEATURE_DEBUG_FIFO_OUTPUT		(1 << 6) /* output fifo output */
#define RADAR_FEATURE_DEBUG_INTV_PW		(1 << 7) /* output intervals and pruned pw */
#define RADAR_FEATURE_UK_DETECT			(1 << 8) /* enable UK radar detection */
#define RADAR_FEATURE_DEBUG_EU_TYPE		(1 << 9) /* output EU type debug messages */
#define RADAR_FEATURE_FCC_DETECT		(1 << 11) /* enable FCC radar detection */
#define RADAR_FEATURE_ETSI_DETECT		(1 << 12) /* enable ETSI radar detection */
#define RADAR_FEATURE_USE_MAX_PW		(1 << 13) /* for combining pulse use max of pw(i)
						           * and pw(i-1) inlieu of pw(i-1) + pw(i)
						           */
#define RADAR_FEATURE_DEBUG_REJECTED_RADAR	(1 << 14) /* output the skipped large intervals */

void phy_radar_shared_attach(phy_info_t *pi);

/*
 * Run the radar detect algorithm.
 */
uint8 phy_radar_run_nphy(phy_info_t *pi, radar_detected_info_t *radar_detected,
bool sec_pll, bool bw80_80_mode);

#endif /* _phy_radar_shared_h_ */
