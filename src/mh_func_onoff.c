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

#include <stdlib.h>
#include <glib.h>

#include "mh_func_onoff.h"
#include "mh_popup.h"
#include "mh_string.h"

static bool is_wifi_tethering_checkbox_popup_active = false;

void _update_tethering_enabling_item(mh_appdata_t *ad, tethering_type_e type, mh_state_e state)
{
	switch(type) {
	case TETHERING_TYPE_WIFI:
		_update_wifi_item(ad, MH_STATE_NONE);
		ad->is_wifi_teth_enabling = false;
		break;

	case TETHERING_TYPE_BT:
		_update_bt_item(ad, MH_STATE_NONE);
		ad->is_bt_teth_enabling = false;
		break;

	case TETHERING_TYPE_USB:
		_update_usb_item(ad, MH_STATE_NONE);
		ad->is_usb_teth_enabling = false;
		break;

	default :
		ERR("invalid type \n");
		break;
	}
}
void _wifi_tethering_checkbox_popup_status_set(bool value)
{
	is_wifi_tethering_checkbox_popup_active = value;
}

bool _wifi_tethering_checkbox_popup_status_get(void)
{
	return is_wifi_tethering_checkbox_popup_active;
}

int _get_vconf_usb_state()
{
	int value = VCONFKEY_SYSMAN_USB_DISCONNECTED;

	if (vconf_get_int(VCONFKEY_SYSMAN_USB_STATUS, &value) < 0) {
		ERR("vconf_get_int is failed\n");
		return 0;
	}
	DBG("%s : %d\n", VCONFKEY_SYSMAN_USB_STATUS, value);

	return value;
}

void _update_tethering_item(mh_appdata_t * ad, mh_state_e state)
{
	ERR("type : %d state : %d\n", ad->type, state);
	switch(ad->type)	{
		case TETHERING_TYPE_WIFI:
			_update_wifi_item(ad, state);
			break;
		case TETHERING_TYPE_BT:
			_update_bt_item(ad, state);
			break;
		case TETHERING_TYPE_USB:
			_update_usb_item(ad, state);
			break;
		default:
			break;
	}
}

gboolean _ps_recheck_timeout_cb(gpointer data)
{
	connection_cellular_state_e cellular_state = _get_cellular_state();
	mh_appdata_t *ad = (mh_appdata_t *)data;
	static int recheck_count = 0;

	DBG("Re-Check cellular state (%d)\n", recheck_count);

	if(cellular_state == CONNECTION_CELLULAR_STATE_FLIGHT_MODE) {
		_update_tethering_item(ad, MH_STATE_NONE);
		recheck_count = 0;
		return FALSE;
	}

	if (cellular_state == CONNECTION_CELLULAR_STATE_CONNECTED ||
		cellular_state == CONNECTION_CELLULAR_STATE_AVAILABLE) {
		if (ad->type == TETHERING_TYPE_WIFI) {
			if (_create_wifi_hotspot_on_popup(ad) < 0) {
				ERR("__create_wifi_hotspot_on_popup fail\n");
				_update_tethering_item(ad, MH_STATE_NONE);
				recheck_count = 0;
				return FALSE;
			}
		_send_signal_qp(QP_SIGNAL_PROGRESS_ON);
		} else if (ad->type == TETHERING_TYPE_BT && _create_bt_tethering_on_popup(ad) < 0) {
				ERR("_create_bt_tethering_on_popup fail\n");
				_update_tethering_item(ad, MH_STATE_NONE);
				recheck_count = 0;
				return FALSE;
		} else if (ad->type == TETHERING_TYPE_USB && _create_usb_tethering_on_popup(ad) < 0) {
				ERR("__create_usb_hotspot_on_popup fail\n");
				_update_tethering_item(ad, MH_STATE_NONE);
				recheck_count = 0;
				return FALSE;
		} else {
			ERR("Unknown tethering type \n");
			recheck_count = 0;
			return FALSE;
		}
	} else {
		if(++recheck_count >= PS_RECHECK_COUNT_MAX) {
			DBG("Cellular network is not connected : %d\n", cellular_state);
			_update_tethering_item(ad, MH_STATE_NONE);
			_prepare_popup(MH_POPUP_NETWORK_OUT_OF_RANGE, STR_NO_DATA_SERVICE);
			_create_popup(ad);
			recheck_count = 0;
			return FALSE;
		}
		return TRUE;
	}
	recheck_count = 0;
	return FALSE;
}

static bool __is_connected_wifi_net(mh_appdata_t *ad)
{
	connection_wifi_state_e wifi_state = CONNECTION_WIFI_STATE_DEACTIVATED;
	int ret;

	ret = connection_get_wifi_state(ad->conn_handle, &wifi_state);
	if (ret != CONNECTION_ERROR_NONE) {
		ERR("connection_get_wifi_state() is failed : %d\n");
		return false;
	}

	if (wifi_state != CONNECTION_WIFI_STATE_CONNECTED) {
		ERR("Wi-Fi network is not connected : %d\n", wifi_state);
		return false;
	}

	DBG("Wi-Fi network is connected\n");
	return true;
}

static bool __is_connected_ethernet_net(mh_appdata_t *ad)
{
	connection_ethernet_state_e ethernet_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;
	int ret;

	ret = connection_get_ethernet_state(ad->conn_handle, &ethernet_state);
	if (ret != CONNECTION_ERROR_NONE) {
		ERR("connection_get_ethernet_state() is failed : %d\n");
		return false;
	}

	if (ethernet_state != CONNECTION_ETHERNET_STATE_CONNECTED) {
		ERR("Ethernet network is not connected : %d\n", ethernet_state);
		return false;
	}

	DBG("Ethernet network is connected\n");
	return true;
}

static int __is_preconditions_handled(mh_appdata_t *ad)
{
	DBG("+\n");

	connection_cellular_state_e cellular_state = CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	int dnet_state = 0;

	/* check ethernet connection */
	if (__is_connected_ethernet_net(ad)) {
		DBG("ethernet network is connected\n");
		return 1;
	}

	/*  check wifi connection */
	if (ad->type != TETHERING_TYPE_WIFI) {
		if (__is_connected_wifi_net(ad)) {
			DBG("wifi network is connected\n");
			return 1;
		}
	}

	/* Check SIM state */
	if (_get_sim_state() != VCONFKEY_TELEPHONY_SIM_INSERTED) {
		if(ad->type == TETHERING_TYPE_WIFI)
			_prepare_popup(MH_POPUP_NO_SIM, STR_INSERT_SIM_TO_USE_TETH);
		else
			_prepare_popup(MH_POPUP_NO_SIM, STR_CONN_MOBILE_DATA_TO_USE_TETH);

		_create_popup(ad);
		return -1;
	}

	/* Check cellular state */
	cellular_state = _get_cellular_state();
	DBG("cellular state is :  %d \n", cellular_state);
	if (cellular_state == CONNECTION_CELLULAR_STATE_FLIGHT_MODE) {
		_prepare_popup(MH_POPUP_FLIGHT_MODE, STR_DISABLE_FLIGHT_MODE_MSG);
		_create_popup(ad);
		ERR("Flight mode is ON\n");
		return -1;
	} else {
		if (vconf_get_bool(VCONFKEY_3G_ENABLE , &dnet_state) < 0) {
			ERR("vconf_get_bool is failed\n");
		} else if(dnet_state == 0) {
			DBG("Data Network is not connected");
			_prepare_popup(MH_POPUP_MOBILE_DATA_OFF, STR_NO_NET_CONN_MSG);
			_create_popup(ad);
			return 0;
		}
		if (cellular_state != CONNECTION_CELLULAR_STATE_CONNECTED &&
			cellular_state != CONNECTION_CELLULAR_STATE_AVAILABLE) {
			if(ad->ps_recheck_timer_id > 0) {
				g_source_remove(ad->ps_recheck_timer_id);
				ad->ps_recheck_timer_id = 0;
				if (ad->is_wifi_teth_enabling == true && ad->type != TETHERING_TYPE_WIFI) {
					_update_tethering_enabling_item(ad, TETHERING_TYPE_WIFI, MH_STATE_NONE);
				}
				if (ad->is_bt_teth_enabling == true && ad->type != TETHERING_TYPE_BT) {
					_update_tethering_enabling_item(ad, TETHERING_TYPE_BT, MH_STATE_NONE);
				}
				if (ad->is_usb_teth_enabling == true && ad->type != TETHERING_TYPE_USB) {
					_update_tethering_enabling_item(ad, TETHERING_TYPE_USB, MH_STATE_NONE);
				}
			}
			ad->ps_recheck_timer_id = g_timeout_add(PS_RECHECK_INTERVAL, _ps_recheck_timeout_cb, (void*)ad);
			return 0;
		}
	}

	DBG("Cellular network is connected\n");
	DBG("-\n");
	return 1;
}

int _create_wifi_hotspot_on_popup(mh_appdata_t *ad)
{
	char *fmt = NULL;
	char *str = NULL;
	bool wifi_state = false;
	int value = 0;

	wifi_is_activated(&wifi_state);
	_set_vconf_prev_wifi_state(wifi_state);
	value = _get_checkbox_status(TETHERING_TYPE_WIFI);
	if (0 == value) {
		if (wifi_state == true || _is_wifi_direct_on() == true) {
			fmt = STR_TETH_ON_DESC_1;
		} else {
			fmt = STR_TETH_ON_DESC_2;
		}
		str = g_malloc0(MH_LABEL_LENGTH_MAX);
		if (str == NULL) {
			ERR("memory allocation is failed\n");
			return -1;
		}
		snprintf(str, MH_LABEL_LENGTH_MAX, fmt, TETHERING_WIFI_MAX_CONNECTED_STA);
		_wifi_tethering_checkbox_popup_status_set(true);
		_prepare_popup(MH_POPUP_WIFI_ON_CHECKBOX, str);
		g_free(str);
		_create_popup(ad);
	} else {
		_prepare_popup_type(MH_POPUP_WIFI_ON_CHECKBOX);
		_teth_on(ad);
	}
	return 0;
}

int _create_bt_tethering_on_popup(mh_appdata_t *ad)
{
	char *str;
	int value = 0;
	value = _get_checkbox_status(TETHERING_TYPE_BT);
	if (0 == value) {
		str = g_malloc0(MH_LABEL_LENGTH_MAX);
		if (str == NULL) {
			ERR("memory allocation is failed\n");
			return -1;
		}
		snprintf(str, MH_LABEL_LENGTH_MAX, STR_TETH_ON_DESC_2,
				TETHERING_BT_MAX_CONNECTED_STA);
		_prepare_popup(MH_POPUP_BT_ON_CHECKBOX, str);
		g_free(str);
		_create_popup(ad);
	} else {
		_prepare_popup_type(MH_POPUP_BT_ON_CHECKBOX);
		_teth_on(ad);
	}
	return 0;
}

int _create_usb_tethering_on_popup(mh_appdata_t *ad)
{
	int value = 0;
	char *str = NULL;
	value = _get_checkbox_status(TETHERING_TYPE_USB);
	DBG("%s : %d\n", VCONF_MOBILE_AP_USB_POPUP_CHECKBOX_STATUS, value);
	if (0 == value) {
		str = g_malloc0(MH_LABEL_LENGTH_MAX);
		if (str == NULL) {
			ERR("memory allocation is failed\n");
			return -1;
		}
		snprintf(str, MH_LABEL_LENGTH_MAX, "%s",
				STR_TETH_ON_DESC_3);
		_prepare_popup(MH_POPUP_USB_ON_CHECKBOX, str);
		g_free(str);
		_create_popup(ad);
	} else {
		_prepare_popup_type(MH_POPUP_USB_ON_CHECKBOX);
		_teth_on(ad);
	}
	return 0;
}

static void __disable_tethering_by_ind(mh_appdata_t *ad, tethering_disabled_cause_e cause)
{
	if (ad == NULL) {
		ERR("Param is NULL\n");
		return;
	}

	DBG("cause : %d\n", cause);
	switch (cause) {
	case TETHERING_DISABLED_BY_WIFI_ON:
		break;

	case TETHERING_DISABLED_BY_BT_OFF:
		break;

	case TETHERING_DISABLED_BY_USB_DISCONNECTION:
		if (ad->is_bt_teth_enabling && ad->main.bt_item) {
			_update_bt_item(ad, MH_STATE_NONE);
			ad->is_bt_teth_enabling = false;
		}
		if (ad->is_wifi_teth_enabling && ad->main.wifi_item) {
			_update_wifi_item(ad, MH_STATE_NONE);
			ad->is_wifi_teth_enabling = false;
		}
		break;

	case TETHERING_DISABLED_BY_FLIGHT_MODE:
		break;

	case TETHERING_DISABLED_BY_TIMEOUT:
		break;

	case TETHERING_DISABLED_BY_OTHERS:
		if (ad->main.wifi_item && _get_vconf_prev_wifi_state() == true) {
			elm_object_item_disabled_set(ad->main.wifi_item, EINA_TRUE);
		}
		break;

	case TETHERING_DISABLED_BY_LOW_BATTERY:
		break;

	default:
		break;
	}

	return;
}

/* Wi-Fi callbacks */
static void __wifi_activated_cb(wifi_error_e result, void *user_data)
{
	DBG("+\n");

	return;
}

/* Tethering callbacks */
void _enabled_cb(tethering_error_e result, tethering_type_e type, bool is_requested, void *user_data)
{
	DBG("+\n");
	DBG("result : %d, type : %d, is_requested : %d\n", result, type, is_requested);

	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	if (!is_requested) {
		if (ad->type == type && ad->popup) {
			DBG("This tethering type is already enabled\n");
			evas_object_del(ad->popup);
			ad->popup = NULL;
		}
		_update_main_view(ad, type);
		if (type == TETHERING_TYPE_RESERVED) {
			DBG("MobileAP is turned on\n");
			_prepare_popup(MH_POPUP_WIFI_AP_OFF, STR_WIFI_AP_CONTROLLED_ANOTHER_APP);
			_create_popup(ad);
		}
		return;
	}

	if (result != TETHERING_ERROR_NONE) {
	}
	_update_main_view(ad, type);

	return;
}

void _disabled_cb(tethering_error_e result, tethering_type_e type, tethering_disabled_cause_e cause, void *user_data)
{
	DBG("+\n");
	DBG("result : %d, type : %d, cause : %d\n", result, type, cause);

	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	if (cause != TETHERING_DISABLED_BY_REQUEST) {
		if (ad->type == type && ad->popup) {
			DBG("This tethering type is already disabled\n");
			evas_object_del(ad->popup);
			ad->popup = NULL;
		}
		__disable_tethering_by_ind(ad, cause);
		_update_main_view(ad, type);
		return;
	}

	if (result != TETHERING_ERROR_NONE) {
		_prepare_popup(MH_POPUP_TETH_ENABLING_FAILED, STR_UNABLE_TO_USE_TETH);
		_create_popup(ad);
		_update_main_view(ad, type);
		return;
	}

	if (ad->main.wifi_item && type == TETHERING_TYPE_WIFI && _get_vconf_prev_wifi_state() == true) {
		elm_object_item_disabled_set(ad->main.wifi_item, EINA_TRUE);
	}

	_update_main_view(ad, type);

	return;
}

void _connection_changed_cb(tethering_client_h client, bool is_opened, void *user_data)
{
	DBG("+\n");

	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;
	char *mac_addr = NULL;

	if (is_opened) {
		_append_list_client_handle(ad, client);

#ifdef TETHERING_DATA_USAGE_SUPPORT
		if (ad->is_foreground && _get_list_clients_count(ad) == 1) {
			_start_update_data_packet_usage(ad);
		}
#endif
	} else {
		tethering_client_get_mac_address(client, &mac_addr);
		if (mac_addr) {
			_delete_list_client_handle(ad, mac_addr);
			free(mac_addr);
		}
#ifdef TETHERING_DATA_USAGE_SUPPORT
		if (ad->is_foreground && _get_list_clients_count(ad) == 0)
			_stop_update_data_packet_usage(ad);
#endif
	}

	ap_update_data_device(ad);

	return;
}

void _wifi_state_changed_cb(wifi_device_state_e state, void *user_data)
{
	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	DBG("+\n");

	mh_appdata_t *ad= (mh_appdata_t *)user_data;
	char *str = NULL;
	char *fmt = NULL;
	if (state == WIFI_DEVICE_STATE_ACTIVATED) {
		if (ad->main.wifi_item && elm_object_item_disabled_get(ad->main.wifi_item))
			elm_object_item_disabled_set(ad->main.wifi_item, EINA_FALSE);
		_set_vconf_prev_wifi_state(false);
	} else if (state == WIFI_CONNECTION_STATE_DISCONNECTED){
		_set_vconf_prev_wifi_state(true);
	}

	if (ad->type == TETHERING_TYPE_WIFI && ad->popup && ad->popup_checkbox &&
			_wifi_tethering_checkbox_popup_status_get()) {
		evas_object_del(ad->popup_checkbox);
		ad->popup_checkbox = NULL;
		evas_object_del(ad->popup);
		ad->popup = NULL;
		if (state == WIFI_DEVICE_STATE_ACTIVATED) {
			fmt = STR_TETH_ON_DESC_1;
		} else {
			fmt = STR_TETH_ON_DESC_2;
		}
		str = g_malloc0(MH_LABEL_LENGTH_MAX);
		if (str == NULL) {
			ERR("memory allocation is failed\n");
			return;
		}
		snprintf(str, MH_LABEL_LENGTH_MAX, fmt, TETHERING_WIFI_MAX_CONNECTED_STA);
		_prepare_popup(MH_POPUP_WIFI_ON_CHECKBOX, str);
		g_free(str);
		_create_popup(ad);
	}
	DBG("-\n");
}

void _visibility_changed_cb(bool is_visible, void *user_data)
{
	DBG("+\n");

	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	if (ad->setup.visibility == is_visible)
		return;

	ad->setup.visibility = is_visible;
	ad->setup.visibility_new = is_visible;
	if (ad->setup.hide_item)
		elm_genlist_item_update(ad->setup.hide_item);

	DBG("-\n");

	return;
}

void _security_type_changed_cb(tethering_wifi_security_type_e changed_type, void *user_data)
{
	DBG("+\n");

	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	if (ad->setup.security_type == changed_type)
		return;

	ad->setup.security_type = changed_type;
	ad->setup.security_type_new = changed_type;

	if (ad->setup.security_item)
		elm_genlist_item_update(ad->setup.security_item);

	DBG("-\n");

	return;
}

void _passphrase_changed_cb(void *user_data)
{
	DBG("+\n");

	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;
	char *passphrase = NULL;
	int ret;

	ret = tethering_wifi_get_passphrase(ad->handle, &passphrase);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("tethering_wifi_get_passphrase failed ret = [0x%x]\n", ret);
		return;
	}

	if (!g_strcmp0(passphrase, ad->setup.wifi_passphrase)) {
		goto DONE;
	}

	g_strlcpy(ad->setup.wifi_passphrase, passphrase,
		sizeof(ad->setup.wifi_passphrase));

	g_strlcpy(ad->setup.wifi_passphrase_new, passphrase,
		sizeof(ad->setup.wifi_passphrase_new));

	if (ad->setup.pw_item)
		elm_genlist_item_update(ad->setup.pw_item);

DONE:
	g_free(passphrase);
	DBG("-\n");
	return;
}

#ifdef TETHERING_DATA_USAGE_SUPPORT
void _data_usage_cb(tethering_error_e result, unsigned long long received_data, unsigned long long sent_data, void *user_data)
{
	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	if (ad->data_statistics.pdp_total_sent != sent_data ||
			ad->data_statistics.pdp_total_receive != received_data) {
		ad->data_statistics.pdp_total_sent = sent_data;
		ad->data_statistics.pdp_total_receive = received_data;
		ap_update_data_packet_usage(ad);
	}

	ad->data_statistics.is_updated = true;

	return;
}
#endif
/* End of Tethering callbacks */

int _handle_wifi_onoff_change(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	int ret;
	int connected_wifi_clients = 0;

	/* Turn off WiFi hotspot */
	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI) {
		if (_get_no_of_connected_device(ad, &connected_wifi_clients,
					TETHERING_TYPE_WIFI) == EINA_FALSE) {
			ERR("Getting the number of connected device is failed\n");
		}
		if (connected_wifi_clients > 0) {
			_prepare_popup(MH_POPUP_WIFI_OFF, STR_CLOSE_INTERNET_Q);
			_create_popup(ad);
		} else {
			ret = tethering_disable(ad->handle, TETHERING_TYPE_WIFI);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("wifi tethering off is failed : %d\n", ret);
				return -1;
			}
			_send_signal_qp(QP_SIGNAL_PROGRESS_OFF);
		}
		return 0;
	}

	/* Turn on WiFi hotspot */
	ret = __is_preconditions_handled(ad);
	if (ret < 0)
		return -1;
	else if (ret == 0)
		return 0;

	if (_create_wifi_hotspot_on_popup(ad) < 0) {
		ERR("__create_wifi_hotspot_on_popup fail\n");
		return -1;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return 0;
}

int _handle_bt_onoff_change(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	int ret;

	/* Turn off Bluetooth tethering */
	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_BT) {
		ret = tethering_disable(ad->handle, TETHERING_TYPE_BT);
		if (ret) {
			ERR("Error disable bt tethering [%d]\n", ret);
			return -1;
		}
		return 0;
	}

	/* Turn on Bluetooth tethering */
	ret = __is_preconditions_handled(ad);
	if (ret < 0)
		return -1;
	else if (ret == 0)
		return 0;

	if (_create_bt_tethering_on_popup(ad) < 0) {
		ERR("_create_bt_tethering_on_popup fail\n");
		return -1;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return 0;
}

int _handle_usb_onoff_change(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	int ret;

	/* Turn off USB tethering */
	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_USB) {
		ret = tethering_disable(ad->handle, TETHERING_TYPE_USB);
		if (ret) {
			ERR("Error disable usb tethering : %d\n", ret);
			return -1;
		}
		return 0;
	}

	/* Turn on USB tethering */
	ret = __is_preconditions_handled(ad);
	if (ret < 0)
		return -1;
	else if (ret == 0)
		return 0;

	if (_create_usb_tethering_on_popup(ad) < 0) {
		ERR("_create_usb_tethering_on_popup fail\n");
		return -1;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return 0;
}

int _turn_on_wifi(void)
{
	int ret;

	ret = wifi_activate(__wifi_activated_cb, NULL);
	if (ret != WIFI_ERROR_NONE) {
		ERR("wifi_activate() is failed : %d\n", ret);
		return -1;
	}

	return 0;
}

bool _is_wifi_direct_on(void)
{
	int wifi_direct_state = 0;
	int ret;

	ret = vconf_get_int(VCONFKEY_WIFI_DIRECT_STATE, &wifi_direct_state);
	if (ret < 0) {
		ERR("vconf_get_int() is failed : %d\n", ret);
		return false;
	}

	return wifi_direct_state != 0 ? true : false;
}
