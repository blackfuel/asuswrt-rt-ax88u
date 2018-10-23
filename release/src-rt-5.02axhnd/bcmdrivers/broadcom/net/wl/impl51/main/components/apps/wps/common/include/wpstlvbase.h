/*
 * WPS TLV
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
 * $Id: wpstlvbase.h 525052 2015-01-08 20:18:35Z $
 */
#ifndef __TLV__H
#define __TLV__H

#ifdef __cplusplus
extern "C" {
#endif // endif

#include <portability.h>
#include <wpserror.h>

#ifndef UNDER_CE /* to be used for everything other than WinCE */
	#ifndef __unaligned
	#define __unaligned
	#endif
#endif /* ifndef UNDER_CE */

/* Declare TLV header as extern, since it will be defined elsewhere */
typedef struct {
	uint16 attributeType;
	uint16 dataLength;
} WpsTlvHdr;

/* Buffer class */
#define BUF_BLOCK_SIZE 256
#define BUF_ALLOC_MAGIC 0x12345678

typedef struct {
	uint8 *pBase, *pCurrent;
	uint32 m_bufferLength;
	uint32 m_currentLength;
	uint32 m_dataLength;
	bool m_allocated;
	uint32 magic;
} BufferObj;

/* allocate object + data buffer and intialize internals */
BufferObj *buffobj_new(void);

/*
 * initialize as a deserializing buffer. In that case
 * we don't own the data buffer (allocated = false).
 */
void buffobj_dserial(BufferObj * b, uint8 *ptr, uint32 length);

/* create a buffer obj around an existing buffer (allocated = false) */
BufferObj * buffobj_setbuf(uint8 *buf, int len);
void buffobj_del(BufferObj *);
uint8 *buffobj_Advance(BufferObj *b, uint32 offset);
uint8 *buffobj_Pos(BufferObj *b);
uint32 buffobj_Length(BufferObj *b);
uint32 buffobj_Remaining(BufferObj *b);
uint8 *buffobj_Append(BufferObj *b, uint32 length, uint8 *pBuff);
uint8 *buffobj_GetBuf(BufferObj *b);
uint8 *buffobj_Set(BufferObj *b, uint8 *pos);
uint16 buffobj_NextType(BufferObj *b);
uint8 *buffobj_Reset(BufferObj *b);
uint8 *buffobj_RewindLength(BufferObj *b, uint32 length);
uint8 *buffobj_Rewind(BufferObj *b);
int buffobj_lock(BufferObj *b);
int buffobj_unlock(BufferObj *b);

/* WSC 2.0 */
uint8 buffobj_NextSubId(BufferObj *b);
uint8 *buffobj_Write(BufferObj *b, uint32 length, uint8 *pBuff);

/* TLV Base class */
typedef struct {
	void *next;
	uint16 m_type;
	uint16 m_len;
	uint8 *m_pos;
} tlvbase_s;

typedef struct {
	tlvbase_s tlvbase;
	uint8 m_data;
} TlvObj_uint8;

typedef struct {
	tlvbase_s tlvbase;
	uint16 m_data;
} TlvObj_uint16;

typedef struct {
	tlvbase_s tlvbase;
	uint32 m_data;
} TlvObj_uint32;

typedef struct {
	tlvbase_s tlvbase;
	uint8 *m_data;
	bool m_allocated;
} TlvObj_ptru;

typedef struct {
	tlvbase_s tlvbase;
	char *m_data;
	bool m_allocated;
} TlvObj_ptr;

/* Vendor Extension Subelement */
typedef struct {
	uint8 subelementId;
	uint8 subelementLen;
} WpsSubTlvHdr;

typedef struct {
	uint8 m_id;
	uint8 m_len;
	uint8 *m_pos;
} subtlvbase_s;

typedef struct {
	subtlvbase_s subtlvbase;
	uint8 m_data;
} SubTlvObj_uint8;

typedef struct {
	subtlvbase_s subtlvbase;
	uint16 m_data;
} SubTlvObj_uint16;

typedef struct {
	subtlvbase_s subtlvbase;
	uint32 m_data;
} SubTlvObj_uint32;

typedef struct {
	subtlvbase_s subtlvbase;
	uint8 *m_data;
	bool m_allocated;
} SubTlvObj_ptru;

typedef struct {
	subtlvbase_s subtlvbase;
	char *m_data;
	bool m_allocated;
} SubTlvObj_ptr;

#ifdef  __cplusplus
}
#endif // endif

#endif /* WPS_TLV_H */
