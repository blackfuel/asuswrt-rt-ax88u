/*
 * Linux Visualization System common XML helper functions header
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
 * $Id: vis_xmlshared.h 606349 2015-12-15 07:15:11Z $
 */

#ifndef _VIS_XML_SHARED_H_
#define _VIS_XML_SHARED_H_

#include <stdio.h>
#include <libxml/tree.h>
#include "vis_struct.h"

extern int get_int_attribute(xmlNodePtr node, char *attribname, int *attribute);

extern int get_long_element(xmlDocPtr doc, xmlNodePtr node, long *element);

extern int get_string_element(xmlDocPtr doc, xmlNodePtr node, char *element, int nelemlen);

extern int get_int_element(xmlDocPtr doc, xmlNodePtr node, int *element);

extern int get_cdata_element(xmlNodePtr node, char *element, int nelemlen);

extern void add_config_node(xmlNodePtr *root_node, configs_t *configs);

extern int get_packet_header(xmlDocPtr doc, xmlNodePtr node, packet_header_t *pktheader);

extern int get_config_from_packet(xmlDocPtr doc, xmlNodePtr node, configs_t *conf);

extern void add_packet_version(xmlNodePtr *root_node, int version);

extern void add_packet_header_node(xmlNodePtr *root_node, packet_header_t *pktheader);

extern void add_dut_info_node(xmlDocPtr *doc, xmlNodePtr *root_node, dut_info_t *dut_info);

extern void add_networks_node(xmlDocPtr *doc, xmlNodePtr *root_node,
	networks_list_t *networks_list);

extern void add_congestion_node(xmlNodePtr *root_node, congestion_list_t *congestion);

extern void add_associated_sta_node(xmlNodePtr *root_node, assoc_sta_list_t *stas_list);

extern void add_counters_node(xmlNodePtr *root_node, counters_t *counters);

extern void add_ampdu_stats_node(xmlNodePtr *root_node, tx_ampdu_list_t *ampdulist);

extern int send_xml_data(int sockfd, xmlDocPtr *doc);

extern void free_xmldata(char **data);

extern int add_graph_header_details(xmlNodePtr *root_node, xmlNodePtr *node,
	iovar_handler_t *handle);

extern iovar_handler_t* find_iovar(const char *name);

extern int add_bargraph_details(xmlNodePtr *root_node, graph_list_t *graphdata, char *iovar_name);

extern int get_dut_settings_tag(xmlDocPtr doc, xmlNodePtr node, dut_settings_t *dutset);

extern void add_dut_settings_node(xmlNodePtr *root_node, dut_settings_t *dutset);

extern void add_rrm_sta_stats_node(xmlNodePtr *root_node, vis_rrm_statslist_t *rrmstatslist);

extern void add_sta_stats_node(xmlNodePtr *root_node, vis_sta_statslist_t *stastatslist);

#endif /* _VIS_XML_SHARED_H_ */
