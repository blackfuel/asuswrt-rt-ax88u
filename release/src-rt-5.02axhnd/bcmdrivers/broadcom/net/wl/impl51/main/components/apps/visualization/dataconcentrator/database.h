/*
 * Visualization system data concentrator database utility header
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
 * $Id: database.h 606349 2015-12-15 07:15:11Z $
 */

#ifndef _VIS_DATABASE_H_
#define _VIS_DATABASE_H_

#include <sqlite3.h>
#include "vis_struct.h"
#include "vis_db_defines.h"
#include "defines.h"

#define MAX_QUERY_SIZE	1024

extern int open_one_connection();

extern int close_one_connection();

extern int open_database(const char *dbname, sqlite3 **db);

extern int close_database(sqlite3 **db);

extern int create_database_and_tables();

extern int create_graphs_table();

extern int get_all_tablenames();

extern int get_config_from_db(configs_t *configs);

extern networks_list_t *get_networks_from_db(int rowid, int band);

extern assoc_sta_list_t *get_associated_sta_from_db(int rowid);

extern int get_ampdus_from_db(tx_ampdu_list_t **ampdulist);

extern int get_glitch_counters_from_db(counters_t *counters);

extern int save_configs_in_db(configs_t *configs, int forceupdate);

extern int store_dut_details(dut_info_t *dutinfo);

extern int is_dut_present(char *mac, int *rowid, int islock, int ischeckenabled);

extern int insert_networks_details(dut_info_t *dutinfo, networks_list_t *networks_list);

extern int insert_empty_networks_details(dut_info_t *dutinfo, networks_list_t *networks_list);

extern int insert_assocsta_details(dut_info_t *dutinfo, assoc_sta_list_t *stas_list);

extern int insert_channel_stats_details(dut_info_t *dutinfo, congestion_list_t *congestion);

extern int insert_ampdu_stats_details(dut_info_t *dutinfo, tx_ampdu_list_t *ampdulist);

extern int insert_graph_details(int sockfd, dut_info_t *dutinfo, graph_list_t *graphs,
	graphdetails_t *graphdet, iovar_handler_t *handle);

extern int get_graph_row(graphdetails_t *graphdet, iovar_handler_t *handle);

extern int get_tab_data_from_db(int sockfd, char *tabname, int rowid, char *mac);

extern int get_graphs_list_from_db();

extern void free_graphs_list();

extern graphnamelist_t* get_json_for_graphs();

extern int remove_rows_on_db_size_exceedes();

extern int update_enable_graphs(onlygraphnamelist_t *graphname);

extern int get_dut_details_from_db(char *mac, dut_info_t *dutdet);

extern congestion_list_t *get_channel_capacity_from_db(int rowid,
	int freqband, int curchannel, int *capacity);

extern channel_maplist_t* get_channel_map(int rowid);

extern tx_ampdu_list_t *get_ampdu_stats_from_db(int rowid, int freqband);

extern void set_max_db_size(int dbsize);

extern int update_gatewayip_in_db(char *gatewayip);

extern int get_alldut_settings_from_db();

extern int update_dut_settings_in_db(dut_settings_t *dutset);

extern void get_current_dut_settings(int rowid, dut_settings_t *dutset);

extern int get_number_of_rows(char *table, char *query);

extern congestion_list_t *get_chanim_from_db(int rowid, int freqband, int curchannel);

extern dut_list_t *get_all_duts_from_db();

extern int delete_all_from_table(char *table);

extern int insert_sta_side_stats_details(dut_info_t *dutinfo, vis_sta_statslist_t *stastatslist);

extern int insert_rrm_adv_sta_side_stats_details(dut_info_t *dutinfo,
	vis_rrm_statslist_t *rrmstatlist);

extern int get_stastats_from_db(int rowid, char *mac, vis_sta_stats_t *stastats);

extern vis_rrm_statslist_t* get_rrm_adv_stastats_from_db(int rowid, char *mac);

extern int get_adv_sta_stats_from_db(int sockfd, char *tabname, int rowid, char *mac,
	vis_rrm_statslist_t *rrmstatslist);

extern void update_sta_graphs_status(onlygraphnamelist_t *graphname);
#endif /* _VIS_DATABASE_H_ */
