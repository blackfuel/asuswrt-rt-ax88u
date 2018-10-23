/*
 * Broadcom HWA driver RTE layer
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
 * $Id:$
 */

#include <bcmutils.h>
#include <rte_cons.h>
#include <hndsoc.h>
#include <ethernet.h>
#include <bcmevent.h>
#include <hwa_rte.h>
#include <hwa_lib.h>

#ifdef HWA_DPC_BUILD
static void _hwa_intrson(hwa_dev_t *dev);
static void _hwa_dpc(hwa_dev_t *dev);
static void _hwa_dpctask(void *cbdata);

#ifndef RTE_POLL
static void hwa_isr(void *cbdata);
#ifdef THREAD_SUPPORT
static void hwa_dpc_thread(void *cbdata);
#endif /* THREAD_SUPPORT */
static void hwa_run(hwa_dev_t *dev);
#endif /* !RTE_POLL */

/* thread-safe interrupt enable */
static void
_hwa_intrson(hwa_dev_t *dev)
{
	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
#endif  /* THREAD_SUPPORT */

	hwa_intrson(dev);

#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_restore(state);
#endif  /* THREAD_SUPPORT */
}

static void
_hwa_dpc(hwa_dev_t *dev)
{
	bool resched = 0;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);
	HWA_ASSERT(dev->sys_dev != (hnd_dpc_t*)NULL);

	resched = hwa_dpc(dev);

	/* flush wl->rxcpl_list for rxcpl fast path. */
	/* flush accumulated txrxstatus here */
	hwa_wl_flush_all(dev);

	if (resched) {
		if (!hnd_dpc_schedule(dev->sys_dev)) {
			ASSERT(FALSE);
		}
	} else {
		_hwa_intrson(dev);
	}
}

/* re-schedule handle function */
static void
_hwa_dpctask(void *cbdata)
{
	hwa_dev_t *drv = (hwa_dev_t*)cbdata;

	hwa_intrsupd(drv);
	_hwa_dpc(drv);
}

#ifndef RTE_POLL
static void
hwa_run(hwa_dev_t *dev)
{
	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

#ifndef THREAD_SUPPORT
	hwa_intrsoff(dev);
#endif  /* THREAD_SUPPORT */
	/* call common first level interrupt handler */
	if (hwa_dispatch(dev)) {
		/* if more to do... */
		_hwa_dpc(dev);
	} else {
		/* isr turned off interrupts */
		_hwa_intrson(dev);
	}
}

static void
hwa_isr(void *cbdata)
{
	hwa_dev_t *dev = (hwa_dev_t *)cbdata;

#ifdef THREAD_SUPPORT
	/* deassert interrupt */
	hwa_intrsoff(dev);
#else
	hwa_run(dev);
#endif	/* THREAD_SUPPORT */
}

#ifdef THREAD_SUPPORT
static void
hwa_dpc_thread(void *cbdata)
{
	hwa_dev_t *dev = (hwa_dev_t *)cbdata;

	hwa_run(dev);
}
#endif /* THREAD_SUPPORT */

#endif /* !RTE_POLL */
#endif /* HWA_DPC_BUILD */

#define AXI_MEM16_FMT	" AXI_MEM16<0x%08x> value<0x%04x>"
#define AXI_MEM32_FMT	" AXI_MEM32<0x%08x> value<0x%08x>"
static void
_hwa_dump(void *arg, int argc, char *argv[])
{
#if defined(BCMDBG)
	hwa_dev_t *dev = (hwa_dev_t *)arg;
	bool verbose = FALSE;
	bool dump_regs = FALSE;
	bool dump_txfifo_shadow = FALSE;
#endif // endif
	hwa_mem_addr_t axi_mem_addr;
	uint32 u32, size, count, c_idx;
	uint16 u16;

	// 1. Usage:  dhd -i eth1 cons "hwa dump [verbose] [dump_regs][Txshadow]"
	// 2. Usage:  dhd -i eth1 cons "hwa mem [addr] [size] [val]"
	// NOTE: use "md" console command to dump sysmem "md [addr] [size] [count]
	// i.e. dhd -i eth1 cons "md 0x4bd7c0 4 32"
	if (argc < 2) {
		HWA_PRINT("Wrong argument!\n");
		return;
	}

	if (strcmp(argv[1], "mem") == 0) {
		// dhd -i eth1 cons "hwa mem [0x28509000] [4] [0x12345678]"

		if (argc < 4) {
			HWA_PRINT("Wrong argument!\n");
			return;
		}

		axi_mem_addr = bcm_strtoul(argv[2], NULL, 0);
		size = bcm_strtoul(argv[3], NULL, 0);
		if (size != 2 && size != 4) {
			HWA_PRINT("Wrong argument!\n");
			return;
		}

		if (argv[4]) {
			/* SET */
			if (size == 2) {
				u16 = (uint16)bcm_strtoul(argv[4], NULL, 0);
				HWA_WR_MEM16("AXI_MEM", uint16, axi_mem_addr, &u16);
				HWA_PRINT("Set" AXI_MEM16_FMT "\n", axi_mem_addr, u16);
			}
			else {
				u32 = (uint32)bcm_strtoul(argv[4], NULL, 0);
				HWA_WR_MEM32("AXI_MEM", uint32, axi_mem_addr, &u32);
				HWA_PRINT("Set" AXI_MEM32_FMT "\n", axi_mem_addr, u32);
			}
		} else {
			/* GET */
			if (size == 2) {
				HWA_RD_MEM16("AXI_MEM", uint16, axi_mem_addr, &u16);
				HWA_PRINT("Get" AXI_MEM16_FMT "\n", axi_mem_addr, u16);
			} else {
				HWA_RD_MEM32("AXI_MEM", uint32, axi_mem_addr, &u32);
				HWA_PRINT("Get" AXI_MEM32_FMT "\n", axi_mem_addr, u32);
			}
		}
#if defined(BCMDBG)
	} else if (strcmp(argv[1], "dump") == 0) {
		// dhd -i eth1 cons "hwa dump [verbose] [dump_regs]"
		if (argc >= 3)
			verbose = atoi(argv[2]);
		if (argc >= 4)
			dump_regs = atoi(argv[3]);
		if (argc >= 5)
			dump_txfifo_shadow = atoi(argv[4]);
		hwa_dump(dev, NULL, HWA_DUMP_ALL, verbose, dump_regs, dump_txfifo_shadow, NULL);
#endif // endif
	} else if (strcmp(argv[1], "membytes") == 0) {
		// dhd -i eth1 cons "hwa membytes [0x28509000] [4] [32]"
		if (argc < 5) {
			HWA_PRINT("Wrong argument!\n");
			return;
		}

		axi_mem_addr = bcm_strtoul(argv[2], NULL, 0);
		size = bcm_strtoul(argv[3], NULL, 0);
		if (size != 2 && size != 4) {
			HWA_PRINT("Wrong argument!\n");
			return;
		}
		count = bcm_strtoul(argv[4], NULL, 0);
		/* GET */
		for (c_idx = 0; c_idx < count; c_idx++) {
			if (size == 2) {
				HWA_RD_MEM16("AXI_MEM", uint16, axi_mem_addr, &u16);
				HWA_PRINT("Get" AXI_MEM16_FMT "\n", axi_mem_addr, u16);
				axi_mem_addr += 2;
			} else {
				HWA_RD_MEM32("AXI_MEM", uint32, axi_mem_addr, &u32);
				HWA_PRINT("Get" AXI_MEM32_FMT "\n", axi_mem_addr, u32);
				axi_mem_addr += 4;
			}
		}
	} else {
		HWA_PRINT("Wrong argument!\n");
		return;
	}
}

int
BCMATTACHFN(hwa_probe)(struct hwa_dev *dev, uint irq, uint coreid, uint unit,
	osl_ext_task_t* thread, uint bus)
{
	int ret = BCME_OK;

	HWA_FTRACE(HWA00);

	HWA_ASSERT(dev != (hwa_dev_t*)NULL);

	if (!dev)
		return BCME_BADARG;

#ifdef HWA_DPC_BUILD
#ifndef RTE_POLL
	ret = hnd_isr_register(irq, coreid, unit, hwa_isr, hwa_dev,
		hwa_dpc_thread, hwa_dev, thread, bus);
	if (ret != BCME_OK) {
		HWA_ERROR(("%s: hnd_isr_register for hwa%d failed\n", HWA00, unit));
	}
#endif /* !RTE_POLL */

	dev->sys_dev = hnd_dpc_create(_hwa_dpctask, dev, NULL);
	if (dev->sys_dev == NULL) {
		return BCME_NORESOURCE;
	}
#endif /* HWA_DPC_BUILD */

	/* Add "hwa" dump function for debug */
	if (!hnd_cons_add_cmd("hwa", _hwa_dump, dev)) {
		HWA_ERROR(("%s: hnd_cons_add_cmd hwa%d failed\n", HWA00, unit));
	}

	if (hwa_wlc_module_register(dev)) {
		HWA_ERROR(("%s wlc_module_register() failed\n", HWA00));
		ret = BCME_ERROR;
	}

	return ret;
}

#ifdef HWA_DPC_BUILD
void
BCMATTACHFN(hwa_osl_detach)(struct hwa_dev *dev)
{
	if (dev->sys_dev == NULL)
		return;

	hnd_dpc_free(dev->sys_dev);
}
#endif /* HWA_DPC_BUILD */
