/*
 * Chip-specific hardware definitions for
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
 * $Id: $
 */

#ifndef	_D11REGS_H
#define	_D11REGS_H

#include <typedefs.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <sbconfig.h>

#if defined(BCMDONGLEHOST) || defined(WL_UNITTEST)
typedef struct {
	uint32 pad;
} d11regdefs_t;

typedef volatile uint8 d11regs_t;
typedef struct _d11regs_info {
	uint32 pad;
} d11regs_info_t;

#define D11_gptimer(x)		(0x18)
#define D11_TSFTimerLow(x)	(0x180)
#else	/* defined(BCMDONGLEHOST)|| defined(WL_UNITTEST) */
#include <d11regsoffs.h>

typedef struct _d11regs_info {
	d11regs_t *regs;
	const d11regdefs_t *regoffsets;
} d11regs_info_t;

#endif /* !defined(BCMDONGLEHOST)|| !defined(WL_UNITTEST) */

typedef volatile struct {
	uint32	intstatus;
	uint32	intmask;
} intctrlregs_t;

/**
 * read: 32-bit register that can be read as 32-bit or as 2 16-bit
 * write: only low 16b-it half can be written
 */
typedef volatile union {
	uint32 pmqhostdata;		/**< read only! */
	struct {
		uint16 pmqctrlstatus;	/**< read/write */
		uint16 PAD;
	} w;
} pmqreg_t;

/** pio register set 2/4 bytes union for d11 fifo */
typedef volatile union {
	pio2regp_t	b2;		/**< < corerev 8 */
	pio4regp_t	b4;		/**< >= corerev 8 */
} u_pioreg_t;

/** dma/pio corerev >= 11 */
typedef volatile struct {
	dma64regs_t	dmaxmt;		/* dma tx */
	pio4regs_t	piotx;		/* pio tx */
	dma64regs_t	dmarcv;		/* dma rx */
	pio4regs_t	piorx;		/* pio rx */
} fifo64_t;

/** indirect dma corerev >= 64 */
typedef volatile struct {
	dma64regs_t	dma;		/**< dma tx */
	uint32		indintstatus;
	uint32		indintmask;
} ind_dma_t;

#define GET_MACINTSTATUS(osh, hw)		R_REG((osh), D11_MACINTSTATUS(hw))
#define SET_MACINTSTATUS(osh, hw, val)		W_REG((osh), D11_MACINTSTATUS(hw), (val))
#define GET_MACINTMASK(osh, hw)			R_REG((osh), D11_MACINTMASK(hw))
#define SET_MACINTMASK(osh, hw, val)		W_REG((osh), D11_MACINTMASK(hw), (val))

#define GET_MACINTSTATUS_X(osh, hw)		R_REG((osh), D11_MACINTSTATUS_psmx(hw))
#define SET_MACINTSTATUS_X(osh, hw, val)	W_REG((osh), D11_MACINTSTATUS_psmx(hw), (val))
#define GET_MACINTMASK_X(osh, hw)		R_REG((osh), D11_MACINTMASK_psmx(hw))
#define SET_MACINTMASK_X(osh, hw, val)		W_REG((osh), D11_MACINTMASK_psmx(hw), (val))

#define GET_MACINTSTATUS_R1(osh, hw)		R_REG((osh), D11_MACINTSTATUS_R1(hw))
#define SET_MACINTSTATUS_R1(osh, hw, val)	W_REG((osh), D11_MACINTSTATUS_R1(hw), (val))
#define GET_MACINTMASK_R1(osh, hw)		R_REG((osh), D11_MACINTMASK_R1(hw))
#define SET_MACINTMASK_R1(osh, hw, val)		W_REG((osh), D11_MACINTMASK_R1(hw), (val))

#define D11Reggrp_intctrlregs(hw, ix) ((intctrlregs_t*)(((volatile uint8*)D11_intstat0(hw)) + \
	(sizeof(intctrlregs_t)*ix)))
#define D11Reggrp_inddma(hw, ix) ((ind_dma_t*)(((volatile uint8*)D11_inddma(hw)) + \
	(sizeof(ind_dma_t)*ix)))
#define D11Reggrp_indaqm(hw, ix) ((ind_dma_t*)(((volatile uint8*)D11_indaqm(hw)) + \
	(sizeof(ind_dma_t)*ix)))
#define D11Reggrp_pmqreg(hw, ix) ((pmqreg_t*)(((volatile uint8*)D11_PMQHOSTDATA(hw)) + \
	(sizeof(pmqreg_t)*ix)))
#define D11Reggrp_f64regs(hw, ix) ((fifo64_t*)(((volatile uint8*)D11_xmt0ctl(hw)) + \
	(sizeof(fifo64_t)*ix)))
#define D11Reggrp_dmafifo(hw, ix) ((dma32diag_t*)(((volatile uint8*)D11_fifobase(hw)) + \
	(sizeof(dma32diag_t)*ix)))
#define D11Reggrp_intrcvlazy(hw, ix) ((volatile uint32*)(((volatile uint8*)D11_intrcvlzy0(hw)) + \
	(sizeof(uint32)*ix)))
#define D11Reggrp_altintmask(hw, ix) ((volatile uint32*)(((volatile uint8*)D11_alt_intmask0(hw)) + \
	(sizeof(uint32)*ix)))

#endif	/* _D11REGS_H */
