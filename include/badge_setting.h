/*
 *  libbadge
 *
 * Copyright (c) 2000 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __BADGE_SETTING_H__
#define __BADGE_SETTING_H__

#include <stdbool.h>
#include <sys/types.h>
#include <badge_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function sets badge property for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] property name
 * @param[in] property value
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 */
int badge_setting_property_set(const char *pkgname, const char *property, const char *value);

int badge_setting_property_set_for_uid(const char *pkgname, const char *property, const char *value, uid_t uid);

/**
 * @brief This function gets badge property for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] property name
 * @param[in] pointer which can save the getting value
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 */
int badge_setting_property_get(const char *pkgname, const char *property, char **value);

int badge_setting_property_get_for_uid(const char *pkgname, const char *property, char **value, uid_t uid);

#ifdef __cplusplus
}
#endif
#endif
