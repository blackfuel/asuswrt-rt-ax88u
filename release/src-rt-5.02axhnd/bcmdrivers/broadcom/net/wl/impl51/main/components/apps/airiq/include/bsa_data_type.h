/*
 * Broadcom AirIQ
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
 * $Id: bsa_data_type.h 753478 2018-03-21 21:29:56Z $
 */

#ifndef __BSA_DATA_TYPE_H__
#define __BSA_DATA_TYPE_H__

/*******************************************************************************
 * BSA Global Data Type                                                        *
 * --- ------ ---- ----                                                        *
 *                                                                             *
 * SUMMARY: Defines global BSA data types and macros.                          *
 *                                                                             *
 ******************************************************************************/

/*******************************************************************************
 * Includes                                                                    *
 ******************************************************************************/
#if !defined(THREADX)
#if !defined WIN32
    #if !defined(MODULE)
	#include <stdint.h>
    #endif
    #include <linux/types.h>
#else
#ifndef _NTDDK_
#include <windows.h>
#endif // endif
#endif // endif
#endif // endif

#if defined(THREADX)
#include <stdint.h>
#endif // endif

#if !(defined(AIRIQ_APP_SIRP) || defined(AIRIQ_APP_WL))
#include "target.h"
#endif // endif

#if !defined(BSADLLSPEC)
#if defined(WIN32)
	#ifdef _DLLLIB
		#define BSADLLSPEC         _declspec(dllexport)
	#else
		#if defined(__cplusplus)
			#define BSADLLSPEC extern "C" _declspec(dllimport)
		#else
			#define BSADLLSPEC
		#endif
	#endif
#else
		#define BSADLLSPEC         __attribute__ ((visibility("default")))
#endif // endif
#endif // endif

/* __AICS__ needed for certain alert events, e.g. high-CU alert. */
#define __AICS__                           1
#define __SWSA__                           1

#ifdef WIN32
#pragma pack(1)
#endif // endif

/*******************************************************************************
 * Data structures and type definitions                                        *
 ******************************************************************************/
#if defined(_MSC_VER)
	#define BSA_INLINE __inline
#else
	#define BSA_INLINE inline
#endif // endif

/* Scalar Data Types **********************************************************/
#if defined(THREADX) || !defined(WIN32)
#define BSA_INLINE         inline
typedef int8_t     BSA_INT8;
typedef uint8_t    BSA_UINT8;
typedef int16_t    BSA_INT16;
typedef uint16_t   BSA_UINT16;
typedef int32_t    BSA_INT32;
typedef uint32_t   BSA_UINT32;
typedef int64_t    BSA_INT64;
typedef uint64_t   BSA_UINT64;
typedef char       BSA_CHAR;
typedef int32_t    BSA_BOOL;
typedef void       BSA_VOID;

typedef int8_t     *PBSA_INT8;
typedef uint8_t    *PBSA_UINT8;
typedef int16_t    *PBSA_INT16;
typedef uint16_t   *PBSA_UINT16;
typedef int32_t    *PBSA_INT32;
typedef uint32_t   *PBSA_UINT32;
typedef int64_t    *PBSA_INT64;
typedef uint64_t   *PBSA_UINT64;
typedef char       *PBSA_CHAR;
typedef int32_t    *PBSA_BOOL;
typedef void       *PBSA_VOID;
#else
typedef CHAR       BSA_INT8;
typedef UCHAR      BSA_UINT8;
typedef INT16      BSA_INT16;
typedef UINT16     BSA_UINT16;
typedef INT32      BSA_INT32;
typedef UINT32     BSA_UINT32;
typedef LONG64     BSA_INT64;
typedef ULONG64    BSA_UINT64;
typedef char       BSA_CHAR;
typedef UINT32     BSA_BOOL;
typedef void       BSA_VOID;

typedef BSA_INT8   *PBSA_INT8;
typedef BSA_UINT8  *PBSA_UINT8;
typedef BSA_INT16  *PBSA_INT16;
typedef BSA_UINT16 *PBSA_UINT16;
typedef BSA_INT32  *PBSA_INT32;
typedef BSA_UINT32 *PBSA_UINT32;
typedef BSA_INT64  *PBSA_INT64;
typedef BSA_UINT64 *PBSA_UINT64;
typedef BSA_CHAR   *PBSA_CHAR;
typedef BSA_BOOL   *PBSA_BOOL;
typedef void       *PBSA_VOID;
#endif // endif

#define BSA_TRUE                    1
#define BSA_FALSE                   0

/* Status/Return Codes ********************************************************/
typedef BSA_UINT32 BSA_STATUS;

#define BSA_SUCCESS                 0x00000000
#define BSA_FAIL                    0x00000001
#define BSA_CONNECTION_REFUSED      0x00000002
#define BSA_DESTINATION_UNREACH     0x00000003
#define BSA_NOT_CONNECTED           0x00000004
#define BSA_UNSUPPORTED             0x00000005
#define BSA_ALREADY_LOCKED          0x00000006
#define BSA_ALREADY_STARTED         0x00000007
#define BSA_DEVICE_NOT_PRESENT      0x00000008
#define BSA_UNSUPPORTED_CHANNEL     0x00000009
#define BSA_UNSUPPORTED_BAND        0x0000000a
#define BSA_UNSUPPORTED_BANDWIDTH   0x0000000b
#define BSA_UNSUPPORTED_SCAN_MODE   0x0000000c
#define BSA_BAD_HANDLE              0x0000000d
#define BSA_OUT_OF_MEMORY           0x0000000e
#define BSA_BUFFER_TOO_SMALL        0x0000000f
#define BSA_INT_REQUEST_FAILED      0x00000010
#define BSA_MEMORY_ERROR            0x00000011
#define BSA_EVENT_TIMEOUT           0x00000012
#define BSA_BSOCK_ERROR             0x00000013
#define BSA_REQUEST_TIMEOUT         0x00000014
#define BSA_UNSUPPORTED_INTERFERER  0x00000015
#define BSA_TOO_MANY_CHANNELS       0x00000016
#define BSA_UNSUPPORTED_AGC_MODE    0x00000017
#define BSA_INVALID_ANTENNA         0x00000018
#define BSA_INVALID_FW_SIZE         0x00000019
#define BSA_FW_ALREADY_LOADED       0x0000001a
#define BSA_BAD_CALLBACK_PTR        0x0000001b
#define BSA_VENDOR_MISMATCH         0x0000001c
#define BSA_VERSION_MISMATCH        0x0000001d
#define BSA_INCOMPATIBLE_VERSION    0x0000001e
#define BSA_NOT_AUTHORIZED          0x0000001f
#define BSA_BAD_FILESPEC            0x00000020
#define BSA_FILE_IO_ERROR           0x00000021
#define BSA_IOCTL_VER_INCOMPATIBLE  0x00000022
#define BSA_IOCTL_IF_NOT_INIT       0x00000023
#define BSA_INCOMPATIBLE_FW_VERSION 0x00000024
#define BSA_OLDER_FW_VERSION        0x00000025
#define BSA_TOO_MANY_BANDS          0x00000026
#define BSA_PARAM_OUT_OF_RANGE      0x00000027
#define BSA_ABORTED                 0x00000028

/* BSA Firmware Segment **********************************************************/
/* NOTE: this should be the same as the defines in hex2src.c */
#define MAX_SEG_WORD_SIZE           (1024)
#define MAX_SEG_SIZE                (4 * MAX_SEG_WORD_SIZE)
#define BSA_MAX_FW_SEGMENTS         48     /* 32 */

#ifdef WIN32
/* //#pragma pack(1) */
#endif // endif
typedef struct {
	BSA_UINT32 Size;
	BSA_UINT32 BaseAddr;
	BSA_UINT32 Data[MAX_SEG_WORD_SIZE];
}
#ifndef WIN32
__attribute__((packed))
#endif // endif
BSA_FW_SEG, *PBSA_FW_SEG;

#ifdef WIN32
/* //#pragma pack() */
#endif // endif

/* BSA Device Handle **********************************************************/
typedef void *BSA_DEVICE_HANDLE;

/* Event Id's *****************************************************************/
typedef BSA_UINT32 BSA_EVENT_ID;

#define BSA_EVENT_ID_FFT_DATA           0x00000001
#define BSA_EVENT_ID_INTERFERENCE       0x00000002
#define BSA_EVENT_ID_CHAN_UTIL          0x00000004
#define BSA_EVENT_ID_CONN_STATUS        0x00000008
#define BSA_EVENT_ID_AICS_CHAN_CHANGE   0x00000010
#define BSA_EVENT_ID_AICS_STATS         0x00000020
#define BSA_EVENT_ID_AGGR_FFT_DATA      0x00000040
#define BSA_EVENT_ID_ALERT              0x00000080
#define BSA_EVENT_ID_CHAN_UTIL_WITH_INT 0x00000100
#define BSA_EVENT_ID_IQ_DATA            0x00000200
#define BSA_EVENT_ID_SA_MODE_STATUS     0x00000400
#define BSA_EVENT_ID_SCAN_MODE_CHANGE   0x00000800
#define BSA_EVENT_ID_DEBUG_LOG          0x00001000
#define BSA_EVENT_ID_FFTCPX             0x00002000
#define BSA_EVENT_ID_SCAN_COMPLETE      0x00004000
#ifndef AIRIQ_OFFLOAD
#define BSA_EVENT_ID_OL_REBOOT          0x00008000
#endif // endif
#define BSA_EVENT_ID_DGRAM_DATA         0x10000000

#define BSA_EVENT_ID_ALL                0x100007ff

/* Event Handler **************************************************************/
typedef void (*BSA_EVENT_HANDLER)(
        BSA_EVENT_ID      EventId,
        BSA_DEVICE_HANDLE DevHandle,
        void             *EventData
        );

typedef void (*BSA_MS_EVENT_HANDLER)(
        BSA_UINT32   DeviceId,
        BSA_UINT8   *pData,
        BSA_UINT32   Size,
        BSA_EVENT_ID EventId);
/* Authentication Key *********************************************************/
#define BSA_AUTH_KEY_DIGEST_LEN     16

/* Information Element Types **************************************************/
typedef BSA_UINT32 BSA_IE_TYPE;

#define BSA_IE_FFT_CAP              0x00000001
#define BSA_IE_CLASSIFICATION_CAP   0x00000002
#define BSA_IE_CHAN_UTIL_CAP        0x00000003
#define BSA_IE_SCAN_MODE_CAP        0x00000004
#define BSA_IE_FREQ_BAND_CAP        0x00000005
#define BSA_IE_BANDWIDTH_CAP        0x00000006
#define BSA_IE_VERSION              0x00000007
#define BSA_IE_AICS_CAP             0x00000008

/* Serial Number **************************************************************/
#define BSA_MAX_SERIAL_NUM_LEN      64

/* Scan Modes *****************************************************************/
typedef BSA_UINT32 BSA_SCAN_MODE;

#define BSA_SCAN_MODE_DWELL         0x00000001
#define BSA_SCAN_MODE_24GHZ         0x00000002
#define BSA_SCAN_MODE_24GHZ_5GHZ    0x00000004
#define BSA_SCAN_MODE_5GHZ_LOW      0x00000008
#define BSA_SCAN_MODE_5GHZ_MID      0x00000010
#define BSA_SCAN_MODE_5GHZ_HIGH     0x00000020
#define BSA_SCAN_MODE_5GHZ          0x00000040
#define BSA_SCAN_MODE_49GHZ         0x00000080
#define BSA_SCAN_MODE_USER1         0x00000100
#define BSA_SCAN_MODE_USER2         0x00000200
#define BSA_SCAN_MODE_USER3         0x00000400
#define BSA_SCAN_MODE_USER4         0x00000800

#define BSA_CHANNELS_PER_SCAN_MODE  62

#ifdef AIRIQ_OFFLOAD
/* Offload memory resources are severely constrained, however
 *  we may consider htis optimization for all Air-IQ, as the
 *  concept of scan modes is no longer all that relevant */
#define SCAN_MODE_COUNT             1
#else
#define SCAN_MODE_COUNT             12
#endif // endif
/* Connection Events **********************************************************/
typedef BSA_UINT32 BSA_CONN_EVENT;

#define BSA_CONN_ESTABLISHED        0x00000001
#define BSA_CONN_TIMEOUT            0x00000002
#define BSA_CONN_TERMINATED         0x00000004
#define BSA_CONN_BSAP_ERROR         0x00000008
#define BSA_CONN_RESTART_FOR_IMPORT 0x00000010

/* SPECTRUM Model  **********************************************************/
typedef BSA_UINT32 BSA_SA_MODEL;

#define BSA_UNKNOWN_SA              0x00000000
#define BSA_MINIPCI_SA              0x00000001
#define BSA_USB_SA                  0x00000002
#define BSA_PCI_EXP_SA              0x00000003
#define BSA_BRCM_SW_SA              0x00000004

/* Vendor specific information ****************************************/
typedef BSA_UINT32 BSA_VENDOR_ID;

typedef struct {
	BSA_VENDOR_ID VendorId;
	BSA_UINT32    Reserved0;
	BSA_UINT32    Reserved1;
	BSA_UINT32    Reserved2;
	BSA_UINT32    Reserved3;
	BSA_UINT32    Reserved4;
	BSA_UINT32    Reserved5;
	BSA_UINT32    Reserved6;
	BSA_UINT32    Reserved7;
} BSA_VENDOR_INFO, *PBSA_VENDOR_INFO;

/* API and SIRP Version information ****************************************/
typedef struct {
	BSA_UINT32 BsaApiVerMajor;
	BSA_UINT32 BsaApiVerMinor;
	BSA_UINT32 SirpVer;
	BSA_UINT32 Reserved0;
	BSA_UINT32 Reserved1;
	BSA_UINT32 Reserved2;
	BSA_UINT32 Reserved3;
	BSA_UINT32 Reserved4;
	BSA_UINT32 Reserved5;
	BSA_UINT32 Reserved6;
	BSA_UINT32 Reserved7;
} BSA_CONN_VERSION_INFO, *PBSA_CONN_VERSION_INFO;

/* Interferer types ***********************************************************/
typedef BSA_UINT32 BSA_INTERFERER_TYPE;

#define BSA_NUM_INTERFERERS              24

#define BSA_INT_UNKNOWN                  0x00000001
#define BSA_INT_UWAVE_OVEN               0x00000002
#define BSA_INT_IUWAVE_OVEN              0x00000004
#define BSA_INT_NARROWBAND_CW            0x00000008
#define BSA_INT_BLUETOOTH                0x00000010
#define BSA_INT_DSSS_PHONE               0x00000020
#define BSA_INT_RADAR                    0x00000040
#define BSA_INT_WVIDEO                   0x00000080
#define BSA_INT_FHSS_PHONE               0x00000100
#define BSA_INT_BABY_MONITOR             0x00000200
#define BSA_INT_DIGITAL_VIDEO_MONITOR    0x00000400
#define BSA_INT_GAME_CONTROLLER          0x00000800
#define BSA_INT_POSSIBLE_INT             0x00001000
#define BSA_INT_JAMMER                   0x00002000
#define BSA_INT_MOTION_DETECTOR          0x00004000
#define BSA_INT_DIGITAL_VIDEO_MONITOR2   0x00008000
#define BSA_INT_YDI_NB_JAMMER            0x00010000
#define BSA_INT_ZIGBEE                   0x00020000
#define BSA_INT_NANO_MOUSE               0x00040000
#define BSA_INT_AIRHORN                  0x00080000
#define BSA_INT_GAME_CONTROLLER_XBOX     0x00100000
#define BSA_INT_WLAN_FH                  0x00200000
#define BSA_INT_CANOPY                   0x00400000
#define BSA_INT_WIRELESS_BRIDGE          0x00800000

#define BSA_INT_ALL                      0x00ffffff

#define INTERFERER_STR(type)     \
        ((type) == BSA_INT_UNKNOWN) ? "Unknown" : \
        ((type) == BSA_INT_UWAVE_OVEN) ? "Microwave Oven" : \
        ((type) == BSA_INT_IUWAVE_OVEN) ? "Inverter Microwave oven" : \
        ((type) == BSA_INT_NARROWBAND_CW) ? "Analog Cordless Phone" : \
        ((type) == BSA_INT_BLUETOOTH) ? "Bluetooth" : \
        ((type) == BSA_INT_DSSS_PHONE) ? "DSSS Cordless Phone" : \
        ((type) == BSA_INT_WVIDEO) ? "Wireless Video Camera" : \
        ((type) == BSA_INT_FHSS_PHONE) ? "FHSS Cordless Phone or Headset" : \
        ((type) == BSA_INT_BABY_MONITOR) ? "Digital Baby Monitor (Single Carrier)" : \
        ((type) == BSA_INT_DIGITAL_VIDEO_MONITOR) ? "Digital Baby Monitor (FHSS)" : \
        ((type) == BSA_INT_GAME_CONTROLLER) ? "Wireless Game Controller (PS2)" : \
        ((type) == BSA_INT_POSSIBLE_INT) ? "Possible Interferer" : \
        ((type) == BSA_INT_JAMMER) ? "Wideband Jammer" : \
        ((type) == BSA_INT_MOTION_DETECTOR) ? "S-Band Motion Detector" : \
        ((type) == BSA_INT_DIGITAL_VIDEO_MONITOR2) ? "Digital Baby Monitor (DSSS)" : \
        ((type) == BSA_INT_YDI_NB_JAMMER) ? "Narrowband Jamming Device" : \
        ((type) == BSA_INT_ZIGBEE) ? "ZigBee Device" : \
        ((type) == BSA_INT_NANO_MOUSE) ? "Nano Wireless Mouse" : \
        ((type) == BSA_INT_AIRHORN) ? "AirHORN Wi-Fi Signal Generator" : \
        ((type) == BSA_INT_GAME_CONTROLLER_XBOX) ? "Wireless Game Controller (XBOX)" : \
        ((type) == BSA_INT_WLAN_FH) ? "WLAN FH" : \
        ((type) == BSA_INT_CANOPY) ? "Canopy" : \
        ((type) == BSA_INT_WIRELESS_BRIDGE) ? "Wireless Bridge" : \
        "Unknown"

#define BSA_INT_24GHZ_COMMON             (BSA_INT_BABY_MONITOR | BSA_INT_FHSS_PHONE \
                                          | BSA_INT_WVIDEO | BSA_INT_DSSS_PHONE \
                                          | BSA_INT_BLUETOOTH | BSA_INT_NARROWBAND_CW \
                                          | BSA_INT_IUWAVE_OVEN | BSA_INT_UWAVE_OVEN \
                                          | BSA_INT_DIGITAL_VIDEO_MONITOR | BSA_INT_GAME_CONTROLLER \
                                          | BSA_INT_POSSIBLE_INT | BSA_INT_JAMMER \
                                          | BSA_INT_MOTION_DETECTOR | BSA_INT_DIGITAL_VIDEO_MONITOR2 \
                                          | BSA_INT_YDI_NB_JAMMER | BSA_INT_ZIGBEE \
                                          | BSA_INT_NANO_MOUSE | BSA_INT_AIRHORN \
                                          | BSA_INT_GAME_CONTROLLER_XBOX | BSA_INT_WLAN_FH)

#define BSA_INT_5GHZ_COMMON              (BSA_INT_BABY_MONITOR | BSA_INT_FHSS_PHONE \
                                          | BSA_INT_WVIDEO | BSA_INT_DSSS_PHONE \
                                          | BSA_INT_NARROWBAND_CW | BSA_INT_POSSIBLE_INT \
                                          | BSA_INT_AIRHORN | BSA_INT_CANOPY \
                                          | BSA_INT_WIRELESS_BRIDGE)

/* Frequency bands ************************************************************/
typedef BSA_UINT32 BSA_FREQ_BAND;

#define BSA_NUM_FREQ_BAND                6
#define BSA_BAND_24GHZ                   0x00000001
#define BSA_BAND_5GHZ_LOW                0x00000002
#define BSA_BAND_5GHZ_MID                0x00000004
#define BSA_BAND_5GHZ_HIGH               0x00000008
#define BSA_BAND_49GHZ                   0x00000010
#define BSA_BAND_CELLULAR                0x00000020

#define BSA_BAND_24GHZ_BASE_FREQ_KHZ     2402000
#define BSA_BAND_5GHZ_LOW_BASE_FREQ_KHZ  5170000
#define BSA_BAND_5GHZ_MID_BASE_FREQ_KHZ  5490000
#define BSA_BAND_5GHZ_HIGH_BASE_FREQ_KHZ 5735000
#define BSA_BAND_49GHZ_BASE_FREQ_KHZ     4910000

/* Bandwidth ******************************************************************/
typedef BSA_UINT32 BSA_BANDWIDTH;

#define BSA_BANDWIDTH_20MHZ              0x00000001
#define BSA_BANDWIDTH_40MHZ              0x00000002
#define BSA_BANDWIDTH_30MHZ              0x00000004
#define BSA_BANDWIDTH_10MHZ              0x00000008
#define BSA_BANDWIDTH_5MHZ               0x00000010
#define BSA_BANDWIDTH_80MHZ              0x00000020
#define BSA_BANDWIDTH_160MHZ             0x00000040

typedef BSA_UINT32 BSA_RADIO_BW;

#define BSA_RADIO_BW_20MHZ               0x00000001
#define BSA_RADIO_BW_22MHZ               0x00000002
#define BSA_RADIO_BW_30MHZ               0x00000004
#define BSA_RADIO_BW_40MHZ               0x00000008
#define BSA_RADIO_BW_42MHZ               0x00000010
#define BSA_RADIO_BW_12MHZ               0x00000020
#define BSA_RADIO_BW_5MHZ                0x00000040
#define BSA_RADIO_BW_10MHZ               0x00000080
#define BSA_RADIO_BW_80MHZ               0x00000100
#define BSA_RADIO_BW_160MHZ              0x00000200
/* AGC Mode *******************************************************************/
typedef BSA_UINT32 BSA_AGC_MODE;

#define BSA_AGC_MODE_HW                  0x1
#define BSA_AGC_MODE_HW_FW               0x2
#define BSA_AGC_MODE_FW                  0x4
#define BSA_AGC_MODE_USER                0x8

/* USB Antenna Selection*******************************************************/
typedef BSA_UINT32 BSA_USB_ANTENNA;

#define BSA_USB_ANTENNA_EXT              0x0
#define BSA_USB_ANTENNA_INT              0x1

/* AICS ***********************************************************************/
#define BSA_AICS_MAX_WLAN_DEV            2
#define BSA_AICS_MAX_CHANNELS            45
/* 2Ghz: 1,2,3,4,5,6,7,8,9,10,11,12,13,14=> 14 Ch
 * 5Ghz lower: 36, 40, 44, 48, 52, 56, 60, 64 => 8 Ch
 * 5GHz mid: 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 => 11 ch
 * 5GHz high: 149, 153, 157, 161, 165 =>  5 ch
 */

/* AICS Averaging Time *********************************************************/
typedef BSA_UINT32 BSA_AICS_AVG_TIME;

#define BSA_AICS_AVG_TIME_5SEC           0x00000001
#define BSA_AICS_AVG_TIME_10SEC          0x00000002
#define BSA_AICS_AVG_TIME_30SEC          0x00000003
#define BSA_AICS_AVG_TIME_1MIN           0x00000004
#define BSA_AICS_AVG_TIME_2MIN           0x00000005
#define BSA_AICS_AVG_TIME_5MIN           0x00000006
#define BSA_AICS_AVG_TIME_10MIN          0x00000007
#define BSA_AICS_AVG_TIME_20MIN          0x00000008

/* clock time *****************************************************************/
typedef struct {
	BSA_UINT32 SecondsHi;
	BSA_UINT32 SecondsLo;
	BSA_INT32  TimeZone;
} BSA_TIME, *PBSA_TIME;

/* FFT Bin ********************************************************************/
#ifdef WIN32
/* #pragma pack(1) */
#endif // endif
typedef struct {
	BSA_INT8 PeakMagDB;
	BSA_INT8 MagDB;
}
#ifndef WIN32
__attribute__((packed))
#endif // endif
BSA_FFT_BIN, *PBSA_FFT_BIN;

#ifdef WIN32
/* #pragma pack() */
#endif // endif

/* FFT data message ***********************************************************/
#ifdef WIN32
/* #pragma pack(1) */
#endif // endif
typedef struct {
	BSA_TIME      Timestamp;
	BSA_UINT32    BinCount;
	BSA_UINT32    StartFreqKhz;
	BSA_UINT32    StopFreqKhz;
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    FFTResHz;
	BSA_UINT32    Reserved;
	BSA_FFT_BIN  *FFTData;
}
#ifndef WIN32
__attribute__((packed))
#endif // endif
BSA_FFT_DATA_MSG, *PBSA_FFT_DATA_MSG;

#ifdef WIN32
/* #pragma pack() */
#endif // endif

/* AGGREGATED FFT data message ***********************************************************/
#define BSA_MAX_CH_PER_BAND   14
#define BSA_MAX_AGGR_FFT_BINS (BSA_MAX_CH_PER_BAND * 128) /* 14 chan = 280 MHz */

#ifdef WIN32
/* #pragma pack(1) */
#endif // endif
typedef struct {
	BSA_TIME      Timestamp;
	BSA_UINT32    BinCount;
	BSA_UINT32    StartFreqKhz;
	BSA_UINT32    StopFreqKhz;
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    FFTResHz;
	BSA_UINT32    Reserved;
	BSA_FFT_BIN  *FFTData;
}
#ifndef WIN32
__attribute__((packed))
#endif // endif
BSA_AGGR_FFT_DATA_MSG, *PBSA_AGGR_FFT_DATA_MSG;

#ifdef WIN32
/* #pragma pack() */
#endif // endif

#ifdef WIN32
/* #pragma pack(1) */
#endif // endif
/* BSA interference message ***************************************************/
typedef struct {
	BSA_TIME            Timestamp;
	BSA_INTERFERER_TYPE InterfererType;
	BSA_UINT32          InterfererId;
	BSA_UINT32          ChanUtil;
	BSA_UINT32          Reserved3;
	BSA_INT32           RssiDbm;
	BSA_UINT32          FreqKhz;
	BSA_UINT32          ImpactedChannelCount;
	BSA_UINT8          *ImpactedChannels;
} BSA_INTERFERENCE_MSG, *PBSA_INTERFERENCE_MSG;

#ifdef WIN32
/* #pragma pack() */
#endif // endif

/* BSA channel utilization message ********************************************/
typedef struct {
	BSA_UINT32    ChannelNum;
	BSA_UINT32    CenterFreqKhz;
	BSA_UINT32    BandwidthMhz;
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    WlanUtil;
	BSA_UINT32    NonWlanUtil;
	BSA_UINT32    Reserved1;
	BSA_UINT32    Reserved2;
} BSA_CHAN_UTIL_DATA, *PBSA_CHAN_UTIL_DATA;

typedef struct {
	BSA_TIME            Timestamp;
	BSA_UINT32          Count;
	BSA_CHAN_UTIL_DATA *ChanUtil;
} BSA_CHAN_UTIL_MSG, *PBSA_CHAN_UTIL_MSG;

/* BSA channel utilization with interference message ***************************/

typedef struct {
	BSA_INTERFERER_TYPE IntType;
	BSA_UINT32          ChanUtil;
	BSA_UINT32          Reserved;
} BSA_INT_CHAN_UTIL_DATA, *PBSA_INT_CHAN_UTIL_DATA;

typedef struct {
	BSA_UINT32    ChannelNum;
	BSA_UINT32    CenterFreqKhz;
	BSA_UINT32    BandwidthMhz;
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    WlanUtil;
	BSA_UINT32    NonWlanUtil;
	BSA_UINT32    TotalUtil;
	BSA_UINT32    NumInterferers;
} BSA_CHAN_UTIL_WITH_INT_DATA, *PBSA_CHAN_UTIL_WITH_INT_DATA;

/* In the BSA_CHAN_UTIL_WITH_INT_MSG data structure the *ChanUtilData pointer
 * points to a buffer that contains channel utilization data for each channel.
 * Each channel's data is given with the BSA_CHAN_UTIL_WITH_INT_DATA data
 * structure.  Each channel's data record is immediately followed by NumInterferers
 * BSA_INT_CHAN_UTIL_DATA data structures which list the contribution to the
 * nonWLAN channel utilization for each interferer that is active on the channel.
 * The number of channels included in the message is given by the Count field.
 * The total amount of data in the buffer is given by DataLen.
 */

typedef struct {
	BSA_TIME      Timestamp;
	BSA_UINT32    SeqNumber;
	BSA_BOOL      Last;
	BSA_SCAN_MODE ScanMode;
	BSA_UINT32    Count;
	BSA_UINT32    DataLen;
	BSA_UINT8    *ChanUtilData;
} BSA_CHAN_UTIL_WITH_INT_MSG, *PBSA_CHAN_UTIL_WITH_INT_MSG;

#define BSA_CHAN_UTIL_WITH_INT_MAX_INT      4
#define BSA_CHAN_UTIL_WITH_INT_MAX_CHAN     16
#define BSA_CHAN_UTIL_WITH_INT_MAX_DATA_LEN (BSA_CHAN_UTIL_WITH_INT_MAX_CHAN * (sizeof(BSA_CHAN_UTIL_WITH_INT_DATA) + BSA_CHAN_UTIL_WITH_INT_MAX_INT * sizeof(BSA_INT_CHAN_UTIL_DATA)))

/* BSA IQ Data message ***************************************************/
#ifdef __SWSA__
typedef struct {
	BSA_TIME      Timestamp;
	BSA_UINT32    SampleCount;
	BSA_UINT32    ChannelNum;
	BSA_UINT32    SampleRateMHz;
	BSA_UINT32    CenterFreqMHz;
	BSA_UINT32    FilterBandwidthMHz;
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    DataFormat;
	BSA_INT32     RxGain;
	BSA_UINT32    OtherRadioCenterFreqMHz;
	BSA_UINT32    OtherRadioBandwidthMHz;
	BSA_UINT32    Reserved1;
	BSA_UINT32    Reserved2;
	BSA_UINT32    Reserved3;
	BSA_INT8     *IData;
	BSA_INT8     *QData;
	BSA_INT8     *RssiScale;
} BSA_IQ_DATA_MSG, *PBSA_IQ_DATA_MSG;

typedef struct {
	BSA_UINT32 TimestampUs;
	BSA_UINT16 Chanspec;
	BSA_UINT16 SequenceNum;
	BSA_INT16  AgcGainDb;
	BSA_INT16  BinCnt;
	BSA_UINT16 Desens;
	BSA_UINT16 CaptureType;
	BSA_UINT16 CoreRev;
	BSA_UINT16 Unit;
	BSA_UINT32 Reserved;
	BSA_INT32  DataBytes;
} BSA_FFTCPX, *PBSA_FFTCPX;

#define BSA_FFTCPX_MAX_LENGTH (16 * 1024)
typedef struct {
	BSA_TIME   Timestamp;
	BSA_UINT32 FFTCount;
	BSA_UINT32 Length;
	BSA_UINT32 Reserved1;
	BSA_UINT32 Reserved2;
	BSA_UINT8 *FFTData; /* Array of BSA_FFTCPX + Data */
} BSA_FFTCPX_DATA_MSG, *PBSA_FFTCPX_DATA_MSG;

typedef struct {
	BSA_TIME   Timestamp;
	BSA_UINT32 SaEnabled;
	BSA_UINT32 Reserved1;
	BSA_UINT32 Reserved2;
} BSA_SA_MODE_STATUS_MSG, *PBSA_SA_MODE_STATUS_MSG;

typedef struct {
	BSA_TIME      Timestamp;
	BSA_SCAN_MODE ScanMode;
	BSA_UINT32    Reserved1;
	BSA_UINT32    Reserved2;
} BSA_SCAN_MODE_CHANGE_MSG, *PBSA_SCAN_MODE_CHANGE_MSG;

typedef struct {
	BSA_TIME   Timestamp;
	BSA_CHAR   IfName[32];
	BSA_UINT32 TimestampValid;
	BSA_UINT32 StartTimestamp;
	BSA_UINT32 EndTimestamp;
	BSA_UINT32 Status;
	BSA_UINT32 Reserved2;
} BSA_SCAN_COMPLETE_MSG, *PBSA_SCAN_COMPLETE_MSG;

#ifndef AIRIQ_OFFLOAD
typedef struct {
	BSA_TIME   Timestamp;
	BSA_BOOL   CoreUp;
	BSA_UINT32 Reserved1;
	BSA_UINT32 Reserved2;
} BSA_OL_REBOOT_MSG, *PBSA_OL_REBOOT_MSG;
#endif // endif

#ifdef WIN32
/* #pragma pack() */
#endif // endif
#endif // endif

/* BSA datagram message *******************************************************/
#define BSA_MAX_DGRAM_DATA 4096
typedef struct {
	BSA_UINT32 Length;
	BSA_UINT8 *RawData;
} BSA_DGRAM_DATA, *PBSA_DGRAM_DATA;

/* BSA connection status message **********************************************/
typedef struct {
	BSA_TIME       Timestamp;
	BSA_CONN_EVENT Event;
	BSA_UINT32     Reserved1;
	BSA_UINT32     Reserved2;
	BSA_UINT32     Reserved3;
} BSA_CONN_STATUS_MSG, *PBSA_CONN_STATUS_MSG;

/* BSA Debug log message ******************************************************/
#define BSA_DEBUG_LOG_MAX_LENGTH 4096
typedef struct {
	BSA_TIME   Timestamp;
	BSA_UINT32 Length;
	BSA_UINT32 Reserved1;
	BSA_UINT32 Reserved2;
	BSA_CHAR  *DebugStream;
} BSA_DEBUG_LOG_MSG, *PBSA_DEBUG_LOG_MSG;

/* IPv4 Address ***************************************************************/
typedef BSA_UINT32 BSA_IP_ADDR, *PBSA_IP_ADDR;

/* FFT Capabilities ***********************************************************/
typedef struct {
	BSA_IE_TYPE Type;
	BSA_UINT32  Length;
	BSA_UINT32  ResHz;
	BSA_UINT32  BinsPerFFT;
	BSA_UINT32  Reserved1;
	BSA_UINT32  Reserved2;
} BSA_FFT_CAP_IE;

/* Channel utilization capabilities *******************************************/
typedef struct {
	BSA_IE_TYPE Type;
	BSA_UINT32  Length;
	BSA_UINT32  Reserved1;
	BSA_UINT32  Reserved2;
} BSA_CHAN_UTIL_CAP_IE;

/* Interference classification capabilities ***********************************/
typedef struct {
	BSA_IE_TYPE         Type;
	BSA_UINT32          Length;
	BSA_INTERFERER_TYPE Interferers;
	BSA_UINT32          Reserved1;
	BSA_UINT32          Reserved2;
	BSA_UINT32          Reserved3;
} BSA_CLASSIFICATION_CAP_IE;

/* Scan mode capabilities *****************************************************/
typedef struct {
	BSA_IE_TYPE   Type;
	BSA_UINT32    Length;
	BSA_SCAN_MODE ScanModes;
	BSA_UINT32    Reserved1;
	BSA_UINT32    Reserved2;
} BSA_SCAN_MODE_CAP_IE;

/* Frequency band capabilities ************************************************/
typedef struct {
	BSA_IE_TYPE   Type;
	BSA_UINT32    Length;
	BSA_FREQ_BAND FreqBands;
	BSA_UINT32    Reserved1;
	BSA_UINT32    Reserved2;
} BSA_FREQ_BAND_CAP_IE;

/* Bandwidth capabilities *****************************************************/
typedef struct {
	BSA_IE_TYPE   Type;
	BSA_UINT32    Length;
	BSA_BANDWIDTH Bandwidths;
	BSA_UINT32    Reserved1;
	BSA_UINT32    Reserved2;
} BSA_BANDWIDTH_CAP_IE;

/* AICS capabilities *****************************************************/
typedef struct {
	BSA_IE_TYPE Type;
	BSA_UINT32  Length;
	BSA_UINT32  Reserved1;
	BSA_UINT32  Reserved2;
} BSA_AICS_CAP_IE;

/* Version information element ************************************************/
typedef struct {
	BSA_IE_TYPE  Type;
	BSA_UINT32   Length;
	BSA_UINT16   ApiMajor;
	BSA_UINT16   ApiMinor;
	BSA_UINT16   DeviceDriverMajor;
	BSA_UINT16   DeviceDriverMinor;
	BSA_UINT16   FirmwareMajor;
	BSA_UINT16   FirmwareMinor;
	BSA_UINT16   DeviceMajor;
	BSA_UINT16   DeviceMinor;
	BSA_SA_MODEL SaModel;
	BSA_UINT32   Reserved2;
} BSA_VERSION_IE, *PBSA_VERSION_IE;

/* Ioctl command */
typedef BSA_UINT32 BSA_IOCTL_CMD;

/* Scan mode channel structure ************************************************/
typedef struct {
	BSA_FREQ_BAND       FreqBand;
	BSA_UINT32          ChannelNumber;
	BSA_UINT32          CenterFreqMhz;
	BSA_BANDWIDTH       FrontEndBw;
	BSA_RADIO_BW        RadioBw;
	BSA_BOOL            DoDisplay;
	BSA_BOOL            DoClassification;
	BSA_BOOL            DoChannelUtilization;
	BSA_BOOL            DoRadar;

	BSA_INTERFERER_TYPE InterfererMap;

	/* This is the amount of time the scan module dwells on this channel during */
	/* normal scanning. */
	BSA_UINT32 DwellTimeMs;

	/* This is the maximum time allowed to dwell on a channel, used by the */
	/* scan state machine when an extended dwell is requested by a classifier. */
	/* E.g. the classifier requests a dwell time of 5 seconds, and MaxDwellTime */
	/* is 1.5 seconds, then the scan state machine will dwell on this channel */
	/* for 1.5 seconds, followed by a sweep of the band(s), followed by a sub- */
	/* sequent 1.5 seconds, followed by another sweep, until the 5 second */
	/* extended dwell period has elapsed. */
	BSA_UINT32 MaxDwellTimeMs;

	/* This is the channel utilization period (ms), used to configure the */
	/* channel utilization hardware counters in the SA. This is the period */
	/* over which to calculate channel utilization. */
	BSA_UINT32 ChanUtilPdMs;

	/* User controlled gain settings */
	BSA_UINT32 agc_ctrl_reg;
	BSA_UINT32 agc_le_cfg_reg;
	BSA_UINT32 agc_setpoint_reg;
	BSA_UINT32 agc_gains_reg;
	BSA_UINT32 agc_rf_reg;
	BSA_UINT32 agc_gain_config_reg;
} BSA_SCAN_CHANNEL, *PBSA_SCAN_CHANNEL;

typedef struct {
	BSA_UINT32       ChannelCount;
	BSA_SCAN_MODE    ScanMode;
	BSA_SCAN_CHANNEL Channels[BSA_CHANNELS_PER_SCAN_MODE];
} BSA_SCAN_CHANNEL_SET;

/* AICS Channel ***************************************************************/
typedef struct {
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    Number;
	BSA_BANDWIDTH Bandwidth;
} BSA_AICS_CHANNEL, *PBSA_AICS_CHANNEL;

/* AICS Channel Information Element *******************************************/
typedef struct {
	BSA_BOOL         Available;
	BSA_AICS_CHANNEL Chan;
} BSA_AICS_CHANNEL_IE, *PBSA_AICS_CHANNEL_IE;

/* AICS Configuration *********************************************************/
typedef struct {
	BSA_BOOL            Enabled;
	BSA_AICS_AVG_TIME   AvgTime;
	BSA_UINT32          MinCsmThreshold;
	BSA_INTERFERER_TYPE IntExclusion;
	BSA_BOOL            PrefChanEnable;
	BSA_AICS_CHANNEL    PrefChan;
	BSA_AICS_CHANNEL    CurrentChan;
} BSA_AICS_CONFIG, *PBSA_AICS_CONFIG;

/* AICS Channel Change Message ************************************************/
typedef struct {
	BSA_TIME         Timestamp;
	BSA_UINT32       WlanDevId;
	BSA_AICS_CHANNEL CurrentChan;
	BSA_AICS_CHANNEL BestChan;
} BSA_AICS_CHAN_CHANGE_MSG, *PBSA_AICS_CHAN_CHANGE_MSG;

/* AICS Statistics ************************************************************/
typedef struct {
	BSA_AICS_CHANNEL Channel;
	BSA_UINT32       ChanSwitchMetric;
	BSA_UINT32       ChanOccMetric;
	BSA_UINT32       Reserved1;
	BSA_UINT32       Reserved2;
} BSA_AICS_STATS, *PBSA_AICS_STATS;

/* AICS Statistics Record ****************************************************/
typedef struct {
	BSA_UINT32      WlanDevId;
	BSA_AICS_CONFIG AicsConfig;
	BSA_UINT32      NumStats;
	BSA_AICS_STATS *AicsStats; /* pointer to array of AICS statistics */
} BSA_AICS_STATS_REC, *PBSA_AICS_STATS_REC;

/* AICS Statistics Message ****************************************************/
typedef struct {
	BSA_TIME            Timestamp;
	BSA_UINT32          NumWlanDev;
	BSA_AICS_STATS_REC *AicsStatsRec; /* pointer to array of AICS stats records */
} BSA_AICS_STATS_MSG, *PBSA_AICS_STATS_MSG;

/* AICS Statistics ************************************************************/
typedef struct {
	BSA_AICS_CHANNEL Channel;
	BSA_UINT32       ChanSwitchMetric;
	BSA_UINT32       ChanOccMetric;
} BSA_AICS_STATS2, *PBSA_AICS_STATS2;

/* AICS Statistics Record ****************************************************/
typedef struct {
	BSA_UINT32      WlanDevId;
	BSA_AICS_CONFIG AicsConfig;
	BSA_UINT32      NumStats;
	BSA_UINT32      Reserved;
} BSA_AICS_STATS_REC2, *PBSA_AICS_STATS_REC2;

/* New AICS Statistics Message ***********************************************/
#define BSA_AICS_STATS_MAX_DATA_LEN \
        BSA_AICS_MAX_WLAN_DEV * (sizeof(BSA_AICS_STATS_REC2) + \
                                 BSA_AICS_MAX_CHANNELS * sizeof(BSA_AICS_STATS2))
typedef struct {
	BSA_TIME   Timestamp;
	BSA_UINT32 NumWlanDev;
	BSA_UINT32 DataLen;
	BSA_UINT8 *pAicsStatsData; /* pointer to array of AICS stats records and stats */
} BSA_AICS_STATS_MSG2, *PBSA_AICS_STATS_MSG2;

/* AICS Channel Ranking *******************************************************/
typedef struct {
	BSA_AICS_CHANNEL CurrentChan;
	BSA_UINT32       NumChannels;
	BSA_AICS_CHANNEL TopChan[BSA_AICS_MAX_CHANNELS]; /* point to array of channels */
} BSA_AICS_CHAN_RANKING, *PBSA_AICS_CHAN_RANKING;

/* Interference log message ***************************************************/
#define BSA_INT_LOG_MAX_CHANNELS 16

#ifdef WIN32
/* #pragma pack(1) */
#endif // endif
typedef struct {
	BSA_UINT32          StartTimestamp;
	BSA_UINT32          StopTimestamp;
	BSA_INTERFERER_TYPE InterfererType;
	BSA_INT8            MaxRssiDbm;
	BSA_INT8            MinRssiDbm;
	BSA_INT8            AvgRssiDbm;
	BSA_INT8            Active;
	BSA_UINT32          FreqKhz;
	BSA_UINT32          InterfererId;
	BSA_UINT32          ChanUtil;
	BSA_UINT32          Reserved;
	BSA_UINT32          ImpactedChannelCount;
	BSA_UINT8           ImpactedChannels[BSA_INT_LOG_MAX_CHANNELS];
} BSA_INTERFERENCE_EVENT_LOG_ENTRY, *PBSA_INTERFERENCE_EVENT_LOG_ENTRY;

#ifdef WIN32
/* #pragma pack() */
#endif // endif

typedef struct {
	BSA_UINT32 CurrentTimestamp;
	BSA_UINT32 EventCount;
	BSA_INTERFERENCE_EVENT_LOG_ENTRY *EventLog;
} BSA_INTERFERENCE_EVENT_LOG, *PBSA_INTERFERENCE_EVENT_LOG;

/* Sensor ID ******************************************************************/
typedef BSA_UINT32 BSA_SENSOR_ID, *PBSA_SENSOR_ID;

/* Device Discovery Data****************************************************/
#define BSA_DEFAULT_DD_PORT        38184
#define BSA_MIN_DISCOVERY_TIME_SEC 2

typedef struct {
	BSA_SENSOR_ID SensorId;
	BSA_IP_ADDR   IpAddr;
	BSA_UINT32    TcpPort;
	BSA_UINT32    NumDevices;
} BSA_DEV_DISCOVERY_DATA, *PBSA_DEV_DISCOVERY_DATA;

/* Device Discovery Event Handler ********************************************/
typedef void (*BSA_DEV_DISCOVERY_HANDLER)(PBSA_DEV_DISCOVERY_DATA pDevDiscoveryData);

/* Alert Severity *************************************************************/
typedef BSA_UINT32 BSA_ALERT_SEVERITY;
#define BSA_ALERT_SEVERITY_INFO     0x00000001      /* Least severe */
#define BSA_ALERT_SEVERITY_NOTICE   0x00000002
#define BSA_ALERT_SEVERITY_WARNING  0x00000004
#define BSA_ALERT_SEVERITY_CRITICAL 0x00000008
#define BSA_ALERT_SEVERITY_EMERG    0x00000010          /* Most severe */

/* Alert Type *****************************************************************/
typedef BSA_UINT32 BSA_ALERT_TYPE;

#define BSA_ALERT_DEV_CONN          0x00000001
#define BSA_ALERT_INT_DET           0x00000002
#define BSA_ALERT_EXC_AICS          0x00000004
#define BSA_ALERT_HIGH_CHAN_UTIL    0x00000008
#define BSA_ALERT_LOW_CHAN_QUAL     0x00000010
#define BSA_ALERT_SW_TRIGGERED      0x00000020

/* Channel Utilization/Quality Types *****************************************/
typedef BSA_UINT32 BSA_CHAN_UTIL_TYPE;

#define BSA_CHAN_UTIL_NON_WLAN      0x00000001
#define BSA_CHAN_UTIL_WLAN          0x00000002
#define BSA_CHAN_UTIL_TOTAL         0x00000004

#define BSA_NUM_CHAN_UTIL_TYPES     3

typedef BSA_UINT32 BSA_CHAN_QUAL_TYPE;

#define BSA_CHAN_QUAL_NON_WLAN      0x00000001
#define BSA_CHAN_QUAL_WLAN          0x00000002
#define BSA_CHAN_QUAL_TOTAL         0x00000004

#define BSA_NUM_CHAN_QUAL_TYPES     3

#define BSA_ALERT_CONFIG_MISMATCH   -1
/* Alert Subtype *****************************************************************/
typedef union {
	BSA_CONN_EVENT      DevConnType;
	BSA_INTERFERER_TYPE IntType;
	BSA_UINT32          WlanDevId;
	BSA_CHAN_UTIL_TYPE  ChanUtilType;
	BSA_CHAN_QUAL_TYPE  ChanQualType;
	BSA_UINT32          SwTriggeredType;
} BSA_ALERT_SUBTYPE, *PBSA_ALERT_SUBTYPE;

/* Channel Type ***************************************************************/
typedef struct {
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    Number;
} BSA_CHANNEL, *PBSA_CHANNEL;

/* Alert Parameters ***********************************************************/
/* Device connection status */
typedef struct {
	BSA_UINT32 Reserved0;
	BSA_UINT32 Reserved1;
} BSA_ALERT_PARAMS_DEV_CONN, *PBSA_ALERT_PARAMS_DEV_CONN;

/* Interferer alert */
/* Triggered if the specified interferer(s) is detected */
typedef struct {
	BSA_INT32  MinRssi;
	BSA_UINT32 MinDurationSec;
	BSA_UINT32 ChannelCount;
	BSA_UINT32 Channels[BSA_CHANNELS_PER_SCAN_MODE];
} BSA_ALERT_PARAMS_INT_DET, *PBSA_ALERT_PARAMS_INT_DET;

/* Excessive AICS events alert
 * Triggered if there are more than CountThreshold AICS events in the last
 * TimeWindowSec seconds
 */
typedef struct {
	BSA_UINT32 TimeWindowSec;
	BSA_UINT32 CountThreshold;
} BSA_ALERT_PARAMS_EXC_AICS, *PBSA_ALERT_PARAMS_EXC_AICS;

/* High Channel Utilization alert */
typedef struct {
	BSA_UINT32  AveragingTime;
	BSA_UINT32  MaxChanUtilThresh;
	BSA_UINT32  MinDurationSec;
	BSA_UINT32  UpdatePeriodSec;
	BSA_UINT32  ChannelListLength;
	BSA_CHANNEL ChannelList[BSA_CHANNELS_PER_SCAN_MODE];
} BSA_ALERT_PARAMS_HIGH_CHAN_UTIL, *PBSA_ALERT_PARAMS_HIGH_CHAN_UTIL;

/* Low channel quality */
typedef struct {
	BSA_UINT32  MinChanQualThresh;
	BSA_UINT32  MinDurationSec;
	BSA_UINT32  UpdatePeriodSec;
	BSA_UINT32  ChannelListLength;
	BSA_CHANNEL ChannelList[BSA_CHANNELS_PER_SCAN_MODE];
} BSA_ALERT_PARAMS_LOW_CHAN_QUAL, *PBSA_ALERT_PARAMS_LOW_CHAN_QUAL;

typedef struct {
	BSA_ALERT_TYPE    AlertType;
	BSA_ALERT_SUBTYPE AlertSubtype;
	union {
		BSA_ALERT_PARAMS_DEV_CONN       DevConn;
		BSA_ALERT_PARAMS_EXC_AICS       ExcAics;
		BSA_ALERT_PARAMS_INT_DET        IntDet;
		BSA_ALERT_PARAMS_HIGH_CHAN_UTIL HighChanUtil;
		BSA_ALERT_PARAMS_LOW_CHAN_QUAL  LowChanQual;
	};
} BSA_ALERT_PARAMS, *PBSA_ALERT_PARAMS;

/* Alert Message Data *********************************************************/
/* Device connection alert message */
typedef struct {
	BSA_CONN_EVENT ConnectionStatusEvent;
	BSA_UINT32     Reserved0;
	BSA_UINT32     Reserved1;
	BSA_UINT32     Reserved2;
} BSA_ALERT_MSG_DEV_CONN, *PBSA_ALERT_MSG_DEV_CONN;

typedef struct {
	BSA_UINT32 DurationSec;
	BSA_INT8   RssiDbmMax;
	BSA_INT8   RssiDbmMin;
	BSA_INT8   RssiDbmMean;
	BSA_UINT8  Active;
	BSA_UINT32 FreqKHzLast;
	BSA_UINT32 FreqKHzMax;
	BSA_UINT32 FreqKHzMin;
	BSA_UINT32 FreqKHzMean;
	BSA_UINT32 ChanUtilLast;
	BSA_UINT32 ChanUtilMin;
	BSA_UINT32 ChanUtilMax;
	BSA_UINT32 ChanUtilMean;
	BSA_UINT64 ClusterRecordId;
	BSA_UINT32 ImpactedChannelCount;
	BSA_UINT8  ImpactedChannels[BSA_INT_LOG_MAX_CHANNELS];
} BSA_ALERT_MSG_INT_DET, *PBSA_ALERT_MSG_INT_DET;

typedef struct {
	BSA_UINT32 AicsEvtCount;
} BSA_ALERT_MSG_EXC_AICS, *PBSA_ALERT_MSG_EXC_AICS;

typedef struct {
	BSA_CHANNEL Channel;
	BSA_UINT32  AvgChanUtil;
	BSA_UINT32  DurationSec;
} BSA_ALERT_MSG_HIGH_CHAN_UTIL, *PBSA_ALERT_MSG_HIGH_CHAN_UTIL;

typedef struct {
	BSA_CHANNEL Channel;
	BSA_UINT32  AvgChanQual;
	BSA_UINT32  DurationSec;
} BSA_ALERT_MSG_LOW_CHAN_QUAL, *PBSA_ALERT_MSG_LOW_CHAN_QUAL;

typedef struct {
	BSA_UINT32 Reserved0;
} BSA_ALERT_MSG_SW_TRIGGERED, *PBSA_ALERT_MSG_SW_TRIGGERED;

typedef union {
	BSA_ALERT_MSG_DEV_CONN       DevConn;
	BSA_ALERT_MSG_INT_DET        IntDet;
	BSA_ALERT_MSG_EXC_AICS       ExcAics;
	BSA_ALERT_MSG_HIGH_CHAN_UTIL HighChanUtil;
	BSA_ALERT_MSG_LOW_CHAN_QUAL  LowChanQual;
	BSA_ALERT_MSG_SW_TRIGGERED   SwTriggered;
} BSA_ALERT_MSG_DATA, *PBSA_ALERT_MSG_DATA;

typedef struct {
	BSA_UINT32         StartTimestamp;
	BSA_UINT32         StopTimestamp;
	BSA_ALERT_SEVERITY Severity;
	BSA_ALERT_TYPE     Type;
	BSA_ALERT_SUBTYPE  Subtype;
	BSA_BOOL           Active;
	BSA_UINT32         RecordingId;
	BSA_ALERT_MSG_DATA Data;
} BSA_ALERT_MSG, *PBSA_ALERT_MSG;

typedef struct {
	BSA_UINT32    Timestamp;
	BSA_BOOL      Update;
	BSA_ALERT_MSG AlertMsg;
}
#ifndef WIN32
__attribute__((packed))
#endif // endif
BSA_ALERT_EVT_MSG, *PBSA_ALERT_EVT_MSG;

/* Specan option *****************************************************************/
typedef BSA_UINT32 BSA_SPECAN_OPTION;
#define BSA_SPECAN_OPTION_DENSITY               0x1
#define BSA_SPECAN_OPTION_WINDOW                0x2
#define BSA_SPECAN_OPTION_RADAR_CLASS           0x3
#define BSA_SPECAN_OPTION_UPDATE_PERIOD         0x4
#define BSA_SPECAN_OPTION_CU_RSSI_THR           0x5
#define BSA_SPECAN_OPTION_CASO_CONFIG           0x6
#define BSA_SPECAN_OPTION_CASO_STATUS           0x7
#define BSA_SPECAN_OPTION_CASO_RECALC           0x8
#define BSA_SPECAN_OPTION_ENABLE_SA             0x9
/* #define BSA_SPECAN_OPTION_SCAN_SPEED  0xA */
#define BSA_SPECAN_OPTION_SAMPLE_COLLECT        0xB
#define BSA_SPECAN_OPTION_CU_SMOOTHING_FACTOR   0xC
#define BSA_SPECAN_OPTION_AGC_CONFIG            0xD
#define BSA_SPECAN_OPTION_FFT_DECIMATION_CONFIG 0xF
#define BSA_SPECAN_OPTION_CLASS_ACCURACY        0x10
#define BSA_SPECAN_OPTION_CU_REL_THR            0x11
#define BSA_SPECAN_OPTION_UCE_CONFIG            0x12
#define BSA_SPECAN_OPTION_CLASSIFIER_CONFIG     0x13
#define BSA_SPECAN_OPTION_CLASSIFIER_RESET      0x14
#define BSA_SPECAN_OPTION_DISABLE_AGC_CAL       0x15

typedef BSA_UINT32 BSA_SPECAN_WINDOW_TYPE;
#define BSA_SPECAN_WINDOW_TYPE_RECT             0
#define BSA_SPECAN_WINDOW_TYPE_HAMMING          1
#define BSA_SPECAN_WINDOW_TYPE_HANN             2
#define BSA_SPECAN_WINDOW_TYPE_BH               3

typedef BSA_UINT32 BSA_SPECAN_DENSITY;

typedef BSA_UINT32 BSA_RADAR_CLASS;
#define BSA_RADAR_CLASS_FCC                     1
#define BSA_RADAR_CLASS_ETSI                    2
#define BSA_RADAR_CLASS_TELEC                   3

typedef BSA_UINT32 BSA_SCAN_SPEED;
#define BSA_SCAN_SPEED_MIN                      0
#define BSA_SCAN_SPEED_MAX                      255

/* CASO: Channel adaptive sensitivity optimization.
 * This is an algorithm that allows the firmware to
 * adjust receiver gain to mitigate the effects of
 * received EMI/ambient signals. By turning down
 * receiver gain, the rx power of the background noise
 * can be pushed below the sensitivity level of the
 * baseband receiver, providing the signal analyzer
 * with a accurate picture of the "true" RF environment.
 */
typedef BSA_UINT32 BSA_CASO_MODE;
/* Fix sens adjustment at supplied levels. Equivalent to enable/disable
 * of the CASO algorithm.
 */
#define BSA_CASO_MODE_FIXED                     1

/* Semi mode: Init @ supplied levels and adjust till stable, then freeze. */
#define BSA_CASO_MODE_SEMI                      2

/* Semi w/ desens: Same as Semi, but after frozen allow auto desensitization
 * as necessary.
 */
#define BSA_CASO_MODE_SEMI_WITH_DESENS          3

/* Full auto mode: Init with supplied values, adjusting sensitivity both up
 * and down, continuously adapting to background noise level.
 */
#define BSA_CASO_MODE_FULL                      4

typedef struct {
	BSA_UINT32    ChannelCount;
	BSA_INT32     SensAdjustmentDb[BSA_CHANNELS_PER_SCAN_MODE];
	BSA_INT32     SensAdjustmentMaxDb;
	BSA_INT32     SensAdjustmentMinDb;
	BSA_INT32     NoiseThresholdDbm;
	BSA_CASO_MODE Mode;
} BSA_CASO_CONFIG, *PBSA_CASO_CONFIG;

typedef struct {
	BSA_UINT32    ChannelCount;
	BSA_SCAN_MODE ScanMode;
	BSA_INT32     SensAdjustmentDb[BSA_CHANNELS_PER_SCAN_MODE];
} BSA_CASO_STATUS, *PBSA_CASO_STATUS;

#define BSA_INTERFACE_NAME_LEN 16
typedef struct {
	BSA_UINT32    FreqKhz;
	BSA_FREQ_BAND FreqBand;
	BSA_UINT32    ChannelNum;
	BSA_CHAR      InterfaceName[BSA_INTERFACE_NAME_LEN];
} BSA_SAMPLE_COLLECT, *PBSA_SAMPLE_COLLECT;

typedef struct {
	union {
		BSA_SPECAN_WINDOW_TYPE WindowType;
		BSA_SPECAN_DENSITY     Density;
		BSA_RADAR_CLASS        RadarClass;
		BSA_UINT32             UpdatePeriod;
		BSA_INT32              CuRssiThrDbm;
		BSA_CASO_CONFIG        CasoConfig;
		BSA_CASO_STATUS        CasoStatus;
		BSA_UINT32             EnableSa;
		BSA_SAMPLE_COLLECT     SampleCollect;
		BSA_UINT32             ClassAccuracy;
		BSA_UINT32             ResetType;
		BSA_BOOL               DisableAgcCal;
	};
} BSA_SPECAN_OPTION_DATA, *PBSA_SPECAN_OPTION_DATA;

/* Debug output definitions *********************************************************/
#define BSA_MAX_FILESPEC_LENGTH 128

/* Target for BSA Log messages */
typedef BSA_UINT32 BSA_LOG_TARGET;
#define BSA_LOG_STDOUT          0x00000001
#define BSA_LOG_STDERR          0x00000002
#define BSA_LOG_SYSLOG          0x00000004
#define BSA_LOG_FILE            0x00000008

/* This is a modifier to the log target that causes */
/* the log output to be prefixed with a timestamp. */
#define BSA_LOG_TIMESTAMP       0x80000000

/* Type of log messages */
typedef BSA_UINT32 BSA_LOG_TYPE;

/* Error log values */
/* BSA_LOG_TYPE enumeration: */
#define BSA_LOG_CRITICAL        0x00000001
#define BSA_LOG_ERROR           0x00000002
#define BSA_LOG_WARNING         0x00000004
#define BSA_LOG_INFO            0x00000008
#define BSA_LOG_API             0x00000010
#define BSA_LOG_API_PARAM       0x00000020
#define BSA_LOG_FCN_ENTRY       0x00000040
#define BSA_LOG_NET_MSG         0x00000080
#define BSA_LOG_DEBUG0          0x00000100
#define BSA_LOG_DEBUG1          0x00000200
#define BSA_LOG_DEBUG2          0x00000400
#define BSA_LOG_DEBUG3          0x00000800

#define BSA_LOG_DEFAULT         (BSA_LOG_CRITICAL | BSA_LOG_ERROR | BSA_LOG_WARNING | BSA_LOG_INFO)
#define BSA_LOG_FULL_DEBUG      (BSA_LOG_DEFAULT | BSA_LOG_DEBUG0 | BSA_LOG_DEBUG1 | BSA_LOG_DEBUG2 | BSA_LOG_DEBUG3)

#ifdef WIN32
#pragma pack()
#endif // endif

/*  */
/* Mgrsvc startup flags: For configuring certain mgrsvc functions when */
/* loaded as a dynamic library. */
/*  */
#define BSA_MGRSVC_FLAGS_MAGIC 0xaa55bb66
typedef struct {
	BSA_UINT32 Magic;
	BSA_CHAR   SwsaConfigFile[256];
	/* Add future options below. */
} BSA_MGRSVC_FLAGS, *PBSA_MGRSVC_FLAGS;
#endif // endif
