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

#include <time.h>
#include <limits.h>
#include <efl_extension.h>

#include "mh_view_main.h"
#include "mh_popup.h"
#include "mh_string.h"

#if 0 /* device rename not supported */
static void __ctx_move_more_ctxpopup(Evas_Object *ctx, mh_appdata_t *ad);
static void __ctx_delete_more_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool rotate_flag = EINA_FALSE;
#endif
void _genlist_update_device_item(mh_appdata_t *ad);
mh_appdata_t *g_ad = NULL;
#define UPDATE_INTERVAL 1

#if 0 /* device rename not supported */
void _rotate_adjust_rename_popup(void)
{
	__MOBILE_AP_FUNC_ENTER__;

	int change_ang = 0;

	change_ang = elm_win_rotation_get(g_ad->win);
	if (change_ang == 0 || change_ang == 180) {
		_hadnle_rename_popup_rotation(EINA_TRUE);
	} else {
		_hadnle_rename_popup_rotation(EINA_FALSE);
	}
	__MOBILE_AP_FUNC_EXIT__;
}
#endif

void _select_connected_dev(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	_create_connected_client_view(ad);
	__MOBILE_AP_FUNC_EXIT__;
}

static bool _connected_clients_cb(tethering_client_h client, void *user_data)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return true;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	_append_list_client_handle(ad, client);

	__MOBILE_AP_FUNC_EXIT__;
	return true;
}

void ap_update_data_device(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	_genlist_update_device_item(ad);

	__MOBILE_AP_FUNC_EXIT__;
}

#ifdef TETHERING_DATA_USAGE_SUPPORT
Eina_Bool ap_update_data_packet_usage(mh_appdata_t *ad)
{
	if (ad == NULL) {
		ERR("Invalid param\n");
		return EINA_FALSE;
	}
	if (ad->main.usage_item == NULL) {
		return EINA_TRUE;
	}

	if (ad->main.usage_item)
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
		ad->update_statistics_handle = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	/* If previous data is not updated, new data is not required */
	if (ad->data_statistics.is_updated == false) {
		return ECORE_CALLBACK_RENEW;
	}

	/* Because previous data is updated, new data is required.
	   It will be returned asynchronously. */
	tethering_get_data_usage(ad->handle, _data_usage_cb, (void *)ad);
	ad->data_statistics.is_updated = false;

	return ECORE_CALLBACK_RENEW;
}
#endif

Eina_Bool ap_update_device_conn_time(void * data)
{
	if (!data) {
		ERR("The param is NULL\n");
		return ECORE_CALLBACK_CANCEL;
	}
	mh_appdata_t *ad = (mh_appdata_t *)data;

	int count = 0;
	if (ad->main.hotspot_mode == VCONFKEY_MOBILE_HOTSPOT_MODE_NONE) {
		_stop_update_device_conn_time(ad);
		__MOBILE_AP_FUNC_EXIT__;
		return ECORE_CALLBACK_CANCEL;
	}

	while (count < ad->connected_device.no_of_clients) {
		if (ad->connected_device.station_items[count])
			elm_genlist_item_fields_update(ad->connected_device.station_items[count++],
					"elm.text.sub", ELM_GENLIST_ITEM_FIELD_TEXT);
	}
	return ECORE_CALLBACK_RENEW;
}

void _start_update_device_conn_time(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		__MOBILE_AP_FUNC_EXIT__;
		return;
	}

	if (ad->update_conn_time_handle) {
		_stop_update_device_conn_time(ad);
	}
	ad->update_conn_time_handle = ecore_timer_add(UPDATE_INTERVAL,
			ap_update_device_conn_time, (void *)ad);

	__MOBILE_AP_FUNC_EXIT__;
}

void _stop_update_device_conn_time(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	if (ad->update_conn_time_handle) {
		ecore_timer_del(ad->update_conn_time_handle);
		ad->update_conn_time_handle = NULL;
	}
	__MOBILE_AP_FUNC_EXIT__;
}

#ifdef TETHERING_DATA_USAGE_SUPPORT
void _start_update_data_packet_usage(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	if (ad->update_statistics_handle) {
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
#endif

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
	bool visibility;
	tethering_wifi_security_type_e type;

	ad->main.hotspot_mode = _get_vconf_hotspot_mode();

#ifdef TETHERING_DATA_USAGE_SUPPORT
	ad->data_statistics.pdp_total_sent = 0;
	ad->data_statistics.pdp_total_receive = 0;
	if (ad->main.hotspot_mode != VCONFKEY_MOBILE_HOTSPOT_MODE_NONE)
		tethering_get_data_usage(ad->handle, _data_usage_cb, (void *)ad);
#endif

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

	ret = tethering_wifi_get_ssid_visibility(ad->handle, &visibility);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_get_ssid_visibility is failed\n");
	}
	ad->setup.visibility = visibility;
	ad->setup.visibility_new = visibility;

	ret = tethering_wifi_get_security_type(ad->handle, &type);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_get_security_type is failed\n");
	}
	ad->setup.security_type = type;
	ad->setup.security_type_new = type;

	__MOBILE_AP_FUNC_EXIT__;
}

void _update_wifi_item(mh_appdata_t *ad, int wifi_state)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->main.wifi_state == wifi_state) {
		return;
	}

	if (ad->main.wifi_state == MH_STATE_PROCESS) {
		ad->main.wifi_state = MH_STATE_NONE;
		elm_genlist_item_select_mode_set(ad->main.wifi_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
	} else if (ad->main.wifi_state == MH_STATE_NONE) {
		ad->main.wifi_state = MH_STATE_PROCESS;
		elm_genlist_item_select_mode_set(ad->main.wifi_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	if (ad->main.wifi_item)
		elm_genlist_item_update(ad->main.wifi_item);

	if (ad->main.setup_item)
		elm_object_item_signal_emit(ad->main.setup_item, "elm,state,bottom", "");

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

void _update_bt_item(mh_appdata_t *ad, int bt_state)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->main.bt_state == bt_state) {
		return;
	}

	if (ad->main.bt_state == MH_STATE_PROCESS) {
		ad->main.bt_state = MH_STATE_NONE;
		elm_genlist_item_select_mode_set(ad->main.bt_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
	} else if (ad->main.bt_state == MH_STATE_NONE) {
		ad->main.bt_state = MH_STATE_PROCESS;
		elm_genlist_item_select_mode_set(ad->main.bt_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	if (ad->main.bt_item)
		elm_genlist_item_update(ad->main.bt_item);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

void _update_usb_item(mh_appdata_t *ad, int usb_state)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->main.usb_state == usb_state) {
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

void _genlist_update_device_item(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}
	unsigned int no_of_dev = 0;
	no_of_dev = _get_list_clients_count(ad);
	Elm_Object_Item *item = NULL;

	if (ad->main.device_item) {
		elm_object_item_del(ad->main.device_item);
		ad->main.device_item = NULL;
		if (no_of_dev == 0) {
			item = elm_genlist_item_append(ad->main.genlist,
					ad->main.device0_itc, ad, NULL,
					ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			item = elm_genlist_item_append(ad->main.genlist,
					ad->main.device_itc, ad, NULL,
					ELM_GENLIST_ITEM_NONE, _select_connected_dev, (void *)ad);
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DEFAULT);
		}
		ad->main.device_item = item;
	}

	if (ad->connected_device.navi_it) {
		_update_conn_clients(ad);
	}

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

void _update_main_view(mh_appdata_t *ad, tethering_type_e type)
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
	Eina_Bool wifi_ap_state = EINA_FALSE;
	Elm_Object_Item *item = NULL;
	Evas_Object *obj;
	int no_of_dev = 0;

	ad->main.hotspot_mode = _get_vconf_hotspot_mode();
	state = ad->main.hotspot_mode;

	wifi_state = (Eina_Bool)(state & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI);
	bt_state = (Eina_Bool)(state & VCONFKEY_MOBILE_HOTSPOT_MODE_BT);
	usb_state = (Eina_Bool)(state & VCONFKEY_MOBILE_HOTSPOT_MODE_USB);
	wifi_ap_state = (Eina_Bool)(state & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI_AP);

	if (wifi_ap_state) {
		if (ad->main.wifi_item)
			elm_object_item_disabled_set(ad->main.wifi_item, EINA_TRUE);
		if (ad->main.setup_item)
			elm_object_item_disabled_set(ad->main.setup_item, EINA_TRUE);
		if (ad->main.bt_item)
			elm_object_item_disabled_set(ad->main.bt_item, EINA_TRUE);
		if (ad->main.usb_item)
			elm_object_item_disabled_set(ad->main.usb_item, EINA_TRUE);
	} else {
		if (ad->main.setup_item)
			elm_object_item_disabled_set(ad->main.setup_item, EINA_FALSE);
		if (ad->main.bt_item)
			elm_object_item_disabled_set(ad->main.bt_item, EINA_FALSE);
		if (_get_vconf_usb_state() != VCONFKEY_SYSMAN_USB_AVAILABLE) {
			if (ad->main.usb_item)
				elm_object_item_disabled_set(ad->main.usb_item, EINA_TRUE);
		} else {
			if (ad->main.usb_item)
				elm_object_item_disabled_set(ad->main.usb_item, EINA_FALSE);
		}
	}

	if (wifi_state || bt_state || usb_state) {
#ifdef TETHERING_DATA_USAGE_SUPPORT
		if (ad->main.usage_item == NULL) {
			item = elm_genlist_item_insert_before(ad->main.genlist,
					ad->main.usage_itc, ad, NULL,
					ad->main.sp_item[0],
					ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(item,
					ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			ad->main.usage_item = item;
		}
#endif

		no_of_dev = _get_list_clients_count(ad);
		if (ad->main.device_item == NULL) {
			if (no_of_dev == 0) {
				item = elm_genlist_item_append(ad->main.genlist,
						ad->main.device0_itc, ad, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				item = elm_genlist_item_append(ad->main.genlist,
						ad->main.device_itc, ad, NULL,
						ELM_GENLIST_ITEM_NONE, _select_connected_dev, (void *)ad);
				elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DEFAULT);
			}
			ad->main.device_item = item;
		}
	} else {
		if (ad->main.device_item) {
			elm_object_item_del(ad->main.device_item);
			ad->main.device_item = NULL;
		}
#ifdef TETHERING_DATA_USAGE_SUPPORT
		if (ad->main.usage_item) {
			elm_object_item_del(ad->main.usage_item);
			ad->main.usage_item = NULL;
		}
#endif
	}

	if (wifi_state) {
		if (ad->main.help_item) {
			elm_genlist_item_update(ad->main.help_item);
		} else {
			item = elm_genlist_item_insert_after(ad->main.genlist,
					ad->main.help_itc, ad, NULL,
					ad->main.wifi_item,
					ELM_GENLIST_ITEM_NONE, NULL,
					NULL);
			if (item == NULL) {
				ERR("elm_genlist_item_insert_after NULL\n");
			} else {
				elm_genlist_item_select_mode_set(item,
						ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
				ad->main.help_item = item;
			}
		}
	} else {
		if (ad->main.help_item) {
			elm_object_item_del(ad->main.help_item);
			ad->main.help_item = NULL;
		}
	}

	switch (type) {
	case TETHERING_TYPE_WIFI:
		/* Update Wi-Fi tethering on / off button */
		if (ad->main.wifi_state != MH_STATE_NONE) {
			_update_wifi_item(ad, MH_STATE_NONE);
		} else {
			obj = elm_object_item_part_content_get(ad->main.wifi_item, "elm.swallow.end");
			if (obj != NULL) {
				elm_check_state_set(obj, wifi_state);
			}

			if (ad->main.wifi_item) {
				elm_genlist_item_update(ad->main.wifi_item);
			}
		}
		break;

	case TETHERING_TYPE_BT:
		/* Update BT tethering on / off button */
		if (ad->main.bt_state != MH_STATE_NONE) {
			_update_bt_item(ad, MH_STATE_NONE);
		} else {
			obj = elm_object_item_part_content_get(ad->main.bt_item, "elm.swallow.end");
			if (obj != NULL) {
				elm_check_state_set(obj, bt_state);
			}

			if (ad->main.bt_item) {
				elm_genlist_item_update(ad->main.bt_item);
			}
		}
		break;

	case TETHERING_TYPE_USB:
		/* Update USB tethering on / off button */
		if (ad->main.usb_state != MH_STATE_NONE) {
			_update_usb_item(ad, MH_STATE_NONE);
		} else {
			obj = elm_object_item_part_content_get(ad->main.usb_item, "elm.swallow.end");
			if (obj != NULL)
				elm_check_state_set(obj, usb_state);

			if (ad->main.usb_item)
				elm_genlist_item_update(ad->main.usb_item);
		}
		break;

	default:
		DBG("Unknown tethering type : %d\n", type);
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
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
	ad->type = TETHERING_TYPE_WIFI;
	ad->is_wifi_teth_enabling = true;
	if (_handle_wifi_onoff_change(ad) != 0) {
		ERR("_handle_wifi_onoff_change is failed\n");
		_update_wifi_item(ad, MH_STATE_NONE);
		ad->is_wifi_teth_enabling = false;
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
	mh_draw_wifi_setup_view(ad);

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
	ad->type = TETHERING_TYPE_BT;
	ad->is_bt_teth_enabling = true;

	if (_handle_bt_onoff_change(ad) != 0) {
		ERR("_handle_bt_onoff_change is failed\n");
		_update_bt_item(ad, MH_STATE_NONE);
		ad->is_bt_teth_enabling = false;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __select_bt_item(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Object *content;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);

	content = elm_object_item_part_content_get(item, "elm.icon");
	__bt_onoff_changed_cb(data, content, NULL);

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
	ad->type = TETHERING_TYPE_USB;
	ad->is_usb_teth_enabling = true;
	if (_handle_usb_onoff_change(ad) != 0) {
		ERR("_handle_usb_onoff_change is failed\n");
		_update_usb_item(ad, MH_STATE_NONE);
		ad->is_usb_teth_enabling = false;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __select_usb_item(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Object *content;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	elm_genlist_item_selected_set(item, EINA_FALSE);

	content = elm_object_item_part_content_get(item, "elm.icon");
	__usb_onoff_changed_cb(data, content, NULL);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static Eina_Bool __back_btn_cb(void *data, Elm_Object_Item *navi_item)
{
	INFO("+\n");

	if (data == NULL) {
		ERR("The param is NULL\n");
		return EINA_FALSE;
	}

	mh_appdata_t *ad = (mh_appdata_t*)data;

	_release_list_client_handle(ad);
	_main_callback_del(ad);

	ug_destroy_me(((mh_ugdata_t *)ad->gadget)->ug);

	INFO("-\n");
	return EINA_FALSE;
}

static char *__get_wifi_label(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp("elm.text", part)) {
		return strdup(STR_WIFI_TETH);
	}

	return NULL;
}

static Evas_Object *__get_wifi_icon(void *data, Evas_Object *obj,
		const char *part)
{
	mh_appdata_t *ad = (mh_appdata_t*)data;
	Evas_Object *btn = NULL;
	Evas_Object *progressbar = NULL;
	Evas_Object *icon_layout = NULL;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (!strcmp("elm.swallow.end", part)) {
		icon_layout = elm_layout_add(obj);
		elm_layout_theme_set(icon_layout, "layout", "list/C/type.3", "default");

		if (ad->main.wifi_state == MH_STATE_PROCESS) {
			progressbar = _create_progressbar(obj, "process_medium");
			elm_layout_content_set(icon_layout, "elm.swallow.content", progressbar);
		} else {
			btn = elm_check_add(obj);
			elm_object_style_set(btn, "on&off");
			evas_object_show(btn);

			evas_object_pass_events_set(btn, EINA_TRUE);
			evas_object_propagate_events_set(btn, EINA_FALSE);
			elm_check_state_set(btn, ad->main.hotspot_mode &
				VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI ? EINA_TRUE : EINA_FALSE);

			evas_object_smart_callback_add(btn, "changed", __wifi_onoff_changed_cb, ad);
			elm_layout_content_set(icon_layout, "elm.swallow.content", btn);
		}
	}

	return icon_layout;
}

static char *__get_bt_label(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp("elm.text", part)) {
		return strdup(STR_BLUETOOTH_TETH);
	}

	return NULL;
}

static Evas_Object *__get_bt_icon(void *data, Evas_Object *obj, const char *part)
{
	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;
	Evas_Object *progressbar = NULL;
	Evas_Object *icon_layout = NULL;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (!strcmp("elm.swallow.end", part)) {
		icon_layout = elm_layout_add(obj);
		elm_layout_theme_set(icon_layout, "layout", "list/C/type.3", "default");

		if (ad->main.bt_state == MH_STATE_PROCESS) {
			progressbar = _create_progressbar(obj, "process_medium");
			elm_layout_content_set(icon_layout, "elm.swallow.content", progressbar);
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
			evas_object_smart_callback_add(btn, "changed", __bt_onoff_changed_cb, ad);
			elm_layout_content_set(icon_layout, "elm.swallow.content", btn);
		}
	}

	return icon_layout;
}

static char *__get_usb_label(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp("elm.text", part)) {
		return strdup(STR_USB_TETH);
	}

	return NULL;
}

static Evas_Object *__get_usb_icon(void *data, Evas_Object *obj,
							const char *part)
{
	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;
	Evas_Object *progressbar = NULL;
	Evas_Object *icon_layout = NULL;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (!strcmp("elm.swallow.end", part)) {
		icon_layout = elm_layout_add(obj);
		elm_layout_theme_set(icon_layout, "layout", "list/C/type.3", "default");

		if (ad->main.usb_state == MH_STATE_PROCESS) {
			progressbar = _create_progressbar(obj, "process_medium");
			elm_layout_content_set(icon_layout, "elm.swallow.content", progressbar);
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
			evas_object_smart_callback_add(btn, "changed", __usb_onoff_changed_cb, ad);
			elm_layout_content_set(icon_layout, "elm.swallow.content", btn);
		}
	}

	return icon_layout;
}

static char *__get_help_label(void *data, Evas_Object *obj, const char *part)
{
	mh_appdata_t *ad = (mh_appdata_t *)data;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };
	char device_name[MH_LABEL_LENGTH_MAX] = {0, };
	char passphrase[MH_LABEL_LENGTH_MAX] = {0, };
	char security_type[MH_LABEL_LENGTH_MAX] = {0, };
	char *hidden = "";
	char *ptr = NULL;
	char *device_name_utf = NULL;
	int wifi_state = VCONFKEY_MOBILE_HOTSPOT_MODE_NONE;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (!strcmp("elm.text.multiline", part)) {
		device_name_utf = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
		if (device_name_utf == NULL) {
			ERR("vconf_get_str failed \n");
			return NULL;
		}

		ptr = elm_entry_utf8_to_markup(device_name_utf);
		if (ptr == NULL) {
			g_free(device_name_utf);
			ERR("elm_entry_utf8_to_markup is failed\n");
			return NULL;
		}

		g_strlcpy(ad->setup.device_name, ptr,
				sizeof(ad->setup.device_name));
		g_strlcpy(device_name, ptr, MH_LABEL_LENGTH_MAX);
		g_free(device_name_utf);
		g_free(ptr);
		ptr = NULL;

		if (ad->setup.security_type != TETHERING_WIFI_SECURITY_TYPE_NONE) {
			ptr = elm_entry_utf8_to_markup(ad->setup.wifi_passphrase);
			if (ptr == NULL) {
				ERR("elm_entry_utf8_to_markup is failed\n");
				return NULL;
			}
			g_strlcpy(passphrase, ptr, MH_LABEL_LENGTH_MAX);
			g_free(ptr);

			snprintf(security_type, sizeof(security_type),
					STR_SECURITY_TYPE_PS, "WPA2 PSK");
		}

		wifi_state = ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI;

		if (wifi_state && ad->setup.visibility == false)
			hidden = STR_WIFI_TETH_HIDDEN;

		if (wifi_state && ad->setup.security_type != TETHERING_WIFI_SECURITY_TYPE_NONE) {
			snprintf(buf, MH_LABEL_LENGTH_MAX,
					"<font_size=30>"
					"%s: %s<br>"
					"%s: %s<br>"
					"%s<br>"
					"%s%s"
					"%s"
					"</font_size>",
					STR_DEV_NAME,
					device_name,
					STR_PASSWORD,
					passphrase,
					security_type,
					STR_PASSWORD_FOR_WIFI_TETH,
					hidden[0] != '\0' ? "<br>" : "",
					hidden);
		} else {
			snprintf(buf, MH_LABEL_LENGTH_MAX,
					"<font_size=30>"
					"%s: %s%s"
					"%s"
					"</font_size>",
					STR_DEV_NAME,
					device_name,
					hidden[0] != '\0' ? "<br>" : "",
					hidden);
		}

		return strdup(buf);
	}

	return NULL;
}

static char *__get_no_connected_device_label(void *data, Evas_Object *obj,
							const char *part)
{
	mh_appdata_t *ad = (mh_appdata_t*)data;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };
	int no_of_dev;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (!strcmp("elm.text.multiline", part)) {
		no_of_dev = _get_list_clients_count(ad);
		snprintf(buf, MH_LABEL_LENGTH_MAX, "<font_size=30>%s<br>%d</font_size>", STR_CONNECTED_DEV, no_of_dev);
		return strdup(buf);
	}

	return NULL;
}

static char *__get_connected_device_label(void *data, Evas_Object *obj,
							const char *part)
{
	mh_appdata_t *ad = (mh_appdata_t*)data;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };
	int no_of_dev;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (!strcmp("elm.text.sub", part)) {
		g_strlcpy(buf, STR_CONNECTED_DEV, sizeof(buf));
		return strdup(buf);
	} else if (!strcmp("elm.text", part)) {
		no_of_dev = _get_list_clients_count(ad);
		snprintf(buf, MH_LABEL_LENGTH_MAX, "%d", no_of_dev);
		return strdup(buf);
	}

	return NULL;
}

#ifdef TETHERING_DATA_USAGE_SUPPORT
static char *__get_usage_label(void *data, Evas_Object *obj, const char *part)
{
	mh_appdata_t *ad = (mh_appdata_t*)data;
	unsigned long long total = 0;
	unsigned long long sent = 0;
	unsigned long long received = 0;
	char *fmt_str;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };
	char label[MH_LABEL_LENGTH_MAX] = {0, };

	if (data == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (!strcmp("elm.text", part)) {
		g_strlcpy(label, STR_DATA_USAGE, sizeof(label));
		return strdup(label);
	} else if (!strcmp("elm.text.multiline", part)) {
		sent = ad->data_statistics.pdp_total_sent;
		received = ad->data_statistics.pdp_total_receive;

		if (sent >= MH_MB || received >= MH_MB) {
			sent /= MH_MB;
			received /= MH_MB;

			total = sent + received;
			fmt_str = STR_MB;
		} else if (sent + received >= MH_MB) {
			total = (sent + received) / MH_MB;
			fmt_str = STR_MB;
		} else if (sent >= MH_KB || received >= MH_KB) {
			sent /= MH_KB;
			received /= MH_KB;

			total = sent + received;
			fmt_str = STR_KB;
		} else if (sent + received >= MH_KB) {
			total = (sent + received) / MH_KB;
			fmt_str = STR_KB;
		} else {
			total = sent + received;
			fmt_str = STR_BYTE;
		}

		if (total > INT_MAX) {
			ERR("data usage overflow\n");
			total = 0;
		}
		snprintf(label, MH_LABEL_LENGTH_MAX, fmt_str, (int)total);
		return strdup(label);
	}

	return NULL;
}
#endif

static void __set_genlist_itc(mh_appdata_t *ad)
{
	/* On, Off view's item class for genlist */
#if 0 /* not used */
	ad->main.sp_itc = elm_genlist_item_class_new();
	if (ad->main.sp_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.sp_itc->item_style = "dialogue/separator";
	ad->main.sp_itc->func.text_get = NULL;
	ad->main.sp_itc->func.content_get = NULL;
	ad->main.sp_itc->func.state_get = NULL;
	ad->main.sp_itc->func.del = NULL;
#endif
	ad->main.wifi_itc = elm_genlist_item_class_new();
	if (ad->main.wifi_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.wifi_itc->item_style = MH_GENLIST_1LINE_TEXT_ICON_STYLE;
	ad->main.wifi_itc->func.text_get = __get_wifi_label;
	ad->main.wifi_itc->func.content_get =  __get_wifi_icon;
	ad->main.wifi_itc->func.state_get = NULL;
	ad->main.wifi_itc->func.del = NULL;

#if 0 /* not used */
	ad->main.sp2_itc = elm_genlist_item_class_new();
	if (ad->main.sp2_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.sp2_itc->item_style = "dialogue/separator.2";
	ad->main.sp2_itc->func.text_get = NULL;
	ad->main.sp2_itc->func.content_get = NULL;
	ad->main.sp2_itc->func.state_get = NULL;
	ad->main.sp2_itc->func.del = NULL;
#endif
	/* End of On, Off view's item class for genlist */

	/* Off view's item class for genlist */
	ad->main.bt_itc = elm_genlist_item_class_new();
	if (ad->main.bt_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.bt_itc->item_style = MH_GENLIST_1LINE_TEXT_ICON_STYLE;
	ad->main.bt_itc->func.text_get = __get_bt_label;
	ad->main.bt_itc->func.content_get = __get_bt_icon;
	ad->main.bt_itc->func.state_get = NULL;
	ad->main.bt_itc->func.del = NULL;

	ad->main.usb_itc = elm_genlist_item_class_new();
	if (ad->main.usb_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.usb_itc->item_style = MH_GENLIST_1LINE_TEXT_ICON_STYLE;
	ad->main.usb_itc->func.text_get = __get_usb_label;
	ad->main.usb_itc->func.content_get = __get_usb_icon;
	ad->main.usb_itc->func.state_get = NULL;
	ad->main.usb_itc->func.del = NULL;

	ad->main.help_itc = elm_genlist_item_class_new();
	if (ad->main.help_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.help_itc->item_style = MH_GENLIST_MULTILINE_TEXT_STYLE;
	ad->main.help_itc->func.text_get = __get_help_label;
	ad->main.help_itc->func.content_get = NULL;
	ad->main.help_itc->func.state_get = NULL;
	ad->main.help_itc->func.del = NULL;

	/* End of Off view's item class for genlist */

	/* On view's item class for genlist */
	ad->main.device0_itc = elm_genlist_item_class_new();
	if (ad->main.device0_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.device0_itc->item_style = MH_GENLIST_MULTILINE_TEXT_STYLE;
	ad->main.device0_itc->func.text_get = __get_no_connected_device_label;
	ad->main.device0_itc->func.content_get = NULL;
	ad->main.device0_itc->func.state_get = NULL;
	ad->main.device0_itc->func.del = NULL;

	ad->main.device_itc = elm_genlist_item_class_new();
	if (ad->main.device_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.device_itc->item_style = MH_GENLIST_2LINE_BOTTOM_TEXT_STYLE;
	ad->main.device_itc->func.text_get = __get_connected_device_label;
	ad->main.device_itc->func.content_get = NULL;
	ad->main.device_itc->func.state_get = NULL;
	ad->main.device_itc->func.del = NULL;

#ifdef TETHERING_DATA_USAGE_SUPPORT
	ad->main.usage_itc = elm_genlist_item_class_new();
	if (ad->main.usage_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		return;
	}

	ad->main.usage_itc->item_style = MH_GENLIST_MULTILINE_TEXT_STYLE;
	ad->main.usage_itc->func.text_get = __get_usage_label;
	ad->main.usage_itc->func.content_get = NULL;
	ad->main.usage_itc->func.state_get = NULL;
	ad->main.usage_itc->func.del = NULL;
#endif
	/* End of On view's item class for genlist */
	return;
}

static void __gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	if (data == NULL || event_info == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Evas_Object *ao;
	Evas_Object *btn;
	char str[MH_LABEL_LENGTH_MAX] = {0, };
	int i = 0;

	if (item == ad->main.wifi_item || item == ad->main.bt_item || item == ad->main.usb_item) {
		ao = elm_object_item_access_object_get(item);
		btn = elm_object_item_part_content_get(item, "on&off");
		snprintf(str, sizeof(str), "%s, %s", "On/off button",
				(elm_check_state_get(btn) ? "On" : "Off"));
		elm_access_info_set(ao, ELM_ACCESS_CONTEXT_INFO, str);

		if (item == ad->main.wifi_item || item == ad->main.bt_item) {
			DBG("Wi-Fi or BT item : %p\n", item);
			elm_object_item_signal_emit(item, "elm,state,top", "");
		} else if (item == ad->main.usb_item) {
			DBG("USB item\n");
			elm_object_item_signal_emit(item, "elm,state,bottom", "");
		}
	} else if (item == ad->main.setup_item) {
		DBG("setup_item\n");
		ao = elm_object_item_access_object_get(item);
		elm_access_info_set(ao, ELM_ACCESS_CONTEXT_INFO, "Item");

		elm_object_item_signal_emit(item, "elm,state,bottom", "");
	} else if (item == ad->main.device_item) {
		DBG("device_item\n");
		ao = elm_object_item_access_object_get(item);
		snprintf(str, sizeof(str), "%s, %s", "Expandable list",
				"Double tap to open list");
		elm_access_info_set(ao, ELM_ACCESS_CONTEXT_INFO, str);

		elm_object_item_signal_emit(item, "elm,state,top", "");
	} else if (ad->main.device_item != NULL &&
			ad->main.device_item == elm_genlist_item_parent_get(item)) {
		DBG("device_item's child\n");
		elm_object_item_signal_emit(item, "elm,state,center", "");
	} else {
		for (i = 0; i < 4; i++) {
			if (item == ad->main.sp_item[i])
				elm_object_item_access_unregister(item);
		}
	}
#ifdef TETHERING_DATA_USAGE_SUPPORT
	if (item == ad->main.usage_item) {
			DBG("usage_item\n");
			elm_object_item_signal_emit(item, "elm,state,bottom", "");
	}
#endif
	return;
}

static void __create_inner_contents(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	Elm_Object_Item *item = NULL;
	int no_of_dev = 0;

	__read_setting(ad);
	ad->main.genlist = elm_genlist_add(ad->naviframe);
	elm_genlist_mode_set(ad->main.genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(ad->main.genlist, "realized", __gl_realized, ad);

	__set_genlist_itc(ad);

	item = elm_genlist_item_append(ad->main.genlist, ad->main.wifi_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE,
			__select_wifi_item, ad);
	ad->main.wifi_item = item;

	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI) {
		item = elm_genlist_item_append(ad->main.genlist, ad->main.help_itc,
				ad, NULL, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		ad->main.help_item = item;
	}

	item = elm_genlist_item_append(ad->main.genlist, ad->main.bt_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE,
			__select_bt_item, ad);
	ad->main.bt_item = item;

	item = elm_genlist_item_append(ad->main.genlist, ad->main.usb_itc,
			ad, NULL, ELM_GENLIST_ITEM_NONE,
			__select_usb_item, ad);
	ad->main.usb_item = item;
	if (_get_vconf_usb_state() != VCONFKEY_SYSMAN_USB_AVAILABLE) {
		if (ad->main.usb_item)
			elm_object_item_disabled_set(ad->main.usb_item, EINA_TRUE);
	} else {
		if (ad->main.usb_item)
			elm_object_item_disabled_set(ad->main.usb_item, EINA_FALSE);
	}
	if (ad->main.hotspot_mode & (VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI |
				VCONFKEY_MOBILE_HOTSPOT_MODE_USB |
				VCONFKEY_MOBILE_HOTSPOT_MODE_BT)) {
#ifdef TETHERING_DATA_USAGE_SUPPORT
		item = elm_genlist_item_append(ad->main.genlist, ad->main.usage_itc,
				ad, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(item,
				ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		ad->main.usage_item = item;
#endif
		/* Insert "Connected devices" item */
		tethering_foreach_connected_clients(ad->handle, TETHERING_TYPE_ALL,
				_connected_clients_cb, (void *)ad);

		no_of_dev = _get_list_clients_count(ad);
		if (no_of_dev == 0) {
			item = elm_genlist_item_append(ad->main.genlist,
						ad->main.device0_itc, ad, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			item = elm_genlist_item_append(ad->main.genlist,
						ad->main.device_itc, ad, NULL,
						ELM_GENLIST_ITEM_NONE, _select_connected_dev, (void *)ad);
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DEFAULT);
		}
		ad->main.device_item = item;
	}

	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI_AP) {
		if (ad->main.wifi_item)
			elm_object_item_disabled_set(ad->main.wifi_item, EINA_TRUE);
		if (ad->main.setup_item)
			elm_object_item_disabled_set(ad->main.setup_item, EINA_TRUE);
		if (ad->main.bt_item)
			elm_object_item_disabled_set(ad->main.bt_item, EINA_TRUE);
		if (ad->main.usb_item)
			elm_object_item_disabled_set(ad->main.usb_item, EINA_TRUE);
	}

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

void _main_free_genlist_itc(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL)
		return;

	mh_main_view_t *mv = &ad->main;

#ifdef TETHERING_DATA_USAGE_SUPPORT
	_free_genlist_itc(&mv->usage_itc);
#endif
	_free_genlist_itc(&mv->device_itc);
	_free_genlist_itc(&mv->device0_itc);
	_free_genlist_itc(&mv->help_itc);
	_free_genlist_itc(&mv->usb_itc);
	_free_genlist_itc(&mv->bt_itc);
	_free_genlist_itc(&mv->setup_itc);
	_free_genlist_itc(&mv->wifi_itc);
#if 0 /* not used */
	_free_genlist_itc(&mv->sp_itc);
	_free_genlist_itc(&mv->sp2_itc);
#endif
	__MOBILE_AP_FUNC_EXIT__;
	return;
}

void _main_callback_del(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	Evas_Object *obj;

	obj = elm_object_item_part_content_get(ad->main.wifi_item, "elm.swallow.end");
	if (obj != NULL)
		evas_object_smart_callback_del(obj, "changed", __wifi_onoff_changed_cb);

	obj = elm_object_item_part_content_get(ad->main.bt_item, "elm.swallow.end");
	if (obj != NULL)
		evas_object_smart_callback_del(obj, "changed", __bt_onoff_changed_cb);

	obj = elm_object_item_part_content_get(ad->main.usb_item, "elm.swallow.end");
	if (obj != NULL)
		evas_object_smart_callback_del(obj, "changed", __usb_onoff_changed_cb);

	evas_object_smart_callback_del(ad->main.genlist, "realized", __gl_realized);

	__MOBILE_AP_FUNC_EXIT__;
}

#if 0 /* device rename not supported */
static void __ctx_move_more_ctxpopup(Evas_Object *ctx, mh_appdata_t *ad)
{
	Evas_Coord w;
	Evas_Coord h;
	int pos = -1;
	__MOBILE_AP_FUNC_ENTER__;

	elm_win_screen_size_get(ad->win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(ad->win);

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(ctx, w/2, h);
			break;
		case 90:
			evas_object_move(ctx, h/2, w);
			break;
		case 270:
			evas_object_move(ctx, h/2, w);
			break;
	}
	__MOBILE_AP_FUNC_EXIT__;
}

static void __rotate_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *ctx = ad->ctxpopup;

	__ctx_move_more_ctxpopup(ctx, ad);
	evas_object_show(ctx);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __dismissed_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *ctx = ad->ctxpopup;

	if (!rotate_flag) {
		evas_object_del(ctx);
		ctx = NULL;
	} else {
		__ctx_move_more_ctxpopup(ctx, ad);
		evas_object_show(ctx);
		rotate_flag = EINA_FALSE;
	}
	__MOBILE_AP_FUNC_EXIT__;
}

static void __ctx_delete_more_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj,
		void *event_info)
{
	Evas_Object *navi = (Evas_Object *)data;
	Evas_Object *ctx = obj;

	if (navi == NULL) {
		ERR("data is null\n");
		return;
	}
	__MOBILE_AP_FUNC_ENTER__;

	evas_object_smart_callback_del(ctx, "dismissed",
			__dismissed_more_ctxpopup_cb);
	evas_object_smart_callback_del(elm_object_top_widget_get(ctx),
			"rotation,changed", __rotate_more_ctxpopup_cb);
	evas_object_event_callback_del_full(ctx, EVAS_CALLBACK_DEL,
			__ctx_delete_more_ctxpopup_cb, navi);
	__MOBILE_AP_FUNC_EXIT__;
}

static void _gl_rename_device_sel(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;

	if (ad == NULL) {
		ERR("ad is null\n");
		return;
	}
	evas_object_del(ad->ctxpopup);
	ad->ctxpopup = NULL;

	_create_rename_device_popup((void *)ad);
	__MOBILE_AP_FUNC_EXIT__;
}

static void __create_ctxpopup_more_button(void *data, Evas_Object *obj,
		void *event_info)
{
	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *ctxpopup = NULL;

	if (ad == NULL) {
		ERR("ad is null\n");
		return;
	}
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->setup.navi_it == NULL) {
		return;
	}

	elm_naviframe_item_pop_cb_set(ad->setup.navi_it, _setting_back_btn_cb, (void *)ad);
	ctxpopup = elm_ctxpopup_add(ad->naviframe);
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);

	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK,
			eext_ctxpopup_back_cb, ad);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE,
			eext_ctxpopup_back_cb, ad);
	elm_object_style_set(ctxpopup, "more/default");
	evas_object_smart_callback_add(ctxpopup, "dismissed",
			__dismissed_more_ctxpopup_cb, ad);
	evas_object_smart_callback_add(elm_object_top_widget_get(ctxpopup), "rotation,changed",
			__rotate_more_ctxpopup_cb, ad);
	evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_DEL,
			__ctx_delete_more_ctxpopup_cb, ad->naviframe);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
			ELM_CTXPOPUP_DIRECTION_DOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	__ctx_move_more_ctxpopup(ctxpopup, ad);
	elm_ctxpopup_item_append(ctxpopup, STR_RENAME_DEVICE_HEADER,
			NULL, _gl_rename_device_sel, ad);

	evas_object_show(ctxpopup);

	ad->ctxpopup = ctxpopup;

	__MOBILE_AP_FUNC_EXIT__;
}
#endif

void _main_draw_contents(mh_appdata_t *ad)
{
	INFO("+\n");

	Elm_Object_Item *navi_item;

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

	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK,
			eext_naviframe_back_cb, NULL);
#if 0 /* device rename not supported */
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE,
			__create_ctxpopup_more_button, ad);
#endif

	evas_object_smart_callback_add(ad->main.back_btn, "clicked", __back_btn_cb, (void *)ad);
	elm_object_focus_allow_set(ad->main.back_btn, EINA_FALSE);

	navi_item = elm_naviframe_item_push(ad->naviframe, IDS_TETH,
				ad->main.back_btn, NULL, ad->main.genlist, NULL);
	elm_object_item_domain_text_translatable_set(navi_item, PKGNAME, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(navi_item, __back_btn_cb, (void *)ad);
	ad->navi_item = navi_item;
	g_ad = ad;
	INFO("-\n");
	return;
}
