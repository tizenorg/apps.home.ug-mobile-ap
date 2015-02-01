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

#ifdef TETHERING_DATA_USAGE_SUPPORT
void _data_usage_cb(tethering_error_e result, unsigned long long received_data, unsigned long long sent_data, void *user_data);
#endif

void _visibility_changed_cb(bool is_visible, void *user_data);
void _security_type_changed_cb(tethering_wifi_security_type_e changed_type, void *user_data);
void _passphrase_changed_cb(void *user_data);
int _handle_wifi_onoff_change(mh_appdata_t *ad);
int _handle_bt_onoff_change(mh_appdata_t *ad);
int _handle_usb_onoff_change(mh_appdata_t *ad);
int _turn_on_wifi(void);
bool _is_wifi_direct_on(void);
void _wifi_state_changed_cb(wifi_device_state_e state, void *user_data);
void _wifi_tethering_checkbox_popup_status_set(bool value);
bool _wifi_tethering_checkbox_popup_status_get(void);
gboolean _ps_recheck_timeout_cb(gpointer data);
int _create_wifi_hotspot_on_popup(mh_appdata_t *ad);
int _create_bt_tethering_on_popup(mh_appdata_t *ad);
int _create_usb_tethering_on_popup(mh_appdata_t *ad);
void _update_tethering_item(mh_appdata_t * ad, mh_state_e state);
void _update_tethering_enabling_item(mh_appdata_t *ad, tethering_type_e type, mh_state_e state);

#ifdef __cplusplus
}
#endif
#endif
