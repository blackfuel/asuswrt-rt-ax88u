/*
 * IOCV module interface - iovar command facility.
 * iovar command is in xTLV format.
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
 * $Id: wlc_iocv_cmd.c 631222 2016-04-13 18:51:05Z $
 */

#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlc_types.h>
#include <wlc_iocv_cmd.h>

/* Process iovar command.
 * The command in xTLV format is in in_buf/in_len.
 * The out_buf and in_buf may point to the same memory.
 */
int
wlc_iocv_iov_cmd_proc(void *ctx, const wlc_iov_cmd_t *tbl, uint sz, bool set,
	void *in_buf, uint in_len, void *out_buf, uint out_len, struct wlc_if *wlcif)
{
	bcm_xtlv_t *cmd = (bcm_xtlv_t *)in_buf;
	uint16 type = cmd->id;
	uint16 len = cmd->len;
	uint i;
	uint hdrlen = BCM_XTLV_HDR_SIZE;
	int err = BCME_NOTFOUND;

	/* Handle a single command for now */

	for (i = 0; i < sz; i ++) {
		if (tbl[i].cmd != type) {
			continue;
		}

		if (in_len - hdrlen < len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* Do some validation... */

		if (tbl[i].hdlr == NULL) {
			err = BCME_ERROR;
			break;
		}

		len = (uint16)out_len;

		return (tbl[i].hdlr)
		        (ctx, (uint8 *)cmd + hdrlen, cmd->len,
		         out_buf, &len, set, wlcif);
	}

	return err;
}
