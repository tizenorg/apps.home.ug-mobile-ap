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

#ifndef __APP_MH_VIEW_WIFI_SETUP_H__
#define __APP_MH_VIEW_WIFI_SETUP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mobile_hotspot.h"
#include "mh_func_onoff.h"
#include "mh_common_utility.h"

void mh_draw_wifi_setup_view(mh_appdata_t *ad);
Eina_Bool _setting_back_btn_cb(void *data, Elm_Object_Item *navi_item);

#ifdef __cplusplus
}
#endif
#endif
