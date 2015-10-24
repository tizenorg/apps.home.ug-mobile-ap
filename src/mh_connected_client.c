/*
 * mh_connected_client.c
 *
 *  Created on: 05-Sep-2014
 *      Author: sach.sharma
 */
#include <time.h>

#include "mh_view_main.h"
#include "mobile_hotspot.h"
#include "mh_string.h"

static void __deconstruct_conn_clients_view(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	mh_conn_client_view_t *client = &ad->connected_device;
	int no_of_dev;
	int i = 0;

	no_of_dev = _get_list_clients_count(ad);
	for (i = 0; i < no_of_dev; i++) {
		_free_genlist_item(&ad->connected_device.station_items[i]);
	}

	_free_genlist_itc(&client->dev_itc[TETHERING_TYPE_WIFI]);
	_free_genlist_itc(&client->dev_itc[TETHERING_TYPE_BT]);
	_free_genlist_itc(&client->dev_itc[TETHERING_TYPE_USB]);

	evas_object_del(client->genlist);
	client->genlist = NULL;
	client->navi_it = NULL;
	_stop_update_device_conn_time(ad);

	__MOBILE_AP_FUNC_EXIT__;
}

static void __update_connected_client_view(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}

	Elm_Object_Item *item = NULL;
	tethering_type_e connection_type = 0;
	GSList *l = NULL;
	tethering_client_h *handle;
	int count = 0;
	mh_conn_client_view_t *cli_view = &(ad->connected_device);

	elm_genlist_clear(cli_view->genlist);

	for (l = ad->client_list; l != NULL; l = g_slist_next(l) ) {
		handle = (tethering_client_h *)l->data;
		tethering_client_get_tethering_type(handle, &connection_type);

		item = elm_genlist_item_append(cli_view->genlist,
				cli_view->dev_itc[connection_type],
				(void *)handle,
				NULL, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		if (item == NULL) {
			ERR("elm_genlist_item_append is failed\n");
			continue;
		}
		cli_view->station_items[count++] = item;
		elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
	cli_view->no_of_clients = count;
	_start_update_device_conn_time(ad);
	__MOBILE_AP_FUNC_EXIT__;
	return;
}

void _update_conn_clients(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("Invalid param\n");
		return;
	}
	unsigned int no_of_dev = 0;
	no_of_dev = _get_list_clients_count(ad);

	if (no_of_dev == 0) {
		__deconstruct_conn_clients_view(ad);
		elm_naviframe_item_pop(ad->naviframe);
	} else {
		__update_connected_client_view(ad);
	}
	__MOBILE_AP_FUNC_EXIT__;
	return;
}

static char *__get_diff_time(time_t connection_time)
{
	time_t current_time;
	char buf[MH_LABEL_LENGTH_MAX] = {0, };
	int day;
	int hour;
	int min;
	int sec;
	double diff;

	time(&current_time);
	diff = difftime(current_time, connection_time);
	day = diff / (60 * 60 * 24);
	diff = diff - (day * 60 * 60 * 24);
	hour = diff / (60 * 60);
	diff = diff - (hour * 60 * 60);
	min = diff / 60;
	diff = diff - (min * 60);
	sec = diff;

	if (day > 0)
		hour = hour + day * 24;

	if (hour > 0)
		snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hour, min, sec);
	else
		snprintf(buf, sizeof(buf), "%02d:%02d", min, sec);

	return g_strdup(buf);
}

static char *__gl_get_dev_label(void *data, Evas_Object *obj, const char *part)
{
	__MOBILE_AP_FUNC_ENTER__;

	tethering_client_h client = (tethering_client_h)data;
	char *name = NULL;
	char *dev_name = NULL;
	time_t tm;

	if (data == NULL || obj == NULL || part == NULL) {
		ERR("Invalid param\n");
		return NULL;
	}

	if (!strcmp("elm.text", part)) {
		tethering_client_get_name(client, &name);
		if (NULL == name) {
			return NULL;
		}

		if (!strcmp(name, "UNKNOWN")) {
			free(name);
			return strdup(STR_NO_NAME);
		}

		dev_name = elm_entry_utf8_to_markup(name);
		free(name);
		return dev_name;
	} else if (!strcmp("elm.text.sub", part)) {
		tethering_client_get_time(client, &tm);
		return __get_diff_time(tm);
	}

	__MOBILE_AP_FUNC_EXIT__;
	return NULL;
}

static Evas_Object *__gl_get_dev_wifi_icon(void *data, Evas_Object *obj,
							const char *part)
{
	Evas_Object *ly = NULL;
	Evas_Object *icon = NULL;

	if (!strcmp("elm.swallow.icon", part)) {
		ly = elm_layout_add(obj);
		elm_layout_theme_set(ly, "layout", "list/C/type.1", "default");

		icon = elm_icon_add(obj);
		elm_image_file_set(icon, EDJDIR"/"TETHERING_IMAGES_EDJ, WIFI_ICON);
		evas_object_color_set(icon, 2, 61, 132, 255);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(ly, "elm.swallow.content", icon);
	}

	return ly;
}

static Evas_Object *__gl_get_dev_usb_icon(void *data, Evas_Object *obj,
							const char *part)
{
	Evas_Object *ly = NULL;
	Evas_Object *icon = NULL;

	if (!strcmp("elm.swallow.icon", part)) {
		ly = elm_layout_add(obj);
		elm_layout_theme_set(ly, "layout", "list/C/type.1", "default");

		icon = elm_icon_add(obj);
		elm_image_file_set(icon, EDJDIR"/"TETHERING_IMAGES_EDJ, USB_ICON);
		evas_object_color_set(icon, 2, 61, 132, 255);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(ly, "elm.swallow.content", icon);
	}

	return ly;
}

static Evas_Object *__gl_get_dev_bt_icon(void *data, Evas_Object *obj,
							const char *part)
{
	Evas_Object *ly = NULL;
	Evas_Object *icon = NULL;

	if (!strcmp("elm.swallow.icon", part)) {
		ly = elm_layout_add(obj);
		elm_layout_theme_set(ly, "layout", "list/C/type.1", "default");

		icon = elm_image_add(obj);
		elm_image_file_set(icon, EDJDIR"/"TETHERING_IMAGES_EDJ, BT_ICON);
		evas_object_color_set(icon, 2, 61, 132, 255);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_content_set(ly, "elm.swallow.content", icon);
	}

	return ly;
}

static void __set_genlist_itc(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	ad->connected_device.dev_itc[TETHERING_TYPE_WIFI] = elm_genlist_item_class_new();
	ad->connected_device.dev_itc[TETHERING_TYPE_WIFI]->item_style = MH_GENLIST_2LINE_TOP_TEXT_ICON_STYLE;
	ad->connected_device.dev_itc[TETHERING_TYPE_WIFI]->func.text_get = __gl_get_dev_label;
	ad->connected_device.dev_itc[TETHERING_TYPE_WIFI]->func.content_get = __gl_get_dev_wifi_icon;
	ad->connected_device.dev_itc[TETHERING_TYPE_WIFI]->func.state_get = NULL;
	ad->connected_device.dev_itc[TETHERING_TYPE_WIFI]->func.del = NULL;

	ad->connected_device.dev_itc[TETHERING_TYPE_USB] = elm_genlist_item_class_new();
	ad->connected_device.dev_itc[TETHERING_TYPE_USB]->item_style = MH_GENLIST_2LINE_TOP_TEXT_ICON_STYLE;
	ad->connected_device.dev_itc[TETHERING_TYPE_USB]->func.text_get = __gl_get_dev_label;
	ad->connected_device.dev_itc[TETHERING_TYPE_USB]->func.content_get = __gl_get_dev_usb_icon;
	ad->connected_device.dev_itc[TETHERING_TYPE_USB]->func.state_get = NULL;
	ad->connected_device.dev_itc[TETHERING_TYPE_USB]->func.del = NULL;

	ad->connected_device.dev_itc[TETHERING_TYPE_BT] = elm_genlist_item_class_new();
	ad->connected_device.dev_itc[TETHERING_TYPE_BT]->item_style = MH_GENLIST_2LINE_TOP_TEXT_ICON_STYLE;
	ad->connected_device.dev_itc[TETHERING_TYPE_BT]->func.text_get = __gl_get_dev_label;
	ad->connected_device.dev_itc[TETHERING_TYPE_BT]->func.content_get = __gl_get_dev_bt_icon;
	ad->connected_device.dev_itc[TETHERING_TYPE_BT]->func.state_get = NULL;
	ad->connected_device.dev_itc[TETHERING_TYPE_BT]->func.del = NULL;

	__MOBILE_AP_FUNC_EXIT__;
}

static void __scroll_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	_stop_update_device_conn_time((mh_appdata_t *)data);
	__MOBILE_AP_FUNC_EXIT__;
}

static void __scroll_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	__MOBILE_AP_FUNC_ENTER__;

	_start_update_device_conn_time((mh_appdata_t *)data);
	__MOBILE_AP_FUNC_EXIT__;
}

static void __create_genlist(mh_appdata_t *ad)
{
	__MOBILE_AP_FUNC_ENTER__;

	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	Evas_Object *genlist;
	Elm_Object_Item *item = NULL;
	tethering_type_e connection_type = 0;
	GSList *l = NULL;
	tethering_client_h *handle;
	int count = 0;

	mh_conn_client_view_t *client = &ad->connected_device;

	genlist = elm_genlist_add(ad->naviframe);
	if (genlist == NULL) {
		ERR("genlist is NULL\n");
		return;
	}

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	client->genlist = genlist;

	__set_genlist_itc(ad);

	for (l = ad->client_list; l != NULL; l = g_slist_next(l) ) {
		handle = (tethering_client_h *)l->data;
		tethering_client_get_tethering_type(handle, &connection_type);

		item = elm_genlist_item_append(client->genlist,
				client->dev_itc[connection_type],
				(void *)handle,
				NULL, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		if (item == NULL) {
			ERR("elm_genlist_item_append is failed\n");
			continue;
		}
		client->station_items[count++] = item;
		elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
	evas_object_smart_callback_add(genlist, "scroll,anim,start",
				__scroll_start_cb, ad);
	evas_object_smart_callback_add(genlist, "scroll,drag,start",
				__scroll_start_cb, ad);
	evas_object_smart_callback_add(genlist, "scroll,anim,stop",
				__scroll_stop_cb, ad);
	evas_object_smart_callback_add(genlist, "scroll,drag,stop",
				__scroll_stop_cb, ad);

	client->no_of_clients = count;
	_start_update_device_conn_time(ad);

	__MOBILE_AP_FUNC_EXIT__;
}

static Eina_Bool __back_btn_cb(void *data, Elm_Object_Item *navi_item)
{
	DBG("+\n");

	if (data == NULL) {
		ERR("The param is NULL\n");
		return EINA_FALSE;
	}

	mh_appdata_t *ad = (mh_appdata_t *)data;

	__deconstruct_conn_clients_view(ad);

	DBG("-\n");
	return EINA_TRUE;
}

void _create_connected_client_view(mh_appdata_t *ad)
{
	DBG("+\n");
	if (ad == NULL) {
		ERR("ad is NULL\n");
		return;
	}

	mh_conn_client_view_t *client = &ad->connected_device;

	if (client->navi_it != NULL) {
		ERR("connected device view already exists\n");
		return;
	}

	__create_genlist(ad);
	if (client->genlist == NULL) {
		ERR("__create_genlist failed NULL\n");
		return;
	}

	client->navi_it = elm_naviframe_item_push(ad->naviframe, STR_CONN_DEVICES, NULL, NULL, client->genlist, NULL);
	elm_object_item_domain_text_translatable_set(client->navi_it, PKGNAME, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(client->navi_it, __back_btn_cb, (void *)ad);
	DBG("-\n");
}
