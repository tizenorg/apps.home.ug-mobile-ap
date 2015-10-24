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

#include <net_connection.h>
#include <dbus/dbus.h>
#include <gio/gio.h>

#include "mh_common_utility.h"
#include "mobile_hotspot.h"

void _handle_network_cellular_state_changed_cb(keynode_t *key, void *data)
{
	if (key == NULL || data == NULL) {
		ERR("Parameter is NULL\n");
		return;
	}
	mh_appdata_t *ad = (mh_appdata_t *)data;
	int vconf_key = 0;

	if (vconf_keynode_get_type(key) != VCONF_TYPE_INT) {
		ERR("Invalid vconf key type\n");
		return;
	}

	vconf_key = vconf_keynode_get_int(key);
	SDBG("key = %s, value = %d(int)\n",
			vconf_keynode_get_name(key), vconf_key);

	if (vconf_key != VCONFKEY_NETWORK_CELLULAR_FLIGHT_MODE)
		return;

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	if (ad->is_wifi_teth_enabling) {
		_update_wifi_item(ad, MH_STATE_NONE);
		ad->is_wifi_teth_enabling = false;
	}
	if (ad->is_bt_teth_enabling) {
		_update_bt_item(ad, MH_STATE_NONE);
		ad->is_bt_teth_enabling = false;
	}
	if (ad->is_usb_teth_enabling) {
		_update_usb_item(ad, MH_STATE_NONE);
		ad->is_usb_teth_enabling = false;
	}
	return;
}

void _device_name_changed_cb(keynode_t *key, void *data)
{
	if (key == NULL || data == NULL) {
		ERR("Parameter is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	char *dev_name = NULL;

	if (vconf_keynode_get_type(key) != VCONF_TYPE_STRING) {
		ERR("Invalid vconf key type\n");
		return;
	}
	dev_name = vconf_keynode_get_str(key);
	if (ad->setup.name_item != NULL) {
		elm_genlist_item_update(ad->setup.name_item);
	}
	if (ad->main.help_item != NULL) {
		elm_genlist_item_update(ad->main.help_item);
	}
	if (ad->rename_popup) {
		elm_entry_entry_set(ad->rename_entry, dev_name);
		elm_entry_cursor_end_set(ad->rename_entry);
	}
	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI) {
		_update_wifi_item(ad, MH_STATE_PROCESS);
	}
	return;
}

Evas_Object *_create_progressbar(Evas_Object *parent, const char *style)
{
	if (parent == NULL || style == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	Evas_Object *progressbar;

	progressbar = elm_progressbar_add(parent);
	if (progressbar == NULL) {
		ERR("progressbar is NULL\n");
		return NULL;
	}

	elm_object_style_set(progressbar, style);
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);

	return progressbar;
}

Evas_Object *_create_bg(Evas_Object *parent, const char *style)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (parent == NULL || style == NULL) {
		ERR("The param is NULL\n");
		return NULL;
	}

	Evas_Object *bg = NULL;

	bg = elm_bg_add(parent);
	if (bg == NULL) {
		ERR("bg is NULL\n");
		return NULL;
	}

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_style_set(bg, style);
	evas_object_show(bg);

	return bg;
}

Evas_Object *_create_win_layout(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad->win == NULL) {
		ERR("There is no main window\n");
		return NULL;
	}

	Evas_Object *layout;
	Evas_Object *bg;

	layout = elm_layout_add(ad->win);
	if (layout == NULL) {
		ERR("layout is NULL\n");
		return NULL;
	}

	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);

	bg = _create_bg(layout, "group_list");
	if (bg == NULL) {
		ERR("bg is NULL\n");
		evas_object_del(layout);
		return NULL;
	}
	elm_object_part_content_set(layout, "elm.swallow.bg", bg);

	evas_object_show(layout);

	ad->layout = layout;
	ad->bg = bg;

	__MOBILE_AP_FUNC_EXIT__;

	return layout;
}

Evas_Object *_create_naviframe(Evas_Object *parent)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (parent == NULL) {
		ERR("parent is NULL\n");
		return NULL;
	}

	Evas_Object *naviframe;

	naviframe = elm_naviframe_add(parent);
	if (naviframe == NULL) {
		ERR("naviframe is NULL\n");
		return NULL;
	}
	evas_object_show(naviframe);

	__MOBILE_AP_FUNC_EXIT__;

	return naviframe;
}

Evas_Object *_create_button(Evas_Object *parent, const char *text, const char *part,
		Evas_Smart_Cb func, void *user_data)
{
	if (parent == NULL || text == NULL || part == NULL || func == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	Evas_Object *btn;

	btn = elm_button_add(parent);
	elm_object_style_set(btn, "popup");
	elm_object_domain_translatable_text_set(btn, PACKAGE, text);
	elm_object_part_content_set(parent, part, btn);
	evas_object_smart_callback_add(btn, "clicked", func, user_data);
	evas_object_show(btn);

	return btn;
}

void _handle_mobileap_syspopup_popup_response(keynode_t *key, void *data)
{
	if (!data) {
		ERR("The param is NULL\n");
		return;
	}

	if (vconf_keynode_get_type(key) != VCONF_TYPE_INT) {
		ERR("Invalid vconf key\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	int vconf_key = 0;

	vconf_key = vconf_keynode_get_int(key);

	if (vconf_key) {
		if (vconf_set_int(VCONF_KEY_MOBILEAP_SYSPOPUP_RESPONSE, 0) < 0) {
			ERR("vconf_set_int is failed\n");
		}
		if (ad->type != TETHERING_TYPE_WIFI) {
			DBG("no need to handle user response\n");
			return;
		}
		if (ad->popup) {
			evas_object_del(ad->popup);
			ad->popup = NULL;
		}
		if (ad->popup_checkbox) {
			evas_object_del(ad->popup_checkbox);
			ad->popup_checkbox = NULL;
		}
		if (vconf_key == 1)
			_update_wifi_item(ad, MH_STATE_NONE);
	}
}

void _handle_usb_status_change(keynode_t *key, void *data)
{
	int vconf_key = 0;

	if (!data) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	if (vconf_keynode_get_type(key) != VCONF_TYPE_INT) {
		ERR("Invalid vconf key\n");
		return;
	}

	vconf_key = vconf_keynode_get_int(key);
	if (vconf_key == SETTING_USB_NONE_MODE) {
		return;
	}
	if (vconf_key != VCONFKEY_SYSMAN_USB_AVAILABLE) {
		if (ad->type == TETHERING_TYPE_USB && ad->popup) {
			evas_object_del(ad->popup);
			ad->popup = NULL;
			_update_tethering_item(ad, MH_STATE_NONE);
		}
		if (ad->main.usb_item)
			elm_object_item_disabled_set(ad->main.usb_item, EINA_TRUE);
	} else {
		if (ad->main.usb_item)
			elm_object_item_disabled_set(ad->main.usb_item, EINA_FALSE);
	}
}

int _get_vconf_hotspot_mode(void)
{
	int value = VCONFKEY_MOBILE_HOTSPOT_MODE_NONE;

	if (vconf_get_int(VCONFKEY_MOBILE_HOTSPOT_MODE, &value) < 0) {
		ERR("vconf_get_int is failed\n");
		return 0;
	}
	DBG("%s : %d\n", VCONFKEY_MOBILE_HOTSPOT_MODE, value);

	return value;
}

Eina_Bool _get_no_of_connected_device(mh_appdata_t *ad, int *no, tethering_type_e type)
{
	if (ad == NULL) {
		ERR("Invalid param\n");
		return EINA_FALSE;
	}

	if (ad->client_list == NULL) {
		*no = 0;
		return EINA_TRUE;
	}

	GSList *l = NULL;
	tethering_client_h handle;
	tethering_type_e type2;
	tethering_error_e ret;
	int count = 0;

	for (l = ad->client_list; l != NULL; l = g_slist_next(l) ) {
		handle = l->data;
		if (handle == NULL)
			continue;

		ret = tethering_client_get_tethering_type(handle, &type2);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("tethering_client_get_tethering_type is failed [0x%X]\n", ret);
			continue;
		}

		if (type != TETHERING_TYPE_ALL && type != type2)
			continue;

		count++;
	}
	*no = count;

	return EINA_TRUE;
}

void _append_list_client_handle(mh_appdata_t *ad, tethering_client_h client)
{
	if (ad == NULL || client == NULL) {
		ERR("Invalid param\n");
		return;
	}

	int ret;
	tethering_client_h handle;

	ret = tethering_client_clone(&handle, client);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("Unable to clone client handle");
		return;
	}

	ad->client_list = g_slist_append(ad->client_list, handle);

	return;
}

void _release_list_client_handle(mh_appdata_t *ad)
{
	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	if (ad->client_list == NULL)
		return;

	GSList *l = NULL;
	tethering_client_h handle;

	for (l = ad->client_list; l != NULL; l = g_slist_next(l) ) {
		handle = l->data;
		if (handle == NULL)
			continue;

		tethering_client_destroy(handle);
	}

	g_slist_free(ad->client_list);
	ad->client_list = NULL;

	return;
}

static int __find_mac_address(gconstpointer a, gconstpointer b)
{
	tethering_client_h handle = (tethering_client_h)a;
	const char *udn = (const char *)b;
	char *mac_addr;
	int ret;

	ret = tethering_client_get_mac_address(handle, &mac_addr);
	if (ret != TETHERING_ERROR_NONE)
		return -1;

	ret = g_ascii_strcasecmp(mac_addr, udn);

	if (mac_addr)
		free(mac_addr);

	return ret;
}

void _delete_list_client_handle(mh_appdata_t *ad, const char *mac_addr)
{
	if (ad == NULL || mac_addr == NULL) {
		ERR("Invalid param\n");
		return;
	}

	GSList *l = NULL;
	tethering_client_h handle = NULL;

	l = g_slist_find_custom(ad->client_list, mac_addr, __find_mac_address);
	if (!l) {
		ERR("Not found\n");
		return;
	}

	handle = (tethering_client_h)l->data;
	ad->client_list = g_slist_delete_link(ad->client_list, l);
	if (handle == NULL)
		return;

	tethering_client_destroy(handle);
	return;
}

int _get_list_clients_count(mh_appdata_t *ad)
{
	int client_list_counts;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return 0;
	}

	client_list_counts = g_slist_length(ad->client_list);
	if (client_list_counts > TETHERING_WIFI_MAX_CONNECTED_STA) {
		INFO("client_list_counts = [%d]\n", client_list_counts);
		return TETHERING_WIFI_MAX_CONNECTED_STA;
	} else
		return client_list_counts;
}

void _get_list_clients_informations(mh_appdata_t *ad)
{
	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	GSList *l = NULL;
	tethering_client_h handle;
	int i = 1;
	int ret;
	tethering_type_e type = 0;
	char *name = NULL;
	char *mac_addr = NULL;

	for (l = ad->client_list; l != NULL; l = g_slist_next(l), i++) {
		handle = (tethering_client_h)l->data;

		ret = tethering_client_get_name(handle, &name);
		if (ret != TETHERING_ERROR_NONE)
			ERR("tethering_client_get_name is failed : %d\n", ret);

		ret = tethering_client_get_mac_address(handle, &mac_addr);
		if (ret != TETHERING_ERROR_NONE)
			ERR("tethering_client_get_mac_address is failed : %d\n", ret);

		ret = tethering_client_get_tethering_type(handle, &type);
		if (ret != TETHERING_ERROR_NONE)
			ERR("tethering_client_get_tethering_type is failed : %d\n", ret);
		if (name) {
			SDBG("Client[%d] : %s\n", i, name);
			free(name);
			name = NULL;
		}

		if (mac_addr) {
			SDBG("MAC[%d] : %s\n", i, mac_addr);
			free(mac_addr);
			mac_addr = NULL;
		}

		SDBG("Type[%d] : %d\n", i, type);
	}

	return;
}

void _free_genlist_item(Elm_Object_Item **item)
{
	if (*item == NULL) {
		return;
	}

	elm_object_item_del(*item);
	*item = NULL;
}

void _free_genlist_itc(Elm_Genlist_Item_Class **itc)
{
	if (*itc == NULL) {
		return;
	}

	elm_genlist_item_class_free(*itc);
	*itc = NULL;
}

int _get_sim_state(void)
{
	int value = 0;

	if (vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &value) < 0) {
		ERR("vconf_get_int is failed\n");
	}
	DBG("%s : %d\n", VCONFKEY_TELEPHONY_SIM_SLOT, value);
	if (value == VCONFKEY_TELEPHONY_SIM_INSERTED) {
		return value;
	}

#if defined TIZEN_DUALSIM_ENABLE
	if (vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT2, &value) < 0) {
		ERR("vconf_get_int is failed\n");;
	}
	DBG("%s : %d\n", VCONFKEY_TELEPHONY_SIM_SLOT2, value);
#endif

	return value;
}

connection_cellular_state_e _get_cellular_state(void)
{
	int ret;
	connection_h handle = NULL;
	connection_cellular_state_e cellular_state;

	ret = connection_create(&handle);
	if (ret != CONNECTION_ERROR_NONE) {
		ERR("Connection create failed");
		return CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	}
	ret = connection_get_cellular_state(handle, &cellular_state);
	if (ret != CONNECTION_ERROR_NONE) {
		ERR("connection_get_cellular_state() is failed : %d\n");
		connection_destroy(handle);
		return CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	}

	connection_destroy(handle);
	return cellular_state;
}

int _get_checkbox_status(tethering_type_e type)
{
	int value = 0;
	char *vconf_key = NULL;

	if (type == TETHERING_TYPE_WIFI)
		vconf_key = VCONF_MOBILE_AP_WIFI_POPUP_CHECKBOX_STATUS;
	else if (type == TETHERING_TYPE_BT)
		vconf_key = VCONF_MOBILE_AP_BT_POPUP_CHECKBOX_STATUS;
	else if (type == TETHERING_TYPE_USB)
		vconf_key = VCONF_MOBILE_AP_USB_POPUP_CHECKBOX_STATUS;
	else
		return 0;
	if (vconf_get_int(vconf_key, &value) < 0) {
		ERR("vconf_get_int() is failed\n");
		return 0;
	}
	DBG("%s : %d\n", vconf_key, value);
	return value;
}

bool _set_checkbox_status(tethering_type_e type, int value)
{
	char *vconf_key = NULL;

	if (type == TETHERING_TYPE_WIFI)
		vconf_key = VCONF_MOBILE_AP_WIFI_POPUP_CHECKBOX_STATUS;
	else if (type == TETHERING_TYPE_BT)
		vconf_key = VCONF_MOBILE_AP_BT_POPUP_CHECKBOX_STATUS;
	else if (type == TETHERING_TYPE_USB)
		vconf_key = VCONF_MOBILE_AP_USB_POPUP_CHECKBOX_STATUS;
	else
		return false;

	if (vconf_set_int(vconf_key, value) < 0) {
		ERR("vconf_set_int is failed\n");
		return false;
	}
	return true;
}

bool _set_vconf_prev_wifi_state(bool value)
{
	if (vconf_set_bool(VCONF_MOBILE_AP_PREV_WIFI_STATUS, value) < 0) {
		ERR("vconf_set_bool failed\n");
		return false;
	}
	return true;
}

bool _get_vconf_prev_wifi_state()
{
	int value = 0;

	if (vconf_get_bool(VCONF_MOBILE_AP_PREV_WIFI_STATUS, &value) < 0) {
		ERR("vconf_get_bool is failed\n");
		return false;
	}
	DBG("%s : %d\n", VCONF_MOBILE_AP_PREV_WIFI_STATUS, value);

	return value ? true : false;
}

int _send_signal_qp(const char *cmd)
{
	DBG("+\n");

	if (cmd == NULL) {
		ERR("Invalid param");
		return -1;
	}

	GDBusConnection *conn = NULL;
	int ret = 0;
	GVariant *message = NULL;
	GError *err = NULL;

	DBG("Sent dbus signal : %s\n", cmd);

	conn = g_bus_get_sync(DBUS_BUS_SYSTEM, NULL, &err);
	if (err != NULL) {
		ERR("Failed connection to system bus[%s]", err->message);
		g_error_free(err);
		err = NULL;
		return -1;
	}
	message = g_variant_new("(ss)", "wifi_hotspot", cmd);
	g_dbus_connection_emit_signal(conn, NULL, "/Org/Tizen/Quickpanel",
			"org.tizen.quickpanel", "ACTIVITY", message, &err);
	if (err) {
		ERR("g_dbus_connection_emit_signal is failed and error is %s\n", err->message);
		g_error_free(err);
		ret = -1;
	}
	g_variant_unref(message);

	if (conn)
		g_object_unref(conn);

	DBG("-\n");
	return ret;
}
