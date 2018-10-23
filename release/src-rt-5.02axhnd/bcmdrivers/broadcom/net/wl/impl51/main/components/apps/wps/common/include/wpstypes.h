/*
 * WPS TYPE
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
 * $Id: wpstypes.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _WPS_TYPE_
#define _WPS_TYPE_

#if !defined(WIN32)
#include <stdint.h> /* for uintptr_t */
#endif /* !WIN32 */

#define UINT2PTR(val)   ((void *)(uintptr_t)(uint32)(val))
#define PTR2UINT(val)   ((uint32)(uintptr_t)(val))

#ifndef	BOOL
#define	BOOL int
#endif // endif

#ifdef __cplusplus
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
#endif // endif

#ifdef _MSC_VER
#pragma warning(disable:4786)   /* identifier truncated to 255 characters in debug info */
typedef unsigned __int64 uint64;
#endif // endif

#ifndef IN
#define IN
#endif // endif

#ifndef OUT
#define OUT
#endif // endif

#define SIZE_1_BYTE         1
#define SIZE_2_BYTES        2
#define SIZE_3_BYTES        3
#define SIZE_4_BYTES        4
#define SIZE_6_BYTES        6
#define SIZE_8_BYTES        8
#define SIZE_16_BYTES       16
#define SIZE_20_BYTES       20
#define SIZE_32_BYTES       32
#define SIZE_64_BYTES       64
#define SIZE_80_BYTES       80
#define SIZE_128_BYTES      128
#define SIZE_192_BYTES      192

#define SIZE_SSID_LENGTH       33

#define SIZE_64_BITS        8
#define SIZE_128_BITS       16
#define SIZE_160_BITS       20
#define SIZE_256_BITS       32

#define SIZE_ENCR_IV            SIZE_128_BITS
#define ENCR_DATA_BLOCK_SIZE    SIZE_128_BITS
#define SIZE_DATA_HASH          SIZE_160_BITS
#define SIZE_PUB_KEY_HASH       SIZE_160_BITS
#define SIZE_UUID               SIZE_16_BYTES
#define SIZE_MAC_ADDR           SIZE_6_BYTES
#define SIZE_PUB_KEY            SIZE_192_BYTES /* 1536 BITS */
#define SIZE_ENROLLEE_NONCE     SIZE_128_BITS

#define SIZE_AUTHORIZEDMACS_NUM	5

#endif /* _WPS_TYPE_ */
