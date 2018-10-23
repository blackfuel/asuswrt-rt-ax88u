/*
 *  High Efficiency (802.11ax) (HE) phy module public interface (to MAC driver).
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
 * $Id: phy_hecap_api.h 758800 2018-04-20 14:09:26Z $
 */

#ifndef _phy_hecap_api_h_
#define _phy_hecap_api_h_

#include <typedefs.h>
#include <phy_api.h>
#include <802.11ax.h>

/* HE rate capabilities, per bw/nss, encoded in 8*2 bits (nss*mcs), see HE_CAP_MAX_MCS... */
typedef struct wlc_he_rateset {
	uint16 bw80_tx_mcs_nss;
	uint16 bw80_rx_mcs_nss;
	uint16 bw80p80_tx_mcs_nss;
	uint16 bw80p80_rx_mcs_nss;
	uint16 bw160_tx_mcs_nss;
	uint16 bw160_rx_mcs_nss;
} wlc_he_rateset_t;

#define HE_BSSCOLOR_IDX0		0
#define HE_BSSCOLOR_IDX1		1
#define STAID0				0
#define STAID1				1
#define STAID2				2
#define STAID3				3
#define HE_BSSCOLOR_MAX_STAID		4

typedef struct wlc_bsscolor {
	uint8	index;				/**< bsscolor index 0-1 */
	uint8	color;				/**< bsscolor value from 0 to 63 */
	bool	disable;			/**< To disable BSS coloring */
	uint16	staid[HE_BSSCOLOR_MAX_STAID];	/**< 0-3 staid info of each bsscolor */
} wlc_bsscolor_t;

/* External declarations */
extern void phy_hecap_fill_phycap_info(phy_info_t *pi, he_phy_cap_t *phycap);
extern void phy_hecap_write_bsscolor(phy_info_t *pi, wlc_bsscolor_t *bsscolor);
extern void phy_hecap_write_pe_dur(phy_info_t *pi, uint8 pe_dur);
extern void phy_hecap_get_rateset(phy_info_t *pi, wlc_he_rateset_t *he_rateset);
extern uint8 phy_hecap_get_ppet(phy_info_t *pi);

#endif /* _phy_hecap_api_h_ */
