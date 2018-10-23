/*
 * Common code for wl command line utility
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
 * $Id: wlu.h 733321 2017-11-28 01:37:08Z $
 */

#ifndef _wlu_h_
#define _wlu_h_

#include <bcmipv6.h>
#include "wlu_cmd.h"

typedef struct {
	uint value;
	const char *string;
} dbg_msg_t;

typedef struct {
	int8 pwr2p5;
	int8 pwr5;
	int8 pwr10;
	int8 pwr20;
	int8 pwr40;
	int8 pwr20in40;
	int8 pwr80;
	int8 pwr20in80;
	int8 pwr40in80;
	int8 pwr160;
	int8 pwr20in160;
	int8 pwr40in160;
	int8 pwr80in160;
	int8 pwr8080;
	int8 pwr8080chan2;
	int8 pwr20in8080;
	int8 pwr40in8080;
	int8 pwr80in8080;
} txpwr_row_t;

extern const char *wlu_av0;
extern int g_wlc_idx;

struct escan_bss {
	struct escan_bss *next;
	uint8 bss[];
};

/* parse common option */
extern int wl_option(char ***pargv, char **pifname, int *phelp);
extern void wl_cmd_init(void);
extern void wlu_init(void);

/* print usage */
extern void wl_usage(FILE *fid, cmd_t *port_cmds);
extern void wl_cmds_usage(FILE *fid, cmd_t *port_cmds);

/* print helpers */
extern void wl_printlasterror(void *wl);
extern void wl_printint(int val);

/* check driver version */
extern int wl_check(void *wl);

/* returns major version from "wlc_ver" command */
extern int wlc_ver_major(void * wl);

extern void dump_bss_info(void *bi_generic);
extern cmd_func_t wl_phy_rate;
extern cmd_func_t wl_sta_report;
extern cmd_func_t wl_sta_supp_chan;
extern cmd_func_t wl_seq_start;
extern cmd_func_t wl_seq_stop;
extern cmd_func_t wl_bsscfg_int;
extern cmd_func_t wl_bcm_config;

/* return global ioctl_version */
extern int wl_get_ioctl_version(void);
/* return the address of bufstruct_wlu, global buf */
extern char *wl_get_buf(void);

extern chanspec_t wl_chspec_to_driver(chanspec_t chanspec);
extern chanspec_t wl_chspec_from_driver(chanspec_t chanspec);
extern uint32 wl_chspec32_to_driver(chanspec_t chanspec);
extern chanspec_t wl_chspec32_from_driver(uint32 chanspec32);
/* return TRUE if all the values in the array are uniformly the same */
extern int wl_array_uniform(uint8 *pwr, int start, int count);
extern void wl_txpwr_print_header(int8 channel_bandwidth, bool verbose);
extern void wl_txpwr_print_row(const char *label, uint8 chains, txpwr_row_t powers,
	int8 unsupported_rate, int8 channel_bandwidth, bool verbose);
extern int get_oui_bytes(uchar *oui_str, uchar *oui);

/* Format a ratespec for output of any of the wl_rate() iovars */
extern char* wl_rate_print(char *rate_buf, uint32 rspec);
/* convert rate string in Mbit/s format, like "11", "5.5", to internal 500 Kbit/s units */
extern int rate_string2int(char *s);
/* convert rate internal 500 Kbits/s units to string in Mbits/s format, like "11", "5.5" */
extern char* rate_int2string(char *rate_buf, int val);

extern int wlu_reg3args(void *wl, cmd_t *cmd, char **argv);

/* register commands for a module */
extern void wl_module_cmds_register(cmd_t *cmds);

extern cmd_t *wlu_find_cmd(char *name);

/* wluc_module init functions */
extern void wluc_common_module_init(void);
extern void wluc_phy_module_init(void);
extern void wluc_wnm_module_init(void);
extern void wluc_cac_module_init(void);
extern void wluc_rmc_module_init(void);
extern void wluc_rrm_module_init(void);
extern void wluc_wowl_module_init(void);
extern void wluc_nan_module_init(void);
extern void wluc_ap_module_init(void);
extern void wluc_ampdu_module_init(void);
extern void wluc_ampdu_cmn_module_init(void);
extern void wluc_bmac_module_init(void);
extern void wluc_ht_module_init(void);
extern void wluc_wds_module_init(void);
extern void wluc_keymgmt_module_init(void);
extern void wluc_scan_module_init(void);
extern void wluc_obss_module_init(void);
extern void wluc_prot_obss_module_init(void);
extern void wluc_lq_module_init(void);
extern void wluc_seq_cmds_module_init(void);
extern void wluc_btcx_module_init(void);
extern void wluc_led_module_init(void);
extern void wluc_interfere_module_init(void);
extern void wluc_ltecx_module_init(void);
extern void wluc_pkt_filter_module_init(void);
extern void wluc_mfp_module_init(void);
extern void wluc_ota_module_init(void);
extern void wluc_bssload_module_init(void);
extern void wluc_sdio_module_init(void);
extern void wluc_stf_module_init(void);
extern void wluc_offloads_module_init(void);
extern void wluc_tpc_module_init(void);
extern void wluc_toe_module_init(void);
extern void wluc_arpoe_module_init(void);
extern void wluc_ndoe_module_init(void);
extern void wluc_keep_alive_module_init(void);
extern void wluc_pfn_module_init(void);
extern void wluc_tbow_module_init(void);
extern void wluc_p2p_module_init(void);
extern void wluc_tdls_module_init(void);
extern void wluc_trf_mgmt_module_init(void);
extern void wluc_proxd_module_init(void);
extern void wluc_p2po_module_init(void);
extern void wluc_anqpo_module_init(void);
extern void wluc_btcdyn_module_init(void);
extern void wluc_mesh_module_init(void);
extern void wluc_msch_module_init(void);
extern void wluc_bdo_module_init(void);
extern void wluc_randmac_module_init(void);
extern void wluc_tko_module_init(void);
extern void wluc_natoe_module_init(void);
extern void wluc_rsdb_module_init(void);
extern void wluc_he_module_init(void);
extern void wluc_twt_module_init(void);
extern void wluc_mbo_module_init(void);
extern void wluc_hoffload_module_init(void);
extern void wluc_slot_bss_module_init(void);
extern void wluc_tdmtx_module_init(void);
extern void wluc_oce_module_init(void);
extern void wluc_esp_module_init(void);

extern void wluc_ecounters_module_init(void);
extern int hexstrtobitvec(const char *cp, uchar *bitvec, int veclen);

extern void wluc_leakyapstats_module_init(void);
extern void wluc_pwropt_module_init(void);
extern void wluc_tvpm_module_init(void);

extern int get_counter_offset(char *name, uint32 *offset, uint8 counter_ver);
extern int print_counter_help(uint8 counter_ver);

extern void wluc_btl_module_init(void);

/* wl functions used by the ndis wl. */
struct ipv4_addr;	/* forward declaration */
extern cmd_func_t wl_int;

extern void wl_printlasterror(void *wl);
extern bool wc_cmd_check(const char *cmd);
extern int fsize(void *fp);

#define WL_IOV_BATCH_DELIMITER		"+"

extern void wluc_heb_module_init(void);
/* functions for downloading firmware to a device via serial or other transport */
#ifdef SERDOWNLOAD
extern int dhd_init(void *dhd, cmd_t *cmd, char **argv);
extern int dhd_download(void *dhd, cmd_t *cmd, char **argv);
extern int rwl_download(void *dhd, cmd_t *cmd, char **argv);
#endif /* SERDOWNLOAD */

extern void wluc_adps_module_init(void);
extern void wluc_rpsnoa_module_init(void);
extern void wluc_bam_module_init(void);

#ifdef BCMDLL
#ifdef LOCAL
extern FILE *dll_fd;
#else
extern void * dll_fd_out;
extern void * dll_fd_in;
#endif // endif
#undef printf
#undef fprintf
#define printf printf_to_fprintf	/* printf to stdout */
#define fprintf fprintf_to_fprintf	/* fprintf to stderr */
extern void fprintf_to_fprintf(FILE * stderror, const char *fmt, ...);
extern void printf_to_fprintf(const char *fmt, ...);
extern void raw_puts(const char *buf, void *dll_fd_out);
#define	fputs(buf, stdout) raw_puts(buf, dll_fd_out)
#endif /* BCMDLL */

#define	PRNL()		pbuf += sprintf(pbuf, "\n")

#define RAM_SIZE_4325  0x60000
#define RAM_SIZE_4329  0x48000
#define RAM_SIZE_43291 0x60000
#define RAM_SIZE_4330_a1  0x3c000
#define RAM_SIZE_4330_b0  0x48000

#define SROM_PAVAR		21

/* useful macros */
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#endif /* ARRAYSIZE */

/* buffer length needed for wl_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL
 */
#ifndef SSID_FMT_BUF_LEN
#define SSID_FMT_BUF_LEN (4*32+1)	/* Length for SSID format string */
#endif // endif

/* some OSes (FC4) have trouble allocating (kmalloc) 128KB worth of memory,
 * hence keeping WL_DUMP_BUF_LEN below that
 */
#if !defined(WL_DUMP_BUF_LEN)
#if defined(BWL_SMALL_WLU_DUMP_BUF)
#define WL_DUMP_BUF_LEN (4 * 1024)
#else
#define WL_DUMP_BUF_LEN (255 * 1024)
#endif /* BWL_SMALL_WLU_DUMP_BUF */
#endif /* WL_DUMP_BUF_LEN */

#ifdef linux
#define ESCAN_EVENTS_BUFFER_SIZE 2048
#endif /* linux */

#define CMDLINESZ		80
#define USAGE_ERROR  -1		/* Error code for Usage */
#define CMD_DEPRECATED -4 /* Commands that are functionally deprecated or don't provide
			   * a useful value to a specific OS port of wl
			   */

/* integer output format */
#define INT_FMT_DEC	0	/* signed integer */
#define INT_FMT_UINT	1	/* unsigned integer */
#define INT_FMT_HEX	2	/* hexdecimal */

#define DIV_QUO(num, div) ((num)/div)  /* Return the quotient of division to avoid floats */
#define DIV_REM(num, div) (((num%div) * 100)/div) /* Return the remainder of division */

/* command line argument usage */
#define CMD_ERR	-1	/* Error for command */
#define CMD_OPT	0	/* a command line option */
#define CMD_WL	1	/* the start of a wl command */

#define LED_MAX_INDEX	16	/* index limitation for cmd ledbh */

#define SCAN_USAGE_TYPE "\t-t ST, --scan_type=ST\t[active|passive|prohibit|offchan|hotspot] "\
"scan type\n"

#define SCAN_USAGE_TX ""

#define SCAN_USAGE	"" \
"\tDefault to an active scan across all channels for any SSID.\n" \
"\tOptional arg: SSIDs, list of [up to 10] SSIDs to scan (comma or space separated).\n" \
"\tOptions:\n" \
"\t-s S, --ssid=S\t\tSSIDs to scan\n" \
SCAN_USAGE_TYPE \
"\t--bss_type=BT\t\t[bss/infra|ibss/adhoc] bss type to scan\n" \
"\t-b MAC, --bssid=MAC\tparticular BSSID MAC address to scan, xx:xx:xx:xx:xx:xx\n" \
"\t-n N, --nprobes=N\tnumber of probes per scanned channel\n" \
"\t-a N, --active=N\tdwell time per channel for active scanning\n" \
"\t-p N, --passive=N\tdwell time per channel for passive scanning\n" \
"\t-h N, --home=N\t\tdwell time for the home channel between channel scans\n" \
"\t-c L, --chanspecs=L\tcomma or space separated list of chanspecs to scan" \
SCAN_USAGE_TX

#define WL_EVENT_TIMEOUT 10 /* Timeout in second for event from driver */
#define MAX_SUBCOUNTER_SUPPORTED	64 /* Max subcounters supported. */

#define DINGO_FW_REV 0x9000000

/* Number of legacy TX FIFOs. The value should match the one in d11.h */
#define NFIFO_LEGACY	6

#define BCM_CONFIG_ARRAY_SIZE 10

#define OUI_STR_SIZE	8	/* OUI string size */
#define MAX_OUI_SIZE	3	/* MAX  OUI size */
#define MAX_BYTE_CHARS	2	/* MAX num chars */
#define MAX_DATA_COLS	16	/* MAX data cols */

#define RADIO_CORE_SYN                           (0x0 << 12)
#define RADIO_CORE_TX0                           (0x2 << 12)
#define RADIO_CORE_TX1                           (0x3 << 12)
#define RADIO_CORE_RX0                           (0x6 << 12)
#define RADIO_CORE_RX1                           (0x7 << 12)
#define RADIO_CORE_CR0                           (0x0 << 10)
#define RADIO_CORE_CR1                           (0x1 << 10)
#define RADIO_CORE_CR2                           (0x2 << 10)
#define RADIO_CORE_ALL                           (0x3 << 10)
#define RADIO_2069_CORE_CR0                      (0x0 << 9)
#define RADIO_2069_CORE_CR1                      (0x1 << 9)
#define RADIO_2069_CORE_CR2                      (0x2 << 9)
#define RADIO_2069_CORE_ALL                      (0x3 << 9)
#define RADIO_2069_CORE_PLL                      (0x4 << 9)
#define RADIO_2069_CORE_PLL0                     (0x4 << 9)
#define RADIO_2069_CORE_PLL1                     (0x5 << 9)

#define WL_EVENTING_MASK_MAX_LEN	64
#define WL_EVENTINT_MAX_GET_SIZE	(WL_EVENTING_MASK_MAX_LEN + EVENTMSGS_EXT_STRUCT_SIZE)

/* WLC_VER_MAJOR for KUDU */
#define KUDU_WLC_VER_MAJOR		9
#endif /* _wlu_h_ */
