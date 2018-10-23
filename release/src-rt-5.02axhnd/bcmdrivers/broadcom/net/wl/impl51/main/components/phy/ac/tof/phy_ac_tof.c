/*
 * ACPHY TOF module implementation
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
 * $Id: phy_ac_tof.c 639713 2016-05-24 18:02:57Z $
 */
#ifdef WL_PROXDETECT

#include <hndpmu.h>
#include <sbchipc.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <phy_utils_var.h>
#include <phy_rstr.h>
#include <phy_cache.h>
#include "phy_type_tof.h"
#include <phy_ac.h>
#include <phy_rxgcrs_api.h>
#include <phy_tof_api.h>
#include <phy_ac_info.h>
#include <phy_ac_tof.h>
#include <phy_stf.h>

static const  uint16 BCMATTACHDATA(proxd_4345_80m_k_values)[] =
{ 0x0, 0xee12, 0xe201, 0xe4fc, 0xe6f8, 0xe6f7 /* 42, 58, 106, 122, 138, 155 */ };

static const  uint16 BCMATTACHDATA(proxd_4345_40m_k_values)[] =
{ 0x7b7b, 0x757c, 0x7378, 0x7074, 0x9a9a, 0x9898, /* 38, 46, 54, 62, 102,110 */
0x9898, 0x9898, 0x9393, 0x9494, 0x9191, 0x8484 /* 118, 126, 134, 142,151,159 */ };

static const  uint16 BCMATTACHDATA(proxd_4345_20m_k_values)[] =
{ 0x0f0f, 0x0101, 0x0101, 0x1313, 0x0f0f, 0x0101, 0x0f0f, 0x0505, /* 36 -64 */
0xe9e9, 0xe8e8, 0xe6e6, 0xe4e4, 0xcbcb, 0xcbcb, 0xcbcb, 0xcbcb, /* 100 -128 */
0xcbcb, 0xd5d5, 0xdada, 0xcbcb, /* 132 -144 */
0xcbcb, 0xbfbf, 0xd5d5, 0xbfbf, 0xcbcb /* 149 - 165 */
};

static const  uint16 BCMATTACHDATA(proxd_4345_2g_k_values)[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, /* 1 -7 */
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 /* 8 -14 */
};

static const  uint16 BCMATTACHDATA(proxd_4350_80m_k_values)[] =
{ 0x0, 0xef02, 0xf404, 0xf704, 0xfc04, 0xf3f9 /* 42, 58, 106, 122, 138, 155 */ };

static const  uint16 BCMATTACHDATA(proxd_4350_40m_k_values)[] =
{ 0x0, 0xfdfd, 0xf6f6, 0x1414, 0xebeb, 0xebeb, /* 38, 46, 54, 62, 102,110 */
0xeeee, 0xeeee, 0xe2e2, 0xe5e5, 0xfdfa, 0xe5e5 /* 118, 126, 134, 142,151,159 */ };

static const  uint16 BCMATTACHDATA(proxd_4350_20m_k_values)[] =
{ 0x0, 0xfdfd, 0xfdfd, 0xf8f8, 0xf8f8, 0xf5f5, 0xf5f5, 0xf5f5, /* 36 -64 */
0xe9e9, 0xe6e6, 0xe3e3, 0xe3e3, 0xe6e6, 0xe6e6, 0xe6e6, 0xe6e6, /* 100 - 128 */
0xe6e6, 0xe6e6, 0xd6d6, 0xe9e9, /* 132 -144 */
0xe9e9, 0x0808, 0xd4d4, 0xd4d4, 0xd4d4 /* 149 - 165 */
};

static const  uint16 BCMATTACHDATA(proxd_4350_2g_k_values)[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, /* 1 -7 */
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 /* 8 -14 */
};

static const  uint16 BCMATTACHDATA(proxd_4354_80m_k_values)[] =
{ 0, 0xF0F3, 0xFCFE, 0, 0xFEFE, 0xF0F4 /* 42, 58, 106, 122, 138, 155 */ };

static const  uint16 BCMATTACHDATA(proxd_4354_40m_k_values)[] =
{ 0, 0xFC04, 0xFA00, 0x0d1D, 0xF1FB, 0xF0FB, /* 38, 46, 54, 62, 102 */
0, 0, 0xE9F5, 0xE5F4, 0xFE0B, 0xE0EF /* 110, 118, 126, 134, 142,151,159 */ };

static const  uint16 BCMATTACHDATA(proxd_4354_20m_k_values)[] =
{ 0, 0xFFFE, 0xFAFB, 0xFBFA, 0xF9F8, 0xFAF7, 0xF4F4, 0xFDEC, /* 36 -64 */
0xEEE1, 0xE3E3, 0xE2E0, 0xE2E2, 0, 0, 0, 0, 0xD9DB, 0xDEDD, 0xD7D7, 0xD9D6, /* 100 -144 */
0x0BFD, 0x0209, 0xD6D1, 0xE1C6, 0xE1C6 /* 149 - 165 */
};

static const  uint16 BCMATTACHDATA(proxd_4354_2g_k_values)[] =
{ 0xbbbb, 0xf1f1, 0xebeb, 0xe5e5, 0xe6e6, 0xe6e6, 0xe3e3, /* 1 -7 */
0xe6e6, 0xe3e3, 0xe3e3, 0xe6e6, 0xb5b5, 0xf0f0, 0x0c0c /* 8 -14 */
};

static const  uint16 BCMATTACHDATA(proxd_4349_80m_k_values)[] =
{ 0, 0xFFFE, 0xF2F2, 0xF2F2, 0xF0EF, 0xE8E7 /* 42, 58, 106, 122, 138, 155 */ };

static const  uint16 BCMATTACHDATA(proxd_4349_40m_k_values)[] =
{ 0, 0x00FF, 0x00FF, 0xFFFF, 0xFAFB, 0xF2F3, /* 38, 46, 54, 62, 102 110 */
0xF2F3, 0xF2F3, 0xE7E8, 0xE7E8, 0xE5E6, 0xE5E6 /* 118, 126, 134, 142,151,159 */ };

static const  uint16 BCMATTACHDATA(proxd_4349_20m_k_values)[] =
{ 0, 0x0, 0x0300, 0x0300, 0xFCFF, 0xFAFC, 0xF9F9, 0xF9F7, /* 36 -64 */
0xEFEE, 0xE8EC, 0xE9E9, 0xE6E8, 0xCDCE, 0xCDCE, 0xCDCE, 0xCDCE, 0xCCCF, 0xCCCF,  /* 100 -136 */
0xC8CC, 0xC6CA, 0xC6CA, 0xC5C8, 0xC6C7, 0xC5C6, 0xC5C6 /* 140 - 165 */
};

static const  uint16 BCMATTACHDATA(proxd_4349_2g_k_values)[] =
{ 0x2222, 0x1e1e, 0x1919, 0x1313, 0x0f0f, 0x0e0e, 0x0f0f, /* 1 -7 */
0x1111, 0x1313, 0x1313, 0x1515, 0x1818, 0x1d1d, 0x3939 /* 8 -14 */
};

static const  uint16 BCMATTACHDATA(proxd_43602_80m_k_values)[] =
{ 0x0000, 0x0000, 0x0000, 0, 0x0000, 0x0000 /* 42, 58, 106, 122, 138, 155 */ };

static const  uint16 BCMATTACHDATA(proxd_43602_40m_k_values)[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, /* 38, 46, 54, 62, 102 110 */
0x0, 0x0, 0x0, 0x0, 0x0, 0x0 /* 118, 126, 134, 142,151,159 */ };

static const  uint16 BCMATTACHDATA(proxd_43602_20m_k_values)[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, /* 36 -64 */
0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0x0, 0x0, 0x0, 0x0, /* 100 -144 */
0x0, 0x0, 0x0, 0x0, 0x0 /* 149 - 165 */
};

static const  uint16 BCMATTACHDATA(proxd_43602_2g_k_values)[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, /* 1 -7 */
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 /* 8 -14 */
};

/* 4361 k values */
#define TOF_INITIATOR_K_4361_80M      35934 /* initiator K value for 80M */
#define TOF_TARGET_K_4361_80M         35934 /* target K value for 80M */

#define TOF_INITIATOR_K_4361_40M      35490 /* initiator K value for 40M */
#define TOF_TARGET_K_4361_40M         35490 /* target K value for 40M */

#define TOF_INITIATOR_K_4361_20M      37412 /* initiator K value for 20M */
#define TOF_TARGET_K_4361_20M         37416 /* target K value for 20M */

#define TOF_INITIATOR_K_4361_2G       37492 /* initiator K value for 2G */
#define TOF_TARGET_K_4361_2G          37492 /* target K value for 2G */

static const  uint16 BCMATTACHDATA(proxd_4361_80m_k_values)[] =
{0x1818, 0, 0, 0, 0, 0 /* 42, 58, 106, 122, 138, 155 */};

static const  uint16 BCMATTACHDATA(proxd_4361_40m_k_values)[] =
{0x2020, 0x1a1a, 0, 0, 0, 0, /* 38, 46, 54, 62, 102 110 */
0, 0, 0, 0, 0x0000, 0x0000 /* 118, 126, 134, 142,151,159 */};

static const  uint16 BCMATTACHDATA(proxd_4361_20m_k_values)[] =
{0x3838, 0x3838, 0x3030, 0x3030, 0x0, 0x0, 0x0, 0x0, /* 36 -64 */
0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0x0, 0x0, 0x0, 0x0, /* 100 -144 */
0x0505, 0x0000, 0x0000, 0x0a0a, 0x0a0a /* 149 - 165 */
};

static const  uint16 BCMATTACHDATA(proxd_4361_2g_k_values)[] =
{0x0606, 0x0606, 0x0606, 0x0606, 0x0, 0x0, 0x0, /* 1 -7 */
0x0, 0x00303, 0x00303, 0x00303, 0x00303, 0x0303, 0x0303 /* 8 -14 */
};

/* ratespec related k offset table for initiator <legacy 6M, legacy non-6M, HT-MCS0, HT-MCS(1-7)> */
static const  int16 BCMATTACHDATA(proxdi_rate_offset_2g)[] = { 0, 0, 0, 0 };
static const  int16 BCMATTACHDATA(proxdi_rate_offset_20m)[] = { 0, 0, 0, 0 };
static const  int16 BCMATTACHDATA(proxdi_rate_offset_40m)[] = { 0, 0, 0, 0 };
static const  int16 BCMATTACHDATA(proxdi_rate_offset_80m)[] = { 0, 0, 0, 0 };

/* ratespec related k offset table for target <legacy 6M, legacy non-6M, HT-MCS0, HT-MCS(1-7)> */
static const  int16 BCMATTACHDATA(proxdt_rate_offset_2g)[] = { 0, 13731, 61, 5743 };
static const  int16 BCMATTACHDATA(proxdt_rate_offset_20m)[] = { 0, 13768, 236, 5776 };
static const  int16 BCMATTACHDATA(proxdt_rate_offset_40m)[] = { 0, 14779, 7, 6333 };
static const int16 BCMATTACHDATA(proxdt_rate_offset_80m)[] = { 0, 13400, -2804, -2804 };

/* legacy ack offset table for initiator  <80M, 40M, 20M, 2g> */
static const int16 BCMATTACHDATA(proxdi_ack_offset)[] = { -2, -3, 2360, 2333 };

/* legacy ack offset table for initiator  <80M, 40M, 20M, 2g> */
static const int16 BCMATTACHDATA(proxdt_ack_offset)[] = { 2728, 2031, 0, 0 };

/* different bandwidth  k offset table <VHT legacy 6M, legacy non-6M, HT-MCS0, HT-MCS(1-7)> */
static const int16 BCMATTACHDATA(proxd_subbw_offset)[3][5] = {
	/* 80M-40M */
	{ 1036, 325, 1366, 2094, 5273 },
	/* 80M -20M */
	{ 1470, -170, 1400, 1412, 5507 },
	/* 40M - 20M */
	{ 200, -714, -216, -800, 20 }
};

#ifdef WL_PROXD_SEQ

static const uint16 k_tof_seq_tiny_tbls[] = {
	ACPHY_TBL_ID_RFSEQ,
	0x260,
	15,
	0x42, 0x10, 0x8b, 0x9b, 0xaa, 0xb9, 0x8c, 0x9c, 0x9d,
	0xab, 0x8d, 0x9e, 0x43, 0x42, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x290,
	14,
	0x42, 0x10, 0x88, 0x98, 0xa8, 0xb8, 0x89, 0x99, 0xa9,
	0x8a, 0x9a, 0x43, 0x42, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x2f0,
	15,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04, 0x04,
	0x04, 0x01, 0x04, 0x1e, 0x01, 0x01,
	ACPHY_TBL_ID_RFSEQ,
	0x320,
	14,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04, 0x04,
	0x01, 0x04, 0x1e, 0x01, 0x01,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x008,
	18,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x018,
	21,
	0x0009, 0x8000, 0x0007, 0xf009, 0x8066, 0x0007, 0xf009, 0x8066, 0x0004,
	0x00c9, 0x8060, 0x0007, 0xf7d9, 0x8066, 0x0007, 0xf7f9, 0x8066, 0x0007,
	0xf739, 0x8066, 0x0004,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x028,
	12,
	0x0009, 0x0000, 0x0000, 0x0229, 0x0000, 0x0000, 0x0019, 0x0000, 0x0000,
	0x0099, 0x0000, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x038,
	6,
	0x0fc9, 0x00a6, 0x0000, 0x0fc9, 0x00a6, 0x0000,
};

/* RF Seq Bundle Table with RF Controls for TX ON and TX OFF in 2G mode */
static const uint16 k_tof_seq_tiny_tbls_2G[] = {
	ACPHY_TBL_ID_RFSEQ,
	0x260,
	15,
	0x42, 0x10, 0x8b, 0x9b, 0xaa, 0xb9, 0x8c, 0x9c, 0x9d,
	0xab, 0x8d, 0x9e, 0x43, 0x42, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x290,
	14,
	0x42, 0x10, 0x88, 0x98, 0xa8, 0xb8, 0x89, 0x99, 0xa9,
	0x8a, 0x9a, 0x43, 0x42, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x2f0,
	15,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04, 0x04,
	0x04, 0x01, 0x04, 0x1e, 0x01, 0x01,
	ACPHY_TBL_ID_RFSEQ,
	0x320,
	14,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04, 0x04,
	0x01, 0x04, 0x1e, 0x01, 0x01,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x008,
	18,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x018,
	21,
	0x0009, 0x8000, 0x0007, 0xf009, 0x801d, 0x0007, 0xf009, 0x801d, 0x0004,
	0x00c9, 0x8018, 0x0007, 0xf7d9, 0x801d, 0x0007, 0xf7f9, 0x801d, 0x0007,
	0xf739, 0x801d, 0x0004,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x028,
	12,
	0x0009, 0x0000, 0x0000, 0x0129, 0x0000, 0x0000, 0x0019, 0x0000, 0x0000,
	0x0059, 0x0000, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x038,
	6,
	0x0fc9, 0x00a6, 0x0000, 0x0fc9, 0x00a6, 0x0000,
};

static const uint16 k_tof_seq_tbls[] = {
	ACPHY_TBL_ID_RFSEQ,
	0x260,
	14,
	0x10, 0x8b, 0x9b, 0xaa, 0xb9, 0xc9, 0xd9, 0x8c, 0x9c,
	0x9d, 0xab, 0x8d, 0x9e, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x290,
	13,
	0x10, 0x88, 0x98, 0xa8, 0xb8, 0xc8, 0xd8, 0x89, 0x99,
	0xa9, 0x8a, 0x9a, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x2f0,
	14,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04,
	0x04, 0x04, 0x01, 0x04, 0x01,
	ACPHY_TBL_ID_RFSEQ,
	0x320,
	13,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04,
	0x04, 0x01, 0x04, 0x01,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x008,
	18,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x018,
	21,
	0x0009, 0x8000, 0x000f, 0xff09, 0x8066, 0x000f, 0xff09, 0x8066, 0x000c,
	0x00c9, 0x8060, 0x000f, 0xffd9, 0x8066, 0x000f, 0xfff9, 0x8066, 0x000f,
	0xff39, 0x8066, 0x000c,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x028,
	12,
	0x0009, 0x0000, 0x0000, 0x0229, 0x0000, 0x0000, 0x0019, 0x0000, 0x0000,
	0x0099, 0x0000, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x038,
	6,
	0x0fc9, 0x00a6, 0x0000, 0x0fc9, 0x00a6, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x048,
	6,
	0x2099, 0x586e, 0x0000, 0x1519, 0x586e, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x058,
	6,
	0x2409, 0xc040, 0x000c, 0x6b89, 0xc040, 0x000c,
};

const uint16 k_tof_seq_tbls_2G[] = {
	ACPHY_TBL_ID_RFSEQ,
	0x260,
	15,
	0x0, 0x10, 0x8b, 0x9b, 0xaa, 0xb9, 0xc9, 0xd9, 0x8c, 0x9c,
	0x9d, 0xab, 0x8d, 0x9e, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x290,
	14,
	0x0, 0x10, 0x88, 0x98, 0xa8, 0xb8, 0xc8, 0xd8, 0x89, 0x99,
	0xa9, 0x8a, 0x9a, 0x1f,
	ACPHY_TBL_ID_RFSEQ,
	0x2f0,
	15,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04,
	0x04, 0x04, 0x01, 0x04, 0x01,
	ACPHY_TBL_ID_RFSEQ,
	0x320,
	14,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x01, 0x04,
	0x04, 0x01, 0x04, 0x01,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x008,
	18,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	0x0000, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x018,
	21,
	0x0009, 0x8000, 0x0003, 0x7f09, 0x801d, 0x0003, 0x7f09, 0x801d, 0x0000,
	0x00c9, 0x8018, 0x0003, 0x7fd9, 0x801d, 0x0003, 0x7ff9, 0x801d, 0x0003,
	0x7f39, 0x801d, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x028,
	12,
	0x0009, 0x0000, 0x0000, 0x0129, 0x0000, 0x0000, 0x0019, 0x0000, 0x0000,
	0x0059, 0x0000, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x038,
	6,
	0x0fc9, 0x00a6, 0x0000, 0x0fc9, 0x00a6, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x048,
	6,
	0x2099, 0x586e, 0x0000, 0x1519, 0x586e, 0x0000,
	ACPHY_TBL_ID_RFSEQBUNDLE,
	0x058,
	6,
	0x2409, 0xc040, 0x000c, 0x6b89, 0xc040, 0x000c,
};

static const uint16 k_tof_seq_fem_gains[] = {
	(8 | (1 << 5) | (1 << 9)), /* fem hi */
	(8 | (1 << 5)), /* fem lo */
};

const uint16 k_tof_seq_fem_gains_2g[] = {
	(8 | (1 << 5) | (1 << 8)), /* fem hi */
	(8 | (1 << 4) | (1 << 8)), /* fem lo */
};

#endif /* WL_PROXD_SEQ */

/* adjust the window for the EB delay, delay is in tenth of nano-sec */
#define TOF_W_ADJ_EMU(emu_delay, FS) ((emu_delay * 2 * FS) / 10000)
/* are we using the emulator box? */
#define TOF_EB(emu_delay) ((emu_delay > 2000) ? 1 : 0)
/* extend the sc buffer if using EB */
#define TOF_BUF_EXT 670
#define TOF_RX_MODE_EXT 8 /* extend the Rx mode when EB is used, in usec */
#define TOF_SC_FS_80MHZ 160
#define TOF_SC_FS_40MHZ 80
#define TOF_SC_FS_20MHZ 40

const uint32 k_tof_seq_spb_tx[2 * K_TOF_SEQ_SPB_LEN_MAX] = { 0 };
const uint32 k_tof_seq_spb_rx[2 * K_TOF_SEQ_SPB_LEN_MAX] = { 0 };

const uint16 band_length_20MHz[K_TOF_NUM_LEGACY_BL_20M] = { 25, 25 };
const uint16 nonzero_sc_idx_legacy_20MHZ[K_TOF_NUM_LEGACY_NZ_SC_20M] = {
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 62
};

const uint16 band_length_80MHz[K_TOF_NUM_LEGACY_BL_80M] = { 13, 13, 13, 10, 10, 13, 13, 13 };
const uint16 nonzero_sc_idx_legacy_80MHZ[K_TOF_NUM_LEGACY_NZ_SC_80M] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
	198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
	227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253,
};

#if defined(TOF_SEQ_20MHz_BW) || defined(TOF_SEQ_20MHz_BW_512IFFT)
/* 20MHz case */
static const uint16 k_tof_ucode_dlys_us_20MHz[2][5] = {
	{ 1, 8, 8, TOF_W(200, 2, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
	200, 0, K_TOF_SEQ_LOG2_N_20MHZ) }, /* RX -> TX */

	{ 2, 6, 8, TOF_W(480, -2, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
	480, 0, K_TOF_SEQ_LOG2_N_20MHZ) }, /* TX -> RX */
};

/* 2G case */
const uint16 k_tof_ucode_dlys_us_2g_20MHz[2][5] = {
	{ 1, 21, 10, TOF_W(564, 4, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
	564, 0, K_TOF_SEQ_LOG2_N_20MHZ) }, /* RX -> TX 1012, 500 */
	{ 8, 11, 13, TOF_W(1084, -4, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
	1084, 0, K_TOF_SEQ_LOG2_N_20MHZ) }, /* TX -> RX 508, 1020 */
};

#ifdef TOF_DLY_290us_313us
/* 2G case */
const uint16 K_TOF_UCODE_DLYS_US_2G_20MHZ_2_0[2][5] = {
	{1, 29, 7, TOF_W(444, 7, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
		444, 0, K_TOF_SEQ_LOG2_N_20MHZ)}, /* 1900, 364 RX->TX Orig=428, n=12 */
	{9, 7, 47, TOF_W(1434, -7, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
		1434, 0, K_TOF_SEQ_LOG2_N_20MHZ)}, /* 444, 1980 TX->RX */
};
#endif // endif

/* 2G Secure Ranging 2.0 case */
/* For Delays 273us from Target, 313us from Initiator */
const uint16 K_TOF_UCODE_DLYS_US_2G_20MHZ_2_0[2][5] = {
	{1, 45, 7, TOF_W(404, 12, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
		404, 0, K_TOF_SEQ_LOG2_N_20MHZ)}, /* 1900, 364 RX->TX Orig=428, n=12 */
	{9, 7, 47, TOF_W(2044, -12, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
		2044, 0, K_TOF_SEQ_LOG2_N_20MHZ)}, /* 444, 1980 TX->RX */
};

/* 2G case for 4360 and 43602 */
const uint16 K_TOF_UCODE_DLYS_US_2G_20MHZ_4360[2][5] = {
	{1, 21, 10, TOF_W(554, 4, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
		554, 0, K_TOF_SEQ_LOG2_N_20MHZ)}, /* RX -> TX 1000, 488 */
	{8, 11, 13, TOF_W(1094, -4, K_TOF_SEQ_LOG2_N_20MHZ), TOF_W(
		1094, 0, K_TOF_SEQ_LOG2_N_20MHZ)}, /* TX -> RX 488, 1000 */
};
#endif /* TOF_SEQ_20MHz_BW || TOF_SEQ_20MHz_BW_512IFFT */

#ifdef TOF_SEQ_40MHz_BW
/* For 20 in 40MHz */
static const uint16 k_tof_ucode_dlys_us_40MHz[2][5] = {
	{ 1, 8, 8, TOF_W(338, 3, K_TOF_SEQ_LOG2_N_40MHZ), TOF_W(
	338, 0, K_TOF_SEQ_LOG2_N_40MHZ) },  /* RX -> TX */
	{ 2, 6, 8, TOF_W(1188, -3, K_TOF_SEQ_LOG2_N_40MHZ), TOF_W(
	1188, 0, K_TOF_SEQ_LOG2_N_40MHZ) }, /* TX -> RX */
};
#endif /* TOF_SEQ_40MHz_BW */

#ifdef TOF_SEQ_40_IN_40MHz
/* For 40 in 40MHz */
static const uint16 k_tof_ucode_dlys_us_40MHz_40[2][5] = {
	{ 1, 8, 8, TOF_W(338, 3, K_TOF_SEQ_LOG2_N_40MHZ), TOF_W(
	338, 0, K_TOF_SEQ_LOG2_N_40MHZ) },  /* RX -> TX */
	{ 2, 6, 8, TOF_W(1188, -3, K_TOF_SEQ_LOG2_N_40MHZ), TOF_W(
	1188, 0, K_TOF_SEQ_LOG2_N_40MHZ) }, /* TX -> RX */
};
#endif /* TOF_SEQ_40_IN_40MHz */

#ifdef TOF_SEQ_20_IN_80MHz
/* For 20 in 80MHz */
static const uint16 k_tof_ucode_dlys_us_80MHz_20[2][5] = {
	{ 1, 8, 8, TOF_W(698, 3, K_TOF_SEQ_LOG2_N_80MHZ_20), TOF_W(
	698, 0, K_TOF_SEQ_LOG2_N_80MHZ_20) }, /* RX -> TX */
	{ 2, 6, 8, TOF_W(2368, -3, K_TOF_SEQ_LOG2_N_80MHZ_20), TOF_W(
	2368, 0, K_TOF_SEQ_LOG2_N_80MHZ_20) }, /* TX -> RX */
};
#endif /* TOF_SEQ_20_IN_80MHz */

#if !defined(TOF_SEQ_20_IN_80MHz)
/* Original 80MHz case */
static const uint16 k_tof_ucode_dlys_us_80MHz[2][5] = {
	{ 1, 6, 6, TOF_W(750, 4, K_TOF_SEQ_LOG2_N_80MHZ), TOF_W(
	750, 0, K_TOF_SEQ_LOG2_N_80MHZ) }, /* RX -> TX */
	{ 2, 6, 6, TOF_W(1850, -4, K_TOF_SEQ_LOG2_N_80MHZ), TOF_W(
	1850, 0, K_TOF_SEQ_LOG2_N_80MHZ) }, /* TX -> RX */
};
#endif /* !TOF_SEQ_20_IN_80MHz */

#if defined(TOF_SEQ_20_IN_80MHz) || defined(TOF_SEQ_20MHz_BW) || \
	defined(TOF_SEQ_40MHz_BW) || defined(TOF_SEQ_20MHz_BW_512IFFT)
static const uint32 k_tof_seq_spb_20MHz[2 * K_TOF_SEQ_SPB_LEN_20MHZ] = {
	0x1f11ff10,
	0x1fffff1f,
	0x1f1f1ff1,
	0x000ff111,
	0x11110000,
	0x1f1f11ff,
	0x1ff11111,
	0x1111f1f1,
};
#endif /* TOF_SEQ_20_IN_80MHz || TOF_SEQ_20MHz_BW || TOF_SEQ_40MHz_BW || TOF_SEQ_20MHz_BW_512IFFT */

#ifdef TOF_SEQ_40_IN_40MHz
static const uint32 k_tof_seq_spb_40MHz[2 * K_TOF_SEQ_SPB_LEN_40MHZ] = {
	0xf11ff110,
	0x111111f1,
	0x1f1f11ff,
	0x1ff11111,
	0xfff1f1f1,
	0xf1ff11ff,
	0xff1111f1,
	0x0000001f,
	0xf0000000,
	0x1ff11f11,
	0x1111f1f1,
	0x1f11ff11,
	0xf111111f,
	0xf1f1f11f,
	0xff11ffff,
	0x1111f1f1,
};
#endif /* TOF_SEQ_40_IN_40MHz */

#if !defined(TOF_SEQ_20_IN_80MHz)
static const uint32 k_tof_seq_spb_80MHz[2 * K_TOF_SEQ_SPB_LEN_80MHZ] = {
	0xee2ee200,
	0xe2e2ee22,
	0xe22eeeee,
	0xeeee2e2e,
	0xe2ee22ee,
	0xe22222e2,
	0xe2e2e22e,
	0x00000eee,
	0x11000000,
	0x1f1f11ff,
	0x1ff11111,
	0x1111f1f1,
	0x1f11ff11,
	0x1fffff1f,
	0x1f1f1ff1,
	0x01fff111,
};
#endif /* !defined(TOF_SEQ_20_IN_80MHz) */

/* module private states */
struct phy_ac_tof_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_tof_info_t *ti;
	uint16 proxd_80m_k_values[6];
	uint16 proxd_40m_k_values[12];
	uint16 proxd_20m_k_values[25];
	uint16 proxd_2g_k_values[14];
	int16 proxdi_rate_offset_80m[4];
	int16 proxdi_rate_offset_40m[4];
	int16 proxdi_rate_offset_20m[4];
	int16 proxdi_rate_offset_2g[4];
	int16 proxdt_rate_offset_80m[4];
	int16 proxdt_rate_offset_40m[4];
	int16 proxdt_rate_offset_20m[4];
	int16 proxdt_rate_offset_2g[4];
	int16 proxdi_ack_offset[4];
	int16 proxdt_ack_offset[4];
	int16 proxd_subbw_offset[3][5];
	int32 rssi[PHY_CORE_MAX];
	uint16 proxd_ki[4];
	uint16 proxd_kt[4];
	uint16 tof_ucode_dlys_us[2][5];
	uint16 tof_rfseq_bundle_offset;
	uint16 tof_shm_ptr;
	uint32 *tof_seq_spb_tx;
	uint32 *tof_seq_spb_rx;
	uint8 tof_sc_FS;
	uint8 tof_seq_log2_n;
	uint8 tof_seq_spb_len;
	uint8 tof_rx_fdiqcomp_enable;
	uint8 tof_tx_fdiqcomp_enable;
	uint8 tof_core;
	uint8 tof_smth_enable;
	uint8 tof_smth_dump_mode;
	uint32 alloc_size;
	uint8 *rtx;
	uint8 *rrx;
	uint8 *ri_rr;
	int32 *chan;
	wl_proxd_phy_error_t tof_phy_error;
	wl_proxd_snr_t snr;
	wl_proxd_bitflips_t bitflips;
	bool tof_setup_done;
	bool tof_active;
	bool tof_smth_forced;
	bool tof_tx;
	uint32 tof_pllctrl;
	uint8 isInvalid;
	uint16 proxd_seq_kval[4];
	uint8 aci_en;
	uint8 aci_state;
	uint8 prev_aci_state;
	uint8 hwaci_sleep;
	int32 emu_delay;
	bool flag_sec_2_0;
	uint16 bundle_offs_38[3];
	uint16 bundle_offs_39[3];
	uint16 start_seq_time;
	uint16 delta_time_tx2rx;
};

/* Local functions */
static void phy_ac_tof_en_dis_aci(phy_ac_tof_info_t *tofi, bool enter);
static void phy_ac_tof_fdiqcomp_save_disable(phy_ac_tof_info_t *tofi);
static int phy_ac_tof(phy_type_tof_ctx_t *ctx, bool enter, bool tx, bool hw_adj,
	bool seq_en, int core, int emu_delay);
static int phy_ac_tof_info(phy_type_tof_ctx_t *ctx, wlc_phy_tof_info_t *tof_info,
	wlc_phy_tof_info_type_t tof_info_mask, uint8 core);
static void phy_ac_tof_cmd(phy_type_tof_ctx_t *ctx, bool seq, int emu_delay);
static int phy_ac_tof_seq_params_get_set(phy_type_tof_ctx_t *ctx, uint8 *delays,
	bool set, bool tx, int size);
static int phy_ac_tof_set_ri_rr(phy_type_tof_ctx_t *ctx, const uint8 *ri_rr, const uint16 len,
	const uint8 core, const bool isInitiator, const bool isSecure,
	wlc_phy_tof_secure_2_0_t secure_params);
static int phy_ac_tof_kvalue(phy_type_tof_ctx_t *ctx, chanspec_t chanspec, uint32 rspecidx,
	uint32 *kip, uint32 *ktp, uint8 seq_en);
static void phy_ac_tof_seq_upd_dly(phy_type_tof_ctx_t *ctx, bool tx, uint8 core,
	bool mac_suspend);
static int phy_ac_tof_seq_params(phy_type_tof_ctx_t *ctx, bool assign_buffer);
static int phy_ac_tof_chan_freq_response(phy_type_tof_ctx_t *ctx, int len, int nbits,
		bool swap_pn_half, uint32 offset, cint32* H, uint32* Hraw, uint8 core,
		uint32 sts_offset, const bool single_core);
static void phy_ac_tof_setup_ack_core(phy_type_tof_ctx_t *ctx, int core);
static void phy_ac_tof_core_select(phy_type_tof_ctx_t *ctx, const uint32 gdv_th,
		const int32 gdmm_th, const int8 rssi_th, const int8 delta_rssi_th,
		uint8* core, uint8 core_mask);
static int phy_ac_tof_calc_snr_bitflips(phy_type_tof_ctx_t *ctx, void *In,
	wl_proxd_bitflips_t *bit_flips, wl_proxd_snr_t *snr);
static int phy_ac_chan_mag_sqr_impulse_response(phy_type_tof_ctx_t *ctx, int frame_type,
	int len, int offset, int nbits, int32* h, int* pgd, uint32* hraw, uint16 tof_shm_ptr);
static int phy_ac_seq_ts(phy_type_tof_ctx_t *ctx, int n, cint32* p_buffer, int tx, int cfo,
	int adj, void* pparams, int32* p_ts, int32* p_seq_len, uint32* p_raw, uint8* ri_rr,
	const uint8 smooth_win_en);
static void phy_ac_nvram_proxd_read(phy_info_t *pi,  phy_ac_tof_info_t *tofi);
static int phy_ac_tof_reset(phy_type_tof_ctx_t *ctx);
static int phy_ac_tof_set_ri_rr_spb_only(phy_info_t *pi, const bool isInitiator,
		const bool macSuspend);
static void phy_ac_tof_fill_write_k_tof_seq_ucode_regs(phy_info_t *pi);
static void phy_ac_tof_fill_write_shm(phy_info_t *pi, uint16 *shm, uint8 core, uint16 mask);
/* DEBUG */
#ifdef TOF_DBG
static int phy_ac_tof_dbg(phy_type_tof_ctx_t *ctx, int arg);
#endif // endif
static void phy_ac_tof_init_gdmm_th(phy_type_tof_ctx_t *ctx, int32 *gdmm_th);
static void phy_ac_tof_init_gdv_th(phy_type_tof_ctx_t *ctx, uint32 *gdv_th);
static void phy_ac_tof_adjust_rx_tx_timing(phy_info_t *pi, bool tx);

/* register phy type specific implementation */
phy_ac_tof_info_t *
BCMATTACHFN(phy_ac_tof_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_tof_info_t *ti)
{
	phy_ac_tof_info_t *tofi;
	phy_type_tof_fns_t fns;

	uint32 tofi_size = sizeof(phy_ac_tof_info_t);
	uint32 rtx_size = FTM_TPK_RI_PHY_LEN_SECURE_2_0 * sizeof(uint8);
	uint32 rrx_size = FTM_TPK_RI_PHY_LEN_SECURE_2_0 * sizeof(uint8);
	uint32 ri_rr_size = FTM_RI_RR_BUF_LEN * sizeof(uint8);
	uint32 chan_size = 2 * (PHY_CORE_MAX + 1) * K_TOF_COLLECT_CHAN_SIZE * sizeof(int32);
	uint8 align_boundary = 4;
	/*
	Alloc size is 3 bytes more than total required size, this is get 4 byte alignment
	for chan
	*/
	uint32 alloc_size = tofi_size + rtx_size + rrx_size + ri_rr_size + chan_size;
	alloc_size = ALIGN_SIZE(alloc_size, align_boundary);

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((tofi = phy_malloc(pi, alloc_size)) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	tofi->pi = pi;
	tofi->aci = aci;
	tofi->ti = ti;
	tofi->alloc_size = alloc_size;

	tofi->rtx = ((uchar *)tofi) + tofi_size;
	tofi->rrx = ((uchar *)tofi->rtx) + rtx_size;
	tofi->ri_rr = ((uchar *)tofi->rrx) + rrx_size;
	tofi->chan = (int32 *)(((uint8 *)tofi->ri_rr) + ri_rr_size);
	/* Align chan to 4 byte address */
	tofi->chan = ALIGN_ADDR(tofi->chan, align_boundary);

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init_tof = phy_ac_tof_reset;
	fns.tof = phy_ac_tof;
	fns.cmd = phy_ac_tof_cmd;
	fns.info = phy_ac_tof_info;
	fns.kvalue = phy_ac_tof_kvalue;
	fns.seq_params = phy_ac_tof_seq_params;
	fns.seq_upd_dly = phy_ac_tof_seq_upd_dly;
	fns.chan_freq_response = phy_ac_tof_chan_freq_response;
	fns.chan_mag_sqr_impulse_response = phy_ac_chan_mag_sqr_impulse_response;
	fns.seq_ts = phy_ac_seq_ts;
	fns.set_ri_rr = phy_ac_tof_set_ri_rr;
	fns.seq_params_get_set_acphy = phy_ac_tof_seq_params_get_set;
	fns.setup_ack_core = phy_ac_tof_setup_ack_core;
	fns.core_select = phy_ac_tof_core_select;
	fns.init_gdmm_th = phy_ac_tof_init_gdmm_th;
	fns.init_gdv_th = phy_ac_tof_init_gdv_th;
	fns.calc_snr_bitflips = phy_ac_tof_calc_snr_bitflips;
	/* DEBUG */
#ifdef TOF_DBG
	fns.dbg = phy_ac_tof_dbg;
#endif // endif
	fns.ctx = tofi;
	phy_ac_nvram_proxd_read(pi, tofi);
	phy_tof_register_impl(ti, &fns);

	/* Register the maximum required scratch buffer size for phy_ac_tof_seq_params() The max
	 * required buffer size depends on tofi->tof_seq_log2_n.
	 */
#ifdef TOF_SEQ_20_IN_80MHz
	phy_cache_register_reuse_size(pi->cachei, sizeof(cint32) *
		(1 << K_TOF_SEQ_LOG2_N_80MHZ_20));
#else
	phy_cache_register_reuse_size(pi->cachei, sizeof(cint32) *
		(1 << K_TOF_SEQ_LOG2_N_80MHZ));
#endif /* TOF_SEQ_20_IN_80MHz */

	return tofi;

	/* error handling */
fail:
	if (tofi != NULL) {
		phy_mfree(pi, tofi, alloc_size);
	}
	return NULL;
}

void
BCMATTACHFN(phy_ac_tof_unregister_impl)(phy_ac_tof_info_t *tofi)
{
	phy_info_t *pi = tofi->pi;
	phy_tof_info_t *ti = tofi->ti;
	uint32 alloc_size = tofi->alloc_size;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_tof_unregister_impl(ti);

	phy_mfree(pi, tofi, alloc_size);
}

static int
WLBANDINITFN(phy_ac_tof_reset)(phy_type_tof_ctx_t *ctx)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;

	tofi->tof_setup_done = FALSE;
	tofi->tof_active = FALSE;
	tofi->tof_smth_forced = FALSE;
	tofi->tof_rfseq_bundle_offset = 0;
	tofi->tof_shm_ptr =
		(wlapi_bmac_read_shm(tofi->pi->sh->physhim, M_TOF_BLK_PTR(tofi->pi)) * 2);
	tofi->start_seq_time = TOF_DEFAULT_START_SEQ_TIME;
	tofi->delta_time_tx2rx = TOF_DEFAULT_DELTA_TIME_TX2RX;

	return BCME_OK;
}

/* Inter-module data api */
bool
phy_ac_tof_is_active(phy_ac_tof_info_t *tofi)
{
	return tofi->tof_active;
}

bool
phy_ac_tof_forced_smth(phy_ac_tof_info_t *tofi)
{
	return tofi->tof_smth_forced;
}

void
phy_ac_tof_tbl_offset(phy_ac_tof_info_t *tofi, uint32 id, uint32 offset)
{
	/* To keep track of first available offset usable by tof procs */
	if ((id == ACPHY_TBL_ID_RFSEQBUNDLE) && ((offset & 0xf) > tofi->tof_rfseq_bundle_offset))
		tofi->tof_rfseq_bundle_offset = (uint16)(offset & 0xf);
	if ((id == ACPHY_TBL_ID_RFSEQBUNDLE) || (id == ACPHY_TBL_ID_SAMPLEPLAY))
		tofi->tof_setup_done = FALSE;
}

#undef TOF_TEST_TONE

#ifdef WL_PROXD_SEQ

static void wlc_phy_tof_conj_arr(cint32* pIn, int len)
{
	int i = 0;
	cint32* pTmp = pIn;
	for (i = 0; i < len; i++) {
		pTmp->q = -pTmp->q;
		pTmp++;
	}
}

static void wlc_phy_tof_fliplr(cint32* pIn, int len, bool conj)
{
	cint32 tmp = {0, 0};
	cint32* pTmps = pIn + 1;
	cint32* pTmpe = pIn + len - 1;
	while (pTmps < pTmpe) {
		tmp.i = pTmps->i;
		tmp.q = pTmps->q;
		pTmps->i = pTmpe->i;
		pTmps->q = pTmpe->q;
		pTmpe->i = tmp.i;
		pTmpe->q = tmp.q;
		pTmps++;
		pTmpe--;
	}
	if (conj) {
		wlc_phy_tof_conj_arr(pIn, len);
	}
}

static void phy_ac_tof_sc(phy_info_t *pi, bool setup, int sc_start, int sc_stop, uint16 cfg)
{
	uint16 phy_ctl;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	if (D11REV_GE(pi->sh->corerev, 50)) {
		phy_ctl = R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
		phy_ctl &= ~((1<<0) | (1<<2));
		W_REG(pi->sh->osh, D11_SMP_CTRL(pi), phy_ctl);
	} else {
		phy_ctl = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi)) & ~((1<<4) | (1<<5));
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl);
	}

		MOD_PHYREG(pi, RxFeTesMmuxCtrl, samp_coll_core_sel, pi_ac->tofi->tof_core);
		MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 4);

	WRITE_PHYREG(pi, AdcDataCollect, 0);

	/* Disable MAC Clock gating for 43012 */

	if (setup) {
		acphy_set_sc_startptr(pi, (uint32)sc_start);
		acphy_set_sc_stopptr(pi, (uint32)sc_stop);
		if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
			uint32 pmu_chipctReg5 = si_pmu_chipcontrol(pi->sh->sih,
				PMU_CHIPCTL5, 0, 0) & 0xcfe0ffff;
			pmu_chipctReg5 |= (0x1036 << 16);
			si_pmu_chipcontrol(pi->sh->sih, PMU_CHIPCTL5, 0xFFFFFFFF, pmu_chipctReg5);
			pmu_chipctReg5 |= (1 << 19);
			si_pmu_chipcontrol(pi->sh->sih, PMU_CHIPCTL5, 0xFFFFFFFF, pmu_chipctReg5);
		}
	}
	if (cfg) {
		if (D11REV_GE(pi->sh->corerev, 50)) {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
			phy_ctl | (1 << 0) | (1 << 2));
		} else {
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi),
			phy_ctl | ((1 << 4) | (1 << 5)));
		}
		WRITE_PHYREG(pi, AdcDataCollect, cfg);
	}
}

static int phy_ac_tof_sc_read(phy_info_t *pi, bool iq, int n, cint32* pIn,
	int16 sc_ptr, int16 sc_base_ptr, int16* p_sc_start_ptr)
{
	uint32 dataL = 0, dataH, data;
	cint32* pEnd = pIn + n;
	int nbits = 0, n_out = 0;
	int32* pOut = (int32*)pIn;
	int16 sc_end_ptr;

	if (sc_ptr <= 0) {
		/* Offset from sc_base_ptr */
		sc_ptr = (-sc_ptr >> 2);
		*p_sc_start_ptr = (sc_ptr << 2);
		if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
			sc_ptr = 3 * sc_ptr;
		} else {
			sc_ptr = (sc_ptr << 2);
		}
		sc_ptr += sc_base_ptr;
	} else {
		/* Actual address */
		if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
			sc_ptr = (sc_ptr - sc_base_ptr) / 3;
			*p_sc_start_ptr = 4 * sc_ptr + sc_base_ptr;
			sc_ptr = 3 * sc_ptr + sc_base_ptr;
		} else {
			*p_sc_start_ptr = sc_ptr;
		}
	}
	sc_end_ptr = R_REG(pi->sh->osh, D11_SCP_CURPTR(pi));
	W_REG(pi->sh->osh, D11_XMT_TEMPLATE_RW_PTR(pi), ((uint32)sc_ptr << 2));
	while ((pIn < pEnd) && (sc_ptr < sc_end_ptr)) {
		if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
			dataH = dataL;
			dataL = (uint32)R_REG(pi->sh->osh, D11_XMT_TEMPLATE_RW_DATA(pi));
			nbits += 32;
			do {
				nbits -= 12;
				data = (dataL >> nbits);
				if (nbits) {
					data |= (dataH << (32 - nbits));
				}
				if (nbits & 4) {
					pIn->q = (int32)(data)& 0xfff;
				} else {
					pIn->i = (int32)(data)& 0xfff;
					pIn++;
					n_out++;
				}
			} while (nbits >= 12);
		} else {
			dataL = (uint32)R_REG(pi->sh->osh, D11_XMT_TEMPLATE_RW_DATA(pi));
			pIn->i = (int32)(dataL & 0xfff);
			pIn->q = (int32)((dataL >> 16) & 0xfff);
			pIn++;
			n_out++;
		}
		sc_ptr++;
	}
	if (iq) {
		int32 datum;

		n = 2 * n_out;
		while (n-- > 0) {
			datum = *pOut;
			if (datum > 2047)
				datum -= 4096;
			*pOut++ = datum;
		}
	}
	return n_out;
}

static int
wlc_tof_rfseq_event_offset(phy_info_t *pi, uint16 event, uint16* rfseq_events)
{
	int i;

	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, (uint32)0, 16, (void*)rfseq_events);

	for (i = 0; i < 16; i++) {
		if (rfseq_events[i] == event) {
			break;
		}
	}
	return i;
}

static void phy_ac_tof_cfo(phy_info_t *pi, int cfo, int n, cint32* pIn)
{
	/* CFO Correction Function Placeholder */
}

static void phy_ac_tof_cross_corr_sum(const cint32* In, const int32 len, cint32* accum)
{
	int i = 0;
	cint32 mult = { 0, 0 };
	for (i = 1; i < len; i++) {
		math_cmplx_mult_cint32_cfixed((In + i), (In + i - 1), 0, &mult, TRUE);
		math_cmplx_add_cint32(&mult, accum, accum);
	}
}
static void phy_ac_tof_group_delay_mv(const cint32* In, const uint16 len,
		const uint16* band_length, const uint16* sc_idx_arr, const int32 th,
		int32* gd_m, uint32* gd_v, int32* delta_th)
{
	cint32 mult = {0, 0};
	int32 theta[256] = {0}, max = INT32_MIN, min = INT32_MAX;
	uint32 run_sum = 0, div_mask = (1 << 30), theta_err = 0;
	uint16 band_offset = 0, i, j, idx, num_s = 0;
	*gd_m = 0;
	*gd_v = 0;
	*delta_th = 0;

	for (i = 0; i < len; i++) {
		num_s += band_length[i] - 1;
	}

	if (num_s != 0) {
		for (i = 0; i < len; i++) {
			const cint32* pTmp = (In + sc_idx_arr[band_offset]);
			for (j = 1; j < band_length[i]; j++) {
				math_cmplx_mult_cint32_cfixed((pTmp + j), (pTmp + j - 1),
					0, &mult, TRUE);
				idx = i*(band_length[i]-1) + j - 1;
				theta[idx] = -math_cordic_ptr(&mult);
				if (theta[idx] < min) {
					min = theta[idx];
				}
				if (theta[idx] > max) {
					max = theta[idx];
				}
				*gd_m += theta[idx];
			}
			band_offset += band_length[i];
		}
		(*delta_th) = th - max + min;
		(*gd_m) = (*gd_m) / num_s;
		for (i = 0; i < num_s; i++) {
			theta_err = theta[i] - (*gd_m);
			run_sum += theta_err*theta_err;
			if (run_sum & div_mask) {
				*gd_v += (run_sum/num_s);
				run_sum = 0;
			}
		}
		*gd_v += (run_sum/num_s);
	}
}

static int32 phy_ac_tof_group_delay(const cint32* In, const uint16 len, const uint16* band_length,
		const uint16* sc_idx_arr)
{
	uint16 band_offset = 0, i;
	cint32 accum = {0, 0};
	for (i = 0; i < len; i++) {
		uint16 idx = sc_idx_arr[band_offset];
		phy_ac_tof_cross_corr_sum((In+idx), band_length[i], &accum);
		band_offset += band_length[i];
	}
	return math_cordic_ptr(&accum);
}

static int32 phy_ac_tof_phase_est_theta(cint32* In, int32 gd_theta, int32 ph_offset,
	int32 len, bool gd_corr)
{
	int32 i = 0;
	cint32 mult = { 0, 0 }, accum = { 0, 0 };
	cint32 expj = { 0, 0 };
	cint32* tmp = &mult;
	for (i = 0; i < len; i++) {
		expj.i = math_cos_tbl(i*gd_theta + ph_offset);
		expj.q = math_sin_tbl(-(i*gd_theta + ph_offset));
		if (gd_corr) {
			tmp = In + i;
		}
		math_cmplx_mult_cint32_cfixed((In + i), &expj, K_TOF_TWDL_SFT, tmp, FALSE);
		math_cmplx_add_cint32(tmp, &accum, &accum);
	}
	return math_cordic_ptr(&accum);
}

static void phy_ac_tof_phase_corr_chan(uint16 len, const uint16* band_length,
	const uint16* sc_idx_arr, cint32* chan, cint32* mf_out)
{
	cint32 ph_corr = { 0, 0 };
	int32 i = 0, j = 0;
	uint16 band_offset;
	int32 gd_theta1, gd_theta2;
	int32 phi_off_c[len], phi_off_m[len];
	int32 ph_corr_th;

	gd_theta1 = phy_ac_tof_group_delay(chan, len, band_length, sc_idx_arr);
	band_offset = 0;
	for (i = 0; i < len; i++) {
		uint16 idx = sc_idx_arr[band_offset];
		phi_off_c[i] = phy_ac_tof_phase_est_theta((chan + idx), gd_theta1,
			(idx)*gd_theta1, band_length[i], TRUE);
		band_offset += band_length[i];
	}
	gd_theta2 = phy_ac_tof_group_delay(mf_out, len, band_length, sc_idx_arr);
	band_offset = 0;
	for (i = 0; i < len; i++) {
		uint16 idx = sc_idx_arr[band_offset];
		phi_off_m[i] = phy_ac_tof_phase_est_theta((mf_out + idx), gd_theta2,
			(idx)*gd_theta2, band_length[i], FALSE);
		band_offset += band_length[i];
	}

	/* Debug Prints */
	PHY_INFORM(("gd_theta1=[%d];\n", gd_theta1));
	for (i = 0; i < len; i++) {
		PHY_INFORM(("phi_off_c%d=[%d];\n", i, phi_off_c[i]));
	}
	PHY_INFORM(("gd_theta2=[%d];\n", gd_theta2));
	for (i = 0; i < len; i++) {
		PHY_INFORM(("phi_off_m%d=[%d];\n", i, phi_off_m[i]));
	}
	/* Debug Prints */

	band_offset = 0;
	for (i = 0; i < len; i++) {
		for (j = 0; j < band_length[i]; j++) {
			int idx = sc_idx_arr[band_offset + j];
			ph_corr_th = phi_off_m[i] - phi_off_c[i] + idx*gd_theta2;
			ph_corr.i = math_cos_tbl(ph_corr_th);
			ph_corr.q = math_sin_tbl(ph_corr_th);
			math_cmplx_mult_cint32_cfixed((chan + idx), &ph_corr, K_TOF_TWDL_SFT,
				(chan + idx), FALSE);
		}
		band_offset += band_length[i];
	}
}

static void phy_ac_tof_calc_snr_bpsk(const cint32* mf_out, const cint32* chan,
	const uint16* sc_idx_arr, const uint16 len, wl_proxd_bitflips_t *bit_flips,
	wl_proxd_snr_t *snr)
{
	uint32 sig_pwr = 0, chan_pwr = 0, noise_pwr = 0, final_sc, sig_sc, chan_sc;
	cint32 sig_val, chan_out;
	int32 i = 0, k = 0, noise_val;
	*bit_flips = 0;
	/*
	* We use matched filter output instead of rx'ed signal to calculate noise as
	* MF output is sign inversed in some bit positions where LTF is (-1).
	* If rx'ed sig is given by R = H*T + N, then N = R - H*T.
	* Instead we calculate N*T = R*T - H (T*T = 1, as T = +/- 1).
	* Here R*T is MF out and H is channel estimate.
	* N*T = +/- N depending on T = +/-1, we don't care about sign as we need to calculate
	* N*T*N*T = N*N which is always +ve.`
	*
	* All the tricks in this function can be used only because transmitted signal is BPSK
	* (+1, -1).
	*/
	math_cmplx_power_cint32_arr(mf_out, sc_idx_arr, len, &sig_pwr);
	sig_pwr = sqrt_int(sig_pwr);
	math_cmplx_power_cint32_arr(chan, sc_idx_arr, len, &chan_pwr);
	chan_pwr = sqrt_int(chan_pwr);
	/*
	* The scaling factors for normalization have been chosen this way to minimize
	* the impact of dividing by a large number and getting a 0, as data type is
	* integer type.
	*/
	if ((sig_pwr == 0) || (chan_pwr == 0)) {
		PHY_ERROR(("Error : chan_pwr = %d, sig_pwr = %d\n", chan_pwr, sig_pwr));
		ASSERT(0);
		*snr = 1;
		*bit_flips = len;
		return;
	}
	if (sig_pwr > chan_pwr) {
		sig_sc = (1 << K_TOF_SNR_FP_PREC) * 1;
		chan_sc = (sig_sc*sig_pwr) / chan_pwr;
		final_sc = sig_pwr;
	}
	else {
		chan_sc = (1 << K_TOF_SNR_FP_PREC) * 1;
		sig_sc = (chan_sc*chan_pwr) / sig_pwr;
		final_sc = chan_pwr;
	}

	/* Debug Prints */
	PHY_INFORM(("sig_sc = [%d];\n", sig_sc));
	PHY_INFORM(("chan_sc = [%d];\n", chan_sc));
	PHY_INFORM(("final_sc = [%d];\n", final_sc));
	PHY_INFORM(("noise_val = [ "));
	/* Debug Prints */

	cint32 sig_sc_cmplx = { 0, sig_sc };  /* {Imag, Real} */
	cint32 chan_sc_cmplx = { 0, chan_sc }; /* {Imag, Real} */

	for (k = 0; k < len; k++) {
		i = sc_idx_arr[k];
		math_cmplx_mult_cint32_cfixed((mf_out + i), &sig_sc_cmplx, 0, &sig_val, FALSE);
		math_cmplx_mult_cint32_cfixed((chan + i), &chan_sc_cmplx, 0, &chan_out, FALSE);
		/*
		* We are using BPSK for LTF (+1, -1) so we are concerned with in phase noise only.
		* noise_val being calculated here can be +/- of actual noise but we don't care as
		* we want only noise power.
		*/
		noise_val = sig_val.i - chan_out.i;

		/* Debug Prints */
		PHY_INFORM(("%d, ", noise_val));
		/* Debug Prints */

		noise_pwr += ((noise_val*noise_val) / (1 << (2 * K_TOF_SNR_FP_PREC)));
		/*
		* Calculate bit flips
		* Again instead of calculating R*H'/|H|^2 = T + N*H'/|H|^2
		* we calculate R*T*H'/|H|^2 = 1 + N*T*H'/|H|^2, so the slicer decision becomes
		* easier
		*/
		math_cmplx_mult_cint32_cfixed((mf_out + i), (chan + i), 0, &sig_val, TRUE);
		*bit_flips += ((sig_val.i < 0) ? 1 : 0);
	}

	/* Debug Prints */
	PHY_INFORM(("];\n"));
	PHY_INFORM(("noise_pwr=[%d];\n", noise_pwr));
	/* Debug Prints */

	*snr = (wl_proxd_snr_t)(noise_pwr == 0) ? 0xffff : (len*((final_sc*final_sc) / noise_pwr));
}

static int phy_ac_tof_demod_snr(void *In, void *chan_in, uint8 bw_factor,
	wl_proxd_bitflips_t *bit_flips, wl_proxd_snr_t *snr)
{
	cint32* pIn = (cint32*)In;
	cint32* chan, chan_out[K_TOF_HALF_CHAN_LENGTH_2X_OS_80M];
	const uint16 *sc_idx_arr = NULL, *band_length = NULL;
	uint16 num_bl = 0;
	uint32 i = 0, j = 0;

	switch (bw_factor) {
	case 2:
		chan = chan_out;
		j = 0;
		for (i = 0; i < (K_TOF_HALF_CHAN_LENGTH_2X_OS_80M >> 1); i++) {
			chan[j].i = (((cint32 *)chan_in) + 2 * i)->i;
			chan[j].q = (((cint32 *)chan_in) + 2 * i)->q;
			j++;
			if (j == ((K_TOF_HALF_CHAN_LENGTH_80M >> 1) + 1)) {
				j += (K_TOF_HALF_CHAN_LENGTH_2X_OS_80M >> 1);
			}
		}
		num_bl = K_TOF_NUM_LEGACY_BL_80M;
		band_length = band_length_80MHz;
		sc_idx_arr = nonzero_sc_idx_legacy_80MHZ;
		break;
	case 0:
	default:
		num_bl = K_TOF_NUM_LEGACY_BL_20M;
		band_length = band_length_20MHz;
		sc_idx_arr = nonzero_sc_idx_legacy_20MHZ;
		chan = (cint32*)chan_in;
	}

	phy_ac_tof_phase_corr_chan(num_bl, band_length, sc_idx_arr, chan, pIn);
	int NUM_LEGACY_NZ_SC = 0;
	for (i = 0; i < num_bl; i++) {
		NUM_LEGACY_NZ_SC += band_length[i];
	}
	phy_ac_tof_calc_snr_bpsk(pIn, chan, sc_idx_arr, NUM_LEGACY_NZ_SC, bit_flips, snr);
	return BCME_OK;
}

static int phy_ac_tof_mf(phy_info_t *pi, int n, cint32* pIn, bool seq, bool isTx,
	int a, int b, int cfo, int s1, int k2, int s2,
	uint16 bitflip_thresh, uint16 snr_thresh, const uint8 smooth_win_en)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)pi->u.pi_acphy->tofi;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	int i, k, ret_val = BCME_OK;
	cint32 *pTmp;
	int32 tmp;
	int nF = tofi->tof_seq_spb_len;
	const uint32* pF = (seq || isTx) ? tofi->tof_seq_spb_tx : tofi->tof_seq_spb_rx;
	const uint16 smooth_win[K_TOF_SEQ_FFT_20MHZ] = {
		65518, 65371, 65078, 64641, 64061, 63341, 62484, 61494,
		60377, 59136, 57777, 56307, 54733, 53061, 51298, 49453,
		47534, 45549, 43506, 41416, 39285, 37124, 34941, 32746,
		30547, 28353, 26173, 24015, 21887, 19798, 17755, 15764,
		15764, 17755, 19798, 21887, 24015, 26173, 28353, 30547,
		32746, 34941, 37124, 39285, 41416, 43506, 45549, 47534,
		49453, 51298, 53061, 54733, 56307, 57777, 59136, 60377,
		61494, 62484, 63341, 64061, 64641, 65078, 65371, 65518};
	int idx = 0;
	cint32 rot = {-1, 1};

	uint16 bw_factor = 0;
	const int num_max_cores = PHYCORENUM((pi)->pubpi->phy_corenum);

	if (CHSPEC_IS80(pi->radio_chanspec)) {
		bw_factor = 2;
	} else if (CHSPEC_IS20(pi->radio_chanspec)) {
		bw_factor = 0;
	} else {
		PHY_ERROR(("ERROR: Bandwidth other than 20 and 80 is not supported.\n"));
	}

	/* Debug Prints */
	if ((seq) && (isTx)) {
		PHY_INFORM(("SPB.... : \n"));
	} else if ((!seq) && (isTx)) {
		PHY_INFORM(("Tx... : \n"));
	} else {
		PHY_INFORM(("Matched filter... : \n"));
	}
	for (i = 0; i < 2 * nF; i++) {
		PHY_INFORM(("0x%08x \n", *(pF + i)));
	}
	/* Debug Prints */

	pTmp = pIn;
	for (i = 0; i < n; i++) {
		if (seq) {
			pTmp->q = 0;
			pTmp->i = (int32)s1;
		} else {
			pTmp->q = (pTmp->q + ((pTmp->i*(int32)a) >> 10)) << s1;
			pTmp->i = (pTmp->i + ((pTmp->i*(int32)b) >> 10)) << s1;
		}
		pTmp++;
	}

	if (!seq) {
		if (cfo) {
			phy_ac_tof_cfo(pi, cfo, n, pIn);
		}
#ifdef TOF_SEQ_20MHz_BW_512IFFT
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			pTmp = pIn;
			for (i = 0; i < K_TOF_SEQ_FFT_20MHZ; i++) {
				/* No down sampling for 43012 */
				pTmp[i] = pIn[i * 2];
			}

			memset(&pIn[K_TOF_SEQ_FFT_20MHZ], 0,
			(K_TOF_SEQ_FFT_20MHZ * sizeof(cint32)));
			n = K_TOF_SEQ_FFT_20MHZ;
		}
#endif /* TOF_SEQ_20MHz_BW_512IFFT */

		/* Undo IQ-SWAP for 43012 */
		wlapi_fft(pi->sh->physhim, n, (void*)pIn, (void*)pIn, 2);
#ifdef TOF_DBG_SEQ
		PRINT_SAMP("fft_out", seq, n, pIn);
#endif // endif
		if (pi_ac->tofi->flag_sec_2_0) {
			pTmp = pIn;
			for (i = 0; i < n; i++) {
				math_cmplx_mult_cint32_cfixed(pTmp, &rot,
				0, pTmp, TRUE);
				pTmp->q = -pTmp->q;
				pTmp++;
			}
		}
#ifdef TOF_DBG_SEQ
		PRINT_SAMP("derot_fft_out", seq, n, pIn);
#endif // endif
	}

	pTmp = pIn;
	for (k = 0; k < 2; k++) {
		for (i = 0; i < nF; i++) {
			uint32 f;
			int j;

			f = *pF++;
			for (j = 0; j < 32; j += 4) {
				if (f & K_TOF_FILT_NON_ZERO_MASK) {
					if (!(f & K_TOF_FILT_1_MASK)) {
						tmp = pTmp->q;
						if (pi_ac->tofi->flag_sec_2_0) {
							pTmp->q = pTmp->i;
							pTmp->i = -tmp;
						} else {
							pTmp->q = -pTmp->i;
							pTmp->i = tmp;
						}
					}
					if (f & K_TOF_FILT_NEG_MASK) {
						pTmp->i = -pTmp->i;
						pTmp->q = -pTmp->q;
					}
				} else {
					pTmp->i = 0;
					pTmp->q = 0;
				}
				if (pi_ac->tofi->flag_sec_2_0) {
					if (seq) {
						math_cmplx_mult_cint32_cfixed(pTmp,
						&rot, 0, pTmp, FALSE);
						pTmp->q = -pTmp->q;
					}
				}
				pTmp++;
				f = f >> 4;
			}
		}
		if (!k) {
			for (i = 0; i < (n - 2 * 8 * nF); i++) {
				pTmp->i = 0;
				pTmp->q = 0;
				pTmp++;
			}
		}
	}
#ifdef TOF_DBG_SEQ
	PRINT_SAMP("mfout0", seq, n, pIn);
#endif // endif

	if (!(seq || isTx)) {
		cint32* chan = (cint32*)tofi->chan;
		uint16	print_chan_len = (K_TOF_CHAN_LENGTH_20M << bw_factor);

		chan = (chan + num_max_cores*print_chan_len);
		ret_val = phy_ac_tof_demod_snr((void*)pIn, (void*)chan, bw_factor,
		&(tofi->bitflips), &(tofi->snr));

		PHY_ERROR(("SNR = %d, Bit Flips = %d\n", tofi->snr,  tofi->bitflips));
		if ((ret_val != BCME_OK) || (tofi->bitflips > bitflip_thresh) ||
		(tofi->snr < snr_thresh)) {
			if (ret_val != BCME_OK) {
				tofi->tof_phy_error |= WL_PROXD_PHY_ERR_LOW_CONFIDENCE;
			}
			if (tofi->bitflips > bitflip_thresh) {
				tofi->tof_phy_error |= WL_PROXD_PHY_ERR_BITFLIP;
			}
			if (tofi->snr < snr_thresh) {
				tofi->tof_phy_error |= WL_PROXD_PHY_ERR_SNR;
			}
			PHY_ERROR(("SNR_Thres = %d,  BF_Thres = %d\n", snr_thresh, bitflip_thresh));
		}
	}

	if ((!seq) && (smooth_win_en == 1) && CHSPEC_IS20(pi->radio_chanspec)) {
		pTmp = pIn;
		for (k = 0; k < 2; k++) {
			for (i = 0; i < 8*nF; i++) {
				idx = 8*nF*k + i;
				pTmp->i *= smooth_win[idx];
				pTmp->q *= smooth_win[idx];
				pTmp->i >>= 16;
				pTmp->q >>= 16;
				pTmp++;
			}
			if (!k) {
				pTmp += (n - 2*8*nF);
			}
		}
	}

#ifdef TOF_SEQ_20MHz_BW_512IFFT
	if (!seq) {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			pTmp = pIn;
			bcopy(pTmp, pIn, (2 * 32 * sizeof(uint32)));
			bcopy((pTmp + 32), (pIn + 480), (2 * 32 * sizeof(uint32)));
			memset(&pIn[32], 0, (448 * sizeof(cint32)));
			n = K_TOF_SEQ_IFFT_20MHZ;
		}
	}
#endif /* TOF_SEQ_20MHz_BW_512IFFT */

	wlapi_fft(pi->sh->physhim, n, (void*)pIn, (void*)pIn, 2);
	if (pi_ac->tofi->flag_sec_2_0) {
		if (!seq) {
			wlc_phy_tof_fliplr(pIn, n, FALSE);
		}
	}
	if (s2) {
		int32 *pTmpIQ, m2 = (int32)(1 << (s2 - 1));
		pTmpIQ = (int32*)pIn;
		for (i = 0; i < 2 * n; i++) {
			tmp = ((k2*(*pTmpIQ) + m2) >> s2);
			*pTmpIQ++ = tmp;
		}
	}
	return ret_val;
}

static void
wlc_tof_seq_write_shm_acphy(phy_info_t *pi, int len, uint16 offset, uint16* p)
{
	uint16 p_shm = pi->u.pi_acphy->tofi->tof_shm_ptr;

	while (len-- > 0) {
		wlapi_bmac_write_shm(pi->sh->physhim, (p_shm + offset), *p);
		p++;
		offset += 2;
	}
}

static void
phy_ac_tof_adjust_rx_tx_timing(phy_info_t *pi, bool tx)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)pi->u.pi_acphy->tofi;

	int16 trig_dly_offset;
	uint16 ofdm_mult, ofdm_mult_remainder, ofdm_sym_len_10us;

	ofdm_sym_len_10us = (((1 << K_TOF_SEQ_LOG2_N_20MHZ) *
		TOF_US_TO_TEN_US)/tofi->tof_sc_FS);
	trig_dly_offset = (tofi->delta_time_tx2rx -
		TOF_DEFAULT_DELTA_TIME_TX2RX);
	ofdm_mult = (int)((tofi->delta_time_tx2rx * TOF_US_TO_TEN_US)/
		ofdm_sym_len_10us);
	ofdm_mult_remainder = (int) ((tofi->delta_time_tx2rx *
		TOF_US_TO_TEN_US) - (((tofi->delta_time_tx2rx *
		TOF_US_TO_TEN_US)/ofdm_sym_len_10us)*ofdm_sym_len_10us));
#ifdef TOF_DBG_SEQ
	PHY_ERROR(("%s: ofdm_mult = %d, ofdm_mult_remainder = %d\n",
		__FUNCTION__, ofdm_mult, ofdm_mult_remainder));
#endif // endif
	if (ofdm_mult_remainder >= (ofdm_sym_len_10us/2)) {
		ofdm_mult++;
	}
	if (tx) {
		tofi->tof_ucode_dlys_us[1][2] += trig_dly_offset;
		tofi->tof_ucode_dlys_us[1][4] =
			(tofi->tof_ucode_dlys_us[1][3] +
			(1 << K_TOF_SEQ_LOG2_N_20MHZ) * ofdm_mult);
#ifdef TOF_DBG_SEQ
		PHY_ERROR(("%s: trig_dly_offset = %d,",
			__FUNCTION__, trig_dly_offset));
		PHY_ERROR(("Adj_Delay = %d\n",
			tofi->tof_ucode_dlys_us[1][2]));
#endif // endif
	} else {
		tofi->tof_ucode_dlys_us[0][1] += trig_dly_offset;
		tofi->tof_ucode_dlys_us[0][3] =
			(tofi->tof_ucode_dlys_us[0][4] +
			(1 << K_TOF_SEQ_LOG2_N_20MHZ) * ofdm_mult);
#ifdef TOF_DBG_SEQ
		PHY_ERROR(("%s: trig_dly_offset = %d,",
			__FUNCTION__, trig_dly_offset));
		PHY_ERROR(("Adj_Delay = %d\n",
			tofi->tof_ucode_dlys_us[0][1]));
#endif // endif
	}
}

static void
phy_ac_tof_trig_dly_setup_acphy(phy_info_t *pi, bool enter, bool tx)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint offset;
	uint16 shm_val, tof_trig_delay;
	int16 trig_dly_offset;

	if (pi_ac->tofi->flag_sec_2_0) {
		PHY_ERROR(("%s: SECURE_RANGING 2.0: Enter = %d\n",
			__FUNCTION__, enter));
		if ((ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) ||
			ACMINORREV_4(pi))) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			if (enter) {
				trig_dly_offset = ((pi_ac->tofi->start_seq_time -
					TOF_DEFAULT_START_SEQ_TIME) * TOF_GPT_TIMER_RES_MULT);
				offset = pi_ac->tofi->tof_shm_ptr + M_TOF_UCODE_SET;
				shm_val = wlapi_bmac_read_shm(pi->sh->physhim, offset);
				if (tx) {
					shm_val |= K_TOF_IS_TARGET_FLAG;
					tof_trig_delay = K_TOF_TARGET_TRIG_DLY;
				} else {
					tof_trig_delay = K_TOF_INI_TRIG_DLY;
					shm_val &= ~K_TOF_IS_TARGET_FLAG;
				}
				tof_trig_delay += trig_dly_offset;
				wlapi_bmac_write_shm(pi->sh->physhim, offset, shm_val);
				offset = pi_ac->tofi->tof_shm_ptr + M_TOF_TRIG_DLY;
				PHY_ERROR(("%s: tof_trig_delay = %d\n",
					__FUNCTION__, tof_trig_delay));
				wlapi_bmac_write_shm(pi->sh->physhim, offset, tof_trig_delay);
				phy_ac_tof_adjust_rx_tx_timing(pi, tx);
			} else {
				offset = pi_ac->tofi->tof_shm_ptr + M_TOF_TRIG_DLY;
				wlapi_bmac_write_shm(pi->sh->physhim, offset, 0);
			}
		}
	}
}

static void
phy_ac_tof_seq_upd_dly(phy_type_tof_ctx_t *ctx, bool tx, uint8 core, bool mac_suspend)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	uint8 stall_val;
	int i;
	uint16 *pSrc;
	uint16 shm[18];
	uint16 wrds_per_us, rfseq_trigger;
	bool  suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

	if (mac_suspend) {
		if (!suspend) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);
	}

	/* Setup delays and triggers */
	if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
	  wrds_per_us = ((3*tofi->tof_sc_FS) >> 2);
	} else {
	  wrds_per_us = tofi->tof_sc_FS;
	}

	pSrc = shm;
	rfseq_trigger = READ_PHYREG(pi, RfseqTrigger) &
		ACPHY_RfseqTrigger_en_pkt_proc_dcc_ctrl_MASK(0);
	for (i = 0; i < 3; i++) {
		*pSrc++ = tofi->tof_ucode_dlys_us[(tx ? 1 : 0)][i] * wrds_per_us;
		if (i ^ tx)
			*pSrc = rfseq_trigger | ACPHY_RfseqTrigger_ocl_shut_off_MASK(0);
		else
			*pSrc = rfseq_trigger | ACPHY_RfseqTrigger_ocl_reset2rx_MASK(0);
		pSrc++;
	}
	shm[K_TOF_SEQ_SHM_DLY_LEN - 1] = rfseq_trigger; /* Restore value */
	shm[K_TOF_SEQ_SHM_DLY_LEN] = ACPHY_PhyStatsGainInfo0(0) + 0x200 * core;

	wlc_tof_seq_write_shm_acphy(pi,
		(K_TOF_SEQ_SHM_DLY_LEN + 1),
		K_TOF_SEQ_SHM_DLY_OFFSET,
		shm);
	if (mac_suspend) {
		ACPHY_ENABLE_STALL(pi, stall_val);
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
		if (!suspend) {
			wlapi_enable_mac(pi->sh->physhim);
		}
	}
}

static void phy_ac_tof_fill_write_k_tof_seq_ucode_regs(phy_info_t *pi)
{
	int i = 0;
	uint16 k_tof_seq_ucode_regs[K_TOF_SEQ_SHM_SETUP_REGS_LEN] = { 0, };

	k_tof_seq_ucode_regs[i++] = ACPHY_RxControl(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_TableID(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_TableOffset(0);
	k_tof_seq_ucode_regs[i++] = (ACPHY_TableDataWide(0) | (7 << 12));
	k_tof_seq_ucode_regs[i++] = ACPHY_TableID(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_TableOffset(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_TableDataLo(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_TableOffset(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_TableDataLo(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_RfseqMode(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_sampleCmd(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_RfseqMode(0);
	if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		k_tof_seq_ucode_regs[i++] = ACPHY_AdcDataCollect(0);
	} else {
		k_tof_seq_ucode_regs[i++] = ACPHY_SlnaControl(pi->pubpi->phy_rev);
	}
	k_tof_seq_ucode_regs[i++] = ACPHY_AdcDataCollect(0);
	k_tof_seq_ucode_regs[i++] = ACPHY_RxControl(0);

	wlc_tof_seq_write_shm_acphy(pi,
	                            K_TOF_SEQ_SHM_SETUP_REGS_LEN,
	                            K_TOF_SEQ_SHM_SETUP_REGS_OFFSET,
	                            (uint16*)k_tof_seq_ucode_regs);

}

static void phy_ac_tof_fill_write_shm(phy_info_t *pi, uint16 *shm, uint8 core, uint16 mask)
{
	uint16 rfseq_mode, rfseq_offset, rx_ctrl, tof_rfseq_event;

	/* Set rx gain during loopback -- cant use rf bundles due to hw bug in 4350 */
	if (TINY_RADIO(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			shm[0] = K_TOF_SEQ_RX_LOOPBACK_GAIN_TINY_2G;
		} else {
			shm[0] = K_TOF_SEQ_RX_LOOPBACK_GAIN_TINY;
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
				shm[0] = K_TOF_SEQ_RX_LOOPBACK_GAIN_2G_4360;
			} else if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
				if (IS_4364_3x3(pi)) {
					shm[0] = K_TOF_SEQ_RX_LOOPBACK_GAIN_2G_4364;
				} else {
					shm[0] = K_TOF_SEQ_RX_LOOPBACK_GAIN_2G_43602;
				}
			} else {
				shm[0] = K_TOF_SEQ_RX_LOOPBACK_GAIN_2G;
			}
		} else {
			shm[0] = K_TOF_SEQ_RX_LOOPBACK_GAIN;
		}
	}
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
		(K_TOF_SEQ_RFSEQ_GAIN_BASE + core * 0x10
		+ K_TOF_SEQ_RFSEQ_LOOPBACK_GAIN_OFFSET),
		16, (void*)&shm[0]);

	/* Setup shm which tells ucode sequence of phy reg writes */
	/* before/after triggering sequence */
	rx_ctrl = READ_PHYREG(pi, RxControl);
	rfseq_mode = (READ_PHYREG(pi, RfseqMode) &
		~(ACPHY_RfseqMode_CoreActv_override_MASK(0) |
		ACPHY_RfseqMode_Trigger_override_MASK(0)));
	rfseq_offset = wlc_tof_rfseq_event_offset(pi, K_TOF_RFSEQ_TX_GAIN_EVENT, shm);
	rfseq_offset += 1;
	tof_rfseq_event = shm[rfseq_offset];

	phy_ac_tof_fill_write_k_tof_seq_ucode_regs(pi);

	bzero((void *)shm, sizeof(uint16)* K_TOF_SHM_ARR_LENGTH);
	shm[0] = rx_ctrl | ACPHY_RxControl_dbgpktprocReset_MASK(0); /* first setup */
	shm[1] = ACPHY_TBL_ID_RFSEQBUNDLE;
	if (TINY_RADIO(pi))
	  shm[2] = K_TOF_SEQ_TINY_RX_FEM_GAIN_OFFSET;
	else
	  shm[2] = K_TOF_SEQ_RX_FEM_GAIN_OFFSET;
	if (CHSPEC_IS2G(pi->radio_chanspec))
		shm[3] = k_tof_seq_fem_gains_2g[0] | mask;
	else
		shm[3] = k_tof_seq_fem_gains[0] | mask;
	shm[6] = ACPHY_TBL_ID_RFSEQ; /* first restore */
	shm[7] = K_TOF_SEQ_RFSEQ_GAIN_BASE + core*0x10 + K_TOF_SEQ_RFSEQ_RX_GAIN_OFFSET;
#ifdef TOF_DBG_SEQ
	if (TINY_RADIO(pi))
	  shm[8] = K_TOF_SEQ_RX_GAIN_TINY;
	else
	  shm[8] = K_TOF_SEQ_RX_GAIN;
#endif // endif
	shm[9] = rfseq_offset;
	shm[10] = K_TOF_RFSEQ_END_EVENT;
	shm[11] = (rfseq_mode | ACPHY_RfseqMode_CoreActv_override_MASK(0));
	shm[12] = ACPHY_sampleCmd_start_MASK(0);
	shm[13] = (rfseq_mode |
	           ACPHY_RfseqMode_Trigger_override_MASK(0) |
	           ACPHY_RfseqMode_CoreActv_override_MASK(0));
	if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		shm[14] = ACPHY_AdcDataCollect_adcDataCollectEn_MASK(0);
	}
	else {
		PHY_ERROR(("Slna Core = 0x%x\n",
			((~core) & 3) << ACPHY_SlnaControl_SlnaCore_SHIFT(0)));
		shm[14] = ((~core) & 3) << ACPHY_SlnaControl_SlnaCore_SHIFT(0);
	}
	shm[15] = ACPHY_AdcDataCollect_adcDataCollectEn_MASK(0); /* last setup */
	wlc_tof_seq_write_shm_acphy(pi,
	                            K_TOF_SEQ_SHM_SETUP_VALS_LEN,
	                            K_TOF_SEQ_SHM_SETUP_VALS_OFFSET,
	                            shm);
	shm[10] = tof_rfseq_event;
	shm[11] = rfseq_mode;
	shm[12] = ACPHY_sampleCmd_stop_MASK(0);
	shm[13] = rfseq_mode;
	if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		shm[14] = 0;
	}
	else {
		shm[14] = READ_PHYREG(pi, SlnaControl);
	}
	shm[15] = 0;
	shm[16] = rx_ctrl; /* last restore */
	wlc_tof_seq_write_shm_acphy(pi,
	                            K_TOF_SET_SHM_RESTR_VALS_LEN,
	                            K_TOF_SET_SHM_RESTR_VALS_OFFSET,
	                            &shm[9]);
}

static void
phy_ac_tof_setup_rf_bundle(phy_info_t *pi)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)pi->u.pi_acphy->tofi;
	uint32 read_val[2];
	uint8 afe_iqadc_flash_only, afe_iqadc_reset_ov_det, afe_iqadc_clamp_en;
	uint8 afe_iqadc_rx_div4_en, afe_iqadc_adc_bias, afe_ctrl_flash17lvl;
	uint8 afe_iqadc_flashhspd, afe_iqadc_pwrup, afe_iqadc_mode;

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1,
				TOF_RFSEQEXT_ADC_20MHz_OFFS, TOF_RFSEQEXT_SIZE, &read_val);
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1,
				TOF_RFSEQEXT_ADC_40MHz_OFFS, TOF_RFSEQEXT_SIZE, &read_val);
	} else {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1,
				TOF_RFSEQEXT_ADC_80MHz_OFFS, TOF_RFSEQEXT_SIZE, &read_val);
	}
	afe_iqadc_flash_only = ((read_val[0] >> TOF_RFSEQ_AFE_IQADC_FLASH_ONLY_SHIFT)
			& TOF_RFSEQ_AFE_IQADC_FLASH_ONLY_SZ);
	afe_iqadc_reset_ov_det = ((read_val[0] >> TOF_RFSEQ_AFE_RESET_OV_DET_SHIFT)
			& TOF_RFSEQ_AFE_RESET_OV_DET_SZ);
	afe_iqadc_clamp_en = ((read_val[0] >> TOF_RFSEQ_AFE_IQADC_CLAMP_EN_SHIFT)
			& TOF_RFSEQ_AFE_IQADC_CLAMP_EN_SZ);
	afe_iqadc_rx_div4_en = ((read_val[0] >> TOF_RFSEQ_AFE_IQADC_RX_DIV4_EN_SHIFT)
			& TOF_RFSEQ_AFE_IQADC_RX_DIV4_EN_SZ);
	afe_iqadc_adc_bias = ((read_val[0] >> TOF_RFSEQ_AFE_IQADC_ADC_BIAS_SHIFT)
			& TOF_RFSEQ_AFE_IQADC_ADC_BIAS_SZ);
	afe_ctrl_flash17lvl = ((read_val[0] >> TOF_RFSEQ_AFE_CTRL_FLASH17LVL_SHIFT)
			& TOF_RFSEQ_AFE_CTRL_FLASH17LVL_SZ);
	afe_iqadc_flashhspd = ((read_val[0] >> TOF_RFSEQ_AFE_IQADC_FLASHHSPD_SHIFT)
			& TOF_RFSEQ_AFE_IQADC_FLASHHSPD_SZ);
	afe_iqadc_pwrup = ((read_val[0] >> TOF_RFSEQ_AFE_IQADC_PWRUP_SHIFT)
			& TOF_RFSEQ_AFE_IQADC_PWRUP_SZ);
	afe_iqadc_mode = ((read_val[0]) & TOF_RFSEQ_AFE_IQADC_MODE_SZ);

	PHY_INFORM(("%s: Readval = 0x%x\n", __FUNCTION__, read_val[0]));
	PHY_INFORM(("RFSeq offset 38 Before = 0x%04x%04x%04x\n",
			tofi->bundle_offs_38[2], tofi->bundle_offs_38[1], tofi->bundle_offs_38[0]));
	PHY_INFORM(("RFSeq offset 39 Before = 0x%04x%04x%04x\n",
			tofi->bundle_offs_39[2], tofi->bundle_offs_39[1], tofi->bundle_offs_39[0]));
	PHY_INFORM(("afe_iqadc_flash_only = 0x%x\n", afe_iqadc_flash_only));
	PHY_INFORM(("afe_iqadc_reset_ov_det = 0x%x\n", afe_iqadc_reset_ov_det));
	PHY_INFORM(("afe_iqadc_clamp_en = 0x%x\n", afe_iqadc_clamp_en));
	PHY_INFORM(("afe_iqadc_rx_div4_en = 0x%x\n", afe_iqadc_rx_div4_en));
	PHY_INFORM(("afe_iqadc_adc_bias = 0x%x\n", afe_iqadc_adc_bias));
	PHY_INFORM(("afe_ctrl_flash17lvl = 0x%x\n", afe_ctrl_flash17lvl));
	PHY_INFORM(("afe_iqadc_flashhspd = 0x%x\n", afe_iqadc_flashhspd));
	PHY_INFORM(("afe_iqadc_pwrup = 0x%x\n", afe_iqadc_pwrup));
	PHY_INFORM(("afe_iqadc_mode = 0x%x\n", afe_iqadc_mode));

	tofi->bundle_offs_38[0] &= ~TOF_RFBNDL_ADC_MASK_LB;
	tofi->bundle_offs_38[1] &= ~TOF_RFBNDL_ADC_MASK_HB;

	tofi->bundle_offs_39[0] &= ~TOF_RFBNDL_ADC_MASK_LB;
	tofi->bundle_offs_39[1] &= ~TOF_RFBNDL_ADC_MASK_HB;

	PHY_INFORM(("RFSeq offset 38 After Mask = 0x%04x%04x%04x\n", tofi->bundle_offs_38[2],
			tofi->bundle_offs_38[1], tofi->bundle_offs_38[0]));
	PHY_INFORM(("RFSeq offset 39 After Mask = 0x%04x%04x%04x\n", tofi->bundle_offs_39[2],
			tofi->bundle_offs_39[1], tofi->bundle_offs_39[0]));

	tofi->bundle_offs_38[0] |=
		((afe_iqadc_reset_ov_det << TOF_RFBNDL_AFE_RESET_OV_DET_SHIFT) |
		(afe_iqadc_clamp_en << TOF_RFBNDL_AFE_IQADC_CLAMP_EN_SHIFT) |
		(afe_iqadc_adc_bias << TOF_RFBNDL_AFE_IQADC_ADC_BIAS_SHIFT) |
		(afe_iqadc_pwrup << TOF_RFBNDL_AFE_IQADC_PWRUP_SHIFT) |
		((afe_iqadc_mode & TOF_RFBNDL_AFE_IQADC_MODE_LB_MASK) <<
		 TOF_RFBNDL_AFE_IQADC_MODE_LB_SHIFT));

	tofi->bundle_offs_38[1] |=
		((afe_iqadc_rx_div4_en << TOF_RFBNDL_AFE_IQADC_RX_DIV4_EN_SHIFT) |
		(afe_ctrl_flash17lvl << TOF_RFBNDL_AFE_CTRL_FLASH17LVL_SHIFT) |
		(afe_iqadc_flashhspd << TOF_RFBNDL_AFE_IQADC_FLASHHSPD_SHIFT) |
		(((afe_iqadc_mode &  TOF_RFBNDL_AFE_IQADC_MODE_HB_MASK) >>
		TOF_RFBNDL_AFE_IQADC_MODE_HB_SHIFT)) |
		(afe_iqadc_flash_only << TOF_RFBNDL_AFE_IQADC_FLASH_ONLY_SHIFT));

	tofi->bundle_offs_39[0] |=
		((afe_iqadc_reset_ov_det << TOF_RFBNDL_AFE_RESET_OV_DET_SHIFT) |
		(afe_iqadc_clamp_en << TOF_RFBNDL_AFE_IQADC_CLAMP_EN_SHIFT) |
		(afe_iqadc_adc_bias << TOF_RFBNDL_AFE_IQADC_ADC_BIAS_SHIFT) |
		(afe_iqadc_pwrup << TOF_RFBNDL_AFE_IQADC_PWRUP_SHIFT) |
		((afe_iqadc_mode & TOF_RFBNDL_AFE_IQADC_MODE_LB_MASK) <<
		 TOF_RFBNDL_AFE_IQADC_MODE_LB_SHIFT));

	tofi->bundle_offs_39[1] |=
		((afe_iqadc_rx_div4_en << TOF_RFBNDL_AFE_IQADC_RX_DIV4_EN_SHIFT) |
		(afe_ctrl_flash17lvl << TOF_RFBNDL_AFE_CTRL_FLASH17LVL_SHIFT) |
		(afe_iqadc_flashhspd << TOF_RFBNDL_AFE_IQADC_FLASHHSPD_SHIFT) |
		(((afe_iqadc_mode &  TOF_RFBNDL_AFE_IQADC_MODE_HB_MASK) >>
		TOF_RFBNDL_AFE_IQADC_MODE_HB_SHIFT)) |
		(afe_iqadc_flash_only << TOF_RFBNDL_AFE_IQADC_FLASH_ONLY_SHIFT));

	PHY_INFORM(("RFSeq offset 38 After = 0x%04x%04x%04x\n", tofi->bundle_offs_38[2],
			tofi->bundle_offs_38[1], tofi->bundle_offs_38[0]));
	PHY_INFORM(("RFSeq offset 39 After = 0x%04x%04x%04x\n", tofi->bundle_offs_39[2],
			tofi->bundle_offs_39[1], tofi->bundle_offs_39[0]));

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, TOF_RFBNDL_ADC_OFFSET_38,
			TOF_RFSEQ_BUNDLE_SIZE, (void *)(tofi->bundle_offs_38));

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, TOF_RFBNDL_ADC_OFFSET_39,
			TOF_RFSEQ_BUNDLE_SIZE, (void *)(tofi->bundle_offs_39));
}

static int
phy_ac_tof_setup_core_params(phy_info_t *pi, uint8 core, bool assign_buffer)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)pi->u.pi_acphy->tofi;
	const uint16 *pSrc;
	const uint16 *pEnd;
	uint16 shm[K_TOF_SHM_ARR_LENGTH], mask;
	int i = 0;

	tofi->tof_core = core;
	mask = (1 << tofi->tof_core);

	if (phy_ac_tof_seq_params(tofi, assign_buffer) != BCME_OK)
		return BCME_ERROR;

	if (TINY_RADIO(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pSrc = k_tof_seq_tiny_tbls_2G;
			pEnd = pSrc + ARRAYSIZE(k_tof_seq_tiny_tbls_2G);
		} else {
			pSrc = k_tof_seq_tiny_tbls;
			pEnd = pSrc + ARRAYSIZE(k_tof_seq_tiny_tbls);
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pSrc = k_tof_seq_tbls_2G;
			pEnd = pSrc + ARRAYSIZE(k_tof_seq_tbls_2G);
		} else {
			pSrc = k_tof_seq_tbls;
			pEnd = pSrc + ARRAYSIZE(k_tof_seq_tbls);
		}
	}
	while (pSrc != pEnd) {
		uint32 id, width, len, tbl_len, offset;
		const uint16 *pTblData;

		id = (uint32)*pSrc++;
		offset = (uint32)*pSrc++;
		len = (uint32)*pSrc++;
		if (id == ACPHY_TBL_ID_RFSEQBUNDLE) {
			width = 48;
			tbl_len = len / 3;
			for (i = 0; i < len; i++) {
				shm[i] = pSrc[i];
				if ((offset >= 0x10) && ((i % 3) == 0)) {
					shm[i] = (shm[i] & ~7) | mask;
				}
			}
			pTblData = shm;
		} else {
			width = 16;
			tbl_len = len;
			pTblData = pSrc;
		}
		if (offset == 0x38) {
			tofi->bundle_offs_38[0] = pTblData[0];
			tofi->bundle_offs_38[1] = pTblData[1];
			tofi->bundle_offs_38[2] = pTblData[2];
			tofi->bundle_offs_39[0] = pTblData[3];
			tofi->bundle_offs_39[1] = pTblData[4];
			tofi->bundle_offs_39[2] = pTblData[5];
			PHY_INFORM(("len = %d RFSeq offset 0x%x Init = 0x%04x%04x%04x\n",
				tbl_len, offset, pTblData[2],
				pTblData[1], pTblData[0]));
		}
		wlc_phy_table_write_acphy(pi, id, tbl_len, offset, width, pTblData);
		pSrc += len;
	}
	phy_ac_tof_fill_write_shm(pi, shm, core, mask);
	phy_ac_tof_setup_rf_bundle(pi);

	return BCME_OK;
}

static int
phy_ac_tof_seq_setup(phy_info_t *pi, bool enter, bool tx, uint8 core, bool isInitiator,
		bool loadSPB, bool firstCall)
{
	/*
	 * 1) When firstCall = FALSE, enter and tx are warning.
	 * 2) isInitiator value is warning if TOF_TEST_TONE is not defined or if loadSPB is FALSE.
	*/

	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)pi->u.pi_acphy->tofi;
	uint8 stall_val;
	uint16 tof_rfseq_bundle_offset;
	int ret = BCME_OK;
	bool  suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	uint16 tof_seq_n;

	if ((!enter) && firstCall) {
		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			// turn off sample capture clock
			(void) si_pmu_pllcontrol(pi->sh->sih, PMU1_PLL0_PLLCTL0,
			                         0x00000020, tofi->tof_pllctrl);

			// Now toggle pllctlupdate so the pll sees the new values
			si_pmu_pllupd(pi->sh->sih);
		}
		return BCME_OK;
	}

	if (((tofi->tof_rfseq_bundle_offset >= K_TOF_RFSEQ_BUNDLE_BASE) && !TINY_RADIO(pi)) ||
		((tofi->tof_rfseq_bundle_offset >= K_TOF_RFSEQ_TINY_BUNDLE_BASE) &&
		TINY_RADIO(pi)) || (READ_PHYREGFLD(pi, OCLControl1, ocl_mode_enable))) {
		return BCME_ERROR;
	}

	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		tofi->tof_pllctrl = si_pmu_pllcontrol(pi->sh->sih, PMU1_PLL0_PLLCTL0, 0x0, 0x0);
		(void) si_pmu_pllcontrol(pi->sh->sih, PMU1_PLL0_PLLCTL0, 0x00000020, 0x00000000);
		si_pmu_pllupd(pi->sh->sih);
	}

	if (!enter) {
		phy_ac_tof_trig_dly_setup_acphy(pi, enter, tx);
	}

	if (firstCall) {
		if (phy_ac_tof_seq_params(tofi, TRUE) != BCME_OK)
			return BCME_ERROR;

		tof_seq_n = (1 << tofi->tof_seq_log2_n);
		WRITE_PHYREG(pi, sampleLoopCount, 0xffff);
		WRITE_PHYREG(pi, sampleDepthCount, (tof_seq_n - 1));

		if ((tx != tofi->tof_tx) && tofi->tof_setup_done) {
			tofi->tof_setup_done = FALSE;
		}

		if (tofi->tof_setup_done) {
			return BCME_OK;
		}

		tofi->tof_tx = tx;
		tofi->tof_phy_error = 0;
		tofi->isInvalid = 0;
	}

	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	/* Setup rfcontrol sequence for tx_on / tx_off events */
	tof_rfseq_bundle_offset = tofi->tof_rfseq_bundle_offset;
	phy_ac_tof_setup_core_params(pi, core, TRUE);
	phy_ac_tof_trig_dly_setup_acphy(pi, enter, tx);
	phy_ac_tof_seq_upd_dly(tofi, tx, core, FALSE);
	tofi->tof_rfseq_bundle_offset = tof_rfseq_bundle_offset;

	if (loadSPB) {
		ret = phy_ac_tof_set_ri_rr_spb_only(pi, isInitiator, FALSE);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

	/* tofi->tof_setup_done = TRUE; */
	return ret;
}

#if defined(TOF_CALC_SYM_BNDRY_OFFSET)
static int
wlc_phy_tof_calc_offset_from_sym_bndry(phy_info_t *pi, int16 sliding_ptr_offset,
	int16* ofdm_sym_bndry, int32* offset_from_bndry, int* bndry_idx)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	int i, offset, diff, sym_len;

	sym_len = (1 << pi_ac->tof_seq_log2_n);

	offset = (int)(sliding_ptr_offset-ofdm_sym_bndry[K_TOF_SYM_ARR_LEN-1]);

	for (i = 0; i < K_TOF_SYM_ARR_LEN; i++) {
		diff = (int)(sliding_ptr_offset-ofdm_sym_bndry[i]);
		if ((diff >= 0) && (diff < offset)) {
			offset = diff;
			*bndry_idx = i;
		}
	}
	if (offset > sym_len) {
		PHY_ERROR(("%s: ERROR: Couldn't Find First Symbol Boundary\n", __FUNCTION__));
		offset = 0;
		*offset_from_bndry = offset;
		*bndry_idx = 0;
		return BCME_ERROR;
	}

	*offset_from_bndry = offset;
	return BCME_OK;

}
#endif /* TOF_CALC_SYM_BNDRY_OFFSET */

static int
phy_ac_tof_chk_rx_window_off(phy_type_tof_ctx_t *ctx, int16 sc_ptr_offset,
	int16 sc_base_ptr, uint16 tof_seq_n)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;
	cint32 sc_win[10];
	int n, i;
	uint32 acc_pwr_start_win, acc_pwr_end_win;
	int16 sc_ptr;

	n = phy_ac_tof_sc_read(pi, TRUE, K_TOF_RX_SAMP_CHECK_NUM, &sc_win[0],
		sc_ptr_offset, sc_base_ptr, &sc_ptr);
	if (n != K_TOF_RX_SAMP_CHECK_NUM) {
		PHY_ERROR(("%s: Unable to dump the first %d samples of the window\n",
		__FUNCTION__, K_TOF_RX_SAMP_CHECK_NUM));
		return BCME_ERROR;
	}

	acc_pwr_start_win = 0;
	for (i = 0; i < K_TOF_RX_SAMP_CHECK_NUM; i++) {
#ifdef TOF_DBG_SEQ

		PHY_ERROR(("%s: i = %d, Start i,q = %d, %d\n", __FUNCTION__, i,
			sc_win[i].i, sc_win[i].q));
#endif // endif
		acc_pwr_start_win += (sc_win[i].i * sc_win[i].i + sc_win[i].q * sc_win[i].q);
	}

	if (acc_pwr_start_win < K_MIN_ACC_TOF_RX_SAMP_PWR) {
		PHY_ERROR(("%s: Rx Power Lower than Min Pwr, start_pwr = %d\n",  __FUNCTION__,
			acc_pwr_start_win));
		tofi->tof_phy_error |= WL_PROXD_PHY_RX_STRT_WIN_OFF;
	}

	if (sc_ptr_offset < 0)
		sc_ptr_offset -= (tof_seq_n - K_TOF_RX_SAMP_CHECK_NUM -1);
	else
		sc_ptr_offset += (tof_seq_n - K_TOF_RX_SAMP_CHECK_NUM - 1);

	n = phy_ac_tof_sc_read(pi, TRUE, K_TOF_RX_SAMP_CHECK_NUM, &sc_win[0],
		sc_ptr_offset, sc_base_ptr, &sc_ptr);

	if (n != K_TOF_RX_SAMP_CHECK_NUM) {
		PHY_ERROR(("%s: Unable to dump the last %d samples of the window\n",
		__FUNCTION__, K_TOF_RX_SAMP_CHECK_NUM));
		return BCME_ERROR;
	}
	acc_pwr_end_win = 0;

	for (i = 0; i < K_TOF_RX_SAMP_CHECK_NUM; i++) {
#ifdef TOF_DBG_SEQ
		PHY_ERROR(("%s: i = %d, End i,q = %d, %d\n", __FUNCTION__, i,
			sc_win[i].i, sc_win[i].q));
#endif // endif
		acc_pwr_end_win += (sc_win[i].i * sc_win[i].i + sc_win[i].q * sc_win[i].q);
	}

	PHY_ERROR(("%s: acc_pwr_start_win = %d, acc_pwr_end_win = %d\n", __FUNCTION__,
		acc_pwr_start_win, acc_pwr_end_win));
	if (acc_pwr_end_win < K_MIN_ACC_TOF_RX_SAMP_PWR) {
		PHY_ERROR(("%s: Rx Power Lower than Min Pwr, end_pwr = %d, Thresh = %d\n",
		__FUNCTION__, acc_pwr_end_win, K_MIN_ACC_TOF_RX_SAMP_PWR));
		tofi->tof_phy_error |= WL_PROXD_PHY_RX_END_WIN_OFF;
		if (acc_pwr_start_win < K_MIN_ACC_TOF_RX_SAMP_PWR) {
			PHY_ERROR(("%s: Rx Power failed check on Start and End. No Valid Samples\n",
			 __FUNCTION__));
		}
	}
	return BCME_OK;
}

static int phy_ac_seq_ts(phy_type_tof_ctx_t *ctx, int n, cint32* p_buffer, int tx,
	int cfo, int adj, void* pparams, int32* p_ts,
	int32* p_seq_len, uint32* p_raw, uint8* ri_rr,
	const uint8 smooth_win_en)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;
	int16 sc_ptr;
	int32 ts[2], dT;
	int i, n_out, a, b, ret_val_l, ret_val = BCME_OK;
	uint16 tof_seq_n;
	uint16 tmp_tof_sc_Fs;
	int32 tof_seq_M;
	uint16 snr_thresh, bitflip_thresh;

#ifdef TOF_COLLECT
	int collect_hraw_size = 0;
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		collect_hraw_size = K_TOF_COLLECT_HRAW_SIZE_20MHZ;
	} else {
		collect_hraw_size = K_TOF_COLLECT_HRAW_SIZE_80MHZ;
	}
#endif /* TOF_COLLECT */

	tof_seq_n = (1 << tofi->tof_seq_log2_n);

	tofi->tof_phy_error = 0;
	if (n < tof_seq_n) {
		return BCME_ERROR;
	}

	a = READ_PHYREGCE(pi, Core1RxIQCompA, tofi->tof_core);
	b = READ_PHYREGCE(pi, Core1RxIQCompB, tofi->tof_core);
	if (a > 511) {
		a -= 1024;
	}
	if (b > 511) {
		b -= 1024;
	}

	wlapi_tof_retrieve_thresh(pparams, &bitflip_thresh, &snr_thresh);

	for (i = 0; i < 2; i++) {
		tof_seq_n = (1 << tofi->tof_seq_log2_n);
		if (i) {
			phy_ac_tof_chk_rx_window_off(pi, -(tofi->tof_ucode_dlys_us[tx][3 + i]),
				K_TOF_SEQ_SC_START, tof_seq_n);
		}
		n_out = phy_ac_tof_sc_read(pi, TRUE, n, p_buffer,
			-(tofi->tof_ucode_dlys_us[tx][3 + i]),
			K_TOF_SEQ_SC_START, &sc_ptr);

		if (n_out != n) {
			return BCME_ERROR;
		}
#ifdef TOF_COLLECT
		if (p_raw && (2 * (n_out + 1) <= collect_hraw_size)) {
			int j;
			for (j = 0; j < n_out; j++) {
#if defined(TOF_DBG_SEQ)
				if ((j == 0) || (j == (n_out-1))) {
					PHY_ERROR(("%s: buf[%d] = %d, %d\n",
					 __FUNCTION__, j, p_buffer[j].i, p_buffer[j].q));
				}
#endif // endif
				*p_raw++ = (uint32)(p_buffer[j].i & 0xffff) |
					((uint32)(p_buffer[j].q & 0xffff) << 16);
			}
			*p_raw++ = (uint32)((int32)a & 0xffff) |
				(uint32)(((int32)b & 0xffff) << 16);
		}
#endif /* TOF_COLLECT */

#if defined(TOF_TEST_TONE)
		ts[i] = 0;
		tmp_tof_sc_Fs = tofi->tof_sc_FS;
		adj = 0;
#else
		uint16 tmp_seq_log2_n;
		ret_val_l = phy_ac_tof_mf(pi, tof_seq_n, p_buffer, FALSE, (i == 0),
			a, b, (i) ? cfo : 0,
			K_TOF_MF_IN_SHIFT,
			K_TOF_MF_OUT_SCALE,
			K_TOF_MF_OUT_SHIFT,
			bitflip_thresh,
			snr_thresh,
			smooth_win_en);
#ifdef TOF_SEQ_20MHz_BW_512IFFT
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			tmp_seq_log2_n = K_TOF_SEQ_N_20MHZ;
			tmp_tof_sc_Fs = 160;
			tof_seq_n = (1 << tmp_seq_log2_n);
		} else
#endif /* TOF_SEQ_20MHz_BW_512IFFT */
		{
			tmp_seq_log2_n = tofi->tof_seq_log2_n;
			tmp_tof_sc_Fs = tofi->tof_sc_FS;
		}

		ret_val_l = wlapi_tof_pdp_ts(tmp_seq_log2_n, (void*)p_buffer, tmp_tof_sc_Fs, i,
			pparams, &ts[i], NULL, &(tofi->tof_phy_error));
		if (ret_val_l != BCME_OK) {
			printf("func %s iter %d error 1 \n", __FUNCTION__, i);
			ret_val = BCME_ERROR;
		}

		if (tofi->isInvalid) {
			PHY_ERROR(("Low confidence in the"
				" measurement on this core due to low rssi.\n"));
		}

#endif /* defined(TOF_TEST_TONE) || 0 */
	}

#ifdef TOF_COLLECT
	int ri_rr_len;
	if (pi_ac->tofi->flag_sec_2_0) {
		ri_rr_len = FTM_TPK_RI_RR_LEN_SECURE_2_0;
	} else {
		ri_rr_len = FTM_TPK_RI_RR_LEN;
	}
	if (ri_rr != NULL) {
		for (i = 0; i < ri_rr_len; i++) {
			*(ri_rr + i) = tofi->ri_rr[i] & 0xff;
		}
	}
#endif /* TOF_COLLECT */

	tof_seq_M = ((10000 * tof_seq_n) / tmp_tof_sc_Fs);

	ts[0] += adj;
	dT = (ts[tx] - ts[tx ^ 1]);
	if (dT < 0) {
		dT += tof_seq_M;
	}
	*p_ts = dT;
	*p_seq_len = tof_seq_M;
	return ret_val;
}

#if defined(TOF_DBG_SEQ)
static int
phy_ac_tof_dbg(phy_type_tof_ctx_t *ctx, int arg)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	cint32 buf[K_TOF_DBG_SC_DELTA];
	int16  p, p_start;
	int i = 0, n = 0;
	bool  suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	uint8 stall_val;

	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	p = K_TOF_SEQ_SC_START;
	if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev))
		p += ((K_TOF_DBG_SC_DELTA >> 2) * arg * 3);
	else
		p += (K_TOF_DBG_SC_DELTA * arg);
	if (arg >= 255) {
		int j;
		uint16 v, offset = 0;
		uint16 bundle[3 * 16];
		const uint16 bundle_addr[] = { 0x00, 0x10, 0x20, 0x30, 0x40, 0x50 };
		for (i = 0; i < sizeof(bundle_addr) / sizeof(uint16); i++) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE,
				8, bundle_addr[i] + 8, 48, (void*)bundle);
			for (j = 0; j < 16; j++) {
				printf("RFBUNDLE 0x%x : 0x%04x%04x%04x\n",
					(bundle_addr[i]+j+8),
					bundle[3 * j + 2], bundle[3 * j + 1], bundle[3 * j + 0]);
			}
		}
		for (i = 0; i < 16; i++) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x290 + i), 16,
				(void *)&bundle[0]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x320 + i), 16,
				(void *)&bundle[1]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x260 + i), 16,
				(void *)&bundle[2]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x2f0 + i), 16,
				(void *)&bundle[3]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x000 + i), 16,
				(void *)&bundle[4]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x070 + i), 16,
				(void *)&bundle[5]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x010 + i), 16,
				(void *)&bundle[6]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x080 + i), 16,
				(void *)&bundle[7]);
			PHY_INFORM(("RFSEQ RXs 0x%04x 0x%04x TXs 0x%04x 0x%04x TX 0x%04x 0x%04x "
				"RX 0x%04x 0x%04x\n",
				bundle[0], bundle[1], bundle[2], bundle[3],
				bundle[4], bundle[5], bundle[6], bundle[7]));
		}
		for (i = 0; i < 3; i++) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				(K_TOF_SEQ_RFSEQ_GAIN_BASE + i * 0x10 +
				K_TOF_SEQ_RFSEQ_LOOPBACK_GAIN_OFFSET), 16, (void*)&bundle[0]);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				(K_TOF_SEQ_RFSEQ_GAIN_BASE + i * 0x10 +
				K_TOF_SEQ_RFSEQ_RX_GAIN_OFFSET), 16, (void*)&bundle[1]);
			printf("GC%d  LBK 0x%04x RX 0x%04x\n", i, bundle[0], bundle[1]);
		}
		offset = K_TOF_SEQ_SHM_SETUP_REGS_OFFSET;
		n = 46;
		while (n-- > 0) {
			v = wlapi_bmac_read_shm(pi->sh->physhim, (tofi->tof_shm_ptr + offset));
			PHY_INFORM(("SHM %d 0x%04x\n",
				((offset - K_TOF_SEQ_SHM_SETUP_REGS_OFFSET) >> 1), v));
			offset += 2;
		}
		n = 1;
	} else if (arg == 252) {
		printf("MAC 0x%x STRT %d STP %d CUR %d\n",
			R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi)),
			R_REG(pi->sh->osh, D11_SCP_STRTPTR(pi)),
			R_REG(pi->sh->osh, D11_SCP_STOPPTR(pi)),
			R_REG(pi->sh->osh, D11_SCP_CURPTR(pi)));
		n = 1;
	} else {
		if (arg == 0) {
			printf("MAC 0x%x STRT %d STP %d CUR %d\n",
				R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi)),
				R_REG(pi->sh->osh, D11_SCP_STRTPTR(pi)),
				R_REG(pi->sh->osh, D11_SCP_STOPPTR(pi)),
				R_REG(pi->sh->osh, D11_SCP_CURPTR(pi)));
		}
		if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
			n = (((int)R_REG(pi->sh->osh,
				D11_SCP_CURPTR(pi)) - p) / 3) << 2;
		} else {
			n = ((int)R_REG(pi->sh->osh, D11_SCP_CURPTR(pi)) - p);
		}
		if (n > K_TOF_DBG_SC_DELTA) {
			n = K_TOF_DBG_SC_DELTA;
		}
		if (n > 0) {
			arg = phy_ac_tof_sc_read(pi, TRUE, n, buf, p, K_TOF_SEQ_SC_START, &p_start);
			i = 0;
			while (i < arg) {
				if (buf[i].i > 2047)
					buf[i].i -= 4096;
				if (buf[i].q > 2047)
					buf[i].q -= 4096;
				printf("SD %4d %d %d\n", p_start, buf[i].i, buf[i].q);
				i++;
				p_start++;
			}
		} else {
			printf("\n");
		}
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

	return (n > 0) ? 1 : 0;
}
#endif /* defined(TOF_DBG_SEQ) */

#endif /* WL_PROXD_SEQ */

static int phy_ac_tof(phy_type_tof_ctx_t *ctx, bool enter, bool tx, bool hw_adj,
	bool seq_en, int core, int emu_delay)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;
	phy_info_acphy_t *aci = tofi->aci;
	bool change = tofi->tof_active != enter;
	bool  suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	int retval = BCME_OK;
	bool loadSPB;

	tofi->emu_delay = emu_delay;
	if (change) {
#ifdef WL_PROXD_SEQ
		if (seq_en) {
#ifndef K_TOF_LOAD_SPB_ONLY
			loadSPB = TRUE;
#else
			loadSPB = FALSE;
#endif /* !K_TOF_LOAD_SPB_ONLY */
			retval = phy_ac_tof_seq_setup(pi, enter, tx, core, TRUE, loadSPB,
				TRUE);
			if (!suspend) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			}
			if (enter) {
				phy_watchdog_suspend(pi);
				phy_ac_tof_en_dis_aci(tofi, enter);
				phy_ac_tof_fdiqcomp_save_disable(tofi);
				phy_rxgcrs_sel_classifier(pi, TOF_CLASSIFIER_BPHY_OFF_OFDM_ON);
			} else {
				phy_ac_tof_en_dis_aci(tofi, enter);
				/* Restore fdiqcomp state */
				MOD_PHYREG(pi, rxfdiqImbCompCtrl, rxfdiqImbCompEnable,
					tofi->tof_rx_fdiqcomp_enable);
				MOD_PHYREG(pi, fdiqImbCompEnable, txfdiqImbCompEnable,
					tofi->tof_tx_fdiqcomp_enable);
				phy_rxgcrs_sel_classifier(pi, TOF_CLASSIFIER_BPHY_ON_OFDM_ON);
				phy_watchdog_resume(pi);
			}
			if (!suspend) {
				wlapi_enable_mac(pi->sh->physhim);
			}
		}
		else
#endif /* WL_PROXD_SEQ */
		{
			tofi->tof_setup_done = FALSE;
			if (!suspend) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			}
			if (enter) {
				/* Disable BT coex */
				phy_btcx_disable_arbiter(pi->btcxi);
				phy_ac_tof_fdiqcomp_save_disable(tofi);
				if (hw_adj) {
					/* Save channel smoothing state and enable special  mode */
					phy_ac_chanmgr_save_smoothing(aci->chanmgri,
						&tofi->tof_smth_enable, &tofi->tof_smth_dump_mode);
					wlc_phy_smth(pi, SMTH_ENABLE, SMTH_TIMEDUMP_AFTER_IFFT);
					tofi->tof_smth_forced = TRUE;
				}
			} else {
				/* Enable BT coex */
				phy_btcx_enable_arbiter(pi->btcxi);
				/* Restore fdiqcomp state */
				MOD_PHYREG(pi, rxfdiqImbCompCtrl, rxfdiqImbCompEnable,
					tofi->tof_rx_fdiqcomp_enable);
				MOD_PHYREG(pi, fdiqImbCompEnable, txfdiqImbCompEnable,
					tofi->tof_tx_fdiqcomp_enable);
				if (tofi->tof_smth_forced) {
					/* Restore channel smoothing state */
					tofi->tof_smth_forced = FALSE;
					wlc_phy_smth(pi, tofi->tof_smth_enable,
						tofi->tof_smth_dump_mode);
				}
			}

			wlc_phy_resetcca_acphy(pi);
			if (!suspend) {
				wlapi_enable_mac(pi->sh->physhim);
			}
		}
	}
	tofi->tof_active = enter;
	return retval;
}

/* Save state fdiqcomp and disable */
static void
phy_ac_tof_fdiqcomp_save_disable(phy_ac_tof_info_t *tofi)
{
	tofi->tof_rx_fdiqcomp_enable = (uint8)READ_PHYREGFLD(tofi->pi, rxfdiqImbCompCtrl,
		rxfdiqImbCompEnable);
	tofi->tof_tx_fdiqcomp_enable = (uint8)READ_PHYREGFLD(tofi->pi, fdiqImbCompEnable,
		txfdiqImbCompEnable);
	MOD_PHYREG(tofi->pi, rxfdiqImbCompCtrl, rxfdiqImbCompEnable, 0);
	MOD_PHYREG(tofi->pi, fdiqImbCompEnable, txfdiqImbCompEnable, 0);
}

/* Enable Disable ACI */
static void
phy_ac_tof_en_dis_aci(phy_ac_tof_info_t *tofi, bool enter)
{
	phy_info_t *pi = tofi->pi;
	if (enter) {
		/* Force ACI Mitigation mode */
		if ((ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) ||
			ACMINORREV_4(pi))) ||
			(ACMAJORREV_5(pi->pubpi->phy_rev))) {
			tofi->aci_en = READ_PHYREGFLD(pi, ACI_Detect_CTRL,
				aci_detect_enable);
			PHY_ERROR(("%s: ACI Mode = %d\n", __FUNCTION__,
				tofi->aci_en));
			MOD_PHYREG(pi, ACI_Detect_CTRL,
				aci_detect_enable, 0);
			wlc_phy_hwaci_mitigate_acphy(pi, TRUE);
		} else if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		}
	} else {
		/* Restore back ACI state */
		if ((ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) ||
			ACMINORREV_4(pi))) ||
			(ACMAJORREV_5(pi->pubpi->phy_rev))) {
			/* This ensures an interrupt is triggered with current */
			/* aci_present_state once detection is enabled */
			wlapi_bmac_write_shm(pi->sh->physhim, M_HWACI_ST,
				K_TOF_RESTORE_HWACI_INTR);
			wlc_phy_hwaci_mitigate_acphy(pi, tofi->prev_aci_state);
			MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_enable,
				tofi->aci_en);
		} else if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		}

	}
}

/* Unpacks floating point to fixed point for further processing */
/* Fixed point format:

A.fmt = TRUE
sign      real         sign       image             exp
|-|--------------------||-|--------------------||----------|
size:            nman                   nman              nexp

B.fmt = FALSE
exp     sign		 image                  real
|----------||-|-||-------------------||--------------------|
size:		 nexp	 1 1          nman					nman

When Hi is NULL, we return "real * real + image * image" in Hr array, otherwise
real and image is save in Hr and Hi.

When autoscale is TRUE, calculate the max shift to save fixed point value into uint32
*/

/*
H holds upacked 32 bit data when the function is called.
H and Hout could partially overlap.
H and h can not overlap
*/

static int
wlc_unpack_float_acphy(int nbits, int autoscale, int shft,
int fmt, int nman, int nexp, int nfft, uint32* H, cint32* Hout, int32* h)
{
	int e_p, maxbit, e, i, pwr_shft = 0, e_zero, sgn;
	int n_out, e_shift;
	int8 He[256];
	int32 vi, vq, *pOut;
	uint32 x, iq_mask, e_mask, sgnr_mask, sgni_mask;

	/* when fmt is TRUE, the size nman include its sign bit */
	/* so need to minus one to get value mask */
	if (fmt)
		iq_mask = (1 << (nman - 1)) - 1;
	else
		iq_mask = (1 << nman) - 1;

	e_mask = (1 << nexp) - 1;	/* exp part mask */
	e_p = (1 << (nexp - 1));	/* max abs value of exp */

	if (h) {
		/* Set pwr_shft to make sure that square sum can be hold by uint32 */
		pwr_shft = (2 * nman + 1 - 31);
		if (pwr_shft < 0)
			pwr_shft = 0;
		pwr_shft = (pwr_shft + 1) >> 1;
		sgnr_mask = 0;	/* don't care sign for square sum */
		sgni_mask = 0;
		e_zero = -(2 * (nman - pwr_shft) + 1);
		pOut = (int32*)h;
		n_out = nfft;
		e_shift = 0;
	} else {
		/* Set the location of sign bit */
		if (fmt) {
			sgnr_mask = (1 << (nexp + 2 * nman - 1));
			sgni_mask = (sgnr_mask >> nman);
		} else {
			sgnr_mask = (1 << 2 * nman);
			sgni_mask = (sgnr_mask << 1);
		}
		e_zero = -nman;
		pOut = (int32*)Hout;
		n_out = (nfft << 1);
		e_shift = 1;
	}

	maxbit = -e_p;
	for (i = 0; i < nfft; i++) {
		/* get the real, image and exponent value */
		if (fmt) {
			vi = (int32)((H[i] >> (nexp + nman)) & iq_mask);
			vq = (int32)((H[i] >> nexp) & iq_mask);
			e = (int)(H[i] & e_mask);
		} else {
			vi = (int32)(H[i] & iq_mask);
			vq = (int32)((H[i] >> nman) & iq_mask);
			e = (int32)((H[i] >> (2 * nman + 2)) & e_mask);
		}

		/* adjust exponent */
		if (e >= e_p)
			e -= (e_p << 1);

		if (h) {
			/* calculate square sum of real and image data */
			vi = (vi >> pwr_shft);
			vq = (vq >> pwr_shft);
			h[i] = vi*vi + vq*vq;
			vq = 0;
			e = 2 * (e + pwr_shft);
		}

		He[i] = (int8)e;

		/* auto scale need to find the maximus exp bits */
		x = (uint32)vi | (uint32)vq;
		if (autoscale && x) {
			uint32 m = 0xffff0000, b = 0xffff;
			int s = 16;

			while (s > 0) {
				if (x & m) {
					e += s;
					x >>= s;
				}
				s >>= 1;
				m = (m >> s) & b;
				b >>= s;
			}
			if (e > maxbit)
				maxbit = e;
		}

		if (!h) {
			if (H[i] & sgnr_mask)
				vi |= K_TOF_UNPACK_SGN_MASK;
			if (H[i] & sgni_mask)
				vq |= K_TOF_UNPACK_SGN_MASK;
			Hout[i].i = vi;
			Hout[i].q = vq;
		}
	}

	/* shift bits */
	if (autoscale)
		shft = nbits - maxbit;

	/* scal and sign */
	for (i = 0; i < n_out; i++) {
		e = He[(i >> e_shift)] + shft;
		vi = *pOut;
		sgn = 1;
		if (!h && (vi & K_TOF_UNPACK_SGN_MASK)) {
			sgn = -1;
			vi &= ~K_TOF_UNPACK_SGN_MASK;
		}
		/* trap the zero case */
		if (e < e_zero) {
			vi = 0;
		} else if (e < 0) {
			e = -e;
			vi = (vi >> e);
		} else {
			vi = (vi << e);
		}
		*pOut++ = (int32)sgn*vi;
	}

	return shft;
}

static void
phy_ac_tof_setup_ack_core(phy_type_tof_ctx_t *ctx, int core)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	uint16 phyctl0;
	uint offset = tofi->tof_shm_ptr + M_TOF_PHYCTL0;

	ASSERT(pi->sh->clk);

	phyctl0 = wlapi_bmac_read_shm(pi->sh->physhim, offset);
	phyctl0 &= ~(0x7 << 6);
	phyctl0 |=  (1 << (core + 6));

	wlapi_bmac_write_shm(pi->sh->physhim, offset, phyctl0);
}

static int
phy_ac_tof_calc_snr_bitflips(phy_type_tof_ctx_t *ctx, void *In,
	wl_proxd_bitflips_t *bit_flips, wl_proxd_snr_t *snr)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;
	cint32* chan = (cint32*) tofi->chan;
	uint16 bw_factor = 0;
	uint16  print_chan_len;
	int retval = BCME_OK;
	const int num_max_cores = PHYCORENUM((pi)->pubpi->phy_corenum);

	if (CHSPEC_IS80(pi->radio_chanspec)) {
		bw_factor = 2;
	} else if (CHSPEC_IS20(pi->radio_chanspec)) {
		bw_factor = 0;
	} else {
		PHY_INFORM(("ERROR: Bandwidth other than 20 and 80 is not supported.\n"));
		return BCME_UNSUPPORTED;
	}
	print_chan_len = (K_TOF_CHAN_LENGTH_20M << bw_factor);
	chan += (num_max_cores * print_chan_len);
	if (tofi->flag_sec_2_0) {
		wlc_phy_tof_conj_arr(chan, print_chan_len);
	}
	retval = phy_ac_tof_demod_snr(In, (void*)chan, bw_factor, bit_flips, snr);
	return retval;
}

static void
phy_ac_tof_core_select(phy_type_tof_ctx_t *ctx, const uint32 gdv_th, const int32 gdmm_th,
		const int8 rssi_th, const int8 delta_rssi_th, uint8* core, uint8 core_mask)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	const int num_max_cores = PHYCORENUM((pi)->pubpi->phy_corenum);
	const uint16 *band_length, *sc_idx_arr;
	cint32* chan = (cint32*) tofi->chan;
	int32 gd_m[num_max_cores], minm_gd = INT32_MAX, minm_idx = -1, minv_idx = -1, minr_idx = -1;
	int32 delta_th[num_max_cores], maxdth_v = INT32_MIN, maxdth_idx = -1, gd_th;
	uint32 gd_v[num_max_cores], minv_gd = UINT32_MAX;
	uint16 num_bl, chan_len, num_rssi_true = 0, num_var_true = 0, num_delta_th_true = 0;
	uint8 bw_factor = 0, i; //, shared_core = 0;
	int8 sel_core = -1, max_rssi = INT8_MIN;
	bool cond_rssi[num_max_cores], cond_var[num_max_cores], cond_delta_rssi[num_max_cores];
	bool cond_delta_th[num_max_cores], cond_core_on[num_max_cores];

	gd_th = gdmm_th;
	PHY_ERROR(("gd_th = %d", gd_th));

	tofi->isInvalid = 0;
	/* if (num_max_cores > 1)
		shared_core = (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags2)
						& BFL2_BT_SHARE_ANT0) ? 0 : 1;
	*/

	for (i = 0; i < num_max_cores; i++) {
		cond_rssi[i] = FALSE;
		cond_delta_rssi[i] = FALSE;
		cond_var[i] = FALSE;
		cond_delta_th[i] = FALSE;
		delta_th[i] = 0;
		tofi->rssi[i] = phy_rssi_get_rssi(pi, i);

		cond_core_on[i] = (((stf_shdata->phytxchain >> i) & 0x1) &
			((stf_shdata->phyrxchain >> i) & 0x1)) ? ((core_mask >> i) & 0x1) : FALSE;

		PHY_ERROR(("rssi[%d] = %d, cond_core_on[%d] = %d\n", i, tofi->rssi[i],
			i, cond_core_on[i]));

		if ((tofi->rssi[i] > rssi_th) && (tofi->rssi[i] != 0) && cond_core_on[i]) {
			num_rssi_true++;
			minr_idx = i;
			cond_rssi[i] = TRUE;
		}

		if ((tofi->rssi[i] > max_rssi) && cond_rssi[i]) {
			max_rssi = tofi->rssi[i];
		}
	}

	if (num_rssi_true != 1) {
		if (CHSPEC_IS80(pi->radio_chanspec)) {
			bw_factor = 2;
		} else if (CHSPEC_IS20(pi->radio_chanspec)) {
			bw_factor = 0;
		} else {
			PHY_ERROR(("ERROR: Bandwidth other than 20 and 80 is not supported.\n"));
		}

		chan_len = (K_TOF_CHAN_LENGTH_20M << bw_factor);
		switch (bw_factor) {
			case 2:
				num_bl = K_TOF_NUM_LEGACY_BL_80M;
				band_length = band_length_80MHz;
				sc_idx_arr = nonzero_sc_idx_legacy_80MHZ;
				break;
			case 0:
			default:
				num_bl = K_TOF_NUM_LEGACY_BL_20M;
				band_length = band_length_20MHz;
				sc_idx_arr = nonzero_sc_idx_legacy_20MHZ;
		}
		for (i = 0; i < num_max_cores; i++) {
			cond_delta_rssi[i] = cond_rssi[i] &&
				((max_rssi - tofi->rssi[i]) < delta_rssi_th);
			PHY_ERROR(("cond_delta_rssi[%d] = %d\n", i, cond_delta_rssi[i]));

			if (((num_rssi_true == 0) || cond_rssi[i]) &&
					cond_delta_rssi[i] && cond_core_on[i]) {
				phy_ac_tof_group_delay_mv((chan + i*chan_len), num_bl, band_length,
						sc_idx_arr, gd_th, (gd_m + i), (gd_v + i),
						(delta_th + i));
				PHY_ERROR(("core = %d, gd_m[%d] = %d, gd_v[%d] = %u,"
						"delta_th[%d] = %d\n",
						i, i, gd_m[i], i, gd_v[i], i, delta_th[i]));
				if (delta_th[i] >= 0) {
					cond_delta_th[i] = TRUE;
					num_delta_th_true++;
				}
				if (delta_th[i] >= maxdth_v) {
					maxdth_v = delta_th[i];
					maxdth_idx = i;
				}
				if (gd_v[i] < gdv_th) {
					num_var_true++;
					cond_var[i] = TRUE;
				}
				if (gd_v[i] <= minv_gd) {
					minv_gd = gd_v[i];
					minv_idx = i;
				}
			}
		}
		if (num_delta_th_true == 0) {
			sel_core = maxdth_idx;
		} else {
			if (num_var_true == 0) {
				sel_core = minv_idx;
			} else {
				for (i = 0; i < num_max_cores; i++) {
					if ((gd_m[i] <= minm_gd) && (cond_var[i]) &&
							(cond_delta_th[i])) {
						minm_gd = gd_m[i];
						minm_idx = i;
					}
					PHY_ERROR(("core = %d, minm_gd = %d, minm_idx = %d\n",
							i, minm_gd, minm_idx));
				}
				sel_core = minm_idx;
			}
		}
		PHY_ERROR(("Selecting core : "));
	} else {
		PHY_ERROR(("Selecting core based on rssi : "));
		sel_core = minr_idx;
	}
	if ((sel_core < 0) || (sel_core >= num_max_cores)) {
		*core = (cond_core_on[0]) ? 0 : 1;
		PHY_ERROR(("Error : Can't select core. Defaulting to core %d.\n", *core));
	} else {
		*core = sel_core;
		PHY_ERROR(("core = %d\n", *core));
	}
	phy_ac_tof_setup_ack_core(tofi, *core);
	if (!cond_rssi[*core]) {
		tofi->isInvalid = tofi->isInvalid | 0x1;
	}
	/*
	if (!cond_var[*core]) {
		tofi->isInvalid = tofi->isInvalid | 0x2;
	}
	*/
}

/* Get channel frequency response for deriving 11v rx timestamp */
static int
phy_ac_tof_chan_freq_response(phy_type_tof_ctx_t *ctx, int len, int nbits, bool swap_pn_half,
		uint32 offset, cint32* H, uint32* Hraw, uint8 core, uint32 sts_offset,
		const bool single_core)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	uint32 *pTmp, *pIn;
	int i, i_l, i_r, n1, n2, n3, nfft, nfft_over_2;
	uint32 table_idx, table_width, t_core, t_offset;

	t_core = single_core ? tofi->tof_core : core;
	t_offset = single_core ? 0 : sts_offset;
#ifdef TOF_DBG
	int collect_hraw_size = 0;
#if defined(K_TOF_COLLECT_HRAW_SIZE_20MHZ) && defined(K_TOF_COLLECT_HRAW_SIZE_80MHZ)
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		collect_hraw_size = K_TOF_COLLECT_HRAW_SIZE_20MHZ;
	} else {
		collect_hraw_size = K_TOF_COLLECT_HRAW_SIZE_80MHZ;
	}
#endif /* defined(K_TOF_COLLECT_HRAW_SIZE_20MHZ) && defined(K_TOF_COLLECT_HRAW_SIZE_80MHZ) */
#endif /* TOF_DBG */

	if ((len != TOF_NFFT_20MHZ) && (len != TOF_NFFT_40MHZ) && (len != TOF_NFFT_80MHZ)) {
		return BCME_ERROR;
	}

	ASSERT(sizeof(cint32) == 2 * sizeof(int32));

	if (!swap_pn_half) {
		H = ((cint32 *)tofi->chan + offset);
		ASSERT(H != NULL);
	}

	pTmp = (uint32*)H;
	pIn = (uint32*)H + len;
	table_idx = ACPHY_TBL_ID_CHANEST(t_core);
	table_width = CORE0CHANESTTBL_TABLE_WIDTH;

	wlc_phy_table_read_acphy(pi, table_idx, len, t_offset, table_width, pTmp);
	printf("ClassifierCtrl : 0x%x\n", READ_PHYREG(pi, ClassifierCtrl));
#ifdef TOF_DBG
#if defined(K_TOF_COLLECT_HRAW_SIZE_20MHZ) && defined(K_TOF_COLLECT_HRAW_SIZE_80MHZ)
	if (Hraw && (len <= collect_hraw_size)) {
		bcopy((void*)pTmp, (void*)Hraw, len*sizeof(uint32));
	}
#else
	if (Hraw) {
		bcopy((void*)pTmp, (void*)Hraw, len*sizeof(uint32));
	}
#endif  /* defined(K_TOF_COLLECT_HRAW_SIZE_20MHZ) && defined(K_TOF_COLLECT_HRAW_SIZE_80MHZ) */
	/* store raw data for log collection */
#endif /* TOF_DBG */
	memset((void *)pIn, 0, len * sizeof(int32));
	if (swap_pn_half) {
		/* reorder tones */
		nfft = len;
		nfft_over_2 = (len >> 1);
		if (CHSPEC_IS80(pi->radio_chanspec) ||
			PHY_AS_80P80(pi, pi->radio_chanspec)) {
			i_l = 122;
			i_r = 2;
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
			i_l = 122;
			i_r = 2;
			ASSERT(0);
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			i_l = 58;
			i_r = 2;
		} else {
			/* for legacy this is 26, for vht-20 this is 28 */
			i_l = 28;
			i_r = 1;
		}
		for (i = i_l; i >= i_r; i--) {
			n1 = nfft_over_2 - i;
			n2 = nfft_over_2 + i;
			n3 = nfft - i;
			pIn[n1] = pTmp[n3];
			pIn[n2] = pTmp[i];
		}
	} else {
		for (i = 0; i < len; i++) {
			pIn[i] = pTmp[i];
		}
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
		int32 chi, chq;

		for (i = 0; i < len; i++) {
			chi = ((int32)pIn[i] >> CORE0CHANESTTBL_INTEGER_DATA_SIZE) &
				CORE0CHANESTTBL_INTEGER_DATA_MASK;
			chq = (int32)pIn[i] & CORE0CHANESTTBL_INTEGER_DATA_MASK;
			if (chi >= CORE0CHANESTTBL_INTEGER_MAXVALUE) {
				chi = chi - (CORE0CHANESTTBL_INTEGER_MAXVALUE << 1);
			}
			if (chq >= CORE0CHANESTTBL_INTEGER_MAXVALUE) {
				chq = chq - (CORE0CHANESTTBL_INTEGER_MAXVALUE << 1);
			}
			H[i].i = chi;
			H[i].q = chq;
		}
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) ||
		ACMAJORREV_4(pi->pubpi->phy_rev) ||
		ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		wlc_unpack_float_acphy(nbits, UNPACK_FLOAT_AUTO_SCALE, 0,
			CORE0CHANESTTBL_FLOAT_FORMAT, CORE0CHANESTTBL_REV2_DATA_SIZE,
			CORE0CHANESTTBL_REV2_EXP_SIZE, len, pIn, H, NULL);
	} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		wlc_unpack_float_acphy(nbits, UNPACK_FLOAT_AUTO_SCALE, 0,
			CORE0CHANESTTBL_FLOAT_FORMAT, CORE0CHANESTTBL_REV0_DATA_SIZE,
			CORE0CHANESTTBL_REV0_EXP_SIZE, len, pIn, H, NULL);
	} else {
		return BCME_UNSUPPORTED;
	}

	return BCME_OK;
}

/* Get mag sqrd channel impulse response(from channel smoothing hw) to derive 11v rx timestamp */
static int
phy_ac_chan_mag_sqr_impulse_response(phy_type_tof_ctx_t *ctx, int frame_type,
	int len, int offset, int nbits, int32* h, int* pgd, uint32* hraw, uint16 tof_shm_ptr)
{
	uint8 stall_val;
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;
	phy_info_acphy_t *pi_ac = tofi->aci;

	int N, s0, s1, m_mask, idx, i, j, l, m, i32x4, i4x2, tbl_offset, gd, n;
	uint32 *pdata, *ptmp, data_l, data_h;
	uint16 chnsm_status0;
	uint16 channel_smoothing = (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47(pi->pubpi->phy_rev)) ?
		AC2PHY_TBL_ID_CHANNELSMOOTHING_1x1 : ACPHY_TBL_ID_CHANNELSMOOTHING_1x1;
#ifdef TOF_DBG
	int collect_hraw_size = 0;
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		collect_hraw_size = K_TOF_COLLECT_HRAW_SIZE_20MHZ;
	} else {
		collect_hraw_size = K_TOF_COLLECT_HRAW_SIZE_80MHZ;
	}
#endif /* TOF_DBG */

	idx = -offset;

	if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		/* Only 11n/acphy frames */
		if (frame_type < 2)
			return BCME_UNSUPPORTED;
	} else if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		/* 11n/acphy frames + 20MHz legacy frames */
		if ((frame_type < 2) && !((frame_type == 1) && (pi_ac->curr_bw == 0)))
			return BCME_UNSUPPORTED;
	} else {
		return BCME_UNSUPPORTED;
	}

	if (CHSPEC_IS80(pi->radio_chanspec)) {
		N = TOF_NFFT_80MHZ;
		s0 = 2 + TOF_BW_80MHZ_INDEX;
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		N = TOF_NFFT_40MHZ;
		s0 = 2 + TOF_BW_40MHZ_INDEX;
	} else if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
		N = TOF_NFFT_20MHZ;
		s0 = 2 + TOF_BW_20MHZ_INDEX;
	} else
		return BCME_ERROR;

	chnsm_status0 = wlapi_bmac_read_shm(pi->sh->physhim, tof_shm_ptr +
		M_TOF_CHNSM_0_OFFSET(pi));
	gd = (int)((chnsm_status0 >>
		ACPHY_chnsmStatus0_group_delay_SHIFT(pi->pubpi->phy_rev)) & 0xff);
	if (gd > 127)
		gd -= 256;

	if (ACMAJORREV_3(pi->pubpi->phy_rev))
		idx += gd; /* Impulse response is not shited, 0 is @ gd */

	*pgd = Q1_NS * gd; /* gd in Q1 ns */

	if (len > N)
		len = N;

	s1 = s0 + 2;
	m_mask = (1 << s1) - 1;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	if (stall_val == 0)
		ACPHY_DISABLE_STALL(pi);

	phy_utils_write_phyreg(pi, ACPHY_TableID(pi->pubpi->phy_rev), channel_smoothing);

	n = len;
	ptmp = (uint32*)h + len;
	pdata = ptmp;
	while (n > 0) {
		idx = idx & (N - 1);
		i = (idx >> 2);
		j = (idx & 3);
		l = i + j;
		i32x4 = (i & 0xc);
		if (N == TOF_NFFT_80MHZ) {
			m  = (i32x4 + (i << 2) + (i32x4 >> 2)) & 0xf;
			m += (i ^ ((i << 1) & 0x20)) & 0x30;
			l += (i >> 4) + (i >> 2);
		} else if (N == TOF_NFFT_40MHZ) {
			i4x2 = (i >> 3) & 2;
			m  = ((0x1320 >> i32x4) & 0xf) << 3;
			m += ((((i >> 2) & 1) + i + i4x2) & 3) << 1;
			m += (i4x2 >> 1);
			l += (0x130 >> i32x4) + i4x2;
		} else {
			m  = ((i & 3) + (i32x4 ^ (i32x4 << 1))) & 0xf;
			l += (i >> 2);
		}
		l = (l & 3);
		tbl_offset = ((m + (l << s0)) & m_mask) + (l << s1) + CHANNELSMOOTHING_DATA_OFFSET;
		phy_utils_write_phyreg(pi, ACPHY_TableOffset(pi->pubpi->phy_rev),
			(uint16) tbl_offset);
		data_l = (uint32)phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
		data_h = (uint32)phy_utils_read_phyreg_wide(pi);
		*pdata++ = (data_h << 16) | (data_l & 0xffff);
		idx++;
		n--;
	};

	if (stall_val == 0) {
		ACPHY_ENABLE_STALL(pi, stall_val);
	}

#ifdef TOF_DBG
#if defined(K_TOF_COLLECT_HRAW_SIZE_20MHZ) && defined(K_TOF_COLLECT_HRAW_SIZE_80MHZ)
	if (hraw && ((len + 1) <= collect_hraw_size)) {
		bcopy((void*)ptmp, (void*)hraw, len*sizeof(uint32));
		/* store raw data for log collection */
		hraw[len] = (uint32)chnsm_status0;
	}
#else
	if (hraw) {
		bcopy((void*)ptmp, (void*)hraw, len*sizeof(uint32));
		hraw[len] = (uint32)chnsm_status0;
	}
#endif /* defined(K_TOF_COLLECT_HRAW_SIZE_20MHZ) && defined(K_TOF_COLLECT_HRAW_SIZE_80MHZ) */
#endif /* TOF_DBG */
	wlc_unpack_float_acphy(nbits, UNPACK_FLOAT_AUTO_SCALE, 0, CHANNELSMOOTHING_FLOAT_FORMAT,
		CHANNELSMOOTHING_FLOAT_DATA_SIZE, CHANNELSMOOTHING_FLOAT_EXP_SIZE,
		len, ptmp, NULL, h);

	return BCME_OK;
}

/* get rxed frame type, bandwidth and rssi value */
static int phy_ac_tof_info(phy_type_tof_ctx_t *ctx, wlc_phy_tof_info_t *tof_info,
	wlc_phy_tof_info_type_t tof_info_mask, uint8 core)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	uint16 status0, status1, status5;
	uint16 subband_shift;
	int frame_bw, cfo;

	status0 = (int)READ_PHYREG(pi, RxStatus0) & 0xffff;
	status1 = (int)READ_PHYREG(pi, RxStatus1);
	status5 = (int)READ_PHYREG(pi, RxStatus5);

	if (tof_info_mask & WLC_PHY_TOF_INFO_TYPE_FRAME_TYPE) {
		tof_info->frame_type = status0 & PRXS0_FT_MASK;
		tof_info->info_mask |= WLC_PHY_TOF_INFO_TYPE_FRAME_TYPE;
	}

	if (tof_info_mask & WLC_PHY_TOF_INFO_TYPE_FRAME_BW) {
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			frame_bw = status1 & PRXS1_ACPHY_SUBBAND_MASK_GEN2;
			subband_shift = PRXS1_ACPHY_SUBBAND_SHIFT_GEN2;
		} else {
			frame_bw = status0 & PRXS0_ACPHY_SUBBAND_MASK;
			subband_shift = PRXS0_ACPHY_SUBBAND_SHIFT;
		}
		if (frame_bw == (PRXS_SUBBAND_80 << subband_shift)) {
		   frame_bw = TOF_BW_80MHZ_INDEX | (frame_bw << 16);
		}
		else if ((frame_bw == (PRXS_SUBBAND_40L << subband_shift)) ||
			(frame_bw == (PRXS_SUBBAND_40U << subband_shift))) {
		   frame_bw = TOF_BW_40MHZ_INDEX | (frame_bw << 16);
		}
		else {
		   frame_bw = TOF_BW_20MHZ_INDEX | (frame_bw << 16);
		}
		tof_info->frame_bw = frame_bw;
		tof_info->info_mask |= WLC_PHY_TOF_INFO_TYPE_FRAME_BW;
	}

	if (tof_info_mask & WLC_PHY_TOF_INFO_TYPE_RSSI) {
		tof_info->rssi = (wl_proxd_rssi_t)phy_rssi_get_rssi(pi, core);
		tof_info->info_mask |= WLC_PHY_TOF_INFO_TYPE_RSSI;
	}

	if (tof_info_mask & WLC_PHY_TOF_INFO_TYPE_CFO) {
		cfo = ((int)status5 & 0xff);
		if (cfo > 127) {
			   cfo -= 256;
		}
		cfo = cfo * 2298;
		tof_info->cfo = cfo;
		tof_info->info_mask |= WLC_PHY_TOF_INFO_TYPE_CFO;
	}

	if (tof_info_mask & WLC_PHY_TOF_INFO_TYPE_SNR) {
		tof_info->snr = tofi->snr;
		tof_info->info_mask |= WLC_PHY_TOF_INFO_TYPE_SNR;
	}

	if (tof_info_mask & WLC_PHY_TOF_INFO_TYPE_BITFLIPS) {
		tof_info->bitflips = tofi->bitflips;
		tof_info->info_mask |= WLC_PHY_TOF_INFO_TYPE_BITFLIPS;
	}

	if (tof_info_mask & WLC_PHY_TOF_INFO_TYPE_PHYERROR) {
		tof_info->tof_phy_error = tofi->tof_phy_error;
		tof_info->info_mask |= WLC_PHY_TOF_INFO_TYPE_PHYERROR;
	}
	return BCME_OK;
}

/* turn on classification to receive frames */
static void phy_ac_tof_cmd(phy_type_tof_ctx_t *ctx, bool seq, int emu_delay)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

#ifdef WL_PROXD_SEQ
	bool  suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	if (seq) {
		uint16 tof_seq_fem_gains[2], mask = (1 << tofi->tof_core);
		uint32 sc_stop = K_TOF_SEQ_SC_STOP;
		tofi->emu_delay = emu_delay;
		/* extend the sample capture buffer to accomodate the emulator delay */
		if (TOF_EB(emu_delay)) {
		  sc_stop += TOF_BUF_EXT;
		}
		MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, mask);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			tof_seq_fem_gains[0] = k_tof_seq_fem_gains_2g[0] | mask;
			tof_seq_fem_gains[1] = k_tof_seq_fem_gains_2g[1] | mask;
		}
		else {
			tof_seq_fem_gains[0] = k_tof_seq_fem_gains[0] | mask;
			tof_seq_fem_gains[1] = k_tof_seq_fem_gains[1] | mask;
		}
		wlc_tof_seq_write_shm_acphy(pi, 1, K_TOF_SEQ_SHM_FEM_RADIO_HI_GAIN_OFFSET,
			(uint16*)tof_seq_fem_gains);
		wlc_tof_seq_write_shm_acphy(pi, 1, K_TOF_SEQ_SHM_FEM_RADIO_LO_GAIN_OFFSET,
			(uint16*)tof_seq_fem_gains + 1);
		phy_ac_tof_sc(pi, TRUE, K_TOF_SEQ_SC_START, sc_stop, 0);
	}
#endif /* WL_PROXD_SEQ */

	phy_rxgcrs_sel_classifier(pi, TOF_CLASSIFIER_BPHY_OFF_OFDM_ON);
#ifdef WL_PROXD_SEQ
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
#endif // endif
}

/* get K value for initiator or target  */
static
const uint16 *phy_ac_tof_kvalue_tables(phy_info_t *pi, phy_ac_tof_info_t *tofi,
chanspec_t chanspec, int32* ki, int32* kt, int32* kseq)
{
	uint16 const *kvalueptr = NULL;
	int channel = CHSPEC_CHANNEL(chanspec);

	if (CHSPEC_IS80(chanspec)) {
		*ki = tofi->proxd_ki[0];
		*kt = tofi->proxd_kt[0];
	} else if (CHSPEC_IS40(chanspec)) {
		*ki = tofi->proxd_ki[1];
		*kt = tofi->proxd_kt[1];
	} else if (CHSPEC_IS20_5G(chanspec)) {
		*ki = tofi->proxd_ki[2];
		*kt = tofi->proxd_kt[2];
	} else {
		*ki = tofi->proxd_ki[3];
		*kt = tofi->proxd_kt[3];
	}

	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		/* For 4345 B0/B1 */
		if (CHSPEC_IS80(chanspec)) {
			kvalueptr = proxd_4345_80m_k_values;
		} else if (CHSPEC_IS40(chanspec)) {
			kvalueptr = proxd_4345_40m_k_values;
		} else if (channel >= 36) {
			kvalueptr = proxd_4345_20m_k_values;
		} else {
			kvalueptr = proxd_4345_2g_k_values;
		}
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_1(pi) ||
		ACMINORREV_4(pi))) {
		/* For 4350 C0/C1/C2 */
		if (CHSPEC_IS80(chanspec)) {
			kvalueptr = proxd_4350_80m_k_values;
			*kseq = 90;
		} else if (CHSPEC_IS40(chanspec)) {
			kvalueptr = proxd_4350_40m_k_values;
		} else if (channel >= 36) {
			kvalueptr = proxd_4350_20m_k_values;
		} else {
			kvalueptr = proxd_4350_2g_k_values;
			*kseq = TOF_SEQ_K_4350_2G;
		}
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_3(pi) ||
		ACMINORREV_5(pi))) {
		/* For 4354A1/4345A2(4356) */
		if (CHSPEC_IS80(chanspec)) {
			kvalueptr = proxd_4354_80m_k_values;
			*kseq = 90;
		} else if (CHSPEC_IS40(chanspec)) {
			kvalueptr = proxd_4354_40m_k_values;
		} else if (channel >= 36) {
			kvalueptr = proxd_4354_20m_k_values;
		} else {
			kvalueptr = proxd_4354_2g_k_values;
		}
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* For 4349/4355/4359 */
		if (CHSPEC_IS80(chanspec)) {
			kvalueptr = proxd_4349_80m_k_values;
#ifdef TOF_TUNE_KVAL
			*kseq = tofi->proxd_seq_kval[TOF_SEQ_80MHz_IDX];
#endif // endif
		} else if (CHSPEC_IS40(chanspec)) {
			kvalueptr = proxd_4349_40m_k_values;
		} else if (channel >= 36) {
			kvalueptr = proxd_4349_20m_k_values;
#ifdef TOF_TUNE_KVAL
			*kseq = tofi->proxd_seq_kval[TOF_SEQ_20MHz_IDX];
#endif // endif
		} else {
			kvalueptr = proxd_4349_2g_k_values;
#ifdef TOF_TUNE_KVAL
			*kseq = tofi->proxd_seq_kval[TOF_SEQ_2G_20MHz_IDX];
#endif // endif
		}
	} else if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		/* For 4360 */
		if (CHSPEC_IS80(chanspec)) {
			kvalueptr = NULL;
			*kseq = TOF_SEQ_K_4360_80MHz;
		} else if (CHSPEC_IS40(chanspec)) {
			kvalueptr = NULL;
		} else if (channel >= 36) {
			kvalueptr = NULL;
		} else {
			kvalueptr = NULL;
			*kseq = TOF_SEQ_K_4360_2G;
		}
	} else if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* For 43602 */
		if (CHSPEC_IS80(chanspec)) {
			kvalueptr = NULL;
			*kseq = TOF_SEQ_K_43602_80MHz;
		} else if (CHSPEC_IS40(chanspec)) {
			kvalueptr = NULL;
		} else if (channel >= 36) {
			kvalueptr = NULL;
		} else {
			kvalueptr = NULL;
			*kseq = TOF_SEQ_K_43602_2G;
		}
	} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		if (CHSPEC_IS80(chanspec)) {
			kvalueptr = proxd_4361_80m_k_values;
		} else if (CHSPEC_IS40(chanspec)) {
			kvalueptr = proxd_4361_40m_k_values;
		} else if (channel >= 36) {
			kvalueptr = proxd_4361_20m_k_values;
		} else {
			kvalueptr = proxd_4361_2g_k_values;
		}
	}

	return kvalueptr;
}

static int phy_ac_tof_kvalue(phy_type_tof_ctx_t *ctx, chanspec_t chanspec, uint32 rspecidx,
	uint32 *kip, uint32 *ktp, uint8 flag)
{
	uint16 const *kvaluep = NULL;
	int idx = 0, channel = CHSPEC_CHANNEL(chanspec);
	int32 ki = 0, kt = 0, kseq = 0;
	int rtt_adj = 0, rtt_adj_ts, irate_adj = 0, iack_adj = 0, trate_adj = 0, tack_adj = 0;
	int rtt_adj_papd = 0, papd_en = 0;
	uint8 bwidx;

	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	kvaluep = phy_ac_tof_kvalue_tables(pi, tofi, chanspec, &ki, &kt, &kseq);
	if (flag & WL_PROXD_SEQEN) {
		if (kip) {
			*kip = kseq;
		}
		if (ktp) {
			*ktp = kseq;
		}
		return BCME_OK;
	}
	bwidx = flag & WL_PROXD_BW_MASK;

	if (kvaluep) {
		int8 rateidx, ackidx; /* VHT = -1, legacy6M = 0, legacy = 1, mcs0 = 2, mcs = 3 */
		rateidx = rspecidx & 0xff;
		rateidx--;
		ackidx = (rspecidx >> 8) & 0xff;
		ackidx--;
		rtt_adj = (4 - READ_PHYREGFLD(pi, RxFeCtrl1, rxfe_bilge_cnt));
		rtt_adj_ts = 80;
		if (READ_PHYREGFLD(pi, PapdEnable0, papd_compEnb0)) {
			papd_en = 1;
		}
		if (CHSPEC_IS80(chanspec)) {
			if (channel <= 58) {
				idx = (channel - 42) >> 4;
			} else if (channel <= 138) {
				idx = ((channel - 106) >> 4) + 2;
			} else {
				idx = 5;
			}
			rtt_adj_ts = 25;
			if (papd_en) {
				rtt_adj_papd = 25;
			}
			if (rateidx != -1) {
				irate_adj = tofi->proxdi_rate_offset_80m[rateidx];
				trate_adj = tofi->proxdt_rate_offset_80m[rateidx];
			}
			if (ackidx != -1) {
				iack_adj = tofi->proxdi_ack_offset[0];
				tack_adj = tofi->proxdt_ack_offset[0];
			}
		} else if (CHSPEC_IS40(chanspec)) {
			if (channel <= 62) {
				idx = (channel - 38) >> 3;
			} else if (channel <= 142) {
				idx = ((channel - 102) >> 3) + 4;
			} else {
				idx = ((channel - 151) >> 3) + 10;
			}
			rtt_adj_ts = 40;
			if (papd_en) {
				rtt_adj_papd = 30;
			}
			if (rateidx != -1) {
				irate_adj = tofi->proxdi_rate_offset_40m[rateidx];
				trate_adj = tofi->proxdt_rate_offset_40m[rateidx];
			}
			if (ackidx != -1) {
				iack_adj = tofi->proxdi_ack_offset[1];
				tack_adj = tofi->proxdt_ack_offset[1];
			}
		} else if (CHSPEC_IS20_5G(chanspec)) {
			/* 5G 20M Hz channels */
			if (channel <= 64) {
				idx = (channel - 36) >> 2;
			} else if (channel <= 144) {
				idx = ((channel - 100) >> 2) + 8;
			} else {
				idx = ((channel - 149) >> 2) + 20;
			}
			if (papd_en) {
				rtt_adj_papd = 66;
			}
			if (rateidx != -1) {
				irate_adj = tofi->proxdi_rate_offset_20m[rateidx];
				trate_adj = tofi->proxdt_rate_offset_20m[rateidx];
			}
			if (ackidx != -1) {
				iack_adj = tofi->proxdi_ack_offset[2];
				tack_adj = tofi->proxdt_ack_offset[2];
			}
		} else if (channel >= 1 && channel <= 14) {
			/* 2G channels */
			idx = channel - 1;
			if (papd_en) {
				rtt_adj_papd = 70;
			}
			if (rateidx != -1) {
				irate_adj = tofi->proxdi_rate_offset_2g[rateidx];
				trate_adj = tofi->proxdt_rate_offset_2g[rateidx];
			}
			if (ackidx != -1) {
				iack_adj = tofi->proxdi_ack_offset[3];
				tack_adj = tofi->proxdt_ack_offset[3];
			}
		}
		rtt_adj = (rtt_adj_ts * rtt_adj) >> K_TOF_K_RTT_ADJ_Q;
		ki += ((int32)rtt_adj + (int32)rtt_adj_papd - irate_adj - iack_adj);
		kt += ((int32)rtt_adj + (int32)rtt_adj_papd - trate_adj - tack_adj);
		if (bwidx) {
			kt -= tofi->proxd_subbw_offset[bwidx - 1][rateidx + 1];
		}
		if (kip) {
			*kip = (uint32)(ki + (int8)(kvaluep[idx] & 0xff));
		}
		if (ktp) {
			*ktp = (uint32)(kt + (int8)(kvaluep[idx] >> 8));
		}
		return BCME_OK;
	}

	return BCME_ERROR;
}

int
phy_ac_tof_set_ri_rr(phy_type_tof_ctx_t *ctx, const uint8 *ri_rr, const uint16 len,
		const uint8 core, const bool isInitiator, const bool isSecure,
		wlc_phy_tof_secure_2_0_t secure_params)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	uint16 i = 0;
	uint16 loop_len, ri_phy_len, rr_phy_len;
	int ret = BCME_OK;
	uint8 tmp = 0;
	bool loadSPB;
	uint8 *r_ptr = (isInitiator) ? tofi->rtx : tofi->rrx;
	uint8 align_boundary = 4;

	BCM_REFERENCE(secure_params);

	if ((len != FTM_TPK_RI_RR_LEN) && (len != FTM_TPK_RI_RR_LEN_SECURE_2_0)) {
		return BCME_BADLEN;
	}

	tofi->flag_sec_2_0 = (len == FTM_TPK_RI_RR_LEN_SECURE_2_0) ? TRUE : FALSE;
	PHY_ERROR(("%s: secure ranging 2.0 Flag : %d\n", __FUNCTION__, tofi->flag_sec_2_0));

	loop_len = tofi->flag_sec_2_0 ? len : (len+1);
	ri_phy_len = tofi->flag_sec_2_0 ? FTM_TPK_RI_PHY_LEN_SECURE_2_0 : FTM_TPK_RI_PHY_LEN;
	rr_phy_len = tofi->flag_sec_2_0 ? FTM_TPK_RR_PHY_LEN_SECURE_2_0 : FTM_TPK_RR_PHY_LEN;

	if (tofi->flag_sec_2_0) {
		tofi->start_seq_time = secure_params.start_seq_time;
		tofi->delta_time_tx2rx = secure_params.delta_time_tx2rx;
	}
#ifdef TOF_DBG_SEQ
	PHY_ERROR(("%s: start_seq_time = %d, delta_time_tx2rx = %d\n",
		__FUNCTION__, tofi->start_seq_time, tofi->delta_time_tx2rx));
#endif // endif

	if (isSecure) {
		for (i = 0; i < loop_len; i++) {
			if (!tofi->flag_sec_2_0) {
				if (i < (FTM_TPK_RI_PHY_LEN - 1)) {
					tmp = *(ri_rr + i) & 0xff;
				} else if (i == (FTM_TPK_RI_PHY_LEN - 1)) {
					tmp = *(ri_rr + i) & 0xf;
				} else if (i < FTM_TPK_RI_RR_LEN) {
					tmp = (((*(ri_rr + i - 1) >> 4) & 0xf) |
						(*(ri_rr + i) << 4)) & 0xff;
				} else {
					tmp = ((*(ri_rr + i - 1) >> 4) & 0xf);
				}
			} else {
				tmp = *(ri_rr + i) & 0xff;
			}
			if (*r_ptr != tmp) {
				*r_ptr = tmp;
			}
			if (i == (ri_phy_len - 1)) {
				r_ptr = (isInitiator) ? tofi->rrx : tofi->rtx;
			} else {
				r_ptr++;
			}
		}
	}

	uint8 *tmp_rtx;
	uint8 *tmp_rrx;
	uint32 alloc_size_rtx, alloc_size_rrx;
	alloc_size_rtx = ri_phy_len * sizeof(uint8);
	alloc_size_rtx = ALIGN_SIZE(alloc_size_rtx, align_boundary);
	if ((tmp_rtx = phy_malloc(pi, alloc_size_rtx)) == NULL) {
			PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
			return BCME_NOMEM;
	}
	alloc_size_rrx = rr_phy_len * sizeof(uint8);
	alloc_size_rrx = ALIGN_SIZE(alloc_size_rrx, align_boundary);
	if ((tmp_rrx = phy_malloc(pi, alloc_size_rrx)) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		if (tmp_rtx != NULL) {
			phy_mfree(pi, tmp_rtx, alloc_size_rtx);
		}
		return BCME_NOMEM;
	}

	if (isSecure) {
		/*
		* Scramble FTM provided random bit sequences
		*/
		PHY_INFORM(("rtx = ["));
		for (i = 0; i < ri_phy_len; i++) {
			PHY_INFORM(("0x%x, ", tofi->rtx[i]));
		}
		PHY_INFORM(("];\n"));
		phy_ac_tof_gen_scrambled_output(tofi->rtx, tofi->rrx,
			((8*len) >> 1), tmp_rtx, tofi->flag_sec_2_0);
		PHY_INFORM(("tmp_rtx = ["));
		for (i = 0; i < ri_phy_len; i++) {
			PHY_INFORM(("0x%x, ", tmp_rtx[i]));
		}
		PHY_INFORM(("];\n"));
		PHY_INFORM(("rrx = ["));
		for (i = 0; i < rr_phy_len; i++) {
			PHY_INFORM(("0x%x, ", tofi->rrx[i]));
		}
		PHY_INFORM(("];\n"));

		phy_ac_tof_gen_scrambled_output(tofi->rrx, tofi->rtx,
			((8*len) >> 1), tmp_rrx, tofi->flag_sec_2_0);
		PHY_INFORM(("tmp_rrx = ["));
		for (i = 0; i < rr_phy_len; i++) {
			PHY_INFORM(("0x%x, ", tmp_rrx[i]));
		}
		PHY_INFORM(("];\n"));
		/* tof_seq_spb_len and tof_seq_spb has already been assigned and
		 * allocated memory during call to wlc_phy_tof_seq_params_acphy
		 */
		GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
		tofi->tof_seq_spb_tx = (uint32 *)k_tof_seq_spb_tx;
		tofi->tof_seq_spb_rx = (uint32 *)k_tof_seq_spb_rx;
		GCC_DIAGNOSTIC_POP();

		phy_ac_tof_bytes_to_spb(tmp_rtx, 2 * tofi->tof_seq_spb_len,
			tofi->tof_seq_spb_tx, tofi->flag_sec_2_0);
		phy_ac_tof_bytes_to_spb(tmp_rrx, 2 * tofi->tof_seq_spb_len,
			tofi->tof_seq_spb_rx, tofi->flag_sec_2_0);

		/*
		PHY_INFORM(("Tx : \n"));
		for(i = 0; i < 2 * aci->tof_seq_spb_len; i++) {
			PHY_INFORM(("0x%08x\n", tofi->tof_seq_spb_tx[i]));
		}
		PHY_INFORM(("\n"));
		PHY_INFORM(("Rx : \n"));
		for(i = 0; i < 2*aci->tof_seq_spb_len; i++) {
			PHY_INFORM(("0x%08x\n", tofi->tof_seq_spb_rx[i]));
		}
		PHY_INFORM(("\n"));
		*/
		for (i = 0; i < len; i++) {
			tofi->ri_rr[i] = *(ri_rr + i) & 0xff;
		}
	} else {
		for (i = 0; i < len; i++) {
			tofi->ri_rr[i] = 0;
		}
	}
	PHY_INFORM(("%s: PHY_RI_RR:\n", __FUNCTION__));
	for (i = 0; i < len; i++) {
		PHY_INFORM(("%x\t", *(ri_rr + i) & 0xff));
	}
	PHY_INFORM(("\n"));
	if (tmp_rtx != NULL) {
		phy_mfree(pi, tmp_rtx, alloc_size_rtx);
	}
	if (tmp_rrx != NULL) {
		phy_mfree(pi, tmp_rrx, alloc_size_rrx);
	}

#ifdef K_TOF_LOAD_SPB_ONLY
	loadSPB = TRUE;
#else
	loadSPB = FALSE;
#endif /* K_TOF_LOAD_SPB_ONLY */
	ret = phy_ac_tof_seq_setup(pi, FALSE, FALSE, core, isInitiator, loadSPB,
		FALSE);
	tofi->tof_phy_error = 0;
	return ret;
}

int
phy_ac_tof_set_ri_rr_spb_only(phy_info_t *pi, const bool isInitiator, const bool macSuspend)
{
	/*
	 * isInitiator value is warning if TOF_TEST_TONE is not defined or if loadSPB is FALSE.
	*/

	phy_ac_tof_info_t *tofi = pi->u.pi_acphy->tofi;
	bool suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	uint16 tof_seq_n = (1 << tofi->tof_seq_log2_n);

#ifdef TOF_TEST_TONE
	uint16 i = 0;
	const int32 v[16] = { 96, 89, 68, 37, 0, -37, -68, -89, -96, -89, -68, -37, 0, 37,
		68, 89 };
	const int32 x[16] = { 96, 68, 0, -68, -96, -68, 0, 68, 96, 	68, 0, -68, -96, -68,
		0, 68 };
#endif /* TOF_TEST_TONE */

	/* Acquire the memory buffer, make sure the correct buffer size is registered during attach
	 */
	cint32 *pSeq = phy_cache_acquire_reuse_buffer(pi->cachei, tof_seq_n * sizeof(*pSeq));

	if ((macSuspend) && (!suspend)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	/*
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	*/

	/* Setup sample play */
	phy_ac_tof_mf(pi, tof_seq_n, pSeq, TRUE, TRUE, 0, 0, 0,
		K_TOF_SEQ_IN_SCALE, K_TOF_SEQ_OUT_SCALE, K_TOF_SEQ_OUT_SHIFT,
		K_TOF_BITFLIP_TH_DEFAULT, K_TOF_SNR_TH_DEFAULT, 0);

#ifdef TOF_TEST_TONE
	for (i = 0; i < tof_seq_n; i++) {
		if (isInitiator) {
			pSeq[i].i = v[i & 0xf];
			pSeq[i].q = v[(i - 4) & 0xf];
		} else {
			pSeq[i].i = x[i & 0xf];
			pSeq[i].q = x[(i - 4) & 0xf];
		}
	}
#endif /* TOF_TEST_TONE */

	wlc_phy_loadsampletable_acphy(pi, pSeq, tof_seq_n, TRUE);
	PHY_INFORM(("SETUP CORE %d\n", 0));
	/* Release the memory buffer */
	phy_cache_release_reuse_buffer(pi->cachei);

	/*
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	*/
	if ((macSuspend) && (!suspend)) {
		wlapi_enable_mac(pi->sh->physhim);
	}

	return BCME_OK;
}

static int
phy_ac_tof_seq_params(phy_type_tof_ctx_t *ctx, bool assign_buffer)
{
	/*
	In case we are running secure ranging (ranging sequence is genrated out of ri_rr every
	session), this function should be called with assign_buffer = TRUE only before FTM1.
	After FTM1 when we have finished set_ri_rr function call, this should never be called
	with assign_buffer = TRUE otherwise ri_rr will never go in effect.
	*/

	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	if (assign_buffer) {
		if (CHSPEC_IS80(pi->radio_chanspec)) {
			tofi->tof_sc_FS = TOF_SC_FS_80MHZ;
#ifdef TOF_SEQ_20_IN_80MHz
			tofi->tof_seq_log2_n = K_TOF_SEQ_LOG2_N_80MHZ_20;
			tofi->tof_seq_spb_len = K_TOF_SEQ_SPB_LEN_20MHZ;

			GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
			tofi->tof_seq_spb_tx = (uint32 *)k_tof_seq_spb_20MHz;
			tofi->tof_seq_spb_rx = (uint32 *)k_tof_seq_spb_20MHz;
			GCC_DIAGNOSTIC_POP();
#else
			tofi->tof_seq_log2_n = K_TOF_SEQ_LOG2_N_80MHZ;
			tofi->tof_seq_spb_len = K_TOF_SEQ_SPB_LEN_80MHZ;

			GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
			tofi->tof_seq_spb_tx = (uint32 *)k_tof_seq_spb_80MHz;
			tofi->tof_seq_spb_rx = (uint32 *)k_tof_seq_spb_80MHz;
			GCC_DIAGNOSTIC_POP();

#endif /* TOF_SEQ_20_IN_80MHz */

			return BCME_OK;
		}

#if defined(TOF_SEQ_40MHz_BW) || defined(TOF_SEQ_40_IN_40MHz)
		else if (CHSPEC_IS40(pi->radio_chanspec)) {
			tofi->tof_sc_FS = TOF_SC_FS_40MHZ;
			tofi->tof_seq_log2_n = K_TOF_SEQ_LOG2_N_40MHZ;

#ifdef TOF_SEQ_40_IN_40MHz
			tofi->tof_seq_spb_len = K_TOF_SEQ_SPB_LEN_40MHZ;

			GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
			tofi->tof_seq_spb_tx = (uint32 *)k_tof_seq_spb_40MHz;
			tofi->tof_seq_spb_rx = (uint32 *)k_tof_seq_spb_40MHz;
			GCC_DIAGNOSTIC_POP();

#else
			tofi->tof_seq_spb_len = K_TOF_SEQ_SPB_LEN_20MHZ;

			GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
			tofi->tof_seq_spb_tx = (uint32 *)k_tof_seq_spb_20MHz;
			tofi->tof_seq_spb_rx = (uint32 *)k_tof_seq_spb_20MHz;
			GCC_DIAGNOSTIC_POP();

#endif /* TOF_SEQ_40_IN_40MHz */

			return BCME_OK;
		}
#endif /* defined(TOF_SEQ_40MHz_BW) || defined (TOF_SEQ_40_IN_40MHz) */

#if defined(TOF_SEQ_20MHz_BW) || defined(TOF_SEQ_20MHz_BW_512IFFT)
		else if (CHSPEC_IS2G(pi->radio_chanspec)) {
			tofi->tof_sc_FS = TOF_SC_FS_20MHZ;
			tofi->tof_seq_log2_n = K_TOF_SEQ_LOG2_N_20MHZ;
			tofi->tof_seq_spb_len = K_TOF_SEQ_SPB_LEN_20MHZ;

			GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
			tofi->tof_seq_spb_tx = (uint32 *)k_tof_seq_spb_20MHz;
			tofi->tof_seq_spb_rx = (uint32 *)k_tof_seq_spb_20MHz;
			GCC_DIAGNOSTIC_POP();

			return BCME_OK;
		} else {
			tofi->tof_sc_FS = TOF_SC_FS_20MHZ;
			tofi->tof_seq_log2_n = K_TOF_SEQ_LOG2_N_20MHZ;
			tofi->tof_seq_spb_len = K_TOF_SEQ_SPB_LEN_20MHZ;

			GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
			tofi->tof_seq_spb_tx = (uint32 *)k_tof_seq_spb_20MHz;
			tofi->tof_seq_spb_rx = (uint32 *)k_tof_seq_spb_20MHz;
			GCC_DIAGNOSTIC_POP();

			return BCME_OK;
		}
#endif /* defined(TOF_SEQ_20MHz_BW) || defined(TOF_SEQ_20MHz_BW_512IFFT) */
	}
	if (CHSPEC_IS80(pi->radio_chanspec)) {
#ifdef TOF_SEQ_20_IN_80MHz
		memcpy(tofi->tof_ucode_dlys_us, k_tof_ucode_dlys_us_80MHz_20, sizeof(uint16)*10);
#else
		memcpy(tofi->tof_ucode_dlys_us, k_tof_ucode_dlys_us_80MHz, sizeof(uint16)*10);
#endif // endif
		return BCME_OK;
	}
#if defined(TOF_SEQ_40MHz_BW) || defined(TOF_SEQ_40_IN_40MHz)
	else if (CHSPEC_IS40(pi->radio_chanspec)) {
#ifdef TOF_SEQ_40_IN_40MHz
		memcpy(tofi->tof_ucode_dlys_us, k_tof_ucode_dlys_us_40MHz_40, sizeof(uint16)*10);
#else
		memcpy(tofi->tof_ucode_dlys_us, k_tof_ucode_dlys_us_40MHz, sizeof(uint16)*10);
#endif // endif
		return BCME_OK;

	}
#endif /* defined(TOF_SEQ_40MHz_BW) || defined (TOF_SEQ_40_IN_40MHz) */

#if defined(TOF_SEQ_20MHz_BW) || defined(TOF_SEQ_20MHz_BW_512IFFT)
	else if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
			memcpy(tofi->tof_ucode_dlys_us, K_TOF_UCODE_DLYS_US_2G_20MHZ_4360,
				sizeof(uint16)*K_TOF_SEQ_LOG3_N_20MHZ);
		} else {
			if (tofi->flag_sec_2_0) {
				PHY_ERROR(("%s: SECURE RANGING 2.0 params", __FUNCTION__));
				memcpy(tofi->tof_ucode_dlys_us, K_TOF_UCODE_DLYS_US_2G_20MHZ_2_0,
					sizeof(uint16)* K_TOF_SEQ_LOG3_N_20MHZ);
			} else {
				PHY_ERROR(("%s: SECURE RANGING 1.0 params", __FUNCTION__));
				memcpy(tofi->tof_ucode_dlys_us, k_tof_ucode_dlys_us_2g_20MHz,
					sizeof(uint16)* K_TOF_SEQ_LOG3_N_20MHZ);
			}
		}
		tofi->tof_ucode_dlys_us[0][3] += TOF_W_ADJ_EMU(tofi->emu_delay, tofi->tof_sc_FS);
		tofi->tof_ucode_dlys_us[0][4] += TOF_W_ADJ_EMU(tofi->emu_delay, tofi->tof_sc_FS);
		tofi->tof_ucode_dlys_us[1][4] += TOF_W_ADJ_EMU(tofi->emu_delay, tofi->tof_sc_FS);
		/* extent the Rx mode for 8 usec if emulator box is used */
		if (TOF_EB(tofi->emu_delay)) {
		  tofi->tof_ucode_dlys_us[0][1] += TOF_RX_MODE_EXT;
		  tofi->tof_ucode_dlys_us[1][2] += TOF_RX_MODE_EXT;
		}
		return BCME_OK;
	}
	else {
		memcpy(tofi->tof_ucode_dlys_us, k_tof_ucode_dlys_us_2g_20MHz, sizeof(uint16)*10);
		tofi->tof_ucode_dlys_us[0][3] += TOF_W_ADJ_EMU(tofi->emu_delay, tofi->tof_sc_FS);
		tofi->tof_ucode_dlys_us[0][4] += TOF_W_ADJ_EMU(tofi->emu_delay, tofi->tof_sc_FS);
		tofi->tof_ucode_dlys_us[1][4] += TOF_W_ADJ_EMU(tofi->emu_delay, tofi->tof_sc_FS);
		/* extend the Rx mode for 8 usec if emulator box is used */
		if (TOF_EB(tofi->emu_delay)) {
		  tofi->tof_ucode_dlys_us[0][1] += TOF_RX_MODE_EXT;
		  tofi->tof_ucode_dlys_us[1][2] += TOF_RX_MODE_EXT;
		}
		return BCME_OK;

	}
#endif /* TOF_SEQ_20MHz_BW || TOF_SEQ_20MHz_BW_512IFFT */

	return BCME_ERROR;
}

static void
BCMATTACHFN(phy_ac_nvram_proxd_read)(phy_info_t *pi, phy_ac_tof_info_t *tofi)
{
	uint8 i;

	if (PHY_GETVAR(pi, rstr_proxd_basekival)) {
		for (i = 0; i < 4; i++) {
			tofi->proxd_ki[i] = (uint16)PHY_GETINTVAR_ARRAY(pi,
				rstr_proxd_basekival, i);
		}
	} else {
		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			/* For 4345 B0/B1 */
			tofi->proxd_ki[0] = TOF_INITIATOR_K_4345_80M;
			tofi->proxd_ki[1] = TOF_INITIATOR_K_4345_40M;
			tofi->proxd_ki[2] = TOF_INITIATOR_K_4345_20M;
			tofi->proxd_ki[3] = TOF_INITIATOR_K_4345_2G;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_1(pi) ||
			ACMINORREV_4(pi))) {
			/* For 4350 C0/C1/C2 */
			tofi->proxd_ki[0] = TOF_INITIATOR_K_4350_80M;
			tofi->proxd_ki[1] = TOF_INITIATOR_K_4350_40M;
			tofi->proxd_ki[2] = TOF_INITIATOR_K_4350_20M;
			tofi->proxd_ki[3] = TOF_INITIATOR_K_4350_2G;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_3(pi) ||
			ACMINORREV_5(pi))) {
			/* For 4354A1/4345A2(4356) */
			tofi->proxd_ki[0] = TOF_INITIATOR_K_4354_80M;
			tofi->proxd_ki[1] = TOF_INITIATOR_K_4354_40M;
			tofi->proxd_ki[2] = TOF_INITIATOR_K_4354_20M;
			tofi->proxd_ki[3] = TOF_INITIATOR_K_4354_2G;
		} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* For 4349/4355/4359 */
			tofi->proxd_ki[0] = TOF_INITIATOR_K_4349_80M;
			tofi->proxd_ki[1] = TOF_INITIATOR_K_4349_40M;
			tofi->proxd_ki[2] = TOF_INITIATOR_K_4349_20M;
			tofi->proxd_ki[3] = TOF_INITIATOR_K_4349_2G;
		} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			/* For 43602 */
			tofi->proxd_ki[0] = TOF_INITIATOR_K_43602_80M;
			tofi->proxd_ki[1] = TOF_INITIATOR_K_43602_40M;
			tofi->proxd_ki[2] = TOF_INITIATOR_K_43602_20M;
			tofi->proxd_ki[3] = TOF_INITIATOR_K_43602_2G;
		} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			tofi->proxd_ki[0] = TOF_INITIATOR_K_4361_80M;
			tofi->proxd_ki[1] = TOF_INITIATOR_K_4361_40M;
			tofi->proxd_ki[2] = TOF_INITIATOR_K_4361_20M;
			tofi->proxd_ki[3] = TOF_INITIATOR_K_4361_2G;
		}
	}

	if (PHY_GETVAR(pi, rstr_proxd_basektval)) {
		for (i = 0; i < 4; i++) {
			tofi->proxd_kt[i] = (uint16)PHY_GETINTVAR_ARRAY(pi,
				rstr_proxd_basektval, i);
		}
	} else {
		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			/* For 4345 B0/B1 */
			tofi->proxd_kt[0] = TOF_TARGET_K_4345_80M;
			tofi->proxd_kt[1] = TOF_TARGET_K_4345_40M;
			tofi->proxd_kt[2] = TOF_TARGET_K_4345_20M;
			tofi->proxd_kt[3] = TOF_TARGET_K_4345_2G;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_1(pi) ||
			ACMINORREV_4(pi))) {
			/* For 4350 C0/C1/C2 */
			tofi->proxd_kt[0] = TOF_TARGET_K_4350_80M;
			tofi->proxd_kt[1] = TOF_TARGET_K_4350_40M;
			tofi->proxd_kt[2] = TOF_TARGET_K_4350_20M;
			tofi->proxd_kt[3] = TOF_TARGET_K_4350_2G;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_3(pi) ||
			ACMINORREV_5(pi))) {
			/* For 4354A1/4345A2(4356) */
			tofi->proxd_kt[0] = TOF_TARGET_K_4354_80M;
			tofi->proxd_kt[1] = TOF_TARGET_K_4354_40M;
			tofi->proxd_kt[2] = TOF_TARGET_K_4354_20M;
			tofi->proxd_kt[3] = TOF_TARGET_K_4354_2G;
		} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* For 4349/4355/4359 */
			tofi->proxd_kt[0] = TOF_TARGET_K_4349_80M;
			tofi->proxd_kt[1] = TOF_TARGET_K_4349_40M;
			tofi->proxd_kt[2] = TOF_TARGET_K_4349_20M;
			tofi->proxd_kt[3] = TOF_TARGET_K_4349_2G;
		} else if (ACMAJORREV_5(pi->pubpi->phy_rev) || ACMAJORREV_0(pi->pubpi->phy_rev)) {
			/* For 43602 */
			tofi->proxd_kt[0] = TOF_TARGET_K_43602_80M;
			tofi->proxd_kt[1] = TOF_TARGET_K_43602_40M;
			tofi->proxd_kt[2] = TOF_TARGET_K_43602_20M;
			tofi->proxd_kt[3] = TOF_TARGET_K_43602_2G;
		} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			tofi->proxd_kt[0] = TOF_TARGET_K_4361_80M;
			tofi->proxd_kt[1] = TOF_TARGET_K_4361_40M;
			tofi->proxd_kt[2] = TOF_TARGET_K_4361_20M;
			tofi->proxd_kt[3] = TOF_TARGET_K_4361_2G;
		}
	}

	if (PHY_GETVAR(pi, rstr_proxd_80mkval)) {
		for (i = 0; i < 6; i++) {
			tofi->proxd_80m_k_values[i] = (uint16)PHY_GETINTVAR_ARRAY(pi,
				rstr_proxd_80mkval, i);
		}
	} else {
		uint16 const *kvalueptr;

		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			/* For 4345 B0/B1 */
			kvalueptr = proxd_4345_80m_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) || ACMINORREV_4(pi))) {
			/* For 4350 C0/C1/C2 */
			kvalueptr = proxd_4350_80m_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_3(pi) || ACMINORREV_5(pi))) {
			/* For 4354A1/4345A2(4356) */
			kvalueptr = proxd_4354_80m_k_values;
		} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* For 4349/4355/4359 */
			kvalueptr = proxd_4349_80m_k_values;
		} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			/* For 43602 */
			kvalueptr = proxd_43602_80m_k_values;
		} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			kvalueptr = proxd_4361_80m_k_values;
		} else {
			return;
		}
		for (i = 0; i < 6; i++) {
			tofi->proxd_80m_k_values[i] = kvalueptr[i];
		}
	}

	if (PHY_GETVAR(pi, rstr_proxd_40mkval)) {
		for (i = 0; i < 12; i++) {
			tofi->proxd_40m_k_values[i] = (uint16)PHY_GETINTVAR_ARRAY(pi,
				rstr_proxd_40mkval, i);
		}
	} else {
		uint16 const *kvalueptr;

		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			/* For 4345 B0/B1 */
			kvalueptr = proxd_4345_40m_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) || ACMINORREV_4(pi))) {
			/* For 4350 C0/C1/C2 */
			kvalueptr = proxd_4350_40m_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_3(pi) || ACMINORREV_5(pi))) {
			/* For 4354A1/4345A2(4356) */
			kvalueptr = proxd_4354_40m_k_values;
		} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* For 4349/4355/4359 */
			kvalueptr = proxd_4349_40m_k_values;
		} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			/* For 43602 */
			kvalueptr = proxd_43602_40m_k_values;
		} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			kvalueptr = proxd_4361_40m_k_values;
		} else {
			return;
		}
		for (i = 0; i < 12; i++) {
			tofi->proxd_40m_k_values[i] = kvalueptr[i];
		}
	}

	if (PHY_GETVAR(pi, rstr_proxd_20mkval)) {
		for (i = 0; i < 25; i++) {
			tofi->proxd_20m_k_values[i] = (uint16)PHY_GETINTVAR_ARRAY(pi,
				rstr_proxd_20mkval, i);
		}
	} else {
		uint16 const *kvalueptr;

		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			/* For 4345 B0/B1 */
			kvalueptr = proxd_4345_20m_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) || ACMINORREV_4(pi))) {
			/* For 4350 C0/C1/C2 */
			kvalueptr = proxd_4350_20m_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_3(pi) || ACMINORREV_5(pi))) {
			/* For 4354A1/4345A2(4356) */
			kvalueptr = proxd_4354_20m_k_values;
		} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* For 4349/4355/4359 */
			kvalueptr = proxd_4349_20m_k_values;
		} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			/* For 43602 */
			kvalueptr = proxd_43602_20m_k_values;
		} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			kvalueptr = proxd_4361_20m_k_values;
		} else {
			return;
		}
		for (i = 0; i < 25; i++) {
			tofi->proxd_20m_k_values[i] = kvalueptr[i];
		}
	}

	if (PHY_GETVAR(pi, rstr_proxd_2gkval)) {
		for (i = 0; i < 14; i++) {
			tofi->proxd_2g_k_values[i] = (uint16)PHY_GETINTVAR_ARRAY(pi,
				rstr_proxd_2gkval, i);
		}
	} else {
		uint16 const *kvalueptr;
		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			/* For 4345 B0/B1 */
			kvalueptr = proxd_4345_2g_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) || ACMINORREV_4(pi))) {
			/* For 4350 C0/C1/C2 */
			kvalueptr = proxd_4350_2g_k_values;
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_3(pi) || ACMINORREV_5(pi))) {
			/* For 4354A1/4345A2(4356) */
			kvalueptr = proxd_4354_2g_k_values;
		} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* For 4349/4355/4359 */
			kvalueptr = proxd_4349_2g_k_values;
		} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			/* For 43602 */
			kvalueptr = proxd_43602_2g_k_values;
		} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			kvalueptr = proxd_4361_2g_k_values;
		} else {
			return;
		}
		for (i = 0; i < 14; i++) {
			tofi->proxd_2g_k_values[i] = kvalueptr[i];
		}
	}

	for (i = 0; i < sizeof(proxdi_rate_offset_80m) / sizeof(int16); i++) {
		tofi->proxdi_rate_offset_80m[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdi_rate80m, i, proxdi_rate_offset_80m[i]);
	}

	for (i = 0; i < sizeof(proxdi_rate_offset_40m) / sizeof(int16); i++) {
		tofi->proxdi_rate_offset_40m[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdi_rate40m, i, proxdi_rate_offset_40m[i]);
	}

	for (i = 0; i < sizeof(proxdi_rate_offset_20m) / sizeof(int16); i++) {
		tofi->proxdi_rate_offset_20m[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdi_rate20m, i, proxdi_rate_offset_20m[i]);
	}

	for (i = 0; i < sizeof(proxdi_rate_offset_2g) / sizeof(int16); i++) {
		tofi->proxdi_rate_offset_2g[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdi_rate2g, i, proxdi_rate_offset_2g[i]);
	}

	for (i = 0; i < sizeof(proxdt_rate_offset_80m) / sizeof(int16); i++) {
		tofi->proxdt_rate_offset_80m[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdt_rate80m, i, proxdt_rate_offset_80m[i]);
	}

	for (i = 0; i < sizeof(proxdt_rate_offset_40m) / sizeof(int16); i++) {
		tofi->proxdt_rate_offset_40m[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdt_rate40m, i, proxdt_rate_offset_40m[i]);
	}

	for (i = 0; i < sizeof(proxdt_rate_offset_20m) / sizeof(int16); i++) {
		tofi->proxdt_rate_offset_20m[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdt_rate20m, i, proxdt_rate_offset_20m[i]);
	}

	for (i = 0; i < sizeof(proxdt_rate_offset_2g) / sizeof(int16); i++) {
		tofi->proxdt_rate_offset_2g[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdt_rate2g, i, proxdt_rate_offset_2g[i]);
	}

	for (i = 0; i < sizeof(proxdi_ack_offset) / sizeof(int16); i++) {
		tofi->proxdi_ack_offset[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdi_ack, i, proxdi_ack_offset[i]);
	}

	for (i = 0; i < sizeof(proxdt_ack_offset) / sizeof(int16); i++) {
		tofi->proxdt_ack_offset[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxdt_ack, i, proxdt_ack_offset[i]);
	}

	for (i = 0; i < 5; i++) {
		tofi->proxd_subbw_offset[0][i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxd_sub80m40m, i, proxd_subbw_offset[0][i]);
	}

	for (i = 0; i < 5; i++) {
		tofi->proxd_subbw_offset[1][i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxd_sub80m20m, i, proxd_subbw_offset[1][i]);
	}

	for (i = 0; i < 5; i++) {
		tofi->proxd_subbw_offset[2][i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
			rstr_proxd_sub40m20m, i, proxd_subbw_offset[2][i]);
	}

#ifdef TOF_TUNE_KVAL
	if (PHY_GETVAR(pi, rstr_proxd_seq_kval)) {
		int16 kval;
		/* 5G-80MHz K Value */
		kval = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_proxd_seq_kval, 0);
		tofi->proxd_seq_kval[TOF_SEQ_80MHz_IDX] = (kval >= 0) ? kval :
			(ACMAJORREV_4(pi->pubpi->phy_rev) ?
			TOF_SEQ_K_4355_5G_80MHz : 0);
		/* 5G-40MHz K value - not tuned */
		tofi->proxd_seq_kval[TOF_SEQ_40MHz_IDX] = 0;
		/* 5G-20MHz K value */
		kval = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_proxd_seq_kval, 2);
		tofi->proxd_seq_kval[TOF_SEQ_20MHz_IDX] = (kval >= 0) ? kval :
			(ACMAJORREV_4(pi->pubpi->phy_rev) ?
			TOF_SEQ_K_4355_5G_20MHz : 0);
		/* 2G-20MHz K value */
		kval = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_proxd_seq_kval, 3);
		tofi->proxd_seq_kval[TOF_SEQ_2G_20MHz_IDX] = (kval >= 0) ? kval :
			(ACMAJORREV_4(pi->pubpi->phy_rev) ?
			TOF_SEQ_K_4355_2G_20MHz : 0);
	} else {
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* 4355b3 case */
			tofi->proxd_seq_kval[TOF_SEQ_80MHz_IDX] = TOF_SEQ_K_4355_5G_80MHz;
			tofi->proxd_seq_kval[TOF_SEQ_40MHz_IDX] = 0;
			tofi->proxd_seq_kval[TOF_SEQ_20MHz_IDX] = TOF_SEQ_K_4355_5G_20MHz;
			tofi->proxd_seq_kval[TOF_SEQ_2G_20MHz_IDX] = TOF_SEQ_K_4355_2G_20MHz;
		}
	}
#endif /* TOF_TUNE_KVAL */
}

int
phy_ac_tof_seq_params_get_set(phy_type_tof_ctx_t *ctx, uint8 *delays,
	bool set, bool tx, int size)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	uint8 *tmp = delays;
	int i;
	int ret = BCME_OK;

	if (tofi->tof_ucode_dlys_us[0][0] == 0) {
	  if ((ret = phy_ac_tof_seq_params(ctx, TRUE)) != BCME_OK) {
			PHY_ERROR(("%s: phy_tof_seq_params_get_set() returned err %d\n",
				__FUNCTION__, ret));
			return ret;
		}
	}
	if (size < TOF_UCODE_DLYS_MIN_LEN)
		return BCME_BADLEN;

	if (!set) {
		for (i = 0; i < TOF_UCODE_DLYS_MIN_LEN; i++) {
			*tmp = tofi->tof_ucode_dlys_us[(tx ? 1 : 0)][i];
			tmp++;
		}
		return BCME_OK;
	} else {
		/* NEED TO ADD LOGIC to Update the Params Received from the Remote */
		return BCME_OK;
	}
}

static void
phy_ac_tof_init_gdmm_th(phy_type_tof_ctx_t *ctx, int32 *gdmm_th)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	if (
		ACMAJORREV_2(pi->pubpi->phy_rev) &&
		(ACMINORREV_1(pi) || ACMINORREV_4(pi))) {
		*gdmm_th = K_TOF_GDMM_TH_4350;
	} else if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		*gdmm_th = K_TOF_GDMM_TH_4360;
	} else {
		*gdmm_th = K_TOF_GDMM_TH_43602;
	}
}

static void
phy_ac_tof_init_gdv_th(phy_type_tof_ctx_t *ctx, uint32 *gdv_th)
{
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)ctx;
	phy_info_t *pi = tofi->pi;

	if (
		ACMAJORREV_2(pi->pubpi->phy_rev) &&
		(ACMINORREV_1(pi) || ACMINORREV_4(pi))) {
		*gdv_th = K_TOF_GDV_TH_4350;
	} else {
		*gdv_th = K_TOF_GDV_TH_4360_43602;
	}
}
void phy_ac_tof_gen_scrambled_output(const uint8* In1, const uint8* In2, const uint16 len,
	uint8 *Out, bool flag_sec_2_0)
{
	BCM_REFERENCE(*In2);
	uint16 mask4 = 0x8, mask7 = 0x40, Rshift4 = 3, Rshift7 = 6, i = 0, j = 0, imax;
	uint8 byte_in, byte_out;
	uint8 state_mask = 0x7f, mask1 = 0x1, bit, x7 = 0, x4 = 0;
	uint8 state;
	if (flag_sec_2_0) {
		state = *In1 & state_mask;
	} else {
		state = (*In2 & (state_mask << 1)) >> 1;
	}
	imax = (len / 8) + 1;
	for (i = 0; i < imax; i++) {
		byte_in = *(In1 + i);
		byte_out = 0;
		for (j = 0; j < 8; j++) {
			if ((i*8 + j) >= len) {
				break;
			}
			x7 = (state & mask7) >> Rshift7;
			x4 = (state & mask4) >> Rshift4;
			bit = byte_in & mask1;
			PHY_INFORM(("state = %x, bit = %x, ",  state, bit));
			bit = bit ^ (x7 ^ x4);
			PHY_INFORM(("bit_out = %x\n", bit));
			byte_out = byte_out | (bit << j);
			state = (state << 1) & state_mask;
			state = state | (x7^x4);
			byte_in = byte_in >> 1;
		}
		*(Out + i) = byte_out;
	}
}

void
phy_ac_tof_bytes_to_spb(const uint8* In, const uint16 len, uint32* Out, bool flag_sec_2_0)
{
	/*
	* Assumes a 52 bit incoming sequence and maps it to 20MHz LTF SPB
	*/
	uint32 tmp[8] = { 0 };
	uint32 out = 0;
	uint8 in = *(In), nibble, bit;
	uint16 i = 0, j = 0, k = 0, idx, shift_ctr = 0;
	const uint16 nibble_shift = 4;
	const uint8 bits_in_byte = 8;
	uint8 pair;

	uint16 ltf_neg_mask;
	uint16 ltf_pos_mask;

	if (flag_sec_2_0) {
		ltf_neg_mask = K_TOF_LTF_MASK_NEG_2_0;
		ltf_pos_mask = K_TOF_LTF_MASK_POS_2_0;
	} else {
		ltf_neg_mask = K_TOF_LTF_MASK_NEG;
		ltf_pos_mask = K_TOF_LTF_MASK_POS;
	}

	PHY_INFORM(("%s ", __FUNCTION__));
	for (i = 0; i < 16; i++) {
		PHY_INFORM(("0x%2x ", *(In + i)));
	}
	PHY_INFORM(("\n"));

	for (i = 0; i < (2 * K_TOF_SEQ_SPB_LEN_20MHZ); i++) {
		out = 0;
		for (j = 0; j < bits_in_byte; j++) {
			idx = i*bits_in_byte + j;
			if ((idx == 0) || ((idx <= ltf_neg_mask) &&
				(idx >= ltf_pos_mask))) {
				nibble = 0x0;
			} else {
				if (shift_ctr == 0) {
					in = *(In + k);
					k++;
				}
				if (flag_sec_2_0) {
					pair = in & 0x3;
					switch (pair) {
					case 0x1:
						nibble = 0x1;
						break;
					case 0x2:
						nibble = 0xf;
						break;
					case 0x3:
						nibble = 0x2;
						break;
					case 0x0:
					default:
						nibble = 0xe;
					}
					in = (in >> 2);
					shift_ctr += 2;
				} else {
					bit = in & 0x1;
					nibble = (bit == 0x0 ? 0xf : 0x1);
					in = (in >> 1);
					shift_ctr++;
				}
				shift_ctr = shift_ctr % bits_in_byte;
			}
			out = out | (nibble << (nibble_shift*j));
		}
		*(tmp + i) = 0xffffffff & out;
	}
	j = (len == 2 * K_TOF_SEQ_SPB_LEN_20MHZ) ? 0 : 4;
	i = 0;
	while (i < len) {
		if (!flag_sec_2_0) {
			if ((len == 2 * K_TOF_SEQ_SPB_LEN_80MHZ) && (i < K_TOF_SEQ_SPB_LEN_80MHZ)) {
				Out[i] = (tmp[j] << 1) & 0xeeeeeeee;
			} else
				Out[i] = tmp[j];
		} else {
			Out[i] = tmp[j];
			PHY_INFORM(("0x%x\n", Out[i]));
		}
		j = j + 1;
		j = j % (2 * K_TOF_SEQ_SPB_LEN_20MHZ);
		i++;
	}
}

void phy_ac_tof_set_setup_done(phy_ac_tof_info_t *tofi, bool done)
{
	tofi->tof_setup_done = done;
}

uint16 phy_ac_tof_get_rfseq_bundle_offset(phy_ac_tof_info_t *tofi)
{
	return tofi->tof_rfseq_bundle_offset;
}

#endif /* WL_PROXDETECT */
