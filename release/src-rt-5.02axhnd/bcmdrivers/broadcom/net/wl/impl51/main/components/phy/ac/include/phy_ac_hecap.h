/*
 * ACPHY HE module interface
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

#ifndef _phy_ac_hecap_h_
#define _phy_ac_hecap_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_type_hecap.h>

/* HE (11ax) capabilities */
#ifdef WL11AX
/* HE Generic capabilies */
#define PHY_CAP_HE_BW				0x00000003 /* Supported BW */
#define PHY_CAP_HE_PE				0x0000000c /* Supported Packet Extension Modes */
#define PHY_CAP_HE_DCM				0x00000010 /* Support Dual Carrier Modulation */
#define PHY_CAP_HE_BEAM_CHANGE			0x00000020 /* Supprot Beam change */
#define PHY_CAP_HE_1XLTF_0P8CP			0x00000100 /* Support 1x LTF and 0.8us CP */
#define PHY_CAP_HE_2XLTF_0P8CP			0x00000200 /* Support 1x LTF and 0.8us CP */
#define PHY_CAP_HE_2XLTF_1P6CP			0x00000400 /* Support 1x LTF and 0.8us CP */
#define PHY_CAP_HE_4XLTF_3P2CP			0x00001000 /* Support 1x LTF and 0.8us CP */

/* HE Single user capabilies */
#define PHY_CAP_HE_SU_STBC_RX			0x00000001 /* Single User STBC RX */
#define PHY_CAP_HE_SU_256Q_BCC_NSS1_0P8CP	0x00000002 /* SU 256QAM with NSS 1 & 0.8us cp */
#define PHY_CAP_HE_SU_256Q_BCC_NSS2_0P8CP	0x00000004 /* SU 256QAM with NSS 2 & 0.8us cp */
#define PHY_CAP_HE_SU_256Q_BCC_NSS1_1P6CP	0x00000008 /* SU 256QAM with NSS 1 & 1.6us cp */
#define PHY_CAP_HE_SU_256Q_BCC_NSS2_1P6CP	0x00000010 /* SU 256QAM with NSS 2 & 1.6us cp */
#define PHY_CAP_HE_SU_256Q_LDPC			0x00000020 /* SU 256QAM LDPC */
#define PHY_CAP_HE_SU_1024Q_LDPC		0x00000040 /* SU 1024QAM LDPC */
#define PHY_CAP_HE_SU_BFR			0x00000080 /* SU Beamformer cap */
#define PHY_CAP_HE_SU_BFE			0x00000100 /* SU Beamformee cap */
#define PHY_CAP_HE_SU_MU_BFR			0x00000200 /* SU MU Beamformer cap */
#define PHY_CAP_HE_SU_MU_BFE			0x00000400 /* SU MU Beamformee cap */
#define PHY_CAP_HE_SU_NUM_SOUND			0x00003800 /* SU Number of Sounding Dimensions */

/* HE Resource Unit capabilies */
#define PHY_CAP_HE_RU_STBC_RX			0x00010000 /* Resource Unit STBC RX */
#define PHY_CAP_HE_RU_256Q_BCC_NSS1_0P8CP	0x00020000 /* RU 256QAM with NSS 1 & 0.8us cp */
#define PHY_CAP_HE_RU_256Q_BCC_NSS2_0P8CP	0x00040000 /* RU 256QAM with NSS 2 & 0.8us cp */
#define PHY_CAP_HE_RU_256Q_BCC_NSS1_1P6CP	0x00080000 /* RU 256QAM with NSS 1 & 1.6us cp */
#define PHY_CAP_HE_RU_256Q_BCC_NSS2_1P6CP	0x00100000 /* RU 256QAM with NSS 2 & 1.6us cp */
#define PHY_CAP_HE_RU_256Q_LDPC			0x00200000 /* RU 256QAM LDPC */
#define PHY_CAP_HE_RU_1024Q_LDPC		0x00400000 /* RU 1024QAM LDPC */
#define PHY_CAP_HE_RU_BFR			0x00800000 /* RU Beamformer cap */
#define PHY_CAP_HE_RU_BFE			0x01000000 /* RU Beamformee cap */
#define PHY_CAP_HE_RU_MU_BFR			0x02000000 /* RU MU Beamformer cap */
#define PHY_CAP_HE_RU_MU_BFE			0x04000000 /* RU MU Beamformee cap */
#define PHY_CAP_HE_RU_NUM_SOUND			0x38000000 /* RU Number of Sounding Dimensions */
#endif /* WL11AX */

/* forward declaration */

typedef struct phy_ac_hecap_info phy_ac_hecap_info_t;

#ifdef WL11AX
/* register/unregister ACPHY specific implementations to/from common */
phy_ac_hecap_info_t *phy_ac_hecap_register_impl(phy_info_t *pi,
	phy_ac_info_t *ac_info, phy_hecap_info_t *ri);
void phy_ac_hecap_unregister_impl(phy_ac_hecap_info_t *info);

/* External Declarations */
extern uint32 wlc_phy_ac_hecaps(phy_info_t *pi);
extern uint32 wlc_phy_ac_hecaps1(phy_info_t *pi);
#endif /* WL11AX */

#endif /* _phy_ac_hecap_h_ */
