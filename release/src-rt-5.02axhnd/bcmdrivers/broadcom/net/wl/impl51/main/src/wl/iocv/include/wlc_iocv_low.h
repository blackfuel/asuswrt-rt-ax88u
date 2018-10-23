/*
 * IOCV module interface.
 * For BMAC/PHY.
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
 * $Id: wlc_iocv_low.h 614277 2016-01-21 20:28:50Z $
 */

#ifndef _wlc_iocv_low_h_
#define _wlc_iocv_low_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <osl_decl.h>

#include <wlc_iocv_types.h>

/* attach/detach */
wlc_iocv_info_t *wlc_iocv_low_attach(osl_t *osh, uint16 iovt_cnt, uint16 ioct_cnt);
void wlc_iocv_low_detach(wlc_iocv_info_t *ii);

/* dump function - need to be registered to dump facility by a caller */
int wlc_iocv_low_dump(wlc_iocv_info_t *ii, struct bcmstrbuf *b);

/*
 * Dispatch iovar with table id 'tid' and action id 'aid'
 * to a registered dispatch callback.
 *
 * Return BCME_XXXX.
 */
int wlc_iocv_low_dispatch_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint v_sz, struct wlc_if *wlcif);

/*
 * Dispatch ioctl with table id 'tid' and command id 'cid'
 * to a registered dispatch callback.
 *
 * Return BCME_XXXX.
 */
int wlc_iocv_low_dispatch_ioc(wlc_iocv_info_t *ii, uint16 tid, uint32 cid,
	uint8 *a, uint a_len, struct wlc_if *wlcif);

#endif /* _wlc_iocv_low_h_ */
