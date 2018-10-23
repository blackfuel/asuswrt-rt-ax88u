/*
 * ACPHY Core module implementation
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
 * $Id: phy_ac_iovt.c 647115 2016-07-04 01:33:05Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <wlc_iocv_types.h>
#include <phy_ac_chanmgr_iov.h>
#include <phy_ac_misc_iov.h>
#include <phy_ac_radio_iov.h>
#include <phy_ac_rssi_iov.h>
#include <phy_ac_rxgcrs_iov.h>
#include <phy_ac_tbl_iov.h>
#include <phy_ac_tpc_iov.h>
#include "phy_type_ac.h"
#include "phy_type_ac_iovt.h"

/* local functions */

/* register iovar tables/handlers to IOC module */
int
BCMATTACHFN(phy_ac_register_iovt)(phy_info_t *pi, phy_type_info_t *ti, wlc_iocv_info_t *ii)
{
	int err;

	/* Register Channel Manager module ACPHY iovar table/handlers */
	if ((err = phy_ac_chanmgr_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_chanmgr_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register Miscellaneous module ACPHY iovar table/handlers */
	if ((err = phy_ac_misc_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_misc_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register Radio control module ACPHY iovar table/handlers */
	if ((err = phy_ac_radio_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_radio_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register RSSICompute module ACPHY iovar table/handlers */
	if ((err = phy_ac_rssi_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_rssi_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register Rx Gain Control and Carrier Sense module ACPHY iovar table/handlers */
	if ((err = phy_ac_rxgcrs_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_rxgcrs_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PHYTbl module ACPHY iovar table/handlers */
	if ((err = phy_ac_tbl_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_tbl_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register TxPowerControl module ACPHY iovar table/handlers */
	if ((err = phy_ac_tpc_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_tpc_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}
