/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#include "ru.h"

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_RES_CNTRL
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_SLV_RES_CNTRL_REG = 
{
    "UBUS_SLV_RES_CNTRL",
#if RU_INCLUDE_DESC
    "RESPONDER_CTRL Register",
    "Responder side contol. These registers are releated to ubus Responder control",
#endif
    RNR_QUAD_UBUS_SLV_RES_CNTRL_REG_OFFSET,
    0,
    0,
    262,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_VPB_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_SLV_VPB_BASE_REG = 
{
    "UBUS_SLV_VPB_BASE",
#if RU_INCLUDE_DESC
    "VPB_BASE Register",
    "VPB Base address",
#endif
    RNR_QUAD_UBUS_SLV_VPB_BASE_REG_OFFSET,
    0,
    0,
    263,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_VPB_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_SLV_VPB_MASK_REG = 
{
    "UBUS_SLV_VPB_MASK",
#if RU_INCLUDE_DESC
    "VPB_MASK Register",
    "VPB mask address",
#endif
    RNR_QUAD_UBUS_SLV_VPB_MASK_REG_OFFSET,
    0,
    0,
    264,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_APB_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_SLV_APB_BASE_REG = 
{
    "UBUS_SLV_APB_BASE",
#if RU_INCLUDE_DESC
    "APB_BASE Register",
    "APB Base address",
#endif
    RNR_QUAD_UBUS_SLV_APB_BASE_REG_OFFSET,
    0,
    0,
    265,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_APB_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_SLV_APB_MASK_REG = 
{
    "UBUS_SLV_APB_MASK",
#if RU_INCLUDE_DESC
    "APB_MASK Register",
    "APB mask address",
#endif
    RNR_QUAD_UBUS_SLV_APB_MASK_REG_OFFSET,
    0,
    0,
    266,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_DQM_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_SLV_DQM_BASE_REG = 
{
    "UBUS_SLV_DQM_BASE",
#if RU_INCLUDE_DESC
    "DQM_BASE Register",
    "DQM Base address",
#endif
    RNR_QUAD_UBUS_SLV_DQM_BASE_REG_OFFSET,
    0,
    0,
    267,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_SLV_DQM_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_SLV_DQM_MASK_REG = 
{
    "UBUS_SLV_DQM_MASK",
#if RU_INCLUDE_DESC
    "DQM_MASK Register",
    "DQM mask address",
#endif
    RNR_QUAD_UBUS_SLV_DQM_MASK_REG_OFFSET,
    0,
    0,
    268,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG = 
{
    "PARSER_CORE_CONFIGURATION_ENG",
#if RU_INCLUDE_DESC
    "ENG Register",
    "Engineering Configuration reserved for Broadlight use",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG_OFFSET,
    0,
    0,
    269,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG",
#if RU_INCLUDE_DESC
    "PARSER_MISC_CFG Register",
    "Parser Miscellaneous Configuration",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG_OFFSET,
    0,
    0,
    270,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_0_1",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_0_1 Register",
    "Config VID Filter 0 & 1",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG_OFFSET,
    0,
    0,
    271,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_2_3",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_2_3 Register",
    "Config VID Filter 2 & 3",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG_OFFSET,
    0,
    0,
    272,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_4_5",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_4_5 Register",
    "Config VID Filter 4 & 5",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG_OFFSET,
    0,
    0,
    273,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG = 
{
    "PARSER_CORE_CONFIGURATION_VID_6_7",
#if RU_INCLUDE_DESC
    "VID_CONFIGURATION_6_7 Register",
    "Config VID Filter 6 & 7",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG_OFFSET,
    0,
    0,
    274,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER0_CFG Register",
    "Config the IP Address filtering."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[4]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG_OFFSET,
    0,
    0,
    275,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER1_CFG Register",
    "Config the IP Address filtering."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[5]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG_OFFSET,
    0,
    0,
    276,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER0_MASK_CFG Register",
    "Config the IP Address masking."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[4]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG_OFFSET,
    0,
    0,
    277,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTER1_MASK_CFG Register",
    "Config the IP Address masking."
    "Notice that the enable bit is located in the IP_FILTERS_CFG[5]",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG_OFFSET,
    0,
    0,
    278,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG",
#if RU_INCLUDE_DESC
    "IP_FILTERS_CFG Register",
    "IP Address Filters (0..3) configurations:"
    ""
    "(1) SIP or DIP selection config per each filter"
    "(1) Valid bit per each filter",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG_OFFSET,
    0,
    0,
    279,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG = 
{
    "PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE",
#if RU_INCLUDE_DESC
    "SNAP_ORGANIZATION_CODE Register",
    "Identifies SNAP tunneling organization code",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG_OFFSET,
    0,
    0,
    280,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG = 
{
    "PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE",
#if RU_INCLUDE_DESC
    "PPP_IP_PROTOCOL_CODE Register",
    "PPP Protocol Code to indicate L3 is IP",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG_OFFSET,
    0,
    0,
    281,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE",
#if RU_INCLUDE_DESC
    "QTAG_ETHERTYPE Register",
    "Ethertype values to identify the presence of VLAN QTAG",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG_OFFSET,
    0,
    0,
    282,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURTION_0_1 Register",
    "Configures user Ethertype values",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG_OFFSET,
    0,
    0,
    283,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURTION_2_3 Register",
    "Configures user Ethertype values",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG_OFFSET,
    0,
    0,
    284,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG",
#if RU_INCLUDE_DESC
    "USER_ETHERTYPE_CONFIGURATION Register",
    "Configure protocol and enables user Ethertype",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG_OFFSET,
    0,
    0,
    285,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG",
#if RU_INCLUDE_DESC
    "IPV6_HDR_EXT_FLTR_MASK_CFG Register",
    "IPV6 Header Extension Filter Mask register",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG_OFFSET,
    0,
    0,
    286,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_NEST",
#if RU_INCLUDE_DESC
    "QTAG_NESTING Register",
    "Qtag Nesting config",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG_OFFSET,
    0,
    0,
    287,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_0 Register",
    "QTAG Hard Nest Profile 0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG_OFFSET,
    0,
    0,
    288,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_1 Register",
    "QTAG Hard Nest Profile 1",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG_OFFSET,
    0,
    0,
    289,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG = 
{
    "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2",
#if RU_INCLUDE_DESC
    "QTAG_HARD_NEST_PROFILE_2 Register",
    "QTAG Hard Nest Profile 2",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG_OFFSET,
    0,
    0,
    290,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG = 
{
    "PARSER_CORE_CONFIGURATION_USER_IP_PROT",
#if RU_INCLUDE_DESC
    "USER_DEFINED_IP_PROTOCL Register",
    "IP Protocols to be matched to IP Protocol field and to be indicated in the output summary word",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG_OFFSET,
    0,
    0,
    291,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT0_VAL_L Register",
    "Config DA filter 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG_OFFSET,
    0,
    0,
    292,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT0_VAL_H Register",
    "Config DA filter0 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG_OFFSET,
    0,
    0,
    293,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT1_VAL_L Register",
    "Config DA filter1 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG_OFFSET,
    0,
    0,
    294,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT1_VAL_H Register",
    "Config DA filter1 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG_OFFSET,
    0,
    0,
    295,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT2_VAL_L Register",
    "Config DA filter2 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG_OFFSET,
    0,
    0,
    296,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT2_VAL_H Register",
    "Config DA filter2 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG_OFFSET,
    0,
    0,
    297,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT3_VAL_L Register",
    "Config DA filter3 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG_OFFSET,
    0,
    0,
    298,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT3_VAL_H Register",
    "Config DA filter3 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG_OFFSET,
    0,
    0,
    299,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT4_VAL_L Register",
    "Config DA filter4 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG_OFFSET,
    0,
    0,
    300,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT4_VAL_H Register",
    "Config DA Filter4 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG_OFFSET,
    0,
    0,
    301,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT5_VAL_L Register",
    "Config DA filter5 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG_OFFSET,
    0,
    0,
    302,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT5_VAL_H Register",
    "Config DA Filter5 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG_OFFSET,
    0,
    0,
    303,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT6_VAL_L Register",
    "Config DA filter6 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG_OFFSET,
    0,
    0,
    304,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT6_VAL_H Register",
    "Config DA Filter6 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG_OFFSET,
    0,
    0,
    305,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT7_VAL_L Register",
    "Config DA filter7 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG_OFFSET,
    0,
    0,
    306,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT7_VAL_H Register",
    "Config DA Filter7 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG_OFFSET,
    0,
    0,
    307,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L",
#if RU_INCLUDE_DESC
    "DA_FILT8_VAL_L Register",
    "Config DA filter8 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG_OFFSET,
    0,
    0,
    308,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H",
#if RU_INCLUDE_DESC
    "DA_FILT8_VAL_H Register",
    "Config DA Filter8 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG_OFFSET,
    0,
    0,
    309,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT0_MASK_L Register",
    "Config DA Filter mask 15:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG_OFFSET,
    0,
    0,
    310,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H",
#if RU_INCLUDE_DESC
    "DA_FILT0_MASK_H Register",
    "Config DA Filter0 mask 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG_OFFSET,
    0,
    0,
    311,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L",
#if RU_INCLUDE_DESC
    "DA_FILT1_MASK_L Register",
    "Config DA Filter1 mask 31:0",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG_OFFSET,
    0,
    0,
    312,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H",
#if RU_INCLUDE_DESC
    "DA_FILT1_MASK_H Register",
    "Config DA Filter1 mask 47:32",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG_OFFSET,
    0,
    0,
    313,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_0 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG_OFFSET,
    0,
    0,
    314,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_1 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG_OFFSET,
    0,
    0,
    315,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG = 
{
    "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2",
#if RU_INCLUDE_DESC
    "DA_FILT_VALID_CFG_PROFILE_2 Register",
    "Valid configuration of all DA filters: there is a dedicated bit per each DA filter that says if the current DA filter is valid or not. Used for on-the-fly DA filter value (mask) modifications, since the DA filter parameters are not assigned on single SW register.",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG_OFFSET,
    0,
    0,
    316,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG",
#if RU_INCLUDE_DESC
    "GRE_PROTOCOL_CFG Register",
    "GRE Protocol",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG_OFFSET,
    0,
    0,
    317,
};

/******************************************************************************
 * Register: RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG = 
{
    "PARSER_CORE_CONFIGURATION_PROP_TAG_CFG",
#if RU_INCLUDE_DESC
    "PROP_TAG_CFG Register",
    "Prop Tag Configuration",
#endif
    RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG_OFFSET,
    0,
    0,
    318,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG = 
{
    "GENERAL_CONFIG_DMA_ARB_CFG",
#if RU_INCLUDE_DESC
    "DMA_ARB_CFG Register",
    "DMA arbiter Configuration",
#endif
    RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG_OFFSET,
    0,
    0,
    319,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM0_BASE",
#if RU_INCLUDE_DESC
    "PSRAM0_BASE Register",
    "Configure PSRAM0 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG_OFFSET,
    0,
    0,
    320,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM1_BASE",
#if RU_INCLUDE_DESC
    "PSRAM1_BASE Register",
    "Configure PSRAM1 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG_OFFSET,
    0,
    0,
    321,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM2_BASE",
#if RU_INCLUDE_DESC
    "PSRAM2_BASE Register",
    "Configure PSRAM2 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG_OFFSET,
    0,
    0,
    322,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG = 
{
    "GENERAL_CONFIG_PSRAM3_BASE",
#if RU_INCLUDE_DESC
    "PSRAM3_BASE Register",
    "Configure PSRAM3 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG_OFFSET,
    0,
    0,
    323,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR0_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG = 
{
    "GENERAL_CONFIG_DDR0_BASE",
#if RU_INCLUDE_DESC
    "DDR0_BASE Register",
    "Configure DDR0 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG_OFFSET,
    0,
    0,
    324,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR1_BASE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG = 
{
    "GENERAL_CONFIG_DDR1_BASE",
#if RU_INCLUDE_DESC
    "DDR1_BASE Register",
    "Configure DDR1 base"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG_OFFSET,
    0,
    0,
    325,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM0_MASK",
#if RU_INCLUDE_DESC
    "PSRAM0_MASK Register",
    "Configure PSRAM0 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG_OFFSET,
    0,
    0,
    326,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM1_MASK",
#if RU_INCLUDE_DESC
    "PSRAM1_MASK Register",
    "Configure PSRAM1 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG_OFFSET,
    0,
    0,
    327,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM2_MASK",
#if RU_INCLUDE_DESC
    "PSRAM2_MASK Register",
    "Configure PSRAM2 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG_OFFSET,
    0,
    0,
    328,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG = 
{
    "GENERAL_CONFIG_PSRAM3_MASK",
#if RU_INCLUDE_DESC
    "PSRAM3_MASK Register",
    "Configure PSRAM3 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG_OFFSET,
    0,
    0,
    329,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR0_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG = 
{
    "GENERAL_CONFIG_DDR0_MASK",
#if RU_INCLUDE_DESC
    "DDR0_MASK Register",
    "Configure DDR0 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG_OFFSET,
    0,
    0,
    330,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_DDR1_MASK
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG = 
{
    "GENERAL_CONFIG_DDR1_MASK",
#if RU_INCLUDE_DESC
    "DDR1_MASK Register",
    "Configure DDR1 mask"
    "",
#endif
    RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG_OFFSET,
    0,
    0,
    331,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG = 
{
    "GENERAL_CONFIG_PROFILING_CONFIG",
#if RU_INCLUDE_DESC
    "PROFILING_CONFIG Register",
    "Profiling configuration",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG_OFFSET,
    0,
    0,
    332,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_PROFILING_COUNTER
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_PROFILING_COUNTER_REG = 
{
    "GENERAL_CONFIG_PROFILING_COUNTER",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER Register",
    "Profiling counter value read-only",
#endif
    RNR_QUAD_GENERAL_CONFIG_PROFILING_COUNTER_REG_OFFSET,
    0,
    0,
    333,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_0_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_0 Register",
    "Breakpoint 0 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG_OFFSET,
    0,
    0,
    334,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_1_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_1 Register",
    "Breakpoint 1 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG_OFFSET,
    0,
    0,
    335,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_2_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_2 Register",
    "Breakpoint 2 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG_OFFSET,
    0,
    0,
    336,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_3_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_3 Register",
    "Breakpoint 3 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG_OFFSET,
    0,
    0,
    337,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_4_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_4 Register",
    "Breakpoint 4 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG_OFFSET,
    0,
    0,
    338,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_5_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_5 Register",
    "Breakpoint 5 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG_OFFSET,
    0,
    0,
    339,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_6_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_6 Register",
    "Breakpoint 6 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG_OFFSET,
    0,
    0,
    340,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_7_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_7 Register",
    "Breakpoint 7 configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG_OFFSET,
    0,
    0,
    341,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG = 
{
    "GENERAL_CONFIG_BKPT_GEN_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG_GEN Register",
    "Breakpoint general configuration.",
#endif
    RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG_OFFSET,
    0,
    0,
    342,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG = 
{
    "GENERAL_CONFIG_POWERSAVE_CONFIG",
#if RU_INCLUDE_DESC
    "POWERSAVE_CONFIG Register",
    "Powersaving  configuration",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG_OFFSET,
    0,
    0,
    343,
};

/******************************************************************************
 * Register: RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG = 
{
    "GENERAL_CONFIG_POWERSAVE_STATUS",
#if RU_INCLUDE_DESC
    "POWERSAVE_STATUS Register",
    "Powersave status indications",
#endif
    RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG_OFFSET,
    0,
    0,
    344,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_FIFO_CONFIG
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_FIFO_CONFIG_REG = 
{
    "DEBUG_FIFO_CONFIG",
#if RU_INCLUDE_DESC
    "FIFO_CONFIG Register",
    "FIFOs configuration",
#endif
    RNR_QUAD_DEBUG_FIFO_CONFIG_REG_OFFSET,
    0,
    0,
    345,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG = 
{
    "DEBUG_PSRAM_HDR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_STATUS Register",
    "PSRAM Header FIFO status",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    346,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG = 
{
    "DEBUG_PSRAM_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_STATUS Register",
    "PSRAM Data FIFO status",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    347,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG = 
{
    "DEBUG_DDR_HDR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_STATUS Register",
    "DDR Header FIFO status",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    348,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG = 
{
    "DEBUG_DDR_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "DDR_DATA_FIFO_STATUS Register",
    "DDR Data FIFO status",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    349,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG = 
{
    "DEBUG_DDR_DATA_FIFO_STATUS2",
#if RU_INCLUDE_DESC
    "DDR_DATA_FIFO_STATUS2 Register",
    "DDR Data FIFO status 2",
#endif
    RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG_OFFSET,
    0,
    0,
    350,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG = 
{
    "DEBUG_PSRAM_HDR_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_DATA1 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG_OFFSET,
    0,
    0,
    351,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG = 
{
    "DEBUG_PSRAM_HDR_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "PSRAM_HDR_FIFO_DATA2 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG_OFFSET,
    0,
    0,
    352,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG = 
{
    "DEBUG_PSRAM_DATA_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_DATA1 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG_OFFSET,
    0,
    0,
    353,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG = 
{
    "DEBUG_PSRAM_DATA_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "PSRAM_DATA_FIFO_DATA2 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG_OFFSET,
    0,
    0,
    354,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG = 
{
    "DEBUG_DDR_HDR_FIFO_DATA1",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_DATA1 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG_OFFSET,
    0,
    0,
    355,
};

/******************************************************************************
 * Register: RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG = 
{
    "DEBUG_DDR_HDR_FIFO_DATA2",
#if RU_INCLUDE_DESC
    "DDR_HDR_FIFO_DATA2 Register",
    "Read contents of FIFO memory",
#endif
    RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG_OFFSET,
    0,
    0,
    356,
};

/******************************************************************************
 * Register: RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG = 
{
    "EXT_FLOWCTRL_CONFIG_TOKEN_VAL",
#if RU_INCLUDE_DESC
    "TOKEN %i Register",
    "Token value for flow control",
#endif
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG_OFFSET,
    RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG_RAM_CNT,
    4,
    357,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG = 
{
    "UBUS_DECODE_CFG_PSRAM_UBUS_DECODE",
#if RU_INCLUDE_DESC
    "PSRAM_UBUS_DECODE %i Register",
    "Decode for PSRAM Queue",
#endif
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG_OFFSET,
    RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG_RAM_CNT,
    4,
    358,
};

/******************************************************************************
 * Register: RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE
 ******************************************************************************/
const ru_reg_rec RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG = 
{
    "UBUS_DECODE_CFG_DDR_UBUS_DECODE",
#if RU_INCLUDE_DESC
    "DDR_UBUS_DECODE %i Register",
    "Decode for DDR Queue",
#endif
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG_OFFSET,
    RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG_RAM_CNT,
    4,
    359,
};

/******************************************************************************
 * Block: RNR_QUAD
 ******************************************************************************/
static const ru_reg_rec *RNR_QUAD_REGS[] =
{
    &RNR_QUAD_UBUS_SLV_RES_CNTRL_REG,
    &RNR_QUAD_UBUS_SLV_VPB_BASE_REG,
    &RNR_QUAD_UBUS_SLV_VPB_MASK_REG,
    &RNR_QUAD_UBUS_SLV_APB_BASE_REG,
    &RNR_QUAD_UBUS_SLV_APB_MASK_REG,
    &RNR_QUAD_UBUS_SLV_DQM_BASE_REG,
    &RNR_QUAD_UBUS_SLV_DQM_MASK_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_ENG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_0_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_2_3_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_4_5_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_VID_6_7_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_NEST_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_USER_IP_PROT_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_REG,
    &RNR_QUAD_PARSER_CORE_CONFIGURATION_PROP_TAG_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_DMA_ARB_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR0_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR1_BASE_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM0_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM1_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM2_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PSRAM3_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR0_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_DDR1_MASK_REG,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_CONFIG_REG,
    &RNR_QUAD_GENERAL_CONFIG_PROFILING_COUNTER_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_0_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_1_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_2_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_3_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_4_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_5_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_6_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_7_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_BKPT_GEN_CFG_REG,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_CONFIG_REG,
    &RNR_QUAD_GENERAL_CONFIG_POWERSAVE_STATUS_REG,
    &RNR_QUAD_DEBUG_FIFO_CONFIG_REG,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS_REG,
    &RNR_QUAD_DEBUG_DDR_DATA_FIFO_STATUS2_REG,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA1_REG,
    &RNR_QUAD_DEBUG_PSRAM_HDR_FIFO_DATA2_REG,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA1_REG,
    &RNR_QUAD_DEBUG_PSRAM_DATA_FIFO_DATA2_REG,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA1_REG,
    &RNR_QUAD_DEBUG_DDR_HDR_FIFO_DATA2_REG,
    &RNR_QUAD_EXT_FLOWCTRL_CONFIG_TOKEN_VAL_REG,
    &RNR_QUAD_UBUS_DECODE_CFG_PSRAM_UBUS_DECODE_REG,
    &RNR_QUAD_UBUS_DECODE_CFG_DDR_UBUS_DECODE_REG,
};

unsigned long RNR_QUAD_ADDRS[] =
{
    0x82288000,
    0x82388000,
    0x82488000,
    0x82588000,
};

const ru_block_rec RNR_QUAD_BLOCK = 
{
    "RNR_QUAD",
    RNR_QUAD_ADDRS,
    4,
    98,
    RNR_QUAD_REGS
};

/* End of file XRDP_RNR_QUAD.c */
