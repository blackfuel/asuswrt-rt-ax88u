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
 * $Id: airiq_app.c 759093 2018-04-23 22:50:35Z $
 */

/* bsa_service.c: Starts manager service and SIRP server */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include "bsa_api.h"
#include <pthread.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <wlioctl_airiq.h>
#include <security_ipc.h>
#include "bcmevent.h"
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <miniopt.h>
#include <errno.h>
#include <bcmendian.h>

#if defined(CLASSIFIER_DEBUG)
#include <stdbool.h>
#define FALSE false
#define TRUE  true
#endif // endif

#if defined(AIRIQ_APP_LOCAL)
typedef struct {
	BSA_UINT32 MsgType;
	void *Data;
	BSA_UINT32 DataSize;
} BSA_MS_MSG;
#define MS_MSG_TERMINATE 4
extern BSADLLSPEC void *MsMain(void *dummy);
extern BSADLLSPEC BSA_STATUS MsQueueMsg(BSA_MS_MSG *Msg);
void TerminateMgrSvc(void);
#endif /* AIRIQ_APP_LOCAL */

#if   defined(AIRIQ_APP_SIRP)
#elif defined(AIRIQ_APP_LOCAL)
#else
#error You must define a configuration: AIRIQ_APP_WITH_SIRP or AIRIQ_APP_LOCAL
#endif // endif

BSA_UINT32 gAggrFFTCount;
BSA_UINT32 gChanUtilCount;
BSA_UINT32 gInterferenceCount;

BSA_UINT32 gFFTCpxCount  = 0;
BSA_UINT32 gFFTCpxBytes  = 0;
BSA_UINT32 gDbgLogCount  = 0;
BSA_UINT32 gDbgLogBytes  = 0;
BSA_UINT32 gIntCount     = 0;
BSA_UINT32 gIntLogBytes  = 0;
BSA_UINT32 gTerminate    = 0;
FILE      *gDbgLogFile   = NULL;
FILE      *gFFTCpxFile   = NULL;
FILE      *gIntFile      = NULL;

BSA_UINT32 gLogEventMask = 0;
BSA_INT32  gLogSeconds   = -1;

#define PHYMODE_3x3_1x1            31
#define CHANNELS_24GHZ_40MHZ_COUNT 4

#define MAX_IF                     3
char cfg_ifname[MAX_IF][32];
int  cfg_ifcount      = 0;
int  cfg_2gidx        = 0;
int  cfg_5gidx        = 0;
bool cfg_verbose      = 0;
int  cfg_print_events = 0;
bool checkbw          = FALSE;

#define EVENT_MASK_ALL \
        (BSA_EVENT_ID_AGGR_FFT_DATA | \
         BSA_EVENT_ID_INTERFERENCE  | \
         BSA_EVENT_ID_CHAN_UTIL_WITH_INT | \
         BSA_EVENT_ID_SCAN_COMPLETE | \
         BSA_EVENT_ID_CONN_STATUS)

/* #define PRINTF_EVENT_MASK EVENT_MASK_ALL */
#define PRINTF_EVENT_MASK BSA_EVENT_ID_SCAN_COMPLETE

#define PRINTF_EVENT(LVL, ...) \
        do { \
		if ((LVL)&(cfg_print_events)) { \
			printf(__VA_ARGS__); \
		} \
	} while (0)

#define PRINTF_VERBOSE(...)

/*  */
/* Type definitions */
/*  */
struct SCAN_MODE {
	BSA_SCAN_MODE code;
	char         *codeName;
	char         *name;
};
#define CNAME(X) .code = X, .codeName = # X

typedef struct {
	BSA_UINT16 longdwell;
	BSA_UINT16 longfftcnt;
	BSA_UINT16 shortdwell;
	BSA_UINT16 shortfftcnt;
	int        band;
	int        aband;
	int        bband;
	int        scan_channel;
	int        core;
	int        phy_mode;
	int        bw;
	BSA_UINT16 capture_interval;
	int        scan80only;
} airiq_scan_specs_t;

typedef struct {
	char           ifname[32];
	struct ifreq   if_req;
	int            offloads;
	wlc_rev_info_t revinfo;
	int            dwell_time;
	int            wlband;
	int            aband;
	int            bband;
	int            capture_time;
	int            scan_channel;
	int            homescan;
	int            core;
	int            phy_mode;
	int            capture_interval;
} airiq_scan_req_t;

/*  */
/* Global variables */
/*  */
struct SCAN_MODE gScanModeTable[] =
{
	{ CNAME(BSA_SCAN_MODE_DWELL),      .name       = "Dwell"          },
	{ CNAME(BSA_SCAN_MODE_24GHZ),      .name       = "2.4 GHz"        },
	{ CNAME(BSA_SCAN_MODE_24GHZ_5GHZ), .name       = "2.4 + 5 GHz"    },
	{ CNAME(BSA_SCAN_MODE_5GHZ_LOW),   .name       = "5 GHz low"      },
	{ CNAME(BSA_SCAN_MODE_5GHZ_MID),   .name       = "5 GHz mid"      },
	{ CNAME(BSA_SCAN_MODE_5GHZ_HIGH),  .name       = "5 GHz high"     },
	{ CNAME(BSA_SCAN_MODE_5GHZ),       .name       = "5 low+mid+high" },
	{ CNAME(BSA_SCAN_MODE_USER1),      .name       = "User-defined 1" },
	{ CNAME(BSA_SCAN_MODE_USER2),      .name       = "User-defined 2" },
	{ CNAME(BSA_SCAN_MODE_USER3),      .name       = "User-defined 3" },
	{ CNAME(BSA_SCAN_MODE_USER4),      .name       = "User-defined 4" },
};

const int       gScanModeCount = sizeof(gScanModeTable) / sizeof(struct SCAN_MODE);

pthread_cond_t  gScanCond;
pthread_mutex_t gScanMutex;

/*LTE_U variables*/
static int   term = 0;
struct ifreq if_req;
uint16       cfg_reg[8];
bool         lte_cfg_set = FALSE;
char         cfg_ifname_lteu[32];
#define WL_EVENTS_BUFFER_SIZE 2048
int          gMainFlags = 0x0;

/*  */
/* Function Prototypes */
/*  */
struct SCAN_MODE *GetScanModeByCode(BSA_SCAN_MODE m);

int LTEU_Main(int argc, char*argv[]);
int AbortLTEUScan();
void LTEU_Usage(void);

void Usage(void)
{
	printf("\n");
	printf("Usage:\n\n");
	printf("   airiq_app [-i [INTERFACE_NAME]] [-p [IPADDR]] [-h] [-help]\n\n");
	printf("For single radio interface \n");
	printf("-v                  Enable verbose output\n");
	printf("-print_events       Enable printing of event data\n");
	printf("-i [INTERFACE_NAME] Specify interface name. The default\n");
	printf("                    interface used is the first 11AC interface \n");
	printf("-d [DWELL TIME]     Set the dwell time to be used for user-controlled scanning\n");
	printf("                    Units: Milliseconds. Default value: 1000 ms.\n");
	printf("                    Acceptable values: 400 - 1000 ms\n");
	printf("-c [CAPTURE_TIME]   Set the capture time to be used for user-controlled scanning\n");
	printf("                    Units: Milliseconds. Default value: 0 ms.\n");
	printf("                    Acceptable values: 0 - 50 ms\n");
	printf("-int [CAPTURE_INTERVAL]  Set the FFT capture interval to be used for user-controlled scanning\n");
	printf("                    Units: Microseconds. Default value: 100 us.\n");
	printf("                    Acceptable values: 50 - 500 us\n");
	printf("-b                  Used to force the radio interface to 2.4GHz scanning only. \n");
	printf("-a                  Used to force the radio interface to 5GHz scanning only. \n");
	printf("-ch                 Used to specify a single scan channel in the 5GHz band\n");
	printf("-home               Used to specify a single scan channel on the home channel in the 5GHz band. \n");
	printf("-core               Used to specify the core to use for the scan\n");
	printf("-phy_mode           Used to specify the PHY mode to use for the scan 4x4 or 3+1\n");
	printf("-p [IPADDR]         Set the IP address of airiq_service server. The default\n");
	printf("                    to 127.0.0.1 for local connection\n");
	printf("-h or -help         Display the command line help text.\n");
#ifdef CLASSIFIER_DEBUG
	printf("-log <filename>     Log debug data to a file. Specify filename without extension.\n");
	printf("-logtime <dur sec>  Duration, in seconds, to capture before exiting.\n");
#endif // endif
	printf("\n");
	printf("Examples:\n");
	printf("1. Scan both 2.4GHz and 5GHz band on single radio\n");
	printf("dwell time: default; capture time: default; capture interval: default\n");
	printf("    <airiq_app -i eth2>\n\n");
	printf("2. Scan only 2.4GHz band on single radio\n");
	printf("dwell time: 1000ms; capture time: 10ms; capture interval: 200us\n");
	printf("    <airiq_app -i eth2 -b -d 1000 -c 10 -int 200>\n\n");
	printf("3. Scan only 5GHz band on single radio\n");
	printf("dwell time: 1000ms; capture time: 10ms; capture interval: default\n");
	printf("    <airiq_app -i eth2 -a -d 1000 -c 10>\n\n");
	printf("4. Scan only 5GHz home channel on single radio\n");
	printf("dwell time: 1000ms; capture time: 10ms; capture interval: default\n");
	printf("    <airiq_app -i eth2 -d 1000 -c 10 -a -home>\n\n");
	printf("5. Scan 5GHz channel 36 on single radio\n");
	printf("dwell time: 1000ms; capture time: 10ms; capture interval: default\n");
	printf("    <airiq_app -i eth2 -d 1000 -c 10 -a -ch 36>\n\n");

	printf("\n\nFor multiple radio interfaces \n");
	printf("-i [2.4GHZ_RADIO] -i [5GHZ_RADIO]               Specify interface name. The first radio\n");
	printf("                                                specified is the 2.4GHz radio and the second is 5GHz radio \n");
	printf("-d [2.4GHZ_DWELL_TIME] -d [5GHZ_DWELL_TIME]     Set the dwell time to be used for each radio I/F\n");
	printf("                                                Units: Milliseconds. Default value: 1000 ms.\n");
	printf("-c [2.4GHZ_CAPTURE_TIME] -c [5GHZ_CAPTURE_TIME] Set the capture time to be used for each radio I/F\n");
	printf("                                                Units: Milliseconds. Default value: 0 ms.\n");
	printf("-int [2.4GHZ_CAPTURE_INTERVAL] -int [5GHZ_CAPTURE_INTERVAL] Set the capture time to be used for each radio I/F\n");
	printf("                                                Units: Microseconds. Default value: 100 us.\n");
	printf("-ch                                             Used to specify a single scan channel in the 5GHz band\n");
	printf("-home                                           Used to scan ONLY the home channel in the 5GHz band. \n");
	printf("-p [IPADDR]                                     Set the IP address of airiq_service server. The default\n");
	printf("                                                to 127.0.0.1 for local connection\n");
	printf("-h or -help                                     Display the command line help text.\n");
	printf("\n");
	printf("Examples:\n");
	printf("1. Scan 2.4GHz on first radio and 5GHz on second radio\n");
	printf("dwell time: default; capture time: default; capture interval: default\n");
	printf("    <airiq_app -i eth2 -i eth3>\n\n");
	printf("2. Scan 2.4GHz on first radio and 5GHz on second radio\n");
	printf("dwell time: 1000ms (eth2) 5000ms (eth3); capture time: 10ms; capture interval 200us\n");
	printf("    <airiq_app -i eth2 -d 1000 -c 10 -int 200 -i eth3 -d 5000 -c 10 -int 200>\n\n");
	printf("3. Scan 2.4GHz on first radio and 5GHz home channel on second radio\n");
	printf("dwell time: 1000ms (eth2) 1000ms (eth3); capture time: 10ms; capture interval: default\n");
	printf("    <airiq_app -i eth2 -d 1000 -c 10 -i eth3 -d 1000 -c 10 -home>\n\n");
	printf("4. Scan 2.4GHs on first radio and 5GHz channel 36 on second radio\n");
	printf("dwell time: 1000ms (eth2) 2000ms (eth3); capture time: 10ms; capture interval: default\n");
	printf("    <airiq_app -i eth2 -d 1000 -c 10 -i eth3 -d 2000 -c 10 -ch 36>\n\n");
}

void catch_signal(int sig_num)
{
	if (gTerminate) {
		return;
	}

	switch (sig_num) {
	case SIGINT:
		gTerminate++;
		break;
	case SIGTERM:
		gTerminate++;
		break;
	default:
		break;
	}

	if (gTerminate) {
		if (gLogEventMask) {
			gLogEventMask = 0;
			if (gIntFile) {
				fclose(gIntFile);
				gIntFile = NULL;
			}
			if (gDbgLogFile) {
				fclose(gDbgLogFile);
				gDbgLogFile = NULL;
			}
			if (gFFTCpxFile) {
				fclose(gFFTCpxFile);
				gFFTCpxFile = NULL;
			}
		}
	}
}

const unsigned char bcm_ctype[256];
/* #define bcm_ismask_local(x)   ((0xff & x)) */
#define _BCM_S       0x20    /* white space (space/lf/tab) */
/* #define bcm_isspace_local(c)  (bcm_ismask_local(c) == _BCM_S) */
#define DEV_TYPE_LEN 3

int FindDefaultInterface()
{
	char         proc_net_dev[] = "/proc/net/dev";
	FILE        *fp;
	char         buf[1000], *c, *name;
	char         dev_type[DEV_TYPE_LEN];
	int          status;
	struct ifreq ifr;
	char        *ifname = cfg_ifname[0];

	ifname[0] = '\0';

	if (!(fp = fopen(proc_net_dev, "r"))) {
		return BCME_ERROR;
	}

	/* eat first two lines */
	if (!fgets(buf, sizeof(buf), fp) ||
	    !fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return BCME_ERROR;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		c = buf;
		while (bcm_isspace(*c)) {
			c++;
		}
		if (!(name = strsep(&c, ":"))) {
			continue;
		}

		strncpy(ifname, name, IFNAMSIZ);
		strncpy(ifr.ifr_name, name, IFNAMSIZ);
		if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
		    !strncmp(dev_type, "wl", 2)) {

			if (wl_probe(name) == 0) {
				break;
			}
		}
		ifname[0] = '\0';
	}
	if (ifname[0] == '\0') {
		status = BCME_ERROR;
	} else {
		status = BCME_OK;
	}
	fclose(fp);
	return status;
}

/*  */
/* Callback function for connection status events. */
/*  */
void ConnEventHandler(BSA_EVENT_ID      EventId,
                      BSA_DEVICE_HANDLE DevHandle,
                      void             *EventData)
{
	BSA_CONN_STATUS_MSG *msg;

	if (EventId != BSA_EVENT_ID_CONN_STATUS) {
		printf("%s: Unexpected event id 0x%x\n", __FUNCTION__, EventId);
		exit(-1);
	}

	msg = (BSA_CONN_STATUS_MSG*)EventData;
	switch (msg->Event) {
	case BSA_CONN_ESTABLISHED:
		printf("%s: Connection established.\n", __FUNCTION__);
		break;
	case BSA_CONN_TIMEOUT:
		printf("%s: Connection timeout.\n", __FUNCTION__);
		break;
	case BSA_CONN_TERMINATED:
		printf("%s: Connection terminated.\n", __FUNCTION__);
		exit(0);
		break;
	default:
		printf("%s: Unknown connection status event: 0x%x.\n", __FUNCTION__, msg->Event);
		break;
	}
}

/*  */
/* Callback function for aggregate FFT events. */
/*  */
void AggrFftHandler(BSA_EVENT_ID      EventId,
                    BSA_DEVICE_HANDLE DevHandle,
                    void             *EventData)
{
	BSA_AGGR_FFT_DATA_MSG *msg;

	if (EventId != BSA_EVENT_ID_AGGR_FFT_DATA) {
		printf("%s: Unexpected event id 0x%x\n", __FUNCTION__, EventId);
		exit(-1);
	}

	gAggrFFTCount++;

	msg = (BSA_AGGR_FFT_DATA_MSG*)EventData;

	PRINTF_EVENT(BSA_EVENT_ID_AGGR_FFT_DATA,
	             "@event:AGGR_FFT_DATA, 0x%08x,%4d MHz,%4d MHz,\n",
	             msg->Timestamp.SecondsLo, msg->StartFreqKhz / 1000, msg->StopFreqKhz / 1000);
}

/*  */
/* Callback function for interference classification events. */
/*  */
void InterferenceHandler(BSA_EVENT_ID      EventId,
                         BSA_DEVICE_HANDLE DevHandle,
                         void             *EventData)
{
	BSA_INTERFERENCE_MSG *msg;

	if (EventId != BSA_EVENT_ID_INTERFERENCE) {
		printf("%s: Unexpected event id 0x%x\n", __FUNCTION__, EventId);
		exit(-1);
	}

	gInterferenceCount++;

	msg = (BSA_INTERFERENCE_MSG*)EventData;

	PRINTF_EVENT(BSA_EVENT_ID_INTERFERENCE,
	             "@event:INTERFERENCE, 0x%08x, 0x%08x, %s, %d kHz, Duty Cycle=%3d%% RSSI=%4ddBm\n",
	             msg->Timestamp.SecondsLo, msg->InterfererType, INTERFERER_STR(msg->InterfererType),
	             msg->FreqKhz, msg->ChanUtil / 100, msg->RssiDbm);

	if (gLogEventMask & BSA_EVENT_ID_INTERFERENCE && gIntFile) {
		fprintf(gIntFile,
		        "@event:INTERFERENCE, 0x%08x, 0x%08x, %s, %d kHz, Duty Cycle=%3d%% RSSI=%4ddBm\n",
		        msg->Timestamp.SecondsLo, msg->InterfererType, INTERFERER_STR(msg->InterfererType),
		        msg->FreqKhz, msg->ChanUtil / 100, msg->RssiDbm);
	}
}

/*  */
/* Callback function for channel utilization events. */
/*  */
void ChanUtilHandler(BSA_EVENT_ID      EventId,
                     BSA_DEVICE_HANDLE DevHandle,
                     void             *EventData)
{
	BSA_CHAN_UTIL_WITH_INT_MSG  *msg;
	BSA_UINT32                   k, n;
	BSA_UINT8                   *p;
	BSA_INT_CHAN_UTIL_DATA      *pint;
	BSA_CHAN_UTIL_WITH_INT_DATA *pcu;

	if (EventId != BSA_EVENT_ID_CHAN_UTIL_WITH_INT) {
		printf("%s: Unexpected event id 0x%x\n", __FUNCTION__, EventId);
		exit(-1);
	}

	gChanUtilCount++;

	msg = (BSA_CHAN_UTIL_WITH_INT_MSG*)EventData;

	if (msg->Last) {
		PRINTF_EVENT(BSA_EVENT_ID_CHAN_UTIL_WITH_INT,
		             "@event:CHAN_UTIL_WITH_INT, 0x%08x, Seqn:0x%4x, %d channels, LAST\n",
		             msg->Timestamp.SecondsLo, msg->SeqNumber, msg->Count);
	} else {
		PRINTF_EVENT(BSA_EVENT_ID_CHAN_UTIL_WITH_INT,
		             "@event:CHAN_UTIL_WITH_INT, 0x%08x, Seqn:0x%4x, %d channels, NOTLAST\n",
		             msg->Timestamp.SecondsLo, msg->SeqNumber, msg->Count);
	}
	p = msg->ChanUtilData;

	/* format of the data: */
	/* <BSA_CHAN_UTIL_WITH_INT_MSG> */
	/* <BSA_CHAN_UTIL_WITH_INT_DATA 0> */
	/* [BSA_INT_CHAN_UTIL_DATA 0] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 0) */
	/* [BSA_INT_CHAN_UTIL_DATA 1] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 1) */
	/* [BSA_INT_CHAN_UTIL_DATA 2] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 2) */
	/* [BSA_INT_CHAN_UTIL_DATA 3] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 3) */
	/* <BSA_CHAN_UTIL_WITH_INT_DATA 1> */
	/* [BSA_INT_CHAN_UTIL_DATA 0] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 0) */
	/* [BSA_INT_CHAN_UTIL_DATA 1] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 1) */
	/* [BSA_INT_CHAN_UTIL_DATA 2] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 2) */
	/* [BSA_INT_CHAN_UTIL_DATA 3] (if BSA_CHAN_UTIL_WITH_INT_DATA.NumInterferers > 3) */
	/* ... */

	for (k = 0; k < msg->Count; k++) {
		pcu = (BSA_CHAN_UTIL_WITH_INT_DATA*)p;
		PRINTF_EVENT(BSA_EVENT_ID_CHAN_UTIL_WITH_INT,
		             "#eventdata:%2d Channel %3d: Non-wifi: %3d%% ",
		             k, pcu->ChannelNum, pcu->NonWlanUtil / 100);

		/* Increment read ptr by chan util datum */
		p   += sizeof(BSA_CHAN_UTIL_WITH_INT_DATA);
		pint = (BSA_INT_CHAN_UTIL_DATA*)p;

		/* If there are interferer-cu records, print those out. */
		for (n = 0; n < pcu->NumInterferers; n++) {
			PRINTF_EVENT(BSA_EVENT_ID_CHAN_UTIL_WITH_INT,
			             "[Type:%x %d%%]", pint[n].IntType, pint[n].ChanUtil / 100);
		}
		PRINTF_EVENT(BSA_EVENT_ID_CHAN_UTIL_WITH_INT, "\n");

		/* Increment read ptr */
		p += pcu->NumInterferers * sizeof(BSA_INT_CHAN_UTIL_DATA);

	}
}

void ScanModeChangeHandler(BSA_EVENT_ID      EventId,
                           BSA_DEVICE_HANDLE DevHandle,
                           void             *EventData)
{
	BSA_SCAN_MODE_CHANGE_MSG *msg;
	struct SCAN_MODE         *sm;

	msg = (BSA_SCAN_MODE_CHANGE_MSG*)EventData;

	sm  = GetScanModeByCode(msg->ScanMode);

	PRINTF_EVENT(BSA_EVENT_ID_SCAN_MODE_CHANGE,
	             "@event:SCAN_MODE_CHANGE, 0x%08x, %s [%s]\n",
	             msg->Timestamp.SecondsLo, sm->name, sm->codeName);
}

void ScanCompleteHandler(BSA_EVENT_ID      EventId,
                         BSA_DEVICE_HANDLE DevHandle,
                         void             *EventData)
{
	BSA_SCAN_COMPLETE_MSG *msg;
	int ret;
	int ix;
	int fnd = 0;

	msg = (BSA_SCAN_COMPLETE_MSG*)EventData;

	for (ix = 0; ix < cfg_ifcount; ix++) {
		/* if it is a match */
		if (!strcmp(cfg_ifname[ix], msg->IfName)) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		PRINTF_EVENT(BSA_EVENT_ID_SCAN_COMPLETE,
		             "!ERROR: Received scan complete for %s. I don't know about this interface\n",
		             msg->IfName);
		return;
	}

	if (msg->TimestampValid) {
		switch (msg->Status) {
		case BSA_SUCCESS:
			PRINTF_EVENT(BSA_EVENT_ID_SCAN_COMPLETE,
			             "@event:SCAN_COMPLETE, 0x%08x, %s, success, %d usec\n",
			             msg->Timestamp.SecondsLo, msg->IfName, msg->EndTimestamp - msg->StartTimestamp);
			break;
		case BSA_ABORTED:
			PRINTF_EVENT(BSA_EVENT_ID_SCAN_COMPLETE,
			             "@event:SCAN_COMPLETE, 0x%08x, %s, error aborted, %d usec\n",
			             msg->Timestamp.SecondsLo, msg->IfName, msg->EndTimestamp - msg->StartTimestamp);
			checkbw = TRUE;
			break;
		default:
			PRINTF_EVENT(BSA_EVENT_ID_SCAN_COMPLETE,
			             "@event:SCAN_COMPLETE, 0x%08x, %s, error unknown, %d usec\n",
			             msg->Timestamp.SecondsLo, msg->IfName, msg->EndTimestamp - msg->StartTimestamp);
			break;
		}
	} else {
		switch (msg->Status) {
		case BSA_SUCCESS:
			PRINTF_EVENT(BSA_EVENT_ID_SCAN_COMPLETE,
			             "@event:SCAN_COMPLETE, 0x%08x, %s, success\n",
			             msg->Timestamp.SecondsLo, msg->IfName);
			break;
		case BSA_ABORTED:
			PRINTF_EVENT(BSA_EVENT_ID_SCAN_COMPLETE,
			             "@event:SCAN_COMPLETE, 0x%08x, %s, error abort\n",
			             msg->Timestamp.SecondsLo, msg->IfName);
			checkbw = TRUE;
			break;
		default:
			PRINTF_EVENT(BSA_EVENT_ID_SCAN_COMPLETE,
			             "@event:SCAN_COMPLETE, 0x%08x, %s, error unknown status: 0x%08x\n",
			             msg->Timestamp.SecondsLo, msg->IfName, msg->Status);
			break;
		}
	}
	ret = pthread_mutex_lock(&gScanMutex);
	if (ret) {
		fprintf(stderr, "%s: pthread_mutex_lock failed err=%d\n",
		        __FUNCTION__, ret);
		exit(-1);
	}

	ret = pthread_cond_signal(&gScanCond);
	if (ret) {
		fprintf(stderr, "%s: pthread_cond_signal failed err=%d\n",
		        __FUNCTION__, ret);
		exit(-1);
	}

	ret = pthread_mutex_unlock(&gScanMutex);
	if (ret) {
		fprintf(stderr, "%s: pthread_mutex_unlock failed err=%d\n",
		        __FUNCTION__, ret);
		exit(-1);
	}
}

#if defined(AIRIQ_APP_SIRP)
/*  */
/* Connect to a host with specified hostname. */
/*  */
void ConnectToHost(char *host, BSA_DEVICE_HANDLE *DevHandle)
{
	struct addrinfo      *ai, *aip;
	struct addrinfo       ai_hints;
	struct sockaddr_in   *ipv4, alt_sa;
	struct hostent       *h;
	int                   ret;

	BSA_IP_ADDR           bsaIp = 0;
	BSA_STATUS            status;
	BSA_CONN_VERSION_INFO connVersionInfo;
	BSA_VENDOR_INFO       vendorInfo;
	BSA_CHAR              err[256];

	bzero(&ai_hints, sizeof(ai_hints));
	ai_hints.ai_family = AF_INET;
	ai_hints.ai_flags  = AI_CANONNAME;
	ret                = getaddrinfo(host, NULL, &ai_hints, &ai);

	if (ret) {
		printf("warning: getaddrinfo failed: %s (%d) errno=%d %s\n", gai_strerror(ret), ret,
		       errno, strerror(errno));
		/*  */
		/* Try alternative method */
		/*  */
		printf("trying gethostbyname (legacy)...\n");
		h = gethostbyname(host);
		if (h == NULL) {
			printf("gethostbyname error: errno=%d %s\n", errno, strerror(errno));
			exit(-1);
		}
		bcopy( h->h_addr, &(alt_sa.sin_addr.s_addr), h->h_length);
		ipv4 = &alt_sa;
	} else {

		for (aip = ai; aip != NULL; aip = aip->ai_next) {
			switch (aip->ai_family) {
			case AF_INET:
				ipv4 = (struct sockaddr_in*)aip->ai_addr;
				break;
			default:
				printf("unsupported family.\n");
				exit(-1);
			}
			bsaIp = (BSA_IP_ADDR)htonl(ipv4->sin_addr.s_addr);
			break;
		}
		freeaddrinfo(ai);
	}
	status = BsaConnect(bsaIp, /* IN:IPv4 Address */
	                    0, /* IN:Device Id 0 */
	                    38182, /* IN:TCP port # */
	                    ConnEventHandler, /* IN:Conn status event handler */
	                    DevHandle, /* OUT:Device handle */
	                    &connVersionInfo, /* OUT:Version info */
	                    &vendorInfo, /* OUT:Vendor info */
	                    err); /* OUT:Error message */

	if (status != BSA_SUCCESS) {
		printf("BsaConnect error code 0x%x: %s\n", status, err);
		exit(-1);
	}
	printf("BsaConnect succeeded. VendorId = 0x%x API Version=%d.%d SirpVer=%d\n",
	       vendorInfo.VendorId,
	       connVersionInfo.BsaApiVerMajor,
	       connVersionInfo.BsaApiVerMinor,
	       connVersionInfo.SirpVer);

}
#elif defined(AIRIQ_APP_LOCAL)
void ConnectToHost(char *host, BSA_DEVICE_HANDLE *DevHandle)
{
	BSA_STATUS            status;
	BSA_CONN_VERSION_INFO connVersionInfo;
	BSA_VENDOR_INFO       vendorInfo;
	BSA_CHAR              err[256];

	(void)host; /* unused parameter. */

	status = BsaConnect(0, /* IN:IPv4 Address */
	                    0, /* IN:Device Id 0 */
	                    0, /* IN:TCP port # */
	                    ConnEventHandler, /* IN:Conn status event handler */
	                    DevHandle, /* OUT:Device handle */
	                    &connVersionInfo, /* OUT:Version info */
	                    &vendorInfo, /* OUT:Vendor info */
	                    err); /* OUT:Error message */

	if (status != BSA_SUCCESS) {
		printf("BsaConnect error code 0x%x: %s\n", status, err);
		exit(-1);
	}
	printf("BsaConnect succeeded. VendorId = 0x%x API Version=%d.%d SirpVer=%d\n",
	       vendorInfo.VendorId,
	       connVersionInfo.BsaApiVerMajor,
	       connVersionInfo.BsaApiVerMinor,
	       connVersionInfo.SirpVer);

}

void TerminateMgrSvc(void)
{
	BSA_MS_MSG Msg;
	// Send a terminate message
	Msg.MsgType = MS_MSG_TERMINATE;
	Msg.DataSize = 0;
	//printf("Terminating Manager Service\\n");
	MsQueueMsg(&Msg);
}
#endif /* AIRIQ_APP_LOCAL */

/*  */
/* Convert BSA Bandwidth to MHz value. */
/*  */
BSA_UINT32 BandwidthToMHz(BSA_BANDWIDTH bw)
{
	switch (bw) {
	case BSA_BANDWIDTH_20MHZ:
		return 20;
	case BSA_BANDWIDTH_40MHZ:
		return 40;
	case BSA_BANDWIDTH_80MHZ:
		return 80;
	case BSA_BANDWIDTH_30MHZ:
		return 30;
	default:
		return 0;
	}
}

/*  */
/* Print scan mode details to console. */
/*  */
void DumpScanMode(BSA_SCAN_CHANNEL Channels[], BSA_UINT32 Count)
{
	BSA_UINT32 k;

	for (k = 0; k < Count; k++) {
		printf("%2d| Channel %3d: %d MHz Band 0x%3x BW: %d MHz ",
		       k, Channels[k].ChannelNumber, Channels[k].CenterFreqMhz,
		       Channels[k].FreqBand, BandwidthToMHz(Channels[k].FrontEndBw));
		if (Channels[k].DoDisplay) {
			printf("[FFT] ");
		}
		if (Channels[k].DoClassification) {
			printf("[Class Int. Map=0x%x] ", Channels[k].InterfererMap);
		}
		if (Channels[k].DoChannelUtilization) {
			printf("[CU]");
		}
		printf("\n");
	}
}

struct SCAN_MODE *GetScanModeByCode(BSA_SCAN_MODE m)
{
	int k;

	for (k = 0; k < gScanModeCount; k++) {
		if (gScanModeTable[k].code == m) {
			return &gScanModeTable[k];
		}
	}
	return NULL;
}

/*  */
/* Queries scan mode definition from the BSA device and prints detailed */
/* info to the console. */
/*  */
void DisplayScanMode(BSA_DEVICE_HANDLE DevHandle)
{
	BSA_SCAN_CHANNEL  ScanChannels[BSA_CHANNELS_PER_SCAN_MODE];
	BSA_UINT32        Count;
	BSA_STATUS        status;
	BSA_SCAN_MODE     ScanMode;
	BSA_AGC_MODE      Dummy;
	struct SCAN_MODE *sm;

	status = BsaGetScanMode(DevHandle, &ScanMode, &Dummy);
	if (status != BSA_SUCCESS) {
		printf("%s: BsaGetScanMode call failed: 0x%x\n",
		       __FUNCTION__, status);
		exit(-1);
	}

	/* Initialize Count to the length of the ScanChannels buffer */
	Count  = BSA_CHANNELS_PER_SCAN_MODE;

	status = BsaGetScanModeDefinition(DevHandle,
	                                  ScanMode,
	                                  &Count,
	                                  ScanChannels);

	if (status != BSA_SUCCESS) {
		printf("%s: BsaGetScanModeDefinition call failed: 0x%x\n",
		       __FUNCTION__, status);
		exit(-1);
	}

	sm = GetScanModeByCode(ScanMode);

	if (sm == NULL) {
		printf("%s: GetScanModeByCode failed. Could not find mode 0x%x\n",
		       __FUNCTION__, ScanMode);
		exit(-1);
	}

	printf("Scan mode: %s [%s]\n", sm->name, sm->codeName);
	DumpScanMode(ScanChannels, Count);

}

/*  */
/* Demonstrates Registering for events using BSA API. */
/*  */
void RegisterForEvents(BSA_DEVICE_HANDLE DevHandle)
{
	BSA_STATUS status;

	status = BsaRegisterEventHandler(
	        DevHandle,
	        BSA_EVENT_ID_AGGR_FFT_DATA,
	        AggrFftHandler);

	if (status != BSA_SUCCESS) {
		printf("%s: BsaRegisterEventHandler failed (AGGR_FFT). status = 0x%x\n",
		       __FUNCTION__, status);
		exit(-1);
	}

	status = BsaRegisterEventHandler(
	        DevHandle,
	        BSA_EVENT_ID_INTERFERENCE,
	        InterferenceHandler);

	if (status != BSA_SUCCESS) {
		printf("%s: BsaRegisterEventHandler failed (INTERFERENCE). status = 0x%x\n",
		       __FUNCTION__, status);
		exit(-1);
	}

	status = BsaRegisterEventHandler(
	        DevHandle,
	        BSA_EVENT_ID_CHAN_UTIL_WITH_INT,
	        ChanUtilHandler);

	if (status != BSA_SUCCESS) {
		printf("%s: BsaRegisterEventHandler failed (CHAN_UTIL_WITH_INT). status = 0x%x\n",
		       __FUNCTION__, status);
		exit(-1);
	}

	status = BsaRegisterEventHandler(
	        DevHandle,
	        BSA_EVENT_ID_SCAN_COMPLETE,
	        ScanCompleteHandler);

	if (status != BSA_SUCCESS) {
		printf("%s: BsaRegisterEventHandler failed (SCAN_COMPLETE). status = 0x%x\n",
		       __FUNCTION__, status);
		exit(-1);
	}
}

void FFTCpxLogger(BSA_FFTCPX_DATA_MSG *msg)
{
	int nitems;

	gFFTCpxCount++;
	gFFTCpxBytes += 7 * sizeof(BSA_UINT32) + msg->Length;

	/* Log to file */
	nitems = fwrite(&msg->Timestamp.SecondsHi, sizeof(msg->Timestamp.SecondsHi), 1, gFFTCpxFile);
	nitems = fwrite(&msg->Timestamp.SecondsLo, sizeof(msg->Timestamp.SecondsLo), 1, gFFTCpxFile);
	nitems = fwrite(&msg->Timestamp.TimeZone, sizeof(msg->Timestamp.TimeZone), 1, gFFTCpxFile);
	nitems = fwrite(&msg->FFTCount, sizeof(msg->FFTCount), 1, gFFTCpxFile);
	nitems = fwrite(&msg->Length, sizeof(msg->Length), 1, gFFTCpxFile);
	nitems = fwrite(&msg->Reserved1, sizeof(msg->Reserved1), 1, gFFTCpxFile);
	nitems = fwrite(&msg->Reserved2, sizeof(msg->Reserved2), 1, gFFTCpxFile);

	nitems = fwrite(msg->FFTData, 1, msg->Length, gFFTCpxFile);
}

void DebugDataLogger(BSA_DEBUG_LOG_MSG *msg)
{
	int nitems;

	gDbgLogCount++;
	gDbgLogBytes += msg->Length + 2 * sizeof(BSA_UINT32);

	/* Log to file */
	nitems = fwrite(msg->DebugStream, 1, msg->Length, gDbgLogFile);
}
/*  */
/* Callback function for FFT CPX raw data */
/*  */
void FFTCpxHandler(BSA_EVENT_ID      EventId,
                   BSA_DEVICE_HANDLE DevHandle,
                   void             *EventData)
{
	BSA_FFTCPX_DATA_MSG *msg;

	if (!gFFTCpxFile) {
		fprintf(stderr, "%s: Null file\n", __FUNCTION__);
		exit(-1);
	}

	if (EventId != BSA_EVENT_ID_FFTCPX) {
		fprintf(stderr, "%s: Unexpected event id 0x%x\n", __FUNCTION__, EventId);
		exit(-1);
	}

	if (gLogEventMask & BSA_EVENT_ID_FFTCPX) {
		msg = (BSA_FFTCPX_DATA_MSG*)EventData;
		FFTCpxLogger(msg);
	}
}

/*  */
/* Callback function for debug log events */
/*  */
void DebugLogHandler(BSA_EVENT_ID      EventId,
                     BSA_DEVICE_HANDLE DevHandle,
                     void             *EventData)
{
	BSA_DEBUG_LOG_MSG *msg;

	if (!gDbgLogFile) {
		fprintf(stderr, "%s: Null file\n", __FUNCTION__);
		exit(-1);
	}

	if (EventId != BSA_EVENT_ID_DEBUG_LOG) {
		fprintf(stderr, "%s: Unexpected event id 0x%x\n", __FUNCTION__, EventId);
		exit(-1);
	}

	if (gLogEventMask & BSA_EVENT_ID_DEBUG_LOG) {
		msg = (BSA_DEBUG_LOG_MSG*)EventData;
		DebugDataLogger(msg);
	}
}

void OpenDumpFiles(char *filename, FILE **FFTCpxFile, FILE **DbgFile, FILE **IntFile)
{
	char *fname;
	int   len;

	len   = strnlen(filename, 255) + 8;

	fname = (char*)malloc(len);

	if (!fname) {
		fprintf(stderr, "Out of memory.\n");
		exit(-1);
	}

	snprintf(fname, len, "%s.fft", filename);

	/* FFT File */
	*FFTCpxFile = fopen(fname, "w");

	if (!*FFTCpxFile) {
		fprintf(stderr, "error opening %s for writing: %s\n", fname, strerror(errno));
		exit(-1);
	}
	printf("Opened %s for writing.\n", fname);
	/* DBG File */
	snprintf(fname, len, "%s.dbg", filename);

	*DbgFile = fopen(fname, "w");

	if (!*DbgFile) {
		fprintf(stderr, "error opening %s for writing: %s\n", fname, strerror(errno));
		exit(-1);
	}
	printf("Opened %s for writing.\n", fname);

	snprintf(fname, len, "%s.int", filename);

	*IntFile = fopen(fname, "w");

	if (!*IntFile) {
		fprintf(stderr, "error opening %s for writing: %s\n", fname, strerror(errno));
		exit(-1);
	}
	printf("Opened %s for writing.\n", fname);

	free(fname);
}

void CloseDumpFiles(FILE *FFTFile, FILE *DbgFile, FILE *IntFile)
{
	fclose(FFTFile);
	fclose(DbgFile);
	fclose(IntFile);
}

void RegisterForLogEvents(BSA_DEVICE_HANDLE DevHandle)
{
	BSA_STATUS status;

	status = BsaRegisterEventHandler(
	        DevHandle,
	        BSA_EVENT_ID_DEBUG_LOG,
	        DebugLogHandler);

	if (status != BSA_SUCCESS) {
		fprintf(stderr, "%s: BsaRegisterEventHandler failed (DBG LOG). status = 0x%x\n",
		        __FUNCTION__, status);
		exit(-1);
	}
	status = BsaRegisterEventHandler(
	        DevHandle,
	        BSA_EVENT_ID_INTERFERENCE,
	        InterferenceHandler);

	if (status != BSA_SUCCESS) {
		fprintf(stderr, "%s: BsaRegisterEventHandler failed (INTERFERENCE). status = 0x%x\n",
		        __FUNCTION__, status);
		exit(-1);
	}

	status = BsaRegisterEventHandler(
	        DevHandle,
	        BSA_EVENT_ID_FFTCPX,
	        FFTCpxHandler);

	if (status != BSA_SUCCESS) {
		fprintf(stderr, "%s: BsaRegisterEventHandler failed (FFT_CPX). status = 0x%x\n",
		        __FUNCTION__, status);
		exit(-1);
	}
}

int GetInterfaceBand(char *ifname)
{
	int band;
	int s, err;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		/* syserr("socket"); */
		printf("Error: %s(%d) could not open socket: %s",
		       __FUNCTION__, __LINE__, strerror(errno));
		return -1;
	}

	err = wl_ioctl(ifname, WLC_GET_BAND, &band, sizeof(band));

	if (err) {
		printf("%s: Could not get band for interface %s. err=%d\n",
		       __FUNCTION__, ifname, err);
		return -1;
	}
	close(s);

	return band;
}

void ConfigureSWSAScanParam(airiq_config_t *pParam, airiq_scan_specs_t *scanspec)
{
	int        channels_24GHz_40MHz[CHANNELS_24GHZ_40MHZ_COUNT] = { 3, 6, 9, 14 };

	BSA_UINT32 cnt = 0;
	BSA_UINT32 i;
/*  */
	BSA_UINT16 longdwell        = scanspec->longdwell;
	BSA_UINT16 longfftcnt       = scanspec->longfftcnt;
	BSA_UINT16 shortdwell       = scanspec->shortdwell;
	BSA_UINT16 shortfftcnt      = scanspec->shortfftcnt;
	int        aband            = scanspec->aband;
	int        bband            = scanspec->bband;
	int        scan_channel     = scanspec->scan_channel;
	int        core             = scanspec->core;
	int        phy_mode         = scanspec->phy_mode;
	int        bw               = scanspec->bw;
	BSA_UINT16 capture_interval = scanspec->capture_interval;
	int        scan80only       = scanspec->scan80only;

/*  */

	pParam->sweep_cnt = 1;
	pParam->start     = 1;

	if (aband) {
		if (scan_channel) {
			pParam->chanspec_list[cnt]       = scan_channel  | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
			pParam->dwell_interval_ms[cnt]   = longdwell;
			pParam->capture_interval_us[cnt] = capture_interval;
			pParam->capture_count[cnt]       = longfftcnt;
			pParam->core_config[cnt]         = core;
			cnt++;
		} else {
			if (bw == WL_CHANSPEC_BW_20) {
				/* 5GHz low 36,40,44,48,52,56,60,64 (8 channels)  --> 80ms */
				for (i = 36; i <= 64; i = i + 4) {
					pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
					pParam->dwell_interval_ms[cnt]   = shortdwell;
					pParam->capture_interval_us[cnt] = capture_interval;
					pParam->capture_count[cnt]       = shortfftcnt;
					pParam->core_config[cnt]         = core;
					cnt++;
				}
				/* 5GHz mid 100,104,108,112,116,120,124,128,132,136,140 (11 channels) --> 110ms */
				for (i = 100; i <= 140; i = i + 4) {
					pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
					pParam->dwell_interval_ms[cnt]   = shortdwell;
					pParam->capture_interval_us[cnt] = capture_interval;
					pParam->capture_count[cnt]       = shortfftcnt;
					pParam->core_config[cnt]         = core;
					cnt++;
				}

				/* 5GHz high 148, 149, 152, 153, 156, 157, 160, 161, 164, 165, 168, 170 (12 channels) --> 120ms */
				for (i = 149; i <= 165; i = i + 4) {
					uint tmp_i = i - 1;
					pParam->chanspec_list[cnt]       = tmp_i   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
					pParam->dwell_interval_ms[cnt]   = shortdwell;
					pParam->capture_interval_us[cnt] = capture_interval;
					pParam->capture_count[cnt]       = shortfftcnt * 2;
					pParam->core_config[cnt]         = core;
					cnt++;
					pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
					pParam->dwell_interval_ms[cnt]   = shortdwell;
					pParam->capture_interval_us[cnt] = capture_interval;
					pParam->capture_count[cnt]       = shortfftcnt * 2;
					pParam->core_config[cnt]         = core;
					cnt++;
				}
				pParam->chanspec_list[cnt]       = 168   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt * 2;
				pParam->core_config[cnt]         = core;
				cnt++;

				pParam->chanspec_list[cnt]       = 170   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt * 2;
				pParam->core_config[cnt]         = core;
				cnt++;
			} else if (bw == WL_CHANSPEC_BW_40) {
				/* 5GHz low 38,46,54,62 (4 channels) --> 40ms */
				for (i = 38; i <= 62; i = i + 8) {
					pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_40 | WL_CHANSPEC_BAND_5G;
					pParam->dwell_interval_ms[cnt]   = shortdwell;
					pParam->capture_interval_us[cnt] = capture_interval;
					pParam->capture_count[cnt]       = shortfftcnt;
					pParam->core_config[cnt]         = core;
					cnt++;
				}
				/* 5GHz mid 102,110,118,126,134,142 (6 channels) --> 60ms */
				for (i = 102; i <= 142; i = i + 8) {
					pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_40 | WL_CHANSPEC_BAND_5G;
					pParam->dwell_interval_ms[cnt]   = shortdwell;
					pParam->capture_interval_us[cnt] = capture_interval;
					pParam->capture_count[cnt]       = shortfftcnt;
					pParam->core_config[cnt]         = core;
					cnt++;
				}
				/* 5GHz high 149,153,157,161,165 (5 channels) --> 50ms * 2 */
				for (i = 149; i <= 165; i = i + 4) {
					pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_40 | WL_CHANSPEC_BAND_5G;
					pParam->dwell_interval_ms[cnt]   = shortdwell * 2;
					pParam->capture_interval_us[cnt] = capture_interval;
					pParam->capture_count[cnt]       = shortfftcnt * 4;
					pParam->core_config[cnt]         = core;
					cnt++;
				}
			} else if (bw == WL_CHANSPEC_BW_80) {
				pParam->chanspec_list[cnt]       = 42  | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				/* pParam->chanspec_list[cnt] = 0xe02a; */
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt;
				pParam->core_config[cnt]         = core;
				cnt++;

				pParam->chanspec_list[cnt]       = 58  | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt;
				pParam->core_config[cnt]         = core;
				cnt++;

				pParam->chanspec_list[cnt]       = 106 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt;
				pParam->core_config[cnt]         = core;
				cnt++;

				pParam->chanspec_list[cnt]       = 122 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt;
				pParam->core_config[cnt]         = core;
				cnt++;

				pParam->chanspec_list[cnt]       = 138 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt;
				pParam->core_config[cnt]         = core;
				cnt++;

				pParam->chanspec_list[cnt]       = 155 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				/* pParam->chanspec_list[cnt] = 0xe29b; */
				pParam->dwell_interval_ms[cnt]   = longdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = longfftcnt;
				pParam->core_config[cnt]         = core;
				cnt++;

				if (phy_mode == PHYMODE_3x3_1x1) {
					pParam->chanspec_list[cnt] = 168 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				} else if (scan80only) {
					pParam->chanspec_list[cnt] = 165 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
				} else {
					pParam->chanspec_list[cnt] = 165 | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
				}
				pParam->dwell_interval_ms[cnt]   = longdwell / 2;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = longfftcnt / 2;
				pParam->core_config[cnt]         = core;
				cnt++;
			} else {
				printf("%s: Unknown bandwidth bw = 0x%x\n", __FUNCTION__, bw);
			}
		}
	}

	if (bband) {
		if (bw == WL_CHANSPEC_BW_20) {
			/* 14 channels * 10ms --> 140ms */
			for (i = 1; i < 15; i++) {
				pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_2G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt * 2;
				pParam->core_config[cnt]         = core;
				cnt++;
			}
		} else if (bw == WL_CHANSPEC_BW_40) {
			/* 4 channels * 10ms --> 40ms */
			for (i = 0; i < CHANNELS_24GHZ_40MHZ_COUNT; i++) {
				pParam->chanspec_list[cnt]       = channels_24GHz_40MHz[i]  | WL_CHANSPEC_BW_40 | WL_CHANSPEC_BAND_2G;
				pParam->dwell_interval_ms[cnt]   = shortdwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = shortfftcnt * 2;
				pParam->core_config[cnt]         = core;
				cnt++;
			}
		} else if (bw == WL_CHANSPEC_BW_80) {
			pParam->chanspec_list[cnt]       = 7   | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_2G;
			pParam->dwell_interval_ms[cnt]   = longdwell;
			pParam->capture_interval_us[cnt] = capture_interval;
			pParam->capture_count[cnt]       = longfftcnt;
			pParam->core_config[cnt]         = core;
			cnt++;
			if ((phy_mode == PHYMODE_3x3_1x1) || scan80only) {
				pParam->chanspec_list[cnt] = 14   | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_2G;
			} else {
				pParam->chanspec_list[cnt] = 14   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_2G;
			}
			pParam->dwell_interval_ms[cnt]   = shortdwell;
			pParam->capture_interval_us[cnt] = capture_interval;
			pParam->capture_count[cnt]       = shortfftcnt;
			pParam->core_config[cnt]         = core;
			cnt++;
		} else {
			printf("%s: Unknown bandwidth bw = 0x%x\n", __FUNCTION__, bw);
		}
	}
	pParam->chanspec_cnt = cnt;
	pParam->phy_mode     = phy_mode;

	if (cfg_verbose) {
		/* Dump */
		printf("%s: %3d chanspecs phy_mode = %d\n",
		       __FUNCTION__, pParam->chanspec_cnt, pParam->phy_mode);
		for (i = 0; i < pParam->chanspec_cnt; i++) {
			printf("[%4x] core %d dwell=%4dms fft intvl=%4dusec capcnt=%4d\n",
			       pParam->chanspec_list[i], pParam->core_config[i], pParam->dwell_interval_ms[i],
			       pParam->capture_interval_us[i], pParam->capture_count[i]);
		}
	}
}

/*  */
/* AbortSwsaScan() demonstrates how to abort an ongoing */
/* airiq_scan. */
/*  */
int AbortSwsaScan(struct ifreq *if_req)
{
	int ret;

	ret = wl_iovar_setint(if_req->ifr_name, "airiq_scan_abort", 1);
	if (ret) {
		fprintf(stderr, "%s: set_buf airiq_scan abort failure.%d\n",
		        __FUNCTION__, ret);
	}

	return ret;
}

int DoSwsaScan(airiq_config_t *airiqScanConfig, struct ifreq *if_req, int homescan)
{
	int             ret;
	struct timeval  tv0;
	struct timezone tz;
	struct timespec req;
	char            setBuf[2500];
	int             scancompleted;
	int             up;
	int             bcmerr;

	up            = 0;
	scancompleted = 0;
	gettimeofday(&tv0, &tz);

	/* Event-driven approach: Wait for the scan-complete event */
	ret = pthread_mutex_lock(&gScanMutex);
	if (ret) {
		fprintf(stderr, "%s: mutex lock failed err=%d\n",
		        __FUNCTION__, ret);
		exit(-1);
	}

	/* Test if radio is up */
	ret = wl_ioctl(if_req->ifr_name, WLC_GET_UP, &up, sizeof(int));
	if (ret) {
		fprintf(stderr, "%s: iovar failed getting 'up' status %d. up=%d\n",
		        __FUNCTION__, ret, up);
		exit(-1);
	}

	if ( up ) {
		if (homescan) {
			ret = wl_iovar_setint(if_req->ifr_name, "airiq_home_scan", airiqScanConfig->capture_count[0]);
			if (ret) {
				fprintf(stderr, "%s: set_buf airiq_home_scan (%d) failure.%d\n",
				        __FUNCTION__, airiqScanConfig->capture_count[0], ret);
			}
		} else {
			do {
				ret = wl_iovar_setbuf(if_req->ifr_name, "airiq_scan", airiqScanConfig,
				                      sizeof(airiq_config_t), &setBuf, sizeof(setBuf));
				if (ret) {
					ret = wl_iovar_getint(if_req->ifr_name, "bcmerror", &bcmerr);
					if (!ret) {
						if (bcmerr == BCME_UNSUPPORTED) {
							fprintf(stderr, "ERROR: %s: Invalid scan params (BCME_UNSUPPORTED %d).\n",
							        __FUNCTION__, bcmerr);
							exit(-1);
						} else if (bcmerr == BCME_BUSY) {
							fprintf(stderr, "WARNING: %s: scan in progress (BCME_BUSY %d).\n",
							        __FUNCTION__, bcmerr);
						} else {
							fprintf(stderr, "ERROR: %s: Unknown failure (%d).\n",
							        __FUNCTION__, bcmerr);
							exit(-1);
						}
					}
					fprintf(stderr, "%s: airiq_scan failure: %d. Sleep 1 sec and retry...\n",
					        __FUNCTION__, bcmerr);

					req.tv_sec  = 1;
					req.tv_nsec = 0;
					nanosleep(&req, &req);
				}
			} while (ret);
		}
		/* set scan timeout to to 2 sec */
		req.tv_sec  = tv0.tv_sec + 2;
		req.tv_nsec = tv0.tv_usec * 1000;

		ret         = pthread_cond_timedwait(&gScanCond, &gScanMutex, &req);

		if (ret == ETIMEDOUT) {
			fprintf(stderr, "warning: wait time out without scan completion signal.\n");
		} else if (ret) {
			fprintf(stderr, "%s: pthread_cond_timedwait failed err=%d\n",
			        __FUNCTION__, ret);
			exit(-1);
		} else {
			scancompleted = 1;
		}
	} else { /* not up */
		scancompleted = -1;
	}

	ret = pthread_mutex_unlock(&gScanMutex);
	if (ret) {
		fprintf(stderr, "%s: mutex unlock failed err=%d\n",
		        __FUNCTION__, ret);
		exit(-1);
	}

	return scancompleted;
}

void ComputeScanSpecs(airiq_scan_req_t *scanreq, airiq_scan_specs_t *scanspecs)
{
	int ret;

	if (scanreq->capture_time == 0) {
		if (scanreq->dwell_time < 400) {
			fprintf(stderr, "%s: Illegal dwell time: %d. Too small.\n",
			        __FUNCTION__, scanreq->dwell_time);
			exit(-1);
		} else if (scanreq->dwell_time < 450) {
			scanspecs->longdwell   = 20;
			scanspecs->longfftcnt  = 150;
			scanspecs->shortdwell  = 10;
			scanspecs->shortfftcnt = 20;
		} else if (scanreq->dwell_time < 550) {
			scanspecs->longdwell   = 20;
			scanspecs->longfftcnt  = 200;
			scanspecs->shortdwell  = 10;
			scanspecs->shortfftcnt = 20;
		} else if (scanreq->dwell_time < 650) {
			scanspecs->longdwell   = 30;
			scanspecs->longfftcnt  = 250;
			scanspecs->shortdwell  = 10;
			scanspecs->shortfftcnt = 20;
		} else if (scanreq->dwell_time < 750) {
			scanspecs->longdwell   = 30;
			scanspecs->longfftcnt  = 300;
			scanspecs->shortdwell  = 10;
			scanspecs->shortfftcnt = 20;
		} else if (scanreq->dwell_time < 850) {
			scanspecs->longdwell   = 40;
			scanspecs->longfftcnt  = 350;
			scanspecs->shortdwell  = 10;
			scanspecs->shortfftcnt = 20;
		} else if (scanreq->dwell_time < 950) {
			scanspecs->longdwell   = 40;
			scanspecs->longfftcnt  = 400;
			scanspecs->shortdwell  = 10;
			scanspecs->shortfftcnt = 20;
		} else {
			scanspecs->longdwell   = 50;
			scanspecs->longfftcnt  = 450;
			scanspecs->shortdwell  = 10;
			scanspecs->shortfftcnt = 20;
		}
	} else {
		scanspecs->longdwell   = scanreq->capture_time;
		scanspecs->longfftcnt  = scanreq->capture_time * 10;
		scanspecs->shortdwell  = scanreq->capture_time;
		scanspecs->shortfftcnt = scanreq->capture_time * 10;
	}
	scanspecs->band             = scanreq->wlband;
	scanspecs->aband            = scanreq->aband;
	scanspecs->bband            = scanreq->bband;
	scanspecs->scan_channel     = scanreq->scan_channel;
	scanspecs->core             = scanreq->core;
	scanspecs->phy_mode         = scanreq->phy_mode;
	scanspecs->capture_interval = scanreq->capture_interval;

	if (scanreq->revinfo.corerev == 56) {
		scanspecs->scan80only = 1;
	} else if (scanreq->revinfo.corerev >= 64 && scanreq->phy_mode == 0) {
		scanspecs->scan80only = 1;
	} else {
		scanspecs->scan80only = 0;
	}
	if (scanreq->phy_mode == PHYMODE_3x3_1x1 && scanreq->core == 3 &&
	    //scanreq->revinfo.corerev == 65) {
	    scanreq->revinfo.corerev >= 65) {
		int chanspec_int = 0;
		/* Get bandwidth from wl driver */
		ret = wl_iovar_getint(scanreq->if_req.ifr_name, "chanspec", &chanspec_int);
		if (ret) {
			fprintf(stderr, "%s: iovar failed getting 'chanspec' status %d. chanspec=%x\n",
			        __FUNCTION__, ret, chanspec_int);
			exit(-1);
		}

		scanspecs->bw = CHSPEC_BW((chanspec_t)chanspec_int);
	} else if (scanreq->phy_mode == PHYMODE_3x3_1x1 && scanreq->core == 3 &&
		scanreq->revinfo.corerev >= 128) {
		scanspecs->bw = WL_CHANSPEC_BW_80;
		scanspecs->scan80only = 1;
	} else if (scanreq->phy_mode == 0) {
		scanspecs->bw = WL_CHANSPEC_BW_80;
	} else {
		fprintf(stderr, "%s: Invalid phy mode/core request: %d core:%d chip rev %d\n",
		        __FUNCTION__, scanreq->phy_mode, scanreq->core, scanreq->revinfo.corerev);
		exit(-1);
	}
}

void DumpScanReq(airiq_scan_req_t *scanreq)
{
	printf("*******************************\n");
	printf("Scan request for [%s]\n", scanreq->ifname);
	printf("Core rev:        %d\n", scanreq->revinfo.corerev);
	printf("dwell_time:      %d\n", scanreq->dwell_time);
	printf("wlband:          %d\n", scanreq->wlband);
	printf("aband:           %d\n", scanreq->aband);
	printf("bband:           %d\n", scanreq->bband);
	printf("capture_time:    %d\n", scanreq->capture_time);
	printf("scan_channel     %d\n", scanreq->scan_channel);
	printf("homescan:        %d\n", scanreq->homescan);
	printf("core:            %d\n", scanreq->core);
	printf("phy_mode:        %d\n", scanreq->phy_mode);
	printf("capture_interval:%d\n", scanreq->capture_interval);
	printf("offloads:        0x%x\n", scanreq->offloads);
	printf("*******************************\n");
}

void StartUserControlledSWSAScanning(int ifcnt, airiq_scan_req_t *scanreqs)
{
	airiq_config_t  airiqScanConfig[2];
	struct ifreq    if_req, if_req2;
	struct timeval  tv1, tv0;
	struct timezone tz;
	int             dt;
	struct timespec req;
	int             timeToScan[2];
	int             idx;
	int             scan_cntr, scan_offset;
	int             scancompleted;
	int             newbw;
	int             ret;
	int             i;

	/*  */
	airiq_scan_specs_t scanspec[2];

	bzero(scanspec, sizeof(scanspec));
	/*  */

	bzero(&tz, sizeof(tz));
	bzero(&tv0, sizeof(tv0));
	bzero(&airiqScanConfig, sizeof(airiqScanConfig));

	gettimeofday(&tv0, &tz);

	if (cfg_verbose) {
		printf("%s: Configured with %d interfaces\n", __FUNCTION__, cfg_ifcount);
		for (i = 0; i < ifcnt; i++) {
			printf("%s: if: %s\n", __FUNCTION__, scanreqs[i].ifname);
			DumpScanReq(&scanreqs[i]);
		}
	}

	bzero(&if_req, sizeof(struct ifreq));
	bzero(&if_req2, sizeof(struct ifreq));
	strncpy(if_req.ifr_name, scanreqs[0].ifname, IFNAMSIZ);

	if (cfg_ifcount > 1) {
		strncpy(if_req2.ifr_name, scanreqs[1].ifname, IFNAMSIZ);

		timeToScan[0] = scanreqs[0].dwell_time;
		timeToScan[1] = scanreqs[1].dwell_time;
	} else {
		timeToScan[0] = 0;
		timeToScan[1] = 0;
	}

	/*  */
	/* Configure the scan parameters. */
	/*  */
	for (i = 0; i < cfg_ifcount; i++) {
		ComputeScanSpecs(&scanreqs[i], &scanspec[i]);
		ConfigureSWSAScanParam(&airiqScanConfig[i], &scanspec[i]);

		if (scanreqs[i].bband) {
			printf("2GHz: %s corerev=%d phymode=%d core=%d scan80only=%d bw=0x%4x\n",
			       scanreqs[i].ifname, scanreqs[i].revinfo.corerev, scanreqs[i].phy_mode,
			       scanreqs[i].core, scanspec[i].scan80only, scanspec[i].bw);
		}
		if (scanreqs[i].aband) {
			printf("5GHz: %s corerev=%d phymode=%d core=%d scan80only=%d bw=0x%4x\n",
			       scanreqs[i].ifname, scanreqs[i].revinfo.corerev, scanreqs[i].phy_mode,
			       scanreqs[i].core, scanspec[i].scan80only, scanspec[i].bw);
		}
	}

	scan_cntr = 0;
	while (1) {
		if (checkbw) {
			for (i = 0; i < cfg_ifcount; i++) {
				if (scanreqs[i].phy_mode == PHYMODE_3x3_1x1 && scanreqs[i].core == 3 &&
					//scanreqs[i].revinfo.corerev < 128) {
					scanreqs[i].revinfo.corerev < 130) {
					int chanspec;
					/* Check bandwidth and reconfigure airiqScanConfig */
					ret = wl_iovar_getint(scanreqs[i].if_req.ifr_name, "chanspec", &chanspec);
					if (ret) {
						fprintf(stderr, "%s: iovar failed getting 'chanspec' status %d. chanspec=%x\n",
						        __FUNCTION__, ret, chanspec);
						exit(-1);
					}

					newbw = CHSPEC_BW((chanspec_t)chanspec);

					if (ret) {
						fprintf(stderr, "%s: iovar failed getting 'chanspec' status %d. chanspec=%x\n",
						        __FUNCTION__, ret, chanspec);
						exit(-1);
					} else if (newbw != scanspec[i].bw) {
						printf("%s: New Bandwidth = 0x%x Old Bandwidth = 0x%x\n",
						       __FUNCTION__, newbw, scanspec[i].bw);
						scanspec[i].bw = newbw;
						ConfigureSWSAScanParam(&airiqScanConfig[i], &scanspec[i]);
					}
				}
			}
			checkbw = FALSE;
		}
		/* Introduce small timing offset to reduce synchronization */
		/* with FHSS phones. */
		if (scan_cntr == 10) {
			scan_cntr   = 0;
			scan_offset = 977;
		} else {
			scan_offset = 0;
		}
		scan_cntr++;

		if (cfg_ifcount == 1) {
			printf("I/F 0: Sleeping %d ms.\n", scanreqs[0].dwell_time + scan_offset);
			req.tv_sec  = scanreqs[0].dwell_time / 1000;
			req.tv_nsec = MIN(999999999, (scanreqs[0].dwell_time + scan_offset - req.tv_sec * 1000) * 1000000);
			nanosleep(&req, &req);

			gettimeofday(&tv0, &tz);
			scancompleted = DoSwsaScan(&airiqScanConfig[0], &scanreqs[0].if_req, scanreqs[0].homescan);

			if (scancompleted < 0) {
				/* The radio is not up */
				printf("I/F 0: Radio is down. Sleeping 10 sec.\n");
				req.tv_sec  = 10;
				req.tv_nsec = 0;
				nanosleep(&req, &req);
			} else {
				gettimeofday(&tv1, &tz);
				dt = (tv1.tv_sec - tv0.tv_sec) * 1000 + (tv1.tv_usec - tv0.tv_usec) / 1000;
				printf("I/F 0: SWSA time (%s): %d ms\n", scanreqs[0].ifname, dt);
			}
			/***********
			*  Vendor specific scanning code. i.e. perform rogue detection and mitigation scan,
			*  you would do it here.
			***********/
		} else {
			/* If there are two interfaces in use */
			if (scanreqs[0].dwell_time != scanreqs[1].dwell_time) {
				/* Two interfaces have different dwell times */
				/* Figure out which interface is to be scanned next and */
				/* update timeToScan for each interface accordingly. */
				if (timeToScan[0] < timeToScan[1]) {
					idx = 0;
				} else {
					idx = 1;
				}
				printf("Sleeping %d ms.\n", timeToScan[idx]);
				req.tv_sec  = timeToScan[idx] / 1000;
				req.tv_nsec = MIN(999999999, (timeToScan[idx] + scan_offset - req.tv_sec * 1000) * 1000000);
				nanosleep(&req, &req);

				gettimeofday(&tv0, &tz);
				scancompleted = DoSwsaScan(&airiqScanConfig[idx], &scanreqs[idx].if_req,
				                           scanreqs[idx].homescan);
				gettimeofday(&tv1, &tz);
				dt = (tv1.tv_sec - tv0.tv_sec) * 1000 + (tv1.tv_usec - tv0.tv_usec) / 1000;
				printf("I/F %d: SWSA time (%s): %d ms\n", idx, scanreqs[idx].ifname, dt);

				if (scancompleted < 0) {
					/* The radio is not up */
					printf("I/F %d: Radio is down. Retry in 10 sec\n", idx);
					timeToScan[idx] = 10 * 1000;
				} else {
					if (idx == 0) {
						timeToScan[1] = MAX(timeToScan[1] - timeToScan[0] - dt, 10);
						timeToScan[0] = scanreqs[0].dwell_time;
					} else {
						timeToScan[0] = MAX(timeToScan[0] - timeToScan[1] - dt, 10);
						timeToScan[1] = scanreqs[1].dwell_time;
					}
				}
			} else {
				/* Two interfaces have same dwell times */
				printf("Sleeping %d ms.\n", scanreqs[0].dwell_time);
				req.tv_sec  = scanreqs[0].dwell_time / 1000;
				req.tv_nsec = MIN(999999999, (scanreqs[0].dwell_time + scan_offset -
				                              req.tv_sec * 1000) * 1000000);
				nanosleep(&req, &req);

				/* Interface 0 */
				gettimeofday(&tv0, &tz);
				scancompleted = DoSwsaScan(&airiqScanConfig[0], &scanreqs[0].if_req,
				                           scanreqs[0].homescan);
				if (scancompleted < 0) {
					/* The radio is not up */
					printf("I/F 0: Radio is down.\n");
				}
				gettimeofday(&tv1, &tz);
				dt = (tv1.tv_sec - tv0.tv_sec) * 1000 + (tv1.tv_usec - tv0.tv_usec) / 1000;
				printf("I/F 0: SWSA time (%s): %d ms\n", scanreqs[0].ifname, dt);

				/* Interface 1 */
				gettimeofday(&tv0, &tz);
				scancompleted = DoSwsaScan(&airiqScanConfig[1], &scanreqs[1].if_req,
				                           scanreqs[1].homescan);
				if (scancompleted < 0) {
					/* The radio is not up */
					printf("I/F 1: Radio is down.\n");
				}
				gettimeofday(&tv1, &tz);
				dt = (tv1.tv_sec - tv0.tv_sec) * 1000 + (tv1.tv_usec - tv0.tv_usec) / 1000;
				printf("I/F 1: SWSA time (%s): %d ms\n", scanreqs[1].ifname, dt);
			}
			/***********
			*  Vendor specific scanning code. i.e. perform rogue detection and mitigation scan,
			*  you would do it here.
			***********/
		}
	}
}

#if defined(AIRIQ_APP_LOCAL)
BSA_MGRSVC_FLAGS gMgrsvcFlags;

void StartMsMainThread(void)
{
	pthread_t      t;

	int            rc;
	pthread_attr_t attr, *attrp;
	BSA_UINT32     tempSize;

	attrp = &attr;

	rc    = pthread_attr_init(attrp);
	if (rc != 0) {
		printf("%s: pthread_attr_init failed! rc=%d\n",
		       __FUNCTION__, rc);
		exit(-1);
	}
#define STACK_SIZE 0x40000
	/* Set the stack size */
	rc = pthread_attr_setstacksize(attrp, STACK_SIZE); /* 256K */
	if (rc != 0) {
		printf("%s: pthread_attr_setstacksize failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}
	/* Set stack size to at least 128KB */
	rc = pthread_attr_getstacksize(attrp, &tempSize);
	if (rc != 0) {
		printf("%s: pthread_attr_getstacksize failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}

	if (tempSize < STACK_SIZE) {
		printf("%s: stack size %d < %d!\n",
		       __FUNCTION__, tempSize, STACK_SIZE);

		rc = pthread_attr_setstacksize(attrp, STACK_SIZE);
		if (rc != 0) {
			printf("%s: pthread_attr_setstacksize failed! rc=%d\n",
			       __FUNCTION__, rc);
			pthread_attr_destroy(attrp);
			exit(-1);
		}
	}

	rc = pthread_attr_setdetachstate(attrp, PTHREAD_CREATE_DETACHED);
	if (rc != 0) {
		printf("%s: pthread_attr_setdetachstate failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}

	gMgrsvcFlags.Magic = BSA_MGRSVC_FLAGS_MAGIC;
	strncpy(gMgrsvcFlags.SwsaConfigFile, "/etc/airiq.cfg", sizeof(gMgrsvcFlags.SwsaConfigFile));

	rc = pthread_create(&t,
	                    attrp,
	                    MsMain,
	                    &gMgrsvcFlags);

	if (rc != 0) {
		printf("%s: pthread_attr_setdetachstate failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}

	pthread_attr_destroy(attrp);
}
#endif /* AIRIQ_APP_LOCAL */

int main(int argc, char*argv[])
{
	char                   host[32] = "127.0.0.1";
	int                    i;
	int                    ifidx    = 0;
	BSA_DEVICE_HANDLE      DevHandle;
	int                    ret;
	int                    bcmerr;
	struct timespec        req;
	BSA_SPECAN_OPTION_DATA OptionData;
	BSA_STATUS             status;
	char                   LogFilename[256];

	/* Check if this is a lte_u scan and call lte_u scan function */
	for (i = 1; i < argc; i++) {
		/* printf("argv[%d] = {%s}\n", i, argv[i]); */
		/* Check for log output option */
		if (strcmp(argv[i], "-lteu") == 0) {
			/* This is a lte_u scan. */
			LTEU_Main(argc, argv);
			return 0;
		}

	}

	airiq_scan_req_t scanreqs[MAX_IF];
	bzero(scanreqs, sizeof(scanreqs));
	/*  */
	for (i = 0; i < MAX_IF; i++) {
		scanreqs[i].dwell_time       = 1000;
		scanreqs[i].capture_time     = 0;
		scanreqs[i].capture_interval = 150;
		scanreqs[i].core             = 3;
		scanreqs[i].phy_mode         = PHYMODE_3x3_1x1;
	}

	PRINTF_VERBOSE("CLI: argc = %d\n", argc);
	/* Get the command line options */
	for (i = 1; i < argc; i++) {
		/* Check for log output option */

		if (strcmp(argv[i], "-i") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					strncpy(scanreqs[cfg_ifcount].ifname, argv[i], IFNAMSIZ);
					scanreqs[cfg_ifcount].ifname[sizeof(scanreqs[cfg_ifcount].ifname) - 1] = '\0';
					strncpy(cfg_ifname[cfg_ifcount], scanreqs[cfg_ifcount].ifname, IFNAMSIZ);
				}
			} else {
				fprintf(stderr, "No interface specified\n");
				exit(-1);
			}
			/* build ifreq */
			bzero(&scanreqs[cfg_ifcount].if_req, sizeof(struct ifreq));
			strncpy(scanreqs[cfg_ifcount].if_req.ifr_name, scanreqs[cfg_ifcount].ifname, IFNAMSIZ);

			/* Query revinfo */
			ret = wl_ioctl(scanreqs[cfg_ifcount].if_req.ifr_name, WLC_GET_REVINFO,
			               &scanreqs[cfg_ifcount].revinfo, sizeof(wlc_rev_info_t));
			if (ret) {
				fprintf(stderr, "%s: iovar failed getting 'revinfo' for %s status %d. \n",
				        __FUNCTION__, scanreqs[cfg_ifcount].ifname, ret);
				exit(-1);
			}
			/* Query offloads */
			ret = wl_iovar_getint(scanreqs[cfg_ifcount].if_req.ifr_name, "offloads", &scanreqs[cfg_ifcount].offloads);
			if (ret) {
				ret = wl_iovar_getint(scanreqs[cfg_ifcount].if_req.ifr_name, "bcmerror", &bcmerr);
				if (!ret && bcmerr == BCME_UNSUPPORTED) {
					/* this indicates offloads is not supported, which is OK */
					scanreqs[cfg_ifcount].offloads = 0;
				} else {
					fprintf(stderr, "%s: Error querying offloads. err=%d\n",
					        __FUNCTION__, bcmerr);
					exit(-1);
				}
			}
			ifidx = cfg_ifcount; /* one less than the number of interfaces */
			cfg_ifcount++;
			continue;
		}
		if (strcmp(argv[i], "-d") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					scanreqs[ifidx].dwell_time = atoi(argv[i]);
					if (scanreqs[ifidx].dwell_time <= 0) {
						fprintf(stderr, "illegal dwell time specified: %s. must be greater than 99 ms.\n", argv[i]);
						exit(-1);
					}
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-c") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					scanreqs[ifidx].capture_time = atoi(argv[i]);
					if (scanreqs[ifidx].capture_time <= 0) {
						fprintf(stderr, "Illegal capture time specified: %s. Must be greater than 99 ms.\n", argv[i]);
						exit(-1);
					}
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-ch") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					scanreqs[ifidx].scan_channel = atoi(argv[i]);
					if (scanreqs[ifidx].scan_channel < 0) {
						fprintf(stderr, "Illegal scan channel number specified: %s. Must be greater than 0.\n", argv[i]);
						exit(-1);
					}
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-b") == 0) {
			scanreqs[ifidx].bband = 1;
			PRINTF_VERBOSE("CLI: argv[%d] = %s\n", i, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-a") == 0) {
			scanreqs[ifidx].aband = 1;
			PRINTF_VERBOSE("CLI: argv[%d] = %s\n", i, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-home") == 0) {
			scanreqs[ifidx].homescan = 1;
			PRINTF_VERBOSE("CLI: argv[%d] = %s\n", i, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-core") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					scanreqs[ifidx].core = atoi(argv[i]);
					if (scanreqs[ifidx].core < 0) {
						fprintf(stderr, "Illegal core number specified: %s. Must be greater than 0.\n", argv[i]);
						exit(-1);
					}
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-phy_mode") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					if ((strcmp(argv[i], "4x4") == 0)) {
						scanreqs[ifidx].phy_mode = 0;
					} else if ((strcmp(argv[i], "3+1") == 0)) {
						scanreqs[ifidx].phy_mode = PHYMODE_3x3_1x1;
					} else {
						fprintf(stderr, "Illegal phy mode  specified: %s. Must be  4x4 or 3+1.\n", argv[i]);
						exit(-1);
					}
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-int") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					scanreqs[ifidx].capture_interval = atoi(argv[i]);
					if (scanreqs[ifidx].capture_interval <= 0) {
						fprintf(stderr, "Illegal capture interval specified: %s. Must be greater than 50usec.\n", argv[i]);
						exit(-1);
					}
				}
			}
			continue;
		}

		if (strcmp(argv[i], "-v") == 0) {
			cfg_verbose = 1;
			PRINTF_VERBOSE("CLI: argv[%d] = %s\n", i, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-print_events") == 0) {
			cfg_print_events = EVENT_MASK_ALL;
			PRINTF_VERBOSE("CLI: argv[%d] = %s\n", i, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-p") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					strncpy(host, argv[i], sizeof(host));
					host[sizeof(host) - 1] = '\0';
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-log") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					strncpy(LogFilename, argv[i], sizeof(LogFilename));
				} else {
					fprintf(stderr, "Specify filename to -log: %s\n", argv[i + 1]);
					exit(-1);
				}
			} else {
				fprintf(stderr, "Specify filename to -log: %s\n", argv[i + 1]);
				exit(-1);
			}
			gLogEventMask = BSA_EVENT_ID_FFTCPX | BSA_EVENT_ID_DEBUG_LOG | BSA_EVENT_ID_INTERFERENCE;
			PRINTF_VERBOSE("CLI: argv[%d] = %s\n", i, argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-logtime") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					PRINTF_VERBOSE("CLI: argv[%d] = %s val=%s\n", i, argv[i], argv[i + 1]);
					i++;
					gLogSeconds = atoi(argv[i]);
					if (gLogSeconds <= 0) {
						fprintf(stderr, "Illegal log time specified: %s. Must be greater than 0.\n", argv[i]);
						exit(-1);
					}
				}
			}
			continue;
		}
		/* Check for help options */
		if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-help") == 0)) {
			Usage();
			printf("***********Help for LTE_U scanning********\n");
			LTEU_Usage();
			exit(0);
		}
	}

	cfg_2gidx = 0;
	cfg_5gidx = 0;

	/* call to start user-controlled SWSA scanning */
	if (cfg_ifcount <= 0) {
		fprintf(stderr, "Please specify WLAN interface to use with " "-i <ifname>" "\n");
		/* This code is currently non-functional. */
		/* FindDefaultInterface(); */
		exit(-1);
	} else if (cfg_ifcount == 1) {
		/* the default is dual-band scan, if no args */
		if (scanreqs[0].aband + scanreqs[0].bband == 0) {
			scanreqs[0].aband = scanreqs[0].bband = 1;
		}
		printf("One radio interface: %s (%s)\n",
		       scanreqs[0].ifname,
		       (scanreqs[0].offloads & 0x100) ? "OFFLOADED" : "NOT OFFLOADED");
	} else if (cfg_ifcount == 2) {
		scanreqs[0].wlband = GetInterfaceBand(scanreqs[0].ifname);
		scanreqs[1].wlband = GetInterfaceBand(scanreqs[1].ifname);

		if (scanreqs[0].wlband < 0 || scanreqs[1].wlband < 0) {
			fprintf(stderr, "Invalid bands: 0x%x 0x%x\n",
			        scanreqs[0].wlband, scanreqs[1].wlband);
			exit(-1);
		}

		if (scanreqs[0].wlband == scanreqs[1].wlband && scanreqs[0].wlband != WLC_BAND_AUTO) {
			fprintf(stderr, "Error: %s and %s set to same band. Only one 2.4 and 5 GHz radio allowed.\n",
			        scanreqs[0].ifname, scanreqs[1].ifname);
			exit(-1);
		}
		/* handle 'auto' bands and settle on a 2g or 5g */
		if (scanreqs[0].wlband == WLC_BAND_AUTO) {
			/* Check the other interface */
			switch (scanreqs[1].wlband) {
			case WLC_BAND_2G:
				/* 0:5G,1:2G */
				scanreqs[0].aband = 1;
				scanreqs[0].bband = 0;
				scanreqs[1].aband = 0;
				scanreqs[1].bband = 1;
				break;
			case WLC_BAND_5G:
				/* 0:2G,1:5G */
				scanreqs[0].aband = 0;
				scanreqs[0].bband = 1;
				scanreqs[1].aband = 1;
				scanreqs[1].bband = 0;
				break;
			case WLC_BAND_AUTO:
				/* 0:2G,1:5G */
				scanreqs[0].aband = 0;
				scanreqs[0].bband = 1;
				scanreqs[1].aband = 1;
				scanreqs[1].bband = 0;
			default:
				printf("Error: Unknown band %s : 0x%x.\n",
				       scanreqs[1].ifname, scanreqs[1].wlband);
				exit(-1);
			}
		} else if (scanreqs[1].wlband == WLC_BAND_AUTO) {
			/* Check the other interface */
			switch (scanreqs[0].wlband) {
			case WLC_BAND_2G:
				/* 0:2G,1:5G */
				scanreqs[0].aband = 0;
				scanreqs[0].bband = 1;
				scanreqs[1].aband = 1;
				scanreqs[1].bband = 0;
				break;
			case WLC_BAND_5G:
				/* 0:5G,1:2G */
				scanreqs[0].aband = 1;
				scanreqs[0].bband = 0;
				scanreqs[1].aband = 0;
				scanreqs[1].bband = 1;
				break;
			case WLC_BAND_AUTO:
				/* 0:2G,1:5G */
				scanreqs[0].aband = 0;
				scanreqs[0].bband = 1;
				scanreqs[1].aband = 1;
				scanreqs[1].bband = 0;
			default:
				printf("Error: Unknown band %s : 0x%x.\n",
				       scanreqs[0].ifname, scanreqs[0].wlband);
				exit(-1);
			}
		} else {
			scanreqs[0].aband = (scanreqs[0].wlband == WLC_BAND_5G) ? 1 : 0;
			scanreqs[0].bband = (scanreqs[0].wlband == WLC_BAND_2G) ? 1 : 0;
			scanreqs[1].aband = (scanreqs[1].wlband == WLC_BAND_5G) ? 1 : 0;
			scanreqs[1].bband = (scanreqs[1].wlband == WLC_BAND_2G) ? 1 : 0;
		}
		/* 2g/5g or 5g/2g only. */
		if (scanreqs[0].aband) {
			cfg_2gidx = 1;
			cfg_5gidx = 0;
		} else {
			cfg_2gidx = 1;
			cfg_5gidx = 0;
		}
		printf("Two radio interfaces: 2.4GHz: %s (%s) and 5GHz: %s (%s)\n",
		       scanreqs[cfg_2gidx].ifname,
		       (scanreqs[cfg_2gidx].offloads & 0x100) ? "OFFLOADED" : "NOT OFFLOADED",
		       scanreqs[cfg_5gidx].ifname,
		       (scanreqs[cfg_5gidx].offloads & 0x100) ? "OFFLOADED" : "NOT OFFLOADED");
	}

	printf("Connecting to %s...\n", host);

#if defined(AIRIQ_APP_LOCAL)
	StartMsMainThread();
	sleep(2);
#endif /* AIRIQ_APP_LOCAL */
	/* Initialize BSA API (required) */
	BsaInitialize();

	ConnectToHost(host, &DevHandle);
	printf("connected to %s\n", host);

	/* Debug logger mode of operation */
	if (gLogEventMask) {
		BSA_UINT32 BytesTotal, BytesTotalLast = 0, BytesDiff;
		int        logtime = 1, sec = 0;

		OpenDumpFiles(LogFilename, &gFFTCpxFile, &gDbgLogFile, &gIntFile);

		RegisterForLogEvents(DevHandle);

		/* wait for user to signal quit */
		signal(SIGINT, catch_signal);
		signal(SIGTERM, catch_signal);

		/* Do something while data is being captured. */
		req.tv_sec  = 1;
		req.tv_nsec = 0;
		if (gLogSeconds > 0) {
			logtime = gLogSeconds;
		}
		printf("Logging data to file... Hit control-C to exit\n");
		while (logtime > 0 && !gTerminate) {
			BSA_UINT32 FFTCpxBytesLast, DbgLogBytesLast, IntBytesLast;

			nanosleep(&req, &req);
			sec += req.tv_sec;

			if (gLogSeconds > 0) {
				logtime -= (int)req.tv_sec;
			}

			BytesTotal = gFFTCpxBytes + gDbgLogBytes + gIntLogBytes;
			BytesDiff  = BytesTotal - BytesTotalLast;
			if (sec > 5) {
				if (gLogSeconds > 0) {
					printf("(T-%d sec) Logged: %8d bytes (%8d kB total)\n",
					       logtime, BytesDiff, BytesTotal / 1024);
				} else {
					printf("Logged: %8d bytes (%8d kB total)\n",
					       BytesDiff, BytesTotal / 1024);
				}
				sec = 0;
			}
			BytesTotalLast = BytesTotal;
		}

	} else {

		/* reset the classifiers */
		OptionData.ResetType = 0;
		status               = BsaSetSpecanOption(DevHandle, BSA_SPECAN_OPTION_CLASSIFIER_RESET, &OptionData);
		if (status != BSA_SUCCESS) {
			fprintf(stderr, "could not reset AirIQ classifiers. status=%d\n",
			        status);
		}

		gAggrFFTCount      = 0;
		gChanUtilCount     = 0;
		gInterferenceCount = 0;

		/* Initialize scan mutex/condition var */
		if (pthread_mutex_init(&gScanMutex, NULL)) {
			fprintf(stderr, "pthread_mutex_init failed.\n");
			exit(-1);
		}

		if (pthread_cond_init(&gScanCond, NULL)) {
			fprintf(stderr, "pthread_cond_init failed.\n");
			exit(-1);
		}

		/* Wait for a while. capture fft's and interference. */
		RegisterForEvents(DevHandle);
		/* ////////////////////////////////////////////// */
		/* ADJUST DUTY-CYCLE MEASUREMENT MINIMUM RSSI */
		/* ////////////////////////////////////////////// */
		OptionData.CuRssiThrDbm = -75;

		status = BsaSetSpecanOption(DevHandle, BSA_SPECAN_OPTION_CU_RSSI_THR, &OptionData);
		if (status != BSA_SUCCESS) {
			fprintf(stderr, "could not set Channel utilization (duty cycle) rssi to -50 dBm. status=%d\n",
			        status);
		}

		/* Here's how to query the duty cycle measurement */
		status = BsaGetSpecanOption(DevHandle, BSA_SPECAN_OPTION_CU_RSSI_THR, &OptionData);
		if (status != BSA_SUCCESS) {
			fprintf(stderr, "could not get Channel utilization (duty cycle) rssi to -50 dBm. status=%d\n",
			        status);
		} else {
			printf("BSA_SPECAN_OPTION_CU_RSSI_THR = %d dBm\n", OptionData.CuRssiThrDbm);
		}
		/* ////////////////////////////////////////////// */
		/* Disable AGC calibration */
		/* ////////////////////////////////////////////// */
		/*
		OptionData.DisableAgcCal = 1;
		status = BsaSetSpecanOption(DevHandle, BSA_SPECAN_OPTION_DISABLE_AGC_CAL, &OptionData);
		if (status != BSA_SUCCESS) {
			fprintf(stderr, "could not disable AGC cal status=%d\n",
			        status);
		}
		*/

		/* StartUserControlledSWSAScanning(dwell_time,bband,aband,capture_time,scan_channel,homescan,core,phy_mode,capture_interval); */
		StartUserControlledSWSAScanning(cfg_ifcount, scanreqs);

		/* Initialize scan mutex/condition var */
		if (pthread_mutex_destroy(&gScanMutex)) {
			fprintf(stderr, "pthread_mutex_destroy failed.\n");
			exit(-1);
		}

		if (pthread_cond_destroy(&gScanCond)) {
			fprintf(stderr, "pthread_cond_destroy failed.\n");
			exit(-1);
		}
	}

	BsaDisconnect(DevHandle);
	BsaShutdown();
#if defined(AIRIQ_APP_LOCAL)
	TerminateMgrSvc();
	sleep(2);
#endif /* AIRIQ_APP_LOCAL */
	return 0;
}

/*************LTE_U scanning related functions*/

void LTEU_Usage(void)
{
	printf("\n");
	printf("Usage:\n\n");
	printf("   airiq_app -lteu [-i [INTERFACE_NAME]]  [-h] [-help]\n\n");
	printf("For single radio interface \n");
	printf("-i [INTERFACE_NAME] Specify interface name. The default\n");
	printf("                    interface used is the first 11AC interface \n");
	printf("-d [DWELL TIME]     Set the dwell time to be used for LTE-U scan\n");
	printf("                    Units: Milliseconds. Default value: 200 ms.\n");
	printf("                    Acceptable values: >= 200ms\n");
	printf("-c [CAPTURE_COUNT]  Set the capture count to be used for LTE-U scan\n");
	printf("                    Default value: 200 \n");
	printf("-int [CAPTURE_INTERVAL]  Set the FFT capture interval to be used for LTE-U scan\n");
	printf("                    Units: Microseconds. Default value: 656 us.\n");
	printf("                    Acceptable values: Min 656us to multiples of 328us\n");
	printf("-lte_cfg [LTE CONFIG REG VALUES]  Set the config for the LTE_U detector\n");
	printf("                    values reg0 reg1 reg2 reg3 reg4 reg5 reg6 reg7 \n");
	printf("-ch                 Used to specify a single scan channel in the 5GHz band\n");
	printf("-h or -help         Display the command line help text.\n");
	printf("\n");
	printf("Examples: \n");
	printf("1. Run with default settings \n");
	printf("airiq_app -lteu -i eth1\n");
	printf("\n");
	printf("2. Increase the capture count and dwell time per channel for higher detection probability \n");
	printf("airiq_app -lteu -i eth1 -d 300 -c 400\n");
	printf("\n");
	printf("3. Set a specific channel instead of scanning the default list of channels \n");
	printf("airiq_app -lteu -i eth1 -ch 36\n");
	printf("\n");

}

void lteu_catch_signal(int sig_num)
{
	if (term) {
		/* printf("%s: Caught duplicate SIGTERM/SIGINT. term=%d\n",__FUNCTION__, term); */
		return;
	}

	switch (sig_num) {
	case SIGINT:
		/* printf("%s: Caught SIGINT. term=%d\n",__FUNCTION__, term); */
		term++;
		break;
	case SIGTERM:
		/* printf("%s: Caught SIGTERM. term=%d\n",__FUNCTION__, term); */
		term++;
		break;
	default:
		/* printf("%s: Caught signal %d.\n",__FUNCTION__,  sig_num); */
		break;
	}

	if (term) {
		/* send the scan abort IOVAR */
		AbortLTEUScan();
		sleep(5);
	}
}

void ConfigureLTEUScanParam(airiq_config_t *pParam,
                            uint16          dwell,
                            uint16          iqcnt,
                            int             scan_channel,
                            int             bw,
                            uint16          capture_interval,
                            int             scan80only)
{

	uint32 cnt = 0;
	uint32 i;

	if (scan_channel) {
		pParam->chanspec_list[cnt]       = scan_channel  | bw | WL_CHANSPEC_BAND_5G;
		pParam->dwell_interval_ms[cnt]   = dwell;
		pParam->capture_interval_us[cnt] = capture_interval;
		pParam->capture_count[cnt]       = iqcnt;
		cnt++;
	} else {
		if (bw == WL_CHANSPEC_BW_20) {
			/* 5GHz low 36,40,44,48 (4 channels) */
			for (i = 36; i <= 48; i = i + 4) {
				pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = dwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = iqcnt;
				cnt++;
			}
			/* 5GHz high 149, 153, 157, 161, 165 (5 channels) */
			for (i = 149; i <= 165; i = i + 4) {
				pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_20 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = dwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = iqcnt;
				cnt++;
			}
		} else if (bw == WL_CHANSPEC_BW_40) {
			/* 5GHz low 38,46 (2 channels) */
			for (i = 38; i <= 46; i = i + 8) {
				pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_40 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = dwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = iqcnt;
				cnt++;
			}
			/* 5GHz high 151,159,165 (5 channels) */
			for (i = 151; i <= 159; i = i + 8) {
				pParam->chanspec_list[cnt]       = i   | WL_CHANSPEC_BW_40 | WL_CHANSPEC_BAND_5G;
				pParam->dwell_interval_ms[cnt]   = dwell;
				pParam->capture_interval_us[cnt] = capture_interval;
				pParam->capture_count[cnt]       = iqcnt;
				cnt++;
			}
			/* Ch 165/40 Used to cover Wi-Fi channel 165/20 */
			pParam->chanspec_list[cnt]       = 165 | WL_CHANSPEC_BW_40 | WL_CHANSPEC_BAND_5G;
			pParam->dwell_interval_ms[cnt]   = dwell;
			pParam->capture_interval_us[cnt] = capture_interval;
			pParam->capture_count[cnt]       = iqcnt;
			cnt++;
		} else if (bw == WL_CHANSPEC_BW_80) {
			pParam->chanspec_list[cnt]       = 42  | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
			pParam->dwell_interval_ms[cnt]   = dwell;
			pParam->capture_interval_us[cnt] = capture_interval;
			pParam->capture_count[cnt]       = iqcnt;
			cnt++;
			pParam->chanspec_list[cnt]       = 155 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
			pParam->dwell_interval_ms[cnt]   = dwell;
			pParam->capture_interval_us[cnt] = capture_interval;
			pParam->capture_count[cnt]       = iqcnt;
			cnt++;
			/* Ch 165/80 Used to cover Wi-Fi channel 165/20 */
			pParam->chanspec_list[cnt]       = 165 | WL_CHANSPEC_BW_80 | WL_CHANSPEC_BAND_5G;
			pParam->dwell_interval_ms[cnt]   = dwell;
			pParam->capture_interval_us[cnt] = capture_interval;
			pParam->capture_count[cnt]       = iqcnt;
			cnt++;
		} else {
			printf("%s: Unknown bandwidth bw = 0x%x\n", __FUNCTION__, bw);
		}
	}

	pParam->chanspec_cnt = cnt;

}

/*  */
/* AbortLTEUScan() demonstrates how to abort an ongoing */
/* lte_u_scan. */
/*  */
int AbortLTEUScan()
{
	int ret;

	ret = wl_iovar_setint(if_req.ifr_name, "lte_u_scan_abort", 1);

	if (ret) {
		fprintf(stderr, "%s: set_buf lte_u_scan abort failure.%d\n",
		        __FUNCTION__, ret);
	}

	return ret;
}

int ConfigLTEUDetector(  )
{
	int  ret;
	char setBuf[2500];
	lte_u_detector_config_t lte_uDetectorConfig;

	if (!lte_cfg_set) {
		return 0;
	}
	lte_uDetectorConfig.nshift    =   cfg_reg[0];
	lte_uDetectorConfig.edlow     =   cfg_reg[1];
	lte_uDetectorConfig.edhigh    =   cfg_reg[2];
	lte_uDetectorConfig.edshift   =   cfg_reg[3];
	lte_uDetectorConfig.dtlow     =   cfg_reg[4];
	lte_uDetectorConfig.dthigh    =   cfg_reg[5];
	lte_uDetectorConfig.dtabslow  =   cfg_reg[6];
	lte_uDetectorConfig.dtabshigh =   cfg_reg[7];

	ret = wl_iovar_setbuf(if_req.ifr_name, "lte_u_detector_config", &lte_uDetectorConfig,
	                      sizeof(lte_u_detector_config_t), &setBuf, sizeof(setBuf));
	if (ret) {
		fprintf(stderr, "%s: lte_u_detector_config failure: %d. .\n",
		        __FUNCTION__, ret);
	}
	return ret;
}

int ConfigLTEUScan(airiq_config_t *lte_uScanConfig)
{
	int  ret;
	char setBuf[2500];

	ret = wl_iovar_setbuf(if_req.ifr_name, "lte_u_scan_config", lte_uScanConfig,
	                      sizeof(airiq_config_t), &setBuf, sizeof(setBuf));
	if (ret) {
		fprintf(stderr, "%s: lte_u_scan_config failure: %d. \n",
		        __FUNCTION__, ret);
	}
	return ret;
}

int DoLTEUScan(airiq_config_t *lte_uScanConfig)
{
	int             ret;
	struct timespec req;
	char            setBuf[2500];
	int             scancompleted;
	int             up;

	up            = 0;
	scancompleted = 0;

	/* Test if radio is up */
	ret = wl_ioctl(if_req.ifr_name, WLC_GET_UP, &up, sizeof(int));

	if (ret) {
		fprintf(stderr, "%s: iovar failed getting 'up' status %d. up=%d\n",
		        __FUNCTION__, ret, up);
		exit(-1);
	}

	if ( up ) {
		do {
			ret = wl_iovar_setint(if_req.ifr_name, "lte_u_scan_start", 0);
			if (ret) {
				fprintf(stderr, "%s: lte_u_scan failure: %d. Sleep 1 sec and retry...\n",
				        __FUNCTION__, ret);

				req.tv_sec  = 1;
				req.tv_nsec = 0;
				nanosleep(&req, &req);
			}
		} while (ret);

		scancompleted = 1;
	} else { /* not up */
		fprintf(stderr, "%s: Radio interface not 'up' Cannot scan status %d. up=%d\n",
		        __FUNCTION__, ret, up);
		exit(-1);
	}

	return scancompleted;
}

void StartUserControlledLTEUScanning(int dwell_time, int capture_count, int scan_channel, int capture_interval)
{
	airiq_config_t  lte_uScanConfig;
	struct timespec req;
	int             ix, idx;
	int             scan_cntr, scan_offset;
	int             scancompleted;
	int             bw;
	int             bw2;
	int             newbw;
	int             ret;
	int             i;
	uint16          dwell;
	uint16          iqcnt;
	int             scan80only;
	wlc_rev_info_t  revinfo;
	int             chanspec;

	bzero(&lte_uScanConfig, sizeof(lte_uScanConfig));

	/* Get bandwidth from wl driver */
	/* ret = wl_ioctl(if_req.ifr_name, WLC_GET_BANDWIDTH,&bw,sizeof(int)); */
	ret = wl_iovar_getint(if_req.ifr_name, "chanspec", &chanspec);
	bw  = CHSPEC_BW((chanspec_t)chanspec);
	if (ret) {
		fprintf(stderr, "%s: iovar failed getting 'bandwidth' status %d. bandwidth=%d\n",
		        __FUNCTION__, ret, bw);
		exit(-1);
	}
	/* printf("%s: Bandwidth = 0x%x\n",__FUNCTION__,bw); */

	/* Find out revinfo */
	ret = wl_ioctl(if_req.ifr_name, WLC_GET_REVINFO, &revinfo, sizeof(wlc_rev_info_t));
	if (ret) {
		fprintf(stderr, "%s: iovar failed getting 'revinfo' status %d. \n",
		        __FUNCTION__, ret);
		exit(0);
	} else {
		if (revinfo.corerev  <  64) {
			fprintf(stderr, "%s: LTE_U scan not supported on this chip revision %d. \n",
			        __FUNCTION__, revinfo.corerev );
			return;
		} else {
			scan80only = 1;
		}
	}

	/*  */
	/* Configure the scan parameters. */
	/*  */
	if (capture_count != 0) {
		if (dwell_time < 2) {
			fprintf(stderr, "%s: Illegal dwell time: %d. Too small.\n",
			        __FUNCTION__, dwell_time);
			return;
		} else {
			dwell = dwell_time;
			iqcnt = capture_count;
		}

		ConfigureLTEUScanParam(&lte_uScanConfig, dwell, iqcnt, scan_channel, bw, capture_interval, scan80only);

		scan_cntr = 0;

		ret = ConfigLTEUScan(&lte_uScanConfig);

		if (ret) {
			fprintf(stderr, " lte_u_scan_config failure: exiting() \n");
			exit(-1);
		}
		if (lte_cfg_set) {
			ConfigLTEUDetector();
		}

		scancompleted = DoLTEUScan(&lte_uScanConfig);

		if (scancompleted < 0) {
			/* The radio is not up */
			printf("I/F 0: Radio is down. Sleeping 10 sec.\n");
			req.tv_sec  = 10;
			req.tv_nsec = 0;
			nanosleep(&req, &req);
		} else {
			printf("I/F (%s): LTE-U scan started\n", if_req.ifr_name);
		}
	}
}

void *ProcessLTEUEvent(lte_u_event_t *lte_u_event)
{
	lte_u_scan_status_t   *lte_scan_status;
	uint8                  i;
	uint32                 abort_status;
	lte_u_iqdata_header_t *hdr;
	uint32                *iqdata;
	FILE                  *fp;

	switch (lte_u_event->lte_u_event_type) {
	case LTE_U_EVENT_SCAN_STATUS:
	{
		printf("@LTE_U EVENT: LTE_U_EVENT_SCAN_STATUS Event\n");
		lte_scan_status = (lte_u_scan_status_t*)lte_u_event->data;
		if (lte_scan_status->lte_u_active) {
			printf("@LTE_U EVENT: LTE-U DETECTED channel=%d rssi=%d  lte_u_active=%d  \n", lte_scan_status->chanspec & 0xff,
			       lte_scan_status->rssi_smoothed, lte_scan_status->lte_u_active );
		} else {
			printf("@LTE_U EVENT: LTE-U NOT DETECTED channel=%d rssi=%d  lte_u_active=%d  \n", lte_scan_status->chanspec & 0xff,
			       lte_scan_status->rssi_smoothed, lte_scan_status->lte_u_active );
		}

		break;
	}
	case LTE_U_EVENT_SCAN_ABORT:
	{
		abort_status = *(uint32*)lte_u_event->data;
		printf("@LTE_U EVENT: LTE_U_EVENT_SCAN_ABORT Event reason=%x  \n", abort_status );
		term++;
		break;
	}
	case LTE_U_EVENT_IQ_CAPTURE:
	{
		fp     = fopen("/tmp/lte_capture.iq", "a");
		iqdata = (uint32*)(lte_u_event->data);
		/* printf("@LTE_U EVENT: IQ capture data recieved data_len=%d\n",lte_u_event->data_len); */
		if (fp) {
			for (i = 0; i < (lte_u_event->data_len - sizeof(lte_u_event_t)) / 4; i++) {
				fprintf(fp, "%08x\n", iqdata[i]);
			}
			fclose(fp);
		}
		break;
	}
	default:
		printf("unKnown LTE event type\n");
		break;
	}
}

void *WaitLTEUEventThread(void *lte_flags)
{

	int                fd, err, octets;
	struct sockaddr_in sockaddr;
	int                reuse = 1;
	fd_set             readset;
	char              *data;
	uint               event_type;
	struct ifreq       ifr;
	int                result;
	bcm_event_t       *event;
	uint32             status;
	lte_u_event_t     *lte_u_event;
	int                pkt_offset = IFNAMSIZ;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, cfg_ifname_lteu, (IFNAMSIZ - 1));

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		printf("Cannot create socket %d\n", fd);
		return NULL;
	}

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family      = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port        = htons(EAPD_WKSP_LTE_U_UDP_SPORT);

	if ((err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse))) < 0) {
		printf("Cannot setsockopt %d\n", err);
		close(fd);
		return NULL;
	}

	if ((err = bind(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr))) < 0) {
		printf("Cannot bind %d\n", err);
		close(fd);
		return NULL;
	}

	data = (char*)malloc(WL_EVENTS_BUFFER_SIZE);

	if (data == NULL) {
		printf("Cannot not allocate %d bytes for events receive buffer\n",
		       WL_EVENTS_BUFFER_SIZE);
		close(fd);
		return NULL;
	}

	printf("Waiting for LTE_U events  \n");

	do {
		/* Call select() */
		do {
			FD_ZERO(&readset);
			FD_SET(fd, &readset);
			result = select(fd + 1, &readset, NULL, NULL, NULL);
		} while (result == -1 && errno == EINTR);
		if (result > 0) {
			memset(data, 0, WL_EVENTS_BUFFER_SIZE);
			octets     = recv(fd, data, WL_EVENTS_BUFFER_SIZE, 0);
			UNUSED_PARAMETER(octets);
			event      = (bcm_event_t*)(data + pkt_offset);
			event_type = ntoh32(event->event.event_type);
			switch (event_type) {
			case WLC_E_LTE_U_EVENT:
			{
				status = ntoh32(event->event.status);
				if (status == WLC_E_STATUS_SUCCESS) {

					lte_u_event = (lte_u_event_t*)&data[ sizeof(bcm_event_t) + pkt_offset ];
					ProcessLTEUEvent(lte_u_event);

				} else {
					printf("WLC_E_LTE_U_EVENT received without WLC_E_STATUS_SUCCESS: status =%d\n", status);
				}
				break;
			}
			default:
				printf(" Not a LTE-U Event !!!! event_type %d received\n", event_type);
				break;
			}
			fflush(stdout);
		}
	} while (term == 0);

	return NULL;
}

void StartLTEUMainThread(void)
{
	pthread_t      t;

	int            rc;
	pthread_attr_t attr, *attrp;
	uint32         tempSize;

	attrp = &attr;

	rc    = pthread_attr_init(attrp);
	if (rc != 0) {
		printf("%s: pthread_attr_init failed! rc=%d\n",
		       __FUNCTION__, rc);
		exit(-1);
	}
#define STACK_SIZE 0x40000
	/* Set the stack size */
	rc = pthread_attr_setstacksize(attrp, STACK_SIZE); /* 256K */
	if (rc != 0) {
		printf("%s: pthread_attr_setstacksize failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}
	/* Set stack size to at least 128KB */
	rc = pthread_attr_getstacksize(attrp, &tempSize);
	if (rc != 0) {
		printf("%s: pthread_attr_getstacksize failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}

	if (tempSize < STACK_SIZE) {
		printf("%s: stack size %d < %d!\n",
		       __FUNCTION__, tempSize, STACK_SIZE);

		rc = pthread_attr_setstacksize(attrp, STACK_SIZE);
		if (rc != 0) {
			printf("%s: pthread_attr_setstacksize failed! rc=%d\n",
			       __FUNCTION__, rc);
			pthread_attr_destroy(attrp);
			exit(-1);
		}
	}

	rc = pthread_attr_setdetachstate(attrp, PTHREAD_CREATE_DETACHED);
	if (rc != 0) {
		printf("%s: pthread_attr_setdetachstate failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}

	rc = pthread_create(&t,
	                    attrp,
	                    WaitLTEUEventThread,
	                    &gMainFlags);

	if (rc != 0) {
		printf("%s: pthread_attr_setdetachstate failed! rc=%d\n",
		       __FUNCTION__, rc);
		pthread_attr_destroy(attrp);
		exit(-1);
	}

	pthread_attr_destroy(attrp);
}

int LTEU_Main(int argc, char*argv[])
{
	int i, k;
	int dwell_time;
	int capture_count;
	int capture_interval;
	int scan_channel = 0;
	int cfg_ifcount  = 0;

	/* capture_count 200 * capture_interval 656us = 131.2ms should be less than dwell time 200ms */
	dwell_time       = 200;
	capture_count    = 200;
	capture_interval = 656;

	/* Get the command line options */
	for (i = 1; i < argc; i++) {
		/* printf("argv[%d] = {%s}\n", i, argv[i]); */
		/* Check for log output option */
		if (strcmp(argv[i], "-i") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					i++;
					strncpy(cfg_ifname_lteu, argv[i], sizeof(cfg_ifname_lteu));
					cfg_ifname_lteu[sizeof(cfg_ifname_lteu) - 1] = '\0';
				}
			}
			cfg_ifcount++;
			continue;
		}
		if (strcmp(argv[i], "-d") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					i++;
					dwell_time = atoi(argv[i]);
					if (dwell_time <= 0) {
						fprintf(stderr, "Illegal dwell time specified: %s. Must be greater than 99 ms.\n", argv[i]);
						exit(0);
					}
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-c") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					i++;
					capture_count = atoi(argv[i]);
					if (capture_count <= 0) {
						fprintf(stderr, "Illegal capture count specified: %s. Must be greater than 0.\n", argv[i]);
						exit(0);
					}
				}
			}
			continue;
		}
		if (strcmp(argv[i], "-ch") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					i++;
					scan_channel = atoi(argv[i]);
					if (scan_channel < 0) {
						fprintf(stderr, "Illegal scan channel number specified: %s. Must be greater than 0.\n", argv[i]);
						exit(0);
					}
				}
			}
			continue;
		}

		if (strcmp(argv[i], "-lte_cfg") == 0) {
			for (k = 0; k < 8; k++) {
				if ((i + 1) < argc) {
					if (argv[i + 1][0] != '-') {
						i++;
						cfg_reg[k] = atoi(argv[i]);
					}

				}
				if (k != 8) {
					fprintf(stderr, "Illegal number of lte_u config parameters. Must be 8.\n");
					exit(0);
				}
			}
			lte_cfg_set = TRUE;
			continue;
		}

		if (strcmp(argv[i], "-int") == 0) {
			if ((i + 1) < argc) {
				if (argv[i + 1][0] != '-') {
					i++;
					capture_interval = atoi(argv[i]);
					if (capture_interval <= 0) {
						fprintf(stderr, "Illegal capture interval specified: %s. Must be greater than 50usec.\n", argv[i]);
						exit(0);
					}
				}
			}
			continue;
		}

		/* Check for help options */
		if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-help") == 0)) {
			LTEU_Usage();
			exit(0);
		}
	}

	if (cfg_ifcount <= 0) {
		fprintf(stderr, "Please specify WLAN interface to use with " "-i <ifname>" "\n");
		exit(-1);
	}
	/* printf("%s: if: %s\n", __FUNCTION__, cfg_ifname_lteu); */

	bzero(&if_req, sizeof(struct ifreq));
	strncpy(if_req.ifr_name, cfg_ifname_lteu, IFNAMSIZ);

	/* This thread will wait for LTE_U events */
	StartLTEUMainThread();
	sleep(2);

	/* call to start user-controlled LTEU scanning */
	StartUserControlledLTEUScanning(dwell_time, capture_count, scan_channel, capture_interval);

	/* wait for user to signal ABORT with q */
	signal(SIGINT, lteu_catch_signal);
	signal(SIGTERM, lteu_catch_signal);

	do {
		sleep(5);
	} while (term == 0);

	return 0;
}
