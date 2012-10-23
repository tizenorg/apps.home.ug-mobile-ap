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
	Eina_Bool pw_disabled = EINA_FALSE;
	tethering_wifi_security_type_e temp_security_type;
	int ret = 0;

	if (ad->setup.security_type == TETHERING_WIFI_SECURITY_TYPE_NONE)
		temp_security_type = TETHERING_WIFI_SECURITY_TYPE_WPA2_PSK;
	else
		temp_security_type = TETHERING_WIFI_SECURITY_TYPE_NONE;

	ret = tethering_wifi_set_security_type(ad->handle, temp_security_type);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_security_type is failed : %d\n", ret);
		return;
	}
	ad->setup.security_type = temp_security_type;

	pw_disabled = ad->setup.security_type != TETHERING_WIFI_SECURITY_TYPE_NONE ?
		EINA_FALSE : EINA_TRUE;
	DBG("security_type : %d, pw_disabled : %d\n", ad->setup.security_type, pw_disabled);

	elm_object_item_disabled_set(ad->setup.pw_item, pw_disabled);
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

	if (strcmp(part, "elm.text.1") != 0) {
		ERR("Invalid param\n");
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
	const char *entry_string = NULL;
	char *utf8_string = NULL;
	char wifi_passphrase[WIFI_PASSPHRASE_LENGTH_MAX + 1] = {0, };

	entry_string = elm_entry_entry_get(st->pw_entry);
	if (entry_string == NULL) {
		ERR("elm_entry_entry_get() Failed!!!\n");
		return EINA_FALSE;
	}

	utf8_string = elm_entry_markup_to_utf8(entry_string);
	if (utf8_string == NULL) {
		ERR("elm_entry_markup_to_utf8() Failed!!!\n");
		return EINA_FALSE;
	}
	g_strlcpy(wifi_passphrase, utf8_string, sizeof(wifi_passphrase));
	free(utf8_string);

	if (g_strcmp0(st->wifi_passphrase, wifi_passphrase) == 0) {
		DBG("Password is not changed\n");
		return EINA_TRUE;
	}

	if (strlen(wifi_passphrase) < WIFI_PASSPHRASE_LENGTH_MIN) {
		DBG("Password is shorter than %d\n", WIFI_PASSPHRASE_LENGTH_MIN);
		_prepare_popup(ad, MH_POP_WIFI_PASSWORD_SHORT,
				_("IDS_ST_BODY_ENTER_PASSWORD_OF_AT_LEAST_8_CHARACTERS"));
		_create_popup(ad);
		return EINA_FALSE;
	}

	ret = tethering_wifi_set_passphrase(ad->handle, wifi_passphrase);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_passphrase is failed : %d\n", ret);
		return EINA_FALSE;
	}

	DBG("SUCCESS : setting up VCONFKEY_MOBILE_HOTSPOT_WIFI_KEY\n");
	g_strlcpy(st->wifi_passphrase, wifi_passphrase, sizeof(st->wifi_passphrase));

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

	if (_hide_imf(ad->setup.pw_entry) == EINA_FALSE) {
		ERR("_hide_imf is failed\n");
	}

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

	if (_hide_imf(ad->setup.pw_entry) == EINA_FALSE) {
		ERR("_hide_imf is failed\n");
	}

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

	if (!elm_object_focus_get(st->pw_layout))
		return;

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
		ERR("Invalid part\n");
		return NULL;
	}

	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;
	Evas_Object *entry = NULL;
	char *ptr = NULL;

	st->pw_layout = elm_layout_add(obj);
	if (st->pw_layout == NULL) {
		ERR("elm_layout_add returns NULL\n");
		return NULL;
	}
	elm_layout_theme_set(st->pw_layout, "layout", "editfield", "title");

	entry = elm_entry_add(st->pw_layout);
	if (entry == NULL) {
		ERR("elm_entry_add returns NULL\n");
		evas_object_del(st->pw_layout);
		st->pw_layout = NULL;
		return NULL;
	}
	st->pw_entry = entry;

	elm_object_part_content_set(st->pw_layout, "elm.swallow.content", entry);
	elm_object_part_text_set(st->pw_layout, "elm.text",
			_("IDS_MOBILEAP_BODY_PASSWORD"));
	elm_object_part_text_set(st->pw_layout, "elm.guidetext",
			_("IDS_ST_BODY_ENTER_PASSWORD_OF_AT_LEAST_8_CHARACTERS"));

	/* Set editable mode */
	DBG("security_type : %d\n", st->security_type);
	elm_entry_input_panel_enabled_set(entry, st->security_type ==
			TETHERING_WIFI_SECURITY_TYPE_NONE ?
			EINA_FALSE : EINA_TRUE);

	/* Set single line of entry */
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);

	/* Set the maximum length filter for passphrase entry */
	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = WIFI_PASSPHRASE_LENGTH_MAX;
	elm_entry_markup_filter_append(entry,
			elm_entry_filter_limit_size, &limit_filter_data);

	ptr = elm_entry_utf8_to_markup(st->wifi_passphrase);
	if (ptr != NULL) {
		elm_entry_entry_set(entry, ptr);
		free(ptr);
	} else {
		ERR("elm_entry_utf8_to_markup is failed\n");
	}

	if (!elm_entry_is_empty(entry))
		elm_object_signal_emit(st->pw_layout,
				"elm,state,guidetext,hide", "elm");

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
	elm_object_signal_callback_add(st->pw_layout, "elm,eraser,clicked", "elm",
			__pw_layout_eraser_clicked_cb, data);

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
	ad->setup.sp_itc->item_style = "dialogue/separator/21/with_line";
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
	ad->setup.end_sp_itc->item_style = "dialogue/separator/end";
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

	st->hide_btn = NULL;
	st->security_btn = NULL;
	st->pw_layout = NULL;
	st->pw_entry = NULL;
	st->back_btn = NULL;
	st->genlist = NULL;
	st->conform = NULL;

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

	if (_hide_imf(st->pw_entry) == EINA_FALSE) {
		ERR("_hide_imf is failed\n");
	}

	if (__save_wifi_passphrase(ad) == EINA_FALSE) {
		ERR("__save_wifi_passphrase is failed\n");
		return;
	}

	__deconstruct_wifi_setup_view(st);

	evas_object_del(ad->setup.genlist);
	__free_genlist_itc(ad);

	elm_naviframe_item_pop(ad->naviframe);

	__MOBILE_AP_FUNC_EXIT__;
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
	Evas_Object *genlist = NULL;
	Elm_Object_Item *item = NULL;

	genlist = elm_genlist_add(ad->naviframe);
	if (genlist == NULL) {
		ERR("genlist is NULL\n");
		return NULL;
	}
	elm_object_style_set(genlist, "dialogue");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	__set_genlist_itc(ad);
	item = elm_genlist_item_append(genlist, st->sp_itc, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	st->hide_item = elm_genlist_item_append(genlist, st->hide_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_hide_item_sel, data);

	st->security_item = elm_genlist_item_append(genlist, st->security_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_security_item_sel, data);

	DBG("security_type : %d\n", st->security_type);
	st->pw_item = elm_genlist_item_append(genlist, st->pw_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_object_item_disabled_set(st->pw_item, st->security_type ==
			TETHERING_WIFI_SECURITY_TYPE_NONE ?
			EINA_TRUE : EINA_FALSE);
	elm_genlist_item_update(st->pw_item);

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

	if (st->conform != NULL) {
		ERR("Wi-Fi setup view already exists\n");
		return;
	}

	st->conform = elm_conformant_add(ad->naviframe);
	if (st->conform == NULL) {
		ERR("elm_conformant_add returns NULL\n");
		goto FAIL;
	}
	elm_object_style_set(st->conform, "internal_layout");
	evas_object_show(st->conform);

	st->genlist = __create_genlist(ad);
	if (st->genlist == NULL) {
		ERR("__create_genlist returns NULL\n");
		goto FAIL;
	}
	elm_object_content_set(st->conform, st->genlist);

	st->back_btn = elm_button_add(ad->naviframe);
	if (st->back_btn == NULL) {
		ERR("elm_button_add returns NULL\n");
		goto FAIL;
	}
	elm_naviframe_item_push(ad->naviframe,
			_("IDS_MOBILEAP_MBODY_WI_FI_TETHERING_SETTINGS"),
			st->back_btn, NULL, st->conform, NULL);

	/* Style set should be called after elm_naviframe_item_push(). */
	elm_object_style_set(st->back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(st->back_btn, "clicked",
			__back_btn_cb, ad);
	elm_object_focus_allow_set(st->back_btn, EINA_FALSE);

	__MOBILE_AP_FUNC_EXIT__;

	return;

FAIL:
	if (st->back_btn)
		evas_object_del(st->back_btn);
	if (st->genlist)
		evas_object_del(st->genlist);
	if (st->conform)
		evas_object_del(st->conform);

	st->back_btn = NULL;
	st->genlist = NULL;
	st->conform = NULL;
}
