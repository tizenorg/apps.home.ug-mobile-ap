/*
* ug-mobile-ap
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://floralicense.org/license/

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

#define MH_UPDATE_INTERVAL	5	/* 5 Sec */
#define MH_KB			(1000)
#define MH_MB			(MH_KB * MH_KB)

void ap_update_data_onoff(void *data);
void ap_update_data_device(mh_appdata_t *ad);
Eina_Bool ap_update_data_packet_usage(mh_appdata_t *ad);
void _start_update_data_packet_usage(mh_appdata_t *ad);
void _stop_update_data_packet_usage(mh_appdata_t *ad);
void ap_draw_contents(mh_appdata_t *ad);
void ap_callback_del(mh_appdata_t *ad);
void _update_wifi_item(mh_appdata_t *ad, int wifi_state);
void _update_bt_item(mh_appdata_t *ad, int bt_state);
void _update_usb_item(mh_appdata_t *ad, int usb_state);
void _update_main_view(mh_appdata_t *ad);

#ifdef __cplusplus
}
#endif
#endif
