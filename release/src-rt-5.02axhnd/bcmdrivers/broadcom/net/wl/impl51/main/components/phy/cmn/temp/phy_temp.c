/*
 * TEMPerature sense module implementation.
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
 * $Id: phy_temp.c 763047 2018-05-17 04:32:07Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_rstr.h>
#include "phy_type_temp.h"
#include "phy_temp_st.h"
#include <phy_temp.h>
#include <phy_hc.h>
#include <phy_temp_api.h>

#include <phy_utils_var.h>

/* module private states */
struct phy_temp_info {
	phy_info_t *pi;
	phy_type_temp_fns_t *fns;
	/* tempsense */
	phy_txcore_temp_t *temp;
};

/* module private states memory layout */
typedef struct {
	phy_temp_info_t info;
	phy_type_temp_fns_t fns;
	phy_txcore_temp_t temp;
/* add other variable size variables here at the end */
} phy_temp_mem_t;

static void wlc_phy_read_tempdelta_settings(phy_temp_info_t *tempi, int maxtempdelta);

/* local function declaration */

/* attach/detach */
phy_temp_info_t *
BCMATTACHFN(phy_temp_attach)(phy_info_t *pi)
{
	phy_temp_info_t *info;
	phy_txcore_temp_t *temp;
	uint8 init_txrxchain;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_temp_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_temp_mem_t *)info)->fns;

	/* Parameters for temperature-based fallback to 1-Tx chain */
	temp = &((phy_temp_mem_t *)info)->temp;
	info->temp = temp;

	init_txrxchain = (1 << PHYCORENUM(pi->pubpi->phy_corenum)) - 1;

	temp->disable_temp = (uint8)PHY_GETINTVAR(pi, rstr_tempthresh);
	temp->disable_temp_max_cap = temp->disable_temp;
	temp->hysteresis = (uint8)PHY_GETINTVAR(pi, rstr_temps_hysteresis);
	if ((temp->hysteresis == 0) || (temp->hysteresis == 0xf)) {
		temp->hysteresis = PHY_HYSTERESIS_DELTATEMP;
	}

	temp->enable_temp =
		temp->disable_temp - temp->hysteresis;

	temp->heatedup = FALSE;
	temp->degrade1RXen = FALSE;

	temp->bitmap = (init_txrxchain << 4 | init_txrxchain);

#ifdef DUTY_CYCLE_THROTTLING
	pi->_dct = TRUE;
#else
	pi->_dct = FALSE;
#endif /* DUTY_CYCLE_THROTTLING */
	temp->duty_cycle = 100;
	temp->duty_cycle_throttle_depth = 10;
	temp->duty_cycle_throttle_state = 0;
	temp->skip_tempsense = FALSE;

	pi->phy_tempsense_offset = 0;

	wlc_phy_read_tempdelta_settings(info, PHY_CAL_MAXTEMPDELTA);

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_temp_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_temp_detach)(phy_temp_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null temp module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_temp_mem_t));
}

/*
 * Query the states pointer.
 */
phy_txcore_temp_t *
phy_temp_get_st(phy_temp_info_t *ti)
{
	return ti->temp;
}

/* temp. throttle */
uint16
phy_temp_throttle(phy_temp_info_t *ti)
{
	phy_type_temp_fns_t *fns = ti->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->throt == NULL)
		return 0;

	return (fns->throt)(fns->ctx);
}

/* vbat functions */
uint8
phy_vbat_sense_read(wlc_phy_t *ppi)
{
	phy_type_temp_fns_t *fns = ((phy_info_t *)ppi)->tempi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->vbat_sense == NULL)
		return 0;

	return (fns->vbat_sense)(fns->ctx);
}

uint8
phy_vbat_sense_get(wlc_phy_t *ppi)
{
	phy_type_temp_fns_t *fns = ((phy_info_t *)ppi)->tempi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->vbat_get == NULL)
		return 0;

	return (fns->vbat_get)(fns->ctx);
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_temp_register_impl)(phy_temp_info_t *ti, phy_type_temp_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ti->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_temp_unregister_impl)(phy_temp_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

#if defined(BCMDBG) || defined(WLTEST) || defined(TEMPSENSE_OVERRIDE)
int16
phy_temp_get_override(wlc_phy_t *ppi)
{
	return  ((phy_info_t *)ppi)->tempsense_override;
}
#endif /* BCMDBG || WLTEST || TEMPSENSE_OVERRIDE */

#ifdef	WL_DYNAMIC_TEMPSENSE
int
phy_temp_get_thresh(phy_temp_info_t *ti)
{
	phy_txcore_temp_t *temp = ti->temp;

	return temp->disable_temp;
}

#endif /* WL_DYNAMIC_TEMPSENSE */

int16
phy_temp_sense_get(wlc_phy_t *ppi)
{
	phy_type_temp_fns_t *fns = ((phy_info_t *)ppi)->tempi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->get == NULL)
		return PHY_TEMP_MIN;

	return ((int16)(fns->get)(fns->ctx));
}

int16
phy_temp_sense_read(wlc_phy_t *ppi)
{
	phy_type_temp_fns_t *fns = ((phy_info_t *)ppi)->tempi->fns;
	int16 retval = PHY_TEMP_MIN;

	if (fns->do_tempsense) {
		retval = (fns->do_tempsense)(fns->ctx);
	}

	return retval;
}

void
wlc_phy_upd_gain_wrt_temp_phy(phy_info_t *pi, int16 *gain_err_temp_adj)
{
	phy_type_temp_fns_t *fns = pi->tempi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	*gain_err_temp_adj = 0;
	if (fns->upd_gain != NULL) {
		(fns->upd_gain)(fns->ctx, gain_err_temp_adj);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

void
wlc_phy_upd_gain_wrt_gain_cal_temp_phy(phy_info_t *pi, int16 *gain_err_temp_adj)
{
	phy_type_temp_fns_t *fns = pi->tempi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));
	*gain_err_temp_adj = 0;
	if (fns->upd_gain_cal != NULL) {
		(fns->upd_gain_cal)(fns->ctx, gain_err_temp_adj);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

bool
phy_temp_is_heatedup(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_txcore_temp_t *temp = phy_temp_get_st(pi->tempi);

	return temp->heatedup;
}

/*
 * Read the phy calibration temperature delta parameters from NVRAM.
 */
static void
BCMATTACHFN(wlc_phy_read_tempdelta_settings)(phy_temp_info_t *tempi, int maxtempdelta)
{
	phy_info_t *pi = tempi->pi;
	phy_txcore_temp_t *temp = phy_temp_get_st(tempi);

	/* Read the temperature delta from NVRAM */
	temp->phycal_tempdelta = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_phycal_tempdelta, 0);

	/* Range check, disable if incorrect configuration parameter */
	/* Preserve default, in case someone wants to use it. */
	if (temp->phycal_tempdelta > maxtempdelta) {
		temp->phycal_tempdelta = temp->phycal_tempdelta_default;
	} else {
		temp->phycal_tempdelta_default = temp->phycal_tempdelta;
	}
}
#ifdef RADIO_HEALTH_CHECK

#define PHY_INVALID_TEMPERATURE	-128

int
phy_temp_get_cur_temp_radio_health_check(phy_temp_info_t *ti)
{
	phy_type_temp_fns_t *fns = ti->fns;
	phy_info_t *pi = ti->pi;
	int ct = PHY_INVALID_TEMPERATURE;

	if (fns->get != NULL)
		ct = (fns->get)(fns->ctx);

	if (ct >= phy_hc_get_rhc_tempthresh(pi->hci))
		return BCME_RANGE;

	return ct;
}

int
phy_temp_current_temperature_check(phy_temp_info_t *tempi)
{
	int temperature;
	if ((temperature = phy_temp_get_cur_temp_radio_health_check(tempi)) == BCME_RANGE)
		return BCME_RANGE;
	return BCME_OK;
}
#endif /* RADIO_HEALTH_CHECK */
