/*
 * Common (OS-independent) portion of
 * WLM (Wireless LAN Manufacturing) test library.
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
 * $Id: wlm.h 758241 2018-04-18 07:43:37Z $
 */

#ifndef _wlm_h
#define _wlm_h

#ifdef __cplusplus
extern "C" {
#endif // endif

/**
\mainpage mfgtest_api.h

Overview:

The MFG API DLL allows a user to talk to a DUT by providing functions
that abstract out the lower level details of the driver.

The goal of this API is to make testing easier by providing enough
functionality for the user to perform various tasks without
requiring the user to know anything about the driver internals and,
in addition, any differences there may be between drivers.

*/

#ifndef TRUE
#define TRUE 	1
#endif // endif
#ifndef FALSE
#define FALSE 	0
#endif // endif

#ifndef NULL
#define NULL 0
#endif // endif

#define WLM_MAX_FILENAME_LEN 256

#if defined(WIN32)
#define WLM_FUNCTION __declspec(dllexport)
#else
#define WLM_FUNCTION
#endif /* WIN32 */

#define WLM_VERSION_STR "wlm version: 7.10.31.0 (support 4335, 4345, 4350 and more )"

/* Supported DUT interfaces. */
/* Values are used to select the interface used to talk to the DUT. */
enum {
	WLM_DUT_LOCAL  = 0,	/* client and DUT on same device */
	WLM_DUT_SERIAL = 1,	/* client to DUT via serial */
	WLM_DUT_SOCKET = 2,	/* client to DUT via Ethernet */
	WLM_DUT_WIFI   = 3,	/* client to DUT via WLAN */
	WLM_DUT_DONGLE = 4	/* client to DUT dongle via serial */
};
typedef int WLM_DUT_INTERFACE;

/* Supported server ports. */
/* Values are used to configure server port (for WLM_DUT_SOCKET only). */
enum {
	WLM_DEFAULT_DUT_SERVER_PORT = 8000	/* default */
};
typedef int WLM_DUT_SERVER_PORT;

/* Supported DUT OS. */
/* Values are used to select the OS of the DUT. */
enum {
	WLM_DUT_OS_LINUX = 1,  	/* Linux DUT */
	WLM_DUT_OS_WIN32 = 2 	/* Win32 DUT */
};
typedef int WLM_DUT_OS;

/* Supported join modes. */
/* Values are used to select the mode used to join a network. */
enum {
	WLM_MODE_IBSS = 0,   /* IBSS Mode. (Adhoc) */
	WLM_MODE_BSS = 1,    /* BSS Mode. */
	WLM_MODE_AUTO = 2    /* Auto. */
};
typedef int WLM_JOIN_MODE;

/* Supported bands. */
/* Values are used to select the band used for testing. */
enum {
	WLM_BAND_AUTO = 0,	/* auto select */
	WLM_BAND_5G = 1,        /* 5G Band. */
	WLM_BAND_2G = 2,        /* 2G Band. */
	WLM_BAND_DUAL = WLM_BAND_2G | WLM_BAND_5G	/* Dual Band. */
};
typedef int WLM_BAND;

/* Supported gmode. */
/* Values are used to select gmode used for testing. */
enum {
	WLM_GMODE_LEGACYB = 0,
	WLM_GMODE_AUTO = 1,
	WLM_GMODE_GONLY = 2,
	WLM_GMODE_BDEFERED = 3,
	WLM_GMODE_PERFORMANCE = 4,
	WLM_GMODE_LRS = 5
};
typedef int WLM_GMODE;

/* Supported legacy rates. */
/* Values are used to select the rate used for testing. */
enum {
	WLM_RATE_AUTO = 0,
	WLM_RATE_1M = 2,
	WLM_RATE_2M = 4,
	WLM_RATE_5M5 = 11,
	WLM_RATE_6M = 12,
	WLM_RATE_9M = 18,
	WLM_RATE_11M = 22,
	WLM_RATE_12M = 24,
	WLM_RATE_18M = 36,
	WLM_RATE_24M = 48,
	WLM_RATE_36M = 72,
	WLM_RATE_48M = 96,
	WLM_RATE_54M = 108
};
typedef int WLM_RATE;

/* Supported MCS rates */
enum {
	WLM_MCS_RATE_0 = 0,
	WLM_MCS_RATE_1 = 1,
	WLM_MCS_RATE_2 = 2,
	WLM_MCS_RATE_3 = 3,
	WLM_MCS_RATE_4 = 4,
	WLM_MCS_RATE_5 = 5,
	WLM_MCS_RATE_6 = 6,
	WLM_MCS_RATE_7 = 7,
	WLM_MCS_RATE_8 = 8,
	WLM_MCS_RATE_9 = 9,
	WLM_MCS_RATE_10 = 10,
	WLM_MCS_RATE_11 = 11,
	WLM_MCS_RATE_12 = 12,
	WLM_MCS_RATE_13 = 13,
	WLM_MCS_RATE_14 = 14,
	WLM_MCS_RATE_15 = 15,
	WLM_MCS_RATE_16 = 16,
	WLM_MCS_RATE_17 = 17,
	WLM_MCS_RATE_18 = 18,
	WLM_MCS_RATE_19 = 19,
	WLM_MCS_RATE_20 = 20,
	WLM_MCS_RATE_21 = 21,
	WLM_MCS_RATE_22 = 22,
	WLM_MCS_RATE_23 = 23,
	WLM_MCS_RATE_24 = 24,
	WLM_MCS_RATE_25 = 25,
	WLM_MCS_RATE_26 = 26,
	WLM_MCS_RATE_27 = 27,
	WLM_MCS_RATE_28 = 28,
	WLM_MCS_RATE_29 = 29,
	WLM_MCS_RATE_30 = 30,
	WLM_MCS_RATE_31 = 31,
	WLM_MCS_RATE_32 = 32
};

typedef int WLM_MCS_RATE;

/* Supported STF mode */
enum {
	WLM_STF_MODE_SISO = 0, 	/* stf mode SISO */
	WLM_STF_MODE_CDD = 1,  	/* stf mode CDD  */
	WLM_STF_MODE_STBC = 2,  /* stf mode STBC */
	WLM_STF_MODE_SDM  = 3	/* stf mode SDM  */
};
typedef int WLM_STF_MODE;

/* Supported PLCP preambles. */
/* Values are used to select the preamble used for testing. */
enum {
	WLM_PREAMBLE_AUTO = -1,
	WLM_PREAMBLE_SHORT = 0,
	WLM_PREAMBLE_LONG = 1
};
typedef int WLM_PREAMBLE;

/* Supported MIMO preamble types */
enum {
	WLM_MIMO_MIXEDMODE = 0,
	WLM_MIMO_GREENFIELD = 1
};
typedef int WLM_MIMO_PREAMBLE;

enum {
	WLM_CHAN_FREQ_RANGE_2G = 0,
	WLM_CHAN_FREQ_RANGE_5GL = 1,
	WLM_CHAN_FREQ_RANGE_5GM = 2,
	WLM_CHAN_FREQ_RANGE_5GH = 3,
	WLM_CHAN_FREQ_RANGE_5GLL_5BAND = 4,
	WLM_CHAN_FREQ_RANGE_5GLH_5BAND = 5,
	WLM_CHAN_FREQ_RANGE_5GML_5BAND = 6,
	WLM_CHAN_FREQ_RANGE_5GMH_5BAND = 7,
	WLM_CHAN_FREQ_RANGE_5GH_5BAND = 8,
	WLM_CHAN_FREQ_RANGE_5G_BAND0 = 1, /* AC 5G sub band */
	WLM_CHAN_FREQ_RANGE_5G_BAND1 = 2, /* AC 5G sub band */
	WLM_CHAN_FREQ_RANGE_5G_BAND2 = 3, /* AC 5G sub band */
	WLM_CHAN_FREQ_RANGE_5G_BAND3 = 4, /* AC 5G sub band */
	WLM_CHAN_FREQ_RANGE_5G_4BAND = 5,
	WLM_CHAN_FREQ_RANGE_5G_BAND4 = 5
};
typedef int WLM_BANDRANGE;

enum {
	WLM_BANDWIDTH_20MHZ = 0,
	WLM_BANDWIDTH_40MHZ = 1,
	WLM_BANDWIDTH_80MHZ = 2
};
typedef int WLM_BANDWIDTH;

/* Supported image formats modes. */
/* Values are used to select the type of image to read/write. */
enum {
	WLM_TYPE_AUTO = 0,  /* Auto mode. */
	WLM_TYPE_SROM = 1,  /* SROM. */
	WLM_TYPE_OTP = 2    /* OTP. */
};
typedef int WLM_IMAGE_TYPE;

/* Supported authentication type. */
/* Values are used to select the authentication type used to join a network. */
enum {
	WLM_TYPE_OPEN = 0,    /* Open */
    WLM_TYPE_SHARED = 1   /* Shared */
};
typedef int WLM_AUTH_TYPE;

/* Supported authentication mode. */
/* Values are used to select the authentication mode used to join a network. */
enum {
	WLM_WPA_AUTH_DISABLED = 0x0000,	/* Legacy (i.e., non-WPA) */
	WLM_WPA_AUTH_NONE = 0x0001,		/* none (IBSS) */
	WLM_WPA_AUTH_PSK = 0x0004,		/* Pre-shared key */
	WLM_WPA2_AUTH_PSK = 0x0080		/* Pre-shared key */
};
typedef int WLM_AUTH_MODE;

/* WLAN Security Encryption. */
/* Values are used to select the type of encryption used for testing. */
enum {
	WLM_ENCRYPT_NONE = 0,    /* No encryption. */
	WLM_ENCRYPT_WEP = 1,     /* WEP encryption. */
	WLM_ENCRYPT_TKIP = 2,    /* TKIP encryption. */
	WLM_ENCRYPT_AES = 4,     /* AES encryption. */
	WLM_ENCRYPT_WSEC = 8,    /* Software WSEC encryption. */
	WLM_ENCRYPT_FIPS = 0x80  /* FIPS encryption. */
};
typedef int WLM_ENCRYPTION;

typedef struct wlmRxIQEstChannel_t {
	int channel;
	float rxIqEst;
} wlmRxIQEstChannel_t;

typedef struct wlmRxIQEstSweepReult_t {
	int numChannels;
	wlmRxIQEstChannel_t value[1];
} wlmRxIQEstSweepReult_t;

/* Country abbreviative code */
#define WLM_COUNTRY_ALL "ALL"	/* Default country code */
#define WLM_COUNTRY_JAPAN "JP"     /* Japan country code */
#define WLM_COUNTRY_KOREA "KR"     /* Korea country code */

/* number of 5G channel offsets */
#define CGA_5G_OFFSETS_LEN 24

/* number of 2G channel offsets */
#define CGA_2G_OFFSETS_LEN 14

/* Initialization related functions */

/* Get the WLM version.
 * param[out] buffer version string for current wlm (dll, lib and dylib)
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmWLMVersionGet(const char **buffer);

/* Performs any initialization required internally by the DLL.
 * NOTE: This method needs to be called before any other API function.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmApiInit(void);

/* Performs any cleanup required internally by the DLL.
 * NOTE: This method needs to be called by the user at the end of the application.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmApiCleanup(void);

/* Selects the interface to use for talking to the DUT
 *
 * param[in] ifType The desired interface.
 * param[in] ifName Expected value depends on ifType:
 *   For ifType = WLM_DUT_DONGLE, ifName is the COM port to use (ex: "COM1").
 *   For ifType = WLM_DUT_SOCKET, ifName is the IP address to use
 *		(ex: "192.168.1.1" or "localhost").
 * param[in] dutServerPort Server port of DUT (used only for WLM_DUT_SOCKET).
 * param[in] dutOs Operating system of DUT.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSelectInterface(WLM_DUT_INTERFACE ifType, char *ifName,
	WLM_DUT_SERVER_PORT dutServerPort, WLM_DUT_OS dutOs);

/* Creates a secondary interface on SDB supported products
* param[in] secondary interface type, sta or ap or nan
* param[in] optional interface type args.
* Empty string for default usage, e.g. ""
* Or specify optional parameters [-m <MAC-address>] [-b <BSSID>] [-f if_index]
* [-c  <wlc_index>]. E.g. "-m 00:11:22:33:44:55 -f 1 -c 1"
* return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmCreateSecondaryInterface(char *ifType, char *ifTypeArgs);

/* Selects the interface index
 * param[in] options such as "-i" and "-w" to specify the interface.
 * param[in] interface index such as eth1, wl0.1 or wlc index for rsdb cases.
 * return - True for success, false for failure.
 */

WLM_FUNCTION
int wlmSelectIntfidx(char *opt, char *idx);

/* Selects the interface to use for talking to the DUT
 *
 * param[in] ifType The desired interface.
 * param[in] ifName Expected value depends on ifType:
 *   For ifType = WLM_DUT_DONGLE, ifName is the COM port to use (ex: "COM1").
 *   For ifType = WLM_DUT_SOCKET, ifName is the IP address to use
 *		(ex: "192.168.1.1" or "localhost").
 * param[in] dutServerPort Server port of DUT (used only for WLM_DUT_SOCKET).
 * param[in] dutOs Operating system of DUT.
 * param[in] baud rate for serial interface. Default 115200.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSelectInterfaceAndBaud(WLM_DUT_INTERFACE ifType, char *ifName,
	WLM_DUT_SERVER_PORT dutServerPort, WLM_DUT_OS dutOs, int baud);

/* Gets the WLAN driver version.
 *
 * param[in] buffer Buffer for the version string.
 * param[in] length Maximum length of buffer.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmVersionGet(char *buffer, int length);

/* Enables or disables wireless adapter.
 *
 * param[in] enable Set to true to bring adapter up, false to bring down.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmEnableAdapterUp(int enable);

/* Check if adapter is currently up.
 *
 * param[in] up Return the up/down state of adapter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmIsAdapterUp(int *up);

/* Enables or disables Minimum Power Consumption (MPC).
 * MPC will automatically turn off the radio if not associated to a network.
 *
 * param[in] enable Set to true to enable MPC, false to disable MPC.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmMinPowerConsumption(int enable);

/* Sets the operating channel of the DUT.
 *
 * param[in] channel Desired channel.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmChannelSet(int channel);

/* Sets the operating legacy rate (bg_rate or a_rate) for the DUT.
 *
 * param[in] rate Desired rate.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRateSet(WLM_RATE rate);

/* Sets the operating legacy rate (nrate) of the DUT.
 *
 * param[in] rate Desired rate.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmLegacyRateSet(WLM_RATE rate);

/* Sets the operating mcs rate and STF of the DUT
 * param[in] Desired mcs rate [0, 7] for SISO (single-in-single-out) device.
 * param[in] Stf mode, 0=SISO, 1= CDD, 2=STBC, 3=SDM
 * return - True for sucess, false for failure.
*/
WLM_FUNCTION
int wlmMcsRateSet(WLM_MCS_RATE mcs_rate, WLM_STF_MODE stf_mode);

/* Sets the operating HT rate
 * param[in] Desired mcs rate [0, 23]
 * param[in] 1 = Use STBC extention; 0 = Don't use STBC extention
 * param[in] 1 = Uses LDPC encoding; 0 = don't use LDPC encoding
 * param[in] 1 = Use Short Guard Interval; 0 = don't use Short Guard Interval
 * param[in] Number of tx chains beyond the minimum required for Space-Time-Streams
 * param[in] transmition bandwidth in MHz; 20, 40, 80
 * return - True for sucess, false for failure.
*/
WLM_FUNCTION
int wlmHTRateSet(WLM_MCS_RATE mcs_rate, int stbc, int ldpc, int sgi, int tx_exp, int bw);

/* Sets the operating VHT rate
 * param[in] Desired mcs rate [0, 9]
 * param[in] Number  of Spacial Stream
 * param[in] 1 = Use STBC extention; 0 = Don't use STBC extention
 * param[in] 1 = Uses LDPC encoding; 0 = don't use LDPC encoding
 * param[in] 1 = Use Short Guard Interval; 0 = don't use Short Guard Interval
 * param[in] Number of tx chains beyond the minimum required for Space-Time-Streams
 * param[in] transmition bandwidth in MHz; 20, 40, 80
 * return - True for sucess, false for failure.
*/
WLM_FUNCTION
int wlmVHTRateSet(WLM_MCS_RATE mcs_rate, int NSS, int stbc, int ldpc, int sgi, int tx_exp, int bw);

/* Sets the PLCP preamble.
 *
 * param[in] preamble Desired preamble.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPreambleSet(WLM_PREAMBLE preamble);

/* Set the band.
 *
 * param[in] The Desired band.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmBandSet(WLM_BAND band);

/* Get current band.
 *
 * param[out] The current band.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmBandGet(WLM_BAND *band);

/* Get available bands.
 *
 * param[in] Available bands returned in the form of WLM_BANDs added.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmGetBandList(WLM_BAND * bands);

/* Set gmode.
 *
 * param[in] The desired gmode.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmGmodeSet(WLM_GMODE gmode);

/* Sets the receive antenna.
 *
 * param[in] antenna Desired antenna.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRxAntSet(int antenna);

/* Sets the transmit antenna.
 *
 * param[in] antenna Desired antenna.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxAntSet(int antenna);

/* Retrieves the current estimated power in milli-dBm.
 *
 * param[out] estPower Power value returned.
 * param[in] chain For MIMO device, specifies which chain to retrieve power value from.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmEstimatedPowerGet(int *estPower, int chain);

/* Retrieves the current TX power in milli-dB.
 *
 * param[in] power Power value returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxPowerGet(int *power);

/* Sets the current TX power.
 *
 * param[in] powerValue Desired power value. Expected to be in milli-dB. Use -1 to restore default.
 *           valid range [-128000, 127000)
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxPowerSet(int powerValue);

/* Retrieves the tx power control parameters for SISO system.
 *
 * param[in] bandrange. The desired band range for getting PA parameters.
 * param[out] a1 PA parameter returned.
 * param[out] b0 PA parameter returned.
 * param[out] b1 PA parameter returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPaParametersGet(WLM_BANDRANGE bandrange,
	unsigned int *a1, unsigned int *b0, unsigned int *b1);

/* Sets tx power control parameters for SISO system.
 *
 * param[in] bandrange. The desired band range for getting PA parameters.
 * param[in] a1 PA parameter.
 * param[in] b0 PA parameter.
 * param[in] b1 PA parameter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPaParametersSet(WLM_BANDRANGE bandrange,
	unsigned int a1, unsigned int b0, unsigned int b1);

/* Retrieves the tx power control parameters for MIMO system.
 *
 * param[in] bandrange The desired band range for getting PA parameters.
 * param[in] chain. The desired tx chain for getting PA parameters
 * param[out] a1 PA parameter returned.
 * param[out] b0 PA parameter returned.
 * param[out] b1 PA parameter returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmMIMOPaParametersGet(WLM_BANDRANGE bandrange, int chain,
	unsigned int *a1, unsigned int *b0, unsigned int *b1);

/* Sets tx power control parameters for MIMO system.
 *
 * param[in] bandrange. The desired band range for getting PA parameters.
 * param[in] chain. The desired tx chain for getting PA parameters.
 * param[in] a1 PA parameter.
 * param[in] b0 PA parameter.
 * param[in] b1 PA parameter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmMIMOPaParametersSet(WLM_BANDRANGE bandrange, int chain,
	unsigned int a1, unsigned int b0, unsigned int b1);

/* Retrieves the tx power control parameters for AC system.
 *
 * param[in] bandrange. The desired band range for getting PA parameters.
 *           WLM_CHAN_FREQ_RANGE_2G 0
 *           WLM_CHAN_FREQ_RANGE_5G_BAND0 1
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND1 2
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND2 3
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND3 4
 * param[in] modetype. The desired modulation or bw mode. Eg. 4335
 *           pa2g-ofdm-20mh 0
 *           pa2g-cck-20mhz 1
 *           pa2g-reserved 2
 *           pa5g-20mhz 0
 *           pa5g-40mhz 1
 *           pa5g-80mhz 2
 * param[in] chain. The desired tx chain for getting PA parameters
 * param[out] a1 PA parameter returned.
 * param[out] b0 PA parameter returned.
 * param[out] b1 PA parameter returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmACPaParametersGet(WLM_BANDRANGE bandrange, int modetype,
	unsigned int *a1, unsigned int *b0, unsigned int *b1);

/* Sets tx power control parameters for AC system.
 *
 * param[in] bandrange. The desired band range for getting PA parameters.
 *           WLM_CHAN_FREQ_RANGE_2G 0
 *           WLM_CHAN_FREQ_RANGE_5G_BAND0 1
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND1 2
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND2 3
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND3 4
 * param[in] modetype. The desired modulation or bw mode. Eg. 4335
 *           pa2g-ofdm-20mh 0
 *           pa2g-cck-20mhz 1
 *           pa2g-reserved 2
 *           pa5g-20mhz 0
 *           pa5g-40mhz 1
 *           pa5g-80mhz 2
 * param[in] a1 PA parameter.
 * param[in] b0 PA parameter.
 * param[in] b1 PA parameter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmACPaParametersSet(WLM_BANDRANGE bandrange, int modetype,
	unsigned int a1, unsigned int b0, unsigned int b1);

/* Retrieves the tx power control parameters for AC system using Rev12 SROM format
 *
 * param[in] bandrange. The desired band range for getting PA parameters.
 *           WLM_CHAN_FREQ_RANGE_2G 0
 *           WLM_CHAN_FREQ_RANGE_5G_BAND0 1
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND1 2
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND2 3
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND3 4
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND4 5
 * param[in] chain
 * param[in] bandwidth.
 *           WLM_BANDWIDTH_20MHZ 0
 *           WLM_BANDWIDTH_40MHZ 1
 *           WLM_BANDWIDTH_80MHZ 2 (only applies to 5G band)
 * param[out] b0 PA parameter.
 * param[out] b1 PA parameter.
 * param[out] b2 PA parameter.
 * param[out] b3 PA parameter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmACPaParametersRev12Get(WLM_BANDRANGE bandrange, int chain, WLM_BANDWIDTH bandwidth,
	unsigned int *b0, unsigned int *b1, unsigned int *b2, unsigned int *b3);

/* Sets tx power control parameters for AC system using Rev12 SROM format.
 *
 * param[in] bandrange. The desired band range
 *           WLM_CHAN_FREQ_RANGE_2G 0
 *           WLM_CHAN_FREQ_RANGE_5G_BAND0 1
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND1 2
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND2 3
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND3 4
 *	     WLM_CHAN_FREQ_RANGE_5G_BAND4 5
 * param[in] chain
 * param[in] bandwidth.
 *           WLM_BANDWIDTH_20MHZ 0
 *           WLM_BANDWIDTH_40MHZ 1
 *           WLM_BANDWIDTH_80MHZ 2 (only applies to 5G band)
 * param[in] b0 PA parameter.
 * param[in] b1 PA parameter.
 * param[in] b2 PA parameter.
 * param[in] b3 PA parameter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmACPaParametersRev12Set(WLM_BANDRANGE bandrange, int chain, WLM_BANDWIDTH bandwidth,
	unsigned int b0, unsigned int b1, unsigned int b2, unsigned int b3);

/* Retrieves the tx power control parameters for AC system using Rev17 SROM format
 *
 * param[in] bandrange. The desired band range for getting PA parameters.
 *			WLM_CHAN_FREQ_RANGE_2G 0
 *			WLM_CHAN_FREQ_RANGE_5G_BAND0 1
 *			WLM_CHAN_FREQ_RANGE_5G_BAND1 2
 *			WLM_CHAN_FREQ_RANGE_5G_BAND2 3
 *			WLM_CHAN_FREQ_RANGE_5G_BAND3 4
 * param[in] chain
 * param[in] bandwidth.
 *			WLM_BANDWIDTH_20MHZ 0
 *			WLM_BANDWIDTH_40MHZ 1
 *			WLM_BANDWIDTH_80MHZ 2 (only applies to 5G band)
 * param[out] b0 PA parameter.
 * param[out] b1 PA parameter.
 * param[out] b2 PA parameter.
 * param[out] b3 PA parameter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmACPaParametersRev17Get(WLM_BANDRANGE bandrange, int chain, WLM_BANDWIDTH bandwidth,
	unsigned int *b0, unsigned int *b1, unsigned int *b2, unsigned int *b3);

/* Sets tx power control parameters for AC system using Rev17 SROM format.
 *
 * param[in] bandrange. The desired band range
 *			WLM_CHAN_FREQ_RANGE_2G 0
 *			WLM_CHAN_FREQ_RANGE_5G_BAND0 1
 *			WLM_CHAN_FREQ_RANGE_5G_BAND1 2
 *			WLM_CHAN_FREQ_RANGE_5G_BAND2 3
 *			WLM_CHAN_FREQ_RANGE_5G_BAND3 4
 * param[in] chain
 * param[in] bandwidth.
 *			WLM_BANDWIDTH_20MHZ 0
 *			WLM_BANDWIDTH_40MHZ 1
 *			WLM_BANDWIDTH_80MHZ 2 (only applies to 5G band)
 * param[in] b0 PA parameter.
 * param[in] b1 PA parameter.
 * param[in] b2 PA parameter.
 * param[in] b3 PA parameter.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmACPaParametersRev17Set(WLM_BANDRANGE bandrange, int chain, WLM_BANDWIDTH bandwidth,
	unsigned int b0, unsigned int b1, unsigned int b2, unsigned int b3);

/* Retrieves the current MAC address of the device.
 *
 * param[in] macAddr MAC address returned.
 * param[in] length Length of macAddr buffer.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmMacAddrGet(char *macAddr, int length);

/* Sets the MAC address of the device.
 *
 * param[in] macAddr The desired MAC address. Expected format is "XX:XX:XX:XX:XX:XX".
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmMacAddrSet(const char* macAddr);

/* Enables or disables output of a carrier tone.
 *
 * param[in] enable Set to true to enable carrier tone, false to disable.
 * param[in] channel Desired channel. Ignored if <i>enable</i> is false.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmEnableCarrierTone(int enable, int channel);

/* Enables or disables an EVM test.
 *
 * param[in] enable Set to true to enable EVM test, false to disable.
 * param[in] channel Desired channel. Ignored if <i>enable</i> is false.
 * param[in] rate Desired rate. Ignored if <i>enable</i> is false.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmEnableEVMTest(int enable, WLM_RATE rate, int channel);

/* RX/TX related functions */

/* Starts sending packets with supplied parameters.
 *
 * param[in] shortPreamble Set to true to use a short preamble, false for a long preamble.
 * param[in] interPacketDelay Delay between each packet.
 * param[in] numPackets The number of packets transmitted.
 * param[in] packetLength Length of packet transmitted.
 * param[in] destMac The MAC address of the destination.
 * param[in] withAck Ack response expected for transmitted packet.
 * param[in] syncMode 0 for async, 1 for sync, 2 for sync unblk.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxPacketStart(unsigned int interPacketDelay,
	unsigned int numPackets, unsigned int packetLength,
	const char* destMac, int withAck, int syncMode);

/* Stops sending packets.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxPacketStop(void);

/* Starts receiving packets with supplied parameters.
 *
 * param[in] srcMac The MAC address of the packet source..
 * param[in] withAck Ack response expected for received packet.
 * param[in] syncMode Enable synchronous mode to receive packets before returning or timeout.
 * param[in] numPackets Number of receive packets before returning (sync mode only).
 * param[in] timeout Maximum timeout in msec (sync mode only).
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRxPacketStart(const char* srcMac, int withAck,
	int syncMode, unsigned int numPackets, unsigned int timeout);

/* Stops receiving packets.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRxPacketStop(void);

/* Returns number of packets that were ACKed successfully.
 *
 * param[in] count Packet count returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxGetAckedPackets(unsigned int *count);

/* Returns number of ACK packets transmitted.
 *
 * param[in] txackfrm count returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmGetTxAckPackets(unsigned int *count);

/* Returns counters message
 *
 * param[out] counters message returned.
 * param[int] return buffer length.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmGetCounters(char *stats, int length);

/* Returns value of specific counters
 *
 * param[in] the arguments to the command. Eg. subcounters 11 <cntr1 ... cntrn>.
 * param[out] buffer pointer.
 * param[in] buffer length.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmGetSubCounters(char *cmd_line, char *stats, int length);

/* Returns number of packets received successfully.
 *
 * param[in] count Packet count returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRxGetReceivedPackets(unsigned int *count);

/* Returns Receive Signal Strength Indicator (RSSI).
 *
 * param[in] rssi RSSI returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRssiGet(int *rssi);

/* Sequence related functions */

/* Initiates a command batch sequence.
 * Any commands issued after calling this function will be queued
 * instead of being executed. Call sequenceStop() to run queued commands.
 *
 * param[in] clientBatching Set to true to batch on client, false to batch on server/DUT.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSequenceStart(int clientBatching);

/* Signals the end of a command batch sequence.
 * Requires a previous call to sequenceStart(). Any commands
 * issued after previously calling sequenceStart() will be run.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSequenceStop(void);

/* Sets a delay between commands, in ms.
 * Requires a previous call to sequenceStart(). Once sequenceStart() is called,
 * calling sequenceDelay() after a command will insert a delay after the
 * previous command is complete.
 *
 * param[in] msec Amount of time to wait after previous command has complete.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSequenceDelay(int msec);

/* If a command sequence fails at a particular command, the remaining
 * commands will be aborted. Calling this function, will return the index
 * of which command failed. NOTE: Index starts at 1.
 *
 * param[in] index Failed command index returned.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSequenceErrorIndex(int *index);

/*  Image access (SROM/OTP) functions: */

/* Programs the device with a supplied image. Used to program the SROM/OTP.
 *
 * param[in] byteStream A stream of bytes loaded from an image file.
 * param[in] length The number of bytes to write.
 * param[in] imageType Use this to force a specific image type (ex: SROM, OTP, etc).
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmDeviceImageWrite(const char* byteStream, int length, WLM_IMAGE_TYPE imageType);

/* Dumps the image from the device. Used to read the SROM/OTP.
 *
 * param[out] byteStream A user supplied buffer.
 * param[in] length The size of the buffer supplied by byteStream.
 * param[in] imageType Use this to force a specific image type (ex: SROM, OTP, etc).
 *
 * return - Size of image in bytes. -1 if size of image is larger than supplied buffer.
 */
WLM_FUNCTION
int wlmDeviceImageRead(char* byteStream, unsigned int length, WLM_IMAGE_TYPE imageType);

/* Write a hex byte stream to specified byte offset to
 * the CIS source (either SROM or OTP).
 * CRC will be written to the OTP if a valid configuration is
 * programmed in OTP_CRC_Configuration region
 * param[in] byte offset
 * param[in] hex byte stream
 * param[in] preview - option allows you to review the update without committing it
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmCisUpdate(int offset, char *byteStream, int preview);

/* Write raw data to on-chip OTP
 * CRC will be written to OTP if a valid configuration is
 * programmed in OTP_CRC_Configuration region
 * param[in] file to write
 * param[in] pciecis Write OTP for PCIe full-dongle
 * param[in] preview - option allows you to review the update without committing it
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmCisWrite(char *fileName, int pciecis, int preview);

/* Write raw data to on-chip OTP
 * param[in] bit offset where data to be written
 * param[in] number of bits to be written
 * param[in] data stream in hex. '0x' before data stream is optional
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOtpRaw(int offset, int num_bits, char *data);

/* Write raw data to on-chip OTP
 * param[in] file to write
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOtpW(char *fileName);

/* Get check if the currently programmed CRC is correct or not
 * param[in] 0 = do not update CRC, 1 = update CRC if current CRC is wrong
 * param[out] 0 = CRC fail, 1 = CRC pass
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOtpCrc(int update_crc, int *result);

/* Get the crc configuration at configuration space on OTP
 * param[out] Start address of CRC block. start_addr = end_addr-num_crc+1
 * param[out] End address of CRC block.
 * param[out] Number of CRC to store
 * param[out] CRC version. For future expension
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOtpCrcConfigGet(int *start_addr, int *end_addr, int *num_crc, int *crc_ver);

/* Set the crc configuration at configuration space in OTP.
 * Either hex value of config or individual parameters are allowed.
 * param[in] End address of OTP CRC region
 * param[in] Size of OTP CRC region. This should be a multiple of 4 and
 * should be between 4 and 28 (both included).
 * param[in] Version of OTP CRC configuration
 * param[in] Optional configuration to be set instead of individual args.
 * Value should be in hex, eg 0x7BBE. If unused, set to "NULL"
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOtpCrcConfigSet(int address, int size, int ver, char *config);

/* Dumps the CIS image from the device.
 * param[out] output user supplied buffer.
 * param[in] return buffer length
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmCisDump(char *output, int length);

/* Network related functions */

/* Set the current wireless security.
 *
 * param[in] authType Desired authentication type.
 * param[in] authMode Desired authentication mode.
 * param[in] encryption The desired encryption method.
 * param[in] key The encryption key string (null terminated).
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSecuritySet(WLM_AUTH_TYPE authType, WLM_AUTH_MODE authMode,
	WLM_ENCRYPTION encryption, const char *key);

/* Joins a  network.
 *
 * param[in] ssid The SSID of the network to join.
 * param[in] mode The mode used for the join.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmJoinNetwork(const char* ssid, WLM_JOIN_MODE mode);

/* Disassociates from a currently joined network.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmDisassociateNetwork(void);

/* Retrieves the current SSID of the AP.
 *
 * param[in] ssid SSID returned. SSID may be returned if not associated
 * (i.e. attempting to associate, roaming).
 * param[in] length Length of ssid buffer.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSsidGet(char *ssid, int length);

/* Retrieves the current BSSID of the AP.
 *
 * param[in] bssid BSSID returned. If the device is not associated, empty string returned.
 * param[in] length Length of bssid buffer.
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmBssidGet(char *ssid, int length);

/* Set/Get the MIMO Preamble mode
 * Valid options are:
 * WLC_N_PREAMBLE_MIXEDMODE 0
 *  WLC_N_PREAMBLE_GF 1
 */

WLM_FUNCTION
int wlmMimoPreambleSet(int type);

WLM_FUNCTION
int wlmMimoPreambleGet(int* type);

/* Set/Get the CGA coefficients in the 2.4G/5G band
 * requires an array of 24 uint8s to be set
 * and a specified length of 24 to get
 * All of these CGA value query functions only applies to 4329 solution
 */

WLM_FUNCTION
int wlmCga5gOffsetsSet(char* values, int len);

WLM_FUNCTION
int wlmCga5gOffsetsGet(char* buffer, int len);

WLM_FUNCTION
int wlmCga2gOffsetsSet(char* values, int len);

WLM_FUNCTION
int wlmCga2gOffsetsGet(char* buffer, int len);

/* Set Glacial Timer
 * param[in] timer duration in msec
 *
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmGlacialTimerSet(int val);

/* Set Fast Timer
 * param[in] timer duration in msec
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmFastTimerSet(int val);

/* Set Slow Timer
 * param[in] timer duration in msec
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSlowTimerSet(int val);

/* Enable/Disable Scansuppress
 * param[in] set to TRUE enables scansuppress, set to false disable scansuppress
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmScanSuppress(int val);

/* Set Country Code
 * param[in] country code abbrv. Default use WLM_COUNTRY_CODE_ABBRV_ALL
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmCountryCodeSet(const char *country_name);

/* Set fullcal
 * Trigger lpphy fullcal
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmFullCal(void);

/* Get Receiver IQ Estimation
 * param[out] estimated rxiq power in dBm at 0.25dBm resolution
 * param[in] sampel count, 0 to 15
 * param[in] antenna, 0 to 3
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmRxIQEstGet(float *val, int sampleCount, int ant);

/* Get Receiver IQ Estimation (extended method)
 * param[out] estimated rxiq power in dBm at 0.25dBm resolution
 * param[in] sampel count, 0 to 15
 * param[in] antenna, 0 to 3
 * param[in] elna on/off 0 = off; 1 = on
 * param[in] gain index, 0 to 75
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmRxIQEstExtGet(float *val, int sampleCount, int ant, int elna, int gainindex);

/* Get Receiver IQ Estimation (extended method)
 * param[out] estimated rxiq power in dBm at 0.25dBm resolution
 * param[in] sampel count, 0 to 15
 * param[in] antenna, 0 to 3
 * param[in] extra gain 0, 3, ... 21, 24
 * param[in] gain index, 0 = default gain; 1 = fixed high gain (elna on); 4 = fixed low gain
 * param[in] gain correction toggle, 0 = off; or 1, 2, 3, 4
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmRxIQEstACGet(float *val, int sampleCount, int ant, int extragain, int gainindex,
    int gain_correct);

/* Get Receiver IQ Estimation for requested channels
 * param[out] pointer to a static wlmRxIQEstSweepReult_t structure of estimated rxiq power in dBm
	at 0.25dBm resolution for each requested channel
 * param[in] 0 terminated list of the required channels. If NULL - Estimation will be done for all
	channels
 * param[in] sample count, 0 to 15
 * param[in] antenna, 0 to 3
 * param[in] extra gain 0, 3, ... 21, 24
 * param[in] gain index, 0 = default gain; 1 = fixed high gain (elna on); 4 = fixed low gain
 * param[in] gain correction toggle, 0 = off; or 1, 2, 3, 4
 * param[in] elna
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmRxIQEstACSweepGet(wlmRxIQEstSweepReult_t **result, int *channels, int sampleCount,
	int antenna, int extraGain, int gainIndex, int gainCorrect, int niter, int delay);
WLM_FUNCTION
int wlmRxIQEstSweepGet(wlmRxIQEstSweepReult_t **result, int *channels, int sampleCount,
	int antenna);
WLM_FUNCTION
int wlmRxIQEstExtSweepGet(wlmRxIQEstSweepReult_t **result, int *channels, int sampleCount,
	int antenna, int elna, int gainIndex);

/* Get PHY txpwrindex
 * param[out] txpwrindex
 *            The index for each core [0, 3] is mapped to each of the 4 bytes
 * param[in] chip id: 4325, 4329, 43291, 4330, 4336 and 43236
 */
WLM_FUNCTION
int wlmPHYTxPowerIndexGet(unsigned int *val, const char *chipid);

/* Set PHY txpwrindex
 * param[in] txpwrindex
 *           the index for each core [0, 3] is mapped to each core of the 4 bytes
 * param[in] chip id: 4325, 4329, 43291, 4330, 4336 and 43236
 */
WLM_FUNCTION
int wlmPHYTxPowerIndexSet(unsigned int val, const char *chipid);

/* Enable/Disable RIFS
 * param[in] Set RIFS mode. 1 = enable ; 0 = disable
 */
WLM_FUNCTION
int wlmRIFSEnable(int enable);

/* Get/Set IOCTL
 * param[in] cmd IOCTL command.
 * param[in] buf Get data returned or set data input.
 * param[in] len Length of buf.
 * return - True for success, false for failure.
 */
WLM_FUNCTION int wlmIoctlGet(int cmd, void *buf, int len);
WLM_FUNCTION int wlmIoctlSet(int cmd, void *buf, int len);

/* Get/Set IOVAR
 * param[in] iovar IOVAR command.
 * param[in] buf Get data returned or set data input.
 * param[in] len Length of buf.
 * return - True for success, false for failure.
 */
WLM_FUNCTION int wlmIovarGet(const char *iovar, void *buf, int len);
WLM_FUNCTION int wlmIovarSet(const char *iovar, void *buf, int len);

/* Get/Set IOVAR integer.
 * param[in] iovar IOVAR integer command.
 * param[out/in] val Get integer returned or set integer input.
 * return - True for success, false for failure.
 */
WLM_FUNCTION int wlmIovarIntegerGet(const char *iovar, int *val);
WLM_FUNCTION int wlmIovarIntegerSet(const char *iovar, int val);

/* Get/Set IOVAR using internal buffer.
 * param[in] iovar IOVAR command.
 * param[in] param Input parameters.
 * param[in] param_len Length of parameters.
 * param[out] bufptr Buffer returning get data.
 * return - True for success, false for failure.
 */
WLM_FUNCTION int wlmIovarBufferGet(const char *iovar, void *param, int param_len, void **bufptr);
WLM_FUNCTION int wlmIovarBufferSet(const char *iovar, void *param, int param_len);

#ifdef SERDOWNLOAD
WLM_FUNCTION int wlmDhdDownload(const char *firmware, const char* vars);
WLM_FUNCTION int wlmDhdInit(const char *chip);
#endif // endif

/* Enables radio
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRadioOn(void);

/* Disable radio .
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRadioOff(void);

/* Set Power Saving Mode
 * param[in] 0 = CAM, 1 =Power Save, 2 = Fast PS Mode
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPMmode(int val);

/* Enable roaming
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRoamingOff(void);

/* Disable roaming
 *
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRoamingOn(void);

/* Get roaming trigger level
 * param[out] roaming trigger level in dBm returned
 * param[in]  frequency band
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRoamTriggerLevelGet(int *val, WLM_BAND band);

/* Set roaming trigger level
 * param[in] roaming trigger level in dBm
 * param[in]  frequence band
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRoamTriggerLevelSet(int val, WLM_BAND band);

/* Set framburst mode on
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmFrameBurstOn(void);

/* Set framburst mode off
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmFrameBurstOff(void);

/* Set beacon interval
 * param[in] beacon interval in ms
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmBeaconIntervalSet(int val);

/* Set AMPDU mode on/off
 * param[in] on = 1, off = 0
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmAMPDUModeSet(int val);

/* Send addba to specified ea-tid
 * param[in] tid
 * param[in] macAddr. Expected format is "XX:XX:XX:XX:XX:XX"
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmAMPDUSendAddBaSet(int tid, const char* macAddr);

/* Set MIMO bandwidth capability
 * param[in] mimo bandwidth capability. 0 = 20Mhz, 1 =40Mhz
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmMIMOBandwidthCapabilitySet(int val);

/* Set interference on/off
 * param[in] on = 1, 0ff = 0
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmInterferenceSet(int val);

/* Set interferenceoverride on/off
 * param[in] on = 1, 0ff = 0
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmInterferenceOverrideSet(int val);

/* Set MIMO transmit banddwith
 * param[in] auto = -1, 2 = 20Mhz, 3 = 20Mhz upper , 4 =40 Mhz, 5 =40dup (mcs32 only)
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTransmitBandwidthSet(int val);

/* Set MIMO short guard intervaltransmit banddwith for tx stream
 * param[in] auto = -1, 1 = on, 0 = off
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmShortGuardIntervalSet(int val);

/* Set MIMO short guard intervaltransmit banddwith for rx stream
 * param[in] auto = -1, 1 = on, 0 = off
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmShortGuardIntervalRxSet(int val);

/* Set MIMO OBSS coex set
 * param[in] auto = -1, 1 = on, 0 = off
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmObssCoexSet(int val);

/* Set PHY Periodical call
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYPeriodicalCalSet(void);

/* Set PHY Force call
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYForceCalSet(void);

/* Disable scrambler update
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYScramblerUpdateDisable(void);

/* Enable scrambler update
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYScramblerUpdateEnable(void);

/* Turn PHY watchdog on/off
 * param[in] on = 1, off = 0
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYWatchdogSet(int val);

/* Disable temperature sensore
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTemperatureSensorDisable(void);

/* Enable temperature sensore
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTemperatureSensorEnable(void);

/* Set transmit core
 * param[in] active core (bitmask) to be used when transmitting frames
 *           0x1 = core 1
 *           0x2 = core 2
 *           0x4 = core 3
 *           0x8 = core 4
 *           0x3 = core 1 and core 2
 *           0x7 = core 1, core 2 and core 3 ...
 * param[in] number of spacial streams
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTransmitCoreSet(int core, int streams);

/* Get temperature sensor read
 * param[out] chip core temperature in C
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTempSenseGet(int *val);

/* Get chip OTP Fab ID
 * param[out] chip fab id
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOtpFabidGet(int *val);

/* Set mimo channnel specifications
 * param[in] channel
 * param[in] bandwidth 20 = 20Mhz, 40 = 40Mhz, 80Mhz and 160Mhz
 * param[in] sideband 1 = upper, -1 = lower, 0 = none
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmChannelSpecSet(int channel, int bandwidth, int sideband);

/* Set channnel specifications
 * param[in] ch1 = Primary 80MHz channel
 * param[in] ch2 = Sedonary 80MHz channel
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlm80Plus80ChannelSpecSet(int ch1, int ch);

/* Set rts threshold
 * param[in] rts threshold value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRtsThresholdOverride(int val);

/* Turn STBC Tx mode on/off
 * param[in] on = 1, off = 0
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSTBCTxSet(int val);

/* Turn STBC Rx mode on/off
 * param[in] on = 1, off = 0
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSTBCRxSet(int val);

/* MIMO single stream tx chain selection
 * param[in] chain 1 = 1, chain 2 = 2
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxChainSet(int val);

/* MIMO single stream rx chain selection
 * param[in] chain 1 = 1, chain 2 = 2
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRxChainSet(int val);

WLM_FUNCTION
int wlmScanNetworks(char* results_buf, int len);

WLM_FUNCTION
int wlmCommandPassThrough(char *cmd_line, char *result_buf, int *len);

/* Toggle GPIO control
 * param[in] mask to gpio control bits, eg 0x0040 for gpio_6
 * param[in] bit control value. 1 = enable/high, 0 = disable/low
 */
WLM_FUNCTION
int wlmGpioOut(unsigned int mask, unsigned int value);

/* Get rssi gain delta value for 2g band.
 * param[in] iovar name for 2g band rssi gain delta values, they are,
 *           "phy_rssi_gain_delta_2g"
 * param[out] delta values buffer of 4 elements.
 * param[in] core ID
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainDelta2gGet(char *varname, int *deltaValues, int core);

/* Set rssi gain delta value for 2g band.
 * param[in] iovar name for 2g band rssi gain delta values, they are,
 *           "phy_rssi_gain_delta_2g"
 * param[in] delta values buffer of 4 elements.
 * param[in] core ID
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainDelta2gSet(char *varname, int *deltaValues, int core);

/* Get rssi gain delta value for 5g band.
 * param[in] iovar name for 5g band rssi gain delta values, they are,
 *           "phy_rssi_gain_delta_5gl"
 *           "phy_rssi_gain_delta_5gml"
 *           "phy_rssi_gain_delta_5gmu"
 *           "phy_rssi_gain_delta_5gh"
 * param[out] delta values buffer of 6 elements.
 * param[in] core ID
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainDelta5gGet(char *varname, int *deltaValues, int core);

/* Set rssi gain delta value for 5g band.
 * param[in] iovar name for 5g band rssi gain delta values, they are,
 *           "phy_rssi_gain_delta_5gl"
 *           "phy_rssi_gain_delta_5gml"
 *           "phy_rssi_gain_delta_5gmu"
 *           "phy_rssi_gain_delta_5gh"
 * param[in] delta values buffer of 6 elements.
 * param[in] core ID
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainDelta5gSet(char *varname, int *deltaValues, int core);

/* Set VHT Features
 * param[in] VHT featurs rates bitmap
 *           Bit 0:5G MCS 0-9 BW 160MHz
 *           Bit 1:5G MCS 0-9 support BW 80MHz
 *           Bit 2:5G MCS 0-9 support BW 20MHz
 *           Bit 3:2.4G MCS 0-9 support BW 20MHz
 *           Bits 4:7Reserved for future use
 *           Bit 8:VHT 5G support
 *           Bit 9:VHT 2.4G support
 *           Bit 10:11Allowed MCS map
 *           0 is MCS 0-7
 *           1 is MCS 0-8
 *           2 is MCS 0-9
 *           3 is Disabled
 */
WLM_FUNCTION
int wlmVHTFeaturesSet(int val);

/* Enable Tx Beamforming
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int  wlmTxBFEnable(void);

/* Disable Tx Beamforming
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxBFDisable(void);

/* Set 802.11h Spectrum Management mode
 * param[in] spect mode
 *           0 - Off\n"
 *           1 - Loose interpretation of 11h spec - may join non-11h APs
 *           2 - Strict interpretation of 11h spec - may not join non-11h APs
 *           3 - Disable 11h and enable 11d
 *           4 - Loose interpretation of 11h+d spec - may join non-11h APs
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSpectModeSet(int val);

/* Enable IBSS Gmode
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmIBSSGmodeEnable(void);

/* Disable IBSS Gmode
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmIBSSGmodeDisable(void);

/* Set per band bandwidth
 * param[in] band
 * param[in] bandwdith
 *          0x1 = 20Mhz
 *          0x3 = 20/40Mhz
 *          0x7 = 20/40/80Mhz
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmBandWidthCapabilitySet(WLM_BAND band, int val);

/* Get per band bandwidth
 * param[in] band
 * param[out] bandwdith
 *            0x1 = 20Mhz
 *            0x3 = 20/40Mhz
 *            0x7 = 20/40/80Mhz
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmBandWidthCapabilityGet(WLM_BAND band, int *val);

/* Enable/Disable Maximum Pkteng size limit
 * param[in] 0 = disable; 1 = enable
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmEnableLongPacket(int enable);

/* Set spatial policy
 * param[in] 2g band mode setting. -1 = auto; 0 = turn off; 1 = turn on
 * param[in] 5g lower band mode setting. -1 = auto; 0 = turn off; 1 = turn on
 * param[in] 5g mid band mode setting. -1 = auto; 0 = turn off; 1 = turn on
 * param[in] 5g high band mode setting. -1 = auto; 0 = turn off; 1 = turn on
 * param[in] 5g upper band mode setting. -1 = auto; 0 = turn off; 1 = turn on
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSpatialPolicySet(int mode_2g, int mode_5g_low, int mode_5g_mid,
	int mode_5g_high, int mode_5g_upper);

/* Populate the receprocity compenstation table based on SROM cal content
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYRPTableSet(void);

/* force the beamformer into implicit TXBF mode and ready to construct steering matrix
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYForceImplicitBeamforming(void);

/* Force the beamfomer to apply steering matrix when txbf in on
 * param[in] 0 = on; 1 = off
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmPHYForceSteer(int on);

/* Get rssi gain error value for 2g band.
 * param[in] iovar name for 2g band rssi gain error values, they are,
 *           "phy_rxgainerr_2g"
 * param[out] rx gain error values buffer.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainError2gGet(char *varname, int *errorValues);

/* Set rssi gain error value for 2g band.
 * param[in] iovar name for 2g band rssi gain error values, they are,
 *           "phy_rxgainerr_2g"
 * param[in] rx gain error values buffer.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainError2gSet(char *varname, int *errorValues);

/* Get rssi gain error value for 5g band.
 * param[in] iovar name for 5g band rssi gain error values, they are,
 *           "phy_rxgainerr_5gl"
 *           "phy_rxgainerr_5gm"
 *           "phy_rxgainerr_5gh"
 *           "phy_rxgainerr_5gu"
 * param[out] rx gain error values buffer.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainError5gGet(char *varname, int *errorValues);

/* Set rssi gain error value for 5g band.
 * param[in] iovar name for 5g band rssi gain error values, they are,
 *           "phy_rxgainerr_5gl"
 *           "phy_rxgainerr_5gm"
 *           "phy_rxgainerr_5gh"
 *           "phy_rxgainerr_5gu"
 * param[in] rx gain error values buffer.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainError5gSet(char *varname, int *errorValues);

/* Get raw tempsense value for AC chips
 * param[out] raw tempsense value.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRawTempsenseGet(int *val);

/* Set raw tempsense value for AC chps
 * param[in] raw tempsense value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRawTempsenseSet(int val);

/* Set temperary RPCAL parameters aftert driver attached, before driver up
 * param[in] RPCAL parameter for 2g band
 * param[in] RPCAL parameter for 5g band 0
 * param[in] RPCAL parameter for 5g band 1
 * param[in] RPCAL parameter for 5g band 2
 * param[in] RPCAL parameter for 5g band 3
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRPCalValsSet(int rpcal2g, int rpcal5gb0, int rpcal5gb1, int rpcal5gb2,
	int rpcal5gb3, int rpcal2gcore3, int rpcal5gb0core3, int rpcal5gb1core3,
	int rpcal5gb2core3, int rpcal5gb3core3);

/* Get temperary RPCAL parameters* param[out] RPCAL parameter for 2g band
 * param[out] RPCAL parameter for 5g band 0
 * param[out] RPCAL parameter for 5g band 1
 * param[out] RPCAL parameter for 5g band 2
 * param[out] RPCAL parameter for 5g band 3
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRPCalValsGet(int *rpcal2g, int *rpcal5gb0, int *rpcal5gb1, int *rpcal5gb2,
	int *rpcal5gb3, int *rpcal2gcore3, int *rpcal5gb0core3, int *rpcal5gb1core3,
	int *rpcal5gb2core3, int *rpcal5gb3core3);

/* Get phy register dump based on the list name
 * param[in] the register list name
 * param[out] dumped data buffer
 * param[out] number of data returned
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRegDump(char *reg_name, int *dump_vals, int *len);

/* Get rssi calibration frequency group for each channel at 2g band
 * param[out] 2g band rssi calibration frequency group values, one for each channel
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiCalibrationFrequencyGroup2gGet(int *values);

/* Set rssi calibration frequency group for each channel at 2g band
 * param[in] 2g band rssi calibration frequency group values, one for each channel
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiCalibrationFrequencyGroup2gSet(int *values);

/* Get rssi gain delta sub for 2g band
 * param[in] iovar name for 2g band rssi gain delta
 *           "phy_rssi_gain_delta_2gb0"
 *           "phy_rssi_gain_delta_2gb1"
 *           "phy_rssi_gain_delta_2gb2"
 *           "phy_rssi_gain_delta_2gb3"
 *           "phy_rssi_gain_delta_2gb4"
 * param[in] 8 rssi gain delta values
 * param[in] phy core id (0, 1, 2)
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainDelta2gSubGet(char *varname, int *values, int coreid);

/* Set rssi gain delta sub for 2g band
 * param[in] iovar name for 2g band rssi gain delta
 *           "phy_rssi_gain_delta_2gb0"
 *           "phy_rssi_gain_delta_2gb1"
 *           "phy_rssi_gain_delta_2gb2"
 *           "phy_rssi_gain_delta_2gb3"
 *           "phy_rssi_gain_delta_2gb4"
 * param[in] 8 rssi gain delta values
 * param[in] phy core id (0, 1, 2)
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRssiGainDelta2gSubSet(char *varname, int *values, int coreid);

/* Get tx calibration gain sweep test tssi and target power results
 * param[in] number of gain sweep data entries requested
 * param[out] return tssi value array
 * param[out] return taraget tx power value array
 * param[in] core id number
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCalGainSweepMeasGet(int count, int *tssi, int *txpower, int core);

/* Set tx calibration gain sweep measured tx powers
 * param[in] measured tx power array
 * param[in] number of gain sweep data entries
 * param[in] core id number
 * param[out] return taraget tx power value array
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCalGainSweepMeasSet(int *measPower, int count, int core);

/* To trigger noise cal.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPHYForceCalNoiseSet(void);

/* Run tx calibration gain sweep test
 * param[in] destination MAC address for tx packets
 * param[in] delay between tx packets
 * param[in] packet length of tx packets
 * param[in] number of tx frames
 * param[in] starting gain index [0, 127]
 * param[in] stop gain index [0, 127]
 * param[in] gain index sweep step
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCalGainSweep(char *desMac, int delay, int length, int nFrames,
	int start, int stop, int step);

/* Multi core crsmin override
 * param[in] values[0] core0 threshold, -1 = auto; 0 = default; (0, 127]
 *           values[1] core1 offset from range [-128, 127]
 *           values[2] core2 offset from range [-128, 127]
 * param[in] number of value entries
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyForceCRSMinSet(int *values, int count);

/* Get current adjusted tssi values
 * param[in] core id
 * param[out] return adjusted tssi value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmAdjustedTssiGet(int core, int *adjTssi);

/* Get current idle tssi value
 * param[in] core id
 * param[out] return tssi value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTestIdleTssiGet(int core, int *tssi);
/* Get current tssi value
 * param[in] core id
 * param[out] return tssi value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTestTssiGet(int core, int *tssi);

/* Get current Txcal adjusted tssi values
 * param[in] core id
 * param[in]  desire channel number
 * param[out] return start power level
 * param[out] return number of entries in the power tssi table
 * param[out] return tssi value array
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCalPowerTssiTableGet(int core, int *channel, int *startPower, int *num, int *tssiValues);

/* Set tssi values generated by txcal_gainsweep
 * param[in] core id
 * param[in] current channel number
 * param[in] start power level
 * param[in] number of entries in the power tssi table
 * param[in] input tssi value array
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCalPowerTssiTableSet(int core, int channel, int startPower, int num, int *tssiValues);

/* Generate interpolated power tssi table from wl txcal_gainsweep and wl txcal_gainsweep_meas
 * param[in] core id
 * param[in] current channel number
 * param[in] return start power level
 * param[in] return number of entries in the power tssi table
 * param[out] return tssi value array
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCalPowerTssiTableGenerate(int core, int channel, int startpower, int num, int *tssiValues);

/* Query txcal estimated power lookup table
 * param[in] core id
 * param[out] return estimated power array
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCalEstimatePowerLookupTableRead(int core, int *estPrwLut);

/* Apply txcal power tssi table
 * param[in] 1 fill up estPwrLut using consolidated pwr Tssi table by Txcal
 *           0 fill up estPwrLut using consolidated PA Params
 * param[out] return estimated power array
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int  wlmTxCalApplyPowerTssiTable(int mode);

/* Query OLPC Anchor point gain index
 * param[in] core id
 * param[in] channel number
 * param[out] anchor point gain index
 * param[out] anchor point tempsense
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int  wlmOlpcAnchorIndexGet(int core, int channel, int *pwr_start_index, int *tempsense);

/* Set OLPC Anchor point gain index
 * param[in] core id
 * param[in] channel number
 * param[in] anchor point gain index
 * param[in] anchor point tempsense
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int  wlmOlpcAnchorIndexSet(int core, int channel, int pwr_start_index, int tempsense);

/* Query anchor power level for 2G band in qdBm format
 * param[out] anchor power level
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcAnchorPower2GGet(int *power);

/* Set anchor power level for 2G band in qdBm format
 * param[in] anchor power level
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcAnchorPower2GSet(int power);

/* Query anchor power level for 5G band in qdBm format
 * param[out] anchor power level
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcAnchorPower5GGet(int *power);

/* Set anchor power level for 5G band in qdBm format
 * param[in] anchor power level
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcAnchorPower5GSet(int power);

/* Disable OLPC
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcDisable(void);

/* Disable OLPC
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcEanble(void);

/* Query tssi visibility threshold in qdBm format
 * param[out] olpc threshold
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcThresholdGet(int *val);

/* Set tssi visibility threshold in qdBm format
 * param[in] olpc threshold
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcThresholdSet(int val);

/* Query olpc_idx_valid status
 * param[out] olpc_idx_valid status
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcIndexValidGet(int *val);

/* Set olpc_idx_valid status
 * param[in] olpc_idx_valid status
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOlpcIndexValidSet(int val);

/* Query Maximum Tx Power Cap
 * param[in] bits 7:0 txpwrcap for current antenna on core 0
 *           bits 15:8 txpwrcap for current antenna on core 1
 *           bits 23:16 txpwrcap for current antenna on core 2
 *           bits 31:24 txpwrcap for current antenna on core 3
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTxPowerCapGet(int *txpwrcap);

/* Query phy cell status
 * param[out] cell status
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyCellStatusGet(int *val);

/* Query Maximum Tx Power Cap Table
 * param[in] core id number X [0, 7]
 * param[out] number of antennas for core id X
 * param[out] Cell on power caps for all the antennas at current channel at core id X
 * param[out] Cell off power caps for current channel at core id X
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTxPowerCapTableGet(int core, int *numAnt, int *cellOnCaps, int *cellOffCaps);

/* Set Maximum Tx Power Cap Table
 * param[in] number of cores for this chip
 * param[in] array for antenna numbers of each core, max 2 antennas per core
 *           eg., 2 1 (2 antennas for core 0 and 1 antennas for core 1)
 * param[in] array for cell on power caps for each antenna at each core for current channel
 * param[in] array for cell off power caps for each antenna at each core for current channel
 * param[in] number of elements for cell on or cell off caps arrays
 *           this number should match the total number of antennas
 *           based on the above example, this number would be 3
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTxPowerCapTableSet(int numOfCores,
	int *numAntPerCore, int *cellOnCaps, int *cellOffCaps, int numOfCaps);

/* Set phy cell status
 * param[in] cell status
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyCellStatusSet(int val);

/* Get phy Tx power control value
 * param[out] power control value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTxPowerControlGet(int* val);

/* Set phy Tx power control value
 * param[in] power control value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTxPowerControlSet(int val);

/* clm file download
 * param[in] clm blob file name
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmClmLoad(char *fileName);

/* Query clm file download status
 * param[out] clm file download status
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmClmLoadStatusGet(int *status);

/* query clm versin strings
 * param[out] clm version string
 * param[in] expect lengh of clm version string
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmClmVersionGet(char *clmverstr, int len);

/* Max tx cap file download
 * param[in] txcap file name
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCapLoad(char *fileName);

/* Query txcap version
 * param[out] txcap version string
 * param[in] expected lengh of txcap version string
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTxCapVersionGet(char *txcapverstr, int len);

/* Cal file download
 * param[in] Cal MSF file name
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmCalLoad(char *fileName);

/* Query Cal file download status
 * param[out] Cal file download status
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmCalLoadStatusGet(int *status);

/* Cal information dump (to a boinary file)
 * param[in] Cal file name with raw data in customer specific storage foramt.
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmCalDump(char *fileName);

/* Query software diversity stats
 * param[out] software diversity stats
 * param[in] expected length of swdiv_stats
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmSWDiversityStatsGet(char *stats, int length);

/* Trigger Phy VCO Calibration
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyVcoCalSet(void);

/* Query PHY register value
 * param[in] phy register offset
 * param[in] frequency band
 * param[out] returned phy register value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRegGet(int offset, WLM_BAND band, int *val);

/* Set PHY register value
 * param[in] phy register offset
 * param[in] frequency band
 * param[in] phy register value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyRegSet(int offset, WLM_BAND band, int val);

/* Query radio register value
 * param[in] radio register offset
 * param[in] frequency band
 * param[in] phy core number [0,1,2,3] and pll id [4]
 * param[out] returned radio register value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRadioRegGet(int offset, WLM_BAND band, int id, int *val);

/* Set radio register value
 * param[in] radio register offset
 * param[in] frequency band
 * param[in] phy core number [0,1,2,3] and pll id [4]
 * param[in] radio register value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmRadioRegSet(int reg, WLM_BAND band, int id, int val);

/* Set LDO VCO value and run VCO calibration
 * param[in] LDO VCL value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmLDOSetVcoCal(int val, WLM_BAND band);

/* Download ota test flow file
 * param[in] Ota test flow file name
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOTALoadTestFlowFile(char *flowfile);

/* Trigger OTA Test
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmOTATriggerTest();

/* Enable/Disable Tx power Ctrl Init base index override
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmTwPwrOvrInitBaseIdx(int val);

/* 2G Tx power Ctrl Init base index override value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmInitBaseIdx2g(int val);

/* 5G Tx power Ctrl Init base index override value
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmInitBaseIdx5g(int val);

/* Set Tx Power Ctrl ADC Av value for a given core and sub-band
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTpcAvSet(int core, int subband, int val);

/* Get Tx Power Ctrl ADC Av value for a given core and sub-band
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTpcAvGet(int core, int subband, int *val);

/* Set Tx Power Ctrl ADC Vmid value for a given core and sub-band
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTpcVmidSet(int core, int subband, int val);

/* Get Tx Power Ctrl ADC Vmid value for a given core and sub-band
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmPhyTpcVmidGet(int core, int subband, int *val);

/* Set value of config Iovars such as rsdb_mode
 * param[in] configuration value/string
 * for example: rsdb_mode [ auto | mimo | rsdb | 80p80 ]
 * or rsdb_mode [ -1 | 0 | 1 | 2 ]
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmConfigSet(const char *iovar, char *config);

/* Get value of config Iovars such as rsdb_mode
 * param[out] current status(string) of the config iovar
 * param[out] current status(integer value) of the config iovar
 * for example: auto -1 mimo 0 rsdb 1 80p80 2
 * return - True for success, false for failure.
 */
WLM_FUNCTION
int wlmConfigGet(const char *iovar, char *status_str, unsigned int *val);

/* Query desense mode for rxgain
 * param[out] band
 * param[out] number of cores on which desense is applied
 * param[out] array of desense values corresponding to number of cores
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmBtcoexDesenseRxgainGet(WLM_BAND *band, int *num_cores, int *desense_array);

/* Set desense mode for rxgain
 * param[in] band
 * param[in] number of cores
 * param[in] array of desense values corresponding to number of cores
 * return - True for success, false for failure.
*/
WLM_FUNCTION
int wlmBtcoexDesenseRxgainSet(WLM_BAND band, int num_cores, int *desense_array);

WLM_FUNCTION
int wlmOtatestStatus(int cnt);

WLM_FUNCTION
int wlOtatestRssi();

const char* wlmGetLastError(void);

#ifdef __cplusplus
}
#endif // endif

#endif /* _wlm_h */
