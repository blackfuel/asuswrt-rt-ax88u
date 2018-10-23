/*
 * FCBS module interface (to other PHY modules).
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
 * $Id: phy_fcbs.h 612041 2016-01-12 22:04:08Z $
 */

#ifndef _phy_fcbs_h_
#define _phy_fcbs_h_

#include <phy_api.h>

/* Definition of FCBS macros that are used by AC specific layer and common layer */
#ifdef ENABLE_FCBS
#define IS_FCBS(pi) (((pi)->HW_FCBS) && ((pi)->FCBS))
#else
#define IS_FCBS(pi) 0
#endif /* ENABLE_FCBS */

/* Fast channel band switch (FCBS) structures and definitions */
#ifdef ENABLE_FCBS

/* FCBS cache id */
#define FCBS_CACHE_RADIOREG		1
#define FCBS_CACHE_PHYREG		2
#define FCBS_CACHE_PHYTBL16		3
#define FCBS_CACHE_PHYTBL32		4

/* Channel index of pseudo-simultaneous dual channels */
#define FCBS_CHAN_A	0
#define FCBS_CHAN_B 1

/* Flag to tell ucode to turn on/off BPHY core */
#define FCBS_BPHY_UPDATE		0x1
#define FCBS_BPHY_ON			(FCBS_BPHY_UPDATE | 0x2)
#define FCBS_BPHY_OFF			(FCBS_BPHY_UPDATE | 0x0)

/* FCBS TBL format */
/* bit shift for core info of radioregs */
#define FCBS_TBL_RADIOREG_CORE_SHIFT 0x9
/* ORed with the reg offset indicates an instruction */
#define FCBS_TBL_INST_INDICATOR 0x2000

#endif /* ENABLE_FCBS */

/* Number of pseudo-simultaneous channels that we support */
#define MAX_FCBS_CHANS	2

/* length of the HW FCBS TBL */
#define FCBS_HW_TBL_LEN 1024

/* forward declaration */
typedef struct phy_fcbs_info phy_fcbs_info_t;

/* attach/detach */
phy_fcbs_info_t *phy_fcbs_attach(phy_info_t *pi);
void phy_fcbs_detach(phy_fcbs_info_t *cmn_info);

/* up/down */
int phy_fcbs_init(phy_fcbs_info_t *cmn_info);

/* Intra-module API */
/* Fast Channel/Band Switch (FCBS) engine functions */
int phy_fcbs_preinit(phy_info_t *pi, int chanidx);
bool wlc_phy_is_fcbs_pending(phy_info_t *pi, chanspec_t chanspec, int *chanidx_ptr);
bool wlc_phy_is_fcbs_chan(phy_info_t *pi, chanspec_t chanspec, int *chanidx_ptr);
uint16 wlc_phy_channelindicator_obtain(phy_info_t *pi);
#endif /* _phy_fcbs_h_ */
