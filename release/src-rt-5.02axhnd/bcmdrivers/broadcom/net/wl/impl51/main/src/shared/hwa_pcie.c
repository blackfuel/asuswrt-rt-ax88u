/*
 * HWA library routines for PCIE facing blocks: HWA1a, HWA2b, HWA3a, and HWA4b
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
#include <bcmendian.h>
#include <dngl_api.h>
#include <pciedev.h>
#include <bcmmsgbuf.h>
#include <hwa_lib.h>
#include <d11_cfg.h>

//+ ----------------------------------------------------------------------------

#ifdef HWA_RXPOST_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: HWA1a RxPost Block
 * -----------------------------------------------------------------------------
 *
 * CONFIG:
 * -------
 * - hwa_rxpost_init() configures the HWA1a RxPost Host2Dongle Ring. Currently
 *   only a single RxPost ring is supported. May be extended to multiple RxPost
 *   rings, dedicated per MAC Core, by reserving multiple ring index in the PCIE
 *   Full Dongle IPC. HWA-2.0 does not support seqnum auditing.
 *   In the event that HWA1b RxFill block is not built, a h2s RxPost interface
 *   is instantiated to transfer RxPost work items to software.
 *
 * RUNTIME:
 * --------
 * - hwa_rph_allocate() - When HWA1b is also enabled, SW may allocated a RPH
 *   that was posted by the Host. This would allow SW to construct an 802.3
 *   packet in dongle for reception in the host. RPH may NOT be used for dongle
 *   originating events or ioctls. PCIE FD specifies explicit buffers for ioctls
 *   and events. There is no use case currently for dongle SW autonomously
 *   sourcing Rx packets to host.
 *
 * DEBUG:
 * ------
 * - hwa_rxpost_dump()
 *
 * -----------------------------------------------------------------------------
 */

#if (HWA_AGGR_MAX != 4)
#error "HWA-2.0 1a RxPost WI is bounded to a max aggregation of size 4"
#endif // endif

// Format of RxPort work item. Aggregate forms default to HWA_RXPOST_AGGR_MAX
typedef enum hwa_rxpost_wi_format
{
	HWA_RXPOST_CWI32_FORMAT  = 0,       // !Aggregate Compact 32 bit haddr
	HWA_RXPOST_CWI64_FORMAT  = 1,       // !Aggregate Compact 64 bit haddr
	HWA_RXPOST_ACWI32_FORMAT = 2,       //  Aggregate Compact 32 bit haddr
	HWA_RXPOST_ACWI64_FORMAT = 3,       //  Aggregate Compact 64 bit haddr
	HWA_RXPOST_WI_FORMAT_MAX = 4
} hwa_rxpost_wi_format_t;

#ifdef HWA_RXPOST_ONLY_BUILD
// Next 4 functions are customized for RxPost CWI32, CWI64, ACWI32 and ACWI64

// Handlers for parsing RxPost WI formats: CWI32, CWI64, ACWI32 and ACWI64
static uint32 hwa_rxpost_cwi32_parser(hwa_rxpost_t *rxpost,
	hwa_ring_t *rxpost_ring, uint32 elem_ix, hwa_handler_t *rxpost_handler);
static uint32 hwa_rxpost_cwi64_parser(hwa_rxpost_t *rxpost,
	hwa_ring_t *rxpost_ring, uint32 elem_ix, hwa_handler_t *rxpost_handler);
static uint32 hwa_rxpost_acwi32_parser(hwa_rxpost_t *rxpost,
	hwa_ring_t *rxpost_ring, uint32 elem_ix, hwa_handler_t *rxpost_handler);
static uint32 hwa_rxpost_acwi64_parser(hwa_rxpost_t *rxpost,
	hwa_ring_t *rxpost_ring, uint32 elem_ix, hwa_handler_t *rxpost_handler);
#else /* !HWA_RXPOST_ONLY_BUILD */
// No handlers required when RxFill is built
#define hwa_rxpost_cwi32_parser  ((hwa_rxpost_wi_parser_fn_t)NULL)
#define hwa_rxpost_cwi64_parser  ((hwa_rxpost_wi_parser_fn_t)NULL)
#define hwa_rxpost_acwi32_parser ((hwa_rxpost_wi_parser_fn_t)NULL)
#define hwa_rxpost_acwi64_parser ((hwa_rxpost_wi_parser_fn_t)NULL)
#endif /* !HWA_RXPOST_ONLY_BUILD */

// HWA1a possible combinations of RxPost WorkItems and their sizes
// HWA-2.0: ~0 is used to indicate undefined offsets of length or address fields
const hwa_rxpost_config_t hwa_rxpost_config[HWA_RXPOST_WI_FORMAT_MAX] =
{
	{ hwa_rxpost_cwi32_parser, (uint8)HWA_RXPOST_CWI32_FORMAT,
		(uint8)sizeof(hwa_rxpost_cwi32_t),   ~0,  4, "CWI32" },
	{ hwa_rxpost_cwi64_parser, (uint8)HWA_RXPOST_CWI64_FORMAT,
		(uint8)sizeof(hwa_rxpost_cwi64_t),   ~0,  8, "CWI64" },
	{ hwa_rxpost_acwi32_parser, (uint8)HWA_RXPOST_ACWI32_FORMAT,
		(uint8)sizeof(hwa_rxpost_acwi32_t), ~0, ~0, "ACWI32" },
	{ hwa_rxpost_acwi64_parser, (uint8)HWA_RXPOST_ACWI64_FORMAT,
		(uint8)sizeof(hwa_rxpost_acwi64_t), ~0, ~0, "ACWI64" }
};

static void hwa_rxpost_reclaim(hwa_rxpost_t *rxpost, uint32 core);

int // HWA1a: Allocate resources configuration for HWA1a block
hwa_rxpost_preinit(hwa_rxpost_t *rxpost)
{
	hwa_dev_t *dev;

	HWA_FTRACE(HWA1a);

	// Audit pre-conditions
	dev = HWA_DEV(rxpost);

	// Setup locals

	// PCIE IPC capability advertized by host: haddr and wi aggr
	if (dev->host_addressing == HWA_32BIT_ADDRESSING) { // 32bit host
		rxpost->config = (const hwa_rxpost_config_t*)
			((dev->wi_aggr_cnt == 0) ?
			 &hwa_rxpost_config[HWA_RXPOST_CWI32_FORMAT] :
			 &hwa_rxpost_config[HWA_RXPOST_ACWI32_FORMAT]);
	} else { // HWA_64BIT_ADDRESSING 64bit host
		rxpost->config = (const hwa_rxpost_config_t*)
			((dev->wi_aggr_cnt == 0) ?
			 &hwa_rxpost_config[HWA_RXPOST_CWI64_FORMAT] :
			 &hwa_rxpost_config[HWA_RXPOST_ACWI64_FORMAT]);
	}

	// Configure RxPost
	HWA_TRACE(("%s %s config parser<%p> format<%u> size<%u>\n",
		HWA1a, rxpost->config->wi_name, rxpost->config->wi_parser,
		rxpost->config->wi_format, rxpost->config->wi_size));

#ifdef HWA_RXPOST_ONLY_BUILD
	{
		void *memory;
		uint32 mem_sz;
		uint32 depth;
		hwa_regs_t *regs;
		pcie_ipc_rings_t *pcie_ipc_rings;
		pcie_ipc_ring_mem_t *pcie_ipc_ring_mem;

		regs = dev->regs
		pcie_ipc_rings = dev->pcie_ipc_rings;

		// Single Host2Dongle RxPost common ring serves multiple MAC cores.
		pcie_ipc_ring_mem =
			HWA_UINT2PTR(pcie_ipc_ring_mem_t, pcie_ipc_rings->ring_mem_daddr32)
			+ BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT;

		// Allocate HWA1a->SW RxPost Interface
		depth  = pcie_ipc_ring_mem->max_items;
		mem_sz = depth * rxpost->config->wi_size;
		if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
			HWA_ERROR(("%s rxpost_ring %s malloc size<%u> failure\n",
				HWA1a, rxpost->config->wi_name, mem_sz));
			HWA_ASSERT(memory != (void*)NULL);
			return HWA_FAILURE;
		}
		HWA_TRACE(("%s rxpost_ring +memory[%p:%u]\n", HWA1a, memory, mem_sz));
		hwa_ring_init(&rxpost->rxpost_ring, "RXP",
			HWA_RXPOST_ID, HWA_RING_H2S, HWA_RXPOST_WI_H2S_RINGNUM, depth, memory,
			&regs->rx_core[0].rxpdest_ring_wrindex,
			&regs->rx_core[0].rxpdest_ring_rdindex);
	}
#endif  /* HWA_RXPOST_ONLY_BUILD */

	// Initialize the RxPost memory AXI memory address
	rxpost->rxpost_addr = hwa_axi_addr(dev, HWA_AXI_RXPOST_MEMORY);

	return HWA_SUCCESS;

}

void // HWA1a: Free resources for HWA1a block
hwa_rxpost_free(hwa_rxpost_t *rxpost)
{
#ifdef HWA_RXPOST_ONLY_BUILD
	void *memory;
	uint32 core, mem_sz;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA1b);

	if (rxpost == (hwa_rxpost_t*)NULL)
		return; // nothing to release done

	// Audit pre-conditions
	dev = HWA_DEV(rxpost);

	// Release resources used by HWA1a->SW RxPost Interface
	if (rxpost->rxpost_ring.memory != (void*)NULL) {
		memory = rxpost->rxpost_ring.memory;
		mem_sz = rxpost->rxpost_ring.depth * rxpost->config->wi_size
		HWA_TRACE(("%s rxfree_ring -memory[%p,%u]\n", HWA1a, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		hwa_ring_fini(&rxpost->rxpost_ring);
		rxpost->rxpost_ring.memory = (void*)NULL;
	}
#endif  /* HWA_RXPOST_ONLY_BUILD */

}

int // HWA1a initialization, supports only 1 core
hwa_rxpost_init(hwa_rxpost_t *rxpost)
{
	uint32 u32;
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	pcie_ipc_rings_t *pcie_ipc_rings;
	pcie_ipc_ring_mem_t *pcie_ipc_ring_mem;

	HWA_FTRACE(HWA1a);

	// Audit pre-conditions
	dev = HWA_DEV(rxpost);
	HWA_ASSERT(dev->pcie_ipc_rings != (pcie_ipc_rings_t*)NULL);
	HWA_ASSERT((dev->pcie_ipc->hcap1 & PCIE_IPC_HCAP1_HWA_RXPOST_IDMA) != 0);

	HWA_ASSERT(HWA_RX_CORES == 1);

	// Setup locals
	regs = dev->regs;
	pcie_ipc_rings = dev->pcie_ipc_rings;

	// Reset RPH req pending flag
	rxpost->pending_rph_req = FALSE;

	// Single Host2Dongle RxPost common ring serves multiple MAC cores.
	pcie_ipc_ring_mem =
		HWA_UINT2PTR(pcie_ipc_ring_mem_t, pcie_ipc_rings->ring_mem_daddr32)
		+ BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT;

	// Configure Host2Dongle RxPost common ring in HWA1a
	HWA_ERROR(("%s H2D RxPost ring: id<%u> type<%u>"
		" item_type<%u> max_items<%u> len_item<%u>\n",
		HWA1a, pcie_ipc_ring_mem->id, pcie_ipc_ring_mem->type,
		pcie_ipc_ring_mem->item_type, pcie_ipc_ring_mem->max_items,
		pcie_ipc_ring_mem->item_size));

	HWA_ERROR(("%s item_size %u %s config parser<%p> format<%u> size<%u>\n", HWA1a,
		pcie_ipc_ring_mem->item_size, rxpost->config->wi_name, rxpost->config->wi_parser,
		rxpost->config->wi_format, rxpost->config->wi_size));

	HWA_ASSERT(pcie_ipc_ring_mem->item_size == rxpost->config->wi_size);

	// Configure H2D Rxpost ring base address in host memory
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_ring_addr_lo,
		HADDR64_LO(pcie_ipc_ring_mem->haddr64));
	u32 = HWA_HOSTADDR64_HI32(HADDR64_HI(pcie_ipc_ring_mem->haddr64));
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_ring_addr_hi, u32);

#if HWA_REVISION_EQ_128
	// WAR for 43684A0 HWA coherent issue.
	u32 = (0U
		// | BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_SEQNUMENABLE)
		// | BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_SEQNUM_OR_POLARITY)
		// | BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_SEQNUM_WIDTH)
		| BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_UPDATE_RDIDX)
		| BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_UPDATE_RDIDX_RD_AFTER_WR)
		| BCM_SBF(dev->host_coherency, HWA_RX_RXPSRC_RING_CFG_TEMPLATE_NOTPCIE)
		/* alway FALSE! */
		//| BCM_SBF(dev->host_coherency, HWA_RX_RXPSRC_RING_CFG_TEMPLATE_COHERENT)
		| BCM_SBF(0, HWA_RX_RXPSRC_RING_CFG_TEMPLATE_ADDREXT)
		| BCM_SBF(pcie_ipc_ring_mem->item_size/4, HWA_RX_RXPSRC_RING_CFG_ELSIZE)
		| BCM_SBF(pcie_ipc_ring_mem->max_items, HWA_RX_RXPSRC_RING_CFG_DEPTH)
		| 0U);
#else /* HWA_REVISION_GE_129 */
	u32 = (0U
		// | BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_SEQNUMENABLE)
		// | BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_SEQNUM_OR_POLARITY)
		// | BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_SEQNUM_WIDTH)
		| BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_UPDATE_RDIDX)
		| BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_UPDATE_RDIDX_RD_AFTER_WR)
		// | BCM_SBIT(HWA_RX_RXPSRC_RING_CFG_TEMPLATE_NOTPCIE) always FALSE!
		| BCM_SBF(dev->host_coherency, HWA_RX_RXPSRC_RING_CFG_TEMPLATE_COHERENT)
		| BCM_SBF(0, HWA_RX_RXPSRC_RING_CFG_TEMPLATE_ADDREXT)
		| BCM_SBF(pcie_ipc_ring_mem->item_size/4, HWA_RX_RXPSRC_RING_CFG_ELSIZE)
		| BCM_SBF(pcie_ipc_ring_mem->max_items, HWA_RX_RXPSRC_RING_CFG_DEPTH)
		| 0U);
#endif /* HWA_REVISION_GE_129 */

	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_ring_cfg, u32);

	u32 = (0U
		| BCM_SBF(HWA_RXPOST_INTRAGGR_COUNT,
		        HWA_RX_RXPSRC_INTRAGGR_SEQNUM_CFG_AGGR_COUNT)
		// | BCM_SBF(0, HWA_RX_RXPSRC_INTRAGGR_SEQNUM_CFG_SEQNUM_MODULO_HI)
		| BCM_SBF(HWA_RXPOST_INTRAGGR_TMOUT,
		        HWA_RX_RXPSRC_INTRAGGR_SEQNUM_CFG_AGGR_TIMER)
		| 0U);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_intraggr_seqnum_cfg, u32);

	// H2D RxPost Seqnum not supported in 43684
	u32 = (0U
		// | BCM_SBF(0, HWA_RX_RXPSRC_SEQNUM_CFG_START_SEQNUMPOL)
		// | BCM_SBF(0, HWA_RX_RXPSRC_SEQNUM_CFG_SEQNUM_POS)
		// | BCM_SBF(0, HWA_RX_RXPSRC_SEQNUM_CFG_SEQNUM_MODULO_LO
		| 0U);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_seqnum_cfg, u32);

	// Configure H2D RxPost Ring RD Index
#if !defined(SBTOPCIE_INDICES)
	u32 = pcie_ipc_rings->h2d_rd_daddr32 +
		BCMPCIE_RW_INDEX_OFFSET(HWA_PCIE_RW_INDEX_SZ,
		BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT_IDX);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_rdindexupd_addr_lo, u32);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_rdindexupd_addr_hi, 0);
#else
	u32 = HADDR64_LO(pcie_ipc_rings->h2d_rd_haddr64) +
		BCMPCIE_RW_INDEX_OFFSET(HWA_PCIE_RW_INDEX_SZ,
		BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT_IDX);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_rdindexupd_addr_lo, u32);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_rdindexupd_addr_hi,
		HWA_HOSTADDR64_HI32(HADDR64_HI(pcie_ipc_rings->h2d_rd_haddr64)));
#endif /* !SBTOPCIE_INDICES */

	HWA_ERROR(("%s rxpost_data_buf_len<%u>\n", HWA1a, pcie_ipc_rings->rxpost_data_buf_len));

	// Configure the H2D Compact RxPost fields
	u32 = (0U
		| BCM_SBF(pcie_ipc_rings->rxpost_data_buf_len,
		          HWA_RX_RXPSRC_RING_HWA2CFG_RXP_DATA_BUF_LEN)
		| BCM_SBF(dev->wi_aggr_cnt/2,
		          HWA_RX_RXPSRC_RING_HWA2CFG_PKTS_PER_AGGR)
		| BCM_SBF((dev->wi_aggr_cnt > 0),
		          HWA_RX_RXPSRC_RING_HWA2CFG_RXP_AGGR_MODE)
#if HWA_REVISION_GE_129
		| BCM_SBF((dev->driver_mode == HWA_NIC_MODE),
		          HWA_RX_RXPSRC_RING_HWA2CFG_NIC64_1B)
		| BCM_SBF((dev->driver_mode == HWA_NIC_MODE),
		          HWA_RC_RXPSRC_RING_HWA2CFG_NIC_1A_DISABLE)
#endif // endif
		| 0U);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpsrc_ring_hwa2cfg, u32);

	/* rxpdestfifo_max_items FW programmed max items that will fit in the
	 * hwa internal rxpdest fifo. This should be programmed by FW based on
	 * the rxp memory size available (see capability) and the size of rxp
	 * workitem entry.
	 */
	u32 = ((HWA_RXPOST_LOCAL_MEM_DEPTH * HWA_RXPOST_MEM_ADDR_W) /
		pcie_ipc_ring_mem->item_size);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxp_localfifo_cfg_status, u32);

#ifdef HWA_RXPOST_ONLY_BUILD
	// Intitialize HWA1a->SW RxPost Interface
	u32 = HWA_PTR2UINT(rxpost->rxpost_ring.memory);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpdest_ring_addr_lo, u32);
	u32 = 0U;
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpdest_ring_addr_hi, u32);

	u32 = (0U
		| BCM_SBIT(HWA_RX_RXPDEST_RING_CFG_TEMPLATE_NOTPCIE)
		| BCM_SBIT(HWA_RX_RXPDEST_RING_CFG_TEMPLATE_COHERENT)
		| BCM_SBF(0, HWA_RX_RXPDEST_RING_CFG_TEMPLATE_ADDREXT)
		| BCM_SBF(rxpost->config->wi_size, HWA_RX_RXPDEST_RING_CFG_ELSIZE)
		| BCM_SBF(rxpost->rxpost_ring.depth, HWA_RX_RXPDEST_RING_CFG_DEPTH)
		| 0U);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpdest_ring_cfg, u32);
	u32 = (0U
		| BCM_SBF(HWA_RXPOST_INTRAGGR_COUNT,
		        HWA_RX_RXPDEST_INTRAGGR_SEQNUM_CFG_AGGR_COUNT)
		| BCM_SBF(HWA_RXPOST_INTRAGGR_TMOUT,
		        HWA_RX_RXPDEST_INTRAGGR_SEQNUM_CFG_AGGR_TIMER)
		| 0U);
	HWA_WR_REG_NAME(HWA1a, regs, rx_core[0], rxpdest_intraggr_seqnum_cfg, u32);

	// Assign the interested rxpost interrupt mask.
	dev->defintmask |= HWA_COMMON_INTSTATUS_RXPDEST0_INT_MASK;

#else  /* !HWA_RXPOST_ONLY_BUILD */

	// Software may reserve RPH when HWA1a and HWA1b are built.
	// Rx pktfetch needs to use RPH.

	// Single RxPost serving both cores, presently.
	HWA_ASSERT(HWA_RPH_RESERVE_COUNT > 0);
	u32 = HWA_RPH_RESERVE_COUNT;
	HWA_WR_REG_NAME(HWA1b, regs, rx_core[0], rph_reserve_cfg, u32);

	// Assign the interested rxpost interrupt mask.
	dev->defintmask |= (HWA_COMMON_INTSTATUS_RXPSRC0_INT_MASK |
		HWA_COMMON_INTSTATUS_RXPDEST0_INT_MASK |
		HWA_COMMON_INTSTATUS_D11BDEST0_INT_MASK |
		HWA_COMMON_INTSTATUS_FREEIDXSRC0_INT_MASK |
		HWA_COMMON_INTSTATUS_FWALERT0_INT_MASK);
#endif /* !HWA_RXPOST_ONLY_BUILD */

	return HWA_SUCCESS;

} // hwa_rxpost_init

int // HWA1a deinitialization, supports only 1 core
hwa_rxpost_deinit(hwa_rxpost_t *rxpost)
{
	return HWA_SUCCESS;
}

static void
hwa_rxpost_reclaim(hwa_rxpost_t *rxpost, uint32 core)
{
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;
	uint32 u32, dest_space_avail, max_items;
	int32 wi_cnt;
	uint8 rdptr, wrptr, idx;

	HWA_FTRACE(HWA1a);

	// Audit pre-conditions
	dev = HWA_DEV(rxpost);

	// Setup locals
	regs = dev->regs;

	// When reset hwa 1a and 1b, driver also need to reclaim the workitems in
	// HWA internal memory.
	// Then return those host_pktid to host memory.
	// The occupied space = rxpdestfifo_max_items - dest_space_avail_el.
	u32 = HWA_RD_REG_NAME(HWA1a, regs, rx_core[core], rxpmgr_tfrstatus);
	dest_space_avail = BCM_GBF(u32, HWA_RX_RXPMGR_TFRSTATUS_DEST_SPACE_AVAIL_EL);

	u32 = HWA_RD_REG_NAME(HWA1a, regs, rx_core[core], rxp_localfifo_cfg_status);
	max_items = BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_MAX_ITEMS);
	rdptr = BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_RDPTR);
	wrptr = BCM_GBF(u32, HWA_RX_RXP_LOCALFIFO_CFG_STATUS_RXPDESTFIFO_WRPTR);

	wi_cnt = max_items - dest_space_avail;

	HWA_ERROR(("%s wi_cnt<%u> in hwa internal memory<RD:%u WR:%u>\n",
		HWA1a, wi_cnt, rdptr, wrptr));

	if (wi_cnt == 0)
		return;

	do {
		switch (rxpost->config->wi_format) {
		case HWA_RXPOST_CWI32_FORMAT: {
			hwa_rxpost_cwi32_t cwi32;
			sys_mem = &cwi32.u32[0];
			axi_mem_addr = HWA_TABLE_ADDR(hwa_rxpost_cwi32_t,
				rxpost->rxpost_addr, rdptr);
			HWA_RD_MEM32(HWA1a, hwa_rxpost_cwi32_t, axi_mem_addr, sys_mem);
			hwa_rxpath_queue_rxcomplete_fast(dev, cwi32.host_pktid);
			break;
		}
		case HWA_RXPOST_CWI64_FORMAT: {
			hwa_rxpost_cwi64_t cwi64;
			sys_mem = &cwi64.u32[0];
			axi_mem_addr = HWA_TABLE_ADDR(hwa_rxpost_cwi64_t,
				rxpost->rxpost_addr, rdptr);
			HWA_RD_MEM32(HWA1a, hwa_rxpost_cwi64_t, axi_mem_addr, sys_mem);
			hwa_rxpath_queue_rxcomplete_fast(dev, cwi64.host_pktid);
			break;
		}
		case HWA_RXPOST_ACWI32_FORMAT: {
			hwa_rxpost_acwi32_t acwi32;
			sys_mem = &acwi32.u32[0];
			axi_mem_addr = HWA_TABLE_ADDR(hwa_rxpost_acwi32_t,
				rxpost->rxpost_addr, rdptr);
			HWA_RD_MEM32(HWA1a, hwa_rxpost_acwi32_t, axi_mem_addr, sys_mem);
			for (idx = 0; idx < HWA_AGGR_MAX; idx++) {
				if (HWA_HOST_PKTID_ISVALID(acwi32.host_pktid[idx])) {
					hwa_rxpath_queue_rxcomplete_fast(dev,
						acwi32.host_pktid[idx]);
				}
			}
			break;
		}
		case HWA_RXPOST_ACWI64_FORMAT: {
			hwa_rxpost_acwi64_t acwi64;
			sys_mem = &acwi64.u32[0];
			axi_mem_addr = HWA_TABLE_ADDR(hwa_rxpost_acwi64_t,
				rxpost->rxpost_addr, rdptr);
			HWA_RD_MEM32(HWA1a, hwa_rxpost_acwi64_t, axi_mem_addr, sys_mem);
			for (idx = 0; idx < HWA_AGGR_MAX; idx++) {
				if (HWA_HOST_PKTID_ISVALID(acwi64.host_pktid[idx])) {
					hwa_rxpath_queue_rxcomplete_fast(dev,
						acwi64.host_pktid[idx]);
				}
			}
			break;
		}
		default:
			HWA_ASSERT(0);
			break;
		}

		rdptr = (rdptr + 1) % max_items;
		wi_cnt--;
	} while (rdptr != wrptr);

	if (wi_cnt) {
		HWA_ERROR(("%s wi_cnt<%u> remains!\n", HWA1a, wi_cnt));
	}

	hwa_rxpath_xmit_rxcomplete_fast(dev);

}

uint16
hwa_rxpost_data_buf_len(void)
{
	hwa_dev_t *dev;

	// Audit pre-conditions
	dev = HWA_DEVP(TRUE); // CAUTION: global access with audit

	return (dev->pcie_ipc_rings->rxpost_data_buf_len);
}

#ifdef HWA_RXPOST_ONLY_BUILD
// Next 4 functions are customized for RxPost CWI32, CWI64, ACWI32 and ACWI64

static uint32 // Parse RxPost CWI32 and forward to upstream RxPost handler
hwa_rxpost_cwi32_parser(hwa_rxpost_t *rxpost, hwa_ring_t *rxpost_ring,
	uint32 elem_ix, hwa_handler_t *rxpost_handler)
{
	hwa_dev_t *dev;
	const uint32 core = 0U;
	uint32 host_pktid, host_physaddrhi;
	hwa_rxpost_cwi32_t *rxpost_cwi32;

	// Audit pre-conditions
	dev = HWA_DEV(rxpost);

	// Setup locals
	host_physaddrhi = dev->host_physaddrhi; // HWA_PCI64ADDR_HI32 form: 0x80000000 ORed

	// Fetch location of next CWI32 in rxpost_ring to process
	rxpost_cwi32 = HWA_RING_ELEM(hwa_rxpost_cwi32_t, rxpost_ring, elem_ix);
	host_pktid = rxpost_cwi32->host_pktid;

	// Invalid Host PktID is not permitted in non aggregated mode
	if (HWA_HOST_PKTID_ISVALID(host_pktid)) {

		// Handoff to upstream handler
		(*rxpost_handler->callback)(rxpost_handler->context,
			(uintptr)core, (uintptr)host_pktid,
			rxpost_cwi32->data_buf_haddr32, host_physaddrhi);

	} else {
		HWA_ERROR(("%s cwi32_parse host_pktid<null>\n", HWA1a));
		HWA_ASSERT(0);
		return 0;
	}

	return 1;

} // hwa_rxpost_cwi32_parse

static uint32 // Parse RxPost CWI64 and forward to upstream RxPost handler
hwa_rxpost_cwi64_parser(hwa_rxpost_t *rxpost, hwa_ring_t *rxpost_ring,
	uint32 elem_ix, hwa_handler_t *rxpost_handler)
{
	haddr64_t *haddr64;
	const uint32 core = 0U;
	uint32 host_pktid;
	hwa_rxpost_cwi64_t *rxpost_cwi64;

	// Fetch location of next CWI64 in rxpost_ring to process
	rxpost_cwi64 = HWA_RING_ELEM(hwa_rxpost_cwi64_t, rxpost_ring, elem_ix);
	host_pktid = rxpost_cwi64->host_pktid;

	// Invalid Host PktID is not permitted in non aggregated mode
	if (HWA_HOST_PKTID_ISVALID(host_pktid)) {
		haddr64 = &rxpost_cwi64->data_buf_haddr64;

		// Handoff to upstream handler
		(*rxpost_handler->callback)(rxpost_handler->context,
			(uintptr)core, (uintptr)host_pktid,
			HADDR64_LO(*haddr64), HWA_HOSTADDR64_HI32(HADDR64_HI(*haddr64)));

	} else {
		HWA_ERROR(("%s cwi64_parse host_pktid<null>\n", HWA1a));
		HWA_ASSERT(0);
		return 0;
	}

	return 1;

} // hwa_rxpost_cwi64_parse

static uint32 // Parse RxPost ACWI32 and forward to upstream RxPost handler
hwa_rxpost_acwi32_parser(hwa_rxpost_t *rxpost, hwa_ring_t *rxpost_ring,
	uint32 elem_ix, hwa_handler_t *rxpost_handler)
{
	hwa_dev_t *dev;
	const uint32 core = 0U;
	uint32 host_pktid, host_physaddrhi, parse_cnt;
	hwa_rxpost_acwi32_t *rxpost_acwi32;

	// Audit pre-conditions
	dev = HWA_DEV(rxpost);

	// Setup locals
	host_physaddrhi = dev->host_physaddrhi; // HWA_PCI64ADDR_HI32 form: 0x80000000 ORed

	// Fetch location of next ACWI32 in rxpost_ring to process
	rxpost_acwi32 = HWA_RING_ELEM(hwa_rxpost_acwi32_t, rxpost_ring, elem_ix);

	for (parse_cnt = 0; parse_cnt < HWA_AGGR_MAX; parse_cnt++) {
		host_pktid = rxpost_acwi32->host_pktid[parse_cnt];

		// Stop processing on the first invalid host_pktid in an aggregate
		if (HWA_HOST_PKTID_ISVALID(host_pktid)) {

			// Handoff to upstream handler
			(*rxpost_handler->callback)(rxpost_handler->context,
				(uintptr)core, (uintptr)host_pktid,
				rxpost_acwi32->data_buf_haddr32[parse_cnt], host_physaddrhi);

		} else {
			break;
		}
	}

	return parse_cnt;

} // hwa_rxpost_acwi32_parse

static uint32 // Parse RxPost ACWI64 and forward to upstream RxPost handler
hwa_rxpost_acwi64_parser(hwa_rxpost_t *rxpost, hwa_ring_t *rxpost_ring,
	uint32 elem_ix, hwa_handler_t *rxpost_handler)
{
	haddr64_t *haddr64;
	const uint32 core = 0U;
	uint32 host_pktid, parse_cnt;
	hwa_rxpost_acwi64_t *rxpost_acwi64;

	// Fetch location of next ACWI64 in rxpost_ring to process
	rxpost_acwi64 = HWA_RING_ELEM(hwa_rxpost_acwi64_t, rxpost_ring, elem_ix);

	for (parse_cnt = 0; parse_cnt < HWA_AGGR_MAX; parse_cnt++) {
		host_pktid = rxpost_acwi64->host_pktid[parse_cnt];

		// Stop processing on the first invalid host_pktid in an aggregate
		if (HWA_HOST_PKTID_ISVALID(host_pktid)) {
			haddr64 = &rxpost_acwi64->data_buf_haddr64[parse_cnt];

			// Handoff to upstream handler
			(*rxpost_handler->callback)(rxpost_handler->context,
				(uintptr)core, (uintptr)host_pktid,
				HADDR64_LO(*haddr64), HWA_HOSTADDR64_HI32(HADDR64_HI(*haddr64)));

		} else {
			break;
		}
	}

	return parse_cnt;

} // hwa_rxpost_acwi64_parse

int // Process RxPost workitems forwarded by HWA1a in RXPOST ONLY builds
hwa_rxpost_process(hwa_dev_t *dev)
{
	uint32 elem_ix; // location of next element to read
	uint32 rxpost_cnt; // total rxpost work items processed count
	hwa_rxpost_t *rxpost; // SW rxpost state
	hwa_ring_t *h2s_ring; // H2S RXPOST workitem rxpost_ring
	hwa_handler_t *rxpost_handler; // upstream rx post workitem processing
	hwa_rxpost_wi_parser_fn_t rxpost_wi_parser; // RxPost WI parser

	HWA_FTRACE(HWA1a);

	// Audit parameters and pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	rxpost = &dev->rxpost;

	// Fetch registered upstream callback handlers
	rxpost_handler = &dev->handlers[HWA_RXPOST_RECV];
	rxpost_wi_parser = rxpost->config->wi_parser; // custom WI handler - 4 forms

	h2s_ring = &rxpost->rxpost_ring;

	hwa_ring_cons_get(h2s_ring); // fetch HWA rxpost ring's WR index once
	rxpost_cnt = 0U;

	// Consume all RxPost WI received in RxPort ring, handing them upstream
	while ((elem_ix = hwa_ring_cons_upd(h2s_ring)) != BCM_RING_EMPTY) {

		// Parse next rxpost WI and forward to upstream rxpost handler
		rxpost_cnt += (rxpost_wi_parser)(h2s_ring, elem_ix, rxpost_handler);

		hwa_ring_cons_put(h2s_ring); // commit RD index now
	}

	HWA_STATS_EXPR(rxpost->rxpost_cnt += rxpost_cnt);

	return HWA_SUCCESS;

} // hwa_rxpost_process

#else /* !HWA_RXPOST_ONLY_BUILD */

static void hwa_rph_reclaim(hwa_rxpost_t *rxpost, uint32 core);

hwa_rxpost_hostinfo_t * // Allocate an RPH from HWA, returned value overwritten
hwa_rph_allocate(uint32 *bufid, uint16 *len, dma64addr_t *haddr64, bool pre_req)
{
	uint32 u32, loopcnt;
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	hwa_rxpost_t *rxpost;
	hwa_rxpost_hostinfo_t *rph_req;

	HWA_FTRACE(HWA1x);

	// Audit pre-conditions
	dev = HWA_DEVP(TRUE); // CAUTION: global access with audit

	// Setup locals
	regs = dev->regs;
	rxpost = &dev->rxpost;
	rph_req = &dev->rph_req;

	// If a pre request has allocated.
	if (rxpost->pre_rph_allocated)
		goto allocated;

	// If a previous request eventually completed.
	if (rxpost->pending_rph_req)
		goto pending_rph_req;

	// Setup location where HWA will DMA the RPH
	u32 = HWA_PTR2UINT(rph_req);
	HWA_WR_REG_NAME(HWA1x, regs, rx_core[0], rph_sw_buffer_addr, u32);

	// Synchronous request for an RPH
	u32 = (0U
		| BCM_SBIT(HWA_RX_RPH_RESERVE_REQ_REQ)
		// | BCM_SBIT(HWA_RX_RPH_RESERVE_REQ_INT_ON_NEW_RPH)
		| BCM_SBF(1, HWA_RX_RPH_RESERVE_REQ_RPH_TRANSFER_COUNT)
		| 0U);
	HWA_WR_REG_NAME(HWA1x, regs, rx_core[0], rph_reserve_req, u32);

pending_rph_req:

	// Synchronously wait for a request to complete
	for (loopcnt = 0; loopcnt < HWA_LOOPCNT; loopcnt++) {
		uint32 rph_done, rph_count;
		u32 = HWA_RD_REG_NAME(HWA1x, regs, rx_core[0], rph_reserve_resp);
		rph_done = BCM_GBIT(u32, HWA_RX_RPH_RESERVE_RESP_RPH_DONE);
		if (rph_done) {
			rph_count = BCM_GBIT(u32, HWA_RX_RPH_RESERVE_RESP_RPH_TRANSFERRED_COUNT);
			if (rph_count == 0) {
				// Request done but count is zero.

				HWA_STATS_EXPR(rxpost->rph_fails_cnt++);
				HWA_WARN(("%s Got rph count 0, pending_rph_req<%d>\n", HWA1x,
					rxpost->pending_rph_req));
				rxpost->pending_rph_req = FALSE;
				return (hwa_rxpost_hostinfo_t*)NULL;
			}

			HWA_ASSERT(BCM_GBIT(u32,
				HWA_RX_RPH_RESERVE_RESP_RPH_TRANSFERRED_COUNT) == 1);
			goto allocated;
		}
	}

	// In the case there were no RPH available for loop cnt, avoid a leak
	rxpost->pending_rph_req = TRUE;

	HWA_STATS_EXPR(rxpost->rph_fails_cnt++);
	HWA_WARN(("%s rph alloc failure%s\n", HWA1x, (pre_req) ? " [Pre Req]" : ""));

	return (hwa_rxpost_hostinfo_t*)NULL;

allocated:
	// Clear pending rph req flag
	rxpost->pending_rph_req = FALSE;

	if (pre_req) {
		rxpost->pre_rph_allocated = TRUE;
		// Don't copy out RPH, just retuen non NULL value.
		return rph_req;
	}
	// Clear pre allocated flag
	rxpost->pre_rph_allocated = FALSE;

	// Copy out the RPH
	*len = dev->pcie_ipc_rings->rxpost_data_buf_len;
	if (dev->host_addressing & HWA_32BIT_ADDRESSING) { // 32bit host
		*bufid = rph_req->hostinfo32.host_pktid;
		HADDR64_LO_SET(*haddr64, rph_req->hostinfo32.data_buf_haddr32);
		HADDR64_HI_SET(*haddr64, dev->host_physaddrhi);

		HWA_TRACE(("%s rph alloc pktid<%u> haddr<0x%08x:0x%08x>\n", HWA1x,
			rph_req->hostinfo32.host_pktid, dev->host_physaddrhi,
			rph_req->hostinfo32.data_buf_haddr32));
	} else { // HWA_64BIT_ADDRESSING 64bit host
		*bufid = rph_req->hostinfo32.host_pktid;
		HADDR64_SET(*haddr64, rph_req->hostinfo64.data_buf_haddr64);

		rph_req->hostinfo64.data_buf_haddr64.hiaddr =
			HWA_HOSTADDR64_HI32(rph_req->hostinfo64.data_buf_haddr64.hiaddr);

		HWA_TRACE(("%s rph alloc pktid<%u> haddr<0x%08x:0x%08x>\n", HWA1x,
			rph_req->hostinfo64.host_pktid,
			rph_req->hostinfo64.data_buf_haddr64.hiaddr,
			rph_req->hostinfo64.data_buf_haddr64.loaddr));
	}

	HWA_STATS_EXPR(rxpost->rph_alloc_cnt++);

	return rph_req;

} // hwa_rph_allocate

// Reclaim SW pre-allocated reserved RPH
static void
hwa_rph_reclaim(hwa_rxpost_t *rxpost, uint32 core)
{
	hwa_dev_t *dev;

	HWA_FTRACE(HWA1x);

	if (!rxpost->pre_rph_allocated)
		return;

	// Audit pre-conditions
	dev = HWA_DEVP(TRUE); // CAUTION: global access with audit

	// Clear pre-allocated RPH flag
	rxpost->pre_rph_allocated = FALSE;

	// Free it.
	hwa_rxpath_queue_rxcomplete_fast(dev,
		dev->rph_req.hostinfo32.host_pktid);
	hwa_rxpath_xmit_rxcomplete_fast(dev);

	HWA_WARN(("%s Reclaim 1 RPH\n", HWA1a));
}

#endif /* !HWA_RXPOST_ONLY_BUILD */

#if defined(BCMDBG)

void // Debug support for HWA1a RxPost block
hwa_rxpost_dump(hwa_rxpost_t *rxpost, struct bcmstrbuf *b, bool verbose)
{
	HWA_BPRINT(b, "%s dump<%p>\n", HWA1a, rxpost);

	if (rxpost == (hwa_rxpost_t*)NULL)
		return;

	HWA_RXPOST_ONLY_EXPR(hwa_ring_dump(&rxpost->rxpost_ring, b, "+ rxpost_ring"));
	HWA_RXPOST_ONLY_EXPR(HWA_STATS_EXPR(
		HWA_BPRINT(b, "+ rxpost_cnt<%u>\n", rxpost->rxpost_cnt)));

	HWA_BPRINT(b, "+ Config: %s parser<%p> format<%u> size<%u> "
		" offset len<%u> addr<%u>\n",
		rxpost->config->wi_name, rxpost->config->wi_parser,
		rxpost->config->wi_format, rxpost->config->wi_size,
		rxpost->config->len_offset, rxpost->config->addr_offset);

	HWA_BPRINT(b, "+ pending_rph_req<%u>\n", rxpost->pending_rph_req);
	HWA_STATS_EXPR(HWA_BPRINT(b, "+ rph_alloc_cnt<%u> rph_fails_cnt<%u>\n",
			rxpost->rph_alloc_cnt, rxpost->rph_fails_cnt));

} // hwa_rxpost_dump

#endif /* BCMDBG */

#endif /* HWA_RXPOST_BUILD */

#ifdef HWA_RXPATH_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: HWA1a RxPost + HWA1b RxFill + HWA2a RxPath Common functionality
 * -----------------------------------------------------------------------------
 */

void // HWA1x: Release resources used by RxPath HWA1x blocks 1a + 1b + 2a
BCMATTACHFN(hwa_rxpath_detach)(hwa_rxpath_t *rxpath) // rxpath may be NULL
{
	hwa_dev_t *dev;

	HWA_FTRACE(HWA1x);

	dev = HWA_DEV(rxpath);
	BCM_REFERENCE(dev);

	HWA_RXPOST_EXPR(hwa_rxpost_free(&dev->rxpost));
	HWA_RXFILL_EXPR(hwa_rxfill_free(&dev->rxfill));
	HWA_RXFILL_EXPR(hwa_rxfill_detach(&dev->rxfill));
} // hwa_rxpath_detach

hwa_rxpath_t * // HWA1x: Allocate resources for RxPath HWA1x blocks 1a + 1b + 2a
BCMATTACHFN(hwa_rxpath_attach)(hwa_dev_t *dev)
{
	hwa_rxpath_t *rxpath; // HWA1x state

	HWA_FTRACE(HWA1x);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	HWA_RXPOST_EXPR({
	// Verify HWA 1x block's structures
	HWA_ASSERT(sizeof(hwa_rxpost_cwi32_t) == HWA_RXPOST_CWI32_BYTES);
	HWA_ASSERT(sizeof(hwa_rxpost_cwi64_t) == HWA_RXPOST_CWI64_BYTES);
	HWA_ASSERT(sizeof(hwa_rxpost_acwi32_t) == HWA_RXPOST_ACWI32_BYTES);
	HWA_ASSERT(sizeof(hwa_rxpost_acwi64_t) == HWA_RXPOST_ACWI64_BYTES);
	HWA_ASSERT(sizeof(hwa_rxpost_hostinfo32_t) == HWA_RXPOST_HOSTINFO32_BYTES);
	HWA_ASSERT(sizeof(hwa_rxpost_hostinfo64_t) == HWA_RXPOST_HOSTINFO64_BYTES);
	})

	// Confirm HWA RxPath Capabilities against 43684 Generic
	{
		uint32 cap1, rxnumbuf, rxpfifosz;
		cap1 = HWA_RD_REG_NAME(HWA1a, dev->regs, top, hwahwcap1);
		rxnumbuf = BCM_GBF(cap1, HWA_TOP_HWAHWCAP1_RXNUMBUF);
		rxpfifosz = BCM_GBF(cap1, HWA_TOP_HWAHWCAP1_RXPFIFOSZ);
		BCM_REFERENCE(rxnumbuf);
		BCM_REFERENCE(rxpfifosz);

		HWA_ASSERT((rxnumbuf * 256) >= HWA_RXPATH_PKTS_MAX);
		HWA_ASSERT((rxpfifosz * 128) ==
			(HWA_RXPOST_LOCAL_MEM_DEPTH * HWA_RXPOST_MEM_ADDR_W));
	}

	// Rxfill attach.
	HWA_RXFILL_EXPR(hwa_rxfill_attach(dev));

	// Complete attaching HWA1x objects to master object
	rxpath = &dev->rxpath;

	return rxpath; // HWA1x object

} // hwa_rxpath_attach

/* Handle MAC DMA reset */
int
hwa_rxpath_dma_reset(struct hwa_dev *dev, uint32 core)
{
	uint32 u32, idle, loop_count;

	HWA_TRACE(("%s %s: Disable Rx block.\n", HWA1b, __FUNCTION__));

	// Disable Rx block
	u32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, module_enable);
	u32 &= ~(HWA_COMMON_MODULE_ENABLE_BLOCKRXBM_ENABLE_MASK |
		HWA_COMMON_MODULE_ENABLE_BLOCKRXCORE0_ENABLE_MASK |
		HWA_COMMON_MODULE_ENABLE_BLOCKRXCORE1_ENABLE_MASK);
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_enable, u32);

	loop_count = HWA_MODULE_IDLE_BURNLOOP;
	do { // Burnloop: allowing HWA1a to complete a previous RdIdx DMA
		u32 = HWA_RD_REG_NAME(HWA00, dev->regs, common, module_idle);
		idle = BCM_GBIT(u32, HWA_COMMON_MODULE_IDLE_BLOCKRXCORE0_IDLE);
	} while (!idle && loop_count--);

	if (!loop_count) {
		HWA_ERROR(("%s %s: Rx block idle<%u> loop<%u>\n", HWA1b, __FUNCTION__,
			idle, HWA_MODULE_IDLE_BURNLOOP));
	}

	return BCME_OK;
}

void // Reclaim MAC Rx DMA posted buffer
hwa_rxpath_dma_reclaim(struct hwa_dev *dev)
{
	uint32 u32, core;

	// Reclaim posted packets
	for (core = 0; core < HWA_RX_CORES; core++) {
		// Send RxPost stop refill event to host
		HWA_RXPOST_EXPR(hwa_wlc_mac_event(dev, WLC_E_HWA_RX_STOP_REFILL));

		// Flush all SW Rx packets.
		HWA_RXFILL_EXPR(hwa_wl_reclaim_rx_packets(dev));

		// Reclaim SW pre-allocated reserved RPH
		HWA_RXPOST_EXPR(hwa_rph_reclaim(&dev->rxpost, core));

		// Reclaim all RxBuffers in RxBM
		HWA_RXFILL_EXPR(hwa_rxfill_rxbuffer_reclaim(dev, core));

		// Reclaim RxPost WI in HWA internal memory
		HWA_RXPOST_EXPR(hwa_rxpost_reclaim(&dev->rxpost, core));
	}

	// Reset Rx block
	u32 = (0U
		HWA_RXPOST_EXPR(
			| HWA_COMMON_MODULE_RESET_BLOCKRXCORE0_RESET_MASK
			| HWA_COMMON_MODULE_RESET_BLOCKRXCORE1_RESET_MASK)
		HWA_RXFILL_EXPR(
			| HWA_COMMON_MODULE_RESET_BLOCKRXBM_RESET_MASK));
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_reset, u32);
	HWA_WR_REG_NAME(HWA00, dev->regs, common, module_reset, 0);
}

int // Add rxcple workitems to pciedev directly.
hwa_rxpath_queue_rxcomplete_fast(hwa_dev_t *dev, uint32 pktid)
{
	BCMPCIE_IPC_HPA_TEST(dev->pciedev, pktid,
		BCMPCIE_IPC_PATH_RECEIVE, BCMPCIE_IPC_TRANS_REQUEST);
	return pciedev_hwa_queue_rxcomplete_fast(dev->pciedev, pktid);
}

void // Xmit rxcple workitems from pciedev to host
hwa_rxpath_xmit_rxcomplete_fast(hwa_dev_t *dev)
{
	PCIEDEV_XMIT_RXCOMPLETE(dev->pciedev);
}

#if defined(BCMPCIE_IPC_HPA)
void /* A HWA wrap function to Test a PktId on entry and exit from dongle */
hwa_rxpath_hpa_req_test(hwa_dev_t *dev, uint32 pktid)
{
	BCMPCIE_IPC_HPA_TEST(dev->pciedev, pktid,
		BCMPCIE_IPC_PATH_RECEIVE, BCMPCIE_IPC_TRANS_REQUEST);
}
#endif // endif

void // Flush all rxcpl workitmes from pciedev to host
hwa_rxpath_flush_rxcomplete(hwa_dev_t *dev)
{
	pciedev_hwa_flush_rxcomplete(dev->pciedev);
}

// HWA1x blocks 1a + 1b + 2a statistics collection
static void _hwa_rxpath_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 core);

void // Clear statistics for HWA1x blocks 1a + 1b
hwa_rxpath_stats_clear(hwa_rxpath_t *rxpath, uint32 core)
{
	hwa_dev_t *dev;

	dev = HWA_DEV(rxpath);

	hwa_stats_clear(dev, HWA_STATS_RXPOST_CORE0 + core); // common

} // hwa_rxpath_stats_clear

void // Print the common statistics for HWA1x blocks 1a + 1b
_hwa_rxpath_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 core)
{
	hwa_rxpath_stats_t *rxpath_stats = &dev->rxpath.stats[core];
	struct bcmstrbuf *b = (struct bcmstrbuf *)buf;

	HWA_BPRINT(b, "%s statistics core[%u] [wi<%u> dma<%u>]\n"
		"+ fifo[desc<%u> empty<%u>] stalls[d11b<%u> bm<%u> dma<%u>]\n"
		"+ RD upd<%u> bm[alloc<%u> frees<%u>] duration[bm<%u> dma<%u>]\n",
		HWA1x, core,
		rxpath_stats->num_rxpost_wi, rxpath_stats->num_rxpost_dma,
		rxpath_stats->num_fifo_descr, rxpath_stats->num_fifo_empty,
		rxpath_stats->num_stalls_d11b, rxpath_stats->num_stalls_bm,
		rxpath_stats->num_stalls_dma, rxpath_stats->num_d2h_rd_upd,
		rxpath_stats->num_d11b_allocs, rxpath_stats->num_d11b_frees,
		rxpath_stats->dur_bm_empty, rxpath_stats->dur_dma_busy);

} // _hwa_rxpath_stats_dump

void // Query and dump common statistics for HWA1x blocks 1a + 1b
hwa_rxpath_stats_dump(hwa_rxpath_t *rxpath, struct bcmstrbuf *b, uint8 clear_on_copy)
{
	uint32 core;
	hwa_dev_t *dev;

	dev = HWA_DEV(rxpath);

	for (core = 0; core < HWA_RX_CORES; core++) {
		hwa_rxpath_stats_t *rxpath_stats = &rxpath->stats[core];
		hwa_stats_copy(dev, HWA_STATS_RXPOST_CORE0 + core,
			HWA_PTR2UINT(rxpath_stats), HWA_PTR2HIADDR(rxpath_stats),
			/* num_sets */ 1, clear_on_copy, &_hwa_rxpath_stats_dump,
			(uintptr)b, core);
	}

} // hwa_rxpath_stats_dump

int // Determine if any Rx path block HWA1a or HWA1b has an error
hwa_rxpath_error(hwa_dev_t *dev, uint32 core)
{
	uint32 u32;
	hwa_regs_t *regs;

	HWA_ASSERT(core < HWA_RX_CORES);

	HWA_AUDIT_DEV(dev);
	regs = dev->regs;

	u32 = HWA_RD_REG_NAME(HWA1x, regs, rx_core[core], debug_errorstatus);
	if (u32) {
		HWA_TRACE(("%s error<0x%08x>\n", HWA1x, u32));
		return HWA_FAILURE;
	}
	u32 = HWA_RD_REG_NAME(HWA1x, regs, rx_core[core], debug_hwa2status);
	if (u32) {
		HWA_TRACE(("%s error pktid<%u> pktaddr<%u>\n", HWA1x,
			BCM_GBIT(u32, HWA_RX_DEBUG_HWA2STATUS_PKTID_ERR),
			BCM_GBIT(u32, HWA_RX_DEBUG_HWA2STATUS_PKTADDR_ERR)));
		return HWA_FAILURE;
	}
	u32 = HWA_RD_REG_NAME(HWA1x, regs, rx_core[core], debug_hwa2errorstatus);
	if (u32) {
		HWA_TRACE(("%s hwa2 error<0x%08x>\n", HWA1x, u32));
		return HWA_FAILURE;
	}

	return HWA_SUCCESS;

} // hwa_rxpath_error

#if defined(BCMDBG)

void // Dump software state for HWA1x blocks 1a + 1b
hwa_rxpath_dump(hwa_rxpath_t *rxpath, struct bcmstrbuf *b, bool verbose, bool dump_regs)
{
	if (rxpath == (hwa_rxpath_t*)NULL)
		return;

	HWA_BPRINT(b, "%s dump<%p>\n", HWA1x, rxpath);

	if (verbose)
		hwa_rxpath_stats_dump(rxpath, b, /* clear */ 0);

#if defined(WLTEST)
	if (dump_regs)
		hwa_rxpath_regs_dump(rxpath, b);
#endif // endif
} // hwa_rxpath_dump

#if defined(WLTEST)
void // Dump HWA registers for HWA1x blocks 1a + 1b
hwa_rxpath_regs_dump(hwa_rxpath_t *rxpath, struct bcmstrbuf *b)
{
	uint32 core;
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	dev = HWA_DEV(rxpath);
	regs = dev->regs;

#define HWA_BPR_RING_REG(b, CORE, NAME) \
	({ \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_ring_addr_lo); \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_ring_addr_hi); \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_ring_wrindex); \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_ring_rdindex); \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_ring_cfg); \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_intraggr_seqnum_cfg); \
	})

#define HWA_BPR_FIFO_REG(b, CORE, NAME) \
	({ \
		HWA_BPR_RING_REG(b, CORE, NAME); \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_wrindexupd_addrlo); \
		HWA_BPR_REG(b, rx_core[CORE], NAME##_status); \
	})

	for (core = 0; core < HWA_RX_CORES; core++) {
		HWA_BPRINT(b, "%s registers[%p] offset[0x%04x]\n",
			HWA1x, &regs->rx_core[core], OFFSETOF(hwa_regs_t, rx_core[core]));

#ifdef HWA_RXPOST_BUILD
		HWA_BPR_RING_REG(b, core, rxpsrc);
		HWA_BPR_REG(b, rx_core[core], rxpsrc_seqnum_cfg);
		HWA_BPR_REG(b, rx_core[core], rxpsrc_seqnum_status);
		HWA_BPR_REG(b, rx_core[core], rxpsrc_rdindexupd_addr_lo);
		HWA_BPR_REG(b, rx_core[core], rxpsrc_rdindexupd_addr_hi);
		HWA_BPR_REG(b, rx_core[core], rxpsrc_status);
		HWA_BPR_REG(b, rx_core[core], rxpsrc_ring_hwa2cfg);
#ifndef HWA_RXFILL_BUILD
		HWA_BPR_RING_REG(b, core, rxpdest);
		HWA_BPR_REG(b, rx_core[core], rxpdest_status);
#endif /* !HWA_RXFILL_BUILD */
#endif /* HWA_RXPOST_BUILD */
#ifdef HWA_RXFILL_BUILD
		HWA_BPR_FIFO_REG(b, core, d0dest);
		HWA_BPR_FIFO_REG(b, core, d1dest);
		HWA_BPR_RING_REG(b, core, d11bdest);
		HWA_BPR_RING_REG(b, core, freeidxsrc);
#endif /* HWA_RXFILL_BUILD */
#ifdef HWA_RXPOST_BUILD
		HWA_BPR_REG(b, rx_core[core], rxpmgr_cfg);
		HWA_BPR_REG(b, rx_core[core], rxpmgr_tfrstatus);
#endif /* HWA_RXPOST_BUILD */
#ifdef HWA_RXFILL_BUILD
		HWA_BPR_REG(b, rx_core[core], d0mgr_tfrstatus);
		HWA_BPR_REG(b, rx_core[core], d1mgr_tfrstatus);
		HWA_BPR_REG(b, rx_core[core], d11bmgr_tfrstatus);
		HWA_BPR_REG(b, rx_core[core], freeidxmgr_tfrstatus);
#endif /* HWA_RXFILL_BUILD */
#ifdef HWA_RXPOST_BUILD
		HWA_BPR_REG(b, rx_core[core], rxp_localfifo_cfg_status);
#endif /* HWA_RXPOST_BUILD */
#ifdef HWA_RXFILL_BUILD
		HWA_BPR_REG(b, rx_core[core], d0_localfifo_cfg_status);
		HWA_BPR_REG(b, rx_core[core], d1_localfifo_cfg_status);
		HWA_BPR_REG(b, rx_core[core], d11b_localfifo_cfg_status);
		HWA_BPR_REG(b, rx_core[core], freeidx_localfifo_cfg_status);
		HWA_BPR_REG(b, rx_core[core], mac_counter_ctrl);
		HWA_BPR_REG(b, rx_core[core], mac_counter_status);
		HWA_BPR_REG(b, rx_core[core], fw_alert_cfg);
		HWA_BPR_REG(b, rx_core[core], fw_rxcompensate);
		HWA_BPR_REG(b, rx_core[core], rxfill_ctrl0);
		HWA_BPR_REG(b, rx_core[core], rxfill_ctrl1);
		HWA_BPR_REG(b, rx_core[core], rxfill_compresslo);
		HWA_BPR_REG(b, rx_core[core], rxfill_compresshi);
		HWA_BPR_REG(b, rx_core[core], rxfill_desc0_templ_lo);
		HWA_BPR_REG(b, rx_core[core], rxfill_desc0_templ_hi);
		HWA_BPR_REG(b, rx_core[core], rxfill_desc1_templ_lo);
		HWA_BPR_REG(b, rx_core[core], rxfill_desc1_templ_hi);
		HWA_BPR_REG(b, rx_core[core], rxfill_status0);
		HWA_BPR_REG(b, rx_core[core], rph_reserve_cfg);
		HWA_BPR_REG(b, rx_core[core], rph_sw_buffer_addr);
		HWA_BPR_REG(b, rx_core[core], rph_reserve_req);
		HWA_BPR_REG(b, rx_core[core], rph_reserve_resp);
#endif /* HWA_RXFILL_BUILD */
		HWA_BPR_REG(b, rx_core[core], debug_intstatus);
		HWA_BPR_REG(b, rx_core[core], debug_errorstatus);
		HWA_BPR_REG(b, rx_core[core], debug_hwa2status);
		HWA_BPR_REG(b, rx_core[core], debug_hwa2errorstatus);
	} // for core

} // hwa_rxpath_regs_dump

#endif // endif

#endif /* BCMDBG */

#endif /* HWA_RXPATH_BUILD */

#ifdef HWA_TXPOST_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: HWA3a TxPost Block
 * -----------------------------------------------------------------------------
 */
static int hwa_txpost_sendup(void *context, uintptr arg1, uintptr arg2, uint32 pkt_count,
	uint32 total_octets);

/*
 * -----------------------------------------------------------------------------
 * Section: HWA3a block bringup: attach, detach, and init phases
 * -----------------------------------------------------------------------------
 */

void // HWA3a: Cleanup/Free resources used by TxPost block
BCMATTACHFN(hwa_txpost_detach)(hwa_txpost_t *txpost)
{
	void *memory;
	uint32 mem_sz;
	hwa_dev_t *dev;

	HWA_FTRACE(HWA3a);

	if (txpost == (hwa_txpost_t*)NULL)
		return;

	// Audit pre-conditions
	dev = HWA_DEV(txpost);

	// HWA3a TxPost SchedCmd Ring: free memory and reset ring
	if (txpost->schedcmd_ring.memory != (void*)NULL) {
		memory = txpost->schedcmd_ring.memory;
		mem_sz = txpost->schedcmd_ring.depth * sizeof(hwa_txpost_schedcmd_t);
		HWA_TRACE(("%s schedcmd_ring -memory[%p:%u]\n", HWA3a, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		hwa_ring_fini(&txpost->schedcmd_ring);
		txpost->schedcmd_ring.memory = (void*)NULL;
	}

	// HWA3a TxPost PktChain Ring: free memory and reset ring
	if (txpost->pktchain_ring.memory != (void*)NULL) {
		memory = txpost->pktchain_ring.memory;
		mem_sz = txpost->pktchain_ring.depth * sizeof(hwa_txpost_pktchain_t);
		HWA_TRACE(("%s pktchain_ring -memory[%p:%u]\n", HWA3a, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		hwa_ring_fini(&txpost->pktchain_ring);
		txpost->pktchain_ring.memory = (void*)NULL;
	}

	// HWA3a TxPost FlowRingConfig FRC: free memory
	if (txpost->frc_table != (hwa_txpost_frc_t*)NULL) {
		memory = txpost->frc_table;
		mem_sz = HWA_TXPOST_FLOWRINGS_MAX * sizeof(hwa_txpost_frc_t);
		HWA_TRACE(("%s frc_table -memory[%p:%u]\n", HWA3a, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		txpost->frc_table = (hwa_txpost_frc_t*)NULL;
	}

#if HWA_REVISION_GE_129
	if (dev->tx_pktdealloc) {
		// HWA3a TxPost TxFree Ring: free memory
		if (txpost->txfree_ring.memory != (void*)NULL) {
			memory = txpost->txfree_ring.memory;
			mem_sz = txpost->txfree_ring.depth * sizeof(hwa_txpost_txfree_t);
			HWA_TRACE(("%s txfree_ring -memory[%p:%u]\n", HWA3a, memory, mem_sz));
			MFREE(dev->osh, memory, mem_sz);
			hwa_ring_fini(&txpost->txfree_ring);
			txpost->txfree_ring.memory = (void*)NULL;
		}
	}
#endif /* HWA_REVISION_GE_129 */

} // hwa_txpost_detach

hwa_txpost_t * // HWA3a: Allocate resources for HWA3a txpost block
BCMATTACHFN(hwa_txpost_attach)(hwa_dev_t *dev)
{
	void *memory;
	uint32 mem_sz, depth;
	hwa_regs_t *regs;
	hwa_tx_regs_t *tx_regs;
	hwa_txpost_t *txpost;

	HWA_FTRACE(HWA3a);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	regs = dev->regs;
	tx_regs = &regs->tx;
	txpost = &dev->txpost;

	// Verify HWA3a block's structures
	HWA_ASSERT(sizeof(hwa_txpost_frc_t) == HWA_TXPOST_FRC_BYTES);
	HWA_ASSERT(sizeof(hwa_txpost_schedcmd_t) == HWA_TXPOST_SCHEDCMD_BYTES);
	HWA_ASSERT(sizeof(hwa_txpost_pktchain_t) == HWA_TXPOST_PKTCHAIN_BYTES);
#if !defined(HWA_NO_LUT)
	HWA_ASSERT(sizeof(hwa_txpost_sada_lut_elem_t) == HWA_TXPOST_SADA_BYTES);
	HWA_ASSERT(sizeof(hwa_txpost_flow_lut_elem_t) == HWA_TXPOST_FLOW_LUT_BYTES);
#endif // endif

	// Confirm HWA TxPost Capabilities against 43684 Generic
	{
		uint32 cap, numbuf3a;

		cap = HWA_RD_REG_NAME(HWA3a, regs, top, hwahwcap1);
		HWA_ASSERT(BCM_GBF(cap, HWA_TOP_HWAHWCAP1_MAXSEGCNT3A3B) ==
			HWA_TX_DATABUF_SEGCNT_MAX);
#if HWA_REVISION_EQ_128
		HWA_ASSERT(BCM_GBF(cap, HWA_TOP_HWAHWCAP1_NUMTXPOSTLOCALBUF3A) ==
			HWA_TXPOST_LOCAL_WORKITEMS);
#endif /* HWA_REVISION_EQ_128 */
		numbuf3a = BCM_GBF(cap, HWA_TOP_HWAHWCAP1_NUMBUF3A);

#if HWA_REVISION_EQ_128
		// In HWA2.0, numbuf3a high bits in cap2 registers.
		cap = HWA_RD_REG_NAME(HWA3a, regs, top, hwahwcap2);
		numbuf3a |= BCM_GBF(cap, HWA_TOP_HWAHWCAP2_NUMBUF3A_HN) << 4U;
#endif /* HWA_REVISION_EQ_128 */

#ifndef BCM43684A0_COHERENT_WAR
		HWA_ASSERT((numbuf3a * 256) == HWA_TXPATH_PKTS_MAX_CAP);
#endif // endif
		BCM_REFERENCE(numbuf3a);
	}

	// Allocate and initialize S2H schedule command interface
	depth = HWA_TXPOST_SCHEDCMD_RING_DEPTH;
	mem_sz = depth * sizeof(hwa_txpost_schedcmd_t);
	if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
		HWA_ERROR(("%s schedcmd_ring malloc size<%u> failure\n",
			HWA3a, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		goto failure;
	}
	HWA_TRACE(("%s schedcmd_ring +memory[%p,%u]\n", HWA3a, memory, mem_sz));
	hwa_ring_init(&txpost->schedcmd_ring, "CMD",
		HWA_TXPOST_ID, HWA_RING_S2H, HWA_TXPOST_SCHEDCMD_S2H_RINGNUM,
		depth, memory, &tx_regs->fw_cmdq_wr_idx, &tx_regs->fw_cmdq_rd_idx);

	// Allocate and initialize H2S packet chain interface
	depth  = HWA_TXPOST_PKTCHAIN_RING_DEPTH;
	mem_sz = depth * sizeof(hwa_txpost_pktchain_t);
	if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
		HWA_ERROR(("%s pktchain_ring malloc size<%u> failure\n",
			HWA3a, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		goto failure;
	}
	HWA_TRACE(("%s pktchain_ring +memory[%p,%u]\n", HWA3a, memory, mem_sz));
	hwa_ring_init(&txpost->pktchain_ring, "CHN",
		HWA_TXPOST_ID, HWA_RING_S2H, HWA_TXPOST_PKTCHAIN_H2S_RINGNUM,
		depth, memory, &tx_regs->pkt_chq_wr_idx, &tx_regs->pkt_chq_rd_idx);

	// Allocate and initialize FRC table
	mem_sz = HWA_TXPOST_FLOWRINGS_MAX * sizeof(hwa_txpost_frc_t);
	if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
		HWA_ERROR(("%s frc table malloc size<%u> failure\n",
			HWA3a, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		goto failure;
	}
	HWA_TRACE(("%s frc_table +memory[%p,%u]\n", HWA3a, memory, mem_sz));
	txpost->frc_table = (hwa_txpost_frc_t*)memory;

#if HWA_REVISION_GE_129
	if (dev->tx_pktdealloc) {
		// Allocate and initialize S2H "FREEIDXTX" interface
		depth = HWA_TXPOST_TXFREE_DEPTH;
		mem_sz = depth * sizeof(hwa_txpost_txfree_t);
		if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
			HWA_ERROR(("%s txfree malloc size<%u> failure\n", HWA3a, mem_sz));
			HWA_ASSERT(memory != (void*)NULL);
			goto failure;
		}
		HWA_TRACE(("%s txfree_ring +memory[%p,%u]\n", HWA3a, memory, mem_sz));
		hwa_ring_init(&txpost->txfree_ring, "TXF", HWA_TXPOST_ID,
			HWA_RING_S2H, HWA_TXPOST_TXFREE_S2H_RINGNUM, depth, memory,
			&regs->tx.pktdealloc_ring_wrindex,
			&regs->tx.pktdealloc_ring_rdindex);
	}
#endif /* HWA_REVISION_GE_129 */

	/*
	 * Following registers are ONLY for debugging HWA
	 *  hwa_tx_pkt_ch_t tx_pkt_ch[HWA_TX_PKT_CH_MAX]
	 *  pkt_ch_valid
	 *  pkt_ch_flowid_reg[HWA_TX_PKT_CH_MAX / 2]
	 */

	// Octet Count based Schedule Commands NOT SUPPORTED txplavg_weights_reg

#if !defined(HWA_NO_LUT)
	// Initialize Per interface Priority LUT
	txpost->prio_addr = hwa_axi_addr(dev, HWA_AXI_TXPOST_PRIO_LUT);

	// Initialize Unique SADA LUT
	txpost->sada_addr = hwa_axi_addr(dev, HWA_AXI_TXPOST_SADA_LUT);

	// Inilialize Flow LUT
	// Auto find next free flowid feature not supported: reg txpost_config
	txpost->flow_addr = hwa_axi_addr(dev, HWA_AXI_TXPOST_FLOW_LUT);
#endif /* !HWA_NO_LUT */

	// Over-write the register handle function
	hwa_register(dev, HWA_PKTCHAIN_SENDUP, dev, hwa_txpost_sendup);

	// Reset schedule command histogram
	HWA_BCMDBG_EXPR(memset(txpost->schecmd_histogram, 0,
		sizeof(txpost->schecmd_histogram)));

	return txpost;

failure:
	hwa_txpost_detach(txpost);
	HWA_WARN(("%s attach failure\n", HWA3a));

	return ((hwa_txpost_t*)NULL);

} // hwa_txpost_attach

void // HWA3a: Init TxPost block AFTER DHD handshake pcie_ipc initialized.
hwa_txpost_init(hwa_txpost_t *txpost)
{
	uint32 u32;
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	pcie_ipc_rings_t *pcie_ipc_rings;

	HWA_FTRACE(HWA3a);

	// Audit pre-conditions
	dev = HWA_DEV(txpost);
	HWA_ASSERT(dev->pcie_ipc_rings != (pcie_ipc_rings_t*)NULL);

	// Check initialization.
	if (dev->inited)
		return;

	// Setup locals
	regs = dev->regs;
	pcie_ipc_rings = dev->pcie_ipc_rings;

	// Setup IPv4 and Ipv6 EtherType
	u32 = (0U
		| BCM_SBF(HTON16(HWA_TXPOST_ETHTYPE_IPV4),
			HWA_TX_TXPOST_ETHR_TYPE_ETHERNETTYPE1)
		| BCM_SBF(HTON16(HWA_TXPOST_ETHTYPE_IPV6),
			HWA_TX_TXPOST_ETHR_TYPE_ETHERNETTYPE2)
		| 0U);
	HWA_ASSERT(u32 == 0xdd860008);
	HWA_WR_REG_NAME(HWA3a, regs, tx, txpost_ethr_type, u32);

	// Initialize S2H schedule command interface
	u32 = HWA_PTR2UINT(txpost->schedcmd_ring.memory);
	HWA_WR_REG_NAME(HWA3a, regs, tx, fw_cmdq_base_addr, u32);

	u32 = (0U
		| BCM_SBF(txpost->schedcmd_ring.depth, HWA_TX_FW_CMDQ_CTRL_FWCMDQUEUEDEPTH)
		| 0U);
	HWA_WR_REG_NAME(HWA3a, regs, tx, fw_cmdq_ctrl, u32);

	// HWA3a CmdQ RD index update interrupt will be masked
	u32 = (0U // lazy interrupt for schedcmd_ring
		| BCM_SBF(HWA_TXPOST_SCHEDCMD_RING_LAZYCOUNT,
			HWA_TX_FW_CMDQ_LAZY_INTR_LAZYCOUNT)
		| BCM_SBF(HWA_TXPOST_SCHEDCMD_RING_LAZYTMOUT,
			HWA_TX_FW_CMDQ_LAZY_INTR_LAZYINTRTIMEOUT)
		| 0U);
	HWA_WR_REG_NAME(HWA3a, regs, tx, fw_cmdq_lazy_intr, u32);

	// Initialize H2S packet chain interface
	u32 = HWA_PTR2UINT(txpost->pktchain_ring.memory);
	HWA_WR_REG_NAME(HWA3a, regs, tx, pkt_chq_base_addr, u32);

	u32 = (0U
		| BCM_SBF(txpost->pktchain_ring.depth, HWA_TX_PKT_CHQ_CTRL_PKTCHAINQUEUEDEPTH)
		| BCM_SBF(HWA_TXPOST_PKTCHAIN_RING_LAZYCOUNT,
			HWA_TX_PKT_CHQ_CTRL_PKTCHAINLAZYCOUNT)
		| BCM_SBF(HWA_TXPOST_PKTCHAIN_RING_LAZYTMOUT,
			HWA_TX_PKT_CHQ_CTRL_PKTCHAINLAZYINTRTIMEOUT)
		| 0U);
	HWA_WR_REG_NAME(HWA3a, regs, tx, pkt_chq_ctrl, u32);

	// FRC table base address in dongle memory
	u32 = HWA_PTR2UINT(txpost->frc_table);
	HWA_WR_REG_NAME(HWA3a, regs, tx, txpost_frc_base_addr, u32);

#if HWA_REVISION_GE_129
	if (dev->tx_pktdealloc) {
		// Initialize S2H "FREEIDXTX" interface
		u32 = HWA_PTR2UINT(txpost->txfree_ring.memory);
		HWA_WR_REG_NAME(HWA3a, regs, tx, pktdealloc_ring_addr, u32);

		u32 = BCM_SBF(txpost->txfree_ring.depth,
				HWA_TX_PKTDEALLOC_RING_DEPTH_PKTDEALLOCRINGRDDEPTH);
		HWA_WR_REG_NAME(HWA3a, regs, tx, pktdealloc_ring_depth, u32);
		u32 = (0U
			| BCM_SBF(HWA_TXPOST_RING_INTRAGGR_COUNT,
				HWA_TX_PKTDEALLOC_RING_LAZYINTRCONFIG_PKTDEALLOCRINGINTRAGGRCOUNT)
			| BCM_SBF(HWA_TXPOST_RING_INTRAGGR_TMOUT,
				HWA_TX_PKTDEALLOC_RING_LAZYINTRCONFIG_PKTDEALLOCRINGINTRAGGRTIMER)
			| 0U);
		HWA_WR_REG_NAME(HWA3a, regs, tx, pktdealloc_ring_lazyintrconfig, u32);
	}
#endif /* HWA_REVISION_GE_129 */

	{   // Update txpost_config::TxPostLocalMemDepth to be multiple of WI size
		uint8 wi_size; // size of a TxPost work item
		uint32 num_wi; // number of work items
		uint32 local_mem_depth; // HWA3a local memory to save DMAed work items
#if HWA_REVISION_EQ_128
		uint32 stats_reg_set; // HWA3a number of statistics register sets

		BCM_REFERENCE(stats_reg_set);
#endif // endif

		// Read HW default value
		u32 = HWA_RD_REG_NAME(HWA3a, regs, tx, txpost_config);
		local_mem_depth =
			BCM_GBF(u32, HWA_TX_TXPOST_CONFIG_TXPOSTLOCALMEMDEPTH);
#if HWA_REVISION_EQ_128
		stats_reg_set =
			BCM_GBF(u32, HWA_TX_TXPOST_CONFIG_NUMSTATISTICSREGSET);
#endif // endif

#if HWA_REVISION_EQ_128 && (HWA_CHIP_GENERIC == 0x43684)
		/* A0: local_mem_depth HW default is not correct */
		HWA_PRINT("%s HW(rev128) default TxPostConfig<0x%08x>, override "
			"local_mem_depth<%u> to <%u>\n",
			HWA3a, u32, local_mem_depth, HWA_TXPOST_LOCAL_MEM_DEPTH);
		local_mem_depth = HWA_TXPOST_LOCAL_MEM_DEPTH;
		HWA_ASSERT(local_mem_depth == HWA_TXPOST_LOCAL_MEM_DEPTH);
		HWA_ASSERT(stats_reg_set == HWA_TXPOST_FRC_SRS_MAX);
#endif /* HWA_REVISION_EQ_128 && HWA_CHIP_GENERIC == 0x43684 */

		/* HWA 2.x use HWA_TXPOST_CWI64_BYTES always */
		HWA_ASSERT(HWA_REVISION_GE_128);
		// 1536 x 4 is a multiple of 192 hwa_txpost_cwi64_t workitems
		num_wi = (local_mem_depth * HWA_TXPOST_MEM_ADDR_W)
			/ HWA_TXPOST_CWI64_BYTES;
		// NOTE: The # of WI in HWA local memory cannot > 128
		// Override the local_mem_depth to meet maximum 128 workitems.
		if (num_wi > HWA_TXPOST_LOCAL_WORKITEMS) {
			HWA_PRINT("HWA_64BIT_ADDRESSING: adjust num_wi <%u> to <%u>\n",
				num_wi, HWA_TXPOST_LOCAL_WORKITEMS);
			num_wi = HWA_TXPOST_LOCAL_WORKITEMS;
		}

		local_mem_depth = (num_wi * HWA_TXPOST_CWI64_BYTES)
			/ HWA_TXPOST_MEM_ADDR_W;
		// new local_mem_depth = 1024, limited by maximum
		// HWA_TXPOST_LOCAL_WORKITEMS.

		wi_size = HWA_TXPOST_CWI64_BYTES;
		HWA_PRINT("%s(%d): HWA_TXPOST_CWI64_BYTES, num_wi<%u> local_mem_depth<%u>"
			" wi_size<%u>\n", __FUNCTION__, __LINE__, num_wi, local_mem_depth,
			wi_size);

		u32 = (0U
#if HWA_REVISION_GE_129
			| BCM_SBF(hwa_txpost_sequence_fw_command,
				HWA_TX_TXPOST_CONFIG_TXPOSTLOCALMEMMODE)
#endif // endif
			| BCM_SBF(local_mem_depth, HWA_TX_TXPOST_CONFIG_TXPOSTLOCALMEMDEPTH)
#if HWA_REVISION_EQ_128
			| BCM_SBIT(HWA_TX_TXPOST_CONFIG_TXPOSTBLOCKENABLE)
			// | BCM_SBIT(HWA_TX_TXPOST_CONFIG_SEQNUMENABLE) HWA1.0
			| BCM_SBF(stats_reg_set, HWA_TX_TXPOST_CONFIG_NUMSTATISTICSREGSET)
#else
			| BCM_SBF(HWA_TXPOST_MIN_FETCH_THRESH_FREEIDX,
				HWA_TX_TXPOST_CONFIG_MIN_FETCH_THRESH_TXPPKTDEALLOC)
			| BCM_SBF((dev->tx_pktdealloc) ? 1 : 0,
				HWA_TX_TXPOST_CONFIG_SWTXPKTDEALLOCIFENABLE)
#endif // endif
			| BCM_SBIT(HWA_TX_TXPOST_CONFIG_AUTOSTARTNEXTEMPYLOCDISABLE)
			| BCM_SBIT(HWA_TX_TXPOST_CONFIG_TXPKTPKTNEXTDEALLOCHW)
			| 0U);
		HWA_WR_REG_NAME(HWA3a, regs, tx, txpost_config, u32);

		u32 = (0U
			| BCM_SBF(wi_size, HWA_TX_TXPOST_WI_CTRL_WORKITEMSIZE)
			| BCM_SBF(0U, HWA_TX_TXPOST_WI_CTRL_WORKITEMOFFSET)
			| BCM_SBF(wi_size, HWA_TX_TXPOST_WI_CTRL_WORKITEMSIZECOPY)
			| 0U);
		HWA_WR_REG_NAME(HWA3a, regs, tx, txpost_wi_ctrl, u32);

		txpost->wi_size = wi_size;

		u32 = (0U
			| BCM_SBF(0, HWA_TXPOST_FRC_AGGR_DEPTH)
			| BCM_SBF(HWA_TXPOST_FRC_COMPACT_MSG_FORMAT,
				HWA_TXPOST_FRC_AGGR_MODE)
			| 0U);

		// Ensure all FRC configuration use common aggr_spec
		txpost->aggr_spec =
			(uint8)(u32 >> HWA_TXPOST_FRC_AGGR_DEPTH_SHIFT);
		HWA_TRACE(("%s init wi_size<%u> aggr_spec<%x>\n",
			HWA3a, txpost->wi_size, txpost->aggr_spec));
	}

	// Enable HWA3a to BZERO the rest of the SW Packet
	u32 = (0U
#ifdef HWA_TXPOST_BZERO_PKTTAG
		| BCM_SBIT(HWA_TX_TXPOST_CFG1_BZERO_PKTTAG_SUPPORT)
#endif // endif
#if HWA_REVISION_GE_129
		| BCM_SBIT(HWA_TX_TXPOST_CFG1_BIT63OFPAYLOADADDR)
		| BCM_SBIT(HWA_TX_TXPOST_CFG1_SETSOFEOF4WIF)
#endif /* HWA_REVISION_GE_129 */
		| 0U);
	HWA_WR_REG_NAME(HWA3a, regs, tx, txpost_cfg1, u32);

	// Settings "NotPCIE, Coherent and AddrExt" for misc HW DMA transactions
	u32 = (0U /* 19b = 0x0007FF22 */
		// | BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMAPCIEDESCTEMPLATENOTPCIE)
		| BCM_SBF(dev->host_coherency,
		          HWA_TX_DMA_DESC_TEMPLATE_DMAPCIEDESCTEMPLATECOHERENT)
		| BCM_SBF(0, HWA_TX_DMA_DESC_TEMPLATE_DMAPCIEDESCTEMPLATEADDREXT)
		// | BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_ dmapcierdidxdesctemplatenotpcie)
		| BCM_SBF(dev->host_coherency,
		          HWA_TX_DMA_DESC_TEMPLATE_DMAPCIERDIDXDESCTEMPLATECOHERENT)
		| BCM_SBF(0, HWA_TX_DMA_DESC_TEMPLATE_DMAPCIERDIDXDESCTEMPLATEADDREXT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMARDIDXDESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMARDIDXDESCTEMPLATECOHERENT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMACMDQDESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMACMDQDESCTEMPLATECOHERENT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMAFRCDESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMAFRCDESCTEMPLATECOHERENT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMATXPKTDESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMATXPKTDESCTEMPLATECOHERENT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMAPKTCHQDESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMAPKTCHQDESCTEMPLATECOHERENT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_DMAHWADESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_NONDMAFRCREADTEMPLATECOHERENT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_NONDMAPLAVGFRCWRTEMPLATECOHERENT)
		| BCM_SBIT(HWA_TX_DMA_DESC_TEMPLATE_NONDMAPKTTAGWRTEMPLATECOHERENT)
		| 0U);
	HWA_WR_REG_NAME(HWA3a, regs, tx, dma_desc_template, u32);

	{   // Setup base addresses of indices arrays in dongle and host

		// Exclude the RD WR indices of H2D Common rings

		// WR indices base address in dongle memory
		u32 = pcie_ipc_rings->h2d_wr_daddr32 + HWA_TXPOST_FLOWRING_RDWR_BASE_OFFSET;
		HWA_WR_REG_NAME(HWA3a, regs, tx, h2d_wr_ind_array_base_addr, u32);

		// RD indices base address in dongle memory
		u32 = pcie_ipc_rings->h2d_rd_daddr32  + HWA_TXPOST_FLOWRING_RDWR_BASE_OFFSET;
		HWA_WR_REG_NAME(HWA3a, regs, tx, h2d_rd_ind_array_base_addr, u32);

		// RD indices base address in host memory
		u32 = HADDR64_LO(pcie_ipc_rings->h2d_rd_haddr64)
		      + HWA_TXPOST_FLOWRING_RDWR_BASE_OFFSET;
		HWA_WR_REG_NAME(HWA3a, regs, tx,
			h2d_rd_ind_array_host_base_addr_l, u32);
		HWA_WR_REG_NAME(HWA3a, regs, tx, h2d_rd_ind_array_host_base_addr_h,
			HWA_HOSTADDR64_HI32(HADDR64_HI(pcie_ipc_rings->h2d_rd_haddr64)));
	}

	hwa_txpost_frc_table_init(txpost);
#if !defined(HWA_NO_LUT)
	hwa_txpost_prio_lut_init(txpost);
	hwa_txpost_sada_swt_init(txpost);
	hwa_txpost_flow_swt_init(txpost);
#endif /* !HWA_NO_LUT */

	// Assign the interested txpost interrupt mask.
	dev->defintmask |= HWA_COMMON_INTSTATUS_TXPKTCHN_INT_MASK;

} // hwa_txpost_init

void // HWA3a: Deinit TxPost block
hwa_txpost_deinit(hwa_txpost_t *txpost)
{
}

void
hwa_txpost_wait_to_finish(hwa_txpost_t *txpost)
{
	hwa_dev_t *dev;
	uint32 u32, loop_count;
	uint32 busy;

	HWA_FTRACE(HWA3a);

	// Audit pre-conditions
	dev = HWA_DEV(txpost);

	// Wait until HWA cons all 3a schedcmd ring
	loop_count = HWA_FSM_IDLE_POLLLOOP;
	while (!hwa_ring_is_cons_all(&txpost->schedcmd_ring)) {
		HWA_TRACE(("%s HWA consuming 3a schedcmd ring\n",
			__FUNCTION__));
		OSL_DELAY(1);
		if (--loop_count == 0) {
			HWA_ERROR(("%s Cannot consume 3a schedcmd ring\n", __FUNCTION__));
			break;
		}
	}

#if HWA_REVISION_GE_129
	if (dev->tx_pktdealloc) {
		// Wait until HWA cons all 3a txfree ring
		loop_count = HWA_FSM_IDLE_POLLLOOP;
		while (!hwa_ring_is_cons_all(&txpost->txfree_ring)) {
			HWA_TRACE(("%s HWA consuming 3a txfree ring\n",
			__FUNCTION__));
			OSL_DELAY(1);
			if (--loop_count == 0) {
				HWA_ERROR(("%s Cannot consume 3a txfree ring\n", __FUNCTION__));
				break;
			}
		}
	}
#endif /* HWA_REVISION_GE_129 */

	// Poll txpost_status_reg2 to make sure it's done
	loop_count = 0;
	do {
		if (loop_count)
			OSL_DELAY(1);
		u32 = HWA_RD_REG_NAME(HWA3a, dev->regs, tx, txpost_status_reg2);
		HWA_TRACE(("%s Polling txpost_status_reg2 <0x%x>\n",
			__FUNCTION__, u32));
		busy = BCM_GBF(u32, HWA_TX_TXPOST_STATUS_REG2_CMDFRCDMAFSM);
		busy |= BCM_GBF(u32, HWA_TX_TXPOST_STATUS_REG2_TXPDMAFSM);
		busy |= BCM_GBF(u32, HWA_TX_TXPOST_STATUS_REG2_SWTXPKTPROCFSM);
		busy |= BCM_GBF(u32, HWA_TX_TXPOST_STATUS_REG2_CLKREQ);
	} while (busy && ++loop_count != HWA_FSM_IDLE_POLLLOOP);
	if (loop_count == HWA_FSM_IDLE_POLLLOOP) {
		HWA_ERROR(("%s txpost_status_reg2 is not idle <0x%x>\n",
			__FUNCTION__, u32));
	}
}

/* Initialize lbuf compatibile structures */
void
hwa_txpost_bm_lb_init(hwa_dev_t *dev, void *memory, uint16 pkt_total,
	uint16 pkt_size, uint16 hw_size)
{
	int i;
	char *txbuf, *head;

	ASSERT(pkt_size > hw_size);

	head = (char *)memory;

	HWA_TRACE(("XXX: pktid %d want %d\n", hnd_pktid_free_cnt(), pkt_total));

	for (i = 0; i < pkt_total; i++) {
		txbuf = head + (pkt_size * i);
		PKTINIT(dev->osh, (void *)(txbuf + hw_size), lbuf_frag,
			(pkt_size - hw_size));
	}
}

/*
 * -----------------------------------------------------------------------------
 * Section: Support for requesting HWA to DMA RD indices
 * -----------------------------------------------------------------------------
 */

INLINE void // Request HWA to transfer count number of RD indices, given offset
hwa_txpost_rdidx_update(hwa_txpost_t *txpost, uint32 offset, uint32 count)
{
	uint32 u32;
	uint32 dma_busy; // busy status of a previous DMA transfer
	uint32 loop_count; // number of times to try for a previous DMA to finish
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	HWA_TRACE(("%s rdidx update offset<%u> count<%u>\n",
		HWA3a, offset, count));

	// Audit pre-conditions
	dev = HWA_DEV(txpost);

	// Setup locals
	regs = dev->regs;
	loop_count = HWA_TXPOST_RDIDX_DMA_BUSY_BURNLOOP;

	do { // Burnloop: allowing HWA3a to complete a previous RdIdx DMA
		u32 = HWA_RD_REG_NAME(HWA3a, regs, tx, txp_host_rdidx_update_reg);
		dma_busy =
		    BCM_GBIT(u32,
		        HWA_TX_TXP_HOST_RDIDX_UPDATE_REG_TXPHOSTRDIDXTRANSFERBUSY);

	} while (dma_busy && loop_count--);

	if (loop_count <= 0) {
		HWA_ERROR(("%s rdidx update loop_count<%u> reg_val<0x%08x>\n",
			HWA3a, loop_count, u32));
		HWA_ASSERT(loop_count > 0);
	}

	//NOTE: actually the "count" is in "bytes". HWA handle it as bytes not count.
	count = count * HWA_PCIE_RW_INDEX_SZ;

	u32 = (0U
		| BCM_SBF(offset,
		        HWA_TX_TXP_HOST_RDIDX_UPDATE_REG_TXPHOSTRDIDXARRAYOFFSET)
		| BCM_SBF(count,
		        HWA_TX_TXP_HOST_RDIDX_UPDATE_REG_TXPHOSTRDIDXUPDATECOUNT)
		| 0U);
	HWA_WR_REG_NAME(HWA3a, regs, tx, txp_host_rdidx_update_reg, u32);

} // hwa_txpost_rdidx_update

void // Request HWA to update the readindex for a flowring, given ring_id
hwa_txpost_flowring_rdidx_update(hwa_txpost_t *txpost, uint32 ring_id)
{
	uint32 offset, frc_id;

	HWA_FTRACE(HWA3a);

	frc_id = HWA_TXPOST_FLOWRINGID_2_FRCID(ring_id); // Convert ring_id to FRCid
	offset = HWA_PCIE_RW_INDEX_SZ * frc_id; // compute RD index offset

	HWA_TRACE(("%s ring<%u> frc<%u> rdidx update offset<%u>\n",
		HWA3a, ring_id, frc_id, offset));

	hwa_txpost_rdidx_update(txpost, offset, 1);

} // hwa_txpost_flowring_rdidx_update

/*
 * -----------------------------------------------------------------------------
 * Section: Support for posting Schedule Commands in SchedCmd S2H hwa_ring
 * -----------------------------------------------------------------------------
 */

int // Handle a flowring schedule command from WLAN driver, returns trans_id.
hwa_txpost_schedcmd_request(struct hwa_dev *dev,
	uint32 flowring_id, uint16 rd_idx, uint32 transfer_count)
{
	hwa_ring_t *schedcmd_ring; // schedcmd ring context
	hwa_txpost_schedcmd_t *schedcmd; // Element in a schedcmd ring
	hwa_txpost_t *txpost;
	hwa_bm_t *bm;
	uint32 frc_id;

	HWA_ASSERT(transfer_count != 0);
	HWA_ASSERT(dev != (struct hwa_dev *)NULL);
	HWA_FTRACE(HWA3a);

	bm = &dev->tx_bm;
	if (bm->avail_sw < transfer_count) {
		HWA_TRACE(("%s avail_sw<%u> transfer_count<%u>\n",
			__FUNCTION__, bm->avail_sw, transfer_count));
		HWA_STATS_EXPR(bm->avail_sw_low++);
		goto failure;
	}

	txpost = &dev->txpost;
	schedcmd_ring = &txpost->schedcmd_ring;

	if (hwa_ring_is_full(schedcmd_ring) || transfer_count == 0)
		goto failure;

	if (txpost->flowring_id[txpost->schedcmd_id] != 0) {
		HWA_TRACE(("%s schedcmd_id<%u> process is not finish\n",
			__FUNCTION__, txpost->schedcmd_id));
		goto failure;
	}

	// Find location where schedcmd needs to be constructed, and populate it
	schedcmd = HWA_RING_PROD_ELEM(hwa_txpost_schedcmd_t, schedcmd_ring);

	// Design Note: Experimental Octet Count mode not supported.
	frc_id = HWA_TXPOST_FLOWRINGID_2_FRCID(flowring_id);
	schedcmd->u32[0] = (
		BCM_SBF(frc_id, HWA_TXPOST_SCHEDCMD_FLOWRING_ID) |
		BCM_SBF(HWA_TXPOST_SCHEDCMD_WORKITEMS,
			HWA_TXPOST_SCHEDCMD_TRANSFER_TYPE));
	schedcmd->u32[1] =
		BCM_SBF(transfer_count, HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT);
	schedcmd->rd_idx = rd_idx;
	schedcmd->trans_id = txpost->schedcmd_id;

	HWA_DEBUG_EXPR({
		HWA_ASSERT(schedcmd->transfer_type == HWA_TXPOST_SCHEDCMD_WORKITEMS);
		txpost->wi_count[txpost->schedcmd_id] = transfer_count;
	});

	// Recoder the flowring_id per schedcmd_id for flowid
	// lookup when 3a audit failure.
	txpost->flowring_id[txpost->schedcmd_id] = flowring_id;

	hwa_ring_prod_upd(schedcmd_ring, 1, TRUE); // update/commit WR

	HWA_TRACE(("%s sched[%u,%u] ring<%u> cmd<%u> rd<%u> count<%u> id<%u>\n",
		HWA3a, HWA_RING_STATE(schedcmd_ring)->write,
		HWA_RING_STATE(schedcmd_ring)->read,
		schedcmd->flowring_id, schedcmd->transfer_type, schedcmd->rd_idx,
		schedcmd->transfer_count, schedcmd->trans_id));

	//Increase schedcmd_id
	txpost->schedcmd_id = NEXTTXP(txpost->schedcmd_id, HWA_TXPOST_SCHEDCMD_RING_DEPTH);

	HWA_BCMDBG_EXPR({
		if (transfer_count > HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT)
			txpost->schecmd_histogram[HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT]++;
		else
			txpost->schecmd_histogram[transfer_count-1]++;
	});

	bm->avail_sw -= transfer_count;

	return schedcmd->trans_id;

failure:
	HWA_TRACE(("%s schedcmd failure ring<%u> rd<%u> count<%u>\n",
		HWA3a, flowring_id, rd_idx, transfer_count));

	return HWA_FAILURE;

} // hwa_txpost_schedcmd_request

/*
 * -----------------------------------------------------------------------------
 * Section: Support for retrieving Packet Chains from PktChain H2S hwa_ring
 * -----------------------------------------------------------------------------
 */

int // Consume all packet chains in packet chain ring
hwa_txpost_pktchain_process(hwa_dev_t *dev)
{
	bool pend_trans;
	uint32 elem_ix; // location of next element to read
	void *head, *tail; // pktchain head pkt and tail pkt pointer
	hwa_txpost_t *txpost; // SW txpost state
	hwa_ring_t *h2s_ring; // H2S pktchain ring
	hwa_txpost_pktchain_t *pktchain; // element in a pktchain ring
	hwa_handler_t *sendup_handler; // upstream processing handler
	hwa_handler_t *trans_handler; // schedcmd transaction completion handler
	uint8 trans_id; // current trans_id
	uint16 pkt_count; // current pkt_count;
	uint32 total_octets; // current total_octets;

	HWA_FTRACE(HWA3a);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	txpost = &dev->txpost;

	// Fetch registered upstream callback handlers
	sendup_handler = &dev->handlers[HWA_PKTCHAIN_SENDUP];
	trans_handler = &dev->handlers[HWA_SCHEDCMD_DONE];

	h2s_ring = &txpost->pktchain_ring;

	hwa_ring_cons_get(h2s_ring); // fetch HWA pktchain ring's WR index once
	pend_trans = FALSE;
	pktchain = (hwa_txpost_pktchain_t*)NULL;
	trans_id = (HWA_TXPOST_SCHEDCMD_RING_DEPTH - 1);
	pkt_count = 0;
	total_octets = 0;

	// Consume all packet chains in ring, sending them upstream
	while ((elem_ix = hwa_ring_cons_upd(h2s_ring)) != BCM_RING_EMPTY) {

		// Fetch location of next packet chain to process
		pktchain = HWA_RING_ELEM(hwa_txpost_pktchain_t, h2s_ring, elem_ix);
		pend_trans = TRUE; // remember to issue a schedcmd done callback

		// Get current pktchain info.
		head = HWA_UINT2PTR(void, pktchain->head);
		tail = HWA_UINT2PTR(void, pktchain->tail);
		trans_id = pktchain->trans_id;
		pkt_count = pktchain->pkt_count;
		total_octets = pktchain->total_octets;

		HWA_STATS_EXPR(txpost->pkt_proc_cnt += pkt_count);
		HWA_STATS_EXPR(txpost->oct_proc_cnt += total_octets);

		HWA_TRACE(("%s pktchain<%p> id<%u> chn<%x,%x> cnt<%x,%x>\n", HWA3a,
			pktchain, trans_id, HWA_PTR2UINT(head), HWA_PTR2UINT(tail),
			pkt_count, total_octets));

		hwa_ring_cons_put(h2s_ring); // commit RD index immediately

		/* Don't use pktchain point to retrieve each filed after SW commit RD index,
		 * HWA may update new one to the correspending RD index.
		 */

		HWA_DEBUG_EXPR(
			txpost->wi_count[trans_id] -= pkt_count);

		// schedcmd to pktchain transaction management
		if (trans_id != txpost->pktchain_id) {
			HWA_DEBUG_EXPR(
				// Ensure that HWA3a indeed complete entire schedcmd
				HWA_ASSERT(txpost->wi_count[txpost->pktchain_id] == 0);

				// Ensure that HWA3a has not skipped a schedcmd
				HWA_ASSERT(trans_id == NEXTTXP(txpost->pktchain_id,
					HWA_TXPOST_SCHEDCMD_RING_DEPTH)));

			// Inform upstream that schedcmd transaction has indeed completed
			(*trans_handler->callback)(trans_handler->context,
				HWA_SCHEDCMD_CONFIRMED_DONE, 0,
				txpost->pktchain_id, trans_id);

			// Clear flowring_id
			txpost->flowring_id[txpost->pktchain_id] = 0;

			pend_trans = FALSE; // schedcmd done notification sent upstream
			txpost->pktchain_id = trans_id; // start next trans
		}

		// Update flowid_override if it's invalid
		if (pkt_count == 1) {
			uint32 flowid;
			hwa_txpost_pkt_t *txpost_pkt;

			txpost_pkt = (hwa_txpost_pkt_t *)head;
			flowid = txpost_pkt->flowid_override;
			if (flowid == HWA_TXPOST_FLOWID_NULL ||
				flowid == HWA_TXPOST_FLOWID_INVALID) {
				txpost_pkt->flowid_override = txpost->flowring_id[trans_id];
				HWA_TRACE((" %s: flowid_override <%u> is invalid update to <%u>\n",
					HWA3a, flowid, txpost_pkt->flowid_override));
			}
		}
		HWA_DEBUG_EXPR(HWA_PRINT(" %s: trans_id <%u> head <%p>\n", HWA3a, trans_id, head));

		// Send packet chain upstream
		(*sendup_handler->callback)(sendup_handler->context,
			(uintptr)head, (uintptr)tail,
			pkt_count, total_octets);
	} // while pktchain_ring not empty

	// Inform upstream that schedcmd transaction potentially completed
	if (pend_trans == TRUE) {
		HWA_ASSERT(pktchain != (hwa_txpost_pktchain_t*)NULL);
		(*trans_handler->callback)(trans_handler->context,
			HWA_SCHEDCMD_POTENTIALLY_DONE, 0,
			txpost->pktchain_id, trans_id);
		// do pktchain_id update if SchedCmd ring is empty.
		if (hwa_ring_is_empty(&txpost->schedcmd_ring)) {
			txpost->pktchain_id = trans_id;
		}
	}

	return HWA_SUCCESS;

} // hwa_txpost_pktchain_process

/*
 * -----------------------------------------------------------------------------
 * Section: Flow Ring Configuration [FRC] management
 * -----------------------------------------------------------------------------
 */

void // Initialize FRC table: each FRC initiaized to default RESET values
hwa_txpost_frc_table_init(hwa_txpost_t *txpost)
{
	int frc_id;
	hwa_txpost_frc_t *frc;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_FTRACE(HWA3a);

	frc = txpost->frc_table;

	for (frc_id = 0; frc_id < HWA_TXPOST_FLOWRINGS_MAX; frc_id++) {

		frc->haddr64.loaddr = 0U; // --- RUNTIME

		frc->haddr64.hiaddr = HWA_PCI64ADDR_HI32; // --- RUNTIME

		// Compact format - NO Aggregation
		frc->aggr_depth = 0;
		frc->aggr_mode = HWA_TXPOST_FRC_COMPACT_MSG_FORMAT;

		// Base offset registered, excludes H2D common ring's indices.
		frc->wr_idx_offset = (HWA_PCIE_RW_INDEX_SZ * frc_id);

		frc->ring_size = 0; // --- RUNTIME
		frc->srs_idx = 0; // use SRS 0

		frc->flowid = HWA_TXPOST_FLOWID_INVALID; // CFP mode: from prio lut
		frc->lkup_override = HWA_TXPOST_FRC_LKUP_OVERRIDE_NONE;

		frc->pyld_min_length = 21; // The second packet of "ping -s 1473" is 21
		frc->ifid = HWA_IFID_INVALID; // --- RUNTIME

		frc->lkup_type = HWA_TXPOST_FRC_LKUP_TYPE_b11; // prio lut

		frc->etype_ip_enable = 1; // enable etype == IPv4|6 check

#if HWA_REVISION_GE_129
		frc->audit_enable = 1; // Enable audit
#else
		frc->audit_enable = 0; // Disable audit
#endif // endif
		frc->avg_pkt_size = 0; // Octet scheduling not supported

		frc->epoch = 0; // no epoch audit

		frc++;
	}

} // hwa_txpost_frc_table_init

int // Configure a FRC - also used to restore a FRC to default RESET values
hwa_txpost_frc_table_config(struct hwa_dev *dev, uint32 ring_id,
	uint8 ifid, uint16 ring_size, dma64addr_t *haddr64)
{
	uint32 frc_id;
	hwa_txpost_frc_t *frc;
	hwa_txpost_t *txpost;

	HWA_ASSERT(dev != (struct hwa_dev*)NULL);

	txpost = &dev->txpost;
	HWA_ASSERT((ifid < HWA_INTERFACES_TOT) || (ifid == HWA_IFID_INVALID));
	HWA_ASSERT(txpost->wi_size != 0);

	/*
	 * Have not provided means to define aggr_mode, aggr_depth.
	 * This needs to be a system wide specification due to TxPostLocalMemDepth
	 */

	HWA_TRACE(("%s frc config"
		" ring<%u> frc<%u> ifid<%u> size<%u> haddr<0x%08x,0x%08x>\n",
		HWA3a, ring_id, HWA_TXPOST_FLOWRINGID_2_FRCID(ring_id), ifid, ring_size,
		haddr64->loaddr, haddr64->hiaddr));

	// Convert PCIe IPC ring_id to a flowring id
	frc_id = HWA_TXPOST_FLOWRINGID_2_FRCID(ring_id); // convert ring_id to FRCid

	frc = txpost->frc_table + frc_id; // locate FRC

	// Apply RUNTIME values
	frc->ifid = ifid;
	frc->ring_size = ring_size;
	frc->haddr64.loaddr = haddr64->loaddr;
	frc->haddr64.hiaddr = HWA_HOSTADDR64_HI32(haddr64->hiaddr);

	// flowid lookup will use frc->flowid directly if it's valid.
	// So that we don't need to program prio, da/sa ... lookup tables.
	frc->flowid = ring_id;

	return ring_id;

} // hwa_txpost_frc_table_config

void // Customize a FRC
hwa_txpost_frc_table_customize(hwa_txpost_t *txpost, uint32 ring_id,
	hwa_txpost_frc_field_t field, uint32 arg1, uint32 arg2)
{
	uint32 frc_id;
	hwa_txpost_frc_t *frc;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	HWA_TRACE(("%s frc customize ring<%u> frc<%u> field<%u> args<%u><%u>\n",
		HWA3a, ring_id, HWA_TXPOST_FLOWRINGID_2_FRCID(ring_id),
		field, arg1, arg2));

	// Convert PCIe IPC ring_id to a flowring id
	frc_id = HWA_TXPOST_FLOWRINGID_2_FRCID(ring_id); // convert ring_id to FRCid

	frc = txpost->frc_table + frc_id; // locate FRC

	HWA_ASSERT(frc->ring_size != 0); // configure before customizing

	switch (field) {
		case HWA_TXPOST_FRC_SRS_IDX: frc->srs_idx = arg1; break;
		case HWA_TXPOST_FRC_FLOWID: frc->flowid = arg1; break;
		case HWA_TXPOST_FRC_LKUP_OVERRIDE: frc->lkup_override = arg1; break;
		case HWA_TXPOST_FRC_PYLD_MIN_LENGTH: frc->pyld_min_length = arg1; break;
		case HWA_TXPOST_FRC_LKUP_TYPE: frc->lkup_type = arg1; break;
		case HWA_TXPOST_FRC_ENABLES:
			frc->etype_ip_enable = arg1; frc->audit_enable = arg2; break;
	}

} // hwa_txpost_frc_table_customize

int // Restore a FRC to default RESET values on a flowring delete
hwa_txpost_frc_table_reset(struct hwa_dev *dev, uint32 ring_id)
{
	dma64addr_t haddr64 = { .lo = 0U, .hi = 0U };

	HWA_ASSERT(dev != (struct hwa_dev*)NULL);
	HWA_FTRACE(HWA3a);

	return hwa_txpost_frc_table_config(dev, ring_id, ~0, 0, &haddr64);

} // hwa_txpost_frc_table_reset

#if !defined(HWA_NO_LUT)

/*
 * -----------------------------------------------------------------------------
 * Section: Per Interface Priority based FlowId Lookup Table management
 * -----------------------------------------------------------------------------
 */

static void _hwa_txpost_prio_lut_set(hwa_txpost_t *txpost,
	uint8 ifid, const uint16 *flowid); // program the HWA and local SW state.

void // Initialize the prio lut table
hwa_txpost_prio_lut_init(hwa_txpost_t *txpost)
{
	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_FTRACE(HWA3a);

	// SW limit on the max number of interfaces based prio lut
	HWA_ASSERT(HWA_INTERFACES_MAX <= HWA_TXPOST_PRIO_CFG_MAX);

} // hwa_txpost_prio_lut_init

static void // Configure the flowids for all priorities for a given interface
_hwa_txpost_prio_lut_set(hwa_txpost_t *txpost,
	uint8 ifid, const uint16 *flowid)
{
	uint8 prio;
	uint16 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;

	sys_mem = txpost->prio_lut.if_table[ifid].flowid_table;
	axi_mem_addr = txpost->prio_addr
	             + (ifid * sizeof(hwa_txpost_prio_flowid_t));

	for (prio = 0; prio < HWA_PRIORITIES_MAX; prio++) {
		HWA_ASSERT(*(flowid+prio) <= HWA_TXPOST_FLOWID_INVALID);
		*(sys_mem + prio) = *(flowid + prio); // Fill SW state
	}

	// Flush SW state to HWA AXI memory
	HWA_WR_MEM16(HWA3a, hwa_txpost_prio_flowid_t, axi_mem_addr, sys_mem);

} // _hwa_txpost_prio_lut_set

void // Configure the flowids for all priorities for a given interface
hwa_txpost_prio_lut_config(hwa_txpost_t *txpost,
	uint8 ifid, const uint16 *flowid)
{
	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_ASSERT(ifid < HWA_INTERFACES_MAX);
	HWA_FTRACE(HWA3a);

	_hwa_txpost_prio_lut_set(txpost, ifid, flowid);
	setbit(txpost->prio_cfg, ifid);

} // hwa_txpost_prio_lut_config

void // Configure the flowids for all priorities for a given interface
hwa_txpost_prio_lut_reset(hwa_txpost_t *txpost, uint8 ifid)
{
	const uint16 flowid_lut[HWA_PRIORITIES_MAX] = { 0 };

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_ASSERT(ifid < HWA_INTERFACES_MAX);
	HWA_FTRACE(HWA3a);

	_hwa_txpost_prio_lut_set(txpost, ifid, flowid_lut);
	clrbit(txpost->prio_cfg, ifid);

} // hwa_txpost_prio_lut_reset

/*
 * -----------------------------------------------------------------------------
 * Section: Unique SADA AXI LUT and SW table management
 * -----------------------------------------------------------------------------
 */
static const uint16 sada_null[3] = { 0x0000, 0x0000, 0x0000 };

void // Initialize SW table, by placing entries in free list, 0th in active list
hwa_txpost_sada_swt_init(hwa_txpost_t *txpost)
{
	int i;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_FTRACE(HWA3a);

	// SW limit on the max number of unique SADA for FLow LUT
	HWA_ASSERT(HWA_TXPOST_SADA_LUT_DEPTH <= HWA_TXPOST_SADA_CFG_MAX);

	// sada 0th table element is reserved as sada_null and placed in active list
	txpost->sada_swt.table[0].next = (hwa_txpost_sada_t*)NULL;
	txpost->sada_alist = &txpost->sada_swt.table[0];
	setbit(txpost->sada_cfg, 0);

	// all other entries are in free list
	for (i = HWA_TXPOST_SADA_LUT_DEPTH - 1; i; i--) {
		txpost->sada_swt.table[i].next = txpost->sada_flist;
		txpost->sada_flist = &txpost->sada_swt.table[i];
	}

} // hwa_txpost_sada_swt_init

int // Find the index of a sada in the SADA LUT
hwa_txpost_sada_swt_find(hwa_txpost_t *txpost, const uint16 *ea)
{
	hwa_txpost_sada_t *sada; // active list iterator
	hwa_txpost_sada_t *sada_swt_table;

	sada_swt_table = txpost->sada_swt.table;

	for (sada = txpost->sada_alist; sada != NULL; sada = sada->next) {

		if (eacmp(sada->elem.sada, ea) == 0) {
			return HWA_TABLE_INDX(hwa_txpost_sada_t, sada_swt_table, sada);
		}
	}

	return HWA_FAILURE;

} // hwa_txpost_sada_swt_find

int // Find or add a sada entry to table, and return table index or failure
hwa_txpost_sada_swt_insert(hwa_txpost_t *txpost, const uint16 *ea)
{
	hwa_txpost_sada_t *sada; // active list iterator
	hwa_txpost_sada_t *sada_swt_table;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_TRACE(("%s sada insert ea<%04x%04x%04x>\n",
		HWA3a, ea[0], ea[1], ea[2]));

	sada_swt_table = txpost->sada_swt.table;

	// Search table if entry already exists
	for (sada = txpost->sada_alist; sada != NULL; sada = sada->next) {
		if (eacmp(sada->elem.sada, ea) == 0) { // match found
			goto found;
		}
	}

	// Find a free slot
	if ((sada = txpost->sada_flist) == (hwa_txpost_sada_t*)NULL) {
		HWA_STATS_EXPR(txpost->sada_err_cnt++);
		HWA_WARN(("%s sada insert ea<%04x%04x%04x> overflow\n",
			HWA3a, ea[0], ea[1], ea[2]));
		return HWA_FAILURE; // no free slot
	}

	// Fetch element from free list and move to active list
	txpost->sada_flist = sada->next; // pop from free list
	sada->next = txpost->sada_alist; // add to active list
	txpost->sada_alist = sada;
	eacopy(ea, sada->elem.u16); // configure SW state

	// Design Note: HWA' SADA LUT is not yet configured.

	HWA_STATS_EXPR(txpost->sada_ins_cnt++);

found:
	return HWA_TABLE_INDX(hwa_txpost_sada_t, sada_swt_table, sada);

} // hwa_txpost_sada_swt_insert

int // Delete a sada entry and return the index in the table
hwa_txpost_sada_swt_delete(hwa_txpost_t *txpost, const uint16 *ea)
{
	hwa_txpost_sada_t *sada, *prev; // active list iterator and prev entry
	hwa_txpost_sada_t *sada_swt_table;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	HWA_TRACE(("%s sada delete ea<%04x%04x%04x>\n",
		HWA3a, ea[0], ea[1], ea[2]));

	if (eacmp(sada_null, ea) == 0) {
		return 0; // sada_null entry at index 0 is never deleted.
	}

	sada_swt_table = txpost->sada_swt.table;

	// Search the active list for a matching ea entry
	for (sada = txpost->sada_alist, prev = NULL;
		sada != NULL; prev = sada, sada = sada->next)
	{
		if (eacmp(sada->elem.sada, ea) == 0) { // found an entry

			if (prev == NULL) { // head entry in active list
				txpost->sada_alist = sada->next;
			} else {
				prev->next = sada->next;
			}
			sada->next = txpost->sada_flist;
			txpost->sada_flist = sada;
			eacopy(sada_null, sada->elem.u16);

			HWA_STATS_EXPR(txpost->sada_del_cnt++);

			return HWA_TABLE_INDX(hwa_txpost_sada_t, sada_swt_table, sada);
		}
	}

	HWA_ERROR(("%s sada delete ea<%04x%04x%04x> failure\n",
		HWA3a, ea[0], ea[1], ea[2]));

	HWA_ASSERT(0); // No matching SADA entry found for deletion. SW bug.

	return HWA_FAILURE;

} // hwa_txpost_sada_swt_delete

void // Audit SADA SW and HWA configuration consistency
hwa_txpost_sada_swt_audit(hwa_txpost_t *txpost)
{
	int sada_idx;
	hwa_txpost_sada_t *sada;
	hwa_txpost_sada_t *sada_swt_table;

	BCM_REFERENCE(sada_idx);

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	HWA_TRACE(("%s sada audit\n", HWA3a));

	sada_swt_table = txpost->sada_swt.table;

	// Audit active list
	sada = txpost->sada_alist;
	while (sada != NULL) {
		sada_idx = HWA_TABLE_INDX(hwa_txpost_sada_t, sada_swt_table, sada);
		HWA_ASSERT(sada_idx < HWA_TXPOST_SADA_LUT_DEPTH);
		HWA_ASSERT(isset(txpost->sada_cfg, sada_idx));
	}

	// Audit free list
	sada = txpost->sada_flist;
	while (sada != NULL) {
		sada_idx = HWA_TABLE_INDX(hwa_txpost_sada_t, sada_swt_table, sada);
		HWA_ASSERT(sada_idx < HWA_TXPOST_SADA_LUT_DEPTH);
		HWA_ASSERT(isclr(txpost->sada_cfg, sada_idx));
	}

} // hwa_txpost_sada_swt_audit

void // Explicitly configure sada entry in HWA3a AXI SADA LUT
hwa_txpost_sada_lut_config(hwa_txpost_t *txpost, int sada_idx,
	bool insert)
{
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_ASSERT((sada_idx < HWA_TXPOST_SADA_LUT_DEPTH) && (sada_idx > 0U));

	HWA_TRACE(("%s sada config %d\n", HWA3a, sada_idx));

	// Flush SW state to HW, making states consistent
	if (insert == TRUE)
		setbit(txpost->sada_cfg, sada_idx);
	else
		clrbit(txpost->sada_cfg, sada_idx);

	sys_mem = txpost->sada_swt.table[sada_idx].elem.u32;
	axi_mem_addr = HWA_TABLE_ADDR(hwa_txpost_sada_lut_elem_t,
	                                  txpost->sada_addr, sada_idx);

	// Flush SW state to HWA AXI memory, even if redundant.
	HWA_WR_MEM32(HWA3a, hwa_txpost_sada_lut_elem_t, axi_mem_addr, sys_mem);

} // hwa_txpost_sada_lut_config

/*
 * -----------------------------------------------------------------------------
 * Section: FlowId Hash based AXI LUT and SW table management
 * -----------------------------------------------------------------------------
 */

void // Initialize the flowid software lookup table
hwa_txpost_flow_swt_init(hwa_txpost_t *txpost)
{
	int i;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_FTRACE(HWA3a);

	// First HWA_TXPOST_SADA_LUT_DEPTH are not placed in free list
	for (i = 0; i < HWA_TXPOST_SADA_LUT_DEPTH; i++) {
		txpost->flow_swt.table[i].next = NULL;
		txpost->flow_swt.table[i].u32  = 0U;
	}
	for (i = HWA_TXPOST_SADA_LUT_DEPTH; i < HWA_TXPOST_FLOW_LUT_DEPTH; i++) {
		txpost->flow_swt.table[i].next = txpost->flow_flist;
		txpost->flow_flist             = &txpost->flow_swt.table[i];
		txpost->flow_swt.table[i].u32  = 0U;
	}

} // hwa_txpost_flow_swt_init

int // Insert a new flowid into the SW table
hwa_txpost_flow_lut_insert(hwa_txpost_t *txpost,
	const uint16 *ea, uint8 ifid, uint8 prio, uint16 flowid)
{
	int sada_idx, coll_depth; // collision list depth
	hwa_txpost_flow_t *flow, *prev;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_ASSERT((ea != NULL) && (ifid < HWA_INTERFACES_MAX) &&
	           (prio < HWA_PRIORITIES_MAX) &&
	           (flowid != HWA_TXPOST_FLOWID_NULL) &&
	           (flowid != HWA_TXPOST_FLOWID_INVALID));

	HWA_TRACE(("%s flow insert ea<%04x%04x%04x> ifid<%u> prio<%u> flow<%u>\n",
		HWA3a, ea[0], ea[1], ea[2], ifid, prio, flowid));

	if ((sada_idx = hwa_txpost_sada_swt_insert(txpost, ea)) == HWA_FAILURE)
		goto failure;

	HWA_ASSERT(sada_idx < HWA_TXPOST_SADA_LUT_DEPTH);

	// Fetch the head of collision list
	flow = &txpost->flow_swt.table[sada_idx];
	prev = (hwa_txpost_flow_t*)NULL;

	if (flow->u32 == 0U) { // head of collision list is free
		HWA_ASSERT(flow->next == NULL);
		txpost->flow_collsz[sada_idx] = 1;
		goto configure_lut1;
	}

	coll_depth = 0;
	do { // traverse collision list and look for match - exclude flowid
		if (((flow->prio ^ prio) | (flow->ifid ^ ifid)) == 0) {
			HWA_TRACE(("%s flow lut overwite 0x%04x\n",
				HWA3a, flow->flowid));
			goto configure_lut2; // found a match, overwrite flowid
		}
		if (++coll_depth >= HWA_TXPOST_FLOW_LUT_COLL_DEPTH) {
			HWA_WARN(("%s flow lut collision list too long\n", HWA3a));
			goto failure;
		}
		prev = flow; // previous flow in sll
		flow = flow->next;
	} while (flow);

	// could not find a matching flow, so lets add a new one

	if (txpost->flow_flist) {
		flow = txpost->flow_flist;
		txpost->flow_flist = flow->next;
		prev->next = flow;
		flow->next = NULL;
		txpost->flow_collsz[sada_idx]++;
	} else {
		HWA_WARN(("%s flow lut free list depletion\n", HWA3a));
		goto failure;
	}

configure_lut1:
	flow->prio = prio;
	flow->ifid = ifid;

configure_lut2:
	flow->flowid = flowid;

	// Configure the SADA LUT and Flow LUT in HWA AXI
	hwa_txpost_flow_lut_config(txpost, prev, flow, sada_idx);

	HWA_STATS_EXPR(txpost->flow_ins_cnt++);
	return HWA_SUCCESS;

failure:

	HWA_STATS_EXPR(txpost->flow_err_cnt++);
	return HWA_FAILURE;

} // hwa_txpost_flow_lut_insert

int // Delete an active flow in the LUT given an ea, ifid, prio and flowid
hwa_txpost_flow_lut_delete(hwa_txpost_t *txpost,
	const uint16 *ea, uint8 ifid, uint8 prio, uint16 flowid)
{
	int sada_idx, flow_idx;
	hwa_txpost_flow_t *flow, *prev;
	hwa_txpost_flow_t *flow_swt_table;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_ASSERT((ea != NULL) && (ifid < HWA_INTERFACES_MAX) &&
	           (prio < HWA_PRIORITIES_MAX) &&
	           (flowid != HWA_TXPOST_FLOWID_NULL) &&
	           (flowid != HWA_TXPOST_FLOWID_INVALID));

	HWA_TRACE(("%s flow delete ea<%04x%04x%04x> ifid<%u> prio<%u> flow<%u>\n",
		HWA3a, ea[0], ea[1], ea[2], ifid, prio, flowid));

	sada_idx = hwa_txpost_sada_swt_find(txpost, ea);
	if (sada_idx == HWA_FAILURE) {
		HWA_WARN(("%s sada not found\n", HWA3a));
		goto failure;
	}

	HWA_ASSERT(sada_idx < HWA_TXPOST_SADA_LUT_DEPTH);

	flow_swt_table = txpost->flow_swt.table;

	// Fetch the head of collision list
	flow = &txpost->flow_swt.table[sada_idx];
	prev = (hwa_txpost_flow_t*)NULL;

	do { // find the flow by traversing the collision list
		if (((flow->prio ^ prio) | (flow->ifid ^ ifid) |
		     (flow->flowid ^ flowid)) == 0)
		{
			goto found;
		}
		prev = flow;
		flow = flow->next;
	} while (flow);

	HWA_WARN(("%s flow not found\n", HWA3a));

failure:

	HWA_STATS_EXPR(txpost->flow_err_cnt++);

	HWA_ASSERT(0); // Cannot find flow in LUT for deletion. SW bug

	return HWA_FAILURE;

found:

	flow_idx = HWA_TABLE_INDX(hwa_txpost_flow_t, flow_swt_table, flow);

	if (prev)
		prev->next = flow->next;
	flow->u32 = 0; // zero out all fields

	if (flow_idx >= HWA_TXPOST_SADA_LUT_DEPTH) {
		flow->next = txpost->flow_flist;
		txpost->flow_flist = flow;
	}

	txpost->flow_collsz[sada_idx]--;

	HWA_STATS_EXPR(txpost->flow_del_cnt++);

	// Configure the SADA LUT and Flow LUT in HWA AXI
	hwa_txpost_flow_lut_config(txpost, prev, flow, sada_idx);

	return HWA_TABLE_INDX(hwa_txpost_flow_t, flow_swt_table, flow);

} // hwa_txpost_flow_lut_delete

void // Delete all flows associated with a SADA
hwa_txpost_flow_lut_reset(hwa_txpost_t *txpost, const uint16 *ea)
{
	int sada_idx, flow_idx;
	hwa_txpost_flow_t *flow, *next;
	hwa_txpost_flow_t *flow_swt_table;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_ASSERT(ea != NULL);

	HWA_TRACE(("%s flow reset ea<%04x%04x%04x>\n",
		HWA3a, ea[0], ea[1], ea[2]));

	sada_idx = hwa_txpost_sada_swt_find(txpost, ea);
	if (sada_idx == HWA_FAILURE) {
		HWA_WARN(("%s sada not found\n", HWA3a));
		return;
	}

	HWA_ASSERT(sada_idx < HWA_TXPOST_SADA_LUT_DEPTH);

	flow_swt_table = txpost->flow_swt.table;

	// Fetch the head of collision list
	flow = &txpost->flow_swt.table[sada_idx];

	do {
		flow_idx = HWA_TABLE_INDX(hwa_txpost_flow_t, flow_swt_table, flow);
		next = flow->next;
		if (flow_idx >= HWA_TXPOST_SADA_LUT_DEPTH) {
			flow->next = txpost->flow_flist;
			txpost->flow_flist = flow;
		}
		flow->u32 = 0;

		hwa_txpost_flow_lut_config(txpost, NULL, flow, sada_idx);

		HWA_STATS_EXPR(txpost->flow_del_cnt++);
		flow = next;
	} while (flow);

} // hwa_txpost_flow_lut_reset

void
hwa_txpost_flow_lut_config(hwa_txpost_t *txpost,
	const hwa_txpost_flow_t *prev, const hwa_txpost_flow_t *flow,
	int sada_idx)
{
	uint32 flow_ix;
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;
	hwa_txpost_flow_t *flow_swt_table;
	hwa_txpost_flow_lut_elem_t flow_lut_elem;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);
	HWA_ASSERT(flow != NULL);
	HWA_ASSERT(sada_idx != HWA_FAILURE);

	HWA_TRACE(("%s flow config prev<%p> flow<%u:%u:%04x> sada_idx<%d>\n",
		HWA3a, prev, flow->ifid, flow->prio, flow->flowid, sada_idx));

	sys_mem = &flow_lut_elem.u32[0];

	flow_swt_table = txpost->flow_swt.table;

	// First configure flow
	flow_ix = HWA_TABLE_INDX(const hwa_txpost_flow_t, flow_swt_table, flow);

	if (flow->u32) {
		flow_lut_elem.u32[0] = (
			BCM_SBF(HWA_TXPOST_FLOW_LINK_NULL, HWA_TXPOST_FLOW_LINK) |
			BCM_SBF(flow->flowid, HWA_TXPOST_FLOW_FLOWID) |
			BCM_SBF(flow->prio, HWA_TXPOST_FLOW_PRIO) |
			BCM_SBF(flow->ifid, HWA_TXPOST_FLOW_IFID) |
			BCM_SBF(1, HWA_TXPOST_FLOW_VALID));
	} else {
		flow_lut_elem.u32[0] = 0U;
	}

	axi_mem_addr = HWA_TABLE_ADDR(hwa_txpost_flow_lut_elem_t,
	                                  txpost->flow_addr, flow_ix);

	// Flush flow configuration to HWA AXI memory
	HWA_WR_MEM32(HWA3a, hwa_txpost_flow_lut_elem_t, axi_mem_addr, sys_mem);

	// Next configure prev, unless prev is NULL - flow is head of collision list
	if (prev) {
		uint32 prev_ix;

		prev_ix = HWA_TABLE_INDX(const hwa_txpost_flow_t, flow_swt_table, prev);

		if (prev->u32) {
			flow_lut_elem.u32[0] = (
				BCM_SBF(flow_ix, HWA_TXPOST_FLOW_LINK) |
				BCM_SBF(prev->flowid, HWA_TXPOST_FLOW_FLOWID) |
				BCM_SBF(prev->prio, HWA_TXPOST_FLOW_PRIO) |
				BCM_SBF(prev->ifid, HWA_TXPOST_FLOW_IFID) |
				BCM_SBF(1, HWA_TXPOST_FLOW_VALID));
		} else {
			flow_lut_elem.u32[0] = 0U; // tag as inactive
		}

		axi_mem_addr = HWA_TABLE_ADDR(hwa_txpost_flow_lut_elem_t,
		                                  txpost->flow_addr, prev_ix);

		// Flush prev flow configuration to HWA AXI memory
		HWA_WR_MEM32(HWA3a, hwa_txpost_flow_lut_elem_t, axi_mem_addr, sys_mem);

	} else {
		// Flow is head of collision list, so configure sada
		if ((sada_idx) && (flow_ix < HWA_TXPOST_SADA_LUT_DEPTH))
			hwa_txpost_sada_lut_config(txpost, sada_idx, TRUE);
	}

} // hwa_txpost_flow_lut_config
#endif /* !HWA_NO_LUT */

/*
 * -----------------------------------------------------------------------------
 * Section: Support for HWA3a block common and FRC statistics registers
 * -----------------------------------------------------------------------------
 */

static void _hwa_txpost_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 arg);
static void _hwa_txpost_stats_frc_srs_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b,
	uint8 set_idx);
static void _hwa_txpost_stats_frc_srs_cb(hwa_dev_t *dev, uintptr buf, uint32 set_idx);

void
hwa_txpost_stats_clear(hwa_txpost_t *txpost, uint32 set_idx)
{
	hwa_dev_t *dev;

	dev = HWA_DEV(txpost);

	if (set_idx == ~0U) {
		hwa_stats_clear(dev, HWA_STATS_TXPOST_COMMON); // common
		for (set_idx = 0; set_idx < HWA_TXPOST_FRC_SRS_MAX; set_idx++)
			hwa_stats_clear(dev, HWA_STATS_TXPOST_RING + set_idx);
	} else {
		hwa_stats_clear(dev, HWA_STATS_TXPOST_COMMON);
		hwa_stats_clear(dev, HWA_STATS_TXPOST_RING + set_idx);
	}

} // hwa_txpost_stats_clear

void // Print the HWA3a block common statistics
_hwa_txpost_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 arg)
{
	hwa_txpost_t *txpost = &dev->txpost;
	struct bcmstrbuf *b = (struct bcmstrbuf *)buf;

	HWA_BPRINT(b, "%s statistics stalls[sw<%u> hw<%u>] duration[bm<%u> dma<%u>]\n",
		HWA3a, txpost->stats.num_stalls_sw,
		txpost->stats.num_stalls_hw,
		txpost->stats.dur_bm_empty,
		txpost->stats.dur_dma_busy);

} // _hwa_txpost_stats_dump

static void // Print out a FlowRing Statistics Register Set
_hwa_txpost_stats_frc_srs_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b, uint8 set_idx)
{
	hwa_txpost_frc_srs_t *frc_srs = &txpost->frc_srs[set_idx];

	HWA_BPRINT(b, "+ %2u. sched<%u> pktchain<%u,%u> audits<%u>"
		" oct64<%u,%u> rdupd<%u> pkts<%u,%u>\n", set_idx,
		frc_srs->schedcmds, frc_srs->swpkts, frc_srs->max_pktch_size,
		frc_srs->max_pktch_size, frc_srs->octets_ls32, frc_srs->octets_ms32,
		frc_srs->rd_idx_updates, frc_srs->pkt_allocs, frc_srs->pkt_deallocs);

} // _hwa_txpost_stats_frc_srs_dump

void // Dump all|a FRC statistics register set
_hwa_txpost_stats_frc_srs_cb(hwa_dev_t *dev, uintptr buf, uint32 set_idx)
{
	struct bcmstrbuf *b = (struct bcmstrbuf *)buf;

	HWA_BPRINT(b, "%s statistics frc\n", HWA3a);

	if (set_idx == ~0U) {
		for (set_idx = 0; set_idx < HWA_TXPOST_FRC_SRS_MAX; set_idx++)
			_hwa_txpost_stats_frc_srs_dump(&dev->txpost, b, (uint8)set_idx);
	} else {
		_hwa_txpost_stats_frc_srs_dump(&dev->txpost, b, (uint8)set_idx);
	}

} // _hwa_txpost_stats_frc_srs_cb

void // Query and dump all|a FRC statistics Register Set, given set idx [0..16]
hwa_txpost_stats_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b,
	uint32 set_idx, uint8 clear_on_copy)
{
	hwa_dev_t *dev;
	uint32 hiaddr = 0x00000000;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	// Audit pre-conditions
	dev = HWA_DEV(txpost);

	hwa_stats_copy(dev, HWA_STATS_TXPOST_COMMON,
		HWA_PTR2UINT(txpost->stats.u32), hiaddr, /* num_sets */ 1,
		clear_on_copy, &_hwa_txpost_stats_dump,
		(uintptr)b, HWA_STATS_TXPOST_COMMON);

	if (set_idx == ~0U) {
		hwa_stats_copy(dev, HWA_STATS_TXPOST_RING,
			HWA_PTR2UINT(txpost->frc_srs), hiaddr, HWA_TXPOST_FRC_SRS_MAX,
			clear_on_copy, &_hwa_txpost_stats_frc_srs_cb,
			(uintptr)b, set_idx);
	} else {
		void *ptr = &txpost->frc_srs[set_idx];

		HWA_ASSERT(set_idx < HWA_TXPOST_FRC_SRS_MAX);
		hwa_stats_copy(dev, HWA_STATS_TXPOST_RING + set_idx,
			HWA_PTR2UINT(ptr), hiaddr, /* num_sets */ 1,
			clear_on_copy, &_hwa_txpost_stats_frc_srs_cb,
			(uintptr)b, set_idx);
	}

} // hwa_txpost_stats_dump

#if HWA_REVISION_GE_129
static INLINE int // Handle a Free TxBuffer request from WLAN driver, returns success|failure
_hwa_txpost_txbuffer_free(hwa_txpost_t *txpost, void *buf)
{
	hwa_dev_t *dev;
	hwa_ring_t *txfree_ring; // S2H FREEIDXTX interface
	hwa_txpost_txfree_t *txfree;
	HWA_DEBUG_EXPR(uint32 offset);

	HWA_TRACE(("%s free txbuffer<%p>\n", HWA3a, buf));

	// Audit parameters and pre-conditions
	dev = HWA_DEV(txpost);
	HWA_ASSERT(buf != (void*)NULL);

	// Audit tx_buffer.
	HWA_ASSERT(buf >= dev->tx_bm.memory);
	HWA_ASSERT(((char*)buf) <= (((char *)dev->tx_bm.memory) +
		((dev->tx_bm.pkt_total - 1) * dev->tx_bm.pkt_size)));
	HWA_DEBUG_EXPR({
		offset = ((char *)buf) - ((char *)dev->tx_bm.memory);
		HWA_ASSERT((offset % dev->tx_bm.pkt_size) == 0);
	});

	// Setup locals
	txfree_ring = &txpost->txfree_ring;

	if (hwa_ring_is_full(txfree_ring))
		goto failure;

	// Find the location where txfree needs to be constructed, and populate it
	txfree = HWA_RING_PROD_ELEM(hwa_txpost_txfree_t, txfree_ring);

	// Convert rxbuffer pointer to its index within Tx Buffer Manager
	txfree->index = hwa_bm_ptr2idx(&dev->tx_bm, buf);

	hwa_ring_prod_upd(txfree_ring, 1, TRUE); // update/commit WR

	HWA_STATS_EXPR(txpost->txfree_cnt++);

	HWA_TRACE(("%s TXF[%u,%u][buf<%p> index<%u>]\n", HWA3a,
		HWA_RING_STATE(txfree_ring)->write, HWA_RING_STATE(txfree_ring)->read,
		buf, txfree->index));

	dev->tx_bm.avail_sw++;

	return HWA_SUCCESS;

failure:
	HWA_WARN(("%s txbuffer free <0x%p> failure\n", HWA3a, buf));

	return HWA_FAILURE;

} // hwa_txpost_txbuffer_free
#endif /* HWA_REVISION_GE_129 */

void * // Handle SW allocate tx buffer from TxBM. Return txpost buffer || NULL
hwa_txpost_txbuffer_get(struct hwa_dev *dev)
{
	dma64addr_t buf_addr;
	void *lfrag;
	hwa_txpost_pkt_t *txbuf;

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	if (hwa_bm_alloc(&dev->tx_bm, &buf_addr) == HWA_FAILURE)
		return NULL;

	txbuf = HWA_UINT2PTR(hwa_txfifo_pkt_t, buf_addr.loaddr);

	/* lfrag start after HWA_TXPOST_PKT_BYTES */
	lfrag = HWAPKT2LFRAG((char *)txbuf);

	/* Terminate next */
	PKTSETCLINK(lfrag, NULL);
	HWA_PKT_TERM_NEXT(txbuf);

	/* Mark packet as TXFRAG and HWAPKT */
	PKTSETTXFRAG(dev->osh, lfrag);
	PKTSETHWAPKT(dev->osh, lfrag);
	PKTRESETHWA3BPKT(dev->osh, lfrag);

	/* Clear data point, caller need to set it. */
	PKTSETBUF(dev->osh, lfrag, NULL, 0);

	return txbuf;
}

int // Handle a Free TxBuffer request from WLAN driver, returns success|failure
hwa_txpost_txbuffer_free(struct hwa_dev *dev, void *buf)
{
	int ret = HWA_SUCCESS;

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

#if HWA_REVISION_GE_129
	if (dev->tx_pktdealloc) {
		ret = _hwa_txpost_txbuffer_free(&dev->txpost, buf);
	}
	else
#endif // endif
	{
		ret = hwa_bm_free(&dev->tx_bm, hwa_bm_ptr2idx(&dev->tx_bm, buf));
	}

	return ret;
}

/*
 * -----------------------------------------------------------------------------
 * Section: Support for debug - SW state and HWA3a registers
 * -----------------------------------------------------------------------------
 */

#if defined(BCMDBG)

void // HWA3a FRC Table dump
hwa_txpost_frc_table_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b, bool verbose)
{
	int frc_id;
	hwa_txpost_frc_t *frc;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	frc = txpost->frc_table;

	HWA_BPRINT(b, "%s frc table<%p>\n", HWA3a, frc);
	for (frc_id = 0; frc_id < HWA_TXPOST_FLOWRINGS_MAX; frc_id++, frc++) {

		if (frc->ifid == HWA_IFID_INVALID)
			continue;

		HWA_BPRINT(b, "+ %3u. frc<%p> haddr<0x%08x,0x%08x> wr_idx<%u> ring_sz<%u>"
			" srs<%u> flow<%u:0x%3x> ifid<%u>\n", frc_id, frc,
			frc->haddr64.loaddr, frc->haddr64.hiaddr,
			frc->wr_idx_offset, frc->ring_size,
			frc->srs_idx, frc->flowid, frc->flowid,
			frc->ifid);
		HWA_BPRINT(b, "       aggr_depth<%u> aggr_mode<%u> avg_pkt_size<%u>"
			" lkup_type<%u>\n", frc->aggr_depth, frc->aggr_mode,
			frc->avg_pkt_size, frc->lkup_type);
		HWA_BPRINT(b, "       pyld_min_length<%u> etype_ip_enable<%u> epoch<%u>"
			" audit_en<%u>\n", frc->pyld_min_length, frc->etype_ip_enable,
			frc->epoch, frc->audit_enable);
	}

} // hwa_txpost_frc_table_dump

#if !defined(HWA_NO_LUT)
void // HWA3a Priority LUT dump
hwa_txpost_prio_lut_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b, bool verbose)
{
	uint8 ifid, prio, flowid;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	for (ifid = 0; ifid < HWA_INTERFACES_MAX; ifid++) {
		if (isset(txpost->prio_cfg, ifid)) {
			HWA_BPRINT(b, "%s prio_lut ifid<%2u>\n", HWA3a, ifid);
			if (verbose) {
				for (prio = 0; prio < HWA_PRIORITIES_MAX; prio++) {
					flowid =
					    txpost->prio_lut.if_table[ifid].flowid_table[prio] &
						HWA_TXPOST_FLOWID_MASK;
					HWA_BPRINT(b, "+ prio<%u> flowid<0x%03x, %4u>\n",
						prio, flowid, flowid);
				} // for prio
			} // if verbose
		} // if isset
	} // for ifid

} // hwa_txpost_prio_lut_dump

void // HWA3a Unique SADA LUT dump
hwa_txpost_sada_lut_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b, bool verbose)
{
	const uint16 *ea;
	hwa_txpost_sada_t *sada;
	uint32 acnt = 0U, fcnt = 0U;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	HWA_BPRINT(b, "%s sada_lut:\n", HWA3a);

	sada = txpost->sada_alist;
	while (sada != NULL) {
		ea = sada->elem.u16;
		acnt++;
		if (verbose)
			HWA_BPRINT(b, "+ ea<%04x%04x%04x>\n", ea[0], ea[1], ea[2]);
		sada = sada->next;
	}
	sada = txpost->sada_flist;
	while (sada != NULL) { fcnt++; sada = sada->next; }

	HWA_BPRINT(b, "+ active<%u> free<%u>\n", acnt, fcnt);
	HWA_STATS_EXPR(
		HWA_BPRINT(b, "+ ins<%u> dels<%u> err<%u>\n", txpost->sada_ins_cnt,
			txpost->sada_del_cnt, txpost->sada_err_cnt));

} // hwa_txpost_sada_lut_dump

void // HWA3a Flow LUT dump
hwa_txpost_flow_lut_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b, bool verbose)
{
	int sada_idx;
	uint32 acnt = 0U, fcnt = 0U, coll_sz;
	hwa_txpost_flow_t *flow;

	HWA_ASSERT(txpost != (hwa_txpost_t*)NULL);

	for (sada_idx = 0; sada_idx < HWA_TXPOST_SADA_LUT_DEPTH; sada_idx++) {
		flow = &txpost->flow_swt.table[sada_idx];
		if (flow->u32)
			HWA_BPRINT(b, "+ FLOW<%u> coll_sz<%u>: ",
				sada_idx, txpost->flow_collsz[sada_idx]);
		else {
			fcnt++;
			continue;
		}
		coll_sz = 0U;
		while (flow != (hwa_txpost_flow_t*)NULL) {
			acnt++;
			coll_sz++;
			HWA_BPRINT(b, "[0x%04x, %u, %u] ",
				flow->flowid, flow->ifid, flow->prio);
			flow = flow->next;
		}
		HWA_BPRINT(b, " %u\n", coll_sz);
		HWA_ASSERT(txpost->flow_collsz[sada_idx] == coll_sz);
	}

	flow = txpost->flow_flist;
	while (flow != NULL) { fcnt++; flow = flow->next; }

	HWA_BPRINT(b, "+ active<%u> free<%u>\n", acnt, fcnt);
	HWA_STATS_EXPR(
		HWA_BPRINT(b, "+ ins<%u> del<%u> err<%u>\n", txpost->flow_ins_cnt,
			txpost->flow_del_cnt, txpost->flow_err_cnt));

} // hwa_txpost_flow_lut_dump
#endif /* !HWA_NO_LUT */

void // HWA3a TxPOST: debug dump
hwa_txpost_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b, bool verbose, bool dump_regs)
{
	HWA_BPRINT(b, "%s dump<%p>\n", HWA3a, txpost);

	if (txpost == (hwa_txpost_t*)NULL)
		return;

	hwa_ring_dump(&txpost->schedcmd_ring, b, "+ schedcmd");
	hwa_ring_dump(&txpost->pktchain_ring, b, "+ pktchain");
#if HWA_REVISION_GE_129
	hwa_ring_dump(&txpost->txfree_ring, b, "+ txfree_ring");
#endif // endif
	HWA_BPRINT(b, "+ TransId: schedcmd<%u> pktchain<%u>\n",
		txpost->schedcmd_id, txpost->pktchain_id);

	HWA_STATS_EXPR(
		HWA_BPRINT(b, "+ pkt_post<%u> oct_proc<%u>\n",
		          txpost->pkt_proc_cnt, txpost->oct_proc_cnt));
#if HWA_REVISION_GE_129
	if (txpost->txfree_ring.memory != NULL) {
		HWA_STATS_EXPR(HWA_BPRINT(b, "+ txfree_cnt<%u>\n", txpost->txfree_cnt));
	}
#endif // endif

	if (verbose == TRUE) {
		hwa_txpost_stats_dump(txpost, b, /* all sets */ ~0U, /* clear */ 0);
	}

#if defined(BCMDBG)
	hwa_txpost_frc_table_dump(txpost, b, verbose);
#if !defined(HWA_NO_LUT)
	hwa_txpost_prio_lut_dump(txpost, b, verbose);
	hwa_txpost_sada_lut_dump(txpost, b, verbose);
	hwa_txpost_flow_lut_dump(txpost, b, verbose);
#endif /* !HWA_NO_LUT */
#endif /* BCMDBG */

#if defined(WLTEST)
	if (dump_regs == TRUE)
		hwa_txpost_regs_dump(txpost, b);
#endif // endif

} // hwa_txpost_dump

#if defined(WLTEST)

void // Dump HWA3a block registers
hwa_txpost_regs_dump(hwa_txpost_t *txpost, struct bcmstrbuf *b)
{
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	HWA_DEBUG_EXPR(int i);

	if (txpost == (hwa_txpost_t*)NULL)
		return;

	// Audit pre-conditions
	dev = HWA_DEV(txpost);

	// Setup locals
	regs = dev->regs;

	HWA_BPRINT(b, "%s registers[%p] offset[0x%04x]\n",
		HWA3a, &regs->tx, OFFSETOF(hwa_regs_t, tx));

	HWA_BPR_REG(b, tx, txpost_config);
	HWA_BPR_REG(b, tx, txpost_wi_ctrl);
	HWA_BPR_REG(b, tx, txpost_ethr_type);
	HWA_BPR_REG(b, tx, fw_cmdq_base_addr);
	HWA_BPR_REG(b, tx, fw_cmdq_wr_idx);
	HWA_BPR_REG(b, tx, fw_cmdq_rd_idx);
	HWA_BPR_REG(b, tx, fw_cmdq_ctrl);
	HWA_BPR_REG(b, tx, pkt_chq_base_addr);
	HWA_BPR_REG(b, tx, pkt_chq_wr_idx);
	HWA_BPR_REG(b, tx, pkt_chq_rd_idx);
	HWA_BPR_REG(b, tx, pkt_chq_ctrl);
	HWA_BPR_REG(b, tx, txpost_frc_base_addr);

#if HWA_REVISION_GE_129
	HWA_BPR_REG(b, tx, pktdealloc_ring_addr);
	HWA_BPR_REG(b, tx, pktdealloc_ring_wrindex);
	HWA_BPR_REG(b, tx, pktdealloc_ring_rdindex);
	HWA_BPR_REG(b, tx, pktdealloc_ring_depth);
	HWA_BPR_REG(b, tx, pktdealloc_ring_lazyintrconfig);
#endif /* HWA_REVISION_GE_129 */

	HWA_DEBUG_EXPR({
		HWA_BPR_REG(b, tx, pkt_ch_valid);
		for (i = 0; i < 4; i++) {
			HWA_BPR_REG(b, tx, pkt_ch_flowid_reg[i]);
		}});

	HWA_BPR_REG(b, tx, dma_desc_template);
	HWA_BPR_REG(b, tx, h2d_wr_ind_array_base_addr);
	HWA_BPR_REG(b, tx, h2d_rd_ind_array_base_addr);
	HWA_BPR_REG(b, tx, h2d_rd_ind_array_host_base_addr_lo);
	HWA_BPR_REG(b, tx, h2d_rd_ind_array_host_base_addr_hi);
	HWA_BPR_REG(b, tx, txplavg_weights_reg);
	HWA_BPR_REG(b, tx, txp_host_rdidx_update_reg);
	{
		hwa_txpost_status_reg_t reg;
		reg.u32 = HWA_BPR_REG(b, tx, txpost_status_reg);
		HWA_BPRINT(b, "+ audit[et<%u> pl<%u> pa<%u> ma<%u> if<%u> pb<%u> mt<%u>]"
			" lut[sa<%u> fl<%u>] full<%u> find[%u, %u]\n",
			reg.auditfail_ethtype, reg.auditfail_pyldlen,
			reg.auditfail_pyldaddr, reg.auditfail_md_addr,
			reg.auditfail_ifid, reg.auditfail_phasebit,
			reg.msgtype_unknown, reg.sada_lkup_miss, reg.flow_lkup_miss,
			reg.flow_lut_full, reg.flow_find_valid, reg.flow_find_loc);
	}
	{
		hwa_txpost_status_reg2_t reg;
		reg.u32 = HWA_BPR_REG(b, tx, txpost_status_reg2);
		HWA_BPRINT(b, "+ FSM[cmd<%u> dma<%u> pkt<%u>]"
			" xfer<%u> evallch<%u> chnint<%u> clk<%u> idle<%u> evlong<%u>"
			" cmdq_rd_intr<%u>"
#if HWA_REVISION_GE_129
			" pkt_dealloc_rd_intr<%u>"
#endif /* HWA_REVISION_GE_129 */
			"\n",
			reg.cmd_frc_dma_fsm, reg.dma_fsm, reg.pkt_proc_fsm,
			reg.pkt_xfer_done, reg.evict_all_chains, reg.pktchn_interrupt,
			reg.clk_req, reg.txdata_idle, reg.evict_longest_chain,
#if HWA_REVISION_GE_129
			reg.cmdq_rd_intr, reg.pkt_dealloc_rd_intr);
#else
			reg.cmdq_rd_intr);
#endif /* HWA_REVISION_GE_129 */
	}

	HWA_BPR_REG(b, tx, txpost_cfg1);

#if HWA_REVISION_GE_129
	HWA_BPR_REG(b, tx, pktdeallocmgr_tfrstatus);
	HWA_BPR_REG(b, tx, pktdealloc_localfifo_cfg_status);
	HWA_BPR_REG(b, tx, txpost_aggr_config);
	HWA_BPR_REG(b, tx, txpost_aggr_wi_ctrl);
	{
		hwa_txpost_debug_reg_t reg;
		reg.u32 = HWA_PR_REG(tx, txpost_debug_reg);
		HWA_BPRINT(b, "+ cmd uflow<%u> oflow<%u> zero_wi<%u> mis mem0<%u> mem1<%u>"
			"Qin 0_wi<%u> 1k_wi<%u> zero pktid0<%u> pktid1<%u> order_err<%u>"
			"transid oflow<%u> uflow<%u> cmd_cnt<%u>\n",
			reg.fw_cmd_uflow, reg.fw_cmd_oflow, reg.fw_zero_wi,
			reg.fw_cmd_mem_0_mis, reg.fw_cmd_mem_1_mis,
			reg.fw_Q_in_zero_wi, reg.fw_Q_in_1k_wi,
			reg.pktid0_zero, reg.pktid1_zero, reg.pktid_order_err,
			reg.transid_oflow, reg.transid_uflow, reg.fw_cmd_cnt);
	}
#endif /* HWA_REVISION_GE_129 */

} // hwa_txpost_regs_dump

#endif // endif

#endif /* BCMDBG */

void
hwa_txpost_dump_pkt(hwa_txfifo_pkt_t *pkt, struct bcmstrbuf *b,
	const char *title, uint32 pkt_index, bool one_shot)
{
#if (HWA_DEBUG_BUILD >= 3) || defined(BCMDBG) || 0
	uint32 index;
	hwa_txpost_pkt_t *curr;
	char eabuf_da[ETHER_ADDR_STR_LEN];
	char eabuf_sa[ETHER_ADDR_STR_LEN];

	HWA_ASSERT(pkt);

	index = pkt_index;
	curr = pkt;
	bcm_ether_ntoa((struct ether_addr *)curr->eth_sada.u8, eabuf_da);
	bcm_ether_ntoa((struct ether_addr *)&curr->eth_sada.u8[ETHER_ADDR_LEN],
		eabuf_sa);

	while (curr) {
		index++;
		HWA_PRINT(" [%s] 3a:swpkt-%d at <%p> lbuf <%p>\n", title ? title : "",
			index, curr, HWAPKT2LFRAG((char *)curr));
		HWA_PRINT("    <%p>:\n", curr);
		HWA_PRINT("           next <%p>\n", curr->next);
		HWA_PRINT("           daddr <0x%x>\n", curr->hdr_buf_daddr32);
		HWA_PRINT("           pktid <0x%x,%u>\n", curr->host_pktid, curr->host_pktid);
		HWA_PRINT("           ifid <%u> prio <%u> C <%u> flags <0x%x> hlen <%u>\n",
			curr->ifid,  curr->prio, curr->copy, curr->flags, curr->data_buf_hlen);
		HWA_PRINT("           haddr<0x%08x,0x%08x>\n", curr->data_buf_haddr.loaddr,
			curr->data_buf_haddr.hiaddr);
		HWA_PRINT("           dasa <%s,%s>\n", eabuf_da, eabuf_sa);
		HWA_PRINT("           etype <0x%04x> info <0x%x> flowidovd <%u>\n",
			HTON16(curr->eth_type), curr->info, curr->flowid_override);
		HWA_PRINT("           rdidx <%u> pktflags <0x%x>\n", curr->rd_index,
			curr->audit_flags);

		if (one_shot)
			break;
		curr = curr->next;
	}
#endif // endif
}

/* Forward packets to WL..
 * Go through HW provided packet chain and initial lbuf.
 */
static int
hwa_txpost_sendup(void *context, uintptr arg1, uintptr arg2, uint32 pkt_count, uint32 total_octets)
{
	hwa_dev_t *dev = (hwa_dev_t *)context;
	hwa_txpost_pkt_t *head = (hwa_txpost_pkt_t *)arg1;
	hwa_txpost_pkt_t *tail = (hwa_txpost_pkt_t *)arg2;
	void *pktc;
	uint32 flowid;
	bool drop_pkt;
	void *pktcs[HWA_PKTC_TYPE_MAX];
	uint32 pktc_cnts[HWA_PKTC_TYPE_MAX];
	uint32 type;

	/* Storage space to move data across hwa-cpf-pciedev */
	hwa_cfp_tx_info_t hwa_cfp_tx_info = {0};

	HWA_FTRACE(HWA3a);
	HWA_ASSERT(context != (void *)NULL);
	HWA_ASSERT(arg1 != 0);
	HWA_ASSERT(arg2 != 0);

	// Setup locals
	flowid = head->flowid_override;
	drop_pkt = FALSE;

	/* Drop the packet chain if
	 * 1. HWA device is down.
	 * 2. pciedev_flowring_drop_pkts() return TRUE.
	 */
	if (!dev->up || pciedev_flowring_drop_pkts(dev->pciedev, flowid)) {
		drop_pkt = TRUE;
	}

#ifdef WLCFP
	/* Check if CFP Enabled for the given packet list
	 * BUS layer to fillup hwa_cfp_tx_info if CFP enabled
	 */
	hwa_cfp_tx_info.pktlist_count = pkt_count;
	pciedev_hwa_cfp_tx_enabled(dev->pciedev, head, &hwa_cfp_tx_info);
#endif /* WLCFP */

	/* Translate hwa_txpost_pkt_t 3a-SWPKT (44 Bytes) to lfrag */
	pciedev_hwa_txpost_pkt2native(dev->pciedev, head, tail, pkt_count, total_octets,
		&hwa_cfp_tx_info, pktcs, pktc_cnts);

	for (type = HWA_PKTC_TYPE_UNICAST; type < HWA_PKTC_TYPE_MAX; type++) {
		if (!pktc_cnts[type])
			continue;

		pktc = pktcs[type];
		HWA_ASSERT(pktc);

		// Send to bus layer
		if (drop_pkt || (type == HWA_PKTC_TYPE_FLOW_MISMATCH)) {
			void *p, *n;
#ifdef WLSQS
			/* Stale flowring pkts. Free and adjust v2r count */
			sqs_v2r_sendup_slow(dev->pciedev, flowid,
			    hwa_cfp_tx_info.pktlist_prio, pktc_cnts[type]);
#endif // endif
			p = pktc;
			FOREACH_CHAINED_PKT(p, n) {
				PKTCLRCHAINED(dev->osh, p);
				PKTFREE(dev->osh, p, FALSE);
			}

			/* Check if driver need to send pending_flring_resp */
			pciedev_hwa_process_pending_flring_resp(dev->pciedev, flowid);
		} else {
			/* Forward the tx packets to the wireless subsystem */
#ifdef WLCFP
			if (hwa_cfp_tx_info.cfp_capable &&
				type == HWA_PKTC_TYPE_UNICAST) {
				/* Update pkt_count, 3a pktc may break by bcmc/null DA */
				hwa_cfp_tx_info.pktlist_count = pktc_cnts[type];

				/* CFP Path Dongle Sendup */
				wlc_cfp_tx_sendup(0, hwa_cfp_tx_info.cfp_flowid,
					hwa_cfp_tx_info.pktlist_prio,
					hwa_cfp_tx_info.pktlist_head,
					hwa_cfp_tx_info.pktlist_tail,
					hwa_cfp_tx_info.pktlist_count);
			} else
#endif /* WLCFP */
			{
				/* Legacy path Dongle Sendup */
#ifdef WLSQS
				sqs_v2r_sendup_slow(dev->pciedev, flowid,
					hwa_cfp_tx_info.pktlist_prio, pktc_cnts[type]);
#endif // endif
				if (type != HWA_PKTC_TYPE_BCMC) {
					dngl_sendup(dev->dngl, pktc);
				} else {
					void *p, *n;

					p = pktc;
					FOREACH_CHAINED_PKT(p, n) {
						PKTCLRCHAINED(dev->osh, p);
						dngl_sendup(dev->dngl, p);
					}
				}
			}
		}
	}

	return HWA_SUCCESS;
}

void
hwa_txpost_schecmd_dens(struct hwa_dev *dev, struct bcmstrbuf *b, bool clear)
{
	HWA_BCMDBG_EXPR({
		uint32 i;
		uint32 count = 0;
		uint32 total = 0;
		hwa_txpost_t *txpost;

		HWA_FTRACE(HWA3a);

		HWA_ASSERT(dev != (struct hwa_dev *)NULL);

		txpost = &dev->txpost;

		if (clear) {
			bzero(txpost->schecmd_histogram,
				sizeof(txpost->schecmd_histogram));
			return;
		}

		for (i = 0; i <= HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT; i++)
			total += txpost->schecmd_histogram[i];

		bcm_bprintf(b, "ScheCmdDens:");
		for (i = 0; i <= HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT; i++) {
			if (txpost->schecmd_histogram[i] == 0)
				continue;
			count++;
			bcm_bprintf(b, " [%3d]%3d (%d%%)", i+1,
				txpost->schecmd_histogram[i], (total == 0) ?
				0 : (txpost->schecmd_histogram[i] * 100 / total));
			if ((count % 8) == 0)
				bcm_bprintf(b, "\n           :");
		}
		bcm_bprintf(b, "\n");
	});
}

#endif /* HWA_TXPOST_BUILD */

#ifdef HWA_CPLENG_BUILD
/*
 * -----------------------------------------------------------------------------
 * Section: HWA2b RxCplE and HWA4b TxCplE completion engines
 * -----------------------------------------------------------------------------
 */

#define HWA_CPLE_AWI_NONE       ((void*)(NULL))

// Fetch the pointer to current AWI using the AWI index in the AWI array
#define HWA_CPLE_AWI_CURR(cple) \
	BCM_RING_ELEM((cple)->awi_base, (cple)->awi_entry, (cple)->awi_size)

// Sync or Lazy WI refresh requests
typedef enum hwa_cple_refresh_req
{
	HWA_CPLE_REFRESH_SYNC = 0, // sync refresh due to depleted WI array
	HWA_CPLE_REFRESH_LAZY = 1  // skip refresh if WI are free
} hwa_cple_refresh_req_t;

// User or Lazy WI commit requests
typedef enum hwa_cple_commit_req
{
	HWA_CPLE_COMMIT_SYNC = 0, // sync commit request from user
	HWA_CPLE_COMMIT_LAZY = 1  // lazy commit on WI add
} hwa_cple_commit_req_t;

// Refresh free ACWI by recycling CEDs consumed by HWA
static INLINE void __hwa_cple_refresh(hwa_cple_t *cple);

// Allocate a ACWI with refresh if requested
static INLINE void * __hwa_cple_allocate(hwa_cple_t *cple,
	const hwa_cple_refresh_req_t refresh_req);

// Check if there is space for a WI in a ACWI.
static INLINE void * __hwa_cple_prepare(hwa_cple_t *cple);

// Post a CED to the CEDQ S2H Interface
static INLINE void __hwa_cple_flush(hwa_cple_t *cple);

// Commit a CED in the CEDQ to transfer all pending work items
static INLINE void __hwa_cple_commit(hwa_cple_t *cple,
	const hwa_cple_commit_req_t commit_req);

// Detach a completion engine
static void BCMATTACHFN(_hwa_cple_dettach)(
	hwa_dev_t *dev, const char *name, hwa_cple_t *cple);

// Attach a completion engine
static hwa_cple_t * BCMATTACHFN(_hwa_cple_attach)(
	hwa_dev_t *dev, const char *name,
	hwa_cple_t *cple, hwa_cpleng_cedq_idx_t cedq_idx,
	uint16 cedq_ring_depth, uint32 cedq_ring_id, uint32 cedq_ring_num,
	uint16 awi_ring_depth, uint16 awi_size, uint32 awi_commit,
	uint32 d2h_ring_idx);

static INLINE void
__hwa_cple_refresh(hwa_cple_t *cple)
{
	int elem_ix, awi_recycle_count;
	bcm_ring_t ring;
	hwa_cpleng_ced_t *ced;

	HWA_FTRACE(HWAce);

	// Prepare locals
	awi_recycle_count = 0;
	bcm_ring_init(&ring);

	// Determine how HWAce progressed in processing posted CEDs
	ring.read = cple->cedq_ring.state.read;

	hwa_ring_prod_get(&cple->cedq_ring); // Sync SW state RD from HWA register
	ring.write = cple->cedq_ring.state.read;

	// Now determine how many AWI may be recycled
	while ((elem_ix = bcm_ring_cons(&ring, cple->cedq_ring.depth)) != BCM_RING_EMPTY)
	{
		ced = HWA_RING_ELEM(hwa_cpleng_ced_t, &cple->cedq_ring, elem_ix);

		HWA_TRACE(("%s CEDQ<%u> CED<%p:%u> AWI xfer<0x%08x> count<%u>\n",
			HWAce, cple->cedq_idx, ced, elem_ix,
			ced->wi_array_addr, ced->wi_count));

		awi_recycle_count += ced->wi_count;
	}

	cple->awi_free += awi_recycle_count;
	cple->awi_ring.read = (cple->awi_ring.read + awi_recycle_count) %
	                      cple->awi_depth;

	HWA_STATS_EXPR(cple->awi_sync_count += awi_recycle_count);
	HWA_STATS_EXPR(cple->cedq_upd_count++);

	HWA_TRACE(("%s CEDQ<%u> refresh<%u> free<%u>\n",
		HWAce, cple->cedq_idx, awi_recycle_count, cple->awi_free));

} // __hwa_cple_refresh

static INLINE void * // Allocate a ACWI
__hwa_cple_allocate(hwa_cple_t *cple, const hwa_cple_refresh_req_t refresh_req)
{
	int awi_entry;

	HWA_ASSERT(cple->awi_curr == HWA_CPLE_AWI_NONE);

	// Refresh free AWIs by recyling CEDs consumed by HWA
	if ((cple->awi_free == 0) | (refresh_req == HWA_CPLE_REFRESH_SYNC)) {
		__hwa_cple_refresh(cple);
	}

	// No free aggregate WI after an refresh
	if (cple->awi_free == 0) {
		HWA_TRACE(("%s CEDQ<%u> allocate failure\n", HWAce, cple->cedq_idx));
		HWA_STATS_EXPR(cple->awi_full_count++);
		return HWA_CPLE_AWI_NONE;
	}

	// Allocate a new AWI
	awi_entry = bcm_ring_prod(&cple->awi_ring, cple->awi_depth);
	HWA_ASSERT(awi_entry != BCM_RING_FULL);
	cple->awi_entry = awi_entry;
	cple->awi_curr = HWA_CPLE_AWI_CURR(cple);
	if (cple->awi_xfer == HWA_CPLE_AWI_NONE) {
		cple->awi_xfer = cple->awi_curr;
	}
	cple->awi_free--;

	HWA_ASSERT(cple->awi_entry < cple->awi_depth);
	HWA_ASSERT(cple->awi_free ==
		bcm_ring_prod_avail(&cple->awi_ring, cple->awi_depth));

	// Prepare the new AWI for WI addition
	cple->wi_entry = 0;

	HWA_TRACE(("%s CEDQ<%u> allocate AWI<%p:%u> free<%u>\n", HWAce,
		cple->cedq_idx, cple->awi_curr, cple->awi_entry, cple->awi_free));

	return cple->awi_curr;

} // __hwa_cple_allocate

static INLINE void * // Prepare space in an aggregate work item
__hwa_cple_prepare(hwa_cple_t *cple)
{
	if (cple->awi_curr == HWA_CPLE_AWI_NONE) {
		return __hwa_cple_allocate(cple, HWA_CPLE_REFRESH_SYNC);
	}

	return cple->awi_curr;

} // __hwa_cple_prepare

static INLINE void // Post a CED to the CEDQ S2H Interface
__hwa_cple_flush(hwa_cple_t *cple)
{
	hwa_ring_t *cedq_ring; // paired CEDQ
	hwa_cpleng_ced_t *ced; // current CED in CEDQ

	cedq_ring = &cple->cedq_ring; // paired CEDQ

	// CEDQ and AWI rings are of equal depth
	HWA_ASSERT(hwa_ring_is_full(cedq_ring) == FALSE);

	ced = HWA_RING_PROD_ELEM(hwa_cpleng_ced_t, cedq_ring);

	// Finalize the current CED with all pending AWI to be transferred
	ced->wi_count = cple->awi_pend;
	ced->wi_array_addr = HWA_PTR2UINT(cple->awi_xfer);

	HWA_STATS_EXPR(cple->awi_xfer_count += cple->awi_pend);

	cple->awi_pend = 0;
	cple->wi_added = 0;
	cple->awi_xfer = HWA_CPLE_AWI_NONE;

	HWA_TRACE(("%s CEDQ<%u> CED<%p:%u> AWI xfer<0x%08x> count<%u>\n",
		HWAce, cple->cedq_idx, ced, cedq_ring->state.write,
		ced->wi_array_addr, ced->wi_count));

	// Kickstart HWAce by updating the HWA S2H WR index register
	hwa_ring_prod_upd(cedq_ring, 1, TRUE);

	HWA_STATS_EXPR(cple->cedq_xfer_count++);

} // __hwa_cple_flush

static INLINE void // Check whether CED must be committed
__hwa_cple_commit(hwa_cple_t *cple, const hwa_cple_commit_req_t commit_req)
{
	HWA_TRACE(("%s commit<%u> CEDQ<%u> WI entry<%u> added<%u>"
		" AWI curr<%p:%u> free<%u> xfer<%p> pend<%u>\n",
		HWAce, commit_req, cple->cedq_idx, cple->wi_entry, cple->wi_added,
		cple->awi_curr, cple->awi_entry, cple->awi_free,
		cple->awi_xfer, cple->awi_pend));

	HWA_ASSERT(cple->wi_added != 0);
	HWA_ASSERT(cple->awi_xfer != HWA_CPLE_AWI_NONE);
	HWA_ASSERT(cple->awi_entry < cple->awi_depth);

	// Determine whether a lazy or forced commit is to be performed

	if (commit_req == HWA_CPLE_COMMIT_LAZY) {

		// Update wi_entry and lazy commit on WI add path
		cple->wi_entry++; // new wi added into an aggregate

		if (cple->wi_entry >= cple->aggr_max) { // current AWI is full

			cple->awi_pend++; // increment num AWI that are pending flush

			// Check whether we need to commit the current CED
			if ((cple->awi_pend >= cple->awi_comm) // over commit threshold
			    | (cple->awi_free == 0) // no more free AWIs
			    | ((cple->awi_entry + 1) >= cple->awi_depth)) // wrap around
			{
				cple->awi_curr = HWA_CPLE_AWI_NONE;
				goto commit_ced; // sync refresh
			}

			// Allocate a new AWI and prepare for next WI addition
			HWA_ASSERT(bcm_ring_is_full(&cple->awi_ring, cple->awi_depth)
			           == FALSE);

			cple->awi_entry = bcm_ring_prod(&cple->awi_ring, cple->awi_depth);
			cple->awi_curr = HWA_CPLE_AWI_CURR(cple);
			cple->awi_free--;

			HWA_ASSERT(cple->awi_entry < cple->awi_depth);
			HWA_ASSERT(cple->awi_free ==
				bcm_ring_prod_avail(&cple->awi_ring, cple->awi_depth));

			cple->wi_entry = 0; // prepare for next WI addition
		}

		return; // lazy path

	}

commit_ced: // Post CED due to threshold, wrap, no free AWI, or SYNC commit req

	__hwa_cple_flush(cple);

	// Lazily allocate a new AWI and prepare it for WI additions
	if (cple->awi_curr == HWA_CPLE_AWI_NONE) {
		cple->awi_xfer = __hwa_cple_allocate(cple, HWA_CPLE_REFRESH_LAZY);
	} else {
		cple->awi_xfer = cple->awi_curr;
	}

} // __hwa_cple_commit

static void
BCMATTACHFN(_hwa_cple_dettach)(hwa_dev_t *dev,
	const char *name, hwa_cple_t *cple)
{
	void *memory;
	uint32 mem_sz;

	HWA_TRACE(("%s %s detach\n", HWAce, name));

	// Free memory and reset HWAce CED Queue ring
	if (cple->cedq_ring.memory != (void*)NULL) {
		memory = cple->cedq_ring.memory;
		mem_sz = cple->cedq_ring.depth * sizeof(hwa_cpleng_ced_t);
		HWA_TRACE(("%s cedq_ring -memory[%p,%u]\n", HWAce, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		hwa_ring_fini(&cple->cedq_ring);
	}

	// Free memory for local circular ring of CPL work items
	if (cple->awi_base != (void*)NULL) {
		memory = cple->awi_base;
		mem_sz = cple->awi_depth * cple->awi_size;
		HWA_TRACE(("%s wi array -memory[%p,%u]\n", HWAce, memory, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
	}

	memset(cple, 0, sizeof(*cple));

} // _hwa_cple_dettach

static hwa_cple_t *
BCMATTACHFN(_hwa_cple_attach)(hwa_dev_t *dev, const char *name,
	hwa_cple_t *cple, hwa_cpleng_cedq_idx_t cedq_idx,
	uint16 cedq_ring_depth, uint32 cedq_ring_id, uint32 cedq_ring_num,
	uint16 awi_ring_depth, uint16 awi_size, uint32 awi_commit,
	uint32 d2h_ring_idx)
{
	uint16 idx;
	void *memory;
	uint32 u32, mem_sz;
	hwa_regs_t *regs;
	hwa_cpl_regs_t *cpl_regs;
	hwa_cpleng_ced_t *ced;
	uint32 cpl_ring_idx = d2h_ring_idx - BCMPCIE_D2H_MSGRING_TX_COMPLETE_IDX;

	HWA_TRACE(("%s %s ring_idx<%u> cedq depth<%u> WIring[depth<%u> size<%u>]\n",
		HWAce, name, cpl_ring_idx, cedq_ring_depth, awi_ring_depth, awi_size));

	HWA_ASSERT(cpl_ring_idx < HWA_CPLENG_CEDQ_TOT);
	HWA_ASSERT(cedq_idx == cpl_ring_idx);

	// Setup locals
	regs = dev->regs;
	cpl_regs = &regs->cpl;

	// CAUTION: One CEDQ per D2H Completion common Ring

	// Verify HWAce block's structures
	HWA_ASSERT(sizeof(hwa_cpleng_ced_t) == HWA_CPLENG_CED_BYTES);

	// Allocate and initialize S2H Rx CEDQ interface
	mem_sz = cedq_ring_depth * sizeof(hwa_cpleng_ced_t);
	if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
		HWA_ERROR(("%s %s cedq_ring malloc size<%u> failure\n",
			HWAce, name, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		return (hwa_cple_t*)NULL;
	}
	HWA_TRACE(("%s %s CEDQ ring +memory[%p,%u]\n",
		HWAce, name, memory, mem_sz));
	u32 = HWA_PTR2UINT(memory);
	HWA_WR_REG_NAME(HWAce, regs, cpl, cedq[cedq_idx].base, u32);
	HWA_WR_REG_NAME(HWAce, regs, cpl, cedq[cedq_idx].depth, cedq_ring_depth);
	hwa_ring_init(&cple->cedq_ring, name, cedq_ring_id,
		HWA_RING_S2H, cedq_ring_num, cedq_ring_depth, memory,
		&cpl_regs->cedq[cedq_idx].wridx, &cpl_regs->cedq[cedq_idx].rdidx);

	// Pre-build CEDs in entire S2H CEDQ Interface
	for (idx = 0; idx < cedq_ring_depth; idx++) {

		ced = HWA_RING_ELEM(hwa_cpleng_ced_t, &cple->cedq_ring, idx);

		// Setup remote and local interrupt generation per CED
		ced->host_intr_valid = 1;
		ced->fw_intr_valid   = 0; //((idx % HWA_CPLENG_CEDQ_FW_WAKEUP) == 0);
		ced->cpl_ring_id     = cpl_ring_idx; // all CEDs to same D2H ring
		ced->md_count        = 0;
		ced->md_array_addr   = 0U;

		// Runtime: wi_count, wi_array_addr need only be filled
	}

	// Complete initializing CplE SW context
	cple->cpl_ring_idx = cpl_ring_idx;
	cple->cedq_base    = (hwa_cpleng_ced_t*)memory;
	cple->aggr_max     = (HWA_PCIEIPC_WI_AGGR_CNT == 0) ? 1 : HWA_PCIEIPC_WI_AGGR_CNT;

	// Allocate and initialize AWI array circular ring
	mem_sz = awi_ring_depth * awi_size;
	if ((memory = MALLOCZ(dev->osh, mem_sz)) == NULL) {
		HWA_ERROR(("%s %s AWI array malloc size<%u> failure\n",
			HWAce, name, mem_sz));
		HWA_ASSERT(memory != (void*)NULL);
		return (hwa_cple_t*)NULL;
	}
	HWA_TRACE(("%s %s AWI array +memory[%p,%u]>\n",
		HWAce, name, memory, mem_sz));

	bcm_ring_init(&cple->awi_ring);

	cple->awi_base  = memory;         // AWI array memory
	cple->awi_depth = awi_ring_depth; // AWI ring array depth
	cple->awi_size  = awi_size;       // size of an ACWI in the ring
	cple->awi_pend  = 0;              // number of pending AWI
	cple->awi_comm  = awi_commit;     // pending AWI flush threshold for commit

	// Setup one AWI for immediate use, with wi_entry identifying next entry

	cple->wi_added  = 0;
	cple->wi_entry  = 0; // next entry is the 0th position in AWI
	cple->awi_xfer  = memory; // base of list of AWI to be xfered using a CED

	// Pre-allocate an AWI entry and advance WR to AWI array index 1
	cple->awi_entry = bcm_ring_prod(&cple->awi_ring, cple->awi_depth);
	cple->awi_curr  = HWA_CPLE_AWI_CURR(cple);
	cple->awi_free  = bcm_ring_prod_avail(&cple->awi_ring, cple->awi_depth);
	// max available is awi_ring_depth - 1, and 1 in use
	HWA_ASSERT(cple->awi_free == (awi_ring_depth - 1 - 1));

	cple->cedq_idx = cedq_idx; // pair the wi array buffer to cedq

	return cple;

} // _hwa_cple_attach

void // HWAce: Cleanup/Free resources used by HWA2b and HWA4b CpleEng blocks
BCMATTACHFN(hwa_cpleng_detach)(hwa_cpleng_t *cpleng)
{
	hwa_dev_t *dev;
#ifdef RXCPL4
	uint32 i;
#endif // endif

	HWA_FTRACE(HWAce);

	if (cpleng == (hwa_cpleng_t*)NULL)
		return;

	// Audit pre-conditions
	dev = HWA_DEV(cpleng);

	HWA_TXCPLE_EXPR(_hwa_cple_dettach(dev, "TXC", &cpleng->txcple));
#ifdef RXCPL4
	for (i = 0; i < 4; i++) {
		HWA_RXCPLE_EXPR(_hwa_cple_dettach(dev, "RXC", &cpleng->rxcple[i]));
	}
#else
	HWA_RXCPLE_EXPR(_hwa_cple_dettach(dev, "RXC", &cpleng->rxcple));
#endif // endif

} // hwa_cpleng_detach

hwa_cpleng_t * // HWAce: Allocate resources for HWA2b and HWA4b cpleng blocks
BCMATTACHFN(hwa_cpleng_attach)(hwa_dev_t *dev)
{
	hwa_cple_t *cple;
	hwa_cpleng_t *cpleng;
#ifdef RXCPL4
	HWA_RXCPLE_EXPR(uint32 i);
#endif // endif

	HWA_FTRACE(HWAce);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	cpleng = &dev->cpleng;

	// Verify HWA CplEngine block's structures

	HWA_ASSERT(sizeof(hwa_cpleng_ced_t) == HWA_CPLENG_CED_BYTES);

	// Confirm HWA Completion Engine Capabilities agains 43684 Generic
	{
		uint32 cap1;
		cap1 = HWA_RD_REG_NAME(HWAce, dev->regs, top, hwahwcap1);
		BCM_REFERENCE(cap1);
		HWA_ASSERT(BCM_GBF(cap1, HWA_TOP_HWAHWCAP1_MAXHOSTRINGS2B4B) ==
			HWA_CPLENG_CED_QUEUES_MAX);

		// 43684 allows upto 4 RxCpl rings, for per AccessCategory RxPath.
		// Currently, only one RxCompletion ring is instantiated in DHD.

		// DHD instantiates two D2H Completion rings, one for Tx and one for Rx
		HWA_ASSERT(HWA_CPLENG_CEDQ_TOT <= HWA_CPLENG_CED_QUEUES_MAX);
	}

#ifdef HWA_TXCPLE_BUILD
	HWA_ASSERT(sizeof(hwa_txcple_acwi_t) == HWA_TXCPLE_ACWI_BYTES);
	cple = _hwa_cple_attach(dev, "TXC", &cpleng->txcple,
	           HWA_CPLENG_CEDQ_TX, HWA_TXCPLE_CEDQ_RING_DEPTH,
	           HWA_TXCPLE_ID, HWA_TXCPLE_CED_S2H_RINGNUM,
	           HWA_TXCPLE_ACWI_RING_DEPTH,
	           ((HWA_PCIEIPC_WI_AGGR_CNT == 0) ? HWA_TXCPLE_CWI_BYTES : HWA_TXCPLE_ACWI_BYTES),
	           ((HWA_PCIEIPC_WI_AGGR_CNT == 0) ?
	               (HWA_TXCPLE_FLUSH_THRESHOLD * 4) : HWA_TXCPLE_FLUSH_THRESHOLD),
	           BCMPCIE_D2H_RING_IDX(BCMPCIE_D2H_MSGRING_TX_COMPLETE));
	if (cple == (NULL)) {
		HWA_ERROR(("%s TXCPLE attach failure\n", HWA2b));
		goto failure;
	}
#endif /* HWA_TXCPLE_BUILD */

#ifdef HWA_RXCPLE_BUILD
	// Presently only one Rx Completion Ring is supported by DHD/Runner
	// HWA has the capability to support 4 RX CEDQ rings for Rx - one per AC
	HWA_ASSERT(sizeof(hwa_rxcple_acwi_t) == HWA_RXCPLE_ACWI_BYTES);
#ifdef RXCPL4
	for (i = 0; i < 4; i++) {
		cple = _hwa_cple_attach(dev, "RXC", &cpleng->rxcple[i],
			HWA_CPLENG_CEDQ_RX + i, HWA_RXCPLE_CEDQ_RING_DEPTH,
			HWA_RXCPLE_ID, HWA_RXCPLE_CED_S2H_RINGNUM,
			HWA_RXCPLE_ACWI_RING_DEPTH,
			((HWA_PCIEIPC_WI_AGGR_CNT == 0) ?
			HWA_RXCPLE_CWI_BYTES : HWA_RXCPLE_ACWI_BYTES),
			((HWA_PCIEIPC_WI_AGGR_CNT == 0) ?
			(HWA_RXCPLE_FLUSH_THRESHOLD * 4) : HWA_RXCPLE_FLUSH_THRESHOLD),
			BCMPCIE_D2H_RING_IDX(BCMPCIE_D2H_MSGRING_RX_COMPLETE) + i);
		if (cple == (NULL)) {
			HWA_ERROR(("%s RXCPLE attach failure\n", HWA2b));
			goto failure;
		}
	}
#else
	cple = _hwa_cple_attach(dev, "RXC", &cpleng->rxcple,
	           HWA_CPLENG_CEDQ_RX, HWA_RXCPLE_CEDQ_RING_DEPTH,
	           HWA_RXCPLE_ID, HWA_RXCPLE_CED_S2H_RINGNUM,
	           HWA_RXCPLE_ACWI_RING_DEPTH,
	           ((HWA_PCIEIPC_WI_AGGR_CNT == 0) ? HWA_RXCPLE_CWI_BYTES : HWA_RXCPLE_ACWI_BYTES),
	           ((HWA_PCIEIPC_WI_AGGR_CNT == 0) ?
	               (HWA_RXCPLE_FLUSH_THRESHOLD * 4) : HWA_RXCPLE_FLUSH_THRESHOLD),
	           BCMPCIE_D2H_RING_IDX(BCMPCIE_D2H_MSGRING_RX_COMPLETE));
	if (cple == (NULL)) {
		HWA_ERROR(("%s RXCPLE attach failure\n", HWA2b));
		goto failure;
	}
#endif /* RXCPL4 */
#endif /* HWA_RXCPLE_BUILD */

	cpleng->ring_addr = hwa_axi_addr(dev, HWA_AXI_CPLE_RING_CONTEXTS);

	HWA_TRACE(("%s axi ring_addr<0x%08x>\n", HWAce, cpleng->ring_addr));

	return cpleng;

failure:
	hwa_cpleng_detach(cpleng);
	HWA_WARN(("%s attach failure\n", HWAce));

	return ((hwa_cpleng_t*)NULL);

} // hwa_cpleng_attach

int // HWA2b and HWA4b configuration and initialization
hwa_cpleng_init(hwa_cpleng_t *cpleng)
{
	uint32 u32;
#if defined(HWA_RXCPLE_BUILD) && defined(RXCPL4)
	uint32 idx;
#endif // endif
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	pcie_ipc_rings_t *pcie_ipc_rings;
	pcie_ipc_ring_mem_t *pcie_ipc_ring_mem;
	hwa_cpleng_ring_t cpleng_ring;
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;

	HWA_FTRACE(HWAce);

	// Audit pre-conditions
	dev = HWA_DEV(cpleng);
	HWA_ASSERT(dev->pcie_ipc_rings != (pcie_ipc_rings_t*)NULL);
	HWA_ASSERT((dev->pcie_ipc->hcap1 &
		(PCIE_IPC_HCAP1_HWA_TXCPL_IDMA | PCIE_IPC_HCAP1_HWA_RXCPL_IDMA)) != 0);

	// Check initialization.
	if (dev->inited)
		return HWA_SUCCESS;

	// Setup locals
	regs = dev->regs;
	pcie_ipc_rings = dev->pcie_ipc_rings;
	sys_mem = &cpleng_ring.u32[0];

	u32 = HWA_RD_REG_NAME(HWAce, regs, cpl, cpl_dma_config);
	HWA_TRACE(("%s cpl_dma_config numcompletionworkitem<%u>\n",
		HWAce, BCM_GBF(u32, HWA_CPL_CPL_DMA_CONFIG_NUMCOMPLETIONWORKITEM)));

	// Current D2H CPL Ring order:
	// D2H_TXCPL = 0, D2H_RXCPL = 1.
	// To make sure correct read index for TXCPL. Need to configured TxCpl offset to 0.
	u32 = BCM_CBF(u32, HWA_CPL_CPL_DMA_CONFIG_TXCPLRINGOFFSET);
	HWA_WR_REG_NAME(HWAce, regs, cpl, cpl_dma_config, u32);

	//        how WR index is updated in host RD index arrays.

	// Setup base addresses of indices arrays in host memory
	u32 = HADDR64_LO(pcie_ipc_rings->d2h_wr_haddr64)
		+ HWA_CPLENG_RING_RDWR_BASE_OFFSET;
	HWA_WR_REG_NAME(HWAce, regs, cpl, host_wridx_addr_l, u32);
	HWA_WR_REG_NAME(HWAce, regs, cpl, host_wridx_addr_h,
		HWA_HOSTADDR64_HI32(HADDR64_HI(pcie_ipc_rings->d2h_wr_haddr64)));

	// NotPCIE = TCM and HWA. Coherent = PCIE and TCM, AddrExt = None
	u32 = (0U
		// PCIE as Src/Dest: NotPcie = 0, Coh = host, AddrExt = 0
		// | BCM_SBIT(HWA_CPL_DMA_DESC_TEMPLATE_CPL_DMAPCIEDESCTEMPLATENOTPCIE)
		| BCM_SBF(dev->host_coherency,
		          HWA_CPL_DMA_DESC_TEMPLATE_CPL_DMAPCIEDESCTEMPLATECOHERENT)
		| BCM_SBF(0U, HWA_CPL_DMA_DESC_TEMPLATE_CPL_DMAPCIEDESCTEMPLATEADDREXT)
		// Dngl TCM as Src/Dst: NotPcie = 1, Coh = 1, AddrExt = 0
		| BCM_SBIT(HWA_CPL_DMA_DESC_TEMPLATE_CPL_DMATCMDESCTEMPLATENOTPCIE)
		| BCM_SBIT(HWA_CPL_DMA_DESC_TEMPLATE_CPL_DMATCMDESCTEMPLATECOHERENT)
		| BCM_SBF(0U,
		          HWA_CPL_DMA_DESC_TEMPLATE_CPL_DMATCMPCIEDESCTEMPLATEADDREXT)
		| BCM_SBIT(HWA_CPL_DMA_DESC_TEMPLATE_CPL_DMAHWADESCTEMPLATENOTPCIE)
		// non DMA direct AXI transfer of CPL WR index update to host memory
		| BCM_SBF(dev->host_coherency,
		          HWA_CPL_DMA_DESC_TEMPLATE_CPL_NONDMAHOSTWRIDTEMPLATECOHERENT)
		| 0U);
	HWA_WR_REG_NAME(HWAce, regs, cpl, dma_desc_template_cpl, u32);

#ifdef HWA_TXCPLE_BUILD
	// Single D2H TxCpl common ring serves multiple MAC cores
	pcie_ipc_ring_mem =
		HWA_UINT2PTR(pcie_ipc_ring_mem_t, pcie_ipc_rings->ring_mem_daddr32)
		+ BCMPCIE_D2H_MSGRING_TX_COMPLETE;

	HWA_ASSERT(pcie_ipc_ring_mem->id == BCMPCIE_D2H_MSGRING_TX_COMPLETE);
	HWA_ASSERT(pcie_ipc_ring_mem->max_items == D2HRING_TXCMPLT_MAX_ITEM);
#if defined(HWA_TXCPLE_CWI_4B_T)
	HWA_ASSERT(((pcie_ipc_ring_mem->item_type == MSGBUF_WI_CWI) &&
		((pcie_ipc_ring_mem->item_size == HWA_TXCPLE_CWI_BYTES) ||
		(pcie_ipc_ring_mem->item_size == HWA_TXCPLE_CWI_4_BYTES))) ||
		((pcie_ipc_ring_mem->item_type == MSGBUF_WI_ACWI) &&
		(pcie_ipc_ring_mem->item_size == HWA_TXCPLE_ACWI_BYTES)));
#else /* !HWA_TXCPLE_CWI_4B_T */
	HWA_ASSERT(((pcie_ipc_ring_mem->item_type == MSGBUF_WI_CWI) &&
		(pcie_ipc_ring_mem->item_size == HWA_TXCPLE_CWI_BYTES)) ||
		((pcie_ipc_ring_mem->item_type == MSGBUF_WI_ACWI) &&
		(pcie_ipc_ring_mem->item_size == HWA_TXCPLE_ACWI_BYTES)));
#endif /* !HWA_TXCPLE_CWI_4B_T */

	memset(&cpleng_ring, 0, sizeof(cpleng_ring));

	cpleng_ring.base_addr_hi   =
		HWA_HOSTADDR64_HI32(HADDR64_HI(pcie_ipc_ring_mem->haddr64));
	cpleng_ring.base_addr_lo   = HADDR64_LO(pcie_ipc_ring_mem->haddr64);
	cpleng_ring.wr_index       = 0;
	cpleng_ring.rd_index       = 0;
	cpleng_ring.element_sz     = pcie_ipc_ring_mem->item_size;
	cpleng_ring.depth          = pcie_ipc_ring_mem->max_items;
	cpleng_ring.seqnum_enable  = FALSE;
	cpleng_ring.aggr_mode	   = (dev->wi_aggr_cnt > 0);
	cpleng_ring.pkts_per_aggr  = dev->wi_aggr_cnt/2;
	cpleng_ring.intraggr_count = HWA_TXCPLE_INTRAGGR_COUNT;
	cpleng_ring.intraggr_tmout = HWA_TXCPLE_INTRAGGR_TMOUT;
	// Compact WI do not have seqnum: seqnum_start, seqnum_offset, seqnum_width

	// Flush Tx Completion ring configuration to HWA AXI memory
	axi_mem_addr = HWA_TABLE_ADDR(hwa_cpleng_ring_t, cpleng->ring_addr,
	                                  HWA_CPLENG_CEDQ_TX);
	HWA_WR_MEM32(HWA4b, hwa_cpleng_ring_t, axi_mem_addr, sys_mem);
#if defined(HWA_TXCPLE_CWI_4B_T)
	/* Update the HWA awi_size with the size from host */
	cpleng->txcple.awi_size = pcie_ipc_ring_mem->item_size;
#endif /* HWA_TXCPLE_CWI_4B_T */
#endif /* HWA_TXCPLE_BUILD */

#ifdef HWA_RXCPLE_BUILD
	// Single D2H RxCpl common ring serves multiple MAC cores
	pcie_ipc_ring_mem =
		HWA_UINT2PTR(pcie_ipc_ring_mem_t, pcie_ipc_rings->ring_mem_daddr32)
		+ BCMPCIE_D2H_MSGRING_RX_COMPLETE;

	HWA_ASSERT(pcie_ipc_ring_mem->id == BCMPCIE_D2H_MSGRING_RX_COMPLETE);
	HWA_ASSERT(((pcie_ipc_ring_mem->item_type == MSGBUF_WI_CWI) &&
		(pcie_ipc_ring_mem->item_size == HWA_RXCPLE_CWI_BYTES)) ||
		((pcie_ipc_ring_mem->item_type == MSGBUF_WI_ACWI) &&
		(pcie_ipc_ring_mem->item_size == HWA_RXCPLE_ACWI_BYTES)));

#ifdef RXCPL4
	for (idx = 0; idx < 4; idx++, pcie_ipc_ring_mem++) {
		memset(&cpleng_ring, 0, sizeof(cpleng_ring));

		cpleng_ring.base_addr_hi   =
			HWA_HOSTADDR64_HI32(HADDR64_HI(pcie_ipc_ring_mem->haddr64));
		cpleng_ring.base_addr_lo   = HADDR64_LO(pcie_ipc_ring_mem->haddr64);
		cpleng_ring.wr_index	   = 0;
		cpleng_ring.rd_index	   = 0;
		cpleng_ring.element_sz	   = pcie_ipc_ring_mem->item_size;
		cpleng_ring.depth		   = pcie_ipc_ring_mem->max_items;
		cpleng_ring.seqnum_enable  = FALSE;
		cpleng_ring.aggr_mode	   = (dev->wi_aggr_cnt > 0);
		cpleng_ring.pkts_per_aggr  = dev->wi_aggr_cnt/2;
		cpleng_ring.intraggr_count = HWA_RXCPLE_INTRAGGR_COUNT;
		cpleng_ring.intraggr_tmout = HWA_RXCPLE_INTRAGGR_TMOUT;
		// Compact WI do not have seqnum: seqnum_start, seqnum_offset, seqnum_width

		// Flush Rx Completion ring configuration to HWA AXI memory
		axi_mem_addr = HWA_TABLE_ADDR(hwa_cpleng_ring_t, cpleng->ring_addr,
			(HWA_CPLENG_CEDQ_RX + idx));
		HWA_WR_MEM32(HWA2b, hwa_cpleng_ring_t, axi_mem_addr, sys_mem);
	}
#else
	memset(&cpleng_ring, 0, sizeof(cpleng_ring));

	cpleng_ring.base_addr_hi   =
		HWA_HOSTADDR64_HI32(HADDR64_HI(pcie_ipc_ring_mem->haddr64));
	cpleng_ring.base_addr_lo   = HADDR64_LO(pcie_ipc_ring_mem->haddr64);
	cpleng_ring.wr_index       = 0;
	cpleng_ring.rd_index       = 0;
	cpleng_ring.element_sz     = pcie_ipc_ring_mem->item_size;
	cpleng_ring.depth          = pcie_ipc_ring_mem->max_items;
	cpleng_ring.seqnum_enable  = FALSE;
	cpleng_ring.aggr_mode	   = (dev->wi_aggr_cnt > 0);
	cpleng_ring.pkts_per_aggr  = dev->wi_aggr_cnt/2;
	cpleng_ring.intraggr_count = HWA_RXCPLE_INTRAGGR_COUNT;
	cpleng_ring.intraggr_tmout = HWA_RXCPLE_INTRAGGR_TMOUT;
	// Compact WI do not have seqnum: seqnum_start, seqnum_offset, seqnum_width

	// Flush Rx Completion ring configuration to HWA AXI memory
	axi_mem_addr = HWA_TABLE_ADDR(hwa_cpleng_ring_t, cpleng->ring_addr,
	                                  HWA_CPLENG_CEDQ_RX);
	HWA_WR_MEM32(HWA2b, hwa_cpleng_ring_t, axi_mem_addr, sys_mem);
#endif /* RXCPL4 */
#endif /* HWA_RXCPLE_BUILD */

	return HWA_SUCCESS;

} // hwa_cpleng_init

void // HWA2b and HWA4b deinitialization
hwa_cpleng_deinit(hwa_cpleng_t *cpleng)
{
}

void // HWA2b and HWA4b status
hwa_cpleng_status(hwa_cpleng_t *cpleng, struct bcmstrbuf *b)
{
	uint32 u32;
#ifdef RXCPL4
	uint32 idx;
#endif // endif
	hwa_dev_t *dev;
	hwa_regs_t *regs;
	uint32 *sys_mem;
	hwa_mem_addr_t axi_mem_addr;
	hwa_cpleng_ring_t cpleng_ring;

	HWA_FTRACE(HWAce);

	// Audit pre-conditions
	dev = HWA_DEV(cpleng);

	// Setup locals
	regs = dev->regs;
	sys_mem = &cpleng_ring.u32[0];

	u32 = HWA_RD_REG_NAME(HWAce, regs, cpl, ce_sts);
	HWA_BPRINT(b, "%s ce_sts state<%u> stop<%u>\n", HWAce,
		BCM_GBF(u32, HWA_CPL_CE_STS_ENGINESTATE),
		BCM_GBF(u32, HWA_CPL_CE_STS_ENGINESTOPREASON));

	u32 = HWA_RD_REG_NAME(HWAce, regs, cpl, ce_bus_err);
	HWA_BPRINT(b, "%s ce_bus_err cedq<%u> md<%u> cw<%u> md_item<%u>\n", HWAce,
		BCM_GBF(u32, HWA_CPL_CE_BUS_ERR_CEDQNUM),
		BCM_GBF(u32, HWA_CPL_CE_BUS_ERR_MDERROR),
		BCM_GBF(u32, HWA_CPL_CE_BUS_ERR_CWERROR),
		BCM_GBF(u32, HWA_CPL_CE_BUS_ERR_ITEMNUM));

	// Fetch the WR and RD index of the D2H PCIE Tx Completion common ring
	axi_mem_addr = HWA_TABLE_ADDR(hwa_cpleng_ring_t, cpleng->ring_addr,
	                                  HWA_CPLENG_CEDQ_TX);
	HWA_RD_MEM32(HWA4b, hwa_cpleng_ring_t, axi_mem_addr, sys_mem);
	HWA_BPRINT(b, "%s cedq<%u> WR<%u> RD<%u>\n",
		HWA4b, HWA_CPLENG_CEDQ_TX, cpleng_ring.wr_index, cpleng_ring.rd_index);

	// Fetch the WR and RD index of the D2H PCIE Rx Completion common ring
#ifdef RXCPL4
	for (idx = 0; idx < 4; idx++) {
		axi_mem_addr = HWA_TABLE_ADDR(hwa_cpleng_ring_t, cpleng->ring_addr,
		                                  (HWA_CPLENG_CEDQ_RX + idx));
		HWA_RD_MEM32(HWA2b, hwa_cpleng_ring_t, axi_mem_addr, sys_mem);
		HWA_BPRINT(b, "%s cedq<%u> WR<%u> RD<%u>\n",
			HWA2b, HWA_CPLENG_CEDQ_RX + idx, cpleng_ring.wr_index,
			cpleng_ring.rd_index);
	}
#else
	axi_mem_addr = HWA_TABLE_ADDR(hwa_cpleng_ring_t, cpleng->ring_addr,
	                                  HWA_CPLENG_CEDQ_RX);
	HWA_RD_MEM32(HWA2b, hwa_cpleng_ring_t, axi_mem_addr, sys_mem);
	HWA_BPRINT(b, "%s cedq<%u> WR<%u> RD<%u>\n",
		HWA2b, HWA_CPLENG_CEDQ_RX, cpleng_ring.wr_index, cpleng_ring.rd_index);
#endif // endif

} // hwa_cpleng_status

// HWA2b and HWA4b statistics collection
static void _hwa_cpleng_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 cedq);

void // Clear queue statistics for HWA2b and 4b, as well as common
hwa_cpleng_stats_clear(hwa_cpleng_t *cpleng, uint32 cedq)
{
	hwa_dev_t *dev;

	dev = HWA_DEV(cpleng);

	if (cedq == ~0U)
		hwa_stats_clear(dev, HWA_STATS_CPLENG_COMMON); // clear common stats
	else
		hwa_stats_clear(dev, HWA_STATS_CPLENG_CEDQ + cedq); // per cdeq stats

} // hwa_cpleng_stats_clear

void // Print the common or queue statistics for HWA2b and 4b CplEngines
_hwa_cpleng_stats_dump(hwa_dev_t *dev, uintptr buf, uint32 cedq)
{
	hwa_cpleng_t *cpleng = &dev->cpleng;
	struct bcmstrbuf *b = (struct bcmstrbuf *)buf;

	if (cedq == ~0U) {
		// Dump statistics common across all CEDQs
		hwa_cpleng_common_stats_t *common_stats = &cpleng->common_stats;
		HWA_BPRINT(b, "%s statistics stall dma<%u> duration dma<%u>\n",
			HWAce, common_stats->num_stalls_dma, common_stats->dur_dma_busy);
	} else {
		// Dump per CEDQ statistics
		hwa_cpleng_cedq_stats_t *cedq_stats = &cpleng->cedq_stats[cedq];
		HWA_BPRINT(b, "%s statistics queue<%u> completions<%u> interrupts<%u>\n"
			"+ aggr[cnt<%u> touts<%u>] explicit[fw<%u> host<%u>] stalls<%u>\n",
			HWAce, cedq,
			cedq_stats->num_completions, cedq_stats->num_interrupts,
			cedq_stats->num_aggr_counts, cedq_stats->num_aggr_tmouts,
			cedq_stats->num_expl_fw_intr, cedq_stats->num_expl_host_intr,
			cedq_stats->num_host_ring_full);
	}

} //_hwa_cpleng_stats_dump

void // Query and dump block statistics for HWA2b and HWA4b CplEngines
hwa_cpleng_stats_dump(hwa_cpleng_t *cpleng, struct bcmstrbuf *b, uint8 clear_on_copy)
{
	uint32 cedq;
	hwa_dev_t *dev;
	hwa_cpleng_common_stats_t *common_stats;
	hwa_cpleng_cedq_stats_t *cedq_stats;

	dev = HWA_DEV(cpleng);

	common_stats = &cpleng->common_stats;
	hwa_stats_copy(dev, HWA_STATS_CPLENG_COMMON,
		HWA_PTR2UINT(common_stats), HWA_PTR2HIADDR(common_stats),
		/* num_sets */ 1, clear_on_copy, &_hwa_cpleng_stats_dump,
		(uintptr)b, ~0U);

	for (cedq = 0; cedq < HWA_CPLENG_CEDQ_TOT; cedq++) {
		cedq_stats = &cpleng->cedq_stats[cedq];
		hwa_stats_copy(dev, HWA_STATS_CPLENG_CEDQ + cedq,
			HWA_PTR2UINT(cedq_stats), HWA_PTR2HIADDR(cedq_stats),
			/* num_sets */ 1, clear_on_copy, &_hwa_cpleng_stats_dump,
			(uintptr)b, cedq);
	}

} // hwa_cpleng_stats_dump

#if defined(BCMDBG)

void // Dump software state of a completion engine
hwa_cple_dump(hwa_cple_t *cple, struct bcmstrbuf *b, const char *name, bool verbose)
{
	HWA_BPRINT(b, "%s dump cple<%p> cedq_idx<%u> cpl_ring_idx<%u>\n",
		name, cple, cple->cedq_idx, cple->cpl_ring_idx);

	hwa_ring_dump(&cple->cedq_ring, b, "+ cedq_ring");
	HWA_BPRINT(b, "+ CEDQ base<%p>\n", cple->cedq_base);
	HWA_BPRINT(b, "+ WI array[WR<%u> RD<%u>] xfer<%p> curr<%p> entry<%u> add<%u>\n"
		"+ AWI pend<%u> free<%u> comm<%u> array<%p> size<%u> depth<%u>\n",
		cple->awi_ring.write, cple->awi_ring.read,
		cple->awi_xfer, cple->awi_curr, cple->wi_entry, cple->wi_added,
		cple->awi_pend, cple->awi_free, cple->awi_comm,
		cple->awi_base, cple->awi_size, cple->awi_depth);

	HWA_STATS_EXPR(
		HWA_BPRINT(b, "+ ced upd<%u> xfer<%u>\n"
			"+ wi total<%u> awi xfer<%u> wi/xfer<%u> sync<%u> full<%u>\n",
			cple->cedq_upd_count, cple->cedq_xfer_count,
			cple->wi_total_count, cple->awi_xfer_count,
			(cple->awi_xfer_count == 0) ? 0 :
			cple->wi_total_count / cple->awi_xfer_count,
			cple->awi_sync_count, cple->awi_full_count));

} // hwa_cple_dump

void // Dump software state for HWA2b and HWA4b blocks
hwa_cpleng_dump(hwa_cpleng_t *cpleng, struct bcmstrbuf *b, bool verbose, bool dump_regs)
{
#ifdef RXCPL4
	uint i;
#endif // endif
	if (cpleng == (hwa_cpleng_t*)NULL)
		return;

	HWA_BPRINT(b, "%s dump<%p> axi ring_addr<0x%08x>\n",
		HWAce, cpleng, cpleng->ring_addr);

	HWA_TXCPLE_EXPR(hwa_cple_dump(&cpleng->txcple, b, "TXCPLE", verbose));
#ifdef RXCPL4
	for (i = 0; i < 4; i++) {
		HWA_RXCPLE_EXPR(hwa_cple_dump(&cpleng->rxcple[i], b, "RXCPLE", verbose));
	}
#else
	HWA_RXCPLE_EXPR(hwa_cple_dump(&cpleng->rxcple, b, "RXCPLE", verbose));
#endif // endif

	hwa_cpleng_status(cpleng, b);

	if (verbose)
		hwa_cpleng_stats_dump(cpleng, b, /* clear */ 0);

#if defined(WLTEST)
	if (dump_regs)
		hwa_cpleng_regs_dump(cpleng, b);
#endif // endif
} // hwa_cpleng_dump

#if defined(WLTEST)
void // Dump HWA registers for HWA2b and HWA4b blocks
hwa_cpleng_regs_dump(hwa_cpleng_t *cpleng, struct bcmstrbuf *b)
{
	uint32 i;
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	dev = HWA_DEV(cpleng);
	regs = dev->regs;

	HWA_BPRINT(b, "%s registers[%p] offset[0x%04x]\n",
		HWAce, &regs->cpl, OFFSETOF(hwa_regs_t, cpl));

	HWA_BPR_REG(b, cpl, ce_sts);
	HWA_BPR_REG(b, cpl, ce_bus_err);

	for (i = 0; i < HWA_CPLENG_CEDQ_TOT; i++) {
		HWA_BPR_REG(b, cpl, cedq[i].base);
		HWA_BPR_REG(b, cpl, cedq[i].depth);
		HWA_BPR_REG(b, cpl, cedq[i].wridx);
		HWA_BPR_REG(b, cpl, cedq[i].rdidx);
	} // for cedq

	HWA_BPR_REG(b, cpl, host_wridx_addr_h);
	HWA_BPR_REG(b, cpl, host_wridx_addr_l);
	HWA_BPR_REG(b, cpl, cpl_dma_config);
	HWA_BPR_REG(b, cpl, dma_desc_template_cpl);

} // hwa_cpleng_regs_dump

#endif // endif

#endif /* BCMDBG */

#endif /* HWA_CPLENG_BUILD */

#ifdef HWA_RXCPLE_BUILD
int // Add a RxCompletion WI to the circular WI array
hwa_rxcple_wi_add(struct hwa_dev *dev, uint8 ifid, uint8 flags,
	uint8 data_offset, uint16 data_len, uint32 pktid)
{
	hwa_cple_t *rxcple;
	hwa_rxcple_acwi_t *rxcple_awi; // RxCompletion aggregated compact WI

	HWA_TRACE(("%s dev<%p> ifid<%u> flags<%u> data off<%u> len<%u> pktid<0x%08x,%u>\n",
		HWA2b, dev, ifid, flags, data_offset, data_len, pktid, pktid));

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	HWA_ASSERT(ifid < HWA_INTERFACES_TOT);
	HWA_ASSERT(flags < 8); // 3 bits
	HWA_ASSERT(pktid != HWA_HOST_PKTID_NULL);

	// Setup locals
#ifdef RXCPL4
	rxcple = &dev->cpleng.rxcple[dev->rxcpl_inuse];
#else
	rxcple = &dev->cpleng.rxcple;
#endif // endif

	// Allocate space for a WI in an ACWI
	if (__hwa_cple_prepare(rxcple) == HWA_CPLE_AWI_NONE) {
		return HWA_FAILURE;
	}

	HWA_ASSERT(rxcple->awi_curr != HWA_CPLE_AWI_NONE);
	HWA_ASSERT(rxcple->wi_entry < rxcple->aggr_max);

	// Fetch the current aggregate work item
	rxcple_awi = (hwa_rxcple_acwi_t*)rxcple->awi_curr;

	// Populate the composite entry into the aggregate composite WI
	rxcple_awi->u32[rxcple->wi_entry * HWA_RXCPLE_CWI_WORDS] = (uint32)
		BCM_SBF(ifid, HWA_RXCPLE_IFID) | // 5b
		BCM_SBF(flags, HWA_RXCPLE_FLAGS) | // 3b
		BCM_SBF(data_offset, HWA_RXCPLE_DATA_OFFSET) |
		BCM_SBF(data_len, HWA_RXCPLE_DATA_LEN);
	rxcple_awi->aggr[rxcple->wi_entry].host_pktid = pktid;

	rxcple->wi_added++;

	HWA_TRACE(("%s +AWI<%p,%u,%u>\n",
		HWA2b, rxcple_awi, rxcple->awi_entry, rxcple->wi_entry));

	HWA_STATS_EXPR(rxcple->wi_total_count++);

	// Update wi_entry and lazy commit CED if required
	__hwa_cple_commit(rxcple, HWA_CPLE_COMMIT_LAZY);

	return HWA_SUCCESS;

} // hwa_rxcple_wi_add

void // Commit all added WI to the D2H RxCompletion common ring
hwa_rxcple_commit(struct hwa_dev *dev)
{
	hwa_cple_t *rxcple;

	HWA_FTRACE(HWA2b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
#ifdef RXCPL4
	rxcple = &dev->cpleng.rxcple[dev->rxcpl_inuse];
#else
	rxcple = &dev->cpleng.rxcple;
#endif // endif

	if (rxcple->wi_added) { // at least one pending WI to be committed

		// If an AWI is not fully populated, tag the last WI entry as INVALID
		if ((rxcple->wi_entry) & (rxcple->wi_entry < rxcple->aggr_max)) {
			hwa_rxcple_acwi_t *rxcple_awi;
			rxcple_awi = (hwa_rxcple_acwi_t*)rxcple->awi_curr;
			rxcple_awi->aggr[rxcple->wi_entry].host_pktid = HWA_HOST_PKTID_NULL;
			rxcple->awi_pend++;
			rxcple->awi_curr = HWA_CPLE_AWI_NONE;
		}

		// Commit all pending RxCpl WI to the D2H Completion common ring
		__hwa_cple_commit(rxcple, HWA_CPLE_COMMIT_SYNC);
	}
}

bool // Check if there is space to add workitem for the D2H RxCompletion common ring
hwa_rxcple_resource_avail_check(struct hwa_dev *dev)
{
	hwa_cple_t *rxcple;

	HWA_FTRACE(HWA2b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
#ifdef RXCPL4
	rxcple = &dev->cpleng.rxcple[dev->rxcpl_inuse];
#else
	rxcple = &dev->cpleng.rxcple;
#endif // endif
	// Allocate space for a WI in an ACWI
	if (__hwa_cple_prepare(rxcple) == HWA_CPLE_AWI_NONE) {
		return FALSE;
	}

	return TRUE;

}

uint16
hwa_rxcple_pend_item_cnt(struct hwa_dev *dev)
{
	hwa_cple_t *rxcple;

	HWA_FTRACE(HWA2b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
#ifdef RXCPL4
	rxcple = &dev->cpleng.rxcple[dev->rxcpl_inuse];
#else
	rxcple = &dev->cpleng.rxcple;
#endif // endif
	return (rxcple->wi_added);
}

#endif /* HWA_RXCPLE_BUILD */

#ifdef HWA_TXCPLE_BUILD
int // Add a TxCompletion WI to the circular WI array
hwa_txcple_wi_add(struct hwa_dev *dev, uint32 pktid, uint16 ringid, uint8 ifindx)
{
	hwa_cple_t *txcple;
	hwa_txcple_acwi_t *txcple_awi; // TxCompletion aggregated compact WI

	HWA_TRACE(("%s dev<%p> pktid<0x%08x,%u>\n", HWA4b, dev, pktid, pktid));

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	if (pktid == HWA_HOST_PKTID_NULL) {
		HWA_ERROR(("%s NULL PKTID! ringid<%d> ifindx<%d>\n", HWA4b, ringid, ifindx));
		return HWA_SUCCESS;
	}

	// Setup locals
	txcple = &dev->cpleng.txcple;

	// Allocate space for a WI in an ACWI
	if (__hwa_cple_prepare(txcple) == HWA_CPLE_AWI_NONE) {
		return HWA_FAILURE;
	}

	HWA_ASSERT(txcple->awi_curr != HWA_CPLE_AWI_NONE);
	HWA_ASSERT(txcple->wi_entry < txcple->aggr_max);

	// Fetch the current aggregate work item
	txcple_awi = (hwa_txcple_acwi_t*)txcple->awi_curr;

	// Populate the composite entry into the aggregate composite WI
	txcple_awi->aggr[txcple->wi_entry].host_pktid = pktid;
	txcple_awi->aggr[txcple->wi_entry].flow_ring_id = ringid;
	txcple_awi->aggr[txcple->wi_entry].if_id = ifindx;

	txcple->wi_added++;

	HWA_TRACE(("%s +AWI<%p,%u,%u>\n",
		HWA4b, txcple_awi, txcple->awi_entry, txcple->wi_entry));

	HWA_STATS_EXPR(txcple->wi_total_count++);

	// Update wi_entry and lazy commit CED if required
	__hwa_cple_commit(txcple, HWA_CPLE_COMMIT_LAZY);

	return HWA_SUCCESS;

} // hwa_txcple_wi_add

void // Commit all added WI to the D2H TxCompletion common ring
hwa_txcple_commit(struct hwa_dev *dev)
{
	hwa_cple_t *txcple;

	HWA_FTRACE(HWA4b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	txcple = &dev->cpleng.txcple;

	if (txcple->wi_added) { // at least one pending WI to be committed

		// If an AWI is not fully populated, tag the last WI entry as INVALID
		if ((txcple->wi_entry > 0) & (txcple->wi_entry < txcple->aggr_max)) {
			hwa_txcple_acwi_t *txcple_awi;
			txcple_awi = (hwa_txcple_acwi_t*)txcple->awi_curr;
			txcple_awi->aggr[txcple->wi_entry].host_pktid = HWA_HOST_PKTID_NULL;
			txcple->awi_pend++;
			txcple->awi_curr = HWA_CPLE_AWI_NONE;
		}

		// Commit all pending TxCpl WI to the D2H Completion common ring
		__hwa_cple_commit(txcple, HWA_CPLE_COMMIT_SYNC);
	}
}

bool // Check if there is space to add workitem for the D2H TxCompletion common ring
hwa_txcple_resource_avail_check(struct hwa_dev *dev)
{
	hwa_cple_t *txcple;

	HWA_FTRACE(HWA4b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	txcple = &dev->cpleng.txcple;

	// Allocate space for a WI in an ACWI
	if (__hwa_cple_prepare(txcple) == HWA_CPLE_AWI_NONE) {
		return FALSE;
	}

	return TRUE;

}

uint16
hwa_txcple_pend_item_cnt(struct hwa_dev *dev)
{
	hwa_cple_t *txcple;

	HWA_FTRACE(HWA4b);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	txcple = &dev->cpleng.txcple;

	return (txcple->wi_added);
}
#endif /* HWA_TXCPLE_BUILD */
