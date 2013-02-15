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

#include "mh_common_utility.h"
#include "mobile_hotspot.h"

static void __one_btn_popup_resp(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	evas_object_del(ad->popup);
	ad->popup = NULL;

	DBG("popup_type : %d\n", ad->popup_type);
	switch (ad->popup_type) {
	case MH_POP_USB_CONNECT:
		_update_usb_item(ad, MH_STATE_NONE);
		vconf_ignore_key_changed(VCONFKEY_SETAPPL_USB_MODE_INT,
				_handle_usb_mode_change);
		break;

	case MH_POP_INFORMATION:
		break;

	case MH_POP_WIFI_PASSWORD_SHORT:
		if (ad->setup.pw_entry == NULL)
			break;

		evas_object_show(ad->setup.pw_entry);
		elm_object_focus_set(ad->setup.pw_entry, EINA_TRUE);
		break;

	default:
		DBG("Unknown popup_type : %d\n", ad->popup_type);
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void __alert_popup_resp(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	evas_object_del(ad->popup);
	ad->popup = NULL;

	DBG("popup_type : %d\n", ad->popup_type);
	switch (ad->popup_type) {
	case MH_POP_INFORMATION_WO_BUTTON:
		break;

	default:
		DBG("Unknown popup_type : %d\n", ad->popup_type);
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void __popup_resp_yes(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	bool wifi_state;
	int ret = 0;

	evas_object_del(ad->popup);
	ad->popup = NULL;

	DBG("popup_type : %d\n", ad->popup_type);
	switch (ad->popup_type) {
	case MH_POP_WIFI_ON_CONF:
		wifi_is_activated(&wifi_state);
		if (wifi_state == true) {
			if (_turn_off_wifi(ad) != 0) {
				ERR("_turn_off_wifi is failed\n");
				_update_wifi_item(ad, MH_STATE_NONE);
			}
		} else if (_is_wifi_direct_on() == true) {
			if (_turn_off_wifi_direct(ad) != 0) {
				ERR("_turn_off_wifi_direct is failed\n");
				_update_wifi_item(ad, MH_STATE_NONE);
			}
		} else {
			ret = tethering_enable(ad->handle, TETHERING_TYPE_WIFI);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("wifi tethering on is failed : %d\n", ret);
				_update_wifi_item(ad, MH_STATE_NONE);
			}
		}
		break;

	case MH_POP_WIFI_OFF_CONF:
		ret = tethering_disable(ad->handle, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("wifi tethering off is failed : %d\n", ret);
			_update_wifi_item(ad, MH_STATE_NONE);
		}
		break;

	case MH_POP_BT_ON_CONF:
		ret = tethering_enable(ad->handle, TETHERING_TYPE_BT);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("Error enable bt tethering : %d\n", ret);
			_update_bt_item(ad, MH_STATE_NONE);
		}
		break;

	case MH_POP_USB_ON_CONF:
		if (_get_vconf_usb_state() != VCONFKEY_SYSMAN_USB_AVAILABLE) {
			_prepare_popup(ad, MH_POP_USB_CONNECT,
					_("IDS_MOBILEAP_POP_CONNECT_USB_CABLE"));
			_create_popup(ad);
			vconf_notify_key_changed(VCONFKEY_SETAPPL_USB_MODE_INT,
					_handle_usb_mode_change, (void *)ad);
			break;
		}

		_prepare_popup(ad, MH_POP_USB_ON_PREVCONN_CONF,
				_("IDS_MOBILEAP_POP_ENABLING_USB_TETHERING_WILL_DISCONNECT_PREVIOUS_USB_CONNECTION"));
		_create_popup(ad);
		break;

	case MH_POP_USB_ON_PREVCONN_CONF:
		ret = tethering_enable(ad->handle, TETHERING_TYPE_USB);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("Error enable usb tethering : %d\n", ret);
			_update_usb_item(ad, MH_STATE_NONE);
		}
		break;

	case MH_POP_ENTER_TO_WIFI_SETUP_CONF:
		_update_wifi_item(ad, MH_STATE_PROCESS);
		ret = tethering_disable(ad->handle, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("Wi-Fi tethering off is failed : %d\n", ret);
			_update_wifi_item(ad, MH_STATE_NONE);
		} else
			ad->main.need_recover_wifi_tethering = true;

		mh_draw_wifi_setup_view(ad);
		break;

	default:
		DBG("Unknown popup_type : %d\n", ad->popup_type);
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void __popup_resp_no(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	evas_object_del(ad->popup);
	ad->popup = NULL;

	DBG("popup_type : %d\n", ad->popup_type);
	switch (ad->popup_type) {
	case MH_POP_WIFI_ON_CONF:
		_update_wifi_item(ad, MH_STATE_NONE);
		break;

	case MH_POP_WIFI_OFF_CONF:
		_update_wifi_item(ad, MH_STATE_NONE);
		break;

	case MH_POP_BT_ON_CONF:
		_update_bt_item(ad, MH_STATE_NONE);
		break;

	case MH_POP_USB_ON_CONF:
		_update_usb_item(ad, MH_STATE_NONE);
		break;

	case MH_POP_USB_ON_PREVCONN_CONF:
		_update_usb_item(ad, MH_STATE_NONE);
		break;

	case MH_POP_ENTER_TO_WIFI_SETUP_CONF:
		break;

	default:
		DBG("Unknown popup_type : %d\n", ad->popup_type);
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static bool _count_connected_clients_cb(tethering_client_h client, void *user_data)
{
	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return true;
	}

	int *count = (int *)user_data;

	*count += 1;

	return true;
}

void _prepare_popup(mh_appdata_t *ad, int type, const char *str)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL || str == NULL) {
		ERR("param is NULL : ad[%x], str[%x]\n", ad, str);
		return;
	}

	int nLen = 0;

	nLen = strlen(str);
	if (sizeof(ad->popup_string) <= nLen)
		ERR("We should make the message[%s] less than %d",
				str, sizeof(ad->popup_string));

	ad->popup_type = type;
	snprintf(ad->popup_string, sizeof(ad->popup_string), "%s", str);

	__MOBILE_AP_FUNC_EXIT__;
}

Eina_Bool _create_popup(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Object *btn = NULL;

	if (ad == NULL) {
		ERR("The param is NULL\n");
		return 0;
	}

	if (ad->popup != NULL) {
		DBG("Pop-up already exists. Delete it.\n");
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	DBG("Create_popup %d\n", ad->popup_type);
	switch (ad->popup_type) {
	case MH_POP_WIFI_ON_CONF:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_YES"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_yes, (void *)ad);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_NO"));
		elm_object_part_content_set(ad->popup, "button2", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_no, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_WIFI_OFF_CONF:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_YES"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_yes, (void *)ad);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_NO"));
		elm_object_part_content_set(ad->popup, "button2", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_no, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_USB_CONNECT:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_CANCEL"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__one_btn_popup_resp, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_BT_ON_CONF:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_OK"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_yes, (void *)ad);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_CANCEL"));
		elm_object_part_content_set(ad->popup, "button2", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_no, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_USB_ON_CONF:
	case MH_POP_USB_ON_PREVCONN_CONF:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_OK"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_yes, (void *)ad);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_CANCEL"));
		elm_object_part_content_set(ad->popup, "button2", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_no, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_INFORMATION:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_POP_CLOSE"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__one_btn_popup_resp, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_WIFI_PASSWORD_SHORT:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_POP_CLOSE"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__one_btn_popup_resp, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_INFORMATION_WO_BUTTON:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		elm_popup_timeout_set(ad->popup, MH_POPUP_TIMEOUT);
		evas_object_smart_callback_add(ad->popup, "timeout",
				__alert_popup_resp, (void *)ad);
		evas_object_smart_callback_add(ad->popup, "block,clicked",
				__alert_popup_resp, (void *)ad);

		evas_object_show(ad->popup);
		break;

	case MH_POP_ENTER_TO_WIFI_SETUP_CONF:
		ad->popup = elm_popup_add(ad->win);
		evas_object_size_hint_weight_set(ad->popup,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		elm_object_text_set(ad->popup, ad->popup_string);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_OK"));
		elm_object_part_content_set(ad->popup, "button1", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_yes, (void *)ad);

		btn = elm_button_add(ad->popup);
		elm_object_style_set(btn, "popup_button/default");
		elm_object_text_set(btn, S_("IDS_COM_SK_CANCEL"));
		elm_object_part_content_set(ad->popup, "button2", btn);
		evas_object_smart_callback_add(btn, "clicked",
				__popup_resp_no, (void *)ad);

		evas_object_show(ad->popup);
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return TRUE;
}

void _destroy_popup(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	ad->popup_type = MH_POPUP_NONE;

	__MOBILE_AP_FUNC_EXIT__;

	return;
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

void _handle_usb_mode_change(keynode_t *key, void *data)
{
	mh_appdata_t *ad = (mh_appdata_t *)data;
	int vconf_key = 0;

	if (!data) {
		ERR("The param is NULL\n");
		return;
	}

	if (vconf_keynode_get_type(key) != VCONF_TYPE_INT) {
		ERR("Invalid vconf key\n");
		return;
	}

	vconf_key = vconf_keynode_get_int(key);
	DBG("vconf_key : %d\n", vconf_key);
	if (vconf_key == SETTING_USB_NONE_MODE) {
		DBG("usb-utilties is processing mode change\n");
		return;
	}

	vconf_ignore_key_changed(VCONFKEY_SETAPPL_USB_MODE_INT,
				_handle_usb_mode_change);

	evas_object_del(ad->popup);
	ad->popup = NULL;

	if (tethering_enable(ad->handle, TETHERING_TYPE_USB) != TETHERING_ERROR_NONE) {
		DBG("Error enable usb tethering\n");
		_update_usb_item(ad, MH_STATE_NONE);
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

Eina_Bool _get_no_of_connected_device(tethering_h handle, int *no, tethering_type_e type)
{
	if (handle == NULL || no == NULL)
		return FALSE;

	tethering_foreach_connected_clients(handle, type,
			_count_connected_clients_cb, (void *)no);

	return TRUE;
}
