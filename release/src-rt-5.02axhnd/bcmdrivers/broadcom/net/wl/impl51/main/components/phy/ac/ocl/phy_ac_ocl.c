/*
 * ACPHY One Core Listen (OCL) module implementation
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
 * $Id: $
 */
#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_ocl.h>
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <phy_ac_ocl.h>
#include <wlc_radioreg_20693.h>
#include <phy_rstr.h>
#include <phy_utils_var.h>
#include <wlioctl.h>
#include <phy_stf.h>

/* module private states */
struct phy_ac_ocl_info {
	phy_info_t *pi;
	phy_ac_info_t *pi_ac;
	phy_ocl_info_t *ocl_info;

	phy_ac_ocl_data_t	*data; /* shared data */
	uint8 ocl_en;
};

typedef struct {
	phy_ac_ocl_info_t info;
	phy_ac_ocl_data_t data;
} phy_ac_ocl_mem_t;

#ifdef OCL
static uint16 *get_tiny_4349_ocl_reset2rx_cmd(void);
static uint16 *get_tiny_4349_ocl_reset2rx_dly(void);
static uint16 *get_tiny_4349_ocl_shutoff_cmd(void);
static uint16 *get_tiny_4349_ocl_shutoff_dly(void);
static uint16 *get_tiny_4349_ocl_tx2rx_cmd(void);
static uint16 *get_tiny_4349_ocl_tx2rx_dly(void);
static uint16 *get_tiny_4349_ocl_rx2tx_tssi_sleep_cmd(void);
static uint16 *get_tiny_4349_ocl_rx2tx_tssi_sleep_dly(void);
static uint16 *get_tiny_4349_ocl_wakeoncrs_cmd(void);
static uint16 *get_tiny_4349_ocl_wakeoncrs_dly(void);
static uint16 *get_tiny_4349_ocl_wakeoncrs_cck_cmd(void);
static uint16 *get_tiny_4349_ocl_wakeoncrs_cck_dly(void);
static uint16 *get_tiny_4349_ocl_wakeonclip_cmd(void);
static uint16 *get_tiny_4349_ocl_wakeonclip_dly(void);
static uint16 *BCMRAMFN(get_ocl_shutoff_cmd_rev40)(void);
static uint16 *BCMRAMFN(get_ocl_shutoff_dly_rev40)(void);
static uint16 *BCMRAMFN(get_ocl_wakeoncrs_cmd_rev44)(void);
static uint16 *BCMRAMFN(get_ocl_wakeoncrs_dly_rev44)(void);
static uint16 *BCMRAMFN(get_ocl_wakeonclip_cmd_rev44)(void);
static uint16 *BCMRAMFN(get_ocl_wakeonclip_dly_rev44)(void);
static uint16 *BCMRAMFN(get_ocl_shutoff_cmd_rev44)(void);
static uint16 *BCMRAMFN(get_ocl_shutoff_dly_rev44)(void);

/* ocl reset2rx */
uint16 tiny_4349_ocl_reset2rx_cmd[] =
       {0x84, 0x4, 0x3, 0x6, 0x12, 0x11, 0x10, 0x16, 0x2a, 0x2b, 0x24, 0x85,
	0x1f, 0x1f, 0x1f, 0x1f};
uint16 tiny_4349_ocl_reset2rx_dly[] =
       {0xa, 0xc, 0x2, 0x2, 0x4, 0x4, 0x6, 0x1, 0x4, 0x1, 0x2, 0xa, 0x1, 0x1, 0x1, 0x1};

/* oc shutoff */
uint16 tiny_4349_ocl_shutoff_cmd[] =
       {0x84, 0x25, 0x12, 0x22, 0x11, 0x10, 0x16, 0x24, 0x2a, 0x2b, 0x85,
	0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
uint16 tiny_4349_ocl_shutoff_dly[] =
       {0xa, 0x4, 0x4, 0x2, 0x4, 0x6, 0x1, 0x2, 0x4, 0x1, 0xa, 0x1, 0x1, 0x1, 0x1, 0x1};

/* ocl tx2rx */
uint16 tiny_4349_ocl_tx2rx_cmd[] =
	{0x84, 0x4, 0x3, 0x6, 0x12, 0x11, 0x10, 0x16, 0x2a, 0x24, 0x0, 0x24, 0x2b, 0x0, 0x85, 0x1F};

uint16 tiny_4349_ocl_tx2rx_dly[] =
	{0x8, 0x8, 0x4, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0x1, 0x2, 0x1, 0x40, 0x1, 0x1, 0x2, 0x1};
/* ocl rx2tx */
uint16 tiny_4349_ocl_rx2tx_tssi_sleep_cmd[] =
	{0, 0x1, 0x2, 0x8, 0x5, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x00, 0x00, 0x84, 0x36, 0x1f};
uint16 tiny_4349_ocl_rx2tx_tssi_sleep_dly[] =
	{0x1, 0x6, 0x6, 0x4, 0x4, 0x10, 0x26, 0x2, 0x5, 0x4, 0xFA, 0xFA, 0x88, 0xa, 0x1, 0x1};

/* ocl wake on crs */
uint16 tiny_4349_ocl_wakeoncrs_cmd[] =
	{0x84, 0x12, 0x23, 0x24, 0x85, 0x15, 0x16, 0x18, 0x19, 0x2a, 0x2b, 0x1f,
		0x1f, 0x1f, 0x1f, 0x1f};
uint16 tiny_4349_ocl_wakeoncrs_dly[] =
	{0x1, 0x4, 0x2, 0x2, 0x1, 0x6, 0x12, 0x8, 0x1, 0x4, 0x1, 0x1, 0x4, 0x1,  0x1, 0x1};

/* ocl wake on clip */
uint16 tiny_4349_ocl_wakeonclip_cmd[] =
	{0x84, 0x12, 0x15, 0x24, 0x85, 0x17, 0x00, 0x16, 0x2a, 0x2b, 0x1f, 0x1f,
		0x1f, 0x1f, 0x1f, 0x1f};
uint16 tiny_4349_ocl_wakeonclip_dly[] =
	{0x1, 0x4, 0x2, 0x1, 0x4, 0x2, 0x2, 0x2, 0x2, 0x4, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};

/* ocl wake on crs cck */
uint16 tiny_4349_ocl_wakeoncrs_cck_cmd[] =
	{0x84, 0x12, 0x23, 0x24, 0x85, 0x15, 0x16, 0x00, 0x19, 0x2a, 0x2b, 0x1f,
		0x1f, 0x1f, 0x1f, 0x1f};
uint16 tiny_4349_ocl_wakeoncrs_cck_dly[] =
	{0x1, 0x4, 0x2, 0x2, 0x1, 0x6, 0x12, 0x8, 0x1, 0x4, 0x1, 0x4, 0x1, 0x1, 0x1, 0x1};

uint16 const ocl_wakeoncrs_cmd_rev40[] = {0x12, 0x15, 0x16, 0x18, 0x00, 0x24, 0x1f};
uint16 const ocl_wakeoncrs_dly_rev40[] = {0x04, 0x16, 0x12, 0x08, 0x10, 0x02, 0x01};

uint16 const ocl_wakeonclip_cmd_rev40[] = {0x12, 0x15, 0x17, 0x00, 0x16, 0x24, 0x1f};
uint16 const ocl_wakeonclip_dly_rev40[] = {0x04, 0x02, 0x02, 0x02, 0x01, 0x02, 0x01};

uint16 ocl_shutoff_cmd_rev40[] = {0x25, 0x85, 0x12, 0x11, 0x10, 0x16, 0x24, 0x86, 0x1f};
uint16 ocl_shutoff_dly_rev40[] = {0x02, 0x02, 0x04, 0x04, 0x06, 0x01, 0x02, 0x02, 0x01};

uint16 ocl_tx2rx_w_far_reset_cmd_rev40[] =
        {0x85, 0x04, 0x03, 0x06, 0x3d, 0x12, 0x11, 0x10, 0x16, 0x2a, 0x2b, 0x24, 0x3e, 0x86, 0x1f};
uint16 ocl_tx2rx_w_far_reset_dly_rev40[] =
        {0x14, 0x08, 0x04, 0x02, 0x12, 0x02, 0x03, 0x04, 0x06, 0x04, 0x01, 0x0a, 0x22, 0x24, 0x01};

uint16 ocl_wakeoncrs_cmd_rev44[] = {0x12, 0x15, 0x16, 0x18, 0x00, 0x24, 0x1f};
uint16 ocl_wakeoncrs_dly_rev44[] = {0x04, 0x16, 0x12, 0x08, 0x10, 0x02, 0x01};

uint16 ocl_wakeonclip_cmd_rev44[] = {0x12, 0x15, 0x17, 0x00, 0x16, 0x24, 0x1f};
uint16 ocl_wakeonclip_dly_rev44[] = {0x04, 0x02, 0x02, 0x02, 0x01, 0x02, 0x01};

/* Dly increased to 0x20 for cmd 0x12 to avoid stalls in certain parts */
uint16 ocl_shutoff_cmd_rev44[] = {0x25, 0x85, 0x12, 0x11, 0x10, 0x16, 0x24, 0x86, 0x1f};
uint16 ocl_shutoff_dly_rev44[] = {0x02, 0x02, 0x20, 0x04, 0x06, 0x01, 0x02, 0x02, 0x01};

uint16 ocl_tx2rx_w_far_reset_cmd_rev44[] =
        {0x85, 0x04, 0x03, 0x06, 0x3d, 0x12, 0x11, 0x10, 0x16, 0x24, 0x3e, 0x86, 0x1f};
uint16 ocl_tx2rx_w_far_reset_dly_rev44[] =
        {0x24, 0x08, 0x04, 0x12, 0x24, 0x04, 0x04, 0x06, 0x01, 0x0a, 0x24, 0x24, 0x01};

static uint16 *BCMRAMFN(get_ocl_shutoff_cmd_rev40)(void)
{
	return ocl_shutoff_cmd_rev40;
}
static uint16 *BCMRAMFN(get_ocl_shutoff_dly_rev40)(void)
{
	return ocl_shutoff_dly_rev40;
}
static uint16 *BCMRAMFN(get_ocl_tx2rx_cmd_rev40)(void)
{
	return ocl_tx2rx_w_far_reset_cmd_rev40;
}
static uint16 *BCMRAMFN(get_ocl_tx2rx_dly_rev40)(void)
{
	return ocl_tx2rx_w_far_reset_dly_rev40;
}

static uint16 *BCMRAMFN(get_ocl_wakeoncrs_cmd_rev44)(void)
{
	return ocl_wakeoncrs_cmd_rev44;
}
static uint16 *BCMRAMFN(get_ocl_wakeoncrs_dly_rev44)(void)
{
	return ocl_wakeoncrs_dly_rev44;
}
static uint16 *BCMRAMFN(get_ocl_wakeonclip_cmd_rev44)(void)
{
	return ocl_wakeonclip_cmd_rev44;
}
static uint16 *BCMRAMFN(get_ocl_wakeonclip_dly_rev44)(void)
{
	return ocl_wakeonclip_dly_rev44;
}
static uint16 *BCMRAMFN(get_ocl_shutoff_cmd_rev44)(void)
{
	return ocl_shutoff_cmd_rev44;
}
static uint16 *BCMRAMFN(get_ocl_shutoff_dly_rev44)(void)
{
	return ocl_shutoff_dly_rev44;
}
static uint16 *BCMRAMFN(get_ocl_tx2rx_cmd_rev44)(void)
{
	return ocl_tx2rx_w_far_reset_cmd_rev44;
}
static uint16 *BCMRAMFN(get_ocl_tx2rx_dly_rev44)(void)
{
	return ocl_tx2rx_w_far_reset_dly_rev44;
}

extern uint16 const tiny_rfseq_rx2tx_tssi_sleep_cmd[];
extern uint16 const tiny_rfseq_rx2tx_tssi_sleep_dly[];

static uint16 *BCMRAMFN(get_tiny_4349_ocl_reset2rx_cmd)(void)
{
	return tiny_4349_ocl_reset2rx_cmd;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_reset2rx_dly)(void)
{
	return tiny_4349_ocl_reset2rx_dly;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_shutoff_cmd)(void)
{
	return tiny_4349_ocl_shutoff_cmd;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_shutoff_dly)(void)
{
	return tiny_4349_ocl_shutoff_dly;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_tx2rx_cmd)(void)
{
	return tiny_4349_ocl_tx2rx_cmd;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_tx2rx_dly)(void)
{
	return tiny_4349_ocl_tx2rx_dly;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_rx2tx_tssi_sleep_cmd)(void)
{
	return tiny_4349_ocl_rx2tx_tssi_sleep_cmd;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_rx2tx_tssi_sleep_dly)(void)
{
	return tiny_4349_ocl_rx2tx_tssi_sleep_dly;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_wakeoncrs_cmd)(void)
{
	return tiny_4349_ocl_wakeoncrs_cmd;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_wakeoncrs_cck_cmd)(void)
{
	return tiny_4349_ocl_wakeoncrs_cck_cmd;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_wakeoncrs_dly)(void)
{
	return tiny_4349_ocl_wakeoncrs_dly;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_wakeoncrs_cck_dly)(void)
{
	return tiny_4349_ocl_wakeoncrs_cck_dly;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_wakeonclip_cmd)(void)
{
	return tiny_4349_ocl_wakeonclip_cmd;
}
static uint16 *BCMRAMFN(get_tiny_4349_ocl_wakeonclip_dly)(void)
{
	return tiny_4349_ocl_wakeonclip_dly;
}

/* locally used functions */
static void phy_ac_ocl_nvram_attach(phy_ac_ocl_info_t *ac_ocl_info);
static void phy_ac_ocl_apply_coremask(phy_info_t *pi);
static void phy_ac_ocl_config_rfseq(phy_info_t *pi);
static void phy_ac_ocl_config_phyreg(phy_info_t *pi);
static void phy_ac_set_ocl_crs_peak_thresh(phy_info_t *pi);
static void phy_ac_set_ocl_crs_peak_thresh_28nm(phy_info_t *pi);
static void phy_ac_ocl_config_28nm(phy_info_t *pi);
static void phy_ac_ocl_apply_coremask_28nm(phy_info_t *pi);

/* Functions used by common layer as callbacks */
static int phy_ac_ocl_coremask_change(phy_type_ocl_ctx_t *ctx, uint8 coremask);
static uint8 phy_ac_ocl_get_coremask(phy_type_ocl_ctx_t *ctx);
static int phy_ac_ocl_status_get(phy_type_ocl_ctx_t *ctx,
		uint16 *reqs, uint8 *coremask, bool *ocl_en);
static int phy_ac_ocl_disable_req_set(phy_type_ocl_ctx_t *ctx,
		uint16 req, bool disable, uint8 req_id);

/* register phy type specific implementation */
phy_ac_ocl_info_t*
BCMATTACHFN(phy_ac_ocl_register_impl)(phy_info_t *pi,
	phy_ac_info_t *pi_ac, phy_ocl_info_t *ocli)
{
	phy_ac_ocl_info_t *ac_ocl_info;
	phy_type_ocl_fns_t fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_ocl_info = phy_malloc(pi, sizeof(phy_ac_ocl_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	ac_ocl_info->pi = pi;
	ac_ocl_info->pi_ac = pi_ac;
	ac_ocl_info->ocl_info = ocli;
	ac_ocl_info->data = &(((phy_ac_ocl_mem_t *) ac_ocl_info)->data);

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));

	fns.ocl_coremask_change = phy_ac_ocl_coremask_change;
	fns.ocl_get_coremask = phy_ac_ocl_get_coremask;
	fns.ocl_status_get = phy_ac_ocl_status_get;
	fns.ocl_disable_req_set = phy_ac_ocl_disable_req_set;

	fns.ctx = ac_ocl_info;

	/* Read srom params from nvram */
	phy_ac_ocl_nvram_attach(ac_ocl_info);

	/* Set the bitmap. */
	phy_set_feature_flag(pi, PHY_FEATURE_OCL_IDX, TRUE);

	if (phy_ocl_register_impl(ocli, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_ocl_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_ocl_info;
	/* error handling */
fail:
	if (ac_ocl_info != NULL)
		phy_mfree(pi, ac_ocl_info, sizeof(phy_ac_ocl_mem_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_ocl_unregister_impl)(phy_ac_ocl_info_t *ac_ocl_info)
{
	phy_info_t *pi = ac_ocl_info->pi;
	phy_ocl_info_t *ocli = ac_ocl_info->ocl_info;
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_ocl_unregister_impl(ocli);
	phy_mfree(pi, ac_ocl_info, sizeof(phy_ac_ocl_mem_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static void
BCMATTACHFN(phy_ac_ocl_nvram_attach)(phy_ac_ocl_info_t *ac_ocl_info)
{
	uint8 ocl, ocl_cm;
	phy_info_t *pi = ac_ocl_info->pi;

	ocl = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_ocl, 0x1);
	ocl_cm = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_ocl_cm, 0x1);
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		if ((ocl_cm != 1) && (ocl_cm != 2))
			ocl_cm = 1;
	}

	ac_ocl_info->ocl_en = ocl;
	ac_ocl_info->data->ocl_coremask = ocl_cm;
	/* nvram to control init ocl_disable state */
	if (ac_ocl_info->ocl_en)
		ac_ocl_info->data->ocl_disable_reqs = 0;
	else
		ac_ocl_info->data->ocl_disable_reqs = OCL_DISABLED_HOST;
}

static void
phy_ac_ocl_apply_coremask(phy_info_t *pi)
{
	uint8 noop = 0;
	phy_ac_ocl_info_t *ac_ocl_info = pi->u.pi_acphy->ocli;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	if (phy_get_phymode(pi) == PHYMODE_MIMO) {
		uint8 sleeping_core, active_core;
		/* write ocl wakeonclip cmd and dly */
		si_core_cflags(pi->sh->sih, SICF_PHYMODE, 0x20000);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x230, 16,
			get_tiny_4349_ocl_wakeonclip_cmd());
		si_core_cflags(pi->sh->sih, SICF_PHYMODE, 0x40000);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x230, 16,
			get_tiny_4349_ocl_wakeonclip_cmd());
		si_core_cflags(pi->sh->sih, SICF_PHYMODE, 0x00000);

		/* Replace 0x84 and 0x85 in ocl wakeoncrs and wakeonclip
		   with noop only on active core
		   on 4349 variants phyreg or tables written with phymode 0x20000
		   gets written only to core0 even if they are common reg or table's
		*/
		if (ac_ocl_info->data->ocl_coremask == 1) {
			/* switch phymode to 0x20000. writes happen only on core0 */
			si_core_cflags(pi->sh->sih, SICF_PHYMODE, 0x20000);
			active_core = CORE0_IDX;
			sleeping_core = CORE1_IDX;
		} else {
			/* switch phymode to 0x40000. writes happen only on core1 */
			si_core_cflags(pi->sh->sih, SICF_PHYMODE, 0x40000);
			active_core = CORE1_IDX;
			sleeping_core = CORE0_IDX;
		}
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x240, 16, &noop);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x244, 16, &noop);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x250, 16, &noop);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x254, 16, &noop);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x230, 16, &noop);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x234, 16, &noop);
		si_core_cflags(pi->sh->sih, SICF_PHYMODE, 0x00000);
		wlc_phy_set_crs_min_offsets_acphy(pi, active_core, CRSMIN_MIN, CRSMIN_MIN);
		wlc_phy_set_crs_min_offsets_acphy(pi, sleeping_core, CRSMIN_MAX, CRSMIN_MAX);

		MOD_PHYREG(pi, OCLControl1, ocl_rx_core_mask,
				ac_ocl_info->data->ocl_coremask);
	}
	wlapi_enable_mac(pi->sh->physhim);
}

static void
phy_ac_ocl_config_rfseq(phy_info_t *pi)
{
	uint16 regval = 0;

	if (phy_get_phymode(pi) == PHYMODE_MIMO) {
		/* write ocl shutoff cmd and dly */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x260, 16,
			get_tiny_4349_ocl_shutoff_cmd());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x3f0, 16,
			get_tiny_4349_ocl_shutoff_dly());

		/* write ocl tx2rx cmd and dly */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x280, 16,
			get_tiny_4349_ocl_tx2rx_cmd());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x310, 16,
			get_tiny_4349_ocl_tx2rx_dly());

		/* write ocl reset2rx cmd and dly */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x290, 16,
			get_tiny_4349_ocl_reset2rx_cmd());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x320, 16,
			get_tiny_4349_ocl_reset2rx_dly());

		/* write ocl wakeoncrs cmd and dly */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x240, 16,
			get_tiny_4349_ocl_wakeoncrs_cmd());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x2d0, 16,
			get_tiny_4349_ocl_wakeoncrs_dly());

		/* write ocl wakeoncrs cck cmd and dly */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x250, 16,
			get_tiny_4349_ocl_wakeoncrs_cck_cmd());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x2e0, 16,
			get_tiny_4349_ocl_wakeoncrs_cck_dly());

		/* write ocl wakeonclip cmd and dly */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x230, 16,
			get_tiny_4349_ocl_wakeonclip_cmd());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x2c0, 16,
			get_tiny_4349_ocl_wakeonclip_dly());

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			1, 0x14B, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x210, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x170, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x176, 16, &regval);

		regval = 0xFFFC;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x212, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x177, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x17d, 16, &regval);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			1, 0x15B, 16, &regval);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x340, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x180, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x186, 16, &regval);

		regval = 0xFFFC;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x342, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x187, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x18d, 16, &regval);

		regval = 0xAEEF;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x36F, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x37F, 16, &regval);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			1, 0xF6, 16, &regval);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1D3, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1D0, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1D1, 16, &regval);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			1, 0xF9, 16, &regval);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1D7, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1D4, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1D5, 16, &regval);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			1, 0xF7, 16, &regval);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1E3, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1E0, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1E1, 16, &regval);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			1, 0xFA, 16, &regval);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1E7, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1E4, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1E5, 16, &regval);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			1, 0x129, 16, &regval);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3B9, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3Bb, 16, &regval);
	}
}

static void
phy_ac_ocl_config_phyreg(phy_info_t *pi)
{
	/* Make gain backoff on sleeping core 0 */
	WRITE_PHYREG(pi, OCL_WakeOnDetect_Backoff0, 0);
	WRITE_PHYREG(pi, OCL_WakeOnDetect_Backoff1, 0);

	MOD_PHYREG(pi, OCL_tx2rx_Ctrl, use_tx2rx_seq_in_ocl, 0);
	MOD_PHYREG(pi, OCLControl5, use_tx2rx_seq_in_ocl, 0);

	MOD_PHYREG(pi, OCLControl1, ocl_bphy_dontwakeup_core, 0);
	MOD_PHYREG(pi, OCLControl1, ocl_core_mask_ovr, 1);
	MOD_PHYREG(pi, OCLControl1, dsss_cck_scd_shutOff_enable, 0);

	/* force c-str on active core only when wake on crs happens */
	MOD_PHYREG(pi, OCLControl2, wo_det_hipwrant_sel, 1);
	/* force f-str, cfo, ffo to active core only when wake on crs happens */
	MOD_PHYREG(pi, OCLControl2, scale_ant_weights_ocl_wo_det, 0);
	MOD_PHYREG(pi, OCLControl2, use_only_active_core_cfo_ffo, 1);
	MOD_PHYREG(pi, OCLControl2, OCLcckdigigainEnCntValue,
		READ_PHYREGFLD(pi, overideDigiGain1, cckdigigainEnCntValue));
	/* Increase ocl_ant_decision_est_holdoff_ctr to 2usec for ofdm and cck
	   to delay rssi latching on sleeping core by 1usec.
	 */
	WRITE_PHYREG(pi, ocl_ant_decision_est_holdoff_ctr_ofdm_det, 0x50);
	WRITE_PHYREG(pi, ocl_ant_decision_est_holdoff_ctr_cck_det, 0x50);
}

static void
phy_ac_set_ocl_crs_peak_thresh(phy_info_t *pi)
{
	MOD_PHYREG(pi, OCL_crsThreshold2uSub1, OCL_peakDiffThresh,
		READ_PHYREGFLD(pi, crsThreshold2uSub10, peakDiffThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2uSub1, OCL_peakThresh,
		READ_PHYREGFLD(pi, crsThreshold2uSub10, peakThresh));
	MOD_PHYREG(pi, OCL_crsThreshold3uSub1, OCL_peakValThresh,
		READ_PHYREGFLD(pi, crsThreshold3uSub10, peakValThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2lSub1, OCL_peakDiffThresh,
		READ_PHYREGFLD(pi, crsThreshold2lSub10, peakDiffThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2lSub1, OCL_peakThresh,
		READ_PHYREGFLD(pi, crsThreshold2lSub10, peakThresh));
	MOD_PHYREG(pi, OCL_crsThreshold3lSub1, OCL_peakValThresh,
		READ_PHYREGFLD(pi, crsThreshold3lSub10, peakValThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2u, OCL_peakDiffThresh,
		READ_PHYREGFLD(pi, crsThreshold2u0, peakDiffThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2u, OCL_peakThresh,
		READ_PHYREGFLD(pi, crsThreshold2u0, peakThresh));
	MOD_PHYREG(pi, OCL_crsThreshold3u, OCL_peakValThresh,
		READ_PHYREGFLD(pi, crsThreshold3u0, peakValThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2l, OCL_peakDiffThresh,
		READ_PHYREGFLD(pi, crsThreshold2l0, peakDiffThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2l, OCL_peakThresh,
		READ_PHYREGFLD(pi, crsThreshold2l0, peakThresh));
	MOD_PHYREG(pi, OCL_crsThreshold3l, OCL_peakValThresh,
		READ_PHYREGFLD(pi, crsThreshold3l0, peakValThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1uSub1, OCL_highpowautoThresh2,
		READ_PHYREGFLD(pi, crshighpowThreshold1uSub10, highpowautoThresh2));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1uSub1, OCL_highpowpeakValThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold1uSub10, highpowpeakValThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1lSub1, OCL_highpowautoThresh2,
		READ_PHYREGFLD(pi, crshighpowThreshold1lSub10, highpowautoThresh2));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1lSub1, OCL_highpowpeakValThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold1lSub10, highpowpeakValThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1u, OCL_highpowautoThresh2,
		READ_PHYREGFLD(pi, crshighpowThreshold1u0, highpowautoThresh2));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1u, OCL_highpowpeakValThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold1u0, highpowpeakValThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1l, OCL_highpowautoThresh2,
		READ_PHYREGFLD(pi, crshighpowThreshold1l0, highpowautoThresh2));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1l, OCL_highpowpeakValThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold1l0, highpowpeakValThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2uSub1, OCL_highpowpeakDiffThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2uSub10, highpowpeakDiffThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2uSub1, OCL_highpowpeakThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2uSub10, highpowpeakThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2lSub1, OCL_highpowpeakDiffThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2lSub10, highpowpeakDiffThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2lSub1, OCL_highpowpeakThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2lSub10, highpowpeakThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2u, OCL_highpowpeakDiffThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2u0, highpowpeakDiffThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2u, OCL_highpowpeakThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2u0, highpowpeakThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2l, OCL_highpowpeakDiffThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2l0, highpowpeakDiffThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2l, OCL_highpowpeakThresh,
		READ_PHYREGFLD(pi, crshighpowThreshold2l0, highpowpeakThresh));
}

static void
phy_ac_set_ocl_crs_peak_thresh_28nm(phy_info_t *pi)
{
	uint8 core;
	phy_ac_ocl_info_t *ac_ocl_info = pi->u.pi_acphy->ocli;

	if (ac_ocl_info->data->ocl_coremask == 1) {
		core = 0;
	} else {
		core = 1;
	}
	MOD_PHYREG(pi, OCL_crsThreshold2u, OCL_peakDiffThresh,
		READ_PHYREGFLDCE(pi, crsThreshold2u, core, peakDiffThresh));
	MOD_PHYREG(pi, OCL_crsThreshold2u, OCL_peakThresh,
		READ_PHYREGFLDCE(pi, crsThreshold2u, core, peakThresh));
	MOD_PHYREG(pi, OCL_crsThreshold3u, OCL_peakValThresh,
		READ_PHYREGFLDCE(pi, crsThreshold3u, core, peakValThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1u, OCL_highpowautoThresh2,
		READ_PHYREGFLDCE(pi, crshighpowThreshold1u, core, highpowautoThresh2));
	MOD_PHYREG(pi, OCL_crshighpowThreshold1u, OCL_highpowpeakValThresh,
		READ_PHYREGFLDCE(pi, crshighpowThreshold1u, core, highpowpeakValThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2u, OCL_highpowpeakDiffThresh,
		READ_PHYREGFLDCE(pi, crshighpowThreshold2u, core, highpowpeakDiffThresh));
	MOD_PHYREG(pi, OCL_crshighpowThreshold2u, OCL_highpowpeakThresh,
		READ_PHYREGFLDCE(pi, crshighpowThreshold2u, core, highpowpeakThresh));

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		MOD_PHYREG(pi, OCL_crsThreshold2l, OCL_peakDiffThresh,
			READ_PHYREGFLDCE(pi, crsThreshold2l, core, peakDiffThresh));
		MOD_PHYREG(pi, OCL_crsThreshold2l, OCL_peakThresh,
			READ_PHYREGFLDCE(pi, crsThreshold2l, core, peakThresh));
		MOD_PHYREG(pi, OCL_crsThreshold3l, OCL_peakValThresh,
			READ_PHYREGFLDCE(pi, crsThreshold3l, core, peakValThresh));
		MOD_PHYREG(pi, OCL_crshighpowThreshold1l, OCL_highpowautoThresh2,
			READ_PHYREGFLDCE(pi, crshighpowThreshold1l, core, highpowautoThresh2));
		MOD_PHYREG(pi, OCL_crshighpowThreshold1l, OCL_highpowpeakValThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold1l, core, highpowpeakValThresh));
		MOD_PHYREG(pi, OCL_crshighpowThreshold2l, OCL_highpowpeakDiffThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold2l, core, highpowpeakDiffThresh));
		MOD_PHYREG(pi, OCL_crshighpowThreshold2l, OCL_highpowpeakThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold2l, core, highpowpeakThresh));
	}

	if (CHSPEC_IS80(pi->radio_chanspec)) {
		MOD_PHYREG(pi, OCL_crsThreshold2uSub1, OCL_peakDiffThresh,
			READ_PHYREGFLDCE(pi, crsThreshold2uSub1, core, peakDiffThresh));
		MOD_PHYREG(pi, OCL_crsThreshold2uSub1, OCL_peakThresh,
			READ_PHYREGFLDCE(pi, crsThreshold2uSub1, core, peakThresh));
		MOD_PHYREG(pi, OCL_crsThreshold3uSub1, OCL_peakValThresh,
			READ_PHYREGFLDCE(pi, crsThreshold3uSub1, core, peakValThresh));
		MOD_PHYREG(pi, OCL_crsThreshold2lSub1, OCL_peakDiffThresh,
			READ_PHYREGFLDCE(pi, crsThreshold2lSub1, core, peakDiffThresh));
		MOD_PHYREG(pi, OCL_crsThreshold2lSub1, OCL_peakThresh,
			READ_PHYREGFLDCE(pi, crsThreshold2lSub1, core, peakThresh));
		MOD_PHYREG(pi, OCL_crsThreshold3lSub1, OCL_peakValThresh,
			READ_PHYREGFLDCE(pi, crsThreshold3lSub1, core, peakValThresh));

		MOD_PHYREG(pi, OCL_crshighpowThreshold1uSub1, OCL_highpowautoThresh2,
			READ_PHYREGFLDCE(pi, crshighpowThreshold1uSub1, core, highpowautoThresh2));
		MOD_PHYREG(pi, OCL_crshighpowThreshold1uSub1, OCL_highpowpeakValThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold1uSub1, core,
				highpowpeakValThresh));
		MOD_PHYREG(pi, OCL_crshighpowThreshold1lSub1, OCL_highpowautoThresh2,
			READ_PHYREGFLDCE(pi, crshighpowThreshold1lSub1, core, highpowautoThresh2));
		MOD_PHYREG(pi, OCL_crshighpowThreshold1lSub1, OCL_highpowpeakValThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold1lSub1, core,
				highpowpeakValThresh));

		MOD_PHYREG(pi, OCL_crshighpowThreshold2uSub1, OCL_highpowpeakDiffThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold2uSub1, core,
				highpowpeakDiffThresh));
		MOD_PHYREG(pi, OCL_crshighpowThreshold2uSub1, OCL_highpowpeakThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold2uSub1, core, highpowpeakThresh));
		MOD_PHYREG(pi, OCL_crshighpowThreshold2lSub1, OCL_highpowpeakDiffThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold2lSub1, core,
				highpowpeakDiffThresh));
		MOD_PHYREG(pi, OCL_crshighpowThreshold2lSub1, OCL_highpowpeakThresh,
			READ_PHYREGFLDCE(pi, crshighpowThreshold2lSub1, core, highpowpeakThresh));
	}
}

static void phy_ac_ocl_init_gain_config_28nm(phy_info_t *pi);
static void phy_ac_ocl_rf_seq_config_28nm(phy_info_t *pi);
static void phy_ac_ocl_phy_reg_config_28nm(phy_info_t *pi);
static void phy_ac_ocl_logen_config_28nm(phy_info_t *pi);

static void
phy_ac_ocl_config_28nm(phy_info_t *pi)
{
	uint16 regval = 0;

	//radio config
	regval = 0x82df;
	//trsw_rx_pwrup(fast_nap_bias_pu) set to 1
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x170, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x180, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x176, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x186, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x210, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x340, 16, &regval);

	regval = 0xfff8;
	//sw_tia_bq1 is connected to rx2g_gm_bypass in 4347A0, set to 0 to avoid gm bypass
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x177, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x187, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x179, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x189, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x17b, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x18b, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x17d, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x18d, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x212, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x342, 16, &regval);

	phy_ac_ocl_init_gain_config_28nm(pi);
	phy_ac_ocl_rf_seq_config_28nm(pi);
	phy_ac_ocl_phy_reg_config_28nm(pi);
	phy_ac_ocl_logen_config_28nm(pi);
}

static void
phy_ac_ocl_init_gain_config_28nm(phy_info_t *pi)
{
	uint16 regval;

	//copy regular init gain for ocl
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xf9, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d4, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d5, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d6, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d7, 16, &regval);

	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xf6, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d0, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d1, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d2, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1d3, 16, &regval);

	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xfa, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e4, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e5, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e6, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e7, 16, &regval);

	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xf7, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e0, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e1, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e2, 16, &regval);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1e3, 16, &regval);
}

static void
phy_ac_ocl_rf_seq_config_28nm(phy_info_t *pi)
{
	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		//update wake_on_clip
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x230, 16,
				ocl_wakeonclip_cmd_rev40);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x2c0, 16,
				ocl_wakeonclip_dly_rev40);

		//update wake_on_crs
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x240, 16,
				ocl_wakeoncrs_cmd_rev40);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x2d0, 16,
				ocl_wakeoncrs_dly_rev40);

		//update ocl_shutoff
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 9, 0x260, 16,
				get_ocl_shutoff_cmd_rev40());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 9, 0x2f0, 16,
				get_ocl_shutoff_dly_rev40());

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 15, 0x280, 16,
				get_ocl_tx2rx_cmd_rev40());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 15, 0x310, 16,
				get_ocl_tx2rx_dly_rev40());
	} else {
		// REV 44 or 46
		//update wake_on_clip
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x230, 16,
				get_ocl_wakeonclip_cmd_rev44());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x2c0, 16,
				get_ocl_wakeonclip_dly_rev44());

		//update wake_on_crs
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x240, 16,
				get_ocl_wakeoncrs_cmd_rev44());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 7, 0x2d0, 16,
				get_ocl_wakeoncrs_dly_rev44());

		//update ocl_shutoff
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 9, 0x260, 16,
				get_ocl_shutoff_cmd_rev44());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 9, 0x2f0, 16,
				get_ocl_shutoff_dly_rev44());

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 15, 0x280, 16,
				get_ocl_tx2rx_cmd_rev44());
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 15, 0x310, 16,
				get_ocl_tx2rx_dly_rev44());
	}
}

static void
phy_ac_ocl_phy_reg_config_28nm(phy_info_t *pi)
{
	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, OCLControl1, dsss_cck_scd_shutOff_enable, 0);
			MOD_PHYREG(pi, ocl_inactivecore_gain_ctrl,
					ocl_inactivecore_wakeon_crs_defer_pktgain, 1);
			MOD_PHYREG(pi, ocl_inactivecore_gain_ctrl,
					ocl_inactivecore_wakeon_crs_defer_pktgain_initindx, 2);
			WRITE_PHYREG(pi, ocl_ant_decision_est_holdoff_ctr_ofdm_det, 64);
			WRITE_PHYREG(pi, ocl_ant_decision_est_holdoff_ctr_cck_det, 64);
			//more blanking time needed for asymmetric SOI
			WRITE_PHYREG(pi, ocl_inactivecore_wakeon_crs_pktabort_blanking_len, 360);
			MOD_PHYREG(pi, OCLControl2, OCLcckdigigainEnCntValue, 130);
		} else {	/* ACMAJORREV_40 */
			MOD_PHYREG(pi, OCLControl1, dsss_cck_scd_shutOff_enable, 1);
			MOD_PHYREG(pi, ocl_inactivecore_gain_ctrl,
					ocl_inactivecore_wakeon_crs_defer_pktgain, 1);
			MOD_PHYREG(pi, ocl_inactivecore_gain_ctrl,
					ocl_inactivecore_wakeon_crs_defer_pktgain_initindx, 2);
			WRITE_PHYREG(pi, ocl_ant_decision_est_holdoff_ctr_ofdm_det, 64);
			WRITE_PHYREG(pi, ocl_ant_decision_est_holdoff_ctr_cck_det, 64);
			//more blanking time needed for asymmetric SOI
			WRITE_PHYREG(pi, ocl_inactivecore_wakeon_crs_pktabort_blanking_len, 360);
			MOD_PHYREG(pi, OCLControl2, OCLcckdigigainEnCntValue, 130);
		}
		if (CHSPEC_IS20(pi->radio_chanspec))
			MOD_PHYREG(pi, FSTRHiPwrTh, finestr_hiPwr_th, 0x38);
		else
			MOD_PHYREG(pi, FSTRHiPwrTh, finestr_hiPwr_th, 0x35);
	}

	//MOD_PHYREG(pi, fineRxclockgatecontrol, useOclRxfrontEndGating, 1);
	MOD_PHYREG(pi, OCL_WakeOnDetect_Backoff0, ocl_cck_gain_backoff_db, 0);
	MOD_PHYREG(pi, OCL_WakeOnDetect_Backoff1, ocl_cck_gain_backoff_db, 0);
	MOD_PHYREG(pi, OCLControl1, ocl_wake_on_energy_en, 0);
	MOD_PHYREG(pi, OCLControl1, dis_ocl_ed_if_clip, 1);
	MOD_PHYREG(pi, OCLControl2, use_gud_pkt_active_core, 1);
}

static void
phy_ac_ocl_logen_config_28nm(phy_info_t *pi)
{
	uint16 regval, mask;
	phy_ac_ocl_info_t *ac_ocl_info = pi->u.pi_acphy->ocli;

	if (ACMAJORREV_40(pi->pubpi->phy_rev) &&
		phy_ac_chanmgr_get_val_nonbf_logen_mode(pi->u.pi_acphy->chanmgri)) {
		regval = 0x9000;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe2, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe4, 16, &regval);

		regval = 0xfee0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1a0, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1a7, 16, &regval);

		regval = 0xfeff;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1a2, 16, &regval);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1a6, 16, &regval);

		if (RADIOMAJORREV(pi) >= 3) {
			//4347B0 For low power logen bit 7 is set to 0 for core1
			regval = 0xfe60;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b0, 16, &regval);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b7, 16, &regval);
			//4347B0 For low power logen bit 7 is set to 0 for core1
			regval = 0xfe7f;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b2, 16, &regval);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b6, 16, &regval);
		} else {
			regval = 0xfee0;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b0, 16, &regval);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b7, 16, &regval);
			regval = 0xfeff;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b2, 16, &regval);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1b6, 16, &regval);

		}
	}
	/* rf seq config for logen. Related JIRAs: HW-1518, HW-1519 */
	/* Related TCL proc acphy_load_rfseq_logen */
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {

		mask = 0x8484;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe8, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe7, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe3, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe5, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe1, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xeb, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe4, 16, mask);

		mask = 0x303;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x586, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x581, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x583, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xee, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xef, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x582, 16, mask);

		mask = 0x3;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x585, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xed, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x580, 16, mask);

		mask = 0x88a;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x36f, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x37f, 16, mask);

		mask = 0x2;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x176, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x170, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x172, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x174, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x186, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x210, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x340, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x180, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x182, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x181, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x183, 16, mask);

		mask = 0x8400;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe2, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe6, 16, mask);

		mask = 0x300;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x580, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x584, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x585, 16, mask);

		mask = 0x800;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x36e, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x37e, 16, mask);

		mask = 0x84;
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe0, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0xe2, 16, mask);

		/* turn off rx2g_gm_bias_en and lna2g_lna1_bias_pu */
		mask = 0x600;
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x1A0, 16, mask);
		wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x1B0, 16, mask);

		/* 0x36f, 0x37f, 0x38f corresonds logen_pwrup for core 0, 1 and 2(legacy) */
		mask = 0xa020;
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x36f, 16, mask);
		wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x37f, 16, mask);

		if (ac_ocl_info->data->ocl_coremask == 1) {
			/* settings for OCL Shutdown for RXPD_TXPD, RXPD_TXPD1 and RXPD_TXPD2
			 * 0x171, 0x173, 0x175 corresponds to logen_core0_pu for PD, PD1 and PD2
			 * 0x181, 0x183, 0x185 corresponds to logen_core1_pu for PD, PD1 and PD2
			 * 0x191, 0x193, 0x195 corresponds to logen_core2_pu for PD, PD1 and PD2
			 */
			mask = 0x2;
			wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x171, 16, mask);
			wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x181, 16, mask);

			wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x173, 16, mask);
			wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x183, 16, mask);

			wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x175, 16, mask);
			wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x185, 16, mask);
		} else if (ac_ocl_info->data->ocl_coremask == 2) {
			/* settings for OCL Shutdown for RXPD_TXPD, RXPD_TXPD1 and RXPD_TXPD2
			 * 0x36f, 0x37f, 0x38f corresonds logen_pwrup for core 0, 1 and 2(legacy)
			 * 0x171, 0x173, 0x175 corresponds to logen_core0_pu for PD, PD1 and PD2
			 * 0x181, 0x183, 0x185 corresponds to logen_core1_pu for PD, PD1 and PD2
			 * 0x191, 0x193, 0x195 corresponds to logen_core2_pu for PD, PD1 and PD2
			 */
			mask = 0x2;
			wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x171, 16, mask);
			wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x181, 16, mask);

			wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x173, 16, mask);
			wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x183, 16, mask);

			wlc_phy_table_clrbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x175, 16, mask);
			wlc_phy_table_setbit_acphy(pi, ACPHY_TBL_ID_RFSEQ, 0x185, 16, mask);
		}

	}
}

static void
phy_ac_ocl_apply_coremask_28nm(phy_info_t *pi)
{
	uint8 ocl_listen_core;
	phy_ac_ocl_info_t *ac_ocl_info = pi->u.pi_acphy->ocli;

	if (ac_ocl_info->data->ocl_coremask == 1) {
		ocl_listen_core = 0;
		MOD_PHYREGCE(pi, pllLogenMaskCtrl, 0, logen_reset_mask, 0);
		MOD_PHYREGCE(pi, pllLogenMaskCtrl, 1, logen_reset_mask, 1);
	} else {
		ocl_listen_core = 1;
		MOD_PHYREGCE(pi, pllLogenMaskCtrl, 0, logen_reset_mask, 1);
		MOD_PHYREGCE(pi, pllLogenMaskCtrl, 1, logen_reset_mask, 0);
	}
	MOD_PHYREG(pi, OCLControl1, ocl_core_mask_ovr, 1);
	MOD_PHYREG(pi, OCLControl1, ocl_rx_core_mask, ac_ocl_info->data->ocl_coremask);
	//move to the enable function
	//MOD_PHYREG(pi, HPFBWovrdigictrl, bphy_core_sel_ovr, 1);
	MOD_PHYREG(pi, HPFBWovrdigictrl, bphy_core_sel_val, ocl_listen_core);
	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, ocl_sgi_eventDelta_AdjCount, scd_core_ovr_en, 1);
		MOD_PHYREG(pi, ocl_sgi_eventDelta_AdjCount, scd_core_ovr_val, ocl_listen_core);
	}
}

static void
phy_ac_ocl_enable(phy_info_t *pi, bool enable)
{
	uint16 core;
	uint8 sleeping_core, active_core;
	phy_ac_info_t *pi_ac = pi->u.pi_acphy;
	bool ocl_en_status = (READ_PHYREGFLD(pi, OCLControl1, ocl_mode_enable) == 1);
	phy_ac_ocl_info_t *ac_ocl_info = pi_ac->ocli;

	if (enable == ocl_en_status)
		return;
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, 1);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	OSL_DELAY(10);
	if (enable && (phy_stf_get_data(pi->stfi)->phyrxchain == 3)) {
		MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 0);
		MOD_PHYREG(pi, fineRxclockgatecontrol, useOclRxfrontEndGating, 1);
		MOD_PHYREG(pi, OCLControl1, ocl_mode_enable, 1);
		if (TINY_RADIO(pi)) {
			MOD_PHYREG(pi, OCLControl1, InactiveCore_Adc_turnOff, 0);
			if (ac_ocl_info->data->ocl_coremask == 1) {
				WRITE_PHYREG(pi, PREMPT_per_pkt_en1, 0);
				MOD_RADIO_REG_20693(pi, SPARE_CFG15, 1, offset_dac_ovr, 3);
				active_core = CORE0_IDX;
				sleeping_core = CORE1_IDX;
			} else {
				WRITE_PHYREG(pi, PREMPT_per_pkt_en0, 0);
				MOD_RADIO_REG_20693(pi, SPARE_CFG15, 0, offset_dac_ovr, 3);
				active_core = CORE1_IDX;
				sleeping_core = CORE0_IDX;
			}
			wlc_phy_set_crs_min_offsets_acphy(pi, active_core, CRSMIN_MIN,
				CRSMIN_MIN);
			wlc_phy_set_crs_min_offsets_acphy(pi, sleeping_core, CRSMIN_MAX,
				CRSMIN_MAX);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
				get_tiny_4349_ocl_rx2tx_tssi_sleep_cmd());
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				get_tiny_4349_ocl_rx2tx_tssi_sleep_dly());
			MOD_PHYREG(pi, AntDivConfig2059, bphy_core_div, 0);
		}
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			if (HW_ACMINORREV(pi) == 1) {
				MOD_PHYREG(pi, HPFBWovrdigictrl, bphyNoCoreRemap, 0);
			}
			MOD_PHYREG(pi, bphymrcCtrl, bphy_mrc_en, 0);
			MOD_PHYREG(pi, HPFBWovrdigictrl, bphy_core_sel_ovr, 1);
			if (HW_ACMINORREV(pi) == 0) {
				MOD_PHYREG(pi, fineRxclockgatecontrol, EncodeGainClkEn, 0);
				MOD_PHYREG(pi, RxFeCtrl1, forceSdFeClkEn, 3);
			}
		} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			if (pi->pubpi->slice == DUALMAC_AUX) {
				MOD_PHYREG(pi, HPFBWovrdigictrl, bphy_core_sel_ovr, 1);
			}
			MOD_PHYREG(pi, OCLControl1, InactiveCore_Adc_turnOff, 1);
		}
	} else {
		MOD_PHYREG(pi, OCLControl1, ocl_mode_enable, 0);
		MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 1);
		MOD_PHYREG(pi, fineRxclockgatecontrol, useOclRxfrontEndGating, 0);
		if (TINY_RADIO(pi)) {
			FOREACH_CORE(pi, core) {
				WRITE_PHYREGCE(pi, PREMPT_per_pkt_en, core, 0x3d);
				MOD_RADIO_REG_20693(pi, SPARE_CFG15, core, offset_dac_ovr, 0);
				wlc_phy_set_crs_min_offsets_acphy(pi, core, CRSMIN_MIN,
					CRSMIN_MIN);
			}
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
				tiny_rfseq_rx2tx_tssi_sleep_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				tiny_rfseq_rx2tx_tssi_sleep_dly);
			MOD_PHYREG(pi, AntDivConfig2059, bphy_core_div, 1);
		}
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			if (pi->sh->bphymrc_en && CHSPEC_IS2G(pi->radio_chanspec)) {
				if (HW_ACMINORREV(pi) == 1) {
					MOD_PHYREG(pi, HPFBWovrdigictrl, bphyNoCoreRemap, 1);
				}
				MOD_PHYREG(pi, bphymrcCtrl, bphy_mrc_en, 1);
			}
			MOD_PHYREG(pi, HPFBWovrdigictrl, bphy_core_sel_ovr, 0);
			if (HW_ACMINORREV(pi) == 0) {
				MOD_PHYREG(pi, fineRxclockgatecontrol, EncodeGainClkEn, 1);
				MOD_PHYREG(pi, RxFeCtrl1, forceSdFeClkEn, 0);
			}
		} else if (ACMAJORREV_44(pi->pubpi->phy_rev) && pi->pubpi->slice == DUALMAC_AUX) {
			MOD_PHYREG(pi, HPFBWovrdigictrl, bphy_core_sel_ovr, 0);
		}
	}
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	wlc_phy_resetcca_acphy(pi);
	MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, 0);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);
	wlapi_enable_mac(pi->sh->physhim);
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/* ****       Functions used by other AC modules     **** */
void
phy_ac_ocl_config(phy_ac_ocl_info_t *ocli)
{
	phy_info_t *pi = ocli->pi;

	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		phy_ac_ocl_config_28nm(pi);

		phy_ac_set_ocl_crs_peak_thresh_28nm(pi);

		phy_ac_ocl_apply_coremask_28nm(pi);
	} else {
		if (phy_get_phymode(pi) == PHYMODE_MIMO) {
			phy_ac_ocl_config_rfseq(pi);

			phy_ac_ocl_config_phyreg(pi);

			/* copy ocl crs peak thresholds from regular crs peak thresholds */
			phy_ac_set_ocl_crs_peak_thresh(pi);

			phy_ac_ocl_apply_coremask(pi);
		}
	}
}

phy_ac_ocl_data_t *
phy_ac_ocl_get_data(phy_ac_ocl_info_t *ocli)
{
	return ocli->data;
}

/* **************************************** */
/*	Callback Functions                                          */
/* **************************************** */
static int
phy_ac_ocl_coremask_change(phy_type_ocl_ctx_t *ctx, uint8 coremask)
{
	phy_ac_ocl_info_t *info = (phy_ac_ocl_info_t *)ctx;
	phy_info_t *pi = info->pi;

	info->data->ocl_coremask = coremask;
	phy_ac_ocl_enable(pi, FALSE);
	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		phy_ac_ocl_apply_coremask_28nm(pi);
	} else {
		phy_ac_ocl_apply_coremask(pi);
	}
	phy_ac_ocl_enable(pi, !info->data->ocl_disable_reqs);

#ifdef WL_NAP
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev) &&
			PHY_NAP_ENAB(pi->sh->physhim)) {
			phy_ac_nap_update_energy_threshold(pi->u.pi_acphy->napi);
			}
#endif /* WL_NAP */
	return BCME_OK;
}

static uint8
phy_ac_ocl_get_coremask(phy_type_ocl_ctx_t *ctx)
{
	phy_ac_ocl_info_t *info = (phy_ac_ocl_info_t *)ctx;

	return info->data->ocl_coremask;
}

static int
phy_ac_ocl_status_get(phy_type_ocl_ctx_t *ctx, uint16 *reqs, uint8 *coremask, bool *ocl_en)
{
	phy_ac_ocl_info_t *info = (phy_ac_ocl_info_t *)ctx;
	phy_info_t *pi = info->pi;

	if (reqs != NULL) {
		*reqs = info->data->ocl_disable_reqs;
	}
	if (coremask != NULL) {
		*coremask = info->data->ocl_coremask;
	}
	if (ocl_en != NULL && pi->sh->clk) {
		*ocl_en = (READ_PHYREGFLD(pi, OCLControl1, ocl_mode_enable) == 1);
	}

	return BCME_OK;
}

static int
phy_ac_ocl_disable_req_set(phy_type_ocl_ctx_t *ctx, uint16 req, bool disable, uint8 req_id)
{
	phy_ac_ocl_info_t *info = (phy_ac_ocl_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint16 disable_req_old;
	uint8 cm_hw = 0;
	bool ocl_en = FALSE;
	bool log = TRUE;

	disable_req_old = info->data->ocl_disable_reqs;

	if (disable) {
		info->data->ocl_disable_reqs |= req;
	} else {
		info->data->ocl_disable_reqs &= ~req;
	}

	if (pi->sh->clk) {
		phy_ac_ocl_enable(pi, !info->data->ocl_disable_reqs);
#ifdef WL_NAP
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev) &&
			PHY_NAP_ENAB(pi->sh->physhim)) {
			phy_ac_nap_update_energy_threshold(pi->u.pi_acphy->napi);
			}
#endif /* WL_NAP */
	}

#if defined(EVENT_LOG_COMPILE)
	if (!EVENT_LOG_IS_ON(EVENT_LOG_TAG_OCL_INFO)) {
		log = FALSE;
	}
#endif /* EVENT_LOG_COMPILE */

	if (log) {
		phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);
		if (pi->sh->clk) {
			cm_hw = READ_PHYREGFLD(pi, OCLControl1, ocl_rx_core_mask);
			ocl_en = (READ_PHYREGFLD(pi, OCLControl1, ocl_mode_enable) == 1);
		}
		BCM_REFERENCE(cm_hw);
		BCM_REFERENCE(ocl_en);
		BCM_REFERENCE(stf_shdata);
		BCM_REFERENCE(disable_req_old);

		PHY_INFORM(("Request from %d: disable new|old %08x "
						  "chan|cm_sw_hw|ocl_clk %08x, tx|rx %08x\n",
						  req_id,
						  ((info->data->ocl_disable_reqs << 16) |
						  disable_req_old),
						  ((pi->radio_chanspec << 16)|
						   ((info->data->ocl_coremask & 0x03) << 12) |
						   ((cm_hw & 0x3) << 8) |
						   (ocl_en << 4) | pi->sh->clk),
						  ((stf_shdata->hw_phytxchain << 24) |
						   (stf_shdata->hw_phyrxchain << 16) |
						   (stf_shdata->phytxchain << 8) |
						   stf_shdata->phyrxchain)));
	}

	return BCME_OK;
}
#endif /* OCL */
