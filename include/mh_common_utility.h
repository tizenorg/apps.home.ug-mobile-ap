/*
* ug-mobile-ap
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://floralicense.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef __APP_MH_COMMON_UTILITY_H_
#define __APP_MH_COMMON_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mobile_hotspot.h"
#include "mh_view_main.h"

#define MH_POPUP_TIMEOUT	2.0f

void _prepare_popup_with_content(int type, Evas_Object *obj);
void _prepare_popup(int type, const char *str);
Eina_Bool _create_popup(mh_appdata_t *ad);
void _destroy_popup(mh_appdata_t *ad);
Evas_Object *_create_bg(Evas_Object *parent, const char *style);
Evas_Object *_create_win_layout(mh_appdata_t *ad);
Evas_Object *_create_naviframe(Evas_Object *parent);
void _handle_usb_mode_change(keynode_t *key, void *data);
int _get_vconf_hotspot_mode(void);
Eina_Bool _get_no_of_connected_device(tethering_h handle, int *no, tethering_type_e type);
Evas_Object *_create_label(Evas_Object *parent, const char *text);
Evas_Object *_create_slide_title(Evas_Object *parent, const char *text);

#ifdef __cplusplus
}
#endif
#endif
