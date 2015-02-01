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

#ifndef __APP_MH_POPUP_H_
#define __APP_MH_POPUP_H_

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
void _teth_on(mh_appdata_t* ad);
void _prepare_popup_type(int type);
void _handle_mobile_data_onoff(mh_appdata_t *ad);

#ifdef __cplusplus
}
#endif
#endif
