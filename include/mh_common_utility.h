/*
* ug-mobile-ap
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.tizenopensource.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef __APP_MH_COMMON_UTILITY_H_
#define __APP_MH_COMMON_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mobile_hotspot.h"
#include "mh_view_main.h"

#define QP_SIGNAL_PROGRESS_ON	"progress_on"
#define QP_SIGNAL_PROGRESS_OFF	"progress_off"
#define QP_SIGNAL_PROGRESS_RESET	"progress_reset"

/* Genlist new style for Tizen 2.4 */
#define MH_GENLIST_1LINE_TEXT_STYLE "type1"
#define MH_GENLIST_1LINE_TEXT_ICON_STYLE "type1"
#define MH_GENLIST_2LINE_TOP_TEXT_STYLE "type1"
#define MH_GENLIST_2LINE_TOP_TEXT_ICON_STYLE "type1"
#define MH_GENLIST_2LINE_BOTTOM_TEXT_STYLE "type2"
#define MH_GENLIST_2LINE_BOTTOM_TEXT_ICON_STYLE "type2"
#define MH_GENLIST_MULTILINE_TEXT_STYLE "multiline"
#define MH_GENLIST_GROUP_INDEX_STYLE "group_index"

Evas_Object *_create_progressbar(Evas_Object *parent, const char *style);
Evas_Object *_create_bg(Evas_Object *parent, const char *style);
Evas_Object *_create_win_layout(mh_appdata_t *ad);
Evas_Object *_create_naviframe(Evas_Object *parent);
Evas_Object *_create_button(Evas_Object *parent, const char *text, const char *part,
		Evas_Smart_Cb func, void *user_data);
void _handle_usb_status_change(keynode_t *key, void *data);
int _get_vconf_hotspot_mode(void);
Eina_Bool _get_no_of_connected_device(mh_appdata_t *ad, int *no, tethering_type_e type);

void _append_list_client_handle(mh_appdata_t *ad, tethering_client_h client);
void _release_list_client_handle(mh_appdata_t *ad);
void _delete_list_client_handle(mh_appdata_t *ad, const char *mac_addr);
int _get_list_clients_count(mh_appdata_t *ad);
void _get_list_clients_informations(mh_appdata_t *ad);
void _free_genlist_item(Elm_Object_Item **item);
void _free_genlist_itc(Elm_Genlist_Item_Class **itc);
int _get_sim_state(void);
connection_cellular_state_e _get_cellular_state(void);
int _get_checkbox_status(tethering_type_e type);
bool _set_checkbox_status(tethering_type_e type, int value);
int _send_signal_qp(const char *cmd);
void _device_name_changed_cb(keynode_t *key, void *data);
bool _set_vconf_prev_wifi_state(bool value);
bool _get_vconf_prev_wifi_state();
void _handle_network_cellular_state_changed_cb(keynode_t *key, void *data);
void _handle_mobileap_syspopup_popup_response(keynode_t *key, void *data);

#ifdef __cplusplus
}
#endif
#endif
