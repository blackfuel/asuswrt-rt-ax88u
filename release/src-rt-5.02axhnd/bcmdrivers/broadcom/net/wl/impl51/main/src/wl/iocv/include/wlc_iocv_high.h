/*
 * IOCV module interface.
 * For WLC.
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
 * $Id: wlc_iocv_high.h 677784 2017-01-05 03:38:30Z $
 */

#ifndef _wlc_iocv_high_h_
#define _wlc_iocv_high_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <osl_decl.h>

#include <wlc_iocv_types.h>

/* attach/detach */
wlc_iocv_info_t *wlc_iocv_high_attach(osl_t *osh, uint16 iovt_cnt, uint16 ioct_cnt,
	void *high_ctx, void *low_ctx);
void wlc_iocv_high_detach(wlc_iocv_info_t *ii);

/* dump function - need to be registered to dump facility by a caller */
int wlc_iocv_high_dump(wlc_iocv_info_t *ii, struct bcmstrbuf *b);

/*
 * TODO: Remove unnecessary interface
 * once integrated with existing iovar table handling.
 */

/* lookup iovar and return iovar entry and table id if found */
const bcm_iovar_t *wlc_iocv_high_find_iov(wlc_iocv_info_t *ii, const char *name, uint16 *tid);

/* forward iovar to registered table dispatcher */
int wlc_iocv_high_fwd_iov(wlc_iocv_info_t *ii, uint16 tid, uint32 aid, const bcm_iovar_t *vi,
	uint8 *p, uint p_len, uint8 *a, uint a_len, uint var_sz, struct wlc_if *wlcif);

/* lookup ioctl and return ioctl entry and table id if found */
const wlc_ioctl_cmd_t *wlc_iocv_high_find_ioc(wlc_iocv_info_t *ii, uint32 cid, uint16 *tid);

/* forward ioctl to registered table dispatcher */
int wlc_iocv_high_fwd_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	uint8 *a, uint a_len, struct wlc_if *wlcif);

/* dispatch ioctl that doesn't have a table registered */
int wlc_iocv_high_proc_ioc(wlc_iocv_info_t *ii, uint32 cid,
	uint8 *a, uint a_len, struct wlc_if *wlcif);

/* query all iovar names */
int wlc_iocv_high_iov_names(wlc_iocv_info_t *ii, uint name_bytes, uint var_start, uint out_max,
	uint8 *buf, uint len);

/* validate ioctl */
int wlc_iocv_high_vld_ioc(wlc_iocv_info_t *ii, uint16 tid, const wlc_ioctl_cmd_t *ci,
	void *a, uint a_len);
void *wlc_iovc_obj_registry(void *high_ctx, uint sz);
void *wlc_iovc_obj_registry_unref(void *high_ctx, void *cmn, uint sz);

#endif /* _wlc_iocv_high_h_ */
