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
#include <efl_assist.h>

#include "mh_common_utility.h"
#include "mobile_hotspot.h"
#include "mh_popup.h"
#include "mh_string.h"

static mh_popup_type_e popup_type = MH_POPUP_NONE;
static Evas_Object *popup_content = NULL;
static char *popup_string = NULL;

static void __handle_one_btn(void *data)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	__MOBILE_AP_FUNC_EXIT__;
}

static void __popup_one_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	__handle_one_btn(data);
}

static void __popup_one_btn_mouse_event_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	Evas_Event_Mouse_Up *ev = event_info;

	if (ev->button == 3) {
		__handle_one_btn(data);
	}
}

static void __launch_network_app(void)
{
	int ret = APP_CONTROL_ERROR_NONE;
	app_control_h app_control;

	app_control_create(&app_control);
	if(ret != APP_CONTROL_ERROR_NONE) {
		ERR("app_control_create failed");
		return;
	}
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	app_control_set_window(app_control, elm_win_xwindow_get(ug_get_window()));
	app_control_set_app_id(app_control, "setting-network-efl");

	ret = app_control_send_launch_request(app_control, NULL, NULL);
	if(ret == APP_CONTROL_ERROR_NONE) {
		DBG("Launch network ug successful");
	} else {
		ERR("Fail to launch network ug");
	}

	app_control_destroy(app_control);
}

static void __handle_popup_resp(void *data, bool response)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;
	int ret = 0;
	Eina_Bool state = 0;

	if (ad->popup_checkbox) {
		state = elm_check_state_get(ad->popup_checkbox);
		evas_object_del(ad->popup_checkbox);
		ad->popup_checkbox = NULL;
		DBG("Checkbox status is = %d\n", state);
	}
	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

	if (response == true) {
		switch (popup_type) {
		case MH_POPUP_WIFI_ON_CHECKBOX:
			if(state) {
				_set_checkbox_status(TETHERING_TYPE_WIFI, 1);
			}
			ad->is_wifi_teth_enabling = false;
			ret = tethering_enable(ad->handle, TETHERING_TYPE_WIFI);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("wifi tethering on is failed : %d\n", ret);
				_update_wifi_item(ad, MH_STATE_NONE);
			}
			_wifi_tethering_checkbox_popup_status_set(false);
			_send_signal_qp(QP_SIGNAL_PROGRESS_ON);
			break;

		case MH_POPUP_BT_ON_CHECKBOX:
			if(state) {
				_set_checkbox_status(TETHERING_TYPE_BT, 1);
			}
			ad->is_bt_teth_enabling = false;
			ret = tethering_enable(ad->handle, TETHERING_TYPE_BT);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("Error enable bt tethering : %d\n", ret);
				_update_bt_item(ad, MH_STATE_NONE);
			}
			break;

		case MH_POPUP_USB_ON_CHECKBOX:
			if(state) {
				_set_checkbox_status(TETHERING_TYPE_USB, 1);
			}
			if (_get_vconf_usb_state() != VCONFKEY_SYSMAN_USB_AVAILABLE) {
				_update_tethering_item(ad, MH_STATE_NONE);
				break;
			}
			ad->is_usb_teth_enabling = false;
			ret = tethering_enable(ad->handle, TETHERING_TYPE_USB);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("Error enable usb tethering : %d\n", ret);
				_update_usb_item(ad, MH_STATE_NONE);
			}
			break;

		case MH_POPUP_WIFI_AP_OFF:
			ret = tethering_disable(ad->handle, TETHERING_TYPE_RESERVED);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("MobileAP off is failed : %d\n", ret);
			}
			break;

		case MH_POPUP_WIFI_OFF:
			ret = tethering_disable(ad->handle, TETHERING_TYPE_WIFI);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("wifi tethering off is failed : %d\n", ret);
				_update_wifi_item(ad, MH_STATE_NONE);
			}
			_send_signal_qp(QP_SIGNAL_PROGRESS_OFF);
			break;

		case MH_POPUP_MOBILE_DATA_OFF:
			__launch_network_app();
			_update_tethering_enabling_item(ad, ad->type, MH_STATE_NONE);
			break;

		default:
			ERR("Unknown popup_type : %d\n", popup_type);
			break;
		}
	} else {
		switch (popup_type) {
		case MH_POPUP_WIFI_ON_CHECKBOX:
			_update_tethering_enabling_item(ad, TETHERING_TYPE_WIFI, MH_STATE_NONE);
			_send_signal_qp(QP_SIGNAL_PROGRESS_RESET);
			_wifi_tethering_checkbox_popup_status_set(false);
			break;

		case MH_POPUP_BT_ON_CHECKBOX:
			_update_tethering_enabling_item(ad, TETHERING_TYPE_BT, MH_STATE_NONE);
			break;

		case MH_POPUP_USB_ON_CHECKBOX:
			_update_tethering_enabling_item(ad,TETHERING_TYPE_USB, MH_STATE_NONE);
			break;

		case MH_POPUP_WIFI_AP_OFF:
			if (ad->setup.genlist)
				elm_naviframe_item_pop(ad->naviframe);
			elm_naviframe_item_pop(ad->naviframe);
			break;

		case MH_POPUP_WIFI_OFF:
			_update_tethering_enabling_item(ad, TETHERING_TYPE_WIFI, MH_STATE_NONE);
			_send_signal_qp(QP_SIGNAL_PROGRESS_RESET);
			break;

		case MH_POPUP_MOBILE_DATA_OFF:
			_update_tethering_enabling_item(ad, ad->type, MH_STATE_NONE);
			break;
		default:
			ERR("Unknown popup_type : %d\n", popup_type);
			break;
		}
	}

	__MOBILE_AP_FUNC_EXIT__;
}

void _teth_on(mh_appdata_t *ad)
{
	__handle_popup_resp(ad, true);
}

static void __popup_yes_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	__handle_popup_resp(data, true);
}

static void __popup_no_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	__handle_popup_resp(data, false);
}

static void __popup_no_btn_mouse_event_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (data == NULL || obj == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	Evas_Event_Mouse_Up *ev = event_info;

	if (ev->button == 3) {
		__handle_popup_resp(data, false);
	}
}

void _prepare_popup_with_content(int type, Evas_Object *obj)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (obj == NULL) {
		ERR("param is NULL\n");
		return;
	}

	popup_type = type;

	if (popup_content)
		evas_object_del(popup_content);
	popup_content = obj;

	if (popup_string) {
		free(popup_string);
		popup_string = NULL;
	}

	__MOBILE_AP_FUNC_EXIT__;
}

void _prepare_popup(int type, const char *str)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (str == NULL) {
		ERR("param is NULL\n");
		return;
	}
	popup_type = type;

	popup_content = NULL;

	if (popup_string)
		free(popup_string);

	popup_string = strndup(str, MH_LABEL_LENGTH_MAX);
	if (popup_string == NULL)
		ERR("strndup is failed\n");

	__MOBILE_AP_FUNC_EXIT__;
}

void _prepare_popup_type(int type)
{
	popup_type = type;
}

static void __language_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_appdata_t *ad = (mh_appdata_t *)data;
	Evas_Object *label = NULL;
	bool wifi_state = false;
	char *fmt = NULL;
	char str[MH_LABEL_LENGTH_MAX];

	if (ad->type < TETHERING_TYPE_USB || ad->type > TETHERING_TYPE_BT ) {
		ERR("Invalid Tethering type\n");
		return;
	}

	switch(ad->type) {
	case TETHERING_TYPE_WIFI:
		wifi_is_activated(&wifi_state);
		_set_vconf_prev_wifi_state(wifi_state);
		if (wifi_state == true || _is_wifi_direct_on() == true) {
			fmt = STR_TETH_ON_DESC_1;
		} else {
			fmt = STR_TETH_ON_DESC_2;
		}
		snprintf(str, MH_LABEL_LENGTH_MAX, fmt, TETHERING_WIFI_MAX_CONNECTED_STA);
		break;

	case TETHERING_TYPE_BT:
		snprintf(str, MH_LABEL_LENGTH_MAX, STR_TETH_ON_DESC_2, TETHERING_BT_MAX_CONNECTED_STA);
		break;

	case TETHERING_TYPE_USB:
		snprintf(str, MH_LABEL_LENGTH_MAX, STR_TETH_ON_DESC_3);
		break;
	default:
		break;
	}

	label = elm_label_add(ad->main.check_popup_ly);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, str);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);
	elm_object_part_content_set(ad->main.check_popup_ly, "elm.swallow.content", label);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __popup_with_checkbox(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Object *popup = NULL;
	Evas_Object *yes_button = NULL;
	Evas_Object *cancel_button = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *label = NULL;
	Evas_Object *check = NULL;

	popup = elm_popup_add(ad->win);
	ad->popup = popup;
	switch (ad->type) {
	case TETHERING_TYPE_WIFI:
		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
				"IDS_MOBILEAP_MBODY_WI_FI_TETHERING");
		break;
	case TETHERING_TYPE_BT:
		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
				"IDS_MOBILEAP_BODY_BLUETOOTH_TETHERING");
		break;
	case TETHERING_TYPE_USB:
		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
				"IDS_MOBILEAP_MBODY_USB_TETHERING");
		break;
	default:
		break;
	}

	/* layout */
	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, FILE_PATH_OF_EDC, "popup_checkview_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ad->main.check_popup_ly = layout;

	/* check */
	check = elm_check_add(popup);
	elm_object_style_set(check, "popup");
	elm_object_domain_translatable_text_set(check, PACKAGE, STR_NO_SHOW_AGAIN);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "elm.swallow.end", check);
	ad->popup_checkbox = check;

	/* label */
	label = elm_label_add(layout);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, popup_string);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);
	elm_object_part_content_set(layout, "elm.swallow.content", label);

	cancel_button = _create_button(popup,  "IDS_BR_SK_CANCEL",
				"button1", __popup_no_btn_clicked_cb, ad);
	yes_button = _create_button(popup, "IDS_ST_BUTTON_ENABLE_M_USE",
				"button2", __popup_yes_btn_clicked_cb, ad);

	evas_object_smart_callback_add(popup, "language,changed", __language_changed_cb, ad);

	ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
			__popup_no_btn_clicked_cb, (void *)ad);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
			__popup_no_btn_mouse_event_cb, (void *)ad);

	if (check && yes_button) {
		elm_object_focus_next_object_set(check, yes_button, ELM_FOCUS_PREVIOUS);
		elm_object_focus_next_object_set(yes_button, check, ELM_FOCUS_NEXT);
		if (cancel_button) {
			elm_object_focus_next_object_set(cancel_button, check, ELM_FOCUS_PREVIOUS);
			elm_object_focus_next_object_set(check ,cancel_button, ELM_FOCUS_NEXT);
		}
	}

	elm_object_content_set(popup, layout);
	evas_object_show(popup);
	ad->popup = popup;

	__MOBILE_AP_FUNC_EXIT__;
}

Eina_Bool _create_popup(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	Evas_Object *popup = NULL;
	Evas_Object *cancel_button = NULL;
	Evas_Object *yes_button = NULL;
	Evas_Object *label = NULL;

	if (ad == NULL) {
		ERR("The param is NULL\n");
		if (popup_string) {
			free(popup_string);
			popup_string = NULL;
		}
		popup_content = NULL;

		return EINA_FALSE;
	}

	if (ad->popup != NULL) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	ad->popup_checkbox = NULL;
	DBG("popup type : %d", popup_type);

	switch (popup_type) {
	case MH_POPUP_WIFI_ON_CHECKBOX:
	case MH_POPUP_BT_ON_CHECKBOX:
	case MH_POPUP_USB_ON_CHECKBOX:
		__popup_with_checkbox(ad);
		break;

	case MH_POPUP_WIFI_OFF:
		popup = elm_popup_add(ad->win);
		ad->popup = popup;

		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
				"IDS_MOBILEAP_MBODY_WI_FI_TETHERING");

		evas_object_size_hint_weight_set(popup,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		if (popup_content == NULL)
			elm_object_domain_translatable_text_set(popup, PACKAGE, popup_string);
		else
			elm_object_content_set(popup, popup_content);

		cancel_button =_create_button(popup, STR_CANCEL,
				"button1", __popup_no_btn_clicked_cb, ad);
		yes_button = _create_button(popup, STR_TURN_OFF,
				"button2", __popup_yes_btn_clicked_cb, ad);

		ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
				__popup_no_btn_clicked_cb, (void *)ad);
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
				__popup_no_btn_mouse_event_cb, (void *)ad);

		evas_object_show(popup);
		break;

	case MH_POPUP_TETH_ENABLING_FAILED:
	case MH_POPUP_NO_SIM:
	case MH_POPUP_FLIGHT_MODE:
	case MH_POPUP_NETWORK_OUT_OF_RANGE:
		popup = elm_popup_add(ad->win);
		ad->popup = popup;
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);
		switch (popup_type) {
		case MH_POPUP_NO_SIM:
			if (ad->type == TETHERING_TYPE_WIFI) {
				elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
						STR_SIM_CARD_ERROR);
			} else if (ad->type == TETHERING_TYPE_RESERVED) {
				elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
						STR_UNABLE_TO_USE_TETH_HEADER);
			} else {
				elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
						STR_CONN_TO_MOB_NET);
			}
			break;

		case MH_POPUP_FLIGHT_MODE:
			if (ad->type == TETHERING_TYPE_RESERVED) {
				elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
						STR_UNABLE_TO_USE_TETH_HEADER);
			} else {
				elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
						STR_FLIGHT_MODE_HEADER);
			}
			break;

		case MH_POPUP_NETWORK_OUT_OF_RANGE:
			elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
					STR_DATA_USAGE_LIMIT_REACHED);
			break;

		case MH_POPUP_TETH_ENABLING_FAILED:

		default:
			DBG("Invalid option \n");
			break;
		}
		label = elm_label_add(popup);
		elm_object_style_set(label, "popup/default");
		elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
		elm_object_domain_translatable_text_set(label, PACKAGE, popup_string);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL,
								EVAS_HINT_FILL);
		elm_object_content_set(popup, label);
		yes_button = _create_button(popup,  STR_OK,
				"button1", __popup_one_btn_clicked_cb, ad);

		ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
				__popup_one_btn_clicked_cb, (void *)ad);
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
				__popup_one_btn_mouse_event_cb, (void *)ad);
		evas_object_show(popup);
		break;

	case MH_POPUP_MOBILE_DATA_OFF:
		popup = elm_popup_add(ad->win);
		ad->popup = popup;
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);

		if (popup_content == NULL)
			elm_object_domain_translatable_text_set(popup, PACKAGE, popup_string);
		else
			elm_object_content_set(popup, popup_content);
		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
				STR_NO_NET_CONN);
		cancel_button = _create_button(popup,  STR_CANCEL,
				"button1", __popup_no_btn_clicked_cb, ad);
		yes_button = _create_button(popup, STR_SETTING,
				"button2", __popup_yes_btn_clicked_cb, ad);

		ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
				__popup_no_btn_clicked_cb, (void *)ad);
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
				__popup_no_btn_mouse_event_cb, (void *)ad);

		evas_object_show(popup);
		break;

	case MH_POPUP_WIFI_AP_OFF:
		popup = elm_popup_add(ad->win);
		ad->popup = popup;
		evas_object_size_hint_weight_set(popup,
				EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE,
				STR_TETH);
		if (popup_content == NULL)
			elm_object_domain_translatable_text_set(popup, PACKAGE, popup_string);
		else
			elm_object_content_set(popup, popup_content);

		cancel_button = _create_button(popup,  STR_CANCEL,
				"button1", __popup_no_btn_clicked_cb, ad);
		yes_button = _create_button(popup, STR_ENABLE,
				"button2", __popup_yes_btn_clicked_cb, ad);

		ea_object_event_callback_add(popup, EA_CALLBACK_BACK,
				__popup_no_btn_clicked_cb, (void *)ad);
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
				__popup_no_btn_mouse_event_cb, (void *)ad);

		evas_object_show(popup);
		break;

	default:
		ERR("Unknown popup_type : %d\n", popup_type);
		break;
	}

	if (cancel_button && yes_button) {
		elm_object_focus_next_object_set(cancel_button, yes_button, ELM_FOCUS_PREVIOUS);
		elm_object_focus_next_object_set(yes_button, cancel_button, ELM_FOCUS_NEXT);
	}

	if (popup_string) {
		free(popup_string);
		popup_string = NULL;
	}
	popup_content = NULL;

	__MOBILE_AP_FUNC_EXIT__;

	return EINA_TRUE;
}

void _destroy_popup(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is null\n");
		return;
	}

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}
	ad->popup_checkbox = NULL;

	if (popup_string) {
		free(popup_string);
		popup_string = NULL;
	}
	popup_content = NULL;
	popup_type = MH_POPUP_NONE;

	__MOBILE_AP_FUNC_EXIT__;

	return;
}
