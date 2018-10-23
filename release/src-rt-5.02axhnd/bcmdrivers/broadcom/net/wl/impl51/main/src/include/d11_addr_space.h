/*
 * Chip/core-specific address space definitions, masks and other macros for d11 core
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: d11_addr_space.h 687689 2017-03-02 06:53:30Z $
 */

#ifndef	_D11_ADDR_SPACE_H
#define	_D11_ADDR_SPACE_H

/* Default values - need to be changed for when revision specifc changes are made */

/* for UCODE_IN_ROM_SUPPORT, 2 axi slave ports are expected */
#define D11_AXI_SP_IDX		(1) /* Index 0 and 1 (First and Second slave port) */
#define	D11_AXI_SP_ID		(2)

#define UCM_INSTR_WIDTH_BYTES	(8)
#define UCM_RAM_SIZE		(8 * 1024)
#define UCM_END			((UCM_RAM_SIZE * UCM_INSTR_WIDTH_BYTES) - 1)

#define PA_START_OFFSET		(0)
#define PIA_START_OFFSET	(UCM_RAM_SIZE * UCM_INSTR_WIDTH_BYTES)

/* pa address mask in pia[0]. Note that addr width is 13 bits in pia */
#define UCM_PA_ADDR_MASK	(0x1fff)

/* pia or pa minimum sizes */
#define UCM_PATCH_MIN_BYTES	(4)

/* at the end of pa or pia, there is 8 byte delimiter signifying end of patch
 * code. This is 8 byte zeroes. This has to be excluded from the patch
 * download.
 */
#define UCM_PATCH_TERM_SZ	(8)

/* pa will get written 4 bytes at a time as direct AXI register write for non-m2mdma case */
#define UCM_PATCH_WRITE_SZ	(4)

#define APB_UCM_BASE		(0x0)

#define AXI_ADDR_MASK		(0x7FFFFF)
#define APB_ADDR_MASK		(0xFFF)
#define COMMON_ADDR_MASK	(AXI_ADDR_MASK | APB_ADDR_MASK)

/* offsets of different memories based axi slave base, for core d11rev>60 /43012 */
#define AXISL_IHR_BASE		(0x00000000)
#define AXISL_IHR_SIZE		(0x0FFF)
#define AXISL_IHR_END		(AXISL_IHR_BASE + AXISL_IHR_SIZE)
#define AXISL_FCBS_BASE		(0x00002000)
#define AXISL_FCBS_SIZE		(0x001F)
#define AXISL_FCBS_END		(AXISL_FCBS_BASE + AXISL_FCBS_SIZE)
#define AXISL_SHM_BASE		(0x00004000)
#define AXISL_SHM_SIZE		(0x1FFF)
#define AXISL_SHM_END		(AXISL_SHM_BASE + AXISL_SHM_SIZE)
#define AXISL_SCR_BASE		(0x00009000)
#define AXISL_SCR_SIZE		(0x00FF)
#define AXISL_SCR_END		(AXISL_SCR_BASE + AXISL_SCR_SIZE)
#define AXISL_AMT_BASE		(0x0000B000)
#define AXISL_AMT_SIZE		(0x01FF)
#define AXISL_AMT_END		(AXISL_AMT_BASE + AXISL_AMT_SIZE)
#define AXISL_UCM_BASE		(0x00020000)
#define AXISL_UCM_SIZE		(0x07FF)
#define AXISL_UCM_END		(AXISL_UCM_BASE + AXISL_UCM_SIZE)
#define AXISL_BM_BASE		(0x00400000)
#define AXISL_BM_SIZE		(0x17FFF)
#define AXISL_BM_END		(AXISL_BM_BASE + AXISL_BM_SIZE)

#define IS_ADDR_AXISL_SUBSPACE(axi_base, addr, subspace)	\
	((addr) >= ((axi_base) + AXISL_ ## subspace ## _BASE) &&	\
	(addr) <= ((axi_base) + AXISL_ ## subspace ## _END))

#define IS_ADDR_AXISL(axi_base, addr)				\
	(IS_ADDR_AXISL_SUBSPACE((axi_base), (addr), IHR) ||	\
	IS_ADDR_AXISL_SUBSPACE((axi_base), (addr), FCBS) ||	\
	IS_ADDR_AXISL_SUBSPACE((axi_base), (addr), SHM)	||	\
	IS_ADDR_AXISL_SUBSPACE((axi_base), (addr), SCR)	||	\
	IS_ADDR_AXISL_SUBSPACE((axi_base), (addr), AMT)	||	\
	IS_ADDR_AXISL_SUBSPACE((axi_base), (addr), UCM)	||	\
	IS_ADDR_AXISL_SUBSPACE((axi_base), (addr), BM))
#endif	/* _D11_ADDR_SPACE_H */
