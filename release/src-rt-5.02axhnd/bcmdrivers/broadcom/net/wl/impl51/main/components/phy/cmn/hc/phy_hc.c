/*
 * Health check module.
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
 * $Id: phy_hc.c 690362 2017-03-15 23:31:16Z $
 */
#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_hc.h"
#include <phy_hc.h>
#include <phy_utils_var.h>
#include <phy_rstr.h>
#include <phy_hc_api.h>
#include "phy_radio_api.h"
#include <phy_temp_api.h>

/* module private states */
struct phy_hc_info {
	phy_info_t *pi;
	uint8 hc_tempfail_count;
	int8	hc_mode;	/* -1 : Auto, 0: Disabled, 1: Trigger immediate */
	uint16 rhc_tempthresh;
	uint16 rhc_temp_fail_time;
	bool _health_check;
	phy_type_hc_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_hc_info_t info;
	phy_type_hc_fns_t fns;
/* add other variable size variables here at the end */
} phy_hc_mem_t;

/* Health check Support */
#ifdef RADIO_HEALTH_CHECK
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define HEALTH_CHECK_ENAB(hci)   (hci->_health_check)
#elif defined(RADIO_HEALTH_CHECK_DISABLED)
	#define HEALTH_CHECK_ENAB(hci)   0
#else
	#define HEALTH_CHECK_ENAB(hci)   (hci->_health_check)
#endif // endif
#else
	#define HEALTH_CHECK_ENAB(hci)   0
#endif	/* RADIO_HEALTH_CHECK */

#ifdef RADIO_HEALTH_CHECK

/* local function declaration */
static void BCMATTACHFN(phy_hc_read_rhc_tempthresh)(phy_hc_info_t *info);
static bool phy_hc_wd(phy_wd_ctx_t *ctx);

/* attach/detach */
phy_hc_info_t *
BCMATTACHFN(phy_hc_attach)(phy_info_t *pi)
{
	phy_hc_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_hc_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_hc_mem_t *)info)->fns;

	/* Initialize health check params */

	/* Initiailize the consecutive health check temperature failures */
	info->hc_tempfail_count = 0;
	/* Set the default health check mode */
	info->hc_mode = -1;
	info->_health_check = TRUE;
	phy_hc_read_rhc_tempthresh(info);

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_hc_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_HEALTH_CHECK,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
	return info;

	/* error */
fail:
	phy_hc_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_hc_detach)(phy_hc_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null health check module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_hc_mem_t));
}

static void
BCMATTACHFN(phy_hc_read_rhc_tempthresh)(phy_hc_info_t *hci)
{
	phy_info_t *pi = hci->pi;
	hci->rhc_tempthresh = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_rhc_tempthresh,
		PHY_HC_TEMP_THRESHOLD);
	hci->rhc_temp_fail_time = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_rhc_temp_fail_time,
		PHY_HC_TEMP_FAIL_THRESHOLD);
}

/* Watchdog call back */
static bool
phy_hc_wd(phy_wd_ctx_t *ctx)
{
	phy_hc_info_t *hci = (phy_hc_info_t *)ctx;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (HEALTH_CHECK_ENAB(hci)) {
		if (hci->hc_mode)
			phy_hc_healthcheck(hci, PHY_HC_ALL);
	}

	return TRUE;
}

/*
 * Register/unregister PHY type implementation to the Health check module.
 * It returns BCME_XXXX.
 */
int
phy_hc_register_impl(phy_hc_info_t *hci, phy_type_hc_fns_t *fns)
{
	*(hci->fns) = *fns;

	return BCME_OK;
}

void
phy_hc_unregister_impl(phy_hc_info_t *hci)
{
	BCM_REFERENCE(hci);
	/* nothing to do at this moment */
}

/* Radio/PHY Health check support functions */
int
phy_hc_debugcrash_forcefail(wlc_phy_t *ppi, phy_crash_reason_t dctype)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_hc_info_t *hci = pi->hci;
	phy_type_hc_fns_t *fns = hci->fns;

	PHY_TRACE(("%s: Debug Crash type : %d \n", __FUNCTION__, dctype));

	if (HEALTH_CHECK_ENAB(hci)) {
		if (fns->force_fail)
			return (fns->force_fail)(fns->ctx, dctype);
	}
	return BCME_UNSUPPORTED;
}

int
phy_hc_debugcrash_health_check(wlc_phy_t *ppi, phy_healthcheck_type_t hc)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_hc_info_t *hci = pi->hci;
	if (HEALTH_CHECK_ENAB(hci)) {
		return phy_hc_healthcheck(hci, hc);
	} else {
		return PHY_RC_NONE;
	}
}

int
phy_hc_healthcheck(phy_hc_info_t *hci, phy_healthcheck_type_t hc)
{
	phy_crash_reason_t rc = PHY_RC_NONE;
	phy_info_t *pi = hci->pi;
	PHY_TRACE(("%s: Health check type : %d \n", __FUNCTION__, hc));

	/* Tempsense check */
	if ((hc == PHY_HC_ALL) || (hc == PHY_HC_TEMPSENSE)) {
		int16 temperature;
		if (phy_temp_current_temperature_check(pi->tempi) == BCME_RANGE) {
			temperature = phy_temp_sense_read((wlc_phy_t *)pi);
			if (temperature != PHY_TEMP_MIN) {
				PHY_INFORM(("\n %s : current temperature : %d \n", __FUNCTION__,
						temperature));
				if (phy_temp_current_temperature_check(pi->tempi) == BCME_RANGE)
					hci->hc_tempfail_count++;
				else
					hci->hc_tempfail_count = 0;
				if (hci->hc_tempfail_count > hci->rhc_temp_fail_time) {
					phy_radio_switch(pi, OFF);
					rc = PHY_RC_TEMPSENSE_LIMITS;
					PHY_FATAL_ERROR_MESG(("\n %s : Temperature exceeds the"
						"limits: %d\n", __FUNCTION__, rc));
					goto end;
				}
			}
		} else {
			hci->hc_tempfail_count = 0;
		}
	}
	/* VCO check */
	if ((hc == PHY_HC_ALL) || (hc == PHY_HC_VCOCAL)) {
		if (phy_radio_pll_lock(pi->radioi) == FALSE) {
			bool pll_lock_status;
			if ((RADIOID(pi->pubpi->radioid) == BCM2069_ID) ||
				(RADIOID(pi->pubpi->radioid) == BCM20691_ID) ||
				(RADIOID(pi->pubpi->radioid) == BCM20693_ID) ||
				(RADIOID(pi->pubpi->radioid) == BCM20694_ID)) {
				int vcocal_status;
				phy_vcocal_force(pi);
				vcocal_status = phy_vcocal_status(pi->vcocali);
				if (vcocal_status == RADIO2069X_VCOCAL_NOT_DONE) {
					rc = PHY_RC_VCOCAL_FAILED;
					PHY_FATAL_ERROR_MESG(("\n %s : VCO cal not done: %d \n",
						__FUNCTION__, rc));
					goto end;
				}
			}
			pll_lock_status = phy_radio_pll_lock(pi->radioi);
			if (pll_lock_status == FALSE) {
				rc = PHY_RC_PLL_NOTLOCKED;
				PHY_FATAL_ERROR_MESG(("\n %s : PLL not locked: %d \n",
						__FUNCTION__, rc));
				goto end;
			}
		}
	}
	/* Rx chain */
	/* Desense check */
	if ((hc == PHY_HC_ALL) || (hc == PHY_HC_RX)) {
		if ((rc = phy_noise_healthcheck_desense(pi->noisei)) != PHY_RC_NONE) {
			goto end;
		}
	}
	/* Tx chain */
	if ((hc == PHY_HC_ALL) || (hc == PHY_HC_TX)) {
		uint8 rxchain, txchain;
		phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

		if ((rc = phy_radio_health_check_baseindex(pi)) != PHY_RC_NONE) {
			goto end;
		}
		/* Monitor Tx chain */
		rxchain = stf_shdata->phyrxchain;
		txchain = stf_shdata->phytxchain;
		if ((txchain & rxchain) != txchain) {
			rc = PHY_RC_TXCHAIN_INVALID;
			PHY_FATAL_ERROR_MESG(("\n %s : Tx chain (0x%x) is not a subset of Rx chain"
				" (0x%x) \n", __FUNCTION__, txchain, rxchain));
			goto end;
		}
	}
	return BCME_OK;
end:
	/* Fatal error */
	PHY_FATAL_ERROR_MESG(("\n %s: HC ERROR : %d \n", __FUNCTION__, rc));
	PHY_FATAL_ERROR(pi, rc);
	return BCME_ERROR;
}

int
phy_hc_iovar_hc_mode(phy_hc_info_t *hci, bool get, int int_val)
{
	if (!HEALTH_CHECK_ENAB(hci)) {
		return BCME_UNSUPPORTED;
	} else if (get) {
		return hci->hc_mode;
	} else {
		int err = BCME_OK;

		if (int_val == -1) {
			hci->hc_mode = -1;
		} else if (int_val == 0) {
			hci->hc_mode = 0;
		} else if (int_val == 1) {
			err = phy_hc_healthcheck(hci, PHY_HC_ALL);
		}
		else
			err = BCME_UNSUPPORTED;
		return err;
	}
}

uint16
phy_hc_get_rhc_tempthresh(phy_hc_info_t *hci)
{
	return hci->rhc_tempthresh;
}
#endif /* RADIO_HEALTH_CHECK */
