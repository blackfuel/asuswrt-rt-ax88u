/*
 * DeepSleep Health Check Module
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hnd_trap.h>
#include <bcmutils.h>
#include <hnd_debug.h>
#include <hnd_event.h>
#include <hnd_ds.h>

#ifdef EVENT_LOG_COMPILE
#define DS_HC_ERROR(args)  EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_HEALTH_CHECK_ERROR, args)
#else
#define DS_HC_ERROR(args)  printf args
#endif // endif

bool _ds_hc_enabled = FALSE;
static hnd_hc_info_t *hnd_hc_info = NULL;
static int hnd_healthcheck_fn(uint8 *buffer,
	uint16 length, void *context, int16 *bytes_written);

void
BCMATTACHFN(hnd_health_check_init)(osl_t *osh)
{
	int i, j;
	health_check_client_info_t* hc_cl_info;

	hnd_hc_info = MALLOCZ(osh, sizeof(struct hnd_hc_info));
	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("RTE Health check init module malloc failed\n"));
		return;
	}
	hnd_hc_info->hc = health_check_init(osh,
		HCHK_SW_ENTITY_RTE, sizeof(bcm_dngl_pcie_hc_t));
	if (hnd_hc_info->hc == NULL) {
		DS_HC_ERROR(("RTE Health check init failed\n"));
		MFREE(osh, hnd_hc_info, sizeof(struct hnd_hc_info));
		return;
	}
	hc_cl_info = health_check_module_register(hnd_hc_info->hc,
		hnd_healthcheck_fn, (void *)hnd_hc_info, 0);
	if (hc_cl_info == NULL) {
		DS_HC_ERROR(("RTE Health check init register failed\n"));
		MFREE(osh, hnd_hc_info, sizeof(struct hnd_hc_info));
		return;
	}
	for (i = 0; i < HEALTH_CHECK_LAST_LOG; i ++) {
		if ((hnd_hc_info->healthcheck_log[i] =
			MALLOCZ(osh,
				HEALTH_CHECK_LOG_MAX * sizeof(healthcheck_log_store_t))) == NULL) {
			DS_HC_ERROR(("Cannot allocate healthcheck_log:%d\n", i));
			for (j = 0; j < i; j++) {
				MFREE(osh, hnd_hc_info->healthcheck_log[j],
					HEALTH_CHECK_LOG_MAX * sizeof(healthcheck_log_store_t));
			}
			health_check_module_unregister(hnd_hc_info->hc, hc_cl_info);
			MFREE(osh, hnd_hc_info, sizeof(struct hnd_hc_info));
			return;
		}
	}
	_ds_hc_enabled = TRUE;
}

void
hnd_hc_bus_ds_cb_register(hnd_ds_cb_t cb, void *arg)
{
	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited! Cannot register callback!\n"));
		return;
	}
	hnd_hc_info->ds_cb = cb;
	hnd_hc_info->ds_arg = arg;
}

static mbool *
BCMRAMFN(hnd_health_check_ds_notification_get)(uint8 slice)
{
	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		return 0;
	}
	if (slice >= HEALTH_CHECK_MAX_WL_SILICE)
		return 0;
	return (&(hnd_hc_info->healthcheck_notification[slice]));
}

static healthcheck_log_store_t *
BCMRAMFN(hnd_health_check_ds_log_get)(hnd_health_check_log_type_t logtype)
{
	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		return NULL;
	}
	if (logtype >= HEALTH_CHECK_LAST_LOG)
		return NULL;

	return (hnd_hc_info->healthcheck_log[logtype]);
}

static uint32
BCMRAMFN(hnd_health_check_ds_log_ind_get)(hnd_health_check_log_type_t logtype)
{
	uint32 index;

	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		return NULL;
	}

	if (logtype >= HEALTH_CHECK_LAST_LOG)
		return 0;
	index = hnd_hc_info->healthcheck_log_ind[logtype];
	hnd_hc_info->healthcheck_log_ind[logtype] =
		(hnd_hc_info->healthcheck_log_ind[logtype] + 1) % HEALTH_CHECK_LOG_MAX;

	return index;
}

/* Log healthhceck related actions */
void
hnd_health_check_log(hnd_health_check_log_type_t hc_log_type,
	uint32 val, uint32 caller, uint32 slice)
{
	uint32 log_ind;

	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		return;
	}

	healthcheck_log_store_t *hc_log = hnd_health_check_ds_log_get(hc_log_type);

	if (!hc_log)
		return;

	log_ind = hnd_health_check_ds_log_ind_get(hc_log_type);
	hc_log[log_ind].slice = slice;
	hc_log[log_ind].val = val;
	hc_log[log_ind].caller = caller;
	hc_log[log_ind].ts = OSL_SYSUPTIME();
}

void
hnd_health_check_notify(mbool notification, bool state, uint8 slice)
{
	mbool *ds_notification;

	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		return;
	}
	if (slice >= HEALTH_CHECK_MAX_WL_SILICE)
		return;
	ds_notification = hnd_health_check_ds_notification_get(slice);
	if (state) {
		mboolset(*ds_notification, notification);
	} else {
		mboolclr(*ds_notification, notification);
	}
}

bool
hnd_health_check_ds_notification(void)
{
	int i;
	mbool *ds_notification;

	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		return FALSE;
	}

	for (i = 0; i < HEALTH_CHECK_MAX_WL_SILICE; i ++) {
		ds_notification = hnd_health_check_ds_notification_get(i);
		if (*ds_notification)
			return TRUE;
	}
	return FALSE;
}

mbool
hnd_health_check_ds_notification_dump(void)
{
	int i;
	mbool result = 0;
	mbool *ds_notification;

	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		return result;
	}

	for (i = 0; i < HEALTH_CHECK_MAX_WL_SILICE; i ++) {
		ds_notification = hnd_health_check_ds_notification_get(i);
		result |= *ds_notification <<
			(sizeof(mbool) * NBBY / HEALTH_CHECK_MAX_WL_SILICE) * i;
	}
	return (result);
}

static int
hnd_healthcheck_fn(uint8 *buffer, uint16 length, void *context, int16 *bytes_written)
{
	bcm_dngl_pcie_hc_t *hnd_hc = (bcm_dngl_pcie_hc_t *)buffer;

	if (hnd_hc_info == NULL) {
		DS_HC_ERROR(("ds hc info not inited!\n"));
		ASSERT(0);
	}
	/* system deepsleep healthcheck */
	hnd_hc_info->no_ds_period = hnd_hc_info->ds_cb(hnd_hc_info->ds_arg,
		hnd_health_check_ds_notification(),
		OSL_SYSUPTIME(),
		hnd_hc->pcie_config_regs,
		HC_PCIEDEV_CONFIG_REGLIST_MAX,
		HND_DS_CHECK_TIME_LO_THRESHOLD,
		HND_DS_CHECK_TIME_THRESHOLD);

	if (hnd_hc_info->no_ds_period) {
		*bytes_written = sizeof(bcm_dngl_pcie_hc_t);
		if (hnd_hc_info->no_ds_period > HND_NO_DS_TRAP_THRESHOLD) {
			DS_HC_ERROR(("CHIP NOT GOING TO DEEPSLEEP!\n"));
			hnd_hc->pcie_flag |= HEALTH_CHECK_PCIEDEV_FLAG_NODS;
			hnd_hc_info->ds_hc_trap = TRUE;
			*bytes_written = sizeof(bcm_dngl_pcie_hc_t);
			return (HEALTH_CHECK_STATUS_TRAP);
		} else {
			hnd_hc->pcie_err_ind_type = HEALTH_CHECK_PCIEDEV_NODS_IND;
			return (hnd_hc->pcie_flag << HEALTH_CHECK_STATUS_MSB_SHIFT |
				HEALTH_CHECK_STATUS_ERROR);
		}
	}
	*bytes_written = 0;
	return HEALTH_CHECK_STATUS_OK;
}
