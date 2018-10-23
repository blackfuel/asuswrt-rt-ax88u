/*
 * RADIO control module public interface (to MAC driver).
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
 * $Id: phy_radio_api.h 742511 2018-01-22 14:14:24Z $
 */

#ifndef _phy_radio_api_h_
#define _phy_radio_api_h_

#include <typedefs.h>
#include <phy_api.h>

#define PHY_BOARD_2G_FEM_MASK 1
#define PHY_BOARD_5G_FEM_MASK 2
#define PHY_BOARD_2G_FEM_SHIFT 0
#define PHY_BOARD_5G_FEM_SHIFT 1

/* Radio macros */
/* Radio ID */
#define	IDCODE_VER_MASK		0x0000000f
#define	IDCODE_VER_SHIFT	0
#define	IDCODE_MFG_MASK		0x00000fff
#define	IDCODE_MFG_SHIFT	0
#define	IDCODE_ID_MASK		0x0ffff000
#define	IDCODE_ID_SHIFT		12
#define	IDCODE_REV_MASK		0xf0000000
#define	IDCODE_REV_SHIFT	28

#define	BCM2050_ID		0x2050
#define	BCM2050_IDCODE		0x02050000
#define	BCM2050A0_IDCODE	0x1205017f
#define	BCM2050A1_IDCODE	0x2205017f
#define	BCM2050R8_IDCODE	0x8205017f

#define BCM2055_ID		0x2055
#define BCM2055_IDCODE		0x02055000
#define BCM2055A0_IDCODE	0x1205517f

#define BCM2056_ID		0x2056
#define BCM2056_IDCODE		0x02056000
#define BCM2056A0_IDCODE	0x1205617f

#define BCM2057_ID		0x2057
#define BCM2057_IDCODE		0x02057000
#define BCM2057A0_IDCODE	0x1205717f

#define BCM2059_ID		0x2059
#define BCM2059A0_IDCODE	0x0205917f

#define	BCM2060_ID		0x2060
#define	BCM2060_IDCODE		0x02060000
#define	BCM2060WW_IDCODE	0x1206017f

#define BCM2062_ID		0x2062
#define BCM2062_IDCODE		0x02062000
#define BCM2062A0_IDCODE	0x0206217f

#define BCM2063_ID		0x2063
#define BCM2063_IDCODE		0x02063000
#define BCM2063A0_IDCODE	0x0206317f

#define BCM2064_ID		0x2064
#define BCM2064_IDCODE		0x02064000
#define BCM2064A0_IDCODE	0x0206417f

#define BCM2065_ID		0x2065
#define BCM2065_IDCODE		0x02065000
#define BCM2065A0_IDCODE	0x0206517f

#define BCM2067_ID		0x2067
#define BCM2067_IDCODE		0x02067000
#define BCM2067A0_IDCODE	0x0206717f

#define BCM2066_ID		0x2066
#define BCM2066_IDCODE		0x02066000
#define BCM2066A0_IDCODE	0x0206617f

#define BCM20671_ID		0x022e
#define BCM20671_IDCODE		0x0022e000
#define BCM20671A0_IDCODE	0x0022e17f
#define BCM20671A1_IDCODE	0x1022e17f
#define BCM20671B0_IDCODE	0x11022e17f

#define BCM2069_ID		0x2069
#define BCM2069A0_IDCODE	0x02069000

#define BCM20691_ID		0x30b
#define BCM20692_ID		0x3da
#define BCM20693_ID		0x3eb
#define BCM20694_ID		0x45a
#define BCM20695_ID		0x53a
#define BCM20696_ID		0x58e
#define BCM20697_ID		0x5fb
#define BCM20698_ID		0x5fc
#define BCM20704_ID		0x50e0

/* AFE Override */
#define PHY_AFE_OVERRIDE_USR	1
#define PHY_AFE_OVERRIDE_DRV	2

#define PHY_BOARD_2G_FEM(pi) \
	((phy_get_board_bandcap(pi) & PHY_BOARD_2G_FEM_MASK) >> \
	 PHY_BOARD_2G_FEM_SHIFT)

#define PHY_BOARD_5G_FEM(pi) \
	((phy_get_board_bandcap(pi) & PHY_BOARD_5G_FEM_MASK) >> \
	 PHY_BOARD_5G_FEM_SHIFT)

/* switch the radio on/off */
void phy_radio_switch(phy_info_t *pi, bool on);

/* switch the radio off when switching band */
void phy_radio_xband(phy_info_t *pi);

/* switch the radio off when initializing */
void phy_radio_init(phy_info_t *pi);

/* query board fem capability */
uint8 phy_get_board_bandcap(phy_info_t *pi);

/* init power on reset */
void phy_radio_por_inform(wlc_phy_t *ppi);

/* run bist config */
void phy_radio_runbist_config(wlc_phy_t *ppi, bool start_end);
#endif /* _phy_radio_api_h_ */
