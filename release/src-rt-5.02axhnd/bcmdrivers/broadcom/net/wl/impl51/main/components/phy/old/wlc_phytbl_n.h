/*
 * Declarations for Broadcom PHY core tables,
 * Networking Adapter Device Driver.
 *
 * THIS IS A GENERATED FILE - DO NOT EDIT (ARE WE SURE??)
 * Generated on Wed Aug 30 17:06:38 PDT 2006
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
 * All Rights Reserved.
 *
 * $Id: wlc_phytbl_n.h 606042 2015-12-14 06:21:23Z $
 */
/* FILE-CSTYLED */

#ifndef _WLC_PHYTBL_N_H_
#define _WLC_PHYTBL_N_H_

/* The position of the ant_swctrl_tbl in the volatile array of tables */
#define ANT_SWCTRL_TBL_REV3_IDX (0)

typedef phytbl_info_t mimophytbl_info_t;

typedef struct _phytbl_init {
	uint16	base;
	uint8 	num;
} phytbl_init_t;

extern CONST mimophytbl_info_t mimophytbl_info_rev0[], mimophytbl_info_rev0_volatile[];
extern CONST uint32 mimophytbl_info_sz_rev0, mimophytbl_info_sz_rev0_volatile;

extern CONST mimophytbl_info_t mimophytbl_info_rev3[], mimophytbl_info_rev3_volatile[],
        mimophytbl_info_rev3_volatile1[],  mimophytbl_info_rev3_volatile2[],
        mimophytbl_info_rev3_volatile3[];
extern CONST uint32 mimophytbl_info_sz_rev3, mimophytbl_info_sz_rev3_volatile,
        mimophytbl_info_sz_rev3_volatile1, mimophytbl_info_sz_rev3_volatile2,
        mimophytbl_info_sz_rev3_volatile3;

extern CONST uint32 noise_var_tbl_rev3[];

extern CONST mimophytbl_info_t mimophytbl_info_rev7[];
extern CONST uint32 mimophytbl_info_sz_rev7;
extern CONST uint32 noise_var_tbl_rev7[];
extern CONST uint32 tmap_tbl_rev7[];

extern CONST mimophytbl_info_t mimophytbl_info_rev8to10[];
extern CONST uint32 mimophytbl_info_sz_rev8to10;
extern CONST uint32 frame_struct_rev8[];

extern CONST uint32 frame_struct_rev8_redux[];
extern CONST uint32 tmap_tbl_rev7_redux[];
extern CONST phytbl_init_t frame_struct_rev8_offsets[];
extern CONST phytbl_init_t tmap_tbl_rev7_offsets[];
extern CONST uint8 chanest_tbl_rev3_offsets[];

extern CONST mimophytbl_info_t mimophytbl_info_43236[];
extern CONST uint32 mimophytbl_info_sz_43236;

/* LCNXN */
extern CONST mimophytbl_info_t mimophytbl_info_rev16[];
extern CONST uint32 mimophytbl_info_sz_rev16;

#endif /* _WLC_PHYTBL_N_H_ */
