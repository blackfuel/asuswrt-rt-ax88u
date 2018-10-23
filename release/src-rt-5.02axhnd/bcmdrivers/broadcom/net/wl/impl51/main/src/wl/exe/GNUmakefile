
# GNUmakefile for wl/exe
#
#  Copyright 2018 Broadcom
#
#  This program is the proprietary software of Broadcom and/or
#  its licensors, and may only be used, duplicated, modified or distributed
#  pursuant to the terms and conditions of a separate, written license
#  agreement executed between you and Broadcom (an "Authorized License").
#  Except as set forth in an Authorized License, Broadcom grants no license
#  (express or implied), right to use, or waiver of any kind with respect to
#  the Software, and Broadcom expressly reserves all rights in and to the
#  Software and all intellectual property rights therein.  IF YOU HAVE NO
#  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
#  WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
#  THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1. This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use
#  all reasonable efforts to protect the confidentiality thereof, and to
#  use this information only in connection with your use of Broadcom
#  integrated circuit products.
#
#  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
#  REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
#  OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
#  DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
#  NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
#  ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
#  CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
#  OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
#  BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
#  SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
#  IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
#  IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
#  ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
#  OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
#  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#
#  <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id: GNUmakefile 766220 2018-07-27 05:40:29Z $

# Work out the directory of this file.
MWD := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

WLAN_ComponentsInUse := awd bcmwifi ppr math bcmcrypto
include $(MWD)/../../makefiles/WLAN_Common.mk

SRCBASE := $(WLAN_SrcBaseR)

ifeq ($(WLAN_CYGWIN_HOST),1)

#-----------------------------------------------------------------
# Windows build, don't include Makerules due to all: conflict

include ../../GNUmakefile.inc

# MfgTest XP and WIN7 WLU DLL builds are built from other standalone Makefiles
# from App and MfgTest teams
# To build XP WLU DLL (output is in windows/winxp/obj/*/mfg_dll/{free,checked}/...)
#    $(MAKE) -C src/wl/exe -f GNUmakefile.wlu_dll
# To build Win7/Vista WLU DLL (output is in windows/win7/obj/*/mfg_dll/{Release,Debug}/...)
#    $(MAKE) -C src/wl/exe/win7

else # WLAN_CYGWIN_HOST

#-----------------------------------------------------------------
# Linux build
#

# This should be one of values recognized in src/Makerules
# 2) not windows, need to include first to pick up TARGETENV dependent vars
include $(SRCBASE)/Makerules

# GNU make function to do compilation and dependency generation
# in one step.
define CompileAndMakedep
$(strip $1 $(call wlan_cc,-c -MMD -MF $(@D)/.$(@F).depend -MP $2))
endef

CFLAGS += $(WLAN_StdIncPathR) $(WLAN_ComponentIncPathR)

ifeq ($(CONFIG_WLEXE),y)
ifneq ($(CONFIG_STBAP),y)
  CFLAGS += -fno-strict-aliasing -O2 -s
else
  CFLAGS += -fno-strict-aliasing
endif # CONFIG_STBAP
endif

# Use newer 11ac ratespec for wl command line
CFLAGS += -DD11AC_IOTYPES

# Use opaque PPR structures
CFLAGS += -DPPR_API

#ifdef SR_DEBUG
CFLAGS += -DSR_DEBUG
#endif // endif

#ifdef WLCNT
  CFLAGS += -DWLCNT
#endif // endif

#ifdef WIFI_ACT_FRAME
  CFLAGS += -DWIFI_ACT_FRAME
#endif // endif

ifeq ($(WLTEST),1)
    CFLAGS += -DSERDOWNLOAD
    CFLAGS += -DWLTEST
endif

ifeq ($(PCIE_MFGTEST),1)
    CFLAGS += -DPCIE_MFGTEST
endif

ifeq ($(OLYMPIC_RWL),1)
    CFLAGS += -DOLYMPIC_RWL
endif

ifneq ($(findstring $(TARGETENV), "linux linuxmips linuxmips_be linuxarm linuxarm_le android android_ndk_r6b"),)

#ifdef WLP2P
  CFLAGS += -DWLP2P
#endif // endif

#ifdef WLMCHAN
  CFLAGS += -DWLMCHAN
#endif // endif

#ifdef WLTDLS
 CFLAGS += -DWLTDLS
#endif // endif

#ifdef WLNDOE
  CFLAGS += -DWLNDOE
#endif // endif

#ifdef WLP2PO
  CFLAGS += -DWLP2PO
#endif // endif

#ifdef WLANQPO
  CFLAGS += -DWLANQPO
#endif // endif

#ifdef WLBDO
  CFLAGS += -DWLBDO
#endif // endif

#ifdef WLTKO
  CFLAGS += -DWLTKO
#endif // endif

#ifdef WL_PROXDETECT
  CFLAGS += -DWL_PROXDETECT
#endif // endif

#ifdef WL_RANDMAC
  CFLAGS += -DWL_RANDMAC
#endif // endif

#ifdef WL_MSCH
  CFLAGS += -DWL_MSCH
#endif // endif

#ifdef WL11ULB
  CFLAGS += -DWL11ULB
#endif // endif

# netlink driver interface
ifeq ($(NL80211),1)
  CFLAGS += -DNL80211
endif

#ifeq ($(tbow),1)
  CFLAGS += -DBT_WIFI_HANDOVER
#endif // endif

#ifdef WLWNM
 CFLAGS += -DWLWNM
#endif // endif

#if defined(WLBSSLOAD_REPORT)
 CFLAGS += -DWLBSSLOAD_REPORT
#endif // endif

#ifdef WLRSDB
CFLAGS += -DWLRSDB
#endif // endif

#ifdef WLSLOTTED_BSS
CFLAGS += -DWLSLOTTED_BSS
#endif // endif

#ifdef WL_BTCDYN
 CFLAGS += -DWL_BTCDYN
#endif // endif

#ifdef WL_MPF
CFLAGS += -DWL_MPF
#endif // endif

#ifdef WLRCC
CFLAGS += -DWLRCC
#endif // endif
#ifdef WL_MBO
CFLAGS += -DWL_MBO
#endif // endif
#ifdef WL_MBO_WFA_CERT
CFLAGS += -DWL_MBO_WFA_CERT
#endif // endif

#ifdef WL_OCE
CFLAGS += -DWL_OCE
#endif // endif

#ifdef WL_ESP
CFLAGS += -DWL_ESP
#endif // endif

#ifdef ECOUNTERS
  CFLAGS += -DECOUNTERS
#endif // endif

#ifdef WL_IFSTATS
  CFLAGS += -DWL_IFSTATS
#endif // endif

#ifdef WLADPS
CFLAGS += -DWLADPS
#endif // endif

#ifdef WL_LEAKY_AP_STATS
  CFLAGS += -DWL_LEAKY_AP_STATS
#endif // endif

#ifdef WL_RPSNOA
CFLAGS += -DWL_RPSNOA
#endif // endif

#ifdef WL_PWROPT
  CFLAGS += -DWL_PWROPT
#endif // endif

#ifdef WL_BTL
CFLAGS += -DWL_BTL
#endif // endif

#ifdef WL_TVPM
  CFLAGS += -DWL_TVPM
#endif // endif

#ifdef WLBAM
CFLAGS += -DWLBAM
#endif // endif

#ifdef WL_TDMTX
  CFLAGS += -DWL_TDMTX
#endif // endif

ifeq ($(CONFIG_EAP),y)
  CFLAGS += -DWL_EAP_STA_SCB_TIMEOUT
  CFLAGS += -DWL_EAP_PER_VAP_CONFIG
  CFLAGS += -DWL_EAP_PER_VAP_AMSDU_HWDAGG_DIS
  CFLAGS += -DWL_EAP_EMSGLVL
  CFLAGS += -DWL_EAP_SCAN_TX
  CFLAGS += -DWL_EAP_TPDUMP
  CFLAGS += -DWL_EAP_KEY_CACHE
  CFLAGS += -DWL_EAP_CUSTOM_SCAN
  CFLAGS += -DWL_EAP_AP
  CFLAGS += -DWL_EAP_SCAN_BEACON_DELAY
  CFLAGS += -DWL_EAP_SCAN_TEST
  CFLAGS += -DWL_EAP_80211RAW
  CFLAGS += -DWL_EAP_PER_VAP_DTIM
  CFLAGS += -DWL_EAP_CUST_EVENT_HNDLR
  CFLAGS += -DWL_EAP_MONITOR
  CFLAGS += -DWL_EAP_STATS -DWLPROPRIETARY_11N_RATES
  CFLAGS += -DWL_EAP_DROP_RX_MGMT_RSSI
  CFLAGS += -DWL_EAP_PER_VAP_CONFIG_RATESET
  CFLAGS += -DWL_EAP_SNR
endif

ifeq ($(CONFIG_AIRIQ),y)
  CFLAGS += -DWL_AIR_IQ
endif

# extra warnings
ifneq ($(filter mips_be arm_le arm arm64 arm_android arm_android_ndk_r6b arm64_android,$(TARGETARCH)),)
CFLAGS += -Wextra -Wall
else
CFLAGS += -Wextra -Wall -Werror
endif
CFLAGS += -DWLPFN -DWLPFN_AUTO_CONNECT
CFLAGS += -DLINUX
# These were added for router support. A better solution should be possible.
ifneq (,$(wildcard $(SRCBASE)/../../main/components/math/include))
  CFLAGS += -I$(SRCBASE)/../../main/components/math/include
endif
ifneq (,$(wildcard $(SRCBASE)/../../main/components/bcmcrypto/include))
  CFLAGS += -I$(SRCBASE)/../../main/components/bcmcrypto/include
endif

# Discard any "MMX" or other qualifications on x86 so that
# any TARGETARCH containing x86 is just "x86"
ifeq ($(findstring x86_mmx,$(TARGETARCH)),x86_mmx)
	TARGETARCH = x86
endif

# ASD is obsolete but the _noasd extension is preserved for compatibility.
NOASDDIR    = _noasd
RWL        ?= 1
ifeq ($(RWL),0)
  override RWL:=
endif

RWL_DONGLE        ?= 1
RWL_SOCKET        ?= 1
RWL_WIFI          ?= 1
RWL_SERIAL        ?= 1

ifneq ($(RWL_DONGLE),0)
RWL_SERVERS += $(SERVER_DONGLE)
endif
ifneq ($(RWL_SOCKET),0)
RWL_SERVERS += $(SERVER_SOCKET)
endif
ifneq ($(RWL_WIFI),0)
RWL_SERVERS += $(SERVER_WIFI)
endif
ifneq ($(RWL_SERIAL),0)
RWL_SERVERS += $(SERVER_SERIAL)
endif

# $(TARGETARCH) is set based on TARGETENV in src/Makerules.* files
WL_OBJS      := wlu.o wlu_common.o wlu_linux.o bcmutils.o bcm_math.o bcmwifi_channels.o wlu_cmd.o wlu_iov.o wlu_client_shared.o wlu_pipe_linux.o wlu_pipe.o miniopt.o bcm_app_utils.o wlu_rates_matrix.o wlc_ppr.o bcmxtlv.o bcmbloom.o sha2.o sha2x.o md5.o wlu_subcounters.o
SERVER_OBJS  := wlu_server_linux.o wlu_pipe_linux.o wlu_pipe.o wlu_server_shared.o shellproc_linux.o
WLM_OBJS     := $(WL_OBJS) wlm.o

ifneq ($(wildcard ../../../components/hml),)
  CFLAGS += -DHOFFLOAD_MODULES
  CFLAGS += -I../../../components/hml/include -I../../pciedev
  WL_OBJS += wluc_hoffload.o
endif

WL_OBJS += wluc_phy.o
WL_OBJS += wluc_wnm.o
WL_OBJS += wluc_cac.o
WL_OBJS += wluc_relmcast.o
WL_OBJS += wluc_rrm.o
WL_OBJS += wluc_wowl.o
WL_OBJS += wluc_pkt_filter.o
WL_OBJS += wluc_mfp.o
WL_OBJS += wluc_ota_test.o
WL_OBJS += wluc_bssload.o
WL_OBJS += wluc_stf.o
WL_OBJS += wluc_offloads.o
WL_OBJS += wluc_tpc.o
WL_OBJS += wluc_toe.o
WL_OBJS += wluc_arpoe.o
WL_OBJS += wluc_keep_alive.o
WL_OBJS += wluc_ap.o
WL_OBJS += wluc_ampdu.o
WL_OBJS += wluc_ampdu_cmn.o
WL_OBJS += wluc_bmac.o
WL_OBJS += wluc_ht.o
WL_OBJS += wluc_wds.o
WL_OBJS += wluc_keymgmt.o
WL_OBJS += wluc_scan.o
WL_OBJS += wluc_obss.o
WL_OBJS += wluc_prot_obss.o
WL_OBJS += wluc_lq.o
WL_OBJS += wluc_seq_cmds.o
WL_OBJS += wluc_btcx.o
WL_OBJS += wluc_led.o
WL_OBJS += wluc_interfere.o
WL_OBJS += wluc_ltecx.o
WL_OBJS += wlu_avail_utils.o
WL_OBJS += wluc_otp.o
WL_OBJS += bcm_math.o
WL_OBJS += wluc_mbo.o

ifneq ($(findstring WL_BTCDYN, $(CFLAGS)),)
WL_OBJS += wluc_btcdyn.o
endif

ifneq ($(findstring WL_NAN, $(CFLAGS)),)
WL_OBJS += wluc_nan.o
endif

ifneq ($(findstring WLRSDB, $(CFLAGS)),)
WL_OBJS += wluc_rsdb.o
endif

ifneq ($(findstring WLSLOTTED_BSS, $(CFLAGS)),)
WL_OBJS += wluc_slotted_bss.o
endif

ifneq ($(findstring BCMSDIO, $(CFLAGS)),)
WL_OBJS += wluc_sdio.o
endif

ifneq ($(findstring WLNDOE, $(CFLAGS)),)
WL_OBJS += wluc_ndoe.o
endif

ifneq ($(filter -DWLP2PO, $(CFLAGS)),)
WL_OBJS += wluc_p2po.o
endif

ifneq ($(findstring WLANQPO, $(CFLAGS)),)
WL_OBJS += wluc_anqpo.o
endif

ifneq ($(findstring WLBDO, $(CFLAGS)),)
WL_OBJS += wluc_bdo.o
endif

ifneq ($(findstring WLTKO, $(CFLAGS)),)
WL_OBJS += wluc_tko.o
endif

ifneq ($(findstring WLPFN, $(CFLAGS)),)
WL_OBJS += wluc_pfn.o
endif

ifneq ($(findstring BT_WIFI_HANDOVER, $(CFLAGS)),)
WL_OBJS += wluc_tbow.o
endif

ifneq ($(filter -DWLP2P, $(CFLAGS)),)
WL_OBJS += wluc_p2p.o
endif

ifneq ($(findstring WLTDLS, $(CFLAGS)),)
WL_OBJS += wluc_tdls.o
endif

ifneq ($(findstring WL_PROXDETECT, $(CFLAGS)),)
WL_OBJS += wluc_proxd.o
endif

ifneq ($(findstring WL_RANDMAC, $(CFLAGS)),)
WL_OBJS += wluc_randmac.o
endif

ifneq ($(findstring WLMESH, $(CFLAGS)),)
WL_OBJS += wluc_mesh.o
endif

ifneq ($(findstring WL_NATOE, $(CFLAGS)),)
WL_OBJS += wluc_natoe.o
endif

ifneq ($(findstring WL_MSCH, $(CFLAGS)),)
WL_OBJS += wluc_msch.o
endif

WL_OBJS += wluc_he.o
WL_OBJS += wluc_twt.o
WL_OBJS += wluc_heb.o

ifneq ($(findstring WL_MBO, $(CFLAGS)),)
WL_OBJS += wluc_mbo.o
endif

ifneq ($(findstring WL_OCE, $(CFLAGS)),)
WL_OBJS += wluc_oce.o
endif

ifneq ($(findstring WL_ESP, $(CFLAGS)),)
WL_OBJS += wluc_esp.o
endif

ifneq ($(findstring ECOUNTERS, $(CFLAGS)),)
WL_OBJS += wluc_ecounters.o
endif

ifneq ($(findstring WLADPS, $(CFLAGS)),)
WL_OBJS += wluc_adps.o
endif

ifneq ($(findstring WL_LEAKY_AP_STATS, $(CFLAGS)),)
WL_OBJS += wluc_leakyapstats.o
endif

ifneq ($(findstring WL_RPSNOA, $(CFLAGS)),)
WL_OBJS += wluc_rpsnoa.o
endif

ifneq ($(findstring WL_PWROPT, $(CFLAGS)),)
WL_OBJS += wluc_pwropt.o
endif

ifneq ($(findstring WL_BTL, $(CFLAGS)),)
WL_OBJS += wluc_btl.o
endif

ifneq ($(findstring WL_TVPM, $(CFLAGS)),)
WL_OBJS += wluc_tvpm.o
endif

ifneq ($(findstring WLBAM, $(CFLAGS)),)
WL_OBJS += wluc_bad_ap_manager.o
endif

#TODO add build option check
ifneq ($(findstring WL_TDMTX, $(CFLAGS)),)
WL_OBJS += wluc_tdmtx.o
endif

# include build settings for nl80211 support
ifeq ($(NL80211),1)
include netlink.inc
endif

# Prefix obj/<type>/TARGETARCH to produced .obj files
WL_OBJS      := $(WL_OBJS:%.o=obj/wl$(NOASDDIR)/$(TARGETARCH)/%.o)
DONGLE_OBJS  := $(SERVER_OBJS:%.o=obj/dongle$(NOASDDIR)/$(TARGETARCH)/%.o)
SOCKET_OBJS  := $(SERVER_OBJS:%.o=obj/socket$(NOASDDIR)/$(TARGETARCH)/%.o)
WIFI_OBJS    := $(SERVER_OBJS:%.o=obj/wifi$(NOASDDIR)/$(TARGETARCH)/%.o)
SERIAL_OBJS  := $(SERVER_OBJS:%.o=obj/serial$(NOASDDIR)/$(TARGETARCH)/%.o)
WLM_OBJS     := $(WLM_OBJS:%.o=obj/wlm/$(TARGETARCH)/%.o)

# Derive a final list of all known objects.
OBJS         := $(WL_OBJS) $(DONGLE_OBJS) $(SOCKET_OBJS) $(WIFI_OBJS) $(SERIAL_OBJS) $(WLM_OBJS)

# Make a list of dependency files, one per object file.
DEPENDS      := $(join $(dir $(OBJS)),$(addprefix .,$(addsuffix .depend,$(notdir $(OBJS)))))

# Include any dependency files which currently exist.
-include $(DEPENDS)

# TODO: Move final built objects to respective TARGETARCH dirs as well
# Final exe names
ifneq ($(TARGETARCH),x86)
	ARCH_SFX   = $(TARGETARCH)
endif
WL_EXE       := wl$(ARCH_SFX)
SERVER_SOCKET = socket$(NOASDDIR)/$(TARGETARCH)/wl_server_socket$(ARCH_SFX)
SERVER_DONGLE = dongle$(NOASDDIR)/$(TARGETARCH)/wl_server_dongle$(ARCH_SFX)
SERVER_WIFI   = wifi$(NOASDDIR)/$(TARGETARCH)/wl_server_wifi$(ARCH_SFX)
SERVER_SERIAL = serial$(NOASDDIR)/$(TARGETARCH)/wl_server_serial$(ARCH_SFX)
WLM_SO       := wlm/$(TARGETARCH)/wlm$(ARCH_SFX).so

# excluding lbrt inclusion for android build
ifeq (,$(findstring android,$(TARGETARCH)))
LDFLAGS      += -lrt
endif

INSTALL_DIR ?= apps

vpath %.c $(WLAN_StdSrcDirsR) $(WLAN_ComponentSrcDirsR)
vpath %.c $(SRCBASE)/../../main/components/math/src
vpath %.c $(SRCBASE)/../../main/components/bcmcrypto/src

all:  build_deps

# Build servers for all but mips_be
ifeq ($(filter mips_be,$(TARGETARCH)),)
all:  $(WL_EXE) servers
else
all: $(WL_EXE)
endif

.PHONY: build_deps
build_deps:

wlm: $(WLM_SO)

servers: $(RWL_SERVERS)

# Handle creation of directories for objects mentioned below.
$(addprefix obj/,$(addsuffix $(NOASDDIR)/$(TARGETARCH),wl dongle socket wifi serial) $(addsuffix /$(TARGETARCH),wlm)):
	@mkdir -pv $@

# Compilation targets
obj/wl$(NOASDDIR)/$(TARGETARCH)/%.o: %.c | obj/wl$(NOASDDIR)/$(TARGETARCH)
ifneq ($(RWL),)
	$(call CompileAndMakedep,$(CC),$(CFLAGS) $(ASD_CFLAGS) -DRWL_SOCKET -DRWL_DONGLE -DRWL_WIFI -DRWL_SERIAL -o $@ $<)
else
	$(call CompileAndMakedep,$(CC),$(CFLAGS) $(ASD_CFLAGS) -o $@ $<)
endif

obj/dongle$(NOASDDIR)/$(TARGETARCH)/%.o: %.c | obj/dongle$(NOASDDIR)/$(TARGETARCH)
	$(call CompileAndMakedep,$(CC),$(CFLAGS) $(ASD_CFLAGS) -DRWL_DONGLE -o $@ $<)

obj/socket$(NOASDDIR)/$(TARGETARCH)/%.o: %.c | obj/socket$(NOASDDIR)/$(TARGETARCH)
	$(call CompileAndMakedep,$(CC),$(CFLAGS) $(ASD_CFLAGS) -DRWL_SOCKET -o $@ $<)

obj/wifi$(NOASDDIR)/$(TARGETARCH)/%.o: %.c | obj/wifi$(NOASDDIR)/$(TARGETARCH)
	$(call CompileAndMakedep,$(CC),$(CFLAGS) $(ASD_CFLAGS) -DRWL_WIFI -o $@ $<)

obj/serial$(NOASDDIR)/$(TARGETARCH)/%.o: %.c | obj/serial$(NOASDDIR)/$(TARGETARCH)
	$(call CompileAndMakedep,$(CC),$(CFLAGS) $(ASD_CFLAGS) -DRWL_SERIAL -o $@ $<)

obj/wlm/$(TARGETARCH)/%.o: %.c | obj/wlm/$(TARGETARCH)
	$(call CompileAndMakedep,$(CC),$(CFLAGS) $(ASD_CFLAGS) -DRWL_DONGLE -DRWL_SOCKET -DRWL_WIFI -DRWL_SERIAL -DWLMSO -fPIC -o $@ $<)

# Final link targets
$(WL_EXE): $(WL_OBJS)
	$(strip $(CC) -o $@ $^ $(LDFLAGS) $(LIBS))
ifneq ($(filter arm_android_ndk_r6b,$(TARGETARCH)),)
ifneq ($(TARGET_PREFIX),)
	$(TARGET_PREFIX)strip $(WL_EXE)
endif
endif

# net_priv target is for building the dhd with NET_ADMIN privilege (i.e. to avoid running as root)
# note: The 'setcap' command is availble from the libcap2-bin package
# note: linux kernel < 2.6.33 doesn't support Security File Capabilities by default
net_priv: $(WL_EXE)
	sudo setcap CAP_NET_ADMIN=eip $^

$(SERVER_DONGLE): $(DONGLE_OBJS) | build_deps
	@mkdir -pv $(@D)
	$(strip $(CC) $(LDFLAGS) -o $@ $^ $(ASD_LDFLAGS))

$(SERVER_SOCKET): $(SOCKET_OBJS) | build_deps
	@mkdir -pv $(@D)
	$(strip $(CC) $(LDFLAGS) -o $@ $^ $(ASD_LDFLAGS))

$(SERVER_WIFI): $(WIFI_OBJS) | build_deps
	@mkdir -pv $(@D)
	$(strip $(CC) $(LDFLAGS) -o $@ $^ $(ASD_LDFLAGS))

$(SERVER_SERIAL): $(SERIAL_OBJS) | build_deps
	@mkdir -pv $(@D)
	$(strip $(CC) $(LDFLAGS) -o $@ $^ $(ASD_LDFLAGS))

$(WLM_SO): $(WLM_OBJS)
	@mkdir -pv $(@D)
	$(strip $(CC) $(LDFLAGS) -shared -o $@ $^)

wldefs: wlmain.o
	$(CC) $(LDFLAGS) -o $@ $<

.PHONY: all clean wlm release_bins net_priv

release_bins:
	@mkdir -p $(INSTALL_DIR)
	install -pv $(WL_EXE) $(INSTALL_DIR)
ifeq ($(filter mips_be,$(TARGETARCH)),)
	install -pv $(SERVER_SOCKET) $(INSTALL_DIR)
	install -pv $(SERVER_DONGLE) $(INSTALL_DIR)
	install -pv $(SERVER_WIFI) $(INSTALL_DIR)
	install -pv $(SERVER_SERIAL) $(INSTALL_DIR)
endif

clean:
	$(RM) -r $(WL_EXE) obj *$(NOASDDIR)

endif # TARGETENV linux

#-----------------------------------------------------------------
# MacOS build
#

ifeq ($(TARGETENV), macos)

ifndef MACOS_VER
  MACOS_VER := $(shell sw_vers -productVersion)
endif

include $(SRCBASE)/Makerules.env

.PHONY: all everything clean wl wl_server_socket wl_server_wifi wl_dongle

PROJTGT	:= wl
BUILD	:= xcodebuild
PROJECT	:= wl.xcodeproj
OBJDIR	:= $(TARGETENV)/$(MACOS_VER)

all: wl wl_server_socket

everything: wl wl_server_socket wl_server_wifi wl_dongle

clean:
	$(RM) -r $(TARGETENV) build

wl wl_server_socket wl_server_wifi: | $(OBJDIR)

#
# dongle wl is located at ./build/Debug/wl
#
wl_dongle:
	$(BUILD) -project $(PROJECT) -target $(PROJTGT) -configuration Debug build

WL_EXE		:= $(OBJDIR)/wl
SERVER_SOCKET	:= $(OBJDIR)/socket/wl_server_socket
SERVER_WIFI	:= $(OBJDIR)/wifi/wl_server_wifi

WL_OBJS := $(addprefix $(OBJDIR)/,wlu.o wlu_common.o wlu_macos.o bcmutils.o bcm_math.o bcmwifi_channels.o wlu_cmd.o wlu_iov.o miniopt.o bcm_app_utils.o wlu_rates_matrix.o wlc_ppr.o bcmxtlv.o wlu_subcounters.o)
SERVER_OBJS := wlu_server_macos.o  wlu_pipe_linux.o wlu_pipe.o wlu_server_shared.o shellproc_linux.o
SOCKET_OBJS := $(SERVER_OBJS:%.o=$(OBJDIR)/socket/%.o)
WIFI_OBJS := $(SERVER_OBJS:%.o=$(OBJDIR)/wifi/%.o)

# Derive a final list of all known objects.
OBJS         := $(WL_OBJS) $(SOCKET_OBJS) $(WIFI_OBJS)

# Create the sub-directories needed to hold object files.
$(sort $(dir $(OBJS))):
	@mkdir -pv $@

# Make a list of dependency files, one per object file.
DEPENDS      := $(join $(dir $(OBJS)),$(addprefix .,$(addsuffix .depend,$(notdir $(OBJS)))))

# Include any dependency files which currently exist.
-include $(DEPENDS)

vpath %.c .. $(WLAN_StdSrcDirsR) $(WLAN_ComponentSrcDirsR)

ifneq (,$(findstring 10.11,$(MACOS_VER)))
        GCDEFS  += -DWLP2P -DWLMCHAN -DPPR_API -DWLANQPO -DWLP2PO -DWL_DUMP_BUF_LEN=819200
else
ifneq (,$(findstring 10.10,$(MACOS_VER)))
        GCDEFS  += -DWLP2P -DWLMCHAN -DPPR_API -DWLANQPO -DWLP2PO -DWL_DUMP_BUF_LEN=819200
else
ifneq (,$(findstring 10.9,$(MACOS_VER)))
        GCDEFS  += -DWLP2P -DWLMCHAN -DPPR_API -DWLANQPO -DWLP2PO
else
        GCDEFS  += -DWLP2P -DWLMCHAN -DPPR_API -DWLANQPO -DWLP2PO -DWL_DUMP_BUF_LEN=819200
endif
endif
endif

#ifdef WLTKO
GCDEFS += -DWLTKO
#endif // endif

#ifdef WLNDOE
GCDEFS += -DWLNDOE
#endif // endif

#ifdef WLBDO
GCDEFS += -DWLBDO
#endif // endif

#ifdef WLRSDB
GCDEFS += -DWLRSDB
#endif // endif

#ifdef WLSLOTTED_BSS
GCDEFS += -DWLSLOTTED_BSS
#endif // endif

WL_OBJS += $(OBJDIR)/wluc_phy.o
WL_OBJS += $(OBJDIR)/wluc_wnm.o
WL_OBJS += $(OBJDIR)/wluc_cac.o
WL_OBJS += $(OBJDIR)/wluc_relmcast.o
WL_OBJS += $(OBJDIR)/wluc_rrm.o
WL_OBJS += $(OBJDIR)/wluc_wowl.o
WL_OBJS += $(OBJDIR)/wluc_pkt_filter.o
WL_OBJS += $(OBJDIR)/wluc_mfp.o
WL_OBJS += $(OBJDIR)/wluc_ota_test.o
WL_OBJS += $(OBJDIR)/wluc_bssload.o
WL_OBJS += $(OBJDIR)/wluc_stf.o
WL_OBJS += $(OBJDIR)/wluc_offloads.o
WL_OBJS += $(OBJDIR)/wluc_tpc.o
WL_OBJS += $(OBJDIR)/wluc_toe.o
WL_OBJS += $(OBJDIR)/wluc_arpoe.o
WL_OBJS += $(OBJDIR)/wluc_keep_alive.o
WL_OBJS += $(OBJDIR)/wluc_ap.o
WL_OBJS += $(OBJDIR)/wluc_ampdu.o
WL_OBJS += $(OBJDIR)/wluc_ampdu_cmn.o
WL_OBJS += $(OBJDIR)/wluc_bmac.o
WL_OBJS += $(OBJDIR)/wluc_ht.o
WL_OBJS += $(OBJDIR)/wluc_wds.o
WL_OBJS += $(OBJDIR)/wluc_keymgmt.o
WL_OBJS += $(OBJDIR)/wluc_scan.o
WL_OBJS += $(OBJDIR)/wluc_obss.o
WL_OBJS += $(OBJDIR)/wluc_prot_obss.o
WL_OBJS += $(OBJDIR)/wluc_lq.o
WL_OBJS += $(OBJDIR)/wluc_seq_cmds.o
WL_OBJS += $(OBJDIR)/wluc_btcx.o
WL_OBJS += $(OBJDIR)/wluc_led.o
WL_OBJS += $(OBJDIR)/wluc_interfere.o
WL_OBJS += $(OBJDIR)/wluc_ltecx.o
WL_OBJS += $(OBJDIR)/wluc_otp.o

ifneq ($(or $(findstring WL_BTCDYN, $(GCDEFS)),\
	    $(findstring WL_BTCDYN, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_btcdyn.o
endif

ifneq ($(findstring WLMESH, $(CFLAGS)),)
WL_OBJS += $(OBJDIR)/wluc_mesh.o
endif

ifneq ($(or $(findstring WL_NAN, $(GCDEFS)),\
	    $(findstring WL_NAN, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_nan.o
endif

ifneq ($(or $(findstring WLRSDB, $(GCDEFS)),\
	    $(findstring WLRSDB, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_rsdb.o
endif

ifneq ($(or $(findstring WLSLOTTED_BSS, $(GCDEFS)),\
	    $(findstring WLSLOTTED_BSS, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_slotted_bss.o
endif

ifneq ($(or $(findstring WL_NATOE, $(GCDEFS)),\
	    $(findstring WL_NATOE, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_natoe.o
endif

ifneq ($(or $(findstring BCMSDIO, $(GCDEFS)),\
	    $(findstring BCMSDIO, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_sdio.o
endif

ifneq ($(or $(findstring WLNDOE, $(GCDEFS)),\
	    $(findstring WLNDOE, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_ndoe.o
endif

ifneq ($(or $(filter -DWLP2PO, $(GCDEFS)),\
	    $(filter -DWLP2PO, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_p2po.o
endif

ifneq ($(or $(findstring WLANQPO, $(GCDEFS)),\
	    $(findstring WLANQPO, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_anqpo.o
endif

ifneq ($(or $(findstring WLBDO, $(GCDEFS)),\
	    $(findstring WLBDO, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_bdo.o
endif

ifneq ($(or $(findstring WLTKO, $(GCDEFS)),\
	    $(findstring WLTKO, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_tko.o
endif
ifneq ($(or $(findstring WLPFN, $(GCDEFS)),\
	    $(findstring WLPFN, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_pfn.o
endif

ifneq ($(or $(filter -DWLP2P, $(GCDEFS)),\
	    $(filter -DWLP2P, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_p2p.o
endif

ifneq ($(or $(findstring WLTDLS, $(GCDEFS)),\
	    $(findstring WLTDLS, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_tdls.o
endif

ifneq ($(or $(findstring WL_PROXDETECT, $(GCDEFS)),\
	    $(findstring WL_PROXDETECT, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_proxd.o
endif

ifneq ($(or $(findstring WL_MSCH, $(GCDEFS)),\
	    $(findstring WL_MSCH, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_msch.o
endif

ifneq ($(or $(findstring ECOUNTERS, $(GCDEFS)),\
	    $(findstring ECOUNTERS, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_ecounters.o
endif

ifneq ($(or $(findstring WLADPS, $(GCDEFS)),\
	    $(findstring WLADPS, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_adps.o
endif

ifneq ($(or $(findstring WL_RPSNOA, $(GCDEFS)),\
	    $(findstring WL_RPSNOA, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_rpsnoa.o
endif

ifneq ($(or $(findstring WL_PWROPT, $(GCDEFS)),\
	    $(findstring WL_PWROPT, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_pwropt.o
endif

ifneq ($(or $(findstring WLBAM, $(GCDEFS)),\
	    $(findstring WLBAM, $(CFLAGS))),)
WL_OBJS += $(OBJDIR)/wluc_bad_ap_manager.o
endif

WL_OBJS += $(OBJDIR)/wluc_he.o
WL_OBJS += $(OBJDIR)/wluc_twt.o
WL_OBJS += $(OBJDIR)/wluc_heb.o

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(call CompileAndMakedep,$(CC),$(CFLAGS) -o $@ $<)

wl: $(OBJDIR)/wl
$(OBJDIR)/wl: $(WL_OBJS)
	$(CC) -L/usr/lib $(LDFLAGS) -o $@ $^

wl_server_socket: $(SERVER_SOCKET)
$(SERVER_SOCKET): $(SOCKET_OBJS)
	$(strip $(CC) $(LDFLAGS) -o $@ $^)

$(OBJDIR)/socket/%.o: %.c | $(OBJDIR)/socket
	$(call CompileAndMakedep,$(CC),$(CFLAGS) -DRWL_SOCKET -DREMOTE_WL -o $@ $<)

wl_server_wifi: $(SERVER_WIFI)
$(SERVER_WIFI): $(WIFI_OBJS)
	$(strip $(CC) $(LDFLAGS) -o $@ $^)

$(OBJDIR)/wifi/%.o: %.c | $(OBJDIR)/wifi
	$(call CompileAndMakedep,$(CC),$(CFLAGS) -DRWL_WIFI -DREMOTE_WL -o $@ $<)

endif # TARGETENV macos

endif # WLAN_CYGWIN_HOST
