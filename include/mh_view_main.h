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

#ifndef __APP_MH_VIEW_MAIN_H__
#define __APP_MH_VIEW_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mobile_hotspot.h"
#include "mh_func_onoff.h"
#include "mh_common_utility.h"
#include "mh_view_wifi_setup.h"

#define PKGNAME "ug-setting-mobileap-efl"

#define MH_UPDATE_INTERVAL	5	/* 5 Sec */
#define MH_KB			(1000LLU)
#define MH_MB			(MH_KB * MH_KB)
#define FILE_PATH_OF_EDC "/usr/apps/ug-setting-mobileap-efl/res/edje/ug-setting-mobileap-efl/tethering.edj"
#define PS_RECHECK_INTERVAL		500
#define PS_RECHECK_COUNT_MAX	5

void ap_update_data_device(mh_appdata_t *ad);

#ifdef TETHERING_DATA_USAGE_SUPPORT
Eina_Bool ap_update_data_packet_usage(mh_appdata_t *ad);
void _start_update_data_packet_usage(mh_appdata_t *ad);
void _stop_update_data_packet_usage(mh_appdata_t *ad);
#endif

void _update_wifi_item(mh_appdata_t *ad, int wifi_state);
void _update_bt_item(mh_appdata_t *ad, int bt_state);
void _update_usb_item(mh_appdata_t *ad, int usb_state);
void _update_main_view(mh_appdata_t *ad, tethering_type_e type);
void _start_update_device_conn_time(mh_appdata_t *ad);
void _stop_update_device_conn_time(mh_appdata_t *ad);
void _main_draw_contents(mh_appdata_t *ad);
void _main_callback_del(mh_appdata_t *ad);
void _main_free_genlist_itc(mh_appdata_t *ad);
#if 0 /* device rename not supported */
void _create_rename_device_popup(void *data);
void _hadnle_rename_popup_rotation(Eina_Bool is_portrait);
void _rotate_adjust_rename_popup(void);
#endif
void _select_connected_dev(void *data, Evas_Object *obj, void *event_info);
void _genlist_update_device_item(mh_appdata_t *ad);
void _update_conn_clients(mh_appdata_t *ad);
void _create_connected_client_view(mh_appdata_t *ad);

#ifdef __cplusplus
}
#endif
#endif
