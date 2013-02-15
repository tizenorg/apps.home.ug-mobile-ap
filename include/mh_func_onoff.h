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

#ifndef __APP_FUNC_ONOFF_H__
#define __APP_FUNC_ONOFF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mobile_hotspot.h"
#include "mh_view_main.h"
#include "mh_common_utility.h"

int _get_vconf_usb_state(void);
void _enabled_cb(tethering_error_e result, tethering_type_e type, bool is_requested, void *user_data);
void _disabled_cb(tethering_error_e result, tethering_type_e type, tethering_disabled_cause_e cause, void *user_data);
void _connection_changed_cb(tethering_client_h client, bool opened, void *user_data);
void _data_usage_cb(tethering_error_e result, unsigned long long received_data, unsigned long long sent_data, void *user_data);

int _handle_wifi_onoff_change(mh_appdata_t *ad);
int _handle_bt_onoff_change(mh_appdata_t *ad);
int _handle_usb_onoff_change(mh_appdata_t *ad);
int _turn_off_wifi(mh_appdata_t *ad);
int _turn_on_wifi(void);
bool _is_wifi_direct_on(void);
int _turn_off_wifi_direct(mh_appdata_t *ad);

#ifdef __cplusplus
}
#endif
#endif
