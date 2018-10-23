/*
 * Common code for wl routines
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
 * $Id: wlu_common.h 760445 2018-05-02 05:18:36Z $
 */
#include <wlioctl.h>
#include <bcmutils.h>
#include "wlu_cmd.h"

#if	defined(_CFE_) /* router boot loader */
#include <lib_types.h>
#include <lib_string.h>
#include <lib_printf.h>
#include <lib_malloc.h>
#include <cfe_error.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#endif /* defined(_CFE_) */

#if defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION >= 0x00020000)
#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))
#define strtol(nptr, endptr, base)  (long)bcm_strtoul((nptr), (endptr), (base))
#endif /* defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION >= 0x00020000) */

#if defined(linux)

#define stricmp strcasecmp
#define strnicmp strncasecmp

#elif	defined(_CRT_SECURE_NO_DEPRECATE) /* MSVC common runtime */

#define stricmp _stricmp
#define strnicmp _strnicmp

#elif	defined(DONGLEBUILD)

#define stricmp strcmp
#define strnicmp strncmp

#elif	defined(_CFE_) /* router boot loader */

#include <bcmutils.h>
#include <osl.h>
#define isalnum(c) bcm_isalnum(c)
#define isalpha(c) bcm_isalpha(c)
#define iscntrl(c) bcm_iscntrl(c)
#define isdigit(c) bcm_isdigit(c)
#define isgraph(c) bcm_isgraph(c)
#define islower(c) bcm_islower(c)
#define isprint(c) bcm_isprint(c)
#define ispunct(c) bcm_ispunct(c)
#define isspace(c) bcm_isspace(c)
#define isupper(c) bcm_isupper(c)
#define isxdigit(c) bcm_isxdigit(c)
#define stricmp(s1, s2) lib_strcmpi((s1), (s2))
#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))
#define tolower(c) (bcm_isupper((c)) ? ((c) + 'a' - 'A') : (c))
#define fprintf(stream, fmt, args...) xprintf(fmt, ##args)
#define fputs(s, stream) puts(s)
#define malloc(size) KMALLOC((size), 0)
#define free(ptr) KFREE(ptr)
#define strnicmp(s1, s2, len) strncmp((s1), (s2), (len))
#define strspn(s, accept) (0)
#define strtol(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))

#elif	defined(BWL_STRICMP)

#define stricmp bcmstricmp
#define strnicmp bcmstrnicmp

#endif /* BWL_STRICMP */

#if defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION >= 0x00020000)
#include <sys/stat.h>

extern int wlu_efi_stat(char *filename, struct stat *filest);
extern long wlu_efi_ftell(void *fp);
extern int wlu_efi_fseek(void *fp, long offset, int whence);
extern size_t wlu_efi_fwrite(void *buf, size_t size, size_t nmemb, void *fp);
extern size_t wlu_efi_fread(void *buf, size_t size, size_t nmemb, void *fp);
extern void wlu_efi_fclose(void *fp);
extern void * wlu_efi_fopen(const char *filename, char *mode);
extern void wlu_efi_fgets(char *dst, int max, FILE *fp);
extern int wlu_efi_feof(void *fp);
extern int wlu_efi_ferror(void *fp);

#define fopen(filename, mode)			(FILE *)wlu_efi_fopen(filename, mode)
#define fread(buf, size, nmemb, fp)		wlu_efi_fread(buf, size, nmemb, fp)
#define fwrite(buf, size, nmemb, fp)	wlu_efi_fwrite(buf, size, nmemb, fp)
#define fseek(fp, offset, origin)		wlu_efi_fseek(fp, offset, origin)
#define ftell(fp)						wlu_efi_ftell(fp)
#define stat(fname, filest)				wlu_efi_stat(fname, (struct stat *)(filest))
#define fclose(fp)						wlu_efi_fclose(fp)
#define fgets(dst, max, fp)				wlu_efi_fgets(dst, max, fp)
#undef  feof
#define feof(fp)						wlu_efi_feof(fp)
#undef  ferror
#define ferror(fp)						wlu_efi_ferror(fp)
#define fprintf(stream, fmt, args...)	printf(fmt, ##args)
#ifdef stderr
#undef stderr
#define stderr stdout
#endif // endif
#endif /* defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION >= 0x00020000) */

/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
/* The below macros handle endian mis-matches between wl utility and wl driver. */
extern bool g_swap;
#define htod64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define dtoh64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define htodchanspec(i) (g_swap?htod16(i):i)
#define dtohchanspec(i) (g_swap?dtoh16(i):i)
#define htodenum(i) (g_swap?((sizeof(i) == 4) ? htod32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)
#define dtohenum(i) (g_swap?((sizeof(i) == 4) ? dtoh32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)

/* command batching data structure */
typedef struct wl_seq_cmd_pkt {
	struct wl_seq_cmd_pkt	*next;
	wl_seq_cmd_ioctl_t cmd_header;
	char * data;			/* user buffer */
} wl_seq_cmd_pkt_t;

typedef struct wl_cmd_list {
	wl_seq_cmd_pkt_t	*head;
	wl_seq_cmd_pkt_t	*tail;
} wl_cmd_list_t;

/*
 * Name table to associate strings with numeric IDs
 */
typedef struct wlu_name_entry {
	uint id;
	const char *name;
} wlu_name_entry_t;

typedef union wl_rateset_args_u {
	wl_rateset_args_v1_t rsv1;
	wl_rateset_args_v2_t rsv2;
} wl_rateset_args_u_t;

extern wl_cmd_list_t cmd_list;
extern int cmd_pkt_list_num;
extern bool cmd_batching_mode;

extern int wlu_iovar_getbuf(void* wl, const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen);
extern int wlu_iovar_setbuf(void* wl, const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen);
extern int wlu_var_setbuf(void *wl, const char *iovar, void *param, int param_len);
extern int wlu_iovar_getint(void *wl, const char *iovar, int *pval);
extern void init_cmd_batchingmode(void);
extern void clean_up_cmd_list(void);
extern int wl_check(void *wl);

extern int add_one_batched_cmd(int cmd, void *cmdbuf, int len);
extern int wlu_get_req_buflen(int cmd, void *cmdbuf, int len);
extern int wlu_get(void *wl, int cmd, void *cmdbuf, int len);
extern int wlu_set(void *wl, int cmd, void *cmdbuf, int len);
extern int wlu_iovar_get(void *wl, const char *iovar, void *outbuf, int len);
extern int wlu_iovar_set(void *wl, const char *iovar, void *param, int paramlen);
extern int wlu_iovar_getint(void *wl, const char *iovar, int *pval);
extern int wlu_iovar_setint(void *wl, const char *iovar, int val);
extern const char* wlu_lookup_name(const wlu_name_entry_t* tbl, uint id);

/* pretty print an SSID */
extern int wl_format_ssid(char* buf, uint8* ssid, int ssid_len);

/* pretty hex print a contiguous buffer */
extern void wl_hexdump(uchar *buf, uint nbytes);

int wl_prefixiovar_mkbuf(const char *iovar, const char *prefix, int prefix_index, void *param,
	int paramlen, void *bufptr, int buflen, int *perr);
int wlu_bssiovar_setbuf(void* wl, const char *iovar, int bssidx,
	void *param, int paramlen, void *bufptr, int buflen);
int wl_bssiovar_getbuf(void* wl, const char *iovar, int bssidx,
	void *param, int paramlen, void *bufptr, int buflen);
int wl_bssiovar_set(void *wl, const char *iovar, int bssidx, void *param, int paramlen);
int wlu_bssiovar_get(void *wl, const char *iovar, int bssidx, void *outbuf, int len);
int wl_bssiovar_setint(void *wl, const char *iovar, int bssidx, int val);
int wl_bssiovar_getint(void *wl, const char *iovar, int bssidx, int *pval);
int wl_get_rateset_args_info(void *wl, int *rs_len, int *rs_ver);
void wl_rateset_get_fields(wl_rateset_args_u_t* rs, int rsver, uint32 **rscount,
	uint8 **rsrates, uint8 **rsmcs, uint16 **rsvht_mcs, uint16 **rshe_mcs);
void wl_print_vhtmcsset(uint16 *mcsset);
int wlu_bcmp(const void *b1, const void *b2, int len);

extern int wlu_var_getbuf(void *wl, const char *iovar,
	void *param, int param_len, void **bufptr);
extern int wlu_var_getbuf_sm(void *wl, const char *iovar,
	void *param, int param_len, void **bufptr);
extern int wlu_var_getbuf_med(void *wl, const char *iovar,
	void *param, int param_len, void **bufptr);
extern int wlu_var_setbuf_sm(void *wl, const char *iovar,
		void *param, int param_len);

extern int wl_parse_ssid_list(char* list_str, wlc_ssid_t* ssid, int idx, int max);

extern cmd_func_t wl_void;
extern cmd_func_t wl_var_void;
extern cmd_func_t wl_var_setint;
extern cmd_func_t wl_var_get;
extern cmd_func_t wl_var_getandprintstr;
extern cmd_func_t wl_reg;
extern cmd_func_t wl_ssid;
extern cmd_func_t wl_iov_mac;
extern cmd_func_t wl_sta_info;
extern cmd_func_t wl_hostip;
extern cmd_func_t wl_hostipv6;
extern cmd_func_t wl_offload_cmpnt;
extern cmd_func_t wl_mkeep_alive;
extern cmd_func_t wlu_srwrite;
extern cmd_func_t wlu_reg2args;
extern cmd_func_t wl_macaddr;
extern cmd_func_t wl_maclist;
extern cmd_func_t wl_interface_create_action;
extern cmd_func_t wl_print_deprecate;
extern cmd_func_t wl_rssi;
extern cmd_func_t wl_gmode;
extern cmd_func_t wl_nvdump, wl_nvget, wl_nvset;
extern cmd_func_t wl_wlc_ver;
extern cmd_func_t wl_struct_ver;
extern cmd_func_t wl_assoc_info;
extern cmd_func_t wl_rxfifo_counters;
extern cmd_func_t wl_hs20_ie;
extern cmd_func_t wlu_cur_etheraddr;

/* wl functions used by the ndis wl. */
extern void dump_rateset(uint8 *rates, uint count);
extern uint freq2channel(uint freq);
extern int wl_ether_atoe(const char *a, struct ether_addr *n);
extern char *wl_ether_etoa(const struct ether_addr *n);
extern int wl_atoip(const char *a, struct ipv4_addr *n);
extern char *wl_iptoa(const struct ipv4_addr *n);
extern cmd_func_t wl_varint;
extern void wl_dump_raw_ie(bcm_tlv_t *ie, uint len);
extern int wl_mk_ie_setbuf(const char *command, uint32 pktflag_ok, char **argv,
	vndr_ie_setbuf_t **buf, int *buf_len);
extern cmd_func_t wl_list_ie;

int wl_wait_for_event(void *wl, char **argv, uint event_id, uint evbuf_size,
	void (*event_cb_fn)(int event_type, bcm_event_t *bcm_event));

extern int wl_cfg_option(char **argv, const char *fn_name, int *bsscfg_idx, int *consumed);
extern int wl_scan_prep(void *wl, cmd_t *cmd, char **argv,
	wl_scan_params_t *params, int *params_size);
extern int wl_parse_channel_list(char* list_str, uint16* channel_list, int channel_num);
extern int get_ie_data(uchar *data_str, uchar *ie_data, int len);

extern void wl_print_mcsset(char *mcsset);
extern int wl_atoipv6(const char *a, struct ipv6_addr *n);
/* Convert user's input in hex pattern to byte-size mask */
extern int wl_pattern_atoh(char *src, char *dst);

extern int ARGCNT(char **argv);

extern int wl_parse_chanspec_list(char* list_str, chanspec_t *chanspec_list, int chanspec_num);

extern int hexstr2hex(char *str);
extern char* find_pattern(char **argv, const char *pattern, uint *val);
extern char* find_pattern2(char **argv, const char *pattern, uint *val, int vnum);
extern char * wl_ipv6toa(const void *ipv6);
extern int parse_wep(char **argv, wl_wsec_key_t *key, bool options);

/* print usage */
extern void wl_cmd_usage(FILE *fid, cmd_t *cmd);

extern void wl_nrate_print(uint32 rspec, int ioctl_version);
extern void wl_wnm_print(uint32  wnm_cap);
extern void wl_print_hemcsset(uint16 *mcsset);
extern void wl_print_hemcsnss(uint16 *mcsset);
extern int wl_sta_info_print(void *wl, void *buf);
extern int wl_wds_info_all(void *wl, cmd_t *cmd);
