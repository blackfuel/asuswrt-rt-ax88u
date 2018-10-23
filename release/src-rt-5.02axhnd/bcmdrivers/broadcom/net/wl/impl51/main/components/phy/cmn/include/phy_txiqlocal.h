/*
 * TXIQLO CAL module interface (to other PHY modules).
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
 * $Id: phy_txiqlocal.h 690650 2017-03-17 06:16:02Z $
 */

#ifndef _phy_txiqlocal_h_
#define _phy_txiqlocal_h_

#include <phy_api.h>

#define CAL_COEFF_READ    0
#define CAL_COEFF_WRITE   1
#define TXCAL_CACHE_VALID		0xACDC
#define RXCAL_CACHE_VALID		TXCAL_CACHE_VALID /* [PHY_REARCH] New value may be added */

/* defs for iqlo cal */
enum {  /* mode selection for reading/writing tx iqlo cal coefficients */
	TB_START_COEFFS_AB, TB_START_COEFFS_D, TB_START_COEFFS_E, TB_START_COEFFS_F,
	TB_BEST_COEFFS_AB,  TB_BEST_COEFFS_D,  TB_BEST_COEFFS_E,  TB_BEST_COEFFS_F,
	TB_OFDM_COEFFS_AB,  TB_OFDM_COEFFS_D,  TB_BPHY_COEFFS_AB,  TB_BPHY_COEFFS_D,
	PI_INTER_COEFFS_AB, PI_INTER_COEFFS_D, PI_INTER_COEFFS_E, PI_INTER_COEFFS_F,
	PI_FINAL_COEFFS_AB, PI_FINAL_COEFFS_D, PI_FINAL_COEFFS_E, PI_FINAL_COEFFS_F
};

/* forward declaration */
typedef struct phy_txiqlocal_info phy_txiqlocal_info_t;

/* attach/detach */
phy_txiqlocal_info_t *phy_txiqlocal_attach(phy_info_t *pi);
void phy_txiqlocal_detach(phy_txiqlocal_info_t *cmn_info);

/* init */

/* phy txcal coeffs structure */
typedef struct txcal_coeffs {
	uint16 txa;
	uint16 txb;
	uint16 txd;	/* contain di & dq */
	uint8 txei;
	uint8 txeq;
	uint8 txfi;
	uint8 txfq;
	uint16 rxa;
	uint16 rxb;
} txcal_coeffs_t;

/* Intra-module API for IOVAR */
void phy_txiqlocal_txiqccget(phy_info_t *pi, void *a);
void phy_txiqlocal_txiqccset(phy_info_t *pi, void *p);
void phy_txiqlocal_txloccget(phy_info_t *pi, void *a);
void phy_txiqlocal_txloccset(phy_info_t *pi, void *p);

void phy_txiqlocal_scanroam_cache(phy_info_t *pi, bool set);
#endif /* _phy_txiqlocal_h_ */
