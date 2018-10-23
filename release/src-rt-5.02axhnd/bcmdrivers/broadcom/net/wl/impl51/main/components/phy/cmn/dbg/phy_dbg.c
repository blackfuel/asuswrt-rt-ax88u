/*
 * PHY module debug utilities
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
 * $Id: phy_dbg.c 729400 2017-10-31 18:34:19Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <wlc_dump_reg.h>
#include <phy_mem.h>
#include <phy_dbg_api.h>
#include "phy_type_dbg.h"
#include <phy_dbg.h>
#ifdef PHY_DUMP_BINARY
#include <phy_misc.h>
#include <phy_radio.h>
#include <phy_utils_reg.h>
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
#include <bcmdevs.h>
#include <phy_utils_reg.h>
#endif /* BCMDBG || WLTEST  */

typedef struct phy_dbg_mem phy_dbg_mem_t;

/* module private states */
struct phy_dbg_info {
	phy_info_t *pi;
	wlc_dump_reg_info_t *dump;
	phy_type_dbg_fns_t *fns;
	phy_dbg_mem_t *mem;
	/* PHY and radio dump list and sz */
	phyradregs_list_t *phyreglist;
	uint16 phyreglist_sz;
	phyradregs_list_t *radreglist;
	uint16 radreglist_sz;

	/*
	* The index,bitmap and cnt at which table read opration
	* had to be aborted due to lack of print buffer space.
	*/
	uint16 regval_last_write_index;
	uint32 regval_last_bitmap;
	uint32 regval_last_cnt;
	uint16 regval_last_addr;
	/*
	* Signals the data exchage between
	* the host and the driver. (e.g  more
	* data or read end.)
	*/
	uint16 control_flag;
	/* phy or radio? */
	uint8 type;
	/* use page to dump tables? */
	uint8 use_page;
};

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
#define PHY_DBG_NUM_PAGES 10
typedef struct phy_dbg_page_info {
	int start_index;
	int size;
} phy_dbg_page_info_t;

phy_dbg_page_info_t page_info[PHY_DBG_NUM_PAGES];
#endif // endif

/* module private states memory layout */
struct phy_dbg_mem {
	phy_dbg_info_t info;
	phy_type_dbg_fns_t fns;
/* add other variable size variables here at the end */
};

/* default registries sizes */
#ifndef PHY_DUMP_REG_SZ
#define PHY_DUMP_REG_SZ 32
#endif // endif

#if defined(BCMDBG_ERR) || defined(BCMDBG)
uint32 phyhal_msg_level = PHYHAL_ERROR;
#else
uint32 phyhal_msg_level = 0;
#endif // endif

#ifdef PHY_DUMP_BINARY
#define PHYRAD_DUMP_BMPCNT_MASK	0x80000000
#define SIZEOF_PHYRADREG_VALUE	2

/* Number of header bytes for rad/phy binary dump */
#define PHYRAD_BINDUMP_HDSZ	12

#define PRINT_BUFFER_SIZE 30
#endif // endif

/* module attach/detach */
phy_dbg_info_t *
BCMATTACHFN(phy_dbg_attach)(phy_info_t *pi)
{
	phy_dbg_info_t *di = NULL;
	phy_dbg_mem_t *mem;

	if ((mem = phy_malloc(pi, sizeof(phy_dbg_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	di = &(mem->info);
	di->mem = mem;
	di->pi = pi;
	di->dump = wlc_dump_reg_create(pi->sh->osh, PHY_DUMP_REG_SZ);
	if (di->dump == NULL) {
		PHY_ERROR(("%s: wlc_dump_reg_create failed\n", __FUNCTION__));
		goto fail;
	}
	di->fns = &(mem->fns);

	return di;

fail:
	phy_dbg_detach(di);
	return NULL;
}

void
BCMATTACHFN(phy_dbg_detach)(phy_dbg_info_t *di)
{
	phy_info_t *pi;
	phy_dbg_mem_t *mem;

	if (di == NULL)
		return;

	pi = di->pi;

	if (di->dump) {
		wlc_dump_reg_destroy(di->dump);
	}

	mem = di->mem;

	if (mem != NULL)
		phy_mfree(pi, mem, sizeof(phy_dbg_mem_t));
}

/* add a dump fn */
int
phy_dbg_add_dump_fns(phy_info_t *pi, const char *name,
	phy_dump_dump_fn_t fn, phy_dump_clr_fn_t clr, void *ctx)
{
	phy_dbg_info_t *di = pi->dbgi;

	return wlc_dump_reg_add_fns(di->dump, name, fn, clr, ctx);
}

/*
 * invoke dump function for name.
 */
int
phy_dbg_dump(phy_info_t *pi, const char *name, struct bcmstrbuf *b)
{
	phy_dbg_info_t *di = pi->dbgi;

	return wlc_dump_reg_invoke_dump_fn(di->dump, name, b);
}

/*
 * invoke dump function for name.
 */
int
phy_dbg_dump_clr(phy_info_t *pi, const char *name)
{
	phy_dbg_info_t *di = pi->dbgi;

	return wlc_dump_reg_invoke_clr_fn(di->dump, name);
}

/*
 * List dump names
 */
int
phy_dbg_dump_list(phy_info_t *pi, struct bcmstrbuf *b)
{
	phy_dbg_info_t *di = pi->dbgi;

	return wlc_dump_reg_list(di->dump, b);
}

/*
 * Dump dump registry internals
 */
int
phy_dbg_dump_dump(phy_info_t *pi, struct bcmstrbuf *b)
{
	phy_dbg_info_t *di = pi->dbgi;

	return wlc_dump_reg_dump(di->dump, b);
}

/*
 * Register/unregister PHY type implementation to the Debug module.
 * It returns BCME_XXXX.
 */
int
phy_dbg_register_impl(phy_dbg_info_t *di, phy_type_dbg_fns_t *fns)
{
	*(di->fns) = *fns;

	return BCME_OK;
}

void
phy_dbg_unregister_impl(phy_dbg_info_t *di)
{
	BCM_REFERENCE(di);
	/* nothing to do at this moment */
}

/* dump txerr */
void
phy_dbg_txerr_dump(phy_info_t *pi, uint16 err)
{
	phy_dbg_info_t *di = pi->dbgi;
	phy_type_dbg_fns_t *fns = di->fns;

	if (fns->txerr_dump != NULL) {
		(fns->txerr_dump)(fns->ctx, err);
	}
}

#if defined(DNG_DBGDUMP)
void wlc_phy_print_phydbg_regs(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_dbg_fns_t *fns = pi->dbgi->fns;
	if (fns->print_phydbg_regs != NULL) {
		(fns->print_phydbg_regs)(fns->ctx);
	}
}
#endif /* DNG_DBGDUMP */

#if defined(BCMDBG) || defined(WL_MACDBG)
/* dump txerr */
void
phy_dbg_gpio_out_enab(phy_info_t *pi, bool enab)
{
	phy_dbg_info_t *di = pi->dbgi;
	phy_type_dbg_fns_t *fns = di->fns;

	if (fns->gpio_out_enab != NULL) {
		(fns->gpio_out_enab)(fns->ctx, enab);
	}
}
#endif // endif

void phy_fatal_error(phy_info_t *pi, phy_crash_reason_t reason_code)
{
	pi->phy_crash_rc = reason_code;
	wlapi_wlc_fatal_error(pi->sh->physhim, (uint32)reason_code);
}

#ifdef AWD_EXT_TRAP
int phy_dbg_dump_ext_trap(phy_info_t *pi, phy_dbg_ext_trap_err_t *ext_trap_data)
{
	phy_dbg_info_t *di = pi->dbgi;
	phy_type_dbg_fns_t *fns = di->fns;

	ext_trap_data->err = pi->phy_crash_rc;
	if (pi->phy_crash_rc != PHY_RC_NONE && fns->ext_trap != NULL) {
		(fns->ext_trap)(fns->ctx, ext_trap_data);
	}

	return pi->phy_crash_rc;
}
#endif /* AWD_EXT_TRAP */

#ifdef PHY_DUMP_BINARY
static uint16
phy_dbg_dump_tblsz(phy_info_t *pi, phyradregs_list_t *list, uint16 totalsize)
{
	uint16 binsz, i;

	binsz = totalsize;
	for (i = 0; i < (totalsize/sizeof(*list)); i++) {
		uint32 bmp_cnt = (list[i].bmp_cnt[0] << 24) |
			(list[i].bmp_cnt[1] << 16) |
			(list[i].bmp_cnt[2] << 8) |
			(list[i].bmp_cnt[3]);

		if (bmp_cnt & PHYRAD_DUMP_BMPCNT_MASK) {
			binsz += (bmp_cnt & (~PHYRAD_DUMP_BMPCNT_MASK)) *
					SIZEOF_PHYRADREG_VALUE;
		} else {
			binsz += bcm_bitcount((uint8 *)&(bmp_cnt), sizeof(bmp_cnt)) *
					SIZEOF_PHYRADREG_VALUE;
		}
	}
	return binsz;
}
#endif /* PHY_DUMP_BINARY */

int
phy_dbg_dump_getlistandsize(wlc_phy_t *ppi, uint8 type)
{
#ifdef PHY_DUMP_BINARY
	phy_info_t *pi = (phy_info_t*)ppi;
	uint16 binsz = PHYRAD_BINDUMP_HDSZ;
	int ret = BCME_OK;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	if ((type != TAG_TYPE_PHY) && (type != TAG_TYPE_RAD)) {
		PHY_INFORM(("%s: wl%d: unsupported tag for phy and radio dump %d\n",
			__FUNCTION__,  pi->sh->unit, type));
		return BCME_BADARG;
	}

	if (type == TAG_TYPE_PHY) {
		pi->dbgi->phyreglist = NULL;
		pi->dbgi->phyreglist_sz = 0;
		if ((ret = phy_misc_getlistandsize(pi, &(pi->dbgi->phyreglist),
				&(pi->dbgi->phyreglist_sz))) != BCME_OK) {
			return ret;
		}
		/* Calculate the total binary size */
		binsz += phy_dbg_dump_tblsz(pi, pi->dbgi->phyreglist, pi->dbgi->phyreglist_sz);
	} else if (type == TAG_TYPE_RAD) {
		pi->dbgi->radreglist = NULL;
		pi->dbgi->radreglist_sz = 0;
		if ((ret = phy_radio_getlistandsize(pi, &(pi->dbgi->radreglist),
				&(pi->dbgi->radreglist_sz))) != BCME_OK) {
			return ret;
		}
		/* Calculate the total binary size */
		binsz += phy_dbg_dump_tblsz(pi, pi->dbgi->radreglist, pi->dbgi->radreglist_sz);
	}

	/* Last check for bad length calculation and header addition */
	if (binsz == PHYRAD_BINDUMP_HDSZ)
		return BCME_BADLEN;

	return (int)binsz;
#else /* PHY_DUMP_BINARY */
	return BCME_UNSUPPORTED;
#endif /* PHY_DUMP_BINARY */
}

#if defined(WLC_SRAMPMAC) && defined(PHY_DUMP_BINARY)
void
phy_dbg_regval_set_type(phy_dbg_info_t *dbgi, uint8 type)
{
	if (type == WLC_REGVAL_DUMP_PHYREG) {
		dbgi->type = TAG_TYPE_PHY;
	} else if (type == WLC_REGVAL_DUMP_RADREG) {
		dbgi->type = TAG_TYPE_RAD;
	}
}

int
phy_dbg_regval_get(phy_dbg_info_t *dbgi, void *data, int length)
{
	phy_info_t *pi = dbgi->pi;
	wl_regval_capture_args_t *capture_args = (wl_regval_capture_args_t *) data;
	int ret = BCME_OK;
	int buffer_length_bytes = length - sizeof(*capture_args)
		- (sizeof("wlc:") - 1) - 4;
	void *buffer = data + sizeof(*capture_args);

	/* Disable paging. */
	dbgi->use_page = 0;

	if (dbgi->control_flag != WLC_REGVAL_MORE_DATA)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	ret = phy_dbg_dump_tbls((wlc_phy_t *)pi,
		dbgi->type, buffer, &buffer_length_bytes, FALSE);
	capture_args->control_flag = dbgi->control_flag;

	if (dbgi->control_flag != WLC_REGVAL_MORE_DATA)
		wlapi_enable_mac(pi->sh->physhim);

	return ret;
}
#endif /* PHY_DUMP_BINARY && WLC_SRAMPMAC */

int
phy_dbg_dump_tbls(wlc_phy_t *ppi, uint8 type, uchar *p, int * buf_len, bool fatal_dump)
{
#ifdef PHY_DUMP_BINARY
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_dbg_info_t *di = pi->dbgi;
	phy_type_dbg_fns_t *fns = di->fns;
	phyradregs_list_t *pregs, *preglist;
	uint i = 0;
	uint size;
	uint16 addr;
	uint16	val_16 = 0;
	uint array_size = 0;
	int rem_len = *buf_len;
	uint8 corenum;
	struct bcmstrbuf b;
	struct bcmstrbuf *bptr = &b;
	int num_printed_char = 0;
	char print_buffer[PRINT_BUFFER_SIZE];
	const char *fmtstr;

	corenum = (uint8) PHYCORENUM(pi->pubpi->phy_corenum);

	if (phy_dbg_dump_getlistandsize(ppi, type) < 0)
		return BCME_NOTREADY;

	if (fatal_dump) {
		if (rem_len < PHYRAD_BINDUMP_HDSZ) {
			PHY_INFORM(("%s: wl%d: \n buffer too short \n",
				__FUNCTION__, pi->sh->unit));
			return BCME_BUFTOOSHORT;
		}

		if (type == TAG_TYPE_PHY) {
			*(uint16 *)p = TAG_TYPE_PHY;
			*(uint16 *)(p + 2) = (uint16) pi->pubpi->phy_type;
			*(uint16 *)(p + 4) = (uint16) pi->pubpi->phy_rev;
			*(uint16 *)(p + 6) = PHYRADDBG1_TYPE;
			preglist = pi->dbgi->phyreglist;
			array_size = pi->dbgi->phyreglist_sz / sizeof(*pi->dbgi->phyreglist);
			PHY_INFORM(("\n %s: phytype: %d phyrev: %d structtype: %d\n", __FUNCTION__,
					pi->pubpi->phy_type, pi->pubpi->phy_rev, PHYRADDBG1_TYPE));
		} else if (type == TAG_TYPE_RAD) {
			*(uint16 *)p = TAG_TYPE_RAD;
			*(uint16 *)(p + 2) = pi->pubpi->radioid;
			*(uint16 *)(p + 4) = (pi->pubpi->radiorev);
			*(uint16 *)(p + 6) = PHYRADDBG1_TYPE;
			preglist = pi->dbgi->radreglist;
			array_size = pi->dbgi->radreglist_sz / sizeof(*pi->dbgi->radreglist);
			PHY_INFORM(("\n %s: radioid: %d wlc->band->radiorev: %d structtype: %d\n",
				__FUNCTION__, pi->pubpi->radioid,
				pi->pubpi->radiorev, PHYRADDBG1_TYPE));
		} else {
			PHY_INFORM(("%s: unsupported type in function\n", __FUNCTION__));
				return BCME_UNSUPPORTED;
		}

		*(uint16 *)(p + 8) = (uint16) array_size;
		*(uint16 *)(p + 10) = (uint16) corenum;
		PHY_INFORM(("\n %s: arraysize: %d corenum: %d \n",
			__FUNCTION__, array_size, corenum));

		p += PHYRAD_BINDUMP_HDSZ;
		rem_len -= PHYRAD_BINDUMP_HDSZ;
		PHY_INFORM(("\n %s: sizeof(phyradregs_list_t)=%d \n",
			__FUNCTION__, (int) sizeof(phyradregs_list_t)));
		size = array_size;
	} else {
		/* Init the buffer. */
		bcm_binit(bptr, (char *)p, *buf_len);

		/* Initialize the array. */
		if (type == TAG_TYPE_PHY) {
			preglist = pi->dbgi->phyreglist;
			array_size = pi->dbgi->phyreglist_sz / sizeof(*pi->dbgi->phyreglist);
		} else {
			preglist = pi->dbgi->radreglist;
			array_size = pi->dbgi->radreglist_sz / sizeof(*pi->dbgi->radreglist);
		}

		/* Restore i to the last accessed index */
		if (pi->dbgi->control_flag == WLC_REGVAL_MORE_DATA) {
			i = pi->dbgi->regval_last_write_index;
		}
		size = array_size;

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
		if (pi->dbgi->use_page) {
			i = page_info[pi->pdpi->page].start_index;
			size = page_info[pi->pdpi->page].size;
		}
#endif // endif
		memset(print_buffer, 0, PRINT_BUFFER_SIZE);
	}

	for (; i < size; i++) {
		uint32 bitmap = 0, cnt = 0, bmp_cnt;

		pregs = &preglist[i];
		addr = pregs->addr;

		if (fatal_dump) {
			if (rem_len < (int)sizeof(phyradregs_list_t)) {
				return BCME_BUFTOOSHORT;
			}
			bcopy(pregs, p, sizeof(phyradregs_list_t));
			p += sizeof(phyradregs_list_t);
			rem_len -= sizeof(phyradregs_list_t);
		}

		/* bmp_cnt has either bitmap or count, if the MSB is set, then
		*   bmp_cnt[30:0] has count, i.e, number of registers who values are
		*   contigous from the start address. If MSB is zero, then the value
		*   should be considered as a bitmap of 31 discreet addresses
		*/
		bmp_cnt = (pregs->bmp_cnt[0] << 24) | (pregs->bmp_cnt[1] << 16) |
				(pregs->bmp_cnt[2] << 8) | (pregs->bmp_cnt[3]);

		if (bmp_cnt & PHYRAD_DUMP_BMPCNT_MASK)
			cnt = (bmp_cnt) & (~PHYRAD_DUMP_BMPCNT_MASK);
		else
			bitmap = (bmp_cnt) & (~PHYRAD_DUMP_BMPCNT_MASK);

		if (!fatal_dump) {
			/* Continue from last accessed location? */
			if (pi->dbgi->control_flag == WLC_REGVAL_MORE_DATA)
			{
				/*
				* Restore the bitmap, cnt and addr.
				*/
				bitmap = pi->dbgi->regval_last_bitmap;
				cnt = pi->dbgi->regval_last_cnt;
				addr = pi->dbgi->regval_last_addr;
			}
		}

		while (bitmap || cnt) {
			if ((cnt) || (bitmap & 0x1)) {
				if (fatal_dump && (rem_len < (int)sizeof(val_16))) {
					return BCME_BUFTOOSHORT;
				}

				if (type == TAG_TYPE_PHY) {
					/*
					 * The TableDataWide register is only valid to read once the
					 * TableID and TableOffset registers are set.
					 */
					if (fns->phyregaddr != NULL) {
						val_16 = (fns->phyregaddr)(fns->ctx, addr);
					} else {
						val_16 = phy_utils_read_phyreg(pi, addr);
					}
					PHY_INFORM(("\n %s : phyreg addr=0x%x, val=0x%x, "
						"rem_len=%d\n",
						__FUNCTION__, addr, val_16, rem_len));
				} else if (type == TAG_TYPE_RAD) {
					val_16 = phy_utils_read_radioreg(pi, addr);
					PHY_INFORM(("\n %s : radioreg addr=0x%x, val=0x%x, "
						"rem_len=%d\n", __FUNCTION__,
						addr, val_16, rem_len));
				}

				if (fatal_dump) {
					*(uint16 *)(p) = val_16;
					p += sizeof(val_16);
					rem_len -= sizeof(val_16);
				} else {

					if (addr < 0x1000) {
						fmtstr = "%03x %04x\n";
					} else {
						fmtstr = "%04x %04x\n";
					}

					/* Check if buffer has enough space to write. */
					num_printed_char = snprintf(print_buffer,
						PRINT_BUFFER_SIZE, fmtstr, addr, val_16);

					if (num_printed_char < BCMSTRBUF_LEN(bptr)) {
						/* There is space in buffer to print this string. */
						bcm_bprintf(bptr, print_buffer);
					} else {
						/*
						* The string wont fit into the buffer.
						* Save the state variables for next pass.
						* Request for a new bufffer.
						*/
						pi->dbgi->regval_last_write_index = i;
						pi->dbgi->regval_last_bitmap = bitmap;
						pi->dbgi->regval_last_cnt = cnt;
						pi->dbgi->regval_last_addr = addr;

						/* Indicate to host that more data is pending. */
						pi->dbgi->control_flag = WLC_REGVAL_MORE_DATA;
						return BCME_OK;
					}
				}
			} /* if block */

			/* Decrement the count or delete bitmap bits */
			if (cnt)
				cnt--;
			else
				bitmap = bitmap >> 1;

			++addr;
		} /* end while */

		if (!fatal_dump) {
			/* clean up the state variables. */
			pi->dbgi->regval_last_write_index = 0;
			pi->dbgi->regval_last_bitmap = 0;
			pi->dbgi->regval_last_cnt = 0;
			pi->dbgi->regval_last_addr = 0;

			/*
			* Proceed to the next element in the for loop.
			* No need to restore the bitmap/cnt/addr.
			*/
			pi->dbgi->control_flag = WLC_REGVAL_READ_CONTINUE;
		}
	} /* end for */

	if (fatal_dump) {
		*buf_len = *buf_len - rem_len;
	} else {
		/* Indicate end of read to host. */
		pi->dbgi->control_flag = WLC_REGVAL_READ_END;
	}
	return BCME_OK;
#else /* PHY_DUMP_BINARY */
	return BCME_UNSUPPORTED;
#endif /* PHY_DUMP_BINARY */
}

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)

/* Sets up the page based on buffer length. */
int
phy_dbg_page_parser(phy_info_t *pi, uint8 type, int buf_len)
{
#ifdef PHY_DUMP_BINARY
	const char *fmtstr = "0000 0000\n";
	int bytes_consumed = 0;
	phyradregs_list_t *preglist, *pregs;
	int array_size = 0;
	int i;
	int page_index = 0;
	int space_remaining = buf_len;

	phy_dbg_dump_getlistandsize((wlc_phy_t *)pi, type);

	if (type == TAG_TYPE_PHY) {
		preglist = pi->dbgi->phyreglist;
		array_size = pi->dbgi->phyreglist_sz / sizeof(*pi->dbgi->phyreglist);
	} else {
		preglist = pi->dbgi->radreglist;
		array_size = pi->dbgi->radreglist_sz / sizeof(*pi->dbgi->radreglist);
	}

	for (i = 0; i < PHY_DBG_NUM_PAGES; i++) {
		page_info[i].start_index = 0;
		page_info[i].size = 0;
	}

	bytes_consumed = strlen(fmtstr);
	page_info[page_index].start_index = 0;

	/* Iterate over the array to initialize the pages */
	for (i = 0; i < array_size; i++) {
		uint32 bitmap = 0, cnt = 0, bmp_cnt;
		pregs = &preglist[i];
		bmp_cnt = (pregs->bmp_cnt[0] << 24) | (pregs->bmp_cnt[1] << 16) |
			(pregs->bmp_cnt[2] << 8) | (pregs->bmp_cnt[3]);
		if (bmp_cnt & PHYRAD_DUMP_BMPCNT_MASK) {
			cnt = (bmp_cnt) & (~PHYRAD_DUMP_BMPCNT_MASK);
		} else {
			bitmap = (bmp_cnt) & (~PHYRAD_DUMP_BMPCNT_MASK);
		}

		while (bitmap || cnt) {
			if ((cnt) || (bitmap & 0x1)) {
				space_remaining -= bytes_consumed;
				if (space_remaining < 0) {
					/* Reinit space to buffer size. */
					space_remaining = buf_len;
					/*
					* The current index wont fit into the buffer.
					* Set current index as the size.
					*/
					page_info[page_index].size = i;

					/* Go to next page. */
					page_index++;
					/* Set start index for the new page. */
					page_info[page_index].start_index = i;

					/* Break and restart form the current index. */
					bitmap = 0;
					cnt = 0;
					i = i - 1;
				}
			}
			if (cnt) {
				cnt--;
			} else {
				bitmap = bitmap >> 1;
			}
		} /* end while */
	} /* end for */
	if (space_remaining >= 0) {
		/* end index for very last index. */
		page_info[page_index].size = i;
	}
	pi->dbgi->use_page = 1;

	return BCME_OK;
#else /* #ifdef PHY_DUMP_BINARY */
	return BCME_UNSUPPORTED;
#endif /* #ifdef PHY_DUMP_BINARY */
}
#endif /* #if ... */
/*
* ((defined(BCMDBG) || defined(BCMDBG_DUMP))
* && (defined(BCMINTERNAL) || defined(DBG_PHY_IOV)))
* || defined(BCMDBG_PHYDUMP)
*/

#if defined(BCMDBG) || defined(WLTEST)
void
phy_dbg_test_evm_init(phy_info_t *pi)
{
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL) {
		PHY_INFORM(("wl%d: %s: PACTRL boardflag set, clearing gpio 0x%04x\n",
			pi->sh->unit, __FUNCTION__, BOARD_GPIO_PACTRL));
		/* Store initial values */
		pi->evm_o = R_REG(pi->sh->osh, D11_PSM_GPIOOUT(pi));
		pi->evm_oe = R_REG(pi->sh->osh, D11_PSM_GPIOEN(pi));
		AND_REG(pi->sh->osh, D11_PSM_GPIOOUT(pi), ~BOARD_GPIO_PACTRL);
		OR_REG(pi->sh->osh, D11_PSM_GPIOEN(pi), BOARD_GPIO_PACTRL);
		OSL_DELAY(1000);
	}
}

uint16
phy_dbg_test_evm_reg(uint rate)
{
	uint16 reg = TST_TXTEST_RATE_2MBPS;
	switch (rate) {
	case 2:
		reg = TST_TXTEST_RATE_1MBPS;
		break;
	case 4:
		reg = TST_TXTEST_RATE_2MBPS;
		break;
	case 11:
		reg = TST_TXTEST_RATE_5_5MBPS;
		break;
	case 22:
		reg = TST_TXTEST_RATE_11MBPS;
		break;
	}
	return ((reg << TST_TXTEST_RATE_SHIFT) & TST_TXTEST_RATE);
}

/*
 * Rate is number of 500 Kb units.
 */
int
phy_dbg_test_evm(phy_info_t *pi, int channel, uint rate, int txpwr)
{
	phy_type_dbg_fns_t *fns = pi->dbgi->fns;
	int err = BCME_UNSUPPORTED;

	if (fns->test_evm != NULL) {
		err = (fns->test_evm)(fns->ctx, channel, rate, txpwr);
	} else {
		uint16 reg = 0;
		int bcmerror = 0;

		/* stop any test in progress */
		wlc_phy_test_stop(pi);

		/* channel 0 means restore original contents and end the test */
		if (channel == 0) {
			W_REG(pi->sh->osh, D11_PHY_REG_A(pi), pi->evm_phytest);

			pi->evm_phytest = 0;

			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL) {
				W_REG(pi->sh->osh, D11_PSM_GPIOOUT(pi), pi->evm_o);
				W_REG(pi->sh->osh, D11_PSM_GPIOEN(pi), pi->evm_oe);
				OSL_DELAY(1000);
			}
			return 0;
		}

		phy_dbg_test_evm_init(pi);

		if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
			return bcmerror;

		reg = phy_dbg_test_evm_reg(rate);

		PHY_INFORM(("wlc_evm: rate = %d, reg = 0x%x\n", rate, reg));

		/* Save original contents */
		if (pi->evm_phytest == 0) {
			pi->evm_phytest = R_REG(pi->sh->osh, D11_PHY_REG_A(pi));
		}

		/* Set EVM test mode */
		AND_REG(pi->sh->osh, D11_PHY_REG_A(pi),
		        ~(TST_TXTEST_ENABLE|TST_TXTEST_RATE|TST_TXTEST_PHASE));
		OR_REG(pi->sh->osh, D11_PHY_REG_A(pi), TST_TXTEST_ENABLE | reg);
		err = BCME_OK;
	}
	return err;
}

int
phy_dbg_test_carrier_suppress(phy_info_t *pi, int channel)
{
	phy_type_dbg_fns_t *fns = pi->dbgi->fns;
	int err = BCME_UNSUPPORTED;

	if (fns->test_carrier_suppress != NULL) {
		err = (fns->test_carrier_suppress)(fns->ctx, channel);
	} else {
		int bcmerror = 0;

		/* stop any test in progress */
		wlc_phy_test_stop(pi);

		/* channel 0 means restore original contents and end the test */
		if (channel == 0) {
			W_REG(pi->sh->osh, D11_PHY_REG_A(pi), pi->car_sup_phytest);

			pi->car_sup_phytest = 0;
			return 0;
		}

		if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
			return bcmerror;

		/* Save original contents */
		if (pi->car_sup_phytest == 0) {
			pi->car_sup_phytest = R_REG(pi->sh->osh, D11_PHY_REG_A(pi));
		}

		/* set carrier suppression test mode */
		AND_REG(pi->sh->osh, D11_PHY_REG_A(pi), 0xfc00);
		OR_REG(pi->sh->osh, D11_PHY_REG_A(pi), 0x0228);
		err = BCME_OK;
	}
	return err;
}
#endif /* BCMDBG || WLTEST  */
