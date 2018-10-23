/*
 * HWA library routines for MAC facing blocks: 1b, 2a, 3b, and 4a
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
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 */

/*
 * README, how to dump hwa
 * Usage:
 * wl dump hwa [-b <blocks> -v -r -s -f <HWA fifos> -h]
 *	b: specific hwa blocks that you are interesting
 *	<blocks>: <top cmn dma 1a 1b 2a 2b 3a 3b 4a 4b>
 *	v: verbose, to dump more information
 *	r: dump registers
 *	s: dump txfifo shadow
 *	f: specific fifos that you are interesting
 *	<HWA fifos>: <0..69>
 *	h: help
 * NOTE: The <HWA fifos> is physical fifo index.
 *	More FIFO mapping info is in WLC_HW_MAP_TXFIFO.
 * NOTE: Dump 3b TxFIFO context is dangerous, it may cause AXI timeout.
 *	(Use it only for debugging purpose)
 * Example: Dump HWA 3b block info for fifos 65, 67.
 *	wl dump hwa [-b 3b -v -r -f 65 67]
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlc.h>
#include <wlc_tx.h>
#include <wlc_rx.h>
#include <wlc_ampdu_rx.h>
#include <wlc_amsdu.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_pktfetch.h>
#include <phy_rssi_api.h>
#ifdef DONGLEBUILD
#include <wl_rte_priv.h>
#endif /* DONGLEBUILD */
#include <hwa_lib.h>
#include "hnddma_priv.h"
#include <wlc_event_utils.h>
#include <wlc_cfp.h>

typedef struct wl_info wl_info_t; // forward declaration

#ifdef HWA_RXFILL_BUILD

#ifdef HWA_RXFILL_RXFREE_AUDIT_ENABLED

/* Audits the RX buffer id between HWA and SW */
static inline void
_hwa_rxfill_rxfree_audit(hwa_rxfill_t *rxfill, uint32 bufidx, const bool alloc)
{
	// Always +1, because the bufidx can be zero.
	bufidx += 1;

	if (bufidx > HWA_RXPATH_PKTS_MAX) {
		HWA_ERROR(("_hwa_rxfill_rxfree_audit: Invalid bufidx<%d>, %s\n", bufidx,
			alloc ? "alloc" : "free"));
		return;
	}

	if (alloc) {
		if (!bcm_mwbmap_isfree(rxfill->rxfree_map, bufidx)) {
			HWA_ERROR(("RxBM audit: Get duplicate rxbuffer<%d> "
				"from RxBM\n", bufidx));
			return;
		}
		bcm_mwbmap_force(rxfill->rxfree_map, bufidx);
	} else {
		if (bcm_mwbmap_isfree(rxfill->rxfree_map, bufidx)) {
			HWA_ERROR(("RxBM audit: Double free rxbuffer<%d> "
				"to RxBM\n", bufidx));
			return;
		}
		bcm_mwbmap_free(rxfill->rxfree_map, bufidx);
	}
} // _hwa_rxfill_rxfree_audit

/* Audits the RX buffer id between HWA and SW */
void
hwa_rxfill_rxfree_audit(struct hwa_dev *dev, uint32 core,
	hwa_rxbuffer_t *rx_buffer, const bool alloc)
{
	uint32 bufidx;
	hwa_rxfill_t *rxfill;
	HWA_DEBUG_EXPR(uint32 offset);

	rxfill = &dev->rxfill;

	// Audit parameters and pre-conditions
	HWA_ASSERT(rx_buffer != (hwa_rxbuffer_t*)NULL);
	HWA_ASSERT(core < HWA_RX_CORES);

	// Audit rx_buffer, wrong rx_buffer causes 1b stuck.
	HWA_ASSERT(rx_buffer >= (hwa_rxbuffer_t*)dev->rx_bm.memory);
	HWA_ASSERT(rx_buffer <= ((hwa_rxbuffer_t*)dev->rx_bm.memory) + (dev->rx_bm.pkt_total - 1));
	HWA_DEBUG_EXPR({
		offset = ((char *)rx_buffer) - ((char *)dev->rx_bm.memory);
		HWA_ASSERT((offset % dev->rx_bm.pkt_size) == 0);
	});

	// Convert rxbuffer pointer to its index within Rx Buffer Manager
	bufidx = HWA_TABLE_INDX(hwa_rxbuffer_t, dev->rx_bm.memory, rx_buffer);

	_hwa_rxfill_rxfree_audit(rxfill, bufidx, alloc);

} // hwa_rxfill_rxfree_audit

#endif /* HWA_RXFILL_RXFREE_AUDIT_ENABLED */

hwa_rxfill_t *
BCMATTACHFN(hwa_rxfill_attach)(hwa_dev_t *dev)
{
	hwa_rxfill_t *rxfill;

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	// Setup locals
	rxfill = &dev->rxfill;

#ifdef HWA_RXFILL_RXFREE_AUDIT_ENABLED

	// Handler for hwa rx buffer index
	rxfill->rxfree_map = bcm_mwbmap_init(dev->osh, HWA_RXPATH_PKTS_MAX + 1);
	if (rxfill->rxfree_map == NULL) {
		HWA_ERROR(("rxfree_map for audit allocation failed\n"));
		goto failure;
	}
	bcm_mwbmap_force(rxfill->rxfree_map, 0); /* id=0 is invalid */

	return rxfill;

failure:
	hwa_rxfill_detach(rxfill);
	HWA_WARN(("%s attach failure\n", HWA1b));

	return ((hwa_rxfill_t*)NULL);
#else

	return rxfill;

#endif /* HWA_RXFILL_RXFREE_AUDIT_ENABLED */
}

void
BCMATTACHFN(hwa_rxfill_detach)(hwa_rxfill_t *rxfill)
{
#ifdef HWA_RXFILL_RXFREE_AUDIT_ENABLED
	hwa_dev_t *dev;

	HWA_FTRACE(HWA1b);

	if (rxfill == (hwa_rxfill_t*)NULL)
		return;

	// Audit pre-conditions
	dev = HWA_DEV(rxfill);

	// Handler for hwa rx buffer index
	if (rxfill->rxfree_map != NULL) {
		bcm_mwbmap_fini(dev->osh, rxfill->rxfree_map);
		rxfill->rxfree_map = NULL;
	}
#endif /* HWA_RXFILL_RXFREE_AUDIT_ENABLED */

	return;
}

/*
 * -----------------------------------------------------------------------------
 * Section: HWA1b RxFill block ... may be used in Dongle and NIC mode
 * -----------------------------------------------------------------------------
 *
 * CONFIG:
 * -------
 * - hwa_rxfill_fifo_attach() may be invoked by WL layer, after allocating the
 *   MAC FIFO0 and/or FIFO1 DMA descriptor rings. FIFO DMA descriptor rings will
 *   continue to be allocated by WL layer using hnddma library dma_ringalloc().
 *   WL layer will pass the MD descriptor ring parameters to HWA1b RxFill block
 *   at attach time using hwa_rxfill_fifo_attach(). No registers are configured
 *   in this HWA1b RxFill library function, as hwa_attach() may not yet be
 *   invoked.
 *
 * - hwa_rxfill_init() may be invoked AFTER hwa_rxfill_fifo_attach() is invoked.
 *   H2S and S2H interfaces are allocated and the HWA1b RxFill block is
 *   configured. RxFill relies on Rx Buffer Manager.
 *
 * RUNTIME:
 * --------
 * - hwa_rxfill_rxbuffer_free() handles rxBuffer free requests from upper layer.
 *   An RxBuffer may be paired with a host side buffer context saved in the RPH.
 *
 * - hwa_rxfill_rxbuffer_process() is invoked by the H2S WR index update
 *   interrupt path. The WR index will be updated when the D11 MAC completes
 *   packet reception into the Rx FIFO0 and FIFO1. This function processes the
 *   received packet from RD to WR index, by invoking the bound upper layer
 *   handler. The RPH and the rxBuffer pointer are handed to the upper layer
 *   handler, which may chose to convert to the native network stack packet
 *   context, e.g. mbuf, skbuff, fkbuf, lbuf. In Full Dongle, with .11 to .3
 *   conversion offloaded to the D11 MAC, the excplicit conversion to RxLbufFrag
 *   may be skipped and directly RxRordering using RxCpls may be performed.
 *
 * DEBUG:
 * ------
 * hwa_rxfill_fifo_avail() queries HWA for number of available RxBuffers.
 * hwa_rxfill_dump() debug support for HWA1b RxFill block
 *
 * -----------------------------------------------------------------------------
 */

static int hwa_rxfill_bmac_recv(void *context, uintptr arg1, uintptr arg2,
	uint32 core, uint32 arg4);
static int hwa_rxfill_bmac_done(void *context, uintptr arg1, uintptr arg2,
	uint32 core, uint32 rxfifo_cnt);

void // WL layer may directly configure HWA MAC FIFOs
BCMATTACHFN(hwa_rxfill_fifo_attach)(void *wlc, uint32 core, uint32 fifo,
	uint32 depth, dma64addr_t fifo_addr, uint32 dmarcv_ptr)
{
	hwa_dev_t *dev;
	hwa_rxfill_t *rxfill; // HWA1b RxFill SW state

	HWA_TRACE(("%s PHASE MAC FIFO ATTACH wlc<%p> core<%u> fifo<%u>"
		" depth<%u> addr<0x%08x,0x%08x> dmarecv_ptr<0x%08x>\n",
		HWA1b, wlc, core, fifo,
		depth, fifo_addr.loaddr, fifo_addr.hiaddr, dmarcv_ptr));

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit

	// Audit parameters and pre-conditions
	HWA_ASSERT(wlc != (void*)NULL);
	HWA_ASSERT(core < HWA_RX_CORES);
	HWA_ASSERT(fifo < HWA_RXFIFO_MAX);
	HWA_ASSERT(depth != 0U);
	HWA_ASSERT((fifo_addr.loaddr | fifo_addr.hiaddr) != 0U);

	// Setup locals
	rxfill = &dev->rxfill;

	// Save settings in local state
	rxfill->wlc[core] = wlc;
	rxfill->fifo_depth[core][fifo] = depth;
	rxfill->dmarcv_ptr[core][fifo] = dmarcv_ptr;
	rxfill->fifo_addr[core][fifo].loaddr = fifo_addr.loaddr;
	rxfill->fifo_addr[core][fifo].hiaddr = fifo_addr.hiaddr;

	// Mark FIFO as initialized. Used in construction of H2S RxFIFO interface
	rxfill->inited[core][fifo] = TRUE;

} // hwa_rxfill_fifo_attach

int // HWA1b: Allocate resources configuration for HWA1b block
hwa_rxfill_preinit(hwa_rxfill_t *rxfill)
{
	hwa_dev_t *dev;
	void *memory;
	uint32 core, depth, mem_sz;
	uint8 rxh_offset;
	hwa_regs_t *regs;

	HWA_FTRACE(HWA1b);

	// Audit pre-conditions
	dev = HWA_DEV(rxfill);

	// Setup locals
	regs = dev->regs;

	// Configure RxFill
	if (dev->host_addressing == HWA_32BIT_ADDRESSING) {
		rxfill->config.rph_size = sizeof(hwa_rxpost_hostinfo32_t);
		rxfill->config.addr_offset =
			OFFSETOF(hwa_rxpost_hostinfo32_t, data_buf_haddr32);
	} else { // HWA_64BIT_ADDRESSING 64bit host
		rxfill->config.rph_size = sizeof(hwa_rxpost_hostinfo64_t);
		rxfill->config.addr_offset =
			OFFSETOF(hwa_rxpost_hostinfo64_t, data_buf_haddr64);
	}

	//These fields will be updated later
	dev->rxfill.config.wrxh_offset = 0;
	dev->rxfill.config.d11_offset = 0;
	dev->rxfill.config.len_offset = ~0;

	// Verify HWA1b block's structures
	HWA_ASSERT(sizeof(hwa_rxfill_rxfree_t) == HWA_RXFILL_RXFREE_BYTES);
	HWA_ASSERT(sizeof(hwa_rxfill_rxfifo_t) == HWA_RXFILL_RXFIFO_BYTES);

	// Setup D11 offset and SW RXHDR offset
	rxfill->config.d11_offset = WLC_RXHDR_LEN;
	rxfill->config.wrxh_offset = rxfill->config.rph_size;
	// B0 has the same requirement.
	rxh_offset = (rxfill->config.rph_size + rxfill->config.d11_offset);
	rxfill->config.wrxh_offset += rxh_offset % 8;

	rxfill->config.rx_size = HWA_RXBUFFER_BYTES - rxfill->config.wrxh_offset;

	HWA_TRACE(("%s config rph<%u> offset d11<%u> len<%u> addr<%u>\n",
		HWA1b, dev->rxfill.config.rph_size,
		dev->rxfill.config.d11_offset, dev->rxfill.config.len_offset,
		dev->rxfill.config.addr_offset));

	// We need RxBM at 8B alignment.
	mem_sz = HWA_RXBUFFER_BYTES * HWA_RXPATH_PKTS_MAX;
	// We need RxBM at 8B alignment.
	if ((memory = MALLOC_ALIGN(dev->osh, mem_sz, 3)) == NULL) {
		HWA_ERROR(("%s rxbm malloc size<%u> failure\n", HWA00, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		goto failure;
	}
	ASSERT(ISALIGNED(memory, 8));
	bzero(memory, mem_sz);

	HWA_TRACE(("%s rxbm +memory[%p,%u]\n", HWA00, memory, mem_sz));
	hwa_bm_config(dev, &dev->rx_bm, "BM RxPATH", HWA_RX_BM,
		HWA_RXPATH_PKTS_MAX, HWA_RXBUFFER_BYTES,
		HWA_PTR2UINT(memory), HWA_PTR2HIADDR(memory), memory);

	rxfill->rxbm_base = dev->rx_bm.pkt_base.loaddr; // loaddr ONLY

	for (core = 0; core < HWA_RX_CORES; core++) {
		// Allocate and initialize S2H "FREEIDXSRC" interface
		depth = HWA_RXFILL_RXFREE_DEPTH;
		mem_sz = depth * sizeof(hwa_rxfill_rxfree_t);
		// (HWA_RXFILL_RXFREE_BYTES == 4B, should be NO)
		if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
			HWA_ERROR(("%s rxfree malloc size<%u> failure\n", HWA1b, mem_sz));
			HWA_ASSERT(memory != (void*)NULL);
			goto failure;
		}
		HWA_TRACE(("%s rxfree_ring +memory[%p,%u]\n", HWA1b, memory, mem_sz));
		hwa_ring_init(&rxfill->rxfree_ring[core], "RXF", HWA_RXFILL_ID,
			HWA_RING_S2H, HWA_RXFILL_RXFREE_S2H_RINGNUM, depth, memory,
			&regs->rx_core[core].freeidxsrc_ring_wrindex,
			&regs->rx_core[core].freeidxsrc_ring_rdindex);

		// Allocate and initialize H2S "D11BDEST" interface
		depth = rxfill->fifo_depth[core][0];
		mem_sz = depth * sizeof(hwa_rxfill_rxfifo_t);
		// (HWA_RXFILL_RXFIFO_BYTES == 4B, should be NO)
		if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
			HWA_ERROR(("%s rxfifo malloc size<%u> failure\n", HWA1b, mem_sz));
			HWA_ASSERT(memory != (void*)NULL);
			goto failure;
		}
		HWA_TRACE(("%s rxfifo_ring +memory[%p,%u]\n", HWA1b, memory, mem_sz));
		hwa_ring_init(&rxfill->rxfifo_ring[core], "D11", HWA_RXFILL_ID,
			HWA_RING_H2S, HWA_RXFILL_RXFIFO_H2S_RINGNUM, depth, memory,
			&regs->rx_core[core].d11bdest_ring_wrindex,
			&regs->rx_core[core].d11bdest_ring_rdindex);
	}

	// Override registered dpc callback handler
	hwa_register(dev, HWA_RXFIFO_RECV, dev, hwa_rxfill_bmac_recv);
	hwa_register(dev, HWA_RXFIFO_DONE, dev, hwa_rxfill_bmac_done);

	return HWA_SUCCESS;

failure:

	hwa_rxfill_free(rxfill);

	return HWA_FAILURE;

}

void // HWA1b: Free resources for HWA1b block
hwa_rxfill_free(hwa_rxfill_t *rxfill)
{
	void *memory;
	uint32 core, mem_sz;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA1b);

	if (rxfill == (hwa_rxfill_t*)NULL)
		return; // nothing to release done

	// Audit pre-conditions
	dev = HWA_DEV(rxfill);

	if (dev->rx_bm.memory != (void*)NULL) {
		memory = dev->rx_bm.memory;
		mem_sz = HWA_RXBUFFER_BYTES * HWA_RXPATH_PKTS_MAX;
		HWA_TRACE(("%s rx_bm -memory[%p,%u]\n", HWA1b, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		dev->rx_bm.memory = (void*)NULL;
	}

	for (core = 0; core < HWA_RX_CORES; core++) {
		// Release resources used by HWA1b RxFill "FREEIDXSRC" interface
		if (rxfill->rxfree_ring[core].memory != (void*)NULL) {
			memory = rxfill->rxfree_ring[core].memory;
			mem_sz = HWA_RXFILL_RXFREE_DEPTH * sizeof(hwa_rxfill_rxfree_t);
			HWA_TRACE(("%s rxfree_ring -memory[%p,%u]\n",
			       HWA1b, memory, mem_sz));
			MFREE(dev->osh, memory, mem_sz);
			hwa_ring_fini(&rxfill->rxfree_ring[core]);
			rxfill->rxfree_ring[core].memory = (void*)NULL;
		}

		// Release resources used by HWA1b RxFill "D11BDEST" interface
		if (rxfill->rxfifo_ring[core].memory != (void*)NULL) {
			memory = rxfill->rxfifo_ring[core].memory;
			mem_sz = rxfill->fifo_depth[core][0] * sizeof(hwa_rxfill_rxfifo_t);
			HWA_TRACE(("%s rxfifo_ring -memory[%p,%u]\n",
			       HWA1b, memory, mem_sz));
			MFREE(dev->osh, memory, mem_sz);
			hwa_ring_fini(&rxfill->rxfifo_ring[core]);
			rxfill->rxfifo_ring[core].memory = (void*)NULL;
		}
	}

}

int // HWA1b RxFill: Init RxFill interfaces AFTER MAC FIFOs are attached.
hwa_rxfill_init(hwa_rxfill_t *rxfill)
{
	void *memory;
	uint32 u32, core;
	uint32 ring_cfg, ring_intraggr; // S2H and H2S ring interfaces
	uint32 fifo_cfg, fifo_intraggr; // MAC RX FIFO0 and FIFO1
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	uint16 u16;

	// Setup locals
	dev = HWA_DEV(rxfill);
	regs = dev->regs;

	// Confirm that the HWA Rx Buffer Manager is indeed initialized
	// All buffer indices are with respect to Rx Buffer Manager base address
	HWA_ASSERT(dev->rx_bm.enabled == TRUE);

	// Compute values common to HWA1b RxFill FREEIDXSRC and D11BDEST interfaces

	// D0DEST and D1DEST use same SHIFT MASK values
	fifo_intraggr = (0U
		| BCM_SBF(HWA_RXFILL_FIFO_INTRAGGR_COUNT,
		        HWA_RX_D0DEST_INTRAGGR_SEQNUM_CFG_AGGR_COUNT)
		| BCM_SBF(HWA_RXFILL_FIFO_INTRAGGR_TMOUT,
		        HWA_RX_D0DEST_INTRAGGR_SEQNUM_CFG_AGGR_TIMER)
		| 0U);

	// Same reg layout for both S2H FREEIDXSRC and H2S D11BDEST ring interfaces.
	// Using D11BDEST based SHIFT and MASK macros for programming registers

	// FREEIDXSRC and D11BDEST use same SHIFT and MASK values
#if HWA_REVISION_EQ_128
	/* FREEIDXSRC_RING_CFG:template_coherent[bit6] map to
	 * DMA_DESCRIPTOR:FixedBurst[bit16] and FREEIDXSRC_RING_CFG:template_notpcie[bit5]
	 * map to DMA_DESCRIPTOR:Coherent[bit17].
	 * So we still can use FREEIDXSRC_RING_CFG:template_notpcie[bit5] to enable
	 * DMA_DESCRIPTOR:Coherent[bit17]
	 */
	ring_cfg = (0U
		//| BCM_SBF(dev->macif_placement, HWA_RX_D11BDEST_RING_CFG_TEMPLATE_NOTPCIE)
		| BCM_SBF(dev->macif_coherency, HWA_RX_D11BDEST_RING_CFG_TEMPLATE_NOTPCIE)
		| BCM_SBF(0, HWA_RX_D11BDEST_RING_CFG_TEMPLATE_COHERENT)
		| BCM_SBF(0, HWA_RX_D11BDEST_RING_CFG_TEMPLATE_ADDREXT)
		| 0U);
#else /* HWA_REVISION_GE_129 */
	ring_cfg = (0U
		| BCM_SBF(dev->macif_placement, HWA_RX_D11BDEST_RING_CFG_TEMPLATE_NOTPCIE)
		| BCM_SBF(dev->macif_coherency, HWA_RX_D11BDEST_RING_CFG_TEMPLATE_COHERENT)
		| BCM_SBF(0, HWA_RX_D11BDEST_RING_CFG_TEMPLATE_ADDREXT)
		| 0U);
#endif /* HWA_REVISION_GE_129 */

	// ring_intraggr configuration is applied FREEIDXSRC, D11BDEST,

	// FREEIDXSRC and D11BDEST use same SHIFT and MASK values
	ring_intraggr = (0U
		| BCM_SBF(HWA_RXFILL_RING_INTRAGGR_COUNT,
		        HWA_RX_D11BDEST_INTRAGGR_SEQNUM_CFG_AGGR_COUNT)
		| BCM_SBF(HWA_RXFILL_RING_INTRAGGR_TMOUT,
		        HWA_RX_D11BDEST_INTRAGGR_SEQNUM_CFG_AGGR_TIMER)
		| 0U);

	// Configure HWA1b "FREEIDXSRC" and "D11BDEST" interfaces for inited cores
	for (core = 0; core < HWA_RX_CORES; core++)
	{
		if (rxfill->inited[core][0] == FALSE) {
			HWA_ASSERT(dev->driver_mode == HWA_NIC_MODE);
			continue;
		}

		// Same register layout for both "D0DEST" and "D1DEST"
		// Using D0DEST SHIFT and MASK macros for depth and coherency settings
#if HWA_REVISION_EQ_128
		/* D0DEST_RING_CFG:template_coherent[bit6] map to
		 * DMA_DESCRIPTOR:FixedBurst[bit16] so we don't have solution
		 * to control it. A0 ECO tie it low.
		 */
		fifo_cfg = (
			BCM_SBF(0,
				HWA_RX_D0DEST_RING_CFG_TEMPLATE_COHERENT) |
			BCM_SBF(rxfill->fifo_depth[core][0], HWA_RX_D0DEST_RING_CFG_DEPTH));
#else /* HWA_REVISION_GE_129 */
		fifo_cfg = (
			BCM_SBF(dev->macif_coherency,
				HWA_RX_D0DEST_RING_CFG_TEMPLATE_COHERENT) |
			BCM_SBF(rxfill->fifo_depth[core][0], HWA_RX_D0DEST_RING_CFG_DEPTH));
#endif /* HWA_REVISION_GE_129 */

		// Configure HWA MAC FIFO0
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d0dest_ring_addr_lo,
			rxfill->fifo_addr[core][0].loaddr);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d0dest_ring_addr_hi,
			rxfill->fifo_addr[core][0].hiaddr);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d0dest_ring_cfg, fifo_cfg);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d0dest_intraggr_seqnum_cfg,
			fifo_intraggr);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d0dest_wrindexupd_addrlo,
			rxfill->dmarcv_ptr[core][0]);

#ifdef BCMPCIEDEV
		// In FD, assert FIFO0 and FIFO1 are both present and depths are equal
		HWA_ASSERT(dev->driver_mode == HWA_FD_MODE);
		HWA_ASSERT(rxfill->inited[core][1] == TRUE);
		HWA_ASSERT(rxfill->fifo_depth[core][1] == rxfill->fifo_depth[core][0]);

		// Configure HWA MAC FIFO1 for FullDongle mode HWA1b RxFill block
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d1dest_ring_addr_lo,
			rxfill->fifo_addr[core][1].loaddr);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d1dest_ring_addr_hi,
			rxfill->fifo_addr[core][1].hiaddr);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d1dest_ring_cfg, fifo_cfg);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d1dest_intraggr_seqnum_cfg,
			fifo_intraggr);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d1dest_wrindexupd_addrlo,
			rxfill->dmarcv_ptr[core][1]);
#endif /* BCMPCIEDEV */

		// Initialize S2H "FREEIDXSRC" interface
		memory = rxfill->rxfree_ring[core].memory;
		u32 = HWA_PTR2UINT(memory);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core],
		                freeidxsrc_ring_addr_lo, u32);
		u32 = HWA_PTR2HIADDR(memory); // hiaddr for NIC mode 64b hosts
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core],
		                freeidxsrc_ring_addr_hi, u32);
		u32 = (ring_cfg |
			BCM_SBF(HWA_RXFILL_RXFREE_DEPTH, HWA_RX_FREEIDXSRC_RING_CFG_DEPTH));
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], freeidxsrc_ring_cfg, u32);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core],
		                freeidxsrc_intraggr_seqnum_cfg, ring_intraggr);

		// Initialize H2S "D11BDEST" interface
		memory = rxfill->rxfifo_ring[core].memory;
		u32 = HWA_PTR2UINT(memory);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d11bdest_ring_addr_lo, u32);
		u32 = HWA_PTR2HIADDR(memory); // hiaddr for NIC mode 64b hosts
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d11bdest_ring_addr_hi, u32);
		u32 = (ring_cfg |
			BCM_SBF(rxfill->fifo_depth[core][0], HWA_RX_D11BDEST_RING_CFG_DEPTH) |
			BCM_SBIT(HWA_RX_D11BDEST_RING_CFG_INDEX_AFTER_MAC));
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], d11bdest_ring_cfg, u32);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core],
		                d11bdest_intraggr_seqnum_cfg, ring_intraggr);

		// HWA1b RxFill bypass is disabled by default ... unless HWA1b is broken
		u32 = BCM_SBF(0, HWA_RX_RXPMGR_CFG_BYPASS);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxpmgr_cfg, u32);

		// Setup the minimum number of descriptors threshold for FIFO refilling
		u32 = BCM_SBF(HWA_RXFILL_FIFO_MIN_THRESHOLD,
		              HWA_RX_MAC_COUNTER_CTRL_POSTCNT);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], mac_counter_ctrl, u32);

		// Setup SW alert threshold and duration for FIFO starvation
		u32 = (0U
			| BCM_SBF(HWA_RXFILL_FIFO_ALERT_THRESHOLD,
			        HWA_RX_FW_ALERT_CFG_ALERT_THRESH)
			| BCM_SBF(HWA_RXFILL_FIFO_ALERT_DURATION,
			        HWA_RX_FW_ALERT_CFG_ALERT_TIMER)
			| 0U);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], fw_alert_cfg, u32);

		// fw_rxcompensate ... not required

		// Setup RxFILL Ctrl0
#if HWA_REVISION_EQ_128
		/* RXFILL_CTRL0:template_coherent[bit4] map to
		 * DMA_DESCRIPTOR:FixedBurst[bit16] and RXFILL_CTRL0:template_notpcie[bit3]
		 * map to DMA_DESCRIPTOR:Coherent[bit17].
		 * So we still can use RXFILL_CTRL0:template_notpcie[bit3] to enable
		 * DMA_DESCRIPTOR:Coherent[bit17]
		 */
		u32 = (0U
			| BCM_SBIT(HWA_RX_RXFILL_CTRL0_USE_CORE0_FIH)
			// | BCM_SBIT(HWA_RX_RXFILL_CTRL0_RPH_COMPRESS_ENABLE) NA in HWA2.0
			| BCM_SBIT(HWA_RX_RXFILL_CTRL0_WAIT_FOR_D11B_DONE)
			// RX Descr1 buffers based on MAC IF placement in dongle SysMem
			| BCM_SBF(dev->macif_coherency,
			          HWA_RX_RXFILL_CTRL0_TEMPLATE_NOTPCIE)
			| BCM_SBF(0,
			          HWA_RX_RXFILL_CTRL0_TEMPLATE_COHERENT)
			| BCM_SBF(0, HWA_RX_RXFILL_CTRL0_TEMPLATE_ADDREXT)
			// Core1 shares RXP from Core0
			| BCM_SBIT(HWA_RX_RXFILL_CTRL0_USE_CORE0_RXP)
			| BCM_SBF(rxfill->config.rph_size, HWA_RX_RXFILL_CTRL0_RPHSIZE)
			// NA in HWA2.0
			// | BCM_SBF(rxfill->config.len_offset,
			//          HWA_RX_RXFILL_CTRL0_LEN_OFFSET_IN_RPH)
			| BCM_SBF(rxfill->config.addr_offset,
				HWA_RX_RXFILL_CTRL0_ADDR_OFFSET_IN_RPH)
			| 0U);
#else /* HWA_REVISION_GE_129 */
		u32 = (0U
			| BCM_SBIT(HWA_RX_RXFILL_CTRL0_USE_CORE0_FIH)
			// | BCM_SBIT(HWA_RX_RXFILL_CTRL0_RPH_COMPRESS_ENABLE) NA in HWA2.0
			| BCM_SBIT(HWA_RX_RXFILL_CTRL0_WAIT_FOR_D11B_DONE)
			// RX Descr1 buffers based on MAC IF placement in dongle SysMem
			| BCM_SBF(dev->macif_placement,
			          HWA_RX_RXFILL_CTRL0_TEMPLATE_NOTPCIE)
			| BCM_SBF(dev->macif_coherency,
			          HWA_RX_RXFILL_CTRL0_TEMPLATE_COHERENT)
			| BCM_SBF(0, HWA_RX_RXFILL_CTRL0_TEMPLATE_ADDREXT)
			// Core1 shares RXP from Core0
			| BCM_SBIT(HWA_RX_RXFILL_CTRL0_USE_CORE0_RXP)
			| BCM_SBF(rxfill->config.rph_size, HWA_RX_RXFILL_CTRL0_RPHSIZE)
			// NA in HWA2.0
			// | BCM_SBF(rxfill->config.len_offset,
			//          HWA_RX_RXFILL_CTRL0_LEN_OFFSET_IN_RPH)
			| BCM_SBF(rxfill->config.addr_offset,
				HWA_RX_RXFILL_CTRL0_ADDR_OFFSET_IN_RPH)
			| 0U);
#endif /* HWA_REVISION_GE_129 */

		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_ctrl0, u32);

		// Setup RxFILL Ctrl1
		{
			uint32 d11b_offset = rxfill->config.wrxh_offset +
				rxfill->config.d11_offset;
			uint32 d11b_length = HWA_RXBUFFER_BYTES - d11b_offset;

			u32 = (0U
				| BCM_SBF(d11b_length, HWA_RX_RXFILL_CTRL1_D1_LEN)
				| BCM_SBF(d11b_offset, HWA_RX_RXFILL_CTRL1_D1_OFFSET)
				| BCM_SBF(HWA_RXFILL_MIN_FETCH_THRESH_RXP,
					HWA_RX_RXFILL_CTRL1_MIN_FETCH_THRESH_RXP)
				| BCM_SBF(HWA_RXFILL_MIN_FETCH_THRESH_FREEIDX,
					HWA_RX_RXFILL_CTRL1_MIN_FETCH_THRESH_FREEIDX)
				| BCM_SBF(dev->macif_coherency,
					HWA_RX_RXFILL_CTRL1_TEMPLATE_COHERENT_NONDMA)
				| 0U);
			HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_ctrl1, u32);
		}

		// Setup RxPost to RPH Compression
		u32 = 0U; // No compression in HWA-2.0
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_compresslo, u32);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_compresshi, u32);

		// Reference sbhnddma.h descriptor control flags 1
		// NotPcie bit#18: This field must be zero for receive descriptors
#if defined(BCMPCIEDEV) /* FD mode SW driver */
		u32 = (0U
			// bits 15:00 - reserved, will cause protocol error if not set to 0
			// | D64_CTRL1_FIXEDBURST
			| D64_CTRL1_COHERENT
			// | D64_CTRL1_NOTPCIE
			// | D64_CTRL1_DS
			//   bits 27:20 - core specific flags
			//   bit  28    - D64_CTRL1_EOT will be set by HWA1b
			//   bit  29    - D64_CTRL1_IOC will be set by HWA1b
			//   bit  30    - D64_CTRL1_EOF will be set by HWA1b
			//   bit  31    - D64_CTRL1_SOF will be set by HWA1b
			| 0U);
#else /* !BCMPCIEDEV */
		u32 = 0U;
		if (dev->macif_placement || dev->host_coherency)
			u32 |= D64_CTRL1_COHERENT;
#endif /* !BCMPCIEDEV */
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_desc0_templ_lo, u32);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_desc1_templ_lo, u32);

		// Reference sbhnddma.h descriptor control flags 2
		u32 = (0U
			// bits 47:32 - buffer byte count will be set by HWA1b
			// bits 49:48 - address extension is 0
			// bit  50    - D64_CTRL2_PARITY parity always calculated internally
			// bits 63:51 - reserved, will cause protocol error if not set to 0
			| 0U);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_desc0_templ_hi, u32);
		HWA_WR_REG_NAME(HWA1b, regs, rx_core[core], rxfill_desc1_templ_hi, u32);

	} // for core

	// Enable Req-Ack based interface between MAC-HWA on rx DMA is enabled.
	u16 = HWA_RD_REG16_ADDR(HWA1b, &dev->mac_regs->hwa_macif_ctl);
	u16 |= BCM_SBIT(_HWA_MACIF_CTL_RXDMAEN);
	HWA_WR_REG16_ADDR(HWA1b, &dev->mac_regs->hwa_macif_ctl, u16);

	return HWA_SUCCESS;

} // hwa_rxfill_init

int // HWA1b RxFill: Deinit RxFill interfaces
hwa_rxfill_deinit(hwa_rxfill_t *rxfill)
{
	hwa_dev_t *dev;
	uint16 u16;

	// Setup locals
	dev = HWA_DEV(rxfill);

	// Disable Req-Ack based interface between MAC-HWA on rx DMA.
	u16 = HWA_RD_REG16_ADDR(HWA1b, &dev->mac_regs->hwa_macif_ctl);
	u16 = BCM_CBIT(u16, _HWA_MACIF_CTL_RXDMAEN);
	HWA_WR_REG16_ADDR(HWA1b, &dev->mac_regs->hwa_macif_ctl, u16);

	return HWA_SUCCESS;
}

int // Handle a Free RxBuffer request from WLAN driver, returns success|failure
hwa_rxfill_rxbuffer_free(struct hwa_dev *dev, uint32 core,
	hwa_rxbuffer_t *rx_buffer, bool has_rph)
{
	hwa_rxfill_t *rxfill;
	hwa_ring_t *rxfree_ring; // S2H FREEIDXSRC interface
	hwa_rxfill_rxfree_t *rxfree;
	HWA_DEBUG_EXPR(uint32 offset);

	rxfill = &dev->rxfill;

	HWA_TRACE(("%s free core<%u> rxbuffer<%p> has_rph<%u>\n",
		HWA1b, core, rx_buffer, has_rph));

	// Audit parameters and pre-conditions
	HWA_ASSERT(rx_buffer != (hwa_rxbuffer_t*)NULL);
	HWA_ASSERT(core < HWA_RX_CORES);

	// Audit rx_buffer, wrong rx_buffer causes 1b stuck.
	HWA_ASSERT(rx_buffer >= (hwa_rxbuffer_t*)dev->rx_bm.memory);
	HWA_ASSERT(rx_buffer <= ((hwa_rxbuffer_t*)dev->rx_bm.memory) + (dev->rx_bm.pkt_total - 1));
	HWA_DEBUG_EXPR({
		offset = ((char *)rx_buffer) - ((char *)dev->rx_bm.memory);
		HWA_ASSERT((offset % dev->rx_bm.pkt_size) == 0);
	});

	// Setup locals
	rxfree_ring = &rxfill->rxfree_ring[core];

	if (hwa_ring_is_full(rxfree_ring)) {
		HWA_ASSERT(1);
		goto failure;
	}

	// Find the location where rxfree needs to be constructed, and populate it
	rxfree = HWA_RING_PROD_ELEM(hwa_rxfill_rxfree_t, rxfree_ring);

	// Convert rxbuffer pointer to its index within Rx Buffer Manager
	rxfree->index = HWA_TABLE_INDX(hwa_rxbuffer_t,
	                               dev->rx_bm.memory, rx_buffer);

	rxfree->control_info = (has_rph == TRUE) ?
	                        HWA_RXFILL_RXFREE_PAIRED : HWA_RXFILL_RXFREE_SIMPLE;

#ifdef HWA_RXFILL_RXFREE_AUDIT_ENABLED
	_hwa_rxfill_rxfree_audit(rxfill, rxfree->index, FALSE);
#endif // endif

	hwa_ring_prod_upd(rxfree_ring, 1, TRUE); // update/commit WR

	HWA_STATS_EXPR(rxfill->rxfree_cnt[core]++);

	HWA_TRACE(("%s RXF[%u,%u][index<%u> ctrl<%u>]\n", HWA1b,
		HWA_RING_STATE(rxfree_ring)->write, HWA_RING_STATE(rxfree_ring)->read,
		rxfree->index, rxfree->control_info));

	return HWA_SUCCESS;

failure:
	HWA_WARN(("%s rxbuffer free <0x%p> failure\n", HWA1b, rx_buffer));

	return HWA_FAILURE;

} // hwa_rxfill_rxbuffer_free

void // Reclaim all RxBuffers in RxBM
hwa_rxfill_rxbuffer_reclaim(hwa_dev_t *dev, uint32 core)
{
	uintptr rxbm_base; // loaddr of Rx Buffer Manager
	hwa_rxfill_t *rxfill; // SW rxfill state
	hwa_ring_t *h2s_ring; // H2S D11BDEST RxFIFO ring
	hwa_ring_t *s2h_ring; // S2H FREEIDXSRC RxFREE ring
	hwa_rxbuffer_t *rxbuffer; // pointer to RxBuffer
	hwa_rxfill_rxfifo_t *rxfifo; // element in H2S D11BDEST RxFIFO ring
	hwa_rxpost_hostinfo_t *rph_req;
	uint32 rxfifo_cnt; // total rxbuffers processed
	int wr_idx, rd_idx;
	uint16 depth;

	HWA_FTRACE(HWA1b);

	// Audit parameters and pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_RX_CORES);

	// Setup locals
	rxfill = &dev->rxfill;
	rxbm_base = rxfill->rxbm_base;
	h2s_ring = &rxfill->rxfifo_ring[core];
	s2h_ring = &rxfill->rxfree_ring[core];

	hwa_ring_cons_get(h2s_ring); // fetch HWA rxfifo ring's WR index once

	rxfifo_cnt = hwa_rxfill_fifo_avail(rxfill, core);
	depth = h2s_ring->depth;
	wr_idx = (HWA_RING_STATE(h2s_ring)->write + rxfifo_cnt) % depth;
	rd_idx = HWA_RING_STATE(h2s_ring)->read;

	HWA_PRINT("%s reclaim core<%u> %u rxbuffers RD<%u> WR<%u>\n",
		HWA1b, core, rxfifo_cnt, rd_idx, wr_idx);

	while (rd_idx != wr_idx) {
		// Fetch location of packet in rxfifo to process
		rxfifo = HWA_RING_ELEM(hwa_rxfill_rxfifo_t, h2s_ring, rd_idx);

		// Get RxBuffer from RxBM
		rxbuffer = HWA_TABLE_ELEM(hwa_rxbuffer_t, rxbm_base, rxfifo->index);

		rph_req = (hwa_rxpost_hostinfo_t *)rxbuffer;

#ifdef BCMDBG
		HWA_DEBUG_EXPR({
		// Show RPH value,
		if (dev->host_addressing & HWA_32BIT_ADDRESSING) { // 32bit host
			HWA_PRINT("%s rph32 alloc pktid<%u> haddr<0x%08x:0x%08x>\n", HWA1x,
				rph_req->hostinfo32.host_pktid, dev->host_physaddrhi,
				rph_req->hostinfo32.data_buf_haddr32);
		} else { // 64bit host
			HWA_PRINT("%s rph64 alloc pktid<%u> haddr<0x%08x:0x%08x>\n", HWA1x,
				rph_req->hostinfo64.host_pktid,
				rph_req->hostinfo64.data_buf_haddr64.hiaddr,
				rph_req->hostinfo64.data_buf_haddr64.loaddr);
		}});
#endif /* BCMDBG */

		hwa_rxpath_queue_rxcomplete_fast(dev, rph_req->hostinfo64.host_pktid);

		rd_idx = (rd_idx + 1) % depth;
	}

	ASSERT(rd_idx == wr_idx);

	hwa_rxpath_xmit_rxcomplete_fast(dev);

	// Reset RxFIFO ring and RxFREE ring
	HWA_RING_STATE(h2s_ring)->read = 0;
	HWA_RING_STATE(h2s_ring)->write = 0;
	HWA_RING_STATE(s2h_ring)->read = 0;
	HWA_RING_STATE(s2h_ring)->write = 0;

}

int // Process H2S RxFIFO interface WR index update interrupt from MAC
hwa_rxfill_rxbuffer_process(hwa_dev_t *dev, uint32 core, bool bound)
{
	uint32 elem_ix; // location of next element to read
	uintptr rxbm_base; // loaddr of Rx Buffer Manager
	hwa_rxfill_t *rxfill; // SW rxfill state
	hwa_ring_t *h2s_ring; // H2S D11BDEST RxFIFO ring
	hwa_rxbuffer_t *rxbuffer; // pointer to RxBuffer
	hwa_rxfill_rxfifo_t *rxfifo; // element in H2S D11BDEST RxFIFO ring
	hwa_handler_t *rxfifo_recv_handler; // upstream rx buffer processing
	hwa_handler_t *rxfifo_done_handler; // upstream rx fifo done processing
	uint32 rxfifo_cnt; // total rxbuffers processed
	int ret, elem_ix_pend; // location of next pend element to read
	wlc_info_t *wlc;	// wlc pointer

	HWA_FTRACE(HWA1b);

	// Audit parameters and pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_RX_CORES);

	// Setup locals
	rxfill = &dev->rxfill;
	rxbm_base = rxfill->rxbm_base;
	wlc = (wlc_info_t *)rxfill->wlc[core];

	// Read and save TSF register once
	rxfill->tsf_l = R_REG(wlc->osh, D11_TSFTimerLow(wlc));

	// Fetch registered upstream callback handlers
	rxfifo_recv_handler = &dev->handlers[HWA_RXFIFO_RECV];
	rxfifo_done_handler = &dev->handlers[HWA_RXFIFO_DONE];

	h2s_ring = &rxfill->rxfifo_ring[core];

	hwa_ring_cons_get(h2s_ring); // fetch HWA rxfifo ring's WR index once
	rxfifo_cnt = 0U;

	// Consume all packets to be received in FIFO, sending them upstream
	// Use hwa_ring_cons_pend, because rxfifo_recv_handler could return error.
	while ((elem_ix = hwa_ring_cons_pend(h2s_ring, &elem_ix_pend)) != BCM_RING_EMPTY) {

		// Fetch location of next packet in rxfifo to process
		rxfifo = HWA_RING_ELEM(hwa_rxfill_rxfifo_t, h2s_ring, elem_ix);

		// Send RxBuffer to upstream handler
		rxbuffer = HWA_TABLE_ELEM(hwa_rxbuffer_t, rxbm_base, rxfifo->index);

		HWA_TRACE(("%s elem_ix<%u> recv core<%u> rxbuffer<%p>\n",
			HWA1b, elem_ix, core, rxbuffer));

		ret = (*rxfifo_recv_handler->callback)(rxfifo_recv_handler->context,
			(uintptr)rxfill->wlc[core], (uintptr)rxbuffer, core, rxfifo->index);

		// Callback cannot handle it, don't update ring read and break the loop.
		if (ret != HWA_SUCCESS)
			break;

		// Commit a previously pending read
		hwa_ring_cons_done(h2s_ring, elem_ix_pend);

		rxfifo_cnt++;

		if ((rxfifo_cnt % HWA_RXFILL_LAZY_RD_UPDATE) == 0) {
			if (bound) {
				break;
			} else {
				hwa_ring_cons_put(h2s_ring); // commit RD index lazily
			}
		}
	}

	hwa_ring_cons_put(h2s_ring); // commit RD index now

	// Done processing all rx packets in fifo
	(*rxfifo_done_handler->callback)(rxfifo_done_handler->context,
		(uintptr)rxfill->wlc[core], (uintptr)0, core, rxfifo_cnt);

	HWA_STATS_EXPR(rxfill->rxfifo_cnt[core] += rxfifo_cnt);

	if (!hwa_ring_is_empty(h2s_ring)) {
		/* need re-schdeule */
		if (core == 0)
			dev->intstatus |= HWA_COMMON_INTSTATUS_D11BDEST0_INT_MASK;
		else
			dev->intstatus |= HWA_COMMON_INTSTATUS_D11BDEST1_INT_MASK;
	}

	return HWA_SUCCESS;

} // hwa_rxfill_rxbuffer_process

uint32 // Return the number of available RxBuffers for reception in the RX FIFOs
hwa_rxfill_fifo_avail(hwa_rxfill_t *rxfill, uint32 core)
{
	uint32 u32;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA1b);

	// Audit parameters and pre-conditions
	dev = HWA_DEV(rxfill);
	HWA_ASSERT(core < HWA_RX_CORES);

	u32 = HWA_RD_REG_NAME(HWA1b, dev->regs, rx_core[core], mac_counter_status);

	HWA_PRINT("%s mac_counter_status sat<%u> need_post<%u> aval<%u>\n",
		HWA1b, BCM_GBF(u32, HWA_RX_MAC_COUNTER_STATUS_SATURATED),
		BCM_GBF(u32, HWA_RX_MAC_COUNTER_STATUS_NEED_POST),
		BCM_GBF(u32, HWA_RX_MAC_COUNTER_STATUS_CTR_VAL));

	u32 = BCM_GBF(u32, HWA_RX_MAC_COUNTER_STATUS_CTR_VAL);

	return u32;

} // hwa_rxfill_fifo_avail

#if defined(BCMDBG)

void // Debug support for HWA1b RxFill block
hwa_rxfill_dump(hwa_rxfill_t *rxfill, struct bcmstrbuf *b, bool verbose)
{
	uint32 core, fifo;

	HWA_BPRINT(b, "%s dump<%p>\n", HWA1b, rxfill);

	if (rxfill == (hwa_rxfill_t*)NULL)
		return;

	HWA_BPRINT(b, "+ Config: rph_sz<%u> offset align<%u> d11<%u> len<%u> addr<%u>\n",
		rxfill->config.rph_size,
		(rxfill->config.wrxh_offset - rxfill->config.rph_size),
		rxfill->config.d11_offset, rxfill->config.len_offset,
		rxfill->config.addr_offset);

	for (core = 0; core < HWA_RX_CORES; core++) { // per inited core

		if (rxfill->inited[core][0] == FALSE)
			continue;

		HWA_BPRINT(b, "+ core%u wlc<0x%p>\n", core, rxfill->wlc[core]);

		HWA_STATS_EXPR(
			HWA_BPRINT(b, "+ rxfree_cnt<%u> rxfifo_cnt<%u>\n",
				rxfill->rxfree_cnt[core], rxfill->rxfifo_cnt[core]));

		hwa_ring_dump(&rxfill->rxfree_ring[core], b, "+ rxfree_ring");
		hwa_ring_dump(&rxfill->rxfifo_ring[core], b, "+ rxfifo_ring");

		for (fifo = 0; fifo < HWA_RXFIFO_MAX; fifo++) { // per inited fifo
			if (rxfill->inited[core][fifo] == TRUE) {
				HWA_BPRINT(b, "+ core%u fifo%u<0x%08x,0x%08x>"
					" depth<%u> dmarcv_ptr<0x%08x>\n",
					core, fifo,
					rxfill->fifo_addr[core][fifo].loaddr,
					rxfill->fifo_addr[core][fifo].hiaddr,
					rxfill->fifo_depth[core][fifo],
					rxfill->dmarcv_ptr[core][fifo]);
			}
		} // for fifo

	} // for core

} // hwa_rxfill_dump

#if defined(WLTEST)

// Debug dump of various Transfer Status, using RXPMGR TRFSTATUS layout
#define HWA_RXFILL_TFRSTATUS_DECLARE(mgr) \
void hwa_rxfill_##mgr##_tfrstatus(hwa_rxfill_t *rxfill, uint32 core) { \
	hwa_dev_t *dev = HWA_DEV(rxfill); \
	uint32 u32; \
	u32 = HWA_RD_REG_NAME(HWA1b, dev->regs, rx_core[core], mgr##_tfrstatus); \
	HWA_PRINT("%s core<%u> %s_tfrstatus<%u> sz<%u> occup<%u> avail<%u>\n", \
		HWA1b, core, #mgr, \
		BCM_GBF(u32, HWA_RX_RXPMGR_TFRSTATUS_STATE), \
		BCM_GBF(u32, HWA_RX_RXPMGR_TFRSTATUS_TRANSFER_SIZE), \
		BCM_GBF(u32, HWA_RX_RXPMGR_TFRSTATUS_SRC_OCCUPIED_EL), \
		BCM_GBF(u32, HWA_RX_RXPMGR_TFRSTATUS_DEST_SPACE_AVAIL_EL)); \
}
HWA_RXFILL_TFRSTATUS_DECLARE(rxpmgr);     // hwa_rxfill_rxpmgr_tfrstatus
HWA_RXFILL_TFRSTATUS_DECLARE(d0mgr);      // hwa_rxfill_d0mgr_tfrstatus
HWA_RXFILL_TFRSTATUS_DECLARE(d1mgr);      // hwa_rxfill_d1mgr_tfrstatus
HWA_RXFILL_TFRSTATUS_DECLARE(d11bmgr);    // hwa_rxfill_d11bmgr_tfrstatus
HWA_RXFILL_TFRSTATUS_DECLARE(freeidxmgr); // hwa_rxfill_freeidxmgr_tfrstatus

// Debug dump of various localfifo configuration, using RXP LOCALFIFO CFG STATUS
#define HWA_RXFILL_LOCALFIFO_STATUS_DECLARE(mgr) \
void hwa_rxfill_##mgr##_localfifo_status(hwa_rxfill_t *rxfill, uint32 core) { \
	hwa_dev_t *dev = HWA_DEV(rxfill); \
	uint32 u32; \
	u32 = HWA_RD_REG_NAME(HWA1b, dev->regs, rx_core[core], \
	          mgr##_localfifo_cfg_status); \
	HWA_PRINT("%s core<%u> %s_localfifo_cfg_status " \
		"max_items<%u> wrptr<%u> rdptr<%u> notempty<%u> isfull<%u>\n", \
		HWA1b, core, #mgr, \
		BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_MAX_ITEMS), \
		BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_WRPTR), \
		BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_RDPTR), \
		BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_NOTEMPTY), \
		BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_FULL)); \
}
HWA_RXFILL_LOCALFIFO_STATUS_DECLARE(rxp);  // hwa_rxfill_rxp_localfifo_status
HWA_RXFILL_LOCALFIFO_STATUS_DECLARE(d0);   // hwa_rxfill_d0_localfifo_status
HWA_RXFILL_LOCALFIFO_STATUS_DECLARE(d1);   // hwa_rxfill_d1_localfifo_status
HWA_RXFILL_LOCALFIFO_STATUS_DECLARE(d11b); // hwa_rxfill_d11b_localfifo_status
HWA_RXFILL_LOCALFIFO_STATUS_DECLARE(freeidx);

void // Dump the HWA1b RxFill status
hwa_rxfill_status(hwa_rxfill_t *rxfill, uint32 core)
{
	uint32 u32;
	hwa_dev_t *dev;

	dev = HWA_DEV(rxfill);

	u32 = HWA_RD_REG_NAME(HWA1b, dev->regs, rx_core[core], rxfill_status0);

	HWA_PRINT("%s status<%u> fih_buf_valid<%u> "
		"rxpf[empty<%u> mac<%u>] full[d0<%u> d1<%u> d11b<%u>] d11b_avail<%u> "
		"buf_index<%u>\n",
		HWA1b, BCM_GBF(u32, HWA_RX_RXFILL_STATUS0_STATE),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_FIH_BUF_VALID),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_RXPF_EMPTY),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_RXPF_EMPTY_FOR_MAC),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_D0_FULL),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_D1_FULL),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_D11B_FULL),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_POPPED_BUF_AVAIL),
		BCM_GBIT(u32, HWA_RX_RXFILL_STATUS0_BUF_INDEX));

} // hwa_rxfill_status

#endif // endif

#endif /* BCMDBG */

static int
hwa_rxfill_bmac_recv(void *context, uintptr arg1, uintptr arg2, uint32 core, uint32 arg4)
{
	hwa_dev_t *dev = (hwa_dev_t *)context;
	uchar *rxbuffer = (uchar *)arg2;
	hwa_rxfill_t *rxfill;
	hwa_rxpost_hostinfo_t *rph_req;
	wlc_info_t *wlc = (wlc_info_t *)arg1;
	struct lbuf_frag *frag;
	wlc_d11rxhdr_t *wrxh;
	d11rxhdr_t *rxh;
	uint16 rx_size;

	HWA_FTRACE(HWA2a);
	HWA_ASSERT(context != (void *)NULL);
	HWA_ASSERT(arg1 != 0);
	HWA_ASSERT(arg2 != 0);

	rxfill = &dev->rxfill;

	//prhex("hwa_rxfill_bmac_recv: rxbuffer Raw Data", (uchar *)rxbuffer, HWA_RXBUFFER_BYTES);

	rph_req = (hwa_rxpost_hostinfo_t *)rxbuffer;
	HWA_DEBUG_EXPR({
	// Show RPH value,
	if (dev->host_addressing & HWA_32BIT_ADDRESSING) { // 32bit host
		HWA_PRINT("%s rph32 alloc pktid<%u> haddr<0x%08x:0x%08x>\n", HWA1x,
			rph_req->hostinfo32.host_pktid, dev->host_physaddrhi,
			rph_req->hostinfo32.data_buf_haddr32);
	} else { // 64bit host
		HWA_PRINT("%s rph64 alloc pktid<%u> haddr<0x%08x:0x%08x>\n", HWA1x,
			rph_req->hostinfo64.host_pktid,
			rph_req->hostinfo64.data_buf_haddr64.hiaddr,
			rph_req->hostinfo64.data_buf_haddr64.loaddr);
	}});

	// Get one lbuf_frag from rxfrag pool.
	frag = (struct lbuf_frag *)pktpool_get_ext(SHARED_RXFRAG_POOL, lbuf_basic, NULL);
	if (frag == NULL)
		return HWA_FAILURE;

	/* Set up lbuf frag */
	/* Set this packet as HWA packet in first place */
	PKTSETHWAPKT(dev->osh, frag);
	PKTSETRXFRAG(dev->osh, frag);

	/* Load 64 bit host address */
	PKTSETFRAGDATA_HI(dev->osh, frag, 1, rph_req->hostinfo64.data_buf_haddr64.hiaddr);
	PKTSETFRAGDATA_LO(dev->osh, frag, 1, rph_req->hostinfo64.data_buf_haddr64.loaddr);

	/* frag len */
	PKTSETFRAGLEN(dev->osh, frag, 1, HWARX_DATA_BUF_HLEN(frag));

	// Set host_pktid
	PKTSETFRAGPKTID(dev->osh, frag, rph_req->hostinfo64.host_pktid);

	// Start from RxStatus
	PKTSETBUF(dev->osh, frag, rxbuffer + rxfill->config.wrxh_offset, rxfill->config.rx_size);

	// Save the wrxh_offset value for rxbuffer point retrieving.
	PKTSETHWARXOFFSET(frag, rxfill->config.wrxh_offset);

	// SW RXHDR
	wrxh = (wlc_d11rxhdr_t *)PKTDATA(dev->osh, frag);
	rxh = &wrxh->rxhdr;

	// Dongle part
	rx_size = (uint16)MIN(rxfill->config.rx_size, wlc->hwrxoff +
		D11RXHDR_ACCESS_VAL(rxh, wlc->pub->corerev, RxFrameSize));
	PKTSETLEN(dev->osh, frag, rx_size);

#ifdef BULKRX_PKTLIST
	wlc_bmac_process_split_fifo_pkt(wlc->hw, frag, WLC_RXHDR_LEN);
#endif // endif

	// Fillup Fifo info in rxstatus
	*(D11RXHDR_GE128_ACCESS_REF(rxh, fifo)) = (uint8)RX_FIFO1;

	// Record TSF info inside SW header
	wrxh->tsf_l = rxfill->tsf_l;

	HWA_TRACE(("%s frag<%p> rxbuffer[rph]<%p> data<%p> len <%d> head<%p> end<%p> "
		"wrxh_offset<%d> wlc_d11rxhdr_t<%d> used_len<%d>\n", HWA1x, frag,
		LBHWARXPKT(frag), PKTDATA(dev->osh, frag), PKTLEN(dev->osh, frag),
		PKTHEAD(dev->osh, frag), frag->lbuf.end,
		rxfill->config.wrxh_offset, wlc->hwrxoff,
		PKTFRAGUSEDLEN(dev->osh, frag)));

	// Audit it before pass to WL
#ifdef HWA_RXFILL_RXFREE_AUDIT_ENABLED
	_hwa_rxfill_rxfree_audit(rxfill, arg4, TRUE);
#endif // endif

	if (rxfill->rx_tail == NULL) {
		rxfill->rx_head = rxfill->rx_tail = frag;
	} else {
		PKTSETLINK(rxfill->rx_tail, frag);
		rxfill->rx_tail = frag;
	}

	/* Pkt processing Done */
	return HWA_SUCCESS;
}

static int
hwa_rxfill_bmac_done(void *context, uintptr arg1, uintptr arg2, uint32 core, uint32 rxfifo_cnt)
{
	hwa_dev_t *dev = (hwa_dev_t *)context;
	wlc_info_t *wlc = (wlc_info_t *)arg1;
	hwa_rxfill_t *rxfill;
	void *rx_list, *p;
#ifdef PKTC_DONGLE
	void *pktc_head = NULL;
	void *pktc_tail = NULL;
	uint16 pktc_index = 0;
#endif // endif

	HWA_FTRACE(HWA1b);
	HWA_ASSERT(context != (void *)NULL);
	HWA_ASSERT(arg1 != 0);

	rxfill = &dev->rxfill;

	/* Terminate PKTLINK */
	if (rxfill->rx_tail) {
		PKTSETLINK(rxfill->rx_tail, NULL);
	}

	rx_list = rxfill->rx_head;
#ifdef STS_FIFO_RXEN
	if (STS_RX_ENAB(wlc->pub) && rx_list) {
		void *sts_list = dma_sts_rx(wlc->hw->di[STS_FIFO]);
		wlc_bmac_recv_process_sts(wlc->hw, rx_list, sts_list, WLC_RXHDR_LEN);
	}
#endif /* STS_FIFO_RXEN */
	while (rx_list != NULL) {
		p = rx_list;
		rx_list = PKTLINK(rx_list);
		PKTSETLINK(p, NULL);

#if defined(WLCFP) && defined(WLCFP_RXSM4)
		if (CFP_RCB_ENAB(wlc->cfp)) {
			/* Classify the packets based on per packet info. On the very first
			 * unchained packet, release all chained packets and continue.
			 */
			wlc_cfp_rxframe(wlc, p);
		} else
#endif /* WLCFP && WLCFP_RXSM4 */
		{
			/* Legacy RX processing */
#ifdef PKTC_DONGLE
			if (PKTC_ENAB(wlc->pub) &&
				wlc_rxframe_chainable(wlc, (void **)&p, pktc_index)) {
				if (p != NULL) {
					PKTCENQTAIL(pktc_head, pktc_tail, p);
					pktc_index++;
				}
			} else
#endif /* PKTC_DONGLE */
			{
#ifdef PKTC_DONGLE
				if (pktc_tail) {
					/* pass to WL, wlc_sendup_chain will set up the tsf */
					wlc_sendup_chain(wlc, pktc_head);
					pktc_tail = NULL;
					pktc_index = 0;
				}
#endif /* PKTC_DONGLE */
				/* compute the RSSI from d11rxhdr and record it in wlc_rxd11hr */
				phy_rssi_compute_rssi((phy_info_t *)wlc->hw->band->pi,
					(wlc_d11rxhdr_t *)PKTDATA(dev->osh, p));

				/* Legacy Slow RX path */
				wlc_recv(wlc, p);
			}
		}

	}

	rxfill->rx_head = rxfill->rx_tail = NULL;

#if defined(WLCFP) && defined(WLCFP_RXSM4)
	if (CFP_RCB_ENAB(wlc->cfp)) {
		/* Sendup chained CFP packets */
		wlc_cfp_rx_sendup(wlc, NULL);
	} else
#endif /* WLCFP && WLCFP_RXSM4 */
	{
#if defined(PKTC_DONGLE)
		if (pktc_tail) {
			/* pass to WL, wlc_sendup_chain will set up the tsf */
			wlc_sendup_chain(wlc, pktc_head);
		}
#endif /* PKTC_DONGLE */
	}

	HWA_TRACE(("%s %s(): core<%u> rxfifo_cnt<%u>\n", HWA00, __FUNCTION__, core, rxfifo_cnt));

	return HWA_SUCCESS;
}

#endif /* HWA_RXFILL_BUILD */

#ifdef HWA_MAC_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: Configure HWA Common registers required per MAC Core
 * -----------------------------------------------------------------------------
 */

void // Invoked by MAC to configure the HWA Common block registers
hwa_mac_config(hwa_mac_config_t config,
	uint32 core, volatile void *ptr, uintptr val)
{
	uint32 v32;
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	HWA_TRACE(("%s PHASE MAC config<%u> core<%u> ptr<%p> val<0x%08x,%u>\n",
		HWA00, config, core, ptr, val, val));

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit

	// Audit parameters and pre-conditions
	HWA_ASSERT(core < HWA_RX_CORES);

	regs = dev->regs;

	switch (config) {

		case HWA_HC_HIN_REGS: // carries RxFilterEn and RxHwaCtrl registers
			HWA_ASSERT(ptr != (volatile void*)NULL);
			dev->mac_regs = (volatile hc_hin_regs_t*)ptr;
			break;

		case HWA_MAC_AXI_BASE:
			HWA_ASSERT(val != 0U);
			HWA_RXDATA_EXPR(dev->rxdata.mac_fhr_base = val + MAC_AXI_RXDATA_FHRTABLE);
			HWA_RXDATA_EXPR(dev->rxdata.mac_fhr_stats = val + MAC_AXI_RXDATA_FHRSTATS);
			break;

		case HWA_MAC_BASE_ADDR:
			HWA_ASSERT(val != 0U);
			if (core == 0) {
				HWA_WR_REG_NAME(HWA00, regs, common, mac_base_addr_core0, val);
			} else {
				HWA_WR_REG_NAME(HWA00, regs, common, mac_base_addr_core1, val);
			}
			break;

		case HWA_MAC_FRMTXSTATUS:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, mac_frmtxstatus);
			if (core == 0) {
				v32 = BCM_CBF(v32,
					HWA_COMMON_MAC_FRMTXSTATUS_MAC_FRMTXSTATUS_CORE0);
				v32 |= BCM_SBF(val,
					HWA_COMMON_MAC_FRMTXSTATUS_MAC_FRMTXSTATUS_CORE0);
			} else {
				v32 = BCM_CBF(v32,
					HWA_COMMON_MAC_FRMTXSTATUS_MAC_FRMTXSTATUS_CORE1);
				v32 |= BCM_SBF(val,
					HWA_COMMON_MAC_FRMTXSTATUS_MAC_FRMTXSTATUS_CORE1);
			}
			HWA_WR_REG_NAME(HWA00, regs, common, mac_frmtxstatus, v32);
			break;

		case HWA_MAC_DMA_PTR:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, mac_dma_ptr);
			if (core == 0)
				v32 |= BCM_SBF(val, HWA_COMMON_MAC_DMA_PTR_MAC_DMA_PTR_CORE0);
			else
				v32 |= BCM_SBF(val, HWA_COMMON_MAC_DMA_PTR_MAC_DMA_PTR_CORE1);
			HWA_WR_REG_NAME(HWA00, regs, common, mac_dma_ptr, v32);
			break;

		case HWA_MAC_IND_XMTPTR:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, mac_ind_xmtptr);
			if (core == 0) {
				v32 = BCM_CBF(v32, HWA_COMMON_MAC_IND_XMTPTR_MAC_XMT_PTR_CORE0);
				v32 |= BCM_SBF(val, HWA_COMMON_MAC_IND_XMTPTR_MAC_XMT_PTR_CORE0);
			}
			else {
				v32 = BCM_CBF(v32, HWA_COMMON_MAC_IND_XMTPTR_MAC_XMT_PTR_CORE1);
				v32 |= BCM_SBF(val, HWA_COMMON_MAC_IND_XMTPTR_MAC_XMT_PTR_CORE1);
			}
			HWA_WR_REG_NAME(HWA00, regs, common, mac_ind_xmtptr, v32);
			break;

		case HWA_MAC_IND_QSEL:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, mac_ind_qsel);
			if (core == 0) {
				v32 = BCM_CBF(v32, HWA_COMMON_MAC_IND_QSEL_MAC_INDQSEL_CORE0);
				v32 |= BCM_SBF(val, HWA_COMMON_MAC_IND_QSEL_MAC_INDQSEL_CORE0);
			}
			else {
				v32 = BCM_CBF(v32, HWA_COMMON_MAC_IND_QSEL_MAC_INDQSEL_CORE1);
				v32 |= BCM_SBF(val, HWA_COMMON_MAC_IND_QSEL_MAC_INDQSEL_CORE1);
			}
			HWA_WR_REG_NAME(HWA00, regs, common, mac_ind_qsel, v32);
			break;

		default:
			HWA_ERROR(("%s mac config<%u> invalid\n", HWA00, config));
			break;

	} // switch

} // hwa_mac_config

#endif /* HWA_MAC_BUILD */

#ifdef HWA_RXDATA_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: HWA2a RxData block bringup: attach, detach, and init phases
 * -----------------------------------------------------------------------------
 */

#ifdef HWA_RXDATA_FHR_IND_BUILD
// FHR Register file and FHR counter statistics. Then remove this function.

static void // Use Indirect Access to configure FHR register file
hwa_rxdata_fhr_indirect_write(hwa_rxdata_t *rxdata,
	hwa_rxdata_fhr_entry_t *filter)
{
	int i, j;
	uint32 v32;
	hwa_dev_t *dev;
	volatile hc_hin_regs_t *mac_regs;
	hwa_rxdata_fhr_param_t *param;

	HWA_TRACE(("%s FHR write filter<%u>\n", HWA2a, filter->config.id));

	// Audit pre-conditions
	dev = HWA_DEV(rxdata);

	HWA_ASSERT(dev->mac_regs != (hc_hin_regs_t*)NULL);

	// Setup locals
	mac_regs = dev->mac_regs;

	// Select FHR
	v32 = (0U
		| BCM_SBF(MAC_AXI_RXDATA_FHR_RF(filter->config.id), _OBJADDR_INDEX)
		| BCM_SBF(MAC_AXI_RXDATA_FHR_SELECT, _OBJADDR_SELECT)
		| BCM_SBIT(_OBJADDR_WRITEINC)
		| 0U);
	HWA_WR_REG_ADDR(HWA2a, &mac_regs->objaddr, v32);
	v32 = HWA_RD_REG_ADDR(HWA2a, &mac_regs->objaddr); // ensure WR completes

	// Write-out the entire filter using WR address auto increment and ObjData
	v32 = filter->u32[0]; // hwa_rxdata_fhr_entry_t::config
	HWA_WR_REG_ADDR(HWA2a, &mac_regs->objdata, v32);

	for (i = 0; i < HWA_RXDATA_FHR_PARAMS_MAX; i++) {
		param = &filter->params[i];
		for (j = 0; j < HWA_RXDATA_FHR_PARAM_WORDS; j++) { // 5 Words
			v32 = param->u32[j];
			HWA_WR_REG_ADDR(HWA2a, &mac_regs->objdata, v32);
		} // for params: config, bitmask[0..31, 32..63] pattern[0..31, 32..63]
	} // for param_count

} // hwa_rxdata_fhr_indirect_write

#endif /* HWA_RXDATA_FHR_IND_BUILD */

void // HWA2a RxData: Cleanup/Free resources used by RxData block
BCMATTACHFN(hwa_rxdata_detach)(hwa_rxdata_t *rxdata)
{
	HWA_FTRACE(HWA2a);
	// ... placeholder
} // hwa_rxdata_detach

hwa_rxdata_t *
BCMATTACHFN(hwa_rxdata_attach)(hwa_dev_t *dev)
{
	int i;
	hwa_rxdata_t *rxdata;

	HWA_FTRACE(HWA2a);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// 32bit map used to track FHR filter types: see fhr_pktfetch, fhr_l2filter
	HWA_ASSERT(HWA_RXDATA_FHR_FILTERS_MAX == (NBBY * NBU32));
	HWA_ASSERT(sizeof(hwa_rxdata_fhr_entry_t) == HWA_RXDATA_FHR_ENTRY_BYTES);
	HWA_ASSERT(sizeof(hwa_rxdata_fhr_t) == HWA_RXDATA_FHR_BYTES);

	rxdata = &dev->rxdata;

	// Place all FHR filters into the free list
	for (i = 0; i < HWA_RXDATA_FHR_FILTERS_SW; i++) {
		rxdata->fhr[i].next = &rxdata->fhr[i + 1];
		rxdata->fhr[i].filter.config.id = i;
		rxdata->fhr[i].filter.config.type = HWA_RXDATA_FHR_FILTER_DISABLED;
	}
	rxdata->fhr[i - 1].next = (hwa_rxdata_fhr_filter_t*)NULL;
	rxdata->fhr_flist = &rxdata->fhr[0];

	return rxdata;

} // hwa_rxdata_attach

void // HWA2a RxData: Init RxData block after MAC has setup mac_fhr_base
hwa_rxdata_init(hwa_rxdata_t *rxdata)
{
	uint32 v32;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA2a);

	// Audit pre-conditions
	dev = HWA_DEV(rxdata);

	// Ensure that MAC has initialized mac_fhr_base, mac_fhr_stats and mac_regs
	HWA_ASSERT(rxdata->mac_fhr_base != 0U);
	HWA_ASSERT(rxdata->mac_fhr_stats != 0U);
	HWA_ASSERT(dev->mac_regs != (hc_hin_regs_t*)NULL);

	v32 =
		BCM_SBIT(_RXHWACTRL_GLOBALFILTEREN) |
		BCM_SBIT(_RXHWACTRL_PKTCOMPEN) |
		BCM_SBIT(_RXHWACTRL_CLRALLFILTERSTAT);
	HWA_WR_REG_ADDR(HWA2a, &dev->mac_regs->rxhwactrl, v32);

	// Only need to configure it once.
	if (!dev->inited) {
		hwa_rxdata_fhr_filter_init_pktfetch(rxdata);
	} else {
		// After core reset, rxfilteren will be 0. Driver need to reconfigure it.
		HWA_WR_REG_ADDR(HWA2a, &dev->mac_regs->rxfilteren, rxdata->rxfilteren);
	}
} // hwa_rxdata_init

void // HWA2a RxData: Deinit RxData block
hwa_rxdata_deinit(hwa_rxdata_t *rxdata)
{
	hwa_dev_t *dev;

	HWA_FTRACE(HWA2a);

	// Audit pre-conditions
	dev = HWA_DEV(rxdata);

	// Ensure that MAC has initialized mac_fhr_base, mac_fhr_stats and mac_regs
	HWA_ASSERT(rxdata->mac_fhr_base != 0U);
	HWA_ASSERT(rxdata->mac_fhr_stats != 0U);
	HWA_ASSERT(dev->mac_regs != (hc_hin_regs_t*)NULL);

	HWA_WR_REG_ADDR(HWA2a, &dev->mac_regs->rxhwactrl, 0);
}

// HWA2a RxData FHR Table Management

// Init exist known pktfetch filter
void
hwa_rxdata_fhr_filter_init_pktfetch(hwa_rxdata_t *rxdata)
{
	hwa_dev_t *dev;
	wlc_info_t *wlc;
	int32 fid;
	uint32 offset, bitmask, pattern;

	HWA_FTRACE(HWA2a);

	// Audit pre-conditions
	dev = HWA_DEV(rxdata);

	wlc = (wlc_info_t *)dev->wlc;
	BCM_REFERENCE(wlc);

	/* HW will take care ETHER_TYPE_1_OFFSET by itself. */

	/* EAPOL_FETCH */
	fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_PKTFETCH, 0, 1);
	if (fid != HWA_FAILURE) {
		offset = ETHER_TYPE_2_OFFSET;
		bitmask = 0xffff;
		pattern = HTON16(ETHER_TYPE_802_1X);
		hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask, (uint8 *)&pattern, 2);
		hwa_rxdata_fhr_filter_add(fid);
	}

	/* Set a filter for NON IP packet
	 *
	 * HW should set match the filter if incoming packet is not IPV4 and not IPV6
	 * Set the polarity fields of the params
	 */
	fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_L2FILTER, 0, 2);
	if (fid != HWA_FAILURE) {
		offset = ETHER_TYPE_2_OFFSET;
		bitmask = 0xffff;
		pattern = HTON16(ETHER_TYPE_IP);
		hwa_rxdata_fhr_param_add(fid, 1, offset, (uint8 *)&bitmask, (uint8 *)&pattern, 2);

		offset = ETHER_TYPE_2_OFFSET;
		bitmask = 0xffff;
		pattern = HTON16(ETHER_TYPE_IPV6);
		hwa_rxdata_fhr_param_add(fid, 1, offset, (uint8 *)&bitmask, (uint8 *)&pattern, 2);

		hwa_rxdata_fhr_filter_add(fid);
	}

	/* Multicast Filter : addr[0] & 1 == 1 */
	fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_L2FILTER, 0, 1);
	if (fid != HWA_FAILURE) {
		offset = HW_HDR_CONV_PAD;	/* 1st byte in DA */
		bitmask = 0x1;
		pattern = 0x1;	/* Check for 0x1 */
		hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask, (uint8 *)&pattern, 1);
		hwa_rxdata_fhr_filter_add(fid);
	}
#ifdef WLTDLS
	/* TDLS_PKTFETCH */
	if (TDLS_ENAB(wlc->pub)) {
		fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_PKTFETCH, 0, 1);
		if (fid != HWA_FAILURE) {
			offset = ETHER_TYPE_2_OFFSET;
			bitmask = 0xffff;
			pattern = HTON16(ETHER_TYPE_89_0D);
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 2);
			hwa_rxdata_fhr_filter_add(fid);
		}
	}
#endif /* WLTDLS */

}

#ifdef WLNDOE
void
hwa_rxdata_fhr_filter_ndoe(bool enable)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	int32 fid, i;
	uint32 offset, bitmask, pattern;

	HWA_FTRACE(HWA2a);

	// Audit preconditions
	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

	if (enable) {
		if (rxdata->ndoe_filter != 0) {
			// Already enable it, do nothing.
			return;
		}

		// NDOE_PKTFETCH_NS
		fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_PKTFETCH, 0, 3);
		if (fid != HWA_FAILURE) {
			offset = ETHER_TYPE_2_OFFSET;
			bitmask = 0xffff;
			pattern = HTON16(ETHER_TYPE_IPV6);
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 2);

			offset += (ETHER_TYPE_LEN + ICMP6_NEXTHDR_OFFSET_SPLIT_MODE4);
			bitmask = 0xff;
			pattern = ICMPV6_HEADER_TYPE;
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 1);

			offset += (ICMP6_TYPE_OFFSET_SPLIT_MODE4 -
				ICMP6_NEXTHDR_OFFSET_SPLIT_MODE4);
			bitmask = 0xff;
			pattern = ICMPV6_PKT_TYPE_NS;
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 1);
			hwa_rxdata_fhr_filter_add(fid);
			rxdata->ndoe_filter |= BCM_BIT(fid);
		}

		/* NDOE_PKTFETCH_RA */
		fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_PKTFETCH, 0, 3);
		if (fid != HWA_FAILURE) {
			offset = ETHER_TYPE_2_OFFSET;
			bitmask = 0xffff;
			pattern = HTON16(ETHER_TYPE_IPV6);
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 2);

			offset += (ETHER_TYPE_LEN + ICMP6_NEXTHDR_OFFSET_SPLIT_MODE4);
			bitmask = 0xff;
			pattern = ICMPV6_HEADER_TYPE;
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 1);

			offset += (ICMP6_TYPE_OFFSET_SPLIT_MODE4 -
				ICMP6_NEXTHDR_OFFSET_SPLIT_MODE4);
			bitmask = 0xff;
			pattern = ICMPV6_PKT_TYPE_RA;
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 1);
			hwa_rxdata_fhr_filter_add(fid);
			rxdata->ndoe_filter |= BCM_BIT(fid);
		}
	} else {
		if (rxdata->ndoe_filter) {
			for (i = 0; i < HWA_RXDATA_FHR_FILTERS_MAX; i++) {
				if (isset(rxdata->ndoe_filter, i)) {
					hwa_rxdata_fhr_filter_del(i);
				}
			}
			rxdata->ndoe_filter = 0;
		}
	}

}
#endif /* WLNDOE */

#if defined(BDO) || defined(TKO) || defined(ICMP)
void
hwa_rxdata_fhr_filter_ip(bool enable)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	int32 fid, i;
	uint32 offset, bitmask, pattern;

	HWA_FTRACE(HWA2a);

	// Audit preconditions
	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

	if (enable) {
		if (rxdata->ip_filter != 0) {
			// Already enable it, do nothing.
			return;
		}

		// PKTFETCH_IPV4
		fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_PKTFETCH, 0, 1);
		if (fid != HWA_FAILURE) {
			offset = ETHER_TYPE_2_OFFSET;
			bitmask = 0xffff;
			pattern = HTON16(ETHER_TYPE_IP);
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 2);
			hwa_rxdata_fhr_filter_add(fid);
			rxdata->ip_filter |= BCM_BIT(fid);
		}

		// PKTFETCH_IPV6
		fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_PKTFETCH, 0, 1);
		if (fid != HWA_FAILURE) {
			offset = ETHER_TYPE_2_OFFSET;
			bitmask = 0xffff;
			pattern = HTON16(ETHER_TYPE_IPV6);
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 2);
			hwa_rxdata_fhr_filter_add(fid);
			rxdata->ip_filter |= BCM_BIT(fid);
		}
	} else {
		if (rxdata->ip_filter) {
			for (i = 0; i < HWA_RXDATA_FHR_FILTERS_MAX; i++) {
				if (isset(rxdata->ip_filter, i)) {
					hwa_rxdata_fhr_filter_del(i);
				}
			}
			rxdata->ip_filter = 0;
		}
	}

}
#endif /* defined(BDO) || defined(TKO) || defined(ICMP) */

#ifdef WL_TBOW
void
hwa_rxdata_fhr_filter_tbow(bool enable)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	int32 fid, i;
	uint32 offset, bitmask, pattern;

	HWA_FTRACE(HWA2a);

	// Audit preconditions
	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

	if (enable) {
		if (rxdata->tbow_filter != 0) {
			// Already enable it, do nothing.
			return;
		}

		fid = hwa_rxdata_fhr_filter_new(HWA_RXDATA_FHR_PKTFETCH, 0, 1);
		if (fid != HWA_FAILURE) {
			offset = ETHER_TYPE_2_OFFSET;
			bitmask = 0xffff;
			pattern = HTON16(ETHER_TYPE_TBOW);
			hwa_rxdata_fhr_param_add(fid, 0, offset, (uint8 *)&bitmask,
				(uint8 *)&pattern, 2);
			hwa_rxdata_fhr_filter_add(fid);
			rxdata->tbow_filter |= BCM_BIT(fid);
		}
	} else {
		if (rxdata->tbow_filter) {
			for (i = 0; i < HWA_RXDATA_FHR_FILTERS_MAX; i++) {
				if (isset(rxdata->tbow_filter, i)) {
					hwa_rxdata_fhr_filter_del(i);
				}
			}
			rxdata->tbow_filter = 0;
		}
	}

}
#endif /* WL_TBOW */

int // Allocate a new filter
hwa_rxdata_fhr_filter_new(hwa_rxdata_fhr_filter_type_t filter_type,
	uint32 filter_polarity, uint32 param_count)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	hwa_rxdata_fhr_entry_t *filter;

	HWA_TRACE(("%s new filter type<%u> polarity<%u> count<%u>\n",
		HWA2a, filter_type, filter_polarity, param_count));

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

	HWA_ASSERT((param_count > 0) && (param_count < HWA_RXDATA_FHR_PARAMS_MAX));

	if (rxdata->fhr_flist == (hwa_rxdata_fhr_filter_t*)NULL) {
		HWA_WARN(("%s filter new fail\n", HWA2a));

		HWA_STATS_EXPR(rxdata->fhr_err_cnt++);

		return HWA_FAILURE;
	}

	if (rxdata->fhr_build != (hwa_rxdata_fhr_filter_t*)NULL) {
		HWA_WARN(("%s ovewriting filter<%u>\n",
			HWA2a, rxdata->fhr_build->filter.config.id));
	} else {
		rxdata->fhr_build = rxdata->fhr_flist;
		rxdata->fhr_flist = rxdata->fhr_flist->next;
		rxdata->fhr_build->next = NULL;
	}

	// Prepare the filter context ...
	filter = &rxdata->fhr_build->filter;

	filter->config.type        = filter_type; // SW only
	filter->config.polarity    = filter_polarity;
	filter->config.param_count = param_count;

	rxdata->param_count = 0U;

	return filter->config.id;

} // hwa_rxdata_fhr_filter_new

// Build the filter by specifying parameters, and finally configure in HWA FHR
int // Build a new filter by adding a parameter
hwa_rxdata_fhr_param_add(uint32 filter_id, uint32 polarity, uint32 offset,
	uint8 *bitmask, uint8 *pattern, uint32 match_sz)
{
	int i, j;
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	hwa_rxdata_fhr_param_t *param;
	hwa_rxdata_fhr_entry_t *filter;
	uint32 prefix;

	HWA_FTRACE(HWA2a);

	// Audit preconditions
	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

	HWA_ASSERT(filter_id < HWA_RXDATA_FHR_FILTERS_MAX);
	HWA_ASSERT(offset < (1 << 12)); // 11 bit offset in hwa_rxdata_fhr_entry_t
	HWA_ASSERT((bitmask != NULL) && (pattern != NULL));
	HWA_ASSERT(match_sz <= HWA_RXDATA_FHR_PATTERN_BYTES);

	HWA_ASSERT(rxdata->fhr_build != (hwa_rxdata_fhr_filter_t*)NULL);
	HWA_ASSERT(rxdata->param_count <
	           rxdata->fhr_build->filter.config.param_count);

	// Fetch filter
	filter = &rxdata->fhr_build->filter;
	HWA_ASSERT(filter->config.id == filter_id);

	// Shift the last valid byte to the end so that we can
	// filter small length packet.  i.e. EAPOL-WSC-START
	prefix = HWA_RXDATA_FHR_PATTERN_BYTES - match_sz;
	if (offset >= prefix) {
		offset = offset - prefix;
	}
	else {
		prefix = offset;
		offset = 0;
	}

	// Add parameter to the filter
	param = &filter->params[rxdata->param_count];
	param->config.polarity = polarity;
	param->config.offset   = offset;
	for (i = 0; i < prefix; i++) {
		param->bitmask[i]  = 0x00;
		param->pattern[i]  = 0x00;
	}
	for (j = 0; j < match_sz; i++, j++) {
		param->bitmask[i]  = bitmask[j];
		param->pattern[i]  = pattern[j];
	}
	for (; i < HWA_RXDATA_FHR_PATTERN_BYTES; i++) {
		param->bitmask[i]  = 0x00;
		param->pattern[i]  = 0x00;
	}

	rxdata->param_count++;

	HWA_ASSERT(rxdata->param_count <= filter->config.param_count);

	return filter_id;

} // hwa_rxdata_fhr_param_add

int // Add the built filter into MAC FHR register file
hwa_rxdata_fhr_filter_add(uint32 filter_id)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	hc_hin_regs_t *mac_regs;
	hwa_rxdata_fhr_entry_t *filter;
	hwa_rxdata_fhr_filter_type_t filter_type;

	HWA_FTRACE(HWA2a);

	// Audit preconditions
	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;
	mac_regs = dev->mac_regs;

	HWA_ASSERT(rxdata->mac_fhr_base != 0U);
	HWA_ASSERT(mac_regs != (hc_hin_regs_t*)NULL);
	HWA_ASSERT(filter_id < HWA_RXDATA_FHR_FILTERS_MAX);
	HWA_ASSERT(rxdata->fhr_build != (hwa_rxdata_fhr_filter_t*)NULL);

	// Setup locals
	filter = &rxdata->fhr_build->filter;
	HWA_ASSERT(filter->config.id == filter_id);
	HWA_ASSERT(filter->config.param_count == rxdata->param_count);

	filter_type = filter->config.type; // SW only
	switch (filter_type) {
		case HWA_RXDATA_FHR_PKTFETCH:
			setbit(&rxdata->fhr_pktfetch, filter_id); break;
		case HWA_RXDATA_FHR_L2FILTER:
			setbit(&rxdata->fhr_l2filter, filter_id); break;
		default: HWA_ASSERT(0); break;
	}

	filter->config.type = 0U; // SW use only

#ifdef HWA_RXDATA_FHR_IND_BUILD
	hwa_rxdata_fhr_indirect_write(rxdata, filter);
#else  /* ! HWA_RXDATA_FHR_IND_BUILD */
	{
		hwa_mem_addr_t fhr_addr;
		// Copy the filter to the HWA2a FHR Reg File using 32bit AXI access
		fhr_addr = HWA_TABLE_ADDR(hwa_rxdata_fhr_entry_t,
		                              rxdata->mac_fhr_base, filter_id);

		HWA_WR_MEM32(HWA2a, hwa_rxdata_fhr_entry_t, fhr_addr, filter);
	}
#endif /* ! HWA_RXDATA_FHR_IND_BUILD */

	// Enable the filter
	setbit(&rxdata->rxfilteren, filter_id);
	HWA_WR_REG_ADDR(HWA2a, &mac_regs->rxfilteren, rxdata->rxfilteren);

	HWA_STATS_EXPR(rxdata->fhr_ins_cnt++);

	filter->config.type = filter_type; // SW only

	rxdata->param_count = 0U;
	rxdata->fhr_build = (hwa_rxdata_fhr_filter_t*)NULL;

	return filter_id;

} // hwa_rxdata_fhr_filter_add

int // Delete a previously configured FHR filter
hwa_rxdata_fhr_filter_del(uint32 filter_id)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	hc_hin_regs_t *mac_regs;
	hwa_rxdata_fhr_filter_t *fhr_filter;
	hwa_rxdata_fhr_entry_t *filter;
	hwa_rxdata_fhr_filter_type_t filter_type;

	HWA_FTRACE(HWA2a);

	// Audit preconditions
	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;
	mac_regs = dev->mac_regs;

	HWA_ASSERT(rxdata->mac_fhr_base != 0U);
	HWA_ASSERT(mac_regs != (hc_hin_regs_t*)NULL);
	HWA_ASSERT(filter_id < HWA_RXDATA_FHR_FILTERS_MAX);
	HWA_ASSERT(isset(&rxdata->rxfilteren, filter_id));

	// Setup locals
	fhr_filter = &rxdata->fhr[filter_id];
	filter = &fhr_filter->filter;
	HWA_ASSERT(filter->config.id == filter_id);

	filter_type = filter->config.type; // SW only
	switch (filter_type) {
		case HWA_RXDATA_FHR_PKTFETCH:
			clrbit(&rxdata->fhr_pktfetch, filter_id); break;
		case HWA_RXDATA_FHR_L2FILTER:
			clrbit(&rxdata->fhr_l2filter, filter_id); break;
		default: HWA_ASSERT(0); break;
	}

	filter->config.type = 0;
	filter->config.polarity = 0;
	filter->config.param_count = 0;
	memset(filter->params, 0, HWA_RXDATA_FHR_ENTRY_BYTES - NBU32);

	// Disable the filter
	clrbit(&rxdata->rxfilteren, filter_id);
	HWA_WR_REG_ADDR(HWA2a, &mac_regs->rxfilteren, rxdata->rxfilteren);

#ifdef HWA_RXDATA_FHR_IND_BUILD
	hwa_rxdata_fhr_indirect_write(rxdata, filter);
#else  /* ! HWA_RXDATA_FHR_IND_BUILD */
	{
		hwa_mem_addr_t fhr_addr;
		// Copy the filter to the HWA2a FHR Reg File using 32bit AXI access
		fhr_addr = HWA_TABLE_ADDR(hwa_rxdata_fhr_entry_t,
		                              rxdata->mac_fhr_base, filter_id);

		HWA_WR_MEM32(HWA2a, hwa_rxdata_fhr_entry_t, fhr_addr, filter);
	}
#endif /* ! HWA_RXDATA_FHR_IND_BUILD */

	HWA_STATS_EXPR(rxdata->fhr_del_cnt++);

	filter->config.type = HWA_RXDATA_FHR_FILTER_DISABLED;

	if (rxdata->fhr_flist == (hwa_rxdata_fhr_filter_t*)NULL) {
		rxdata->fhr_flist = fhr_filter;
	} else {
		fhr_filter->next = rxdata->fhr_flist;
		rxdata->fhr_flist = fhr_filter;
	}

	return filter_id;

} // hwa_rxdata_fhr_filter_del

uint32 // Determine whether any filters hit a pktfetch filter type
hwa_rxdata_fhr_is_pktfetch(uint32 fhr_filter_match)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
#ifdef WOWLPF
	wlc_info_t *wlc;
#endif // endif

	HWA_FTRACE(HWA2a);

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

#ifdef WOWLPF
	wlc = (wlc_info_t *)dev->wlc;
	if (WOWLPF_ACTIVE(wlc->pub))
		return TRUE;
#endif // endif

	return (rxdata->fhr_pktfetch & fhr_filter_match);

} // hwa_rxdata_fhr_is_pktfetch

uint32 // Determine whether any filters hit a l2filter type
hwa_rxdata_fhr_is_l2filter(uint32 fhr_filter_match)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;

	HWA_FTRACE(HWA2a);

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

	return (rxdata->fhr_l2filter & fhr_filter_match);
}

// FHR match statistics management
uint32 // Get the hit statistics for a filter
hwa_rxdata_fhr_hits_get(uint32 filter_id)
{
	hwa_dev_t *dev;
	hwa_rxdata_t *rxdata;
	uint32 filter_hits;
	hwa_mem_addr_t fhr_stats_addr;

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit
	rxdata = &dev->rxdata;

	HWA_ASSERT(rxdata->mac_fhr_stats != 0U);
	HWA_ASSERT(filter_id < HWA_RXDATA_FHR_FILTERS_MAX);

	if (!dev->up) {
		return 0;
	}

	fhr_stats_addr = HWA_TABLE_ADDR(hwa_rxdata_fhr_stats_entry_t,
	                                    rxdata->mac_fhr_stats, filter_id);

	HWA_RD_MEM32(HWA2a, hwa_rxdata_fhr_stats_entry_t,
	             fhr_stats_addr, &filter_hits);

	HWA_TRACE(("%s fhr filter<%u> hits<%u>\n", HWA2a, filter_id, filter_hits));

	return filter_hits;

} // hwa_rxdata_fhr_hits_get

void // Clear the statistics for a filter or ALL filters i.e filter_id = ~0U
hwa_rxdata_fhr_hits_clr(uint32 filter_id)
{
	uint32 v32;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA2a);

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit

	HWA_ASSERT(dev->mac_regs != (hc_hin_regs_t*)NULL);

	// Register is shared with ENables, so use read-modify-write
	v32 = HWA_RD_REG_ADDR(HWA2a, &dev->mac_regs->rxhwactrl);
	v32 &= _RXHWACTRL_GLOBALFILTEREN_MASK | _RXHWACTRL_PKTCOMPEN_MASK;

	if (filter_id == ~0U) {
		v32 |= BCM_SBIT(_RXHWACTRL_CLRALLFILTERSTAT);
	} else {
		v32 |= BCM_SBF(filter_id, _RXHWACTRL_ID) |
		      BCM_SBIT(_RXHWACTRL_CLRAFILTERSTAT);
	}

	HWA_WR_REG_ADDR(HWA2a, &dev->mac_regs->rxhwactrl, v32);

} // hwa_rxdata_fhr_hits_clr

#if defined(BCMDBG)
// HWA2a RxData debug support

static void hwa_rxdata_filter_dump(hwa_rxdata_fhr_entry_t *filter, struct bcmstrbuf *b);

static void // HWA2a RxData dump a FHR filter
hwa_rxdata_filter_dump(hwa_rxdata_fhr_entry_t *filter, struct bcmstrbuf *b)
{
	int id, byte;
	hwa_rxdata_fhr_param_t *param;
	HWA_ASSERT(filter != (hwa_rxdata_fhr_entry_t*)NULL);

	// Filter config
	HWA_BPRINT(b, "+ Filter<%u> polarity<%u> params<%u>\n",
		filter->config.id, filter->config.polarity, filter->config.param_count);

	for (id = 0; id < filter->config.param_count; id++) {
		param = &filter->params[id];
		// Param config
		HWA_BPRINT(b, "+    Param<%u> polarity<%u> offset<%u>: bitmask[",
			id, param->config.polarity, param->config.offset);
		for (byte = 0; byte < HWA_RXDATA_FHR_PATTERN_BYTES; byte++)
			HWA_BPRINT(b, "%02X", param->bitmask[byte]); // param bitmask bytes
		HWA_BPRINT(b, "] pattern[");
		for (byte = 0; byte < HWA_RXDATA_FHR_PATTERN_BYTES; byte++)
			HWA_BPRINT(b, "%02X", param->pattern[byte]); // param pattern bytes
		HWA_BPRINT(b, "]\n");
	}

	HWA_BPRINT(b, "+    Filter_hits<%u>\n", hwa_rxdata_fhr_hits_get(filter->config.id));

} // hwa_rxdata_filter_dump

void // HWA2a RxData: dump FHR table
hwa_rxdata_fhr_dump(hwa_rxdata_t *rxdata, struct bcmstrbuf *b, bool verbose)
{
	int id;
	hwa_rxdata_fhr_entry_t *filter;

	HWA_ASSERT(rxdata != (hwa_rxdata_t*)NULL);

	HWA_BPRINT(b, "%s FHR rxfilteren<0x%08x> pktfetch<0x%08x> l2filter<0x%08x>\n",
		HWA2a, rxdata->rxfilteren, rxdata->fhr_pktfetch, rxdata->fhr_l2filter);

	if (rxdata->fhr_build != (hwa_rxdata_fhr_filter_t*)NULL) {
		HWA_BPRINT(b, "+ fhr_build id<%u> param_count<%u>\n",
			rxdata->fhr_build->filter.config.id, rxdata->param_count);
		hwa_rxdata_filter_dump(&rxdata->fhr_build->filter, b);
	}

	for (id = 0; id < HWA_RXDATA_FHR_FILTERS_SW; id++) {

		filter = &rxdata->fhr[id].filter;
		if (filter->config.type != HWA_RXDATA_FHR_FILTER_DISABLED)
			hwa_rxdata_filter_dump(filter, b);
	}

	HWA_STATS_EXPR(HWA_BPRINT(b, "+ FHR stats ins<%u> del<%u> err<%u>\n",
		rxdata->fhr_ins_cnt, rxdata->fhr_del_cnt, rxdata->fhr_err_cnt));

} // hwa_rxdata_fhr_dump

void // HWA2a RxData: debug dump
hwa_rxdata_dump(hwa_rxdata_t *rxdata, struct bcmstrbuf *b, bool verbose)
{
	HWA_BPRINT(b, "%s dump<%p>\n", HWA2a, rxdata);

	if (rxdata == (hwa_rxdata_t*)NULL)
		return;

	HWA_BPRINT(b, "%s mac_fhr_base<0x%08x> mac_fhr_stats<0x%08x>\n",
		HWA2a, rxdata->mac_fhr_base, rxdata->mac_fhr_stats);

	hwa_rxdata_fhr_dump(rxdata, b, verbose);

} // hwa_rxdata_dump

#endif /* BCMDBG */

#endif /* HWA_RXDATA_BUILD */

#ifdef HWA_TXFIFO_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: HWA3b TxFifo block bringup: attach, detach, and init phases
 * -----------------------------------------------------------------------------
 */

// Free pkts of TxFIFOs shadow context
static int16 hwa_txfifo_shadow_reclaim(hwa_dev_t *dev, uint32 core, uint32 fifo_idx);

void // HWA3b: Cleanup/Free resources used by TxFifo block
BCMATTACHFN(hwa_txfifo_detach)(hwa_txfifo_t *txfifo)
{
	void *memory;
	uint32 mem_sz;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA3b);

	if (txfifo == (hwa_txfifo_t*)NULL)
		return;

	// Audit pre-conditions
	dev = HWA_DEV(txfifo);

	// HWA3b TxFifo PktChain Ring: free memory and reset ring
	if (txfifo->pktchain_ring.memory != (void*)NULL) {
		memory = txfifo->pktchain_ring.memory;
		mem_sz = txfifo->pktchain_ring.depth * txfifo->pktchain_fmt;
		HWA_TRACE(("%s pktchain_ring -memory[%p:%u]\n", HWA3b, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		hwa_ring_fini(&txfifo->pktchain_ring);
		txfifo->pktchain_ring.memory = (void*)NULL;
	}

} // hwa_txfifo_detach

hwa_txfifo_t * // HWA3b: Allocate resources for TxFifo block
BCMATTACHFN(hwa_txfifo_attach)(hwa_dev_t *dev)
{
	void *memory;
	uint32 mem_sz, depth;
	hwa_regs_t *regs;
	hwa_txdma_regs_t *txdma;
	hwa_txfifo_t *txfifo;

	HWA_FTRACE(HWA3b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	regs = dev->regs;
	txdma = &regs->txdma;
	txfifo = &dev->txfifo;

	// Verify HWA3b block's structures
	HWA_ASSERT(HWA_TX_CORES == 1);
	HWA_ASSERT(sizeof(hwa_txfifo_pktchain32_t) == HWA_TXFIFO_PKTCHAIN32_BYTES);
	HWA_ASSERT(sizeof(hwa_txfifo_pktchain64_t) == HWA_TXFIFO_PKTCHAIN64_BYTES);
	HWA_ASSERT(sizeof(hwa_txfifo_ovflwqctx_t) == HWA_TXFIFO_OVFLWQCTX_BYTES);
	HWA_ASSERT(sizeof(hwa_txfifo_fifoctx_t) == HWA_TXFIFO_FIFOCTX_BYTES);

	// Confirm HWA TxFIFO Capabilities against 43684 Generic
	{
		uint32 cap1, cap2;
		BCM_REFERENCE(cap1); BCM_REFERENCE(cap2);
		cap1 = HWA_RD_REG_NAME(HWA3b, regs, top, hwahwcap1);
		cap2 = HWA_RD_REG_NAME(HWA3b, regs, top, hwahwcap2);
		HWA_ASSERT(BCM_GBF(cap1, HWA_TOP_HWAHWCAP1_MAXSEGCNT3A3B) ==
			HWA_TX_DATABUF_SEGCNT_MAX);
#if !HWA_REVISION_EQ_128 && !HWA_REVISION_GE_129
		// What's this "bit 15:13 / maxRingInfo3b / Maximum number of ringinfo
		// table structures supported in 3b." ?
		// Don't check in rev128 and rev129
		HWA_ASSERT(BCM_GBF(cap1, HWA_TOP_HWAHWCAP1_MAXRINGINFO3B) ==
			HWA_TXFIFO_CHICKEN_FEATURE);
#endif /* !HWA_REVISION_EQ_128 && !HWA_REVISION_GE_129 */
		HWA_ASSERT(BCM_GBF(cap2, HWA_TOP_HWAHWCAP2_NUMMACTXFIFOQ) ==
			HWA_TX_FIFOS);
		HWA_ASSERT(BCM_GBIT(cap2, HWA_TOP_HWAHWCAP2_TOP2REGS_RINFO_CAP) ==
			HWA_TXFIFO_CHICKEN_FEATURE);
		HWA_ASSERT(BCM_GBIT(cap2, HWA_TOP_HWAHWCAP2_TOP2REGS_PKTINFO_CAP) ==
			HWA_TXFIFO_CHICKEN_FEATURE);
		HWA_ASSERT(BCM_GBIT(cap2, HWA_TOP_HWAHWCAP2_TOP2REGS_CACHEINFO_CAP) ==
			HWA_TXFIFO_CHICKEN_FEATURE);
		HWA_ASSERT(BCM_GBIT(cap2, HWA_TOP_HWAHWCAP2_TOP2REGS_OVFLOWQ_PRESENT) ==
			(HWA_OVFLWQ_MAX != 0));
	}

	// FD always uses a 32b packet chain. NIC may use 32bit or 64bit.
	if (sizeof(memory) == sizeof(int)) {
		txfifo->pktchain_fmt = sizeof(hwa_txfifo_pktchain32_t);
	} else { // NIC 64bit
		txfifo->pktchain_fmt = sizeof(hwa_txfifo_pktchain64_t);
	}

	// Allocate and initialize S2H pktchain xmit request interface
	depth = HWA_TXFIFO_PKTCHAIN_RING_DEPTH;
	mem_sz = depth * txfifo->pktchain_fmt;
	// hwa_regs::txdma::sw2hwa_tx_pkt_chain_q_base_addr_h
	if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
		HWA_ERROR(("%s pktchain_ring malloc size<%u> failure\n",
			HWA3b, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		goto failure;
	}
	HWA_TRACE(("%s pktchain_ring +memory[%p,%u]\n", HWA3b, memory, mem_sz));
	hwa_ring_init(&txfifo->pktchain_ring, "CHN",
		HWA_TXFIFO_ID, HWA_RING_S2H, HWA_TXFIFO_PKTCHAIN_S2H_RINGNUM,
		depth, memory, &txdma->sw2hwa_tx_pkt_chain_q_wr_index,
		&txdma->sw2hwa_tx_pkt_chain_q_rd_index);

	// Initialize the Overflow Queue AXI memory address
	txfifo->ovflwq_addr = hwa_axi_addr(dev, HWA_AXI_TXFIFO_OVFLWQS);

	// Initialize the TxFIFO and AQM FIFO AXI memory address
	txfifo->txfifo_addr = hwa_axi_addr(dev, HWA_AXI_TXFIFO_TXFIFOS);

	txfifo->fifo_total = HWA_TX_FIFOS;

	// Allocate and initialize TxFIFOs shadow context
	if (sizeof(memory) == sizeof(int)) {
		depth = HWA_TX_FIFOS * sizeof(hwa_txfifo_shadow32_t);
	} else { // NIC 64bit
		depth = HWA_TX_FIFOS * sizeof(hwa_txfifo_shadow64_t);
	}
	mem_sz = depth * HWA_TX_FIFOS;
	if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
		HWA_ERROR(("%s txfifos shadow context malloc size<%u> failure\n",
			HWA3b, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		goto failure;
	}
	txfifo->txfifo_shadow = memory;
	HWA_TRACE(("%s txfifos shadow +memory[%p,%u]\n", HWA3b, memory, mem_sz));

	return txfifo;

failure:
	hwa_txfifo_detach(txfifo);
	HWA_WARN(("%s attach failure\n", HWA3b));

	return ((hwa_txfifo_t*)NULL);

} // hwa_txfifo_attach

void // hwa_txfifo_init
hwa_txfifo_init(hwa_txfifo_t *txfifo)
{
	uint32 u32, i, hi32, addr64;
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	hwa_ring_t *pktchain_ring; // S2H pktchain ring context
#if HWA_REVISION_GE_129
	uint16 u16;
#endif // endif

	HWA_FTRACE(HWA3b);

	// Audit pre-conditions
	dev = HWA_DEV(txfifo);

	// Setup locals
	regs = dev->regs;
	pktchain_ring = &txfifo->pktchain_ring;

	for (i = 0; i < HWA_TX_FIFOS; i++) {
		if (isset(txfifo->fifo_enab, i)) {
			// Use register interface to program hwa_txfifo_fifoctx_t in AXI memory
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_index, i);
			// PKT FIFO context: hwa_txfifo_fifoctx::pkt_fifo
			// hwa_txfifo_fifoctx::pkt_fifo.base.loaddr
			u32 = txfifo->fifo_base[i].loaddr;
#if HWA_REVISION_EQ_128
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_base_addr, u32);
#else
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_base_addrlo, u32);
#endif // endif
			// hwa_txfifo_fifoctx::pkt_fifo.base.hiaddr
			u32 = txfifo->fifo_base[i].hiaddr;
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_base_addrhi, u32);
			// hwa_txfifo_fifoctx::pkt_fifo.curr_ptr
			u32 = txfifo->fifo_base[i].loaddr & 0xFFFF;
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_rd_index, u32);
			// hwa_txfifo_fifoctx::pkt_fifo.last_ptr
			u32 = txfifo->fifo_base[i].loaddr & 0xFFFF;
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_wr_index, u32);
			// hwa_txfifo_fifoctx::pkt_fifo.attrib
			u32 = (0U
				| BCM_SBIT(HWA_TXFIFO_FIFO_COHERENCY)
				| 0U);
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_attrib, u32);
			// hwa_txfifo_fifoctx::pkt_fifo.depth
			u32 = txfifo->fifo_depth[i];
			HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_depth, u32);
			// AQM FIFO context: hwa_txfifo_fifoctx::aqm_fifo
			// hwa_txfifo_fifoctx::aqm_fifo.base.loaddr
			u32 = txfifo->aqm_fifo_base[i].loaddr;
			HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_base_addrlo, u32);
			// hwa_txfifo_fifoctx::aqm_fifo.base.hiaddr
			u32 = txfifo->aqm_fifo_base[i].hiaddr;
			HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_base_addrhi, u32);
			// hwa_txfifo_fifoctx::aqm_fifo.curr_ptr
			u32 = txfifo->aqm_fifo_base[i].loaddr & 0xFFFF;
			HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_rd_index, u32);
			// hwa_txfifo_fifoctx::aqm_fifo.last_ptr
			u32 = txfifo->aqm_fifo_base[i].loaddr & 0xFFFF;
			HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_wr_index, u32);
			// hwa_txfifo_fifoctx::aqm_fifo.attrib
			u32 = (0U
				| BCM_SBIT(HWA_TXFIFO_FIFO_COHERENCY)
				| BCM_SBIT(HWA_TXFIFO_FIFO_NOTPCIE)
				| 0U);
			HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_attrib, u32);
			// hwa_txfifo_fifoctx::aqm_fifo.depth
			u32 = txfifo->aqm_fifo_depth[i];
			HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_depth, u32);
		}
	}

	u32 = HWA_PTR2UINT(pktchain_ring->memory);
	HWA_WR_REG_NAME(HWA3b, regs, txdma,
		sw2hwa_tx_pkt_chain_q_base_addr_l, u32);

	u32 = BCM_SBF(HWA_TXFIFO_PKTCHAIN_RING_DEPTH,
			HWA_TXDMA_SW2HWA_TX_PKT_CHAIN_Q_CTRL_PKTCHAINQDEPTH);
	HWA_WR_REG_NAME(HWA3b, regs, txdma, sw2hwa_tx_pkt_chain_q_ctrl, u32);

	u32 = HWA_PTR2HIADDR(pktchain_ring->memory);
	HWA_WR_REG_NAME(HWA3b, regs, txdma,
		sw2hwa_tx_pkt_chain_q_base_addr_h, u32);

#if HWA_REVISION_EQ_128
	HWA_WR_REG_NAME(HWA3b, regs, txdma,
		num_txfifo_percore, txfifo->fifo_total);
#endif // endif

	// Settings "NotPCIE, Coherent and AddrExt" for misc HW DMA transactions
	u32 = (0U
	// PCIE Source or Destination: NIC mode MAC Ifs may be placed in SysMem
	| BCM_SBF(dev->macif_placement, // NIC mode S2H PktChainIf is over PCIe
	          HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_DMAPCIEDESCTEMPLATENOTPCIE)
	| BCM_SBF(dev->macif_coherency, // NIC mode S2H PktChainIf is host_coh
		HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_DMAPCIEDESCTEMPLATECOHERENT)
	| BCM_SBF(0U,
		HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_DMAPCIEDESCTEMPLATEADDREXT)
	// TCM Source or Destination: NotPcie = 1, Coh = 1, AddrExt = 0b00
	| BCM_SBIT(HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_DMATCMDESCTEMPLATENOTPCIE)
	| BCM_SBIT(HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_DMATCMDESCTEMPLATECOHERENT)
	| BCM_SBF(0U, HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_DMATCMDESCTEMPLATEADDREXT)
	// HWA local memory as source or destination: NotPcie = 1
	| BCM_SBIT(HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_DMAHWADESCTEMPLATENOTPCIE)
	// MAC TxFIFO WR update
#ifdef BCMPCIEDEV
	| BCM_SBIT(HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_NONDMAHWADESCTEMPLATECOHERENT)
#else
	| BCM_SBIT(dev->host_coherency,
		HWA_TXDMA_DMA_DESC_TEMPLATE_TXDMA_NONDMAHWADESCTEMPLATECOHERENT)
#endif // endif
	| 0U);
	HWA_WR_REG_NAME(HWA3b, regs, txdma, dma_desc_template_txdma, u32);

	// Setup HWA_TXDMA2_CFG1
#ifdef BCMPCIEDEV
	// Dongle uses 32bit pointers for head tail, with hi32 = 0U
	hi32 = 0U;
	addr64 = 0U;
#else
	hi32 = dev->host_physaddrhi;
	addr64 = dev->host_addressing;
#endif /* ! BCMPCIEDEV */
	u32 = (0U
		| BCM_SBIT(HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_PKTCHAIN_NEW_FORMAT)
		| BCM_SBF(addr64,
		          HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_PKTCHAIN_64BITADDRESS)
		| BCM_SBIT(HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_USE_OVFLOWQ)
		// BCM_SBIT(HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_NON_AQM_CTDMA_MODE)
#if HWA_REVISION_GE_129
		//CRWLHWA-446
		//txdma_last_ptr_update	AQM/TxDMA last ptr update
		//0: with direct path; 1 : with APB interface.
		| BCM_SBF(dev->txfifo_apb, HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_LAST_PTR_UPDATE)
		// CRWLHWA-439
		| BCM_SBF(dev->txfifo_hwupdnext,
			HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_HW_UPDATE_PKTNXT)
		// SW check TXFIFO context with signal burst. 0:muliti 1:signal.
		| BCM_SBIT(HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_CHECK_TXFIFO_CONTEXT)
#endif // endif
		| 0U);
	HWA_WR_REG_NAME(HWA3b, regs, txdma, hwa_txdma2_cfg1, u32);

	// Setup HWA_TXDMA2_CFG2
#if HWA_REVISION_EQ_128
	HWA_WR_REG_NAME(HWA3b, regs, txdma, hwa_txdma2_cfg2, hi32);
#else
	// We must to set txdma_aggr_aqm_descriptor_enable when txfifo_apb is 0,
	// otherwise we will hit frameid mismatch type3.
	// RTL simulation suggest to keep txdma_aggr_aqm_descriptor_enable and
	// txdma_aggr_aqm_descriptor_threshold as default 1 and 3.

	// txs->frameid 0x8ac1 seq 277 txh->TxFrameID 0x8a41 seq 276 when it
	// configure more than 0. (0 imply 1 aqm).  So, keep it update per one aqm.
	// Enlarge HWA_TXDMA_HWA_TXDMA2_CFG3_AQM_DESC_FREE_SPACE
	// doesn't help.
	u32 = (0U
		| BCM_SBF(0,
			HWA_TXDMA_HWA_TXDMA2_CFG2_TXDMA_AGGR_AQM_DESCRIPTOR_THRESHOLD)
		| BCM_SBIT(HWA_TXDMA_HWA_TXDMA2_CFG2_TXDMA_AGGR_AQM_DESCRIPTOR_ENABLE)
		| BCM_SBF(HWA_TXFIFO_PKTCNT_THRESHOLD,
			HWA_TXDMA_HWA_TXDMA2_CFG2_TXDMA_SWTXPKT_CNT_REACH)
		| 0U);
	HWA_WR_REG_NAME(HWA3b, regs, txdma, hwa_txdma2_cfg2, u32);
#endif /* HWA_REVISION_EQ_128 */

	// Setup HWA_TXDMA2_CFG3
	u32 = HWA_TXFIFO_LIMIT_THRESHOLD;
#if HWA_REVISION_GE_129
	// aqm_desc_free_space
	u32 |= (BCM_SBF(HWA_TXFIFO_AGGR_AQM_DESC_THRESHOLD + 1,
		HWA_TXDMA_HWA_TXDMA2_CFG3_AQM_DESC_FREE_SPACE));
#endif // endif
	HWA_WR_REG_NAME(HWA3b, regs, txdma, hwa_txdma2_cfg3, u32);

	// Setup HWA_TXDMA2_CFG4 that controls descriptor generation
	u32 = (0U
		| BCM_SBF(HWA_TXFIFO_EMPTY_THRESHOLD,
			HWA_TXDMA_HWA_TXDMA2_CFG4_TXDMA_EMPTY_CNT_REACH)
#if HWA_REVISION_EQ_128
		| BCM_SBF(HWA_TXFIFO_PKTCNT_THRESHOLD,
			HWA_TXDMA_HWA_TXDMA2_CFG4_TXDMA_SWTXPKT_CNT_REACH)
#endif // endif
		| 0U);
	HWA_WR_REG_NAME(HWA3b, regs, txdma, hwa_txdma2_cfg4, u32);

	// pktchain head, tail and swtx::pkt_next all point to packets and
	HWA_WR_REG_NAME(HWA3b, regs, txdma, sw_tx_pkt_nxt_h, hi32);

#if HWA_REVISION_GE_129
	//CRWLHWA-446
	//Enable Req-Ack based MAC_HWA i/f is enabled for tx Dma last index update.
	u16 = HWA_RD_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl);
	if (dev->txfifo_apb == 0) {
		u16 |= BCM_SBIT(_HWA_MACIF_CTL_TXDMAEN);
	} else {
		u16 = BCM_CBIT(u16, _HWA_MACIF_CTL_TXDMAEN);
	}
	HWA_WR_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl, u16);
#endif // endif

	return;
} // hwa_txfifo_init

void // HWA3b: Deinit TxFifo block
hwa_txfifo_deinit(hwa_txfifo_t *txfifo)
{
#if HWA_REVISION_GE_129
	hwa_dev_t *dev;

	HWA_FTRACE(HWA3b);

	// Audit pre-conditions
	dev = HWA_DEV(txfifo);

	//Disable Req-Ack based MAC_HWA i/f is enabled for tx Dma last index update.
	if (dev->txfifo_apb == 0) {
		uint16 u16;

		u16 = HWA_RD_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl);
		u16 = BCM_CBIT(u16, _HWA_MACIF_CTL_TXDMAEN);
		HWA_WR_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl, u16);
	}
#endif /* HWA_REVISION_GE_129 */
}

int // Configure a TxFIFO's pkt and aqm ring context in HWA AXI memory
hwa_txfifo_config(struct hwa_dev *dev, uint32 core, uint32 fifo_idx,
	dma64addr_t fifo_base, uint32 fifo_depth,
	dma64addr_t aqm_fifo_base, uint32 aqm_fifo_depth)
{
	hwa_txfifo_t *txfifo;

	HWA_TRACE(("%s config core<%u> FIFO %u base<0x%08x,0x%08x> depth<%u>"
		" AQM base<0x%08x,0x%08x> depth<%u>\n",
		HWA3b, core, fifo_idx, fifo_base.hiaddr, fifo_base.loaddr, fifo_depth,
		aqm_fifo_base.hiaddr, aqm_fifo_base.loaddr, aqm_fifo_depth));

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_TX_CORES);
	HWA_ASSERT(fifo_idx < HWA_TX_FIFOS);

	// Setup locals
	txfifo = &dev->txfifo;

	// Save settings in local state
	txfifo->fifo_base[fifo_idx] = fifo_base;
	txfifo->fifo_depth[fifo_idx] = fifo_depth;
	txfifo->aqm_fifo_base[fifo_idx] = aqm_fifo_base;
	txfifo->aqm_fifo_depth[fifo_idx] = aqm_fifo_depth;

	setbit(txfifo->fifo_enab, fifo_idx);

	return HWA_SUCCESS;

} // hwa_txfifo_config

void // Prepare to disable 3b block before MAC suspend.
hwa_txfifo_disable_prep(struct hwa_dev *dev, uint32 core)
{
	hwa_txfifo_t *txfifo;
	uint32 u32, idle, loop_count;
	uint32 dma_desc_busy;

	HWA_FTRACE(HWA3b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_TX_CORES);

	// Setup locals
	txfifo = &dev->txfifo;

	if (!dev)
		return;

	// Before disable HWA_MODULE_TXFIFO, we need to
	// wait until HWA consume all 3b pktchain ring
	loop_count = HWA_FSM_IDLE_POLLLOOP;
	while (!hwa_ring_is_cons_all(&txfifo->pktchain_ring)) {
		HWA_TRACE(("%s HWA consuming 3b pktchain ring\n", __FUNCTION__));
		OSL_DELAY(1);
		if (--loop_count == 0) {
			HWA_ERROR(("%s Cannot consume 3b pktchain ring\n", __FUNCTION__));
			break;
		}
	}
	// Poll STATE_STS::sw_pkt_curstate <= 1
	loop_count = 0;
	do {
		if (loop_count)
			OSL_DELAY(1);
		u32 = HWA_RD_REG_NAME(HWA3b, dev->regs, txdma, state_sts);
		idle = BCM_GBF(u32, HWA_TXDMA_STATE_STS_SW_PKT_CURSTATE);
		HWA_TRACE(("%s Polling STATE_STS::sw_pkt_curstate <%d>\n",
			__FUNCTION__, idle));
	} while (idle > 1 && ++loop_count != HWA_FSM_IDLE_POLLLOOP);
	if (loop_count == HWA_FSM_IDLE_POLLLOOP) {
		HWA_ERROR(("%s STATE_STS::sw_pkt_curstate is not idle <%u>\n",
			__FUNCTION__, idle));
	}

	// Set HWA_TXDMA2_CFG1::txdma_stop_ovflowq bit.
	u32 = HWA_RD_REG_NAME(HWA3b, dev->regs, txdma, hwa_txdma2_cfg1);
	u32 |= BCM_SBIT(HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_STOP_OVFLOWQ);
	HWA_WR_REG_NAME(HWA3b, dev->regs, txdma, hwa_txdma2_cfg1, u32);

	// Poll STATE_STS2::overflowq_state <= 1
	loop_count = 0;
	do {
		if (loop_count)
			OSL_DELAY(1);
		u32 = HWA_RD_REG_NAME(HWA3b, dev->regs, txdma, state_sts2);
		idle = BCM_GBF(u32, HWA_TXDMA_STATE_STS2_OVERFLOWQ_STATE);
		HWA_TRACE(("%s Polling STATE_STS2::overflowq_state <%d>\n",
			__FUNCTION__, idle));
	} while (idle > 1 && ++loop_count != HWA_FSM_IDLE_POLLLOOP);
	if (loop_count == HWA_FSM_IDLE_POLLLOOP) {
		HWA_ERROR(("%s STATE_STS2::overflowq_state is not idle <%d>\n",
			__FUNCTION__, idle));
	}

	// Poll STATE_STS::dma_des_curstate and fifo_dma_curstate are both 0
	loop_count = 0;
	do {
		if (loop_count)
			OSL_DELAY(1);
		u32 = HWA_RD_REG_NAME(HWA3b, dev->regs, txdma, state_sts);
		HWA_TRACE(("%s Polling STATE_STS::sw_pkt_curstate <%d>\n",
			__FUNCTION__, idle));
		dma_desc_busy = BCM_GBF(u32, HWA_TXDMA_STATE_STS_DMA_DES_CURSTATE);
		dma_desc_busy |= BCM_GBF(u32, HWA_TXDMA_STATE_STS_FIFO_DMA_CURSTATE);
	} while (dma_desc_busy && ++loop_count != HWA_FSM_IDLE_POLLLOOP);
	if (loop_count == HWA_FSM_IDLE_POLLLOOP) {
		HWA_ERROR(("%s STATE_STS::state_sts dma desc is not idle <%u>\n",
			__FUNCTION__, u32));
	}
}

void // Enable or Disable 3b block
hwa_txfifo_enable(struct hwa_dev *dev, uint32 core, bool enable)
{
	uint32 u32, idle, loop_count;
	uint32 dma_desc_busy;

	HWA_FTRACE(HWA3b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_TX_CORES);

	if (!dev)
		return;

	// Handle enable case and return.
	if (enable) {
		// Enable TXFIFO module
		hwa_module_request(dev, HWA_MODULE_TXFIFO, HWA_MODULE_ENABLE, enable);

		// Clear HWA_TXDMA2_CFG1::txdma_stop_ovflowq
		u32 = HWA_RD_REG_NAME(HWA3b, dev->regs, txdma, hwa_txdma2_cfg1);
		u32 = BCM_CBIT(u32, HWA_TXDMA_HWA_TXDMA2_CFG1_TXDMA_STOP_OVFLOWQ);
		HWA_WR_REG_NAME(HWA3b, dev->regs, txdma, hwa_txdma2_cfg1, u32);

		return;
	}

	// Poll STATE_STS::dma_des_curstate and fifo_dma_curstate are both 0
	// Check again after MAC suspended.
	loop_count = 0;
	do {
		if (loop_count)
			OSL_DELAY(1);
		u32 = HWA_RD_REG_NAME(HWA3b, dev->regs, txdma, state_sts);
		HWA_TRACE(("%s Polling STATE_STS::sw_pkt_curstate <%d>\n",
			__FUNCTION__, idle));
		dma_desc_busy = BCM_GBF(u32, HWA_TXDMA_STATE_STS_DMA_DES_CURSTATE);
		dma_desc_busy |= BCM_GBF(u32, HWA_TXDMA_STATE_STS_FIFO_DMA_CURSTATE);
	} while (dma_desc_busy && ++loop_count != HWA_FSM_IDLE_POLLLOOP);
	if (loop_count == HWA_FSM_IDLE_POLLLOOP) {
		HWA_ERROR(("%s STATE_STS::state_sts dma desc is not idle <%u>\n",
			__FUNCTION__, u32));
	}

	// Disable TXFIFO module
	hwa_module_request(dev, HWA_MODULE_TXFIFO, HWA_MODULE_ENABLE, enable);

	// Poll module idle bit
	loop_count = HWA_MODULE_IDLE_BURNLOOP;
	do { // Burnloop: allowing HWA3b to complete a previous DMA
		idle = hwa_module_request(dev, HWA_MODULE_TXFIFO, HWA_MODULE_IDLE, TRUE);
	} while (!idle && loop_count--);

	if (!loop_count) {
		HWA_ERROR(("%s %s: Tx block idle<%u> loop<%u>\n", HWA3b, __FUNCTION__,
			idle, HWA_MODULE_IDLE_BURNLOOP));
	}
}

void // Reset TxFIFO's curr_ptr and last_ptr of pkt and aqm ring context in HWA AXI memory
hwa_txfifo_dma_init(struct hwa_dev *dev, uint32 core, uint32 fifo_idx)
{
	hwa_regs_t *regs;
	hwa_txfifo_t *txfifo;
	uint32 u32;

	HWA_TRACE(("%s %s core<%u> FIFO %u\n", HWA3b, __FUNCTION__, core, fifo_idx));

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_TX_CORES);
	HWA_ASSERT(fifo_idx < HWA_TX_FIFOS);

	if (!dev) {
		return;
	}

	// Setup locals
	regs = dev->regs;
	txfifo = &dev->txfifo;

	if (!dev->up) {
		return;
	}

	if (isset(txfifo->fifo_enab, fifo_idx)) {
		// Use register interface to program hwa_txfifo_fifoctx_t in AXI memory
		HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_index, fifo_idx);

		// PKT FIFO context: hwa_txfifo_fifoctx::pkt_fifo
		// hwa_txfifo_fifoctx::pkt_fifo.curr_ptr
		u32 = txfifo->fifo_base[fifo_idx].loaddr & 0xFFFF;
		HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_rd_index, u32);
		// hwa_txfifo_fifoctx::pkt_fifo.last_ptr
		u32 = txfifo->fifo_base[fifo_idx].loaddr & 0xFFFF;
		HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_wr_index, u32);
		// hwa_txfifo_fifoctx::pkt_fifo.depth
		// We have to reconfig depth field to trigger HWA update
		u32 = txfifo->fifo_depth[fifo_idx];
		HWA_WR_REG_NAME(HWA3b, regs, txdma, fifo_depth, u32);

		// AQM FIFO context: hwa_txfifo_fifoctx::aqm_fifo
		// hwa_txfifo_fifoctx::aqm_fifo.curr_ptr
		u32 = txfifo->aqm_fifo_base[fifo_idx].loaddr & 0xFFFF;
		HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_rd_index, u32);
		// hwa_txfifo_fifoctx::aqm_fifo.last_ptr
		u32 = txfifo->aqm_fifo_base[fifo_idx].loaddr & 0xFFFF;
		HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_wr_index, u32);
		// hwa_txfifo_fifoctx::aqm_fifo.depth
		// We have to reconfig depth field to trigger HWA update
		u32 = txfifo->aqm_fifo_depth[fifo_idx];
		HWA_WR_REG_NAME(HWA3b, regs, txdma, aqm_depth, u32);
	}
}

uint // Get TxFIFO's active descriptor count
hwa_txfifo_dma_active(struct hwa_dev *dev, uint32 core, uint32 fifo_idx)
{
	hwa_txfifo_shadow32_t *shadow32;

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_TX_CORES);
	HWA_ASSERT(fifo_idx < HWA_TX_FIFOS);

	if (!dev)
		return 0;

	// Read txfifo shadow packet count
	shadow32 = (hwa_txfifo_shadow32_t *)dev->txfifo.txfifo_shadow;
	return shadow32[fifo_idx].pkt_count;
}

void // Reclaim MAC Tx DMA posted packets
hwa_txfifo_dma_reclaim(struct hwa_dev *dev, uint32 core)
{
	hwa_txfifo_t *txfifo;
	hwa_ring_t *pktchain_ring; // S2H pktchain ring context
	wlc_info_t *wlc;
	uint i;
	int16 pktcnt;

	HWA_FTRACE(HWA3b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	txfifo = &dev->txfifo;
	pktchain_ring = &txfifo->pktchain_ring;
	wlc = (wlc_info_t *)dev->wlc;

	for (i = 0; i < WLC_HW_NFIFO_INUSE(wlc); i++) {
		/* free any posted tx packets */
		pktcnt = hwa_txfifo_shadow_reclaim(dev, 0,
			WLC_HW_MAP_TXFIFO(wlc, i));
		if (pktcnt > 0) {
			WLC_TXFIFO_COMPLETE(wlc, i, pktcnt, 0);
			HWA_ERROR(("%s reclaim fifo %d pkts %d\n", HWA3b, i, pktcnt));
		}

		pktcnt = TXPKTPENDGET(wlc, i);
		if (pktcnt > 0) {
			HWA_ERROR(("%s fifo %d REMAINS %d pkts\n", HWA3b, i, pktcnt));
		}
		HWA_TRACE(("%s pktpend fifo %d cleared\n", HWA3b, i));
	}

	// 3B reset impacts 3A/4A HWA internal memory regions, so we need to
	// make sure 3A/4A DMA jobs are done.  Here the MAC has suspended.
	// Wait until 3A finish schedcmd and txfree_ring
	HWA_TXPOST_EXPR(hwa_txpost_wait_to_finish(&dev->txpost));

	// Make sure 4A has finished it's job.
	HWA_TXSTAT_EXPR(hwa_txstat_wait_to_finish(&dev->txstat, 0));

	// Reset 3b block
	hwa_module_request(dev, HWA_MODULE_TXFIFO, HWA_MODULE_RESET, TRUE);
	hwa_module_request(dev, HWA_MODULE_TXFIFO, HWA_MODULE_RESET, FALSE);

	// Reset S2H Packet Chain ring
	HWA_RING_STATE(pktchain_ring)->read = 0;
	HWA_RING_STATE(pktchain_ring)->write = 0;

}

static int16 // Free pkts of TxFIFOs shadow context
hwa_txfifo_shadow_reclaim(hwa_dev_t *dev, uint32 core, uint32 fifo_idx)
{
	void *p;
	int16 cnt;

	HWA_FTRACE(HWA3b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	cnt = 0;

	while ((p = hwa_txfifo_getnexttxp32(dev, fifo_idx, HNDDMA_RANGE_ALL))) {
		PKTFREE(dev->osh, p, TRUE);
		cnt++;
	}

	return cnt;
}

void // Clear OvflowQ pkt_count, mpdu_count to avoid 3b process reclaimed packets
hwa_txfifo_clear_ovfq(struct hwa_dev *dev, uint32 core, uint32 fifo_idx)
{
	hwa_txfifo_t *txfifo;
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;
	hwa_txfifo_ovflwqctx_t ovflwq;

	txfifo = &dev->txfifo;
	sys_mem = &ovflwq.u32[0];
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);

	if (fifo_idx >= txfifo->fifo_total || !isset(txfifo->fifo_enab, fifo_idx)) {
		HWA_ERROR(("%s: Wrong argument fifo_idx<%u:%u>\n",
			__FUNCTION__, fifo_idx, txfifo->fifo_total));
		return;
	}

	// Clear pkt_count
	/* NOTE: We need to update one full context (28-bytes at least)
	 * to trigger HWA update
	 */
	axi_mem_addr = HWA_TABLE_ADDR(hwa_txfifo_ovflwqctx_t,
		txfifo->ovflwq_addr, fifo_idx);
	HWA_RD_MEM32(HWA3b, hwa_txfifo_ovflwqctx_t, axi_mem_addr, sys_mem);
	if (ovflwq.pkt_count) {
		// Add pkt_count to append_count to make it sync with shadow pkt_count
		ovflwq.append_count += ovflwq.pkt_count;
		// Clear counts
		ovflwq.pkt_count = 0;
		ovflwq.mpdu_count = 0;
		HWA_WR_MEM32(HWA3b, hwa_txfifo_ovflwqctx_t, axi_mem_addr, sys_mem);
	}
}

static INLINE void __hwa_txfifo_pktchain32_post(hwa_txfifo_t *txfifo,
	uint32 fifo_idx, void *pktchain_head, void *pktchain_tail,
	uint16 pkt_count, uint16 mpdu_count);
static INLINE void __hwa_txfifo_pktchain64_post(hwa_txfifo_t *txfifo,
	uint32 fifo_idx, void *pktchain_head, void *pktchain_tail,
	uint16 pkt_count, uint16 mpdu_count);

static INLINE void
__hwa_txfifo_pktchain32_post(hwa_txfifo_t *txfifo,
	uint32 fifo_idx, void *pktchain_head, void *pktchain_tail,
	uint16 pkt_count, uint16 mpdu_count)
{
	hwa_txfifo_pktchain32_t *s2h_req32;
	hwa_ring_t *pktchain_ring;
	hwa_txfifo_shadow32_t *shadow32, *myshadow32;
	hwa_txfifo_pkt_t *txfifo_pkt;

	ASSERT(pktchain_head);
	ASSERT(pktchain_tail);
	ASSERT(pkt_count);
	ASSERT(mpdu_count);

	pktchain_ring = &txfifo->pktchain_ring;
	shadow32 = (hwa_txfifo_shadow32_t *)txfifo->txfifo_shadow;
	myshadow32 = &shadow32[fifo_idx];

	s2h_req32 = HWA_RING_PROD_ELEM(hwa_txfifo_pktchain32_t, pktchain_ring);
	s2h_req32->pkt_head   = HWA_PTR2UINT(pktchain_head);
	s2h_req32->pkt_tail   = HWA_PTR2UINT(pktchain_tail);
	s2h_req32->pkt_count  = pkt_count;
	s2h_req32->mpdu_count = mpdu_count;
	s2h_req32->fifo_idx   = fifo_idx;

	// Save in the shadow before update the write index.
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);
	if (myshadow32->pkt_head == NULL) {
		myshadow32->pkt_head = HWA_PTR2UINT(pktchain_head);
		myshadow32->pkt_tail = HWA_PTR2UINT(pktchain_tail);
		myshadow32->pkt_count = pkt_count;
		myshadow32->mpdu_count = mpdu_count;
	} else {
		/* SW MUST mantain the next pointer linking even in A0, we saw a
		 * case that 3b doesn't update the swpkt1 next to point to swpkt2
		 * becase 3b process swpkt1 from 3b chainQ to ovfQ to MAC DMA
		 * to fast so that swpkt1/swpkt2 never co-exist at ovfQ.
		 */
		txfifo_pkt = HWA_UINT2PTR(hwa_txfifo_pkt_t, myshadow32->pkt_tail);
		HWA_PKT_LINK_NEXT(txfifo_pkt, (hwa_txfifo_pkt_t *)pktchain_head);
		myshadow32->pkt_tail = HWA_PTR2UINT(pktchain_tail);
		myshadow32->pkt_count += pkt_count;
		myshadow32->mpdu_count += mpdu_count;
	}
	myshadow32->stats.pkt_count += pkt_count;
	myshadow32->stats.mpdu_count += mpdu_count;
} // __hwa_txfifo_pktchain32_post

static INLINE void
__hwa_txfifo_pktchain64_post(hwa_txfifo_t *txfifo,
	uint32 fifo_idx, void *pktchain_head, void *pktchain_tail,
	uint16 pkt_count, uint16 mpdu_count)
{
	HWA_ASSERT(0);
	HWA_ERROR(("XXX: %s pktchain64 not ready\n", HWA3b));
} // __hwa_txfifo_pktchain64_post

#if (HWA_DEBUG_BUILD >= 1) && (defined(BCMDBG) || 0)
static void
_hwa_txfifo_dump_pkt(hwa_txfifo_pkt_t *pkt, struct bcmstrbuf *b,
	const char *title, bool one_shot)
{
	uint32 pkt_index = 0;
	hwa_txfifo_pkt_t *curr = pkt;

	HWA_ASSERT(pkt);

	while (curr) {
		pkt_index++;
		HWA_BPRINT(b, " [%s] 3b:swpkt-%d at <%p> lbuf <%p>\n", title, pkt_index, curr,
			HWAPKT2LFRAG((char *)curr));
		HWA_BPRINT(b, "    <%p>:\n", curr);
		HWA_BPRINT(b, "           next <%p>\n", curr->next);
		HWA_BPRINT(b, "           daddr <0x%x>\n", curr->hdr_buf_daddr32);
		HWA_BPRINT(b, "           numdesc <%u>\n", curr->num_desc);
		HWA_BPRINT(b, "           dlen <%u>\n", curr->hdr_buf_dlen);
		HWA_BPRINT(b, "           amsdulen <%u>\n", curr->amsdu_total_len);
		HWA_BPRINT(b, "           hlen <%u>\n", curr->data_buf_hlen);
		HWA_BPRINT(b, "           haddr<0x%08x,0x%08x>\n", curr->data_buf_haddr.loaddr,
			curr->data_buf_haddr.hiaddr);
		HWA_BPRINT(b, "           txdmaflags <0x%x>\n", curr->txdma_flags);
		if (one_shot)
			break;
		curr = curr->next;
	}
}

static void
_hwa_txfifo_dump_shadow32(hwa_txfifo_t *txfifo, struct bcmstrbuf *b,
	uint32 fifo_idx, bool dump_txfifo_shadow)
{
	hwa_txfifo_pkt_t *head;
	hwa_txfifo_shadow32_t *shadow32, *myshadow32;

	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);

	shadow32 = (hwa_txfifo_shadow32_t *)txfifo->txfifo_shadow;
	myshadow32 = &shadow32[fifo_idx];

	HWA_BPRINT(b, "+ %2u txshadow32 head<0x%08x> tail<0x%08x> count<%u,%u> "
		"stats.count<%u,%u>\n", fifo_idx,
		myshadow32->pkt_head, myshadow32->pkt_tail,
		myshadow32->pkt_count, myshadow32->mpdu_count,
		myshadow32->stats.pkt_count, myshadow32->stats.mpdu_count);

	// Save in the shadow before update the write index.
	head = HWA_UINT2PTR(hwa_txfifo_pkt_t, myshadow32->pkt_head);
	if (head == NULL)
		return;

	if (dump_txfifo_shadow) {
		_hwa_txfifo_dump_pkt(head, b, "shadow32", FALSE);
	}
}
#endif /* HWA_DEBUG_BUILD >= 1 */

#if HWA_REVISION_EQ_128
/* SW WAR to handle frameid mismatch type2 in A0 */
void *
hwa_txfifo_picknext2txp32(struct hwa_dev *dev, uint32 fifo_idx)
{
	uint16 mpdu_count;
	hwa_txfifo_t *txfifo;
	hwa_txfifo_shadow32_t *shadow32, *myshadow32;
	hwa_txfifo_pkt_t *txfifo_pkt, *curr;

	HWA_PRINT("%s: fifo <%u>\n", __FUNCTION__, fifo_idx);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);

	txfifo = &dev->txfifo;
	shadow32 = (hwa_txfifo_shadow32_t *)txfifo->txfifo_shadow;
	myshadow32 = &shadow32[fifo_idx];
	txfifo_pkt = HWA_UINT2PTR(hwa_txfifo_pkt_t, myshadow32->pkt_head);

	if (txfifo_pkt == NULL)
		return NULL;

	//The num_desc in pkt_head should not be 0
	ASSERT(txfifo_pkt->num_desc > 0);

	//Find the next non-zero num_desc 3b-SWPTK
	curr = txfifo_pkt;
	mpdu_count = 0;
	while (curr) {
		if (curr->num_desc != 0)
			mpdu_count++;
		if (mpdu_count == 2)
			return (void*)HWAPKT2LFRAG((char *)curr);
		curr = curr->next;
	}

	return NULL;
}
#endif /* HWA_REVISION_EQ_128 */

void * // Provide the TXed packet
hwa_txfifo_getnexttxp32(struct hwa_dev *dev, uint32 fifo_idx, uint32 range)
{
	uint16 pkt_count;
	hwa_txfifo_t *txfifo;
	hwa_txfifo_shadow32_t *shadow32, *myshadow32;
	hwa_txfifo_pkt_t *txfifo_pkt, *curr;

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);

	txfifo = &dev->txfifo;
	shadow32 = (hwa_txfifo_shadow32_t *)txfifo->txfifo_shadow;
	myshadow32 = &shadow32[fifo_idx];
	txfifo_pkt = HWA_UINT2PTR(hwa_txfifo_pkt_t, myshadow32->pkt_head);
	if (txfifo_pkt == NULL) {
		if (range != HNDDMA_RANGE_ALL) {
			HWA_ERROR(("%s TX FIFO-%d shadow is empty\n",
				HWA3b, fifo_idx));
		}
		return NULL;
	}

	// For debug HWA3b update the swpkt next.
	//HWA_DEBUG_EXPR(_hwa_txfifo_dump_shadow32(txfifo, NULL, fifo_idx, TRUE));

	//The num_desc in pkt_head should not be 0
	ASSERT(txfifo_pkt->num_desc > 0);

	//Find the next non-zero num_desc 3b-SWPTK
	curr = txfifo_pkt;
	pkt_count = 1;
	while (curr->next && curr->next->num_desc == 0) {
		pkt_count++;
		curr = curr->next;
	}

	//Update shdow32[fifo_idx]
	if (curr->next == NULL) {
		ASSERT(myshadow32->pkt_count == pkt_count);
		ASSERT(myshadow32->mpdu_count == 1);
		myshadow32->pkt_head = NULL;
		myshadow32->pkt_tail = NULL;
		myshadow32->pkt_count = 0;
		myshadow32->mpdu_count = 0;
	} else {
		myshadow32->pkt_head = HWA_PTR2UINT(curr->next);
		myshadow32->pkt_count -= pkt_count;
		myshadow32->mpdu_count--;
	}

	// Set terminater of MPDU
	// for debug, don't clear it so that we can trace it.
	//HWA_PKT_TERM_NEXT(curr);

	HWA_TRACE(("%s Get a txp %p (swpkt@%p) from TX FIFO-%d shadow\n", HWA3b,
		(void*)HWAPKT2LFRAG((char *)txfifo_pkt), txfifo_pkt, fifo_idx));

	//HWA_DEBUG_EXPR(_hwa_txfifo_dump_pkt(txfifo_pkt, NULL, "getnexttxp32", FALSE));

	return (void*)HWAPKT2LFRAG((char *)txfifo_pkt);
}

void * // Like hwa_txfifo_getnexttxp32 but no reclaim
hwa_txfifo_peeknexttxp(struct hwa_dev *dev, uint32 fifo_idx)
{
	hwa_txfifo_t *txfifo;
	hwa_txfifo_shadow32_t *shadow32, *myshadow32;
	hwa_txfifo_pkt_t *txfifo_pkt;

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);

	txfifo = &dev->txfifo;
	shadow32 = (hwa_txfifo_shadow32_t *)txfifo->txfifo_shadow;
	myshadow32 = &shadow32[fifo_idx];
	txfifo_pkt = HWA_UINT2PTR(hwa_txfifo_pkt_t, myshadow32->pkt_head);
	if (txfifo_pkt == NULL)
		return NULL;

	//The num_desc in pkt_head should not be 0
	ASSERT(txfifo_pkt->num_desc > 0);

	return (void*)HWAPKT2LFRAG((char *)txfifo_pkt);
}

int // HWA txfifo map function.
hwa_txfifo_map_pkts(struct hwa_dev *dev, uint32 fifo_idx, void *cb, void *ctx)
{
	hwa_txfifo_t *txfifo;
	hwa_txfifo_shadow32_t *shadow32, *myshadow32;
	hwa_txfifo_pkt_t *txfifo_pkt, *curr;
	map_pkts_cb_fn map_pkts_cb = (map_pkts_cb_fn)cb;

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);

	txfifo = &dev->txfifo;
	shadow32 = (hwa_txfifo_shadow32_t *)txfifo->txfifo_shadow;
	myshadow32 = &shadow32[fifo_idx];
	txfifo_pkt = HWA_UINT2PTR(hwa_txfifo_pkt_t, myshadow32->pkt_head);

	if (txfifo_pkt == NULL)
		return BCME_OK;

	//The num_desc in pkt_head should not be 0
	ASSERT(txfifo_pkt->num_desc > 0);

	//Do map on each packets that num_desc is not 0
	curr = txfifo_pkt;
	while (curr) {
		if (curr->num_desc) {
			/* ignoring the return 'delete' bool since hwa
			 * does not allow deleting pkts on the ring.
			 */
			(void)map_pkts_cb(ctx, (void*)HWAPKT2LFRAG((char *)curr));
		}
		curr = curr->next;
	}

	return BCME_OK;
}

int // Handle a request from WLAN driver for transmission of a packet chain
hwa_txfifo_pktchain_xmit_request(struct hwa_dev *dev, uint32 core,
	uint32 fifo_idx, void *pktchain_head, void *pktchain_tail,
	uint16 pkt_count, uint16 mpdu_count)
{
	hwa_ring_t *pktchain_ring; // S2H pktchain ring context
	hwa_txfifo_t *txfifo;

	HWA_ASSERT(dev != (struct hwa_dev*)NULL);
	HWA_FTRACE(HWA3b);

	txfifo = &dev->txfifo;
	pktchain_ring = &txfifo->pktchain_ring;

	if (hwa_ring_is_full(pktchain_ring))
		goto failure;

	if (txfifo->pktchain_fmt == sizeof(hwa_txfifo_pktchain32_t))
		__hwa_txfifo_pktchain32_post(txfifo,
			fifo_idx, pktchain_head, pktchain_tail, pkt_count, mpdu_count);
	else
		__hwa_txfifo_pktchain64_post(txfifo,
			fifo_idx, pktchain_head, pktchain_tail, pkt_count, mpdu_count);

	hwa_ring_prod_upd(pktchain_ring, 1, TRUE); // update/commit WR

	HWA_TRACE(("%s xmit pktchain[%u,%u]"
		" fifo<%u> head<%p> tail<%p> pkt<%u> mpdu<%u>\n",
		HWA3b, HWA_RING_STATE(pktchain_ring)->write,
		HWA_RING_STATE(pktchain_ring)->read,
		fifo_idx, pktchain_head, pktchain_tail, pkt_count, mpdu_count));

#if defined(BCMDBG)
	HWA_DEBUG_EXPR(_hwa_txfifo_dump_shadow32(txfifo, NULL, fifo_idx, TRUE));
#endif // endif

	return txfifo->pktchain_id++;

failure:
	HWA_WARN(("%s xmit failure fifo<%u> head<%p> tail<%p> pkt<%u> mpdu<%u>\n",
		HWA3b, fifo_idx, pktchain_head, pktchain_tail, pkt_count, mpdu_count));

	return HWA_FAILURE;

} // hwa_txfifo_pktchain_xmit_request

// HWA3b TxFifo block statistics collection
static void _hwa_txfifo_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 core);

void // Clear statistics for HWA3b TxFifo block
hwa_txfifo_stats_clear(hwa_txfifo_t *txfifo, uint32 core)
{
	hwa_dev_t *dev;

	dev = HWA_DEV(txfifo);

	hwa_stats_clear(dev, HWA_STATS_TXDMA); // common

} // hwa_txfifo_stats_clear

void // Print the common statistics for HWA3b TxFifo block
_hwa_txfifo_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 core)
{
	hwa_txfifo_stats_t *txfifo_stats = &dev->txfifo.stats;
	struct bcmstrbuf *b = (struct bcmstrbuf *)buf;

#if HWA_REVISION_EQ_128
	HWA_BPRINT(b, "%s statistics TBD<%u>\n", HWA3b, txfifo_stats->TBD);
#else
	HWA_BPRINT(b, "%s statistics pktcq_empty<%u> ovfq_empty<%u> "
		"ovfq_full<%u> stall<%u>\n", HWA3b,
		txfifo_stats->pktc_empty, txfifo_stats->ovf_empty,
		txfifo_stats->ovf_full, txfifo_stats->pull_fsm_stall);
#endif // endif

} // _hwa_txfifo_stats_dump

void // Query and dump common statistics for HWA3b TxFifo block
hwa_txfifo_stats_dump(hwa_txfifo_t *txfifo, struct bcmstrbuf *b, uint8 clear_on_copy)
{
	hwa_dev_t *dev;

	dev = HWA_DEV(txfifo);

	hwa_txfifo_stats_t *txfifo_stats = &txfifo->stats;
	hwa_stats_copy(dev, HWA_STATS_TXDMA,
		HWA_PTR2UINT(txfifo_stats), HWA_PTR2HIADDR(txfifo_stats),
		/* num_sets */ 1, clear_on_copy, &_hwa_txfifo_stats_dump,
		(uintptr)b, 0U);

} // hwa_txfifo_stats_dump

#if defined(BCMDBG)

void // Dump HWA3b state
hwa_txfifo_state(hwa_dev_t *dev)
{
	hwa_regs_t *regs;

	HWA_AUDIT_DEV(dev);
	regs = dev->regs;

	HWA_PRINT("%s state sts<0x%08x. sts2<0x%08x> sts3<0x%08x>\n", HWA3b,
		HWA_RD_REG_NAME(HWA3b, regs, txdma, state_sts),
		HWA_RD_REG_NAME(HWA3b, regs, txdma, state_sts2),
		HWA_RD_REG_NAME(HWA3b, regs, txdma, state_sts3));

} // hwa_txfifo_state

void
hwa_txfifo_dump_ovfq(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 fifo_idx)
{
	hwa_txfifo_t *txfifo;
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;
	hwa_txfifo_ovflwqctx_t ovflwq;

	txfifo = &dev->txfifo;
	sys_mem = &ovflwq.u32[0];
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);

	if (fifo_idx >= txfifo->fifo_total || !isset(txfifo->fifo_enab, fifo_idx)) {
		HWA_BPRINT(b, "Wrong argument fifo_idx<%u:%u>\n",
			fifo_idx, txfifo->fifo_total);
		return;
	}

	axi_mem_addr = HWA_TABLE_ADDR(hwa_txfifo_ovflwqctx_t,
		txfifo->ovflwq_addr, fifo_idx);
	HWA_RD_MEM32(HWA3b, hwa_txfifo_ovflwqctx_t,
		axi_mem_addr, sys_mem);
	HWA_BPRINT(b, "+ %2u ovflwq head<0x%08x,0x%08x> tail<0x%08x,0x%08x>"
		" count<%u,%u> hwm<%u,%u> append<%u>\n", fifo_idx,
		ovflwq.pktq_head.hiaddr, ovflwq.pktq_head.loaddr,
		ovflwq.pktq_tail.hiaddr, ovflwq.pktq_tail.loaddr,
		ovflwq.pkt_count, ovflwq.mpdu_count,
		ovflwq.pkt_count_hwm, ovflwq.mpdu_count_hwm,
		ovflwq.append_count);
}

/* NOTE:  Accress fifoctx through AXI is dangerous,
 * it could be AXI timeout.  Only for debug.
 */
void
hwa_txfifo_dump_fifoctx(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 fifo_idx)
{
	hwa_txfifo_t *txfifo;
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;
	hwa_txfifo_fifoctx_t fifoctx;

	txfifo = &dev->txfifo;
	sys_mem = &fifoctx.u32[0];
	HWA_ASSERT(fifo_idx < HWA_TX_FIFOS);

	if (fifo_idx >= txfifo->fifo_total || !isset(txfifo->fifo_enab, fifo_idx)) {
		HWA_BPRINT(b, "Wrong argument fifo_idx<%u:%u>\n",
			fifo_idx, txfifo->fifo_total);
		return;
	}

	// Don't access txfifoctx when 3B is disable
	if (hwa_module_request(dev, HWA_MODULE_TXFIFO, HWA_MODULE_IDLE, TRUE)) {
		HWA_BPRINT(b, "+ Ignore fifoctx (3B is disabled)\n");
		return;
	}

	axi_mem_addr = HWA_TABLE_ADDR(hwa_txfifo_fifoctx_t,
		txfifo->txfifo_addr, fifo_idx);
	HWA_RD_MEM32(HWA3b, hwa_txfifo_fifoctx_t,
		axi_mem_addr, sys_mem);
	HWA_BPRINT(b, "+ %2u fifoctx-tx  base<0x%08x,0x%08x> "
		"curr_ptr<0x%04x> last_ptr<0x%04x> attrib<%u> depth<%u>\n", fifo_idx,
		fifoctx.pkt_fifo.base.hiaddr, fifoctx.pkt_fifo.base.loaddr,
		fifoctx.pkt_fifo.curr_ptr, fifoctx.pkt_fifo.last_ptr,
		fifoctx.pkt_fifo.attrib, fifoctx.pkt_fifo.depth);
	HWA_BPRINT(b, "+ %2u fifoctx-aqm base<0x%08x,0x%08x> "
		"curr_ptr<0x%04x> last_ptr<0x%04x> attrib<%u> depth<%u>\n", fifo_idx,
		fifoctx.aqm_fifo.base.hiaddr, fifoctx.aqm_fifo.base.loaddr,
		fifoctx.aqm_fifo.curr_ptr, fifoctx.aqm_fifo.last_ptr,
		fifoctx.aqm_fifo.attrib, fifoctx.aqm_fifo.depth);
}

void
hwa_txfifo_dump_shadow(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 fifo_idx)
{
	hwa_txfifo_t *txfifo;
	hwa_txfifo_shadow32_t *shadow32;
	hwa_txfifo_shadow32_t *myshadow32;

	txfifo = &dev->txfifo;
	shadow32 = (hwa_txfifo_shadow32_t *)txfifo->txfifo_shadow;
	HWA_ASSERT(fifo_idx < HWA_OVFLWQ_MAX);
	myshadow32 = &shadow32[fifo_idx];

	if (fifo_idx >= txfifo->fifo_total || !isset(txfifo->fifo_enab, fifo_idx)) {
		HWA_BPRINT(b, "Wrong argument fifo_idx<%u:%u>\n",
			fifo_idx, txfifo->fifo_total);
		return;
	}

	HWA_BPRINT(b, "+ %2u txshadow: head<0x%08x> tail<0x%08x> "
		"count<%u,%u> stats.count<%u,%u>\n", fifo_idx,
		myshadow32->pkt_head, myshadow32->pkt_tail,
		myshadow32->pkt_count, myshadow32->mpdu_count,
		myshadow32->stats.pkt_count, myshadow32->stats.mpdu_count);
}

void // HWA3b TxFifo: debug dump
hwa_txfifo_dump(hwa_txfifo_t *txfifo, struct bcmstrbuf *b,
	bool verbose, bool dump_regs, bool dump_txfifo_shadow,
	uint8 *fifo_bitmap)
{
	hwa_dev_t *dev;

	dev = HWA_DEV(txfifo);

	HWA_BPRINT(b, "%s dump<%p>\n", HWA3b, txfifo);

	if (txfifo == (hwa_txfifo_t*)NULL)
		return;

	hwa_ring_dump(&txfifo->pktchain_ring, b, "+ pktchain");
	HWA_BPRINT(b, "+ requests<%u> fmt<%u>\n",
		txfifo->pktchain_id, txfifo->pktchain_fmt);
	HWA_BPRINT(b, "+ TxFIFO config<%u> total<%u>\n+ TxFIFO Enabled:\n",
		txfifo->fifo_config, txfifo->fifo_total);

#if defined(WLTEST)
	if (dump_regs == TRUE)
		hwa_txfifo_regs_dump(txfifo, b);
#endif // endif

	HWA_STATS_EXPR(
		HWA_BPRINT(b, "+ pktchain<%u> pkt_xmit<%u> tx_desc<%u>\n",
		          txfifo->pktchain_cnt, txfifo->pkt_xmit_cnt,
		          txfifo->tx_desc_cnt));

	if (verbose == TRUE) {
		hwa_txfifo_stats_dump(txfifo, b, /* clear */ 0);
	}

	if (verbose) {
		uint32 idx;
		uint32 *sys_mem;
		hwa_mem_addr_t axi_mem_addr;
		hwa_txfifo_ovflwqctx_t ovflwq;
		hwa_txfifo_fifoctx_t fifoctx;

		HWA_BPRINT(b, "+ Overflow Queue Context total<%u>\n", txfifo->fifo_total);
		sys_mem = &ovflwq.u32[0];
		for (idx = 0; idx < txfifo->fifo_total; idx++) {
			if (fifo_bitmap && !isset(fifo_bitmap, idx))
				continue;

			if (isset(txfifo->fifo_enab, idx)) {
				axi_mem_addr = HWA_TABLE_ADDR(hwa_txfifo_ovflwqctx_t,
					txfifo->ovflwq_addr, idx);
				HWA_RD_MEM32(HWA3b, hwa_txfifo_ovflwqctx_t,
					axi_mem_addr, sys_mem);
				HWA_BPRINT(b, "+ %2u ovflwq head<0x%08x,0x%08x> tail<0x%08x,0x%08x>"
					" count<%u,%u> hwm<%u,%u> append<%u>\n", idx,
					ovflwq.pktq_head.hiaddr, ovflwq.pktq_head.loaddr,
					ovflwq.pktq_tail.hiaddr, ovflwq.pktq_tail.loaddr,
					ovflwq.pkt_count, ovflwq.mpdu_count,
					ovflwq.pkt_count_hwm, ovflwq.mpdu_count_hwm,
					ovflwq.append_count);
				HWA_STATS_EXPR(HWA_BPRINT(b, " req<%u>\n", txfifo->req_cnt[idx]));
			}
		}

		// dump txfifo/aqm context
		HWA_BPRINT(b, "+ TxFIFO and AQM Context total<%u>\n", txfifo->fifo_total);
		// Don't access txfifoctx when 3B is disable
		if (hwa_module_request(dev, HWA_MODULE_TXFIFO, HWA_MODULE_IDLE, TRUE)) {
			HWA_BPRINT(b, "+  Ignore fifoctx (3B is disabled)\n");
			goto dump_shadow;
		}
		sys_mem = &fifoctx.u32[0];
		for (idx = 0; idx < txfifo->fifo_total; idx++) {
			if (fifo_bitmap && !isset(fifo_bitmap, idx))
				continue;

			if (isset(txfifo->fifo_enab, idx)) {
				axi_mem_addr = HWA_TABLE_ADDR(hwa_txfifo_fifoctx_t,
					txfifo->txfifo_addr, idx);
				HWA_RD_MEM32(HWA3b, hwa_txfifo_fifoctx_t,
					axi_mem_addr, sys_mem);
				HWA_BPRINT(b, "+  TX%2u fifoctx base<0x%08x,0x%08x> "
					"curr_ptr<0x%04x> last_ptr<0x%04x> attrib<%u> depth<%u>\n",
					idx,
					fifoctx.pkt_fifo.base.hiaddr, fifoctx.pkt_fifo.base.loaddr,
					fifoctx.pkt_fifo.curr_ptr, fifoctx.pkt_fifo.last_ptr,
					fifoctx.pkt_fifo.attrib, fifoctx.pkt_fifo.depth);
				HWA_BPRINT(b, "+ AQM%2u fifoctx base<0x%08x,0x%08x> "
					"curr_ptr<0x%04x> last_ptr<0x%04x> attrib<%u> depth<%u>\n",
					idx,
					fifoctx.aqm_fifo.base.hiaddr, fifoctx.aqm_fifo.base.loaddr,
					fifoctx.aqm_fifo.curr_ptr, fifoctx.aqm_fifo.last_ptr,
					fifoctx.aqm_fifo.attrib, fifoctx.aqm_fifo.depth);
			}
		}

dump_shadow:
		// dump shadow
		HWA_BPRINT(b, "+ Tx Shadow32 total<%u>\n", txfifo->fifo_total);
		for (idx = 0; idx < txfifo->fifo_total; idx++) {
			if (fifo_bitmap && !isset(fifo_bitmap, idx))
				continue;

			if (isset(txfifo->fifo_enab, idx)) {
				_hwa_txfifo_dump_shadow32(txfifo, b, idx, dump_txfifo_shadow);
			}
		}
	}
} // hwa_txfifo_dump

#if defined(WLTEST)

void // HWA3b TxFifo: dump block registers
hwa_txfifo_regs_dump(hwa_txfifo_t *txfifo, struct bcmstrbuf *b)
{
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	if (txfifo == (hwa_txfifo_t*)NULL)
		return;

	dev = HWA_DEV(txfifo);
	regs = dev->regs;

	HWA_BPRINT(b, "%s registers[0x%p] offset[0x%04x]\n",
		HWA3b, &regs->txdma, OFFSETOF(hwa_regs_t, txdma));

#if HWA_REVISION_EQ_128
	HWA_BPR_REG(b, txdma, txd_ctrl);
	// HWA-1.0 txd_host_addr_h, txd_rinfo_XXX, txd_cache_XXX,
	// txd_pktinfo_XXX, txd_cacheinfo_XXX
	HWA_BPR_REG(b, txdma, mac_txd_bm_pool_base_addr_l);
	HWA_BPR_REG(b, txdma, mac_txd_bm_config);
	HWA_BPR_REG(b, txdma, mac_txd_bm_pool_avail_count_sts);
	// HWA-1.0 eth_type_out[0..3]
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_base_addr_l);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_wr_index);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_rd_index);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_ctrl);
	HWA_BPR_REG(b, txdma, fifo_index);
	HWA_BPR_REG(b, txdma, fifo_base_addr);
	HWA_BPR_REG(b, txdma, fifo_wr_index);
	HWA_BPR_REG(b, txdma, fifo_depth);
	HWA_BPR_REG(b, txdma, sw_pkt_size);
	HWA_BPR_REG(b, txdma, dma_desc_template_txdma);
	HWA_BPR_REG(b, txdma, state_sts);
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg1);
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg2);
	HWA_BPR_REG(b, txdma, ovflowq_base_addr_lo);
	HWA_BPR_REG(b, txdma, ovflowq_base_addr_hi);
	HWA_BPR_REG(b, txdma, num_txfifo_percore);
	HWA_BPR_REG(b, txdma, fifo_rd_index);
	HWA_BPR_REG(b, txdma, fifo_attrib);
	HWA_BPR_REG(b, txdma, fifo_base_addrhi);
	HWA_BPR_REG(b, txdma, aqm_base_addr_lo);
	HWA_BPR_REG(b, txdma, aqm_base_addr_hi);
	HWA_BPR_REG(b, txdma, aqm_wr_index);
	HWA_BPR_REG(b, txdma, aqm_depth);
	HWA_BPR_REG(b, txdma, aqm_rd_index);
	HWA_BPR_REG(b, txdma, aqm_attrib);
	HWA_BPR_REG(b, txdma, state_sts2);
	HWA_BPR_REG(b, txdma, state_sts3);
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg3);
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg4);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_base_addr_h);
	HWA_BPR_REG(b, txdma, sw_tx_pkt_nxt_h);
#else  /* HWA_REVISION_GE_129 */
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg1);
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg2);
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg3);
	HWA_BPR_REG(b, txdma, hwa_txdma2_cfg4);
	HWA_BPR_REG(b, txdma, state_sts);
	HWA_BPR_REG(b, txdma, state_sts2);
	HWA_BPR_REG(b, txdma, state_sts3);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_base_addr_lo);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_base_addr_hi);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_wr_index);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_rd_index);
	HWA_BPR_REG(b, txdma, sw2hwa_tx_pkt_chain_q_ctrl);
	HWA_BPR_REG(b, txdma, fifo_index);
	HWA_BPR_REG(b, txdma, fifo_base_addr_lo);
	HWA_BPR_REG(b, txdma, fifo_base_addr_hi);
	HWA_BPR_REG(b, txdma, fifo_wr_index);
	HWA_BPR_REG(b, txdma, fifo_rd_index);
	HWA_BPR_REG(b, txdma, fifo_depth);
	HWA_BPR_REG(b, txdma, fifo_attrib);
	HWA_BPR_REG(b, txdma, aqm_base_addr_lo);
	HWA_BPR_REG(b, txdma, aqm_base_addr_hi);
	HWA_BPR_REG(b, txdma, aqm_wr_index);
	HWA_BPR_REG(b, txdma, aqm_rd_index);
	HWA_BPR_REG(b, txdma, aqm_depth);
	HWA_BPR_REG(b, txdma, aqm_attrib);
	HWA_BPR_REG(b, txdma, sw_tx_pkt_nxt_h);
	HWA_BPR_REG(b, txdma, dma_desc_template_txdma);
#endif /* HWA_REVISION_GE_129 */
} // hwa_txfifo_regs_dump

#endif // endif

#endif /* BCMDBG */

#endif /* HWA_TXFIFO_BUILD */

#ifdef HWA_TXSTAT_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: HWA4a TxSTAT block bringup: attach, detach, and init phases
 * -----------------------------------------------------------------------------
 */

static int hwa_txstat_bmac_proc(void *context, uintptr arg1, uintptr arg2,
	uint32 core, uint32 arg4);
static int hwa_txstat_bmac_done(void *context, uintptr arg1, uintptr arg2,
	uint32 core, uint32 rxfifo_cnt);

// HWA4a TxStat block level management
void
BCMATTACHFN(hwa_txstat_detach)(hwa_txstat_t *txstat) // txstat may be NULL
{
	void *memory;
	uint32 core, mem_sz;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA4a);

	if (txstat == (hwa_txstat_t*)NULL)
		return; // nothing to release done

	// Audit pre-conditions
	dev = HWA_DEV(txstat);

	// HWA4a TxStat Status Ring: free memory and reset ring
	for (core = 0; core < HWA_TX_CORES_MAX; core++) {
		if (txstat->status_ring[core].memory != (void*)NULL) {
			HWA_ASSERT(txstat->status_size[core] != 0U);
			memory = txstat->status_ring[core].memory;
			mem_sz = txstat->status_ring[core].depth
			         * txstat->status_size[core];
			HWA_TRACE(("%s status_ring[%u] -memory[%p:%u]\n",
				HWA4a, core, memory, mem_sz));
			MFREE(dev->osh, memory, mem_sz);
			hwa_ring_fini(&txstat->status_ring[core]);
			txstat->status_ring[core].memory = (void*)NULL;
			txstat->status_size[core] = 0U;
		}
	}

	return;

} // hwa_txstat_detach

hwa_txstat_t *
BCMATTACHFN(hwa_txstat_attach)(hwa_dev_t *dev)
{
	void *memory;
	uint32 v32, core, mem_sz, depth;
	hwa_regs_t *regs;
	hwa_txstat_t *txstat;

	HWA_FTRACE(HWA4a);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	regs = dev->regs;
	txstat = &dev->txstat;

	// Allocate and initialize the H2S txstatus interface
	for (core = 0; core < HWA_TX_CORES; core++) {
		depth = HWA_TXSTAT_QUEUE_DEPTH;
		mem_sz = depth * sizeof(hwa_txstat_status_t);
		if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
			HWA_ERROR(("%s status_ring<%u> malloc size<%u> failure\n",
				HWA4a, core, mem_sz));
			HWA_ASSERT(memory != (void*)NULL);
			goto failure;
		}
		HWA_TRACE(("%s status_ring<%u> +memory[%p,%u]\n",
			HWA4a, core, memory, mem_sz));
		v32 = HWA_PTR2UINT(memory);
		HWA_WR_REG_NAME(HWA4a, regs, tx_status[core], tseq_base_lo, v32);
		v32 = HWA_PTR2HIADDR(memory); // hiaddr for NIC mode 64b hosts
		HWA_WR_REG_NAME(HWA4a, regs, tx_status[core], tseq_base_hi, v32);
		hwa_ring_init(&txstat->status_ring[core], "TXS", HWA_TXSTAT_ID,
			HWA_RING_H2S, HWA_TXSTAT_QUEUE_H2S_RINGNUM, depth, memory,
			&regs->tx_status[core].tseq_wridx,
			&regs->tx_status[core].tseq_rdidx);
		v32 = HWA_TXSTAT_QUEUE_DEPTH;
		HWA_WR_REG_NAME(HWA4a, regs, tx_status[core], tseq_size, v32);
		v32 = (0U
			| BCM_SBF(HWA_TXSTAT_RING_ELEM_SIZE,
			        HWA_TX_STATUS_TSE_CTL_MACTXSTATUSSIZE)
			| BCM_SBF(HWA_TXSTAT_INTRAGGR_TMOUT,
			        HWA_TX_STATUS_TSE_CTL_LAZYINTRTIMEOUT)
			| BCM_SBF(HWA_TXSTAT_INTRAGGR_COUNT,
			         HWA_TX_STATUS_TSE_CTL_LAZYCOUNT)
			| 0U);
		HWA_WR_REG_NAME(HWA4a, regs, tx_status[core], tse_ctl, v32);

	} // for core

	// Registered dpc callback handler
	hwa_register(dev, HWA_TXSTAT_PROC, dev, hwa_txstat_bmac_proc);
	hwa_register(dev, HWA_TXSTAT_DONE, dev, hwa_txstat_bmac_done);

	return txstat;

failure:
	hwa_txstat_detach(txstat);
	HWA_WARN(("%s attach failure\n", HWA4a));

	return ((hwa_txstat_t*)NULL);

} // hwa_txstat_attach

void // HWA4a TxStatus block initialization
hwa_txstat_init(hwa_txstat_t *txstat)
{
	uint32 v32, core;
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	uint16 v16;

	HWA_FTRACE(HWA4a);

	// Audit pre-conditions
	dev = HWA_DEV(txstat);

	// Check initialization.
	if (dev->inited)
		goto done;

	// Setup locals
	regs = dev->regs;

	for (core = 0; core < HWA_TX_CORES; core++) {
		v32 = (0U
		// PCIE as source | destination: NotPcie = 0, Coh = host, AddrExt = 0b00
		//| BCM_SBIT(HWA_TX_STATUS_DMA_DESC_TEMPLATE_TXS_DMAPCIEDESCTEMPLATENOTPCIE)
		| BCM_SBF(dev->host_coherency,
			HWA_TX_STATUS_DMA_DESC_TEMPLATE_TXS_DMAPCIEDESCTEMPLATECOHERENT)
		| BCM_SBF(0U,
			HWA_TX_STATUS_DMA_DESC_TEMPLATE_TXS_DMAPCIEDESCTEMPLATEADDREXT)
		//| BCM_SBIT(HWA_TX_STATUS_DMA_DESC_TEMPLATE_TXS_DMAHWADESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_TX_STATUS_DMA_DESC_TEMPLATE_TXS_DMAHWADESCTEMPLATECOHERENT)
		//| BCM_SBIT(HWA_TX_STATUS_DMA_DESC_TEMPLATE_TXS_NONDMAHWADESCTEMPLATECOHERENT)
		| 0U);
		HWA_WR_REG_NAME(HWA4a, regs, tx_status[core],
			dma_desc_template_txs, v32);

#if HWA_REVISION_GE_129
		//CRWLHWA-454
		//Access mode
		//0: get MAC TX status with APB interface. 1: get MAX TX status with AXI interface.
		v32 = BCM_SBF((dev->txstat_apb == 0) ? 1 : 0,
			HWA_TX_STATUS_TXE_CFG1_ACCESSMODE);
		HWA_WR_REG_NAME(HWA4a, regs, tx_status[core], txe_cfg1, v32);
#endif /* HWA_REVISION_GE_129 */
	} // for core

done:

	// Enable Req-Ack based MAC_HWA i/f is enabled for txStatus.
	v16 = HWA_RD_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl);
	v16 |= BCM_SBIT(_HWA_MACIF_CTL_TXSTATUSEN);
	v16 |= BCM_SBF(HWA_TXSTAT_RING_ELEM_SIZE/HWA_TXSTAT_PKG_SIZE,
		_HWA_MACIF_CTL_TXSTATUS_COUNT);
#if HWA_REVISION_GE_129
	//CRWLHWA-454:
	//0: get MAC TX status with APB interface. 1: get MAX TX status with AXI interface.
	if (dev->txstat_apb == 0) {
		v16 |= BCM_SBIT(_HWA_MACIF_CTL_TXSTATUSMEM_AXI);
	} else {
		v16 = BCM_CBIT(v16, _HWA_MACIF_CTL_TXSTATUSMEM_AXI);
	}
#endif /* HWA_REVISION_GE_129 */
	HWA_WR_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl, v16);

	// Assign the txstat interrupt mask.
	dev->defintmask |= HWA_COMMON_INTSTATUS_TXSWRINDUPD_INT_CORE0_MASK;
	if (HWA_TX_CORES > 1)
		dev->defintmask |= HWA_COMMON_INTSTATUS_TXSWRINDUPD_INT_CORE1_MASK;

} // hwa_txstat_init

void // HWA4a TxStatus block deinitialization
hwa_txstat_deinit(hwa_txstat_t *txstat)
{
	hwa_dev_t *dev;
	uint16 v16;

	HWA_FTRACE(HWA4a);

	// Audit pre-conditions
	dev = HWA_DEV(txstat);

	// Disable Req-Ack based MAC_HWA i/f for txStatus.
	v16 = HWA_RD_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl);
	v16 = BCM_CBIT(v16, _HWA_MACIF_CTL_TXSTATUSEN);
	HWA_WR_REG16_ADDR(HWA4a, &dev->mac_regs->hwa_macif_ctl, v16);

}

void
hwa_txstat_wait_to_finish(hwa_txstat_t *txstat, uint32 core)
{
	hwa_dev_t *dev;
	uint32 v32, loop_count;
	uint32 busy, start_curstate, num_tx_status_count;

	HWA_FTRACE(HWA4a);

	HWA_ASSERT(core < HWA_TX_CORES);

	// Audit pre-conditions
	dev = HWA_DEV(txstat);

	// Poll txs_debug_reg to make sure it's done
	loop_count = 0;
	do {
		if (loop_count)
			OSL_DELAY(1);
		v32 = HWA_RD_REG_NAME(HWA4a, dev->regs, tx_status[core], txs_debug_reg);
		start_curstate = BCM_GBF(v32, HWA_TX_STATUS_TXS_DEBUG_REG_START_CURSTATE);
		num_tx_status_count = BCM_GBF(v32, HWA_TX_STATUS_TXS_DEBUG_REG_NUM_TX_STATUS_COUNT);
		busy = (start_curstate > 1) | (num_tx_status_count != 0);
		HWA_TRACE(("%s Polling txs_debug_reg <%u:%u>\n", __FUNCTION__,
			start_curstate, num_tx_status_count));
	} while (busy && ++loop_count != HWA_FSM_IDLE_POLLLOOP);
	if (loop_count == HWA_FSM_IDLE_POLLLOOP) {
		HWA_ERROR(("%s txs_debug_reg is not idle <%u:%u>\n", __FUNCTION__,
			start_curstate, num_tx_status_count));
	}
}

void // HWA4a TxStatus block reclaim
hwa_txstat_reclaim(hwa_dev_t *dev)
{
	uint32 core;
	hwa_txstat_t *txstat; // SW txstat state
	hwa_ring_t *h2s_ring; // H2S TxStatus status_ring

	HWA_FTRACE(HWA4a);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	txstat = &dev->txstat;

	// Consume all txstatus in H2S TxStatus status_ring
	for (core = 0; core < HWA_TX_CORES; core++) {
		h2s_ring = &txstat->status_ring[core];
		hwa_ring_cons_get(h2s_ring); // fetch HWA txstatus ring's WR index once
		hwa_ring_cons_all(h2s_ring); // consume all elements.
		hwa_ring_cons_put(h2s_ring); // commit RD index now
	}
}

int // Consume all txstatus in H2S txstatus interface
hwa_txstat_process(struct hwa_dev *dev, uint32 core, bool bound)
{
	int ret;
	uint32 proc_cnt;
	uint32 elem_ix; // location of next element to read
	void *txstatus; // MAC generate txstatus blob
	hwa_txstat_t *txstat; // SW txstat state
	hwa_ring_t *h2s_ring; // H2S TxStatus status_ring
	hwa_handler_t *proc_handler; // upstream per TxStatus handler
	hwa_handler_t *done_handler; // upstream all TxStatus done handler
	bool fatal;
	wlc_info_t *wlc;

	HWA_FTRACE(HWA4a);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);
	HWA_ASSERT(core < HWA_TX_CORES);

	// Setup locals
	fatal = FALSE;
	proc_cnt = 0;
	txstat = &dev->txstat;
	h2s_ring = &txstat->status_ring[core];
	wlc = (wlc_info_t *)dev->wlc;

	// Fetch registered upstream callback handler
	proc_handler = &dev->handlers[HWA_TXSTAT_PROC];
	done_handler = &dev->handlers[HWA_TXSTAT_DONE];

	HWA_STATS_EXPR(txstat->wake_cnt[core]++);
	hwa_ring_cons_get(h2s_ring); // fetch HWA txstatus ring's WR index once

	// Consume all TxStatus received in status_ring, handing each upstream
	while ((elem_ix = hwa_ring_cons_upd(h2s_ring)) != BCM_RING_EMPTY) {

		txstatus = HWA_RING_ELEM(hwa_txstat_status_t, h2s_ring, elem_ix);

		if (!wlc->hw->up)
			break;

		// Invoke upstream bound handler
		ret = (*proc_handler->callback)(proc_handler->context,
			(uintptr)txstatus, (uintptr)&fatal, core, 0);

		proc_cnt++;

		// Callback cannot handle it, break the loop.
		if (ret != HWA_SUCCESS)
			break;

		if ((proc_cnt % HWA_TXSTAT_LAZY_RD_UPDATE) == 0) {
			if (bound) {
				break;
			} else {
				hwa_ring_cons_put(h2s_ring); // commit RD index lazily
			}
		}
	}

	hwa_ring_cons_put(h2s_ring); // commit RD index now

	HWA_STATS_EXPR(txstat->proc_cnt[core] += proc_cnt);

	if (proc_cnt) {
		(*done_handler->callback)(done_handler->context,
			(uintptr)&fatal, (uintptr)0, core, proc_cnt);
	}

	if (!hwa_ring_is_empty(h2s_ring)) {
		/* need re-schdeule */
		if (core == 0)
			dev->intstatus |= HWA_COMMON_INTSTATUS_TXSWRINDUPD_INT_CORE0_MASK;
		else
			dev->intstatus |= HWA_COMMON_INTSTATUS_TXSWRINDUPD_INT_CORE1_MASK;
	}

	return HWA_SUCCESS;
}

/**
 * Read a 16 byte status package from the TxStatus.
 * The first word has a valid bit that indicates if the fifo had
 * a valid entry or if the fifo was empty.
 *
 * @return int BCME_OK if the entry was valid, BCME_NOTREADY if
 *         the TxStatus fifo was empty, BCME_NODEVICE if the
 *         read returns 0xFFFFFFFF indicating a target abort
 */
static int
hwa_txstat_read_txs_pkg16(wlc_info_t *wlc, wlc_txs_pkg16_t *txs)
{
	uint32 s1;

	s1 = txs->word[0];

	if ((s1 & TXS_V) == 0) {
		return BCME_NOTREADY;
	}
	/* Invalid read indicates a dead chip */
	if (s1 == 0xFFFFFFFF) {
		return BCME_NODEVICE;
	}

#if defined(WL_TXS_LOG)
	wlc_bmac_txs_hist_pkg16(wlc->hw, txs);
#endif /* WL_TXS_LOG */

	return BCME_OK;
}

static int
hwa_txstat_bmac_proc(void *context, uintptr arg1, uintptr arg2, uint32 core, uint32 arg4)
{
	hwa_dev_t *dev = (hwa_dev_t *)context;
	wlc_info_t *wlc = (wlc_info_t *)dev->wlc;
	wlc_txs_pkg16_t *pkg = (wlc_txs_pkg16_t *)arg1;
	bool *fatal = (bool *)arg2;
	osl_t *osh = dev->osh;
	int txserr = BCME_OK;
	tx_status_t txs;
	/* pkg 1 */
	uint32 v_s1, v_s2, v_s3, v_s4;
#ifdef WLC_TSYNC
	/* pkg 3 */
	uint32 v_s9, v_s10, v_s11;
#endif // endif
	uint16 status_bits;
	uint16 ncons;

	HWA_FTRACE(HWA4a);
	HWA_ASSERT(context != (void *)NULL);
	HWA_ASSERT(arg1 != 0);
	HWA_ASSERT(arg2 != 0);

	if (pkg == NULL) {
		HWA_ERROR(("%s %s(%d) txstatus is NULL!!!\n", HWA4a, __FUNCTION__, __LINE__));
		return HWA_FAILURE;
	}

	/* Get pkg 1 */
	if ((txserr = hwa_txstat_read_txs_pkg16(wlc, pkg)) == BCME_OK) {
		v_s1 = pkg->word[0];
		v_s2 = pkg->word[1];
		v_s3 = pkg->word[2];
		v_s4 = pkg->word[3];

		HWA_TRACE(("%s: s1=%0x ampdu=%d\n", __FUNCTION__, v_s1,
			((v_s1 & 0x4) != 0)));
		txs.frameid = (v_s1 & TXS_FID_MASK) >> TXS_FID_SHIFT;
		txs.sequence = v_s2 & TXS_SEQ_MASK;
		txs.phyerr = (v_s2 & TXS_PTX_MASK) >> TXS_PTX_SHIFT;
		txs.lasttxtime = R_REG(osh, D11_TSFTimerLow(wlc));
		status_bits = v_s1 & TXS_STATUS_MASK;
		txs.status.raw_bits = status_bits;
		txs.status.is_intermediate = (status_bits & TX_STATUS40_INTERMEDIATE) != 0;
		txs.status.pm_indicated = (status_bits & TX_STATUS40_PMINDCTD) != 0;

		ncons = ((status_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT);
		txs.status.was_acked = ((ncons <= 1) ?
			((status_bits & TX_STATUS40_ACK_RCV) != 0) : TRUE);
		txs.status.suppr_ind =
				(status_bits & TX_STATUS40_SUPR) >> TX_STATUS40_SUPR_SHIFT;

		/* pkg 2 comes always */
		txserr = hwa_txstat_read_txs_pkg16(wlc, ++pkg);
		/* store saved extras (check valid pkg) */
		if (txserr != BCME_OK) {
			/* if not a valid package, assert and bail */
			HWA_ERROR(("wl%d: %s: package 2 read not valid\n",
				wlc->pub->unit, __FUNCTION__));
			goto done;
		}

		HWA_TRACE(("wl%d: %s calls dotxstatus\n", wlc->pub->unit, __FUNCTION__));

		HWA_PTRACE(("wl%d: %s:: Raw txstatus %08X %08X %08X %08X "
			"%08X %08X %08X %08X\n",
			wlc->pub->unit, __FUNCTION__,
			v_s1, v_s2, v_s3, v_s4,
			pkg->word[0], pkg->word[1], pkg->word[2], pkg->word[3]));

		txs.status.s3 = v_s3;
		txs.status.s4 = v_s4;
		txs.status.s5 = pkg->word[0];
		txs.status.ack_map1 = pkg->word[1];
		txs.status.ack_map2 = pkg->word[2];
		txs.status.s8 = pkg->word[3];

		txs.status.rts_tx_cnt = ((pkg->word[0] & TX_STATUS40_RTS_RTX_MASK) >>
								 TX_STATUS40_RTS_RTX_SHIFT);
		txs.status.cts_rx_cnt = ((pkg->word[0] & TX_STATUS40_CTS_RRX_MASK) >>
								 TX_STATUS40_CTS_RRX_SHIFT);

		if ((pkg->word[0] & TX_STATUS64_MUTX)) {
			/* Only RT0 entry is used for frag_tx_cnt in ucode */
			txs.status.frag_tx_cnt = TX_STATUS40_TXCNT_RT0(v_s3);
		} else {
			txs.status.frag_tx_cnt = TX_STATUS40_TXCNT(v_s3, v_s4);
		}

#ifdef WLFCTS
		if (WLFCTS_ENAB(wlc->pub)) {
			uint32 lasttxtime_lo16 = (pkg->word[3] >> 16) & 0x0000ffff;
			uint32 dequeuetime_lo16 = pkg->word[3] & 0x0000ffff;
			txs.dequeuetime = ((txs.lasttxtime - dequeuetime_lo16) & 0xffff0000)
					| dequeuetime_lo16;
			txs.lasttxtime = ((txs.lasttxtime - lasttxtime_lo16) & 0xffff0000)
					| lasttxtime_lo16;
		}
#endif /* WLFCTS */

#ifdef WLC_TSYNC
		if (TSYNC_ENAB(wlc->pub)) {
			txserr = hwa_txstat_read_txs_pkg16(wlc, ++pkg);
			if (txserr != BCME_OK) {
				HWA_ERROR(("wl%d: %s: package 3 read not valid\n",
					wlc->pub->unit, __FUNCTION__));
				goto done;
			}
			v_s9 = pkg->word[0];
			v_s10 = pkg->word[1];
			v_s11 = pkg->word[2];

			HWA_PTRACE(("wl%d: %s:: Raw txstatus %08X %08X %08X\n",
				wlc->pub->unit, __FUNCTION__,
				v_s9, v_s10, v_s11));

			txs.status.s9 = v_s9;
			txs.status.s10 = v_s10;
			txs.status.s11 = v_s11;
		}
#endif /* WLC_TSYNC */

		*fatal = wlc_bmac_dotxstatus(wlc->hw, &txs, v_s2);
	}

done:
	if (txserr == BCME_NODEVICE) {
		HWA_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		HWA_ASSERT(pkg->word[0] != 0xffffffff);
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		return HWA_FAILURE;
	}

	if (*fatal) {
		HWA_ERROR(("error %d caught in %s\n", *fatal, __FUNCTION__));
		return HWA_FAILURE;
	}

	return HWA_SUCCESS;
}

static int
hwa_txstat_bmac_done(void *context, uintptr arg1, uintptr arg2, uint32 core, uint32 proc_cnt)
{
	hwa_dev_t *dev = (hwa_dev_t *)context;
	wlc_info_t *wlc = (wlc_info_t *)dev->wlc;
	bool *fatal = (bool *)arg1;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	HWA_TRACE(("%s %s(): core<%u> proc_cnt<%u>\n", HWA4a, __FUNCTION__, core, proc_cnt));

	if (wlc->active_queue != NULL && WLC_TXQ_OCCUPIED(wlc)) {
		WLDURATION_ENTER(wlc, DUR_DPC_TXSTATUS_SENDQ);
		wlc_send_q(wlc, wlc->active_queue);
		WLDURATION_EXIT(wlc, DUR_DPC_TXSTATUS_SENDQ);
	}

	if (*fatal) {
		HWA_ERROR(("wl%d: %s HAMMERING fatal txs err\n",
			wlc_hw->unit, __FUNCTION__));
		if (wlc_hw->need_reinit == WL_REINIT_RC_NONE) {
			wlc_hw->need_reinit = WL_REINIT_RC_INV_TX_STATUS;
		}
		WLC_FATAL_ERROR(wlc);
	}

	return HWA_SUCCESS;
}

// HWA4a TxStat block statistics collection
static void _hwa_txstat_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 core);

void // Clear statistics for HWA4a TxStat block
hwa_txstat_stats_clear(hwa_txstat_t *txstat, uint32 core)
{
	hwa_dev_t *dev;

	dev = HWA_DEV(txstat);

	hwa_stats_clear(dev, HWA_STATS_RXPOST_CORE0 + core); // common

} // hwa_txstat_stats_clear

void // Print the common statistics for HWA4a TxStat block
_hwa_txstat_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 core)
{
	hwa_txstat_stats_t *txstat_stats = &dev->txstat.stats[core];
	struct bcmstrbuf *b = (struct bcmstrbuf *)buf;

	HWA_BPRINT(b, "%s statistics core[%u] lazy<%u> qfull<%u> stalls<%u> dur<%u>\n",
		HWA4a, core,
		txstat_stats->num_lazy_intr, txstat_stats->num_queue_full_ctr_sat,
		txstat_stats->num_stalls_dma, txstat_stats->dur_dma_busy);

} // _hwa_txstat_stats_dump

void // Query and dump common statistics for HWA4a TxStat block
hwa_txstat_stats_dump(hwa_txstat_t *txstat, struct bcmstrbuf *b, uint8 clear_on_copy)
{
	uint32 core;
	hwa_dev_t *dev;

	dev = HWA_DEV(txstat);

	for (core = 0; core < HWA_TX_CORES; core++) {
		hwa_txstat_stats_t *txstat_stats = &txstat->stats[core];
		hwa_stats_copy(dev, HWA_STATS_TXSTS_CORE0 + core,
			HWA_PTR2UINT(txstat_stats), HWA_PTR2HIADDR(txstat_stats),
			/* num_sets */ 1, clear_on_copy, &_hwa_txstat_stats_dump,
			(uintptr)b, core);
	}

} // hwa_txstat_stats_dump

#if defined(BCMDBG)

void // HWA4a TxStat: debug dump
hwa_txstat_dump(hwa_txstat_t *txstat, struct bcmstrbuf *b, bool verbose, bool dump_regs)
{
	uint32 core;

	HWA_BPRINT(b, "%s dump<%p>\n", HWA4a, txstat);

	if (txstat == (hwa_txstat_t*)NULL)
		return;

	HWA_BPRINT(b, "%s dump<%p>\n", HWA4a, txstat);

	for (core = 0; core < HWA_TX_CORES; core++) {
		hwa_ring_dump(&txstat->status_ring[core], b, "+ status");

		HWA_STATS_EXPR(
			HWA_BPRINT(b, "+ core<%u> wake<%u> proc<%u>\n",
				core, txstat->wake_cnt[core], txstat->proc_cnt[core]));
	}

	if (verbose == TRUE) {
		hwa_txstat_stats_dump(txstat, b, /* clear */ 0);
	}

#if defined(WLTEST)
	if (dump_regs == TRUE)
		hwa_txstat_regs_dump(txstat, b);
#endif // endif

} // hwa_txstat_dump

#if defined(WLTEST)

void // HWA4a TxStat: dump block registers
hwa_txstat_regs_dump(hwa_txstat_t *txstat, struct bcmstrbuf *b)
{
	uint32 core;
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	if (txstat == (hwa_txstat_t*)NULL)
		return;

	dev = HWA_DEV(txstat);
	regs = dev->regs;

	for (core = 0; core < HWA_TX_CORES; core++) {
		HWA_BPRINT(b, "%s registers[0x%p] offset[0x%04x]\n",
			HWA4a, &regs->tx_status[core],
			(uint32)OFFSETOF(hwa_regs_t, tx_status[core]));

		HWA_BPR_REG(b, tx_status[core], tseq_base_lo);
		HWA_BPR_REG(b, tx_status[core], tseq_base_hi);
		HWA_BPR_REG(b, tx_status[core], tseq_size);
		HWA_BPR_REG(b, tx_status[core], tseq_wridx);
		HWA_BPR_REG(b, tx_status[core], tseq_rdidx);
		HWA_BPR_REG(b, tx_status[core], tse_ctl);
		HWA_BPR_REG(b, tx_status[core], tse_sts);
		HWA_BPR_REG(b, tx_status[core], txs_debug_reg);
		HWA_BPR_REG(b, tx_status[core], dma_desc_template_txs);
#if HWA_REVISION_GE_129
		HWA_BPR_REG(b, tx_status[core], txe_cfg1);
		HWA_BPR_REG(b, tx_status[core], tse_axi_base);
		HWA_BPR_REG(b, tx_status[core], tse_axi_ctl);
#endif /* HWA_REVISION_GE_129 */
	} // for core

} // hwa_txstat_regs_dump

#endif // endif

#endif /* BCMDBG */

#endif /* HWA_TXSTAT_BUILD */

#if defined(BCMHWA) && defined(HWA_RXPATH_BUILD)
void
hwa_wl_reclaim_rx_packets(hwa_dev_t *dev)
{
	wlc_info_t *wlc;

	// Setup locals
	wlc = dev->wlc;

	// Flush rx pktfetch
	wlc_recvdata_pktfetch_queue_flush(wlc);

	// Flush amsdu deagg
	wlc_amsdu_flush(wlc->ami);

	// Flush ampdu rx reordering queue
	wlc_ampdu_rx_flush_all(wlc);
}
#endif /* BCMHWA && HWA_RXPATH_BUILD */

void
hwa_wl_flush_all(hwa_dev_t *dev)
{
#if defined(HWA_RXFILL_BUILD) || defined(HWA_TXSTAT_BUILD)
	wl_info_t *wl = ((wlc_info_t *)dev->wlc)->wl;

	/* flush wl->rxcpl_list for rxcpl fast path. */
	HWA_RXFILL_EXPR(wl_flush_rxreorderqeue_flow(wl, &wl->rxcpl_list));
#ifdef DONGLEBUILD
	/* flush accumulated txrxstatus here */
	wl_busioctl(wl, BUS_FLUSH_CHAINED_PKTS, NULL, 0, NULL, NULL, FALSE); // function in rte
#else
	BCM_REFERENCE(wl);
#endif /* DONGLEBUILD */
#endif /* HWA_RXFILL_BUILD || HWA_TXSTAT_BUILD */
}

void
hwa_wlc_mac_event(hwa_dev_t *dev, uint reason)
{
	wlc_info_t *wlc = (wlc_info_t *)dev->wlc;
	wlc_mac_event(wlc, WLC_E_HWA_EVENT, NULL, WLC_E_STATUS_SUCCESS, reason, 0, 0, 0);
}

void
hwa_caps(struct hwa_dev *dev, struct bcmstrbuf *b)
{
	if (dev == (hwa_dev_t*)NULL)
		return;

	HWA_BPRINT(b, "hwa");
	HWA_RXPOST_EXPR(HWA_BPRINT(b, "-1a"));
	HWA_RXFILL_EXPR(HWA_BPRINT(b, "-1b"));
	HWA_RXDATA_EXPR(HWA_BPRINT(b, "-2a"));
	HWA_RXCPLE_EXPR(HWA_BPRINT(b, "-2b"));
	HWA_TXPOST_EXPR(HWA_BPRINT(b, "-3a"));
	HWA_TXFIFO_EXPR(HWA_BPRINT(b, "-3b"));
	HWA_TXSTAT_EXPR(HWA_BPRINT(b, "-4a"));
	HWA_TXCPLE_EXPR(HWA_BPRINT(b, "-4b"));
	HWA_BPRINT(b, " ");
}

/** IOVAR table */
enum {
	IOV_HWA_RX_ENABLE,
	IOV_LAST
};

static const bcm_iovar_t hwa_iovars[] = {
	{"hwa_rx_en", IOV_HWA_RX_ENABLE, (0), 0, IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0, 0}
};

static int hwa_doiovar(void *ctx, uint32 actionid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif);
static int hwa_up(void *hdl);
static int hwa_down(void *hdl);

int
hwa_wlc_module_register(hwa_dev_t *dev)
{
	wlc_info_t *wlc = (wlc_info_t *)dev->wlc;

	/* register module */
	if (wlc_module_register(wlc->pub, hwa_iovars, "hwa", dev, hwa_doiovar,
		NULL, hwa_up, hwa_down) != BCME_OK) {
		HWA_ERROR(("%s wlc_module_register() failed\n", HWA00));
		return BCME_ERROR;
	}

	return BCME_OK;
}

/** HWA iovar handler */
static int
hwa_doiovar(void *ctx, uint32 actionid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	hwa_dev_t *dev;
	wlc_info_t *wlc; /* WLC module */

	int32 *ret_int_ptr;
	bool bool_val;
	int32 int_val = 0;
	int err = BCME_OK;

	ASSERT(ctx != NULL);
	dev = (hwa_dev_t *)ctx;
	wlc = dev->wlc;

	BCM_REFERENCE(vsize);
	BCM_REFERENCE(alen);
	BCM_REFERENCE(wlcif);
	BCM_REFERENCE(ret_int_ptr);
	BCM_REFERENCE(bool_val);
	BCM_REFERENCE(wlc);

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)a;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_SVAL(IOV_HWA_RX_ENABLE):
		if (bool_val) {
			hwa_rx_enable(dev);
		}
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	if (!IOV_ISSET(actionid))
		return err;

	return err;
}

static int
hwa_up(void *hdl)
{
	hwa_dev_t *dev = (hwa_dev_t *)hdl;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	if (dev->reinit) {
		hwa_reinit(dev);
	} else {
		/* Initialize HWA core */
		hwa_init(dev);

		hwa_enable(dev);

		dev->up = TRUE;

		HWA_RXPOST_EXPR(hwa_wlc_mac_event(dev, WLC_E_HWA_RX_POST));

		HWA_DPC_EXPR(hwa_intrson(dev));

		HWA_TRACE(("\n\n----- HWA INTR ON: intmask<0x%08x> defintmask<0x%08x> "
			"intstatus<0x%08x> -----\n\n", dev->intmask, dev->defintmask,
			dev->intstatus));
	}

	return BCME_OK;
}

static int
hwa_down(void *hdl)
{
	hwa_dev_t *dev = (hwa_dev_t *)hdl;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	if (!dev->up)
		return BCME_OK;

	hwa_disable(dev);

	hwa_deinit(dev);

	dev->up = FALSE;

	return BCME_OK;
}

#if defined(BCMDBG)
#define HWA_DUMP_ARGV_MAX	64
#define HWA_DUMP_FIFO_MAX	128
/* wl dump hwa [-b all top cmn dma 1a 1b 2a 2b 3a 3b 4a 4b -v -r -s] */
static int
hwa_wl_dump_parse_args(wlc_info_t *wlc, uint32 *block_bitmap, bool *verbose,
	bool *dump_regs, bool *dump_txfifo_shadow, uint8 *fifo_bitmap, bool *help)
{
	int i, err = BCME_OK;
	char *args = wlc->dump_args;
	char *p, **argv = NULL;
	uint argc = 0;
	char opt, curr = '\0';
	uint32 val32;

	if (args == NULL || block_bitmap == NULL || verbose == NULL ||
		dump_regs == NULL || dump_txfifo_shadow == NULL) {
		err = BCME_BADARG;
		goto exit;
	}

	/* allocate argv */
	if ((argv = MALLOC(wlc->osh, sizeof(*argv) * HWA_DUMP_ARGV_MAX)) == NULL) {
		HWA_ERROR(("wl%d: %s: failed to allocate the argv buffer\n",
		          wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* get each token */
	p = bcmstrtok(&args, " ", 0);
	while (p && argc < HWA_DUMP_ARGV_MAX-1) {
		argv[argc++] = p;
		p = bcmstrtok(&args, " ", 0);
	}
	argv[argc] = NULL;

	/* initial default */
	*block_bitmap = HWA_DUMP_ALL;
	*verbose = FALSE;
	*dump_regs = FALSE;
	*dump_txfifo_shadow = FALSE;
	*help = FALSE;

	/* parse argv */
	argc = 0;
	while ((p = argv[argc++])) {
		if (!strncmp(p, "-", 1)) {
			if (strlen(p) > 2) {
				err = BCME_BADARG;
				goto exit;
			}
			opt = p[1];

			switch (opt) {
				case 'b':
					curr = 'b';
					*block_bitmap = 0;
					break;
				case 'v':
					curr = 'v';
					*verbose = TRUE;
					break;
				case 'r':
					curr = 'r';
					*dump_regs = TRUE;
					break;
				case 's':
					curr = 's';
					*dump_txfifo_shadow = TRUE;
					break;
				case 'f':
					curr = 'f';
					for (i = 0; i < HWA_DUMP_FIFO_MAX; i++) {
						clrbit(fifo_bitmap, i);
					}
					break;
				case 'h':
					curr = 'h';
					*help = TRUE;
					break;
				default:
					err = BCME_BADARG;
					goto exit;
			}
		} else {
			switch (curr) {
				case 'b':
					if (!strcmp(p, "all")) {
						*block_bitmap = HWA_DUMP_ALL;
					} else if (!strcmp(p, "top")) {
						*block_bitmap |= HWA_DUMP_TOP;
					} else if (!strcmp(p, "cmn")) {
						*block_bitmap |= HWA_DUMP_CMN;
					} else if (!strcmp(p, "dma")) {
						*block_bitmap |= HWA_DUMP_DMA;
					} else if (!strcmp(p, "1a")) {
						*block_bitmap |= HWA_DUMP_1A;
					} else if (!strcmp(p, "1b")) {
						*block_bitmap |= HWA_DUMP_1B;
					} else if (!strcmp(p, "2a")) {
						*block_bitmap |= HWA_DUMP_2A;
					} else if (!strcmp(p, "2b")) {
						*block_bitmap |= HWA_DUMP_2B;
					} else if (!strcmp(p, "3a")) {
						*block_bitmap |= HWA_DUMP_3A;
					} else if (!strcmp(p, "3b")) {
						*block_bitmap |= HWA_DUMP_3B;
					} else if (!strcmp(p, "4a")) {
						*block_bitmap |= HWA_DUMP_4A;
					} else if (!strcmp(p, "4b")) {
						*block_bitmap |= HWA_DUMP_4B;
					}
					break;
				case 'f':
					if (strcmp(p, "all") == 0) {
						for (i = 0; i < HWA_DUMP_FIFO_MAX; i++) {
							setbit(fifo_bitmap, i);
						}
					}
					else {
						val32 = (uint32)bcm_strtoul(p, NULL, 0);
						if (val32 < HWA_DUMP_FIFO_MAX) {
							setbit(fifo_bitmap, val32);
						}
					}
					break;
				default:
					err = BCME_BADARG;
					goto exit;
			}
		}
	}

exit:
	if (argv) {
		MFREE(wlc->osh, argv, sizeof(*argv) * HWA_DUMP_ARGV_MAX);
	}

	return err;
}

/* wl dump hwa [-b <blocks> -v -r -s -f <HWA fifos> -h] */
int
hwa_wl_dump(struct hwa_dev *dev, struct bcmstrbuf *b)
{
	int i, err = BCME_OK;
	wlc_info_t *wlc = (wlc_info_t *)dev->wlc;
	uint32 block_bitmap = HWA_DUMP_ALL;
	bool verbose = FALSE;
	bool dump_regs = FALSE;
	bool dump_txfifo_shadow = FALSE;
	bool help = FALSE;
	uint8 fifo_bitmap[(HWA_DUMP_FIFO_MAX/NBBY)+1];

	/* Set default value for legacy dump */
	for (i = 0; i < HWA_DUMP_FIFO_MAX; i++) {
		setbit(fifo_bitmap, i);
	}

	/* Parse args if needed */
	if (wlc->dump_args) {
		err = hwa_wl_dump_parse_args(wlc, &block_bitmap, &verbose,
			&dump_regs, &dump_txfifo_shadow, fifo_bitmap, &help);
		if (err != BCME_OK)
			return err;

		if (help) {
			char *help_str = "Dump HWA:\n"
				"\tUsage: wl dump hwa [-b <blocks> -v -r -s -f <HWA fifos> -h]\n"
				"\t       blocks: <top> <cmn> <dma> <1a> <1b> <2a> <2b>"
				" <3a> <3b> <4a> <4b>\n"
				"\t       v: verbose\n"
				"\t       r: dump registers\n"
				"\t       s: dump txfifo shadow\n"
				"\t       f: specific fifos\n";
			bcm_bprintf(b, "%s", help_str);
			return BCME_OK;
		}
	}

	hwa_dump(dev, b, block_bitmap, verbose, dump_regs, dump_txfifo_shadow, fifo_bitmap);

	return BCME_OK;
}
#endif // endif
