/*
 * vasip module implementation.
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
 * $Id: phy_vasip.c 748133 2018-02-21 19:19:22Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_rstr.h>
#include <phy_vasip.h>
#include <phy_vasip_api.h>
#include <phy_utils_var.h>

/* forward declaration */
#if defined(BCMDBG)
static int phy_dump_bfd_status(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* attach/detach */
phy_vasip_info_t *
BCMATTACHFN(phy_vasip_attach)(phy_info_t *pi)
{
	phy_vasip_mem_t	*mem = NULL;
	phy_vasip_info_t *cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_vasip_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Register callbacks */

#if defined(BCMDBG)
	phy_dbg_add_dump_fn(pi, "bfdstatus", phy_dump_bfd_status, cmn_info);
#endif // endif

	return cmn_info;

	/* error */
fail:
	phy_vasip_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_vasip_detach)(phy_vasip_info_t *cmn_info)
{
	phy_vasip_mem_t	*mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info)
		return;

	/* Freeup memory associated with cmn_info */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null vasip module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_vasip_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_vasip_register_impl)(phy_vasip_info_t *vi, phy_type_vasip_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*vi->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_vasip_unregister_impl)(phy_vasip_info_t *vi)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/*
 * Return vasip version, -1 if not present.
 */
uint8
phy_vasip_get_ver(phy_info_t *pi)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->get_ver != NULL)
		return fns->get_ver(fns->ctx);
	else
		return VASIP_NOVERSION;
}

/*
 * reset/activate vasip.
 */
void
phy_vasip_reset_proc(phy_info_t *pi, int reset)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->reset_proc != NULL)
		fns->reset_proc(fns->ctx, reset);
}

void
phy_vasip_set_clk(phy_info_t *pi, bool val)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->set_clk != NULL)
		fns->set_clk(fns->ctx, val);
}

void
phy_vasip_write_bin(phy_info_t *pi, const uint32 vasip_code[], const uint nbytes)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->write_bin != NULL) {
		fns->write_bin(fns->ctx, vasip_code, nbytes);
	}
}

#ifdef VASIP_SPECTRUM_ANALYSIS
void
phy_vasip_write_spectrum_tbl(phy_info_t *pi,
        const uint32 vasip_spectrum_tbl[], const uint nbytes_tbl)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->write_spectrum_tbl != NULL) {
		fns->write_spectrum_tbl(fns->ctx, vasip_spectrum_tbl, nbytes_tbl);
	}
}
#endif // endif

void
phy_vasip_write_svmp(phy_info_t *pi, uint32 offset, uint16 val)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->write_svmp != NULL) {
		fns->write_svmp(fns->ctx, offset, val);
	}
}

void
phy_vasip_read_svmp(phy_info_t *pi, uint32 offset, uint16 *val)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->read_svmp != NULL) {
		fns->read_svmp(fns->ctx, offset, val);
	}
}

void
phy_vasip_write_svmp_blk(phy_info_t *pi, uint32 offset, uint16 len, uint16 *val)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->write_svmp_blk != NULL) {
		fns->write_svmp_blk(fns->ctx, offset, len, val);
	}
}

void
phy_vasip_read_svmp_blk(phy_info_t *pi, uint32 offset, uint16 len, uint16 *val)
{
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->read_svmp_blk != NULL) {
		fns->read_svmp_blk(fns->ctx, offset, len, val);
	}
}

#if defined(BCMDBG)
static int
phy_dump_bfd_status(void *ctx, struct bcmstrbuf *b)
{
	phy_vasip_info_t *cmn_info = (phy_vasip_info_t *) ctx;
	phy_info_t *pi = cmn_info->pi;
	phy_type_vasip_fns_t *fns = pi->vasipi->fns;

	if (fns->dump_bfd_status != NULL) {
		fns->dump_bfd_status(fns->ctx, b);
	}
	return BCME_OK;
}
#endif // endif
