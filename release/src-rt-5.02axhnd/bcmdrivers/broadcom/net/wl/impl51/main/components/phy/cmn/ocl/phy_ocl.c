/*
 * One Core Listen (OCL) phy module implementation
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
 * $Id: $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_ocl.h>
#include <phy_ocl.h>
#include <phy_ocl_api.h>

/* module private states */
struct phy_ocl_info {
	phy_info_t *pi;
	phy_type_ocl_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_ocl_info_t ocl_info;
	phy_type_ocl_fns_t fns;
/* add other variable size variables here at the end */
} phy_ocl_mem_t;

/* function prototypes */

#ifdef OCL
/* attach/detach */
phy_ocl_info_t *
BCMATTACHFN(phy_ocl_attach)(phy_info_t *pi)
{
	phy_ocl_info_t *ocl_info;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((ocl_info = phy_malloc(pi, sizeof(phy_ocl_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	ocl_info->pi = pi;
	ocl_info->fns = &((phy_ocl_mem_t *)ocl_info)->fns;

	return ocl_info;

	/* error */
fail:
	phy_ocl_detach(ocl_info);
	return NULL;

}

void

BCMATTACHFN(phy_ocl_detach)(phy_ocl_info_t *ocl_info)
{
	phy_info_t *pi;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (ocl_info == NULL) {
		PHY_INFORM(("%s: null ocl module\n", __FUNCTION__));
		return;
	}

	pi = ocl_info->pi;
	phy_mfree(pi, ocl_info, sizeof(phy_ocl_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_ocl_register_impl)(phy_ocl_info_t *ocl_info,
	phy_type_ocl_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ocl_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_ocl_unregister_impl)(phy_ocl_info_t *ocl_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* **************************************** */
/*	PHY HAL functions                                           */
/* **************************************** */
int
phy_ocl_coremask_change(wlc_phy_t *ppi, uint8 coremask)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_ocl_fns_t *fns = pi->ocli->fns;

	if (fns->ocl_coremask_change) {
		return (fns->ocl_coremask_change)(fns->ctx, coremask);
	} else {
		return BCME_UNSUPPORTED;
	}
}

uint8
phy_ocl_get_coremask(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_ocl_fns_t *fns = pi->ocli->fns;

	if (fns->ocl_get_coremask) {
		return (fns->ocl_get_coremask)(fns->ctx);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_ocl_status_get(wlc_phy_t *ppi, uint16 *reqs, uint8 *coremask, bool *ocl_en)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_ocl_fns_t *fns = pi->ocli->fns;

	if (fns->ocl_status_get) {
		return (fns->ocl_status_get)(fns->ctx, reqs, coremask, ocl_en);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_ocl_disable_req_set(wlc_phy_t *ppi, uint16 req, bool disable, uint8 req_id)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_ocl_fns_t *fns = pi->ocli->fns;

	if (fns->ocl_disable_req_set) {
		return (fns->ocl_disable_req_set)(fns->ctx, req, disable, req_id);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* OCL */
