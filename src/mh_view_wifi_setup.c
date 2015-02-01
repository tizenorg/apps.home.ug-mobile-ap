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
#include <utilX.h>

#include "mh_popup.h"
#include "mh_view_wifi_setup.h"
#include "mh_string.h"

static void __gl_realized(void *data, Evas_Object *obj, void *event_info);
static void __cancel_btn_cb(void *data, Evas_Object *object, void *event_info);
static void __save_btn_cb(void *data, Evas_Object *object, void *event_info);

static void __hide_btn_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	ad->setup.visibility_new = !ad->setup.visibility_new;

	__MOBILE_AP_FUNC_EXIT__;
}

static bool __save_hide_btn_change(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return false;
	}

	int ret = 0;

	ret = tethering_wifi_set_ssid_visibility(ad->handle,
			ad->setup.visibility_new);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_ssid_visibility is failed : %d\n", ret);
		return false;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return true;
}

static void __security_btn_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("Invalid param\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;

	if (st->security_type_new == TETHERING_WIFI_SECURITY_TYPE_NONE)
		st->security_type_new = TETHERING_WIFI_SECURITY_TYPE_WPA2_PSK;
	else
		st->security_type_new = TETHERING_WIFI_SECURITY_TYPE_NONE;

	/* update wifi passphrase item */
	if (st->security_type_new == TETHERING_WIFI_SECURITY_TYPE_NONE) {
		elm_object_item_disabled_set(st->pw_item, EINA_TRUE);
		elm_object_disabled_set(st->save_button, EINA_FALSE);
	} else {
		elm_object_item_disabled_set(st->pw_item, EINA_FALSE);
		if (st->pw_entry == NULL || strlen(elm_entry_entry_get(st->pw_entry)) == 0)
			elm_object_disabled_set(st->save_button, EINA_TRUE);
	}

	elm_genlist_item_update(st->pw_item);

	__MOBILE_AP_FUNC_EXIT__;
}

static bool __save_security_btn_change(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return false;
	}

	int ret = 0;

	ret = tethering_wifi_set_security_type(ad->handle, ad->setup.security_type_new);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_security_type is failed : %d\n", ret);
		return false;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return true;
}

static char *__gl_device_name_title_label_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;
	mh_appdata_t *ad = (mh_appdata_t *)data;
	char *device_name_utf = NULL;
	char *ptr = NULL;
	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (!strcmp(part, "elm.text.multiline")) {
		device_name_utf = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
		if (device_name_utf == NULL) {
			ERR("vconf_get_str failed \n");
		}
		ptr = elm_entry_utf8_to_markup(device_name_utf);
		if (ptr == NULL) {
			ERR("elm_entry_utf8_to_markup is failed\n");
			free(device_name_utf);
			return NULL;
		}
		g_strlcpy(ad->setup.device_name, ptr,
				sizeof(ad->setup.device_name));
		free(device_name_utf);
		return ptr;
	}

	if (!strcmp(part, "elm.text.sub")) {
		return strdup(STR_MY_DEVICE_NAME);
	}

	__MOBILE_AP_FUNC_EXIT__;
	return NULL;
}

static char *__gl_hide_label_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.text.main.left") != 0) {
		return NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return strdup(STR_HIDE_MY_DEV);
}

static char *__gl_security_label_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.text.main.left") != 0) {
		return NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return strdup(STR_SECURITY_TYPE);
}

static Evas_Object *__gl_hide_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.icon.2") != 0) {
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;
	Evas_Object *icon_layout = NULL;

	icon_layout = elm_layout_add(obj);
	elm_layout_theme_set(icon_layout, "layout", "list/C/type.3", "default");
	btn = elm_check_add(obj);
	if (btn == NULL) {
		ERR("btn is NULL\n");
		return NULL;
	}
	elm_object_style_set(btn, "on&off");
	evas_object_show(btn);
	evas_object_pass_events_set(btn, EINA_TRUE);
	evas_object_propagate_events_set(btn, EINA_FALSE);
	elm_object_focus_allow_set(btn, EINA_FALSE);
	elm_check_state_set(btn, ad->setup.visibility_new ? EINA_FALSE : EINA_TRUE);
	evas_object_smart_callback_add(btn, "changed",
			__hide_btn_changed_cb, (void *)ad);

	ad->setup.hide_btn = btn;

	elm_layout_content_set(icon_layout, "elm.swallow.content", btn);
	__MOBILE_AP_FUNC_EXIT__;
	return icon_layout;
}

static Evas_Object *__gl_security_icon_get(void *data, Evas_Object *obj,
		const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.icon.2") != 0) {
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *btn = NULL;
	Evas_Object *icon_layout = NULL;

	icon_layout = elm_layout_add(obj);
	elm_layout_theme_set(icon_layout, "layout", "list/C/type.3", "default");

	btn = elm_check_add(obj);
	elm_object_style_set(btn, "on&off");
	evas_object_show(btn);
	evas_object_pass_events_set(btn, EINA_TRUE);
	evas_object_propagate_events_set(btn, EINA_FALSE);
	elm_check_state_set(btn, ad->setup.security_type_new ==
			TETHERING_WIFI_SECURITY_TYPE_NONE ?
			EINA_FALSE : EINA_TRUE);
	evas_object_smart_callback_add(btn, "changed",
			__security_btn_changed_cb, (void *)ad);
	ad->setup.security_btn = btn;

	elm_layout_content_set(icon_layout, "elm.swallow.content", btn);
	__MOBILE_AP_FUNC_EXIT__;
	return icon_layout;
}

static bool __save_wifi_passphrase(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return false;
	}

	mh_wifi_setting_view_t *st = &ad->setup;
	int ret = 0;

	ret = tethering_wifi_set_passphrase(ad->handle, st->wifi_passphrase_new);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_set_passphrase is failed : %d\n", ret);
		return false;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return true;
}

static void __pw_entry_changed_cb(void *data, Evas_Object *obj,
				void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;
	const char *changed_text;
	char *utf8_string;
	int len = 0;

	changed_text = elm_entry_entry_get(obj);
	if (changed_text == NULL) {
		ERR("elm_entry_entry_get is failed\n");
		return;
	}

	utf8_string = elm_entry_markup_to_utf8(changed_text);

	if (utf8_string == NULL) {
		ERR("elm_entry_markup_to_utf8() Failed!!!\n");
	} else {
		len = strlen(utf8_string);
		if (WIFI_PASSPHRASE_LENGTH_MIN <= len) {
			elm_object_disabled_set(st->save_button, EINA_FALSE);
			elm_entry_input_panel_return_key_disabled_set(obj, FALSE);
		} else {
			elm_object_disabled_set(st->save_button, EINA_TRUE);
			elm_entry_input_panel_return_key_disabled_set(obj, TRUE);
		}
		g_strlcpy(st->wifi_passphrase_new, utf8_string,
						sizeof(st->wifi_passphrase_new));
		free(utf8_string);
		utf8_string = NULL;
	}

	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (elm_object_focus_get(obj)) {
			if (elm_entry_is_empty(obj))
				elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
			else
				elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		}
	}

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __pw_entry_maxlength_reached_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (obj == NULL) {
		ERR("The param is NULL\n");
		__MOBILE_AP_FUNC_EXIT__;
		return;
	}
	char buf[MH_LABEL_LENGTH_MAX] = { 0, };
	notification_error_e ret;

	snprintf(buf, sizeof(buf), STR_PASSWORD_MIN_MAX,
			WIFI_PASSPHRASE_LENGTH_MIN, WIFI_PASSPHRASE_LENGTH_MAX);

	ret = notification_status_message_post(buf);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_status_message_post() is failed : %d\n", ret);
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void __pw_entry_activated_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (obj == NULL) {
		ERR("Invalid parameter");
		return;
	}

	elm_object_focus_set(obj, EINA_FALSE);
	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __pw_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (!elm_entry_is_empty(obj))
			elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		else
			elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	}
	elm_object_signal_emit(obj, "elm,state,focus,on", "");
}

static void __pw_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear"))
		elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	elm_object_signal_emit(obj, "elm,state,focus,off", "");
}

static void __eraser_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	elm_entry_entry_set(data, "");
	elm_entry_input_panel_return_key_disabled_set(data, TRUE);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __pw_entry_language_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (obj == NULL || data == NULL) {
		ERR("NULL param\n");
		return;
	}
	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;
	char buf[MH_LABEL_LENGTH_MAX];

	snprintf(buf, sizeof(buf), STR_PW_GUIDE_TEXT, WIFI_PASSPHRASE_LENGTH_MIN);

	if (st->security_type_new == TETHERING_WIFI_SECURITY_TYPE_NONE) {
		elm_genlist_item_update(st->pw_item);
	} else {
		elm_object_part_text_set(obj, "elm.guide", buf);
	}
	__MOBILE_AP_FUNC_EXIT__;
}

static Evas_Object *__get_pw_entry(void *data, Evas_Object *parent)
{
	__MOBILE_AP_FUNC_ENTER__;

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	Evas_Object *entry = NULL;
	char *ptr = NULL;
	Evas_Object * clr_btn = NULL;
	char buf[MH_LABEL_LENGTH_MAX];

	if (parent == NULL || data == NULL) {
		ERR("null param \n");
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;

	entry = ea_editfield_add(parent, EA_EDITFIELD_SCROLL_SINGLELINE);
	if (entry == NULL) {
		ERR("elm_entry_add returns NULL\n");
		st->pw_entry = NULL;
		return NULL;
	}
	st->pw_entry = entry;
	evas_object_smart_callback_add(entry, "language,changed",
			__pw_entry_language_changed_cb, ad);

	ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	elm_entry_prediction_allow_set(entry, EINA_FALSE);
	elm_object_signal_emit(entry, "elm,action,hide,search_icon", "");
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
	elm_entry_select_allow_set(entry, EINA_TRUE);
	elm_entry_cursor_end_set(entry);
	snprintf(buf, sizeof(buf), STR_PW_GUIDE_TEXT, WIFI_PASSPHRASE_LENGTH_MIN);
	elm_object_part_text_set(entry, "elm.guide", buf);
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);

	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = WIFI_PASSPHRASE_LENGTH_MAX;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

	if (st->security_type_new == TETHERING_WIFI_SECURITY_TYPE_NONE) {
		elm_object_part_text_set(entry, "default", buf);
		elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
		elm_entry_select_allow_set(entry, EINA_FALSE);
		elm_object_disabled_set(entry, EINA_TRUE);
		elm_object_item_signal_emit(st->pw_item, "elm,state,rename,hide", "");
		return entry;
	} else {
		ptr = elm_entry_utf8_to_markup(st->wifi_passphrase_new);
		if (ptr != NULL) {
			elm_entry_entry_set(entry, ptr);
			free(ptr);
		} else {
			ERR("elm_entry_utf8_to_markup is failed\n");
		}
	}

	elm_entry_input_panel_return_key_type_set(st->pw_entry,
			ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	clr_btn = elm_button_add(entry);
	elm_object_style_set(clr_btn, "search_clear");
	elm_object_focus_allow_set(clr_btn, EINA_FALSE);
	elm_object_part_content_set(entry, "elm.swallow.clear", clr_btn);
	evas_object_smart_callback_add(clr_btn, "clicked", __eraser_btn_clicked_cb, entry);
	evas_object_show(clr_btn);

	evas_object_smart_callback_add(entry, "changed",
			__pw_entry_changed_cb, ad);
	evas_object_smart_callback_add(entry, "maxlength,reached",
			__pw_entry_maxlength_reached_cb, ad);
	evas_object_smart_callback_add(entry, "activated",
			__pw_entry_activated_cb, NULL);
	evas_object_smart_callback_add(entry, "focused",
			__pw_entry_focused_cb, NULL);
	evas_object_smart_callback_add(entry, "unfocused",
			__pw_entry_unfocused_cb, NULL);

	__MOBILE_AP_FUNC_EXIT__;
	return st->pw_entry;
}

static char *__gl_pw_text_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (strcmp(part, "elm.text.main") != 0) {
		return NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return strdup(STR_PASSWORD);
}

static Evas_Object *__gl_pw_content_get(void *data, Evas_Object *obj, const char *part)
{

	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("data is null \n");
		return NULL;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	Evas_Object * layout = NULL;
	Evas_Object * entry = NULL;

	if (g_strcmp0(part, "elm.icon.entry") != 0)
		return NULL;

	layout = elm_layout_add(obj);
	elm_layout_file_set(layout, FILE_PATH_OF_EDC, "entry_style");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	entry = __get_pw_entry(ad, layout);

	elm_object_part_content_set(layout, "entry_part", entry);
	__MOBILE_AP_FUNC_EXIT__;
	return layout;
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
	__hide_btn_changed_cb(ad, obj, NULL);

	elm_check_state_set(ad->setup.hide_btn, ad->setup.visibility_new ?
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
	elm_check_state_set(ad->setup.security_btn, ad->setup.security_type_new ==
			TETHERING_WIFI_SECURITY_TYPE_NONE ? EINA_FALSE : EINA_TRUE);

	__MOBILE_AP_FUNC_EXIT__;
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

	ad->setup.name_itc = elm_genlist_item_class_new();
	ad->setup.name_itc->item_style = "multiline_main.sub";
	ad->setup.name_itc->func.text_get = __gl_device_name_title_label_get;
	ad->setup.name_itc->func.content_get = NULL;
	ad->setup.name_itc->func.state_get = NULL;
	ad->setup.name_itc->func.del = NULL;

	ad->setup.hide_itc = elm_genlist_item_class_new();
	ad->setup.hide_itc->item_style = "1line"; 
	ad->setup.hide_itc->func.text_get = __gl_hide_label_get;
	ad->setup.hide_itc->func.content_get = __gl_hide_icon_get;
	ad->setup.hide_itc->func.state_get = NULL;
	ad->setup.hide_itc->func.del = NULL;

	ad->setup.security_itc = elm_genlist_item_class_new();
	ad->setup.security_itc->item_style = "1line";
	ad->setup.security_itc->func.text_get = __gl_security_label_get;
	ad->setup.security_itc->func.content_get = __gl_security_icon_get;
	ad->setup.security_itc->func.state_get = NULL;
	ad->setup.security_itc->func.del = NULL;

	ad->setup.pw_itc = elm_genlist_item_class_new();
	ad->setup.pw_itc->item_style = "entry.main";
	ad->setup.pw_itc->func.text_get = __gl_pw_text_get;
	ad->setup.pw_itc->func.content_get = __gl_pw_content_get;
	ad->setup.pw_itc->func.state_get = NULL;
	ad->setup.pw_itc->func.del = NULL;

	ad->setup.sp2_itc = elm_genlist_item_class_new();
	ad->setup.sp2_itc->item_style = "dialogue/separator.2";
	ad->setup.sp2_itc->func.text_get = NULL;
	ad->setup.sp2_itc->func.content_get = NULL;
	ad->setup.sp2_itc->func.state_get = NULL;
	ad->setup.sp2_itc->func.del = NULL;

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

	evas_object_smart_callback_del(st->hide_btn, "changed",
			__hide_btn_changed_cb);
	evas_object_smart_callback_del(st->security_btn, "changed",
			__security_btn_changed_cb);
	evas_object_smart_callback_del(st->cancel_button, "clicked",
			__cancel_btn_cb);
	evas_object_smart_callback_del(st->save_button, "clicked",
			__save_btn_cb);
	evas_object_smart_callback_del(st->genlist, "realized",
			__gl_realized);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __settings_reloaded_cb(tethering_error_e result, void *user_data)
{
	if (user_data == NULL) {
		ERR("Invalid parameter\n");
		return;
	}

	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	if (result != TETHERING_ERROR_NONE)
		ERR("tethering_wifi_reload_settings is failed [0x%X]\n", result);
	_update_wifi_item(ad, MH_STATE_NONE);

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __save_btn_cb(void *data, Evas_Object *object, void *event_info)
{
	DBG("+\n");
	if (data == NULL) {
		ERR("data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;
	bool ret = false;
	bool is_setting_changed = false;

	/* handle hide button change */
	if (st->visibility != st->visibility_new) {
		ret = __save_hide_btn_change(ad);
		if (ret == false) {
			elm_check_state_set(st->hide_btn, st->visibility ?
					EINA_FALSE : EINA_TRUE);
		} else {
			st->visibility = st->visibility_new;
			is_setting_changed = true;
		}
	}
	/* handle security button change */
	if (st->security_type != st->security_type_new) {
		ret = __save_security_btn_change(ad);
		if (ret == false) {
			elm_check_state_set(st->security_btn, st->security_type ==
					TETHERING_WIFI_SECURITY_TYPE_NONE ? EINA_FALSE : EINA_TRUE);
		} else {
				st->security_type = st->security_type_new;
				is_setting_changed = true;
		}
	}

	/* handle wifi passphrase change */
	if (strcmp(st->wifi_passphrase, st->wifi_passphrase_new)) {
		ret = __save_wifi_passphrase(ad);
		if (ret == false) {
			g_strlcpy(st->wifi_passphrase_new, st->wifi_passphrase,
					sizeof(st->wifi_passphrase_new));
			elm_genlist_item_update(st->pw_item);
		} else {
			g_strlcpy(st->wifi_passphrase, st->wifi_passphrase_new,
					sizeof(st->wifi_passphrase));
			is_setting_changed = true;
			if (ad->main.help_item) {
				elm_genlist_item_update(ad->main.help_item);
			}
		}
	}

	if (is_setting_changed) {
		_update_wifi_item(ad, MH_STATE_PROCESS);

		if (ad->main.help_item)
			elm_genlist_item_update(ad->main.help_item);

		/* reload wifi settings */
		ret = tethering_wifi_reload_settings(ad->handle, __settings_reloaded_cb,
				(void *)ad);
		if (ret != TETHERING_ERROR_NONE)
			ERR("reload_configuration is failed : %d\n", ret);
	}

	elm_naviframe_item_pop(ad->naviframe);

	DBG("-\n");
	return;
}

static void __cancel_btn_cb(void *data, Evas_Object *object, void *event_info)
{
	DBG("+\n");

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_naviframe_item_pop(ad->naviframe);
	DBG("-\n");
	return;
}

Eina_Bool _setting_back_btn_cb(void *data, Elm_Object_Item *navi_item)
{
	DBG("+\n");

	if (data == NULL) {
		ERR("The param is NULL\n");
		return EINA_FALSE;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	mh_wifi_setting_view_t *st = &ad->setup;

	if (st->visibility != st->visibility_new) {
		st->visibility_new = st->visibility;
	}

	if (st->security_type != st->security_type_new) {
		st->security_type_new = st->security_type;
	}

	if (strcmp(st->wifi_passphrase_new, st->wifi_passphrase)) {
		g_strlcpy(st->wifi_passphrase_new, st->wifi_passphrase,
					sizeof(st->wifi_passphrase_new));
	}
	st->pw_entry = NULL;
	__deconstruct_wifi_setup_view(st);

	if (ad->rename_popup != NULL) {
		evas_object_del(ad->rename_popup);
		ad->rename_popup = NULL;
	}
	ad->setup.navi_it = NULL;

	DBG("-\n");
	return EINA_TRUE;
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
	int no_of_sp;
	int i;

	if (item == st->hide_item) {
		elm_object_item_signal_emit(item, "elm,state,top", "");
	} else if (item == st->security_item) {
		elm_object_item_signal_emit(item, "elm,state,center", "");
	} else if (item == st->pw_item) {
		elm_object_item_signal_emit(item, "elm,state,bottom", "");
	} else if (item == st->name_item) {
	}

	no_of_sp = sizeof(st->sp_item) / sizeof(st->sp_item[0]);
	for (i = 0; i < no_of_sp; i++) {
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

	if (item == st->pw_item) {
		elm_object_item_access_unregister(item);
		ao = elm_object_item_access_register(item);
		snprintf(str, sizeof(str), "%s, %s", STR_PASSWORD, st->wifi_passphrase_new);
		elm_access_info_set(ao, ELM_ACCESS_CONTEXT_INFO, str);
	}

	__MOBILE_AP_FUNC_EXIT__;

	return;
}

static void __select_passphrase_item(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Evas_Object *layout = NULL;
	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	layout = elm_layout_add(obj);
	elm_layout_file_set(layout, FILE_PATH_OF_EDC, "entry_style");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	__get_pw_entry(ad, layout);
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

	genlist = elm_genlist_add(ad->naviframe);
	if (genlist == NULL) {
		ERR("genlist is NULL\n");
		return NULL;
	}

	elm_object_style_set(genlist, "dialogue");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "realized", __gl_realized, ad);

	__set_genlist_itc(ad);

	st->name_item = elm_genlist_item_append(genlist, st->name_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(st->name_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_object_item_disabled_set(st->name_item, EINA_TRUE);

	st->hide_item = elm_genlist_item_append(genlist, st->hide_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_hide_item_sel, data);

	st->security_item = elm_genlist_item_append(genlist, st->security_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __gl_security_item_sel, data);

	st->pw_item = elm_genlist_item_append(genlist, st->pw_itc, data, NULL,
			ELM_GENLIST_ITEM_NONE, __select_passphrase_item, ad);
	if (st->security_type == TETHERING_WIFI_SECURITY_TYPE_NONE)
		elm_object_item_disabled_set(st->pw_item, EINA_TRUE);
	else
		elm_object_item_disabled_set(st->pw_item, EINA_FALSE);

/* End separator is removed because of P131104-03336. It is confirmed by UX team for this case.
	item = elm_genlist_item_append(genlist, st->sp_itc, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	st->sp_item[i++] = item;
*/

	__MOBILE_AP_FUNC_EXIT__;

	return genlist;
}

void mh_draw_wifi_setup_view(mh_appdata_t *ad)
{
	DBG("+\n");

	if (ad == NULL) {
		ERR("ad is NULL\n");
		DBG("-\n");
		return;
	}
	Evas_Object *btn;
	mh_wifi_setting_view_t *st = &ad->setup;

	if (ad->setup.navi_it != NULL) {
		ERR("Wi-Fi setup view already exists\n");
		DBG("-\n");
		return;
	}

	st->genlist = __create_genlist(ad);
	if (st->genlist == NULL) {
		ERR("__create_genlist returns NULL\n");
		DBG("-\n");
		return;
	}

	st->navi_it = elm_naviframe_item_push(ad->naviframe, "IDS_MOBILEAP_MBODY_WI_FI_TETHERING",
			NULL, NULL, st->genlist, NULL);
	elm_object_item_domain_text_translatable_set(st->navi_it, PKGNAME, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(st->navi_it, _setting_back_btn_cb, (void *)ad);

	btn = elm_button_add(ad->naviframe);
	elm_object_style_set(btn, "naviframe/title_cancel");
	evas_object_smart_callback_add(btn, "clicked", __cancel_btn_cb, ad);
	elm_object_item_part_content_set(st->navi_it, "title_left_btn", btn);
	st->cancel_button = btn;

	btn = elm_button_add(ad->naviframe);
	elm_object_style_set(btn, "naviframe/title_done");
	evas_object_smart_callback_add(btn, "clicked", __save_btn_cb, ad);
	elm_object_item_part_content_set(st->navi_it, "title_right_btn", btn);
	st->save_button = btn;
	DBG("-\n");
}
