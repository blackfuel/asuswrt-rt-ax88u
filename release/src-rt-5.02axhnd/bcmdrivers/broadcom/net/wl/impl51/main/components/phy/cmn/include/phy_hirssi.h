/*
 * HiRSSI eLNA Bypass module internal interface (to other PHY modules).
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
 * $Id: phy_hirssi.h 617885 2016-02-09 00:18:54Z $
 */

#ifndef _phy_hirssi_h_
#define _phy_hirssi_h_

#include <typedefs.h>
#include <phy_api.h>

/* HiRSSI options */
typedef enum phy_hirssi {
	PHY_HIRSSI_BYP,
	PHY_HIRSSI_RES
} phy_hirssi_t;

/* forward declaration */
typedef struct phy_hirssi_info phy_hirssi_info_t;

/* attach/detach */
phy_hirssi_info_t *phy_hirssi_attach(phy_info_t *pi);
void phy_hirssi_detach(phy_hirssi_info_t *info);

/* functions */
int phy_hirssi_get_period(phy_info_t *pi, int32 *period);
int phy_hirssi_set_period(phy_info_t *pi, int32 period);
int phy_hirssi_get_en(phy_info_t *pi, int32 *enable);
int phy_hirssi_set_en(phy_info_t *pi, int32 enable);
int phy_hirssi_get_rssi(phy_info_t *pi, int32 *rssi, phy_hirssi_t opt);
int phy_hirssi_set_rssi(phy_info_t *pi, int32 rssi, phy_hirssi_t opt);
int phy_hirssi_get_cnt(phy_info_t *pi, int32 *cnt, phy_hirssi_t opt);
int phy_hirssi_set_cnt(phy_info_t *pi, int32 cnt, phy_hirssi_t opt);
int phy_hirssi_get_status(phy_info_t *pi, int32 *status);

#endif /* _phy_hirssi_h_ */
