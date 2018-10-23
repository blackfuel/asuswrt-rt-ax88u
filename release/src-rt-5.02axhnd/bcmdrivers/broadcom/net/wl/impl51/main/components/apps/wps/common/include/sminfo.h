/*
 * State machine infomation
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
 * $Id: sminfo.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _SM_INFO_H
#define _SM_INFO_H

#include <wps_devinfo.h>

/* data structures for each instance of registration protocol */
typedef enum {
	START = 0,
	CONTINUE,
	RESTART,
	SUCCESS,
	FAILURE
} ESMState;

typedef enum {
	MSTART = 0,
	M1,
	M2,
	M2D,
	M3,
	M4,
	M5,
	M6,
	M7,
	M8,
	DONE,
	MNONE = 99
} EMsg;

/*
 * data structure to store info about a particular instance
 * of the Registration protocol
 */
typedef struct {
	ESMState e_smState;
	EMsg e_lastMsgRecd;
	EMsg e_lastMsgSent;

	TRANSPORT_TYPE transportType;
	bool initialized;

	/* TODO: must store previous message as well to compute hash */

	/* enrollee endpoint - filled in by the Registrar, NULL for Enrollee */
	DevInfo *enrollee;

	/* Registrar endpoint - filled in by the Enrollee, NULL for Registrar */
	DevInfo *registrar;

	DevInfo *dev_info;

	/* Diffie Hellman parameters */
	BIGNUM *DH_PubKey_Peer; /* peer's pub key stored in bignum format */
	uint8 pke[SIZE_PUB_KEY]; /* enrollee's raw pub key */
	uint8 pkr[SIZE_PUB_KEY]; /* registrar's raw pub key */

	uint8 enrolleeNonce[SIZE_128_BITS]; /* N1 */
	uint8 registrarNonce[SIZE_128_BITS]; /* N2 */

	uint8 psk1[SIZE_128_BITS];
	uint8 psk2[SIZE_128_BITS];

	uint8 eHash1[SIZE_256_BITS];
	uint8 eHash2[SIZE_256_BITS];
	uint8 es1[SIZE_128_BITS];
	uint8 es2[SIZE_128_BITS];

	uint8 rHash1[SIZE_256_BITS];
	uint8 rHash2[SIZE_256_BITS];
	uint8 rs1[SIZE_128_BITS];
	uint8 rs2[SIZE_128_BITS];

	BufferObj *authKey;
	BufferObj *keyWrapKey;
	BufferObj *emsk;
	BufferObj *x509csr;
	BufferObj *x509Cert;

	BufferObj *inMsg; /* A recd msg will be stored here */
	BufferObj *outMsg; /* Contains msg to be transmitted */
} RegData;

RegData *state_machine_new();
void state_machine_delete(RegData *reg_info);
uint32 state_machine_init_sm(RegData *reg_info);

#endif /* _SM_INFO_H */
