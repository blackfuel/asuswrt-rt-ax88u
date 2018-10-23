/*
 * TuTrace
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
 * $Id: tutrace.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _TUTRACE_H
#define _TUTRACE_H

#ifdef __cplusplus
extern "C" {
#endif // endif

typedef void (*WPS_TRACEMSG_OUTPUT_FN)(int is_err, char *traceMsg);
void wps_set_traceMsg_output_fn(WPS_TRACEMSG_OUTPUT_FN fn);
void wps_tutrace_set_msglevel(unsigned int level);
unsigned int wps_tutrace_get_msglevel();
int WPS_HexDumpAscii(unsigned int level, char *title, unsigned char *buf, unsigned int len);

/* Default trace level */
#define TUTRACELEVEL    (TUERR | TUINFO)

/* trace levels */
#define TUERR		0x0001
#define TUINFO		0x0002
#define TUNFC		0x0004
#define TUDUMP_MSG	0x0008
#define TUDUMP_IE	0x0010
#define TUDUMP_PROBE	0x0020
#define TUDUMP_KEY	0x0040
#define TUDUMP_NFC	0x0080
#define TUTIME		0x8000

#define TUTRACE_ERR        TUERR, __FUNCTION__, __LINE__
#define TUTRACE_INFO       TUINFO, __FUNCTION__, __LINE__
#define TUTRACE_NFC        TUNFC, __FUNCTION__, __LINE__

#ifdef _TUDEBUGTRACE

#define TUTRACE(VARGLST)   print_traceMsg VARGLST

void print_traceMsg(int level, const char *lpszFile,
	int nLine, char *lpszFormat, ...);

#else

#define TUTRACE(VARGLST)    ((void)0)

#endif /* _TUDEBUGTRACE */

#ifdef __cplusplus
}
#endif // endif

#endif /* _TUTRACE_H */
