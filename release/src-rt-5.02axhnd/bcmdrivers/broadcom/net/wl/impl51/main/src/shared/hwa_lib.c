/*
 * HWA library routines that are not specific to PCIE or MAC facing blocks.
 *
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
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#ifdef HWA_BUS_BUILD
#include <rte_cfg.h>
#include <pciedev.h>
#endif // endif
#include <bcmmsgbuf.h>
#include <hwa_lib.h>
#include <hndsoc.h>

/*
 * -----------------------------------------------------------------------------
 * Section: hwa_ring_t
 * -----------------------------------------------------------------------------
 *
 * Uses the bcm_ring abstraction to implement a HWA S2H or H2S ring interface.
 * Extends the bcm_ring library with the capability to fetch and update WR or RD
 * indices from HWA registers.Addresses of these WR and RD registers are saved
 * in the hwa_ring_t abstraction.
 *
 * -----------------------------------------------------------------------------
 */
void // Initialize a HWA circular ring
hwa_ring_init(hwa_ring_t *ring, const char *name,
	uint8 block_id, uint8 ring_dir, uint8 ring_num,
	uint16 depth, void *memory, hwa_reg_addr_t reg_wr, hwa_reg_addr_t reg_rd)
{
	HWA_TRACE(("%s %s <%p>[%x::%u:%u] memory[%p:%u] wr[%p] rd[%p]\n",
		name, __FUNCTION__, ring, block_id, ring_dir, ring_num,
		memory, depth, reg_wr, reg_rd));

	HWA_RING_ASSERT(ring != HWA_RING_NULL);
	HWA_RING_ASSERT((ring_dir == HWA_RING_S2H) || (ring_dir == HWA_RING_H2S));
	HWA_RING_ASSERT(_CSBTBL[block_id] == 1);
	HWA_RING_ASSERT(depth != 0);
	HWA_RING_ASSERT(memory != (void*)NULL);

	bcm_ring_init(HWA_RING_STATE(ring));

	ring->reg_wr = reg_wr; // "address" of HWA WR register
	ring->reg_rd = reg_rd; // "address" of HWA RD register
	ring->memory = memory; // ring memory
	ring->depth  = depth;  // number of elements in the ring

	// Construct a unique ring identifier - used for debug only
	ring->id.u16 = HWA_RING_ID(block_id, ring_dir, ring_num);
	snprintf(ring->name, HWA_RING_NAME_SIZE, "%s", name);

	// Reset HW registers and SW state
	if (ring_dir == HWA_RING_S2H)
		HWA_WR_REG16_ADDR(ring->name, ring->reg_wr, 0U);
	else
		HWA_WR_REG16_ADDR(ring->name, ring->reg_rd, 0U);
	HWA_RING_STATE(ring)->write = 0;
	HWA_RING_STATE(ring)->read = 0;

} // hwa_ring_init

void // Finish using a ring, Reset HW registers to prevent accidental use
hwa_ring_fini(hwa_ring_t *ring)
{
	if (ring == NULL)
		return;

	// Reset HW registers and SW state
	HWA_WR_REG_ADDR(ring->name, ring->reg_wr, 0U);
	HWA_WR_REG_ADDR(ring->name, ring->reg_rd, 0U);
	memset(ring, 0, sizeof(*ring));

} // hwa_ring_fini

void // Debug dump a hwa_ring_t
hwa_ring_dump(hwa_ring_t *ring, struct bcmstrbuf *b, const char *prefix)
{
	if (ring == HWA_RING_NULL || ring->memory == NULL)
		return;

	HWA_BPRINT(b, "%s ring<%s,%p>[%x::%u:%u] memory<%p:%u>"
		" wr[%p::%u][%u] rd[%p:%u][%u]\n", prefix,
		ring->name, ring,
		ring->id.block_id, ring->id.ring_dir, ring->id.ring_num,
		ring->memory, ring->depth,
		ring->reg_wr, HWA_RD_REG_ADDR(ring->name, ring->reg_wr),
		HWA_RING_STATE(ring)->write,
		ring->reg_rd, HWA_RD_REG_ADDR(ring->name, ring->reg_rd),
		HWA_RING_STATE(ring)->read);

} // hwa_ring_dump

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Internal DMA Engines
 * -----------------------------------------------------------------------------
 */
void
BCMATTACHFN(hwa_dma_attach)(hwa_dev_t *dev)
{
	uint32 v32, tx32, rx32, i;
	hwa_regs_t *regs;

	HWA_FTRACE(HWAde);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	regs = dev->regs;

	v32 = HWA_RD_REG_NAME(HWAde, regs, dma, corecontrol);
	dev->dma.arbitration = BCM_GBF(v32, HWA_DMA_CORECONTROL_ARBTYPE);
	HWA_TRACE(("%s DMA arbitration<%s>\n",
		HWAde, (dev->dma.arbitration == 0) ? "RR" : "FXD"));

	v32 = HWA_RD_REG_NAME(HWAde, regs, dma, corecapabilities);
	dev->dma.channels_max =
		BCM_GBF(v32, HWA_DMA_CORECAPABILITIES_CHANNELCNT);
	dev->dma.burst_length_max =
		BCM_GBF(v32, HWA_DMA_CORECAPABILITIES_MAXBURSTLEN);
	dev->dma.outstanding_rds_max =
		BCM_GBF(v32, HWA_DMA_CORECAPABILITIES_MAXRDOUTSTANDING);

	/* HWA_DMA_CHANNEL1 burst length is adjustable, experiment value can reach to 128 */
	dev->dma.channels = HWA_DMA_CHANNELS_MAX;
	dev->dma.burst_length[HWA_DMA_CHANNEL0] = DMA_BL_64;
	dev->dma.burst_length[HWA_DMA_CHANNEL1] = DMA_BL_64;
	dev->dma.outstanding_rds = DMA_MR_2;
	/* channelCnt contains one less than the number of DMA channel
	 * pairs supported by this device
	 */
	HWA_ASSERT(dev->dma.channels <= (dev->dma.channels_max + 1));
	HWA_ASSERT(dev->dma.burst_length[HWA_DMA_CHANNEL0] <= dev->dma.burst_length_max);
	HWA_ASSERT(dev->dma.outstanding_rds <= dev->dma.outstanding_rds_max);

	HWA_TRACE(("%s channels[%u:%u] burstlen[%u:%u:%u] outst_rds[%u:%u]\n",
		HWAde, dev->dma.channels, dev->dma.channels_max,
		dev->dma.burst_length[HWA_DMA_CHANNEL0],
		dev->dma.burst_length[HWA_DMA_CHANNEL1],
		dev->dma.burst_length_max,
		dev->dma.outstanding_rds, dev->dma.outstanding_rds_max));

	/*
	 * HWA DMA engines are not programmed by SW, and no INTs are expected
	 * other than errors. Likewise the HWA DMA engines are not truly descriptor
	 * rings with base address etc. HWA cores directly program these DMA engines
	 * bypassing the prefetch logic and wait upon DMA dones.
	 *
	 * Per channel, descErr, dataErr, descProtoErr and rcvDescUf? may be enabled
	 * for HWA debug. SW may enable this and monitor interrupt status for DBG.
	 */
	v32 = 0;
	HWA_WR_REG_NAME(HWAde, regs, dma, intcontrol, v32);

	/*
	 * SW has no control on which DMA channel is used for transfers to/from
	 * host DDR (i.e over PCIE) and for transfers to/from Device SysMem.
	 * As such, settings like: Coherency, BurstAlignEn, AddrExt on XmtCtrl and
	 * RcvCtrl are not useful.
	 *
	 * Likewise, DMA engines' descriptor prefetch does not apply.
	 */
	for (i = 0; i < dev->dma.channels; i++) {

		tx32 = (0U
			// | D64_XC_XE // "XmtEn" will be done in hwa_dma_enable()
			// | D64_XC_SE "SuspEn"
			// | D64_XC_LE "LoopbackEn"
			// | D64_XC_FL "Flush"
			// | D64_XC_BE // "BurstAlignEn"
			| BCM_SBF(dev->dma.outstanding_rds, D64_XC_MR)
			// | D64_XC_CS "ChannelSwitchEn"
			// | D64_XC_PD "PtyChkDisable"
			// | D64_XC_SA "SelectActive"
			// | D64_XC_AE "AddrExt" ... NIC Mode impacts???
			| BCM_SBF(dev->dma.burst_length[i], D64_XC_BL)
			| BCM_SBF(DMA_PC_0, D64_XC_PC) // "PrefetchCtl" = 0
			| BCM_SBF(DMA_PT_1, D64_XC_PT) // "PrefetchThresh" = 0
			| BCM_SBIT(D64_XC_CO) // Coherent
			| 0U);
		HWA_WR_REG_NAME(HWAde, regs, dma, channels[i].tx.control, tx32);

		// Enabling WaitForComplete is only required for specififc DMA transfers
		// to Host DDR over PCIE, to avoid readback traffic.
		rx32 = (0U
			// | D64_RC_RE // "RcvEn" will be done in hwa_dma_enable()
			| BCM_SBF(0, D64_RC_RO) // "RcvOffset" = 0
			// | D64_RC_FM "FIFOMode"
			// | D64_RC_SH "SepRxHdrDescrEn"
			// | D64_RC_OC "OverflowCont"
			// | D64_RC_PD "PtyChkDisable"
			// | D64_RC_SA "SelectActive"
			// | D64_RC_GE "GlomEn"
			// | D64_RC_AE "AddrExt" ... NIC Mode impacts???
			| BCM_SBF(dev->dma.burst_length[i], D64_RC_BL)
			| BCM_SBF(DMA_PC_0, D64_RC_PC) // "PrefetchCtl" = 0
			| BCM_SBF(DMA_PT_1, D64_RC_PT) // "PrefetchThresh" = 0
			| BCM_SBIT(D64_RC_CO) // Coherent
			| 0U);
		HWA_WR_REG_NAME(HWAde, regs, dma, channels[i].rx.control, rx32);

	} // for dma.channels

	// DMA channels not yet enabled, see hwa_dma_enable()
	dev->dma.enabled = FALSE;

} // hwa_dma_attach

void // Enable HWA DMA channels
hwa_dma_enable(hwa_dma_t *dma)
{
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	uint32 i, v32, tx32, rx32;

	HWA_FTRACE(HWAde);

	dev = HWA_DEV(dma);
	regs = dev->regs;

	for (i = 0; i < dma->channels; i++) {
		v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[i].tx.control);
		tx32 = v32 | D64_XC_XE;
		HWA_WR_REG_NAME(HWAde, regs, dma, channels[i].tx.control, tx32);

		v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[i].rx.control);
		rx32 = v32 | D64_RC_RE;
		HWA_WR_REG_NAME(HWAde, regs, dma, channels[i].rx.control, rx32);
	}

	dma->enabled = TRUE;

} // hwa_dma_enable

hwa_dma_status_t // Get the HWA DMA Xmt and Rcv status for a given channel
hwa_dma_channel_status(hwa_dma_t *dma, uint32 channel)
{
	uint32 v32;
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	hwa_dma_status_t status;

	HWA_FTRACE(HWAde);

	HWA_ASSERT(dma != (hwa_dma_t*)NULL);
	HWA_ASSERT(channel < dma->channels);

	dev = HWA_DEV(dma);
	regs = dev->regs;

	status.u32 = 0U;
	v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[channel].tx.status0);
	status.xmt_state = BCM_GBF(v32, HWA_DMA_XMTSTATUS0_XMTSTATE);
	v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[channel].tx.status1);
	status.xmt_error = BCM_GBF(v32, HWA_DMA_XMTSTATUS1_XMTERR);
	v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[channel].rx.status0);
	status.rcv_state = BCM_GBF(v32, HWA_DMA_RCVSTATUS0_RCVSTATE);
	v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[channel].rx.status1);
	status.rcv_error = BCM_GBF(v32, HWA_DMA_RCVSTATUS1_XMTERR);

	return status;
} // hwa_dma_channel_status

#if defined(BCMDBG)
void // Dump the dma channels state
hwa_dma_dump(hwa_dma_t *dma, struct bcmstrbuf *b, bool verbose, bool dump_regs)
{
	if (dma == (hwa_dma_t*)NULL)
		return;

	HWA_BPRINT(b, "%s channels[%u:%u] burstlen[%u:%u:%u] outst_rds[%u:%u] %s\n",
		HWAde, dma->channels, dma->channels_max,
		dma->burst_length[HWA_DMA_CHANNEL0],
		dma->burst_length[HWA_DMA_CHANNEL1],
		dma->burst_length_max,
		dma->outstanding_rds, dma->outstanding_rds_max,
		dma->enabled ? "ENABLED" : "DISABLED");
#if defined(WLTEST)
	if (dump_regs)
		hwa_dma_regs_dump(dma, b);
#endif // endif
} // hwa_dma_dump

#if defined(WLTEST)
void // Dump the dma block registers
hwa_dma_regs_dump(hwa_dma_t *dma, struct bcmstrbuf *b)
{
	uint32 i;
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	if (dma == (hwa_dma_t*)NULL)
		return;

	dev = HWA_DEV(dma);
	regs = dev->regs;

	HWA_BPRINT(b, "%s registers[%p] offset[0x%04x]\n",
		HWAde, &regs->dma, (uint32)OFFSETOF(hwa_regs_t, dma));
	HWA_BPR_REG(b, dma, corecontrol);
	HWA_BPR_REG(b, dma, corecapabilities);
	HWA_BPR_REG(b, dma, intcontrol);
	HWA_BPR_REG(b, dma, intstatus);
	for (i = 0; i < dma->channels; i++) {
		HWA_BPR_REG(b, dma, chint[i].status);
		HWA_BPR_REG(b, dma, chint[i].mask);
	}
	for (i = 0; i < dma->channels; i++) {
		HWA_BPR_REG(b, dma, chintrcvlazy[i]);
	}
	HWA_BPR_REG(b, dma, clockctlstatus);
	HWA_BPR_REG(b, dma, workaround);
	HWA_BPR_REG(b, dma, powercontrol);
	HWA_BPR_REG(b, dma, gpioselect);
	HWA_BPR_REG(b, dma, gpiooutout);
	HWA_BPR_REG(b, dma, gpiooe);
	for (i = 0; i < dma->channels; i++) {
		HWA_BPR_REG(b, dma, channels[i].tx.control);
		HWA_BPR_REG(b, dma, channels[i].tx.ptr);
		HWA_BPR_REG(b, dma, channels[i].tx.addrlow);
		HWA_BPR_REG(b, dma, channels[i].tx.addrhigh);
		HWA_BPR_REG(b, dma, channels[i].tx.status0);
		HWA_BPR_REG(b, dma, channels[i].tx.status1);
		HWA_BPR_REG(b, dma, channels[i].rx.control);
		HWA_BPR_REG(b, dma, channels[i].rx.ptr);
		HWA_BPR_REG(b, dma, channels[i].rx.addrlow);
		HWA_BPR_REG(b, dma, channels[i].rx.addrhigh);
		HWA_BPR_REG(b, dma, channels[i].rx.status0);
		HWA_BPR_REG(b, dma, channels[i].rx.status1);
	}

} // hwa_dma_regs_dump

#endif // endif
#endif /* BCMDBG */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Buffer Manager, used by TxBM and RxBM
 * -----------------------------------------------------------------------------
 */
void // Configure Buffer Manager
hwa_bm_config(hwa_dev_t *dev, hwa_bm_t *bm,
	const char *name, hwa_bm_instance_t instance,
	uint16 pkt_total, uint16 pkt_size,
	uint32 pkt_base_loaddr, uint32 pkt_base_hiaddr, void *memory)
{
	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(bm != (hwa_bm_t*)NULL);
	HWA_ASSERT((pkt_total != 0) && (pkt_size != 0) && (pkt_base_loaddr != 0U));
	HWA_ASSERT(memory != (void*)NULL);

	bm->dev = dev;

	HWA_TRACE(("%s %s config size<%u> total<%u> base32<0x%08x,0x%08x>\n",
		HWAbm, name, pkt_size, pkt_total, pkt_base_loaddr, pkt_base_hiaddr));

	bm->pkt_total       = pkt_total;
	bm->pkt_size        = pkt_size;
	bm->pkt_base.loaddr = pkt_base_loaddr;
	bm->pkt_base.hiaddr = pkt_base_hiaddr;
	bm->memory          = memory;
	bm->instance        = instance;
	snprintf(bm->name, HWA_BM_NAME_SIZE - 1, "%s", name);

	if (bm->instance == HWA_TX_BM) {
		bm->pkt_max     = HWA_TXPATH_PKTS_MAX;
		bm->avail_sw    = HWA_TXPATH_PKTS_MAX - 16;
	}
	else {
		bm->pkt_max     = HWA_RXPATH_PKTS_MAX;
	}

	if (bm->instance == HWA_TX_BM) {
		/* Initial lbuf */
		HWA_TXPOST_EXPR(hwa_txpost_bm_lb_init(dev, memory, pkt_total,
			pkt_size, HWA_TXPOST_PKT_BYTES));
	}

} // hwa_bm_config

void // Enable or Disable a Buffer Manager
hwa_bm_init(hwa_bm_t *bm, bool enable)
{
	uint32 v32;
	hwa_regs_t *regs;

	HWA_FTRACE(HWAbm);

	HWA_ASSERT(bm != (hwa_bm_t*)NULL);
	HWA_ASSERT(bm->pkt_base.loaddr != 0U);

	regs = bm->dev->regs;

	v32 = // BM pkt pool sizing
		BCM_SBF(bm->pkt_total, HWA_BM_BUFFER_CONFIG_NUMBUFS) |
		BCM_SBF(bm->pkt_size, HWA_BM_BUFFER_CONFIG_BUFSIZE);

	if (bm->instance == HWA_TX_BM) {
		HWA_WR_REG_NAME(bm->name, regs, tx_bm, buffer_config, v32);
		v32 = bm->pkt_base.loaddr;
		HWA_WR_REG_NAME(bm->name, regs, tx_bm, pool_start_addr_lo, v32);
		v32 = 0U;
		HWA_WR_REG_NAME(bm->name, regs, tx_bm, pool_start_addr_hi, v32);
	} else {
		HWA_WR_REG_NAME(bm->name, regs, rx_bm, buffer_config, v32);
		v32 = bm->pkt_base.loaddr;
		HWA_WR_REG_NAME(bm->name, regs, rx_bm, pool_start_addr_lo, v32);
		v32 = bm->pkt_base.hiaddr;
		HWA_WR_REG_NAME(bm->name, regs, rx_bm, pool_start_addr_hi, v32);
	}

	v32 = (enable) ? BCM_SBIT(HWA_BM_BM_CTRL_ENABLE) : 0;
	if (bm->instance == HWA_TX_BM)
		HWA_WR_REG_NAME(bm->name, regs, tx_bm, bm_ctrl, v32);
	else
		HWA_WR_REG_NAME(bm->name, regs, rx_bm, bm_ctrl, v32);

	bm->enabled = enable;

	HWA_TRACE(("%s %s state %s\n",
		HWAbm, bm->name, (enable)? "ENABLED" : "DISABLED"));

} // hwa_bm_init

int // Allocate a SW Pkt buffer from the BM
hwa_bm_alloc(hwa_bm_t *bm, dma64addr_t *buf_addr)
{
	int index;
	uint32 v32, status;
	hwa_regs_t *regs;

	HWA_ASSERT(bm != (hwa_bm_t*)NULL);
	HWA_ASSERT(buf_addr != NULL);

	HWA_TRACE(("%s %s alloc\n", HWAbm, bm->name));

	regs = bm->dev->regs;

	if (bm->instance == HWA_TX_BM) {
		if (bm->avail_sw == 0) {
			HWA_TRACE(("%s: No SW pkt buffer\n", __FUNCTION__));
			bm->avail_sw_low++;
			return HWA_FAILURE;
		}
		v32 = HWA_RD_REG_NAME(bm->name, regs, tx_bm, alloc_index);
	}
	else {
		v32 = HWA_RD_REG_NAME(bm->name, regs, rx_bm, alloc_index);
	}
	status = BCM_GBF(v32, HWA_BM_ALLOC_INDEX_ALLOCSTATUS);

	if (status == HWA_BM_SUCCESS_SW) {
		if (bm->instance == HWA_TX_BM) {
			buf_addr->loaddr =
			  HWA_RD_REG_NAME(bm->name, regs, tx_bm, alloc_addr.loaddr);
			buf_addr->hiaddr =
			  HWA_RD_REG_NAME(bm->name, regs, tx_bm, alloc_addr.hiaddr);
			bm->avail_sw--;
		} else {
			buf_addr->loaddr =
			  HWA_RD_REG_NAME(bm->name, regs, rx_bm, alloc_addr.loaddr);
			buf_addr->hiaddr =
			  HWA_RD_REG_NAME(bm->name, regs, rx_bm, alloc_addr.hiaddr);
		}

		index = BCM_GBF(v32, HWA_BM_ALLOC_INDEX_ALLOCINDEX);

		HWA_ASSERT(index < bm->pkt_max);
		HWA_ASSERT(index ==
			hwa_bm_ptr2idx(bm, (void*)(uintptr)buf_addr->loaddr));

		HWA_STATS_EXPR(bm->allocs++);
		HWA_TRACE(("%s %s alloc buf<%u><0x%08x, 0x%08x>\n",
			HWAbm, bm->name, index, buf_addr->loaddr, buf_addr->hiaddr));

		return index;
	}

	HWA_STATS_EXPR(bm->fails++);
	HWA_WARN(("%s %s alloc failure\n", HWAbm, bm->name));

	return HWA_FAILURE;

} // hwa_bm_alloc

int // Free a previously allocated Tx BM SW Pkt buffer by its index
hwa_bm_free(hwa_bm_t *bm, uint16 buf_idx)
{
	uint32 v32, status, loopcnt;
	hwa_regs_t *regs;

	HWA_ASSERT(bm != (hwa_bm_t*)NULL);
	HWA_ASSERT(buf_idx < bm->pkt_max);

	HWA_TRACE(("%s %s free buf_idx<%u>\n", HWAbm, bm->name, buf_idx));

	regs = bm->dev->regs;

	v32 = BCM_SBF(buf_idx, HWA_BM_DEALLOC_INDEX_DEALLOCINDEX);
	if (bm->instance == HWA_TX_BM)
		HWA_WR_REG_NAME(bm->name, regs, tx_bm, dealloc_index, v32);
	else
		HWA_WR_REG_NAME(bm->name, regs, rx_bm, dealloc_index, v32);

	for (loopcnt = 0; loopcnt < HWA_BM_LOOPCNT; loopcnt++) {
		if (loopcnt > bm->loopcnt_hwm)
			bm->loopcnt_hwm = loopcnt;

		if (bm->instance == HWA_TX_BM)
			v32 =
			    HWA_RD_REG_NAME(bm->name, regs, tx_bm, dealloc_status);
		else
			v32 =
			    HWA_RD_REG_NAME(bm->name, regs, rx_bm, dealloc_status);
		status = BCM_GBF(v32, HWA_BM_DEALLOC_STATUS_DEALLOCSTATUS);

#if HWA_REVISION_GE_128
		// WAR for HWA2.x
		if (status == HWA_BM_SUCCESS_SW) {
			HWA_STATS_EXPR(bm->frees++);
			bm->avail_sw++;
			return HWA_SUCCESS;
		}
#endif /* HWA_REVISION_GE_128 */

		if (status & HWA_BM_DONEBIT) {
			HWA_ASSERT(status == HWA_BM_SUCCESS);

			HWA_STATS_EXPR(bm->frees++);
			bm->avail_sw++;
			return HWA_SUCCESS;
		}
	}

	HWA_STATS_EXPR(bm->fails++);
	HWA_WARN(("%s %s free failure\n", HWAbm, bm->name));

	HWA_ASSERT(0); // Failure in deletion of a HWA managed buffer. SW|HWA bug

	return HWA_FAILURE;

} // hwa_bm_free

uint32 // Get the runtime count of available buffers in Buffer Manager
hwa_bm_avail(hwa_bm_t *bm)
{
	uint32 v32, avail_cnt;
	hwa_regs_t *regs;

	HWA_ASSERT(bm != (hwa_bm_t*)NULL);

	regs = bm->dev->regs;

	if (bm->instance == HWA_TX_BM)
		v32 = HWA_RD_REG_NAME(bm->name, regs, tx_bm, bm_ctrl);
	else
		v32 = HWA_RD_REG_NAME(bm->name, regs, rx_bm, bm_ctrl);
	avail_cnt = BCM_GBF(v32, HWA_BM_BM_CTRL_AVAILCNT);

	HWA_TRACE(("%s %s avail<%u>\n", HWAbm, bm->name, avail_cnt));

	return avail_cnt;

} // hwa_bm_avail

#if defined(BCMDBG)
void // Dump HWA Buffer Manager software state
hwa_bm_dump(hwa_bm_t *bm, struct bcmstrbuf *b, bool verbose, bool dump_regs)
{
	HWA_ASSERT(bm != (hwa_bm_t*)NULL);

	HWA_BPRINT(b, "%s dump num<%u> size<%u> base<0x%08x,0x%08x>\n", bm->name,
		bm->pkt_total, bm->pkt_size, bm->pkt_base.loaddr, bm->pkt_base.hiaddr);
	HWA_STATS_EXPR(
		HWA_BPRINT(b, "+ alloc<%u> frees<%u> fails<%u> avail_sw_low<%u>\n",
			bm->allocs, bm->frees, bm->fails, bm->avail_sw_low));
	HWA_STATS_EXPR(bm->avail_sw_low = 0);
#if defined(WLTEST)
	if (dump_regs)
		hwa_bm_regs_dump(bm, b);
#endif // endif
} // hwa_bm_dump

#if defined(WLTEST)
void // Dump HWA Buffer Manager registers
hwa_bm_regs_dump(hwa_bm_t *bm, struct bcmstrbuf *b)
{
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	if ((bm == (hwa_bm_t*)NULL) || (bm->dev == (hwa_dev_t*)NULL) ||
	        (bm->dev->regs == (hwa_regs_t*)NULL))
		return;

	dev = bm->dev;
	regs = dev->regs;

	// Skip: following registers as reading has side effect.
	// alloc_index, alloc_addrlo, alloc_addrhi, dealloc_index*, dealloc_status

	if (bm->instance == HWA_TX_BM) {
		HWA_BPRINT(b, "%s %s registers[%p] offset[0x%04x]\n", HWAbm, bm->name,
			&regs->tx_bm, (uint32)OFFSETOF(hwa_regs_t, tx_bm));
		HWA_BPR_REG(b, tx_bm, buffer_config);
		HWA_BPR_REG(b, tx_bm, pool_start_addr_lo);
		HWA_BPR_REG(b, tx_bm, pool_start_addr_hi);
		HWA_BPR_REG(b, tx_bm, bm_ctrl);
	} else {
		HWA_BPRINT(b, "%s %s registers[%p] offset[0x%04x]\n", HWAbm, bm->name,
			&regs->rx_bm, (uint32)OFFSETOF(hwa_regs_t, rx_bm));
		HWA_BPR_REG(b, rx_bm, buffer_config);
		HWA_BPR_REG(b, rx_bm, pool_start_addr_lo);
		HWA_BPR_REG(b, rx_bm, pool_start_addr_hi);
		HWA_BPR_REG(b, rx_bm, bm_ctrl);
	}

} // hwa_bm_regs_dump

#endif // endif
#endif /* BCMDBG */

/*
 * -----------------------------------------------------------------------------
 * Section: Statistics Register Set Management
 * -----------------------------------------------------------------------------
 */
#define HWA_STATS_BUSY_WAIT_BUSY_BURNLOOP  1024 // burnloop until DMA finishes
static void _hwa_stats_busy_wait(hwa_dev_t *dev);

static void
_hwa_stats_busy_wait(hwa_dev_t *dev)
{
	uint32 v32;
	uint32 is_busy;
	uint32 loop_count = HWA_STATS_BUSY_WAIT_BUSY_BURNLOOP;

	/* Add loop count in case we try to get stats when HWA DMA is dead alreay. */
	v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, statscontrolreg);
	is_busy = BCM_GBF(v32, HWA_COMMON_STATSCONTROLREG_STARTBUSY);

	while (is_busy && loop_count--) {
		OSL_DELAY(1);
		v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, statscontrolreg);
		is_busy = BCM_GBF(v32, HWA_COMMON_STATSCONTROLREG_STARTBUSY);
	}

	if (is_busy && loop_count == 0) {
		HWA_ERROR(("%s(): stats is not ready, HWA DMA could be error!\n", __FUNCTION__));
	}
} // _hwa_stats_busy_wait

void
hwa_stats_clear(hwa_dev_t *dev, hwa_stats_set_index_t set_idx)
{
	uint32 v32;
	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(set_idx < HWA_STATS_SET_INDEX_MAX);
	HWA_FTRACE(HWA00);

	v32 =
		BCM_SBF(set_idx, HWA_COMMON_STATSCONTROLREG_STATSSETIDX) |
		BCM_SBF(1, HWA_COMMON_STATSCONTROLREG_CLEARSTATSSET) |
		BCM_SBF(0, HWA_COMMON_STATSCONTROLREG_COPYSTATSSET) |
		BCM_SBF(0, HWA_COMMON_STATSCONTROLREG_NUMSETS) |
#ifdef BCMPCIEDEV
		BCM_SBF(1, HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATENOTPCIE) |
#else
		BCM_SBF(0, HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATENOTPCIE) |
#endif // endif
		BCM_SBF(dev->host_coherency,
			HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATECOHERENT) |
		BCM_SBF(0, HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATEADDREXT) |
		BCM_SBF(1, HWA_COMMON_STATSCONTROLREG_DMAHWADESCTEMPLATENOTPCIE);
	HWA_WR_REG_NAME(HWA00, dev->regs, common, statscontrolreg, v32);

	_hwa_stats_busy_wait(dev);

} // hwa_stats_clear

void // Fetch HWA statistics and invoke the callback handler on completion
hwa_stats_copy(hwa_dev_t *dev, hwa_stats_set_index_t set_idx,
	uint32 loaddr, uint32 hiaddr, uint32 num_sets, uint8 clear_on_copy,
	hwa_stats_cb_t hwa_stats_cb, uintptr arg1, uint32 arg2)
{
	uint32 v32;

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(set_idx < HWA_STATS_SET_INDEX_MAX);
	HWA_FTRACE(HWA00);

	HWA_WR_REG_NAME(HWA00, dev->regs, common, statdonglreaddressreg, loaddr);
	HWA_WR_REG_NAME(HWA00, dev->regs, common, statdonglreaddresshireg, hiaddr);

	//        in the Common::StatsControlReg?
	//        By placing dma template in this reg we need to repeatedly set them
	v32 =
		BCM_SBF(set_idx, HWA_COMMON_STATSCONTROLREG_STATSSETIDX) |
		BCM_SBF(clear_on_copy, HWA_COMMON_STATSCONTROLREG_CLEARSTATSSET) |
		BCM_SBF(1, HWA_COMMON_STATSCONTROLREG_COPYSTATSSET) |
		BCM_SBF(num_sets, HWA_COMMON_STATSCONTROLREG_NUMSETS) |
#ifdef BCMPCIEDEV
		BCM_SBF(1, HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATENOTPCIE) |
#else
		BCM_SBF(0, HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATENOTPCIE) |
#endif // endif
		BCM_SBF(dev->host_coherency,
			HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATECOHERENT) |
		BCM_SBF(0, HWA_COMMON_STATSCONTROLREG_DMAPCIEDESCTEMPLATEADDREXT) |
		BCM_SBF(1, HWA_COMMON_STATSCONTROLREG_DMAHWADESCTEMPLATENOTPCIE);
	HWA_WR_REG_NAME(HWA00, dev->regs, common, statscontrolreg, v32);

	_hwa_stats_busy_wait(dev); // loop until DMA done ... synchronous

	(*hwa_stats_cb)(dev, arg1, arg2);

} // hwa_stats_copy

/*
 * -----------------------------------------------------------------------------
 * Section: Upstream callback handlers
 * -----------------------------------------------------------------------------
 */
static int // Default HWA noop handler
hwa_callback_noop(void *context,
	uintptr arg1, uintptr arg2, uint32 arg3, uint32 arg4)
{
	HWA_TRACE(("%s(arg1<0x%08x> arg2<0x%08x> arg3<0x%08x> arg4<0x%08x>)\n",
		__FUNCTION__, HWA_PTR2UINT(arg1), HWA_PTR2UINT(arg2), arg3, arg4));
	return HWA_FAILURE;

} // hwa_callback_noop

void // Register an upper layer upstream callback handler
hwa_register(hwa_dev_t *dev,
	hwa_callback_t cb, void *cb_ctx, hwa_callback_fn_t cb_fn)
{
	dev->handlers[cb].context  = cb_ctx;
	dev->handlers[cb].callback = cb_fn;
} // hwa_register

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Device carrying SW driver state of all HWA blocks.
 * -----------------------------------------------------------------------------
 */

// Global instance of HWA device
hwa_dev_t hwa_dev_g = { .coreid = 0, .corerev = 0 };
hwa_dev_t *hwa_dev = &hwa_dev_g;

struct hwa_dev * // HWA attach, uses global hwa_dev
BCMATTACHFN(hwa_attach)(void *wlc, uint device, osl_t *osh,
	volatile void *regs, uint bustype)
{
	int i;
	uint32 v32;
	char *val;

	HWA_TRACE(("\n\n+++++ HWA ATTACH PHASE BEGIN +++++\n\n"));
	HWA_TRACE(("%s attach device<%x> regs<%p> bustype<%u>\n",
		HWA00, device, regs, bustype));

	// Use global hwa_dev
	if (hwa_dev->coreid != 0)
		return hwa_dev;

	HWA_ASSERT(regs != NULL); // pointer to HWA registers

	memset(hwa_dev, 0, sizeof(hwa_dev_t));

	// Attach to backplane
	hwa_dev->sih = si_attach(device, osh, regs, bustype, NULL, NULL, 0);
	if (!hwa_dev->sih) {
		goto fail_si_attach;
	}

	// Bring back to PCIe core
	si_setcore(hwa_dev->sih, HWA_CORE_ID, 0);

	if (!si_iscoreup(hwa_dev->sih)) {
		si_core_reset(hwa_dev->sih, 0, 0);
	}

	// Setup the HWA platform contexts and regs
	hwa_dev->wlc = wlc;
	hwa_dev->osh = osh;
	hwa_dev->regs = regs;
	hwa_dev->device = device;
	hwa_dev->bustype = bustype;

	// Setup the HWA core id and revision
	hwa_dev->coreid = si_coreid(hwa_dev->sih);
	hwa_dev->corerev = si_corerev(hwa_dev->sih);

	// Setup the SW driver mode as NIC or Full Dongle
	hwa_dev->driver_mode = HWA_DRIVER_MODE;

	// For B0 verification.
	// Default values
#ifdef RXCPL4
	hwa_dev->rxcpl_inuse = 0;
#endif // endif
	hwa_dev->txfifo_apb = 1;
	hwa_dev->txstat_apb = 1;
	hwa_dev->txfifo_hwupdnext = 1;
	hwa_dev->tx_pktdealloc = 0;

#if HWA_REVISION_GE_129
	hwa_dev->txfifo_apb = 0;
	hwa_dev->txstat_apb = 1;
	hwa_dev->txfifo_hwupdnext = 0;
	hwa_dev->tx_pktdealloc = 1;
#endif // endif

	// Update from nvram if any.
#ifdef RXCPL4
	if ((val = getvar(NULL, "rxcpl_inuse")) != NULL)
		hwa_dev->rxcpl_inuse = bcm_strtoul(val, NULL, 0);
	HWA_ASSERT(hwa_dev->rxcpl_inuse < 4);
#endif // endif
	if ((val = getvar(NULL, "txfifo_apb")) != NULL)
		hwa_dev->txfifo_apb = bcm_strtoul(val, NULL, 0);
	if ((val = getvar(NULL, "txstat_apb")) != NULL)
		hwa_dev->txstat_apb = bcm_strtoul(val, NULL, 0);
	if ((val = getvar(NULL, "txfifo_hwupdnext")) != NULL)
		hwa_dev->txfifo_hwupdnext = bcm_strtoul(val, NULL, 0);
	if ((val = getvar(NULL, "tx_pktdealloc")) != NULL)
		hwa_dev->tx_pktdealloc = bcm_strtoul(val, NULL, 0);

	HWA_PRINT("%s "
#ifdef RXCPL4
		"rxcpl_inuse<%u> "
#endif // endif
		"txfifo_apb<%u> txstat_apb<%u> "
		"txfifo_hwupdnext<%u> tx_pktdealloc<%u>\n",
		HWA00,
#ifdef RXCPL4
		hwa_dev->rxcpl_inuse,
#endif // endif
		hwa_dev->txfifo_apb, hwa_dev->txstat_apb,
		hwa_dev->txfifo_hwupdnext, hwa_dev->tx_pktdealloc);

	// Fetch the pcie_ipc object
	HWA_BUS_EXPR(hwa_dev->pcie_ipc = hnd_get_pcie_ipc());
	// Fetch the pciedev object
	HWA_BUS_EXPR(hwa_dev->pciedev = pciedev_get_handle());
	HWA_BUS_EXPR(hwa_dev->dngl = pciedev_dngl(hwa_dev->pciedev));

	// Global object is safe for use
	hwa_dev->self = hwa_dev;

	HWA_TRACE(("%s driver mode<%s> hwa_dev<%p> size<%d> core id<%u> rev<%u>"
		" device<%u> bustype<%u>\n",
		HWA00, (hwa_dev->driver_mode == HWA_FD_MODE) ? "FD" : "NIC", hwa_dev,
		(int)sizeof(hwa_dev_t), hwa_dev->coreid, hwa_dev->corerev,
		hwa_dev->device, hwa_dev->bustype));

	HWA_ASSERT(hwa_dev->coreid  == HWA_CORE_ID); // hnd_soc.h
	HWA_ASSERT(hwa_dev->corerev == HWA_REVISION_ID); // hwa_regs.h, -DBCMHWA

	v32 = HWA_RD_REG_NAME(HWA00, hwa_dev->regs, top, hwahwcap2);
	BCM_REFERENCE(v32);
	HWA_ASSERT(BCM_GBF(v32, HWA_TOP_HWAHWCAP2_HWABLKSPRESENT) ==
		HWA_BLKS_PRESENT);

	// Attach HWA DMA engines/channels
	hwa_dma_attach(hwa_dev);

	for (i = 0; i < HWA_CALLBACK_MAX; i++) {
		hwa_register(hwa_dev, i, NULL, hwa_callback_noop);
	}

	// Allocate all HWA blocks state
	HWA_RXPATH_EXPR(hwa_rxpath_attach(hwa_dev)); // HWA1a HWA1b
	HWA_RXDATA_EXPR(hwa_rxdata_attach(hwa_dev)); // HWA2a

	HWA_TXPOST_EXPR(hwa_txpost_attach(hwa_dev)); // HWA3a
	HWA_TXFIFO_EXPR(hwa_txfifo_attach(hwa_dev)); // HWA3b
	HWA_TXSTAT_EXPR(hwa_txstat_attach(hwa_dev)); // HWA4a

	HWA_CPLENG_EXPR(hwa_cpleng_attach(hwa_dev)); // HWA2b HWA4b

	HWA_TRACE(("\n\n----- HWA ATTACH PHASE DONE -----\n\n"));

	return HWA_DEVP(TRUE);

fail_si_attach:

	hwa_detach(hwa_dev);
	HWA_WARN(("HWA Attach failure\n"));
	HWA_ASSERT(0);

	return (hwa_dev_t*)NULL;

} // hwa_attach

void
BCMATTACHFN(hwa_detach)(struct hwa_dev *dev)
{
	HWA_TRACE(("\n\n+++++ HWA DETACH PHASE BEGIN +++++\n\n"));

	HWA_RXPATH_EXPR(hwa_rxpath_detach(&dev->rxpath)); // HWA1a HWA1b
	HWA_RXDATA_EXPR(hwa_rxdata_detach(&dev->rxdata)); // HWA2a

	HWA_TXPOST_EXPR(hwa_txpost_detach(&dev->txpost)); // HWA3a
	HWA_TXFIFO_EXPR(hwa_txfifo_detach(&dev->txfifo)); // HWA3b
	HWA_TXSTAT_EXPR(hwa_txstat_detach(&dev->txstat)); // HWA4a

	HWA_CPLENG_EXPR(hwa_cpleng_detach(&dev->cpleng)); // HWA2b HWA4b

#ifdef HWA_DPC_BUILD
#ifdef DONGLEBUILD
	hwa_osl_detach(dev);
#endif /* DONGLEBUILD */
#endif /* HWA_DPC_BUILD */

	HWA_TRACE(("\n\n----- HWA DETACH PHASE DONE -----\n\n"));

} // hwa_detach

void // HWA CONFIG PHASE
hwa_config(struct hwa_dev *dev)
{
	uint32 v32, splithdr;
	hwa_regs_t *regs;

	HWA_TRACE(("\n\n+++++ HWA CONFIG PHASE BEGIN +++++\n\n"));
	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	regs = dev->regs;

#if !defined(BCMPCIEDEV) /* NIC mode SW driver */

	HWA_ASSERT(dev->driver_mode == HWA_NIC_MODE);

	dev->macif_placement = HWA_MACIF_IN_HOSTMEM; // Currently MACIf(s) in DDR
	dev->macif_coherency = HWA_HW_COHERENCY;

	// Following configuration should be fetched using the dev->osh OS layer
	dev->host_coherency  = HWA_HW_COHERENCY;
	dev->host_addressing = HWA_64BIT_ADDRESSING;
	dev->host_physaddrhi = HWA_INVALID_HIADDR;

	splithdr = 0U; // NIC mode uses contiguous packet

#else  /* defined BCMPCIEDEV: FullDongle mode SW driver */

	HWA_ERROR(("%s Host advertized capabilities<0x%08x>\n", HWA00, dev->pcie_ipc->hcap1));

	HWA_ASSERT(dev->driver_mode == HWA_FD_MODE);

	// Ensure Host has advertised capability in pcie_ipc structure
	HWA_ASSERT(dev->pcie_ipc != (struct pcie_ipc *)NULL);

	dev->macif_placement = HWA_MACIF_IN_DNGLMEM; // Always in device memory
	dev->macif_coherency = HWA_HW_COHERENCY;     // Always HW coherent

	// Determine Host coherency model using host advertised capability
	dev->host_coherency =
		(dev->pcie_ipc->hcap1 & PCIE_IPC_HCAP1_HW_COHERENCY) ?
		HWA_HW_COHERENCY : HWA_SW_COHERENCY;
	// Note: Dongle CPU complex is always assumed to be HW coherent.

	if (dev->pcie_ipc->hcap1 & PCIE_IPC_HCAP1_ADDR64) {
		dev->host_addressing = HWA_64BIT_ADDRESSING;
		dev->host_physaddrhi = HWA_INVALID_HIADDR;
	} else {
		dev->host_addressing = HWA_32BIT_ADDRESSING;
		dev->host_physaddrhi = // Use fixed HI address provided by Host
			HWA_HOSTADDR64_HI32(dev->pcie_ipc->host_physaddrhi);
	}

	dev->pcie_ipc_rings = // fetch the ring info passed via the pcie_ipc
		HWA_UINT2PTR(pcie_ipc_rings_t, dev->pcie_ipc->rings_daddr32);
	HWA_ASSERT(dev->pcie_ipc_rings != (pcie_ipc_rings_t*)NULL);

	HWA_ASSERT((dev->pcie_ipc->hcap1 & PCIE_IPC_HCAP1_ACWI) != 0);
	dev->wi_aggr_cnt = HWA_PCIEIPC_WI_AGGR_CNT; // WI format aggregation cnt
	HWA_TRACE(("%s wi_aggr_cnt<%u>\n", HWA00, dev->wi_aggr_cnt));

#ifdef PCIE_DMAINDEX16 /* dmaindex16 firmware target build */
	HWA_ASSERT((dev->pcie_ipc->flags & PCIE_IPC_FLAGS_DMA_INDEX) != 0);
	HWA_ASSERT((dev->pcie_ipc->flags & PCIE_IPC_FLAGS_2BYTE_INDICES) != 0);
	v32 = HWA_RD_REG_NAME(HWA00, regs, common, intr_control);
	v32 |= BCM_SBIT(HWA_COMMON_INTR_CONTROL_HOST_2BYTE_INDEX) |
		BCM_SBF(dev->host_coherency, HWA_COMMON_INTR_CONTROL_TEMPLATE_COHERENT);
	HWA_WR_REG_NAME(HWA00, regs, common, intr_control, v32);
#else
	HWA_WARN(("%s PCIE_DMAINDEX16 not defined\n", HWA00));
	HWA_ASSERT(0);
#endif /* PCIE_DMAINDEX16 */

	splithdr = 1U; // PCIE FD uses split header and data, later in host memory.

#endif /* defined BCMPCIEDEV: FullDongle mode SW driver */

	HWA_TRACE(("%s config mode<%s> macif<%s:%s> coh<%s> haddr<%s> hi32<0x%08x>\n",
		HWA00,
		(dev->driver_mode == HWA_FD_MODE) ? "FD" : "NIC",
		(dev->macif_placement == HWA_MACIF_IN_DNGLMEM) ? "DNGL" : "HOST",
		(dev->macif_coherency == HWA_HW_COHERENCY) ? "HW" : "SW",
		(dev->host_coherency == HWA_HW_COHERENCY) ? "HW" : "SW",
		(dev->host_addressing == HWA_64BIT_ADDRESSING) ? "64b" : "32b",
		dev->host_physaddrhi));

	// Configure the min threshold beyond which stalls are counted
	v32 = BCM_SBF(HWA_STATISTICS_MIN_BUSY,
	              HWA_COMMON_STATMINBUSYTIMEREG_STATSMINBUSYTIME);
	HWA_WR_REG_NAME(HWA00, regs, common, statminbusytimereg, v32);

	/*
	 * Configure whether 32bit or 64bit addresses will be specified
	 * The below complexity is intentionally introduced to significantly reduce
	 * the latency for HWA3b to promptly flush from Overflow queues to TxFIFO.
	 * This operation is deadline driven and address compression to reduce the
	 * size of a SW Tx packet (3b) is required. Memory savings is secondary.
	 *
	 *      -------------------------------------------------------------
	 *      |    capabilities    |      |  HWA3b SW Pkt Address fields  |
	 *      |sw_pkt32 | data_buf | MODE | pkt_next | hdr_buf | data_buf |
	 *      |---------|----------|------|----------|---------|----------|
	 *      |  0~     |     1*   |  FD* |    32b~  |   64b   |   64b    |
	 *      |  1      |     1*   |  FD* |    32b   |   32b   |   64b    |
	 *      |  0~     |     0    |  NIC |    64b~  |   64b   |    -     |
	 *      |  1      |     0    |  NIC |    32b   |   32b   |    -     |
	 *      -------------------------------------------------------------
	 *
	 * FD Mode:
	 * - data_buf_cap is always 1, implying a 64b data_buf_addr resides in host.
	 * - pkt_next is always 32b as dongle always uses 32bit addressing.
	 * - sw_pkt32_cap controls only hdr_buf_addr
	 *
	 * NIC Mode:
	 * - data_buf_cap is always 0 and SW Pkt does not have a data_buf_addr field
	 * - sw_pkt32_cap setting controls size of pkt_next and hdr_buf_addr
	 *
	 * When 32bit host addressing is enabled, a fixed hiaddr will be used to
	 * configure various descriptors.
	 */
	v32 = (0U
		| BCM_SBF(splithdr, HWA_COMMON_HWA2HWCAP_DATA_BUF_CAP)
		| BCM_SBF(dev->host_addressing,
		          HWA_COMMON_HWA2HWCAP_PCIE_IPC_PKT_ADDR32_CAP)
		| BCM_SBF((splithdr) ? splithdr : dev->host_addressing,
		          HWA_COMMON_HWA2HWCAP_SWPKT_ADDR32_CAP));
	HWA_WR_REG_NAME(HWA00, regs, common, hwa2hwcap, v32);

	// In NIC mode, setting pcieipc pkt high32 has no effect.

	// For TxFifo
	if (splithdr)
		v32 = 0;
	else
		v32 = dev->host_physaddrhi;
	HWA_WR_REG_NAME(HWA00, regs, common, hwa2swpkt_high32_pa, v32);

	// For RxFill
	v32 = dev->host_physaddrhi;
	HWA_WR_REG_NAME(HWA00, regs, common, hwa2pciepc_pkt_high32_pa, v32);

	HWA_RXPOST_EXPR(hwa_rxpost_preinit(&dev->rxpost));
	HWA_RXFILL_EXPR(hwa_rxfill_preinit(&dev->rxfill));

	HWA_TXPOST_EXPR({
		void *memory;
		uint32 mem_sz;
		uint32 txbuf_bytes;

		txbuf_bytes = HWA_TXPOST_PKT_BYTES + LBUFFRAGSZ;
		mem_sz = txbuf_bytes * HWA_TXPATH_PKTS_MAX;
		if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
			HWA_ERROR(("%s txbm malloc size<%u> failure\n", HWA00, mem_sz));
			HWA_ASSERT(memory != (void*)NULL);
			return;
		}
		HWA_TRACE(("%s txbm +memory[%p,%u]\n", HWA00, memory, mem_sz));
		hwa_bm_config(dev, &dev->tx_bm, "BM TxPATH", HWA_TX_BM,
			HWA_TXPATH_PKTS_MAX, txbuf_bytes,
			HWA_PTR2UINT(memory), HWA_PTR2HIADDR(memory), memory);
	});

	HWA_TRACE(("\n\n----- HWA CONFIG PHASE DONE -----\n\n"));

} // hwa_config

void // HWA INIT PHASE
hwa_init(hwa_dev_t *dev)
{
	uint32 v32;

	HWA_TRACE(("\n\n+++++ HWA INIT PHASE BEGIN +++++\n\n"));
	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(dev->regs != (hwa_regs_t*)NULL);

	v32 = 0U; // Disable module clk gating
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_clkgating_enable, v32);

	v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, module_clkext);
	HWA_ASSERT(v32 >= 3);

	// Enable clk to all blocks - not using hwa_module_request()
	v32 = 0U
		HWA_RXPATH_EXPR(
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCKRXCORE0_CLKENABLE)
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCKRXBM_CLKENABLE))
		HWA_CPLENG_EXPR(
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCKCPL_CLKENABLE))
		HWA_TXSTAT_EXPR(
			// Set block3B_clkEnable to enalbe 4A TxS internal memory region for DMA.
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCK3B_CLKENABLE)
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCKTXSTS_CLKENABLE))
		HWA_TXPOST_EXPR(
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCK3B_CLKENABLE)
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCK3A_CLKENABLE))
		HWA_TXFIFO_EXPR(
			| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCK3B_CLKENABLE))
		| BCM_SBF(1, HWA_COMMON_MODULE_CLK_ENABLE_BLOCKSTATISTICS_CLKENABLE);
	HWA_RXPATH_EXPR(
		if (HWA_RX_CORES > 1)
			v32 |= BCM_SBIT(HWA_COMMON_MODULE_CLK_ENABLE_BLOCKRXCORE1_CLKENABLE));
	HWA_TXSTAT_EXPR(
		if (HWA_TX_CORES > 1)
			v32 |= BCM_SBIT(HWA_COMMON_MODULE_CLK_ENABLE_BLOCKTXSTS1_CLKENABLE));
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_clk_enable, v32);

	// Buffer Managers enabled now
	HWA_RXFILL_EXPR(hwa_bm_init(&dev->rx_bm, TRUE));
	HWA_TXPOST_EXPR(hwa_bm_init(&dev->tx_bm, TRUE));

	// Initialize all blocks
	HWA_RXPOST_EXPR(hwa_rxpost_init(&dev->rxpost));
	HWA_RXFILL_EXPR(hwa_rxfill_init(&dev->rxfill));
	HWA_RXDATA_EXPR(hwa_rxdata_init(&dev->rxdata));

	HWA_TXPOST_EXPR(hwa_txpost_init(&dev->txpost));
	HWA_TXFIFO_EXPR(hwa_txfifo_init(&dev->txfifo));
	HWA_TXSTAT_EXPR(hwa_txstat_init(&dev->txstat));

	HWA_CPLENG_EXPR(hwa_cpleng_init(&dev->cpleng));

	dev->inited = TRUE;

	HWA_TRACE(("\n\n----- HWA INIT PHASE DONE -----\n\n"));

} // hwa_init

void // HWA DEINIT PHASE
hwa_deinit(hwa_dev_t *dev)
{
	HWA_TRACE(("\n\n+++++ HWA DEINIT PHASE BEGIN +++++\n\n"));
	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	// DeInitialize all blocks
	HWA_RXDATA_EXPR(hwa_rxdata_deinit(&dev->rxdata));
	HWA_RXFILL_EXPR(hwa_rxfill_deinit(&dev->rxfill));
	HWA_RXPOST_EXPR(hwa_rxpost_deinit(&dev->rxpost));

	/* Make sure all rxstatus are updated */
	HWA_RXPOST_EXPR(hwa_rxpath_flush_rxcomplete(dev));
	HWA_RXPOST_EXPR(hwa_wlc_mac_event(dev, WLC_E_HWA_RX_STOP));

	HWA_TXSTAT_EXPR(hwa_txstat_deinit(&dev->txstat));
	HWA_TXFIFO_EXPR(hwa_txfifo_deinit(&dev->txfifo));
	HWA_TXPOST_EXPR(hwa_txpost_deinit(&dev->txpost));

	HWA_CPLENG_EXPR(hwa_cpleng_deinit(&dev->cpleng));

	HWA_TRACE(("\n\n----- HWA DEINIT PHASE DONE -----\n\n"));

}

void
hwa_set_reinit(struct hwa_dev *dev)
{
	dev->reinit = TRUE;
}

void
hwa_reinit(hwa_dev_t *dev)
{
	uint32 v32;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	hwa_deinit(dev);

	hwa_init(dev);

	// DMA channels are enabled now
	hwa_dma_enable(&dev->dma);

	// Enable all blocks except 1a and 1b
	v32 = (0U
		HWA_CPLENG_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKCPL_ENABLE))
		HWA_TXPOST_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCK3A_ENABLE))
		HWA_TXFIFO_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCK3B_ENABLE))
		HWA_TXSTAT_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKTXSTS_ENABLE))
		| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKSTATISTICS_ENABLE));
	HWA_TXSTAT_EXPR(
		if (HWA_TX_CORES > 1)
			v32 |= BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKTXSTS1_ENABLE));
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_enable, v32);

	HWA_RXPOST_EXPR(hwa_wlc_mac_event(dev, WLC_E_HWA_RX_REINIT));

	dev->reinit = FALSE;

}

void // Enable 1a 1b blocks.
hwa_rx_enable(hwa_dev_t *dev)
{
	HWA_RXPATH_EXPR({
		uint32 v32;

		HWA_FTRACE(HWA00);

		HWA_ASSERT(dev != (hwa_dev_t*)NULL);

		// Enable 1a 1b
		v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, module_enable);
		v32 |= (BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKRXCORE0_ENABLE) |
			BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKRXBM_ENABLE));
		if (HWA_RX_CORES > 1)
			v32 |= BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKRXCORE1_ENABLE);
		HWA_WR_REG_NAME(HWA00, dev->regs, common, module_enable, v32);
	});
} // hwa_rx_enable

void // Enable all blocks. DMA engines and BMs are enabled in INIT Phase
hwa_enable(hwa_dev_t *dev)
{
	uint32 v32;

	HWA_TRACE(("\n\n+++++ HWA ENABLE PHASE BEGIN +++++\n\n"));
	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	// DMA channels are enabled now
	hwa_dma_enable(&dev->dma);

	// Enable all blocks except 1a 1b - not using hwa_module_request()
	v32 = (0U
		// RXDATA FHR and PktComparison are enabled in hwa_rxdata_init()
		HWA_CPLENG_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKCPL_ENABLE))
		HWA_TXPOST_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCK3A_ENABLE))
		HWA_TXFIFO_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCK3B_ENABLE))
		HWA_TXSTAT_EXPR(
			| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKTXSTS_ENABLE))
		| BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKSTATISTICS_ENABLE));
	HWA_TXSTAT_EXPR(
		if (HWA_TX_CORES > 1)
			v32 |= BCM_SBIT(HWA_COMMON_MODULE_ENABLE_BLOCKTXSTS1_ENABLE));
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_enable, v32);

	HWA_TRACE(("\n\n----- HWA ENABLE PHASE DONE -----\n\n"));
} // hwa_enable

void // Disable all blocks. DMA engines.
hwa_disable(hwa_dev_t *dev)
{
	uint32 v32;

	HWA_TRACE(("\n\n+++++ HWA DISABLE PHASE BEGIN +++++\n\n"));
	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	// Disable all blocks except 3a, and cpl engine.
	v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, module_enable);
	v32 &= (HWA_COMMON_MODULE_ENABLE_BLOCKCPL_ENABLE_MASK |
		HWA_COMMON_MODULE_ENABLE_BLOCK3A_ENABLE_MASK);
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_enable, v32);

	HWA_TRACE(("\n\n----- HWA DISABLE PHASE DONE -----\n\n"));
}

uint32 // Fetch AXI address of an HWA object given its offset
hwa_axi_addr(hwa_dev_t *dev, const uint32 hwa_mem_offset)
{
	uint32 cur_core_idx;
	uint32 axi_mem_addr = 0U;

	cur_core_idx = si_coreidx(dev->sih); // save current core

	if (si_setcore(dev->sih, dev->coreid, /* unit = */ 0) != NULL) {

		axi_mem_addr = // Get the HWA axi memory base
			si_addrspace(dev->sih, CORE_SLAVE_PORT_1, CORE_BASE_ADDR_0);

		HWA_ASSERT(axi_mem_addr == HWA_AXI_BASE_ADDR);
		axi_mem_addr += hwa_mem_offset;

	} else {
		HWA_ERROR(("%s: Failed to find HWA core<0x%08x>, offset<0x%08x>\n",
			__FUNCTION__, dev->coreid, hwa_mem_offset));
		ASSERT(0);
	}

	(void) si_setcoreidx(dev->sih, cur_core_idx); // restore current core

	HWA_TRACE(("%s hwa_mem_offset<%u> addr<0x%08x>\n",
		__FUNCTION__, hwa_mem_offset, axi_mem_addr));

	return axi_mem_addr;

} // hwa_axi_addr

// Errors reported within hwa::common::intstatus
#define HWA_INTSTATUS_ERRORS \
	(HWA_COMMON_INTSTATUS_ERROR_INT_MASK | \
	HWA_COMMON_INTSTATUS_TXSQUEUEFULL_INT_CORE0_MASK | \
	HWA_COMMON_INTSTATUS_TXSQUEUEFULL_INT_CORE1_MASK | \
	HWA_COMMON_INTSTATUS_CE2REG_INVALID_IND_UPDATE_MASK)

// Error reported within DMA Engine
#define HWA_DMA_ERRORS \
	(HWA_DMA_CH0INTSTATUS_DESCERR_MASK | \
	 HWA_DMA_CH0INTSTATUS_DATAERR_MASK | \
	 HWA_DMA_CH0INTSTATUS_DESCPROTOERR_MASK | \
	 HWA_DMA_CH0INTSTATUS_RCVDESCUF_MASK)

#define HWA_CHK_ERROR(blk, regname) \
({ \
	uint32 u32_val = HWA_RD_REG_NAME(HWA00, regs, blk, regname); \
	if (u32_val) HWA_WARN(("%s %s %s<0x%08x\n", HWA00, #blk, #regname, u32_val)); \
})

void // Lookup various error registers and dump. May be invoked in a poll timer
hwa_error(hwa_dev_t *dev)
{
	uint32 i, v32;
	hwa_regs_t *regs;

	if (dev == (hwa_dev_t*)NULL)
		return;

	regs = dev->regs;

	v32 = HWA_RD_REG_NAME(HWA00, regs, common, intstatus);
	if (v32 & HWA_INTSTATUS_ERRORS)
		HWA_WARN(("%s common intstatus<0x%08x>\n", HWA00, v32));
	HWA_CHK_ERROR(common, errorstatusreg);
	HWA_CHK_ERROR(common, directaxierrorstatus);

	for (i = 0; i < dev->dma.channels; i++) {
		HWA_CHK_ERROR(dma, chint[i].status);

		v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[i].tx.status1);
		if (BCM_GBF(v32, HWA_DMA_XMTSTATUS1_XMTERR))
			HWA_WARN(("%s dma xmt""status1<0x%08x>\n", HWA00, v32));
		v32 = HWA_RD_REG_NAME(HWAde, regs, dma, channels[i].rx.status1);
		if (BCM_GBF(v32, HWA_DMA_RCVSTATUS1_XMTERR)) // typo "XMT" ? RCVERR ...
			HWA_WARN(("%s dma rcv::status1<0x%08x>\n", HWA00, v32));
	}

} // hwa_error

uint32 // HWA clock control
hwa_clkctl_request(hwa_clkctl_cmd_t cmd, bool enable)
{
	uint32 top32; // dma32;
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit

	// Setup locals
	regs = dev->regs;

	top32 = HWA_RD_REG_NAME(HWA00, regs, top, clkctlstatus);
	// dma32 = HWA_RD_REG_NAME(HWA00, regs, dma, clockctlstatus);

	if (enable == TRUE) {
		// enable bit in clkctlstatus
		top32 |= (1 << cmd); // dma32 |= (1 << cmd);
	} else {
		if (cmd <= HWA_CLK_REQUEST_MAX) {
			// disable bit in clkctlstatus
			top32 &= ~(1 << cmd); // dma32 &= ~(1 << cmd);
		} else { // READ request of clk availability
			top32 = ((top32 & (1 << cmd)) >> cmd); // fetch current clkstatus
			goto done;
		}
	}

	HWA_WR_REG_NAME(HWA00, regs, top, clkctlstatus, top32);
	/*
	 * HWA::dma::clockctlstatus duplicated in HWA::top::clkctlstatus
	 * HWA::dma::clockctlstatus registers need not be programmed
	 *
	 * if (cmd < HWA_HWA_CLK_REQUEST)
	 *    HWA_WR_REG_NAME(HWA00, regs, dma, clockctlstatus, dma32);
	 */

done:
	HWA_TRACE(("%s clkctl cmd<%u> enable<%u> reg<0x%08x>\n",
		HWA00, cmd, enable, top32));

	return top32;

} // hwa_clkctl_request

uint32 // HWA power control
hwa_pwrctl_request(hwa_pwrctl_cmd_t cmd, uint32 arg_v32)
{
	uint32 v32;
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	dev = HWA_DEVP(FALSE); // CAUTION: global access without audit

	// Setup locals
	regs = dev->regs;

	v32 = HWA_RD_REG_NAME(HWA00, regs, top, powercontrol);
	switch (cmd) {
		case HWA_MEM_CLK_GATING:
			if (arg_v32)
				v32 |= BCM_SBF(arg_v32, HWA_TOP_POWERCONTROL_ENABLE_MEM_CLK_GATING);
			else
				v32 = BCM_CBF(v32, HWA_TOP_POWERCONTROL_ENABLE_MEM_CLK_GATING);
			break;
		case HWA_POWER_KEEP_ON:
			if (arg_v32)
				v32 |= BCM_SBF(arg_v32, HWA_TOP_POWERCONTROL_POWERCTL);
			else
				v32 = BCM_CBF(v32, HWA_TOP_POWERCONTROL_POWERCTL);
			break;
		default:
			break;
	}

	HWA_WR_REG_NAME(HWA00, regs, top, powercontrol, v32);
	HWA_TRACE(("%s pwrctl cmd<%u> arg_v32<%u> reg<0x%08x>\n",
	           HWA00, cmd, arg_v32, v32));

	return v32;

} // hwa_pwrctl_request

uint32 // Control a HWA module
hwa_module_request(hwa_dev_t *dev, hwa_module_block_t blk,
	hwa_module_cmd_t cmd, bool enable)
{
	uint32 v32;
	hwa_regs_t *regs;

	// Audit parameters and pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	regs = dev->regs;

	switch (cmd) {
		case HWA_MODULE_CLK_ENABLE:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, module_clk_enable);
			break;
		case HWA_MODULE_CLK_GATING:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, module_clkgating_enable);
			break;
		case HWA_MODULE_RESET:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, module_reset);
			break;
		case HWA_MODULE_CLK_AVAIL:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, module_clkavail);
			v32 = ((v32 & (1 << blk)) >> blk);
			goto done;
		case HWA_MODULE_ENABLE:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, module_enable);
			break;
		case HWA_MODULE_IDLE:
			v32 = HWA_RD_REG_NAME(HWA00, regs, common, module_idle);
			v32 = ((v32 & (1 << blk)) >> blk);
			goto done;
		default:
			HWA_WARN(("%s module blk<%u> cmd<%u> enable<%u> failure\n",
				HWA00, blk, cmd, enable));
			return -1;
	}

	if (enable == TRUE) {
		v32 |= (1 << blk); // enable bit for requested module block
	} else {
		v32 &= ~(1 << blk); // disable bit for requested module block
	}

	switch (cmd) {
		case HWA_MODULE_CLK_ENABLE:
			HWA_WR_REG_NAME(HWA00, regs, common, module_clk_enable, v32);
			break;
		case HWA_MODULE_CLK_GATING:
			HWA_WR_REG_NAME(HWA00, regs, common, module_clkgating_enable, v32);
			break;
		case HWA_MODULE_RESET:
			HWA_WR_REG_NAME(HWA00, regs, common, module_reset, v32); break;
		case HWA_MODULE_ENABLE:
			HWA_WR_REG_NAME(HWA00, regs, common, module_enable, v32); break;
		default:
			return -1;
	}

done:
	HWA_TRACE(("%s module blk<%u> cmd<%u> enable<%u> reg<0x%08x>\n",
		HWA00, blk, cmd, enable, v32));

	return v32;

} // hwa_module_request

/**
 * HWA2.0 only provides a single Completion Ring interrupt. Runner requires
 * a unique TxCpl<addr,val> and RxCpl<addr,val> mechanism to allow TxCompletions
 * and RxCompletions to be redirected to explicit Runner Cores on which explicit
 * Runner HW threads will be kick started.
 *
 * For RxPost RD index update and TxPost RD index update, presently, dongle does
 * not issue a doorbell interrupt. DHD uses Lazy RD fetch for RxPost and TxPost
 * production. If dongle needs to explicitly deliver an interrupt on RD updates
 * for RxPost or TxPost, then the legact PCIE doorbell FN0 register's address
 * and a dummy value (0xdeadbeef) may be registered.
 */
void // Register a software doorbell
hwa_sw_doorbell_request(struct hwa_dev *dev, hwa_sw_doorbell_t request,
	uint32 index, dma64addr_t haddr64, uint32 value)
{
	uint32 v32;
	hwa_regs_t *regs;

	HWA_TRACE(("%s sw doorbell<%u> haddr64<0x%08x,0x%08x> value<0x%08x>\n",
		HWA00, request, haddr64.hiaddr, haddr64.loaddr, value));

	// Audit parameters and pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	regs = dev->regs;

	v32 = HWA_RD_REG_NAME(HWA00, regs, common, intr_control);

	switch (request) {
		case HWA_TX_DOORBELL:
			HWA_WR_REG_NAME(HWA00, regs, common, txintraddrlo, haddr64.loaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, txintraddrhi, haddr64.hiaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, txintrval, value);
			v32 |= (0U
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_TXHOSTINTR_EN)
#if HWA_REVISION_EQ_128
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_USEVAL)
#else
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_USEVAL_TX)
#endif // endif
				| 0U);
			break;

		case HWA_RX_DOORBELL:
			HWA_WR_REG_NAME(HWA00, regs, common, rxintraddrlo, haddr64.loaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, rxintraddrhi, haddr64.hiaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, rxintrval, value);
			v32 |= (0U
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_RXHOSTINTR_EN)
#if HWA_REVISION_EQ_128
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_USEVAL)
#else
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_USEVAL_RX)
#endif // endif
				| 0U);
			break;

		case HWA_TXCPL_DOORBELL:
#if HWA_REVISION_EQ_128
			HWA_WR_REG_NAME(HWA00, regs, common, cplintraddrlo, haddr64.loaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintraddrhi, haddr64.hiaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintrval, value);
#else
			HWA_WR_REG_NAME(HWA00, regs, common, cplintr_tx_addrlo, haddr64.loaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintr_tx_addrhi, haddr64.hiaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintr_tx_val, value);
#endif // endif
			v32 |= (0U
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_CPLHOSTINTR_EN)
#if HWA_REVISION_EQ_128
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_USEVAL)
#else
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_USEVAL_CPLTX)
#endif // endif
				| 0U);

			break;

		case HWA_RXCPL_DOORBELL:
		{
#if HWA_REVISION_GE_129
			uint32 useval_cplrx;
#endif // endif
#if HWA_REVISION_EQ_128
			HWA_WR_REG_NAME(HWA00, regs, common, cplintraddrlo, haddr64.loaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintraddrhi, haddr64.hiaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintrval, value);
#else
			HWA_ASSERT(index < 4);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintr_rx[index].addrlo,
				haddr64.loaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintr_rx[index].addrhi,
				haddr64.hiaddr);
			HWA_WR_REG_NAME(HWA00, regs, common, cplintr_rx[index].val, value);
			useval_cplrx = 1 << index;
#endif // endif
			v32 |= (0U
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_CPLHOSTINTR_EN)
#if HWA_REVISION_EQ_128
				| BCM_SBIT(HWA_COMMON_INTR_CONTROL_USEVAL)
#else
				| BCM_SBF(useval_cplrx, HWA_COMMON_INTR_CONTROL_USEVAL_CPLRX)
#endif // endif
				| 0U);
			break;
		}

		default: HWA_WARN(("%s doorbell<%u> failure\n", HWA00, request));
			HWA_ASSERT(0);
	}

	v32 |= BCM_SBF(dev->host_coherency, HWA_COMMON_INTR_CONTROL_TEMPLATE_COHERENT);
	HWA_WR_REG_NAME(HWA00, regs, common, intr_control, v32);

} // hwa_sw_doorbell_request

#if defined(BCMDBG)

void // Dump all HWA blocks
hwa_dump(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 block_bitmap,
	bool verbose, bool dump_regs, bool dump_txfifo_shadow, uint8 *fifo_bitmap)
{
	if (dev == (hwa_dev_t*)NULL)
		return;

	HWA_BPRINT(b, "HWA[%s] rev<%d> driver<%d> dev<%p> regs<%p> "
		"intmask<0x%08x> defintmask<0x%08x> intstatus<0x%08x> "
		"core id<%u> rev<%u>\n",
		HWA_FAMILY, HWA_REVISION_ID, HWA_DRIVER_VERSION,
		dev, dev->regs, dev->intmask, dev->defintmask, dev->intstatus,
		dev->coreid, dev->corerev);
	HWA_BPRINT(b, "HWA [ ");
	HWA_RXPOST_EXPR(HWA_BPRINT(b, "1A ")); HWA_RXFILL_EXPR(HWA_BPRINT(b, "1B "));
	HWA_RXDATA_EXPR(HWA_BPRINT(b, "2A ")); HWA_RXCPLE_EXPR(HWA_BPRINT(b, "2B "));
	HWA_TXPOST_EXPR(HWA_BPRINT(b, "3A ")); HWA_TXFIFO_EXPR(HWA_BPRINT(b, "3B "));
	HWA_TXSTAT_EXPR(HWA_BPRINT(b, "4A ")); HWA_TXCPLE_EXPR(HWA_BPRINT(b, "4B "));
	HWA_BPRINT(b, "] blocks are enabled\n");

	HWA_BUS_EXPR(
		HWA_BPRINT(b, "+ osh<%p> sih<%p> pcie_sh<%p> pcie_ipc_rings<%p> wi_aggr<%u>\n",
			dev->osh, dev->sih, dev->pcie_ipc,
			dev->pcie_ipc_rings, dev->wi_aggr_cnt));

	HWA_BPRINT(b, "+ driver_mode<%s> macif<%s, %s> host<%s, %s, 0x%08x>\n",
		(dev->driver_mode == HWA_FD_MODE) ? "FD" : "NIC",
		(dev->macif_placement == HWA_MACIF_IN_DNGLMEM) ? "DNGL" : "HOST",
		(dev->macif_coherency == HWA_HW_COHERENCY) ? "HW" : "SW",
		(dev->host_coherency == HWA_HW_COHERENCY) ? "HW" : "SW",
		(dev->host_addressing == HWA_64BIT_ADDRESSING) ? "64b" : "32b",
		dev->host_physaddrhi);

	if (verbose) {
		int cb;
		for (cb = 0; cb < HWA_CALLBACK_MAX; cb++) {
			HWA_BPRINT(b, "+ hwa_handler %u <%p, %p>\n",
				cb, dev->handlers[cb].context, dev->handlers[cb].callback);
		}
	}

#if defined(WLTEST)
	if (dump_regs) {
		hwa_regs_dump(dev, b,
			(block_bitmap & (HWA_DUMP_TOP|HWA_DUMP_CMN))); // top and common only
	}
#endif // endif

	if (block_bitmap & HWA_DUMP_DMA) {
		hwa_dma_dump(&dev->dma, b, verbose, dump_regs); // dump DMA channels
	}
	if (block_bitmap & (HWA_DUMP_1A|HWA_DUMP_1B)) {
		HWA_RXFILL_EXPR(hwa_bm_dump(&dev->rx_bm, b, verbose, dump_regs)); // HWA rx_bm
	}
	if (block_bitmap & (HWA_DUMP_3A|HWA_DUMP_3B)) {
		HWA_TXPOST_EXPR(hwa_bm_dump(&dev->tx_bm, b, verbose, dump_regs));
	}
	if (block_bitmap & HWA_DUMP_1A) {
		HWA_RXPOST_EXPR(hwa_rxpost_dump(&dev->rxpost, b, verbose)); // HWA1a
	}
	if (block_bitmap & HWA_DUMP_1B) {
		HWA_RXFILL_EXPR(hwa_rxfill_dump(&dev->rxfill, b, verbose)); // HWA1b
	}
	if (block_bitmap & (HWA_DUMP_1A|HWA_DUMP_1B)) {
		HWA_RXPATH_EXPR(hwa_rxpath_dump(&dev->rxpath, b, verbose, dump_regs)); // HWA1x
	}
	if (block_bitmap & HWA_DUMP_2A) {
		HWA_RXDATA_EXPR(hwa_rxdata_dump(&dev->rxdata, b, verbose)); // HWA2a
	}
	if (block_bitmap & HWA_DUMP_3A) {
		HWA_TXPOST_EXPR(hwa_txpost_dump(&dev->txpost, b, verbose, dump_regs)); // HWA3a
	}
	if (block_bitmap & HWA_DUMP_3B) {
		HWA_TXFIFO_EXPR(hwa_txfifo_dump(&dev->txfifo, b, verbose, dump_regs,
			dump_txfifo_shadow, fifo_bitmap)); // HWA3b
	}
	if (block_bitmap & HWA_DUMP_4A) {
		HWA_TXSTAT_EXPR(hwa_txstat_dump(&dev->txstat, b, verbose, dump_regs)); // HWA4a
	}
	if (block_bitmap & (HWA_DUMP_2B|HWA_DUMP_4B)) {
		HWA_CPLENG_EXPR(hwa_cpleng_dump(&dev->cpleng, b, verbose, dump_regs)); // HWAce
	}
} // hwa_dump

#if defined(WLTEST)

void // Dump all HWA block registers or just top and common blocks
hwa_regs_dump(hwa_dev_t *dev, struct bcmstrbuf *b, uint32 block_bitmap)
{
	hwa_regs_t *regs;

	if (dev == (hwa_dev_t*)NULL)
		return;

	regs = dev->regs;

	HWA_BPRINT(b, "HWA[%s] rev<%d> driver<%d> dev<%p> registers<%p>\n",
		HWA_FAMILY, HWA_REVISION_ID, HWA_DRIVER_VERSION, dev, regs);

	if (block_bitmap & HWA_DUMP_TOP) {
		// TOP block
		hwa_top_regs_t *top = &regs->top;
		HWA_BPRINT(b, "HWA Top: registers[%p] offset[0x%04x]\n",
			top, (uint32)OFFSETOF(hwa_regs_t, top));
		HWA_BPR_REG(b, top, debug_fifobase);
		HWA_BPR_REG(b, top, debug_fifodata_lo);
		HWA_BPR_REG(b, top, debug_fifodata_hi);
		HWA_BPR_REG(b, top, clkctlstatus);
		HWA_BPR_REG(b, top, workaround);
		HWA_BPR_REG(b, top, powercontrol);
		HWA_BPR_REG(b, top, hwahwcap1);
		HWA_BPR_REG(b, top, hwahwcap2);
	}

	if (block_bitmap & HWA_DUMP_CMN) {
		// COMMON block
		hwa_common_regs_t *common = &regs->common;
		HWA_BPRINT(b, "%s registers[%p] offset[0x%04x]\n",
			HWA00, common, (uint32)OFFSETOF(hwa_regs_t, common));
		HWA_BPR_REG(b, common, rxpost_wridx_r0);
		HWA_BPR_REG(b, common, rxpost_wridx_r1);
		HWA_BPR_REG(b, common, intstatus);
		HWA_BPR_REG(b, common, intmask);
		HWA_BPR_REG(b, common, statscontrolreg);
		HWA_BPR_REG(b, common, statdonglreaddressreg);
		HWA_BPR_REG(b, common, statminbusytimereg);
		HWA_BPR_REG(b, common, errorstatusreg);
		HWA_BPR_REG(b, common, directaxierrorstatus);
		HWA_BPR_REG(b, common, statdonglreaddresshireg);
		HWA_BPR_REG(b, common, txintraddr_lo);
		HWA_BPR_REG(b, common, txintraddr_hi);
		HWA_BPR_REG(b, common, rxintraddr_lo);
		HWA_BPR_REG(b, common, rxintraddr_hi);
#if HWA_REVISION_EQ_128
		HWA_BPR_REG(b, common, cplintraddr_lo);
		HWA_BPR_REG(b, common, cplintraddr_hi);
		HWA_BPR_REG(b, common, cplintrval);
#else
		HWA_BPR_REG(b, common, cplintr_tx_addr_lo);
		HWA_BPR_REG(b, common, cplintr_tx_addr_hi);
		HWA_BPR_REG(b, common, cplintr_tx_val);
#endif // endif
		HWA_BPR_REG(b, common, intr_control);
		HWA_BPR_REG(b, common, module_clk_enable);
		HWA_BPR_REG(b, common, module_clkgating_enable);
		HWA_BPR_REG(b, common, module_reset);
		HWA_BPR_REG(b, common, module_clkavail);
		HWA_BPR_REG(b, common, module_clkext);
		HWA_BPR_REG(b, common, module_enable);
		HWA_BPR_REG(b, common, module_idle);
		HWA_BPR_REG(b, common, gpiomuxcfg);
		HWA_BPR_REG(b, common, gpioout);
		HWA_BPR_REG(b, common, gpiooe);
		HWA_BPR_REG(b, common, mac_base_addr_core0);
		HWA_BPR_REG(b, common, mac_base_addr_core1);
		HWA_BPR_REG(b, common, mac_frmtxstatus);
		HWA_BPR_REG(b, common, mac_dma_ptr);
		HWA_BPR_REG(b, common, mac_ind_xmtptr);
		HWA_BPR_REG(b, common, mac_ind_qsel);
		HWA_BPR_REG(b, common, hwa2hwcap);
		HWA_BPR_REG(b, common, hwa2swpkt_high32_pa);
		HWA_BPR_REG(b, common, hwa2pciepc_pkt_high32_pa);
#if HWA_REVISION_GE_129
		HWA_BPR_REG(b, common, cplintr_rx[0].addr_lo);
		HWA_BPR_REG(b, common, cplintr_rx[0].addr_hi);
		HWA_BPR_REG(b, common, cplintr_rx[0].val);
		HWA_BPR_REG(b, common, cplintr_rx[1].addr_lo);
		HWA_BPR_REG(b, common, cplintr_rx[1].addr_hi);
		HWA_BPR_REG(b, common, cplintr_rx[1].val);
		HWA_BPR_REG(b, common, cplintr_rx[2].addr_lo);
		HWA_BPR_REG(b, common, cplintr_rx[2].addr_hi);
		HWA_BPR_REG(b, common, cplintr_rx[2].val);
		HWA_BPR_REG(b, common, cplintr_rx[3].addr_lo);
		HWA_BPR_REG(b, common, cplintr_rx[3].addr_hi);
		HWA_BPR_REG(b, common, cplintr_rx[3].val);
#endif // endif
	}

	// DMA
	if (block_bitmap & HWA_DUMP_DMA) {
		hwa_dma_regs_dump(&dev->dma, b);
	}

	// RXBM, TXBM
	if (block_bitmap & (HWA_DUMP_1A | HWA_DUMP_1B)) {
		HWA_RXFILL_EXPR(hwa_bm_regs_dump(&dev->rx_bm, b));
	}
	if (block_bitmap & (HWA_DUMP_3A | HWA_DUMP_3B)) {
		HWA_TXPOST_EXPR(hwa_bm_regs_dump(&dev->tx_bm, b));
	}

	// RXPATH(RXPOST, RXFIFO)
	if (block_bitmap & (HWA_DUMP_1A|HWA_DUMP_1B)) {
		HWA_RXPATH_EXPR(hwa_rxpath_regs_dump(&dev->rxpath, b));
	}

	// TXPOST, TXFIFO and TXSTAT
	if (block_bitmap & HWA_DUMP_3A) {
		HWA_TXPOST_EXPR(hwa_txpost_regs_dump(&dev->txpost, b));
	}
	if (block_bitmap & HWA_DUMP_3B) {
		HWA_TXFIFO_EXPR(hwa_txfifo_regs_dump(&dev->txfifo, b));
	}
	if (block_bitmap & HWA_DUMP_4A) {
		HWA_TXSTAT_EXPR(hwa_txstat_regs_dump(&dev->txstat, b));
	}

	// TxCPLE and RxCPLE
	if (block_bitmap & (HWA_DUMP_2B|HWA_DUMP_4B)) {
		HWA_CPLENG_EXPR(hwa_cpleng_regs_dump(&dev->cpleng, b));
	}
} // hwa_regs_dump

void // Read a single register in HWA given its offset in HWA regs
hwa_reg_read(hwa_dev_t *dev, uint32 reg_offset)
{
	uint32 v32;
	uintptr reg_base;

	if (dev == (hwa_dev_t*)NULL)
		return;

	reg_base = HWA_PTR2UINT(dev->regs);
	v32 = HWA_RD_REG((uintptr)reg_base + reg_offset);

	HWA_PRINT("HWA_REGISTER[0x%p] offset<0x%04x> v32[0x%08x, %10u]\n",
		(void*)(reg_base + reg_offset), reg_offset, v32, v32);

} // hwa_reg_read

#endif // endif

#endif /* BCMDBG */

#if defined(WLTEST)
static int
hwa_dbg_get_regaddr(hwa_regs_t *regs, char *type, uintptr *reg_addr)
{
	*reg_addr = 0;

	if (!strcmp(type, "top")) {
		// From regs@(offset 0) instead of top@(offset 0x104)
		*reg_addr = HWA_PTR2UINT(regs);
	} else if (!strcmp(type, "cmn")) {
		*reg_addr = HWA_PTR2UINT(&regs->common);
	} else if (!strcmp(type, "dma")) {
		*reg_addr = HWA_PTR2UINT(&regs->dma);
	} else if (!strcmp(type, "rx")) {
		HWA_RXPATH_EXPR(*reg_addr = HWA_PTR2UINT(&regs->rx_core[0]));  //core 0
	} else if (!strcmp(type, "rxbm")) {
		HWA_RXPATH_EXPR(*reg_addr = HWA_PTR2UINT(&regs->rx_bm));
	} else if (!strcmp(type, "tx")) {
		HWA_TXPOST_EXPR(*reg_addr = HWA_PTR2UINT(&regs->tx));
	} else if (!strcmp(type, "txbm")) {
		HWA_TXPOST_EXPR(*reg_addr = HWA_PTR2UINT(&regs->tx_bm));
	} else if (!strcmp(type, "txdma")) {
		HWA_TXFIFO_EXPR(*reg_addr = HWA_PTR2UINT(&regs->txdma));
	} else if (!strcmp(type, "txs")) {
		HWA_TXSTAT_EXPR(*reg_addr = HWA_PTR2UINT(&regs->tx_status[0])); //core 0
	} else if (!strcmp(type, "cpl")) {
		HWA_CPLENG_EXPR(*reg_addr = HWA_PTR2UINT(&regs->cpl));
	} else {
		return BCME_BADARG;
	}

	if (*reg_addr == 0)
		return BCME_BADARG;

	return BCME_OK;
}

int
hwa_dbg_regread(struct hwa_dev *dev, char *type, uint32 reg_offset, int32 *ret_int_ptr)
{
	int ret = BCME_OK;
	uintptr reg_addr;

	if (dev == (hwa_dev_t*)NULL)
		return BCME_ERROR;

	ret = hwa_dbg_get_regaddr(dev->regs, type, &reg_addr);
	if (ret != BCME_OK)
		return ret;

	*ret_int_ptr = HWA_RD_REG((uintptr)reg_addr + reg_offset);

	return ret;
}

int
hwa_dbg_regwrite(struct hwa_dev *dev, char *type, uint32 reg_offset, uint32 val)
{
	int ret = BCME_OK;
	uintptr reg_addr;

	if (dev == (hwa_dev_t*)NULL)
		return BCME_ERROR;

	ret = hwa_dbg_get_regaddr(dev->regs, type, &reg_addr);
	if (ret != BCME_OK)
		return ret;

	HWA_WR_REG(((uintptr)reg_addr + reg_offset), val);

	return BCME_OK;
}
#endif // endif

#ifdef HWA_DPC_BUILD
/* ========= Common ISR/DPC handle functions ========= */
void
hwa_intrson(hwa_dev_t *dev)
{
	uint32 v32;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(dev->regs != (hwa_regs_t*)NULL);

	dev->intmask = dev->defintmask;
	v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, intmask);
	v32 |= dev->intmask;
	HWA_WR_REG_NAME(HWA00, dev->regs, common, intmask, v32);
}

void
hwa_intrsoff(hwa_dev_t *dev)
{
	uint32 v32;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(dev->regs != (hwa_regs_t*)NULL);

	v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, intmask);
	v32 &= ~dev->intmask;
	HWA_WR_REG_NAME(HWA00, dev->regs, common, intmask, v32);

	dev->intmask = 0;
}

void
hwa_intrsupd(hwa_dev_t *dev)
{
	uint32 intstatus;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	/* read and clear intstatus */
	intstatus = hwa_intstatus(dev);

	/* update interrupt status in software */
	dev->intstatus |= intstatus;
}

uint32
hwa_intstatus(hwa_dev_t *dev)
{
	uint32 v32;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(dev->regs != (hwa_regs_t*)NULL);

	v32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, intstatus);
	v32 &= dev->defintmask;
	if (v32) {
		HWA_WR_REG_NAME(HWA00, dev->regs, common, intstatus, v32);
		(void)HWA_RD_REG_NAME(HWA00, dev->regs, common, intstatus); /* sync readback */
	}

	return v32;
}

bool
hwa_dispatch(hwa_dev_t *dev)
{
	uint32 intstatus;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	/* read and clear intstatus */
	intstatus = hwa_intstatus(dev);

	/* assign interrupt status in software */
	dev->intstatus = intstatus;

	return intstatus ? TRUE : FALSE;
}

bool
hwa_txdma_dpc(hwa_dev_t *dev, uint32 intstatus)
{
	return FALSE;
}

/* Run DPC,
 * Return TRUE : need re-schedule
 * Return FALSE: no re-schedule
 */
bool
hwa_dpc(hwa_dev_t *dev)
{
	uint32 intstatus;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	intstatus = dev->intstatus;
	dev->intstatus = 0;

	if (intstatus == 0)
		return FALSE;  // no re-schedule

	// PCIE face.
#ifdef HWA_RXPOST_ONLY_BUILD
	// HWA1a only RxPost, intr-bit1
	if (intstatus & HWA_COMMON_INTSTATUS_RXPDEST0_INT_MASK) {
		(void)hwa_rxpost_process(dev);
	}
#endif // endif

#ifdef HWA_TXPOST_BUILD
	// HWA3a TxPOST, intr-bit18
	if (intstatus & HWA_COMMON_INTSTATUS_TXPKTCHN_INT_MASK) {
		(void)hwa_txpost_pktchain_process(dev);
	}
#endif // endif

	if ((!dev->up) || (dev->reinit)) {
		goto done;
	}

	// MAC face.
#ifdef HWA_RXFILL_BUILD
	// HWA1b RxFILL, intr-bit2
	if (intstatus & HWA_COMMON_INTSTATUS_D11BDEST0_INT_MASK) {
		(void)hwa_rxfill_rxbuffer_process(dev, 0, HWA_PROCESS_BOUND); // core 0
	}
	// HWA1b RxFILL, intr-bit6
	if (intstatus & HWA_COMMON_INTSTATUS_D11BDEST1_INT_MASK) {
		(void)hwa_rxfill_rxbuffer_process(dev, 1, HWA_PROCESS_BOUND); // core 1
	}
#endif // endif

#ifdef HWA_TXSTAT_BUILD
	// HWA4a TxSTAT, intr-bit17
	if (intstatus & HWA_COMMON_INTSTATUS_TXSWRINDUPD_INT_CORE0_MASK) {
		(void)hwa_txstat_process(dev, 0, HWA_PROCESS_BOUND); // core 0
	}
	// HWA4a TxSTAT, intr-bit28
	if (intstatus & HWA_COMMON_INTSTATUS_TXSWRINDUPD_INT_CORE1_MASK) {
		(void)hwa_txstat_process(dev, 1, HWA_PROCESS_BOUND); // core 1
	}
#endif // endif
done:
	/* In each dpc functions they may update the dev->intstatus to request re-schedule */
	return (dev->intstatus) ? TRUE : FALSE;
}
#endif /* HWA_DPC_BUILD */
