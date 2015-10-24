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

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

//#include <setting-cfg.h>

#include "mobile_hotspot.h"
#include "mh_view_main.h"
#include "mh_func_onoff.h"
#include "mh_common_utility.h"
#include "mh_popup.h"
#include "mh_string.h"
#include "mh_view_wifi_setup.h"

//UG_MODULE_API int setting_plugin_search_init(app_control_h app_control, void * priv, char ** domainname);

static Evas_Object *create_content(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	ad->naviframe = _create_naviframe(ad->layout);
	_main_draw_contents(ad);

	__MOBILE_AP_FUNC_EXIT__;

	return ad->naviframe;
}

static void __set_callbacks(tethering_h handle, void *user_data)
{
	DBG("+\n");

	int ret;

	ret = tethering_set_enabled_cb(handle, TETHERING_TYPE_ALL,
			_enabled_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_set_enabled_cb [0x%X]\n", ret);

	ret = tethering_set_enabled_cb(handle, TETHERING_TYPE_RESERVED,
			_enabled_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_set_enabled_cb [0x%X]\n", ret);

	ret = tethering_wifi_set_passphrase_changed_cb(handle,
			_passphrase_changed_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_set_passphrase_changed_cb [0x%X]\n", ret);

	ret = tethering_wifi_set_ssid_visibility_changed_cb(handle,
			_visibility_changed_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_set_ssid_visibility_changed_cb [0x%X]\n", ret);

	ret = tethering_wifi_set_security_type_changed_cb(handle,
			_security_type_changed_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_set_security_type_changed_cb [0x%X]\n", ret);

	ret = tethering_set_disabled_cb(handle, TETHERING_TYPE_ALL,
			_disabled_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_set_disabled_cb [0x%X]\n", ret);

	ret = tethering_set_disabled_cb(handle, TETHERING_TYPE_RESERVED,
			_disabled_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_set_disabled_cb [0x%X]\n", ret);

	ret = tethering_set_connection_state_changed_cb(handle,
			TETHERING_TYPE_ALL,
			_connection_changed_cb, user_data);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_set_connection_state_changed_cb [0x%X]\n", ret);

	ret = wifi_set_device_state_changed_cb(_wifi_state_changed_cb, user_data);
	if (ret != WIFI_ERROR_NONE)
		ERR("wifi_set_device_state_changed_cb [0x%X]\n", ret);
	vconf_notify_key_changed(VCONFKEY_SETAPPL_DEVICE_NAME_STR,
				_device_name_changed_cb, user_data);

	vconf_notify_key_changed(VCONFKEY_NETWORK_CELLULAR_STATE,
			_handle_network_cellular_state_changed_cb, user_data);

	vconf_notify_key_changed(VCONFKEY_SYSMAN_USB_STATUS,
			_handle_usb_status_change, (void *)user_data);

	vconf_notify_key_changed(VCONF_KEY_MOBILEAP_SYSPOPUP_RESPONSE,
			_handle_mobileap_syspopup_popup_response, (void *)user_data);

	DBG("-\n");
}

static void __unset_callbacks(tethering_h handle)
{
	DBG("+\n");

	int ret;

	ret = wifi_unset_device_state_changed_cb();
	if (ret != WIFI_ERROR_NONE)
		ERR("wifi_unset_device_state_changed_cb [0x%X]\n", ret);

	ret = tethering_unset_connection_state_changed_cb(handle,
			TETHERING_TYPE_ALL);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_unset_connection_state_changed_cb[0x%X]\n", ret);

	ret = tethering_unset_disabled_cb(handle, TETHERING_TYPE_RESERVED);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_unset_disabled_cb [0x%X]\n", ret);

	ret = tethering_unset_disabled_cb(handle, TETHERING_TYPE_ALL);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_unset_disabled_cb [0x%X]\n", ret);

	ret = tethering_wifi_unset_security_type_changed_cb(handle);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_unset_security_type_changed_cb [0x%X]\n", ret);

	ret = tethering_wifi_unset_ssid_visibility_changed_cb(handle);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_unset_ssid_visibility_changed_cb [0x%X]\n", ret);

	ret = tethering_wifi_unset_passphrase_changed_cb(handle);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_unset_passphrase_changed_cb [0x%X]\n", ret);

	ret = tethering_unset_enabled_cb(handle, TETHERING_TYPE_RESERVED);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_unset_enabled [0x%X]\n", ret);

	ret = tethering_unset_enabled_cb(handle, TETHERING_TYPE_ALL);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_unset_enabled [0x%X]\n", ret);

	vconf_ignore_key_changed(VCONFKEY_SETAPPL_DEVICE_NAME_STR,
			_device_name_changed_cb);

	vconf_ignore_key_changed(VCONFKEY_NETWORK_CELLULAR_STATE,
			_handle_network_cellular_state_changed_cb);

	vconf_ignore_key_changed(VCONFKEY_SYSMAN_USB_STATUS, _handle_usb_status_change);

	vconf_ignore_key_changed(VCONF_KEY_MOBILEAP_SYSPOPUP_RESPONSE,
			_handle_mobileap_syspopup_popup_response);

	DBG("-\n");
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode,
		app_control_h app_control, void *priv)
{
	DBG("+\n");

	if (!ug || !priv) {
		ERR("The param is NULL\n");
		return NULL;
	}

	if (mode != UG_MODE_FULLVIEW) {
		ERR("Only Fullview is supported\n");
		return NULL;
	}

	Evas_Object *layout;
	Evas_Object *content;
	mh_ugdata_t *ugd;
	mh_appdata_t *ad;
	int ret;

	bindtextdomain(MH_TEXT_DOMAIN, MH_LOCALEDIR);
	dgettext(PACKAGE, LOCALEDIR);

	ad = (mh_appdata_t *)malloc(sizeof(mh_appdata_t));
	if (ad == NULL) {
		ERR("Error!!! failed to allocate memory()\n");
		return NULL;
	}
	memset(ad, 0x0, sizeof(mh_appdata_t));

	ugd = (mh_ugdata_t *)priv;
	ugd->ad = ad;
	ugd->ug = ug;
	ad->gadget = ugd;

	_send_signal_qp(QP_SIGNAL_PROGRESS_RESET);

	ecore_imf_init();

	ret = tethering_create(&ad->handle);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_create is failed : %d\n", ret);
		free(ad);
		ugd->ad = NULL;
		return NULL;
	}

	ad->win = ug_get_parent_layout(ug);
	if (!ad->win) {
		ERR("ad->win is NULL\n");
		free(ad);
		ugd->ad = NULL;
		return NULL;
	}

	/* set rotation */
	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = {0, 90, 180, 270};
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	layout = _create_win_layout(ad);
	if (layout == NULL) {
		ERR("_create_win_layout is failed\n");
		free(ad);
		ugd->ad = NULL;
		return NULL;
	}

	content = create_content(ad);
	if (content == NULL) {
		ERR("create_content is failed\n");
		free(ad);
		ugd->ad = NULL;
		return NULL;
	}

	elm_object_part_content_set(layout, "elm.swallow.content", content);
	evas_object_show(layout);
	ret = connection_create(&ad->conn_handle);
	if (ret != CONNECTION_ERROR_NONE) {
		ERR("connection_create() is failed : %d\n", ret);
	}

	ret = wifi_initialize();
	if (ret != WIFI_ERROR_NONE) {
		ERR("wifi_initialize() is failed : %d\n", ret);
	}

	__set_callbacks(ad->handle, (void *)ad);

	DBG("-\n");
	return layout;
}

static void on_start(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	DBG("+\n");

	char * viewtype = NULL;

	mh_ugdata_t *ugd = (mh_ugdata_t *)priv;
	mh_appdata_t *ad = ugd->ad;

	ad->is_foreground = true;

	if (_get_list_clients_count(ad) > 0) {
#ifdef TETHERING_DATA_USAGE_SUPPORT
		_start_update_data_packet_usage(ad);
#endif
		if (ad->connected_device.navi_it) {
			_start_update_device_conn_time(ad);
		}
	}

	if (tethering_is_enabled(NULL, TETHERING_TYPE_RESERVED) == true) {
		DBG("MobileAP is turned on\n");
		_prepare_popup(MH_POPUP_WIFI_AP_OFF, STR_WIFI_AP_CONTROLLED_ANOTHER_APP);
		_create_popup(ad);
	}

	app_control_get_extra_data(app_control, "viewtype", &viewtype);

	if (viewtype != NULL) {
		if(strcmp(viewtype, "wifisettings") == 0)
			mh_draw_wifi_setup_view(ad);
		g_free(viewtype);
	}

	DBG("-\n");
	return;
}

static void on_pause(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	DBG("+\n");

	mh_ugdata_t *ugd = (mh_ugdata_t *)priv;
	mh_appdata_t *ad = ugd->ad;

#ifdef TETHERING_DATA_USAGE_SUPPORT
	_stop_update_data_packet_usage(ad);
#endif
	_stop_update_device_conn_time(ad);
	ad->is_foreground = false;

	DBG("-\n");
}

static void on_resume(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	DBG("+\n");

	mh_ugdata_t *ugd = (mh_ugdata_t *)priv;
	mh_appdata_t *ad = ugd->ad;
	Elm_Object_Item *item = ad->main.device_item;
	GSList *l = NULL;

	ad->is_foreground = true;

	if (item && elm_genlist_item_expanded_get(item)) {
		for (l = ad->client_list; l != NULL; l = g_slist_next(l) ) {
			item = elm_genlist_item_next_get(item);
			elm_genlist_item_fields_update(item, "elm.text", ELM_GENLIST_ITEM_FIELD_TEXT);
		}
	}

	if (_get_list_clients_count(ad) > 0) {
#ifdef TETHERING_DATA_USAGE_SUPPORT
		_start_update_data_packet_usage(ad);
#endif
		if (ad->connected_device.navi_it) {
			_start_update_device_conn_time(ad);
		}
	}
	DBG("-\n");
}

static void on_destroy(ui_gadget_h ug, app_control_h app_control, void *priv)
{
	DBG("+\n");

	if (priv == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_ugdata_t *ugd = priv;
	mh_appdata_t *ad = ugd->ad;
	int ret = 0;

	if (ad == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	__unset_callbacks(ad->handle);

#ifdef TETHERING_DATA_USAGE_SUPPORT
	_stop_update_data_packet_usage(ad);
#endif
	_stop_update_device_conn_time(ad);

	if (vconf_set_int(VCONF_MOBILE_AP_CONNECT_USB_POPUP_STATUS, 0) < 0) {
		ERR("vconf_set_int is failed\n");
	}

	ret = wifi_deinitialize();
	if (ret != WIFI_ERROR_NONE) {
		ERR("wifi_deinitialize() is failed : %d\n", ret);
	}

	ret = connection_destroy(ad->conn_handle);
	if (ret != CONNECTION_ERROR_NONE) {
		ERR("connection_destroy() is failed : %d\n", ret);
	}

	ret = tethering_destroy(ad->handle);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_destroy() is failed : %d\n", ret);
	}

	if (ad->layout == NULL) {
		ERR("ad->layout is NULL\n");
		free(ugd->ad);
		ugd->ad = NULL;
		return;
	}

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	if(ad->ps_recheck_timer_id > 0) {
		g_source_remove(ad->ps_recheck_timer_id);
		ad->ps_recheck_timer_id = 0;
	}

	evas_object_del(ad->bg);
	ad->bg = NULL;

	evas_object_del(ad->layout);
	ad->layout = NULL;

	_main_free_genlist_itc(ad);

	ecore_imf_shutdown();

	free(ugd->ad);
	ugd->ad = NULL;

	DBG("-\n");
	return;
}

static void on_message(ui_gadget_h ug, app_control_h msg,
		app_control_h app_control, void *priv)
{
}

static void on_event(ui_gadget_h ug, enum ug_event event,
		app_control_h app_control, void *priv)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (!ug || !priv) {
		ERR("The param is NULL\n");
		return;
	}

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		break;
	case UG_EVENT_LOW_BATTERY:
		break;
	case UG_EVENT_LANG_CHANGE:
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		DBG("UG_EVENT_ROTATE_PORTRAIT[_UPSIDEDOWN]\n");
#if 0 /* device rename not supported */
		_rotate_adjust_rename_popup();
#endif
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		DBG("UG_EVENT_ROTATE_LANDSCAPE[_UPSIDEDOWN]\n");
#if 0 /* device rename not supported */
		_rotate_adjust_rename_popup();
#endif
		break;
	default:
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event,
		app_control_h app_control, void *priv)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (priv == NULL || ug == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_ugdata_t *ugd = (mh_ugdata_t *)priv;
	mh_appdata_t *ad = ugd->ad;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	switch (event) {
	case UG_KEY_EVENT_END:
		DBG("UG_KEY_EVENT_END is received : %p\n", ad->popup);
		if (NULL == ad->popup)
			ug_destroy_me(ug);
		else
			_destroy_popup(ad);
		break;

	default:
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	DBG("+\n");

	if (!ops) {
		ERR("The param is NULL\n");
		return -1;
	}

	mh_ugdata_t *ugd;
	ugd = calloc(1, sizeof(mh_ugdata_t));
	if (!ugd) {
		ERR("Quit : calloc failed(ugd)\n");
		return -1;
	}

	ops->create = on_create;
	ops->start = on_start;
	ops->pause = on_pause;
	ops->resume = on_resume;
	ops->destroy = on_destroy;
	ops->message = on_message;
	ops->event = on_event;
	ops->key_event = on_key_event;
	ops->priv = ugd;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	DBG("+\n");

	if (!ops) {
		ERR("The param is NULL\n");
		return;
	}

	mh_ugdata_t *ugd = (mh_ugdata_t *)ops->priv;

	if (ugd)
		free(ugd);
}

static void __mh_reset_vconf(tethering_h handle)
{
	int ret = 0;

	ret = vconf_set_int(VCONF_MOBILE_AP_WIFI_POPUP_CHECKBOX_STATUS, 0);
	if (ret != 0)
		ERR("vconf_set_int failed\n");

	ret = vconf_set_int(VCONF_MOBILE_AP_BT_POPUP_CHECKBOX_STATUS, 0);
	if (ret != 0)
		ERR("vconf_set_int failed\n");

	ret = vconf_set_int(VCONF_MOBILE_AP_USB_POPUP_CHECKBOX_STATUS, 0);
	if (ret != 0)
		ERR("vconf_set_int failed\n");

	ret = vconf_set_int(VCONF_MOBILE_AP_PREV_WIFI_STATUS, 0);
	if (ret != 0)
		ERR("vconf_set_int failed\n");

	ret = vconf_set_int(VCONF_MOBILE_AP_CONNECT_USB_POPUP_STATUS, 0);
	if (ret != 0)
		ERR("vconf_set_int is failed\n");

	ret = tethering_wifi_set_ssid_visibility(handle, true);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_set_ssid_visibility failed\n");

	ret = tethering_wifi_set_security_type(handle, TETHERING_WIFI_SECURITY_TYPE_WPA2_PSK);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_set_security_type failed\n");

	return;
}

UG_MODULE_API int setting_plugin_reset(app_control_h app_control, void *priv)
{
	DBG("+\n");

	int ret = -1;
	tethering_h handle = NULL;

	if (tethering_create(&handle) != TETHERING_ERROR_NONE) {
		ERR("tethering_create failed\n");
		return -1;
	}

	if (tethering_is_enabled(handle, TETHERING_TYPE_USB) ||
			tethering_is_enabled(handle, TETHERING_TYPE_WIFI) ||
			tethering_is_enabled(handle, TETHERING_TYPE_BT)) {
		ret = tethering_disable(handle, TETHERING_TYPE_ALL);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("tethering_disable failed : %d\n", ret);
			tethering_destroy(handle);
			return -1;
		}

	}

	__mh_reset_vconf(handle);

	tethering_destroy(handle);

	return 0;
}

/* Below code is commented as search in setting is no more supported in kiran */

#if 0
UG_MODULE_API int setting_plugin_search_init(app_control_h app_control, void * priv, char ** domainname)
{
	*domainname = strdup(PACKAGE":/usr/apps/ug-setting-mobileap-efl/res/locale");

	Eina_List ** pplist = (Eina_List**)priv;
	void * node = NULL;

	node = setting_plugin_search_item_add("IDS_MOBILEAP_MBODY_USB_TETHERING", "viewtype:frontpage", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	node = setting_plugin_search_item_add("IDS_MOBILEAP_MBODY_WI_FI_TETHERING", "viewtype:frontpage", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	node = setting_plugin_search_item_add("IDS_MOBILEAP_BODY_BLUETOOTH_TETHERING", "viewtype:frontpage", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	node = setting_plugin_search_item_add("IDS_MOBILEAP_MBODY_WI_FI_TETHERING_SETTINGS", "viewtype:wifisettings", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	node = setting_plugin_search_item_add("IDS_MOBILEAP_BODY_HIDE_MY_DEVICE", "viewtype:wifisettings", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	node = setting_plugin_search_item_add("IDS_MOBILEAP_BODY_SECURITY", "viewtype:wifisettings", NULL, 5, NULL);
	*pplist = eina_list_append(*pplist, node);

	return 0;
}
#endif
