/*
 * PHY modules debug utilities
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
 * $Id: phy_dbg.h 692080 2017-03-25 01:24:18Z $
 */

#ifndef _phy_dbg_h_
#define _phy_dbg_h_

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_dump_reg.h>
#include <phy_api.h>
#include <phy_dbg_api.h>

#include <phyioctl_defs.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif

#if defined(BCMDBG) || defined(WL_MACDBG) || defined(WLTEST) || defined(WLC_SRAMPMAC) \
	|| defined(BCMDBG_PHYDUMP) || defined(PHYTXERR_DUMP)
	#define PHY_DBG_ENABLED
#endif // endif

extern uint32 phyhal_msg_level;

#if defined(BCMDBG) && !defined(BCMDONGLEHOST) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
char *wlc_dbg_get_hw_timestamp(void);
#define PHY_TIMESTAMP()	do {						\
		if (phyhal_msg_level & PHYHAL_TIMESTAMP) {		\
			printf("%s", wlc_dbg_get_hw_timestamp());	\
		}							\
	} while (0)
#else
#define PHY_TIMESTAMP()
#endif // endif

#if defined(BCMTSTAMPEDLOGS)
extern void phy_log(phy_info_t *pi, const char* str, uint32 p1, uint32 p2);
#else
#define phy_log(wlc, str, p1, p2)       do {} while (0)
#endif // endif

#define PHY_PRINT(args) do { PHY_TIMESTAMP(); printf args; } while (0)

#if defined(EVENT_LOG_COMPILE) && defined(BCMDBG_ERR) && defined(ERR_USE_EVENT_LOG)
#define	PHY_ERROR(args)		do { \
				if (phyhal_msg_level & PHYHAL_ERROR) { \
				EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_PHY_ERROR, args); }\
				} while (0)
#elif defined(BCMDBG_ERR) || defined(BCMDBG) || defined(PHYDBG)
#define	PHY_ERROR(args)	do {if (phyhal_msg_level & PHYHAL_ERROR) PHY_PRINT(args);} while (0)
#else
#define	PHY_ERROR(args)
#endif /* defined(BCMDBG_ERR) && defined(ERR_USE_EVENT_LOG) */

#if defined(BCMDBG) || defined(PHYDBG)
#define	PHY_TRACE(args)	do {if (phyhal_msg_level & PHYHAL_TRACE) PHY_PRINT(args);} while (0)
#define	PHY_INFORM(args) do {if (phyhal_msg_level & PHYHAL_INFORM) PHY_PRINT(args);} while (0)
#define	PHY_TMP(args)	do {if (phyhal_msg_level & PHYHAL_TMP) PHY_PRINT(args);} while (0)
#define	PHY_TXPWR(args)	do {if (phyhal_msg_level & PHYHAL_TXPWR) PHY_PRINT(args);} while (0)
#define	PHY_CAL(args)	do {if (phyhal_msg_level & PHYHAL_CAL) PHY_PRINT(args);} while (0)
#define	PHY_ACI(args)	do {if (phyhal_msg_level & PHYHAL_ACI) PHY_PRINT(args);} while (0)
#define	PHY_RADAR(args)	do {if (phyhal_msg_level & PHYHAL_RADAR) PHY_PRINT(args);} while (0)
#define PHY_THERMAL(args) do {if (phyhal_msg_level & PHYHAL_THERMAL) PHY_PRINT(args);} while (0)
#define PHY_PAPD(args)	do {if (phyhal_msg_level & PHYHAL_PAPD) PHY_PRINT(args);} while (0)
#define PHY_FCBS(args)	do {if (phyhal_msg_level & PHYHAL_FCBS) PHY_PRINT(args);} while (0)
#define PHY_RXIQ(args)	do {if (phyhal_msg_level & PHYHAL_RXIQ) PHY_PRINT(args);} while (0)
#define PHY_WD(args)	do {if (phyhal_msg_level & PHYHAL_WD) PHY_PRINT(args);} while (0)
#define PHY_CHANLOG(w, s, i, j)  \
	do {if (phyhal_msg_level & PHYHAL_CHANLOG) phy_log(w, s, i, j);} while (0)

#define	PHY_NONE(args)	do {} while (0)
#else
#define	PHY_TRACE(args)
#define	PHY_INFORM(args)
#define	PHY_TMP(args)
#define	PHY_TXPWR(args)
#define	PHY_CAL(args)
#define	PHY_ACI(args)
#define	PHY_RADAR(args)
#define PHY_THERMAL(args)
#define PHY_PAPD(args)
#define PHY_FCBS(args)
#define PHY_RXIQ(args)
#define PHY_WD(args)
#define PHY_CHANLOG(w, s, i, j)
#define	PHY_NONE(args)
#endif /* BCMDBG || PHYDBG */

#define PHY_INFORM_ON()		(phyhal_msg_level & PHYHAL_INFORM)
#define PHY_THERMAL_ON()	(phyhal_msg_level & PHYHAL_THERMAL)
#define PHY_CAL_ON()		(phyhal_msg_level & PHYHAL_CAL)

#define PHY_FATAL_ERROR_MESG(args)	do {PHY_PRINT(args);} while (0)
#define PHY_FATAL_ERROR(pi, reason_code) phy_fatal_error(pi, reason_code)
void phy_fatal_error(phy_info_t *pi, phy_crash_reason_t reason_code);

/* PHY and RADIO register binary dump data types */
#define PHYRADDBG1_TYPE       1

/* PHYRADDBG1_TYPE structure definition */
/* Note: bmp_cnt has either bitmap or count, if the MSB (bit 31) is set, then
*   bmp_cnt[30:0] has count, i.e, number of valid registers whose values are
*   contigous from the start address. If MSB is zero, then the value
*   should be considered as a bitmap of 31 discreet addresses from the base addr.
*   The data type for bmp_cnt is chosen as an array of uint8 to avoid
*   padding. A uint32 type will lead to 2bytes of padding which will consume
*   ~20% more space per table
*/
typedef struct _phyradregs_bmp_list {
	uint16 addr;		/* start address */
	uint8 bmp_cnt[4];	/* bit[31]=1, bit[30:0] is count else it is a bitmap */
} phyradregs_list_t;

typedef struct phy_dbg_info phy_dbg_info_t;

/* attach/detach */
phy_dbg_info_t *phy_dbg_attach(phy_info_t *pi);
void phy_dbg_detach(phy_dbg_info_t *di);

/* add a dump and/or dump clear callback(s) */
typedef wlc_dump_reg_dump_fn_t phy_dump_dump_fn_t;
typedef wlc_dump_reg_clr_fn_t phy_dump_clr_fn_t;
int phy_dbg_add_dump_fns(phy_info_t *pi, const char *name,
	phy_dump_dump_fn_t fn, phy_dump_clr_fn_t clr, void *ctx);
/* keep the old style API for all existing callers who don't have dump clear fn */
#define phy_dbg_add_dump_fn(pi, name, fn, ctx) \
	phy_dbg_add_dump_fns(pi, name, fn, NULL, ctx)

#if defined(BCMDBG) || defined(WLTEST)
void phy_dbg_test_evm_init(phy_info_t *pi);
uint16 phy_dbg_test_evm_reg(uint rate);
int phy_dbg_test_evm(phy_info_t *pi, int channel, uint rate, int txpwr);
int phy_dbg_test_carrier_suppress(phy_info_t *pi, int channel);
#endif // endif

#if defined(PHY_DUMP_BINARY)
int phy_dbg_regval_get(phy_dbg_info_t *dbgi, void *data, int length);
void phy_dbg_regval_set_type(phy_dbg_info_t *dbgi, uint8 type);
#endif /* PHY_DUMP_BINARY */

#endif /* _phy_dbg_h_ */
