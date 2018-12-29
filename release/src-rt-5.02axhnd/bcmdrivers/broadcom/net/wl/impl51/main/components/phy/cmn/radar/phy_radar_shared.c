/*
 * RadarDetect module implementation (shared by PHY implementations)
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
 * $Id: phy_radar_shared.c 767730 2018-09-25 03:30:30Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <phy_api.h>
#include <phy_radar.h>
#include "phy_radar_st.h"
#include <phy_radar_utils.h>
#include <phy_radar_shared.h>
#include <phy_radar_api.h>

#include <phy_utils_reg.h>

#ifndef ALL_NEW_PHY_MOD
#include <wlc_phy_int.h>

#include <wlc_phy_n.h>
#include <wlc_phyreg_n.h>

#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#endif /* !ALL_NEW_PHY_MOD */

#define PHY_RADAR_FIFO_SUBBAND_FORMAT(pi) \
		(ACMAJORREV_GE32((pi)->pubpi->phy_rev) && !ACMAJORREV_36((pi)->pubpi->phy_rev))

#define MIN_PULSES		6	/* the minimum number of required consecutive pulses */
#define INTERVAL_TOLERANCE	16	/* in 1/20 us */
#define PULSE_WIDTH_TOLERANCE	30	/* in 1/20 us */
#define PULSE_WIDTH_DIFFERENCE	12	/* in 1/20 us */
#define PULSE_WIDTH_DIFFERENCE_ETSI4     21 /* in 1/20 us */
#define PULSE_WIDTH_DIFFERENCE_STG_LESI  17 /* in 1/20 us */

typedef struct {
	uint8 radar_type;	/* one of RADAR_TYPE_XXX */
	uint8 num_pulses;	/* min number of pulses required */
	uint16 min_pw;		/* minimum pulse-width (usec * 20) */
	uint16 max_pw;		/* maximum pulse-width (usec * 20) */
	uint16 min_pri;		/* minimum pulse repetition interval (usec) */
	uint16 max_pri;		/* maximum pulse repetition interval (usec) */
} radar_spec_t;

/*
 * Define the specifications used to detect the different Radar types.
 * Note: These are the simple ones.  The long pulse and staggered Radars will be handled seperately.
 * Note: The number of pulses are currently not used, because due to noise we need to reduce the
 *       actual number of pulses to check for (i.e. MIN_PULSES).
 * Note: The order of these are important.  We could execute a sorting routine at startup, but
 *       that will just add to the code size, so instead we order them properly here.
 *       The order should go from smallest max_pw to largest and from smallest range to largest.
 *       This is so that we detect the correct Radar where there's overlap between them.
 */
static const radar_spec_t radar_spec[] = {
	/* Japan 2.1.1: PW 0.5us, PRI 1389us */
	{ RADAR_TYPE_JP2_1_1, 5, 10, 10, 1389, 1389 },	/* num_pulses reduced from 18 */

	/* FCC 6: PW 1us, PRI 333us - same as RADAR_TYPE_JP4 */
	{ RADAR_TYPE_FCC_6, 9, 20, 20, 333, 333 },

	/* Korean 4: PW 1us, PRI 333us (similar to JP 4) */
	{ RADAR_TYPE_KN4, 3, 20, 20, 333, 333 },

	/* Korean 2: PW 1us, PRI 556us */
	{ RADAR_TYPE_KN2, 10, 20, 20, 556, 556 },

	/* ETSI 0: PW 1us, PRI 1429us */
	{ RADAR_TYPE_ETSI_0, 5, 20, 20, 1428, 1429 },	/* num_pulses reduced from 18 */

	/* FCC 0: PW 1us, PRI 1428us - same RADAR_TYPE_JP1_1, RADAR_TYPE_JP2_2_1, RADAR_TYPE_KN1 */
	{ RADAR_TYPE_FCC_0, 5, 20, 20, 1428, 1429 },	/* num_pulses reduced from 18 */

	/* Japan 1.2: PW 2.5us, PRI 3846us */
	{ RADAR_TYPE_JP1_2, 5, 50, 50, 3846, 3846 },	/* num_pulses reduced from 18 */

	/* Japan 2.1.2: PW 2us, PRI 4000us */
	{ RADAR_TYPE_JP2_1_2, 5, 40, 40, 4000, 4000 },	/* num_pulses reduced from 18 */

	/* Korean 3: PW 2us, PRI 3030us */
	{ RADAR_TYPE_KN3, 70, 40, 40, 3030, 3030 },

	/* FCC 1: PW 1us, PRI 518-3066us */
	{ RADAR_TYPE_FCC_1, 5, 20, 20, 518, 3066 },	/* num_pulses reduced from 18 */

	/* FCC 2: PW 1-5us, PRI 150-230us */
	{ RADAR_TYPE_FCC_2, 23, 20, 100, 150, 230 },

	/* ETSI 1: PW 0.5-5us, PRI 1000-5000us */
	{ RADAR_TYPE_ETSI_1, 3, 10, 100, 1000, 5000 },	/* num_pulses reduced from 10 */

	/* Japan 2.2.2: same as FCC 2 */
	{ RADAR_TYPE_JP2_2_2, 23, 20, 100, 150, 230 },

	/* FCC 3: PW 6-10us, PRI 200-500us */
	{ RADAR_TYPE_FCC_3, 16, 120, 200, 200, 500 },

	/* Japan 2.2.3: PW 6-10us, PRI 250-500us (similar to FCC 3) */
	{ RADAR_TYPE_JP2_2_3, 16, 120, 200, 250, 500 },

	/* ETSI 2: PW 0.5-15us, PRI 625-5000us */
	{ RADAR_TYPE_ETSI_2, 5, 10, 300, 625, 5000 },	/* num_pulses reduced from 15 */

	/* ETSI 3: PW 0.5-15us, PRI 250-435us */
	{ RADAR_TYPE_ETSI_3, 25, 10, 300, 250, 435 },

	/* FCC 4: PW 11-20us, PRI 200-500us */
	{ RADAR_TYPE_FCC_4, 12, 220, 400, 200, 500 },

	/* Japan 2.2.4: PW 11-20us, PRI 250-500us (similar to FCC 4) */
	{ RADAR_TYPE_JP2_2_4, 12, 220, 400, 250, 500 },

	/* ETSI 4: PW 20-30us, PRI 250-500us */
	{ RADAR_TYPE_ETSI_4, 20, 400, 600, 250, 500 },

	/* UK 1: PW 1us, PRI 333us (similar to JP 4) */
	{ RADAR_TYPE_UK1, 3, 20, 20, 333, 333 },

	/* UK 2: PW 20us, PRI 222us */
	{ RADAR_TYPE_UK2, 5, 400, 400, 222, 222 },

	{ RADAR_TYPE_NONE, 0, 0, 0, 0, 0 }
};

/* current radar fifo sample info */
typedef struct {
	/* pulses (in tstamps) read from PHY FIFO */
	uint16 length_input[RDR_NANTENNAS];
	pulse_data_t pulses_input[RDR_NANTENNAS][RDR_LIST_SIZE];

	/* pulses (in tstamps) used for checking FCC 5 Radar match */
	uint16 length_bin5[RDR_NANTENNAS];
	pulse_data_t pulses_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];

	/* pulses (in intervals) to process for checking short-pulse Radar match */
	uint16 length;
	pulse_data_t pulses[RDR_LIST_SIZE];
} radar_work_t;

/* Field Description: fm_chk_pw_thrs1, fm_chk_pw_thrs2 */
static const uint16 fm_chk_pw[] = {25, 65};
/* Set individual fm threshold in below three pw regions:
 * Region 1: pw < fm_chk_pw_thrs1; Region 2: fm_chk_pw_thrs1 < pw < fm_chk_pw_thrs2;
 * Region 3: fm_chk_pw_thrs2 < pw; Field Description: fmthrs_20_rgn1, fmthrs_20_rgn2,
 * fmthrs_20_rgn3, fmthrs_40_rgn1, fmthrs_40_rgn2, fmthrs_40_rgn3, fmthrs_80_rgn1,
 * fmthrs_80_rgn2, fmthrs_80_rgn3, fmthrs_160_rgn1, fmthrs_160_rgn2, fmthrs_160_rgn3
 */
static const uint16 fm_thrs_sp[] = {0, 15, 60, 0, 170, 255, 30, 250, 255, 10, 250, 255};
static const uint16 fm_thrs_sp_lesi[] = {0, 15, 60, 0, 0, 200, 0, 0, 100, 0, 20, 200};

#define SUBBAND_NUM_BW40	2
#define SUBBAND_NUM_BW80	4
#define SUBBAND_NUM_BW160	8
#define ALLSUBBAND_BW20	1
#define ALLSUBBAND_BW40	3
#define ALLSUBBAND_BW80	15

#define FC_TOL_SB 2
#define FC_TOL_BIN5_SB 5
#define PW_TOL_HIPOWWAR_BIN5 400
#define PW_CHIRP_ADJ_TH_BIN5 800
#define CHIRP_TH1 4
#define CHIRP_TH2 6
#define CHIRP_FC_TH 60

#define FCC5_CHIRP	40	/* 20 MHz */
#define ETSI4_CHIRP	12	/* 6 MHz */
#define SUBBAND_BW	40	/* 20 MHz */

#define MIN_FM_THRESH_SHORT_PULSES	100
#define MIN_FM_THRESH_ETSI4		240

void
BCMATTACHFN(phy_radar_shared_attach)(phy_info_t *pi)
{
	/* scratch buffer to use in phy_radar_run_nphy() for fifo sample data */
	phy_cache_register_reuse_size(pi->cachei, sizeof(radar_work_t));
}

static void
wlc_phy_radar_read_table(phy_info_t *pi, phy_radar_st_t *st, radar_work_t *rt, bool sec_pll,
bool bw80_80_mode)
{
	int i;
	uint8 core;
	uint8 ANT_num = 1;
	uint16 w0, w1, w2, w3, w4, w5;
	int FMOffset = 0;
	uint8 time_shift = 0;
	uint16 RadarFifoCtrlReg[2];
	uint16 RadarFifoDataReg[2];
	radar_lp_info_t *rlpt = (sec_pll) ? st->radar_work_lp_sc : &st->radar_lp_info;
	uint8 rdr_core_mask = READ_PHYREGFLD(pi, CoreConfig, CoreMask) & READ_PHYREGFLD(pi,
		RadarDetectConfig1, core_select_overwrite);

	bzero(rt->pulses_input, sizeof(rt->pulses_input));

	if (ISACPHY(pi)) {
		if (bw80_80_mode == FALSE) {
			//4x4, 2core normal radar
			RadarFifoCtrlReg[0] = ACPHY_Antenna0_radarFifoCtrl(pi->pubpi->phy_rev);
			RadarFifoCtrlReg[1] = ACPHY_Antenna1_radarFifoCtrl(pi->pubpi->phy_rev);
			RadarFifoDataReg[0] = ACPHY_Antenna0_radarFifoData(pi->pubpi->phy_rev);
			RadarFifoDataReg[1] = ACPHY_Antenna1_radarFifoData(pi->pubpi->phy_rev);

			//3+1, 1core  scan radar
			if (sec_pll == TRUE) {
				if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
					RadarFifoCtrlReg[0] =
						ACPHY_Antenna0_radarFifoCtrl(pi->pubpi->phy_rev);
					RadarFifoCtrlReg[1] = 0;
					RadarFifoDataReg[0] =
						ACPHY_Antenna0_radarFifoData(pi->pubpi->phy_rev);
					RadarFifoDataReg[1] = 0;
				} else { /* 4365/66 */
					RadarFifoCtrlReg[0] =
						ACPHY_Antenna0_radarFifoCtrl_SC(pi->pubpi->phy_rev);
					RadarFifoCtrlReg[1] = 0;
					RadarFifoDataReg[0] =
						ACPHY_Antenna0_radarFifoData_SC(pi->pubpi->phy_rev);
					RadarFifoDataReg[1] = 0;
				}
				ANT_num = 1;
			} else if (ACMAJORREV_47(pi->pubpi->phy_rev) && ((rdr_core_mask  == 0x8) ||
				(rdr_core_mask == 0x4) || (rdr_core_mask == 0x2))) {
				RadarFifoCtrlReg[0] =
					ACPHY_Antenna1_radarFifoCtrl(pi->pubpi->phy_rev);
				RadarFifoCtrlReg[1] = 0;
				RadarFifoDataReg[0] =
					ACPHY_Antenna1_radarFifoData(pi->pubpi->phy_rev);
				RadarFifoDataReg[1] = 0;
				ANT_num = 1;
			} else {
				ANT_num = GET_RDR_NANTENNAS(pi);
			}
		} else {
			ANT_num = 1;
			//L80 in 80+80, 1core normal radar
			if (sec_pll == FALSE) {
				RadarFifoCtrlReg[0] =
					ACPHY_Antenna0_radarFifoCtrl(pi->pubpi->phy_rev);
				RadarFifoCtrlReg[1] = 0;
				RadarFifoDataReg[0] =
					ACPHY_Antenna0_radarFifoData(pi->pubpi->phy_rev);
				RadarFifoDataReg[1] = 0;
			} else {
			//U80 in 80+80, 1core normal radar
				RadarFifoCtrlReg[0] =
					ACPHY_Antenna1_radarFifoCtrl(pi->pubpi->phy_rev);
				RadarFifoCtrlReg[1] = 0;
				RadarFifoDataReg[0] =
					ACPHY_Antenna1_radarFifoData(pi->pubpi->phy_rev);
				RadarFifoDataReg[1] = 0;
			}
		}
		if (TONEDETECTION) {
			FMOffset = 256;
		}
	} else {
		RadarFifoCtrlReg[0] = NPHY_Antenna0_radarFifoCtrl;
		RadarFifoCtrlReg[1] = NPHY_Antenna1_radarFifoCtrl;
		RadarFifoDataReg[0] = NPHY_Antenna0_radarFifoData;
		RadarFifoDataReg[1] = NPHY_Antenna1_radarFifoData;
	}

	if (!PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
		if (IS20MHZ(pi)) {
			time_shift = 0;
		} else if (IS40MHZ(pi)) {
			time_shift = 1;
		} else {
			time_shift = 2;
		}
	}

	for (core = 0; core < ANT_num; core++) {
		if (ACMAJORREV_47(pi->pubpi->phy_rev) && (sec_pll == TRUE)) /* 43684 */
			rt->length_input[core] =
				phy_utils_read_phyreg_p1c(pi, RadarFifoCtrlReg[core]) & 0x3ff;
		else /* 4365/66 */
			rt->length_input[core] =
				phy_utils_read_phyreg(pi, RadarFifoCtrlReg[core]) & 0x3ff;

		if (rt->length_input[core] > MAX_FIFO_SIZE) {
			PHY_RADAR(("FIFO LENGTH in ant %d is greater than max_fifo_size of %d\n",
			           core, MAX_FIFO_SIZE));
			rt->length_input[core] = 0;
		}

#ifdef BCMDBG
		/* enable pulses received at each antenna messages if feature_mask bit-2 is set */
		if (st->rparams.radar_args.feature_mask & RADAR_FEATURE_DEBUG_PULSES_PER_ANT &&
		    rt->length_input[core] > 5) {
			PHY_RADAR(("ant %d:%d\n", core, rt->length_input[core]));
		}
#endif /* BCMDBG */
		if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
			rt->length_input[core] /= 6; /* 6 words per pulse */
		} else {
			rt->length_input[core] /= 4; /* 4 words per pulse */
		}

		/* use the last sample for bin5 */
		rt->pulses_bin5[core][0] = rlpt->pulse_tail[core];

		for (i = 0; i < rt->length_input[core]; i++) {
			ASSERT(i < RDR_LIST_SIZE);
			if (ACMAJORREV_47(pi->pubpi->phy_rev) && (sec_pll == TRUE)) {
				w0 = phy_utils_read_phyreg_p1c(pi, RadarFifoDataReg[core]);
				w1 = phy_utils_read_phyreg_p1c(pi, RadarFifoDataReg[core]);
				w2 = phy_utils_read_phyreg_p1c(pi, RadarFifoDataReg[core]);
				w3 = phy_utils_read_phyreg_p1c(pi, RadarFifoDataReg[core]);
			} else {
				w0 = phy_utils_read_phyreg(pi, RadarFifoDataReg[core]);
				w1 = phy_utils_read_phyreg(pi, RadarFifoDataReg[core]);
				w2 = phy_utils_read_phyreg(pi, RadarFifoDataReg[core]);
				w3 = phy_utils_read_phyreg(pi, RadarFifoDataReg[core]);
			}
			if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
				if (ACMAJORREV_47(pi->pubpi->phy_rev) && (sec_pll == TRUE)) {
					w4 = phy_utils_read_phyreg_p1c(pi, RadarFifoDataReg[core]);
					w5 = phy_utils_read_phyreg_p1c(pi, RadarFifoDataReg[core]);
				} else {
					w4 = phy_utils_read_phyreg(pi, RadarFifoDataReg[core]);
					w5 = phy_utils_read_phyreg(pi, RadarFifoDataReg[core]);
				}

				/* extract the timestamp, pw, chirp, notradar from the 6 words */
				rt->pulses_input[core][i].interval = (uint32)((w0 << 16) +
						((w1 & 0x0fff) << 4) + (w3 & 0xf));
				if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
					rt->pulses_input[core][i].pw = (((w3 & 0x10) << 8) +
						((w2 & 0x00ff) << 4) + ((w1 >> 12) & 0x000f)) +
						((w3 & 0x8000) << 13);
				} else {
					rt->pulses_input[core][i].pw = (((w3 & 0x10) << 8) +
						((w2 & 0x00ff) << 4) + ((w1 >> 12) & 0x000f));
				}
				rt->pulses_input[core][i].fm =
					((w3 & 0x20) << 3) + ((w2 >> 8) & 0x00ff) - FMOffset;

				rt->pulses_input[core][i].fc = (int16)(w4 & 0x01ff);
				if (rt->pulses_input[core][i].fc > 256) {
					rt->pulses_input[core][i].fc =
						rt->pulses_input[core][i].fc-512;
				}
				rt->pulses_input[core][i].chirp = ((w3 & 0xffc0) >> 6);
				rt->pulses_input[core][i].notradar = ((w5 & 0xfff8) >> 3);

			} else {
				/* extract the timestamp and pulse widths from the 4 words */
				rt->pulses_input[core][i].interval =
					(uint32)(((w0 << 16) + ((w1 & 0x0fff) << 4) + (w3 & 0xf))
					>> time_shift);
				rt->pulses_input[core][i].pw =
					(((w3 & 0x10) << 8) +
					((w2 & 0x00ff) << 4) + ((w1 >> 12) & 0x000f))
					>> time_shift;
				rt->pulses_input[core][i].fm =
					((w3 & 0x20) << 3) + ((w2 >> 8) & 0x00ff) - FMOffset;
				rt->pulses_input[core][i].fc = 0;
				rt->pulses_input[core][i].chirp = 0;
				rt->pulses_input[core][i].notradar = 0;
			}

			rt->pulses_bin5[core][i + 1] =
				rt->pulses_input[core][i];
		}

		/* save the last (tail) sample */
		rlpt->pulse_tail[core] = rt->pulses_bin5[core][i+1];
		rt->length_bin5[core] = rt->length_input[core] + 1;
	}
}

static bool
wlc_phy_radar_valid(const phy_info_t *pi, uint16 feature_mask, uint8 radar_type)
{
	bool uk_new_valid = FALSE;

	if ((CHSPEC_CHANNEL(pi->radio_chanspec) == 138) ||
		(CHSPEC_CHANNEL(pi->radio_chanspec) >= 142))
		uk_new_valid = TRUE;

	switch (radar_type) {
	case RADAR_TYPE_NONE:
	case RADAR_TYPE_UNCLASSIFIED:
		return FALSE;

	case RADAR_TYPE_ETSI_0:
	case RADAR_TYPE_ETSI_1:
	case RADAR_TYPE_ETSI_2:
	case RADAR_TYPE_ETSI_3:
	case RADAR_TYPE_ETSI_4:
		return ((feature_mask & RADAR_FEATURE_ETSI_DETECT) != 0);
	case RADAR_TYPE_UK1:
	case RADAR_TYPE_UK2:
		return (uk_new_valid && ((feature_mask & RADAR_FEATURE_UK_DETECT) != 0));

	default:
		;
	}

	return (feature_mask & RADAR_FEATURE_FCC_DETECT) ? TRUE : FALSE;
}

static const radar_spec_t *
wlc_phy_radar_detect_match(phy_info_t *pi, radar_params_t *rparams, const pulse_data_t *pulse,
	uint16 feature_mask)
{
	const radar_spec_t *pRadar;
	const pulse_data_t *next_pulse = pulse + 1;
	int intv_tol = (ACMAJORREV_47(pi->pubpi->phy_rev) && rparams->radar_args.max_deltat == 1) ?
		rparams->radar_args.quant : INTERVAL_TOLERANCE;

	/* scan the list of Radars for a possible match */
	for (pRadar = &radar_spec[0]; pRadar->radar_type != RADAR_TYPE_NONE; ++pRadar) {

		/* skip Radars not in our region */
		if (!wlc_phy_radar_valid(pi, feature_mask, pRadar->radar_type))
			continue;

		/* need consistent intervals */
		if (ABS((int32)(pulse->interval - next_pulse->interval)) <
		    intv_tol + (int32)pulse->interval / 4000) {
			uint16 max_pw = pRadar->max_pw + pRadar->max_pw/10 + PULSE_WIDTH_TOLERANCE;

			/* check against current radar specifications */

			/* Note: min_pw is currently not used due to a hardware fault causing
			 *       some valid pulses to be spikes of very small widths.  We need to
			 *       ignore the min_pw otherwise we could miss some valid Radar signals
			 */
			if (pulse->pw <= max_pw && next_pulse->pw <= max_pw &&
			    pulse->interval >=
			    (uint32)pRadar->min_pri * 20 - pRadar->min_pri/9 - intv_tol &&
			    pulse->interval <=
			    (uint32)pRadar->max_pri * 20 + pRadar->max_pri/9 + intv_tol) {
				/* match found */
				break;
			}
		}
	}

	return pRadar;	/* points to an entry in radar_spec[] */
}

/* remove outliers from a list */
static int
wlc_phy_radar_filter_list(pulse_data_t inlist[], int length, uint32 min_val, uint32 max_val)
{
	int i, j;
	j = 0;
	for (i = 0; i < length; i++) {
		if ((inlist[i].interval >= min_val) && (inlist[i].interval <= max_val)) {
			inlist[j] = inlist[i];
			j++;
		}
	}
	return j;
}

static void
wlc_phy_radar_fc_chirp_cmb(pulse_data_t *input0, pulse_data_t *input1)
{
	int16 l_fc = 0;
	int16 s_fc = 0;
	uint16 l_pw = 0;
	uint16 s_pw = 0;
	if (input0->pw > input1->pw) {
		l_fc = input0->fc;
		l_pw = input0->pw;
		s_fc = input1->fc;
		s_pw = input1->pw;
	} else {
		l_fc = input1->fc;
		l_pw = input1->pw;
		s_fc = input0->fc;
		s_pw = input0->pw;
	}
	if (l_pw > (s_pw << 4)) {
		input0->fc = ((l_fc << 4)-l_fc+s_fc) >> 4;
	} else if (l_pw > (s_pw << 3)) {
		input0->fc = ((l_fc << 3)-l_fc+s_fc) >> 3;
	} else if (l_pw > (s_pw << 2)) {
		input0->fc = ((l_fc << 2)-l_fc+s_fc) >> 2;
	} else {
		input0->fc = (l_fc+s_fc) >> 1;
	}
	/* Combine chirp */
	input0->chirp = input0->chirp + input1->chirp;
	/* Combine notradar */
	input0->notradar = (input0->notradar > input1->notradar ?
			input0->notradar : input1->notradar);
}

static uint16
wlc_phy_radar_fc_chirp_2_subband(const phy_info_t *pi, int fc,
	uint chirp, uint fcc5)
{
	uint16 subband_result = 0;
	uint16 subband_num = 1;
	uint16 subband_idx = 0;
	uint16 subband_weighting = 1;
	int fc_tol = 0;

	//subband detection
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		subband_result = ALLSUBBAND_BW20;
		return subband_result;
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		subband_num = SUBBAND_NUM_BW40;
	} else if (CHSPEC_IS80(pi->radio_chanspec) ||
		PHY_AS_80P80(pi, pi->radio_chanspec)) {
		subband_num = SUBBAND_NUM_BW80;
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		subband_num = SUBBAND_NUM_BW160;
	}

	if (fcc5 == 0) {
		fc_tol = FC_TOL_SB;
	} else {
		fc_tol = FC_TOL_BIN5_SB;
	}

	subband_weighting = subband_weighting << (subband_num - 1);
	for (subband_idx = 0; subband_idx < subband_num; subband_idx++) {
		if ((fc + (int)(chirp/2) + fc_tol) >=
			(((int)(0 - subband_num/2 + subband_idx))*SUBBAND_BW) &&
			(fc - (int)(chirp/2) - fc_tol) <=
			(((int)(1 - subband_num/2 + subband_idx))*SUBBAND_BW)) {
			subband_result = subband_result + (subband_weighting >> subband_idx);
		}
	}
	return subband_result;
}

#define MIN_STAGGERED_INTERVAL		16667	/* 833 us */
#define MAX_STAGGERED_INTERVAL		66667	/* 3333 us */
#define MAX_STAGGERED_PULSE_WIDTH	40	/* 2 us */
#define	MAX_STAGGERED_ETSI_6_INTERVAL	50000	/* 2500 us */

static void phy_radar_check_staggered(phy_info_t *pi, radar_params_t *rparams, int *nconseq,
	const pulse_data_t pulse[], uint16 j, int stg)
{
	int intv_tol = (ACMAJORREV_47(pi->pubpi->phy_rev) && rparams->radar_args.max_deltat == 1) ?
		rparams->radar_args.quant : INTERVAL_TOLERANCE;
	int lesi_rev47 = (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) ? TRUE : FALSE;

	if (ABS((int32)(pulse[j].interval - pulse[j - stg].interval)) <= intv_tol &&
	    (int)pulse[j].pw <= MAX_STAGGERED_PULSE_WIDTH + PULSE_WIDTH_TOLERANCE &&
	    (int)pulse[j].interval >= MIN_STAGGERED_INTERVAL - intv_tol &&
	    (int)pulse[j].interval <= MAX_STAGGERED_INTERVAL + intv_tol) {
		int k;
		uint16 curr_pw = pulse[j].pw;
		uint16 min_pw = curr_pw;
		uint16 max_pw = curr_pw;

		++(*nconseq);

		/* check all alternate pulses for pulse width tolerance */
		for (k = 0; k < *nconseq; k++) {
			curr_pw = pulse[j - stg * k].pw;
			if (curr_pw < min_pw) {
				min_pw = curr_pw;
			}
			if (curr_pw > max_pw) {
				max_pw = curr_pw;
			}
		}
		if (max_pw - min_pw > ((ACMAJORREV_47(pi->pubpi->phy_rev) &&
			rparams->radar_args.max_deltat == 1) ? rparams->radar_args.max_pw_tol :
			((lesi_rev47 ? PULSE_WIDTH_DIFFERENCE_STG_LESI : PULSE_WIDTH_DIFFERENCE) +
			curr_pw / 200))) {
			/* alternate pulse widths inconsistent */
			*nconseq = 0;
		}
	} else {
		/* alternate intervals inconsistent */
		*nconseq = 0;
	}
}

static void phy_radar_check_staggered2(phy_info_t *pi, radar_params_t *rparams, int *nconseq,
	const pulse_data_t pulse[], uint16 j)
{
	phy_radar_check_staggered(pi, rparams, nconseq, pulse, j, 2);
}

static void phy_radar_check_staggered3(phy_info_t *pi, radar_params_t *rparams, int *nconseq,
	const pulse_data_t pulse[], uint16 j)
{
	phy_radar_check_staggered(pi, rparams, nconseq, pulse, j, 3);
}

#define EXT_FM_CHK_DBG 0x1
#define EXT_FM_ABS_CHK_SP 0x2
#define EXT_FM_VAR_CHK_SP 0x4
#define EXT_FM_ABS_CHK_ETSI4 0x8

static bool phy_radar_ext_fm_check(phy_info_t *pi, radar_params_t *rparams, pulse_data_t pulses[],
	uint8 det_type, uint8 previous_det_type, uint16 j, int nconsecq_pulses, int min_det_pw,
	bool sec_pll)
{
	int k;
	int fm_dif;
	int fm_tol;
	int fm_min = 1030;
	int fm_max = 0;
	bool fm_chk = TRUE;
	phy_radar_info_t *ri = pi->radari;
	phy_radar_st_t *st = phy_radar_get_st(ri);
	wl_radar_args_t *radar_args = &rparams->radar_args;
	wl_radar_thr2_t *radar_thrs2 = &rparams->radar_thrs2;
	int etsi4_fm_thresh = MIN_FM_THRESH_ETSI4;
	int pw_thrs_fm_var_chk = (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG) ?
		radar_thrs2->fm_var_chk_pw : 25;
	int div_fm_tol = (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG) ?
		radar_thrs2->fm_tol_div : 5;

	/* special case for ETSI 4, as it has a chirp of +/-2.5MHz */
	if (det_type == RADAR_TYPE_ETSI_4) {
		if (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG) {
			etsi4_fm_thresh = radar_thrs2->fm_thresh_etsi4;
		} else if (ACMAJORREV_47(pi->pubpi->phy_rev) && radar_args->max_deltat == 1) {
			etsi4_fm_thresh = ((radar_args->fra_pulse_err >> 8) & 0xff) -
				((radar_args->ncontig >> 6) & 0xf);
		} else if (sec_pll == TRUE) {
			etsi4_fm_thresh = (st->chanset_elna_chk == (NC_CHAN_2GBW20 |
				NC_CHAN_2GBW40 | SC_CHAN_5GBW20 | EXT_LNA_5G_OFF)) ? 0 : 50;
		}
		/* not a valid ETSI 4 if fm doesn't vary */
		for (k = 0; k < nconsecq_pulses; k++) {
			if (ISACPHY(pi) && TONEDETECTION) {
				if (radar_thrs2->fm_chk_opt & EXT_FM_ABS_CHK_ETSI4) {
					if (pulses[j - k].fm < etsi4_fm_thresh) {
						fm_chk = FALSE;
#ifdef BCMDBG
						if ((radar_args->feature_mask &
							RADAR_FEATURE_DEBUG_SHORT_PULSE) != 0) {
							PHY_RADAR(("\nShort pulses fail ETSI-4"
							" absolute fm value check"));
						}
#endif /* BCMDBG */
						break;
					}
				} else {
					break;
				}
			} else {
				if (pulses[j - k].fm < fm_min) {
					fm_min = pulses[j - k].fm;
				}
				if (pulses[j - k].fm > fm_max) {
					fm_max = pulses[j - k].fm;
				}
				fm_dif = ABS(fm_max - fm_min);
				fm_tol = (fm_min + fm_max) / 4;
				if (fm_dif > fm_tol) {
					fm_chk = FALSE;
					break;
				}
			}
		}
	} else if ((radar_thrs2->fm_chk_opt & EXT_FM_VAR_CHK_SP) &&
		(min_det_pw >= pw_thrs_fm_var_chk) && (sec_pll == FALSE)) {
		for (k = 0; k < nconsecq_pulses; k++) {
			if (pulses[j - k].fm < fm_min) {
				fm_min = pulses[j - k].fm;
			}
			if (pulses[j - k].fm > fm_max) {
				fm_max = pulses[j - k].fm;
			}
			fm_dif = ABS(fm_max - fm_min);
			fm_tol = (fm_min + fm_max) / div_fm_tol;
			if (fm_dif > fm_tol) {
				fm_chk = FALSE;
#ifdef BCMDBG
				if ((radar_args->feature_mask &
					RADAR_FEATURE_DEBUG_SHORT_PULSE) != 0) {
					PHY_RADAR(("\nShort pulses fail fm"
					" variation check"));
				}
#endif /* BCMDBG */
				break;
			}
		}
	}
	return fm_chk;
}

#define EXT_PW_TOL_SET1 0x1
#define EXT_PW_TOL_SET2 0x2
#define EXT_PW_TOL_SET3 0x4
#define EXT_PW_TOL_SET4 0x8
#define EXT_PW_TOL_SETALL 0Xf

static bool phy_radar_ext_pw_tol(phy_info_t *pi, radar_params_t *rparams, int max_det_pw,
	int min_det_pw, uint16 blank_time, uint16 set)
{
	bool chk_set1, chk_set2, chk_set3, chk_set4;
	int max_pw_tol = (ACMAJORREV_47(pi->pubpi->phy_rev) &&
		rparams->radar_args.max_deltat == 1) ? rparams->radar_args.max_pw_tol :
		PULSE_WIDTH_DIFFERENCE;
	int lesi_rev47 = (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) ? TRUE : FALSE;
	/* ext pw tol for pw variations <= 37(1.85us)  */
	chk_set1 = ((set & EXT_PW_TOL_SET1) && (((max_det_pw >= 30) && (max_det_pw <= (lesi_rev47 ?
		60 : 50))) && ((max_det_pw - min_det_pw) <= (max_pw_tol + blank_time)))) ?
		TRUE : FALSE;
	/* ext pw tol for pw variations >= 19(0.95us) and pw variations <= 31(1.55us) */
	chk_set2 = ((set & EXT_PW_TOL_SET2) && (((max_det_pw > 50) && (max_det_pw <= 430)) &&
		(((max_det_pw - min_det_pw)  >= (blank_time - (lesi_rev47 ? max_pw_tol :
		max_pw_tol/2))) && ((max_det_pw - min_det_pw) <= (blank_time + (lesi_rev47 ?
		max_pw_tol : max_pw_tol/2)))))) ? TRUE : FALSE;
	/* ext pw tol for pw variations >= 44(2.2us) and pw variations <= 56(2.8us) */
	chk_set3 = ((set & EXT_PW_TOL_SET3) && (((max_det_pw > 80) && (max_det_pw <= 430)) &&
		(((max_det_pw - min_det_pw)  >= (2*blank_time - (max_pw_tol/2))) &&
		((max_det_pw - min_det_pw) <= (2*blank_time + (max_pw_tol/2)))))) ?
		TRUE : FALSE;
	/* ext pw tol for pw variations >= 69(3.45us) and pw variations <= 81(4.05us) */
	chk_set4 = ((set & EXT_PW_TOL_SET4) && (((max_det_pw > 200) && (max_det_pw <= 430)) &&
		(((max_det_pw - min_det_pw)  >= (3*blank_time - (max_pw_tol/2))) &&
		((max_det_pw - min_det_pw) <= (3*blank_time + (max_pw_tol/2)))))) ?
		TRUE : FALSE;

	return (chk_set1 || chk_set2 || chk_set3 || chk_set4);
}

#define MIN_INTERVAL	3000	/* 150 us */
#define MAX_INTERVAL	120000	/* 6 ms */
#define MIN_CHIRP	0	/* 0 MHz */
#define MAX_CHIRP	80	/* 40 MHz */
#define MIN_FC	-160	/* -80 MHz */
#define MAX_FC	160	/* 80 MHz */

static void
get_min_max_avg_statistic(pulse_data_t pulses[], int idx, int num_pulses,
	uint32 *min_detected_interval_p, uint32 *max_detected_interval_p,
	int *avg_fc_p, int *var_fc_p)
{
	int i;
	uint32 min_interval = MAX_INTERVAL;
	uint32 max_interval = MIN_INTERVAL;
	uint32 interval;
	uint min_chirp = MAX_CHIRP;
	uint max_chirp = MIN_CHIRP;
	uint chirp;
	int min_fc = MAX_FC;
	int max_fc = MIN_FC;
	int fc;
	int sum_detected_fc = 0;

	for (i = idx; i < idx + num_pulses; ++i) {
		ASSERT(i < RDR_LIST_SIZE);
		interval = pulses[i].interval;
		chirp = pulses[i].chirp;
		fc = pulses[i].fc;
		if (interval < min_interval) {
			min_interval = interval;
		}
		if (interval > max_interval) {
			max_interval = interval;
		}
		if (chirp < min_chirp) {
			min_chirp = chirp;
		}
		if (chirp > max_chirp) {
			max_chirp = chirp;
		}
		if (fc < min_fc) {
			min_fc = fc;
		}
		if (fc > max_fc) {
			max_fc = fc;
		}
		sum_detected_fc = sum_detected_fc + fc;
	}

	*min_detected_interval_p = min_interval;
	*max_detected_interval_p = max_interval;
	*avg_fc_p = sum_detected_fc / num_pulses;
	if ((max_fc - *avg_fc_p) >
		(*avg_fc_p - min_fc)) {
		*var_fc_p = *avg_fc_p - min_fc;
	} else {
		*var_fc_p = max_fc - *avg_fc_p;
	}
}

static void
wlc_phy_radar_detect_run_epoch(phy_info_t *pi,
	radar_work_t *rt, radar_params_t *rparams,
	int epoch_length,
	uint8 *det_type_p, int *nconsecq_pulses_p,
	int *detected_pulse_index_p, int *min_detected_pw_p, int *max_detected_pw_p,
	uint32 *min_detected_interval_p, uint32 *max_detected_interval_p, int *avg_fc_p,
	int *var_fc_p, uint16 blank_time, bool sec_pll)
{
	uint16 j;
	bool radar_detected = FALSE, fm_in_range = TRUE;
	uint8 det_type;
	uint8 previous_det_type = RADAR_TYPE_NONE;
	int detected_pulse_index = 0;
	int nconsecq_pulses = 0;
	int nconseq2even, nconseq2odd;
	int nconseq3a, nconseq3b, nconseq3c;
	int first_interval;
	bool pw_blank_condition;
	const radar_spec_t *pRadar;
	int pw_var_tol;

	epoch_length = wlc_phy_radar_filter_list(rt->pulses, epoch_length,
	                                         MIN_INTERVAL, MAX_INTERVAL);

	/* Detect contiguous only pulses */
	detected_pulse_index = 0;
	nconsecq_pulses = 0;
	nconseq2even = 0;
	nconseq2odd = 0;
	nconseq3a = 0;
	nconseq3b = 0;
	nconseq3c = 0;

	first_interval = rt->pulses[0].interval;
	*min_detected_pw_p = rt->pulses[0].pw;
	*max_detected_pw_p = *min_detected_pw_p;

	for (j = 0; j < epoch_length - 1; j++) {
		uint16 curr_pw = rt->pulses[j].pw;
		uint32 curr_interval = rt->pulses[j].interval;

		/* contiguous pulse detection */
		pRadar = wlc_phy_radar_detect_match(pi, rparams, &rt->pulses[j],
		                                    rparams->radar_args.feature_mask);
		det_type = pRadar->radar_type;

		if (det_type != RADAR_TYPE_NONE) {
			/* same interval detected as first ? */
			if (ABS((int32)(curr_interval - first_interval)) <
			    ((ACMAJORREV_47(pi->pubpi->phy_rev) &&
			    rparams->radar_args.max_deltat == 1) ?
			    rparams->radar_args.quant : (INTERVAL_TOLERANCE +
			    (int32)curr_interval / 4000))) {
				if (curr_pw < *min_detected_pw_p) {
					*min_detected_pw_p = curr_pw;
				}
				if (curr_pw > *max_detected_pw_p) {
					*max_detected_pw_p = curr_pw;
				}
				if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
					ACMAJORREV_33(pi->pubpi->phy_rev)) {
					if (PHY_LESI_ON(pi)) {
						pw_blank_condition = phy_radar_ext_pw_tol(
						pi, rparams, *max_detected_pw_p, *min_detected_pw_p,
						blank_time, EXT_PW_TOL_SETALL);
					} else {
						pw_blank_condition = phy_radar_ext_pw_tol(
						pi, rparams, *max_detected_pw_p, *min_detected_pw_p,
						blank_time, EXT_PW_TOL_SET1);
					}
					pw_var_tol = PULSE_WIDTH_DIFFERENCE + curr_pw / 200;
				} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
					pw_blank_condition = phy_radar_ext_pw_tol(
					pi, rparams, *max_detected_pw_p, *min_detected_pw_p,
					blank_time, (EXT_PW_TOL_SET1 | EXT_PW_TOL_SET2));
					pw_var_tol = ((det_type == RADAR_TYPE_ETSI_4) ?
						PULSE_WIDTH_DIFFERENCE_ETSI4 :
						PULSE_WIDTH_DIFFERENCE) + curr_pw / 200;
				} else {
					pw_blank_condition = FALSE;
					pw_var_tol = PULSE_WIDTH_DIFFERENCE + curr_pw / 200;
				}

				if ((*max_detected_pw_p - *min_detected_pw_p <=
				    ((ACMAJORREV_47(pi->pubpi->phy_rev) &&
				    rparams->radar_args.max_deltat == 1) ?
				    rparams->radar_args.max_pw_tol : pw_var_tol)) ||
				    pw_blank_condition) {
					uint8 min_pulses;
					if (ACMAJORREV_47(pi->pubpi->phy_rev) &&
					    rparams->radar_args.max_deltat == 1) {
					    min_pulses = (det_type == RADAR_TYPE_FCC_1 ||
					      det_type == RADAR_TYPE_JP4 ||
					      det_type == RADAR_TYPE_JP2_1_1) ?
					      ((rparams->radar_args.ncontig >> 13) & 0x7) - 1 :
					      (det_type == RADAR_TYPE_JP1_2) ?
					      ((rparams->radar_args.ncontig >> 10) & 0x7) - 1 :
					      (det_type == RADAR_TYPE_ETSI_1) ?
					      (rparams->radar_args.npulses_fra & 0xf) - 1 :
					      (det_type == RADAR_TYPE_ETSI_2) ?
					      ((rparams->radar_args.npulses_fra >> 4) & 0xf) - 1 :
					      (det_type == RADAR_TYPE_ETSI_3 ||
					      det_type == RADAR_TYPE_ETSI_4) ?
					      ((rparams->radar_args.npulses_fra >> 8) & 0xf) - 1 :
					      rparams->radar_args.npulses - 1;
					} else {
					    min_pulses = MIN(MIN_PULSES, pRadar->num_pulses);
					}
					if (++nconsecq_pulses >= min_pulses) {
						fm_in_range = phy_radar_ext_fm_check(pi, rparams,
						rt->pulses, det_type, previous_det_type, j,
						nconsecq_pulses, *min_detected_pw_p, sec_pll);
						/* requirements match a Radar */
						radar_detected = TRUE;
						*nconsecq_pulses_p = nconsecq_pulses;
						detected_pulse_index = j - min_pulses + 1;
						get_min_max_avg_statistic(rt->pulses,
								detected_pulse_index, min_pulses,
								min_detected_interval_p,
								max_detected_interval_p,
								avg_fc_p,
								var_fc_p);
						PHY_RADAR(("Radar %d: detected_pulse_index=%d\n",
							det_type, detected_pulse_index));
						break;
					}
				} else {
					/* reset to current pulse */
					first_interval = rt->pulses[j].interval;
					*min_detected_pw_p = curr_pw;
					*max_detected_pw_p = curr_pw;
					previous_det_type = det_type;
					nconsecq_pulses = 0;
				}
			} else {
				/* reset to current pulse */
				first_interval = rt->pulses[j].interval;
				*min_detected_pw_p = curr_pw;
				*max_detected_pw_p = curr_pw;
				previous_det_type = det_type;
				nconsecq_pulses = 0;
			}
		} else {
			previous_det_type = RADAR_TYPE_NONE;
			first_interval = rt->pulses[j].interval;
			nconsecq_pulses = 0;
		}

		/* staggered 2/3 single filters */
		/* no stagger for ch > 144 (UK radar) */
		if ((rparams->radar_args.feature_mask & RADAR_FEATURE_ETSI_DETECT) &&
			CHSPEC_CHANNEL(pi->radio_chanspec) <= 144) {
			/* staggered 2 even */
			if (j >= 2 && j % 2 == 0) {
				phy_radar_check_staggered2(pi, rparams, &nconseq2even,
					rt->pulses, j);
			}

			/* staggered 2 odd */
			if (j >= 3 && j % 2 == 1) {
				phy_radar_check_staggered2(pi, rparams, &nconseq2odd,
					rt->pulses, j);
			}

			if (nconseq2even >= rparams->radar_args.npulses_stg2 / 2 &&
			    nconseq2odd >= rparams->radar_args.npulses_stg2 / 2) {
				radar_detected = TRUE;
				detected_pulse_index = j -
					(rparams->radar_args.npulses_stg2 / 2) * 2 - 1;
				if (detected_pulse_index < 0)
					detected_pulse_index = 0;
				if ((rt->pulses[detected_pulse_index].interval +
				    rt->pulses[detected_pulse_index+1].interval) / 2 >=
				    MAX_STAGGERED_ETSI_6_INTERVAL) {
					det_type = RADAR_TYPE_ETSI_5_STG2;
				} else {
					det_type = RADAR_TYPE_ETSI_6_STG2;
				}
				*min_detected_pw_p = rt->pulses[detected_pulse_index].pw;
				*max_detected_pw_p = *min_detected_pw_p;
				*nconsecq_pulses_p = nconseq2odd + nconseq2even;
				get_min_max_avg_statistic(rt->pulses, detected_pulse_index,
					rparams->radar_args.npulses_stg2, min_detected_interval_p,
					max_detected_interval_p, avg_fc_p, var_fc_p);
				PHY_RADAR(("j=%d detected_pulse_index=%d\n",
					j, detected_pulse_index));
				break;	/* radar detected */
			}

			/* staggered 3-a */
			if (j >= 3 && j % 3 == 0) {
				phy_radar_check_staggered3(pi, rparams, &nconseq3a, rt->pulses, j);
			}

			/* staggered 3-b */
			if (j >= 4 && j % 3 == 1) {
				phy_radar_check_staggered3(pi, rparams, &nconseq3b, rt->pulses, j);
			}

			/* staggered 3-c */
			if (j >= 5 && j % 3 == 2) {
				phy_radar_check_staggered3(pi, rparams, &nconseq3c, rt->pulses, j);
			}

			if (nconseq3a >= rparams->radar_args.npulses_stg3 / 3 &&
			    nconseq3b >= rparams->radar_args.npulses_stg3 / 3 &&
			    nconseq3c >= rparams->radar_args.npulses_stg3 / 3) {
				radar_detected = TRUE;
				detected_pulse_index = j -
					(rparams->radar_args.npulses_stg3 / 3) * 3 - 2;
				if (detected_pulse_index < 0)
					detected_pulse_index = 0;
				if ((rt->pulses[detected_pulse_index].interval +
				    rt->pulses[detected_pulse_index+1].interval) / 2 >=
				    MAX_STAGGERED_ETSI_6_INTERVAL) {
					det_type = RADAR_TYPE_ETSI_5_STG3;
				} else {
					det_type = RADAR_TYPE_ETSI_6_STG3;
				}
				*min_detected_pw_p = rt->pulses[detected_pulse_index].pw;
				*max_detected_pw_p = *min_detected_pw_p;
				*nconsecq_pulses_p = nconseq3a + nconseq3b + nconseq3c;
				get_min_max_avg_statistic(rt->pulses, detected_pulse_index,
					rparams->radar_args.npulses_stg3, min_detected_interval_p,
					max_detected_interval_p, avg_fc_p, var_fc_p);
				PHY_RADAR(("j=%d detected_pulse_index=%d\n",
					j, detected_pulse_index));
				break;	/* radar detected */
			}
		}
	}  /* for (j = 0; j < epoch_length - 2; j++) */

	if (radar_detected && (fm_in_range == TRUE)) {
		*det_type_p = det_type;
		*detected_pulse_index_p = detected_pulse_index;
	} else {
		*det_type_p = RADAR_TYPE_NONE;
		*min_detected_interval_p = 0;
		*max_detected_interval_p = 0;
		*nconsecq_pulses_p = 0;
		*detected_pulse_index_p = 0;
		*var_fc_p = 0;
	}
}

#ifdef BCMDBG
static void
phy_radar_output_pulses(phy_info_t *pi, pulse_data_t pulses[], uint16 length, uint16 feature_mask)
{
	uint16 i;

	if (feature_mask & RADAR_FEATURE_DEBUG_PULSE_DATA) {
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_47(pi->pubpi->phy_rev)) {
			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < length; i++)
			  PHY_RADAR(("%u ", pulses[i].interval));
					  PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < length; i++)
				PHY_RADAR(("%u ", pulses[i].interval -
				   pulses[i - 1].interval));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < length; i++)
			  PHY_RADAR(("%d-%d ", pulses[i].pw, i));

			PHY_RADAR(("\nFM:  "));
			for (i = 0; i < length; i++)
			  PHY_RADAR(("%d-%d ", pulses[i].fm, i));

			PHY_RADAR(("\nNotradar:  "));
			for (i = 0; i < length; i++)
			  PHY_RADAR(("%d-%d ", pulses[i].notradar, i));
			PHY_RADAR(("\n"));
		} else {
			PHY_RADAR(("\ntstamp=[ "));
			for (i = 0; i < length; i++) {
				PHY_RADAR(("%u ", pulses[i].interval));
			}
			PHY_RADAR(("];\n"));

			PHY_RADAR(("(Interval, PW, FM, FC, CHIRP, NOTRADAR) = [ "));
			for (i = 1; i < length; i++) {
				PHY_RADAR(("(%u, %u, %d, %d, %u, %u) ",
					pulses[i].interval - pulses[i - 1].interval,
					pulses[i].pw, pulses[i].fm,
					pulses[i].fc, pulses[i].chirp,
					pulses[i].notradar));
			}
			PHY_RADAR(("];\n"));
		}
	}
}
#else
#define phy_radar_output_pulses(a, b, c)
#endif /* BCMDBG */

static void
phy_radar_detect_fcc5(phy_info_t *pi,
                     phy_radar_st_t *st, radar_work_t *rt, bool sec_pll, bool bw80_80_mode,
                     radar_detected_info_t *radar_detected)
{
	uint16 i;
	uint16 j;
	int k;
	uint8 ant;
	int wr_ptr;
	uint16 mlength;
	int32 deltat;
	int skip_type;
	uint16 width;
	int16 fm;
	bool valid_lp;
	int32 salvate_intv;
	int pw_dif, pw_tol, fm_dif, fm_tol;
	int FMCombineOffset;
	int16 ant_num = 1;
	int min_fm_lp_sc = 0;
	int16 var_fc_bin5 = 0;
	radar_params_t *rparams = &st->rparams;
	bool *first_radar_indicator;
	radar_lp_info_t *rlpt;
	wl_radar_status_t *rt_status;

	if (ISACPHY(pi) && TONEDETECTION) {
		FMCombineOffset =
			MAX((2 << (((rparams->radar_args.st_level_time>> 12) & 0xf) -1)) -1, 0);
	} else {
		FMCombineOffset = 0;
	}

	/* t2_min[15:12] = x; if n_non_single >= x && lp_length > npulses_lp => bin5 detected */
	/* t2_min[11:10] = # times combining adjacent pulses < min_pw_lp  */
	/* t2_min[9] = fm_tol enable */
	/* t2_min[8] = skip_type 5 enable */
	/* t2_min[7:4] = y; bin5 remove pw <= 10*y  */
	/* t2_min[3:0] = t; non-bin5 remove pw <= 5*y */
	if (sec_pll == FALSE || !(PHY_SUPPORT_SCANCORE(pi) || PHY_SUPPORT_BW80P80(pi))) {
		if (bw80_80_mode == FALSE) {
			ant_num = GET_RDR_NANTENNAS(pi);
		} else {
			ant_num = 1;
		}
		first_radar_indicator = &st->first_radar_indicator;
		rt_status = &st->radar_status;
		rlpt = &st->radar_lp_info;
	} else {
		ant_num = 1;
		if (((st->rparams.radar_thrs2.highpow_war_enb >> 2) & 0x3) == 0) {
			min_fm_lp_sc = rparams->radar_args.min_fm_lp >> 1;
		} else if (((st->rparams.radar_thrs2.highpow_war_enb >> 2) & 0x3) == 1) {
			min_fm_lp_sc = (rparams->radar_args.min_fm_lp * 3) >> 2;
		} else if (((st->rparams.radar_thrs2.highpow_war_enb >> 2) & 0x3) == 2) {
			min_fm_lp_sc = (rparams->radar_args.min_fm_lp * 7) >> 3;
		} else {
			min_fm_lp_sc = rparams->radar_args.min_fm_lp;
		}
		if (bw80_80_mode == FALSE) {
			rt_status = st->radar_status_sc;
			first_radar_indicator = &st->first_radar_indicator_sc;
		} else {
			rt_status = &st->radar_status;
			first_radar_indicator = &st->first_radar_indicator;
		}
		rlpt = st->radar_work_lp_sc;
	}
	/* remove "noise" pulses with small pw */
	for (ant = 0; ant < ant_num; ant++) {
		wr_ptr = 0;
		mlength = rt->length_bin5[ant];
		for (i = 0; i < rt->length_bin5[ant]; i++) {
			if (rt->pulses_bin5[ant][i].pw >
				10*((rparams->radar_args.t2_min >> 4) & 0xf) &&
				rt->pulses_bin5[ant][i].fm >
				((ISACPHY(pi) && TONEDETECTION &&
				sec_pll == FALSE && bw80_80_mode == FALSE) ?
				rparams->radar_args.min_fm_lp : min_fm_lp_sc)) {
				rt->pulses_bin5[ant][wr_ptr] = rt->pulses_bin5[ant][i];
				++wr_ptr;
			} else {
				mlength--;
			}
		}	/* for mlength loop */
		rt->length_bin5[ant] = mlength;
	}	/* for ant loop */

#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_PULSE_DATA) &&
		((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_SHORT_PULSE) == 0)) {
		/* bin5 */
		for (ant = 0; ant < ant_num; ant++) {
			PHY_RADAR(("\nBin5 after removing noise pulses with pw <= %d",
				((rparams->radar_args.t2_min >> 4) & 0xf) * 10));
			PHY_RADAR(("\nAnt %d: %d pulses, ", ant, rt->length_bin5[ant]));

			phy_radar_output_pulses(pi, rt->pulses_bin5[ant], rt->length_bin5[ant],
			                        rparams->radar_args.feature_mask);
		}
	}
#endif /* BCMDBG */

	/* Combine pulses that are adjacent for ant 0 and ant 1 */
	for (ant = 0; ant < GET_RDR_NANTENNAS(pi); ant++) {
		rt->length =  rt->length_bin5[ant];
		for (k = 0; k < ((rparams->radar_args.t2_min >> 10) & 0x3); k++) {
			mlength = rt->length;
			if (mlength > 1) {
				for (i = 1; i < mlength; i++) {
					deltat = ABS((int32)(rt->pulses_bin5[ant][i].interval -
					             rt->pulses_bin5[ant][i-1].interval));

					if (deltat <= (int32)rparams->radar_args.max_pw_lp) {
						wlc_phy_radar_fc_chirp_cmb(
							&rt->pulses_bin5[ant][i-1],
							&rt->pulses_bin5[ant][i]);
						rt->pulses_bin5[ant][i-1].pw =
							deltat + rt->pulses_bin5[ant][i].pw;
						rt->pulses_bin5[ant][i-1].fm =
							(rt->pulses_bin5[ant][i-1].fm +
							rt->pulses_bin5[ant][i].fm) -
							FMCombineOffset;
						for (j = i; j < mlength - 1; j++) {
							rt->pulses_bin5[ant][j] =
								rt->pulses_bin5[ant][j+1];
						}
						mlength--;
						rt->length--;
					}	/* if deltat */
				}	/* for mlength loop */
			}	/* mlength > 1 */
		}
	} /* for ant loop */

	ant = 0;	/* Use data from one of the antenna */
	rt->length = 0;
	for (i = 0; i < rt->length_bin5[ant]; i++) {
		rt->pulses[rt->length] = rt->pulses_bin5[ant][i];
		rt->length++;
	}

#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_PULSE_DATA) &&
		((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_SHORT_PULSE) == 0)) {
		/* bin5 */
		PHY_RADAR(("\nBin5 after use data from one of two antennas"));
		PHY_RADAR(("\n%d pulses, ", rt->length));

		phy_radar_output_pulses(pi, rt->pulses, rt->length,
			rparams->radar_args.feature_mask);
	}
#endif /* BCMDBG */

	/* remove pulses that are spaced less than high byte of "blank" */
	for (i = 1; i < rt->length; i++) {
		deltat = ABS((int32)(rt->pulses[i].interval - rt->pulses[i-1].interval));
		if (deltat < (ISACPHY(pi) ?
			(((int32)rparams->radar_args.blank & 0xff00) >> 8) : 128)) {
			for (j = i - 1; j < (rt->length); j++) {
				rt->pulses[j] = rt->pulses[j + 1];
			}
			rt->length--;
		}
	}

	if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
		/* remove entries chirp out of range */
		j = 0;
		for (i = 0; i < rt->length; i++) {
			if (rt->pulses[i].chirp <= FCC5_CHIRP) {
				rt->pulses[j] = rt->pulses[i];
				j++;
			}
		}
		rt->length = j;
	}

#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_PULSE_DATA) &&
		((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_SHORT_PULSE) == 0)) {
		/* bin5 */
		PHY_RADAR(("\nBin5 after removing pulses that are spaced < %d",
		           rparams->radar_args.quant >> 8));
		PHY_RADAR(("\n%d pulses, ", rt->length));

		phy_radar_output_pulses(pi, rt->pulses, rt->length,
			rparams->radar_args.feature_mask);
	}
#endif /* BCMDBG */

	/* prune lp buffer */
	/* remove any entry outside the time max delta_t_lp */
	if (rlpt->lp_length > 1) {
		deltat = ABS((int32)(rlpt->lp_buffer[rlpt->lp_length - 1] - rlpt->lp_buffer[0]));
		i = 0;
		while ((i < (rlpt->lp_length - 1)) && (deltat > MAX_LP_BUFFER_SPAN_20MHZ)) {
			i++;
			deltat = ABS((int32)(rlpt->lp_buffer[rlpt->lp_length - 1] -
					rlpt->lp_buffer[i]));
		}

		if (i > 0) {
			for (j = i; j < rlpt->lp_length; j++) {
				rlpt->lp_buffer[j-i] = rlpt->lp_buffer[j];
			}

			rlpt->lp_length -= i;
		}
	}
	/* First perform FCC-5 detection */
	/* add new pulses */

	/* process each new pulse */
	for (i = 0; i < rt->length; i++) {
		deltat = ABS((int32)(rt->pulses[i].interval - rlpt->last_tstart));
		salvate_intv = ABS((int32)(rt->pulses[i].interval - rlpt->last_skipped_time));

		if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi) &&
		    (st->rparams.radar_thrs2.highpow_war_enb & 0x1) == 1) {
			valid_lp = (rt->pulses[i].pw >= PW_TOL_HIPOWWAR_BIN5) &&
				(rt->pulses[i].pw <= rparams->radar_args.max_pw_lp) &&
				(rt->pulses[i].fm >=
				((ISACPHY(pi) && TONEDETECTION &&
					sec_pll == FALSE && bw80_80_mode == FALSE) ?
				rparams->radar_args.min_fm_lp/2 :
				min_fm_lp_sc/2)) &&
				(deltat >= rparams->min_deltat_lp);
		} else {
			valid_lp = (rt->pulses[i].pw >= rparams->radar_args.min_pw_lp) &&
				(rt->pulses[i].pw <= rparams->radar_args.max_pw_lp) &&
				(rt->pulses[i].fm >= ((sec_pll == FALSE && bw80_80_mode == FALSE) ?
				rparams->radar_args.min_fm_lp : min_fm_lp_sc)) &&
				(deltat >= rparams->min_deltat_lp);
		}

		if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
			if (((st->rparams.radar_thrs2.highpow_war_enb >> 1) & 0x1) == 1) {
				bool chirp_condi;
				if (!PHY_AS_80P80(pi, pi->radio_chanspec)) {
					chirp_condi = (rt->pulses[i].chirp >
					((rt->pulses[i].pw >=
					PW_CHIRP_ADJ_TH_BIN5) ?
					CHIRP_TH2 : CHIRP_TH1));
				} else {
					/* ignore chirp condition when its */
					/* estimation is abort and fc is > 30MHz */
					chirp_condi = (rt->pulses[i].chirp >
					((rt->pulses[i].pw >=
					PW_CHIRP_ADJ_TH_BIN5) ?
					CHIRP_TH2 : CHIRP_TH1)) ||
					(rt->pulses[i].chirp == 0 &&
					rt->pulses[i].fc >= CHIRP_FC_TH);
				}
				valid_lp = valid_lp && chirp_condi;
			}
		}

		/* filter out: max_deltat_l < pburst_intv_lp < min_burst_intv_lp */
		/* this was skip-type = 2, now not skipping for this */
		valid_lp = valid_lp &&(deltat <= (int32) rparams->max_deltat_lp ||
			deltat >= (int32) rparams->radar_args.min_burst_intv_lp);

		//Need to check notradar for bin5 radar detection of scan core
		if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
			if (sec_pll == FALSE && bw80_80_mode == FALSE) {
				if ((st->rparams.radar_thrs2.notradar_enb>>3 & 0x1) == 1)
					valid_lp = valid_lp &&
						(rt->pulses[i].notradar <
						st->rparams.radar_thrs2.max_notradar_lp);
			} else {
				if ((st->rparams.radar_thrs2.notradar_enb>>1 & 0x1) == 1)
					valid_lp = valid_lp &&
						(rt->pulses[i].notradar <
						st->rparams.radar_thrs2.max_notradar_lp_sc);
			}
		}

		if (rlpt->lp_length > 0 && rlpt->lp_length < LP_BUFFER_SIZE) {
			valid_lp = valid_lp &&
				(rt->pulses[i].interval != rlpt->lp_buffer[rlpt->lp_length]);
		}

		skip_type = 0;
		if (valid_lp && deltat >= (int32) rparams->radar_args.min_burst_intv_lp &&
			deltat < (int32) rparams->radar_args.max_burst_intv_lp) {
			rlpt->lp_cnt = 0;
		}

		/* skip the pulse if outside of pulse interval range (1-2ms), */
		/* burst to burst interval not within range, more than 3 pulses in a */
		/* burst, and not skip salvated */

		if ((valid_lp && ((rlpt->lp_length != 0)) &&
			((deltat < (int32) rparams->min_deltat_lp) ||
			(deltat > (int32) rparams->max_deltat_lp &&
			deltat < (int32) rparams->radar_args.min_burst_intv_lp) ||
			(deltat > (int32) rparams->radar_args.max_burst_intv_lp) ||
			(rlpt->lp_cnt > 2)))) {	/* possible skip lp */

			/* get skip type */
			if (deltat < (int32) rparams->min_deltat_lp) {
				skip_type = 1;
			} else if (deltat > (int32) rparams->max_deltat_lp &&
				deltat < (int32) rparams->radar_args.min_burst_intv_lp) {
				skip_type = 2;
			} else if (deltat > (int32) rparams->radar_args.max_burst_intv_lp) {
				skip_type = 3;
				rlpt->lp_cnt = 0;
			} else if (rlpt->lp_cnt > 2) {
				skip_type = 4;
			} else {
				skip_type = 999;
			}

			/* skip_salvate */
			if ((((salvate_intv > (int32) rparams->min_deltat_lp &&
				salvate_intv < (int32) rparams->max_deltat_lp)) ||
				((salvate_intv > (int32)rparams->radar_args.min_burst_intv_lp) &&
				(salvate_intv < (int32)rparams->radar_args.max_burst_intv_lp))) &&
				(skip_type != 4)) {
				/* note valid_lp is not reset in here */
				skip_type = -1;  /* salvated PASSED */
				if (salvate_intv >= (int32) rparams->radar_args.min_burst_intv_lp &&
					salvate_intv <
						(int32) rparams->radar_args.max_burst_intv_lp) {
					rlpt->lp_cnt = 0;
				}
			}
		} else {  /* valid lp not by skip salvate */
			skip_type = -2;
		}

		width = 0;
		fm = 0;
		pw_dif = 0;
		fm_dif = 0;
		pw_tol = rparams->radar_args.max_span_lp & 0xff;
		fm_tol = 0;
		/* monitor the number of pw and fm matching */
		/* max_span_lp[15:12] = skip_tot max */
		/* max_span_lp[11:8] = x, x/16 = % alowed fm tollerance */
		/* max_span_lp[7:0] = alowed pw tollerance */
		if (valid_lp && skip_type <= 0) {
			if (rlpt->lp_cnt == 0) {
				rlpt->lp_pw[0] = rt->pulses[i].pw;
				rlpt->lp_fm[0] = rt->pulses[i].fm;
			} else if (rlpt->lp_cnt == 1) {
				width = rlpt->lp_pw[0];
				fm = rlpt->lp_fm[0];
				pw_dif = ABS(rt->pulses[i].pw - width);
				fm_dif = ABS(rt->pulses[i].fm - fm);
				if (rparams->radar_args.t2_min & 0x200) {
					if ((st->rparams.radar_thrs2.highpow_war_enb & 0x1)
						== 1) {
						fm_tol = (fm*
							((rparams->radar_args.max_span_lp
							>> 8) & 0xf))/4;
					} else {
						fm_tol = (fm*((rparams->radar_args.max_span_lp >> 8)
							& 0xf))/16;
					}
				} else {
					fm_tol = 999;
				}
				if (pw_dif < pw_tol && fm_dif < fm_tol) {
					rlpt->lp_pw[1] = rt->pulses[i].pw;
					rlpt->lp_fm[1] = rt->pulses[i].fm;
					++rlpt->lp_n_non_single_pulses;
					++rlpt->lp_pw_fm_matched;
				} else if (rlpt->lp_just_skipped) {
					width = rlpt->lp_skipped_pw;
					fm = rlpt->lp_skipped_fm;
					pw_dif = ABS(rt->pulses[i].pw - width);
					fm_dif = ABS(rt->pulses[i].fm - fm);
					if (rparams->radar_args.t2_min & 0x200) {
						if ((st->rparams.radar_thrs2.highpow_war_enb & 0x1)
							== 1) {
							fm_tol = (fm*
								((rparams->radar_args.max_span_lp
								>> 8) & 0xf))/4;
						} else {
							fm_tol = (fm*
							((rparams->radar_args.max_span_lp >> 8)
							& 0xf))/16;
						}
					} else {
						fm_tol = 999;
					}
					if (pw_dif < pw_tol && fm_dif < fm_tol) {
						rlpt->lp_pw[1] = rt->pulses[i].pw;
						rlpt->lp_fm[1] = rt->pulses[i].fm;
						++rlpt->lp_n_non_single_pulses;
						++rlpt->lp_pw_fm_matched;
						skip_type = -1;  /* salvated PASSED */
					} else {
						if (rparams->radar_args.t2_min & 0x100) {
							skip_type = 5;
						}
					}
				} else {
					if (rparams->radar_args.t2_min & 0x100) {
						skip_type = 5;
					}
				}
			} else if (rlpt->lp_cnt == 2) {
				width = rlpt->lp_pw[1];
				fm = rlpt->lp_fm[1];
				pw_dif = ABS(rt->pulses[i].pw - width);
				fm_dif = ABS(rt->pulses[i].fm - fm);
				if (rparams->radar_args.t2_min & 0x200) {
					fm_tol = (fm*((rparams->radar_args.max_span_lp >> 8)
						& 0xf))/16;
				} else {
					fm_tol = 999;
				}
				if (pw_dif < pw_tol && fm_dif < fm_tol) {
					rlpt->lp_pw[2] = rt->pulses[i].pw;
					rlpt->lp_fm[2] = rt->pulses[i].fm;
					++rlpt->lp_n_non_single_pulses;
					++rlpt->lp_pw_fm_matched;
				} else if (rlpt->lp_just_skipped) {
					width = rlpt->lp_skipped_pw;
					fm = rlpt->lp_skipped_fm;
					pw_dif = ABS(rt->pulses[i].pw - width);
					fm_dif = ABS(rt->pulses[i].fm - fm);
					if (rparams->radar_args.t2_min & 0x200) {
						fm_tol = (fm*((rparams->radar_args.max_span_lp >> 8)
							& 0xf))/16;
					} else {
						fm_tol = 999;
					}
					if (pw_dif < pw_tol && fm_dif < fm_tol) {
						rlpt->lp_pw[2] = rt->pulses[i].pw;
						rlpt->lp_fm[2] = rt->pulses[i].fm;
						++rlpt->lp_n_non_single_pulses;
						++rlpt->lp_pw_fm_matched;
						skip_type = -1;  /* salvated PASSED */
					} else {
						if (rparams->radar_args.t2_min & 0x100) {
							skip_type = 5;
						}
					}
				} else {
					if (rparams->radar_args.t2_min & 0x100) {
						skip_type = 5;
					}
				}
			}
		}

		if (valid_lp && skip_type != -1 && skip_type != -2) {	/* skipped lp */
			valid_lp = FALSE;
			rlpt->lp_skip_cnt++;
			rlpt->lp_skip_tot++;
			rlpt->lp_just_skipped = TRUE;
			rlpt->lp_skipped_pw = rt->pulses[i].pw;
			rlpt->lp_skipped_fm = rt->pulses[i].fm;

			rlpt->last_skipped_time = rt->pulses[i].interval;

#ifdef BCMDBG
			/* print "SKIPPED LP" debug messages */
			PHY_RADAR(("Skipped LP:"
/*
				" KTstart=%u Klast_ts=%u Klskip=%u"
*/
				" nLP=%d nSKIP=%d KIntv=%u"
				" Ksalintv=%d PW=%d FM=%d"
				" Type=%d pulse#=%d skip_tot=%d csect_single=%d\n",
/*
			(rt->pulses[i].interval)/1000, rlpt->last_tstart/1000, tmp_uint32/1000,
*/
				rlpt->lp_length, rlpt->lp_skip_cnt, deltat/1000, salvate_intv/1000,
				rt->pulses[i].pw, rt->pulses[i].fm,
				skip_type, rlpt->lp_cnt, rlpt->lp_skip_tot, rlpt->lp_csect_single));
			if (skip_type == 5) {
				PHY_RADAR(("           "
					" pw2=%d pw_dif=%d pw_tol=%d fm2=%d fm_dif=%d fm_tol=%d\n",
					width, pw_dif, pw_tol, fm, fm_dif, fm_tol));
			}
			if (skip_type == 999) {
				PHY_RADAR(("UNKOWN SKIP TYPE: %d\n", skip_type));
			}
#endif /* BCMDBG */

			/* if a) 2 consecutive skips, or */
			/*    b) too many consective singles, or */
			/*    c) too many total skip so far */
			/*  then reset lp buffer ... */
			if (rlpt->lp_skip_cnt >= rparams->radar_args.nskip_rst_lp) {
				if (rlpt->lp_len_his_idx < LP_LEN_HIS_SIZE) {
					rlpt->lp_len_his[rlpt->lp_len_his_idx] = rlpt->lp_length;
					rlpt->lp_len_his_idx++;
				}
				rlpt->lp_length = 0;
				rlpt->lp_skip_tot = 0;
				rlpt->lp_skip_cnt = 0;
				rlpt->lp_csect_single = 0;
				rlpt->lp_pw_fm_matched = 0;
				rlpt->lp_n_non_single_pulses = 0;
				rlpt->lp_cnt = 0;
				rlpt->min_detected_fc_bin5 = MAX_FC;
				rlpt->max_detected_fc_bin5 = MIN_FC;
				rlpt->avg_detected_fc_bin5 = 0;
				rlpt->subband_result = 0;
			}
		} else if (valid_lp && (rlpt->lp_length < LP_BUFFER_SIZE)) {	/* valid lp */
			if (rt->pulses[i].fc <= rlpt->min_detected_fc_bin5)
				rlpt->min_detected_fc_bin5 = rt->pulses[i].fc;
			if (rt->pulses[i].fc >= rlpt->max_detected_fc_bin5)
				rlpt->max_detected_fc_bin5 = rt->pulses[i].fc;

			if (rlpt->avg_detected_fc_bin5 == 0) {
				rlpt->avg_detected_fc_bin5 = (rt->pulses[i].fc*4);
			} else {
				rlpt->avg_detected_fc_bin5 = 3*(rlpt->avg_detected_fc_bin5)
								+ (rt->pulses[i].fc*4);
				rlpt->avg_detected_fc_bin5 = rlpt->avg_detected_fc_bin5/4;
			}
			/* reset consecutive singles counter if pulse # > 0 */
			if (rlpt->lp_cnt > 0) {
				rlpt->lp_csect_single = 0;
			} else {
				++rlpt->lp_csect_single;
			}

			rlpt->lp_just_skipped = FALSE;
			/* print "VALID LP" debug messages */
			rlpt->lp_skip_cnt = 0;
			PHY_RADAR(("Valid LP:"
/*
				" KTstart=%u KTlast_ts=%u Klskip=%u"
*/
				" KIntv=%u"
				" Ksalintv=%d PW=%d FM=%d pulse#=%d"
				" pw2=%d pw_dif=%d pw_tol=%d fm2=%d fm_dif=%d fm_tol=%d\n"
				" FC=%d CHIRP=%d NOTRADAR=%d\n",
/*
				(rt->pulses[i].interval)/1000, rlpt->last_tstart/1000,
					rlpt->last_skipped_time/1000,
*/
				deltat/1000, salvate_intv/1000,
				rt->pulses[i].pw, rt->pulses[i].fm, rlpt->lp_cnt,
				width, pw_dif, pw_tol,
				fm, fm_dif, fm_tol,
				rt->pulses[i].fc, rt->pulses[i].chirp, rt->pulses[i].notradar));
			PHY_RADAR(("         "
				" nLP=%d nSKIP=%d skipped_salvate=%d"
				" pw_fm_matched=%d #non-single=%d skip_tot=%d csect_single=%d\n",
				rlpt->lp_length + 1, rlpt->lp_skip_cnt, (skip_type == -1 ? 1 : 0),
				rlpt->lp_pw_fm_matched,
				rlpt->lp_n_non_single_pulses, rlpt->lp_skip_tot,
				rlpt->lp_csect_single));

				rlpt->lp_buffer[rlpt->lp_length] = rt->pulses[i].interval;
				rlpt->lp_length++;
				rlpt->last_tstart = rt->pulses[i].interval;
				rlpt->last_skipped_time = rt->pulses[i].interval;

				rlpt->lp_cnt++;

			if (rlpt->lp_csect_single >= ((rparams->radar_args.t2_min >> 12) & 0xf)) {
				if (rlpt->lp_length > rparams->radar_args.npulses_lp / 2)
					rlpt->lp_length -= rparams->radar_args.npulses_lp / 2;
				else {
					rlpt->lp_length = 0;
					rlpt->min_detected_fc_bin5 = MAX_FC;
					rlpt->max_detected_fc_bin5 = MIN_FC;
					rlpt->avg_detected_fc_bin5 = 0;
					rlpt->subband_result = 0;
				}
			}
		}
	}

	if (rlpt->lp_length > LP_BUFFER_SIZE)
		PHY_ERROR(("WARNING: LP buffer size is too long\n"));

#ifdef RADAR_DBG
	PHY_RADAR(("\n FCC-5 \n"));
	for (i = 0; i < rlpt->lp_length; i++) {
		PHY_RADAR(("%u  ", rlpt->lp_buffer[i]));
	}
	PHY_RADAR(("\n"));
#endif // endif
	if (rlpt->lp_length >= rparams->radar_args.npulses_lp) {
		/* reject detection spaced more than 3 minutes */
		deltat = (uint32) (pi->sh->now - rlpt->last_detection_time_lp);
		PHY_RADAR(("last_detection_time_lp=%u, watchdog_timer=%u, deltat=%d,"
			" deltat_min=%d, deltat_sec=%d\n",
			rlpt->last_detection_time_lp, pi->sh->now, deltat, deltat/60, deltat%60));
		rlpt->last_detection_time_lp = pi->sh->now;
		if ((uint32) deltat < (rparams->radar_args.fra_pulse_err & 0xff)*60 ||
		    (*first_radar_indicator == 1 && (uint32) deltat < 30 * 60)) {
			if (rlpt->lp_csect_single <= rparams->radar_args.npulses_lp - 2 &&
			    rlpt->lp_skip_tot < ((rparams->radar_args.max_span_lp >> 12) & 0xf)) {
				PHY_RADAR(("FCC-5 Radar Detection. Time from last detection"
					" = %u, = %dmin %dsec\n",
					deltat, deltat / 60, deltat % 60));

				radar_detected->radar_type = RADAR_TYPE_FCC_5;
				rt_status->detected = TRUE;
				rt_status->count =
					rt_status->count + 1;
				rt_status->pretended = FALSE;
				rt_status->radartype = radar_detected->radar_type;
				rt_status->timenow = (uint32) (pi->sh->now);
				rt_status->timefromL = deltat;
				rt_status->ch = pi->radio_chanspec;
				rt_status->lp_csect_single = rlpt->lp_csect_single;

				//check fc variation
				if ((rlpt->max_detected_fc_bin5 -
				    (rlpt->avg_detected_fc_bin5+2)/4) >=
				    ((rlpt->avg_detected_fc_bin5+2)/4 -
				    rlpt->min_detected_fc_bin5)) {
					var_fc_bin5 = ((rlpt->avg_detected_fc_bin5+2)/4
							- rlpt->min_detected_fc_bin5);
				} else {
					var_fc_bin5 = (rlpt->max_detected_fc_bin5
							- (rlpt->avg_detected_fc_bin5+2)/4);
				}
				if (var_fc_bin5 > st->rparams.radar_thrs2.fc_varth_bin5_sb) {
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						radar_detected->subband = ALLSUBBAND_BW40;
					} else if (CHSPEC_IS80(pi->radio_chanspec) ||
						PHY_AS_80P80(pi, pi->radio_chanspec)) {
						radar_detected->subband = ALLSUBBAND_BW80;
					}
				} else {
					//subband detection
					radar_detected->subband =
						wlc_phy_radar_fc_chirp_2_subband(pi,
						(rlpt->avg_detected_fc_bin5+2)/4, FCC5_CHIRP, 1);
				}
				rlpt->subband_result = radar_detected->subband;
			}
		}
#ifdef BCMDBG
		else {
			if (rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_REJECTED_RADAR) {
				PHY_RADAR(("SKIPPED false FCC-5 Radar Detection."
					" Time from last detection = %u, = %dmin %dsec,"
					" ncsect_single=%d\n",
					deltat, deltat / 60, deltat % 60, rlpt->lp_csect_single));
			}
		}
#endif /* BCMDBG */
		if (rlpt->lp_len_his_idx < LP_LEN_HIS_SIZE) {
			rlpt->lp_len_his[rlpt->lp_len_his_idx] = rlpt->lp_length;
			rlpt->lp_len_his_idx++;
		}
		rlpt->lp_length = 0;
		rlpt->lp_pw_fm_matched = 0;
		rlpt->lp_n_non_single_pulses = 0;
		rlpt->lp_skip_tot = 0;
		rlpt->lp_csect_single = 0;
		*first_radar_indicator = 0;
		rlpt->avg_detected_fc_bin5 = 0;
	}
}

/* [PHY_REARCH] Need to rename this function. It is used in AC, N and HT */
uint8
phy_radar_run_nphy(phy_info_t *pi, radar_detected_info_t *radar_detected, bool sec_pll,
bool bw80_80_mode)
{
	phy_radar_info_t *ri = pi->radari;
	phy_radar_st_t *st = phy_radar_get_st(ri);
	wl_radar_status_t *rt_status;
	bool *first_radar_indicator;
	bool *dfs_lp_buffer_nphy;
/* NOTE: */
/* PLEASE UPDATE THE DFS_SW_VERSION #DEFINE'S IN FILE WLC_PHY_INT.H */
/* EACH TIME ANY DFS FUNCTION IS MODIFIED EXCEPT RADAR THRESHOLD CHANGES */
	uint16 i;
	uint16 j;
	int k;
	int wr_ptr;
	uint8 ant;
	uint16 mlength;
	int32 deltat;
	radar_work_t *rt;
	radar_lp_info_t *rlpt;
	radar_params_t *rparams = &st->rparams;
	wl_radar_thr2_t *radar_thrs2 = &rparams->radar_thrs2;
	uint8 det_type = RADAR_TYPE_NONE;
	int min_detected_pw;
	int max_detected_pw;
	uint32 min_detected_interval;
	uint32 max_detected_interval;
	int nconsecq_pulses = 0;
	int detected_pulse_index = 0;
	int32 deltat2 = 0;
	int min_fm_lp_sc = 0;
	int16 ant_num = 1;
	int var_fc = 0;
	int avg_fc = 0;
	uint16 blank_time = 0;
	int max_detected_chirp = 0;
	int fm_thrs = -50;
	int fm_chk_pw_thrs1 = fm_chk_pw[0];
	int fm_chk_pw_thrs2 = fm_chk_pw[1];

	/* 4366 enlarge ofdm_nominal_clip_th when in DFS band to avoid radar triggers preemption */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
	  if (sec_pll == FALSE) {
		phy_utils_write_phyreg(pi, ACPHY_PREMPT_ofdm_nominal_clip_th0(pi->pubpi->phy_rev),
				0x8000);
		phy_utils_write_phyreg(pi, ACPHY_PREMPT_ofdm_nominal_clip_th1(pi->pubpi->phy_rev),
				0x8000);
		phy_utils_write_phyreg(pi, ACPHY_PREMPT_ofdm_nominal_clip_th2(pi->pubpi->phy_rev),
				0x8000);
		phy_utils_write_phyreg(pi, ACPHY_PREMPT_ofdm_nominal_clip_th3(pi->pubpi->phy_rev),
				0x8000);
	  }
	}

	if (sec_pll == FALSE || !(PHY_SUPPORT_SCANCORE(pi) || PHY_SUPPORT_BW80P80(pi))) {
		rt_status = &st->radar_status;
		rlpt = &st->radar_lp_info;
		first_radar_indicator = &st->first_radar_indicator;
		dfs_lp_buffer_nphy = &pi->dfs_lp_buffer_nphy;
		if (bw80_80_mode == FALSE) {
			ant_num = GET_RDR_NANTENNAS(pi);
		} else {
			ant_num = 1;
		}
		blank_time = phy_utils_read_phyreg(pi,
			ACPHY_RadarBlankCtrl(pi->pubpi->phy_rev));
	} else {
		rlpt = st->radar_work_lp_sc;
		if (bw80_80_mode == FALSE) {
			rt_status = st->radar_status_sc;
		} else {
			//share the same radar status under BW80+80
			rt_status = &st->radar_status;
		}
		first_radar_indicator = &st->first_radar_indicator_sc;
		dfs_lp_buffer_nphy = &pi->dfs_lp_buffer_nphy_sc;
		if (((st->rparams.radar_thrs2.highpow_war_enb >> 2) & 0x3) == 0) {
			min_fm_lp_sc = rparams->radar_args.min_fm_lp >> 1;
		} else if (((st->rparams.radar_thrs2.highpow_war_enb >> 2) & 0x3) == 1) {
			min_fm_lp_sc = (rparams->radar_args.min_fm_lp * 3) >> 2;
		} else if (((st->rparams.radar_thrs2.highpow_war_enb >> 2) & 0x3) == 2) {
			min_fm_lp_sc = (rparams->radar_args.min_fm_lp * 7) >> 3;
		} else {
			min_fm_lp_sc = rparams->radar_args.min_fm_lp;
		}
		ant_num = 1;
		if (bw80_80_mode == FALSE) {
			blank_time = phy_utils_read_phyreg(pi,
				ACPHY_RadarBlankCtrl_SC(pi->pubpi->phy_rev));
		} else {
			blank_time = phy_utils_read_phyreg(pi,
				ACPHY_RadarBlankCtrl(pi->pubpi->phy_rev));
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_47(pi->pubpi->phy_rev)) {
		blank_time = (blank_time & 0xff);
		if (CHSPEC_IS40(pi->radio_chanspec)) {
			blank_time = blank_time/2;
		} else if (CHSPEC_IS80(pi->radio_chanspec) ||
			PHY_AS_80P80(pi, pi->radio_chanspec)) {
			blank_time = blank_time/4;
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
			blank_time = blank_time/8;
		}
	} else {
		blank_time = 0;
	}

	rparams = &st->rparams;

	ASSERT(radar_detected != NULL);

	(void) memset(radar_detected, 0, sizeof(*radar_detected));

	/* clear LP buffer if requested, and print LP buffer count history */
	if (*dfs_lp_buffer_nphy != 0) {
		*dfs_lp_buffer_nphy = 0;
		*first_radar_indicator = 1;
		PHY_RADAR(("DFS LP buffer =  "));
		for (i = 0; i < rlpt->lp_len_his_idx; i++) {
			PHY_RADAR(("%d, ", rlpt->lp_len_his[i]));
		}
		PHY_RADAR(("%d; now CLEARED\n", rlpt->lp_length));
		rlpt->lp_length = 0;
		rlpt->lp_pw_fm_matched = 0;
		rlpt->lp_n_non_single_pulses = 0;
		rlpt->lp_cnt = 0;
		rlpt->lp_skip_cnt = 0;
		rlpt->lp_skip_tot = 0;
		rlpt->lp_csect_single = 0;
		rlpt->lp_len_his_idx = 0;
		rlpt->min_detected_fc_bin5 = MAX_FC;
		rlpt->max_detected_fc_bin5 = MIN_FC;
		rlpt->avg_detected_fc_bin5 = 0;
		rlpt->last_detection_time = pi->sh->now;
		rlpt->last_detection_time_lp = pi->sh->now;
		rlpt->subband_result = 0;
	}
	if (!rparams->radar_args.npulses) {
		PHY_ERROR(("%s: radar params not initialized\n", __FUNCTION__));
		return RADAR_TYPE_NONE;
	}

	min_detected_pw = rparams->radar_args.max_pw;
	max_detected_pw = rparams->radar_args.min_pw;

	/* current radar fifo sample info */
	rt = phy_cache_acquire_reuse_buffer(pi->cachei, sizeof(radar_work_t));

	/* suspend mac before reading phyregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	wlc_phy_radar_read_table(pi, st, rt, sec_pll, bw80_80_mode);

	/* restart mac after reading phyregs */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	/* skip radar detect if doing periodic cal
	 * (the test-tones utilized during cal can trigger
	 * radar detect)
	 * NEED TO BE HERE AFTER READING DATA FROM (CLEAR) THE FIFO
	 */
	if (ISNPHY(pi)) {
	  if (pi->u.pi_nphy->nphy_rxcal_active) {
	    pi->u.pi_nphy->nphy_rxcal_active = FALSE;
	    PHY_RADAR(("DOING RXCAL, SKIP RADARS\n"));
	    goto radar_end;
	  }
	}
	if (ISACPHY(pi)) {
	  if (pi->u.pi_acphy->radar_cal_active) {
	    pi->u.pi_acphy->radar_cal_active = FALSE;
	    PHY_RADAR(("DOING CAL, SKIP RADARS\n"));
	    goto radar_end;
	  }
	}

	/*
	 * Reject if no pulses recorded
	 */
	if (ant_num == 2) {
		if ((rt->length_input[0] < 1) && (rt->length_input[1] < 1)) {
			goto radar_end;
		}
	} else {
		if (rt->length_input[0] < 1) {
			goto radar_end;
		}
	}

#ifdef BCMDBG
	if (rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_PULSE_DATA) {
		for (ant = 0; ant < ant_num; ant++) {
			PHY_RADAR(("\nPulses read from PHY FIFO"));
			PHY_RADAR(("\nAnt %d: %d pulses, ", ant, rt->length_input[ant]));

			phy_radar_output_pulses(pi, rt->pulses_input[ant], rt->length_input[ant],
				rparams->radar_args.feature_mask);
		}
	}
#endif /* BCMDBG */

	/* START LONG PULSES (BIN5) DETECTION */
	if (rparams->radar_args.feature_mask & RADAR_FEATURE_FCC_DETECT) {	/* if fcc */
		phy_radar_detect_fcc5(pi, st, rt, sec_pll, bw80_80_mode, radar_detected);

		if (radar_detected->radar_type == RADAR_TYPE_FCC_5) {
			det_type = RADAR_TYPE_FCC_5;
			goto radar_end;
		}
	}

	/* START SHORT PULSES (NON-BIN5) DETECTION */
	/* remove "noise" pulses with  pw > 400 and fm < 244 */
	if (ISACPHY(pi) && TONEDETECTION) {
	for (ant = 0; ant < ant_num; ant++) {
		wr_ptr = 0;
		mlength = rt->length_input[ant];
		for (i = 0; i < rt->length_input[ant]; i++) {
		  if (rt->pulses_input[ant][i].pw  < (rparams->radar_args.st_level_time & 0x0fff) ||
		      rt->pulses_input[ant][i].fm >
		      ((sec_pll == 0 && bw80_80_mode == 0) ?
		      rparams->radar_args.min_fm_lp : min_fm_lp_sc)) {
				rt->pulses_input[ant][wr_ptr] = rt->pulses_input[ant][i];
				++wr_ptr;
			} else {
				mlength--;
			}
		}	/* for mlength loop */
		rt->length_input[ant] = mlength;
	}	/* for ant loop */
	}
	/* Combine pulses that are adjacent */
	for (ant = 0; ant < ant_num; ant++) {
		for (k = 0; k < 2; k++) {
			mlength = rt->length_input[ant];
			if (mlength > 1) {
			for (i = 1; i < mlength; i++) {
				deltat = ABS((int32)(rt->pulses_input[ant][i].interval -
					rt->pulses_input[ant][i-1].interval));
				if ((deltat < (int32)rparams->radar_args.min_deltat && FALSE) ||
				    (deltat <= (int32)rparams->radar_args.max_pw && TRUE))
				{
					/* Combine fc, take pw as weighting */
					if (ISACPHY(pi) && TONEDETECTION &&
					    PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
						wlc_phy_radar_fc_chirp_cmb(
							&rt->pulses_input[ant][i-1],
							&rt->pulses_input[ant][i]);
					}

					if (ISNPHY(pi) || ISACPHY(pi)) {
						if (((uint32)(rt->pulses_input[ant][i].interval -
							rt->pulses_input[ant][i-1].interval))
							<= (uint32) rparams->radar_args.max_pw) {
							rt->pulses_input[ant][i-1].pw =
							ABS((int32)(
							rt->pulses_input[ant][i].interval -
							rt->pulses_input[ant][i-1].interval)) +
							rt->pulses_input[ant][i].pw;
						}
					} else {
#ifdef BCMDBG
						/* print pulse combining debug messages */
						PHY_RADAR(("*%d,%d,%d ",
							rt->pulses_input[ant][i].interval -
							rt->pulses_input[ant][i-1].interval,
							rt->pulses_input[ant][i].pw,
							rt->pulses_input[ant][i-1].pw));
#endif // endif
						if (rparams->radar_args.feature_mask &
								RADAR_FEATURE_USE_MAX_PW) {
							rt->pulses_input[ant][i-1].pw =
								(rt->pulses_input[ant][i-1].pw >
								rt->pulses_input[ant][i].pw ?
								rt->pulses_input[ant][i-1].pw :
								rt->pulses_input[ant][i].pw);
						} else {
							rt->pulses_input[ant][i-1].pw =
								rt->pulses_input[ant][i-1].pw +
								rt->pulses_input[ant][i].pw;
						}
					}

					/* Combine fm */
					if (ISACPHY(pi) && TONEDETECTION) {
						rt->pulses_input[ant][i-1].fm =
							(rt->pulses_input[ant][i-1].fm >
							rt->pulses_input[ant][i].fm) ?
							rt->pulses_input[ant][i-1].fm :
							rt->pulses_input[ant][i].fm;
					} else {
						rt->pulses_input[ant][i-1].fm =
							rt->pulses_input[ant][i-1].fm +
							rt->pulses_input[ant][i].fm;
					}

					for (j = i; j < mlength - 1; j++) {
						rt->pulses_input[ant][j] =
							rt->pulses_input[ant][j+1];
					}
					mlength--;
					rt->length_input[ant]--;
				}
			} /* for i < mlength */
			} /* mlength > 1 */
		}
	}
#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_SHORT_PULSE) != 0 &&
	    rt->length_input[0] > 0) {	/* short pulses */
		for (ant = 0; ant < ant_num; ant++) {
			PHY_RADAR(("\nShort Pulse After combining adjacent pulses"));
			PHY_RADAR(("\nAnt %d: %d pulses, ", ant, rt->length_input[ant]));

			phy_radar_output_pulses(pi, rt->pulses_input[ant], rt->length_input[ant],
			                        rparams->radar_args.feature_mask);
		}
	}
#endif /* BCMDBG */

	for (ant = 0; ant < ant_num; ant++) {

	bzero(rt->pulses, sizeof(rt->pulses));
	rt->length = 0;

	/* Use data from one of the antenna */
	for (i = 0; i < rt->length_input[ant]; i++) {
		rt->pulses[rt->length] = rt->pulses_input[ant][i];
		rt->length++;
	}

#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_SHORT_PULSE) != 0 &&
	    rt->length > 0) {	/* short pulses */
		PHY_RADAR(("\nShort pulses after use data from one of two antennas"));
		PHY_RADAR(("\n%d pulses, ", rt->length));

		phy_radar_output_pulses(pi, rt->pulses, rt->length,
			rparams->radar_args.feature_mask);
	}
#endif /* BCMDBG */

	/* remove pulses spaced less than lower byte of "blank" */
	for (i = 1; i < rt->length; i++) {
		deltat = (int32)(rt->pulses[i].interval - rt->pulses[i-1].interval);
		if (deltat < (ISACPHY(pi) ? ((int32)rparams->radar_args.blank & 0xff) : 20)) {
			for (j = i; j < (rt->length - 1); j++) {
				rt->pulses[j] = rt->pulses[j + 1];
			}
			rt->length--;
		}
	}

	if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
		/* remove entries chirp greater than min_chirp */
		j = 0;
		for (i = 0; i < rt->length; i++) {
			if (rt->pulses[i].chirp <= ETSI4_CHIRP) {
				rt->pulses[j] = rt->pulses[i];
				j++;
			}
		}
		rt->length = j;
	}

#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_SHORT_PULSE) != 0 &&
	    rt->length > 0) {	/* short pulses */
		PHY_RADAR(("\nShort pulses after removing pulses with interval < %d",
		          rparams->radar_args.blank & 0xff));
		PHY_RADAR(("\n%d pulses, ", rt->length));

		phy_radar_output_pulses(pi, rt->pulses, rt->length,
			rparams->radar_args.feature_mask);
	}
#endif /* BCMDBG */

	/*
	 * filter based on pulse width
	 */
	j = 0;
	for (i = 0; i < rt->length; i++) {
		if ((rt->pulses[i].pw >= rparams->radar_args.min_pw) &&
		    (rt->pulses[i].pw <= rparams->radar_args.max_pw)) {
			rt->pulses[j] = rt->pulses[i];
			j++;
		}
	}
	rt->length = j;

	if (ISACPHY(pi) && TONEDETECTION) {
		j = 0;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			fm_thrs = 0;
		} else {
			fm_thrs = -50;
		}
		for (i = 0; i < rt->length; i++) {
			if (radar_thrs2->fm_chk_opt & EXT_FM_ABS_CHK_SP) {
				if (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG) {
					fm_chk_pw_thrs1 = (radar_thrs2->fm_chk_pw >> 8) & 0xff;
					fm_chk_pw_thrs2 = radar_thrs2->fm_chk_pw & 0xff;
				}
				if (sec_pll == TRUE) {
					/* set fm_thrs = -5 for A1 to pass p1c absolute fm check */
					fm_thrs = (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG)
						? (int16)radar_thrs2->fm_thresh_p1c : -5;
				} else if (rt->pulses[i].pw < fm_chk_pw_thrs1) {
					fm_thrs = (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG)
						? radar_thrs2->fm_thresh_sp1 :
						CHSPEC_IS20(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[0] : fm_thrs_sp[0]) :
						CHSPEC_IS40(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[3] : fm_thrs_sp[3]) :
						CHSPEC_IS80(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[6] : fm_thrs_sp[6]) :
						(PHY_LESI_ON(pi) ? fm_thrs_sp_lesi[9] :
						fm_thrs_sp[9]);
				} else if (rt->pulses[i].pw >= fm_chk_pw_thrs1 &&
					rt->pulses[i].pw < fm_chk_pw_thrs2) {
					fm_thrs = (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG)
						? radar_thrs2->fm_thresh_sp2 :
						CHSPEC_IS20(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[1] : fm_thrs_sp[1]) :
						CHSPEC_IS40(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[4] : fm_thrs_sp[4]) :
						CHSPEC_IS80(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[7] : fm_thrs_sp[7]) :
						(PHY_LESI_ON(pi) ? fm_thrs_sp_lesi[10] :
						fm_thrs_sp[10]);
				} else if (rt->pulses[i].pw >= fm_chk_pw_thrs2) {
					fm_thrs = (radar_thrs2->fm_chk_opt & EXT_FM_CHK_DBG)
						? radar_thrs2->fm_thresh_sp3 :
						CHSPEC_IS20(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[2] : fm_thrs_sp[2]) :
						CHSPEC_IS40(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[5] : fm_thrs_sp[5]) :
						CHSPEC_IS80(pi->radio_chanspec) ? (PHY_LESI_ON(pi) ?
						fm_thrs_sp_lesi[8] : fm_thrs_sp[8]) :
						(PHY_LESI_ON(pi) ? fm_thrs_sp_lesi[11] :
						fm_thrs_sp[11]);
				}
				if (rt->pulses[i].fm >= fm_thrs) {
					rt->pulses[j] = rt->pulses[i];
					j++;
				}
#ifdef BCMDBG
				else {
					if ((rparams->radar_args.feature_mask &
						RADAR_FEATURE_DEBUG_SHORT_PULSE) != 0) {
						PHY_RADAR(("\nShort pulses fail absolute"
						" fm value check"));
					}
				}
#endif /* BCMDBG */
			} else {
				if (rt->pulses[i].fm >= fm_thrs) {
					rt->pulses[j] = rt->pulses[i];
					j++;
				}
			}
		}
		rt->length = j;
	}

#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_SHORT_PULSE) != 0) {
		PHY_RADAR(("\nShort pulses after removing pulses with pw outside [%d, %d]",
		           rparams->radar_args.min_pw, rparams->radar_args.max_pw));
		PHY_RADAR((" or fm < %d", fm_thrs));
		PHY_RADAR(("\n%d pulses, ", rt->length));

		phy_radar_output_pulses(pi, rt->pulses, rt->length,
			rparams->radar_args.feature_mask);
	}
#endif /* BCMDBG */

	if (ISACPHY(pi) && TONEDETECTION &&
	    PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
		if (sec_pll == FALSE && bw80_80_mode == FALSE &&
			((st->rparams.radar_thrs2.notradar_enb >> 2) & 0x1)) {
			j = 0;
			for (i = 0; i < rt->length; i++) {
				if ((rt->pulses[i].notradar <=
					st->rparams.radar_thrs2.max_notradar)) {
					rt->pulses[j] = rt->pulses[i];
					j++;
				}
			}
			rt->length = j;
		} else {
			j = 0;
			for (i = 0; i < rt->length; i++) {
				if ((rt->pulses[i].notradar <=
					st->rparams.radar_thrs2.max_notradar_sc)) {
					rt->pulses[j] = rt->pulses[i];
					j++;
				}
			}
			rt->length = j;
		}
	}

	if (rt->length == 0)
		continue;

/*
	ASSERT(rt->length <= RDR_LIST_SIZE);
*/
	if (rt->length > RDR_LIST_SIZE) {
		rt->length = RDR_LIST_SIZE;
		PHY_RADAR(("WARNING: radar rt->length=%d > list_size=%d\n",
			rt->length, RDR_LIST_SIZE));
	}

	det_type = RADAR_TYPE_NONE;

	/* convert pulses[].interval from tstamps to intervals */
	--rt->length;
	for (j = 0; j < rt->length; j++) {
		rt->pulses[j].interval = ABS((int32)(rt->pulses[j + 1].interval -
		                                     rt->pulses[j].interval));
	}

	wlc_phy_radar_detect_run_epoch(pi, rt, rparams, rt->length,
		&det_type, &nconsecq_pulses, &detected_pulse_index,
		&min_detected_pw, &max_detected_pw,
		&min_detected_interval, &max_detected_interval,
		&avg_fc, &var_fc, blank_time, sec_pll);

#ifdef BCMDBG
	if ((rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_INTV_PW) != 0) {
		PHY_RADAR(("\nShort pulses after pruning (filtering)"));

		phy_radar_output_pulses(pi, rt->pulses, rt->length,
			rparams->radar_args.feature_mask);

		PHY_RADAR(("nconsecq_pulses=%d max_pw_delta=%d min_pw=%d max_pw=%d\n",
			nconsecq_pulses, max_detected_pw - min_detected_pw, min_detected_pw,
			max_detected_pw));
	}
#endif /* BCMDBG */

	if (min_detected_interval != 0 || det_type != RADAR_TYPE_NONE) {
#ifdef BCMDBG
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_47(pi->pubpi->phy_rev)) {
			PHY_RADAR(("\nPruned Intv: "));
			for (i = 0; i < rt->length; i++) {
				PHY_RADAR(("%d-%d ", rt->pulses[i].interval, i));
			}
			PHY_RADAR(("\n"));

			PHY_RADAR(("Pruned PW:  "));
			for (i = 0; i <  rt->length; i++) {
				PHY_RADAR(("%i-%d ", rt->pulses[i].pw, i));
			}
			PHY_RADAR(("\n"));

			PHY_RADAR(("Pruned FM:  "));
			for (i = 0; i <  rt->length; i++) {
				PHY_RADAR(("%i-%d ", rt->pulses[i].fm, i));
			}

			PHY_RADAR(("\n"));

			PHY_RADAR(("Pruned Notradar:  "));
			for (i = 0; i <  rt->length; i++) {
				PHY_RADAR(("%i-%d ", rt->pulses[i].notradar, i));
			}
			PHY_RADAR(("\n"));

			PHY_RADAR(("Pruned Fc:  "));
			for (i = 0; i <  rt->length; i++) {
				PHY_RADAR(("%i-%d ", rt->pulses[i].fc, i));
			}
			PHY_RADAR(("\n"));

			PHY_RADAR(("Pruned Chirp:  "));
			for (i = 0; i <  rt->length; i++) {
				PHY_RADAR(("%i-%d ", rt->pulses[i].chirp, i));
			}

			PHY_RADAR(("\n"));
		} else {
			PHY_RADAR(("\nPruned %d pulses, ", rt->length));

			PHY_RADAR(("\n(Interval, PW, FM) = [ "));
			for (i = 0; i < rt->length; i++) {
				PHY_RADAR(("(%u, %u, %d) ",
					rt->pulses[i].interval, rt->pulses[i].pw,
					rt->pulses[i].fm));
			}
			PHY_RADAR(("];\n"));
		}
		PHY_RADAR(("det_idx=%d pw_delta=%d min_pw=%d max_pw=%d\n",
				detected_pulse_index,
				max_detected_pw - min_detected_pw, min_detected_pw,
				max_detected_pw));
#endif /* BCMDBG */

		bzero(rt_status->intv, sizeof(rt_status->intv));
		bzero(rt_status->pw, sizeof(rt_status->pw));
		bzero(rt_status->fm, sizeof(rt_status->fm));
#ifdef BCMDBG
		bzero(rt_status->fc, sizeof(rt_status->fc));
		bzero(rt_status->chirp, sizeof(rt_status->chirp));
#endif // endif
		for (i = 0; i < rt->length; i++) {
			if (i >= detected_pulse_index && i < detected_pulse_index + 10) {
				rt_status->intv[i - detected_pulse_index] =
					rt->pulses[i].interval;
				rt_status->pw[i - detected_pulse_index] = rt->pulses[i].pw;
				rt_status->fm[i - detected_pulse_index] = rt->pulses[i].fm;
#ifdef BCMDBG
				rt_status->fc[i - detected_pulse_index] = rt->pulses[i].fc;
				rt_status->chirp[i - detected_pulse_index] = rt->pulses[i].chirp;
#endif // endif
			}
		}

		deltat2 = (uint32) (pi->sh->now - rlpt->last_detection_time);
		/* detection not valid if detected pulse index too large */
		if (detected_pulse_index < ((rparams->radar_args.ncontig) & 0x3f)) {
			rlpt->last_detection_time = pi->sh->now;
		}
		/* reject detection spaced more than 3 minutes and detected pulse index too larg */
		if (((uint32) deltat2 < (rparams->radar_args.fra_pulse_err & 0xff)*60 ||
			(*first_radar_indicator == 1 && (uint32) deltat2 < 30*60)) &&
			(detected_pulse_index < ((rparams->radar_args.ncontig) & 0x3f))) {
			if (PHY_RADAR_FIFO_SUBBAND_FORMAT(pi)) {
				if (det_type == RADAR_TYPE_ETSI_4) {
					//assign chirp value to 5MHz in EU Type4 radar
					max_detected_chirp = ETSI4_CHIRP;
				} else {
					max_detected_chirp = 0;
				}
			}

			//check fc variation
			if (var_fc > st->rparams.radar_thrs2.fc_varth_sb ||
				det_type == RADAR_TYPE_JP4 ||
				det_type == RADAR_TYPE_FCC_6) {
				if (CHSPEC_IS20(pi->radio_chanspec)) {
					radar_detected->subband = ALLSUBBAND_BW20;
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					radar_detected->subband = ALLSUBBAND_BW40;
				} else if (CHSPEC_IS80(pi->radio_chanspec) ||
					PHY_AS_80P80(pi, pi->radio_chanspec)) {
					radar_detected->subband = ALLSUBBAND_BW80;
				}
			} else {
				radar_detected->subband =
					wlc_phy_radar_fc_chirp_2_subband(pi,
						avg_fc, max_detected_chirp, 0);
			}
			rlpt->subband_result = radar_detected->subband;

			PHY_RADAR(("Type %d Radar Detection. Detected pulse index=%d"
				" nconsecq_pulses=%d."
				" Time from last detection = %u, = %dmin %dsec \n",
				det_type, detected_pulse_index, nconsecq_pulses,
				deltat2, deltat2/60, deltat2%60));
			rt_status->detected = TRUE;
			rt_status->count = rt_status->count + 1;
			rt_status->pretended = FALSE;
			rt_status->radartype = det_type;
			rt_status->timenow = (uint32) pi->sh->now;
			rt_status->timefromL = (uint32) deltat2;
			rt_status->detected_pulse_index =  detected_pulse_index;
			rt_status->nconsecq_pulses = nconsecq_pulses;
			rt_status->ch = pi->radio_chanspec;
			*first_radar_indicator = 0;

			radar_detected->radar_type = det_type;
			radar_detected->min_pw = (uint16)min_detected_pw;	/* in 20 * usec */
			radar_detected->max_pw = (uint16)max_detected_pw;	/* in 20 * usec */
			radar_detected->min_pri =
				(uint16)((min_detected_interval + 10) / 20);	/* in usec */
			radar_detected->max_pri =
				(uint16)((max_detected_interval + 10) / 20);	/* in usec */
		} else {
			if (rparams->radar_args.feature_mask & RADAR_FEATURE_DEBUG_REJECTED_RADAR) {
				PHY_RADAR(("SKIPPED false Type %d Radar Detection."
					" min_pw=%d pw_delta=%d pri=%u"
					" nconsecq_pulses=%d. Time from last"
					" detection = %u, = %dmin %dsec",
					det_type, min_detected_pw,
					max_detected_pw - max_detected_pw,
					min_detected_interval, nconsecq_pulses, deltat2,
					deltat2 / 60, deltat2 % 60));
				if (detected_pulse_index < ((rparams->radar_args.ncontig) & 0x3f) -
					rparams->radar_args.npulses)
					PHY_RADAR((". Detected pulse index: %d\n",
						detected_pulse_index));
				else
					PHY_ERROR((". Detected pulse index too high: %d\n",
						detected_pulse_index));
			}
		}
		break;
	}
	}	/* end for (ant = 0; ant < GET_RDR_NANTENNAS(pi); ant++) */

radar_end:
	phy_cache_release_reuse_buffer(pi->cachei);

	return det_type;
}
