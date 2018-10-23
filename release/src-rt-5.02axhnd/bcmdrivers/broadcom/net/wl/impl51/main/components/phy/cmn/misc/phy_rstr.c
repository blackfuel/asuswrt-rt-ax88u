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
 * $Id: phy_rstr.c 752074 2018-03-14 19:56:20Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_rstr.h>

const char BCMATTACHDATA(rstr_txpwrbckof)[] = "txpwrbckof";
const char BCMATTACHDATA(rstr_rssicorratten)[] = "rssicorratten";
const char BCMATTACHDATA(rstr_phycal_tempdelta)[] = "phycal_tempdelta";
const char BCMATTACHDATA(rstr_mintxpower)[] = "mintxpower";
const char BCMATTACHDATA(rstr_cckPwrIdxCorr)[] = "cckPwrIdxCorr";
const char BCMATTACHDATA(rstr_calmgr)[] = "calmgr";
const char BCMATTACHDATA(rstr_interference)[] = "interference";
const char BCMATTACHDATA(rstr_tssilimucod)[] = "tssilimucod";
const char BCMATTACHDATA(rstr_rssicorrnorm)[] = "rssicorrnorm";
const char BCMATTACHDATA(rstr_5g_cga)[] = "5g_cga";
const char BCMATTACHDATA(rstr_2g_cga)[] = "2g_cga";
const char BCMATTACHDATA(rstr_tempthresh)[] = "tempthresh";
const char BCMATTACHDATA(rstr_temps_hysteresis)[] = "temps_hysteresis";
const char BCMATTACHDATA(rstr_phyrxdesens)[] = "phyrxdesens";
const char BCMATTACHDATA(rstr_ldpc)[] = "ldpc";
const char BCMATTACHDATA(rstr_core2slicemap)[] = "core2slicemap";

#ifdef RADIO_HEALTH_CHECK
const char BCMATTACHDATA(rstr_rhc_tempthresh)[] = "rhc_tempthresh";
const char BCMATTACHDATA(rstr_rhc_temp_fail_time)[] = "rhc_temp_fail_time";
#endif /* RADIO_HEALTH_CHECK */

#ifdef WL_PROXDETECT
const char BCMATTACHDATA(rstr_proxd_basekival)[] = "proxd_basekival";
const char BCMATTACHDATA(rstr_proxd_basektval)[] = "proxd_basektval";
const char BCMATTACHDATA(rstr_proxd_80mkval)[] = "proxd_80mkval";
const char BCMATTACHDATA(rstr_proxd_40mkval)[] = "proxd_40mkval";
const char BCMATTACHDATA(rstr_proxd_20mkval)[] = "proxd_20mkval";
const char BCMATTACHDATA(rstr_proxd_2gkval)[] = "proxd_2gkval";
const char BCMATTACHDATA(rstr_proxdi_rate80m)[] = "proxdi_rate80m";
const char BCMATTACHDATA(rstr_proxdi_rate40m)[] = "proxdi_rate40m";
const char BCMATTACHDATA(rstr_proxdi_rate20m)[] = "proxdi_rate20m";
const char BCMATTACHDATA(rstr_proxdi_rate2g)[] = "proxdi_rate2g";
const char BCMATTACHDATA(rstr_proxdt_rate80m)[] = "proxdt_rate80m";
const char BCMATTACHDATA(rstr_proxdt_rate40m)[] = "proxdt_rate40m";
const char BCMATTACHDATA(rstr_proxdt_rate20m)[] = "proxdt_rate20m";
const char BCMATTACHDATA(rstr_proxdt_rate2g)[] = "proxdt_rate2g";
const char BCMATTACHDATA(rstr_proxdi_ack)[] = "proxdi_ack";
const char BCMATTACHDATA(rstr_proxdt_ack)[] = "proxdt_ack";
const char BCMATTACHDATA(rstr_proxd_sub80m40m)[] = "proxd_sub80m40m";
const char BCMATTACHDATA(rstr_proxd_sub80m20m)[] = "proxd_sub80m20m";
const char BCMATTACHDATA(rstr_proxd_sub40m20m)[] = "proxd_sub40m20m";
const char BCMATTACHDATA(rstr_proxd_seq_kval)[] = "proxd_seq_kal";
#endif /* WL_PROXDETECT */

/* reclaim strings that are only used in attach functions */
const char BCMATTACHDATA(rstr_swctrlmap_2g)[]                  = "swctrlmap_2g";
const char BCMATTACHDATA(rstr_swctrlmap_5g)[]                  = "swctrlmap_5g";
const char BCMATTACHDATA(rstr_swctrlmapext_2g)[]               = "swctrlmapext_2g";
const char BCMATTACHDATA(rstr_swctrlmapext_5g)[]               = "swctrlmapext_5g";
const char BCMATTACHDATA(rstr_txswctrlmap_2g)[]                = "txswctrlmap_2g";
const char BCMATTACHDATA(rstr_txswctrlmap_2g_mask)[]           = "txswctrlmap_2g_mask";
const char BCMATTACHDATA(rstr_txswctrlmap_5g)[]                = "txswctrlmap_5g";
const char BCMATTACHDATA(rstr_fem_table_init_val)[]            = "fem_table_init_val";
const char BCMATTACHDATA(rstr_asymmetricjammermod)[]           = "asymmetricjammermod";
const char BCMATTACHDATA(rstr_lesi_en)[] = "lesimode";

/* Used by et module ac layer */
const char BCMATTACHDATA(rstr_et_mode)[] = "etmode";

/* Used by noise module ac specific layer */
const char BCMATTACHDATA(rstr_noiselvl2gaD)[]                  = "noiselvl2ga%d";
const char BCMATTACHDATA(rstr_noiselvl5gaD)[]                  = "noiselvl5ga%d";

/* Radio Band Capability Indicator */
const char BCMATTACHDATA(rstr_bandcap)[] = "bandcap";

/* Used by TPC module for SROM reading */
const char BCMATTACHDATA(rstr_maxp2ga0)[] = "maxp2ga0";
const char BCMATTACHDATA(rstr_pa2ga0)[] = "pa2ga0";
const char BCMATTACHDATA(rstr_maxp2ga1)[] = "maxp2ga1";
const char BCMATTACHDATA(rstr_tssifloor2g)[] = "tssifloor2g";
const char BCMATTACHDATA(rstr_pa2ga1)[] = "pa2ga1";
const char BCMATTACHDATA(rstr_pa2ga2)[] = "pa2ga2";
const char BCMATTACHDATA(rstr_maxp2ga2)[] = "maxp2ga2";
const char BCMATTACHDATA(rstr_pa2ga3)[] = "pa2ga3";
const char BCMATTACHDATA(rstr_pa2gbw40a0)[] = "pa2gbw40a0";
const char BCMATTACHDATA(rstr_pa2g20ccka3)[] = "pa2g20ccka3";
const char BCMATTACHDATA(rstr_pa2g20ccka2)[] = "pa2g20ccka2";
const char BCMATTACHDATA(rstr_pa2g20ccka1)[] = "pa2g20ccka1";
const char BCMATTACHDATA(rstr_pa2g20ccka0)[] = "pa2g20ccka0";
const char BCMATTACHDATA(rstr_pa2gccka1)[] = "pa2gccka1";
const char BCMATTACHDATA(rstr_pa2gccka0)[] = "pa2gccka0";
const char BCMATTACHDATA(rstr_pdoffset40ma0)[]      = "pdoffset40ma0";
const char BCMATTACHDATA(rstr_pdoffset2g40mvalid)[] = "pdoffset2g40mvalid";
const char BCMATTACHDATA(rstr_pdoffset5gsubbanda0)[] = "pdoffset5gsubbanda0";
const char BCMATTACHDATA(rstr_pdoffsetcckma0)[]     = "pdoffsetcckma0";
const char BCMATTACHDATA(rstr_pdoffset80ma0)[]      = "pdoffset80ma0";
const char BCMATTACHDATA(rstr_pdoffset2g40ma0)[]    = "pdoffset2g40ma0";
const char BCMATTACHDATA(rstr_cckulbpwroffset0)[]      = "cckulbpwroffset0";
const char BCMATTACHDATA(rstr_pdoffset40ma1)[]      = "pdoffset40ma1";
const char BCMATTACHDATA(rstr_cckpwroffset0)[]      = "cckpwroffset0";
const char BCMATTACHDATA(rstr_pdoffset5gsubbanda1)[] = "pdoffset5gsubbanda1";
const char BCMATTACHDATA(rstr_pdoffsetcckma1)[]     = "pdoffsetcckma1";
const char BCMATTACHDATA(rstr_cckulbpwroffset2)[]      = "cckulbpwroffset2";
const char BCMATTACHDATA(rstr_pdoffset2g40ma1)[]    = "pdoffset2g40ma1";
const char BCMATTACHDATA(rstr_cckulbpwroffset1)[]      = "cckulbpwroffset1";
const char BCMATTACHDATA(rstr_pdoffset80ma2)[]      = "pdoffset80ma2";
const char BCMATTACHDATA(rstr_cckpwroffset1)[]      = "cckpwroffset1";
const char BCMATTACHDATA(rstr_pdoffset40ma2)[]      = "pdoffset40ma2";
const char BCMATTACHDATA(rstr_pdoffsetcckma2)[]     = "pdoffsetcckma2";
const char BCMATTACHDATA(rstr_pdoffset2g40ma2)[]    = "pdoffset2g40ma2";
const char BCMATTACHDATA(rstr_cckpwroffset2)[]      = "cckpwroffset2";
const char BCMATTACHDATA(rstr_tempoffset)[] = "tempoffset";
const char BCMATTACHDATA(rstr_pa2g40a0)[] = "pa2g40a0";
const char BCMATTACHDATA(rstr_pa2g40a1)[] = "pa2g40a1";
const char BCMATTACHDATA(rstr_pa2g40a2)[] = "pa2g40a2";
const char BCMATTACHDATA(rstr_pa2g40a3)[] = "pa2g40a3";
const char BCMATTACHDATA(rstr_pa5ga0)[] = "pa5ga0";
const char BCMATTACHDATA(rstr_pa5g40a0)[] = "pa5g40a0";
const char BCMATTACHDATA(rstr_pa5g80a0)[] = "pa5g80a0";
const char BCMATTACHDATA(rstr_pa5g160a0)[] = "pa5g160a0";
const char BCMATTACHDATA(rstr_pa5ga1)[] = "pa5ga1";
const char BCMATTACHDATA(rstr_pa5g40a1)[] = "pa5g40a1";
const char BCMATTACHDATA(rstr_pa5g80a1)[] = "pa5g80a1";
const char BCMATTACHDATA(rstr_pa5g160a1)[] = "pa5g160a1";
const char BCMATTACHDATA(rstr_pa5ga2)[] = "pa5ga2";

const char BCMATTACHDATA(rstr_maxp5ga0)[] = "maxp5ga0";
const char BCMATTACHDATA(rstr_tssifloor5g)[] = "tssifloor5g";
const char BCMATTACHDATA(rstr_maxp5ga1)[] = "maxp5ga1";
const char BCMATTACHDATA(rstr_maxp5ga2)[] = "maxp5ga2";
const char BCMATTACHDATA(rstr_pa5ga3)[] = "pa5ga3";
const char BCMATTACHDATA(rstr_pa5gbw40a0)[] = "pa5gbw40a0";
const char BCMATTACHDATA(rstr_pa5gbw80a0)[] = "pa5gbw80a0";
const char BCMATTACHDATA(rstr_pa5gbw4080a0)[] = "pa5gbw4080a0";
const char BCMATTACHDATA(rstr_pa5gbw4080a1)[] = "pa5gbw4080a1";

const char BCMATTACHDATA(rstr_txpwr2gAdcScale)[] = "txpwr2gAdcScale";
const char BCMATTACHDATA(rstr_txpwr5gAdcScale)[] = "txpwr5gAdcScale";

const char BCMATTACHDATA(rstr_cckbw202gpo)[] = "cckbw202gpo";
const char BCMATTACHDATA(rstr_cckbw20ul2gpo)[] = "cckbw20ul2gpo";
const char BCMATTACHDATA(rstr_ofdmlrbw202gpo)[] = "ofdmlrbw202gpo";
const char BCMATTACHDATA(rstr_dot11agofdmhrbw202gpo)[] = "dot11agofdmhrbw202gpo";
const char BCMATTACHDATA(rstr_mcsbw202gpo)[] = "mcsbw202gpo";
const char BCMATTACHDATA(rstr_mcsbw402gpo)[] = "mcsbw402gpo";
const char BCMATTACHDATA(rstr_sb20in40lrpo)[] = "sb20in40lrpo";
const char BCMATTACHDATA(rstr_sb20in40hrpo)[] = "sb20in40hrpo";
const char BCMATTACHDATA(rstr_dot11agduphrpo)[] = "dot11agduphrpo";
const char BCMATTACHDATA(rstr_dot11agduplrpo)[] = "dot11agduplrpo";
const char BCMATTACHDATA(rstr_mcsbw205glpo)[] = "mcsbw205glpo";
const char BCMATTACHDATA(rstr_mcsbw405glpo)[] = "mcsbw405glpo";
const char BCMATTACHDATA(rstr_mcsbw205gmpo)[] = "mcsbw205gmpo";
const char BCMATTACHDATA(rstr_mcsbw405gmpo)[] = "mcsbw405gmpo";
const char BCMATTACHDATA(rstr_mcsbw205ghpo)[] = "mcsbw205ghpo";
const char BCMATTACHDATA(rstr_mcsbw405ghpo)[] = "mcsbw405ghpo";

const char BCMATTACHDATA(rstr_mcslr5glpo)[] = "mcslr5glpo";
const char BCMATTACHDATA(rstr_mcslr5gmpo)[] = "mcslr5gmpo";
const char BCMATTACHDATA(rstr_mcslr5ghpo)[] = "mcslr5ghpo";
const char BCMATTACHDATA(rstr_mcsbw805glpo)[] = "mcsbw805glpo";
const char BCMATTACHDATA(rstr_mcsbw805gmpo)[] = "mcsbw805gmpo";

const char BCMATTACHDATA(rstr_mcsbw805ghpo)[] = "mcsbw805ghpo";
const char BCMATTACHDATA(rstr_sb20in80and160lr5glpo)[] = "sb20in80and160lr5glpo";
const char BCMATTACHDATA(rstr_sb20in80and160hr5glpo)[] = "sb20in80and160hr5glpo";
const char BCMATTACHDATA(rstr_sb20in80and160lr5gmpo)[] = "sb20in80and160lr5gmpo";
const char BCMATTACHDATA(rstr_sb20in80and160hr5gmpo)[] = "sb20in80and160hr5gmpo";
const char BCMATTACHDATA(rstr_sb20in80and160lr5ghpo)[] = "sb20in80and160lr5ghpo";
const char BCMATTACHDATA(rstr_sb20in80and160hr5ghpo)[] = "sb20in80and160hr5ghpo";
const char BCMATTACHDATA(rstr_sb40and80lr5glpo)[] = "sb40and80lr5glpo";
const char BCMATTACHDATA(rstr_sb40and80hr5glpo)[] = "sb40and80hr5glpo";
const char BCMATTACHDATA(rstr_sb40and80lr5gmpo)[] = "sb40and80lr5gmpo";
const char BCMATTACHDATA(rstr_sb40and80hr5gmpo)[] = "sb40and80hr5gmpo";
const char BCMATTACHDATA(rstr_sb40and80lr5ghpo)[] = "sb40and80lr5ghpo";
const char BCMATTACHDATA(rstr_sb40and80hr5ghpo)[] = "sb40and80hr5ghpo";

const char BCMATTACHDATA(rstr_mcsbw205gx1po)[] = "mcsbw205gx1po";
const char BCMATTACHDATA(rstr_mcsbw405gx1po)[] = "mcsbw405gx1po";
const char BCMATTACHDATA(rstr_mcsbw205gx2po)[] = "mcsbw205gx2po";
const char BCMATTACHDATA(rstr_mcsbw405gx2po)[] = "mcsbw405gx2po";
const char BCMATTACHDATA(rstr_mcslr5gx1po)[] = "mcslr5gx1po";
const char BCMATTACHDATA(rstr_mcslr5gx2po)[] = "mcslr5gx2po";
const char BCMATTACHDATA(rstr_mcsbw805gx1po)[] = "mcsbw805gx1po";
const char BCMATTACHDATA(rstr_mcsbw805gx2po)[] = "mcsbw805gx2po";
const char BCMATTACHDATA(rstr_mcsbw1605gx1po)[] = "mcsbw1605gx1po";
const char BCMATTACHDATA(rstr_mcsbw1605gx2po)[] = "mcsbw1605gx2po";
const char BCMATTACHDATA(rstr_sb20in80and160lr5gx1po)[] = "sb20in80and160lr5gx1po";
const char BCMATTACHDATA(rstr_sb20in80and160hr5gx1po)[] = "sb20in80and160hr5gx1po";
const char BCMATTACHDATA(rstr_sb20in80and160lr5gx2po)[] = "sb20in80and160lr5gx2po";
const char BCMATTACHDATA(rstr_sb20in80and160hr5gx2po)[] = "sb20in80and160hr5gx2po";
const char BCMATTACHDATA(rstr_sb40and80lr5gx1po)[] = "sb40and80lr5gx1po";
const char BCMATTACHDATA(rstr_sb40and80hr5gx1po)[] = "sb40and80hr5gx1po";
const char BCMATTACHDATA(rstr_sb40and80lr5gx2po)[] = "sb40and80lr5gx2po";
const char BCMATTACHDATA(rstr_sb40and80hr5gx2po)[] = "sb40and80hr5gx2po";
const char BCMATTACHDATA(rstr_mcsbw1605glpo)[] = "mcsbw1605glpo";
const char BCMATTACHDATA(rstr_mcsbw1605gmpo)[] = "mcsbw1605gmpo";
const char BCMATTACHDATA(rstr_mcsbw1605ghpo)[] = "mcsbw1605ghpo";

/* 1024qam PPR parameters */
const char BCMATTACHDATA(rstr_mcs1024qam2gpo)[] = "mcs1024qam2gpo";
const char BCMATTACHDATA(rstr_mcs1024qam5glpo)[] = "mcs1024qam5glpo";
const char BCMATTACHDATA(rstr_mcs1024qam5gmpo)[] = "mcs1024qam5gmpo";
const char BCMATTACHDATA(rstr_mcs1024qam5ghpo)[] = "mcs1024qam5ghpo";
const char BCMATTACHDATA(rstr_mcs1024qam5gx1po)[] = "mcs1024qam5gx1po";
const char BCMATTACHDATA(rstr_mcs1024qam5gx2po)[] = "mcs1024qam5gx2po";

const char BCMATTACHDATA(rstr_mcs8poexp)[] = "mcs8poexp";
const char BCMATTACHDATA(rstr_mcs9poexp)[] = "mcs9poexp";
const char BCMATTACHDATA(rstr_mcs10poexp)[] = "mcs10poexp";
const char BCMATTACHDATA(rstr_mcs11poexp)[] = "mcs11poexp";

const char BCMATTACHDATA(rstr_pdoffset80ma1)[]      = "pdoffset80ma1";
const char BCMATTACHDATA(rstr_pa5g40a2)[] = "pa5g40a2";
const char BCMATTACHDATA(rstr_pa5g80a2)[] = "pa5g80a2";
const char BCMATTACHDATA(rstr_pa5g160a2)[] = "pa5g160a2";
const char BCMATTACHDATA(rstr_pa5g40a3)[] = "pa5g40a3";
const char BCMATTACHDATA(rstr_pa5g80a3)[] = "pa5g80a3";
const char BCMATTACHDATA(rstr_pa5g160a3)[] = "pa5g160a3";
const char BCMATTACHDATA(rstr_maxp2gb0a0)[] = "maxp2ga0";
const char BCMATTACHDATA(rstr_maxp5gb0a0)[] = "maxp5gb0a0";
const char BCMATTACHDATA(rstr_maxp5gb1a0)[] = "maxp5gb1a0";
const char BCMATTACHDATA(rstr_maxp5gb2a0)[] = "maxp5gb2a0";
const char BCMATTACHDATA(rstr_maxp5gb3a0)[] = "maxp5gb3a0";
const char BCMATTACHDATA(rstr_maxp5gb4a0)[] = "maxp5gb4a0";
const char BCMATTACHDATA(rstr_maxp2gb0a1)[] = "maxp2ga1";
const char BCMATTACHDATA(rstr_maxp5gb0a1)[] = "maxp5gb0a1";
const char BCMATTACHDATA(rstr_maxp5gb1a1)[] = "maxp5gb1a1";
const char BCMATTACHDATA(rstr_maxp5gb2a1)[] = "maxp5gb2a1";
const char BCMATTACHDATA(rstr_maxp5gb3a1)[] = "maxp5gb3a1";
const char BCMATTACHDATA(rstr_maxp5gb4a1)[] = "maxp5gb4a1";
const char BCMATTACHDATA(rstr_maxp2gb0a2)[] = "maxp2ga2";
const char BCMATTACHDATA(rstr_maxp5gb0a2)[] = "maxp5gb0a2";
const char BCMATTACHDATA(rstr_maxp5gb1a2)[] = "maxp5gb1a2";
const char BCMATTACHDATA(rstr_maxp5gb2a2)[] = "maxp5gb2a2";
const char BCMATTACHDATA(rstr_maxp5gb3a2)[] = "maxp5gb3a2";
const char BCMATTACHDATA(rstr_maxp5gb4a2)[] = "maxp5gb4a2";
const char BCMATTACHDATA(rstr_maxp2gb0a3)[] = "maxp2ga3";
const char BCMATTACHDATA(rstr_maxp5gb0a3)[] = "maxp5gb0a3";
const char BCMATTACHDATA(rstr_maxp5gb1a3)[] = "maxp5gb1a3";
const char BCMATTACHDATA(rstr_maxp5gb2a3)[] = "maxp5gb2a3";
const char BCMATTACHDATA(rstr_maxp5gb3a3)[] = "maxp5gb3a3";
const char BCMATTACHDATA(rstr_maxp5gb4a3)[] = "maxp5gb4a3";
const char BCMATTACHDATA(rstr_pdoffset2gcck)[]     = "pdoffsetcck";
const char BCMATTACHDATA(rstr_pdoffset2gcck20m)[]    = "pdoffsetcck20m";
const char BCMATTACHDATA(rstr_pdoffset20in40m5gb0)[] = "pdoffset20in40m5gb0";
const char BCMATTACHDATA(rstr_pdoffset20in40m5gb1)[] = "pdoffset20in40m5gb1";
const char BCMATTACHDATA(rstr_pdoffset20in40m5gb2)[] = "pdoffset20in40m5gb2";
const char BCMATTACHDATA(rstr_pdoffset20in40m5gb3)[] = "pdoffset20in40m5gb3";
const char BCMATTACHDATA(rstr_pdoffset20in40m5gb4)[] = "pdoffset20in40m5gb4";
const char BCMATTACHDATA(rstr_pdoffset20in80m5gb0)[] = "pdoffset20in80m5gb0";
const char BCMATTACHDATA(rstr_pdoffset20in80m5gb1)[] = "pdoffset20in80m5gb1";
const char BCMATTACHDATA(rstr_pdoffset20in80m5gb2)[] = "pdoffset20in80m5gb2";
const char BCMATTACHDATA(rstr_pdoffset20in80m5gb3)[] = "pdoffset20in80m5gb3";
const char BCMATTACHDATA(rstr_pdoffset20in80m5gb4)[] = "pdoffset20in80m5gb4";
const char BCMATTACHDATA(rstr_pdoffset40in80m5gb0)[] = "pdoffset40in80m5gb0";
const char BCMATTACHDATA(rstr_pdoffset40in80m5gb1)[] = "pdoffset40in80m5gb1";
const char BCMATTACHDATA(rstr_pdoffset40in80m5gb2)[] = "pdoffset40in80m5gb2";
const char BCMATTACHDATA(rstr_pdoffset40in80m5gb3)[] = "pdoffset40in80m5gb3";
const char BCMATTACHDATA(rstr_pdoffset40in80m5gb4)[] = "pdoffset40in80m5gb4";
const char BCMATTACHDATA(rstr_pdoffset20in160m5gc0)[] = "pdoffset20in160m5gc0";
const char BCMATTACHDATA(rstr_pdoffset20in160m5gc1)[] = "pdoffset20in160m5gc1";
const char BCMATTACHDATA(rstr_pdoffset20in160m5gc2)[] = "pdoffset20in160m5gc2";
const char BCMATTACHDATA(rstr_pdoffset20in160m5gc3)[] = "pdoffset20in160m5gc3";
const char BCMATTACHDATA(rstr_pdoffset40in160m5gc0)[] = "pdoffset40in160m5gc0";
const char BCMATTACHDATA(rstr_pdoffset40in160m5gc1)[] = "pdoffset40in160m5gc1";
const char BCMATTACHDATA(rstr_pdoffset40in160m5gc2)[] = "pdoffset40in160m5gc2";
const char BCMATTACHDATA(rstr_pdoffset40in160m5gc3)[] = "pdoffset40in160m5gc3";
const char BCMATTACHDATA(rstr_pdoffset80in160m5gc0)[] = "pdoffset80in160m5gc0";
const char BCMATTACHDATA(rstr_pdoffset80in160m5gc1)[] = "pdoffset80in160m5gc1";
const char BCMATTACHDATA(rstr_pdoffset80in160m5gc2)[] = "pdoffset80in160m5gc2";
const char BCMATTACHDATA(rstr_pdoffset80in160m5gc3)[] = "pdoffset80in160m5gc3";
const char BCMATTACHDATA(rstr_pdoffset2g20in20a0)[] = "pdoffset2g20in20a0";
const char BCMATTACHDATA(rstr_pdoffset2g20in20a1)[] = "pdoffset2g20in20a1";
const char BCMATTACHDATA(rstr_pdoffset2g20in20a2)[] = "pdoffset2g20in20a2";
const char BCMATTACHDATA(rstr_pdoffset20in40m5gcore3)[]   = "pdoffset20in40m5gcore3";
const char BCMATTACHDATA(rstr_pdoffset20in40m5gcore3_1)[] = "pdoffset20in40m5gcore3_1";
const char BCMATTACHDATA(rstr_pdoffset20in80m5gcore3)[]   = "pdoffset20in80m5gcore3";
const char BCMATTACHDATA(rstr_pdoffset20in80m5gcore3_1)[] = "pdoffset20in80m5gcore3_1";
const char BCMATTACHDATA(rstr_pdoffset40in80m5gcore3)[]   = "pdoffset40in80m5gcore3";
const char BCMATTACHDATA(rstr_pdoffset40in80m5gcore3_1)[] = "pdoffset40in80m5gcore3_1";
const char BCMATTACHDATA(rstr_pdoffset20in40m2g)[]        = "pdoffset20in40m2g";
const char BCMATTACHDATA(rstr_pdoffset20in40m2gcore3)[]   = "pdoffset20in40m2gcore3";

const char BCMATTACHDATA(rstr_pdoffset20in160m5gcore3)[]   = "pdoffset20in160m5gcore3";
const char BCMATTACHDATA(rstr_pdoffset20in160m5gcore3_1)[] = "pdoffset20in160m5gcore3_1";
const char BCMATTACHDATA(rstr_pdoffset40in160m5gcore3)[]   = "pdoffset40in160m5gcore3";
const char BCMATTACHDATA(rstr_pdoffset40in160m5gcore3_1)[] = "pdoffset40in160m5gcore3_1";
const char BCMATTACHDATA(rstr_pdoffset80in160m5gcore3)[]   = "pdoffset80in160m5gcore3";
const char BCMATTACHDATA(rstr_pdoffset80in160m5gcore3_1)[] = "pdoffset80in160m5gcore3_1";

const char BCMATTACHDATA(rstr_clb2gslice0core0)[]              = "clb2gslice0core0";
const char BCMATTACHDATA(rstr_clb2gslice1core0)[]              = "clb2gslice1core0";
const char BCMATTACHDATA(rstr_clb5gslice0core0)[]              = "clb5gslice0core0";
const char BCMATTACHDATA(rstr_clb5gslice1core0)[]              = "clb5gslice1core0";
const char BCMATTACHDATA(rstr_clb2gslice0core1)[]              = "clb2gslice0core1";
const char BCMATTACHDATA(rstr_clb2gslice1core1)[]              = "clb2gslice1core1";
const char BCMATTACHDATA(rstr_clb5gslice0core1)[]              = "clb5gslice0core1";
const char BCMATTACHDATA(rstr_clb5gslice1core1)[]              = "clb5gslice1core1";
const char BCMATTACHDATA(rstr_btc_prisel_mask)[]               = "btc_prisel_mask";
const char BCMATTACHDATA(rstr_btc_prisel_ant_mask)[]           = "btc_prisel_ant_mask";
const char BCMATTACHDATA(rstr_clb_swctrl_smask_ant0)[]         = "clb_swctrl_smask_ant0";
const char BCMATTACHDATA(rstr_clb_swctrl_smask_ant1)[]         = "clb_swctrl_smask_ant1";

/* ACPHY MISC params */
const char BCMATTACHDATA(rstr_rawtempsense)[]                  = "rawtempsense";
const char BCMATTACHDATA(rstr_rxgainerr2ga0)[]                 = "rxgainerr2ga0";
const char BCMATTACHDATA(rstr_rxgainerr2ga1)[]                 = "rxgainerr2ga1";
const char BCMATTACHDATA(rstr_rxgainerr2ga2)[]                 = "rxgainerr2ga2";
const char BCMATTACHDATA(rstr_rxgainerr2ga3)[]                 = "rxgainerr2ga3";
const char BCMATTACHDATA(rstr_rxgainerr5ga0)[]                 = "rxgainerr5ga0";
const char BCMATTACHDATA(rstr_rxgainerr5ga1)[]                 = "rxgainerr5ga1";
const char BCMATTACHDATA(rstr_rxgainerr5ga2)[]                 = "rxgainerr5ga2";
const char BCMATTACHDATA(rstr_rxgainerr5ga3)[]                 = "rxgainerr5ga3";

/* ACPHY radio param */
const char BCMATTACHDATA(rstr_use5gpllfor2g)[] = "use5gpllfor2g";

/* ACPHY RSSI params */
const char BCMATTACHDATA(rstr_rxgaintempcoeff2g)[]             = "rstr_rxgaintempcoeff2g";
const char BCMATTACHDATA(rstr_rxgaintempcoeff5gl)[]            = "rstr_rxgaintempcoeff5gl";
const char BCMATTACHDATA(rstr_rxgaintempcoeff5gml)[]           = "rstr_rxgaintempcoeff5gml";
const char BCMATTACHDATA(rstr_rxgaintempcoeff5gmu)[]           = "rstr_rxgaintempcoeff5gmu";
const char BCMATTACHDATA(rstr_rxgaintempcoeff5gh)[]            = "rstr_rxgaintempcoeff5gh";
const char BCMATTACHDATA(rstr_rssicorrnorm_cD)[]               = "rssicorrnorm_c%d";
const char BCMATTACHDATA(rstr_rssicorrnorm5g_cD)[]             = "rssicorrnorm5g_c%d";
const char BCMATTACHDATA(rstr_rssi_delta_2g_cD)[]              = "rssi_delta_2g_c%d";
const char BCMATTACHDATA(rstr_rssi_delta_5gS_cD)[]             = "rssi_delta_5g%s_c%d";
const char BCMATTACHDATA(rstr_gain_cal_temp)[]                 = "gain_cal_temp";
const char BCMATTACHDATA(rstr_rssi_cal_rev)[]                  = "rssi_cal_rev";
const char BCMATTACHDATA(rstr_rssi_qdB_en)[]                   = "rssi_qdB_en";
const char BCMATTACHDATA(rstr_rxgaincal_rssical)[]             = "rxgaincal_rssical";
const char BCMATTACHDATA(rstr_rud_agc_enable)[]                = "rud_agc_enable";
const char BCMATTACHDATA(rstr_rssi_delta_2gS)[]                = "rssi_delta_2g%s";
const char BCMATTACHDATA(rstr_rssi_delta_5gS)[]                = "rssi_delta_5g%s";
const char BCMATTACHDATA(rstr_rssi_cal_freq_grp_2g)[]          = "rssi_cal_freq_grp_2g";
const char BCMATTACHDATA(rstr_num_rssi_cal_gi_2g)[]               = "num_rssi_cal_gi_2g";
const char BCMATTACHDATA(rstr_num_rssi_cal_gi_5g)[]               = "num_rssi_cal_gi_5g";
const char BCMATTACHDATA(rstr_rssi_lna1_routadj_en_5g)[]              = "rssi_lna1_routadj_en_5g";
const char BCMATTACHDATA(rstr_dot11b_opts)[]                   = "rstr_dot11b_opts";
const char BCMATTACHDATA(rstr_tiny_maxrxgain)[]                = "rstr_tiny_maxrxgain";

#ifdef POWPERCHANNL
const char BCMATTACHDATA(rstr_PowOffs2GTNA0)[] = "powoffs2gtna0";
const char BCMATTACHDATA(rstr_PowOffs2GTNA1)[] = "powoffs2gtna1";
const char BCMATTACHDATA(rstr_PowOffs2GTNA2)[] = "powoffs2gtna2";
const char BCMATTACHDATA(rstr_PowOffs2GTLA0)[] = "powoffs2gtla0";
const char BCMATTACHDATA(rstr_PowOffs2GTLA1)[] = "powoffs2gtla1";
const char BCMATTACHDATA(rstr_PowOffs2GTLA2)[] = "powoffs2gtla2";
const char BCMATTACHDATA(rstr_PowOffs2GTHA0)[] = "powoffs2gtha0";
const char BCMATTACHDATA(rstr_PowOffs2GTHA1)[] = "powoffs2gtha1";
const char BCMATTACHDATA(rstr_PowOffs2GTHA2)[] = "powoffs2gtha2";
const char BCMATTACHDATA(rstr_PowOffsTempRange)[] = "powoffstemprange";
#endif /* POWPERCHANNL */

/* reclaim strings that are only used in attach functions */
const char BCMATTACHDATA(rstr_pagc2g)[] = "pagc2g";
const char BCMATTACHDATA(rstr_sw_txchain_mask)[] = "sw_txchain_mask";
const char BCMATTACHDATA(rstr_sw_rxchain_mask)[] = "sw_rxchain_mask";
const char BCMATTACHDATA(rstr_pagc5g)[] = "pagc5g";
const char BCMATTACHDATA(rstr_rpcal2g)[] = "rpcal2g";
const char BCMATTACHDATA(rstr_rpcal2gcore3)[] = "rpcal2gcore3";
const char BCMATTACHDATA(rstr_femctrl)[] = "femctrl";
const char BCMATTACHDATA(rstr_papdmode)[] = "papdmode";
const char BCMATTACHDATA(rstr_pdgain2g)[] = "pdgain2g";
const char BCMATTACHDATA(rstr_pdgain5g)[] = "pdgain5g";
const char BCMATTACHDATA(rstr_epacal2g)[] = "epacal2g";
const char BCMATTACHDATA(rstr_epacal5g)[] = "epacal5g";
const char BCMATTACHDATA(rstr_itrsw)[] = "itrsw";
const char BCMATTACHDATA(rstr_offtgpwr)[] = "offtgpwr";
const char BCMATTACHDATA(rstr_epagain2g)[] = "epagain2g";
const char BCMATTACHDATA(rstr_epagain5g)[] = "epagain5g";
const char BCMATTACHDATA(rstr_rpcal5gb0)[] = "rpcal5gb0";
const char BCMATTACHDATA(rstr_rpcal5gb1)[] = "rpcal5gb1";
const char BCMATTACHDATA(rstr_rpcal5gb2)[] = "rpcal5gb2";
const char BCMATTACHDATA(rstr_rpcal5gb3)[] = "rpcal5gb3";
const char BCMATTACHDATA(rstr_rpcal5gb0core3)[] = "rpcal5gb0core3";
const char BCMATTACHDATA(rstr_rpcal5gb1core3)[] = "rpcal5gb1core3";
const char BCMATTACHDATA(rstr_rpcal5gb2core3)[] = "rpcal5gb2core3";
const char BCMATTACHDATA(rstr_rpcal5gb3core3)[] = "rpcal5gb3core3";
const char BCMATTACHDATA(rstr_txidxcap2g)[] = "txidxcap2g";
const char BCMATTACHDATA(rstr_txidxcap5g)[] = "txidxcap5g";
const char BCMATTACHDATA(rstr_txidxmincap2g)[] = "txidxmincap2g";
const char BCMATTACHDATA(rstr_txidxmincap5g)[] = "txidxmincap5g";
const char BCMATTACHDATA(rstr_txidxcaplow)[] = "txidxcaplow";
const char BCMATTACHDATA(rstr_maxepagain)[] = "maxepagain";
const char BCMATTACHDATA(rstr_maxchipoutpower)[] = "maxchipoutpower";
const char BCMATTACHDATA(rstr_extpagain2g)[] = "extpagain2g";
const char BCMATTACHDATA(rstr_extpagain5g)[] = "extpagain5g";
const char BCMATTACHDATA(rstr_boardflags3)[] = "boardflags3";
const char BCMATTACHDATA(rstr_txiqcalidx2g)[] = "txiqcalidx2g";
const char BCMATTACHDATA(rstr_txiqcalidx5g)[] = "txiqcalidx5g";
const char BCMATTACHDATA(rstr_txgaintbl5g)[] = "txgaintbl5g";
const char BCMATTACHDATA(rstr_pwrdampingen)[] = "pwrdampingen";
const char BCMATTACHDATA(rstr_subband5gver)[] = "subband5gver";
const char BCMATTACHDATA(rstr_dacratemode2g)[] = "dacratemode2g";
const char BCMATTACHDATA(rstr_dacratemode5g)[] = "dacratemode5g";
const char BCMATTACHDATA(rstr_paprrmcsgamma2g)[] = "paprrmcsgamma2g";
const char BCMATTACHDATA(rstr_paprrmcsgain2g)[] = "paprrmcsgain2g";
const char BCMATTACHDATA(rstr_paprrmcsgamma5g20)[] = "paprrmcsgamma5g20";
const char BCMATTACHDATA(rstr_paprrmcsgamma5g40)[] = "paprrmcsgamma5g40";
const char BCMATTACHDATA(rstr_paprrmcsgamma5g80)[] = "paprrmcsgamma5g80";
const char BCMATTACHDATA(rstr_paprrmcsgain5g20)[] = "paprrmcsgain5g20";
const char BCMATTACHDATA(rstr_paprrmcsgain5g40)[] = "paprrmcsgain5g40";
const char BCMATTACHDATA(rstr_paprrmcsgain5g80)[] = "paprrmcsgain5g80";
const char BCMATTACHDATA(rstr_paprrmcsgamma2g_ch13)[] = "paprrmcsgamma2g_ch13";
const char BCMATTACHDATA(rstr_paprrmcsgamma2g_ch1)[] = "paprrmcsgamma2g_ch1";
const char BCMATTACHDATA(rstr_paprrmcsgain2g_ch13)[] = "paprrmcsgain2g_ch13";
const char BCMATTACHDATA(rstr_paprrmcsgain2g_ch1)[] = "paprrmcsgain2g_ch1";
const char BCMATTACHDATA(rstr_oob_gaint)[] = "useoobgaint";
const char BCMATTACHDATA(rstr_vcodivmode)[] = "vcodivmode";
const char BCMATTACHDATA(rstr_fdss_interp_en)[] = "fdss_interp_en";
const char BCMATTACHDATA(rstr_fdss_level_2g)[] = "fdss_level_2g";
const char BCMATTACHDATA(rstr_fdss_level_5g)[] = "fdss_level_5g";
const char BCMATTACHDATA(rstr_fdss_bandedge_2g_en)[] = "fdss_bandedge_2g_en";
const char BCMATTACHDATA(rstr_fdss_level_2g_ch13)[] = "fdss_level_2g_ch13";
const char BCMATTACHDATA(rstr_fdss_level_2g_ch1)[] = "fdss_level_2g_ch1";
const char BCMATTACHDATA(rstr_ldo3p3_voltage)[] = "ldo3p3_voltage";
const char BCMATTACHDATA(rstr_paldo3p3_voltage)[] = "paldo3p3_voltage";
const char BCMATTACHDATA(rstr_epacal2g_mask)[] = "epacal2g_mask";
const char BCMATTACHDATA(rstr_cckdigfilttype)[] = "cckdigfilttype";
const char BCMATTACHDATA(rstr_ofdmfilttype_5gbe)[] = "ofdmfilttype_5gbe";
const char BCMATTACHDATA(rstr_ofdmfilttype_2gbe)[] = "ofdmfilttype_2gbe";
const char BCMATTACHDATA(rstr_tworangetssi2g)[] = "tworangetssi2g";
const char BCMATTACHDATA(rstr_tworangetssi5g)[] = "tworangetssi5g";
const char BCMATTACHDATA(rstr_lowpowerrange2g)[] = "lowpowerrange2g";
const char BCMATTACHDATA(rstr_lowpowerrange5g)[] = "lowpowerrange5g";
const char BCMATTACHDATA(rstr_paprdis)[] = "paprdis";
const char BCMATTACHDATA(rstr_papdwar)[] = "papdwar";
const char BCMATTACHDATA(rstr_low_adc_rate_en)[] = "low_adc_rate_en";
const char BCMATTACHDATA(rstr_bphymrc_en)[] = "bphymrc";

const char BCMATTACHDATA(rstr_tssisleep_en)[] = "tssisleep_en";
const char BCMATTACHDATA(ed_thresh2g)[] = "ed_thresh2g";
const char BCMATTACHDATA(ed_thresh5g)[] = "ed_thresh5g";
const char BCMATTACHDATA(hwaci_sw_mitigation)[] = "hwaci_sw_mitigation";
const char BCMATTACHDATA(rstr_LTEJ_WAR_en)[] = "LTEJ_WAR_en";
const char BCMATTACHDATA(rstr_thresh_noise_cal)[] = "thresh_noise_cal";
const char BCMATTACHDATA(rstr_bphyscale)[] = "bphyscale";
const char BCMATTACHDATA(rstr_antdiv_rfswctrlpin_a0)[]         = "antdiv_rfswctrlpin_a0";
const char BCMATTACHDATA(rstr_antdiv_rfswctrlpin_a1)[]         = "antdiv_rfswctrlpin_a1";
#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
const char BCMATTACHDATA(rstr_swctrl_to_bt_in_coex)[]          = "swctrl_to_bt_in_coex";
#endif // endif
#if defined(WLC_TXCAL) || (defined(WLOLPC) && !defined(WLOLPC_DISABLED))
const char BCMATTACHDATA(rstr_olpc_thresh)[]                   = "olpc_thresh";
const char BCMATTACHDATA(rstr_olpc_thresh2g)[]                 = "olpc_thresh2g";
const char BCMATTACHDATA(rstr_olpc_thresh5g)[]                 = "olpc_thresh5g";
const char BCMATTACHDATA(rstr_olpc_tempslope2g)[]              = "olpc_tempslope2g";
const char BCMATTACHDATA(rstr_olpc_tempslope5g)[]              = "olpc_tempslope5g";
const char BCMATTACHDATA(rstr_olpc_anchor2g)[]                 = "olpc_anchor2g";
const char BCMATTACHDATA(rstr_olpc_anchor5g)[]                 = "olpc_anchor5g";
const char BCMATTACHDATA(rstr_olpc_idx_in_use)[]               = "olpc_idx_in_use";
const char BCMATTACHDATA(rstr_olpc_offset)[]                   = "olpc_offset";
const char BCMATTACHDATA(rstr_disable_olpc)[]                  = "disable_olpc";
#endif /* WLC_TXCAL || ((WLOLPC) && !(WLOLPC_DISABLED)) */
const char BCMATTACHDATA(rstr_initbaseidx5govrval)[]	      = "initbaseidx5govrval";
const char BCMATTACHDATA(rstr_txpwrindexlimit)[]              = "txpwrindexlimit";
const char BCMATTACHDATA(rstr_initbaseidx2govrval)[]	      = "initbaseidx2govrval";
const char BCMATTACHDATA(rstr_txnospurmod5g)[] = "txnospurmod5g";
const char BCMATTACHDATA(rstr_txnospurmod2g)[] = "txnospurmod2g";
const char BCMATTACHDATA(rstr_txnoBW80ClkSwitch)[] = "txnoBW80ClkSwitch";
const char BCMATTACHDATA(rstr_etmode)[] = "etmode";

const char BCMATTACHDATA(rstr_lpflags)[]                       = "lpflags";

/* FCC power limit control on ch12/13 */
#ifdef FCC_PWR_LIMIT_2G
const char BCMATTACHDATA(rstr_fccpwrch12)[] = "fccpwrch12";
const char BCMATTACHDATA(rstr_fccpwrch13)[] = "fccpwrch13";
const char BCMATTACHDATA(rstr_fccpwroverride)[] = "fccpwroverride";
#endif /* FCC_PWR_LIMIT_2G */

#if (defined(WLTEST) || defined(WLPKTENG))
const char BCMATTACHDATA(rstr_perratedpd2g)[] = "perratedpd2g";
const char BCMATTACHDATA(rstr_perratedpd5g)[] = "perratedpd5g";
#endif // endif

#ifdef WLC_TXFDIQ
const char BCMATTACHDATA(rstr_txfdiqcalenable)[] = "txfdiqcalenable";
#endif // endif

const char BCMATTACHDATA(rstr_w1clipmod)[] = "w1clipmod";

#if defined(WLTEST)
const char BCMATTACHDATA(rstr_cbuck_out)[] = "cbuck_out_mfg";
#else
const char BCMATTACHDATA(rstr_cbuck_out)[] = "cbuck_out";
#endif // endif
const char BCMATTACHDATA(rstr_ldo3p3_2g)[]	= "ldo3p3_2g";
const char BCMATTACHDATA(rstr_ldo3p3_5g)[]	= "ldo3p3_5g";
const char BCMATTACHDATA(rstr_ccktpc_loop_en)[] = "ccktpc_loop_en";
const char BCMATTACHDATA(rstr_csml)[] = "csml";
const char BCMATTACHDATA(rstr_ocl)[] = "ocl";
const char BCMATTACHDATA(rstr_ocl_cm)[] = "ocl_cm";

const char BCMATTACHDATA(rstr_nap_en)[] = "ulpnap";
const char BCMATTACHDATA(rstr_swctrlmap4_cfg)[]               = "swctrlmap4_cfg";
const char BCMATTACHDATA(rstr_swctrlmap4_S2g_fem3to0)[]       = "swctrlmap4_%s2g_fem3to0";
const char BCMATTACHDATA(rstr_swctrlmap4_S5g_fem3to0)[]       = "swctrlmap4_%s5g_fem3to0";
const char BCMATTACHDATA(rstr_swctrlmap4_S2g_fem7to4)[]       = "swctrlmap4_%s2g_fem7to4";
const char BCMATTACHDATA(rstr_swctrlmap4_S5g_fem7to4)[]       = "swctrlmap4_%s5g_fem7to4";
const char BCMATTACHDATA(rstr_swctrlmap4_S2g_fem9to8)[]       = "swctrlmap4_%s2g_fem9to8";
const char BCMATTACHDATA(rstr_swctrlmap4_S5g_fem9to8)[]       = "swctrlmap4_%s5g_fem9to8";
const char BCMATTACHDATA(rstr_gainctrlsph)[] = "gainctrlsph";

/* NVRAM PARAM String for ulp_adc_mode */
const char BCMATTACHDATA(rstr_ulpadc)[] = "ulpadc";
const char BCMATTACHDATA(rstr_spurcan_chlist)[] = "spurcan_ch_list_MHz";
const char BCMATTACHDATA(rstr_spurcan_spfreq)[] = "spurcan_sp_freq_KHz";
const char BCMATTACHDATA(rstr_spurcan_numspur)[] = "spurcan_NumSpur";

const char BCMATTACHDATA(rstr_vcotune)[] = "vcotune";

#ifdef WLC_SW_DIVERSITY
const char BCMATTACHDATA(rstr_swdiv_en)[] = "swdiv_en";
const char BCMATTACHDATA(rstr_swdiv_gpio)[] = "swdiv_gpio";
const char BCMATTACHDATA(rstr_swdiv_gpioctrl)[] = "swdiv_gpioctrl";
const char BCMATTACHDATA(rstr_swdiv_swctrl_en)[] = "swdiv_swctrl_en";
const char BCMATTACHDATA(rstr_swdiv_swctrl_mask)[] = "swdiv_swctrl_mask";
const char BCMATTACHDATA(rstr_swdiv_swctrl_ant0)[] = "swdiv_swctrl_ant0";
const char BCMATTACHDATA(rstr_swdiv_swctrl_ant1)[] = "swdiv_swctrl_ant1";
const char BCMATTACHDATA(rstr_swdiv_coreband_map)[] = "swdiv_coreband_map";
const char BCMATTACHDATA(rstr_swdiv_antmap2g_main)[] = "swdiv_antmap2g_main";
const char BCMATTACHDATA(rstr_swdiv_antmap5g_main)[] = "swdiv_antmap5g_main";
const char BCMATTACHDATA(rstr_swdiv_antmap2g_aux)[] = "swdiv_antmap2g_aux";
const char BCMATTACHDATA(rstr_swdiv_antmap5g_aux)[] = "swdiv_antmap5g_aux";
#endif /* WLC_SW_DIVERSITY */

const char BCMATTACHDATA(rstr_elna2g)[] = "elna2g";
const char BCMATTACHDATA(rstr_elna5g)[] = "elna5g";
const char BCMATTACHDATA(rstr_aa2g)[] = "aa2g";
const char BCMATTACHDATA(rstr_aa5g)[] = "aa5g";
const char BCMATTACHDATA(rstr_tssipos2g)[] = "tssipos2g";
const char BCMATTACHDATA(rstr_pdetrange2g)[] = "pdetrange2g";
const char BCMATTACHDATA(rstr_antswctl2g)[] = "antswctl2g";
const char BCMATTACHDATA(rstr_tssipos5g)[] = "tssipos5g";
const char BCMATTACHDATA(rstr_pdetrange5g)[] = "pdetrange5g";
const char BCMATTACHDATA(rstr_antswctl5g)[] = "antswctl5g";
const char BCMATTACHDATA(rstr_bw40po)[] = "bw40po";
const char BCMATTACHDATA(rstr_cddpo)[] = "cddpo";
const char BCMATTACHDATA(rstr_stbcpo)[] = "stbcpo";
const char BCMATTACHDATA(rstr_bwduppo)[] = "bwduppo";
const char BCMATTACHDATA(rstr_txpid2ga0)[] = "txpid2ga0";
const char BCMATTACHDATA(rstr_txpid2ga1)[] = "txpid2ga1";
const char BCMATTACHDATA(rstr_pa2gw0a2)[] = "pa2gw0a2";
const char BCMATTACHDATA(rstr_pa2gw1a2)[] = "pa2gw1a2";
const char BCMATTACHDATA(rstr_pa2gw2a2)[] = "pa2gw2a2";
const char BCMATTACHDATA(rstr_maxp5gla2)[] = "maxp5gla2";
const char BCMATTACHDATA(rstr_pa5glw0a2)[] = "pa5glw0a2";
const char BCMATTACHDATA(rstr_pa5glw1a2)[] = "pa5glw1a2";
const char BCMATTACHDATA(rstr_pa5glw2a2)[] = "pa5glw2a2";
const char BCMATTACHDATA(rstr_pa5gw0a2)[] = "pa5gw0a2";
const char BCMATTACHDATA(rstr_pa5gw1a2)[] = "pa5gw1a2";
const char BCMATTACHDATA(rstr_pa5gw2a2)[] = "pa5gw2a2";
const char BCMATTACHDATA(rstr_maxp5gha2)[] = "maxp5gha2";
const char BCMATTACHDATA(rstr_pa5ghw0a2)[] = "pa5ghw0a2";
const char BCMATTACHDATA(rstr_pa5ghw1a2)[] = "pa5ghw1a2";
const char BCMATTACHDATA(rstr_pa5ghw2a2)[] = "pa5ghw2a2";
const char BCMATTACHDATA(rstr_pa2gw0a0)[] = "pa2gw0a0";
const char BCMATTACHDATA(rstr_pa2gw0a1)[] = "pa2gw0a1";
const char BCMATTACHDATA(rstr_pa2gw1a0)[] = "pa2gw1a0";
const char BCMATTACHDATA(rstr_pa2gw1a1)[] = "pa2gw1a1";
const char BCMATTACHDATA(rstr_pa2gw2a0)[] = "pa2gw2a0";
const char BCMATTACHDATA(rstr_pa2gw2a1)[] = "pa2gw2a1";
const char BCMATTACHDATA(rstr_itt2ga0)[] = "itt2ga0";
const char BCMATTACHDATA(rstr_itt2ga1)[] = "itt2ga1";
const char BCMATTACHDATA(rstr_cck2gpo)[] = "cck2gpo";
const char BCMATTACHDATA(rstr_ofdm2gpo)[] = "ofdm2gpo";
const char BCMATTACHDATA(rstr_mcs2gpo0)[] = "mcs2gpo0";
const char BCMATTACHDATA(rstr_mcs2gpo1)[] = "mcs2gpo1";
const char BCMATTACHDATA(rstr_mcs2gpo2)[] = "mcs2gpo2";
const char BCMATTACHDATA(rstr_mcs2gpo3)[] = "mcs2gpo3";
const char BCMATTACHDATA(rstr_mcs2gpo4)[] = "mcs2gpo4";
const char BCMATTACHDATA(rstr_mcs2gpo5)[] = "mcs2gpo5";
const char BCMATTACHDATA(rstr_mcs2gpo6)[] = "mcs2gpo6";
const char BCMATTACHDATA(rstr_mcs2gpo7)[] = "mcs2gpo7";
const char BCMATTACHDATA(rstr_txpid5gla0)[] = "txpid5gla0";
const char BCMATTACHDATA(rstr_txpid5gla1)[] = "txpid5gla1";
const char BCMATTACHDATA(rstr_maxp5gla0)[] = "maxp5gla0";
const char BCMATTACHDATA(rstr_maxp5gla1)[] = "maxp5gla1";
const char BCMATTACHDATA(rstr_pa5glw0a0)[] = "pa5glw0a0";
const char BCMATTACHDATA(rstr_pa5glw0a1)[] = "pa5glw0a1";
const char BCMATTACHDATA(rstr_pa5glw1a0)[] = "pa5glw1a0";
const char BCMATTACHDATA(rstr_pa5glw1a1)[] = "pa5glw1a1";
const char BCMATTACHDATA(rstr_pa5glw2a0)[] = "pa5glw2a0";
const char BCMATTACHDATA(rstr_pa5glw2a1)[] = "pa5glw2a1";
const char BCMATTACHDATA(rstr_ofdm5glpo)[] = "ofdm5glpo";
const char BCMATTACHDATA(rstr_mcs5glpo0)[] = "mcs5glpo0";
const char BCMATTACHDATA(rstr_mcs5glpo1)[] = "mcs5glpo1";
const char BCMATTACHDATA(rstr_mcs5glpo2)[] = "mcs5glpo2";
const char BCMATTACHDATA(rstr_mcs5glpo3)[] = "mcs5glpo3";
const char BCMATTACHDATA(rstr_mcs5glpo4)[] = "mcs5glpo4";
const char BCMATTACHDATA(rstr_mcs5glpo5)[] = "mcs5glpo5";
const char BCMATTACHDATA(rstr_mcs5glpo6)[] = "mcs5glpo6";
const char BCMATTACHDATA(rstr_mcs5glpo7)[] = "mcs5glpo7";
const char BCMATTACHDATA(rstr_txpid5ga0)[] = "txpid5ga0";
const char BCMATTACHDATA(rstr_txpid5ga1)[] = "txpid5ga1";
const char BCMATTACHDATA(rstr_pa5gw0a0)[] = "pa5gw0a0";
const char BCMATTACHDATA(rstr_pa5gw0a1)[] = "pa5gw0a1";
const char BCMATTACHDATA(rstr_pa5gw1a0)[] = "pa5gw1a0";
const char BCMATTACHDATA(rstr_pa5gw1a1)[] = "pa5gw1a1";
const char BCMATTACHDATA(rstr_pa5gw2a0)[] = "pa5gw2a0";
const char BCMATTACHDATA(rstr_pa5gw2a1)[] = "pa5gw2a1";
const char BCMATTACHDATA(rstr_itt5ga0)[] = "itt5ga0";
const char BCMATTACHDATA(rstr_itt5ga1)[] = "itt5ga1";
const char BCMATTACHDATA(rstr_ofdm5gpo)[] = "ofdm5gpo";
const char BCMATTACHDATA(rstr_mcs5gpo0)[] = "mcs5gpo0";
const char BCMATTACHDATA(rstr_mcs5gpo1)[] = "mcs5gpo1";
const char BCMATTACHDATA(rstr_mcs5gpo2)[] = "mcs5gpo2";
const char BCMATTACHDATA(rstr_mcs5gpo3)[] = "mcs5gpo3";
const char BCMATTACHDATA(rstr_mcs5gpo4)[] = "mcs5gpo4";
const char BCMATTACHDATA(rstr_mcs5gpo5)[] = "mcs5gpo5";
const char BCMATTACHDATA(rstr_mcs5gpo6)[] = "mcs5gpo6";
const char BCMATTACHDATA(rstr_mcs5gpo7)[] = "mcs5gpo7";
const char BCMATTACHDATA(rstr_txpid5gha0)[] = "txpid5gha0";
const char BCMATTACHDATA(rstr_txpid5gha1)[] = "txpid5gha1";
const char BCMATTACHDATA(rstr_maxp5gha0)[] = "maxp5gha0";
const char BCMATTACHDATA(rstr_maxp5gha1)[] = "maxp5gha1";
const char BCMATTACHDATA(rstr_pa5ghw0a0)[] = "pa5ghw0a0";
const char BCMATTACHDATA(rstr_pa5ghw0a1)[] = "pa5ghw0a1";
const char BCMATTACHDATA(rstr_pa5ghw1a0)[] = "pa5ghw1a0";
const char BCMATTACHDATA(rstr_pa5ghw1a1)[] = "pa5ghw1a1";
const char BCMATTACHDATA(rstr_pa5ghw2a0)[] = "pa5ghw2a0";
const char BCMATTACHDATA(rstr_pa5ghw2a1)[] = "pa5ghw2a1";
const char BCMATTACHDATA(rstr_ofdm5ghpo)[] = "ofdm5ghpo";
const char BCMATTACHDATA(rstr_mcs5ghpo0)[] = "mcs5ghpo0";
const char BCMATTACHDATA(rstr_mcs5ghpo1)[] = "mcs5ghpo1";
const char BCMATTACHDATA(rstr_mcs5ghpo2)[] = "mcs5ghpo2";
const char BCMATTACHDATA(rstr_mcs5ghpo3)[] = "mcs5ghpo3";
const char BCMATTACHDATA(rstr_mcs5ghpo4)[] = "mcs5ghpo4";
const char BCMATTACHDATA(rstr_mcs5ghpo5)[] = "mcs5ghpo5";
const char BCMATTACHDATA(rstr_mcs5ghpo6)[] = "mcs5ghpo6";
const char BCMATTACHDATA(rstr_mcs5ghpo7)[] = "mcs5ghpo7";
const char BCMATTACHDATA(rstr_nonbf_logen_mode_en)[] = "nonbf_logen_mode_en";

const char BCMATTACHDATA(rstr_triso2g)[] = "triso2g";
const char BCMATTACHDATA(rstr_triso5g)[] = "triso5g";
const char BCMATTACHDATA(rstr_pa2gw0a3)[] = "pa2gw0a3";
const char BCMATTACHDATA(rstr_pa2gw1a3)[] = "pa2gw1a3";
const char BCMATTACHDATA(rstr_pa2gw2a3)[] = "pa2gw2a3";
const char BCMATTACHDATA(rstr_maxp5ga3)[] = "maxp5ga3";
const char BCMATTACHDATA(rstr_pa5gw0a3)[] = "pa5gw0a3";
const char BCMATTACHDATA(rstr_pa5gw1a3)[] = "pa5gw1a3";
const char BCMATTACHDATA(rstr_pa5gw2a3)[] = "pa5gw2a3";
const char BCMATTACHDATA(rstr_maxp5gla3)[] = "maxp5gla3";
const char BCMATTACHDATA(rstr_pa5glw2a3)[] = "pa5glw2a3";
const char BCMATTACHDATA(rstr_pa5glw0a3)[] = "pa5glw0a3";
const char BCMATTACHDATA(rstr_pa5glw1a3)[] = "pa5glw1a3";
const char BCMATTACHDATA(rstr_maxp5gha3)[] = "maxp5gha3";
const char BCMATTACHDATA(rstr_pa5ghw0a3)[] = "pa5ghw0a3";
const char BCMATTACHDATA(rstr_pa5ghw1a3)[] = "pa5ghw1a3";
const char BCMATTACHDATA(rstr_pa5ghw2a3)[] = "pa5ghw2a3";

const char BCMATTACHDATA(rstr_legofdmbw202gpo)[] = "legofdmbw202gpo";
const char BCMATTACHDATA(rstr_legofdmbw20ul2gpo)[] = "legofdmbw20ul2gpo";
const char BCMATTACHDATA(rstr_legofdmbw205glpo)[] = "legofdmbw205glpo";
const char BCMATTACHDATA(rstr_legofdmbw20ul5glpo)[] = "legofdmbw20ul5glpo";
const char BCMATTACHDATA(rstr_legofdmbw205gmpo)[] = "legofdmbw205gmpo";
const char BCMATTACHDATA(rstr_legofdmbw20ul5gmpo)[] = "legofdmbw20ul5gmpo";
const char BCMATTACHDATA(rstr_legofdmbw205ghpo)[] = "legofdmbw205ghpo";
const char BCMATTACHDATA(rstr_legofdmbw20ul5ghpo)[] = "legofdmbw20ul5ghpo";
const char BCMATTACHDATA(rstr_mcsbw20ul2gpo)[] = "mcsbw20ul2gpo";
const char BCMATTACHDATA(rstr_mcsbw20ul5glpo)[] = "mcsbw20ul5glpo";
const char BCMATTACHDATA(rstr_mcsbw20ul5gmpo)[] = "mcsbw20ul5gmpo";
const char BCMATTACHDATA(rstr_mcsbw20ul5ghpo)[] = "mcsbw20ul5ghpo";
const char BCMATTACHDATA(rstr_legofdm40duppo)[] = "legofdm40duppo";
