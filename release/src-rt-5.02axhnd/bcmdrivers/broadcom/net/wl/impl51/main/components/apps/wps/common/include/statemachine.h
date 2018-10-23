/*
 * State Machine
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
 * $Id: statemachine.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#ifdef __cplusplus
extern "C" {
#endif // endif

#if defined(__linux__)
	#define stringPrintf snprintf
#else
	#define stringPrintf _snprintf
#endif // endif

#define WPS_PROTOCOL_TIMEOUT 120
#define WPS_MESSAGE_TIMEOUT 25

/* M2D Status values */
#define SM_AWAIT_M2     0
#define SM_RECVD_M2     1
#define SM_RECVD_M2D    2
#define SM_M2D_RESET    3

uint32 state_machine_send_ack(RegData *reg_info, BufferObj *outmsg, uint8 *regNonce);
uint32 state_machine_send_nack(RegData *reg_info, uint16 configError, BufferObj *outmsg,
	uint8 *regNonce);

typedef struct {
	RegData *reg_info;

	/* The peer's encrypted settings will be stored here */
	void *mp_peerEncrSettings;
	bool m_localSMEnabled;
	bool m_passThruEnabled;

	/* Temporary state variable */
	bool m_sentM2;
	void *g_mc;

	/* External Registrar variables */
	bool m_er_sentM2;
	uint8 m_er_nonce[SIZE_128_BITS];

	uint32 err_code; /* Real err code, initially used for RPROT_ERR_INCOMPATIBLE_WEP */
} RegSM;

RegSM *reg_sm_new(void *g_mc);
void reg_sm_delete(RegSM *r);
uint32 reg_sm_initsm(RegSM *r, DevInfo *p_enrolleeInfo, bool enableLocalSM, bool enablePassthru);
uint32 reg_sm_step(RegSM *r, uint32 msgLen, uint8 *p_msg, uint8 *outbuffer, uint32 *out_len);
uint32 reg_sm_get_devinfo_step(RegSM *r); /* brcm */
void reg_sm_restartsm(RegSM *r);
uint32 reg_sm_handleMessage(RegSM *r, BufferObj *msg, BufferObj *outmsg);

typedef struct {
	RegData *reg_info;

	uint32 m_m2dStatus;

	/* The peer's encrypted settings will be stored here */
	void *mp_peerEncrSettings;
	void *g_mc;

	uint32 err_code; /* Real err code, initially used for RPROT_ERR_INCOMPATIBLE_WEP */
} EnrSM;

uint32 enr_sm_step(EnrSM *e, uint32 msgLen, uint8 *p_msg, uint8 *outbuffer, uint32 *out_len);
EnrSM * enr_sm_new(void *g_mc);
void enr_sm_delete(EnrSM *e);
uint32 enr_sm_initializeSM(EnrSM *e, DevInfo *dev_info);
void enr_sm_restartSM(EnrSM *e);
uint32 enr_sm_handleMessage(EnrSM *e, BufferObj *msg, BufferObj *outmsg);

#ifdef __cplusplus
}
#endif // endif

#endif /* __STATEMACHINE_H__ */
