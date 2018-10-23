/*
 * VASIP PHY module public interface (to MAC driver).
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
 * $Id: phy_vasip_api.h 659421 2016-09-14 06:45:22Z $
 */

#ifndef _phy_vasip_api_h_
#define _phy_vasip_api_h_

#include <phy_api.h>

#ifndef VASIP_NOVERSION
#define VASIP_NOVERSION 0xFF
#endif // endif

/*
 * Return vasip version, -1 if not present.
 */
uint8 phy_vasip_get_ver(phy_info_t *pi);
/*
 * reset/activate vasip.
 */
void phy_vasip_reset_proc(phy_info_t *pi, int reset);

void phy_vasip_set_clk(phy_info_t *pi, bool val);
void phy_vasip_write_bin(phy_info_t *pi, const uint32 vasip_code[], const uint nbytes);

#ifdef VASIP_SPECTRUM_ANALYSIS
void phy_vasip_write_spectrum_tbl(phy_info_t *pi,
        const uint32 vasip_spectrum_tbl[], const uint nbytes_tbl);
#endif // endif

void phy_vasip_write_svmp(phy_info_t *pi, uint32 offset, uint16 val);
void phy_vasip_read_svmp(phy_info_t *pi, uint32 offset, uint16 *val);
void phy_vasip_write_svmp_blk(phy_info_t *pi, uint32 offset, uint16 len, uint16 *val);
void phy_vasip_read_svmp_blk(phy_info_t *pi, uint32 offset, uint16 len, uint16 *val);
#endif /* _phy_vasip_api_h_ */
