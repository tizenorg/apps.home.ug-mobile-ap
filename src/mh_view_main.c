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

#include "mh_view_main.h"

static void _gl_device_item_sel(void *data, Evas_Object *obj, void *event_info);
static void _gl_exp(void *data, Evas_Object *obj, void *event_info);
static void _gl_con(void *data, Evas_Object *obj, void *event_info);

static bool _connected_clients_cb(tethering_client_h client, void *user_data)
{
	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return true;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;
	int i = ad->clients.number++;

	tethering_client_clone(&(ad->clients.handle[i]), client);

	return true;
}

static void __genlist_update_device_subitem(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	Elm_Object_Item *item = NULL;
	tethering_type_e connection_type = 0;
	int i;

	for (i = 0; i < ad->clients.number; i++) {
		tethering_client_get_tethering_type(ad->clients.handle[i], &connection_type);

		item = elm_genlist_item_append(ad->main.genlist,
				ad->main.dev_itc[connection_type],
				(void *)ad->clients.handle[i],
				ad->main.device_item, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		if (item == NULL) {
			ERR("elm_genlist_item_append is failed\n");
			continue;
		}

		elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __genlist_update_device_item(mh_appdata_t *ad, int no_of_dev)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	Elm_Object_Item *item = NULL;
	const Elm_Genlist_Item_Class *itc = NULL;
	Elm_Genlist_Item_Type item_flags = ELM_GENLIST_ITEM_NONE;
	Eina_Bool expanded = EINA_FALSE;

	if (ad->main.device_item != NULL) {
		itc = elm_genlist_item_item_class_get(ad->main.device_item);
		if (itc == NULL) {
			ERR("itc is NULL\n");
			return;
		}

		if ((itc == ad->main.device0_itc && no_of_dev == 0) ||
				(itc == ad->main.device_itc && no_of_dev > 0)) {
			DBG("No need to change genlist item flags\n");
			elm_genlist_item_update(ad->main.device_item);

			expanded = elm_genlist_item_expanded_get(ad->main.device_item);
			if (expanded == EINA_TRUE) {
				elm_genlist_item_subitems_clear(ad->main.device_item);
				__genlist_update_device_subitem(ad);
			}

			return;
		}

		elm_genlist_item_subitems_clear(ad->main.device_item);
		elm_object_item_del(ad->main.device_item);
		ad->main.device_item = NULL;
	}

	if (no_of_dev == 0) {
		itc = ad->main.device0_itc;
		item_flags = ELM_GENLIST_ITEM_NONE;
	} else if (no_of_dev > 0) {
		itc = ad->main.device_itc;
		item_flags = ELM_GENLIST_ITEM_TREE;
	}

	item = elm_genlist_item_insert_before(ad->main.genlist,
			itc, ad, NULL, ad->main.usage_item, item_flags,
			_gl_device_item_sel, NULL);
	if (item == NULL) {
		ERR("elm_genlist_item_insert_before is failed\n");
		return;
	}
	ad->main.device_item = item;

	if (no_of_dev == 0) {
		elm_genlist_item_select_mode_set(item,
				ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		evas_object_smart_callback_del(ad->main.genlist, "expanded",
				_gl_exp);
		evas_object_smart_callback_del(ad->main.genlist, "contracted",
				_gl_con);
	} else if (no_of_dev > 0) {
		evas_object_smart_callback_add(ad->main.genlist, "expanded",
				_gl_exp, ad);
		evas_object_smart_callback_add(ad->main.genlist, "contracted",
				_gl_con, ad);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

void ap_update_data_device(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	int i;

	if (ad->clients.number > 0) {
		for (i = 0; i < ad->clients.number; i++) {
			if (!ad->clients.handle[i])
				continue;

			tethering_client_destroy(ad->clients.handle[i]);
			ad->clients.handle[i] = NULL;
		}
	}
	ad->clients.number = 0;

	tethering_foreach_connected_clients(ad->handle, TETHERING_TYPE_ALL,
			_connected_clients_cb, (void *)ad);
	__genlist_update_device_item(ad, ad->clients.number);

	__MOBILE_AP_FUNC_EXIT__;
}

Eina_Bool ap_update_data_packet_usage(mh_appdata_t *ad)
{
	if (ad == NULL) {
		ERR("Invalid param\n");
		return EINA_FALSE;
	}

	elm_genlist_item_update(ad->main.usage_item);

	return EINA_TRUE;
}

Eina_Bool ap_get_data_statistics(void *data)
{
	if (!data) {
		ERR("The param is NULL\n");
		return ECORE_CALLBACK_CANCEL;
	}
	mh_appdata_t *ad = (mh_appdata_t *)data;

	if (ad->main.hotspot_mode == VCONFKEY_MOBILE_HOTSPOT_MODE_NONE) {
		DBG("Mobile hotspot is turned off.\n");
		ad->update_statistics_handle = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	/* If previous data is not updated, new data is not required */
	if (ad->data_statistics.is_updated == false)
		return ECORE_CALLBACK_RENEW;

	/* Because previous data is updated, new data is required.
	   It will be returned asynchronously. */
	tethering_get_data_usage(ad->handle, _data_usage_cb, (void *)ad);
	ad->data_statistics.is_updated = false;

	return ECORE_CALLBACK_RENEW;
}

void _start_update_data_packet_usage(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	if (ad->update_statistics_handle) {
		DBG("update_statistics_handle is not NULL.\n");
		_stop_update_data_packet_usage(ad);
	}

	ad->data_statistics.is_updated = false;
	tethering_get_data_usage(ad->handle, _data_usage_cb, (void *)ad);
	ad->update_statistics_handle = ecore_timer_add(MH_UPDATE_INTERVAL,
			ap_get_data_statistics, (void *)ad);

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

void _stop_update_data_packet_usage(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	if (ad->update_statistics_handle) {
		ecore_timer_del(ad->update_statistics_handle);
		ad->data_statistics.is_updated = false;
		ad->update_statistics_handle = NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __read_setting(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	int ret = 0;
	char *ssid = NULL;
	char *passphrase = NULL;

	ad->main.hotspot_mode = _get_vconf_hotspot_mode();

	ret = vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_CONNECTED_DEVICE,
			&ad->clients.number);
	if (ret != 0) {
		ERR("vconf_get_int is failed\n");
		ad->clients.number = 0;
	}

	ad->data_statistics.pdp_total_sent = 0;
	ad->data_statistics.pdp_total_receive = 0;
	if (ad->main.hotspot_mode != VCONFKEY_MOBILE_HOTSPOT_MODE_NONE)
		tethering_get_data_usage(ad->handle, _data_usage_cb, (void *)ad);

	ret = tethering_wifi_get_ssid(ad->handle, &ssid);
	if (ret != TETHERING_ERROR_NONE || ssid == NULL) {
		ERR("tethering_wifi_get_ssid is failed : %d\n", ret);
		return;
	}
	g_strlcpy(ad->setup.device_name, ssid, sizeof(ad->setup.device_name));
	free(ssid);

	ret = tethering_wifi_get_passphrase(ad->handle, &passphrase);
	if (ret != TETHERING_ERROR_NONE || passphrase == NULL) {
		ERR("tethering_wifi_get_passphrase is failed : %d\n", ret);
		return;
	}
	g_strlcpy(ad->setup.wifi_passphrase, passphrase,
			sizeof(ad->setup.wifi_passphrase));
	g_strlcpy(ad->setup.wifi_passphrase_new, passphrase,
			sizeof(ad->setup.wifi_passphrase_new));
	free(passphrase);

	ret = tethering_wifi_get_ssid_visibility(ad->handle, &ad->setup.visibility);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_get_ssid_visibility is failed\n");
	}

	ret = tethering_wifi_get_security_type(ad->handle, &ad->setup.security_type);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_get_security_type is failed\n");
	}

	DBG("VCONFKEY_MOBILE_HOTSPOT_MODE : %d\n", ad->main.hotspot_mode);
	DBG("Device name - %s\n", ad->setup.device_name);
	DBG("Wifi passphrase- %s\n", ad->setup.wifi_passphrase);
	DBG("Connected device : %d\n", ad->clients.number);
	DBG("Visibility: %d\n", ad->setup.visibility);
	DBG("Security: %d\n", ad->setup.security_type);
	DBG("End of Load setting value \n");

	__MOBILE_AP_FUNC_EXIT__;
}

void _update_wifi_item(mh_appdata_t *ad, int wifi_state)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->main.wifi_state == wifi_state) {
		DBG("aready updated\n");
		return;
	}

	if (ad->main.wifi_state == MH_STATE_PROCESS) {
		ad->main.wifi_state = MH_STATE_NONE;
		elm_genlist_item_select_mode_set(ad->main.wifi_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
		elm_object_item_disabled_set(ad->main.setup_item, EINA_FALSE);
	} else if (ad->main.wifi_state == MH_STATE_NONE) {
		ad->main.wifi_state = MH_STATE_PROCESS;
		elm_genlist_item_select_mode_set(ad->main.wifi_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		elm_object_item_disabled_set(ad->main.setup_item, EINA_TRUE);
	}

	elm_genlist_item_update(ad->main.wifi_item);
	elm_object_item_signal_emit(ad->main.setup_item, "elm,state,bottom", "");

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

void _update_bt_item(mh_appdata_t *ad, int bt_state)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->main.bt_state == bt_state) {
		DBG("aready updated\n");
		return;
	}

	if (ad->main.bt_state == MH_STATE_PROCESS) {
		ad->main.bt_state = MH_STATE_NONE;
		elm_genlist_item_select_mode_set(ad->main.bt_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
	} else if (ad->main.bt_state == MH_STATE_NONE) {
		ad->main.bt_state = MH_STATE_PROCESS;
		elm_genlist_item_select_mode_set(ad->main.bt_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	elm_genlist_item_update(ad->main.bt_item);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

void _update_usb_item(mh_appdata_t *ad, int usb_state)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->main.usb_state == usb_state) {
		DBG("aready updated\n");
		return;
	}

	if (ad->main.usb_state == MH_STATE_PROCESS) {
		ad->main.usb_state = MH_STATE_NONE;
		elm_genlist_item_select_mode_set(ad->main.usb_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
	} else if (ad->main.usb_state == MH_STATE_NONE) {
		ad->main.usb_state = MH_STATE_PROCESS;
		elm_genlist_item_select_mode_set(ad->main.usb_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	elm_genlist_item_update(ad->main.usb_item);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

void _update_main_view(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	int state = VCONFKEY_MOBILE_HOTSPOT_MODE_NONE;
	Eina_Bool wifi_state = EINA_FALSE;
	Eina_Bool bt_state = EINA_FALSE;
	Eina_Bool usb_state = EINA_FALSE;
	Elm_Object_Item *item = NULL;

	ad->main.hotspot_mode = _get_vconf_hotspot_mode();
	state = ad->main.hotspot_mode;

	ap_update_data_device(ad);

	wifi_state = (Eina_Bool)(state & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI);
	bt_state = (Eina_Bool)(state & VCONFKEY_MOBILE_HOTSPOT_MODE_BT);
	usb_state = (Eina_Bool)(state & VCONFKEY_MOBILE_HOTSPOT_MODE_USB);
	DBG("wifi_state : %d, bt_state : %d, usb_state : %d\n",
			wifi_state, bt_state, usb_state);

	/* Update Wi-Fi tethering on / off button */
	if (ad->main.wifi_state != MH_STATE_NONE) {
		_update_wifi_item(ad, MH_STATE_NONE);
	} else {
		elm_check_state_set(ad->main.wifi_btn, wifi_state);
	}

	/* Update BT tethering on / off button */
	if (ad->main.bt_state != MH_STATE_NONE) {
		_update_bt_item(ad, MH_STATE_NONE);
	} else {
		elm_check_state_set(ad->main.bt_btn, bt_state);
	}

	/* Update USB tethering on / off button */
	if (ad->main.usb_state != MH_STATE_NONE) {
		_update_usb_item(ad, MH_STATE_NONE);
	} else {
		elm_check_state_set(ad->main.usb_btn, usb_state);
	}

	if (wifi_state || bt_state || usb_state) {
		_start_update_data_packet_usage(ad);
	}

	if (wifi_state || bt_state) {
		if (ad->main.help_item) {
			DBG("Just update help label item\n");
			elm_genlist_item_update(ad->main.help_item);
			return;
		} else {
			DBG("Add help item\n");
			item = elm_genlist_item_insert_after(ad->main.genlist,
					ad->main.help_itc, ad, NULL,
					ad->main.usb_item,
					ELM_GENLIST_ITEM_NONE, NULL,
					NULL);
			if (item == NULL) {
				ERR("elm_genlist_item_insert_after NULL\n");
				return;
			}
			elm_genlist_item_select_mode_set(item,
					ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			ad->main.help_item = item;
		}
	} else {
		if (ad->main.help_item) {
			DBG("Remove help item\n");
			elm_object_item_del(ad->main.help_item);
			ad->main.help_item = NULL;
		}
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void __wifi_onoff_changed_cb(void *data, Evas_Object *obj,
							void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	_update_wifi_item(ad, MH_STATE_PROCESS);

	if (_handle_wifi_onoff_change(ad) != 0) {
		ERR("_handle_wifi_onoff_change is failed\n");
		_update_wifi_item(ad, MH_STATE_NONE);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __select_wifi_item(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	__wifi_onoff_changed_cb(data, ad->main.wifi_btn, NULL);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __select_setup_item(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	mh_appdata_t *ad = (mh_appdata_t *)data;
	int connected_wifi_clients = 0;
	int ret = 0;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);

	if (tethering_is_enabled(ad->handle, TETHERING_TYPE_WIFI) == false) {
		mh_draw_wifi_setup_view(ad);
		return;
	}

	DBG("Wi-Fi tethering is on\n");
	if (_get_no_of_connected_device(ad->handle, &connected_wifi_clients,
				TETHERING_TYPE_WIFI) == FALSE) {
		ERR("Getting the number of connected device is failed\n");
	}

	if (connected_wifi_clients > 0) {
		_prepare_popup(ad, MH_POP_ENTER_TO_WIFI_SETUP_CONF,
				_("IDS_MOBILEAP_POP_CONNECTED_DEVICE_WILL_BE_DISCONNECTED"));
		_create_popup(ad);
	} else {
		_update_wifi_item(ad, MH_STATE_PROCESS);
		ret = tethering_disable(ad->handle, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("Wi-Fi tethering off is failed : %d\n", ret);
			_update_wifi_item(ad, MH_STATE_NONE);
		} else
			ad->main.need_recover_wifi_tethering = true;

		mh_draw_wifi_setup_view(ad);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __bt_onoff_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	_update_bt_item(ad, MH_STATE_PROCESS);

	if (_handle_bt_onoff_change(ad) != 0) {
		ERR("_handle_bt_onoff_change is failed\n");
		_update_bt_item(ad, MH_STATE_NONE);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __select_bt_item(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	mh_appdata_t *ad = (mh_appdata_t *)data;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);

	__bt_onoff_changed_cb(data, ad->main.bt_btn, NULL);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __usb_onoff_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	_update_usb_item(ad, MH_STATE_PROCESS);
	if (_handle_usb_onoff_change(ad) != 0) {
		ERR("_handle_usb_onoff_change is failed\n");
		_update_usb_item(ad, MH_STATE_NONE);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __select_usb_item(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	mh_appdata_t *ad = (mh_appdata_t *)data;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);

	__usb_onoff_changed_cb(data, ad->main.usb_btn, NULL);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __back_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t*)data;

	ug_destroy_me(((mh_ugdata_t *)ad->gadget)->ug);

	__MOBILE_AP_FUNC_EXIT__;
}

static char *__get_wifi_label(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.text") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	return strdup(_("IDS_MOBILEAP_MBODY_WI_FI_TETHERING"));
}

static Evas_Object *__get_wifi_icon(void *data, Evas_Object *obj,
		const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (strcmp(part, "elm.icon") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t*)data;
	Evas_Object *btn = NULL;
	Evas_Object *progressbar = NULL;

	if (ad->main.wifi_state == MH_STATE_PROCESS) {
		progressbar = elm_progressbar_add(obj);
		if (progressbar == NULL) {
			ERR("progressbar is NULL\n");
			return NULL;
		}
		elm_object_style_set(progressbar, "list_process");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		evas_object_show(progressbar);
		ad->main.wifi_btn = progressbar;
	} else {
		btn = elm_check_add(obj);
		elm_object_style_set(btn, "on&off");
		evas_object_show(btn);

		evas_object_pass_events_set(btn, EINA_TRUE);
		evas_object_propagate_events_set(btn, EINA_FALSE);
		elm_check_state_set(btn, ad->main.hotspot_mode &
				VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI ? EINA_TRUE : EINA_FALSE);

		evas_object_smart_callback_add(btn, "changed", __wifi_onoff_changed_cb,
				ad);
		ad->main.wifi_btn = btn;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return ad->main.wifi_btn;
}

static char *__get_setup_label(void *data, Evas_Object *obj, const char *part)
{
	if (data == NULL) {

		ERR("The param is NULL\n");
		return NULL;
	}

	if (strcmp(part, "elm.text") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	return strdup(_("IDS_MOBILEAP_MBODY_WI_FI_TETHERING_SETTINGS"));
}

static char *__get_bt_label(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.text") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	return strdup(_("IDS_MOBILEAP_BODY_BLUETOOTH_TETHERING"));
}

static Evas_Object *__get_bt_icon(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;
	Evas_Object *progressbar = NULL;

	if (strcmp(part, "elm.icon") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	ad->main.bt_btn = NULL;
	if (ad->main.bt_state == MH_STATE_PROCESS) {
		progressbar = elm_progressbar_add(obj);
		if (progressbar == NULL) {
			ERR("progressbar is NULL\n");
			return NULL;
		}
		elm_object_style_set(progressbar, "list_process");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		evas_object_show(progressbar);
		ad->main.bt_btn = progressbar;
	} else {
		btn = elm_check_add(obj);
		if (btn == NULL) {
			ERR("btn is NULL\n");
			return NULL;
		}
		elm_object_style_set(btn, "on&off");
		evas_object_pass_events_set(btn, EINA_TRUE);
		evas_object_propagate_events_set(btn, EINA_FALSE);
		elm_check_state_set(btn, ad->main.hotspot_mode &
				VCONFKEY_MOBILE_HOTSPOT_MODE_BT ? EINA_TRUE : EINA_FALSE);
		evas_object_show(btn);
		evas_object_smart_callback_add(btn, "changed", __bt_onoff_changed_cb,
				ad);
		ad->main.bt_btn = btn;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return ad->main.bt_btn;
}

static char *__get_usb_label(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.text") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	return strdup(_("IDS_MOBILEAP_MBODY_USB_TETHERING"));
}

static Evas_Object *__get_usb_icon(void *data, Evas_Object *obj,
							const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;
	Evas_Object *progressbar = NULL;

	if (strcmp(part, "elm.icon") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	ad->main.usb_btn = NULL;
	if (ad->main.usb_state == MH_STATE_PROCESS) {
		progressbar = elm_progressbar_add(obj);
		if (progressbar == NULL) {
			ERR("progressbar is NULL\n");
			return NULL;
		}
		elm_object_style_set(progressbar, "list_process");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		evas_object_show(progressbar);
		ad->main.usb_btn = progressbar;
	} else {
		btn = elm_check_add(obj);
		if (btn == NULL) {
			ERR("btn is NULL\n");
			return NULL;
		}
		elm_object_style_set(btn, "on&off");
		evas_object_pass_events_set(btn, EINA_TRUE);
		evas_object_propagate_events_set(btn, EINA_FALSE);
		elm_check_state_set(btn, ad->main.hotspot_mode &
				VCONFKEY_MOBILE_HOTSPOT_MODE_USB ? EINA_TRUE : EINA_FALSE);
		evas_object_show(btn);
		evas_object_smart_callback_add(btn, "changed", __usb_onoff_changed_cb,
				ad);
		ad->main.usb_btn = btn;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return ad->main.usb_btn;
}

static char *__get_help_label(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };
	char device_name[MH_LABEL_LENGTH_MAX] = {0, };
	char passphrase[MH_LABEL_LENGTH_MAX] = {0, };
	char *hidden = "";
	char *ptr = NULL;
	int wifi_state = VCONFKEY_MOBILE_HOTSPOT_MODE_NONE;

	if (strcmp(part, "elm.text") != 0) {
		ERR("Invalid param : %s\n", part);
		return NULL;
	}

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	ptr = elm_entry_utf8_to_markup(ad->setup.device_name);
	if (ptr == NULL) {
		ERR("elm_entry_utf8_to_markup is failed\n");
		return NULL;
	}
	g_strlcpy(device_name, ptr, MH_LABEL_LENGTH_MAX);
	free(ptr);

	if (ad->setup.security_type != TETHERING_WIFI_SECURITY_TYPE_NONE) {
		ptr = elm_entry_utf8_to_markup(ad->setup.wifi_passphrase);
		if (ptr == NULL) {
			ERR("elm_entry_utf8_to_markup is failed\n");
			return NULL;
		}
		g_strlcpy(passphrase, ptr, MH_LABEL_LENGTH_MAX);
		free(ptr);
	}

	wifi_state = ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI;

	if (wifi_state && ad->setup.visibility == false)
		hidden = _("IDS_MOBILEAP_BODY_WI_FI_TETHERING_HIDDEN");

	if (wifi_state && ad->setup.security_type != TETHERING_WIFI_SECURITY_TYPE_NONE) {
		snprintf(buf, MH_LABEL_LENGTH_MAX,
				"%s: %s<br>"
				"%s: %s<br><br>"
				"%s"
				"%s"
				"%s",
				_("IDS_MOBILEAP_BODY_DEVICE_NAME"),
				device_name,
				_("IDS_MOBILEAP_BODY_PASSWORD"),
				passphrase,
				_("IDS_MOBILEAP_BODY_THIS_PASSWORD_IS_ONLY_FOR_WI_FI_TETHERING"),
				hidden[0] != '\0' ? "<br>" : "",
				hidden);

	} else {
		snprintf(buf, MH_LABEL_LENGTH_MAX,
				"%s : %s%s%s",
				_("IDS_MOBILEAP_BODY_DEVICE_NAME"),
				device_name,
				hidden[0] != '\0' ? "<br>" : "",
				hidden);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return strdup(buf);
}

static char *__get_connected_device_label(void *data, Evas_Object *obj,
							const char *part)
{
	if (strcmp(part, "elm.text.1") != 0 && strcmp(part, "elm.text.2") != 0) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t*)data;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };

	if (strcmp(part, "elm.text.1") != 0)
		snprintf(buf, MH_LABEL_LENGTH_MAX, "%d",
				ad->clients.number);
	else if (strcmp(part, "elm.text.2") != 0)
		g_strlcpy(buf, _("IDS_MOBILEAP_BODY_CONNECTED_DEVICE"),
				sizeof(buf));

	return strdup(buf);
}

static char *__get_usage_label(void *data, Evas_Object *obj, const char *part)
{
	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (strcmp(part, "elm.text.1") != 0 && strcmp(part, "elm.text.2") != 0)	{
		ERR("Invalid param\n");
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t*)data;
	int sent = 0;
	int received = 0;
	char *sent_str = NULL;
	char *received_str = NULL;
	char fmt[MH_LABEL_LENGTH_MAX] = {0, };
	char label[MH_LABEL_LENGTH_MAX] = {0, };

	if (strcmp(part, "elm.text.1") != 0 ) {
		sent = ad->data_statistics.pdp_total_sent;
		received = ad->data_statistics.pdp_total_receive;

		if (sent >= MH_MB) {
			sent /= MH_MB;
			sent_str =  _("IDS_MOBILEAP_BODY_PD_MB");
		} else if (sent >= MH_KB) {
			sent /= MH_KB;
			sent_str =  _("IDS_MOBILEAP_BODY_PD_KB");
		} else {
			sent_str =  _("IDS_MOBILEAP_BODY_PD_BYTES");
		}

		if (received >= MH_MB) {
			received /= MH_MB;
			received_str =  _("IDS_MOBILEAP_BODY_PD_MB");
		} else if (received >= MH_KB) {
			received /= MH_KB;
			received_str =  _("IDS_MOBILEAP_BODY_PD_KB");
		} else {
			received_str =  _("IDS_MOBILEAP_BODY_PD_BYTES");
		}

		snprintf(fmt, sizeof(fmt), "%s %s / %s %s",
				_("IDS_MOBILEAP_BODY_SENT_C"),
				sent_str,
				_("IDS_MOBILEAP_BODY_RECEIVED_C"),
				received_str);
		snprintf(label, sizeof(label), fmt, sent, received);
	} else if (strcmp(part, "elm.text.2") != 0)
		g_strlcpy(label, _("IDS_MOBILEAP_MBODY_DATA_USAGE"),
				sizeof(label));

	return strdup(label);
}

static char *__gl_get_dev_label(void *data, Evas_Object *obj, const char *part)
{
	if (data == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	tethering_client_h client = (tethering_client_h)data;
	char *name = NULL;

	if (!strcmp(part, "elm.text")) {
		tethering_client_get_name(client, &name);
		DBG("Device name : %s\n", name);

		if (!strcmp(name, "UNKNOWN")) {
			return strdup(S_("IDS_COM_BODY_NO_NAME"));
		}

		return name;
	}

	return NULL;
}

static Evas_Object *__gl_get_dev_wifi_icon(void *data, Evas_Object *obj,
							const char *part)
{
	Evas_Object *icon;

	if (!strncmp(part, "elm.icon", 8)) {
		icon = elm_icon_add(obj);
		elm_image_file_set(icon, EDJDIR"/"TETHERING_IMAGES_EDJ, WIFI_ICON);
		evas_object_size_hint_aspect_set(icon,
				EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	return NULL;
}

static Evas_Object *__gl_get_dev_usb_icon(void *data, Evas_Object *obj,
							const char *part)
{
	Evas_Object *icon;

	if (!strncmp(part, "elm.icon", 8)) {
		icon = elm_icon_add(obj);
		elm_image_file_set(icon, EDJDIR"/"TETHERING_IMAGES_EDJ, USB_ICON);
		evas_object_size_hint_aspect_set(icon,
				EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	return NULL;
}

static Evas_Object *__gl_get_dev_bt_icon(void *data, Evas_Object *obj,
							const char *part)
{
	Evas_Object *icon;

	if (!strncmp(part, "elm.icon", 8)) {
		icon = elm_icon_add(obj);
		elm_image_file_set(icon, EDJDIR"/"TETHERING_IMAGES_EDJ, BT_ICON);
		evas_object_size_hint_aspect_set(icon,
					EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	return NULL;
}

static void _gl_exp(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	ap_update_data_device(ad);

	__MOBILE_AP_FUNC_EXIT__;
}

static void _gl_con(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (event_info == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_genlist_item_subitems_clear(ad->main.device_item);

	__MOBILE_AP_FUNC_EXIT__;
}

static void _gl_device_item_sel(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Elm_Object_Item *item = (Elm_Object_Item*)event_info;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	Eina_Bool status = elm_genlist_item_expanded_get(item);
	DBG("+ expand status:%d\n", status);

	elm_genlist_item_expanded_set(item, !status);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __free_genlist_itc(mh_appdata_t *ad)
{
	elm_genlist_item_class_free(ad->main.sp_itc);
	elm_genlist_item_class_free(ad->main.wifi_itc);
	elm_genlist_item_class_free(ad->main.setup_itc);
	elm_genlist_item_class_free(ad->main.bt_itc);
	elm_genlist_item_class_free(ad->main.usb_itc);
	elm_genlist_item_class_free(ad->main.help_itc);
	elm_genlist_item_class_free(ad->main.device_itc);
	elm_genlist_item_class_free(ad->main.device0_itc);
	elm_genlist_item_class_free(ad->main.usage_itc);
	elm_genlist_item_class_free(ad->main.dev_itc[TETHERING_TYPE_WIFI]);
	elm_genlist_item_class_free(ad->main.dev_itc[TETHERING_TYPE_USB]);
	elm_genlist_item_class_free(ad->main.dev_itc[TETHERING_TYPE_BT]);
	return;
}

static void __set_genlist_itc(mh_appdata_t *ad)
{
	/* On, Off view's item class for genlist */
	ad->main.sp_itc = elm_genlist_item_class_new();
	ad->main.sp_itc->item_style = "dialogue/separator";
	ad->main.sp_itc->func.text_get = NULL;
	ad->main.sp_itc->func.content_get = NULL;
	ad->main.sp_itc->func.state_get = NULL;
	ad->main.sp_itc->func.del = NULL;

	ad->main.wifi_itc = elm_genlist_item_class_new();
	ad->main.wifi_itc->item_style = "dialogue/1text.1icon";
	ad->main.wifi_itc->func.text_get = __get_wifi_label;
	ad->main.wifi_itc->func.content_get = __get_wifi_icon;
	ad->main.wifi_itc->func.state_get = NULL;
	ad->main.wifi_itc->func.del = NULL;

	ad->main.end_sp_itc = elm_genlist_item_class_new();
	ad->main.end_sp_itc->item_style = "dialogue/separator";
	ad->main.end_sp_itc->func.text_get = NULL;
	ad->main.end_sp_itc->func.content_get = NULL;
	ad->main.end_sp_itc->func.state_get = NULL;
	ad->main.end_sp_itc->func.del = NULL;
	/* End of On, Off view's item class for genlist */

	/* Off view's item class for genlist */
	ad->main.setup_itc = elm_genlist_item_class_new();
	ad->main.setup_itc->item_style = "dialogue/1text";
	ad->main.setup_itc->func.text_get = __get_setup_label;
	ad->main.setup_itc->func.content_get = NULL;
	ad->main.setup_itc->func.state_get = NULL;
	ad->main.setup_itc->func.del = NULL;

	ad->main.bt_itc = elm_genlist_item_class_new();
	ad->main.bt_itc->item_style = "dialogue/1text.1icon";
	ad->main.bt_itc->func.text_get = __get_bt_label;
	ad->main.bt_itc->func.content_get = __get_bt_icon;
	ad->main.bt_itc->func.state_get = NULL;
	ad->main.bt_itc->func.del = NULL;

	ad->main.usb_itc = elm_genlist_item_class_new();
	ad->main.usb_itc->item_style = "dialogue/1text.1icon";
	ad->main.usb_itc->func.text_get = __get_usb_label;
	ad->main.usb_itc->func.content_get = __get_usb_icon;
	ad->main.usb_itc->func.state_get = NULL;
	ad->main.usb_itc->func.del = NULL;

	ad->main.help_itc = elm_genlist_item_class_new();
	ad->main.help_itc->item_style = "multiline/1text";
	ad->main.help_itc->func.text_get = __get_help_label;
	ad->main.help_itc->func.content_get = NULL;
	ad->main.help_itc->func.state_get = NULL;
	ad->main.help_itc->func.del = NULL;

	/* End of Off view's item class for genlist */

	/* On view's item class for genlist */
	ad->main.device_itc = elm_genlist_item_class_new();
	ad->main.device_itc->item_style = "dialogue/2text.3/expandable";
	ad->main.device_itc->func.text_get = __get_connected_device_label;
	ad->main.device_itc->func.content_get = NULL;
	ad->main.device_itc->func.state_get = NULL;
	ad->main.device_itc->func.del = NULL;

	ad->main.device0_itc = elm_genlist_item_class_new();
	ad->main.device0_itc->item_style = "dialogue/2text.3";
	ad->main.device0_itc->func.text_get = __get_connected_device_label;
	ad->main.device0_itc->func.content_get = NULL;
	ad->main.device0_itc->func.state_get = NULL;
	ad->main.device0_itc->func.del = NULL;

	ad->main.usage_itc = elm_genlist_item_class_new();
	ad->main.usage_itc->item_style = "dialogue/2text.3";
	ad->main.usage_itc->func.text_get = __get_usage_label;
	ad->main.usage_itc->func.content_get = NULL;
	ad->main.usage_itc->func.state_get = NULL;
	ad->main.usage_itc->func.del = NULL;

	ad->main.dev_itc[TETHERING_TYPE_WIFI] = elm_genlist_item_class_new();
	ad->main.dev_itc[TETHERING_TYPE_WIFI]->item_style =
		"dialogue/1text.1icon/expandable2";
	ad->main.dev_itc[TETHERING_TYPE_WIFI]->func.text_get = __gl_get_dev_label;
	ad->main.dev_itc[TETHERING_TYPE_WIFI]->func.content_get = __gl_get_dev_wifi_icon;
	ad->main.dev_itc[TETHERING_TYPE_WIFI]->func.state_get = NULL;
	ad->main.dev_itc[TETHERING_TYPE_WIFI]->func.del = NULL;

	ad->main.dev_itc[TETHERING_TYPE_USB] = elm_genlist_item_class_new();
	ad->main.dev_itc[TETHERING_TYPE_USB]->item_style =
		"dialogue/1text.1icon/expandable2";
	ad->main.dev_itc[TETHERING_TYPE_USB]->func.text_get = __gl_get_dev_label;
	ad->main.dev_itc[TETHERING_TYPE_USB]->func.content_get = __gl_get_dev_usb_icon;
	ad->main.dev_itc[TETHERING_TYPE_USB]->func.state_get = NULL;
	ad->main.dev_itc[TETHERING_TYPE_USB]->func.del = NULL;

	ad->main.dev_itc[TETHERING_TYPE_BT] = elm_genlist_item_class_new();
	ad->main.dev_itc[TETHERING_TYPE_BT]->item_style =
		"dialogue/1text.1icon/expandable2";
	ad->main.dev_itc[TETHERING_TYPE_BT]->func.text_get = __gl_get_dev_label;
	ad->main.dev_itc[TETHERING_TYPE_BT]->func.content_get = __gl_get_dev_bt_icon;
	ad->main.dev_itc[TETHERING_TYPE_BT]->func.state_get = NULL;
	ad->main.dev_itc[TETHERING_TYPE_BT]->func.del = NULL;
	/* End of On view's item class for genlist */

	return;
}

static void __gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	mh_appdata_t *ad = (mh_appdata_t *)data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (item == ad->main.wifi_item || item == ad->main.bt_item || item == ad->main.device_item)
		elm_object_item_signal_emit(item, "elm,state,top", "");
	else if (item == ad->main.setup_item || item == ad->main.usage_item || item == ad->main.usb_item)
		elm_object_item_signal_emit(item, "elm,state,bottom", "");

	return;
}

static void __create_inner_contents(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	Elm_Object_Item *item = NULL;

	__read_setting(ad);
	if (ad->main.hotspot_mode != VCONFKEY_MOBILE_HOTSPOT_MODE_NONE)
		_start_update_data_packet_usage(ad);

	ad->main.genlist = elm_genlist_add(ad->naviframe);
	elm_genlist_mode_set(ad->main.genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(ad->main.genlist, "realized", __gl_realized, ad);

	__set_genlist_itc(ad);

	/* separator */
	item = elm_genlist_item_append(ad->main.genlist, ad->main.sp_itc, NULL,
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	item = elm_genlist_item_append(ad->main.genlist, ad->main.wifi_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE,
			__select_wifi_item, ad);
	ad->main.wifi_item = item;

	item = elm_genlist_item_append(ad->main.genlist, ad->main.setup_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE,
			__select_setup_item, ad);
	ad->main.setup_item = item;

	/* separator */
	item = elm_genlist_item_append(ad->main.genlist, ad->main.sp_itc, NULL,
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	item = elm_genlist_item_append(ad->main.genlist, ad->main.bt_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE,
			__select_bt_item, ad);
	ad->main.bt_item = item;

	item = elm_genlist_item_append(ad->main.genlist, ad->main.usb_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE,
			__select_usb_item, ad);
	ad->main.usb_item = item;

	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI ||
			ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_BT) {
		item = elm_genlist_item_append(ad->main.genlist, ad->main.help_itc,
				ad, NULL, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		DBG("elm_genlist_item_append for help_itc : %x\n", item);
		ad->main.help_item = item;
	}

	/* separator */
	item = elm_genlist_item_append(ad->main.genlist, ad->main.sp_itc, ad,
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	item = elm_genlist_item_append(ad->main.genlist, ad->main.usage_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	ad->main.usage_item = item;

	/* Insert "Connected devices" item */
	ap_update_data_device(ad);

	item = elm_genlist_item_append(ad->main.genlist, ad->main.end_sp_itc, NULL,
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

void ap_callback_del(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	evas_object_smart_callback_del(ad->main.back_btn, "clicked",
			__back_btn_cb);
	evas_object_smart_callback_del(ad->main.wifi_btn, "changed",
			__wifi_onoff_changed_cb);
	evas_object_smart_callback_del(ad->main.bt_btn, "changed",
			__bt_onoff_changed_cb);
	evas_object_smart_callback_del(ad->main.usb_btn, "changed",
			__usb_onoff_changed_cb);

	evas_object_smart_callback_del(ad->main.genlist, "expanded", _gl_exp);
	evas_object_smart_callback_del(ad->main.genlist, "contracted", _gl_con);
	evas_object_smart_callback_del(ad->main.genlist, "realized", __gl_realized);

	__MOBILE_AP_FUNC_EXIT__;
}

void ap_update_data_onoff(void* data)
{
	__MOBILE_AP_FUNC_ENTER__;

	__MOBILE_AP_FUNC_EXIT__;
}

void ap_draw_contents(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	__create_inner_contents(ad);

	ad->main.back_btn = elm_button_add(ad->naviframe);
	if (ad->main.back_btn == NULL) {
		ERR("elm_button_add is failed\n");
		if (ad->main.genlist) {
			evas_object_del(ad->main.genlist);
			ad->main.genlist = NULL;
		}
		return;
	}

	elm_object_style_set(ad->main.back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(ad->main.back_btn, "clicked",
			__back_btn_cb, ad);

	elm_naviframe_item_push(ad->naviframe,
			_("IDS_MOBILEAP_BODY_TETHERING"),
			ad->main.back_btn, NULL, ad->main.genlist, NULL);

	__MOBILE_AP_FUNC_EXIT__;
	return;
}
