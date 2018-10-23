/*
 * ACPHY TOF module interface
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
 * $Id: phy_ac_tof.h 691134 2017-03-20 23:16:15Z $
 */
#ifndef _phy_ac_tof_h_
#define _phy_ac_tof_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_tof.h>

#ifndef INT32_MAX
#define INT32_MAX	0x7fffffff
#endif /* INT32_MAX */

#ifndef INT32_MIN
#define INT32_MIN	((int32)0x80000000)
#endif /* INT32_MIN */

#ifndef INT8_MIN
#define INT8_MIN	((int8)0x80)
#endif /* INT8_MIN */

#ifndef UINT32_MAX
#define UINT32_MAX	0xffffffff
#endif /* UINT32_MAX */

#ifndef UINT32_MIN
#define UINT32_MIN	0x0
#endif /* UINT32_MIN */

#define K_TOF_IS_TARGET_FLAG 0x4
#define K_TOF_TARGET_TRIG_DLY 2211 /* 12320 - 273us, 15008 - 290us */
#define K_TOF_INI_TRIG_DLY 2514 /* 18112 - 273us from Target, 20768 - 290us from target */
#define K_TOF_SYM_ARR_LEN 6
#define TOF_GPT_TIMER_RES_MULT 8
#define TOF_US_TO_TEN_US 10
#define TOF_DEFAULT_START_SEQ_TIME 273
#define TOF_DEFAULT_DELTA_TIME_TX2RX 40

#define K_TOF_LOAD_SPB_ONLY
#define K_TOF_BITFLIP_TH_DEFAULT	10
#define K_TOF_SNR_TH_DEFAULT		0
#define K_TOF_GDMM_TH_43602		INT32_MAX
#define K_TOF_GDMM_TH_4360		INT32_MAX
#define K_TOF_GDMM_TH_4350		INT32_MAX
#define K_TOF_GDV_TH_4360_43602		UINT32_MAX
#define K_TOF_GDV_TH_4350		UINT32_MAX

#define K_TOF_LTF_MASK_NEG 37
#define K_TOF_LTF_MASK_POS 27
#define K_TOF_SEQ_SPB_LEN_MAX 8

#define K_TOF_LTF_MASK_NEG_2_0 35
#define K_TOF_LTF_MASK_POS_2_0 29

#define K_TOF_SHM_ARR_LENGTH 25
#define K_TOF_SNR_FP_PREC (4)
#define K_TOF_TWDL_SFT (8)
#define K_TOF_NUM_LEGACY_NZ_SC_20M 50
#define K_TOF_NUM_LEGACY_BL_20M 2 /* 10 MHz sub bands */

#define K_TOF_NUM_LEGACY_NZ_SC_80M 98
#define K_TOF_NUM_LEGACY_BL_80M 8 /* 10 MHz sub bands */

#define K_TOF_CHAN_LENGTH_20M 64
/*
* In 80 MHz time domain, Second half of the OFDM symbol is equal to first half.
* So in frequency domain, channel estimate only on even numbered subcarriers is desired.
* As data is 2X oversampled in time domain, 80MHz frequency domain will have 2x number
* of subcarriers with 65 to 192 being zeros.
*/
#define K_TOF_HALF_CHAN_LENGTH_80M (K_TOF_CHAN_LENGTH_20M * 2)
#define K_TOF_HALF_CHAN_LENGTH_2X_OS_80M (K_TOF_HALF_CHAN_LENGTH_80M * 2)

#ifdef WL_PROXD_SEQ
#define K_TOF_FILT_1_MASK   0x1
#define K_TOF_FILT_NEG_MASK 0xc
#define K_TOF_FILT_NON_ZERO_MASK 0x3

#ifdef TOF_SEQ_20MHz_BW_512IFFT
#define K_TOF_SEQ_FFT_20MHZ 64
#define K_TOF_SEQ_N_20MHZ 9
#define K_TOF_SEQ_IFFT_20MHZ (1 << K_TOF_SEQ_N_20MHZ)
#endif /* TOF_SEQ_20MHz_BW_512IFFT */

#define K_TOF_RFSEQ_TINY_BUNDLE_BASE 8
#define K_TOF_SEQ_TINY_RX_FEM_GAIN_OFFSET 0x29
#define K_TOF_SEQ_TINY_TX_FEM_GAIN_OFFSET 0x2b
#endif /* WL_PROXD_SEQ */

#define TOF_INITIATOR_K_4345_80M	34434 /* initiator K value for 80M */
#define TOF_TARGET_K_4345_80M		34474 /* target K value for 80M */
#define TOF_INITIATOR_K_4345_40M	35214 /* initiator K value for 40M */
#define TOF_TARGET_K_4345_40M		35214 /* target K value for 40M */
#define TOF_INITIATOR_K_4345_20M	36553 /* initiator K value for 20M */
#define TOF_TARGET_K_4345_20M		36553 /* target K value for 20M */
#define TOF_INITIATOR_K_4345_2G		37169 /* initiator K value for 2G */
#define TOF_TARGET_K_4345_2G		37169 /* target K value for 2G */

#define TOF_INITIATOR_K_4350_80M	36546 /* initiator K value for 80M */
#define TOF_TARGET_K_4350_80M		36569 /* target K value for 80M */
#define TOF_INITIATOR_K_4350_40M	35713 /* initiator K value for 40M */
#define TOF_TARGET_K_4350_40M		35713 /* target K value for 40M */
#define TOF_INITIATOR_K_4350_20M	37733 /* initiator K value for 20M */
#define TOF_TARGET_K_4350_20M		37733 /* target K value for 20M */
#define TOF_INITIATOR_K_4350_2G		37733 /* initiator K value for 2G */
#define TOF_TARGET_K_4350_2G		37733 /* target K value for 2G */
#define TOF_SEQ_K_4350_2G               129   /* Seq offset for 2G */
#define TOF_SEQ_K_43602_2G		140 /*  Seq offset for 2G for 43602 */
#define TOF_SEQ_K_43602_80MHz		90 /*  Seq offset for 5G-80MHz for 43602 */
#define TOF_SEQ_K_4360_80MHz		90 /* Seq offset for 5G-80MHz for 4360 */
#define TOF_SEQ_K_4360_2G		114 /* Seq offset for 2G for 4360 */

#define TOF_INITIATOR_K_4354_80M	36553 /* initiator K value for 80M */
#define TOF_TARGET_K_4354_80M		36559 /* target K value for 80M */
#define TOF_INITIATOR_K_4354_40M	35699 /* initiator K value for 40M */
#define TOF_TARGET_K_4354_40M		35713 /* target K value for 40M */
#define TOF_INITIATOR_K_4354_20M	37723 /* initiator K value for 20M */
#define TOF_TARGET_K_4354_20M		37728 /* target K value for 20M */
#define TOF_INITIATOR_K_4354_2G		37816 /* initiator K value for 2G */
#define TOF_TARGET_K_4354_2G		37816 /* target K value for 2G */

#define TOF_INITIATOR_K_4349_80M	35984 /* initiator K value for 80M */
#define TOF_TARGET_K_4349_80M		35982 /* target K value for 80M */
#define TOF_INITIATOR_K_4349_40M	35512 /* initiator K value for 40M */
#define TOF_TARGET_K_4349_40M		35513 /* target K value for 40M */
#define TOF_INITIATOR_K_4349_20M	37466 /* initiator K value for 20M */
#define TOF_TARGET_K_4349_20M		37470 /* target K value for 20M */
#define TOF_INITIATOR_K_4349_2G		37550 /* initiator K value for 2G */
#define TOF_TARGET_K_4349_2G		37550 /* initiator K value for 2G */

#define TOF_INITIATOR_K_43602_80M      33836 /* initiator K value for 80M */
#define TOF_TARGET_K_43602_80M         33836 /* target K value for 80M */
#define TOF_INITIATOR_K_43602_40M      35450 /* initiator K value for 40M */
#define TOF_TARGET_K_43602_40M         35475 /* target K value for 40M */
#define TOF_INITIATOR_K_43602_20M      36000 /* initiator K value for 20M */
#define TOF_TARGET_K_43602_20M         35936 /* target K value for 20M */
#define TOF_INITIATOR_K_43602_2G       36390 /* initiator K value for 2G */
#define TOF_TARGET_K_43602_2G          36390 /* target K value for 2G */

#define TOF_SEQ_K_4355_5G_80MHz			0 /* Seq offset for 5G-80 for 4355 */
#define TOF_SEQ_K_4355_5G_80MHz			0 /* Seq offset for 5G-80 for 4355 */
#define TOF_SEQ_K_4355_5G_80MHz			0 /* Seq offset for 5G-80 for 4355 */

#define TOF_SEQ_K_4355_5G_80MHz			0 /* Seq offset for 5G-80 for 4355 */

#define K_TOF_SEQ_RX_GAIN_TINY ((0 << 13) | (0 << 10) | (5 << 6) | (0 << 3) | (4 << 0))
#define K_TOF_SEQ_RX_LOOPBACK_GAIN_TINY ((0 << 13) | (0 << 10) | (3 << 6) | (2 << 3) | (3 << 0))
#define K_TOF_SEQ_RX_LOOPBACK_GAIN_TINY_2G ((0 << 13) | (0 << 10) | (5 << 6) | (2 << 3) | (3 << 0))

#define K_TOF_RFSEQ_BUNDLE_BASE 8
#define K_TOF_SEQ_RX_FEM_GAIN_OFFSET 0x29
#define K_TOF_SEQ_TX_FEM_GAIN_OFFSET 0x2b

#if defined(WL_PROXD_SEQ)

#define K_TOF_SEQ_RX_GAIN ((0 << 13) | (2 << 10) | (2 << 6) | (6 << 3) | (4 << 0))
#define K_TOF_SEQ_RX_LOOPBACK_GAIN ((1 << 13) | (1 << 10) | (3 << 6) | (2 << 3) | (2 << 0))
#define K_TOF_SEQ_RX_LOOPBACK_GAIN_2G ((2 << 13) | (2 << 10) | (3 << 6) | (2 << 3) | (2 << 0))
#define K_TOF_SEQ_RX_LOOPBACK_GAIN_2G_43602 ((1 << 13) | (1 << 10) | (3 << 6) | (3 << 3) | (3 << 0))
#define K_TOF_SEQ_RX_LOOPBACK_GAIN_2G_4360 ((2 << 13) | (1 << 10) | (3 << 6) | (3 << 3) | (3 << 0))
#define K_TOF_SEQ_RX_LOOPBACK_GAIN_2G_4364 ((1 << 13) | (1 << 10) | (1 << 6) | (1 << 3) | (2 << 0))

#define K_TOF_SEQ_RFSEQ_GAIN_BASE 0x1d0
#define K_TOF_SEQ_RFSEQ_RX_GAIN_OFFSET 7
#define K_TOF_SEQ_RFSEQ_LOOPBACK_GAIN_OFFSET  4

#define K_TOF_SEQ_SHM_OFFSET 4
#define K_TOF_SEQ_SHM_SETUP_REGS_OFFSET 2 * (0 + K_TOF_SEQ_SHM_OFFSET)
#define K_TOF_SEQ_SHM_SETUP_REGS_LEN    15
#define K_TOF_SEQ_SHM_SETUP_VALS_OFFSET 2 * (15 + K_TOF_SEQ_SHM_OFFSET)
#define K_TOF_SEQ_SHM_SETUP_VALS_LEN    16
#define K_TOF_SET_SHM_RESTR_REGS_OFFSET 2 * (7 + K_TOF_SEQ_SHM_OFFSET)
#define K_TOF_SET_SHM_RESTR_VALS_OFFSET 2 * (31 + K_TOF_SEQ_SHM_OFFSET)
#define K_TOF_SET_SHM_RESTR_VALS_LEN    8
#define K_TOF_SEQ_SHM_FEM_RADIO_HI_GAIN_OFFSET 2 * (18 + K_TOF_SEQ_SHM_OFFSET)
#define K_TOF_SEQ_SHM_FEM_RADIO_LO_GAIN_OFFSET 2 * (39 + K_TOF_SEQ_SHM_OFFSET)
#define K_TOF_SEQ_SHM_DLY_OFFSET 2 * (40 + K_TOF_SEQ_SHM_OFFSET)
#define K_TOF_SEQ_SHM_DLY_LEN    (3 * 2)

#define TOF_RFSEQEXT_ADC_20MHz_OFFS 6
#define TOF_RFSEQEXT_ADC_40MHz_OFFS 7
#define TOF_RFSEQEXT_ADC_80MHz_OFFS 8

#define TOF_RFSEQEXT_SIZE 60
#define TOF_RFSEQ_BUNDLE_SIZE 48

#define TOF_RFSEQ_AFE_IQADC_FLASH_ONLY_SHIFT 19
#define TOF_RFSEQ_AFE_IQADC_FLASH_ONLY_SZ 1
#define TOF_RFSEQ_AFE_RESET_OV_DET_SHIFT 18
#define TOF_RFSEQ_AFE_RESET_OV_DET_SZ 1
#define TOF_RFSEQ_AFE_IQADC_CLAMP_EN_SHIFT 17
#define TOF_RFSEQ_AFE_IQADC_CLAMP_EN_SZ 1
#define TOF_RFSEQ_AFE_IQADC_RX_DIV4_EN_SHIFT 13
#define TOF_RFSEQ_AFE_IQADC_RX_DIV4_EN_SZ 1
#define TOF_RFSEQ_AFE_IQADC_ADC_BIAS_SHIFT 11
#define TOF_RFSEQ_AFE_IQADC_ADC_BIAS_SZ 3
#define TOF_RFSEQ_AFE_CTRL_FLASH17LVL_SHIFT 10
#define TOF_RFSEQ_AFE_CTRL_FLASH17LVL_SZ 1
#define TOF_RFSEQ_AFE_IQADC_FLASHHSPD_SHIFT 9
#define TOF_RFSEQ_AFE_IQADC_FLASHHSPD_SZ 1
#define TOF_RFSEQ_AFE_IQADC_PWRUP_SHIFT 3
#define TOF_RFSEQ_AFE_IQADC_PWRUP_SZ 0x3f
#define TOF_RFSEQ_AFE_IQADC_MODE_SZ 7

#define TOF_RFBNDL_ADC_OFFSET_38  0x38
#define TOF_RFBNDL_ADC_OFFSET_39  0x39

#define TOF_RFBNDL_ADC_MASK_LB 0xfff0
#define TOF_RFBNDL_ADC_MASK_HB 0x4f

#define TOF_RFBNDL_AFE_IQADC_FLASH_ONLY_SHIFT 3
#define TOF_RFBNDL_AFE_RESET_OV_DET_SHIFT 5
#define TOF_RFBNDL_AFE_IQADC_CLAMP_EN_SHIFT 4
#define TOF_RFBNDL_AFE_IQADC_RX_DIV4_EN_SHIFT 6
#define TOF_RFBNDL_AFE_IQADC_ADC_BIAS_SHIFT 12
#define TOF_RFBNDL_AFE_CTRL_FLASH17LVL_SHIFT 2
#define TOF_RFBNDL_AFE_IQADC_FLASHHSPD_SHIFT 1
#define TOF_RFBNDL_AFE_IQADC_PWRUP_SHIFT 6
#define TOF_RFBNDL_AFE_IQADC_MODE_LB_SHIFT 14
#define TOF_RFBNDL_AFE_IQADC_MODE_HB_SHIFT 2
#define TOF_RFBNDL_AFE_IQADC_MODE_LB_MASK 0x3
#define TOF_RFBNDL_AFE_IQADC_MODE_HB_MASK 0x4

#define K_TOF_SEQ_SC_START 1024
#define K_TOF_SEQ_SC_STOP  (K_TOF_SEQ_SC_START + 6730)
#define K_TOF_SC_MODE_4S_RX_FARROW 4

#define K_TOF_RFSEQ_DC_RUN_EVENT 0x43
#define K_TOF_RFSEQ_EPA_EVENT    0x4
#define K_TOF_RFSEQ_END_EVENT    0x1f
#define K_TOF_RFSEQ_TSSI_EVENT   0x35
#define K_TOF_RFSEQ_IPA_EVENT    0x3
#define K_TOF_RFSEQ_TX_GAIN_EVENT 0x6

#define K_TOF_SEQ_IN_SCALE (1 << 12)
#define K_TOF_SEQ_OUT_SCALE 11
#define K_TOF_SEQ_OUT_SHIFT 8
#define K_TOF_MF_IN_SHIFT  0
#define K_TOF_MF_OUT_SCALE 0
#define K_TOF_MF_OUT_SHIFT 0

#if defined(TOF_DBG)
#define K_TOF_DBG_SC_DELTA 32
#endif // endif

#endif /* WL_PROXD_SEQ */

#define NUM_LEGACY_NZ_SC_20M 50
#define NUM_LEGACY_BL_20M 2 /* 10 MHz sub bands */

#define NUM_LEGACY_NZ_SC_80M 98
#define NUM_LEGACY_BL_80M 8 /* 10 MHz sub bands */
#define K_TOF_SC_FS_80MHZ 160

#if defined(TOF_SEQ_20MHz_BW) || defined(TOF_SEQ_20MHz_BW_512IFFT)
#define K_TOF_SEQ_LOG2_N_20MHZ 7
#define K_TOF_SEQ_LOG3_N_20MHZ 10
#endif // endif

#ifdef TOF_SEQ_40MHz_BW
#define K_TOF_SEQ_LOG2_N_40MHZ 8
#endif // endif

#ifdef TOF_SEQ_40_IN_40MHz
#define K_TOF_SEQ_LOG2_N_40MHZ 8
#endif // endif

#ifdef TOF_SEQ_20_IN_80MHz
#define K_TOF_SEQ_LOG2_N_80MHZ_20 9
#endif // endif

#if !defined(TOF_SEQ_20_IN_80MHz)
#define K_TOF_SEQ_LOG2_N_80MHZ 8
#endif // endif

#if defined(TOF_SEQ_20_IN_80MHz) || defined(TOF_SEQ_20MHz_BW) || \
	defined(TOF_SEQ_40MHz_BW) || defined(TOF_SEQ_20MHz_BW_512IFFT)
#define K_TOF_SEQ_SPB_LEN_20MHZ 4
#endif // endif

#ifdef TOF_SEQ_40_IN_40MHz
#define K_TOF_SEQ_SPB_LEN_40MHZ 8
#endif // endif

#if !defined(TOF_SEQ_20_IN_80MHz)
#define K_TOF_SEQ_SPB_LEN_80MHZ 8
#endif // endif

#define K_TOF_SHM_DUMP_LENGTH 47
#define K_TOF_UNPACK_SGN_MASK (1<<31)
#define K_TOF_K_RTT_ADJ_Q 2
#define TOF_UCODE_DLYS_MIN_LEN 3

#define K_MIN_ACC_TOF_RX_SAMP_PWR 60000
#define K_TOF_RX_SAMP_CHECK_NUM 10

#define K_TOF_RESTORE_HWACI_INTR 2

#define TOF_W(x, n, log2_n) (x - ((1 << log2_n) / 2) + (n * (1 << log2_n)))

#define TOF_PRINT_SAMP(str, seq, max_samp, buf) if (!seq) { \
	int i = 0, j = 0; \
	printf("%s = ", str); \
	for (i = 0, j = 0; i < 8; i++) { \
		printf("(%d, %d), ", buf[j].i, buf[j].q); \
		j++; \
		if (i == 3) { \
			j = max_samp - j - 1; \
		} \
	} \
	printf(";\n"); \
}

/* forward declaration */
typedef struct phy_ac_tof_info phy_ac_tof_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_tof_info_t *phy_ac_tof_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_tof_info_t *ri);
void phy_ac_tof_unregister_impl(phy_ac_tof_info_t *info);

/* Inter-module data api */
/* used by txiqlocal and rxiqcal */
bool phy_ac_tof_is_active(phy_ac_tof_info_t *tofi);
/* used by chanmgr */
bool phy_ac_tof_forced_smth(phy_ac_tof_info_t *tofi);

void phy_ac_tof_bytes_to_spb(const uint8* In, const uint16 len, uint32* Out,
	bool flag_sec_2_0);
void phy_ac_tof_gen_scrambled_output(const uint8* In1,
	const uint8* In2, const uint16 len, uint8* Out,
	bool flag_sec_2_0);

void phy_ac_tof_set_setup_done(phy_ac_tof_info_t *tofi, bool done);
uint16 phy_ac_tof_get_rfseq_bundle_offset(phy_ac_tof_info_t *tofi);

#if defined(WL_PROXDETECT)
void phy_ac_tof_tbl_offset(phy_ac_tof_info_t *tofi, uint32 id, uint32 offset);
#endif /* defined(WL_PROXDETECT) */

#endif /* _phy_ac_tof_h_ */
