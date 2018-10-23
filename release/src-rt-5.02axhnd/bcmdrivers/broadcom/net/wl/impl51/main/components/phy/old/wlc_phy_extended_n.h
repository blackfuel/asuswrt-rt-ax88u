/*
 * NPHY specific defines and declarations
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
 * $Id: wlc_phy_extended_n.h 672101 2016-11-24 06:06:10Z $
 */

#ifndef _wlc_phy_extended_n_h_
#define _wlc_phy_extended_n_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>

/* ------------------- */
/*  MACRO definitions  */
/* ------------------- */

#define NPHY_INTF_RSSI_ENAB(pi)	(CHIPID_43236X_FAMILY(pi))

#define MOD_PHYREG3(pi, phy_type, reg, field, value)	\
	phy_utils_mod_phyreg(pi, phy_type##_##reg, \
	phy_type##_##reg##_##field##_##MASK, (value) << phy_type##_##reg##_##field##_##SHIFT);

#define READ_PHYREG3(pi, phy_type, reg, field)	\
	((phy_utils_read_phyreg(pi, phy_type##_##reg) \
	& phy_type##_##reg##_##field##_##MASK) >> phy_type##_##reg##_##field##_##SHIFT)

#define WRITE_PHY_REG3(pi, phy_type, reg, value)	\
	phy_utils_write_phyreg(pi, phy_type##_##reg, value);

#define	READ_RADIO_REG2(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, radio_type##_##jspace##_##reg_name | \
	((core == PHY_CORE_0) ? radio_type##_##jspace##0 : radio_type##_##jspace##1))
#define	WRITE_RADIO_REG2(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, radio_type##_##jspace##_##reg_name | \
	((core == PHY_CORE_0) ? radio_type##_##jspace##0 : radio_type##_##jspace##1), value);
#define	WRITE_RADIO_SYN(pi, radio_type, reg_name, value) \
	phy_utils_write_radioreg(pi, radio_type##_##SYN##_##reg_name, value);

#define	READ_RADIO_REG3(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##jspace##0##_##reg_name : \
	radio_type##_##jspace##1##_##reg_name));
#define	WRITE_RADIO_REG3(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##jspace##0##_##reg_name : \
	radio_type##_##jspace##1##_##reg_name), value);
#define	READ_RADIO_REG4(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##reg_name##_##jspace##0 : \
	radio_type##_##reg_name##_##jspace##1));
#define	WRITE_RADIO_REG4(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##reg_name##_##jspace##0 : \
	radio_type##_##reg_name##_##jspace##1), value);

#define wlc_phy_set_target_tx_pwr_nphy(pi, target) \
	phy_utils_write_phyreg(pi, \
		      NPHY_TxPwrCtrlTargetPwr,				\
		      ((uint16)MAX(pi->u.pi_nphy->tssi_minpwr_limit,	\
				   (MIN(pi->u.pi_nphy->tssi_maxpwr_limit, (uint16)(target)))) \
		       << NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT) |	\
		      ((uint16)MAX(pi->u.pi_nphy->tssi_minpwr_limit,	\
				   (MIN(pi->u.pi_nphy->tssi_maxpwr_limit, (uint16)(target)))) \
		       << NPHY_TxPwrCtrlTargetPwr_targetPwr1_SHIFT))

#define wlc_phy_get_target_tx_pwr_nphy(pi) \
	((phy_utils_read_phyreg(pi, NPHY_TxPwrCtrlTargetPwr) & \
		NPHY_TxPwrCtrlTargetPwr_targetPwr0_MASK) >> \
		NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

#define NPHY_ACI_MAX_UNDETECT_WINDOW_SZ 8 /* max window of aci detect */
#define NPHY_ACI_CHANNEL_DELTA 5   /* How far a signal can bleed */
#define NPHY_ACI_CHANNEL_SKIP 4	   /* Num of immediately surrounding channels to skip */
#define NPHY_ACI_40MHZ_CHANNEL_DELTA 6
#define NPHY_ACI_40MHZ_CHANNEL_SKIP 5
#define NPHY_ACI_40MHZ_CHANNEL_DELTA_GE_REV3 6
#define NPHY_ACI_40MHZ_CHANNEL_SKIP_GE_REV3 5
#define NPHY_ACI_CHANNEL_DELTA_GE_REV3 4   /* How far a signal can bleed */
#define NPHY_ACI_CHANNEL_SKIP_GE_REV3 3	   /* Num of immediately surrounding channels to skip */

/* noise immunity raise/lowered using crsminpwr or init gain value */
/* before assoc, consec crs glitch before raising noise immunity */
#define NPHY_NOISE_NOASSOC_GLITCH_TH_UP 2

/* before assoc, consec crs glitch before lowering noise immunity */
#define NPHY_NOISE_NOASSOC_GLITCH_TH_DN 2

/* after assoc, no aci, consec crs glitch before raising noise immunity */
#define NPHY_NOISE_ASSOC_GLITCH_TH_UP 2

/* after assoc, no aci, consec crs glitch before lowering noise immunity */
#define NPHY_NOISE_ASSOC_GLITCH_TH_DN 2

/* after assoc, aci on, consec crs glitch before raising noise immunity */
#define NPHY_NOISE_ASSOC_ACI_GLITCH_TH_UP 2

/* after assoc, aci on, consec crs glitch before lowering noise immunity */
#define NPHY_NOISE_ASSOC_ACI_GLITCH_TH_DN 2

/* not associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (only ofdm) */
#define NPHY_NOISE_NOASSOC_ENTER_TH  400
#define NPHY_NOISE_NOASSOC_ENTER_TH_REV7  500

/* associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (only ofdm) */
#define NPHY_NOISE_ASSOC_ENTER_TH  400
#define NPHY_NOISE_ASSOC_ENTER_TH_REV7  500

/* associated, threshold for noise ma to raise inband immunity */
/* compared against rx crs glitches and bad plcps (for both ofdm and bphy) */
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH  400
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH_REV7  400
#define NPHY_NOISE_ASSOC_RX_GLITCH_BADPLCP_ENTER_TH_ELNA 100

#define W3_NB_THRESH 20
#define W3_NB_CNT_THRESH 1

/* wl interference 4 array for crs min pwr index  */
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX 44
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_7 120
#define NPHY_NOISE_CRSMINPWR_ARRAY_MAX_INDEX_REV_17 120

/* wl interference 4, no assoc, crsminpwr index increment */
#define NPHY_NOISE_NOASSOC_CRSIDX_INCR 16

/* wl interference 4, assoc, crsminpwr index increment */
#define NPHY_NOISE_ASSOC_CRSIDX_INCR 8
/* wl interference 4, crsminpwr index decr */
#define NPHY_NOISE_CRSIDX_DECR   1

/* BPHY desense params */
#ifdef BPHY_DESENSE
#define BPHY_DESENSE_NOISE_ENTER_TH	40 /* (nphy_th/5) */
#define BPHY_DESENSE_NOISE_CRSIDX_INCR	1
#define BPHY_DESENSE_NOISE_CRSIDX_DECR	1
#define BPHY_DESENSE_CRSMINPWR_BASELINE	0x46
#define BPHY_DESENSE_CRSMINPWR_ARRAY_MAX_INDEX 15
#endif // endif

/* rssi cal defines */
#define NPHY_RSSICAL_MAXREAD 31 /* max possible reading from 6-bit ADC */

#define NPHY_RSSICAL_NPOLL 8
#define NPHY_RSSICAL_MAXD  (1<<20)
#define NPHY_MIN_RXIQ_PWR 2
#define ENABLE_RXIQCAL_DBG 0

#define NPHY_RSSICAL_W1_TARGET 25
#define NPHY_RSSICAL_W2_TARGET NPHY_RSSICAL_W1_TARGET
#define NPHY_RSSICAL_NB_TARGET 0

#define NPHY_RSSICAL_W1_TARGET_REV3 29
#define NPHY_RSSICAL_W2_TARGET_REV3 NPHY_RSSICAL_W1_TARGET_REV3

#define NPHY_CALSANITY_RSSI_NB_MAX_POS  9
#define NPHY_CALSANITY_RSSI_NB_MAX_NEG -9
#define NPHY_CALSANITY_RSSI_W1_MAX_POS  12
#define NPHY_CALSANITY_RSSI_W1_MAX_NEG (NPHY_RSSICAL_W1_TARGET - NPHY_RSSICAL_MAXREAD)
#define NPHY_CALSANITY_RSSI_W2_MAX_POS  NPHY_CALSANITY_RSSI_W1_MAX_POS
#define NPHY_CALSANITY_RSSI_W2_MAX_NEG (NPHY_RSSICAL_W2_TARGET - NPHY_RSSICAL_MAXREAD)
#define NPHY_RSSI_SXT(x) ( (int8) ( -((x) & 0x20)  +  ((x) & 0x1f) ) ) /* sign ext 6 to 8 */
#define NPHY_RSSI_NB_VIOL(x)  (((x) > NPHY_CALSANITY_RSSI_NB_MAX_POS) || \
			       ((x) < NPHY_CALSANITY_RSSI_NB_MAX_NEG))
#define NPHY_RSSI_W1_VIOL(x)  (((x) > NPHY_CALSANITY_RSSI_W1_MAX_POS) || \
			       ((x) < NPHY_CALSANITY_RSSI_W1_MAX_NEG))
#define NPHY_RSSI_W2_VIOL(x)  (((x) > NPHY_CALSANITY_RSSI_W2_MAX_POS) || \
			       ((x) < NPHY_CALSANITY_RSSI_W2_MAX_NEG))

#define NPHY_IQCAL_NUMGAINS 9
#define NPHY_N_GCTL 0x66

#define NPHY_PAPD_EPS_TBL_SIZE 64
#define NPHY_PAPD_SCL_TBL_SIZE 64
#define	NPHY_DIG_FILT_COEFFS_CCK	2
#define	NPHY_DIG_FILT_COEFFS_OFDM22	7
#define	NPHY_DIG_FILT_COEFFS_OFDM26	11
#define	NPHY_DIG_FILT_COEFFS_OFDM5	12
#define NPHY_DIG_FILT_COEFFS_OFDM4_43217	14
#define NPHY_NUM_DIG_FILT_COEFFS 15

#define NPHY_PAPD_COMP_OFF 0
#define NPHY_PAPD_COMP_ON  1

#define NPHY_SROM_TEMPSHIFT		32
#define NPHY_SROM_MAXTEMPOFFSET		16
#define NPHY_SROM_MINTEMPOFFSET		-16

#define NPHY_CAL_MAXTEMPDELTA		64

/* Length of noisevar table */
#define NPHY_NOISEVAR_TBLLEN40 256
#define NPHY_NOISEVAR_TBLLEN20 128

/* Start and end address for noise-variance offset in NPHY_TBL_ID_NOISEVAR */
#define NPHY_RATE_BASED_NV_OFFSET_START	256
#define NPHY_RATE_BASED_NV_OFFSET_END	271

/* Draconian Power Limits for Sulley */
#define NPHY_TSSI_SET_MAX_LIMIT 1
#define NPHY_TSSI_SET_MIN_LIMIT 2
#define NPHY_TSSI_SET_MIN_MAX_LIMIT 3

/* To set eLNA gain in clip lo region */
#define NPHY_DELTGAIN_LEVEL_24 24
#define NPHY_DELTGAIN_LEVEL_21 21
#define NPHY_DELTGAIN_LEVEL_18 18
#define NPHY_DELTGAIN_LEVEL_15 15
#define NPHY_DELTGAIN_LEVEL_12 12
#define NPHY_DELTGAIN_LEVEL_9 9
#define NPHY_gainType_HI 1
#define NPHY_gainType_MD 2
#define NPHY_gainType_LO 3
#define NPHY_gainType_INIT 4

/* RXIQCAL Params */
#define NPHY_RXCAL_TONEAMP 181
#define NPHY_RXCAL_TONEFREQ_40MHz 4000
#define NPHY_RXCAL_TONEFREQ_20MHz 2000

/* spectrum shaping filter for intPA (ofdm2, ofdm40, cck) */
#define TXFILT_SHAPING_OFDM20   0
#define TXFILT_SHAPING_OFDM40   1
#define TXFILT_SHAPING_CCK      2
#define TXFILT_DEFAULT_OFDM20   3
#define TXFILT_DEFAULT_OFDM40   4

#define NPHY_IPA_RXCAL_MAXGAININDEX (6 - 1)
#define NPHY_NUM_LOWPWR_LO_COEFFS 40

/* includes crsminpwr calc offset w.r.t initgain and headroom */
#define NPHY_RSSI_TO_CRS_GAIN_OFFSET 12

/* ------------------- */
/*  fn() declarations  */
/* ------------------- */

extern bool wlc_phy_chan2freq_nphy(phy_info_t *pi, uint channel, int *f,
	chan_info_nphy_radio2057_t **t0, chan_info_nphy_radio205x_t **t1,
	chan_info_nphy_radio2057_rev5_t **t2);
extern void wlc_phy_copy_ppr_offsets_nphy(phy_info_t *pi);
extern void wlc_phy_rfctrl_override_nphy_rev19(phy_info_t *pi, uint16 field, uint16 value,
	uint8 core_mask, uint8 off, uint8 override_id);

extern void wlc_phy_noise_home_channel_nphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_rxfe_ctrl_nphy(phy_info_t *pi);
extern void wlc_phy_program_rx_initgain_rfseq_nphy(phy_info_t *pi, uint32 init_gain);
#endif	/* _wlc_phy_extended_n_h_ */
