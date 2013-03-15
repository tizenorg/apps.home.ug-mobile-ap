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

#include "mobile_hotspot.h"
#include "mh_view_main.h"
#include "mh_func_onoff.h"

static Evas_Object *create_content(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	ad->naviframe = _create_naviframe(ad->layout);
	ap_draw_contents(ad);

	__MOBILE_AP_FUNC_EXIT__;

	return ad->naviframe;
}

static void __set_callbacks(tethering_h handle, void *user_data)
{
	tethering_set_enabled_cb(handle, TETHERING_TYPE_USB, _enabled_cb, user_data);
	tethering_set_enabled_cb(handle, TETHERING_TYPE_WIFI, _enabled_cb, user_data);
	tethering_set_enabled_cb(handle, TETHERING_TYPE_BT, _enabled_cb, user_data);

	tethering_set_disabled_cb(handle, TETHERING_TYPE_USB, _disabled_cb, user_data);
	tethering_set_disabled_cb(handle, TETHERING_TYPE_WIFI, _disabled_cb, user_data);
	tethering_set_disabled_cb(handle, TETHERING_TYPE_BT, _disabled_cb, user_data);

	tethering_set_connection_state_changed_cb(handle, TETHERING_TYPE_USB, _connection_changed_cb, user_data);
	tethering_set_connection_state_changed_cb(handle, TETHERING_TYPE_WIFI, _connection_changed_cb, user_data);
	tethering_set_connection_state_changed_cb(handle, TETHERING_TYPE_BT, _connection_changed_cb, user_data);
}

static void __unset_callbacks(tethering_h handle)
{
	tethering_unset_enabled_cb(handle, TETHERING_TYPE_USB);
	tethering_unset_enabled_cb(handle, TETHERING_TYPE_WIFI);
	tethering_unset_enabled_cb(handle, TETHERING_TYPE_BT);

	tethering_unset_disabled_cb(handle, TETHERING_TYPE_USB);
	tethering_unset_disabled_cb(handle, TETHERING_TYPE_WIFI);
	tethering_unset_disabled_cb(handle, TETHERING_TYPE_BT);

	tethering_unset_connection_state_changed_cb(handle, TETHERING_TYPE_USB);
	tethering_unset_connection_state_changed_cb(handle, TETHERING_TYPE_WIFI);
	tethering_unset_connection_state_changed_cb(handle, TETHERING_TYPE_BT);
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode,
		service_h service, void *priv)
{
	__MOBILE_AP_FUNC_ENTER__;

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
		// TODO review this
		ERR("Error!!! failed to allocate memory()\n");
		return NULL;
	}
	memset(ad, 0x0, sizeof(mh_appdata_t));

	ugd = (mh_ugdata_t *)priv;
	ugd->ad = ad;
	ugd->ug = ug;
	ad->gadget= ugd;

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

	__MOBILE_AP_FUNC_EXIT__;
	return layout;
}

static void on_start(ui_gadget_h ug, service_h service, void *priv)
{
}

static void on_pause(ui_gadget_h ug, service_h service, void *priv)
{

}

static void on_resume(ui_gadget_h ug, service_h service, void *priv)
{

}

static void on_destroy(ui_gadget_h ug, service_h service, void *priv)
{
	__MOBILE_AP_FUNC_ENTER__;

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
	_stop_update_data_packet_usage(ad);

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

	ap_callback_del(ad);
	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	evas_object_del(ad->bg);
	ad->bg = NULL;

	evas_object_del(ad->layout);
	ad->layout = NULL;

	ecore_imf_shutdown();

	free(ugd->ad);
	ugd->ad = NULL;

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void on_message(ui_gadget_h ug, service_h msg,
		service_h service, void *priv)
{
}

static void on_event(ui_gadget_h ug, enum ug_event event,
		service_h service, void *priv)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (!ug || !priv) {
		ERR("The param is NULL\n");
		return;
	}

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		DBG("UG_EVENT_LOW_MEMORY\n");
		break;
	case UG_EVENT_LOW_BATTERY:
		DBG("UG_EVENT_LOW_BATTERY\n");
		break;
	case UG_EVENT_LANG_CHANGE:
		DBG("UG_EVENT_LANG_CHANGE\n");
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
		DBG("UG_EVENT_ROTATE_PORTRAIT\n");
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		DBG("UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN\n");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		DBG("UG_EVENT_ROTATE_LANDSCAPE\n");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		DBG("UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN\n");
		break;
	default:
		DBG("default\n");
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event,
		service_h service, void *priv)
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
		DBG("UG_KEY_EVENT_END is received  :  %p\n", ad->popup);
		if (NULL == ad->popup) {
			ug_destroy_me(ug);
			break;
		}

		if (ad->popup_type != MH_POP_INFORMATION_WO_BUTTON) {
			evas_object_del(ad->popup);
			ad->popup = NULL;
			ad->popup_type = MH_POPUP_NONE;
		}
		break;

	default:
		break;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	__MOBILE_AP_FUNC_ENTER__;

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

	__MOBILE_AP_FUNC_EXIT__;

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (!ops) {
		ERR("The param is NULL\n");
		return;
	}

	mh_ugdata_t *ugd = (mh_ugdata_t *)ops->priv;

	if (ugd)
		free(ugd);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __mh_reset_vconf(tethering_h handle)
{
	int ret = 0;

	ret = vconf_set_int(VCONF_MOBILE_AP_PREV_WIFI_STATUS, 0);
	if (ret != 0)
		ERR("vconf_set_int failed\n");

	ret = tethering_wifi_set_ssid_visibility(handle, true);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_set_ssid_visibility failed\n");

	ret = tethering_wifi_set_security_type(handle, TETHERING_WIFI_SECURITY_TYPE_NONE);
	if (ret != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_set_security_type failed\n");

	return;
}

UG_MODULE_API int setting_plugin_reset(service_h service, void *priv)
{
	__MOBILE_AP_FUNC_ENTER__;

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

	__MOBILE_AP_FUNC_EXIT__;

	return 0;
}
