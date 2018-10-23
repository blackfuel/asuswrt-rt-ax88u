/*
 * HWA-2.0 definitions. No support for HWA-1.0.
 *
 * HWA AXI memory resident structures extracted from DV UVM scripts.
 * Layouts defined to allow BitField(BF), shift/mask and C-Datatype(CD) access.
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

#ifndef _HWA_DEFS_H
#define _HWA_DEFS_H

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Miscellaneous compile primitives and defines
 * -----------------------------------------------------------------------------
 */

#ifdef BCMHWADBG
#define HWA_DEBUG_BUILD             3           // Debug/Verbosity level
#else
#define HWA_DEBUG_BUILD             1           // Debug/Verbosity level
#endif // endif

#define HWA_STATS_BUILD                         // Enable HWA statistics

#ifdef BCMPCIEDEV                               // Build Dongle (PCIE)
#define HWA_BUS_BUILD                           // Build HWA BUS Facing blocks
/*
 * NOTE: Host should communicate "capability" to dongle, the following:
 * Host Addressing: 32 or 64 bit PCIE IPC capability (init handshake)
 * Host Coherency:  SW or HW coherency capability (init handshake)
 * PCIeIPC Format: Compact or Aggregate Format and Count (RxPost,RxCpl,TxCpl)
 */
#define HWA_SW_DRIVER_MODE          HWA_FD_MODE
#else /* ! BCMPCIEDEV */
#define HWA_SW_DRIVER_MODE          HWA_NIC_MODE
#endif /* ! BCMPCIEDEV */

// HWA MAC facing interfaces in Host DDR or SysMem
#define HWA_MACIF_NOTPCIE           (HWA_MACIF_SYSMEM)

#define HWA_NO_LUT                              // No Lkup Table support just use flowid in frc.

#define HWA_MAC_BUILD                           // Build HWA MAX Facing Blocks

#ifdef HWA_BUS_BUILD                            // ## BUS Facing
#ifdef BCMHWA1AB
#define HWA_RXPOST_BUILD                        // 1a BUS Facing RxPost Ring
#endif // endif
#if defined(BCMHWA3AB) || defined(BCMHWA3A)
#define HWA_TXPOST_BUILD                        // 3a BUS Facing TxPost Ring
#endif // endif
#ifdef BCMHWA2B
#define HWA_RXCPLE_BUILD                        // 2b BUS Facing RxCpl Ring
#endif // endif
#ifdef BCMHWA4B
#define HWA_TXCPLE_BUILD                        // 4b BUS Facing TxCpl Ring
#endif // endif
#endif /* HWA_BUS_BUILD */

#if defined(HWA_RXCPLE_BUILD) || defined(HWA_TXCPLE_BUILD)
#define HWA_CPLENG_BUILD                        // *b BUS Facing Rx|Tx Cpl Ring
#endif /* HWA_RXCPLE_BUILD || HWA_TXCPLE_BUILD */

#ifdef HWA_MAC_BUILD                            // ## MAC Facing
#ifdef BCMHWA1AB
#define HWA_RXFILL_BUILD                        // 1b MAC Facing RxFill FIFO
#endif // endif
#ifdef BCMHWA2A
#define HWA_RXDATA_BUILD                        // 2a MAC Facing RxData
#endif // endif
#ifdef BCMHWA3AB
#define HWA_TXFIFO_BUILD                        // 3b MAC Facing TxDMA AQM/FIFO
#endif // endif
#ifdef BCMHWA4A
#define HWA_TXSTAT_BUILD                        // 4a MAC Facing TxStatus
#endif // endif
#endif /* HWA_MAC_BLOCK */

#if defined(HWA_RXPOST_BUILD) && !defined(HWA_RXFILL_BUILD)
#define HWA_RXPOST_ONLY_BUILD                   // 1a ONLY
#endif /* HWA_RXPOST_BUILD && !HWA_RXFILL_BUILD */
/*
 * Register set for blocks 1a and 1b are grouped together. While it is
 * feasible to separate these blocks into their individual blocks the following
 * implementation groups them, with the intention that the three RxPath related
 * blocks, namely RxPost 1a and RxFill 1b, are all two functional
 * and the RxBM is commonly used across them two.
 */
#if defined(HWA_RXPOST_BUILD) || defined(HWA_RXFILL_BUILD)
#define HWA_RXPATH_BUILD                        // 1a, 1b and RxBM
#endif /* HWA_RXPOST_BUILD || HWA_RXFILL_BUILD */

#if defined(HWA_RXPOST_ONLY_BUILD) || defined(HWA_RXFILL_BUILD) || \
	defined(HWA_CPLENG_BUILD) || defined(HWA_TXPOST_BUILD) || defined(HWA_TXSTAT_BUILD)
#define HWA_DPC_BUILD                           // Build HWA DPC blocks
#endif // endif

/* Check if we need BCM_DHDHDR support for HWA. */
#if defined(DONGLEBUILD) && !defined(BCM_DHDHDR) && (defined(HWA_RXFILL_BUILD) || \
	defined(HWA_TXPOST_BUILD))
#error "HWA need BCM_DHDHDR support in dongle mode"
#endif // endif

// SW defined IDS, no significance to HW
#define HWA_RING_S2H                0x01
#define HWA_RING_H2S                0x02

#define HWA_RXPOST_ID               0x01
#define HWA_RXFILL_ID               0x02
#define HWA_RXDATA_ID               0x04
#define HWA_RXCPLE_ID               0x08
#define HWA_TXPOST_ID               0x10
#define HWA_TXFIFO_ID               0x20
#define HWA_TXSTAT_ID               0x40
#define HWA_TXCPLE_ID               0x80

#define HWA_NONE
#define HWA_NOOP                    do { /* noop */ } while (0)
#define HWA_FAILURE                 (-1)
#define HWA_SUCCESS                 (0)
#define HWA_INVALID                 (~0)

// HWA blocks are always Little Endian
#ifdef BCMPCIEDEV
#define HWA_DRIVER_MODE_LE
#else
// SW Driver for MAC facing blocks may be BigEndian or LittleEndian.
// assume little endian for now
#define HWA_DRIVER_MODE_LE
#endif // endif

#if defined(HWA_DRIVER_MODE_BE)
#define HWA_BE_EXPR(expr)           expr
#define HWA_LE_EXPR(expr)           HWA_NONE
#elif defined(HWA_DRIVER_MODE_LE)
#define HWA_BE_EXPR(expr)           HWA_NONE
#define HWA_LE_EXPR(expr)           expr
#else /* !(HWA_DRIVER_MODE_BE || HWA_DRIVER_MODE_LE) */
#error "Compile: setup endianess for platform!"
#endif /* !(HWA_DRIVER_MODE_BE || HWA_DRIVER_MODE_LE) */

// Embed BUS Facing declarations, expressions and code
#ifdef HWA_BUS_BUILD
#define HWA_BUS_EXPR(expr)          expr
#else  /* !HWA_BUS_BUILD */
#define HWA_BUS_EXPR(expr)          HWA_NONE
#endif /* !HWA_BUS_BUILD */

#ifdef HWA_DPC_BUILD
#define HWA_DPC_EXPR(expr)          expr
#else  /* !HWA_DPC_BUILD */
#define HWA_DPC_EXPR(expr)          HWA_NONE
#endif /* !HWA_DPC_BUILD */

typedef uintptr hwa_mem_addr_t;

#if __WORDSIZE == 64
#define HWA_PTR2HIADDR(PTR) \
({ \
	uint32 hiaddr; \
	if (sizeof(uintptr) > NBU32) \
		hiaddr = (uint64)(PTR) >> 32; \
	else \
		hiaddr = 0x00000000; \
	hiaddr; \
})
#else
#define HWA_PTR2HIADDR(PTR)         (0x00000000)
#endif // endif

#define HWA_PTR2UINT(PTR)           ((uintptr)(PTR))
/* NOTE: All second arguments to the macro HWA_UINT2PTR must be valid 64 bit values on
 * 64 bit platform
 */
#define HWA_UINT2PTR(TYPE, UINTPTR)     ((TYPE*)(UINTPTR))

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Statistics Support
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_STATS_BUILD)
#define HWA_STATS_EXPR(expr)        expr
#else  /* ! HWA_STATS_BUILD */
#define HWA_STATS_EXPR(expr)        HWA_NONE
#endif /* ! HWA_STATS_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Debug Support
 * -----------------------------------------------------------------------------
 */
#define HWA_PRINT                   printf
#define HWA_BPRINT(b, fmt, ...) ((b) ? \
	bcm_bprintf(b, fmt, ##__VA_ARGS__) : \
	HWA_PRINT(fmt, ##__VA_ARGS__))

// HWA Errors always enabled
#define HWA_ERROR(args)             HWA_PRINT args

// Debug level 1: Enable asserts and warnings
#if (HWA_DEBUG_BUILD >= 1)
#define HWA_ASSERT(expr)            ASSERT(expr)
#define HWA_WARN(args)              HWA_PRINT args
#else  /* HWA_DEBUG_BUILD < 1 */
#define HWA_ASSERT(expr)            HWA_NOOP
#define HWA_WARN(args)              HWA_NOOP
#endif /* HWA_DEBUG_BUILD < 1 */

// Debug level 2: Enable Tracing, HWA Register and Memory Accesses
#if (HWA_DEBUG_BUILD >= 2)
#define HWA_TRACE(args)             HWA_PRINT args
#define HWA_DBGREG(args)            HWA_PRINT args
#define HWA_DBGMEM(args)            HWA_PRINT args
#else  /* HWA_DEBUG_BUILD < 2 */
#define HWA_TRACE(args)             HWA_NOOP
#define HWA_DBGREG(args)            HWA_NOOP
#define HWA_DBGMEM(args)            HWA_NOOP
#endif /* HWA_DEBUG_BUILD < 2 */

// Debug level 3: Verbose packet tracing
#if (HWA_DEBUG_BUILD >= 3)
#define HWA_PTRACE(args)            HWA_PRINT args
#define HWA_FTRACE(blk)             HWA_PRINT("HWA %s %s\n", blk, __FUNCTION__)
#define HWA_DEBUG_EXPR(expr)        expr
#else  /* HWA_DEBUG_BUILD < 3 */
#define HWA_PTRACE(args)            HWA_NOOP
#define HWA_FTRACE(blk)             HWA_NOOP
#define HWA_DEBUG_EXPR(expr)        HWA_NONE
#endif /* HWA_DEBUG_BUILD < 3 */

// WL debug level
#if (HWA_DEBUG_BUILD >= 2) || defined(BCMDBG) || 0 || defined(WLTEST) || \
	defined(BCMDBG_AMPDU)
#define HWA_BCMDBG_EXPR(expr)       expr
#else
#define HWA_BCMDBG_EXPR(expr)       HWA_NONE
#endif // endif

// HWA prefixes used in debug printing
#define HWA00                       "HWA00 Common:" // Common/TOP/DMA/BM/...
#define HWAde                       "HWA00 DMAEng:" // HWA Internal DMA Engines
#define HWAbm                       "HWA00 Bufgr:"  // HWA Buffer Manager

#define HWA1a                       "HWA1a RxPOST:" // BUS Facing RxPost Ring
#define HWA3a                       "HWA3a TxPOST:" // BUS Facing TxPost Ring
#define HWA2b                       "HWA2b RxCPLE:" // BUS Facing RxCpl Engine
#define HWA4b                       "HWA4b TxCPLE:" // BUS Facing TxCpl Engine

#define HWA1x                       "HWA1x RxPATH:" // 1a and 1b common paths
#define HWAce                       "HWA*b CPLENG:" // BUS Facing Rx/Tx Cpl Ring

#define HWA1b                       "HWA1b RxFILL:" // MAC Facing RxFill FIFO
#define HWA2a                       "HWA2a RxDATA:" // MAC Facing RxData
#define HWA3b                       "HWA3b TxFIFO:" // MAC Facing TxDMA AQM/FIFO
#define HWA4a                       "HWA4a TxSTAT:" // MAC Facing TxStatus

/*
 * -----------------------------------------------------------------------------
 * Section: 43684 specific settings for HWA, CoreId = 0x851
 * -----------------------------------------------------------------------------
 */
// ARM IRQ input for HWA : OOB Bus A
#define HWA_IRQ_OOB_BUSA            6

// -----------------------------------------------------------------------------

// HWA APB Register Region : CCI400 M0 Port
#define HWA_APB_BASE_ADDR           0x28005000
#define HWA_APB_REGION_SZ           0x00000fff  // 4 KBytes
#define HWA_AXI_APB0_PORT           5

// HWA IDM Wrapper Register Region on APB1
#define HWA_IDM_BASE_ADDR           0x28105000
#define HWA_IDM_REGION_SZ           0x00000fff  // 4 KBytes

// -----------------------------------------------------------------------------

// HWA AXI Memory Region : CCI400 M0 Port
#define HWA_AXI_BASE_ADDR           0x28500000
#define HWA_AXI_REGION_SZ           0x000fffff  // 1 MByte

// HWA1a block memories offsets in AXI memory space
#define HWA_AXI_RXPOST_MEMORY       0x00001000  // HWA1a Rx Post memory

// HWA3b block memories offsets in AXI memory space
#define HWA_AXI_TXFIFO_OVFLWQS      0x00008000  // HWA3b Overflow Queue Contexts
#define HWA_AXI_TXFIFO_TXFIFOS      0x00009000  // HWA3b TxFIFO Contexts

// HWA3a block memories offsets in AXI memory space
#define HWA_AXI_TXPOST_PRIO_LUT     0x0000c000  // HWA3a Priority Lkup Table
#define HWA_AXI_TXPOST_SADA_LUT     0x0000c400  // HWA3a SADA Lkup Table
#define HWA_AXI_TXPOST_FLOW_LUT     0x0000d000  // HWA3a FlowId Lkup table
#define HWA_AXI_TXPOST_FRCT_LUT     0x0000f000  // Deleted in HWA-2.0

// HWA2b, HWA4b block memories offsets in AXI memory space
#define HWA_AXI_CPLE_RING_CONTEXTS  0x00012000  // 1 TxCpl 4 RxCpl

// -----------------------------------------------------------------------------

// MAC AXI Memory Region : CCI400 M0 Port
#define MAC_AXI_BASE_ADDR           0x28800000  // DOT11MAC region
#define MAC_AXI_REGION_SZ           0x007fffff  // 8 MByte

// MAC HWA2a register files direct access via AXI - rev128
#define MAC_AXI_RXDATA_FLOW_AGE     0x0000ce00  // Deleted in HWA-1.0
#define MAC_AXI_RXDATA_FHRTABLE     0x0000d000  // HWA2a 0xd000 - 0xefff
#define MAC_AXI_RXDATA_FHRSTATS     0x0000f000  // HWA2a 0xf000 - 0xf0ff

// Use HC_HIN_REGS::ObjAddr(0x160) and ObjData(0x164) for indirect access
// Use Indirect Access via ObjAddr/ObjData to write RxData FHR register file
#define HWA_RXDATA_FHR_IND_BUILD

// See HWA_RXDATA_FHR_IND_BUILD hwa_rxdata_fhr_indirect_write()
#define MAC_AXI_RXDATA_FHR_SELECT   0xe        // Indirect Access to FHR
#define MAC_AXI_RXDATA_FHR_RF(idx)  ((idx) << 5) // bits 9:5 in ObjAddr

// -----------------------------------------------------------------------------

// HWA DMA engines access to host memory
#define HWA_PCI64ADDR_HI32          0x80000000      // address[63 ]
#define HWA_HOSTADDR64_HI32(addr)   ((addr) | HWA_PCI64ADDR_HI32)

#ifndef HWA_ETHER_ADDR_LEN
#define HWA_ETHER_ADDR_LEN          6 /* Length of a 802.3 Ethernet Address */

typedef union hwa_txpost_eth_sada
{
	uint8   u8[HWA_ETHER_ADDR_LEN * 2];
	uint32 u32[(HWA_ETHER_ADDR_LEN * 2) / NBU32];
} hwa_txpost_eth_sada_t;
#endif /* ! HWA_ETHER_ADDR_LEN */

/*
 * -----------------------------------------------------------------------------
 * Section: 43684 HWA Generics
 * -----------------------------------------------------------------------------
 */
#define HWA_CHIP_GENERIC            0x43684

/*
 * Blks present bit settings - not explicitly exposed in generate top_reg_defs
 *
 * b9:8     rsdb=0,     rxptfr=1,
 * b7:4     rxfill=1,   cpl=1,      txsts=1,    txdata=1,
 * b3:0     txdma=1,    stats=1     sgl_rxp=1   sgl_freering=1
 */
#define HWA_BLKS_PRESENT            0x1ff

#ifndef HWA_AGGR_MAX
#define HWA_AGGR_MAX                4 /* max aggregation factor in a ACWI */
#endif // endif

#define HWA_TX_CORES                1           // No support for RSDB - TBD
#define HWA_RX_CORES                1           // No support for RSDB - TBD
#define HWA_TX_FIFOS                70          // Number of AQM and TxFIFOs
#define HWA_OVFLWQ_MAX              HWA_TX_FIFOS

#define HWA_STATIONS_MAX            128         // Maximum number of stations

// Total interfaces supported 16 = [0..15]. ifid = 16 used as override in LUTs
#define HWA_INTERFACES_TOT          16          // Maximum number of interfaces
#define HWA_INTERFACES_MAX          (HWA_INTERFACES_TOT + 1)

// Total packet priorities 8 = [0..7]. prio = 8 used as override in LUTs
#define HWA_PRIORITIES_TOT          8           // Maximum packet priorities
#define HWA_PRIORITIES_MAX          (HWA_PRIORITIES_TOT + 1)

// HWA1a RxPost, 1b RxFill
#define HWA_RXPOST_RINGS_MAX        1           // Maximum number of RxPost ring

#define HWA_RXPOST_MEM_ADDR_W       4
#define HWA_RXPOST_LOCAL_WORKITEMS  32
// HWA-1.0 version RxPost uses bcmmsgbuf.h spec : H2DRING_RXPOST_ITEMSIZE
#define HWA_RXPOST_WORKITEM_SIZE    32
#define HWA_RXPOST_LOCAL_MEM_DEPTH  \
	((HWA_RXPOST_LOCAL_WORKITEMS * HWA_RXPOST_WORKITEM_SIZE) \
	 / HWA_RXPOST_MEM_ADDR_W)                   // 1024 = ((32 x 32) / 4) = 256

//The HWA_RXPATH_PKTS_MAX will be same as RXFRAG_POOL_LEN
#ifndef HWA_RXPATH_PKTS_MAX
#define HWA_RXPATH_PKTS_MAX         2048        // RX BM: 2^6 x 32b = 2K
#endif // endif

// HWA2a RxData
#define HWA_RXDATA_FHR_FILTERS_MAX  32          // Max 2a Filters in FHR
#define HWA_RXDATA_FHR_PARAMS_MAX   6           // Max 2a Params per FHR Filter
// SW doesnt program more than 16 filters in HWA 2.a block .
// To support fifo-3 phyrxtstaus, 16 bits from the filtermap were given away.
#define HWA_RXDATA_FHR_FILTERS_SW   16

// HWA3a TxPost
#if defined(HWA_TXPOST_BUILD)
#define HWA_TXPOST_FLOWRINGS_MAX	BCMPCIE_MAX_TX_FLOWS
#endif // endif

#define HWA_TXPOST_MEM_ADDR_W       4
// DON"T CHANGE HWA_TXPOST_LOCAL_WORKITEMS and
// HWA_TXPOST_WORKITEM_SIZE HW DEFAULT SETTING
// 1536B Local memory for 128 legacy TxPost workitems of size 48B = 12[4B words]
#define HWA_TXPOST_LOCAL_WORKITEMS  128         // 128 = 1536 / 12[4B words]
// HWA-1.0 version TxPost uses bcmmsgbuf.h spec : H2DRING_TXPOST_ITEMSIZE
#define HWA_TXPOST_WORKITEM_SIZE    48
#define HWA_TXPOST_LOCAL_MEM_DEPTH \
	((HWA_TXPOST_LOCAL_WORKITEMS * HWA_TXPOST_WORKITEM_SIZE) \
	/ HWA_TXPOST_MEM_ADDR_W)

//NOTE: We don't need to use LUT.
#if !defined(HWA_NO_LUT)

#define HWA_TXPOST_SADA_LUT_DEPTH   32

#define HWA_TXPOST_FLOW_LUT_DEPTH \
	(HWA_STATIONS_MAX + HWA_INTERFACES_TOT)     // 144 = 128 + 16
#endif /* !HWA_NO_LUT */

#ifndef HWA_TXPATH_PKTS_MAX
#define HWA_TXPATH_PKTS_MAX         8192        // TX BM: (2^8) x 32b = 8K
#endif // endif
#define HWA_TXPATH_PKTS_MAX_CAP     8192        // TX BM: (2^8) x 32b = 8K

#define HWA_TXPOST_FRC_SRS_MAX      16

#define HWA_TX_DATABUF_SEGCNT_MAX   1           // databuf is contiguous

// Chicken feature: PktInfo, CacheInfo, RateInfo is deleted in HWA2.0
#define HWA_TXFIFO_CHICKEN_FEATURE  0

// HWA2b, HWA4b  CplEng
#define HWA_CPLENG_CED_QUEUES_MAX   5           // 1 TxCpl + 4 RxCpl

// HWA3a along with iDMA uses 2B indices ref. bcmpcie.h
#define HWA_PCIE_RW_INDEX_SZ        PCIE_RW_INDEX_SZ

#define HWA_LOOPCNT                 32          // max SW loops on alloc request

#define HWA_STALL_DELAY             1024        // In usecs

#define HWA_DMA_CHANNEL0            0           // HWA internal DMA channels
#define HWA_DMA_CHANNEL1            1           // HWA internal DMA channels (Pcie face)
#define HWA_DMA_CHANNELS_MAX        (HWA_DMA_CHANNEL1 + 1) // HWA internal DMA channels

// Tx and Rx BM.                                   2bX0 dealloc pending
#define HWA_BM_SUCCESS_SW           1           // 2b01
#define HWA_BM_DONEBIT              2           // 2b1X
#define HWA_BM_SUCCESS              2           // 2b10
#define HWA_BM_FAILURE              3           // 2b11
#define HWA_BM_LOOPCNT              HWA_LOOPCNT // SW loops until transaction

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Register Access in APB space. All accesses are 32bit wide.
 * -----------------------------------------------------------------------------
 *
 * Usage example: reference hwa_regs_t in hwa_regs.h
 *
 *   my_val = HWA_RD_REG_NAME(HWA00, hwa_dev->regs, top, hwahwcap2);
 *
 *   HWA_WR_REG_NAME(HWA3a, hwa_dev->regs, tx, fw_cmdq_base_addr, (uint32)addr);
 *
 * -----------------------------------------------------------------------------
 */
typedef volatile uint32 *hwa_reg_addr_t; // 32bit or 64bit address

#define HWA_RD_REG_NAME(HWA_NAME, HWA_REGS, STRUCT, MEMBER) \
({ \
	uint32 val32; \
	STATIC_ASSERT(sizeof((HWA_REGS)->STRUCT.MEMBER) == sizeof(uint32)); \
	val32 = R_REG(NULL, &((HWA_REGS)->STRUCT.MEMBER)); \
	HWA_DBGREG(("HWA %s RD_REG[%p] val32<0x%08x,%u> %s::%s\n", HWA_NAME, \
		&(HWA_REGS)->STRUCT.MEMBER, val32, val32, #STRUCT, #MEMBER)); \
	val32; \
})

#define HWA_RD_REG_ADDR(HWA_NAME, HWA_REG_ADDR) \
({ \
	uint32 val32; \
	val32 = R_REG(NULL, (volatile uint32 *)(HWA_REG_ADDR)); \
	HWA_DBGREG(("HWA %s RD_REG[%p] val32<0x%08x,%u>\n", \
		HWA_NAME, (HWA_REG_ADDR), val32, val32)); \
	val32; \
})

#define HWA_RD_REG16_ADDR(HWA_NAME, HWA_REG_ADDR) \
({ \
	uint16 val16; \
	val16 = R_REG(NULL, (volatile uint16 *)(HWA_REG_ADDR)); \
	HWA_DBGREG(("HWA %s RD_REG[%p] val16<0x%08x,%u>\n", \
		HWA_NAME, (HWA_REG_ADDR), val16, val16)); \
	val16; \
})

#define HWA_WREG32_OSL(a, v) \
	({STATIC_ASSERT(sizeof(*(a)) == sizeof(uint32)); W_REG(NULL, (a), (v));})

#define HWA_WREG16_OSL(a, v) \
	({STATIC_ASSERT(sizeof(*(a)) == sizeof(uint16)); W_REG(NULL, (a), (v));})

#define HWA_WR_REG_NAME(HWA_NAME, HWA_REGS, STRUCT, MEMBER, VAL32) \
({ \
	HWA_DBGREG(("HWA %s WR_REG[%p] val32<0x%08x,%u> %s::%s\n", HWA_NAME, \
		&(HWA_REGS)->STRUCT.MEMBER, (VAL32), (VAL32), #STRUCT, #MEMBER)); \
	STATIC_ASSERT(sizeof((HWA_REGS)->STRUCT.MEMBER) == sizeof(uint32)); \
	W_REG(NULL, &(HWA_REGS)->STRUCT.MEMBER, (VAL32)); \
})

#define HWA_WR_REG_ADDR(HWA_NAME, HWA_REG_ADDR, VAL32) \
({ \
	HWA_DBGREG(("HWA %s WR_REG[%p] val32<0x%08x,%u>\n", \
		HWA_NAME, (HWA_REG_ADDR), (VAL32), (VAL32))); \
	W_REG(NULL, (volatile uint32 *)(HWA_REG_ADDR), (VAL32)); \
})

#define HWA_WR_REG16_NAME(HWA_NAME, HWA_REGS, STRUCT, MEMBER, VAL16) \
({ \
	HWA_DBGREG(("HWA %s WR_REG[%p] val16<0x%08x,%u> %s::%s\n", HWA_NAME, \
		&(HWA_REGS)->STRUCT.MEMBER, (VAL16), (VAL16), #STRUCT, #MEMBER)); \
	HWA_WREG16_OSL(&(HWA_REGS)->STRUCT.MEMBER, (VAL16)); \
})

#define HWA_WR_REG16_ADDR(HWA_NAME, HWA_REG_ADDR, VAL16) \
({ \
	HWA_DBGREG(("HWA %s WR_REG[%p] val16<0x%08x,%u>\n", \
		HWA_NAME, (HWA_REG_ADDR), (VAL16), (VAL16))); \
	W_REG(NULL, (volatile uint16 *)(HWA_REG_ADDR), (VAL16)); \
})

// No debug trace - for use in dump functions
#define HWA_RD_REG(HWA_REG_ADDR)        R_REG(NULL, (volatile uint32 *)(HWA_REG_ADDR))
#define HWA_WR_REG(HWA_REG_ADDR, VAL32) W_REG(NULL, (volatile uint32 *)(HWA_REG_ADDR), (VAL32))

#if defined(WLTEST)
#define _HWA_PR_REG(BUF, STRUCT, MEMBER) \
({ \
	uint32 val32 = HWA_RD_REG(&hwa_dev->regs->STRUCT.MEMBER); \
	uintptr off = (uintptr)(&hwa_dev->regs->STRUCT.MEMBER) \
	              - (uintptr)(&hwa_dev->regs->STRUCT); \
	HWA_BPRINT(BUF, "\t0x%04x [0x%08x, %10u] %s\n", \
		 (int)off, val32, val32, #MEMBER); \
	val32; \
})
#define HWA_PR_REG(STRUCT, MEMBER)  _HWA_PR_REG(NULL, STRUCT, MEMBER)
#define HWA_BPR_REG(b, s, m)	_HWA_PR_REG(b, s, m)
#else  /* ! BCMINTERNAL */
#define HWA_PR_REG(STRUCT, MEMBER)   HWA_NOOP
#define HWA_BPR_REG(b, s, m)   HWA_NOOP
#endif // endif

/*
 * -----------------------------------------------------------------------------
 * Section: HWA Memory Access in AXI space. All accesses are 32bit wide.
 * -----------------------------------------------------------------------------
 *
 * Usage example:
 *    Write an entire hwa_txpost_flow into AXI flow_lut_mem index 10
 *
 *    HWA_WR_MEM32(HWA3a, hwa_txpost_flow_t, &hwa_txpost->flow_addr[10], &flow);
 *
 * -----------------------------------------------------------------------------
 */

// What is frontdoor/backdoor access? see hwa_axi_addr()
#if HWA_REVISION_EQ_128
#define HWA_RD_MEM32(HWA_NAME, STRUCT, HWA_MEM, SYS_MEM) \
({ \
	int i; \
	volatile uint32 *src_p = HWA_UINT2PTR(uint32, HWA_MEM); \
	uint32 *dst_p = (uint32 *)(SYS_MEM); \
	for (i = 0; i < sizeof(STRUCT) / NBU32; i++) { \
		*(dst_p + i) = *(src_p + i); \
 \
		*(dst_p + i) = *(src_p + i); \
		HWA_DBGREG(("HWA %s RD_MEM[%p]<0x%08x> val32<0x%08x,%u> %s[%d]\n", HWA_NAME, \
			dst_p + i, *(dst_p + i), *(src_p + i), *(src_p + i), #STRUCT, i)); \
	} \
})
#else
#define HWA_RD_MEM32(HWA_NAME, STRUCT, HWA_MEM, SYS_MEM) \
({ \
	int i; \
	volatile uint32 *src_p = HWA_UINT2PTR(uint32, HWA_MEM); \
	uint32 *dst_p = (uint32 *)(SYS_MEM); \
	for (i = 0; i < sizeof(STRUCT) / NBU32; i++) { \
		*(dst_p + i) = *(src_p + i); /* AXI Access  */ \
		HWA_DBGREG(("HWA %s RD_MEM[%p]<0x%08x> val32<0x%08x,%u> %s[%d]\n", HWA_NAME, \
			dst_p + i, *(dst_p + i), *(src_p + i), *(src_p + i), #STRUCT, i)); \
	} \
})
#endif /* HWA_REVISION_EQ_128 */

#define HWA_WR_MEM32(HWA_NAME, STRUCT, HWA_MEM, SYS_MEM) \
({ \
	int i; \
	uint32 *src_p = (uint32 *)(SYS_MEM); \
	volatile uint32 *dst_p = HWA_UINT2PTR(uint32, HWA_MEM); \
	for (i = 0; i < sizeof(STRUCT) / NBU32; i++) { \
		*(dst_p + i) = *(src_p + i); \
		HWA_DBGREG(("HWA %s WR_MEM[%p] val32<0x%08x,%u> %s[%d]\n", HWA_NAME, \
			dst_p + i, *(src_p + i), *(src_p + i), #STRUCT, i)); \
	} \
})

#define HWA_RD_MEM16(HWA_NAME, STRUCT, HWA_MEM, SYS_MEM) \
({ \
	int i; \
	volatile uint16 *src_p = HWA_UINT2PTR(uint16, HWA_MEM); \
	uint16 *dst_p = (uint16 *)(SYS_MEM); \
	for (i = 0; i < sizeof(STRUCT) / sizeof(uint16); i++) { \
		*(dst_p + i) = *(src_p + i); \
		HWA_DBGREG(("HWA %s RD_MEM[%p] val16<0x%04x,%u> %s[%d]\n", HWA_NAME, \
			dst_p + i, *(src_p + i), *(src_p + i), #STRUCT, i)); \
	} \
})

#define HWA_WR_MEM16(HWA_NAME, STRUCT, HWA_MEM, SYS_MEM) \
({ \
	int i; \
	uint16 *src_p = (uint16 *)(SYS_MEM); \
	volatile uint16 *dst_p = HWA_UINT2PTR(uint16, HWA_MEM); \
	for (i = 0; i < sizeof(STRUCT) / sizeof(uint16); i++) { \
		*(dst_p + i) = *(src_p + i); \
		HWA_DBGREG(("HWA %s WR_MEM[%p] val16<0x%04x,%u> %s[%d]\n", HWA_NAME, \
			dst_p + i, *(src_p + i), *(src_p + i), #STRUCT, i)); \
	} \
})

/*
 * -----------------------------------------------------------------------------
 * Section: HWA SW driver defines and structures
 * SW driver stuff common to multiple blocks, eg HWA DMA, BM, etc goes here.
 * -----------------------------------------------------------------------------
 */

#ifdef HWA_RXFILL_BUILD
/*
 *
 * HWA RX BM is constructed of buffers of size 256 Bytes, also referred to as
 * d11buffers in HWA spec.
 *
 * In Full Dongle, this RxBuffer will be posted to FIFO1 and includes RPH
 * RPH (RxPost Host Info) = hwa_rxpost_hostinfo32_t or hwa_rxpost_hostinfo64_t
 *
 * HWA parses the RPH to fetch the address and length to be programmed in the
 * descriptor of FIFO0. In the case when HWA 1a and 1b are enabled, HWA is
 * responsible for constructing the RPH from a RxPost workitem, using a
 * RxPost Workitem to RPH compression specification. For paired RxBuffers freed
 * via the "FREEIDXSRC" S2H Interface, HWA1b will appropriately parse the RPH.
 *
 */

/* Use MAXPKTRXFRAGSZ for the RX BM buffer size. Must be 8 bytes aligned.
 * 43684A0 is 512 bytes.
 * 43684B0 is 360 bytes.
 */
#define HWA_RXBUFFER_WORDS          (HWA_RXBUFFER_BYTES / NBU32)
#define HWA_RXBUFFER_BYTES          ((MAXPKTRXFRAGSZ + (8 - 1)) & ~ (8 - 1))

typedef union hwa_rxbuffer
{
	uint8   u8[HWA_RXBUFFER_BYTES];           // 256 Bytes
	uint32 u32[HWA_RXBUFFER_WORDS];           //  64 Words
} hwa_rxbuffer_t;
#endif /* HWA_RXFILL_BUILD */

// HWA Statistics Management

#define HWA_BLOCK_STATISTICS_WORDS  32    // per block 32 max 32b statistics
#define HWA_BLOCK_STATISTICS_BYTES  (HWA_BLOCK_STATISTICS_WORDS * NBU32)

typedef enum hwa_stats_set_index
{
	HWA_STATS_RXPOST_CORE0      =  0,
	HWA_STATS_RXPOST_CORE1      =  1,
	HWA_STATS_CPLENG_COMMON     =  2,
	HWA_STATS_CPLENG_CEDQ       =  3, // indices  3 .. 12, for 10 rings
	HWA_STATS_CPLENG_CEDQ_LAST  = 12,
	HWA_STATS_TXDMA             = 13,
	HWA_STATS_TXSTS_CORE0       = 14,
	HWA_STATS_TXSTS_CORE1       = 15,
	HWA_STATS_TXPOST_COMMON     = 16,
	HWA_STATS_TXPOST_RING       = 17, // indices 17 .. 48, for 32 rings
	HWA_STATS_TXPOST_RING_LAST  = 48,
	HWA_STATS_SET_INDEX_MAX     = 49
} hwa_stats_set_index_t;

typedef union hwa_stats_control
{
	uint32 u32;
	struct {
		uint32 stats_set_index  : 10;   // see hwa_stats_set_index_t
		uint32 clear_command    :  1;   // clear HWA managed stats
		uint32 copy_command     :  1;   // copy out to SW state
		uint32 num_sets         : 10;   // number of sets to copy
#ifdef HWA_TXPOST_BUILD
		uint32 update_command   :  1;   // SW busy with FRC update
#else
		uint32 PAD              :  1;
#endif // endif
		uint32 notpcie          :  1;   // Used in DMA descr for stats xfer
		uint32 coherent         :  1;
		uint32 addrext          :  2;
		uint32 hwa_notpcie      :  1;
		uint32 PAD              :  3;
	};
} hwa_stats_control_t;

/*
 * -----------------------------------------------------------------------------
 * Section: HWA RxPost Block 1a
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_RXPOST_BUILD)
#define HWA_RXPOST_EXPR(expr)       expr
#else  /* ! HWA_RXPOST_BUILD */
#define HWA_RXPOST_EXPR(expr)       HWA_NONE
#endif /* ! HWA_RXPOST_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA1a RxPost Block is built and HWA1b RxFill Block is not built
 * -----------------------------------------------------------------------------
 */
#ifdef HWA_RXPOST_ONLY_BUILD
#define HWA_RXPOST_ONLY_EXPR(expr)  expr
#else /* ! HWA_RXPOST_ONLY_BUILD */
#define HWA_RXPOST_ONLY_EXPR(expr)  HWA_NONE
#endif /* ! HWA_RXPOST_ONLY_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA1b RxFill Block
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_RXFILL_BUILD)
#define HWA_RXFILL_EXPR(expr)       expr

// Rx frees audit
//#define HWA_RXFILL_RXFREE_AUDIT_ENABLED

// SW frees simple or paired RxBuffers to HWA for posting to RxFIFO, via FREEIDX
#define HWA_RXFILL_RXFREE_PAIRED    0
#define HWA_RXFILL_RXFREE_SIMPLE    1

// After rev129, HWA can support both 64bits or 32bits, (nic64_1b control it)
// In FD mode, we keep using 32bits mode.
#if HWA_REVISION_GE_129 && !defined(BCMPCIEDEV)
#define HWA_RXFILL_RXFREE_WORDS     2
#else
#define HWA_RXFILL_RXFREE_WORDS     1
#endif // endif
#define HWA_RXFILL_RXFREE_BYTES     (HWA_RXFILL_RXFREE_WORDS * NBU32)

typedef union hwa_rxfill_rxfree               // aka FREEIDXSRC S2H Interface
{
	uint8   u8[HWA_RXFILL_RXFREE_BYTES];      //  4 Bytes
	uint32 u32[HWA_RXFILL_RXFREE_WORDS];      //  1 Words

	struct {
		uint16 index;                         // +----- word#0
		uint8  control_info;                  // paired, simple
		uint8  PAD;
	};
#if HWA_REVISION_GE_129 && !defined(BCMPCIEDEV)
	struct {                       // +----- word #0 #1
		union {
			uint32 RSVD_FIELD       : 2;
			uint32 haddr_lsb30      : 30;
		};
		uint32 haddr_msb32;
	};
#endif // endif
} hwa_rxfill_rxfree_t;

// HWA1b provides sequence of RxBufIndices posted to MAC Rx FIFO0 / FIFO1
// After rev129, HWA can support both 64bits or 32bits, (nic64_1b control it)
// In FD mode, we keep using 32bits mode.
#if HWA_REVISION_GE_129 && !defined(BCMPCIEDEV)
#define HWA_RXFILL_RXFIFO_WORDS     2
#else
#define HWA_RXFILL_RXFIFO_WORDS     1
#endif // endif
#define HWA_RXFILL_RXFIFO_BYTES     (HWA_RXFILL_RXFIFO_WORDS * NBU32)

typedef union hwa_rxfill_rxfifo               // aka D11BDEST H2S Interface
{
	uint8   u8[HWA_RXFILL_RXFIFO_BYTES];      //  4 Bytes
	uint32 u32[HWA_RXFILL_RXFIFO_WORDS];      //  1 Words

	struct {
		uint16 index;                         // +----- word#0 lsb16
		uint16 PAD;
	};
#if HWA_REVISION_GE_129 && !defined(BCMPCIEDEV)
	struct {                       // +----- word #0 #1
		union {
			uint32 RSVD_FIELD       : 2;
			uint32 haddr_lsb30      : 30;
		};
		uint32 haddr_msb32;
	};
#endif // endif
} hwa_rxfill_rxfifo_t;

#else  /* ! HWA_RXFILL_BUILD */
#define HWA_RXFILL_EXPR(expr)       HWA_NONE
#endif /* ! HWA_RXFILL_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA RxPATH functionality common to HWA1a and HWA1b, and HWA2a
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_RXPATH_BUILD)
#define HWA_RXPATH_EXPR(expr)       expr

// 1a and 1b block level common statistics
typedef union hwa_rxpath_stats
{
	struct {
		uint32 num_rxpost_wi;                 // RxPost workitems fetched
		uint32 num_rxpost_dma;                // RxPost mem2mem DMAs performed
		uint32 num_fifo_descr;                // FIFO descr created
		uint16 num_fifo_empty;                // FIFO queues empty occurrences
		uint16 num_stalls_d11b;               // H2S d11b index ring full
		uint16 num_stalls_bm;                 // Rx BM depleted occurrences
		uint16 num_stalls_dma;                // H2D mem2mem DMA busy
		uint32 num_d2h_rd_upd;                // D2H RD doorbell INTs generated
		uint32 num_d11b_allocs;               // Rx BM allocations
		uint32 num_d11b_frees;                // Rx BM de-allocations
		uint16 dur_bm_empty;                  // duation BM depleted
		uint16 dur_dma_busy;                  // duration DMA busy
	};
	uint32 u32[HWA_BLOCK_STATISTICS_WORDS];
} hwa_rxpath_stats_t;

// RxPost Host Info is used by both 1a and 1b
#define HWA_RXPOST_HOSTINFO32_WORDS 2
#define HWA_RXPOST_HOSTINFO32_BYTES (HWA_RXPOST_HOSTINFO32_WORDS * NBU32)

typedef union hwa_rxpost_hostinfo32
{
	uint8   u8[HWA_RXPOST_HOSTINFO32_BYTES];  //  8 Bytes
	uint32 u32[HWA_RXPOST_HOSTINFO32_WORDS];  //  2 Words

	struct {
		uint32 host_pktid;                    // +----- word#0
		uint32 data_buf_haddr32;              // +----- word#1
	};
} hwa_rxpost_hostinfo32_t;

#define HWA_RXPOST_HOSTINFO64_WORDS 3
#define HWA_RXPOST_HOSTINFO64_BYTES (HWA_RXPOST_HOSTINFO64_WORDS * NBU32)

typedef union hwa_rxpost_hostinfo64
{
	uint8   u8[HWA_RXPOST_HOSTINFO64_BYTES];  // 12 Bytes
	uint32 u32[HWA_RXPOST_HOSTINFO64_WORDS];  //  3 Words

	struct {
		uint32 host_pktid;                    // +----- word#0
		dma64addr_t data_buf_haddr64;         // +----- word#1 word#2
	};
} hwa_rxpost_hostinfo64_t;

typedef union hwa_rxpost_hostinfo
{
	hwa_rxpost_hostinfo32_t hostinfo32;
	hwa_rxpost_hostinfo64_t hostinfo64;
} hwa_rxpost_hostinfo_t;

#else  /* ! HWA_RXPATH_BUILD */
#define HWA_RXPATH_EXPR(expr)       HWA_NONE
#endif /* ! HWA_RXPATH_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA2a RxData Block
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_RXDATA_BUILD)
#define HWA_RXDATA_EXPR(expr)       expr

// HWA2a RxData FHR Filter Match 32 bit result byte offset in RxStatus
// CAUTION 32 bit filtermap is NOT on a 32 bit boundary.
#define HWA_RXDATA_RXSTATUS_FILTERMAP_OFFSET  10

// HWA2a Packet Classification Difference map byte offset in RxStatus
#define HWA_RXDATA_RXSTATUS_PKTCLASS_OFFSET   14

// HWA2a RxData AMT based Flow Lookup FLOWID byte offset in RxStatus
#define HWA_RXDATA_RXSTATUS_FLOWID_OFFSET     16

// Layout of a 16bit Packet Comparison classification result in RxStatus
typedef union hwa_rxdata_pktclass
{
	uint16 diffmap;
	struct {
		uint16 flowid_diff          : 1;      // FlowId check for valid FlowId
		uint16 A1_diff              : 1;      // A1  check for all frames
		uint16 A2_diff              : 1;      // A2  check for all frames
		uint16 A3_diff              : 1;      // A3  check for all frames
		uint16 tid_diff             : 1;      // TID check for QOS frames
		uint16 epoch_diff           : 1;      // AMPDU boundary - epoch toggle
		uint16 amsdu_da_diff        : 1;      // DA  check for all amsdu frames
		uint16 amsdu_sa_diff        : 1;      // SA  check for all amsdu frames
		uint16 fc_diff              : 1;      // FC  check for all frames
		uint16 PAD                  : 7;
	};
} hwa_rxdata_pktclass_t;

// Layout of a 16bit Flowid in RxStatus
typedef union hwa_rxdata_flowid
{
	uint16 u16[1];
	struct {
		uint16 flowid   : 11; // 12 bit flowid
		uint16 new_flow :  1; // new/evict for HWA managed LUT - discontinued
		uint16 PAD      :  4;
	};
} hwa_rxdata_flowid_t;

// Layout of a MAC RxStatus
typedef struct hwa_rxdata_rxstatus
{
	struct {
		uint16 rxfrmsz;      // size of received fram in bytes
		uint16 dmaflags;     // last msdu; hw, ucode, phy status validity
		uint16 mrxs0;        // amsdu, agg type, pad, hdr conv, msdu counter
		uint16 rxfrmsz0;     // hdrconv ? rcvhdrconvsts : rcvfifo0len
		uint16 hdrsts;       // rcv_hdr_ctlsts
		uint16 filtermap[2]; // CAUTION: 32 bit filtermap is NOT uint32 aligned
		uint16 pktclass;     // Packet similarity classification
		uint16 flowid;       // MAC AMT based Flow Lookup ID
		uint16 errflags;     // HW Rx path error flags
	} hw; // 20 Bytes
	struct {
		uint16 mrxs1;
		uint16 mrxs2;
		uint16 rxchan;
		uint16 avbrxtimel;
		uint16 avbrxtimeh;
		uint16 rxtsftml;
		uint16 rxtsftmh;
		uint16 murate;
	} ucode; // 16 Bytes
	struct {
		uint16 prxs[16];
	} phy;
} hwa_rxdata_rxstatus_t;

static INLINE uint32 // HWA2a RxData 32 bit filtermap is not 32 bit word aligned
hwa_rxdata_rxstatus_filtermap(hwa_rxdata_rxstatus_t *rxstatus)
{
	union { uint16 u16[2]; uint32 u32; } fmap;
	fmap.u16[0] = rxstatus->hw.filtermap[0];
	fmap.u16[1] = rxstatus->hw.filtermap[1];
	return fmap.u32; // no endian conversion ...
}

// Each filter's bitmask and pattern is 64 bits wide
#define HWA_RXDATA_FHR_PATTERN_WORDS    2     // 64 bits wide
#define HWA_RXDATA_FHR_PATTERN_BYTES    (HWA_RXDATA_FHR_PATTERN_WORDS * NBU32)

// A filter's parameter has a 32bit configuration, and a bitmask and patter.
#define HWA_RXDATA_FHR_PARAM_WORDS  5
#define HWA_RXDATA_FHR_PARAM_BYTES  (HWA_RXDATA_FHR_PARAM_WORDS * NBU32)

typedef union hwa_rxdata_fhr_param
{
	uint8   u8[HWA_RXDATA_FHR_PARAM_BYTES];   // 20 Bytes
	uint32 u32[HWA_RXDATA_FHR_PARAM_WORDS];   // (1 + 2 + 2) = 5 Words

	struct {
		struct {
			uint32 polarity         :  1;     // +----- word#0
			uint32 PAD              :  7;
			uint32 offset           : 11;
			uint32 PAD              : 13;
		} config;

		uint8 bitmask[HWA_RXDATA_FHR_PATTERN_BYTES]; // word#1, #2 = 64 bits
		uint8 pattern[HWA_RXDATA_FHR_PATTERN_BYTES]; // word#3, #4 = 64 bits
	};

} hwa_rxdata_fhr_param_t;

// Each filter has a 32bit configuration, and upto 6 parameters.
#define HWA_RXDATA_FHR_ENTRY_WORDS \
	(1 + (HWA_RXDATA_FHR_PARAM_WORDS * HWA_RXDATA_FHR_PARAMS_MAX) +1)
#define HWA_RXDATA_FHR_ENTRY_BYTES  (HWA_RXDATA_FHR_ENTRY_WORDS * NBU32)

// Layout of a Filter Hardware Resource' Filter in MAC memory
typedef union hwa_rxdata_fhr_entry
{
	uint8   u8[HWA_RXDATA_FHR_ENTRY_BYTES];   // 128 Bytes
	uint32 u32[HWA_RXDATA_FHR_ENTRY_WORDS];   //  32 Words

	struct {
		struct {
			uint32 id               :  8;     // +----- word#0
			uint32 polarity         :  1;
			uint32 type             :  3;     // SW use only, RSVD in HWA
			uint32 param_count      :  4;
			uint32 PAD              : 16;
		} config;                             // filter configuration

		hwa_rxdata_fhr_param_t params[HWA_RXDATA_FHR_PARAMS_MAX];
		uint32 rsvd;
	};
} hwa_rxdata_fhr_entry_t;

#define HWA_RXDATA_FHR_WORDS \
	(HWA_RXDATA_FHR_ENTRY_WORDS * HWA_RXDATA_FHR_FILTERS_MAX)
#define HWA_RXDATA_FHR_BYTES        (HWA_RXDATA_FHR_WORDS * NBU32)

// Layout of a FHR filter in MAC memory
typedef union hwa_rxdata_fhr
{
	uint8   u8[HWA_RXDATA_FHR_BYTES];
	uint32 u32[HWA_RXDATA_FHR_WORDS];

	hwa_rxdata_fhr_entry_t fhr[HWA_RXDATA_FHR_FILTERS_MAX];
} hwa_rxdata_fhr_t;

// Layout of FHR statistics in MAC memory
typedef uint32 hwa_rxdata_fhr_stats_entry_t;
typedef struct hwa_rxdata_fhr_stats
{
	hwa_rxdata_fhr_stats_entry_t fhr_stats[HWA_RXDATA_FHR_FILTERS_MAX];
} hwa_rxdata_fhr_stats_t;

#else  /* ! HWA_RXDATA_BUILD */
#define HWA_RXDATA_EXPR(expr)       HWA_NONE
#endif /* ! HWA_RXDATA_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA3a TxPost Block
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_TXPOST_BUILD)
#define HWA_TXPOST_EXPR(expr)       expr

// Enable bzero pkttag by HWA support
#define HWA_TXPOST_BZERO_PKTTAG

// IPv4 and IPv6 EtherTypes
#define HWA_TXPOST_ETHTYPE_IPV4     0x0800
#define HWA_TXPOST_ETHTYPE_IPV6     0x86dd

// Flowid is a 12bit entity in HWA
#define HWA_TXPOST_FLOWID_MASK      0xfff

#define HWA_TXPOST_FLOWID_NULL      0x000
#define HWA_TXPOST_FLOWID_INVALID   0xfff

// TxPost Flowrings RD and WR indices begin after H2D Common Rings
#define HWA_TXPOST_FLOWRING_RDWR_BASE_OFFSET \
	(HWA_PCIE_RW_INDEX_SZ * BCMPCIE_H2D_MSGRING_TXFLOW_IDX_START)

// TxPost Flowring's "ring_id" start from ring_id = 2 ref. bcmpcie.h
#define HWA_TXPOST_FLOWRINGID_2_FRCID(ring_id) \
({ \
	uint32 u32_frc_id = ((ring_id) - BCMPCIE_H2D_COMMON_MSGRINGS); \
	HWA_ASSERT(u32_frc_id < HWA_TXPOST_FLOWRINGS_MAX); \
	u32_frc_id; \
})

#define HWA_TXPOST_FRCID_2_FLOWRINGID(frc_id) \
({ \
	HWA_ASSERT(frc_id < HWA_TXPOST_FLOWRINGS_MAX); \
	((frc_id) + BCMPCIE_H2D_COMMON_MSGRINGS); \
})

// 48bit timestamp in HWA ... see hwa_txpost_pktchain_t
typedef union hwa_timestamp48 {
	uint64 u64;
	struct {
		uint32 lsb32;                           // timestamp lower 32b
		uint16 msb16;                           // timestamp upper 16b
		uint16 RSVD_FIELD;
	};
} hwa_timestamp48_t;

typedef union hwa_txpost_status_reg
{
	uint32 u32;
	struct {
		uint32 auditfail_ethtype  :  1;       // IP ethertype audit
		uint32 auditfail_pyldlen  :  1;       // pkt payload length audit
		uint32 auditfail_pyldaddr :  1;       // pkt payload addr audit
		uint32 auditfail_md_addr  :  1;       // metadat addr audit
		uint32 auditfail_ifid     :  1;       // ifid(FRC != WI)
		uint32 auditfail_phasebit :  1;       // phasebit(schedcmd != WI)
		uint32 msgtype_unknown    :  1;       // aka COPY bit
		uint32 sada_lkup_miss     :  1;       // sada lut miss
		uint32 flow_lkup_miss     :  1;       // sada or flow lut miss
		uint32 flow_lut_full      :  1;       // flow lut table full
		uint32 flow_find_valid    :  1;       // flow lut entry loc valid
		uint32 PAD                :  5;
		uint32 flow_find_loc      : 12;       // flow entry loc free index
		uint32 PAD                :  4;
	};
} hwa_txpost_status_reg_t;

typedef union hwa_txpost_status_reg2          // HWA3a Internal Debug
{
	uint32 u32;
	struct {
		uint32 cmd_frc_dma_fsm    :  4;
		uint32 dma_fsm            :  3;
		uint32 pkt_proc_fsm       :  5;
		uint32 pkt_xfer_done      :  1;
		uint32 evict_all_chains   :  1;
		uint32 pktchn_interrupt   :  1;
		uint32 clk_req            :  1;
		uint32 txdata_idle        :  1;
		uint32 evict_longest_chain:  1;
		uint32 cmdq_rd_intr       :  1;
		uint32 pkt_dealloc_rd_intr:  1;
		uint32 PAD                : 12;
	};
} hwa_txpost_status_reg2_t;

typedef union hwa_txpost_debug_reg
{
	uint32 u32;
	struct {
		uint32 fw_cmd_uflow       :  1;
		uint32 fw_cmd_oflow       :  1;
		uint32 fw_zero_wi         :  1;
		uint32 fw_cmd_mem_0_mis   :  1;
		uint32 fw_cmd_mem_1_mis   :  1;
		uint32 fw_Q_in_zero_wi    :  1;
		uint32 fw_Q_in_1k_wi      :  1;
		uint32 pktid0_zero        :  1;
		uint32 pktid1_zero        :  1;
		uint32 pktid_order_err    :  1;
		uint32 transid_oflow      :  1;
		uint32 transid_uflow      :  1;
		uint32 PAD                :  4;
		uint32 fw_cmd_cnt         :  6;
		uint32 PAD                : 10;
	};
} hwa_txpost_debug_reg_t;

typedef enum hwa_txpost_local_mem_mode
{
	hwa_txpost_sequence_fw_command = 0,
	hwa_txpost_sequence_undefined  = 1,
	hwa_txpost_sequence_compact    = 2,
	hwa_txpost_sequence_aggregate  = 3
} hwa_txpost_local_mem_mode_t;

/*
 * -----------------------------------------------------------------------------
 * HWA3a block FlowRingContext (FRC).
 *
 * SW allocates an array of FRCs in AXI memory space to carry the configuration
 * of each Flowring. When SW issues a Flowring Schedule command to HWA3a, HWA
 * will fetch the FRC. SW may not change a FRC when Schedule commands are
 * outstanding.
 * -----------------------------------------------------------------------------
 */

// HWA-2.0 : Selection of Compact of Aggregate Compact Message Format
#define HWA_TXPOST_FRC_COMPACT_MSG_FORMAT       0                     // HWA-2.0
#define HWA_TXPOST_FRC_AGGR_COMPACT_MSG_FORMAT  1                     // HWA-2.0

// RingConfig2: FlowId Lookup Overrides Selection - for don't care lookup
#define HWA_TXPOST_FRC_LKUP_OVERRIDE_NONE   0 // Use TxPost values
#define HWA_TXPOST_FRC_LKUP_OVERRIDE_PRIO   1 // prio = 8
#define HWA_TXPOST_FRC_LKUP_OVERRIDE_IFID   2 // ifid = 31
#define HWA_TXPOST_FRC_LKUP_OVERRIDE_SADA   4 // 00:00:00:00:00:00

// RingConfig3: LUT Selector and which SA/DA to use
#define HWA_TXPOST_FRC_LKUP_TYPE_b00        0 // Use DA in FlowTable LUT
#define HWA_TXPOST_FRC_LKUP_TYPE_b01        1 // Use SA in FlowTable LUT
#define HWA_TXPOST_FRC_LKUP_TYPE_b10        2 // Use DA in FlowTable LUT
#define HWA_TXPOST_FRC_LKUP_TYPE_b11        3 // Use Interface Priority LUT

#define HWA_TXPOST_FRC_WORDS        7
#define HWA_TXPOST_FRC_BYTES        (HWA_TXPOST_FRC_WORDS * NBU32)

typedef union hwa_txpost_frc                  // Flow Ring Context configuration
{
	uint8   u8[HWA_TXPOST_FRC_BYTES];         // 28 Bytes
	uint32 u32[HWA_TXPOST_FRC_WORDS];         //  7 Words

	struct {
		dma64addr_t haddr64;                  // +----- word#0 #1

		union {                               // +----- word#2
			struct {                          // BF Access
				uint32 RSVD_FIELD       : 16; // ref: CD[wr_idx_offset]
				uint32 aggr_depth       :  4; // packets per aggr WI     HWA-2.0
				uint32 aggr_mode        :  1; // Compact|AggrCompact     HWA-2.0
				uint32 PAD              : 11; // UNDEFINED
			};                                //
			struct {                          // CD Access
				uint16 wr_idx_offset;         // offset into H2D WR array
				uint16 RSVD_FIELD;            // ref: BF[aggr_depth..aggr_mode]
			};                                //
		}; // u32[2]
		struct {                              // +----- word#3 : RingConfig1
			uint16 ring_size;                 // number of WI in ring
			uint16 srs_idx;                   // index into statistics reg set
		}; // u32[3]
		union {                               // +----- word#4 : RingConfig2
			struct {                          // BF access
				uint32 flowid           : 12; // FRC defined flowid
				uint32 lkup_override    :  4; // [prio, ifid, SADA] overrides
				uint32 pyld_min_length  :  7; // used in WI audit
				uint32 PAD              :  1; // UNDEFINED
				uint32 RSVD_FIELD       :  8; // ref: CD[ifid]
			};                                //
			struct {                          // CD Access
				uint8  RSVD_FIELD[3];         // ref BF[flowid..pyld_min_length]
				uint8  ifid;                  // used in WI audit
			};                                //
		}; // u32[4]
		union {                               // +----- word#5 : RingConfig3
			struct {                          // BF Access
				uint32  lkup_type       :  2; // DA, SA, DA, Prio
				uint32  etype_ip_enable :  1; // enable Etype IPv4/6 check
				uint32  audit_enable    :  1; // enable workitem audit
				uint32  PAD             : 12; // UNDEFINED
				uint32  RSVD_FIELD      : 16; // ref: CD[avg_pkt_size]
			};                                //
			struct {                          // CD Access
				uint16  RSVD_FIELD;           // ref BF[lkup_type..audit_enable]
				uint16  avg_pkt_size;         // average packet size ... hmmm
			};                                //
		}; // u32[5]
		struct {                              // +----- word#6
			uint8   epoch;                    // used in audit
			uint8   PAD[3];                   // UNDEFINED
		}; // u32[6]
	};

} hwa_txpost_frc_t;

// Bits in hwa_txpost_frc::word#2
#define HWA_TXPOST_FRC_WR_IDX_OFFSET_SHIFT      0
#define HWA_TXPOST_FRC_WR_IDX_OFFSET_MASK \
	(0xffff << HWA_TXPOST_FRC_WR_IDX_OFFSET_SHIFT)
#define HWA_TXPOST_FRC_AGGR_DEPTH_SHIFT      16
#define HWA_TXPOST_FRC_AGGR_DEPTH_MASK \
	(0xf << HWA_TXPOST_FRC_AGGR_DEPTH_SHIFT)
#define HWA_TXPOST_FRC_AGGR_MODE_SHIFT      20
#define HWA_TXPOST_FRC_AGGR_MODE_MASK \
	(0x1 << HWA_TXPOST_FRC_AGGR_MODE_SHIFT)

// Bits in hwa_txpost_frc::word#4: RingConfig2
#define HWA_TXPOST_FRC_FLOWID_SHIFT      0
#define HWA_TXPOST_FRC_FLOWID_MASK \
	(0xfff << HWA_TXPOST_FRC_FLOWID_SHIFT)
#define HWA_TXPOST_FRC_LKUP_OVERRIDE_SHIFT      12
#define HWA_TXPOST_FRC_LKUP_OVERRIDE_MASK \
	(0xf << HWA_TXPOST_FRC_LKUP_OVERRIDE_SHIFT)
#define HWA_TXPOST_FRC_PYLD_MIN_LENGTH_SHIFT      16
#define HWA_TXPOST_FRC_PYLD_MIN_LENGTH_MASK \
	(0x7f << HWA_TXPOST_FRC_PYLD_MIN_LENGTH_SHIFT)
#define HWA_TXPOST_FRC_IFID_SHIFT    24
#define HWA_TXPOST_FRC_IFID_MASK \
	(0xff << HWA_TXPOST_FRC_IFID_SHIFT)

// Bits in hwa_txpost_frc::word#5: RingConfig3
#define HWA_TXPOST_FRC_LKUP_TYPE_SHIFT      0
#define HWA_TXPOST_FRC_LKUP_TYPE_MASK \
	(0x3 << HWA_TXPOST_FRC_LKUP_TYPE_SHIFT)
#define HWA_TXPOST_FRC_ETYPE_IP_ENABLE_SHIFT      2
#define HWA_TXPOST_FRC_ETYPE_IP_ENABLE_MASK \
	(0x1 << HWA_TXPOST_FRC_ETYPE_IP_ENABLE_SHIFT)
#define HWA_TXPOST_FRC_AUDIT_ENABLE_SHIFT      3
#define HWA_TXPOST_FRC_ENABLE_MASK \
	(0x1 << HWA_TXPOST_FRC_AUDIT_ENABLE_SHIFT)
#define HWA_TXPOST_FRC_AVG_PKT_SIZE_SHIFT      16
#define HWA_TXPOST_FRC_AVG_PKT_SIZE_MASK \
	(0xffff << HWA_TXPOST_FRC_AVG_PKT_SIZE_SHIFT)

/*
 * -----------------------------------------------------------------------------
 * Flowring Schedule Command posted by FW to HWA3a Block.
 *
 * A Schedule command may request transfer in units of workitems or octets. HWA
 * will process a Schedule command and construct one or more Packet Chains from
 * WorkItems. HWA posts PacketChains to SW. A PacketChain can be correlated to
 * a Schedule command using the transaction id. SW provides the RD index.
 * For Octet based transfers, HWA will use the FRC::wr_idx_offset to determine
 * number of workitems available in flowring.
 *
 * S2H Circular Ring sized to HWA_TXPOST_SCHEDCMD_RING_MAX
 *
 * -----------------------------------------------------------------------------
 */

#define HWA_TXPOST_SCHEDCMD_WORKITEMS       0 // units of Workitems
#define HWA_TXPOST_SCHEDCMD_OCTETS          1 // units of Octets

#define HWA_TXPOST_SCHEDCMD_WORDS   2
#define HWA_TXPOST_SCHEDCMD_BYTES   (HWA_TXPOST_SCHEDCMD_WORDS * NBU32)

typedef union hwa_txpost_schedcmd             // FW -> HWA3a Schedule Command
{
	uint8   u8[HWA_TXPOST_SCHEDCMD_BYTES];    // 8 Bytes
	uint32 u32[HWA_TXPOST_SCHEDCMD_WORDS];    // 2 Words

	struct {
		union {                               // +----- word#0
			struct {                          // BF Access
				uint32 flowring_id      : 12; // identify flowring
				uint32 transfer_type    :  1; // WIs or Octets based cmd
				uint32 phase_bit        :  1; // expected WI phase, audit
				uint32 PAD              :  2; // UNDEFINED
				uint32 RSVD_FIELD       : 16; // ref: CD[rd_idx]
			};                                //
			struct {                          // CD Access
				uint16 RSVD_FIELD;            // ref: BF[flowring_id..phase_bit]
				uint16 rd_idx;                // read index in flowring
			};                                //
		}; // u32[0]
		union {                               // +----- word#1
			struct {                          // BF Access
				uint32 transfer_count   : 24; // in units of WIs or Octets
				uint32 RSVD_FIELD       :  8; // ref: CD[trans_id]
			};                                //
			struct {                          // CD Access
				uint8  RSVD_FIELD[3];         // ref: BF[transfer_count]
				uint8  trans_id;              // cmd's transaction id
			};                                //
		}; // u32[1]
	};

} hwa_txpost_schedcmd_t;

// Bits in hwa_txpost_schedcmd::word#0
#define HWA_TXPOST_SCHEDCMD_FLOWRING_ID_SHIFT      0
#define HWA_TXPOST_SCHEDCMD_FLOWRING_ID_MASK \
	(0xfff << HWA_TXPOST_SCHEDCMD_FLOWRING_ID_SHIFT)
#define HWA_TXPOST_SCHEDCMD_TRANSFER_TYPE_SHIFT      12
#define HWA_TXPOST_SCHEDCMD_TRANSFER_TYPE_MASK \
	(0x1 << HWA_TXPOST_SCHEDCMD_TRANSFER_TYPE_SHIFT)
#define HWA_TXPOST_SCHEDCMD_PHASE_BIT_SHIFT      13
#define HWA_TXPOST_SCHEDCMD_PHASE_BIT_MASK \
	(0x1 << HWA_TXPOST_SCHEDCMD_PHASE_BIT_SHIFT)
#define HWA_TXPOST_SCHEDCMD_RD_IDX_SHIFT      16
#define HWA_TXPOST_SCHEDCMD_RD_IDX_MASK \
	(0xffff << HWA_TXPOST_SCHEDCMD_RD_IDX_SHIFT)

// Bits in hwa_txpost_schedcmd::word#1
#define HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT_SHIFT      0
#define HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT_MASK \
	(0xffffff << HWA_TXPOST_SCHEDCMD_TRANSFER_COUNT_SHIFT)
#define HWA_TXPOST_SCHEDCMD_TRANS_ID_SHIFT      24
#define HWA_TXPOST_SCHEDCMD_TRANS_ID_MASK \
	(0xff << HWA_TXPOST_SCHEDCMD_TRANS_ID_SHIFT)

/*
 * -----------------------------------------------------------------------------
 * In response to a Schedule Request, HWA3a provides one or more PacketChain(s)
 * transmit requests to SW. Each PacketChain includes a transaction id, that
 * may be used by SW to correlate it to the SW issued Schedule request command.
 *
 * HWA3a includes a 48bit timestamp in each PacketChain response.
 *
 * NOTE: In HWA-1.0, a PacketChain may include one additional dummy buffer past
 * the tail pointer. SW needs to check whether the tail packet's next is
 * pointing to a dummy packet and explicitly free it. Packet count does not
 * include the dummy packet.
 *
 * H2S Circular Ring sized to HWA_TXPOST_PKTCHAIN_RING_MAX
 *
 * -----------------------------------------------------------------------------
 */

#define HWA_TXPOST_PKTCHAIN_WORDS   5
#define HWA_TXPOST_PKTCHAIN_BYTES   (HWA_TXPOST_PKTCHAIN_WORDS * NBU32)

typedef union hwa_txpost_pktchain             // HWA3a --> FW pktchain response
{
	uint8   u8[HWA_TXPOST_PKTCHAIN_BYTES];    // 20 Bytes
	uint32 u32[HWA_TXPOST_PKTCHAIN_WORDS];    //  5 Words

	struct {
		uint32 head;                          // +----- word#0
		uint32 tail;                          // +----- word#1
		union {                               // +----- word#2
			struct {                          // BF Access
				uint32 total_octets     : 24; // total octets in pkt chain
				uint32 RSVD_FIELD       :  8; // ref: CD[trans_id]
			};                                //
			struct {                          // CD Access
				uint8  RSVD_FIELD[3];         // ref: BF[total_octets]
				uint8  trans_id;              // sched command transaction id
			};                                //
		}; // u32[2]
		struct {                              // +----- word#3
			uint16 pkt_count;                 // num packets in chain
			uint16 timestamp_msb16;           // timestamp upper 16b
		}; // u32[3]
		uint32 timestamp_lsb32;               // +----- word#4
	};

} hwa_txpost_pktchain_t;

// Bits in hwa_txpost_pktchain::word#2
#define HWA_TXPOST_PKTCHAIN_TOTAL_OCTETS_SHIFT      0
#define HWA_TXPOST_PKTCHAIN_TOTAL_OCTETS_MASK \
	(0xffffff << HWA_TXPOST_PKTCHAIN_TOTAL_OCTETS_SHIFT)
#define HWA_TXPOST_PKTCHAIN_TRANS_ID_SHIFT      24
#define HWA_TXPOST_PKTCHAIN_TRANS_ID_MASK \
	(0xff << HWA_TXPOST_PKTCHAIN_TRANS_ID_SHIFT)

#if !defined(HWA_NO_LUT)

/*
 * -----------------------------------------------------------------------------
 * HWA3a Block Priority based FlowId Lkup Table
 *
 * Two dimensional array, indexed by ifid and priority.
 *
 * HWA-1.0: Each entry is 12 bits wide, with ifid = [0..16] and prio = [0..8]
 *          ifid = 16 and prio=8 are valid values (FRC overrides)
 * HWA-2.0: Each 12 bit flowid entry is padded by 4 bits.
 *          No flowid wraps around 32 bit word.
 *          ifid = [0..16] and prio = [0..8]
 *          ifid = 16 and prio=8 are valid values (FRC overrides)
 *
 * -----------------------------------------------------------------------------
 */

typedef struct hwa_txpost_prio_flowid {
	uint16 flowid_table[HWA_PRIORITIES_MAX];    // [0..7, 8] 8 is override
} hwa_txpost_prio_flowid_t;

typedef struct hwa_txpost_prio_lut {
	hwa_txpost_prio_flowid_t if_table[HWA_INTERFACES_MAX]; // [0..15, 16]
} hwa_txpost_prio_lut_t;

/*
 * -----------------------------------------------------------------------------
 * HWA3a Block SA/DA Memory:
 *
 * Array of 48bit ethernet addresses, SADA_fid_mem_entry. Unique SADA table is
 * first looked up. The index of the SADA matched entry, serves as a hash index
 * into the Flowid LUT (described below).
 *
 * HWA-1.0: Each ethernet address entry is 48 bit wide.
 *          Entries may cross a 32bit boundary.
 * HWA-2.0: Each 48 bit ethernet address is padded by 16 bits, to 64 bit ???
 *
 * -----------------------------------------------------------------------------
 */

#define HWA_TXPOST_SADA_WORDS       2
#define HWA_TXPOST_SADA_BYTES       (HWA_TXPOST_SADA_WORDS * NBU32)
#define HWA_TXPOST_SADA_UINT16      (HWA_ETHER_ADDR_LEN >> 1)

typedef union hwa_txpost_sada_lut_elem        // 64 bit entry in sada LUT
{
	uint8    u8[HWA_TXPOST_SADA_BYTES];       // 8 Bytes
	uint16  u16[HWA_TXPOST_SADA_UINT16];      // 3 x uint16 (+ uint16 pad)
	uint32  u32[HWA_TXPOST_SADA_WORDS];       // 2 Words

	uint8   sada[HWA_ETHER_ADDR_LEN];         // Ethernet SA or DA address

} hwa_txpost_sada_lut_elem_t;

typedef struct hwa_txpost_sada_lut            // Unique Ethernet SADA LUT
{
	hwa_txpost_sada_lut_elem_t table[HWA_TXPOST_SADA_LUT_DEPTH];
} hwa_txpost_sada_lut_t;

/*
 * -----------------------------------------------------------------------------
 * HWA3a block Flowid Lookup Table
 *
 * Flowid LUT is implemented as a hash table. Unique SADA lookup result is used
 * to hash into the Flowid LUT. Each Flowid LUT entry carries a link and valid
 * setting. the link implements the collision list.
 *
 * -----------------------------------------------------------------------------
 */
#define HWA_TXPOST_FLOW_LINK_NULL   0x3ff

#define HWA_TXPOST_FLOW_LUT_WORDS   1
#define HWA_TXPOST_FLOW_LUT_BYTES   (HWA_TXPOST_FLOW_LUT_WORDS * NBU32)

typedef union hwa_txpost_flow_lut_elem        // 32 bit Entry in Flow based LUT
{
	uint32 u32[HWA_TXPOST_FLOW_LUT_WORDS];    // 1 Word
	uint8   u8[HWA_TXPOST_FLOW_LUT_BYTES];    // 4 Bytes
	struct {
		uint32 link    : 10;                  // next entry in list
		uint32 flowid  : 12;                  // flowid
		uint32 prio    :  4;                  // [0..8] incld prio = 8
		uint32 ifid    :  5;                  // [0..16] incld ifid = 16
		uint32 valid   :  1;                  // entry is valid
	};
} hwa_txpost_flow_lut_elem_t;

typedef struct hwa_txpost_flow_lut
{
	hwa_txpost_flow_lut_elem_t table[HWA_TXPOST_FLOW_LUT_DEPTH];
} hwa_txpost_flow_lut_t;

#define HWA_TXPOST_FLOW_LINK_SHIFT      0
#define HWA_TXPOST_FLOW_LINK_MASK \
	(0x3ff << HWA_TXPOST_FLOW_LINK_SHIFT)
#define HWA_TXPOST_FLOW_FLOWID_SHIFT      10
#define HWA_TXPOST_FLOW_FLOWID_MASK \
	(0xfff << HWA_TXPOST_FLOW_FLOWID_SHIFT)
#define HWA_TXPOST_FLOW_PRIO_SHIFT      22
#define HWA_TXPOST_FLOW_PRIO_MASK \
	(0xf << HWA_TXPOST_FLOW_PRIO_SHIFT)
#define HWA_TXPOST_FLOW_IFID_SHIFT      26
#define HWA_TXPOST_FLOW_IFID_MASK \
	(0x1f << HWA_TXPOST_FLOW_IFID_SHIFT)
#define HWA_TXPOST_FLOW_VALID_SHIFT      31
#define HWA_TXPOST_FLOW_VALID_MASK \
	(0x1 << HWA_TXPOST_FLOW_VALID_SHIFT)
#endif /* !HWA_NO_LUT */

/*
 * -----------------------------------------------------------------------------
 * HWA3a block Statistics Register Set (SRS)
 *
 * Register based control of SRS.
 * Number of SRS is limited to HWA_TXPOST_FRC_SRS_MAX
 *
 * -----------------------------------------------------------------------------
 */

#define HWA_TXPOST_FRC_SRS_WORDS    8	/* It's 8 wrods not 7 words */
#define HWA_TXPOST_FRC_SRS_BYTES    (HWA_TXPOST_FRC_SRS_WORDS * NBU32)

typedef union hwa_txpost_frc_srs              // TxPost Statistics Register Set
{
	uint8   u8[HWA_TXPOST_FRC_SRS_BYTES];     // 32 Bytes
	uint32 u32[HWA_TXPOST_FRC_SRS_WORDS];     //  8 Words

	struct {
		uint32 schedcmds;                     // schedule cmds serviced by HWA
		uint32 swpkts;                        // swpkts transferred to SW
		uint16 audit_fails;                   // workitems that failed audit
		uint16 max_pktch_size;                // longest packet chain
		uint32 octets_ls32;                   // total octets fetched ls32
		uint32 octets_ms32;                   // total octets fetched ms32
		uint32 rd_idx_updates;                // RD index updates serviced
		uint16 pkt_allocs;                    // pkts allocated by SW
		uint16 pkt_deallocs;                  // pkts de-allocated by SW
	};
} hwa_txpost_frc_srs_t;

// Block level common statistics
typedef union hwa_txpost_stats
{
	struct {
		uint16 num_stalls_sw;                 // Tx BM depleted, PktChain full
		uint16 num_stalls_hw;                 // DMA busy, local memory full
		uint16 dur_bm_empty;                  // duation BM depleted
		uint16 dur_dma_busy;                  // duration DMA busy
	};
	uint32 u32[HWA_BLOCK_STATISTICS_WORDS];
} hwa_txpost_stats_t;

/*
 * -----------------------------------------------------------------------------
 * HWA-2.0 block 3a facing SW Packet Structure
 * -----------------------------------------------------------------------------
 */

#define HWAPKT2LFRAG(pkt)	(((char *)(pkt)) + HWA_TXPOST_PKT_BYTES)
#define LFRAG2HWAPKT(lb)	(((char *)(lb)) - HWA_TXPOST_PKT_BYTES)

#define HWA_TXPOST_PKT_FLAGS_SADA_MISS  12
#define HWA_TXPOST_PKT_FLAGS_FLOW_MISS  13
#define HWA_TXPOST_PKT_FLAGS_COPY_ASIS  14
#define HWA_TXPOST_PKT_FLAGS_AUDITFAIL  15

#define HWA_TXPOST_PKT_WORDS        11
#define HWA_TXPOST_PKT_BYTES        (HWA_TXPOST_PKT_WORDS * NBU32)

typedef union hwa_txpost_pkt
{
	uint8   u8[HWA_TXPOST_PKT_BYTES];         // 44 Bytes
	uint32 u32[HWA_TXPOST_PKT_WORDS];         // 11 Words

	struct {                                  // 11 words seen by HWA-2.0 3a
		union hwa_txpost_pkt *next;           // +----- word#0
		union {                               // +----- word#1
			void   *hdr_buf;                  // 32b pointer to header buffer
			uint32  hdr_buf_daddr32;          // 32b address in dongle sysmem
		}; // u32[1]
		union {
			uint32     host_pktid;            // +----- word#2
			struct {                          // 3b
				uint8 num_desc;
				uint8 RSVD_FIELD;
				uint16 hdr_buf_dlen;          // header buffer length
			};
		}; // u32[2]
		union {                               // +----- word#3
			struct {                          // BF Access
				uint32 ifid             :  5; // interface id
				uint32 prio             :  3; // packet priority
				uint32 copy             :  1; // COPY as-is bit
				uint32 flags            :  7; // flags
				uint32 RSVD_FIELD       : 16; // ref: CD[data_buf_hlen]
			};                                //
			struct {                          // CD Access
				union {
					uint16 RSVD_FIELD;        // ref: BF[ifid..flags]
					uint16 amsdu_total_len;   // 3b
				};
				uint16 data_buf_hlen;         // data buffer length
			};                                //
		}; // u32[3]
		dma64addr_t data_buf_haddr;           // +----- word#4 #5
		union {                               // +----- word#6 #7 #8
			hwa_txpost_eth_sada_t eth_sada;
			struct {                          // for TXFIFO 3b
				uint32 txdma_flags;           // map to 3b SWPKT txdma_flags
				uint32 RSVD_FIELD[2];
			};
		}; // u32[6,7,8]
		union {                               // +----- word#9
			struct {                          // BF Access
				uint32 RSVD_FIELD       : 16; // ref: CD[eth_type]
				uint32 info             :  4; // Host field for future use
				uint32 flowid_override  : 12; // Host flowid override
			};                                //
			struct {                          // CD Access
				uint16 eth_type;              // packet Ether-Type
				uint16 RSVD_FIELD;            // ref: BF[info..flowid_override]
			};                                //
		}; // u32[9]
		union {                               // +----- word#10
			struct {                          // BF Access
				uint32 RSVD_FIELD       : 16; // ref: CD[rd_index]
				uint32 PAD              : 12; // undefined
				uint32 sada_miss        :  1; // SADA Lkup Miss
				uint32 flow_miss        :  1; // flowid LUT Miss  or SADA miss
				uint32 copy_asis        :  1; // copy as-is
				uint32 audit_fail       :  1; // audit failure
			};                                //
			struct {                          // CD Access
				uint16 rd_index;              // flowring rd index
				uint16 audit_flags;           // ref: BF[sada_miss..audit_fail]
			};
		}; // u32[10]
	};
} hwa_txpost_pkt_t, hwa_txfifo_pkt_t;

// Bits in hwa_txpost_pkt::word#3
#define HWA_TXPOST_PKT_IFID_SHIFT      0
#define HWA_TXPOST_PKT_IFID_MASK \
	(0x1f << HWA_TXPOST_PKT_IFID_SHIFT)
#define HWA_TXPOST_PKT_PRIO_SHIFT      5
#define HWA_TXPOST_PKT_PRIO_MASK \
	(0x7 << HWA_TXPOST_PKT_PRIO_SHIFT)
#define HWA_TXPOST_PKT_COPY_SHIFT      8
#define HWA_TXPOST_PKT_COPY_MASK \
	(0x1 << HWA_TXPOST_PKT_COPY_SHIFT)
#define HWA_TXPOST_PKT_FLAGS_SHIFT      9
#define HWA_TXPOST_PKT_FLAGS_MASK \
	(0x7f << HWA_TXPOST_PKT_FLAGS_SHIFT)

// Bits in hwa_txpost_pkt::word#6
#define HWA_TXPOST_PKT_ETH_TYPE_SHIFT      0
#define HWA_TXPOST_PKT_ETH_TYPE_MASK \
	(0xffff << HWA_TXPOST_PKT_ETH_TYPE_SHIFT)
#define HWA_TXPOST_PKT_INFO_SHIFT      16
#define HWA_TXPOST_PKT_INFO_MASK \
	(0xf << HWA_TXPOST_PKT_INFO_SHIFT)
#define HWA_TXPOST_PKT_FLOWID_OVERRIDE_SHIFT      20
#define HWA_TXPOST_PKT_FLOWID_OVERRIDE_MASK \
	(0xfff << HWA_TXPOST_PKT_FLOWID_OVERRIDE_SHIFT)

// Bits in hwa_txpost_pkt::word#7
#define HWA_TXPOST_PKT_RD_INDEX_SHIFT      0
#define HWA_TXPOST_PKT_RD_INDEX_MASK \
	(0xffff << HWA_TXPOST_PKT_RD_INDEX_SHIFT)
#define HWA_TXPOST_PKT_AUDIT_FLAGS_SADA_MISS_SHIFT      28
#define HWA_TXPOST_PKT_AUDIT_FLAGS_SADA_MISS_MASK \
	(0x1 << HWA_TXPOST_PKT_AUDIT_FLAGS_SADA_MISS_SHIFT)
#define HWA_TXPOST_PKT_AUDIT_FLAGS_FLOW_MISS_SHIFT      29
#define HWA_TXPOST_PKT_AUDIT_FLAGS_FLOW_MISS_MASK \
	(0x1 << HWA_TXPOST_PKT_AUDIT_FLAGS_FLOW_MISS_SHIFT)
#define HWA_TXPOST_PKT_AUDIT_FLAGS_COPY_ASIS_SHIFT      30
#define HWA_TXPOST_PKT_AUDIT_FLAGS_COPY_ASIS_MASK \
	(0x1 << HWA_TXPOST_PKT_AUDIT_FLAGS_COPY_ASIS_SHIFT)
#define HWA_TXPOST_PKT_AUDIT_FLAGS_AUDIT_FAIL_SHIFT      31
#define HWA_TXPOST_PKT_AUDIT_FLAGS_AUDIT_FAIL_MASK \
	(0x1 << HWA_TXPOST_PKT_AUDIT_FLAGS_AUDIT_FAIL_SHIFT)

#if HWA_REVISION_GE_129
/*
 * -----------------------------------------------------------------------------
 * S2H TX Free TxBuf Index ring aka "FREEIDXTX"
 * S2H Circular Ring sized to HWA_TXPOST_TXFREE_DEPTH
 * -----------------------------------------------------------------------------
 */

#define HWA_TXPOST_TXFREE_WORDS 1
#define HWA_TXPOST_TXFREE_BYTES     (HWA_TXPOST_TXFREE_WORDS * NBU32)

typedef union hwa_txpost_txfree               // aka FREEIDXTX S2H Interface
{
	uint8   u8[HWA_TXPOST_TXFREE_BYTES];      //  4 Bytes
	uint32 u32[HWA_TXPOST_TXFREE_WORDS];      //  1 Words

	struct {
		uint16 index;                         // +----- word#0
		uint16 PAD;
	};
} hwa_txpost_txfree_t;
#endif /* HWA_REVISION_GE_129 */

#else  /* ! HWA_TXPOST_BUILD */
#define HWA_TXPOST_EXPR(expr)       HWA_NONE
#endif /* ! HWA_TXPOST_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA3b TxFIFO Block
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_TXFIFO_BUILD)
#define HWA_TXFIFO_EXPR(expr)       expr

// Bits in hwa_txfifo_pkt::txdma_flags word#6
#define HWA_TXFIFO_TXDMA_FLAGS_COHERENT_SHIFT     0
#define HWA_TXFIFO_TXDMA_FLAGS_COHERENT_MASK \
	(0x1 << HWA_TXFIFO_TXDMA_FLAGS_COHERENT_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_NOTPCIE_SHIFT      1
#define HWA_TXFIFO_TXDMA_FLAGS_NOTPCIE_MASK \
	(0x1 << HWA_TXFIFO_TXDMA_FLAGS_NOTPCIE_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_AC_SHIFT           2
#define HWA_TXFIFO_TXDMA_FLAGS_AC_MASK \
	(0xf << HWA_TXFIFO_TXDMA_FLAGS_AC_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_INTONCOMP_SHIFT    6
#define HWA_TXFIFO_TXDMA_FLAGS_INTONCOMP_MASK \
	(0x1 << HWA_TXFIFO_TXDMA_FLAGS_INTONCOMP_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_TXDTYPE_SHIFT      7
#define HWA_TXFIFO_TXDMA_FLAGS_TXDTYPE_MASK \
	(0x1 << HWA_TXFIFO_TXDMA_FLAGS_TXDTYPE_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_EPOCH_SHIFT        8
#define HWA_TXFIFO_TXDMA_FLAGS_EPOCH_MASK \
	(0x3 << HWA_TXFIFO_TXDMA_FLAGS_EPOCH_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_FRAMETYPE_SHIFT   10
#define HWA_TXFIFO_TXDMA_FLAGS_FRAMETYPE_MASK \
	(0x3 << HWA_TXFIFO_TXDMA_FLAGS_FRAMETYPE_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_FRAG_SHIFT        12
#define HWA_TXFIFO_TXDMA_FLAGS_FRAG_MASK \
	(0x1 << HWA_TXFIFO_TXDMA_FLAGS_FRAG_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_SOFTBITS_SHIFT    13
#define HWA_TXFIFO_TXDMA_FLAGS_SOFTBITS_MASK \
	(0x7f << HWA_TXFIFO_TXDMA_FLAGS_SOFTBITS_SHIFT)
#define HWA_TXFIFO_TXDMA_FLAGS_ADDREXT_SHIFT     20
#define HWA_TXFIFO_TXDMA_FLAGS_ADDREXT_MASK \
	(0x3 << HWA_TXFIFO_TXDMA_FLAGS_ADDREXT_SHIFT)

#define HWA_TXFIFO_PKT_WORDS        7
#define HWA_TXFIFO_PKT_BYTES        (HWA_TXFIFO_PKT_WORDS * NBU32)

// HWA3b allows for 32bit and 64bit Packet Address in PktChain specification
// In Full Dongle mode, pktchain32 form will be used.
// In NIC mode, pktchain64 may be used -- even on 32bit hosts.

// Layout of a PktChain32 request sent to HWA3b over S2H PktChain Interface
#define HWA_TXFIFO_PKTCHAIN32_WORDS 4
#define HWA_TXFIFO_PKTCHAIN32_BYTES (HWA_TXFIFO_PKTCHAIN32_WORDS * NBU32)

typedef union hwa_txfifo_pktchain32           // TxFIFO PktChain32 xfer request
{
	uint8   u8[HWA_TXFIFO_PKTCHAIN32_BYTES];  // 16 Bytes
	uint32 u32[HWA_TXFIFO_PKTCHAIN32_WORDS];  //  4 Words

	struct {
		uint32 pkt_head;                      // +----- word#0
		uint32 pkt_tail;                      // +----- word#1
		struct {                              // +----- word#2
			uint16 pkt_count;                 // Total packets in chain, 12bits
			uint16 mpdu_count;                // Total MPDUs in chain, 12bits
		};
		struct {                              // +----- word#3
			uint8 fifo_idx;                   // 0 .. HWA_TX_FIFOS
			uint8 RSVD_FIELD[3];
		};
	};
} hwa_txfifo_pktchain32_t;

// Layout of a PktChain64 request sent to HWA3b over S2H PktChain Interface
#define HWA_TXFIFO_PKTCHAIN64_WORDS 6
#define HWA_TXFIFO_PKTCHAIN64_BYTES (HWA_TXFIFO_PKTCHAIN64_WORDS * NBU32)

typedef union hwa_txfifo_pktchain64           // TxFIFO PktChain64 xfer request
{
	uint8   u8[HWA_TXFIFO_PKTCHAIN64_BYTES];  // 24 Bytes
	uint32 u32[HWA_TXFIFO_PKTCHAIN64_WORDS];  //  6 Words

	struct {
		dma64addr_t pkt_head;                 // +----- word#0 #1
		dma64addr_t pkt_tail;                 // +----- word#2 #3
		struct {                              // +----- word#4
			uint16 pkt_count;                 // Total packets in chain, 12bits
			uint16 mpdu_count;                // Total MPDUs in chain, 12bits
		};
		struct {                              // +----- word#5
			uint8 fifo_idx;                   // 0 .. HWA_TX_FIFOS
			uint8 RSVD_FIELD[3];
		};
	};
} hwa_txfifo_pktchain64_t;

// HWA3b TxFIFO Layout of a Overflow Queue Context, 64 bit pkt addr form
#define HWA_TXFIFO_OVFLWQCTX_WORDS 8
#define HWA_TXFIFO_OVFLWQCTX_BYTES (HWA_TXFIFO_OVFLWQCTX_WORDS * NBU32)

typedef union hwa_txfifo_ovflwqctx            // Overflow Queue Context
{
	uint8   u8[HWA_TXFIFO_OVFLWQCTX_BYTES];   // 32 Bytes
	uint32 u32[HWA_TXFIFO_OVFLWQCTX_WORDS];   //  8 Words

	struct {
		dma64addr_t pktq_head;                // +----- word#0 #1
		dma64addr_t pktq_tail;                // +----- word#2 #3

		// Overflow Queue Statistics
		struct {                              // +----- word#4
			uint16 pkt_count;                 // total packets in queue
			uint16 mpdu_count;                // total MPDU in queue
		};
		struct {                              // +----- word#5
			uint16 pkt_count_hwm;             // hwm of total packets in queue
			uint16 mpdu_count_hwm;            // hwm of total MPDUs in queue
		};
		uint32 append_count;                  // +----- word#6
	};
	uint32 RSVD_FIELD;
} hwa_txfifo_ovflwqctx_t;

// HWA3b TxFIFO Layout of TxFIFO and AQM Context, 64 bit descriptor table addr
typedef struct hwa_txfifo_fifo
{
	dma64addr_t base;                         // +----- word#0 #1
	struct {                                  // +----- word#2
		uint16  curr_ptr;                     // RD index
		uint16  last_ptr;                     // WR index
	};
	union {                                   // +----- word#3
		struct {
			uint16 attrib;                    // addrext, coh, notpcie
			uint16 depth;                     // number of descriptors in FIFO
		};
		struct {
			uint16 addr_ext  : 2;             // Large PCIE wind AddrExt
			uint16 coherency : 1;             // Coherent DMA transfer
			uint16 not_pcie  : 1;             // DMA transfer over PCIE
		};
	};
} hwa_txfifo_fifo_t;

// HWA3b TxFIFO packets shadow.
typedef struct hwa_txfifo_shadow32
{
	uint32 pkt_head;                 // head pointer of this shadow
	uint32 pkt_tail;                 // tail pointer of this shadow
	uint16 pkt_count;                 // total packets in the shadow
	uint16 mpdu_count;                // total MPDU in the shadow
	struct {
		uint32 pkt_count;            // total packets in the shadow
		uint32 mpdu_count;           // total MPDU in the shadow
	} stats;
} hwa_txfifo_shadow32_t;

typedef struct hwa_txfifo_shadow64
{
	dma64addr_t pkt_head;            // head pointer of this shadow
	dma64addr_t pkt_tail;            // tail pointer of this shadow
	uint16 pkt_count;                 // total packets in the shadow
	uint16 mpdu_count;                // total MPDU in the shadow
	struct {
		uint32 pkt_count;            // total packets in the shadow
		uint32 mpdu_count;           // total MPDU in the shadow
	} stats;
} hwa_txfifo_shadow64_t;

// TxFIFO Attributes
#define HWA_TXFIFO_FIFO_ADDR_EXT_SHIFT  0
#define HWA_TXFIFO_FIFO_ADDR_EXT_MASK   (0x3 << HWA_TXFIFO_FIFO_ADDR_EXT_SHIFT)
#define HWA_TXFIFO_FIFO_COHERENCY_SHIFT 2
#define HWA_TXFIFO_FIFO_COHERENCY_MASK  (0x1 << HWA_TXFIFO_FIFO_COHERENCY_SHIFT)
#define HWA_TXFIFO_FIFO_NOTPCIE_SHIFT   3
#define HWA_TXFIFO_FIFO_NOTPCIE_MASK    (0x1 << HWA_TXFIFO_FIFO_NOTPCIE_SHIFT)

#define HWA_TXFIFO_FIFOCTX_WORDS    8
#define HWA_TXFIFO_FIFOCTX_BYTES    (HWA_TXFIFO_FIFOCTX_WORDS * NBU32)

// HWA3b TxFIFO DMA Descriptor tables for Pkt TxFIFO and CTDMA AQM FIFO
typedef union hwa_txfifo_fifoctx
{
	uint8   u8[HWA_TXFIFO_FIFOCTX_BYTES];     // 32 Bytes
	uint32 u32[HWA_TXFIFO_FIFOCTX_WORDS];     //  8 Words

	struct {
		hwa_txfifo_fifo_t pkt_fifo;           // +----- word#0 #1 #2 #3
		hwa_txfifo_fifo_t aqm_fifo;           // +----- word#4 #5 #6 #7
	};
} hwa_txfifo_fifoctx_t;

// HWA4a TxFifo block level statistics
typedef union hwa_txfifo_stats
{
	struct {
		uint32 TBD;                           // TBD ...
	};

#if HWA_REVISION_GE_129
	struct { // Valid after REV129
		uint32 pktc_empty;                    // ptkChain queue empty
		uint32 ovf_empty;                     // overflow queue empty
		uint32 ovf_full;                      // overflow queue full
		uint32 DBG[2];                        // debug #4, #5
		uint32 pull_fsm_stall;                // pull fsm stall
	};
#endif // endif

	uint32 u32[HWA_BLOCK_STATISTICS_WORDS];
} hwa_txfifo_stats_t;

#else  /* ! HWA_TXFIFO_BUILD */
#define HWA_TXFIFO_EXPR(expr)       HWA_NONE
#endif /* ! HWA_TXFIFO_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA4a TxStat Block
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_TXSTAT_BUILD)
#define HWA_TXSTAT_EXPR(expr)       expr

// 4a block level statistics
typedef union hwa_txstat_stats
{
	struct {
		uint32 num_lazy_intr;                 // num lazy intr generated
		uint32 num_queue_full_ctr_sat;        // num times queue full cntr sats
		uint16 num_stalls_dma;                // H2D mem2mem DMA busy
		uint16 dur_dma_busy;                  // duration DMA busy
	};
	uint32 u32[HWA_BLOCK_STATISTICS_WORDS];
} hwa_txstat_stats_t;

#else  /* ! HWA_TXSTAT_BUILD */
#define HWA_TXSTAT_EXPR(expr)       HWA_NONE
#endif /* ! HWA_TXSTAT_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: HWA2b RxCpl Block and HWA4b TxCpl Block
 * -----------------------------------------------------------------------------
 */
#if defined(HWA_RXCPLE_BUILD)
#define HWA_RXCPLE_EXPR(expr)       expr
#else  /* !HWA_RXCPLE_BUILD */
#define HWA_RXCPLE_EXPR(expr)       HWA_NONE
#endif /* !HWA_RXCPLE_BUILD */

#if defined(HWA_TXCPLE_BUILD)
#define HWA_TXCPLE_EXPR(expr)       expr
#else  /* !HWA_TXCPLE_BUILD */
#define HWA_TXCPLE_EXPR(expr)       HWA_NONE
#endif /* !HWA_TXCPLE_BUILD */

#if defined(HWA_CPLENG_BUILD)
#define HWA_CPLENG_EXPR(expr)       expr

// Completion Rings RD and WR indices begin after D2H Control Ring
#define HWA_CPLENG_RING_RDWR_BASE_OFFSET \
	(HWA_PCIE_RW_INDEX_SZ * BCMPCIE_D2H_MSGRING_TX_COMPLETE_IDX)

// CAUTION: In CPLENG, hi and lo addresses are swapped and are not 8B aligned.
//          So do not use dma64addr_t!

// Layout of Completion Engine Descriptors
#define HWA_CPLENG_CED_WORDS        4
#define HWA_CPLENG_CED_BYTES        (HWA_CPLENG_CED_WORDS * NBU32)

typedef union hwa_cpleng_ced                  // Completion Entry Descriptor
{
	uint8   u8[HWA_CPLENG_CED_BYTES];         // 16 Bytes
	uint32 u32[HWA_CPLENG_CED_WORDS];         //  4 Words

	struct {
		struct {
			uint8  fw_intr_valid    :  1; // Generate FW interrupt on CED
			uint8  host_intr_valid  :  1; // Generate Host interrupt on CED
			uint8  PAD              :  6;
			uint8  PAD;
			uint16 cpl_ring_id;           // Selects a completion ring
		};
		struct {
			uint16 md_count;              // number of MD items in CED
			uint16 wi_count;              // number of WI items in CED
		};
		uint32  wi_array_addr;                // 32bit address of WI array
		uint32  md_array_addr;                // 32bit address of MD array
	};
} hwa_cpleng_ced_t;

// Layout of Metadata buffer Descriptor in metadata array
#define HWA_CPLENG_MD_WORDS         4
#define HWA_CPLENG_MD_BYTES         (HWA_CPLENG_MD_WORDS * NBU32)

typedef union hwa_cpleng_md                  // Metadata Buffer Descriptor
{
	uint8   u8[HWA_CPLENG_MD_BYTES];         // 16 Bytes
	uint32 u32[HWA_CPLENG_MD_WORDS];         //  4 Words

	struct {
		uint32 dngl_addr;                    // src location in dongle SysMem
		uint32 host_addr_hi;                 // dst location in host memory
		uint32 host_addr_lo;
		uint16 length;                       // length of metadata
		uint16 PAD;
	};
} hwa_cpleng_md_t;

// Layout of Host Tx / Rx Completion Rings Context
#define HWA_CPLENG_RING_WORDS       8
#define HWA_CPLENG_RING_BYTES       (HWA_CPLENG_RING_WORDS * NBU32)

typedef union hwa_cpleng_ring                 // completion ring context
{
	uint8   u8[HWA_CPLENG_RING_BYTES];        // 32 Bytes
	uint32 u32[HWA_CPLENG_RING_WORDS];        //  8 Words

	struct {
		uint32 base_addr_hi;                  // +----- word#0 : high 32
		uint32 base_addr_lo;                  // +----- word#1 : low 32
		struct {                              // +----- word#2
			uint16 rd_index;              // read index
			uint16 PAD;
		};
		struct {                              // +----- word#3
			uint32 element_sz       :  6; // size of each acwi compl element
#if HWA_REVISION_GE_129
			uint32 seqnum_enable    :  1; // HWA assisted seqnum insertion
			uint32 aggr_mode        :  1; // aggregation enabled
			uint32 pkts_per_aggr    :  4; // aggregation size
			uint32 PAD              :  4;
			uint32 depth            : 16; // depth of ring
#else
			uint32 depth            : 16; // depth of ring
			uint32 seqnum_enable    :  1; // HWA assisted seqnum insertion
			uint32 aggr_mode        :  1; // aggregation enabled
			uint32 pkts_per_aggr    :  4; // aggregation size
			uint32 PAD              :  4;
#endif /* HWA_REVISION_GE_129 */
		};
		struct {                              // +----- word#4
			uint16 wr_index;              // write index
			uint16 PAD;
		};
		struct {                              // +----- word#5
			uint32 intraggr_count   :  8; // interrupt aggregation count
			uint32 intraggr_tmout   : 20; // interrupt aggregation timeout
			uint32 PAD              :  2;
		};
		struct {                              // +----- word#6
			uint16 seqnum_start;          // start seqnum value
			uint8  seqnum_offset;         // position of seqnum in WI
			uint8  seqnum_width     :  1; // Size of seqnum: 0 = 8b, 1 = 16b
			uint8  PAD              :  7;
		};
		struct {                              // +----- word#7
			uint16 seqnum_current;        // current seqnum value - runtime
			uint16 PAD;
		};
	};
} hwa_cpleng_ring_t;

// 2b, 4b common CPL Engine statistics
typedef union hwa_cpleng_common_stats
{
	struct {
		uint16 num_stalls_dma;                // D2H mem2mem DMA busy
		uint16 dur_dma_busy;                  // duration DMA busy
	};
	uint32 u32[HWA_BLOCK_STATISTICS_WORDS];
} hwa_cpleng_common_stats_t;

// 2b, 4b block level Completion Entry Queue statistics
typedef union hwa_cpleng_cedq_stats
{
	struct {
		uint32 num_completions;               // number of cpl wi xfer to host
		uint32 num_interrupts;                // number of cpl ring interrupts
		uint32 num_aggr_counts;               // num ints gen due to aggr count
		uint32 num_aggr_tmouts;               // num ints gen due to aggr tmout
		uint32 num_expl_fw_intr;              // num explicit fw ints gen
		uint32 num_expl_host_intr;            // num explicit host ints gen
		uint32 num_host_ring_full;            // num CPL ring full stalls
	};
} hwa_cpleng_cedq_stats_t;

#else  /* ! HWA_CPLENG_BUILD */
#define HWA_RXCPLE_EXPR(expr)       HWA_NONE
#define HWA_TXCPLE_EXPR(expr)       HWA_NONE
#define HWA_CPLENG_EXPR(expr)       HWA_NONE
#endif /* ! HWA_CPLENG_BUILD */

#endif /* _HWA_DEFS_H */
