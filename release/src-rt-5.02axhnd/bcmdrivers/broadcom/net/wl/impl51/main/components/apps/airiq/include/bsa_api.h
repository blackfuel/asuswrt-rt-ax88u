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
 * $Id: bsa_api.h 729449 2017-10-31 22:40:50Z $
 */

#ifndef __BSA_API_H__
#define __BSA_API_H__

/*******************************************************************************
 * BSA Application Programming Interface (BSA-API)                             *
 * --- ----------- ----------- --------- ---------                             *
 *                                                                             *
 * SUMMARY: Provides access to BSA devices and software.                       *
 *                                                                             *
 * Documentation: See BSA-API design specification.                            *
 *                                                                             *
 *                                                                             *
 ******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif // endif
/*******************************************************************************
 * Includes                                                                    *
 ******************************************************************************/
#include "bsa_data_type.h"

/*******************************************************************************
 * API Version Information                                                     *
 ******************************************************************************/
#define BSA_API_VERSION_MAJOR 2
#define BSA_API_VERSION_MINOR 5

/*******************************************************************************
 * Function prototypes                                                         *
 ******************************************************************************/
BSADLLSPEC BSA_STATUS BsaInitialize(void);

BSADLLSPEC BSA_STATUS BsaShutdown(void);

BSADLLSPEC BSA_IP_ADDR BsaIpAddr(
        BSA_UINT8 oct0,
        BSA_UINT8 oct1,
        BSA_UINT8 oct2,
        BSA_UINT8 oct3
        );

BSADLLSPEC BSA_STATUS BsaGetNumLocalDevices(BSA_UINT32 *NumDevices);

BSADLLSPEC BSA_STATUS BsaConnect(
        BSA_IP_ADDR            IpAddr,
        BSA_UINT32             DeviceId,
        BSA_UINT32             TcpPort,
        BSA_EVENT_HANDLER      EventHandler,
        BSA_DEVICE_HANDLE     *DevHandle,
        BSA_CONN_VERSION_INFO *SvrVersionInfo,
        BSA_VENDOR_INFO       *SvrVendorInfo,
        BSA_CHAR              *ErrorMsg
        );

BSADLLSPEC BSA_STATUS BsaConnectWithKey(
        BSA_IP_ADDR            ipAddr,
        BSA_UINT32             deviceId,
        BSA_UINT32             tcpPort,
        BSA_EVENT_HANDLER      eventHandler,
        BSA_UINT32             authKeyLen,
        BSA_UINT8             *authKey,
        BSA_DEVICE_HANDLE     *bsaDevHandle,
        BSA_CONN_VERSION_INFO *svrVersionInfo,
        BSA_VENDOR_INFO       *svrVendorInfo,
        BSA_CHAR              *errorMsg
        );

BSADLLSPEC BSA_STATUS BsaDisconnect(BSA_DEVICE_HANDLE DevHandle);

BSADLLSPEC BSA_STATUS BsaGetCapabilities(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_UINT8        *Caps,
        BSA_UINT32       *Length
        );

BSADLLSPEC BSA_STATUS BsaGetSerialNumber(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_UINT32       *Length,
        BSA_UINT8        *SerialNumber
        );

BSADLLSPEC BSA_STATUS BsaGetScanMode(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_SCAN_MODE    *ScanMode,
        BSA_AGC_MODE     *AgcMode
        );

BSADLLSPEC BSA_STATUS BsaSetScanMode(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_SCAN_MODE     ScanMode,
        BSA_AGC_MODE      AgcMode
        );

BSADLLSPEC BSA_STATUS BsaGetScanModeDefinition(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_SCAN_MODE     ScanMode,
        BSA_UINT32       *ChannelCount,
        BSA_SCAN_CHANNEL  Channels[]
        );

BSADLLSPEC BSA_STATUS BsaDefineScanMode(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_SCAN_MODE     ScanMode,
        BSA_UINT32       *ChannelCount,
        BSA_SCAN_CHANNEL  Channels[]
        );

BSADLLSPEC BSA_STATUS BsaBsp2500GetAgcRegs(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_UINT32        ChannelIndex,
        BSA_UINT32       *agc_ctrl_reg,
        BSA_UINT32       *agc_le_cfg_reg,
        BSA_UINT32       *agc_setpoint_reg,
        BSA_UINT32       *agc_gains_reg,
        BSA_UINT32       *agc_rf_reg,
        BSA_UINT32       *agc_gain_config_reg
        );

BSADLLSPEC BSA_STATUS BsaBsp2500SetAgcRegs(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_UINT32        ChannelIndex,
        BSA_UINT32        agc_ctrl_reg,
        BSA_UINT32        agc_le_cfg_reg,
        BSA_UINT32        agc_setpoint_reg,
        BSA_UINT32        agc_gains_reg,
        BSA_UINT32        agc_rf_reg,
        BSA_UINT32        agc_gain_config_reg
        );

BSADLLSPEC BSA_STATUS BsaEnableInterferenceClassification(BSA_DEVICE_HANDLE DevHandle);

BSADLLSPEC BSA_STATUS BsaDisableInterferenceClassification(BSA_DEVICE_HANDLE DevHandle);

BSADLLSPEC BSA_STATUS BsaGetInterferenceClassificationStatus(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_BOOL         *Enable
        );

BSADLLSPEC BSA_STATUS BsaUsbSelectAntenna(BSA_DEVICE_HANDLE DevHandle,
                                          BSA_USB_ANTENNA   Antenna);
BSADLLSPEC BSA_STATUS BsaUsbGetSelectedAntenna(BSA_DEVICE_HANDLE DevHandle,
                                               BSA_USB_ANTENNA  *Antenna);

BSADLLSPEC BSA_STATUS BsaRegisterEventHandler(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_EVENT_ID      EventId,
        BSA_EVENT_HANDLER EventHandler
        );

BSADLLSPEC BSA_STATUS BsaUnregisterEventHandler(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_EVENT_ID      EventId
        );

BSADLLSPEC BSA_STATUS BsaGetEventStatus(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_EVENT_ID      EventId,
        BSA_BOOL         *Started
        );

BSADLLSPEC BSA_STATUS BsaLockConfiguration(BSA_DEVICE_HANDLE DevHandle);

BSADLLSPEC BSA_STATUS BsaUnlockConfiguration(BSA_DEVICE_HANDLE DevHandle);

BSADLLSPEC BSA_STATUS BsaGetVersion(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_VERSION_IE   *Version
        );

BSADLLSPEC BSA_STATUS BsaGetInterferenceLog(BSA_DEVICE_HANDLE           DevHandle,
                                            BSA_INTERFERENCE_EVENT_LOG *InterferenceLog);

BSADLLSPEC BSA_STATUS BsaLoadFirmware(BSA_DEVICE_HANDLE DevHandle,
                                      BSA_UINT32        Flags,
                                      BSA_UINT32        NumSegments,
                                      BSA_FW_SEG        FirmwareSegment[]);

#ifdef __AICS__
/* AICS API functions */
BSADLLSPEC BSA_STATUS BsaAicsGetConfig(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_UINT32        WlanDevId,
        BSA_AICS_CONFIG  *AicsConfig
        );

BSADLLSPEC BSA_STATUS BsaAicsSetConfig(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_UINT32        WlanDevId,
        BSA_AICS_CONFIG   AicsConfig
        );

BSADLLSPEC BSA_STATUS BsaAicsSetCurrentChannel(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_UINT32        WlanDevId,
        BSA_AICS_CHANNEL  CurrentChan
        );

BSADLLSPEC BSA_STATUS BsaAicsGetChannelList(
        BSA_DEVICE_HANDLE   DevHandle,
        BSA_UINT32          WlanDevId,
        BSA_UINT32         *NumChannels,
        BSA_AICS_CHANNEL_IE ChannelList[]
        );

BSADLLSPEC BSA_STATUS BsaAicsSetChannelList(
        BSA_DEVICE_HANDLE   DevHandle,
        BSA_UINT32          WlanDevId,
        BSA_UINT32          NumChannels,
        BSA_AICS_CHANNEL_IE ChannelList[]
        );

BSADLLSPEC BSA_STATUS BsaAicsGetChanRanking( BSA_DEVICE_HANDLE      DevHandle,
                                             BSA_UINT32             WlanDevId,
                                             PBSA_AICS_CHAN_RANKING pChanRanking
                                             );
#endif /* __AICS__ */

BSADLLSPEC BSA_STATUS BsaSetAlertConfig(BSA_DEVICE_HANDLE  DevHandle,
                                        BSA_ALERT_TYPE     AlertType,
                                        BSA_ALERT_SUBTYPE  AlertSubtype,
                                        BSA_BOOL           Enabled,
                                        BSA_ALERT_SEVERITY AlertSeverity,
                                        BSA_ALERT_PARAMS   AlertParams);

BSADLLSPEC BSA_STATUS BsaGetAlertConfig(BSA_DEVICE_HANDLE   DevHandle,
                                        BSA_ALERT_TYPE      AlertType,
                                        BSA_ALERT_SUBTYPE   AlertSubtype,
                                        BSA_BOOL           *Enabled,
                                        BSA_ALERT_SEVERITY *AlertSeverity,
                                        BSA_ALERT_PARAMS   *AlertParams);

BSADLLSPEC BSA_STATUS BsaDeviceDiscovery(
        BSA_IP_ADDR               IpAddrLower,
        BSA_IP_ADDR               IpAddrUpper,
        BSA_UINT32                Port,
        BSA_UINT32                DiscoveryTimeSec,
        BSA_DEV_DISCOVERY_HANDLER DevDiscoveryHandler
        );

BSADLLSPEC BSA_STATUS BsaSetAuthKey(BSA_DEVICE_HANDLE DevHandle,
                                    BSA_UINT32        AuthKeyLen,
                                    BSA_UINT8        *AuthKey);

BSADLLSPEC BSA_STATUS BsaSetSpecanOption(BSA_DEVICE_HANDLE       DevHandle,
                                         BSA_SPECAN_OPTION       Option,
                                         BSA_SPECAN_OPTION_DATA *OptionData);

BSADLLSPEC BSA_STATUS BsaGetSpecanOption(BSA_DEVICE_HANDLE       DevHandle,
                                         BSA_SPECAN_OPTION       Option,
                                         BSA_SPECAN_OPTION_DATA *OptionData);
BSADLLSPEC BSA_STATUS BsaGetDebugConfig(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_LOG_TYPE     *ServiceDbgLevel,
        BSA_LOG_TYPE     *SirpDbgLevel,
        BSA_LOG_TYPE     *BsockDbgLevel,
        BSA_LOG_TARGET   *LogType,    /* Debug log type */
        BSA_CHAR         *pLogFileSpec    /* Debug log filespec */
        );

BSADLLSPEC BSA_STATUS BsaSetDebugConfig(
        BSA_DEVICE_HANDLE DevHandle,
        BSA_LOG_TYPE      ServiceDbgLevel,
        BSA_LOG_TYPE      SirpDbgLevel,
        BSA_LOG_TYPE      BsockDbgLevel,
        BSA_LOG_TARGET    LogTarget, /* Debug log target */
        BSA_CHAR         *pLogFileSpec    /* Debug log filespec */
        );

#ifdef __cplusplus
}
#endif // endif
#endif // endif
