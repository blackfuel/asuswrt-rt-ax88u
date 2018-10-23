/*
 * PHY modules reclaimable strings.
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
 * $Id: phy_rstr.h 752074 2018-03-14 19:56:20Z $
 */

#ifndef _phy_rstr_h_
#define _phy_rstr_h_

extern const char rstr_txpwrbckof[];
extern const char rstr_rssicorratten[];
extern const char rstr_phycal_tempdelta[];
extern const char rstr_mintxpower[];
extern const char rstr_cckPwrIdxCorr[];
extern const char rstr_calmgr[];
extern const char rstr_interference[];
extern const char rstr_tssilimucod[];
extern const char rstr_rssicorrnorm[];
extern const char rstr_5g_cga[];
extern const char rstr_2g_cga[];

extern const char rstr_tempthresh[];
extern const char rstr_temps_hysteresis[];
extern const char rstr_phyrxdesens[];
extern const char rstr_ldpc[];
extern const char rstr_core2slicemap[];

#ifdef RADIO_HEALTH_CHECK
extern const char rstr_rhc_tempthresh[];
extern const char rstr_rhc_temp_fail_time[];
#endif /* RADIO_HEALTH_CHECK */

#ifdef WL_PROXDETECT
extern const char rstr_proxd_basekival[];
extern const char rstr_proxd_basektval[];
extern const char rstr_proxd_80mkval[];
extern const char rstr_proxd_40mkval[];
extern const char rstr_proxd_20mkval[];
extern const char rstr_proxd_2gkval[];
extern const char rstr_proxdi_rate80m[];
extern const char rstr_proxdi_rate40m[];
extern const char rstr_proxdi_rate20m[];
extern const char rstr_proxdi_rate2g[];
extern const char rstr_proxdt_rate80m[];
extern const char rstr_proxdt_rate40m[];
extern const char rstr_proxdt_rate20m[];
extern const char rstr_proxdt_rate2g[];
extern const char rstr_proxdi_ack[];
extern const char rstr_proxdt_ack[];
extern const char rstr_proxd_sub80m40m[];
extern const char rstr_proxd_sub80m20m[];
extern const char rstr_proxd_sub40m20m[];
#endif /* WL_PROXDETECT */

/* reclaim strings that are only used in attach functions */
extern const char BCMATTACHDATA(rstr_swctrlmap_2g)[];
extern const char BCMATTACHDATA(rstr_swctrlmap_5g)[];
extern const char BCMATTACHDATA(rstr_swctrlmapext_2g)[];
extern const char BCMATTACHDATA(rstr_swctrlmapext_5g)[];
extern const char BCMATTACHDATA(rstr_txswctrlmap_2g)[];
extern const char BCMATTACHDATA(rstr_txswctrlmap_2g_mask)[];
extern const char BCMATTACHDATA(rstr_txswctrlmap_5g)[];

extern const char BCMATTACHDATA(rstr_fem_table_init_val)[];
extern const char BCMATTACHDATA(rstr_dot11b_opts)[];
extern const char BCMATTACHDATA(rstr_tiny_maxrxgain)[];
extern const char BCMATTACHDATA(rstr_clb2gslice0core0)[];
extern const char BCMATTACHDATA(rstr_clb2gslice1core0)[];
extern const char BCMATTACHDATA(rstr_clb5gslice0core0)[];
extern const char BCMATTACHDATA(rstr_clb5gslice1core0)[];
extern const char BCMATTACHDATA(rstr_clb2gslice0core1)[];
extern const char BCMATTACHDATA(rstr_clb2gslice1core1)[];
extern const char BCMATTACHDATA(rstr_clb5gslice0core1)[];
extern const char BCMATTACHDATA(rstr_clb5gslice1core1)[];
extern const char BCMATTACHDATA(rstr_btc_prisel_mask)[];
extern const char BCMATTACHDATA(rstr_btc_prisel_ant_mask)[];
extern const char BCMATTACHDATA(rstr_clb_swctrl_smask_ant0)[];
extern const char BCMATTACHDATA(rstr_clb_swctrl_smask_ant1)[];

/* Used by et module ac layer */
extern const char BCMATTACHDATA(rstr_et_mode)[];

/* Radio Band Capability Indicator */
extern const char BCMATTACHDATA(rstr_bandcap)[];

/* Used in noise module ac specific layer */
extern const char BCMATTACHDATA(rstr_noiselvl2gaD)[];
extern const char BCMATTACHDATA(rstr_noiselvl5gaD)[];

/* Used by TPC module: common layer */
extern const char BCMATTACHDATA(rstr_maxp2ga0)[];
extern const char BCMATTACHDATA(rstr_pa2ga0)[];
extern const char BCMATTACHDATA(rstr_maxp2ga1)[];
extern const char BCMATTACHDATA(rstr_tssifloor2g)[];
extern const char BCMATTACHDATA(rstr_pa2ga1)[];
extern const char BCMATTACHDATA(rstr_pa2ga2)[];
extern const char BCMATTACHDATA(rstr_maxp2ga2)[];
extern const char BCMATTACHDATA(rstr_pa2ga3)[];
extern const char BCMATTACHDATA(rstr_pa2gbw40a0)[];
extern const char BCMATTACHDATA(rstr_pa2g20ccka3)[];
extern const char BCMATTACHDATA(rstr_pa2g20ccka2)[];
extern const char BCMATTACHDATA(rstr_pa2g20ccka1)[];
extern const char BCMATTACHDATA(rstr_pa2g20ccka0)[];
extern const char BCMATTACHDATA(rstr_pa2gccka1)[];
extern const char BCMATTACHDATA(rstr_pa2gccka0)[];
extern const char BCMATTACHDATA(rstr_pdoffset40ma0)[];
extern const char BCMATTACHDATA(rstr_pdoffset2g40mvalid)[];
extern const char BCMATTACHDATA(rstr_pdoffset2g20in20a0)[];
extern const char BCMATTACHDATA(rstr_pdoffset2g20in20a1)[];
extern const char BCMATTACHDATA(rstr_pdoffset2g20in20a2)[];
extern const char BCMATTACHDATA(rstr_pdoffset5gsubbanda0)[];
extern const char BCMATTACHDATA(rstr_pdoffsetcckma0)[];
extern const char BCMATTACHDATA(rstr_pdoffset80ma0)[];
extern const char BCMATTACHDATA(rstr_pdoffset2g40ma0)[];
extern const char BCMATTACHDATA(rstr_cckulbpwroffset0)[];
extern const char BCMATTACHDATA(rstr_pdoffset40ma1)[];
extern const char BCMATTACHDATA(rstr_cckpwroffset0)[];
extern const char BCMATTACHDATA(rstr_pdoffset5gsubbanda1)[];
extern const char BCMATTACHDATA(rstr_pdoffsetcckma1)[];
extern const char BCMATTACHDATA(rstr_cckulbpwroffset2)[];
extern const char BCMATTACHDATA(rstr_pdoffset2g40ma1)[];
extern const char BCMATTACHDATA(rstr_cckulbpwroffset1)[];
extern const char BCMATTACHDATA(rstr_pdoffset80ma2)[];
extern const char BCMATTACHDATA(rstr_cckpwroffset1)[];
extern const char BCMATTACHDATA(rstr_pdoffset40ma2)[];
extern const char BCMATTACHDATA(rstr_pdoffsetcckma2)[];
extern const char BCMATTACHDATA(rstr_pdoffset2g40ma2)[];
extern const char BCMATTACHDATA(rstr_cckpwroffset2)[];
extern const char BCMATTACHDATA(rstr_tempoffset)[];
extern const char BCMATTACHDATA(rstr_pa2g40a0)[];
extern const char BCMATTACHDATA(rstr_pa2g40a1)[];
extern const char BCMATTACHDATA(rstr_pa2g40a2)[];
extern const char BCMATTACHDATA(rstr_pa2g40a3)[];
extern const char BCMATTACHDATA(rstr_pa5ga0)[];
extern const char BCMATTACHDATA(rstr_pa5g40a0)[];
extern const char BCMATTACHDATA(rstr_pa5g80a0)[];
extern const char BCMATTACHDATA(rstr_pa5g160a0)[];
extern const char BCMATTACHDATA(rstr_pa5ga1)[];
extern const char BCMATTACHDATA(rstr_pa5g40a1)[];
extern const char BCMATTACHDATA(rstr_pa5g80a1)[];
extern const char BCMATTACHDATA(rstr_pa5g160a1)[];
extern const char BCMATTACHDATA(rstr_pa5ga2)[];

extern const char BCMATTACHDATA(rstr_maxp5ga0)[];
extern const char BCMATTACHDATA(rstr_tssifloor5g)[];
extern const char BCMATTACHDATA(rstr_maxp5ga1)[];
extern const char BCMATTACHDATA(rstr_maxp5ga2)[];
extern const char BCMATTACHDATA(rstr_pa5ga3)[];
extern const char BCMATTACHDATA(rstr_pa5gbw40a0)[];
extern const char BCMATTACHDATA(rstr_pa5gbw80a0)[];
extern const char BCMATTACHDATA(rstr_pa5gbw4080a0)[];
extern const char BCMATTACHDATA(rstr_pa5gbw4080a1)[];

extern const char BCMATTACHDATA(rstr_txpwr2gAdcScale)[];
extern const char BCMATTACHDATA(rstr_txpwr5gAdcScale)[];

extern const char BCMATTACHDATA(rstr_cckbw202gpo)[];
extern const char BCMATTACHDATA(rstr_cckbw20ul2gpo)[];
extern const char BCMATTACHDATA(rstr_ofdmlrbw202gpo)[];
extern const char BCMATTACHDATA(rstr_dot11agofdmhrbw202gpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw202gpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw402gpo)[];
extern const char BCMATTACHDATA(rstr_sb20in40lrpo)[];
extern const char BCMATTACHDATA(rstr_sb20in40hrpo)[];
extern const char BCMATTACHDATA(rstr_dot11agduphrpo)[];
extern const char BCMATTACHDATA(rstr_dot11agduplrpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw205glpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw405glpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw205gmpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw405gmpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw205ghpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw405ghpo)[];

extern const char BCMATTACHDATA(rstr_mcslr5glpo)[];
extern const char BCMATTACHDATA(rstr_mcslr5gmpo)[];
extern const char BCMATTACHDATA(rstr_mcslr5ghpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw805glpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw805gmpo)[];

extern const char BCMATTACHDATA(rstr_mcsbw805ghpo)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160lr5glpo)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160hr5glpo)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160lr5gmpo)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160hr5gmpo)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160lr5ghpo)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160hr5ghpo)[];
extern const char BCMATTACHDATA(rstr_sb40and80lr5glpo)[];
extern const char BCMATTACHDATA(rstr_sb40and80hr5glpo)[];
extern const char BCMATTACHDATA(rstr_sb40and80lr5gmpo)[];
extern const char BCMATTACHDATA(rstr_sb40and80hr5gmpo)[];
extern const char BCMATTACHDATA(rstr_sb40and80lr5ghpo)[];
extern const char BCMATTACHDATA(rstr_sb40and80hr5ghpo)[];

extern const char BCMATTACHDATA(rstr_mcsbw205gx1po)[];
extern const char BCMATTACHDATA(rstr_mcsbw405gx1po)[];
extern const char BCMATTACHDATA(rstr_mcsbw205gx2po)[];
extern const char BCMATTACHDATA(rstr_mcsbw405gx2po)[];
extern const char BCMATTACHDATA(rstr_mcslr5gx1po)[];
extern const char BCMATTACHDATA(rstr_mcslr5gx2po)[];
extern const char BCMATTACHDATA(rstr_mcsbw805gx1po)[];
extern const char BCMATTACHDATA(rstr_mcsbw805gx2po)[];
extern const char BCMATTACHDATA(rstr_mcsbw1605gx1po)[];
extern const char BCMATTACHDATA(rstr_mcsbw1605gx2po)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160lr5gx1po)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160hr5gx1po)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160lr5gx2po)[];
extern const char BCMATTACHDATA(rstr_sb20in80and160hr5gx2po)[];
extern const char BCMATTACHDATA(rstr_sb40and80lr5gx1po)[];
extern const char BCMATTACHDATA(rstr_sb40and80hr5gx1po)[];
extern const char BCMATTACHDATA(rstr_sb40and80lr5gx2po)[];
extern const char BCMATTACHDATA(rstr_sb40and80hr5gx2po)[];
extern const char BCMATTACHDATA(rstr_mcsbw1605glpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw1605gmpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw1605ghpo)[];
extern const char BCMATTACHDATA(rstr_mcs1024qam2gpo)[];
extern const char BCMATTACHDATA(rstr_mcs1024qam5glpo)[];
extern const char BCMATTACHDATA(rstr_mcs1024qam5gmpo)[];
extern const char BCMATTACHDATA(rstr_mcs1024qam5ghpo)[];
extern const char BCMATTACHDATA(rstr_mcs1024qam5gx1po)[];
extern const char BCMATTACHDATA(rstr_mcs1024qam5gx2po)[];
extern const char BCMATTACHDATA(rstr_mcs8poexp)[];
extern const char BCMATTACHDATA(rstr_mcs9poexp)[];
extern const char BCMATTACHDATA(rstr_mcs10poexp)[];
extern const char BCMATTACHDATA(rstr_mcs11poexp)[];

extern const char BCMATTACHDATA(rstr_pdoffset80ma1)[];
extern const char BCMATTACHDATA(rstr_pa5g40a2)[];
extern const char BCMATTACHDATA(rstr_pa5g80a2)[];
extern const char BCMATTACHDATA(rstr_pa5g160a2)[];
extern const char BCMATTACHDATA(rstr_pa5g40a3)[];
extern const char BCMATTACHDATA(rstr_pa5g80a3)[];
extern const char BCMATTACHDATA(rstr_pa5g160a3)[];
extern const char BCMATTACHDATA(rstr_maxp2gb0a0)[];
extern const char BCMATTACHDATA(rstr_maxp5gb0a0)[];
extern const char BCMATTACHDATA(rstr_maxp5gb1a0)[];
extern const char BCMATTACHDATA(rstr_maxp5gb2a0)[];
extern const char BCMATTACHDATA(rstr_maxp5gb3a0)[];
extern const char BCMATTACHDATA(rstr_maxp5gb4a0)[];
extern const char BCMATTACHDATA(rstr_maxp2gb0a1)[];
extern const char BCMATTACHDATA(rstr_maxp5gb0a1)[];
extern const char BCMATTACHDATA(rstr_maxp5gb1a1)[];
extern const char BCMATTACHDATA(rstr_maxp5gb2a1)[];
extern const char BCMATTACHDATA(rstr_maxp5gb3a1)[];
extern const char BCMATTACHDATA(rstr_maxp5gb4a1)[];
extern const char BCMATTACHDATA(rstr_maxp2gb0a2)[];
extern const char BCMATTACHDATA(rstr_maxp5gb0a2)[];
extern const char BCMATTACHDATA(rstr_maxp5gb1a2)[];
extern const char BCMATTACHDATA(rstr_maxp5gb2a2)[];
extern const char BCMATTACHDATA(rstr_maxp5gb3a2)[];
extern const char BCMATTACHDATA(rstr_maxp5gb4a2)[];
extern const char BCMATTACHDATA(rstr_maxp2gb0a3)[];
extern const char BCMATTACHDATA(rstr_maxp5gb0a3)[];
extern const char BCMATTACHDATA(rstr_maxp5gb1a3)[];
extern const char BCMATTACHDATA(rstr_maxp5gb2a3)[];
extern const char BCMATTACHDATA(rstr_maxp5gb3a3)[];
extern const char BCMATTACHDATA(rstr_maxp5gb4a3)[];
extern const char BCMATTACHDATA(rstr_pdoffset2gcck)[];
extern const char BCMATTACHDATA(rstr_pdoffset2gcck20m)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m5gb0)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m5gb1)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m5gb2)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m5gb3)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m5gb4)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in80m5gb0)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in80m5gb1)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in80m5gb2)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in80m5gb3)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in80m5gb4)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in80m5gb0)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in80m5gb1)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in80m5gb2)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in80m5gb3)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in80m5gb4)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m5gcore3)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m5gcore3_1)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in80m5gcore3)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in80m5gcore3_1)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in80m5gcore3)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in80m5gcore3_1)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m2g)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in40m2gcore3)[];

extern const char BCMATTACHDATA(rstr_pdoffset20in160m5gc0)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in160m5gc1)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in160m5gc2)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in160m5gc3)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in160m5gc0)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in160m5gc1)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in160m5gc2)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in160m5gc3)[];
extern const char BCMATTACHDATA(rstr_pdoffset80in160m5gc0)[];
extern const char BCMATTACHDATA(rstr_pdoffset80in160m5gc1)[];
extern const char BCMATTACHDATA(rstr_pdoffset80in160m5gc2)[];
extern const char BCMATTACHDATA(rstr_pdoffset80in160m5gc3)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in160m5gcore3)[];
extern const char BCMATTACHDATA(rstr_pdoffset20in160m5gcore3_1)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in160m5gcore3)[];
extern const char BCMATTACHDATA(rstr_pdoffset40in160m5gcore3_1)[];
extern const char BCMATTACHDATA(rstr_pdoffset80in160m5gcore3)[];
extern const char BCMATTACHDATA(rstr_pdoffset80in160m5gcore3_1)[];

/* ACPHY MISC params */
extern const char BCMATTACHDATA(rstr_rawtempsense)[];
extern const char BCMATTACHDATA(rstr_rxgainerr2ga0)[];
extern const char BCMATTACHDATA(rstr_rxgainerr2ga1)[];
extern const char BCMATTACHDATA(rstr_rxgainerr2ga2)[];
extern const char BCMATTACHDATA(rstr_rxgainerr2ga3)[];
extern const char BCMATTACHDATA(rstr_rxgainerr5ga0)[];
extern const char BCMATTACHDATA(rstr_rxgainerr5ga1)[];
extern const char BCMATTACHDATA(rstr_rxgainerr5ga2)[];
extern const char BCMATTACHDATA(rstr_rxgainerr5ga3)[];

/* ACPHY radio param */
extern const char BCMATTACHDATA(rstr_use5gpllfor2g)[];

/* ACPHY RSSI params */
extern const char BCMATTACHDATA(rstr_rxgaintempcoeff2g)[];
extern const char BCMATTACHDATA(rstr_rxgaintempcoeff5gl)[];
extern const char BCMATTACHDATA(rstr_rxgaintempcoeff5gml)[];
extern const char BCMATTACHDATA(rstr_rxgaintempcoeff5gmu)[];
extern const char BCMATTACHDATA(rstr_rxgaintempcoeff5gh)[];
extern const char BCMATTACHDATA(rstr_rssicorrnorm_cD)[];
extern const char BCMATTACHDATA(rstr_rssicorrnorm5g_cD)[];
extern const char BCMATTACHDATA(rstr_rssi_delta_2g_cD)[];
extern const char BCMATTACHDATA(rstr_rssi_delta_5gS_cD)[];
extern const char BCMATTACHDATA(rstr_gain_cal_temp)[];
extern const char BCMATTACHDATA(rstr_rssi_cal_rev)[];
extern const char BCMATTACHDATA(rstr_rssi_qdB_en)[];
extern const char BCMATTACHDATA(rstr_rxgaincal_rssical)[];
extern const char BCMATTACHDATA(rstr_rud_agc_enable)[];
extern const char BCMATTACHDATA(rstr_rssi_delta_2gS)[];
extern const char BCMATTACHDATA(rstr_rssi_delta_5gS)[];
extern const char BCMATTACHDATA(rstr_rssi_cal_freq_grp_2g)[];
extern const char BCMATTACHDATA(rstr_num_rssi_cal_gi_2g)[];
extern const char BCMATTACHDATA(rstr_num_rssi_cal_gi_5g)[];
extern const char BCMATTACHDATA(rstr_rssi_lna1_routadj_en_5g)[];

#ifdef POWPERCHANNL
extern const char BCMATTACHDATA(rstr_PowOffs2GTNA0)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTNA1)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTNA2)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTLA0)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTLA1)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTLA2)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTHA0)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTHA1)[];
extern const char BCMATTACHDATA(rstr_PowOffs2GTHA2)[];
extern const char BCMATTACHDATA(rstr_PowOffsTempRange)[];
#endif /* POWPERCHANNL */

/* reclaim strings that are only used in attach functions */
extern const char BCMATTACHDATA(rstr_pagc2g)[];
extern const char BCMATTACHDATA(rstr_sw_txchain_mask)[];
extern const char BCMATTACHDATA(rstr_sw_rxchain_mask)[];
extern const char BCMATTACHDATA(rstr_pagc5g)[];
extern const char BCMATTACHDATA(rstr_rpcal2g)[];
extern const char BCMATTACHDATA(rstr_rpcal2gcore3)[];
extern const char BCMATTACHDATA(rstr_femctrl)[];
extern const char BCMATTACHDATA(rstr_papdmode)[];
extern const char BCMATTACHDATA(rstr_pdgain2g)[];
extern const char BCMATTACHDATA(rstr_pdgain5g)[];
extern const char BCMATTACHDATA(rstr_epacal2g)[];
extern const char BCMATTACHDATA(rstr_epacal5g)[];
extern const char BCMATTACHDATA(rstr_itrsw)[];
extern const char BCMATTACHDATA(rstr_offtgpwr)[];
extern const char BCMATTACHDATA(rstr_epagain2g)[];
extern const char BCMATTACHDATA(rstr_epagain5g)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb0)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb1)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb2)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb3)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb0core3)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb1core3)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb2core3)[];
extern const char BCMATTACHDATA(rstr_rpcal5gb3core3)[];
extern const char BCMATTACHDATA(rstr_txidxcap2g)[];
extern const char BCMATTACHDATA(rstr_txidxcap5g)[];
extern const char BCMATTACHDATA(rstr_txidxmincap2g)[];
extern const char BCMATTACHDATA(rstr_txidxmincap5g)[];
extern const char BCMATTACHDATA(rstr_txidxcaplow)[];
extern const char BCMATTACHDATA(rstr_maxepagain)[];
extern const char BCMATTACHDATA(rstr_maxchipoutpower)[];
extern const char BCMATTACHDATA(rstr_extpagain2g)[];
extern const char BCMATTACHDATA(rstr_extpagain5g)[];
extern const char BCMATTACHDATA(rstr_boardflags3)[];
extern const char BCMATTACHDATA(rstr_txiqcalidx2g)[];
extern const char BCMATTACHDATA(rstr_txiqcalidx5g)[];
extern const char BCMATTACHDATA(rstr_txgaintbl5g)[];
extern const char BCMATTACHDATA(rstr_pwrdampingen)[];
extern const char BCMATTACHDATA(rstr_subband5gver)[];
extern const char BCMATTACHDATA(rstr_dacratemode2g)[];
extern const char BCMATTACHDATA(rstr_dacratemode5g)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgamma2g)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgain2g)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgamma5g20)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgamma5g40)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgamma5g80)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgain5g20)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgain5g40)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgain5g80)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgamma2g_ch13)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgamma2g_ch1)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgain2g_ch13)[];
extern const char BCMATTACHDATA(rstr_paprrmcsgain2g_ch1)[];
extern const char BCMATTACHDATA(rstr_oob_gaint)[];
extern const char BCMATTACHDATA(rstr_vcodivmode)[];
extern const char BCMATTACHDATA(rstr_fdss_interp_en)[];
extern const char BCMATTACHDATA(rstr_fdss_level_2g)[];
extern const char BCMATTACHDATA(rstr_fdss_level_5g)[];
extern const char BCMATTACHDATA(rstr_fdss_bandedge_2g_en)[];
extern const char BCMATTACHDATA(rstr_fdss_level_2g_ch13)[];
extern const char BCMATTACHDATA(rstr_fdss_level_2g_ch1)[];
extern const char BCMATTACHDATA(rstr_ldo3p3_voltage)[];
extern const char BCMATTACHDATA(rstr_paldo3p3_voltage)[];
extern const char BCMATTACHDATA(rstr_epacal2g_mask)[];
extern const char BCMATTACHDATA(rstr_cckdigfilttype)[];
extern const char BCMATTACHDATA(rstr_ofdmfilttype_5gbe)[];
extern const char BCMATTACHDATA(rstr_ofdmfilttype_2gbe)[];
extern const char BCMATTACHDATA(rstr_tworangetssi2g)[];
extern const char BCMATTACHDATA(rstr_tworangetssi5g)[];
extern const char BCMATTACHDATA(rstr_lowpowerrange2g)[];
extern const char BCMATTACHDATA(rstr_lowpowerrange5g)[];
extern const char BCMATTACHDATA(rstr_paprdis)[];
extern const char BCMATTACHDATA(rstr_papdwar)[];
extern const char BCMATTACHDATA(rstr_low_adc_rate_en)[];
extern const char BCMATTACHDATA(rstr_asymmetricjammermod)[];

extern const char BCMATTACHDATA(rstr_tssisleep_en)[];
extern const char BCMATTACHDATA(ed_thresh2g)[];
extern const char BCMATTACHDATA(ed_thresh5g)[];
extern const char BCMATTACHDATA(rstr_LTEJ_WAR_en)[];
extern const char BCMATTACHDATA(rstr_thresh_noise_cal)[];
extern const char BCMATTACHDATA(rstr_bphyscale)[];
extern const char BCMATTACHDATA(rstr_antdiv_rfswctrlpin_a0)[];
extern const char BCMATTACHDATA(rstr_antdiv_rfswctrlpin_a1)[];
#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
extern const char BCMATTACHDATA(rstr_swctrl_to_bt_in_coex)[];
#endif // endif
#if defined(WLC_TXCAL) || (defined(WLOLPC) && !defined(WLOLPC_DISABLED))
extern const char BCMATTACHDATA(rstr_olpc_thresh)[];
extern const char BCMATTACHDATA(rstr_olpc_thresh2g)[];
extern const char BCMATTACHDATA(rstr_olpc_thresh5g)[];
extern const char BCMATTACHDATA(rstr_olpc_tempslope2g)[];
extern const char BCMATTACHDATA(rstr_olpc_tempslope5g)[];
extern const char BCMATTACHDATA(rstr_olpc_anchor2g)[];
extern const char BCMATTACHDATA(rstr_olpc_anchor5g)[];
extern const char BCMATTACHDATA(rstr_olpc_idx_in_use)[];
extern const char BCMATTACHDATA(rstr_olpc_offset)[];
extern const char BCMATTACHDATA(rstr_disable_olpc)[];
#endif /* WLC_TXCAL || ((WLOLPC) && !(WLOLPC_DISABLED)) */
extern const char BCMATTACHDATA(rstr_initbaseidx5govrval)[];
extern const char BCMATTACHDATA(rstr_txpwrindexlimit)[];
extern const char BCMATTACHDATA(rstr_initbaseidx2govrval)[];
extern const char BCMATTACHDATA(rstr_txnospurmod5g)[];
extern const char BCMATTACHDATA(rstr_txnospurmod2g)[];
extern const char BCMATTACHDATA(rstr_txnoBW80ClkSwitch)[];
extern const char BCMATTACHDATA(rstr_etmode)[];

extern const char BCMATTACHDATA(rstr_lpflags)[];

/* FCC power limit control on ch12/13 */
#ifdef FCC_PWR_LIMIT_2G
extern const char BCMATTACHDATA(rstr_fccpwrch12)[];
extern const char BCMATTACHDATA(rstr_fccpwrch13)[];
extern const char BCMATTACHDATA(rstr_fccpwroverride)[];
#endif /* FCC_PWR_LIMIT_2G */

#if (defined(WLTEST) || defined(WLPKTENG))
extern const char BCMATTACHDATA(rstr_perratedpd2g)[];
extern const char BCMATTACHDATA(rstr_perratedpd5g)[];
#endif // endif

#ifdef WLC_TXFDIQ
extern const char BCMATTACHDATA(rstr_txfdiqcalenable)[];
#endif // endif

extern const char BCMATTACHDATA(rstr_w1clipmod)[];

#if defined(WLTEST)
extern const char BCMATTACHDATA(rstr_cbuck_out)[];
#else
extern const char BCMATTACHDATA(rstr_cbuck_out)[];
#endif // endif
extern const char BCMATTACHDATA(rstr_ldo3p3_2g)[];
extern const char BCMATTACHDATA(rstr_ldo3p3_5g)[];
extern const char BCMATTACHDATA(rstr_ccktpc_loop_en)[];
extern const char BCMATTACHDATA(rstr_csml)[];
extern const char BCMATTACHDATA(rstr_ocl)[];
extern const char BCMATTACHDATA(rstr_ocl_cm)[];
extern const char BCMATTACHDATA(rstr_lesi_en)[];
extern const char BCMATTACHDATA(rstr_bphymrc_en)[];

extern const char BCMATTACHDATA(rstr_nap_en)[];
extern const char BCMATTACHDATA(rstr_swctrlmap4_cfg)[];
extern const char BCMATTACHDATA(rstr_swctrlmap4_S2g_fem3to0)[];
extern const char BCMATTACHDATA(rstr_swctrlmap4_S5g_fem3to0)[];
extern const char BCMATTACHDATA(rstr_swctrlmap4_S2g_fem7to4)[];
extern const char BCMATTACHDATA(rstr_swctrlmap4_S5g_fem7to4)[];
extern const char BCMATTACHDATA(rstr_swctrlmap4_S2g_fem9to8)[];
extern const char BCMATTACHDATA(rstr_swctrlmap4_S5g_fem9to8)[];
extern const char BCMATTACHDATA(rstr_gainctrlsph)[];

/* NVRAM PARAM String for ulp_adc_mode */
extern const char BCMATTACHDATA(rstr_ulpadc)[];
extern const char BCMATTACHDATA(rstr_spurcan_chlist)[];
extern const char BCMATTACHDATA(rstr_spurcan_spfreq)[];
extern const char BCMATTACHDATA(rstr_spurcan_numspur)[];

extern const char BCMATTACHDATA(rstr_vcotune)[];

#ifdef WLC_SW_DIVERSITY
extern const char BCMATTACHDATA(rstr_swdiv_en)[];
extern const char BCMATTACHDATA(rstr_swdiv_gpio)[];
extern const char BCMATTACHDATA(rstr_swdiv_gpioctrl)[];
extern const char BCMATTACHDATA(rstr_swdiv_swctrl_en)[];
extern const char BCMATTACHDATA(rstr_swdiv_swctrl_mask)[];
extern const char BCMATTACHDATA(rstr_swdiv_swctrl_ant0)[];
extern const char BCMATTACHDATA(rstr_swdiv_swctrl_ant1)[];
extern const char BCMATTACHDATA(rstr_swdiv_coreband_map)[];
extern const char BCMATTACHDATA(rstr_swdiv_antmap2g_main)[];
extern const char BCMATTACHDATA(rstr_swdiv_antmap5g_main)[];
extern const char BCMATTACHDATA(rstr_swdiv_antmap2g_aux)[];
extern const char BCMATTACHDATA(rstr_swdiv_antmap5g_aux)[];
#endif /* WLC_SW_DIVERSITY */
extern const char BCMATTACHDATA(rstr_nonbf_logen_mode_en)[];
extern const char BCMATTACHDATA(rstr_elna2g)[];
extern const char BCMATTACHDATA(rstr_elna5g)[];
extern const char BCMATTACHDATA(rstr_aa2g)[];
extern const char BCMATTACHDATA(rstr_aa5g)[];
extern const char BCMATTACHDATA(rstr_tssipos2g)[];
extern const char BCMATTACHDATA(rstr_pdetrange2g)[];
extern const char BCMATTACHDATA(rstr_antswctl2g)[];
extern const char BCMATTACHDATA(rstr_tssipos5g)[];
extern const char BCMATTACHDATA(rstr_pdetrange5g)[];
extern const char BCMATTACHDATA(rstr_antswctl5g)[];
extern const char BCMATTACHDATA(rstr_bw40po)[];
extern const char BCMATTACHDATA(rstr_cddpo)[];
extern const char BCMATTACHDATA(rstr_stbcpo)[];
extern const char BCMATTACHDATA(rstr_bwduppo)[];
extern const char BCMATTACHDATA(rstr_txpid2ga0)[];
extern const char BCMATTACHDATA(rstr_txpid2ga1)[];
extern const char BCMATTACHDATA(rstr_pa2gw0a2)[];
extern const char BCMATTACHDATA(rstr_pa2gw1a2)[];
extern const char BCMATTACHDATA(rstr_pa2gw2a2)[];
extern const char BCMATTACHDATA(rstr_maxp5gla2)[];
extern const char BCMATTACHDATA(rstr_pa5glw0a2)[];
extern const char BCMATTACHDATA(rstr_pa5glw1a2)[];
extern const char BCMATTACHDATA(rstr_pa5glw2a2)[];
extern const char BCMATTACHDATA(rstr_pa5gw0a2)[];
extern const char BCMATTACHDATA(rstr_pa5gw1a2)[];
extern const char BCMATTACHDATA(rstr_pa5gw2a2)[];
extern const char BCMATTACHDATA(rstr_maxp5gha2)[];
extern const char BCMATTACHDATA(rstr_pa5ghw0a2)[];
extern const char BCMATTACHDATA(rstr_pa5ghw1a2)[];
extern const char BCMATTACHDATA(rstr_pa5ghw2a2)[];
extern const char BCMATTACHDATA(rstr_pa2gw0a0)[];
extern const char BCMATTACHDATA(rstr_pa2gw0a1)[];
extern const char BCMATTACHDATA(rstr_pa2gw1a0)[];
extern const char BCMATTACHDATA(rstr_pa2gw1a1)[];
extern const char BCMATTACHDATA(rstr_pa2gw2a0)[];
extern const char BCMATTACHDATA(rstr_pa2gw2a1)[];
extern const char BCMATTACHDATA(rstr_itt2ga0)[];
extern const char BCMATTACHDATA(rstr_itt2ga1)[];
extern const char BCMATTACHDATA(rstr_cck2gpo)[];
extern const char BCMATTACHDATA(rstr_ofdm2gpo)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo0)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo1)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo2)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo3)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo4)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo5)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo6)[];
extern const char BCMATTACHDATA(rstr_mcs2gpo7)[];
extern const char BCMATTACHDATA(rstr_txpid5gla0)[];
extern const char BCMATTACHDATA(rstr_txpid5gla1)[];
extern const char BCMATTACHDATA(rstr_maxp5gla0)[];
extern const char BCMATTACHDATA(rstr_maxp5gla1)[];
extern const char BCMATTACHDATA(rstr_pa5glw0a0)[];
extern const char BCMATTACHDATA(rstr_pa5glw0a1)[];
extern const char BCMATTACHDATA(rstr_pa5glw1a0)[];
extern const char BCMATTACHDATA(rstr_pa5glw1a1)[];
extern const char BCMATTACHDATA(rstr_pa5glw2a0)[];
extern const char BCMATTACHDATA(rstr_pa5glw2a1)[];
extern const char BCMATTACHDATA(rstr_ofdm5glpo)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo0)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo1)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo2)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo3)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo4)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo5)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo6)[];
extern const char BCMATTACHDATA(rstr_mcs5glpo7)[];
extern const char BCMATTACHDATA(rstr_txpid5ga0)[];
extern const char BCMATTACHDATA(rstr_txpid5ga1)[];
extern const char BCMATTACHDATA(rstr_pa5gw0a0)[];
extern const char BCMATTACHDATA(rstr_pa5gw0a1)[];
extern const char BCMATTACHDATA(rstr_pa5gw1a0)[];
extern const char BCMATTACHDATA(rstr_pa5gw1a1)[];
extern const char BCMATTACHDATA(rstr_pa5gw2a0)[];
extern const char BCMATTACHDATA(rstr_pa5gw2a1)[];
extern const char BCMATTACHDATA(rstr_itt5ga0)[];
extern const char BCMATTACHDATA(rstr_itt5ga1)[];
extern const char BCMATTACHDATA(rstr_ofdm5gpo)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo0)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo1)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo2)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo3)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo4)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo5)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo6)[];
extern const char BCMATTACHDATA(rstr_mcs5gpo7)[];
extern const char BCMATTACHDATA(rstr_txpid5gha0)[];
extern const char BCMATTACHDATA(rstr_txpid5gha1)[];
extern const char BCMATTACHDATA(rstr_maxp5gha0)[];
extern const char BCMATTACHDATA(rstr_maxp5gha1)[];
extern const char BCMATTACHDATA(rstr_pa5ghw0a0)[];
extern const char BCMATTACHDATA(rstr_pa5ghw0a1)[];
extern const char BCMATTACHDATA(rstr_pa5ghw1a0)[];
extern const char BCMATTACHDATA(rstr_pa5ghw1a1)[];
extern const char BCMATTACHDATA(rstr_pa5ghw2a0)[];
extern const char BCMATTACHDATA(rstr_pa5ghw2a1)[];
extern const char BCMATTACHDATA(rstr_ofdm5ghpo)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo0)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo1)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo2)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo3)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo4)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo5)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo6)[];
extern const char BCMATTACHDATA(rstr_mcs5ghpo7)[];
extern const char BCMATTACHDATA(rstr_triso2g)[];
extern const char BCMATTACHDATA(rstr_triso5g)[];
extern const char BCMATTACHDATA(rstr_pa2gw0a3)[];
extern const char BCMATTACHDATA(rstr_pa2gw1a3)[];
extern const char BCMATTACHDATA(rstr_pa2gw2a3)[];
extern const char BCMATTACHDATA(rstr_maxp5ga3)[];
extern const char BCMATTACHDATA(rstr_pa5gw0a3)[];
extern const char BCMATTACHDATA(rstr_pa5gw1a3)[];
extern const char BCMATTACHDATA(rstr_pa5gw2a3)[];
extern const char BCMATTACHDATA(rstr_maxp5gla3)[];
extern const char BCMATTACHDATA(rstr_pa5glw2a3)[];
extern const char BCMATTACHDATA(rstr_pa5glw0a3)[];
extern const char BCMATTACHDATA(rstr_pa5glw1a3)[];
extern const char BCMATTACHDATA(rstr_maxp5gha3)[];
extern const char BCMATTACHDATA(rstr_pa5ghw0a3)[];
extern const char BCMATTACHDATA(rstr_pa5ghw1a3)[];
extern const char BCMATTACHDATA(rstr_pa5ghw2a3)[];

extern const char BCMATTACHDATA(rstr_legofdmbw202gpo)[];
extern const char BCMATTACHDATA(rstr_legofdmbw20ul2gpo)[];
extern const char BCMATTACHDATA(rstr_legofdmbw205glpo)[];
extern const char BCMATTACHDATA(rstr_legofdmbw20ul5glpo)[];
extern const char BCMATTACHDATA(rstr_legofdmbw205gmpo)[];
extern const char BCMATTACHDATA(rstr_legofdmbw20ul5gmpo)[];
extern const char BCMATTACHDATA(rstr_legofdmbw205ghpo)[];
extern const char BCMATTACHDATA(rstr_legofdmbw20ul5ghpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw20ul2gpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw20ul5glpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw20ul5gmpo)[];
extern const char BCMATTACHDATA(rstr_mcsbw20ul5ghpo)[];
extern const char BCMATTACHDATA(rstr_legofdm40duppo)[];

#endif /* _phy_rstr_h_ */
