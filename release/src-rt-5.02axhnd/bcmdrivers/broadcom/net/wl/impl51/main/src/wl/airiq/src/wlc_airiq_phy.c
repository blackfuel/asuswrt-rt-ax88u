/*
 * @file
 * @brief
 *
 *  Air-IQ PHY
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc_channel.h>
//#include <wlc_tx.h>
#include <wlc_scandb.h>
#include <wlc.h>
#include <phy_ac.h>
#include <phy_utils_reg.h>
#include <wlc_phyreg_ac.h>
#include <wlc_bmac.h>
#include <wl_export.h>
#include <wlc_scan.h>
#include <wlc_types.h>
#include <wlc_airiq.h>
#include <wlc_modesw.h>
#include <wlc_dfs.h>
#include <wlc_radioreg_20693.h>

#include <phy_dbg.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <phy_btcx.h>
#include <phy_calmgr.h>
#include <phy_type_txiqlocal.h>
#include <phy_ac.h>
#include <phy_ac_txiqlocal.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <wlc_radioreg_20694.h>
#include <wlc_radioreg_20695.h>
#include <wlc_radioreg_20696.h>
#include <wlc_radioreg_20697.h>
#include <wlc_radioreg_20698.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <wlc_phy_radio.h>
#include <phy_ac_info.h>
#include <phy_ac_radio.h>
#include <phy_rxgcrs_api.h>
#include <phy_ac_tof.h>
#include <phy_rstr.h>
#include <phy_utils_var.h>
#include <bcmdevs.h>
#include <phy_stf.h>

#define AC_PHY_GAIN_TABLE_OFFSET_LNA1 8
#define AC_PHY_GAIN_TABLE_OFFSET_LNA2 16
#define AC_PHY_GAIN_TABLE_OFFSET_MIXERTIA 32
#define ACPHY_DISABLE_STALL(pi) MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, 1)

#define ACPHY_ENABLE_STALL(pi, stall_val) MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, stall_val)

#define airiq_gaincode(lna1, lna2, mixtia, biq1, biq2, lnarout, elna) \
	(((lna1)  & 0x7)           | \
	 (((lna2)  & 0x7) <<  3) | \
	 (((mixtia) & 0xf) <<  6) | \
	 (((biq1)  & 0x7) << 10) | \
	 (((biq2)  & 0x7) << 13) | \
	 (((lnarout) & 0xff) << 16) | \
	 (((elna)  & 0x1) << 24))

#define airiq_gaincode_lna1(code) ((code)      & 0x7)
#define airiq_gaincode_lna2(code) (((code) >> 3) & 0x7)
#define airiq_gaincode_mix(code)  (((code) >> 6) & 0xf)
#define airiq_gaincode_biq1(code) (((code) >> 10) & 0x7)
#define airiq_gaincode_biq2(code) (((code) >> 13) & 0x7)
#define airiq_gaincode_lnarout(code) ((uint8)(((code) >> 16) & 0xff))
#define airiq_gaincode_elna(code) (((code) >> 24) & 0x1)

const airiq_gain_table_t airiq_gaintbl_rev65_24ghz = {
	.valid = TRUE,
	.lna1 = { 0, 5, 10, 16, 21, 28, 28, 28  },
	.lna2 = { 0, 6, 12, 18, 0, 6, 12, 18  },
	.mixtia	= { 0, 2, 5, 8, 11, 14, 17, 20, 23, 26},
	.lna_rout = { 0, 0, -1, -2, -3, -4, -5, -6, -8, -10, -12, -14, -14, -14, -14, -14},
	.lna_rout_lut = { 0x00, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 },
	.offset	= 0,
	.rxloss	= 2,
	.elna = 0,
};

const airiq_gain_table_t airiq_gaintbl_rev65_5ghz = {
	.valid		= TRUE,
	.lna1		= { 0, 0, 0, 3, 7, 12, 17, 25 },
	.lna2		= { 0, 6, 12, 17, 0, 6, 12, 17 },
	.mixtia		= { 0, 4, 7, 10, 13, 16, 19, 22, 25, 28},
	.lna_rout	= { 0, 0, 0, -1, -1, -2, -3, -4, -5, -6, -8, -9, -9, -9, -9, -9},
	.lna_rout_lut	= { 0x00, 0x33, 0x55, 0x66, 0x77, 0x88, 0x99 },
	.offset		= 0,
	.rxloss		= 3,
	.elna		= 0,
};

const airiq_gain_table_t airiq_gaintbl_rev128_24ghz = {
	.valid = TRUE,
	.lna1 = { 0, 5, 10, 14, 19, 29, 29, 29  },
	.lna2 = { 0, 6, 12, 18, 0, 6, 12, 18  },
	.mixtia	= { 0, 3, 7, 11, 14, 17, 20, 22, 25, 27},
	.lna_rout = { 0, -1 -1, -2, -2, -3, -4, -6, -7, -9, -10, -12, -12, -12, -12, -12},
	.lna_rout_lut = { 0x00, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 },
	.offset	= 0,
	.rxloss	= 2,
	.elna = 0,
};

const airiq_gain_table_t airiq_gaintbl_rev128_5ghz = {
	.valid		= TRUE,
	.lna1		= { 0, 0, 2, 4, 8, 13, 18, 28 },
	.lna2		= { 0, 5, 11, 14 },
	.mixtia		= { 0, 3, 6, 9, 12, 15, 18, 21, 24, 26},
	.lna_rout	= { 0, -1, -1, -1, -2, -3, -4, -5, -6, -8, -10, -12, -12},
	.lna_rout_lut	= { 0x00, 0x33, 0x55, 0x66, 0x77, 0x88, 0x99 },
	.offset		= 0,
	.rxloss		= 3,
	.elna		= 0,
};

/* Skyworks 85806 dual-band FEM on 43465MC */
const int8 airiq_sky58506_elna_24ghz = 15;
const int8 airiq_sky58506_elna_5ghz  = 16;
/* Skyworks 85201-11 2.4 GHz FEM on 43465MC2HL */
const int8 airiq_sky85201_elna_24ghz = 13;
const int8 airiq_sky85201_elna_5ghz  =  0;
/* Skyworks 85605-11 5 GHz FEM on 43465MC5HL */
const int8 airiq_sky85605_elna_24ghz =  0;
const int8 airiq_sky85605_elna_5ghz  = 12;
/* Skyworks 85806 dual-band FEM on 43684MCM */
const int8 airiq_sky85337_elna_24ghz = 15;
const int8 airiq_sky85755_elna_5ghz  = 16;
const int8 airiq_sky85331_elna_24ghz = 15;
const int8 airiq_sky85743_elna_5ghz  = 16;

/* Femctrl override mappings */
const uint8 airiq_43465mc2h_femctrl_lut[4][2] =
	{ { 0xd, 0x9 }, { 0xd, 0x9 }, { 0xd, 0x9 }, { 0xd, 0x9 } };
const uint8 airiq_43465mc5h_femctrl_lut[4][2] =
	{ { 0x7, 0x3 }, { 0x7, 0x3 }, { 0x7, 0x3 }, { 0x7, 0x3 } };
#ifdef BCA_HNDROUTER
//4908 board
const uint8 airiq_43465mc_femctrl_lut_2ghz[4][2] =
	{ { 0x19, 0x9 }, { 0x19, 0x9 }, { 0x19, 0x9 }, { 0x19, 0x9 } };
const uint8 airiq_43465mc_femctrl_lut_5ghz[4][2] =
	{ { 0x7, 0x3 }, { 0x7, 0x3 }, { 0x7, 0x3 }, { 0x7, 0x3 } };
#else
const uint8 airiq_43465mc_femctrl_lut_2ghz[4][2] =
	{ { 0x3, 0x1 }, { 0x3, 0x1 }, { 0x5, 0x1 }, { 0x3, 0x1 } };
const uint8 airiq_43465mc_femctrl_lut_5ghz[4][2] =
	{ { 0x3, 0x1 }, { 0x3, 0x1 }, { 0x15, 0x11 }, { 0xb, 0x9 } };
#endif // endif
const uint8 airiq_43684mc_femctrl_lut_2ghz[4][2] =
	{ { 0x6, 0x2 }, { 0x6, 0x2 }, { 0x6, 0x2 }, { 0x6, 0x2 } };
const uint8 airiq_43684mc_femctrl_lut_5ghz[4][2] =
	{ { 0x6, 0x2 }, { 0x6, 0x2 }, { 0x6, 0x2 }, { 0x6, 0x2 } };

/* Scale factors {20MHz, 40MHz, 80MHz} */
const int8 lte_u_rev64_scale_3plus1[3] = {50, 52, 50};

/* FFT Scale factors {20MHz, 40MHz, 80MHz} */
const int8 airiq_rev65_scale_hwfft[3] = { 65, 68, 71 };
const int8 airiq_rev65_scale_3plus1[3] = { 50, 52, 50 };

#define lte_u_gaincode(lna1, lna2, mixtia, biq1, biq2, lnarout, elna) \
	(((lna1)  & 0x7)           | \
	 (((lna2)  & 0x7) <<  3) | \
	 (((mixtia) & 0xf) <<  6) | \
	 (((biq1)  & 0x7) << 10) | \
	 (((biq2)  & 0x7) << 13) | \
	 (((lnarout) & 0xff) << 16) | \
	 (((elna)  & 0x1) << 24))

#define lte_u_gaincode_lna1(code) ((code)      & 0x7)
#define lte_u_gaincode_lna2(code) (((code) >> 3) & 0x7)
#define lte_u_gaincode_mix(code)  (((code) >> 6) & 0xf)
#define lte_u_gaincode_biq1(code) (((code) >> 10) & 0x7)
#define lte_u_gaincode_biq2(code) (((code) >> 13) & 0x7)
#define lte_u_gaincode_lnarout(code) ((uint8)(((code) >> 16) & 0xff))
#define lte_u_gaincode_elna(code) (((code) >> 24) & 0x1)

#ifdef VASIP_HW_SUPPORT
void
wlc_airiq_phy_ack_vasip_fft(airiq_info_t *airiqh)
{
	//handshake - we have completed reading fft.
	phy_utils_write_phyreg((phy_info_t*)WLC_PI(airiqh->wlc),
		ACPHY_v2m_msg(64), (1 << 15) | (1 << 5));
}

void
wlc_airiq_phy_clear_svmp_status(airiq_info_t *airiqh)
{
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_status_addr, 1, 0);
}

void
wlc_airiq_phy_init_overrides(airiq_info_t *airiqh)
{
	/* Initialize override PHY regs to a default (known) state.
	 */
	phy_info_t *pi = (phy_info_t*)WLC_PI(airiqh->wlc);
	int core;

	if (D11REV_IS(airiqh->wlc->pub->corerev, 64) ||
	    D11REV_IS(airiqh->wlc->pub->corerev, 65)) {
		WL_INFORM(("Air-IQ: Resetting FEMCTRL overrides.\n"));
		FOREACH_CORE(pi, core) {
			/* Disable Fem override */
			WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core, 0);
		}
	}
}
#endif /* VASIP_HW_SUPPORT */

static void
wlc_airiq_build_desens_lut(airiq_info_t *airiqh, airiq_gain_table_t *gt,
		uint32 gaincode[], int16 gaindb[],
		uint8 band2or5)
{
	int16 gain, maxgain;
	uint16 lna1;
	uint16 lna2;
	uint16 tia;
	uint16 biq0;
	uint16 biq1;
	uint16 k;
	uint16 lnarout_ix;
	uint8 elna;

	if (D11REV_GE(airiqh->wlc->pub->corerev, 64)) {
		elna = (gt->elna > 0); /* If ELNA present */
		lna1 = 7;
		lna2 = 3;
		tia  = 3;
		biq0 = 3;
		biq1 = 3;
		maxgain = 75;
	} else if (D11REV_IS(airiqh->wlc->pub->corerev, 42)) {
		elna = (gt->elna > 0); /* If ELNA present */
		lna1 = 5;
		lna2 = 5;
		tia  = 3;
		biq0 = 2;
		biq1 = 1;
		maxgain = gt->elna + gt->lna1[lna1] + gt->lna2[lna2] +
			gt->mixtia[tia] + 3 * biq0 + 3 * biq1;
	} else if (D11REV_IS(airiqh->wlc->pub->corerev, 56)) {
		elna = (gt->elna > 0); /* If ELNA present */
		if (band2or5 == 2) {
			lna1 = 5;
			lna2 = 3;
			tia  = 3;
			biq0 = 0;
			biq1 = 0;
			maxgain = gt->elna + gt->lna1[lna1] + gt->lna2[lna2] +
				gt->mixtia[tia] + 3 * biq0 + 3 * biq1;
		} else {
			lna1 = 7;
			lna2 = 3;
			tia  = 6;
			biq0 = 0;
			biq1 = 0;
			maxgain = gt->elna + gt->lna1[lna1] + gt->lna2[lna2] +
				gt->mixtia[tia] + 3 * biq0 + 3 * biq1;
		}
	} else {
		elna = 1;
		lna1 = 5;
		lna2 = 5;
		tia  = 3;
		biq0 = 2;
		biq1 = 1;
		maxgain = 75;
	}

	gain = gt->elna + gt->lna1[lna1] + gt->lna2[lna2] +
		gt->mixtia[tia] + 3 * biq0 + 3 * biq1;

	WL_AIRIQ(("Desens gain elna lna1 lna2 tia biq0 biq1 rout\n"));
	WL_AIRIQ(("=============================================\n"));

	for (k = 0; k < AIRIQ_DESENS_CNT; k++) {
		lnarout_ix = 0;
		while (gain > (maxgain - 2 * k)) {
			if (gain - (maxgain - 2 * k) < AIRIQ_LNA_ROUT_LUT_CNT) {
				lnarout_ix = gain - (maxgain - 2 * k);
				gain = elna * gt->elna + gt->lna1[lna1] + gt->lna2[lna2] +
					gt->mixtia[tia] + 3 * biq0 + 3 * biq1;
				break;
			}

			if (tia > 3) {
				tia--;
			} else if (lna2 > 0 &&
					!D11REV_IS(airiqh->wlc->pub->corerev, 56)) {
				lna2--;
			} else if (lna1 > 1) {
				lna1--;
			} else if (elna > 0) {
				elna--;
			} else {
				/* maxed out */
				lnarout_ix = AIRIQ_LNA_ROUT_LUT_CNT - 1;
				break;
			}
			gain = elna * gt->elna + gt->lna1[lna1] + gt->lna2[lna2] +
				gt->mixtia[tia] + 3 * biq0 + 3 * biq1;

		}
		gaindb[k] = gain - gt->rxloss - lnarout_ix + gt->offset;
		gaincode[k] = airiq_gaincode(lna1, lna2, tia, biq0, biq1,
				gt->lna_rout_lut[lnarout_ix], elna);
		WL_AIRIQ(("%6d %4d %3d %3d %3d %3d %4d %4d %4x\n",
				gain - maxgain, gaindb[k], elna, lna1, lna2, tia,
				biq0, biq1, gt->lna_rout_lut[lnarout_ix]));
	}
}

void
wlc_airiq_phy_init(airiq_info_t *airiqh)
{
	int corerev = airiqh->wlc->pub->corerev;

	switch (corerev) {
	case 65:
		// This will set the 'valid' field to true.
		memcpy(&airiqh->gaintbl_24ghz, &airiq_gaintbl_rev65_24ghz,
		       sizeof(airiqh->gaintbl_24ghz));
		memcpy(&airiqh->gaintbl_5ghz, &airiq_gaintbl_rev65_5ghz,
		       sizeof(airiqh->gaintbl_5ghz));
		/* Now determine ELNA */
		//TODO:check boardflags/srom
		if (IS_WAVE2_2G(airiqh->wlc->deviceid)) {
			airiqh->gaintbl_24ghz.elna = airiq_sky85201_elna_24ghz;
			airiqh->gaintbl_24ghz.offset = 2;
			airiqh->gaintbl_5ghz.elna  = airiq_sky85201_elna_5ghz;
			airiqh->gaintbl_5ghz.offset = 8;
		} else if (IS_WAVE2_5G(airiqh->wlc->deviceid)) {
			airiqh->gaintbl_24ghz.elna = airiq_sky85605_elna_24ghz;
			airiqh->gaintbl_5ghz.elna  = airiq_sky85605_elna_5ghz;
		} else {
		#ifdef BCA_HNDROUTER
			//4908 board uses sky85201 and sky85605
			airiqh->gaintbl_24ghz.elna = airiq_sky85201_elna_24ghz;
			airiqh->gaintbl_24ghz.offset = 2;
			airiqh->gaintbl_5ghz.elna  = airiq_sky85605_elna_5ghz;
		#else
			airiqh->gaintbl_24ghz.elna = airiq_sky58506_elna_24ghz;
			airiqh->gaintbl_5ghz.elna  = airiq_sky58506_elna_5ghz;
			airiqh->gaintbl_5ghz.offset = 4;
		#endif
		}
		/*init vasip addresses*/
		airiqh->svmp_fft_data_addr= SVMP_FFT_DATA_ADDR;
		airiqh->svmp_status_addr= SVMP_STATUS_ADDR;
		airiqh->svmp_smpl_ctrl_addr= SVMP_SMPL_CTRL_ADDR;
		airiqh->svmp_smpl_enable_addr= SVMP_SMPL_ENABLE_ADDR;
		airiqh->svmp_smpl_interval_addr= SVMP_SMPL_INTERVAL_ADDR;
		airiqh->svmp_header_addr= SVMP_HEADER_ADDR;
		airiqh->svmp_smpl_size_addr= SVMP_SMPL_SIZE_ADDR;
		airiqh->svmp_smpl_mode_addr= SVMP_SMPL_MODE_ADDR;
		airiqh->svmp_smpl_core_addr= SVMP_SMPL_CORE_ADDR;
		airiqh->svmp_smpl_gain_addr= SVMP_SMPL_GAIN_ADDR;
		airiqh->svmp_header_ext_addr= SVMP_HEADER_EXT_ADDR;
		airiqh->svmp_smpl_timestamp_addr_l= SVMP_SMPL_TIMESTAMP_ADDR_L ;
		airiqh->svmp_smpl_timestamp_addr_h= SVMP_SMPL_TIMESTAMP_ADDR_H;
		airiqh->svmp_smpl_seq_num_addr= SVMP_SMPL_SEQ_NUM_ADDR;
		airiqh->svmp_smpl_chanspec_addr= SVMP_SMPL_CHANSPEC_ADDR;
		airiqh->svmp_smpl_chanspec_3x3_addr= SVMP_SMPL_CHANSPEC_3X3_ADDR;

		break;
	case 128:
	case 129:
		// This will set the 'valid' field to true.
		memcpy(&airiqh->gaintbl_24ghz, &airiq_gaintbl_rev128_24ghz,
		       sizeof(airiqh->gaintbl_24ghz));
		memcpy(&airiqh->gaintbl_5ghz, &airiq_gaintbl_rev128_5ghz,
		       sizeof(airiqh->gaintbl_5ghz));
		/* Now determine ELNA */
		//TODO:check boardflags/srom
		if (IS_WAVE2_2G(airiqh->wlc->deviceid)) {
			airiqh->gaintbl_24ghz.elna = airiq_sky85331_elna_24ghz;
			airiqh->gaintbl_24ghz.offset = 2;
			airiqh->gaintbl_5ghz.elna  = airiq_sky85743_elna_5ghz;
			airiqh->gaintbl_5ghz.offset = 8;
		} else if (IS_WAVE2_5G(airiqh->wlc->deviceid)) {
			airiqh->gaintbl_24ghz.elna = airiq_sky85331_elna_24ghz;
			airiqh->gaintbl_5ghz.elna  = airiq_sky85743_elna_5ghz;
		} else {
			airiqh->gaintbl_24ghz.elna = airiq_sky85337_elna_24ghz;
			airiqh->gaintbl_5ghz.elna  = airiq_sky85755_elna_5ghz;
			airiqh->gaintbl_5ghz.offset = 4;
		}
		/*init vasip addresses*/
		airiqh->svmp_fft_data_addr= SVMP_AX_FFT_DATA_ADDR;
		airiqh->svmp_status_addr= SVMP_AX_STATUS_ADDR;
		airiqh->svmp_smpl_ctrl_addr= SVMP_AX_SMPL_CTRL_ADDR;
		airiqh->svmp_smpl_enable_addr= SVMP_AX_SMPL_ENABLE_ADDR;
		airiqh->svmp_smpl_interval_addr= SVMP_AX_SMPL_INTERVAL_ADDR;
		airiqh->svmp_header_addr= SVMP_AX_HEADER_ADDR;
		airiqh->svmp_smpl_size_addr= SVMP_AX_SMPL_SIZE_ADDR;
		airiqh->svmp_smpl_mode_addr= SVMP_AX_SMPL_MODE_ADDR;
		airiqh->svmp_smpl_core_addr= SVMP_AX_SMPL_CORE_ADDR;
		airiqh->svmp_smpl_gain_addr= SVMP_AX_SMPL_GAIN_ADDR;
		airiqh->svmp_header_ext_addr= SVMP_AX_HEADER_EXT_ADDR;
		airiqh->svmp_smpl_timestamp_addr_l= SVMP_AX_SMPL_TIMESTAMP_ADDR_L ;
		airiqh->svmp_smpl_timestamp_addr_h= SVMP_AX_SMPL_TIMESTAMP_ADDR_H;
		airiqh->svmp_smpl_seq_num_addr= SVMP_AX_SMPL_SEQ_NUM_ADDR;
		airiqh->svmp_smpl_chanspec_addr= SVMP_AX_SMPL_CHANSPEC_ADDR;
		airiqh->svmp_smpl_chanspec_3x3_addr= SVMP_AX_SMPL_CHANSPEC_3X3_ADDR;

		break;
	default:
		memcpy(&airiqh->gaintbl_24ghz, &airiq_gaintbl_rev65_24ghz,
				sizeof(airiqh->gaintbl_24ghz));
		memcpy(&airiqh->gaintbl_5ghz, &airiq_gaintbl_rev65_5ghz,
				sizeof(airiqh->gaintbl_5ghz));
		WL_ERROR(("%s: unsupported core rev %d. Using uncalibrated defaults.\n",
			__FUNCTION__, airiqh->wlc->pub->corerev));
		break;
	}
	wlc_airiq_build_desens_lut(airiqh, &airiqh->gaintbl_24ghz,
			airiqh->desens_lut_24ghz,
			airiqh->gain_lut_24ghz,
			2);
	wlc_airiq_build_desens_lut(airiqh, &airiqh->gaintbl_5ghz,
			airiqh->desens_lut_5ghz,
			airiqh->gain_lut_5ghz,
			5);

}

static uint32
wlc_airiq_get_gaincode(uint32 gaincode_lut[],
		int16 gain_lut[],
		uint16 desens,
		int16 *gain)
{
	uint16 ix;

	ix = desens >> 1;
	if (desens < AIRIQ_DESENS_CNT) {
		*gain = gain_lut[ix];
		return gaincode_lut[ix];
	} else {
		*gain = gain_lut[AIRIQ_DESENS_CNT - 1];
		return gaincode_lut[AIRIQ_DESENS_CNT - 1];
	}
}

int
airiq_get_gain(airiq_info_t *airiqh, phy_info_t *pi, airiq_gain_t *g)
{
	memcpy(g, &airiqh->gain, sizeof(airiq_gain_t));
	return BCME_OK;
}

int
airiq_set_gain(airiq_info_t *airiqh, airiq_gain_t *g)
{
	memcpy(&airiqh->gain, g, sizeof(airiq_gain_t));
	return BCME_OK;
}

static void
wlc_airiq_phy_override_fem(airiq_info_t *airiqh, phy_info_t *pi, uint8 core,
	uint8 elna, uint8 is5g, uint8 restore)
{

	if (elna > 1) {
		WL_ERROR(("%s: Invalid elna setting %d\n", __FUNCTION__, elna));
		return;
	}
	if (D11REV_GE(airiqh->wlc->pub->corerev, 128)) {
		if (restore) {
			/* Restore FemOvrd */
			WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core, airiqh->femctrl_ovrd);
			return;
		}
		/* Override external LNA */
		airiqh->femctrl_ovrd = READ_PHYREGCE(pi, FemOutputOvrCtrl, core);

		if (is5g) {
			WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core,
				airiq_43684mc_femctrl_lut_5ghz[core][elna]);
		} else {
			WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core,
				airiq_43684mc_femctrl_lut_2ghz[core][elna]);
		}

	} else if (D11REV_GE(airiqh->wlc->pub->corerev, 64)) {
		if (restore) {
			/* Restore FemOvrd */
			WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core, airiqh->femctrl_ovrd);
			return;
		}
		/* Override external LNA */
		airiqh->femctrl_ovrd = READ_PHYREGCE(pi, FemOutputOvrCtrl, core);

		if (IS_WAVE2_2G(airiqh->wlc->deviceid)) {
			/* 43465 MC2HL */
			WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core,
				airiq_43465mc2h_femctrl_lut[core][elna]);
		} else if (IS_WAVE2_5G(airiqh->wlc->deviceid)) {
			/* 43465 MC5HL */
			WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core,
				airiq_43465mc5h_femctrl_lut[core][elna]);
		} else {
			/* 43465 MC Dual band */
			if (is5g) {
				WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core,
					airiq_43465mc_femctrl_lut_5ghz[core][elna]);
			} else {
				WRITE_PHYREGCE(pi, FemOutputOvrCtrl, core,
					airiq_43465mc_femctrl_lut_2ghz[core][elna]);
			}
		}
	} else {
		WL_ERROR(("%s: unsupported core rev %d\n",
			__FUNCTION__, airiqh->wlc->pub->corerev));
	}

}

static void
wlc_airiq_phy_rfctrl_override_rxgain_acphy(airiq_info_t *airiqh, phy_info_t *pi, uint8 core,
	uint8 restore, rxgain_t rxgain[], uint8 lnarout, uint8 elna)
{
	uint8 stall_val;
	uint8 is5g;
    int32 radio_chanspec_sc;

	phy_ac_chanmgr_get_val_sc_chspec(PHY_AC_CHANMGR(pi), &radio_chanspec_sc);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	if (stall_val == 0) {
		ACPHY_DISABLE_STALL(pi);
	}

	if (restore == 1) {
		/* restore the stored values */
		WRITE_PHYREGCE(pi, RfctrlOverrideGains, core, airiqh->rxgain_ovrd[core].rfctrlovrd);
		WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN1, core, airiqh->rxgain_ovrd[core].rxgain);
		WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN2, core, airiqh->rxgain_ovrd[core].rxgain2);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core, airiqh->rxgain_ovrd[core].lpfgain);

		if (phy_get_phymode(pi) == PHYMODE_3x3_1x1) {
			if (D11REV_GE(airiqh->wlc->pub->corerev, 128)) {
				if (CHSPEC_IS5G(radio_chanspec_sc)) {
					MOD_RADIO_REG_20698(pi, RX5G_CFG1_OVR, 3, ovr_rx5g_lna_gc, 0);
					MOD_RADIO_REG_20698(pi, RX5G_CFG1_OVR, 3, ovr_rx5g_lna_rout, 0);
					MOD_RADIO_REG_20698(pi, RX5G_CFG1_OVR, 3, ovr_rx5g_gm_gc, 0);

				} else {
					MOD_RADIO_REG_20698(pi, RX2G_CFG1_OVR, 3,
						ovr_lna2g_lna1_gain, 0);
					MOD_RADIO_REG_20698(pi, RX2G_CFG1_OVR, 3,
						ovr_lna2g_lna1_Rout, 0);
					MOD_RADIO_REG_20698(pi, RX2G_CFG1_OVR, 3,
						ovr_rx2g_gm_gain, 0);
				}
			} else if (D11REV_GE(airiqh->wlc->pub->corerev, 64)) {
				if (CHSPEC_IS5G(radio_chanspec_sc)) {
					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, 3, ovr_lna5g_gctl1, 0);
					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, 3, ovr_lna5g_gctl1_ln, 0);
					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR2, 3, ovr_lna5g_lna2_gain, 0);
				} else {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, 3,
						ovr_lna2g_lna1_gain, 0);
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, 3,
						ovr_lna2g_lna1_Rout, 0);
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, 3,
						ovr_lna2g_lna2_gain, 0);
				}
			}
		}

		wlc_airiq_phy_override_fem(airiqh, pi, core, 0, 0, restore);

	} else {
		/* Save the original values */
		airiqh->rxgain_ovrd[core].rfctrlovrd = READ_PHYREGCE(pi, RfctrlOverrideGains,
		    core);
		airiqh->rxgain_ovrd[core].rxgain = READ_PHYREGCE(pi, RfctrlCoreRXGAIN1, core);
		airiqh->rxgain_ovrd[core].rxgain2 = READ_PHYREGCE(pi, RfctrlCoreRXGAIN2, core);
		airiqh->rxgain_ovrd[core].lpfgain = READ_PHYREGCE(pi, RfctrlCoreLpfGain, core);

		/* Write the rxgain override registers */
		WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN1, core,
				(rxgain[core].dvga << 10) | (rxgain[core].mix << 6) |
				(rxgain[core].lna2 << 3) | rxgain[core].lna1);

		WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN2, core, lnarout);

		WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core,
			(rxgain[core].lpf1 << 3) | rxgain[core].lpf0);

		if (phy_get_phymode(pi) == PHYMODE_3x3_1x1) {
			if (D11REV_GE(airiqh->wlc->pub->corerev, 128)) {
				if (CHSPEC_IS5G(radio_chanspec_sc)) {
					MOD_RADIO_REG_20698(pi, RX5G_CFG1_OVR, 3, ovr_rx5g_lna_gc, 1);
					MOD_RADIO_REG_20698(pi, RX5G_CFG1_OVR, 3, ovr_rx5g_lna_rout, 1);
					MOD_RADIO_REG_20698(pi, RX5G_CFG1_OVR, 3, ovr_rx5g_gm_gc, 1);

					MOD_RADIO_REG_20698(pi, RX5G_REG1, 3, rx5g_lna_gc,
						rxgain[core].lna1);
					MOD_RADIO_REG_20698(pi, RX5G_REG1, 3, rx5g_lna_rout,
						lnarout & 0xf);
					MOD_RADIO_REG_20698(pi, RX5G_REG5, 3, rx5g_gm_gc,
						rxgain[core].lna2);

				} else {

					MOD_RADIO_REG_20698(pi, RX2G_CFG1_OVR, 3,
						ovr_lna2g_lna1_gain, 1);
					MOD_RADIO_REG_20698(pi, RX2G_CFG1_OVR, 3,
						ovr_lna2g_lna1_Rout, 1);
					MOD_RADIO_REG_20698(pi, RX2G_CFG1_OVR, 3,
						ovr_rx2g_gm_gain, 1);

					MOD_RADIO_REG_20698(pi, LNA2G_REG1, 3, lna2g_lna1_gain,
						rxgain[core].lna1);
					MOD_RADIO_REG_20698(pi, LNA2G_REG1, 3, lna2g_lna1_Rout,
						lnarout & 0xf);
					MOD_RADIO_REG_20698(pi, RX2G_REG3, 3, rx2g_gm_gain,
						rxgain[core].lna2);

				}

			} else if (D11REV_GE(airiqh->wlc->pub->corerev, 64)) {
				if (CHSPEC_IS5G(radio_chanspec_sc)) {
					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, 3, ovr_lna5g_gctl1, 1);
					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, 3, ovr_lna5g_gctl1_ln, 1);
					MOD_RADIO_REG_20693(pi, LNA5G_CFG1, 3, lna5g_gctl1,
						rxgain[core].lna1);

					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR2, 3, ovr_lna5g_lna2_gain, 1);
					MOD_RADIO_REG_20693(pi, LNA5G_CFG1, 3, lna5g_lna2_gain,
						rxgain[core].lna2);

					MOD_RADIO_REG_20693(pi, LNA5G_CFG2, 3, lna5g_gctl1_ln,
						lnarout & 0xf);
				} else {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, 3,
						ovr_lna2g_lna1_gain, 1);
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, 3,
						ovr_lna2g_lna1_Rout, 1);
					MOD_RADIO_REG_20693(pi, LNA2G_CFG1, 3, lna2g_lna1_gain,
						rxgain[core].lna1);
					MOD_RADIO_REG_20693(pi, LNA2G_CFG1, 3, lna2g_lna1_Rout,
						lnarout & 0xf);

					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, 3,
						ovr_lna2g_lna2_gain, 1);
					MOD_RADIO_REG_20693(pi, LNA2G_CFG2, 3, lna2g_lna2_gain,
						rxgain[core].lna2);
				}
			}
		}

		if (phy_get_phymode(pi) == PHYMODE_3x3_1x1) {
			is5g = CHSPEC_IS5G(radio_chanspec_sc);
		} else {
			is5g = CHSPEC_IS5G(pi->radio_chanspec);
		}

		wlc_airiq_phy_override_fem(airiqh, pi, core, elna, is5g, restore);

		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, rxgain, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq1_gain, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 1);
	}

	if (stall_val == 0) {
		ACPHY_ENABLE_STALL(pi, stall_val);
	}
}

void
wlc_airiq_phy_enable_fft_capture(airiq_info_t* airiqh,
	uint16 fft_interval,
	uint16 desens,
	chanspec_t chanspec,
	uint16 fft_count)
{
	phy_info_t *pi;
	uint16 k;
	rxgain_t rxgain[PHY_CORE_MAX];
	wlc_info_t *wlc = airiqh->wlc;
	uint32 gaincode;
	int16 gaindb;
	uint16 lnarout;
	uint8 elna;
	uint16 bw;
	const int8 *bw_scale;
    int32 radio_chanspec_sc;

	pi = (phy_info_t*)WLC_PI(wlc);
	phy_ac_chanmgr_get_val_sc_chspec(PHY_AC_CHANMGR(pi), &radio_chanspec_sc);
	if (airiqh->mangain != 0) {
		/* override */
		gaincode = airiqh->mangain;
		gaindb = 80; /* nominal */
	} else if (CHSPEC_IS5G(chanspec)) {
		gaincode = wlc_airiq_get_gaincode(airiqh->desens_lut_5ghz,
				airiqh->gain_lut_5ghz,
				desens, &gaindb);
	} else {
		gaincode = wlc_airiq_get_gaincode(airiqh->desens_lut_24ghz,
				airiqh->gain_lut_24ghz,
				desens, &gaindb);
	}

	// Adjust calibration settings
	if (airiqh->phy_mode == PHYMODE_3x3_1x1) {
		bw_scale = airiq_rev65_scale_3plus1;
	} else if (D11REV_GE(airiqh->wlc->pub->corerev, 65)) {
		bw_scale = airiq_rev65_scale_hwfft;
	} else {
		WL_ERROR(("%s: Air-IQ needs rev >= 65: %d\n",
			__FUNCTION__, airiqh->wlc->pub->corerev));
		ASSERT(0);
		return;
	}

	bw = CHSPEC_BW(chanspec);

	switch (bw) {
	case WL_CHANSPEC_BW_20:
		gaindb += bw_scale[0];
		break;
	case WL_CHANSPEC_BW_40:
		gaindb += bw_scale[1];
		break;
	case WL_CHANSPEC_BW_80:
		gaindb += bw_scale[2];
		break;
	default:
		WL_ERROR(("%s: chanspec 0x%x unknown bw 0x%x\n", __FUNCTION__, chanspec, bw));
		break;
	}

	// encode the requested desens and calculated gain.
	gaindb = AIRIQ_GAINCODE(gaindb, desens);

	/* Disable interference mitigation mode during FFT capture
	 * since it can desense the radio and corrupt spectral data
	 */
	for (k = 0; k < PHY_CORE_MAX; k++) {
		rxgain[k].dvga = 3;
		rxgain[k].mix  = airiq_gaincode_mix(gaincode);
		rxgain[k].lna2 = airiq_gaincode_lna2(gaincode);
		rxgain[k].lna1 = airiq_gaincode_lna1(gaincode);
		rxgain[k].lpf0 = airiq_gaincode_biq1(gaincode);
		rxgain[k].lpf1 = airiq_gaincode_biq2(gaincode);
	}
	lnarout = airiq_gaincode_lnarout(gaincode);
	elna    = airiq_gaincode_elna(gaincode);

	wlc_airiq_phy_rfctrl_override_rxgain_acphy(airiqh, pi, (uint8)airiqh->core, 0, rxgain,
		lnarout, elna);

	/* streamline vasip fft collect */

	if (airiqh->phy_mode == PHYMODE_3x3_1x1) {
		wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_chanspec_addr, 1,
			radio_chanspec_sc);
	} else {
		wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_chanspec_addr, 1,
			pi->radio_chanspec);
	}
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_interval_addr, 1, fft_interval);
	if (D11REV_GE(airiqh->wlc->pub->corerev, 128)) {
		if (airiqh->phy_mode == PHYMODE_3x3_1x1) {
			// Core value passed to VASIP fw should be 4 for +1 core
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_core_addr, 1, AIRIQ_SCAN_P1C);
		} else {
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_core_addr, 1, airiqh->core);
		}
	} else {
		wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_core_addr, 1, airiqh->core);
	}
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_seq_num_addr, 1, 0);

	MOD_PHYREG(pi, RxFeCtrl1, swap_iq3, 0);

	wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_gain_addr, 1, gaindb);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_chanspec_3x3_addr, 1, pi->radio_chanspec);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_ctrl_addr, 1, fft_count);
	/* Reinitialize VASIP FFT handshake */
	wlc_airiq_phy_clear_svmp_status(airiqh);
	/* Check if AIRIQ is offloaded */
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_enable_addr, 1,
		SVMP_SMPL_ENABLE_NON_OFFLOAD_FLAG);

	airiqh->iq_capture_enable = TRUE;
}

void
wlc_airiq_phy_disable_fft_capture(airiq_info_t *airiqh)
{
	phy_info_t *pi;
	wlc_info_t *wlc = airiqh->wlc;
	uint16 svmp_enable;

	wlc_svmp_mem_read_axi(airiqh->wlc->hw, &svmp_enable, airiqh->svmp_smpl_enable_addr, 1);

	if (svmp_enable) {
		pi = (phy_info_t*)WLC_PI(wlc);
		wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_ctrl_addr, 1, 0);
		wlc_svmp_mem_set_axi(airiqh->wlc->hw, airiqh->svmp_smpl_enable_addr, 1, 0);
		//wlc_write_shm(wlc, M_AIRIQ_ENABLE, 0);
		airiqh->iq_capture_enable = FALSE;

		/* restore old settings */
		wlc_airiq_phy_rfctrl_override_rxgain_acphy(airiqh, pi, (uint8)airiqh->core,
			1, NULL, 0, 0);
	}
}

void
wlc_airiq_phy_dump_gain(airiq_info_t *airiqh, struct bcmstrbuf *b)
{
	int16 k, maxgain, gain;
	uint32 gaincode;
	uint8 lna1, lna2, mix, biq1, biq2, elna, lnarout;

	if (airiqh->gaintbl_24ghz.valid) {
		bcm_bprintf(b, "Gain LUT 2.4 GHz\n");
		bcm_bprintf(b, "Desens gain elna lna1 lna2 tia biq0 biq1 code     rout\n");
		bcm_bprintf(b, "======================================================\n");
		maxgain = airiqh->gain_lut_24ghz[0];

		for (k = 0; k < AIRIQ_DESENS_CNT; k++) {
			gain = airiqh->gain_lut_24ghz[k];
			gaincode = airiqh->desens_lut_24ghz[k];
			mix  = airiq_gaincode_mix(gaincode);
			lna2 = airiq_gaincode_lna2(gaincode);
			lna1 = airiq_gaincode_lna1(gaincode);
			biq1 = airiq_gaincode_biq1(gaincode);
			biq2 = airiq_gaincode_biq2(gaincode);
			elna = airiq_gaincode_elna(gaincode);
			lnarout = airiq_gaincode_lnarout(gaincode);
			bcm_bprintf(b, "%6d %4d %4d %4d %4d %3d %4d %4d %08x %04x\n",
					gain - maxgain, gain, elna, lna1, lna2, mix, biq1, biq2,
					gaincode, lnarout);
		}

		bcm_bprintf(b, "Gain tables:\n");
		bcm_bprintf(b, "ELNA=%3d rxloss=%3d offset=%3d\n",
				airiqh->gaintbl_24ghz.elna, airiqh->gaintbl_24ghz.rxloss,
				airiqh->gaintbl_24ghz.offset);
		bcm_bprintf(b, "code LNA1 LNA2 (dB)\n");
		for (k = 0; k < AIRIQ_ACPHY_LNA_COUNT; k++) {
			bcm_bprintf(b, "%4d %4d %4d\n", k, airiqh->gaintbl_24ghz.lna1[k],
					airiqh->gaintbl_24ghz.lna2[k]);
		}
		bcm_bprintf(b, "code mixtia (dB)\n");
		for (k = 0; k < AIRIQ_ACPHY_MIXTIA_COUNT; k++) {
			bcm_bprintf(b, "%4d %4d\n", k, airiqh->gaintbl_24ghz.mixtia[k]);
		}
	}
	if (airiqh->gaintbl_5ghz.valid) {
		bcm_bprintf(b, "Gain LUT 5 GHz elna=%d\n", airiqh->gaintbl_5ghz.elna);
		bcm_bprintf(b, "Desens gain elna lna1 lna2 tia biq0 biq1 code     rout\n");
		bcm_bprintf(b, "======================================================\n");
		maxgain = airiqh->gain_lut_5ghz[0];

		for (k = 0; k < AIRIQ_DESENS_CNT; k++) {
			gain = airiqh->gain_lut_5ghz[k];
			gaincode = airiqh->desens_lut_5ghz[k];
			mix  = airiq_gaincode_mix(gaincode);
			lna2 = airiq_gaincode_lna2(gaincode);
			lna1 = airiq_gaincode_lna1(gaincode);
			biq1 = airiq_gaincode_biq1(gaincode);
			biq2 = airiq_gaincode_biq2(gaincode);
			elna = airiq_gaincode_elna(gaincode);
			lnarout = airiq_gaincode_lnarout(gaincode);
			bcm_bprintf(b, "%6d %4d %4d %4d %4d %3d %4d %4d %08x %04x\n",
					gain - maxgain, gain, elna, lna1, lna2, mix, biq1, biq2,
					gaincode, lnarout);
		}
		bcm_bprintf(b, "Gain tables:\n");
		bcm_bprintf(b, "ELNA=%3d rxloss=%3d offset=%3d\n",
				airiqh->gaintbl_5ghz.elna, airiqh->gaintbl_5ghz.rxloss,
				airiqh->gaintbl_5ghz.offset);
		bcm_bprintf(b, "code LNA1 LNA2 (dB)\n");
		for (k = 0; k < AIRIQ_ACPHY_LNA_COUNT; k++) {
			bcm_bprintf(b, "%4d %4d %4d\n", k, airiqh->gaintbl_5ghz.lna1[k],
					airiqh->gaintbl_5ghz.lna2[k]);
		}
		bcm_bprintf(b, "code mixtia (dB)\n");
		for (k = 0; k < AIRIQ_ACPHY_MIXTIA_COUNT; k++) {
			bcm_bprintf(b, "%4d %4d\n", k, airiqh->gaintbl_5ghz.mixtia[k]);
		}
	}
}

static void
configure_lte_u_detector(airiq_info_t *airiqh)
{

	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_NSHIFT_ADDR, 1,
		airiqh->detector_config.nshift);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_EDLOW_ADDR, 1,
		airiqh->detector_config.edlow);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_EDHIGH_ADDR, 1,
		airiqh->detector_config.edhigh);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_EDSHIFT_ADDR, 1,
		airiqh->detector_config.edshift);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_DTLOW_ADDR, 1,
		airiqh->detector_config.dtlow);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_DTHIGH_ADDR, 1,
		airiqh->detector_config.dthigh);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_DTABSLOW_ADDR, 1,
		airiqh->detector_config.dtabslow);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_DTABSHIGH_ADDR, 1,
		airiqh->detector_config.dtabshigh);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_NSHIFTABSQLOW_ADDR, 1,
		airiqh->detector_config.nshiftabsqlow);
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_CONFIG_NSHIFTABSQHIGH_ADDR, 1,
		airiqh->detector_config.nshiftabsqhigh);
}

void
wlc_lte_u_clear_svmp_status(airiq_info_t *airiqh)
{
	wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_SYNC_ADDR, 1, 0);
}

void
wlc_lte_u_phy_enable_iq_capture_shm(airiq_info_t* airiqh,
		uint16 sample_interval,
		uint16 desens,
		chanspec_t chanspec,
		uint16 capture_count)
{
	phy_info_t *pi;
	uint16 k;
	rxgain_t rxgain[PHY_CORE_MAX];
	wlc_info_t *wlc = airiqh->wlc;
	uint32 gaincode = 0;
	int16 gaindb;
	uint16 lnarout;
	uint8 elna;
	uint16 bw;
	const int8 *bw_scale;
	uint16 channel;
	uint16 NshiftED = 0;
	uint16 NshiftInputLow = 0;
	uint16 NshiftInputHigh = 0;
    int32 radio_chanspec_sc;

	pi = (phy_info_t*)WLC_PI(wlc);
	bw = CHSPEC_BW(chanspec);
	channel = CHSPEC_CHANNEL(chanspec);
	phy_ac_chanmgr_get_val_sc_chspec(PHY_AC_CHANMGR(pi), &radio_chanspec_sc);
	if (airiqh->mangain != 0) {
		/* override */
		gaincode = airiqh->mangain;
		gaindb = 80; /* nominal */
	} else if (CHSPEC_IS5G(chanspec)) {
		switch (bw) {
		case WL_CHANSPEC_BW_20:
			switch (channel) {
			case 36:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x12;
				airiqh->detector_config.dtabshigh = 0x12;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 40:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x1;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x18;
				airiqh->detector_config.dtabshigh = 0x18;
				gaincode = lte_u_gaincode(6, 3, 7, 3, 3, 0, 1);
				break;
			case 44:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x2;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x10;
				airiqh->detector_config.dtabshigh = 0x10;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 48:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x2;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x12;
				airiqh->detector_config.dtabshigh = 0x12;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 149:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x20;
				airiqh->detector_config.dtabshigh = 0x20;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 153:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x20;
				airiqh->detector_config.dtabshigh = 0x20;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 157:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x20;
				airiqh->detector_config.dtabshigh = 0x20;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 161:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x20;
				airiqh->detector_config.dtabshigh = 0x20;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 165:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x20;
				airiqh->detector_config.dtabshigh = 0x20;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			default:
				WL_ERROR(("%s: Incorrect scan channel\n", __FUNCTION__));
				break;
			}
			break;
		case WL_CHANSPEC_BW_40:
			switch (channel) {
			case 36:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 40:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x24;
				airiqh->detector_config.dtabshigh = 0x24;
				gaincode = lte_u_gaincode(6, 3, 7, 3, 3, 0, 1);
				break;
			case 44:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 48:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 149:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0x20;
				airiqh->detector_config.dtabshigh = 0x20;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 153:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0x18;
				airiqh->detector_config.dtabshigh = 0x18;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 157:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0x18;
				airiqh->detector_config.dtabshigh = 0x18;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 161:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0x18;
				airiqh->detector_config.dtabshigh = 0x18;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 165:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0x18;
				airiqh->detector_config.dtabshigh = 0x18;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			default:
				WL_ERROR(("%s: Incorrect scan channel\n", __FUNCTION__));
				break;
			}
			break;
		case WL_CHANSPEC_BW_80:
			switch (channel) {
			case 36:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 40:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xc;
				airiqh->detector_config.dthigh = 0xc;
				airiqh->detector_config.dtabslow = 0x12;
				airiqh->detector_config.dtabshigh = 0x12;
				gaincode = lte_u_gaincode(6, 3, 7, 3, 3, 0, 1);
				break;
			case 44:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 48:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xd;
				airiqh->detector_config.dthigh = 0xd;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 149:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xb;
				airiqh->detector_config.dthigh = 0xb;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 153:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xb;
				airiqh->detector_config.dthigh = 0xb;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 157:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xb;
				airiqh->detector_config.dthigh = 0xb;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 161:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xb;
				airiqh->detector_config.dthigh = 0xb;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			case 165:
				NshiftED = 0x6;
				airiqh->detector_config.nshiftabsqlow = 0x2;
				airiqh->detector_config.nshiftabsqhigh = 0x0;
				NshiftInputLow = 0x0;
				NshiftInputHigh = 0x0;
				airiqh->detector_config.edlow = 100;
				airiqh->detector_config.edhigh = 1000;
				airiqh->detector_config.dtlow = 0xb;
				airiqh->detector_config.dthigh = 0xb;
				airiqh->detector_config.dtabslow = 0xd;
				airiqh->detector_config.dtabshigh = 0xd;
				gaincode = lte_u_gaincode(7, 3, 6, 3, 3, 0, 1);
				break;
			default:
				WL_ERROR(("%s: Incorrect scan channel\n", __FUNCTION__));
				break;
			}
			break;
		default:
			WL_ERROR(("%s: chanspec 0x%x unknown bw 0x%x\n", __FUNCTION__, chanspec,
				bw));
			break;
		}
		airiqh->detector_config.nshift =
			NshiftED << 12 | NshiftInputLow << 4 | NshiftInputHigh;
		configure_lte_u_detector(airiqh);
		WL_AIRIQ(("[%d]: NshiftED=0x%x NshiftABSQLOW=0x%x NshiftABSQHIGH=0x%x "
			"NshiftInputLow=0x%x NshiftInputHigh=0x%x\n",
			channel, NshiftED, airiqh->detector_config.nshiftabsqlow,
			airiqh->detector_config.nshiftabsqhigh, NshiftInputLow,
			NshiftInputHigh));
		WL_AIRIQ(("[%d]: EDLOW=%d EDHIGH=%d DTLOW=0x%x DTHIGH=0x%x DTABSLOW=0x%x "
			"DTABSHIGH=0x%x\n",
			channel, airiqh->detector_config.edlow, airiqh->detector_config.edhigh,
			airiqh->detector_config.dtlow, airiqh->detector_config.dthigh,
			airiqh->detector_config.dtabslow, airiqh->detector_config.dtabshigh));
	} else {
		WL_ERROR(("%s: Invalid chanspec\n", __FUNCTION__));
		return;
	}

	// Adjust calibration settings
	if (airiqh->phy_mode == PHYMODE_3x3_1x1) {
		bw_scale = lte_u_rev64_scale_3plus1;
	} else {
		// invalid phy mode
		return;
	}

	switch (bw) {
	case WL_CHANSPEC_BW_20:
		gaindb += bw_scale[0];
		break;
	case WL_CHANSPEC_BW_40:
		gaindb += bw_scale[1];
		break;
	case WL_CHANSPEC_BW_80:
		gaindb += bw_scale[2];
		break;
	default:
		WL_ERROR(("%s: chanspec 0x%x unknown bw 0x%x\n", __FUNCTION__, chanspec, bw));
		break;
	}

	// encode the requested desens and calculated gain.
	gaindb = LTE_U_GAINCODE(gaindb, desens);

	/* Disable interference mitigation mode during iq capture
	 * since it can desense the radio and corrupt spectral data
	 */
	for (k = 0; k < PHY_CORE_MAX; k++) {
		rxgain[k].dvga = 3;
		rxgain[k].mix  = lte_u_gaincode_mix(gaincode);
		rxgain[k].lna2 = lte_u_gaincode_lna2(gaincode);
		rxgain[k].lna1 = lte_u_gaincode_lna1(gaincode);
		rxgain[k].lpf0 = lte_u_gaincode_biq1(gaincode);
		rxgain[k].lpf1 = lte_u_gaincode_biq2(gaincode);
	}
	lnarout = lte_u_gaincode_lnarout(gaincode);
	elna    = lte_u_gaincode_elna(gaincode);

	wlc_airiq_phy_rfctrl_override_rxgain_acphy(airiqh, pi, (uint8)airiqh->core, 0, rxgain,
		lnarout, elna);

	if (airiqh->phy_mode == PHYMODE_3x3_1x1) {
		if (D11REV_IS(airiqh->wlc->pub->corerev, 65)) {
			// 43465 C0
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_INTERVAL_ADDR, 1,
				sample_interval);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_CORE_ADDR, 1,
				airiqh->core);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_SEQ_NUM_ADDR, 1, 0);
			// Force RxFeCtrl1.swap_iq3.
			MOD_PHYREG(pi, RxFeCtrl1, swap_iq3, 0);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_GAIN_ADDR, 1, gaindb);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_CHANSPEC_ADDR, 1,
				radio_chanspec_sc);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_CHANSPEC_3X3_ADDR, 1,
				pi->radio_chanspec);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_CTRL_ADDR, 1,
				capture_count);
			/* Reinitialize VASIP IQ capture handshake */
			wlc_lte_u_clear_svmp_status(airiqh);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_ENABLE_ADDR, 1, 2);
		} else {
			WL_ERROR(("%s wl%d: 3+1 not supported on corerev %d\n",
				__FUNCTION__, WLCWLUNIT(wlc), airiqh->wlc->pub->corerev));
		}
	}
	airiqh->iq_capture_enable = TRUE;
}

void
wlc_lte_u_phy_disable_iq_capture_shm(airiq_info_t *airiqh)
{
	phy_info_t *pi;
	wlc_info_t *wlc = airiqh->wlc;
	uint16 svmp_enable;

	if ((airiqh->phy_mode == PHYMODE_3x3_1x1) && D11REV_IS(airiqh->wlc->pub->corerev, 65)) {
		wlc_svmp_mem_read_axi(airiqh->wlc->hw, &svmp_enable, SVMP_LTE_U_INFO_ENABLE_ADDR, 1);
		if (svmp_enable) {
			pi = (phy_info_t*)WLC_PI(wlc);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_CTRL_ADDR, 1, 0);
			wlc_svmp_mem_set_axi(airiqh->wlc->hw, SVMP_LTE_U_INFO_ENABLE_ADDR, 1, 0);
			airiqh->iq_capture_enable = FALSE;

			/* restore old settings */
			wlc_airiq_phy_rfctrl_override_rxgain_acphy(airiqh, pi,
				(uint8)airiqh->core, 1, NULL, 0, 0);
		}
	}
}
