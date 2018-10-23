/*
 * Registrar protocol
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
 * $Id: reg_proto.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _REGPROT_
#define _REGPROT_

#ifdef __cplusplus
extern "C" {
#endif // endif

/* build message methods */
uint32 reg_proto_create_devinforsp(RegData *regInfo, BufferObj *msg);
uint32 reg_proto_create_m1(RegData *regInfo, BufferObj *msg);
uint32 reg_proto_create_ack(RegData *regInfo, BufferObj *msg, uint8 *regNonce);
uint32 reg_proto_create_nack(RegData *regInfo, BufferObj *msg, uint16 configError,
	uint8 *regNonce);

/* process message methods */
uint32 reg_proto_process_ack(RegData *regInfo, BufferObj *msg);
uint32 reg_proto_process_nack(RegData *regInfo, BufferObj *msg, uint16 *configError);

/* utility methods */
int reg_proto_BN_bn2bin(const BIGNUM *a, unsigned char *to);
uint32 reg_proto_generate_dhkeypair(DH **DHKeyPair);
uint32 reg_proto_generate_prebuild_dhkeypair(DH **DHKeyPair, uint8 *pre_privkey);
void reg_proto_generate_sha256hash(BufferObj *inBuf, BufferObj *outBuf);
void reg_proto_derivekey(BufferObj *KDK, BufferObj *prsnlString, uint32 keyBits, BufferObj *key);
bool reg_proto_validate_mac(BufferObj *data, uint8 *hmac, BufferObj *key);
bool reg_proto_validate_keywrapauth(BufferObj *data, uint8 *hmac, BufferObj *key);
void reg_proto_encrypt_data(BufferObj *plainText, BufferObj *encrKey, BufferObj *authKey,
	BufferObj *cipherText, BufferObj *iv);
void reg_proto_decrypt_data(BufferObj *cipherText, BufferObj *iv, BufferObj *encrKey,
	BufferObj *authKey, BufferObj *plainText);
uint32 reg_proto_generate_psk(IN uint32 length, OUT BufferObj *PSK);
uint32 reg_proto_check_nonce(IN uint8 *nonce, IN BufferObj *msg, IN int nonceType);
uint32 reg_proto_get_msg_type(uint32 *msgType, BufferObj *msg);
uint32 reg_proto_get_nonce(uint8 *nonce, BufferObj *msg, int nonceType);

uint32 reg_proto_vendor_ext_vp(DevInfo *devInfo, BufferObj *msg);
unsigned char *reg_proto_generate_priv_key(unsigned char *priv_key, unsigned char *pub_key_hash);

#ifdef __cplusplus
}
#endif // endif

#endif /* _REGPROT_ */
