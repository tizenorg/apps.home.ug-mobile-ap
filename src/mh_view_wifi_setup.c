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

#include "mh_view_wifi_setup.h"

static void __back_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void __gl_realized(void *data, Evas_Object *obj, void *event_info);

static void __input_panel_event_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	if (data == NULL) {
		ERR("data is NULL\n");
		return;
	}

	mh_wifi_setting_view_t *st = (mh_wifi_setting_view_t *)data;

	if (st->navi_it == NULL) {
		DBG("naviframe item is not pushed yet\n");
		return;
	}

	if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW) {
		DBG("value == ECORE_IMF_INPUT_PANEL_STATE_SHOW\n");
		elm_object_item_signal_emit(st->navi_it, "elm,state,sip,shown", "");
	} else if(value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		DBG("value == ECORE_IMF_INPUT_PANEL_STATE_HIDE\n");
		elm_object_item_signal_emit(st->navi_it, "elm,state,sip,hidden", "");
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
		ERR("Invalid param\n");
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
		ERR("Invalid param\n");
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
		ERR("Invalid param : %s\n", part);
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
		ERR("Invalid param\n");
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
		ERR("Invalid param\n");
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

	DBG("SUCCESS : setting up VCONFKEY_MOBILE_HOTSPOT_WIFI_KEY\n");
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

	if (ad->popup != NULL) {
		ERR("Popup already exists\n");
		return;
	}

	elm_object_focus_set(ad->setup.pw_entry, EINA_FALSE);

	if (__save_wifi_passphrase(ad) == EINA_FALSE) {
		ERR("__save_wifi_passphrase is failed\n");
		return;
	}

	snprintf(buf, sizeof(buf),
			_("IDS_MOBILEAP_POP_PASSWORD_MUST_CONTAIN_AT_LEAST_PD_CHARACTERS_AND_NOT_EXCEED_PD_CHARACTERS"),
			WIFI_PASSPHRASE_LENGTH_MIN, WIFI_PASSPHRASE_LENGTH_MAX);

	_prepare_popup(ad, MH_POP_INFORMATION_WO_BUTTON, buf);
	_create_popup(ad);

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

	elm_object_focus_set(ad->setup.pw_entry, EINA_FALSE);

	if (__save_wifi_passphrase(ad) == EINA_FALSE) {
		ERR("__save_wifi_passphrase is failed\n");
		return;
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

	if (!elm_object_focus_get(st->pw_layout)) {
		return;
	}

	if (elm_entry_is_empty(st->pw_entry)) {
		elm_object_signal_emit(st->pw_layout,
				"elm,state,eraser,hide", "elm");
	} else {
		elm_object_signal_emit(st->pw_layout,
				"elm,state,eraser,show", "elm");
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

	elm_entry_cursor_end_set(st->pw_entry);

	if (!elm_entry_is_empty(st->pw_entry))
		elm_object_signal_emit(st->pw_layout,
				"elm,state,eraser,show", "elm");

	elm_object_signal_emit(st->pw_layout,
			"elm,state,guidetext,hide", "elm");

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

	if (elm_entry_is_empty(st->pw_entry))
		elm_object_signal_emit(st->pw_layout,
				"elm,state,guidetext,show", "elm");

	elm_object_signal_emit(st->pw_layout,
			"elm,state,eraser,hide", "elm");

	__MOBILE_AP_FUNC_EXIT__;

	return;

}

static void __pw_layout_eraser_clicked_cb(void *data, Evas_Object *obj,
		const char *emission, const char *source)
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

static Evas_Object *__gl_pw_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.icon") != 0) {
		ERR("Invalid part : %s\n", part);
		return NULL;
	}

	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;
	Evas_Object *entry = NULL;
	char *ptr = NULL;
	Ecore_IMF_Context *imf_context;

	st->pw_layout = elm_layout_add(obj);
	if (st->pw_layout == NULL) {
		ERR("elm_layout_add returns NULL\n");
		return NULL;
	}
	elm_layout_theme_set(st->pw_layout, "layout", "editfield", "title");
	evas_object_size_hint_weight_set(st->pw_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	entry = elm_entry_add(st->pw_layout);
	if (entry == NULL) {
		ERR("elm_entry_add returns NULL\n");
		evas_object_del(st->pw_layout);
		st->pw_layout = NULL;
		return NULL;
	}
	st->pw_entry = entry;

	/* Set single line of entry */
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);

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
		elm_entry_context_menu_disabled_set(entry, EINA_TRUE);
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
	if (imf_context)
		ecore_imf_context_input_panel_event_callback_add(imf_context,
				ECORE_IMF_INPUT_PANEL_STATE_EVENT,
				__input_panel_event_cb, (void *)st);

	if (!elm_entry_is_empty(entry)) {
		elm_object_signal_emit(st->pw_layout,
				"elm,state,guidetext,hide", "elm");
	}

	elm_object_part_text_set(st->pw_layout, "elm.text",
			_("IDS_MOBILEAP_BODY_PASSWORD"));
	elm_object_part_content_set(st->pw_layout, "elm.swallow.content", entry);
	elm_object_part_text_set(st->pw_layout, "elm.guidetext",
			_("IDS_ST_BODY_ENTER_PASSWORD_OF_AT_LEAST_8_CHARACTERS"));

	elm_object_signal_callback_add(st->pw_layout, "elm,eraser,clicked", "elm",
			__pw_layout_eraser_clicked_cb, data);

	evas_object_show(entry);
	evas_object_show(st->pw_layout);

	__MOBILE_AP_FUNC_EXIT__;

	return st->pw_layout;
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
	ad->setup.pw_itc->item_style = "dialogue/1icon";
	ad->setup.pw_itc->func.text_get = NULL;
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
	st->pw_layout = NULL;
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
	int ret;

	elm_object_focus_set(st->pw_entry, EINA_FALSE);

	if (strlen(st->wifi_passphrase_new) < WIFI_PASSPHRASE_LENGTH_MIN) {
		DBG("Password is shorter than %d\n", WIFI_PASSPHRASE_LENGTH_MIN);
		_prepare_popup(ad, MH_POP_WIFI_PASSWORD_SHORT,
				_("IDS_ST_BODY_ENTER_PASSWORD_OF_AT_LEAST_8_CHARACTERS"));
		_create_popup(ad);
		return;
	}

	if (g_strcmp0(st->wifi_passphrase, st->wifi_passphrase_new) != 0) {
		if (__save_wifi_passphrase(ad) == EINA_FALSE) {
			ERR("__save_wifi_passphrase is failed\n");
		}
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

static void __title_back_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_object_focus_set(ad->setup.pw_entry, EINA_FALSE);

	if (__save_wifi_passphrase(ad) == EINA_FALSE) {
		ERR("__save_wifi_passphrase is failed\n");
		return;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &(ad->setup);
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

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

	st->hide_item = elm_genlist_item_append(genlist, st->hide_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_hide_item_sel, data);

	st->security_item = elm_genlist_item_append(genlist, st->security_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_security_item_sel, data);

	st->pw_item = elm_genlist_item_append(genlist, st->pw_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(st->pw_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	st->name_item = elm_genlist_item_append(genlist, st->name_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(st->name_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	item = elm_genlist_item_append(genlist, st->end_sp_itc, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);


	__MOBILE_AP_FUNC_EXIT__;

	return genlist;
}

void mh_draw_wifi_setup_view(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	mh_wifi_setting_view_t *st = &ad->setup;

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

	st->navi_it = elm_naviframe_item_push(ad->naviframe,
			_("IDS_MOBILEAP_MBODY_WI_FI_TETHERING_SETTINGS"),
			st->back_btn, NULL, st->genlist, NULL);

	st->title_back_btn = elm_button_add(ad->naviframe);
	elm_object_style_set(st->title_back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(st->title_back_btn, "clicked", __title_back_btn_cb, ad);
	elm_object_item_part_content_set(st->navi_it, "title_prev_btn", st->title_back_btn);

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
