/*
 * RADIO control module internal interface (to other PHY modules).
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
 * $Id: phy_radio.h 689894 2017-03-13 23:32:18Z $
 */

#ifndef _phy_radio_h_
#define _phy_radio_h_

#include <typedefs.h>
#include <phy_api.h>
#include <phy_dbg.h>

/* forward declaration */
typedef struct phy_radio_info phy_radio_info_t;

typedef struct radio_20xx_regs {
	uint16 address;
	uint16  init;
	uint8  do_init;
} radio_20xx_regs_t;

extern radio_20xx_regs_t regs_2057_rev4[], regs_2057_rev5[], regs_2057_rev5v1[];
extern radio_20xx_regs_t regs_2057_rev7[], regs_2057_rev8[];
extern radio_20xx_regs_t regs_2057_rev12[];
extern radio_20xx_regs_t regs_2057_rev13[];
extern radio_20xx_regs_t regs_2057_rev14[];
extern radio_20xx_regs_t regs_2057_rev14v1[];
extern radio_20xx_regs_t regs_2069_rev0[];

/* attach/detach */
phy_radio_info_t *phy_radio_attach(phy_info_t *pi);
void phy_radio_detach(phy_radio_info_t *ri);

/* query radio idcode */
uint32 phy_radio_query_idcode(phy_radio_info_t *ri);

#ifdef PHY_DUMP_BINARY
/* get radio register list and size */
int phy_radio_getlistandsize(phy_info_t *pi, phyradregs_list_t **phyreglist, uint16 *phyreglist_sz);
#endif // endif
#ifdef RADIO_HEALTH_CHECK
bool phy_radio_pll_lock(phy_radio_info_t *radioi);
#endif /* RADIO_HEALTH_CHECK */
uint phy_radio_init_radio_regs_allbands(phy_info_t *pi, const radio_20xx_regs_t *radioregs);
#endif /* _phy_radio_h_ */
