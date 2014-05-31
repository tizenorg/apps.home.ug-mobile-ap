/*
* ug-mobile-ap
*
* Copyright 2012-2013  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://floralicense.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
#include <notification.h>
#include "mh_view_wifi_setup.h"

//#define SK_BACK_SUPPORT
static void __back_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __gl_realized(void *data, Evas_Object *obj, void *event_info);

static void __input_panel_event_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	if (data == NULL) {
		ERR("data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;

	ad->imf_state = value;

	if (st->navi_it == NULL) {
		DBG("naviframe item is not pushed yet\n");
		return;
	}

	if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW &&
			(ad->rotate_state == UG_EVENT_ROTATE_LANDSCAPE ||
			 ad->rotate_state == UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN)) {
		DBG("ECORE_IMF_INPUT_PANEL_STATE_SHOW and Landscape mode\n");
		elm_naviframe_item_title_visible_set(st->navi_it, EINA_FALSE);
	} else if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		DBG("ECORE_IMF_INPUT_PANEL_STATE_HIDE\n");
		elm_naviframe_item_title_visible_set(st->navi_it, EINA_TRUE);
	}

	return;
}

static void __hide_btn_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	int ret = 0;

	ret = tethering_wifi_set_ssid_visibility(ad->handle,
			!ad->setup.visibility);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_ssid_visibility is failed : %d\n", ret);
		return;
	}

	ad->setup.visibility = !ad->setup.visibility;

	__MOBILE_AP_FUNC_EXIT__;
}

static void __security_btn_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	int ret = 0;

	if (ad->setup.security_type == TETHERING_WIFI_SECURITY_TYPE_NONE)
		ad->setup.security_type = TETHERING_WIFI_SECURITY_TYPE_WPA2_PSK;
	else
		ad->setup.security_type = TETHERING_WIFI_SECURITY_TYPE_NONE;

	ret = tethering_wifi_set_security_type(ad->handle, ad->setup.security_type);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_security_type is failed : %d\n", ret);
		return;
	}

	elm_genlist_item_update(ad->setup.pw_item);

	__MOBILE_AP_FUNC_EXIT__;
}

static char *__gl_hide_label_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.text") != 0) {
		DBG("Unknown part : %s\n", part);
		return NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return strdup(_("IDS_MOBILEAP_BODY_HIDE_MY_DEVICE"));
}

static char *__gl_security_label_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.text") != 0) {
		DBG("Unknown part : %s\n", part);
		return NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return strdup(_("IDS_MOBILEAP_BODY_SECURITY"));
}

static char *__gl_name_label_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.text") != 0) {
		DBG("Unknown part : %s\n", part);
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	char label[MH_LABEL_LENGTH_MAX] = {0, };
	char name_change_label[MH_LABEL_LENGTH_MAX] = {0, };
	char path[MH_LABEL_LENGTH_MAX] = {0, };
	char *device_name = NULL;

	DBG("Device name : %s\n", ad->setup.device_name);

	device_name = elm_entry_utf8_to_markup(ad->setup.device_name);
	if (device_name == NULL) {
		ERR("elm_entry_utf8_to_markup is failed\n");
		return NULL;
	}

	snprintf(path, sizeof(path), "%s > %s",
			S_("IDS_COM_BODY_SETTINGS"),
			_("IDS_ST_BODY_ABOUT_PHONE"));
	snprintf(name_change_label, sizeof(name_change_label),
			_("IDS_MOBILEAP_BODY_DEVICE_NAME_CAN_BE_CHANGED_IN_PS"),
			path);
	snprintf(label, sizeof(label), "%s: %s<br>%s",
			_("IDS_MOBILEAP_BODY_DEVICE_NAME"),
			device_name,
			name_change_label);
	free(device_name);

	__MOBILE_AP_FUNC_EXIT__;

	return strdup(label);
}

static Evas_Object *__gl_hide_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.icon") != 0) {
		DBG("Unknown part : %s\n", part);
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;

	btn = elm_check_add(obj);
	if (btn == NULL) {
		ERR("btn is NULL\n");
		return NULL;
	}
	elm_object_style_set(btn, "on&off");
	evas_object_show(btn);
	evas_object_pass_events_set(btn, EINA_TRUE);
	evas_object_propagate_events_set(btn, EINA_FALSE);
	elm_check_state_set(btn, ad->setup.visibility ? EINA_FALSE : EINA_TRUE);
	evas_object_smart_callback_add(btn, "changed",
			__hide_btn_changed_cb, (void *)ad);
	ad->setup.hide_btn = btn;

	__MOBILE_AP_FUNC_EXIT__;
	return btn;
}

static Evas_Object *__gl_security_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.icon") != 0) {
		DBG("Unknown part : %s\n", part);
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;

	btn = elm_check_add(obj);

	elm_object_style_set(btn, "on&off");
	evas_object_show(btn);
	evas_object_pass_events_set(btn, EINA_TRUE);
	evas_object_propagate_events_set(btn, EINA_FALSE);
	elm_check_state_set(btn, ad->setup.security_type ==
			TETHERING_WIFI_SECURITY_TYPE_NONE ?
			EINA_FALSE : EINA_TRUE);
	evas_object_smart_callback_add(btn, "changed",
			__security_btn_changed_cb, (void *)ad);
	ad->setup.security_btn = btn;

	__MOBILE_AP_FUNC_EXIT__;
	return btn;
}

static Eina_Bool __save_wifi_passphrase(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return EINA_FALSE;
	}

	mh_wifi_setting_view_t *st = &ad->setup;
	int ret = 0;

	ret = tethering_wifi_set_passphrase(ad->handle, st->wifi_passphrase_new);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_passphrase is failed : %d\n", ret);
		return EINA_FALSE;
	}

	g_strlcpy(st->wifi_passphrase, st->wifi_passphrase_new, sizeof(st->wifi_passphrase));

	__MOBILE_AP_FUNC_EXIT__;

	return EINA_TRUE;
}

static void __passphrase_maxlength_reached_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };
	int ret;

	ecore_imf_input_panel_hide();
	elm_object_focus_set(ad->setup.pw_entry, EINA_FALSE);

	if (__save_wifi_passphrase(ad) == EINA_FALSE) {
		ERR("__save_wifi_passphrase is failed\n");
	}

	snprintf(buf, sizeof(buf),
			_("IDS_MOBILEAP_POP_PASSWORD_MUST_CONTAIN_AT_LEAST_PD_CHARACTERS_AND_NOT_EXCEED_PD_CHARACTERS"),
			WIFI_PASSPHRASE_LENGTH_MIN, WIFI_PASSPHRASE_LENGTH_MAX);

	ret = notification_status_message_post(buf);
	if (ret != NOTIFICATION_ERROR_NONE)
		ERR("notification_status_message_post() is failed : %d\n", ret);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __passphrase_activated_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	int ret;
	char *ptr;

	ecore_imf_input_panel_hide();
	elm_object_focus_set(ad->setup.pw_entry, EINA_FALSE);

	if (strlen(ad->setup.wifi_passphrase_new) < WIFI_PASSPHRASE_LENGTH_MIN) {
		DBG("Password is shorter than %d\n", WIFI_PASSPHRASE_LENGTH_MIN);
		ret = notification_status_message_post(_("IDS_ST_BODY_ENTER_PASSWORD_OF_AT_LEAST_8_CHARACTERS"));
		if (ret != NOTIFICATION_ERROR_NONE)
			ERR("notification_status_message_post() is failed : %d\n", ret);

		ptr = elm_entry_utf8_to_markup(ad->setup.wifi_passphrase);
		if (ptr != NULL) {
			elm_entry_entry_set(ad->setup.pw_entry, ptr);
			free(ptr);
		} else {
			ERR("elm_entry_utf8_to_markup is failed\n");
		}

		return;
	}

	if (__save_wifi_passphrase(ad) == EINA_FALSE) {
		ERR("__save_wifi_passphrase is failed\n");
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __pw_entry_changed_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;
	const char *changed_text;
	char *utf8_string;

	changed_text = elm_entry_entry_get(st->pw_entry);
	if (changed_text == NULL) {
		ERR("elm_entry_entry_get is failed\n");
		return;
	}

	utf8_string = elm_entry_markup_to_utf8(changed_text);
	if (utf8_string == NULL) {
		ERR("elm_entry_markup_to_utf8() Failed!!!\n");
	} else {
		g_strlcpy(st->wifi_passphrase_new, utf8_string,
				sizeof(st->wifi_passphrase_new));
		free(utf8_string);
	}

	if (!elm_object_focus_get(st->pw_entry)) {
		return;
	}

	if (elm_entry_is_empty(st->pw_entry)) {
		elm_object_item_signal_emit(st->pw_item,
				"elm,state,eraser,hide", "");
	} else {
		elm_object_item_signal_emit(st->pw_item,
				"elm,state,eraser,show", "");
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;

}

static void __pw_entry_focused_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;

	if (!elm_entry_is_empty(st->pw_entry)) {
		elm_entry_cursor_end_set(st->pw_entry);
		elm_object_item_signal_emit(st->pw_item,
				"elm,state,eraser,show", "");
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;

}

static void __pw_entry_unfocused_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;

	elm_object_item_signal_emit(st->pw_item,
			"elm,state,eraser,hide", "");

	__MOBILE_AP_FUNC_EXIT__;

	return;

}

static void __pw_layout_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;

	elm_entry_entry_set(st->pw_entry, "");

	__MOBILE_AP_FUNC_EXIT__;

	return;

}

static char *__gl_pw_text_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid parameter\n");
		return NULL;
	}

	if (g_strcmp0(part, "elm.text") != 0) {
		ERR("Invalid part : %s\n", part);
		return NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return strdup(_("IDS_MOBILEAP_BODY_PASSWORD"));
}

static Evas_Object *__gl_pw_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (g_strcmp0(part, "elm.icon.entry") == 0) {
		static Elm_Entry_Filter_Limit_Size limit_filter_data;

		mh_appdata_t *ad = (mh_appdata_t *)data;
		mh_wifi_setting_view_t *st = &ad->setup;
		Evas_Object *entry = NULL;
		char *ptr = NULL;
		Ecore_IMF_Context *imf_context;

		entry = elm_entry_add(obj);
		if (entry == NULL) {
			ERR("elm_entry_add returns NULL\n");
			st->pw_entry = NULL;
			return NULL;
		}

		/* Set single line of entry */
		elm_entry_scrollable_set(entry, EINA_TRUE);
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
		elm_entry_select_allow_set(entry, EINA_FALSE);
		elm_entry_context_menu_disabled_set(entry, EINA_TRUE);

		/* Set the maximum length filter for passphrase entry */
		limit_filter_data.max_char_count = 0;
		limit_filter_data.max_byte_count = WIFI_PASSPHRASE_LENGTH_MAX;
		elm_entry_markup_filter_append(entry,
				elm_entry_filter_limit_size, &limit_filter_data);

		DBG("security_type : %d\n", st->security_type);
		if (st->security_type == TETHERING_WIFI_SECURITY_TYPE_NONE) {
			ptr = elm_entry_utf8_to_markup(st->wifi_passphrase);
			if (ptr != NULL) {
				elm_entry_entry_set(entry, ptr);
				free(ptr);
			} else {
				ERR("elm_entry_utf8_to_markup is failed\n");
			}

			/* Set editable mode */
			elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
			elm_object_disabled_set(entry, EINA_TRUE);
		} else {
			ptr = elm_entry_utf8_to_markup(st->wifi_passphrase_new);
			if (ptr != NULL) {
				elm_entry_entry_set(entry, ptr);
				free(ptr);
			} else {
				ERR("elm_entry_utf8_to_markup is failed\n");
			}
		}

		evas_object_smart_callback_add(entry, "maxlength,reached",
				__passphrase_maxlength_reached_cb, data);
		evas_object_smart_callback_add(entry, "activated",
				__passphrase_activated_cb, data);
		evas_object_smart_callback_add(entry, "changed",
				__pw_entry_changed_cb, data);
		evas_object_smart_callback_add(entry, "focused",
				__pw_entry_focused_cb, data);
		evas_object_smart_callback_add(entry, "unfocused",
				__pw_entry_unfocused_cb, data);

		imf_context = (Ecore_IMF_Context *)elm_entry_imf_context_get(entry);
		if (imf_context == NULL) {
			ERR("elm_entry_imf_context_get() is failed\n");
		} else {
			ecore_imf_context_input_panel_event_callback_add(imf_context,
					ECORE_IMF_INPUT_PANEL_STATE_EVENT,
					__input_panel_event_cb, (void *)ad);
		}

		elm_object_item_signal_emit(st->pw_item,
				"elm,state,rename,hide", "");
		if (!elm_entry_is_empty(entry)) {
			elm_object_item_signal_emit(st->pw_item,
					"elm,state,eraser,hide", "");
		}

		elm_object_part_text_set(entry, "elm.guide",
				S_("IDS_COM_BODY_ENTER_PASSWORD"));

		evas_object_show(entry);
		st->pw_entry = entry;

		__MOBILE_AP_FUNC_EXIT__;
		return st->pw_entry;
	} else if (g_strcmp0(part, "elm.icon.eraser") == 0) {
		Evas_Object *btn = elm_button_add(obj);
		elm_object_style_set(btn, "editfield_clear");
		evas_object_smart_callback_add(btn, "clicked", __pw_layout_eraser_clicked_cb, data);
		return btn;
	} else {
		DBG("Unknown part : %s\n", part);
	}

	return NULL;
}

static void __gl_hide_item_sel(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || event_info == NULL) {
		ERR("param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_genlist_item_selected_set((Elm_Object_Item*)event_info, EINA_FALSE);

	__hide_btn_changed_cb(data, obj, NULL);
	elm_check_state_set(ad->setup.hide_btn, ad->setup.visibility ?
			EINA_FALSE : EINA_TRUE);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __gl_security_item_sel(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || event_info == NULL) {
		ERR("param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_genlist_item_selected_set((Elm_Object_Item*)event_info, EINA_FALSE);

	__security_btn_changed_cb(data, obj, NULL);
	elm_check_state_set(ad->setup.security_btn, ad->setup.security_type ==
			TETHERING_WIFI_SECURITY_TYPE_NONE ? EINA_FALSE : EINA_TRUE);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __free_genlist_itc(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	elm_genlist_item_class_free(ad->setup.sp_itc);
	elm_genlist_item_class_free(ad->setup.hide_itc);
	elm_genlist_item_class_free(ad->setup.security_itc);
	elm_genlist_item_class_free(ad->setup.pw_itc);
	elm_genlist_item_class_free(ad->setup.name_itc);

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __set_genlist_itc(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	ad->setup.sp_itc = elm_genlist_item_class_new();
	ad->setup.sp_itc->item_style = "dialogue/separator";
	ad->setup.sp_itc->func.text_get = NULL;
	ad->setup.sp_itc->func.content_get = NULL;
	ad->setup.sp_itc->func.state_get = NULL;
	ad->setup.sp_itc->func.del = NULL;

	ad->setup.hide_itc = elm_genlist_item_class_new();
	ad->setup.hide_itc->item_style = "dialogue/1text.1icon";
	ad->setup.hide_itc->func.text_get = __gl_hide_label_get;
	ad->setup.hide_itc->func.content_get = __gl_hide_icon_get;
	ad->setup.hide_itc->func.state_get = NULL;
	ad->setup.hide_itc->func.del = NULL;

	ad->setup.security_itc = elm_genlist_item_class_new();
	ad->setup.security_itc->item_style = "dialogue/1text.1icon";
	ad->setup.security_itc->func.text_get = __gl_security_label_get;
	ad->setup.security_itc->func.content_get = __gl_security_icon_get;
	ad->setup.security_itc->func.state_get = NULL;
	ad->setup.security_itc->func.del = NULL;

	ad->setup.pw_itc = elm_genlist_item_class_new();
	ad->setup.pw_itc->item_style = "dialogue/editfield/title";
	ad->setup.pw_itc->func.text_get = __gl_pw_text_get;
	ad->setup.pw_itc->func.content_get = __gl_pw_icon_get;
	ad->setup.pw_itc->func.state_get = NULL;
	ad->setup.pw_itc->func.del = NULL;

	ad->setup.name_itc = elm_genlist_item_class_new();
	ad->setup.name_itc->item_style = "multiline/1text";
	ad->setup.name_itc->func.text_get = __gl_name_label_get;
	ad->setup.name_itc->func.content_get = NULL;
	ad->setup.name_itc->func.state_get = NULL;
	ad->setup.name_itc->func.del = NULL;

	ad->setup.end_sp_itc = elm_genlist_item_class_new();
	ad->setup.end_sp_itc->item_style = "dialogue/separator";
	ad->setup.end_sp_itc->func.text_get = NULL;
	ad->setup.end_sp_itc->func.content_get = NULL;
	ad->setup.end_sp_itc->func.state_get = NULL;
	ad->setup.end_sp_itc->func.del = NULL;

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __deconstruct_wifi_setup_view(mh_wifi_setting_view_t *st)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (st == NULL) {
		ERR("st is NULL\n");
		return;
	}

	if (st->pw_entry == NULL) {
		ERR("pw_entry is NULL");
		return;
	}

	evas_object_smart_callback_del(st->hide_btn, "changed",
			__hide_btn_changed_cb);
	evas_object_smart_callback_del(st->security_btn, "changed",
			__security_btn_changed_cb);
	if (st->pw_entry) {
		evas_object_smart_callback_del(st->pw_entry,
				"maxlength,reached",
				__passphrase_maxlength_reached_cb);
		evas_object_smart_callback_del(st->pw_entry,
				"activated",
				__passphrase_activated_cb);
	}
	evas_object_smart_callback_del(st->back_btn, "clicked",
			__back_btn_cb);
	evas_object_smart_callback_del(st->genlist, "realized",
			__gl_realized);

	st->hide_btn = NULL;
	st->security_btn = NULL;
	st->pw_entry = NULL;
	st->back_btn = NULL;
	st->genlist = NULL;

	__MOBILE_AP_FUNC_EXIT__;
}

static void __back_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;
	Eina_Bool imf_state;
	int ret;
	char *ptr = NULL;

	imf_state = ecore_imf_input_panel_hide();
	elm_object_focus_set(st->pw_entry, EINA_FALSE);

	if (strlen(st->wifi_passphrase_new) < WIFI_PASSPHRASE_LENGTH_MIN) {
		DBG("Password is shorter than %d\n", WIFI_PASSPHRASE_LENGTH_MIN);
		ret = notification_status_message_post(_("IDS_ST_BODY_ENTER_PASSWORD_OF_AT_LEAST_8_CHARACTERS"));
		if (ret != NOTIFICATION_ERROR_NONE)
			ERR("notification_status_message_post() is failed : %d\n", ret);

		ptr = elm_entry_utf8_to_markup(st->wifi_passphrase);
		if (ptr != NULL) {
			elm_entry_entry_set(st->pw_entry, ptr);
			free(ptr);
		} else {
			ERR("elm_entry_utf8_to_markup is failed\n");
		}

		return;
	}

	if (g_strcmp0(st->wifi_passphrase, st->wifi_passphrase_new) != 0) {
		if (__save_wifi_passphrase(ad) == EINA_FALSE) {
			ERR("__save_wifi_passphrase is failed\n");
		}
	}

	if (imf_state == EINA_TRUE) {
		DBG("IMF is hided\n");
		return;
	}

	if (ad->main.need_recover_wifi_tethering == true) {
		DBG("Turn on Wi-Fi tethering again\n");
		_update_wifi_item(ad, MH_STATE_PROCESS);
		if (ad->main.help_item)
			elm_genlist_item_update(ad->main.help_item);

		ret = tethering_enable(ad->handle, TETHERING_TYPE_WIFI);
		if (ret != TETHERING_ERROR_NONE) {
			ERR("Wi-Fi tethering on is failed : %d\n", ret);
			_update_wifi_item(ad, MH_STATE_NONE);
			ad->main.need_recover_wifi_tethering = false;
		}
	}

	__deconstruct_wifi_setup_view(st);

	evas_object_del(ad->setup.genlist);
	__free_genlist_itc(ad);

	elm_naviframe_item_pop(ad->naviframe);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &(ad->setup);
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Evas_Object *ao;
	Evas_Object *btn;
	char str[MH_LABEL_LENGTH_MAX] = {0, };
	int i = 0;

	if (item == st->hide_item) {
		DBG("Hide item is realized\n");
		elm_object_item_signal_emit(item, "elm,state,top", "");
	} else if (item == st->security_item) {
		DBG("Security item is realized\n");
		elm_object_item_signal_emit(item, "elm,state,center", "");
	} else if (item == st->pw_item) {
		DBG("Password item is realized\n");
		elm_object_item_signal_emit(item, "elm,state,bottom", "");
	} else if (item == st->name_item) {
		DBG("Name item is realized\n");
	}

	for (i = 0; i < 2; i++) {
		if (item == st->sp_item[i])
			elm_object_item_access_unregister(item);
	}

	if (item == st->hide_item || item == st->security_item) {
		ao = elm_object_item_access_object_get(item);
		btn = elm_object_item_part_content_get(item, "on&off");
		snprintf(str, sizeof(str), "%s, %s", "On/off button",
				(elm_check_state_get(btn) ? "On" : "Off"));
		elm_access_info_set(ao, ELM_ACCESS_CONTEXT_INFO, str);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

Evas_Object *__create_genlist(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return NULL;
	}

	void *data = (void *)ad;
	mh_wifi_setting_view_t *st = &ad->setup;
	Evas_Object *genlist;
	Elm_Object_Item *item;
	int i = 0;

	genlist = elm_genlist_add(ad->naviframe);
	if (genlist == NULL) {
		ERR("genlist is NULL\n");
		return NULL;
	}

	elm_object_style_set(genlist, "dialogue");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "realized", __gl_realized, ad);

	__set_genlist_itc(ad);
	item = elm_genlist_item_append(genlist, st->sp_itc, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	st->sp_item[i++] = item;

	st->hide_item = elm_genlist_item_append(genlist, st->hide_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_hide_item_sel, data);

	st->security_item = elm_genlist_item_append(genlist, st->security_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_security_item_sel, data);

	st->pw_item = elm_genlist_item_append(genlist, st->pw_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(st->pw_item, ELM_OBJECT_SELECT_MODE_NONE);

	st->name_item = elm_genlist_item_append(genlist, st->name_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(st->name_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	item = elm_genlist_item_append(genlist, st->end_sp_itc, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	st->sp_item[i++] = item;


	__MOBILE_AP_FUNC_EXIT__;

	return genlist;
}

char *__slide_title_access_info_cb(void *data, Evas_Object *obj)
{
	Evas_Object *label;
	Elm_Object_Item *navi_it;
	Eina_Strbuf *buf;
	const char *info;
	char *ret;

	navi_it = (Elm_Object_Item *) data;
	label = elm_object_item_part_content_get(navi_it, "elm.swallow.title");
	if (label == NULL)
		return NULL;

	info = elm_object_text_get(label);
	if (info == NULL)
		return NULL;

	buf = eina_strbuf_new();
	eina_strbuf_append(buf, info);

	ret = eina_strbuf_string_steal(buf);
	eina_strbuf_free(buf);
	return ret;
}

static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *label;
	Elm_Object_Item *navi_it = event_info;
	if (navi_it == NULL)
		return;

	label = elm_object_item_part_content_get(navi_it, "elm.swallow.title");
	if (label == NULL)
		return;

	elm_label_slide_go(label);
}

void mh_draw_wifi_setup_view(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	mh_wifi_setting_view_t *st = &ad->setup;
	Evas_Object *ao;
	Evas_Object *label;


	if (st->genlist != NULL) {
		ERR("Wi-Fi setup view already exists\n");
		return;
	}

	st->genlist = __create_genlist(ad);
	if (st->genlist == NULL) {
		ERR("__create_genlist returns NULL\n");
		goto FAIL;
	}

	st->back_btn = elm_button_add(ad->naviframe);
	if (st->back_btn == NULL) {
		ERR("elm_button_add returns NULL\n");
		goto FAIL;
	}
	elm_object_style_set(st->back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(st->back_btn, "clicked",
			__back_btn_cb, ad);
	elm_object_focus_allow_set(st->back_btn, EINA_FALSE);

	st->navi_it = elm_naviframe_item_push(ad->naviframe,
			_("IDS_MOBILEAP_MBODY_WI_FI_TETHERING_SETTINGS"),
			st->back_btn, NULL, st->genlist, NULL);

#ifndef SK_BACK_SUPPORT
	elm_naviframe_item_pop_cb_set(st->navi_it, __back_btn_cb, (void *)ad);
#endif

	/* Slide title */
	label = _create_slide_title(ad->naviframe, _("IDS_MOBILEAP_MBODY_WI_FI_TETHERING_SETTINGS"));

	elm_object_item_part_content_set(st->navi_it, "elm.swallow.title", label);
	evas_object_smart_callback_add(ad->naviframe, "title,clicked", __title_clicked_cb, NULL);

	ao = elm_object_item_access_object_get(st->navi_it);
	if (ao) {
		elm_access_info_set(ao, ELM_ACCESS_TYPE, "title");
		elm_access_info_cb_set(ao, ELM_ACCESS_INFO,
				__slide_title_access_info_cb, st->navi_it);
		elm_object_focus_set(ao, EINA_TRUE);
	}

	/* Realize genlist item forcely since UIFW doesn't make realize to avoid crash on landscape mode*/
	elm_genlist_item_update(st->pw_item);

	__MOBILE_AP_FUNC_EXIT__;

	return;

FAIL:
	if (st->back_btn)
		evas_object_del(st->back_btn);
	if (st->genlist)
		evas_object_del(st->genlist);

	st->back_btn = NULL;
	st->genlist = NULL;
}
