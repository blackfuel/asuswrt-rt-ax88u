/*
 * PHY Core module implementation - IOVarTable registration
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
 * $Id: phy_iovt.c 692080 2017-03-25 01:24:18Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_api.h>
#include "phy_iovt.h"
#include <phy_btcx_iov.h>
#include <phy_chanmgr_iov.h>
#include <phy_calmgr_iov.h>
#include <phy_hirssi_iov.h>
#include <phy_radar_iov.h>
#include <phy_temp_iov.h>
#include <phy_dsi_iov.h>
#include <phy_misc_iov.h>
#include <phy_noise_iov.h>
#include <phy_tpc_iov.h>
#include <phy_rxgcrs_iov.h>
#include <phy_antdiv_iov.h>
#include <phy_papdcal_iov.h>
#include <phy_rssi_iov.h>
#include <phy_rxspur_iov.h>
#include <phy_vcocal_iov.h>
#include <phy_tssical_iov.h>
#ifdef WL_NAP
#include <phy_nap_iov.h>
#endif // endif
#include <phy_txiqlocal_iov.h>
#include <wlc_iocv_types.h>
#include <phy_fcbs_iov.h>
#include <phy_dbg_iov.h>
#ifdef WLC_TXPWRCAP
#include <phy_txpwrcap_iov.h>
#endif // endif
#ifdef IQPLAY_DEBUG
#include <phy_samp_iov.h>
#endif /* IQPLAY_DEBUG */
#ifdef RADIO_HEALTH_CHECK
#include <phy_hc_iov.h>
#endif /* RADIO_HEALTH_CHECK */

/* local functions */

#ifndef ALL_NEW_PHY_MOD
int phy_legacy_register_iovt(phy_info_t *pi, wlc_iocv_info_t *ii);
#endif // endif

/* Register all modules' iovar tables/handlers */
int
BCMATTACHFN(phy_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	ASSERT(ii != NULL);

#ifndef ALL_NEW_PHY_MOD
	/* Register legacy iovar table/handlers */
	if (phy_legacy_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_legacy_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

#if defined(AP) && defined(RADAR)
	/* Register radar common iovar table/handlers */
	if (phy_radar_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_radar_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register TEMPerature sense common iovar table/handlers */
	if (phy_temp_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_temp_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

#ifdef WL_DSI
#endif /* WL_DSI */

	/* Register  Miscellaneous module common iovar table/handlers */
	if (phy_misc_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_misc_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register TxPowerControl common iovar table/handlers */
	if (phy_tpc_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_tpc_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

#ifdef WLC_TXPWRCAP
	/* Register TxPowerCap common iovar table/handlers */
	if (phy_txpwrcap_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_txpwrcap_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register Rx Gain Control and Carrier Sense common iovar table/handlers */
	if (phy_rxgcrs_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_rxgcrs_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register TSSI CAL common iovar table/handlers */
	if (phy_tssical_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_tssical_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#ifdef WL_NAP
	/* Register NAP common iovar table/handlers */
	if (phy_nap_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_nap_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* Register ANTennaDIVersity common iovar tables/handlers */
	if (phy_antdiv_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_antdiv_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PAPD Calibration common iovar tables/handlers */
	if (phy_papdcal_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_papdcal_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register VCO Calibration common iovar tables/handlers */
	if (phy_vcocal_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_vcocal_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register Channel Manager common iovar tables/handlers */
	if (phy_chanmgr_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_chanmgr_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register Cal Manager common iovar tables/handlers */
	if (phy_calmgr_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_calmgr_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#ifdef ENABLE_FCBS
	/* Register FCBS module common iovar table/handlers */
	if (phy_fcbs_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_fcbs_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* ENABLE_FCBS */
	/* Register tx iqlo calibration module common iovar table/handlers */
	if (phy_txiqlocal_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_txiqlocal_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register hi rssi elna bypass module common iovar table/handlers */
	if (phy_hirssi_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_hirssi_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register bluetooth coexistence module common iovar table/handlers */
	if (phy_btcx_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_btcx_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register RSSICompute module common iovar table/handlers */
	if (phy_rssi_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_rssi_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register NOISEmeasure module common iovar table/handlers */
	if (phy_noise_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_rxspur_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register Rx Spur canceller module common iovar table/handlers */
	if (phy_rxspur_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_rxspur_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

#ifdef IQPLAY_DEBUG
	/* Register sample play  module common iovar table/handlers */
	if (phy_samp_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_samp_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* IQPLAY_DEBUG */

#ifdef RADIO_HEALTH_CHECK
	/* Register health check common iovar table/handlers */
	if (phy_hc_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_hc_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* RADIO_HEALTH_CHECK */
#ifdef PHY_DBG_ENABLED
	/* Register dbg common iovar table/handlers */
	if (phy_dbg_register_iovt(pi, ii) != BCME_OK) {
		PHY_ERROR(("%s: phy_dbg_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* PHY_DBG_ENABLED */
	/* Register other modules' common iovar tables/dispatchers here ... */

	return BCME_OK;

fail:
	return BCME_ERROR;
}
