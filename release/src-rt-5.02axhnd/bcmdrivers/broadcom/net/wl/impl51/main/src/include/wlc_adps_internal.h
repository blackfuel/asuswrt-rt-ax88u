/*
 * ADaptive Power Save Functions
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
 * $Id$
 */

#ifndef _WLC_ADPS_INTERNAL_H_
#define _WLC_ADPS_INTERNAL_H_

#include <wlioctl.h>

#define ADPS_MIN_STEP_NUM 2
#define ADPS_MAX_STEP_NUM 10

#define WL_ADPS_IOV_PARAMS	0x8001

typedef struct wl_adps_step {
	uint8 pm2_sleep_ret_time;		/* PM2 sleep return time */
	uint8 padding[3];
	uint16 pkt_cnt_threshold[ADPS_NUM_DIR];	/* Tx/Rx packet counter threshold value */
} wl_adps_step_t;

typedef struct wl_adps_step_params_v1 {
	uint16 version;
	uint16 length;
	uint8 band;		/* band - 2G or 5G */
	uint8 mode;		/* operation mode, default = 0 (ADPS disable) */
	uint8 pspoll_prd;	/* PS-Poll interval for ADPS Step 0 (PM1 + Periodic PS-Poll) */
	uint8 num_step;		/* number of PM2 steps */
	wl_adps_step_t step[1];	/* PM2 step parameters for each step */
} wl_adps_step_params_v1_t;

typedef struct wl_adps_step_params_v2 {
	uint16 version;
	uint16 length;
	uint16 fixed_length;	/* length of total fixed fields */
	uint8 band;		/* band - 2G or 5G */
	uint8 mode;		/* operation mode, default = 0 (ADPS disable) */
	uint8 pspoll_prd;	/* PS-Poll interval for ADPS Step 0 (PM1 + Periodic PS-Poll) */
	uint8 num_step;		/* number of PM2 steps */
	uint8 padding[2];
	wl_adps_step_t step[1];	/* PM2 step parameters for each step */
} wl_adps_step_params_v2_t;

#define	WL_ADPS_IOV_DUMP_CMD_SUMMARY		0x00
#define	WL_ADPS_IOV_DUMP_CMD_GENERAL		0x01
#define	WL_ADPS_IOV_DUMP_CMD_STAT		0x02
#define	WL_ADPS_IOV_DUMP_CMD_STEP_WEIGHT	0x04
#define	WL_ADPS_IOV_DUMP_CMD_HISTOGRAM		0x08
#define	WL_ADPS_IOV_DUMP_CMD_LAST		0x0f

#define WL_ADPS_IOV_DUMP_CMD_BIT_MASK		0x01

typedef struct wl_adps_dump_general_v1 {
	uint16 version;
	uint16 length;
	uint8 mode;		/* operation mode, default = 0 (ADPS disable) */
	uint8 flags;		/* restrict flags */
	uint8 current_step;	/* current step */
	uint8 padding;
} wl_adps_dump_general_v1_t;

typedef struct wl_adps_dump_stat_v1 {
	uint16 version;
	uint16 length;
	uint16 fixed_length;		/* length of total fixed fields */
	uint8 mode;			/* operation mode, default = 0 (ADPS disable) */
	uint8 num_step;			/* number of step */
	adps_stat_elem_t elem[1];	/* statistics (max = 10) */
} wl_adps_dump_stat_v1_t;

typedef struct wl_adps_dump_step_weight_v1 {
	uint16 version;
	uint16 length;
	uint16 fixed_length;		/* length of total fixed fields */
	uint8 mode;			/* operation mode, default = 0 (ADPS disable) */
	uint8 num_step;			/* number of step */
	int8 weight[ADPS_NUM_DIR][1];	/* weigh to detect traffic continuity */
} wl_adps_dump_step_weight_v1_t;

#define ADPS_HISTOGRAM_SIZE		10
typedef struct wl_adps_pkt_histogram {
	uint32 current[ADPS_NUM_DIR];		/* 0: Rx packet 1: Tx packet */
	uint32 accumulated[ADPS_NUM_DIR];
	uint16 step[ADPS_NUM_DIR];
} wl_adps_pkt_histogram_t;

typedef struct wl_adps_dump_histogram_v1 {
	uint16 version;
	uint16 length;
	uint8 mode;				/* operation mode, default = 0 (ADPS disable) */
	uint8 histogram_idx;			/* histogram current idx */
	wl_adps_pkt_histogram_t histogram[1];	/* packet histogram to detect periodicity */
} wl_adps_dump_histogram_v1_t;

/* ADPS Operation flags */
#define OPERATION_PAUSE         0x01
#define OPERATION_SUSPEND	0x01
#define WEAK_SIGNAL             0x02
#define BTCOEX_ACTIVE		0x04
#define DETECT_BADAP		0x08
#define PERIODIC_PATTERN	0x10
#define ADPS_11B_AP			0x20
#define LOW_TX_ABILITY		0x40
#define ADPS_SCAN_IN_PROGRESS	0x80

#define COEX_FLAG_MASK \
	(BTCOEX_ACTIVE)

#define RESTRICT_FLAGS_MASK \
	(COEX_FLAG_MASK | WEAK_SIGNAL | DETECT_BADAP | ADPS_11B_AP | \
	 LOW_TX_ABILITY| ADPS_SCAN_IN_PROGRESS)

#define ADPS_FLAGS_MASK \
	(COEX_FLAG_MASK | OPERATION_SUSPEND | WEAK_SIGNAL | \
	 DETECT_BADAP | ADPS_11B_AP | LOW_TX_ABILITY | ADPS_SCAN_IN_PROGRESS)

#endif  /* _WLC_ADPS_INTERNAL_H_ */
