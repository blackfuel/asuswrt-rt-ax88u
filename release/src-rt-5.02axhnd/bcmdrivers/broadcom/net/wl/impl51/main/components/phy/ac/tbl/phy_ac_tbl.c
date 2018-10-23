/*
 * ACPHY PHYTableInit module implementation
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
 * $Id: phy_ac_tbl.c 752608 2018-03-16 19:02:24Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_tbl.h"
#include <phy_ac.h>
#include <phy_ac_tbl.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#include <phy_ac_tof.h>
#include <wlc_phyreg_ac.h>
#include <bcmotp.h>
#include <hndpmu.h>
#include <wlc_phytbl_ac.h>
#include <phy_ac_chanmgr.h>
#include <phy_ac_txiqlocal.h>
#include <phy_rxgcrs_api.h>
#include <phy_stf.h>

#include <d11.h>
#include "wlc_phy_radio.h"
#include "wlc_radioreg_20691.h"
#include "wlc_radioreg_20693.h"
#include <wlc_phytbl_ac_gains.h>

#include <phy_rstr.h>
#include <bcmdevs.h>
#include <phy_utils_var.h>
#include <phy_utils_pmu.h>
#ifdef WLC_TXPWRCAP
#include <phy_ac_txpwrcap.h>
#endif // endif

#ifdef WLC_TXCAL
#include <phy_tssical.h>
#endif /* WLC_TXCAL */

/* module private states */
struct phy_ac_tbl_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_tbl_info_t *ti;
	phy_ac_tbl_data_t *data; /* shared data */
	uint16	*gaintbl_2g;
	uint16	*gaintbl_5g;
	uint16	rfldo;
	uint16	phy_hw_minor_rev;
	uint8	cbuck_out;
};

typedef struct {
	phy_ac_tbl_info_t info;
	phy_ac_tbl_data_t data;
} phy_ac_tbl_mem_t;

/* ************************************************************************************ */
/* ************************************************************************************ */

#ifdef BCMPHYACMINORREV_ROUTER
#define HW_AC_MINOR_REV_VALUE(tbli) BCMPHYACMINORREV_ROUTER
#else   /* BCMPHYACMINORREV_ROUTER */
/* Read MinorVersion HW register from chip */
#define HW_AC_MINOR_REV_VALUE(tbli) ((tbli)->phy_hw_minor_rev)
#endif  /* BCMPHYACMINORREV_ROUTER */

#define HW_AC_MINOR_REV_IS(tbli, value) (HW_AC_MINOR_REV_VALUE(tbli) == (value))
/* ************************************************************************************ */

/* local functions */
static bool wlc_phy_attach_chan_tuning_tbl(phy_ac_tbl_info_t *tbli, phy_info_t *pi);
static int phy_ac_tbl_init(phy_type_tbl_ctx_t *ctx);
#ifdef WL11ULB
static bool wlc_phy_ac_ulb_10_capable(phy_info_t *pi);
static bool wlc_phy_ac_ulb_5_capable(phy_info_t *pi);
static bool wlc_phy_ac_ulb_2P5_capable(phy_info_t *pi);
#endif /* WL11ULB */
static void phy_ac_set_decode_timeouts(phy_info_t *pi);
#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
static void phy_ac_tbl_read_table(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval);
static int phy_ac_tbl_dumptbl(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ac_tbl_read_table NULL
#define phy_ac_tbl_dumptbl NULL
#endif // endif
static void wlc_phy_rfldo_trim_value(phy_ac_tbl_info_t *tbli, phy_info_t *pi);
static void wlc_phy_force_dac_clk_from_bbpll(phy_info_t *pi, bool set, uint16 *orig_dac_clk_pu);

/* local functions */
static void phy_ac_tbl_nvram_attach(phy_ac_tbl_info_t *tbli, phy_info_t *pi);
static void wlc_phy_init_acphy(phy_ac_tbl_info_t *tbli);
static bool phy_ac_tbl_create_gain_tbls(phy_ac_tbl_info_t *tbli);
static void phy_ac_tbl_delete_gain_tbls(phy_ac_tbl_info_t *tbli);

#if defined(BCMDBG)
static int phy_ac_tbl_dump_txv0(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
static int phy_ac_tbl_dump_axmacphyif(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
#endif // endif

#define PMU_REG6_RFLDO_CTRL              0x000000E0
#define PMU_REG6_RFLDO_CTRL_SHFT         5

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_tbl_info_t *
BCMATTACHFN(phy_ac_tbl_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_tbl_info_t *ti)
{
	phy_ac_tbl_info_t *info;
	phy_type_tbl_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_tbl_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->ti = ti;
	info->data = &(((phy_ac_tbl_mem_t *) info)->data);

	/* set the gain tables for tiny radio */
	if (TINY_RADIO(pi) || IS_4364_3x3(pi)) {
		if (!phy_ac_tbl_create_gain_tbls(info)) {
			goto fail;
		}
	}

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_ac_tbl_init;
	fns.readtbl = phy_ac_tbl_read_table;
	fns.dump = phy_ac_tbl_dumptbl;
	fns.ctx = info;
#if defined(BCMDBG)
	fns.dump_txv0 = phy_ac_tbl_dump_txv0;
	fns.dump_axmacphyif = phy_ac_tbl_dump_axmacphyif;
#endif // endif

	/* Read srom params from nvram */
	phy_ac_tbl_nvram_attach(info, pi);

	/* malloc chan_tuning here */
	if (!wlc_phy_attach_chan_tuning_tbl(info, pi))
		goto fail;

	/* PHY-Feature specific parameter initialization */
	/* Read RFLDO from OTP */
	/* ac_info->rfldo = wlc_phy_rfldo_trim_value(pi); */
	/* Read RFLDO from OTP */
	wlc_phy_rfldo_trim_value(info, pi);

	phy_tbl_register_impl(ti, &fns);

	return info;
fail:
	phy_ac_tbl_unregister_impl(info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_tbl_unregister_impl)(phy_ac_tbl_info_t *info)
{
	phy_info_t *pi;
	phy_tbl_info_t *ti;

	if (info == NULL) {
		PHY_INFORM(("%s: Null info pointer\n", __FUNCTION__));
		return;
	}
	pi = info->pi;
	ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tbl_unregister_impl(ti);

	if (RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) {
		if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 1) {
			if (PHY_XTAL_IS52M(pi)) {
				if (info->data->chan_tuning != NULL) {
					phy_mfree(pi, info->data->chan_tuning,
						NUM_ROWS_CHAN_TUNING *
						sizeof(chan_info_radio2069revGE25_52MHz_t));
				}
			}
		} else if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) {
			if (info->data->chan_tuning != NULL) {
				phy_mfree(pi, info->data->chan_tuning,
					NUM_ROWS_CHAN_TUNING *
					sizeof(chan_info_radio2069revGE32_t));
			}
		}
	}

	phy_ac_tbl_delete_gain_tbls(info);

	phy_mfree(pi, info, sizeof(phy_ac_tbl_mem_t));
}

/* Inter-module data api between ac modules */
phy_ac_tbl_data_t *
phy_ac_tbl_get_data(phy_ac_tbl_info_t *tbli)
{
	return tbli->data;
}

/* Apply min or max gain cap to tx gain table by copying/repeating last
 * entry over specified range
 */
static void
phy_ac_tbl_blanking(phy_ac_tbl_info_t *tbli, uint16 *tx_pwrctrl_tbl,
	uint8 txidxcap, bool is_max_cap)
{
	uint16 k, m, K, start, end;
	/* ACPHY has 48bit gaincode = 3 16-bit word */
	uint16 nword_per_gaincode = 3;

	K = txidxcap & 0xFF;

	if (is_max_cap) {
		start = 0;
		end = K - 1;
	} else {
		start = K + 1;
		end = 127;
	}

	for (k = start; k <= end; k++) {
		for (m = 0; m < nword_per_gaincode; m++) {
			tx_pwrctrl_tbl[k*nword_per_gaincode + m] =
			        tx_pwrctrl_tbl[K*nword_per_gaincode + m];
		}
	}
}

static bool
BCMATTACHFN(phy_ac_tbl_create_gain_tbls)(phy_ac_tbl_info_t *tbli)
{
	uint16 *tx_pwrctrl_tbl = NULL;
	phy_info_t *pi = tbli->pi;

	ASSERT(tbli->gaintbl_2g == NULL);

	if ((tbli->gaintbl_2g = phy_malloc(pi,
		sizeof(uint16) * TXGAIN_TABLES_LEN)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

	if (RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) {
		if (PHY_IPA(pi)) {
			tx_pwrctrl_tbl = (pi->extpagain2g != 0)
				? txgain_20691_phyrev13_2g_ipa
				: txgain_20691_phyrev13_ipa_2g_for_epa;
		} else {
			if (IS_4364_1x1(pi)) {
			/* Using 0.5dB gain table for 4364 1x1 */
				tx_pwrctrl_tbl = txgain_20691_phyrev13_2g_point5_epa;
			} else {
#ifdef WLC_POINT5_DB_TX_GAIN_STEP /* 0.5 dB gain step */
				tx_pwrctrl_tbl = txgain_20691_phyrev13_2g_point5_epa;
#else
				tx_pwrctrl_tbl = txgain_20691_phyrev13_2g_epa;
#endif /* WLC_POINT5_DB_TX_GAIN_STEP */
			}
		}
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		tx_pwrctrl_tbl = (PHY_IPA_ATTACH_2G_PARAMS(pi)) ? txgain_20693_phyrev12_2g_ipa
			: (RADIOREV(pi->pubpi->radiorev) == 32 ||
			RADIOREV(pi->pubpi->radiorev) == 33) ? txgain_20693_phyrev32_2g_epa
			: txgain_20693_phyrev12_2g_epa;
	} else if (IS_4364_3x3(pi)) {
		tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev64;
	} else {
		ASSERT(0);
		return FALSE;
	}
	if (IS_4364(pi)) {
		if (BF3_2GTXGAINTBL_BLANK(tbli->aci)) {
			phy_ac_tbl_blanking(tbli, tx_pwrctrl_tbl, pi->sromi->txidxcap2g, TRUE);
		}
		if (pi->sromi->txidxmincap2g > 0) {
			phy_ac_tbl_blanking(tbli, tx_pwrctrl_tbl, pi->sromi->txidxmincap2g,
				FALSE);
		}
	}
	ASSERT(tx_pwrctrl_tbl != NULL);

	/* Copy the table to the new location. */
	bcopy(tx_pwrctrl_tbl, tbli->gaintbl_2g, sizeof(uint16) * TXGAIN_TABLES_LEN);

#ifdef BAND5G
	ASSERT(tbli->gaintbl_5g == NULL);
	if ((tbli->gaintbl_5g = phy_malloc(pi,
		sizeof(uint16) * TXGAIN_TABLES_LEN)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

	if (RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) {
		if (pi->ipa5g_on) {
			tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_ipa;
		} else {
			if (IS_4364_1x1(pi)) {
			/* Using 0.5dB gain table for 4364 1x1 */
				if (pi->txgaintbl5g == 1) {
					tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_1_point5_epa;
				} else {
					tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_point5_epa;
				}
			} else {
#ifdef WLC_POINT5_DB_TX_GAIN_STEP /* 0.5 dB gain step */
				if (pi->txgaintbl5g == 1) {
					tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_1_point5_epa;
				} else {
					tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_point5_epa;
				}
#else
				if (pi->txgaintbl5g == 1) {
					tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_1_epa;
				} else {
					tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_epa;
				}
#endif /* WLC_POINT5_DB_TX_GAIN_STEP */
			}
		}
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		tx_pwrctrl_tbl = (PHY_IPA_ATTACH_5G_PARAMS(pi))?
			(ROUTER_4349(pi))? txgain_20693_phyrev15_5g_ipa_53573:
			txgain_20693_phyrev12_5g_ipa
			: (ROUTER_4349(pi))? txgain_20693_phyrev15_5g_epa_53573:
			(RADIOREV(pi->pubpi->radiorev) == 32 ||
			RADIOREV(pi->pubpi->radiorev) == 33) ?
			txgain_20693_phyrev32_5g_epa: txgain_20693_phyrev12_5g_epa;
	} else if (IS_4364_3x3(pi)) {
		tx_pwrctrl_tbl = (USE_OOB_GAINT(pi)) ?
			acphy_txgain_epa_5g_2069rev64_A0_SPUR_WAR:
			acphy_txgain_epa_5g_2069rev64;
	} else {
		ASSERT(0);
		return FALSE;
	}
	if (IS_4364(pi)) {
		if (BF3_5GTXGAINTBL_BLANK(tbli->aci)) {
			phy_ac_tbl_blanking(tbli, tx_pwrctrl_tbl, pi->sromi->txidxcap5g, TRUE);
		}
		if (pi->sromi->txidxmincap5g > 0) {
			phy_ac_tbl_blanking(tbli, tx_pwrctrl_tbl, pi->sromi->txidxmincap5g,
				FALSE);
		}
	}
	ASSERT(tx_pwrctrl_tbl != NULL);

	/* Copy the table to the new location. */
	bcopy(tx_pwrctrl_tbl, tbli->gaintbl_5g, sizeof(uint16) * TXGAIN_TABLES_LEN);
#else
	/* Set the pointer to null if 5G is not defined. */
	tbli->gaintbl_5g = NULL;
#endif /* BAND5G */
	return TRUE;
}

static void
BCMATTACHFN(phy_ac_tbl_delete_gain_tbls)(phy_ac_tbl_info_t *tbli)
{
	if (tbli->gaintbl_2g) {
		phy_mfree(tbli->pi, (void *)tbli->gaintbl_2g,
			sizeof(uint16)*TXGAIN_TABLES_LEN);
		tbli->gaintbl_2g = NULL;
	}

#ifdef BAND5G
	if (tbli->gaintbl_5g) {
		phy_mfree(tbli->pi, (void *)tbli->gaintbl_5g,
			sizeof(uint16)*TXGAIN_TABLES_LEN);
		tbli->gaintbl_5g = NULL;
	}
#endif // endif
}

/* h/w init/down */
static int
WLBANDINITFN(phy_ac_tbl_init)(phy_type_tbl_ctx_t *ctx)
{
	phy_ac_tbl_info_t *ti = (phy_ac_tbl_info_t *)ctx;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_init_acphy(ti);

	return BCME_OK;
}

static bool
BCMATTACHFN(wlc_phy_attach_chan_tuning_tbl)(phy_ac_tbl_info_t *tbli, phy_info_t *pi)
{
	const chan_info_radio2069revGE32_t *chan_info_tbl_GE32 = NULL;
	const chan_info_radio2069revGE25_52MHz_t *chan_info_tbl_GE25_52MHz = NULL;

	uint32 tbl_len = 0;
	tbli->data->chan_tuning = NULL;
	tbli->data->chan_tuning_tbl_len = 0;

	if (RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) {
		if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 1) {
			if (PHY_XTAL_IS52M(pi)) {
				if ((tbli->data->chan_tuning =
				     phy_malloc(pi,
				            NUM_ROWS_CHAN_TUNING *
				            sizeof(chan_info_radio2069revGE25_52MHz_t))) == NULL) {
					PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes",
						pi->sh->unit, __FUNCTION__, MALLOCED(pi->sh->osh)));
					return FALSE;
				}
				switch (RADIO2069REV(pi->pubpi->radiorev)) {
				case 25:
				case 26:
					chan_info_tbl_GE25_52MHz =
					     chan_tuning_2069rev_GE_25_52MHz;
					tbl_len =
					ARRAYSIZE(chan_tuning_2069rev_GE_25_52MHz);
					break;
				default:

					PHY_ERROR(("wl%d: %s: Unsupported radio revision %d\n",
						pi->sh->unit,
						__FUNCTION__, RADIO2069REV(pi->pubpi->radiorev)));
					ASSERT(0);
					return FALSE;
				}
				tbli->data->chan_tuning_tbl_len = tbl_len;
				memcpy(tbli->data->chan_tuning, chan_info_tbl_GE25_52MHz,
					NUM_ROWS_CHAN_TUNING *
					sizeof(chan_info_radio2069revGE25_52MHz_t));
			}

		} else if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) {
			/* malloc chan tuning */
			if ((tbli->data->chan_tuning =
			     phy_malloc(pi,
			            NUM_ROWS_CHAN_TUNING *
			            sizeof(chan_info_radio2069revGE32_t))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit, __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			switch (RADIO2069REV(pi->pubpi->radiorev)) {
			case 32:
			case 33:
			case 34:
			case 35:
			case 37:
			case 38:
				/* can have more conditions based on different radio revs */
				/*  RADIOREV(pi->pubpi->radiorev) =32/33/34 */
				/* currently tuning tbls for these are all same */
				if (PHY_XTAL_IS40M(pi)) {
					chan_info_tbl_GE32 = chan_tuning_2069_rev33_37_40;
					tbl_len = ARRAYSIZE(chan_tuning_2069_rev33_37_40);
				} else {
					chan_info_tbl_GE32 = chan_tuning_2069_rev33_37;
					tbl_len = ARRAYSIZE(chan_tuning_2069_rev33_37);
				}
				break;
			case 39:
			case 40:
			case 44:
				if (PHY_XTAL_IS40M(pi)) {
					chan_info_tbl_GE32 = chan_tuning_2069_rev39_40;
					tbl_len = ARRAYSIZE(chan_tuning_2069_rev39_40);
				} else {
					chan_info_tbl_GE32 = chan_tuning_2069_rev39;
					tbl_len = ARRAYSIZE(chan_tuning_2069_rev39);
				}
				break;
			case 36:
				if (PHY_XTAL_IS40M(pi)) {
					chan_info_tbl_GE32 = chan_tuning_2069_rev36_40;
					tbl_len = ARRAYSIZE(chan_tuning_2069_rev36_40);
				} else {
					chan_info_tbl_GE32 = chan_tuning_2069_rev36;
					tbl_len = ARRAYSIZE(chan_tuning_2069_rev36);
				}
				break;
			default:

				PHY_ERROR(("wl%d: %s: Unsupported radio revision %d\n",
				           pi->sh->unit,
				           __FUNCTION__, RADIO2069REV(pi->pubpi->radiorev)));
				ASSERT(0);
				return FALSE;
			}
			tbli->data->chan_tuning_tbl_len = tbl_len;
			memcpy(tbli->data->chan_tuning, chan_info_tbl_GE32,
			        NUM_ROWS_CHAN_TUNING * sizeof(chan_info_radio2069revGE32_t));
		}
	}
	return TRUE;
}

static void
BCMATTACHFN(phy_ac_tbl_nvram_attach)(phy_ac_tbl_info_t *tbli, phy_info_t *pi)
{
#ifndef BOARD_FLAGS3
	uint32 bfl3; /* boardflags3 */
#endif // endif

	pi->ldo3p3_voltage = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_ldo3p3_voltage, -1));
	pi->paldo3p3_voltage = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_paldo3p3_voltage, -1));

#ifndef BOARD_FLAGS3
	if ((PHY_GETVAR_SLICE(pi, rstr_boardflags3)) != NULL) {
		bfl3 = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_boardflags3);
		BF3_FEMTBL_FROM_NVRAM(tbli->aci) = (bfl3 & BFL3_FEMTBL_FROM_NVRAM)
			>> BFL3_FEMTBL_FROM_NVRAM_SHIFT;
	} else {
		BF3_FEMTBL_FROM_NVRAM(tbli->aci) = 0;
	}
#endif /* BOARD_FLAGS3 */

	tbli->cbuck_out = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_cbuck_out, 0);
	pi->ccktpcloop_en = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_ccktpc_loop_en, 0);
	pi->txgaintbl5g = (int8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txgaintbl5g, 1));
}

uint8
wlc_phy_get_tbl_id_gainctrlbbmultluts(phy_info_t *pi, uint8 core)
{
	uint8 tbl_id = ACPHY_TBL_ID_GAINCTRLBBMULTLUTS;
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		tbl_id = ACPHY_TBL_ID_GAINCTRLBBMULTLUTS0;

		if ((phy_get_phymode(pi) != PHYMODE_RSDB) && (core == 1))  {
			tbl_id = ACPHY_TBL_ID_GAINCTRLBBMULTLUTS1;
		}
	} else if (IS_4364_3x3(pi)) {
	   switch (core) {
		   case 0:
			   tbl_id = ACPHY_TBL_ID_TXGAINCTRLBBMULTLUTS0;
			   break;
		   case 1:
			   tbl_id = ACPHY_TBL_ID_TXGAINCTRLBBMULTLUTS1;
			   break;
		   case 2:
			   tbl_id = ACPHY_TBL_ID_TXGAINCTRLBBMULTLUTS2;
			   break;
	   }
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		tbl_id = ACPHY_TBL_ID_GAINCTRLBBMULTLUTS0;
	} else if (ACREV_GE(pi->pubpi->phy_rev, 40) ||
		ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		tbl_id = AC2PHY_TBL_ID_GAINCTRLBBMULTLUTS0;
	} else {
		tbl_id = ACPHY_TBL_ID_GAINCTRLBBMULTLUTS;
	}
	return (tbl_id);
}

uint8
wlc_phy_get_tbl_id_estpwrshftluts(phy_info_t *pi, uint8 core)
{
	uint8 tbl_id;
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		tbl_id = ACPHY_TBL_ID_ESTPWRSHFTLUTS0;

		if ((phy_get_phymode(pi) != PHYMODE_RSDB) && (core == 1))  {
			tbl_id = ACPHY_TBL_ID_ESTPWRSHFTLUTS1;
		}
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		tbl_id = ACPHY_TBL_ID_ESTPWRSHFTLUTS0;
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		tbl_id = AC2PHY_TBL_ID_ESTPWRSHFTLUTS0;
	} else {
		tbl_id = ACPHY_TBL_ID_ESTPWRSHFTLUTS;
	}
	return (tbl_id);
}

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)

static phy_table_info_t acphy_tables_rev32[] = {
	{  2,  0,     40},
	{  3,  1,    256},
	{  4,  0,    256},
	{  5,  1,     22},
	{  6,  0,     72},
	{  7,  0,   1407},
	{  8,  3,      9},
	{  9,  0,     64},
	{ 10,  0,   1024},
	{ 13,  1,     68},
	{ 14,  1,    512},
	{ 19,  0,    800},
	{ 20,  2,    128},
	{ 21,  0,     96},
	{ 25,  1,     64},
	{ 29,  1,     35},
	{ 31,  1,     35},
	{ 32,  1,    128},
	{ 33,  1,    128},
	{ 34,  0,    128},
	{ 35,  0,    128},
	{ 37,  1,     77},
	{ 38,  1,     35},
	{ 39,  1,     35},
	{ 40,  1,      5},
	{ 50,  0,    160},
	{ 64,  1,    128},
	{ 65,  1,    128},
	{ 66,  0,    128},
	{ 67,  0,    128},
	{ 68,  0,    119},
	{ 69,  0,    119},
	{ 70,  1,     22},
	{ 72,  1,    128},
	{ 78,  1,    256},
	{ 88,  0,     72},
	{ 89,  0,     16},
	{ 91,  0,     64},
	{ 92,  0,     64},
	{ 94,  0,    105},
	{ 96,  1,    128},
	{ 97,  1,    128},
	{ 98,  0,    128},
	{ 99,  0,    128},
	{100,  0,    119},
	{101,  0,    119},
	{102,  1,     22},
	{104,  1,    128},
	{110,  1,    256},
	{114,  1,    183},
	{115,  1,      8},
	{126,  0,    105},
	{127,  1,     11},
	{128,  1,    128},
	{129,  1,    128},
	{130,  0,    128},
	{131,  0,    128},
	{132,  0,    119},
	{133,  0,    119},
	{134,  1,     22},
	{136,  1,    128},
	{142,  1,    256},
	{146,  1,    183},
	{147,  1,      8},
	{158,  0,    105},
	{159,  1,     11},
	{160,  1,    128},
	{161,  1,    128},
	{162,  0,    128},
	{163,  0,    128},
	{164,  0,    119},
	{165,  0,    119},
	{166,  1,     22},
	{168,  1,    128},
	{174,  1,    256},
	{178,  1,    183},
	{179,  1,      8},
	{190,  0,    105},
	{191,  1,     11},
	{210,  1,    183},
	{211,  1,      8},
	{223,  1,     11},
	{224,  1,     64},
	{225,  1,     64},
	{226,  0,     24},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev33[] = {
	{  2,  0,     40},
	{  3,  1,    256},
	{  4,  0,    256},
	{  5,  1,     22},
	{  6,  0,     72},
	{  7,  0,   1407},
	{  8,  3,      9},
	{  9,  0,     64},
	{ 10,  0,   1024},
	{ 13,  1,     68},
	{ 14,  1,    512},
	{ 19,  0,    800},
	{ 20,  2,    128},
	{ 21,  0,     96},
	{ 25,  1,     64},
	{ 29,  1,     35},
	{ 31,  1,     35},
	{ 37,  1,     77},
	{ 38,  1,     35},
	{ 39,  1,     35},
	{ 40,  1,      5},
	{ 50,  0,    160},
	{ 64,  1,    128},
	{ 65,  1,    128},
	{ 66,  0,    128},
	{ 67,  0,    128},
	{ 68,  0,	 119},
	{ 69,  0,	 119},
	{ 70,  1,	  22},
	{ 72,  1,	 128},
	{ 78,  1,	 256},
	{ 88,  0,	  72},
	{ 89,  0,	  16},
	{ 91,  0,	  64},
	{ 92,  0,	  64},
	{ 94,  0,	 105},
	{ 96,  1,	 128},
	{ 97,  1,	 128},
	{ 98,  0,	 128},
	{ 99,  0,	 128},
	{100,  0,	 119},
	{101,  0,	 119},
	{102,  1,	  22},
	{104,  1,	 128},
	{110,  1,	 256},
	{113,  1,	  64},
	{115,  1,	   8},
	{126,  0,	 105},
	{127,  1,	  11},
	{128,  1,	 128},
	{129,  1,	 128},
	{130,  0,	 128},
	{131,  0,	 128},
	{132,  0,	 119},
	{133,  0,	 119},
	{134,  1,	  22},
	{136,  1,	 128},
	{142,  1,	 256},
	{147,  1,	   8},
	{158,  0,	 105},
	{159,  1,	  11},
	{160,  1,	 128},
	{161,  1,	 128},
	{162,  0,	 128},
	{163,  0,	 128},
	{164,  0,	 119},
	{165,  0,	 119},
	{166,  1,	  22},
	{168,  1,	 128},
	{174,  1,	 256},
	{179,  1,	   8},
	{190,  0,	 105},
	{191,  1,	  11},
	{211,  1,	   8},
	{223,  1,	  11},
	{224,  1,	  64},
	{225,  1,	  64},
	{226,  0,	  24},
	{11,   1,	  22},
	{15,   1,	 128},
	{54,   1,	 128},
	{55,   0,	  16},
	{56,   0,	 128},
	{57,   0,	  96},
	{58,   0,	  24},
	{59,   0,	  11},
	{60,   0,	  77},
	{61,   1,	  16},
	{320,  0,	 247},
	{321,  1,	  36},
	{322,  1,	  24},
	{323,  0,	  24},
	{324,  0,	 105},
	{325,  0,	 119},
	{326,  0,	 119},
	{327,  1,	  22},
	{328,  1,	  11},
	{352,  0,	 247},
	{353,  1,	  36},
	{354,  1,	  24},
	{355,  1,	  24},
	{356,  0,	 105},
	{357,  0,	 119},
	{358,  0,	 119},
	{359,  1,	  22},
	{360,  1,	  11},
	{384,  0,	 247},
	{385,  1,	  36},
	{386,  1,	  24},
	{387,  0,	  24},
	{388,  0,	 105},
	{389,  0,	 119},
	{390,  0,	 119},
	{391,  1,	  22},
	{392,  1,	  11},
	{416,  0,	 247},
	{417,  1,	  36},
	{418,  1,	  24},
	{419,  0,	  24},
	{420,  0,	 105},
	{421,  0,	 119},
	{422,  0,	 119},
	{423,  1,	  22},
	{424,  1,	  11},
	{0xff, 0,      0}
};

static phy_table_info_t acphy_tables_rev47[] = {
	{  2,  0,   40},
	{  3,  1,  512},
	{  4,  0,  512},
	{  5,  1,  150},
	{  6,  0,   72},
	{  7,  0, 1407},
	{  8,  3,   44},
	{  9,  0,   64},
	{ 10,  0, 1024},
	{ 11,  1,  150},
	{ 13,  1,   68},
	{ 19,  0,  800},
	{ 20,  2,  128},
	{ 21,  0,   96},
	{ 25,  1,   64},
	{ 29,  1,   35},
	{ 31,  1,   35},
	{ 37,  1,   77},
	{ 38,  1,   35},
	{ 39,  1,   35},
	{ 40,  1,    5},
	{ 42,  1,  160},
	{ 43,  1,  512},
	{ 44,  1,  512},
	{ 45,  1, 1024},
	{ 50,  0,  160},
	{ 54,  1,  128},
	{ 55,  0,   16},
	{ 56,  0,  128},
	{ 57,  0,   96},
	{ 58,  0,   24},
	{ 59,  0,   11},
	{ 60,  0,   77},
	{ 61,  1,   16},
	{ 62,  0,    8},
	{ 63,  1,  256},
	{ 64,  1,  128},
	{ 65,  1,  128},
	{ 66,  0,  128},
	{ 67,  0,  128},
	{ 68,  0,  119},
	{ 69,  0,  119},
	{ 70,  1,   22},
	{ 72,  1,  128},
	{ 88,  0,   72},
	{ 89,  0,   16},
	{ 90,  0,   64},
	{ 91,  0,   64},
	{ 92,  0,   64},
	{ 94,  0,  105},
	{ 96,  1,  128},
	{ 97,  1,  128},
	{ 98,  0,  128},
	{ 99,  0,  128},
	{100,  0,  119},
	{101,  0,  119},
	{102,  1,   22},
	{104,  1,  128},
	{112,  2,  128},
	{113,  1,   64},
	{115,  1,    8},
	{126,  0,  105},
	{127,  1,   11},
	{128,  1,  128},
	{129,  1,  128},
	{130,  0,  128},
	{131,  0,  128},
	{132,  0,  119},
	{133,  0,  119},
	{134,  1,   22},
	{136,  1,  128},
	{147,  1,    8},
	{158,  0,  105},
	{159,  1,   11},
	{160,  1,  128},
	{161,  1,  128},
	{162,  0,  128},
	{163,  0,  128},
	{164,  0,  119},
	{165,  0,  119},
	{166,  1,   22},
	{168,  1,  128},
	{179,  1,    8},
	{190,  0,  105},
	{191,  1,   11},
	{211,  1,    8},
	{223,  1,   11},
	{226,  0,   24},
	{227,  1,  512},
	{228,  1,  256},
	{229,  3,  128},
	{320,  0,  247},
	{321,  1,   36},
	{322,  1,   24},
	{323,  0,   24},
	{324,  0,  105},
	{325,  0,  119},
	{326,  0,  119},
	{327,  1,   22},
	{328,  1,   11},
	{352,  0,  247},
	{353,  1,   36},
	{354,  1,   24},
	{355,  0,   24},
	{356,  0,  105},
	{357,  0,  119},
	{358,  0,  119},
	{359,  1,   22},
	{360,  1,   11},
	{384,  0,  247},
	{385,  1,   36},
	{386,  1,   24},
	{387,  0,   24},
	{388,  0,  105},
	{389,  0,  119},
	{390,  0,  119},
	{391,  1,   22},
	{392,  1,   11},
	{416,  0,  247},
	{417,  1,   36},
	{418,  1,   24},
	{419,  0,   24},
	{420,  0,  105},
	{421,  0,  119},
	{422,  0,  119},
	{423,  1,   22},
	{424,  1,   11},
	{0xff,   0,  0},
};

static phy_table_info_t acphy_tables_rev51[] = {
	// FIXME63178: need to convert from def(acphy_table_list) in acphydefs_rev51.tcl
	{0xff,   0,  0},
};

static phy_table_info_t acphy_tables_rev12_rsdb[] = {
	{2,	0,	40},
	{3,	1,	256},
	{4,	0,	256},
	{5,	1,	22},
	{6,	0,	72},
	{7,	0,	1136},
	{8,	3,	9},
	{9,	0,	48},
	{10,	0,	768},
	{12,	0,	160},
	{13,	1,	68},
	{14,	1,	512},
	{19,	0,	800},
	{20,	2,	128},
	{24,	1,	520},
	{25,	1,	64},
	{29,	1,	35},
	{31,	1,	35},
	{35,	1,	77},
	{36,	1,	35},
	{37,	1,	35},
	{38,	1,	5},
	{64,	1,	128},
	{65,	1,	128},
	{66,	0,	128},
	{67,	0,	128},
	{68,	0,	119},
	{69,	0,	119},
	{70,	1,	22},
	{71,	1,	512},
	{72,	1,	128},
	{73,	1,	1024},
	{78,	1,	256},
	{79,	2,	64},
	{80,	0,	8},
	{85,	0,	128},
	{86,	0,	128},
	{87,	0,	128},
	{88,	0,	72},
	{89,	0,	16},
	{90,	0,	64},
	{91,	0,	64},
	{92,	0,	64},
	{93,	0,	128},
	{94,	0,	105},
	{105,	1,	1024},
	{113,	1,	64},
	{114,	0,	24},
	{115,	1,	8},
	{116,	1,	64},
	{128,	2,	128},
	{129,	1,	64},
	{148,	1,	64},
	{0xff,	0,	0}
};

static phy_table_info_t acphy_tables_rev12_mimo_80p80[] = {
	{2,	0,	40},
	{3,	1,	256},
	{4,	0,	256},
	{5,	1,	22},
	{6,	0,	72},
	/* Table dump crash is seen due to incontinuity in addresses of RFSeq- table id#7 */
	//{7,	0,	1136},
	{8,	3,	9},
	/* table id#9-AntSwCtrlLUT changed to dump for 2 cores only */
	{9,	0,	32},
	/* table id#10-FEMCtrlLUT changed to dump for 2 cores only */
	{10,	0,	512},
	{12,	0,	160},
	{13,	1,	68},
	{14,	1,	512},
	{19,	0,	800},
	{20,	2,	128},
	//{23,	1,	520}, /* bfrRptTbl - Not required for 4349 chips */
	{24,	1,	520},
	{25,	1,	64},
	{27,	0,	46},
	{29,	1,	35},
	{30,	1,	64},
	{31,	1,	35},
	{35,	1,	77},
	{36,	1,	35},
	{37,	1,	35},
	{38,	1,	5},
	{44,	0,	160},
	{64,	1,	128},
	{65,	1,	128},
	{66,	0,	128},
	{67,	0,	128},
	{68,	0,	119},
	{69,	0,	119},
	{70,	1,	22},
	//{71,	1,	512}, /* epsilonTable0 - Not required for 4349 chips */
	{72,	1,	128},
	{73,	1,	1024},
	{78,	1,	256},
	{79,	2,	64},
	//{80,	0,	8},   /* snoopAGC - Not required for 4349 chips */
	{85,	0,	128},
	//{86,	0,	128}, /* bbpdepsilonTablei0 - Not required for 4349 chips */
	//{87,	0,	128}, /* bbpdepsilonTableq0 - Not required for 4349 chips */
	{88,	0,	72},
	{89,	0,	16},
	{90,	0,	64},
	{91,	0,	64},
	{92,	0,	64},
	{93,	0,	128},
	{94,	0,	105},
	{96,	1,	128},
	{97,	1,	128},
	{98,	0,	128},
	{99,	0,	128},
	{100,	0,	119},
	{101,	0,	119},
	{102,	1,	22},
	//{103,	1,	512}, /* epsilonTable1 - Not required for 4349 chips */
	{104,	1,	128},
	{105,	1,	1024},
	{110,	1,	256},
	{111,	2,	64},
	{113,	1,	64},
	{114,	0,	24},
	{115,	1,	8},
	{116,	1,	64},
	{117,	0,	128},
	//{118,	0,	128}, /* bbpdepsilonTablei1 - Not required for 4349 chips */
	//{119,	0,	128}, /* bbpdepsilonTableq1 - Not required for 4349 chips */
	{120,	0,	72},
	{121,	0,	16},
	{122,	0,	64},
	{123,	0,	64},
	{124,	0,	64},
	{125,	0,	128},
	{126,	0,	105},
	{128,	2,	128},
	{129,	1,	64},
	{147,	1,	8},
	{148,	1,	64},
	{160,	2,	128},
	{161,	1,	64},
	{0xff,	0,	0}
};

static phy_table_info_t acphy_tables_rev9[] = {
	{  1, 0,  128},
	{  2, 0,   40},
	{  3, 1,  256},
	{  4, 0,  256},
	{  5, 1,   22},
	{  6, 0,   72},
	{  7, 0, 1136},
	{  8, 3,    9},
	{  9, 0,   48},
	{ 10, 0,  768},
	{ 12, 0,  160},
	{ 13, 1,   68},
	{ 14, 1,  512},
	{ 15, 1,  128},
	{ 16, 1, 8784},
	{ 17, 2, 464},
	{ 18, 1, 1920},
	{ 19, 0,  800},
	{ 20, 2,  128},
	{ 21, 0,   72},
	{ 22, 0,   64},
	{ 23, 1,  520},
	{ 24, 1,  520},
	{ 25, 1,  64},
	{ 26, 1,  22},
	{ 27, 0,   46},
	{ 28, 0,   46},
	{ 29, 1,   35},
	{ 30, 1,   64},
	{ 31, 1,   35},
	{ 32, 2,  128},
	{ 33, 1,   24},
	{ 34, 2,   512},
	{ 35, 1,   77},
	{ 64, 0,  128},
	{ 65, 1,  128},
	{ 66, 0,  128},
	{ 67, 0,  128},
	{ 68, 0,  119},
	{ 69, 0,  119},
	{ 70, 1,   22},
	{ 71, 1,  512},
	{ 72, 1,  128},
	{ 73, 1, 1024},
	{ 80, 0,    8},
	{ 81, 0,  128},
	{ 82, 0,  128},
	{ 83, 0,  128},
	{ 84, 0,  128},
	{ 85, 0,  128},
	{ 86, 0,  128},
	{ 87, 0,  128},
	{ 88, 0,  72},
	{ 89, 0,  16},
	{ 90, 0,  64},
	{ 91, 0,  64},
	{ 92, 0,  64},
	{ 93, 0,  128},
	{ 94, 0,  105},
	{ 96, 0,  128},
	{ 97, 1,  128},
	{ 98, 0,  128},
	{ 99, 0,  128},
	{100, 0,  119},
	{101, 0,  119},
	{102, 1,   22},
	{103, 1,  512},
	{104, 1,  128},
	{105, 1, 1024},
	{112, 1,   64},
	{113, 1,   64},
	{117, 0,   128},
	{118, 0,   128},
	{119, 0,   128},
	{120, 0,  72},
	{121, 0,  16},
	{122, 0,  64},
	{123, 0,  64},
	{124, 0,  64},
	{125, 0,  128},
	{126, 0,  105},
	{128, 0,  128},
	{129, 1,  128},
	{130, 0,  128},
	{131, 0,  128},
	{132, 0,  119},
	{133, 0,  119},
	{134, 1,   22},
	{135, 1,  512},
	{136, 1,  128},
	{137, 1, 1024},
	{149, 0,   128},
	{150, 0,   128},
	{151, 0,   128},
	{152, 0,   72},
	{153, 0,   16},
	{154, 0,   64},
	{155, 0,   64},
	{156, 0,   64},
	{157, 0,   128},
	{158, 0,   105},
	{ 0xff,	0,	0 }
};

static phy_table_info_t acphy_tables_rev13[] = {
/*	{  1, 0, 128},	// mcstbl */
	{  2, 0, 40},	/* TxEvmTbl */
	{  3, 0, 256},	/* NvNoiseShapingTbl */
/*	{  4, 0, 256},	// NvRxEvmShapingTbl */
/*	{  5, 1, 22},	// PhaseTrackTbl */
/*	{  7, 0, 1136},	// RFSeq */
	{  8, 3, 9},	/* RFSeqExt */
	{  9, 0, 48},	/* AntSwCtrlLUT */
	{ 10, 0, 768},	/* FEMCtrlLUT */
	{ 11, 0, 105},	/* GainLimit */
	{ 12, 0, 160},	/* iqloCaltbl */
	{ 13, 1, 68},	/* paprtbl */
	{ 14, 1, 512},	/* sampleplaytbl */
	{ 17, 2, 464},	/* bfeConfigTbl */
	{ 19, 0, 1600},	/* fastchswTable */
	{ 20, 2, 128},	/* RfseqBundle */
/*	{ 21, 0, 72},	// lnaRoutLUT */
	{ 25, 1, 64},	/* NvAdjTbl */
	{ 26, 1, 22},	/* PhaseTrackTbl */
	{ 29, 1, 35},	/* SlnaGain */
	{ 31, 1, 35},	/* SlnaGainbtExtLna */
	{ 32, 2, 128},	/* gainCtrlbbMultLuts */
	{ 33, 1, 24},	/* estPwrShftLuts */
	{ 34, 2, 512},	/* channelSmootheningTbl */
	{ 64, 0, 128},	/* estPwrLuts0 */
	{ 65, 1, 128},	/* iqCoefLuts0 */
	{ 66, 0, 128},	/* loftCoefLuts0 */
	{ 67, 0, 128},	/* rfPwrLuts0 */
	{ 68, 0, 119},	/* Gain0 */
	{ 69, 0, 119},	/* GainBits0 */
	{ 70, 1, 22},	/* RssiClipGain0 */
/*	{ 71, 1, 512},	// epsilonTable0 */
	{ 72, 1, 128},	/* scalarTable0 */
	{ 73, 1, 1024},	/* core0chanestTbl */
	{ 80, 0, 8},	/* snoopAGC */
	{ 81, 0, 128},	/* snoopPeak */
	{ 82, 0, 128},	/* snoopcckLms */
	{ 83, 0, 128},	/* snoopLms */
	{ 84, 0, 128},	/* snoopdccmp */
	{ 85, 0, 128},	/* bbpdrfPwrLuts0 */
/*	{ 86, 0, 128},	// bbpdepsilonTablei0 */
/*	{ 87, 0, 128},	// bbpdepsilonTableq0 */
	{ 88, 0, 72},	/* mcsinfotbl0 */
	{ 89, 0, 16},	/* scaleadjustfactorstbl0 */
	{ 90, 0, 64},	/* breakpointstbl0 */
	{ 91, 0, 64},	/* scalefactorstbl0 */
	{ 92, 0, 64},	/* scalefactorsdeltatbl0 */
	{ 93, 0, 128},	/* papdlutselect0 */
	{112, 1, 64},	/* adcsampcap */
	{113, 1, 64},	/* dacrawsampplay */
	{ 0xff,	0,	0 }
};

static phy_table_info_t acphy_tables_rev3[] = {
	{  1, 0,  128},
	{  2, 0,   40},
	{  3, 0,  256},
	{  4, 0,  256},
	{  5, 1,   22},
	{  6, 0,   72},
	{  7, 0, 1136},
	{  8, 3,    9},
	{  9, 0,   48},
	{ 10, 0,  320},
	{ 11, 0,  105},
	{ 12, 0,  160},
	{ 13, 1,   68},
	{ 14, 1,  512},
	{ 15, 1,  128},
	{ 16, 1, 8784},
	{ 18, 1, 1920},
	{ 19, 0,  512},
	{ 20, 2,  128},
	{ 21, 0,   72},
	{ 22, 0,   64},
	{ 23, 1,  520},
	{ 24, 1,  520},
	{ 27, 0,   46},
	{ 28, 0,   46},
	{ 29, 1,   35},
	{ 30, 1,   64},
	{ 32, 2,  128},
	{ 33, 1,   24},
	{ 64, 0,  128},
	{ 65, 1,  128},
	{ 66, 0,  128},
	{ 67, 0,  128},
	{ 68, 0,  119},
	{ 69, 0,  119},
	{ 70, 1,   22},
	{ 71, 1,  128},
	{ 72, 1,  128},
	{ 73, 1, 1024},
	{ 80, 0,    8},
	{ 81, 0,  128},
	{ 82, 0,  128},
	{ 83, 0,  128},
	{ 84, 0,  128},
	{ 96, 0,  128},
	{ 97, 1,  128},
	{ 98, 0,  128},
	{ 99, 0,  128},
	{100, 0,  119},
	{101, 0,  119},
	{102, 1,   22},
	{103, 1,  128},
	{104, 1,  128},
	{105, 1, 1024},
	{ 0xff,	0,	0 }
};
static phy_table_info_t acphy2_tables[] = {
	{ 2, 0,   40},
	{ 3, 1,  256},
	{ 4, 0,  256},
	{ 7, 0, 1136},
	{ 8, 3,    9},
	{ 9, 0,   16},
	{10, 0,  256},
	{11, 0,  105},
	{12, 0,  160},
	{13, 1,   68},
	{14, 1,  512},
	{19, 0,  512},
	{20, 2,  128},
	{21, 0,   24},
	{22, 0,   35},
	{25, 1,   64},
	{26, 1,   22},
	{29, 1,   35},
	{31, 1,   35},
	{32, 2,  128},
	{33, 0,   24},
	{34, 0,  512},
	{64, 0,  128},
	{65, 1,  128},
	{66, 0,  128},
	{67, 0,  128},
	{68, 0,  119},
	{69, 0,  119},
	{70, 1,   19},
	{71, 1,   64},
	{72, 1,   64},
	{73, 1, 1024},
	{80, 0,    8},
	{81, 0,  128},
	{82, 0,  128},
	{83, 0,  128},
	{84, 0,  128},

	{0xff, 0, 0}
};
static phy_table_info_t acphy_tables[] = {
	{  1, 0,  128},
	{  2, 0,   40},
	{  3, 1,  256},
	{  4, 0,  256},
	{  5, 1,   22},
	{  6, 0,   72},
	{  7, 0, 1136},
	{  8, 3,    9},
	{  9, 0,   48},
	{ 10, 0,   96},
	{ 11, 0,  105},
	{ 12, 0,  160},
	{ 13, 1,   68},
	{ 14, 1,  512},
	{ 15, 1,  128},
	{ 16, 1, 8784},
	{ 17, 2,  464},
	{ 18, 1, 1920},
	{ 19, 0,  512},
	{ 20, 2,  128},
	{ 21, 0,   72},
	{ 22, 0,   64},
	{ 23, 1,  520},
	{ 24, 1,  520},
	{ 32, 2,  128},
	{ 33, 1,   24},
	{ 64, 0,  128},
	{ 65, 1,  128},
	{ 66, 0,  128},
	{ 67, 0,  128},
	{ 68, 0,  119},
	{ 69, 0,  119},
	{ 70, 1,   19},
	{ 71, 1,   64},
	{ 72, 1,   64},
	{ 73, 1, 1024},
	{ 80, 0,    8},
	{ 81, 0,  128},
	{ 82, 0,  128},
	{ 83, 0,  128},
	{ 84, 0,  128},
	{ 96, 0,  128},
	{ 97, 1,  128},
	{ 98, 0,  128},
	{ 99, 0,  128},
	{100, 0,  119},
	{101, 0,  119},
	{102, 1,   19},
	{103, 1,   64},
	{104, 1,   64},
	{105, 1, 1024},
	{128, 0,  128},
	{129, 1,  128},
	{130, 0,  128},
	{131, 0,  128},
	{132, 0,  119},
	{133, 0,  119},
	{134, 1,   19},
	{135, 1,   64},
	{136, 1,   64},
	{137, 1, 1024},
	{ 0xff,	0,	0 }
};

static phy_table_info_t acphy_tables_rev44[] = {
	{2, 0,     56},
	{3, 1,   1024},
	{4, 0,   1024},
	{5, 1,     86},
	{6, 0,     72},
	{8, 3,     44},
	{9, 0,     64},
	{10, 0,	 1024},
	{12, 2,	  256},
	{13, 1,	  68 },
	{14, 1,	  256},
	{16, 1,	 8784},
	{19, 1,	   36},
	{20, 2,	  128},
	{22, 0,	   64},
	{24, 1,	  688},
	{25, 1,	   64},
	{27, 0,	   46},
	{29, 1,	   35},
	{30, 1,	   64},
	{31, 1,	   35},
	{37, 1,	   77},
	{38, 1,	   35},
	{39, 1,	   35},
	{40, 1,	    5},
	{50, 0,	  160},
	{51, 0,	 1024},
	{52, 0,	 1024},
	{53, 1,	 1024},
	{54, 1,	  128},
	{55, 0,	   16},
	{56, 0,	  128},
	{58, 0,	   24},
	{59, 0,	   11},
	{60, 0,	   77},
	{64, 1,	  128},
	{65, 1,	  128},
	{66, 1,	  128},
	{67, 0,	  128},
	{68, 0,	  119},
	{69, 0,	  119},
	{70, 1,	   22},
	{71, 1,	  512},
	{72, 1,	  128},
	{73, 1,	 4096},
	{78, 1,	 2048},
	{85, 0,	  128},
	{88, 0,	   72},
	{89, 0,	   16},
	{90, 0,	   64},
	{91, 0,	   64},
	{92, 0,	   64},
	{93, 0,	  128},
	{94, 0,	  105},
	{96, 1,	  128},
	{97, 1,	  128},
	{98, 1,	  128},
	{99, 0,	  128},
	{100, 0,  119},
	{101, 0,  119},
	{102, 1,   22},
	{103, 1,  512},
	{104, 1,  128},
	{105, 1, 4096},
	{112, 2,  128},
	{113, 1,   64},
	{115, 1,    8},
	{116, 1,   64},
	{117, 0,  128},
	{125, 0,  128},
	{126, 0,  105},
	{127, 1,   11},
	{144, 2,  128},
	{147, 1,    8},
	{159, 1,   11},
	{320, 0,  247},
	{321, 1,   36},
	{322, 1,   24},
	{323, 0,   24},
	{324, 0,  105},
	{325, 0,  119},
	{326, 0,  119},
	{327, 1,   22},
	{328, 1,   11},
	{352, 0,  247},
	{353, 1,   36},
	{354, 1,   24},
	{355, 0,   24},
	{356, 0,  105},
	{357, 0,  119},
	{358, 0,  119},
	{359, 1,   22},
	{360, 1,   11},
	{0xff, 0,   0}
};

static void
phy_ac_tbl_read_table(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval)
{
	phy_ac_tbl_info_t *aci = (phy_ac_tbl_info_t *)ctx;
	phy_info_t *pi = aci->pi;

	uint16 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	phy_utils_write_phyreg(pi, ACPHY_TableID(pi->pubpi->phy_rev), (uint16)ti->table);
	phy_utils_write_phyreg(pi, ACPHY_TableOffset(pi->pubpi->phy_rev), (uint16)addr);

	if (ti->q == 3) {
		*qval++ = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
		*qval++ = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
		*qval = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
	} else if (ti->q == 2) {
		*qval++ = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
		*qval = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataWide(pi->pubpi->phy_rev));
	} else if (ti->q == 1) {
		*qval = phy_utils_read_phyreg(pi, ACPHY_TableDataLo(pi->pubpi->phy_rev));
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataHi(pi->pubpi->phy_rev));
	} else {
		*val = phy_utils_read_phyreg(pi, ACPHY_TableDataLo(pi->pubpi->phy_rev));
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static int
phy_ac_tbl_dumptbl(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ((phy_ac_tbl_info_t *)ctx)->pi;
	phy_table_info_t *ti = NULL;

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
		ti = acphy_tables_rev32;
	} else if (ACMAJORREV_33(pi->pubpi->phy_rev)) {
		ti = acphy_tables_rev33;
	} else if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		ti = acphy_tables_rev9;
	} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		ti = acphy_tables_rev47;
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		ti = acphy_tables_rev51;
	} else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		ti = acphy_tables_rev33;
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		uint16 phymode = phy_get_phymode(pi);
		if (phymode == PHYMODE_RSDB)
			ti = acphy_tables_rev12_rsdb;
		else if ((phymode == PHYMODE_MIMO) || (phymode == PHYMODE_80P80))
			ti = acphy_tables_rev12_mimo_80p80;
		else
			ASSERT(0);
	} else if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			ti = acphy_tables_rev13;
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
			ti = acphy_tables_rev3;
	} else if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			ti = acphy2_tables;
	} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			ti = acphy_tables_rev44;
	} else {
		ti = acphy_tables;
	}

	phy_tbl_do_dumptbl(((phy_ac_tbl_info_t *)ctx)->ti, ti, b);

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	return BCME_OK;
}
#endif // endif

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static bool wlc_phy_ac_proprietary_rates(phy_info_t *pi);
static bool wlc_phy_ac_stbc_capable(phy_info_t *pi);
#ifdef WL11AC_160
static bool wlc_phy_ac_160_capable(phy_info_t *pi);
#endif // endif
#ifdef WL11AC_80P80
static bool phy_ac_tbl_80p80_capable(phy_info_t *pi);
#endif // endif

/** Returns TRUE if PHY is capable of VHT Proprietary Rates */
static bool
wlc_phy_ac_proprietary_rates(phy_info_t *pi)
{
	return ((ACREV_IS(pi->pubpi->phy_rev, 1)) ||
		(ACREV_IS(pi->pubpi->phy_rev, 3)) ||
		(ACREV_GE(pi->pubpi->phy_rev, 6)));
}

/** Returns TRUE if PHY is capable of 11n Proprietary Rates */
static bool
wlc_phy_ac_ht_proprietary_rates(phy_info_t *pi)
{
	/* 43602 : no capability register available
	 *
	 *  4349 : miscSigCtrl.brcm_11n_256qam_support must be set in order to use 11n 256Q rates
	 *         Currently, 4349 supports 11n 256Q rates only on 2G 20Mhz channels
	 */

	return ACMAJORREV_3(pi->pubpi->phy_rev) ||
	       ACMAJORREV_4(pi->pubpi->phy_rev) ||
	       ACMAJORREV_5(pi->pubpi->phy_rev) ||
	       ACMAJORREV_36(pi->pubpi->phy_rev);
}

/** Return True if PHY is capable of STBC. Currently returns 1 for all ACPHY except 4335A0/B0 */
static bool
wlc_phy_ac_stbc_capable(phy_info_t *pi)
{
	return !(ACMAJORREV_1(pi->pubpi->phy_rev) &&
	(ACMINORREV_0(pi) || ACMINORREV_1(pi)));
}

#ifdef WL11AC_160
/** Return True if PHY is capable of 160 */
static bool wlc_phy_ac_160_capable(phy_info_t *pi)
{
	return (ACMAJORREV_47(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev));
}
#endif /* WL11AC_160 */

#ifdef WL11AC_80P80
/** Return True if PHY is capable of 80p80 */
static bool phy_ac_tbl_80p80_capable(phy_info_t *pi)
{
	return (ACMAJORREV_33(pi->pubpi->phy_rev)) ? TRUE: FALSE;
}
#endif /* WL11AC_80P80 */

uint8 wlc_phy_ac_phycap_maxbw(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	if (PHY_CAP_160MHZ & pi_ac->phy_caps) return BW_160MHZ;
	else if (PHY_CAP_80MHZ & pi_ac->phy_caps) return BW_80MHZ;
	else if (PHY_CAP_40MHZ & pi_ac->phy_caps) return BW_40MHZ;
	else return BW_20MHZ;
}

static bool wlc_phy_ac_mu_bfr_capable(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	return (pi_ac->phy_caps & PHY_CAP_MU_BFR) ? TRUE : FALSE;
}

static bool wlc_phy_ac_mu_bfe_capable(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	return (pi_ac->phy_caps & PHY_CAP_MU_BFE) ? TRUE : FALSE;
}

static bool wlc_phy_ac_1024qam_capable(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	return (pi_ac->phy_caps & PHY_CAP_1024QAM) ? TRUE : FALSE;
}

#ifdef WL11ULB
static bool wlc_phy_ac_ulb_10_capable(phy_info_t *pi)
{
	if ((ACMAJORREV_4(pi->pubpi->phy_rev)) || (ACMAJORREV_36(pi->pubpi->phy_rev)) ||
			IS_4364_1x1(pi) || (ACMAJORREV_32(pi->pubpi->phy_rev)) ||
			(ACMAJORREV_33(pi->pubpi->phy_rev)))
		/* No phycapability bit indicating ulb capable for 4349 */
		/* 10MHz mode is supported in 4349B0 and 4364 1x1 PHYs */
		return TRUE;
	else
		return FALSE;
}

static bool wlc_phy_ac_ulb_5_capable(phy_info_t *pi)
{
	if ((ACMAJORREV_4(pi->pubpi->phy_rev)) || (ACMAJORREV_36(pi->pubpi->phy_rev)) ||
			IS_4364_1x1(pi) || (ACMAJORREV_32(pi->pubpi->phy_rev)) ||
			(ACMAJORREV_33(pi->pubpi->phy_rev)))
		/* No phycapability bit indicating ulb capable for 4349 */
		/* 5MHz mode is supported in 4349B0 and 4364 1x1 PHYs */
		return TRUE;
	else
		return FALSE;
}

static bool wlc_phy_ac_ulb_2P5_capable(phy_info_t *pi)
{
	if (IS_4364_1x1(pi) ||
			(ACMAJORREV_36(pi->pubpi->phy_rev)))
		/* 2.5MHz mode is supported in 4364 1x1 PHY */
		return TRUE;
	else
		/* 4349B0 doesnot support 2.5 MHz */
		return FALSE;
}
#endif /* WL11ULB */

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
void
BCMATTACHFN(wlc_phy_rfldo_trim_value)(phy_ac_tbl_info_t *tbli, phy_info_t *pi)
{

	uint8 otp_select, srom_present;
	uint16 otp = 0;

	uint32 sromctl;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	sromctl = si_get_sromctl(pi->sh->sih);
	otp_select = (sromctl >> 4) & 0x1;
	srom_present = sromctl & 0x1;

	if (otp_select == 0 && srom_present)
		si_set_sromctl(pi->sh->sih, sromctl | (1 << 4));
	otp_read_word(pi->sh->sih, 16, &otp);
	if (otp_select == 0 && srom_present)
		si_set_sromctl(pi->sh->sih, sromctl);
	otp = (otp >> 8) & 0x1f;

	tbli->rfldo = otp;
}

void
WLBANDINITFN(wlc_phy_init_acphy)(phy_ac_tbl_info_t *tbli)
{
	phy_info_t *pi = tbli->pi;
	uint32 rfldo = 0;
	uint8 phyver;
	uint8 core;
	uint32 temp;
	uint32 ldo_setting = pi->ldo3p3_voltage;
	uint32 paldo_setting = pi->paldo3p3_voltage;
#ifdef ENABLE_FCBS
	int chanidx;
	uint8 iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, 0);
	uint32 tmp_val[160];
	uint8 stall_val;
	chanidx = 0;
	tmp_val[0] = 0;
	stall_val = 0;
#endif // endif
#if defined(WL_BEAMFORMING)
	uint32 txbf_stall_val;
	uint32 mlbf_lut[64] = {0x19003af, 0x19003af, 0x1d10390, 0x8f03f6, 0x12b03d3, 0x8f03f6,
		0xe603e6, 0x11a03d8, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400, 0x400,
		0x400, 0x400, 0x19003af, 0x19003af, 0x1d10390, 0x8f03f6, 0x12b03d3, 0x8f03f6,
		0xe603e6, 0x11a03d8, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400, 0x400,
		0x400, 0x400, 0x18003b5, 0x18003b5, 0x1a003a7, 0xb203f0, 0xf803e2, 0x8f03f6,
		0xc303ed, 0xe603e6, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400, 0x400,
		0x400, 0x400, 0x1d10390, 0x1d10390, 0x2d402d4, 0x400, 0x23d0351, 0x12b03d3,
		0x1f00380, 0x22e035b, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x2d402d4, 0x400,
		0x400, 0x400, 0x400};
	uint32 bfmuserdx_lut[8] = {0x11141000, 0x133c1228, 0x15641450, 0x178c1678, 0x19b418a0,
		0x1bdc1ac8, 0x1e041cf0, 0x1f18};
	uint32 mu_vmaddr[1] = {0x0e000c00};
#endif // endif
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && (tbli->cbuck_out != 0)) {
		si_pmu_regcontrol(pi->sh->sih, 5, 0xff000000,
			(tbli->cbuck_out << 24) | (tbli->cbuck_out << 28));
		FOREACH_CORE(pi, core) {
			MOD_RADIO_REGC(pi, GE32_PMU_CFG1, core, vrefadj_cbuck, tbli->cbuck_out);
		}
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		tbli->phy_hw_minor_rev = READ_PHYREG(pi, MinorVersion);
	} else {
		if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
			!ACMAJORREV_33(pi->pubpi->phy_rev) &&
			!ACMAJORREV_37(pi->pubpi->phy_rev) &&
			!ACMAJORREV_47_51(pi->pubpi->phy_rev))
			tbli->phy_hw_minor_rev = 0;
	}

	/* update corenum and coremask state variables */
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		phy_ac_update_phycorestate(pi);
		if (DCT_ENAB(pi))
			phy_ac_update_dutycycle_throttle_state(pi);
	}

	if (ACMAJORREV_5(pi->pubpi->phy_rev) ||
		(ACMAJORREV_2(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) ||
		ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		ACPHY_ENABLE_STALL(pi, 0);
	}

	/* Enable VHT prorietary rates if the PHY supports it */
	if (wlc_phy_ac_proprietary_rates(pi)) {
		MOD_PHYREG(pi, HTSigTones, ldpc_proprietary_mcs_vht, 1);
	}

	phyver = READ_PHYREGFLD(pi, Version, version);
	if (ACMAJORREV_5(pi->pubpi->phy_rev) || ACMAJORREV_0(pi->pubpi->phy_rev) ||
		ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (phyver == 0) {
			/* 4360a0 */
			if (tbli->rfldo == 0) {
				/* Use rfldo = 1.26 V for phyver = 0 by default */
				rfldo = 5;
			} else {
				rfldo = tbli->rfldo;
			}
		} else {
			/* 4360b0 and 43602 */
			if (tbli->rfldo <= 3) {
				rfldo = 0;
			} else {
				rfldo = tbli->rfldo - 3;
			}
		}
		if (ACMAJORREV_0(pi->pubpi->phy_rev) ||
				ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_37(pi->pubpi->phy_rev) ||
				ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			rfldo = rfldo << 20;
			si_pmu_regcontrol(pi->sh->sih, 0, 0x1f00000, rfldo);
		} else {
			rfldo = rfldo << 17;
			si_pmu_regcontrol(pi->sh->sih, 0, 0x3E0000, rfldo);
		}
	}
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* JIRA:
		 * for RSDB core 1, enable mini pmu cal in ucode wake up path
		 * this is true from 4355B1 variants of 4349
		 */
		if ((RADIO20693REV(pi->pubpi->radiorev) >= 0x13) &&
			(RADIO20693REV(pi->pubpi->radiorev) != 0x14) &&
			(RADIO20693REV(pi->pubpi->radiorev) != 0x15)) {
				wlapi_bmac_mhf(pi->sh->physhim, MHF4, MHF4_RSDB_CR1_MINIPMU_CAL_EN,
					MHF4_RSDB_CR1_MINIPMU_CAL_EN, WLC_BAND_ALL);
		}
	}

	/* Check if board uses internal 3.3V LDOs to supply the iPAs
	 * and enable the LDOs if used
	 */
	if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PALDO) != 0) {
		if (BCM4350_CHIP(pi->sh->chip)) {
			si_pmu_regcontrol(pi->sh->sih, 0x7, 0x100, 0x100);
		}
	}
	if ((ACMAJORREV_4(pi->pubpi->phy_rev) && !PHY_IPA(pi) && !ROUTER_4349(pi))) {
		if (ldo_setting != -1) {
			/* Below are the ldo_setting values */
			/* and the corresponding ldo3p3 voltage */
			/* 0->3.3V,7->3.2V,6->3.1V,5->3.0V,4->2.9V,1->3.35V,2->3.40V,3->3.45V */
			/* setting the address for LDO and writing the 3p3 voltage */
			phy_utils_pmu_regcontrol_access(pi, 4, &temp, 0);
			temp = temp & 0xFFFF8FFF;
			temp = temp | (ldo_setting << 12);
			phy_utils_pmu_regcontrol_access(pi, 4, &temp, 1);
		}
		if (paldo_setting != -1) {
			/* Below are the paldo_setting values */
			/* and the corresponding paldo3p3 voltage */
			/* 0->3.3V,7->3.2V,6->3.1V,5->3.0V,4->2.9V,1->3.35V,2->3.40V,3->3.45V */
			/* setting the address for PALDO and writing the 3p3 voltage */
			phy_utils_pmu_regcontrol_access(pi, 7, &temp, 0);
			temp = temp & 0xFFFFFFF8;
			temp = temp | paldo_setting;
			phy_utils_pmu_regcontrol_access(pi, 7, &temp, 1);
		}
	}

	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		if (paldo_setting != -1) {
			si_t *sih;
			sih = (si_t*)pi->sh->sih;
			/*
			 * paldo3p3 voltage LUT
			 * 3->3.45V 2->3.40V, 1->3.35V, 0->3.3V,
			 * 7->3.2V, 6->3.1V, 5->3.0V, 4->2.9V
			 */
			si_pmu_regcontrol(sih, 6, PMU_REG6_RFLDO_CTRL,
				(pi->paldo3p3_voltage<< PMU_REG6_RFLDO_CTRL_SHFT));
		}
	}
	/* Start with PHY not controlling any gpio's */
	si_gpiocontrol(pi->sh->sih, 0xffff, 0, GPIO_DRV_PRIORITY);

	if (BCM43602_CHIP(pi->sh->chip)) {
		W_REG(pi->sh->osh, D11_PSM_INTSEL_1(pi), 0x0);
	}

	if (IS_4364_1x1(pi) || IS_4364_3x3(pi)) {
		si_gci_set_femctrl(pi->sh->sih, pi->sh->osh, 1);
	}

	/* Set PHY timeouts */
	phy_ac_set_decode_timeouts(pi);

#ifdef WL_EAP_NOISE_MEASUREMENTS
	/* knoise rxgain override value initialization for 47452 */
	if ((ACMAJORREV_4(pi->pubpi->phy_rev) && ACMINORREV_3(pi))) {
		/* Gain tables for 47452 have trimmed LNA1 and BQ1 gains. Need
		 * to compensate with GM and BQ2 to align knoise measurements
		 * to other platforms.
		 */
		uint16 rxgain_hi, rxgain_lo, lpfgain_hi, lpfgain_lo;
		if (PHY_ILNA(pi)) {
			rxgain_hi  = 0x02b5;
			rxgain_lo  = 0x01d2;
			lpfgain_hi = 0x0010;
			lpfgain_lo = 0x0018;
		} else {
			rxgain_hi  = 0x0275;
			rxgain_lo  = 0x01d3;
			lpfgain_hi = 0x0010;
			lpfgain_lo = 0x0018;
		}
		wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_RXGAIN_HI(pi), rxgain_hi);
		wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_RXGAIN_LO(pi), rxgain_lo);
		wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_HI(pi), lpfgain_hi);
		wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_LO(pi), lpfgain_lo);
	}
#endif /* WL_EAP_NOISE_MEASUREMENTS */

#ifdef ENABLE_FCBS
	/* For shadowed reg/table, initialed values have to be put on both sets.
	 * Here I copy initialed values from setA to setB
	 */
	if (IS_FCBS(pi) && !ACMAJORREV_32(pi->pubpi->phy_rev) &&
		!ACMAJORREV_33(pi->pubpi->phy_rev)) {
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);

		wlc_phy_table_read_acphy(pi, iqlocal_tbl_id,
			160, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id,
			160, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_PAPR,
			68, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR,
			68, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS0,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS0,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN0,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN0,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS1,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS1,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN1,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN1,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS2,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS2,
			128, 0, 16, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN2,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RSSICLIPGAIN2,
			19, 0, 32, &tmp_val);
		wlc_phy_prefcbsinit_acphy(pi, 0);
		ACPHY_ENABLE_STALL(pi, stall_val);
	}
#endif /* ENABLE_FCBS */
	/* SW4345-273 Add TXBF support for 4345 */
#if defined(WL_BEAMFORMING)
	if ((ACMAJORREV_3(pi->pubpi->phy_rev) && ACMINORREV_3(pi)))
		MOD_PHYREG(pi, FrontEndDebug, txbfReportReadEn, 0x01);

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		txbf_stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);

		MOD_PHYREG(pi, BfrMuConfigReg0, useTxbfIndexAddr, 0);
		MOD_PHYREG(pi, BfeMuConfigReg0, useTxbfIndexAddr, 0);
		//no need for 43684
		if (!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			WRITE_PHYREG(pi, BfrMuConfigReg1, 0x1000);
			WRITE_PHYREG(pi, BfmMuConfig3, 0x1000);
			WRITE_PHYREG(pi, BfeMuConfigReg1, 0x1000);
			WRITE_PHYREG(pi, BfrMuConfigReg2, 0x1f92);
			WRITE_PHYREG(pi, BfmMuConfig4, 0x1f92);
			WRITE_PHYREG(pi, BfeMuConfigReg2, 0x1f92);
			MOD_PHYREG(pi, BfmMuConfig0, mlbfEnable, 1);
		}
		/* Enable column-wise phase-alignment */
		if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			(ACMAJORREV_32(pi->pubpi->phy_rev) && ACMINORREV_2(pi))) {
			MOD_PHYREG(pi, BfmMuConfig0, colPhaseAlignEnable, 1);
		}

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX, 8, 0x1000, 32,
			bfmuserdx_lut);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX, 64, 0x1040, 32, mlbf_lut);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX, 1, 0x1020, 32, mu_vmaddr);

		ACPHY_ENABLE_STALL(pi, txbf_stall_val);
	}
#endif /* WL_BEAMFORMING */

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RfseqMode, Trigger_override, 0);
		FOREACH_CORE(pi, core) {
			MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);
			MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, 1);
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RfseqMode, Trigger_override, 0);
	}
}

// 2000 * 25 us = 50 ms
#define TIMEOUT_50MS_CLKDIV8 2000

void
WLBANDINITFN(phy_ac_set_decode_timeouts)(phy_info_t *pi)
{
#if defined(WLTEST)
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev) ||
	    (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
		(ACMAJORREV_4(pi->pubpi->phy_rev)) ||
	    ACMAJORREV_5(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
#else /* defined(BCMINTERNAL) || defined(WLTEST) */
	/* For customer builds we like to enable PHY timeouts that will catch when
	 * pktproc is hanging and reset the PHY
	 */
	{ /* unconditionally enable timeouts for production images */
#endif // endif

		/* Reset the PHY if we are too long in CCK/OFDM payload decode state */
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, timeoutEn, cckpaydecodetimeoutEn, 1)
			MOD_PHYREG_ENTRY(pi, timeoutEn, ofdmpaydecodetimeoutEn, 1)
			MOD_PHYREG_ENTRY(pi, timeoutEn, resetRxontimeout, 1)
			WRITE_PHYREG_ENTRY(pi, ofdmpaydecodetimeoutlen, TIMEOUT_50MS_CLKDIV8)
			WRITE_PHYREG_ENTRY(pi, cckpaydecodetimeoutlen,  TIMEOUT_50MS_CLKDIV8)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	/* WAR: Enable timeout for 4347A0 for paydecode pktproc hang state */
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		ACPHY_REG_LIST_START
			WRITE_PHYREG_ENTRY(pi, ofdmpaydecodetimeoutlen, 0x7d0)
			/* 2000 * 25 us = 50 ms */
			WRITE_PHYREG_ENTRY(pi, cckpaydecodetimeoutlen, 0x7d0)
			/* 2000 * 25 us = 50 ms */
			WRITE_PHYREG_ENTRY(pi, nonpaydecodetimeoutlen, 0x20)
			/* 32 * 25 us = 800 us */
			WRITE_PHYREG_ENTRY(pi, timeoutEn, 0x817)
			/* ofdmpaydecodetimeoutEn = 1
			* cckpaydecodetimeoutEn = 1
			* nonpaydecodetimeoutEn = 1
			* resetCCAontimeout = 0
			* resetRxontimeout = 1
			*/
		ACPHY_REG_LIST_EXECUTE(pi);
	}
}

/* Returns AC PHY Capabilities */
uint32
wlc_phy_ac_caps(phy_info_t *pi)
{
	uint32 cap = (PHY_CAP_SGI | PHY_CAP_LDPC);
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	cap |= ((PHY_CAP_40MHZ | PHY_CAP_80MHZ) & pi_ac->phy_caps);
	cap |= wlc_phy_ac_proprietary_rates(pi) ? PHY_CAP_VHT_PROP_RATES : 0;
	cap |= wlc_phy_ac_ht_proprietary_rates(pi) ? PHY_CAP_HT_PROP_RATES : 0;
	cap |= wlc_phy_ac_stbc_capable(pi) ? PHY_CAP_STBC : 0;
#ifdef WL11AC_160
	cap |= wlc_phy_ac_160_capable(pi) ? PHY_CAP_160MHZ : 0;
#endif /* WL11AC_160 */
#ifdef WL11AC_80P80
	cap |= phy_ac_tbl_80p80_capable(pi) ? PHY_CAP_8080MHZ : 0;
#endif /* WL11AC_80P80 */
	cap |= (PHY_CAP_SU_BFR | PHY_CAP_SU_BFE);
	cap |= wlc_phy_ac_mu_bfr_capable(pi) ? PHY_CAP_MU_BFR : 0;
	cap |= wlc_phy_ac_mu_bfe_capable(pi) ? PHY_CAP_MU_BFE : 0;
	cap |= wlc_phy_ac_1024qam_capable(pi) ? PHY_CAP_1024QAM : 0;
#ifdef WL11ULB
	if (PHY_ULB_ENAB(pi)) {
		cap |= wlc_phy_ac_ulb_10_capable(pi) ? PHY_CAP_10MHZ : 0;
		cap |= wlc_phy_ac_ulb_5_capable(pi) ? PHY_CAP_5MHZ : 0;
		cap |= wlc_phy_ac_ulb_2P5_capable(pi) ? PHY_CAP_2P5MHZ : 0;
	}
#endif /* WL11ULB */
	return cap;
}

/* Returns additional AC capabilities added in ACREV_GE(44) */
uint32
wlc_phy_ac_caps1(phy_info_t *pi)
{
	uint32 cap = 0;
	if (ACREV_GE(pi->pubpi->phy_rev, HECAP_FIRST_ACREV))
		cap = pi->u.pi_acphy->phy_caps1;

	return cap;
}

void
wlc_phy_table_write_ext_acphy(phy_info_t *pi, const acphytbl_info_t *ptbl_info)
{
	uint8 trig_disable;

	/* Disable any ongoing ACI/FCBS if in progress */
	trig_disable = (ACPHY_ENABLE_FCBS_HWACI(pi)) ? wlc_phy_disable_hwaci_fcbs_trig(pi) : 0;

	phy_utils_write_phytable_ext(pi, ptbl_info, ACPHY_TableID(pi->pubpi->phy_rev),
		ACPHY_TableOffset(pi->pubpi->phy_rev), ACPHY_TableDataWide(pi->pubpi->phy_rev),
		ACPHY_TableDataHi(pi->pubpi->phy_rev), ACPHY_TableDataLo(pi->pubpi->phy_rev));

	/* Restore FCBS Trigger if needed */
	if (ACPHY_ENABLE_FCBS_HWACI(pi)) {
		wlc_phy_restore_hwaci_fcbs_trig(pi, trig_disable);
	} else {
		BCM_REFERENCE(trig_disable);
	}
}

void
wlc_phy_table_write_acphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset, uint32 width,
	const void *data)
{
	acphytbl_info_t tbl;
	uint8 mac_suspend = 1;
	uint8 stall_val = 0;

	mac_suspend = (R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (mac_suspend) {
		/* MAC suspend if MAC is enabled */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

#if defined(WL_PROXDETECT)
	phy_ac_tof_info_t *tofi = (phy_ac_tof_info_t *)pi->u.pi_acphy->tofi;
	uint16 tof_rfseq_bundle_offset = phy_ac_tof_get_rfseq_bundle_offset(tofi);

	phy_ac_tof_tbl_offset(pi->u.pi_acphy->tofi, id, offset);

	/* To keep track of first available offset usable by tof procs */
	if ((id == ACPHY_TBL_ID_RFSEQBUNDLE) && ((offset & 0xf) > tof_rfseq_bundle_offset))
		tof_rfseq_bundle_offset = (uint16)(offset & 0xf);
	if ((id == ACPHY_TBL_ID_RFSEQBUNDLE) || (id == ACPHY_TBL_ID_SAMPLEPLAY))
		phy_ac_tof_set_setup_done(tofi, FALSE);
#endif /* defined(WL_PROXDETECT) */

	if (stall_val == 0) {
		ACPHY_DISABLE_STALL(pi);
	}

	ASSERT(READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls) == 1);

	/*
	* PHY_TRACE(("wlc_phy_table_write_acphy, id %d, len %d, offset %d, width %d\n",
	* 	id, len, offset, width));
	*/
	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;

	wlc_phy_table_write_ext_acphy(pi, &tbl);

	if (stall_val == 0) {
		ACPHY_ENABLE_STALL(pi, stall_val);
	}

	if (mac_suspend) {
		/* Resume suspended MAC */
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_table_read_ext_acphy(phy_info_t *pi, const acphytbl_info_t *ptbl_info)
{
	uint8 trig_disable;

	/* Disable any ongoing ACI/FCBS if in progress */
	trig_disable = (ACPHY_ENABLE_FCBS_HWACI(pi)) ? wlc_phy_disable_hwaci_fcbs_trig(pi) : 0;

	phy_utils_read_phytable_ext(pi, ptbl_info, ACPHY_TableID(pi->pubpi->phy_rev),
		ACPHY_TableOffset(pi->pubpi->phy_rev), ACPHY_TableDataWide(pi->pubpi->phy_rev),
		ACPHY_TableDataHi(pi->pubpi->phy_rev), ACPHY_TableDataLo(pi->pubpi->phy_rev));

	/* Restore FCBS Trigger if needed */
	if (ACPHY_ENABLE_FCBS_HWACI(pi)) {
		wlc_phy_restore_hwaci_fcbs_trig(pi, trig_disable);
	} else {
		BCM_REFERENCE(trig_disable);
	}
}

void
wlc_phy_table_read_acphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, void *data)
{
	acphytbl_info_t tbl;
	uint8 stall_val = 0, mac_suspend = 1;
	uint8 coremask = phy_stf_get_data(pi->stfi)->phyrxchain;

	mac_suspend = (R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (mac_suspend) {
		/* MAC suspend if MAC is enabled */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	if (stall_val == 0)
		ACPHY_DISABLE_STALL(pi);

	ASSERT(READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls) == 1);

	/*	PHY_TRACE(("wlc_phy_table_read_acphy, id %d, len %d, offset %d, width %d\n",
	 *	id, len, offset, width));
	 */
	tbl.tbl_id = id;
	tbl.tbl_len = len;
	tbl.tbl_offset = offset;
	tbl.tbl_width = width;
	tbl.tbl_ptr = data;
	/* EPS TBL dump WAR--> turn on AFELDO and ADCLDO fore coremask 2 case in MIMO */
	if (id == 71) {
		if ((ACMAJORREV_4(pi->pubpi->phy_rev)) && (coremask == 2))
			wlc_phy_radio20693_mimo_lpopt_restore(pi);
	}

	wlc_phy_table_read_ext_acphy(pi, &tbl);
	/* EPS TBL dump WAR--> turn off AFELDO and ADCLDO fore coremask 2 case in MIMO */
		if (id == 71) {
			if ((ACMAJORREV_4(pi->pubpi->phy_rev)) && (coremask == 2))
				acphy_set_lpmode(pi, ACPHY_LP_RADIO_LVL_OPT);
	}

	if (stall_val == 0)
		ACPHY_ENABLE_STALL(pi, stall_val);

	if (mac_suspend) {
		/* Resume suspended MAC */
		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_table_setbit_acphy(phy_info_t *pi, uint32 id, uint32 offset,
	uint32 width, uint32 mask)
{
	uint32 temp;
	/* Implementation not extended for len > 1 */
	wlc_phy_table_read_acphy(pi, id, 1, offset, width, &temp);
	temp |= mask;
	wlc_phy_table_write_acphy(pi, id, 1, offset, width, &temp);
}

void
wlc_phy_table_clrbit_acphy(phy_info_t *pi, uint32 id, uint32 offset,
	uint32 width, uint32 mask)
{
	uint32 temp;
	/* Implementation not extended for len > 1 */
	wlc_phy_table_read_acphy(pi, id, 1, offset, width, &temp);
	temp &= ~mask;
	wlc_phy_table_write_acphy(pi, id, 1, offset, width, &temp);
}

/* This function is currently specific to 43012. To access the epsilon table this function
 * forces the dac clk to be taken from bbpll
 */
static void
wlc_phy_force_dac_clk_from_bbpll(phy_info_t *pi, bool set, uint16 *orig_dac_clk_pu)
{
	/* Currently commented out since the new phyregs are not yet checked in */
	if (set) {
		uint16 vals[] = {1, 3, 7, 5, 4};
		uint8 i;

		*orig_dac_clk_pu = READ_PHYREG(pi, dac_clock_from_bbpll);
		for (i = 0; i < 5; i++) {
			WRITE_PHYREG(pi, dac_clock_from_bbpll, vals[i]);
		}
	} else {
		/* Restore original value */
		WRITE_PHYREG(pi, dac_clock_from_bbpll, *orig_dac_clk_pu);
	}
}

/* Use this proc to write tables which need dac clk power up WAR.
 * Current list of tables which need this WAR are
 * epsilon0, epsilon1, bbpdepsiloni0, bbpdepsiloni1, bbpdepsilonq0, bbpdepsilonq1
 * Assuming that the function is called only for the above tables
 */
void wlc_phy_table_write_acphy_dac_war(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, void *data, uint8 core)
{
	uint16 orig_dac_clk_pu = 0, orig_ovr_dac_clk_pu = 0;
	bool dacclk_saved = FALSE;

	/* Implement the dac_war for tiny chips.For others simply call table read/write functions */
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* Force dacclk_saved to TRUE so that the value of this register is restored */
		dacclk_saved = TRUE;

		wlc_phy_force_dac_clk_from_bbpll(pi, 1, &orig_ovr_dac_clk_pu);
	} else if (TINY_RADIO(pi)) {
		dacclk_saved = wlc_phy_poweron_dac_clocks(pi, core, &orig_dac_clk_pu,
			&orig_ovr_dac_clk_pu);
	}
	if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		MOD_PHYREGCE(pi, wbcal_ctl_21, core, wb_mem_access_sel, 0);
		MOD_PHYREGCE(pi, papdEpsilonTable, core, mem_access_sel, 0);
	}
	/* Now Call table write */
	wlc_phy_table_write_acphy(pi, id, len, offset, width, data);

	if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		MOD_PHYREGCE(pi, wbcal_ctl_21, core, wb_mem_access_sel, 1);
		MOD_PHYREGCE(pi, papdEpsilonTable, core, mem_access_sel, 1);
	}

	/* Restore dac clocks after the table access */
	if (dacclk_saved == TRUE) {
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			wlc_phy_force_dac_clk_from_bbpll(pi, 0, &orig_ovr_dac_clk_pu);
		} else {
			wlc_phy_restore_dac_clocks(pi, core, orig_dac_clk_pu, orig_ovr_dac_clk_pu);
		}
	}
}

/* Use this proc to read tables which need dac clk power up WAR.
 * Current list of tables which need this WAR are
 * epsilon0, epsilon1, bbpdepsiloni0, bbpdepsiloni1, bbpdepsilonq0, bbpdepsilonq1
 * Assuming that the function is called only for the above tables
 */
void wlc_phy_table_read_acphy_dac_war(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, void *data, uint8 core)
{
	uint16 orig_dac_clk_pu = 0, orig_ovr_dac_clk_pu = 0;
	bool dacclk_saved = FALSE;

	/* Implement the dac_war for tiny chips.For others simply call table read/write functions */
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		dacclk_saved = TRUE;
		wlc_phy_force_dac_clk_from_bbpll(pi, 1, &orig_ovr_dac_clk_pu);
	} else if (TINY_RADIO(pi)) {
		dacclk_saved = wlc_phy_poweron_dac_clocks(pi, core, &orig_dac_clk_pu,
			&orig_ovr_dac_clk_pu);
	}

	/* Now Call table read */
	wlc_phy_table_read_acphy(pi, id, len, offset, width, data);

	/* Restore dac clocks after the table access */
	if (dacclk_saved == TRUE) {
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			wlc_phy_force_dac_clk_from_bbpll(pi, 0, &orig_ovr_dac_clk_pu);
		} else {
		wlc_phy_restore_dac_clocks(pi, core, orig_dac_clk_pu, orig_ovr_dac_clk_pu);
		}
	}
}

/* Use this proc to access tables channel smoothing filter tables in 4349.
 * These tables require the MAC_CLK_EN to be forced through PHYCTL
 */
void wlc_phy_table_write_tiny_chnsmth(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, const void *data)
{
	uint16 phyctl_save_val = 0, phyctl_saved = 0;

	if (ACMAJORREV_4(pi->pubpi->phy_rev) && (id == ACPHY_TBL_ID_CORE1CHANSMTH_FLTR)) {
		phyctl_saved = 1;
		wlc_phy_force_mac_clk(pi, &phyctl_save_val);
	}

	/* Now Call table read or write */
	wlc_phy_table_write_acphy(pi, id, len, offset, width, data);

	if (phyctl_saved) {
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phyctl_save_val);
	}
}

/* Proc to force mac_clk in phyctl register */
void wlc_phy_force_mac_clk(phy_info_t *pi, uint16 *orig_phy_ctl)
{
	*orig_phy_ctl = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
	W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi),
		(*orig_phy_ctl | MAC_PHY_FORCE_CLK));
}

void
wlc_phy_clear_static_table_acphy(phy_info_t *pi, const phytbl_info_t *ptbl_info,
	const uint32 tbl_info_cnt)
{
	uint32 idx, dpth_idx;
	/* Used 2 uint32 zeros because max table width we have is 64 */
	uint32 zeros[2] = { 0, 0};
	for (idx = 0; idx < tbl_info_cnt; idx++) {
		for (dpth_idx = 0; dpth_idx < ptbl_info[idx].tbl_len; dpth_idx++) {
			wlc_phy_table_write_acphy(pi, ptbl_info[idx].tbl_id, 1,
				dpth_idx, ptbl_info[idx].tbl_width, zeros);
		}
	}
}

static const uint16 *
wlc_phy_get_tx_pwrctrl_tbl_2069(phy_info_t *pi)
{
	const uint16 *tx_pwrctrl_tbl = NULL;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {

		tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev0;

		if (PHY_IPA(pi)) {

			switch (RADIO2069REV(pi->pubpi->radiorev)) {
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev16;
				break;
			case 17:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev17;
				break;
			case 23: /* iPa  case TXGain tables */
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev17;
				break;
			case 25: /* iPa  case TXGain tables */
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev25;
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev18;
				break;
			case 32:
			case 33:
			case 34:
			case 35:
			case 37:
			case 38:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev33_37;
				break;
			case 39:
			case 40:
			case 41:
			case 44:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev39;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_ipa_2g_2069rev0;
				break;

			}
		} else {

			switch (RADIO2069REV(pi->pubpi->radiorev)) {
			case 17:
			case 23:
			case 25:
				if (BF3_2GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
					phy_ac_tbl_blanking(pi->u.pi_acphy->tbli,
					acphy_txgain_epa_2g_2069rev17, pi->sromi->txidxcap2g, TRUE);
				}
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev17;
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev18;
				break;
			case 4:
			case 8:
			case 7: /* 43602a0 uses radio rev4 tx pwr ctrl tables */
				switch (BF3_TXGAINTBLID(pi->u.pi_acphy)) {
				case 0:
					tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev4;
					break;
				case 1:
					tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev4_id1;
					break;
				default:
					tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev4;
					break;
				}
				break;
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev16;
				break;
			case 32:
			case 33:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev33_35_36_37;
				break;
			case 34:
			case 36:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev34;
				break;
			case 35:
			case 37:
			case 38:
			case 39:
			case 40:
			case 41:
			case 44:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev33_35_36_37;
				break;
			case 64:
			case 66:
				tx_pwrctrl_tbl = pi->u.pi_acphy->tbli->gaintbl_2g;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_epa_2g_2069rev0;
				break;
			}
		}
	} else {
		if (PHY_IPA(pi)) {
			switch (RADIO2069REV(pi->pubpi->radiorev)) {
			case 17:
			case 23:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev17;
				break;
			case 25:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev25;
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev18;
				break;
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev16;
				break;
			case 32:
			case 33:
			case 34:
			case 35:
			case 37:
			case 38:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev33_37;
				break;
			case 39:
			case 40:
			case 41:
			case 44:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev39;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_ipa_5g_2069rev0;
				break;
			}
		} else {

			switch (RADIO2069REV(pi->pubpi->radiorev)) {
			case 17:
			case 23:
			case 25:
				if (BF3_5GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
					phy_ac_tbl_blanking(pi->u.pi_acphy->tbli,
					acphy_txgain_epa_5g_2069rev17, pi->sromi->txidxcap5g, TRUE);
				}
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev17;
				break;
			case 18:
			case 24:
			case 26:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev18;
				break;
			case 4:
			case 8:
			case 7: /* 43602a0 uses radio rev4 tx pwr ctrl tables */
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev4;
				break;
			case 64:
			case 66:
				tx_pwrctrl_tbl = pi->u.pi_acphy->tbli->gaintbl_5g;
				break;
			case 16:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev16;
				break;
			case 32:
			case 33:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev33_35_36;
				break;
			case 34:
			case 36:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev34;
				break;
			case 35:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev33_35_36;
				break;
			case 37:
			case 38:
			case 39:
			case 40:
			case 41:
			case 44:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev37_38;
				break;
			default:
				tx_pwrctrl_tbl = acphy_txgain_epa_5g_2069rev0;
				break;

			}
		}
	}

	return tx_pwrctrl_tbl;
}

static const uint16 *
wlc_phy_get_txgain_tbl_20694(phy_info_t *pi)
{
	uint sicoreunit;
	const uint16 *tx_pwrctrl_tbl = NULL;

	ASSERT((RADIOID_IS(pi->pubpi->radioid, BCM20694_ID)));

	sicoreunit = wlapi_si_coreunit(pi->sh->physhim);

	/* For B0, RADIOREV returns Main slice rev */
	if (sicoreunit == DUALMAC_AUX) {
		switch (RADIOREV_AUX(pi->pubpi->radiorev)) {
			case 4:
				tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ?
						acphy28nm_txgain_ipa_2g_p5_20694a0_rev4
							: acphy28nm_txgain_epa_5g_p5_20694a0_rev4;
				break;
			case 11:
				tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ?
						acphy28nm_txgain_ipa_2g_p5_20694b0_rev11
							: acphy28nm_txgain_epa_5g_p5_20694b0_rev8;
				break;
			case 5:
				tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ?
						acphy28nm_txgain_epa_2g_p5_20694a0_rev5
							: acphy28nm_txgain_epa_5g_p5_20694a0_rev5;
				break;
			case 9:
			case 15:
			default:
				tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ?
						acphy28nm_txgain_epa_2g_p5_20694b0_rev8
							: acphy28nm_txgain_epa_5g_p5_20694b0_rev8;
				break;

		}
	} else {
		switch (RADIOREV(pi->pubpi->radiorev)) {
			case 8:
			case 10:
			case 15:
				tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ?
						acphy28nm_txgain_epa_2g_p5_20694b0_rev8
							: acphy28nm_txgain_epa_5g_p5_20694b0_rev8;
				break;
			default:
				tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ?
						acphy28nm_txgain_epa_2g_p5_20694a0_rev5
							: acphy28nm_txgain_epa_5g_p5_20694a0_rev5;
		}
	}
	return tx_pwrctrl_tbl;
}

const uint16 *
wlc_phy_get_txgain_tbl_20695(phy_info_t *pi)
{
	const uint16 *tx_pwrctrl_tbl = NULL;

	ASSERT((RADIOID_IS(pi->pubpi->radioid, BCM20695_ID)));

	if (PHY_BAND5G_ENAB(pi)) {
		tx_pwrctrl_tbl = (PHY_IPA(pi)) ?
			((CHSPEC_IS2G(pi->radio_chanspec)) ? txgain_20695_phyrev36_2g_ipa
				: txgain_20695_phyrev36_5g_ipa)
			: ((CHSPEC_IS2G(pi->radio_chanspec)) ? txgain_20695_phyrev36_2g_epa
				: (ACMINORREV_0(pi) ? txgain_20695_phyrev36_5g_epa
				: txgain_20695_phyrev36_5g_epa_B0));
	} else {
		tx_pwrctrl_tbl = (PHY_IPA(pi)) ? txgain_20695_phyrev36_2g_ipa
			: ((CHSPEC_IS2G(pi->radio_chanspec)) ? txgain_20695_phyrev36_2g_epa
			: NULL);
	}

	return tx_pwrctrl_tbl;
}

static const uint16 *
wlc_phy_get_txgain_tbl_20696(phy_info_t *pi)
{
	const uint16 *tx_pwrctrl_tbl = NULL;

	ASSERT((RADIOID_IS(pi->pubpi->radioid, BCM20696_ID)));

	tx_pwrctrl_tbl = CHSPEC_IS2G(pi->radio_chanspec) ? acphy28nm_txgain_epa_2g_p5_20696a0_rev0
						: acphy28nm_txgain_epa_5g_p5_20696a0_rev0;

	return tx_pwrctrl_tbl;
}

static const uint16 *
wlc_phy_get_txgain_tbl_20698(phy_info_t *pi)
{
	const uint16 *tx_pwrctrl_tbl = NULL;

	ASSERT((RADIOID_IS(pi->pubpi->radioid, BCM20698_ID)));

	/* Choose the right table to use */
	switch (RADIOREV(pi->pubpi->radiorev)) {
	case 0:
	case 1: /* These revs use same tx gain table */
		tx_pwrctrl_tbl = CHSPEC_IS2G(pi->radio_chanspec) ?
			acphy28nm_txgain_epa_2g_p5_20698a0_rev0:
			acphy28nm_txgain_epa_5g_p5_20698a0_rev0;
		break;
	case 2:
		tx_pwrctrl_tbl = CHSPEC_IS2G(pi->radio_chanspec) ?
			acphy28nm_txgain_epa_2g_p5_20698a0_rev0:
			acphy28nm_txgain_epa_5g_p5_20698b0_rev2;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unsupported radio revision %d\n",
			pi->sh->unit, __FUNCTION__, RADIOREV(pi->pubpi->radiorev)));
		ASSERT(FALSE);
	}

	return tx_pwrctrl_tbl;
}

static const uint16 *
wlc_phy_get_txgain_tbl_20704(phy_info_t *pi)
{
	const uint16 *tx_pwrctrl_tbl = NULL;

	ASSERT((RADIOID_IS(pi->pubpi->radioid, BCM20704_ID)));

	/* Choose the right table to use */
	switch (RADIOREV(pi->pubpi->radiorev)) {
	case 0:
		tx_pwrctrl_tbl = CHSPEC_IS2G(pi->radio_chanspec) ?
			acphy28nm_txgain_epa_2g_p5_20704a0_rev0:
			acphy28nm_txgain_epa_5g_p5_20704a0_rev0;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unsupported radio revision %d\n",
			pi->sh->unit, __FUNCTION__, RADIOREV(pi->pubpi->radiorev)));
		ASSERT(FALSE);
	}

	return tx_pwrctrl_tbl;
}

static const uint16 *
wlc_phy_get_txgain_tbl_20697(phy_info_t *pi)
{
	const uint16 *tx_pwrctrl_tbl = NULL;
	uint16 idx, dac_attn, mix_gm_gc;

	ASSERT((RADIOID_IS(pi->pubpi->radioid, BCM20697_ID)));

	if (pi->pubpi->slice == DUALMAC_AUX) {
		switch (RADIOREV_AUX(pi->pubpi->radiorev)) {
			case 7:
				tx_pwrctrl_tbl = acphy28nm_txgain_aux_epa_2g_p5_20697a0_rev7;
				break;
			case 9:
				tx_pwrctrl_tbl = acphy28nm_txgain_aux_ipa_2g_p5_20697a0_rev9;
				break;
			default:
				tx_pwrctrl_tbl = acphy28nm_txgain_aux_epa_2g_p5_20697a0_rev7;
				break;
		}
	} else {
		switch (RADIOREV(pi->pubpi->radiorev)) {
			case 6:
				tx_pwrctrl_tbl = acphy28nm_txgain_main_epa_5g_p5_20697a0_rev6;
				break;
			default:
				tx_pwrctrl_tbl = acphy28nm_txgain_main_epa_5g_p5_20697a0_rev6;
		}
	}

	/* Sanitizing IPA gain table entries */
	if (tx_pwrctrl_tbl != NULL && PHY_IPA(pi)) {
		for (idx = 0; idx < 128; idx++) {
			dac_attn = ((tx_pwrctrl_tbl[3*idx] >> 8) & 0xf);
			mix_gm_gc = ((tx_pwrctrl_tbl[3*idx+2] >> 4) & 0xf);
			if ((dac_attn != IPA_20697_DAC_ATTN_VAL) &&
				(mix_gm_gc > IPA_20697_MAX_MIX_GM_GC_VAL)) {
				PHY_ERROR(("%s: Invalid dac_attn(%d) or mx_gm_gc(%d) value\n",
					__FUNCTION__, dac_attn, mix_gm_gc));
				ASSERT(0);
			}
		}
	}
	return tx_pwrctrl_tbl;
}

static const uint16 *
wlc_phy_get_tiny_txgain_tbl(phy_info_t *pi)
{
	phy_ac_tbl_info_t *tbli = pi->u.pi_acphy->tbli;
	const uint16 *tx_pwrctrl_tbl = NULL;

	ASSERT((RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) ||
		(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)));

	tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ? tbli->gaintbl_2g
						: tbli->gaintbl_5g;

	return tx_pwrctrl_tbl;
}

void
wlc_phy_ac_gains_load(phy_ac_tbl_info_t *tbli)
{
	uint idx;
	const uint16 *tx_pwrctrl_tbl = NULL;
	uint16 GainWord_Tbl[3];
	uint8 Gainoverwrite = 0;
	uint8 PAgain = 0xff;
	phy_info_t *pi = tbli->pi;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	bool suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

	if (NORADIO_ENAB(pi->pubpi))
		return;

	/* Setting tx gain table resolution */
	if (ROUTER_4349(pi)) {
		tbli->data->is_p25TxGainTbl = TRUE;
	}

	/* Load tx gain table */
	if (RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) {
		tx_pwrctrl_tbl = wlc_phy_get_tx_pwrctrl_tbl_2069(pi);
	} else if ((RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) ||
		(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID))) {
		tx_pwrctrl_tbl = wlc_phy_get_tiny_txgain_tbl(pi);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20694_ID)) {
		tx_pwrctrl_tbl = wlc_phy_get_txgain_tbl_20694(pi);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20695_ID)) {
		tx_pwrctrl_tbl = wlc_phy_get_txgain_tbl_20695(pi);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20696_ID)) {
		tx_pwrctrl_tbl = wlc_phy_get_txgain_tbl_20696(pi);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20697_ID)) {
		tx_pwrctrl_tbl = wlc_phy_get_txgain_tbl_20697(pi);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20698_ID)) {
		tx_pwrctrl_tbl = wlc_phy_get_txgain_tbl_20698(pi);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20704_ID)) {
		tx_pwrctrl_tbl = wlc_phy_get_txgain_tbl_20704(pi);
	} else {
		PHY_ERROR(("%s: Unsupported Radio!: 0x%x\n", __FUNCTION__,
			RADIOID(pi->pubpi->radioid)));
		ASSERT(0);
		return;
	}

	ASSERT(tx_pwrctrl_tbl != NULL);

	/* suspend mac */
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (PHY_PAPDEN(pi)) {
		phy_ac_papdcal_get_gainparams(pi_ac->papdcali, &Gainoverwrite, &PAgain);
	}

	if (Gainoverwrite > 0) {
		for (idx = 0; idx < 128; idx++) {
			GainWord_Tbl[0] = tx_pwrctrl_tbl[3*idx];
			GainWord_Tbl[1] = (tx_pwrctrl_tbl[3*idx+1] & 0xff00) + (0xff & PAgain);

			GainWord_Tbl[2] = tx_pwrctrl_tbl[3*idx+2];
			wlc_phy_tx_gain_table_write_acphy(tbli,
				1, idx, 48, GainWord_Tbl);
		}
	} else {
		wlc_phy_tx_gain_table_write_acphy(tbli,
			128, 0, 48, tx_pwrctrl_tbl);
	}

	if (TINY_RADIO(pi) && (phy_ac_calmgr_get_enIndxCap(pi_ac->calmgri))) {
		uint16 k, txgaintemp[3];
		uint16 txidxcap = 0;
		BCM_REFERENCE(txidxcap);
#if defined(WLC_TXCAL)
		if (!phy_tssical_get_olpc_idx_valid(pi->tssicali)) {
#endif /* WLC_TXCAL */
			if (CHSPEC_IS2G(pi->radio_chanspec) &&
				BF3_2GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
					txidxcap = pi->sromi->txidxcap2g & 0xFF;
			}
			if (CHSPEC_IS5G(pi->radio_chanspec) &&
				BF3_5GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
					txidxcap = pi->sromi->txidxcap5g & 0xFF;
			}
			if (txidxcap != 0) {
				txgaintemp[0] = tx_pwrctrl_tbl[3*txidxcap];
				txgaintemp[1] = tx_pwrctrl_tbl[3*txidxcap+1];
				txgaintemp[2] = tx_pwrctrl_tbl[3*txidxcap+2];

				for (k = 0; k < txidxcap; k++) {
					wlc_phy_tx_gain_table_write_acphy(tbli,
					1, k, 48, txgaintemp);
				}
			}
#if defined(WLC_TXCAL)
	}
#endif /* WLC_TXCAL */

#if defined(WLC_TXCAL)
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			if (phy_tssical_get_olpc_idx_valid_in_use(pi->tssicali)) {
				uint8 core = 0, iidx;
				int8 max_fem_out;
				uint8 band = (CHSPEC_IS2G(pi->radio_chanspec) ? 0 : 1);
				/* save the original tx_power_max_per_core */
				int8 tx_power_max_per_core_orig = pi->tx_power_max_per_core[core];

				/* Gain Table Capping on the lower side */
				pi->tx_power_max_per_core[core] = pi->sromi->txidxcaplow[band];
				wlc_phy_olpc_idx_tempsense_comp_acphy(pi, &iidx, core);
				txgaintemp[0] = tx_pwrctrl_tbl[3*iidx];
				txgaintemp[1] = tx_pwrctrl_tbl[3*iidx+1];
				txgaintemp[2] = tx_pwrctrl_tbl[3*iidx+2];
				for (k = iidx + 1; k < 128; k++) {
					wlc_phy_tx_gain_table_write_acphy(tbli,
					1, k, 48, txgaintemp);
				}

				/* Gain Table Capping on the higher side */
				if (pi->sromi->maxepagain[band] != 0) {
					max_fem_out = pi->sromi->maxchipoutpower[band]
						+ pi->sromi->maxepagain[band];
					pi->tx_power_max_per_core[core] = max_fem_out;
					wlc_phy_olpc_idx_tempsense_comp_acphy(pi, &iidx, core);
					txgaintemp[0] = tx_pwrctrl_tbl[3*iidx];
					txgaintemp[1] = tx_pwrctrl_tbl[3*iidx+1];
					txgaintemp[2] = tx_pwrctrl_tbl[3*iidx+2];
					for (k = 0; k < iidx; k++) {
						wlc_phy_tx_gain_table_write_acphy(tbli,
						1, k, 48, txgaintemp);
					}
				}
				/* restore the original tx_power_max_per_core */
				pi->tx_power_max_per_core[core] = tx_power_max_per_core_orig;
			}
		}
#endif /* WLC_TXCAL */

	}

	/* resume mac */
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_phy_tx_gain_table_write_acphy(phy_ac_tbl_info_t *tbli, uint32 l, uint32 o,
	uint32 w, const void *d)
{
	uint8 core, tx_gain_tbl_id;
	phy_info_t *pi = tbli->pi;
	FOREACH_CORE(pi, core) {
		tx_gain_tbl_id = wlc_phy_get_tbl_id_gainctrlbbmultluts(pi, core);
		if ((!(ACMAJORREV_4(pi->pubpi->phy_rev) ||
			ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev))) && (core != 0) &&
			!(ACMAJORREV_5(pi->pubpi->phy_rev) && ACMINORREV_2(pi))) {
			continue;
		}
		wlc_phy_table_write_acphy(pi, tx_gain_tbl_id, l, o, w, d);
	}

}

#if defined(BCMDBG)
static int
phy_ac_tbl_dump_txv0(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ac_tbl_info_t *tbli = (phy_ac_tbl_info_t *)ctx;
	phy_info_t *pi = tbli->pi;
	uint16 k;
	uint32 tbl_val;
	uint8 stall_val;

	phy_utils_phyreg_enter(pi);

	/* disable stall */
	stall_val = (phy_utils_read_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev)) &
	             ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev))
	        >> ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev);
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
	            ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
	            1 << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));

	/* id=16, depth=1952 words for usr 0, width=32 */
	for (k = 0; k < 1952; k++) {
		wlc_phy_table_read_acphy(pi, 16, 1, k, 32, &tbl_val);
		bcm_bprintf(b, "0x%08x\n", tbl_val);
	}

	/* restore stall value */
	phy_utils_mod_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev),
	            ACPHY_RxFeCtrl1_disable_stalls_MASK(pi->pubpi->phy_rev),
	            stall_val << ACPHY_RxFeCtrl1_disable_stalls_SHIFT(pi->pubpi->phy_rev));

	phy_utils_phyreg_exit(pi);

	return BCME_OK;
}
/* 43684BU, dump function used for UCODE team to dump table on PSM watchdog assert */
static int
phy_ac_tbl_dump_axmacphyif(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ac_tbl_info_t *tbli = (phy_ac_tbl_info_t *)ctx;
	phy_info_t *pi = tbli->pi;

	return phy_ac_dump_axmacphyif(pi, b, TRUE);
}

int
phy_ac_dump_axmacphyif(phy_info_t *pi, struct bcmstrbuf *b, bool suspend)
{
	acphytbl_info_t tbl;
	uint8 stall_val = 0, mac_suspend = 1;
	uint32 data[2];
	int i;

	if (suspend) {
		mac_suspend = (R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
		if (mac_suspend) {
			/* MAC suspend if MAC is enabled */
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

		if (stall_val == 0)
			ACPHY_DISABLE_STALL(pi);

		ASSERT(READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls) == 1);
	}

	tbl.tbl_id = 229;	/* axmacphyIfTbl */
	tbl.tbl_len = 1;
	tbl.tbl_width = 64;
	tbl.tbl_ptr = data;

	for (i = 0; i < 64; i++) {
		tbl.tbl_offset = i;
		wlc_phy_table_read_ext_acphy(pi, &tbl);
		bcm_bprintf(b, "%2d: %08x %08x\n", i, data[0], data[1]);
	}

	if (suspend) {
		if (stall_val == 0)
			ACPHY_ENABLE_STALL(pi, stall_val);

		if (mac_suspend) {
			/* Resume suspended MAC */
			wlapi_enable_mac(pi->sh->physhim);
		}
	}

	return BCME_OK;
}
#endif // endif
