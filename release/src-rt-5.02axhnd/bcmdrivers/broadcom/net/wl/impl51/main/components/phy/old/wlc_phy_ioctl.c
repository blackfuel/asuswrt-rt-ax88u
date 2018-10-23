/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abg
 * PHY ioctl processing of Broadcom BCM43XX 802.11abg
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
 * $Id: wlc_phy_ioctl.c 689610 2017-03-12 13:11:51Z $
 */

/*
 * This file contains high portion PHY ioctl processing and table.
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <wlioctl.h>

#include <wlc_iocv_types.h>
#include <wlc_iocv_desc.h>
#include <wlc_iocv_reg.h>

#include <phy_utils_reg.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_n.h>
#include <wlc_phy_lcn20.h>
#include <phy_noise_api.h>
#include <phy_misc_api.h>

static const wlc_ioctl_cmd_t phy_ioctls[] = {
	{WLC_RESTART, 0, 0},
#if defined(BCMDBG) || defined(WLTEST) || defined(ATE_BUILD)
	{WLC_GET_RADIOREG, WLC_IOCF_REG_CHECK, 0},
	{WLC_SET_RADIOREG, WLC_IOCF_REG_CHECK, 0},
#endif // endif
#if defined(BCMDBG)
	{WLC_GET_TX_PATH_PWR, WLC_IOCF_BAND_CHECK_AUTO, 0},
	{WLC_SET_TX_PATH_PWR, WLC_IOCF_BAND_CHECK_AUTO, 0},
#endif // endif
#if defined(BCMDBG) || defined(WLTEST) || defined(WL_EXPORT_GET_PHYREG) || \
	defined(ATE_BUILD)
	{WLC_GET_PHYREG, WLC_IOCF_REG_CHECK, 0},
#endif // endif
#if defined(BCMDBG) || defined(WLTEST) || defined(ATE_BUILD)
	{WLC_SET_PHYREG, WLC_IOCF_REG_CHECK, 0},
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
	{WLC_GET_TSSI, WLC_IOCF_REG_CHECK_AUTO, 0},
	{WLC_GET_ATTEN, WLC_IOCF_REG_CHECK_AUTO, 0},
	{WLC_SET_ATTEN, WLC_IOCF_BAND_CHECK_AUTO, 0},
	{WLC_GET_PWRIDX, WLC_IOCF_BAND_CHECK_AUTO, 0},
	{WLC_SET_PWRIDX, WLC_IOCF_BAND_CHECK_AUTO, 0},
	{WLC_LONGTRAIN, WLC_IOCF_REG_CHECK_AUTO, 0},
	{WLC_EVM, WLC_IOCF_REG_CHECK, 0},
	{WLC_FREQ_ACCURACY, WLC_IOCF_REG_CHECK, 0},
	{WLC_CARRIER_SUPPRESS, WLC_IOCF_REG_CHECK_AUTO, 0},
#endif /* BCMDBG || WLTEST  */
	{WLC_GET_INTERFERENCE_MODE, 0, 0},
	{WLC_SET_INTERFERENCE_MODE, 0, 0},
	{WLC_GET_INTERFERENCE_OVERRIDE_MODE, 0, 0},
	{WLC_SET_INTERFERENCE_OVERRIDE_MODE, 0, 0},
};

#include <wlc_phy_shim.h>
#include <wlc_phy_int.h>

static int
phy_legacy_doioctl(void *ctx, uint32 cmd, void *arg, uint len, struct wlc_if *wlcif)
{
	phy_info_t *pi = ctx;
	bool ta = FALSE;
	int err = wlc_phy_ioctl_dispatch(pi, (int)cmd, (int)len, arg, &ta);
	wlapi_taclear(pi->sh->physhim, ta);
	return err;
}

/* register ioctl table to the system */
int phy_legacy_register_ioct(phy_info_t *pi, wlc_iocv_info_t *ii);

int
BCMATTACHFN(phy_legacy_register_ioct)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_ioct_desc_t iocd;

	ASSERT(ii != NULL);

	wlc_iocv_init_iocd(phy_ioctls, ARRAYSIZE(phy_ioctls),
	                   NULL,
	                   phy_legacy_cmd_proc, phy_legacy_result_proc,
	                   phy_legacy_doioctl, pi,
	                   &iocd);

	return wlc_iocv_register_ioct(ii, &iocd);
}

/* %%%%%% IOCTL */
int
wlc_phy_ioctl_dispatch(phy_info_t *pi, int cmd, int len, void *arg, bool *ta_ok)
{
	wlc_phy_t *pih = (wlc_phy_t *)pi;
	int bcmerror = 0;
	int val, *pval;
	bool bool_val;
	uint8 max_aci_mode;
	bool suspend;

	UNUSED_PARAMETER(suspend);
	(void)pih;

	/* default argument is generic integer */
	pval = (int*)arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	else
		val = 0;

	/* bool conversion to avoid duplication below */
	bool_val = (val != 0);
	BCM_REFERENCE(bool_val);

	switch (cmd) {
	case WLC_RESTART:
		break;
	default:
		if ((arg == NULL) || (len <= 0)) {
			PHY_ERROR(("wl%d: %s: Command %d needs arguments\n",
			          pi->sh->unit, __FUNCTION__, cmd));
			return BCME_BADARG;
		}
		break;
	}

	switch (cmd) {

	case WLC_GET_PHY_NOISE:
		ASSERT(pval != NULL);
		*pval = phy_noise_avg(pih);
		break;

	case WLC_RESTART:
		/* Reset calibration results to uninitialized state in order to
		 * trigger recalibration next time wlc_init() is called.
		 */
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
		phy_type_reset_impl(pi->typei);
		break;

#if defined(BCMDBG)|| defined(WLTEST) || defined(DBG_PHY_IOV) || defined(ATE_BUILD)
	case WLC_GET_RADIOREG:
		/* Suspend MAC if haven't done so */
#if (ACCONF != 0) || (ACCONF2 != 0)
		wlc_phy_conditional_suspend(pi, &suspend);
#endif // endif
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}
		ASSERT(pval != NULL);

		phy_utils_phyreg_enter(pi);
		phy_utils_radioreg_enter(pi);
		if (val == RADIO_IDCODE)
			*pval = phy_radio_query_idcode(pi->radioi);
		else
			*pval = phy_utils_read_radioreg(pi, (uint16)val);
		phy_utils_radioreg_exit(pi);
		phy_utils_phyreg_exit(pi);
#if (ACCONF != 0) || (ACCONF2 != 0)
		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);
#endif // endif
		break;

	case WLC_SET_RADIOREG:
#if (ACCONF != 0) || (ACCONF2 != 0)
		/* Suspend MAC if haven't done so */
		wlc_phy_conditional_suspend(pi, &suspend);
#endif // endif
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		phy_utils_phyreg_enter(pi);
		phy_utils_radioreg_enter(pi);
		phy_utils_write_radioreg(pi, (uint16)val, (uint16)(val >> NBITS(uint16)));
		phy_utils_radioreg_exit(pi);
		phy_utils_phyreg_exit(pi);
#if (ACCONF != 0) || (ACCONF2 != 0)
		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);
#endif // endif
		break;
#endif // endif

#if defined(BCMDBG) || defined(WLTEST) || defined(DBG_PHY_IOV) || \
	defined(WL_EXPORT_GET_PHYREG) || defined(ATE_BUILD)
	case WLC_GET_PHYREG:
#if (ACCONF != 0) || (ACCONF2 != 0)
		/* Suspend MAC if haven't done so */
		wlc_phy_conditional_suspend(pi, &suspend);
#endif // endif
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		phy_utils_phyreg_enter(pi);

		ASSERT(pval != NULL);
		*pval = phy_utils_read_phyreg(pi, (uint16)val);

		phy_utils_phyreg_exit(pi);
#if (ACCONF != 0) || (ACCONF2 != 0)
		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);
#endif // endif
		break;
#endif // endif

#if defined(BCMDBG) || defined(WLTEST) || defined(DBG_PHY_IOV) || defined(ATE_BUILD)
	case WLC_SET_PHYREG:
#if (ACCONF != 0) || (ACCONF2 != 0)
		/* Suspend MAC if haven't done so */
		wlc_phy_conditional_suspend(pi, &suspend);
#endif // endif
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		phy_utils_phyreg_enter(pi);
		phy_utils_write_phyreg(pi, (uint16)val, (uint16)(val >> NBITS(uint16)));
		phy_utils_phyreg_exit(pi);
#if (ACCONF != 0) || (ACCONF2 != 0)
		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);
#endif // endif
		break;
#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_TSSI: {

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		ASSERT(pval != NULL);
		*pval = 0;
		switch (pi->pubpi->phy_type) {
		case PHY_TYPE_LCN20:
			PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
			CASECHECK(PHYTYPE, PHY_TYPE_LCN20);
			{
				int8 ofdm_pwr = 0, cck_pwr = 0;
#if (defined(LCN20CONF) && (LCN20CONF != 0))
				wlc_lcn20phy_get_tssi(pi, &ofdm_pwr, &cck_pwr);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
				*pval =  ((uint16)ofdm_pwr << 8) | (uint16)cck_pwr;
				break;
			}
		case PHY_TYPE_N:
			CASECHECK(PHYTYPE, PHY_TYPE_N);
			{
			*pval = (phy_utils_read_phyreg(pi, NPHY_TSSIBiasVal1) & 0xff) << 8;
			*pval |= (phy_utils_read_phyreg(pi, NPHY_TSSIBiasVal2) & 0xff);
			break;
			}
		}

		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;
	}

	case WLC_GET_ATTEN: {
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

	case WLC_SET_ATTEN: {
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

	case WLC_GET_PWRIDX:
		bcmerror = BCME_UNSUPPORTED;
		break;

	case WLC_SET_PWRIDX:	/* set A band radio/baseband power index */
		bcmerror = BCME_UNSUPPORTED;
		break;

	case WLC_LONGTRAIN:
		{
		longtrnfn_t long_train_fn = NULL;

		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		long_train_fn = pi->pi_fptr->longtrn;
		if (long_train_fn)
			bcmerror = (*long_train_fn)(pi, val);
		else
			PHY_ERROR(("WLC_LONGTRAIN: unsupported phy type\n"));

			break;
		}

	case WLC_EVM:
		ASSERT(arg != NULL);
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		bcmerror = phy_dbg_test_evm(pi, val, *(((uint *)arg) + 1), *(((int *)arg) + 2));
		break;

	case WLC_FREQ_ACCURACY:
/* This if condition is already present in BISON6T branch
 * needed for 'wl fqacurcy' IOVAR to work on 43430
 */
#if !SSLPNCONF && !LCN20CONF
		/* SSLPNCONF transmits a few frames before running PAPD Calibration
		 * it does papd calibration each time it enters a new channel
		 * We cannot be down for this reason
		 */
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
#endif // endif
		bcmerror = wlc_phy_test_freq_accuracy(pi, val);
		break;

	case WLC_CARRIER_SUPPRESS:
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
		bcmerror = phy_dbg_test_carrier_suppress(pi, val);
		break;
#endif /* BCMDBG || WLTEST  */

#ifndef WLC_DISABLE_ACI
#if defined(WLTEST)
	case WLC_GET_ACI_ARGS:
		ASSERT(arg != NULL);
		bcmerror = phy_noise_aci_args(pi, arg, TRUE, len);
		break;

	case WLC_SET_ACI_ARGS:
		ASSERT(arg != NULL);
		bcmerror = phy_noise_aci_args(pi, arg, FALSE, len);
		break;

#endif // endif
#endif /* Compiling out ACI code */

	case WLC_GET_INTERFERENCE_MODE:
		ASSERT(pval != NULL);
		*pval = pi->sh->interference_mode;
		if (pi->aci_state & ACI_ACTIVE) {
			*pval |= AUTO_ACTIVE;
			*pval |= (pi->aci_active_pwr_level << 4);
		}
		break;

	case WLC_SET_INTERFERENCE_MODE:
		max_aci_mode = ISACPHY(pi) ? ACPHY_ACI_MAX_MODE : WLAN_AUTO_W_NOISE;
		if (val < INTERFERE_NONE || val > max_aci_mode) {
			bcmerror = BCME_RANGE;
			break;
		}

		if (pi->sh->interference_mode == val)
			break;

		/* push to sw state */
		pi->sh->interference_mode = val;

		if (!pi->sh->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifndef WLC_DISABLE_ACI
		/* turn interference mode to off before entering another mode */
		if (val != INTERFERE_NONE)
			phy_noise_set_mode(pi->noisei, INTERFERE_NONE, TRUE);

#if defined(RXDESENS_EN)
		if (ISNPHY(pi))
			wlc_nphy_set_rxdesens((wlc_phy_t *)pi, 0);
#endif // endif
		if (phy_noise_set_mode(pi->noisei, pi->sh->interference_mode, TRUE) != BCME_OK)
			bcmerror = BCME_BADOPTION;
#endif /* !defined(WLC_DISABLE_ACI) */

		wlapi_enable_mac(pi->sh->physhim);
		break;

	case WLC_GET_INTERFERENCE_OVERRIDE_MODE:
		if (!(ISLCN20PHY(pi) || ISACPHY(pi) || ISNPHY(pi))) {
			break;
		}

		ASSERT(pval != NULL);
		if (pi->sh->interference_mode_override == FALSE) {
			*pval = INTERFERE_OVRRIDE_OFF;
		} else {
			*pval = pi->sh->interference_mode;
		}
		break;

	case WLC_SET_INTERFERENCE_OVERRIDE_MODE:
		max_aci_mode = ISACPHY(pi) ? ACPHY_ACI_MAX_MODE : WLAN_AUTO_W_NOISE;
		if (!(ISLCN20PHY(pi) || ISACPHY(pi) || ISNPHY(pi))) {
			break;
		}

		if (val < INTERFERE_OVRRIDE_OFF || val > max_aci_mode) {
			bcmerror = BCME_RANGE;
			break;
		}

		bcmerror = wlc_phy_set_interference_override_mode(pi, val);

		break;

	default:
		bcmerror = BCME_UNSUPPORTED;
	}

	return bcmerror;
}
