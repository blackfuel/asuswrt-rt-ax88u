/*
 *  BRCM SDIO HOST CONTROLLER driver
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
 * $Id: bcmsdbrcm.h 537479 2015-02-26 16:46:31Z $
 */

/* global msglevel for debug messages */
#ifdef BCMDBG
#define sd_err(x)	do { if (sd_msglevel & SDH_ERROR_VAL) printf x; } while (0)
#define sd_trace(x)	do { if (sd_msglevel & SDH_TRACE_VAL) printf x; } while (0)
#define sd_info(x)	do { if (sd_msglevel & SDH_INFO_VAL)  printf x; } while (0)
#define sd_debug(x)	do { if (sd_msglevel & SDH_DEBUG_VAL) printf x; } while (0)
#define sd_data(x)	do { if (sd_msglevel & SDH_DATA_VAL)  printf x; } while (0)
#define sd_ctrl(x)	do { if (sd_msglevel & SDH_CTRL_VAL)  printf x; } while (0)
#else
#define sd_err(x)
#define sd_trace(x)
#define sd_info(x)
#define sd_debug(x)
#define sd_data(x)
#define sd_ctrl(x)
#endif // endif

#ifdef BCMPERFSTATS
#define sd_log(x)	do { if (sd_msglevel & SDH_LOG_VAL)	 bcmlog x; } while (0)
#else
#define sd_log(x)
#endif // endif

/* SDIOH public information, used by per-port code */
struct sdioh_pubinfo {
	int 		local_intrcount;	/* Controller interrupt count */
	bool 		dev_init_done;		/* Client SDIO interface initted */
	bool		host_init_done;		/* Controller initted */
	bool		intr_registered;	/* Client handler registered */
	bool		dev_intr_enabled;	/* Device interrupt enabled/disabled */
	uint		lockcount;		/* Next count of sdbrcm_lock() calls */
	void 		*sdos_info;		/* Pointer to per-OS private data */
};

#define BLOCK_SIZE_4318 64
#define BLOCK_SIZE_4328 512

/* private bus modes */
/* move to API or hardware header. */
#define SDIOH_MODE_SPI		0
#define SDIOH_MODE_SD1		1
#define SDIOH_MODE_SD4		2

/* Expected dev status value for CMD7 */
#define SDIOH_CMD7_EXP_STATUS   0x00001E00

#define RETRIES_SMALL		20
#define RETRIES_LARGE		500000
#define SD_TIMEOUT		250000	/* Timeout after several ms if CMD not complete. */

#define ARVM_MASK		0xFF10

#define USE_PIO			0x0		/* DMA or PIO */
#define USE_DMA			0x1

#define NTXD			4
#define NRXD			4
#define RXBUFSZ			8192
#define NRXBUFPOST		4
#define	HWRXOFF			8

#define USE_BLOCKMODE		0x2		/* Block mode can be single block or multi */
#define USE_MULTIBLOCK		0x4

#define EXT_CLK			0xffffffff	/* external clock improve comment. */

extern int isr_sdbrcm_check_dev_intr(struct sdioh_pubinfo *sd);
extern uint sd_msglevel;

extern uint32 *sdbrcm_reg_map(osl_t *osh, int32 addr, int size);
extern void sdbrcm_reg_unmap(osl_t *osh, int32 addr, int size);
extern int sdbrcm_register_irq(sdioh_info_t *sd, uint irq);
extern void sdbrcm_free_irq(uint irq, sdioh_info_t *sd);

extern void sdbrcm_lock(sdioh_info_t *sd);
extern void sdbrcm_unlock(sdioh_info_t *sd);

extern void sdbrcm_devintr_on(sdioh_info_t *sd);
extern void sdbrcm_devintr_off(sdioh_info_t *sd);

/* Allocate/init/free per-OS private data */
extern int sdbrcm_osinit(sdioh_info_t *sd, osl_t *osh);
extern void sdbrcm_osfree(sdioh_info_t *sd, osl_t *osh);
