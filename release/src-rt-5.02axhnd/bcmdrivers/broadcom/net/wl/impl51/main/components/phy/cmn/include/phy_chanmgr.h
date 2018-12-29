/*
 * Channel manager internal interface (to other PHY modules).
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
 * $Id: phy_chanmgr.h 767248 2018-08-31 20:55:03Z $
 */

#ifndef _phy_chanmgr_h_
#define _phy_chanmgr_h_

#include <typedefs.h>
#include <phy_api.h>

#if defined(WLSRVSDB)
#define SR_MEMORY_BANK	2
#endif /* end WLSRVSDB */

/* forward declaration */
typedef struct phy_chanmgr_info phy_chanmgr_info_t;

/* attach/detach */
phy_chanmgr_info_t *phy_chanmgr_attach(phy_info_t *pi);
void phy_chanmgr_detach(phy_chanmgr_info_t *ri);

bool phy_chanmgr_skip_band_init(phy_chanmgr_info_t *chanmgri);
int wlc_phy_chanspec_bandrange_get(phy_info_t*, chanspec_t);

#if defined(WLTEST)
int phy_chanmgr_get_smth(phy_info_t *pi, int32 *ret_int_ptr);
int phy_chanmgr_set_smth(phy_info_t *pi, int8 int_val);
#endif /* defined(WLTEST) */

int phy_chanmgr_set_shm(phy_info_t *pi, chanspec_t chanspec);

/* VSDB, RVSDB Module realted declaration */
void phy_sr_vsdb_reset(wlc_phy_t *pi);
void phy_reset_srvsdb_engine(wlc_phy_t *pi);

uint16 phy_chanmgr_get_home_chanspec(phy_chanmgr_info_t *chanmgri);

void phy_chanmgr_dccal_force(phy_info_t *pi);
#endif /* _phy_chanmgr_h_ */
