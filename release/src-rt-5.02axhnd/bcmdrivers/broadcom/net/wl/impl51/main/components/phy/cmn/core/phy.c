/*
 * PHY Core module implementation
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
 * $Id: phy.c 733511 2017-11-28 21:51:26Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_rstr.h>
#include <phy_api.h>
#include "phy_iovt.h"
#include "phy_ioct.h"
#include "phy_type.h"
#include "phy_type_disp.h"
#include <phy_cmn.h>
#include <phy_mem.h>
#include <bcmutils.h>
#include <phy_init.h>
#include <phy_wd.h>
#include <phy_ana.h>
#include <phy_ana_api.h>
#include <phy_btcx_api.h>
#include <phy_et.h>
#include <phy_radio.h>
#include <phy_radio_api.h>
#include <phy_tbl.h>
#include <phy_tpc.h>
#ifdef WLC_TXPWRCAP
#include <phy_txpwrcap.h>
#endif // endif
#include <phy_radar.h>
#include <phy_antdiv.h>
#include <phy_noise.h>
#include <phy_temp.h>
#include <phy_rssi.h>
#include <phy_btcx.h>
#include <phy_txiqlocal.h>
#include <phy_rxiqcal.h>
#include <phy_papdcal.h>
#include <phy_vcocal.h>
#include <phy_chanmgr.h>
#include <phy_chanmgr_api.h>
#include <phy_cache.h>
#include <phy_calmgr.h>
#include <phy_chanmgr_notif.h>
#include <phy_fcbs.h>
#include <phy_lpc.h>
#include <phy_dsi.h>
#include <phy_dccal.h>
#include <phy_tof.h>
#include <phy_hirssi.h>
#include <phy.h>
#include <phy_nap.h>
#include <phy_hecap.h>
#include <phy_rxgcrs.h>
#include <phy_vasip.h>
#include <phy_smc.h>
#include <phy_stf.h>
#include <phy_misc_api.h>

#include <phy_utils_var.h>
#include <phy_temp_api.h>

#ifndef ALL_NEW_PHY_MOD
/* TODO: remove these lines... */
#include <wlc_phy_int.h>
#include <wlc_phy_hal.h>
#endif // endif

#define PHY_TXPWR_MIN		9	/* default min tx power */

#define PHY_WREG_LIMIT	24	/* number of consecutive phy register write before a readback */
#define PHY_WREG_LIMIT_VENDOR 1	/* num of consec phy reg write before a readback for vendor */

/* local functions */
static void wlc_phy_srom_attach(phy_info_t *pi, int bandtype);
static void wlc_phy_std_params_attach(phy_info_t *pi);
static int _phy_init(phy_init_ctx_t *ctx);
static void phy_register_dumps(phy_info_t *pi);
static void phy_init_done(phy_info_t *pi);

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
/* phydump page infra */
static void phyreg_page_parser(phy_info_t *pi, phy_regs_t *reglist, struct bcmstrbuf *b);
#endif /* BCMDBG_PHYDUMP */

/* returns a pointer to per interface instance data */
shared_phy_t *
BCMATTACHFN(wlc_phy_shared_attach)(shared_phy_params_t *shp)
{
	shared_phy_t *sh;
	int ref_count = 0;

#ifdef EVENT_LOG_COMPILE
	/* First thing to do.. initialize the PHY_ERROR tag's attributes. */
	/* This is the attach function for the PHY component. */
	event_log_tag_start(EVENT_LOG_TAG_PHY_ERROR, EVENT_LOG_SET_ERROR,
		EVENT_LOG_TAG_FLAG_LOG);
#endif // endif

	/* allocate wlc_info_t state structure */
	if ((sh = (shared_phy_t*) MALLOCZ(shp->osh, sizeof(shared_phy_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			shp->unit, __FUNCTION__, MALLOCED(shp->osh)));
		goto fail;
	}

	/* OBJECT REGISTRY: check if shared key has value already stored */
	sh->sromi = (phy_srom_info_t *)wlapi_obj_registry_get(shp->physhim, OBJR_PHY_CMN_SROM_INFO);
	if (sh->sromi == NULL) {
		if ((sh->sromi = (phy_srom_info_t *)MALLOCZ(shp->osh,
			sizeof(phy_srom_info_t))) == NULL) {

			PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				shp->unit, __FUNCTION__, MALLOCED(shp->osh)));
			goto fail;
		}

		/* OBJECT REGISTRY: We are the first instance, store value for key */
		wlapi_obj_registry_set(shp->physhim, OBJR_PHY_CMN_SROM_INFO, sh->sromi);
	}
	/* OBJECT REGISTRY: Reference the stored value in both instances */
	ref_count = wlapi_obj_registry_ref(shp->physhim, OBJR_PHY_CMN_SROM_INFO);
	ASSERT(ref_count <= MAX_RSDB_MAC_NUM);
	BCM_REFERENCE(ref_count);

	sh->osh		= shp->osh;
	sh->sih		= shp->sih;
	sh->physhim	= shp->physhim;
	sh->unit	= shp->unit;
	sh->corerev	= shp->corerev;
	sh->vid		= shp->vid;
	sh->did		= shp->did;
	sh->chip	= shp->chip;
	sh->chiprev	= shp->chiprev;
	sh->chippkg	= shp->chippkg;
	sh->sromrev	= shp->sromrev;
	sh->boardtype	= shp->boardtype;
	sh->boardrev	= shp->boardrev;
	sh->boardvendor	= shp->boardvendor;
	sh->boardflags	= shp->boardflags;
	sh->boardflags2	= shp->boardflags2;
	sh->boardflags4	= shp->boardflags4;
#if defined(WL_EAP_BOARD_RF_5G_FILTER)
	sh->board5gfilter	= shp->board5gfilter;
#endif /* WL_EAP_BOARD_RF_5G_FILTER */
	sh->bustype	= shp->bustype;
	sh->buscorerev	= shp->buscorerev;
	strncpy(sh->vars_table_accessor, shp->vars_table_accessor, sizeof(sh->vars_table_accessor));
	sh->vars_table_accessor[sizeof(sh->vars_table_accessor)-1] = '\0';

	/* create our timers */
	sh->fast_timer	= PHY_SW_TIMER_FAST;
	sh->slow_timer	= PHY_SW_TIMER_SLOW;
	sh->glacial_timer = PHY_SW_TIMER_GLACIAL;

	/* reset cal scheduler */
	sh->scheduled_cal_time = 0;

	/* ACI mitigation mode is auto by default */
	sh->interference_mode = WLAN_AUTO;
	/* sh->snr_mode = SNR_ANT_MERGE_MAX; */
	sh->rssi_mode = RSSI_ANT_MERGE_MAX;
	/* enabling BPHY MRC by default.
	 * use iovar "wl phy_bphymrc" to disable on wl down
	 */
	sh->bphymrc_en = 1;
	return sh;
fail:
	wlc_phy_shared_detach(sh);
	return NULL;
}

void
BCMATTACHFN(wlc_phy_shared_detach)(shared_phy_t *sh)
{
	if (sh != NULL) {
		/* phy_head must have been all detached */
		if (sh->phy_head) {
			PHY_ERROR(("wl%d: %s non NULL phy_head\n", sh->unit, __FUNCTION__));
			ASSERT(!sh->phy_head);
		}
		if (sh->sromi != NULL) {
			if (wlapi_obj_registry_unref(sh->physhim, OBJR_PHY_CMN_SROM_INFO) == 0) {
				wlapi_obj_registry_set(sh->physhim, OBJR_PHY_CMN_SROM_INFO, NULL);
				MFREE(sh->osh, sh->sromi, sizeof(phy_srom_info_t));
			}
		}
		MFREE(sh->osh, sh, sizeof(shared_phy_t));
	}
}

void
wlc_phy_set_shmdefs(wlc_phy_t *ppi, const shmdefs_t *shmdefs)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->shmdefs = shmdefs;
}

/* attach/detach the PHY Core module to the system. */
phy_info_t *
BCMATTACHFN(phy_module_attach)(shared_phy_t *sh, void *regs, int bandtype, char *vars)
{
	phy_info_t *pi = sh->phy_head;
	uint32 sflags = 0;
	uint phyversion;
	osl_t *osh = sh->osh;
	int ret = 0;

	PHY_TRACE(("wl: %s(%p, %d, %p)\n", __FUNCTION__, regs, bandtype, sh));

	sflags = si_core_sflags(sh->sih, 0, 0);

	if (BAND_5G(bandtype)) {
		/* WAR: skip band capability check for phy_maj44_min0,
		 * main slice band capabilities are not set properly, to be fixed in phy_maj44_min1
		 */
		if (pi && !ACREV_IS(pi->pubpi->phy_rev, 44)) {
			if ((sflags & (SISF_5G_PHY | SISF_DB_PHY)) == 0) {
				PHY_ERROR(("wl%d: %s: No phy available for 5G\n",
				          sh->unit, __FUNCTION__));
				return NULL;
			}
		}
	}

	/* Figure out if we have a phy for the requested band and attach to it */
	if ((sflags & SISF_DB_PHY) && (pi = sh->phy_head)) {
		pi->vars = vars;
		/* For the second band in dualband phys, load the band specific
		 * NVRAM parameters
		  * The second condition excludes UNO3 inorder to
		  * keep the device id as 0x4360 (dual band).
		  * Purely to be backward compatible to previous UNO3 NVRAM file.
		  *
		 */
		/* For the second band in dualband phys, just bring the core back out of reset */
		wlapi_bmac_corereset(pi->sh->physhim, pi->pubpi->coreflags);

		pi->refcnt++;
		goto exit;
	}

	/* ONLY common PI is allocated. pi->u.pi_xphy is not available yet */
	if ((pi = (phy_info_t *)MALLOCZ(osh, sizeof(phy_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
		return NULL;
	}
	pi->sh = sh; /* Assign sh so that phy_malloc can be used from here on */

	if ((pi->pubpi = phy_malloc(pi, sizeof(wlc_phy_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced pubpi %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->pubpi_ro = phy_malloc(pi, sizeof(wlc_phy_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced pubpi_ro %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->interf = phy_malloc(pi, sizeof(interference_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced interf %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->pwrdet = phy_malloc(pi, sizeof(srom_pwrdet_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced pwrdet %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->def_cal_info = phy_malloc(pi, sizeof(phy_cal_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced def_cal_info %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

#ifdef ENABLE_FCBS
	if ((pi->phy_fcbs = phy_malloc(pi, sizeof(fcbs_info))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced phy_fcbs %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}
#endif /* ENABLE_FCBS */

	if ((pi->fem2g = phy_malloc(pi, sizeof(srom_fem_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced fem2g %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->fem5g = phy_malloc(pi, sizeof(srom_fem_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced fem5g %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

#ifdef WLSRVSDB
	if ((pi->srvsdb_state = phy_malloc(pi, sizeof(srvsdb_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced srvsdb_state %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}
#endif /* WLSRVSDB */

	if ((pi->pi_fptr = phy_malloc(pi, sizeof(phy_func_ptr_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced pi_fptr %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->ppr = phy_malloc(pi, sizeof(ppr_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced ppr %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	if ((pi->tx_power_offset = ppr_create(pi->sh->osh, ppr_get_max_bw())) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced tx_power_offset %d bytes\n", sh->unit,
				__FUNCTION__, ppr_size(ppr_get_max_bw())));
		goto err;
	}

	if ((pi->pwrdet_ac = phy_malloc(pi, (SROMREV(sh->sromrev) >= 12 ? sizeof(srom12_pwrdet_t)
					       : sizeof(srom11_pwrdet_t)))) == NULL) {
	    PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", sh->unit,
	       __FUNCTION__, MALLOCED(sh->osh)));
	    goto err;
	}

	if ((pi->pdpi = phy_malloc(pi, sizeof(*pi->pdpi))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced pdpi %d bytes\n",
			sh->unit, __FUNCTION__, MALLOCED(osh)));
		goto err;
	}

	if ((pi->ff = phy_malloc(pi, sizeof(phy_feature_flags_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced phy_features_enab %d bytes\n",
			sh->unit, __FUNCTION__, MALLOCED(osh)));
		goto err;
	}

	if ((pi->ff->_feature_enab = (uint8 *) phy_malloc(pi,
		CEIL(PHY_FEATURE_MAX_IDX, 8))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced phy_features_enab %d bytes\n",
			sh->unit, __FUNCTION__, MALLOCED(osh)));
		goto err;
	}

	pi->regs = (d11regs_t *)regs;
	pi->vars = vars;
	ret = d11regs_select_offsets_tbl(&pi->regoffsets, pi->sh->corerev);
	if (ret) {
		PHY_ERROR(("%s failed, regs for rev not found\n", __FUNCTION__));
		goto err;
	}

	/* point pi->sromi to phy_sh->sromi */
	pi->sromi = pi->sh->sromi;

	/* Good phy, increase refcnt and put it in list */
	pi->refcnt++;
	pi->next = pi->sh->phy_head;
	sh->phy_head = pi;

	/* set init power on reset to TRUE */
	pi->phy_init_por = TRUE;

	if ((pi->sh->boardvendor == VENDOR_APPLE) &&
	    (pi->sh->boardtype == 0x0093)) {
		pi->phy_wreg_limit = PHY_WREG_LIMIT_VENDOR;
	}
	else {
		pi->phy_wreg_limit = PHY_WREG_LIMIT;
	}
	if (BAND_2G(bandtype) && (sflags & SISF_2G_PHY)) {
		/* Set the sflags gmode indicator */
		pi->pubpi->coreflags = SICF_GMODE;
	}

	/* get the phy type & revision */
	/* Note: corereset seems to be required to get the phyversion read correctly */
	wlapi_bmac_corereset(pi->sh->physhim, pi->pubpi->coreflags);
	phyversion = R_REG(osh, D11_PHY_REG_0(pi));
	pi->pubpi->phy_type = PHY_TYPE(phyversion);
	pi->pubpi->phy_rev = phyversion & PV_PV_MASK;
	pi->pubpi->slice = (uint8)wlapi_si_coreunit(pi->sh->physhim);

	/* Read the fabid */
	pi->fabid = si_fabid(GENERIC_PHY_INFO(pi)->sih);

	if (((pi->sh->chip == BCM43235_CHIP_ID) ||
	     (pi->sh->chip == BCM43236_CHIP_ID) ||
	     (pi->sh->chip == BCM43238_CHIP_ID) ||
	     (pi->sh->chip == BCM43234_CHIP_ID)) &&
	    ((pi->sh->chiprev == 2) || (pi->sh->chiprev == 3))) {
		pi->pubpi->phy_rev = 9;
	}

	/* LCNXN */
	if (pi->pubpi->phy_type == PHY_TYPE_LCNXN) {
		pi->pubpi->phy_type = PHY_TYPE_N;
		pi->pubpi->phy_rev += LCNXN_BASEREV;
	}

	/* Default to 1 core. Each PHY specific attach should initialize it
	 * to PHY/chip specific.
	 */
	pi->pubpi->phy_corenum = PHY_CORE_NUM_1;
	pi->pubpi->ana_rev = (phyversion & PV_AV_MASK) >> PV_AV_SHIFT;

	if (!VALID_PHYTYPE(pi)) {
		PHY_ERROR(("wl%d: %s: invalid phy_type %d\n",
		          sh->unit, __FUNCTION__, pi->pubpi->phy_type));
		goto err;
	}

	/* default channel and channel bandwidth is 20 MHZ */
	pi->bw = WL_CHANSPEC_BW_20;
	pi->radio_chanspec = BAND_2G(bandtype) ? CH20MHZ_CHSPEC(1) : CH20MHZ_CHSPEC(36);

	/* attach nvram driven variables */
	wlc_phy_srom_attach(pi, bandtype);

	/* ######## Attach process start ######## */

	/* ======== Attach infrastructure services ======== */
#ifdef PHY_DBG_ENABLED
	/* Attach debug/dump registry module - MUST BE THE FIRST! */
	if ((pi->dbgi = phy_dbg_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_dbg_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* PHY_DBG_ENABLED */
	/* Attach PHY Common info */
	if ((pi->cmni = phy_cmn_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_cmn_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach PHY type specific implementation dispatch info */
	if ((pi->typei = phy_type_disp_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_type_disp_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach INIT control module */
	if ((pi->initi = phy_init_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_init_attach failed\n", __FUNCTION__));
		goto err;
	}
#ifdef PHYCAL_CACHING
	/* Attach Channel Manager Notification module */
	if ((pi->chanmgr_notifi = phy_chanmgr_notif_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_chanmgr_notif_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif
	/* Attach CACHE module */
	if ((pi->cachei = phy_cache_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_cache_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach WATCHDOG module */
	if ((pi->wdi = phy_wd_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_wd_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach CALibrationManaGeR module */
	if ((pi->calmgri = phy_calmgr_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_calmgr_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach STF module */
	if ((pi->stfi = phy_stf_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_stf_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* update standard configuration params to defaults */
	wlc_phy_std_params_attach(pi);

	/* ======== Attach PHY specific layer ======== */
	/* Attach PHY Core type specific implementation */
	if (pi->typei != NULL &&
	    (*(phy_type_info_t **)(uintptr)&pi->u =
	     phy_type_attach(pi->typei, bandtype)) == NULL) {
		PHY_ERROR(("%s: phy_type_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* ======== Attach modules' common layer ======== */

	/* Attach ANAcore control module */
	if ((pi->anai = phy_ana_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_ana_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RADIO control module */
	if ((pi->radioi = phy_radio_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_radio_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach PHYTableInit module */
	if ((pi->tbli = phy_tbl_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_tbl_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach TxPowerCtrl module */
	if ((pi->tpci = phy_tpc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_tpc_attach failed\n", __FUNCTION__));
		goto err;
	}

#if defined(WLC_TXPWRCAP) && !defined(WLC_TXPWRCAP_DISABLED)
	/* Attach TxPowerCap module */
	if ((pi->txpwrcapi = phy_txpwrcap_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_txpwrcap_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif

#if defined(AP) && defined(RADAR)
	/* Attach RadarDetect module */
	if ((pi->radari = phy_radar_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_radar_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif

	/* Attach ANTennaDIVersity module */
	if ((pi->antdivi = phy_antdiv_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_antdiv_attach failed\n", __FUNCTION__));
		goto err;
	}

#ifndef WLC_DISABLE_ACI
	/* Attach NOISE module */
	if ((pi->noisei = phy_noise_attach(pi, bandtype)) == NULL) {
		PHY_ERROR(("%s: phy_noise_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif

	/* Attach TEMPerature sense module */
	if ((pi->tempi = phy_temp_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_temp_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RSSICompute module */
	if ((pi->rssii = phy_rssi_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rssi_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach BlueToothCoExistence module */
	if ((pi->btcxi = phy_btcx_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_btcx_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach TxIQLOCal module */
	if ((pi->txiqlocali = phy_txiqlocal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_txiqlocal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RxIQCal module */
	if ((pi->rxiqcali = phy_rxiqcal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rxiqcal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach PAPDCal module */
	if ((pi->papdcali = phy_papdcal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_papdcal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach VCOCal module */
	if ((pi->vcocali = phy_vcocal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_vcocal_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach CHannelManaGeR module */
	if ((pi->chanmgri = phy_chanmgr_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_chanmgr_attach failed\n", __FUNCTION__));
		goto err;
	}
#ifdef ENABLE_FCBS
	/* Attach FCBS module */
	if ((pi->fcbsi = phy_fcbs_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_fcbs_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* ENABLE_FCBS */
#ifdef WL_LPC
	/* Attach LPC module */
	if ((pi->lpci = phy_lpc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_lpc_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* WL_LPC */
	/* Attach MISC module */
	if ((pi->misci = phy_misc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_misc_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach TSSI Cal module */
	if ((pi->tssicali = phy_tssical_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_tssi_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RXGCRS module */
	if ((pi->rxgcrsi = phy_rxgcrs_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rxgcrs_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Attach RXSPUR module */
	if ((pi->rxspuri = phy_rxspur_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_rxspur_attach failed\n", __FUNCTION__));
		goto err;
	}

#ifdef SAMPLE_COLLECT
	/* Attach sample collect module */
	if ((pi->sampi = phy_samp_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_samp_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* SAMPLE_COLLECT */

#ifdef WL_DSI
	/* Attach DeepSleepInit module */
	if ((pi->dsii = phy_dsi_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_dsi_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* WL_DSI */

#ifdef WL_MU_RX
	/* Attach MU-MIMO module */
	if ((pi->mui = phy_mu_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_mu_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif  /* WL_MU_RX */

#ifdef OCL
	/* Attach OCL module */
	if ((pi->ocli = phy_ocl_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_ocl_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif  /* OCL */

#ifdef WL11AX
	/* Attach HECAP module if PHY can support it */
	if ((pi->hecapi = phy_hecap_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_hecap_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* WL11AX */

	/* Attach MU-MIMO module */
	if ((pi->dccali = phy_dccal_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_dccal_attach failed\n", __FUNCTION__));
		goto err;
	}

#ifdef WL_PROXDETECT
	/* Attach TOF module */
	if ((pi->tofi = phy_tof_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_tof_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif  /* WL_PROXDETECT */

#if defined(WL_NAP) && !defined(WL_NAP_DISABLED)
	/* Attach nap module */
	if ((pi->napi = phy_nap_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_nap_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* WL_NAP */

	/* Attach HiRSSIeLNABypass module */
	if ((pi->hirssii = phy_hirssi_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_hirssi_attach failed\n", __FUNCTION__));
		goto err;
	}
#if defined(WL_ETMODE) && !defined(WL_ETMODE_DISABLED)
	/* Attach envelope tracking module */
	if ((pi->eti = phy_et_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_et_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* WL_ETMODE && !WL_ETMODE_DISABLED */
#ifdef RADIO_HEALTH_CHECK
	/* Attach health check module */
	if ((pi->hci = phy_hc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_hc_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* RADIO_HEALTH_CHECK */

#ifdef WLVASIP
	/* Attach vasip check module */
	if ((pi->vasipi = phy_vasip_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_vasip_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* WLVASIP */

#ifdef WLSMC
	/* Attach smc check module */
	if ((pi->smci = phy_smc_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_smc_attach failed\n", __FUNCTION__));
		goto err;
	}
#endif /* WLSMC */

	/* ...Attach other modules... */

	/* ======== Attach modules' PHY specific layer ======== */

	/* Register PHY type implementation layer to common layer */
	if (pi->typei != NULL &&
	    phy_type_register_impl(pi->typei, bandtype) != BCME_OK) {
		PHY_ERROR(("%s: phy_type_register_impl failed\n", __FUNCTION__));
		goto err;
	}

#ifdef WLTXPWR_CACHE
	if ((pi->txpwr_cache = wlc_phy_txpwr_cache_create(pi->sh->osh)) == NULL) {
		PHY_ERROR(("%s: Init phy txpwr_cache failed\n", __FUNCTION__));
		goto err;
	}
#endif // endif

	/* ######## Attach process end ######## */

	/* register reset fn */
	if (phy_init_add_init_fn(pi->initi, _phy_init, pi, PHY_INIT_PHYIMPL) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto err;
	}

	/* Make a public copy of the attach time constant phy attributes */
	bcopy(pi->pubpi, pi->pubpi_ro, sizeof(wlc_phy_t));

	/* register dump functions */
	phy_register_dumps(pi);

exit:
	/* Mark that they are not longer available so we can error/assert.  Use a pointer
	 * to self as a flag.
	 */
	pi->vars = (char *)&pi->vars;
	return pi;

err:
	phy_module_detach(pi);
	return NULL;
}

prephy_info_t *
BCMATTACHFN(prephy_module_attach)(shared_phy_t *sh, void *regs)
{
	prephy_info_t *pi;
	osl_t *osh = sh->osh;
	uint phyversion;
	int ret = 0;

	PHY_TRACE(("prephy_module_attach, regs %p, sh %p)\n", regs, sh));

	/* ONLY common PI is allocated. pi->u.pi_xphy is not available yet */
	if ((pi = (prephy_info_t *)MALLOC_NOPERSIST(osh, sizeof(prephy_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
		return NULL;
	}

	pi->sh = sh; /* Assign sh so that phy_malloc can be used from here on */
	pi->regs = (d11regs_t *)regs;
	ret = d11regs_select_offsets_tbl(&pi->regoffsets, sh->corerev);
	if (ret) {
		PHY_ERROR(("%s: prephy_module_attach failed, regs for rev not found\n",
			__FUNCTION__));
		goto err;
	}

	if ((pi->pubpi = phy_malloc(pi, sizeof(wlc_phy_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced pubpi %d bytes\n", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
	    goto err;
	}

	phyversion = R_REG(osh, D11_PHY_REG_0(pi));
	pi->pubpi->phy_type = PHY_TYPE(phyversion);
	pi->pubpi->phy_rev = phyversion & PV_PV_MASK;

	if ((pi->prephyi = phy_prephy_attach(pi)) == NULL) {
		PHY_ERROR(("%s: phy_prephy_attach failed\n", __FUNCTION__));
		goto err;
	}

	/* Mark that they are not longer available so we can error/assert.	Use a pointer
	 * to self as a flag.
	 */
	return pi;

err:
	prephy_module_detach(pi);
	return NULL;
}

shared_phy_t *
BCMATTACHFN(wlc_prephy_shared_attach)(shared_phy_params_t *shp)
{
	shared_phy_t *sh;

	/* allocate wlc_info_t state structure */
	if ((sh = (shared_phy_t*) MALLOC_NOPERSIST(shp->osh, sizeof(shared_phy_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			shp->unit, __FUNCTION__, MALLOCED(shp->osh)));
		return NULL;
	}

	sh->osh	= shp->osh;
	sh->sih	= shp->sih;
	sh->physhim	= shp->physhim;
	sh->unit	= shp->unit;
	sh->corerev	= shp->corerev;

	return sh;
}

static void
BCMATTACHFN(wlc_phy_srom_attach)(phy_info_t *pi, int bandtype)
{
	uint8 i = 0;
	uint8 j = 0;
	uint8 k = 0;

	pi->rssi_corr_normal = (int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_rssicorrnorm, 0);
	pi->rssi_corr_boardatten = (int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_rssicorratten, 7);

	/* Re-init the interference value based on the nvram variables */
	if (PHY_GETVAR_SLICE(pi, rstr_interference) != NULL) {
		pi->sh->interference_mode_2G = (int)PHY_GETINTVAR_SLICE(pi, rstr_interference);
		pi->sh->interference_mode_5G = (int)PHY_GETINTVAR_SLICE(pi, rstr_interference);

		if (BAND_2G(bandtype))
			pi->sh->interference_mode = pi->sh->interference_mode_2G;
		else
			pi->sh->interference_mode = pi->sh->interference_mode_5G;
	}

#if defined(RXDESENS_EN)
	/* phyrxdesens in db for SS SPC production */
	pi->sh->phyrxdesens = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_phyrxdesens, 0);
#endif // endif

	/* CCK Power index correction read from nvram */
	pi->sromi->cckPwrIdxCorr = (int16) PHY_GETINTVAR_DEFAULT(pi, rstr_cckPwrIdxCorr, 0);

	pi->min_txpower = PHY_TXPWR_MIN;
	pi->tx_pwr_backoff = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_txpwrbckof, PHY_TXPWRBCKOF_DEF);
	/* Some old boards doesn't have valid value programmed, use default */
	if (pi->tx_pwr_backoff == -1)
		pi->tx_pwr_backoff = PHY_TXPWRBCKOF_DEF;

	pi->phy_tempsense_offset = 0;

	pi->core2slicemap = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_core2slicemap, 0);
	if (pi->core2slicemap != ((1 << DUAL_PHY_NUM_CORE_MAX) -1)) {
	  for (i = 0; i < DUAL_PHY_NUM_CORE_MAX; i++) {
	    if ((pi->core2slicemap >> i) & 1) {
	      pi->aux_slice_ant[k] = i;
	      k++;
	    } else {
	      pi->main_slice_ant[j] = i;
	      j++;
	    }
	  }
	}
}

static void
BCMATTACHFN(wlc_phy_std_params_attach)(phy_info_t *pi)
{
	/* set default rx iq est antenna/samples */
	pi->phy_rxiq_samps = PHY_NOISE_SAMPLE_LOG_NUM_NPHY;
	pi->phy_rxiq_antsel = ANT_RX_DIV_DEF;

	/* initialize SROM "isempty" flags for rxgainerror */
	pi->rxgainerr2g_isempty = FALSE;
	pi->rxgainerr5gl_isempty = FALSE;
	pi->rxgainerr5gm_isempty = FALSE;
	pi->rxgainerr5gh_isempty = FALSE;
	pi->rxgainerr5gu_isempty = FALSE;

	/* Do not enable the PHY watchdog for ATE */
#ifndef ATE_BUILD
	pi->phywatchdog_override = TRUE;
#endif // endif

	/* Enable both cores by default */
	phy_stf_set_phyrxchain(pi->stfi, 0x3);

#if defined(WLTEST)
	/* Initialize to invalid index values */
	pi->nphy_tbldump_minidx = -1;
	pi->nphy_tbldump_maxidx = -1;
	pi->nphy_phyreg_skipcnt = 0;
#endif // endif

	/* This is the temperature at which the last PHYCAL was done.
	 * Initialize to a very low value.
	 */
	pi->def_cal_info->last_cal_temp = -50;
	pi->def_cal_info->cal_suppress_count = 0;

	/* default, PHY type overrides if interrupt based noise measurement isn't supported */
	pi->phynoise_polling = TRUE;

	/* Assign the default cal info */
	pi->cal_info = pi->def_cal_info;
	pi->cal_info->cal_suppress_count = 0;
}

void
BCMATTACHFN(phy_module_detach)(phy_info_t *pi)
{

	PHY_TRACE(("wl: %s: pi = %p\n", __FUNCTION__, pi));

	if (pi == NULL)
		return;

	ASSERT(pi->refcnt > 0);

	if (--pi->refcnt)
		return;

	/* ======== Detach modules' PHY specific layer ======== */

	/* Unregister PHY type implementations from common - MUST BE THE FIRST! */
	if (pi->typei != NULL)
		phy_type_unregister_impl(pi->typei);

	/* ======== Detach modules' common layer ======== */

	/* ...Detach other modules... */
	/* Detach STF module */
	if (pi->stfi != NULL)
		phy_stf_detach(pi->stfi);

#ifdef WLVASIP
	/* Detach vasip module */
	if (pi->vasipi != NULL) {
		phy_vasip_detach(pi->vasipi);
	}
#endif /* WLVASIP */

#ifdef WLSMC
	/* Detach smc module */
	if (pi->smci != NULL) {
		phy_smc_detach(pi->smci);
	}
#endif /* WLSMC */

#ifdef RADIO_HEALTH_CHECK
	/* Detach health check module */
	if (pi->hci != NULL)
		phy_hc_detach(pi->hci);
#endif /* RADIO_HEALTH_CHECK */
#if defined(WL_ETMODE) && !defined(WL_ETMODE_DISABLED)
	/* Detach envelope tracking module */
	if (pi->eti != NULL)
		phy_et_detach(pi->eti);
#endif /* WL_ETMODE && !WL_ETMODE_DISABLED */
	/* Detach HiRSSIeLNABypass module */
	if (pi->hirssii != NULL)
		phy_hirssi_detach(pi->hirssii);

#if defined(WL_NAP) && !defined(WL_NAP_DISABLED)
	/* Detach Nap module */
	if (pi->napi != NULL) {
		phy_nap_detach(pi->napi);
	}
#endif /* WL_NAP */

#ifdef WL_PROXDETECT
	/* Detach TOF module */
	if (pi->tofi != NULL)
		phy_tof_detach(pi->tofi);
#endif  /* WL_PROXDETECT */

	/* Detach dccal module */
	if (pi->dccali != NULL)
		phy_dccal_detach(pi->dccali);

#ifdef WL_MU_RX
	/* Detach MU-MIMO module */
	if (pi->mui != NULL)
		phy_mu_detach(pi->mui);
#endif /* WL_MU_RX */

#ifdef WL_DSI
	/* Detach DeepSleepInit module */
	if (pi->dsii != NULL)
		phy_dsi_detach(pi->dsii);
#endif /* WL_DSI */

#ifdef SAMPLE_COLLECT
	/* Detach SAMPle collect module */
	if (pi->sampi != NULL)
		phy_samp_detach(pi->sampi);
#endif /* SAMPLE_COLLECT */

#ifdef OCL
	/* Detach OCL module */
	if (pi->ocli != NULL)
		phy_ocl_detach(pi->ocli);
#endif /* OCL */

#ifdef WL11AX
	/* Detach HE module */
	if (pi->hecapi != NULL)
		phy_hecap_detach(pi->hecapi);
#endif /* WL11AX */

	/* Detach RXSPUR module */
	if (pi->rxspuri != NULL)
		phy_rxspur_detach(pi->rxspuri);

	/* Detach RXGCRS module */
	if (pi->rxgcrsi != NULL)
		phy_rxgcrs_detach(pi->rxgcrsi);

	/* Detach TSSI Cal module */
	if (pi->tssicali != NULL)
		phy_tssical_detach(pi->tssicali);

	/* Detach misc module */
	if (pi->misci != NULL)
		phy_misc_detach(pi->misci);

#ifdef WL_LPC
	/* Detach LPC module */
	if (pi->lpci != NULL)
		phy_lpc_detach(pi->lpci);
#endif /* WL_LPC */
#ifdef ENABLE_FCBS
	/* Detach FCBS module */
	if (pi->fcbsi != NULL)
		phy_fcbs_detach(pi->fcbsi);
#endif /* ENABLE_FCBS */
	/* Detach CHannelManaGeR module */
	if (pi->chanmgri != NULL)
		phy_chanmgr_detach(pi->chanmgri);

	/* Detach VCO Cal module */
	if (pi->vcocali != NULL)
		phy_vcocal_detach(pi->vcocali);

	/* Detach PAPD Cal module */
	if (pi->papdcali != NULL)
		phy_papdcal_detach(pi->papdcali);

	/* Detach RXIQ Cal module */
	if (pi->rxiqcali != NULL)
		phy_rxiqcal_detach(pi->rxiqcali);

	/* Detach TXIQLO Cal module */
	if (pi->txiqlocali != NULL)
		phy_txiqlocal_detach(pi->txiqlocali);

	/* Detach BlueToothCoExistence module */
	if (pi->btcxi != NULL)
		phy_btcx_detach(pi->btcxi);

	/* Detach RSSICompute module */
	if (pi->rssii != NULL)
		phy_rssi_detach(pi->rssii);

	/* Detach TEMPerature sense module */
	if (pi->tempi != NULL)
		phy_temp_detach(pi->tempi);

#ifndef WLC_DISABLE_ACI
	/* Detach INTerFerence module */
	if (pi->noisei != NULL)
		phy_noise_detach(pi->noisei);
#endif // endif

	/* Detach ANTennaDIVersity module */
	if (pi->antdivi != NULL)
		phy_antdiv_detach(pi->antdivi);

#if defined(AP) && defined(RADAR)
	/* Detach RadarDetect module */
	if (pi->radari != NULL)
		phy_radar_detach(pi->radari);
#endif // endif

#if defined(WLC_TXPWRCAP) && !defined(WLC_TXPWRCAP_DISABLED)
	/* Detach TxPowerCtrl module */
	if (pi->txpwrcapi != NULL)
		phy_txpwrcap_detach(pi->txpwrcapi);
#endif // endif

	/* Detach TxPowerCtrl module */
	if (pi->tpci != NULL)
		phy_tpc_detach(pi->tpci);

	/* Detach PHYTableInit module */
	if (pi->tbli != NULL)
		phy_tbl_detach(pi->tbli);

	/* Detach RADIO control module */
	if (pi->radioi != NULL)
		phy_radio_detach(pi->radioi);

	/* Detach ANAcore control module */
	if (pi->anai != NULL)
		phy_ana_detach(pi->anai);

	/* ======== Detach PHY specific layer ======== */

	/* Detach PHY type implementation layer from common layer */
	if (pi->typei != NULL &&
	    *(phy_type_info_t **)(uintptr)&pi->u != NULL)
		phy_type_detach(pi->typei, *(phy_type_info_t **)(uintptr)&pi->u);

	/* ======== Detach infrastructure services ======== */

	/* Detach CALibrationManaGeR module */
	if (pi->calmgri != NULL)
		phy_calmgr_detach(pi->calmgri);

	/* Detach watchdog module */
	if (pi->wdi != NULL)
		phy_wd_detach(pi->wdi);

	/* Detach CACHE module */
	if (pi->cachei != NULL)
		phy_cache_detach(pi->cachei);
#ifdef PHYCAL_CACHING
	/* Detach CHannelManaGeR Notification module */
	if (pi->chanmgr_notifi != NULL)
		phy_chanmgr_notif_detach(pi->chanmgr_notifi);
#endif // endif
	/* Detach INIT control module */
	if (pi->initi != NULL)
		phy_init_detach(pi->initi);

	/* Detach PHY type implementation dispatch info */
	if (pi->typei != NULL)
		phy_type_disp_detach(pi->typei);

	/* Detach PHY Common info */
	if (pi->cmni != NULL)
		phy_cmn_detach(pi->cmni);
#ifdef PHY_DBG_ENABLED
	/* Detach dump registry - MUST BE THE LAST */
	if (pi->dbgi != NULL)
		phy_dbg_detach(pi->dbgi);
#endif /* PHY_DBG_ENABLED */

/* *********************************************** */

#if defined(PHYCAL_CACHING)
	pi->phy_calcache_on = FALSE;
#endif // endif

	/* Quick-n-dirty remove from list */
	if (pi->sh->phy_head == pi)
		pi->sh->phy_head = pi->next;
	else if (pi->sh->phy_head->next == pi)
		pi->sh->phy_head->next = NULL;
	else
		ASSERT(0);

	if (pi->tx_power_offset != NULL) {
		/* Restore the correct bandwidth */
		ppr_set_ch_bw(pi->tx_power_offset, ppr_get_max_bw());
		ppr_delete(pi->sh->osh, pi->tx_power_offset);
	}

#ifdef WLTXPWR_CACHE
	if (pi->txpwr_cache != NULL)
		wlc_phy_txpwr_cache_close(pi->sh->osh, pi->txpwr_cache);
#endif	/* WLTXPWR_CACHE */

	if (pi->ff != NULL) {
		if (pi->ff->_feature_enab != NULL) {
			phy_mfree(pi, pi->ff->_feature_enab, CEIL(PHY_FEATURE_MAX_IDX, 8));
		}
		phy_mfree(pi, pi->ff, sizeof(phy_feature_flags_t));
		pi->ff = NULL;
	}

	if (pi->pdpi != NULL) {
		phy_mfree(pi, pi->pdpi, sizeof(*pi->pdpi));
		pi->pdpi = NULL;
	}

	if (pi->pwrdet_ac != NULL) {
	    phy_mfree(pi, pi->pwrdet_ac, (SROMREV(pi->sh->sromrev) >= 12 ?
	       sizeof(srom12_pwrdet_t) : sizeof(srom11_pwrdet_t)));
	    pi->pwrdet_ac = NULL;
	}

	if (pi->ppr != NULL) {
		phy_mfree(pi, pi->ppr, sizeof(ppr_info_t));
		pi->ppr = NULL;
	}

	if (pi->pi_fptr != NULL) {
		phy_mfree(pi, pi->pi_fptr, sizeof(phy_func_ptr_t));
		pi->pi_fptr = NULL;
	}
#ifdef WLSRVSDB
	if (pi->srvsdb_state != NULL) {
		phy_mfree(pi, pi->srvsdb_state, sizeof(srvsdb_info_t));
		pi->srvsdb_state = NULL;
	}
#endif /* WLSRVSDB */
	if (pi->fem5g != NULL) {
		phy_mfree(pi, pi->fem5g, sizeof(srom_fem_t));
		pi->fem5g = NULL;
	}
	if (pi->fem2g != NULL) {
		phy_mfree(pi, pi->fem2g, sizeof(srom_fem_t));
		pi->fem2g = NULL;
	}
#ifdef ENABLE_FCBS
	if (pi->phy_fcbs != NULL) {
	    phy_mfree(pi, pi->phy_fcbs, sizeof(fcbs_info));
	    pi->phy_fcbs = NULL;
	}
#endif /* ENABLE_FCBS */
	if (pi->def_cal_info != NULL) {
	    phy_mfree(pi, pi->def_cal_info, sizeof(phy_cal_info_t));
	    pi->def_cal_info = NULL;
	}
	if (pi->pwrdet != NULL) {
	    phy_mfree(pi, pi->pwrdet, sizeof(srom_pwrdet_t));
	    pi->pwrdet = NULL;
	}
	if (pi->interf != NULL) {
	    phy_mfree(pi, pi->interf, sizeof(interference_info_t));
	    pi->interf = NULL;
	}

	if (pi->pubpi_ro != NULL) {
	    phy_mfree(pi, pi->pubpi_ro, sizeof(wlc_phy_t));
	    pi->pubpi_ro = NULL;
	}

	if (pi->pubpi != NULL) {
	    phy_mfree(pi, pi->pubpi, sizeof(wlc_phy_t));
	    pi->pubpi = NULL;
	}

	MFREE(pi->sh->osh, pi, sizeof(phy_info_t));
}

void
BCMATTACHFN(prephy_module_detach)(prephy_info_t *pi)
{

	PHY_TRACE(("wl: %s: pi = %p\n", __FUNCTION__, pi));

	if (pi == NULL)
		return;

	/* ======== Detach modules' PHY specific layer ======== */

	if (pi->prephyi != NULL) {
		phy_prephy_detach(pi->prephyi);
	}

	if (pi->pubpi != NULL) {
	    phy_mfree(pi, pi->pubpi, sizeof(wlc_phy_t));
	    pi->pubpi = NULL;
	}

	MFREE(pi->sh->osh, pi, sizeof(prephy_info_t));
}

void
BCMATTACHFN(wlc_prephy_shared_detach)(shared_phy_t *sh)
{
	if (sh != NULL) {
		MFREE(sh->osh, sh, sizeof(shared_phy_t));
	}
}

/* Register all iovar tables to/from system */
int
BCMATTACHFN(phy_register_iovt_all)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	int err;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Register all common layer's iovar tables/handlers */
	if ((err = phy_register_iovt(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PHY type implementation layer's iovar tables/handlers */
	if ((err = phy_type_register_iovt(pi->typei, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_type_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

/* Register all ioctl tables to/from system */
int
BCMATTACHFN(phy_register_ioct_all)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	int err;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Register all common layer's ioctl tables/handlers */
	if ((err = phy_register_ioct(pi, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_register_ioct failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register PHY type implementation layer's ioctl tables/handlers */
	if ((err = phy_type_register_ioct(pi->typei, ii)) != BCME_OK) {
		PHY_ERROR(("%s: phy_type_register_ioct failed\n", __FUNCTION__));
		goto fail;
	}

	return BCME_OK;

fail:
	return err;
}

/* Init/deinit the PHY h/w. */
int
WLBANDINITFN(phy_init)(phy_info_t *pi, chanspec_t chanspec)
{
	uint32	mc;
#if defined(BCMDBG) || defined(PHYDBG)
	char chbuf[CHANSPEC_STR_LEN];
#endif // endif
	int err = BCME_OK;
	ASSERT(pi != NULL);

	pi->phy_crash_rc = PHY_RC_NONE;

#if defined(BCMDBG) || defined(PHYDBG)
	PHY_TRACE(("wl%d: %s chanspec %s\n", pi->sh->unit, __FUNCTION__,
		wf_chspec_ntoa(chanspec, chbuf)));
#endif // endif

	/* skip if this function is called recursively(e.g. when bw is changed) */
	if (pi->init_in_progress) {
		err = BCME_NOTREADY;
		goto end;
	}

	pi->last_radio_chanspec = pi->radio_chanspec;

	pi->init_in_progress = TRUE;
	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);
	pi->phynoise_state = 0;

	/* Update ucode channel value */
	phy_chanmgr_set_shm(pi, chanspec);

	mc = R_REG(pi->sh->osh, D11_MACCONTROL(pi));
	if ((mc & MCTL_EN_MAC) != 0) {
		if (mc == 0xffffffff)
			PHY_ERROR(("wl%d: %s: chip is dead !!!\n", pi->sh->unit, __FUNCTION__));
		else
			PHY_ERROR(("wl%d: %s: MAC running! mc=0x%x\n",
			          pi->sh->unit, __FUNCTION__, mc));
		ASSERT((const char*)"wlc_phy_init: Called with the MAC running!" == NULL);
	}

	/* clear during init. To be set by higher level wlc code */
	pi->cur_interference_mode = INTERFERE_NONE;

	/* init PUB_NOT_ASSOC */
	if (!(pi->measure_hold & PHY_HOLD_FOR_SCAN) && pi->interf->aci.nphy != NULL &&
	    !(pi->interf->aci.nphy->detection_in_progress)) {
#ifdef WLSRVSDB
		if (!pi->srvsdb_state->srvsdb_active)
			pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
#else
		pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
#endif // endif
	}

	/* check D11 is running on Fast Clock */
	ASSERT(si_core_sflags(pi->sh->sih, 0, 0) & SISF_FCLKA);

#ifdef WL_DSI
	if (phy_get_dsi_trigger_st(pi)) {
		/* DeepSleepInit */
		phy_dsi_restore(pi);
	} else
#endif /* WL_DSI */
	{
		/* ######## Init process start ######## */

		/* ======== Common inits ======== */

		/* Init each feature/module including s/w and h/w */
		if ((err = phy_init_invoke_init_fns(pi->initi)) != BCME_OK) {
			PHY_ERROR(("wl%d: %s: phy_init_invoke_init_fns failed."
				"phy_type %d, rev %d\n", pi->sh->unit, __FUNCTION__,
				pi->pubpi->phy_type, pi->pubpi->phy_rev));
			goto end;
		}

		/* ======== Special inits ======== */

		/* ^^^Add other special init calls here^^^ */

		/* ######## Init process end ######## */
	}

	/* Indicate a power on reset isn't needed for future phy init's */
	pi->phy_init_por = FALSE;

	pi->init_in_progress = FALSE;

	/* clear flag */
	phy_init_done(pi);

	pi->bt_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_BTCX_BLK_PTR(pi));

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
	/* Ensure that WLAN/BT coex FEMctrl shmems are populated upon
	 * the first WLAN init
	 */
	wlc_phy_femctrl_mask_on_band_change(pi);
#endif // endif
	if (CHIPID(pi->sh->chip) == BCM43012_CHIP_ID) {
		int16 curtemp = phy_temp_sense_read((wlc_phy_t *)pi);
		wlapi_openloop_cal(pi->sh->physhim, (uint16) curtemp);
	}
end:
	return err;
}

/* driver up/init processing */
static int
WLBANDINITFN(_phy_init)(phy_init_ctx_t *ctx)
{
	phy_info_t *pi = (phy_info_t *)ctx;

	return phy_type_init_impl(pi->typei);
}

int
BCMUNINITFN(phy_down)(phy_info_t *pi)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* all activate phytest should have been stopped */
	ASSERT(pi->phytest_on == FALSE);

	/* ^^^Add other special down calls here^^^ */

	/* ======== Common down ======== */

	phy_init_invoke_down_fns(pi->initi);

	return callbacks;
}

void
BCMATTACHFN(phy_machwcap_set)(wlc_phy_t *ppi, uint32 machwcap)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->sh->machwcap = machwcap;
}

int
phy_bss_init(wlc_phy_t *pih, bool bonlyap, int noise)
{
	phy_info_t *pi = (phy_info_t*)pih;

	if (bonlyap) {
	}

	return phy_noise_bss_init(pi->noisei, noise);
}

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
static int
_phy_dump_phyregs(void *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ctx;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	return phy_type_dump_phyregs(pi->typei, b);

}

#define PAGE_SZ 512
#define NUM_PAGES 10

typedef struct {
	phy_regs_t *rl_start;
	phy_regs_t *rl_end;
} dp_info_t;

dp_info_t dpi[NUM_PAGES];

static void
phyreg_page_parser(phy_info_t *pi, phy_regs_t *reglist, struct bcmstrbuf *b)
{
	int i = 0, nr = 0;
	bool rl_end_set = FALSE, dbg_prints = FALSE;

	phy_regs_t *rl_end = NULL, *rl_last = NULL;

	/* initialize base */
	dpi[0].rl_start = reglist;

	for (i = 0; i < ARRAYSIZE(dpi); i++) {
		nr = 0; rl_end_set = FALSE;
		reglist = dpi[i].rl_start;

		while (reglist->num > 0) {
			if (nr > PAGE_SZ) {
				rl_end = reglist;
				rl_end_set = TRUE;
				break;
			}
			nr += reglist->num;
			rl_last = ++reglist;
		}

		if (!rl_end_set)
			rl_end = rl_last;

		dpi[i].rl_end = rl_end;

		if (i < (ARRAYSIZE(dpi) - 1))
			dpi[i + 1].rl_start = dpi[i].rl_end;

		if (dbg_prints) {
			bcm_bprintf(b, "%d: sb %04x eb %04x\n", i,
					dpi[i].rl_start->base,
					dpi[i].rl_end->base);
		}
	}

	for (i = 0; i < ARRAYSIZE(dpi); i++) {
		if (dpi[i].rl_start == dpi[i].rl_end) {
			pi->pdpi->phyregs_pmax = --i;
			break;
		}
	}

	if (dbg_prints)
		bcm_bprintf(b, "Num of Pages = %d\n", pi->pdpi->phyregs_pmax);
}

/* dump phyregs listed in 'reglist' */
void
phy_dump_phyregs(phy_info_t *pi, const char *str,
	phy_regs_t *rl, uint16 off, struct bcmstrbuf *b)
{
	uint16 addr, val = 0, num;
	phy_regs_t *reglist = rl, *rl_end = NULL;

#if defined(WLTEST)
	uint16 i = 0;
	bool skip;
#endif // endif
	if (reglist == NULL)
		return;

	/* setup dump pages */
	phyreg_page_parser(pi, reglist, b);

	if (pi->pdpi->page > pi->pdpi->phyregs_pmax) {
		bcm_bprintf(b, "\n(%d > %d) | Exceeding # of available pages \n\n",
				pi->pdpi->page, pi->pdpi->phyregs_pmax);
		return;
	}

	reglist = dpi[pi->pdpi->page].rl_start;
	rl_end = dpi[pi->pdpi->page].rl_end;

	if (reglist == rl_end) {
		bcm_bprintf(b, "\nMax # of pages exceeded\n\n", str);
		return;
	}

	bcm_bprintf(b, "----- %06s -----\n", str);
	bcm_bprintf(b, "Add Value\n");

	while (((num = reglist->num) > 0) && (reglist != rl_end)) {
#if defined(WLTEST)
		skip = FALSE;

		for (i = 0; i < pi->nphy_phyreg_skipcnt; i++) {
			if (pi->nphy_phyreg_skipaddr[i] == reglist->base) {
				skip = TRUE;
				break;
			}
		}

		if (skip) {
			reglist++;
			continue;
		}
#endif // endif

		for (addr = reglist->base + off; num && b->size > 0; addr++, num--) {
			val = phy_type_read_phyreg(pi->typei, addr);

			if (PHY_INFORM_ON() && si_taclear(pi->sh->sih, FALSE)) {
				PHY_INFORM(("%s: TA reading phy reg %s:0x%x\n",
				           __FUNCTION__, str, addr));
				bcm_bprintf(b, "%03x tabort\n", addr);
			} else
				bcm_bprintf(b, "%03x %04x\n", addr, val);
		}
		reglist++;
	}
}
#endif // endif

static void
BCMATTACHFN(phy_register_dumps)(phy_info_t *pi)
{
#if defined(BCMDBG_PHYDUMP)
	phy_dbg_add_dump_fn(pi, "phyreg", _phy_dump_phyregs, pi);
#endif // endif
	/*  default page */
	pi->pdpi->page = 0;
}

void *
phy_malloc_fatal(phy_info_t *pi, uint sz)
{
	void* ptr;
	if ((ptr = phy_malloc(pi, sz)) == NULL) {
		PHY_FATAL_ERROR_MESG(("Memory allocation failed in function %p,"
			"malloced %d bytes\n", CALL_SITE, MALLOCED(pi->sh->osh)));
		PHY_FATAL_ERROR(pi, PHY_RC_NOMEM);
	}
	return ptr;
}

/* PHYMODE switch requires a clean phy initialization,
 * set a flag to indicate phyinit is pending
 */
bool
phy_init_pending(phy_info_t *pi)
{
	ASSERT(pi != NULL);
	return pi->phyinit_pending;
}

/* clear flag upon phyinit */
static void
phy_init_done(phy_info_t *pi)
{
	ASSERT(pi != NULL);
	pi->phyinit_pending = FALSE;
}

mbool
phy_get_measure_hold_status(phy_info_t *pi)
{
	return pi->measure_hold;
}

void
phy_set_measure_hold_status(phy_info_t *pi, mbool set)
{
	pi->measure_hold = set;
}

int8
phy_preamble_override_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->n_preamble_override;
}

int
phy_hw_clk_state_upd(wlc_phy_t *pih, bool newstate)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!pi || !pi->sh) {
		return BCME_ERROR;
	}

	pi->sh->clk = newstate;
	return BCME_OK;
}

int
phy_hw_state_upd(wlc_phy_t *pih, bool newstate)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!pi || !pi->sh) {
		return BCME_ERROR;
	}

	pi->sh->up = newstate;
	return BCME_OK;
}

bool
phy_feature_enabled(phy_info_t *pi, uint idx)
{
	return isset(pi->ff->_feature_enab, idx);
}

void
phy_set_feature_flag(phy_info_t *pi, uint idx, bool enable)
{
	if (enable) {
		setbit(pi->ff->_feature_enab, idx);
	} else {
		clrbit(pi->ff->_feature_enab, idx);
	}
}
