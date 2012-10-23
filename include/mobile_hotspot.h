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
#define PACKAGE "ug-setting-mobile-ap-ug"
#endif

#if !defined(LOCALEDIR)
#define	LOCALEDIR "/usr/ug/res/locale"
#endif

#if !defined(EDJDIR)
#define EDJDIR "/usr/ug/res/edje/ug-mobile-ap"
#endif

#if !defined(PREFIX)
#define PREFIX "/usr/ug"
#endif

#include <sys/time.h>
#include <glib.h>
#include <Elementary.h>
#include <vconf.h>
#include <ui-gadget-module.h>
#include <wifi.h>
#include <net_connection.h>
#include <sim.h>
#include <tethering.h>
#include <dlog.h>

#define MH_TEXT_DOMAIN				PACKAGE
#define MH_LOCALEDIR				LOCALEDIR

#ifdef S_
#undef S_
#endif
#define S_(str)					dgettext("sys_string", str)

#ifdef N_
#undef N_
#endif
#define N_(str)					gettext_noop(str)

#ifdef _
#undef _
#endif
#define _(str)					dgettext(MH_TEXT_DOMAIN, str)

#define VCONF_MOBILE_AP_PREV_WIFI_STATUS	"db/private/libug-setting-mobileap-efl/prev_wifi_status"

#define MOBILE_AP_TAG					"mobile_ap"
#define DBG(fmt, args...) LOG(LOG_DEBUG, MOBILE_AP_TAG, "[%s()][Ln:%d] "fmt, \
						__func__, __LINE__, ##args)
#define ERR(fmt, args...) LOG(LOG_ERROR, MOBILE_AP_TAG, "[%s()][Ln:%d] "fmt, \
						__func__, __LINE__, ##args)

#define __MOBILE_AP_FUNC_ENTER__	DBG("Entering: +\n")
#define __MOBILE_AP_FUNC_EXIT__ 	DBG("Exit: -\n")

#define MOBILE_AP_IMG_DIR		PREFIX"/res/images/ug-mobile-ap"

#define MH_DEFAULT_PASSWORD		"qwertyuiop"

#define DEVICE_NAME_LENGTH_MAX		31
#define WIFI_PASSPHRASE_LENGTH_MIN	8
#define WIFI_PASSPHRASE_LENGTH_MAX	63
#define MH_LABEL_LENGTH_MAX		1024
#define FONT_SIZE_20			20
#define FONT_SIZE_25			25
#define FONT_SIZE_30			30
#define FONT_SIZE_32			32

#define _EDJ(obj)			elm_layout_edje_get(obj)

/* This is from tethering_private.h */
#define TETHERING_MAX_CONNECTED_STA	16	/**< Maximum connected station. 8(Wi-Fi) + 7(BT) + 1(USB) */
#define TETHERING_TYPE_MAX		4	/**< All, USB, Wi-Fi, BT */

typedef enum {
	MH_POPUP_NONE,

	/* Two buttons pop-up */
	MH_POP_WIFI_OFF_CONF,
	MH_POP_WIFI_ON_CONF,
	MH_POP_USB_ON_CONF,

	/* One button pop-up */
	MH_POP_INFORMATION,
	MH_POP_USB_CONNECT,
	MH_POP_WIFI_PASSWORD_SHORT,

	/* No button & timeout pop-up */
	MH_POP_INFORMATION_WO_BUTTON,

	MH_POPUP_TYPE_MAX
} mh_popup_type_e;

typedef enum {
	MH_STATE_NONE,
	MH_STATE_PROCESS,
	MH_STATE_MAX
} mh_state_e;

typedef struct ap_app_main {
	Evas_Object 			*conform;
	Evas_Object			*genlist;

	Evas_Object			*back_btn;
	Evas_Object 			*wifi_btn;
	Evas_Object 			*bt_btn;
	Evas_Object 			*usb_btn;

	Elm_Genlist_Item_Class		*sp_itc;
	Elm_Genlist_Item_Class		*end_sp_itc;
	Elm_Genlist_Item_Class		*wifi_itc;
	Elm_Genlist_Item_Class		*setup_itc;
	Elm_Genlist_Item_Class		*setup_help_itc;
	Elm_Genlist_Item_Class		*bt_itc;
	Elm_Genlist_Item_Class		*usb_itc;
	Elm_Genlist_Item_Class		*help_itc;

	Elm_Genlist_Item_Class		*device_itc;
	Elm_Genlist_Item_Class		*device0_itc;
	Elm_Genlist_Item_Class		*usage_itc;
	Elm_Genlist_Item_Class		*dev_itc[TETHERING_TYPE_MAX];

	Elm_Object_Item			*wifi_item;
	Elm_Object_Item			*setup_item;
	Elm_Object_Item			*setup_help_item;
	Elm_Object_Item			*bt_item;
	Elm_Object_Item			*usb_item;
	Elm_Object_Item			*device_item;
	Elm_Object_Item			*usage_item;
	Elm_Object_Item			*help_item;

	int 				hotspot_mode;
	int				wifi_state;
	int				usb_state;
} mh_main_view_t;

typedef struct {
	Evas_Object 			*conform;
	Evas_Object			*genlist;

	Evas_Object			*back_btn;
	Evas_Object 			*hide_btn;
	Evas_Object 			*security_btn;
	Evas_Object 	 		*pw_layout;
	Evas_Object 	 		*pw_entry;

	Elm_Genlist_Item_Class		*sp_itc;
	Elm_Genlist_Item_Class		*end_sp_itc;
	Elm_Genlist_Item_Class		*hide_itc;
	Elm_Genlist_Item_Class		*security_itc;
	Elm_Genlist_Item_Class		*pw_itc;
	Elm_Genlist_Item_Class		*name_itc;

	Elm_Object_Item			*hide_item;
	Elm_Object_Item			*security_item;
	Elm_Object_Item			*pw_item;
	Elm_Object_Item			*name_item;

	bool 				visibility;
	tethering_wifi_security_type_e	security_type;

	char 				device_name[DEVICE_NAME_LENGTH_MAX + 1];
	char 				wifi_passphrase[WIFI_PASSPHRASE_LENGTH_MAX + 1];
} mh_wifi_setting_view_t;

typedef struct {
	int number;
	tethering_client_h handle[TETHERING_MAX_CONNECTED_STA];
} mh_clients_t;

typedef struct {
	bool			is_updated;
	unsigned long long	pdp_total_sent;
	unsigned long long	pdp_total_receive;
} mh_data_usage_t;

typedef struct {
	void				*gadget;
	tethering_h			handle;
	connection_h			conn_handle;

	Evas_Object 			*win;
	Evas_Object 			*naviframe;
	Evas_Object 			*popup;

	Ecore_Timer			*update_statistics_handle;

	mh_main_view_t			main;
	mh_wifi_setting_view_t		setup;

	mh_data_usage_t			data_statistics;
	mh_clients_t			clients;

	int				popup_type;
	char                            popup_string[MH_LABEL_LENGTH_MAX];
} mh_appdata_t;

typedef struct {
	Evas_Object		*base;
	mh_appdata_t		*ad;
	ui_gadget_h		ug;
} mh_ugdata_t;

#ifdef __cplusplus
}
#endif
#endif /* __MOBILE_HOTSPOT_H__ */
