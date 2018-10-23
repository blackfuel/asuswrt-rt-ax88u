/*
 * ACPHY High Efficiency 802.11ax (HE) module implementation
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
 * $Id: $
 */

#ifdef WL11AX

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_hecap.h>
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <phy_ac_hecap.h>
#include <phy_hecap_api.h>
#include <phy_utils_reg.h>
#include <wlc_phyreg_ac.h>
#include <802.11ax.h>

/* Beamformee STS for <= 80MHz */
#define HE_PHY_BFE_STS_BELOW80		0x3
/* Beamformee STS for > 80MHz */
#define HE_PHY_BFE_STS_ABOVE80		0x3
/* Total STS for <= 80MHz */
#define HE_PHY_NSTS_BELOW80		0x3
/* Number of sounding dimensions for <= 80MHz */
#define HE_PHY_SOUND_DIM_BELOW80	0x3
/* Number of sounding dimensions for > 80MHz */
#define HE_PHY_SOUND_DIM_ABOVE80	0x3
/* Maximum Nc for beamforming sounding feedback */
#define HE_PHY_MAC_NC			0x3

/* module private states */
struct phy_ac_hecap_info {
	phy_info_t *pi;
	phy_ac_info_t *pi_ac;
	phy_hecap_info_t *hecap_info;
	uint32 phy_he_caps;		/* Capabilities from HE specific registers */
	uint32 phy_he_caps1;		/* Capabilities from HE specific registers */
};

/**
 * 11ax D1.4 - 9.4.2.237.4 Supported HE-MCS And NSS Set field
 */

/**
 * 43684a0:
 * 80MHz: tx/rx max mcs 0..11x4
 * 80p80 & 160: not supported
 */
static const wlc_he_rateset_t he_rateset_rev47 = {
	/* .bw80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_0_11 << 4)  | (HE_CAP_MAX_MCS_0_11 << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_0_11 << 4)  | (HE_CAP_MAX_MCS_0_11 << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14)
};

/**
 * 43684b0:
 * 80MHz, 160Mhz: tx/rx max mcs 0..11x4
 * 80p80: not supported
 */
static const wlc_he_rateset_t he_rateset_rev47_1 = {
	/* .bw80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_0_11 << 4)  | (HE_CAP_MAX_MCS_0_11 << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_0_11 << 4)  | (HE_CAP_MAX_MCS_0_11 << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_0_11 << 4)  | (HE_CAP_MAX_MCS_0_11 << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_0_11 << 4)  | (HE_CAP_MAX_MCS_0_11 << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14)
};

/**
 * 63178:
 * 80MHz: tx/rx max mcs 0..11x2
 * 80p80 & 160: not supported
 */
static wlc_he_rateset_t he_rateset_rev51 = {
	/* .bw80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14)
};

/**
 * Main slice 11x2 80MHz
 * 80p80 & 160: not supported
 */
static wlc_he_rateset_t he_rateset_slice0 = {
	/* .bw80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_11 << 0)  | (HE_CAP_MAX_MCS_0_11 << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14)
};

/**
 * Main slice 9x2 20MHz (=>80Mhz definition)
 * 80p80 & 160: not supported
 */
static const wlc_he_rateset_t he_rateset_slice1 = {
	/* .bw80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_9  << 0)  | (HE_CAP_MAX_MCS_0_9  << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_0_9  << 0)  | (HE_CAP_MAX_MCS_0_9  << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14)
};

/**
 * 80, 80p80 & 160: not supported
 */
static const wlc_he_rateset_t he_rateset_none = {
	/* .bw80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw80p80_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_tx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14),
	/* .bw160_rx_mcs_nss = */
	(HE_CAP_MAX_MCS_NONE << 0)  | (HE_CAP_MAX_MCS_NONE << 2)  |
	(HE_CAP_MAX_MCS_NONE << 4)  | (HE_CAP_MAX_MCS_NONE << 6)  |
	(HE_CAP_MAX_MCS_NONE << 8)  | (HE_CAP_MAX_MCS_NONE << 10) |
	(HE_CAP_MAX_MCS_NONE << 12) | (HE_CAP_MAX_MCS_NONE << 14)
};

/* local functions */
static void phy_ac_hecap_get_phycap_info(phy_type_hecap_ctx_t *ctx, he_phy_cap_t *phycap);
static uint8 phy_ac_hecap_device_class(phy_info_t *pi);
static uint8 phy_ac_hecap_bfe_sts_below80(phy_info_t *pi);
static uint8 phy_ac_hecap_bfe_sts_above80(phy_info_t *pi);
static uint8 phy_ac_hecap_sound_dim_below80(phy_info_t *pi);
static uint8 phy_ac_hecap_sound_dim_above80(phy_info_t *pi);
static uint8 phy_ac_hecap_su_codebook_support(phy_info_t *pi);
static uint8 phy_ac_hecap_mu_codebook_support(phy_info_t *pi);
static uint8 phy_ac_hecap_max_nc_support(phy_info_t *pi);
static void phy_ac_hecap_write_bsscolor(phy_type_hecap_ctx_t *ctx, wlc_bsscolor_t *bsscolor);
static void phy_ac_hecap_write_def_pe_dur(phy_type_hecap_ctx_t *ctx, uint8 def_pe_dur);
static void phy_ac_hecap_get_rateset(phy_type_hecap_ctx_t *ctx, wlc_he_rateset_t *he_rateset);
static uint8 phy_ac_hecap_get_ppet(phy_type_hecap_ctx_t *ctx);

/* External Definitions */

/* register phy type specific implementation */
phy_ac_hecap_info_t*
BCMATTACHFN(phy_ac_hecap_register_impl)(phy_info_t *pi,
	phy_ac_info_t *pi_ac, phy_hecap_info_t *hecapi)
{
	phy_ac_hecap_info_t *ac_hecap_info;
	phy_type_hecap_fns_t fns;
	uint8 temp_reg = 0;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_hecap_info = phy_malloc(pi, sizeof(phy_ac_hecap_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	ac_hecap_info->pi = pi;
	ac_hecap_info->pi_ac = pi_ac;
	ac_hecap_info->hecap_info = hecapi;

	/* HE Capabilities */
	/* HE Generic Capabilities */
	ac_hecap_info->phy_he_caps |= READ_PHYREGFLD(pi, PhyInternalCapability5, BWSupport11ax) ?
		PHY_CAP_HE_BW : 0;
	ac_hecap_info->phy_he_caps |= READ_PHYREGFLD(pi, PhyInternalCapability5, PESupport11ax) ?
		PHY_CAP_HE_PE : 0;
	ac_hecap_info->phy_he_caps |= READ_PHYREGFLD(pi, PhyCapability2, Support11axDCM) ?
		PHY_CAP_HE_DCM : 0;
	ac_hecap_info->phy_he_caps |= READ_PHYREGFLD(pi, PhyInternalCapability5,
		BeamChangeSupport11ax) ? PHY_CAP_HE_BEAM_CHANGE : 0;
	temp_reg = READ_PHYREGFLD(pi, PhyInternalCapability5, CPLTFModesSupport11ax);
	ac_hecap_info->phy_he_caps |= (temp_reg & BIT0) ? PHY_CAP_HE_1XLTF_0P8CP : 0;
	ac_hecap_info->phy_he_caps |= (temp_reg & BIT1) ? PHY_CAP_HE_2XLTF_0P8CP : 0;
	ac_hecap_info->phy_he_caps |= (temp_reg & BIT2) ? PHY_CAP_HE_2XLTF_1P6CP : 0;
	ac_hecap_info->phy_he_caps |= (temp_reg & BIT3) ? PHY_CAP_HE_4XLTF_3P2CP : 0;

	/* Single User Capabilities */
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3,
		SupportSTBCRx11axSU) ? PHY_CAP_HE_SU_STBC_RX : 0;
	temp_reg = READ_PHYREGFLD(pi, PhyInternalCapability3, Support256qamBccSU);
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT0) ? PHY_CAP_HE_SU_256Q_BCC_NSS1_0P8CP : 0;
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT1) ? PHY_CAP_HE_SU_256Q_BCC_NSS2_0P8CP : 0;
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT2) ? PHY_CAP_HE_SU_256Q_BCC_NSS1_1P6CP : 0;
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT3) ? PHY_CAP_HE_SU_256Q_BCC_NSS2_1P6CP : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3,
		Support256qamLdpcSU) ? PHY_CAP_HE_SU_256Q_LDPC : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3,
		Support1024qamLdpcSU) ? PHY_CAP_HE_SU_1024Q_LDPC : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3, SupportSUBfmer) ?
		PHY_CAP_HE_SU_BFR : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3, SupportSUBfee) ?
		PHY_CAP_HE_SU_BFE : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3,
		SupportMUMIMOBfmerSU) ? PHY_CAP_HE_SU_MU_BFR : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3,
		SupportMUMIMOBfeeSU) ? PHY_CAP_HE_SU_MU_BFE : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability3, NumSoundDimSU) ?
		PHY_CAP_HE_SU_NUM_SOUND : 0;

	/* Resource Unit Capabilities */
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		SupportSTBCRx11axRU) ? PHY_CAP_HE_RU_STBC_RX : 0;
	temp_reg = READ_PHYREGFLD(pi, PhyInternalCapability4, Support256qamBccRU);
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT0) ? PHY_CAP_HE_RU_256Q_BCC_NSS1_0P8CP : 0;
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT1) ? PHY_CAP_HE_RU_256Q_BCC_NSS2_0P8CP : 0;
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT2) ? PHY_CAP_HE_RU_256Q_BCC_NSS1_1P6CP : 0;
	ac_hecap_info->phy_he_caps1 |= (temp_reg & BIT3) ? PHY_CAP_HE_RU_256Q_BCC_NSS2_1P6CP : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		Support256qamLdpcRU) ? PHY_CAP_HE_RU_256Q_LDPC : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		Support1024qamLdpcRU) ? PHY_CAP_HE_RU_1024Q_LDPC : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		SupportRUBfmer) ? PHY_CAP_HE_RU_BFR : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		SupportRUBfee) ? PHY_CAP_HE_RU_BFE : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		SupportMUMIMOBfmerRU) ? PHY_CAP_HE_RU_MU_BFR : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		SupportMUMIMOBfeeRU) ? PHY_CAP_HE_RU_MU_BFE : 0;
	ac_hecap_info->phy_he_caps1 |= READ_PHYREGFLD(pi, PhyInternalCapability4,
		NumSoundDimRU) ? PHY_CAP_HE_RU_NUM_SOUND : 0;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));

	fns.hecap_get = phy_ac_hecap_get_phycap_info;
	fns.bsscolor_write = phy_ac_hecap_write_bsscolor;
	fns.pe_dur_write = phy_ac_hecap_write_def_pe_dur;
	fns.hecap_get_rateset = phy_ac_hecap_get_rateset;
	fns.hecap_get_ppet = phy_ac_hecap_get_ppet;

	fns.ctx = pi;

	phy_hecap_register_impl(hecapi, &fns);

	return ac_hecap_info;
	/* error handling */
fail:
	if (ac_hecap_info != NULL)
		phy_mfree(pi, ac_hecap_info, sizeof(phy_ac_hecap_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_hecap_unregister_impl)(phy_ac_hecap_info_t *ac_hecap_info)
{
	phy_info_t *pi = ac_hecap_info->pi;
	phy_hecap_info_t *hecapi = ac_hecap_info->hecap_info;
	PHY_TRACE(("%s\n", __FUNCTION__));
	/* unregister from common */
	phy_hecap_unregister_impl(hecapi);
	phy_mfree(pi, ac_hecap_info, sizeof(phy_ac_hecap_info_t));
}

/* Returns Generic HE capabilities */
uint32
wlc_phy_ac_hecaps(phy_info_t *pi)
{
	return (pi->u.pi_acphy->hecapi->phy_he_caps);
}

/* Returns SU and MU HE capabilities */
uint32
wlc_phy_ac_hecaps1(phy_info_t *pi)
{
	if (pi->u.pi_acphy && pi->u.pi_acphy->hecapi) {
		return (pi->u.pi_acphy->hecapi->phy_he_caps1);
	} else {
		return 0;
	}
}

static void
phy_ac_hecap_get_phycap_info(phy_type_hecap_ctx_t *ctx, he_phy_cap_t *phycap)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	uint32 sflags;
	uint32 phyaccap = wlc_phy_cap_get((wlc_phy_t *)pi);
	uint8 bwcap = 0;

	sflags = si_core_sflags(pi->sh->sih, 0, 0);

	/* Fill default phy capabilities common for HE AP/STA */

	if (sflags & (SISF_2G_PHY | SISF_DB_PHY)) {
		/* b0: Support 40 MHz channel width in 2.4 GHz */
		bwcap |= HE_PHY_CH_WIDTH_2G_40;

		/* if b0=0, reserved. else
		 * b4: Support for 242/106/52/26-tone RU mapping in 40 MHz
		 */
	}

	if (sflags & (SISF_5G_PHY | SISF_DB_PHY)) {
		/* b1: support 40 MHz and 80 MHz channel width in 5 GHz.
		 * if b1=0, 20 MHz only support in 5 GHz
		 */
		if (phyaccap & PHY_CAP_80MHZ)
			bwcap |= HE_PHY_CH_WIDTH_5G_80;

		/* b2: 160 MHz support in 5 GHz */
		if (phyaccap & PHY_CAP_160MHZ) {
			bwcap |= HE_PHY_CH_WIDTH_5G_160;

			/* Remove flag from bwcap if the HW rev is not support BW160 */
			if (ACMAJORREV_47(pi->pubpi->phy_rev) && HW_MINREV_IS(pi, 0))
				bwcap &= ~HE_PHY_CH_WIDTH_5G_160;
		}

		/* b3: 80+80 MHz support in 5 GHz */
		if (phyaccap & PHY_CAP_8080MHZ)
			bwcap |= HE_PHY_CH_WIDTH_5G_80P80;

		/* b4: only relates to 80p80 support, not relevant at this point */
	}

	/* b1-b7: Channel width support */
	setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_CH_WIDTH_SET_IDX,
		HE_PHY_CH_WIDTH_SET_FSZ, bwcap);

	/* b8-b11: Preamble puncturing Rx. */

	/* b12: Device class */
	setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_DEVICE_CLASS_IDX,
		HE_PHY_DEVICE_CLASS_FSZ,  phy_ac_hecap_device_class(pi));

	/* b13: LDPC coding in Payload */
	if (phyaccap & PHY_CAP_LDPC) {
		setbit((uint8 *)phycap, HE_PHY_LDPC_PYLD_IDX);
	}

	/* b14: HE SU PPDU with 1x HE-LTF and 0.8 us GI, not supported */

	/* b15-b16: reserved */

	/* b17: NDP With 4x HE LTF and 3.2 us GI */
	setbit((uint8 *)phycap, HE_PHY_NDP_4x_LTF_3_2_GI_RX_IDX);

	/* b18-b19: STBC TX and RX for HE PPDUs.
	 * Not yet supported
	 */

	/* b20-b21: Doppler, not supported */

	/* b22-b23: UL MU-MIMO, not supported */

	if (wlc_phy_ac_hecaps(pi) & PHY_CAP_HE_DCM) {
		/* b24-b29: DCM Encoding at Tx and Rx. Not supported */
	}

	/* b30: UL HE MU PPDU Payload over 106-tone RU */
	setbit((uint8 *)phycap, HE_PHY_UL_MU_PYLD_IDX);

	if (wlc_phy_ac_hecaps1(pi) & PHY_CAP_HE_SU_BFR) {
		/* b31: SU Beamformer */
		setbit((uint8 *)phycap, HE_PHY_SU_BEAMFORMER_IDX);

		/* b33: MU Beamformer */
		setbit((uint8 *)phycap, HE_PHY_MU_BEAMFORMER_IDX);

		/* b40-b42: BFR capability of number of sounding dimensions for <= 80MHz */
		setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_SOUND_DIM_BELOW80MHZ_IDX,
			HE_PHY_SOUND_DIM_BELOW80MHZ_FSZ, phy_ac_hecap_sound_dim_below80(pi));

		/* b43-b45: BFR capability of number of sounding dimensions for > 80MHz */
		setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_SOUND_DIM_ABOVE80MHZ_IDX,
			HE_PHY_SOUND_DIM_ABOVE80MHZ_FSZ, phy_ac_hecap_sound_dim_above80(pi));
	}

	if (wlc_phy_ac_hecaps1(pi) & PHY_CAP_HE_SU_BFE) {
		/* b32: SU Beamformee */
		setbit((uint8 *)phycap, HE_PHY_SU_BEAMFORMEE_IDX);

		/* b34-b36: Beamformee STS for <= 80MHz */
		setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_BEAMFORMEE_STS_BELOW80MHZ_IDX,
			HE_PHY_BEAMFORMEE_STS_BELOW80MHZ_FSZ, phy_ac_hecap_bfe_sts_below80(pi));

		/* b37-b39 Beamformee STS for > 80MHz */
		setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_BEAMFORMEE_STS_ABOVE80MHZ_IDX,
			HE_PHY_BEAMFORMEE_STS_ABOVE80MHZ_FSZ, phy_ac_hecap_bfe_sts_above80(pi));

		/* b46: Ng = 16 For SU Feedback */
		setbit((uint8 *)phycap, HE_PHY_SU_FEEDBACK_NG16_SUPPORT_IDX);

		/* b47: Ng = 16 For MU Feedback not Support */

		/* b48: Codebook Size {4, 2} For SU Support */
		setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_SU_CODEBOOK_SUPPORT_IDX,
			HE_PHY_SU_CODEBOOK_SUPPORT_FSZ, phy_ac_hecap_su_codebook_support(pi));

		/* b49: Codebook Size {7, 5} For MU Support */
		setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_MU_CODEBOOK_SUPPORT_IDX,
			HE_PHY_MU_CODEBOOK_SUPPORT_FSZ, phy_ac_hecap_mu_codebook_support(pi));

		/* b59-b61: Max Nc */
		setbits((uint8 *)phycap, sizeof(*phycap), HE_PHY_MAX_NC_IDX,
			HE_PHY_MAX_NC_FSZ, phy_ac_hecap_max_nc_support(pi));

	}

	/* b50-52: trigger related settings */

	/* b53: Partial Bandwidth Extended Range. not supported */

	/* b54: DL MU-MIMO on Partial BW */

	/* b55: PPE Threshold Present */

	/* b56: SRP based SR support */

	/* b57: Power Boost factor Support */

	/* b58: 4x HE LTF and 0.8 GI Support */

	/* b62-b63: STBC > 80MHz, not supported */

	/* b64: HE ER SU PPDU 4x HE-LTF and 0.8 us GI */
}

static uint8
phy_ac_hecap_device_class(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	return (ACMAJORREV_44_46(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev));
}

static uint8
phy_ac_hecap_bfe_sts_below80(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev))
		return HE_PHY_BFE_STS_BELOW80;

	return 0;
}

static uint8
phy_ac_hecap_bfe_sts_above80(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	if (ACMAJORREV_47(pi->pubpi->phy_rev))
		return HE_PHY_BFE_STS_ABOVE80;

	return 0;
}

static uint8
phy_ac_hecap_sound_dim_below80(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev))
		return HE_PHY_SOUND_DIM_BELOW80;

	return 0;
}

static uint8
phy_ac_hecap_sound_dim_above80(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	if (ACMAJORREV_47(pi->pubpi->phy_rev))
		return HE_PHY_SOUND_DIM_ABOVE80;

	return 0;
}

static uint8
phy_ac_hecap_su_codebook_support(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	return (ACMAJORREV_44_46(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev));
}

static uint8
phy_ac_hecap_mu_codebook_support(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	return (ACMAJORREV_44_46(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev));
}

static uint8
phy_ac_hecap_max_nc_support(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
	if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
		return HE_PHY_MAC_NC;
	return 0;
}

static void
phy_ac_hecap_write_bsscolor(phy_type_hecap_ctx_t *ctx, wlc_bsscolor_t *bsscolor)
{
	phy_info_t *pi = (phy_info_t *)ctx;

	/* Only two indexes supported ! */
	ASSERT(bsscolor->index <= HE_BSSCOLOR_IDX1);

	if (bsscolor->index == HE_BSSCOLOR_IDX0) {
		/* BSS0 STA-IDs */
		MOD_PHYREG(pi, HESigBStaId00, HESigBStaId0, bsscolor->staid[STAID0]);
		MOD_PHYREG(pi, HESigBStaId01, HESigBStaId1, bsscolor->staid[STAID1]);
		MOD_PHYREG(pi, HESigBStaId02, HESigBStaId2, bsscolor->staid[STAID2]);
		MOD_PHYREG(pi, HESigBStaId03, HESigBStaId3, bsscolor->staid[STAID3]);
		MOD_PHYREG(pi, HESigBMyBSS0, he_my_bss_sta_id_len0, HE_BSSCOLOR_MAX_STAID);
		if (bsscolor->disable == FALSE) {
			MOD_PHYREG(pi, HESigBMyBSS0, HESigBMyBSS0, bsscolor->color);
			MOD_PHYREG(pi, HESigParseCtrl, he_mybss_color_len, 1);
		} else {
			/* Disable bsscolor filtering */
			MOD_PHYREG(pi, HESigParseCtrl, he_mybss_color_len, 0);
		}
	} else {
		/* BSS1 STA-IDs */
		MOD_PHYREG(pi, HESigBStaId10, HESigBStaId0, bsscolor->staid[STAID0]);
		MOD_PHYREG(pi, HESigBStaId11, HESigBStaId1, bsscolor->staid[STAID1]);
		MOD_PHYREG(pi, HESigBStaId12, HESigBStaId2, bsscolor->staid[STAID2]);
		MOD_PHYREG(pi, HESigBStaId13, HESigBStaId3, bsscolor->staid[STAID3]);
		MOD_PHYREG(pi, HESigBMyBSS1, he_my_bss_sta_id_len1, HE_BSSCOLOR_MAX_STAID);
		if (bsscolor->disable == FALSE) {
			MOD_PHYREG(pi, HESigBMyBSS1, HESigBMyBSS1, bsscolor->color);
			/* Use both bsscolor sets */
			MOD_PHYREG(pi, HESigParseCtrl, he_mybss_color_len, 2);
		} else {
			/* Disable bsscolor filtering */
			MOD_PHYREG(pi, HESigParseCtrl, he_mybss_color_len, 0);
		}
	}

}

static void
phy_ac_hecap_write_def_pe_dur(phy_type_hecap_ctx_t *ctx, uint8 def_pe_dur)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	MOD_PHYREG(pi, HESigSupportCtrl, he_rx_pkt_ext_category, def_pe_dur);
}

static void
phy_ac_hecap_get_rateset(phy_type_hecap_ctx_t *ctx, wlc_he_rateset_t *he_rateset)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	*he_rateset = he_rateset_none;

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		if (ACMINORREV_0(pi)) {
			/* 43684a0, 43684a1 */
			*he_rateset = he_rateset_rev47;
		} else {
			/* 43684b0 and newer */
			*he_rateset = he_rateset_rev47_1;
		}
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		*he_rateset = he_rateset_rev51;
	} else if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		if (pi->pubpi->slice == DUALMAC_MAIN) {
			*he_rateset = he_rateset_slice0;
		} else {
			*he_rateset = he_rateset_slice1;
		}
	}
}

static uint8
phy_ac_hecap_get_ppet(phy_type_hecap_ctx_t *ctx)
{
	/* At this point all revs 128 and above require ppet16 for all nss/ru/mcs combinations */
	return WL_HE_PPET_16US;
}

#endif /* WL11AX */
