/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#include "ru.h"

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_MACTYPE
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG = 
{
    "COMMON_CONFIGURATIONS_MACTYPE",
#if RU_INCLUDE_DESC
    "MAC_TYPE Register",
    "The BBH supports working with different MAC types. Each MAC requires different interface and features. This register defines the type of MAC the BBH works with.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG_OFFSET,
    0,
    0,
    842,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG = 
{
    "COMMON_CONFIGURATIONS_BBCFG_1_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_1 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG_OFFSET,
    0,
    0,
    843,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG = 
{
    "COMMON_CONFIGURATIONS_BBCFG_2_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_2 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG_OFFSET,
    0,
    0,
    844,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG = 
{
    "COMMON_CONFIGURATIONS_DDRCFG_TX",
#if RU_INCLUDE_DESC
    "RD_ADDR_CFG Register",
    "Configurations for determining the address to read from the DDR/PSRAm",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG_OFFSET,
    0,
    0,
    845,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG = 
{
    "COMMON_CONFIGURATIONS_RNRCFG_1",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_RAM_CNT,
    4,
    846,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG = 
{
    "COMMON_CONFIGURATIONS_RNRCFG_2",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_RAM_CNT,
    4,
    847,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG = 
{
    "COMMON_CONFIGURATIONS_DMACFG_TX",
#if RU_INCLUDE_DESC
    "DMA_CFG Register",
    "The BBH reads the packet data from the DDR in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the DMA memory space. The read descriptors are arranged in a predefined space in the DMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG_OFFSET,
    0,
    0,
    848,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG = 
{
    "COMMON_CONFIGURATIONS_SDMACFG_TX",
#if RU_INCLUDE_DESC
    "SDMA_CFG Register",
    "The BBH reads the packet data from the PSRAM in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the SDMA memory space. The read descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG_OFFSET,
    0,
    0,
    849,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG = 
{
    "COMMON_CONFIGURATIONS_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "When packet transmission is done, the BBH releases the SBPM buffers."
    "This register defines which release command is used:"
    "1. Normal free with context"
    "2. Special free with context"
    "3. free without context",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG_OFFSET,
    0,
    0,
    850,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG = 
{
    "COMMON_CONFIGURATIONS_DDRTMBASEL",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_LOW %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_RAM_CNT,
    4,
    851,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG = 
{
    "COMMON_CONFIGURATIONS_DDRTMBASEH",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_HIGH %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_RAM_CNT,
    4,
    852,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG = 
{
    "COMMON_CONFIGURATIONS_DFIFOCTRL",
#if RU_INCLUDE_DESC
    "DATA_FIFO_CTRL Register",
    "The BBH orders data both from DDR and PSRAM. The returned data is stored in two FIFOs for reordering. The two FIFOs are implemented in a single RAM. This register defines the division of the RAM to two FIFOs.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG_OFFSET,
    0,
    0,
    853,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG = 
{
    "COMMON_CONFIGURATIONS_ARB_CFG",
#if RU_INCLUDE_DESC
    "ARB_CFG Register",
    "configurations related to different arbitration processes (ordering PDs, ordering data)",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG_OFFSET,
    0,
    0,
    854,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_BBROUTE
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG = 
{
    "COMMON_CONFIGURATIONS_BBROUTE",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "override configuration for the route of one of the peripherals (DMA/SDMMA/FPM/SBPM?Runners)",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG_OFFSET,
    0,
    0,
    855,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_Q2RNR
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG = 
{
    "COMMON_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR %i Register",
    "configuration which queue is managed by each of the two runners."
    ""
    "Each register in this array configures 2 queues.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG_OFFSET,
    BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    856,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG = 
{
    "COMMON_CONFIGURATIONS_TXRSTCMD",
#if RU_INCLUDE_DESC
    "TX_RESET_COMMAND Register",
    "This register enables reset of internal units (for possible WA purposes).",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG_OFFSET,
    0,
    0,
    857,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_DBGSEL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG = 
{
    "COMMON_CONFIGURATIONS_DBGSEL",
#if RU_INCLUDE_DESC
    "DEBUG_SELECT Register",
    "This register selects 1 of 8 debug vectors."
    "The selected vector is reflected to DBGOUTREG.",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG_OFFSET,
    0,
    0,
    858,
};

/******************************************************************************
 * Register: BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG = 
{
    "COMMON_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    859,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDBASE
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG = 
{
    "WAN_CONFIGURATIONS_PDBASE",
#if RU_INCLUDE_DESC
    "PD_FIFO_BASE %i Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the Status FIFO, 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG_RAM_CNT,
    4,
    860,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "WAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE %i Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG_RAM_CNT,
    4,
    861,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "WAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH %i Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "Each register defines the threshold of 2 queues."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG_RAM_CNT,
    4,
    862,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "WAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD %i Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration for the rest (TCONTs 8-39)."
    "Each register in this array defines the threshold of 2 queues.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG_RAM_CNT,
    4,
    863,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG = 
{
    "WAN_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET,
    0,
    0,
    864,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG = 
{
    "WAN_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues)."
    "This configuration is global for all queues."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG_OFFSET,
    0,
    0,
    865,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG = 
{
    "WAN_CONFIGURATIONS_STSRNRCFG_1",
#if RU_INCLUDE_DESC
    "STS_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG_RAM_CNT,
    4,
    866,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG = 
{
    "WAN_CONFIGURATIONS_STSRNRCFG_2",
#if RU_INCLUDE_DESC
    "STS_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG_RAM_CNT,
    4,
    867,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG = 
{
    "WAN_CONFIGURATIONS_MSGRNRCFG_1",
#if RU_INCLUDE_DESC
    "MSG_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG_RAM_CNT,
    4,
    868,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG = 
{
    "WAN_CONFIGURATIONS_MSGRNRCFG_2",
#if RU_INCLUDE_DESC
    "MSG_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG_OFFSET,
    BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG_RAM_CNT,
    4,
    869,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_EPNCFG
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG = 
{
    "WAN_CONFIGURATIONS_EPNCFG",
#if RU_INCLUDE_DESC
    "EPN_CFG Register",
    "Configurations related to EPON MAC.",
#endif
    BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG_OFFSET,
    0,
    0,
    870,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG = 
{
    "WAN_CONFIGURATIONS_FLOW2PORT",
#if RU_INCLUDE_DESC
    "FLOW2PORT Register",
    "interface for SW to access the flow id to port-id table",
#endif
    BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG_OFFSET,
    0,
    0,
    871,
};

/******************************************************************************
 * Register: BBH_TX_WAN_CONFIGURATIONS_TS
 ******************************************************************************/
const ru_reg_rec BBH_TX_WAN_CONFIGURATIONS_TS_REG = 
{
    "WAN_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "enable to the 1588 interface in X/EPON mode",
#endif
    BBH_TX_WAN_CONFIGURATIONS_TS_REG_OFFSET,
    0,
    0,
    872,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDBASE
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDBASE_REG = 
{
    "LAN_CONFIGURATIONS_PDBASE",
#if RU_INCLUDE_DESC
    "PD_FIFO_BASE Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDBASE_REG_OFFSET,
    0,
    0,
    873,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "LAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    0,
    0,
    874,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "LAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "Each register defines the threshold of 2 queues."
    "",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    0,
    0,
    875,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "LAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration for the rest (TCONTs 8-39)."
    "Each register in this array defines the threshold of 2 queues.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    0,
    0,
    876,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG = 
{
    "LAN_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET,
    0,
    0,
    877,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_REG = 
{
    "LAN_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues)."
    "This configuration is global for all queues."
    "Relevant only for EPON BBH.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_REG_OFFSET,
    0,
    0,
    878,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_REG = 
{
    "LAN_CONFIGURATIONS_TXTHRESH",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD Register",
    "Transmit threshold in 8 bytes resolution."
    "The BBH TX will not start to transmit data towards the XLMAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_REG_OFFSET,
    0,
    0,
    879,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_EEE
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_EEE_REG = 
{
    "LAN_CONFIGURATIONS_EEE",
#if RU_INCLUDE_DESC
    "EEE Register",
    "The BBH is responsible for indicating the XLMAC that no traffic is about to arrive so the XLMAC may try to enter power saving mode."
    ""
    "This register is used to enable this feature.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_EEE_REG_OFFSET,
    0,
    0,
    880,
};

/******************************************************************************
 * Register: BBH_TX_LAN_CONFIGURATIONS_TS
 ******************************************************************************/
const ru_reg_rec BBH_TX_LAN_CONFIGURATIONS_TS_REG = 
{
    "LAN_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the XLMAC that it should and calculate timestamp for the current packet that is being transmitted. The BBH gets the timestamping parameters in the PD and forward it to the XLMAC."
    ""
    "This register is used to enable this feature.",
#endif
    BBH_TX_LAN_CONFIGURATIONS_TS_REG_OFFSET,
    0,
    0,
    881,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMPD
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SRAMPD_REG = 
{
    "DEBUG_COUNTERS_SRAMPD",
#if RU_INCLUDE_DESC
    "SRAM_PD_COUNTER Register",
    "This counter counts the number of received PD for packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMPD_REG_OFFSET,
    0,
    0,
    882,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRPD
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DDRPD_REG = 
{
    "DEBUG_COUNTERS_DDRPD",
#if RU_INCLUDE_DESC
    "DDR_PD_COUNTER Register",
    "This counter counts the number of received PDs for packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRPD_REG_OFFSET,
    0,
    0,
    883,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_PDDROP
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_PDDROP_REG = 
{
    "DEBUG_COUNTERS_PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP_COUNTER Register",
    "This counter counts the number of PDs which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_PDDROP_REG_OFFSET,
    0,
    0,
    884,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_STSCNT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_STSCNT_REG = 
{
    "DEBUG_COUNTERS_STSCNT",
#if RU_INCLUDE_DESC
    "STS_COUNTER Register",
    "This counter counts the number of STS messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_STSCNT_REG_OFFSET,
    0,
    0,
    885,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_STSDROP
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_STSDROP_REG = 
{
    "DEBUG_COUNTERS_STSDROP",
#if RU_INCLUDE_DESC
    "STS_DROP_COUNTER Register",
    "This counter counts the number of STS which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_STSDROP_REG_OFFSET,
    0,
    0,
    886,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_MSGCNT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_MSGCNT_REG = 
{
    "DEBUG_COUNTERS_MSGCNT",
#if RU_INCLUDE_DESC
    "MSG_COUNTER Register",
    "This counter counts the number of MSG (DBR/Ghost) messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_MSGCNT_REG_OFFSET,
    0,
    0,
    887,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_MSGDROP
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_MSGDROP_REG = 
{
    "DEBUG_COUNTERS_MSGDROP",
#if RU_INCLUDE_DESC
    "MSG_DROP_COUNTER Register",
    "This counter counts the number of MSG which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_MSGDROP_REG_OFFSET,
    0,
    0,
    888,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_GETNEXTNULL
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG = 
{
    "DEBUG_COUNTERS_GETNEXTNULL",
#if RU_INCLUDE_DESC
    "GET_NEXT_IS_NULL_COUNTER Register",
    "This counter counts the number Get next responses with a null BN."
    "It counts the packets for all TCONTs together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "This counter is relevant for Ethernet only.",
#endif
    BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG_OFFSET,
    0,
    0,
    889,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_FLUSHPKTS
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG = 
{
    "DEBUG_COUNTERS_FLUSHPKTS",
#if RU_INCLUDE_DESC
    "FLUSHED_PACKETS_COUNTER Register",
    "This counter counts the number of packets that were flushed (bn was released without sending the data to the EPON MAC) due to flush request."
    "The counter is global for all queues."
    "The counter is read clear.",
#endif
    BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG_OFFSET,
    0,
    0,
    890,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_LENERR
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_LENERR_REG = 
{
    "DEBUG_COUNTERS_LENERR",
#if RU_INCLUDE_DESC
    "REQ_LENGTH_ERROR_COUNTER Register",
    "This counter counts the number of times a length error (mismatch between a request from the MAC and a PD from the Runner) occured."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    BBH_TX_DEBUG_COUNTERS_LENERR_REG_OFFSET,
    0,
    0,
    891,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_AGGRLENERR
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG = 
{
    "DEBUG_COUNTERS_AGGRLENERR",
#if RU_INCLUDE_DESC
    "AGGREGATION_LENGTH_ERROR_COUNTER Register",
    "This counter Counts aggregation length error events."
    "If one or more of the packets in an aggregated PD is shorter than 60 bytes, this counter will be incremented by 1."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG_OFFSET,
    0,
    0,
    892,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMPKT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG = 
{
    "DEBUG_COUNTERS_SRAMPKT",
#if RU_INCLUDE_DESC
    "SRAM_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG_OFFSET,
    0,
    0,
    893,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRPKT
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DDRPKT_REG = 
{
    "DEBUG_COUNTERS_DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRPKT_REG_OFFSET,
    0,
    0,
    894,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SRAMBYTE
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SRAMBYTE_REG = 
{
    "DEBUG_COUNTERS_SRAMBYTE",
#if RU_INCLUDE_DESC
    "SRAM_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the SRAM."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_SRAMBYTE_REG_OFFSET,
    0,
    0,
    895,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DDRBYTE
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DDRBYTE_REG = 
{
    "DEBUG_COUNTERS_DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the DDR."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    BBH_TX_DEBUG_COUNTERS_DDRBYTE_REG_OFFSET,
    0,
    0,
    896,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDEN
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDEN_REG = 
{
    "DEBUG_COUNTERS_SWRDEN",
#if RU_INCLUDE_DESC
    "SW_RD_EN Register",
    "writing to this register creates a rd_en pulse to the selected array the SW wants to access."
    ""
    "Each bit in the register represents one of the arrays the SW can access."
    ""
    "The address inside the array is determined in the previous register (sw_rd_address)."
    ""
    "When writing to this register the SW should assert only one bit. If more than one is asserted, The HW will return the value read from the lsb selected array.",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDEN_REG_OFFSET,
    0,
    0,
    897,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDADDR
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG = 
{
    "DEBUG_COUNTERS_SWRDADDR",
#if RU_INCLUDE_DESC
    "SW_RD_ADDR Register",
    "the address inside the array the SW wants to read",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG_OFFSET,
    0,
    0,
    898,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_SWRDDATA
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG = 
{
    "DEBUG_COUNTERS_SWRDDATA",
#if RU_INCLUDE_DESC
    "SW_RD_DATA Register",
    "indirect memories and arrays read data",
#endif
    BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG_OFFSET,
    0,
    0,
    899,
};

/******************************************************************************
 * Register: BBH_TX_DEBUG_COUNTERS_DBGOUTREG
 ******************************************************************************/
const ru_reg_rec BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG = 
{
    "DEBUG_COUNTERS_DBGOUTREG",
#if RU_INCLUDE_DESC
    "DEBUG_OUT_REG %i Register",
    "an array including all the debug vectors of the BBH TX",
#endif
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_OFFSET,
    BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG_RAM_CNT,
    4,
    900,
};

/******************************************************************************
 * Block: BBH_TX
 ******************************************************************************/
static const ru_reg_rec *BBH_TX_REGS[] =
{
    &BBH_TX_COMMON_CONFIGURATIONS_MACTYPE_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_1_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_RNRCFG_2_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DMACFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_SBPMCFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_ARB_CFG_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_BBROUTE_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_Q2RNR_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_TXRSTCMD_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_DBGSEL_REG,
    &BBH_TX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDBASE_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_WAN_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_1_REG,
    &BBH_TX_WAN_CONFIGURATIONS_STSRNRCFG_2_REG,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_1_REG,
    &BBH_TX_WAN_CONFIGURATIONS_MSGRNRCFG_2_REG,
    &BBH_TX_WAN_CONFIGURATIONS_EPNCFG_REG,
    &BBH_TX_WAN_CONFIGURATIONS_FLOW2PORT_REG,
    &BBH_TX_WAN_CONFIGURATIONS_TS_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDBASE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDSIZE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDWKUPH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &BBH_TX_LAN_CONFIGURATIONS_PDEMPTY_REG,
    &BBH_TX_LAN_CONFIGURATIONS_TXTHRESH_REG,
    &BBH_TX_LAN_CONFIGURATIONS_EEE_REG,
    &BBH_TX_LAN_CONFIGURATIONS_TS_REG,
    &BBH_TX_DEBUG_COUNTERS_SRAMPD_REG,
    &BBH_TX_DEBUG_COUNTERS_DDRPD_REG,
    &BBH_TX_DEBUG_COUNTERS_PDDROP_REG,
    &BBH_TX_DEBUG_COUNTERS_STSCNT_REG,
    &BBH_TX_DEBUG_COUNTERS_STSDROP_REG,
    &BBH_TX_DEBUG_COUNTERS_MSGCNT_REG,
    &BBH_TX_DEBUG_COUNTERS_MSGDROP_REG,
    &BBH_TX_DEBUG_COUNTERS_GETNEXTNULL_REG,
    &BBH_TX_DEBUG_COUNTERS_FLUSHPKTS_REG,
    &BBH_TX_DEBUG_COUNTERS_LENERR_REG,
    &BBH_TX_DEBUG_COUNTERS_AGGRLENERR_REG,
    &BBH_TX_DEBUG_COUNTERS_SRAMPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_DDRPKT_REG,
    &BBH_TX_DEBUG_COUNTERS_SRAMBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_DDRBYTE_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDEN_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDADDR_REG,
    &BBH_TX_DEBUG_COUNTERS_SWRDDATA_REG,
    &BBH_TX_DEBUG_COUNTERS_DBGOUTREG_REG,
};

unsigned long BBH_TX_ADDRS[] =
{
    0x82d00000,
    0x82d04000,
    0x82d08000,
    0x82d0c000,
    0x82d10000,
    0x82d14000,
    0x82d18000,
    0x82d1c000,
    0x80170000,
};

const ru_block_rec BBH_TX_BLOCK = 
{
    "BBH_TX",
    BBH_TX_ADDRS,
    9,
    59,
    BBH_TX_REGS
};

/* End of file XRDP_BBH_TX.c */
