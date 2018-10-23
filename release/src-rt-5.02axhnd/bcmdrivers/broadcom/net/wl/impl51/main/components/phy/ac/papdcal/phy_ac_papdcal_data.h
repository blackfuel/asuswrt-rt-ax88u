/*
 * ACPHY PAPD CAL module data interface
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
 * $Id: phy_ac_papdcal_data.h 712753 2017-07-26 09:28:47Z $
 */

#ifndef _phy_ac_papdcal_data_h_
#define _phy_ac_papdcal_data_h_

#include <typedefs.h>

#define SZ_SCAL_TABLE_0		64
#define SZ_SCAL_TABLE_1		128
#define SZ_PGA_GAIN_ARRAY	256
#define SZ_WBPAPD_WAVEFORM	4000

extern const int8 pga_gain_array_2g[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_0[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_1[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_2[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_3[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_2g_4354[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_4354[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_435x_radiorev40[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_435x_radiorev44[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_2g_epapd[2][SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_epapd_0[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_epapd_1[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_epapd_2[SZ_PGA_GAIN_ARRAY];
extern const int8 pga_gain_array_5g_epapd_3[SZ_PGA_GAIN_ARRAY];
extern const int8 lut_log20_bbmult[100];

extern const uint32 acphy_papd_scaltbl_128[SZ_SCAL_TABLE_1];
extern const uint32 acphy_papd_scaltbl[SZ_SCAL_TABLE_0];
extern const uint32 acphy_wbpapd_waveform[SZ_WBPAPD_WAVEFORM];
extern uint32 fcbs_wbpapd_waveform[SZ_WBPAPD_WAVEFORM];

#endif /* _phy_ac_papdcal_data_h_ */
