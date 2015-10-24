/*
 * mh_device_rename.c
 *
 *  Created on: 26-May-2014
 *      Author: sach.sharma
 */

#include <Elementary.h>
#include <vconf.h>
#include <efl_extension.h>

#include "mh_view_main.h"
#include "mh_string.h"
#include "mh_popup.h"

static Eina_Bool __is_portrait_mode = EINA_TRUE;
static int (*rotate_cb)(Eina_Bool , void*) = NULL;
static void *rotate_cb_data = NULL;
static void __set_rotate_cb(int (*cb)(Eina_Bool, void*) , void *data);

static void __rename_popup_keyback_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;

	elm_object_focus_set(ad->rename_entry, EINA_FALSE);

	if (ad->rename_popup) {
		evas_object_del(ad->rename_popup);
	}
	ad->rename_popup = NULL;
	ad->rename_entry = NULL;
	__set_rotate_cb(NULL, NULL);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __rename_popup_mouseup_cb(void *data,
		Evas *e, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Event_Mouse_Up *ev = event_info;
	mh_appdata_t *ad = (mh_appdata_t *)data;

	if (ev->button == 3) {
		elm_object_focus_set(ad->rename_entry, EINA_FALSE);
		if (ad->rename_popup) {
			evas_object_del(ad->rename_popup);
		}
		ad->rename_popup = NULL;
		ad->rename_entry = NULL;
	}
	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static bool __is_space_str(const char *str)
{
	while (str) {
		if (*str != '\0' && *str != ' ') {
			return FALSE;
		} else if (*str == '\0') {
			return TRUE;
		}
		str++;
	}
	return TRUE;
}

static void __rename_popop_entry_changed_cb(void *data, Evas_Object *obj,
				void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;
	mh_appdata_t *ad = (mh_appdata_t *)data;

	const char *entry_text 	= NULL;
	char *input_str 		= NULL;
	bool is_space_string = FALSE;

	entry_text = elm_entry_entry_get(obj);
	input_str = elm_entry_markup_to_utf8(entry_text);

	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (elm_object_focus_get(obj)) {
			if (elm_entry_is_empty(obj))
				elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
			else
				elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		}
	}


	if (input_str == NULL || (strlen(input_str) == 0)) {
		elm_object_disabled_set(ad->rename_button, TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, TRUE);
		if (input_str) {
			free(input_str);
		}
		return;
	}

	is_space_string = __is_space_str(input_str);
	if (is_space_string) {
		elm_object_disabled_set(ad->rename_button, TRUE);
		elm_entry_input_panel_return_key_disabled_set(ad->rename_entry, TRUE);
		free(input_str);
		return;
	}

	if (strlen(input_str) > 0) {
		elm_object_disabled_set(ad->rename_button, FALSE);
		elm_entry_input_panel_return_key_disabled_set(ad->rename_entry, FALSE);
	}
	if (input_str) {
		free(input_str);
	}
	__MOBILE_AP_FUNC_EXIT__;
}

static void __device_name_maxlength_reached_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	char buf[MH_LABEL_LENGTH_MAX] = { 0, };
	notification_error_e ret;

	snprintf(buf, sizeof(buf), STR_PW_MAX_LEN_WARN_MSG, DEVICE_NAME_LENGTH_MAX);
	ret = notification_status_message_post(buf);
	if (ret != NOTIFICATION_ERROR_NONE) {
		ERR("notification_status_message_post() is failed : %d\n", ret);
	}

	__MOBILE_AP_FUNC_EXIT__;
}

static void __entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (!elm_entry_is_empty(obj))
			elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		else
			elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	}
	elm_object_signal_emit(obj, "elm,state,focus,on", "");

	__MOBILE_AP_FUNC_EXIT__;
}

static void __entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (elm_object_part_content_get(obj, "elm.swallow.clear"))
		elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	elm_object_signal_emit(obj, "elm,state,focus,off", "");

	__MOBILE_AP_FUNC_EXIT__;
}

static void __eraser_btn_clicked_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("data is null\n");
		return;
	}
	mh_appdata_t *ad = (mh_appdata_t*) data;

	elm_entry_entry_set(ad->rename_entry, "");
	elm_object_disabled_set(ad->rename_button, TRUE);
	elm_entry_input_panel_return_key_disabled_set(ad->rename_entry, TRUE);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __rename_popup_cancel_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("data is null\n");
		return;
	}
	mh_appdata_t *ad = (mh_appdata_t*) data;

	if (ad->rename_popup) {
		evas_object_del(ad->rename_popup);
		ad->rename_popup = NULL;
	}
	if (ad->rename_entry) {
		evas_object_del(ad->rename_entry);
		ad->rename_entry = NULL;
	}
	__set_rotate_cb(NULL, NULL);

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __rename_popup_ok_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL || obj == NULL) {
		ERR("data is null\n");
		return;
	}
	mh_appdata_t *ad = (mh_appdata_t*) data;
	char *device_name_str = NULL;
	const char *entry_str = elm_entry_entry_get(ad->rename_entry);

	device_name_str = elm_entry_markup_to_utf8(entry_str);
	if (device_name_str == NULL) {
		ERR("elm_entry_utf8_to_markup is failed\n");
		return;
	}

	DBG("New device name:[%s]\n", device_name_str);
	__set_rotate_cb(NULL, NULL);
	if (ad->rename_popup) {
		evas_object_del(ad->rename_popup);
		ad->rename_popup = NULL;
	}
	if (ad->rename_entry) {
		evas_object_del(ad->rename_entry);
		ad->rename_entry = NULL;
	}
	if (g_strcmp0(ad->setup.device_name, device_name_str)) {
		if (vconf_set_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR, device_name_str) != 0) {
			DBG("Set vconf[%s] failed\n",VCONFKEY_SETAPPL_DEVICE_NAME_STR);
		}

		g_strlcpy(ad->setup.device_name, entry_str,
				sizeof(ad->setup.device_name));

		if (ad->main.help_item != NULL) {
			if (ad->main.help_item)
				elm_genlist_item_update(ad->main.help_item);
		}

		if (ad->setup.name_item != NULL) {
			if (ad->setup.name_item)
				elm_genlist_item_update(ad->setup.name_item);
		}
	}
	free(device_name_str);

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __enter_key_down_cb(void *data, Evas *evas, Evas_Object *obj,
				void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Event_Key_Down *ev = event_info;

	if (g_strcmp0(ev->key, "KP_Enter") == 0 ||
			g_strcmp0(ev->key, "Return") == 0) {
		Ecore_IMF_Context *imf_context;
		elm_entry_cursor_end_set(obj);
		imf_context = (Ecore_IMF_Context*)elm_entry_imf_context_get(obj);
		if (imf_context)
			ecore_imf_context_input_panel_hide(imf_context);
		elm_object_focus_set(obj, EINA_FALSE);
	}
	__MOBILE_AP_FUNC_EXIT__;
}

static void __set_rotate_cb(int (*cb)(Eina_Bool, void*), void *data)
{
	rotate_cb = cb;
	rotate_cb_data = data;
}

Eina_Bool __is_current_mode_portrait(void)
{
	return __is_portrait_mode;
}

static void __set_portrait_mode(Eina_Bool is_portrait)
{
	__is_portrait_mode = is_portrait;
}


void _hadnle_rename_popup_rotation(Eina_Bool is_portrait)
{
	__MOBILE_AP_FUNC_ENTER__;

	__set_portrait_mode(is_portrait);
	if (rotate_cb) {
		rotate_cb(is_portrait, rotate_cb_data);
	}
	__MOBILE_AP_FUNC_EXIT__;
}

static Evas_Object *__rename_entry_icon_get(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Object *entry = NULL;
	Evas_Object *button = NULL;
	char *ptr = NULL;
	char *device_name_utf = NULL;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	if (data == NULL) {
		ERR("data is null\n");
		return NULL;
	}
	mh_appdata_t *ad = (mh_appdata_t*) data;

	if (strcmp(part, "elm.icon.entry")) {
		return NULL;
	}

	entry = elm_entry_add(obj);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	limit_filter_data.max_char_count = DEVICE_NAME_LENGTH_MAX;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	elm_object_signal_emit(entry, "elm,action,hide,search_icon", "");
	elm_object_domain_translatable_part_text_set(entry, "elm.guide", PACKAGE,
			STR_DEVICE_NAME);

	elm_entry_input_panel_return_key_type_set(entry,
			ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
	ad->rename_entry = entry;

	device_name_utf = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
	if (device_name_utf == NULL) {
		ERR("vconf_get_str failed \n");
		return NULL;
	}
	ptr = elm_entry_utf8_to_markup(device_name_utf);
	if (ptr == NULL) {
		ERR("elm_entry_utf8_to_markup is failed\n");
		free(device_name_utf);
		return NULL;
	} else {
		elm_entry_entry_set(entry, ptr);
	}
	free(device_name_utf);
	free(ptr);
	elm_entry_cursor_end_set(entry);

	evas_object_smart_callback_add(entry, "changed,user", __rename_popop_entry_changed_cb, ad);
	evas_object_smart_callback_add(entry, "changed", __rename_popop_entry_changed_cb, ad);
	evas_object_smart_callback_add(entry, "preedit,changed",
			__rename_popop_entry_changed_cb, ad);

	evas_object_smart_callback_add(entry, "maxlength,reached",
			__device_name_maxlength_reached_cb, ad);
	evas_object_smart_callback_add(entry, "focused", __entry_focused_cb, NULL);
	evas_object_smart_callback_add(entry, "unfocused", __entry_unfocused_cb, NULL);
	evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_DOWN,
			__enter_key_down_cb, ad);

	button = elm_button_add(obj);
	elm_object_style_set(button, "search_clear");
	elm_object_focus_allow_set(button, EINA_FALSE);
	elm_object_part_content_set(entry, "elm.swallow.clear", button);
	evas_object_smart_callback_add(button, "clicked", __eraser_btn_clicked_cb, ad);
	evas_object_show(entry);
	elm_object_focus_set(entry, EINA_TRUE);
	return entry;
}

static char *__rename_desc_label_get(void *data, Evas_Object *obj,
					const char *part)
{
	if (!strcmp(part, "elm.text.multiline" )) {
		return g_strdup(STR_RENAME_DEVICE_MSG);
	}
	return NULL;
}

void _create_rename_device_popup(void *data)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("data is null\n");
		__MOBILE_AP_FUNC_EXIT__;
		return;
	}

	Evas_Object *popup = NULL;
	Evas_Object *button = NULL;
	Evas_Object *rename_btn = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *item = NULL;
	Evas_Object *layout = NULL;
	mh_appdata_t *ad = (mh_appdata_t*) data;

	if (ad->rename_popup) {
		evas_object_del(ad->rename_popup);
		ad->rename_popup = NULL;
	}

	popup = elm_popup_add(ad->layout);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
			"IDS_ST_HEADER_RENAME_DEVICE");
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
			__rename_popup_keyback_cb, (void *)ad);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
			__rename_popup_mouseup_cb, (void *)ad);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, FILE_PATH_OF_EDC, "rename_device_ly");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);

	/* Entry genlist item */
	ad->setup.rename_entry_itc = elm_genlist_item_class_new();
	if (ad->setup.rename_entry_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		__MOBILE_AP_FUNC_EXIT__;
		return;
	}
	ad->setup.rename_entry_itc->item_style = "entry";
	ad->setup.rename_entry_itc->func.text_get = NULL;
	ad->setup.rename_entry_itc->func.content_get = __rename_entry_icon_get;
	ad->setup.rename_entry_itc->func.state_get = NULL;
	ad->setup.rename_entry_itc->func.del = NULL;

	elm_genlist_item_append(genlist, ad->setup.rename_entry_itc, ad,
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	/* Description genlist item */
	ad->setup.rename_descp_itc = elm_genlist_item_class_new();
	if (ad->setup.rename_descp_itc == NULL) {
		ERR("elm_genlist_item_class_new failed\n");
		__MOBILE_AP_FUNC_EXIT__;
		return;
	}
	ad->setup.rename_descp_itc->item_style = "multiline_sub";
	ad->setup.rename_descp_itc->func.text_get = __rename_desc_label_get;
	ad->setup.rename_descp_itc->func.content_get = NULL;
	ad->setup.rename_descp_itc->func.state_get = NULL;
	ad->setup.rename_descp_itc->func.del = NULL;
	item = elm_genlist_item_append(genlist, ad->setup.rename_descp_itc, NULL, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	button = elm_button_add(popup);
	elm_object_style_set(button, "popup");
	elm_object_text_set(button, STR_CANCEL);
	elm_object_domain_translatable_text_set(button, PACKAGE, STR_CANCEL);
	elm_object_part_content_set(popup, "button1", button);
	evas_object_smart_callback_add(button, "clicked",
			__rename_popup_cancel_cb, ad);
	evas_object_show(button);

	rename_btn = elm_button_add(popup);
	ad->rename_button = rename_btn;
	elm_object_style_set(rename_btn, "popup");
	elm_object_domain_translatable_text_set(rename_btn, PACKAGE, STR_RENAME_DEVICE);
	elm_object_part_content_set(popup, "button2", rename_btn);
	evas_object_smart_callback_add(rename_btn, "clicked", __rename_popup_ok_cb, ad);
	evas_object_show(rename_btn);

	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	evas_object_show(genlist);
	elm_object_part_content_set(layout, "elm.swallow.layout", genlist);
	evas_object_show(layout);
	elm_object_content_set(popup, layout);

	evas_object_show(popup);
	ad->rename_popup = popup;
	elm_object_focus_set(ad->rename_entry, EINA_TRUE);

	__MOBILE_AP_FUNC_EXIT__;
	return;
}
