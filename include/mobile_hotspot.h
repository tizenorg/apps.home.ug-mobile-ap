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

#ifndef __MOBILE_HOTSPOT_H__
#define __MOBILE_HOTSPOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(PACKAGE)
#define PACKAGE "ug-setting-mobileap-efl"
#endif

#if !defined(LOCALEDIR)
#define	LOCALEDIR "/usr/apps/ug-setting-mobileap-efl/res/locale"
#endif

#if !defined(EDJDIR)
#define EDJDIR "/usr/apps/ug-setting-mobileap-efl/res/edje/ug-setting-mobileap-efl"
#endif

#if !defined(PREFIX)
#define PREFIX "/usr/ug"
#endif

#define TETHERING_EDJ "tethering.edj"
#define POPUP_EDJ_PATH EDJDIR"/"TETHERING_EDJ

#include <sys/time.h>
#include <glib.h>
#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi.h>
#include <net_connection.h>
#include <tethering.h>
#include <dlog.h>
#include <notification.h>

#define MH_TEXT_DOMAIN				PACKAGE
#define MH_LOCALEDIR				LOCALEDIR


#define VCONF_MOBILE_AP_PREV_WIFI_STATUS	"db/private/libug-setting-mobileap-efl/prev_wifi_status"
#define VCONF_MOBILE_AP_WIFI_POPUP_CHECKBOX_STATUS "file/private/libug-setting-mobileap-efl/wifi_popup_check_box_status"
#define VCONF_MOBILE_AP_BT_POPUP_CHECKBOX_STATUS "file/private/libug-setting-mobileap-efl/bt_popup_check_box_status"
#define VCONF_MOBILE_AP_USB_POPUP_CHECKBOX_STATUS "file/private/libug-setting-mobileap-efl/usb_popup_check_box_status"
#define VCONF_MOBILE_AP_CONNECT_USB_POPUP_STATUS "memory/private/libug-setting-mobileap-efl/trying_usb_tethering"
#define COLOR_TABLE "/usr/apps/ug-setting-mobileap-efl/shared/res/tables/ug-setting-mobileap-efl_ChangeableColorInfo.xml"
#define FONT_TABLE "/usr/apps/ug-setting-mobileap-efl/shared/res/tables/ug-setting-mobileap-efl_fontInfoTable.xml"

#define VCONF_KEY_MOBILEAP_SYSPOPUP_RESPONSE "memory/private/libug-setting-mobileap-efl/mobileap_syspopup_user_response"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG				"UG_SETTING_MOBILEAP_EFL"

#define DBG(fmt, args...)		LOGD(fmt, ##args)
#define INFO(fmt, args...)		LOGI(fmt, ##args)
#define ERR(fmt, args...)		LOGE(fmt, ##args)
#define SDBG(fmt, args...)		SECURE_LOGD(fmt, ##args)
#define SERR(fmt, args...)		SECURE_LOGE(fmt, ##args)

#define __MOBILE_AP_FUNC_ENTER__	DBG("Entering: +\n")
#define __MOBILE_AP_FUNC_EXIT__		DBG("Exit: -\n")

#define TETHERING_IMAGES_EDJ		"tethering_images.edj"
#define WIFI_ICON			"A01-5_device_WIFI.png"
#define BT_ICON				"A01-5_device_bluetooth.png"
#define USB_ICON			"A01-5_device_USB_icon.png"

#define DEVICE_NAME_LENGTH_MAX		32
#define WIFI_PASSPHRASE_LENGTH_MIN	8
#define WIFI_PASSPHRASE_LENGTH_MAX	63
#define MH_LABEL_LENGTH_MAX		1024

#define _EDJ(obj)			elm_layout_edje_get(obj)

#define TETHERING_WIFI_MAX_CONNECTED_STA	10
#define TETHERING_BT_MAX_CONNECTED_STA		4
#define MAX_CONNECTED_STA 15

/* This is from tethering_private.h */
#define TETHERING_TYPE_MAX		4	/**< All, USB, Wi-Fi, BT */

typedef enum {
	MH_POPUP_NONE,
	MH_POPUP_WIFI_ON_CHECKBOX,
	MH_POPUP_BT_ON_CHECKBOX,
	MH_POPUP_USB_ON_CHECKBOX,
	MH_POPUP_WIFI_AP_OFF,
	MH_POPUP_NO_SIM,
	MH_POPUP_FLIGHT_MODE,
	MH_POPUP_MOBILE_DATA_OFF,
	MH_POPUP_CANCEL_MOBILE_DATA_ON,
	MH_POPUP_NETWORK_OUT_OF_RANGE,
	MH_POPUP_WIFI_OFF,
	MH_POPUP_TETH_ENABLING_FAILED,
	MH_POPUP_SPECIAL_LETTER_WARN,
	MH_POPUP_MAX
} mh_popup_type_e;

typedef enum {
	MH_STATE_NONE,
	MH_STATE_PROCESS,
	MH_STATE_MAX
} mh_state_e;

typedef struct ap_app_main {
	Evas_Object		*genlist;
	Evas_Object		*back_btn;
	Evas_Object		*check_popup_ly;

#if 0 /* not used */
	Elm_Genlist_Item_Class	*sp_itc;
	Elm_Genlist_Item_Class	*sp2_itc;
#endif
	Elm_Genlist_Item_Class	*wifi_itc;
	Elm_Genlist_Item_Class	*setup_itc;
	Elm_Genlist_Item_Class	*bt_itc;
	Elm_Genlist_Item_Class	*usb_itc;
	Elm_Genlist_Item_Class	*help_itc;
	Elm_Genlist_Item_Class	*device_itc;
	Elm_Genlist_Item_Class	*device0_itc;
	Elm_Genlist_Item_Class	*popup_descp_itc;
	Elm_Genlist_Item_Class	*popup_check_itc;

	Elm_Object_Item		*sp_item[4];
	Elm_Object_Item		*help_sp_item;
	Elm_Object_Item		*wifi_item;
	Elm_Object_Item		*setup_item;
	Elm_Object_Item		*bt_item;
	Elm_Object_Item		*usb_item;
	Elm_Object_Item		*device_item;
	Elm_Object_Item		*help_item;
	Elm_Object_Item		*popup_descp_item;
	Elm_Object_Item		*popup_check_item;

#ifdef TETHERING_DATA_USAGE_SUPPORT
	Elm_Genlist_Item_Class	*usage_itc;
	Elm_Object_Item		*usage_item;
#endif

	int		hotspot_mode;
	int		wifi_state;
	int		bt_state;
	int		usb_state;
} mh_main_view_t;

typedef struct {
	Elm_Object_Item		*navi_it;
	Evas_Object			*genlist;
	Evas_Object			*rename_genlist;
	Evas_Object		*hide_btn;
	Evas_Object		*security_btn;
	Evas_Object		*pw_entry;
	Evas_Object		*popup_passphrase;
	Evas_Object		*save_button;
	Evas_Object		*cancel_button;

#if 0 /* not used */
	Elm_Genlist_Item_Class		*sp_itc;
	Elm_Genlist_Item_Class		*sp2_itc;
#endif
	Elm_Genlist_Item_Class		*hide_itc;
	Elm_Genlist_Item_Class		*security_itc;
	Elm_Genlist_Item_Class		*pw_itc;
	Elm_Genlist_Item_Class		*name_itc;
	Elm_Genlist_Item_Class		*rename_entry_itc;
	Elm_Genlist_Item_Class		*rename_descp_itc;

	Elm_Object_Item		*sp_item[2];
	Elm_Object_Item		*hide_item;
	Elm_Object_Item		*security_item;
	Elm_Object_Item		*pw_item;
	Elm_Object_Item		*name_item;
	Elm_Object_Item		*pw_guide_item;
	Elm_Object_Item		*rename_entry_item;
	Elm_Object_Item		*rename_descp_item;

	bool	visibility;
	bool	visibility_new;
	tethering_wifi_security_type_e	security_type;
	tethering_wifi_security_type_e	security_type_new;

	char	device_name[DEVICE_NAME_LENGTH_MAX + 1];
	char	wifi_passphrase[WIFI_PASSPHRASE_LENGTH_MAX + 1];
	char	wifi_passphrase_new[WIFI_PASSPHRASE_LENGTH_MAX + 1];
} mh_wifi_setting_view_t;

typedef struct {
	bool	is_updated;
	unsigned long long	pdp_total_sent;
	unsigned long long	pdp_total_receive;
} mh_data_usage_t;

typedef struct {
	Elm_Object_Item *navi_it;
	Evas_Object *genlist;

	Elm_Genlist_Item_Class *dev_itc[TETHERING_TYPE_MAX];
	Elm_Object_Item *dev_item;
	Elm_Object_Item		*station_items[MAX_CONNECTED_STA];
	int no_of_clients;
} mh_conn_client_view_t;

typedef struct {
	void	*gadget;
	GSList	*client_list;
	bool	is_foreground;

	tethering_h		handle;
	connection_h	conn_handle;
	tethering_type_e	type;

	Evas_Object		*win;
	Evas_Object		*layout;
	Evas_Object		*bg;
	Evas_Object		*naviframe;
	Evas_Object		*popup_checkbox;
	Evas_Object		*popup;
	Evas_Object		*rename_entry;
	Ecore_Timer		*update_statistics_handle;
	Ecore_Timer		*update_conn_time_handle;
	Evas_Object		*ctxpopup;
	Evas_Object		*rename_popup;
	Evas_Object		*rename_button;
	Elm_Object_Item		*navi_item;

	int		ps_recheck_timer_id;
	int		is_wifi_teth_enabling;
	int		is_usb_teth_enabling;
	int		is_bt_teth_enabling;

	mh_main_view_t	main;
	mh_wifi_setting_view_t	setup;
	mh_data_usage_t		data_statistics;
	mh_conn_client_view_t connected_device;
} mh_appdata_t;

typedef struct {
	mh_appdata_t	*ad;
	ui_gadget_h		ug;
} mh_ugdata_t;

#ifdef __cplusplus
}
#endif
#endif /* __MOBILE_HOTSPOT_H__ */
