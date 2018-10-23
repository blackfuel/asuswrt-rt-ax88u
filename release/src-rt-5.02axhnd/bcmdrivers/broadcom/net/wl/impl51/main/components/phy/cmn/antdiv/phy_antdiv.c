/*
 * ANTennaDIVersity module implementation.
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
 * $Id: phy_antdiv.c 671526 2016-11-22 08:37:30Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_antdiv.h"
#include <phy_antdiv_cfg.h>
#include <phy_antdiv_api.h>
#include <phy_antdiv.h>
#include <phy_rstr.h>
#include <phy_utils_var.h>

/* module private states */
struct phy_antdiv_info {
	phy_info_t *pi;
	phy_type_antdiv_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_antdiv_info_t info;
	phy_type_antdiv_fns_t fns;
/* add other variable size variables here at the end */
} phy_antdiv_mem_t;

/* local function declaration */
static int phy_antdiv_init(phy_init_ctx_t *ctx);

/* attach/detach */
phy_antdiv_info_t *
BCMATTACHFN(phy_antdiv_attach)(phy_info_t *pi)
{
	phy_antdiv_info_t *info;
	phy_type_antdiv_fns_t *fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_antdiv_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	/* init the type specific fns */
	fns = &((phy_antdiv_mem_t *)info)->fns;
	info->fns = fns;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_antdiv_init, info, PHY_INIT_ANTDIV) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register callbacks */

	return info;

	/* error */
fail:
	phy_antdiv_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_antdiv_detach)(phy_antdiv_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null antdiv module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_antdiv_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_antdiv_register_impl)(phy_antdiv_info_t *di, phy_type_antdiv_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*di->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_antdiv_unregister_impl)(phy_antdiv_info_t *di)
{
	BCM_REFERENCE(di);
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* initialization */
static int
WLBANDINITFN(phy_antdiv_init)(phy_init_ctx_t *ctx)
{
	phy_antdiv_info_t *di = (phy_antdiv_info_t *)ctx;
#ifdef WLC_SW_DIVERSITY
	phy_type_antdiv_fns_t *fns = di->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->initswdiv == NULL)
		return BCME_OK;

	(fns->initswdiv)(fns->ctx);
#else
	phy_info_t *pi = di->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_antdiv_set_rx(pi, pi->sh->rx_antdiv);
#endif // endif
	return BCME_OK;
}

/* set/get antenna diversity mask */
int
phy_antdiv_set_rx(phy_info_t *pi, uint8 ant)
{
	phy_antdiv_info_t *di = pi->antdivi;
	phy_type_antdiv_fns_t *fns;
	bool suspend;

	PHY_TRACE(("%s: ant 0x%x\n", __FUNCTION__, ant));

	ASSERT(di != NULL);

	fns = di->fns;

	if (fns->setrx == NULL)
		return BCME_OK;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	(fns->setrx)(fns->ctx, ant);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}

void
phy_antdiv_get_rx(phy_info_t *pi, uint8 *ant)
{
	*ant = pi->sh->rx_antdiv;
}

void
phy_antdiv_set_sw_control(phy_info_t *pi, int8 divOvrride, int core)
{
	phy_antdiv_info_t *di = pi->antdivi;
	phy_type_antdiv_fns_t *fns = di->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->set_sw_control != NULL) {
		(fns->set_sw_control)(fns->ctx, divOvrride, core);
	} else {
		PHY_ERROR(("IOVAR is not supported for this chip \n"));
	}
}

void
phy_antdiv_get_sw_control(phy_info_t *pi, int32 *ret_int_ptr, int core)
{
	phy_antdiv_info_t *di = pi->antdivi;
	phy_type_antdiv_fns_t *fns = di->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->get_sw_control != NULL) {
		(fns->get_sw_control)(fns->ctx, ret_int_ptr, core);
	} else {
		PHY_ERROR(("IOVAR is not supported for this chip \n"));
	}
}

int phy_antdiv_set_txswctrlmap(phy_info_t *pi, int32 int_val)
{
	phy_type_antdiv_fns_t *fns;
	int err = BCME_OK;

	ASSERT(pi != NULL);

	fns = pi->antdivi->fns;

	if (fns->set_txswctrlmap != NULL) {
		err = (fns->set_txswctrlmap)(fns->ctx, int_val);
	} else {
		/* Not implemented for this phy. */
		err = BCME_UNSUPPORTED;
		PHY_ERROR(("Command not supported for this phy\n"));
	}

	return err;
}

int phy_antdiv_get_txswctrlmap(phy_info_t *pi, int32 *ret_int_ptr)
{
	phy_type_antdiv_fns_t *fns;
	int err = BCME_OK;

	ASSERT(pi != NULL);

	fns = pi->antdivi->fns;

	if (fns->get_txswctrlmap != NULL) {
		err = (fns->get_txswctrlmap)(fns->ctx, ret_int_ptr);
	} else {
		/* Not implemented for this phy. */
		err = BCME_UNSUPPORTED;
		PHY_ERROR(("Command not supported for this phy\n"));
	}

	return err;
}

#ifdef WLC_SW_DIVERSITY
void
phy_antdiv_set_swdiv_ant(wlc_phy_t *ppi, uint8 ant)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_antdiv_info_t *di = pi->antdivi;
	phy_type_antdiv_fns_t *fns = di->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!pi->sh->clk)
		return;

	if (fns->setswdiv == NULL)
		return;

	(fns->setswdiv)(fns->ctx, ant);
}

uint8
phy_antdiv_get_swdiv_ant(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_antdiv_info_t *di = pi->antdivi;
	phy_type_antdiv_fns_t *fns = di->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!pi->sh->clk)
		return 0;

	if (fns->getswdiv == NULL)
		return 0;

	return (fns->getswdiv)(fns->ctx);
}

void
BCMATTACHFN(phy_swdiv_read_srom)(phy_info_t *pi, phy_swdiv_t *swdiv)
{
	/* Note: following swdiv parameters are also read and stored independently in wlc.c */
	/*       (not intended to be used as dynamic parameters) */
	swdiv->swdiv_enable      = (bool)PHY_GETINTVAR(pi, rstr_swdiv_en);
	swdiv->swdiv_gpio_num    = (uint8)PHY_GETINTVAR(pi, rstr_swdiv_gpio);
	swdiv->swdiv_gpio_ctrl = (uint8)PHY_GETINTVAR(pi, rstr_swdiv_gpioctrl);
	swdiv->swdiv_swctrl_en   = (wlc_swdiv_swctrl_t)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_en);
	swdiv->swdiv_swctrl_mask = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_mask);
	swdiv->swdiv_swctrl_ant0 = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_ant0);
	swdiv->swdiv_swctrl_ant1 = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_swctrl_ant1);
	swdiv->swdiv_coreband_map = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_coreband_map);
	swdiv->swdiv_antmap2g_main = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_antmap2g_main);
	swdiv->swdiv_antmap5g_main = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_antmap5g_main);
	swdiv->swdiv_antmap2g_aux = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_antmap2g_aux);
	swdiv->swdiv_antmap5g_aux = (uint16)PHY_GETINTVAR(pi, rstr_swdiv_antmap5g_aux);
}
#endif /* WLC_SW_DIVERSITY */

int
phy_antdiv_antsel_type_set(wlc_phy_t *ppi, uint8 antsel_type)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->antsel_type = antsel_type;

	/* initialize flag to init HW Rx antsel if the board supports it */
	if ((pi->antsel_type == ANTSEL_2x3_HWRX) || (pi->antsel_type == ANTSEL_1x2_HWRX))
		pi->nphy_enable_hw_antsel = TRUE;
	else
		pi->nphy_enable_hw_antsel = FALSE;

	return BCME_OK;
}
