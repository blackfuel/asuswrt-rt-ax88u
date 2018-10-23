/*
 * FCBS module implementation.
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
 * $Id: phy_fcbs.c 657351 2016-08-31 23:00:22Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_fcbs.h"
#include <phy_rstr.h>
#include <phy_fcbs.h>

#define FCBS_MAX_ITERS 200

/* forward declaration */
typedef struct phy_fcbs_mem phy_fcbs_mem_t;

/* module private states */
struct phy_fcbs_info {
	phy_info_t			*pi;	/* PHY info ptr */
	phy_type_fcbs_fns_t *fns;	/* PHY specific function ptrs */
	phy_fcbs_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_fcbs_mem {
	phy_fcbs_info_t 	cmn_info;
	phy_type_fcbs_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */
#ifdef ENABLE_FCBS
static bool wlc_phy_hw_fcbs_init(wlc_phy_t *ppi, int chanidx);
static bool wlc_phy_hw_fcbs_init_chanidx(wlc_phy_t *ppi, int chanidx);
static void wlc_phy_fcbs_read_regs_tbls(phy_info_t *pi, int chanidx, chanspec_t chanspec);
static int wlc_phy_hw_fcbs(wlc_phy_t *ppi, int chanidx, bool set);
static int phy_fcbs_dump(void *ctx, struct bcmstrbuf *b);
#endif /* ENABLE_FCBS */

/* attach/detach */
phy_fcbs_info_t *
BCMATTACHFN(phy_fcbs_attach)(phy_info_t *pi)
{
	phy_fcbs_mem_t	*mem = NULL;
	phy_fcbs_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_fcbs_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize fcbs params */

	/* Register callbacks */
#if defined(BCMDBG)
	/* register dump callback */
#ifdef ENABLE_FCBS
	phy_dbg_add_dump_fn(pi, "phyfcbs", phy_fcbs_dump, cmn_info);
#endif /* ENABLE_FCBS */
#endif // endif

	return cmn_info;

	/* error */
fail:
	phy_fcbs_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_fcbs_detach)(phy_fcbs_info_t *cmn_info)
{
	phy_fcbs_mem_t *mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */

	/* Malloc has failed. No cleanup is necessary here. */
	if (!cmn_info)
		return;

	/* Cleanup the memory associated with cmn_info. */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null fcbs module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_fcbs_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_fcbs_register_impl)(phy_fcbs_info_t *cmn_info, phy_type_fcbs_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*cmn_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_fcbs_unregister_impl)(phy_fcbs_info_t *cmn_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* Local functions */
#ifdef ENABLE_FCBS
static bool wlc_phy_hw_fcbs_init(wlc_phy_t *ppi, int chanidx)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_fcbs_fns_t *fns = pi->fcbsi->fns;

	if (chanidx >= MAX_FCBS_CHANS) {
		PHY_ERROR(("%s: ERROR: Out of empty contexts!!\n", __FUNCTION__));
		return FALSE;
	}

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

	PHY_INFORM(("%s: Initting slot %d, channel %d\n",
		__FUNCTION__, chanidx, CHSPEC_CHANNEL(pi->radio_chanspec)));

	pi->phy_fcbs->chanspec[chanidx] = pi->radio_chanspec;

	/* Get the pointer to the PHY specfic FCBS init function */
	if (fns->fcbsinit == NULL) {
		PHY_ERROR(("%s: Missing fcbsinit pointer\n", __FUNCTION__));
		pi->phy_fcbs->initialized[chanidx] = FALSE;
		ASSERT(fns->fcbsinit);
		return TRUE;
	}
	/* phy-specific fns->fcbsinit function needs to just initialize the pointers to the
	 * lists of regs/tbls that need to be saved into the FCBS TBL
	 */
	fns->fcbsinit(fns->ctx, chanidx, pi->radio_chanspec);
	/* actual reading and saving of all the required regs/btls is done in this fctn */
	pi->phy_fcbs->initialized[chanidx] =
		wlc_phy_hw_fcbs_init_chanidx((wlc_phy_t*)pi, chanidx);

	return TRUE;
}

static bool wlc_phy_hw_fcbs_init_chanidx(wlc_phy_t *ppi, int chanidx)
{
	/* compiling and writing all FCBS data */

	uint16 *p_fcbs_tbl_data;
	fcbs_radioreg_core_list_entry *radioreg_list_ptr;
	uint16 *reg_list_ptr;
	fcbs_phytbl_list_entry *tbl_list_ptr;
	phy_info_t *pi = (phy_info_t*)ppi;
	int length;
	uint8 stall_val;

	p_fcbs_tbl_data = pi->phy_fcbs->hw_fcbs_tbl_data;
	length = 0;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	for (radioreg_list_ptr = pi->phy_fcbs->fcbs_radioreg_list;
			radioreg_list_ptr->regaddr != 0xFFFF; radioreg_list_ptr++) {
		*p_fcbs_tbl_data = (radioreg_list_ptr->regaddr) |
			((radioreg_list_ptr->core_info & 0x7) << FCBS_TBL_RADIOREG_CORE_SHIFT) |
			FCBS_TBL_INST_INDICATOR;
		p_fcbs_tbl_data += (chanidx + 1);
		*p_fcbs_tbl_data = phy_utils_read_radioreg(pi, radioreg_list_ptr->regaddr);
		p_fcbs_tbl_data += (MAX_FCBS_CHANS - chanidx);
	}
	*p_fcbs_tbl_data = 0xFFFF;
	p_fcbs_tbl_data++;

	for (reg_list_ptr = pi->phy_fcbs->fcbs_phyreg_list;
			*reg_list_ptr != 0xFFFF; reg_list_ptr++) {
		*p_fcbs_tbl_data = *reg_list_ptr | FCBS_TBL_INST_INDICATOR;
		p_fcbs_tbl_data += (chanidx + 1);
		*p_fcbs_tbl_data = phy_utils_read_phyreg(pi, *reg_list_ptr);
		p_fcbs_tbl_data += (MAX_FCBS_CHANS - chanidx);
	}
	*p_fcbs_tbl_data = 0xFFFF;
	p_fcbs_tbl_data++;

	if (!pi->phy_fcbs->FCBS_ucode) {
		for (tbl_list_ptr = pi->phy_fcbs->fcbs_phytbl16_list;
		    tbl_list_ptr->tbl_id != 0xFFFF; tbl_list_ptr++) {
			int num_entries;
			int tbl_idx;
			uint16 fcbs_phytbl_copy[20];

			num_entries = tbl_list_ptr->num_entries;
			/* Setup the information tuple */
			*p_fcbs_tbl_data++ = tbl_list_ptr->tbl_id | FCBS_TBL_INST_INDICATOR;
			*p_fcbs_tbl_data++ = tbl_list_ptr->tbl_offset;
			*p_fcbs_tbl_data++ = tbl_list_ptr->tbl_offset +
				tbl_list_ptr->num_entries - 1;

			stall_val =
				(phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev)) &
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev))
				>> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
				1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));
			wlc_phy_table_read_acphy(pi, tbl_list_ptr->tbl_id,
			     tbl_list_ptr->num_entries, tbl_list_ptr->tbl_offset,
			     16, fcbs_phytbl_copy);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
				stall_val <<
				ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));

			/* valA and valB are interleved and we can align the 1st element correctly
			 * and then increment by 2 to write the subsequent entries of the same chan
			 */
			p_fcbs_tbl_data += chanidx;
			for (tbl_idx = 0; tbl_idx < num_entries; tbl_idx++) {
				*p_fcbs_tbl_data = fcbs_phytbl_copy[tbl_idx];
				p_fcbs_tbl_data += MAX_FCBS_CHANS;
			}
			p_fcbs_tbl_data -= chanidx;
			/* Setup the information tuple */
		}
	} else {
		uint16 * p_phytbl16_buf;
		int offset, len, phytbl_idx;
		uint16 num_entries;

		p_phytbl16_buf = pi->phy_fcbs->phytbl16_buf[chanidx];
		phytbl_idx = 0;
		/* radio regs are being handle through HW FCBS  */
		if (chanidx == FCBS_CHAN_A) {
			pi->phy_fcbs->chan_cache_offset[chanidx] =
			    pi->phy_fcbs->cache_startaddr;
		}
		offset = pi->phy_fcbs->chan_cache_offset[chanidx];
		len = 0;
		pi->phy_fcbs->radioreg_cache_offset[chanidx] = offset;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_radioreg, (uint16)len);
		PHY_FCBS(("radio reg buf: start offset=%d, len=%d\n", offset, len));

		/* Store the 16-bit PHY table entries in the FCBS cache */
		offset += len;
		len = 0;
		for (tbl_list_ptr = pi->phy_fcbs->fcbs_phytbl16_list;
		    tbl_list_ptr->tbl_id != 0xFFFF; tbl_list_ptr++) {
			/* Setup the information tuple */
			p_phytbl16_buf[phytbl_idx++] = (tbl_list_ptr->tbl_id << 10) |
			    tbl_list_ptr->tbl_offset;
			p_phytbl16_buf[phytbl_idx++] = tbl_list_ptr->num_entries;
			num_entries = tbl_list_ptr->num_entries;

			stall_val = (phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev))
				& ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev))
				>> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
				1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));
			wlc_phy_table_read_acphy(pi, tbl_list_ptr->tbl_id,
			     tbl_list_ptr->num_entries, tbl_list_ptr->tbl_offset,
			     16, p_phytbl16_buf + phytbl_idx);
			phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
				ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
				stall_val <<
				ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));

			phytbl_idx += tbl_list_ptr->num_entries;

			/* if we don't have even number of entries pad
			 * the buffer by 16-bits of zeros
			 */
			if (((num_entries/2) * 2) != num_entries) {
				p_phytbl16_buf[phytbl_idx++] = 0;
				num_entries += 1;
			}

			len += 4 + (num_entries * 2);
		}

		pi->phy_fcbs->phytbl16_cache_offset[chanidx] = offset;
		/* buffer has to end at 4-byte boundary in the RAM */
		if (((len/4) * 4) != len) {
			len += 2;
		}
		wlapi_bmac_write_template_ram(pi->sh->physhim,
		    offset, len, p_phytbl16_buf);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl16, (uint16)len);
		PHY_FCBS(("phytbl16: start offset = %d, len = %d\n", offset, len));

		/* 32-bit PHY tables entries handled through HW FCBS TBL */
		offset += len;
		len = 0;
		pi->phy_fcbs->phytbl32_cache_offset[chanidx] = offset;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl32, (uint16)len);
		PHY_FCBS(("phytbl32: start offset = %d, len = %d\n", offset, len));

		/* PHY regs handled through HW FCBS TBL */
		offset += len;
		len = 0;
		pi->phy_fcbs->phyreg_cache_offset[chanidx] = offset;

		pi->phy_fcbs->phyreg_buflen[chanidx] = len;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phyreg, (uint16)len);
		PHY_FCBS(("PHY reg buf: start offset=%d, len = %d\n", offset, len));

		if (chanidx == FCBS_CHAN_A) {
			/* Now that we have finished storing the
			 * cache for CHAN_A, we know the starting
			 * cache address for CHAN_B
			 */
			pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_B] = offset + len;
		}

		/* Clear the cache pointer in case it had a non-zero value
		 * before the init
		 */
		wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr, 0);

	}

	*p_fcbs_tbl_data = 0xFFFF;
	p_fcbs_tbl_data++;
	/* length gets number of array entries */
	length = (int)(p_fcbs_tbl_data - pi->phy_fcbs->hw_fcbs_tbl_data);
	stall_val = (phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev)) &
		ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev))
		>> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev);
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
		ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
		1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH,
	                         length, 0, 16, pi->phy_fcbs->hw_fcbs_tbl_data);
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
		ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
		stall_val << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));
	/* FCBS has been written into FCBS tbl */

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
	return TRUE;
}

static void wlc_phy_fcbs_read_regs_tbls(phy_info_t *pi, int chanidx, chanspec_t chanspec)
{
	/* TBD */
}

static int wlc_phy_hw_fcbs(wlc_phy_t *ppi, int chanidx, bool set)
{
	uint16 ptr;
	int i;
	fcbspostfn_t post_fcbs = NULL;
	fcbsprefn_t pre_fcbs = NULL;
	fcbsfn_t fcbs = NULL;
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_fcbs_fns_t *fns = pi->fcbsi->fns;

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));
	if (!pi->phy_fcbs->initialized[chanidx]) {
		return -1;
	}
	if (!set) {
		return pi->phy_fcbs->curr_fcbs_chan;
	}
	pi->phy_fcbs->curr_fcbs_chan = chanidx;

	/* About to start FCBS. Call the PHY specific pre-FCBS function, if any */
	if (fns->prefcbs != NULL)
		fns->prefcbs(fns->ctx, chanidx);

	if (fns->fcbs != NULL)
		fns->fcbs(fns->ctx, chanidx);

	/* If we are currently in the 2G band and we are switching to 5G, tell
	   the ucode to turn the BPHY core off
	*/
	if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[pi->phy_fcbs->curr_fcbs_chan])) {
		if (CHSPEC_IS5G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_OFF);
		}
	} else {
		/* We are currently in 5G. If we are switching to the 2G band
		   tell the ucode to turn the BPHY core ON
		*/
		if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_ON);
		}
	}

	/* If one of the FCBS channel is in the 2G band and the other is in the
	   5G band, then the length of the PHY register cache will be different
	   due to the BPHY register values. In this case update the shmem
	   location with the appropriate value for the channel that we are
	   switching to
	*/
	if (pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_A] !=
		pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_B]) {
		wlapi_bmac_write_shm(pi->sh->physhim,
			pi->phy_fcbs->shmem_phyreg, (uint16)(pi->phy_fcbs->phyreg_buflen[chanidx]));
	}

	/* Now tell the ucode which cache (CHAN_A or CHAN_B) it should use */
	wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr,
	    (uint16) (pi->phy_fcbs->chan_cache_offset[chanidx]));

	/*
	Wait for ucode to write register tables:
	Using 4331 Rev B0
	Measuring 162 usecs from poking cache_ptr until it reads back 0.
	Each wlapi_bmac_read_shm() call takes 1.8 usecs.
	*/
	OSL_DELAY(100);
	ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	for (i = 0; ptr != 0 && i < FCBS_MAX_ITERS; i++) {
		OSL_DELAY(1);
		ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	}
	if (i >= FCBS_MAX_ITERS) {
		PHY_ERROR(("%s: Failed to complete ucode channel switch\n", __FUNCTION__));
	}
	ASSERT(i < FCBS_MAX_ITERS);

	pi->phy_fcbs->switch_count++;

	/* FCBS is done. Call the PHY specific post-FCBS function, if any */
	if (fns->postfcbs != NULL)
		fns->postfcbs(fns->ctx, chanidx);

	return pi->phy_fcbs->curr_fcbs_chan;
}

/* Internal API to other PHY modules/PHY type specific layer */

int phy_fcbs_preinit(phy_info_t *pi, int chanidx)
{
	phy_type_fcbs_fns_t *fns = pi->fcbsi->fns;

	if (fns->prefcbsinit != NULL)
		return fns->prefcbsinit(fns->ctx, chanidx);
	else
		return BCME_UNSUPPORTED;
}

bool wlc_phy_is_fcbs_pending(phy_info_t *pi, chanspec_t chanspec, int *chanidx_ptr)
{
	int chanidx;
	bool retval = FALSE;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (!pi->phy_fcbs->initialized[chanidx]) {
			if ((pi->phy_fcbs->chanspec[chanidx] == chanspec) ||
				((pi->phy_fcbs->chanspec[chanidx] == 0xFFFF))) {
				*chanidx_ptr = chanidx;
				retval = TRUE;
				break;
			}
		}
	}

	return retval;
}

bool wlc_phy_is_fcbs_chan(phy_info_t *pi, chanspec_t chanspec, int *chanidx_ptr)
{
	int chanidx;
	bool retval = FALSE;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (pi->phy_fcbs->initialized[chanidx]) {
			if (pi->phy_fcbs->chanspec[chanidx] == chanspec) {
				*chanidx_ptr = chanidx;
				retval = TRUE;
				break;
			}
		}
	}

	return retval;
}

uint16 wlc_phy_channelindicator_obtain(phy_info_t *pi)
{
	phy_type_fcbs_fns_t *fns = pi->fcbsi->fns;

	if (fns->channelindicator_obtain != NULL)
		return fns->channelindicator_obtain(fns->ctx);
	else
		return 0; /* This should be modified to return unsupported error */
}

/* HAL APIs */

/* Might call mac_suspend() */
/* NOTE: chanidx isn't used anymore, leaving it for now to try it out */

bool wlc_phy_fcbs_init(wlc_phy_t *ppi, int chanidx)
{
	int offset, bphy_offset;
	int len, bphy_len;
	phy_info_t *pi = (phy_info_t*)ppi;

#if defined(FCBS_GPIO_PROFILE)
	uint32 gpio_mask_val = 0x10000; /* CCTRL4331_BT_SHD0_ON_GPIO4 */
#endif // endif
	if (IS_FCBS(pi)) {
		return wlc_phy_hw_fcbs_init((wlc_phy_t*)pi, chanidx);
	}

#if defined(FCBS_GPIO_PROFILE)
	/* Use a GPIO to measure the FCBS time on an oscilloscope */
	if (chanidx == FCBS_CHAN_A) {
		/* Enable the GPIO */
		si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		           gpio_mask_val, 0);
	}
#endif /* FCBS_GPIO_PROFILE */

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

#ifdef LATER
	/* Chan A has to be initialized before Chan B */
	/* Why ?? */
	if ((chanidx == FCBS_CHAN_B) && (!pi->phy_fcbs->initialized[FCBS_CHAN_A])) {
		PHY_ERROR(("%s: Error: Chan A has to be initialized before Chan B\n",
			__FUNCTION__));
		return FALSE;
	}
#endif /* LATER */

	PHY_INFORM(("%s: Initting slot %d, channel %d\n",
		__FUNCTION__, chanidx, CHSPEC_CHANNEL(pi->radio_chanspec)));

	pi->phy_fcbs->chanspec[chanidx] = pi->radio_chanspec;

	/* Verify the pointer to the PHY specfic FCBS init function */
	if (fns->fcbsinit == NULL) {
		PHY_ERROR(("%s: Missing fcbsinit pointer\n", __FUNCTION__));
		pi->phy_fcbs->initialized[chanidx] = FALSE;
		ASSERT(fns->fcbsinit);
		return TRUE;
	}

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	phy_utils_phyreg_enter(pi);

	/* Clear the flag which is used by the PHY specific FCBS init code to indicate
	   if the FCBS engine should load the PHY tables and PHY/Radio registers
	*/
	pi->phy_fcbs->load_regs_tbls = FALSE;

	/* Call the PHY specific initialization function. This will update the phy_fcbs
	   fields with the starting h/w address of the on-chip RAM and the shmem locations
	   used by the driver to specify to the ucode, the various offsets within the
	   FCBS cache

	   This function will also set the pointers to the memory buffers used to
	   read the PHY tables and PHY/radio regs. It may also (optionally) load these
	   buffers with the contents of the PHY tables and PHY/Radio regs.
	*/
	pi->phy_fcbs->initialized[chanidx] = fns->fcbsinit(pi, chanidx, pi->radio_chanspec);

	phy_utils_phyreg_exit(pi);

	if (pi->phy_fcbs->initialized[chanidx] == TRUE) {
		if (chanidx == FCBS_CHAN_A) {
			/* Reset the BPHY update bit. This is used by
			 * the ucode to determine if BPHY registers
			 * need to be updated during FCBS
			 */
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, 0);

			pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_A] =
			    pi->phy_fcbs->cache_startaddr;

			/* This keeps a count of how many times we did a FCBS */
			pi->phy_fcbs->switch_count = 0;
		}

		if (pi->phy_fcbs->load_regs_tbls) {
			/* The PHY specific FCBS init routine wants
			 * the FCBS engine to read the PHY tables
			 * and PHY/Radio regs
			 */
			wlc_phy_fcbs_read_regs_tbls(pi, chanidx,
			    pi->radio_chanspec);
		}

		pi->phy_fcbs->curr_fcbs_chan = chanidx;

		/* Store the radio registers in the FCBS cache */
		offset = pi->phy_fcbs->chan_cache_offset[chanidx];
		/* Each cache entry contains an address and value */
		len = pi->phy_fcbs->num_radio_regs * 2 * sizeof(uint16);
		pi->phy_fcbs->radioreg_cache_offset[chanidx] = offset;
		wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
		    len, pi->phy_fcbs->radioreg_buf[chanidx]);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_radioreg, (uint16)len);
		PHY_FCBS(("radio reg buf: start offset=%d, len=%d\n", offset, len));

		/* Store the 16-bit PHY table entries in the FCBS cache */
		offset += len;
		len = pi->phy_fcbs->phytbl16_buflen;
		pi->phy_fcbs->phytbl16_cache_offset[chanidx] = offset;
		/* buffer has to end at 4-byte boundary in the RAM */
		if (((len/4) * 4) != len) {
			len += 2;
		}
		wlapi_bmac_write_template_ram(pi->sh->physhim,
		    offset, len, pi->phy_fcbs->phytbl16_buf[chanidx]);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl16, (uint16)len);
		PHY_FCBS(("phytbl16: start offset = %d, len = %d\n", offset, len));

		/* Store the 32-bit PHY table entries in the FCBS cache */
		offset += len;
		len = pi->phy_fcbs->phytbl32_buflen;
		pi->phy_fcbs->phytbl32_cache_offset[chanidx] = offset;
		wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
		    len, pi->phy_fcbs->phytbl32_buf[chanidx]);
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phytbl32, (uint16)len);
		PHY_FCBS(("phytbl32: start offset = %d, len = %d\n", offset, len));

		/* Store the PHY registers in the FCBS cache */
		offset += len;
		/* Each cache entry contains an address and value */
		len = pi->phy_fcbs->num_phy_regs * 2 * sizeof(uint16);
		pi->phy_fcbs->phyreg_cache_offset[chanidx] = offset;
		wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
		    len, pi->phy_fcbs->phyreg_buf[chanidx]);

		if (pi->phy_fcbs->num_bphy_regs[chanidx] > 0) {
			/* Store BPHY registers in the cache */
			bphy_offset = offset + len;
			bphy_len = pi->phy_fcbs->num_bphy_regs[chanidx] * sizeof(uint32);
			pi->phy_fcbs->bphyreg_cache_offset[chanidx] = bphy_offset;
			wlapi_bmac_write_template_ram(pi->sh->physhim, bphy_offset,
			    bphy_len, pi->phy_fcbs->bphyreg_buf[chanidx]);
			len += bphy_len;
		}

		pi->phy_fcbs->phyreg_buflen[chanidx] = len;
		wlapi_bmac_write_shm(pi->sh->physhim,
		    pi->phy_fcbs->shmem_phyreg, (uint16)len);
		PHY_FCBS(("PHY reg buf: start offset=%d, len = %d\n", offset, len));

		if (chanidx == FCBS_CHAN_A) {
			/* Now that we have finished storing the
			 * cache for CHAN_A, we know the starting
			 * cache address for CHAN_B
			 */
			pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_B] = offset + len;
		}

		/* Clear the cache pointer in case it had a non-zero value
		 * before the init
		 */
		wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr, 0);

		wlapi_enable_mac(pi->sh->physhim);
	}
	return TRUE;
}

/* using chanspec of 0 cancels an arm request */
/* Might call mac_suspend() */
bool wlc_phy_fcbs_arm(wlc_phy_t *ppi, chanspec_t chanspec, int chanidx)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));
	ASSERT(!wf_chspec_malformed(chanspec) || chanspec == 0 || chanspec == 0xff);

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (pi->phy_fcbs->chanspec[chanidx] == chanspec) {
			PHY_ERROR(("%s: channel %d is already armed.\n",
				__FUNCTION__, CHSPEC_CHANNEL(chanspec)));
			return TRUE;
		}
	}

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
			if ((!pi->phy_fcbs->chanspec[chanidx]))
				break;
	}

	if (chanidx >= MAX_FCBS_CHANS) {
		PHY_ERROR(("%s: ERROR: No Free Entry!!\n", __FUNCTION__));
		return FALSE;
	}

	/* If we lucky enough to be on channel, call init directly */
	if ((chanspec == pi->radio_chanspec) && IS_FCBS(pi) &&
	    !(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi)) &&
		(chanidx == wlc_phy_channelindicator_obtain(pi))) {
		PHY_ERROR(("%s: Already on channel %d, call fcbs_init immediatly\n",
			__FUNCTION__, CHSPEC_CHANNEL(pi->radio_chanspec)));
		pi->phy_fcbs->chanspec[chanidx] = chanspec;
		return wlc_phy_fcbs_init(ppi, chanidx);
	} else {
		PHY_INFORM(("%s: Arming channel %d\n", __FUNCTION__, CHSPEC_CHANNEL(chanspec)));
		pi->phy_fcbs->chanspec[chanidx] = chanspec;
		pi->phy_fcbs->initialized[chanidx] = FALSE;
		return TRUE;
	}
}

void wlc_phy_fcbs_exit(wlc_phy_t *ppi)
{
	int chanidx;
	phy_info_t *pi = (phy_info_t*)ppi;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		pi->phy_fcbs->chanspec[chanidx] = 0;
		pi->phy_fcbs->initialized[chanidx] = FALSE;
	}
}

int wlc_phy_fcbs(wlc_phy_t *ppi, int chanidx, bool set)
{
	fcbspostfn_t post_fcbs = NULL;
	fcbsprefn_t pre_fcbs = NULL;
	uint16 ptr;
	int i;
	phy_info_t *pi = (phy_info_t*)ppi;
#if defined(FCBS_CPU_PROFILE)
	unsigned long tick1, tick2, tick_diff;
#endif /* FCBS_CPU_PROFILE */

	if (IS_FCBS(pi)) {
		return wlc_phy_hw_fcbs((wlc_phy_t*)pi, chanidx, set);
	}
	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

	if (!pi->phy_fcbs->initialized[chanidx]) {
		return -1;
	}

	if (!set) {
		return pi->phy_fcbs->curr_fcbs_chan;
	}

#if defined(FCBS_GPIO_PROFILE)
	/* Assert GPIO 4 before switching */
	si_gpioout(pi->sh->sih, (1 << 4), (1 << 4), GPIO_DRV_PRIORITY);
	si_gpioouten(pi->sh->sih, (1 << 4), (1 << 4), GPIO_DRV_PRIORITY);
#endif /* FCBS_GPIO_PROFILE */

	/* If we are currently in the 2G band and we are switching to 5G, tell
	   the ucode to turn the BPHY core off
	*/
	if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[pi->phy_fcbs->curr_fcbs_chan])) {
		if (CHSPEC_IS5G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_OFF);
		}
	} else {
		/* We are currently in 5G. If we are switching to the 2G band
		   tell the ucode to turn the BPHY core ON
		*/
		if (CHSPEC_IS2G(pi->phy_fcbs->chanspec[chanidx])) {
			wlapi_bmac_write_shm(pi->sh->physhim,
			    pi->phy_fcbs->shmem_bphyctrl, FCBS_BPHY_ON);
		}
	}

	pi->phy_fcbs->curr_fcbs_chan = chanidx;

	/* If one of the FCBS channel is in the 2G band and the other is in the
	   5G band, then the length of the PHY register cache will be different
	   due to the BPHY register values. In this case update the shmem
	   location with the appropriate value for the channel that we are
	   switching to
	*/
	if (pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_A] !=
		pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_B]) {
		wlapi_bmac_write_shm(pi->sh->physhim,
			pi->phy_fcbs->shmem_phyreg, (uint16)(pi->phy_fcbs->phyreg_buflen[chanidx]));
	}

	/* About to start FCBS. Call the PHY specific pre-FCBS function, if any */
	if (fns->prefcbs != NULL)
		fns->prefcbs(fns->ctx, chanidx);

	/* Now tell the ucode which cache (CHAN_A or CHAN_B) it should use */
	wlapi_bmac_write_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr,
	    (uint16) (pi->phy_fcbs->chan_cache_offset[chanidx] >> 2));

	pi->phy_fcbs->switch_count++;

#if defined(FCBS_GPIO_PROFILE)
	/* De-assert GPIO 4 after switching */
	si_gpioout(pi->sh->sih, (1 << 4), 0, GPIO_DRV_PRIORITY);
	si_gpioouten(pi->sh->sih, (1 << 4), (1 << 4),
	    GPIO_DRV_PRIORITY);
#endif /* FCBS_GPIO_PROFILE */

#if defined(FCBS_CPU_PROFILE)
	OSL_GETCYCLES(tick1);
#endif /* FCBS_CPU_PROFILE */

	/*
	Wait for ucode to write register tables:
	Using 4331 Rev B0
	Measuring 162 usecs from poking cache_ptr until it reads back 0.
	Each wlapi_bmac_read_shm() call takes 1.8 usecs.
	*/
	OSL_DELAY(100);
	ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	for (i = 0; ptr != 0 && i < FCBS_MAX_ITERS; i++) {
		OSL_DELAY(1);
		ptr = wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr);
	}
#if defined(FCBS_CPU_PROFILE)
	OSL_GETCYCLES(tick2);
	tick_diff = tick2 - tick1;
	PHY_ERROR(("%s: completed chan %d, %ld usecs\n",
		__FUNCTION__, CHSPEC_CHANNEL(pi->phy_fcbs->chanspec[chanidx]),
		tick_diff/3000)); /* 3 Ghz cpu */
#endif /* FCBS_CPU_PROFILE */
	if (i >= FCBS_MAX_ITERS) {
		PHY_ERROR(("%s: Failed to complete ucode channel switch\n", __FUNCTION__));
	}
	ASSERT(i < FCBS_MAX_ITERS);

	/* FCBS is done. Call the PHY specific post-FCBS function, if any */
	if (fns->postfcbs != NULL)
		fns->postfcbs(fns->ctx, chanidx);

	return pi->phy_fcbs->curr_fcbs_chan;
}

bool wlc_phy_fcbs_uninit(wlc_phy_t *ppi, chanspec_t chanspec)
{
	int chanidx;
	phy_info_t *pi = (phy_info_t*)ppi;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		if (pi->phy_fcbs->chanspec[chanidx] == chanspec) {
			pi->phy_fcbs->chanspec[chanidx] = 0;
			pi->phy_fcbs->initialized[chanidx] = FALSE;
			PHY_INFORM(("%s: Uninitting channel %d on idx %d\n",
				__FUNCTION__, CHSPEC_CHANNEL(chanspec), chanidx));

			return TRUE;
		}
	}
	PHY_ERROR(("%s: Cannot uninit unused fcbs context!! 0x%x, %d\n",
		__FUNCTION__, chanspec, CHSPEC_CHANNEL(chanspec)));

	return FALSE;
}

static int
phy_fcbs_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_fcbs_info_t *fcbsi = (phy_fcbs_info_t *)ctx;
	phy_info_t *pi = fcbsi->pi;
	uint shmem_radioreg, shmem_phytbl16, shmem_phytbl32;
	uint shmem_phyreg, shmem_bphyctrl, shmem_cache_ptr;
	int bwidx[2];
	int len, cache_consumed;
	char *bwstr [ ] = {"", "20MHz", "40MHz"};

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	shmem_radioreg = (pi->phy_fcbs->shmem_radioreg != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_radioreg) : 0;

	shmem_phytbl16 = (pi->phy_fcbs->shmem_phytbl16 != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_phytbl16) : 0;

	shmem_phytbl32 = (pi->phy_fcbs->shmem_phytbl32 != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_phytbl32) : 0;

	shmem_phyreg = (pi->phy_fcbs->shmem_phyreg != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_phyreg) : 0;

	shmem_bphyctrl = (pi->phy_fcbs->shmem_bphyctrl != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_bphyctrl) : 0;

	shmem_cache_ptr = (pi->phy_fcbs->shmem_cache_ptr != 0) ?
	    wlapi_bmac_read_shm(pi->sh->physhim, pi->phy_fcbs->shmem_cache_ptr) : 0;

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	bcm_bprintf(b, "FCBS h/w addresses:\n");
	bcm_bprintf(b, "-------------------\n");
	bcm_bprintf(b, "RAM start address      = 0x%04x\n", pi->phy_fcbs->cache_startaddr);
	bcm_bprintf(b, "SHMEM radio address    = 0x%04x\n", pi->phy_fcbs->shmem_radioreg);
	bcm_bprintf(b, "SHMEM phytbl16 address = 0x%04x\n", pi->phy_fcbs->shmem_phytbl16);
	bcm_bprintf(b, "SHMEM phytbl32 address = 0x%04x\n", pi->phy_fcbs->shmem_phytbl32);
	bcm_bprintf(b, "SHMEM phyreg address   = 0x%04x\n", pi->phy_fcbs->shmem_phyreg);
	bcm_bprintf(b, "SHMEM bphyctrl address = 0x%04x\n", pi->phy_fcbs->shmem_bphyctrl);
	bcm_bprintf(b, "SHMEM cacheptr address = 0x%04x\n\n", pi->phy_fcbs->shmem_cache_ptr);

	bcm_bprintf(b, "FCBS shmem values:\n");
	bcm_bprintf(b, "------------------\n");
	bcm_bprintf(b, "radioreg  = %d\n", shmem_radioreg);
	bcm_bprintf(b, "phytbl16  = %d\n", shmem_phytbl16);
	bcm_bprintf(b, "phytbl32  = %d\n", shmem_phytbl32);
	bcm_bprintf(b, "phyreg    = %d\n", shmem_phyreg);
	bcm_bprintf(b, "bphyctrl  = 0x%04x\n", shmem_bphyctrl);
	bcm_bprintf(b, "cacheptr  = 0x%04x\n\n", shmem_cache_ptr);

	cache_consumed = 0;
	bcm_bprintf(b, "FCBS cache consumption:\n");
	bcm_bprintf(b, "-----------------------\n");
	len = pi->phy_fcbs->num_radio_regs * 2 * sizeof(uint16);
	bcm_bprintf(b, "Radio reg cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "   CHAN_B = %d bytes\n", len);
	cache_consumed += len;

	len = pi->phy_fcbs->phytbl16_buflen;
	if (((len/4) * 4) != len) {
		len += 2;
	}
	bcm_bprintf(b, "PHYTBL16 cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "   CHAN_B = %d bytes\n", len);
	cache_consumed += len;

	len = pi->phy_fcbs->phytbl32_buflen;
	bcm_bprintf(b, "PHYTBL32 cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "   CHAN_B = %d bytes\n", len);
	cache_consumed += len;

	len = pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_A];
	bcm_bprintf(b, "PHY reg cache:\n");
	bcm_bprintf(b, "   CHAN_A = %d bytes\n", len);
	cache_consumed += len;
	len = pi->phy_fcbs->phyreg_buflen[FCBS_CHAN_B];
	bcm_bprintf(b, "   CHAN_B = %d bytes\n\n", len);
	cache_consumed += len;
	bcm_bprintf(b, "Total cache used by FCBS = %d bytes\n\n", cache_consumed);

	bcm_bprintf(b, "FCBS reg and table entry count:\n");
	bcm_bprintf(b, "-------------------------------\n");
	bcm_bprintf(b, "Radio regs       = %d\n", pi->phy_fcbs->num_radio_regs);
	bcm_bprintf(b, "PHYTBL16 entries = %d\n", pi->phy_fcbs->phytbl16_entries);
	bcm_bprintf(b, "PHYTBL32 entries = %d\n", pi->phy_fcbs->phytbl32_entries);
	bcm_bprintf(b, "PHY regs         = %d\n", pi->phy_fcbs->num_phy_regs);
	bcm_bprintf(b, "BPHY regs:\n");
	bcm_bprintf(b, "   CHAN_A = %d\n", pi->phy_fcbs->num_bphy_regs[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = %d\n\n", pi->phy_fcbs->num_bphy_regs[FCBS_CHAN_B]);

	bcm_bprintf(b, "FCBS cache offsets:\n");
	bcm_bprintf(b, "-------------------\n");
	bcm_bprintf(b, "Channel cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->chan_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "Radio reg cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->radioreg_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->radioreg_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "PHYTBL16 cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->phytbl16_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->phytbl16_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "PHYTBL32 cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->phytbl32_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->phytbl32_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "PHY reg cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->phyreg_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n", pi->phy_fcbs->phyreg_cache_offset[FCBS_CHAN_B]);
	bcm_bprintf(b, "BPHY reg cache offset:\n");
	bcm_bprintf(b, "   CHAN_A = 0x%04x\n", pi->phy_fcbs->bphyreg_cache_offset[FCBS_CHAN_A]);
	bcm_bprintf(b, "   CHAN_B = 0x%04x\n\n", pi->phy_fcbs->bphyreg_cache_offset[FCBS_CHAN_B]);

	if (pi->phy_fcbs->initialized[FCBS_CHAN_A]) {
		if (CHSPEC_IS40(pi->phy_fcbs->chanspec[FCBS_CHAN_A])) {
			bwidx[0] = 2;
		} else {
			bwidx[0] = 1;
		}
	} else {
		bwidx[0] = 0;
	}

	if (pi->phy_fcbs->initialized[FCBS_CHAN_B]) {
		if (CHSPEC_IS40(pi->phy_fcbs->chanspec[FCBS_CHAN_B])) {
			bwidx[1] = 2;
		} else {
			bwidx[1] = 1;
		}
	} else {
		bwidx[1] = 0;
	}
	bcm_bprintf(b, "FCBS driver internal:\n");
	bcm_bprintf(b, "---------------------\n");
	bcm_bprintf(b, "Initialized   : CHAN_A=%3d, CHAN_B=%3d\n",
	    pi->phy_fcbs->initialized[FCBS_CHAN_A], pi->phy_fcbs->initialized[FCBS_CHAN_B]);
	bcm_bprintf(b, "Channels      : CHAN_A=%3d, CHAN_B=%3d\n",
	    CHSPEC_CHANNEL(pi->phy_fcbs->chanspec[FCBS_CHAN_A]),
	    CHSPEC_CHANNEL(pi->phy_fcbs->chanspec[FCBS_CHAN_B]));
	bcm_bprintf(b, "Bandwidth     : CHAN_A=%s, CHAN_B=%s\n", bwstr[bwidx[0]], bwstr[bwidx[1]]);
	bcm_bprintf(b, "Channel index : %d\n", pi->phy_fcbs->curr_fcbs_chan);
	bcm_bprintf(b, "Switch count  : %d\n", pi->phy_fcbs->switch_count);
	bcm_bprintf(b, "Load regs/tbls: %d\n", pi->phy_fcbs->load_regs_tbls);

	return BCME_OK;
}
#endif /* ENABLE_FCBS */
