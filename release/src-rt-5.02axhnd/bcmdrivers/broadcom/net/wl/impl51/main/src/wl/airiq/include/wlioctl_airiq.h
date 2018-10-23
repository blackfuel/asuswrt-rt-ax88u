/*
 * @file
 * @brief
 *
 *  Air-IQ userspace interface
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
 * $Id: wlioctl_airiq.h 729858 2017-11-02 19:52:32Z $
 *
 */
#ifndef _wl_airiq_if_h_
#define _wl_airiq_if_h_

#ifndef __KERNEL__
#include <sys/socket.h>
#endif // endif
/* #include <linux/if.h> */

/*
 * Message interface
 */

#define AIRIQ_SCAN_SUCCESS    0
#define AIRIQ_SCAN_ABORTED    1

/*
 * Data queue message types
 *
 */

#define MESSAGE_TYPE_IQDATA 1
#define MESSAGE_TYPE_FFTCPX 2
#define MESSAGE_TYPE_SCAN_COMPLETE 3
#define MESSAGE_TYPE_FFTCPX_VASIP 4
/* Offload message types */
#define MESSAGE_TYPE_OL_DATA 5
#define MESSAGE_TYPE_OL_REBOOT 6

typedef struct {
    uint32 message_type;
    uint32 size_bytes;
    uint16 corerev;
    uint16 unit;
} airiq_message_header_t;

#define MAX_CHANSPECS   190
#define MAX_SCAN_CHANSPECS 64

#define IQDATA_FORMAT_HT 1
#define IQDATA_FORMAT_N  2
#define IQDATA_FORMAT_N_REV7  3

#define IQDATA_FORMAT_AC  (4 << 8)
#define IQDATA_FORMAT_AC_4 (IQDATA_FORMAT_AC | 4)
#define IQDATA_FORMAT_AC_5 (IQDATA_FORMAT_AC | 5)
#define IQDATA_FORMAT_AC_6 (IQDATA_FORMAT_AC | 6)
#define IQDATA_FORMAT_AC_7 (IQDATA_FORMAT_AC | 7)
#define IQDATA_FORMAT_AC_8 (IQDATA_FORMAT_AC | 8)
#define IQDATA_FORMAT_AC_9 (IQDATA_FORMAT_AC | 9)
#define IQDATA_FORMAT_AC_MODE_MASK 0xff

typedef struct {
	/* must be same as airiq_message_header_t */
	uint32 message_type;
	uint32 size_bytes;
	uint16 corerev;
	uint16 unit;
	/*  */
	uint8 phy_type;
	uint8 phy_rev;
	uint8 core;
	uint8 ___padding;
	uint16 format;
	int16 channel_idx;
	int16 channel_cnt;
	int16 fc_mhz;
	int16 agc_gain_db;
	uint16 sample_bits;     /* bits per sample */
	uint16 sample_rate_mhz;
	uint32 timestamp_us;
	uint32 sample_count;
	uint32 data_bytes;
	uint32 latency_us;
	chanspec_t other_radio_chanspec;
	chanspec_t chanspec_list[MAX_SCAN_CHANSPECS];
} airiq_iqdata_header_t;

#define AIRIQ_GAINCODE_GAIN(gaincode)   ((gaincode) & 0xff)
#define AIRIQ_GAINCODE_DESENS(gaincode) (0xff & ((gaincode) >> 8))
#define AIRIQ_GAINCODE(gain, desens) (((gain) & 0xff) | ((desens) << 8))

/* FFT Flags */
#define FFT_FLAG_INTERLEAVED      0x1
#define FFT_FLAG_LAST             0x2

typedef struct {
	/* must be same as airiq_message_header_t */
	uint32 message_type;       /* 0 */
	uint32 size_bytes;         /* 4 */
	uint16 corerev;            /* 6 */
	uint16 unit;               /* 8 */
	/*  */
	int16       gaincode;      /* a */
	uint16      fc_mhz;        /* c */
	chanspec_t  chanspec;      /* e */
	uint16      bins;          /* 10 */
	uint16      seqno;         /* 12 */
	uint16      flags;         /* 14 */
	uint32      timestamp;     /* 18 */
	uint32      data_bytes;    /* 1c */
	uint16      fc_3x3_mhz;    /* 20 */
	uint16      __pad[3];      /* 22 */
	/* 28 */
	/* Please note this structure must be sized to a
	 * multiple of 8 bytes
	 */
} airiq_fftdata_header_t;

typedef struct {
	/* must be same as airiq_message_header_t */
	uint32 message_type;
	uint32 size_bytes;
	uint16 corerev;
	uint16 unit;
	/*  */
#ifndef DONGLEBUILD
	char   ifname[IFNAMSIZ];
#endif // endif
	uint32 tsf_start;
	uint32 tsf_end;
} airiq_scan_complete_t;

typedef struct {
	uint32 freq_khz;
	uint32 channel_num;
	chanspec_t chanspec;
} airiq_sample_collect_args_t;

typedef struct {
	uint32 id;
	uint32 start_freq_mhz;
	uint32 stop_freq_mhz;
	int32  init_gain_db;
	int32  sweep_count;
} airiq_gain_hint_t;

typedef struct {
	int32 sens24ghz;
	int32 sens5ghz_low;
	int32 sens5ghz_mid;
	int32 sens5ghz_high;
	int32 agc_gain_step;
} airiq_radio_settings_t;

typedef struct {
	int16 start;
	int16 chanspec_cnt;
	int16 sweep_cnt;
	uint16 dwell_interval_ms[MAX_SCAN_CHANSPECS];
	uint16 capture_interval_us[MAX_SCAN_CHANSPECS];
	chanspec_t chanspec_list[MAX_SCAN_CHANSPECS];
	uint32 capture_count[MAX_SCAN_CHANSPECS];
	uint8 core_config[MAX_SCAN_CHANSPECS];
	uint16 phy_mode;
} airiq_config_t;

#define AIRIQ_ACPHY_LNA_COUNT        8
#define AIRIQ_ACPHY_MIXTIA_COUNT     10
#define AIRIQ_ACPHY_LNA_ROUT_COUNT   16
#define AIRIQ_LNA_ROUT_LUT_CNT       7
typedef struct {
	int16 valid;
	int8 lna1[AIRIQ_ACPHY_LNA_COUNT];
	int8 lna2[AIRIQ_ACPHY_LNA_COUNT];
	int8 mixtia[AIRIQ_ACPHY_MIXTIA_COUNT];
	int8 lna_rout[AIRIQ_ACPHY_LNA_ROUT_COUNT];
	uint8 lna_rout_lut[AIRIQ_LNA_ROUT_LUT_CNT];
	int8 offset;
	int8 rxloss;
	int8 elna;
} airiq_gain_table_t;

typedef struct {
	uint16 code[MAX_CHANSPECS];
} airiq_gain_t;

typedef struct {
	uint32 command;
	uint32 size;
	uint32 command_status;
	uint32 return_status;
	/* data is appended to the end of this */
} airiq_ol_cmd_t;

enum {
	/* offload IOCTL status */
	AIRIQ_OL_CMD_IDLE        = 0x00,
	AIRIQ_OL_CMD_PENDING     = 0x01,
	AIRIQ_OL_CMD_COMPLETE    = 0x02,
	AIRIQ_OL_CMD_ABORT       = 0x03
};

enum {
	AIRIQ_OL_DRV_CMD_SCAN_COMPLETE = 0x10000,
	AIRIQ_OL_DRV_CMD_ENABLE        = 0x10100,
	AIRIQ_OL_DRV_CMD_DISABLE       = 0x10101
};

typedef struct {
	uint32 DeviceId;
	uint32 EventId;
	uint32 Size;
} airiq_ol_event_t;

#define SCAN_COMPLETE_RADIOS 3

typedef struct {
	uint8  airiq_event_type;
	uint32 scan_complete[SCAN_COMPLETE_RADIOS];
	uint32 scan_status[SCAN_COMPLETE_RADIOS];
	uint32 pkt_counter;
	uint32 data_len;
	uint8  data[];

} airiq_event_t;

enum {
	AIRIQ_EVENT_DATA      = 0x1,
	AIRIQ_EVENT_SCAN_COMPLETE      = 0x2
};

typedef struct {
	/* LTE_U detector parameters */
	uint16 nshift;
	uint16 edlow;
	uint16 edhigh;
	uint16 edshift;
	uint16 dtlow;
	uint16 dthigh;
	uint16 dtabslow;
	uint16 dtabshigh;
} lte_u_detector_config_t;

typedef struct {
	uint8  lte_u_present;
	int16 rssi_smoothed;
	uint32 timestamp;
	uint32 prevTimestamp;
	bool   lte_u_active;
	chanspec_t chanspec;
} lte_u_scan_status_t;

typedef struct {
	uint8  lte_u_event_type;
	uint32 data_len;
	uint8  data[];
} lte_u_event_t;

enum {
	LTE_U_EVENT_SCAN_STATUS      = 0x1,
	LTE_U_EVENT_SCAN_ABORT       = 0x2,
	LTE_U_EVENT_IQ_CAPTURE       = 0x3
};

enum {
	LTE_U_SCAN_ABORT_CMD            = 0x1,
	LTE_U_SCAN_ABORT_WL_DOWN        = 0x2,
	LTE_U_SCAN_ABORT_SCAN_DISABLE   = 0x3,
	LTE_U_SCAN_ABORT_NOT_SUPPORTED  = 0x4,
	LTE_U_SCAN_ABORT_BW_CHANGED     = 0x5,
	LTE_U_SCAN_FINISHED             = 0x6
};

#define LTE_U_ACPHY_LNA_COUNT        8
#define LTE_U_ACPHY_MIXTIA_COUNT     10
#define LTE_U_ACPHY_LNA_ROUT_COUNT   16
#define LTE_U_LNA_ROUT_LUT_CNT       7
typedef struct {
	int16 valid;
	int8 lna1[LTE_U_ACPHY_LNA_COUNT];
	int8 lna2[LTE_U_ACPHY_LNA_COUNT];
	int8 mixtia[LTE_U_ACPHY_MIXTIA_COUNT];
	int8 lna_rout[LTE_U_ACPHY_LNA_ROUT_COUNT];
	uint8 lna_rout_lut[LTE_U_LNA_ROUT_LUT_CNT];
	int8 offset;
	int8 rxloss;
	int8 elna;
} lte_u_gain_table_t;

typedef struct {
	uint16 code[MAX_CHANSPECS];
} lte_u_gain_t;

#define MESSAGE_TYPE_VASIP_IQEVT1 1
#define MESSAGE_TYPE_VASIP_IQEVT2 2
#define LTE_U
typedef struct {
	uint32      message_type;
	uint32      size_bytes;
	uint16      corerev;
	uint16      unit;
	uint32      timestamp;
	uint16      seqno;
	chanspec_t  chanspec;
	uint32      data_bytes;
} lte_u_iqdata_header_t;

#endif /* _wl_airiq_if_h_ */
