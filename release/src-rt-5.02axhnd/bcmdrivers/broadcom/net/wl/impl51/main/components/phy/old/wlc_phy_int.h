/*
 * PHY module internal interface crossing different PHY types
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
 * $Id: wlc_phy_int.h 764742 2018-05-31 02:57:21Z $
 */

#ifndef _wlc_phy_int_h_
#define _wlc_phy_int_h_

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmsrom_fmt.h>
#include <phy_utils_math.h>

#include "wlc_phy_hal.h"
#include <phy_types.h>
#include <d11.h>

/* *********************************************** */
#include <phy_dbg.h>
#include "phy_type_disp.h"
#include <phy_cmn.h>
#include <phy_init.h>
#include <phy_wd.h>
#include <phy_ana.h>
#include <phy_cache.h>
#include <phy_calmgr.h>
#include <phy_radio.h>
#include <phy_tbl.h>
#include <phy_tpc.h>
#include <phy_txpwrcap.h>
#include <phy_radar.h>
#include <phy_antdiv.h>
#include <phy_noise.h>
#include <phy_rssi.h>
#include <phy_temp.h>
#include <phy_btcx.h>
#include <phy_noise.h>
#include <phy_rxiqcal.h>
#include <phy_txiqlocal.h>
#include <phy_papdcal.h>
#include <phy_vcocal.h>
#include <phy_chanmgr.h>
#include <phy_chanmgr_notif.h>
#include <phy_fcbs.h>
#include <phy_lpc.h>
#include <phy_misc.h>
#include <phy_tssical.h>
#include <phy_rxgcrs.h>
#include <phy_temp_st.h>
#include <phy_rxspur.h>
#include <phy_samp.h>
#include <phy_dsi.h>
#include <phy_mu.h>
#include <phy_dccal.h>
#include <phy_tof.h>
#include <phy_nap.h>
#include <phy_hirssi.h>
#include <phy_ocl.h>
#include <phy_hecap.h>
#include <phy_et.h>
#include <phy_prephy.h>
#include <phy_hc.h>
#include <phy_vasip.h>
#include <phy_smc.h>
#include <phy_stf.h>
#include <phy.h>
#include <phy_api.h>
#include <phy_misc_api.h>
#include <phy_radio_api.h>
#include <phy_calmgr.h>
/* *********************************************** */

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008

#define NUM_MCS_PAPRR_GAMMA 12
#define NUM_FRAME_BEFORE_PWRCTRL_CHANGE 16

#if (defined(ACCONF) && ACCONF) || (defined(ACCONF2) && ACCONF2)
#ifndef PREASSOC_PWRCTRL
//#define PREASSOC_PWRCTRL
#endif // endif
#endif	/* ACCONF || ACCONF2 */

#ifdef BOARD_TYPE
#define BOARDTYPE(_type) BOARD_TYPE
#else
#define BOARDTYPE(_type) _type
#endif // endif

#define NPHY_SROM_TEMPSHIFT		32
#define NPHY_SROM_MAXTEMPOFFSET		16
#define NPHY_SROM_MINTEMPOFFSET		-16

#define PHY_CAL_MAXTEMPDELTA		64

#define ACPHY_SROM_TEMPSHIFT		32
#define ACPHY_SROM_MAXTEMPOFFSET	16
#define ACPHY_SROM_MINTEMPOFFSET	-16

#define LCNXN_BASEREV		16

#define NPHY_BPHY_MIN_SENSITIVITY_REV3TO6 (-95)
#define NPHY_BPHY_MIN_SENSITIVITY_REV7TO15 (-95)

#define NPHY_OFDM_MIN_SENSITIVITY_REV3TO6 (-91)
#define NPHY_OFDM_MIN_SENSITIVITY_REV7TO15 (-91)

#define NPHY_DELTA_MIN_SENSITIVITY_ACI_ON_OFF_REV3TO6 (-5)
#define NPHY_DELTA_MIN_SENSITIVITY_ACI_ON_OFF_REV7TO15 (-5)

#define MAX_VALID_RSSI (-1)
#define TXCAL_OLPC_RECALC_TEMP 10
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  inter-module connection					*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* forward declarations */
struct wlc_hw_info;

typedef void (*initfn_t)(phy_info_t *);
typedef int (*longtrnfn_t)(phy_info_t *, int);
typedef void (*txiqccgetfn_t)(phy_info_t *, uint16 *, uint16 *);
typedef void (*txiqccmimogetfn_t)(phy_info_t *, uint16 *, uint16 *, uint16 *, uint16 *);
typedef void (*txiqccsetfn_t)(phy_info_t *, uint16, uint16);
typedef void (*txiqccmimosetfn_t)(phy_info_t *, uint16, uint16, uint16, uint16);
typedef uint16 (*txloccgetfn_t)(phy_info_t *);
typedef void (*txloccsetfn_t)(phy_info_t *pi, uint16 didq);
typedef void (*txloccmimosetfn_t)(phy_info_t *pi, uint16 diq0, uint16 diq1);
typedef void (*txloccmimogetfn_t)(phy_info_t *, uint16 *, uint16 *);
typedef void (*radioloftgetfn_t)(phy_info_t *, uint8 *, uint8 *, uint8 *, uint8 *);
typedef void (*radioloftsetfn_t)(phy_info_t *, uint8, uint8, uint8, uint8);
typedef void (*radioloftmimogetfn_t)(phy_info_t *, uint8 *, uint8 *, uint8 *,
	uint8 *, uint8 *, uint8 *, uint8 *, uint8 *);
typedef void (*radioloftmimosetfn_t)(phy_info_t *, uint8, uint8, uint8, uint8,
	uint8, uint8, uint8, uint8);
typedef int (*txcorepwroffsetfn_t)(phy_info_t *, struct phy_txcore_pwr_offsets*);
typedef uint16 (*gettxpwrctrlfn_t)(phy_info_t *);
typedef void (*settxpwrbyindexfn_t)(phy_info_t *, int);
typedef void (*phywatchdogfn_t)(phy_info_t *);
typedef uint16 (*tssicalsweepfn_t)(phy_info_t *, int8 *, uint8 *);
typedef void (*calibmodesfn_t)(phy_info_t *pi, uint mode);
#if defined(WLC_LOWPOWER_BEACON_MODE)
typedef void (*lowpowerbeaconmodefn_t)(phy_info_t *pi, int lowpower_beacon_mode);
#endif /* WLC_LOWPOWER_BEACON_MODE */

#ifdef WL_LPC
typedef uint8 (*lpcgetminidx_t)(void);
typedef void (*lpcsetmode_t)(phy_info_t *pi, bool enable);
typedef uint8 (*lpcgetpwros_t)(uint8 index);
typedef uint8 (*lpcgettxcpwrval_t)(uint16 phytxctrlword);
typedef void (*lpcsettxcpwrval_t)(uint16 *phytxctrlword, uint8 txcpwrval);
typedef uint8 (*lpccalcpwroffset_t) (uint8 total_offset, uint8 rate_offset);
typedef uint8 (*lpcgetpwridx_t) (uint8 pwr_offset);
typedef uint8 * (*lpcgetpwrlevelptr_t) (void);
#endif // endif

typedef void (*gpaioconfig_t) (phy_info_t *pi, wl_gpaio_option_t option, int core);

/* redefine some wlc_cfg.h macros to take the internal phy_info_t instead of wlc_phy_t */
#undef ISNPHY
#undef ISACPHY
#undef ISLCN20PHY

#define ISNPHY(pi)	PHYTYPE_IS((pi)->pubpi->phy_type, PHY_TYPE_N)
#define ISACPHY(pi)  	PHYTYPE_IS((pi)->pubpi->phy_type, PHY_TYPE_AC)
#define ISLCN20PHY(pi)  	PHYTYPE_IS((pi)->pubpi->phy_type, PHY_TYPE_LCN20)

#define ISPHY_HT_CAP(pi)	(ISACPHY(pi))

#define IS20MHZ(pi)	((pi)->bw == WL_CHANSPEC_BW_20)
#define IS40MHZ(pi)	((pi)->bw == WL_CHANSPEC_BW_40)

#define WL_PKT_BW_20  20
#define WL_PKT_BW_40  40
#define WL_PKT_BW_80  80
#define WL_PKT_BW_160 160

#define NUMSUBBANDS(pi) (((pi)->sromi->subband5Gver == PHY_SUBBAND_4BAND) ? \
	CH_5G_GROUP_EXT + CH_2G_GROUP:CH_5G_GROUP + CH_2G_GROUP)

#define INVALID_ADDRESS	0xFFFFU
#define INVALID_MASK	0x0000U

#define PHY_MAC_REV_CHECK(pi, phy_rev) \
	((phy_rev == 36) ? (D11REV_IS(pi->sh->corerev, 60) || D11REV_IS(pi->sh->corerev, 62)) \
	: 0)

/* defines to optimize the code size */
#ifdef BCMRADIOREV
#define RADIOREV(rev)	BCMRADIOREV
#else /* BCMRADIOREV */
#define RADIOREV(rev)	(rev)
#endif /* BCMRADIOREV */

#ifdef BCMRADIOREV_AUX
#define RADIOREV_AUX(rev)	BCMRADIOREV_AUX
#else /* BCMRADIOREV_AUX */
#define RADIOREV_AUX(rev)	(rev)
#endif /* BCMRADIOREV */

#define HW_RADIOREV(rev) (rev)

#ifdef BCMSROMREV
#define SROMREV(sromrev) BCMSROMREV
#else /* !BCMSROMREV */
#define SROMREV(sromrev) (sromrev)
#endif /* BCMSROMREV */

#ifdef BCMRADIOVER
#define RADIOVER(ver)	BCMRADIOVER
#else /* BCMRADIOVER */
#define RADIOVER(ver)	(ver)
#endif /* BCMRADIOVER */

#ifdef BCMRADIOID
#define RADIOID(id)	BCMRADIOID
#else /* BCMRADIOID */
#define RADIOID(id)	(id)
#endif /* BCMRADIOID */

#if defined(BCMMULTIRADIO0) && defined(BCMMULTIRADIO1)
#ifdef BCMRADIOID
#error "Define either BCMRADIOID or (BCMMULTIRADIO0 and BCMMULTIRADIO1)"
#endif // endif
#define RADIOID_IS(id, value)	\
	((((value) == BCMMULTIRADIO0) || ((value) ==  BCMMULTIRADIO1)) ? \
	((id) == (value)) : 0)
#else
#if defined(BCMMULTIRADIO0) || defined(BCMMULTIRADIO1)
#error "Only one BCMMULTIRADIO was defined"
#endif // endif
#ifdef BCMRADIOID
#define RADIOID_IS(id, value) ((value) == BCMRADIOID)
#else
#define RADIOID_IS(id, value)	((id) == (value))
#endif // endif
#endif /* defined BCMMULTIRADIO0 and BCMMULTIRATIO1 */

#if (defined(BCMRADIO2069REV) || defined(BCMRADIO20691REV) || \
	defined(BCMRADIO20693REV)) && defined(BCMRADIOREV)
#error "Both BCMRADIOREV and radioid specific BCMRADIOxxxxxREV is defined"
#endif // endif

/* For the 2069 Radio:
 * Major and Minor revs are defined as below
 */
#ifdef BCMRADIO2069REV
#define RADIO2069REV(rev)	BCMRADIO2069REV
#else
#define RADIO2069REV(rev)	RADIOREV(rev)
#endif // endif
#define RADIO2069_MAJORREV(rev)	((RADIO2069REV(rev) == 0x40) ? 0: \
	((RADIO2069REV(rev) == 0x42) ? 0:(RADIO2069REV(rev) >> 4)))

#define RADIO2069_MINORREV(rev)	((RADIO2069REV(rev) == 0x40) ? 16 : \
	((RADIO2069REV(rev) == 0x42) ? 17:(RADIO2069REV(rev) & 0x0fU)))

/*
 * For the 20691 Radio:
 *	Major 0 being 0x00 <= rev <= 0x1A
 *		Minor 0 being 0x00 <= rev <= 0x12
 *		Minor 1 being 0x13 <= rev <= 0x15
 *		Minor 2 being 0x16 <= rev <= 0x1A
 *	Major 1 being 0x1B <= rev <= 0x4B
 *		Minor 0 being 0x1B <= rev <= 0x20
 *		Minor 1 being 0x26 <= rev <= 0x37
 *		Minor 2 being 0x38 <= rev <= 0x43
 *		Minor 3 being 0x44 <= rev <= 0x4B
 *		Minor 4 being 0x4C <= rev <= 0x52
 *		or rev = 0x59 or rev = 0x5A
 *		Minor 5 being 0x53 <= rev <= 0x58
 *		Minor 16 being rev = 0x81
 *		Minor 17 being rev = 0x82
 *	Major 2 being rev >= 0x5B
 *		Minor undefined (assume 0)
 *
 *	Note: Chip		Radio Major	Radio Minor
 *	      4345TC0		0
 *	      4345A0		1		0
 *	      43457A0		1		1
 *	      43457B0/B1	1		2
 *	      4345B0		1		3
 *	      4345B1		1		4
 *	      4364A0		1		16
 *	      4364B0		1		17
 */
#ifdef	BCMRADIO20691REV
#define RADIO20691REV(rev)	BCMRADIO20691REV
#else
#define RADIO20691REV(rev)	RADIOREV(rev)
#endif /* Defined BCMRADIO20691REV */

#ifdef	BCMRADIO20697REV
#define RADIO20697REV(rev)	BCMRADIO20697REV
#else
#define RADIO20697REV(rev)	RADIOREV(rev)
#endif /* Defined BCMRADIO20691REV */

#define RADIO20691_MAJORREV(rev)	((RADIO20691REV(rev) <= 0x1A) ? 0 : \
					 (RADIO20691REV(rev) <= 0x5A) ? 1 : \
					 (RADIO20691REV(rev) <= 0x82) ? 1 : 2)
#define RADIO20691_MINORREV(rev)	((RADIO20691_MAJORREV(rev) == 0) ? \
					 ((RADIO20691REV(rev) <= 0x12) ? 0 : \
					  (RADIO20691REV(rev) <= 0x15) ? 1 : 2) : \
					(RADIO20691_MAJORREV(rev) == 1) ? \
					 ((RADIO20691REV(rev) <= 0x20) ? 0 : \
					  (RADIO20691REV(rev) <= 0x37) ? 1 : \
					  (RADIO20691REV(rev) <= 0x43) ? 2 : \
					  (RADIO20691REV(rev) <= 0x4B) ? 3 : \
					  ((RADIO20691REV(rev) <= 0x52) || \
					   (RADIO20691REV(rev) == 0x59) || \
					   (RADIO20691REV(rev) == 0x5A)) ? 4 : \
					  (RADIO20691REV(rev) <= 0x58) ? 5 : \
					  (RADIO20691REV(rev) == 0x81) ? 16 : \
					  (RADIO20691REV(rev) == 0x82) ? 17 : 0) : 0)

/* setting main slice majorrev to 0 (rev is 6), aux slice majorrev to 1 (rev is 9) */
#define RADIO20697_MAJORREV(rev)	((rev) % 2)

/* 43012 wlbga Board */
#define BCM943012WLREF_SSID	0x07d7
#define BCM943012WLETBU_SSID	0x07db
/* 43012 fcbga Board */
#define BCM943012FCREF_SSID	0x07d4
/* 43012 fcbga ET Board */
#define BCM943012FCETREF_SSID	0x07da

/* 43012 package ID's
    http://confluence.broadcom.com/display/WLAN/BCM43012+Variants%2Cpackage%2Cballmap%2Cfloorplan#
    BCM43012Variants,package,ballmap,floorplan-PackageOptions
*/
#define BCM943012_WLCSPOLY_PKG_ID	0x0	/* WLCSP Olympic package */
#define BCM943012_FCBGA_PKG_ID		0x3	/* FCBGA debug package */
#define BCM943012_WLCSPWE_PKG_ID	0x1	/* WLCSP WE package */
#define BCM943012_FCBGAWE_PKG_ID	0x5	/* FCBGA WE package */
#define BCM943012_WLBGA_PKG_ID		0x2	/* WLBGA package */

/*
 * http://confluence.broadcom.com/display/WLAN4349/Design+Documents#DesignDocuments-B0version
 * For the 20693 Radio:
 *	Major 0 being 0x00 <= rev <= 0x02
 *	Major 1 being 0x03 <= rev <= 0x0D
 *		Minor 0 being rev == 0x03
 *		Minor 1 being rev == 0x04, 0x0A
 *		Minor 2 being rev == 0x05, 0x07, 0x09, 0x0B
 *		Minor 3 being rev == 0x06, 0x08, 0x0C
 *		Minor 4 being rev == 0x0D
 *	Major 2 being 0x0E <= rev < 0x20
 *		Minor 0 being rev == 0x10
 *		Minor 1 being rev == 0x0E
 *		Minor 3 being rev == 0x11
 *		Minor 4 being rev == 0x0F
 *	Major 3 being rev >= 0x20
 *		Minor 0 being rev == 0x20
 */
#ifdef BCMRADIO20693REV
#define RADIO20693REV(rev)	BCMRADIO20693REV
#else /* BCMRADIOREV */
#define RADIO20693REV(rev)	RADIOREV(rev)
#endif /* BCMRADIOREV */

#define RADIO20693_MAJORREV(rev)	((RADIO20693REV(rev) <= 0x02) ? 0 : \
	((RADIO20693REV(rev) <= 0x0D) ? 1 : \
	(RADIO20693REV(rev) < 0x20) ? 2 : 3))
#define RADIO20693_MINORREV(rev)	((RADIO20693_MAJORREV(rev) == 0) ? 0 : \
	(RADIO20693_MAJORREV(rev) == 1) ? \
	((RADIO20693REV(rev) == 0x03) ? 0 : \
	(RADIO20693REV(rev) == 0x04 || RADIO20693REV(rev) == 0x0A) ? 1 : \
	(RADIO20693REV(rev) == 0x05 || RADIO20693REV(rev) == 0x07 || RADIO20693REV(rev) == 0x09 || \
	RADIO20693REV(rev) == 0x0B) ? 2 : \
	(RADIO20693REV(rev) == 0x06 || RADIO20693REV(rev) == 0x08 || \
	RADIO20693REV(rev) == 0x0C) ? 3 : 4) : \
	(RADIO20693_MAJORREV(rev) == 2) ? \
	((RADIO20693REV(rev) == 0x10) ? 0 : \
	(RADIO20693REV(rev) == 0x0E) ? 1 : \
	(RADIO20693REV(rev) == 0x11) ? 3 : (RADIO20693REV(rev) == 0x0F) ? 4 : 5) : \
	(RADIO20693_MAJORREV(rev) == 3) ? \
	((RADIO20693REV(rev) == 0x20) ? 0 : 1) : 0)

#ifdef BCMRADIO20694REV
#define RADIO20694REV(rev)	BCMRADIO20694REV
#else /* BCMRADIOREV */
#define RADIO20694REV(rev)	RADIOREV(rev)
#endif /* BCMRADIOREV */

#define RADIO20694_MAJORREV(rev) ((RADIO20694REV(rev) < 0x08) ? 2 : 3)
#define RADIO20694_MINORREV(rev)	0

/*
 * http://confluence.broadcom.com/display/WLAN43012/RadioDig
 * For the 20695 Radio:
 *	Major 0 being 0x00 <= rev <= 0x1f
 *	Major 1 being 0x20 <= rev <= 0x40
 *	Minor 0 irrespective of rev as of now
 */

#ifdef BCMRADIO20695REV
#define RADIO20695REV(rev)	BCMRADIO20695REV
#else /* BCMRADIOREV */
#define RADIO20695REV(rev)	RADIOREV(rev)
#endif /* BCMRADIOREV */

#define RADIO20695_MAJORREV(rev)	((RADIO20695REV(rev) <= 0x1f) ? 0 : \
	((RADIO20695REV(rev) <= 0x40) ? 1 : 2))
#define RADIO20695_MINORREV(rev)	0

#ifdef BCMRADIO20698REV
#define RADIO20698REV(rev)	BCMRADIO20698REV
#else /* BCMRADIOREV */
#define RADIO20698REV(rev)	RADIOREV(rev)
#endif /* BCMRADIOREV */

#define RADIO20698_MAJORREV(rev)	RADIO20698REV(rev)
#define RADIO20698_MINORREV(rev)	0

#ifdef BCMRADIO20704REV
#define RADIO20704REV(rev)	BCMRADIO20704REV
#else /* BCMRADIOREV */
#define RADIO20704REV(rev)	RADIOREV(rev)
#endif /* BCMRADIOREV */

#define RADIO20704_MAJORREV(rev)	RADIO20704REV(rev)
#define RADIO20704_MINORREV(rev)	0

#ifdef BCMRADIOMAJORREV
#define RADIOMAJORREV(pi)	BCMRADIOMAJORREV
#else
#define RADIOMAJORREV(pi)	((RADIOID_IS((pi)->pubpi->radioid, BCM20693_ID)) ? \
					RADIO20693_MAJORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20691_ID)) ? \
					RADIO20691_MAJORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20694_ID)) ? \
					RADIO20694_MAJORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20697_ID)) ? \
					RADIO20697_MAJORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20698_ID)) ? \
					RADIO20698_MAJORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20704_ID)) ? \
					RADIO20704_MAJORREV((pi)->pubpi->radiorev) : \
					RADIO2069_MAJORREV((pi)->pubpi->radiorev))
#endif /* BCMRADIOMAJORREV */

#ifdef BCMRADIOMINORREV
#define RADIOMINORREV(pi)	BCMRADIOMINORREV
#else
#define RADIOMINORREV(pi)	((RADIOID_IS((pi)->pubpi->radioid, BCM20693_ID)) ? \
					RADIO20693_MINORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20691_ID)) ? \
					RADIO20691_MINORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20694_ID)) ? \
					RADIO20694_MINORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20698_ID)) ? \
					RADIO20698_MINORREV((pi)->pubpi->radiorev) : \
					(RADIOID_IS((pi)->pubpi->radioid, BCM20704_ID)) ? \
					RADIO20704_MINORREV((pi)->pubpi->radiorev) : \
					RADIO2069_MINORREV((pi)->pubpi->radiorev))

#endif /* BCMRADIOMINORREV */

/* 'Tiny' architecture is a property of the radio */
#define TINY_RADIO(pi)	(RADIOID_IS((pi)->pubpi->radioid, BCM20691_ID) || \
			RADIOID_IS((pi)->pubpi->radioid, BCM20693_ID))

/* 28nm radio's */
#define IS_28NM_RADIO(pi) (RADIOID_IS((pi)->pubpi->radioid, BCM20694_ID) || \
			RADIOID_IS((pi)->pubpi->radioid, BCM20695_ID) || \
			RADIOID_IS((pi)->pubpi->radioid, BCM20696_ID) || \
			RADIOID_IS((pi)->pubpi->radioid, BCM20697_ID) || \
			RADIOID_IS((pi)->pubpi->radioid, BCM20698_ID) || \
			RADIOID_IS((pi)->pubpi->radioid, BCM20704_ID))

#define IS_4349A2_RADIO(pi) (RADIOID_IS((pi)->pubpi->radioid, BCM20693_ID) && \
				((RADIO20693REV((pi)->pubpi->radiorev) >= 0x0A) && \
				(RADIO20693REV((pi)->pubpi->radiorev) <= 0x0D)))

#ifdef XTAL_FREQ
#define PHY_XTALFREQ(_freq)	XTAL_FREQ
#else
#define PHY_XTALFREQ(_freq)	(_freq)
#endif // endif

#define PHY_XTAL_IS52M(pi)	(PHY_XTALFREQ((pi)->xtalfreq) == 52000000)
#define PHY_XTAL_IS40M(pi)	(PHY_XTALFREQ((pi)->xtalfreq) == 40000000)
#define PHY_XTAL_IS37M4(pi)	(PHY_XTALFREQ((pi)->xtalfreq) == 37400000)

#ifdef WLPHY_IPA_ONLY
#define PHY_IPA(pi)		(1)
#define PHY_IPA_ATTACH_2G_PARAMS(pi)		(1)
#define PHY_IPA_ATTACH_5G_PARAMS(pi)		(1)
#define PHY_EPA_SUPPORT(_epa)	(0)
#else /* WLPHY_IPA_ONLY */
#ifdef WLPHY_EPA_ONLY
#define PHY_IPA(pi)		(0)
#define PHY_IPA_ATTACH_2G_PARAMS(pi)		(0)
#define PHY_IPA_ATTACH_5G_PARAMS(pi)		(0)
#else /* WLPHY_EPA_ONLY - for epa only chips ipa related code can be compiled out */
#define PHY_IPA(pi) \
	(((pi)->ipa2g_on && CHSPEC_IS2G((pi)->radio_chanspec)) || \
	 ((pi)->ipa5g_on && CHSPEC_IS5G((pi)->radio_chanspec)))
#define PHY_IPA_ATTACH_2G_PARAMS(pi)		((pi)->ipa2g_on)
#define PHY_IPA_ATTACH_5G_PARAMS(pi)		((pi)->ipa5g_on)
#endif /* WLPHY_EPA_ONLY */
#ifdef EPA_SUPPORT
#define PHY_EPA_SUPPORT(_epa)	(EPA_SUPPORT)
#else
#define PHY_EPA_SUPPORT(_epa)	(_epa)
#endif /* EPA_SUPPORT */
#endif /* WLPHY_IPA_ONLY */

#ifdef PHY_NO_ILNA
#define PHY_ILNA(pi)	0
#else
/* for ilna only ACPHY chip */
#define PHY_ILNA(pi) \
	(ISACPHY(pi)?\
	((!BF_ELNA_2G((pi)->u.pi_acphy) && CHSPEC_IS2G((pi)->radio_chanspec)) || \
	 (!BF_ELNA_5G((pi)->u.pi_acphy) && CHSPEC_IS5G((pi)->radio_chanspec))):0)
#endif // endif

#ifdef PAPD_SUPPORT
#define PHY_PAPD_ENABLE(_papd)	(PAPD_SUPPORT)
#else
#define PHY_PAPD_ENABLE(_papd)	(_papd)
#endif /* PAPD_FORCE_ENABLE */

#define GENERIC_PHY_INFO(pi)	((pi)->sh)

#ifdef BOARD_FLAGS
#define BOARDFLAGS(flag)	(BOARD_FLAGS)
#else
#define BOARDFLAGS(flag)	(flag)
#endif // endif

#ifdef BOARD_FLAGS2
#define BOARDFLAGS2(flag)	(BOARD_FLAGS2)
#else
#define BOARDFLAGS2(flag)	(flag)
#endif // endif

/* Offset of Target Power per channel in 2GHz feature,
 * designed for 4354 iPa with LTE filter, but can support any ACPHY chip
 */

#ifdef POWPERCHANNL
#define CH20MHz_NUM_2G	14 /* Number of 20MHz channels in 2G band */
#define PWR_PER_CH_NORM_TEMP	0	/* Temp zone  in norm for power per channel  */
#define PWR_PER_CH_LOW_TEMP		1	/* Temp zone  in low for power per channel  */
#define PWR_PER_CH_HIGH_TEMP	2	/* Temp zone  in high for power per channel  */
#define PWR_PER_CH_TEMP_MIN_STEP	5	/* Min temprature step for sensing  */
#define PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM 20 /* maximal power reduction offset: 5dB =20 qdBm */
#define PWR_PER_CH_POS_OFFSET_LIMIT_QDBM 12 /* maximal power increase offset: 3dB =12 qdBm */
#endif /* POWPERCHANNL */

#if defined(BCM94360X52D)
#define IS_X52C_BOARDTYPE(pi) ((CHIPID(pi->sh->chip) == BCM4360_CHIP_ID) && \
			       ((pi->sh->boardtype == BCM94360X52C) || \
			        (pi->sh->boardtype == BCM94360X52D)))
#else
#define IS_X52C_BOARDTYPE(pi) ((CHIPID(pi->sh->chip) == BCM4360_CHIP_ID) && \
			       (pi->sh->boardtype == BCM94360X52C))
#endif /* BCM94360X52D */

#define IS_X29C_BOARDTYPE(pi) ((CHIPID(pi->sh->chip) == BCM4360_CHIP_ID) && \
			       ((pi->sh->boardtype == BCM94360X29C) || \
			        (pi->sh->boardtype == BCM94360X29CP2) || \
			        (pi->sh->boardtype == BCM94360X29CP3)))

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  macro, typedef, enum, structure, global variable		*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define	SC_MODE_0_sd_adc		0
#define	SC_MODE_1_sd_adc_5bits      	1
#define	SC_MODE_2_cic0       		2
#define	SC_MODE_3_cic1       		3
#define	SC_MODE_4s_rx_farrow_1core  	4
#define	SC_MODE_4m_rx_farrow      	5
#define	SC_MODE_5_iq_comp       	6
#define	SC_MODE_6_dc_filt       	7
#define	SC_MODE_7_rx_filt       	8
#define	SC_MODE_8_rssi       		9
#define	SC_MODE_9_rssi_all       	10
#define	SC_MODE_10_tx_farrow      	11
#define	SC_MODE_11_gpio      		12
#define	SC_MODE_12_gpio_trans      	13
#define	SC_MODE_14_spect_ana      	14
#define	SC_MODE_5s_iq_comp      	15
#define	SC_MODE_6s_dc_filt      	16
#define	SC_MODE_7s_rx_filt      	17
#define SC_MODE_1_28nm_sar_adc_ti       19
#define	SC_MODE_0_28nm_sar_adc_nti      20
#define	SC_MODE_2_28nm_dcc_out      	21
#define	SC_MODE_3_28nm_farrow_in      	22
#define	SC_MODE_4_28nm_farrow_out      	23

/* %%%%%% shared */
#define PHY_GET_RFATTN(rfgain)	((rfgain) & 0x0f)
#define PHY_GET_PADMIX(rfgain)	(((rfgain) & 0x10) >> 4)
#define PHY_GET_RFGAINID(rfattn, padmix, width)	((rfattn) + ((padmix)*(width)))
#define PHY_SAT(x, n)		((x) > ((1<<((n)-1))-1) ? ((1<<((n)-1))-1) : \
				((x) < -(1<<((n)-1)) ? -(1<<((n)-1)) : (x)))
/* MATLAB round (x) using fixed point arithmetic */
#define PHY_SHIFT_ROUND(x, n)	((x) >= 0 ? ((x)+(1<<((n)-1)))>>(n) : (x)>>(n))
#define PHY_HW_ROUND(x, s)		((x >> s) + ((x >> (s-1)) & (s != 0)))

/* channels */
#ifdef BCMSROMREV
#define CH_5G_GROUP	(BCMSROMREV >= 12) ? 5 : 3
#else
#define CH_5G_GROUP	5
#endif // endif
/* 5 channel groups in 5G for SROM12 and 3 for others (low, mid, high) */
#define CH_5G_GROUP_EXT   4 /* Extended the 5g to 4 subbands */
#define A_LOW_CHANS	0	/* Index for low channels in A band */
#define A_MID_CHANS	1	/* Index for mid channels in A band */
#define A_HIGH_CHANS	2	/* Index for high channels in A band */
#define CH_2G_GROUP	1	/* B band, channel groups, just one */
#define CH_2G_GROUP_NEW	5	/* B band, channel groups, 5 changed for Olympic */
#define G_ALL_CHANS	0	/* Index for all channels in G band */
#define CH_5G_4BAND 4	/* 4subband in 5G, band0, band1, band2 and band3 */
#define CH_5G_5BAND 5	/* 5subbands in 5G, band0, band1, band2, band3 and band4 */
/* 4 mcs need ppr bit expansion: mcs8/9/10/11 */
#define MCS_PPREXP_GROUP	4
#define PPREXP_MCS8		8
#define PPREXP_MCS9		9
#define PPREXP_MCS_P_10		10
#define PPREXP_MCS_P_11		11

#define FIRST_REF5_CHANNUM	149	/* Lower bound of disable channel-range for srom rev 1 */
#define LAST_REF5_CHANNUM	165	/* Upper bound of disable channel-range for srom rev 1 */
#define	FIRST_5G_CHAN		14	/* First allowed channel index for 5G band */
#define	LAST_5G_CHAN		50	/* Last allowed channel for 5G band */
#define	FIRST_MID_5G_CHAN	14	/* Lower bound of channel for using m_tssi_to_dbm */
#define	LAST_MID_5G_CHAN	35	/* Upper bound of channel for using m_tssi_to_dbm */
#define	FIRST_HIGH_5G_CHAN	36	/* Lower bound of channel for using h_tssi_to_dbm */
#define	LAST_HIGH_5G_CHAN	41	/* Upper bound of channel for using h_tssi_to_dbm */
#define	FIRST_LOW_5G_CHAN	42	/* Lower bound of channel for using l_tssi_to_dbm */
#define	LAST_LOW_5G_CHAN	50	/* Upper bound of channel for using l_tssi_to_dbm */

/* SSLPNPHY has different sub-band range limts for the A-band compared to MIMOPHY
 */
#define FIRST_LOW_5G_CHAN_SSLPNPHY      34
#define LAST_LOW_5G_CHAN_SSLPNPHY       64
#define FIRST_MID_5G_CHAN_SSLPNPHY      100
#define LAST_MID_5G_CHAN_SSLPNPHY       140
#define FIRST_HIGH_5G_CHAN_SSLPNPHY     149
#define LAST_HIGH_5G_CHAN_SSLPNPHY      165

#define PHY_SUBBAND_3BAND_EMBDDED	0
#define PHY_SUBBAND_3BAND_HIGHPWR	1
#define PHY_SUBBAND_5BAND		2
#define PHY_SUBBAND_4BAND		4
#define PHY_MAXNUM_5GSUBBANDS		5
#define NUMSROM8POFFSETS	8
#define PHY_SUBBAND_3BAND_JAPAN		7

#define JAPAN_LOW_5G_CHAN	4900
#define JAPAN_MID_5G_CHAN	5100
#define JAPAN_HIGH_5G_CHAN	5500

#define EMBEDDED_LOW_5G_CHAN	5170
#define EMBEDDED_MID_5G_CHAN	5500
#define EMBEDDED_HIGH_5G_CHAN	5745

#define HIGHPWR_LOW_5G_CHAN	5170
#define HIGHPWR_MID_5G_CHAN	5250
#define HIGHPWR_HIGH_5G_CHAN	5745

#define PHY_SUBBAND_4BAND_BAND0	5170
#define PHY_SUBBAND_4BAND_BAND1	5250
#define PHY_SUBBAND_4BAND_BAND2	5500
#define PHY_SUBBAND_4BAND_BAND3	5745

#define PHY_MAXNUM_5GSUBBANDS_BAND0 5170
#define PHY_MAXNUM_5GSUBBANDS_BAND1 5250
#define PHY_MAXNUM_5GSUBBANDS_BAND2 5500
#define PHY_MAXNUM_5GSUBBANDS_BAND3 5600
#define PHY_MAXNUM_5GSUBBANDS_BAND4 5745

#define PHY_RSSI_SUBBAND_4BAND_BAND0	5170
#define PHY_RSSI_SUBBAND_4BAND_BAND1	5500
#define PHY_RSSI_SUBBAND_4BAND_BAND2	5620
#define PHY_RSSI_SUBBAND_4BAND_BAND3	5745

#define PWROFFSET40_MASK_3     0xf000
#define PWROFFSET40_MASK_2     0xf00
#define PWROFFSET40_MASK_1     0xf0
#define PWROFFSET40_MASK_0     0xf

#define PWROFFSET40_SHIFT_3    12
#define PWROFFSET40_SHIFT_2    8
#define PWROFFSET40_SHIFT_1   4
#define PWROFFSET40_SHIFT_0    0

#define CHAN5G_FREQ(chan)  (5000 + chan*5)
#define CHAN2G_FREQ(chan)  ((chan == 14) ? 2484 : (2407 + chan*5))

/* power per rate array index */
#define CCK_20_PO		0
#define CCK_20UL_PO		1
#define OFDM_20_PO		2
#define OFDM_20UL_PO		3
#define OFDM_40DUP_PO		4
#define MCS_20_PO		5
#define MCS_20UL_PO		6
#define MCS_40_PO		7
#define PWR_OFFSET_SIZE		9

#define PHY_TOTAL_TX_FRAMES(pi) \
	wlapi_bmac_read_shm((pi)->sh->physhim, MACSTAT_ADDR(pi, MCSTOFF_TXFRAME))
#define PHY_TSSI_CAL_DBG_EN 0

#define ADJ_PWR_TBL_LEN		256	/* number of phy-capable rates */

/* PHY/RADIO core(chains) */
#define PHY_CORE_NUM_1	1	/* 1 stream */
#define PHY_CORE_NUM_2	2	/* 2 streams */
#define PHY_CORE_NUM_3	3	/* 3 streams */
#define PHY_CORE_NUM_4	4	/* 4 streams */
#define PHY_CORE_0	0	/* array index for core 0 */
#define PHY_CORE_1	1	/* array index for core 1 */
#define PHY_CORE_2	2	/* array index for core 2 */
#define PHY_CORE_3	3	/* array index for core 3 */

#define PRIMARY_FREQ_SEGMENT    0	/* Primary freq segment for 80P80 */
#define SECONDARY_FREQ_SEGMENT  1	/* Secondary freq segment for 80P80 */

#ifdef BCMPHYCORENUM
#  if BCMPHYCORENUM == 1
#    define PAPARAM_SET_NUM 3
#  elif BCMPHYCORENUM == 2
#    define PAPARAM_SET_NUM 4
#  elif BCMPHYCORENUM == 3
#    define PAPARAM_SET_NUM 6   /* Assuming two-rssi-range */
#  elif BCMPHYCORENUM == 4
#    define PAPARAM_SET_NUM 8
#  else
#    define PAPARAM_SET_NUM 6
#  endif /* BCMPHYCORENUM == 1 */
#else /* !BCMPHYCORENUM */
#  define PAPARAM_SET_NUM 6
#endif /* BCMPHYCORENUM */

/* macros to loop over TX/RX cores */
#define FOREACH_ACTV_CORE(pi, coremask, idx)	\
	for (idx = 0; (int) idx < PHYCORENUM((pi)->pubpi->phy_corenum); idx++) \
		if ((PHYCOREMASK(coremask) >> idx) & 0x1)

#define FOREACH_CORE(pi, idx)	\
	for (idx = 0; (int) idx < PHYCORENUM((pi)->pubpi->phy_corenum); idx++)

#define FOREACH_PI(pi, idx)	\
	for (idx = 0; (int) idx < MAX_RSDB_MAC_NUM; idx++)

#define PHY_GET_NONZERO_OR_DEFAULT(value, default_value) \
	(value ? value : default_value)

#define IF_ACTV_CORE(pi, coremask, idx)	\
	if ((PHYCOREMASK(coremask) >> idx) & 0x1)

/* Frequency Tones in Different Bandwidth */
#define NTONES_BW20 64
#define NTONES_BW40 128

/* aci_state state bits */
#define ACI_ACTIVE	1	/* enabled either manually or automatically */
#define ACI_CHANNEL_DELTA 5	/* How far a signal can bleed */
#define ACI_CHANNEL_SKIP 2	/* Num of immediately surrounding channels to skip */
#define ACI_FIRST_CHAN 1 /* Index for first channel */
#define ACI_LAST_CHAN 13 /* Index for last channel */
#define ACI_INIT_MA 100 /* Initial moving average for glitch for ACI */
#define ACI_SAMPLES 100 /* Number of samples for ACI */
#define ACI_MAX_UNDETECT_WINDOW_SZ 40

#define PHY_NOISE_ASSOC_UNDESENSE_WAIT 4
#define PHY_NOISE_ASSOC_UNDESENSE_WINDOW 8

/* wl interference 1, (only assoc), bphy desense index increment, 1dB steps */
#define PHY_OFDM_BPHY_CRSIDX_INCR_HI	4
#define PHY_OFDM_BPHY_CRSIDX_INCR_LO	2
#define PHY_OFDM_BPHY_CRSIDX_DECR	1

#define PHY_OFDM_NOISE_ASSOC_LOW_TH  300
#define PHY_OFDM_NOISE_ASSOC_HIGH_TH  500

#define PHY_OFDM_NOISE_NOASSOC_LOW_TH  200
#define PHY_OFDM_NOISE_NOASSOC_HIGH_TH  400

#define PHY_BPHY_NOISE_ASSOC_LOW_TH  125
#define PHY_BPHY_NOISE_ASSOC_HIGH_TH  250

#define PHY_BPHY_NOISE_ASSOC_UNDESENSE_WAIT 4
#define PHY_BPHY_NOISE_ASSOC_UNDESENSE_WINDOW 8

#define PHY_NOISE_HIGH_DETECT_TH 2

/* wl interference 1, (only assoc), bphy desense index increment, 1dB steps */
#define PHY_BPHY_NOISE_CRSIDX_INCR_HI 4
#define PHY_BPHY_NOISE_CRSIDX_INCR_LO 2

/* wl interference 1, bphy desense index decr, 1dB steps */
#define PHY_BPHY_NOISE_CRSIDX_DECR   1

#define MA_WINDOW_SZ		8	/* moving average window size */

#define PHY_NOISE_DESENSE_RSSI_MARGIN (4)   /* Permit phy desense up to (current rssi - margin) */
#define PHY_NOISE_DESENSE_RSSI_MAX (-68)    /* Cap on max rssi for limiting phy desense */

/* aci scan period */
#define NPHY_ACI_CHECK_PERIOD 2

/* noise only scan period */
#define NPHY_NOISE_CHECK_PERIOD 2

/* noise/RSSI state */
#define PHY_NOISE_SAMPLE_MON		1	/* sample phy noise for watchdog */
#define PHY_NOISE_SAMPLE_EXTERNAL	2	/* sample phy noise for scan, CQ/RM */
#define PHY_NOISE_SAMPLE_CRSMINCAL	4	/* sample phy noise for crsmin cal */
#define PHY_CRS_SET_FROM_CACHE  	1	/* sample phy noise for crsmin cal */
#define PHY_CRS_RUN_AUTO        	0	/* Flag for autorun of crsmin_cal  */
#define PHY_SIZE_NOISE_CACHE_ARRAY  5
#define PHY_SIZE_NOISE_ARRAY		8	/* Number of noise samples to be averaged  */
#define PHY_NOISE_WINDOW_SZ	16	/* NPHY noisedump window size */
#define PHY_NOISE_GLITCH_INIT_MA 10	/* Initial moving average for glitch cnt */
#define PHY_NOISE_GLITCH_INIT_MA_BADPlCP 10	/* Initial moving average for badplcp cnt */
#define PHY_NOISE_STATE_MON		0x1
#define PHY_NOISE_STATE_EXTERNAL	0x2
#define PHY_NOISE_STATE_CRSMINCAL	0x4
#define PHY_NOISE_SAMPLE_LOG_NUM_NPHY	10
#define PHY_NOISE_SAMPLE_LOG_NUM_UCODE	9	/* ucode uses smaller value to speed up process */
/* G-band: 30 (dbm) - 10*log10(50*(2^9/0.4)^2/16) + 2 (front-end-loss) - 68 (init_gain)
 * A-band: 30 (dbm) - 10*log10(50*(2^9/0.4)^2/16) + 3 (front-end-loss) - 69 (init_gain)
 */
#define PHY_NOISE_OFFSETFACT_4322  (-33)
#define ACPHY_NOISE_FLOOR_20M (-101)
#define ACPHY_NOISE_FLOOR_40M (-98)
#define ACPHY_NOISE_FLOOR_80M (-95)
#define ACPHY_NOISE_INITGAIN_X29_2G   (67)
#define ACPHY_NOISE_INITGAIN_X29_5G   (67)
#define ACPHY_NOISE_INITGAIN  (67)
#define ACPHY_NOISE_SAMPLEPWR_TO_DBM  (-37)
#define ACPHY_NOISE_SAMPLEPWR_TO_DBM_10BIT  (-49)
#define ACPHY_NOISE_RXGAIN_UNSPECIFIED (-100)

/* 10*log10( (0.6/2^12*16)^2 /50 ) + 30 = -39.6 ~-40 */
#define ACPHY_NOISE_SAMPLEPWR_TO_DBM_43012  (-40)

/* 10*log10( ( ((0.7/2)*(1/512) )^2)/50 ) + 30 ~ (-50dBm) -> ~-201qdBm  */
#define ACPHY_NOISE_SAMPLEPWR_TO_QDBM_4347 (-201)
/* 10*log10( ( ((0.6/2)*(1/512) )^2)/50 ) + 30 ~ (-51.63dBm) -> ~-206qdBm  */
#define ACPHY_NOISE_SAMPLEPWR_TO_QDBM_PHYMAJ44 (-206)
/* 10*log10( ( (0.3*(1/2048)*4 )^2)/50 ) + 30 ~ (-51.63dBm) -> ~-206qdBm  */
#define ACPHY_NOISE_SAMPLEPWR_TO_QDBM_PHYMAJ47 (-206)

#define HTPHY_NOISE_FLOOR_20M (-101)
#define HTPHY_NOISE_FLOOR_40M (-98)
#define NPHY_NOISE_INITGAIN (67)
#define NPHY_NOISE_SAMPLEPWR_TO_DBM  (-37)
/* 1) Max rx_iq_est power per sample (digital) = 2*128^2 (I+Q) = 45 dB
 * 2) Max analog input power to ADCs (0.8V p-p) = 10*log10(0.^4*2/50) + 3(I+Q) + 30(dbm) = 8 dBm
 * 3) rx_iq_est power to dBm conversion = 8 - 45 = -37
 */

#define LCN20PHY_NOISE_PWR_TO_DBM (-270)
/* RX_IQ_EST block use DVGA2[4:11] as input, */
/* the RX_IQ_EST output should be left shift 4 bit to get the power at DVGA2 */
/* 10*log10((1<<4)^2) =24 dB = 96 quarter dB */
#define LCN20PHY_RX_IQ_EST_INP_4bit_SHIFT_TO_QDB (96)

#define PHY_NOISE_MA_WINDOW_SZ	2	/* moving average window size */
#define PHY_NOISE_DESENSE_RSSI_MARGIN (4)   /* Permit phy desense up to (current rssi - margin) */

#define	PHY_RSSI_TABLE_SIZE	64	/* Table size for PHY RSSI Table */
#define RSSI_ANT_MERGE_MAX	0	/* pick max rssi of all antennas */
#define RSSI_ANT_MERGE_MIN	1	/* pick min rssi of all antennas */
#define RSSI_ANT_MERGE_AVG	2	/* pick average rssi of all antennas */

/* TSSI/txpower */
#define	PHY_TSSI_TABLE_SIZE	64	/* Table size for PHY TSSI */
#define	APHY_TSSI_TABLE_SIZE	256	/* Table size for A-PHY TSSI */
#define	TX_GAIN_TABLE_LENGTH	64	/* Table size for gain_table */
#define	DEFAULT_11A_TXP_IDX	24	/* Default index for 11a Phy tx power */
#define NUM_TSSI_FRAMES        4	/* Num ucode frames for TSSI estimation */
#define	NULL_TSSI		0x7f	/* Default value for TSSI - byte */
#define	NULL_TSSI_W		0x7f7f	/* Default value for word TSSI */

#if defined(WLTEST)
#define INVALID_IDLETSSI_VAL 999 /* invalid idletssi, used as default value */
#endif // endif

#define PHY_TXPWRBCKOF_DEF	6	/* default tx power back off */

#define PHY_TXPWR_MIN_LEGACYPHY 5	/* for legacy (non-11AC) phy */
#define LCN20PHY_TXPWR_MIN	3	/* for lcn20phy devices */

#define RADIOPWR_OVERRIDE_DEF	(-1)

#define PWRTBL_NUM_COEFF	3	/* b0, b1, a1 */

/* calibraiton */
#define SPURAVOID_AUTO		-1	/* enable on certain channels, disable elsewhere */
#define SPURAVOID_DISABLE	0	/* disabled */
#define SPURAVOID_FORCEON	1	/* on mode1 */
#define SPURAVOID_FORCEON2	2	/* on mode2 (different freq) */

#define PHY_SW_TIMER_FAST	15	/* 15 second timeout */
#define PHY_SW_TIMER_SLOW	60	/* 60 second timeout */
#define PHY_SW_TIMER_GLACIAL	120	/* 2 minute timeout */

#define PHY_PERICAL_AUTO	0	/* cal type: let PHY decide */
#define PHY_PERICAL_FULL	1	/* cal type: full */
#define PHY_PERICAL_PARTIAL	2	/* cal type: partial (save time) */
#define PHY_PERICAL_UNDEF	3	/* cal type: Not defined - This has to be ignored */

#define PHY_PERICAL_NODELAY	0	/* multiphase cal gap, in unit of ms */
#define PHY_PERICAL_INIT_DELAY	5
#define PHY_PERICAL_ASSOC_DELAY	5
#define PHY_PERICAL_WDOG_DELAY	5

#define PHY_CAL_SEARCHMODE_RESTART   0  /* cal search mode (former FULL) */
#define PHY_CAL_SEARCHMODE_REFINE    1 /* cal search mode (former PARTIAL) */
#define PHY_CAL_SEARCHMODE_UNDEF     2 /* cal search mode: Not defined - This has to be ignored */

/* Keeps count of phy watchdog timer ticks (# elapsed seconds) */
#define PHYTIMER_NOW(pi)	((pi)->sh->now)
/* returs device instance id of pi */
#define PI_INSTANCE(pi)	((pi)->sh->unit)

/* returns time instance of last cal done */
#define LAST_CAL_TIME(pi)	((pi)->cal_info->last_cal_time)
/* returns glacial timer */
#define GLACIAL_TIMER(pi)	((pi)->sh->glacial_timer)
/* glacial timeout to trigger periodic cal */
#define GLACIAL_TIMEOUT(pi)	((PHYTIMER_NOW(pi) - LAST_CAL_TIME(pi)) >= GLACIAL_TIMER(pi))

/* PHY SPINWAIT in unit of us */
#define NPHY_SPINWAIT_RXCORE_RESET2RX_STATUS 1000 /* 1ms for reset2rx to reset */
#define NPHY_SPINWAIT_RXCORE_SETSTATE_RFSEQ_STATUS 1000
#define NPHY_SPINWAIT_RFCTRLINTC_REV3_OVERRIDE	10000
#define NPHY_SPINWAIT_RUNSAMPLES_RFSEQ_STATUS	1000
#define NPHY_SPINWAIT_CAL_TXIQLO		20000
#define NPHY_SPINWAIT_RX_IQ_EST			10000
#define NPHY_SPINWAIT_PAPDCAL			200000
#define NPHY_SPINWAIT_FORCE_RFSEQ_STATUS	200000

#ifdef WLUCODE_RDO_SR
#define NPHY_SPINWAIT_RDO_REG_SR_CMD_POLL_TIMEOUT  1000
#endif // endif

#define ACPHY_SPINWAIT_PAPDCAL			5000000

enum {
	PHY_ACI_PWR_NOTPRESENT,   /* ACI is not present */
	PHY_ACI_PWR_LOW,
	PHY_ACI_PWR_MED,
	PHY_ACI_PWR_HIGH
};

/* Multiphase calibration states and cmds per Tx Phase (for NPHY) */
#define MPHASE_TXCAL_NUMCMDS	2  /* Number of Tx cal cmds per phase */
enum {
	MPHASE_CAL_STATE_IDLE = 0,
	MPHASE_CAL_STATE_INIT = 1,
	MPHASE_CAL_STATE_TXPHASE0,
	MPHASE_CAL_STATE_TXPHASE1,
	MPHASE_CAL_STATE_TXPHASE2,
	MPHASE_CAL_STATE_TXPHASE3,
	MPHASE_CAL_STATE_TXPHASE4,
	MPHASE_CAL_STATE_TXPHASE5,
	MPHASE_CAL_STATE_PAPDCAL,	/* IPA */
	MPHASE_CAL_STATE_PAPDCAL1,	/* IPA */
	MPHASE_CAL_STATE_RXCAL,
	MPHASE_CAL_STATE_RXCAL1,
	MPHASE_CAL_STATE_RSSICAL,
	MPHASE_CAL_STATE_IDLETSSI,
	MPHASE_CAL_STATE_NOISECAL
};

enum {
	PHY_TSSI_SET_MAX_LIMIT = 1,
	PHY_TSSI_SET_MIN_LIMIT = 2,
	PHY_TSSI_SET_MIN_MAX_LIMIT = 3
};

typedef enum {
	CAL_FULL,
	CAL_FULL2,
	CAL_RECAL,
	CAL_CURRECAL,
	CAL_DIGCAL,
	CAL_GCTRL,
	CAL_SOFT,
	CAL_DIGLO,
	CAL_IQ_RECAL,
	CAL_IQ_CAL2,
	CAL_IQ_CAL3,
	CAL_TXPWRCTRL
} phy_cal_mode_t;

typedef enum {
	TX_IIR_FILTER_CCK,
	TX_IIR_FILTER_OFDM,
	TX_IIR_FILTER_OFDM40
} phy_tx_iir_filter_mode_t;

typedef struct {
	uint16 gm_gain;
	uint16 pga_gain;
	uint16 pad_gain;
	uint16 dac_gain;
} phy_txgains_t;

typedef struct {
	phy_txgains_t gains;
	bool useindex;
	uint8 index;
} phy_txcalgains_t;

typedef struct {
	uint8 chan;
	int16 a;
	int16 b;
} phy_rx_iqcomp_t;

typedef struct {
	int16 re;
	int16 im;
} phy_spb_tone_t;

typedef struct {
	uint16 re;
	uint16 im;
} phy_unsign16_struct;

typedef struct {
	uint16 ptcentreTs20;
	uint16 ptcentreFactor;
} phy_sfo_cfg_t;

typedef enum {
	PHY_PAPD_CAL_CW,
	PHY_PAPD_CAL_OFDM
} phy_papd_cal_type_t;

typedef struct {
	uchar gm;
	uchar pga;
	uchar pad;
	uchar dac;
	uchar bb_mult;
} phy_tx_gain_tbl_entry;

typedef struct {
	uint8 gm;
	uint8 mix;
	uint8 casc;
	uint8 ipa;
	uint8 bb_mult;
	uint8 dac_attn;
	uint8 dummy;
	uint8 rf_power;
} lcn20phy_tx_gain_tbl_entry;

#define MAX_NUM_ANCHORS 4

typedef struct ratmodel_paparams {
	int64 p[128], n[128];
	int64 rho[128][3];
	int64 rho_t[3][128];
	int64 c1[3][3];
	int64 c2_calc[3][3];
	int64 c3[3][128];
	int64 c4[3][1];
	int64 det_c1;
} ratmodel_paparams_t;

typedef struct tssi_cal_info {
	int target_pwr_qdBm[MAX_NUM_ANCHORS];
	int measured_pwr_qdBm[MAX_NUM_ANCHORS];
	uint8 anchor_bbmult[MAX_NUM_ANCHORS];
	uint16 anchor_txidx[MAX_NUM_ANCHORS];
	uint16 anchor_tssi[MAX_NUM_ANCHORS];
	uint16 curr_anchor;
	uint8 paparams_calc_in_progress;
	uint8 paparams_calc_done;
	ratmodel_paparams_t rsd;
	int64 paparams_new[4];
} tssi_cal_info_t;

#define PHY_NOISE_DBG_UCODE_NUM_SMPLS (0)
#define PHY_NOISE_DBG_DATA_LEN (38 + PHY_NOISE_DBG_UCODE_NUM_SMPLS)
#define PHY_NOISE_DBG_HISTORY 0

#define k_noise_cal_ucode_data_size (8)

#define k_noise_cal_update_steps 2

typedef struct {
	/* state info */
	bool nvram_enable_2g;
	bool nvram_enable_5g;
	bool enable;
	bool global_adj_en;
	bool adj_en;
	bool tainted;
	uint8 state;
	bool noise_cb;
	int8 ref;
	int8 nvram_ref_2g;
	int8 nvram_ref_5g;
	int8 nvram_ref_40_2g;
	int8 nvram_ref_40_5g;
	int8 nvram_po_bias_2g;
	int8 nvram_po_bias_5g;
	bool nvram_dbg_noise;
	int nvram_high_gain;
	int nvram_high_gain_2g;
	int nvram_high_gain_5g;
	int16 nvram_input_pwr_offset_2g;
	int16 nvram_input_pwr_offset_5g[3];
	int16 nvram_input_pwr_offset_40_2g;
	int16 nvram_input_pwr_offset_40_5g[3];
	int8 nvram_gain_tbl_adj_2g;
	int8 nvram_gain_tbl_adj_5g;
	int nvram_nf_substract_val;
	int nvram_nf_substract_val_2g;
	int nvram_nf_substract_val_5g;
	/* phy regs saved */
	int16 high_gain;
	int16 input_pwr_offset;
	int16 input_pwr_offset_40;
	uint16 nf_substract_val;
	uint32 power;
	uint32 ucode_data[k_noise_cal_ucode_data_size];
	int8   ucode_data_len;
	int8   ucode_data_idx;
	uint8  ucode_data_ok_cnt;
	uint8  update_cnt;
	uint8  update_step;
	uint8  update_ucode_interval[k_noise_cal_update_steps];
	uint8  update_data_interval[k_noise_cal_update_steps];
	uint8  update_step_interval[k_noise_cal_update_steps];
#if PHY_NOISE_DBG_HISTORY > 0
	/* dbg */
	int16  dbg_adj_min;
	int16  dbg_adj_max;
	uint16 start_time;
	uint16 per_start_time;
	int8 dbg_dump_idx;
	int8 dbg_dump_sub_idx;
	uint32 dbg_dump_cmd;
	int8 dbg_idx;
#if PHY_NOISE_DBG_UCODE_NUM_SMPLS > 0
	int16 dbg_samples[PHY_NOISE_DBG_UCODE_NUM_SMPLS*2];
#endif // endif
	uint16 dbg_info[PHY_NOISE_DBG_HISTORY][PHY_NOISE_DBG_DATA_LEN];
#endif /* #if PHY_NOISE_DBG_HISTORY > 0 */
} noise_t;

/* For bounding the size of the baseband lo comp results array */
#define STATIC_NUM_RF 32	/* Largest number of RF indexes */
#define STATIC_NUM_BB 9		/* Largest number of BB indexes */

#define BB_MULT_MASK		0x0000ffff
#define BB_MULT_VALID_MASK	0x80000000

#define ACPHY_CHAIN_TX_DISABLE_TEMP	120
#define ACPHY_CHAIN_TX_DISABLE_TEMP_4360	120
#define PHY_CHAIN_TX_DISABLE_TEMP	115
#define PHY_HYSTERESIS_DELTATEMP	5

#define PHY_BITSCNT(x)		bcm_bitcount((uint8 *)&(x), sizeof(uint8))

/* validation macros */
#define VALID_PHYTYPE(pi)	(ISNPHY(pi) || ISACPHY(pi) || ISLCN20PHY(pi))

#define VALID_N_RADIO(radioid)  (radioid == BCM2057_ID)
#define VALID_AC_RADIO(radioid)  (radioid == BCM2069_ID || radioid == BCM20691_ID || \
				  radioid == BCM20693_ID || radioid == BCM20694_ID || \
				  radioid == BCM20695_ID || radioid == BCM20696_ID || \
				  radioid == BCM20697_ID || radioid == BCM20698_ID || \
				  radioid == BCM20704_ID)

/*
* however radio ID checks are run-time. If any future chip breaks this dependency
* driver infrastructure would have to be updated.
*/
#define VALID_LCN20_RADIO(radioid)  ((radioid == BCM20692_ID))

#define	VALID_RADIO(pi, radioid) \
	(ISNPHY(pi) ? VALID_N_RADIO(radioid) :	  \
	    (ISACPHY(pi) ? VALID_AC_RADIO(radioid) :		\
	    (ISLCN20PHY(pi) ? VALID_LCN20_RADIO(radioid) : \
	     FALSE)))

#define RM_INPROG_PHY(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_RM))
#define PLT_INPROG_PHY(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_PLT))
#define ASSOC_INPROG_PHY(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_ASSOC))
#define PHY_MUTED(pi)		(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_MUTE))
#define PUB_NOT_ASSOC(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_NOT_ASSOC))
#define ACI_SCAN_INPROG_PHY(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_ACI_SCAN))
#define DCS_INPROG_PHY(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_DCS))
#define TOF_INPROG_PHY(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_TOF))
#define VSDB_CHANCHG_IN_PROGRESS(pi) (mboolisset((pi)->measure_hold, PHY_HOLD_FOR_VSDB))
#define EXCURSION_IN_PROG(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_EXCURSION | \
	PHY_HOLD_FOR_TOF))
#if defined(WLMCHAN) && defined(WLMULTIQUEUE)
#define SCAN_INPROG_PHY(pi)	EXCURSION_IN_PROG(pi)
#define SCAN_RM_IN_PROGRESS(pi) EXCURSION_IN_PROG(pi)
#else
#define SCAN_INPROG_PHY(pi)	(mboolisset((pi)->measure_hold, PHY_HOLD_FOR_SCAN | \
	PHY_HOLD_FOR_TOF))
#define SCAN_RM_IN_PROGRESS(pi) (mboolisset((pi)->measure_hold, PHY_HOLD_FOR_SCAN | \
	PHY_HOLD_FOR_MPC_SCAN | PHY_HOLD_FOR_RM | PHY_HOLD_FOR_TOF))
#endif // endif

#if defined(EXT_CBALL) || defined(BCMQT)
#define NORADIO_ENAB(pub) ((pub)->radioid == NORADIO_ID)
#else
#define NORADIO_ENAB(pub) 0
#endif // endif

/* ED assert threshold for ltecx */
#ifdef BCMLTECOEX
#define LTECX_ED_THRESH				-40
#endif /* BCMLTECOEX */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  phy_info_t and its prerequisite                             */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef struct _phy_table_info {
	uint	table;
	int	q;
	uint	max;
} phy_table_info_t;

typedef struct _bphy_desense_info {
	uint16 DigiGainLimit;
	uint16 PeakEnergyL;
	int8 drop_initBiq1;
} bphy_desense_info_t;

typedef struct {
	uint16 glitch_badplcp_low_th;
	uint16 glitch_badplcp_high_th;
	uint8 high_detect_total;	/* of consecutive highs (exceeding low_thresh) */
	uint8 low_detect_total;	    /* of consecutive lows (below low_thresh) */
	uint8 high_detect_thresh;   /* high_detect_total needs to exceed this before we dsnse */
	uint8 undesense_window;	    /* comp lo_det_tot with undsens_win to det rate of undsnse */
	uint8 undesense_wait;	    /* max wait time to undsnse, gets halved every udsnse_win */
	uint8 desense_lo_step;      /* define for low_desense */
	uint8 desense_hi_step;		/* define for high_desense */
	uint8 undesense_step;		/* define for undesense */
} noise_thresholds_t;

typedef struct {
	uint16 overrideDigiGain1;
	uint16 bphy_peak_energy_lo;

	uint16 init_gain_code_core0;
	uint16 init_gain_code_core1;
	uint16 init_gain_code_core2;
	uint16 init_gain_table[4];

	uint16 clip1_hi_gain_code_core0;
	uint16 clip1_hi_gain_code_core1;
	uint16 clip1_hi_gain_code_core2;
	uint16 clip1_md_gain_code_core0;
	uint16 clip1_md_gain_code_core1;
	uint16 clip1_md_gain_code_core2;
	uint16 clip1_lo_gain_code_core0;
	uint16 clip1_lo_gain_code_core1;
	uint16 clip1_lo_gain_code_core2;

	uint16 nb_clip_thresh_core0;
	uint16 nb_clip_thresh_core1;
	uint16 nb_clip_thresh_core2;

	uint16 w1_clip_thresh_core0;
	uint16 w1_clip_thresh_core1;
	uint16 w1_clip_thresh_core2;

	uint16 cck_compute_gain_info_core0;
	uint16 cck_compute_gain_info_core1;
	uint16 cck_compute_gain_info_core2;

	uint16 energy_drop_timeout_len;
	uint16 crs_threshold2u;

	bool gain_boost;

	/* Phyreg 0xc33 (bphy energy thresh values for ACI pwr levels */
	uint16 b_energy_lo_aci;
	uint16 b_energy_md_aci;
	uint16 b_energy_hi_aci;

	bool detection_in_progress;

	/* can be modified using ioctl/iovar */
	uint16 adcpwr_enter_thresh;
	uint16 adcpwr_exit_thresh;
	uint16 detect_repeat_ctr;
	uint16 detect_num_samples;
	uint16 undetect_window_sz;
} nphy_aci_interference_info_t;

typedef struct  {
	uint8  curr_home_channel;
	uint16 crsminpwrthld_40_stored;
	uint16 crsminpwrthld_20L_stored;
	uint16 crsminpwrthld_20U_stored;
	uint16 crsminpwrthld_40_stored_core1;
	uint16 crsminpwrthld_20L_stored_core1;
	uint16 crsminpwrthld_20U_stored_core1;
	uint16 crsminpwrthld_20L_base;
	uint16 crsminpwrthld_20U_base;
	uint16 crsminpwrthld_20L_init_cal;
	uint16 crsminpwrthld_20U_init_cal;
	uint16 crsminpwrthld_20L_init_cal_core1;
	uint16 crsminpwrthld_20U_init_cal_core1;
	uint16 bphycrsminpwrthld_init_cal;
	uint16 crsminpwr1thld_40_stored;
	uint16 digigainlimit0_stored;
	uint16 peakenergyl_stored;
	uint16 init_gain_code_core0_stored;
	uint16 init_gain_code_core1_stored;
	uint16 init_gain_code_core2_stored;
	uint16 init_gain_codeb_core0_stored;
	uint16 init_gain_codeb_core1_stored;
	uint16 init_gain_codeb_core2_stored;
	uint16 init_gain_ncal_codeb_core1_stored;
	uint16 init_gain_ncal_codeb_core2_stored;
	uint16 init_gain_table_stored[4];
	uint16 clip1_hi_gain_code_core0_stored;
	uint16 clip1_hi_gain_code_core1_stored;
	uint16 clip1_hi_gain_code_core2_stored;
	uint16 clip1_hi_gain_codeb_core0_stored;
	uint16 clip1_hi_gain_codeb_core1_stored;
	uint16 clip1_hi_gain_codeb_core2_stored;
	uint16 nb_clip_thresh_core0_stored;
	uint16 nb_clip_thresh_core1_stored;
	uint16 nb_clip_thresh_core2_stored;
	uint16 init_ofdmlna2gainchange_stored[4];
	uint16 init_ccklna2gainchange_stored[4];
	uint16 clip1_lo_gain_code_core0_stored;
	uint16 clip1_lo_gain_code_core1_stored;
	uint16 clip1_lo_gain_code_core2_stored;
	uint16 clip1_lo_gain_codeb_core0_stored;
	uint16 clip1_lo_gain_codeb_core1_stored;
	uint16 clip1_lo_gain_codeb_core2_stored;
	uint16 w1_clip_thresh_core0_stored;
	uint16 w1_clip_thresh_core1_stored;
	uint16 w1_clip_thresh_core2_stored;
	uint16 energy_drop_timeout_len_stored;

	uint16 ed_crs40_assertthld0_stored;
	uint16 ed_crs40_assertthld1_stored;
	uint16 ed_crs40_deassertthld0_stored;
	uint16 ed_crs40_deassertthld1_stored;
	uint16 ed_crs20L_assertthld0_stored;
	uint16 ed_crs20L_assertthld1_stored;
	uint16 ed_crs20L_deassertthld0_stored;
	uint16 ed_crs20L_deassertthld1_stored;
	uint16 ed_crs20U_assertthld0_stored;
	uint16 ed_crs20U_assertthld1_stored;
	uint16 ed_crs20U_deassertthld0_stored;
	uint16 ed_crs20U_deassertthld1_stored;
	uint8 lna1_2g_stored_core0[8];
	uint8 lna1_2g_stored_core1[8];
	uint8 lna2_2g_stored_core0[8];
	uint8 lna2_2g_stored_core1[8];

	uint16 radio_chanspec_stored;

	uint	scanroamtimer;

	int8	rssi;
	int8 	rssi_index;
	int8 	rssi_buffer[10];
	int16	max_hpvga_acioff_2G;
	int16	max_hpvga_acion_2G;
	int16	max_hpvga_acioff_5G;
	int16	max_hpvga_acion_5G;

	uint16  badplcp_ma;
	uint16  badplcp_ma_previous;
	uint16  badplcp_ma_total;
	uint16  badplcp_ma_list[MA_WINDOW_SZ];
	int  badplcp_ma_index;
	int16 pre_badplcp_cnt;
	int16  bphy_pre_badplcp_cnt;

	uint16 init_gain_core0;
	uint16 init_gain_core1;
	uint16 init_gain_core2;
	uint16 init_gainb_core0;
	uint16 init_gainb_core1;
	uint16 init_gainb_core2;
	uint16 init_gain_rfseq[4];

	uint16 init_gain_core0_aci_on;
	uint16 init_gain_core1_aci_on;
	uint16 init_gain_core2_aci_on;
	uint16 init_gainb_core0_aci_on;
	uint16 init_gainb_core1_aci_on;
	uint16 init_gainb_core2_aci_on;
	uint16 init_gain_rfseq_aci_on[4];
	uint16 init_gain_core0_aci_off;
	uint16 init_gain_core1_aci_off;
	uint16 init_gain_core2_aci_off;
	uint16 init_gainb_core0_aci_off;
	uint16 init_gainb_core1_aci_off;
	uint16 init_gainb_core2_aci_off;
	uint16 init_gain_rfseq_aci_off[4];

	uint16 crsminpwr0;
	uint16 crsminpwrl0;
	uint16 crsminpwru0;
	uint16 crsminpwr0_core1;
	uint16 crsminpwrl0_core1;
	uint16 crsminpwru0_core1;
	uint16 digigainlimit0;
	uint16 peakenergyl;
	int16 crsminpwr_offset_for_aci_bt;

	uint16 crsminpwr0_aci_on;
	uint16 crsminpwrl0_aci_on;
	uint16 crsminpwru0_aci_on;
	uint16 crsminpwr0_aci_on_core1;
	uint16 crsminpwrl0_aci_on_core1;
	uint16 crsminpwru0_aci_on_core1;

	uint16 crsminpwr0_aci_off;
	uint16 crsminpwrl0_aci_off;
	uint16 crsminpwru0_aci_off;
	uint16 crsminpwr0_aci_off_core1;
	uint16 crsminpwrl0_aci_off_core1;
	uint16 crsminpwru0_aci_off_core1;

#ifdef BPHY_DESENSE
	uint16 bphy_crsminpwr;
	int16 bphy_crsminpwr_index;
	uint16 bphy_crsminpwr_aci_on;
	uint16 bphy_crsminpwr_aci_off;
	int16 bphy_crsminpwr_index_aci_on;
	int16 bphy_crsminpwr_index_aci_off;
	uint16 bphy_crsminpwrthld_stored;
#endif // endif

	int16 crsminpwr_index;
	int16 crsminpwr_index_core1;
	int16 bphy_desense_index;
	int16 bphy_desense_index_scan_restore;
	uint16 bphy_desense_base_initgain[PHY_CORE_MAX];
	bphy_desense_info_t *bphy_desense_lut;
	int16 bphy_desense_lut_size;
	int16 bphy_min_sensitivity;
	int16 crsminpwr_offset_for_bphydesense;

	bphy_desense_info_t *bphy_desense_aci_on_lut;
	int16 bphy_desense_aci_on_lut_size;
	int16 ofdm_desense_index;
	uint16 *ofdm_desense_lut;
	int16 ofdm_desense_lut_size;
	int16 ofdm_min_sensitivity;

	int16 crsminpwr_index_aci_on;
	int16 crsminpwr_index_aci_off;
	int16 crsminpwr_index_aci_on_core1;
	int16 crsminpwr_index_aci_off_core1;

	uint16 radio_2057_core0_rssi_wb1a_gc_stored;
	uint16 radio_2057_core1_rssi_wb1a_gc_stored;
	uint16 radio_2057_core2_rssi_wb1a_gc_stored;
	uint16 radio_2057_core0_rssi_wb1g_gc_stored;
	uint16 radio_2057_core1_rssi_wb1g_gc_stored;
	uint16 radio_2057_core2_rssi_wb1g_gc_stored;
	uint16 radio_2057_core0_rssi_wb2_gc_stored;
	uint16 radio_2057_core1_rssi_wb2_gc_stored;
	uint16 radio_2057_core2_rssi_wb2_gc_stored;
	uint16 radio_2057_core0_rssi_nb_gc_stored;
	uint16 radio_2057_core1_rssi_nb_gc_stored;
	uint16 radio_2057_core2_rssi_nb_gc_stored;

	bool  aci_on_firsttime;
	bool  noise_sw_set;
	uint16  hw_aci_mitig_on;
	bool  store_values;
	bool  aci_active_save;

	bool cca_stats_func_called;
	uint32 cca_stats_total_glitch;
	uint32 cca_stats_bphy_glitch;
	uint32 cca_stats_total_badplcp;
	uint32 cca_stats_bphy_badplcp;
	uint32 cca_stats_mbsstime;

	struct {
		int16 bphy_desense;
		int16 ofdm_desense;
		int max_poss_bphy_desense;
		int max_poss_ofdm_desense;
		uint16 save_initgain_rfseq[4];
		uint16  bphy_glitch_ma;
		uint16  ofdm_glitch_ma;
		uint16  ofdm_glitch_ma_previous;
		uint16  bphy_glitch_ma_previous;
		int16  bphy_pre_glitch_cnt;
		uint16  ofdm_ma_total;
		uint16  bphy_ma_total;
		uint16  ofdm_glitch_ma_list[PHY_NOISE_MA_WINDOW_SZ];
		uint16  bphy_glitch_ma_list[PHY_NOISE_MA_WINDOW_SZ];
		int  ofdm_ma_index;
		int  bphy_ma_index;

		uint16  bphy_badplcp_ma;
		uint16  ofdm_badplcp_ma;
		uint16  ofdm_badplcp_ma_previous;
		uint16  bphy_badplcp_ma_previous;
		uint16  ofdm_badplcp_ma_total;
		uint16  bphy_badplcp_ma_total;
		uint16  ofdm_badplcp_ma_list[PHY_NOISE_MA_WINDOW_SZ];
		int  ofdm_badplcp_ma_index;
		uint16	bphy_badplcp_ma_list[PHY_NOISE_MA_WINDOW_SZ];
		int  bphy_badplcp_ma_index;

		uint16 noise_glitch_low_detect_total;
		uint16 noise_glitch_high_detect_total;
		uint16 bphy_noise_glitch_low_detect_total;
		uint16 bphy_noise_glitch_high_detect_total;

		uint16 newgain_initgain;
		uint16 newgainb_initgain;
		uint16 newgain_rfseq;
		uint32 newgain_rfseq_rev19;
		bool changeinitgain;
		uint16 newcrsminpwr_20U;
		uint16 newcrsminpwr_20L;
		uint16 newcrsminpwr_40;
		uint16 newcrsminpwr_20U_core1;
		uint16 newcrsminpwr_20L_core1;
		uint16 newcrsminpwr_40_core1;

#ifdef BPHY_DESENSE
		uint16 newbphycrsminpwr;
#endif // endif
		uint16 nphy_noise_noassoc_glitch_th_up; /* wl interference 4 */
		uint16 nphy_noise_noassoc_glitch_th_dn;
		uint16 nphy_noise_assoc_glitch_th_up;
		uint16 nphy_noise_assoc_glitch_th_dn;
		uint16 nphy_bphynoise_noassoc_glitch_th_up;
		uint16 nphy_bphynoise_noassoc_glitch_th_dn;
		uint16 nphy_bphynoise_assoc_glitch_th_up;
		uint16 nphy_bphynoise_assoc_glitch_th_dn;
		uint16 nphy_noise_assoc_aci_glitch_th_up;
		uint16 nphy_noise_assoc_aci_glitch_th_dn;
		uint16 nphy_noise_assoc_enter_th;
		uint16 nphy_noise_noassoc_enter_th;
		uint16 nphy_bphynoise_assoc_enter_th;
		uint16 nphy_bphynoise_assoc_high_th;
		uint16 nphy_bphynoise_noassoc_enter_th;
		uint16 nphy_noise_assoc_rx_glitch_badplcp_enter_th;
		uint16 nphy_noise_noassoc_crsidx_incr;
		uint16 nphy_noise_assoc_crsidx_incr;
		uint16 nphy_noise_crsidx_decr;
		uint16 nphy_bphynoise_crsidx_incr;
		uint16 nphy_bphynoise_crsidx_decr;
		noise_thresholds_t bphy_thres;
		noise_thresholds_t ofdm_thres;
	} noise;

	struct {
		uint16  glitch_ma;
		uint16  glitch_ma_previous;
		int  pre_glitch_cnt;
		uint16  ma_total;
		uint16  ma_list[MA_WINDOW_SZ];
		int  ma_index;
		int  exit_thresh;		/* Lowater to exit aci */
		int  enter_thresh;		/* hiwater to enter aci mode */
		int  usec_spintime;		/* Spintime between samples */
		int  glitch_delay;		/* delay between ACI scans when glitch count is
								 * continously high
								 */
		int  countdown;
		char rssi_buf[CH_MAX_2G_CHANNEL][ACI_SAMPLES + 1];
			/* Save rssi vals for debugging */

		nphy_aci_interference_info_t *nphy;

		/* history of ACI detects */
		int  detect_index;
		int  detect_total;
		int  detect_list[ACI_MAX_UNDETECT_WINDOW_SZ];
		int  detect_acipwr_lt_list[ACI_MAX_UNDETECT_WINDOW_SZ];
		int  detect_acipwr_max;

	} aci;
	int8 link_rssi; /* update link rssi */
} interference_info_t;

typedef struct _nphy_iq_comp {
	int16 a0;
	int16 b0;
	int16 a1;
	int16 b1;
} nphy_iq_comp_t;

typedef struct {
	uint16		txcal_interm_coeffs[11]; /* best coefficients */
	chanspec_t	txiqlocal_chanspec;	/* Last Tx IQ/LO cal chanspec */
	uint16 		txiqlocal_coeffs[11]; /* best coefficients */
	bool		txiqlocal_coeffsvalid;   /* bit flag */
} nphy_cal_result_t;

/* Define a dummy acphy_cal_result structure to reduce memory footprint
 * since this struct is union'd with nphy
 */
#ifdef PHYCAL_CACHE_SMALL
typedef struct {
	uint16  txiqlocal_coeffs[1]; /* dummy structure */
	uint16  txiqlocal_interm_coeffs[1];
	bool    txiqlocal_coeffsvalid;
	uint8   txiqlocal_ladder_updated[1];
	txgain_setting_t cal_orig_txgain[1];
	txgain_setting_t txcal_txgain[1];
	chanspec_t chanspec;
	uint16  txiqlocal_biq2byp_coeffs[1];
} acphy_cal_result_t;
#else
typedef struct {
	uint16  txiqlocal_coeffs[20]; /* best ("final") comp coeffs (up to 4 cores) */
	uint16  txiqlocal_interm_coeffs[20]; /* intermediate comp coeffs */
	bool    txiqlocal_coeffsvalid;   /* bit flag */
	uint8   txiqlocal_ladder_updated[PHY_CORE_MAX]; /* up to 4 cores */
	txgain_setting_t cal_orig_txgain[PHY_CORE_MAX];
	txgain_setting_t txcal_txgain[PHY_CORE_MAX];
	chanspec_t chanspec;
	uint16  txiqlocal_biq2byp_coeffs[8]; /* for rx-iq cal */
} acphy_cal_result_t;
#endif /* PHYCAL_CACHE_SMALL */

typedef struct {
	uint8	cal_searchmode;
	uint8	cal_phase_id;	/* mphase cal state */
	uint8	txcal_cmdidx;
	uint8	txcal_numcmds;

	union {
		nphy_cal_result_t ncal;
		acphy_cal_result_t accal;
	} u;

	uint       last_cal_time; /* in [sec], covers 136 years if 32 bit */
	uint       last_temp_cal_time; /* in [sec], covers 136 years if 32 bit */
	uint       cal_suppress_count; /* in sec */
	int16      last_cal_temp;
	uint32 fullphycalcntr;
	uint32 multiphasecalcntr;
	bool ignore_crs_status;
	bool phy_forcecal_request;
} phy_cal_info_t;

/* hold cached values for all channels, useful for debug */

#ifndef ACPHY_PAPD_EPS_TBL_SIZE
#define ACPHY_PAPD_EPS_TBL_SIZE 64
#endif // endif

#ifndef ACPHY_PAPD_RFPWRLUT_TBL_SIZE
#define ACPHY_PAPD_RFPWRLUT_TBL_SIZE 128
#endif // endif

typedef struct {
	uint16 ofdm_txa[PHY_CORE_MAX];
	uint16 ofdm_txb[PHY_CORE_MAX];
	uint16 ofdm_txd[PHY_CORE_MAX]; /* contain di & dq */
	uint16 bphy_txa[PHY_CORE_MAX];
	uint16 bphy_txb[PHY_CORE_MAX];
	uint16 bphy_txd[PHY_CORE_MAX]; /* contain di & dq */
	uint8  txei[PHY_CORE_MAX];
	uint8  txeq[PHY_CORE_MAX];
	uint8  txfi[PHY_CORE_MAX];
	uint8  txfq[PHY_CORE_MAX];
	uint16 rxa[PHY_CORE_MAX];
	uint16 rxb[PHY_CORE_MAX];
	int32 rxs[PHY_CORE_MAX];
	bool rxe;
	int16 idle_tssi[PHY_CORE_MAX];
	uint8 baseidx[PHY_CORE_MAX];

	/* Indicate OLPC cal status */
	bool olpc_caldone;

	/* contain papd cal epsilon values */
	uint32 papd_eps[PHY_CORE_MAX*ACPHY_PAPD_EPS_TBL_SIZE];

	/* contain papd cal epsilon offset also known as RFPWRLUT */
	uint16 eps_offset_cache[PHY_CORE_MAX*ACPHY_PAPD_RFPWRLUT_TBL_SIZE];
#ifdef WLC_TXFDIQ
    int32 txs[PHY_CORE_MAX];
#endif // endif
} acphy_calcache_t;

typedef struct {
	uint16 epsilon_offset[PHY_CORE_MAX];
	uint8 papd_comp_en[PHY_CORE_MAX];
} acphy_ram_calcache_t;

typedef struct {
	uint16 txcal_coeffs[8];
	uint16 txcal_radio_regs[8];
	nphy_iq_comp_t rxcal_coeffs;
	uint16 rssical_radio_regs[2];
	uint16 rssical_phyregs[12];
	uint32 papd_core0_coeffs[64];
	uint32 papd_core1_coeffs[64];
	uint16 noisecal_regs[7];
} nphy_calcache_t;

typedef struct {
	/* TX IQ LO cal results */
	uint16 txiqlocal_a[3];
	uint16 txiqlocal_didq[3];
} lcnphy_calcache_t;

typedef struct ch_calcache {
	struct ch_calcache *next;
	bool valid;
	uint creation_time;
	union {
		acphy_calcache_t acphy_cache;
		nphy_calcache_t nphy_cache;
#ifndef PHYCAL_CACHE_SMALL
		lcnphy_calcache_t lcnphy_cache;
#endif // endif
	} u;
	phy_cal_info_t cal_info;
	bool in_use;
	union {
		acphy_ram_calcache_t *acphy_ram_calcache;
	} u_ram_cache;
} ch_calcache_t;

typedef struct {
	uint8 lna1;
	uint8 lna2;
	uint8 mix;
	uint8 lpf0;
	uint8 lpf1;
	uint8 dvga;
	uint8 trtx;
} rxgain_t;

typedef struct {
	uint16 rfctrlovrd;
	uint16 rxgain;
	uint16 rxgain2;
	uint16 lpfgain;
} rxgain_ovrd_t;

#define PHY_NOISEVAR_BUFSIZE 10	/* Increase this if we need to save more noise vars */

typedef struct phy_srom_info
{
	uint	subband5Gver;			/* 5G subband partition, 1: legacy, 2: new */
	uint8	dBpad;  /* to differentiate between board with and wout pad */
	bool	dettype_5g;  /* determines the detector type for 5G, 1:diode 0:log */
	bool	dettype_2g;  /* determines the detector type for 2G, 1:diode 0:log */
	bool	sr13_dettype_en;		/* enabling the above two dettype flags */
	bool	sr13_cck_spur_en;		/* enabling cck spur reduction setting in srom13 */
	bool	sr13_1p5v_cbuck;		/* using 1.5V cbuck board in 4366 */
	int8    ofdmfilttype_2g;        	/* 20MHz ofdm filter type for 2G */
	int8    cckfilttype;			/* cck filter type */
	int8    ofdmfilttype;			/* 20Mhz ofdm filter type */
	int8    ofdmfilttype40;			/* 40Mhz ofdm filter type */
	bool	sr18_cck_paparam_en;		/* enabling cck PA PARAM in srom18 */

	/* TR isolation indices  */
	uint8 	triso2g;
	uint8 	triso5g;
	uint8 	triso5g_l_c0;
	uint8 	triso5g_m_c0;
	uint8 	triso5g_h_c0;
	uint8 	triso5g_l_c1;
	uint8 	triso5g_m_c1;
	uint8 	triso5g_h_c1;

	/* SW RSSI offsets */
	int16	rssicorrnorm_core0;
	int16	rssicorrnorm_core1;
	int16	rssicorrnorm_core0_5g1;
	int16	rssicorrnorm_core0_5g2;
	int16	rssicorrnorm_core0_5g3;
	int16	rssicorrnorm_core1_5g1;
	int16	rssicorrnorm_core1_5g2;
	int16	rssicorrnorm_core1_5g3;

	/* rpcal params */
	uint16  rpcal2gcore3;
	uint16  rpcal5gb0core3;
	uint16  rpcal5gb1core3;
	uint16  rpcal5gb2core3;
	uint16  rpcal5gb3core3;

	/* tssi floor params */
	int16   tssifloor2ga0;
	int16   tssifloor2ga1;
	int16   tssifloor5gla0;
	int16   tssifloor5gla1;
	int16   tssifloor5ga0;
	int16   tssifloor5ga1;
	int16   tssifloor5gha0;
	int16   tssifloor5gha1;

	/* offsets and thresholds */
	int8    temp_diff;
	int8    temp_offs1_2g;
	int8    temp_offs1_5g;
	int8    temp_offs2_2g;
	int8    temp_offs2_5g;
	int8    vbat_diff;
	int8    vbat_offs1_2g;
	int8    vbat_offs1_5g;
	int8    vbat_offs2_2g;
	int8    vbat_offs2_5g;
	int8    cond_offs1;
	int8    cond_offs2;
	int8    cond_offs3;
	int8    cond_offs4;
	int16   cckPwrIdxCorr;
	int16   cck_pwr_offset;
	uint8   bphy_sm_fix_opt;
	int8	offset_targetpwr;
	int16   bw40_5g_pwr_offset;
	int8	txpwr2gAdcScale;
	int8    txpwr5gAdcScale;
	int8	noisevaroffset;			/* Noise var offset */
	int8	noisecaloffset;			/* Noise cal offset 2G */
	int8	noisecaloffset5g;		/* Noise cal offset 5G */
	int8    high_vbat_threshold;
	int8    low_vbat_threshold;
	int8    high_temp_threshold;
	int8    low_temp_threshold;

	/* tx idx capping */
	uint8   txidxcap2g;
	uint8   txidxcap5g;
	int16	nom_txidxcap_2g;
	int16	nom_txidxcap_5g;
	int16	txidxcap_2g_high;
	int16	txidxcap_5g_high;
	int16	txidxcap_2g_low;
	int16	txidxcap_5g_low;
	int16	txidxcap_high;
	int16	txidxcap_low;

	/* misc others */
	uint8   rcal_otp_flag;
	uint16  rcal_otp_val;
	uint8   TssiAuxgain5g;
	uint8	TssiAuxgain2g;
	uint16	TssiVmid5g;
	uint16	TssiVmid2g;
	uint16  min_txpwrindex_2g;
	uint16  min_txpwrindex_5g;
	uint8	iqcal_lowpwradc;	/* Flag to enable lowpower ADC for IQ Cal */
	uint8  	iqcal_adcclampdisable;	/* To disable ADC clamp for IQ Cal */
	int8	elnabypass2g;
	int8	elnabypass5g;
	uint8	precal_tx_idx;
	uint8	epa_on_during_txiqlocal;

	/* sw tx/rx chain params */
	uint8  sw_txchain_mask;
	uint8  sw_rxchain_mask;
	bool    sr13_en_sw_txrxchain_mask;      /* Enable/disable bit for sw chain mask */

#if defined(RXDESENS_EN)
	uint16  phyrxdesens;
	int	saved_interference_mode; /* saved interference mitigation mode in rxdesens */
#endif // endif
	uint32	lpflags;	/* Low power flags */
	int8   txidxcaplow[2];
	uint8 maxepagain[2];
	int8 maxchipoutpower[2];
	int8    txidxmincap2g;
	int8    txidxmincap5g;
} phy_srom_info_t;

/* phy state that is per device instance */
struct shared_phy
{
	osl_t 	*osh;				/* pointer to OS handle */
	si_t	*sih;				/* si handle (cookie for siutils calls) */
	uint	corerev;			/* core revision */
	uint	bustype;			/* SI_BUS, PCI_BUS  */
	uint	buscorerev; 			/* buscore rev */
	uint16	vid;				/* vendorid */
	uint16	did;				/* deviceid */
	uint	chip;				/* chip number */
	uint	chiprev;			/* chip revision */
	uint	chippkg;			/* chip package option */
	uint	sromrev;			/* srom revision */
	uint	boardtype;			/* board type */
	uint	boardrev;			/* board revision */
	uint	boardvendor;			/* board vendor */
	uint32	boardflags;			/* board specific flags from srom */
	uint32	boardflags2;			/* more board flags if sromrev >= 4 */
	uint32	boardflags4;			/* more board flags if sromrev >= 12 */
#if defined(WL_EAP_BOARD_RF_5G_FILTER)
	int	board5gfilter;			/* custom 5G analog filtering, e.g., BCM949408 */
#endif /* WL_EAP_BOARD_RF_5G_FILTER */

	struct	phy_info *phy_head;     	/* head of phy list */

	uint	unit;				/* device instance number */
	void	*physhim;			/* phy <-> wl shim layer for wlapi */
	uint32	machwcap;			/* mac hw capability */
	bool	up;				/* main driver is up and running */
	bool	clk;				/* main driver make the clk available */
	uint	now;				/* # elapsed seconds */
	uint	fast_timer;			/* Periodic timeout for 'fast' timer */
	uint	slow_timer;			/* Periodic timeout for 'slow' timer */
	uint	glacial_timer;			/* Periodic timeout for 'glacial' timer */
	uint	scheduled_cal_time;		/* schedules/offsets percal events between pi's */
	uint8	rssi_mode;
	bool	radar;				/* radar detection: on or off */
	bool	_rifs_phy;			/* per-pkt rifs flag passed down from wlc_info */
	uint8	rx_antdiv;			/* .11b Ant. diversity (rx) selection override */
	int16	cckPwrIdxCorr;
	int8	phy_noise_window[MA_WINDOW_SZ];	/* noise moving average window */
	uint	phy_noise_index;		/* noise moving average window index */
	int	interference_mode;		/* interference mitigation mode */
	int	interference_mode_2G;		/* 2G interference mitigation mode */
	int	interference_mode_5G;		/* 5G interference mitigation mode */
	int	interference_mode_2G_override;	/* 2G interference mitigation mode */
	int	interference_mode_5G_override;	/* 5G interference mitigation mode */
	bool	interference_mode_override;	/* override */
	uint8	disable_spuravoid;		/* spuravoid flag */
	uint8	phyrxdesens;				/* phyrxdesens */

	phy_srom_info_t *sromi;
	char	vars_table_accessor[VARS_ACCESSOR_SZ];
	uint8 bphymrc_en;			/* bphy mrc enable in 2G */
};

/* %%%%%% phy_info_t */
struct phy_pub {
	uint		phy_type;		/* PHY_TYPE_XX */
	uint		phy_rev;		/* phy revision */
	uint8		phy_corenum;		/* number of cores */
	uint16		radioid;		/* radio id */
	uint8		radiorev;		/* radio revision */
	uint8		radiover;		/* radio version */
	uint16		radiooffset;		/* offset for radio read */

	uint		coreflags;		/* sbtml core/phy specific flags */
	uint		ana_rev;		/* analog core revision */
	uint8		phy_coremask;		/* phy core mask, updated based on phy mode */
	uint16		phy_minor_rev;		/* phy minor rev */
	uint8           slice;                  /* Indicates slice 0: MAIN 1: AUX */
};

struct phy_info_nphy;
typedef struct phy_info_nphy phy_info_nphy_t;

struct phy_info_lcn20phy;
typedef struct phy_info_lcn20phy phy_info_lcn20phy_t;

struct phy_info_acphy;
typedef struct phy_info_acphy phy_info_acphy_t;

typedef void (*epadpdset_t) (phy_info_t *pi, uint8 enab_epa_dpd, bool in_2g_band);

typedef bool (*isperratedpden_t) (phy_info_t *pi);
typedef void (*perratedpdset_t) (phy_info_t *pi, bool enable);

struct phy_func_ptr {
	longtrnfn_t longtrn;
	txiqccgetfn_t txiqccget;
	txiqccmimogetfn_t txiqccmimoget;
	txiqccsetfn_t txiqccset;
	txiqccmimosetfn_t txiqccmimoset;
	txloccgetfn_t txloccget;
	txloccsetfn_t txloccset;
	txloccmimogetfn_t txloccmimoget;
	txloccmimosetfn_t txloccmimoset;
	radioloftgetfn_t radioloftget;
	radioloftsetfn_t radioloftset;
	radioloftmimogetfn_t radioloftmimoget;
	radioloftmimosetfn_t radioloftmimoset;
	initfn_t carrsuppr;
	txcorepwroffsetfn_t txcorepwroffsetget;
	txcorepwroffsetfn_t txcorepwroffsetset;
	gettxpwrctrlfn_t    gettxpwrctrl;
	settxpwrbyindexfn_t settxpwrbyindex;
	tssicalsweepfn_t   tssicalsweep;
	calibmodesfn_t     calibmodes;
#if defined(WLC_LOWPOWER_BEACON_MODE)
	lowpowerbeaconmodefn_t lowpowerbeaconmode;
#endif /* WLC_LOWPOWER_BEACON_MODE */
#ifdef WL_LPC
	lpcgetminidx_t		lpcgetminidx;
	lpcgetpwros_t		lpcgetpwros;
	lpcgettxcpwrval_t	lpcgettxcpwrval;
	lpcsettxcpwrval_t	lpcsettxcpwrval;
	lpcsetmode_t		lpcsetmode;
#ifdef WL_LPC_DEBUG
	lpcgetpwrlevelptr_t	lpcgetpwrlevelptr;
#endif // endif
#endif /* WL_LPC */

	isperratedpden_t        isperratedpdenptr;
	perratedpdset_t         perratedpdsetptr;

	gpaioconfig_t gpaioconfigptr;
};
typedef struct phy_func_ptr phy_func_ptr_t;

typedef struct srom_lcn20_ppr {
	uint16 cck202gpo;
	uint32 ofdmbw202gpo;
	uint32 mcsbw202gpo;
	uint8 propbw202gpo;
} srom_lcn20_ppr_t;

struct srom8_ppr {
	uint32 ofdm[CH_2G_GROUP + CH_5G_GROUP_EXT];
	uint8	   bw40[CH_2G_GROUP + CH_5G_GROUP_EXT];
	uint8   stbc[CH_2G_GROUP + CH_5G_GROUP_EXT];
	uint8   bwdup[CH_2G_GROUP + CH_5G_GROUP_EXT];
	uint8   cdd[CH_2G_GROUP + CH_5G_GROUP_EXT];
	uint16  mcs[CH_2G_GROUP + CH_5G_GROUP_EXT][NUMSROM8POFFSETS];
	uint16  cck2gpo;	/* 2G CCK Power offset */
};

struct ppbw {
	uint32	bw20;
	uint32	bw20ul;
	uint32	bw40;
};

struct srom9_ppr {
	uint16		cckbw202gpo;	/* 2G CCK Power offset */
	uint16		cckbw20ul2gpo;	/* 2G CCK 20UL Power offset */
	uint16		mcs32po;	/* MCS32 power offset */
	uint16		ofdm40duppo;	/* OFMD DUP power offset */

	struct ppbw ofdm[CH_2G_GROUP + CH_5G_GROUP_EXT];
	struct ppbw mcs[CH_2G_GROUP + CH_5G_GROUP_EXT];
};
struct ppbw_cck {
	uint16	bw20;
	uint16	bw20in40;
};
struct ppbw_ag_2g {
	uint32	bw20;
};
struct ppbw_nac_2g {
	uint32	bw20;
	uint32	bw40;
};
struct ppbw_ag_5g {
	uint32	bw20[CH_5G_GROUP];
};
struct ppbw_nac_5g {
	uint32	bw40[CH_5G_GROUP];
	uint32	bw80[CH_5G_GROUP];
	uint32  bw160[CH_5G_GROUP];
};

struct srom11_ppr {
	struct ppbw_cck 		cck;
	struct ppbw_ag_2g 		ofdm_2g;
	struct ppbw_nac_2g		mcs_2g;
	struct ppbw_ag_5g 		ofdm_5g;
	struct ppbw_nac_5g 		mcs_5g;
	uint16 offset_2g;
	uint16 offset_20in40_l;
	uint16 offset_20in40_h;
	uint16 offset_dup_l;
	uint16 offset_dup_h;
	uint16 offset_5g[CH_5G_GROUP];
	uint16 offset_20in80_l[CH_5G_GROUP];
	uint16 offset_20in80_h[CH_5G_GROUP];
	uint16 offset_40in80_l[CH_5G_GROUP];
	uint16 offset_40in80_h[CH_5G_GROUP];
#ifdef NO_PROPRIETARY_VHT_RATES
#else
	uint16	pp1024qam2g;
	uint32	pp1024qam5g[CH_5G_5BAND];
	uint32	ppmcsexp[MCS_PPREXP_GROUP];
#endif // endif
};

struct srom13_ppr {
	struct ppbw_cck		cck;
	struct ppbw_ag_2g	ofdm_2g;
	struct ppbw_nac_2g	mcs_2g;
	struct ppbw_ag_5g	ofdm_5g;
	struct ppbw_nac_5g	mcs_5g;
	uint16			offset_2g;
	uint16			offset_20in40_l;
	uint16			offset_20in40_h;
	uint16			offset_dup_l;
	uint16			offset_dup_h;
	uint16			offset_5g[CH_5G_GROUP];
	uint16			offset_20in80_l[CH_5G_GROUP];
	uint16			offset_20in80_h[CH_5G_GROUP];
	uint16			offset_40in80_l[CH_5G_GROUP];
	uint16			offset_40in80_h[CH_5G_GROUP];
	uint16			pp1024qam2g;
	uint16			ppulb2g;
	uint32			pp1024qam5g[CH_5G_GROUP];
	uint32			pp1605g[CH_5G_GROUP];
	uint32			ppulb5g[CH_5G_GROUP];
	uint32			ppmcsexp[MCS_PPREXP_GROUP];
};

typedef struct sr13_ppr_5g_rateset {
	ppr_ofdm_rateset_t      ofdm20_offset_5g;
	ppr_vht_mcs_rateset_t   mcs20_offset_5g;
	ppr_ofdm_rateset_t      ofdm40_offset_5g;
	ppr_vht_mcs_rateset_t   mcs40_offset_5g;
	ppr_ofdm_rateset_t      ofdm80_offset_5g;
	ppr_vht_mcs_rateset_t   mcs80_offset_5g;
} sr13_ppr_5g_rateset_t;

struct srom_lgcy_ppr {
	int8 opo;			/* OFDM power offset */
	int16 cckpo;			/* cck */
	int32 ofdmgpo;		/* 2g ofdm */
	int32 ofdmapo;		/* 5g middle */
	int32 ofdmalpo;		/* 5g low */
	int32 ofdmahpo;		/* 5g high */
};

typedef struct _nphy_txgains {
	uint16 txlpf[2];
	uint16 txgm[2];
	uint16 pga[2];
	uint16 pad[2];
	uint16 ipa[2];
} nphy_txgains_t;

typedef struct srom_pwrdet {
	int16  pwrdet_a1[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_GROUP_EXT];
	int16  pwrdet_b0[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_GROUP_EXT];
	int16  pwrdet_b1[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_GROUP_EXT];
	uint8   max_pwr[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_GROUP_EXT];
	uint8   pwr_offset40[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_GROUP_EXT];
	int16   tssifloor[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_4BAND];
} srom_pwrdet_t;

typedef struct srom11_pwrdet {
	int16	pwrdet_a1[PAPARAM_SET_NUM][CH_2G_GROUP + CH_5G_4BAND];
	int16	pwrdet_b0[PAPARAM_SET_NUM][CH_2G_GROUP + CH_5G_4BAND];
	int16	pwrdet_b1[PAPARAM_SET_NUM][CH_2G_GROUP + CH_5G_4BAND];
	uint8   max_pwr[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_4BAND];
	uint16	pdoffset40[PHY_CORE_MAX], pdoffset80[PHY_CORE_MAX];
	uint8   pdoffset2g40[PHY_CORE_MAX];
	uint8   pdoffset2g40_flag;
	int16   tssifloor[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_4BAND];
	uint8   pdoffsetcck[PHY_CORE_MAX];
	uint16  pdoffset5gsubband[PHY_CORE_MAX];
	uint8	pdoffset2g20in20[PHY_CORE_MAX];
#ifdef POWPERCHANNL
	uint8   max_pwr_SROM2G[PHY_CORE_MAX];
	int8	PwrOffsets2GNormTemp[PHY_CORE_MAX][CH20MHz_NUM_2G];
	int8	PwrOffsets2GLowTemp[PHY_CORE_MAX][CH20MHz_NUM_2G];
	int8	PwrOffsets2GHighTemp[PHY_CORE_MAX][CH20MHz_NUM_2G];
	int16	Low2NormTemp;  /* Value of 0xff indicates not used */
	int16	High2NormTemp; /* Value of 0xff indicates not used */
	uint8	CurrentTempZone;
#endif /* POWPERCHANNL */
} srom11_pwrdet_t;

typedef struct srom12_pwrdet {
	int16	pwrdet_a[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND+1];
	int16	pwrdet_b[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND+1];
	int16	pwrdet_c[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND+1];
	int16	pwrdet_d[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND+1];
	int16	pwrdet_a_40[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND];
	int16	pwrdet_b_40[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND];
	int16	pwrdet_c_40[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND];
	int16	pwrdet_d_40[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND];
	int16	pwrdet_a_80[PHY_CORE_MAX][CH_5G_5BAND];
	int16	pwrdet_b_80[PHY_CORE_MAX][CH_5G_5BAND];
	int16	pwrdet_c_80[PHY_CORE_MAX][CH_5G_5BAND];
	int16	pwrdet_d_80[PHY_CORE_MAX][CH_5G_5BAND];
	int16	pwrdet_a_160[PHY_CORE_MAX][CH_5G_5BAND-1];
	int16	pwrdet_b_160[PHY_CORE_MAX][CH_5G_5BAND-1];
	int16	pwrdet_c_160[PHY_CORE_MAX][CH_5G_5BAND-1];
	int16	pwrdet_d_160[PHY_CORE_MAX][CH_5G_5BAND-1];
	uint8   max_pwr[PHY_CORE_MAX][CH_2G_GROUP + CH_5G_5BAND];
	uint8   pdoffsetcck[PHY_CORE_MAX];
	uint8   pdoffsetcck20m[PHY_CORE_MAX];
	uint16	pdoffset20in40[PHY_CORE_MAX][CH_5G_5BAND+1];
	uint16	pdoffset20in80[PHY_CORE_MAX][CH_5G_5BAND+1];
	uint16	pdoffset40in80[PHY_CORE_MAX][CH_5G_5BAND+1];
	uint16	pdoffset20in160[PHY_CORE_MAX][CH_5G_5BAND];
	uint16	pdoffset40in160[PHY_CORE_MAX][CH_5G_5BAND];
	uint16	pdoffset80in160[PHY_CORE_MAX][CH_5G_5BAND];
} srom12_pwrdet_t;

/* This structure is used to save/modify/restore the noise vars for specific tones */
typedef struct _nphy_noisevar_buf {
	int bufcount;   /* number of valid entries in the buffer */
	int tone_id[PHY_NOISEVAR_BUFSIZE];
	uint32 noise_vars[PHY_NOISEVAR_BUFSIZE];
	uint32 min_noise_vars[PHY_NOISEVAR_BUFSIZE];
} phy_noisevar_buf_t;

typedef struct {
	uint16 rssical_radio_regs_2G[2];
	uint16 rssical_phyregs_2G[12];

	uint16 rssical_radio_regs_5G[2];
	uint16 rssical_phyregs_5G[12];
} rssical_cache_t;

typedef struct {
	/* calibration coefficients for the last 2 GHz channel that was calibrated */
	uint16 txcal_coeffs_2G[8];
	uint16 txcal_radio_regs_2G[8];
	nphy_iq_comp_t rxcal_coeffs_2G;

	/* calibration coefficients for the last 5 GHz channel that was calibrated */
	uint16 txcal_coeffs_5G[8];
	uint16 txcal_radio_regs_5G[8];
	nphy_iq_comp_t rxcal_coeffs_5G;
} txiqcal_cache_t;

typedef struct _fcbs_radioreg_buf_entry {
	uint16 addr;
	uint16 val;
} fcbs_radioreg_buf_entry;

typedef struct _fcbs_phyreg_buf_entry {
	uint16 addr;
	uint16 val;
} fcbs_phyreg_buf_entry;

typedef struct _fcbs_radioreg_list_entry {
	uint16 regaddr;
	uint16 regval;
} fcbs_radioreg_list_entry;

typedef struct _fcbs_radioreg_core_list_entry {
	uint16 regaddr;
	uint16 core_info;
} fcbs_radioreg_core_list_entry;

typedef struct _fcbs_phytbl_list_entry {
	uint16 tbl_id;
	uint16 tbl_offset;
	uint16 num_entries;
} fcbs_phytbl_list_entry;

typedef struct _fcbs_info {
	/* Allow only a single outstanding request (for now) */

	chanspec_t	chanspec[MAX_FCBS_CHANS];
	bool		initialized[MAX_FCBS_CHANS];
	bool		load_regs_tbls;
	int		curr_fcbs_chan;
	uint32		switch_count;

	/* Contains PHY specific on-chip RAM address reserved for
	   storing the FCBS cache.
	*/
	uint		cache_startaddr;

	/* Contains shmem locations used by the PHY specific ucode
	   to determine the various offsets within the FCBS cache
	*/
	uint		shmem_radioreg;
	uint		shmem_phytbl16;
	uint		shmem_phytbl32;
	uint		shmem_phyreg;
	uint		shmem_bphyctrl;
	uint		shmem_cache_ptr;

	int		num_radio_regs;
	int		phytbl16_entries;
	int		phytbl32_entries;
	int		num_phy_regs;
	int		num_bphy_regs[MAX_FCBS_CHANS];

	int		phytbl16_buflen;
	int		phytbl32_buflen;
	int		phyreg_buflen[MAX_FCBS_CHANS];

	fcbs_radioreg_buf_entry	*radioreg_buf[MAX_FCBS_CHANS];
	uint16			*phytbl16_buf[MAX_FCBS_CHANS];
	uint16			*phytbl32_buf[MAX_FCBS_CHANS];
	fcbs_phyreg_buf_entry	*phyreg_buf[MAX_FCBS_CHANS];
	fcbs_phyreg_buf_entry	*bphyreg_buf[MAX_FCBS_CHANS];

	int		chan_cache_offset[MAX_FCBS_CHANS];
	int		radioreg_cache_offset[MAX_FCBS_CHANS];
	int		phytbl16_cache_offset[MAX_FCBS_CHANS];
	int		phytbl32_cache_offset[MAX_FCBS_CHANS];
	int		phyreg_cache_offset[MAX_FCBS_CHANS];
	int		bphyreg_cache_offset[MAX_FCBS_CHANS];
	uint16	hw_fcbs_tbl_data[FCBS_HW_TBL_LEN];
	fcbs_phytbl_list_entry *fcbs_phytbl16_list;
	fcbs_radioreg_core_list_entry *fcbs_radioreg_list;
	uint16 *fcbs_phyreg_list;
	bool FCBS_ucode;
	bool FCBS_INPROG;
} fcbs_info;

typedef struct {
	uint16 idletssi_2g;
	uint16 idletssi_5gl;
	uint16 idletssi_5gm;
	uint16 idletssi_5gh;
} phy_idletssi_perband_info_t;

#define MAX_RADIO_REGS		400
#define SR_MEMORY_BANK		2
#define RATE_MASK_PHY		0x7F
/* SR VSDB state */
typedef struct srvsdb_info {
	chanspec_t prev_chanspec;
	uint8 sr_vsdb_bank_valid[SR_MEMORY_BANK];
	uint8 swbkp_snapshot_valid[SR_MEMORY_BANK];
	uint16 sr_vsdb_channels[SR_MEMORY_BANK];
	uint8 vsdb_trig_cnt;
	uint16 last_cal_chanspec;
	uint8 srvsdb_active;
	uint8 force_vsdb;
	uint16 force_vsdb_chans[2];

	uint32 prev_crsglitch_cnt[2];
	uint32 sum_delta_crsglitch[2];
	uint32 prev_bphy_rxcrsglitch_cnt[2];
	uint32 sum_delta_bphy_crsglitch[2];

	uint32 prev_badplcp_cnt[2];
	uint32 sum_delta_prev_badplcp[2];
	uint32 prev_bphy_badplcp_cnt[2];
	uint32 sum_delta_prev_bphy_badplcp[2];
	uint32 prev_timer[2];
	uint32 sum_delta_timer[2];
	uint8  num_chan_switch[2];
	bool   acimode_noisemode_reset_done[2];
	bool   switch_successful;
} srvsdb_info_t;

/* SW backup structure */
typedef struct srvsdb_backup {
	phy_info_nphy_t *pi_nphy;
	interference_info_t interf;
	/* remove inter struct for now
	*/
	uint16          bw;
	uint16          phy_classifier_state;
	int16           saved_tempsense;
	bool            saved_tempsense_valid;
	chanspec_t      radio_chanspec;
	int             interference_mode;
	bool            interference_mode_crs;
	bool            phy_init_por;
	bool            radio_is_on;
	uint            aci_state;
	uint		aci_active_pwr_level;
	int             cur_interference_mode;
	uint 		last_aci_check_time;
	uint 		last_aci_call;

	uint8           rx_antdiv;
	uint8           spur_mode;
	/* tx Power related parameters added */
	uint8		tx_power_max[2];
	uint8		tx_power_min[2];
	ppr_t*		tx_power_offset;		/* Offset from base power */
	int8		openlp_tx_power_min;
#ifndef WLUCODE_RDO_SR
	uint16		radio_reg_val[MAX_RADIO_REGS];
#endif // endif
	uint16		tx_power_shm[WLC_NUMRATES];
	bool		do_noisemode_reset;
	bool		do_acimode_reset;
#ifdef RXDESENS_EN
	uint16		phyrxdesens;
	int		saved_interference_mode;
#endif // endif
} vsdb_backup_t;

typedef struct {
	union {
		struct srom_lgcy_ppr srlgcy;
		struct srom8_ppr sr8;
		struct srom9_ppr sr9;
		struct srom11_ppr sr11;
		struct srom13_ppr sr13;
		srom_lcn20_ppr_t sr_lcn20;
	} u;
} ppr_info_t;

typedef struct {
	uint8	page;		/* current PAGE */
	uint16	phyregs_pmax;	/* PHYREGS Page MAX */
	uint16  entry;          /* current ENTRY */
} phydump_page_info_t;

struct phy_param_info;
typedef struct phy_param_info phy_param_info_t;

typedef struct {
	bool	_dsi;
	bool	_apapd;
	bool	_wbpapd;
	bool	_ulb;
	bool	_nap;
	uint8   _et;
	bool	_dband;
	uint8*	_feature_enab;
} phy_feature_flags_t;

struct phy_info
{
	wlc_phy_t	*pubpi_ro;	/* public attach time constant phy state */
	wlc_phy_t	*pubpi;		/* private attach time constant phy state */
	shared_phy_t	*sh;		/* shared phy state pointer */
	phy_func_ptr_t	*pi_fptr;

	union {
		phy_info_nphy_t		*pi_nphy;
		phy_info_acphy_t	*pi_acphy;
		phy_info_lcn20phy_t *pi_lcn20phy;
	} u;

	/* ************************************************************************************ */

	phy_cmn_info_t		*cmni;		/* CommonInfo */
	phy_dbg_info_t		*dbgi;		/* Debug/DUMPRegistry */
	phy_type_disp_t		*typei;		/* PHYTypeDispatch */
	phy_chanmgr_notif_info_t *chanmgr_notifi; /* CHSPECNotification module */
	phy_init_info_t		*initi;		/* INIT control module */
	phy_wd_info_t		*wdi;		/* WatchDog module */
	phy_cache_info_t	*cachei;	/* CACHE module */
	phy_calmgr_info_t	*calmgri;	/* CALibrationManaGeR module */
	phy_ana_info_t		*anai;		/* ANAcore control module */
	phy_radio_info_t	*radioi;	/* RADIO control control module */
	phy_tbl_info_t		*tbli;		/* PHYTableInit module */
	phy_tpc_info_t		*tpci;		/* TxPowerCtrl module */
	phy_txpwrcap_info_t	*txpwrcapi;	/* TxPowerCap module */
	phy_radar_info_t	*radari;	/* RadarDetect module */
	phy_antdiv_info_t	*antdivi;	/* ANTennaDIVersity module */
	phy_rssi_info_t		*rssii;		/* RSSICompute module */
	phy_temp_info_t		*tempi;		/* TEMPerature sense module */
	phy_btcx_info_t		*btcxi;		/* BlueToothCoExistence module */
	phy_noise_info_t	*noisei;	/* NOISEmeansure module */
	phy_rxiqcal_info_t	*rxiqcali;	/* RXIQ calibration module */
	phy_txiqlocal_info_t	*txiqlocali;	/* TXIQLO calibration module */
	phy_papdcal_info_t	*papdcali;	/* PAPD calibration module */
	phy_vcocal_info_t	*vcocali;	/* VCO calibration module */
	phy_chanmgr_info_t	*chanmgri;	/* CHannelManaGeR module */
	phy_fcbs_info_t		*fcbsi;		/* FCBS module */
	phy_lpc_info_t		*lpci;		/* LPC module */
	phy_misc_info_t		*misci;		/* MISC module */
	phy_tssical_info_t	*tssicali;	/* TSSI Cal module */
	phy_rxgcrs_info_t	*rxgcrsi;	/* RXGCRS module */
	phy_rxspur_info_t	*rxspuri;	/* RXSPUR module */
	phy_samp_info_t		*sampi;		/* SAMPle collect module */
	phy_dsi_info_t		*dsii;		/* DeepSleepInit module */
	phy_mu_info_t		*mui;		/* MU-MIMO module */
	phy_dccal_info_t	*dccali;	/* DCCAL module */
	phy_tof_info_t		*tofi;		/* TOF module */
	phy_hirssi_info_t	*hirssii;	/* HiRSSIeLNABypass module */
	phy_et_info_t		*eti;		/* Envelope tracking module */
	phy_ocl_info_t      *ocli;      /* One Core Listen (OCL) */
	phy_hecap_info_t	*hecapi;	/* High Efficiency (HE) (802.11ax) */
	phy_stf_info_t		*stfi;		/* space/time coding (MIMO/STBC/bf) module */
	phy_vasip_info_t	*vasipi;	/* VASIP module */
	phy_smc_info_t	    *smci;	    /* SMC module */

	/* ************************************************************************************ */
	/* [PHY_REARCH] Do not add any variables here. Add them to the individual modules */
	/* ************************************************************************************ */
	ppr_info_t		*ppr;
	phy_srom_info_t		*sromi;
	d11regs_t		*regs;
	const d11regdefs_t	*regoffsets;
	struct phy_info		*next;
	struct	wlapi_timer	*phycal_timer;		/* timer for multiphase cal */
	tssi_cal_info_t		*ptssi_cal;
	srom_fem_t		*fem2g;			/* 2G band FEM attributes */
	srom_fem_t		*fem5g;			/* 5G band FEM attributes */
	srom_pwrdet_t		*pwrdet;
	ppr_t			*tx_power_offset;	/* ppr offsets */
	interference_info_t	*interf;		/* interference params */

	phy_cal_info_t		*def_cal_info;		/* Default cal info (not allocated) */
	phy_cal_info_t		*cal_info;		/* Multiple instances of calib info's */
	tx_pwr_cache_entry_t	*txpwr_cache;
	phy_hc_info_t		*hci;

	/* ************************************************************************************ */
	/* [PHY_REARCH] phy_utils_reg.c ---------------- */
	/* ************************************************************************************ */

	/* Phy table addr/data split access state */
	uint	tbl_save_id;
	uint	tbl_save_offset;
	uint16	phy_wreg;
	uint16	phy_wreg_limit;
	uint16	tbl_data_hi;
	uint16	tbl_data_lo;
	uint16	tbl_addr;
	uint8	phyreg_enter_depth;		/* cntr to track the phy reg enter/exits */

	/* ************************************************************************************ */
	/* [PHY_REARCH] phy_utils_channel.c ---------------- */
	/* ************************************************************************************ */
	uint8	a_band_high_disable;	/* high channels in A band disabled for srom v1 */

	/* ************************************************************************************ */
	/* [PHY_REARCH] LCN20PHY only ---------------- */
	/* ************************************************************************************ */
	int16	txpa_2g_2pwr[PWRTBL_NUM_COEFF]; /* 2G: For two pwr range scheme */
	uint16	hwpwr_txcur; /* hwpwrctl: current tx power index */
	int8	phy_pacalstatus;
	uint8	ldpc_en;
	bool	low_power_mode;

	/* ************************************************************************************ */
	/* [PHY_REARCH] LCNPHY only ---------------- */
	/* ************************************************************************************ */
	int16	txpa_2g[PWRTBL_NUM_COEFF];				/* 2G: pa0b%d */
	int16	txpa_2g_lo[PWRTBL_NUM_COEFF];			/* For 2nd LUT */
	int16	txpa_5g_low[PWRTBL_NUM_COEFF];			/* 5G low: pa1lob%d */
	int16	txpa_5g_mid[PWRTBL_NUM_COEFF];			/* 5G mid: pa1b%d */
	int16	txpa_5g_hi[PWRTBL_NUM_COEFF];			/* 5G hi: pa1hib%d */

	/* ************************************************************************************ */
	/* [PHY_REARCH] NPHY only -------------------- */
	/* ************************************************************************************ */
	vsdb_backup_t	*vsdb_bkp[2];	/* sw bkp of two instances */
	uint	last_aci_call;
	uint	last_aci_check_time;
	uint	aci_active_pwr_level;
	int16	saved_tempsense;
	bool	saved_tempsense_valid;
	uint	nphy_phyreg_skipaddr[128];
	int8	nphy_phyreg_skipcnt;
	int8	nphy_tbldump_minidx;
	int8	nphy_tbldump_maxidx;
	uint8	nphy_ml_type;
	uint8	aci_nams;				/* read as... ACI Non Assoc Mode Sanity */
	uint8	saved_txpwr_idx;		/* saved current hwpwrctl txpwr index */
	uint8	nphy_txpwrctrl;			/* tx power control setting */
	int8	nphy_txrx_chain;		/* chain override for both TX & RX */
	int8	nphy_rssisel;
	bool	nphy_elna_gain_config;	/* flag to reduce Rx gains for external LNA */
	bool	nphy_rssical;			/* enable/disable nphy rssical (for rev3 only) */
	bool	do_noisemode_reset;
	bool	do_acimode_reset;
	bool	nphy_btc_lnldo_bump;	/* indicates the bump in ln-ldo1: btcxwar */
	bool	aci_rev7_subband_cust_fix;
	bool	phy_bphy_evm;			/* continuous CCK transmission is ON/OFF */
	bool	phy_bphy_rfcs;			/* nphy BPHY RFCS testpattern is ON/OFF */

	/* ************************************************************************************ */
	/* [PHY_REARCH] Multiple PHYs ----------------- */
	/* ************************************************************************************ */
	uint8	aa2g, aa5g;		/* antennas available for 2G, 5G: NPHY */

	int8	phy_spuravoid;	/* spur avoidance: NPHY
							 * 0: disable, 1: auto, 2: on, 3: on2
							 */
	int8	phy_spuravoid_mode;		/* Prev chann spur mode: NPHY */
	uint8	phynoise_state;			/* phy noise sample state NPHY */
	uint16  phy_classifier_state;	/* NPHY */
	uint16	phy_clip_state[2];		/* NPHY */
	uint16	old_bphy_test;			/* NPHY */
	uint16	old_bphy_testcontrol;	/* NPHY */
	uint8	ldo_voltage;			/* NPHY */
	bool	edcrs_threshold_lock;	/* lock the edcrs detection threshold NPHY */
	uint16	phy_gpiosel;			/* NPHY */
	int		cur_interference_mode;	/* Track interference mode of phy NPHY */

	uint	phy_lastcal;	/* last time PHY periodic calibration ran LCN20,40 */
	bool	phy_forcecal;	/* run calibration at the earliest opportunity LCN20,40 */
	/* srom max board value (.25 dBm) */
	uint8	tx_srom_max_2g;	/* 2G: pa0maxpwr LCN20,40 */
	uint8	tx_srom_max_5g_low;	/* 5G low: pa1lomaxpwr LCN20,40 */
	uint8	tx_srom_max_5g_mid;	/* 5G mid: pa1maxpwr LCN20,40 */
	uint8	tx_srom_max_5g_hi;	/* 5G hi: pa1himaxpwr LCN20,40 */
	int8	rssi_corr_normal; /* LCN20,40 */
	int8	rssi_corr_boardatten; /* LCN20,40 */
	bool	lpc_algo;			/* LCN20,40 */

	int8	carrier_suppr_disable;	/* disable carrier suppression LCN20 */

	/* ************************************************************************************ */
	/* [PHY_REARCH] Do not add any variables here. Add them to the individual modules */
	/* ************************************************************************************ */

	chanspec_t	last_radio_chanspec;	/* last radio chanspec */
	chanspec_t	radio_chanspec;		/* current radio chanspec */
	char	*vars;				/* phy attach time only copy of vars */
	bool	phytest_on;			/* whether a PHY test is running */
	uint8	antsel_type;			/* Type of boardlevel mimo antenna switch-logic
						 * 0 = N/A, 1 = 2x4 board, 2 = 2x3 CB2 board
						 */
	uint16	bw;				/* b/w (10, 20 or 40) [only 20MHZ on non NPHY] */
	bool	phy_init_por;			/* power on reset prior to phy init call */
	bool	init_in_progress;		/* init in progress */
	bool	initialized;			/* Have we been initialized ? */
	uint	refcnt;
	bool	phywatchdog_override;		/* to disable/enable phy watchdog */
	bool	trigger_noisecal;		/* trigger noisecal */
	uint	phynoise_now;			/* timestamp to run sampling */
	int	phynoise_chan_watchdog;		/* sampling target channel for watchdog */
	bool	phynoise_polling;		/* polling or interrupt for sampling */
	bool	disable_percal;			/* phy agnostic iovar to disable watchdog cals */
	mbool	measure_hold;			/* should we hold off measurements/calibrations */
	int8	phy_rssi_gain_error[PHY_CORE_MAX]; /* per-core gain-error */
	uint8	tx_power_max_per_core[PHY_CORE_MAX];
	uint8	tx_power_min_per_core[PHY_CORE_MAX];
	bool	hwpwrctrl;			/* ucode controls txpower instead of driver */
	uint8	acphy_txpwrctrl;		/* TODO: check if it is used for papdcal */
	bool	phy_5g_pwrgain;			/* flag to indicate 5G Power Gain is enabled */
	int8	n_preamble_override;		/* preamble override for both TX & RX, both band */
	int8	txpwr_est_Pout;			/* Best guess at current txpower */
	int8	openlp_tx_power_min;
	bool	openlp_tx_power_on;
	uint	interference_mode_crs_time; 	/* time at which crs was turned off */
	uint16	crsglitch_prev;			/* crsglitch count at last watchdog */
	bool	interference_mode_crs;		/* aphy crs state, interference mitigation */
	uint	aci_start_time;			/* adjacent channel interference start time */
	int 	aci_exit_check_period;
	int 	aci_enter_check_period; 	/* enter ACI scan, when ACI is not active based on
						 * glitches OR enter_check timer
						 */
	uint	aci_state;
	int32	phy_tx_tone_freq;
	bool	phy_fixed_noise;		/* flag to report PHY_NOISE_FIXED_VAL noise */
	uint32	xtalfreq;			/* Xtal frequency */
	int8	phy_scraminit;
	int8	txpwridx;			/* 11a Power control */
	bool 	hwpwrctrl_capable;
	uint8	phy_noise_index;		/* noise moving average window index */
	int16	phy_noise_win[PHY_CORE_MAX][PHY_NOISE_WINDOW_SZ]; /* noise per antenna */

	bool	ipa2g_on;
	bool	ipa5g_on;

	uint8	phy_rxiq_samps;
	uint8	phy_rxiq_antsel;
	uint8	phy_rxiq_resln;
	uint8	phy_rxiq_lpfhpc;		/* lpf_hpc override select for rxiqest */
	uint8	phy_rxiq_diglpf; 		/* rx dig_lpf override select for rxiqest */
	uint8	phy_rxiq_gain_correct;		/* enable/disable (1/0) gain-correction
						 * when reporting powers in rxiqest
						 */
	uint8	phy_rxiq_extra_gain_3dB;	/* INITgain += (extra_gain_3dB * 3) */
	uint8	phy_rxiq_force_gain_type;

	bool	first_cal_after_assoc;
	uint16	radar_percal_mask;		/* signal periodic_cal is running to blank radar */
	bool	radio_is_on;
	uint8	phy_cal_mode;			/* percal modes: disable, single, mphase */
	uint16	phy_cal_delay;			/* percal delay between each mphase */
	bool	dfs_lp_buffer_nphy;	/* enable/disable clearing DFS LP buffer
						* [PHY_REARCH] Need to rename used not only in nphy
						*/
	bool	dfs_lp_buffer_nphy_sc;
	int16	srom_rawtempsense;
	int16	srom_gain_cal_temp;
	int8	srom_eu_edthresh2g, srom_eu_edthresh5g;
	bool	nphy_enable_hw_antsel; /* [PHY_REARCH] Need to rename used not only in nphy */
	uint16	fabid;
	uint16 	tunings[3];
	bool	block_for_slowcal;
	uint16	blocked_freq_for_slowcal;
	bool	HW_FCBS;
	bool 	FCBS;
	uint8	dacratemode2g;
	uint8	dacratemode5g;
	uint8	fdss_bandedge_2g_en;
	int8	fdss_level_2g[4];
	int8	fdss_level_5g[4];
	int8	fdss_level_2g_ch1[4];
	int8	fdss_level_2g_ch13[4];
	uint8	fdss_interp_en;
	int8	txgaintbl5g;
	int8	phy_tempsense_offset;
	int8	tx_pwr_backoff;				/* in qdBm steps */
	int8	min_txpower;				/* minimum allowed tx power */
	int16	*paprrmcsgamma2g;
	uint8	*paprrmcsgain2g;
	int16	*paprrmcsgamma5g20;
	uint8	*paprrmcsgain5g20;
	int16	*paprrmcsgamma5g40;
	uint8	*paprrmcsgain5g40;
	int16	*paprrmcsgamma5g80;
	uint8	*paprrmcsgain5g80;
	int16	*paprrmcsgamma2g_ch1;
	uint8	*paprrmcsgain2g_ch1;
	int16	*paprrmcsgamma2g_ch13;
	uint8	*paprrmcsgain2g_ch13;
	/* Gain errors measured from phy_rxiqest and stored in srom: */
	int8	rxgainerr_2g[PHY_CORE_MAX];		/* 2G channels */
	bool	rxgainerr2g_isempty;
	int8	rxgainerr_5gl[PHY_CORE_MAX];		/* 5G-low channels */
	bool	rxgainerr5gl_isempty;
	int8	rxgainerr_5gm[PHY_CORE_MAX];		/* 5G-mid channels */
	bool	rxgainerr5gm_isempty;
	int8	rxgainerr_5gh[PHY_CORE_MAX];		/* 5G-high channels */
	bool	rxgainerr5gh_isempty;
	int8	rxgainerr_5gu[PHY_CORE_MAX];		/* 5G-upper channels */
	bool	rxgainerr5gu_isempty;

	/* Gain-corrected noise-levels (dBm) from SROM (measured using phy_rxiqest): */
	int8	noiselvl_2g[PHY_CORE_MAX];		/* 2G channels */
	int8	noiselvl_5gl[PHY_CORE_MAX];		/* 5G-low channels */
	int8	noiselvl_5gm[PHY_CORE_MAX];		/* 5G-mid channels */
	int8	noiselvl_5gh[PHY_CORE_MAX];		/* 5G-high channels */
	int8	noiselvl_5gu[PHY_CORE_MAX];		/* 5G-upper channels */
	void	*pwrdet_ac;

	uint8	afe_override;
	int8	min_txpower_5g;
	uint32	cal_dur;				/* Cumulative ms spent in calibration */
	uint32	cal_period;
	bool	fdiqi_disable;				/* fdiqi disable */
	bool	sdadc_config_override;
	bool	phyinit_pending;			/* flag to indicate phyinit is
							 * pending upon phymode switch
							 */
	uint8	txpwrctrl;				/* tx power control setting */
	uint16	bt_shm_addr;

	/* ************************************************************************************ */
	/*                       Pointers to Structs (with cflags)                              */
	/* ************************************************************************************ */

	srvsdb_info_t		*srvsdb_state;

	fcbs_info		*phy_fcbs;

	/* ************************************************************************************ */
	/*                        Variables in phy info (with cflags)                           */
	/* ************************************************************************************ */

	uint8	phy_calcache_num;		/* Indicates the num of active contexts */
	bool	phy_calcache_on;

	uint16	car_sup_phytest;			/* Save phytest */
	uint16	evm_phytest;				/* Save phytest */
	uint32	evm_o;					/* GPIO output */
	uint32	evm_oe;					/* GPIO Output Enables */
	uint16	tempsense_override;

	/* ************************************************************************************ */
	/* [PHY_REARCH] Do not add any variables here. Add them to the individual modules */
	/* ************************************************************************************ */
	bool _dct;
	phy_feature_flags_t	*ff;		/* phy features enab knobs */
	int8 ldo3p3_voltage;
	int8 paldo3p3_voltage;
	phydump_page_info_t	*pdpi;	/* phydump page utility */
	bool    phynoise_disable; /* Disable scheduling phy noise measurement in the ucode */
	/* FCC_PWR_LIMIT_2G */
	chanspec_t previous_chanspec; /* check previous chanspec */
	const shmdefs_t *shmdefs; /* Pointer to shmdefs array */
	phy_nap_info_t *napi;
	uint16 rcal_value;
	uint8   core2slicemap;
	uint8   main_slice_ant[DUAL_PHY_NUM_CORE_MAX];
	uint8   aux_slice_ant[DUAL_PHY_NUM_CORE_MAX];
	phy_crash_reason_t phy_crash_rc;
	bool	phynoise_pmstate; /* PM State for noise mmt decision */
	uint32	phynoise_lastmmttime; /* Last Noise mmt time. Irrespective of channel. */
	bool	ccktpcloop_en;
	bool    phytxtone_symm; /* A flag to indicate symmetrical tone */
	uint8 phy_chanest_dump_ctr;	/* Chanest dump counter */
	uint	extpagain5g;			/* iPA boards (extapagain2g/5g = 2) */
	uint	extpagain2g;			/* iPA boards (extapagain2g/5g = 2) */
	uint	epagain5g;			/* iPA boards (epagain2g/5g = 2) */
	uint	epagain2g;			/* iPA boards (epagain2g/5g = 2) */
	bool	_swdiv;
	bool _powerperchan;
	/* ************************************************************************************ */
	/* [PHY_REARCH] Do not add any variables here. Add them to the individual modules */
	/* ************************************************************************************ */
};

struct prephy_info
{
	shared_phy_t		*sh;		/* shared phy state pointer */
	d11regs_t			*regs;
	const d11regdefs_t	*regoffsets;
	wlc_phy_t			*pubpi;		/* private attach time constant phy state */
	phy_prephy_info_t	*prephyi;		/* prephy module */
};

/* SW Diversity Support */
#ifdef WLC_SW_DIVERSITY
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define PHYSWDIV_ENAB(pi)   ((pi)->_swdiv)
#elif defined(WLC_SW_DIVERSITY_DISABLED)
	#define PHYSWDIV_ENAB(pi)   (0)
#else
	#define PHYSWDIV_ENAB(pi)	((pi)->_swdiv)
#endif // endif
#else
	#define PHYSWDIV_ENAB(pi)   (0)
#endif	/* WLC_SW_DIVERSITY */

/* %%%%%% shared functions */
typedef int32    fixed;	/* s15.16 fixed-point */

/* radio regs that do not have band-specific values */

typedef struct radio_20xx_dumpregs {
	uint16 address;
} radio_20xx_dumpregs_t;

typedef struct radio_20xx_prefregs {
	uint16 address;
	uint16  init;
} radio_20xx_prefregs_t;

/* %%%%%% utilities */

/* %%%%%% common flow function */
extern bool wlc_phy_attach_nphy(phy_info_t *pi);
extern bool wlc_phy_attach_lcn20phy(phy_info_t *pi);
extern void wlc_phy_detach_lcn20phy(phy_info_t *pi);

extern void wlc_phy_init_nphy(phy_info_t *pi);

extern void wlc_phy_chanspec_set_nphy(phy_info_t *pi, chanspec_t chanspec);
extern uint8 wlc_set_chanspec_sr_vsdb_nphy(phy_info_t *pi, chanspec_t chanspec,
	uint8 *last_chan_saved);

extern int8 wlc_lcn20phy_get_current_tx_pwr_idx(phy_info_t *pi);

extern void wlc_phy_txpower_recalc_target_nphy(phy_info_t *pi);

extern void
ppr_dsss_printf(ppr_t* tx_srom_max_pwr);
extern void
ppr_ofdm_printf(ppr_t* tx_srom_max_pwr);
extern void
ppr_mcs_printf(ppr_t* tx_srom_max_pwr);

extern bool wlc_phy_aci_scan_gphy(phy_info_t *pi);
extern void wlc_phy_aci_interf_nwlan_set_gphy(phy_info_t *pi, bool on);
extern void wlc_phy_aci_ctl_gphy(phy_info_t *pi, bool on);
extern void wlc_phy_aci_upd_nphy(phy_info_t *pi);
extern void wlc_phy_aci_ctl_nphy(phy_info_t *pi, bool enable, int aci_pwr);
extern void wlc_phy_aci_inband_noise_reduction_nphy(phy_info_t *pi, bool on, bool raise);
extern void wlc_phy_aci_sw_reset_nphy(phy_info_t *pi);
extern void wlc_phy_noisemode_reset_nphy(phy_info_t *pi);
extern void wlc_phy_acimode_reset_nphy(phy_info_t *pi); /* reset ACI mode */
extern void wlc_phy_aci_noise_upd_nphy(phy_info_t *pi);
extern void wlc_phy_acimode_upd_nphy(phy_info_t *pi);
extern void wlc_phy_noisemode_upd_nphy(phy_info_t *pi);
extern void wlc_phy_acimode_set_nphy(phy_info_t *pi, bool aci_miti_enable, int aci_pwr);
extern void wlc_phy_aci_init_nphy(phy_info_t *pi);
extern void wlc_phy_bphy_ofdm_noise_hw_set_nphy(phy_info_t *pi);
extern void wlc_phy_aci_noise_reset_nphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc);
extern void wlc_phy_clip_det_nphy(phy_info_t *pi, uint8 write, uint16 *vals);

extern void wlc_lcn20phy_set_tx_pwr_by_index(phy_info_t *pi, int indx);
extern void wlc_phy_ofdm_to_mcs_powers_nphy(uint8 *power, uint rate_mcs_start, uint rate_mcs_end,
	uint rate_ofdm_start);
extern void wlc_phy_mcs_to_ofdm_powers_nphy(uint8 *power, uint rate_ofdm_start,
	uint rate_ofdm_end,  uint rate_mcs_start);
extern bool wlc_phy_txpwr_srom_read_gphy(phy_info_t *pi);
extern bool wlc_phy_txpwr_srom_read_aphy(phy_info_t *pi);

/* %%%%%% LCN20CONF function */
#define LCN20PHY_TX_POWER_TABLE_SIZE	128
#define LCN20PHY_MAX_TX_POWER_INDEX	(LCN20PHY_TX_POWER_TABLE_SIZE - 1)
#define LCN20PHY_TBL_ID_TXPWRCTL 	0x07
#define LCN20PHY_TX_PWR_CTRL_OFF	0
#define LCN20PHY_TX_PWR_CTRL_SW		LCN20PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK
#define LCN20PHY_TX_PWR_CTRL_HW         (LCN20PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
					LCN20PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK | \
					LCN20PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)

extern void wlc_lcn20phy_set_tx_tone_and_gain_idx(phy_info_t *pi);
extern void wlc_lcn20phy_deaf_mode(phy_info_t *pi, bool mode);
extern void wlc_lcn20phy_start_tx_tone(phy_info_t *pi, int32 f_kHz, uint16 max_val,
	bool iqcalmode, bool deafmode, bool rxrfmode);
extern void wlc_lcn20phy_stop_tx_tone(phy_info_t *pi);

/* %%%%%% NCONF function */
#define NPHY_MAX_HPVGA1_INDEX		10
#define NPHY_DEF_HPVGA1_INDEXLIMIT	7

#define CHANNEL_ISRADAR(channel)  ((((channel) >= 52) && ((channel) <= 64)) || \
				   (((channel) >= 100) && ((channel) <= 144)))

extern void wlc_nphy_deaf_mode(phy_info_t *pi, bool mode);
extern bool wlc_phy_get_deaf_nphy(phy_info_t *pi);

#define wlc_phy_write_table_nphy(pi, pti) phy_utils_write_phytable(pi, pti, NPHY_TableAddress, \
	NPHY_TableDataHi, NPHY_TableDataLo)
#define wlc_phy_read_table_nphy(pi, pti) phy_utils_read_phytable(pi, pti, NPHY_TableAddress, \
	NPHY_TableDataHi, NPHY_TableDataLo)
#define wlc_nphy_table_addr(pi, id, off)	phy_utils_write_phytable_addr((pi), (id), (off), \
	NPHY_TableAddress, NPHY_TableDataHi, NPHY_TableDataLo)
#define wlc_nphy_table_data_write(pi, w, v)	phy_utils_write_phytable_data((pi), (w), (v))

extern void wlc_phy_table_read_nphy(phy_info_t *pi, uint32, uint32 l, uint32 o, uint32 w, void *d);
extern void wlc_phy_table_write_nphy(phy_info_t *pi, uint32, uint32, uint32, uint32, const void *);

extern void wlc_phy_resetcca_nphy(phy_info_t *pi);
extern void wlc_phy_cal_perical_nphy_run(phy_info_t *pi, uint8 caltype);
extern void wlc_phy_aci_reset_nphy(phy_info_t *pi);
extern void wlc_phy_pa_override_nphy(phy_info_t *pi, bool en);

extern uint8 wlc_phy_get_chan_freq_range_nphy(phy_info_t *pi, uint chan);
extern void wlc_phy_switch_radio_nphy(phy_info_t *pi, bool on);

extern void wlc_phy_stf_chain_upd_nphy(phy_info_t *pi);

extern void wlc_phy_force_rfseq_nphy(phy_info_t *pi, uint8 cmd);
extern int16 wlc_phy_tempsense_nphy(phy_info_t *pi);

extern uint16 wlc_phy_classifier_nphy(phy_info_t *pi, uint16 mask, uint16 val);

extern void wlc_phy_rx_iq_est_nphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs);

extern void wlc_phy_rx_iq_coeffs_nphy(phy_info_t *pi, uint8 write, nphy_iq_comp_t *comp);
extern void wlc_phy_aci_and_noise_reduction_nphy(phy_info_t *pi);

extern void wlc_phy_rxcore_setstate_nphy(wlc_phy_t *pih, uint8 rxcore_bitmask,
bool enable_phyhangwar);
extern uint8 wlc_phy_rxcore_getstate_nphy(wlc_phy_t *pih);

extern void wlc_phy_txpwrctrl_enable_nphy(phy_info_t *pi, uint8 ctrl_type);
extern void wlc_phy_txpwr_fixpower_nphy(phy_info_t *pi);
extern void wlc_phy_txpwr_apply_nphy(phy_info_t *pi);
extern void wlc_phy_txpwr_papd_cal_nphy(phy_info_t *pi);
extern uint16 wlc_phy_txpwr_idx_get_nphy(phy_info_t *pi);
extern void wlc_phy_store_txindex_nphy(phy_info_t *pi);

/* Get/Set bbmult for nphy */
extern void wlc_phy_get_bbmult_nphy(phy_info_t *pi, int32* ret_ptr);
extern void wlc_phy_set_bbmult_nphy(phy_info_t *pi, uint8 m0, uint8 m1);
extern void wlc_phy_set_oclscd_nphy(phy_info_t *pi);

extern void wlc_phy_get_tx_gain_nphy(phy_info_t *pi, nphy_txgains_t *target_gain);
extern int  wlc_phy_cal_txiqlo_nphy(phy_info_t *pi, nphy_txgains_t target_gain, bool full, bool m);
extern int  wlc_phy_cal_rxiq_nphy(phy_info_t *pi, nphy_txgains_t target_gain, uint8 type, bool d,
	uint8 core_mask);
#ifdef RXIQCAL_FW_WAR
extern int  wlc_phy_cal_rxiq_nphy_fw_war(phy_info_t *pi, nphy_txgains_t target_gain,
	uint8 cal_type, bool debug, uint8 core_mask);
#endif // endif
extern void wlc_phy_txpwr_index_nphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex, bool res);
extern void wlc_phy_rssisel_nphy(phy_info_t *pi, uint8 core, uint8 rssi_type);
extern int  wlc_phy_poll_rssi_nphy(phy_info_t *pi, uint8 rssi_type, int32 *rssi_buf, uint8 nsamps);
extern void wlc_phy_rssi_cal_nphy(phy_info_t *pi);
extern int  wlc_phy_aci_scan_nphy(phy_info_t *pi);
extern nphy_txgains_t wlc_phy_cal_txgainctrl_inttssi_nphy(phy_info_t *pi, int8 target_tssi,
                                                          int8 init_gc_idx);
extern void wlc_phy_cal_txgainctrl_nphy(phy_info_t *pi, int32 dBm_targetpower, bool debug);
extern int
wlc_phy_tx_tone_nphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 mode, uint8, bool);
extern void wlc_phy_stopplayback_nphy(phy_info_t *pi);
extern void wlc_phy_est_tonepwr_nphy(phy_info_t *pi, int32 *qdBm_pwrbuf, uint8 num_samps);
extern void wlc_phy_radio205x_vcocal_nphy(phy_info_t *pi);
extern void wlc_phy_radio205x_check_vco_cal_nphy(phy_info_t *pi);
extern void wlc_phy_init_hw_antsel(phy_info_t *pi);
extern void
wlc_phy_rfctrlintc_override_nphy(phy_info_t *pi, uint8 field, uint16 value,
	uint8 core_code);
#if defined(BCMDBG) || defined(WLTEST)
extern int wlc_phy_freq_accuracy_nphy(phy_info_t *pi, int channel);
#endif // endif
/* Rx desense Module */
#if defined(RXDESENS_EN)
extern int wlc_nphy_set_rxdesens(wlc_phy_t *ppi, int32 int_val);
#endif // endif

#define NPHY_TESTPATTERN_BPHY_EVM   0
#define NPHY_TESTPATTERN_BPHY_RFCS  1

#ifdef BCMDBG
extern void wlc_phy_setinitgain_nphy(phy_info_t *pi, uint16 init_gain);
extern void wlc_phy_sethpf1gaintbl_nphy(phy_info_t *pi, int8 maxindex);
extern void wlc_phy_cal_reset_nphy(phy_info_t *pi, uint32 reset_type);
#endif // endif

#if defined(WLTEST)
extern void wlc_phy_bphy_testpattern_nphy(phy_info_t *pi, uint8 testpattern, bool enable, bool);
extern uint32 wlc_phy_cal_sanity_nphy(phy_info_t *pi);
extern void wlc_phy_test_scraminit_nphy(phy_info_t *pi, int8 init);
extern void wlc_phy_gpiosel_nphy(phy_info_t *pi, uint16 sel);
extern int8 wlc_phy_test_tssi_nphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs);
#endif // endif

/* ACPHY : ACI, BT Desense (start) */
extern void wlc_phydump_aci_acphy(phy_info_t *pi, struct bcmstrbuf *b);
/* ACPHY : ACI, BT Desense (end) */

extern uint32 wlc_phy_txpower_est_power_nphy(phy_info_t *pi);

extern void wlc_phy_lpf_hpc_override_nphy(phy_info_t *pi, bool setup_not_cleanup);
void wlc_phy_set_trloss_reg_acphy(phy_info_t *pi, int8 core);
extern uint8 wlc_phy_calc_extra_init_gain_acphy(phy_info_t *pi, uint8 extra_gain_3dB,
                                        rxgain_t rxgain[]);
extern int16 wlc_phy_tempsense_acphy(phy_info_t *pi);
extern void wlc_phy_rfctrl_override_rxgain_acphy(phy_info_t *pi, uint8 restore,
                                                 rxgain_t rxgain[], rxgain_ovrd_t rxgain_ovrd[]);
#ifdef POWPERCHANNL
extern void BCMATTACHFN(wlc_phy_tx_target_pwr_per_channel_limit_acphy)(phy_info_t *pi);
extern void wlc_phy_tx_target_pwr_per_channel_decide_run_acphy(phy_info_t *pi);
#endif /* POWPERCHANNL */
#if defined(BCMDBG)
extern void wlc_phy_force_gainlevel_acphy(phy_info_t *pi, int16 int_val);
#endif // endif
#if defined(BCMDBG)
extern void wlc_phy_force_fdiqi_acphy(phy_info_t *pi, uint16 int_val);
#endif /* BCMDBG */

#ifdef PHYMON
extern int wlc_phycal_state_nphy(phy_info_t *pi, void* buff, int len);
#endif /* PHYMON */

extern void wlc_phy_txpwr_ppr_bit_ext_srom13_mcs8to11(phy_info_t *pi, ppr_vht_mcs_rateset_t* vht,
	uint8 bshift);
extern void wlc_phy_txpwr_apply_srom13_2g_bw2040(phy_info_t *pi, chanspec_t chanspec,
	uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr);
extern void wlc_phy_txpwr_apply_srom13_5g_bw40(phy_info_t *pi, uint8 band,
	ppr_t *tx_srom_max_pwr, sr13_ppr_5g_rateset_t *rate5g);
extern void wlc_phy_txpwr_apply_srom13_5g_bw80(phy_info_t *pi, uint8 band, chanspec_t chanspec,
	ppr_t *tx_srom_max_pwr, sr13_ppr_5g_rateset_t *rate5g);
extern void wlc_phy_txpwr_apply_srom13_5g_bw160(phy_info_t *pi, uint8 band, chanspec_t chanspec,
	uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr, sr13_ppr_5g_rateset_t *rate5g);
extern void wlc_phy_txpwr_apply_srom13(phy_info_t *pi, uint8 band, chanspec_t chanspec,
	uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr);

#if defined(WLTEST)
typedef enum {
	AV,
	VMID
} wlc_avvmid_t;
extern void wlc_phy_set_avvmid_acphy(phy_info_t *pi, uint8 *avvmid, wlc_avvmid_t avvmid_type);
extern void wlc_phy_get_avvmid_acphy(phy_info_t *pi, int32 *ret_int_ptr, wlc_avvmid_t avvmid_type,
		uint8 *core_sub_band);
#endif // endif

#if defined(PHYCAL_CACHING)
/* Get the calcache entry given the chanspec */
extern ch_calcache_t *wlc_phy_get_chanctx(phy_info_t *phi, chanspec_t chanspec);
#endif /* PHYCAL_CACHING */

/*
 * This helpful macro calculate the number of registers to access for a particular
 * array of addresses and values. There are twice as many array elements as registers,
 * since each register requires both an address and a value.
 */
#define WLC_BULK_SZ(addrvals) (sizeof(addrvals) / (2 * sizeof(addrvals[0])))

extern void wlc_phy_rfctrl_override_nphy_rev19(phy_info_t *pi, uint16 field, uint16 value,
	uint8 core_mask, uint8 off, uint8 override_id);
extern void wlc_phy_txpwr_papd_cal_nphy_dcs(phy_info_t *pi);
extern void wlc_phy_set_rfseq_nphy(phy_info_t *pi, uint8 cmd, uint8 *evts, uint8 *dlys, uint8 len);
extern uint16 wlc_phy_read_lpf_bw_ctl_nphy(phy_info_t *pi, uint16 offset);

extern uint32* wlc_phy_get_epa_gaintbl_nphy(phy_info_t *pi);

/* split radio out of wlc_phy_n.c to wlc_phy_radio_n.c */

/* channel info structure for nphy rev3-6 */
struct _chan_info_nphy_radio205x {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint8  RF_SYN_pll_vcocal1;
	uint8 RF_SYN_pll_vcocal2;
	uint8 RF_SYN_pll_refdiv;
	uint8 RF_SYN_pll_mmd2;
	uint8 RF_SYN_pll_mmd1;
	uint8 RF_SYN_pll_loopfilter1;
	uint8 RF_SYN_pll_loopfilter2;
	uint8 RF_SYN_pll_loopfilter3;
	uint8 RF_SYN_pll_loopfilter4;
	uint8 RF_SYN_pll_loopfilter5;
	uint8 RF_SYN_reserved_addr27;
	uint8 RF_SYN_reserved_addr28;
	uint8 RF_SYN_reserved_addr29;
	uint8 RF_SYN_logen_VCOBUF1;
	uint8 RF_SYN_logen_MIXER2;
	uint8 RF_SYN_logen_BUF3;
	uint8 RF_SYN_logen_BUF4;
	uint8 RF_RX0_lnaa_tune;
	uint8 RF_RX0_lnag_tune;
	uint8 RF_TX0_intpaa_boost_tune;
	uint8 RF_TX0_intpag_boost_tune;
	uint8 RF_TX0_pada_boost_tune;
	uint8 RF_TX0_padg_boost_tune;
	uint8 RF_TX0_pgaa_boost_tune;
	uint8 RF_TX0_pgag_boost_tune;
	uint8 RF_TX0_mixa_boost_tune;
	uint8 RF_TX0_mixg_boost_tune;
	uint8 RF_RX1_lnaa_tune;
	uint8 RF_RX1_lnag_tune;
	uint8 RF_TX1_intpaa_boost_tune;
	uint8 RF_TX1_intpag_boost_tune;
	uint8 RF_TX1_pada_boost_tune;
	uint8 RF_TX1_padg_boost_tune;
	uint8 RF_TX1_pgaa_boost_tune;
	uint8 RF_TX1_pgag_boost_tune;
	uint8 RF_TX1_mixa_boost_tune;
	uint8 RF_TX1_mixg_boost_tune;
	uint16 PHY_BW1a;        /* 4322 register values */
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
};

/* channel info structure for dual-band 2057 (paired w/ nphy rev7+) */
struct _chan_info_nphy_radio2057 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint8  RF_vcocal_countval0;
	uint8  RF_vcocal_countval1;
	uint8  RF_rfpll_refmaster_sparextalsize;
	uint8  RF_rfpll_loopfilter_r1;
	uint8  RF_rfpll_loopfilter_c2;
	uint8  RF_rfpll_loopfilter_c1;
	uint8  RF_cp_kpd_idac;
	uint8  RF_rfpll_mmd0;
	uint8  RF_rfpll_mmd1;
	uint8  RF_vcobuf_tune;
	uint8  RF_logen_mx2g_tune;
	uint8  RF_logen_mx5g_tune;
	uint8  RF_logen_indbuf2g_tune;
	uint8  RF_logen_indbuf5g_tune;
	uint8  RF_txmix2g_tune_boost_pu_core0;
	uint8  RF_pad2g_tune_pus_core0;
	uint8  RF_pga_boost_tune_core0;
	uint8  RF_txmix5g_boost_tune_core0;
	uint8  RF_pad5g_tune_misc_pus_core0;
	uint8  RF_lna2g_tune_core0;
	uint8  RF_lna5g_tune_core0;
	uint8  RF_txmix2g_tune_boost_pu_core1;
	uint8  RF_pad2g_tune_pus_core1;
	uint8  RF_pga_boost_tune_core1;
	uint8  RF_txmix5g_boost_tune_core1;
	uint8  RF_pad5g_tune_misc_pus_core1;
	uint8  RF_lna2g_tune_core1;
	uint8  RF_lna5g_tune_core1;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
};

/* channel info structure for single-band 2057 rev5 (common for 5357[ab]0) */
struct _chan_info_nphy_radio2057_rev5 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	uint8  RF_vcocal_countval0;
	uint8  RF_vcocal_countval1;
	uint8  RF_rfpll_refmaster_sparextalsize;
	uint8  RF_rfpll_loopfilter_r1;
	uint8  RF_rfpll_loopfilter_c2;
	uint8  RF_rfpll_loopfilter_c1;
	uint8  RF_cp_kpd_idac;
	uint8  RF_rfpll_mmd0;
	uint8  RF_rfpll_mmd1;
	uint8  RF_vcobuf_tune;
	uint8  RF_logen_mx2g_tune;
	uint8  RF_logen_indbuf2g_tune;
	uint8  RF_txmix2g_tune_boost_pu_core0;
	uint8  RF_pad2g_tune_pus_core0;
	uint8  RF_lna2g_tune_core0;
	uint8  RF_txmix2g_tune_boost_pu_core1;
	uint8  RF_pad2g_tune_pus_core1;
	uint8  RF_lna2g_tune_core1;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
};

typedef struct _chan_info_nphy_radio2057 chan_info_nphy_radio2057_t;
typedef struct _chan_info_nphy_radio2057_rev5 chan_info_nphy_radio2057_rev5_t;
typedef struct _chan_info_nphy_radio205x chan_info_nphy_radio205x_t;
typedef enum {
ACPHY_LP_CHIP_LVL_OPT,
ACPHY_LP_PHY_LVL_OPT,
ACPHY_LP_RADIO_LVL_OPT
} acphy_lp_opt_levels_t;

void
wlc_phy_chanspec_radio2057_setup(phy_info_t *pi, const chan_info_nphy_radio2057_t *ci,
                                 const chan_info_nphy_radio2057_rev5_t *ci2);

#define CHIPID_43236X_FAMILY(pi)	((CHIPID((pi)->sh->chip) == BCM43236_CHIP_ID) || \
			           (CHIPID((pi)->sh->chip) == BCM43235_CHIP_ID) || \
			           (CHIPID((pi)->sh->chip) == BCM43234_CHIP_ID) || \
			           (CHIPID((pi)->sh->chip) == BCM43238_CHIP_ID))

#define LCNXN_SWCTRL_MASK_DEFAULT     0xFFF /* All line under WLAN control */
#define LCNXN_SWCTRL_MASK_43241IPAAGB 0xFF9 /* RF_SW_CTRL_[2-1] are under BT control */
#define LCNXN_SWCTRL_MASK_43241IPAAGB_eLNA 0xFD9 /* RF_SW_CTRL_[5,2-1] are under BT control */
#define LCNXN_SWCTRL_MASK_43242USBREF 0xFF3 /* RF_SW_CTRL_[3-2] are under BT control */
#define LCNXN_SWCTRL_MASK_4324B1EFOAGB 0xFDC /* RF_SW_CTRL_[5,1-0] are under BT control */

extern void wlc_phy_trigger_cals_for_btc_adjust(phy_info_t *pi);

extern int32 wlc_nphy_tssi_read_iovar(phy_info_t *pi);
extern void wlc_phy_txpwrctrl_idle_tssi_nphy(phy_info_t *pi);
extern void wlc_phy_lcnxn_rx2tx_stallwindow_nphy(phy_info_t *pi, uint8 STALLON);

#if defined(ACI_DBG_PRINTS_EN)
extern void wlc_phy_aci_noise_print_values_nphy(phy_info_t *pi);
#endif // endif

/* PHY specific - Modular attach functions */
extern void BCMATTACHFN(wlc_phy_interference_mode_attach_nphy)(phy_info_t *pi);
extern void BCMATTACHFN(wlc_phy_interference_mode_attach_lcn20phy)(phy_info_t *pi);

extern void
wlc_phy_write_table_ext(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblId,
    uint16 tblOffset, uint16 tblDataWide, uint16 tblDataHi, uint16 tblDataLo);
extern void
wlc_phy_read_table_ext(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblId,
    uint16 tblOffset, uint16 tblDataWide, uint16 tblDataHi, uint16 tblDataLo);
/* *************************** Move or Remove later ************************** */

/* iovar/ioctl */
int wlc_phy_ioctl_dispatch(phy_info_t *pi, int cmd, int len, void *arg, bool *ta_ok);

/* srom */
bool wlc_sslpnphy_txpwr_srom_read(phy_info_t *pi);

#define F_TRACE(pi) \
	printf("wl%d: %s lr %p\n", PI_INSTANCE(pi), __FUNCTION__, CALL_SITE)

/* *************************** Move or Remove later ************************** */

/* REG CACHE infra */
#define RADIOREGS_TXIQCAL	0
#define RADIOREGS_RXIQCAL	1
#define RADIOREGS_PAPDCAL	2
#define PHYREGS_TXIQCAL		3
#define PHYREGS_RXIQCAL		4
#define PHYREGS_PAPDCAL		5
#define RADIOREGS_AFECAL        6
#define PHYREGS_TEMPSENSE_VBAT 7
#define RADIOREGS_TEMPSENSE_VBAT 8
#define RADIOREGS_TSSI  9
#define PHYREGS_TSSI 10

extern void
BCMATTACHFN(phy_ac_reg_cache_detach)(phy_info_acphy_t *pi_ac);
extern int phy_ac_reg_cache_save(phy_info_acphy_t *pi_ac, uint16 id);
extern int phy_ac_reg_cache_restore(phy_info_acphy_t *pi_ac, int id);
extern int phy_ac_reg_cache_save_percore(phy_info_acphy_t *pi_ac, uint16 id, uint8 core);
extern int phy_ac_reg_cache_restore_percore(phy_info_acphy_t *pi_ac, int id, uint8 core);

#if !defined(BW_20MHZ)
#define BW_20MHZ                1
#define BW_40MHZ                2
#define BW_80MHZ                3
#define BW_160MHZ               4
#endif // endif

/* %%%%%% Calibration, ACI, noise/rssi measurement */
extern void wlc_phy_noise_calc(phy_info_t *pi, uint32 *cmplx_pwr, int8 *pwr_ant,
                               uint8 extra_gain_1dB);

#endif	/* _wlc_phy_int_h_ */
