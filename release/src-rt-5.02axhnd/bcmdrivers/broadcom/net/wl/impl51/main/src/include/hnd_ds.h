/*
 * DeepSleep Health check module header file
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
 * $Id: hnd_ds.h 650948 2016-07-23 04:28:56Z $
 */

#ifndef _HND_DS_H
#define _HND_DS_H

#include <typedefs.h>
#ifdef HEALTH_CHECK
#include <hnd_hchk.h>
#endif /* HEALTH_CHECK */

extern bool _ds_hc_enabled;
#ifdef HEALTH_CHECK
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define HND_DS_HC_ENAB()   (_ds_hc_enabled)
#elif defined(HEALTH_CHECK_DISABLED)
	#define HND_DS_HC_ENAB()   (0)
#else
	#define HND_DS_HC_ENAB()   (1)
#endif // endif
#else
	#define HND_DS_HC_ENAB()   (0)
#endif /* HEALTH_CHECK */

#define HND_DS_CHECK_TIME_LO_THRESHOLD  5000 /* ms */
#define HND_DS_CHECK_TIME_THRESHOLD     HND_DS_CHECK_TIME_LO_THRESHOLD + 5000
#define HND_NO_DS_TRAP_THRESHOLD        HND_DS_CHECK_TIME_THRESHOLD + 5000

/* defined for healthcheck notifications */
#define HEALTH_CHECK_WL_SCAN_IN_PROGRESS       0x00000001
#define HEALTH_CHECK_WL_STAY_AWAKE       0x00000002
#define HEALTH_CHECK_WL_RADIO_MPC_OFF       0x00000004
#define HEALTH_CHECK_WL_RADIO_ENABLE       0x00000008

#define HEALTH_CHECK_MAX_WL_SILICE  2
#define HEALTH_CHECK_LOG_MAX    5

typedef int (*hnd_ds_cb_t)(void *arg, uint32 now, bool hnd_ds_notif, uint32 *reg,
	uint32 size, uint32 lo_thr, uint32 hi_thr);

typedef enum _hnd_health_check_log_type {
	HEALTH_CHECK_SECI_CLK_LOG = 0,
	HEALTH_CHECK_WL_PS_LOG = 1,
	HEALTH_CHECK_WL_CLK_LOG = 2,
	HEALTH_CHECK_LAST_LOG
} hnd_health_check_log_type_t;

typedef struct _healthcheck_log_store {
    uint32  slice;
    uint32  val;
    uint32  caller;
    uint32 ts;
} healthcheck_log_store_t;

#ifdef HEALTH_CHECK
typedef struct hnd_hc_info {
	health_check_info_t *hc;    /* Healthcheck Descriptor */
	/* notify healthcheck related inputs */
	mbool healthcheck_notification[HEALTH_CHECK_MAX_WL_SILICE];
	/* log healthcheck input */
	healthcheck_log_store_t *healthcheck_log[HEALTH_CHECK_LAST_LOG];
	uint32 healthcheck_log_ind[HEALTH_CHECK_LAST_LOG];
	hnd_ds_cb_t ds_cb;
	void	*ds_arg;
	uint32	no_ds_period;
	bool	ds_hc_trap;
} hnd_hc_info_t;
#endif /* HEALTH_CHECK */

extern void hnd_health_check_init(osl_t *osh);

extern void hnd_health_check_notify(mbool notification, bool state, uint8 slice);

extern bool hnd_health_check_ds_notification(void);

extern mbool hnd_health_check_ds_notification_dump(void);

void hnd_health_check_log(hnd_health_check_log_type_t hc_log_type,
	uint32 val, uint32 caller, uint32 slice);

extern void hnd_hc_bus_ds_cb_register(hnd_ds_cb_t cb, void *arg);

#endif /* DS Health Check */
