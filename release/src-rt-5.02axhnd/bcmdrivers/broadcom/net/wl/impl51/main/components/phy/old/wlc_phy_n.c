/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abgn
 * Networking Device Driver.
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
 * $Id: wlc_phy_n.c 689894 2017-03-13 23:32:18Z $
 */

#include <wlc_cfg.h>

#if NCONF != 0
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcm_math.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_phy_radio.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <802.11.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <phy_btcx.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_utils_reg.h>
#include <phy_utils_pmu.h>
#include <phy_n_btcx.h>
#include <phy_calmgr.h>
#include <phy_rxgcrs_api.h>
#include <phy_chanmgr_api.h>
#include <phy_chanmgr.h>
#include <phy_misc_api.h>
#include <phy_papdcal.h>
#include <phy_n_rssi.h>
#include <phy_n_rxgcrs.h>
#include <phy_n_rxspur.h>

#include <phy_type_cache.h>

#include <wlc_phyreg_n.h>
#include <wlc_phytbl_n.h>
#include <wlc_phy_n.h>
#include <phy_rstr.h>
#include <phy_stf.h>
#include <phy_noise.h>
#include <phy_calmgr_api.h>
#include <phy_noise_api.h>

#include "wlc_phy_extended_n.h"

/* Needed for saverestore functionality support *
 * for hardware VSDB                            *
*/
#ifdef WLSRVSDB
#include <saverestore.h>

enum {
	SRVSDB_RESTORE,
	SRVSDB_SAVE
};
enum {
	ENTER,
	EXIT
};

#endif /* end WLSRVSDB */

typedef struct _nphy_iqcal_params {
	uint16 txlpf;
	uint16 txgm;
	uint16 pga;
	uint16 pad;
	uint16 ipa;
	uint16 cal_gain;
	uint16 cal_gain1;
	uint16 ncorr[5];
} nphy_iqcal_params_t;

typedef struct _nphy_txiqcal_ladder {
	uint8 percent;
	uint8 g_env;
} nphy_txiqcal_ladder_t;

typedef struct {
	nphy_txgains_t gains;
	bool useindex;
	uint8 index;
} nphy_ipa_txcalgains_t;

typedef struct nphy_papd_restore_state_t {
	uint16 fbmix[NPHY_CORE_NUM];
	uint16 vga_master[NPHY_CORE_NUM];
	uint16 intpa_master[NPHY_CORE_NUM];
	uint16 afectrl[NPHY_CORE_NUM];
	uint16 afeoverride[NPHY_CORE_NUM];
	uint16 pwrup[NPHY_CORE_NUM];
	uint16 atten[NPHY_CORE_NUM];
	uint16 mm;
	uint16 tr2g_config1;
	uint16 tr2g_config1_core[NPHY_CORE_NUM];
	uint16 tr2g_config4_core[NPHY_CORE_NUM];
	uint16 reg10;
	uint16 reg20;
	uint16 reg21;
	uint16 reg29;
} nphy_papd_restore_state;

typedef struct _nphy_ipa_txrxgain {
	uint16 hpvga;
	uint16 lpf_biq1;
	uint16 lpf_biq0;
	uint16 lna2;
	uint16 lna1;
	int8 txpwrindex;
} nphy_ipa_txrxgain_t;

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_5GHz[]=
                                     { {0, 0, 0, 0, 0, 100},
                                       {0, 0, 0, 0, 0, 50},
                                       {0, 0, 0, 0, 0, -1},
                                       {0, 0, 0, 3, 0, -1},
                                       {0, 0, 3, 3, 0, -1},
                                       {0, 2, 3, 3, 0, -1}
                                     };

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_2GHz[]=
                                     { {0, 0, 0, 0, 0, 128},
                                       {0, 0, 0, 0, 0, 70},
                                       {0, 0, 0, 0, 0, 20},
                                       {0, 0, 0, 3, 0, 20},
                                       {0, 0, 3, 3, 0, 20},
                                       {0, 2, 3, 3, 0, 20}
                                     };

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_5GHz_rev7[]=
                                     { {0, 0, 0, 0, 0, 100},
                                       {0, 0, 0, 0, 0, 50},
                                       {0, 0, 0, 0, 0, -1},
                                       {0, 0, 0, 3, 0, -1},
                                       {0, 0, 3, 3, 0, -1},
                                       {0, 0, 5, 3, 0, -1}
                                     };

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_2GHz_rev7[]=
                                     { {0, 0, 0, 0, 0, 10},
                                       {0, 0, 0, 1, 0, 10},
                                       {0, 0, 1, 2, 0, 10},
                                       {0, 0, 1, 3, 0, 10},
                                       {0, 0, 4, 3, 0, 10},
                                       {0, 0, 6, 3, 0, 10}
                                     };

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_5GHz_rev19_en[]=
                                     { {0, 0, 0, 0, 0, 100},
                                       {0, 0, 0, 0, 0, 50},
                                       {0, 0, 0, 0, 0, 10},
                                       {0, 0, 6, 0, 0, 10},
                                       {0, 3, 6, 0, 0, 10},
                                       {0, 5, 6, 0, 0, 10}
                                     };

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_5GHz_rev19_dis[]=
                                     { {0, 0, 0, 0, 0, 72},
                                       {0, 0, 0, 0, 0, 72},
                                       {0, 0, 0, 0, 0, 62},
                                       {0, 0, 0, 0, 0, 42},
                                       {0, 0, 0, 1, 0, 42},
                                       {0, 0, 0, 0, 1, 42}
                                     };

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_2GHz_rev19_en[]=
                                     { {0, 0, 0, 0, 0, 70},
                                       {0, 0, 2, 0, 0, 40},
                                       {0, 0, 4, 0, 0, 10},
                                       {0, 0, 6, 0, 0, 10},
                                       {0, 3, 6, 0, 0, 10},
                                       {0, 5, 6, 0, 0, 10}
                                     };

nphy_ipa_txrxgain_t nphy_ipa_rxcal_gaintbl_2GHz_rev19_dis[]=
                                     { {0, 0, 0, 0, 0, 62},
                                       {0, 0, 0, 0, 1, 62},
                                       {0, 0, 0, 0, 1, 52},
                                       {0, 0, 0, 0, 1, 42},
                                       {0, 0, 1, 0, 1, 42},
                                       {0, 1, 1, 0, 1, 42}
                                     };

enum {
	NPHY_RXCAL_GAIN_INIT = 0,
	NPHY_RXCAL_GAIN_UP,
	NPHY_RXCAL_GAIN_DOWN
};

/**
 * The 4322newest filter was designed for improving the spectral mask margins in the 4324B0 iPA
 * chip. It is based on the 4322 filter, the response was shaped such that the outer subcarriers
 * were attenuated.
 */
uint16 NPHY_IPA_REV4_txdigi_filtcoeffs[][NPHY_NUM_DIG_FILT_COEFFS] = {
	/* #0: ofdm20 */
	{-377, 137, -407, 208, -1527, 956, 93, 186, 93,
	230, -44, 230, 201, -191, 201},
	/* #1: ofdm40 for 40MHz */
	{-77, 20, -98, 49, -93, 60, 56, 111, 56, 26, -5,
	26, 34, -32, 34},
	/* #2: cck gauss2.2 */
	{-360, 164, -376, 164, -1533, 576, 308, -314, 308,
	121, -73, 121, 91, 124, 91},
	/* #3: 4322 default 20mhz */
	{-295, 200, -363, 142, -1391, 826, 151, 301, 151,
	151, 301, 151, 602, -752, 602},
	/* #4: 4322 default 40mhz */
	{-92, 58, -96, 49, -104, 44, 17, 35, 17,
	12, 25, 12, 13, 27, 13},
	/* #5: a-band ofdm20 */
	{-375, 136, -399, 209, -1479, 949, 130, 260, 130,
	230, -44, 230, 201, -191, 201},
	/* #6 : Japan channel 14 (CCK) */
	{0xed9, 0xc8, 0xe95, 0x8e, 0xa91, 0x33a, 0x97, 0x12d, 0x97,
	0x97, 0x12d, 0x97, 0x25a, 0xd10, 0x25a},
	/* #7: ofdm40 sharper cut-off. ofdm22 in Tcl. */
	{-77, 20, -97, 47, -96, 59, 62, 122, 62, 20, -4,
	20, 32, -30, 32},
	/* #8: ofdm40 for 53572 / ofdm5 for 40MHz */
	{-77, 20, -98, 49, -93, 60, 76, 0, 0, 38, 24,
	4, 76, -82, 76},
	/* #9: ofdm26 for 43217 */
	{-77, 20, -96, 49, -92, 60, 58, 99, 58,
	29, -18, 29, 58, -63, 58},
	/* #10: ofdm5 for 43217 20HMz */
	{-377, 137, -409, 213, -1517, 964, 93, 186, 93,
	230, -44, 230, 201, -191, 201},
	/* #11: ofdm20 for 43217 */
	{-375, 136, -407, 208, -1527, 956, 93, 186, 93,
	230, -44, 230, 201, -191, 201},
	/* #12: ofdm3 */
	{-307, 82, -389, 189, -1529, 938, 256, 511, 256,
	102, -20, 102, 273, -260, 273},
	/* #13: OFDM26 40 MHz filter */
	{-77, 20, -96, 49, -92, 60, 58, 99, 58, 29, -18,
	29, 58, -63, 58},
	/* #14: OFDM6 40 MHz filter */
	{-93, 60, -77, 20, -98, 49, 51, 68, 51, 51, -18,
	51, 26, -22, 26},
	/* #15: OFDM8 40 MHZ Filter */
	{-93, 60, -77, 20, -98, 49, 49, 68, 49, 49, -39,
	49, 25, -7, 25},
	/* #16: OFDM1 20MHz filter */
	{-377, 137, -404, 189, -1553, 919, 39, 79, 39, 91,
	183, 92, 91, 183, 91}
};
/* %%%%%% channel/radio */

typedef struct nphy_sfo_cfg {
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} nphy_sfo_cfg_t;

static const uint16 tbl_iqcal_gainparams_nphy[2][NPHY_IQCAL_NUMGAINS][8] = {
	{ /* 2G table */
	{0x000, 0, 0, 2, 0x69, 0x69, 0x69, 0x69},
	{0x700, 7, 0, 0, 0x69, 0x69, 0x69, 0x69},
	{0x710, 7, 1, 0, 0x68, 0x68, 0x68, 0x68},
	{0x720, 7, 2, 0, 0x67, 0x67, 0x67, 0x67},
	{0x730, 7, 3, 0, 0x66, 0x66, 0x66, 0x66},
	{0x740, 7, 4, 0, 0x65, 0x65, 0x65, 0x65},
	{0x741, 7, 4, 1, 0x65, 0x65, 0x65, 0x65},
	{0x742, 7, 4, 2, 0x65, 0x65, 0x65, 0x65},
	{0x743, 7, 4, 3, 0x65, 0x65, 0x65, 0x65}
	},
	{ /* 5G table */
	{0x000, 7, 0, 0, 0x79, 0x79, 0x79, 0x79},
	{0x700, 7, 0, 0, 0x79, 0x79, 0x79, 0x79},
	{0x710, 7, 1, 0, 0x79, 0x79, 0x79, 0x79},
	{0x720, 7, 2, 0, 0x78, 0x78, 0x78, 0x78},
	{0x730, 7, 3, 0, 0x78, 0x78, 0x78, 0x78},
	{0x740, 7, 4, 0, 0x78, 0x78, 0x78, 0x78},
	{0x741, 7, 4, 1, 0x78, 0x78, 0x78, 0x78},
	{0x742, 7, 4, 2, 0x78, 0x78, 0x78, 0x78},
	{0x743, 7, 4, 3, 0x78, 0x78, 0x78, 0x78}
	}
};

static const uint32 nphy_tpc_txgain[] = {
	0x03cc2b44, 0x03cc2b42, 0x03cc2a44, 0x03cc2a42,
	0x03cc2944, 0x03c82b44, 0x03c82b42, 0x03c82a44,
	0x03c82a42, 0x03c82944, 0x03c82942, 0x03c82844,
	0x03c82842, 0x03c42b44, 0x03c42b42, 0x03c42a44,
	0x03c42a42, 0x03c42944, 0x03c42942, 0x03c42844,
	0x03c42842, 0x03c42744, 0x03c42742, 0x03c42644,
	0x03c42642, 0x03c42544, 0x03c42542, 0x03c42444,
	0x03c42442, 0x03c02b44, 0x03c02b42, 0x03c02a44,
	0x03c02a42, 0x03c02944, 0x03c02942, 0x03c02844,
	0x03c02842, 0x03c02744, 0x03c02742, 0x03b02b44,
	0x03b02b42, 0x03b02a44, 0x03b02a42, 0x03b02944,
	0x03b02942, 0x03b02844, 0x03b02842, 0x03b02744,
	0x03b02742, 0x03b02644, 0x03b02642, 0x03b02544,
	0x03b02542, 0x03a02b44, 0x03a02b42, 0x03a02a44,
	0x03a02a42, 0x03a02944, 0x03a02942, 0x03a02844,
	0x03a02842, 0x03a02744, 0x03a02742, 0x03902b44,
	0x03902b42, 0x03902a44, 0x03902a42, 0x03902944,
	0x03902942, 0x03902844, 0x03902842, 0x03902744,
	0x03902742, 0x03902644, 0x03902642, 0x03902544,
	0x03902542, 0x03802b44, 0x03802b42, 0x03802a44,
	0x03802a42, 0x03802944, 0x03802942, 0x03802844,
	0x03802842, 0x03802744, 0x03802742, 0x03802644,
	0x03802642, 0x03802544, 0x03802542, 0x03802444,
	0x03802442, 0x03802344, 0x03802342, 0x03802244,
	0x03802242, 0x03802144, 0x03802142, 0x03802044,
	0x03802042, 0x03801f44, 0x03801f42, 0x03801e44,
	0x03801e42, 0x03801d44, 0x03801d42, 0x03801c44,
	0x03801c42, 0x03801b44, 0x03801b42, 0x03801a44,
	0x03801a42, 0x03801944, 0x03801942, 0x03801844,
	0x03801842, 0x03801744, 0x03801742, 0x03801644,
	0x03801642, 0x03801544, 0x03801542, 0x03801444,
	0x03801442, 0x03801344, 0x03801342, 0x00002b00
	};

/** lo_scale compensates digital LO coeffs for changes in DAC gain based on tx_gain_entries table */
static const uint16 nphy_tpc_loscale[] = {
	256, 256, 271, 271, 287, 256, 256, 271,
	271, 287, 287, 304, 304, 256, 256, 271,
	271, 287, 287, 304, 304, 322, 322, 341,
	341, 362, 362, 383, 383, 256, 256, 271,
	271, 287, 287, 304, 304, 322, 322, 256,
	256, 271, 271, 287, 287, 304, 304, 322,
	322, 341, 341, 362, 362, 256, 256, 271,
	271, 287, 287, 304, 304, 322, 322, 256,
	256, 271, 271, 287, 287, 304, 304, 322,
	322, 341, 341, 362, 362, 256, 256, 271,
	271, 287, 287, 304, 304, 322, 322, 341,
	341, 362, 362, 383, 383, 406, 406, 430,
	430, 455, 455, 482, 482, 511, 511, 541,
	541, 573, 573, 607, 607, 643, 643, 681,
	681, 722, 722, 764, 764, 810, 810, 858,
	858, 908, 908, 962, 962, 1019, 1019, 256
	};

static uint32 nphy_tpc_txgain_ipa[] = {
	0x5ff7002d, 0x5ff7002b, 0x5ff7002a, 0x5ff70029,
	0x5ff70028, 0x5ff70027, 0x5ff70026, 0x5ff70025,
	0x5ef7002d, 0x5ef7002b, 0x5ef7002a, 0x5ef70029,
	0x5ef70028, 0x5ef70027, 0x5ef70026, 0x5ef70025,
	0x5df7002d, 0x5df7002b, 0x5df7002a, 0x5df70029,
	0x5df70028, 0x5df70027, 0x5df70026, 0x5df70025,
	0x5cf7002d, 0x5cf7002b, 0x5cf7002a, 0x5cf70029,
	0x5cf70028, 0x5cf70027, 0x5cf70026, 0x5cf70025,
	0x5bf7002d, 0x5bf7002b, 0x5bf7002a, 0x5bf70029,
	0x5bf70028, 0x5bf70027, 0x5bf70026, 0x5bf70025,
	0x5af7002d, 0x5af7002b, 0x5af7002a, 0x5af70029,
	0x5af70028, 0x5af70027, 0x5af70026, 0x5af70025,
	0x59f7002d, 0x59f7002b, 0x59f7002a, 0x59f70029,
	0x59f70028, 0x59f70027, 0x59f70026, 0x59f70025,
	0x58f7002d, 0x58f7002b, 0x58f7002a, 0x58f70029,
	0x58f70028, 0x58f70027, 0x58f70026, 0x58f70025,
	0x57f7002d, 0x57f7002b, 0x57f7002a, 0x57f70029,
	0x57f70028, 0x57f70027, 0x57f70026, 0x57f70025,
	0x56f7002d, 0x56f7002b, 0x56f7002a, 0x56f70029,
	0x56f70028, 0x56f70027, 0x56f70026, 0x56f70025,
	0x55f7002d, 0x55f7002b, 0x55f7002a, 0x55f70029,
	0x55f70028, 0x55f70027, 0x55f70026, 0x55f70025,
	0x54f7002d, 0x54f7002b, 0x54f7002a, 0x54f70029,
	0x54f70028, 0x54f70027, 0x54f70026, 0x54f70025,
	0x53f7002d, 0x53f7002b, 0x53f7002a, 0x53f70029,
	0x53f70028, 0x53f70027, 0x53f70026, 0x53f70025,
	0x52f7002d, 0x52f7002b, 0x52f7002a, 0x52f70029,
	0x52f70028, 0x52f70027, 0x52f70026, 0x52f70025,
	0x51f7002d, 0x51f7002b, 0x51f7002a, 0x51f70029,
	0x51f70028, 0x51f70027, 0x51f70026, 0x51f70025,
	0x50f7002d, 0x50f7002b, 0x50f7002a, 0x50f70029,
	0x50f70028, 0x50f70027, 0x50f70026, 0x50f70025
};

static uint32 nphy_tpc_txgain_ipa_2g_2057rev3[] = {
	0x70ff0040, 0x70f7003e, 0x70ef003b, 0x70e70039,
	0x70df0037, 0x70d70036, 0x70cf0033, 0x70c70032,
	0x70bf0031, 0x70b7002f, 0x70af002e, 0x70a7002d,
	0x709f002d, 0x7097002c, 0x708f002c, 0x7087002c,
	0x707f002b, 0x7077002c, 0x706f002c, 0x7067002d,
	0x705f002e, 0x705f002b, 0x705f0029, 0x7057002a,
	0x70570028, 0x704f002a, 0x7047002c, 0x7047002a,
	0x70470028, 0x70470026, 0x70470024, 0x70470022,
	0x7047001f, 0x70370027, 0x70370024, 0x70370022,
	0x70370020, 0x7037001f, 0x7037001d, 0x7037001b,
	0x7037001a, 0x70370018, 0x70370017, 0x7027001e,
	0x7027001d, 0x7027001a, 0x701f0024, 0x701f0022,
	0x701f0020, 0x701f001f, 0x701f001d, 0x701f001b,
	0x701f001a, 0x701f0018, 0x701f0017, 0x701f0015,
	0x701f0014, 0x701f0013, 0x701f0012, 0x701f0011,
	0x70170019, 0x70170018, 0x70170016, 0x70170015,
	0x70170014, 0x70170013, 0x70170012, 0x70170010,
	0x70170010, 0x7017000f, 0x700f001d, 0x700f001b,
	0x700f001a, 0x700f0018, 0x700f0017, 0x700f0015,
	0x700f0015, 0x700f0013, 0x700f0013, 0x700f0011,
	0x700f0010, 0x700f0010, 0x700f000f, 0x700f000e,
	0x700f000d, 0x700f000c, 0x700f000b, 0x700f000b,
	0x700f000b, 0x700f000a, 0x700f0009, 0x700f0009,
	0x700f0009, 0x700f0008, 0x700f0007, 0x700f0007,
	0x700f0006, 0x700f0006, 0x700f0006, 0x700f0006,
	0x700f0005, 0x700f0005, 0x700f0005, 0x700f0004,
	0x700f0004, 0x700f0004, 0x700f0004, 0x700f0004,
	0x700f0004, 0x700f0003, 0x700f0003, 0x700f0003,
	0x700f0003, 0x700f0002, 0x700f0002, 0x700f0002,
	0x700f0002, 0x700f0002, 0x700f0002, 0x700f0001,
	0x700f0001, 0x700f0001, 0x700f0001, 0x700f0001,
	0x700f0001, 0x700f0001, 0x700f0001, 0x700f0001
};

static uint32 nphy_tpc_txgain_ipa_2g_2057rev7[] = {
	0x70fe0031, 0x70e60031, 0x70e6002e, 0x70ce002e,
	0x70be002e, 0x70ae002e, 0x709e002f, 0x707e0033,
	0x707e0031, 0x707e002e, 0x7076002e, 0x706e002e,
	0x7066002e, 0x705e002f, 0x70560030, 0x7056002d,
	0x704e002e, 0x70460031, 0x7046002e, 0x7046002c,
	0x70460029, 0x703e002c, 0x703e0029, 0x7036002d,
	0x7036002a, 0x70360028, 0x702e002c, 0x702e002a,
	0x702e0028, 0x702e0026, 0x7026002c, 0x70260029,
	0x70260027, 0x70260025, 0x70260023, 0x701e002c,
	0x701e002a, 0x701e0028, 0x701e0025, 0x701e0024,
	0x701e0022, 0x701e001f, 0x7016002d, 0x7016002b,
	0x70160028, 0x70160026, 0x70160024, 0x70160022,
	0x70160020, 0x7016001e, 0x7016001d, 0x7016001b,
	0x7016001a, 0x70160018, 0x70160017, 0x70160015,
	0x700e002c, 0x700e0029, 0x700e0027, 0x700e0024,
	0x700e0022, 0x700e0021, 0x700e001f, 0x700e001d,
	0x700e001b, 0x700e001a, 0x700e0018, 0x700e0017,
	0x700e0016, 0x700e0015, 0x700e0115, 0x700e0215,
	0x700e0315, 0x700e0415, 0x700e0515, 0x700e0615,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715,
	0x700e0715, 0x700e0715, 0x700e0715, 0x700e0715
};

static uint32 nphy_tpc_txgain_ipa_2g_2057rev7_ver2[] = {
    0x607f0030, 0x60770031, 0x6077002e, 0x606f002f,
    0x60670031, 0x6067002e, 0x605f0030, 0x605f002d,
    0x6057002f, 0x6057002c, 0x6057002a, 0x604f002d,
    0x604f002a, 0x6047002e, 0x6047002b, 0x60470029,
    0x603f002c, 0x603f0029, 0x603f0027, 0x6037002c,
    0x60370029, 0x60370027, 0x602f002d, 0x602f002a,
    0x602f0028, 0x602f0026, 0x602f0024, 0x602f0022,
    0x60270028, 0x60270026, 0x60270024, 0x60270022,
    0x6027001f, 0x6027001e, 0x6027001d, 0x601f0024,
    0x601f0022, 0x601f0021, 0x601f001f, 0x601f001d,
    0x6017002a, 0x60170028, 0x60170026, 0x60170024,
    0x60170022, 0x6017001f, 0x6017001e, 0x6017001c,
    0x6017001a, 0x60170019, 0x60170018, 0x60170016,
    0x60170015, 0x600f002c, 0x600f0029, 0x600f0027,
    0x600f0024, 0x600f0022, 0x600f0021, 0x600f001f,
    0x600f001d, 0x600f001b, 0x600f001a, 0x600f0018,
    0x600f0017, 0x600f0015, 0x600f0015, 0x600f0115,
    0x600f0215, 0x600f0315, 0x600f0415, 0x600f0515,
    0x600f0615, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715,
    0x600f0715, 0x600f0715, 0x600f0715, 0x600f0715
};

static uint32 nphy_tpc_txgain_ipa_2g_2057rev8[] = {
	0x40ff0031, 0x40e70031, 0x40e7002e, 0x40cf002e,
	0x40bf002e, 0x40af002e, 0x409f002f, 0x407f0033,
	0x407f0031, 0x407f002e, 0x4077002e, 0x406f002e,
	0x4067002e, 0x405f002f, 0x40570030, 0x4057002d,
	0x404f002e, 0x40470031, 0x4047002e, 0x4047002c,
	0x40470029, 0x403f002c, 0x403f0029, 0x4037002d,
	0x4037002a, 0x40370028, 0x402f002c, 0x402f002a,
	0x402f0028, 0x402f0026, 0x4027002c, 0x40270029,
	0x40270027, 0x40270025, 0x40270023, 0x401f002c,
	0x401f002a, 0x401f0028, 0x401f0025, 0x401f0024,
	0x401f0022, 0x401f001f, 0x4017002d, 0x4017002b,
	0x40170028, 0x40170026, 0x40170024, 0x40170022,
	0x40170020, 0x4017001e, 0x4017001d, 0x4017001b,
	0x4017001a, 0x40170018, 0x40170017, 0x40170015,
	0x400f002c, 0x400f0029, 0x400f0027, 0x400f0024,
	0x400f0022, 0x400f0021, 0x400f001f, 0x400f001d,
	0x400f001b, 0x400f001a, 0x400f0018, 0x400f0017,
	0x400f0016, 0x400f0015, 0x400f0115, 0x400f0215,
	0x400f0315, 0x400f0415, 0x400f0515, 0x400f0615,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
	0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715
};

/* BCM63268 */
static uint32 nphy_tpc_txgain_ipa_2g_2057rev12[] = {
    0x40ff0031, 0x40f70030, 0x40ef002f, 0x40e7002f,
    0x40df002e, 0x40d7002e, 0x40cf002e, 0x40c7002e,
    0x40bf002e, 0x40b7002e, 0x40af002e, 0x40a7002e,
    0x409f002e, 0x4097002f, 0x408f0030, 0x40870031,
    0x4087002e, 0x4087002c, 0x40870029, 0x407f002c,
    0x4077002d, 0x406f002e, 0x40670030, 0x4067002d,
    0x405f002f, 0x405f002c, 0x4057002e, 0x4057002c,
    0x404f002e, 0x404f002c, 0x404f0029, 0x4047002d,
    0x4047002a, 0x40470028, 0x403f002c, 0x403f0029,
    0x4037002e, 0x4037002b, 0x40370029, 0x40370027,
    0x402f002c, 0x402f0029, 0x402f0027, 0x402f0025,
    0x4027002c, 0x4027002a, 0x40270028, 0x40270025,
    0x40270024, 0x401f002d, 0x401f002b, 0x401f0028,
    0x401f0026, 0x401f0024, 0x401f0022, 0x401f0020,
    0x401f001e, 0x4017002c, 0x40170029, 0x40170027,
    0x40170025, 0x40170023, 0x40170021, 0x4017001f,
    0x4017001d, 0x4017001c, 0x4017001a, 0x40170019,
    0x40170018, 0x40170016, 0x400f002c, 0x400f0029,
    0x400f0027, 0x400f0024, 0x400f0023, 0x400f0021,
    0x400f001f, 0x400f001d, 0x400f001c, 0x400f001a,
    0x400f0018, 0x400f0017, 0x400f0016, 0x400f0015,
    0x400f0115, 0x400f0215, 0x400f0315, 0x400f0415,
    0x400f0515, 0x400f0615, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715,
    0x400f0715, 0x400f0715, 0x400f0715, 0x400f0715
};

static uint32 nphy_tpc_txgain_ipa_2g_2057rev13[] = {
	0x70df002e, 0x70d7002d, 0x70cf002d, 0x70c7002c,
	0x70bf002c, 0x70b7002c, 0x70af002c, 0x70a7002b,
	0x709f002b, 0x7097002c, 0x708f002c, 0x7087002c,
	0x707f002c, 0x7077002c, 0x706f002d, 0x7067002e,
	0x705f0030, 0x705f002e, 0x70570030, 0x7057002d,
	0x704f0030, 0x704f002d, 0x704f002a, 0x7047002e,
	0x7047002b, 0x70470029, 0x703f002c, 0x703f0029,
	0x703f0027, 0x7037002c, 0x70370029, 0x70370027,
	0x702f002c, 0x702f0029, 0x702f0027, 0x702f0025,
	0x7027002c, 0x7027002a, 0x70270027, 0x70270025,
	0x70270023, 0x701f002c, 0x701f002a, 0x701f0028,
	0x701f0026, 0x701f0024, 0x701f0022, 0x701f001f,
	0x701f001e, 0x7017002b, 0x70170029, 0x70170027,
	0x70170024, 0x70170022, 0x70170020, 0x7017001f,
	0x7017001d, 0x7017001b, 0x7017001a, 0x70170018,
	0x70170017, 0x70170015, 0x700f002c, 0x700f0029,
	0x700f0027, 0x700f0024, 0x700f0022, 0x700f0021,
	0x700f001f, 0x700f001d, 0x700f001b, 0x700f001a,
	0x700f0018, 0x700f0017, 0x700f0015, 0x700f0015,
	0x700f0115, 0x700f0215, 0x700f0315, 0x700f0415,
	0x700f0515, 0x700f0615, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715,
	0x700f0715, 0x700f0715, 0x700f0715, 0x700f0715
};

static uint32 nphy_tpc_txgain_ipa_2g_2057rev14[] = {
	0x50df002e, 0x50cf002d, 0x50bf002c, 0x50b7002b,
	0x50af002a, 0x50a70029, 0x509f0029, 0x50970028,
	0x508f0027, 0x50870027, 0x507f0027, 0x50770027,
	0x506f0027, 0x50670027, 0x505f0028, 0x50570029,
	0x504f002b, 0x5047002e, 0x5047002b, 0x50470029,
	0x503f002c, 0x503f0029, 0x5037002c, 0x5037002a,
	0x50370028, 0x502f002d, 0x502f002b, 0x502f0028,
	0x502f0026, 0x5027002d, 0x5027002a, 0x50270028,
	0x50270026, 0x50270024, 0x501f002e, 0x501f002b,
	0x501f0029, 0x501f0027, 0x501f0024, 0x501f0022,
	0x501f0020, 0x501f001f, 0x5017002c, 0x50170029,
	0x50170027, 0x50170024, 0x50170022, 0x50170021,
	0x5017001f, 0x5017001d, 0x5017001b, 0x5017001a,
	0x50170018, 0x50170017, 0x50170015, 0x500f002c,
	0x500f002a, 0x500f0027, 0x500f0025, 0x500f0023,
	0x500f0022, 0x500f001f, 0x500f001e, 0x500f001c,
	0x500f001a, 0x500f0019, 0x500f0018, 0x500f0016,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015,
	0x500f0015, 0x500f0015, 0x500f0015, 0x500f0015
};

static uint32 nphy_tpc_txgain_ipa_5g[] = {
	0x7ff70035, 0x7ff70033, 0x7ff70032, 0x7ff70031,
	0x7ff7002f, 0x7ff7002e, 0x7ff7002d, 0x7ff7002b,
	0x7ff7002a, 0x7ff70029, 0x7ff70028, 0x7ff70027,
	0x7ff70026, 0x7ff70024, 0x7ff70023, 0x7ff70022,
	0x7ef70028, 0x7ef70027, 0x7ef70026, 0x7ef70025,
	0x7ef70024, 0x7ef70023, 0x7df70028, 0x7df70027,
	0x7df70026, 0x7df70025, 0x7df70024, 0x7df70023,
	0x7df70022, 0x7cf70029, 0x7cf70028, 0x7cf70027,
	0x7cf70026, 0x7cf70025, 0x7cf70023, 0x7cf70022,
	0x7bf70029, 0x7bf70028, 0x7bf70026, 0x7bf70025,
	0x7bf70024, 0x7bf70023, 0x7bf70022, 0x7bf70021,
	0x7af70029, 0x7af70028, 0x7af70027, 0x7af70026,
	0x7af70025, 0x7af70024, 0x7af70023, 0x7af70022,
	0x79f70029, 0x79f70028, 0x79f70027, 0x79f70026,
	0x79f70025, 0x79f70024, 0x79f70023, 0x79f70022,
	0x78f70029, 0x78f70028, 0x78f70027, 0x78f70026,
	0x78f70025, 0x78f70024, 0x78f70023, 0x78f70022,
	0x77f70029, 0x77f70028, 0x77f70027, 0x77f70026,
	0x77f70025, 0x77f70024, 0x77f70023, 0x77f70022,
	0x76f70029, 0x76f70028, 0x76f70027, 0x76f70026,
	0x76f70024, 0x76f70023, 0x76f70022, 0x76f70021,
	0x75f70029, 0x75f70028, 0x75f70027, 0x75f70026,
	0x75f70025, 0x75f70024, 0x75f70023, 0x74f70029,
	0x74f70028, 0x74f70026, 0x74f70025, 0x74f70024,
	0x74f70023, 0x74f70022, 0x73f70029, 0x73f70027,
	0x73f70026, 0x73f70025, 0x73f70024, 0x73f70023,
	0x73f70022, 0x72f70028, 0x72f70027, 0x72f70026,
	0x72f70025, 0x72f70024, 0x72f70023, 0x72f70022,
	0x71f70028, 0x71f70027, 0x71f70026, 0x71f70025,
	0x71f70024, 0x71f70023, 0x70f70028, 0x70f70027,
	0x70f70026, 0x70f70024, 0x70f70023, 0x70f70022,
	0x70f70021, 0x70f70020, 0x70f70020, 0x70f7001f
};

static uint32 nphy_tpc_txgain_ipa_5g_2057[] = {
	0x7f7f0044, 0x7f7f0040, 0x7f7f003c, 0x7f7f0039,
	0x7f7f0036, 0x7e7f003c, 0x7e7f0038, 0x7e7f0035,
	0x7d7f003c, 0x7d7f0039, 0x7d7f0036, 0x7d7f0033,
	0x7c7f003b, 0x7c7f0037, 0x7c7f0034, 0x7b7f003a,
	0x7b7f0036, 0x7b7f0033, 0x7a7f003c, 0x7a7f0039,
	0x7a7f0036, 0x7a7f0033, 0x797f003b, 0x797f0038,
	0x797f0035, 0x797f0032, 0x787f003b, 0x787f0038,
	0x787f0035, 0x787f0032, 0x777f003a, 0x777f0037,
	0x777f0034, 0x777f0031, 0x767f003a, 0x767f0036,
	0x767f0033, 0x767f0031, 0x757f003a, 0x757f0037,
	0x757f0034, 0x747f003c, 0x747f0039, 0x747f0036,
	0x747f0033, 0x737f003b, 0x737f0038, 0x737f0035,
	0x737f0032, 0x727f0039, 0x727f0036, 0x727f0033,
	0x727f0030, 0x717f003a, 0x717f0037, 0x717f0034,
	0x707f003b, 0x707f0038, 0x707f0035, 0x707f0032,
	0x707f002f, 0x707f002d, 0x707f002a, 0x707f0028,
	0x707f0025, 0x707f0023, 0x707f0021, 0x707f0020,
	0x707f001e, 0x707f001c, 0x707f001b, 0x707f0019,
	0x707f0018, 0x707f0016, 0x707f0015, 0x707f0014,
	0x707f0013, 0x707f0012, 0x707f0011, 0x707f0010,
	0x707f000f, 0x707f000e, 0x707f000d, 0x707f000d,
	0x707f000c, 0x707f000b, 0x707f000b, 0x707f000a,
	0x707f0009, 0x707f0009, 0x707f0008, 0x707f0008,
	0x707f0007, 0x707f0007, 0x707f0007, 0x707f0006,
	0x707f0006, 0x707f0006, 0x707f0005, 0x707f0005,
	0x707f0005, 0x707f0004, 0x707f0004, 0x707f0004,
	0x707f0004, 0x707f0004, 0x707f0003, 0x707f0003,
	0x707f0003, 0x707f0003, 0x707f0003, 0x707f0003,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0001, 0x707f0001, 0x707f0001, 0x707f0001,
	0x707f0001, 0x707f0001, 0x707f0001, 0x707f0001
};

static uint32 nphy_tpc_txgain_ipa_5g_2057rev7[] = {
	0x7f7f0031, 0x7f7f002e, 0x7f7f002c, 0x7f7f002a,
	0x7f7f0027, 0x7e7f002e, 0x7e7f002c, 0x7e7f002a,
	0x7d7f0030, 0x7d7f002d, 0x7d7f002a, 0x7d7f0028,
	0x7c7f0030, 0x7c7f002d, 0x7c7f002b, 0x7b7f002e,
	0x7b7f002c, 0x7b7f002a, 0x7b7f0027, 0x7a7f002e,
	0x7a7f002c, 0x7a7f002a, 0x797f0030, 0x797f002e,
	0x797f002b, 0x797f0029, 0x787f002f, 0x787f002d,
	0x787f002a, 0x787f0027, 0x777f002f, 0x777f002d,
	0x777f002a, 0x767f0031, 0x767f002e, 0x767f002c,
	0x767f002a, 0x757f0030, 0x757f002e, 0x757f002b,
	0x757f0029, 0x747f0030, 0x747f002d, 0x747f002b,
	0x747f0029, 0x737f002f, 0x737f002d, 0x737f002a,
	0x727f0030, 0x727f002d, 0x727f002b, 0x727f0029,
	0x717f0030, 0x717f002e, 0x717f002b, 0x717f0029,
	0x707f002f, 0x707f002d, 0x707f002a, 0x707f0027,
	0x707f0026, 0x707f0023, 0x707f0021, 0x707f0020,
	0x707f001e, 0x707f001c, 0x707f001a, 0x707f0019,
	0x707f0018, 0x707f0016, 0x707f0015, 0x707f0014,
	0x707f0012, 0x707f0012, 0x707f0011, 0x707f000f,
	0x707f000f, 0x707f000e, 0x707f000d, 0x707f000c,
	0x707f000c, 0x707f000b, 0x707f000b, 0x707f000a,
	0x707f0009, 0x707f0009, 0x707f0008, 0x707f0008,
	0x707f0008, 0x707f0007, 0x707f0007, 0x707f0006,
	0x707f0006, 0x707f0005, 0x707f0005, 0x707f0005,
	0x707f0005, 0x707f0005, 0x707f0004, 0x707f0004,
	0x707f0004, 0x707f0004, 0x707f0003, 0x707f0003,
	0x707f0003, 0x707f0003, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0001, 0x707f0001, 0x707f0001,
	0x707f0001, 0x707f0001, 0x707f0001, 0x707f0001
};

static uint32 nphy_tpc_txgain_ipa_5g_2057rev7_sul[] = {
    0x7f7f0035, 0x7f7f0032, 0x7f7f0030, 0x7f7f002d,
    0x7f7f002b, 0x7e7f0032, 0x7e7f0030, 0x7e7f002d,
    0x7d7f0034, 0x7d7f0030, 0x7d7f002e, 0x7d7f002b,
    0x7c7f0035, 0x7c7f0031, 0x7c7f002f, 0x7b7f0032,
    0x7b7f0030, 0x7b7f002d, 0x7b7f002b, 0x7a7f0032,
    0x7a7f0030, 0x7a7f002d, 0x797f0035, 0x797f0032,
    0x797f002f, 0x797f002c, 0x787f0033, 0x787f0030,
    0x787f002d, 0x787f002b, 0x777f0033, 0x777f0030,
    0x777f002d, 0x767f0035, 0x767f0032, 0x767f0030,
    0x767f002d, 0x757f0035, 0x757f0032, 0x757f002f,
    0x757f002c, 0x747f0035, 0x747f0031, 0x747f002f,
    0x747f002c, 0x737f0033, 0x737f0030, 0x737f002d,
    0x727f0035, 0x727f0031, 0x727f002f, 0x727f002c,
    0x717f0035, 0x717f0032, 0x717f002f, 0x717f002c,
    0x707f0033, 0x707f0030, 0x707f002d, 0x707f002b,
    0x707f0029, 0x707f0026, 0x707f0024, 0x707f0022,
    0x707f0021, 0x707f001e, 0x707f001c, 0x707f001b,
    0x707f001a, 0x707f0018, 0x707f0017, 0x707f0016,
    0x707f0014, 0x707f0013, 0x707f0012, 0x707f0011,
    0x707f0010, 0x707f000f, 0x707f000e, 0x707f000d,
    0x707f000d, 0x707f000c, 0x707f000c, 0x707f000b,
    0x707f000a, 0x707f000a, 0x707f0009, 0x707f0008,
    0x707f0008, 0x707f0008, 0x707f0008, 0x707f0007,
    0x707f0007, 0x707f0006, 0x707f0006, 0x707f0005,
    0x707f0005, 0x707f0005, 0x707f0004, 0x707f0004,
    0x707f0004, 0x707f0004, 0x707f0003, 0x707f0003,
    0x707f0003, 0x707f0003, 0x707f0003, 0x707f0003,
    0x707f0003, 0x707f0003, 0x707f0003, 0x707f0003,
    0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
    0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
    0x707f0002, 0x707f0001, 0x707f0001, 0x707f0001,
    0x707f0001, 0x707f0001, 0x707f0001, 0x707f0001
};

static uint32 nphy_tpc_txgain_ipa_5g_2057rev8[] = {
	0x7f7f0031, 0x7f7f002e, 0x7f7f002c, 0x7f7f002a,
	0x7f7f0027, 0x7e7f002e, 0x7e7f002c, 0x7e7f002a,
	0x7d7f0030, 0x7d7f002d, 0x7d7f002a, 0x7d7f0028,
	0x7c7f0030, 0x7c7f002d, 0x7c7f002b, 0x7b7f002e,
	0x7b7f002c, 0x7b7f002a, 0x7b7f0027, 0x7a7f002e,
	0x7a7f002c, 0x7a7f002a, 0x797f0030, 0x797f002e,
	0x797f002b, 0x797f0029, 0x787f002f, 0x787f002d,
	0x787f002a, 0x787f0027, 0x777f002f, 0x777f002d,
	0x777f002a, 0x767f0031, 0x767f002e, 0x767f002c,
	0x767f002a, 0x757f0030, 0x757f002e, 0x757f002b,
	0x757f0029, 0x747f0030, 0x747f002d, 0x747f002b,
	0x747f0029, 0x737f002f, 0x737f002d, 0x737f002a,
	0x727f0030, 0x727f002d, 0x727f002b, 0x727f0029,
	0x717f0030, 0x717f002e, 0x717f002b, 0x717f0029,
	0x707f002f, 0x707f002d, 0x707f002a, 0x707f0027,
	0x707f0026, 0x707f0023, 0x707f0021, 0x707f0020,
	0x707f001e, 0x707f001c, 0x707f001a, 0x707f0019,
	0x707f0018, 0x707f0016, 0x707f0015, 0x707f0014,
	0x707f0012, 0x707f0012, 0x707f0011, 0x707f000f,
	0x707f000f, 0x707f000e, 0x707f000d, 0x707f000c,
	0x707f000c, 0x707f000b, 0x707f000b, 0x707f000a,
	0x707f0009, 0x707f0009, 0x707f0008, 0x707f0008,
	0x707f0008, 0x707f0007, 0x707f0007, 0x707f0006,
	0x707f0006, 0x707f0005, 0x707f0005, 0x707f0005,
	0x707f0005, 0x707f0005, 0x707f0004, 0x707f0004,
	0x707f0004, 0x707f0004, 0x707f0003, 0x707f0003,
	0x707f0003, 0x707f0003, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
	0x707f0002, 0x707f0001, 0x707f0001, 0x707f0001,
	0x707f0001, 0x707f0001, 0x707f0001, 0x707f0001
};

/* BCM63268 */
static uint32 nphy_tpc_txgain_ipa_5g_2057rev12[] = {
    0x7f7f0031, 0x7f7f002e, 0x7f7f002c, 0x7f7f002a,
    0x7f7f0027, 0x7e7f002e, 0x7e7f002c, 0x7e7f002a,
    0x7d7f0030, 0x7d7f002d, 0x7d7f002a, 0x7d7f0028,
    0x7c7f0030, 0x7c7f002d, 0x7c7f002b, 0x7b7f002e,
    0x7b7f002c, 0x7b7f002a, 0x7b7f0027, 0x7a7f002e,
    0x7a7f002c, 0x7a7f002a, 0x797f0030, 0x797f002e,
    0x797f002b, 0x797f0029, 0x787f002f, 0x787f002d,
    0x787f002a, 0x787f0027, 0x777f002f, 0x777f002d,
    0x777f002a, 0x767f0031, 0x767f002e, 0x767f002c,
    0x767f002a, 0x757f0030, 0x757f002e, 0x757f002b,
    0x757f0029, 0x747f0030, 0x747f002d, 0x747f002b,
    0x747f0029, 0x737f002f, 0x737f002d, 0x737f002a,
    0x727f0030, 0x727f002d, 0x727f002b, 0x727f0029,
    0x717f0030, 0x717f002e, 0x717f002b, 0x717f0029,
    0x707f002f, 0x707f002d, 0x707f002a, 0x707f0027,
    0x707f0026, 0x707f0023, 0x707f0021, 0x707f0020,
    0x707f001e, 0x707f001c, 0x707f001a, 0x707f0019,
    0x707f0018, 0x707f0016, 0x707f0015, 0x707f0014,
    0x707f0012, 0x707f0012, 0x707f0011, 0x707f000f,
    0x707f000f, 0x707f000e, 0x707f000d, 0x707f000c,
    0x707f000c, 0x707f000b, 0x707f000b, 0x707f000a,
    0x707f0009, 0x707f0009, 0x707f0008, 0x707f0008,
    0x707f0008, 0x707f0007, 0x707f0007, 0x707f0006,
    0x707f0006, 0x707f0005, 0x707f0005, 0x707f0005,
    0x707f0005, 0x707f0005, 0x707f0004, 0x707f0004,
    0x707f0004, 0x707f0004, 0x707f0003, 0x707f0003,
    0x707f0003, 0x707f0003, 0x707f0002, 0x707f0002,
    0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
    0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
    0x707f0002, 0x707f0002, 0x707f0002, 0x707f0002,
    0x707f0002, 0x707f0001, 0x707f0001, 0x707f0001,
    0x707f0001, 0x707f0001, 0x707f0001, 0x707f0001
};

/**
 * 2057 gain deltas tables below are generated in matlab from TCL by:
 *   > tmp = fliplr(round(-4*cumsum(fliplr([pad,pga]_gain_delta))));
 *   > [pad,pga]_gain_delta_drv = [tmp(2:end), 0];
 */
/** rfpwr offset for 2.4G 43226a0/a1 & 6362a0 for pga gains 0 to 31 */
static int16 nphy_papd_padgain_dlt_2g_2057rev3n4[] = {
	-159, -113, -86, -72, -62, -54, -48, -43,
	-39, -35, -31, -28, -25, -23, -20, -18,
	-17, -15, -13, -11, -10, -8, -7, -6,
	-5, -4, -3, -3, -2, -1, -1, 0};

/** rfpwr offset for 2.4G 5357a0 for pga gains 0 to 31 */
static int16 nphy_papd_padgain_dlt_2g_2057rev5[] = {
	-109, -109, -82, -68, -58, -50, -44, -39,
	-35, -31, -28, -26, -23, -21, -19, -17,
	-16, -14, -13, -11, -10, -9, -8, -7,
	-5, -5, -4, -3, -2, -1, -1, 0};

/** rfpwr offset for 2.4G 5357b1 for pad gains 0 to 31 */
static int16 nphy_papd_padgain_dlt_2g_2057rev5v1[] = {
	-125, -125, -125, -98, -84, -73, -66, -59,
	-53, -49, -45, -41, -37, -34, -31,
	-29, -26, -25, -23, -20, -19, -17,
	-15, -13, -11, -10, -8, -7,
	-5, -4, -3, -1};

/** rfpwr offset for 2.4G 43236a0, 43236b0, 43236b1 for pga gains 0 to 31 */
static int16 nphy_papd_padgain_dlt_2g_2057rev7[] = {
	-122, -122, -95, -80, -69, -61, -54, -49,
	-43, -39, -35, -32, -28, -26, -23, -21,
	-18, -16, -15, -13, -11, -10, -8, -7,
	-6, -5, -4, -3, -2, -1, -1, 0};

static int16 nphy_papd_padgain_dlt_2g_2057rev7_ver2[] = {
	-199, -135, -108, -93, -81, -73, -66,
	-61, -56, -51, -47, -44, -40, -37, -34,
	-31, -29, -27, -24, -22, -19, -17, -15,
	-13, -12, -10, -8, -6, -5, -3, -1, 0};

/** BCM63268 */
static int16 nphy_papd_padgain_dlt_2g_2057rev12[] = {
	-136, -136,  -111,   -96,   -85,   -77,   -70,   -64,
	-59,   -55,   -51,   -47,   -44,   -41,   -38,   -35,
	-30,   -28,   -25,   -23,   -21,   -18,   -16,   -14,
	-12,   -10,    -9,    -7,    -5,    -3,    -2,     0};

static int16 nphy_papd_padgain_dlt_2g_2057rev13[] = {
	-128, -128, -128, -102, -87, -77, -69,
	-62, -56, -52, -47, -43, -40, -37, -34,
	-31, -28, -27, -25, -22, -20, -18, -16,
	-14, -12, -11, -9, -8, -6, -5, -3, -2};

static int16 nphy_papd_padgain_dlt_2g_2057rev14[] = {
	-111, -111, -111, -84, -70, -59, -52, -45,
	-40, -36, -32, -29, -26, -23, -21, -18, -16,
	-15, -13, -11, -10,	-8, -7,	-6, -5,	-4,
	-4, -3,	-2, -2,	-1,	-1};

static int8 nphy_papd_pgagain_dlt_5g_2057[] = {
	-107, -101, -92, -85, -78, -71, -62, -55,
	-47, -39, -32, -24, -19, -12, -6, 0};

static int8 nphy_papd_pgagain_dlt_5g_2057rev7[] = {
	-110, -104, -95, -88, -81, -74, -66, -58,
	-50, -44, -36, -28, -23, -15, -8, 0};

static uint8 pad_gain_codes_used_2057rev5[] = {
	14, 13, 12, 11, 10, 9, 8,  7,  6,  5,
	4,  3,  2, 1};

static uint8 pad_gain_codes_used_2057rev7[] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6,
	5,  4,  3,  2,  1 };

static uint8 pad_gain_codes_used_2057rev7_sul[] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6,
	5,  4,  3,  2,  1 };

/** BCM63268 */
static uint8 pad_gain_codes_used_2057rev12[] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6,
	5,  4,  3,  2,  1 };

static uint8 pad_all_gain_codes_2057[] = {
	31, 30, 29, 28, 27, 26, 25, 24, 23, 22,
	21, 20, 19, 18, 17, 16, 15, 14, 13, 12,
	11, 10,  9,  8,  7,  6,  5,  4,  3,  2,
	1,  0};

static uint8 pga_all_gain_codes_2057[] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

static uint32 nphy_papd_scaltbl[] = {
	0x0ae2002f, 0x0a3b0032, 0x09a70035, 0x09220038,
	0x0887003c, 0x081f003f, 0x07a20043, 0x07340047,
	0x06d2004b, 0x067a004f, 0x06170054, 0x05bf0059,
	0x0571005e, 0x051e0064, 0x04d3006a, 0x04910070,
	0x044c0077, 0x040f007e, 0x03d90085, 0x03a1008d,
	0x036f0095, 0x033d009e, 0x030b00a8, 0x02e000b2,
	0x02b900bc, 0x029200c7, 0x026d00d3, 0x024900e0,
	0x022900ed, 0x020a00fb, 0x01ec010a, 0x01d0011a,
	0x01b7012a, 0x019e013c, 0x0187014f, 0x01720162,
	0x015d0177, 0x0149018e, 0x013701a5, 0x012601be,
	0x011501d9, 0x010501f5, 0x00f70212, 0x00e90232,
	0x00dc0253, 0x00d00276, 0x00c4029c, 0x00b902c3,
	0x00af02ed, 0x00a5031a, 0x009c0349, 0x0093037a,
	0x008b03af, 0x008303e7, 0x007c0422, 0x00750461,
	0x006e04a3, 0x006804ea, 0x00620534, 0x005d0583,
	0x005805d7, 0x0053062f, 0x004e068d, 0x004a06f1
};

/* EXTPA */
static uint32 nphy_tpc_txgain_rev3[] = {
	0x1f410044, 0x1f410042, 0x1f410040, 0x1f41003e,
	0x1f41003c, 0x1f41003b, 0x1f410039, 0x1f410037,
	0x1e410044, 0x1e410042, 0x1e410040, 0x1e41003e,
	0x1e41003c, 0x1e41003b, 0x1e410039, 0x1e410037,
	0x1d410044, 0x1d410042, 0x1d410040, 0x1d41003e,
	0x1d41003c, 0x1d41003b, 0x1d410039, 0x1d410037,
	0x1c410044, 0x1c410042, 0x1c410040, 0x1c41003e,
	0x1c41003c, 0x1c41003b, 0x1c410039, 0x1c410037,
	0x1b410044, 0x1b410042, 0x1b410040, 0x1b41003e,
	0x1b41003c, 0x1b41003b, 0x1b410039, 0x1b410037,
	0x1a410044, 0x1a410042, 0x1a410040, 0x1a41003e,
	0x1a41003c, 0x1a41003b, 0x1a410039, 0x1a410037,
	0x19410044, 0x19410042, 0x19410040, 0x1941003e,
	0x1941003c, 0x1941003b, 0x19410039, 0x19410037,
	0x18410044, 0x18410042, 0x18410040, 0x1841003e,
	0x1841003c, 0x1841003b, 0x18410039, 0x18410037,
	0x17410044, 0x17410042, 0x17410040, 0x1741003e,
	0x1741003c, 0x1741003b, 0x17410039, 0x17410037,
	0x16410044, 0x16410042, 0x16410040, 0x1641003e,
	0x1641003c, 0x1641003b, 0x16410039, 0x16410037,
	0x15410044, 0x15410042, 0x15410040, 0x1541003e,
	0x1541003c, 0x1541003b, 0x15410039, 0x15410037,
	0x14410044, 0x14410042, 0x14410040, 0x1441003e,
	0x1441003c, 0x1441003b, 0x14410039, 0x14410037,
	0x13410044, 0x13410042, 0x13410040, 0x1341003e,
	0x1341003c, 0x1341003b, 0x13410039, 0x13410037,
	0x12410044, 0x12410042, 0x12410040, 0x1241003e,
	0x1241003c, 0x1241003b, 0x12410039, 0x12410037,
	0x11410044, 0x11410042, 0x11410040, 0x1141003e,
	0x1141003c, 0x1141003b, 0x11410039, 0x11410037,
	0x10410044, 0x10410042, 0x10410040, 0x1041003e,
	0x1041003c, 0x1041003b, 0x10410039, 0x10410037
};

static uint32 nphy_tpc_txgain_HiPwrEPA[] = {
	0x0f410044, 0x0f410042, 0x0f410040, 0x0f41003e,
	0x0f41003c, 0x0f41003b, 0x0f410039, 0x0f410037,
	0x0e410044, 0x0e410042, 0x0e410040, 0x0e41003e,
	0x0e41003c, 0x0e41003b, 0x0e410039, 0x0e410037,
	0x0d410044, 0x0d410042, 0x0d410040, 0x0d41003e,
	0x0d41003c, 0x0d41003b, 0x0d410039, 0x0d410037,
	0x0c410044, 0x0c410042, 0x0c410040, 0x0c41003e,
	0x0c41003c, 0x0c41003b, 0x0c410039, 0x0c410037,
	0x0b410044, 0x0b410042, 0x0b410040, 0x0b41003e,
	0x0b41003c, 0x0b41003b, 0x0b410039, 0x0b410037,
	0x0a410044, 0x0a410042, 0x0a410040, 0x0a41003e,
	0x0a41003c, 0x0a41003b, 0x0a410039, 0x0a410037,
	0x09410044, 0x09410042, 0x09410040, 0x0941003e,
	0x0941003c, 0x0941003b, 0x09410039, 0x09410037,
	0x08410044, 0x08410042, 0x08410040, 0x0841003e,
	0x0841003c, 0x0841003b, 0x08410039, 0x08410037,
	0x07410044, 0x07410042, 0x07410040, 0x0741003e,
	0x0741003c, 0x0741003b, 0x07410039, 0x07410037,
	0x06410044, 0x06410042, 0x06410040, 0x0641003e,
	0x0641003c, 0x0641003b, 0x06410039, 0x06410037,
	0x05410044, 0x05410042, 0x05410040, 0x0541003e,
	0x0541003c, 0x0541003b, 0x05410039, 0x05410037,
	0x04410044, 0x04410042, 0x04410040, 0x0441003e,
	0x0441003c, 0x0441003b, 0x04410039, 0x04410037,
	0x03410044, 0x03410042, 0x03410040, 0x0341003e,
	0x0341003c, 0x0341003b, 0x03410039, 0x03410037,
	0x02410044, 0x02410042, 0x02410040, 0x0241003e,
	0x0241003c, 0x0241003b, 0x02410039, 0x02410037,
	0x01410044, 0x01410042, 0x01410040, 0x0141003e,
	0x0141003c, 0x0141003b, 0x01410039, 0x01410037,
	0x00410044, 0x00410042, 0x00410040, 0x0041003e,
	0x0041003c, 0x0041003b, 0x00410039, 0x00410037
};

/**
 * 6362A0 ExtPA gain table to front load the gains to counter the broadband noise introduced by the
 * TxBuf LPF = 1, gm = 0, intPA = 1 bbmult is kept as high as possible (max 66) throughout the gain
 * table PAD is varied as much as possible before varying bbmult.
 * Note: In this table, the gain codes are limited to having a minimum bbmult of 30 to avoid
 * quantization noise effects. Also, the 7 0.5 dB step-size attens of the DAC are used at the end
 * of the gain table.
 */
static uint32 nphy_tpc_txgain_epa_2057rev3[] = {
	0x80f90040, 0x80e10040, 0x80e1003c, 0x80c9003d,
	0x80b9003c, 0x80a9003d, 0x80a1003c, 0x8099003b,
	0x8091003b, 0x8089003a, 0x8081003a, 0x80790039,
	0x80710039, 0x8069003a, 0x8061003b, 0x8059003d,
	0x8051003f, 0x80490042, 0x8049003e, 0x8049003b,
	0x8041003e, 0x8041003b, 0x8039003e, 0x8039003b,
	0x80390038, 0x80390035, 0x8031003a, 0x80310036,
	0x80310033, 0x8029003a, 0x80290037, 0x80290034,
	0x80290031, 0x80210039, 0x80210036, 0x80210033,
	0x80210030, 0x8019003c, 0x80190039, 0x80190036,
	0x80190033, 0x80190030, 0x8019002d, 0x8019002b,
	0x80190028, 0x8011003a, 0x80110036, 0x80110033,
	0x80110030, 0x8011002e, 0x8011002b, 0x80110029,
	0x80110027, 0x80110024, 0x80110022, 0x80110020,
	0x8011001f, 0x8011001d, 0x8009003a, 0x80090037,
	0x80090034, 0x80090031, 0x8009002e, 0x8009002c,
	0x80090029, 0x80090027, 0x80090025, 0x80090023,
	0x80090021, 0x8009001f, 0x8009001d, 0x8009011d,
	0x8009021d, 0x8009031d, 0x8009041d, 0x8009051d,
	0x8009061d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d,
	0x8009071d, 0x8009071d, 0x8009071d, 0x8009071d
};

/** 5357B1 ExtPA gain table. bbmult smoothing to avoid kinks at EVM 20Mhz / 40Mhz */
static uint32 nphy_tpc_txgain_epa_2057rev5v2[] = {
		0x104a0031, 0x104a002e, 0x104a002b, 0x104a0029,
		0x1042002c, 0x1042002a, 0x10420027, 0x10420025,
		0x10420023, 0x10420021, 0x1042001f, 0x10320029,
		0x10320026, 0x10320024, 0x10320022, 0x10320020,
		0x102a0025, 0x102a0023, 0x102a0021, 0x102a001f,
		0x10220025, 0x10220023, 0x10220021, 0x1022001f,
		0x1022011f, 0x1022021f, 0x1022031f, 0x101a0022,
		0x101a0020, 0x101a0120, 0x101a0220, 0x101a0320,
		0x101a0420, 0x101a0520, 0x101a0620, 0x101a0720,
		0x1012001f, 0x1012001e, 0x1012001c, 0x1012001a,
		0x10120019, 0x10120119, 0x10120219, 0x10120319,
		0x10120419, 0x10120519, 0x10120619, 0x100a0024,
		0x100a0022, 0x100a0020, 0x100a001f, 0x100a001d,
		0x100a001b, 0x100a001a, 0x100a011a, 0x100a021a,
		0x100a031a, 0x100a041a, 0x100a051a, 0x100a061a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
		0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a
};

/**
 * 43236A0 ExtPA gain table
 * LPF = 0, gm = 1, intPA = 1. bbmult is kept as high as possible (max 62) throughout the gain table
 * PAD is varied as much as possible before varying bbmult. Maximum PAD in the gain table is 9
 */
static uint32 nphy_tpc_txgain_epa_2057rev7[] = {
	0x1049003e, 0x1049003b, 0x1041003e, 0x1041003b,
	0x1039003e, 0x1039003b, 0x10390038, 0x10390035,
	0x1031003a, 0x10310036, 0x10310033, 0x1029003a,
	0x10290037, 0x10290034, 0x10290031, 0x10210039,
	0x10210036, 0x10210033, 0x10210030, 0x1019003c,
	0x10190039, 0x10190036, 0x10190033, 0x10190030,
	0x1019002d, 0x1019002b, 0x10190028, 0x1011003a,
	0x10110036, 0x10110033, 0x10110030, 0x1011002e,
	0x1011002b, 0x10110029, 0x10110027, 0x10110024,
	0x10110022, 0x10110020, 0x1011001f, 0x1011001d,
	0x1009003a, 0x10090037, 0x10090034, 0x10090031,
	0x1009002e, 0x1009002c, 0x10090029, 0x10090027,
	0x10090025, 0x10090023, 0x10090021, 0x1009001f,
	0x1009001d, 0x1009011d, 0x1009021d, 0x1009031d,
	0x1009041d, 0x1009051d, 0x1009061d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d,
	0x1009071d, 0x1009071d, 0x1009071d, 0x1009071d
};

/**
 * 43236B1 ExtPA initial gain table. LPF = 0, gm = 1, intPA = 1
 * bbmult is kept as high as possible (max 64) throughout the gain table
 * PAD is varied as much as possible before varying bbmult
 * Maximum PAD in the gain table is 20
 */
static uint32 nphy_tpc_txgain_epa_2057rev7_2gv14[] = {
	0x10790040, 0x1079003c, 0x1071003d, 0x1069003e,
	0x1061003f, 0x1061003c, 0x1059003d, 0x1051003f,
	0x1051003c, 0x10510038, 0x1049003d, 0x10490039,
	0x1041003d, 0x10410039, 0x1039003e, 0x1039003b,
	0x10390037, 0x1031003d, 0x1031003a, 0x10310036,
	0x1029003e, 0x1029003a, 0x10290037, 0x10290034,
	0x1021003d, 0x10210039, 0x10210036, 0x10210033,
	0x10210030, 0x1019003e, 0x1019003a, 0x10190037,
	0x10190034, 0x10190031, 0x1019002e, 0x1019002c,
	0x1011003d, 0x1011003a, 0x10110037, 0x10110034,
	0x10110031, 0x1011002e, 0x1011002b, 0x10110029,
	0x10110027, 0x10110025, 0x10110023, 0x10110021,
	0x1011001f, 0x1009003e, 0x1009003a, 0x10090037,
	0x10090034, 0x10090031, 0x1009002e, 0x1009002c,
	0x10090029, 0x10090027, 0x10090025, 0x10090023,
	0x10090021, 0x1009001f, 0x1009001d, 0x1009001c,
	0x1009001a, 0x10090019, 0x10090017, 0x10090016,
	0x10090015, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014,
	0x10090014, 0x10090014, 0x10090014, 0x10090014
};

static uint32 nphy_tpc_txgain_epa_2057rev7_2gv17[] = {
	0x10520033, 0x10520031, 0x1052002f, 0x104a0031,
	0x104a002e, 0x104a002b, 0x104a0029, 0x1042002c,
	0x1042002a, 0x10420027, 0x10420025, 0x10420023,
	0x10420021, 0x1042001f, 0x10320029, 0x10320026,
	0x10320024, 0x10320022, 0x10320020, 0x102a0025,
	0x102a0023, 0x102a0021, 0x102a001f, 0x10220025,
	0x10220023, 0x10220021, 0x1022001f, 0x1022011f,
	0x1022021f, 0x1022031f, 0x101a0022, 0x101a0020,
	0x101a0120, 0x101a0220, 0x101a0320, 0x101a0420,
	0x101a0520, 0x101a0620, 0x101a0720, 0x1012001f,
	0x1012001e, 0x1012001c, 0x1012001a, 0x10120019,
	0x10120119, 0x10120219, 0x10120319, 0x10120419,
	0x10120519, 0x10120619, 0x100a0024, 0x100a0022,
	0x100a0020, 0x100a001f, 0x100a001d, 0x100a001b,
	0x100a001a, 0x100a011a, 0x100a021a, 0x100a031a,
	0x100a041a, 0x100a051a, 0x100a061a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a,
	0x100a071a, 0x100a071a, 0x100a071a, 0x100a071a
};

/* External PA Gain Table for 43236+ */
static uint32 nphy_tpc_5GHz_txgain_epa_2057rev7[] = {
	0x2d59003c, 0x2d590039, 0x2d590035, 0x2c59003f,
	0x2c59003b, 0x2c590038, 0x2b59003d, 0x2b59003a,
	0x2b590037, 0x2b590034, 0x2a59003e, 0x2a59003b,
	0x2a590037, 0x29590040, 0x2959003d, 0x29590039,
	0x29590036, 0x2859003f, 0x2859003c, 0x28590039,
	0x28590035, 0x2759003f, 0x2759003c, 0x27590039,
	0x27490040, 0x2749003d, 0x27490039, 0x27490036,
	0x27490033, 0x2739003d, 0x27390039, 0x27390036,
	0x27390033, 0x2729003f, 0x2729003c, 0x27290039,
	0x27290035, 0x27290032, 0x27290030, 0x2729002d,
	0x27190040, 0x2719003d, 0x27190039, 0x27190036,
	0x27190033, 0x27190030, 0x2719002d, 0x2719002b,
	0x27190028, 0x27190026, 0x27190024, 0x27190022,
	0x2709003e, 0x2709003b, 0x27090037, 0x27090034,
	0x27090031, 0x2709002e, 0x2709002c, 0x27090029,
	0x27090027, 0x27090025, 0x27090023, 0x27090021,
	0x2709001f, 0x2709001d, 0x2709011d, 0x2709021d,
	0x2709031d, 0x2709041d, 0x2709051d, 0x2709061d,
	0x2709071d, 0x2609071d, 0x2509071d, 0x2409071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d, 0x2309071d,
	0x2309071d, 0x2309071d, 0x2309071d,	0x2309071d
};

/**
 * External PA Gain Tables for 43236B1 43238B1. Change both PGA and PAD. Base gain index is 0x1049.
 * Lower pga to 9 then start to decrease pad. bbmult = [20 - 50]. Lower DAC's gain after bbmult
 * reaches minimum
 */
static uint32 nphy_tpc_5GHz_txgain_epa_2057rev7_5gv7[] = {
	0x1f490032, 0x1f49002f, 0x1f49002d, 0x1f49002a,
	0x1e490031, 0x1e49002e, 0x1e49002b, 0x1e490029,
	0x1d490030, 0x1d49002d, 0x1d49002a, 0x1d490028,
	0x1c490030, 0x1c49002d, 0x1c49002b, 0x1b49002f,
	0x1b49002d, 0x1b49002a, 0x1b490028, 0x1a49002f,
	0x1a49002d, 0x1a49002a, 0x19490031, 0x1949002f,
	0x1949002c, 0x1939002f, 0x1939002d, 0x1939002a,
	0x19390028, 0x19290031, 0x1929002f, 0x1929002c,
	0x1929002a, 0x19290027, 0x19290025, 0x19290023,
	0x19190031, 0x1919002e, 0x1919002b, 0x19190029,
	0x19190027, 0x19190024, 0x19190022, 0x19190020,
	0x1919001f, 0x1919001d, 0x1919001b, 0x1919001a,
	0x1909002f, 0x1909002d, 0x1909002a, 0x19090028,
	0x19090026, 0x19090024, 0x19090022, 0x19090020,
	0x1909001e, 0x1909001c, 0x1909001b, 0x19090019,
	0x19090018, 0x19090016, 0x19090015, 0x19090014,
	0x19090114, 0x19090214, 0x19090314, 0x19090414,
	0x19090514, 0x19090614, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714,
	0x19090714, 0x19090714, 0x19090714, 0x19090714
};

/** External PA Gain Table for brd type 0xF52A corresponding to CISCO E3200 */
static uint32 nphy_tpc_5GHz_txgain_epa_2057rev7_brdtype_0xF52A[] = {
	0x2f590048, 0x2f590044, 0x2f590040, 0x2f59003d,
	0x2f590039, 0x2e590044, 0x2e590040, 0x2e59003d,
	0x2e590039, 0x2d590042, 0x2d59003e, 0x2d59003b,
	0x2c590046, 0x2c590042, 0x2c59003f, 0x2c59003b,
	0x2b590040, 0x2b59003d, 0x2b590039, 0x2a590044,
	0x2a590040, 0x2a59003d, 0x2a590039, 0x29590043,
	0x2959003f, 0x2959003c, 0x28590045, 0x28590041,
	0x2859003d, 0x2859003a, 0x27590045, 0x27590041,
	0x2759003d, 0x2759003a, 0x26590044, 0x26590040,
	0x2659003d, 0x26590039, 0x25590043, 0x2559003f,
	0x2559003c, 0x24590046, 0x24590042, 0x2459003f,
	0x2459003b, 0x23590045, 0x23590041, 0x2359003d,
	0x2359003a, 0x22590042, 0x2259003f, 0x2259003b,
	0x21590047, 0x21590043, 0x2159003f, 0x2159003c,
	0x20590045, 0x20590041, 0x2059003d, 0x2059003a,
	0x20590037, 0x20590034, 0x20590031, 0x2059002e,
	0x2059002b, 0x20590029, 0x20590027, 0x20590024,
	0x20590022, 0x20590021, 0x2059001f, 0x2059001d,
	0x2059011d, 0x2059021d, 0x2059031d, 0x2059041d,
	0x2059051d, 0x2059061d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d,
	0x2059071d, 0x2059071d, 0x2059071d, 0x2059071d
};

/* AntSwCtrlLUT mods for core1 on 535[78] and 47186:
 * mimophy_core1_ant1_[rx,tx] are the dedicated pins (not mimophy_core1_ant0_[rx,tx]
 */
/* to ensure 3rd switch is always in a defined state for boards with 3 antennas */

/* %%%%%% function declaration */

static bool wlc_phy_watchdog_nphy(phy_wd_ctx_t *ctx);
static void wlc_phy_chanspec_nphy_setup(phy_info_t *pi, chanspec_t chans, const nphy_sfo_cfg_t *c);

static void wlc_phy_adjust_min_noisevar_nphy(phy_info_t *pi, int ntones, int *, uint32 *buf);
static void wlc_phy_adjust_base_noisevar_nphy(phy_info_t *pi, int ntones,
                                              int *tone_id_buf, uint32 *base_nvar_delta_buf);
static void wlc_phy_spurwar_nphy(phy_info_t *pi);

/* Note: cap_val contains
 * cap_val[0,1,2] = [big_cap[0], small_cap[0], adc_cap[0]]
 * cap_val[3,4,5] = [big_cap[1], small_cap[1], adc_cap[1]]
 * (in that order)
 */
void wlc_phy_workarounds_nphy(phy_info_t *pi);
void wlc_phy_workarounds_nphy_rev17(phy_info_t *pi);
void wlc_phy_workarounds_nphy_gainctrl(phy_info_t *pi);
void wlc_phy_workarounds_nphy_gainctrl_2057_rev3(phy_info_t *pi);
void wlc_phy_workarounds_nphy_gainctrl_2057_rev5(phy_info_t *pi);
void wlc_phy_workarounds_nphy_gainctrl_2057_rev6(phy_info_t *pi);
void wlc_phy_workarounds_nphy_gainctrl_2057_rev7(phy_info_t *pi);
static void wlc_phy_workarounds_nphy_gainctrl_2057_rev13(phy_info_t *pi);
static void wlc_phy_workarounds_nphy_gainctrl_2057_rev14(phy_info_t *pi);
void wlc_phy_adjust_ClipLO_for_triso_2057_rev5_rev9(phy_info_t *pi);
/* static void wlc_phy_subband_cust_2057_rev7_nphy(phy_info_t *pi); */
static void wlc_phy_subband_cust_43236B1_sulley_nphy(phy_info_t *pi);
static void wlc_phy_subband_cust_43236B1_usbelna_nphy(phy_info_t *pi);
static void wlc_phy_subband_cust_43236B1_preproto_blu2o3_nphy(phy_info_t *pi);
static void wlc_phy_subband_cust_43236B1_um_nphy(phy_info_t *pi);
static void wlc_phy_restore_rssical_nphy(phy_info_t *pi);
static void wlc_phy_rxcal_gainctrl_nphy_rev5(phy_info_t *pi, uint8 rxcore, uint16 *rg, uint8 type);
static void wlc_phy_update_mimoconfig_nphy(phy_info_t *pi, int32 preamble);
static void wlc_phy_aci_home_channel_nphy(phy_info_t *pi, chanspec_t chanspec);
#ifndef WLC_DISABLE_ACI
static void wlc_phy_bphy_ofdm_noise_update_LUT(phy_info_t *pi, bool aci_enable);
#endif /* WLC_DISABLE_ACI */
static void wlc_phy_savecal_nphy(phy_info_t *pi);
static void wlc_phy_restorecal_nphy(phy_info_t *pi);

static void wlc_phy_enable_extlna_ctrl_nphy(phy_info_t *pi);
static void wlc_phy_enable_extpa_ctrl_nphy(phy_info_t *pi);
static void wlc_phy_enable_shared_ant_nphy(phy_info_t *pi);
static void wlc_phy_txpwrctrl_config_nphy(phy_info_t *pi);
static void wlc_phy_ipa_internal_tssi_setup_nphy(phy_info_t *pi, bool use_pad_tapoff);
static void wlc_phy_internal_tssi_cleanup_nphy(phy_info_t *pi);
static void wlc_phy_precal_txgain_nphy(phy_info_t *pi);
static void wlc_phy_tx_iqlo_precal_gctrl_auxadc_nphy(phy_info_t *pi);
static void wlc_phy_update_txcal_ladder_nphy(phy_info_t *pi, uint16 core);

static void wlc_phy_extpa_set_tx_digi_filts_nphy(phy_info_t *pi);
static void wlc_phy_ipa_set_tx_digi_filts_nphy(phy_info_t *pi);
static void wlc_phy_ipa_set_cal_tx_digi_filts_nphy(phy_info_t *pi);
static uint16 wlc_phy_ipa_get_bbmult_nphy(phy_info_t *pi);
static void wlc_phy_ipa_set_bbmult_nphy(phy_info_t *pi, uint8 m0, uint8 m1);
static uint32* wlc_phy_get_ipa_gaintbl_nphy(phy_info_t *pi);
uint16 wlc_phy_gain_from_code(phy_info_t *pi, uint8 type, uint8 core_num);

static void wlc_phy_papd_smooth_nphy(phy_info_t *pi, uint8 core, uint32 winsz, uint32, uint32 e);
static uint8 wlc_phy_papd_cal_gctrl_nphy(phy_info_t *pi, uint8 start_gain, uint8 core,
uint8 *mixgain_ovr, uint8 * atten_ovr);
static void wlc_phy_papd_cal_cleanup_nphy(phy_info_t *pi, nphy_papd_restore_state *state);
static void wlc_phy_papd_cal_setup_nphy(phy_info_t *pi, nphy_papd_restore_state *state, uint8);

#ifndef WLC_DISABLE_ACI
static int wlc_phy_aci_scan_iqbased_nphy(phy_info_t *pi);
static void wlc_phy_aci_pwr_upd_nphy(phy_info_t *pi, int aci_pwr);
static void wlc_phy_aci_noise_shared_reset_nphy(phy_info_t *pi);
static void wlc_phy_noisemode_set_nphy(phy_info_t *pi, uint8 raise);
static void wlc_phy_aci_sw_set_nphy(phy_info_t *pi, bool enable, int aci_pwr);
static void wlc_phy_noise_sw_set_nphy(phy_info_t *pi);
static void wlc_phy_aci_hw_set_nphy(phy_info_t *pi, bool enable, int aci_pwr);
static void wlc_phy_aci_noise_shared_hw_set_nphy(phy_info_t *pi, bool enable,
	bool from_aci_call);
static void wlc_phy_noise_adj_thresholds_nphy(phy_info_t *pi, uint8 raise);
static void wlc_phy_noise_limit_rfgain_nphy(phy_info_t *pi, int16 *newgain);
static void wlc_phy_noise_limit_crsmin_nphy(phy_info_t *pi, uint16 gain);
static void wlc_phy_noisemode_glitch_chk_adj_nphy(phy_info_t *pi, uint16 noise_enter_th,
	uint16 noise_glitch_th_up, uint16 noise_glitch_th_dn);
#endif /* Compiling out ACI code */

static void wlc_phy_aci_noise_store_values_nphy(phy_info_t *pi);

static void wlc_phy_rssi_cal_nphy_rev3(phy_info_t *pi);

static bool wlc_phy_srom_read_nphy(phy_info_t *pi);
static void wlc_phy_txpwr_limit_to_tbl_nphy(phy_info_t *pi);
void wlc_phy_txpwrctrl_coeff_setup_nphy(phy_info_t *pi);
static void wlc_phy_txpwrctrl_pwr_setup_nphy(phy_info_t *pi);

static void wlc_phy_intpa_set_tx_digi_filts_nphy(phy_info_t *pi, uint16 addr_offset, int idx);
static bool wlc_phy_txpwr_ison_nphy(phy_info_t *pi);
static uint8 wlc_phy_txpwr_idx_cur_get_nphy(phy_info_t *pi, uint8 core);
static void wlc_phy_txpwr_idx_cur_set_nphy(phy_info_t *pi, uint8 idx0, uint8 idx1);
static void wlc_phy_txpwr_papd_cal_run(phy_info_t *pi, bool full_cal,
	uint8 core_from, uint8 core_to);

static uint16 wlc_phy_gen_load_samples_nphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val,
                                            uint8 dac_test_mode);
#ifndef DONGLEBUILD
static void wlc_phy_loadsampletable_nphy(phy_info_t *pi, math_cint32 *tone_buf, uint16 num_samps);
#endif // endif
static void wlc_phy_runsamples_nphy(phy_info_t *pi, uint16 n, uint16 lps, uint16 wait, uint8 iq,
                                    uint8 dac_test_mode, bool modify_bbmult);
int16 wlc_phy_rxgaincode_to_dB_nphy(phy_info_t *pi, uint16 gain_code);

#if defined(PHYCAL_CACHING)
extern void wlc_phy_cal_cache_nphy(phy_type_cache_ctx_t * cache_ctx);
#endif // endif

static int16 wlc_phy_get_tssi_pwr_limit_nphy(int16 a1, int16 b0, int16 b1, uint8 maxlimit, uint8
					     offset);
static int16 wlc_phy_tssi2qtrdbm_nphy(int16 tssi, int16 a1, int16 b0, int16 b1, uint8 mult);
static void  wlc_phy_set_tssi_pwr_limit_nphy(phy_info_t *pi, int16 *a1, int16 *b0, int16 *b1, uint8
					     mode);
static void wlc_phy_ant_force_nphy(phy_info_t* pi, uint8* entries);
static void wlc_phy_ant_release_nphy(phy_info_t* pi, uint8* entries);

#ifdef WLSRVSDB
#define SRVSDB_IS_BANK0(pi, channel) (channel == pi->srvsdb_state->sr_vsdb_channels[0])
#define SRVSDB_IS_BANK1(pi, channel) (channel == pi->srvsdb_state->sr_vsdb_channels[1])
#define SRVSDSB_MEM_ALLOC_FAIL(pi)	 (pi->vsdb_bkp[0] == NULL) || (pi->vsdb_bkp[1] == NULL)
#define SRVSDB_BAND_BANK_INVALID(pi, isbank0)		(wlc_phy_sr_vsdb_status(pi) ^ isbank0)
#define ROAM_SRVSDB_RESET(pi) \
	((pi->srvsdb_state->vsdb_trig_cnt == 0) && (wlc_phy_sr_vsdb_status(pi) == 1))
static uint8 wlc_phy_sr_vsdb_status(phy_info_t *pi);
static uint8 sr_vsdb_switch_allowed(phy_info_t *pi, chanspec_t chanspec);
static void sr_vsdb_trigger(phy_info_t *pi, int delay);
static void wlc_phy_srvsdb_prepare(phy_info_t *pi, int sav_res);
static uint8  wlc_vsdb_switch(phy_info_t *ppi, chanspec_t chanspec);
static void wlc_vsdb_sr_chanspec_set(phy_info_t *ppi, uint8 save, uint8 offset);
/* static void wlc_vsdb_disable_stall(phy_info_t *ppi);
static void wlc_vsdb_enable_stall(phy_info_t *ppi);
*/
static void wlc_vsdb_radio_snapshot(phy_info_t *ppi, uint8 offset);
#ifndef WLUCODE_RDO_SR
static void wlc_vsdb_radio_restore(phy_info_t *ppi, uint8 offset);
#endif // endif

static void wlc_vsdb_txpower_snapshot(phy_info_t *ppi, uint8 offset);
static void wlc_vsdb_txpower_restore(phy_info_t *ppi, uint8 offset);

static uint8 wlc_phy_srvsdb_swbackup_restore(phy_info_t *pi, chanspec_t chanspec);
static uint8  wlc_phy_srvsdb_swbackup_save(phy_info_t * pi);
#endif /* WLSRVSDB */
extern uint32 hnd_clk_count(void);

static void wlc_phy_set_srom_eu_edthresh_nphy(phy_info_t *pi);

static bool wlc_phy_txpwr_srom8_read(phy_info_t *pi);

static void wlc_phy_txpwr_srom8_read_ppr(phy_info_t *pi);

/* Dump */
#if defined(BCMDBG)
static int wlc_phydump_papd(void *ctx, struct bcmstrbuf *b);
static int wlc_phydump_lnagain(void *ctx, struct bcmstrbuf *b);
static int wlc_phydump_initgain(void *ctx, struct bcmstrbuf *b);
static int wlc_phydump_hpf1tbl(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* For phyrev 3 to 6, same LUT applies for ACI on and ACI off.
* Only the minimum sensitivity changes.
* The change in sensitivity is constant and equal to
* the difference in initgain of ACI off and ACI on.
*/
/** Bphy desense settings in ~1dB steps, starting from 0dB */
bphy_desense_info_t NPHY_bphy_desense_lut_rev3to6[] = {
	{0x4477, 0x10, 0}, {0x4403, 0x10, 0}, {0x4404, 0x24, 0},
	{0x4403, 0x3b, 1}, {0x4404, 0x20, 1}, {0x4404, 0x24, 1},
	{0x4404, 0x28, 1}, {0x4404, 0x2c, 1}, {0x4404, 0x33, 1},
	{0x4404, 0x40, 1}, {0x4404, 0x2a, 2}, {0x4404, 0x30, 2},
	{0x4404, 0x3a, 2}, {0x4404, 0x45, 2}, {0x4404, 0x32, 3},
	{0x4404, 0x37, 3}, {0x4404, 0x45, 3}, {0x4404, 0x30, 4},
	{0x4404, 0x38, 4}, {0x4404, 0x45, 4}
};

/** Bphy desense settings in ~1dB steps, starting from 0dB */
bphy_desense_info_t NPHY_bphy_desense_aci_off_lut_rev7to15[] = {
	{0x4477, 0x10, 0}, {0x4403, 0x40, 0}, {0x4404, 0x3c, 1},
	{0x4404, 0x34, 2}, {0x4404, 0x3a, 2}, {0x4404, 0x44, 2},
	{0x4404, 0x2c, 3}, {0x4404, 0x33, 3}, {0x4404, 0x42, 3},
	{0x4404, 0x30, 4}, {0x4404, 0x33, 4}, {0x4404, 0x44, 4},
	{0x4404, 0x30, 5}, {0x4404, 0x36, 5}, {0x4404, 0x40, 5},
	{0x4404, 0x30, 6}, {0x4404, 0x38, 6}, {0x4404, 0x3c, 6},
	{0x4404, 0x44, 6}
};

/** Bphy desense settings in ~1dB steps, starting from 0dB */
bphy_desense_info_t NPHY_bphy_desense_aci_on_lut_rev7to15[] = {
	{0x4477, 0x10, 0}, {0x4403, 0x20, 0}, {0x4403, 0x30, 1},
	{0x4403, 0x40, 1}, {0x4404, 0x30, 0}, {0x4403, 0x40, 2},
	{0x4404, 0x20, 2}, {0x4404, 0x30, 1}, {0x4404, 0x40, 1},
	{0x4404, 0x20, 2}, {0x4404, 0x20, 3}, {0x4404, 0x10, 4},
	{0x4404, 0x30, 2}, {0x4404, 0x40, 2}, {0x4404, 0x20, 4},
	{0x4403, 0x40, 5}, {0x4404, 0x40, 3}, {0x4404, 0x30, 4},
	{0x4404, 0x40, 4}, {0x4404, 0x30, 5}
};

/**
 * The crsminpwr threshold in 0.25 dBm steps
 * formula:  round((10.^(([-91:+0.25:-74]-2+69-30)/10)*50)*(2^9/0.4)^2 /(2^4))
 * valid values are < 255. But, when we reduce the BQ1, we just need to
 * divide the crsmin_value by 2 for every tick drop BQ1.
 */
uint16 NPHY_ofdm_desense_lut_rev3to6[] = {
	20, 22, 23, 24, 26, 27, 29, 30, 32, 34, 36, 38, 41, 43, 46, 48, 51,
	54, 57, 61, 64, 68, 72, 77, 81, 86, 91, 96, 102, 108, 115, 121, 129,
	136, 144, 153, 162, 172, 182, 192, 204, 216, 229, 242, 257, 272, 288,
	305, 323, 342, 362, 384, 407, 431, 456, 483, 512, 542, 574, 609, 645,
	683, 723, 766, 811, 860, 910, 964, 1022
};

/** Reclaimable strings that are only accessed in attach functions. */
static const char BCMATTACHDATA(rstr_cckfilttype)[] = "cckfilttype";
static const char BCMATTACHDATA(rstr_disable_spuravoid)[] = "disable_spuravoid";
static const char BCMATTACHDATA(rstr_elnabypass2g)[] = "elnabypass2g";
static const char BCMATTACHDATA(rstr_elnabypass5g)[] = "elnabypass5g";
static const char BCMATTACHDATA(rstr_noisecaloffset)[] = "noisecaloffset";
static const char BCMATTACHDATA(rstr_noisecaloffset5g)[] = "noisecaloffset5g";
static const char BCMATTACHDATA(rstr_noiselvl2ga0)[] = "noiselvl2ga0";
static const char BCMATTACHDATA(rstr_noiselvl2ga1)[] = "noiselvl2ga1";
static const char BCMATTACHDATA(rstr_noiselvl5gha0)[] = "noiselvl5gha0";
static const char BCMATTACHDATA(rstr_noiselvl5gha1)[] = "noiselvl5gha1";
static const char BCMATTACHDATA(rstr_noiselvl5gla0)[] = "noiselvl5gla0";
static const char BCMATTACHDATA(rstr_noiselvl5gla1)[] = "noiselvl5gla1";
static const char BCMATTACHDATA(rstr_noiselvl5gma0)[] = "noiselvl5gma0";
static const char BCMATTACHDATA(rstr_noiselvl5gma1)[] = "noiselvl5gma1";
static const char BCMATTACHDATA(rstr_noiselvl5gua0)[] = "noiselvl5gua0";
static const char BCMATTACHDATA(rstr_noiselvl5gua1)[] = "noiselvl5gua1";
static const char BCMATTACHDATA(rstr_noisevaroffset)[] = "noisevaroffset";
static const char BCMATTACHDATA(rstr_filttype)[] = "filttype";
static const char BCMATTACHDATA(rstr_txiqcal_adc)[] = "txiqcal_adc";
static const char BCMATTACHDATA(rstr_ofdmfilttype)[] = "ofdmfilttype";
static const char BCMATTACHDATA(rstr_ofdmfilttype40)[] = "ofdmfilttype40";
static const char BCMATTACHDATA(rstr_parefldovoltage)[] = "parefldovoltage";
static const char BCMATTACHDATA(rstr_PwrOffset40mhz5g)[] = "PwrOffset40mhz5g";
static const char BCMATTACHDATA(rstr_PwrOffsetcck)[] = "PwrOffsetcck";
static const char BCMATTACHDATA(rstr_bphy_sm_fix_opt)[] = "bphy_sm_fix_opt";
static const char BCMATTACHDATA(rstr_temp_high)[] = "tpc_temp_hi";
static const char BCMATTACHDATA(rstr_temp_offs1_2g)[] = "tpc_temp_offs1_2g";
static const char BCMATTACHDATA(rstr_temp_offs1_5g)[] = "tpc_temp_offs1_5g";
static const char BCMATTACHDATA(rstr_temp_low)[] = "tpc_temp_low";
static const char BCMATTACHDATA(rstr_temp_offs2_2g)[] = "tpc_temp_offs2_2g";
static const char BCMATTACHDATA(rstr_temp_offs2_5g)[] = "tpc_temp_offs2_5g";
static const char BCMATTACHDATA(rstr_temp_diff)[] = "tpc_temp_diff";
static const char BCMATTACHDATA(rstr_vbat_high)[] = "tpc_vbat_hi";
static const char BCMATTACHDATA(rstr_vbat_offs1_2g)[] = "tpc_vbat_offs1_2g";
static const char BCMATTACHDATA(rstr_vbat_offs1_5g)[] = "tpc_vbat_offs1_5g";
static const char BCMATTACHDATA(rstr_vbat_low)[] = "tpc_vbat_low";
static const char BCMATTACHDATA(rstr_vbat_offs2_2g)[] = "tpc_vbat_offs2_2g";
static const char BCMATTACHDATA(rstr_vbat_offs2_5g)[] = "tpc_vbat_offs2_5g";
static const char BCMATTACHDATA(rstr_vbat_diff)[] = "tpc_vbat_diff";
static const char BCMATTACHDATA(rstr_RcalOtpValFlag)[] = "RcalOtpValFlag";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core0)[] = "rssicorrnorm_core0";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core0_5g1)[] = "rssicorrnorm_core0_5g1";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core0_5g2)[] = "rssicorrnorm_core0_5g2";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core0_5g3)[] = "rssicorrnorm_core0_5g3";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core1)[] = "rssicorrnorm_core1";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core1_5g1)[] = "rssicorrnorm_core1_5g1";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core1_5g2)[] = "rssicorrnorm_core1_5g2";
static const char BCMATTACHDATA(rstr_rssicorrnorm_core1_5g3)[] = "rssicorrnorm_core1_5g3";
static const char BCMATTACHDATA(rstr_rxgainerr5gha0)[] = "rxgainerr5gha0";
static const char BCMATTACHDATA(rstr_rxgainerr5gha1)[] = "rxgainerr5gha1";
static const char BCMATTACHDATA(rstr_rxgainerr5gla0)[] = "rxgainerr5gla0";
static const char BCMATTACHDATA(rstr_rxgainerr5gla1)[] = "rxgainerr5gla1";
static const char BCMATTACHDATA(rstr_rxgainerr5gma0)[] = "rxgainerr5gma0";
static const char BCMATTACHDATA(rstr_rxgainerr5gma1)[] = "rxgainerr5gma1";
static const char BCMATTACHDATA(rstr_rxgainerr5gua0)[] = "rxgainerr5gua0";
static const char BCMATTACHDATA(rstr_rxgainerr5gua1)[] = "rxgainerr5gua1";
static const char BCMATTACHDATA(rstr_triso5g_h_c0)[] = "triso5g_h_c0";
static const char BCMATTACHDATA(rstr_triso5g_h_c1)[] = "triso5g_h_c1";
static const char BCMATTACHDATA(rstr_triso5g_l_c0)[] = "triso5g_l_c0";
static const char BCMATTACHDATA(rstr_triso5g_l_c1)[] = "triso5g_l_c1";
static const char BCMATTACHDATA(rstr_triso5g_m_c0)[] = "triso5g_m_c0";
static const char BCMATTACHDATA(rstr_triso5g_m_c1)[] = "triso5g_m_c1";
static const char BCMATTACHDATA(rstr_TssiAv2g)[] = "TssiAv2g";
static const char BCMATTACHDATA(rstr_TssiAv5g)[] = "TssiAv5g";
static const char BCMATTACHDATA(rstr_tssifloor2ga0)[] = "tssifloor2ga0";
static const char BCMATTACHDATA(rstr_tssifloor2ga1)[] = "tssifloor2ga1";
static const char BCMATTACHDATA(rstr_tssifloor5ga0)[] = "tssifloor5ga0";
static const char BCMATTACHDATA(rstr_tssifloor5ga1)[] = "tssifloor5ga1";
static const char BCMATTACHDATA(rstr_tssifloor5gha0)[] = "tssifloor5gha0";
static const char BCMATTACHDATA(rstr_tssifloor5gha1)[] = "tssifloor5gha1";
static const char BCMATTACHDATA(rstr_tssifloor5gla0)[] = "tssifloor5gla0";
static const char BCMATTACHDATA(rstr_tssifloor5gla1)[] = "tssifloor5gla1";
static const char BCMATTACHDATA(rstr_tssioffsetmax)[] = "tssioffsetmax";
static const char BCMATTACHDATA(rstr_tssioffsetmin)[] = "tssioffsetmin";
static const char BCMATTACHDATA(rstr_TssiVmid2g)[] = "TssiVmid2g";
static const char BCMATTACHDATA(rstr_TssiVmid5g)[] = "TssiVmid5g";
static const char BCMATTACHDATA(rstr_txidxcap2g_hi)[] = "txidxcap2g_hi";
static const char BCMATTACHDATA(rstr_txidxcap5g_hi)[] = "txidxcap5g_hi";
static const char BCMATTACHDATA(rstr_txidxcap2g_lo)[] = "txidxcap2g_lo";
static const char BCMATTACHDATA(rstr_txidxcap5g_lo)[] = "txidxcap5g_lo";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_2g)[] = "rssi_gain_delta_2g";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_2gh)[] = "rssi_gain_delta_2gh";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_2ghh)[] = "rssi_gain_delta_2ghh";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gl)[] = "rssi_gain_delta_5gl";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gml)[] = "rssi_gain_delta_5gml";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gmu)[] = "rssi_gain_delta_5gmu";
static const char BCMATTACHDATA(rstr_rssi_gain_delta_5gh)[] = "rssi_gain_delta_5gh";
static const char BCMATTACHDATA(rstr_ed_assert_thresh_dbm)[] = "ed_assert_thresh_dbm";
static const char BCMATTACHDATA(rstr_eu_edthresh2g)[] = "eu_edthresh2g";
static const char BCMATTACHDATA(rstr_eu_edthresh5g)[] = "eu_edthresh5g";

/* %%%%%% Function implementation */
static INLINE void
wlc_phy_btcx_wlan_critical_enter_nphy(phy_info_t *pi)
{
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL, MHF1_WLAN_CRITICAL, WLC_BAND_2G);
}

static INLINE void
wlc_phy_btcx_wlan_critical_exit_nphy(phy_info_t *pi)
{
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL, 0, WLC_BAND_2G);
}

/* Start phy init code from d11procs.tcl */
/** Initialize the bphy in an nphy */
static void
WLBANDINITFN(wlc_phy_bphy_init_nphy)(phy_info_t *pi)
{
	uint16	addr, val;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(ISNPHY(pi));

	/* RSSI LUT is now a memory and must therefore be initialized */
	val = 0x1e1f;
	for (addr = (NPHY_TO_BPHY_OFF + BPHY_RSSI_LUT);
	     addr <= (NPHY_TO_BPHY_OFF + BPHY_RSSI_LUT_END); addr++) {
		PHY_TRACE(("wl%d: %s writing phy addr 0x%x with 0x%x\n", pi->sh->unit,
		          __FUNCTION__, addr, val));
		phy_utils_write_phyreg(pi, addr, val);
		if (addr == (NPHY_TO_BPHY_OFF + 0x97))
			val = 0x3e3f;
		else
			val -= 0x0202;
	}

	PHY_TRACE(("wl%d: %s, RSSI LUT done\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi)) {

		/* only for use on QT */
		PHY_REG_LIST_START_WLBANDINITDATA
			/* CRS thresholds */
			PHY_REG_WRITE_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_PHYCRSTH, 0x3206)
			/* RSSI thresholds */
			PHY_REG_WRITE_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_RSSI_TRESH, 0x281e)
			/* LNA gain range */
			PHY_REG_OR_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_LNA_GAIN_RANGE, 0x1a)
		PHY_REG_LIST_EXECUTE(pi);

	} else	{
		/* kimmer - add change from 0x667 to x668 very slight improvement */
		phy_utils_write_phyreg(pi, NPHY_TO_BPHY_OFF + BPHY_STEP, 0x668);
	}
	PHY_TRACE(("wl%d: %s, exiting\n", pi->sh->unit, __FUNCTION__));
}

void
wlc_phy_table_write_nphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset, uint32 width,
                     const void *data)
{
	mimophytbl_info_t tbl;
	/*	PHY_TRACE(("wlc_phy_table_write_nphy, id %d, len %d, offset %d, width %d\n",
		id, len, offset, width));
	*/

	bool suspend;
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;
	wlc_phy_write_table_nphy(pi, &tbl);

	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

}

void
wlc_phy_table_read_nphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset, uint32 width,
	void *data)
{
	mimophytbl_info_t tbl;
	/*	PHY_TRACE(("wlc_phy_table_read_nphy, id %d, len %d, offset %d, width %d\n",
		id, len, offset, width));
	*/
	bool suspend;
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;
	wlc_phy_read_table_nphy(pi, &tbl);

	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
WLBANDINITFN(wlc_phy_sparse_table_init)(phy_info_t *pi,
                                        uint8 table_id,
                                        CONST phytbl_init_t *offset_list,
                                        CONST uint32 *data_list)
{
	uint16 addr, num, ctr;
	uint8 num_zero_offsets, i;

	/* since the data array of non-zero init values corresponds to non-consecutive offsets,
	 * we need to keep track of pointed in this array
	 */
	ctr = 0;

	/* point to start of table */
	phy_utils_write_phyreg(pi, NPHY_TableAddress, (table_id << 10));

	/* keep going until we exhaust the blocks of offsets with non-zero init values */
	while ((num = offset_list->num) > 0) {

		if (offset_list->base != 0) {
			/* number of offsets that need to be init-ed to zero */
			num_zero_offsets = offset_list->base -
			        ((*(offset_list - 1)).base + (*(offset_list - 1)).num);

			/* loop to init these offsets to zero */
			for (i = 0; i < num_zero_offsets; i++) {
				PHY_REG_LIST_START_WLBANDINITDATA
					PHY_REG_WRITE_ENTRY(NPHY, TableDataHi, 0x0)
					PHY_REG_WRITE_ENTRY(NPHY, TableDataLo, 0x0)
				PHY_REG_LIST_EXECUTE(pi);
			}
		}

		/* loop over num(ber of consecutive offsets with non-zero values) */
		for (addr = offset_list->base; num; addr++, num--) {
			phy_utils_write_phyreg(pi, NPHY_TableDataHi,
			                       (uint16)(data_list[ctr] >> 16));
			phy_utils_write_phyreg(pi, NPHY_TableDataLo, (uint16)(data_list[ctr]));

			/* increment the entry in the data array of non-zero init values */
			ctr++;
		}

		/* move on to the next block of offsets that have non-zero init values */
		offset_list++;
	}

	/* start from the last offset with non-zero init value, and loop over the trailing offsets
	 * that are init-ed to zero
	 */
	num_zero_offsets = offset_list->base -
	        ((*(offset_list - 1)).base + (*(offset_list - 1)).num);

	for (i = 0; i < num_zero_offsets; i++) {
		PHY_REG_LIST_START_WLBANDINITDATA
			PHY_REG_WRITE_ENTRY(NPHY, TableDataHi, 0x0)
			PHY_REG_WRITE_ENTRY(NPHY, TableDataLo, 0x0)
		PHY_REG_LIST_EXECUTE(pi);
	}
}

static void
WLBANDINITFN(wlc_phy_compact_table_init)(phy_info_t *pi)
{
	uint8  core, ctr, chanest_tbl_block_len;
	uint16 offset;
	uint16 chanest_tbl_rev3_vals[2]   = {0x4444, 0x1010};
	uint16 chanest_tbl_rev3_offlen    = 9;
	uint16 noise_var_tbl_rev7_length  = 256;
	uint8  adj_pwr_lut_rev3_val       = 0;
	uint16 adj_pwr_lut_rev3_start     = 64;
	uint16 adj_pwr_lut_rev3_length    = 128;

	/* Noise var table (id 0xa): setup starting offset and then loop over init length,
	 * the offset is auto-incremented at every write
	 */
	phy_utils_write_phyreg(pi, NPHY_TableAddress, (NPHY_TBL_ID_NOISEVAR << 10));

	for (offset = 0; offset < noise_var_tbl_rev7_length; offset = offset + 2) {
		PHY_REG_LIST_START_WLBANDINITDATA
			/* write 0x20c020c */
			PHY_REG_WRITE_ENTRY(NPHY, TableDataHi, 0x020c)
			PHY_REG_WRITE_ENTRY(NPHY, TableDataLo, 0x020c)
			/* write 0x14d */
			PHY_REG_WRITE_ENTRY(NPHY, TableDataLo, 0x014d)
		PHY_REG_LIST_EXECUTE(pi);
	}

	/* Adjusted power per rates (offsets 0x40-0xbf of table ids 0x1a/0x1b).
	 * This table will be overwritten during PHY init anyway, but to be safe,
	 * all 128 entries are init-ed to 0 (even though only 84 entries are used).
	 */
	FOREACH_CORE(pi, core) {
		/* setup starting offset */
		phy_utils_write_phyreg(pi, NPHY_TableAddress, ((core == PHY_CORE_0 ?
		                                       NPHY_TBL_ID_CORE1TXPWRCTL :
		                                       NPHY_TBL_ID_CORE2TXPWRCTL) << 10)
		              | adj_pwr_lut_rev3_start);

		/* loop over the init length, the offset is auto-incremented at every write */
		for (offset = 0; offset < adj_pwr_lut_rev3_length; offset++)
		              phy_utils_write_phyreg(pi, NPHY_TableDataLo, adj_pwr_lut_rev3_val);
	}

	/* The frame struct table is sparse, so we can avoid listing all zero init values
	 * and instead use a list of offsets of non-zero init values and how many there are
	 */
	wlc_phy_sparse_table_init(pi, NPHY_TBL_ID_FRAME_STRUCT, frame_struct_rev8_offsets,
	                          frame_struct_rev8_redux);

	/* The tone mapper table is sparse, so we can avoid listing all zero init values
	 * and instead use a list of offsets of non-zero init values and how many there are
	 */
	wlc_phy_sparse_table_init(pi, NPHY_TBL_ID_TONE_MAPPER, tmap_tbl_rev7_offsets,
	                          tmap_tbl_rev7_redux);

	/* Chanest table contains only blocs of 0x44444444 or 0x10101010 */
	phy_utils_write_phyreg(pi, NPHY_TableAddress, (NPHY_TBL_ID_CHANEST << 10));

	for (ctr = 0; ctr < chanest_tbl_rev3_offlen - 1; ctr++) {

		/* determine the length of the block to init */
		chanest_tbl_block_len = chanest_tbl_rev3_offsets[ctr + 1] -
		        chanest_tbl_rev3_offsets[ctr];

		/* init this block with either 0x44444444 or 0x10101010 */
		for (offset = 0; offset < chanest_tbl_block_len; offset++) {
			phy_utils_write_phyreg(pi, NPHY_TableDataHi,
			                       chanest_tbl_rev3_vals[ctr % 2]);
			phy_utils_write_phyreg(pi, NPHY_TableDataLo,
			                       chanest_tbl_rev3_vals[ctr % 2]);
		}
	}
}

/**
 * Initialize the static tables defined in auto-generated mimophytbls.c,
 * see mimophyprocs.tcl, proc mimophy_init_tbls
 * After called in the attach stage, all the static phy tables are reclaimed.
 */
static void
WLBANDINITFN(wlc_phy_static_table_download_nphy)(phy_info_t *pi)
{
	uint idx;

	/* these tables are not affected by phy reset, only power down */
	/* LCNXN */
	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		for (idx = 0; idx < mimophytbl_info_sz_rev16; idx++)
			wlc_phy_write_table_nphy(pi, &mimophytbl_info_rev16[idx]);

	} else if (NREV_IS(pi->pubpi->phy_rev, 8) || NREV_IS(pi->pubpi->phy_rev, 9)) {

		if ((CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
			(CHIPID(pi->sh->chip) == BCM43235_CHIP_ID) ||
			(CHIPID(pi->sh->chip) == BCM43236_CHIP_ID)) {
			for (idx = 0; idx < mimophytbl_info_sz_43236; idx++)
				wlc_phy_write_table_nphy(pi, &mimophytbl_info_43236[idx]);

		} else {
			for (idx = 0; idx < mimophytbl_info_sz_rev8to10; idx++)
				wlc_phy_write_table_nphy(pi, &mimophytbl_info_rev8to10[idx]);

			wlc_phy_compact_table_init(pi);
		}

	} else {
		for (idx = 0; idx < mimophytbl_info_sz_rev7; idx++)
			wlc_phy_write_table_nphy(pi, &mimophytbl_info_rev7[idx]);

		wlc_phy_compact_table_init(pi);
	}
}

/**
 * Given a map and value array, the length of these arrays and a core_offset, iterate over both
 * arrays and get the index from the map array with the accompaning value from the value array. Then
 * write the value to the antswctrllut LUT at offset = core_offset + index.
 */
static void
WLBANDINITFN(wlc_phy_set_antswctrllut_nphy)(phy_info_t *pi,
                                            const uint8 *map, const uint8 *vals,
                                            uint8 len, uint8 core_offset)
{
	uint8 i;
	uint32 offset;

	for (i = 0; i < len; i++) {
		offset = core_offset + map[i];
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
		                         1, offset, 8, &vals[i]);
	}
}

/**
 * initialize all the tables defined in auto-generated mimophytbls.c,
 * see mimophyprocs.tcl, proc mimophy_init_tbls. Skip static one after first up.
 */
static void
WLBANDINITFN(wlc_phy_tbl_init_nphy)(phy_info_t *pi)
{
	uint8 antswctrllut;
	uint8 Noffset = 0;
	uint8 Nmap = 0;
	uint8 offset_band = 0;
	uint8 ct1 = 0, ct2 = 0;
	uint32 offset;

	PHY_TRACE(("wl%d: %s, dnld tables = %d\n", pi->sh->unit,
	          __FUNCTION__, pi->phy_init_por));

	/* these tables are not affected by phy reset, only power down */
	if (pi->phy_init_por)
		wlc_phy_static_table_download_nphy(pi);

	offset_band = CHSPEC_IS2G(pi->radio_chanspec) ? 0 : 16;

	/* only AntSwCtrlLUT is a volatile table */
	antswctrllut = CHSPEC_IS2G(pi->radio_chanspec) ?
	        pi->fem2g->antswctrllut : pi->fem5g->antswctrllut;

	switch (antswctrllut) {
	case 0: {
		/* 43226, 43236, 6362 */
		uint8 offset_core[] = {0, 32}; /* offsets for 2G core0, 2G core 1 */

		uint8 map[] = {0, 4, 8};
		uint8 antswctrllut_vals[2][3] = {{0x2, 0x12, 0x8}, {0x2, 0x18, 0x2}};

		if (CHIPID(pi->sh->chip) == BCM43235_CHIP_ID) {
			/* for 43235, it was found that putting TR switch on T for
			   unused core during SISO TX helps with HD2/HD3 rejection.
			*/
			antswctrllut_vals[0][0] = 0x1;
			antswctrllut_vals[1][0] = 0x1;
		}

		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));

		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2] + offset_band;
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				                         1, offset, 8,
				                         &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
	}
	case 1: {
		/* for 2G: 5357, 5358, 47186
		   difference is in LUT entries for Core 1 because
		   mimophy_core1_ant1_[rx,tx] instead of mimophy_core1_ant0_[rx,tx]
		   are the dedicated pins5357, 47186, 5358, 5356
		*/

		uint8 map0[] = {0, 4, 8};
		uint8 antswctrllut_vals0[] = {0x1, 0x11, 0x1};

		uint8 map1[] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
		uint8 antswctrllut_vals1[] = {0x4, 0x04, 0x08, 0x4, 0x04, 0x08, 0x11, 0x11, 0x12};

		if (pi->aa2g == 7) {
			/* 3 available antennas
			 * ensure 3rd switch is always in a defined state
			 */
			antswctrllut_vals1[0] = 0x14;
			antswctrllut_vals1[1] = 0x14;
			antswctrllut_vals1[2] = 0x18;
		}

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		break;
	}
	case 2: {
		/* for Olympic Sulley using 43236B1
		 * When in RX mode in 2.4G, look up entries 13, 14, and 45, 46
		 * for cores 0 and 1. Entries 13 and 45 will put ext. LNA in bypass
		 * mode instead of the traditional approach of throwing TR to T
		 * at high enough RX powers
		 * Here, write all 2G, 5G entries that're different from default in one shot
		 */
		uint8 offset_core[] = {0, 32, 16, 48};
		uint8 map[] = {0, 1, 2, 3, 12, 13, 14, 15};
		uint8 antswctrllut_vals[4][8] = {
			{0xa,  0x6,  0xa,  0xa,  0x2,  0x2,  0xa,  0xa}, /* 2G core 0 */
			{0xa,  0x6,  0xa,  0x12, 0x2,  0x2,  0xa,  0x12}, /* 2G core 1 */
			{0xa,  0x9,  0xa,  0xa,  0xa,  0x9,  0xa,  0xa}, /* 5G core 0 */
			{0x12, 0x11, 0x12, 0x12, 0x12, 0x11, 0x12, 0x12} /* 5G core 1 */
		};

		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));
		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2];
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				                         1, offset, 8,
				                         &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
	}
	case 3: {
		/* for 47186nrh that uses 2 SPDT switches */

		uint8 offset_core[] = {0, 32, 16, 48};

		uint8 map[] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
		uint8 antswctrllut_vals[4][9] = {
			{0x0A, 0x05, 0x06, 0x1A, 0x19, 0x1A, 0x0A, 0x05, 0x06},
			{0x1A, 0x15, 0x09, 0x0A, 0x05, 0x09, 0x1A, 0x16, 0x1A},
			{0x02, 0x01, 0x02, 0x12, 0x14, 0x18, 0x02, 0x01, 0x02},
			{0x12, 0x11, 0x12, 0x02, 0x01, 0x02, 0x12, 0x14, 0x18}
		};

		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));

		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2];
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				                         1, offset, 8,
				                         &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
	}
	case 4: {
		/* for Olympic Sulley DEV2 using 43236B1
		 * When in RX mode in 2.4G *and* 5G, look up entries +13, +14
		 * for cores 0 and 1.
		 * Here, write all 2G, 5G entries that're different from default in one shot
		 */
		uint8 offset_core[] = {0, 32, 16, 48};
		uint8 map[] = {0, 1, 2, 3, 12, 13, 14, 15};
		uint8 antswctrllut_vals[4][8] = {
			{0xa,  0x6,  0xa,  0xa,  0x2,  0x2,  0xa,  0xa}, /* 2G core 0 */
			{0xa,  0x6,  0xa,  0x12, 0x2,  0x2,  0xa,  0x12}, /* 2G core 1 */
			{0xa,  0x9,  0xa,  0xa,  0xa,  0xb,  0xa,  0xa}, /* 5G core 0 */
			{0x12, 0x11, 0x12, 0x12, 0x12, 0x13, 0x12, 0x12} /* 5G core 1 */
		};

		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));
		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2];
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				                         1, offset, 8,
				                         &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
	}
	case 5: {
		/* for preproto blu2o3 */

		uint8 offset_core[] = {0, 32}; /* offsets for 2G core0, 2G core 1 */

		uint8 map[] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
		uint8 antswctrllut_vals[2][9] = {
			{0x16, 0x15, 0x16, 0x1a, 0x19, 0x1a, 0x16, 0x05, 0x06},
			{0x16, 0x05, 0x06, 0x16, 0x05, 0x06, 0x1a, 0x19, 0x1a}
		};

		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));

		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2] + offset_band;
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				                         1, offset, 8,
				                         &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
	}
	case 6:{
	        /* for BCM943234ug(2G) and BCM43234usb_div(2G & 5G) */
		uint8 offset_core = 32; /* Only core1 & G-band is in use. */
		uint8 map[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		uint8 antswctrllut_vals[12] =
			{0x0, 0x5, 0x6, 0x5, 0x0, 0x5, 0x6, 0x5, 0x0, 0x9, 0xA, 0x9};

		Nmap = (sizeof(map) / sizeof(map[0]));

		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = offset_core + map[ct2] + offset_band;
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT, 1, offset,
			                         8, &antswctrllut_vals[ct2]);
		}
		break;
	}
	case 7: {
		/* BCM94324A0 ePA+eLNA FCBGA BRCM BU Board 2G(bcm94324epaBU) */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x0a, 0x0a, 0x09, 0x08};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x14, 0x10, 0x14, 0x10};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		break;
	}
	case 8: {
		/* BCM94324A0 ePA+eLNA FCBGA BRCM BU Board 5G(bcm94324epaBU) */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x10, 0x08, 0x12, 0x0a};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x20, 0x10, 0x20, 0x10};

		/* 5G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 5G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		break;
	}
	case 9: {
		/* 4324 2g */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x12, 0x12, 0x11, 0x14};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x28, 0x24, 0x28, 0x24};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43241IPAAGB);
		break;
	}
	case 10: {
		/* 4324 5g */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x0c, 0x14, 0x0a, 0x12};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x14, 0x24, 0x14, 0x24};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43241IPAAGB);
		break;
	}
	case 11: {
		/* for usb elna  using 43236B1
		 * When in RX mode in 2.4G, look up entries 13, 14, and 45, 46
		 * for cores 0 and 1. Entries 13 and 45 will put ext. LNA in bypass
		 * mode instead of the traditional approach of throwing TR to T
		 * at high enough RX powers
		 * Here, write all 2G, 5G entries that're different from default in one shot
		 */
		uint8 offset_core[] = {0, 32, 16, 48};
		uint8 map[] = {12, 13, 14};
		uint8 antswctrllut_vals[4][3] = {
			{ 0x2,  0x11,  0x12}, /* 2G core 0 */
			{ 0x2,  0x11,  0x12}, /* 2G core 1 */
			{ 0x2,  0x11,  0x12}, /* 5G core 0 */
			{ 0x2,  0x11,  0x12}  /* 5G core 1 */
		};
		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));
		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2];
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT, 1,
					offset, 8, &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
	}
	case 12: {
		/* bcm94324epaabg 2g */
		uint8 map0[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals0[] = {0x0A, 0x28, 0x2A, 0x0A, 0x28, 0x2A};

		uint8 map1[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals1[] = {0x28, 0x20, 0x03, 0x28, 0x20, 0x03};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		/* set bit1 to mux out rf_sw_ctrl_5 from LUT */
		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x1a);

		/* to goto LUT 11 state in Hi SNR region, only for core0,
		* since only core0 LNA PU comes from LUT,
		* core1 comes from PHy direct signal
		*/
		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_0_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_0_SHIFT);
		break;
	}
	case 13: {
		/* bcm94324epaabg 5g */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x10, 0x08, 0x12, 0x0a};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x10, 0x20, 0x10, 0x20};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		break;
	}

	/* case 14 is in only TCL: 43238ucg */

	case 15:
		{
		/* for 238UM
		 */
		uint8 offset_core[] = {0, 32, 16, 48};
		uint8 map[] = {0, 1, 2, 3};
		uint8 antswctrllut_vals[4][4] = {
		  { 0x01,  0x10,  0x03, 0x12}, /* 2G core 0 */
		  { 0x01,  0x10,  0x03, 0x12}, /* 2G core 1 */
		  { 0x01,  0x05,  0x09, 0x01}, /* 5G core 0 */
		  { 0x01,  0x05,  0x09, 0x01}  /* 5G core 1 */
		};
		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));
		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2];
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
					1, offset, 8, &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
		}
	case 16: {
		/* Adding SW CTRL support for BCM943228hm4l-P500 boards where,the  */
		/* TX & RX sw ctrl lines are swapped */
		uint8 offset_core[] = {0, 32, 16, 48};
		uint8 map[] = {0, 1, 2, 4, 8};
		uint8 antswctrllut_vals[4][5] = {
			{0x1,  0x2,  0x1, 0x12, 0x8}, /* 2G core 0 */
			{0x2,  0x1,  0x2, 0x18, 0x2}, /* 2G core 1 */
			{0x1,  0x2,  0x1, 0x12, 0x8}, /* 5G core 0 */
			{0x2,  0x1,  0x2, 0x18, 0x2} /* 5G core 1 */
		};

		Noffset = (sizeof(offset_core) / sizeof(offset_core[0]));
		Nmap = (sizeof(map) / sizeof(map[0]));
		for (ct1 = 0; ct1 < Noffset; ct1++) {
			for (ct2 = 0; ct2 < Nmap; ct2++) {
				offset = offset_core[ct1] + map[ct2];
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
					1, offset, 8, &antswctrllut_vals[ct1][ct2]);
			}
		}
		break;
	}
	case 17: {
		/* 4324 2g */
		/* For BCM943241 iPA+eLNA WLBGA Module AZW - 2G(bcm943241wlmdazw) */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x09, 0x2c, 0x09, 0x2c};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x14, 0x18, 0x14, 0x18};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		/* set bit1 to mux out rf_sw_ctrl_5 from LUT */
		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x1a);

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43241IPAAGB_eLNA);
		break;
	}
	case 18: {
		/* 4324 5g */
		/* For BCM943241 iPA+eLNA WLBGA Module AZW - 2G(bcm943241wlmdazw) */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x12, 0x0a, 0x14, 0x0c};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x28, 0x18, 0x28, 0x18};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43241IPAAGB_eLNA);
		break;
	}
	case 19: {
		/* 43242A0 2g */
		/* For BCM943242A0 iPA+iLNA FCBGA BRCM BU board */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x21, 0x24, 0x22, 0x22};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x02, 0x01, 0x02, 0x01};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		break;
	}
	case 20: {
		/* 43242A0 5g */
		/* For BCM943242A0 iPA+iLNA FCBGA BRCM BU board */
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x10, 0x20, 0x12, 0x22};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x01, 0x02, 0x01, 0x02};

		/* 5G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 5G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
				1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		break;
	}
	case 21: {
		/* 43242A0 2G */
		/* For BCM943242USBREF iPA+iLNA FCBGA BRCM REF board */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x2e, 0x2e, 0x29, 0x25};
		const uint8 vals1[4] = {0x02, 0x01, 0x02, 0x01};

		/* 2G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 2G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43242USBREF);
		break;
	}
	case 22: {
		/* 43242A0 5G */
		/* For BCM943242USBREF iPA+iLNA FCBGA BRCM REF board */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x1a, 0x26, 0x14, 0x24};
		const uint8 vals1[4] = {0x01, 0x02, 0x01, 0x02};

		/* 5G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 5G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x3);

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43242USBREF);
		break;
	}
	case 23: {
		/* BCM943241 iPA+eLNA FCBGA SS P10 rev0.2 COB - 2G
		 * for 5G its same as ipaagb ref board (0xa OR case 10)
		 */
		uint8 map0[] = {1, 2, 3, 17, 18};
		uint8 antswctrllut_vals0[] = {0x11, 0x2c, 0x29, 0x11, 0x2c};

		uint8 map1[] = {1, 2, 3, 17, 18};
		uint8 antswctrllut_vals1[] = {0x28, 0x14, 0x3, 0x28, 0x14};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}

		/* set bit1 to mux out rf_sw_ctrl_5 from LUT */
		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x1a);

		/* to goto LUT 11 state in Hi SNR region, only for core0,
		* since only core0 LNA PU comes from LUT,
		* core1 comes from PHy direct signal
		*/
		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_0_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_0_SHIFT);

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43241IPAAGB_eLNA);
		break;
	}
	case 24: {
		/* BCM94324B1 ePA+eLNA FCBGA BRCM REF Board(efoagb COB)
		 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* For FEM SKY65534-11 for 2g
		*/
		uint8 map0[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals0[] = {0x08, 0x29, 0x09, 0x08, 0x29, 0x09};

		uint8 map1[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals1[] = {0x10, 0x14, 0x14, 0x10, 0x14, 0x14};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}

		/* set bit1 to mux out rf_sw_ctrl_5 from LUT */
		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x1a);

		/* to goto LUT 11 state in Hi SNR region, only for core0,
		* since only core0 LNA PU comes from LUT,
		* core1 comes from PHy direct signal
		*/
		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_0_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_0_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_1_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_1_SHIFT);
		} else {
		/* For FEM SKY65535 for 5g
		*/
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x01, 0x09, 0x01, 0x09};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x04, 0x14, 0x04, 0x14};

		/* 5G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 5G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		}
		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
			LCNXN_SWCTRL_MASK_4324B1EFOAGB);

		break;
	}
	case 25: {
		/* 43242A0 2G */
		/* For BCM943242MD5ANT "5 antenna board" */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x21, 0x21, 0x22, 0x21};
		const uint8 vals1[4] = {0x02, 0x01, 0x02, 0x01};

		/* 2G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 2G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		break;
	}
	case 26: {
		/* 43242A0 5G */
		/* For BCM943242MD5ANT "5 antenna board" */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x11, 0x21, 0x11, 0x21};
		const uint8 vals1[4] = {0x01, 0x02, 0x01, 0x02};

		/* 5G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 5G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x3);

		break;
	}
	case 27: {
		/* BCM943241 iPA+eLNA FCBGA SS P10 rev0.2 COB - 5G
		 */
		uint8 map0[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals0[] = {0x0c, 0x14, 0x4, 0x0a, 0x12, 0x2};

		uint8 map1[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals1[] = {0x14, 0x24, 0x4, 0x14, 0x24, 0x4};

		/* 5G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 5G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}

		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_0_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_0_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_1_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_1_SHIFT);

		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
		                     LCNXN_SWCTRL_MASK_43241IPAAGB_eLNA);
		break;
	}
	case 28: {
		/* 43242A0 P303+ 2G */
		/* For BCM943242USBREF iPA+iLNA FCBGA BRCM REF board with seperate BT-Tx */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x36, 0x36, 0x39, 0x35};
		const uint8 vals1[4] = {0x02, 0x01, 0x02, 0x01};

		/* 2G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 2G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		break;
	}
	case 29: {
		/* 43242A0 P303+ 5G */
		/* For BCM943242USBREF iPA+iLNA FCBGA BRCM REF board with seperate BT-Tx */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x16, 0x26, 0x15, 0x25};
		const uint8 vals1[4] = {0x01, 0x02, 0x01, 0x02};

		/* 5G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 5G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x3);

		break;
	}
	case 30: {
		/* BCM94324B1 ePA+eLNA FCBGA BRCM REF Board(efoagb COB)
		 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* For FEM SKY65534-11 for 2g
		*/
		uint8 map0[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals0[] = {0x08, 0x29, 0x09, 0x08, 0x29, 0x09};

		uint8 map1[] = {1, 2, 3, 17, 18, 19};
		uint8 antswctrllut_vals1[] = {0x10, 0x1c, 0x14, 0x10, 0x1c, 0x14};

		/* 2G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 2G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}

		/* set bit1 to mux out rf_sw_ctrl_5 from LUT */
		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x1a);

		/* to goto LUT 11 state in Hi SNR region, only for core0,
		* since only core0 LNA PU comes from LUT,
		* core1 comes from PHy direct signal
		*/
		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_0_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_0_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_Override_TR_rx_pu_high_gain,
			NPHY_REV19_high_gain_ovr_tr_rx_pu_1_MASK,
			1 << NPHY_REV19_high_gain_ovr_tr_rx_pu_1_SHIFT);
		} else {
		/* For FEM SKY65535 for 5g
		*/
		uint8 map0[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals0[] = {0x01, 0x09, 0x01, 0x09};

		uint8 map1[] = {1, 2, 17, 18};
		uint8 antswctrllut_vals1[] = {0x04, 0x14, 0x04, 0x14};

		/* 5G core 0 */
		Nmap = (sizeof(map0) / sizeof(map0[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = map0[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals0[ct2]);
		}

		/* 5G core 1 */
		Nmap = (sizeof(map1) / sizeof(map1[0]));
		for (ct2 = 0; ct2 < Nmap; ct2++) {
			offset = 32 + map1[ct2];
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT,
			                         1, offset, 8, &antswctrllut_vals1[ct2]);
		}
		}
		wlapi_bmac_write_shm(pi->sh->physhim, M_LCNXN_SWCTRL_MASK(pi),
			LCNXN_SWCTRL_MASK_4324B1EFOAGB);

		break;
	}
	case 31: {
		/* 43242A0 P400+ 2G */
		/* For BCM943242USBREF iPA+iLNA FCBGA BRCM REF board with seperate BT-Tx */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x36, 0x36, 0x39, 0x35};
		const uint8 vals1[4] = {0x06, 0x05, 0x06, 0x05};

		/* 2G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 2G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		break;
	}
	case 32: {
		/* 43242A0 P400+ 5G */
		/* For BCM943242USBREF iPA+iLNA FCBGA BRCM REF board with seperate BT-Tx */
		const uint8 map[4]   = {   1,    2,   17,   18};
		const uint8 vals0[4] = {0x16, 0x26, 0x15, 0x25};
		const uint8 vals1[4] = {0x05, 0x02, 0x05, 0x02};

		/* 5G core 0 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals0, ARRAYSIZE(map), 0);

		/* 5G core 1 */
		wlc_phy_set_antswctrllut_nphy(pi, map, vals1, ARRAYSIZE(map), 32);

		phy_utils_write_phyreg(pi, NPHY_REV19_swCtrlLUTConfig, 0x3);

		break;

	}

	default:
		/* Illegal value. Don't silently ignore an error. */
		ASSERT(0);
		break;
	}
}

static void
wlc_phy_write_txmacreg_nphy(phy_info_t *pi, uint16 holdoff, uint16 delay_val)
{
	phy_utils_write_phyreg(pi, NPHY_TxMacIfHoldOff, holdoff);
	phy_utils_write_phyreg(pi, NPHY_TxMacDelay, delay_val);
}

void wlc_phy_nphy_tkip_rifs_war(phy_info_t *pi, uint8 rifs)
{
	uint16 holdoff, delay_val;

	/* if rifs enabled, don't increase PHY holdoff time */
	if (rifs) {
		/* default PHY holdoff time */
		holdoff = 0x10;
		delay_val = 0x258;
	}
	else {
		/* increased PHY holdoff time */
		holdoff = 0x15;
		delay_val = 0x320;
	}
	/* write to phy registers */
	wlc_phy_write_txmacreg_nphy(pi, holdoff, delay_val);

	/* update the hw rifs flag if necesary */
	if (pi->sh->_rifs_phy != rifs) {
		pi->sh->_rifs_phy = rifs;
	}
}

bool
BCMATTACHFN(wlc_phy_attach_nphy)(phy_info_t *pi)
{
	phy_info_nphy_t *pi_nphy;
	shared_phy_t *sh;
	uint i;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(pi);
	if (!pi) {
		PHY_ERROR(("wl: wlc_phy_attach_nphy: NULL pi\n"));
		return FALSE;
	}

	sh = pi->sh;

	if ((pi->u.pi_nphy = (phy_info_nphy_t *) MALLOC(sh->osh,
		sizeof(phy_info_nphy_t))) == NULL) {
		PHY_ERROR(("wl%d: wlc_phy_attach_nphy: out of memory, malloced %d bytes\n",
			sh->unit, MALLOCED(sh->osh)));
		return FALSE;
	}
	bzero((char*)pi->u.pi_nphy, sizeof(phy_info_nphy_t));
	pi_nphy = (void *)pi->u.pi_nphy;

#ifdef WLSRVSDB
	phy_sr_vsdb_reset((wlc_phy_t *)pi);
#endif /* WLSRVSDB */

	/* Set spur mode to 0 */
	pi->phy_spuravoid_mode = 0;

	/* Extract xtal frequency */
	pi->xtalfreq = si_alp_clock(pi->sh->sih);

	/* enable initcal by default */
	pi->u.pi_nphy->do_initcal = TRUE;

	pi->u.pi_nphy->cal_type_override = PHY_PERICAL_AUTO;
	/* Reset the saved noisevar count */
	pi->u.pi_nphy->nphy_saved_noisevars.bufcount = 0;

	/* Initialize to -1 to indicate that rx2tx table wasn't modified
	* to NOP the CLR_RXTX_BIAS entry
	*/
	pi->u.pi_nphy->rx2tx_biasentry = -1;

	((phy_info_nphy_t *)pi_nphy)->nphy_base_nvars_adjusted = FALSE;

	/* Check if extra G-band spur WAR for REV6 40MHz channels 3 through 10
	 * or REV7 all 2G channels (for 43236 only) should be enabled for this board.
	 */
	if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_2G_SPUR_WAR) {
		pi_nphy->nphy_gband_spurwar2_en = TRUE;
	}

	pi->n_preamble_override = AUTO;
	pi->nphy_txrx_chain = AUTO;
	pi->phy_scraminit = AUTO;
	/* Use T (24-31), Couple On (16-23), 181 tone amplitude (0-15) */
	pi_nphy->nphy_rxcalparams = 0x010100B5;

	pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_IDLE;
	pi->def_cal_info->txcal_numcmds = MPHASE_TXCAL_NUMCMDS;

	pi->nphy_elna_gain_config = FALSE;
	pi->radio_is_on = FALSE;
	pi_nphy->nphy_papd_kill_switch_en = FALSE;
	pi_nphy->ppr_offsets_copied = 0;

	pi_nphy->firstTime = FALSE;
#ifdef RXIQCAL_FW_WAR
	pi_nphy->nphy_rxiqcal_fw_war_en = TRUE;
#else
	pi_nphy->nphy_rxiqcal_fw_war_en = FALSE;
#endif // endif

#if defined(WLTEST) || defined(DBG_PHY_IOV)
	/* aci non assoc mode saity */
	/* usage: ./wl aci_nams X
	 * X = 1: allowed
	 * X = 0: not allowed
	 */
	pi->aci_nams = 0;
#endif // endif

	if ((PHY_GETVAR(pi, rstr_subband5gver)) != NULL)
		pi->sromi->subband5Gver = (uint)PHY_GETINTVAR(pi, rstr_subband5gver);
	else
		pi->sromi->subband5Gver = PHY_SUBBAND_3BAND_JAPAN;

	/* disable_spuravoid read from nvram */
	if ((PHY_GETVAR(pi, rstr_disable_spuravoid)) != NULL)
		pi->sh->disable_spuravoid = (uint8)PHY_GETINTVAR(pi, rstr_disable_spuravoid);
	else
		pi->sh->disable_spuravoid = 0;

	for (i = 0; i < pi->pubpi->phy_corenum; i++) {
		pi_nphy->nphy_txpwrindex[i].index = AUTO;
	}

	wlc_phy_txpwrctrl_config_nphy(pi);
	if (pi->nphy_txpwrctrl == PHY_TPC_HW_ON)
		pi->hwpwrctrl_capable = TRUE;

	/* To enable/disable Low power ADC mode for TX IQ Cal */
	pi->sromi->iqcal_lowpwradc = 0;
	pi->sromi->iqcal_adcclampdisable = 0;

	if (PHY_GETVAR(pi, rstr_elna2g)) {
		/* extlnagain2g entry exists, so use it. */
		pi->u.pi_nphy->elna2g = (uint8)PHY_GETINTVAR(pi, rstr_elna2g);
	}

#ifdef BAND5G
	if (PHY_GETVAR(pi, rstr_elna5g)) {
		/* extlnagain5g entry exists, so use it. */
		pi->u.pi_nphy->elna5g = (uint8)PHY_GETINTVAR(pi, rstr_elna5g);
	}
#endif /* BAND5G */

	if (NREV_GE(pi->pubpi->phy_rev, 8) && (pi->sh->sromrev >= 9)) {
		if (wlc_phy_srom_read_nphy(pi)) {
			/* Draconian Power Limits for Sulley */
			pi->u.pi_nphy->tssi_ladder_offset_maxpwr =
				(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmax, 4);
			pi->u.pi_nphy->tssi_ladder_offset_minpwr =
				(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssioffsetmin, 3);
		} else
			goto exit;

	} else {
		if (!wlc_phy_srom_read_nphy(pi))
			goto exit;
	}

	/* Read from SROM/NVRAM. Above one is only for 4324(nvram) */
	pi->srom_eu_edthresh2g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_eu_edthresh2g, 0);
	pi->srom_eu_edthresh5g = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_eu_edthresh5g, 0);

	/* gn_tbl_mode (indicates pwridx resolution)
	 * 0 - 0.25db step size
	 * 1 - 0.50db step size
	 */
	pi_nphy->nphy_txGainTable_mode = 0;

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, wlc_phy_watchdog_nphy, pi,
		PHY_WD_PRD_1TICK, PHY_WD_GLACIAL_CAL, PHY_WD_FLAG_DEF_DEFER) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto exit;
	}

	/* initialize the interference mitigation rssi values */
	/* -60: any number between -30 and -90 */
	for (i = 0; i < PHY_RSSI_WINDOW_SZ; i++) {
			pi_nphy->intf_rssi_vals[i] = PHY_INTF_RSSI_INIT_VAL;
	}

	pi_nphy->crsmin_rssi_avg_acioff_2G = PHY_CRSMIN_GE7_ACIOFF_2G;
	pi_nphy->crsmin_rssi_avg_acion_2G = PHY_CRSMIN_GE7_ACION_2G;
	pi_nphy->crsmin_pwr_aci2g[0] = PHY_CRSMIN_ACI2G_PWR_0;
	pi_nphy->crsmin_pwr_aci2g[1] = PHY_CRSMIN_ACI2G_PWR_1;
	pi_nphy->crsmin_pwr_aci2g[2] = PHY_CRSMIN_ACI2G_PWR_2;

	pi_nphy->rfgain_rssi_avg_acioff_2G = PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_2G;
	pi_nphy->rfgain_rssi_avg_acioff_2G_max = PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_2G_MAX;
	pi_nphy->rfgain_rssi_avg_acioff_5G = PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_5G;
	pi_nphy->rfgain_rssi_avg_acioff_5G_max = PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_5G_MAX;

	pi_nphy->rfgain_rssi_avg_acion_2G = PHY_RFGAIN_RSSI_AVG_GE7_ACION_2G;
	pi_nphy->rfgain_rssi_avg_acion_2G_max = PHY_RFGAIN_RSSI_AVG_GE7_ACION_2G_MAX;

	pi_nphy->rfgain_rssi_avg_acion_5G = PHY_RFGAIN_RSSI_AVG_GE7_ACION_5G;
	pi_nphy->rfgain_rssi_avg_acion_5G_max = PHY_RFGAIN_RSSI_AVG_GE7_ACION_5G_MAX;

#ifdef ATE_BUILD
	pi->pi_fptr->gpaioconfigptr = NULL;
#endif // endif
#if defined(RXDESENS_EN)
	pi_nphy->ntd_current_rxdesens = 0;
	pi_nphy->ntd_save_current_rxdesens = 0;
#endif // endif
	pi_nphy->ed_assert_thresh_dbm =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_ed_assert_thresh_dbm, 0);

#if defined(BCMDBG)
	phy_dbg_add_dump_fn(pi, "phypapd", wlc_phydump_papd, pi);
	phy_dbg_add_dump_fn(pi, "phylnagain", wlc_phydump_lnagain, pi);
	phy_dbg_add_dump_fn(pi, "phyinitgain", wlc_phydump_initgain, pi);
	phy_dbg_add_dump_fn(pi, "phyhpf1tbl", wlc_phydump_hpf1tbl, pi);
#endif // endif

	return TRUE;

exit:
	MFREE(pi->sh->osh, pi_nphy, sizeof(phy_info_nphy_t));
	pi->u.pi_nphy = NULL;

	return FALSE;
}

static void
BCMATTACHFN(wlc_phy_txpwrctrl_config_nphy)(phy_info_t *pi)
{
	pi->nphy_txpwrctrl = PHY_TPC_HW_ON;
	pi->phy_5g_pwrgain = TRUE;
}

/** Function to enable external LNA ctrl if dedicated pins are not available */
void wlc_phy_enable_extlna_ctrl_nphy(phy_info_t *pi)
{
	if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
	    (CHIPID(pi->sh->chip) == BCM43238_CHIP_ID)) {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & (BFL_EXTLNA_5GHz | BFL_EXTLNA)) {
			si_corereg(pi->sh->sih, SI_CC_IDX,
			           OFFSETOF(chipcregs_t, chipcontrol), 0x44, 0x04);
		}
	}
}

/** Function to enable shared ant lines */
void wlc_phy_enable_shared_ant_nphy(phy_info_t *pi)
{
	if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) || (pi->sh->chip == BCM43238_CHIP_ID))
		if ((pi->fem2g->antswctrllut == 11) || (pi->fem5g->antswctrllut == 11) ||
		(pi->fem2g->antswctrllut == 15) ||(pi->fem5g->antswctrllut == 15))
		{
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
				CCTRL43236_ANT_MUX_2o3, CCTRL43236_ANT_MUX_2o3);
		}
}

/** Function to enable external PA ctrl if dedicated pins are not available */
void wlc_phy_enable_extpa_ctrl_nphy(phy_info_t *pi)
{
	if (!PHY_IPA(pi)) {
		if (CHIPID(pi->sh->chip) == BCM43131_CHIP_ID) {
			si_pmu_chipcontrol(pi->sh->sih, 1, CCTRL5357_EXTPA, CCTRL5357_EXTPA);
		}

		/* Set 43217 to enable the toggling of EXT_PA pins */
		if ((CHIPID(pi->sh->chip) == BCM43217_CHIP_ID) &&
		    (!(BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_ANAPACTRL_2G))) {
			si_pmu_chipcontrol(pi->sh->sih, 1,
				CCTRL43217_EXTPA_C0 | CCTRL43217_EXTPA_C1,
				CCTRL43217_EXTPA_C0 | CCTRL43217_EXTPA_C1);
		}

		if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
			(CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
		    (CHIPID(pi->sh->chip) == BCM43238_CHIP_ID)) {
			if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
			(BFL2_ANAPACTRL_2G | BFL2_ANAPACTRL_5G)) {
				/* External PA is controlled using ANALOG PA ctrl lines.
				 * Enable PARLDO_pwrup (bit 12).
				 */
				si_pmu_regcontrol(pi->sh->sih, 0, 0x3000, 0x1000);

				/* set PAREF LDO voltage */
				/* addr = 0, shift = 4, mask = 0xf */
				si_corereg(pi->sh->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, regcontrol_addr), ~0, 0);
				si_corereg(pi->sh->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, regcontrol_data),
					0xf << 4, ((pi->ldo_voltage/5+9) & 0xf) << 4);
			}
			if ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
			     (BFL2_ANAPACTRL_2G | BFL2_ANAPACTRL_5G)) !=
			    (BFL2_ANAPACTRL_2G | BFL2_ANAPACTRL_5G)) {
				if (pi->sh->boardtype != 0x052A) {
					/* External PA is controlled using DIGITAL PA ctrl lines */
					si_corereg(pi->sh->sih, SI_CC_IDX,
					   OFFSETOF(chipcregs_t, chipcontrol), 0x44, 0x44);
				}
			}
		}
	} else {
		/* turn off PAREFLDO */
		if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
		    (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
		    (CHIPID(pi->sh->chip) == BCM43238_CHIP_ID)) {
				si_corereg(pi->sh->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, regcontrol_addr), ~0, 0);
				si_corereg(pi->sh->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, regcontrol_data),
					0x3 << 12, 0 << 12);
		}
	}
}

void
WLBANDINITFN(wlc_phy_init_nphy)(phy_info_t *pi)
{
	uint16 val;
	uint16 clip1_ths[NPHY_CORE_NUM];
	nphy_txgains_t target_gain;
	uint8 tx_pwr_ctrl_state;
	bool do_nphy_cal = FALSE;
	uint core;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	bool aci_rev7to15 = FALSE;
	uint32 *tx_pwrctrl_tbl = NULL;
	uint16 idx;
	int16 pga_gn = 0;
	int16 pad_gn = 0;
	int32 rfpwr_offset = 0;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	core = 0;

	pi->aci_rev7_subband_cust_fix = CHIPID_43236X_FAMILY(pi) &&
		((pi->fem2g->antswctrllut == 11) ||
		(pi->fem5g->antswctrllut == 11) ||
		(pi->fem2g->antswctrllut == 15) ||
		(pi->fem5g->antswctrllut == 15));

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(pi->interf->aci.nphy != NULL);

	/* init PUB_NOT_ASSOC */
	if (!(pi->measure_hold & PHY_HOLD_FOR_SCAN) &&
		!(pi->interf->aci.nphy->detection_in_progress)) {
#ifdef WLSRVSDB
		if (!pi->srvsdb_state->srvsdb_active)
			pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
#else
		pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
#endif // endif
	}

	/* Enable external LNA digital control lines */
	wlc_phy_enable_extlna_ctrl_nphy(pi);

	/* Enable external PA digital control lines */
	wlc_phy_enable_extpa_ctrl_nphy(pi);

	/* Enable Shared ant control lines */
	wlc_phy_enable_shared_ant_nphy(pi);

	/* Currently, all internal PA boards will use the internal envelope detectors for Tx IQ/LO
	 * cal. For MIMOPHY REVs >=7, the external PA board will also use the internal envelope
	 * detectors for Tx IQ/LO cal. For boards wth NREV >= 5 and ext FEM, you can force
	 * the use of the internal envelope detector for TX IQCAL by setting the boardflags.
	 */
	pi_nphy->nphy_use_int_tx_iqlo_cal = TRUE;

	/* By default, internal Tx IQ/LO cal will use the PAD tapoff point */
	pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa = FALSE;

	pi_nphy->nphy_deaf_count = 0;

	/* Step 0, initialize NPHY tables */
	wlc_phy_tbl_init_nphy(pi);

	pi_nphy->nphy_crsminpwr_adjusted = FALSE;
	pi_nphy->nphy_noisevars_adjusted = FALSE;

	/***********************************
	 * Rfctrl, Rfseq, and Afectrl setup
	 */

	/* Step 1, power up and reset 2055
	 * Step 2, clear force reset to 2055
	 * NOT DONE HERE -- already done in wlc_phy_radio_preinit_2055
	 */

	/* Step 3, write 0x0000 to rfctrloverride
	 * Now you are done with the rfctrl (for controlling the 2055) part.
	 */
	PHY_REG_LIST_START_WLBANDINITDATA
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlOverride0, 0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlOverride1, 0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride3, 0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride4, 0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride5, 0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride6, 0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlOverrideAux0, 0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlOverrideAux1, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Step 4, clear bit 8 in RfctrlIntc1 (0x91) to give the control to rfseq
	 * for the antenna for core1.
	 * Step 5, clear bit 8 in RfctrlIntc2 (0x92) to give the control to rfseq
	 * for the antenna for core2
	 */
	PHY_REG_LIST_START_WLBANDINITDATA
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlIntc1, 0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlIntc2, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Step 8, Write 0x0 to RfseqMode to turn off both CoreActv_override
	 * (to give control to Tx control word) and Trigger_override (to give
	 * control to rfseq)
	 *
	 * Now you are done with all rfseq INIT.
	 */
	phy_utils_and_phyreg(pi, NPHY_RfseqMode, ~3);

	/* Step 9, write 0x0 to AfectrlOverride (0xa5) to give control to Auto
	 * control mode.
	 */
	PHY_REG_LIST_START_WLBANDINITDATA
		PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride1, 0)
		PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride2, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* ---------------------------------- */

	/* gdk: not sure what these do but they are used in wmac */
	PHY_REG_LIST_START_WLBANDINITDATA
		PHY_REG_WRITE_ENTRY(NPHY, AfeseqTx2RxPwrUpDownDly20M, 32)
		PHY_REG_WRITE_ENTRY(NPHY, AfeseqTx2RxPwrUpDownDly40M, 32)
	PHY_REG_LIST_EXECUTE(pi);

	/* Delay mimophy start to 2.0 usec to match Skywork FEM board
	 * else Delay mimophy start to 2.3 usec to match BPHY
	 */
	if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_SKWRKFEM_BRD ||
	    ((pi->sh->boardvendor == VENDOR_APPLE) &&
	     (pi->sh->boardtype == 0x8b)))
		phy_utils_write_phyreg(pi, NPHY_TxRealFrameDelay, 160);
	else
		phy_utils_write_phyreg(pi, NPHY_TxRealFrameDelay, 184);

	PHY_REG_LIST_START_WLBANDINITDATA
		/* Turn on TxCRS extension. Need to eventually make the 1.0
		 * be native TxCRSOff (1.0us)
		 */
		PHY_REG_WRITE_ENTRY(NPHY, mimophycrsTxExtension, 200)
		/* Adjust for RX. This should have some better definition of
		 * native RxCRSoff (4.0us)
		 */
		PHY_REG_WRITE_ENTRY(NPHY, payloadcrsExtensionLen, 80)
		/* This number combined with MAC RIFS results in 2.0us RIFS air time */
		PHY_REG_WRITE_ENTRY(NPHY, TxRifsFrameDelay, 48)
	PHY_REG_LIST_EXECUTE(pi);

	if (NREV_LT(pi->pubpi->phy_rev, 8)) {
		wlc_phy_update_mimoconfig_nphy(pi, pi->n_preamble_override);
	}

	/* set tx/rx chain */
	wlc_phy_stf_chain_upd_nphy(pi);

	if (PHY_IPA(pi)) {
		FOREACH_CORE(pi, core) {
			/* initialize PAPD */
			PHY_REG_MOD_CORE(pi, NPHY, core, PapdEnable, compEnable, 1);
			PHY_REG_MOD_CORE(pi, NPHY, core, EpsilonTableAdjust, epsilonOffset,
			             pi_nphy->nphy_papd_epsilon_offset[core]);
		}

		/* change to spectral shaping tx digi filter for IntPA case */
		wlc_phy_ipa_set_tx_digi_filts_nphy(pi);
	} else {
		/* For ExtPA, use sharp tx digital filter for BPHY transmission same as
		 * in the IntPA case to improve spectral-mask margin as well as EVM
		 */
		wlc_phy_extpa_set_tx_digi_filts_nphy(pi);
	}

	wlc_phy_workarounds_nphy(pi);

	aci_rev7to15 = (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
		!(pi->aci_state & ACI_ACTIVE) &&
		!pi->aci_rev7_subband_cust_fix);

	if (aci_rev7to15) {
		wlc_phy_aci_noise_store_values_nphy(pi);
	}

	if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
		!pi->aci_rev7_subband_cust_fix &&
		(CHSPEC_IS2G(pi->radio_chanspec) &&
		(((pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
		pi->sh->interference_mode == WLAN_AUTO) &&
		(pi->aci_state & ACI_ACTIVE)) ||
		pi->sh->interference_mode == WLAN_MANUAL))) {

		/*
		   If home channel, make sure the function to change ACI registers
		   to on-values is not bypassed due to pi->interf->hw_aci_mitig_on
		   being TRUE
		*/
		if (CHSPEC_CHANNEL(pi->radio_chanspec) ==
			pi->interf->curr_home_channel) {
			pi->interf->hw_aci_mitig_on = FALSE;
		}
		if (CHSPEC_IS20(pi->radio_chanspec) ||
			(SCAN_RM_IN_PROGRESS(pi) && CHSPEC_IS40(pi->radio_chanspec)))
			wlc_phy_aci_home_channel_nphy(pi, pi->radio_chanspec);
	}

	/* Pulse reset_cca after initing all the tables */
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

	val = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_write_phyreg(pi, NPHY_BBConfig, val | BBCFG_RESETCCA);
	phy_utils_write_phyreg(pi, NPHY_BBConfig, val & (~BBCFG_RESETCCA));
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);

	wlapi_bmac_macphyclk_set(pi->sh->physhim, ON);

	/* trigger a rx2tx to get TX lpf bw and gain latched into 205x,
	 * trigger a reset2rx seq
	 */

	/* example calls to avoid compiler warnings, not used elsewhere yet: */
	wlc_phy_classifier_nphy(pi, 0, 0);
	wlc_phy_clip_det_nphy(pi, 0, clip1_ths);

	/* Initialize the bphy part */
	if (NREV_LE(pi->pubpi->phy_rev, LCNXN_BASEREV+1))
	{
		if (CHSPEC_IS2G(pi->radio_chanspec))
			wlc_phy_bphy_init_nphy(pi);
	}

	/* ensure power control is off before starting cals */
	tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
	wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

	/* initially force txgain for when txpwrctrl is disabled */
	wlc_phy_txpwr_fixpower_nphy(pi);

	wlc_phy_txpwrctrl_idle_tssi_nphy(pi);

	/* set up IQ & LO coeffs and set up pwr_ctrl params */
	wlc_phy_txpwrctrl_pwr_setup_nphy(pi);

	/* load tx gain tables (28:16 -- 2055 gain   13:8 -- 4321 DAC gain   7:0 -- BB mult)
	 *    entries in tx power table 0011xxxxx
	 *                              0100xxxxx
	 *    currently set 2055    gain to 0x0,
	 *              set DAC     gain to 0x2b (0dB)
	 *                            goes from 0xd -> 0x3f for -12 -> +8dB  in 0.4dB steps
	 *              set BB mult gain to 68 = 0x44 (to get rms of 128 at DAC)
	 */
	if (PHY_IPA(pi)) {
		tx_pwrctrl_tbl = wlc_phy_get_ipa_gaintbl_nphy(pi);
	} else {
		tx_pwrctrl_tbl = wlc_phy_get_epa_gaintbl_nphy(pi);
	}

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE1TXPWRCTL, 128, 192, 32, tx_pwrctrl_tbl);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE2TXPWRCTL, 128, 192, 32, tx_pwrctrl_tbl);

	/* Store the TxGm value that we are using for h/w powerctrl. This is required
	   during gainctrl in Rx IQ cal
	*/
	pi_nphy->nphy_gmval = (uint16) ((*tx_pwrctrl_tbl >> 16) & 0x7000);

	/* NPHY_IPA : initialize PAPD RF pwr offset table */
	if (PHY_IPA(pi)) {
		for (idx = 0; idx < 128; idx ++) {
			pga_gn = (tx_pwrctrl_tbl[idx] >> 24) & 0xf;
			pad_gn = (tx_pwrctrl_tbl[idx] >> 19) & 0x1f;

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
				           (RADIOREV(pi->pubpi->radiorev) == 4) ||
				           (RADIOREV(pi->pubpi->radiorev) == 6)) {
					rfpwr_offset = (int16)
					        nphy_papd_padgain_dlt_2g_2057rev3n4[pad_gn];
				} else if (RADIOREV(pi->pubpi->radiorev) == 5) {
					if (pi->pubpi->radiover == 1) {
					        rfpwr_offset = (int16)
					        nphy_papd_padgain_dlt_2g_2057rev5[pad_gn];
					} else {
						rfpwr_offset = (int16)
					        nphy_papd_padgain_dlt_2g_2057rev5v1[pad_gn];
					}
				} else if ((RADIOREV(pi->pubpi->radiorev) == 7) ||
				           (RADIOREV(pi->pubpi->radiorev) == 8)) {
					if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
					    (RADIOVER(pi->pubpi->radiover) == 2)) {
						rfpwr_offset = (int16)
					      nphy_papd_padgain_dlt_2g_2057rev7_ver2[pad_gn];
					} else {
						rfpwr_offset = (int16)
					        nphy_papd_padgain_dlt_2g_2057rev7[pad_gn];
					}
				} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
					/* BCM63268 */
					rfpwr_offset = (int16)
					        nphy_papd_padgain_dlt_2g_2057rev12[pad_gn];
				} else if (RADIOREV(pi->pubpi->radiorev) == 13) {
					/* 53572A0 */
					rfpwr_offset = (int16)
					        nphy_papd_padgain_dlt_2g_2057rev13[pad_gn];
				} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
					/* BCM43217 */
					rfpwr_offset = (int16)
					        nphy_papd_padgain_dlt_2g_2057rev14[pad_gn];
				} else {
					PHY_ERROR(("Unsupported 2057 radio rev %d\n",
						RADIOREV(pi->pubpi->radiorev)));
					ASSERT(0);
				}

			} else {
				if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
				           (RADIOREV(pi->pubpi->radiorev) == 4) ||
				           (RADIOREV(pi->pubpi->radiorev) == 6)) {
					rfpwr_offset = (int16)
					      nphy_papd_pgagain_dlt_5g_2057[pga_gn];
				} else if ((RADIOREV(pi->pubpi->radiorev) == 7) ||
				           (RADIOREV(pi->pubpi->radiorev) == 8) ||
				           (RADIOREV(pi->pubpi->radiorev) == 12)) {
					rfpwr_offset = (int16)
					      nphy_papd_pgagain_dlt_5g_2057rev7[pga_gn];
				} else {
					PHY_ERROR(("Unsupported 2057 radio rev %d\n",
						RADIOREV(pi->pubpi->radiorev)));
					ASSERT(0);
				}
			}
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE1TXPWRCTL, 1,
			                         576 + idx, 32, &rfpwr_offset);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE2TXPWRCTL, 1,
			                         576 + idx, 32, &rfpwr_offset);
			PHY_CAL(("rfpwr offset %d: pga_gain 0x%x, pad_gain 0x%x, "
			         "offset %d\n", idx, pga_gn, pad_gn, rfpwr_offset));
		}
	}

	/* If any rx cores were disabled before nphy_init,
	   disable them again since nphy init enables all
	   rx cores
	*/
	if (stf_shdata->phyrxchain != 0x3) {
		wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, stf_shdata->phyrxchain, 0);
	}

	/* reset mphase calibration */
	if (PHY_PERICAL_MPHASE_PENDING(pi)) {
#ifdef PHYCAL_CACHING
		/* Switched the context so restart a pending MPHASE cal, else clear the state */
		if (ctx) {
			PHY_CAL(("%s: Restarting calibration for 0x%x phase %d\n",
			         __FUNCTION__, pi->radio_chanspec, pi->cal_info->cal_phase_id));
			/* Delete any exisiting timer just in case */
			wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
			wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
		} else
#endif // endif
			phy_calmgr_mphase_restart(pi->calmgri);
	}

	if (!NORADIO_ENAB(pi->pubpi)) {
		bool do_rssi_cal = FALSE;

		do_rssi_cal = (CHSPEC_IS2G(pi->radio_chanspec)) ?
				(pi_nphy->nphy_rssical_chanspec_2G == 0) :
				(pi_nphy->nphy_rssical_chanspec_5G == 0);

		if (do_rssi_cal) {
			wlc_phy_rssi_cal_nphy(pi);
		} else {
			wlc_phy_restore_rssical_nphy(pi);
		}

		if (!SCAN_RM_IN_PROGRESS(pi)) {
			do_nphy_cal = (CHSPEC_IS2G(pi->radio_chanspec)) ?
				(pi_nphy->nphy_iqcal_chanspec_2G == 0) :
				(pi_nphy->nphy_iqcal_chanspec_5G == 0);
		}

		if (!pi_nphy->do_initcal)
			do_nphy_cal = FALSE;

#if defined(PHYCAL_CACHING)
		if (ctx) {
			PHY_CAL(("wl%d: %s: Not doing a cal because cal caching is enabled\n",
				pi->sh->unit, __FUNCTION__));
			do_nphy_cal = FALSE;
		}
#endif /* PHYCAL_CACHING */

		if (do_nphy_cal) {
			/* read current tx gain and use as target_gain */
			wlc_phy_get_tx_gain_nphy(pi, &target_gain);

			PHY_CAL(("wlc_nphy_init: full cal on chanspec 0x%x\n",
				pi->radio_chanspec));

			if (pi->phy_cal_mode != PHY_PERICAL_MPHASE) {
				wlc_phy_rssi_cal_nphy(pi);

				/* For 4322, set the Tx power to approx 10dBm before
				   doing Tx/Rx cal
				*/
				pi_nphy->nphy_cal_orig_pwr_idx[0] =
				    wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_0);
				pi_nphy->nphy_cal_orig_pwr_idx[1] =
				    wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_1);

				wlc_phy_precal_txgain_nphy(pi);
				target_gain = pi_nphy->nphy_cal_target_gain;

				if (wlc_phy_cal_txiqlo_nphy(pi, target_gain, TRUE, FALSE) ==
				    BCME_OK) {
#ifdef RXIQCAL_FW_WAR
					if ((CHSPEC_IS5G(pi->radio_chanspec) &&
						(wlc_phy_cal_rxiq_nphy_fw_war(pi,
						target_gain, 2, FALSE, 0x3) == BCME_OK)) ||
					    (CHSPEC_IS2G(pi->radio_chanspec) &&
						(wlc_phy_cal_rxiq_nphy(pi, target_gain, 2,
						FALSE, 0x3) == BCME_OK)))
#else
					if (wlc_phy_cal_rxiq_nphy(pi, target_gain, 2,
						FALSE, 0x3) == BCME_OK)
#endif // endif
					{
						wlc_phy_savecal_nphy(pi);
						/* txpwrctrl will be enabled out of this loop, then
						 * wlc_phy_txpwrctrl_coeff_setup_nphy will be called
						 */
					}
				} else
					PHY_ERROR(("wlc_nphy_init: !!! initial cal failed\n"));
			} else if (pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_IDLE) {
				PHY_INFORM(("%s scheduling a multiphase calibration\n",
					__FUNCTION__));
				wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_PERICAL_PHYINIT);
			}
		} else {
#ifdef PHYCAL_CACHING
				if (!ctx)
#endif // endif
					wlc_phy_restorecal_nphy(pi);
		}
	}

	/* configure and enable HW TX power control from soft state in
	 * case we got an init via scan or big hammer.
	 */
	wlc_phy_txpwrctrl_coeff_setup_nphy(pi);

	wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);

#if defined(WLTEST)
	/* reapply txpwrindex soft state in case it was being forced
	 * and we got an init via scan or big hammer.
	 */
	wlc_phy_txpwr_index_nphy(pi, (1 << 0), pi_nphy->nphy_txpwrindex[PHY_CORE_0].index, TRUE);
	wlc_phy_txpwr_index_nphy(pi, (1 << 1), pi_nphy->nphy_txpwrindex[PHY_CORE_1].index, TRUE);
#endif // endif

	wlc_phy_nphy_tkip_rifs_war(pi, pi->sh->_rifs_phy);

	/* Spur avoidance WAR for 4322 */
	wlc_phy_spurwar_nphy(pi);

	if (pi->phy_init_por) {
		wlc_phy_aci_noise_store_values_nphy(pi);

#ifndef WLC_DISABLE_ACI
		if (CHSPEC_IS2G(pi->radio_chanspec) &&
			(!(NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV)) ||
			!(pi->interf->aci.nphy->detection_in_progress))) {
			wlc_phy_acimode_reset_nphy(pi);
		}
		wlc_phy_noisemode_reset_nphy(pi);
#endif // endif
		/* curr_home_channel may have changed from attached value */
		pi->interf->curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	}

#ifndef WLC_DISABLE_ACI
	if (!(SCAN_RM_IN_PROGRESS(pi)) &&
	(CHSPEC_CHANNEL(pi->radio_chanspec) == pi->interf->curr_home_channel)) {
#ifdef WLSRVSDB
		uint8 i = 0;
		for (i = 0; i < 2; i++) {
			if (pi->srvsdb_state->srvsdb_active &&
				(!pi->srvsdb_state->swbkp_snapshot_valid[i] ||
				!pi->srvsdb_state->acimode_noisemode_reset_done[i]) &&
				(CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec) ==
				pi->srvsdb_state->sr_vsdb_channels[i])) {
				pi->do_noisemode_reset = TRUE;
				if (!pi->srvsdb_state->acimode_noisemode_reset_done[i]) {
					pi->srvsdb_state->acimode_noisemode_reset_done[i] = TRUE;
				}
				break;
			}
		}
#endif // endif
		/* resets noisemode state if chspec changes via IOCTL call */

		if (pi->do_noisemode_reset) {
			wlc_phy_noisemode_reset_nphy(pi);
			PHY_ACI(("\n\n\n\n CurCh %d : noise state reset! \n\n\n\n",
				CHSPEC_CHANNEL(pi->radio_chanspec)));

			pi->do_noisemode_reset = FALSE;
		}
	}
#endif /* !defined(WLC_DISABLE_ACI) */

	phy_n_rssi_init_gain_err(pi->u.pi_nphy->rssii);

	phy_utils_mod_phyreg(pi, NPHY_mluA, NPHY_mluA_mluA1_MASK, 4 << NPHY_mluA_mluA1_SHIFT);
	phy_utils_mod_phyreg(pi, NPHY_mluA, NPHY_mluA_mluA2_MASK, 4 << NPHY_mluA_mluA2_SHIFT);
#if defined(RXDESENS_EN)
	/* apply desens back after scans */
	if ((pi_nphy->ntd_save_current_rxdesens != 0) &&
		(pi_nphy->ntd_save_current_rxdesens_channel ==
		CHSPEC_CHANNEL(pi->radio_chanspec))) {
		wlc_nphy_set_rxdesens((wlc_phy_t *)pi, pi_nphy->ntd_save_current_rxdesens);
	}
#endif // endif
}

/**
 * preamble =   WLC_N_PREAMBLE_MIXEDMODE: receive MM frame only
 * 		WLC_N_PREAMBLE_GF	: receive GF frame only
 *		other			: can receive either MM or GF
 */
static void
wlc_phy_update_mimoconfig_nphy(phy_info_t *pi, int32 preamble)
{
	bool gf_preamble = FALSE;
	uint16 val;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (preamble == WLC_N_PREAMBLE_GF) {
		gf_preamble = TRUE;
	}

	/* if autodetect is TRUE, it will override gf_only
	 * If autodetect is FALSE, gf_only determines receiver is gf or mm capable
	 * WARNING: there is slightly performance degradation to enable auto_detect
	 */
	val = phy_utils_read_phyreg(pi, NPHY_MimoConfig);

	val |= RX_GF_MM_AUTO;
	val &= ~RX_GF_OR_MM;
	if (gf_preamble)
		val |= RX_GF_OR_MM;

	phy_utils_write_phyreg(pi, NPHY_MimoConfig, val);
}

void
wlc_phy_resetcca_nphy(phy_info_t *pi)
{
	uint16 val;

	/* MAC should be suspended before calling this function */
	ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));

	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

	val = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_write_phyreg(pi, NPHY_BBConfig, val | BBCFG_RESETCCA);
	OSL_DELAY(1);
	phy_utils_write_phyreg(pi, NPHY_BBConfig, val & (~BBCFG_RESETCCA));

	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);

	wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);
}

void
wlc_phy_pa_override_nphy(phy_info_t *pi, bool en)
{
	uint16 rfctrlintc_override_val;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (!en) {
		/* Switch Off both PA's (set all RfctrlIntc flags to 0) */
		pi_nphy->rfctrlIntc1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc1);
		pi_nphy->rfctrlIntc2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc2);

		rfctrlintc_override_val = 0x1480;

		phy_utils_write_phyreg(pi, NPHY_RfctrlIntc1, rfctrlintc_override_val);
		phy_utils_write_phyreg(pi, NPHY_RfctrlIntc2, rfctrlintc_override_val);
	} else {
		/* Restore Rfctrl override settings */
		phy_utils_write_phyreg(pi, NPHY_RfctrlIntc1, pi_nphy->rfctrlIntc1_save);
		phy_utils_write_phyreg(pi, NPHY_RfctrlIntc2, pi_nphy->rfctrlIntc2_save);
	}

}

void
wlc_phy_stf_chain_upd_nphy(phy_info_t *pi)
{
	/* for REV7, RfseqCoreActv changed to RfseqCoreActv2057,
	 * but address and fields haven't, so keep same code
	 */
	uint16 txrx_chain = (NPHY_RfseqCoreActv_TxRxChain0 | NPHY_RfseqCoreActv_TxRxChain1);
	bool CoreActv_override = FALSE;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (pi->nphy_txrx_chain == WLC_N_TXRX_CHAIN0) {
		txrx_chain = NPHY_RfseqCoreActv_TxRxChain0;
		CoreActv_override = TRUE;
	} else if (pi->nphy_txrx_chain == WLC_N_TXRX_CHAIN1) {
		txrx_chain = NPHY_RfseqCoreActv_TxRxChain1;
		CoreActv_override = TRUE;
	}

	phy_utils_mod_phyreg(pi, NPHY_RfseqCoreActv,
		(NPHY_RfseqCoreActv_EnTx_MASK | NPHY_RfseqCoreActv_EnRx_MASK),
		txrx_chain);

	/* set/clear mimophyreg (RfseqMode.CoreActv_override) */
	if (CoreActv_override) {
		/* Disable Periodic calibrations when TXRX Chain is overriden to avoid
		 * disturbing the phy
		 */
		pi->phy_cal_mode = PHY_PERICAL_DISABLE;
		phy_utils_or_phyreg(pi, NPHY_RfseqMode, NPHY_RfseqMode_CoreActv_override);
	} else {
		pi->phy_cal_mode = PHY_PERICAL_MPHASE;
		phy_utils_and_phyreg(pi, NPHY_RfseqMode, ~NPHY_RfseqMode_CoreActv_override);
	}
}

void
wlc_phy_rxcore_setstate_nphy(wlc_phy_t *pih, uint8 rxcore_bitmask, bool enable_phyhangwar)
{
	uint16 regval;
	phy_info_t *pi = (phy_info_t*)pih;
	bool suspend;
	uint16 rfseqCoreActv_DisRx_save, rfseqCoreActv_EnTx_save;
	uint16 rfseqMode_save, sampleDepthCount_save, sampleLoopCount_save;
	uint16 sampleInitWaitCount_save, sampleCmd_save;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	phy_stf_set_phyrxchain(pi->stfi, rxcore_bitmask);

	if (!pi->sh->clk)
		return;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* be Deaf */
	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	else if (enable_phyhangwar == 1)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Save Registers */
	regval =  phy_utils_read_phyreg(pi, NPHY_RfseqCoreActv);
	rfseqCoreActv_DisRx_save = ((regval & NPHY_RfseqCoreActv_DisRx_MASK) >>
	                           NPHY_RfseqCoreActv_DisRx_SHIFT);
	rfseqCoreActv_EnTx_save = ((regval & NPHY_RfseqCoreActv_EnTx_MASK) >>
	                          NPHY_RfseqCoreActv_EnTx_SHIFT);
	rfseqMode_save = phy_utils_read_phyreg(pi, NPHY_RfseqMode);
	sampleDepthCount_save = phy_utils_read_phyreg(pi, NPHY_sampleDepthCount);
	sampleLoopCount_save = phy_utils_read_phyreg(pi, NPHY_sampleLoopCount);
	sampleInitWaitCount_save = phy_utils_read_phyreg(pi, NPHY_sampleInitWaitCount);
	sampleCmd_save = phy_utils_read_phyreg(pi, NPHY_sampleCmd);

	/* Indicate to PHY of the Inactive Core */
	phy_utils_mod_phyreg(pi, NPHY_CoreConfig,
	            NPHY_CoreConfig_CoreMask_MASK,
	            ((rxcore_bitmask & 0x3) << NPHY_CoreConfig_CoreMask_SHIFT));

	/* Indicate to RFSeq of the Inactive Core */
	phy_utils_mod_phyreg(pi, NPHY_RfseqCoreActv,
	            NPHY_RfseqCoreActv_EnRx_MASK,
	            ((rxcore_bitmask & 0x3) << NPHY_RfseqCoreActv_EnRx_SHIFT));

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	PHY_REG_LIST_START
		/* Rx Chain gets shut off in Rx2Tx Sequence */
		PHY_REG_MOD_ENTRY(NPHY, RfseqCoreActv, DisRx, 3)
		/* Make sure Tx Chain doesn't get turned off during this function */
		PHY_REG_MOD_ENTRY(NPHY, RfseqCoreActv, EnTx, 0)
		PHY_REG_MOD_ENTRY(NPHY, RfseqMode, CoreActv_override, 1)

		/* To Turn off the AFE:
		 * TxFrame needs to toggle on and off.
		 * Accomplished by A) turning sample play ON and OFF
		 * Rx2Tx and Tx2Rx Rfseq is executed during A)
		 * To Turn Off the Radio (except LPF) - Do a Rx2Tx
		 * To Turn off the LPF - Do a Tx2Rx
		 */
		PHY_REG_WRITE_ENTRY(NPHY, sampleDepthCount, 0)
		PHY_REG_WRITE_ENTRY(NPHY, sampleLoopCount, 0xffff)
		PHY_REG_WRITE_ENTRY(NPHY, sampleInitWaitCount, 0)
		PHY_REG_MOD_ENTRY(NPHY, sampleCmd, DisTxFrameInSampleplay, 0)
		PHY_REG_MOD_ENTRY(NPHY, sampleCmd, start, 1)
	PHY_REG_LIST_EXECUTE(pi);

	/*  Allow Time For Rfseq to start */
	OSL_DELAY(1);
	/* Allow Time For Rfseq to stop - 1ms timeout */
	SPINWAIT((phy_utils_read_phyreg(pi, NPHY_RfseqStatus)),
		NPHY_SPINWAIT_RXCORE_SETSTATE_RFSEQ_STATUS);
	ASSERT(!(phy_utils_read_phyreg(pi, NPHY_RfseqStatus)));

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	phy_utils_mod_phyreg(pi, NPHY_sampleCmd,
	            NPHY_sampleCmd_stop_MASK,
	            (1 << NPHY_sampleCmd_stop_SHIFT));

	/* Restore Register */
	phy_utils_mod_phyreg(pi, NPHY_RfseqCoreActv,
	            NPHY_RfseqCoreActv_DisRx_MASK,
	            (rfseqCoreActv_DisRx_save <<
	             NPHY_RfseqCoreActv_DisRx_SHIFT));
	phy_utils_mod_phyreg(pi, NPHY_RfseqCoreActv,
	            NPHY_RfseqCoreActv_EnTx_MASK,
	            (rfseqCoreActv_EnTx_save << NPHY_RfseqCoreActv_EnTx_SHIFT));
	phy_utils_write_phyreg(pi, NPHY_RfseqMode, rfseqMode_save);
	phy_utils_write_phyreg(pi, NPHY_sampleDepthCount, sampleDepthCount_save);
	phy_utils_write_phyreg(pi, NPHY_sampleLoopCount, sampleLoopCount_save);
	phy_utils_write_phyreg(pi, NPHY_sampleInitWaitCount, sampleInitWaitCount_save);
	phy_utils_write_phyreg(pi, NPHY_sampleCmd, sampleCmd_save);
	if ((rxcore_bitmask & 0x3) == 1) {
		phy_utils_or_radioreg(pi, RADIO_2057_RXTXBIAS_CONFIG_CORE1, 0x20);
		phy_utils_or_radioreg(pi, RADIO_2057v7_OVR_REG18, 0x80);
		phy_utils_and_radioreg(pi, RADIO_2057v7_OVR_REG7, 0xfb);
	} else if ((rxcore_bitmask & 0x3) == 2) {
		phy_utils_or_radioreg(pi, RADIO_2057_RXTXBIAS_CONFIG_CORE0, 0x20);
		phy_utils_or_radioreg(pi, RADIO_2057v7_OVR_REG7, 0x4);
		phy_utils_and_radioreg(pi, RADIO_2057v7_OVR_REG18, 0x7f);
	} else if ((rxcore_bitmask & 0x3) == 3) {
		phy_utils_and_radioreg(pi, RADIO_2057v7_OVR_REG18, 0x7f);
		phy_utils_and_radioreg(pi, RADIO_2057v7_OVR_REG7, 0xfb);
	}

	/*  Return from deaf */
	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	else if (enable_phyhangwar == 1)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

uint8
wlc_phy_rxcore_getstate_nphy(wlc_phy_t *pih)
{
	uint16 regval, rxen_bits;
	phy_info_t *pi = (phy_info_t*)pih;

	/* Extract the EnRx field of this register */
	regval = phy_utils_read_phyreg(pi, NPHY_RfseqCoreActv);
	rxen_bits = (regval >> NPHY_RfseqCoreActv_EnRx_SHIFT) & 0xf;

	return ((uint8) rxen_bits);
}

static void
wlc_phy_txpwr_limit_to_tbl_nphy(phy_info_t *pi)
{
	uint8 idx, idx2, i, delta_ind;

	ppr_dsss_rateset_t dsss_array;
	ppr_ofdm_rateset_t ofdm_array;
	ppr_ht_mcs_rateset_t mcs_array;
	int8 *pwr_array = ofdm_array.pwr;
	uint8 max_modes = 1;

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi->tx_power_offset == NULL)
		return;

	/* The assignment to the adj_pwr_tbl_nphy could be done
	 * more as a table driven operation instead of single lines.
	 */

	/* Power offsets in 0.25 dB stepsize for CCK rates */
	/* Bphy rates are seeing an offset compared to ofdm rates on 43227 boards */
	/* introducing a offset to cck rate ofset table */
	/* whose value is inited in nphy_attach */
	ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20,
		WL_TX_CHAINS_1, &dsss_array);

	for (idx = 0; idx < WL_RATESET_SZ_DSSS; idx++) {
		pi_nphy->adj_pwr_tbl_nphy[idx] = dsss_array.pwr[idx];
	}

	/* For each constellation and STF mode, there are 4 power offset entries
	 * corresponding to code rates 1/2, 2/3, 3/4, and 5/6
	 */

	/* if core number is 1, only calculate pwr for SISO mode */
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
		max_modes = 4;
	}
	/* Power offsets in 0.25 dB stepsize for OFDM SISO, CDD, STBC, and SDM rates */
	for (i = 0; i < max_modes; i++) {
		idx = 0;
		idx2 = 0;
		/* Leg OFDM powers have information about BPSK coding rate 3/4 (9 Mbps),
		 * which is not available in 11n MCS rates. Similarly, MCS7 is 64 QAM coding
		 * rate 5/6 rate, which is not present in Legacy OFDM. Use delta_ind to
		 * accordingly populate the AdjPwrLUT depending on whether the power offsets
		 * are provided by Legacy OFDM rates or 11n MCS rates.
		 */
		delta_ind = 0;

		switch (i) {
		case 0:
			/* SISO */
			ppr_get_ofdm(pi->tx_power_offset, WL_TX_BW_20,
				WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_array);
			delta_ind = 1;
			pwr_array = ofdm_array.pwr;
			break;
		case 1:
			/* CDD */
			ppr_get_ofdm(pi->tx_power_offset, WL_TX_BW_20,
				WL_TX_MODE_CDD, WL_TX_CHAINS_2, &ofdm_array);
			pwr_array = ofdm_array.pwr;
			break;
		case 2:
			/* STBC */
			ppr_get_ht_mcs(pi->tx_power_offset, WL_TX_BW_20,
				WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_2, &mcs_array);
			pwr_array = mcs_array.pwr;
			break;
		case 3:
			/* SDM */
			ppr_get_ht_mcs(pi->tx_power_offset, WL_TX_BW_20,
				WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2, &mcs_array);
			pwr_array = mcs_array.pwr;
			break;
		}

		/* BPSK */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 1/2 */
		idx = idx + delta_ind;
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 2/3 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 3/4 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx++]; /* rate5/6 */

		/* QPSK */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx++]; /* rate1/2 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 2/3 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 3/4 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx++]; /* rate5/6 */

		/* 16 QAM */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx++]; /* rate1/2 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 2/3 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 3/4 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx++]; /* rate5/6 */

		/* 64 QAM */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 1/2 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx++]; /* rate2/3 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 3/4 */
		idx = idx + 1 - delta_ind;
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 5/6 */

		/* 256 QAM rates use same power as 64 QAM code rate 5/6 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 1/2 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 2/3 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2++) + i] = pwr_array[idx];  /* rate 3/4 */
		pi_nphy->adj_pwr_tbl_nphy[4 + 4*(idx2) + i] = pwr_array[idx];  /* rate 5/6 */
	}
}

static void wlc_phy_elna_gainctrl_workaround(phy_info_t *pi)
{

	uint8 val;
	int8 elna_gain_db[2];

	val = (CHSPEC_IS2G(pi->radio_chanspec) ?
		pi->u.pi_nphy->elna2g :
		pi->u.pi_nphy->elna5g) * 3 + 9;
	elna_gain_db[0] = val;
	elna_gain_db[1] = val;

	/* update eLNA gain in Gain[12] table */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elna_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elna_gain_db);
}

uint16 wlc_phy_gain_from_code(phy_info_t *pi, uint8 type, uint8 core_num)
{
	uint16 regA = 0x0000, regB = 0x0000;
	uint16 elnaGain, lna1Gain, lna2Gain, mixGain, vgaGain;
	uint16 biq0Gain, biq1Gain;
	uint16 tbl_elna[2], tbl_lna1[6], tbl_lna2[7], tbl_mixtia[10], tbl_vga[11];
	uint16 tbl_biq0[7], tbl_biq1[10];
	uint16 totalGain;
	uint16 tblgain_addr;

	switch (type) {
	case NPHY_gainType_HI :
		if (core_num == 0) {
			regA = phy_utils_read_phyreg(pi, NPHY_Core1clipHiGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core1clipHiGainCodeB2057);
		} else {
			regA = phy_utils_read_phyreg(pi, NPHY_Core2clipHiGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core2clipHiGainCodeB2057);
		}
		break;
	case NPHY_gainType_MD :
		if (core_num == 0) {
			regA = phy_utils_read_phyreg(pi, NPHY_Core1clipmdGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core1clipmdGainCodeB2057);
		} else {
			regA = phy_utils_read_phyreg(pi, NPHY_Core2clipmdGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core2clipmdGainCodeB2057);
		}
		break;
	case NPHY_gainType_LO :
		if (core_num == 0) {
			regA = phy_utils_read_phyreg(pi, NPHY_Core1cliploGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core1cliploGainCodeB2057);
		} else {
			regA = phy_utils_read_phyreg(pi, NPHY_Core2cliploGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core2cliploGainCodeB2057);
		}
		break;
	case NPHY_gainType_INIT :
		if (core_num == 0) {
			regA = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
		} else {
			regA = phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeA2057);
			regB = phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057);
		}
		break;
	}
		/* extract the gain idx */
	elnaGain = regA & 0x1;
	lna1Gain = (regA & 0xe) >> 1;
	lna2Gain = (regA & 0x70) >> 4;
	mixGain = (regA & 0x780) >> 7;
	vgaGain = (regA & 0xf000) >> 12;

	biq0Gain = (regB & 0x00f0) >> 4;
	biq1Gain = (regB & 0x0f00) >> 8;

	tblgain_addr = (core_num == 0) ? NPHY_TBL_ID_GAIN1 :
		NPHY_TBL_ID_GAIN2;
	wlc_phy_table_read_nphy(pi, tblgain_addr, ARRAYSIZE(tbl_elna), 0, 16,
		tbl_elna);
	wlc_phy_table_read_nphy(pi, tblgain_addr, ARRAYSIZE(tbl_lna1), 8, 16,
		tbl_lna1);
	wlc_phy_table_read_nphy(pi, tblgain_addr, ARRAYSIZE(tbl_lna2), 16, 16,
		tbl_lna2);
	wlc_phy_table_read_nphy(pi, tblgain_addr, ARRAYSIZE(tbl_mixtia), 32, 16,
		tbl_mixtia);
	wlc_phy_table_read_nphy(pi, tblgain_addr, ARRAYSIZE(tbl_vga), 80, 16,
		tbl_vga);
	wlc_phy_table_read_nphy(pi, tblgain_addr, ARRAYSIZE(tbl_biq0), 96, 16,
		tbl_biq0);
	wlc_phy_table_read_nphy(pi, tblgain_addr, ARRAYSIZE(tbl_biq1), 112, 16,
		tbl_biq1);

	totalGain = tbl_elna[elnaGain]+ tbl_lna1[lna1Gain]+ tbl_lna2[lna2Gain]+ tbl_mixtia[mixGain]+
		tbl_vga[vgaGain]+ tbl_biq0[biq0Gain]+ tbl_biq1[biq1Gain];

	return (totalGain);
}

static void wlc_phy_backoff_initgain_elna(phy_info_t *pi)
{
	int16 delta_backoff, init_backoff;
	uint16 currgain_init1, currgain_init2, curgain_rfseq1,  curgain_rfseq2, newgainarray[2];

	phy_info_nphy_t *pi_nphy = NULL;
	bool suspend;

	int16 new_biq1_gain1, new_biq1_gain2;
	uint16 biq1_gain1, biq1_gain2;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	pi_nphy = pi->u.pi_nphy;
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		init_backoff = pi_nphy->elna2g;
	} else {
		init_backoff = pi_nphy->elna5g;
	}

	biq1_gain1 = (uint16) ((phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057))
		>>NPHY_Core1InitGainCodeB2057_initbiq1gainIndex_SHIFT);
	biq1_gain2 = (uint16) ((phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057))
		>>NPHY_Core1InitGainCodeB2057_initbiq1gainIndex_SHIFT);

	/* linear mapping from elna2g/5g value to backoff of biq1 */
	/* elna2g/5g = 0 denotes 9 dB backoff, and so biq1 needs to reduce by 3 ticks */
	/* elna2g/5g = 1 denotes 12 dB backoff, and so biq1 needs to reduce by 4 ticks */
	delta_backoff = init_backoff + 3;

	new_biq1_gain1 = (int16) biq1_gain1 - delta_backoff;
	new_biq1_gain2 = (int16) biq1_gain2 - delta_backoff;

	if (new_biq1_gain1 < 0)
		new_biq1_gain1 = 0;
	if (new_biq1_gain2 < 0)
		new_biq1_gain2 = 0;

	currgain_init1 = (uint16) phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
	currgain_init2 = (uint16) phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057);
	phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057,
		(((uint16) new_biq1_gain1) <<8) | (currgain_init1 & 0xff));
	phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057,
		(((uint16) new_biq1_gain2) <<8) | (currgain_init2 & 0xff));

	wlc_phy_table_read_nphy(pi, 7, 1, 0x106, 16, &curgain_rfseq1);
	wlc_phy_table_read_nphy(pi, 7, 1, 0x107, 16, &curgain_rfseq2);
	newgainarray[0] = (((uint16) new_biq1_gain1)<<12) | (curgain_rfseq1 & 0xfff);
	newgainarray[1] = (((uint16) new_biq1_gain2)<<12) | (curgain_rfseq2 & 0xfff);

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, newgainarray);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_workarounds_nphy(phy_info_t *pi)
{
	uint8 rfseq_tx2rx_events_rev3[] = {
		NPHY_REV3_RFSEQ_CMD_EXT_PA,
		NPHY_REV3_RFSEQ_CMD_INT_PA_PU,
		NPHY_REV3_RFSEQ_CMD_RXPD_TXPD,
		NPHY_REV3_RFSEQ_CMD_TR_SWITCH,
		NPHY_REV3_RFSEQ_CMD_RXG_FBW,
		NPHY_REV3_RFSEQ_CMD_CLR_HIQ_DIS,
		NPHY_REV3_RFSEQ_CMD_END
	};
	uint8 rfseq_tx2rx_dlys_rev3[] = {8, 4, 4, 4, 4, 6, 1};
	uint8 rfseq_tx2rx_dlys_ext_pa[] = {70, 4, 4, 4, 4, 6, 1};

	bool ext_pa_ana_2g = ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_ANAPACTRL_2G) != 0);
	bool ext_pa_ana_5g = ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_ANAPACTRL_5G) != 0);

	uint32 leg_data_weights;
	uint8 chan_freq_range = 0;
	const uint16 dac_control = 0x0002;
	uint16 aux_adc_vmid_rev7_core0[] = {0x8e, 0x96, 0x96, 0x96};
	uint16 aux_adc_vmid_rev7_core1[] = {0x8f, 0x9f, 0x9f, 0x96};
	uint16 aux_adc_gain_rev7_core0[] = {0x02, 0x02, 0x02, 0x02};
	uint16 aux_adc_gain_rev7_core1[] = {0x02, 0x02, 0x02, 0x02};
	int32  min_nvar_val = 0x18d;
	const int32  min_nvar_offset_6mbps = 20;
	int32  min_nvar_offset_mcs0to3[] = {0x14, 0x14, 0x14, 0x14};
	uint32 base_nvar_buf[2];
	uint8 pdetrange;
	const uint16 afectrl_adc_ctrl0_bw20_rev7 = 0xA840;
	const uint16 afectrl_adc_ctrl1_bw20_rev7 = 0x8;
	const uint16 afectrl_adc_ctrl0_bw40_rev7 = 0xFC60;
	const uint16 afectrl_adc_ctrl1_bw40_rev7 = 0xC;
	const uint16 afectrl_adc_ctrl1_rev7 = 0x20;
	const uint16 afectrl_adc_ctrl2_rev7 = 0x0;
	const uint16 rfseq_rx2tx_lpf_h_hpc_rev7 = 0x77;
	const uint16 rfseq_tx2rx_lpf_h_hpc_rev7 = 0x77;
	const uint16 rfseq_pktgn_lpf_h_hpc_rev7 = 0x77;
	const uint16 rfseq_htpktgn_lpf_hpc_rev7[] = {0x77, 0x11, 0x11};
	const uint16 rfseq_pktgn_lpf_hpc_rev7[] = {0x11, 0x11};
	const uint16 rfseq_cckpktgn_lpf_hpc_rev7[] = {0x11, 0x11};
	uint16 ipalvlshift_3p3_war_en = 0;
	uint8 local_rc_cal_en;
	uint16 rccal_bcap_val, rccal_scap_val;
	uint16 rccal_tx20_11b_bcap[] = {0, 0};
	uint16 rccal_tx20_11b_scap[] = {0, 0};
	uint16 rccal_tx20_11n_bcap[] = {0, 0};
	uint16 rccal_tx20_11n_scap[] = {0, 0};
	uint16 rccal_tx40_11n_bcap[] = {0, 0};
	uint16 rccal_tx40_11n_scap[] = {0, 0};
	uint16 rx2tx_lpf_rc_lut_tx20_11b = 0;
	uint16 rx2tx_lpf_rc_lut_tx20_11n = 0;
	uint16 rx2tx_lpf_rc_lut_tx40_11n = 0;
	uint16 tx_lpf_bw_ofdm_20mhz[NPHY_CORE_NUM];
	uint16 tx_lpf_bw_ofdm_40mhz[NPHY_CORE_NUM];
	uint16 tx_lpf_bw_11b[NPHY_CORE_NUM];
	uint16 ipa2g_mainbias, ipa2g_casconv, ipa2g_biasfilt;
	uint16 txgm_idac_bleed = 0;
	bool   rccal_ovrd = FALSE;
	int coreNum;
	uint16 curr_channel = 0;
	phy_info_nphy_t *pi_nphy = NULL;

	int freq = 0;
	uint channel = 0;
	uint16 rfseq_lpf_ctl_lut_rev7[] = {0x0, 0x10f, 0x10f};

	chan_info_nphy_radio2057_t		*t0 = NULL;
	chan_info_nphy_radio205x_t		*t1 = NULL;
	chan_info_nphy_radio2057_rev5_t *t2 = NULL;

	/* check phy_bw */
	uint8 is_phybw40;

	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);

	uint core;
	uint16 tone_jammer_war_blk;

	is_phybw40 = CHSPEC_IS40(pi->radio_chanspec);

	pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* bphy classifier on for 2.4G vs off for 5G */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_cck_en, 0);
	} else {
		wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_cck_en, 1);
	}
	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* REVs 0-7 */
	if (!ISSIM_ENAB(pi->sh->sih)) {
		phy_utils_or_phyreg(pi, NPHY_IQFlip, NPHY_IQFlip_ADC1 | NPHY_IQFlip_ADC2);
	}

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, PhaseTrackAlpha0, 293)
		PHY_REG_WRITE_ENTRY(NPHY, PhaseTrackAlpha1, 435)
		PHY_REG_WRITE_ENTRY(NPHY, PhaseTrackAlpha2, 261)
		PHY_REG_WRITE_ENTRY(NPHY, PhaseTrackBeta0,  366)
		PHY_REG_WRITE_ENTRY(NPHY, PhaseTrackBeta1,  205)
		PHY_REG_WRITE_ENTRY(NPHY, PhaseTrackBeta2,  32)
	PHY_REG_LIST_EXECUTE(pi);
	/* Rev7 Specific */
	if (NREV_IS(pi->pubpi->phy_rev, 7)) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(NPHY, fineRx2clockgatecontrol,
				forcechanestgatedClksOn, 1)

			PHY_REG_MOD_ENTRY(NPHY, FreqGain0, freqGainVal0, 32)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain0, freqGainVal1, 39)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain1, freqGainVal2, 46)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain1, freqGainVal3, 51)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain2, freqGainVal4, 55)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain2, freqGainVal5, 58)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain3, freqGainVal6, 60)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain3, freqGainVal7, 62)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain4, freqGainVal8, 62)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain4, freqGainVal9, 63)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain5, freqGainVal10, 63)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain5, freqGainVal11, 64)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain6, freqGainVal12, 64)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain6, freqGainVal13, 64)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain7, freqGainVal14, 64)
			PHY_REG_MOD_ENTRY(NPHY, FreqGain7, freqGainVal15, 64)
		PHY_REG_LIST_EXECUTE(pi);
	} /* [end] Rev7 Specific */

	/* Rev7-8 Specific */
	if (NREV_LE(pi->pubpi->phy_rev, 8)) {
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(NPHY, forceFront0, 0x1b0)
			PHY_REG_WRITE_ENTRY(NPHY, forceFront1, 0x1b0)
		PHY_REG_LIST_EXECUTE(pi);
	} else if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		/* Make sure there is no problem, esp with RIFS */
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(NPHY, forceFront0, 0x7ff)
			PHY_REG_WRITE_ENTRY(NPHY, forceFront1, 0x7ff)
		PHY_REG_LIST_EXECUTE(pi);
	}

	/* Rev8+ Specific */
	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		/* LCNXN */
		/* 43228A0, based on values provided by Sathya/Chiranthan */

		if (!is_phybw40)
			phy_utils_mod_phyreg(pi, NPHY_txTailCountValue,
				NPHY_txTailCountValue_TailCountValue_MASK,
				(160 << NPHY_txTailCountValue_TailCountValue_SHIFT));
		else
			phy_utils_mod_phyreg(pi, NPHY_txTailCountValue,
				NPHY_txTailCountValue_TailCountValue_MASK,
				(100 << NPHY_txTailCountValue_TailCountValue_SHIFT));
	} else if (NREV_GE(pi->pubpi->phy_rev, 8)) {
		phy_utils_mod_phyreg(pi, NPHY_txTailCountValue,
		            NPHY_txTailCountValue_TailCountValue_MASK,
		            (114 << NPHY_txTailCountValue_TailCountValue_SHIFT));
	} /* [End] Rev8+ Specific */

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x00, 16, &dac_control);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x10, 16, &dac_control);

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_CMPMETRICDATAWEIGHTTBL,
	                    1, 0, 32, &leg_data_weights);
	leg_data_weights = leg_data_weights & 0xffffff;
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CMPMETRICDATAWEIGHTTBL,
	                     1, 0, 32, &leg_data_weights);

	/* Need to power up the DAC buffer in the RX2TX RF Sequence. The default value of
	 * 0x10b at offsets 0x15e, 0x15f, 0x16e, and 0x16f in the RFSeq table is incorrect
	 * for REV7.
	 */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
	                     3, 0x15d, 16, rfseq_lpf_ctl_lut_rev7);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
	                     3, 0x16d, 16, rfseq_lpf_ctl_lut_rev7);

	wlc_phy_set_rfseq_nphy(pi, NPHY_RFSEQ_TX2RX, rfseq_tx2rx_events_rev3,
	                       rfseq_tx2rx_dlys_rev3, sizeof(rfseq_tx2rx_events_rev3)/
	                       sizeof(rfseq_tx2rx_events_rev3[0]));

	if (PHY_IPA(pi)) {
		uint8 rfseq_rx2tx_dlys_rev3_ipa[] = {8, 6, 6, 4, 4, 16, 43, 1, 1};
		uint8 rfseq_rx2tx_events_rev3_ipa[] = {
			NPHY_REV3_RFSEQ_CMD_NOP,
			NPHY_REV3_RFSEQ_CMD_RXG_FBW,
			NPHY_REV3_RFSEQ_CMD_TR_SWITCH,
			NPHY_REV3_RFSEQ_CMD_CLR_HIQ_DIS,
			NPHY_REV3_RFSEQ_CMD_RXPD_TXPD,
			NPHY_REV3_RFSEQ_CMD_TX_GAIN,
			NPHY_REV3_RFSEQ_CMD_CLR_RXRX_BIAS,
			NPHY_REV3_RFSEQ_CMD_INT_PA_PU,
			NPHY_REV3_RFSEQ_CMD_END
		};

		wlc_phy_set_rfseq_nphy(pi, NPHY_RFSEQ_RX2TX, rfseq_rx2tx_events_rev3_ipa,
		                     rfseq_rx2tx_dlys_rev3_ipa,
		                     sizeof(rfseq_rx2tx_events_rev3_ipa)/
		                     sizeof(rfseq_rx2tx_events_rev3_ipa[0]));
	}

	/* use s0.12 PAPD epsilon fixed point format */
	PHY_REG_LIST_START
		PHY_REG_MOD_RAW_ENTRY(NPHY_EpsilonOverrideI_0,
			NPHY_REV7_EpsilonOverrideI_epsilonFixedPoint_MASK,
			0x1 << NPHY_REV7_EpsilonOverrideI_epsilonFixedPoint_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_EpsilonOverrideI_1,
			NPHY_REV7_EpsilonOverrideI_epsilonFixedPoint_MASK,
			0x1 << NPHY_REV7_EpsilonOverrideI_epsilonFixedPoint_SHIFT)
	PHY_REG_LIST_EXECUTE(pi);

	/* init LPF BW */
	FOREACH_CORE(pi, coreNum) {
		tx_lpf_bw_ofdm_20mhz[coreNum] = wlc_phy_read_lpf_bw_ctl_nphy(
			pi, (0x154 + coreNum * 0x10));
		tx_lpf_bw_ofdm_40mhz[coreNum] = wlc_phy_read_lpf_bw_ctl_nphy(
			pi, (0x159 + coreNum * 0x10));
		tx_lpf_bw_11b[coreNum] = wlc_phy_read_lpf_bw_ctl_nphy(
			pi, (0x152 + coreNum * 0x10));
	}

	/* keep LOGEN buffers on both cores ON all the time
	 * to fix LOFT issue on SISO Tx
	 */
	if ((!PHY_IPA(pi)) && (CHSPEC_IS5G(pi->radio_chanspec)) && (pi->sh->boardtype != 0xF52A)) {
		phy_utils_mod_radioreg(pi, RADIO_2057_LOGEN_PUS, 0xc, 0xc);
		phy_utils_mod_radioreg(pi, RADIO_2057v7_OVR_REG7, 0x80, 0x80);
		phy_utils_mod_radioreg(pi, RADIO_2057v7_OVR_REG6, 0x1, 0x1);
	}

	rccal_bcap_val = phy_utils_read_radioreg(pi, RADIO_2057_RCCAL_BCAP_VAL);
	rccal_scap_val = phy_utils_read_radioreg(pi, RADIO_2057_RCCAL_SCAP_VAL);

	if (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) {
		uint8 num_ant;
		phy_utils_write_phyreg(pi, NPHY_crsControll, 0x1c);
		phy_utils_mod_phyreg(pi, NPHY_CoreConfig, NPHY_CoreConfig_CoreMask_MASK,
		            0x2 << NPHY_CoreConfig_CoreMask_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_CoreConfig, NPHY_CoreConfig_NumRxCores_MASK,
		            1 << NPHY_CoreConfig_NumRxCores_SHIFT);
		num_ant = CHSPEC_IS2G(pi->radio_chanspec) ? pi->aa2g : pi->aa5g;
		num_ant = (uint8)(bcm_bitcount(&num_ant, sizeof(uint8)));
		phy_utils_mod_phyreg(pi, NPHY_CoreConfig, NPHY_CoreConfig_NumRxAnt_MASK,
		            num_ant << NPHY_CoreConfig_NumRxAnt_SHIFT);
	}

	/* LPF and RC cal tweaks
	 * Ensure that the RC cal override values do not go above 0x1f or less than 0
	 */
	if (PHY_IPA(pi)) {
		/* internal PA RC tweak */
		if (RADIOREV(pi->pubpi->radiorev) == 5) {
			if ((RADIOVER(pi->pubpi->radiover) == 0) &&
			    CHSPEC_IS40(pi->radio_chanspec)) {
				/* 5357A0 40 MHz */

				/* overwrite RC values */
				FOREACH_CORE(pi, coreNum) {
					rccal_tx20_11b_bcap[coreNum] = rccal_bcap_val;
					rccal_tx20_11b_scap[coreNum] = rccal_scap_val;
					rccal_tx20_11n_bcap[coreNum] = rccal_bcap_val;
					rccal_tx20_11n_scap[coreNum] = rccal_scap_val;
					rccal_tx40_11n_bcap[coreNum] =
					        (uint16) MIN((int16)rccal_bcap_val + 1, 0x1f);
					rccal_tx40_11n_scap[coreNum] =
					        (uint16) MIN((int16)rccal_scap_val + 3, 0x1f);
				}

				rccal_ovrd = TRUE;
			} else if (RADIOVER(pi->pubpi->radiover) > 0) {
				/* 5357B0 */
				tx_lpf_bw_ofdm_20mhz[0] = 3;
				tx_lpf_bw_ofdm_20mhz[1] = 2;
				FOREACH_CORE(pi, coreNum) {
					tx_lpf_bw_ofdm_40mhz[coreNum] = 4;

					rccal_tx20_11b_bcap[coreNum] = rccal_bcap_val;
					rccal_tx20_11b_scap[coreNum] = rccal_scap_val;
					if (coreNum == 0) {
						rccal_tx20_11n_bcap[coreNum] =
						        (uint16) MIN((int16)rccal_bcap_val +
						                     20, 0x1f);
						rccal_tx20_11n_scap[coreNum] =
						        (uint16) MIN((int16)rccal_scap_val +
						                     20, 0x1f);
					} else {
						rccal_tx20_11n_bcap[coreNum] =
						        (uint16) MIN((int16)rccal_bcap_val +
						                     16, 0x1f);
						rccal_tx20_11n_scap[coreNum] =
						        (uint16) MIN((int16)rccal_scap_val +
						                     16, 0x1f);
					}
					curr_channel =
					CHSPEC_CHANNEL(pi->radio_chanspec);
					if ((pi->sh->sromrev >= 8) &&
					    (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
					    BFL2_FCC_BANDEDGE_WAR) &&
					    ((curr_channel == 3) &&
					     CHSPEC_IS40(pi->radio_chanspec))) {
						/* RCCAL tweak for 40 MHz bandedge */
						rccal_tx40_11n_bcap[coreNum] = rccal_bcap_val;
						rccal_tx40_11n_scap[coreNum] = rccal_scap_val;
					} else {
						if (coreNum == 0) {
							rccal_tx40_11n_bcap[coreNum] =
							        (uint16)
							        MIN((int16)rccal_bcap_val +
							            20, 0x1f);
							rccal_tx40_11n_scap[coreNum] =
							        (uint16)
							        MIN((int16)rccal_scap_val +
							            20, 0x1f);
						} else {
							rccal_tx40_11n_bcap[coreNum] =
							        (uint16)
							        MIN((int16)rccal_bcap_val +
							            10, 0x1f);
							rccal_tx40_11n_scap[coreNum] =
							        (uint16)
							        MIN((int16)rccal_scap_val +
							            10, 0x1f);
						}
					}
				}

				rccal_ovrd = TRUE;
			}

		} else if (RADIOREV(pi->pubpi->radiorev) == 7) {
			/* 43236 */

			/* overwrite LPF BWs */
			FOREACH_CORE(pi, coreNum) {
				tx_lpf_bw_ofdm_20mhz[coreNum] = 4;
				tx_lpf_bw_11b[coreNum] = 1;

				rccal_tx20_11b_bcap[coreNum] = rccal_bcap_val;
				rccal_tx20_11b_scap[coreNum] = rccal_scap_val;

				/* overwrite RC values */
				if (RADIOVER(pi->pubpi->radiover) == 0x0) {
					/* 43236A0 */
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						rccal_tx20_11n_bcap[coreNum] = rccal_bcap_val;
						rccal_tx20_11n_scap[coreNum] = (uint16)
						        MIN((int16)rccal_scap_val + 2, 0x1f);
						rccal_tx40_11n_bcap[coreNum] = (uint16)
						        MAX((int16)rccal_bcap_val - 2, 0);
						rccal_tx40_11n_scap[coreNum] = rccal_scap_val;
					} else {
						rccal_tx20_11n_bcap[coreNum] = (uint16)
						        MIN((int16)rccal_bcap_val + 8, 0x1f);
						rccal_tx20_11n_scap[coreNum] = (uint16)
						        MIN((int16)rccal_scap_val + 10, 0x1f);
						rccal_tx40_11n_bcap[coreNum] = (uint16)
						        MIN((int16)rccal_bcap_val + 3, 0x1f);
						rccal_tx40_11n_scap[coreNum] = (uint16)
						        MIN((int16)rccal_scap_val + 5, 0x1f);
					}
				} else {
					/* 43236A0TC3/TC4 and 43236B0 */
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
					  rccal_tx20_11n_bcap[coreNum] = (uint16)
						MIN((int16)rccal_bcap_val + 18, 0x1f);
						rccal_tx20_11n_scap[coreNum] = (uint16)
						        MIN((int16)rccal_scap_val + 20, 0x1f);
						rccal_tx40_11n_bcap[coreNum] = rccal_bcap_val;
						rccal_tx40_11n_scap[coreNum] = (uint16)
						        MIN((int16)rccal_scap_val + 2, 0x1f);
					} else {
						rccal_tx20_11n_bcap[coreNum] = (uint16)
						        MIN((int16)rccal_bcap_val + 6, 0x1f);
						rccal_tx20_11n_scap[coreNum] = (uint16)
						        MIN((int16)rccal_scap_val + 8, 0x1f);
						if (pi->sh->boardtype ==
							BCM943236OLYMPICSULLEY_SSID) {
						  rccal_tx40_11n_bcap[coreNum] = (uint16)
							MIN((int16)rccal_bcap_val + 6, 0x1f);
						  rccal_tx40_11n_scap[coreNum] = (uint16)
							MIN((int16)rccal_scap_val + 6, 0x1f);
						} else {
						  rccal_tx40_11n_bcap[coreNum] = rccal_bcap_val;
						  rccal_tx40_11n_scap[coreNum] = (uint16)
						        MIN((int16)rccal_scap_val + 2, 0x1f);
						}
					}
				}
			}

			rccal_ovrd = TRUE;

		} else if (RADIOREV(pi->pubpi->radiorev) == 8) {
			/* 6362B0 */
			curr_channel = CHSPEC_CHANNEL(pi->radio_chanspec);

			/* overwrite LPF BWs */
			FOREACH_CORE(pi, coreNum) {
				tx_lpf_bw_ofdm_20mhz[coreNum] = 4;

				rccal_tx20_11b_bcap[coreNum] = rccal_bcap_val;
				rccal_tx20_11b_scap[coreNum] = rccal_scap_val;

				/* overwrite RC values */
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					rccal_tx20_11n_bcap[coreNum] = 0xc;
					rccal_tx20_11n_scap[coreNum] = 0xc;
					rccal_tx40_11n_bcap[coreNum] = rccal_bcap_val;
					rccal_tx40_11n_scap[coreNum] = rccal_scap_val;
					if (((curr_channel == 3) || (curr_channel == 9)) &&
						CHSPEC_IS40(pi->radio_chanspec)) {
						/* 6362R */
						rccal_tx40_11n_bcap[coreNum] = (uint16)
					        MAX((int16)rccal_bcap_val - 8, 0);
						rccal_tx40_11n_scap[coreNum] = (uint16)
					        MAX((int16)rccal_scap_val - 8, 0);
					}
				} else {
					rccal_tx20_11n_bcap[coreNum] = (uint16)
					        MAX((int16)rccal_bcap_val - 2, 0);
					rccal_tx20_11n_scap[coreNum] = (uint16)
					        MAX((int16)rccal_scap_val - 1, 0);
					rccal_tx40_11n_bcap[coreNum] = (uint16)
					        MAX((int16)rccal_bcap_val - 6, 0);
					rccal_tx40_11n_scap[coreNum] = (uint16)
					        MAX((int16)rccal_scap_val - 5, 0);

					if ((curr_channel == 38) &&
						CHSPEC_IS40(pi->radio_chanspec)) {
						/* 6362R */
						rccal_tx40_11n_bcap[0] = (uint16)
							MAX((int16)rccal_bcap_val - 10, 0);
						rccal_tx40_11n_scap[0] = (uint16)
							MAX((int16)rccal_scap_val - 9, 0);
						rccal_tx40_11n_bcap[1] = (uint16)
							MAX((int16)rccal_bcap_val - 13, 0);
						rccal_tx40_11n_scap[1] = (uint16)
							MAX((int16)rccal_scap_val - 12, 0);
					}

					if ((curr_channel == 62) &&
						CHSPEC_IS40(pi->radio_chanspec)) {
						/* 6362R */
						rccal_tx40_11n_bcap[0] = (uint16)
							MAX((int16)rccal_bcap_val - 11, 0);
						rccal_tx40_11n_scap[0] = (uint16)
							MAX((int16)rccal_scap_val - 11, 0);
						rccal_tx40_11n_bcap[1] = (uint16)
							MAX((int16)rccal_bcap_val - 8, 0);
						rccal_tx40_11n_scap[1] = (uint16)
							MAX((int16)rccal_scap_val - 8, 0);
					}

					if ((curr_channel == 102) &&
						CHSPEC_IS40(pi->radio_chanspec)) {
						/* 6362R */
						rccal_tx40_11n_bcap[coreNum] = (uint16)
							MAX((int16)rccal_bcap_val + 7, 0);
						rccal_tx40_11n_scap[coreNum] = (uint16)
							MAX((int16)rccal_scap_val + 6, 0);
					}
				}
			}

			rccal_ovrd = TRUE;
		} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
			/* BCM63268B0 */
			/* overwrite LPF BWs */
			FOREACH_CORE(pi, coreNum) {
				tx_lpf_bw_ofdm_20mhz[coreNum] = 4;

				rccal_tx20_11b_bcap[coreNum] = rccal_bcap_val;
				rccal_tx20_11b_scap[coreNum] = rccal_scap_val;

				/* overwrite RC values */
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					rccal_tx20_11n_bcap[coreNum] = 0xc;
					rccal_tx20_11n_scap[coreNum] = 0xc;
					rccal_tx40_11n_bcap[coreNum] = rccal_bcap_val;
					rccal_tx40_11n_scap[coreNum] = rccal_scap_val;
				} else {
					rccal_tx20_11n_bcap[coreNum] = (uint16)
					        MAX((int16)rccal_bcap_val - 2, 0);
					rccal_tx20_11n_scap[coreNum] = (uint16)
					        MAX((int16)rccal_scap_val - 1, 0);
					rccal_tx40_11n_bcap[coreNum] = (uint16)
					        MAX((int16)rccal_bcap_val - 6, 0);
					rccal_tx40_11n_scap[coreNum] = (uint16)
					        MAX((int16)rccal_scap_val - 5, 0);
				}
			}

			rccal_ovrd = TRUE;
		} else if ((RADIOREV(pi->pubpi->radiorev) == 13) ||
			(RADIOREV(pi->pubpi->radiorev) == 14)) {
			/* 53572a0, copied from 5357b0 */
			/* 43217 */

			if (RADIOREV(pi->pubpi->radiorev) == 14) {
				tx_lpf_bw_ofdm_20mhz[0] = 3;
				tx_lpf_bw_ofdm_20mhz[1] = 3;
			} else {
				tx_lpf_bw_ofdm_20mhz[0] = 3;
				tx_lpf_bw_ofdm_20mhz[1] = 2;
			}
			FOREACH_CORE(pi, coreNum) {
				tx_lpf_bw_ofdm_40mhz[coreNum] = 4;
				tx_lpf_bw_11b[coreNum] = 1;

				rccal_tx20_11b_bcap[coreNum] = rccal_bcap_val;
				rccal_tx20_11b_scap[coreNum] = rccal_scap_val;
				if (coreNum == 0) {
					rccal_tx20_11n_bcap[coreNum] =
						(uint16) MIN((int16)rccal_bcap_val + 20, 0x1f);
					rccal_tx20_11n_scap[coreNum] =
						(uint16) MIN((int16)rccal_scap_val + 20, 0x1f);
				} else {
					rccal_tx20_11n_bcap[coreNum] =
						(uint16) MIN((int16)rccal_bcap_val + 16, 0x1f);
					rccal_tx20_11n_scap[coreNum] =
						(uint16) MIN((int16)rccal_scap_val + 16, 0x1f);
				}
				curr_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
				if ((pi->sh->sromrev >= 8) &&
				    (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
				    BFL2_FCC_BANDEDGE_WAR) &&
				    ((curr_channel == 3) &&
				     CHSPEC_IS40(pi->radio_chanspec))) {
					/* RCCAL tweak for 40 MHz bandedge */
					rccal_tx40_11n_bcap[coreNum] = rccal_bcap_val;
					rccal_tx40_11n_scap[coreNum] = rccal_scap_val;
				} else {
					if (coreNum == 0) {
						rccal_tx40_11n_bcap[coreNum] = (uint16)
							MIN((int16)rccal_bcap_val + 20, 0x1f);
						rccal_tx40_11n_scap[coreNum] = (uint16)
							MIN((int16)rccal_scap_val + 20, 0x1f);
					} else {
						rccal_tx40_11n_bcap[coreNum] = (uint16)
							MIN((int16)rccal_bcap_val + 10, 0x1f);
						rccal_tx40_11n_scap[coreNum] = (uint16)
							MIN((int16)rccal_scap_val + 10, 0x1f);
					}
				}
			}

			rccal_ovrd = TRUE;
		}
	} else {
		/* external PA RC tweak */
		if ((RADIOREV(pi->pubpi->radiorev) == 5) ||
			(RADIOREV(pi->pubpi->radiorev) >= 7)) {

			/* overwrite LPF BWs */
			FOREACH_CORE(pi, coreNum) {
				tx_lpf_bw_ofdm_20mhz[coreNum] = 1;
				tx_lpf_bw_ofdm_40mhz[coreNum] = 3;

				/* overwrite RC values */
				rccal_tx20_11b_bcap[coreNum] = rccal_bcap_val;
				rccal_tx20_11b_scap[coreNum] = rccal_scap_val;

				rccal_tx20_11n_bcap[coreNum] =
				        (uint16) MIN((int16)rccal_bcap_val + 8, 0x1f);
				rccal_tx20_11n_scap[coreNum] =
				        (uint16) MIN((int16)rccal_scap_val + 8, 0x1f);
				rccal_tx40_11n_bcap[coreNum] =
				        (uint16) MIN((int16)rccal_bcap_val + 8, 0x1f);
				rccal_tx40_11n_scap[coreNum] =
				        (uint16) MIN((int16)rccal_scap_val + 8, 0x1f);
			}
			rccal_ovrd = TRUE;
		}
	}

	if (rccal_ovrd) {
		/* Write the rccal overrides values for bcap and scap along
		 * with lpf_bw_ctl into the RFSeq table
		 */
		local_rc_cal_en = 1;
		FOREACH_CORE(pi, coreNum) {
			/* Read the rx2tx_lpf_rc_lut lpf_bw_ctl values from the
			 * RFSeq table and insert the rccal bcap and scap values
			 * at the right locations
			 */
			rx2tx_lpf_rc_lut_tx20_11b = (local_rc_cal_en << 13) |
			        (rccal_tx20_11b_bcap[coreNum] << 8) |
			        (rccal_tx20_11b_scap[coreNum] << 3) |
				tx_lpf_bw_11b[coreNum];
			rx2tx_lpf_rc_lut_tx20_11n = (local_rc_cal_en << 13) |
			        (rccal_tx20_11n_bcap[coreNum] << 8) |
			        (rccal_tx20_11n_scap[coreNum] << 3) |
				tx_lpf_bw_ofdm_20mhz[coreNum];
			rx2tx_lpf_rc_lut_tx40_11n = (local_rc_cal_en << 13) |
			        (rccal_tx40_11n_bcap[coreNum] << 8) |
			        (rccal_tx40_11n_scap[coreNum] << 3) |
				tx_lpf_bw_ofdm_40mhz[coreNum];

			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x152 + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx20_11b);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x153 + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx20_11n);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x154  + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx20_11n);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x155 + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx40_11n);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x156 + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx40_11n);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x157  + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx40_11n);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x158 + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx40_11n);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
			                         0x159 + coreNum * 0x10,
			                         16, &rx2tx_lpf_rc_lut_tx40_11n);

			/* For 20mhz, we can always hack dupag for 20mhz_11b */
			/* Also needed for 53572 */
			if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV) && (!is_phybw40)) {
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1,
					0x158 + coreNum * 0x10, 16, &rx2tx_lpf_rc_lut_tx20_11b);
			}
		}
	}

	/* Execute only if not runnng on Quickturn */
	if (!NORADIO_ENAB(pi->pubpi)) {
		phy_utils_write_phyreg(pi, NPHY_PingPongComp, 0x3);
	}
	if (RADIOID(pi->pubpi->radioid) == BCM2057_ID) {
		if ((RADIOREV(pi->pubpi->radiorev) == 4) ||
			(RADIOREV(pi->pubpi->radiorev) == 6)) {
			wlc_phy_rfctrl_override_nphy_rev7(pi,
				NPHY_REV7_RfctrlOverride_tx_pu_MASK,
				1, 0x3, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		}

		if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
			(RADIOREV(pi->pubpi->radiorev) == 4) ||
			(RADIOREV(pi->pubpi->radiorev) == 6)) {
			if ((pi->sh->sromrev >= 8) &&
				(BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
				BFL2_IPALVLSHIFT_3P3))
				ipalvlshift_3p3_war_en = 1;

			if (ipalvlshift_3p3_war_en) {
				phy_utils_write_radioreg(pi, RADIO_2057_GPAIO_CONFIG, 0x5);
				phy_utils_write_radioreg(pi, RADIO_2057_GPAIO_SEL1, 0x30);
				phy_utils_write_radioreg(pi, RADIO_2057_GPAIO_SEL0, 0x0);
				phy_utils_or_radioreg(pi, RADIO_2057_RXTXBIAS_CONFIG_CORE0,
				                      0x1);
				phy_utils_or_radioreg(pi, RADIO_2057_RXTXBIAS_CONFIG_CORE1,
				                      0x1);

				/* increase bias from 0x19 */
				ipa2g_mainbias = 0x1f;
				/* change from 0x62 */
				ipa2g_casconv = 0x6f;
				/* change from 0x11 */
				ipa2g_biasfilt = 0xaa;
			} else {
				/* increase bias from 0x19 */
				ipa2g_mainbias = 0x2b;
				/* change from 0x62 */
				ipa2g_casconv = 0x7f;
				/* change from 0x11 */
				ipa2g_biasfilt = 0xee;
			}

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				FOREACH_CORE(pi, coreNum) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_IMAIN, ipa2g_mainbias);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_CASCONV, ipa2g_casconv);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_BIAS_FILTER, ipa2g_biasfilt);
				}
			}
		}
	}
	/* 2G Tx mixer bleed current */
	if (PHY_IPA(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
				(RADIOREV(pi->pubpi->radiorev) == 4) ||
				(RADIOREV(pi->pubpi->radiorev) == 6)) {
				/* Use pref vals for other radio revs */
				txgm_idac_bleed = 0x7f;
			}
			FOREACH_CORE(pi, coreNum) {
				if (txgm_idac_bleed != 0)
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
				                 TXGM_IDAC_BLEED, txgm_idac_bleed);
			}
			if (RADIOREV(pi->pubpi->radiorev) == 5) {
				/* 5357a0 */
				FOREACH_CORE(pi, coreNum) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_CASCONV, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, IPA2G_IMAIN,
					                 0x1f);

					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_BIAS_FILTER, 0xee);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, PAD2G_IDACS, 0x8a);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, PAD_BIAS_FILTER_BWS, 0x3e);
				}

			} else if (RADIOREV(pi->pubpi->radiorev) == 7) {
				/* 43236a0, TC3, TC4, B0 */
				if (!CHSPEC_IS40(pi->radio_chanspec)) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 IPA2G_IMAIN, 0x14);
					if (RADIOVER(pi->pubpi->radiover) == 0) {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
						                 IPA2G_IMAIN, 0x12);
					} else {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
						                 IPA2G_IMAIN, 0x14);
					}
				} else {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 IPA2G_IMAIN, 0x12);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 IPA2G_IMAIN, 0x12);
				}
				if (RADIOVER(pi->pubpi->radiover) > 0) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 IPA2G_BIAS_FILTER, 0xff);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 IPA2G_BIAS_FILTER, 0xff);
				}
				if (RADIOVER(pi->pubpi->radiover) == 2) {
					pi_nphy->nphy_papd_kill_switch_en = TRUE;
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_IDACS, 0x8a);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_IDACS, 0x8a);
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 8) {
				/* 6362B0 */
				FOREACH_CORE(pi, coreNum) {
					if (!CHSPEC_IS40(pi->radio_chanspec)) {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
						                 coreNum, IPA2G_IMAIN,
						                 0x14);
					} else {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
						                 coreNum, IPA2G_IMAIN,
						                 0x1c);
					}
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
				/* BCM63268B0 */
				if (!CHSPEC_IS40(pi->radio_chanspec)) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x43);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x23);
				} else {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x03);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x03);
				}
				FOREACH_CORE(pi, coreNum) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, IPA2G_BIAS_FILTER,
					                 0xff);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, BACKUP1,
					                 0x10);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, TXMIX2G_TUNE_BOOST_PU,
					                 0x41);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_IMAIN, 0x1e);
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 13) {
				/* 53572A0 */
				FOREACH_CORE(pi, coreNum) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_CASCONV, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, IPA2G_IMAIN,
					                 0x26);

					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_BIAS_FILTER, 0xee);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, PAD2G_IDACS, 0x8a);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, PAD_BIAS_FILTER_BWS,
					                 0x3e);
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
				/* 43217 */
				pi_nphy->nphy_papd_kill_switch_en = TRUE;
				FOREACH_CORE(pi, coreNum) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
						IPA2G_CASCONV, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
						TXMIX2G_TUNE_BOOST_PU, 0x21);
					if (RADIOVER(pi->pubpi->radiover) == 0) {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
							coreNum, IPA2G_BIAS_FILTER, 0xFF);
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
							coreNum, PAD2G_IDACS, 0x88);
						if (CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec))
							<= 2452) {
							WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
								coreNum, PAD2G_TUNE_PUS, 0x23);
						} else {
							WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
								coreNum, PAD2G_TUNE_PUS, 0x13);
						}
					} else {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
							coreNum, IPA2G_BIAS_FILTER, 0xff);
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
							coreNum, PAD2G_IDACS, 0x8a);
						if (CHSPEC_IS40(pi->radio_chanspec) ==
							1) {
							WRITE_RADIO_REG4(pi, RADIO_2057,
								CORE, coreNum,
								PAD2G_TUNE_PUS, 0x23);
						} else {
							if (CHAN2G_FREQ(
								CHSPEC_CHANNEL(pi->radio_chanspec))
								<= 2452) {
								WRITE_RADIO_REG4(pi, RADIO_2057,
									CORE, coreNum,
									PAD2G_TUNE_PUS, 0x63);
							} else {
								WRITE_RADIO_REG4(pi, RADIO_2057,
									CORE, coreNum,
									PAD2G_TUNE_PUS, 0x53);
							}
						}
					}
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_IMAIN, 0x16);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 PAD_BIAS_FILTER_BWS, 0x3e);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 BACKUP1, 0x10);
				}
			} else {
				PHY_ERROR(("Unsupported 2057 radio rev %d\n",
				           RADIOREV(pi->pubpi->radiorev)));
				ASSERT(0);
			}

		}
	} else {
		/* Note: For 5357A0 (radio rev 5), the radio xls contains the preferred
		 * registers and tuning tables corresponding to the external PA boards.
		 * ePA-specific settings for other radio revs are not prefvals in .xls
		 */
		if ((RADIOREV(pi->pubpi->radiorev) <= 4) ||
			(RADIOREV(pi->pubpi->radiorev) == 6)) {
			FOREACH_CORE(pi, coreNum) {
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
				                 TXGM_IDAC_BLEED, 0x70);
			}
		} else if ((RADIOREV(pi->pubpi->radiorev) == 7) ||
			(RADIOREV(pi->pubpi->radiorev) == 8) ||
			(RADIOREV(pi->pubpi->radiorev) == 12)) { /* BCM63268 */
			FOREACH_CORE(pi, coreNum) {
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
				                 IPA2G_CASCONV, 0xf);
			}
		}
	}

	/* AFE mode and settings */
	if (RADIOREV(pi->pubpi->radiorev) == 4) {
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x05, 16,
		                     &afectrl_adc_ctrl1_rev7);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x15, 16,
		                     &afectrl_adc_ctrl1_rev7);

		FOREACH_CORE(pi, coreNum) {
			WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum, AFE_VCM_CAL_MASTER,
			                 0x0);
			WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum, AFE_SET_VCM_I,
			                 0x3f);
			WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum, AFE_SET_VCM_Q,
			                 0x3f);
		}

	} else if (RADIOREV(pi->pubpi->radiorev) == 7) {

		/* power ADC down before mode change */
		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_adc_pd_MASK,
				NPHY_REV3_AfectrlCore_adc_pd_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_adc_pd_MASK,
				NPHY_REV3_AfectrlCore_adc_pd_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK)
		PHY_REG_LIST_EXECUTE(pi);

		if (CHSPEC_IS20(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				/* switch ADC to low power mode: set override value */
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
					NPHY_REV3_AfectrlCore_adc_lp_MASK,
					NPHY_REV3_AfectrlCore_adc_lp_MASK)
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
					NPHY_REV3_AfectrlCore_adc_lp_MASK,
					NPHY_REV3_AfectrlCore_adc_lp_MASK)
				/* activate LP mode override */
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK)
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK)
			PHY_REG_LIST_EXECUTE(pi);

			/* use correct control bias current of i/q adc reference buffer */
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x04, 16,
			                         &afectrl_adc_ctrl0_bw20_rev7);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x14, 16,
			                         &afectrl_adc_ctrl0_bw20_rev7);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x05, 16,
			                         &afectrl_adc_ctrl1_bw20_rev7);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x15, 16,
			                         &afectrl_adc_ctrl1_bw20_rev7);
		} else {
			PHY_REG_LIST_START
				/* switch ADC to nominal power mode: set override value */
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
					NPHY_REV3_AfectrlCore_adc_lp_MASK, 0)
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
					NPHY_REV3_AfectrlCore_adc_lp_MASK, 0)
				/* activate LP mode override */
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK)
				PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK,
					NPHY_REV3_AfectrlOverride_adc_lp_MASK)
			PHY_REG_LIST_EXECUTE(pi);

			/* use correct control bias current of i/q adc reference buffer */
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x04, 16,
			                         &afectrl_adc_ctrl0_bw40_rev7);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x14, 16,
			                         &afectrl_adc_ctrl0_bw40_rev7);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x05, 16,
			                         &afectrl_adc_ctrl1_bw40_rev7);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x15, 16,
			                         &afectrl_adc_ctrl1_bw40_rev7);
		}

		/* power ADC up and clear PD override */
		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_adc_pd_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_adc_pd_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK, 0)
		PHY_REG_LIST_EXECUTE(pi);

	} else {

		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_adc_pd_MASK,
				NPHY_REV3_AfectrlCore_adc_pd_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_adc_pd_MASK,
				NPHY_REV3_AfectrlCore_adc_pd_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_adc_lp_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_adc_lp_MASK,
				NPHY_REV3_AfectrlOverride_adc_lp_MASK)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_adc_lp_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_adc_lp_MASK,
				NPHY_REV3_AfectrlOverride_adc_lp_MASK)
		PHY_REG_LIST_EXECUTE(pi);

		/* use correct control bias current of i/q adc reference buffer */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x05, 16,
		                         &afectrl_adc_ctrl2_rev7);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x15, 16,
		                         &afectrl_adc_ctrl2_rev7);

		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_adc_pd_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_adc_pd_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_adc_pd_MASK, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	/* In event of high power spurs/interference that causes crs-glitches,
	 * stay in WAIT_ENERGY_DROP for 1 clk20 instead of default 1 ms
	 * this way, we get back to CARRIER_SEARCH quickly
	 * and will less likely to miss actual packets
	 * ps. this is actually one settings for ACI
	 */
#ifdef NPHYREV7_HTPHY_DFS_WAR
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_utils_write_phyreg(pi, NPHY_energydroptimeoutLen, 0x2);
	} else {
		/* use 1ms WAIT_ENERGY_DROP for DFS detection in 5G */
		phy_utils_write_phyreg(pi, NPHY_energydroptimeoutLen, 0x9c40);
		/* change to 0x8 to prevent the radar to trigger fine timing */
	}
#else
	phy_utils_write_phyreg(pi, NPHY_energydroptimeoutLen, 0x2);
#endif // endif

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1, 256, 32, &min_nvar_offset_6mbps);

	/* pktgn_hpc mid,lo */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x138, 16, &rfseq_pktgn_lpf_hpc_rev7);
	/* pktgn_hpc hi */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, 0x141, 16, &rfseq_pktgn_lpf_h_hpc_rev7);
	/*  htpktgn_hpc hi,mid,lo */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 3, 0x133, 16, &rfseq_htpktgn_lpf_hpc_rev7);
	/* cckpktgn_hpc mid,lo */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x146, 16, &rfseq_cckpktgn_lpf_hpc_rev7);
	/* tx2rx_hpc hi */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, 0x123, 16, &rfseq_tx2rx_lpf_h_hpc_rev7);
	/* rx2tx_hpc hi */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, 0x12A, 16, &rfseq_rx2tx_lpf_h_hpc_rev7);

	/* WARNING: we need to also write back the base_nvar, otherwise write
	 * command will pick up stale lower 32-bit value if its available.
	 */
	if (CHSPEC_IS40(pi->radio_chanspec) == 0) {
		wlc_phy_table_read_nphy(pi,  NPHY_TBL_ID_NOISEVAR, 1, 2,   32, &base_nvar_buf[0]);
		base_nvar_buf[1] = (uint32) min_nvar_val;
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 2, 2,   32, &base_nvar_buf);

		wlc_phy_table_read_nphy(pi,  NPHY_TBL_ID_NOISEVAR, 1, 126, 32, &base_nvar_buf[0]);
		base_nvar_buf[1] = (uint32) min_nvar_val;
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 2, 126, 32, &base_nvar_buf);
	} else {
		wlc_phy_table_read_nphy(pi,  NPHY_TBL_ID_NOISEVAR, 1, 2, 32, &base_nvar_buf[0]);
		base_nvar_buf[1] = noise_var_tbl_rev7[3];
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 2, 2, 32, &base_nvar_buf);

		wlc_phy_table_read_nphy(pi,  NPHY_TBL_ID_NOISEVAR, 1, 126, 32, &base_nvar_buf[0]);
		base_nvar_buf[1] = noise_var_tbl_rev7[127];
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 2, 126, 32, &base_nvar_buf);
	}

	if (CHIPID_43236X_FAMILY(pi))
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 4, 264, 32,
		                         &min_nvar_offset_mcs0to3);

	if ((CHIPID_43236X_FAMILY(pi) || (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID)) &&
	    (CHSPEC_IS2G(pi->radio_chanspec))) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(NPHY, crsControlu, mfLessAve, 0)
			PHY_REG_MOD_ENTRY(NPHY, crsControll, mfLessAve, 0)
			PHY_REG_MOD_ENTRY(NPHY, crsThreshold2u, peakThresh, 85)
			PHY_REG_MOD_ENTRY(NPHY, crsThreshold2l, peakThresh, 85)
		PHY_REG_LIST_EXECUTE(pi);
	}

	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
		if (!CHSPEC_IS5G(pi->radio_chanspec))
			phy_utils_write_phyreg(pi, NPHY_PeakCnt_duringACI, 0x1);
	}

	/* gain control workarounds */
	wlc_phy_workarounds_nphy_gainctrl(pi);

	/* init HW Rx antsel */
	wlc_phy_init_hw_antsel(pi);

	/* Band-specific RF PLL BW settings set in wlc_phy_chanspec_radio2057_setup
	 * since tuning table gets loaded everytime a scan happens.
	 */

	/* SET Vmid and Av for {RSSI, TSSI} based on ID pdetrangeXg FROM SROM
	 * The following settings should only be for REV7 given the TSSI issues
	 */
	pdetrange = (CHSPEC_IS5G(pi->radio_chanspec)) ? pi->fem5g->pdetrange : pi->fem2g->pdetrange;

	chan_freq_range = wlc_phy_get_chan_freq_range_nphy(pi, 0);

	channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (!wlc_phy_chan2freq_nphy(pi, channel, &freq, &t0, &t1, &t2)) {
		PHY_ERROR(("wlc_phy_get_chan_freq_range_nphy: channel invalid\n"));
	}

	if (pdetrange == 0) {
		if (chan_freq_range != WL_CHAN_FREQ_RANGE_2G) {
			aux_adc_vmid_rev7_core0[3] = 0x70;
			aux_adc_vmid_rev7_core1[3] = 0x70;
			aux_adc_gain_rev7_core0[3] = 2;
			aux_adc_gain_rev7_core1[3] = 2;
		} else {
			aux_adc_vmid_rev7_core0[3] = 0x80;
			aux_adc_vmid_rev7_core1[3] = 0x80;
			aux_adc_gain_rev7_core0[3] = 3;
			aux_adc_gain_rev7_core1[3] = 3;
		}
	} else if (pdetrange == 1) {
		if (chan_freq_range != WL_CHAN_FREQ_RANGE_2G) {
			aux_adc_vmid_rev7_core0[3] = 0x7c;
			aux_adc_vmid_rev7_core1[3] = 0x7c;
			aux_adc_gain_rev7_core0[3] = 2;
			aux_adc_gain_rev7_core1[3] = 2;
		} else {
			aux_adc_vmid_rev7_core0[3] = 0x8c;
			aux_adc_vmid_rev7_core1[3] = 0x8c;
			aux_adc_gain_rev7_core0[3] = 1;
			aux_adc_gain_rev7_core1[3] = 1;
		}
	} else if (pdetrange == 2) {
		if (RADIOID(pi->pubpi->radioid) == BCM2057_ID) {
			if (RADIOREV(pi->pubpi->radiorev) == 14) {
				/* 43217 */
				aux_adc_vmid_rev7_core0[3] = 0x80;
				aux_adc_vmid_rev7_core1[3] = 0x90;
				aux_adc_gain_rev7_core0[3] = 3;
				aux_adc_gain_rev7_core1[3] = 3;
			} else if (RADIOREV(pi->pubpi->radiorev) == 13) {
				/* 53572 */
				aux_adc_vmid_rev7_core0[3] = 0x50;
				aux_adc_vmid_rev7_core1[3] = 0x54;
				aux_adc_gain_rev7_core0[3] = 5;
				aux_adc_gain_rev7_core1[3] = 5;
			} else if (RADIOREV(pi->pubpi->radiorev) == 5) {
				/* 5357 */
				aux_adc_vmid_rev7_core0[3] = 0x88;
				aux_adc_vmid_rev7_core1[3] = 0x88;
				aux_adc_gain_rev7_core0[3] = 0;
				aux_adc_gain_rev7_core1[3] = 0;
			} else if (RADIOREV(pi->pubpi->radiorev) == 7) {
				/* 43236 */
			  if (chan_freq_range == WL_CHAN_FREQ_RANGE_2G) {
				if (RADIOVER(pi->pubpi->radiover) == 2) {
				  aux_adc_vmid_rev7_core0[3] = 0x90;
				  aux_adc_vmid_rev7_core1[3] = 0x95;
				  aux_adc_gain_rev7_core0[3] = 0;
				  aux_adc_gain_rev7_core1[3] = 0;
				}	else {
				  aux_adc_vmid_rev7_core0[3] = 0x7e;
				  aux_adc_vmid_rev7_core1[3] = 0x89;
				  aux_adc_gain_rev7_core0[3] = 1;
				  aux_adc_gain_rev7_core1[3] = 1;
				}
			  } else {
				if ((pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID) &&
					(chan_freq_range == WL_CHAN_FREQ_RANGE_5GM)) {
				  /* Do nothing.
					 This is set at subband_cust_43236B1_nphy
				  */
				} else {
				  if (RADIOVER(pi->pubpi->radiover) == 2) {
					aux_adc_vmid_rev7_core0[3] = 0x66;
					aux_adc_vmid_rev7_core1[3] = 0x66;
					aux_adc_gain_rev7_core0[3] = 4;
					aux_adc_gain_rev7_core1[3] = 4;
				  } else {
					aux_adc_vmid_rev7_core0[3] = 0x6c;
					aux_adc_vmid_rev7_core1[3] = 0x74;
					aux_adc_gain_rev7_core0[3] = 3;
					aux_adc_gain_rev7_core1[3] = 3;
				  }
				}
			  }
			} else if (RADIOREV(pi->pubpi->radiorev) == 8) {
				/* 6362 */
				if (chan_freq_range == WL_CHAN_FREQ_RANGE_2G) {
					aux_adc_vmid_rev7_core0[3] = 0x8b;
					aux_adc_vmid_rev7_core1[3] = 0x96;
					aux_adc_gain_rev7_core0[3] = 0;
					aux_adc_gain_rev7_core1[3] = 0;
				} else {
					aux_adc_vmid_rev7_core0[3] = 0x64;
					aux_adc_vmid_rev7_core1[3] = 0x64;
					aux_adc_gain_rev7_core0[3] = 5;
					aux_adc_gain_rev7_core1[3] = 5;
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
				/* BCM63268 */
				if (chan_freq_range == WL_CHAN_FREQ_RANGE_2G) {
					aux_adc_vmid_rev7_core0[3] = 0x8b;
					aux_adc_vmid_rev7_core1[3] = 0x96;
					aux_adc_gain_rev7_core0[3] = 0;
					aux_adc_gain_rev7_core1[3] = 0;
				} else {
					aux_adc_vmid_rev7_core0[3] = 0x64;
					aux_adc_vmid_rev7_core1[3] = 0x68;
					aux_adc_gain_rev7_core0[3] = 5;
					aux_adc_gain_rev7_core1[3] = 6;
				}
			} else {
				PHY_ERROR(("ERROR, UNKNOWN TSSI Vmid/Av FOR 2057 "
				           "radio rev %d\n", (int)pi->pubpi->radiorev));
			}
		}

	} else if (pdetrange == 3) {
		if (chan_freq_range == WL_CHAN_FREQ_RANGE_2G) {
			/* External PA is SiGe 2598L */
			aux_adc_vmid_rev7_core0[3] = 0x89;
			aux_adc_vmid_rev7_core1[3] = 0x89;
			aux_adc_gain_rev7_core0[3] = 0;
			aux_adc_gain_rev7_core1[3] = 0;
		} else {
			/* External PA is LX5530LQ */
			aux_adc_vmid_rev7_core0[3] = 0x92;
			aux_adc_vmid_rev7_core1[3] = 0x92;
			aux_adc_gain_rev7_core0[3] = 0;
			aux_adc_gain_rev7_core1[3] = 0;
		}

	} else if (pdetrange == 4) {
		if (chan_freq_range != WL_CHAN_FREQ_RANGE_2G) {
			/* External PA is SKY 65137 */
			aux_adc_vmid_rev7_core0[3] = 0x3e;
			aux_adc_vmid_rev7_core1[3] = 0x3e;
			aux_adc_gain_rev7_core0[3] = 0;
			aux_adc_gain_rev7_core1[3] = 0;
		} else {
			/* External PA is SiGe 2576L */
			PHY_ERROR(("ERROR, UNKNOWN TSSI Vmid/Av FOR 2057 "
			           "External  is SiGe 2576L fem2g.pdetrange %d\n",
			           pdetrange));
			ASSERT(0);
		}

	} else if (pdetrange == 5) {
		/* Eiffel Dual Band FEM with external PA - SE5503A */
		if (chan_freq_range != WL_CHAN_FREQ_RANGE_2G) {
			aux_adc_vmid_rev7_core0[3] = 0x80;
			aux_adc_vmid_rev7_core1[3] = 0x80;
			aux_adc_gain_rev7_core0[3] = 3;
			aux_adc_gain_rev7_core1[3] = 3;
		} else {
			aux_adc_vmid_rev7_core0[3] = 0x70;
			aux_adc_vmid_rev7_core1[3] = 0x70;
			aux_adc_gain_rev7_core0[3] = 2;
			aux_adc_gain_rev7_core1[3] = 2;
		}
	} else if (pdetrange == 6) {
		if (chan_freq_range != WL_CHAN_FREQ_RANGE_2G) {
			/* External PA is LX5530LQ */
			aux_adc_vmid_rev7_core0[3] = 0x24;
			aux_adc_vmid_rev7_core1[3] = 0x13;
			aux_adc_gain_rev7_core0[3] = 0x0;
			aux_adc_gain_rev7_core1[3] = 0x1;
		} else {
			/* External PA is RTC6691H */
			aux_adc_vmid_rev7_core0[3] = 0x25;
			aux_adc_vmid_rev7_core1[3] = 0x25;
			aux_adc_gain_rev7_core0[3] = 0x1;
			aux_adc_gain_rev7_core1[3] = 0x1;
		}

	} else if (pdetrange == 8) {
		if (chan_freq_range != WL_CHAN_FREQ_RANGE_2G) {
			aux_adc_vmid_rev7_core0[3] = 0x24;
			aux_adc_vmid_rev7_core1[3] = 0x13;
			aux_adc_gain_rev7_core0[3] = 0x0;
			aux_adc_gain_rev7_core1[3] = 0x1;
		} else {
			aux_adc_vmid_rev7_core0[3] = 0x46;
			aux_adc_vmid_rev7_core1[3] = 0x46;
			aux_adc_gain_rev7_core0[3] = 0x0;
			aux_adc_gain_rev7_core1[3] = 0x0;
		}
	}
	else if (pdetrange == 9) {
		if (chan_freq_range != WL_CHAN_FREQ_RANGE_2G) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				if (freq >= 4920 && freq <= 5190)
				{
					aux_adc_vmid_rev7_core0[3] = 0x96;
					aux_adc_vmid_rev7_core1[3] = 0x9B;
					aux_adc_gain_rev7_core0[3] = 0x1;
					aux_adc_gain_rev7_core1[3] = 0x3;
				} else if (freq == 5230) {
					aux_adc_vmid_rev7_core0[3] = 0x96;
					aux_adc_vmid_rev7_core1[3] = 0x87;
					aux_adc_gain_rev7_core0[3] = 0x1;
					aux_adc_gain_rev7_core1[3] = 0x3;
				} else if (freq >= 5270 && freq <= 5310) {
					aux_adc_vmid_rev7_core0[3] = 0xAA;
					aux_adc_vmid_rev7_core1[3] = 0x9B;
					aux_adc_gain_rev7_core0[3] = 0x0;
					aux_adc_gain_rev7_core1[3] = 0x3;
				} else if (freq >= 5510 && freq <= 5550) {
					aux_adc_vmid_rev7_core0[3] = 0xAA;
					aux_adc_vmid_rev7_core1[3] = 0xA0;
					aux_adc_gain_rev7_core0[3] = 0x0;
					aux_adc_gain_rev7_core1[3] = 0x1;
				} else if (freq == 5590) {
					 aux_adc_vmid_rev7_core0[3] = 0xA5;
					 aux_adc_vmid_rev7_core1[3] = 0xA0;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x1;
				} else if (freq >= 5630 && freq <= 5690) {
					 aux_adc_vmid_rev7_core0[3] = 0xA5;
					 aux_adc_vmid_rev7_core1[3] = 0xAF;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x1;
				} else if (freq == 5755) {
					 aux_adc_vmid_rev7_core0[3] = 0x9B;
					 aux_adc_vmid_rev7_core1[3] = 0xB4;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x1;
				} else {
					 aux_adc_vmid_rev7_core0[3] = 0x9B;
					 aux_adc_vmid_rev7_core1[3] = 0xB4;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x1;
				}
			} else {
				 if (freq >= 4920 && freq <= 5180)
				 {
					aux_adc_vmid_rev7_core0[3] = 0x96;
					aux_adc_vmid_rev7_core1[3] = 0x9B;
					aux_adc_gain_rev7_core0[3] = 0x1;
					aux_adc_gain_rev7_core1[3] = 0x2;
				 } else  if (freq >= 5200 && freq <= 5240) {
					 aux_adc_vmid_rev7_core0[3] = 0x96;
					 aux_adc_vmid_rev7_core1[3] = 0x82;
					 aux_adc_gain_rev7_core0[3] = 0x1;
					 aux_adc_gain_rev7_core1[3] = 0x2;
				 } else if (freq >= 5260 && freq <= 5280) {
					 aux_adc_vmid_rev7_core0[3] = 0xAA;
					 aux_adc_vmid_rev7_core1[3] = 0xA5;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x2;
				 } else if (freq >= 5300 && freq <= 5320) {
					 aux_adc_vmid_rev7_core0[3] = 0xAA;
					 aux_adc_vmid_rev7_core1[3] = 0x96;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x2;
				 } else if (freq >= 5500 && freq <= 5580) {
					 aux_adc_vmid_rev7_core0[3] = 0xA5;
					 aux_adc_vmid_rev7_core1[3] = 0x96;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x0;
				 } else if (freq >= 5600 && freq <= 5700) {
					 aux_adc_vmid_rev7_core0[3] = 0x9B;
					 aux_adc_vmid_rev7_core1[3] = 0x9B;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x0;
				 } else {
					 aux_adc_vmid_rev7_core0[3] = 0x9B;
					 aux_adc_vmid_rev7_core1[3] = 0xAF;
					 aux_adc_gain_rev7_core0[3] = 0x0;
					 aux_adc_gain_rev7_core1[3] = 0x0;
				 }
			}
		} else {
			aux_adc_vmid_rev7_core0[3] = 0x46;
			aux_adc_vmid_rev7_core1[3] = 0x46;
			aux_adc_gain_rev7_core0[3] = 0x0;
			aux_adc_gain_rev7_core1[3] = 0x0;
		}
	}
	else {
		PHY_ERROR(("invalid fem%s.pdetrange %d\n",
		         ((CHSPEC_IS5G(pi->radio_chanspec)) ? "5g" : "2g"),
		         pdetrange));
		ASSERT(0);
	}

	PHY_TXPWR(("TSSI AUX ADC Vmid 0x%x 0x%x\n",
		aux_adc_vmid_rev7_core0[3], aux_adc_vmid_rev7_core1[3]));
	PHY_TXPWR(("TSSI AUX ADC Gain 0x%x 0x%x\n",
		aux_adc_gain_rev7_core0[3], aux_adc_gain_rev7_core1[3]));

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 4, 0x08, 16, &aux_adc_vmid_rev7_core0);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 4, 0x18, 16, &aux_adc_vmid_rev7_core1);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 4, 0x0c, 16, &aux_adc_gain_rev7_core0);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 4, 0x1c, 16, &aux_adc_gain_rev7_core1);

	/* In Power-Save mode (Driver Only), Ucode modify
	 * a) Bit10 of the RfctrlCmd Reg to 0 -> shut down Radio
	 * b) AfectrlOverride Regs to 0x7ff -> shut down AFE
	 * Ensure that values in AfectrlCore regs shut down the AFE except BandGap.
	 *
	 * If BandGap is shuted down,
	 * there is a AFE power-up sequence needed for Stable Operation
	 * For ease of coding, ignore shutting down BandGap.
	 * - Bandgap consumes only 0.7mW of power.
	 *
	 * To Power Up AFE/Radio by Ucode in Driver,
	 * a) Bit10 of the RfctrlCmd Reg to 1
	 * b) AfectrlOverride Regs to 0x1
	 * (note in Tcl, no TxPwrCtl, Dac gain is overrided.
	 * Afectrloverride = 0x101)
	 */
	PHY_REG_LIST_START
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
			NPHY_REV3_AfectrlCore_adc_pd_MASK,
			NPHY_REV3_AfectrlCore_adc_pd_MASK)
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
			NPHY_REV3_AfectrlCore_adc_pd_MASK,
			NPHY_REV3_AfectrlCore_adc_pd_MASK)
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
			NPHY_REV3_AfectrlCore_dac_pd_MASK,
			NPHY_REV3_AfectrlCore_dac_pd_MASK)
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
			NPHY_REV3_AfectrlCore_dac_pd_MASK,
			NPHY_REV3_AfectrlCore_dac_pd_MASK)
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
			NPHY_REV3_AfectrlCore_rssi_pd_MASK,
			NPHY_REV3_AfectrlCore_rssi_pd_MASK)
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
			NPHY_REV3_AfectrlCore_rssi_pd_MASK,
			NPHY_REV3_AfectrlCore_rssi_pd_MASK)
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
			NPHY_REV3_AfectrlCore_bg_pd_MASK, 0)
		PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
			NPHY_REV3_AfectrlCore_bg_pd_MASK, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Check if we should Tx CCK pkts on Ant 0 only */
	if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_SINGLEANT_CCK) {
		wlapi_bmac_mhf(pi->sh->physhim,  MHF4, MHF4_BPHY_TXCORE0,
		               MHF4_BPHY_TXCORE0, WLC_BAND_ALL);
	}

	/* 53572a0 */
	/* PHY workarounds for rev17 */
	if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
		wlc_phy_workarounds_nphy_rev17(pi);
	}

	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV + 1)) {
		phy_utils_mod_phyreg(pi, NPHY_SpareReg, 1, 0);
	}
	/* Olympic Sulley boards using 43236B1 */
	if (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID) {
		if (CHSPEC_IS2G(pi->radio_chanspec) ||
		    (CHSPEC_IS5G(pi->radio_chanspec) &&
		     (pi->fem5g->antswctrllut == 4))) {
			/* When in RX mode,
			 * in 2.4G, look up entries 13, 14 and 45, 46 for T and R.
			 * in 5G and if antswctl5g=4, look up entries +13, +14 for T and R
			 */
			PHY_REG_LIST_START
				PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
					NPHY_AntSelConfig2057_RxAntSel_SrcSelect_MASK)
				PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
					NPHY_AntSelConfig2057_MAC_RxAntConfig_MASK)
				PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
					NPHY_AntSelConfig2057_Trigger_MASK)
			PHY_REG_LIST_EXECUTE(pi);
		}

		if (pi->sh->boardrev <= 0x1110) {
			PHY_REG_LIST_START
				PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlLUTLna1, 0xFF, 0x0)
				PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlLUTLna2, 0xFF, 0x0)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}

	/* Usb elna  boards using 43236B1 */
	if ((CHSPEC_IS2G(pi->radio_chanspec) &&
		(pi->fem2g->antswctrllut == 11)) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
		(pi->fem5g->antswctrllut == 11)))
		 {
		PHY_REG_LIST_START
			PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
				NPHY_AntSelConfig2057_RxAntSel_SrcSelect_MASK)
			PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
				NPHY_AntSelConfig2057_MAC_RxAntConfig_MASK)
			PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
				NPHY_AntSelConfig2057_Trigger_MASK)
		PHY_REG_LIST_EXECUTE(pi);
		}

	/* use more relaxed LTRN min-condition (fine-str based radar/garbage rejection)
	 *   - revisit later when radar detection needed in 5G
	 *   - see http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BCM43236B0LabNotebook85
	 */
	if (CHIPID_43236X_FAMILY(pi)) {
		PHY_REG_LIST_START
			/* use tighter (smaller) threshold later, i.e.,
			 * not until higher rx pow
			 */
			PHY_REG_MOD_ENTRY(NPHY, FSTRHiPwrTh, finestr_hiPwr_th, 0x42)
			/* increase hi-power threshold: make easier for
			 * metric min to be below
			 */
			PHY_REG_WRITE_ENTRY(NPHY, FSTRMetricTh, 0x0a20)
		PHY_REG_LIST_EXECUTE(pi);
	}

	if (pi->sh->sromrev >= 9) {
		/* set PHY register to enable the PHY part.
		 * ucode is enabled via host flag that is set in wlc_mhfdef() in wlc_bmac.c
		 */
		phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlCmd,
		            NPHY_REV8_TxPwrCtrlCmd_use_perPktPowerOffset_MASK,
		            (1 << NPHY_REV8_TxPwrCtrlCmd_use_perPktPowerOffset_SHIFT));
	}

	wlapi_bmac_mhf(pi->sh->physhim,  MHF5, 0x1000, 0x1000, WLC_BAND_2G);

	wlc_phy_set_rfseq_nphy(pi, NPHY_RFSEQ_TX2RX, rfseq_tx2rx_events_rev3,
		rfseq_tx2rx_dlys_ext_pa, ARRAYSIZE(rfseq_tx2rx_events_rev3));

	if ((CHSPEC_IS2G(pi->radio_chanspec) && ext_pa_ana_2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && ext_pa_ana_5g)) {
		wlc_phy_set_rfseq_nphy(pi, NPHY_RFSEQ_TX2RX, rfseq_tx2rx_events_rev3,
			rfseq_tx2rx_dlys_ext_pa, sizeof(rfseq_tx2rx_events_rev3)/
			sizeof(rfseq_tx2rx_events_rev3[0]));
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	tone_jammer_war_blk = wlapi_bmac_read_shm(pi->sh->physhim, M_AFEOVR_PTR(pi));
	FOREACH_CORE(pi, core) {
		wlapi_bmac_write_shm(pi->sh->physhim, (tone_jammer_war_blk +  2 * core + 0) * 2,
			NPHY_AfectrlOverrideX(core));
		wlapi_bmac_write_shm(pi->sh->physhim, (tone_jammer_war_blk +  2 * core + 1) * 2,
			NPHY_AfectrlCoreX(core));
	}

	wlapi_bmac_mhf(pi->sh->physhim, MHF5, MHF5_TONEJAMMER_WAR,
		MHF5_TONEJAMMER_WAR, WLC_BAND_ALL);

	if ((CHIPID((pi)->sh->chip) == BCM43217_CHIP_ID) && (pi_nphy->ed_assert_thresh_dbm < 0))
		wlc_phy_adjust_ed_thres(pi,	&pi_nphy->ed_assert_thresh_dbm, TRUE);

	if (region_group == REGION_EU)
		wlc_phy_set_srom_eu_edthresh_nphy(pi);
}

void
wlc_phy_workarounds_nphy_gainctrl(phy_info_t *pi)
{
	if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV + 1)) {
		if (RADIOREV(pi->pubpi->radiorev) == 13) {
		/* 53572a0 */
		wlc_phy_workarounds_nphy_gainctrl_2057_rev13(pi);
		} else {
			/* 43217 and others */
			if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_ELNA_GAINDEF) &&
			    ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) ||
			     (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz))) {
				wlc_phy_backoff_initgain_elna(pi);
				wlc_phy_elna_gainctrl_workaround(pi);
			}
			wlc_phy_workarounds_nphy_gainctrl_2057_rev14(pi);
		}
	} else {
		if (CHIPID_43236X_FAMILY(pi)) {
			/* 43236, 43235, 43234, 43238 (radio rev 7)
			 * For Sulley: see wlc_phy_subband_cust_43236B1_sulley_nphy()
			 */
			if ((pi->sh->boardtype != BCM943236OLYMPICSULLEY_SSID) &&
			    (pi->sh->boardtype != BCM943236PREPROTOBLU2O3_SSID) &&
			    (pi->fem2g->antswctrllut != 11) &&
			    (pi->fem5g->antswctrllut != 11) &&
			    (pi->fem2g->antswctrllut != 15) &&
			    (pi->fem5g->antswctrllut != 15))
				wlc_phy_workarounds_nphy_gainctrl_2057_rev7(pi);

		} else if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
				/* 6362/BCM63268 */
				wlc_phy_workarounds_nphy_gainctrl_2057_rev3(pi);
		} else {
			/* 43226A0, A1 */
			wlc_phy_workarounds_nphy_gainctrl_2057_rev6(pi);
		}

		if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_ELNA_GAINDEF) &&
			((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) ||
			(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA_5GHz))) {
				wlc_phy_backoff_initgain_elna(pi);
				wlc_phy_elna_gainctrl_workaround(pi);
		}

	}
}

void
wlc_phy_init_hw_antsel(phy_info_t *pi)
{

	/* skip if this board doesn't support HW Rx antsel */
	if ((pi->antsel_type != ANTSEL_2x3_HWRX) && (pi->antsel_type != ANTSEL_1x2_HWRX))
		return;

	if (pi->nphy_enable_hw_antsel) {

		/* Enable */
		MOD_PHYREG3(pi, NPHY, AntSelConfig2057, board_switch_div0, (uint16)0x1);
		MOD_PHYREG3(pi, NPHY, AntSelConfig2057, board_switch_div1, (uint16)0x1);
		MOD_PHYREG3(pi, NPHY, AntSelConfig2057, CoreStartAntPos0, (uint16)0x0);
		MOD_PHYREG3(pi, NPHY, AntSelConfig2057, CoreStartAntPos1, (uint16)0x0);
		MOD_PHYREG3(pi, NPHY, CoreConfig, NumRxAnt, (uint16)0x3);

		/* Disable phyreg override to make sure PHY is in charge */
		MOD_PHYREG3(pi, NPHY, AntSelConfig2057, RxAntSel_SrcSelect, (uint16)0x0);

		/* Setting for OFDM */
		MOD_PHYREG3(pi, NPHY, AntennaDivBackOffGain, MinDivSearchGain, (uint16)10);
		MOD_PHYREG3(pi, NPHY, AntennaDivBackOffGain, BackoffGain, (uint16)10);
		WRITE_PHY_REG3(pi, NPHY, DivGainThreshold_OFDM, 84);
		/* Setting for CCK */
		MOD_PHYREG3(pi, NPHY, AntennaDivMinGain, cckMinDivSearchGain, (uint16)0x32);
		MOD_PHYREG3(pi, NPHY, AntennaDivMinGain, cckBackoffGain, (uint16)0xf);
		WRITE_PHY_REG3(pi, NPHY, DivGainThreshold_BPHY, 97);
		/* Settling Time */
		WRITE_PHY_REG3(pi, NPHY, clip1gainSettleLen, 68);
		WRITE_PHY_REG3(pi, NPHY, clip2gainSettleLen, 64);
		WRITE_PHY_REG3(pi, NPHY, pktgainSettleLen, 56);
		WRITE_PHY_REG3(pi, NPHY, initgainSettleLen, 64);
		WRITE_PHY_REG3(pi, NPHY, dssscckgainSettleLen, 91);

	} else {

		/* disable HW antsel */
		MOD_PHYREG3(pi, NPHY, AntSelConfig2057, board_switch_div0, (uint16)0x0);
		MOD_PHYREG3(pi, NPHY, AntSelConfig2057, board_switch_div1, (uint16)0x0);
	}
}

void
wlc_phy_adjust_ClipLO_for_triso_2057_rev5_rev9(phy_info_t *pi)
{
	uint8 triso;
	/* LO Gain (Rfseq Format) */
	triso = (CHSPEC_IS5G(pi->radio_chanspec)) ? pi->fem5g->triso : pi->fem2g->triso;
	switch (triso) {
		case 0:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x68)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x68)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x8)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x8)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x1202)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		case 1:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x68)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x68)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x18)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x18)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x1502)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		case 2:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x18)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x18)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x1902)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		case 3:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x118)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x118)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x1c02)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		case 4:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x218)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x218)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x1f02)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		case 5:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x318)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x318)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x2202)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		case 6:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x418)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x418)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x2502)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		case 7:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x518)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x518)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x2802)
			PHY_REG_LIST_EXECUTE(pi);
			break;
		default:
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x70)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x18)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x18)
				PHY_REG_WRITE_ENTRY(NPHY, TRLossValue, 0x1902)
			PHY_REG_LIST_EXECUTE(pi);
			break;
	}
}

void
wlc_phy_workarounds_nphy_gainctrl_2057_rev5(phy_info_t *pi)
{
	int8  lna1_gain_db[] = {11, 16, 20, 25};
	int8  lna2_gain_db[] = {0, 9, 13, 17};
	int8  tia_gain_db[]  = {-4, -1, 2, 5, 5, 5, 5, 5, 5, 5};
	int8  tia_gainbits[] = {0x0, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};

	PHY_REG_LIST_START
		/* Disable clip2 detect until it's properly characterized */
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		/* set Fine Timing Thresholds */
		PHY_REG_MOD_ENTRY(NPHY, FSTRHiPwrTh, finestr_hiPwr_th, 65)
		/* set bphy crsminpower to 70(dec) for 20Mhz and 40Mhz */
		PHY_REG_MOD_ENTRY(NPHY, bphycrsminpower0, bphycrsminpower0, 0x46)
		/* set crsminpwr */
		PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x42)
		PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x42)
	PHY_REG_LIST_EXECUTE(pi);

	/* LNA1 Gainstep */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1_gain_db);

	/* LNA2 Gainstep */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2_gain_db);

	/* TIA Gain */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, tia_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, tia_gain_db);

	/* TIA Gainbits */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8,
	                         tia_gainbits);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8,
	                         tia_gainbits);

	/* Moved trisolation setting into a function */
	/* Need to be reused for LCNXNPHY */
	wlc_phy_adjust_ClipLO_for_triso_2057_rev5_rev9(pi);

	PHY_REG_LIST_START
		/* NB Clip = 0xe8 */
		PHY_REG_WRITE_ENTRY(NPHY, Core1nbClipThreshold, 0xe8)
		PHY_REG_WRITE_ENTRY(NPHY, Core2nbClipThreshold, 0xe8)

		/* w1 clip */
		PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057, clip1wbThreshold, 0x16)
		PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057, clip1wbThreshold, 0x16)
	PHY_REG_LIST_EXECUTE(pi);

}

/*
 * AUTOGENERATED from subband_cust_43236A0.xls
 * DO NOT EDIT
 */

void
wlc_phy_workarounds_nphy_gainctrl_2057_rev6(phy_info_t *pi)
{
	uint16 currband;
	int8  lna1G_gain_db_rev7[] = {9, 14, 19, 24};
	int8 *lna1_gain_db = NULL;
	int8 *lna1_gain_db_2 = NULL;
	int8 *lna2_gain_db = NULL;
	int8 *tia_gain_db;
	int8 *tia_gainbits;
	uint16 *rfseq_init_gain;
	uint16 init_gaincode;
	uint16 clip1hi_gaincode;
	uint16 clip1md_gaincode = 0;
	uint16 clip1md_gaincode_B; /* REV7+ */
	uint16 clip1lo_gaincode;
	uint16 clip1lo_gaincode_B; /* REV7+ */
	uint8  crsminl_th = 0;
	uint8  crsminu_th;
	uint16 nbclip_th = 0;
	uint8  w1clip_th;
	uint16 freq;
	int8 nvar_baseline_offset0 = 0, nvar_baseline_offset1 = 0;
	uint8 chg_nbclip_th = 0;

	/* Disable clip2 detect until it's properly characterized */
	PHY_REG_LIST_START
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
	PHY_REG_LIST_EXECUTE(pi);

	/* Band-specific configuration */
	currband = phy_utils_read_phyreg(pi, NPHY_BandControl) & NPHY_BandControl_currentBand;
	if (currband == 0) {
		/* 2G-band */

		lna1_gain_db = lna1G_gain_db_rev7;

		/* Update gain tables with measured radio-gains */
		/* LNA1 Gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 8, 8, lna1_gain_db);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 8, 8, lna1_gain_db);

		/* increase crs-min-power by ~ 3 dBs for 20MHz */
		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		            (0x40 << NPHY_crsminpoweru0_crsminpower0_SHIFT));

		/* increase crs-min-power for 40MHz */
		if (CHSPEC_IS40(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x3e)
				PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x3e)
			PHY_REG_LIST_EXECUTE(pi);
		}

		/* set bphy crsminpower to 70(dec) for 20Mhz and 40Mhz */
		phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
		            NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
		            (0x46 << NPHY_bphycrsminpower0_bphycrsminpower0_SHIFT));

		/* Adjust W1Clip Threshold for robustness */
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
					clip1wbThreshold, 13)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
					clip1wbThreshold, 13)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else {
		/* 5G-band */
		int8 lna1A_gain_db_rev7[3][4] = {{11, 16, 20, 24},	/* Low-Gain band */
						{11, 17, 21, 25},	/* High-Gain band */
						{12, 18, 22, 26}};	/* Mid-Gain band */
		int8 lna1A_gain_db_2_rev7[3][4] = {{11, 17, 22, 25},	/* Low-Gain band */
						{12, 18, 22, 26},	/* High-Gain band */
						{12, 18, 22, 26}};	/* Mid-Gain band */
		int8 lna2A_gain_db_rev7[3][4] = {{-1, 6, 10, 14},	/* Low-Gain band */
						{1, 8, 12, 16},		/* High-Gain band */
						{-1, 6, 10, 14}};	/* Mid-Gain band */
		uint16 rfseqA_init_gain_rev7[] = {0x624f, 0x624f};
		int8  tiaA_gain_db_rev7[]  = {-9, -6, -3, 0, 3, 3, 3, 3, 3, 3};
		int8  tiaA_gainbits_rev7[] = {0,  1,  2, 3, 4, 4, 4, 4, 4, 4};

		/* common settings */
		init_gaincode = 0x9e;
		clip1hi_gaincode = 0x9e;
		clip1md_gaincode_B = 0x24;
		clip1lo_gaincode = 0x8a;
		clip1lo_gaincode_B = 8;
		rfseq_init_gain = rfseqA_init_gain_rev7;

		/* implement limiting mixer gain instead of fixing mixer gain */
		tia_gain_db = tiaA_gain_db_rev7;
		tia_gainbits = tiaA_gainbits_rev7;

		freq = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			/* 5G BW20 */
			w1clip_th = 25;
			clip1md_gaincode = 0x82;

			if ((freq <= 5080) || (freq == 5825)) {
				/* Low-Gain band */
				crsminu_th = 0x3e;
				lna1_gain_db = lna1A_gain_db_rev7[0];
				lna1_gain_db_2 = lna1A_gain_db_2_rev7[0];
				lna2_gain_db = lna2A_gain_db_rev7[0];
			} else if ((freq >= 5500) && (freq <= 5700)) {
				/* High-Gain band */
				crsminu_th = 0x45;
				clip1md_gaincode_B = 0x14;
				nbclip_th = 0xff;
				chg_nbclip_th = 1;
				lna1_gain_db = lna1A_gain_db_rev7[1];
				lna1_gain_db_2 = lna1A_gain_db_2_rev7[1];
				lna2_gain_db = lna2A_gain_db_rev7[1];
			} else {
				/* Mid-Gain band */
				crsminu_th = 0x41;
				lna1_gain_db = lna1A_gain_db_rev7[2];
				lna1_gain_db_2 = lna1A_gain_db_2_rev7[2];
				lna2_gain_db = lna2A_gain_db_rev7[2];
			}

			/* adjust NF */
			if (freq <= 4920) {
				nvar_baseline_offset0 = 5;
				nvar_baseline_offset1 = 5;
			} else if ((freq > 4920) && (freq <= 5320)) {
				nvar_baseline_offset0 = 3;
				nvar_baseline_offset1 = 5;
			} else if ((freq > 5320) && (freq <= 5700)) {
				nvar_baseline_offset0 = 3;
				nvar_baseline_offset1 = 2;
			} else {
				nvar_baseline_offset0 = 4;
				nvar_baseline_offset1 = 0;
			}
		} else {
			/* 5G BW40 */
			crsminu_th = 0x3a;
			crsminl_th = 0x3a;
			w1clip_th = 20;

			/* adjust NF */
			if ((freq >= 4920) && (freq <= 5320)) {
				nvar_baseline_offset0 = 4;
				nvar_baseline_offset1 = 5;
			} else if ((freq > 5320) && (freq <= 5550)) {
				nvar_baseline_offset0 = 4;
				nvar_baseline_offset1 = 2;
			} else {
				nvar_baseline_offset0 = 5;
				nvar_baseline_offset1 = 3;
			}
		}

		/* Init Gain */
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, init_gaincode);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, init_gaincode);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, pi->pubpi->phy_corenum,
		                         0x106, 16, rfseq_init_gain);

		/* HI Gain */
		/* Decrease tia/mix gain to reduce clipping of high pwr ACI before lpf */
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, clip1hi_gaincode);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, clip1hi_gaincode);

		/* MD Gain */
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clip1md_gaincode_B);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clip1md_gaincode_B);

		/* LO Gain */
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, clip1lo_gaincode);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, clip1lo_gaincode);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, clip1lo_gaincode_B);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, clip1lo_gaincode_B);

		/* TIA Gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, tia_gain_db);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, tia_gain_db);

		/* TIA Gainbits */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8,
		                         tia_gainbits);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8,
		                         tia_gainbits);

		/* adjust crs min power */
		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		            (crsminu_th << NPHY_crsminpoweru0_crsminpower0_SHIFT));

		/* RSSI Settings
		 * Adjust NB Clip threshold based on gainRange curves
		 */
		if (chg_nbclip_th == 1) {
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, nbclip_th);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, nbclip_th);
		}

		/* Adjust W1 Clip threshold based on gainRange curves */
		phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
		            NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
		            (w1clip_th <<
		             NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
		            NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
		            (w1clip_th <<
		             NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));

		/* Adjust baseline offsets for noise variance */
		phy_utils_mod_phyreg(pi, NPHY_gainAdjNoiseVarOffset,
		            NPHY_gainAdjNoiseVarOffset_gainAdjOffset0_MASK,
		            (nvar_baseline_offset0 <<
		             NPHY_gainAdjNoiseVarOffset_gainAdjOffset0_SHIFT));

		phy_utils_mod_phyreg(pi, NPHY_gainAdjNoiseVarOffset,
		            NPHY_gainAdjNoiseVarOffset_gainAdjOffset1_MASK,
		            (nvar_baseline_offset1 <<
		             NPHY_gainAdjNoiseVarOffset_gainAdjOffset1_SHIFT));

		if (CHSPEC_IS20(pi->radio_chanspec)) {
			/* Update gain tables with measured radio-gains */
			/* LNA1 Gain */
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 8, 8,
			                         lna1_gain_db);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 8, 8,
			                         lna1_gain_db_2);

			/* LNA2 Gain */
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8,
			                         lna2_gain_db);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8,
			                         lna2_gain_db);

			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clip1md_gaincode);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clip1md_gaincode);
		} else {
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			            NPHY_crsminpowerl0_crsminpower0_MASK,
			            (crsminl_th << NPHY_crsminpowerl0_crsminpower0_SHIFT));
		}

	}

}

void
wlc_phy_workarounds_nphy_gainctrl_2057_rev7(phy_info_t *pi)
{
	int32 edcrs_th;
	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);

	/* Disable clip2 detect until it's properly characterized */
	PHY_REG_LIST_START
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
	PHY_REG_LIST_EXECUTE(pi);

	/* Reduce edcrs for EU by 3dBs */
	if (region_group == REGION_EU) {
		/* Need to pass -70dBm/Mhz (2g), -73dBm/Mhz (5g) inband noise */
		edcrs_th = (CHSPEC_IS2G(pi->radio_chanspec)) ? -64 : -67;
		wlc_phy_adjust_ed_thres(pi, &edcrs_th, TRUE);
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* 2G-band */

		const int8 lna1gain[] = {10, 15, 19, 25};
		/* LNA2 Gains [dB] */
		const int8 lna2gain[] = {0, 10, 15, 18};
		/* BPHY CRS MinPwr */
		uint16 bcrsmin  = 0x46;
		uint16 crsminu = 0x36, crsminl = 0x36;

		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {
			/* BW20 customizations */

			/* CRS MinPwr */
			crsminu  = 0x4a;
			crsminl  = 0x4a;

			/* adjust W1Clip Threshold */
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
					clip1wbThreshold, 0xd)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
					clip1wbThreshold, 0xd)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			/* BW40 customizations */

			/* CRS MinPwr */
			crsminu  = 0x48;
			crsminl  = 0x48;

			/* adjust W1Clip Threshold */
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
					clip1wbThreshold, 0x13)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
					clip1wbThreshold, 0x13)
			PHY_REG_LIST_EXECUTE(pi);
		}

		/* adjust crsminpower */
		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		            (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0, NPHY_crsminpowerl0_crsminpower0_MASK,
		            (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));

		phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
		            NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
		            (bcrsmin << NPHY_bphycrsminpower0_bphycrsminpower0_SHIFT));

		/* Update gain tables with measured radio-gains */
		/* LNA1 Gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain);
		/* LNA2 Gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain);

	} else {
		/* 5G-band */

		/* InitGain */
		const uint16 initgain[] = {0x624f, 0x624f, 0x624f, 0x624f};
		/* LNA1 Gains [dB] */
		const int8  lna1gain[] = {11, 17, 21, 26};
		/* LNA2 Gains [dB] */
		const int8   lna2gain[] = {0, 7, 11, 15};
		/* MixTIA Gains */
		const int8   mixtia[] = {-5, -2, 1, 4, 7, 7, 7, 7, 7, 7};
		/* GainBits */
		const int8   gainbits[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};
		/* Radio REGs */
		const uint8  radioreg_0x86 = 0xc0;
		const uint8  radioreg_0x10B = 0xc0;

		/* 47186nrh has 43236 with eLNA */
		bool   elna_board = ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
			BFL_ELNA_GAINDEF) && ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
			BFL_EXTLNA) || (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
			BFL_EXTLNA_5GHz)));

		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {
			/* BW20 customizations */

			/* CRS MinPwr */
			uint16 crsminu = elna_board ? 0x3e : 0x44;
			uint16 crsminl = elna_board ? 0x3e : 0x44;

			/* adjust crsminpower */
			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			                     NPHY_crsminpoweru0_crsminpower0_MASK,
			                     (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			                     NPHY_crsminpowerl0_crsminpower0_MASK,
			                     (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));

			/* adjust W1Clip Threshold */
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
					clip1wbThreshold, 0x19)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
					clip1wbThreshold, 0x19)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			/* BW40 customizations */

			/* CRS MinPwr */
			uint16 crsminu  = elna_board ? 0x3e : 0x3f;
			uint16 crsminl  = elna_board ? 0x3e : 0x3f;

			/* adjust crsminpower */
			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			                     NPHY_crsminpoweru0_crsminpower0_MASK,
			                     (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			                     NPHY_crsminpowerl0_crsminpower0_MASK,
			                     (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));

			/* adjust W1Clip Threshold */
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
					clip1wbThreshold, 0x14)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
					clip1wbThreshold, 0x14)
			PHY_REG_LIST_EXECUTE(pi);
		}

		PHY_REG_LIST_START
			/* NB Clip  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1nbClipThreshold, 0xfe)
			PHY_REG_WRITE_ENTRY(NPHY, Core2nbClipThreshold, 0xfe)
			/* Hi Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipHiGainCodeA2057, 0x9e)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipHiGainCodeA2057, 0x9e)
			/* MD Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeB2057, 0x24)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeB2057, 0x24)
			/* LO Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x8a)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x8a)
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x8)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x8)
			/* InitGCode */
			PHY_REG_WRITE_ENTRY(NPHY, Core1InitGainCodeA2057, 0x9e)
			PHY_REG_WRITE_ENTRY(NPHY, Core2InitGainCodeA2057, 0x9e)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 10, 0x106, 16, initgain);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 32, 8, mixtia);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 32, 8, mixtia);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits);

		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE0,
		                         radioreg_0x86);
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE1,
		                         radioreg_0x10B);

		return;
	}
}

void
wlc_phy_workarounds_nphy_gainctrl_2057_rev3(phy_info_t *pi)
{
	uint16 freq = 0;
	freq = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));

	/* Disable clip2 detect until it's properly characterized */
	PHY_REG_LIST_START
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_LIST_START
		/* set bphy crsminpower to 70(dec) for 20Mhz and 40Mhz */
		PHY_REG_MOD_ENTRY(NPHY, bphycrsminpower0, bphycrsminpower0, 0x46)
		/* adjust crsminpower */
		PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x44)
		PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x44)
		PHY_REG_LIST_EXECUTE(pi);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* 2G-band */

		int8 lna1gain[] = {8, 13, 18, 25};
		/* LNA2 Gains [dB] */
		int8 lna2gain[] = {-4, 6, 10, 15};

		/* MixTIA Gains */
		int8   mixtia[] = {-1, 0, 3, 6, 6, 6, 6, 6, 6, 6};
		/* GainBits */
		int8   gainbits[] = {0x0, 0x1, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};

		/* Update gain tables with measured radio-gains */
		/* LNA1 Gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain);
		/* LNA2 Gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain);
		/* Mix-TIA Gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 32, 8, mixtia);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 32, 8, mixtia);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits);

		/* clipLO gain */
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x74)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x74)
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x18)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x18)
		PHY_REG_LIST_EXECUTE(pi);

		/* W1 Clipx18 */
		PHY_REG_LIST_START
			/* adjust W1Clip Threshold */
			PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057, clip1wbThreshold, 0x18)
			PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057, clip1wbThreshold, 0x18)
		PHY_REG_LIST_EXECUTE(pi);

	} else {
		/* 5G-band */

		int8 lna1gain[]   = {8, 15, 19, 23};
		int8 lna1gain_1[] = {10, 17, 21, 25};

		/* LNA2 Gains [dB] */
		int8   lna2gain[] = {1, 7, 11, 16};
		/* MixTIA Gains */
		int8   mixtia[] = {-7, -4, -1, 2, 5, 5, 5, 5, 5, 5};
		/* GainBits */
		int8   gainbits[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};

		/* InitGain */
		uint16 initgain[] = {0x624f, 0x624f, 0x624f, 0x624f};

		/* Radio REGs */
		uint8  radioreg_0x86 = 0xc0;
		uint8  radioreg_0x10B = 0xc0;

		if (freq <= 4960) {
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain);
		} else {
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain_1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain);
		}
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain);

		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE0,
		                         radioreg_0x86);
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE1,
		                         radioreg_0x10B);

		PHY_REG_LIST_START
			/* InitGCode */
			PHY_REG_WRITE_ENTRY(NPHY, Core1InitGainCodeA2057, 0x9e)
			PHY_REG_WRITE_ENTRY(NPHY, Core2InitGainCodeA2057, 0x9e)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 10, 0x106, 16, initgain);

		PHY_REG_LIST_START
			/* Hi Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipHiGainCodeA2057, 0x9e)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipHiGainCodeA2057, 0x9e)
			/* MD Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeA2057, 0x82)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeA2057, 0x82)
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeB2057, 0x24)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeB2057, 0x24)
		PHY_REG_LIST_EXECUTE(pi);

		/* Adjust Lo / W1Clip */
		if (freq <= 5060) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x9e)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x9e)
				PHY_REG_LIST_EXECUTE(pi);

			/* Adjust W1Clipx12 */
			PHY_REG_LIST_START
				/* adjust W1Clip Threshold */
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
				                  clip1wbThreshold, 0x12)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
				                  clip1wbThreshold, 0x12)
				PHY_REG_LIST_EXECUTE(pi);

		} else if ((freq > 5060) && (freq <= 5320)) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x94)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x94)
				PHY_REG_LIST_EXECUTE(pi);

			/* Adjust W1Clipx12 */
			PHY_REG_LIST_START
				/* adjust W1Clip Threshold */
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
				                  clip1wbThreshold, 0x12)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
				                  clip1wbThreshold, 0x12)
				PHY_REG_LIST_EXECUTE(pi);
		} else {

			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x8a)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x8a)
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x8)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x8)
				PHY_REG_LIST_EXECUTE(pi);

			/* Adjust W1Clipx19 */
			PHY_REG_LIST_START
				/* adjust W1Clip Threshold */
				PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
				                  clip1wbThreshold, 0x19)
				PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
				                  clip1wbThreshold, 0x19)
				PHY_REG_LIST_EXECUTE(pi);
		}

		/* Adjust Limit MixerTia  */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 32, 8, mixtia);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 32, 8, mixtia);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits);
	}
}

/** get the complex freq for nphy. if chan==0, use default radio channel */
uint8
wlc_phy_get_chan_freq_range_nphy(phy_info_t *pi, uint channel)
{
	int freq;
	chan_info_nphy_radio2057_t      *t0 = NULL;
	chan_info_nphy_radio205x_t      *t1 = NULL;
	chan_info_nphy_radio2057_rev5_t *t2 = NULL;

	if (NORADIO_ENAB(pi->pubpi))
		return WL_CHAN_FREQ_RANGE_2G;

	if (channel == 0)
		channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (!wlc_phy_chan2freq_nphy(pi, channel, &freq, &t0, &t1, &t2)) {
		PHY_ERROR(("wlc_phy_get_chan_freq_range_nphy: channel invalid\n"));
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		return WL_CHAN_FREQ_RANGE_2G;
	}
	if ((pi->sromi->subband5Gver == PHY_SUBBAND_3BAND_EMBDDED) ||
	           ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) &&
	            (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID))) {
		if ((freq >= EMBEDDED_LOW_5G_CHAN) && (freq < EMBEDDED_MID_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5G_BAND0;
		} else if ((freq >= EMBEDDED_MID_5G_CHAN) &&
		           (freq < EMBEDDED_HIGH_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5G_BAND1;
		} else {
			return WL_CHAN_FREQ_RANGE_5G_BAND2;
		}
	} else if (pi->sromi->subband5Gver == PHY_SUBBAND_3BAND_HIGHPWR) {
		if ((freq >= HIGHPWR_LOW_5G_CHAN) && (freq < HIGHPWR_MID_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5G_BAND0;
		} else if ((freq >= HIGHPWR_MID_5G_CHAN) && (freq < HIGHPWR_HIGH_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5G_BAND1;
		} else {
			return WL_CHAN_FREQ_RANGE_5G_BAND2;
		}
	} else if ((freq >= JAPAN_LOW_5G_CHAN) && (freq < JAPAN_MID_5G_CHAN)) {
		return WL_CHAN_FREQ_RANGE_5G_BAND0;
	} else if ((freq >= JAPAN_MID_5G_CHAN) && (freq < JAPAN_HIGH_5G_CHAN)) {
		return WL_CHAN_FREQ_RANGE_5G_BAND1;
	} else {
		return WL_CHAN_FREQ_RANGE_5G_BAND2;
	}
}

static void
wlc_phy_adjust_min_noisevar_nphy(phy_info_t *pi, int ntones, int *tone_id_buf, uint32
	*noise_var_buf)
{
	int i;
	uint32 offset;
	int tone_id;
	int tbllen =
	    CHSPEC_IS40(pi->radio_chanspec)? NPHY_NOISEVAR_TBLLEN40 : NPHY_NOISEVAR_TBLLEN20;

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		PHY_ERROR(("%s is not supported for REV7+, may clash with adjust_base_noisevar\n",
		           __FUNCTION__));
		ASSERT(0);
	}

	/* If the min_noise_vars have been adjusted previously (check pi->nphy_noisevars_adjusted),
	 * then reset the min_noise_vars to the default values (stored in
	 * pi_nphy->nphy_saved_noisevars) before adjusting the min_noise_vars to different values.
	 */
	if (pi_nphy->nphy_noisevars_adjusted) {
		for (i = 0; i < pi_nphy->nphy_saved_noisevars.bufcount; i++) {
			tone_id = pi_nphy->nphy_saved_noisevars.tone_id[i];
			offset = (tone_id >= 0)?
			    ((tone_id * 2) + 1) :
			    (tbllen + (tone_id * 2) + 1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			    offset, 32,
			       (void *)&pi_nphy->nphy_saved_noisevars.min_noise_vars[i]);
		}

		pi_nphy->nphy_saved_noisevars.bufcount = 0;
		pi_nphy->nphy_noisevars_adjusted = FALSE;
	}

	/* Change the min_noise_vars on the specified tones. However, store the current values
	 * of min_noise_vars on those tones before changing them.
	 */
	if ((noise_var_buf != NULL) && (tone_id_buf != NULL)) {
		pi_nphy->nphy_saved_noisevars.bufcount = 0;

		for (i = 0; i < ntones; i++) {
			tone_id = tone_id_buf[i];
			offset = (tone_id >= 0)?
			    ((tone_id * 2) + 1) :
			    (tbllen + (tone_id * 2) + 1);
			pi_nphy->nphy_saved_noisevars.tone_id[i] = tone_id;
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			    offset, 32,
			        &pi_nphy->nphy_saved_noisevars.min_noise_vars[i]);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			    offset, 32, (void *) &noise_var_buf[i]);
			pi_nphy->nphy_saved_noisevars.bufcount++;
		}

		pi_nphy->nphy_noisevars_adjusted = TRUE;
	}
}

/** adjust_base_noisevar, adapted from adjust_min_noisevar */
static void
wlc_phy_adjust_base_noisevar_nphy(phy_info_t *pi, int ntones, int *tone_id_buf, uint32
	*base_nvar_buf)
{
	int i;
	uint32 offset;
	uint32 temp_nvar_buf[4];
	uint32 orig_min_nvar;
	uint32 temp_debug;
	int tone_id;
	int tbllen =
	    CHSPEC_IS40(pi->radio_chanspec)? NPHY_NOISEVAR_TBLLEN40 : NPHY_NOISEVAR_TBLLEN20;

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	PHY_INFORM(("wl%d: %s: nphy_base_nvars_adjusted = %d\n", pi->sh->unit, __FUNCTION__,
	            pi_nphy->nphy_base_nvars_adjusted));

	if (pi_nphy->nphy_base_nvars_adjusted) {
		PHY_INFORM(("++++++++++++++++++++++++++++++++++++++++++\n"));
		PHY_INFORM(("       reset base_noise_vars to default values\n"));
		for (i = 0; i < pi_nphy->nphy_saved_noisevars.bufcount; i++) {
			tone_id = pi_nphy->nphy_saved_noisevars.tone_id[i];
			offset = (tone_id >= 0)?
			        (tone_id * 2) :
			        (tbllen + (tone_id * 2));
			PHY_INFORM(("       tone = %d  offset = %d\n", tone_id, offset));

			/* read off both base and min noise vars to save them */
			temp_nvar_buf[0] = pi_nphy->nphy_saved_noisevars.noise_vars[i];
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			                        offset + 1, 32, &temp_nvar_buf[1]);
			PHY_INFORM(("       stored base_nvar   = 0x%x\n", temp_nvar_buf[0]));
			PHY_INFORM(("       retrieved min_nvar = 0x%x\n", temp_nvar_buf[1]));

			temp_nvar_buf[0] = 0x20c020c;

			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 2,
			                         offset, 32, &temp_nvar_buf);

			/* temporary debug */
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			                        offset, 32, &temp_debug);
			PHY_INFORM(("       restored base_nvar = 0x%x\n", temp_debug));
			PHY_INFORM(("++++++++++++++++++++++++++++++++++++++++++\n\n"));
		}

		pi_nphy->nphy_saved_noisevars.bufcount = 0;
		pi_nphy->nphy_base_nvars_adjusted = FALSE;
	}

	/* Change the base_noise_vars on the specified tones. However, store the current values
	 * of base_noise_vars on those tones before changing them.
	 */
	if ((base_nvar_buf != NULL) && (tone_id_buf != NULL)) {
		pi_nphy->nphy_saved_noisevars.bufcount = 0;

		for (i = 0; i < ntones; i++) {

			tone_id = tone_id_buf[i];
			PHY_INFORM(("\n------------------------------------------\n"));
			PHY_INFORM(("       tone = %d ", tone_id));
			offset = (tone_id >= 0)?
			        (tone_id * 2) :
			        (tbllen + (tone_id * 2));
			PHY_INFORM((" offset = %d\n", offset));
			pi_nphy->nphy_saved_noisevars.tone_id[i] = tone_id;

			/* we need the current min_nvar value to write the new base_nvar */
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			    offset + 1, 32, &orig_min_nvar);

			/* read the current base_nvar in order to save it for future restore
			 * upon channel change.
			 */
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			    offset, 32, &pi_nphy->nphy_saved_noisevars.noise_vars[i]);
			PHY_INFORM(("       saved base_nvar = 0x%x\n",
			            pi_nphy->nphy_saved_noisevars.noise_vars[i]));

			/* write: need both the [base,min]_noise_var */
			temp_nvar_buf[0] = base_nvar_buf[i];
			temp_nvar_buf[1] = orig_min_nvar;
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 2,
			    offset, 32, &temp_nvar_buf);
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1,
			    offset, 32, &temp_debug);
			PHY_INFORM(("       written_base_nvar = 0x%x\n", temp_debug));
			PHY_INFORM(("------------------------------------------\n"));

			pi_nphy->nphy_saved_noisevars.bufcount++;
		}

		pi_nphy->nphy_base_nvars_adjusted = TRUE;
	}
}

static void
wlc_phy_spurwar_nphy(phy_info_t *pi)
{
	uint16 cur_channel = 0;
	int nphy_adj_tone_id_buf[] =  {57, 58};
	int nphy_adj_tone_id_buf2[] = {57, 58, 59, 60};
	uint32 nphy_adj_noise_var_buf[] =  {0x3ff, 0x3ff};
	uint32 nphy_adj_noise_var_buf2[] = {0x3ff, 0x3ff, 0x3ff, 0x3ff};
	bool isAdjustNoiseVar = FALSE;
	uint numTonesAdjust = 0;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	cur_channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	/* 43236B0+ only: enable BF triggered LLR deweighting to fight 2G spurs */
	if (((pi_nphy->nphy_gband_spurwar2_en) &&
	     ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) &&
	     (CHIPREV(pi->sh->chiprev) >= 2))) ||
	     (CHIPID(pi->sh->chip) == BCM43428_CHIP_ID))
	{

		/* Increase the base_noise_var on the tones that are affected by
		 * xtal and auxADC spurs (2420, 2440, 2460).
		 * Spur level above noise floor is essentially constant.
		 */

		/* We need to enable this code for both bands, so that noisevartbl is
		 * restored to default values when switching to 5G channels, where
		 * de-weighting is not needed yet, ie, no known spurs found.
		 */

		/* 2G BW20 channels see 2 spurs each */
		if (CHSPEC_IS40(pi->radio_chanspec) == 0) {

			/* Per-channel location of tones affected by spurs.
			 * Since spurs are 20MHz-periodic, we can group channels.
			 */
			PHY_INFORM(("************************* cur_channel = %d\n",
			            cur_channel));
			switch (cur_channel) {
			case 1:
			case 5:
			case 9:
			case 13:
				nphy_adj_tone_id_buf2[0] = 25;
				nphy_adj_tone_id_buf2[1] = 26;
				isAdjustNoiseVar = TRUE;
				break;
			case 2:
			case 6:
			case 10:
				nphy_adj_tone_id_buf2[0] = 9;
				nphy_adj_tone_id_buf2[1] = 10;
				isAdjustNoiseVar = TRUE;
				break;
			case 3:
			case 7:
			case 11:
				nphy_adj_tone_id_buf2[0] = -7;
				nphy_adj_tone_id_buf2[1] = -6;
				isAdjustNoiseVar = TRUE;
				break;
			case 4:
			case 8:
			case 12:
				nphy_adj_tone_id_buf2[0] = -23;
				nphy_adj_tone_id_buf2[1] = -22;
				isAdjustNoiseVar = TRUE;
				break;
			default:
				/* 5G channels fall here, so that we restore noisevartbl
				 * to default values when switching to 5G band.
				 */
				isAdjustNoiseVar = FALSE;
				break;
			}

			/* Absolute base_nvar values (for both cores):
			 *   - ch 5-8 are affected by multi-mode spur at 2440 on both cores,
			 *     in addition to xtal spur at 2440.
			 *   - other channels only have fixed xtal spur at 24[26]0 on core0
			 */
			if ((cur_channel >= 5) && (cur_channel <= 8)) {
				nphy_adj_noise_var_buf2[0] = 0x2570257;
				nphy_adj_noise_var_buf2[1] = 0x2770277;
			} else {
				nphy_adj_noise_var_buf2[0] = 0x20c0257;
				nphy_adj_noise_var_buf2[1] = 0x20c0277;
			}
			if ((CHIPID(pi->sh->chip) == BCM43428_CHIP_ID) && (cur_channel == 13)) {
				nphy_adj_noise_var_buf2[0] = 0x2570283;
				nphy_adj_noise_var_buf2[1] = 0x2570283;
			}
			if (isAdjustNoiseVar) {
				numTonesAdjust = 2;
				wlc_phy_adjust_base_noisevar_nphy(pi, numTonesAdjust,
				                                  nphy_adj_tone_id_buf2,
				                                  nphy_adj_noise_var_buf2);
			} else {
				/* Reset the base_noise_vars to their default values.
				 * Same reason as for restoring min_noise_vars.
				 */
				wlc_phy_adjust_base_noisevar_nphy(pi, 0, NULL, NULL);
			}

		} else {
			/* 2G BW40 channels see 4 spurs each */

			/* Per-channel location of tones affected by spurs.
			 * Since spurs are 20MHz-periodic, we can group channels.
			 */
			switch (cur_channel) {
			case 3:
			case 7:
			case 11:
				nphy_adj_tone_id_buf2[0] = -7;
				nphy_adj_tone_id_buf2[1] = -6;
				nphy_adj_tone_id_buf2[2] = 57;
				nphy_adj_tone_id_buf2[3] = 58;
				isAdjustNoiseVar = TRUE;
				break;
			case 4:
			case 8:
				nphy_adj_tone_id_buf2[0] = -23;
				nphy_adj_tone_id_buf2[1] = -22;
				nphy_adj_tone_id_buf2[2] = 41;
				nphy_adj_tone_id_buf2[3] = 42;
				isAdjustNoiseVar = TRUE;
				break;
			case 5:
			case 9:
				nphy_adj_tone_id_buf2[0] = -39;
				nphy_adj_tone_id_buf2[1] = -38;
				nphy_adj_tone_id_buf2[2] = 25;
				nphy_adj_tone_id_buf2[3] = 26;
				isAdjustNoiseVar = TRUE;
				break;
			case 6:
			case 10:
				nphy_adj_tone_id_buf2[0] = -55;
				nphy_adj_tone_id_buf2[1] = -54;
				nphy_adj_tone_id_buf2[2] = 9;
				nphy_adj_tone_id_buf2[3] = 10;
				isAdjustNoiseVar = TRUE;
				break;
			default:
				/* 5G channels fall here, so that we restore noisevartbl
				 * to default values when switching to 5G band.
				 */
				isAdjustNoiseVar = FALSE;
				break;
			}

			/* Absolute base_nvar values (for both cores) */
			nphy_adj_noise_var_buf2[0] = 0x20c0257;
			nphy_adj_noise_var_buf2[1] = 0x20c0277;
			nphy_adj_noise_var_buf2[2] = 0x20c0257;
			nphy_adj_noise_var_buf2[3] = 0x20c0277;

			if (isAdjustNoiseVar) {
				numTonesAdjust = 4;
				wlc_phy_adjust_base_noisevar_nphy(pi, numTonesAdjust,
				                                  nphy_adj_tone_id_buf2,
				                                  nphy_adj_noise_var_buf2);

			} else {
				/* Reset the base_noise_vars to their default values.
				 * Same reason as for restoring min_noise_vars.
				 */

				wlc_phy_adjust_base_noisevar_nphy(pi, 0, NULL, NULL);
			}
		}
	} else if (((CHIPID(pi->sh->chip) == BCM43131_CHIP_ID) &&
	            (RADIOVER(pi->pubpi->radiover) == 0)) ||
	           ((NREV_IS(pi->pubpi->phy_rev, (LCNXN_BASEREV + 1))) &&
	            (RADIOREV(pi->pubpi->radiorev) == 14) &&
	            (RADIOVER(pi->pubpi->radiover) == 0))) {
		if (CHSPEC_IS40(pi->radio_chanspec) == 0) {
			PHY_INFORM(("************************* cur_channel = %d\n", cur_channel));
			switch (cur_channel) {
			case 1:
			case 5:
			case 9:
				nphy_adj_tone_id_buf2[0] = 25;
				nphy_adj_tone_id_buf2[1] = 26;
				isAdjustNoiseVar = TRUE;
				break;
			case 2:
			case 6:
			case 10:
				nphy_adj_tone_id_buf2[0] = 9;
				nphy_adj_tone_id_buf2[1] = 10;
				isAdjustNoiseVar = TRUE;
				break;
			case 3:
			case 7:
			case 11:
				nphy_adj_tone_id_buf2[0] = -7;
				nphy_adj_tone_id_buf2[1] = -6;
				isAdjustNoiseVar = TRUE;
				break;
			case 4:
			case 8:
			case 12:
				nphy_adj_tone_id_buf2[0] = -23;
				nphy_adj_tone_id_buf2[1] = -22;
				isAdjustNoiseVar = TRUE;
				break;
			default:
				isAdjustNoiseVar = FALSE;
				break;
			}

			nphy_adj_noise_var_buf2[0] = 0x20c0241;
			nphy_adj_noise_var_buf2[1] = 0x20c0261;

			if (isAdjustNoiseVar) {
				numTonesAdjust = 2;
				wlc_phy_adjust_base_noisevar_nphy(pi, numTonesAdjust,
				                                  nphy_adj_tone_id_buf2,
				                                  nphy_adj_noise_var_buf2);

			} else {
				/* Reset the base_noise_vars to their default values.
				 * Same reason as for restoring min_noise_vars.
				 */
				wlc_phy_adjust_base_noisevar_nphy(pi, 0, NULL, NULL);
			}

		} else {
			switch (cur_channel) {
			case 3:
			case 7:
				nphy_adj_tone_id_buf2[0] = -7;
				nphy_adj_tone_id_buf2[1] = -6;
				nphy_adj_tone_id_buf2[2] = 57;
				nphy_adj_tone_id_buf2[3] = 58;
				isAdjustNoiseVar = TRUE;
				break;
			case 4:
			case 8:
				nphy_adj_tone_id_buf2[0] = -23;
				nphy_adj_tone_id_buf2[1] = -22;
				nphy_adj_tone_id_buf2[2] = 41;
				nphy_adj_tone_id_buf2[3] = 42;
				isAdjustNoiseVar = TRUE;
				break;
			case 5:
			case 9:
				nphy_adj_tone_id_buf2[0] = -39;
				nphy_adj_tone_id_buf2[1] = -38;
				nphy_adj_tone_id_buf2[2] = 25;
				nphy_adj_tone_id_buf2[3] = 26;
				isAdjustNoiseVar = TRUE;
				break;
			case 6:
			case 10:
				nphy_adj_tone_id_buf2[0] = -55;
				nphy_adj_tone_id_buf2[1] = -54;
				nphy_adj_tone_id_buf2[2] = 9;
				nphy_adj_tone_id_buf2[3] = 10;
				isAdjustNoiseVar = TRUE;
				break;
			case 11:
				nphy_adj_tone_id_buf2[0] = -7;
				nphy_adj_tone_id_buf2[1] = -6;
				isAdjustNoiseVar = TRUE;
				break;
			default:
				/* 5G channels fall here, so that we restore noisevartbl
				 * to default values when switching to 5G band.
				 */
				isAdjustNoiseVar = FALSE;
				break;
			}

			/* Absolute base_nvar values (for both cores) */
			if (cur_channel == 11) {
				nphy_adj_noise_var_buf2[0] = 0x20c0241;
				nphy_adj_noise_var_buf2[1] = 0x20c0261;
				numTonesAdjust = 2;
			} else {
				nphy_adj_noise_var_buf2[0] = 0x20c0241;
				nphy_adj_noise_var_buf2[1] = 0x20c0261;
				nphy_adj_noise_var_buf2[2] = 0x20c0241;
				nphy_adj_noise_var_buf2[3] = 0x20c0261;
				numTonesAdjust = 4;
			}
			if (isAdjustNoiseVar) {

				wlc_phy_adjust_base_noisevar_nphy(pi, numTonesAdjust,
				                                  nphy_adj_tone_id_buf2,
				                                  nphy_adj_noise_var_buf2);

			} else {
				/* Reset the base_noise_vars to their default values.
				 * Same reason as for restoring min_noise_vars.
				 */

				wlc_phy_adjust_base_noisevar_nphy(pi, 0, NULL, NULL);
			}
		}
	}

	if ((pi_nphy->nphy_aband_spurwar_en) && (CHSPEC_IS5G(pi->radio_chanspec))) {
		switch (cur_channel) {
		case 54:
			nphy_adj_tone_id_buf[0] = 32;
			nphy_adj_noise_var_buf[0] = 0x25f;
			break;
		case 38:
		case 102:
		case 118:
			nphy_adj_tone_id_buf[0] = 0;
			nphy_adj_noise_var_buf[0] = 0x0;
			break;
		case 134:
			nphy_adj_tone_id_buf[0] = 32;
			nphy_adj_noise_var_buf[0] = 0x21f;
			break;
		case 151:
			nphy_adj_tone_id_buf[0] = 16;
			nphy_adj_noise_var_buf[0] = 0x23f;
			break;
		case 153:
		case 161:
			nphy_adj_tone_id_buf[0] = 48;
			nphy_adj_noise_var_buf[0] = 0x23f;
			break;
		default:
			nphy_adj_tone_id_buf[0] = 0;
			nphy_adj_noise_var_buf[0] = 0x0;
			break;
		}

		if (nphy_adj_tone_id_buf[0] && nphy_adj_noise_var_buf[0]) {
			wlc_phy_adjust_min_noisevar_nphy(pi, 1,
			    nphy_adj_tone_id_buf, nphy_adj_noise_var_buf);
		} else {
			wlc_phy_adjust_min_noisevar_nphy(pi, 0, NULL, NULL);
		}
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}
/*
 * AUTOGENERATED from ../47xxdata/mimophy/subband_cust_43236B1_preproto_blu2o3.xls
 * DO NOT EDIT
 */
static void
wlc_phy_subband_cust_43236B1_preproto_blu2o3_nphy(phy_info_t *pi)
{
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {

		/* Default customizations */
		/* CRS MinPwr */
		uint16 bcrsmin  = 0x46;
		uint16 crsminu  = 0x40;
		uint16 crsminl  = 0x40;
		/* Fine Str */
		uint16 fineStrHiPwrTh = 0x5342;
		uint16 FineStrMetricTh = 0x0A20;
		/* Clip1 WB Thresh */
		uint16 clip1wb1 = 0x14;
		uint16 clip1wb2 = 0x14;
		/* NB Clip  */
		uint16 clipnb1  = 0xb4;
		uint16 clipnb2  = 0xb4;
		/* Hi Gain  */
		uint16 g_cliphiA1 = 0x6e;
		uint16 g_cliphiA2 = 0x6e;
		uint16 g_cliphiB1 = 0x14;
		uint16 g_cliphiB2 = 0x14;
		/* MD Gain  */
		uint16 clipmdA1 = 0x42;
		uint16 clipmdA2 = 0x42;
		uint16 clipmdB1 = 0x4;
		uint16 clipmdB2 = 0x4;
		/* LO Gain  */
		uint16 cliploA1 = 0x50;
		uint16 cliploA2 = 0x50;
		uint16 cliploB1 = 0x08;
		uint16 cliploB2 = 0x08;
		/* InitGCode */
		uint16 initGCA1 = 0x74;
		uint16 initGCA2 = 0x74;
		uint16 initGCB1 = 0x614;
		uint16 initGCB2 = 0x614;
		/* InitGain */
		uint16 initgain[] = {0x613a, 0x613a};
		/* eLNA Gains [dB] */
		int8   elnagain1[] = {15, 15};
		int8   elnagain2[] = {15, 15};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {10, 15, 20, 25};
		int8   lna1gain2[] = {10, 15, 20, 25};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 10, 15, 18};
		int8   lna2gain2[] = {0, 10, 15, 18};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* MixTIA Gains */
		int8   mixtia1[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		int8   mixtia2[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		uint8  gainbits2[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		/* Clip2 */
		uint8  clip2c1  = 1;
		uint8  clip2c2  = 1;

		phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
		                     NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
		                     (bcrsmin << NPHY_bphycrsminpower0_bphycrsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		            (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0, NPHY_crsminpowerl0_crsminpower0_MASK,
		            (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_FSTRHiPwrTh, fineStrHiPwrTh);
		phy_utils_write_phyreg(pi, NPHY_FSTRMetricTh, FineStrMetricTh);
		phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
		            NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
		            (clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
		            NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
		            (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
		phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, g_cliphiA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, g_cliphiA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, g_cliphiB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, g_cliphiB2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);
		phy_utils_mod_phyreg(pi, NPHY_Core1computeGainInfo,
		            NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
		            (clip2c1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2computeGainInfo,
		            NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
		            (clip2c2 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

			if ((fc >= 2412 && fc <= 2484)) {
			}

		/* BW40 customizations */
		} else {

			if ((fc >= 2422 && fc <= 2462)) {
			}
		}
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {

		/* Default customizations */
		/* CRS MinPwr */
		uint16 g_crsminu  = 0x42;
		uint16 g_crsminl  = 0x42;
		/* Fine Str */
		uint16 fineStrHiPwrTh = 0x5342;
		uint16 FineStrMetricTh = 0x0A20;
		/* Clip1 WB Thresh */
		uint16 g_clip1wb1 = 0x14;
		uint16 g_clip1wb2 = 0x14;
		/* NB Clip  */
		uint16 g_clipnb1  = 0xb4;
		uint16 g_clipnb2  = 0xb4;
		/* Hi Gain  */
		uint16 g_cliphiA1 = 0x74;
		uint16 g_cliphiA2 = 0x74;
		uint16 g_cliphiB1 = 0x14;
		uint16 g_cliphiB2 = 0x14;
		/* MD Gain  */
		uint16 g_clipmdA1 = 0x60;
		uint16 g_clipmdA2 = 0x60;
		uint16 g_clipmdB1 = 0x14;
		uint16 g_clipmdB2 = 0x14;
		/* LO Gain  */
		uint16 g_cliploA1 = 0x8a;
		uint16 g_cliploA2 = 0x8a;
		uint16 g_cliploB1 = 0x08;
		uint16 g_cliploB2 = 0x08;
		/* InitGCode */
		uint16 g_initGCA1 = 0x94;
		uint16 g_initGCA2 = 0x94;
		uint16 g_initGCB1 = 0x614;
		uint16 g_initGCB2 = 0x614;
		/* InitGain */
		uint16 g_initgain[] = {0x614a, 0x614a};
		/* eLNA Gains [dB] */
		int8   g_elnagain1[] = {13, 13};
		int8   g_elnagain2[] = {13, 13};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {11, 17, 21, 26};
		int8   lna1gain2[] = {11, 17, 21, 26};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 7, 11, 15};
		int8   lna2gain2[] = {0, 7, 11, 15};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* MixTIA Gains */
		int8   mixtia1[] = {-5, -2, 1, 4, 7, 7, 7, 7, 7, 7};
		int8   mixtia2[] = {-5, -2, 1, 4, 7, 7, 7, 7, 7, 7};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};
		uint8  gainbits2[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};
		/* Clip2 */
		uint8  clip2c1  = 1;
		uint8  clip2c2  = 1;
		/* Radio REGs */
		uint8  radioreg_0x86 = 0xc0;
		uint8  radioreg_0x10B = 0xc0;

		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		            (g_crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0, NPHY_crsminpowerl0_crsminpower0_MASK,
		            (g_crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_FSTRHiPwrTh, fineStrHiPwrTh);
		phy_utils_write_phyreg(pi, NPHY_FSTRMetricTh, FineStrMetricTh);
		phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
		            NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
		            (g_clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
		            NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
		            (g_clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, g_clipnb1);
		phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, g_clipnb2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, g_cliphiA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, g_cliphiA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, g_cliphiB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, g_cliphiB2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, g_clipmdA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, g_clipmdA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, g_clipmdB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, g_clipmdB2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, g_cliploA1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, g_cliploA2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, g_cliploB1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, g_cliploB2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, g_initGCA1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, g_initGCA2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, g_initGCB1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, g_initGCB2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, g_initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, g_elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, g_elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);
		phy_utils_mod_phyreg(pi, NPHY_Core1computeGainInfo,
		            NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
		            (clip2c1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2computeGainInfo,
		            NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
		            (clip2c2 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE0,
		                         radioreg_0x86);
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE1,
		                         radioreg_0x10B);

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

			if ((fc >= 4920 && fc <= 4920)) {
			} else if ((fc >= 4925 && fc <= 5080)) {
			} else if ((fc >= 5180 && fc <= 5320)) {
			} else if ((fc >= 5500 && fc <= 5700)) {
			} else if ((fc >= 5745 && fc <= 5805)) {
			} else if ((fc >= 5825 && fc <= 5825)) {
			}

		/* BW40 customizations */
		} else {

			if ((fc >= 4920 && fc <= 5230)) {
				/* Clip1 WB Thresh */
				uint16 clip1wb1 = 0x14;
				uint16 clip1wb2 = 0x14;

				phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				            NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				            (clip1wb1 <<
				             NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
				phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				            NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				            (clip1wb2 <<
				             NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			} else if ((fc >= 5240 && fc <= 5550)) {
				/* Clip1 WB Thresh */
				uint16 clip1wb1 = 0x14;
				uint16 clip1wb2 = 0x14;

				phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				            NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				            (clip1wb1 <<
				             NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
				phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				            NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				            (clip1wb2 <<
				             NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			} else if ((fc >= 5560 && fc <= 5825)) {
				/* Clip1 WB Thresh */
				uint16 clip1wb1 = 0x14;
				uint16 clip1wb2 = 0x14;

				phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				            NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				            (clip1wb1 <<
				             NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
				phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				            NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				            (clip1wb2 <<
				             NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			}
		}
	}
}

/*
 * AUTOGENERATED from ../47xxdata/mimophy/subband_cust_43236B1_sulley.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_43236B1_sulley_nphy(phy_info_t *pi)
{
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {

		/* Default customizations */

		/* InitGain */
		uint16 initgain[] = {0x6236, 0x6236};
		/* eLNA Gains [dB] */
		int8   elnagain1[] = {13, 13};
		int8   elnagain2[] = {13, 13};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {10, 15, 20, 20};
		int8   lna1gain2[] = {10, 15, 20, 20};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x2};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x2};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 10, 10, 10};
		int8   lna2gain2[] = {0, 10, 10, 10};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x1, 0x1};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x1, 0x1};
		/* MixTIA Gains */
		int8   mixtia1[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		int8   mixtia2[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		uint8  gainbits2[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		/* Vmid and Av for {RSSI invalid PWRDET TSSI} */
		uint16 vmidmid0[] = {0x81};
		uint16 avmid0[] = {0x1};
		uint16 vmidmid1[] = {0x89};
		uint16 avmid1[] = {0x1};

		PHY_REG_LIST_START
			/* CRS MinPwr */
			PHY_REG_MOD_ENTRY(NPHY, bphycrsminpower0, bphycrsminpower0, 0x46)
			/* Fine Str */
			PHY_REG_WRITE_ENTRY(NPHY, FSTRHiPwrTh, 0x5342)
			PHY_REG_WRITE_ENTRY(NPHY, FSTRMetricTh, 0x0A20)
			/* Clip1 WB Thresh */
			PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057, clip1wbThreshold, 0x14)
			PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057, clip1wbThreshold, 0x14)
			/* NB Clip  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1nbClipThreshold, 0xb4)
			PHY_REG_WRITE_ENTRY(NPHY, Core2nbClipThreshold, 0xb4)
			/* Hi Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipHiGainCodeA2057, 0x6c)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipHiGainCodeA2057, 0x6c)
			/* MD Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeA2057, 0x60)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeA2057, 0x60)
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeB2057, 0x04)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeB2057, 0x04)
			/* LO Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x4a)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x4a)
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x8)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x8)
			/* InitGCode */
			PHY_REG_WRITE_ENTRY(NPHY, Core1InitGainCodeA2057, 0x6c)
			PHY_REG_WRITE_ENTRY(NPHY, Core2InitGainCodeA2057, 0x6c)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);

		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
				NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
				1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
				NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
				1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x0b, 16, vmidmid0);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x0f, 16, avmid0);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1b, 16, vmidmid1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1f, 16, avmid1);

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

			if ((fc >= 2412 && fc <= 2484)) {
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x40)
					PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x40)
				PHY_REG_LIST_EXECUTE(pi);
			}

		/* BW40 customizations */
		} else {

			if ((fc >= 2422 && fc <= 2462)) {
				PHY_REG_LIST_START
					PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x48)
					PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x48)
				PHY_REG_LIST_EXECUTE(pi);
			}
		}
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {

		/* Default customizations */

		/* InitGain */
		uint16 initgain[] = {0x622a, 0x622a};
		/* eLNA Gains [dB] */
		int8   elnagain1[] = {13, 13};
		int8   elnagain2[] = {13, 13};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {11, 17, 21, 21};
		int8   lna1gain2[] = {11, 17, 21, 21};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x2};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x2};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 7, 11, 11};
		int8   lna2gain2[] = {0, 7, 11, 11};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x2, 0x2};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x2, 0x2};
		/* MixTIA Gains */
		int8   mixtia1[] = {-5, -2, 1, 1, 1, 1, 1, 1, 1, 1};
		int8   mixtia2[] = {-5, -2, 1, 1, 1, 1, 1, 1, 1, 1};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x0, 0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2};
		uint8  gainbits2[] = {0x0, 0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2};
		/* Vmid and Av for {RSSI invalid PWRDET TSSI} */
		uint16 vmidmid0[] = {0x6c};
		uint16 avmid0[] = {0x3};
		uint16 vmidmid1[] = {0x74};
		uint16 avmid1[] = {0x3};
		/* Radio REGs */
		uint8  radioreg_0x86 = 0xc0;
		uint8  radioreg_0x10B = 0xc0;

		PHY_REG_LIST_START
			/* CRS MinPwr */
			PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x3C)
			PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x3C)
			/* Fine Str */
			PHY_REG_WRITE_ENTRY(NPHY, FSTRHiPwrTh, 0x5342)
			PHY_REG_WRITE_ENTRY(NPHY, FSTRMetricTh, 0x0A20)
			/* Clip1 WB Thresh */
			PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057, clip1wbThreshold, 0x14)
			PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057, clip1wbThreshold, 0x14)
			/* NB Clip  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1nbClipThreshold, 0xb4)
			PHY_REG_WRITE_ENTRY(NPHY, Core2nbClipThreshold, 0xb4)
			/* Hi Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipHiGainCodeA2057, 0x54)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipHiGainCodeA2057, 0x54)
			/* MD Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeA2057, 0x24)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeA2057, 0x24)
			PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeB2057, 0x4)
			PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeB2057, 0x4)
			/* LO Gain  */
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x54)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x54)
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeB2057, 0x8)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeB2057, 0x8)
			/* InitGCode */
			PHY_REG_WRITE_ENTRY(NPHY, Core1InitGainCodeA2057, 0x54)
			PHY_REG_WRITE_ENTRY(NPHY, Core2InitGainCodeA2057, 0x54)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);

		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
				NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
				1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
				NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
				1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_LIST_EXECUTE(pi);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x0b, 16, vmidmid0);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x0f, 16, avmid0);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1b, 16, vmidmid1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1f, 16, avmid1);
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE0,
		                         radioreg_0x86);
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE1,
		                         radioreg_0x10B);

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

			if ((fc >= 4920 && fc <= 4920)) {
			} else if ((fc >= 4925 && fc <= 5080)) {
			} else if ((fc >= 5180 && fc <= 5320)) {
			} else if ((fc >= 5500 && fc <= 5700)) {
			        /* Vmid and Av for {RSSI invalid PWRDET TSSI} */
				vmidmid0[0] = 0x5f;
				avmid0[0] = 0x04;

				wlc_phy_table_write_nphy(
					pi, NPHY_TBL_ID_AFECTRL, 1, 0x0b, 16, vmidmid0);
				wlc_phy_table_write_nphy(
					pi, NPHY_TBL_ID_AFECTRL, 1, 0x0f, 16, avmid0);
			} else if ((fc >= 5745 && fc <= 5805)) {
			} else if ((fc >= 5825 && fc <= 5825)) {
			}

		/* BW40 customizations */
		} else {

			if ((fc >= 4920 && fc <= 5320)) {
			} else if ((fc >= 5500 && fc <= 5700)) {
				/* Vmid and Av for {RSSI invalid PWRDET TSSI} */
				vmidmid0[0] = 0x5f;
				avmid0[0] = 0x04;

				wlc_phy_table_write_nphy(
					pi, NPHY_TBL_ID_AFECTRL, 1, 0x0b, 16, vmidmid0);
				wlc_phy_table_write_nphy(
					pi, NPHY_TBL_ID_AFECTRL, 1, 0x0f, 16, avmid0);
			} else if ((fc >= 5745 && fc <= 5825)) {
			}
		}
	}
}

static void
wlc_phy_subband_cust_43236B1_um_nphy(phy_info_t *pi)
{
	uint16 fc;
	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);
	int32 edcrs_th;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* Reduce edcrs for EU by 3dBs */
	if (region_group == REGION_EU) {
		/* Need to pass -70dBm/Mhz (2g), -73dBm/Mhz (5g) inband noise */
		edcrs_th = (CHSPEC_IS2G(pi->radio_chanspec)) ? -64 : -67;
		wlc_phy_adjust_ed_thres(pi, &edcrs_th, TRUE);
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {

		/* Default customizations */
		/* CRS MinPwr */
		uint16 bcrsmin  = 0x46;
		uint16 g_crsminu  = 0x44;
		uint16 g_crsminl  = 0x44;
		/* Fine Str */
		uint16 fineStrHiPwrTh = 0x5342;
		uint16 FineStrMetricTh = 0x0A20;
		/* Clip1 WB Thresh */
		uint16 g_clip1wb1 = 0x10;
		uint16 g_clip1wb2 = 0x10;
		/* NB Clip  */
		uint16 g_clipnb1  = 0xd4;
		uint16 g_clipnb2  = 0xd4;
		/* Hi Gain  */
		uint16 g_cliphiA1 = 0x6c;
		uint16 g_cliphiA2 = 0x6c;
		uint16 g_cliphiB1 = 0x14;
		uint16 g_cliphiB2 = 0x14;
		/* MD Gain  */
		uint16 g_clipmdA1 = 0x60;
		uint16 g_clipmdA2 = 0x60;
		uint16 g_clipmdB1 = 0x4;
		uint16 g_clipmdB2 = 0x4;
		/* LO Gain  */
		uint16 g_cliploA1 = 0x68;
		uint16 g_cliploA2 = 0x68;
		uint16 g_cliploB1 = 0xc;
		uint16 g_cliploB2 = 0xc;
		/* InitGCode */
		uint16 g_initGCA1 = 0x6c;
		uint16 g_initGCA2 = 0x6c;
		uint16 g_initGCB1 = 0x624;
		uint16 g_initGCB2 = 0x624;
		/* InitGain */
		uint16 g_initgain[] = {0x6236, 0x6236};
		/* eLNA Gains [dB] */
		int8   g_elnagain1[] = {15, 15};
		int8   g_elnagain2[] = {15, 15};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {10, 15, 20, 20};
		int8   lna1gain2[] = {10, 15, 20, 20};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x2};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x2};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 10, 10, 10};
		int8   lna2gain2[] = {0, 10, 10, 10};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x1, 0x1};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x1, 0x1};
		/* MixTIA Gains */
		int8   mixtia1[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		int8   mixtia2[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		uint8  gainbits2[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		/* Clip2 */
		uint8  clip2c1  = 1;
		uint8  clip2c2  = 1;

		phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
		                     NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
		                     (bcrsmin << NPHY_bphycrsminpower0_bphycrsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		(g_crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0, NPHY_crsminpowerl0_crsminpower0_MASK,
		(g_crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_FSTRHiPwrTh, fineStrHiPwrTh);
		phy_utils_write_phyreg(pi, NPHY_FSTRMetricTh, FineStrMetricTh);
		phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, g_clipnb1);
		phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, g_clipnb2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, g_cliphiA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, g_cliphiA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, g_cliphiB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, g_cliphiB2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, g_clipmdA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, g_clipmdA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, g_clipmdB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, g_clipmdB2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, g_cliploA1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, g_cliploA2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, g_cliploB1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, g_cliploB2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, g_initGCA1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, g_initGCA2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, g_initGCB1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, g_initGCB2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, g_initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, g_elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, g_elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);
		phy_utils_mod_phyreg(pi, NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c2 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

			if ((fc >= 2412 && fc <= 2484)) {
			}

		/* BW40 customizations */
		} else {

			if ((fc >= 2422 && fc <= 2462)) {
			}
		}
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {

		/* Default customizations */
		/* CRS MinPwr */
		uint16 g_crsminu  = 0x44;
		uint16 g_crsminl  = 0x44;
		/* Fine Str */
		uint16 fineStrHiPwrTh = 0x5342;
		uint16 FineStrMetricTh = 0x0A20;
		/* Clip1 WB Thresh */
		uint16 g_clip1wb1 = 0x2F;
		uint16 g_clip1wb2 = 0x2F;
		/* NB Clip  */
		uint16 g_clipnb1  = 0xB4;
		uint16 g_clipnb2  = 0xB4;
		/* Hi Gain  */
		uint16 g_cliphiA1 = 0x6E;
		uint16 g_cliphiA2 = 0x6E;
		uint16 g_cliphiB1 = 0x0114;
		uint16 g_cliphiB2 = 0x0114;
		/* MD Gain  */
		uint16 g_clipmdA1 = 0x22;
		uint16 g_clipmdA2 = 0x22;
		uint16 g_clipmdB1 = 0x0104;
		uint16 g_clipmdB2 = 0x0104;
		/* LO Gain  */
		uint16 g_cliploA1 = 0x20;
		uint16 g_cliploA2 = 0x20;
		uint16 g_cliploB1 = 0x030C;
		uint16 g_cliploB2 = 0x030C;
		/* InitGCode */
		uint16 g_initGCA1 = 0x6E;
		uint16 g_initGCA2 = 0x6E;
		uint16 g_initGCB1 = 0x0624;
		uint16 g_initGCB2 = 0x0624;
		/* InitGain */
		uint16 g_initgain[] = {0x6237, 0x6237};
		/* eLNA Gains [dB] */
		int8   g_elnagain1[] = {11, 11};
		int8   g_elnagain2[] = {11, 11};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {11, 17, 21, 26};
		int8   lna1gain2[] = {11, 17, 21, 26};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 7, 11, 14};
		int8   lna2gain2[] = {0, 7, 11, 14};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* MixTIA Gains */
		int8   mixtia1[] = {-5, -2, 1, 4, 7, 7, 7, 7, 7, 7};
		int8   mixtia2[] = {-5, -2, 1, 4, 7, 7, 7, 7, 7, 7};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};
		uint8  gainbits2[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};
		/* Clip2 */
		uint8  clip2c1  = 1;
		uint8  clip2c2  = 1;
		/* Radio REGs */
		uint8  radioreg_0x86 = 0xc0;
		uint8  radioreg_0x10B = 0xc0;

		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
			(g_crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0, NPHY_crsminpowerl0_crsminpower0_MASK,
			(g_crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_FSTRHiPwrTh, fineStrHiPwrTh);
		phy_utils_write_phyreg(pi, NPHY_FSTRMetricTh, FineStrMetricTh);
		phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, g_clipnb1);
		phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, g_clipnb2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, g_cliphiA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, g_cliphiA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, g_cliphiB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, g_cliphiB2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, g_clipmdA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, g_clipmdA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, g_clipmdB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, g_clipmdB2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, g_cliploA1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, g_cliploA2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, g_cliploB1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, g_cliploB2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, g_initGCA1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, g_initGCA2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, g_initGCB1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, g_initGCB2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, g_initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, g_elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, g_elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);
		phy_utils_mod_phyreg(pi, NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c2 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE0,
		                         radioreg_0x86);
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE1,
		                         radioreg_0x10B);

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {
		 if ((fc >= 4920 && fc <= 4920)) {
		 } else if ((fc >= 4925 && fc <= 5080)) {
		 } else if ((fc >= 5180 && fc <= 5240)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x48;
			uint16 crsminl  = 0x48;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		 } else if ((fc >= 5260 && fc <= 5320)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x45;
			uint16 crsminl  = 0x45;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			 NPHY_crsminpoweru0_crsminpower0_MASK,
			 (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			 NPHY_crsminpowerl0_crsminpower0_MASK,
			 (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			 NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			 NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		 } else if ((fc >= 5500 && fc <= 5580)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x45;
			uint16 crsminl  = 0x45;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			 NPHY_crsminpoweru0_crsminpower0_MASK,
			 (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			 NPHY_crsminpowerl0_crsminpower0_MASK,
			 (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			 NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			 NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		 } else if ((fc >= 5600 && fc <= 5700)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x47;
			uint16 crsminl  = 0x47;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			 NPHY_crsminpoweru0_crsminpower0_MASK,
			 (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			 NPHY_crsminpowerl0_crsminpower0_MASK,
			 (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			 NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			 NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		 } else if ((fc >= 5745 && fc <= 5805)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x49;
			uint16 crsminl  = 0x49;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			 NPHY_crsminpoweru0_crsminpower0_MASK,
			 (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			 NPHY_crsminpowerl0_crsminpower0_MASK,
			 (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			 NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			 NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
			}
		/* BW40 customizations */
		} else {
		   if ((fc >= 4920 && fc <= 5310)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x44;
			uint16 crsminl  = 0x44;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			 NPHY_crsminpoweru0_crsminpower0_MASK,
			 (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			 NPHY_crsminpowerl0_crsminpower0_MASK,
			 (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			 NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			 NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		 } else if ((fc >= 5510 && fc <= 5690)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x43;
			uint16 crsminl  = 0x43;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			  NPHY_crsminpoweru0_crsminpower0_MASK,
			  (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			 NPHY_crsminpowerl0_crsminpower0_MASK,
			 (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			 NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			  NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			  (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
		 } else if ((fc >= 5755 && fc <= 5825)) {
			/* CRS MinPwr */
			uint16 crsminu  = 0x44;
			uint16 crsminl  = 0x44;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x2F;
			uint16 clip1wb2 = 0x2F;
			/* NB Clip  */
			uint16 clipnb1  = 0xB4;
			uint16 clipnb2  = 0xB4;
			/* Hi Gain  */
			uint16 cliphiA1 = 0x6E;
			uint16 cliphiA2 = 0x6E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain  */
			uint16 clipmdA1 = 0x22;
			uint16 clipmdA2 = 0x22;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain  */
			uint16 cliploA1 = 0x20;
			uint16 cliploA2 = 0x20;
			uint16 cliploB1 = 0x030C;
			uint16 cliploB2 = 0x030C;
			/* InitGCode */
			uint16 initGCA1 = 0x6E;
			uint16 initGCA2 = 0x6E;
			uint16 initGCB1 = 0x0624;
			uint16 initGCB2 = 0x0624;
			/* InitGain */
			uint16 initgain[] = {0x6237, 0x6237};
			/* eLNA Gains [dB] */
			int8   elnagain1[] = {11, 11};
			int8   elnagain2[] = {11, 11};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			 NPHY_crsminpoweru0_crsminpower0_MASK,
			 (crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			 NPHY_crsminpowerl0_crsminpower0_MASK,
			 (crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			 NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			 NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			 (clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, initGCB1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, initGCB2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, elnagain1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, elnagain2);
			}
		}
	}
}

/*
 * AUTOGENERATED from ../47xxdata/mimophy/subband_cust_43236B1_usbelna.xls
 * DO NOT EDIT
 */

static void
wlc_phy_subband_cust_43236B1_usbelna_nphy(phy_info_t *pi)
{
	uint16 fc;
	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* 2G Band Customizations */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {

		/* Default customizations */
		/* CRS MinPwr */
		uint16 bcrsmin	= 0x46;
		uint16 g_crsminu	= 0x48;
		uint16 g_crsminl	= 0x48;
		/* Fine Str */
		uint16 fineStrHiPwrTh = 0x5342;
		uint16 FineStrMetricTh = 0x0A20;
		/* Clip1 WB Thresh */
		uint16 g_clip1wb1 = 0x14;
		uint16 g_clip1wb2 = 0x14;
		/* NB Clip	*/
		uint16 g_clipnb1	= 0xb4;
		uint16 g_clipnb2	= 0xb4;
		/* Hi Gain	*/
		uint16 g_cliphiA1 = 0x6e;
		uint16 g_cliphiA2 = 0x6e;
		uint16 g_cliphiB1 = 0x14;
		uint16 g_cliphiB2 = 0x14;
		/* MD Gain	*/
		uint16 g_clipmdA1 = 0x62;
		uint16 g_clipmdA2 = 0x62;
		uint16 g_clipmdB1 = 0x14;
		uint16 g_clipmdB2 = 0x14;
		/* LO Gain	*/
		uint16 g_cliploA1 = 0x6c;
		uint16 g_cliploA2 = 0x6c;
		uint16 g_cliploB1 = 0x8;
		uint16 g_cliploB2 = 0x8;
		/* InitGCode */
		uint16 g_initGCA1 = 0x6e;
		uint16 g_initGCA2 = 0x6e;
		/* InitGain */
		uint16 g_initgain[] = {0x6237, 0x6237};
		/* eLNA Gains [dB] */
		int8   g_elnagain1[] = {10, 10};
		int8   g_elnagain2[] = {10, 10};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {10, 15, 20, 25};
		int8   lna1gain2[] = {10, 15, 20, 25};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 10, 15, 18};
		int8   lna2gain2[] = {0, 10, 15, 18};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* MixTIA Gains */
		int8   mixtia1[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		int8   mixtia2[] = {2, 2, 2, 5, 5, 5, 5, 5, 5, 5};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		uint8  gainbits2[] = {0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3};
		/* Clip2 */
		uint8  clip2c1	= 1;
		uint8  clip2c2	= 1;

		phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
		                     NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
		                     (bcrsmin << NPHY_bphycrsminpower0_bphycrsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
			(g_crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0, NPHY_crsminpowerl0_crsminpower0_MASK,
			(g_crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_FSTRHiPwrTh, fineStrHiPwrTh);
		phy_utils_write_phyreg(pi, NPHY_FSTRMetricTh, FineStrMetricTh);
		phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, g_clipnb1);
		phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, g_clipnb2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, g_cliphiA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, g_cliphiA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, g_cliphiB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, g_cliphiB2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, g_clipmdA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, g_clipmdA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, g_clipmdB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, g_clipmdB2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, g_cliploA1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, g_cliploA2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, g_cliploB1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, g_cliploB2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, g_initGCA1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, g_initGCA2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, g_initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, g_elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, g_elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);
		phy_utils_mod_phyreg(pi, NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c2 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {

			if ((fc >= 2412 && fc <= 2484)) {
			}

		/* BW40 customizations */
		} else {

			if ((fc >= 2422 && fc <= 2462)) {
			}
		}
	}

	/* 5G Band Customizations */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {

		/* Default customizations */
		/* CRS MinPwr */
		uint16 g_crsminu	= 0x47;
		uint16 g_crsminl	= 0x47;
		/* Fine Str */
		uint16 fineStrHiPwrTh = 0x5342;
		uint16 FineStrMetricTh = 0x0A20;
		/* Clip1 WB Thresh */
		uint16 g_clip1wb1 = 0x14;
		uint16 g_clip1wb2 = 0x14;
		/* NB Clip	*/
		uint16 g_clipnb1	= 0xb4;
		uint16 g_clipnb2	= 0xb4;
		/* Hi Gain	*/
		uint16 g_cliphiA1 = 0x9e;
		uint16 g_cliphiA2 = 0x9e;
		uint16 g_cliphiB1 = 0x14;
		uint16 g_cliphiB2 = 0x14;
		/* MD Gain	*/
		uint16 g_clipmdA1 = 0x62;
		uint16 g_clipmdA2 = 0x62;
		uint16 g_clipmdB1 = 0x24;
		uint16 g_clipmdB2 = 0x24;
		/* LO Gain	*/
		uint16 g_cliploA1 = 0x8a;
		uint16 g_cliploA2 = 0x8a;
		uint16 g_cliploB1 = 0x8;
		uint16 g_cliploB2 = 0x8;
		/* InitGCode */
		uint16 g_initGCA1 = 0x9e;
		uint16 g_initGCA2 = 0x9e;
		/* InitGain */
		uint16 g_initgain[] = {0x6247, 0x6247};
		/* eLNA Gains [dB] */
		int8   g_elnagain1[] = {9, 9};
		int8   g_elnagain2[] = {9, 9};
		/* LNA1 Gains [dB] */
		int8   lna1gain1[] = {11, 17, 21, 26};
		int8   lna1gain2[] = {11, 17, 21, 26};
		/* LNA1 GainBits */
		uint8  lna1gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna1gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* LNA2 Gains [dB] */
		int8   lna2gain1[] = {0, 7, 11, 14};
		int8   lna2gain2[] = {0, 7, 11, 14};
		/* LNA2 GainBits */
		uint8  lna2gainbits1[] = {0x0, 0x1, 0x2, 0x3};
		uint8  lna2gainbits2[] = {0x0, 0x1, 0x2, 0x3};
		/* MixTIA Gains */
		int8   mixtia1[] = {-5, -2, 1, 4, 7, 7, 7, 7, 7, 7};
		int8   mixtia2[] = {-5, -2, 1, 4, 7, 7, 7, 7, 7, 7};
		/* Mixer GainBits */
		uint8  gainbits1[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};
		uint8  gainbits2[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4};
		/* Clip2 */
		uint8  clip2c1	= 1;
		uint8  clip2c2	= 1;
		/* Radio REGs */
		uint8  radioreg_0x86 = 0xc0;
		uint8  radioreg_0x10B = 0xc0;

		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
			NPHY_crsminpoweru0_crsminpower0_MASK,
			(g_crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
			NPHY_crsminpowerl0_crsminpower0_MASK,
			(g_crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_FSTRHiPwrTh, fineStrHiPwrTh);
		phy_utils_write_phyreg(pi, NPHY_FSTRMetricTh, FineStrMetricTh);
		phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
			NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
			NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
			(g_clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
		phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, g_clipnb1);
		phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, g_clipnb2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, g_cliphiA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, g_cliphiA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, g_cliphiB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, g_cliphiB2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, g_clipmdA1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, g_clipmdA2);
		phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, g_clipmdB1);
		phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, g_clipmdB2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, g_cliploA1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, g_cliploA2);
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, g_cliploB1);
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, g_cliploB2);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, g_initGCA1);
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, g_initGCA2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, g_initgain);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 2, 0x0, 8, g_elnagain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 2, 0x0, 8, g_elnagain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x8, 8, lna1gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x8, 8, lna1gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2gain1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2gain2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 4, 0x10, 8, lna2gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 4, 0x10, 8, lna2gainbits2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, mixtia1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, mixtia2);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8, gainbits1);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8, gainbits2);
		phy_utils_mod_phyreg(pi, NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_mod_phyreg(pi, NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			(clip2c2 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT));
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE0,
		                         radioreg_0x86);
		phy_utils_write_radioreg(pi, RADIO_2057_RXRF_IABAND_RXGM_IMAIN_PTAT_CORE1,
		                         radioreg_0x10B);

		/* BW20 customizations */
		if (CHSPEC_IS20(pi->radio_chanspec) == 1) {
			if ((fc >= 4920 && fc <= 4920)) {
			} else if ((fc >= 4925 && fc <= 5080)) {
			} else if ((fc >= 5180 && fc <= 5240)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x44;
			uint16 crsminl	= 0x44;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x42;
			uint16 clipmdA2 = 0x42;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x40;
			uint16 cliploA2 = 0x40;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x6247, 0x6247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			} else if ((fc >= 5260 && fc <= 5320)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x42;
			uint16 crsminl	= 0x42;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x42;
			uint16 clipmdA2 = 0x42;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x40;
			uint16 cliploA2 = 0x40;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x6247, 0x6247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			} else if ((fc >= 5500 && fc <= 5580)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x47;
			uint16 crsminl	= 0x47;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x42;
			uint16 clipmdA2 = 0x42;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x40;
			uint16 cliploA2 = 0x40;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x6247, 0x6247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			} else if ((fc >= 5600 && fc <= 5700)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x49;
			uint16 crsminl	= 0x49;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x42;
			uint16 clipmdA2 = 0x42;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x40;
			uint16 cliploA2 = 0x40;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x5247, 0x5247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			} else if ((fc >= 5745 && fc <= 5805)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x4D;
			uint16 crsminl	= 0x4D;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x42;
			uint16 clipmdA2 = 0x42;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x40;
			uint16 cliploA2 = 0x40;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x5247, 0x5247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			}
		/* BW40 customizations */
		} else {
			if ((fc >= 4920 && fc <= 5310)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x43;
			uint16 crsminl	= 0x43;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x44;
			uint16 clipmdA2 = 0x44;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x42;
			uint16 cliploA2 = 0x42;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x6247, 0x6247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			} else if ((fc >= 5510 && fc <= 5690)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x47;
			uint16 crsminl	= 0x47;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x44;
			uint16 clipmdA2 = 0x44;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x42;
			uint16 cliploA2 = 0x42;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x5247, 0x5247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			} else if ((fc >= 5755 && fc <= 5825)) {
			/* CRS MinPwr */
			uint16 crsminu	= 0x4A;
			uint16 crsminl	= 0x4A;
			/* Clip1 WB Thresh */
			uint16 clip1wb1 = 0x32;
			uint16 clip1wb2 = 0x32;
			/* NB Clip	*/
			uint16 clipnb1	= 0x0B4;
			uint16 clipnb2	= 0x0B4;
			/* Hi Gain	*/
			uint16 cliphiA1 = 0x8E;
			uint16 cliphiA2 = 0x8E;
			uint16 cliphiB1 = 0x0114;
			uint16 cliphiB2 = 0x0114;
			/* MD Gain	*/
			uint16 clipmdA1 = 0x44;
			uint16 clipmdA2 = 0x44;
			uint16 clipmdB1 = 0x0104;
			uint16 clipmdB2 = 0x0104;
			/* LO Gain	*/
			uint16 cliploA1 = 0x42;
			uint16 cliploA2 = 0x42;
			uint16 cliploB1 = 0x0108;
			uint16 cliploB2 = 0x0108;
			/* InitGCode */
			uint16 initGCA1 = 0x8E;
			uint16 initGCA2 = 0x8E;
			/* InitGain */
			uint16 initgain[] = {0x5247, 0x5247};

			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				(crsminu << NPHY_crsminpoweru0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				(crsminl << NPHY_crsminpowerl0_crsminpower0_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				NPHY_Core1clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb1 << NPHY_Core1clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				NPHY_Core2clipwbThreshold2057_clip1wbThreshold_MASK,
				(clip1wb2 << NPHY_Core2clipwbThreshold2057_clip1wbThreshold_SHIFT));
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, clipnb1);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, clipnb2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057, cliphiA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057, cliphiA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057, cliphiB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057, cliphiB2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeA2057, clipmdA1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeA2057, clipmdA2);
			phy_utils_write_phyreg(pi, NPHY_Core1clipmdGainCodeB2057, clipmdB1);
			phy_utils_write_phyreg(pi, NPHY_Core2clipmdGainCodeB2057, clipmdB2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057, cliploA1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057, cliploA2);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, cliploB1);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, cliploB2);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, initGCA1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, initGCA2);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);
			}
		}
	}
}

void
wlc_phy_set_spurmode_nphy(phy_info_t *pi, chanspec_t chanspec)
{
	bool suspend = FALSE;
	uint8 spuravoid = WL_SPURAVOID_OFF;
	uint8 val = CHSPEC_CHANNEL(chanspec);

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
		/* 53572 */
		if (!CHSPEC_IS40(pi->radio_chanspec)) {
		/* BW20 */
			if (val == 13) {
				spuravoid = WL_SPURAVOID_ON1;
			}
		}
	} else {
		/* Default channels */
		if (!CHSPEC_IS40(pi->radio_chanspec)) {
			/* BW20 */
			if ((val == 13) || (val == 14) || (val == 153)) {
				spuravoid = WL_SPURAVOID_ON1;
			}
		} else {
			/* BW40 */
			if (val == 54) {
				spuravoid = WL_SPURAVOID_ON1;
			} else if ((val == 118) || (val == 151)) {
				/* ON2 is available only in Rev8+ */
				if (NREV_GE(pi->pubpi->phy_rev, 8)) {
					spuravoid = WL_SPURAVOID_ON2;
				}
			}
		}
	}

	/* disable spuravoid if disable_spuravoid set to 1 in nvram */
	if (pi->sh->disable_spuravoid)
		spuravoid = WL_SPURAVOID_OFF;

	/* if user forces it to OFF/ON/ON2, honor it */
	if (pi->phy_spuravoid == SPURAVOID_DISABLE) {
		spuravoid = WL_SPURAVOID_OFF;
	} else if (pi->phy_spuravoid == SPURAVOID_FORCEON) {
		spuravoid = WL_SPURAVOID_ON1;
	} else if  (pi->phy_spuravoid == SPURAVOID_FORCEON2) {
		spuravoid = WL_SPURAVOID_ON2;
	}

	PHY_CAL(("wlc_phy_chanspec_nphy_setup: spuravoid %d\n", spuravoid));

	/* Skip pmu-spuravoid if spurmode has not changed */
	if (pi->phy_spuravoid_mode != spuravoid) {
		si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, spuravoid);
		pi->phy_spuravoid_mode = spuravoid;
	}

	/* MAC clock frequency change for certain chips in spur avoidance mode.
	 * These are the chips where the PHY and MAC clocks share the same PLL.
	 */
	wlapi_switch_macfreq(pi->sh->physhim, spuravoid);

	/* setup resampler bits in BBConfig */
	/* NPHY_BBConfig_resample_clk160_MASK not valild for lcnxnrev and higher phys */
	phy_utils_mod_phyreg(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK,
		((spuravoid == WL_SPURAVOID_OFF) ? 0 : NPHY_BBConfig_resample_clk160_MASK));

	if (spuravoid != WL_SPURAVOID_OFF)
		phy_utils_mod_phyreg(pi, NPHY_BBConfig, NPHY_BBConfig_resamplerFreq_MASK,
		            (spuravoid - WL_SPURAVOID_ON1) <<
		            NPHY_BBConfig_resamplerFreq_SHIFT);

	wlc_phy_resetcca_nphy(pi);

	pi->u.pi_nphy->phy_isspuravoid = (spuravoid > WL_SPURAVOID_OFF);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

}

static void
wlc_phy_chanspec_nphy_setup(phy_info_t *pi, chanspec_t chanspec, const nphy_sfo_cfg_t *ci)
{
	uint16 val;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* set phy Band bit, required to ensure correct band's tx/rx board
	 * level controls are being driven, 0 = 2.4G, 1 = 5G.
	 * Enable BPHY in 2G, and disable it in 5G.
	 */
	if (CHSPEC_IS5G(chanspec)) {		/* in 5G */

		/* switch BandControl to 2G temporarily so BPHY regs are accessible */
		phy_utils_and_phyreg(pi, NPHY_BandControl, ~NPHY_BandControl_currentBand_MASK);

		/* enable force gated clock on */
		val = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), (val | MAC_PHY_FORCE_CLK));

		/* Turn-off CCA and put bphy receiver in reset */
		phy_utils_or_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_BB_CONFIG),
			(BBCFG_RESETCCA | BBCFG_RESETRX));

		/* restore force gated clock */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), val);

		/* select 5G band */
		phy_utils_or_phyreg(pi, NPHY_BandControl, NPHY_BandControl_currentBand);

	} else if (!CHSPEC_IS5G(chanspec)) {	/* in 2G */
		/* select 2G band */
		phy_utils_and_phyreg(pi, NPHY_BandControl, ~NPHY_BandControl_currentBand);

		/* enable force gated clock on */
		val = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), (val | MAC_PHY_FORCE_CLK));

		/* Turn-on CCA and take bphy receiver out of reset */
		phy_utils_and_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_BB_CONFIG),
			(uint16)(~(BBCFG_RESETCCA | BBCFG_RESETRX)));

		/* restore force gated clock */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), val);
	}

	/* set SFO parameters
	 * sfo_chan_center_Ts20 = round([fc-10e6 fc fc+10e6] / 20e6 * 8), fc in Hz
	 *                      = round([$channel-10 $channel $channel+10] * 0.4),
	 *                              $channel in MHz
	 */
	phy_utils_write_phyreg(pi, NPHY_BW1a, ci->PHY_BW1a);
	phy_utils_write_phyreg(pi, NPHY_BW2, ci->PHY_BW2);
	phy_utils_write_phyreg(pi, NPHY_BW3, ci->PHY_BW3);

	/* sfo_chan_center_factor = round(2^17./([fc-10e6 fc fc+10e6]/20e6)), fc in Hz
	 *                        = round(2621440./[$channel-10 $channel $channel+10]),
	 *                                $channel in MHz
	 */
	phy_utils_write_phyreg(pi, NPHY_BW4, ci->PHY_BW4);
	phy_utils_write_phyreg(pi, NPHY_BW5, ci->PHY_BW5);
	phy_utils_write_phyreg(pi, NPHY_BW6, ci->PHY_BW6);

	if (CHSPEC_CHANNEL(pi->radio_chanspec) == 14) {
		wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_ofdm_en, 0);
		/* Bit 11 and 6 of BPHY testRegister to '10' */
		phy_utils_or_phyreg(pi, NPHY_TO_BPHY_OFF + BPHY_TEST, 0x800);
	} else {
		wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_ofdm_en,
			NPHY_ClassifierCtrl_ofdm_en);
		/* Bit 11 and 6 of BPHY testRegister to '00' */
		if (CHSPEC_IS2G(chanspec))
			phy_utils_and_phyreg(pi, NPHY_TO_BPHY_OFF + BPHY_TEST, ~0x840);
	}

	/* initially force txgain in case txpwrctrl is disabled */
	if (pi->nphy_txpwrctrl == PHY_TPC_HW_OFF) {
		wlc_phy_txpwr_fixpower_nphy(pi);
	}

	/* needs dynamically spur avoidance, user can override the behavior */
	wlc_phy_set_spurmode_nphy(pi, chanspec);

	/* Subband customization for 43236B1 Sulley boards */
	if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) &&
	    (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)) {
		wlc_phy_subband_cust_43236B1_sulley_nphy(pi);
		/* for Sulley Dev1 legacy */
		/* Maybe we need to revisit it someday. Because the TRISO could be better */
		if (CHSPEC_IS2G(pi->radio_chanspec) && pi->fem2g->antswctrllut == 2) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x62)
				PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x62)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}

	if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) &&
		((pi->fem2g->antswctrllut == 11) ||
		(pi->fem5g->antswctrllut == 11)))
		wlc_phy_subband_cust_43236B1_usbelna_nphy(pi);

	if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) &&
		(pi->sh->boardtype == BCM943236PREPROTOBLU2O3_SSID)) {
		wlc_phy_subband_cust_43236B1_preproto_blu2o3_nphy(pi);
	}

	/* subband customization for 43236B1 UM boards, 2G only */
	if (((CHIPID(pi->sh->chip) == BCM43238_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43236_CHIP_ID)) &&
		((pi->fem2g->antswctrllut == 15) ||
		(pi->fem5g->antswctrllut == 15))) {
		wlc_phy_subband_cust_43236B1_um_nphy(pi);
	}

	phy_utils_write_phyreg(pi, NPHY_NumDatatonesdup40, 0x3830);

	/* Spur avoidance WAR for 4322 */
	wlc_phy_spurwar_nphy(pi);
}

void
wlc_phy_chanspec_set_nphy(phy_info_t *pi, chanspec_t chanspec)
{
	int freq;
	chan_info_nphy_radio2057_t      *t0 = NULL;
	chan_info_nphy_radio205x_t      *t1 = NULL;
	chan_info_nphy_radio2057_rev5_t *t2 = NULL;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);
#ifndef WLC_DISABLE_ACI
	bool aci_active_save = FALSE;
#endif // endif

	pi->aci_rev7_subband_cust_fix = CHIPID_43236X_FAMILY(pi) &&
		((pi->fem2g->antswctrllut == 11) ||
		(pi->fem5g->antswctrllut == 11) ||
		(pi->fem2g->antswctrllut == 15) ||
		(pi->fem5g->antswctrllut == 15));

	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);

	if (NORADIO_ENAB(pi->pubpi)) {
		return;
	}

	if (!wlc_phy_chan2freq_nphy(pi, CHSPEC_CHANNEL(chanspec), &freq, &t0, &t1, &t2))
		return;

#ifndef WLC_DISABLE_ACI
	if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
		(pi->interf->curr_home_channel ==
		CHSPEC_CHANNEL(pi->radio_chanspec)) &&
		pi->aci_rev7_subband_cust_fix) {
		aci_active_save = pi->aci_state & ACI_ACTIVE;
	}
#endif // endif

	/* Set the phy bandwidth as dictated by the chanspec */
	if (CHSPEC_BW(chanspec) != pi->bw)
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(chanspec));

	/* Restore ACI mitigation OFF parameters */
	if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
		pi->aci_rev7_subband_cust_fix) {
		if (((pi->aci_state & ACI_ACTIVE)) ||
			(pi->sh->interference_mode == WLAN_MANUAL)) {
			wlc_phy_acimode_set_nphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);
		}
	}

	pi_nphy->dynamic_rflo_war_en = FALSE;

	/* Set the correct sideband if in 40MHz mode */
	if (CHSPEC_IS40(chanspec)) {
		if (CHSPEC_SB_UPPER(chanspec)) {
			phy_utils_or_phyreg(pi, NPHY_RxControl, BPHY_BAND_SEL_UP20);
			phy_utils_or_phyreg(pi, NPHY_ClassifierCtrl2, PRIM_SEL_UP20);
		} else {
			phy_utils_and_phyreg(pi, NPHY_RxControl, ~BPHY_BAND_SEL_UP20);
			phy_utils_and_phyreg(pi, NPHY_ClassifierCtrl2, (~PRIM_SEL_UP20 & 0xffff));
		}
	}

	/* band specific 2057 radio inits */
	if ((RADIOREV(pi->pubpi->radiorev) <= 4) ||
		(RADIOREV(pi->pubpi->radiorev) == 6)) {
	    phy_utils_mod_radioreg(pi, RADIO_2057_TIA_CONFIG_CORE0, 0x2,
	                  (CHSPEC_IS5G(chanspec) ? (1 << 1) : 0));
	    phy_utils_mod_radioreg(pi, RADIO_2057_TIA_CONFIG_CORE1, 0x2,
	                  (CHSPEC_IS5G(chanspec) ? (1 << 1) : 0));
	}
	wlc_phy_chanspec_radio2057_setup(pi, t0, t2);

	wlc_phy_chanspec_nphy_setup(pi, chanspec,
		((RADIOREV(pi->pubpi->radiorev) == 5) ||
		(RADIOREV(pi->pubpi->radiorev) == 13) ||
		(RADIOREV(pi->pubpi->radiorev) == 14)) ?
		(const nphy_sfo_cfg_t *)&(t2->PHY_BW1a) :
		(const nphy_sfo_cfg_t *)&(t0->PHY_BW1a));

#ifndef WLC_DISABLE_ACI
	/* store current chanspec for interference */
	if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		pi->interf->radio_chanspec_stored = pi->radio_chanspec;
	}
	/* store aci mitigation off values */
	if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
		pi->aci_rev7_subband_cust_fix) {
		wlc_phy_aci_noise_store_values_nphy(pi);
	}
	if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
		(pi->interf->curr_home_channel ==
		CHSPEC_CHANNEL(pi->radio_chanspec)) &&
		pi->aci_rev7_subband_cust_fix) {
			pi->aci_state |= aci_active_save;
	}

	/* restore ACI-on/off register values */
	wlc_phy_aci_home_channel_nphy(pi, chanspec);

	if ((pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
		pi->sh->interference_mode == NON_WLAN)) {
		wlc_phy_noise_home_channel_nphy(pi, chanspec);
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_ACI(("CHANSPEC_SET:  Channel = %X DigiGainLimit = %X, PeakEnergyL = %X,"
			"InitGain = %X, crsmin = %X \n", chanspec,
			phy_utils_read_phyreg(pi, NPHY_DigiGainLimit0),
			phy_utils_read_phyreg(pi, NPHY_TO_BPHY_OFF+BPHY_PEAK_ENERGY_LO),
			phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2056),
			phy_utils_read_phyreg(pi, NPHY_crsminpoweru0)));
	} else {
		PHY_ACI(("CHANSPEC_SET:  Channel = %X,"
			"InitGain = %X, crsmin = %X \n", chanspec,
			phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2056),
			phy_utils_read_phyreg(pi, NPHY_crsminpoweru0)));
	}
#endif /* Compiling out ACI code */

#if defined(AP) && defined(RADAR)
	/* update radar detect mode specific params
	 * based on new chanspec
	 */
	phy_n_radar_upd(pi->u.pi_nphy->radari);
#endif /* defined(AP) && defined(RADAR) */

	phy_n_rssi_init_gain_err(pi->u.pi_nphy->rssii);

#if defined(RXDESENS_EN)
	/* save and remove desens when scan in prog */
	if (SCAN_RM_IN_PROGRESS(pi) && (pi_nphy->ntd_current_rxdesens != 0)) {
		pi_nphy->ntd_save_current_rxdesens = pi_nphy->ntd_current_rxdesens;
		pi_nphy->ntd_save_current_rxdesens_channel = pi->interf->curr_home_channel;
		wlc_nphy_set_rxdesens((wlc_phy_t *)pi, 0);
	}
#endif // endif
#if defined(ACI_DBG_PRINTS_EN)
/*
	wlc_phy_aci_noise_print_values_nphy(pi);
*/
#endif // endif

	if (!SCAN_RM_IN_PROGRESS(pi) && (pi->sh->sromrev >= 9))
		wlc_phy_copy_ppr_offsets_nphy(pi);

#if defined(RXDESENS_EN)
	/* apply desens back after scans */
	if ((pi_nphy->ntd_save_current_rxdesens != 0) &&
		(pi_nphy->ntd_save_current_rxdesens_channel ==
		CHSPEC_CHANNEL(pi->radio_chanspec))) {
		wlc_nphy_set_rxdesens((wlc_phy_t *)pi, pi_nphy->ntd_save_current_rxdesens);
	}
#endif // endif

	/* Set edcrs based on srom/nvram */
	if (region_group == REGION_EU)
		wlc_phy_set_srom_eu_edthresh_nphy(pi);
}

static void
wlc_phy_aci_home_channel_nphy(phy_info_t *pi, chanspec_t chanspec)
{

	bool suspend = FALSE;

	if ((CHSPEC_CHANNEL(chanspec) == pi->interf->curr_home_channel) ||
		(pi->sh->interference_mode == WLAN_MANUAL)) {
		/* back in home channel */
		/* if aci mitigation was on, turn it back on */
		if (CHSPEC_IS2G(chanspec) &&
			((((pi->sh->interference_mode == WLAN_AUTO_W_NOISE) ||
			(pi->sh->interference_mode == WLAN_AUTO)) &&
			(pi->aci_state & ACI_ACTIVE)) ||
			(pi->sh->interference_mode == WLAN_MANUAL))) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				wlc_phy_acimode_set_nphy(pi, TRUE, PHY_ACI_PWR_HIGH);
			}
		}

		if (((pi->sh->interference_mode == WLAN_AUTO_W_NOISE) ||
			(pi->sh->interference_mode == NON_WLAN)) &&
			(pi->phy_init_por == FALSE) &&
			(pi->interf->noise_sw_set == TRUE) &&
			NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {

			/* suspend mac if haven't done so */
			suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) &
				MCTL_EN_MAC);
			if (!suspend) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			}

			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				pi->interf->crsminpwrl0);
			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				pi->interf->crsminpwru0);
#ifdef BPHY_DESENSE
			phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
				NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
				pi->interf->bphy_crsminpwr);
#endif // endif
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057,
				pi->interf->init_gain_core1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057,
				pi->interf->init_gain_core1);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057,
				pi->interf->init_gainb_core1);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057,
				pi->interf->init_gainb_core1);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
				pi->pubpi->phy_corenum, 0x106,
				16, pi->interf->init_gain_rfseq);
			/* unsuspend mac */
			if (!suspend) {
				wlapi_enable_mac(pi->sh->physhim);
			}
		}

	} else {
		if (!(NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
			pi->aci_rev7_subband_cust_fix)) {
			if ((pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
				pi->sh->interference_mode == WLAN_AUTO ||
				pi->sh->interference_mode == WLAN_MANUAL) &&
				(pi->aci_state & ACI_ACTIVE) &&
				((CHSPEC_IS2G(pi->radio_chanspec) &&
				(pi->phy_init_por == FALSE)) ||
				(NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV)))) {
				wlc_phy_acimode_set_nphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);
			}
		}
	}

}

#ifndef WLC_DISABLE_ACI
static void
wlc_phy_bphy_ofdm_noise_update_LUT(phy_info_t *pi, bool aci_enable)
{
	/* If enable == 1, transition ACI_OFF -> ACI_ON
	  * If enable == 0, transition ACI_ON -> ACI_OFF
	  */
	if (NREV_LE(pi->pubpi->phy_rev, 15)) {
		if (aci_enable == 0) {

			pi->interf->bphy_min_sensitivity = NPHY_BPHY_MIN_SENSITIVITY_REV7TO15;
			pi->interf->ofdm_min_sensitivity = NPHY_OFDM_MIN_SENSITIVITY_REV7TO15;

			pi->interf->bphy_desense_lut = NPHY_bphy_desense_aci_off_lut_rev7to15;
			pi->interf->bphy_desense_lut_size =
				sizeof(NPHY_bphy_desense_aci_off_lut_rev7to15)/
				sizeof(bphy_desense_info_t);
		} else {

			pi->interf->bphy_min_sensitivity = NPHY_BPHY_MIN_SENSITIVITY_REV7TO15 -
				NPHY_DELTA_MIN_SENSITIVITY_ACI_ON_OFF_REV7TO15;
			pi->interf->ofdm_min_sensitivity = NPHY_OFDM_MIN_SENSITIVITY_REV7TO15 -
				NPHY_DELTA_MIN_SENSITIVITY_ACI_ON_OFF_REV7TO15;

			pi->interf->bphy_desense_lut = NPHY_bphy_desense_aci_on_lut_rev7to15;
			pi->interf->bphy_desense_lut_size =
				sizeof(NPHY_bphy_desense_aci_on_lut_rev7to15)/
				sizeof(bphy_desense_info_t);
		}
	}

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, pi->pubpi->phy_corenum, 0x106, 16,
		&(pi->interf->noise.save_initgain_rfseq[0]));

}
#endif /* WLC_DISABLE_ACI */

static void
wlc_phy_savecal_nphy(phy_info_t *pi)
{
	void *tbl_ptr;
	int coreNum;
	uint16 *txcal_radio_regs = NULL;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Save Rx calibration values */
		wlc_phy_rx_iq_coeffs_nphy(pi, 0, &pi_nphy->nphy_calibration_cache.rxcal_coeffs_2G);

		txcal_radio_regs = pi_nphy->nphy_calibration_cache.txcal_radio_regs_2G;

		pi_nphy->nphy_iqcal_chanspec_2G = pi->radio_chanspec;
		tbl_ptr = pi_nphy->nphy_calibration_cache.txcal_coeffs_2G;
	} else {
		/* Save Rx calibration values */
		wlc_phy_rx_iq_coeffs_nphy(pi, 0, &pi_nphy->nphy_calibration_cache.rxcal_coeffs_5G);

		txcal_radio_regs = pi_nphy->nphy_calibration_cache.txcal_radio_regs_5G;

		pi_nphy->nphy_iqcal_chanspec_5G = pi->radio_chanspec;
		tbl_ptr = pi_nphy->nphy_calibration_cache.txcal_coeffs_5G;
	}

	FOREACH_CORE(pi, coreNum) {
		/* Fine LOFT compensation */
		txcal_radio_regs[2*coreNum] =
		        READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_I);
		txcal_radio_regs[2*coreNum+1] =
		        READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_Q);

		/* Coarse LOFT compensation */
		txcal_radio_regs[2*coreNum+4] =
		        READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_I);
		txcal_radio_regs[2*coreNum+5] =
		        READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_Q);
	}

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 80, 16, tbl_ptr);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static void
wlc_phy_restorecal_nphy(phy_info_t *pi)
{
	uint16 *loft_comp;
	uint16 txcal_coeffs_bphy[4];
	uint16 *tbl_ptr;
	int coreNum;
	uint16 *txcal_radio_regs = NULL;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* WARNING: Since this function is only called by wlc_nphy_init()
	   don't force the PHY to carrier search mode, even for NREV >= 3.
	   Doing so causes init problems.

	   However, in future if this function is called during non-init
	   time, ensure that the PHY is force to carrier search state for
	   NREV >= 3
	*/

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (pi_nphy->nphy_iqcal_chanspec_2G == 0)
			return;

		tbl_ptr = pi_nphy->nphy_calibration_cache.txcal_coeffs_2G;
		loft_comp = &pi_nphy->nphy_calibration_cache.txcal_coeffs_2G[5];
	} else {
		if (pi_nphy->nphy_iqcal_chanspec_5G == 0)
			return;

		tbl_ptr = pi_nphy->nphy_calibration_cache.txcal_coeffs_5G;
		loft_comp = &pi_nphy->nphy_calibration_cache.txcal_coeffs_5G[5];
	}

	/* Write IQ/LO compensation values for OFDM PHY */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 4, 80, 16, (void *)tbl_ptr);

	/* Write IQ compensation values for B-PHY */
	txcal_coeffs_bphy[0] = tbl_ptr[0];
	txcal_coeffs_bphy[1] = tbl_ptr[1];
	txcal_coeffs_bphy[2] = tbl_ptr[2];
	txcal_coeffs_bphy[3] = tbl_ptr[3];

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 4, 88, 16, txcal_coeffs_bphy);

	/* Write LO compensation values for OFDM PHY */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 85, 16, loft_comp);
	/* Write LO compensation values for B-PHY */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 93, 16, loft_comp);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		txcal_radio_regs = pi_nphy->nphy_calibration_cache.txcal_radio_regs_2G;

		/* Restore Rx calibration values */
		wlc_phy_rx_iq_coeffs_nphy(pi, 1, &pi_nphy->nphy_calibration_cache.rxcal_coeffs_2G);
	} else {
		txcal_radio_regs = pi_nphy->nphy_calibration_cache.txcal_radio_regs_5G;

		/* Restore Rx calibration values */
		wlc_phy_rx_iq_coeffs_nphy(pi, 1, &pi_nphy->nphy_calibration_cache.rxcal_coeffs_5G);
	}

	FOREACH_CORE(pi, coreNum) {
		/* Fine LOFT compensation */
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_I,
		                 txcal_radio_regs[2*coreNum]);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_Q,
		                 txcal_radio_regs[2*coreNum+1]);

		/* Coarse LOFT compensation */
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_I,
		                 txcal_radio_regs[2*coreNum+4]);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_Q,
		                 txcal_radio_regs[2*coreNum+5]);
	}
}

uint16
wlc_phy_classifier_nphy(phy_info_t *pi, uint16 mask, uint16 val)
{
	uint16 curr_ctl, new_ctl;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Turn on/off classification (bphy, ofdm, and wait_ed), mask and
	 * val are bit fields, bit 0: bphy, bit 1: ofdm, bit 2: wait_ed;
	 * for types corresponding to bits set in mask, apply on/off state
	 * from bits set in val; if no bits set in mask, simply returns
	 * current on/off state.
	 */

	curr_ctl = phy_utils_read_phyreg(pi, NPHY_ClassifierCtrl) &
		NPHY_ClassifierCtrl_classifierSel_MASK;

	new_ctl = (curr_ctl & (~mask)) | (val & mask);

	phy_utils_mod_phyreg(pi, NPHY_ClassifierCtrl,
	                        NPHY_ClassifierCtrl_classifierSel_MASK, new_ctl);

	return new_ctl;
}

void
wlc_phy_clip_det_nphy(phy_info_t *pi, uint8 write, uint16 *vals)
{
	/* Make clip detection difficult (impossible?) */

	if (write == 0) {
		vals[0] = phy_utils_read_phyreg(pi, NPHY_Core1Clip1Threshold);
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
			vals[1] = phy_utils_read_phyreg(pi, NPHY_Core2Clip1Threshold);
	} else {
		phy_utils_write_phyreg(pi, NPHY_Core1Clip1Threshold, vals[0]);
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
			phy_utils_write_phyreg(pi, NPHY_Core2Clip1Threshold, vals[1]);
	}
}

void
wlc_phy_force_rfseq_nphy(phy_info_t *pi, uint8 cmd)
{
	uint16 trigger_mask, status_mask;
	uint16 orig_RfseqCoreActv;

	switch (cmd) {
	case NPHY_RFSEQ_RX2TX:
		trigger_mask = NPHY_RfseqTrigger_rx2tx;
		status_mask = NPHY_RfseqStatus_rx2tx;
		break;
	case NPHY_RFSEQ_TX2RX:
		trigger_mask = NPHY_RfseqTrigger_tx2rx;
		status_mask = NPHY_RfseqStatus_tx2rx;
		break;
	case NPHY_RFSEQ_RESET2RX:
		trigger_mask = NPHY_RfseqTrigger_reset2rx;
		status_mask = NPHY_RfseqStatus_reset2rx;
		break;
	case NPHY_RFSEQ_UPDATEGAINH:
		trigger_mask = NPHY_RfseqTrigger_updategainh;
		status_mask = NPHY_RfseqStatus_updategainh;
		break;
	case NPHY_RFSEQ_UPDATEGAINL:
		trigger_mask = NPHY_RfseqTrigger_updategainl;
		status_mask = NPHY_RfseqStatus_updategainl;
		break;
	case NPHY_RFSEQ_UPDATEGAINU:
		trigger_mask = NPHY_RfseqTrigger_updategainu;
		status_mask = NPHY_RfseqStatus_updategainu;
		break;
	case NPHY_RFSEQ_OCLRESET2RX:
		trigger_mask = NPHY_RfseqTrigger_oclreset2rx;
		status_mask = NPHY_RfseqStatus_oclreset2rx;
		break;
	default:
		PHY_ERROR(("wlc_phy_force_rfseq_nphy: unrecognized command.\n"));
		return;
	}

	orig_RfseqCoreActv = phy_utils_read_phyreg(pi, NPHY_RfseqMode);
	phy_utils_or_phyreg(pi, NPHY_RfseqMode,
	           (NPHY_RfseqMode_CoreActv_override | NPHY_RfseqMode_Trigger_override));
	phy_utils_or_phyreg(pi, NPHY_RfseqTrigger, trigger_mask);
	SPINWAIT((phy_utils_read_phyreg(pi, NPHY_RfseqStatus) & status_mask),
		NPHY_SPINWAIT_FORCE_RFSEQ_STATUS);
	phy_utils_write_phyreg(pi, NPHY_RfseqMode, orig_RfseqCoreActv);

	ASSERT((phy_utils_read_phyreg(pi, NPHY_RfseqStatus) & status_mask) == 0);
}

void
wlc_phy_set_rfseq_nphy(phy_info_t *pi, uint8 cmd, uint8 *events, uint8 *dlys, uint8 len)
{
	uint32 t1_offset, t2_offset;
	uint8 ctr;
	uint8 end_event = NPHY_REV3_RFSEQ_CMD_END;
	uint8 end_dly   = 1;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	ASSERT(len <= 16);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	t1_offset = cmd << 4;
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, len, t1_offset, 8, events);

	if ((cmd == NPHY_RFSEQ_OCLWAKEONED) ||
		(cmd == NPHY_RFSEQ_OCLWAKEONCLIP) ||
		(cmd == NPHY_RFSEQ_OCLWAKEONCRSO) ||
		(cmd == NPHY_RFSEQ_OCLWAKEONCRSC) ||
		(cmd == NPHY_RFSEQ_OCLSHUTOFF) ||
		(cmd == NPHY_RFSEQ_SCDSHUTOFF) ||
		(cmd == NPHY_RFSEQ_TX2OCLRX) ||
		(cmd == NPHY_RFSEQ_RESET2OCLRX) ||
		(cmd == NPHY_RFSEQ_OCLWAKEONRIFS)) {
		t2_offset = t1_offset + 0x090;
	} else {
		t2_offset = t1_offset + 0x080;
	}
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, len, t2_offset, 8, dlys);

	for (ctr = len; ctr < 16; ctr++) {
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, t1_offset + ctr, 8, &end_event);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, t2_offset + ctr, 8, &end_dly);
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

/**
 * This function is used for MIMOPHY Revs 7 and 8
 * For the current bandwidth setting, read the value of lpf_bw_ctl from the RFSeq table
 * For 20 MHz bandwidth, the setting for 11n 20in20 mode is read (from rx2tx_lpf_rc_lut_tx20_11n)
 * For 40 MHz bandwidth, the setting for 11n 40in40 mode is read (from rx2tx_lpf_rc_lut_tx40_11n)
 */
uint16 wlc_phy_read_lpf_bw_ctl_nphy(phy_info_t *pi, uint16 offset)
{
	uint16 lpf_bw_ctl_val = 0;
	uint16 rx2tx_lpf_rc_lut_offset = 0;

	/* Read only core 0's value assuming that the values for core 0 and core 1 are indentical */
	if (offset == 0) {
		if (CHSPEC_IS40(pi->radio_chanspec)) {
			rx2tx_lpf_rc_lut_offset = 0x159;
		} else {
			rx2tx_lpf_rc_lut_offset = 0x154;
		}
	} else {
		rx2tx_lpf_rc_lut_offset = offset;
	}
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, (uint32)rx2tx_lpf_rc_lut_offset, 16,
	                    &lpf_bw_ctl_val);

	/* Only the LSB 3 bits of the RFSeq register correspond to lpf_bw_ctl. So apply a mask to
	 * obtain the value of lpf_bw_ctl
	 */
	lpf_bw_ctl_val = lpf_bw_ctl_val & 0x7;

	return lpf_bw_ctl_val;
}

extern void
wlc_phy_rfctrl_override_nphy_rev7(phy_info_t *pi, uint16 field, uint16 value, uint8 core_mask,
                                  uint8 off, uint8 override_id)
{
	uint8  core_num;
	uint16 addr = 0, en_addr = 0, val_addr = 0, en_mask = 0, val_mask = 0;
	uint8  val_shift = 0;

	en_mask = field;
	FOREACH_CORE(pi, core_num) {
		switch (override_id) {
		case NPHY_REV7_RFCTRLOVERRIDE_ID0:
			/* Collect all settings of RfctrlOverride[0,1] here */
			en_addr   = (core_num == 0) ? NPHY_RfctrlOverride0 :
			        NPHY_RfctrlOverride1;
			val_addr  = (core_num == 0) ? NPHY_RfctrlRSSIOTHERS1 :
				NPHY_RfctrlRSSIOTHERS2;
			switch (field) {
			case NPHY_REV7_RfctrlOverride_tx_pu_MASK:
				val_mask  = NPHY_REV7_RfctrlRSSIOTHERS_tx_pu_MASK;
				val_shift = NPHY_REV7_RfctrlRSSIOTHERS_tx_pu_SHIFT;
				if (ISNPHY(pi) && D11REV_IS(pi->sh->corerev, 24)) {
					uint16 rfovrd = 0;
					rfovrd  = phy_utils_read_phyreg(pi,
						NPHY_RfctrlOverride0)
					        & NPHY_REV7_RfctrlOverride_tx_pu_MASK;
					rfovrd |= phy_utils_read_phyreg(pi,
						NPHY_RfctrlOverride1)
					        & NPHY_REV7_RfctrlOverride_tx_pu_MASK;

					if (rfovrd)
						return;
				}
				break;
			case NPHY_REV7_RfctrlOverride_intpa_pu_MASK:
				val_mask  = NPHY_REV7_RfctrlRSSIOTHERS_intpa_pu_MASK;
				val_shift = NPHY_REV7_RfctrlRSSIOTHERS_intpa_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rssi_wb1a_pu_MASK:
				val_mask  = NPHY_REV7_RfctrlRSSIOTHERS_rssi_wb1a_pu_MASK;
				val_shift = NPHY_REV7_RfctrlRSSIOTHERS_rssi_wb1a_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rssi_wb1g_pu_MASK:
				val_mask  = NPHY_REV7_RfctrlRSSIOTHERS_rssi_wb1g_pu_MASK;
				val_shift = NPHY_REV7_RfctrlRSSIOTHERS_rssi_wb1g_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rssi_wb2_pu_MASK:
				val_mask  = NPHY_REV7_RfctrlRSSIOTHERS_rssi_wb2_pu_MASK;
				val_shift = NPHY_REV7_RfctrlRSSIOTHERS_rssi_wb2_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rssi_nb_pu_MASK:
				val_mask  = NPHY_REV7_RfctrlRSSIOTHERS_rssi_nb_pu_MASK;
				val_shift = NPHY_REV7_RfctrlRSSIOTHERS_rssi_nb_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_hpc_MASK:
				val_addr =  (core_num == 0) ? NPHY_RfctrlAuxReg1 :
				        NPHY_RfctrlAuxReg2;
				val_mask = NPHY_REV7_RfctrlAuxReg_Rfctrl_lpf_hpc_MASK;
				val_shift = NPHY_REV7_RfctrlAuxReg_Rfctrl_lpf_hpc_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rxgain_MASK:
				val_addr  = (core_num == 0) ? NPHY_RfctrlRXGAIN1 :
				        NPHY_RfctrlRXGAIN2;
				val_mask  = NPHY_REV7_RfctrlRXGAIN_rxgain_MASK;
				val_shift = NPHY_REV7_RfctrlRXGAIN_rxgain_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_txgain_MASK:
				val_addr  = (core_num == 0) ? NPHY_RfctrlTXGAIN1 :
				        NPHY_RfctrlTXGAIN2;
				val_mask  = NPHY_REV7_RfctrlTXGAIN_txgain_MASK;
				val_shift = NPHY_REV7_RfctrlTXGAIN_txgain_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_gain_biq01_MASK:
				val_addr  = (core_num == 0) ? NPHY_Rfctrl_lpf_gain0 :
				        NPHY_Rfctrl_lpf_gain1;
				val_mask = NPHY_Rfctrl_lpf_gain_MASK;
				val_shift = NPHY_Rfctrl_lpf_gain_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_gain_biq0_MASK:
				val_addr  = (core_num == 0) ? NPHY_Rfctrl_lpf_gain0 :
				        NPHY_Rfctrl_lpf_gain1;
				val_mask = NPHY_Rfctrl_lpf_gain_lpf_gain_biq0_MASK;
				val_shift = NPHY_Rfctrl_lpf_gain_lpf_gain_biq0_SHIFT;
				break;
			default:
				addr = 0xffff;
				break;
			}
			break;
		case NPHY_REV7_RFCTRLOVERRIDE_ID1 :
			/* Collect all settings of RfctrlOverride[3,4] here */
			en_addr = (core_num == 0) ? NPHY_REV7_RfctrlOverride3 :
				NPHY_REV7_RfctrlOverride4;
			val_addr = (core_num == 0) ? NPHY_REV7_RfctrlMiscReg3 :
				NPHY_REV7_RfctrlMiscReg4;
			switch (field) {
			case NPHY_REV7_RfctrlOverride_lpf_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rxmx_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_rxmx_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_rxmx_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lna1_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lna1_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lna1_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lna2_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lna2_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lna2_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rxif_pu_MASK:
				/* In tcl, rxif_pu = rxif_nolpf_pu */
				val_mask = NPHY_REV7_RfctrlMiscReg_rxif_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_rxif_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK:
				/* In tcl, lpf_bw, tx_lpf_bw, rx_lpf_bw
				   all mapped to field lpf_bw_ctl
				*/
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_bw_ctl_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_bw_ctl_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_sel_txrx_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_sel_txrx_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_sel_txrx_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_byp_tx_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_byp_tx_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_byp_tx_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_byp_rx_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_byp_rx_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_byp_rx_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_byp_dc_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_byp_dc_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_byp_dc_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_q_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_q_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_q_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_dc_loop_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_dc_loop_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_dc_loop_pu_SHIFT;
				break;
			default:
				addr = 0xffff;
				break;
			}
			break;
		case NPHY_REV7_RFCTRLOVERRIDE_ID2 :
			/* Collect all settings of RfctrlOverride[5,6] here */
			en_addr = (core_num == 0) ? NPHY_REV7_RfctrlOverride5 :
				NPHY_REV7_RfctrlOverride6;
			val_addr = (core_num == 0) ? NPHY_REV7_RfctrlMiscReg5 :
				NPHY_REV7_RfctrlMiscReg6;
			switch (field) {
			case NPHY_REV7_RfctrlOverride_lpf_byp_dacbuf_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_byp_dacbuf_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_byp_dacbuf_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_rx_buf_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_rx_buf_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_rx_buf_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_tx_buf_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_tx_buf_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_tx_buf_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_lpf_dacbuf_pu_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_lpf_dacbuf_pu_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_lpf_dacbuf_pu_SHIFT;
				break;
			case NPHY_REV7_RfctrlOverride_rc_cal_ovr_en_MASK:
				val_mask = NPHY_REV7_RfctrlMiscReg_rc_cal_ovr_en_MASK;
				val_shift = NPHY_REV7_RfctrlMiscReg_rc_cal_ovr_en_SHIFT;
				break;
			default:
				addr = 0xffff;
				break;
			}
			break;
		case NPHY_REV7_RFCTRLOVERRIDEAUX_ID0 :
			/* Collect all settings of RfctrlOverrideAux[0,1] here */
			en_addr = (core_num == 0) ? NPHY_RfctrlOverrideAux0 :
				NPHY_RfctrlOverrideAux1;
			val_addr = (core_num == 0) ? NPHY_RfctrlAuxReg1 :
				NPHY_RfctrlAuxReg2;
			switch (field) {
			case  NPHY_REV7_RfctrlOverride_tx_bias_reset_MASK :
				val_mask = NPHY_REV7_RfctrlAuxReg_tx_bias_reset_MASK;
				val_shift = NPHY_REV7_RfctrlAuxReg_tx_bias_reset_SHIFT;
				break;
			default:
				addr = 0xffff;
				break;
			}
			break;
		default:
			addr = 0xffff;
			break;
		} /* end of switch(override_id) */

		/* Perform actual overrides */
		if (off) {
			if (field == NPHY_REV7_RfctrlOverride_tx_pu_MASK) {
				if ((RADIOREV(pi->pubpi->radiorev) == 4) ||
				    (RADIOREV(pi->pubpi->radiorev) == 6)) {
					PHY_ERROR(("WARNING: 2057 radio revs 4 and 6 "
					           "require tx_pu to always be forced "
					           "ON because of PR 74381\n"));
				}
			}
			phy_utils_and_phyreg(pi, en_addr,  ~en_mask);
			phy_utils_and_phyreg(pi, val_addr, ~val_mask);
		} else {
			/* set appropriate override enable(s)
			 * one caveat: core_mask == 0 also means both cores
			 */
			if ((core_mask == 0) || (core_mask & (1 << core_num))) {
				phy_utils_or_phyreg(pi, en_addr, en_mask);
				/* perform actual override value reg write */
				if (addr != 0xffff) {
					phy_utils_mod_phyreg(pi, val_addr, val_mask,
					            (value << val_shift));
				}
			}
		}
	} /* for (core_num = 0; core_num < 2; core_num++) */
}

extern void
wlc_phy_rfctrl_override_nphy(phy_info_t *pi, uint16 field, uint16 value, uint8 core_mask,
                             uint8 off)
{
	uint8  core_num;
	uint16 addr = 0, mask = 0;
	uint8  shift = 0;

	/* set/clear appropriate override enable */
	if (off) {
		phy_utils_and_phyreg(pi, NPHY_RfctrlOverride, ~field);
		value = 0x0;
	} else {
		phy_utils_or_phyreg(pi, NPHY_RfctrlOverride, field);
	}

	FOREACH_CORE(pi, core_num) {
		/* setup phyreg address for override value, (different
		 * override values are located in different registers)
		 */
		switch (field) {
		case NPHY_RfctrlOverride_core_sel_MASK:
		case NPHY_RfctrlOverride_RxOrTxn_MASK:
		case NPHY_RfctrlOverride_rxen_MASK:
		case NPHY_RfctrlOverride_txen_MASK:
		case NPHY_RfctrlOverride_seqen_core_MASK:
			addr = NPHY_RfctrlCmd;
			/* hard wire core_mask to 1, these overrides are not per-core */
			core_mask = 0x1;
			break;
		case NPHY_RfctrlOverride_rx_pd_MASK:
		case NPHY_RfctrlOverride_tx_pd_MASK:
		case NPHY_RfctrlOverride_pa_pd_MASK:
		case NPHY_RfctrlOverride_rssi_ctrl_MASK:
		case NPHY_RfctrlOverride_lpf_bw_MASK:
		case NPHY_RfctrlOverride_hpf_bw_hi_MASK:
		case NPHY_RfctrlOverride_hiq_dis_core_MASK:
			addr = (core_num == 0) ? NPHY_RfctrlRSSIOTHERS1 :
				NPHY_RfctrlRSSIOTHERS2;
			break;
		case NPHY_RfctrlOverride_rxgain_MASK:
			addr = (core_num == 0) ? NPHY_RfctrlRXGAIN1 : NPHY_RfctrlRXGAIN2;
			break;
		case NPHY_RfctrlOverride_txgain_MASK:
			addr = (core_num == 0) ? NPHY_RfctrlTXGAIN1 : NPHY_RfctrlTXGAIN2;
			break;
		default:
			addr = 0xffff;
		}

		/* setup phyreg mask for override value, (different override
		 * values are located at different masks in different
		 * registers)
		 */
		switch (field) {
		case NPHY_RfctrlOverride_core_sel_MASK:
			mask = NPHY_RfctrlCmd_core_sel_MASK;
			shift = NPHY_RfctrlCmd_core_sel_SHIFT;
			break;
		case NPHY_RfctrlOverride_RxOrTxn_MASK:
			mask = NPHY_RfctrlCmd_RxOrTxn_MASK;
			shift = NPHY_RfctrlCmd_RxOrTxn_SHIFT;
			break;
		case NPHY_RfctrlOverride_rxen_MASK:
			mask = NPHY_RfctrlCmd_rxen_MASK;
			shift = NPHY_RfctrlCmd_rxen_SHIFT;
			break;
		case NPHY_RfctrlOverride_txen_MASK:
			mask = NPHY_RfctrlCmd_txen_MASK;
			shift = NPHY_RfctrlCmd_txen_SHIFT;
			break;
		case NPHY_RfctrlOverride_seqen_core_MASK:
			mask = NPHY_RfctrlCmd_seqen_core_MASK;
			shift = NPHY_RfctrlCmd_seqen_core_SHIFT;
			break;
		case NPHY_RfctrlOverride_rx_pd_MASK:
			mask = NPHY_RfctrlRSSIOTHERS_rx_pd_MASK;
			shift = NPHY_RfctrlRSSIOTHERS_rx_pd_SHIFT;
			break;
		case NPHY_RfctrlOverride_tx_pd_MASK:
			mask = NPHY_RfctrlRSSIOTHERS_tx_pd_MASK;
			shift = NPHY_RfctrlRSSIOTHERS_tx_pd_SHIFT;
			break;
		case NPHY_RfctrlOverride_pa_pd_MASK:
			mask = NPHY_RfctrlRSSIOTHERS_pa_pd_MASK;
			shift = NPHY_RfctrlRSSIOTHERS_pa_pd_SHIFT;
			break;
		case NPHY_RfctrlOverride_rssi_ctrl_MASK:
			mask = NPHY_RfctrlRSSIOTHERS_rssi_ctrl_MASK;
			shift = NPHY_RfctrlRSSIOTHERS_rssi_ctrl_SHIFT;
			break;
		case NPHY_RfctrlOverride_lpf_bw_MASK:
			mask = NPHY_RfctrlRSSIOTHERS_lpf_bw_MASK;
			shift = NPHY_RfctrlRSSIOTHERS_lpf_bw_SHIFT;
			break;
		case NPHY_RfctrlOverride_hpf_bw_hi_MASK:
			mask = NPHY_RfctrlRSSIOTHERS_hpf_bw_hi_MASK;
			shift = NPHY_RfctrlRSSIOTHERS_hpf_bw_hi_SHIFT;
			break;
		case NPHY_RfctrlOverride_hiq_dis_core_MASK:
			mask = NPHY_RfctrlRSSIOTHERS_hiq_dis_core_MASK;
			shift = NPHY_RfctrlRSSIOTHERS_hiq_dis_core_SHIFT;
			break;
		case NPHY_RfctrlOverride_rxgain_MASK:
			mask = 0x1fff;
			shift = 0x0;
			break;
		case NPHY_RfctrlOverride_txgain_MASK:
			mask = 0x1fff;
			shift = 0x0;
			break;
		default:
			mask = 0x0;
			shift = 0x0;
			break;
		}

		/* perform actual override value reg write */
		if ((addr != 0xffff) && (core_mask & (1 << core_num))) {
			phy_utils_mod_phyreg(pi, addr, mask, (value << shift));
		}
	}

	PHY_REG_LIST_START
		PHY_REG_OR_ENTRY(NPHY, RfctrlOverride, NPHY_RfctrlOverride_trigger_MASK)
		PHY_REG_OR_ENTRY(NPHY, RfctrlCmd, NPHY_RfctrlCmd_startseq_MASK)
	PHY_REG_LIST_EXECUTE(pi);

	OSL_DELAY(1);
	phy_utils_and_phyreg(pi, NPHY_RfctrlOverride, ~NPHY_RfctrlOverride_trigger_MASK);
}

static void
wlc_phy_rfctrl_override_1tomany_nphy(phy_info_t *pi, uint16 cmd, uint16 value, uint8 core_mask,
                                     uint8 off)
{
	uint16 rfmxgain = 0, lpfgain = 0;
	uint16 tgain = 0;

	/* 1-to-many rfctrl override commands */
	switch (cmd) {
	case NPHY_REV7_RfctrlOverride_cmd_rxrf_pu:
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lna1_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lna2_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rxmx_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		break;
	case NPHY_REV7_RfctrlOverride_cmd_rx_pu:
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rxif_nolpf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_dc_loop_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_rx_buf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID2);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_sel_txrx_MASK,
			0, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		break;
	case NPHY_REV7_RfctrlOverride_cmd_tx_pu:
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_tx_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_tx_buf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID2);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_dacbuf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID2);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_sel_txrx_MASK,
			1, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		break;
	case NPHY_REV7_RfctrlOverride_cmd_rxgain:
		rfmxgain = value & NPHY_REV7_RXGAINCODE_RFMXGAIN_MASK;
		lpfgain = value & NPHY_REV7_RXGAINCODE_LPFGAIN_MASK;
		lpfgain = lpfgain >> 8;
		/* dvgagain = value & NPHY_REV7_RXGAINCODE_DVGAGAIN_MASK 0xf0000 */
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rxgain_MASK,
			rfmxgain, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_gain_biq01_MASK,
			lpfgain, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		break;
	case NPHY_REV7_RfctrlOverride_cmd_txgain:
		tgain = value & NPHY_REV7_TXGAINCODE_TGAIN_MASK;
		lpfgain = value & NPHY_REV7_TXGAINCODE_LPFGAIN_MASK;
		lpfgain = lpfgain >> NPHY_REV7_TXGAINCODE_BIQ0GAIN_SHIFT;

		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
			tgain, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_gain_biq0_MASK,
			lpfgain, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		break;
	case NPHY_REV19_RfctrlOverride_cmd_rx_pu:
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV19_RfctrlOverride_lpf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev19(pi,
			NPHY_REV19_RfctrlOverride_dc_loop_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev19(pi,
			NPHY_REV7_RfctrlOverride_lpf_rx_buf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID2);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV19_RfctrlOverride_lpf_bias_pu_MASK,
			value, core_mask, off, NPHY_REV19_RFCTRLOVERRIDE_ID3);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV19_RfctrlOverride_lpf_sel_txrx_MASK,
			0, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		break;

	case NPHY_REV19_RfctrlOverride_cmd_tx_pu:
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV19_RfctrlOverride_lpf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_tx_buf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID2);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_dacbuf_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID2);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV19_RfctrlOverride_lpf_bias_pu_MASK,
			value, core_mask, off, NPHY_REV19_RFCTRLOVERRIDE_ID3);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV19_RfctrlOverride_lpf_sel_txrx_MASK,
			1, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		break;
	case NPHY_REV19_RfctrlOverride_cmd_rxradio_pu:
		wlc_phy_rfctrl_override_nphy_rev19(
			pi, NPHY_REV19_RfctrlOverride_lna1_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev19(
			pi, NPHY_REV19_RfctrlOverride_lna2_pu_MASK,
			value, core_mask, off, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		wlc_phy_rfctrl_override_nphy_rev19(
			pi, NPHY_REV19_RfctrlOverride_lna1_5G_pu_MASK,
			value, core_mask, off, NPHY_REV19_RFCTRLOVERRIDE_ID3);
		wlc_phy_rfctrl_override_nphy_rev19(
			pi, NPHY_REV19_RfctrlOverride_logen_pwrup_MASK,
			value, core_mask, off, NPHY_REV19_RFCTRLOVERRIDE_ID3);
		wlc_phy_rfctrl_override_nphy_rev19(
			pi, NPHY_REV19_RfctrlOverride_logen_rx_pwrup_MASK,
			value, core_mask, off, NPHY_REV19_RFCTRLOVERRIDE_ID3);
		wlc_phy_rfctrl_override_nphy_rev19(
			pi, NPHY_REV3_RfctrlOverrideAux_rxrf_pu_MASK,
			value, core_mask, off, NPHY_REV19_RFCTRLOVERRIDE_ID10);
		break;
	}
}

static void
wlc_phy_scale_offset_rssi_nphy(phy_info_t *pi, uint16 scale, int8 offset, uint8 coresel,
                               uint8 rail, uint8 rssi_type)
{
	uint16 valuetostuff;

	/* core can be:  0 or 1
	 * type can be: nbd, w1, w2, pwrdet, tssi2g, or tssi5g
	 * scaleval can be a floating point value from [1/2 to 95/64]
	 * offset can be: signed, 6 bits, so value can be -32 to 31 integer
	 */

	/* clip to 6 bit range, i.e. to [-32..+31] */
	offset = (offset > NPHY_RSSICAL_MAXREAD) ?
		NPHY_RSSICAL_MAXREAD : offset;
	offset = (offset < (-NPHY_RSSICAL_MAXREAD-1)) ?
		-NPHY_RSSICAL_MAXREAD-1 : offset;

	/* stuff both values into the right locations of register */
	valuetostuff = ((scale & 0x3f) << 8) | (offset & 0x3f);

	/* nbd */
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_NB)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIZ, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_NB)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIZ, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_NB)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIZ, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_NB)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIZ, valuetostuff);
	}

	/* w1 */
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_W1)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIX, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_W1)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIX, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_W1)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIX, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_W1)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIX, valuetostuff);
	}

	/* w2 */
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_W2)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIY, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_W2)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIY, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_W2)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIY, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_W2)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIY, valuetostuff);
	}

	/* tbd */
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_TBD)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0ITBD, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_TBD)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QTBD, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_TBD)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1ITBD, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_TBD)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QTBD, valuetostuff);
	}

	/* pwrdet */
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_IQ)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IPowerDet, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_IQ)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QPowerDet, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_I) && (rssi_type == NPHY_RSSI_SEL_IQ)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IPowerDet, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rail == NPHY_RAIL_Q) && (rssi_type == NPHY_RSSI_SEL_IQ)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QPowerDet, valuetostuff);
	}

	/* tssi2g */
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rssi_type == NPHY_RSSI_SEL_TSSI_2G)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0ITSSI, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rssi_type == NPHY_RSSI_SEL_TSSI_2G)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1ITSSI, valuetostuff);
	}

	/* tssi5g */
	if (((coresel == RADIO_MIMO_CORESEL_CORE1) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rssi_type == NPHY_RSSI_SEL_TSSI_5G)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QTSSI, valuetostuff);
	}
	if (((coresel == RADIO_MIMO_CORESEL_CORE2) ||
	     (coresel == RADIO_MIMO_CORESEL_ALLRX)) &&
	    (rssi_type == NPHY_RSSI_SEL_TSSI_5G)) {
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QTSSI, valuetostuff);
	}
}

void
wlc_phy_rssisel_nphy(phy_info_t *pi, uint8 core_code, uint8 rssi_type)
{
	uint16 mask, val;
	uint16 afectrlovr_rssi_val;
	uint8  core;

	/* core_code is RADIO_MIMO_CORESEL_[OFF,CORE1,CORE2,CORE3,CORE4,ALLRX,ALLTX,ALLRXTX]
	 * rssi_type is NPHY_RSSI_SEL_[W1,W2,NB,IQ,TSSI]
	 */
	if (core_code == RADIO_MIMO_CORESEL_OFF) {
		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_rssi_select_i_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_rssi_select_i_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_rssi_select_i_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_rssi_select_i_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlOverrideAux0,
				NPHY_REV3_RfctrlOverrideAux_rssi_ctrl_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlOverrideAux1,
				NPHY_REV3_RfctrlOverrideAux_rssi_ctrl_MASK, 0)
			/* force 2056 rssi_sel lines & strobe */
			PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlMiscReg1,
				NPHY_RfctrlMiscReg_rssi_wb1a_sel_MASK |
				NPHY_RfctrlMiscReg_rssi_wb1g_sel_MASK |
				NPHY_RfctrlMiscReg_rssi_wb2_sel_MASK |
				NPHY_RfctrlMiscReg_rssi_nb_sel_MASK, 0)
			PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlMiscReg2,
				NPHY_RfctrlMiscReg_rssi_wb1a_sel_MASK |
				NPHY_RfctrlMiscReg_rssi_wb1g_sel_MASK |
				NPHY_RfctrlMiscReg_rssi_wb2_sel_MASK |
				NPHY_RfctrlMiscReg_rssi_nb_sel_MASK, 0)
		PHY_REG_LIST_EXECUTE(pi);

	} else {
		FOREACH_CORE(pi, core) {
			if (core_code == RADIO_MIMO_CORESEL_CORE1 && core == PHY_CORE_1)
				continue;
			else if (core_code == RADIO_MIMO_CORESEL_CORE2 &&
			         core == PHY_CORE_0)
				continue;

			phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
			            NPHY_AfectrlOverride1 : NPHY_AfectrlOverride2,
			            NPHY_REV3_AfectrlOverride_rssi_select_i_MASK,
			            1 << NPHY_REV3_AfectrlOverride_rssi_select_i_SHIFT);

			if (rssi_type == NPHY_RSSI_SEL_W1 ||
			    rssi_type == NPHY_RSSI_SEL_W2 ||
			    rssi_type == NPHY_RSSI_SEL_NB) {

				/* Point AFE-Mux to rssi */
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
				                     NPHY_AfectrlCore1 :
				                     NPHY_AfectrlCore2,
				        NPHY_REV3_AfectrlCore_rssi_select_i_MASK, 0);
				/* force 2056 rssi_sel lines & strobe */
				mask = NPHY_RfctrlMiscReg_rssi_wb1a_sel_MASK |
				        NPHY_RfctrlMiscReg_rssi_wb1g_sel_MASK |
				        NPHY_RfctrlMiscReg_rssi_wb2_sel_MASK |
				        NPHY_RfctrlMiscReg_rssi_nb_sel_MASK;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
				                     NPHY_RfctrlMiscReg1 :
					NPHY_RfctrlMiscReg2, mask, 0);

				if (rssi_type == NPHY_RSSI_SEL_W1) {
					if (CHSPEC_IS5G(pi->radio_chanspec)) {
					mask = NPHY_RfctrlMiscReg_rssi_wb1a_sel_MASK;
					val = 1 << NPHY_RfctrlMiscReg_rssi_wb1a_sel_SHIFT;
					} else {
					mask = NPHY_RfctrlMiscReg_rssi_wb1g_sel_MASK;
					val = 1 << NPHY_RfctrlMiscReg_rssi_wb1g_sel_SHIFT;
					}
				} else if (rssi_type == NPHY_RSSI_SEL_W2) {
					mask = NPHY_RfctrlMiscReg_rssi_wb2_sel_MASK;
					val = 1 << NPHY_RfctrlMiscReg_rssi_wb2_sel_SHIFT;
				} else /* (rssi_type == NPHY_RSSI_SEL_NB) */ {
					mask = NPHY_RfctrlMiscReg_rssi_nb_sel_MASK;
					val = 1 << NPHY_RfctrlMiscReg_rssi_nb_sel_SHIFT;
				}
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
				                     NPHY_RfctrlMiscReg1 :
					NPHY_RfctrlMiscReg2, mask, val);

				mask = NPHY_REV3_RfctrlOverrideAux_rssi_ctrl_MASK;
				val = 1 << NPHY_REV3_RfctrlOverrideAux_rssi_ctrl_SHIFT;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
				            NPHY_RfctrlOverrideAux0 :
				            NPHY_RfctrlOverrideAux1, mask, val);
			} else {
				if (rssi_type == NPHY_RSSI_SEL_TBD) {
					/* force AFE rssi mux sel to "tbd" inputs */
					mask = NPHY_REV3_AfectrlCore_rssi_select_i_MASK;
					val = 1 <<
					        NPHY_REV3_AfectrlCore_rssi_select_i_SHIFT;
					phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
					            NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
					            mask, val);
					mask = NPHY_REV3_AfectrlCore_rssi_select_q_MASK;
					val = 1 <<
					        NPHY_REV3_AfectrlCore_rssi_select_q_SHIFT;
					phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
					            NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
					            mask, val);
				} else if (rssi_type == NPHY_RSSI_SEL_IQ) {
					/* force AFE rssi mux sel to
					 * 2056iq/pwr_det/temp_sense
					 */
					mask = NPHY_REV3_AfectrlCore_rssi_select_i_MASK;
					val = 2 <<
					        NPHY_REV3_AfectrlCore_rssi_select_i_SHIFT;
					phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
					            NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
					            mask, val);
					mask = NPHY_REV3_AfectrlCore_rssi_select_q_MASK;
					val = 2 <<
					        NPHY_REV3_AfectrlCore_rssi_select_q_SHIFT;
					phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
					            NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
					            mask, val);
				} else {
					/* force AFE rssi mux sel to board-PA TSSI */
					mask = NPHY_REV3_AfectrlCore_rssi_select_i_MASK;
					val = 3 <<
					        NPHY_REV3_AfectrlCore_rssi_select_i_SHIFT;
					phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
					            NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
					            mask, val);
					mask = NPHY_REV3_AfectrlCore_rssi_select_q_MASK;
					val = 3 <<
					        NPHY_REV3_AfectrlCore_rssi_select_q_SHIFT;
					phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
					            NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
					            mask, val);

					afectrlovr_rssi_val = 1 <<
					      NPHY_REV3_AfectrlOverride_rssi_select_i_SHIFT;
					phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
					            NPHY_AfectrlOverride1 :
					            NPHY_AfectrlOverride2,
					       NPHY_REV3_AfectrlOverride_rssi_select_i_MASK,
					            afectrlovr_rssi_val);
				}
			}
		}	/* for core */
	}	/* not "OFF" */
}

int
wlc_phy_poll_rssi_nphy(phy_info_t *pi, uint8 rssi_type, int32 *rssi_buf, uint8 nsamps)
{
	int16 rssi0, rssi1;
	uint16 afectrlCore1_save = 0;
	uint16 afectrlCore2_save = 0;
	uint16 afectrlOverride1_save = 0;
	uint16 afectrlOverride2_save = 0;
	uint16 rfctrlOverrideAux0_save = 0;
	uint16 rfctrlOverrideAux1_save = 0;
	uint16 rfctrlMiscReg1_save = 0;
	uint16 rfctrlMiscReg2_save = 0;
	int8 tmp_buf[4];
	uint8 ctr = 0, samp = 0;
	int32 rssi_out_val;

	afectrlCore1_save = phy_utils_read_phyreg(pi, NPHY_AfectrlCore1);
	afectrlCore2_save = phy_utils_read_phyreg(pi, NPHY_AfectrlCore2);
	rfctrlMiscReg1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlMiscReg1);
	rfctrlMiscReg2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlMiscReg2);
	afectrlOverride1_save = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride1);
	afectrlOverride2_save = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride2);
	rfctrlOverrideAux0_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverrideAux0);
	rfctrlOverrideAux1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverrideAux1);

	wlc_phy_rssisel_nphy(pi, RADIO_MIMO_CORESEL_ALLRX, rssi_type);

	for (ctr = 0; ctr < 4; ctr++) {
		rssi_buf[ctr] = 0;
	}

	for (samp = 0; samp < nsamps; samp++) {
		rssi0 = phy_utils_read_phyreg(pi, NPHY_RSSIVal1);
		rssi1 = phy_utils_read_phyreg(pi, NPHY_RSSIVal2);

		ctr = 0;
		tmp_buf[ctr++] = ((int8)((rssi0 & 0x3f) << 2)) >> 2;
		tmp_buf[ctr++] = ((int8)(((rssi0 >> 8) & 0x3f) << 2)) >> 2;
		tmp_buf[ctr++] = ((int8)((rssi1 & 0x3f) << 2)) >> 2;
		tmp_buf[ctr++] = ((int8)(((rssi1 >> 8) & 0x3f) << 2)) >> 2;

		for (ctr = 0; ctr < 4; ctr++) {
			rssi_buf[ctr] += tmp_buf[ctr];
		}

	}

	/* construct long word in the tcl-like order :  [ i_0 q_0 i_1 q_1 ] */
	rssi_out_val = rssi_buf[3] & 0xff;
	rssi_out_val |= (rssi_buf[2] & 0xff) << 8;
	rssi_out_val |= (rssi_buf[1] & 0xff) << 16;
	rssi_out_val |= (rssi_buf[0] & 0xff) << 24;

	/* Restore the saved registers */
	phy_utils_write_phyreg(pi, NPHY_AfectrlCore1, afectrlCore1_save);
	phy_utils_write_phyreg(pi, NPHY_AfectrlCore2, afectrlCore2_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlMiscReg1, rfctrlMiscReg1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlMiscReg2, rfctrlMiscReg2_save);
	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride1, afectrlOverride1_save);
	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride2, afectrlOverride2_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverrideAux0, rfctrlOverrideAux0_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverrideAux1, rfctrlOverrideAux1_save);

	return (rssi_out_val);
}

void
wlc_phy_rssi_cal_nphy(phy_info_t *pi)
{
	/* Includes REV7+ RSSI cal */
	wlc_phy_rssi_cal_nphy_rev3(pi);
}

void
wlc_phy_rfctrlintc_override_nphy(phy_info_t *pi, uint8 field, uint16 value,
	uint8 core_code)
{
	uint16 mask;
	uint16 val;
	uint8  core;

	/* core_code is RADIO_MIMO_CORESEL_[CORE1,CORE2,ALLRX,ALLTX,ALLRXTX] */

	FOREACH_CORE(pi, core) {
		if (core_code == RADIO_MIMO_CORESEL_CORE1 && core == PHY_CORE_1)
			continue;
		else if (core_code == RADIO_MIMO_CORESEL_CORE2 && core == PHY_CORE_0)
			continue;

		/* enable override */
		/* For Rev7+, Fields of reg RfctrlIntc[1,2] redefined for 2057
		 * There's no single "override" bit
		 * but individual "override_[tr_sw,ext_lna,ext_pa]" bits
		 */

		if (field == NPHY_RfctrlIntc_override_OFF) {
			/* clear all bits */
			phy_utils_write_phyreg(pi, (core == PHY_CORE_0) ? NPHY_RfctrlIntc1 :
				NPHY_RfctrlIntc2, 0);

			phy_utils_and_phyreg(pi, NPHY_AntSelConfig2057,
			           ~NPHY_AntSelConfig2057_AntCfg_OverrideEn_MASK);

			/* force rfseq */
			wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);

		} else if (field == NPHY_RfctrlIntc_override_TRSW) {

			/* Get mask for bit 6 (tr_sw_tx_pu) & bit 7 (tr_sw_rx_pu) */
			mask = NPHY_REV7_RfctrlIntc_tr_sw_tx_pu_MASK |
			        NPHY_REV7_RfctrlIntc_tr_sw_rx_pu_MASK;

			val = value << NPHY_REV7_RfctrlIntc_tr_sw_tx_pu_SHIFT;
			phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
			                     NPHY_RfctrlIntc1 :
				NPHY_RfctrlIntc2, mask, val);

			/* set bit 10: override_tr_sw = 1 */
			phy_utils_or_phyreg(pi, (core == PHY_CORE_0) ?
			                    NPHY_RfctrlIntc1 :
				NPHY_RfctrlIntc2,
				NPHY_REV7_RfctrlIntc_override_tr_sw_MASK);

			/* make sure ant. config bits are set to 0
			   => main antennas for both cores
			   for 2 of 3 selection diversity, will need to update
			*/
			PHY_REG_LIST_START
			PHY_REG_AND_ENTRY(NPHY, AntSelConfig2057,
				(uint16)~NPHY_AntSelConfig2057_AntCfg_Override_MASK)
			PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
				NPHY_AntSelConfig2057_AntCfg_OverrideEn_MASK)
			PHY_REG_OR_ENTRY(NPHY, AntSelConfig2057,
				NPHY_AntSelConfig2057_Trigger_MASK)
			PHY_REG_LIST_EXECUTE(pi);
		} else if (field == NPHY_RfctrlIntc_override_PA) {
			/* The difference here is we are explicitly making sure
			   ext_5g_papu is 0 if it's 2g and vice versa.
			*/
			mask = NPHY_REV7_RfctrlIntc_ext_2g_papu_MASK |
			        NPHY_REV7_RfctrlIntc_ext_5g_papu_MASK;

			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				val = value << NPHY_REV7_RfctrlIntc_ext_5g_papu_SHIFT;
			} else {
				val = value << NPHY_REV7_RfctrlIntc_ext_2g_papu_SHIFT;
			}

			phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
			                     NPHY_RfctrlIntc1 : NPHY_RfctrlIntc2, mask, val);

			/* set bit 12: override_ext_pa = 1 */
			phy_utils_or_phyreg(pi, (core == PHY_CORE_0) ?
			                    NPHY_RfctrlIntc1 : NPHY_RfctrlIntc2,
			                    NPHY_REV7_RfctrlIntc_override_ext_pa_MASK);
		} else if (field == NPHY_RfctrlIntc_override_EXT_LNA_PU) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				/* override 5g lna pu */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_5g_pu_MASK;
				val = value <<
				        NPHY_REV7_RfctrlIntc_ext_lna_5g_pu_SHIFT;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, val);

				/* off 2g lna pu */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_2g_pu_MASK;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, 0);
			} else {
				/* 2g */
				/* override 2g lna pu */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_2g_pu_MASK;
				val = value <<
				        NPHY_REV7_RfctrlIntc_ext_lna_2g_pu_SHIFT;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, val);

				/* off 5g lna pu */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_5g_pu_MASK;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, 0);
			}

			/* set ext LNA override bit */
			mask = NPHY_REV7_RfctrlIntc_override_ext_lna_MASK;
			val = 1 << NPHY_REV7_RfctrlIntc_override_ext_lna_SHIFT;
			phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
			                     NPHY_RfctrlIntc1 : NPHY_RfctrlIntc2, mask, val);
		} else if (field == NPHY_RfctrlIntc_override_EXT_LNA_GAIN) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				/* override 5g lna pu */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_5g_gain_MASK;
				val = value <<
				        NPHY_REV7_RfctrlIntc_ext_lna_5g_gain_SHIFT;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, val);

				/* off 2g lna pu */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_2g_gain_MASK;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, 0);
			} else {
				/* 2g */
				/* override 2g lna gain */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_2g_gain_MASK;
				val = value <<
				        NPHY_REV7_RfctrlIntc_ext_lna_2g_gain_SHIFT;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, val);

				/* off 5g lna pu */
				mask = NPHY_REV7_RfctrlIntc_ext_lna_5g_gain_MASK;
				phy_utils_mod_phyreg(pi, (core == PHY_CORE_0)?
				            NPHY_RfctrlIntc1:NPHY_RfctrlIntc2,
				            mask, 0);
			}

			/* set ext LNA override bit */
			mask = NPHY_REV7_RfctrlIntc_override_ext_lna_MASK;
			val = 1 << NPHY_REV7_RfctrlIntc_override_ext_lna_SHIFT;
			phy_utils_mod_phyreg(pi, (core == PHY_CORE_0) ?
			                     NPHY_RfctrlIntc1 :
				NPHY_RfctrlIntc2, mask, val);
		} else {
			PHY_ERROR(("wl%d: %s: Undefined input type %d\n", pi->sh->unit,
			           __FUNCTION__, field));
		}
	}	/* for */
}

static void
wlc_phy_rssi_cal_nphy_rev3(phy_info_t *pi)
{
	phy_info_nphy_t *pi_nphy = NULL;
	uint16 classif_state;
	uint16 clip_state[NPHY_CORE_NUM];
	uint16 clip_off[NPHY_CORE_NUM] = {0xffff, 0xffff};
	int32 target_code;
	uint8  vcm, min_vcm;
	uint8  vcm_final = 0;
	uint8  result_idx, ctr;
	int32  poll_results[8][4] = {
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	};
	int32  poll_result_core[4] = {0, 0, 0, 0};
	int32  min_d = NPHY_RSSICAL_MAXD, curr_d;
	int32  fine_digital_offset[4];
	int32  poll_results_min[4] = {0, 0, 0, 0};
	int32  min_poll;
	uint8  vcm_level_max;
	uint8  core;
	uint8  wb_cnt;
	uint8  rssi_type;
	uint16 NPHY_Rfctrlintc1_save, NPHY_Rfctrlintc2_save;
	uint16 NPHY_AfectrlOverride1_save, NPHY_AfectrlOverride2_save;
	uint16 NPHY_AfectrlCore1_save, NPHY_AfectrlCore2_save;
	uint16 NPHY_RfctrlOverride0_save, NPHY_RfctrlOverride1_save;
	uint16 NPHY_RfctrlOverrideAux0_save, NPHY_RfctrlOverrideAux1_save;
	uint16 NPHY_RfctrlCmd_save, NPHY_AntSelConfig2057_save;
	uint16 NPHY_RfctrlMiscReg1_save, NPHY_RfctrlMiscReg2_save;
	uint16 NPHY_RfctrlRSSIOTHERS1_save, NPHY_RfctrlRSSIOTHERS2_save;
	uint8 rxcore_state;
	uint16 NPHY_REV7_RfctrlOverride3_save, NPHY_REV7_RfctrlOverride4_save;
	uint16 NPHY_REV7_RfctrlOverride5_save, NPHY_REV7_RfctrlOverride6_save;
	uint16 NPHY_REV7_RfctrlMiscReg3_save, NPHY_REV7_RfctrlMiscReg4_save;
	uint16 NPHY_REV7_RfctrlMiscReg5_save, NPHY_REV7_RfctrlMiscReg6_save;

	NPHY_REV7_RfctrlOverride3_save = NPHY_REV7_RfctrlOverride4_save =
	        NPHY_REV7_RfctrlOverride5_save = NPHY_REV7_RfctrlOverride6_save =
	        NPHY_REV7_RfctrlMiscReg3_save =  NPHY_REV7_RfctrlMiscReg4_save =
	        NPHY_REV7_RfctrlMiscReg5_save = NPHY_REV7_RfctrlMiscReg6_save =
	        NPHY_AntSelConfig2057_save = 0;

	pi_nphy = pi->u.pi_nphy;
	/* SAVING STATE */

	/* save classifier and clip threshold state */
	classif_state = wlc_phy_classifier_nphy(pi, 0, 0);
	wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK, 4);
	wlc_phy_clip_det_nphy(pi, 0, clip_state);
	wlc_phy_clip_det_nphy(pi, 1, clip_off);

	/* saving rfctrlIntc, afectrlovr, afectrlcore, rfctrlovr,
	   rfctrlovraux, rfctrlmisc, rfctrlrssiothers regs
	*/
	NPHY_Rfctrlintc1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc1);
	NPHY_Rfctrlintc2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc2);
	NPHY_AfectrlOverride1_save   = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride1);
	NPHY_AfectrlOverride2_save   = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride2);
	NPHY_AfectrlCore1_save       = phy_utils_read_phyreg(pi, NPHY_AfectrlCore1);
	NPHY_AfectrlCore2_save       = phy_utils_read_phyreg(pi, NPHY_AfectrlCore2);
	NPHY_RfctrlOverride0_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride0);
	NPHY_RfctrlOverride1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride1);
	NPHY_REV7_RfctrlOverride3_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride3);
	NPHY_REV7_RfctrlOverride4_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride4);
	NPHY_REV7_RfctrlOverride5_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride5);
	NPHY_REV7_RfctrlOverride6_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride6);
	NPHY_AntSelConfig2057_save       = phy_utils_read_phyreg(pi, NPHY_AntSelConfig2057);
	NPHY_RfctrlOverrideAux0_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverrideAux0);
	NPHY_RfctrlOverrideAux1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverrideAux1);
	NPHY_RfctrlCmd_save          = phy_utils_read_phyreg(pi, NPHY_RfctrlCmd);
	NPHY_RfctrlMiscReg1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlMiscReg1);
	NPHY_RfctrlMiscReg2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlMiscReg2);
	NPHY_REV7_RfctrlMiscReg3_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg3);
	NPHY_REV7_RfctrlMiscReg4_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg4);
	NPHY_REV7_RfctrlMiscReg5_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg5);
	NPHY_REV7_RfctrlMiscReg6_save   = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg6);
	NPHY_RfctrlRSSIOTHERS1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlRSSIOTHERS1);
	NPHY_RfctrlRSSIOTHERS2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlRSSIOTHERS2);

	/* Clear all bits and then set band-appropriate T/R switch to TX */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_OFF, 0,
	                                 RADIO_MIMO_CORESEL_ALLRXTX);
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_TRSW, 1,
	                                 RADIO_MIMO_CORESEL_ALLRXTX);

	/* power down the radio before mixer */
	wlc_phy_rfctrl_override_1tomany_nphy(pi, NPHY_REV7_RfctrlOverride_cmd_rxrf_pu, 0, 0, 0);

	/* power up the radio after mixer */
	wlc_phy_rfctrl_override_1tomany_nphy(pi, NPHY_REV7_RfctrlOverride_cmd_rx_pu, 1, 0, 0);

	/* Turn on rssi circuit */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rssi_nb_pu_MASK,
	                                  1, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rssi_wb2_pu_MASK,
	                                  1, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rssi_wb1g_pu_MASK,
			0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rssi_wb1a_pu_MASK,
			1, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	} else {
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rssi_wb1a_pu_MASK,
			0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rssi_wb1g_pu_MASK,
			1, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	}

	/* Determine which cores are enabled */
	rxcore_state = wlc_phy_rxcore_getstate_nphy((wlc_phy_t *)pi);

	vcm_level_max = 8;
	/* start of nb rssi cal */
	FOREACH_CORE(pi, core) {
		/* If this Rx core is disabled, skip the calibration for this core */
		if ((rxcore_state & (1 << core)) == 0)
			continue;

		/* restore affine transform block to default -- identity-transform */
		wlc_phy_scale_offset_rssi_nphy(pi, 0x0, 0x0, core == PHY_CORE_0 ?
			RADIO_MIMO_CORESEL_CORE1 : RADIO_MIMO_CORESEL_CORE2, NPHY_RAIL_I,
			NPHY_RSSI_SEL_NB);
		wlc_phy_scale_offset_rssi_nphy(pi, 0x0, 0x0, core == PHY_CORE_0 ?
			RADIO_MIMO_CORESEL_CORE1 : RADIO_MIMO_CORESEL_CORE2, NPHY_RAIL_Q,
			NPHY_RSSI_SEL_NB);

		/* collect RSSI polling results for both rails of both cores, for
		 * each of candidate 2056 vcm control settings
		 */
		for (vcm = 0; vcm < vcm_level_max; vcm++) {
			/* set test 2057 vcm out */
			phy_utils_mod_radioreg(
				pi, (core == PHY_CORE_0) ?
				RADIO_2057_NB_MASTER_CORE0 :
				RADIO_2057_NB_MASTER_CORE1,
				RADIO_2057_VCM_MASK, vcm);

			/* sum several samples */
			wlc_phy_poll_rssi_nphy(pi, NPHY_RSSI_SEL_NB, &poll_results[vcm][0],
				NPHY_RSSICAL_NPOLL);
		}

		PHY_CAL(("\n"));
		for (vcm = 0; vcm < vcm_level_max; vcm++) {
			for (ctr = 0; ctr < 4; ctr++) {
				if (core == ctr/2)
				PHY_CAL(("wl%d: %s: %s RSSI, VCM %d; CORE %d; %s-Rail: %d\n",
					pi->sh->unit, __FUNCTION__, "NB",
					vcm, ctr/2, (((ctr % 2) == 0) ? "I" : "Q"),
					poll_results[vcm][ctr]));
			}
		}

		/* for all the values, find the closest to the target;
		 * also find the min poll value for vcm, for later sanity check
		 */
		for (result_idx = 0; result_idx < 4; result_idx++) {
			if ((core == result_idx/2) && (result_idx % 2 == 0)) {
			/* each core's I pointer */
			min_d = NPHY_RSSICAL_MAXD;
			min_vcm = 0;
			min_poll = NPHY_RSSICAL_MAXREAD * NPHY_RSSICAL_NPOLL + 1;
			for (vcm = 0; vcm < vcm_level_max; vcm++) {
				curr_d = poll_results[vcm][result_idx] *
					poll_results[vcm][result_idx] +
					poll_results[vcm][result_idx + 1] *
					poll_results[vcm][result_idx + 1];
				if (curr_d < min_d) {
					min_d = curr_d;
					min_vcm = vcm;
				}
				if (poll_results[vcm][result_idx] < min_poll) {
					min_poll = poll_results[vcm][result_idx];
				}
			}
			vcm_final = min_vcm;
			poll_results_min[result_idx] = min_poll;
			}
		}

		PHY_CAL(("\n"));
		for (ctr = 0; ctr < 4; ctr++) {
			if (core == ctr/2)
				PHY_CAL(("wl%d: %s: 205x %s RSSI vcm: CORE %d; %s-Rail: %d"
					", rssi: %d\n",
					pi->sh->unit, __FUNCTION__, "NB",
					ctr/2, (((ctr % 2) == 0) ? "I" : "Q"), vcm_final,
					poll_results[vcm_final][ctr]));
		}

		/* apply cal result */
		phy_utils_mod_radioreg(pi, (core == PHY_CORE_0) ?
			RADIO_2057_NB_MASTER_CORE0 : RADIO_2057_NB_MASTER_CORE1,
			RADIO_2057_VCM_MASK, vcm_final);

		/* remove residual bias using digital affine transform block */
		for (result_idx = 0; result_idx < 4; result_idx++) {
			if (core == result_idx/2) {
				fine_digital_offset[result_idx] = (NPHY_RSSICAL_NB_TARGET *
					NPHY_RSSICAL_NPOLL) -
					poll_results[vcm_final][result_idx];
				if (fine_digital_offset[result_idx] < 0) {
					fine_digital_offset[result_idx] =
						ABS(fine_digital_offset[result_idx]);
					fine_digital_offset[result_idx] += (NPHY_RSSICAL_NPOLL/2);
					fine_digital_offset[result_idx] /= NPHY_RSSICAL_NPOLL;
					fine_digital_offset[result_idx] =
						-fine_digital_offset[result_idx];
				} else {
					fine_digital_offset[result_idx] += (NPHY_RSSICAL_NPOLL/2);
					fine_digital_offset[result_idx] /= NPHY_RSSICAL_NPOLL;
				}

				/*
				 adjust final offset values by -1 if the rssi appears to be
				 stuck at "high"
				*/
				if (poll_results_min[result_idx] == NPHY_RSSICAL_MAXREAD *
					NPHY_RSSICAL_NPOLL) {
					fine_digital_offset[result_idx] = (NPHY_RSSICAL_NB_TARGET -
						NPHY_RSSICAL_MAXREAD - 1);
				}

				/* apply offsets to phy */
				wlc_phy_scale_offset_rssi_nphy(pi, 0x0,
					(int8)fine_digital_offset[result_idx],
					(result_idx/2 == 0) ? RADIO_MIMO_CORESEL_CORE1 :
					RADIO_MIMO_CORESEL_CORE2,
					(result_idx%2 == 0) ? NPHY_RAIL_I :
					NPHY_RAIL_Q,
					NPHY_RSSI_SEL_NB);
			}
		}

		PHY_CAL(("\n"));
		for (ctr = 0; ctr < 4; ctr++) {
			if (core == ctr/2)
			PHY_CAL(("wl%d: %s: Digital Offset: %s RSSI, CORE %d; %s-Rail: %d\n",
				pi->sh->unit, __FUNCTION__, "NB",
				ctr/2, (((ctr % 2) == 0) ? "I" : "Q"),
				(int8)fine_digital_offset[ctr]));
		}
	}

	/* start of wb1, wb2 rssi cal */
	FOREACH_CORE(pi, core) {
		/* If this Rx core is disabled, skip the calibration for this core */
		if ((rxcore_state & (1 << core)) == 0)
			continue;

		for (wb_cnt = 0; wb_cnt < 2; wb_cnt++) {
			if (wb_cnt == 0) {
				rssi_type = NPHY_RSSI_SEL_W1;
				target_code = NPHY_RSSICAL_W1_TARGET_REV3;
			} else {
				rssi_type = NPHY_RSSI_SEL_W2;
				target_code = NPHY_RSSICAL_W2_TARGET_REV3;
			}

			/* restore affine transform block to default -- identity-transform */
			wlc_phy_scale_offset_rssi_nphy(pi, 0x0, 0x0,
				core == PHY_CORE_0 ? RADIO_MIMO_CORESEL_CORE1 :
				RADIO_MIMO_CORESEL_CORE2, NPHY_RAIL_I, rssi_type);
			wlc_phy_scale_offset_rssi_nphy(pi, 0x0, 0x0,
				core == PHY_CORE_0 ? RADIO_MIMO_CORESEL_CORE1 :
				RADIO_MIMO_CORESEL_CORE2, NPHY_RAIL_Q, rssi_type);

			/* collect RSSI polling results for both rails, sum several samples */
			wlc_phy_poll_rssi_nphy(pi, rssi_type, poll_result_core,
				NPHY_RSSICAL_NPOLL);

			PHY_CAL(("\n"));
				for (ctr = 0; ctr < 4; ctr++) {
				if (core == ctr/2)
					PHY_CAL(("wl%d: %s: %s RSSI; CORE %d; %s-Rail: %d\n",
						pi->sh->unit, __FUNCTION__, (rssi_type ==
						NPHY_RSSI_SEL_W1 ? "WB1" : "WB2"),
						ctr/2, (((ctr % 2) == 0) ? "I" : "Q"),
						poll_result_core[ctr]));
			}

			/* remove residual bias using digital affine transform block */
			for (result_idx = 0; result_idx < 4; result_idx++) {
				if (core == result_idx/2) {
					fine_digital_offset[result_idx] = (target_code *
						NPHY_RSSICAL_NPOLL) -
						poll_result_core[result_idx];
					if (fine_digital_offset[result_idx] < 0) {
						fine_digital_offset[result_idx] =
							ABS(fine_digital_offset[result_idx]);
						fine_digital_offset[result_idx] +=
							(NPHY_RSSICAL_NPOLL/2);
						fine_digital_offset[result_idx] /=
							NPHY_RSSICAL_NPOLL;
						fine_digital_offset[result_idx] =
							-fine_digital_offset[result_idx];
					} else {
						fine_digital_offset[result_idx] +=
							(NPHY_RSSICAL_NPOLL/2);
						fine_digital_offset[result_idx] /=
							NPHY_RSSICAL_NPOLL;
					}

					/* apply offsets to phy */
					wlc_phy_scale_offset_rssi_nphy(pi, 0x0,
						(int8)fine_digital_offset[core * 2],
						(core == PHY_CORE_0) ? RADIO_MIMO_CORESEL_CORE1 :
						RADIO_MIMO_CORESEL_CORE2,
						(result_idx % 2 == 0) ? NPHY_RAIL_I :
						NPHY_RAIL_Q,
						rssi_type);
					}
				}

		PHY_CAL(("\n"));
		for (ctr = 0; ctr < 4; ctr++) {
			if (core == ctr/2)
			PHY_CAL(("wl%d: %s: Digital Offset: %s RSSI, CORE %d; %s-Rail: %d\n",
				pi->sh->unit, __FUNCTION__, (rssi_type ==
				NPHY_RSSI_SEL_W1 ? "WB1" : "WB2"),
				ctr/2, (((ctr % 2) == 0) ? "I" : "Q"),
				(int8)fine_digital_offset[ctr]));
		}
		}
	}

	/* RESTORING STATE */

	/* restoring rfctrlIntc, afectrlovr, afectrlcore, rfctrlovr,
	   rfctrlovraux, rfctrlmisc, rfctrlrssiothers regs
	*/
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc1,        NPHY_Rfctrlintc1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc2,        NPHY_Rfctrlintc2_save);

	/* force rfseq */
	wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);

	/* do trigger sequencing */
	PHY_REG_LIST_START
		PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlOverride0, NPHY_RfctrlOverride_trigger_MASK,
			1 << NPHY_RfctrlOverride_trigger_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlCmd, NPHY_REV3_RfctrlCmd_startseq0_MASK,
			1 << NPHY_REV3_RfctrlCmd_startseq0_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlOverride0, NPHY_RfctrlOverride_trigger_MASK, 0)
		PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlOverride1, NPHY_RfctrlOverride_trigger_MASK,
			1 << NPHY_RfctrlOverride_trigger_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlCmd, NPHY_REV3_RfctrlCmd_startseq1_MASK,
			1 << NPHY_REV3_RfctrlCmd_startseq1_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_RfctrlOverride1, NPHY_RfctrlOverride_trigger_MASK, 0)
	PHY_REG_LIST_EXECUTE(pi);

	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride1,   NPHY_AfectrlOverride1_save);
	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride2,   NPHY_AfectrlOverride2_save);
	phy_utils_write_phyreg(pi, NPHY_AfectrlCore1,       NPHY_AfectrlCore1_save);
	phy_utils_write_phyreg(pi, NPHY_AfectrlCore2,       NPHY_AfectrlCore2_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride0,    NPHY_RfctrlOverride0_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride1,    NPHY_RfctrlOverride1_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride3, NPHY_REV7_RfctrlOverride3_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride4, NPHY_REV7_RfctrlOverride4_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride5, NPHY_REV7_RfctrlOverride5_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride6, NPHY_REV7_RfctrlOverride6_save);
	phy_utils_write_phyreg(pi, NPHY_AntSelConfig2057, NPHY_AntSelConfig2057_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverrideAux0, NPHY_RfctrlOverrideAux0_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverrideAux1, NPHY_RfctrlOverrideAux1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlCmd,          NPHY_RfctrlCmd_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlMiscReg1,     NPHY_RfctrlMiscReg1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlMiscReg2,     NPHY_RfctrlMiscReg2_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg3, NPHY_REV7_RfctrlMiscReg3_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg4, NPHY_REV7_RfctrlMiscReg4_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg5, NPHY_REV7_RfctrlMiscReg5_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg6, NPHY_REV7_RfctrlMiscReg6_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlRSSIOTHERS1,  NPHY_RfctrlRSSIOTHERS1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlRSSIOTHERS2,  NPHY_RfctrlRSSIOTHERS2_save);

	/* Save RSSI calibration values in the cache. This will allow us to restore
	   the band-specific RSSI cal values in wlc_nphy_init() without actually
	   doing the calibration
	*/
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		pi_nphy->nphy_rssical_cache.rssical_radio_regs_2G[0] =
		    phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE0);
		pi_nphy->nphy_rssical_cache.rssical_radio_regs_2G[1] =
		    phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE1);

		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[0] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[1] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[2] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[3] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[4] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[5] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[6] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[7] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[8] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIY);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[9] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIY);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[10]=
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIY);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[11] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIY);

		pi_nphy->nphy_rssical_chanspec_2G = pi->radio_chanspec;
	} else {
		pi_nphy->nphy_rssical_cache.rssical_radio_regs_5G[0] =
		    phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE0);
		pi_nphy->nphy_rssical_cache.rssical_radio_regs_5G[1] =
		    phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE1);

		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[0] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[1] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[2] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[3] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIZ);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[4] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[5] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[6] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[7] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIX);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[8] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIY);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[9] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIY);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[10] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIY);
		pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[11] =
		    phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIY);

		pi_nphy->nphy_rssical_chanspec_5G = pi->radio_chanspec;
	}

	/* restore classifier and clip threshold state */
	wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK, classif_state);
	wlc_phy_clip_det_nphy(pi, 1, clip_state);
}

static void
wlc_phy_restore_rssical_nphy(phy_info_t *pi)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (pi_nphy->nphy_rssical_chanspec_2G == 0)
			return;

		phy_utils_mod_radioreg(pi, RADIO_2057_NB_MASTER_CORE0, RADIO_2057_VCM_MASK,
			pi_nphy->nphy_rssical_cache.rssical_radio_regs_2G[0]);
		phy_utils_mod_radioreg(pi, RADIO_2057_NB_MASTER_CORE1, RADIO_2057_VCM_MASK,
			pi_nphy->nphy_rssical_cache.rssical_radio_regs_2G[1]);

		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[0]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[1]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[2]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[3]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[4]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[5]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[6]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[7]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[8]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[9]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[10]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_2G[11]);

	} else {
		if (pi_nphy->nphy_rssical_chanspec_5G == 0)
			return;

		phy_utils_mod_radioreg(pi, RADIO_2057_NB_MASTER_CORE0, RADIO_2057_VCM_MASK,
			pi_nphy->nphy_rssical_cache.rssical_radio_regs_5G[0]);
		phy_utils_mod_radioreg(pi, RADIO_2057_NB_MASTER_CORE1, RADIO_2057_VCM_MASK,
			pi_nphy->nphy_rssical_cache.rssical_radio_regs_5G[1]);

		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[0]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[1]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[2]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIZ,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[3]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[4]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[5]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[6]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIX,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[7]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0IRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[8]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef0QRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[9]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1IRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[10]);
		phy_utils_write_phyreg(pi, NPHY_RSSIMultCoef1QRSSIY,
		    pi_nphy->nphy_rssical_cache.rssical_phyregs_5G[11]);
	}
}

static uint16
wlc_phy_gen_load_samples_nphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 dac_test_mode)
{
	uint8 phy_bw, is_phybw40;
	uint16 num_samps, t, spur;
	math_fixed theta = 0, rot = 0;
	uint32 tbl_len;
/* to save memory on dongle builds using resolution of 500KHz
 * and cint16 instead of unnecessary cint32
 */
#ifdef DONGLEBUILD
	uint32* tone_buf_pack = NULL;
	math_cint32 data_buf;
#else
	math_cint32* tone_buf = NULL;
#endif // endif
	/* check phy_bw */
	is_phybw40 = CHSPEC_IS40(pi->radio_chanspec);
	phy_bw     = (is_phybw40 == 1)? 40 : 20;
#ifdef DONGLEBUILD
	tbl_len    = (phy_bw << 2);
#else
	tbl_len    = (phy_bw << 3);
#endif // endif

	if (dac_test_mode == 1) {
	        spur = phy_utils_read_phyreg(pi, NPHY_BBConfig);
		spur = (spur >> NPHY_BBConfig_resample_clk160_SHIFT) & 1;
		phy_bw = (spur == 1)? 82 : 80;
		phy_bw = (is_phybw40 == 1)? (phy_bw << 1): phy_bw;

		/* use smaller num_samps to prevent overflow the buffer length */
		tbl_len   = (phy_bw << 1);
	}

	/* allocate buffer */
#ifdef DONGLEBUILD
	if ((tone_buf_pack = (uint32 *)MALLOC(pi->sh->osh, sizeof(uint32) * tbl_len)) == NULL) {
#else
	if ((tone_buf = MALLOC(pi->sh->osh, sizeof(math_cint32) * tbl_len)) == NULL) {
#endif // endif
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		          __FUNCTION__, MALLOCED(pi->sh->osh)));
		return 0;
	}

	/* set up params to generate tone */
	num_samps  = (uint16)tbl_len;
	rot = FIXED((f_kHz * 36)/phy_bw) / 100; /* 2*pi*f/bw/1000  Note: f in KHz */
	theta = 0;			/* start angle 0 */

	/* tone freq = f_c MHz ; phy_bw = phy_bw MHz ; # samples = phy_bw (1us) ; max_val = 151 */
	/* TCL: set tone_buff [mimophy_gen_tone $f_c $phy_bw $phy_bw $max_val] */
	for (t = 0; t < num_samps; t++) {
		/* compute phasor */
#ifdef DONGLEBUILD
		math_cmplx_cordic(theta, &data_buf);
#else
		math_cmplx_cordic(theta, &tone_buf[t]);
#endif // endif
		/* update rotation angle */
		theta += rot;
		/* produce sample values for play buffer */
#ifdef DONGLEBUILD
		data_buf.q = (int32)FLOAT(data_buf.q * max_val);
		data_buf.i = (int32)FLOAT(data_buf.i * max_val);
		tone_buf_pack[t] = ((((unsigned int)data_buf.i) & 0x3ff) << 10) |
		        (((unsigned int)data_buf.q) & 0x3ff);
#else
		tone_buf[t].q = (int32)FLOAT(tone_buf[t].q * max_val);
		tone_buf[t].i = (int32)FLOAT(tone_buf[t].i * max_val);
#endif // endif
	}

	/* load sample table */
#ifdef DONGLEBUILD
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_SAMPLEPLAY, num_samps, 0, 32, tone_buf_pack);
#else
	wlc_phy_loadsampletable_nphy(pi, tone_buf, num_samps);
#endif // endif

#ifdef DONGLEBUILD
	if (tone_buf_pack != NULL)
		MFREE(pi->sh->osh, tone_buf_pack, sizeof(uint32) * tbl_len);
#else
	if (tone_buf != NULL)
		MFREE(pi->sh->osh, tone_buf, sizeof(math_cint32) * tbl_len);
#endif // endif
	return num_samps;
}

int
wlc_phy_tx_tone_nphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 iqmode,
                     uint8 dac_test_mode, bool modify_bbmult)
{
	uint16 num_samps;
	uint16 loops = 0xffff;
	uint16 wait = 0;

	/* Generate the samples of the tone and load it into the playback buffer */
	if ((num_samps = wlc_phy_gen_load_samples_nphy(pi, f_kHz, max_val, dac_test_mode)) == 0) {
		return BCME_ERROR;
	}

	/* Now, play the samples */
	wlc_phy_runsamples_nphy(pi, num_samps, loops, wait, iqmode, dac_test_mode, modify_bbmult);

	return BCME_OK;
}

#ifndef DONGLEBUILD
static void
wlc_phy_loadsampletable_nphy(phy_info_t *pi, math_cint32 *tone_buf, uint16 num_samps)
{
	uint16 t;
	uint32* data_buf = NULL;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* allocate buffer */
	if ((data_buf = (uint32 *)MALLOC(pi->sh->osh, sizeof(uint32) * num_samps)) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		          __FUNCTION__, MALLOCED(pi->sh->osh)));
		return;
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* load samples into sample play buffer
	 * TCL: mimophy_load_sample_table $tone_buff
	 * TCL: set curr_samp_20bit [hexpr ($curr_samp_i << 10) | $curr_samp_q]
	 * mimophy_write_table sampleplaytbl $curr_samp_20bit $ctr
	 */
	for (t = 0; t < num_samps; t++) {
		data_buf[t] = ((((unsigned int)tone_buf[t].i) & 0x3ff) << 10) |
		        (((unsigned int)tone_buf[t].q) & 0x3ff);
	}
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_SAMPLEPLAY, num_samps, 0, 32, data_buf);

	if (data_buf != NULL)
		MFREE(pi->sh->osh, data_buf, sizeof(uint32) * num_samps);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}
#endif /* Code has been optimized for dongle builds to save memory */

static void
wlc_phy_runsamples_nphy(phy_info_t *pi, uint16 num_samps, uint16 loops, uint16 wait, uint8 iqmode,
                        uint8 dac_test_mode, bool modify_bbmult)
{
	uint16 bb_mult;
	uint8 phy_bw, sample_cmd;
	uint16 orig_RfseqCoreActv;
	uint16 lpf_bw_ctl_override3, lpf_bw_ctl_override4, lpf_bw_ctl_miscreg3, lpf_bw_ctl_miscreg4;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* check phy_bw */
	phy_bw = 20;
	if (CHSPEC_IS40(pi->radio_chanspec))
		phy_bw = 40;

	/* Check if the lpf_bw_ctl override has already been set before calling Sample Play.
	 * If the override has been set before calling Sample Play, then the override
	 * should not be overwritten by the Sample Play function.
	 */
	lpf_bw_ctl_override3 = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride3) &
	        NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK;
	lpf_bw_ctl_override4 = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride4) &
	        NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK;
	if (lpf_bw_ctl_override3 | lpf_bw_ctl_override4) {
		lpf_bw_ctl_miscreg3 = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg3) &
		        NPHY_REV7_RfctrlMiscReg_lpf_bw_ctl_MASK;
		lpf_bw_ctl_miscreg4 = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg4) &
		        NPHY_REV7_RfctrlMiscReg_lpf_bw_ctl_MASK;
		PHY_INFORM(("wl%d: %s: Analog LPF BW override via lpf_bw_ctl is already set"
		           "- core 0:"
		           "Ovr=%d, Val=%d, core 1: Ovr=%d, Val=%d. Not overriding inside "
		           "Sample Play proc.\n", pi->sh->unit, __FUNCTION__,
		           lpf_bw_ctl_override3, lpf_bw_ctl_miscreg3,
		           lpf_bw_ctl_override4, lpf_bw_ctl_miscreg4));
		BCM_REFERENCE(lpf_bw_ctl_miscreg3);
		BCM_REFERENCE(lpf_bw_ctl_miscreg4);
	} else {
		wlc_phy_rfctrl_override_nphy_rev7(pi,
			NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
			wlc_phy_read_lpf_bw_ctl_nphy(pi, 0), 0, 0,
			NPHY_REV7_RFCTRLOVERRIDE_ID1);
		/* Set the flag to indicate that the LPF BW override was set inside
		 * the sample play function
		 */
		pi_nphy->nphy_sample_play_lpf_bw_ctl_ovr = TRUE;

		lpf_bw_ctl_miscreg3 = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg3) &
		        NPHY_REV7_RfctrlMiscReg_lpf_bw_ctl_MASK;
		lpf_bw_ctl_miscreg4 = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg4) &
		        NPHY_REV7_RfctrlMiscReg_lpf_bw_ctl_MASK;
		PHY_INFORM(("wl%d: %s: Setting the Analog LPF BW override via lpf_bw_ctl"
		           "in the Sample Play function."
		           "core 0: Ovr=%d, Val=%d, core 1: Ovr=%d, Val=%d.\n",
		           pi->sh->unit, __FUNCTION__,
		           lpf_bw_ctl_override3, lpf_bw_ctl_miscreg3, lpf_bw_ctl_override4,
		           lpf_bw_ctl_miscreg4));
	}

	/* if nphy_bb_mult_save does not exist, save current bb_mult */
	if ((pi_nphy->nphy_bb_mult_save & BB_MULT_VALID_MASK) == 0) {
		/* save does not exist, save current bb_mult value */
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 87, 16, &bb_mult);
		pi_nphy->nphy_bb_mult_save = BB_MULT_VALID_MASK | (bb_mult & BB_MULT_MASK);
	}

	/* set samp_play -> DAC_out loss to 0dB by setting bb_mult (2.6 format) to
	 * 100/64 for bw = 20MHz
	 *  71/64 for bw = 40Mhz
	 * mimophy_write_table iqloCaltbl [expr (val<<8)+val] 87
	 */
	if (modify_bbmult) {
		bb_mult = (phy_bw == 20) ? 100 : 71; /* sqrt(2) difference */
		bb_mult = (bb_mult<<8)+bb_mult;
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 87, 16, &bb_mult);
	}

	/* play full buffer continuously without waiting and not in iqcal_mode */
	/* TCL: mimophy_run_samples [llength $tone_buff] 0xffff 0 0 */
	phy_utils_write_phyreg(pi, NPHY_sampleDepthCount, num_samps-1);

	if (loops != 0xffff) {
		phy_utils_write_phyreg(pi, NPHY_sampleLoopCount, loops - 1);
	} else {
		phy_utils_write_phyreg(pi, NPHY_sampleLoopCount, loops);
	}
	phy_utils_write_phyreg(pi, NPHY_sampleWaitCount, wait);

	/* set mimophyreg(RfseqMode.CoreActv_override) 1 */
	orig_RfseqCoreActv = phy_utils_read_phyreg(pi, NPHY_RfseqMode);
	phy_utils_or_phyreg(pi, NPHY_RfseqMode, NPHY_RfseqMode_CoreActv_override);
	if (iqmode) {
		PHY_REG_LIST_START
			/* set regVal [set mimophyreg(iqloCalCmdGctl)] */
			/* set mimophyreg(iqloCalCmdGctl)
			 * [hexpr $regVal & 0x3FFF] ;# Clear iqlo_cal_en
			 */
			PHY_REG_AND_ENTRY(NPHY, iqloCalCmdGctl, 0x7FFF)
			/* set mimophyreg(iqloCalCmdGctl)
			 * [hexpr $regVal | 0x8000] ;# Set iqlo_cal_en
			 */
			PHY_REG_OR_ENTRY(NPHY, iqloCalCmdGctl, 0x8000)
		PHY_REG_LIST_EXECUTE(pi);
	} else {
		/* set mimophyreg(sampleCmd) 0x1 */
		sample_cmd = (dac_test_mode == 1)? 0x5: 0x1;
		phy_utils_write_phyreg(pi,  NPHY_sampleCmd, sample_cmd);
	}

	/* Wait till the Rx2Tx sequencing is done */
	SPINWAIT(((phy_utils_read_phyreg(pi, NPHY_RfseqStatus) & 0x1) == 1),
		NPHY_SPINWAIT_RUNSAMPLES_RFSEQ_STATUS);

	ASSERT(!((phy_utils_read_phyreg(pi, NPHY_RfseqStatus) & 0x1) == 1));
	/* restore mimophyreg(RfseqMode.CoreActv_override) */
	phy_utils_write_phyreg(pi, NPHY_RfseqMode, orig_RfseqCoreActv);

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

void
wlc_phy_stopplayback_nphy(phy_info_t *pi)
{
	uint16 playback_status;
	uint16 bb_mult;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* check status register */
	playback_status = phy_utils_read_phyreg(pi, NPHY_sampleStatus);
	if (playback_status & 0x1) {
		phy_utils_or_phyreg(pi, NPHY_sampleCmd, NPHY_sampleCmd_STOP);
	} else if (playback_status & 0x2) {
		/* bug in TCL */
		phy_utils_and_phyreg(pi, NPHY_iqloCalCmdGctl,
		                     (uint16)~NPHY_iqloCalCmdGctl_IQLO_CAL_EN);
	} else {
		PHY_CAL(("wlc_phy_stopplayback_nphy: already disabled\n"));
	}
	/* disable the dac_test mode */
	phy_utils_and_phyreg(pi, NPHY_sampleCmd, (uint16) ~NPHY_sampleCmd_DacTestMode_MASK);

	/* if nphy_bb_mult_save does exist, restore bb_mult and
	 * undef nphy_bb_mult_save
	 */
	if ((pi_nphy->nphy_bb_mult_save & BB_MULT_VALID_MASK) != 0) {
		/* restore bb_mult */
		bb_mult = pi_nphy->nphy_bb_mult_save & BB_MULT_MASK;
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 87, 16, &bb_mult);

		/* and undef nphy_bb_mult_save */
		pi_nphy->nphy_bb_mult_save = 0;
	}

	if (pi_nphy->nphy_sample_play_lpf_bw_ctl_ovr) {
		wlc_phy_rfctrl_override_nphy_rev7(pi,
		                                  NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
		                                  0, 0, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
		pi_nphy->nphy_sample_play_lpf_bw_ctl_ovr = FALSE;
		PHY_INFORM(("wl%d: %s: Clearing the Analog LPF BW override that was set"
		           "in the Sample Play function.", pi->sh->unit, __FUNCTION__));
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

void
wlc_phy_get_tx_gain_nphy(phy_info_t *pi, nphy_txgains_t *target_gain)
{
	uint16 base_idx[NPHY_CORE_NUM], curr_gain[NPHY_CORE_NUM];
	uint8 core_no;
	uint32 *tx_pwrctrl_tbl = NULL;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi->nphy_txpwrctrl == PHY_TPC_HW_OFF) {
		if (pi_nphy->phyhang_avoid)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

		/* read current tx gain from RFSeq table and use as target_gain */
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, curr_gain);

		if (pi_nphy->phyhang_avoid)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

		/* extract gain values */
		FOREACH_CORE(pi, core_no) {
			target_gain->ipa[core_no]  = curr_gain[core_no] & 0x0007;
			target_gain->pad[core_no]  = ((curr_gain[core_no] & 0x00F8) >> 3);
			target_gain->pga[core_no]  = ((curr_gain[core_no] & 0x0F00) >> 8);
			target_gain->txgm[core_no] = ((curr_gain[core_no] & 0x7000) >> 12);
			target_gain->txlpf[core_no] = ((curr_gain[core_no] & 0x8000) >> 15);
		}
	} else {
		base_idx[0] = (phy_utils_read_phyreg(pi, NPHY_Core0TxPwrCtrlStatus) >> 8) & 0x7f;
		base_idx[1] = (phy_utils_read_phyreg(pi, NPHY_Core1TxPwrCtrlStatus) >> 8) & 0x7f;

		FOREACH_CORE(pi, core_no) {
			if (PHY_IPA(pi)) {
				tx_pwrctrl_tbl = wlc_phy_get_ipa_gaintbl_nphy(pi);
			} else {
				tx_pwrctrl_tbl = wlc_phy_get_epa_gaintbl_nphy(pi);
			}

			target_gain->ipa[core_no]  =
				(tx_pwrctrl_tbl[base_idx[core_no]] >> 16) & 0x7;
			target_gain->pad[core_no] =
				(tx_pwrctrl_tbl[base_idx[core_no]] >> 19) & 0x1f;
			target_gain->pga[core_no] =
				(tx_pwrctrl_tbl[base_idx[core_no]] >> 24) & 0xf;
			target_gain->txgm[core_no] =
				(tx_pwrctrl_tbl[base_idx[core_no]] >> 28) & 0x7;
			target_gain->txlpf[core_no] =
				(tx_pwrctrl_tbl[base_idx[core_no]] >> 31) & 0x1;
		}
	}
}

static void
wlc_phy_iqcal_gainparams_nphy(phy_info_t *pi, uint16 core_no, nphy_txgains_t target_gain,
                              nphy_iqcal_params_t *params)
{
	params->txlpf = target_gain.txlpf[core_no];
	params->txgm = target_gain.txgm[core_no];
	params->pga  = target_gain.pga[core_no];
	params->pad  = target_gain.pad[core_no];
	params->ipa  = target_gain.ipa[core_no];
	params->cal_gain = ((params->txlpf << 15) | (params->txgm << 12) |
		(params->pga << 8) | (params->pad << 3) |
		(params->ipa));
	params->ncorr[0] = 0x79;
	params->ncorr[1] = 0x79;
	params->ncorr[2] = 0x79;
	params->ncorr[3] = 0x79;
	params->ncorr[4] = 0x79;
}

static void
wlc_phy_txcal_radio_setup_nphy(phy_info_t *pi)
{
	uint16 core;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* Put the stream of read/write regs into a data driven loop. */

	/* set 2057 into iq/lo cal state */
	FOREACH_CORE(pi, core) {
		/* First save the radio registers that will be modified */
		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 0] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MASTER);

		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 1] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_VCM_HG);

		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 2] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_IDAC);

		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 3] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_VCM);

		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 4] = 0;

		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 5] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX);

		if (RADIOREV(pi->pubpi->radiorev) != 5)
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 6] =
			        READ_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA);

		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 7] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG);

		pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 8] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_MISC1);

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MASTER, 0x0a);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_VCM_HG, 0x43);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_IDAC, 0x55);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_VCM, 0x00);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG, 0x00);
			if (pi_nphy->nphy_use_int_tx_iqlo_cal) {
				WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX, 0x4);
				if (!(pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa)) {
					/* enable A-band PAD tapoff */
					WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA, 0x31);
				} else {
					/* enable A-band INTPA tapoff */
					WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA, 0x21);
				}
			}
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_MISC1, 0x00);
		} else {
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MASTER, 0x06);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_VCM_HG, 0x43);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_IDAC, 0x55);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_VCM, 0x00);
			if (RADIOREV(pi->pubpi->radiorev) != 5)
				WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA, 0x00);
			if (pi_nphy->nphy_use_int_tx_iqlo_cal) {
				WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX, 0x06);
				if (!(pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa)) {
					/* enable G-band PAD tapoff */
					WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG, 0x31);
				} else {
					/* enable G-band INTPA tapoff */
					WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG, 0x21);
				}
			}
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_MISC1, 0x00);
		}
	} /* for */
}

static void
wlc_phy_txcal_radio_cleanup_nphy(phy_info_t *pi)
{
	uint16 core;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* Put the stream of read/write regs into a data driven loop. */
	FOREACH_CORE(pi, core) {

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MASTER,
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 0]);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_VCM_HG,
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 1]);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_IDAC,
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 2]);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_VCM,
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 3]);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX,
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 5]);

		if (RADIOREV(pi->pubpi->radiorev) != 5)
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA,
			                 pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 6]);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG,
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 7]);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_MISC1,
			pi_nphy->tx_rx_cal_radio_saveregs[(core*11) + 8]);
	} /* for */
}

static void
wlc_phy_txcal_physetup_nphy(phy_info_t *pi)
{
	uint16 val, mask;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	pi_nphy->tx_rx_cal_phy_saveregs[0] = phy_utils_read_phyreg(pi, NPHY_AfectrlCore1);
	pi_nphy->tx_rx_cal_phy_saveregs[1] = phy_utils_read_phyreg(pi, NPHY_AfectrlCore2);

	mask = (NPHY_REV3_AfectrlCore_rssi_select_i_MASK |
	    NPHY_REV3_AfectrlCore_rssi_select_q_MASK);
	val = (0x2 << NPHY_REV3_AfectrlCore_rssi_select_i_SHIFT);
	val |= (0x2 << NPHY_REV3_AfectrlCore_rssi_select_q_SHIFT);
	phy_utils_mod_phyreg(pi, NPHY_AfectrlCore1, mask, val);
	phy_utils_mod_phyreg(pi, NPHY_AfectrlCore2, mask, val);

	val = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride1);
	pi_nphy->tx_rx_cal_phy_saveregs[2] = val;
	val = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride2);
	pi_nphy->tx_rx_cal_phy_saveregs[3] = val;

	val = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride1);
	val |= (NPHY_REV3_AfectrlOverride_rssi_select_i_MASK |
	    NPHY_REV3_AfectrlOverride_rssi_select_q_MASK);
	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride1, val);

	val = phy_utils_read_phyreg(pi, NPHY_AfectrlOverride2);
	val |= (NPHY_REV3_AfectrlOverride_rssi_select_i_MASK |
	    NPHY_REV3_AfectrlOverride_rssi_select_q_MASK);
	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride2, val);

	/* Disable the re-sampler (in case spur avoidance is on) */
	pi_nphy->tx_rx_cal_phy_saveregs[4] = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_mod_phyreg(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK, 0);

	/* set 6-bit ADC to offset binary */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 3, 16, &val);
	pi_nphy->tx_rx_cal_phy_saveregs[5] = val;
	val = 0;
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 3, 16, &val);

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 19, 16, &val);
	pi_nphy->tx_rx_cal_phy_saveregs[6] = val;
	val = 0;
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 19, 16, &val);

	pi_nphy->tx_rx_cal_phy_saveregs[7] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc1);
	pi_nphy->tx_rx_cal_phy_saveregs[8] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc2);
	pi_nphy->tx_rx_cal_phy_saveregs[11] = phy_utils_read_phyreg(pi, NPHY_AntSelConfig2057);

	/* Turn off external PAs. If we are using external FEM's TSSI for the calibration
	 * then we will turn ON the PA for the specific core in the TX IQ/LO cal function
	 */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_PA, 0,
	    RADIO_MIMO_CORESEL_CORE1 | RADIO_MIMO_CORESEL_CORE2);

	/* Set the TRSW to R position: For Core0 set ant sw0 to R
	 * for core1 set ant sw1 to R.
	 */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_TRSW, 0x2,
	                                 RADIO_MIMO_CORESEL_ALLRXTX);

	/* NPHY_IPA */
	pi_nphy->tx_rx_cal_phy_saveregs[9] = phy_utils_read_phyreg(pi, NPHY_PapdEnable0);
	pi_nphy->tx_rx_cal_phy_saveregs[10] = phy_utils_read_phyreg(pi, NPHY_PapdEnable1);

	PHY_REG_LIST_START
		PHY_REG_MOD_CORE_ENTRY(NPHY, 0, PapdEnable, compEnable, 0)
		PHY_REG_MOD_CORE_ENTRY(NPHY, 1, PapdEnable, compEnable, 0)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_phy_rfctrl_override_nphy_rev7(pi,
	                                  NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
	                                  wlc_phy_read_lpf_bw_ctl_nphy(pi, 0), 0, 0,
	                                  NPHY_REV7_RFCTRLOVERRIDE_ID1);
	if (pi_nphy->nphy_use_int_tx_iqlo_cal &&
		!(pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa)) {
		/* Turn off IPA during Tx IQ/LO cal */
		if (NREV_IS(pi->pubpi->phy_rev, 7)) {
			/* Override the intpa_pu pincontrol */
			phy_utils_mod_radioreg(pi, RADIO_2057_OVR_REG0, 1<<4, 1<<4);

			/* Power down the PA by accessing the radio register */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				phy_utils_mod_radioreg(pi, RADIO_2057_PAD2G_TUNE_PUS_CORE0,
				                       1, 0);
				phy_utils_mod_radioreg(pi, RADIO_2057_PAD2G_TUNE_PUS_CORE1,
				                       1, 0);
			} else {
				phy_utils_mod_radioreg(pi,
				          RADIO_2057_IPA5G_CASCOFFV_PU_CORE0, 1, 0);
				phy_utils_mod_radioreg(pi,
				           RADIO_2057_IPA5G_CASCOFFV_PU_CORE1, 1, 0);
			}
		} else if (NREV_GE(pi->pubpi->phy_rev, 8)) {
			wlc_phy_rfctrl_override_nphy_rev7(
				pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK, 0, 0x3, 0,
				NPHY_REV7_RFCTRLOVERRIDE_ID0);
		}
	}
}

static void
wlc_phy_txcal_phycleanup_nphy(phy_info_t *pi)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	phy_utils_write_phyreg(pi, NPHY_AfectrlCore1, pi_nphy->tx_rx_cal_phy_saveregs[0]);
	phy_utils_write_phyreg(pi, NPHY_AfectrlCore2, pi_nphy->tx_rx_cal_phy_saveregs[1]);
	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride1,
	                       pi_nphy->tx_rx_cal_phy_saveregs[2]);
	phy_utils_write_phyreg(pi, NPHY_AfectrlOverride2,
	                       pi_nphy->tx_rx_cal_phy_saveregs[3]);
	phy_utils_write_phyreg(pi, NPHY_BBConfig, pi_nphy->tx_rx_cal_phy_saveregs[4]);

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 3, 16,
		&pi_nphy->tx_rx_cal_phy_saveregs[5]);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 19, 16,
		&pi_nphy->tx_rx_cal_phy_saveregs[6]);

	/* Restore Rfctrl override settings (disable PA-off override) */
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc1, pi_nphy->tx_rx_cal_phy_saveregs[7]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc2, pi_nphy->tx_rx_cal_phy_saveregs[8]);
	phy_utils_write_phyreg(pi, NPHY_AntSelConfig2057, pi_nphy->tx_rx_cal_phy_saveregs[11]);

	/* NPHY_IPA, Restore Papdcomp settings */
	phy_utils_write_phyreg(pi, NPHY_PapdEnable0, pi_nphy->tx_rx_cal_phy_saveregs[9]);
	phy_utils_write_phyreg(pi, NPHY_PapdEnable1, pi_nphy->tx_rx_cal_phy_saveregs[10]);

	wlc_phy_rfctrl_override_nphy_rev7(pi,
	                                  NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
	                                  0, 0, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);

	wlc_phy_resetcca_nphy(pi);

	if (pi_nphy->nphy_use_int_tx_iqlo_cal &&
		!(pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa)) {
		/* Power up the internal PA after Tx IQ/LOFT cal is completed */
		if (NREV_IS(pi->pubpi->phy_rev, 7)) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				phy_utils_mod_radioreg(pi, RADIO_2057_PAD2G_TUNE_PUS_CORE0,
				                       1, 1);
				phy_utils_mod_radioreg(pi, RADIO_2057_PAD2G_TUNE_PUS_CORE1,
				                       1, 1);
			} else {
				phy_utils_mod_radioreg(pi,
				         RADIO_2057_IPA5G_CASCOFFV_PU_CORE0, 1, 1);
				phy_utils_mod_radioreg(pi,
				         RADIO_2057_IPA5G_CASCOFFV_PU_CORE1, 1, 1);
			}
			/* Reset the override of the intpa_pu pincontrol */
			phy_utils_mod_radioreg(pi, RADIO_2057_OVR_REG0, 1<<4, 0);
		} else if (NREV_GE(pi->pubpi->phy_rev, 8)) {
			wlc_phy_rfctrl_override_nphy_rev7(
				pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK, 0, 0x3, 1,
				NPHY_REV7_RFCTRLOVERRIDE_ID0);
		}
	}
}

#define NPHY_CAL_TSSISAMPS	64
#define NPHY_TEST_TONE_FREQ_40MHz 4000
#define NPHY_TEST_TONE_FREQ_20MHz 2500

void
wlc_phy_est_tonepwr_nphy(phy_info_t *pi, int32 *qdBm_pwrbuf, uint8 num_samps)
{
	uint16 tssi_reg;
	int32 temp, pwrindex[NPHY_CORE_NUM];
	int32 idle_tssi[NPHY_CORE_NUM];
	int32 rssi_buf[4];
	int32 tssival[NPHY_CORE_NUM];
	uint8 tssi_type;

	/* Get the Idle TSSI values */
	tssi_reg = phy_utils_read_phyreg(pi, NPHY_TxPwrCtrlIdleTssi);

	temp = (int32) (tssi_reg & 0x3f);
	idle_tssi[0] = (temp <= 31)? temp : (temp - 64);

	temp = (int32) ((tssi_reg >> 8) & 0x3f);
	idle_tssi[1] = (temp <= 31)?  temp : (temp - 64);

	tssi_type =
	    CHSPEC_IS5G(pi->radio_chanspec)?
	        (uint8)NPHY_RSSI_SEL_TSSI_5G : (uint8)NPHY_RSSI_SEL_TSSI_2G;

	wlc_phy_poll_rssi_nphy(pi, tssi_type, rssi_buf, num_samps);

	tssival[0] = rssi_buf[0] / ((int32)num_samps);
	tssival[1] = rssi_buf[2] / ((int32)num_samps);

	pwrindex[0] = idle_tssi[0] - tssival[0] + 64;
	pwrindex[1] = idle_tssi[1] - tssival[1] + 64;

	if (pwrindex[0] < 0) {
		pwrindex[0] = 0;
	} else if (pwrindex[0] > 63) {
		pwrindex[0] = 63;
	}

	if (pwrindex[1] < 0) {
		pwrindex[1] = 0;
	} else if (pwrindex[1] > 63) {
		pwrindex[1] = 63;
	}

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_CORE1TXPWRCTL, 1,
		(uint32)pwrindex[0], 32, &qdBm_pwrbuf[0]);
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_CORE2TXPWRCTL, 1,
		(uint32)pwrindex[1], 32, &qdBm_pwrbuf[1]);
}

static void
wlc_phy_precal_txgain_nphy(phy_info_t *pi)
{
	bool save_bbmult = FALSE;
	bool cal_target_gain_set_from_idx = TRUE;
	nphy_txgains_t target_gain;
	uint8 txcal_index_2057_rev5n7 = 0;
	uint8 txcal_index_2057_rev5n7_2g = 10;
	uint8 txcal_index_2057_rev3n4n6 = 10;
	int8 target_tssi;
	int8 init_gc_idx;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi_nphy->nphy_use_int_tx_iqlo_cal) {
		if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
		    (RADIOREV(pi->pubpi->radiorev) == 4) ||
		    (RADIOREV(pi->pubpi->radiorev) == 6)) {
			/* 43226a[01] and 6362a0 */
			pi_nphy->nphy_txcal_pwr_idx[0] = txcal_index_2057_rev3n4n6;
			pi_nphy->nphy_txcal_pwr_idx[1] = txcal_index_2057_rev3n4n6;
			wlc_phy_txpwr_index_nphy(pi, 3,
			                         txcal_index_2057_rev3n4n6, FALSE);
			save_bbmult = TRUE;
		} else {
			/* 5357a0/b0, 43236a0, 43236b0, and 6362b0 */
			if (PHY_IPA(pi)) {
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					/* 5357b[01] use precal gain ctrl */
					if ((RADIOREV(pi->pubpi->radiorev) == 5) &&
					     (RADIOVER(pi->pubpi->radiover) > 0)) {
						cal_target_gain_set_from_idx = FALSE;
						target_tssi = 35;
						init_gc_idx = 11;
						target_gain =
						        wlc_phy_cal_txgainctrl_inttssi_nphy(
							pi, target_tssi, init_gc_idx);

					} else if ((RADIOREV(pi->pubpi->radiorev) == 13) ||
						(RADIOREV(pi->pubpi->radiorev) == 14)) {
						cal_target_gain_set_from_idx = FALSE;
						target_tssi = 25;
						init_gc_idx = 11;
						target_gain =
						        wlc_phy_cal_txgainctrl_inttssi_nphy(
							pi, target_tssi, init_gc_idx);
					/* 43236b[01] use precal gain ctrl */
					} else if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
					    (RADIOVER(pi->pubpi->radiover) > 0)) {
						cal_target_gain_set_from_idx = FALSE;
						target_tssi = 35;
						init_gc_idx = 8;
						target_gain =
						        wlc_phy_cal_txgainctrl_inttssi_nphy(
							pi, target_tssi, init_gc_idx);
					} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
						/* BCM63268 */
						cal_target_gain_set_from_idx = FALSE;
						target_tssi = 35;
						init_gc_idx = 8;
						target_gain =
						        wlc_phy_cal_txgainctrl_inttssi_nphy(
							pi, target_tssi, init_gc_idx);
					/* others use fixed Tx gain index */
					} else {
						pi_nphy->nphy_txcal_pwr_idx[0] =
						        txcal_index_2057_rev5n7_2g;
						pi_nphy->nphy_txcal_pwr_idx[1] =
						        txcal_index_2057_rev5n7_2g;
						wlc_phy_txpwr_index_nphy(
							pi, 3, txcal_index_2057_rev5n7_2g,
							FALSE);
						save_bbmult = TRUE;
					}

				/* 5G */
				} else {
					/* 43236b[01] use precal gain ctrl */
					if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
					    (RADIOVER(pi->pubpi->radiover) > 0)) {
						cal_target_gain_set_from_idx = FALSE;
						target_tssi = 12;
						init_gc_idx = 11;
						target_gain =
						   wlc_phy_cal_txgainctrl_inttssi_nphy(
							pi, target_tssi, init_gc_idx);
					} else {
						pi_nphy->nphy_txcal_pwr_idx[0] =
						        txcal_index_2057_rev5n7;
						pi_nphy->nphy_txcal_pwr_idx[1] =
						        txcal_index_2057_rev5n7;
						wlc_phy_txpwr_index_nphy(pi, 3,
						              txcal_index_2057_rev5n7,
						              FALSE);
						save_bbmult = TRUE;
					}
				}

			/* external PA */
			} else {
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					if (((RADIOREV(pi->pubpi->radiorev) == 5) &&
					     (RADIOVER(pi->pubpi->radiover) > 0)) ||
					    (RADIOREV(pi->pubpi->radiorev) >= 7)) {
						cal_target_gain_set_from_idx = FALSE;
						target_tssi = 20;
						init_gc_idx = 11;
						target_gain =
						        wlc_phy_cal_txgainctrl_inttssi_nphy(
							pi, target_tssi, init_gc_idx);
					} else {
						pi_nphy->nphy_txcal_pwr_idx[0] =
						        txcal_index_2057_rev5n7;
						pi_nphy->nphy_txcal_pwr_idx[1] =
						        txcal_index_2057_rev5n7;
						wlc_phy_txpwr_index_nphy(
							pi, 3, txcal_index_2057_rev5n7,
							FALSE);
						save_bbmult = TRUE;
					}

				} else {
					if (pi->sh->boardtype == 0xF52A) {
						/* correspond sto Cisco E3200 brd */
						pi_nphy->nphy_txcal_pwr_idx[0] =
						        txcal_index_2057_rev5n7;
						pi_nphy->nphy_txcal_pwr_idx[1] =
						        txcal_index_2057_rev5n7;
						wlc_phy_txpwr_index_nphy(pi, 3,
						        txcal_index_2057_rev5n7,
						                         FALSE);
						save_bbmult = TRUE;
					} else {
						/* 43236b[01] use precal gain ctrl */
						cal_target_gain_set_from_idx = FALSE;
						target_tssi = 10;
						init_gc_idx = 12;
						target_gain =
						   wlc_phy_cal_txgainctrl_inttssi_nphy(
							pi, target_tssi, init_gc_idx);
					}
				}
			}
		}
	} else { /* external PA */
		wlc_phy_cal_txgainctrl_nphy(pi, 10, FALSE);
	}

	if (save_bbmult) {
		wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &pi_nphy->nphy_txcal_bbmult);
	}

	if (cal_target_gain_set_from_idx) {
		wlc_phy_get_tx_gain_nphy(pi, &pi_nphy->nphy_cal_target_gain);
		wlc_phy_txpwr_index_nphy(pi, 3, -1, FALSE);
	} else {
		pi_nphy->nphy_cal_target_gain = target_gain;
	}
}

/** Function that maintains the Av/Vmid settings to be used for pre-cal gain control */
void wlc_phy_tx_iqlo_precal_gctrl_auxadc_nphy(phy_info_t *pi)
{
	uint16 aux_adc_gain = 5;
	uint16 aux_adc_vmid_core0 = 0x19e;
	uint16 aux_adc_vmid_core1 = 0x19e;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa) {
		PHY_ERROR(("intPA TSSI needs characterization\n"));
	} else {
		/* 2.4GHz */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (PHY_IPA(pi)) {
				if (((RADIOREV(pi->pubpi->radiorev) == 5) &&
					(RADIOVER(pi->pubpi->radiover) > 0)) ||
				    (RADIOREV(pi->pubpi->radiorev) == 13) ||
				    (RADIOREV(pi->pubpi->radiorev) == 14)) {
					aux_adc_gain = 2;
					aux_adc_vmid_core0 = 0x1a9;
					aux_adc_vmid_core1 = 0x1a9;
				} else if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
					(RADIOVER(pi->pubpi->radiover) > 0)) {
					aux_adc_gain = 0;
					aux_adc_vmid_core0 = 0x8c;
					aux_adc_vmid_core1 = 0x90;
				} else if (RADIOVER(pi->pubpi->radiorev) == 9) {
					aux_adc_gain = 1;
					aux_adc_vmid_core0 = 0x7c;
					aux_adc_vmid_core1 = 0x7c;
				} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
					/* BCM63268 */
					aux_adc_gain = 0;
					aux_adc_vmid_core0 = 0x8c;
					aux_adc_vmid_core1 = 0x90;
				} else {
					/* Internal PA boards */
					PHY_ERROR(("2G PAD TSSI needs characterization\n"));
				}

			} else {
				/* External PA boards */
				if (((RADIOREV(pi->pubpi->radiorev) == 5) &&
					(RADIOVER(pi->pubpi->radiover) > 0)) ||
					(RADIOREV(pi->pubpi->radiorev) >= 7))
				{
					aux_adc_gain = 5;
					aux_adc_vmid_core0 = 0x19e;
					aux_adc_vmid_core1 = 0x19e;
				} else {
					PHY_ERROR(("2G PAD TSSI needs characterization\n"));
				}
			}
		/* 5GHz */
		} else {
			/* 4323X B0, B1 */
			if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
			    (RADIOVER(pi->pubpi->radiover) > 0)) {
				if (PHY_IPA(pi)) {
					aux_adc_gain = 0;
					aux_adc_vmid_core0 = 0x8b;
					aux_adc_vmid_core1 = 0x8b;
				} else {
					aux_adc_gain = 3;
					aux_adc_vmid_core0 = 0x50;
					aux_adc_vmid_core1 = 0x50;
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
				/* BCM63268 */
				aux_adc_gain = 0;
				aux_adc_vmid_core0 = 0x8b;
				aux_adc_vmid_core1 = 0x8b;
			} else {
				PHY_ERROR(("5G PAD TSSI needs characterization\n"));
			}
		}
	}

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x0b, 16, &aux_adc_vmid_core0);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1b, 16, &aux_adc_vmid_core1);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x0f, 16, &aux_adc_gain);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1f, 16, &aux_adc_gain);
}

nphy_txgains_t
wlc_phy_cal_txgainctrl_inttssi_nphy(phy_info_t *pi, int8 target_tssi, int8 init_gc_idx)
{
	int gainctrl_loopidx;
	uint core;
	uint16 m0m1, curr_m0m1;
	uint8 bbmult_val;
	uint16 orig_BBConfig;
	uint16 phy_saveregs[3];
	uint32 freq_test;
	uint16 ampl_test = 250;
	bool phyhang_avoid_state = FALSE;
	nphy_txgains_t target_gain, target_gain_prev;
	int8 gain_list_2057_2g[] = {1, 2, 3, 4, 5, 6, 8, 10, 12, 15, 19, 24, 31};
	int8 gain_list_2057_5g[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int8 *gain_list;
	uint16 gain_list_len;
	int8 tssi_delta;
	int8 tssi_delta_prev;
	uint16 aux_adc_gain_save[NPHY_CORE_NUM];
	uint16 aux_adc_vmid_save[NPHY_CORE_NUM];
	int8 idle_tssi_core[NPHY_CORE_NUM];
	int8 meas_tssi;
	int8 curr_index;
	uint16 rad_gain;
	int32 rssi_buf[4];
	uint8 tssi_type;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		freq_test = 5000;
	} else {
		freq_test = 2500;
	}

	tssi_type = CHSPEC_IS5G(pi->radio_chanspec)? (uint8)NPHY_RSSI_SEL_TSSI_5G :
	        (uint8)NPHY_RSSI_SEL_TSSI_2G;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		gain_list = gain_list_2057_2g;
		gain_list_len = sizeof(gain_list_2057_2g) / sizeof(gain_list_2057_2g[0]);
	} else {
	        gain_list = gain_list_2057_5g;
		gain_list_len = sizeof(gain_list_2057_5g) / sizeof(gain_list_2057_5g[0]);
	}

	if (init_gc_idx > gain_list_len - 1) {
		PHY_CAL(("init_gc_idx, %d, is larger than the number of gain codes considered."
		         "Limiting init_gc_idx to maximum gain code index %d\n",
		         init_gc_idx, gain_list_len - 1));
		init_gc_idx = gain_list_len - 1;
	}

	if (init_gc_idx < 0) {
		PHY_CAL(("init_gc_idx cannot be negative. Setting it to 0\n"));
		init_gc_idx = 0;
	}

	/* Save Av/Vmid values for TSSI */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0xb, 16,
	                         &aux_adc_vmid_save[0]);
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0xf, 16,
	                         &aux_adc_gain_save[0]);
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1b, 16,
	                         &aux_adc_vmid_save[1]);
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1f, 16,
	                         &aux_adc_gain_save[1]);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Disable phyhang_avoid so that resetCCA doesn't mess with our
	 * antenna settings
	*/
	phyhang_avoid_state = pi_nphy->phyhang_avoid;
	pi_nphy->phyhang_avoid   = FALSE;

	/* Disable the re-sampler (in case we are in spur avoidance mode) */
	orig_BBConfig = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_mod_phyreg(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK, 0);

	/* Setup TSSI path at the Tx IQ/LO cal detector path
	 * Save the previous envelope detector/mux settings and then modify
	 */
	wlc_phy_ipa_internal_tssi_setup_nphy(pi, !(pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa));

	/* Set the Av/Vmid values for the desired TSSI tap-off */
	wlc_phy_tx_iqlo_precal_gctrl_auxadc_nphy(pi);

	phy_saveregs[0] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc1);
	phy_saveregs[1] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc2);
	phy_saveregs[2] = phy_utils_read_phyreg(pi, NPHY_AntSelConfig2057);

	/* Set all RfctrlIntc flags to 0 */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_OFF, 0,
		RADIO_MIMO_CORESEL_CORE1 | RADIO_MIMO_CORESEL_CORE2);

	/* Turn off external PAs to avoid PA blowup during the calibration and to avoid spewing
	 * tones into the air
	 */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_PA, 0,
		RADIO_MIMO_CORESEL_CORE1 | RADIO_MIMO_CORESEL_CORE2);

	if (!(pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa)) {
		/* Turn IPA off before pre-cal gain control */
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK, 0, 0x3, 0,
			NPHY_REV7_RFCTRLOVERRIDE_ID0);
	}

	/* Set the TRSW to R position: For Core0 set ant sw0 to R
	 * for core1 set ant sw1 to R.
	 */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_TRSW, 0x2,
	                                 RADIO_MIMO_CORESEL_ALLRXTX);

	/* Measure idle TSSI at Tx IQ/LO cal tapoff point */

	/* Set the Tx gain to 0, so that LO leakage will not affect the IDLE Tssi */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
	                                  0, 0x3, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_tx_tone_nphy(pi, 4000, 0, 0, 0, FALSE);

	OSL_DELAY(20);
	wlc_phy_poll_rssi_nphy(pi, tssi_type, rssi_buf, 1);
	wlc_phy_stopplayback_nphy(pi);

	/* Remove the Tx gain override */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	idle_tssi_core[0] = (int8) rssi_buf[0];
	idle_tssi_core[1] = (int8) rssi_buf[2];

	PHY_CAL(("Pre Tx IQ/LO cal gctrl idle_tssi_core0: %d\n", idle_tssi_core[0]));
	PHY_CAL(("Pre Tx IQ/LO cal gctrl idle_tssi_core1: %d\n", idle_tssi_core[1]));

	/* Get the RF Tx gain */
	wlc_phy_get_tx_gain_nphy(pi, &target_gain);

	/* Set default bbmult = 64 and save the bbmult value.
	 * This will be used to scale the IQcal bbmult values in the gain ladders
	 * 5357B1EPA will need to use a lower bbmult at 45, high bbmult will have bad IMGT
	*/
	if ((RADIOREV(pi->pubpi->radiorev) == 5) && (RADIOVER(pi->pubpi->radiover) >= 2)) {
		bbmult_val = 45;
	} else {
		bbmult_val = 64;
	}
	wlc_phy_ipa_set_bbmult_nphy(pi, bbmult_val, bbmult_val);
	m0m1 = (bbmult_val << 8) | bbmult_val;
	pi_nphy->nphy_txcal_bbmult = m0m1;

	FOREACH_CORE(pi, core) {
		tssi_delta_prev = 0;
		wlc_phy_get_tx_gain_nphy(pi, &target_gain_prev);

		/* Remove the Tx gain override */
		wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
		                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);

		curr_index = init_gc_idx;

		/* Zero the bbmult for the core which is not of interest */
		if (core == PHY_CORE_0) {
			curr_m0m1 = m0m1 & 0xff00;
		} else {
			curr_m0m1 = m0m1 & 0x00ff;
		}

		for (gainctrl_loopidx = 0; gainctrl_loopidx < 5; gainctrl_loopidx++) {
			wlc_phy_tx_tone_nphy(pi, freq_test, ampl_test, 0, 0, FALSE);

			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &curr_m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &curr_m0m1);

			/* Vary the PAD gain in the 2G band and the PGA in the 5G band */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				target_gain.pad[core] = gain_list[curr_index];
			} else {
				target_gain.pga[core] = gain_list[curr_index];
			}

			/* For Ext PA brds, the gain table will have low gain and
			 * hence, we force PAD gain to 15 for Tx IQLO cal
			 */
			if ((CHSPEC_IS5G(pi->radio_chanspec)) && !PHY_IPA(pi)) {
				target_gain.pad[core] = 15;
			}
			rad_gain = ((target_gain.txlpf[core] << 15) |
			            (target_gain.txgm[core] << 12) |
			            (target_gain.pga[core] << 8) |
			            (target_gain.pad[core] << 3) |
			            (target_gain.ipa[core]));

			/* Set the RF gain */
			wlc_phy_rfctrl_override_nphy_rev7(
				pi, NPHY_REV7_RfctrlOverride_txgain_MASK, rad_gain, 0x3, 0,
				NPHY_REV7_RFCTRLOVERRIDE_ID0);

			OSL_DELAY(50);

			/* measure TSSI */
			wlc_phy_poll_rssi_nphy(pi, tssi_type, rssi_buf, 10);
			meas_tssi = (int8)(rssi_buf[core * 2] / 10) - idle_tssi_core[core];

			PHY_CAL(("wlc_phy_cal_txgainctrl_inttssi_nphy:  meas_tssi = %d, rad_gain = "
			         "%x, curr_index = %d\n", meas_tssi, rad_gain, curr_index));

			pi_nphy->nphy_bb_mult_save = 0;
			wlc_phy_stopplayback_nphy(pi);

			/* compute new index based on difference between target and measured TSSI */
			tssi_delta = meas_tssi - target_tssi;
			if ((gainctrl_loopidx > 0) && (tssi_delta*tssi_delta_prev <= 0)) {
				if (ABS(tssi_delta_prev) < ABS(tssi_delta))
					target_gain = target_gain_prev;

				break;
			} else if (tssi_delta < 0) {
				curr_index++;
			} else {
				curr_index--;
			}
			tssi_delta_prev = tssi_delta;
			target_gain_prev = target_gain;
			if ((curr_index > gain_list_len - 1) || (curr_index < 0))
				break;
		}
		PHY_CAL(("Pre-cal gain control on core%d converged: txgain: pad = %d,"
			         " pga = %d, txgm = %d, txlpf = %d\n", core,
			         target_gain.pad[core], target_gain.pga[core],
			         target_gain.txgm[core], target_gain.txlpf[core]));
	}

	/* Remove the Tx gain override */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	/* Restore the state of the re-sampler (in case we are in spur avoidance mode) */
	phy_utils_write_phyreg(pi, NPHY_BBConfig, orig_BBConfig);

	/* Restore Rfctrl override settings (PA and TRSW)  */
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc1, phy_saveregs[0]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc2, phy_saveregs[1]);
	phy_utils_write_phyreg(pi, NPHY_AntSelConfig2057, phy_saveregs[2]);

	if (!(pi_nphy->nphy_int_tx_iqlo_cal_tapoff_intpa)) {
		/* Turn IPA on after pre-cal gain control */
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK, 0, 0x3, 1,
			NPHY_REV7_RFCTRLOVERRIDE_ID0);
	}

	/* Restore the previous envelope detector/mux settings */
	wlc_phy_internal_tssi_cleanup_nphy(pi);

	/* Restore previous Av/Vmid values for TSSI */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0xb, 16,
	                         &aux_adc_vmid_save[0]);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0xf, 16,
	                         &aux_adc_gain_save[0]);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1b, 16,
	                         &aux_adc_vmid_save[1]);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_AFECTRL, 1, 0x1f, 16,
	                         &aux_adc_gain_save[1]);

	/* Restore phyhang_avoid state  */
	pi_nphy->phyhang_avoid = phyhang_avoid_state;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	return (target_gain);
}

void
wlc_phy_cal_txgainctrl_nphy(phy_info_t *pi, int32 dBm_targetpower, bool debug)
{
	int gainctrl_loopidx;
	uint core;
	uint16 m0m1, curr_m0m1;
	int32 delta_power;
	int32 txpwrindex;
	int32 qdBm_power[NPHY_CORE_NUM];
	uint16 orig_BBConfig;
	uint16 phy_saveregs[4];
	uint32 freq_test;
	uint16 ampl_test = 250;
	uint stepsize;
	bool phyhang_avoid_state = FALSE;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* For REV7 and higher, the Tx gain indices indicate 0.5 dB step jumps instead of
	 * the 0.25 dB steps that were used in REVs 1 through 6
	 */
	/* mimophy rev 7 and higher */
	stepsize = 2;

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		freq_test = 5000;
	} else {
		freq_test = 2500;
	}

	/* Set the Tx gain, corresponding to the power index values, that we read before
	   h/w power control was disabled
	*/
	wlc_phy_txpwr_index_nphy(pi, 1, pi_nphy->nphy_cal_orig_pwr_idx[0], TRUE);
	wlc_phy_txpwr_index_nphy(pi, 2, pi_nphy->nphy_cal_orig_pwr_idx[1], TRUE);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Disable phyhang_avoid so that resetCCA doesn't mess with our
	 * antenna settings
	*/
	phyhang_avoid_state = pi_nphy->phyhang_avoid;
	pi_nphy->phyhang_avoid   = FALSE;

	/* Enable the external PA on both cores, since we are using
	 * the external TSSI for TX gainctrl.
	 *
	 * Also set the TRSW to R position: For Core0 set ant sw0 to R
	 * for core1 set ant sw1 to R.
	 */
	phy_saveregs[0] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc1);
	phy_saveregs[1] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc2);
	phy_saveregs[2] = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride0);
	phy_saveregs[3] = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride1);
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_PA, 0,
		RADIO_MIMO_CORESEL_CORE1 | RADIO_MIMO_CORESEL_CORE2);

	if (! debug) {
		wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_TRSW, 0x2,
		                                 RADIO_MIMO_CORESEL_ALLRXTX);
	} else {
		wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_TRSW, 0x1,
		                                 RADIO_MIMO_CORESEL_ALLRXTX);
	}

	/* Disable the re-sampler (in case we are in spur avoidance mode) */
	orig_BBConfig = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_mod_phyreg(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK, 0);

	/* Read the current bbmult values */
	wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

	FOREACH_CORE(pi, core) {
		txpwrindex = (int32) pi_nphy->nphy_cal_orig_pwr_idx[core];

		for (gainctrl_loopidx = 0; gainctrl_loopidx < 2; gainctrl_loopidx++) {
			wlc_phy_tx_tone_nphy(pi, freq_test, ampl_test, 0, 0, FALSE);

			/* First play the tone and then switch ON the extPA in order to avoid
			 * a current spike that exceeds 500mA
			 */
			OSL_DELAY(50);
			wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_PA, 1,
			         (core == 0)? RADIO_MIMO_CORESEL_CORE1 : RADIO_MIMO_CORESEL_CORE2);

			/* Zero the bbmult for the core which is not of interest */
			if (core == PHY_CORE_0) {
				curr_m0m1 = m0m1 & 0xff00;
			} else {
				curr_m0m1 = m0m1 & 0x00ff;
			}

			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &curr_m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &curr_m0m1);

			OSL_DELAY(50);

			/* Get the average tone power */
			wlc_phy_est_tonepwr_nphy(pi, qdBm_power, NPHY_CAL_TSSISAMPS);

			pi_nphy->nphy_bb_mult_save = 0;
			wlc_phy_stopplayback_nphy(pi);

			wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_PA, 0,
			                     RADIO_MIMO_CORESEL_CORE1 | RADIO_MIMO_CORESEL_CORE2);

			delta_power = (dBm_targetpower * 4) - qdBm_power[core];

			txpwrindex -= stepsize * delta_power;
			if (txpwrindex < 0) {
				txpwrindex = 0;
			} else if (txpwrindex > 127) {
				txpwrindex = 127;
			}

			/* Limit the Tx Power for boards with high power external PA */
			if (!CHSPEC_IS5G(pi->radio_chanspec)) {
				if (pi->fem2g->extpagain == 3) {
					if (txpwrindex < 50) {
						txpwrindex = 50;
					}
				}
			}

			wlc_phy_txpwr_index_nphy(pi, (1 << core), (uint8)txpwrindex, TRUE);
		}

		pi_nphy->nphy_txcal_pwr_idx[core] = (uint8) txpwrindex;

		if (debug) {
			uint16 radio_gain;
			uint16 dbg_m0m1;

			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &dbg_m0m1);

			wlc_phy_tx_tone_nphy(pi, freq_test, ampl_test, 0, 0, FALSE);

			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &dbg_m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &dbg_m0m1);

			OSL_DELAY(100);

			wlc_phy_est_tonepwr_nphy(pi, qdBm_power, NPHY_CAL_TSSISAMPS);

			wlc_phy_table_read_nphy(pi, 7, 1, (0x110+core), 16, &radio_gain);

			PHY_ERROR(("Check output power on spectrum analyzer for core=%d,"
			    "txpwrindex=%d, estpower=%d, radio_gain=0x%x\n",
			    core, txpwrindex, qdBm_power[core]/4, radio_gain));
			OSL_DELAY(4000000);
			pi_nphy->nphy_bb_mult_save = 0;
			wlc_phy_stopplayback_nphy(pi);
		}
	}

	wlc_phy_txpwr_index_nphy(pi, 1, pi_nphy->nphy_txcal_pwr_idx[0], TRUE);
	wlc_phy_txpwr_index_nphy(pi, 2, pi_nphy->nphy_txcal_pwr_idx[1], TRUE);

	/* Save the bbmult value. This will be used to scale the IQcal bbmult
	   values in the gain ladders
	*/
	wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &pi_nphy->nphy_txcal_bbmult);

	/* Restore the state of the re-sampler (in case we are in spur avoidance mode) */
	phy_utils_write_phyreg(pi, NPHY_BBConfig, orig_BBConfig);

	/* Restore Rfctrl override settings (PA and TRSW)  */
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc1, phy_saveregs[0]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc2, phy_saveregs[1]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride0, phy_saveregs[2]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride1, phy_saveregs[3]);

	/* Restore phyhang_avoid state  */
	pi_nphy->phyhang_avoid = phyhang_avoid_state;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

#define IQ_LADDER_DEPTH 18
#define IQ_LADDER_DEPTH_REV19 22
#define IQ_LADDER_DEPTH_B1 20

static void
wlc_phy_update_txcal_ladder_nphy(phy_info_t *pi, uint16 core)
{
	int indx;
	uint32 bbmult_scale;
	uint16 bbmult;
	uint16 tblentry;
	uint8 lo_ladder_length;

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	nphy_txiqcal_ladder_t ladder_lo_normal[] = {
	{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
	{25, 0}, {25, 1}, {25, 2}, {25, 3}, {25, 4}, {25, 5},
	{25, 6}, {25, 7}, {35, 7}, {50, 7}, {71, 7}, {100, 7}};

	nphy_txiqcal_ladder_t *ladder_lo;

	bbmult = (core == PHY_CORE_0)?
	((pi_nphy->nphy_txcal_bbmult >> 8) & 0xff) : (pi_nphy->nphy_txcal_bbmult & 0xff);

	ladder_lo = ladder_lo_normal;
	lo_ladder_length = 18;

	for (indx = 0; indx < lo_ladder_length; indx++) {
		bbmult_scale = ladder_lo[indx].percent * bbmult;
		bbmult_scale /= 100;

		tblentry = ((bbmult_scale & 0xff) << 8) | ladder_lo[indx].g_env;
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, indx, 16, &tblentry);
	}

	for (indx = 0; indx < IQ_LADDER_DEPTH; indx++) {
		nphy_txiqcal_ladder_t ladder_iq[] = {
		{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
		{25, 0}, {35, 0}, {50, 0}, {71, 0}, {100, 0}, {100, 1},
		{100, 2}, {100, 3}, {100, 4}, {100, 5}, {100, 6}, {100, 7}};

		bbmult_scale = ladder_iq[indx].percent * bbmult;
		bbmult_scale /= 100;

		tblentry = ((bbmult_scale & 0xff) << 8) | ladder_iq[indx].g_env;
		wlc_phy_table_write_nphy(pi,
		NPHY_TBL_ID_IQLOCAL, 1, indx+32, 16, &tblentry);
	} /* for iq_ladder_depth */
}

void
wlc_phy_lcnxn_rx2tx_stallwindow_nphy(phy_info_t *pi, uint8 stallON)
{
	if (stallON == 0) {
		/* Rx stall clocks ON only within a window */
		phy_utils_mod_phyreg(pi, NPHY_AfePuCtrl,
			NPHY_AfePuCtrl_use_rfctrl_adc_pu_MASK,
			1 << NPHY_AfePuCtrl_use_rfctrl_adc_pu_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_Afeseqctrl,
			NPHY_REV19_Afeseqctrl_keep_adc_on_during_entire_tx_MASK,
			0 << NPHY_REV19_Afeseqctrl_keep_adc_on_during_entire_tx_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_rxfeFifoCtrl,
			NPHY_REV19_RxfeFifoCtrl_En_rxfeFifo_Reset_MASK,
			1 << NPHY_REV19_RxfeFifoCtrl_En_rxfeFifo_Reset_SHIFT);
		phy_utils_write_phyreg(pi, NPHY_REV19_AfeseqRx2TxAdcPwrDownDly40M, 4800);
		phy_utils_write_phyreg(pi, NPHY_REV19_AfeseqRx2TxAdcPwrDownDly20M, 2400);
		phy_utils_mod_phyreg(pi, NPHY_REV19_rxfeFifoCtrl,
			NPHY_REV19_rxfeFifoCtrl_rxfeFifoResetCntVal_MASK,
			7 << NPHY_REV19_rxfeFifoCtrl_rxfeFifoResetCntVal_SHIFT);

		phy_utils_write_phyreg(pi, NPHY_AfeseqRx2TxPwrUpDownDly40M, 640);
		phy_utils_write_phyreg(pi, NPHY_AfeseqRx2TxPwrUpDownDly20M, 320);

	} else {
		/* Restore default values for the registers */
		phy_utils_mod_phyreg(pi, NPHY_AfePuCtrl,
			NPHY_AfePuCtrl_use_rfctrl_adc_pu_MASK,
			1 << NPHY_AfePuCtrl_use_rfctrl_adc_pu_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_Afeseqctrl,
			NPHY_REV19_Afeseqctrl_keep_adc_on_during_entire_tx_MASK,
			1 << NPHY_REV19_Afeseqctrl_keep_adc_on_during_entire_tx_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_rxfeFifoCtrl,
			NPHY_REV19_RxfeFifoCtrl_En_rxfeFifo_Reset_MASK,
			1 << NPHY_REV19_RxfeFifoCtrl_En_rxfeFifo_Reset_SHIFT);
		phy_utils_write_phyreg(pi, NPHY_REV19_AfeseqRx2TxAdcPwrDownDly40M, 0x3);
		phy_utils_write_phyreg(pi, NPHY_REV19_AfeseqRx2TxAdcPwrDownDly20M, 0x3);
		phy_utils_mod_phyreg(pi, NPHY_REV19_rxfeFifoCtrl,
			NPHY_REV19_rxfeFifoCtrl_rxfeFifoResetCntVal_MASK,
			4 << NPHY_REV19_rxfeFifoCtrl_rxfeFifoResetCntVal_SHIFT);

		phy_utils_write_phyreg(pi, NPHY_AfeseqRx2TxPwrUpDownDly40M, 0x3);
		phy_utils_write_phyreg(pi, NPHY_AfeseqRx2TxPwrUpDownDly20M, 0x3);
	}
}

void
wlc_phy_cal_perical_nphy_run(phy_info_t *pi, uint8 caltype)
{
	nphy_txgains_t target_gain;
	uint8 tx_pwr_ctrl_state;
	bool fullcal = TRUE;
	bool restore_tx_gain = FALSE;
	bool mphase;
	uint8 entries[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int cal_result;
#ifdef WLSRVSDB
	uint8 i;
#endif // endif

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: Running NPHY periodic calibration chanspec 0x%x phase:%d\n",
		pi->sh->unit,
		pi->radio_chanspec, pi->cal_info->cal_phase_id));

	/* Exit immediately if we are running on Quickturn */
	if (NORADIO_ENAB(pi->pubpi) || (!pi->radio_is_on)) {
		phy_calmgr_mphase_reset(pi->calmgri);
		return;
	}

	/* skip cal if phy is muted */
	if (PHY_MUTED(pi))
		return;

	ASSERT(pi->phy_cal_mode != PHY_PERICAL_DISABLE);

	/*
	 * AUTO mode: If the last calibration was on the current channel, do a partial
	 * calibration, otherwise do a FULL calibration
	 * non-auto mode: full or partial based on caltype
	 */
	if (caltype == PHY_PERICAL_AUTO)
		fullcal = (pi->radio_chanspec != pi->cal_info->u.ncal.txiqlocal_chanspec);
	else if (caltype == PHY_PERICAL_PARTIAL)
		fullcal = FALSE;

	if (pi_nphy->cal_type_override != PHY_PERICAL_AUTO) {
		fullcal = (pi_nphy->cal_type_override == PHY_PERICAL_FULL)? TRUE : FALSE;
	}

#ifdef PHYCAL_CACHING
	if (!ctx) {
#endif /* PHYCAL_CACHING */
		/*
		 * If the previous phase of a multiphase calibration was on a different channel,
		 * then restart the multiphase calibration from the beginning on CURRENT channel
		 */
		if (pi->cal_info->cal_phase_id > MPHASE_CAL_STATE_INIT) {
			if (pi->cal_info->u.ncal.txiqlocal_chanspec != pi->radio_chanspec)
				phy_calmgr_mphase_restart(pi->calmgri);
		}
#ifdef PHYCAL_CACHING
	}
#endif /* PHYCAL_CACHING */

#ifdef WLSRVSDB
	/* Avoid split cals on multiple channels */
	if (!SCAN_RM_IN_PROGRESS(pi)) {
		if (pi->srvsdb_state->srvsdb_active && !pi->srvsdb_state->force_vsdb) {
			if (pi->cal_info->cal_phase_id > MPHASE_CAL_STATE_INIT) {
				/* Return if a chan mismatch happens before full cal finishes */
				if (pi->cal_info->u.ncal.txiqlocal_chanspec !=
					pi->radio_chanspec) {
					return;
				}
			} else if (pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_INIT) {
				/* Make sure both channels get equal chance to do a perical */
				if (pi->radio_chanspec == pi->srvsdb_state->last_cal_chanspec) {
					return;
				}

			}
			/* Triggering Cal. so invalidate saved radio regs */
			for (i = 0; i < SR_MEMORY_BANK; i++) {
				if (CHSPEC_CHANNEL(pi->radio_chanspec) ==
					pi->srvsdb_state->sr_vsdb_channels[i]) {
					pi->srvsdb_state->swbkp_snapshot_valid[i] = 0;
					pi->srvsdb_state->last_cal_chanspec = pi->radio_chanspec;
				}
			}
		}
	}
#endif /* WLSRVSDB */

	/* Make the ucode send a CTS-to-self packet with duration set to 10ms. This
	 * prevents packets from other STAs/AP from interfering with Rx IQcal
	 */
	if (pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_RXCAL) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION(pi), 10000);
	}

	/* send out CTS-to-self before PAPD cal
	 * using ~31ms for now which is ~max available; might or might not be enough;
	 * this is executed by ucode when ucode is suspended a few lines
	 * below here;
	 * for a future case where the initial duration is chosen too long,
	 * can consider using a "CF End" packet (need to add) when ucode
	 * resumes after the end of the calibration
	 */
	if (((pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_PAPDCAL) ||
	     (pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_PAPDCAL1)) &&
	    PHY_IPA(pi) && !phy_papdcal_is_wfd_phy_ll_enable(pi->papdcali)) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION(pi), 31000);
	}

#if defined(RXDESENS_EN)
	if (pi_nphy->ntd_current_rxdesens != 0) {
		pi_nphy->ntd_save_current_rxdesens = pi_nphy->ntd_current_rxdesens;
		wlc_nphy_set_rxdesens((wlc_phy_t *)pi, 0);
	}
#endif // endif

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	/* For 4322, we need to save the current Tx power indices OR current Tx gain, since we
	   may land up changing them during Tx gain control. For 4321, these are just used as
	   information during cal dumps
	*/
	if ((pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_IDLE) ||
	    (pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_INIT)) {
		pi_nphy->nphy_cal_orig_pwr_idx[0] =
		    wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_0);
		pi_nphy->nphy_cal_orig_pwr_idx[1] =
		    wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_1);

		if (pi->nphy_txpwrctrl != PHY_TPC_HW_OFF) {
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2,
			    0x110, 16, pi_nphy->nphy_cal_orig_tx_gain);
		} else {
			pi_nphy->nphy_cal_orig_tx_gain[0] = 0;
			pi_nphy->nphy_cal_orig_tx_gain[1] = 0;
		}
	}
	wlc_phy_get_tx_gain_nphy(pi, &target_gain);
	tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
	wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

	mphase = (pi->cal_info->cal_phase_id != MPHASE_CAL_STATE_IDLE);
	if (!mphase) {
		/* For 4322, set the Tx power to approx 10dBm before doing Tx/Rx cal */
		wlc_phy_precal_txgain_nphy(pi);
		restore_tx_gain = TRUE;
		target_gain = pi_nphy->nphy_cal_target_gain;

		if (BCME_OK == wlc_phy_cal_txiqlo_nphy(pi, target_gain, fullcal, mphase)) {
			/* send out CTS-to-self before PAPD cal */
			if (PHY_IPA(pi)) {
				phy_utils_phyreg_exit(pi);
				wlapi_enable_mac(pi->sh->physhim);
				wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION(pi), 31000);
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
				phy_utils_phyreg_enter(pi);
				wlc_phy_txpwr_papd_cal_run(pi, TRUE, PHY_CORE_0,
					pi->pubpi->phy_corenum - 1);
			}

			/* Enable and disable MAC, so that we can force the ucode to
			   send a CTS-to-self, before starting the RX IQcal
			*/
			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);
			wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION(pi), 10000);
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);

#ifdef RXIQCAL_FW_WAR
				if ((CHSPEC_IS5G(pi->radio_chanspec) &&
					(BCME_OK == wlc_phy_cal_rxiq_nphy_fw_war(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi->u.pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, 0x3))) ||
				    (CHSPEC_IS2G(pi->radio_chanspec) &&
					(BCME_OK == wlc_phy_cal_rxiq_nphy(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi->u.pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, 0x3))))
#else
			if (BCME_OK == wlc_phy_cal_rxiq_nphy(pi, target_gain,
				(pi->first_cal_after_assoc ||
				(pi->u.pi_nphy->cal_type_override ==
				PHY_PERICAL_FULL)) ? 2 : 0,
				FALSE, 0x3))
#endif /* RXIQCAL_FW_WAR */
			{
				wlc_phy_savecal_nphy(pi);

				/* tx pwr control is on, restore coeffs */
				wlc_phy_txpwrctrl_coeff_setup_nphy(pi);

				/* update the timestamp on success */
				pi->cal_info->last_cal_time = pi->sh->now;
			}
		}

		if (caltype != PHY_PERICAL_AUTO) {
			wlc_phy_rssi_cal_nphy(pi);
		}

		/* Idle TSSI measurement is required while starting/joining
		   a BSS/IBSS after the NPHY calibration
		*/
		if (pi->first_cal_after_assoc ||
		    (pi->u.pi_nphy->cal_type_override == PHY_PERICAL_FULL)) {
			pi->first_cal_after_assoc = FALSE;
			wlc_phy_txpwrctrl_idle_tssi_nphy(pi);
			wlc_phy_txpwrctrl_pwr_setup_nphy(pi);
		}

		/* Do a VCO cal to prevent VCO/PLL from losing lock due to temp delta */
		wlc_phy_radio205x_vcocal_nphy(pi);
#if defined(PHYCAL_CACHING)
		if (ctx) {
			PHY_CAL(("%s: Storing the cals for 0x%x phase %d\n",
				__FUNCTION__, pi->radio_chanspec,
				pi->cal_info->cal_phase_id));
			wlc_phy_cal_cache_nphy((wlc_phy_t *)pi);
		}
#endif // endif
	} else {	/* mphase */
		ASSERT(pi->phy_cal_mode >= PHY_PERICAL_MPHASE);

		PHY_CAL(("wlc_phy_periodic_cal_nphy: %d\n", pi->cal_info->cal_phase_id));
		switch (pi->cal_info->cal_phase_id) {
		case MPHASE_CAL_STATE_INIT:

			if (CHSPEC_IS5G(pi->radio_chanspec) &&
			    (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)) {

				int16 nphy_currtemp = 0;
				uint16 crsminpwr_temp_thresh = 10;
				uint16 crsmin;

				wlapi_suspend_mac_and_wait(pi->sh->physhim);
				nphy_currtemp = wlc_phy_tempsense_nphy(pi);

				crsmin = (nphy_currtemp <= crsminpwr_temp_thresh) ?
				        0x44 : phy_utils_read_phyreg(pi, NPHY_crsminpowerl0) &
				        NPHY_crsminpowerl0_crsminpower0_MASK;

				phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				            NPHY_crsminpoweru0_crsminpower0_MASK,
				            (crsmin << NPHY_crsminpoweru0_crsminpower0_SHIFT));
				phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				            NPHY_crsminpowerl0_crsminpower0_MASK,
				            (crsmin << NPHY_crsminpowerl0_crsminpower0_SHIFT));
				wlapi_enable_mac(pi->sh->physhim);
			}

			pi->cal_info->last_cal_time = pi->sh->now;
			pi->cal_info->u.ncal.txiqlocal_chanspec = pi->radio_chanspec;
			/* For 4322, set the Tx power to approx 10dBm before doing Tx/Rx cal */
			wlc_phy_precal_txgain_nphy(pi);

			pi->cal_info->cal_phase_id++;
			break;

		case MPHASE_CAL_STATE_TXPHASE0:	/* The 1st of 6 Tx calibration phases */
		case MPHASE_CAL_STATE_TXPHASE1:
		case MPHASE_CAL_STATE_TXPHASE2:
		case MPHASE_CAL_STATE_TXPHASE3:
		case MPHASE_CAL_STATE_TXPHASE4:
		case MPHASE_CAL_STATE_TXPHASE5:
			if ((pi->radar_percal_mask & 0x10) != 0)
				pi_nphy->nphy_rxcal_active = TRUE;
			cal_result =
				wlc_phy_cal_txiqlo_nphy(pi, pi_nphy->nphy_cal_target_gain, fullcal,
				TRUE);
			if (cal_result != BCME_OK) {
				/* rare case, just reset */
				PHY_ERROR(("wlc_phy_nphy_tqcal_tx failed\n"));
				phy_calmgr_mphase_reset(pi->calmgri);
				break;
			}

			pi->cal_info->cal_phase_id++;
			break;

		case MPHASE_CAL_STATE_PAPDCAL:
			if ((pi->radar_percal_mask & 0x2) != 0)
				pi_nphy->nphy_rxcal_active = TRUE;
			/* add papd cal as part of multi-phase PHASE calibration */
			if (PHY_IPA(pi)) {
				/*  Make antenna stay at the original place. */
				wlc_phy_ant_force_nphy(pi, entries);
				wlc_phy_txpwr_papd_cal_run(pi, TRUE, PHY_CORE_0,
					pi->pubpi->phy_corenum - 1);
				/*  Restore the AntSwCtrlLUT */
				wlc_phy_ant_release_nphy(pi, entries);
			}
			pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_RXCAL;
			break;

		case MPHASE_CAL_STATE_PAPDCAL1:
			if ((pi->radar_percal_mask & 0x2) != 0)
				pi_nphy->nphy_rxcal_active = TRUE;
			/* add papd cal as part of multi-phase PHASE calibration */
			if (PHY_IPA(pi)) {
				/*  Make antenna stay at the original place. */
				wlc_phy_ant_force_nphy(pi, entries);

				wlc_phy_txpwr_papd_cal_run(pi, TRUE, PHY_CORE_1, PHY_CORE_1);

				/*  Restore the AntSwCtrlLUT */
				wlc_phy_ant_release_nphy(pi, entries);
			}

			pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_RXCAL;
			break;

		case MPHASE_CAL_STATE_RXCAL:
			if ((pi->radar_percal_mask & 0x1) != 0)
				pi_nphy->nphy_rxcal_active = TRUE;
			{
				uint8 core_mask = 0x3;

#ifdef RXIQCAL_FW_WAR
				if ((CHSPEC_IS5G(pi->radio_chanspec) &&
					(wlc_phy_cal_rxiq_nphy_fw_war(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, core_mask) == BCME_OK)) ||
				    (CHSPEC_IS2G(pi->radio_chanspec) &&
					(wlc_phy_cal_rxiq_nphy(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, core_mask) == BCME_OK)))
#else
				if (wlc_phy_cal_rxiq_nphy(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, core_mask) == BCME_OK)
#endif /* RXIQCAL_FW_WAR */
				{
					wlc_phy_savecal_nphy(pi);
				}
				/* this step finished, move the phase id */
				pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_RSSICAL;
			}
			break;

		case MPHASE_CAL_STATE_RXCAL1:
			if ((pi->radar_percal_mask & 0x1) != 0)
				pi_nphy->nphy_rxcal_active = TRUE;
#ifdef RXIQCAL_FW_WAR
				if ((CHSPEC_IS5G(pi->radio_chanspec) &&
					(wlc_phy_cal_rxiq_nphy_fw_war(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, 0x2) == BCME_OK)) ||
				    (CHSPEC_IS2G(pi->radio_chanspec) &&
					(wlc_phy_cal_rxiq_nphy(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, 0x2) == BCME_OK)))
#else
				if (wlc_phy_cal_rxiq_nphy(pi, target_gain,
					(pi->first_cal_after_assoc ||
					(pi_nphy->cal_type_override ==
					PHY_PERICAL_FULL)) ? 2 : 0,
					FALSE, 0x2) == BCME_OK)
#endif /* RXIQCAL_FW_WAR */
			{
				wlc_phy_savecal_nphy(pi);
			}

			/* this step finished, move the phase id */
			pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_RSSICAL;
			break;

		case MPHASE_CAL_STATE_RSSICAL:
			if ((pi->radar_percal_mask & 0x4) != 0)
				pi_nphy->nphy_rxcal_active = TRUE;
			wlc_phy_txpwrctrl_coeff_setup_nphy(pi);
			wlc_phy_rssi_cal_nphy(pi);

			/* Do a VCO cal to prevent VCO/PLL from losing lock due to temp delta */
			wlc_phy_radio205x_vcocal_nphy(pi);
			restore_tx_gain = TRUE;
			/* If this is the first calibration after association then we still have one
			 * more phase to go
			 */
			if (pi->first_cal_after_assoc) {
				pi->cal_info->cal_phase_id++;
			} else {

			phy_calmgr_mphase_reset(pi->calmgri);
#if defined(PHYCAL_CACHING)
				/* store the calibrations since done */
				if (ctx) {
					PHY_CAL(("%s: Storing the cals for 0x%x phase %d\n",
						__FUNCTION__, pi->radio_chanspec,
						pi->cal_info->cal_phase_id));
					wlc_phy_cal_cache_nphy((wlc_phy_t *)pi);
				}
#endif // endif
			}

			break;

		case MPHASE_CAL_STATE_IDLETSSI:
			if ((pi->radar_percal_mask & 0x8) != 0)
				pi_nphy->nphy_rxcal_active = TRUE;
			/* Idle TSSI measurement is required while starting/joining
			 * a BSS/IBSS after the NPHY calibration
			 */
			if (pi->first_cal_after_assoc) {
				pi->first_cal_after_assoc = FALSE;
				wlc_phy_txpwrctrl_idle_tssi_nphy(pi);
				wlc_phy_txpwrctrl_pwr_setup_nphy(pi);
			}
			phy_calmgr_mphase_reset(pi->calmgri);
#if defined(PHYCAL_CACHING)
			/* store the calibrations since done */
			if (ctx) {
				PHY_CAL(("%s: Storing the cals for 0x%x phase %d\n",
					__FUNCTION__, pi->radio_chanspec,
					pi->cal_info->cal_phase_id));
				wlc_phy_cal_cache_nphy((wlc_phy_t *)pi);
			}
#endif // endif
			break;

		case MPHASE_CAL_STATE_NOISECAL :

			phy_calmgr_mphase_reset(pi->calmgri);
#if defined(PHYCAL_CACHING)
			/* store the calibrations since done */
			if (ctx) {
				PHY_CAL(("%s: Storing the cals for 0x%x phase %d\n",
					__FUNCTION__, pi->radio_chanspec,
					pi->cal_info->cal_phase_id));
				wlc_phy_cal_cache_nphy((wlc_phy_t *)pi);
			}
#endif // endif
			break;
		default:
			PHY_ERROR(("wlc_phy_periodic_cal_nphy: Invalid calibration phase %d\n",
			    pi->cal_info->cal_phase_id));
			ASSERT(0);
			phy_calmgr_mphase_reset(pi->calmgri);
			break;
		}
	}

	/* For 4322, restore the Tx gains since they may have changed during Tx gain control */
	if (restore_tx_gain) {
		if (tx_pwr_ctrl_state != PHY_TPC_HW_OFF) {
			/* Restore the Tx gain, to the power index values, that we read
			   before h/w power control was disabled
			*/
			wlc_phy_txpwr_index_nphy(pi, 1, pi_nphy->nphy_cal_orig_pwr_idx[0], FALSE);
			wlc_phy_txpwr_index_nphy(pi, 2, pi_nphy->nphy_cal_orig_pwr_idx[1], FALSE);

			/* Reset these to default values */
			pi_nphy->nphy_txpwrindex[0].index = -1;
			pi_nphy->nphy_txpwrindex[1].index = -1;
		} else {
			wlc_phy_txpwr_index_nphy(pi, (1 << 0),
				(int8) (pi_nphy->nphy_txpwrindex[0].index_internal), FALSE);
			wlc_phy_txpwr_index_nphy(pi, (1 << 1),
				(int8) (pi_nphy->nphy_txpwrindex[1].index_internal), FALSE);
		}
	}

	wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

#if defined(RXDESENS_EN)
	if ((pi_nphy->ntd_save_current_rxdesens != 0) && (pi_nphy->ntd_current_rxdesens == 0)) {
		wlc_nphy_set_rxdesens((wlc_phy_t *)pi, pi_nphy->ntd_save_current_rxdesens);
	}
#endif // endif
}

int
wlc_phy_cal_txiqlo_nphy(phy_info_t *pi, nphy_txgains_t target_gain, bool fullcal, bool mphase)
{
	uint16 val;
	uint16 tbl_buf[11];
	uint8 cal_cnt;
	uint16 cal_cmd;
	uint8 num_cals, max_cal_cmds;
	uint16 core_no, cal_type;
	uint16 diq_start = 0;
	uint8 phy_bw;
	uint16 max_val;
	uint16 tone_freq;
	uint16 gain_save[4];
	uint16 cal_gain[NPHY_CORE_NUM];
	nphy_iqcal_params_t cal_params[NPHY_CORE_NUM];
	uint32 tbl_len;
	void *tbl_ptr;
	bool ladder_updated[NPHY_CORE_NUM];
	uint8 mphase_cal_lastphase = 0;
	int bcmerror = BCME_OK;
	bool   phyhang_avoid_state = FALSE;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	uint16 tbl_tx_iqlo_cal_loft_ladder_20[] = {
		0x0300, 0x0500, 0x0700, 0x0900, 0x0d00, 0x1100, 0x1900, 0x1901, 0x1902,
		0x1903, 0x1904, 0x1905, 0x1906, 0x1907, 0x2407, 0x3207, 0x4607, 0x6407
	}; /* LOFT Ladder [m / g_env] */

	uint16 tbl_tx_iqlo_cal_iqimb_ladder_20[] = {
		0x0200, 0x0300, 0x0600, 0x0900, 0x0d00, 0x1100, 0x1900, 0x2400, 0x3200,
		0x4600, 0x6400, 0x6401, 0x6402, 0x6403, 0x6404, 0x6405, 0x6406, 0x6407
	}; /* IQImb Ladder [m / g_en] */

	uint16 tbl_tx_iqlo_cal_loft_ladder_40[] = {
		0x0200, 0x0300, 0x0400, 0x0700, 0x0900, 0x0c00, 0x1200, 0x1201, 0x1202,
		0x1203, 0x1204, 0x1205, 0x1206, 0x1207, 0x1907, 0x2307, 0x3207, 0x4707
	}; /* LOFT Ladder [m / g_env] */

	uint16 tbl_tx_iqlo_cal_iqimb_ladder_40[] = {
		0x0100, 0x0200, 0x0400, 0x0700, 0x0900, 0x0c00, 0x1200, 0x1900, 0x2300,
		0x3200, 0x4700, 0x4701, 0x4702, 0x4703, 0x4704, 0x4705, 0x4706, 0x4707
	}; /* IQImb Ladder [m / g_en] */

	/* IQ/LO cal Rev3+ specific tables */
	uint16 tbl_tx_iqlo_cal_startcoefs_nphyrev3[] = {
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000
	}; /* Start Coefficients */

	uint16 tbl_tx_iqlo_cal_cmds_fullcal_nphyrev3[] = {
		0x8434, 0x8334, 0x8084, 0x8267, 0x8056, 0x8234,
		0x9434, 0x9334, 0x9084, 0x9267, 0x9056, 0x9234
	}; /* Table of commands for calibrations: fullcal */

	uint16 tbl_tx_iqlo_cal_cmds_recal_nphyrev3[] = {
		0x8423, 0x8323, 0x8073, 0x8256, 0x8045, 0x8223,
		0x9423, 0x9323, 0x9073, 0x9256, 0x9045, 0x9223
	}; /* Table of commands for calibrations: recal */
	nphy_cal_result_t *ncal = &pi->cal_info->u.ncal;

	/* The pi_nphy->nphy_use_int_tx_iqlo_cal flag determines whether
	 * the Tx IQ/LOFT cal in external
	 * PA boards would be performed boards using the radio's
	 * internal envelope/peak detectors
	 * or using the external FEM's peak detector. This flag is set in wlc_phy_init_nphy.
	 */

	uint8 phyrxchain;

	BCM_REFERENCE(phyrxchain);

	/* need enable two RX cores before do PHY TXIQ Calibration */
	/* will turn off RX core after calibration */
	phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;

	if (phyrxchain == 0x1) {
		wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, 3, 0);
	}

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Disable phyhang_avoid */
	phyhang_avoid_state = pi_nphy->phyhang_avoid;
	pi_nphy->phyhang_avoid   = FALSE;

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		phy_bw = 40;
	} else {
		phy_bw = 20;
	}

	/* save tx gain setting & set new gain settings */
	/* TCL: set SAVE_gain [mimophy_read_table RFSeq 2 0x110] */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, gain_save);

	FOREACH_CORE(pi, core_no) {
		PHY_CAL(("wlc_phy_cal_txiqlo_nphy: tx core %d target gain: pad = %d,"
		         " pga = %d, txgm = %d, txlpf = %d\n", core_no,
		         target_gain.pad[core_no], target_gain.pga[core_no],
		         target_gain.txgm[core_no], target_gain.txlpf[core_no]));
	}

	/* TCL: set gain_config0 [mimophy_iqcal_get_gainconfig 0] */
	/* TCL: set gain_config1 [mimophy_iqcal_get_gainconfig 1] */
	FOREACH_CORE(pi, core_no) {
		wlc_phy_iqcal_gainparams_nphy(pi, core_no, target_gain, &cal_params[core_no]);
		cal_gain[core_no] = cal_params[core_no].cal_gain;
		PHY_CAL(("%s: tx core %d cal params: pad %d, pga %d, txgm %d."
			"cal_gain 0x%x ncorr[0..3] = [0x%x 0x%x 0x%x 0x%x]\n",
			__FUNCTION__, core_no, cal_params[core_no].pad, cal_params[core_no].pga,
			cal_params[core_no].txgm, cal_gain[core_no],
			cal_params[core_no].ncorr[0], cal_params[core_no].ncorr[1],
			cal_params[core_no].ncorr[2], cal_params[core_no].ncorr[3]));
	}

	/* Set and force tx cal_gain to take effect, TCL: */
	/* TCL: mimophy_tx_gain [lindex $gain_config0 0] [lindex $gain_config0 1] \
	 *        [lindex $gain_config0 2] [lindex $gain_config1 0] [lindex $gain_config1 1] \
	 *        [lindex $gain_config1 2]  1
	 */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, cal_gain);

	/* Note: wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RX2TX); is not needed here,
	 * because the rx2tx switch occurs implicitly in the subsequent call to play tone
	 */

	/* Set the radio in TX iqlo cal state */
	wlc_phy_txcal_radio_setup_nphy(pi);

	/* Setup the PHY for TX iqlo cal */
	wlc_phy_txcal_physetup_nphy(pi);
	ladder_updated[0] = ladder_updated[1] = FALSE;

	/* specify gain ladders, details see TCL code */
	if (phy_bw == 40) {
		tbl_ptr = tbl_tx_iqlo_cal_loft_ladder_40; /* ptr to val */
		tbl_len = ARRAYSIZE(tbl_tx_iqlo_cal_loft_ladder_40); /* # values   */
	} else {
		tbl_ptr = tbl_tx_iqlo_cal_loft_ladder_20; /* ptr to val */
		tbl_len = ARRAYSIZE(tbl_tx_iqlo_cal_loft_ladder_20); /* # values   */
	}
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, tbl_len, 0, 16, tbl_ptr);

	if (phy_bw == 40) {
		tbl_ptr = tbl_tx_iqlo_cal_iqimb_ladder_40; /* ptr to val */
		tbl_len = ARRAYSIZE(tbl_tx_iqlo_cal_iqimb_ladder_40); /* # values   */
	} else {
		tbl_ptr = tbl_tx_iqlo_cal_iqimb_ladder_20; /* ptr to val */
		tbl_len = ARRAYSIZE(tbl_tx_iqlo_cal_iqimb_ladder_20); /* # values   */
	}
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, tbl_len, 32, 16, tbl_ptr);

	/* Set Gain Control Parameters including IQ/LO cal enable bit */
	/*   iqlocal_en<15> / start_index / thresh_d2 / ladder_length_d2
	 * For REV7+, increase the Aux ADC clip thresh_d2 from 10 to 13 for the IQ cal engine to
	 * make use of more dynamic range to obtain better Tx IQ/LOFT cal performance. For REVs 1
	 * through 6, the old setting of 10 is retained since they have already been extensively
	 * tested. The threshold is not increased to the maximum value of 15 to provide some
	 * headroom to guard against the event that the Aux ADC clips for any of the 9 points in
	 * the IQ/LOFT cal grid search.
	 */
	phy_utils_write_phyreg(pi, NPHY_iqloCalCmdGctl, 0x8ad9);

	/* save bb_mult_m value */
	/* TCL: set SAVE_bb_mult_m [mimophy_read_table iqloCaltbl 1 87] */
	/* MOVED TO PLAY TONE */

	/* turn on test tone */
	max_val = 250;
	tone_freq = (phy_bw == 20) ? 2500 : 5000;

	/* wlc_phy_stopplayback_nphy(pi); */     /* mimophy_stop_playback */

	if (pi->cal_info->cal_phase_id > MPHASE_CAL_STATE_TXPHASE0) {
/* Temp fix for dongle builds - but we should not have the number of samlpes
 * hardcoded; it is better to store off the number of samples which have been setup
 * in the pi->cal_info structure and use it to call wlc_phy_runsamples_nphy here
 */
#ifdef DONGLEBUILD
		wlc_phy_runsamples_nphy(pi, phy_bw * 4, 0xffff, 0, 1, 0, FALSE);
#else
		wlc_phy_runsamples_nphy(pi, phy_bw * 8, 0xffff, 0, 1, 0, FALSE);
#endif // endif
		bcmerror = BCME_OK;
	} else {
		bcmerror = wlc_phy_tx_tone_nphy(pi, tone_freq, max_val, 1, 0, FALSE);
	}

	/* !!! if play tone failed, no need to continue */
	if (bcmerror == BCME_OK) {

		/* start coefficients:
		 * a0 b0 a1 b1 ci0_cq0_ci1_cq1 di0_dq0 di1_dq1 ei0_eq0 ei1_eq1 fi0_fq0 fi1_fq1
		 */
		if (pi->cal_info->cal_phase_id > MPHASE_CAL_STATE_TXPHASE0) {
			if (!ncal->txiqlocal_coeffsvalid)
				fullcal = TRUE;
			tbl_ptr = ncal->txcal_interm_coeffs;
			tbl_len = ARRAYSIZE(pi->cal_info->u.ncal.txcal_interm_coeffs);
		} else {
			if ((!fullcal) && (ncal->txiqlocal_coeffsvalid)) {
				/* run a re-calibration only */
				PHY_CAL(("wlc_phy_cal_txiqlo_nphy: recal\n"));
				tbl_ptr = ncal->txiqlocal_coeffs;
				tbl_len = ARRAYSIZE(ncal->txiqlocal_coeffs);
			} else {
				/* run a full calibration */
				PHY_CAL(("wlc_phy_cal_txiqlo_nphy: full cal (forced=%s)\n",
				((!fullcal) ? "yes" : "no")));
				fullcal = TRUE;

				tbl_ptr = tbl_tx_iqlo_cal_startcoefs_nphyrev3;
				tbl_len = ARRAYSIZE(tbl_tx_iqlo_cal_startcoefs_nphyrev3);
			}
		}
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, tbl_len, 64, 16, tbl_ptr);

		/* ------------------
		 * Calibration steps
		 * ------------------
		 */

		if (fullcal) {
			max_cal_cmds = ARRAYSIZE(tbl_tx_iqlo_cal_cmds_fullcal_nphyrev3);
		} else {
			max_cal_cmds = ARRAYSIZE(tbl_tx_iqlo_cal_cmds_recal_nphyrev3);
		}

		if (mphase) {
			cal_cnt = pi->cal_info->txcal_cmdidx;
			if ((cal_cnt + pi->cal_info->txcal_numcmds) < max_cal_cmds) {
				num_cals = cal_cnt + pi->cal_info->txcal_numcmds;
			} else {
				num_cals = max_cal_cmds;
			}
		} else {
			cal_cnt = 0;
			num_cals = max_cal_cmds;
		}

		for (; cal_cnt < num_cals; cal_cnt++) {
			/* read command from table */
			if (fullcal) {
				cal_cmd = tbl_tx_iqlo_cal_cmds_fullcal_nphyrev3[cal_cnt];
			} else {
				cal_cmd = tbl_tx_iqlo_cal_cmds_recal_nphyrev3[cal_cnt];
			}
			/*  get cal type and core_to_be_calibrated */
			core_no = ((cal_cmd & 0x3000) >> 12);
			cal_type = ((cal_cmd & 0x0F00) >> 8);

			if (!(pi_nphy->nphy_use_int_tx_iqlo_cal)) {
				/* Enable the external PA on the core that we are calibrating,
			     * if we are using external TSSI for TX IQCal.
			     */
				wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_PA, 1,
				((core_no == 0)?
				RADIO_MIMO_CORESEL_CORE1 : RADIO_MIMO_CORESEL_CORE2));
				OSL_DELAY(20);
			}

			/* if analog LO cal, store suggested (systematic) dig LOFT comp vals */
			if ((cal_type == 1) || (cal_type == 3) || (cal_type == 4)) {
				/* store suggested (systematic) dig LOFT comp vals */
				wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 69 + core_no,
				16, tbl_buf);

				diq_start = tbl_buf[0];
				/* reset digital DC inj to zero before analog LO cal */
				tbl_buf[0] = 0;
				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 69 + core_no,
				16, tbl_buf);
			}

			if (!ladder_updated[core_no]) {
				wlc_phy_update_txcal_ladder_nphy(pi, core_no);
				ladder_updated[core_no] = TRUE;
			}
			/* SW IQ Calibration WAR */

			val = (cal_params[core_no].ncorr[cal_type] << 8) | NPHY_N_GCTL;

			phy_utils_write_phyreg(pi, NPHY_iqloCalCmdNnum, val);

			/* common part for all cals */
			phy_utils_write_phyreg(pi, NPHY_iqloCalCmd, cal_cmd);
			/* mimophy_iqcal_wait, MAX wait time is ~20ms */
			SPINWAIT(((phy_utils_read_phyreg(pi, NPHY_iqloCalCmd) &
			           0xc000) != 0),
			NPHY_SPINWAIT_CAL_TXIQLO);
			ASSERT((phy_utils_read_phyreg(pi, NPHY_iqloCalCmd) & 0xc000) == 0);

			if (!(pi_nphy->nphy_use_int_tx_iqlo_cal)) {
				wlc_phy_rfctrlintc_override_nphy(pi,
					NPHY_RfctrlIntc_override_PA, 0,
					RADIO_MIMO_CORESEL_CORE1 | RADIO_MIMO_CORESEL_CORE2);
			}

			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, tbl_len, 96, 16, tbl_buf);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, tbl_len, 64, 16, tbl_buf);

			/* if analog LO cal, restore initial diq0 values */
			if ((cal_type == 1) || (cal_type == 3) || (cal_type == 4)) {
				/* restore initial diq0 values */
				/* TCL: mimophy_write_table iqloCaltbl  $diq_start  69+core_no ; */
				tbl_buf[0] = diq_start;
			}

		}

		/* nphase calibration, update the Tx cal cmd index, pointing to
		 * the next set of multiphase commands for the next time around.
		 * If we are done issuing all the Tx cal cmds then reset the Tx cal cmd index
		 */
		if (mphase) {
			pi->cal_info->txcal_cmdidx = num_cals;
			if (pi->cal_info->txcal_cmdidx >= max_cal_cmds)
				pi->cal_info->txcal_cmdidx = 0;
		}

		mphase_cal_lastphase = MPHASE_CAL_STATE_TXPHASE5;

		if (!mphase || (pi->cal_info->cal_phase_id == mphase_cal_lastphase)) {
			/* single phase or last tx stage in multiphase cal, save the results */
			/* currently defunct Mixer iDAC cal - need to include in the loop adequately
			 * for REV2 (B0), see TCL code
			 * Important Note: The assumption is that the last cal *in each core* is NOT
			 *                 an analog LO cal (otherwise have to manually write coeffs
			 *                 from Best to radio regs, see TCL.
			 * Apply IQ Cal Results to OFDM PHY
			 *  TCL: mimophy_write_table iqloCaltbl
			 *  [mimophy_read_table iqloCaltbl 4 96] 80
			 * ---------------------
			 * End Calibration steps
			 * ---------------------
			 */

			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 4, 96, 16, tbl_buf);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 4, 80, 16, tbl_buf);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 4, 88, 16, tbl_buf);

			/* Apply Digital LOFT Comp to OFDM PHY (di0,dq0,di1,dq1) */
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 101, 16, tbl_buf);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 85, 16, tbl_buf);

			/* TCL: mimophy_write_table iqloCaltbl
			   [mimophy_read_table iqloCaltbl 2 101] 93
			 */
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 93, 16, tbl_buf);

			/* Also store results in pi->txiqlocal_coeffs and flag valid */
			/* TCL: set def(tx_iqlo_cal_interm_coeffs)
			   [mimophy_read_table iqloCaltbl 9 96];
			 */
			tbl_len = ARRAYSIZE(ncal->txiqlocal_coeffs);
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL,
			tbl_len, 96, 16, ncal->txiqlocal_coeffs);

			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 101, 16, tbl_buf);
			pi_nphy->lowpwrDiq[0] = tbl_buf[0];
			pi_nphy->lowpwrDiq[1] = tbl_buf[1];

			/* set valid flag */
			ncal->txiqlocal_coeffsvalid = TRUE;
			ncal->txiqlocal_chanspec = pi->radio_chanspec;
		} else {
			tbl_len = ARRAYSIZE(pi->cal_info->u.ncal.txcal_interm_coeffs);
			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL,
			tbl_len, 96, 16, ncal->txcal_interm_coeffs);
		}

		PHY_CAL(("wlc_phy_cal_txiqlo_nphy: results\n"));
		PHY_CAL(("\tA0=%4d B0=%4d A1=%4d B1=%4d Ci0=%4d Cq0=%4d Ci1=%4d Cq1=%4d\n",
		(int16)ncal->txiqlocal_coeffs[0], (int16)ncal->txiqlocal_coeffs[1],
		(int16)ncal->txiqlocal_coeffs[2], (int16)ncal->txiqlocal_coeffs[3],
		(int8)((ncal->txiqlocal_coeffs[4] & (0xF<<12))>>8)>>4,
		(int8)((ncal->txiqlocal_coeffs[4] & (0xF<<8))>>4)>>4,
		(int8)((ncal->txiqlocal_coeffs[4] & (0xF<<4))>>0)>>4,
		(int8)((ncal->txiqlocal_coeffs[4] & (0xF<<0))<<4)>>4));
		PHY_CAL(("\tDi0=%4d Dq0=%4d Di1=%4d Dq1=%4d Ei0=%4d Eq0=%4d Ei1=%4d Eq1=%4d\n",
		(int8)((ncal->txiqlocal_coeffs[5] & (0xFF<<8))>>8),
		(int8)((ncal->txiqlocal_coeffs[5] & (0xFF<<0))>>0),
		(int8)((ncal->txiqlocal_coeffs[6] & (0xFF<<8))>>8),
		(int8)((ncal->txiqlocal_coeffs[6] & (0xFF<<0))>>0),
		(int8)((ncal->txiqlocal_coeffs[7] & (0xFF<<8))>>8),
		(int8)((ncal->txiqlocal_coeffs[7] & (0xFF<<0))>>0),
		(int8)((ncal->txiqlocal_coeffs[8] & (0xFF<<8))>>8),
		(int8)((ncal->txiqlocal_coeffs[8] & (0xFF<<0))>>0)));

		/* Apply Analog LOFT Comp (OFDM-PHY and B-PHY)
		 * -- assumed to be unnecessary for now (see Important Note in TCL)
		 */

		/* Switch off test tone */
		wlc_phy_stopplayback_nphy(pi);	/* mimophy_stop_playback */

		/* disable IQ/LO cal */
		phy_utils_write_phyreg(pi, NPHY_iqloCalCmdGctl, 0x0000);

	} else {
		phy_utils_write_phyreg(pi, NPHY_iqloCalCmdGctl, 0x0000);
	}

	/* Restore bb_mult_m */
	/* TCL: mimophy_write_table iqloCaltbl $SAVE_bb_mult_m 87 */
	/* MOVED TO TX_TONE */

	/* Clean Up PHY */
	wlc_phy_txcal_phycleanup_nphy(pi);

	/* restore tx gain */
	/* TCL: mimophy_write_table RFSeq    $SAVE_gain 0x110 */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, gain_save);

	/* Note: no RFSeq to enforce change, next tx will do it */

	/* Clean up RADIO */
	wlc_phy_txcal_radio_cleanup_nphy(pi);

	/* Enable phyhang_avoid for NPHY_REV4+ */
	pi_nphy->phyhang_avoid = phyhang_avoid_state;

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	if (phyrxchain == 0x1) {
		wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, 1, 0);
	}

	return bcmerror;
}

void
wlc_phy_rx_iq_coeffs_nphy(phy_info_t *pi, uint8 write, nphy_iq_comp_t *pcomp)
{
	if (write) {
		phy_utils_write_phyreg(pi, NPHY_Core1RxIQCompA0, pcomp->a0);
		phy_utils_write_phyreg(pi, NPHY_Core1RxIQCompB0, pcomp->b0);
		phy_utils_write_phyreg(pi, NPHY_Core2RxIQCompA1, pcomp->a1);
		phy_utils_write_phyreg(pi, NPHY_Core2RxIQCompB1, pcomp->b1);
	} else {
		pcomp->a0 = phy_utils_read_phyreg(pi, NPHY_Core1RxIQCompA0);
		pcomp->b0 = phy_utils_read_phyreg(pi, NPHY_Core1RxIQCompB0);
		pcomp->a1 = phy_utils_read_phyreg(pi, NPHY_Core2RxIQCompA1);
		pcomp->b1 = phy_utils_read_phyreg(pi, NPHY_Core2RxIQCompB1);
	}
}

void
wlc_phy_rx_iq_est_nphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs)
{
	uint8 core;

	/* Get Rx IQ Imbalance Estimate from modem */
	phy_utils_write_phyreg(pi, NPHY_IqestSampleCount, num_samps);
	phy_utils_mod_phyreg(pi, NPHY_IqestWaitTime, NPHY_IqestWaitTime_waitTime_MASK,
	            (wait_time << NPHY_IqestWaitTime_waitTime_SHIFT));
	phy_utils_mod_phyreg(pi, NPHY_IqestCmd, NPHY_IqestCmd_iqMode,
	            (wait_for_crs) ? NPHY_IqestCmd_iqMode : 0);

	phy_utils_mod_phyreg(pi, NPHY_IqestCmd, NPHY_IqestCmd_iqstart,
	            NPHY_IqestCmd_iqstart);

	/* wait for estimate */
	SPINWAIT(((phy_utils_read_phyreg(pi, NPHY_IqestCmd) & NPHY_IqestCmd_iqstart) != 0),
		NPHY_SPINWAIT_RX_IQ_EST);
	ASSERT((phy_utils_read_phyreg(pi, NPHY_IqestCmd) & NPHY_IqestCmd_iqstart) == 0);

	if ((phy_utils_read_phyreg(pi, NPHY_IqestCmd) & NPHY_IqestCmd_iqstart) == 0) {
		ASSERT(pi->pubpi->phy_corenum <= NPHY_CORE_NUM);
		FOREACH_CORE(pi, core) {
			est[core].i_pwr =
			        (phy_utils_read_phyreg(pi, NPHY_IqestipwrAccHi(core)) << 16) |
				phy_utils_read_phyreg(pi, NPHY_IqestipwrAccLo(core));
			est[core].q_pwr =
			        (phy_utils_read_phyreg(pi, NPHY_IqestqpwrAccHi(core)) << 16) |
				phy_utils_read_phyreg(pi, NPHY_IqestqpwrAccLo(core));
			est[core].iq_prod =
			        (phy_utils_read_phyreg(pi, NPHY_IqestIqAccHi(core)) << 16) |
				phy_utils_read_phyreg(pi, NPHY_IqestIqAccLo(core));
			PHY_CAL(("wlc_phy_rx_iq_est_nphy: core%i "
				 "i_pwr = %u, q_pwr = %u, iq_prod = %d\n",
				 core, est[core].i_pwr, est[core].q_pwr, est[core].iq_prod));
		}
	} else
		PHY_ERROR(("wlc_phy_rx_iq_est_nphy: IQ measurement timed out\n"));
}

#define CAL_RETRY_CNT 2

static void
wlc_phy_calc_rx_iq_comp_nphy(phy_info_t *pi, uint8 core_mask)
{
	uint8 curr_core;
	phy_iq_est_t est[NPHY_CORE_NUM];
	nphy_iq_comp_t old_comp, new_comp;
	int32  iq = 0;
	uint32 ii = 0, qq = 0;
	int16  iq_nbits, qq_nbits, brsh, arsh;
	int32  a, b, temp;
	int bcmerror = BCME_OK;
	uint cal_retry = 0;

	if (core_mask == 0x0) return;

	/* compute Rx compensation coeffs
	 *   -- assumes no compensation to start
	 */

	/* until have code to refine compensation params
	 *   for each re-cal, zero out comp coeffs and
	 *   do "one-shot" calibration.
	 */
	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &old_comp);
	new_comp.a0 = new_comp.b0 = new_comp.a1 = new_comp.b1 = 0x0;
	wlc_phy_rx_iq_coeffs_nphy(pi, 1, &new_comp);

cal_try:
	wlc_phy_rx_iq_est_nphy(pi, est, 0x4000, 32, 0);

	/* init new comp coeffs to old ones, in case we're preserving some
	 * cores' values
	 */
	new_comp = old_comp;

	FOREACH_ACTV_CORE(pi, core_mask, curr_core) {
		/* for each core, implement the following floating point
		 * operations quantized to 10 fractional bits:
		 *   b = -iq/ii
		 *   a = -1 + sqrt(qq/ii - b*b)
		 */
		iq = est[curr_core].iq_prod;
		ii = est[curr_core].i_pwr;
		qq = est[curr_core].q_pwr;
		if ((CHIPID(pi->sh->chip) == BCM43131_CHIP_ID) ||
		    (CHIPID(pi->sh->chip) == BCM43217_CHIP_ID)) {
			ii = est[curr_core].q_pwr;
			qq = est[curr_core].i_pwr;
		}
		PHY_CAL(("%s: Initial Imbalance (core %d): iq = %d, ii = %d, qq = %d\n",
			__FUNCTION__, curr_core, iq, ii, qq));

		/* bounds check estimate info */
		if ((ii + qq) < NPHY_MIN_RXIQ_PWR) {
			PHY_ERROR(("RXIQ imbalance estimate power too small, skipping cal!\n"));
			bcmerror = BCME_ERROR;
			break;
		}

		iq_nbits = math_nbits_32(iq);
		qq_nbits = math_nbits_32(qq);

		arsh = 10-(30-iq_nbits);
		if (arsh >= 0) {
			a = (-(iq << (30 - iq_nbits)) + (ii >> (1 + arsh)));
			temp = (int32) (ii >>  arsh);
			if (temp == 0) {
				PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
				bcmerror = BCME_ERROR;
				break;
			}
		} else {
			a = (-(iq << (30 - iq_nbits)) + (ii << (-1 - arsh)));
			temp = (int32) (ii << -arsh);
			if (temp == 0) {
				PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
				bcmerror = BCME_ERROR;
				break;
			}
		}

		a /= temp;

		brsh = qq_nbits-31+20;
		if (brsh >= 0) {
			b = (qq << (31-qq_nbits));
			temp = (int32) (ii >>  brsh);
			if (temp == 0) {
				PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
				bcmerror = BCME_ERROR;
				break;
			}
		} else {
			b = (qq << (31-qq_nbits));
			temp = (int32) (ii << -brsh);
			if (temp == 0) {
				PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
				bcmerror = BCME_ERROR;
				break;
			}
		}
		b /= temp;
		b -= a*a;
		b = (int32) math_sqrt_int_32((uint32) b);
		b -= (1 << 10);

		if ((curr_core == PHY_CORE_0) && (core_mask & 0x1)) {
			new_comp.a0 = (int16) a & 0x3ff;
			new_comp.b0 = (int16) b & 0x3ff;
		}
		if ((curr_core == PHY_CORE_1) && (core_mask & 0x2)) {
			new_comp.a1 = (int16) a & 0x3ff;
			new_comp.b1 = (int16) b & 0x3ff;
		}
	}

	/* Restore previous calibration values if we are aborting the Rx cal */
	if (bcmerror != BCME_OK) {
		FOREACH_ACTV_CORE(pi, core_mask, curr_core) {
#ifdef MACOSX
			OSL_LOG("phycal",
				"Aborting Rx IQCAL attempt "
				"%d: core %d, iq_prod=%d, i_pwr=%d, q_pwr=%d\n",
				cal_retry, curr_core,
				est[curr_core].iq_prod, est[curr_core].i_pwr, est[curr_core].q_pwr);
#else
			PHY_ERROR(("Aborting Rx IQCAL: core %d, iq_prod=%d, i_pwr=%d, q_pwr=%d\n",
			          curr_core, est[curr_core].iq_prod,
			          est[curr_core].i_pwr, est[curr_core].q_pwr));
			ASSERT(0);
#endif // endif
		}

		if (cal_retry < CAL_RETRY_CNT) {
			cal_retry++;
			goto cal_try;
		}

		new_comp = old_comp;
	} else if (cal_retry > 0) {
		FOREACH_ACTV_CORE(pi, core_mask, curr_core) {
#ifdef MACOSX
			OSL_LOG("phycal",
			        "Cal %d success: core %d iq_prod=%d, i_pwr=%d, q_pwr=%d\n",
			        cal_retry, curr_core,
			        est[curr_core].iq_prod, est[curr_core].i_pwr, est[curr_core].q_pwr);
#endif // endif
		}
	}

	PHY_CAL(("%s: b0 = 0x%03x, a0 = 0x%03x, b1 = 0x%03x, a1 = 0x%03x\n",
		__FUNCTION__, new_comp.b0, new_comp.a0, new_comp.b1, new_comp.a1));

	/* write computed correction */
	wlc_phy_rx_iq_coeffs_nphy(pi, 1, &new_comp);

#if ENABLE_RXIQCAL_DBG
	/* show residual imbalance for diagnostic purposes */
	wlc_phy_rx_iq_est_nphy(pi, est, 0x4000, 32, 0);
	FOREACH_ACTV_CORE(pi, core_mask, curr_core) {
		PHY_INFORM(("Residual Imbalance: core %d, iq = %d, ii = %d, qq = %d\n",
		           curr_core,
		           est[curr_core].iq_prod, est[curr_core].i_pwr, est[curr_core].q_pwr));
	}
#endif  /* ENABLE_RXIQCAL_DBG */
}

static void
wlc_phy_rxcal_radio_setup_nphy(phy_info_t *pi, uint8 rx_core)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (rx_core == PHY_CORE_0) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			pi_nphy->tx_rx_cal_radio_saveregs[0] =
				phy_utils_read_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_5G_PWRUP);
			pi_nphy->tx_rx_cal_radio_saveregs[1] =
				phy_utils_read_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_5G_ATTEN);

			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_5G_PWRUP, 0x3);
			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_5G_ATTEN, 0xaf);

		} else {
			pi_nphy->tx_rx_cal_radio_saveregs[0] =
			        phy_utils_read_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_2G_PWRUP);
			pi_nphy->tx_rx_cal_radio_saveregs[1] =
			        phy_utils_read_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_2G_ATTEN);

			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_2G_PWRUP, 0x3);
			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_2G_ATTEN, 0x7f);
		}

	} else {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			pi_nphy->tx_rx_cal_radio_saveregs[0] =
			        phy_utils_read_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_5G_PWRUP);
			pi_nphy->tx_rx_cal_radio_saveregs[1] =
			        phy_utils_read_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_5G_ATTEN);

			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_5G_PWRUP, 0x3);
			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_5G_ATTEN, 0xaf);
		} else {
			pi_nphy->tx_rx_cal_radio_saveregs[0] =
			        phy_utils_read_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_2G_PWRUP);
			pi_nphy->tx_rx_cal_radio_saveregs[1] =
			        phy_utils_read_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_2G_ATTEN);

			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_2G_PWRUP, 0x3);
			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_2G_ATTEN, 0x7f);
		}
	}
}

static void
wlc_phy_rxcal_radio_cleanup_nphy(phy_info_t *pi, uint8 rx_core)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (rx_core == PHY_CORE_0) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_5G_PWRUP,
			                pi_nphy->tx_rx_cal_radio_saveregs[0]);
			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_5G_ATTEN,
			                pi_nphy->tx_rx_cal_radio_saveregs[1]);

		} else {
			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_2G_PWRUP,
			                pi_nphy->tx_rx_cal_radio_saveregs[0]);
			phy_utils_write_radioreg(pi, RADIO_2057_TX0_TXRXCOUPLE_2G_ATTEN,
			                pi_nphy->tx_rx_cal_radio_saveregs[1]);
		}

	} else {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_5G_PWRUP,
			                pi_nphy->tx_rx_cal_radio_saveregs[0]);
			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_5G_ATTEN,
			                pi_nphy->tx_rx_cal_radio_saveregs[1]);

		} else {
			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_2G_PWRUP,
			                pi_nphy->tx_rx_cal_radio_saveregs[0]);
			phy_utils_write_radioreg(pi, RADIO_2057_TX1_TXRXCOUPLE_2G_ATTEN,
			                pi_nphy->tx_rx_cal_radio_saveregs[1]);
		}
	}
}

static void
wlc_phy_rxcal_physetup_nphy(phy_info_t *pi, uint8 rx_core)
{
	uint8 tx_core;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* intra-core loopback */
	tx_core = rx_core;

	/* save off pre-cal reg values */
	pi_nphy->tx_rx_cal_phy_saveregs[0] = phy_utils_read_phyreg(pi, NPHY_RfseqCoreActv);
	pi_nphy->tx_rx_cal_phy_saveregs[1] = phy_utils_read_phyreg(pi, (rx_core == PHY_CORE_0) ?
	        NPHY_AfectrlCore1 : NPHY_AfectrlCore2);
	pi_nphy->tx_rx_cal_phy_saveregs[2] = phy_utils_read_phyreg(pi, (rx_core == PHY_CORE_0)?
	        NPHY_AfectrlOverride1 : NPHY_AfectrlOverride2);
	pi_nphy->tx_rx_cal_phy_saveregs[3] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc1);
	pi_nphy->tx_rx_cal_phy_saveregs[4] = phy_utils_read_phyreg(pi, NPHY_RfctrlIntc2);
	pi_nphy->tx_rx_cal_phy_saveregs[5] = phy_utils_read_phyreg(pi, NPHY_RfctrlRSSIOTHERS1);
	pi_nphy->tx_rx_cal_phy_saveregs[6] = phy_utils_read_phyreg(pi, NPHY_RfctrlRSSIOTHERS2);
	pi_nphy->tx_rx_cal_phy_saveregs[7] = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride0);
	pi_nphy->tx_rx_cal_phy_saveregs[8] = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride1);
	pi_nphy->tx_rx_cal_phy_saveregs[11] = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride3);
	pi_nphy->tx_rx_cal_phy_saveregs[12] = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride4);
	pi_nphy->tx_rx_cal_phy_saveregs[13] = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride5);
	pi_nphy->tx_rx_cal_phy_saveregs[14] = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride6);
	pi_nphy->tx_rx_cal_phy_saveregs[15] = phy_utils_read_phyreg(pi, NPHY_AntSelConfig2057);

	/* NPHY_IPA */
	pi_nphy->tx_rx_cal_phy_saveregs[9] = phy_utils_read_phyreg(pi, NPHY_PapdEnable0);
	pi_nphy->tx_rx_cal_phy_saveregs[10] = phy_utils_read_phyreg(pi, NPHY_PapdEnable1);

	PHY_REG_LIST_START
		PHY_REG_MOD_CORE_ENTRY(NPHY, 0, PapdEnable, compEnable, 0)
		PHY_REG_MOD_CORE_ENTRY(NPHY, 1, PapdEnable, compEnable, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* The register name in REV7+ is RfseqCoreActv2057, but fields and address
	 * are the same as in REV3-6, so keep the old name
	 */

	/* Intra-core loopback: enable Tx for core being calibrated... */
	phy_utils_mod_phyreg(pi, NPHY_RfseqCoreActv, NPHY_RfseqCoreActv_EnTx_MASK,
	            (1 << tx_core) << NPHY_RfseqCoreActv_EnTx_SHIFT);

	phy_utils_mod_phyreg(pi, NPHY_RfseqCoreActv, NPHY_RfseqCoreActv_DisRx_MASK,
	            (1 << (1-rx_core)) << NPHY_RfseqCoreActv_DisRx_SHIFT);

	/* Power up the ADC */
	phy_utils_mod_phyreg(pi, ((rx_core == PHY_CORE_0) ? NPHY_AfectrlCore1 : NPHY_AfectrlCore2),
	        NPHY_REV3_AfectrlCore_adc_pd_MASK, 0);
	phy_utils_mod_phyreg(pi, (rx_core == PHY_CORE_0)? NPHY_AfectrlOverride1 :
		NPHY_AfectrlOverride2,
		NPHY_REV3_AfectrlOverride_adc_pd_MASK,
		NPHY_REV3_AfectrlOverride_adc_pd_MASK);

	/* Disable the external PA on both cores */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_PA, 0,
		RADIO_MIMO_CORESEL_CORE1 | RADIO_MIMO_CORESEL_CORE2);

	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK,
		0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_rx_MASK,
		0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_tx_MASK,
		1, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_dc_loop_pu_MASK,
		1, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_rx_buf_pu_MASK,
		1, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID2);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_sel_txrx_MASK,
		0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
			2, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	} else {
		wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
			0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	}
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
		0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lna1_pu_MASK,
		0, 0, 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);

	wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RX2TX);

	/* Set the TR switch to T position on the core being calibrated */
	wlc_phy_rfctrlintc_override_nphy(pi, NPHY_RfctrlIntc_override_TRSW, 0x1, rx_core+1);
}

static void
wlc_phy_rxcal_phycleanup_nphy(phy_info_t *pi, uint8 rx_core)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	/* restore pre-cal reg values */
	phy_utils_write_phyreg(pi, NPHY_RfseqCoreActv, pi_nphy->tx_rx_cal_phy_saveregs[0]);
	phy_utils_write_phyreg(pi, (rx_core == PHY_CORE_0) ? NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
	        pi_nphy->tx_rx_cal_phy_saveregs[1]);
	phy_utils_write_phyreg(pi, (rx_core == PHY_CORE_0)?
	                       NPHY_AfectrlOverride1 : NPHY_AfectrlOverride2,
		pi_nphy->tx_rx_cal_phy_saveregs[2]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc1, pi_nphy->tx_rx_cal_phy_saveregs[3]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlIntc2, pi_nphy->tx_rx_cal_phy_saveregs[4]);

	phy_utils_write_phyreg(pi, NPHY_RfctrlRSSIOTHERS1, pi_nphy->tx_rx_cal_phy_saveregs[5]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlRSSIOTHERS2, pi_nphy->tx_rx_cal_phy_saveregs[6]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride0, pi_nphy->tx_rx_cal_phy_saveregs[7]);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride1, pi_nphy->tx_rx_cal_phy_saveregs[8]);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride3, pi_nphy->tx_rx_cal_phy_saveregs[11]);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride4, pi_nphy->tx_rx_cal_phy_saveregs[12]);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride5, pi_nphy->tx_rx_cal_phy_saveregs[13]);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride6, pi_nphy->tx_rx_cal_phy_saveregs[14]);
	phy_utils_write_phyreg(pi, NPHY_AntSelConfig2057, pi_nphy->tx_rx_cal_phy_saveregs[15]);

	/* NPHY_IPA */
	phy_utils_write_phyreg(pi, NPHY_PapdEnable0, pi_nphy->tx_rx_cal_phy_saveregs[9]);
	phy_utils_write_phyreg(pi, NPHY_PapdEnable1, pi_nphy->tx_rx_cal_phy_saveregs[10]);
}

static void
wlc_phy_rxcal_gainctrl_nphy_rev5(phy_info_t *pi, uint8 rx_core, uint16 *rxgain, uint8 cal_type)
{
	/* Also covers REV7+ */
	uint16 num_samps;
	phy_iq_est_t est[NPHY_CORE_NUM];
	uint8 tx_core;
	nphy_iq_comp_t save_comp, zero_comp;
	uint32 i_pwr, q_pwr, curr_pwr, optim_pwr = 0, prev_pwr = 0, thresh_pwr = 10000;
	int16 desired_log2_pwr, actual_log2_pwr, delta_pwr;
	bool gainctrl_done = FALSE;
	uint8 mix_tia_gain = 3;
	int8 optim_gaintbl_index = 0, prev_gaintbl_index = 0;
	int8 curr_gaintbl_index = 3;
	uint8 gainctrl_dirn = NPHY_RXCAL_GAIN_INIT;
	nphy_ipa_txrxgain_t *nphy_rxcal_gaintbl;
	uint16 hpvga, lpf_biq1, lpf_biq0, lna2, lna1;
	int16 fine_gain_idx;
	int8 txpwrindex;
	uint16 nphy_rxcal_txgain[2];
	uint16 tone_freq;

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* intra-core loopback */
	tx_core = rx_core;

	/* 20Mhz bw: 2 Mhz tone, 40Mhz bw: 4 Mhz tone */
	tone_freq = (CHSPEC_IS40(pi->radio_chanspec)) ? NPHY_RXCAL_TONEFREQ_40MHz :
		NPHY_RXCAL_TONEFREQ_20MHz;

	num_samps = 1024;
	desired_log2_pwr = 13;

	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &save_comp);
	zero_comp.a0 = zero_comp.b0 = zero_comp.a1 = zero_comp.b1 = 0x0;
	wlc_phy_rx_iq_coeffs_nphy(pi, 1, &zero_comp);

	/* Adjust mix_tia gain based on A/G band as per rxgain-control choices */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		mix_tia_gain = 3;
		nphy_rxcal_gaintbl = nphy_ipa_rxcal_gaintbl_5GHz_rev7;
	} else {
		nphy_rxcal_gaintbl = nphy_ipa_rxcal_gaintbl_2GHz_rev7;
	}

	do {
		/* REV7+ does not have an analog VGA, so zero-out it's gain code to be safe */
		hpvga = 0;
		lpf_biq1 = nphy_rxcal_gaintbl[curr_gaintbl_index].lpf_biq1;
		lpf_biq0 = nphy_rxcal_gaintbl[curr_gaintbl_index].lpf_biq0;
		lna2 = nphy_rxcal_gaintbl[curr_gaintbl_index].lna2;
		lna1 = nphy_rxcal_gaintbl[curr_gaintbl_index].lna1;
		txpwrindex = nphy_rxcal_gaintbl[curr_gaintbl_index].txpwrindex;

		wlc_phy_rfctrl_override_1tomany_nphy(
			pi, NPHY_REV7_RfctrlOverride_cmd_rxgain,
			((lpf_biq1 << 12) | (lpf_biq0 << 8) |
			 (mix_tia_gain << 4) | (lna2 << 2) | lna1), 0x3, 0);

		pi_nphy->nphy_rxcal_pwr_idx[tx_core] = txpwrindex;

		if (txpwrindex == -1) {
			nphy_rxcal_txgain[0] = 0x8ff0 | pi_nphy->nphy_gmval;
			nphy_rxcal_txgain[1] = 0x8ff0 | pi_nphy->nphy_gmval;
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
			    2, 0x110, 16, nphy_rxcal_txgain);
		} else {
			wlc_phy_txpwr_index_nphy(pi, tx_core+1, txpwrindex, FALSE);
		}

		/* Send a reference tone */
		wlc_phy_tx_tone_nphy(pi, tone_freq, NPHY_RXCAL_TONEAMP, 0, cal_type, FALSE);

		/* Get the Rx power */
		wlc_phy_rx_iq_est_nphy(pi, est, num_samps, 32, 0);
		i_pwr = (est[rx_core].i_pwr + num_samps / 2) / num_samps;
		q_pwr = (est[rx_core].q_pwr + num_samps / 2) / num_samps;
		curr_pwr = i_pwr + q_pwr;
		PHY_CAL(("core %u (gain idx %d): i_pwr = %u, q_pwr = %u, curr_pwr = %d\n",
		         rx_core, curr_gaintbl_index, i_pwr, q_pwr, curr_pwr));

		switch (gainctrl_dirn) {
		case NPHY_RXCAL_GAIN_INIT:
			if (curr_pwr > thresh_pwr) {
				gainctrl_dirn = NPHY_RXCAL_GAIN_DOWN;
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index--;
			} else {
				gainctrl_dirn = NPHY_RXCAL_GAIN_UP;
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index++;
			}
			break;

		case NPHY_RXCAL_GAIN_UP:
			if (curr_pwr > thresh_pwr) {
				gainctrl_done = TRUE;
				optim_pwr = prev_pwr;
				optim_gaintbl_index = prev_gaintbl_index;
			} else {
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index++;
			}
			break;

		case NPHY_RXCAL_GAIN_DOWN:
			if (curr_pwr > thresh_pwr) {
				prev_gaintbl_index = curr_gaintbl_index;
				curr_gaintbl_index--;
			} else {
				gainctrl_done = TRUE;
				optim_pwr = curr_pwr;
				optim_gaintbl_index = curr_gaintbl_index;
			}
			break;

		default:
			PHY_ERROR(("Invalid gaintable direction id %d\n", gainctrl_dirn));
			ASSERT(0);
		}

		if ((curr_gaintbl_index < 0) ||
			(curr_gaintbl_index > NPHY_IPA_RXCAL_MAXGAININDEX)) {
			gainctrl_done = TRUE;
			optim_pwr = curr_pwr;
			optim_gaintbl_index = prev_gaintbl_index;
		} else {
			prev_pwr = curr_pwr;
		}

		/* Turn off the tone */
		wlc_phy_stopplayback_nphy(pi);
	} while (! gainctrl_done);

	hpvga = nphy_rxcal_gaintbl[optim_gaintbl_index].hpvga;
	lpf_biq1 = nphy_rxcal_gaintbl[optim_gaintbl_index].lpf_biq1;
	lpf_biq0 = nphy_rxcal_gaintbl[optim_gaintbl_index].lpf_biq0;
	lna2 = nphy_rxcal_gaintbl[optim_gaintbl_index].lna2;
	lna1 = nphy_rxcal_gaintbl[optim_gaintbl_index].lna1;
	txpwrindex = nphy_rxcal_gaintbl[optim_gaintbl_index].txpwrindex;

	actual_log2_pwr = math_nbits_32(optim_pwr);
	delta_pwr = desired_log2_pwr - actual_log2_pwr;

	fine_gain_idx = (int)lpf_biq1 + delta_pwr;
	/* Limit Total LPF To 30 dB */
	if (fine_gain_idx + (int)lpf_biq0 > 10) {
		lpf_biq1 = 10 - lpf_biq0;
	} else {
		lpf_biq1 = (uint16) MAX(fine_gain_idx, 0);
	}

	wlc_phy_rfctrl_override_1tomany_nphy(
		pi, NPHY_REV7_RfctrlOverride_cmd_rxgain,
		((lpf_biq1 << 12) | (lpf_biq0 << 8) |
		 (mix_tia_gain << 4) | (lna2 << 2) | lna1), 0x3, 0);
	PHY_CAL(("FINAL: gainIdx=%3d, lna1=%3d, lna2=%3d, mix_tia=%3d, "
	         "lpf0=%3d, lpf1=%3d, txpwridx=%3d\n",
	         optim_gaintbl_index, lna1, lna2, mix_tia_gain,
	         lpf_biq0, lpf_biq1, txpwrindex));

	if (rxgain != NULL) {
		*rxgain++ = lna1;
		*rxgain++ = lna2;
		*rxgain++ = mix_tia_gain;
		*rxgain++ = lpf_biq0;
		*rxgain++ = lpf_biq1;
		*rxgain = hpvga;
	}

	wlc_phy_rx_iq_coeffs_nphy(pi, 1, &save_comp);
}

static void
wlc_phy_rxcal_gainctrl_nphy(phy_info_t *pi, uint8 rx_core, uint16 *rxgain, uint8 cal_type)
{
	wlc_phy_rxcal_gainctrl_nphy_rev5(pi, rx_core, rxgain, cal_type);
}

#define WAIT_FOR_SCOPE	4000000 /* in unit of us */

int
wlc_phy_cal_rxiq_nphy(phy_info_t *pi, nphy_txgains_t target_gain, uint8 cal_type, bool debug,
                      uint8 core_mask)
{
	uint16 orig_BBConfig;
	uint8  core_no, rx_core;
	uint16 gain_save[4];
	uint16 cal_gain[NPHY_CORE_NUM];
	nphy_iqcal_params_t cal_params[NPHY_CORE_NUM];
	uint8 rxcore_state;
	bool   phyhang_avoid_state = FALSE;
	bool skip_rxiqcal = FALSE;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* REV7+ needs IQ-cal only, RC-cal is done in the radio */
	cal_type = 0;

	/* override bt priority */
	wlc_btcx_override_enable(pi);

	/* Disable the re-sampler (in case we are in spur avoidance mode) */
	orig_BBConfig = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_mod_phyreg(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK, 0);

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Disable phyhang_avoid */
	phyhang_avoid_state = pi_nphy->phyhang_avoid;
	pi_nphy->phyhang_avoid   = FALSE;

	/* save tx gain setting & set new gain settings */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, gain_save);
	FOREACH_CORE(pi, core_no) {
		wlc_phy_iqcal_gainparams_nphy(pi, core_no, target_gain,
			&cal_params[core_no]);
			PHY_CAL(("tx core %d cal params: pad %d,pga %d,txgm %d, "
				"txlpf %d,ipa %d, cal_gain 0x%x\n",
				core_no, cal_params[core_no].pad,
				cal_params[core_no].pga, cal_params[core_no].txgm,
				cal_params[core_no].txlpf, cal_params[core_no].ipa,
				cal_params[core_no].cal_gain));
			cal_gain[core_no] = cal_params[core_no].cal_gain;
	}

	/* Set and force tx cal_gain to take effect */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, cal_gain);

	rxcore_state = wlc_phy_rxcore_getstate_nphy((wlc_phy_t *)pi);

	/* run over requested cores */
	for (rx_core = 1- (core_mask & 0x1);
		rx_core <= ((pi->pubpi->phy_corenum == 1) ? 0 : (core_mask >> 1)); rx_core++) {
		/* If this Rx core is disabled, skip the calibration for this core */
		skip_rxiqcal = ((rxcore_state & (1 << rx_core)) == 0) ? TRUE: FALSE;

		/* Save PHY registers and setup the AFE */
		wlc_phy_rxcal_physetup_nphy(pi, rx_core);

		/* Connect the 2 cores internally in the radio */
		wlc_phy_rxcal_radio_setup_nphy(pi, rx_core);

		/* cal_type = 0 ==> rxiq_cal, cal_type = 1 ==> rc_cal, cal_type = 2 ==> both cals */
		if (!skip_rxiqcal) {
			/* per-core rxgain control, ensure that the tone amplitude
			 * covers the desired ADC range of the RX core
			 */
			PHY_CAL(("\nBefore RxIQCAL rxgainctrl\n"));
			wlc_phy_rxcal_gainctrl_nphy(pi, rx_core, NULL, 0);

			/* Do the Rx IQ calibration */
			wlc_phy_tx_tone_nphy(pi,
			    (CHSPEC_IS40(pi->radio_chanspec))?
			    NPHY_RXCAL_TONEFREQ_40MHz : NPHY_RXCAL_TONEFREQ_20MHz,
			    NPHY_RXCAL_TONEAMP,
			    0,
			    cal_type, FALSE);

			if (debug)
				OSL_DELAY(WAIT_FOR_SCOPE);

			wlc_phy_calc_rx_iq_comp_nphy(pi, rx_core+1);
			wlc_phy_stopplayback_nphy(pi);
		}

		/* Break the internal connection between the 2 cores inside the radio */
		wlc_phy_rxcal_radio_cleanup_nphy(pi, rx_core);

		/* Restore PHY registers and AFE */
		wlc_phy_rxcal_phycleanup_nphy(pi, rx_core);
		wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);
	}

	/* Restore the state of the re-sampler (in case we are in spur avoidance mode) */
	phy_utils_write_phyreg(pi, NPHY_BBConfig, orig_BBConfig);

	wlc_phy_resetcca_nphy(pi);

	/* restore pre-cal gain */
	wlc_phy_rfctrl_override_1tomany_nphy(pi, NPHY_REV7_RfctrlOverride_cmd_rxgain, 0, 0x3, 1);
	wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, gain_save);

	/* Enable phyhang_avoid for NPHY_REV4+ */
	pi_nphy->phyhang_avoid = phyhang_avoid_state;

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	/* remove override */
	wlc_phy_btcx_override_disable(pi);

	return BCME_OK;
}

/** configure tx filter for SM shaping in ExtPA case */
static void
wlc_phy_extpa_set_tx_digi_filts_nphy(phy_info_t *pi)
{
	int j;
	uint16 addr_offset = NPHY_ccktxfilt20CoeffStg0A1;
	uint8 filter_type = NPHY_FILT_CCK_GAUSS2_2;
	uint8 ofdm_filter_type = NPHY_FILT_4322_20MHZ;
	uint8 ofdm40_filt_type = NPHY_FILT_4322_40MHZ;
	uint16 ofdm_addr_offset = NPHY_txfilt20CoeffStg0A1;
	uint16 ofdm40_addr_offset = NPHY_txfilt40CoeffStg0A1;

	for (j = 0; j < NPHY_NUM_DIG_FILT_COEFFS; j++) {
		phy_utils_write_phyreg(pi, addr_offset+j,
		              NPHY_IPA_REV4_txdigi_filtcoeffs[filter_type][j]);
		phy_utils_write_phyreg(pi, ofdm_addr_offset+j,
			NPHY_IPA_REV4_txdigi_filtcoeffs[ofdm_filter_type][j]);
		phy_utils_write_phyreg(pi, ofdm40_addr_offset+j,
			NPHY_IPA_REV4_txdigi_filtcoeffs[ofdm40_filt_type][j]);
	}
}

static void
wlc_phy_intpa_set_tx_digi_filts_nphy(phy_info_t *pi, uint16 addr_offset, int idx)
{
	int j;
	for (j = 0; j < NPHY_NUM_DIG_FILT_COEFFS; j++) {
		phy_utils_write_phyreg(pi, addr_offset+j,
			NPHY_IPA_REV4_txdigi_filtcoeffs[idx][j]);
	}
}

/* mimophy_set_ipa_filt_coeffs */
/** configure tx filter for SM shaping in IPA case */
static void
wlc_phy_ipa_set_tx_digi_filts_nphy(phy_info_t *pi)
{
	int txfilt20_type, txfilt40_type, ccktxfilt20_type;
	uint16 addr_offset[] = {NPHY_txfilt20CoeffStg0A1, NPHY_txfilt40CoeffStg0A1,
		NPHY_ccktxfilt20CoeffStg0A1};
	uint16 curr_channel = 0;
	int val; /* general purpose variable */

	curr_channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	txfilt20_type    = NPHY_FILT_OFDM20;
	txfilt40_type    = NPHY_FILT_OFDM40;
	ccktxfilt20_type = NPHY_FILT_CCK_GAUSS2_2;

	/* Using a new cck filter for 43217 */
	if ((NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV + 1)) &&
	    (RADIOREV(pi->pubpi->radiorev) == 14)) {
		txfilt20_type = NPHY_FILT_43217_OFDM20;
	}

	if (NREV_GE(pi->pubpi->phy_rev, 8)) {
		if ((pi->sh->sromrev >= 8) && (pi->sh->boardflags2 & BFL2_FCC_BANDEDGE_WAR) &&
		    ((curr_channel == 3) && CHSPEC_IS40(pi->radio_chanspec))) {
			/* Use sharper 40 MHz filter
			 * Note: Because of channel dependency, the filter coeffs
			 * must also be set in wlc_phy_chanspec_radio2057_setup()
			 */
			if ((NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) &&
			    (RADIOREV(pi->pubpi->radiorev) == 14)) {
				val = NPHY_FILT_43217_OFDM26;
			} else {
				val = NPHY_FILT_OFDM22;
			}
		} else {
			if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
				if ((RADIOREV(pi->pubpi->radiorev) == 14) &&
				    (RADIOID(pi->pubpi->radioid) == BCM2057_ID)) {
					val = NPHY_FILT_OFDM40;
				} else {
					val = NPHY_FILT_53572_OFDM40;
				}
			} else {
				val = NPHY_FILT_OFDM40;
			}
		}
		txfilt40_type = val;
	}

	if (IS40MHZ(pi)) {
		/* to use default 20mhz ofdm filter for 20in40 mode */
		txfilt20_type = NPHY_FILT_4322_20MHZ;
	} else {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			txfilt20_type = NPHY_FILT_ABAND_OFDM20;
		}

		/* Japanese channel 14 bandwidth occupancy requirements */
		if (CHSPEC_CHANNEL(pi->radio_chanspec) == 14) {
			ccktxfilt20_type = NPHY_FILT_CCK_JAPAN_CH14;
		}
	}

	PHY_INFORM(("%s: txfilt20_type=%d, txfilt40_type=%d, ccktxfilt20_type=%d",
		__FUNCTION__, txfilt20_type, txfilt40_type, ccktxfilt20_type));

	/* Do the actual programming */
	wlc_phy_intpa_set_tx_digi_filts_nphy(pi, addr_offset[0], txfilt20_type);
	wlc_phy_intpa_set_tx_digi_filts_nphy(pi, addr_offset[1], txfilt40_type);
	wlc_phy_intpa_set_tx_digi_filts_nphy(pi, addr_offset[2], ccktxfilt20_type);

} /* wlc_phy_ipa_set_tx_digi_filts_nphy */

/** restore to default OFDM filter */
static void
wlc_phy_ipa_set_cal_tx_digi_filts_nphy(phy_info_t *pi)
{
	if (IS40MHZ(pi)) {
		/* 4322 (default 40mhz) */
		wlc_phy_intpa_set_tx_digi_filts_nphy(pi, NPHY_txfilt40CoeffStg0A1,
			NPHY_FILT_4322_40MHZ);
	} else {
		/* 4322 (default 20mhz) */
		wlc_phy_intpa_set_tx_digi_filts_nphy(pi, NPHY_txfilt20CoeffStg0A1,
			NPHY_FILT_4322_20MHZ);
	}
}

static uint16
wlc_phy_ipa_get_bbmult_nphy(phy_info_t *pi)
{
	uint16 m0m1;

	wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

	return m0m1;
}

static void
wlc_phy_ipa_set_bbmult_nphy(phy_info_t *pi, uint8 m0, uint8 m1)
{
	uint16 m0m1 = (uint16)((m0 << 8) | m1);

	wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m0m1);
	wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m0m1);
}

/** Get a pointer to the (iPA) PHY Tx gain table */
static uint32*
wlc_phy_get_ipa_gaintbl_nphy(phy_info_t *pi)
{
	uint32* tx_pwrctrl_tbl = NULL;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* 2.4G gain tables */
		if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV + 1)) {
			if (RADIOREV(pi->pubpi->radiorev) == 13) {
			/* new table for 53572 */
			tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev13;
			} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
				/* new table for 43217 */
				tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev14;
			} else {
				tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev13;
			}
		} else if (NREV_IS(pi->pubpi->phy_rev, 7)) {
			/* 43226a[01], 6362a0 */

			if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
				/* 6362a0 */
				tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev3;
			} else {
				PHY_ERROR(("Unsupported NREV %d radio rev %d combo\n",
				           pi->pubpi->phy_rev, RADIOREV(pi->pubpi->radiorev)));
				ASSERT(0);
			}

		} else if (NREV_IS(pi->pubpi->phy_rev, 8)) {
			/* 43236a0, 6362b0 */
			if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
			           (CHIPID(pi->sh->chip) == BCM43235_CHIP_ID) ||
			           (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID)) {
				/* 43236a0 (b0 and b1 are NREV 9) */
				tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev7;
			} else if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
				if (RADIOREV(pi->pubpi->radiorev) == 12) {
					/* BCM63268 */
					tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev12;
				} else {
					/* 6362b0 */
					tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev8;
				}
			} else {
				PHY_ERROR(("Unsupported NREV %d radio rev %d combo\n",
				           pi->pubpi->phy_rev, RADIOREV(pi->pubpi->radiorev)));
				ASSERT(0);
			}

		} else if (NREV_IS(pi->pubpi->phy_rev, 9)) {
			/* 5357b0, 5357b1, 43236b0, 43236b1 */

			if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
			    (CHIPID(pi->sh->chip) == BCM43235_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43234_CHIP_ID)) {

				if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
					(RADIOVER(pi->pubpi->radiover) == 2)) {
					/* 43236b1 */
					tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev7_ver2;
				} else {
					/* 43236b0, using same as 43236a0 for now */
					tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_2g_2057rev7;
				}

			} else {
				PHY_ERROR(("Unsupported NREV %d radio rev %d and ver %d combo\n",
				           pi->pubpi->phy_rev, RADIOREV(pi->pubpi->radiorev),
				           RADIOVER(pi->pubpi->radiover)));
				ASSERT(0);
			}

		} else {
			/* MIMOPHY REV3-4 */
			tx_pwrctrl_tbl = nphy_tpc_txgain_ipa;
		}

	} else {
		/* 5G gain tables */
		if (NREV_IS(pi->pubpi->phy_rev, 7)) {
			/* 43226a[01], and 6362a0 */
			tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_5g_2057;

		} else if (NREV_IS(pi->pubpi->phy_rev, 8)) {
			/* 43236a0, 6362b0 */

			if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
			    (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID)) {
				/* 43236a0 */
				tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_5g_2057rev7;

			} else if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
				if (RADIOVER(pi->pubpi->radiover) == 12) {
					/* BCM63268 */
					tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_5g_2057rev12;
				} else {
					/* 6362b0 */
					tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_5g_2057rev8;
				}

			} else {
				PHY_ERROR(("Unsupported NREV %d radio rev %d combo\n",
				           pi->pubpi->phy_rev, RADIOREV(pi->pubpi->radiorev)));
				ASSERT(0);
			}

		} else if  (NREV_IS(pi->pubpi->phy_rev, 9)) {
			/* 43236b0,43236b1 only for now */
			if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
			    (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID)) {
				/* 43236b0, using same as 43236a0 for now */
			  if (RADIOVER(pi->pubpi->radiover) == 2) {
				tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_5g_2057rev7_sul;
			  } else {
				tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_5g_2057rev7;
			  }
			} else {
				PHY_ERROR(("Unsupported NREV %d radio rev %d and ver %d combo\n",
				           pi->pubpi->phy_rev, RADIOREV(pi->pubpi->radiorev),
				           RADIOVER(pi->pubpi->radiover)));
				ASSERT(0);
			}

		} else {
			tx_pwrctrl_tbl = nphy_tpc_txgain_ipa_5g;
		}
	}

	return tx_pwrctrl_tbl;
}

/** Get a pointer to the (ePA) PHY Tx gain table */
uint32*
wlc_phy_get_epa_gaintbl_nphy(phy_info_t *pi)
{
	uint32* tx_pwrctrl_tbl = NULL;

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (pi->sh->boardtype == 0xF52A) {
			/* corresponds to the Cisco E3200 brd */
			tx_pwrctrl_tbl = nphy_tpc_5GHz_txgain_epa_2057rev7_brdtype_0xF52A;
		} else if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
		           (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
		           (CHIPID(pi->sh->chip) == BCM43238_CHIP_ID)) {
			if (RADIOVER(pi->pubpi->radiover) == 2) {
				/* 43236b1 */
				tx_pwrctrl_tbl = nphy_tpc_5GHz_txgain_epa_2057rev7_5gv7;
			} else {
				/* 43236b0, using same as 43236a0 for now */
				tx_pwrctrl_tbl = nphy_tpc_5GHz_txgain_epa_2057rev7;
			}
		} else tx_pwrctrl_tbl = nphy_tpc_5GHz_txgain_epa_2057rev7;
	} else {
		if (NREV_IS(pi->pubpi->phy_rev, 7)) {
			/* 6362a0 only */
			if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
				/* 6362a0 */
				tx_pwrctrl_tbl = nphy_tpc_txgain_epa_2057rev3;
			}
		} else if (NREV_IS(pi->pubpi->phy_rev, 8)) {
			/* 43236a0, 6362b0 */
			if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
			           (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
			           (CHIPID(pi->sh->chip) == BCM43238_CHIP_ID) ||
			           (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID)) {
				if (RADIOVER(pi->pubpi->radiover) == 2) {
					/* 43236b1 */
					tx_pwrctrl_tbl = nphy_tpc_txgain_epa_2057rev7_2gv14;
				} else {
					/* 43236b0 43236a0, 6362b0 */
					tx_pwrctrl_tbl = nphy_tpc_txgain_epa_2057rev7;
				}
			}
		} else if (NREV_IS(pi->pubpi->phy_rev, 9)) {
			/* 43236b0 */
			if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
			           (CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
			           (CHIPID(pi->sh->chip) == BCM43238_CHIP_ID)) {
				/* 43236b0, using same as 43236a0 for now */
				tx_pwrctrl_tbl = nphy_tpc_txgain_epa_2057rev7;

			}
		} else if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
			if (CHIPID(pi->sh->chip) == BCM43131_CHIP_ID ||
			           CHIPID(pi->sh->chip) == BCM43217_CHIP_ID) {
				/* 43217 , using independent one */
				tx_pwrctrl_tbl = nphy_tpc_txgain_epa_2057rev7_2gv17;
			}
		} else {
			if (pi->fem2g->extpagain == 3) {
				tx_pwrctrl_tbl =  nphy_tpc_txgain_HiPwrEPA;
			} else {
				tx_pwrctrl_tbl =  nphy_tpc_txgain_rev3;
			}
		}
	}

	if (tx_pwrctrl_tbl == NULL) {
		tx_pwrctrl_tbl = nphy_tpc_txgain_epa_2057rev5v2;
		PHY_ERROR(("Unsupported NREV %d radio rev %d combo\n",
			pi->pubpi->phy_rev, RADIOREV(pi->pubpi->radiorev)));
		ASSERT(0);
	}
	return tx_pwrctrl_tbl;
}

/** NPHY PAPD calibration setup for particular core */
static void
wlc_phy_papd_cal_setup_nphy(phy_info_t *pi, nphy_papd_restore_state *state, uint8 core)
{
	int32 tone_freq;
	uint8 off_core;
	uint16 mixgain = 0;
	uint16 freq = 0;
	uint16 lpfbwctl = 0;
	uint16 val;

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_CAL(("Setting up papd cal on core %d channel: %d\n", core,
	           CHSPEC_CHANNEL(pi->radio_chanspec)));

	ASSERT(core < NPHY_CORE_NUM);

	off_core = core ^ 0x1;

	lpfbwctl = wlc_phy_read_lpf_bw_ctl_nphy(pi, 0);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		freq = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		freq = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	if (CHIPID_43236X_FAMILY(pi)) {
		if (CHSPEC_IS40(pi->radio_chanspec)) {
			if (freq == 5190) {
				lpfbwctl = 2;
			} else if (freq == 5310 || freq == 5510 || freq == 2422 ||
			           freq == 2452) {
				lpfbwctl = 0;
			}
		} else {
			if (freq == 2412 || freq == 2462 || freq == 5180 ||
			    freq == 5320 || freq == 5500)
				lpfbwctl = 0;
		}
	}

	wlc_phy_rfctrl_override_nphy_rev7(pi,
	                                  NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
	                                  lpfbwctl, 0, 0,
	                                  NPHY_REV7_RFCTRLOVERRIDE_ID1);

	/* Select the Rx gain to be used in the PAPD loopback path. The Rx mixer is the
	 * only Rx gain element that is present in the PAPD loopback path.
	 */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (RADIOREV(pi->pubpi->radiorev) == 5) {
			if (RADIOVER(pi->pubpi->radiover) == 0x0) {
				/* 5357A0 */
				mixgain = (core == 0) ? 0x20 : 0x00;
			} else {
				/* 5357B0 */
				mixgain = (core == 0) ? 0x20 : 0x10;
			}

		} else if (RADIOREV(pi->pubpi->radiorev) == 7) {
			/* 43236 */
			mixgain = 0x10;

		} else if ((RADIOREV(pi->pubpi->radiorev) <= 4) ||
			(RADIOREV(pi->pubpi->radiorev) == 6) ||
			(RADIOREV(pi->pubpi->radiorev) == 8)) {
			/* 43226a[01], 6362a0, 6362b0 */
			mixgain = 0x00;
		} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
			/* BCM63268B0 */
			mixgain = 0x00;
		} else if (RADIOREV(pi->pubpi->radiorev) == 13) {
			mixgain = 0x00;
		} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
			/* 43217 */
			mixgain = 0x00;
		} else {
			PHY_ERROR(("Unsupported radio rev %d\n",
				RADIOREV(pi->pubpi->radiorev)));
			ASSERT(0);
		}

	} else {
		if ((RADIOREV(pi->pubpi->radiorev) == 4) ||
		    (RADIOREV(pi->pubpi->radiorev) == 6)) {
			/* 43226a0/1 */
			mixgain = 0x50;
		} else if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
			(RADIOREV(pi->pubpi->radiorev) == 7) ||
			(RADIOREV(pi->pubpi->radiorev) == 8) ||
			(RADIOREV(pi->pubpi->radiorev) == 12))
		{
			/* 43236a0,b0,b1, 6362[ab]0 */
			mixgain = 0x0;
		} else {
			PHY_ERROR(("Unsupported radio rev %d\n",
				RADIOREV(pi->pubpi->radiorev)));
			ASSERT(0);
		}
	}

	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rxgain_MASK,
	                                  mixgain, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	/* ovr Tx (no LPF/iPA) to be ON for core being cal'd, and OFF for alternate core */
	wlc_phy_rfctrl_override_1tomany_nphy(pi, NPHY_REV7_RfctrlOverride_cmd_tx_pu,
	                                     1, (1 << core), 0);
	wlc_phy_rfctrl_override_1tomany_nphy(pi, NPHY_REV7_RfctrlOverride_cmd_tx_pu,
	                                     0, (1 << off_core), 0);

	/* Shut off both PAs. They will be turned on just before the cal to reduce the
	 * spewing of energy into the air.
	 */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK,
	                                  0, 0x3, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rxif_pu_MASK,
	                                  1, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_dc_loop_pu_MASK,
	                                  0, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_rx_buf_pu_MASK,
	                                  1, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID2);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_dc_MASK,
	                                  0, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_rx_MASK,
	                                  1, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	val = 0;
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_tx_MASK,
	                                val, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rxmx_pu_MASK,
	                                  1, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);

	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lna1_pu_MASK,
	                                  0, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lna2_pu_MASK,
	                                  0, (1 << core), 0, NPHY_REV7_RFCTRLOVERRIDE_ID1);

	state->afectrl[core] = phy_utils_read_phyreg(pi, (core == PHY_CORE_0) ?
	                                    NPHY_AfectrlCore1 : NPHY_AfectrlCore2);
	state->afeoverride[core] = phy_utils_read_phyreg(pi, (core == PHY_CORE_0) ?
	                                        NPHY_AfectrlOverride1 :
	                                        NPHY_AfectrlOverride2);
	state->afectrl[off_core] = phy_utils_read_phyreg(pi, (core == PHY_CORE_0) ?
	                                        NPHY_AfectrlCore2 : NPHY_AfectrlCore1);
	state->afeoverride[off_core] = phy_utils_read_phyreg(pi, (core == PHY_CORE_0) ?
	                                            NPHY_AfectrlOverride2 :
	                                            NPHY_AfectrlOverride1);
	state->tr2g_config1 = phy_utils_read_radioreg(pi, RADIO_2057_TR2G_CONFIG1_CORE0_NU);

	PHY_CAL(("PAPD setup : core %d afectrl 0x%x afeoverride 0x%x, off_core %d afectrl "
	         "0x%x afeoverride 0x%x\n", core, state->afectrl[core],
	         state->afeoverride[core], off_core, state->afectrl[off_core],
	         state->afeoverride[off_core]));

	/* Power up the ADC for specified core (adc_pd only) */
	phy_utils_mod_phyreg(pi, ((core == PHY_CORE_0) ? NPHY_AfectrlCore1 : NPHY_AfectrlCore2),
		NPHY_REV3_AfectrlCore_adc_pd_MASK, 0);
	phy_utils_mod_phyreg(pi, ((core == PHY_CORE_0) ? NPHY_AfectrlOverride1 :
	                 NPHY_AfectrlOverride2),
	            NPHY_REV3_AfectrlCore_adc_pd_MASK, NPHY_REV3_AfectrlCore_adc_pd_MASK);
	/* Power down the ADC for the other core */
	phy_utils_mod_phyreg(pi, ((core == PHY_CORE_0) ? NPHY_AfectrlCore2 : NPHY_AfectrlCore1),
		NPHY_REV3_AfectrlCore_adc_pd_MASK, NPHY_REV3_AfectrlCore_adc_pd_MASK);
	phy_utils_mod_phyreg(pi, ((core == PHY_CORE_0) ? NPHY_AfectrlOverride2 :
	                 NPHY_AfectrlOverride1),
	            NPHY_REV3_AfectrlCore_adc_pd_MASK, NPHY_REV3_AfectrlCore_adc_pd_MASK);

	/* Set Rx path mux to PAPD and turn on PAPD mixer */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		state->pwrup[core] = READ_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_2G_PWRUP);
		state->atten[core] = READ_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_2G_ATTEN);
		state->pwrup[off_core] = READ_RADIO_REG3(pi, RADIO_2057, TX, off_core,
		                                          TXRXCOUPLE_2G_PWRUP);
		state->atten[off_core] = READ_RADIO_REG3(pi, RADIO_2057, TX, off_core,
		                                          TXRXCOUPLE_2G_ATTEN);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_2G_PWRUP, 0xc);

		/* Loopback atten is radio-rev specific */
		if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
		    (RADIOREV(pi->pubpi->radiorev) == 4) ||
		    (RADIOREV(pi->pubpi->radiorev) == 6)) {
			/* 43226a0/1 and 6362a0 */
			val = 0xf0;
		} else if (RADIOREV(pi->pubpi->radiorev) == 5) {
			if (RADIOVER(pi->pubpi->radiover) == 0x0) {
				val = (core == 0) ? 0xf7 : 0xf2;
			} else {
				val = (core == 0) ? 0xf3 : 0xf2; /* 5357b0 */
			}
		} else if (RADIOREV(pi->pubpi->radiorev) == 8) {
			val = 0xf0; /* 6362b0 */
		} else if (RADIOREV(pi->pubpi->radiorev) == 7) {
			val = 0xf0; /* 43236 */
		} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
			val = 0xf0; /* BCM63268 */
		} else if (RADIOREV(pi->pubpi->radiorev) == 13) {
			val = 0xf2; /* 53572 */
		} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
			val = 0xf2; /* 43217 */
		} else {
			PHY_ERROR(("Unsupported 2057 radio rev %d\n",
				RADIOREV(pi->pubpi->radiorev)));
			ASSERT(0);
			val = 0; /* prevents compiler error */
		}
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core,     TXRXCOUPLE_2G_ATTEN, val);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, off_core, TXRXCOUPLE_2G_PWRUP, 0x0);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, off_core, TXRXCOUPLE_2G_ATTEN,
		                 0xff);

		/* LCNXN */
		/* the override logic to short the LNA input */
		/* to ground during Tx is not correct in the */
		/* 2G band. Hence, a manual radio override needs */
		/* to be issued to short the LNA1 input to ground */
		/* for PAPD cal. Shorting the input of LNA1 */
		/* to ground helps avoid/reduce the output of the */
		/* internal PA coupling back into the PAPD */
		/* Rx loopback path via LNA1 (which may happen even if it is turned off) */
		if (pi_nphy->nphy_papd_kill_switch_en) {
			if (RADIOREV(pi->pubpi->radiorev) == 7) {
				state->tr2g_config1 = phy_utils_read_radioreg(pi,
					RADIO_2057_TR2G_CONFIG1_CORE0_NU);
				phy_utils_mod_radioreg(pi, RADIO_2057_TR2G_CONFIG1_CORE0_NU,
					0x1, 0x0);
				phy_utils_mod_radioreg(pi, RADIO_2057_TR2G_CONFIG1_CORE1_NU,
					0x1, 0x0);
			} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
				/* 43217 kill switch/short setting */
				state->tr2g_config1 = phy_utils_read_radioreg(pi,
					RADIO_2057_TR2G_CONFIG1_CORE0_NU);
				state->reg10 =
					phy_utils_read_radioreg(pi, RADIO_2057_OVR_REG10);
				state->reg20 =
					phy_utils_read_radioreg(pi, RADIO_2057_OVR_REG20);
				state->reg21 =
					phy_utils_read_radioreg(pi, RADIO_2057_OVR_REG21);
				phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG10, 0xc);
				phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG20, 0x1);
				phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG21, 0x80);
				phy_utils_write_radioreg(pi,
				                RADIO_2057_TR2G_CONFIG1_CORE0_NU, 0x2);
				phy_utils_write_radioreg(pi,
				                RADIO_2057_TR2G_CONFIG1_CORE1_NU, 0x2);
				state->reg29 =
					phy_utils_read_radioreg(pi, RADIO_2057_OVR_REG29);
				state->tr2g_config4_core[0] =
					phy_utils_read_radioreg(pi, RADIO_2057_TR2G_CONFIG4_CORE0);
				state->tr2g_config4_core[1] =
					phy_utils_read_radioreg(pi, RADIO_2057_TR2G_CONFIG4_CORE1);
				phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG29, 0x48);
				phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG4_CORE0,
				                         0x2);
				phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG4_CORE1,
				                         0x2);
			}
		}

	} else {
		state->pwrup[core] = READ_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_5G_PWRUP);
		state->atten[core] = READ_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_5G_ATTEN);
		state->pwrup[off_core] = READ_RADIO_REG3(pi, RADIO_2057, TX, off_core,
		                                          TXRXCOUPLE_5G_PWRUP);
		state->atten[off_core] = READ_RADIO_REG3(pi, RADIO_2057, TX, off_core,
		                                          TXRXCOUPLE_5G_ATTEN);

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_5G_PWRUP, 0xc);

		/* Loopback atten is radio-rev specific */
		if (RADIOREV(pi->pubpi->radiorev) == 7) {
			if (RADIOVER(pi->pubpi->radiover) == 0) {
				val = 0xf4; /* 43236a0 */
			} else {
				val = 0xf0; /* 43236a0TC3, TC4, B0 */
			}
		} else if (RADIOREV(pi->pubpi->radiorev) == 8) {
			/* 6362b0 */
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				val = 0xf0;
			} else {
				val = 0xf2;
			}
		} else {
			val = 0xf0;
		}

		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core,     TXRXCOUPLE_5G_ATTEN, val);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, off_core, TXRXCOUPLE_5G_PWRUP, 0x0);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, off_core, TXRXCOUPLE_5G_ATTEN, 0xff);
	}

	tone_freq = 4000;
	/* test force txpwrindex */
	wlc_phy_tx_tone_nphy(pi, tone_freq, 181, 0, 0, FALSE);

	/* PAPD regs common */
	PHY_REG_MOD_CORE(pi, NPHY, core, PapdEnable, compEnable, NPHY_PAPD_COMP_ON);
	PHY_REG_MOD_CORE(pi, NPHY_REV6, core, PapdCalShifts, calEnable, 1);
	PHY_REG_MOD_CORE(pi, NPHY, off_core, PapdEnable, compEnable, NPHY_PAPD_COMP_OFF);
	PHY_REG_MOD_CORE(pi, NPHY_REV6, off_core, PapdCalShifts, calEnable, 0);
}

static void
wlc_phy_papd_cal_cleanup_nphy(phy_info_t *pi, nphy_papd_restore_state *state)
{
	uint8 core;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_CAL(("Cleanning up papd cal on channel: %d\n", CHSPEC_CHANNEL(pi->radio_chanspec)));

	wlc_phy_stopplayback_nphy(pi);     /* mimophy_stop_playback */

	FOREACH_CORE(pi, core) {
		/* Restore attenuation settings and turn off loopback bath for both cores */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_2G_PWRUP, 0);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_2G_ATTEN,
			                 state->atten[core]);
		} else {
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_5G_PWRUP, 0);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TXRXCOUPLE_5G_ATTEN,
			                 state->atten[core]);
		}
	}
	if (pi_nphy->nphy_papd_kill_switch_en) {
		if (RADIOREV(pi->pubpi->radiorev) == 7) {
			/* LCNXN */
			phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG1_CORE0_NU,
				state->tr2g_config1);
			phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG1_CORE1_NU,
				state->tr2g_config1);
		} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
			/* 43217 restore kill switch setting when papd is done */
			phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG1_CORE0_NU,
				state->tr2g_config1);
			phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG1_CORE1_NU,
				state->tr2g_config1);
			phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG10, state->reg10);
			phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG20, state->reg20);
			phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG21, state->reg21);
			phy_utils_write_radioreg(pi, RADIO_2057_OVR_REG29, state->reg29);
			phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG4_CORE0,
				state->tr2g_config4_core[0]);
			phy_utils_write_radioreg(pi, RADIO_2057_TR2G_CONFIG4_CORE1,
				state->tr2g_config4_core[1]);
		}
	}

	if ((RADIOREV(pi->pubpi->radiorev) == 4) || (RADIOREV(pi->pubpi->radiorev) == 6)) {
		wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_tx_pu_MASK,
			1, 0x3, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	} else {
		wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_tx_pu_MASK,
		                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	}
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_pu_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_tx_buf_pu_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID2);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_dacbuf_pu_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID2);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_sel_txrx_MASK,
	                                  1, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rxgain_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rxif_pu_MASK,
	                                  1, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_dc_loop_pu_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_rx_buf_pu_MASK,
	                                  1, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID2);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_dc_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_rx_MASK,
	                                  1, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lpf_byp_tx_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_rxmx_pu_MASK,
	                                  1, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lna1_pu_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_lna2_pu_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);

	FOREACH_CORE(pi, core) {
		/* Clear ADC override */
		phy_utils_write_phyreg(pi, (core == PHY_CORE_0) ?
		              NPHY_AfectrlCore1 : NPHY_AfectrlCore2,
		              state->afectrl[core]);
		phy_utils_write_phyreg(pi, (core == PHY_CORE_0) ? NPHY_AfectrlOverride1 :
		              NPHY_AfectrlOverride2, state->afeoverride[core]);
	}

	/* Stop tone, restore CRS */
	wlc_phy_ipa_set_bbmult_nphy(pi, (state->mm >> 8) & 0xff, (state->mm & 0xff));

	wlc_phy_rfctrl_override_nphy_rev7(pi,
	                                  NPHY_REV7_RfctrlOverride_lpf_bw_ctl_MASK,
	                                  0, 0, 1, NPHY_REV7_RFCTRLOVERRIDE_ID1);
}

static void
wlc_phy_papd_smooth_nphy(phy_info_t *pi, uint8 core, uint32 winsz, uint32 start, uint32 end)
{
	uint32 *buf, *src, *dst, sz;

	PHY_CAL(("Smoothing papd cal on core: %d\n", core));

	sz = end - start + 1;
	ASSERT(end > start);
	ASSERT(end < NPHY_PAPD_EPS_TBL_SIZE);

	/* Allocate storage for both source & destination tables */
	if ((buf = MALLOC(pi->sh->osh, 2 * sizeof(uint32) * NPHY_PAPD_EPS_TBL_SIZE)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Setup source & destination pointers */
	src = buf;
	dst = buf + NPHY_PAPD_EPS_TBL_SIZE;

	/* Read original table */
	wlc_phy_table_read_nphy(pi, (core == PHY_CORE_0 ? NPHY_TBL_ID_EPSILONTBL0 :
	                         NPHY_TBL_ID_EPSILONTBL1),
	                    NPHY_PAPD_EPS_TBL_SIZE,
	                    0, 32, src);

	/* Average coeffs across window */
	do {
		uint32 win_start, win_end;
		int32 nAvr, eps_r, eps_i, eps_real, eps_imag;

		win_start = end - MIN(end, (winsz >> 1));
		win_end = MIN(NPHY_PAPD_EPS_TBL_SIZE - 1, end + (winsz >> 1));
		nAvr = win_end - win_start + 1;
		eps_real = 0;
		eps_imag = 0;

		do {
			phy_papdcal_decode_epsilon(src[win_end], &eps_r, &eps_i);
			eps_real += eps_r;
			eps_imag += eps_i;
		} while (win_end-- != win_start);

		eps_real /= nAvr;
		eps_imag /= nAvr;
		dst[end] = ((uint32)eps_imag << 13) | ((uint32)eps_real & 0x1fff);
	} while (end-- != start);

	/* Write updated table */
	wlc_phy_table_write_nphy(pi, (core == PHY_CORE_0) ? NPHY_TBL_ID_EPSILONTBL0 :
		NPHY_TBL_ID_EPSILONTBL1, sz, start, 32, dst);

	/* Free allocated buffer */
	MFREE(pi->sh->osh, buf, 2 * sizeof(uint32) * NPHY_PAPD_EPS_TBL_SIZE);
}

/** output papd coeffs (epsilon table) to PHY_CAL trace */
static void
wlc_phy_papd_dump_eps_trace_nphy(phy_info_t *pi)
{
	uint core, j;
	uint32 eps_table[NPHY_PAPD_EPS_TBL_SIZE];
	int32 eps_re, eps_im;

	FOREACH_CORE(pi, core) {
		wlc_phy_table_read_nphy(pi, (core == PHY_CORE_0 ? NPHY_TBL_ID_EPSILONTBL0 :
			NPHY_TBL_ID_EPSILONTBL1),
			NPHY_PAPD_EPS_TBL_SIZE,
			0, 32, eps_table);

		PHY_CAL(("core %d\n", core));

		for (j = 0; j < NPHY_PAPD_EPS_TBL_SIZE; j++) {
			phy_papdcal_decode_epsilon(eps_table[j], &eps_re, &eps_im);
			PHY_CAL(("{%d %d} ", eps_re, eps_im));
		}
		PHY_CAL(("\n"));
	}
	PHY_CAL(("\n"));
}

/** changing the return format of papd. Now returning rqiest pwr for papd gainctrl */
static void
wlc_phy_papd_cal_nphy(phy_info_t *pi, nphy_ipa_txcalgains_t *txgains, phy_cal_mode_t cal_mode,
uint16 num_iter, uint8 core)
{
	uint16 startindex, stopindex, yrefindex;
	uint16 rad_gain = 0;
	uint8 off_core, m[NPHY_CORE_NUM];
	uint32 zero = 0;
	nphy_txgains_t target_gain;
	uint16 val; 	/* general purpose variable */

	off_core = (core == PHY_CORE_0) ? 1 : 0;

	PHY_CAL(("Running papd cal on core %d channel: %d, cal_mode: %d OFF core %d\n", core,
	           CHSPEC_CHANNEL(pi->radio_chanspec), cal_mode, off_core));

	ASSERT((cal_mode == CAL_FULL) || (cal_mode == CAL_GCTRL) || (cal_mode == CAL_SOFT));

	if (core == PHY_CORE_0) {
		/* override bt priority */
		wlc_btcx_override_enable(pi);
	}

	/* Read the current target gain to obtain the LPF, gm, and intPA gain codes in the
	 * 2G band and the LPF, gm, PAD, and intPA gain codes in the 5G band
	 */
	wlc_phy_get_tx_gain_nphy(pi, &target_gain);

	/* Obtain the radio gain at which PAPD cal is to be performed */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		rad_gain = ((target_gain.txlpf[core] << 15) |
		            (target_gain.txgm[core] << 12) |
		            (target_gain.pga[core] << 8) |
		            (txgains->gains.pad[core] << 3) |
		            (target_gain.ipa[core]));
	} else {
		rad_gain = ((target_gain.txlpf[core] << 15) |
		            (target_gain.txgm[core] << 12) |
		            (txgains->gains.pga[core] << 8) |
		            (target_gain.pad[core] << 3) |
		            (target_gain.ipa[core]));
	}
	wlc_phy_rfctrl_override_1tomany_nphy(pi,
		NPHY_REV7_RfctrlOverride_cmd_txgain, rad_gain, (1 << core), 0);

	/* Force bbmult */

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if ((RADIOID(pi->pubpi->radioid) == BCM2057_ID) &&
			((RADIOREV(pi->pubpi->radiorev) <= 4) ||
			(RADIOREV(pi->pubpi->radiorev) == 6))) {
			/* 43226a0/1 and 6362a0 */
			m[core] = IS40MHZ(pi) ? 60 : 79;
		} else {
			/* 5357a0 and 43236a0 and 6362b0 */
			m[core] = IS40MHZ(pi) ? 45 : 64;
		}
	} else {
		m[core] = IS40MHZ(pi) ? 75 : 107;
	}

	m[off_core] = 0;
	wlc_phy_ipa_set_bbmult_nphy(pi, m[0], m[1]);

	stopindex = 63;

	/* Sets the beginning of non-zero portion of PAPD coefficients table */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (RADIOREV(pi->pubpi->radiorev) == 3) {
			startindex = 35;
			yrefindex = 35;
		} else if ((RADIOREV(pi->pubpi->radiorev) == 4) ||
			(RADIOREV(pi->pubpi->radiorev) == 6)) {
			startindex = 30;
			yrefindex = 30;
		} else {
			startindex = 25;
			yrefindex = 25;
		}
	} else {
		if ((RADIOREV(pi->pubpi->radiorev) == 5) ||
			(RADIOREV(pi->pubpi->radiorev) == 7) ||
			(RADIOREV(pi->pubpi->radiorev) == 8) ||
			(RADIOREV(pi->pubpi->radiorev) == 13) ||
			(RADIOREV(pi->pubpi->radiorev) == 14))
		{
			yrefindex = 25;
			startindex = 25;
		} else {
			startindex = 35;
			yrefindex = 35;
		}
	}
	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
		phy_utils_write_phyreg(pi, NPHY_PapdCore2Cal, core);
	}

	if (cal_mode == CAL_GCTRL) {
		if (((RADIOREV(pi->pubpi->radiorev) == 5) ||
			(RADIOREV(pi->pubpi->radiorev) == 8) ||
			(RADIOREV(pi->pubpi->radiorev) == 12) || /* BCM63268 */
			(RADIOREV(pi->pubpi->radiorev) == 13) ||
			(RADIOREV(pi->pubpi->radiorev) == 14)) &&
			(CHSPEC_IS2G(pi->radio_chanspec)))
		{
			startindex = 55;
		} else {
			startindex = 63;
		}

	} else if ((cal_mode != CAL_FULL) && (cal_mode != CAL_SOFT)) {
		/* undefined mode */
		startindex = 35;
		yrefindex = 35;
	}

	/* setup PAPD cal engine */
	PHY_REG_MOD_CORE(pi, NPHY, core, PapdEnable, compEnable, 1);
	PHY_REG_MOD_CORE(pi, NPHY, off_core, PapdEnable, compEnable, 0);

	PHY_REG_MOD_CORE(pi, NPHY_REV6, core, PapdCalShifts, calEnable, 1);
	PHY_REG_MOD_CORE(pi, NPHY_REV6, off_core, PapdCalShifts, calEnable, 0);

	/* setup LMS convergence related params */

	if (	/* ver = 1 / rev = 7 is 43236B0 */
		/* ver = 2 / rev = 7 is 43236B1 */
		(RADIOREV(pi->pubpi->radiorev) == 7) &&
		((RADIOVER(pi->pubpi->radiover) == 1) ||
		(RADIOVER(pi->pubpi->radiover) == 2))) {
		/* standard pulsed papd cal */
		if (CHSPEC_IS40(pi->radio_chanspec) == 1) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, PapdCalSettle, 0x40)
				PHY_REG_WRITE_ENTRY(NPHY, PapdCalCorrelate, 0x40)
			PHY_REG_LIST_EXECUTE(pi);

			PHY_REG_MOD_CORE(pi, NPHY, core, PapdCalShifts, CorrShift, 0x1);
			val = 0x3fe;
		} else {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, PapdCalSettle, 0x20)
				PHY_REG_WRITE_ENTRY(NPHY, PapdCalCorrelate, 0x20)
			PHY_REG_LIST_EXECUTE(pi);

			PHY_REG_MOD_CORE(pi, NPHY, core, PapdCalShifts, CorrShift, 0x0);
			val = 0x1ff;
		}
	} else if (/* ver = 2 / rev = 2 is 43242A1 */
		(RADIOVER(pi->pubpi->radiover) == 2) &&
		(RADIOREV(pi->pubpi->radiorev) == 2)) {
		/* fast pulsed papd cal */
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(NPHY, PapdCalSettle, 0x40)
			PHY_REG_WRITE_ENTRY(NPHY, PapdCalCorrelate, 0x80)
		PHY_REG_LIST_EXECUTE(pi);

		PHY_REG_MOD_CORE(pi, NPHY, core, PapdCalShifts, CorrShift, 0x3);

		if (CHSPEC_IS40(pi->radio_chanspec) == 1) {
			val = 0x3c0;
		} else {
			val = 0x180;
		}
		num_iter = 0x10;
	} else {
		/* standard continuous papd cal */
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(NPHY, PapdCalSettle, 0x80)
			PHY_REG_WRITE_ENTRY(NPHY, PapdCalCorrelate, 0x100)
		PHY_REG_LIST_EXECUTE(pi);

		PHY_REG_MOD_CORE(pi, NPHY, core, PapdCalShifts, CorrShift, 0x3);
		val = 0;
	}
	phy_utils_write_phyreg(pi, NPHY_PapdIpaOffCorr, val);

	PHY_REG_MOD_CORE(pi, NPHY, core, PapdCalShifts, lambdaI, 11);
	PHY_REG_MOD_CORE(pi, NPHY, core, PapdCalShifts, lambdaQ, 11);
	phy_utils_write_phyreg(pi, NPHY_PapdEpsilonUpdateIterations, num_iter);

	/* setup iter, Yref, start and end address */
	PHY_REG_MOD(pi, NPHY, PapdCalYrefEpsilon, YrefAddr, yrefindex);
	PHY_REG_MOD(pi, NPHY, PapdCalAddress, StartAddr, startindex);
	PHY_REG_MOD(pi, NPHY, PapdCalAddress, EndAddr, stopindex);

	/* ovr iPA to be ON for core being cal'd, and OFF for alternate core */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK, 1,
		((core == 0) ? 1 : 2), 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK, 0,
		((core == 0) ? 2 : 1), 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	/* start PAPD calibration */
	phy_utils_write_phyreg(pi, NPHY_PapdCalStart, 1);
	SPINWAIT(phy_utils_read_phyreg(pi, NPHY_PapdCalStart), NPHY_SPINWAIT_PAPDCAL);
	ASSERT(!(phy_utils_read_phyreg(pi, NPHY_PapdCalStart) & 1));

	/* clear iPA ovr */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_intpa_pu_MASK, 0, 0x3,
		0, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	wlc_phy_table_write_nphy(pi, (core == PHY_CORE_0) ? NPHY_TBL_ID_EPSILONTBL0 :
	                     NPHY_TBL_ID_EPSILONTBL1, 1, yrefindex, 32, &zero);

	/* smooth PAPD eps curve */
	if (cal_mode != CAL_GCTRL) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			wlc_phy_papd_smooth_nphy(pi, core, 5, 0, 35);
		}
		if (PHY_CAL_ON())
			wlc_phy_papd_dump_eps_trace_nphy(pi);
	}
	/* Remove the gain override */
	wlc_phy_rfctrl_override_1tomany_nphy(pi,
		NPHY_REV7_RfctrlOverride_cmd_txgain,
		rad_gain, (1 << core), 1);

	if (core == PHY_CORE_0) {
		/* remove override */
		wlc_phy_btcx_override_disable(pi);
	}
}

#define LPBK_TBL_DEPTH_2G 6
#define LPBK_TBL_DEPTH_5G 7

/**
 * For NREVS<7, the function returns gain index in tx power control table for which papd cal is done
 * For NREVs>=7, the function returns the PAD or PGA gain index for which papd cal is done
 *               If both the PGA and the PAD are varied in the 5G band, a better method needs
 *               to be figured out.
 * The reason for using a different method for selecting the radio gain during gain control for
 * NREVs>=7 is because the 2057 radio does not have uniform gain deltas between different PGA
 * (or PAD) gain codes. On the other hand, the 2056 radio more or less has a constant 2 dB
 * step size between different PGA gain codes that makes it easy to implement PAPD gain ctrl
 * using the tx pwr index.
*/
static uint8
wlc_phy_papd_cal_gctrl_nphy(phy_info_t *pi, uint8 start_gain, uint8 core,
	uint8 *mixgain_ovr, uint8 *atten_ovr)
{
	int gain_step;
	int max_iter;
	bool clipping;
	nphy_ipa_txcalgains_t txgains;
	bool prev_clipping = FALSE;
	bool first_pass = TRUE;
	int32 eps_real, eps_imag;
	uint32 val;
	int j;
	bool done = FALSE;
	int gain_index;
	uint8 gain_min = 0;
	uint8 gain_max;
	uint8* gain_codes_used = NULL;
	int32 epsmax = 4095;
	uint16 freq;
	uint16 num_iter = 16;
	uint8 lpbk_settings_table[6][2] = {{0xc0, 0x00}, {0xc0, 0x01}, {0xc0, 0x05},
		{0x80, 0x05}, {0x40, 0x05}, {0x00, 0x05}}; /* {mix, atten}  */
	uint8 lpbk_settings_idx = 2; /* starting index for mix/atten */

	txgains.useindex = TRUE;
	gain_index = start_gain;

	/* start with a default value */
	*mixgain_ovr = lpbk_settings_table[lpbk_settings_idx][0];
	*atten_ovr = lpbk_settings_table[lpbk_settings_idx][1];

	/* For NREVs>=7, implement PAPD gain control in the same way as in Tcl, i.e.,
	 * by scanning through the radio gains directly instead of searching the tx
	 * power indices. This approach is easier to use with the 2057 radio, which
	 * had irregular gain steps in both the PGA and the PAD gain codes
	 */

	max_iter = 20;

	gain_step = 1;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if ((RADIOREV(pi->pubpi->radiorev) == 5) ||
		    (RADIOREV(pi->pubpi->radiorev) == 13) ||
		    (RADIOREV(pi->pubpi->radiorev) == 14)) {
			/* 5357b0 and 53572a0 */
			gain_codes_used = pad_gain_codes_used_2057rev5;
			gain_min = sizeof(pad_gain_codes_used_2057rev5)/
			        sizeof(pad_gain_codes_used_2057rev5[0]) - 1;

		} else if ((RADIOREV(pi->pubpi->radiorev) == 7) ||
			(RADIOREV(pi->pubpi->radiorev) == 8))
		{
		  /* 43236a0 (and 6362b0 until we separate it) */
		  if (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID) {
			gain_codes_used = pad_gain_codes_used_2057rev7_sul;
			gain_min = sizeof(pad_gain_codes_used_2057rev7_sul)/
			  sizeof(pad_gain_codes_used_2057rev7_sul[0]) - 1;
		  } else {
			gain_codes_used = pad_gain_codes_used_2057rev7;
			gain_min = sizeof(pad_gain_codes_used_2057rev7)/
			  sizeof(pad_gain_codes_used_2057rev7[0]) - 1;
		  }
		} else if (RADIOREV(pi->pubpi->radiorev) == 12)
		{
		  /* BCM63268 */
			gain_codes_used = pad_gain_codes_used_2057rev12;
			gain_min = sizeof(pad_gain_codes_used_2057rev12)/
			  sizeof(pad_gain_codes_used_2057rev12[0]) - 1;
		} else {
			/* 43226a[01], 6362a0 */
			gain_codes_used = pad_all_gain_codes_2057;
			gain_min = sizeof(pad_all_gain_codes_2057)/
			        sizeof(pad_all_gain_codes_2057[0]) - 1;
		}

	} else {
		/* for now, all REV7+ chips use all PGA gain codes */
		gain_codes_used = pga_all_gain_codes_2057;
		gain_min = sizeof(pga_all_gain_codes_2057)/ sizeof(pga_all_gain_codes_2057[0]) - 1;
	}

	/* REV7+ interpretation of gain_[min,max] is reversed to prev REVs
	 *
	 * gain_codes_used is arranged so that:
	 *         gain_codes_used[0]   is the gain code that gives the MAX gain in dB
	 *         gain_codes_used[end] is the gain code that gives the MIN gain in dB
	 *
	 * Hence:
	 * gain_min = index that corresponds to the MIN gain in gain_codes_used
	 * gain_max = index that corresponds to the MAX gain in gain_codes_used
	 */

	/* this epsmax code below is useful to limit the radio gain during
	   papd cal.  For example, to lower gain by ~3dB use epsmax = 1700
	   (assuming 12 fractional bits of epsilon).  The math is
	   20*log10(2/(1+1700/4096))= 3 dB.  We are not talking advantage of it below,
	   but it is good to have, just in case we have FCC violations.
	*/

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
	  epsmax = 4095;
	} else {
	  freq = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	  if ((freq >= 5150) && (freq <= 5350)) {
		epsmax = 4095;
	  }
	}

	gain_max = 0;

	for (j = 0; j < max_iter; j++) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			txgains.gains.pad[core] = (uint16)gain_codes_used[gain_index];
		} else {
			txgains.gains.pga[core] = (uint16)gain_codes_used[gain_index];
		}

		/* Do the cal and read iqest values */
		wlc_phy_papd_cal_nphy(pi, &txgains, CAL_GCTRL, num_iter, core);

		/* read last entry in epsilon table */
		wlc_phy_table_read_nphy(pi, (core == PHY_CORE_0 ? NPHY_TBL_ID_EPSILONTBL0 :
		                         NPHY_TBL_ID_EPSILONTBL1), 1, 63, 32, &val);

		phy_papdcal_decode_epsilon(val, &eps_real, &eps_imag);

		PHY_CAL(("papd_gctrl: gain: %d epsilon: %d + j*%d\n",
		         gain_codes_used[gain_index], eps_real, eps_imag));

		clipping = ((eps_real >= epsmax) || (eps_real <= -1 * (epsmax + 1)) ||
		            (eps_imag >= epsmax) || (eps_imag <= -1 * (epsmax + 1)));

		if (!first_pass && (clipping != prev_clipping)) {
			if (!clipping) {
				if (!((((RADIOREV(pi->pubpi->radiorev) == 5) &&
				        (RADIOVER(pi->pubpi->radiover) == 0x0)) ||
				       (RADIOREV(pi->pubpi->radiorev) == 8)) &&
				      (CHSPEC_IS2G(pi->radio_chanspec))))
					gain_index -= (uint8)gain_step;
			}
			done = TRUE;
			break;
		}

		if (clipping)
			gain_index += (uint8)gain_step;
		else
			gain_index -= (uint8)gain_step;

		/* limit index to gain table range */
		if ((gain_index < gain_max) || (gain_index > gain_min)) {
			if (gain_index < gain_max) {
				gain_index = gain_max;
			} else {
				gain_index = gain_min;
			}
			done = TRUE;
			break;
		}
		first_pass = FALSE;
		prev_clipping = clipping;
	}

	if ((gain_index < gain_max) || (gain_index > gain_min)) {
		if (gain_index < gain_max) {
			gain_index = gain_max;
		} else {
			gain_index = gain_min;
		}
	}

	if (!done) {
		PHY_ERROR(("Warning PAPD gain control failed to converge in %d attempts\n",
		           max_iter));
	}

	PHY_CAL(("###### core%d papd gctrl settled to gain %d, index %d in %d attempts\n",
		core, gain_codes_used[gain_index], gain_index, j));

	return (uint8) gain_codes_used[gain_index];

}

/** Run PAPD calibration for NPHY */
static void
wlc_phy_txpwr_papd_cal_run(phy_info_t *pi, bool full_cal, uint8 core_from, uint8 core_to)
{
	phy_info_nphy_t *pi_nphy = NULL;
	nphy_ipa_txcalgains_t txgains[NPHY_CORE_NUM];
	nphy_papd_restore_state restore_state;
	bool suspend;
	uint8 tx_pwr_ctrl_state;
	uint8 core;
	int16 yref_i, yref_q, offset, pgag, padg;
	uint16 nphy_saved_bbconf;
	int16 bbmult_offset, pgagain_offset, padgain_offset;
	uint8 mixgain_ovr = 0;
	uint8 atten_ovr = 0;

	pgagain_offset = 0;
	padgain_offset = 0;
	pgag = 0;
	padg = 0;
	offset = 0;
	pi_nphy = pi->u.pi_nphy;

	if ((pi_nphy->nphy_papd_skip == 1) && !phy_papdcal_is_wfd_phy_ll_enable(pi->papdcali))
		/* For Optimizations 43241/WFD, PAPD is disabled by the caller */
		return;

	/* skip cal if phy is muted */
	if (PHY_MUTED(pi))
		return;

	PHY_CAL(("Entering PAPD cal loop\n"));

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Disable CRS */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	pi_nphy->nphy_force_papd_cal = FALSE;

	for (core = core_from; core <= core_to; core ++) {
		pi_nphy->nphy_papd_tx_gain_at_last_cal[core] =
			wlc_phy_txpwr_idx_cur_get_nphy(pi, core);
	}

	PHY_CAL(("PAPD cal at gain index (%d, %d)\n",
	           pi_nphy->nphy_papd_tx_gain_at_last_cal[0],
	           pi_nphy->nphy_papd_tx_gain_at_last_cal[1]));

	pi_nphy->nphy_papd_last_cal = pi->sh->now;
	pi_nphy->nphy_papd_recal_counter++;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	/* Save tx power control and gain state */
	tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
	wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_SCALARTBL0, 64, 0, 32, nphy_papd_scaltbl);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_SCALARTBL1, 64, 0, 32, nphy_papd_scaltbl);
	nphy_saved_bbconf = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_mod_phyreg(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK, 0);

	for (core = core_from; core <= core_to; core ++) {
		int32 i, val = 0;
		for (i = 0; i < 64; i ++) {
			wlc_phy_table_write_nphy(pi,
				((core == PHY_CORE_0) ? NPHY_TBL_ID_EPSILONTBL0 :
				NPHY_TBL_ID_EPSILONTBL1), 1, i, 32, &val);
		}
	}

	/* always use default tx-filt to calibrate */
	wlc_phy_ipa_set_cal_tx_digi_filts_nphy(pi);

	bzero(&restore_state, sizeof(restore_state));
	restore_state.mm = wlc_phy_ipa_get_bbmult_nphy(pi);
	for (core = core_from; core <= core_to; core ++) {
		wlc_phy_papd_cal_setup_nphy(pi, &restore_state, core);

		/* calibrate and override the txgain */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
				(RADIOREV(pi->pubpi->radiorev) == 4) ||
				(RADIOREV(pi->pubpi->radiorev) == 6)) {
				/* no gctrl for 43226a[01] or 6362a0 */
				pi_nphy->nphy_papd_cal_gain_index[core] = 23;

			} else if (RADIOREV(pi->pubpi->radiorev) == 5) {
				if (RADIOVER(pi->pubpi->radiover) == 0x0) {
					/* 5357a0: start at max PAD gain */
					pi_nphy->nphy_papd_cal_gain_index[core] = 0;
				} else {
					/* 5357b0 */
					pi_nphy->nphy_papd_cal_gain_index[core] = 8;
				}
				pi_nphy->nphy_papd_cal_gain_index[core] =
				        wlc_phy_papd_cal_gctrl_nphy(
						pi, pi_nphy->nphy_papd_cal_gain_index[core],
						core, &mixgain_ovr, &atten_ovr);

			} else if (RADIOREV(pi->pubpi->radiorev) == 7) {
				/* 43236: start at middle PAD gain */
				pi_nphy->nphy_papd_cal_gain_index[core] = 6;

				pi_nphy->nphy_papd_cal_gain_index[core] =
				        wlc_phy_papd_cal_gctrl_nphy(
						pi, pi_nphy->nphy_papd_cal_gain_index[core],
						core, &mixgain_ovr, &atten_ovr);

			} else if (RADIOREV(pi->pubpi->radiorev) == 8) {
				/* 43236: start at max PAD gain */
				pi_nphy->nphy_papd_cal_gain_index[core] = 0;
				pi_nphy->nphy_papd_cal_gain_index[core] =
				        wlc_phy_papd_cal_gctrl_nphy(
						pi, pi_nphy->nphy_papd_cal_gain_index[core],
						core, &mixgain_ovr, &atten_ovr);
			} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
				/* BCM63268 */
				pi_nphy->nphy_papd_cal_gain_index[core] = 0;
				pi_nphy->nphy_papd_cal_gain_index[core] =
				        wlc_phy_papd_cal_gctrl_nphy(
						pi, pi_nphy->nphy_papd_cal_gain_index[core],
						core, &mixgain_ovr, &atten_ovr);
			} else if ((RADIOREV(pi->pubpi->radiorev) == 13) ||
				(RADIOREV(pi->pubpi->radiorev) == 14)) {
				/* 53572a0 */
				pi_nphy->nphy_papd_cal_gain_index[core] = 0;
				pi_nphy->nphy_papd_cal_gain_index[core] =
				        wlc_phy_papd_cal_gctrl_nphy(
						pi, pi_nphy->nphy_papd_cal_gain_index[core],
						core, &mixgain_ovr, &atten_ovr);
			} else {
				PHY_ERROR(("Unsupported radio rev %d\n",
				           RADIOREV(pi->pubpi->radiorev)));
				ASSERT(0);
			}

			txgains[core].gains.pad[core] = pi_nphy->nphy_papd_cal_gain_index[core];
		} else {
			if (RADIOREV(pi->pubpi->radiorev) == 7) {
				/* 43236: start at mid PGA gain */
				pi_nphy->nphy_papd_cal_gain_index[core] = 5;
			} else {
				pi_nphy->nphy_papd_cal_gain_index[core] = 0;
			}
			pi_nphy->nphy_papd_cal_gain_index[core] =
				wlc_phy_papd_cal_gctrl_nphy(
					pi, pi_nphy->nphy_papd_cal_gain_index[core],
					core, &mixgain_ovr, &atten_ovr);
			txgains[core].gains.pga[core] = pi_nphy->nphy_papd_cal_gain_index[core];
		}

		switch (pi_nphy->nphy_papd_cal_type) {
		case 0:
			wlc_phy_papd_cal_nphy(pi, &txgains[core], CAL_FULL, 32, core);
			break;
		case 1:
			wlc_phy_papd_cal_nphy(pi, &txgains[core], CAL_SOFT, 32, core);
			break;
		}

		/* For each core, end with a PAPD cal cleanup. This way, it can be ensured that
		 * the register settings are correctly set and restored for each core, and
		 * the driver code will also be in sync with what is done in Tcl.
		 */
		/* room for acceleration: both cores cleaned up at every call */
		wlc_phy_papd_cal_cleanup_nphy(pi, &restore_state);
	}

	for (core = core_from; core <= core_to; core ++) {
		int eps_offset = 0;		/* artificial shift in papd lut */

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (RADIOREV(pi->pubpi->radiorev) == 3) {
				/* 6362a0 */
				eps_offset = -2;
			} else if (RADIOREV(pi->pubpi->radiorev) == 5) {
				if (RADIOVER(pi->pubpi->radiover) == 0x0) {
					/* 5357a0 */
					eps_offset = 3;
				} else {
					/* 5357b0 */
					eps_offset = (core == 0) ? 2 : 4;
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 7) {
			  if (RADIOVER(pi->pubpi->radiover) > 0) {
				if (RADIOVER(pi->pubpi->radiover) == 2)
				  eps_offset = -2;
				else
				  eps_offset = 0;
			  } else {
					/* 43236a0 */
					eps_offset = -1;
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 13) {
				/* 53572a0 */
				eps_offset = 2;
			} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
				/* 43217 */
				if (CHSPEC_IS40(pi->radio_chanspec) == 1) {
					if (core == 0) {
						eps_offset = -3;
					} else {
						eps_offset = -1;
					}
				} else
					if (RADIOVER(pi->pubpi->radiover) == 0) {
						if (core == 0) {
							eps_offset = -1;
						} else {
							eps_offset = 1;
						}
					} else {
						eps_offset = 1;
					}
			} else {
				/* 6362b0 */
				eps_offset = 2;
			}

		} else {
			if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
				(RADIOVER(pi->pubpi->radiover) > 0)) {
				eps_offset = (core == 0) ? 1 : 2;
			} else {
				eps_offset = 2;
			}
		}

		/* Configure epsilon offset based on calibration txgain
		 * NOTE: The calculation of epsilon offset is slightly different from
		 * the calculation done in Tcl. As a result, some of the epsilon
		 * offset values can differ by a tick from those used in Tcl.
		 * A one tick different should not make a difference to performance,
		 * based on 2056-based intPA designs, but this note is inserted as a
		 * caution in case this difference does cause different performance
		 * between Tcl and driver.
		 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			padg = txgains[core].gains.pad[core];
			bbmult_offset = 0;

			if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
			    (RADIOREV(pi->pubpi->radiorev) == 4) ||
			    (RADIOREV(pi->pubpi->radiorev) == 6)) {
				padgain_offset =
				        -(nphy_papd_padgain_dlt_2g_2057rev3n4[padg]
				          + 1)/2;
				bbmult_offset = -1;
			}  else if (RADIOREV(pi->pubpi->radiorev) == 5) {
				if (pi->pubpi->radiover == 1) {
					padgain_offset =
					-(nphy_papd_padgain_dlt_2g_2057rev5[padg]
					+ 1)/2;
				} else {
					padgain_offset =
					-(nphy_papd_padgain_dlt_2g_2057rev5v1[padg]
					+ 1)/2;
				}
			} else if ((RADIOREV(pi->pubpi->radiorev) == 7) ||
			           (RADIOREV(pi->pubpi->radiorev) == 8)) {

				/* 43236B1 */
				if (RADIOVER(pi->pubpi->radiover) == 2) {
				padgain_offset =
				-(nphy_papd_padgain_dlt_2g_2057rev7_ver2[padg]
				+ 1)/2;

				/* 43236A0, 43236B0, 6362B0 */
				} else {
					padgain_offset =
					-(nphy_papd_padgain_dlt_2g_2057rev7[padg]
					          + 1)/2;
				}
			}  else if (RADIOREV(pi->pubpi->radiorev) == 12) {
				/* BCM63268 */
				padgain_offset =
					-(nphy_papd_padgain_dlt_2g_2057rev12
					[padg] + 1)/2;
			}  else if (RADIOREV(pi->pubpi->radiorev) == 13) {
				/* 53572A0 */
				padgain_offset =
				        -(nphy_papd_padgain_dlt_2g_2057rev13[padg]
				          + 1)/2;
			}  else if (RADIOREV(pi->pubpi->radiorev) == 14) {
				/* 43217 */
				padgain_offset =
				        -(nphy_papd_padgain_dlt_2g_2057rev14[padg]
				          + 1)/2;
			} else {
				PHY_ERROR(("Unsupported radio rev %d\n",
				           RADIOREV(pi->pubpi->radiorev)));
				ASSERT(0);
			}
		} else {
			pgag = txgains[core].gains.pga[core];
			if ((RADIOREV(pi->pubpi->radiorev) == 3) ||
			    (RADIOREV(pi->pubpi->radiorev) == 4) ||
			    (RADIOREV(pi->pubpi->radiorev) == 6)) {
				pgagain_offset = -(nphy_papd_pgagain_dlt_5g_2057[pgag]
				                   + 1)/2;
			} else if ((RADIOREV(pi->pubpi->radiorev) == 7) ||
				(RADIOREV(pi->pubpi->radiorev) == 8) ||
				(RADIOREV(pi->pubpi->radiorev) == 12))
			{
				pgagain_offset = -(nphy_papd_pgagain_dlt_5g_2057rev7[pgag]
				                   + 1)/2;
			} else {
					PHY_ERROR(("Unsupported radio rev %d\n",
					           RADIOREV(pi->pubpi->radiorev)));
					ASSERT(0);
			}

			/* compensate for using bbmult other than 64 */
			bbmult_offset = -9;
		}

		offset = -60 + 27 + eps_offset + bbmult_offset;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			offset += padgain_offset;
		} else {
			offset += pgagain_offset;
		}

		PHY_REG_MOD_CORE(pi, NPHY, core, EpsilonTableAdjust, epsilonOffset, offset);

		/* save this value, in case some other pro re-init phyregs */
		pi_nphy->nphy_papd_epsilon_offset[core] = offset;

		/* dump some status Yref */
		yref_i = (int16) PHY_REG_READ_CORE(pi, NPHY, core, PapdCalYref_I);
		yref_q = (int16) PHY_REG_READ_CORE(pi, NPHY, core, PapdCalYref_Q);

		if (CHSPEC_IS5G(pi->radio_chanspec))  {
			PHY_CAL(("PAPD cal Yref core %d: (%d, %d), pga_gain: %d,"
			         "total offset %d\n", core, yref_i, yref_q,
			         pgag, offset));
		} else {
			PHY_CAL(("PAPD  cal Yref core %d: (%d, %d), pad_gain: %d,"
			         "tot offset %d\n", core, yref_i, yref_q, padg,
			         offset));
		}
		BCM_REFERENCE(yref_i);
		BCM_REFERENCE(yref_q);
	}

	/* leave PAPD comp ON */
	PHY_REG_LIST_START
		PHY_REG_MOD_CORE_ENTRY(NPHY, 0, PapdEnable, compEnable, NPHY_PAPD_COMP_ON)
		PHY_REG_MOD_CORE_ENTRY(NPHY, 1, PapdEnable, compEnable, NPHY_PAPD_COMP_ON)
		PHY_REG_MOD_CORE_ENTRY(NPHY_REV6, 0, PapdCalShifts, calEnable,  0)
		PHY_REG_MOD_CORE_ENTRY(NPHY_REV6, 1, PapdCalShifts, calEnable,  0)
	PHY_REG_LIST_EXECUTE(pi);

	pi_nphy->nphy_papdcomp = NPHY_PAPD_COMP_ON;

	phy_utils_write_phyreg(pi, NPHY_BBConfig, nphy_saved_bbconf);

	/* 4324 tcl is using different filter settings for papd than normal tx in 2G 40Mhz -
	 * transmit mode. Restoring the tx digital filter settings back to normal tx here
	 */
	if (NREV_LE(pi->pubpi->phy_rev, LCNXN_BASEREV + 4)) {
		wlc_phy_ipa_set_tx_digi_filts_nphy(pi);
	}

	/* Restore tx power and reenable tx power control; restore m0/m1!!! */
	wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
	if (tx_pwr_ctrl_state == PHY_TPC_HW_OFF) {
		wlc_phy_txpwr_index_nphy(pi, (1 << 0),
		                         (int8) (pi_nphy->nphy_txpwrindex[0].index_internal),
		                         FALSE);
		wlc_phy_txpwr_index_nphy(pi, (1 << 1),
		                         (int8) (pi_nphy->nphy_txpwrindex[1].index_internal),
		                         FALSE);
	}

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/** use fixed power */
void
wlc_phy_txpwr_fixpower_nphy(phy_info_t *pi)
{
	uint8 core;
	uint32 txgain = 0;
	uint16 rad_gain, dac_gain, bbmult, m1m2;
	uint8 txpi[NPHY_CORE_NUM] = { 91, 91 };
	uint8 chan_freq_range;
	int32 rfpwr_offset;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* initially force txgain in case txpwrctrl is disabled */
	ASSERT(pi->nphy_txpwrctrl == PHY_TPC_HW_OFF);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	if (pi->sh->sromrev < 4) {
		txpi[0] = txpi[1] = 72;
	} else {
		/* pick power index from SROM based on current channel */
		chan_freq_range = wlc_phy_get_chan_freq_range_nphy(pi, 0);
		FOREACH_CORE(pi, core) {
			switch (chan_freq_range) {
			case WL_CHAN_FREQ_RANGE_2G:
				txpi[core] = pi_nphy->nphy_txpid2g[core];
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND0:
				txpi[core] =
					pi_nphy->nphy_txpid5g[core][WL_CHAN_FREQ_RANGE_5G_BAND0];
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND1:
				txpi[core] =
					pi_nphy->nphy_txpid5g[core][WL_CHAN_FREQ_RANGE_5G_BAND1];
				break;
			case WL_CHAN_FREQ_RANGE_5G_BAND2:
				txpi[core] =
					pi_nphy->nphy_txpid5g[core][WL_CHAN_FREQ_RANGE_5G_BAND2];
				break;
			default:
				PHY_TXPWR(("wl%d: %s: ch not found in defined frequency range.\n",
					pi->sh->unit, __FUNCTION__));
				break;
			}
		}
	}

	txpi[0] = txpi[1] = 30;

	pi_nphy->nphy_txpwrindex[PHY_CORE_0].index_internal = txpi[0];
	pi_nphy->nphy_txpwrindex[PHY_CORE_1].index_internal = txpi[1];
	pi_nphy->nphy_txpwrindex[PHY_CORE_0].index_internal_save = txpi[0];
	pi_nphy->nphy_txpwrindex[PHY_CORE_1].index_internal_save = txpi[1];

	FOREACH_CORE(pi, core) {
		if (PHY_IPA(pi)) {
			uint32 *tx_gaintbl = wlc_phy_get_ipa_gaintbl_nphy(pi);
			txgain = tx_gaintbl[txpi[core]];
		} else {
			uint32 *tx_gaintbl = wlc_phy_get_epa_gaintbl_nphy(pi);
			txgain = tx_gaintbl[txpi[core]];
		}

		PHY_TXPWR(("wl%d: %s: Fixed TX Gain for Core %d is 0x%08x.\n",
		          pi->sh->unit, __FUNCTION__, core, txgain));

		rad_gain = (txgain >> 16) & ((1<<(32-16+1))-1);
		dac_gain = (txgain >>  8) & ((1<<(10-8+1))-1);
		bbmult   = (txgain >>  0) & ((1<<(7 - 0+1))-1);

		/* DAC gain */
		phy_utils_mod_phyreg(pi, ((core == PHY_CORE_0) ? NPHY_AfectrlOverride1 :
			NPHY_AfectrlOverride2),
			NPHY_REV3_AfectrlOverride_dac_gain_MASK,
			NPHY_REV3_AfectrlOverride_dac_gain_MASK);
		phy_utils_write_phyreg(pi, (core == PHY_CORE_0) ? NPHY_AfectrlDacGain1 :
			NPHY_AfectrlDacGain2, dac_gain);

		/* radio gain */
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, (0x110 + core), 16, &rad_gain);

		/* bbmult, core 1 and 2 vals are packed into common reg */
		wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m1m2);
		m1m2 &= ((core == PHY_CORE_0) ? 0x00ff : 0xff00);
		m1m2 |= ((core == PHY_CORE_0) ? (bbmult << 8) : (bbmult << 0));
		wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m1m2);

		/* NPHY_IPA : force loading RFPWR OFFSET */
		if (PHY_IPA(pi)) {
			wlc_phy_table_read_nphy(pi,
				(core == PHY_CORE_0 ? NPHY_TBL_ID_CORE1TXPWRCTL :
				NPHY_TBL_ID_CORE2TXPWRCTL), 1,
				576 + txpi[core], 32, &rfpwr_offset);

			PHY_REG_MOD_CORE(pi, NPHY, core, PapdEnable, gainDacRfValue,
				(int16) rfpwr_offset);
			PHY_REG_MOD_CORE(pi, NPHY, core, PapdEnable, gainDacRfOverride, 1);
			PHY_CAL(("PAPD offset for core %d : 0x%04x %d\n", core,
			           (int16) rfpwr_offset, (int16) rfpwr_offset));
		}
	}

	/* ensure lutIndex is 0, since bphyScale table was built on that
	 * assumption. lutIndex == 0 -> scaling of 96/256 in 11b TX path.
	 */
	phy_utils_and_phyreg(pi, NPHY_BphyControl2, (uint16)(~NPHY_BphyControl2_lutIndex_MASK));

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static bool
BCMATTACHFN(wlc_phy_srom_read_nphy)(phy_info_t *pi)
{

	if (pi->sh->sromrev >= 9)
	{
		if (!wlc_phy_txpwr_srom9_read(pi))
			return FALSE;
	}
	else
	{
#ifndef WLC_DISABLE_SROM8
		if (!wlc_phy_txpwr_srom8_read(pi))
			return FALSE;
#endif /* Compiling out sromrev 8 code */

		/* to get 2.5+ 0.01*parefldovoltage */
		if (getvar(pi->vars, rstr_parefldovoltage) != NULL) {
			pi->ldo_voltage = (uint8)PHY_GETINTVAR(pi, rstr_parefldovoltage);
		} else {
			/* Default if not present in NVRAM */
			pi->ldo_voltage = 0x23;	 /* 2.85 V */
		}
	}

	return TRUE;

}

void
wlc_phy_txpower_recalc_target_nphy(phy_info_t *pi)
{
	uint8 tx_pwr_ctrl_state;

	if (NREV_GE(pi->pubpi->phy_rev, 8) && (pi->sh->sromrev >= 9)) {
		/* maps to wlc_update_txppr_offset() in wlc.c */
		wlapi_high_update_txppr_offset(pi->sh->physhim, pi->tx_power_offset);
	} else {
		/* older SROMs use HW table for Tx power offset */
		wlc_phy_txpwr_limit_to_tbl_nphy(pi);
	}

	wlc_phy_txpwrctrl_pwr_setup_nphy(pi);

	/* restore power contrl */
	tx_pwr_ctrl_state = pi->nphy_txpwrctrl;

	wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);

	/* Draconian Power Limits for Olympic Sulley */
	if (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)
		wlc_phy_set_target_tx_pwr_nphy(pi, wlc_phy_get_target_tx_pwr_nphy(pi));
}

/** use IQ & LO cal values from iqloCaltbl to populate per-tx_gain tx_pwr_ctrl table */
void
wlc_phy_txpwrctrl_coeff_setup_nphy(phy_info_t *pi)
{
	uint32 idx;
	uint16 iqloCalbuf[7];
	uint32 iqcomp, locomp, curr_locomp;
	int8   locomp_i, locomp_q;
	int8   curr_locomp_i, curr_locomp_q;
	uint32 tbl_id, tbl_len, tbl_offset;
	uint32 regval[128];
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: %s, RSSI LUT done\n", pi->sh->unit, __FUNCTION__));

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* read TX IQ/LO cal result back from iqloCaltbl
	 *    a0, b0,  are at offset 80, 81
	 *    a1, b1,  are at offset 82, 83
	 *    di0/dq0, are at offset 85
	 *    di1/dq1, are at offset 86
	 */
	wlc_phy_table_read_nphy(pi, 15, 7, 80, 16, iqloCalbuf);

	/* copy IQ cal result to IQ comp tables
	 *    (19:10 -- "a" coeff   9:0 -- "b" coeff)
	 *    entries in tx power table 0101xxxxx
	 *                              0110xxxxx
	 */
	tbl_len    = 128;
	tbl_offset = 320;
	for (tbl_id = NPHY_TBL_ID_CORE1TXPWRCTL; tbl_id <= NPHY_TBL_ID_CORE2TXPWRCTL; tbl_id++) {
		iqcomp = (tbl_id == 26) ?
		    (((uint32)(iqloCalbuf[0] & 0x3ff)) << 10) | (iqloCalbuf[1] & 0x3ff) :
		    (((uint32)(iqloCalbuf[2] & 0x3ff)) << 10) | (iqloCalbuf[3] & 0x3ff);

		for (idx = 0; idx < tbl_len; idx++) {
			regval[idx] = iqcomp;
		}
		wlc_phy_table_write_nphy(pi, tbl_id, tbl_len, tbl_offset, 32, regval);
	}

	/* copy LO cal result to LO comp tables
	 *    (15:8 -- I offset   7:0 -- Q offset)
	 *    entries in tx power table 0111xxxxx
	 *                              1000xxxxx
	 */
	tbl_offset = 448;

	/* Single Stage LO Cal: Normal case */
	for (tbl_id = NPHY_TBL_ID_CORE1TXPWRCTL;
		tbl_id <= NPHY_TBL_ID_CORE2TXPWRCTL; tbl_id++) {

		locomp = (uint32) ((tbl_id == 26) ? iqloCalbuf[5] : iqloCalbuf[6]);
		locomp_i = (int8) ((locomp >> 8) & 0xff);
		locomp_q = (int8) ((locomp) & 0xff);
		for (idx = 0; idx < tbl_len; idx++) {
			curr_locomp_i = locomp_i;
			curr_locomp_q = locomp_q;
			curr_locomp = (uint32) ((curr_locomp_i & 0xff) << 8);
			curr_locomp |= (uint32) (curr_locomp_q & 0xff);
			regval[idx] = curr_locomp;
		}
		wlc_phy_table_write_nphy(pi, tbl_id, tbl_len, tbl_offset, 32, regval);
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static void
wlc_phy_ipa_internal_tssi_setup_nphy(phy_info_t *pi, bool use_pad_tapoff)
{
	uint8 core;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if ((RADIOREV(pi->pubpi->radiorev) == 3) || (RADIOREV(pi->pubpi->radiorev) == 4) ||
	    (RADIOREV(pi->pubpi->radiorev) == 6)) {
		use_pad_tapoff = TRUE;
	}

	FOREACH_CORE(pi, core) {
		pi_nphy->tx_precal_tssi_radio_saveregs[core][0] =
		        (uint8) READ_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_VCM);
		pi_nphy->tx_precal_tssi_radio_saveregs[core][1] =
		        (uint8) READ_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX);
		pi_nphy->tx_precal_tssi_radio_saveregs[core][2] =
		        (uint8) READ_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG);
		if (RADIOREV(pi->pubpi->radiorev) != 5)
			pi_nphy->tx_precal_tssi_radio_saveregs[core][3] =
			        (uint8) READ_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MASTER, 0x5);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX, 0xe);

			if (RADIOREV(pi->pubpi->radiorev) != 5)
				WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA, 0);

			if (!use_pad_tapoff) {
				/* Use intPA tap off point
				 *
				 * Most 2057 radios have correct value as default,
				 * so no need to write tssig here.
				 *
				 */
				if (RADIOREV(pi->pubpi->radiorev) == 7) {
				  if (RADIOVER(pi->pubpi->radiover) == 1) {
				    WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG, 0x1);
				  } else {
				    WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG, 0x2);
				  }
				}
			} else {
				/* use PAD tap off point */
				WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG, 0x11);
			}
		} else {
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MASTER, 0x9);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX, 0xc);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG, 0);

			if (!use_pad_tapoff) {
				/* use intPA tap off point */
				WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA,
				                 0x21);
			} else {
				/* use PAD tap off point */
				WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA,
				                 0x11);
			}
		}
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_VCM_HG, 0);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, IQCAL_IDAC, 0);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_VCM, 0x3);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_MISC1, 0x0);
	}
}

static void
wlc_phy_internal_tssi_cleanup_nphy(phy_info_t *pi)
{
	uint8 core;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	FOREACH_CORE(pi, core) {
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSI_VCM,
		                 (uint16)pi_nphy->tx_precal_tssi_radio_saveregs[core][0]);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TX_SSI_MUX,
		                 (uint16)pi_nphy->tx_precal_tssi_radio_saveregs[core][1]);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIG,
		                 (uint16)pi_nphy->tx_precal_tssi_radio_saveregs[core][2]);
		if (RADIOREV(pi->pubpi->radiorev) != 5)
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, core, TSSIA,
				(uint16)pi_nphy->tx_precal_tssi_radio_saveregs[core][3]);
	}
}

static void
wlc_phy_external_tssi_setup_nphy(phy_info_t *pi)
{
	if (RADIOID(pi->pubpi->radioid) == BCM2057_ID) {
		uint8  core;

		FOREACH_CORE(pi, core) {
			phy_utils_write_radioreg(pi, ((core == PHY_CORE_0) ?
			                              RADIO_2057_TX0_TX_SSI_MUX :
			                              RADIO_2057_TX1_TX_SSI_MUX),
			                         0x11);
		}
		/* The external TSSI input pin in the 2057 radio is shared with the I/Q test pins.
		 * Hence, the IQ test pins need to be powered up so that the external TSSI
		 * signal can be input to the chip.
		 */
		phy_utils_write_radioreg(pi, RADIO_2057_IQTEST_SEL_PU, 0x1);
	}
}

/** measure NPHY idle TSSI by sending 0-magnitude tone */
void
wlc_phy_txpwrctrl_idle_tssi_nphy(phy_info_t *pi)
{
	int32 rssi_buf[4];
	int32 int_val;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* BMAC_NOTE: Why is the idle tss skipped? Is it avoiding doing a cal
	 * on a temporary channel (this is why I would think it would check
	 * SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi)), or is it preventing
	 * any energy emission (this is why I would think it would check
	 * PHY_MUTED()), or both?
	 */
	if (SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || PHY_MUTED(pi))
		/* skip idle tssi cal */
		return;

	if (PHY_IPA(pi)) {
		wlc_phy_ipa_internal_tssi_setup_nphy(pi, FALSE);
	} else {
		/* For NPHY_REVs <= 6, external tssi setup is done in wlc_phy_rssisel_nphy() */
		wlc_phy_external_tssi_setup_nphy(pi);
	}

	/* Set the Tx gain to 0, so that LO leakage will not affect the IDLE Tssi */
	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
	                                  0, 0x3, 0, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	wlc_phy_stopplayback_nphy(pi);

	wlc_phy_tx_tone_nphy(pi, 4000, 0, 0, 0, FALSE);
	OSL_DELAY(20);

	int_val = wlc_phy_poll_rssi_nphy(pi, (uint8)NPHY_RSSI_SEL_TSSI_2G, rssi_buf, 1);

	wlc_phy_stopplayback_nphy(pi);

	wlc_phy_rssisel_nphy(pi, RADIO_MIMO_CORESEL_OFF, 0);

	wlc_phy_rfctrl_override_nphy_rev7(pi, NPHY_REV7_RfctrlOverride_txgain_MASK,
	                                  0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);

	/* For NPHY rev >= 3, the TSSI value is available only on the I component of the
	 * AUX ADC output. The Q component should be ignored
	 */
	pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].idle_tssi_2g = (uint8) ((int_val >> 24) & 0xff);
	pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].idle_tssi_5g = (uint8) ((int_val >> 24) & 0xff);

	pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].idle_tssi_2g = (uint8) ((int_val >> 8) & 0xff);
	pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].idle_tssi_5g = (uint8) ((int_val >> 8) & 0xff);

	PHY_TXPWR(("idletssi 2g: %d %d\n", pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].idle_tssi_2g,
	          pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].idle_tssi_2g));
	PHY_TXPWR(("idletssi 5g: %d %d\n", pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].idle_tssi_5g,
	          pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].idle_tssi_5g));
}

/** write target_pwr reg and TSSI->pwr & adjusted pwr tables */
static void
wlc_phy_txpwrctrl_pwr_setup_nphy(phy_info_t *pi)
{
	uint16 idx;
	int16  a1[NPHY_CORE_NUM], b0[NPHY_CORE_NUM], b1[NPHY_CORE_NUM];
	int8   target_pwr_qtrdbm[NPHY_CORE_NUM] = { 0 };
	int32  pwr_est;
	uint8  chan_freq_range;
	uint16	idle_tssi[NPHY_CORE_NUM] = {0};
	uint32 tbl_id, tbl_len, tbl_offset;
	uint32 regval[64];
	uint8 core;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	srom_pwrdet_t *pwrdet = pi->pwrdet;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* enable TSSI */
	phy_utils_or_phyreg(pi, NPHY_TSSIMode, NPHY_TSSIMode_tssiEn_MASK);

	phy_utils_and_phyreg(pi, NPHY_TxPwrCtrlCmd, (uint16)(~NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK));

	/* Could make a structure with idle_tssi[], target_pwr[], a1[], b0[], b1[]
	 * and the if/switch logic below could look up the pointer to a source structure,
	 * then have only one copy of the code to copy the values to locals.
	 */

	chan_freq_range = wlc_phy_get_chan_freq_range_nphy(pi, 0);

	FOREACH_CORE(pi, core) {
		if (pi->sh->sromrev < 4) {
			idle_tssi[core] = pi_nphy->nphy_pwrctrl_info[core].idle_tssi_2g;
			target_pwr_qtrdbm[core] = 13 * 4;
			a1[core] = -424;
			b0[core] = 5612;
			b1[core] = -1393;
		} else if (pi->sh->sromrev < 9) {
			/* pick power index from SROM based on current channel */

			target_pwr_qtrdbm[core] = pwrdet->max_pwr[core][chan_freq_range];
			a1[core] = pwrdet->pwrdet_a1[core][chan_freq_range];
			b0[core] = pwrdet->pwrdet_b0[core][chan_freq_range];
			b1[core] = pwrdet->pwrdet_b1[core][chan_freq_range];

			switch (chan_freq_range) {
			case WL_CHAN_FREQ_RANGE_2G:
				idle_tssi[core] = pi_nphy->nphy_pwrctrl_info[core].idle_tssi_2g;
				break;

			case WL_CHAN_FREQ_RANGE_5G_BAND0:
			case WL_CHAN_FREQ_RANGE_5G_BAND1:
			case WL_CHAN_FREQ_RANGE_5G_BAND2:
			case WL_CHAN_FREQ_RANGE_5G_BAND3:
				idle_tssi[core] = pi_nphy->nphy_pwrctrl_info[core].idle_tssi_5g;
				break;

			default:
				PHY_ERROR(("wl%d: %s: channel not found in any frequency range.\n",
					pi->sh->unit, __FUNCTION__));
				break;
			}
		} else {
			/* SROM9
			 * pick the power index based on current channel
			 */

			if (chan_freq_range == WL_CHAN_FREQ_RANGE_2G) {
				idle_tssi[core] = pi_nphy->nphy_pwrctrl_info[core].idle_tssi_2g;
			} else {
				idle_tssi[core] = pi_nphy->nphy_pwrctrl_info[core].idle_tssi_5g;
			}

			a1[core] = pwrdet->pwrdet_a1[core][chan_freq_range];
			b0[core] = pwrdet->pwrdet_b0[core][chan_freq_range];
			b1[core] = pwrdet->pwrdet_b1[core][chan_freq_range];
		}

		PHY_TXPWR(("ANT%d: a1_0 %d b0_0 %d b1 %d\n", core, a1[core], b0[core], b1[core]));

		target_pwr_qtrdbm[core] = (int8)pi->tx_power_max_per_core[core];
	}

	/* Fix target_pwr_qtrdbm[] not initialized error
	 * Moved down here, after initialized
	*/

	if (pi->fem2g->tssipos) {
		phy_utils_or_phyreg(pi, NPHY_TxPwrCtrlIdleTssi,
			NPHY_TxPwrCtrlIdleTssi_tssiPosSlope_MASK);
	}

	FOREACH_CORE(pi, core) {
		if (PHY_IPA(pi)) {
			if (RADIOID(pi->pubpi->radioid) == BCM2057_ID) {
				/* intPA: */
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					WRITE_RADIO_REG3(pi, RADIO_2057, TX, core,
						TX_SSI_MUX, 0xe);
				} else {
					WRITE_RADIO_REG3(pi, RADIO_2057, TX, core,
						TX_SSI_MUX, 0xc);
				}
			}
		} else {
		}
	}

	phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlCmd, NPHY_TxPwrCtrlCmd_pwrIndex_init_MASK,
		(NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7 <<
		NPHY_TxPwrCtrlCmd_pwrIndex_init_SHIFT));

	phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlInit, NPHY_TxPwrCtrlInit_pwrIndex_init1_MASK,
		(NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7 <<
		NPHY_TxPwrCtrlInit_pwrIndex_init1_SHIFT));

	/* average over 8 = 2^3 packets,
	 * sample TSSI at 12us = 240 samples into packet
	 */
	phy_utils_write_phyreg(pi, NPHY_TxPwrCtrlNnum,
		(0x3 << NPHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT) |
		(240 << NPHY_TxPwrCtrlNnum_Ntssi_delay_SHIFT));

	/* assume RSSI ADC is outputting 2s complement values
	 *	 and need to get converted to offset_bin for processing
	 * set idle TSSI in 2s complement format (max is 0x1f)
	 */
	phy_utils_write_phyreg(pi, NPHY_TxPwrCtrlIdleTssi,
		(1 << NPHY_TxPwrCtrlIdleTssi_rawTssiOffsetBinFormat_SHIFT) |
		(idle_tssi[0] << NPHY_TxPwrCtrlIdleTssi_idleTssi0_SHIFT) |
		((PHYCORENUM(pi->pubpi->phy_corenum) > 1) ?
		(idle_tssi[1] << NPHY_TxPwrCtrlIdleTssi_idleTssi1_SHIFT) : 0));

	if (pi->sromi->offset_targetpwr) {
		target_pwr_qtrdbm[0] -= (pi->sromi->offset_targetpwr << 2);
		if (PHYCORENUM(pi->pubpi->phy_corenum) == 1) {
			phy_utils_write_phyreg(pi, NPHY_TxPwrCtrlTargetPwr,
				((uint8)(target_pwr_qtrdbm[0])
				<< NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT));
		} else if (PHYCORENUM(pi->pubpi->phy_corenum) == 2) {
			target_pwr_qtrdbm[1] -= (pi->sromi->offset_targetpwr << 2);
			phy_utils_write_phyreg(pi, NPHY_TxPwrCtrlTargetPwr,
				((uint8)(target_pwr_qtrdbm[0])
				<< NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT) |
				((uint8)(target_pwr_qtrdbm[1])
				<< NPHY_TxPwrCtrlTargetPwr_targetPwr1_SHIFT));
		}
	} else {
		/* set target powers in 6.2 format (in dBs) */
		phy_utils_write_phyreg(pi, NPHY_TxPwrCtrlTargetPwr,
			(target_pwr_qtrdbm[0] << NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT) |
			((PHYCORENUM(pi->pubpi->phy_corenum) > 1) ?
			(target_pwr_qtrdbm[1] << NPHY_TxPwrCtrlTargetPwr_targetPwr1_SHIFT) : 0));
	}

	/* load estimated power tables (maps TSSI to power in dBm)
	 * entries in tx power table 0000xxxxxx
	 */
	tbl_len = 64;
	tbl_offset = 0;
	for (tbl_id = NPHY_TBL_ID_CORE1TXPWRCTL;
		  tbl_id <= NPHY_TBL_ID_CORE2TXPWRCTL; tbl_id++) {

		for (idx = 0; idx < tbl_len; idx++) {
			/* S6.3 format output */
			pwr_est = wlc_phy_tssi2qtrdbm_nphy(
				idx, a1[tbl_id - 26], b0[tbl_id - 26], b1[tbl_id-26], 4);
			regval[idx] = (uint32)pwr_est;
		}
		wlc_phy_table_write_nphy(pi, tbl_id, tbl_len, tbl_offset, 32, regval);
	}

	if (!(pi->sh->sromrev >= 9)) {
		/* load adjusted power tables (maps measured power to power if 1Mbps were sent)
		 *  entries in tx power table 0001xxxxx
		 *					0010xxxxx (only 20 of these used)
		 *	  currently assume that all rates sent at same (1Mbps) power level
		 */
		wlc_phy_txpwr_limit_to_tbl_nphy(pi);

		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE1TXPWRCTL, 84, 64, 8,
			pi_nphy->adj_pwr_tbl_nphy);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE2TXPWRCTL, 84, 64, 8,
			pi_nphy->adj_pwr_tbl_nphy);
	}

	/* Draconian Power Limits for Olympic Sulley */
	if (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)
		wlc_phy_set_tssi_pwr_limit_nphy(pi, a1, b0, b1, NPHY_TSSI_SET_MIN_MAX_LIMIT);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static bool
wlc_phy_txpwr_ison_nphy(phy_info_t *pi)
{
	uint16 mask = (NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
	               NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
	               NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK);

	return ((phy_utils_read_phyreg((pi), NPHY_TxPwrCtrlCmd) & mask) == mask);
}

static uint8
wlc_phy_txpwr_idx_cur_get_nphy(phy_info_t *pi, uint8 core)
{
	uint16 pwrCtrlStatus;
	uint8 pwrIndex = 128;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint8 initPwrIndex5G = 0;
	uint8 initPwrIndex2G = 0;

	pwrCtrlStatus = phy_utils_read_phyreg(pi, (core == PHY_CORE_0) ?
	        NPHY_Core0TxPwrCtrlStatus : NPHY_Core1TxPwrCtrlStatus);

	if (pwrCtrlStatus & 0x8000) {
		pwrIndex = (pwrCtrlStatus & NPHY_TxPwrCtrlStatus_baseIndex_MASK) >>
		         NPHY_TxPwrCtrlStatus_baseIndex_SHIFT;
	} else {

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			initPwrIndex5G = NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7_5G;

			pwrIndex = (pi_nphy->nphy_txpwr_idx_5G[core] != 128) ?
			pi_nphy->nphy_txpwr_idx_5G[core] : initPwrIndex5G;
		} else {
			initPwrIndex2G = NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7;

			pwrIndex = (pi_nphy->nphy_txpwr_idx_2G[core] != 128) ?
			pi_nphy->nphy_txpwr_idx_2G[core] : initPwrIndex2G;
		}
	}

	return pwrIndex;
}

static void
wlc_phy_txpwr_idx_cur_set_nphy(phy_info_t *pi, uint8 idx0, uint8 idx1)
{
	phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlCmd,
	                     NPHY_TxPwrCtrlCmd_pwrIndex_init_MASK, idx0);
	phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlInit,
	                     NPHY_TxPwrCtrlInit_pwrIndex_init1_MASK, idx1);
}

uint16
wlc_phy_txpwr_idx_get_nphy(phy_info_t *pi)
{
	uint16 tmp;
	uint16 pwr_idx[NPHY_CORE_NUM];
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint8 initPwrIndex5G = 0;
	uint8 initPwrIndex2G = 0;

	if (wlc_phy_txpwr_ison_nphy(pi)) {
		pwr_idx[0] = wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_0);
		pwr_idx[1] = wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_1);

		tmp = (pwr_idx[1] << 8) | pwr_idx[0];
	} else {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			initPwrIndex5G = NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7_5G;
			pwr_idx[0] = (pi_nphy->nphy_txpwr_idx_5G[0] != 128) ?
			  pi_nphy->nphy_txpwr_idx_5G[0] : initPwrIndex5G;

			pwr_idx[1] = (pi_nphy->nphy_txpwr_idx_5G[1] != 128) ?
			  pi_nphy->nphy_txpwr_idx_5G[1] : initPwrIndex5G;
		} else {
			initPwrIndex2G = NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7;
			pwr_idx[0] = (pi_nphy->nphy_txpwr_idx_2G[0] != 128) ?
			  pi_nphy->nphy_txpwr_idx_2G[0] : initPwrIndex2G;

			pwr_idx[1] = (pi_nphy->nphy_txpwr_idx_2G[1] != 128) ?
			  pi_nphy->nphy_txpwr_idx_2G[1] : initPwrIndex2G;
		}

		tmp = (pwr_idx[1] << 8) | pwr_idx[0];
	}

	return tmp;
}

void
wlc_phy_txpwr_papd_cal_nphy(phy_info_t *pi)
{
	uint32 delta_idx;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint8 txpwr_idx_cur[NPHY_CORE_NUM] = { 0 };
	uint8 core;

	/* skip cal if phy is muted */
	if (PHY_MUTED(pi))
		return;

	/* Perform a PAPD cal if the Tx gain changes by 1 dB */
	delta_idx = 2;

	/* Note: each watchdog event will trigger PAPD cal on at most one core */
	FOREACH_CORE(pi, core)
		txpwr_idx_cur[core] = wlc_phy_txpwr_idx_cur_get_nphy(pi, core);

	if (PHY_IPA(pi) && (pi_nphy->nphy_force_papd_cal || (wlc_phy_txpwr_ison_nphy(pi) &&
		(((uint32)ABS(txpwr_idx_cur[0] -
		pi_nphy->nphy_papd_tx_gain_at_last_cal[0]) >= delta_idx) ||
		((PHYCORENUM(pi->pubpi->phy_corenum) > 1) ?
		((uint32)ABS(txpwr_idx_cur[1] -
		pi_nphy->nphy_papd_tx_gain_at_last_cal[1]) >= delta_idx) : 0))))) {
		wlc_phy_txpwr_papd_cal_run(pi, TRUE, PHY_CORE_0, pi->pubpi->phy_corenum - 1);
	}
}

void
wlc_phy_txpwr_papd_cal_nphy_dcs(phy_info_t *pi)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* for manual mode, let it run */
	if ((pi->phy_cal_mode != PHY_PERICAL_MPHASE) &&
	    (pi->phy_cal_mode != PHY_PERICAL_MANUAL))
		return;

	/* use timer to wait for clean context since this may be called in the middle of nphy_init
	 */
	wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);

	pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_PAPDCAL;
	pi_nphy->ntd_papdcal_dcs = TRUE;
	wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
}

void
wlc_phy_store_txindex_nphy(phy_info_t *pi)
{
	uint8 core;
	uint8 nphy_txpwr_idx;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	FOREACH_CORE(pi, core) {
		nphy_txpwr_idx = (wlc_phy_txpwr_ison_nphy(pi) ?
			wlc_phy_txpwr_idx_cur_get_nphy(pi, (uint8)core) :
			pi_nphy->nphy_txpwrindex[core].index_internal);

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			pi_nphy->nphy_txpwr_idx_5G[core] =
				nphy_txpwr_idx;
		} else {
			pi_nphy->nphy_txpwr_idx_2G[core] =
				nphy_txpwr_idx;
		}
	}
}

#ifdef WLSRVSDB

/* Used to calculate the offset in SHM where tx power values are stored */
const wlc_rateset_t cck_ofdm_rates_phy = {
	12,
	{ /*	1b,   2b,   5.5b, 6,    9,    11b,  12,   18,   24,   36,   48,   54 Mbps */
		0x82, 0x84, 0x8b, 0x0c, 0x12, 0x96, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c
	},
	0x00,
	{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
	}
};

static uint8
wlc_vsdb_switch(phy_info_t *pi, chanspec_t chanspec)
{

	ASSERT(pi->srvsdb_state->srvsdb_active != 0);

	/* make sure chan0 bank0 and chan1 is written in bank 1 */
	switch (pi->srvsdb_state->vsdb_trig_cnt) {
		case 0 :
			/* Init time entry point for bank 0 */
			if (SRVSDB_IS_BANK0(pi, CHSPEC_CHANNEL(chanspec))) {
				printf("1: Invalid bank change at init  !!!!!!!!  "
					"Return %x \n", pi->srvsdb_state->prev_chanspec);
				ASSERT(0);
				return FALSE;
			}
			PHY_INFORM(("1ts Bank SR save %x : restore %x \n",
				pi->srvsdb_state->prev_chanspec,
				pi->srvsdb_state->prev_chanspec));
			pi->srvsdb_state->sr_vsdb_bank_valid[0] = TRUE;
			pi->srvsdb_state->vsdb_trig_cnt++;
			break;
		case 1 :
			/* Init time entry point for bank 1 */
			if (!SRVSDB_IS_BANK0(pi, CHSPEC_CHANNEL(chanspec))) {
				printf("2: Invalid bank change at init  !!!!!!!! "
					"Return %x\n", pi->srvsdb_state->prev_chanspec);
				ASSERT(0);
				return FALSE;
			}
			pi->srvsdb_state->sr_vsdb_bank_valid[1] = TRUE;
			pi->srvsdb_state->vsdb_trig_cnt++;
			PHY_INFORM(("2nd Bank SR save %x : restore %x \n",
				pi->srvsdb_state->prev_chanspec, chanspec));
			break;
		case 2:
			pi->srvsdb_state->vsdb_trig_cnt++;
			PHY_INFORM(("SR save %x : restore %x \n", pi->srvsdb_state->prev_chanspec,
				chanspec));
			break;
		default :
			{
			PHY_INFORM(("SR save %x : restore %x \n", pi->srvsdb_state->prev_chanspec,
				chanspec));
			break;
			}
	}
	/* Trigger Radio register restore from SHM by ucode */
#ifdef WLUCODE_RDO_SR
		wlapi_bmac_write_shm(pi->sh->physhim, M_RDO_REG_UCODE_SR_CMD,
			(0x4 << offset));
#endif // endif
	/* Trigger SR VSDB */
	sr_vsdb_trigger(pi, 100);

	return TRUE;
}

/**
 * This function stores and restores the elements which is been utilized by the function
 * wlc_phy_init_nphy(). sr_vsdb->vsdb_init variable is either TRUE or FALSE, its output is based on
 * the function wlc_vsdb_switch. If it is TRUE, then store all the pi information into arry of
 * structure, or if the variable is FALSE, then restore the array information back to the pi
 * structure.
 */
static void
wlc_vsdb_sr_chanspec_set(phy_info_t *pi, uint8 save, uint8 offset)
{

	vsdb_backup_t *vsdb_bkp[2];

	vsdb_bkp[0] = pi->vsdb_bkp[0];
	vsdb_bkp[1] = pi->vsdb_bkp[1];

	if (save)
	{
		PHY_INFORM(("SRVSDB: SW save for chan %x \n", pi->srvsdb_state->prev_chanspec));

		bcopy(pi->u.pi_nphy, (vsdb_bkp[offset]->pi_nphy), sizeof(phy_info_nphy_t));
		bcopy(&(pi->interf), &(vsdb_bkp[offset]->interf), sizeof(interference_info_t));
		vsdb_bkp[offset]->bw                          = pi->bw;
		vsdb_bkp[offset]->phy_classifier_state        = pi->phy_classifier_state;
		vsdb_bkp[offset]->saved_tempsense             = pi->saved_tempsense;
		vsdb_bkp[offset]->saved_tempsense_valid       = pi->saved_tempsense_valid;
		vsdb_bkp[offset]->radio_chanspec              = pi->srvsdb_state->prev_chanspec;
		/* tx parameters added */
		vsdb_bkp[offset]->tx_power_max[0] = pi->tx_power_max_per_core[0];
		vsdb_bkp[offset]->tx_power_max[1] = pi->tx_power_max_per_core[1];
		vsdb_bkp[offset]->tx_power_min[0] = pi->tx_power_min_per_core[0];
		vsdb_bkp[offset]->tx_power_min[1] = pi->tx_power_min_per_core[1];

		if ((vsdb_bkp[offset]->tx_power_offset != NULL) &&
			(ppr_get_ch_bw(pi->tx_power_offset) !=
			ppr_get_ch_bw(vsdb_bkp[offset]->tx_power_offset))) {
			ppr_delete(pi->sh->osh, vsdb_bkp[offset]->tx_power_offset);
			vsdb_bkp[offset]->tx_power_offset = NULL;
		}
		if (vsdb_bkp[offset]->tx_power_offset == NULL) {
			vsdb_bkp[offset]->tx_power_offset = ppr_create(pi->sh->osh,
				ppr_get_ch_bw(pi->tx_power_offset));
		}
		if (vsdb_bkp[offset]->tx_power_offset != NULL) {
			ppr_copy_struct(pi->tx_power_offset,
				vsdb_bkp[offset]->tx_power_offset);
		}

		vsdb_bkp[offset]->openlp_tx_power_min	      = pi->openlp_tx_power_min;
		vsdb_bkp[offset]->cur_interference_mode       = pi->cur_interference_mode;
		vsdb_bkp[offset]->aci_state                   = pi->aci_state;
		vsdb_bkp[offset]->aci_active_pwr_level	      = pi->aci_active_pwr_level;
		vsdb_bkp[offset]->last_aci_call		      =	pi->last_aci_call;
		vsdb_bkp[offset]->last_aci_check_time	      =	pi->last_aci_check_time;
		vsdb_bkp[offset]->radio_is_on                 = pi->radio_is_on;
		vsdb_bkp[offset]->phy_init_por                = pi->phy_init_por;
		vsdb_bkp[offset]->interference_mode           = pi->sh->interference_mode;
		vsdb_bkp[offset]->interference_mode_crs       = pi->interference_mode_crs;
		vsdb_bkp[offset]->rx_antdiv                   = pi->sh->rx_antdiv;
		vsdb_bkp[offset]->spur_mode                   = pi->phy_spuravoid_mode;
		vsdb_bkp[offset]->do_noisemode_reset          = pi->do_noisemode_reset;
		vsdb_bkp[offset]->do_acimode_reset            = pi->do_acimode_reset;
#ifdef RXDESENS_EN
		vsdb_bkp[offset]->phyrxdesens                 = pi->sromi->phyrxdesens;
		vsdb_bkp[offset]->saved_interference_mode     = pi->sromi->saved_interference_mode;
#endif // endif
	}
	else
	{
		bcopy((vsdb_bkp[offset]->pi_nphy), pi->u.pi_nphy, sizeof(phy_info_nphy_t));
		bcopy(&(vsdb_bkp[offset]->interf), &(pi->interf), sizeof(interference_info_t));
		pi->bw                          = vsdb_bkp[offset]->bw;
		pi->phy_classifier_state        = vsdb_bkp[offset]->phy_classifier_state;
		pi->saved_tempsense             = vsdb_bkp[offset]->saved_tempsense;
		pi->saved_tempsense_valid       = vsdb_bkp[offset]->saved_tempsense_valid;
		pi->radio_chanspec              = vsdb_bkp[offset]->radio_chanspec;

		if (vsdb_bkp[offset]->tx_power_offset != NULL) {
			if ((pi->tx_power_offset != NULL) &&
				(ppr_get_ch_bw(pi->tx_power_offset) !=
				ppr_get_ch_bw(vsdb_bkp[offset]->tx_power_offset))) {
				ppr_delete(pi->sh->osh, pi->tx_power_offset);
				pi->tx_power_offset = NULL;
			}
			if (pi->tx_power_offset == NULL) {
				pi->tx_power_offset = ppr_create(pi->sh->osh,
					ppr_get_ch_bw(vsdb_bkp[offset]->tx_power_offset));
			}
			if (pi->tx_power_offset != NULL) {
				ppr_copy_struct(vsdb_bkp[offset]->tx_power_offset,
					pi->tx_power_offset);
			}
		}

		pi->tx_power_max_per_core[0] = vsdb_bkp[offset]->tx_power_max[0];
		pi->tx_power_max_per_core[1] = vsdb_bkp[offset]->tx_power_max[1];
		pi->tx_power_min_per_core[0] = vsdb_bkp[offset]->tx_power_min[0];
		pi->tx_power_min_per_core[1] = vsdb_bkp[offset]->tx_power_min[1];
		pi->openlp_tx_power_min		= vsdb_bkp[offset]->openlp_tx_power_min;
		pi->cur_interference_mode               = vsdb_bkp[offset]->cur_interference_mode;
		pi->aci_state                           = vsdb_bkp[offset]->aci_state;
		pi->aci_active_pwr_level		= vsdb_bkp[offset]->aci_active_pwr_level;
		pi->last_aci_check_time			= vsdb_bkp[offset]->last_aci_check_time;
		pi->last_aci_call			= vsdb_bkp[offset]->last_aci_call;
		pi->radio_is_on                         = vsdb_bkp[offset]->radio_is_on;
		pi->phy_init_por                        = vsdb_bkp[offset]->phy_init_por;
		pi->sh->interference_mode               = vsdb_bkp[offset]->interference_mode;
		pi->interference_mode_crs               = vsdb_bkp[offset]->interference_mode_crs;
		pi->sh->rx_antdiv                       = vsdb_bkp[offset]->rx_antdiv;
		pi->radio_chanspec                      = vsdb_bkp[offset]->radio_chanspec;
		pi->phy_spuravoid_mode                  = vsdb_bkp[offset]->spur_mode;
		pi->do_noisemode_reset                  = vsdb_bkp[offset]->do_noisemode_reset;
		pi->do_acimode_reset                    = vsdb_bkp[offset]->do_acimode_reset;
#ifdef RXDESENS_EN
		pi->sromi->phyrxdesens                  = vsdb_bkp[offset]->phyrxdesens;
		pi->sromi->saved_interference_mode      = vsdb_bkp[offset]->saved_interference_mode;
#endif // endif

		PHY_INFORM(("SRVSDB: SW restored for chan %x\n", pi->radio_chanspec));
	}
}

#ifndef WLUCODE_RDO_SR
/**
 * Restore all of the Radio registers which was stored during vsdb_radio_snapshot depending on the
 * channel type
 */
void
wlc_vsdb_radio_restore(phy_info_t *pi, uint8 offset)
{
	/* phy_info_t *pi = (phy_info_t *)ppi; */
	uint16 i;
	vsdb_backup_t *vsdb_bkp[2];
	vsdb_bkp[0] = pi->vsdb_bkp[0];
	vsdb_bkp[1] = pi->vsdb_bkp[1];

	if (pi->srvsdb_state->swbkp_snapshot_valid[offset] == 1)
	{
		PHY_INFORM(("SRVSDB: Radio restore for chan %x \n", pi->radio_chanspec));
		for (i = 0; i < MAX_RADIO_REGS; i++)
		{
			phy_utils_write_radioreg(pi, i, vsdb_bkp[offset]->radio_reg_val[i]);
		}
	}
}
#endif /* WLUCODE_RDO_SR */

/**
 * Read and store all of the Radio Register contents into array based on the channel information and
 * offset
 */
void
wlc_vsdb_radio_snapshot(phy_info_t *pi, uint8 offset)
{

#ifdef WLUCODE_RDO_SR
	/* Triggering ucode Radio register save from Driver */
	/* ucode starts save of Radio regs into shm */
	wlapi_bmac_write_shm(pi->sh->physhim, M_RDO_REG_UCODE_SR_CMD, (0x1 << offset));
#else
	uint16 i;
	vsdb_backup_t *vsdb_bkp[2];
	vsdb_bkp[0] = pi->vsdb_bkp[0];
	vsdb_bkp[1] = pi->vsdb_bkp[1];

	PHY_INFORM(("SRVSDB: RADIO save for chan %x \n", pi->srvsdb_state->prev_chanspec));

	for (i = 0; i < MAX_RADIO_REGS; i++)
	{
		vsdb_bkp[offset]->radio_reg_val[i] = phy_utils_read_radioreg(pi, i);
	}
#endif /* WLUCODE_RDO_SR */

}

/**
 * Restore all the Tx power contents back to SHM which was stored during vsdb_txpower_snapshot.
 */
void
wlc_vsdb_txpower_restore(phy_info_t *pi, uint8 offset)
{
	uint16 i;
	const wlc_rateset_t *rs_dflt;
	wlc_rateset_t rs;
	uint8 rate;
	uint16 off_set;

	vsdb_backup_t *vsdb_bkp[2];
	vsdb_bkp[0] = pi->vsdb_bkp[0];
	vsdb_bkp[1] = pi->vsdb_bkp[1];

	rs_dflt = &cck_ofdm_rates_phy;
	bcopy(rs_dflt, &rs, sizeof(wlc_rateset_t));

	if (pi->srvsdb_state->swbkp_snapshot_valid[offset] == 1)
	{
		for (i = 0; i < rs.count; i++)
		{
			rate = rs.rates[i] & RATE_MASK_PHY;
			off_set = wlapi_bmac_rate_shm_offset(pi->sh->physhim, rate);
			wlapi_bmac_write_shm(pi->sh->physhim, off_set + M_RT_TXPWROFF_POS(pi),
				vsdb_bkp[offset]->tx_power_shm[i]);
		}
	}
}

/**
 * Read the Tx power contents from SHM and save into array based on the channel information and
 * offset.
 */
void
wlc_vsdb_txpower_snapshot(phy_info_t *pi, uint8 offset)
{
	uint16 i;
	const wlc_rateset_t *rs_dflt;
	wlc_rateset_t rs;
	uint8 rate;
	uint16 off_set;

	vsdb_backup_t *vsdb_bkp[2];
	vsdb_bkp[0] = pi->vsdb_bkp[0];
	vsdb_bkp[1] = pi->vsdb_bkp[1];

	rs_dflt = &cck_ofdm_rates_phy;
	bcopy(rs_dflt, &rs, sizeof(wlc_rateset_t));

	for (i = 0; i < rs.count; i++)
	{
		rate = rs.rates[i] & RATE_MASK_PHY;
		off_set = wlapi_bmac_rate_shm_offset(pi->sh->physhim, rate);
		vsdb_bkp[offset]->tx_power_shm[i] =  wlapi_bmac_read_shm(pi->sh->physhim,
			off_set + M_RT_TXPWROFF_POS(pi));
	}
}

/**
 * This function tells whether VSDB should be triggered or not based on current channel and previous
 * channel, also based on the second bank is used or not. If second bank is not used switch is not
 * allowed.
 */
uint8
sr_vsdb_switch_allowed(phy_info_t *pi, chanspec_t chanspec)
{
	uint8 allowed = FALSE;

	/* switch allowed only if current channel and last channel are vsdb channels */

	/* forcing VSDB from iovar */
	/* Dont care about the order */
	if (pi->srvsdb_state->force_vsdb)
		return TRUE;

	if ((CHSPEC_CHANNEL(chanspec) == pi->srvsdb_state->sr_vsdb_channels[0]) &&
		(CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec) ==
		pi->srvsdb_state->sr_vsdb_channels[1])) {
		allowed = TRUE;
	} else if ((CHSPEC_CHANNEL(chanspec) == pi->srvsdb_state->sr_vsdb_channels[1]) &&
		(CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec) ==
		pi->srvsdb_state->sr_vsdb_channels[0])) {
		allowed = TRUE;
	} else {
		allowed = FALSE;
		return allowed;
	}
	/* Always make sure 2g band saved in banko and 5g band in bank 1 */
	/* This will be usefull for housekeeping and cross checking at any point of time */
	if (SRVSDB_BAND_BANK_INVALID(pi, SRVSDB_IS_BANK0(pi, CHSPEC_CHANNEL(chanspec)))) {
		PHY_INFORM(("Bank -Band invalid: Defer the switch  chan %x\n", chanspec));
		allowed = FALSE;
	}

	return allowed;
}

/**
 * Function to check whether VSDB switching is needed or not, if switching is not done, then normal
 * initialization of channel is done, else switching of VSDB happens in this particular function.
 */
static uint8
wlc_phy_srvsdb_swbackup_save(phy_info_t * pi)
{
	uint8 offset = 0;

	/* Find appropriate bank to be saved */
	if (CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec) ==
		pi->srvsdb_state->sr_vsdb_channels[0]) {
		offset = 0;
	} else if (CHSPEC_CHANNEL(pi->srvsdb_state->prev_chanspec) ==
		pi->srvsdb_state->sr_vsdb_channels[1]) {
		offset = 1;
	} else {
		printf("Invalid chan combination in SW save \n");
		ASSERT(0);
		return FALSE;
	}

	if (pi->srvsdb_state->swbkp_snapshot_valid[offset] == 0)
	{
		/* Radio snapshot */
		wlc_vsdb_radio_snapshot(pi, offset);
		/* txpower related shm */
		wlc_vsdb_txpower_snapshot(pi, offset);
		/* pi structure backup */
		wlc_vsdb_sr_chanspec_set(pi, SRVSDB_SAVE, offset);

		/* ucode clears the Radio Save bit after Radio Save is done. */
		/* Poll for Radio Save to be clear */
#ifdef WLUCODE_RDO_SR
		SPINWAIT(((wlapi_bmac_read_shm(pi->sh->physhim, M_RDO_REG_UCODE_SR_CMD)& 0x3) != 0),
			NPHY_SPINWAIT_RDO_REG_SR_CMD_POLL_TIMEOUT);

	        ASSERT((wlapi_bmac_read_shm(pi->sh->physhim, M_RDO_REG_UCODE_SR_CMD) & 0x3));
#endif // endif
		pi->srvsdb_state->swbkp_snapshot_valid[offset] = 1;
	}
	return TRUE;
}

#define VSDB_TSF_TIMER_ADJUST	390 /* Fix for 700us drift */

/**
 * Estimate cumulative delta counters for rxcrsglitch, bphy_rxcrsglitch bphy_badplcp badplcp.
 * Whenever cumulative time in one channel exceeds 1 sec, use cum delta counters as inputs to
 * moving avg window in aci_upd_ma
 */
static void
wlc_phy_vsdb_rxcntr_est(phy_info_t *pi, uint8 offset)
{
	int cur_cnt = 0;
	uint32 delta = 0;

	/* determine delta number of rxcrs glitches */
	cur_cnt = wlapi_bmac_read_shm(pi->sh->physhim, MACSTAT_ADDR(pi, MCSTOFF_RXCRSGLITCH));
	delta = pi->srvsdb_state->prev_crsglitch_cnt[!offset] -
		pi->srvsdb_state->prev_crsglitch_cnt[offset];

	pi->srvsdb_state->sum_delta_crsglitch[offset] += delta;
	pi->srvsdb_state->prev_crsglitch_cnt[offset] = cur_cnt;

	/* determine delta number of rxcrs bphy glitches */
	cur_cnt = wlapi_bmac_read_shm(pi->sh->physhim, MACSTAT_ADDR(pi, MCSTOFF_BPHYGLITCH));
	delta = pi->srvsdb_state->prev_bphy_rxcrsglitch_cnt[!offset] -
		pi->srvsdb_state->prev_bphy_rxcrsglitch_cnt[offset];

	pi->srvsdb_state->sum_delta_bphy_crsglitch[offset] +=  delta;
	pi->srvsdb_state->prev_bphy_rxcrsglitch_cnt[offset] = cur_cnt;

	/* determine delta number of rxbad plcp */
	cur_cnt = wlapi_bmac_read_shm(pi->sh->physhim, MACSTAT_ADDR(pi, MCSTOFF_BPHY_BADPLCP));
	delta = pi->srvsdb_state->prev_badplcp_cnt[!offset] -
		pi->srvsdb_state->prev_badplcp_cnt[offset];
	pi->srvsdb_state->sum_delta_prev_badplcp[offset] += delta;
	pi->srvsdb_state->prev_badplcp_cnt[offset] = cur_cnt;

	/* determine delta number of bphy rxbad plcp */
	cur_cnt = wlapi_bmac_read_shm(pi->sh->physhim, MACSTAT_ADDR(pi, MCSTOFF_BPHY_BADPLCP));
	delta = pi->srvsdb_state->prev_bphy_badplcp_cnt[!offset] -
		pi->srvsdb_state->prev_bphy_badplcp_cnt[offset];

	pi->srvsdb_state->sum_delta_prev_bphy_badplcp[offset] += delta;
	pi->srvsdb_state->prev_bphy_badplcp_cnt[offset] = cur_cnt;
}

static uint32
wlc_phy_clk_bwbits(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	uint32 phy_bw_clkbits = 0;

	/* select the phy speed according to selected channel b/w applies to NPHY's only */
	if (pi) {
		switch (pi->bw) {
			case WL_CHANSPEC_BW_10:
				phy_bw_clkbits = SICF_BW10;
				break;
			case WL_CHANSPEC_BW_20:
				phy_bw_clkbits = SICF_BW20;
				break;
			case WL_CHANSPEC_BW_40:
				phy_bw_clkbits = SICF_BW40;
				break;
#ifdef WL_CHANSPEC_BW_80
			case WL_CHANSPEC_BW_80:
				phy_bw_clkbits = SICF_BW80;
				break;
#endif /* WL_CHANSPEC_BW_80 */
			default:
				ASSERT(0); /* should never get here */
				break;
		}
	}

	return phy_bw_clkbits;
}

static uint8
wlc_phy_srvsdb_swbackup_restore(phy_info_t *pi, chanspec_t chanspec)
{
	uint8 offset = 0;
	uint32 phy_bw_clkbits;
	uint32 slow_clk_cnt_before, tsf_time_before, slow_clk_cnt_after,
		tsf_time_after, time_elapsed_us = 0;
	uint32 cur_timer;

	vsdb_backup_t *vsdb_bkp[2];
	vsdb_bkp[0] = pi->vsdb_bkp[0];
	vsdb_bkp[1] = pi->vsdb_bkp[1];

	/* find appropriate offset */
	if (CHSPEC_CHANNEL(chanspec) == pi->srvsdb_state->sr_vsdb_channels[0]) {
		offset = 0;
	} else if (CHSPEC_CHANNEL(chanspec) == pi->srvsdb_state->sr_vsdb_channels[1]) {
		offset = 1;
	} else {
		printf("Invalid chan combination  SW restore \n");
		ASSERT(0);
		return FALSE;
	}
	if (pi->srvsdb_state->swbkp_snapshot_valid[offset]) {
		/* get cumulative time for which device stays in each channel */
		cur_timer = R_REG(GENERIC_PHY_INFO(pi)->osh, D11_TSFTimerLow(pi));
		pi->srvsdb_state->sum_delta_timer[offset] += pi->srvsdb_state->prev_timer[!offset] -
			pi->srvsdb_state->prev_timer[offset];
		pi->srvsdb_state->prev_timer[offset] = cur_timer;

		/* get cumulative of delta counters during every 1 sec */
		wlc_phy_vsdb_rxcntr_est(pi, offset);

		/* Get the num of iterations, the switch ahs happened to each channel */
		pi->srvsdb_state->num_chan_switch[offset]++;

	if (vsdb_bkp[offset]->spur_mode  != pi->phy_spuravoid_mode) {
		/* Read the TSF timer */
		tsf_time_before = R_REG(GENERIC_PHY_INFO(pi)->osh, D11_TSFTimerLow(pi));
		/* Read the hnd timer */
		slow_clk_cnt_before = hnd_clk_count();
		si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, vsdb_bkp[offset]->spur_mode);
		wlapi_switch_macfreq(pi->sh->physhim, vsdb_bkp[offset]->spur_mode);
		/* Read the TSF timer */
		tsf_time_after = R_REG(GENERIC_PHY_INFO(pi)->osh, D11_TSFTimerLow(pi));
		slow_clk_cnt_after = hnd_clk_count();

		time_elapsed_us =  ((((slow_clk_cnt_after - slow_clk_cnt_before) * 305176) / 10000)
					- (tsf_time_after - tsf_time_before));

		/* time_elapsed_us = ((((slow_clk_cnt_after - slow_clk_cnt_before)/2) * 305176) /
			10000) + 100;
		 time_elapsed_us = VSDB_TSF_TIMER_ADJUST;
		*/

		wlapi_tsf_adjust(pi->sh->physhim, (uint32)time_elapsed_us);
	}

		/* BW changes */
		if (vsdb_bkp[offset]->bw != pi->bw) {
			pi->bw = CHSPEC_BW(chanspec);
			phy_bw_clkbits = wlc_phy_clk_bwbits((wlc_phy_t *)pi);
			si_core_cflags(pi->sh->sih, (SICF_BWMASK),
			        (phy_bw_clkbits));
		}

		 /* Update ucode channel value */
		phy_chanmgr_set_shm(pi, chanspec);
		/* pi structire */
		wlc_vsdb_sr_chanspec_set(pi, SRVSDB_RESTORE, offset);
		/* radio regs */
#ifndef WLUCODE_RDO_SR
		wlc_vsdb_radio_restore(pi, offset);
#endif // endif
		/* txpower related shms */
		wlc_vsdb_txpower_restore(pi, offset);
	}
	return TRUE;
}

/**
 * vsdb chiptrigger; trigger the hardware for saving, check for status bit, after saving is
 * completed, restoring will be done in the similar way. If save or restore did not happen within
 * sufficient time, then display error message
 */
/* returns a toggle value after each vsdb switch */
static uint8
wlc_phy_sr_vsdb_status(phy_info_t *pi)
{
	uint32 read_status;
	si_t *sih;
	uint origidx;
	chipcregs_t *cc;
	/*
	 * This bit toggles after every vsdb switch. It can be used to keep track of the band, as
	 * seen by sr_engine.
	 */
	uint sr_chip_status_2_bit = 31;

	sih = (si_t*)pi->sh->sih;

	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);

	read_status = (R_REG(si_osh(sih), &cc->chipstatus) & (1 << sr_chip_status_2_bit));

	read_status = read_status  >> sr_chip_status_2_bit;

	si_setcoreidx(sih, origidx);
	return (uint8)read_status;
}

/** software triggers hardware to start a vsdb hardware context save/restore */
void sr_vsdb_trigger(phy_info_t *pi, int delay)
{
	int trigBit = 0;
	int statusBit;
	int resetTrigBit = 0;
	int read_status, waitCount;
	si_t *sih;
	uint origidx;
	chipcregs_t *cc;

	sih = (si_t*)pi->sh->sih;

	/* sr_fifo_reset(pi, sav_res); */
	trigBit =   0x00800000;
	statusBit = 0x20000000; /* bit29 */

	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	read_status = (R_REG(si_osh(sih), &cc->chipstatus) & statusBit);
	sr_chipcontrol(sih, trigBit, 0); /* pmu chip control 3 voor 43242 */
	sr_chipcontrol(sih, trigBit, trigBit);

	/* Work on Delay */
	waitCount = delay;
	do
	{
		read_status = (R_REG(si_osh(sih), &cc->chipstatus) & statusBit);
		OSL_DELAY(2);
		if ((delay != 0) && (waitCount == 0))
		{
			PHY_ERROR(("vsdbChipTrig: vsdb save/restore not successful\n"));
			printf("vsdbChipTrig: vsdb save/restore not successful\n");
			ASSERT(0);
			break;
		}
		waitCount -= 1;

	} while (read_status != statusBit);
	sr_chipcontrol(sih, trigBit, resetTrigBit);

	si_setcoreidx(sih, origidx);
}

/**
 * wlc_phy_srvsdb_prepare save - avoids PHY stalling when h/w vsdb switching takes place
 * wlc_phy_srvsdb_prepare restore - brings back PHY to normal mode after switching of channel has
 * happened
 */
void wlc_phy_srvsdb_prepare(phy_info_t *pi, int sav_res)
{
	/* phy_info_t *pi = (phy_info_t*)ppi; */

	if (sav_res == ENTER)
	{
		phy_utils_mod_phyreg(pi, NPHY_forceClk,
			NPHY_forceClk_disable_stalls_MASK,
			1 << NPHY_forceClk_disable_stalls_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_REV19_rxfeFifoCtrl,
			NPHY_REV19_RxfeFifoCtrl_force_rxfeFifo_Reset_MASK,
			1 << NPHY_REV19_RxfeFifoCtrl_force_rxfeFifo_Reset_SHIFT);
	}
	else if (sav_res == EXIT)
	{
		phy_utils_mod_phyreg(pi, NPHY_REV19_rxfeFifoCtrl,
			NPHY_REV19_RxfeFifoCtrl_force_rxfeFifo_Reset_MASK,
			0 << NPHY_REV19_RxfeFifoCtrl_force_rxfeFifo_Reset_SHIFT);
		phy_utils_mod_phyreg(pi, NPHY_forceClk,
			NPHY_forceClk_disable_stalls_MASK,
			0 << NPHY_forceClk_disable_stalls_SHIFT);
	}

}

#endif /* end WLSRVSDB */

void
wlc_phy_txpwrctrl_enable_nphy(phy_info_t *pi, uint8 ctrl_type)
{
	uint16 mask = 0, val = 0, ishw = 0;
	uint16 ctr;
	uint core;
	uint8 nphy_txpwr_idx;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* check for recognized commands */
	switch (ctrl_type) {
	case PHY_TPC_HW_OFF:
		pi->nphy_txpwrctrl = ctrl_type;
		break;
	case PHY_TPC_HW_ON:
		pi->nphy_txpwrctrl = ctrl_type;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unrecognized ctrl_type: %d\n",
			pi->sh->unit, __FUNCTION__, ctrl_type));
		break;
	}

	if (ctrl_type == PHY_TPC_HW_OFF) {
		/* save previous txpwr index if txpwrctl was enabled */
		if (wlc_phy_txpwr_ison_nphy(pi)) {
			FOREACH_CORE(pi, core) {
				nphy_txpwr_idx = wlc_phy_txpwr_idx_cur_get_nphy(pi, (uint8)core);

				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					pi_nphy->nphy_txpwr_idx_5G[core] = nphy_txpwr_idx;
				} else {
					pi_nphy->nphy_txpwr_idx_2G[core] = nphy_txpwr_idx;
				}
			}
		}

		/* force to the user-specified index here? */

		if (!(pi->sh->sromrev >= 9)) {
			/* clear adjusted power tables so that reported power is for last frame
			 *   (no rate-based adjustment)
			 */
			uint32 tbl_offset = 64;
			uint32 tbl_len = 84;
			uint8 regval[256];
			for (ctr = 0; ctr < tbl_len; ctr++) {
				regval[ctr] = 0;
			}

			wlc_phy_table_write_nphy(pi, 26, tbl_len, tbl_offset, 8, regval);
			wlc_phy_table_write_nphy(pi, 27, tbl_len, tbl_offset, 8, regval);
		}

		phy_utils_and_phyreg(pi, NPHY_TxPwrCtrlCmd,
			(uint16) (~(NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
			NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
			NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)));

		PHY_REG_LIST_START
			PHY_REG_OR_ENTRY(NPHY, AfectrlOverride1,
			           NPHY_REV3_AfectrlOverride_dac_gain_MASK)
			PHY_REG_OR_ENTRY(NPHY, AfectrlOverride2,
			           NPHY_REV3_AfectrlOverride_dac_gain_MASK)
		PHY_REG_LIST_EXECUTE(pi);

	} else {

		if (!(pi->sh->sromrev >= 9)) {
			/* load adjusted power tables (maps measured pwr to pwr if 1Mbps were sent)
			 *    entries in tx power table 0001xxxxx
			 *                              0010xxxxx (only 20 of these used)
			 *    currently assume that all rates sent at same (1Mbps) power level
			 */
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE1TXPWRCTL, 84, 64, 8,
			                         pi_nphy->adj_pwr_tbl_nphy);
			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_CORE2TXPWRCTL, 84, 64, 8,
			                         pi_nphy->adj_pwr_tbl_nphy);
		}

		/* disable power control before setting init indices so they get picked up
		 * when power control is enabled below
		 */

		phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlCmd,
		                     NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
		                     NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK, 0);

		/* lower the starting txpwr index for A-Band */
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(NPHY, TxPwrCtrlCmd, pwrIndex_init,
					NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7_5G)
				PHY_REG_MOD_ENTRY(NPHY, TxPwrCtrlInit,
					pwrIndex_init1,
					NPHY_TxPwrCtrlCmd_pwrIndex_init_rev7_5G)
			PHY_REG_LIST_EXECUTE(pi);
		}

		/* restore the old txpwr index if they are valid */
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if ((pi_nphy->nphy_txpwr_idx_5G[0] != 128) &&
			    (pi_nphy->nphy_txpwr_idx_5G[1] != 128)) {
				wlc_phy_txpwr_idx_cur_set_nphy(pi,
				   pi_nphy->nphy_txpwr_idx_5G[0],
				   pi_nphy->nphy_txpwr_idx_5G[1]);
			}
		} else {
			if ((pi_nphy->nphy_txpwr_idx_2G[0] != 128) &&
			    (pi_nphy->nphy_txpwr_idx_2G[1] != 128)) {
				wlc_phy_txpwr_idx_cur_set_nphy(pi,
				   pi_nphy->nphy_txpwr_idx_2G[0],
				   pi_nphy->nphy_txpwr_idx_2G[1]);
			}
		}

		/* since pwr_ctrl_type != "off", set appropriate regs
		 *   depending on pwr_ctrl_type
		 */
		ishw = (ctrl_type == PHY_TPC_HW_ON) ? 0x1 : 0x0;
		mask = NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
		        NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK;
		val = (ishw << NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_SHIFT) |
		        (ishw << NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_SHIFT);
		mask |= NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK;
		val |= (ishw << NPHY_TxPwrCtrlCmd_txPwrCtrl_en_SHIFT);

		phy_utils_mod_phyreg(pi, NPHY_TxPwrCtrlCmd, mask, val);

		/* Turn off Override of DAC Gain bit */
		PHY_REG_LIST_START
			PHY_REG_AND_ENTRY(NPHY, AfectrlOverride1,
				~NPHY_REV3_AfectrlOverride_dac_gain_MASK)
			PHY_REG_AND_ENTRY(NPHY, AfectrlOverride2,
				~NPHY_REV3_AfectrlOverride_dac_gain_MASK)
		PHY_REG_LIST_EXECUTE(pi);

		if (PHY_IPA(pi)) {
			PHY_REG_LIST_START
				PHY_REG_MOD_CORE_ENTRY(NPHY, 0, PapdEnable, gainDacRfOverride, 0)
				PHY_REG_MOD_CORE_ENTRY(NPHY, 1, PapdEnable, gainDacRfOverride, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

void
wlc_phy_txpwr_index_nphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex, bool restore_cals)
{
	uint8 core, txpwrctl_tbl;
	uint16 tx_ind0, iq_ind0, lo_ind0;
	uint16 m1m2;
	uint32 txgain;
	uint16 rad_gain, dac_gain;
	uint8 bbmult;
	uint32 iqcomp;
	uint16 iqcomp_a, iqcomp_b;
	uint32 locomp;
	uint16 tmpval;
	uint8 tx_pwr_ctrl_state;
	int32 rfpwr_offset;
	uint16 regval[2];
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: %s, RSSI LUT done\n", pi->sh->unit, __FUNCTION__));

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Set TX power, IQ comp, LO comp based on an input "index"
	 * (Emulate what HW power control would use for a given table index)
	 */

	tx_ind0 = 192;
	iq_ind0 = 320;
	lo_ind0 = 448;

	FOREACH_CORE(pi, core) {

		/* check core_mask */
		if ((core_mask & (1 << core)) == 0) {
			continue;
		}

		txpwrctl_tbl = (core == PHY_CORE_0) ? 26 : 27;

		/* are we forcing a power index [0,128], or are we restoring to "auto"? */
		if (txpwrindex < 0) {
			if (pi_nphy->nphy_txpwrindex[core].index < 0) {
				/* power is already auto for this core, nothing to do;
				 * also in case someone issues "auto" without ever
				 * overriding beforehand, this way we avoid trying to
				 * restore overridden regs to values we've never saved
				 * off.
				 */
				continue;
			}

			/* restore overridden regs to auto defaults */

			/* AfeCtrlOverride */
			phy_utils_mod_phyreg(pi, NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_dac_gain_MASK,
				pi_nphy->nphy_txpwrindex[core].AfectrlOverride);
			phy_utils_mod_phyreg(pi, NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_dac_gain_MASK,
				pi_nphy->nphy_txpwrindex[core].AfectrlOverride);

			/* AfeCtrlDacGain */
			phy_utils_write_phyreg(pi, (core == PHY_CORE_0) ?
			              NPHY_AfectrlDacGain1 : NPHY_AfectrlDacGain2,
			              pi_nphy->nphy_txpwrindex[core].AfeCtrlDacGain);

			/* radio gains */
			wlc_phy_table_write_nphy(pi, 7, 1, (0x110 + core), 16,
			    &pi_nphy->nphy_txpwrindex[core].rad_gain);

			/* iqLoCaltbl bbmult
			 * bbmult for OFDM and BPHY are at offsets 87 and 95, resp.
			 */
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m1m2);
			m1m2 &= ((core == PHY_CORE_0) ? 0x00ff : 0xff00);
			m1m2 |= ((core == PHY_CORE_0) ?
			         (pi_nphy->nphy_txpwrindex[core].bbmult << 8) :
			         (pi_nphy->nphy_txpwrindex[core].bbmult << 0));
			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m1m2);

			wlc_phy_table_read_nphy(pi, 15, 1, 95, 16, &m1m2);
			m1m2 &= ((core == PHY_CORE_0) ? 0x00ff : 0xff00);
			m1m2 |= ((core == PHY_CORE_0) ?
			         (pi_nphy->nphy_txpwrindex[core].bbmult << 8) :
			         (pi_nphy->nphy_txpwrindex[core].bbmult << 0));
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m1m2);

			if (restore_cals) {
				/* iqLoCaltbl a[01]/b[01] for both OFDM & BPHY */
				wlc_phy_table_write_nphy(pi, 15, 2, (80 + 2*core), 16,
				    (void *)&pi_nphy->nphy_txpwrindex[core].iqcomp_a);
				wlc_phy_table_write_nphy(pi, 15, 2, (88 + 2*core), 16,
				    (void *)&pi_nphy->nphy_txpwrindex[core].iqcomp_a);

				/* iqLoCaltbl diq[01] for both OFDM & BPHY */
				wlc_phy_table_write_nphy(pi, 15, 1, (85 + core), 16,
				    &pi_nphy->nphy_txpwrindex[core].locomp);
				wlc_phy_table_write_nphy(pi, 15, 1, (93 + core), 16,
				    (void *)&pi_nphy->nphy_txpwrindex[core].locomp);
			}

			/* restore txpwrctrl to whatever enable state it currently
			 * has.
			 */
			wlc_phy_txpwrctrl_enable_nphy(pi, pi->nphy_txpwrctrl);

			pi_nphy->nphy_txpwrindex[core].index_internal =
				pi_nphy->nphy_txpwrindex[core].index_internal_save;
		} else {
			/* override to desired power index */

			/* were we already forcing a different index?  if not,
			 * cache reg values per core to be available when power is
			 * restored to auto.
			 */
			if (pi_nphy->nphy_txpwrindex[core].index < 0) {
				/* AfeCtrlOverride */
				phy_utils_mod_phyreg(pi, NPHY_AfectrlOverride1,
				            NPHY_REV3_AfectrlOverride_dac_gain_MASK,
				            pi_nphy->nphy_txpwrindex[core].AfectrlOverride);
				phy_utils_mod_phyreg(pi, NPHY_AfectrlOverride2,
				            NPHY_REV3_AfectrlOverride_dac_gain_MASK,
				            pi_nphy->nphy_txpwrindex[core].AfectrlOverride);

				/* AfeCtrlDacGain */
				pi_nphy->nphy_txpwrindex[core].AfeCtrlDacGain =
				        phy_utils_read_phyreg(pi,
					(core == PHY_CORE_0) ? NPHY_AfectrlDacGain1 :
					NPHY_AfectrlDacGain2);

				/* radio gains */
				wlc_phy_table_read_nphy(pi, 7, 1, (0x110 + core), 16,
				   &pi_nphy->nphy_txpwrindex[core].rad_gain);

				/* iqLoCaltbl bbmult (OFDM only, BPHY value should be same) */
				wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &tmpval);
				tmpval >>= ((core == PHY_CORE_0) ? 8 : 0);
				tmpval &= 0xff;
				pi_nphy->nphy_txpwrindex[core].bbmult = (uint8)tmpval;

				/* iqLoCaltbl a[01]/b[01] (OFDM only, BPHY value should be same) */
				wlc_phy_table_read_nphy(pi, 15, 2, (80 + 2*core), 16,
				   (void *)&pi_nphy->nphy_txpwrindex[core].iqcomp_a);

				/* iqLoCaltbl diq[01] (OFDM only, BPHY value should be same) */
				wlc_phy_table_read_nphy(pi, 15, 1, (85 + core), 16,
				   (void *)&pi_nphy->nphy_txpwrindex[core].locomp);

				pi_nphy->nphy_txpwrindex[core].index_internal_save =
					pi_nphy->nphy_txpwrindex[core].index_internal;
			}

			tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
			wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

			/* Extract values from core[12]txpwrctl tables (TX gains,
			 * IQ comp and LO comp are sub tables within
			 * coreXtxpwrctl) currently the length of each sub_table
			 * is 128.
			 */

			/* read tx gain table:
			 * 28:16 - 2055 gain;  31:16 - 2056 gain
			 * 13: 8 - 4321 DAC gain
			 *  7: 0 - BB mult)
			 */
			wlc_phy_table_read_nphy(pi, txpwrctl_tbl, 1,
				(tx_ind0 + txpwrindex), 32, &txgain);

			rad_gain = (txgain >> 16) & ((1<<(32-16+1))-1);
			dac_gain = (txgain >>  8) & ((1<<(13- 8+1))-1);
			bbmult   = (txgain >>  0) & ((1<<(7 - 0+1))-1);

			/* apply tx gain components to various locations for
			 * overriding.
			 */

			/* DAC gain */
			phy_utils_mod_phyreg(pi, ((core == PHY_CORE_0) ?
			                     NPHY_AfectrlOverride1 :
				NPHY_AfectrlOverride2),
				NPHY_REV3_AfectrlOverride_dac_gain_MASK,
				NPHY_REV3_AfectrlOverride_dac_gain_MASK);
			phy_utils_write_phyreg(pi, (core == PHY_CORE_0) ?
			              NPHY_AfectrlDacGain1 : NPHY_AfectrlDacGain2,
			              dac_gain);

			/* radio gain */
			wlc_phy_table_write_nphy(pi, 7, 1, (0x110 + core), 16, &rad_gain);

			/* bbmult, core 1 and 2 vals are packed into common reg,
			 * so need to do a read-modify-write.
			 * bbmult for OFDM and BPHY are at offsets 87 and 95, resp.
			 */
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m1m2);
			m1m2 &= ((core == PHY_CORE_0) ? 0x00ff : 0xff00);
			m1m2 |= ((core == PHY_CORE_0) ? (bbmult << 8) : (bbmult << 0));

			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m1m2);

			wlc_phy_table_read_nphy(pi, 15, 1, 95, 16, &m1m2);
			m1m2 &= ((core == PHY_CORE_0) ? 0x00ff : 0xff00);
			m1m2 |= ((core == PHY_CORE_0) ? (bbmult << 8) : (bbmult << 0));

			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m1m2);

			/* read IQ comp table, and copy to override
			 * 19:10 -- "a" coeff
			 *  9: 0 -- "b" coeff
			 * IQ comp for OFDM and BPHY are at offsets 80 and 88, resp.
			 */
			wlc_phy_table_read_nphy(pi, txpwrctl_tbl, 1,
			   (iq_ind0 + txpwrindex), 32, &iqcomp);
			iqcomp_a = (iqcomp >> 10) & ((1<<(19-10+1))-1);
			iqcomp_b = (iqcomp >>  0) & ((1<<(9 - 0+1))-1);

			if (restore_cals) {
				regval[0] = (uint16)iqcomp_a;
				regval[1] = (uint16)iqcomp_b;
				wlc_phy_table_write_nphy(pi, 15, 2, (80 + 2*core), 16, regval);
				wlc_phy_table_write_nphy(pi, 15, 2, (88 + 2*core), 16, regval);
			}

			/* read LO comp table, and copy to override
			 * 15: 8 -- I offset
			 *  7: 0 -- Q offset
			 * LO comp for OFDM and BPHY are at offsets 85 and 93, resp.
			 */
			wlc_phy_table_read_nphy(pi, txpwrctl_tbl, 1,
			   (lo_ind0 + txpwrindex), 32, &locomp);
			if (restore_cals) {
				wlc_phy_table_write_nphy(pi, 15, 1, (85 + core), 16, &locomp);

				wlc_phy_table_write_nphy(pi, 15, 1, (93 + core), 16, &locomp);
			}

			/* NPHY_IPA : force loading RFPWR OFFSET */
			if (PHY_IPA(pi)) {
				wlc_phy_table_read_nphy(pi, (core == PHY_CORE_0 ?
					NPHY_TBL_ID_CORE1TXPWRCTL :
					NPHY_TBL_ID_CORE2TXPWRCTL), 1, 576 + txpwrindex, 32,
					&rfpwr_offset);

				PHY_REG_MOD_CORE(pi, NPHY, core, PapdEnable, gainDacRfValue,
				             (int16) rfpwr_offset);
				PHY_REG_MOD_CORE(pi, NPHY, core, PapdEnable, gainDacRfOverride, 1);
				PHY_CAL(("TxPwrindex %d : bbmult 0x%04x\t\t",
				           txpwrindex, m1m2));
				PHY_CAL(("PAPD offset for core %d : 0x%04x %d\n", core,
				           (int16) rfpwr_offset, (int16) rfpwr_offset));
			}

			/* pi_nphy->nphy_txpwrindex[core].index_internal = txpwrindex; */

			wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
		}

		/* update the per-core state of power index override */
		pi_nphy->nphy_txpwrindex[core].index = txpwrindex;
	}

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

#ifndef WLC_DISABLE_ACI
/** reset both aci and/or noise states, hardware and software */
void
wlc_phy_aci_noise_reset_nphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc)
{
	bool suspend;

	if ((pi->interf->curr_home_channel == (uint8) channel) && (disassoc == FALSE)) {
		/* same channel, not a disassoc, do not reset */
		PHY_ACI(("Not a disassoc! and CurCh = HomeCh = %d,"
			" so do not reset aci/noise state\n",
			pi->interf->curr_home_channel));
		return;
	}

	PHY_ACI(("%s: reset aci state = %d,"
		" reset noise state = %d, disassoc = %d, sp = %d\n",
		__FUNCTION__, clear_aci_state, clear_noise_state,
		disassoc, SCAN_RM_IN_PROGRESS(pi)));

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

	if (!suspend) {
		/* suspend mac */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (clear_aci_state) {
		wlc_phy_acimode_reset_nphy(pi);
	}

	if (clear_noise_state) {
		wlc_phy_noisemode_reset_nphy(pi);

		/* flag to clear noisemode reset state is
		 * valid only when chsp is changed and no
		 * scans are in prog and home chspecs are
		 * in sync.
		 * If not handled properly it will have
		 * corrupted reset vals in lieu of init
		 * vals.
		 */
		pi->do_noisemode_reset = TRUE;
	}

	if (!SCAN_RM_IN_PROGRESS(pi))
		pi->interf->curr_home_channel = (uint8) channel;

#if defined(WLSRVSDB) && !defined(WLC_DISABLE_ACI)
	if (disassoc && pi->srvsdb_state->srvsdb_active) {
		if (pi->srvsdb_state->sr_vsdb_channels[0] ==
			pi->interf->curr_home_channel) {
			pi->srvsdb_state->acimode_noisemode_reset_done[0] = FALSE;
		} else if (pi->srvsdb_state->sr_vsdb_channels[1] ==
			pi->interf->curr_home_channel) {
			pi->srvsdb_state->acimode_noisemode_reset_done[1] = FALSE;
		}
	}
#endif // endif

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_noisemode_upd_nphy(phy_info_t *pi)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (pi->interf->scanroamtimer != 0) {
		/* moving averages not updated, yet! */
		goto exit;
	}

	if (ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi)) {
		/* not associated */
		wlc_phy_noisemode_glitch_chk_adj_nphy(pi,
			pi->interf->noise.nphy_noise_noassoc_enter_th,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_up,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_dn);
	} else {
		/* associated */
		if ((pi->sh->now % NPHY_NOISE_CHECK_PERIOD) == 0) {
			wlc_phy_noisemode_glitch_chk_adj_nphy(pi,
				pi->interf->noise.nphy_noise_assoc_enter_th,
				pi->interf->noise.nphy_noise_assoc_glitch_th_up,
				pi->interf->noise.nphy_noise_assoc_glitch_th_dn);
		}

	}

	/* print glitch based noiseCal stats */
	if (ISNPHY(pi)) {
		PHY_ACI(("\n wlc_noise_reduction: aci ma is %d,\n"
			" ofdm_ma = %d, badplcp_ma = %d,\n"
			" crsminpwr = 0x%x, crsminpwr index = %d,\n"
			" init gain = 0x%x, channel is %d\n",
			pi->interf->aci.glitch_ma,
			pi->interf->noise.ofdm_glitch_ma,
			pi->interf->badplcp_ma, pi->interf->crsminpwru0,
			pi->interf->crsminpwr_index,
			pi->interf->init_gainb_core1,
			CHSPEC_CHANNEL(pi->radio_chanspec)));
	}

exit:
	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_aci_noise_upd_nphy(phy_info_t *pi)
{
	uint16 noise_assoc_rx_glitch_badplcp_enter_th;
	uint16 noise_noassoc_enter_th;
	uint16 noise_assoc_enter_th;
	bool suspend;

#ifdef WLSRVSDB
	uint8 aci_check_period = NPHY_ACI_CHECK_PERIOD;

	if (pi->srvsdb_state->srvsdb_active) {
		aci_check_period = 2 * NPHY_ACI_CHECK_PERIOD;
	}
#endif // endif

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
	}
	/* override bt priority */
	wlc_btcx_override_enable(pi);

	/* Updating home channel before going into aci scan */
	pi->interf->curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	noise_assoc_enter_th = pi->interf->noise.nphy_noise_assoc_enter_th;
	noise_noassoc_enter_th = pi->interf->noise.nphy_noise_noassoc_enter_th;
	noise_assoc_rx_glitch_badplcp_enter_th =
		pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th;

	if (pi->interf->scanroamtimer != 0) {
		goto end;
	}

#if defined(WLTEST) || defined(DBG_PHY_IOV)
	if ((ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi)) && (!pi->aci_nams))
#else
	if (ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi))
#endif // endif
	{
		/* not associated.  check for noise inband */
		wlc_phy_noisemode_glitch_chk_adj_nphy(pi,
			noise_noassoc_enter_th,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_up,
			pi->interf->noise.nphy_noise_noassoc_glitch_th_dn);
	} else {
		/* currently associated */
		if (!(pi->aci_state & ACI_ACTIVE)) {
			/* not in ACI mitigation mode */
#ifdef WLSRVSDB
			if ((pi->sh->now - pi->last_aci_call) >= aci_check_period) {
				pi->last_aci_call = pi->sh->now;
#else
			if (pi->sh->now % NPHY_ACI_CHECK_PERIOD == 0) {
#endif // endif
				PHY_ACI(("Interf Mode 4, pi->interf->aci.glitch_ma = %d,"
					" pi->interf->badplcp_ma = %d, sum = %d,"
					" crs_min_pwr = u 0x%x, l 0x%x, m 0x%x\n",
					pi->interf->aci.glitch_ma, pi->interf->badplcp_ma,
					pi->interf->aci.glitch_ma + pi->interf->badplcp_ma,
					phy_utils_read_phyreg(pi, NPHY_crsminpoweru0),
						phy_utils_read_phyreg(pi, NPHY_crsminpowerl0),
					phy_utils_read_phyreg(pi, NPHY_crsminpower0)));
				if ((pi->interf->aci.glitch_ma + pi->interf->badplcp_ma) >=
					noise_assoc_rx_glitch_badplcp_enter_th) {
					/* glitch count high, check aci */
					wlc_phy_acimode_upd_nphy(pi);
				}
			}

			if (pi->interf->aci.detect_total == 0) {
				/* aci scan didn't indicate aci present */
				/* so, check for excessive crs glitch */

				wlc_phy_noisemode_glitch_chk_adj_nphy(pi,
					noise_assoc_enter_th,
					pi->interf->noise.nphy_noise_assoc_glitch_th_up,
					pi->interf->noise.nphy_noise_assoc_glitch_th_dn);

			} else {
				/* ACI mitigation just occurred. reset state */
				pi->interf->noise.noise_glitch_high_detect_total = 0;
				pi->interf->noise.noise_glitch_low_detect_total = 0;
			}
		} else {
			/* already active in ACI mitigation mode, check to get out */
#ifdef WLSRVSDB
			if ((pi->sh->now - pi->last_aci_check_time) >= pi->aci_exit_check_period) {
				pi->last_aci_check_time = pi->sh->now;
#else
			if (((pi->sh->now - pi->aci_start_time)
				% pi->aci_exit_check_period) == 0) {
#endif // endif
				wlc_phy_acimode_upd_nphy(pi);
			}

			if (pi->interf->aci.detect_total >= 0) {
				/* in ACI mitigation, done transitioning,
				 * check for inband noise
				 */
				wlc_phy_noisemode_glitch_chk_adj_nphy(pi,
					noise_assoc_enter_th,
					pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up,
					pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn);
			}
		} /* already in mitigation mode, check to exit */
	} /* associated */
end:
	/* remove override */
	wlc_phy_btcx_override_disable(pi);
	/* unsuspend mac */
	if (!suspend) {
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_noisemode_glitch_chk_adj_nphy(phy_info_t *pi, uint16 noise_enter_th,
	uint16 noise_glitch_th_up, uint16 noise_glitch_th_dn)
{
#ifdef BPHY_DESENSE
	if (((pi->interf->noise.ofdm_glitch_ma + pi->interf->noise.ofdm_badplcp_ma) >
		noise_enter_th)	||
		((pi->interf->noise.bphy_glitch_ma + pi->interf->noise.bphy_badplcp_ma) >
		BPHY_DESENSE_NOISE_ENTER_TH))
#else
	if ((pi->interf->noise.ofdm_glitch_ma + pi->interf->noise.ofdm_badplcp_ma) >
		noise_enter_th)
#endif // endif
	{
		/* glitch count is high, could be due to inband noise */
		pi->interf->noise.noise_glitch_high_detect_total++;
		pi->interf->noise.noise_glitch_low_detect_total = 0;
	} else {
		/* glitch count not high */
		pi->interf->noise.noise_glitch_high_detect_total = 0;
		pi->interf->noise.noise_glitch_low_detect_total++;
	}
	/* glitch stats */
	PHY_CAL(("ofdm %d %d | bphy %d %d\n",
		pi->interf->noise.ofdm_glitch_ma, pi->interf->noise.ofdm_badplcp_ma,
		pi->interf->noise.bphy_glitch_ma, pi->interf->noise.bphy_badplcp_ma));

	if (pi->interf->noise.noise_glitch_high_detect_total >=
		noise_glitch_th_up) {

		uint8 raise = 0;
		/* raise
		 * 1	:  apply ofdm desense and remove bphy desense
		 * 2	:  apply bphy desense and remove ofdm desense
		 * 3	:  apply both ofdm and bphy desense
		 * 0	: remove both ofdm and bphy desense
		 */

		/* we have more than noise_glitch_th_up ofdm
		 * glitches in a row. so, let's try raising the
		 * inband noise immunity
		 */

		if ((pi->interf->noise.ofdm_glitch_ma + pi->interf->noise.ofdm_badplcp_ma) >
			noise_enter_th) {
			raise |= 1;
		}
#ifdef BPHY_DESENSE
		if ((pi->interf->noise.bphy_glitch_ma + pi->interf->noise.bphy_badplcp_ma) >
			BPHY_DESENSE_NOISE_ENTER_TH) {
			raise |= 2;
		}
#endif // endif

		wlc_phy_noisemode_set_nphy(pi, raise);

		if (pi->interf->noise.noise_glitch_high_detect_total)
			pi->interf->noise.noise_glitch_high_detect_total = 0;
	} else {
		/* check to see if we can lower noise immunity */
		if (pi->interf->noise.noise_glitch_low_detect_total >=
			noise_glitch_th_dn) {

			/* we have more than noise_glitch_th_dn
			 * non detects in a row.  try lowering noise threshold
			 */
			wlc_phy_noisemode_set_nphy(pi, 0);

			if (pi->interf->noise.noise_glitch_low_detect_total)
				pi->interf->noise.noise_glitch_low_detect_total = 0;
		}
	}

}
#endif /* Compiling out ACI code */

static void
wlc_phy_aci_noise_store_values_nphy(phy_info_t *pi)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* store off orig values for aci/noise mitigation */
	pi->interf->init_gain_code_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2057);
	pi->interf->init_gain_code_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeA2057);
	pi->interf->init_gain_codeb_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
	pi->interf->init_gain_codeb_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057);
	pi->interf->crsminpwrthld_20L_base =
		(uint16) (phy_utils_read_phyreg(pi, NPHY_crsminpowerl0) & 0xff);
#ifndef WLC_DISABLE_ACI
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (pi->aci_state & ACI_ACTIVE) {
			pi->interf->max_hpvga_acion_2G =
			(pi->interf->init_gain_codeb_core1_stored >>
			NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);

		} else {
			pi->interf->max_hpvga_acioff_2G =
			(pi->interf->init_gain_codeb_core1_stored >>
			NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);
		}
	} else {
		pi->interf->max_hpvga_acioff_5G =
			(pi->interf->init_gain_codeb_core1_stored >>
			NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);
	}
#endif /* Compiling out ACI code */

#ifndef WLC_DISABLE_ACI
	pi->interf->crsminpwrthld_20L_stored =
		(uint16) (phy_utils_read_phyreg(pi, NPHY_crsminpowerl0) & 0xff);
	pi->interf->crsminpwrthld_20U_stored =
		(uint16) (phy_utils_read_phyreg(pi, NPHY_crsminpoweru0) & 0xff);
#ifdef BPHY_DESENSE
	pi->interf->bphy_crsminpwrthld_stored =
		(uint16) (phy_utils_read_phyreg(pi, NPHY_bphycrsminpower0) & 0xff);
#endif // endif

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, pi->pubpi->phy_corenum, 0x106, 16,
		&(pi->interf->init_gain_table_stored[0]));

	pi->interf->clip1_hi_gain_code_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1clipHiGainCodeA2057);
	pi->interf->clip1_hi_gain_code_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2clipHiGainCodeA2057);
	pi->interf->clip1_hi_gain_codeb_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1clipHiGainCodeB2057);
	pi->interf->clip1_hi_gain_codeb_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2clipHiGainCodeB2057);

	pi->interf->nb_clip_thresh_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1nbClipThreshold);
	pi->interf->nb_clip_thresh_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2nbClipThreshold);

	wlc_phy_table_read_nphy(pi, 4, 4, 0x10, 16,
		pi->interf->init_ofdmlna2gainchange_stored);
	wlc_phy_table_read_nphy(pi, 4, 4, 0x50, 16,
		pi->interf->init_ccklna2gainchange_stored);

	/* clipLO gain */
	pi->interf->clip1_lo_gain_code_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1cliploGainCodeA2057);
	pi->interf->clip1_lo_gain_code_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2cliploGainCodeA2057);
	pi->interf->clip1_lo_gain_codeb_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1cliploGainCodeB2057);
	pi->interf->clip1_lo_gain_codeb_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2cliploGainCodeB2057);
	pi->interf->w1_clip_thresh_core1_stored =
		phy_utils_read_phyreg(pi, NPHY_Core1clipwbThreshold2057);
	pi->interf->w1_clip_thresh_core2_stored =
		phy_utils_read_phyreg(pi, NPHY_Core2clipwbThreshold2057);

	pi->interf->radio_2057_core1_rssi_nb_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE0) & 0x60;
	pi->interf->radio_2057_core2_rssi_nb_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE1) & 0x60;
	pi->interf->radio_2057_core1_rssi_wb1a_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0) & 0xc;
	pi->interf->radio_2057_core2_rssi_wb1a_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1) & 0xc;
	pi->interf->radio_2057_core1_rssi_wb1g_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0) & 0x3;
	pi->interf->radio_2057_core2_rssi_wb1g_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1) & 0x3;
	pi->interf->radio_2057_core1_rssi_wb2_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE0) & 0xc0;
	pi->interf->radio_2057_core2_rssi_wb2_gc_stored =
		phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE1) & 0xc0;

	if (!(NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV))) {
		pi->interf->energy_drop_timeout_len_stored =
			phy_utils_read_phyreg(pi, NPHY_energydroptimeoutLen);
	}

	pi->interf->ed_crs40_assertthld0_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs40AssertThresh0);
	pi->interf->ed_crs40_assertthld1_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs40AssertThresh1);
	pi->interf->ed_crs40_deassertthld0_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs40DeassertThresh0);
	pi->interf->ed_crs40_deassertthld1_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs40DeassertThresh1);
	pi->interf->ed_crs20L_assertthld0_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20LAssertThresh0);
	pi->interf->ed_crs20L_assertthld1_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20LAssertThresh1);
	pi->interf->ed_crs20L_deassertthld0_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20LDeassertThresh0);
	pi->interf->ed_crs20L_deassertthld1_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20LDeassertThresh1);

	pi->interf->ed_crs20U_assertthld0_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20UAssertThresh0);
	pi->interf->ed_crs20U_assertthld1_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20UAssertThresh1);
	pi->interf->ed_crs20U_deassertthld0_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20UDeassertThresh0);
	pi->interf->ed_crs20U_deassertthld1_stored=
		phy_utils_read_phyreg(pi, NPHY_ed_crs20UDeassertThresh1);

#endif /* Compiling out ACI code */

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

#ifndef WLC_DISABLE_ACI
void
wlc_phy_noisemode_reset_nphy(phy_info_t *pi)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (NREV_GE(pi->pubpi->phy_rev, 16)) {

		/* software reset */
		pi->interf->noise.newgain_initgain = pi->interf->init_gain_code_core1_stored;
		pi->interf->noise.newgain_rfseq = pi->interf->init_gain_table_stored[0];
		pi->interf->noise.newgainb_initgain = pi->interf->init_gain_codeb_core1_stored;
		pi->interf->noise.changeinitgain = FALSE;

		pi->interf->noise.noise_glitch_low_detect_total = 0;
		pi->interf->noise.noise_glitch_high_detect_total = 0;

		/* set base crsminpwr table index for interference mitigation */
		pi->interf->crsminpwr_index = 0;
		pi->interf->crsminpwr_index_core1 = 0;

		pi->interf->crsminpwr_index_aci_on = 0;
		pi->interf->crsminpwr_index_aci_off = 0;

		pi->interf->noise.nphy_noise_noassoc_glitch_th_up =
			NPHY_NOISE_NOASSOC_GLITCH_TH_UP;
		pi->interf->noise.nphy_noise_noassoc_glitch_th_dn =
			NPHY_NOISE_NOASSOC_GLITCH_TH_DN;
		pi->interf->noise.nphy_noise_assoc_glitch_th_up =
			NPHY_NOISE_ASSOC_GLITCH_TH_UP;
		pi->interf->noise.nphy_noise_assoc_glitch_th_dn =
			NPHY_NOISE_ASSOC_GLITCH_TH_DN;
		pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up =
			NPHY_NOISE_ASSOC_ACI_GLITCH_TH_UP;
		pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn =
			NPHY_NOISE_ASSOC_ACI_GLITCH_TH_DN;
		pi->interf->noise.nphy_noise_assoc_enter_th = NPHY_NOISE_ASSOC_ENTER_TH_REV7;
		pi->interf->noise.nphy_noise_noassoc_enter_th = NPHY_NOISE_NOASSOC_ENTER_TH_REV7;
		pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th =
			NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH_REV7;
		pi->interf->noise.nphy_noise_noassoc_crsidx_incr =
			NPHY_NOISE_NOASSOC_CRSIDX_INCR;
		pi->interf->noise.nphy_noise_assoc_crsidx_incr =
			NPHY_NOISE_ASSOC_CRSIDX_INCR;
		pi->interf->noise.nphy_noise_crsidx_decr =
			NPHY_NOISE_CRSIDX_DECR;
#ifdef BPHY_DESENSE
		pi->interf->noise.nphy_bphynoise_crsidx_incr =
			BPHY_DESENSE_NOISE_CRSIDX_INCR;
		pi->interf->noise.nphy_bphynoise_crsidx_decr =
			BPHY_DESENSE_NOISE_CRSIDX_DECR;
#endif // endif

		/* aci and noise state will be reset here */
		wlc_phy_aci_noise_shared_reset_nphy(pi);

	} else if (NREV_LE(pi->pubpi->phy_rev, 15)) {

		pi->interf->noise.bphy_thres.desense_hi_step = PHY_OFDM_BPHY_CRSIDX_INCR_HI;
		pi->interf->noise.bphy_thres.desense_lo_step = PHY_OFDM_BPHY_CRSIDX_INCR_LO;
		pi->interf->noise.bphy_thres.undesense_step = PHY_OFDM_BPHY_CRSIDX_DECR;
		pi->interf->noise.bphy_thres.undesense_wait = PHY_NOISE_ASSOC_UNDESENSE_WAIT;
		pi->interf->noise.bphy_thres.undesense_window = PHY_NOISE_ASSOC_UNDESENSE_WINDOW;
		pi->interf->noise.bphy_thres.high_detect_thresh = PHY_NOISE_HIGH_DETECT_TH;
		pi->interf->noise.bphy_thres.glitch_badplcp_high_th = PHY_BPHY_NOISE_ASSOC_HIGH_TH;
		pi->interf->noise.bphy_thres.glitch_badplcp_low_th = PHY_BPHY_NOISE_ASSOC_LOW_TH;

		pi->interf->noise.ofdm_thres.desense_hi_step = PHY_OFDM_BPHY_CRSIDX_INCR_HI;
		pi->interf->noise.ofdm_thres.desense_lo_step = PHY_OFDM_BPHY_CRSIDX_INCR_LO;
		pi->interf->noise.ofdm_thres.undesense_step = PHY_OFDM_BPHY_CRSIDX_DECR;
		pi->interf->noise.ofdm_thres.undesense_wait = PHY_NOISE_ASSOC_UNDESENSE_WAIT;
		pi->interf->noise.ofdm_thres.undesense_window = PHY_NOISE_ASSOC_UNDESENSE_WINDOW;
		pi->interf->noise.ofdm_thres.high_detect_thresh = PHY_NOISE_HIGH_DETECT_TH;
		if (PUB_NOT_ASSOC(pi)) {
			pi->interf->noise.ofdm_thres.glitch_badplcp_high_th =
				PHY_OFDM_NOISE_NOASSOC_HIGH_TH;
			pi->interf->noise.ofdm_thres.glitch_badplcp_low_th =
				PHY_OFDM_NOISE_NOASSOC_LOW_TH;
		} else {
			pi->interf->noise.ofdm_thres.glitch_badplcp_high_th =
				PHY_OFDM_NOISE_ASSOC_HIGH_TH;
			pi->interf->noise.ofdm_thres.glitch_badplcp_low_th =
				PHY_OFDM_NOISE_ASSOC_LOW_TH;
		}

		pi->interf->noise.ofdm_desense = 0;
		pi->interf->noise.bphy_desense = 0;
		wlc_phy_bphy_ofdm_noise_hw_set_nphy(pi);

		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, pi->pubpi->phy_corenum, 0x106, 16,
			&(pi->interf->noise.save_initgain_rfseq[0]));
	}

	if (NREV_LE(pi->pubpi->phy_rev, 15)) {
		pi->interf->bphy_desense_lut = NPHY_bphy_desense_aci_off_lut_rev7to15;
		pi->interf->bphy_desense_lut_size =
			sizeof(NPHY_bphy_desense_aci_off_lut_rev7to15)/
			sizeof(bphy_desense_info_t);
		pi->interf->bphy_min_sensitivity = NPHY_BPHY_MIN_SENSITIVITY_REV7TO15;
		pi->interf->ofdm_min_sensitivity = NPHY_OFDM_MIN_SENSITIVITY_REV7TO15;
	}

	pi->interf->store_values = FALSE;

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/** noise mitigation mode, hardware and software change */
static void
wlc_phy_noisemode_set_nphy(phy_info_t *pi, uint8 raise)
{
	/* update noise stats */
	wlc_phy_noise_adj_thresholds_nphy(pi, raise);

	/* apply noise stats */
	wlc_phy_aci_noise_shared_hw_set_nphy(pi, FALSE, FALSE);

	/* update s/w state */
	wlc_phy_noise_sw_set_nphy(pi);

	/* print noiseCal stats */
	PHY_ACI(("wlc_phy_noisemode_set_nphy: raise = %d,"
		" crsminpwr = 0x%x, crsminpwr index = %d,"
		" init gain = 0x%x\n", raise, pi->interf->crsminpwru0,
		pi->interf->crsminpwr_index, pi->interf->init_gain_core1));
}

/** noise mitigation hardware modifications */
static void
wlc_phy_noise_adj_thresholds_nphy(phy_info_t *pi, uint8 raise)
{
	/* formula 2057: round(2^3*log2((10.^(([-91:0.25:-60]-3+68-30)/10)*50)*(2^9/0.4)^2)) */
	uint16 crsminpwr_array_rev_7[] = {
		61, 62, 63, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 71, 71,
		72, 73, 73, 74, 75, 75, 76, 77, 77, 78, 79, 79, 80, 81, 81, 82, 83,
		83, 84, 85, 85, 86, 87, 87, 88, 89, 89, 90, 91, 91, 92, 93, 93, 94,
		95, 95, 96, 97, 97, 98, 99, 99, 100, 101, 101, 102, 103, 103, 104, 105, 105,
		106, 107, 107, 108, 109, 109, 110, 111, 111, 112, 113, 113, 114, 115, 115, 116, 117,
		117, 118, 119, 119, 120, 121, 121, 122, 123, 123, 124, 125, 125, 126, 127, 127, 128,
		129, 129, 130, 131, 131, 132, 133, 133, 134, 135, 135, 136, 137, 137, 138, 139, 139,
		140, 141, 141, 142, 143, 143, 144};

#ifdef BPHY_DESENSE
	/* a6 = -92 dBm, b6 = -86, c6 = -82, d6 = -78, e6 = -75, f6 = -72 */
	uint16 bphy_crsminpwr_array_rev_19[] = {
		0x46, 0x56, 0x66, 0x76, 0x86, 0xa6, 0xae, 0xb6,
		0xbe, 0xc6, 0xce, 0xd6, 0xde, 0xe6, 0xee, 0xf6};

	uint16 max_bphy_crsminpwr_idx;
	uint16 bphy_min_crsminpwr_idx;
	uint16 bphy_min_crsminpwr = 0;
	uint16 *bphy_crsminpwr_array_ptr;
	uint16 bphy_crsminpwr_array_max_index;
#endif // endif
	uint16 min_crsminpwr[NPHY_CORE_NUM] = {0, 0};
	uint16 curgain, origgain;
	int16 newgain;
	uint16 *crsminpwr_array_ptr;
	uint16 crsminpwr_array_max_index;
	uint16 tempgain;
	uint16 max_crsminpwr_idx;
	uint16 min_crsminpwr_idx;
	uint8 lna1_backoff_db = 0, lna2_backoff_db = 0, curr_initgain = 69;
	int8 biq0_backoff_db = 0, biq1_backoff_db = 0, koffset = 0;
	uint16 crsmin_rssi = 0x7f;
	int16 crsmin_rssi_index = 123;

	/* initialize minpwr arrays and max traversal indices */
	crsminpwr_array_ptr = crsminpwr_array_rev_7;
#ifdef BPHY_DESENSE
	bphy_crsminpwr_array_ptr = bphy_crsminpwr_array_rev_19;
#endif // endif
	if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV + 1)) {
		crsminpwr_array_max_index =
		NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_17;
	} else {
		/* max_index is 45, translating to ~-80 dBm */
		crsminpwr_array_max_index =
		NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_7;
	}
#ifdef BPHY_DESENSE
	bphy_crsminpwr_array_max_index = BPHY_DESENSE_CRSMINPWR_ARRAY_MAX_INDEX;
#endif // endif

	if (pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
	pi->sh->interference_mode == NON_WLAN) {

		/* to reduce crs glitches, we will try to use crs min pwr */
		/* threshold for ofdm in 1 dbm steps, up to a max value (-83 dbm?) */

		/* APPLY DESENSE */
		if (raise) {

		/* ofdm desense */
		if (raise & 1) {
			/* remember default hpvga (bq1) gain (for crsmin_limit) */
			tempgain = (uint16)
				  ((phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057))
				      >> NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);

			max_crsminpwr_idx = pi->interf->crsminpwr_index;

			/* increase the index by 1, make sure it's in bounds */
			if (max_crsminpwr_idx <	crsminpwr_array_max_index) {
				if (PUB_NOT_ASSOC(pi)) {
					/* raise by 4 dB (0.25 dB steps) */
					pi->interf->crsminpwr_index +=
					pi->interf->noise.nphy_noise_noassoc_crsidx_incr;
				} else {
					/* raise by 2 dB (0.25 dB steps) */
					pi->interf->crsminpwr_index +=
					pi->interf->noise.nphy_noise_assoc_crsidx_incr;
				}
				if (pi->interf->crsminpwr_index >
				    crsminpwr_array_max_index) {
					pi->interf->crsminpwr_index =
					crsminpwr_array_max_index;
				}
			} else {
				/* maxed out crsminpwr index, reduce init gain */
				/* reducing gain by 3 db with last stage elements */

				newgain = ((phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057) &
				NPHY_Core1InitGainCodeB2057_InitBiQ1Index_MASK) >>
				NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT) - 1;

				if (PUB_NOT_ASSOC(pi)) {
					--newgain;
				}

				/* if newgain is less than 0, can't go any lower */
				if (newgain < 0)
					newgain = 0;

				/* for phyrev 3 and later, limit init gain reduction */
				/* based on rssi value */
				if (!(PUB_NOT_ASSOC(pi))) {
					wlc_phy_noise_limit_rfgain_nphy(pi, &newgain);
				}

				pi->interf->noise.newgain_initgain = newgain; /* CY added this */

				/* modify tx to rx rfseq */
				wlc_phy_table_read_nphy(pi, 7, 1, 0x106, 16, &curgain);

				if (PUB_NOT_ASSOC(pi)) {
					newgain = (curgain>> 12) - 2;
				} else {
					newgain = (curgain>> 12) - 1;
				}
				if (newgain < 0)
					newgain = 0;

				if (!(PUB_NOT_ASSOC(pi))) {
					/* can we be associated but not have an rssi value? */
					wlc_phy_noise_limit_rfgain_nphy(pi, &newgain);
				}
				if (tempgain != newgain)
					pi->interf->noise.changeinitgain = TRUE;
				else
					pi->interf->noise.changeinitgain = FALSE;

				/* update hpgva/bq1 to new gain (for crsmin_limit) */
				tempgain = newgain;

				/* map to full gain code */
				newgain = (newgain << 12) | (curgain & 0xfff);

				pi->interf->noise.newgain_rfseq = newgain;
			}

			/* rev 7 changes crsminpwr only, limit the change */
			/* based on rssi value */
			/* concern:  bphy doesn't get limited the same way */
			if (!(PUB_NOT_ASSOC(pi))) {
				wlc_phy_noise_limit_crsmin_nphy(pi, tempgain);
			}
			/* rev 16,17 crsminpwr is limietd by RSSI */
			/* Leverage 43241 experiments, RB15125, SWWLAN-34216 */
			if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV + 1) &&
			(pi->interf->rssi != WLC_RSSI_INVALID)) {
				if (pi->aci_state & ACI_ACTIVE) {
					lna1_backoff_db = 5;
					lna2_backoff_db = 8;
					biq0_backoff_db = 3;
					biq1_backoff_db = -9;
				}
				curr_initgain = curr_initgain - lna1_backoff_db - lna2_backoff_db
				        - biq0_backoff_db - biq1_backoff_db;
				if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
				!(pi->aci_state & ACI_ACTIVE)) {
					koffset = -11;
				}
				/* crsminpwr = 8*log2(16*iqpwr_8bit) + Koffset; and
				rssi_dbm=10*log10(((16*iqpwr_8bit)^(0.5))*0.4/512)^2)/50)+
				30-initgain
				so, crsminpwr = (0.8)log10(2)* (rssi_dbm + initgain - K) where
				K = 10*log10((0.4/512)^2/50) + 30 = -49.1339. Therefore,
				crsminpwr = (0.8*log2(10)*2^10)*
				((rssi_dbm + initgain - K)*2^6) / (2^(10+6)) + Koffset
				*/
				crsmin_rssi = (uint16) (((2721*((((int32)(pi->interf->rssi +
				curr_initgain))<<6) + 3145)) >> 16) + koffset);
				crsmin_rssi_index = (int16)((crsmin_rssi - 61)/8 * 12);
				PHY_ACI(("link_rssi=%d, crsmin_rssi=%d, crsmin_rssi_index=%d\n",
				pi->interf->rssi, crsmin_rssi, crsmin_rssi_index));
				if (crsmin_rssi_index < 0)
					crsmin_rssi_index = 0;
				if (pi->interf->crsminpwr_index > crsmin_rssi_index) {
					pi->interf->crsminpwr_index = crsmin_rssi_index;
				}
			}
		}
#ifdef BPHY_DESENSE
		/* bphy desense */
		if (raise & 2) {
			/* remember default hpvga (bq1) gain (for crsmin_limit) */
			max_bphy_crsminpwr_idx = pi->interf->bphy_crsminpwr_index;
			/* increase the index by 1, make sure it's in bounds */
			if (max_bphy_crsminpwr_idx <
				bphy_crsminpwr_array_max_index) {
					/* raise by 6 dB (3 dB steps) */
				pi->interf->bphy_crsminpwr_index +=
				pi->interf->noise.nphy_bphynoise_crsidx_incr;
				if (pi->interf->bphy_crsminpwr_index >
				    bphy_crsminpwr_array_max_index) {
					pi->interf->bphy_crsminpwr_index =
					bphy_crsminpwr_array_max_index;
				}
			}
		}
#endif /* bphy_desense */
		} /* applied desense */

		/* REMOVE DESENSE */
		if (raise != 3) {

			/* ofdm recovers here */
			if (!(raise & 1)) {

			/* lower */
			if (pi->aci_state & ACI_ACTIVE) {
				origgain = pi->interf->max_hpvga_acion_2G;
			} else {
				origgain = 0;
			}
			newgain = (uint16)
				((phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057))
				>> NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);
			if (newgain < origgain) {
				/* gain currently needs to be raised */
				newgain++;
			} else {
				/* init gain hasn't changed.  check crsminpwr */
				min_crsminpwr_idx = pi->interf->crsminpwr_index;
				if (min_crsminpwr_idx >
				pi->interf->noise.nphy_noise_crsidx_decr) {
					pi->interf->crsminpwr_index -=
					pi->interf->noise.nphy_noise_crsidx_decr;
				} else {
					pi->interf->crsminpwr_index = 0;
				}
			}
			if (newgain <= origgain)
				pi->interf->noise.changeinitgain = TRUE;
			else
				pi->interf->noise.changeinitgain = FALSE;

			pi->interf->noise.newgain_initgain = newgain; /* CY added this */

			/* modify tx to rx rfseq */
			if (pi->aci_state & ACI_ACTIVE) {
				origgain = pi->interf->max_hpvga_acion_2G;
			} else {
				origgain = pi->interf->init_gain_table_stored[0] >> 12;
			}

			wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, 0x106, 16, &curgain);

			newgain = (curgain >> 12) + 1;

			if (newgain > origgain)
				newgain = origgain;

			newgain = (newgain << 12) | (curgain & 0xfff);
				pi->interf->noise.newgain_rfseq = newgain;
			}

#ifdef BPHY_DESENSE
			/* bphy recovers here */
			if (!(raise & 2)) {
				bphy_min_crsminpwr_idx = pi->interf->bphy_crsminpwr_index;
				if (bphy_min_crsminpwr_idx >=
				pi->interf->noise.nphy_bphynoise_crsidx_decr) {
					pi->interf->bphy_crsminpwr_index -=
					pi->interf->noise.nphy_bphynoise_crsidx_decr;
				}
			}
#endif // endif
		} /* recovered from desense */

		/* cap minpwrs such that they shouldn't be less than init/periodic NoiseCal value */
		if (crsminpwr_array_ptr[pi->interf->crsminpwr_index] <
		    pi->interf->crsminpwrthld_20U_stored) {
			min_crsminpwr[0] = pi->interf->crsminpwrthld_20U_stored;
		} else {
			min_crsminpwr[0] = crsminpwr_array_ptr[pi->interf->crsminpwr_index];
		}

		pi->interf->noise.newcrsminpwr_20U = min_crsminpwr[0];
		pi->interf->noise.newcrsminpwr_20L = min_crsminpwr[0];

#ifdef BPHY_DESENSE
		/* limit bphy desense */
		if (bphy_crsminpwr_array_ptr[pi->interf->bphy_crsminpwr_index] <
			BPHY_DESENSE_CRSMINPWR_BASELINE) {
			bphy_min_crsminpwr = BPHY_DESENSE_CRSMINPWR_BASELINE;
		} else {
			bphy_min_crsminpwr =
			bphy_crsminpwr_array_ptr[pi->interf->bphy_crsminpwr_index];
		}

		pi->interf->noise.newbphycrsminpwr = bphy_min_crsminpwr;
#endif /* defined(BPHY_DESENSE) */
	} else {
		/* currently no changes for other revs */
	}
}

/** noise mitigation hardware modifications */
static void
wlc_phy_noise_limit_crsmin_nphy(phy_info_t *pi, uint16 gain)
{
#ifdef STA
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	int16 rssi_avg = pi_nphy->intf_rssi_avg;
	int16 *pwr_array;
	int16 max_hpvga;
	int16 base_rssi_avg;
	int16 idx;
	/* formula 2057: round(2^3*log2((10.^(([-90:0.25:-58]-3+68-30)/10)*50)*(2^9/0.4)^2)) */
	/* if this formula changes, then the constants here need to change too */
	/* for phyrev 7 or greater */

	if (!NPHY_INTF_RSSI_ENAB(pi))
		return;

	pwr_array = pi_nphy->crsmin_pwr_aci2g;

	if (!(pi->aci_state == ACI_ACTIVE)) {
		max_hpvga = pi->interf->max_hpvga_acioff_2G;
		base_rssi_avg = pi_nphy->crsmin_rssi_avg_acioff_2G;
	} else {
		max_hpvga = pi->interf->max_hpvga_acion_2G;
		base_rssi_avg = pi_nphy->crsmin_rssi_avg_acion_2G;
	}

	PHY_TRACE(("%s: rssi_avg = %d input_gain = %d max_hpvga = %d base_rssi_avg = %d \n",
		__FUNCTION__, rssi_avg, gain, max_hpvga, base_rssi_avg));

	if (gain == max_hpvga) {

		if (rssi_avg  < base_rssi_avg)
				rssi_avg = base_rssi_avg - 1;

		idx = ((rssi_avg - base_rssi_avg) + (PHY_CRSMIN_RANGE))  / PHY_CRSMIN_RANGE;

		if (idx > PHY_CRSMIN_IDX_MAX)
			return;

		PHY_TRACE(("%s: pi->interf->crsminpwr_index = %d pwr_array[%d] = %d \n",
			__FUNCTION__, pi->interf->crsminpwr_index, idx, pwr_array[idx]));

		if (pi->interf->crsminpwr_index > pwr_array[idx]) {
			pi->interf->crsminpwr_index = pwr_array[idx];
		}
	}
#endif /* STA */
}

/** noise mitigation hardware modifications */
static void
wlc_phy_noise_limit_rfgain_nphy(phy_info_t *pi, int16 *newgain)
{
#ifdef STA

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	int16 rssi_avg = pi_nphy->intf_rssi_avg;
	int16 max_hpvga;
	int16 base_rssi;
	int16 maxrssi;
	int16 idx;

	if (!NPHY_INTF_RSSI_ENAB(pi))
		return;

	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (pi->aci_state == ACI_ACTIVE) {
			/* 5G channel, currently no ACI mitigation is allowed */
			return;
		}
	}

	if (!(pi->aci_state == ACI_ACTIVE)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			max_hpvga = pi->interf->max_hpvga_acioff_2G;
			base_rssi = pi_nphy->rfgain_rssi_avg_acioff_2G;
			maxrssi = pi_nphy->rfgain_rssi_avg_acioff_2G_max;
		} else {
			max_hpvga = pi->interf->max_hpvga_acioff_5G;
			base_rssi = pi_nphy->rfgain_rssi_avg_acioff_5G;
			maxrssi = pi_nphy->rfgain_rssi_avg_acioff_5G_max;
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			max_hpvga = pi->interf->max_hpvga_acion_2G;
			base_rssi = pi_nphy->rfgain_rssi_avg_acion_2G;
			maxrssi = pi_nphy->rfgain_rssi_avg_acion_2G_max;
		} else {
			max_hpvga = pi->interf->max_hpvga_acion_5G;
			base_rssi = pi_nphy->rfgain_rssi_avg_acion_5G;
			maxrssi = pi_nphy->rfgain_rssi_avg_acion_5G_max;
		}
	}

	PHY_TRACE(("%s:rssi_avg= %d max_hpvga= %d base_rssi= %d maxrssi= %d newgain= %d\n",
		__FUNCTION__, rssi_avg, max_hpvga, base_rssi, maxrssi, *newgain));

	if (rssi_avg > maxrssi)
		return;

	if (rssi_avg < base_rssi)
		rssi_avg = base_rssi - 1;

	idx = (rssi_avg - base_rssi + PHY_CRSMIN_RANGE) / PHY_CRSMIN_RANGE;

	max_hpvga -= idx;

	if (max_hpvga >= 0 && (*newgain < max_hpvga)) {
		*newgain = max_hpvga;

		PHY_TRACE(("%s:newgain-> idx = %d input_newgain = %d max_hpvga = %d\n",
			__FUNCTION__, idx,  *newgain, max_hpvga));
	}

#endif /* STA */
}

/** noise mitigation software modification */
static void
wlc_phy_noise_sw_set_nphy(phy_info_t *pi)
{
	int16 newgain;
	uint i;

	/* only do something for 4322 and above */
	if (((pi->sh->interference_mode == WLAN_AUTO_W_NOISE) ||
		(pi->sh->interference_mode == NON_WLAN))) {

		/* store off the values */
		pi->interf->crsminpwrl0 =
			(uint16) (phy_utils_read_phyreg(pi, NPHY_crsminpowerl0) & 0xff);
		pi->interf->crsminpwru0 =
			(uint16) (phy_utils_read_phyreg(pi, NPHY_crsminpoweru0) & 0xff);
		pi->interf->init_gain_core1 =
			phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2057);
		pi->interf->init_gain_core2 =
			phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeA2057);
		pi->interf->init_gainb_core1 =
			phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
		pi->interf->init_gainb_core2 =
			phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057);
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, 0x106, 16, &newgain);
		for (i = 0; i < pi->pubpi->phy_corenum; i++) {
			pi->interf->init_gain_rfseq[i] = newgain;
		}

		/* pi->interf->noise.noise_glitch_high_detect_total = 0;
		pi->interf->noise.noise_glitch_low_detect_total = 0;
		*/

		pi->interf->noise_sw_set = TRUE;
	}
}

/** initialize aci parameters */
void
wlc_phy_aci_init_nphy(phy_info_t *pi)
{
	ASSERT(pi->interf->aci.nphy != NULL);

	/* Can be changed via ioctl/iovar */
	pi->interf->aci.nphy->detect_repeat_ctr = 2;
	pi->interf->aci.nphy->detect_num_samples = 50;
	pi->interf->aci.enter_thresh = 100;
	pi->interf->aci.nphy->adcpwr_enter_thresh = 500;
	pi->interf->aci.nphy->adcpwr_exit_thresh = 500;
	pi->interf->aci.nphy->undetect_window_sz = 5;

	/* Phyreg 0xc33(bphy eneregy thresh values for ACI pwr levels(lo, md, hi) */
	pi->interf->aci.nphy->b_energy_lo_aci = 0x40;
	pi->interf->aci.nphy->b_energy_md_aci = 0x80;
	pi->interf->aci.nphy->b_energy_hi_aci = 0xc0;
}

/** aci mode reset hardware and software states */
void
wlc_phy_acimode_reset_nphy(phy_info_t *pi)
{
	bool suspend;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		wlc_phy_aci_init_nphy(pi);
		wlc_phy_acimode_set_nphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);
		wlc_phy_aci_sw_reset_nphy(pi);
		wlc_phy_aci_noise_shared_reset_nphy(pi);
	}

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/** aci software state reset */
void
wlc_phy_aci_sw_reset_nphy(phy_info_t *pi)
{
	int i;
	pi->interf->aci.detect_index = 0;
	pi->interf->aci.detect_total = 0;
	pi->interf->aci.nphy->detection_in_progress = FALSE;
	for (i = 0; i < NPHY_ACI_MAX_UNDETECT_WINDOW_SZ; i++) {
		pi->interf->aci.detect_list[i] = 0;
		pi->interf->aci.detect_acipwr_lt_list[i] = -100;
	}
	pi->interf->aci.detect_acipwr_max = PHY_ACI_PWR_NOTPRESENT;

	pi->aci_state &= ~ACI_ACTIVE;
	pi->aci_active_pwr_level = 0;
	wlapi_high_update_phy_mode(pi->sh->physhim, 0);
}

void
wlc_phy_acimode_upd_nphy(phy_info_t *pi)
{
	int aci_pwr;
	uint8 aci_detect = 0;
	uint16 hpvga_gain, orig_hpvga_gain;
	int16 delta_hpvga_gain = 0;
	int16 newgain;
	uint16 curgain, newgainarray[4];
	uint i;
	bool suspend = FALSE;

	/* !! suspend MAC first */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
	}
	wlc_btcx_override_enable(pi);

	aci_pwr = wlc_phy_aci_scan_nphy(pi);

	if (aci_pwr != PHY_ACI_PWR_NOTPRESENT) {
		aci_detect = 1;
	}

	/* evict old value */
	pi->interf->aci.detect_total -= pi->interf->aci.detect_list[pi->interf->aci.detect_index];

	/* admit new value */
	pi->interf->aci.detect_total += aci_detect;

	pi->interf->aci.detect_list[pi->interf->aci.detect_index] = aci_detect;
	pi->interf->aci.detect_acipwr_lt_list[pi->interf->aci.detect_index] = aci_pwr;
	pi->interf->aci.detect_index++;

	if (pi->interf->aci.detect_index >= pi->interf->aci.nphy->undetect_window_sz)
		pi->interf->aci.detect_index = 0;

	for (i = 0; i < pi->interf->aci.nphy->undetect_window_sz; i++) {
		if ((pi->interf->aci.detect_list[i] == 1) &&
		    (pi->interf->aci.detect_acipwr_lt_list[i] > aci_pwr))
			aci_pwr = pi->interf->aci.detect_acipwr_lt_list[i];
	}

	PHY_ACI(("pi->aci_active_pwr_level %d aci_pwr %d\n",
	pi->aci_active_pwr_level, aci_pwr));

	/* Changing the IFCHECK to accomodate new scheme of aci..
	 * where we check aci levels before entering here
	 */
	if (pi->aci_state & ACI_ACTIVE) {
		if (pi->interf->aci.detect_total == 0) {
			PHY_ACI(("exiting aci \n"));
			if (pi->sh->interference_mode == WLAN_AUTO_W_NOISE &&
			    (NREV_GE(pi->pubpi->phy_rev, 16))) {

				/* about to exit ACI mode, save off variables */
				pi->interf->crsminpwrl0_aci_on =
					pi->interf->crsminpwrl0;
				pi->interf->crsminpwru0_aci_on =
					pi->interf->crsminpwru0;
				pi->interf->crsminpwr_index_aci_on =
					pi->interf->crsminpwr_index;
#ifdef BPHY_DESENSE
				pi->interf->bphy_crsminpwr_aci_on =
					pi->interf->bphy_crsminpwr;
				pi->interf->bphy_crsminpwr_index_aci_on =
					pi->interf->bphy_crsminpwr_index;
#endif // endif
				pi->interf->init_gain_core1_aci_on =
					pi->interf->init_gain_core1;
				pi->interf->init_gain_core2_aci_on =
					pi->interf->init_gain_core2;
				pi->interf->init_gainb_core1_aci_on =
					pi->interf->init_gainb_core1;
				pi->interf->init_gainb_core2_aci_on =
					pi->interf->init_gainb_core2;
				wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ,
					pi->pubpi->phy_corenum, 0x106, 16,
					pi->interf->init_gain_rfseq_aci_on);

				/* disable ACI mitigation */
				wlc_phy_acimode_set_nphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);

				/* now, use values of crsminpwr and init gains that were
				 * determined previously
				 */
				pi->interf->crsminpwrl0 =
					pi->interf->crsminpwrl0_aci_off;
				phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
					NPHY_crsminpowerl0_crsminpower0_MASK,
					pi->interf->crsminpwrl0);
				pi->interf->crsminpwru0 =
					pi->interf->crsminpwru0_aci_off;
				phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
					NPHY_crsminpoweru0_crsminpower0_MASK,
					pi->interf->crsminpwru0);
				pi->interf->crsminpwr_index =
					pi->interf->crsminpwr_index_aci_off;

#ifdef BPHY_DESENSE
				pi->interf->bphy_crsminpwr =
					pi->interf->bphy_crsminpwr_aci_off;

				phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
					NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
					pi->interf->bphy_crsminpwr);

				pi->interf->bphy_crsminpwr_index =
					pi->interf->bphy_crsminpwr_index_aci_off;
#endif // endif

				pi->interf->init_gain_core1 = pi->interf->init_gain_core1_aci_off;
				phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057,
					pi->interf->init_gain_core1);
				pi->interf->init_gain_core2 = pi->interf->init_gain_core2_aci_off;
				phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057,
					pi->interf->init_gain_core2);
				pi->interf->init_gainb_core1 = pi->interf->init_gainb_core1_aci_off;
				phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057,
					pi->interf->init_gainb_core1);
				pi->interf->init_gainb_core2 = pi->interf->init_gainb_core2_aci_off;
				phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057,
					pi->interf->init_gainb_core2);
				for (i = 0; i < pi->pubpi->phy_corenum; i++) {
					pi->interf->init_gain_rfseq[i] =
						pi->interf->init_gain_rfseq_aci_off[i];
				}

				wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
					pi->pubpi->phy_corenum, 0x106, 16,
					pi->interf->init_gain_rfseq);

			} else {
				wlc_phy_acimode_set_nphy(pi, FALSE, PHY_ACI_PWR_NOTPRESENT);
			}
		} else if (aci_pwr != pi->interf->aci.detect_acipwr_max) {
			wlc_phy_aci_pwr_upd_nphy(pi, aci_pwr);
		} else {
			/* do nothing */
		}
	} else {
		if (pi->interf->aci.detect_total >= 1) {
			PHY_ACI(("Entering aci with pwr_level %d\n",
			aci_pwr));

			if (pi->sh->interference_mode == WLAN_AUTO_W_NOISE &&
			    (NREV_GE(pi->pubpi->phy_rev, 16))) {

				/* going into ACI mitigation mode, save off values */
				/* save only when entering from aci not present
				 * and not when aci levels change
				 */
				if (pi->aci_active_pwr_level == PHY_ACI_PWR_NOTPRESENT) {

				pi->interf->crsminpwrl0_aci_off =
					pi->interf->crsminpwrl0;
				pi->interf->crsminpwru0_aci_off =
					pi->interf->crsminpwru0;
				pi->interf->crsminpwr_index_aci_off =
					pi->interf->crsminpwr_index;

#ifdef BPHY_DESENSE
				pi->interf->bphy_crsminpwr_aci_off =
					pi->interf->bphy_crsminpwr;
				pi->interf->bphy_crsminpwr_index_aci_off =
					pi->interf->bphy_crsminpwr_index;
#endif // endif

				pi->interf->init_gain_core1_aci_off = pi->interf->init_gain_core1;
				pi->interf->init_gain_core2_aci_off = pi->interf->init_gain_core2;
				pi->interf->init_gainb_core1_aci_off = pi->interf->init_gainb_core1;
				pi->interf->init_gainb_core2_aci_off = pi->interf->init_gainb_core2;
				wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ,
					pi->pubpi->phy_corenum, 0x106, 16,
					pi->interf->init_gain_rfseq_aci_off);

				}

				/* check to see what the delta is for the hpvga gain */
				hpvga_gain = (uint16) ((phy_utils_read_phyreg(pi,
					NPHY_Core1InitGainCodeB2057)) >>
						NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);

				orig_hpvga_gain = (pi->interf->init_gain_code_core1_stored >>
						NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);
				/* minus 2 here for difference in overall init gain */
				/* for aci mitigation on vs. off */
				delta_hpvga_gain = (orig_hpvga_gain - hpvga_gain) - 2;
				if (delta_hpvga_gain < 0)
					delta_hpvga_gain = 0;

				/* enable ACI mitigation */
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_nphy(pi, TRUE, aci_pwr);
				}

				if (pi->interf->aci_on_firsttime) {
					/* never triggered ACI before.  do not use set values */
					pi->interf->aci_on_firsttime = FALSE;

					/* if aci mitigation off hpvga gain is different,
					 * then change aci mitigation on hpvga gain by
					 * the same amount roughly
					 */
					if (delta_hpvga_gain > 0) {
						/*  back off init gains by corresponding amount */
						newgain = (uint16) ((phy_utils_read_phyreg(pi,
						NPHY_Core1InitGainCodeB2057)) >>
						NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT)
							- delta_hpvga_gain;

						if (newgain < 0)
							newgain = 0;

						phy_utils_mod_phyreg(pi,
						NPHY_Core1InitGainCodeB2057,
						NPHY_Core1InitGainCodeB2057_InitBiQ1Index_MASK,
						(newgain <<
						NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT))
						;
						phy_utils_mod_phyreg(pi,
						NPHY_Core2InitGainCodeB2057,
						NPHY_Core1InitGainCodeB2057_InitBiQ1Index_MASK,
						(newgain <<
						NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT))
						;
					} else {
						/*  back off init gains by corresponding amount */
						newgain = (uint16) ((phy_utils_read_phyreg(pi,
						NPHY_Core1InitGainCodeA2056)) >>
						NPHY_Core1InitGainCodeA2056_initvgagainIndex_SHIFT)
							- delta_hpvga_gain;

						if (newgain < 0)
							newgain = 0;

						phy_utils_mod_phyreg(pi,
						NPHY_Core1InitGainCodeA2056,
						NPHY_Core1InitGainCodeA2056_initvgagainIndex_MASK,
						(newgain <<
						NPHY_Core1InitGainCodeA2056_initvgagainIndex_SHIFT))
						;
						phy_utils_mod_phyreg(pi,
						NPHY_Core2InitGainCodeA2056,
						NPHY_Core1InitGainCodeA2056_initvgagainIndex_MASK,
						(newgain <<
						NPHY_Core1InitGainCodeA2056_initvgagainIndex_SHIFT))
						;
					}

					/* modify tx to rx rfseq */
					wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 1, 0x106, 16,
						&curgain);

					newgain = (curgain >> 12) - delta_hpvga_gain;
					if (newgain < 0)
						newgain = 0;

					newgain = (newgain << 12) | (curgain & 0xfff);
					newgainarray[0] = newgain;
					newgainarray[1] = newgain;
					newgainarray[2] = newgain;
					newgainarray[3] = newgain;
					wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
						pi->pubpi->phy_corenum, 0x106, 16,
						newgainarray);

					/* save off values for next time */
					pi->interf->crsminpwrl0 = (uint16)
					        (phy_utils_read_phyreg(pi,
					                NPHY_crsminpowerl0) & 0xff);
					pi->interf->crsminpwru0 = (uint16)
					        (phy_utils_read_phyreg(pi,
					                NPHY_crsminpoweru0) & 0xff);
#ifdef BPHY_DESENSE
					pi->interf->bphy_crsminpwr = (uint16)
					        (phy_utils_read_phyreg(pi,
					                NPHY_bphycrsminpower0) & 0xff);
#endif // endif
					pi->interf->init_gain_core1 =
						phy_utils_read_phyreg(pi,
					               NPHY_Core1InitGainCodeA2057);
					pi->interf->init_gain_core2 =
						phy_utils_read_phyreg(pi,
					               NPHY_Core2InitGainCodeA2057);
					pi->interf->init_gainb_core1 =
						phy_utils_read_phyreg(pi,
					               NPHY_Core1InitGainCodeB2057);
					pi->interf->init_gainb_core2 =
						phy_utils_read_phyreg(pi,
					               NPHY_Core2InitGainCodeB2057);
					wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ,
						pi->pubpi->phy_corenum, 0x106,
						16, pi->interf->init_gain_rfseq);
				} else {
					/* now, use values of crsminpwr and init gains that were
					 * determined previously
					 */
					pi->interf->crsminpwrl0 =
						pi->interf->crsminpwrl0_aci_on;
					phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
						NPHY_crsminpowerl0_crsminpower0_MASK,
						pi->interf->crsminpwrl0);
					pi->interf->crsminpwru0 =
						pi->interf->crsminpwru0_aci_on;
					phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
						NPHY_crsminpoweru0_crsminpower0_MASK,
						pi->interf->crsminpwru0);
					pi->interf->crsminpwr_index =
						pi->interf->crsminpwr_index_aci_on;

#ifdef BPHY_DESENSE
					pi->interf->bphy_crsminpwr =
						pi->interf->bphy_crsminpwr_aci_on;
					phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
						NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
						pi->interf->bphy_crsminpwr);
					pi->interf->bphy_crsminpwr_index =
						pi->interf->bphy_crsminpwr_index_aci_on;
#endif // endif
					pi->interf->init_gain_core1 =
						pi->interf->init_gain_core1_aci_on;
					phy_utils_write_phyreg(pi,
					        NPHY_Core1InitGainCodeA2057,
						pi->interf->init_gain_core1);
					pi->interf->init_gain_core2 =
						pi->interf->init_gain_core2_aci_on;
					phy_utils_write_phyreg(pi,
					        NPHY_Core2InitGainCodeA2057,
						pi->interf->init_gain_core2);
					pi->interf->init_gainb_core1 =
						pi->interf->init_gainb_core1_aci_on;
					phy_utils_write_phyreg(pi,
					        NPHY_Core1InitGainCodeB2057,
						pi->interf->init_gainb_core1);
					pi->interf->init_gainb_core2 =
						pi->interf->init_gainb_core2_aci_on;
					phy_utils_write_phyreg(pi,
					        NPHY_Core2InitGainCodeB2057,
						pi->interf->init_gainb_core2);
					for (i = 0; i < pi->pubpi->phy_corenum; i++) {
						pi->interf->init_gain_rfseq[i] =
							pi->interf->init_gain_rfseq_aci_on[i];
					}

					wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
						pi->pubpi->phy_corenum, 0x106, 16,
						pi->interf->init_gain_rfseq);

				}
			} else {
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_nphy(pi, TRUE, aci_pwr);
				}

			}
		}
	}

	/* unsuspend mac */
	wlc_phy_btcx_override_disable(pi);
	if (!suspend) {
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}

	pi->interf->aci.detect_acipwr_max = aci_pwr;
	PHY_ACI(("wlc_phy_acimode_upd_nphy: aci_state = %d, detect_total = %d\n",
		pi->aci_state, pi->interf->aci.detect_total));
}

int
wlc_phy_aci_scan_nphy(phy_info_t *pi)
{
	int aci_pwr;

	pi->interf->aci.nphy->detection_in_progress = TRUE;
	aci_pwr = wlc_phy_aci_scan_iqbased_nphy(pi);
	pi->interf->aci.nphy->detection_in_progress = FALSE;

	return aci_pwr;
}

/**
 * Return whether or not ACI is present. WARNING: this fcn change/restore channel inside, tested on
 * 2G only
 */
static int
wlc_phy_aci_scan_iqbased_nphy(phy_info_t *pi)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint16 repeat_ctr = pi->interf->aci.nphy->detect_repeat_ctr;
	uint16 nsamps = pi->interf->aci.nphy->detect_num_samples;
	uint16 count_thresh = nsamps/5;
	int num_adc_ranges = 3;	/* ADC power ranges & Valid W2s in that power range */
	uint32 adc_pwrs_rev7[] = {pi->interf->aci.nphy->adcpwr_enter_thresh, 7000, 10000, 14500};
	int8 valid_w2s_rev7[] = {15, 0, -15};
	int16 adc_code;
	uint16 adc_code_thresh;
	uint32 avg_adcpwr, adcpwrL, adcpwrR;
	uint32 pwr_ch, pwr = 0;
	bool adcpwr_val;
	int8 w2 = 0;
	int avgw2, w2_ch;
	uint16 i, ctr, core, samp, count;
	int chan, start, end;
	chanspec_t orig_chanspec;
	uint8 orig_channel;
	uint16 classifier_state;	/* register to be saved/restored */
	uint16 clip_state[NPHY_CORE_NUM];
	uint16 clip_off[NPHY_CORE_NUM] = {0xffff, 0xffff};
	uint16 gpiosel, gpio;
	uint16 gpioLoOutEn, gpioHiOutEn;
	int chan_delta, chan_skip;
	uint8 lna1_rev3 = 0, lna2_rev3 = 0, mix_tia_gain_rev3 = 0, lpf_biq0_rev3 = 0;
	uint8 lpf_biq1_rev3 = 0, hpvga_rev3 = 0;
	uint8 tx_pwr_ctrl_state;
	uint8 saved_pwr_idx[NPHY_CORE_NUM];
	uint16 saved_crsminpwrl0, saved_crsminpwru0;
	uint16 saved_initgaincode_core1 = 0, saved_initgaincode_core2 = 0;
	uint16 saved_initgaincodeb_core1 = 0, saved_initgaincodeb_core2 = 0;
	uint16 saved_initgaincode_rfseq[4];
#ifdef BPHY_DESENSE
	uint16 saved_crsminpwr_bphy = 0;
#endif // endif

	ASSERT(nsamps > 0);
	/* save the original chanspec */
	orig_chanspec = pi->radio_chanspec;
	orig_channel = CHSPEC_CHANNEL(orig_chanspec);

	/* 4322 */
	adc_code_thresh = (uint16) math_sqrt_int_32(adc_pwrs_rev7[0]/2);

	/* Save registers that are going to be changed (start) */
	classifier_state = wlc_phy_classifier_nphy(pi, 0, 0);
	wlc_phy_clip_det_nphy(pi, 0, clip_state);

	/* this for restoring the tx pwr index when we return to orig channel */
	saved_pwr_idx[0] = wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_0);
	saved_pwr_idx[1] = wlc_phy_txpwr_idx_cur_get_nphy(pi, PHY_CORE_1);

	/* save crs min pwr and init gain values */
	saved_crsminpwrl0 = (uint16)(phy_utils_read_phyreg(pi, NPHY_crsminpowerl0) & 0xff);
	saved_crsminpwru0 = (uint16)(phy_utils_read_phyreg(pi, NPHY_crsminpoweru0) & 0xff);
#ifdef BPHY_DESENSE
	saved_crsminpwr_bphy =
	        (uint16)(phy_utils_read_phyreg(pi, NPHY_bphycrsminpower0) & 0xff);
#endif // endif

	saved_initgaincode_core1 = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2057);
	saved_initgaincode_core2 = phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeA2057);
	saved_initgaincodeb_core1 = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
	saved_initgaincodeb_core2 = phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057);

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ,
		pi->pubpi->phy_corenum, 0x106, 16, saved_initgaincode_rfseq);

	/*           instead of using gpioselects (speeds up cal) */
	gpiosel = phy_utils_read_phyreg(pi, NPHY_gpioSel);
	gpioLoOutEn = phy_utils_read_phyreg(pi, NPHY_gpioLoOutEn);
	gpioHiOutEn = phy_utils_read_phyreg(pi, NPHY_gpioHiOutEn);

	/* ************** (end) ************ */

	/* Based on phy bw, set the aci scan delta & skip */
	if (IS40MHZ(pi)) {
		chan_delta = NPHY_ACI_40MHZ_CHANNEL_DELTA_GE_REV3;
		chan_skip = NPHY_ACI_40MHZ_CHANNEL_SKIP_GE_REV3;
	} else {
		chan_delta = NPHY_ACI_CHANNEL_DELTA_GE_REV3;
		chan_skip = NPHY_ACI_CHANNEL_SKIP_GE_REV3;
	}

	/* Channels scan range */
	start = MAX(ACI_FIRST_CHAN, orig_channel - chan_delta);
	end = MIN(ACI_LAST_CHAN, orig_channel + chan_delta);

	/* Algorithm:
	*  1. Scan channels(for power) that are least 4 apart
	*  2. Average adc_pwr & W2 for adc_codes greater than some threshold
	*  3. High power signal can leak in 4 channel apart, so qualify that
	*     with W2. For low adc_pwr W2 will be low, if its coming from adjacent channel
	*/
	for (chan = start; chan <= end; chan++) {
	if ((chan < (orig_channel - chan_skip)) ||
		(chan > (orig_channel + chan_skip))) {

		/* because this function can potentially reinit the phy */
		/* save off the current crs min power and init gain values */
		/* and restore after this function */

		wlc_phy_chanspec_set((wlc_phy_t*)pi, CH20MHZ_CHSPEC(chan));

		/* ** Change phy registers needed for scanning (start) ** */
		/* Classifier off,clip det off, set appropriate gain */
		wlc_phy_classifier_nphy(pi,
			NPHY_ClassifierCtrl_classifierSel_MASK, 4);
		wlc_phy_clip_det_nphy(pi, 1, clip_off);

		/* use overall gain 15 db less than init gain here */
		/* 51dB gain */
		lna1_rev3 = 3; lna2_rev3 = 3;  mix_tia_gain_rev3 = 4;
		lpf_biq0_rev3 = 2; lpf_biq1_rev3 = 0; hpvga_rev3 = 0;
		/* lna1=25dB, lna2=15, mix=5, lpf_biq0=6, lpf_bq1=0 */

		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_rxgain_MASK,
			(hpvga_rev3 << 8) | (mix_tia_gain_rev3 << 4) |
			 (lna2_rev3 << 2) | lna1_rev3, 0x3, 0,
			NPHY_REV7_RFCTRLOVERRIDE_ID0);
		wlc_phy_rfctrl_override_nphy_rev7(
			pi, NPHY_REV7_RfctrlOverride_lpf_gain_biq01_MASK,
			(lpf_biq1_rev3 << 4) | lpf_biq0_rev3, 0x3, 0,
			NPHY_REV7_RFCTRLOVERRIDE_ID0);

		/* Enable the gpio's */
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(NPHY, gpioLoOutEn, 0xffff)
			PHY_REG_WRITE_ENTRY(NPHY, gpioHiOutEn, 0xffff)
		PHY_REG_LIST_EXECUTE(pi);

		/* Select W2 Rssi */
		wlc_phy_rssisel_nphy(pi, RADIO_MIMO_CORESEL_ALLRX, NPHY_RSSI_SEL_W2);

		/* ** end (changing registers) ** */

		pwr_ch = 0;
		w2_ch = 32;

		/* Do i-rail for Core1 & q-rail for Core2 to save time */
		FOREACH_CORE(pi, core) {
		/* program gpiosel */
		phy_utils_write_phyreg(pi, NPHY_gpioSel, 6+core);

		for (ctr = 0; ctr < repeat_ctr; ctr++) {
			avg_adcpwr = 0;
			avgw2 = 0;
			count = 0;
			for (samp = 0; samp < nsamps; samp++) {
				if (core == PHY_CORE_0)
				    gpio = phy_utils_read_phyreg(pi, NPHY_gpioLoOut);
				else
				    gpio = phy_utils_read_phyreg(pi, NPHY_gpioHiOut);

				/* MSB 8bits of ADC are enough */
				adc_code = ((int16)((gpio & 0x3ff) << 6)) >> 8;
				w2 = ((int8)(((gpio >> 10) & 0x3f) << 2)) >> 2;

				if (ABS(adc_code) > adc_code_thresh) {
					count++;
					avg_adcpwr += (adc_code * adc_code);
					avgw2 += w2;
				}
			}
			if (count > count_thresh) {
				avg_adcpwr = avg_adcpwr / count;
			if (avg_adcpwr > pwr_ch) {
				avgw2 = avgw2 / count;
				adcpwr_val = TRUE;
				for (i = 0; i < num_adc_ranges; i++) {
					adcpwrL = adc_pwrs_rev7[i];
					adcpwrR = adc_pwrs_rev7[i+1];
					if ((avg_adcpwr >= adcpwrL) &&
					(avg_adcpwr < adcpwrR)) {
						if (avgw2 < valid_w2s_rev7[i]) {
							adcpwr_val = FALSE;
						}
						break;
					}
				} /* spans over num_adc_range */
				PHY_ACI(("wlc_phy_aci_scan_iqbased_nphy:"
				" chan=%d avg_adc=%d avgw2=%d cnt=%d"
				" core=%d ctr=%d\n",
				chan, avg_adcpwr,
				avgw2, count, core, ctr));
				if (adcpwr_val) {
					pwr_ch = avg_adcpwr;
					w2_ch = avgw2;
				}
			} /* CHECKIF: avg_adcpwr > pwr_ch */
			} /* CHECKIF: count > count_thres */
			OSL_DELAY(10);

		} /* FORLOOP: repeat_cnt */
		pwr = MAX(pwr, pwr_ch);

		} /* FORLOOP: CORE */
		PHY_ACI(("%s:  pwr=%d, chan=%d, pwr_ch=%d, w2_ch=%d\n",
		__FUNCTION__, pwr, chan, pwr_ch, w2_ch));
		BCM_REFERENCE(w2_ch);
	} /* CHECKIF: curr_ch = orig_ch -(+) ch_skip */
	} /* FORLOOP: Spans over adjacent channels */

	/* ************* RESTORE REGISTERS ************* */
	phy_utils_write_phyreg(pi, NPHY_gpioSel, gpiosel);
	phy_utils_write_phyreg(pi, NPHY_gpioLoOutEn, gpioLoOutEn);
	phy_utils_write_phyreg(pi, NPHY_gpioHiOutEn, gpioHiOutEn);
	wlc_phy_rssisel_nphy(pi, RADIO_MIMO_CORESEL_OFF, 0);

	wlc_phy_rfctrl_override_nphy_rev7(
		pi, NPHY_REV7_RfctrlOverride_rxgain_MASK,
		 0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	wlc_phy_rfctrl_override_nphy_rev7(
		pi, NPHY_REV7_RfctrlOverride_lpf_gain_biq01_MASK,
		0, 0x3, 1, NPHY_REV7_RFCTRLOVERRIDE_ID0);
	/* revert to the original chanspec */
	wlc_phy_chanspec_set((wlc_phy_t*)pi, orig_chanspec);

	wlc_phy_clip_det_nphy(pi, 1, clip_state);
	wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
		classifier_state);

	/* restore tx pwr index to original power index */
	tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
	wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		pi_nphy->nphy_txpwr_idx_5G[0] = saved_pwr_idx[0];
		pi_nphy->nphy_txpwr_idx_5G[1] = saved_pwr_idx[1];
	} else {
		pi_nphy->nphy_txpwr_idx_2G[0] = saved_pwr_idx[0];
		pi_nphy->nphy_txpwr_idx_2G[1] = saved_pwr_idx[1];
	}
	wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);

	/* restore the crsminpwr and init gains */
	phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
		NPHY_crsminpowerl0_crsminpower0_MASK,
		saved_crsminpwrl0);
	phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		saved_crsminpwru0);
#ifdef BPHY_DESENSE
	phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
	                     NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
	                     saved_crsminpwr_bphy);
#endif // endif

	phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057, saved_initgaincode_core1);
	phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057, saved_initgaincode_core2);
	phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, saved_initgaincodeb_core1);
	phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, saved_initgaincodeb_core2);

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
		pi->pubpi->phy_corenum, 0x106, 16, saved_initgaincode_rfseq);

	wlc_phy_noise_sw_set_nphy(pi);

	/* Based on adc power level decide what is the ACI power level */
	PHY_ACI(("pwr = %d, pwr > adc_pwrs_rev7[1] = %d\n",
		pwr, (pwr > adc_pwrs_rev7[1])));
	if (pwr > adc_pwrs_rev7[1])
		return PHY_ACI_PWR_HIGH;

	return PHY_ACI_PWR_NOTPRESENT;
}

/** Don't use this feature right now, until the values are correct */
static void
wlc_phy_aci_pwr_upd_nphy(phy_info_t *pi, int aci_pwr)
{
	uint16 b_energy_thresh = pi->interf->aci.nphy->b_energy_hi_aci;

	if (aci_pwr == PHY_ACI_PWR_LOW) {
		b_energy_thresh = pi->interf->aci.nphy->b_energy_lo_aci;
	} else if (aci_pwr == PHY_ACI_PWR_MED) {
		b_energy_thresh = pi->interf->aci.nphy->b_energy_md_aci;
	} else if (aci_pwr == PHY_ACI_PWR_HIGH) {
		b_energy_thresh = pi->interf->aci.nphy->b_energy_hi_aci;
	}

	phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF+BPHY_PEAK_ENERGY_LO), b_energy_thresh);
}

static void
wlc_phy_aci_sw_set_nphy(phy_info_t *pi, bool enable, int aci_pwr)
{

	if (enable) {
		if (!CHSPEC_IS2G(pi->radio_chanspec)) {
			/* not 2 ghz, so do not do this */
			PHY_ERROR(("wlc_phy_aci_sw_set_nphy: do not enable ACI for 5GHz!\n"));
			return;
		}

		pi->aci_state |= ACI_ACTIVE;
		pi->aci_active_pwr_level = aci_pwr;
		pi->aci_start_time = pi->sh->now;
		wlapi_high_update_phy_mode(pi->sh->physhim, PHY_MODE_ACI);
	} else {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) == pi->interf->curr_home_channel) {
			/* on home channel, i wanted to disable ACI, so do so */
			pi->aci_state &= ~ACI_ACTIVE;
			pi->aci_active_pwr_level = aci_pwr;
			wlapi_high_update_phy_mode(pi->sh->physhim, 0);
		}
	}

}

static void
wlc_phy_aci_noise_shared_reset_nphy(phy_info_t *pi)
{
	uint8 core;

	/* SOFTWARE STATE RESET */
	pi->interf->init_gain_core1 = pi->interf->init_gain_code_core1_stored;
	pi->interf->init_gain_core2 = pi->interf->init_gain_code_core2_stored;
	pi->interf->init_gainb_core1 = pi->interf->init_gain_codeb_core1_stored;
	pi->interf->init_gainb_core2 = pi->interf->init_gain_codeb_core2_stored;
	FOREACH_CORE(pi, core) {
		pi->interf->init_gain_rfseq[core] = pi->interf->init_gain_table_stored[core];
	}

	pi->interf->init_gain_core1_aci_off = pi->interf->init_gain_core1;
	pi->interf->init_gain_core2_aci_off = pi->interf->init_gain_core2;
	pi->interf->init_gain_core1_aci_on = pi->interf->init_gain_core1;
	pi->interf->init_gain_core2_aci_on = pi->interf->init_gain_core2;
	pi->interf->init_gainb_core1_aci_off = pi->interf->init_gainb_core1;
	pi->interf->init_gainb_core2_aci_off = pi->interf->init_gainb_core2;
	pi->interf->init_gainb_core1_aci_on = pi->interf->init_gainb_core1;
	pi->interf->init_gainb_core2_aci_on = pi->interf->init_gainb_core2;
	FOREACH_CORE(pi, core) {
		pi->interf->init_gain_rfseq_aci_on[core] =
			pi->interf->init_gain_table_stored[core];
		pi->interf->init_gain_rfseq_aci_off[core] =
			pi->interf->init_gain_table_stored[core];
	}

	pi->interf->crsminpwrl0 = pi->interf->crsminpwrthld_20L_stored;
	pi->interf->crsminpwru0 = pi->interf->crsminpwrthld_20U_stored;
#ifdef BPHY_DESENSE
	pi->interf->bphy_crsminpwr = BPHY_DESENSE_CRSMINPWR_BASELINE;
#endif // endif
	pi->interf->aci_on_firsttime = TRUE;

	/* UPDATE ACI_OFF MINPWRS */
	pi->interf->crsminpwrl0_aci_off = pi->interf->crsminpwrl0;
	pi->interf->crsminpwru0_aci_off = pi->interf->crsminpwru0;
#ifdef BPHY_DESENSE
	pi->interf->bphy_crsminpwr_aci_off = BPHY_DESENSE_CRSMINPWR_BASELINE;
#endif // endif
	/* UPDATE ACI_ON MINPWRS */
	pi->interf->crsminpwrl0_aci_on = pi->interf->crsminpwrl0;
	pi->interf->crsminpwru0_aci_on = pi->interf->crsminpwru0;
#ifdef BPHY_DESENSE
	pi->interf->bphy_crsminpwr_aci_on = BPHY_DESENSE_CRSMINPWR_BASELINE;
#endif // endif

	/* HARDWARE STATE RESET */
	phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057,
		pi->interf->init_gain_code_core1_stored);
	phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057,
		pi->interf->init_gain_code_core2_stored);
	phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057,
		pi->interf->init_gain_codeb_core1_stored);
	phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057,
		pi->interf->init_gain_codeb_core2_stored);

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
		pi->pubpi->phy_corenum, 0x106,
		16, pi->interf->init_gain_table_stored);

	/* reset crsminpwr threshold to original value */
	phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0, NPHY_crsminpowerl0_crsminpower0_MASK,
		pi->interf->crsminpwrthld_20L_stored);
	phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0, NPHY_crsminpoweru0_crsminpower0_MASK,
		pi->interf->crsminpwrthld_20U_stored);

	if (NREV_LE(pi->pubpi->phy_rev, 15)) {
		pi->interf->bphy_desense_lut = NPHY_bphy_desense_aci_off_lut_rev7to15;
		pi->interf->bphy_desense_lut_size =
			sizeof(NPHY_bphy_desense_aci_off_lut_rev7to15)/
			sizeof(bphy_desense_info_t);
		pi->interf->bphy_min_sensitivity = NPHY_BPHY_MIN_SENSITIVITY_REV7TO15;
		pi->interf->ofdm_min_sensitivity = NPHY_OFDM_MIN_SENSITIVITY_REV7TO15;
	}
}

static void
wlc_phy_aci_noise_shared_hw_set_nphy(phy_info_t *pi, bool aci_miti_enable,
	bool from_aci_call)
{

	uint16 aci_present_init_gaincode = 0;
	uint16 aci_present_init_gaincodeb = 0;
	uint16 aci_present_rfseq_init_gain_5357[] = {0x9136, 0x9136, 0x9136, 0x9136};
	uint16 aci_present_rfseq_init_gain_5357_elna[] = {0x5136, 0x5136, 0x5136, 0x5136};
	uint16 newgainarray[4];
	bool band_switch = FALSE;
	bool bw_switch = FALSE;

	if (from_aci_call) {
		if (aci_miti_enable) {
			/* reset crsminpwr threshold to original (baseline OR init)
			 * values
			 */
			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				pi->interf->crsminpwrthld_20L_stored);
			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				pi->interf->crsminpwrthld_20U_stored);
#ifdef BPHY_DESENSE
			phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
				NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
				pi->interf->bphy_crsminpwrthld_stored);
#endif // endif

			/* reduce init gain and also redistribute gains
			 * with less lna gain
			 */

			/* use these values: */
			/* lna1 = 0x2, lna2 = 0x1, hpvga = 8  */
			/* (these settings provide an  */
			/* overall init gain that is 6 db lower */
			if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
				(CHSPEC_IS2G(pi->radio_chanspec))) {
				/* Drop INIT_GAIN in HPVGA gain by 12 dB for ext-LNA
				 * to improve false detection
				 */
				/* Core1InitGainCodeA2057 =
				 * 0000 0000 1000 1100 = 0x8c
				 * Core1InitGainCodeB2057 =
				 * 0000 1001 0000 0000 = 0x900
				 * lna1=0x2, lna2=1, mixer=0x4,
				 * lpf-b0=0x0, lpf-b1=0x9(-4)
				 * lna1=20, lna2=7, mixer=5,
				 * lpf-b0=0, lpf-b1=27(-12) dB
				 */
				aci_present_init_gaincode = 0x6c;
				aci_present_init_gaincodeb = 0x510 |
					(phy_utils_read_phyreg(pi,
					NPHY_Core1InitGainCodeB2057) & 0xf);

			} else {
				aci_present_init_gaincode = 0x6c;
				aci_present_init_gaincodeb = 0x910 |
					(phy_utils_read_phyreg(pi,
					NPHY_Core1InitGainCodeB2057) & 0xf);
				pi->interf->max_hpvga_acion_2G = 9;
			}

			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057,
				aci_present_init_gaincode);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057,
				aci_present_init_gaincode);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057,
				aci_present_init_gaincodeb);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057,
				aci_present_init_gaincodeb);

			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
				pi->pubpi->phy_corenum, 0x106,
				16, aci_present_rfseq_init_gain_5357);

			if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
				(CHSPEC_IS2G(pi->radio_chanspec))) {
				/* Drop INIT_GAIN in HPVGA gain by 12 dB for ext-LNA
				 * to improve false detection
				 */
				wlc_phy_table_write_nphy(pi,
					NPHY_TBL_ID_RFSEQ,
					pi->pubpi->phy_corenum, 0x106, 16,
					aci_present_rfseq_init_gain_5357_elna);
			} else {
				wlc_phy_table_write_nphy(pi,
					NPHY_TBL_ID_RFSEQ,
					pi->pubpi->phy_corenum, 0x106, 16,
					aci_present_rfseq_init_gain_5357);
			}
		} else {
			/* not in home channel and/or not wl interference 4,
			 * so use latest bandinit values
			 */
			if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
				band_switch =
					(CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored)
					> 14 && CHSPEC_CHANNEL(pi->radio_chanspec)
					<= 14) ||
					((CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored)
					<= 14) && (CHSPEC_CHANNEL(pi->radio_chanspec)
					> 14));
				bw_switch =
					((CHSPEC_IS20(pi->interf->radio_chanspec_stored)
					== 1) &&
					(CHSPEC_IS20(pi->radio_chanspec) == 0)) ||
					((CHSPEC_IS20(pi->interf->radio_chanspec_stored)
					== 0) &&
					(CHSPEC_IS20(pi->radio_chanspec) == 1));
			}

			if (NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV) &&
				((band_switch || bw_switch) ||
				(pi->interf->init_gain_code_core1_stored == 0) ||
				(pi->interf->init_gain_code_core2_stored == 0) ||
				(pi->interf->init_gain_codeb_core1_stored == 0) ||
				(pi->interf->init_gain_codeb_core2_stored == 0))) {
				return;
			}

			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057,
				pi->interf->init_gain_code_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057,
				pi->interf->init_gain_code_core2_stored);
			phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057,
				pi->interf->init_gain_codeb_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057,
				pi->interf->init_gain_codeb_core2_stored);

			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
				pi->pubpi->phy_corenum, 0x106,
				16, pi->interf->init_gain_table_stored);

			phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
				NPHY_crsminpowerl0_crsminpower0_MASK,
				pi->interf->crsminpwrthld_20L_stored);
			phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
				NPHY_crsminpoweru0_crsminpower0_MASK,
				pi->interf->crsminpwrthld_20U_stored);

#ifdef BPHY_DESENSE
			phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
				NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
				pi->interf->bphy_crsminpwrthld_stored);
#endif // endif

		}
		if ((pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
			pi->sh->interference_mode == NON_WLAN)) {
			if (NREV_LE(pi->pubpi->phy_rev, 15)) {
				wlc_phy_bphy_ofdm_noise_update_LUT(pi, aci_miti_enable);
			}
		}
	} else {
		/* called from noise mitigation */
		if (pi->interf->noise.changeinitgain) {
			/* change init gain and rfseq init gain */

			/* modify only these bits */
			phy_utils_mod_phyreg(pi, NPHY_Core1InitGainCodeB2057,
				NPHY_Core1InitGainCodeB2057_InitBiQ1Index_MASK,
					(pi->interf->noise.newgain_initgain <<
				NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT));
			phy_utils_mod_phyreg(pi, NPHY_Core2InitGainCodeB2057,
				NPHY_Core1InitGainCodeB2057_InitBiQ1Index_MASK,
					(pi->interf->noise.newgain_initgain <<
				NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT));
			newgainarray[0] = pi->interf->noise.newgain_rfseq;
			newgainarray[1] = pi->interf->noise.newgain_rfseq;
			newgainarray[2] = pi->interf->noise.newgain_rfseq;
			newgainarray[3] = pi->interf->noise.newgain_rfseq;

			wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ,
				pi->pubpi->phy_corenum, 0x106, 16, newgainarray);

			pi->interf->noise.changeinitgain = FALSE;
		}

		phy_utils_mod_phyreg(pi, NPHY_crsminpowerl0,
		                     NPHY_crsminpowerl0_crsminpower0_MASK,
			pi->interf->noise.newcrsminpwr_20L);
		phy_utils_mod_phyreg(pi, NPHY_crsminpoweru0,
		                     NPHY_crsminpoweru0_crsminpower0_MASK,
			pi->interf->noise.newcrsminpwr_20U);
#ifdef BPHY_DESENSE
		phy_utils_mod_phyreg(pi, NPHY_bphycrsminpower0,
			NPHY_bphycrsminpower0_bphycrsminpower0_MASK,
			pi->interf->noise.newbphycrsminpwr);
#endif // endif
	}
}

void
wlc_phy_acimode_set_nphy(phy_info_t *pi, bool aci_miti_enable, int aci_pwr)
{
	uint16 tempgain;
	bool suspend;
	bool phyrev7to15 = NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV);

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	if ((!phyrev7to15 && CHSPEC_IS2G(pi->radio_chanspec)) ||
		(phyrev7to15 && ((CHSPEC_IS2G(pi->radio_chanspec) && aci_miti_enable &&
		!pi->interf->hw_aci_mitig_on) ||
		(!aci_miti_enable && pi->interf->hw_aci_mitig_on)))) {
		wlc_phy_aci_hw_set_nphy(pi, aci_miti_enable, aci_pwr);
		wlc_phy_aci_noise_shared_hw_set_nphy(pi, aci_miti_enable, TRUE);
		wlc_phy_aci_sw_set_nphy(pi, aci_miti_enable, aci_pwr);
		/* rev 7 changes crsminpwr only, limit the change */
		/* based on rssi value */
		/* concern:  bphy doesn't get limited the same way */

		if (!(PUB_NOT_ASSOC(pi))) {
			if (aci_miti_enable) {
				tempgain = (uint16) ((phy_utils_read_phyreg(pi,
					NPHY_Core1InitGainCodeB2057)) >>
					NPHY_Core1InitGainCodeB2057_InitBiQ1Index_SHIFT);
				wlc_phy_noise_limit_crsmin_nphy(pi, tempgain);
				wlc_phy_aci_noise_shared_hw_set_nphy
					(pi, aci_miti_enable, TRUE);
				wlc_phy_noise_sw_set_nphy(pi);
			}
		}
		if (phyrev7to15) {
			if (aci_miti_enable) {
				pi->interf->hw_aci_mitig_on = TRUE;
			} else {
				pi->interf->hw_aci_mitig_on = FALSE;
			}
		}
	}
	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_aci_hw_set_nphy(phy_info_t *pi, bool enable, int aci_pwr)
{
	uint16 aci_prsnt_clip1hi_gaincode;
	uint16 aci_prsnt_clip1hi_gaincodeb;
	uint16 aci_prsnt_nbclip_threshold;
	bool ed_lock;
	uint16 regval[NPHY_CORE_NUM];
	bool band_switch = FALSE;
	bool bw_switch = FALSE;
	bool phyrev7to15 = NREV_LT(pi->pubpi->phy_rev, LCNXN_BASEREV);

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (enable) {
		if (!CHSPEC_IS2G(pi->radio_chanspec)) {
			/* not 2 ghz, so do not do this */
			PHY_ERROR(("wlc_phy_aci_hw_set_nphy: do not enable ACI for 5GHz!\n"));
			return;
		}

		PHY_ACI(("wlc_phy_aci_hw_set_nphy: Enable ACI mitigation level %d, channel is %d\n",
			aci_pwr, CHSPEC_CHANNEL(pi->radio_chanspec)));

		if (pi_nphy->phyhang_avoid)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

		/* set energy drop timeout len to small value, return to
		 * search asap after previous detection fails
		 */
		if (!phyrev7to15) {
			pi->interf->energy_drop_timeout_len_stored =
				phy_utils_read_phyreg(pi, NPHY_energydroptimeoutLen);
#ifdef NPHYREV7_HTPHY_DFS_WAR
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				phy_utils_write_phyreg(pi, NPHY_energydroptimeoutLen, 0x2);
			} else {
				phy_utils_write_phyreg(pi, NPHY_energydroptimeoutLen, 0x9c40);
			}
#else
			phy_utils_write_phyreg(pi, NPHY_energydroptimeoutLen, 0x2);
#endif // endif
		}

		/* clip hi gain: since overall init gain reduced, */
		/* need to reduce clip hi gain and */
		/* since lna gains reduced, we need to also reduce */
		/* nbclip thresholds */

		if (!phyrev7to15) {
			pi->interf->clip1_hi_gain_code_core1_stored =
				phy_utils_read_phyreg(pi, NPHY_Core1clipHiGainCodeA2057);
			pi->interf->clip1_hi_gain_code_core2_stored =
				phy_utils_read_phyreg(pi, NPHY_Core2clipHiGainCodeA2057);
			pi->interf->clip1_hi_gain_codeb_core1_stored =
				phy_utils_read_phyreg(pi, NPHY_Core1clipHiGainCodeB2057);
			pi->interf->clip1_hi_gain_codeb_core2_stored =
				phy_utils_read_phyreg(pi, NPHY_Core2clipHiGainCodeB2057);
		}

		/* use lna1 = 0x2, lna2 = 0x1, mixer = 2, */
		/* lpf-b0 = 2, lpf-b1=2, dvga = 0 */
		aci_prsnt_clip1hi_gaincode = 0x2c;
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057,
			aci_prsnt_clip1hi_gaincode);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057,
			aci_prsnt_clip1hi_gaincode);
		aci_prsnt_clip1hi_gaincodeb = 0x220 |
			(phy_utils_read_phyreg(pi, NPHY_Core1clipHiGainCodeB2057) & 0xf);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057,
			aci_prsnt_clip1hi_gaincodeb);
		aci_prsnt_clip1hi_gaincodeb = 0x220 |
			(phy_utils_read_phyreg(pi, NPHY_Core2clipHiGainCodeB2057) & 0xf);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057,
			aci_prsnt_clip1hi_gaincodeb);

		/* nb threshold changes */
		if (!phyrev7to15) {
			pi->interf->nb_clip_thresh_core1_stored =
				phy_utils_read_phyreg(pi, NPHY_Core1nbClipThreshold);
			pi->interf->nb_clip_thresh_core2_stored =
				phy_utils_read_phyreg(pi, NPHY_Core2nbClipThreshold);
		}

		aci_prsnt_nbclip_threshold = 0x12;
		phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold, aci_prsnt_nbclip_threshold);
		phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold, aci_prsnt_nbclip_threshold);

		/* gain limit */
		/* change lna2gainchange value for ofdm and cck */
		if (!phyrev7to15) {
			wlc_phy_table_read_nphy(pi, 4, 4, 0x10, 16,
				pi->interf->init_ofdmlna2gainchange_stored);
			wlc_phy_table_read_nphy(pi, 4, 4, 0x50, 16,
				pi->interf->init_ccklna2gainchange_stored);
		}

		/* change only the 19th and 83rd values */
		regval[0] = 0x7f;
		wlc_phy_table_write_nphy(pi, 4, 1, 0x13, 16, &regval[0]);
		wlc_phy_table_write_nphy(pi, 4, 1, 0x53, 16, &regval[0]);

		/* clipLO gain */
		if (!phyrev7to15) {
			pi->interf->clip1_lo_gain_code_core1_stored =
				phy_utils_read_phyreg(pi, NPHY_Core1cliploGainCodeA2057);
			pi->interf->clip1_lo_gain_code_core2_stored =
				phy_utils_read_phyreg(pi, NPHY_Core2cliploGainCodeA2057);
		}

		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(NPHY, Core1cliploGainCodeA2057, 0x24)
			PHY_REG_WRITE_ENTRY(NPHY, Core2cliploGainCodeA2057, 0x24)
		PHY_REG_LIST_EXECUTE(pi);

		if (!phyrev7to15) {
			pi->interf->clip1_lo_gain_codeb_core1_stored =
				phy_utils_read_phyreg(pi, NPHY_Core1cliploGainCodeB2057);
			pi->interf->clip1_lo_gain_codeb_core2_stored =
				phy_utils_read_phyreg(pi, NPHY_Core2cliploGainCodeB2057);
		}
		phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057, 0x310 |
			(phy_utils_read_phyreg(pi, NPHY_Core1cliploGainCodeB2057) & 0xf));
		phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057, 0x310 |
			(phy_utils_read_phyreg(pi, NPHY_Core2cliploGainCodeB2057) & 0xf));

		/* RF rssi gain */
		if (!phyrev7to15) {
			pi->interf->radio_2057_core1_rssi_nb_gc_stored =
				phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE0) & 0x60;
			pi->interf->radio_2057_core2_rssi_nb_gc_stored =
				phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE1) & 0x60;
			pi->interf->radio_2057_core1_rssi_wb1a_gc_stored =
				phy_utils_read_radioreg(pi,
				RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0) & 0xc;
			pi->interf->radio_2057_core2_rssi_wb1a_gc_stored =
				phy_utils_read_radioreg(pi,
				RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1) & 0xc;
			pi->interf->radio_2057_core1_rssi_wb1g_gc_stored =
				phy_utils_read_radioreg(pi,
				RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0) & 0x3;
			pi->interf->radio_2057_core2_rssi_wb1g_gc_stored =
				phy_utils_read_radioreg(pi,
				RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1) & 0x3;
			pi->interf->radio_2057_core1_rssi_wb2_gc_stored =
				phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE0) & 0xc0;
			pi->interf->radio_2057_core2_rssi_wb2_gc_stored =
				phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE1) & 0xc0;
		}

		phy_utils_write_radioreg(pi, RADIO_2057_NB_MASTER_CORE0, 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE0) & 0x1f));
		phy_utils_write_radioreg(pi, RADIO_2057_NB_MASTER_CORE1, 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE1) & 0x1f));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0, 0x0 |
			(phy_utils_read_radioreg(pi,
		                RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0) & 0x33));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1, 0x0 |
			(phy_utils_read_radioreg(pi,
		                RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1) & 0x33));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0, 0x0 |
			(phy_utils_read_radioreg(pi,
		                RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0) & 0x3c));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1, 0x0 |
			(phy_utils_read_radioreg(pi,
		                RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1) & 0x3c));
		phy_utils_write_radioreg(pi, RADIO_2057_W2_MASTER_CORE0, 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE0) & 0x3f));
		phy_utils_write_radioreg(pi, RADIO_2057_W2_MASTER_CORE1, 0x40 |
			(phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE1) & 0x3f));

		/* w1 threshold */
		/* Adjust W1 Clip thresh based on gainctrl changes */
		if (!phyrev7to15) {
			pi->interf->w1_clip_thresh_core1_stored =
			        phy_utils_read_phyreg(pi, NPHY_Core1clipwbThreshold2057);
			pi->interf->w1_clip_thresh_core2_stored =
				phy_utils_read_phyreg(pi, NPHY_Core2clipwbThreshold2057);
		}
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057,
				clip1wbThreshold, NPHY_RSSICAL_W1_TARGET - 4)
			PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057,
				clip1wbThreshold, NPHY_RSSICAL_W1_TARGET - 4)
		PHY_REG_LIST_EXECUTE(pi);

		ed_lock = pi->edcrs_threshold_lock ||
		        NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV + 1); /* 43217 */

		/* ED CRS changes */
		if (!ed_lock) {
			PHY_REG_LIST_START
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs40AssertThresh0, 0x3eb)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs40AssertThresh1, 0x3eb)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs40DeassertThresh0, 0x341)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs40DeassertThresh1, 0x341)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LAssertThresh0, 0x42b)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LAssertThresh1, 0x42b)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LDeassertThresh0, 0x381)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LDeassertThresh1, 0x381)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh0, 0x42b)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh1, 0x42b)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh0, 0x381)
			        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh1, 0x381)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else {
		/* Disable ACI mitigation */
		if (pi_nphy->phyhang_avoid)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

		PHY_ACI(("Disable ACI mitigation, on channel %d\n",
			CHSPEC_CHANNEL(pi->radio_chanspec)));

		/* get back to detection, as soon as a previous detection fails */

		if (!phyrev7to15) {
			phy_utils_write_phyreg(pi, NPHY_energydroptimeoutLen,
				pi->interf->energy_drop_timeout_len_stored);
		}
		if (phyrev7to15) {
			band_switch =
			 (CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored) > 14 &&
			  CHSPEC_CHANNEL(pi->radio_chanspec) <= 14) ||
			 ((CHSPEC_CHANNEL(pi->interf->radio_chanspec_stored) <= 14) &&
			  (CHSPEC_CHANNEL(pi->radio_chanspec) > 14));
			bw_switch =
				((CHSPEC_IS20(pi->interf->radio_chanspec_stored) == 1) &&
				(CHSPEC_IS20(pi->radio_chanspec) == 0)) ||
				((CHSPEC_IS20(pi->interf->radio_chanspec_stored) == 0) &&
				(CHSPEC_IS20(pi->radio_chanspec) == 1));
		}

		/* restore clip hi gains */
		if ((pi->interf->init_gain_code_core1_stored == 0) ||
			(pi->interf->init_gain_code_core2_stored == 0) ||
			(pi->interf->init_gain_codeb_core1_stored == 0) ||
			(pi->interf->init_gain_codeb_core2_stored == 0)) {
			return;
		}
		if (!phyrev7to15 ||
			(phyrev7to15 && !(band_switch || bw_switch))) {
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057,
				pi->interf->clip1_hi_gain_code_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057,
				pi->interf->clip1_hi_gain_code_core2_stored);
			phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057,
				pi->interf->clip1_hi_gain_codeb_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057,
				pi->interf->clip1_hi_gain_codeb_core2_stored);
		}

		/* restore nb clip thresholds */
		if (!phyrev7to15 ||
			(phyrev7to15 && !(band_switch || bw_switch))) {
			phy_utils_write_phyreg(pi, NPHY_Core1nbClipThreshold,
				pi->interf->nb_clip_thresh_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2nbClipThreshold,
				pi->interf->nb_clip_thresh_core2_stored);

			/* restore gainlimits */
			wlc_phy_table_write_nphy(pi, 4, 4, 0x10, 16,
			   pi->interf->init_ofdmlna2gainchange_stored);
			wlc_phy_table_write_nphy(pi, 4, 4, 0x50, 16,
			   pi->interf->init_ccklna2gainchange_stored);
		}

		/* restore clip lo gain */
		if (!phyrev7to15 ||
			(phyrev7to15 && !(band_switch || bw_switch))) {
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeA2057,
				pi->interf->clip1_lo_gain_code_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeA2057,
				pi->interf->clip1_lo_gain_code_core2_stored);
			phy_utils_write_phyreg(pi, NPHY_Core1cliploGainCodeB2057,
				pi->interf->clip1_lo_gain_codeb_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2cliploGainCodeB2057,
				pi->interf->clip1_lo_gain_codeb_core2_stored);
		}

		/* restore rssi gain */
		phy_utils_write_radioreg(pi, RADIO_2057_NB_MASTER_CORE0,
			pi->interf->radio_2057_core1_rssi_nb_gc_stored |
			(phy_utils_read_radioreg(pi,
		               RADIO_2057_NB_MASTER_CORE0) & 0x1f));
		phy_utils_write_radioreg(pi, RADIO_2057_NB_MASTER_CORE1,
			pi->interf->radio_2057_core2_rssi_nb_gc_stored |
			(phy_utils_read_radioreg(pi,
		               RADIO_2057_NB_MASTER_CORE1) & 0x1f));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0,
			pi->interf->radio_2057_core1_rssi_wb1a_gc_stored |
			(phy_utils_read_radioreg(pi,
		               RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0)
			& 0x33));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1,
			pi->interf->radio_2057_core2_rssi_wb1a_gc_stored |
			(phy_utils_read_radioreg(pi,
		               RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1)
			& 0x33));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0,
			pi->interf->radio_2057_core1_rssi_wb1g_gc_stored |
			(phy_utils_read_radioreg(pi,
		                RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE0)
			& 0x3c));
		phy_utils_write_radioreg(pi,
		        RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1,
			pi->interf->radio_2057_core2_rssi_wb1g_gc_stored |
			(phy_utils_read_radioreg(pi,
		                RADIO_2057_RSSI_GPAIOSEL_W1_IDACS_CORE1)
			& 0x3c));
		phy_utils_write_radioreg(pi, RADIO_2057_W2_MASTER_CORE0,
			pi->interf->radio_2057_core1_rssi_wb2_gc_stored |
			(phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE0) &
		         0x3f));
		phy_utils_write_radioreg(pi, RADIO_2057_W2_MASTER_CORE1,
			pi->interf->radio_2057_core2_rssi_wb2_gc_stored |
			(phy_utils_read_radioreg(pi, RADIO_2057_W2_MASTER_CORE1) &
		         0x3f));

		/* Adjust W1 Clip thresh based on gainctrl changes */
		if (!phyrev7to15 ||
			(phyrev7to15 && !(band_switch || bw_switch))) {
			phy_utils_write_phyreg(pi, NPHY_Core1clipwbThreshold2057,
				pi->interf->w1_clip_thresh_core1_stored);
			phy_utils_write_phyreg(pi, NPHY_Core2clipwbThreshold2057,
				pi->interf->w1_clip_thresh_core2_stored);
		}

		/* restore ed values */
		if ((!pi->edcrs_threshold_lock) &&
			(!phyrev7to15 ||
			(phyrev7to15 && !(band_switch || bw_switch)))) {
		phy_utils_write_phyreg(pi, NPHY_ed_crs40AssertThresh0,
			pi->interf->ed_crs40_assertthld0_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs40AssertThresh1,
			pi->interf->ed_crs40_assertthld1_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs40DeassertThresh0,
			pi->interf->ed_crs40_deassertthld0_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs40DeassertThresh1,
			pi->interf->ed_crs40_deassertthld1_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20LAssertThresh0,
			pi->interf->ed_crs20L_assertthld0_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20LAssertThresh1,
			pi->interf->ed_crs20L_assertthld1_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20LDeassertThresh0,
			pi->interf->ed_crs20L_deassertthld0_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20LDeassertThresh1,
			pi->interf->ed_crs20L_deassertthld1_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UAssertThresh0,
			pi->interf->ed_crs20U_assertthld0_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UAssertThresh1,
			pi->interf->ed_crs20U_assertthld1_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UDeassertThresh0,
			pi->interf->ed_crs20U_deassertthld0_stored);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UDeassertThresh1,
			pi->interf->ed_crs20U_deassertthld1_stored);

		}

		if (pi_nphy->phyhang_avoid)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	}
}
#endif /* Compiling out ACI code */

void
wlc_nphy_deaf_mode(phy_info_t *pi, bool mode)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (mode) {
		if (pi_nphy->nphy_deaf_count == 0)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
		else
			PHY_ERROR(("%s: Deafness already set\n", __FUNCTION__));
	}
	else {
		if (pi_nphy->nphy_deaf_count > 0)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
		else
			PHY_ERROR(("%s: Deafness already cleared\n", __FUNCTION__));
	}
	wlapi_enable_mac(pi->sh->physhim);
}

bool
wlc_phy_get_deaf_nphy(phy_info_t *pi)
{
	uint16 curr_classifctl;
	uint16 curr_clipdet[NPHY_CORE_NUM];
	int core;
	bool isDeaf = TRUE;

	/* Get current classifier and clip_detect settings */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	curr_classifctl = phy_utils_read_phyreg(pi, NPHY_ClassifierCtrl) &
		NPHY_ClassifierCtrl_classifierSel_MASK;
	wlc_phy_clip_det_nphy(pi, 0, curr_clipdet);
	wlapi_enable_mac(pi->sh->physhim);

	if (curr_classifctl != 4) {
		isDeaf = FALSE;
	} else {
		FOREACH_CORE(pi, core) {
			if (curr_clipdet[core] != 0xffff) {
				isDeaf = FALSE;
				break;
			}
		}
	}
	return isDeaf;
}

#if defined(BCMDBG)
void
wlc_phy_setinitgain_nphy(phy_info_t *pi, uint16 init_gain)
{
	uint8 core;
	uint8 lna_gain, hpvga1_gain, hpvga2_gain;
	uint16 regval[NPHY_CORE_NUM];
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	lna_gain = (init_gain & 0x03);
	hpvga1_gain = (init_gain >> 4) & 0x0f;
	hpvga2_gain = (init_gain >> 8) & 0x1f;

	phy_utils_mod_phyreg(pi, NPHY_Core1InitGainCode,
		NPHY_CoreInitGainCode_initLnaIndex_MASK,
		(lna_gain << NPHY_CoreInitGainCode_initLnaIndex_SHIFT));
	phy_utils_mod_phyreg(pi, NPHY_Core1InitGainCode,
		NPHY_CoreInitGainCode_initHpvga1Index_MASK,
		(hpvga1_gain << NPHY_CoreInitGainCode_initHpvga1Index_SHIFT));
	phy_utils_mod_phyreg(pi, NPHY_Core1InitGainCode,
		NPHY_CoreInitGainCode_initHpvga2Index_MASK,
		(hpvga2_gain << NPHY_CoreInitGainCode_initHpvga2Index_SHIFT));

	phy_utils_mod_phyreg(pi, NPHY_Core2InitGainCode,
		NPHY_CoreInitGainCode_initLnaIndex_MASK,
		(lna_gain << NPHY_CoreInitGainCode_initLnaIndex_SHIFT));
	phy_utils_mod_phyreg(pi, NPHY_Core2InitGainCode,
		NPHY_CoreInitGainCode_initHpvga1Index_MASK,
		(hpvga1_gain << NPHY_CoreInitGainCode_initHpvga1Index_SHIFT));
	phy_utils_mod_phyreg(pi, NPHY_Core2InitGainCode,
		NPHY_CoreInitGainCode_initHpvga2Index_MASK,
		(hpvga2_gain << NPHY_CoreInitGainCode_initHpvga2Index_SHIFT));

	FOREACH_CORE(pi, core) {
		regval[core] = ((hpvga2_gain << 8) | (hpvga1_gain << 4) | (lna_gain << 2));
	}
	wlc_phy_table_write_nphy(pi, 7, pi->pubpi->phy_corenum, 0x106, 16, regval);

	wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

void
wlc_phy_sethpf1gaintbl_nphy(phy_info_t *pi, int8 maxindex)
{
	uint8 ctr;
	uint16 regval[NPHY_MAX_HPVGA1_INDEX+1];
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	if (maxindex == -1) {
		maxindex = NPHY_DEF_HPVGA1_INDEXLIMIT;
	} else if (maxindex > NPHY_MAX_HPVGA1_INDEX) {
		maxindex = NPHY_MAX_HPVGA1_INDEX;
	}

	/* Write to the HPVGA1 gaintable */
	for (ctr = 0; ctr <= maxindex; ctr++) {
	    regval[ctr] = ctr * 3;
	}

	/* Fill out the unwritten entries with the max gains */
	for (; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
	    regval[ctr] = maxindex * 3;
	}

	wlc_phy_table_write_nphy(pi, 0, NPHY_MAX_HPVGA1_INDEX+1, 16, 16, regval);
	wlc_phy_table_write_nphy(pi, 1, NPHY_MAX_HPVGA1_INDEX+1, 16, 16, regval);

	/* Write to the HPVGA1 gainbits table */
	for (ctr = 0; ctr <= maxindex; ctr++) {
	    regval[ctr] = ctr;
	}
	/* Fill out the unwritten entries with the max gainbits */
	for (; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
	    regval[ctr] = maxindex;
	}
	wlc_phy_table_write_nphy(pi, 2, NPHY_MAX_HPVGA1_INDEX+1, 16, 16, regval);
	wlc_phy_table_write_nphy(pi, 3, NPHY_MAX_HPVGA1_INDEX+1, 16, 16, regval);

	wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

#define NPHY_CAL_RESET_ALL	0x0
#define NPHY_CAL_RESET_TXIQ	0x1
#define NPHY_CAL_RESET_TXLO	0x2
#define NPHY_CAL_RESET_RXIQ	0x4

void
wlc_phy_cal_reset_nphy(phy_info_t *pi, uint32 reset_type)
{
	uint32 tbl_len;
	int coreNum;

	nphy_iq_comp_t rxcal_coeffs;

	uint16 tbl_tx_iqlo_cal_coeffs[] = {
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000
	};

	if ((reset_type == NPHY_CAL_RESET_ALL) || (reset_type & NPHY_CAL_RESET_TXIQ)) {
		tbl_len = ARRAYSIZE(tbl_tx_iqlo_cal_coeffs);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, tbl_len, 80,
			16, tbl_tx_iqlo_cal_coeffs);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, tbl_len, 88,
			16, tbl_tx_iqlo_cal_coeffs);
	}

	if ((reset_type == NPHY_CAL_RESET_ALL) || (reset_type & NPHY_CAL_RESET_TXLO)) {
		FOREACH_CORE(pi, coreNum) {
			/* Fine LOFT compensation */
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_I, 0x77);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_Q, 0x77);

			/* Coarse LOFT compensation */
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_I, 0x77);
			WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_Q, 0x77);
		}
	}

	if ((reset_type == NPHY_CAL_RESET_ALL) || (reset_type & NPHY_CAL_RESET_RXIQ)) {
		rxcal_coeffs.a0 = rxcal_coeffs.b0 = rxcal_coeffs.a1 = rxcal_coeffs.b1 = 0x0;
		wlc_phy_rx_iq_coeffs_nphy(pi, 1, &rxcal_coeffs);
	}
}
#endif /* defined(BCMDBG) */
#ifdef SAMPLE_COLLECT
int
phy_n_mac_triggered_sample_collect(phy_info_t *pi,
	wl_samplecollect_args_t *collect, uint32 *buf)
{
	phy_info_nphy_t *pi_nphy;
	uint32 phyctrl_val, timer_ctrl, phy_tmp, dur_1_8_us;
	int pstart1;

	ASSERT(pi);
	pi_nphy = (phy_info_nphy_t *)pi->u.pi_nphy;
	if (!pi_nphy)
		return BCME_ERROR;

	if (collect->trigger == TRIGGER_NOW)
		wlc_phy_set_deaf((wlc_phy_t *) pi, TRUE);

	phy_utils_write_phyreg(pi, NPHY_forceClk, 0x1F);

	/* initial return info pointers */
	wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP(pi), (int8)collect->trigger);

	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		/* For lcnxnphy mac trigger's needs this bit set */
		PHY_REG_MOD(pi, NPHY, AdcDataCollect, disable_data_coll_ctrl, 1);
	}
	dur_1_8_us = (collect->pre_dur << 3) + 240;
	/* WAR: 240usec additional delay */

	W_REG(pi->sh->osh, D11_TSF_GPT_2_CTR_L(pi), dur_1_8_us & 0xFFFF);
	W_REG(pi->sh->osh, D11_TSF_GPT_2_CTR_H(pi), dur_1_8_us >> 16);

	/* Mac triggers for lcnxnphy need this bit set for mac triggers */
	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		collect->mode |= (1 << 14);
	}
	wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_CTL(pi), collect->mode);

	phy_tmp = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
	W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_tmp | (1 << 4));

	phyctrl_val = 1;
	timer_ctrl = 0;

	while (phyctrl_val)
	{
		phy_tmp = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
		phyctrl_val = phy_tmp & 0x30;
		timer_ctrl++;
		if (timer_ctrl < 10000)
		{
			OSL_DELAY(100);
		}
		else
		{
			printf("sample_collect: Function terminated after 1s timeout %d",
			timer_ctrl);
			return BCME_ERROR;
		}
	   }
	  pi_nphy->plast = R_REG(pi->sh->osh, D11_SampleCollectCurPtr(pi));

	  pi_nphy->pstop = R_REG(pi->sh->osh, D11_SampleCollectStopPtr(pi));
	  pstart1 = pi_nphy->plast - (collect->pre_dur * 80);
	  if (pstart1 <  0)
	  {
		pstart1 = pi_nphy->pstop + pstart1;
	  }
	pi_nphy->pstart = (uint32)pstart1;
	return BCME_OK;
}
int
phy_n_mac_triggered_sample_data(phy_info_t *pi, wl_sampledata_t *sample_data, void *b)
{
	uint32 data;
	uint16 cnt, bufsize;
	phy_info_nphy_t *pi_nphy = (phy_info_nphy_t *)pi->u.pi_nphy;
	uint8* head = (uint8 *)b;
	uint32* buf = (uint32 *)(head + sizeof(wl_sampledata_t));

	sample_data->version = htol16(WL_SAMPLEDATA_T_VERSION);
	/* avoid buffer overrun */
	ASSERT(ltoh16(sample_data->length) >= sizeof(wl_sampledata_t));
	bufsize = ltoh16(sample_data->length) - sizeof(wl_sampledata_t);
	bufsize = bufsize >> 3;
	cnt = 0;
	wlapi_bmac_templateptr_wreg(pi->sh->physhim, pi_nphy->pstart << 2);
	while ((cnt < bufsize) && (pi_nphy->pstart != pi_nphy->plast))
	{
		data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
		buf[cnt++] = htol32(data);
		if (pi_nphy->pstart == pi_nphy->pstop)
		{
			pi_nphy->pstart = 0;
		}
		else
		{
			pi_nphy->pstart++;
		}
	}
	if (pi_nphy->pstart != pi_nphy->plast)
	{
		sample_data->flag |= htol32(WL_SAMPLEDATA_MORE_DATA);
	}
	else
	{
		sample_data->flag &= 0xFF;
		wlc_phy_clear_deaf((wlc_phy_t *) pi, TRUE);
		pi->phywatchdog_override = TRUE;
	}
	sample_data->length = htol16(cnt);
	bcopy((uint8 *)sample_data, head, sizeof(wl_sampledata_t));
	return BCME_OK;
}
#endif /* SAMPLE COLLECT */

#if defined(WLTEST)

/**
 * calibration sanity check, support RSSI only for now. Issues a warning in case the compensation
 * values are too large to reflect a non-broken chip/board.
 */
uint32
wlc_phy_cal_sanity_nphy(phy_info_t *pi)
{
	int8   offs_nb_0i, offs_nb_0q, offs_nb_1i, offs_nb_1q;
	int8   offs_w1_0i, offs_w1_0q, offs_w1_1i, offs_w1_1q;
	int8   offs_w2_0i, offs_w2_0q, offs_w2_1i, offs_w2_1q;
	uint8  flag_nb_0i = 0, flag_nb_0q = 0, flag_nb_1i = 0, flag_nb_1q = 0;
	uint8  flag_w1_0i = 0, flag_w1_0q = 0, flag_w1_1i = 0, flag_w1_1q = 0;
	uint8  flag_w2_0i = 0, flag_w2_0q = 0, flag_w2_1i = 0, flag_w2_1q = 0;
	uint16 bitmask_core0, bitmask_core1;

	PHY_CAL(("wl%d: %s: sanity check of radio cal results (RSSI only for now) ..\n",
	   pi->sh->unit, __FUNCTION__));

	/* read currently chosen offsets from PHY */
	/* NB RSSI */
	offs_nb_0i = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIZ));
	offs_nb_0q = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIZ));
	offs_nb_1i = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIZ));
	offs_nb_1q = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIZ));
	/* W1 RSSI */
	offs_w1_0i = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIX));
	offs_w1_0q = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIX));
	offs_w1_1i = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIX));
	offs_w1_1q = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIX));
	/* W2 RSSI */
	offs_w2_0i = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIY));
	offs_w2_0q = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIY));
	offs_w2_1i = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIY));
	offs_w2_1q = NPHY_RSSI_SXT(phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIY));

	/* display current compensation values */
	PHY_CAL(("offs_nb_0i =%4d, offs_nb_0q =%4d, offs_nb_1i =%4d, offs_nb_1q =%4d\n",
		offs_nb_0i, offs_nb_0q, offs_nb_1i, offs_nb_1q));
	PHY_CAL(("offs_w1_0i =%4d, offs_w1_0q =%4d, offs_w1_1i =%4d, offs_w1_1q =%4d\n",
		offs_w1_0i, offs_w1_0q, offs_w1_1i, offs_w1_1q));
	PHY_CAL(("offs_w2_0i =%4d, offs_w2_0q =%4d, offs_w2_1i =%4d, offs_w2_1q =%4d\n",
		offs_w2_0i, offs_w2_0q, offs_w2_1i, offs_w2_1q));

	/* sanity check NB-RSSI */
	if (NPHY_RSSI_NB_VIOL(offs_nb_0i)) {
		PHY_INFORM(("NB-RSSI, Core-0, I-Rail: Offset too large!\n"));
		flag_nb_0i = 1;
	}
	if (NPHY_RSSI_NB_VIOL(offs_nb_0q)) {
		PHY_INFORM(("NB-RSSI, Core-0, Q-Rail: Offset too large!\n"));
		flag_nb_0q = 1;
	}
	if (NPHY_RSSI_NB_VIOL(offs_nb_1i)) {
		PHY_INFORM(("NB-RSSI, Core-1, I-Rail: Offset too large!\n"));
		flag_nb_1i = 1;
	}
	if (NPHY_RSSI_NB_VIOL(offs_nb_1q)) {
		PHY_INFORM(("NB-RSSI, Core-1, Q-Rail: Offset too large!\n"));
		flag_nb_1q = 1;
	}

	/* sanity check W1-RSSI */
	if (NPHY_RSSI_W1_VIOL(offs_w1_0i)) {
		PHY_INFORM(("W1-RSSI, Core-0, I-Rail: Offset too large!\n"));
		flag_w1_0i = 1;
	}
	if (NPHY_RSSI_W1_VIOL(offs_w1_0q)) {
		PHY_INFORM(("W1-RSSI, Core-0, Q-Rail: Offset too large!\n"));
		flag_w1_0q = 1;
	}
	if (NPHY_RSSI_W1_VIOL(offs_w1_1i)) {
		PHY_INFORM(("W1-RSSI, Core-1, I-Rail: Offset too large!\n"));
		flag_w1_1i = 1;
	}
	if (NPHY_RSSI_W1_VIOL(offs_w1_1q)) {
		PHY_INFORM(("W1-RSSI, Core-1, Q-Rail: Offset too large!\n"));
		flag_w1_1q = 1;
	}

	/* sanity check W2-RSSI */
	if (NPHY_RSSI_W2_VIOL(offs_w2_0i)) {
		PHY_INFORM(("W2-RSSI, Core-0, I-Rail: Offset too large!\n"));
		flag_w2_0i = 1;
	}
	if (NPHY_RSSI_W2_VIOL(offs_w2_0q)) {
		PHY_INFORM(("W2-RSSI, Core-0, Q-Rail: Offset too large!\n"));
		flag_w2_0q = 1;
	}
	if (NPHY_RSSI_W2_VIOL(offs_w2_1i)) {
		PHY_INFORM(("W2-RSSI, Core-1, I-Rail: Offset too large!\n"));
		flag_w2_1i = 1;
	}
	if (NPHY_RSSI_W2_VIOL(offs_w2_1q)) {
		PHY_INFORM(("W2-RSSI, Core-1, Q-Rail: Offset too large!\n"));
		flag_w2_1q = 1;
	}

	/* generate bitmasks
	 *   note for future enhancements: bit 2^31 has to be zero due
	 *   to later cast to int32 (otherwise need to fix at higher level);
	 *   could arrange bits in a different order (eg: 1 nibble = 4 cores
	 *   for given quantity)
	 */
	bitmask_core0 = (flag_w2_0q << 5) | (flag_w2_0i << 4) | (flag_w1_0q << 3) |
		(flag_w1_0i << 2) | (flag_nb_0q << 1) | (flag_nb_0i << 0);
	bitmask_core1 = (flag_w2_1q << 5) | (flag_w2_1i << 4) | (flag_w1_1q << 3) |
		(flag_w1_1i << 2) | (flag_nb_1q << 1) | (flag_nb_1i << 0);

	/* final diagnosis */
	PHY_CAL(("%s\n", ((bitmask_core0 != 0) || (bitmask_core1 != 0)) ?
		"*** bad cal, Your chip/board may be broken! ***" : "cal results seem o.k"));

	/* return bitmasks */
	return ((uint32) ((bitmask_core1 << 16) | (bitmask_core0 << 0)));

}

void
wlc_phy_bphy_testpattern_nphy(phy_info_t *pi, uint8 testpattern, bool enable, bool existing_enable)
{
	uint16 clip_off[] = {0xffff, 0xffff};

	/* Turn ON BPHY testpattern */
	if ((enable == TRUE) && (existing_enable == FALSE)) {

		/* Save existing clip_detect state */
		wlc_phy_clip_det_nphy(pi, 0, pi->phy_clip_state);
		/* Turn OFF all clip detections */
		wlc_phy_clip_det_nphy(pi, 1, clip_off);
		/* Save existing classifier state */
		pi->phy_classifier_state = wlc_phy_classifier_nphy(pi, 0, 0);
		/* Turn OFF all classifcation */
		wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK, 4);
		/* Trigger SamplePlay Start */
		phy_utils_write_phyreg(pi,  NPHY_sampleCmd, 0x1);
		/* Save BPHY test and testcontrol registers */
		pi->old_bphy_test = phy_utils_read_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST));
		pi->old_bphy_testcontrol = phy_utils_read_phyreg(pi, NPHY_bphytestcontrol);
		if (testpattern == 0) {
			phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), 0x0038);
		} else {
			phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), 0x0228);
		}
		/* Configure mimophy's bphy testcontrol */
		phy_utils_write_phyreg(pi, NPHY_bphytestcontrol, 0x7);
		/* Force Rx2Tx */
		 wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RX2TX);
		/* Force AFE into DAC mode */
		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_dac_pd_MASK,
				1 << NPHY_REV3_AfectrlOverride_dac_pd_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_dac_pd_MASK,
				1 << NPHY_REV3_AfectrlOverride_dac_pd_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_dac_pd_MASK,
				0 << NPHY_REV3_AfectrlCore_dac_pd_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_dac_pd_MASK,
				0 << NPHY_REV3_AfectrlCore_dac_pd_SHIFT)
		PHY_REG_LIST_EXECUTE(pi);

		PHY_CAL(("wl%d: %s: Turning  ON: TestPattern = %3d  "
		         "en = %3d  existing_en = %3d\n",
		         pi->sh->unit, __FUNCTION__, testpattern, enable, existing_enable));
	} else if ((enable == FALSE) && existing_enable) {
		/* Turn OFF BPHY testpattern */

		/* Turn off AFE Override */
		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride1,
				NPHY_REV3_AfectrlOverride_dac_pd_MASK,
				0 << NPHY_REV3_AfectrlOverride_dac_pd_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlOverride2,
				NPHY_REV3_AfectrlOverride_dac_pd_MASK,
				0 << NPHY_REV3_AfectrlOverride_dac_pd_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore1,
				NPHY_REV3_AfectrlCore_dac_pd_MASK,
				1 << NPHY_REV3_AfectrlCore_dac_pd_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_AfectrlCore2,
				NPHY_REV3_AfectrlCore_dac_pd_MASK,
				1 << NPHY_REV3_AfectrlCore_dac_pd_SHIFT)
		PHY_REG_LIST_EXECUTE(pi);

		/* Force Tx2Rx */
		wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_TX2RX);
		/* Restore BPHY test and testcontrol registers */
		phy_utils_write_phyreg(pi, NPHY_bphytestcontrol, pi->old_bphy_testcontrol);
		phy_utils_write_phyreg(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), pi->old_bphy_test);
		/* Turn ON receive packet activity */
		wlc_phy_clip_det_nphy(pi, 1, pi->phy_clip_state);
		wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
		                        pi->phy_classifier_state);

		/* Trigger samplePlay Stop */
		phy_utils_write_phyreg(pi,  NPHY_sampleCmd, NPHY_sampleCmd_STOP);

		PHY_CAL(("wl%d: %s: Turning OFF: TestPattern = %3d  "
		         "en = %3d  existing_en = %3d\n",
		         pi->sh->unit, __FUNCTION__, testpattern, enable, existing_enable));
	}
}

void
wlc_phy_test_scraminit_nphy(phy_info_t *pi, int8 init)
{
	uint16 mask, value;

	if (init < 0) {
		/* auto: clear Mode bit so that scrambler LFSR will be free
		 * running.  ok to leave scramindexctlEn and initState in
		 * whatever current condition, since their contents are unused
		 * when free running, but for ease of reg diffs, just write
		 * 0x7f to them for repeatability.
		 */
		mask = (NPHY_ScramSigCtrl_scramCtrlMode_MASK |
		        NPHY_ScramSigCtrl_scramindexctlEn_MASK |
		        NPHY_ScramSigCtrl_initStateValue_MASK);
		value = ((0 << NPHY_ScramSigCtrl_scramCtrlMode_SHIFT) |
		         NPHY_ScramSigCtrl_initStateValue_MASK);
		phy_utils_mod_phyreg(pi, NPHY_ScramSigCtrl, mask, value);
	} else {
		/* fixed init: set Mode bit, clear scramindexctlEn, and write
		 * init to initState, so that scrambler LFSR will be
		 * initialized with specified value for each transmission.
		 */
		mask = (NPHY_ScramSigCtrl_scramCtrlMode_MASK |
		        NPHY_ScramSigCtrl_scramindexctlEn_MASK |
		        NPHY_ScramSigCtrl_initStateValue_MASK);
		value = (NPHY_ScramSigCtrl_scramCtrlMode_MASK |
		         (NPHY_ScramSigCtrl_initStateValue_MASK &
		          (init << NPHY_ScramSigCtrl_initStateValue_SHIFT)));
		phy_utils_mod_phyreg(pi, NPHY_ScramSigCtrl, mask, value);
	}
}

int8
wlc_phy_test_tssi_nphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs)
{
	int8 tssi = -127;

	if (ctrl_type >= 0) {
		if ((ctrl_type & 0x1) == 0)
		{
			tssi = phy_utils_read_phyreg(pi, NPHY_TSSIBiasVal1) & 0xff;
		} else {
			tssi = phy_utils_read_phyreg(pi, NPHY_TSSIBiasVal2) & 0xff;
		}
	}
	return (tssi);
}

void
wlc_phy_gpiosel_nphy(phy_info_t *pi, uint16 sel)
{
	uint32 mc;

	/*
	 * see http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/MimophyGPIOUsage
	 * following mimophy gpiosel modes provided:
	 *	 0 : state machine trace
	 *	 1 : crs tx/rx ctrl signal
	 *	 2 : antenna IQ samples
	 *	 3 : antenna IQ samples when pkt_proc state == SAMPLE
	 *	 4 : ADC IQ sample (9:2)
	 *	 5 : RSSI IQ sample
	 *	 ... etc
	 */

	/* for MIMOPHY, must kill the mimophy_oe */
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, gpioLoOutEn, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, gpioHiOutEn, 0x0)
	PHY_REG_LIST_EXECUTE(pi);

	/* take over gpio control from cc */
	si_gpiocontrol(pi->sh->sih, 0xffff, 0xffff, GPIO_DRV_PRIORITY);

	/* clear the mac selects, disable mac oe */
	mc = R_REG(pi->sh->osh, D11_MACCONTROL(pi));
	mc &= ~MCTL_GPOUT_SEL_MASK;
	W_REG(pi->sh->osh, D11_MACCONTROL(pi), mc);
	W_REG(pi->sh->osh, D11_PSM_GPIOEN(pi), 0x0);

	/* set up mimophy GPIO sel */
	phy_utils_write_phyreg(pi, NPHY_gpioSel, sel);

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, gpioLoOutEn, 0xffff)
		PHY_REG_WRITE_ENTRY(NPHY, gpioHiOutEn, 0xffff)
	PHY_REG_LIST_EXECUTE(pi);
}

#endif // endif

#ifdef PHYMON
int
wlc_phycal_state_nphy(phy_info_t *pi, void* buff, int len)
{
	wl_phycal_state_t phycal_state;
	wl_phycal_core_state_t *phycal_core_state;
	int i;
	uchar *buf = (uchar *) buff;
	int used_len = 0;
	nphy_iq_comp_t rxcal_coeffs;
	int16 txcal_ofdm_coeffs[8];
	int16 txcal_bphy_coeffs[8];
	uint16 idle_tssi, pwrctrl_status;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	int ret = BCME_OK;

	phycal_core_state = (wl_phycal_core_state_t *)MALLOC(pi->sh->osh,
		sizeof(wl_phycal_core_state_t) * 2);

	if (!pi->sh->up) {
		ret = BCME_ERROR;
		goto error;
	}

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	phycal_state.version = PHYMON_VERSION;
	phycal_state.num_phy_cores = 2;
	phycal_state.curr_temperature = (int8)wlc_phy_tempsense_nphy(pi);
	phycal_state.aci_state = (pi->aci_state == ACI_ACTIVE)? TRUE : FALSE;
	phycal_state.crsminpower = phy_utils_read_phyreg(pi, NPHY_crsminpower0) & 0xff;
	phycal_state.crsminpowerl = phy_utils_read_phyreg(pi, NPHY_crsminpowerl0) & 0xff;
	phycal_state.crsminpoweru = phy_utils_read_phyreg(pi, NPHY_crsminpoweru0) & 0xff;
	phycal_state.chspec = pi->radio_chanspec;

	/* Per-core states */
	phycal_core_state[0].tx_iqlocal_pwridx  = pi_nphy->nphy_txcal_pwr_idx[0];
	phycal_core_state[1].tx_iqlocal_pwridx  = pi_nphy->nphy_txcal_pwr_idx[1];

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_EPSILONTBL0,
	    NPHY_PAPD_EPS_TBL_SIZE,
	    0, 32, phycal_core_state[0].papd_epsilon_table);
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_EPSILONTBL1,
	    NPHY_PAPD_EPS_TBL_SIZE,
	    0, 32, phycal_core_state[1].papd_epsilon_table);
	phycal_core_state[0].papd_epsilon_offset =
		phy_utils_read_phyreg(pi, NPHY_EpsilonTableAdjust0);
	phycal_core_state[1].papd_epsilon_offset =
		phy_utils_read_phyreg(pi, NPHY_EpsilonTableAdjust1);

	idle_tssi = phy_utils_read_phyreg(pi, NPHY_TxPwrCtrlIdleTssi);
	phycal_core_state[0].idle_tssi =
		(idle_tssi >> NPHY_TxPwrCtrlIdleTssi_idleTssi0_SHIFT) & 0x3f;
	phycal_core_state[1].idle_tssi =
		(idle_tssi >> NPHY_TxPwrCtrlIdleTssi_idleTssi1_SHIFT) & 0x3f;

	pwrctrl_status = phy_utils_read_phyreg(pi, NPHY_Core0TxPwrCtrlStatus);
	phycal_core_state[0].est_tx_pwr = (uint8) (pwrctrl_status & 0xff);
	phycal_core_state[0].curr_tx_pwrindex = (uint8) ((pwrctrl_status >> 8) & 0x7f);
	pwrctrl_status = phy_utils_read_phyreg(pi, NPHY_Core1TxPwrCtrlStatus);
	phycal_core_state[1].est_tx_pwr = (uint8) (pwrctrl_status & 0xff);
	phycal_core_state[1].curr_tx_pwrindex = (uint8) ((pwrctrl_status >> 8) & 0x7f);

	phycal_core_state[0].rx_gaininfo = phy_utils_read_phyreg(pi, NPHY_PhyStatsGainInfo0);
	phycal_core_state[1].rx_gaininfo = phy_utils_read_phyreg(pi, NPHY_PhyStatsGainInfo1);

	phycal_core_state[0].init_gaincode = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2056);
	phycal_core_state[1].init_gaincode = phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeA2056);

	/* See wlc_dump_rssi() in wlc.c to extract this information */
	phycal_core_state[0].est_rx_pwr = 0;
	phycal_core_state[1].est_rx_pwr = 0;

	/* Read Rx calibration co-efficients */
	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &rxcal_coeffs);

	/* Read OFDM Tx calibration co-efficients */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 80, 16, txcal_ofdm_coeffs);

	/* Read BPHY Tx calibration co-efficients */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 88, 16, txcal_bphy_coeffs);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	/* reg access is done, enable the mac */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	txcal_ofdm_coeffs[0] &= 0x3ff;
	txcal_ofdm_coeffs[1] &= 0x3ff;
	txcal_ofdm_coeffs[2] &= 0x3ff;
	txcal_ofdm_coeffs[3] &= 0x3ff;

	if (txcal_ofdm_coeffs[0] > 511)
		txcal_ofdm_coeffs[0] -= 1024;

	if (txcal_ofdm_coeffs[1] > 511)
		txcal_ofdm_coeffs[1] -= 1024;

	if (txcal_ofdm_coeffs[2] > 511)
		txcal_ofdm_coeffs[2] -= 1024;

	if (txcal_ofdm_coeffs[3] > 511)
		txcal_ofdm_coeffs[3] -= 1024;

	phycal_core_state[0].tx_iqlocal_a = txcal_ofdm_coeffs[0];
	phycal_core_state[0].tx_iqlocal_b = txcal_ofdm_coeffs[1];
	phycal_core_state[1].tx_iqlocal_a = txcal_ofdm_coeffs[2];
	phycal_core_state[1].tx_iqlocal_b = txcal_ofdm_coeffs[3];

	/* No longer support RADIO 2056 */
	phycal_core_state[0].tx_iqlocal_ei = 0;
	phycal_core_state[0].tx_iqlocal_eq = 0;
	phycal_core_state[0].tx_iqlocal_fi = 0;
	phycal_core_state[0].tx_iqlocal_fq = 0;

	phycal_core_state[1].tx_iqlocal_ei = 0;
	phycal_core_state[1].tx_iqlocal_eq = 0;
	phycal_core_state[1].tx_iqlocal_fi = 0;
	phycal_core_state[1].tx_iqlocal_fq = 0;

	/* These don't exist for NREV >= 3 */
	phycal_core_state[0].tx_iqlocal_ci = 0;
	phycal_core_state[0].tx_iqlocal_cq = 0;
	phycal_core_state[1].tx_iqlocal_ci = 0;
	phycal_core_state[1].tx_iqlocal_cq = 0;

	phycal_core_state[0].tx_iqlocal_di = (int8) ((txcal_ofdm_coeffs[5] & 0xFF00) >> 8);
	phycal_core_state[0].tx_iqlocal_dq = (int8) (txcal_ofdm_coeffs[5] & 0x00FF);
	phycal_core_state[1].tx_iqlocal_di = (int8) ((txcal_ofdm_coeffs[6] & 0xFF00) >> 8);
	phycal_core_state[1].tx_iqlocal_dq = (int8) (txcal_ofdm_coeffs[6] & 0x00FF);

	/* Rx calibration coefficients are 10-bit signed integers */
	if (rxcal_coeffs.a0 > 511)
		rxcal_coeffs.a0 -= 1024;

	if (rxcal_coeffs.b0 > 511)
		rxcal_coeffs.b0 -= 1024;

	if (rxcal_coeffs.a1 > 511)
		rxcal_coeffs.a1 -= 1024;

	if (rxcal_coeffs.b1 > 511)
		rxcal_coeffs.b1 -= 1024;

	phycal_core_state[0].rx_iqcal_a = rxcal_coeffs.a0;
	phycal_core_state[0].rx_iqcal_b = rxcal_coeffs.b0;
	phycal_core_state[1].rx_iqcal_a = rxcal_coeffs.a1;
	phycal_core_state[1].rx_iqcal_b = rxcal_coeffs.b1;

	phycal_core_state[0].estirr_tx = 0;
	phycal_core_state[0].estirr_rx = 0;
	phycal_core_state[1].estirr_tx = 0;
	phycal_core_state[1].estirr_rx = 0;

	bcopy(&phycal_state, buf, sizeof(wl_phycal_state_t));

	buf += WL_PHYCAL_STAT_FIXED_LEN;
	used_len += WL_PHYCAL_STAT_FIXED_LEN;

	for (i = 0; i < 2; i++) {
		used_len += sizeof(wl_phycal_core_state_t);
		if (used_len > len) {
			ret = BCME_BUFTOOSHORT;
			goto error;
		}

		bcopy(&phycal_core_state[i], buf, sizeof(wl_phycal_core_state_t));
		buf += sizeof(wl_phycal_core_state_t);
	}

error:
	MFREE(pi->sh->osh, phycal_core_state, sizeof(wl_phycal_core_state_t) * 2);
	return ret;
}
#endif /* PHYMON */

void
wlc_phy_workarounds_nphy_rev17(phy_info_t *pi)
{
	int32 tbl_buf = 0x14;
	int32 tbl_buf1 = 0x10;

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, EngCtrl, 0x1)
		/* set Fine Timing Thresholds */
		PHY_REG_MOD_ENTRY(NPHY, FSTRHiPwrTh, finestr_hiPwr_th, 0x40)
	PHY_REG_LIST_EXECUTE(pi);

	/* lower quams */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1, 264, 32, &tbl_buf);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1, 265, 32, &tbl_buf);

	/* Higher quams */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1, 266, 32, &tbl_buf1);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_NOISEVAR, 1, 267, 32, &tbl_buf1);
}

static void
wlc_phy_workarounds_nphy_gainctrl_2057_rev13(phy_info_t *pi)
{
	const int8  lna1_gain_db[] = {10, 15, 19, 25};
	const int8  lna2_gain_db[] = {-1, 8, 12, 16};
	const int8  tia_gain_db[]  = {-4, -1, 2, 5, 5, 5, 5, 5, 5, 5};
	const int8  tia_gainbits[] = {0x0, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
	uint8 rfseq_updategainu_events[] = {
		NPHY_REV3_RFSEQ_CMD_RX_GAIN,
		NPHY_REV3_RFSEQ_CMD_CLR_HIQ_DIS,
		NPHY_REV3_RFSEQ_CMD_SET_LPF_L_HPC
	};
	uint8 rfseq_updategainu_dlys[] = {6, 15, 1};

	PHY_REG_LIST_START
		/* disable clip2 detect until clip2 is characterized properly */
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		/* set Fine Timing Thresholds */
		PHY_REG_MOD_ENTRY(NPHY, FSTRHiPwrTh, finestr_hiPwr_th, 65)
		/* set bphy crsminpower to 70(dec) for 20Mhz and 40Mhz */
		PHY_REG_MOD_ENTRY(NPHY, bphycrsminpower0, bphycrsminpower0, 0x46)
		/* set crsminpwr */
		PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x48)
		PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x48)
		PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower1, 0x48)
		PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower1, 0x48)
	PHY_REG_LIST_EXECUTE(pi);

	/* LNA1 Gainstep */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1_gain_db);

	/* LNA2 Gainstep */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2_gain_db);

	/* TIA Gain */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, tia_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, tia_gain_db);

	/* TIA Gainbits */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8,
	                         tia_gainbits);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8,
	                         tia_gainbits);

	/* Moved trisolation setting into a function */
	/* Need to be reused for LCNXNPHY */
	wlc_phy_adjust_ClipLO_for_triso_2057_rev5_rev9(pi);

	PHY_REG_LIST_START
		/* NB Clip = 0xe8 */
		PHY_REG_WRITE_ENTRY(NPHY, Core1nbClipThreshold, 0xe8)
		PHY_REG_WRITE_ENTRY(NPHY, Core2nbClipThreshold, 0xe8)
		/* w1 clip */
		PHY_REG_MOD_ENTRY(NPHY, Core1clipwbThreshold2057, clip1wbThreshold, 0x16)
		PHY_REG_MOD_ENTRY(NPHY, Core2clipwbThreshold2057, clip1wbThreshold, 0x16)
	PHY_REG_LIST_EXECUTE(pi);

	/* decrease pkt gain timing */
	wlc_phy_set_rfseq_nphy(pi, NPHY_RFSEQ_UPDATEGAINU,
	                       rfseq_updategainu_events,
	                       rfseq_updategainu_dlys,
	                       sizeof(rfseq_updategainu_events) /
	                       sizeof(rfseq_updategainu_events[0]));

	/* enable ADC preclip and increase the counter */
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, ADC_PreClip_Enable, 0x1)
		PHY_REG_WRITE_ENTRY(NPHY, ADC_PreClip1_CtrLen, 0x20)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_phy_workarounds_nphy_gainctrl_2057_rev14(phy_info_t *pi)
{
	int8  lna1_gain_db[] = {8, 15, 20, 25};
	int8  lna2_gain_db[] = {-2, 8, 12, 16};
	int8  tia_gain_db[]  = {-4, -1, 2, 5, 5, 5, 5, 5, 5, 5};
	int8  tia_gainbits[] = {0x0, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
	int16 temp;
	uint8 rfseq_updategainu_events[] = {
		NPHY_REV3_RFSEQ_CMD_RX_GAIN,
		NPHY_REV3_RFSEQ_CMD_CLR_HIQ_DIS,
		NPHY_REV3_RFSEQ_CMD_SET_LPF_L_HPC
	};
	uint8 rfseq_updategainu_dlys[] = {6, 15, 1};
	int16 new_lna1_gain1, new_lna1_gain2;
	uint16 lna1_gain1, lna1_gain2;
	int16 new_biq1_gain1, new_biq1_gain2;
	uint16 biq1_gain1, biq1_gain2;
	uint16 currgain_init1, currgain_init2;
	uint16 curgain_rfseq1, curgain_rfseq2, newgainarray[2];
	uint16 aci_prsnt_clip1hi_gaincode;
	uint16 aci_prsnt_clip1hi_gaincodeb;

	PHY_REG_LIST_START
		/* disable clip2 detect until clip2 is characterized properly */
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core1computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		PHY_REG_MOD_RAW_ENTRY(NPHY_Core2computeGainInfo,
			NPHY_CorecomputeGainInfo_disableClip2detect_MASK,
			1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)
		/* set Fine Timing Thresholds */
		PHY_REG_MOD_ENTRY(NPHY, FSTRHiPwrTh, finestr_hiPwr_th, 65)
		/* set bphy crsminpower to 70(dec) for 20Mhz and 40Mhz */
		PHY_REG_MOD_ENTRY(NPHY, bphycrsminpower0, bphycrsminpower0, 0x46)
		/* set crsminpwr */
		PHY_REG_MOD_ENTRY(NPHY, crsminpoweru0, crsminpower0, 0x45)
		PHY_REG_MOD_ENTRY(NPHY, crsminpowerl0, crsminpower0, 0x45)
	PHY_REG_LIST_EXECUTE(pi);

	/* LNA1 Gainstep */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x8, 8, lna1_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x8, 8, lna1_gain_db);

	/* LNA2 Gainstep */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 4, 0x10, 8, lna2_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 4, 0x10, 8, lna2_gain_db);

	/* TIA Gain */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN1, 10, 0x20, 8, tia_gain_db);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAIN2, 10, 0x20, 8, tia_gain_db);

	/* TIA Gainbits */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS1, 10, 0x20, 8,
	                         tia_gainbits);
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_GAINBITS2, 10, 0x20, 8,
	                         tia_gainbits);

	/* Moved trisolation setting into a function */
	/* Need to be reused for LCNXNPHY */
	wlc_phy_adjust_ClipLO_for_triso_2057_rev5_rev9(pi);

	if (pi->pubpi->radiover == 0) {
		phy_utils_or_phyreg(pi, NPHY_Core1cliploGainCodeA2057, 0x6);
		phy_utils_or_phyreg(pi, NPHY_Core2cliploGainCodeA2057, 0x6);

		temp = phy_utils_read_phyreg(pi, NPHY_crsThreshold1u);
		temp = (temp & 0xff00) | 0x00de;
		phy_utils_write_phyreg(pi, NPHY_crsThreshold1u, temp);

		temp = phy_utils_read_phyreg(pi, NPHY_crsThreshold1l);
		temp = (temp & 0xff00) | 0x00de;
		phy_utils_write_phyreg(pi, NPHY_crsThreshold1l, temp);
	} else if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
		temp = phy_utils_read_phyreg(pi, NPHY_crsThreshold1u);
		temp = (temp & 0xff00) | 0x00e0;
		phy_utils_write_phyreg(pi, NPHY_crsThreshold1u, temp);
		temp = phy_utils_read_phyreg(pi, NPHY_crsThreshold1l);
		temp = (temp & 0xff00) | 0x00e0;
		phy_utils_write_phyreg(pi, NPHY_crsThreshold1l, temp);

		/* Step2 lower LNA1 code by 1 and higher BIQ1 by 2 for initiGain */
		lna1_gain1 =
			(uint16) ((phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2057)) & 0x6);
		lna1_gain2 =
			(uint16) ((phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeA2057)) & 0x6);
		lna1_gain1 = ((uint16) lna1_gain1 >>1);
		lna1_gain2 = ((uint16) lna1_gain2 >>1);
		new_lna1_gain1 = (int16) lna1_gain1 - 1;
		new_lna1_gain2 = (int16) lna1_gain2 - 1;
		if (new_lna1_gain1 < 0)
			new_lna1_gain1 = 0;
		if (new_lna1_gain2 < 0)
			new_lna1_gain2 = 0;
		currgain_init1 = (uint16) phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeA2057);
		currgain_init2 = (uint16) phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeA2057);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeA2057,
			(((uint16) new_lna1_gain1) <<1) | (currgain_init1 & 0xfff9));
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeA2057,
			(((uint16) new_lna1_gain2) <<1) | (currgain_init2 & 0xfff9));

		biq1_gain1 = (uint16) ((phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057))
			>>NPHY_Core1InitGainCodeB2057_initbiq1gainIndex_SHIFT);
		biq1_gain2 = (uint16) ((phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057))
			>>NPHY_Core1InitGainCodeB2057_initbiq1gainIndex_SHIFT);
		new_biq1_gain1 = (int16) biq1_gain1 + 2;
		new_biq1_gain2 = (int16) biq1_gain2 + 2;
		if (new_biq1_gain1 > 15)
			new_biq1_gain1 = 15;
		if (new_biq1_gain2 > 15)
			new_biq1_gain2 = 15;
		currgain_init1 = (uint16) phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
		currgain_init2 = (uint16) phy_utils_read_phyreg(pi, NPHY_Core2InitGainCodeB2057);
		phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057,
			(((uint16) new_biq1_gain1) <<8) | (currgain_init1 & 0xff));
		phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057,
			(((uint16) new_biq1_gain2) <<8) | (currgain_init2 & 0xff));

		wlc_phy_table_read_nphy(pi, 7, 1, 0x106, 16, &curgain_rfseq1);
		wlc_phy_table_read_nphy(pi, 7, 1, 0x107, 16, &curgain_rfseq2);
		newgainarray[0] = (((uint16) new_biq1_gain1)<<12) | ((uint16) new_lna1_gain1) |
			(curgain_rfseq1 & 0xffc);
		newgainarray[1] = (((uint16) new_biq1_gain2)<<12) | ((uint16) new_lna1_gain2) |
			(curgain_rfseq2 & 0xffc);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, newgainarray);

		/* Step3 Reuse cliphiGain in ACI mode */
		/* use lna1 = 0x2, lna2 = 0x1, mixer = 1, */
		/* lpf-b0 = 2, lpf-b1=2, dvga = 0 */
		aci_prsnt_clip1hi_gaincode = 0x2c;
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeA2057,
			aci_prsnt_clip1hi_gaincode);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeA2057,
			aci_prsnt_clip1hi_gaincode);
		aci_prsnt_clip1hi_gaincodeb = 0x220 |
			(phy_utils_read_phyreg(pi,
			NPHY_Core1clipHiGainCodeB2057) & 0xf);
		phy_utils_write_phyreg(pi, NPHY_Core1clipHiGainCodeB2057,
			aci_prsnt_clip1hi_gaincodeb);
		aci_prsnt_clip1hi_gaincodeb = 0x220 |
			(phy_utils_read_phyreg(pi,
			NPHY_Core2clipHiGainCodeB2057) & 0xf);
		phy_utils_write_phyreg(pi, NPHY_Core2clipHiGainCodeB2057,
			aci_prsnt_clip1hi_gaincodeb);
	}

	PHY_REG_LIST_START
	        /* Here is for CE Adaptivity test v1.8.1 (interference+blocker) */
	        /* Step1 ed_crs threshold =-65/-71 */
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LAssertThresh0, 0x3eb)
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LAssertThresh1, 0x3eb)
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LDeassertThresh0, 0x36c)
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20LDeassertThresh1, 0x36c)
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh0, 0x3eb)
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh1, 0x3eb)
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh0, 0x36c)
	        PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh1, 0x36c)

		/* NB Clip = 0xe8 */
		PHY_REG_WRITE_ENTRY(NPHY, Core1nbClipThreshold, 0x98)
		PHY_REG_WRITE_ENTRY(NPHY, Core2nbClipThreshold, 0x98)

		PHY_REG_WRITE_ENTRY(NPHY, Core1clipmdGainCodeB2057, 0x44)
		PHY_REG_WRITE_ENTRY(NPHY, Core2clipmdGainCodeB2057, 0x44)

	PHY_REG_LIST_EXECUTE(pi);

	/* w1 clip */
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
		PHY_REG_MOD(pi, NPHY, Core1clipwbThreshold2057, clip1wbThreshold, 0x2e);
		PHY_REG_MOD(pi, NPHY, Core2clipwbThreshold2057, clip1wbThreshold, 0x2e);
	} else {
		PHY_REG_MOD(pi, NPHY, Core1clipwbThreshold2057, clip1wbThreshold, 0x16);
		PHY_REG_MOD(pi, NPHY, Core2clipwbThreshold2057, clip1wbThreshold, 0x16);
	}
	/* decrease pkt gain timing */
	wlc_phy_set_rfseq_nphy(pi, NPHY_RFSEQ_UPDATEGAINU,
	                       rfseq_updategainu_events,
	                       rfseq_updategainu_dlys,
	                       sizeof(rfseq_updategainu_events) /
	                       sizeof(rfseq_updategainu_events[0]));

	/* enable ADC preclip and increase the counter */
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, ADC_PreClip_Enable, 0x1)
		PHY_REG_WRITE_ENTRY(NPHY, ADC_PreClip1_CtrLen, 0x34)
	PHY_REG_LIST_EXECUTE(pi);
}

static
int16 wlc_phy_get_tssi_pwr_limit_nphy(int16 a1, int16 b0, int16 b1, uint8 maxlimit, uint8 offset)
{
	int16 tssi, pwr, prev_pwr;
	int16 tssi_pwr_limit_nphy;
	uint8 tssi_ladder_cnt = 0;

	if (maxlimit) {
		tssi_pwr_limit_nphy = prev_pwr = (1<<14) - 1;
		for (tssi = 0; tssi < 63; tssi++) {
			pwr = wlc_phy_tssi2qtrdbm_nphy(tssi, a1, b0, b1, 4);
			if (pwr < prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					tssi_pwr_limit_nphy = pwr;
					break;
				}
			}
		}
	} else {
		tssi_pwr_limit_nphy = prev_pwr = (1<<15);
		for (tssi = 63; tssi >= 0; tssi--) {
			pwr = wlc_phy_tssi2qtrdbm_nphy(tssi, a1, b0, b1, 4);
			if (pwr > prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					tssi_pwr_limit_nphy = pwr;
					break;
				}
			}
		}
	}
	return tssi_pwr_limit_nphy;
}

#define MAX_EST_PWR_VAL 255
static int16 wlc_phy_tssi2qtrdbm_nphy(int16 tssi, int16 a1, int16 b0, int16 b1, uint8 mult)
{
	int32 num, den;
	int16 est_pwr;
	num = 8 * (16 * b0 + b1 * tssi);
	den = 32768 + a1 * tssi;
	est_pwr = MAX(((mult * num + den/2)/den), -(2 * mult)); /* not same as tcl */
	/* Ensure that est_pwr does not exceed 255 and wrap around */
	est_pwr = MIN(est_pwr, MAX_EST_PWR_VAL);
	return est_pwr;
}

static void
wlc_phy_set_tssi_pwr_limit_nphy(phy_info_t *pi, int16 *a1, int16 *b0, int16 *b1, uint8 mode)
{
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	int16 maxpwr_limit[NPHY_CORE_NUM];
	int16 minpwr_limit[NPHY_CORE_NUM];
	int core;

	FOREACH_CORE(pi, core) {
		maxpwr_limit[core] =
			wlc_phy_get_tssi_pwr_limit_nphy(
				*a1, *b0, *b1, 1, pi_nphy->tssi_ladder_offset_maxpwr);
		minpwr_limit[core] =
			wlc_phy_get_tssi_pwr_limit_nphy(
				*a1++, *b0++, *b1++, 0, pi_nphy->tssi_ladder_offset_minpwr);
	}
	if ((mode == NPHY_TSSI_SET_MAX_LIMIT) || (mode == NPHY_TSSI_SET_MIN_MAX_LIMIT)) {
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
			pi_nphy->tssi_maxpwr_limit = MIN(maxpwr_limit[0], maxpwr_limit[1]);
		else
			pi_nphy->tssi_maxpwr_limit = maxpwr_limit[0];
	}

	if ((mode == NPHY_TSSI_SET_MIN_LIMIT) || (mode == NPHY_TSSI_SET_MIN_MAX_LIMIT)) {
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
			pi_nphy->tssi_minpwr_limit = MAX(minpwr_limit[0], minpwr_limit[1]);
		else
			pi_nphy->tssi_minpwr_limit = minpwr_limit[0];
	}
}

#if defined(RXDESENS_EN)
#define USE_BOTH_BIQS 0 /* 1: use both biq's | 0: use biq1 alone */
int
wlc_nphy_set_rxdesens(wlc_phy_t *ppi, int32 int_val)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	uint16 regval[3];
	uint16 x1, x2, x3, y1, y2, y3, tot_biq;

	uint16 bphy_crsminpwr_lut[] = {0x46, 0x7b, 0x8b,
		0x9b, 0xab, 0xbb, 0xcb, 0xdb, 0xeb, 0xfb};

	uint32 init_gain = 0x0;

	if (!pi->sh->up)
		return BCME_NOTUP;

	if ((int_val < 0) || (int_val > 40))
		return BCME_RANGE;

	if (pi_nphy->ntd_crs_adjusted == FALSE) {
		pi_nphy->ntd_initgain = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
		regval[0] = phy_utils_read_phyreg(pi, NPHY_crsminpoweru0);
		pi_nphy->ntd_crsminpwr[0] = regval[0] & 0xff;
		pi_nphy->ntd_crs_adjusted = TRUE;
	}

	if (((pi_nphy->ntd_initgain>>8) & 0xf) != 6) {
		pi_nphy->ntd_crs_adjusted = FALSE;
		return BCME_ERROR;
	}

	/* Initial gain is reduced up to 18dB. The remaining part goes to crsminpower */
	x1 = (int_val > 18) ? 18 : int_val;
	y1 = (int_val > 18) ? (int_val-18) : 0;

	/* update initial gain */
	x1 = x1/3 + ((x1%3)>>1);
	x2 = ((pi_nphy->ntd_initgain>>8) & 0xf)- x1;

	regval[0] = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
	regval[0] = (regval[0] & ~(0xf<<8)) | (x2<<8);
	phy_utils_write_phyreg(pi, NPHY_Core1InitGainCodeB2057, regval[0]);
	phy_utils_write_phyreg(pi, NPHY_Core2InitGainCodeB2057, regval[0]);

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phy_table_read_nphy(pi, 7, 1, 0x106, 16, &regval[0]);
	regval[0] = (regval[0] & ~(0xf<<12)) | (x2<<12);
	wlc_phy_table_write_nphy(pi, 7, 1, 0x106, 16, &regval[0]);
	wlc_phy_table_write_nphy(pi, 7, 1, 0x107, 16, &regval[0]);
	wlapi_enable_mac(pi->sh->physhim);

	/* force rfseq */
	wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);

	/* update crsmin power */
	y1 = (y1<< 3) / 3;
	y2 = (pi_nphy->ntd_crsminpwr[0] + y1);

	regval[0] = phy_utils_read_phyreg(pi, NPHY_crsminpowerl0);
	regval[0] = (regval[0] & 0xff00) | y2;
	phy_utils_write_phyreg(pi, NPHY_crsminpowerl0, regval[0]);

	regval[0] = phy_utils_read_phyreg(pi, NPHY_crsminpoweru0);
	regval[0] = (regval[0] & 0xff00) | y2;
	phy_utils_write_phyreg(pi, NPHY_crsminpoweru0, regval[0]);

	return BCME_OK;
}
#endif /* defined(RXDESENS_EN) */

/**
 * Setup/Cleanup routine for high-pass corner (HPC) of LPF:
 * 1) Setup: Save LPF config and set HPC to lowest value (0x1)
 * 2) Cleanup: Restore HPC config
 */
void
wlc_phy_lpf_hpc_override_nphy(phy_info_t *pi, bool setup_not_cleanup)
{
	phy_info_nphy_t *pi_nphy;
	pi_nphy = (phy_info_nphy_t *)pi->u.pi_nphy;
	if (pi_nphy == NULL)
		return;
	if (setup_not_cleanup) {
		 /* Phy "Setup" */

		ASSERT(!pi_nphy->is_orig);
		pi_nphy->is_orig = TRUE;
	} else {

		ASSERT(pi_nphy->is_orig);
		pi_nphy->is_orig = FALSE;
	}
}

int16
wlc_phy_rxgaincode_to_dB_nphy(phy_info_t *pi, uint16 gain_code)
{
	int8 lna1_code, lna2_code, mixtia_code, biq0_code, biq1_code;
	int8 lna1_gain, lna2_gain, mixtia_gain, biq0_gain, biq1_gain;
	int8 hpvga_code, hpvga_gain;
	uint16 TR_loss;
	int16 total_gain;

	/* Extract gain codes for each gain element from overall gain code: */
	lna1_code = gain_code & 0x3;
	lna2_code = (gain_code >> 2) & 0x3;
	mixtia_code = (gain_code >> 4) & 0xf;
	biq0_code = (gain_code >> 8) & 0x3;
	biq1_code = (gain_code >> 10) & 0x3;
	hpvga_code = (gain_code >> 12) & 0xf;

	/* Look up gains for lna1, lna2 and mixtia from indices: */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_GAIN1, 1, (0x8 + lna1_code), 8, &lna1_gain);
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_GAIN1, 1, (0x10 + lna2_code), 8, &lna2_gain);
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_GAIN1, 1, (0x20 + mixtia_code), 8, &mixtia_gain);

	/* Biquad gains: */
	biq0_gain = 6 * biq0_code;
	biq1_gain = 6 * biq1_code;
	hpvga_gain = 3 * hpvga_code;

	/* Need to subtract out TR_loss in Rx mode: */
	TR_loss = (phy_utils_read_phyreg(pi, NPHY_TRLossValue)) & 0x7f;

	/* Total gain: */
	total_gain = lna1_gain + lna2_gain + mixtia_gain + biq0_gain +
		biq1_gain + hpvga_gain - TR_loss;

	return total_gain;
}

static void wlc_phy_ant_force_nphy(phy_info_t *pi, uint8 *entries)
{
	uint16 AntIdx;
	uint16 offset_band;
	uint16 offset_core[] = {0, 32};
	uint8 tmp[8];
	int num_core;
	int idx;

	offset_band = CHSPEC_IS2G(pi->radio_chanspec) ? 0 : 16;
	AntIdx = (uint16)READ_PHYREG3(pi, NPHY, RfctrlAntSwLUTIdx, AntConfig);
	AntIdx = AntIdx << 2;
	num_core = sizeof(offset_core)/sizeof(offset_core[0]);
	for (idx = 0; idx < num_core; idx++) {
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT, 4,
		                        offset_band + offset_core[idx], 8, &entries[4 * idx]);
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT, 4,
		                        offset_band + offset_core[idx] + AntIdx, 8,
		                        &tmp[4 * idx]);
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT, 4,
		                         offset_band + offset_core[idx], 8, &tmp[4 * idx]);
	}
}

static void wlc_phy_ant_release_nphy(phy_info_t *pi, uint8* entries)
{
	uint16 offset_band;
	uint16 offset_core[] = {0, 32};
	int num_core;
	int idx;
	offset_band = CHSPEC_IS2G(pi->radio_chanspec) ? 0 : 16;
	num_core = sizeof(offset_core)/sizeof(offset_core[0]);
	for (idx = 0; idx < num_core; idx++) {
		wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_ANTSWCTRLLUT, 4,
		                         offset_band + offset_core[idx], 8, &entries[4 * idx]);
	}
}

/**
 *  reports estimated power and adjusted estimated power in quarter dBms.
 *    32-bit output consists of estimated power of cores 0 and 1 in MSbyte and in the 2nd MSbyte,
 *    respectively, and the estimated power cores 0 and 1, adjusted using the power offsets in the
 *    adjPwrLut, are in the 3rd MSbyte and the LSbyte, respectively. When estimated power or
 *    adjusted estimated power is not valid, 0x80 is returned for that core
 */
uint32
wlc_phy_txpower_est_power_nphy(phy_info_t *pi)
{
	int16 tx0_status, tx1_status;
	uint16 estPower1, estPower2;
	uint8 pwr0, pwr1, adj_pwr0, adj_pwr1;
	uint32 est_pwr;

	/* Read the Actual Estimated Powers without adjustment */
	estPower1 = phy_utils_read_phyreg(pi, NPHY_EstPower1);
	estPower2 = phy_utils_read_phyreg(pi, NPHY_EstPower2);

	if ((estPower1 & NPHY_EstPower1_estPowerValid_MASK)
	       == NPHY_EstPower1_estPowerValid_MASK) {
		pwr0 = (uint8) (estPower1 & NPHY_EstPower1_estPower_MASK)
		                      >> NPHY_EstPower1_estPower_SHIFT;
	} else {
		pwr0 = 0x80;
	}

	if ((estPower2 & NPHY_EstPower2_estPowerValid_MASK)
	       == NPHY_EstPower2_estPowerValid_MASK) {
		pwr1 = (uint8) (estPower2 & NPHY_EstPower2_estPower_MASK)
		                      >> NPHY_EstPower2_estPower_SHIFT;
	} else {
		pwr1 = 0x80;
	}

	/* Read the Adjusted Estimated Powers */
	tx0_status = phy_utils_read_phyreg(pi, NPHY_Core0TxPwrCtrlStatus);
	tx1_status = phy_utils_read_phyreg(pi, NPHY_Core1TxPwrCtrlStatus);

	if ((tx0_status & NPHY_Core1TxPwrCtrlStatus_estPwrValid_MASK)
	       == NPHY_Core1TxPwrCtrlStatus_estPwrValid_MASK) {
		adj_pwr0 = (uint8) (tx0_status & NPHY_Core1TxPwrCtrlStatus_estPwr_MASK)
		                      >> NPHY_Core1TxPwrCtrlStatus_estPwr_SHIFT;
	} else {
		adj_pwr0 = 0x80;
	}
	if ((tx1_status & NPHY_Core2TxPwrCtrlStatus_estPwrValid_MASK)
	       == NPHY_Core2TxPwrCtrlStatus_estPwrValid_MASK) {
		adj_pwr1 = (uint8) (tx1_status & NPHY_Core2TxPwrCtrlStatus_estPwr_MASK)
		                      >> NPHY_Core2TxPwrCtrlStatus_estPwr_SHIFT;
	} else {
		adj_pwr1 = 0x80;
	}

	est_pwr = (uint32) ((pwr0 << 24) | (pwr1 << 16) | (adj_pwr0 << 8) | adj_pwr1);
	return (est_pwr);
}

#ifdef RXIQCAL_FW_WAR
int
wlc_phy_cal_rxiq_nphy_fw_war(phy_info_t *pi, nphy_txgains_t target_gain,
	uint8 cal_type, bool debug, uint8 core_mask)
{
	int32 temp = 0;
	int32 sin_phi = 0;
	int32 cos_phi = 0;
	int32 core0_a, core0_bplus1, gain_mm_core0;
	int32 core1_a, core1_bplus1, gain_mm_core1;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	nphy_iq_comp_t rxiq_phase_mm;
	nphy_iq_comp_t rxiq_gain_mm;
	nphy_iq_comp_t new_comp;

	/* override bt priority */
	wlc_btcx_override_enable(pi);

	/* getting the phase mismatch metric */
	if (wlc_phy_cal_rxiq_nphy(pi, target_gain, 0, debug, core_mask) == BCME_OK) {
		wlc_phy_savecal_nphy(pi);
	}

	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &rxiq_phase_mm);
	if (rxiq_phase_mm.a0 > 511)
		rxiq_phase_mm.a0 -= 1024;
	if (rxiq_phase_mm.b0 > 511)
		rxiq_phase_mm.b0 -= 1024;
	if (rxiq_phase_mm.a1 > 511)
		rxiq_phase_mm.a1 -= 1024;
	if (rxiq_phase_mm.b1 > 511)
		rxiq_phase_mm.b1 -= 1024;

	/* getting the gain mismatch metric */
	if (wlc_phy_cal_rxiq_nphy(pi, target_gain, 0, debug, core_mask) == BCME_OK) {
		wlc_phy_savecal_nphy(pi);
	}

	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &rxiq_gain_mm);
	if (rxiq_gain_mm.a0 > 511)
		rxiq_gain_mm.a0 -= 1024;
	if (rxiq_gain_mm.b0 > 511)
		rxiq_gain_mm.b0 -= 1024;
	if (rxiq_gain_mm.a1 > 511)
		rxiq_gain_mm.a1 -= 1024;
	if (rxiq_gain_mm.b1 > 511)
		rxiq_gain_mm.b1 -= 1024;

	PHY_CAL(("phase mm : a0 %d; b0 %d; a1 %d; b1 %d\n",
	rxiq_phase_mm.a0, rxiq_phase_mm.b0, rxiq_phase_mm.a1, rxiq_phase_mm.b1));
	PHY_CAL((" gain mm : a0 %d; b0 %d; a1 %d; b1 %d\n",
	rxiq_gain_mm.a0, rxiq_gain_mm.b0, rxiq_gain_mm.a1, rxiq_gain_mm.b1));

	core0_a = rxiq_phase_mm.a0;
	core0_bplus1 = rxiq_phase_mm.b0 + 1024;

	/* compute sin(phi) & cos(phi) */
	/* where phi = atan(core0_a/core0_bplus1) */
	temp = (((core0_a * core0_a + core0_bplus1 * core0_bplus1) << 10) /
		(core0_bplus1 * core0_bplus1));
	sin_phi = (((core0_a << 18) / core0_bplus1) / (int32)math_sqrt_int_32(temp));
	cos_phi = ((1 << 18) / (int32)math_sqrt_int_32(temp));

	/* compute gain_mm (alpha) */
	core0_a = rxiq_gain_mm.a0;
	core0_bplus1 = rxiq_gain_mm.b0 + 1024;
	temp = ((core0_a * core0_a) + (core0_bplus1 * core0_bplus1));
	gain_mm_core0 = math_sqrt_int_32(temp);
	PHY_CAL(("gain_mm_core0 %d\n", gain_mm_core0));

	/* a0 = alpha*sin(phi) */
	new_comp.a0 = (gain_mm_core0 * sin_phi) >> 13;
	if (new_comp.a0 < -512)
		new_comp.a0 += 1024;
	else if (new_comp.a0 > 511)
		new_comp.a0 -= 1024;
	/* b0 = alpha*cos(phi) */
	new_comp.b0 = ((gain_mm_core0 * cos_phi) >> 13) - 1024;
	if (new_comp.b0 < -512)
		new_comp.b0 += 1024;
	else if (new_comp.b0 > 511)
		new_comp.b0 -= 1024;

	core1_a = rxiq_phase_mm.a1;
	core1_bplus1 = rxiq_phase_mm.b1 + 1024;

	/* compute sin(phi) & cos(phi) */
	/* where phi = atan(core1_a/core1_bplus1) */
	temp = (((core1_a * core1_a + core1_bplus1 * core1_bplus1) << 10) /
		(core1_bplus1 * core1_bplus1));
	sin_phi = (((core1_a << 18) / core1_bplus1) / (int32)math_sqrt_int_32(temp));
	cos_phi = ((1 << 18) / (int32)math_sqrt_int_32(temp));

	/* compute gain_mm (alpha) */
	core1_a = rxiq_gain_mm.a1;
	core1_bplus1 = rxiq_gain_mm.b1 + 1024;
	temp = ((core1_a * core1_a) + (core1_bplus1 * core1_bplus1));
	/* returns sqrt(temp) */
	gain_mm_core1 = math_sqrt_int_32(temp);
	PHY_CAL(("gain_mm_core1 %d\n", gain_mm_core1));

	/* a0 = alpha*sin(phi) */
	new_comp.a1 = (gain_mm_core1 * sin_phi) >> 13;
	if (new_comp.a1 < -512)
		new_comp.a1 += 1024;
	else if (new_comp.a1 > 511)
		new_comp.a1 -= 1024;
	/* b0 = alpha*cos(phi) */
	new_comp.b1 = ((gain_mm_core1 * cos_phi) >> 13) - 1024;
	if (new_comp.b1 < -512)
		new_comp.b1 += 1024;
	else if (new_comp.b1 > 511)
		new_comp.b1 -= 1024;

	/* write back rx_iqcc */
	wlc_phy_rx_iq_coeffs_nphy(pi, 1, &new_comp);
	PHY_CAL(("new comp : a0 %d; b0 %d; a1 %d; b1 %d\n",
		new_comp.a0, new_comp.b0, new_comp.a1, new_comp.b1));

	/* remove override */
	wlc_phy_btcx_override_disable(pi);

	return BCME_OK;
}
#endif /* rxiqcal fw war */

void
wlc_phy_get_bbmult_nphy(phy_info_t *pi, int32* ret_ptr)
{
	uint16 m0m1;

	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 87, 16, &m0m1);

	*ret_ptr = (uint32)m0m1;
}

void
wlc_phy_set_bbmult_nphy(phy_info_t *pi, uint8 m0, uint8 m1)
{
	uint16 m0m1;

	m0m1 = (uint16)(m0 << 8) | m1;

	/* OFDM phy */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 87, 16, &m0m1);
	/* CCK phy */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 1, 95, 16, &m0m1);
}

/** phy_oclscdenable IOVAR fn */
void
wlc_phy_set_oclscd_nphy(phy_info_t *pi)
{
	uint16 val;

	/* MAC should be suspended before calling this function */
	ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));

	/* beDeaf */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	phy_utils_mod_phyreg(pi, NPHY_OCLControl1, NPHY_OCLControl1_mode_enable_MASK, 0);
	phy_utils_mod_phyreg(pi, NPHY_OCLControl1, NPHY_OCLControl1_ofdm_scd_shutOff_enable_MASK,
		0);

	/* turn on FGC */
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

	/* resetcca */
	val = phy_utils_read_phyreg(pi, NPHY_BBConfig);
	phy_utils_write_phyreg(pi, NPHY_BBConfig, val | BBCFG_RESETCCA);
	OSL_DELAY(1);
	phy_utils_write_phyreg(pi, NPHY_BBConfig, val & (~BBCFG_RESETCCA));

	/* turn off FGC */
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);

	/* returnFromDeaf */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static bool
wlc_phy_watchdog_nphy(phy_wd_ctx_t *ctx)
{
	phy_info_t *pi = (phy_info_t *) ctx;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	ASSERT(pi_nphy != NULL);
	BCM_REFERENCE(pi_nphy);

	if (!pi->disable_percal) {
		if (!(phy_papdcal_is_wfd_phy_ll_enable(pi->papdcali) && DCS_INPROG_PHY(pi))) {

			/* 1) check to see if new cal needs to be activated */
			if ((pi->phy_cal_mode != PHY_PERICAL_DISABLE) &&
				(pi->phy_cal_mode != PHY_PERICAL_MANUAL) &&
				((pi->sh->now - pi->cal_info->last_cal_time) >=
				pi->sh->glacial_timer)) {
					wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_PERICAL_WATCHDOG);

					if (phy_papdcal_is_wfd_phy_ll_enable(pi->papdcali))
						pi->cal_info->last_cal_time = pi->sh->now;
			}

			if (!phy_papdcal_is_wfd_phy_ll_enable(pi->papdcali)) {
				wlc_phy_txpwr_papd_cal_nphy(pi);
			}

			wlc_phy_radio205x_check_vco_cal_nphy(pi);
		}
	}

	return TRUE;
}

static void
wlc_phy_set_srom_eu_edthresh_nphy(phy_info_t *pi)
{
	int32 eu_edthresh;

	eu_edthresh = CHSPEC_IS2G(pi->radio_chanspec) ? pi->srom_eu_edthresh2g :
	        pi->srom_eu_edthresh5g;
	/* edthresh = 0 & 0xff(-1) are invalid values */
	if (eu_edthresh < -10)
		wlc_phy_adjust_ed_thres(pi, &eu_edthresh, TRUE);
}

static bool
BCMATTACHFN(wlc_phy_txpwr_srom8_read)(phy_info_t *pi)
{

	/* read in antenna-related config */
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, rstr_aa2g);

#ifdef BAND5G
	pi->aa5g = (uint8) PHY_GETINTVAR(pi, rstr_aa5g);
#endif /* BAND5G */

	/* read in FEM stuff */
	pi->fem2g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos2g);
	pi->fem2g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain2g);
	pi->fem2g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange2g);
	pi->fem2g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso2g);
	pi->fem2g->antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl2g);

#ifdef BAND5G
	pi->fem5g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos5g);
	pi->fem5g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain5g);
	pi->fem5g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange5g);
	pi->fem5g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso5g);

	/* If antswctl5g entry exists, use it.
	 * Fallback to antswctl2g value if 5g entry does not exist.
	 * Previous code used 2g value only, thus...
	 * this is a WAR for any legacy NVRAMs that only had a 2g entry.
	 */
	pi->fem5g->antswctrllut = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_antswctl5g,
		PHY_GETINTVAR(pi, rstr_antswctl2g));
#endif /* BAND5G */

	if (PHY_GETVAR(pi, rstr_elna2g)) {
		/* extlnagain2g entry exists, so use it. */
		pi->u.pi_nphy->elna2g = (uint8)PHY_GETINTVAR(pi, rstr_elna2g);

	}
#ifdef BAND5G
	if (PHY_GETVAR(pi, rstr_elna5g)) {
		/* extlnagain5g entry exists, so use it. */
		pi->u.pi_nphy->elna5g = (uint8)PHY_GETINTVAR(pi, rstr_elna5g);
	}
#endif /* BAND5G */

	pi->phy_tempsense_offset = (int8)PHY_GETINTVAR(pi, rstr_tempoffset);
	if (pi->phy_tempsense_offset != 0) {
		if (pi->phy_tempsense_offset >
			(NPHY_SROM_TEMPSHIFT + NPHY_SROM_MAXTEMPOFFSET)) {
			pi->phy_tempsense_offset = NPHY_SROM_MAXTEMPOFFSET;
		} else if (pi->phy_tempsense_offset < (NPHY_SROM_TEMPSHIFT +
			NPHY_SROM_MINTEMPOFFSET)) {
			pi->phy_tempsense_offset = NPHY_SROM_MINTEMPOFFSET;
		} else {
			pi->phy_tempsense_offset -= NPHY_SROM_TEMPSHIFT;
		}
	}

	/* Power per Rate */
	wlc_phy_txpwr_srom8_read_ppr(pi);

	return TRUE;

}

static void
BCMATTACHFN(wlc_phy_txpwr_srom8_read_ppr)(phy_info_t *pi)
{
		uint16 bw40po, cddpo, stbcpo, bwduppo;
		int band_num;
		phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
		srom_pwrdet_t	*pwrdet  = pi->pwrdet;

		/* Read bw40po value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		bw40po = (uint16)PHY_GETINTVAR(pi, rstr_bw40po);
		pi->ppr->u.sr8.bw40[WL_CHAN_FREQ_RANGE_2G] = bw40po & 0xf;
#ifdef BAND5G
		pi->ppr->u.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND0] = (bw40po & 0xf0) >> 4;
		pi->ppr->u.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND1] = (bw40po & 0xf00) >> 8;
		pi->ppr->u.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND2] = (bw40po & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr->u.sr8.bw40[WL_CHAN_FREQ_RANGE_5G_BAND3] =
				(bw40po & 0xf000) >> 12;
#endif /* BAND5G */
		/* Read cddpo value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		cddpo = (uint16)PHY_GETINTVAR(pi, rstr_cddpo);
		pi->ppr->u.sr8.cdd[WL_CHAN_FREQ_RANGE_2G] = cddpo & 0xf;
#ifdef BAND5G
		pi->ppr->u.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND0]  = (cddpo & 0xf0) >> 4;
		pi->ppr->u.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND1]  = (cddpo & 0xf00) >> 8;
		pi->ppr->u.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND2]  = (cddpo & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr->u.sr8.cdd[WL_CHAN_FREQ_RANGE_5G_BAND3]  = (cddpo & 0xf000) >> 12;
#endif /* BAND5G */

		/* Read stbcpo value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		stbcpo = (uint16)PHY_GETINTVAR(pi, rstr_stbcpo);
		pi->ppr->u.sr8.stbc[WL_CHAN_FREQ_RANGE_2G] = stbcpo & 0xf;
#ifdef BAND5G
		pi->ppr->u.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND0]  = (stbcpo & 0xf0) >> 4;
		pi->ppr->u.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND1]  = (stbcpo & 0xf00) >> 8;
		pi->ppr->u.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND2]  = (stbcpo & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr->u.sr8.stbc[WL_CHAN_FREQ_RANGE_5G_BAND3]  = (stbcpo & 0xf000) >> 12;
#endif /* BAND5G */

		/* Read bwduppo value
		 * each nibble corresponds to 2g, 5g, 5gl, 5gh specific offset respectively
		 */
		bwduppo = (uint16)PHY_GETINTVAR(pi, rstr_bwduppo);
		pi->ppr->u.sr8.bwdup[WL_CHAN_FREQ_RANGE_2G] = bwduppo & 0xf;
#ifdef BAND5G
		pi->ppr->u.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND0]  = (bwduppo & 0xf0) >> 4;
		pi->ppr->u.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND1]  = (bwduppo & 0xf00) >> 8;
		pi->ppr->u.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND2]  = (bwduppo & 0xf000) >> 12;
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			pi->ppr->u.sr8.bwdup[WL_CHAN_FREQ_RANGE_5G_BAND3]  =
			(bwduppo & 0xf000) >> 12;
#endif /* BAND5G */

		for (band_num = 0; band_num < NUMSUBBANDS(pi); band_num++) {
			switch (band_num) {
				case WL_CHAN_FREQ_RANGE_2G:
					/* 2G band */
					pi_nphy->nphy_txpid2g[PHY_CORE_0]  =
					(uint8)PHY_GETINTVAR(pi, rstr_txpid2ga0);
					pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
					idle_targ_2g =
					(int8)PHY_GETINTVAR(pi, rstr_itt2ga0);

					pwrdet->max_pwr[PHY_CORE_0][band_num] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a0);

					if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
						pi_nphy->nphy_txpid2g[PHY_CORE_1]  =
						(uint8)PHY_GETINTVAR(pi, rstr_txpid2ga1);
						pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
						idle_targ_2g =
						(int8)PHY_GETINTVAR(pi, rstr_itt2ga1);
						pwrdet->max_pwr[PHY_CORE_1][band_num] =
							(int8)PHY_GETINTVAR(pi, rstr_maxp2ga1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a1);
					}
					if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
						pwrdet->max_pwr[PHY_CORE_2][band_num] =
							(int8)PHY_GETINTVAR(pi, rstr_maxp2ga2);
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a2);
					}

					/* 2G CCK */
					pi->ppr->u.sr8.cck2gpo =
						(uint16)PHY_GETINTVAR(pi, rstr_cck2gpo);

					/* 2G ofdm2gpo power offsets */
					pi->ppr->u.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm2gpo);

					/* 2G mcs2gpo power offsets */
					pi->ppr->u.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo0);
					pi->ppr->u.sr8.mcs[band_num][1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo1);
					pi->ppr->u.sr8.mcs[band_num][2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo2);
					pi->ppr->u.sr8.mcs[band_num][3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo3);
					pi->ppr->u.sr8.mcs[band_num][4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo4);
					pi->ppr->u.sr8.mcs[band_num][5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo5);
					pi->ppr->u.sr8.mcs[band_num][6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo6);
					pi->ppr->u.sr8.mcs[band_num][7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo7);
					break;
#ifdef BAND5G
				case WL_CHAN_FREQ_RANGE_5G_BAND0:
					/* 5G lowband */
					pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num] =
					(uint8)PHY_GETINTVAR(pi, rstr_txpid5gla0);
					pwrdet->max_pwr[PHY_CORE_0][band_num]  =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gla0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a0);

					if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
						pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
						= (uint8)PHY_GETINTVAR(pi, rstr_txpid5gla1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a1);
						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gla1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a1);
					}
					if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a2);
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gla2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a2);
					}

					pi_nphy->nphy_pwrctrl_info[0].
					idle_targ_5g[band_num] = 0;
					pi_nphy->nphy_pwrctrl_info[1].
					idle_targ_5g[band_num] = 0;

					/* 5G lowband ofdm5glpo power offsets */
					pi->ppr->u.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5glpo);

					/* 5G lowband mcs5glpo power offsets */
					pi->ppr->u.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo0);
					pi->ppr->u.sr8.mcs[band_num] [1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo1);
					pi->ppr->u.sr8.mcs[band_num] [2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo2);
					pi->ppr->u.sr8.mcs[band_num] [3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo3);
					pi->ppr->u.sr8.mcs[band_num] [4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo4);
					pi->ppr->u.sr8.mcs[band_num] [5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo5);
					pi->ppr->u.sr8.mcs[band_num] [6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo6);
					pi->ppr->u.sr8.mcs[band_num] [7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5glpo7);
					break;
				case WL_CHAN_FREQ_RANGE_5G_BAND1:
					/* 5G band, mid */
					pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num]  =
					(uint8)PHY_GETINTVAR(pi, rstr_txpid5ga0);
					pwrdet->max_pwr[PHY_CORE_0][band_num] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a0);

					if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
						pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
						= (uint8)PHY_GETINTVAR(pi, rstr_txpid5ga1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a1);
						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5ga1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a1);
					}
					if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a2);
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5ga2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a2);
					}

					pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
					idle_targ_5g[band_num] =
					(int8)PHY_GETINTVAR(pi, rstr_itt5ga0);
					pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
					idle_targ_5g[band_num] =
					(int8)PHY_GETINTVAR(pi, rstr_itt5ga1);

					/* 5G midband ofdm5gpo power offsets */
					pi->ppr->u.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5gpo);

					/* 5G midband mcs5gpo power offsets */
					pi->ppr->u.sr8.mcs[band_num] [0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo0);
					pi->ppr->u.sr8.mcs[band_num] [1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo1);
					pi->ppr->u.sr8.mcs[band_num] [2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo2);
					pi->ppr->u.sr8.mcs[band_num] [3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo3);
					pi->ppr->u.sr8.mcs[band_num] [4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo4);
					pi->ppr->u.sr8.mcs[band_num] [5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo5);
					pi->ppr->u.sr8.mcs[band_num] [6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo6);
					pi->ppr->u.sr8.mcs[band_num] [7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5gpo7);
					break;
				case WL_CHAN_FREQ_RANGE_5G_BAND2:
					/* 5G highband */
					pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num] =
					(uint8)PHY_GETINTVAR(pi, rstr_txpid5gha0);
					pwrdet->max_pwr[PHY_CORE_0][band_num]  =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);

					if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {

						pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
						= (uint8)PHY_GETINTVAR(pi, rstr_txpid5gha1);

						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
					}
					if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
					}

					pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
					idle_targ_5g[band_num] = 0;
					pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
					idle_targ_5g[band_num] = 0;

					/* 5G highband ofdm5ghpo power offsets */
					pi->ppr->u.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5ghpo);

					/* 5G highband mcs5ghpo power offsets */
					pi->ppr->u.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo0);
					pi->ppr->u.sr8.mcs[band_num][1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo1);
					pi->ppr->u.sr8.mcs[band_num][2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo2);
					pi->ppr->u.sr8.mcs[band_num][3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo3);
					pi->ppr->u.sr8.mcs[band_num][4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo4);
					pi->ppr->u.sr8.mcs[band_num][5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo5);
					pi->ppr->u.sr8.mcs[band_num][6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo6);
					pi->ppr->u.sr8.mcs[band_num][7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo7);
					break;

				case WL_CHAN_FREQ_RANGE_5G_BAND3:
					/* 5G highband */

					pi_nphy->nphy_txpid5g[PHY_CORE_0][band_num] =
					(uint8)PHY_GETINTVAR(pi, rstr_txpid5gha0);

					pwrdet->max_pwr[PHY_CORE_0][band_num]  =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
					pwrdet->pwrdet_a1[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
					pwrdet->pwrdet_b0[PHY_CORE_0][band_num]	=
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
					pwrdet->pwrdet_b1[PHY_CORE_0][band_num] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);

					if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {

						pi_nphy->nphy_txpid5g[PHY_CORE_1][band_num]
						= (uint8)PHY_GETINTVAR(pi, rstr_txpid5gha1);

						pwrdet->max_pwr[PHY_CORE_1][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
						pwrdet->pwrdet_a1[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
						pwrdet->pwrdet_b0[PHY_CORE_1][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
						pwrdet->pwrdet_b1[PHY_CORE_1][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
					}
					if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
						pwrdet->max_pwr[PHY_CORE_2][band_num]  =
							(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
						pwrdet->pwrdet_a1[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
						pwrdet->pwrdet_b0[PHY_CORE_2][band_num]	=
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
						pwrdet->pwrdet_b1[PHY_CORE_2][band_num] =
							(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
					}

					pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].
					idle_targ_5g[band_num] = 0;
					pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].
					idle_targ_5g[band_num] = 0;

					/* 5G highband ofdm5ghpo power offsets */
					pi->ppr->u.sr8.ofdm[band_num] =
						(uint32)PHY_GETINTVAR(pi, rstr_ofdm5ghpo);

					/* 5G highband mcs5ghpo power offsets */
					pi->ppr->u.sr8.mcs[band_num][0] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo0);
					pi->ppr->u.sr8.mcs[band_num][1] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo1);
					pi->ppr->u.sr8.mcs[band_num][2] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo2);
					pi->ppr->u.sr8.mcs[band_num][3] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo3);
					pi->ppr->u.sr8.mcs[band_num][4] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo4);
					pi->ppr->u.sr8.mcs[band_num][5] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo5);
					pi->ppr->u.sr8.mcs[band_num][6] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo6);
					pi->ppr->u.sr8.mcs[band_num][7] =
						(uint16)PHY_GETINTVAR(pi, rstr_mcs5ghpo7);
					break;
#endif /* BAND5G */
				}
			}

		/* Finished reading from SROM, calculate and apply powers */
}

#if defined(BCMDBG)
static int
wlc_phydump_papd(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;
	uint32 val, i, j;
	int32 eps_real, eps_imag;

	eps_real = eps_imag = 0;

	if (!pi->sh->up)
		return BCME_NOTUP;

	bcm_bprintf(b, "papd eps table:\n [core 0]\t\t[core 1] \n");
	for (j = 0; j < 64; j++) {
		for (i = 0; i < 2; i++) {
			wlc_phy_table_read_nphy(pi, ((i == 0) ? NPHY_TBL_ID_EPSILONTBL0 :
				NPHY_TBL_ID_EPSILONTBL1), 1, j, 32, &val);
			phy_papdcal_decode_epsilon(val, &eps_real, &eps_imag);
			bcm_bprintf(b, "{%d\t%d}\t\t", eps_real, eps_imag);
		}
		bcm_bprintf(b, "\n");
	}
	bcm_bprintf(b, "\n\n");

	return BCME_OK;
}

static int
wlc_phydump_lnagain(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;
	int core;
	uint16 lnagains[2][4];
	uint16 mingain[2];
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Now, read the gain table */
	for (core = 0; core < 2; core++) {
		wlc_phy_table_read_nphy(pi, core, 4, 8, 16, &lnagains[core][0]);
	}

	mingain[0] =
		(phy_utils_read_phyreg(pi, NPHY_Core1MinMaxGain) &
		NPHY_CoreMinMaxGain_minGainValue_MASK) >>
		NPHY_CoreMinMaxGain_minGainValue_SHIFT;
	mingain[1] =
		(phy_utils_read_phyreg(pi, NPHY_Core2MinMaxGain) &
		NPHY_CoreMinMaxGain_minGainValue_MASK) >>
		NPHY_CoreMinMaxGain_minGainValue_SHIFT;

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	bcm_bprintf(b, "Core 0: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
		lnagains[0][0], lnagains[0][1], lnagains[0][2], lnagains[0][3]);
	bcm_bprintf(b, "Core 1: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n\n",
		lnagains[1][0], lnagains[1][1], lnagains[1][2], lnagains[1][3]);
	bcm_bprintf(b, "Min Gain: Core 0=0x%02x,   Core 1=0x%02x\n\n",
		mingain[0], mingain[1]);

	return BCME_OK;
}

static int
wlc_phydump_initgain(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;
	uint8 ctr;
	uint16 regval[2], tblregval[4];
	uint16 lna_gain[2], hpvga1_gain[2], hpvga2_gain[2];
	uint16 tbl_lna_gain[4], tbl_hpvga1_gain[4], tbl_hpvga2_gain[4];
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	regval[0] = phy_utils_read_phyreg(pi, NPHY_Core1InitGainCode);
	regval[1] = phy_utils_read_phyreg(pi, NPHY_Core2InitGainCode);

	wlc_phy_table_read_nphy(pi, 7, PHYCORENUM(pi->pubpi->phy_corenum), 0x106, 16, tblregval);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	lna_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initLnaIndex_MASK) >>
		NPHY_CoreInitGainCode_initLnaIndex_SHIFT;
	hpvga1_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initHpvga1Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga1Index_SHIFT;
	hpvga2_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initHpvga2Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga2Index_SHIFT;

	lna_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initLnaIndex_MASK) >>
		NPHY_CoreInitGainCode_initLnaIndex_SHIFT;
	hpvga1_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initHpvga1Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga1Index_SHIFT;
	hpvga2_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initHpvga2Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga2Index_SHIFT;

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_lna_gain[ctr] = (tblregval[ctr] >> 2) & 0x3;
	}

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_hpvga1_gain[ctr] = (tblregval[ctr] >> 4) & 0xf;
	}

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_hpvga2_gain[ctr] = (tblregval[ctr] >> 8) & 0x1f;
	}

	bcm_bprintf(b, "Core 0 INIT gain: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
		hpvga2_gain[0], hpvga1_gain[0], lna_gain[0]);
	bcm_bprintf(b, "Core 1 INIT gain: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
		hpvga2_gain[1], hpvga1_gain[1], lna_gain[1]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "INIT gain table:\n");
	bcm_bprintf(b, "----------------\n");
	for (ctr = 0; ctr < 4; ctr++) {
		bcm_bprintf(b, "Core %d: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
			ctr, tbl_hpvga2_gain[ctr], tbl_hpvga1_gain[ctr], tbl_lna_gain[ctr]);
	}

	return BCME_OK;
}

static int
wlc_phydump_hpf1tbl(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;
	uint8 ctr, core;
	uint16 gain[2][NPHY_MAX_HPVGA1_INDEX+1];
	uint16 gainbits[2][NPHY_MAX_HPVGA1_INDEX+1];
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Read from the HPVGA1 gaintable */
	wlc_phy_table_read_nphy(pi, 0, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gain[0][0]);
	wlc_phy_table_read_nphy(pi, 1, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gain[1][0]);
	wlc_phy_table_read_nphy(pi, 2, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gainbits[0][0]);
	wlc_phy_table_read_nphy(pi, 3, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gainbits[1][0]);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	for (core = 0; core < 2; core++) {
		bcm_bprintf(b, "Core %d gain: ", core);
		for (ctr = 0; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
			bcm_bprintf(b, "%2d ", gain[core][ctr]);
		}
		bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, "\n");
	for (core = 0; core < 2; core++) {
		bcm_bprintf(b, "Core %d gainbits: ", core);
		for (ctr = 0; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
			bcm_bprintf(b, "%2d ", gainbits[core][ctr]);
		}
		bcm_bprintf(b, "\n");
	}

	return BCME_OK;
}
#endif // endif

#endif /* NCONF != 0 */
