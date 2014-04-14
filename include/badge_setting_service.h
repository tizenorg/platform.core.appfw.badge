/*
 *  libbadge
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjoo Park <yjoo93.park@samsung.com>,
 *      Seungtaek Chung <seungtaek.chung@samsung.com>, Youngsub Ko <ys4610.ko@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __BADGE_SETTING_SERVICE_H__
#define __BADGE_SETTING_SERVICE_H__

#include <stdbool.h>
#include <badge_error.h>

#ifdef __cplusplus
extern "C" {
#endif

badge_error_e badge_setting_db_set(const char *pkgname, const char *property, const char *value);
badge_error_e badge_setting_db_get(const char *pkgname, const char *property, char **value);

#ifdef __cplusplus
}
#endif
#endif
