/*
 * RadarDetect module public interface (to MAC driver).
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
 * $Id: phy_radar_api.h 752845 2018-03-19 07:24:28Z $
 */

#ifndef _phy_radar_api_h_
#define _phy_radar_api_h_

#include <typedefs.h>
#include <phy_api.h>

/* Note: uncomment the commented-out Radar Types once the above are removed */
#define RADAR_TYPE_NONE		0   /* Radar type None */
#define RADAR_TYPE_ETSI_0	1   /* ETSI 0: PW 1us, PRI 1429us */
#define RADAR_TYPE_ETSI_1	2   /* ETSI 1: PW 0.5-5us, PRI 1000-5000us */
#define RADAR_TYPE_ETSI_2	3   /* ETSI 2: PW 0.5-15us, PRI 625-5000us */
#define RADAR_TYPE_ETSI_3	4   /* ETSI 3: PW 0.5-15us, PRI 250-435us */
#define RADAR_TYPE_ETSI_4	5   /* ETSI 4: PW 20-30us, PRI 250-500us */
#define RADAR_TYPE_ETSI_5_STG2	6   /* staggered-2 ETSI 5: PW 0.5-2us, PRI 2500-3333us */
#define RADAR_TYPE_ETSI_5_STG3	7   /* staggered-3 ETSI 5: PW 0.5-2us, PRI 2500-3333us */
#define RADAR_TYPE_ETSI_6_STG2	8   /* staggered-2 ETSI 6: PW 0.5-2us, PRI 833-2500us */
#define RADAR_TYPE_ETSI_6_STG3	9   /* staggered-3 ETSI 6: PW 0.5-2us, PRI 833-2500us */
#define RADAR_TYPE_ETSI_STG2	RADAR_TYPE_ETSI_5_STG2 /* staggered-2: PW 0.5-2us, PRI 833-3333us */
#define RADAR_TYPE_ETSI_STG3	RADAR_TYPE_ETSI_5_STG3 /* staggered-3: PW 0.5-2us, PRI 833-3333us */
#define RADAR_TYPE_FCC_0	10  /* FCC 0: PW 1us, PRI 1428us */
#define RADAR_TYPE_FCC_1	11  /* FCC 1: PW 1us, PRI 518-3066us */
#define RADAR_TYPE_FCC_2	12  /* FCC 2: PW 1-5us, PRI 150-230us */
#define RADAR_TYPE_FCC_3	13  /* FCC 3: PW 6-10us, PRI 200-500us */
#define RADAR_TYPE_FCC_4	14  /* FCC 4: PW 11-20us, PRI 200-500us */
#define RADAR_TYPE_FCC_5	15  /* FCC 5: PW 50-100us, PRI 1000-2000us */
#define RADAR_TYPE_FCC_6	16  /* FCC 6: PW 1us, PRI 333us */
#define RADAR_TYPE_JP1_1	17  /* Japan 1.1: same as FCC 0 */
#define RADAR_TYPE_JP1_2	18  /* Japan 1.2: PW 2.5us, PRI 3846us */
#define RADAR_TYPE_JP2_1_1	19  /* Japan 2.1.1: PW 0.5us, PRI 1389us */
#define RADAR_TYPE_JP2_1_2	20  /* Japan 2.1.2: PW 2us, PRI 4000us */
#define RADAR_TYPE_JP2_2_1	21  /* Japan 2.2.1: same as FCC 0 */
#define RADAR_TYPE_JP2_2_2	22  /* Japan 2.2.2: same as FCC 2 */
#define RADAR_TYPE_JP2_2_3	23  /* Japan 2.2.3: PW 6-10us, PRI 250-500us (similar to FCC 3) */
#define RADAR_TYPE_JP2_2_4	24  /* Japan 2.2.4: PW 11-20us, PRI 250-500us (similar to FCC 4) */
#define RADAR_TYPE_JP3		25  /* Japan 3: same as FCC 5 */
#define RADAR_TYPE_JP4		26  /* Japan 4: same as FCC 6 */
#define RADAR_TYPE_KN1		27  /* Korean 1: same as FCC 0 */
#define RADAR_TYPE_KN2		28  /* Korean 2: PW 1us, PRI 556us */
#define RADAR_TYPE_KN3		29  /* Korean 3: PW 2us, PRI 3030us */
#define RADAR_TYPE_KN4		30  /* Korean 4: PW 1us, PRI 333us (similar to JP 4) */
#define RADAR_TYPE_UK1		31  /* UK 1: PW 1us, PRI 333us (similar to JP 4) with hopping */
#define RADAR_TYPE_UK2		32  /* UK 2: PW 20us, PRI 222us with chirp & hopping */
#define RADAR_TYPE_UNCLASSIFIED 255 /* Unclassified Radar type */

typedef struct {
	uint8 radar_type;	/* one of RADAR_TYPE_XXX */
	uint16 min_pw;		/* minimum pulse-width (usec * 20) */
	uint16 max_pw;		/* maximum pulse-width (usec * 20) */
	uint16 min_pri;		/* minimum pulse repetition interval (usec) */
	uint16 max_pri;		/* maximum pulse repetition interval (usec) */
	uint16 subband;		/* subband/frequency */
} radar_detected_info_t;

void phy_radar_detect_enable(phy_info_t *pi, bool on);

uint8 phy_radar_detect(phy_info_t *pi, radar_detected_info_t *radar_detected,
	bool sec_pll, bool bw80_80_mode);

typedef enum  phy_radar_detect_mode {
	RADAR_DETECT_MODE_FCC,
	RADAR_DETECT_MODE_EU,
	RADAR_DETECT_MODE_UK,
	RADAR_DETECT_MODE_MAX
} phy_radar_detect_mode_t;

void phy_radar_detect_mode_set(phy_info_t *pi, phy_radar_detect_mode_t mode);

#endif /* _phy_radar_api_h_ */
