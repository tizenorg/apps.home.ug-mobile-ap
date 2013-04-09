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
#include <wifi-direct.h>

#include "mh_func_onoff.h"

static bool __get_vconf_prev_wifi_state()
{
	int value = 0;

	if (vconf_get_bool(VCONF_MOBILE_AP_PREV_WIFI_STATUS, &value) < 0) {
		ERR("vconf_get_bool is failed\n");
		return false;
	}

	DBG("%s : %d\n", VCONF_MOBILE_AP_PREV_WIFI_STATUS, value);

	return value ? true : false;
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

static bool __is_connected_cellular_net(mh_appdata_t *ad)
{
	connection_cellular_state_e cellular_state = CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	sim_state_e sim_state = SIM_STATE_UNAVAILABLE;
	int ret;

	/* Check SIM state */
	ret = sim_get_state(&sim_state);
	if (ret != SIM_ERROR_NONE) {
		ERR("sim_get_state() is failed : %d\n", ret);
		_prepare_popup(MH_POP_INFORMATION,
				_("IDS_MOBILEAP_POP_INSERT_SIM_CARD_AND_RESTART_DEVICE_TO_USE_TETHERING"));
		_create_popup(ad);
		return false;
	}
	DBG("SIM State : %d\n", sim_state);
	if (sim_state != SIM_STATE_AVAILABLE) {
		_prepare_popup(MH_POP_INFORMATION,
				_("IDS_MOBILEAP_POP_INSERT_SIM_CARD_AND_RESTART_DEVICE_TO_USE_TETHERING"));
		_create_popup(ad);
		return false;
	}

	ret = connection_get_cellular_state(ad->conn_handle, &cellular_state);
	if (ret != CONNECTION_ERROR_NONE) {
		ERR("connection_get_cellular_state() is failed : %d\n");
		return false;
	}

	if (cellular_state == CONNECTION_CELLULAR_STATE_FLIGHT_MODE) {
		_prepare_popup(MH_POP_INFORMATION_WO_BUTTON,
				_("IDS_MOBILEAP_POP_UNABLE_TO_USE_TETHERING_IN_FLIGHT_MODE_TO_USE_TETHERING_DISABLE_FLIGHT_MODE"));
		_create_popup(ad);
		ERR("Cellular network is not connected\n");
		return false;
	} else if (cellular_state != CONNECTION_CELLULAR_STATE_CONNECTED &&
			cellular_state != CONNECTION_CELLULAR_STATE_AVAILABLE) {
		_prepare_popup(MH_POP_INFORMATION,
				_("IDS_MOBILEAP_POP_UNABLE_TO_USE_PACKET_DATA_SERVICE_OUT_OF_COVERAGE"));
		_create_popup(ad);
		ERR("Cellular network is not connected : %d\n", cellular_state);
		return false;
	}

	DBG("Cellular network is connected\n");
	return true;
}

static int __create_wifi_hotspot_on_popup(mh_appdata_t *ad)
{
	char *fmt;
	char *str;
	bool wifi_state = false;

	wifi_is_activated(&wifi_state);
	if (wifi_state == true || _is_wifi_direct_on() == true)
		fmt = _("IDS_ST_BODY_WI_FI_NETWORK_WILL_BE_DISCONNECTED_TETHERING_CONSUMES_MORE_BATTERY_POWER_AND_INCREASES_YOUR_DATA_USAGE_THE_MAXIMUM_NUMBER_OF_TETHERED_DEVICES_ALLOWED_IS_PD");
	else
		fmt = _("IDS_ST_BODY_TETHERING_CONSUMES_MORE_BATTERY_POWER_AND_INCREASES_YOUR_DATA_USAGE_THE_MAXIMUM_NUMBER_OF_TETHERED_DEVICES_ALLOWED_IS_PD");

	str = malloc(MH_LABEL_LENGTH_MAX);
	snprintf(str, MH_LABEL_LENGTH_MAX, fmt, TETHERING_WIFI_MAX_CONNECTED_STA);
	_prepare_popup(MH_POP_WIFI_ON_CONF, str);
	free(str);

	_create_popup(ad);

	return 0;
}

static int __create_bt_tethering_on_popup(mh_appdata_t *ad)
{
	char *str;

	str = malloc(MH_LABEL_LENGTH_MAX);
	snprintf(str, MH_LABEL_LENGTH_MAX,
			_("IDS_ST_BODY_TETHERING_CONSUMES_MORE_BATTERY_POWER_AND_INCREASES_YOUR_DATA_USAGE_THE_MAXIMUM_NUMBER_OF_TETHERED_DEVICES_ALLOWED_IS_PD"),
			TETHERING_BT_MAX_CONNECTED_STA);
	_prepare_popup(MH_POP_BT_ON_CONF, str);
	free(str);

	_create_popup(ad);

	return 0;
}

static int __create_usb_tethering_on_popup(mh_appdata_t *ad)
{
	char *str;

	str = _("IDS_MOBILEAP_POP_TETHERING_CONSUMES_MORE_BATTERY_POWER_AND_INCREASES_YOUR_DATA_USAGE");
	_prepare_popup(MH_POP_USB_ON_CONF, str);
	_create_popup(ad);

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
		DBG("TETHERING_DISABLED_IND by WIFI\n");
		break;

	case TETHERING_DISABLED_BY_BT_OFF:
		DBG("TETHERING_DISABLED_BY_BT_DEACTIVATION\n");
		break;

	case TETHERING_DISABLED_BY_USB_DISCONNECTION:
		DBG("TETHERING_DISABLED_IND by USB DISCONNECT\n");
		break;

	case TETHERING_DISABLED_BY_FLIGHT_MODE:
		DBG("TETHERING_DISABLED_IND by FLIGHT_MODE\n");
		break;

	case TETHERING_DISABLED_BY_TIMEOUT:
		DBG("TETHERING_DISABLED_BY_TIMEOUT\n");
		break;

	case TETHERING_DISABLED_BY_OTHERS:
		DBG("TETHERING_DISABLED_IND by OTHERS\n");
		break;

	case TETHERING_DISABLED_BY_LOW_BATTERY:
		DBG("TETHERING_DISABLED_IND by LOW_BATTERY\n");
		break;

	case TETHERING_DISABLED_BY_MDM_ON:
		DBG("TETHERING_DISABLED_IND by MDM\n");
		break;

	default:
		DBG("TETHERING_DISABLED_IND Default\n");
		break;
	}

	return;
}

static void __recover_wifi_station_mode(void)
{
	DBG("+\n");

	if (__get_vconf_prev_wifi_state() == false) {
		DBG("No need to recover wifi station mode\n");
		return;
	}

	if (_turn_on_wifi() != 0)
		ERR("_turn_on_wifi is failed\n");
	if (vconf_set_bool(VCONF_MOBILE_AP_PREV_WIFI_STATUS, 0) < 0)
		ERR("vconf_set_bool failed\n");

	return;
}

/* Wi-Fi Direct callback */
static void _wifi_direct_state_cb(int error_code, wifi_direct_device_state_e state, void *user_data)
{
	DBG("+\n");

	if (user_data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;
	int ret = 0;

	wifi_direct_unset_device_state_changed_cb();
	wifi_direct_deinitialize();
	DBG("-\n");

	if (error_code != 0) {
		ERR("wifi_direct_deactivate fail in cb : %d\n", error_code);
		_update_main_view(ad);
		return;
	}

	if (state != WIFI_DIRECT_DEVICE_STATE_DEACTIVATED) {
		ERR("Unknown state : %d\n", state);
		return;
	}

	ret = tethering_enable(ad->handle, TETHERING_TYPE_WIFI);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("wifi tethering on is failed : %d\n", ret);
		_update_main_view(ad);
		return;
	}

	DBG("-\n");
	return;
}

/* Wi-Fi callbacks */
static void __wifi_activated_cb(wifi_error_e result, void *user_data)
{
	__MOBILE_AP_FUNC_ENTER__;

	DBG("Wi-Fi on is done\n");

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static void __wifi_deactivated_cb(wifi_error_e result, void *user_data)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (user_data == NULL) {
		ERR("The param is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;
	int ret;

	if (result != WIFI_ERROR_NONE) {
		ERR("__wifi_deactivated_cb error : %d\n", result);
		_update_main_view(ad);
		return;
	}

	DBG("Wi-Fi is turned off\n");

	ret = vconf_set_bool(VCONF_MOBILE_AP_PREV_WIFI_STATUS, 1);
	if (ret < 0)
		ERR("vconf_set_bool() is failed : %d\n", ret);

	ret = tethering_enable(ad->handle, TETHERING_TYPE_WIFI);
	if (ret != TETHERING_ERROR_NONE) {
		ERR("wifi tethering on is failed : %d\n", ret);
		_update_main_view(ad);
		return;
	}

	__MOBILE_AP_FUNC_EXIT__;
	return;
}

/* Tethering callbacks */
void _enabled_cb(tethering_error_e result, tethering_type_e type, bool is_requested, void *user_data)
{
	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	ad->main.need_recover_wifi_tethering = false;

	if (!is_requested) {
		if (NULL != ad->popup) {
			evas_object_del(ad->popup);
			ad->popup = NULL;
		}
		_update_main_view(ad);

		return;
	}

	if (result != TETHERING_ERROR_NONE) {
		_prepare_popup(MH_POP_INFORMATION,
				_("IDS_MOBILEAP_POP_UNABLE_TO_USE_TETHERING"));
		_create_popup(ad);
	}

	_update_main_view(ad);

	return;
}

void _disabled_cb(tethering_error_e result, tethering_type_e type, tethering_disabled_cause_e cause, void *user_data)
{
	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;

	if (ad->main.need_recover_wifi_tethering == true) {
		DBG("Wi-Fi tethering will be turned on again\n");
		return;
	}

	if (cause != TETHERING_DISABLED_BY_REQUEST) {
		DBG("Tethering [%d] is disabled because of [%d]\n", type, cause);
		if (NULL != ad->popup) {
			evas_object_del(ad->popup);
			ad->popup = NULL;
		}
		_update_main_view(ad);
		__disable_tethering_by_ind(ad, cause);
		return;
	}

	if (result != TETHERING_ERROR_NONE) {
		_prepare_popup(MH_POP_INFORMATION,
				_("IDS_MOBILEAP_POP_UNABLE_TO_USE_TETHERING"));
		_create_popup(ad);
		_update_main_view(ad);
		return;
	}

	DBG("Tethering [%d] is disabled by reqeust\n", type);
	if (type == TETHERING_TYPE_WIFI) {
		__recover_wifi_station_mode();
	}

	_update_main_view(ad);

	return;
}

void _connection_changed_cb(tethering_client_h client, bool is_opened, void *user_data)
{
	if (user_data == NULL) {
		ERR("user_data is NULL\n");
		return;
	}

	mh_appdata_t *ad = (mh_appdata_t *)user_data;
	char *name = NULL;

	tethering_client_get_name(client, &name);
	DBG("Client %s is %s\n", name, is_opened ?  "connected" : "disconnected");
	if (name)
		free(name);

	ap_update_data_device(ad);

	return;
}

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
/* End of Tethering callbacks */

int _handle_wifi_onoff_change(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	int ret;
	int connected_wifi_clients = 0;

	/* Turn off WiFi hotspot */
	if (ad->main.hotspot_mode & VCONFKEY_MOBILE_HOTSPOT_MODE_WIFI) {
		if (_get_no_of_connected_device(ad->handle, &connected_wifi_clients,
					TETHERING_TYPE_WIFI) == FALSE) {
			ERR("Getting the number of connected device is failed\n");
		}
		if (connected_wifi_clients > 0) {
			_prepare_popup(MH_POP_WIFI_OFF_CONF,
					_("IDS_MOBILEAP_POP_DISABLING_TETHERING_WILL_PREVENT_LINKED_DEVICES_FROM_ACCESSING_THE_INTERNET_CONTINUE_Q"));
			_create_popup(ad);
		} else {
			ret = tethering_disable(ad->handle, TETHERING_TYPE_WIFI);
			if (ret != TETHERING_ERROR_NONE) {
				ERR("wifi tethering off is failed : %d\n", ret);
				return -1;
			}
		}
		return 0;
	}

	/* Turn on WiFi hotspot */
	if (!__is_connected_ethernet_net(ad) && !__is_connected_cellular_net(ad)) {
		ERR("There is no connected network\n");
		return -1;
	}

	if (__create_wifi_hotspot_on_popup(ad) < 0) {
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
	if (!__is_connected_ethernet_net(ad) && !__is_connected_wifi_net(ad) &&
			!__is_connected_cellular_net(ad)) {
		ERR("There is no connected network\n");
		return -1;
	}

	if (__create_bt_tethering_on_popup(ad) < 0) {
		ERR("__create_bt_tethering_on_popup fail\n");
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
			DBG("Error disable usb tethering : %d\n", ret);
			return -1;
		}
		return 0;
	}

	/* Turn on USB tethering */
	if (!__is_connected_ethernet_net(ad) && !__is_connected_wifi_net(ad) &&
			!__is_connected_cellular_net(ad)) {
		ERR("There is no connected network\n");
		return -1;
	}

	if (__create_usb_tethering_on_popup(ad) < 0) {
		ERR("__create_usb_tethering_on_popup fail\n");
		return -1;
	}

	__MOBILE_AP_FUNC_EXIT__;

	return 0;
}

int _turn_off_wifi(mh_appdata_t *ad)
{
	int ret;

	ret = wifi_deactivate(__wifi_deactivated_cb, (void *)ad);
	if (ret != WIFI_ERROR_NONE) {
		ERR("wifi_deactivate() is failed : %d\n", ret);
		return -1;
	}

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

int _turn_off_wifi_direct(mh_appdata_t *ad)
{
	int ret;

	ret = wifi_direct_initialize();
	if (ret < 0) {
		ERR("wifi_direct_initialize() is failed : %d\n", ret);
		return -1;
	}

	ret = wifi_direct_set_device_state_changed_cb(_wifi_direct_state_cb, (void *)ad);
	if (ret < 0) {
		ERR("wifi_direct_set_device_state_changed_cb() is failed : %d\n", ret);
		ret = wifi_direct_deinitialize();
		DBG("wifi_direct_deinitialize() ret : %d\n", ret);
		return -1;
	}

	ret = wifi_direct_deactivate();
	if (ret < 0) {
		ERR("wifi_direct_deactivate() is failed : %d\n", ret);
		ret = wifi_direct_unset_device_state_changed_cb();
		DBG("wifi_direct_unset_device_state_changed_cb() ret : %d\n", ret);
		ret = wifi_direct_deinitialize();
		DBG("wifi_direct_deinitialize() ret : %d\n", ret);
		return -1;
	}

	return 0;
}
