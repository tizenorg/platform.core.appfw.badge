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

#ifndef __BADGE_INTERNAL_DEF_H__
#define __BADGE_INTERNAL_DEF_H__

#include <stdbool.h>
#include <stdarg.h>

#include "badge_error.h"
#include "badge.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _badge_h badge_h;

char *_badge_get_pkgname_by_pid(void);

int _badge_is_existing(const char *pkgname, bool *existing);

int _badge_foreach_existed(badge_cb callback, void *data);

int _badge_insert(badge_h *badge);

int _badge_remove(const char *caller, const char *pkgname);

int _badget_set_count(const char *caller, const char *pkgname,
			unsigned int count);

int _badget_get_count(const char *pkgname, unsigned int *count);

int _badget_set_display(const char *pkgname,
			unsigned int is_display);

int _badget_get_display(const char *pkgname, unsigned int *is_display);

int _badge_register_changed_cb(badge_change_cb callback, void *data);

int _badge_unregister_changed_cb(badge_change_cb callback);

int _badge_free(badge_h *badge);

badge_h *_badge_new(const char *pkgname, const char *writable_pkgs,
		int *err);

char *_badge_pkgs_new(int *err, const char *pkg1, ...);

char *_badge_pkgs_new_valist(int *err,
			const char *pkg1, va_list args);

void badge_changed_cb_call(unsigned int action, const char *pkgname,
			unsigned int count);

/**
 * @internal
 * @brief Creates a badge for the designated package.
 * @since_tizen @if WEARABLE 2.3.1 @elseif MOBILE 2.3 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @details Creates new badge to display.
 * @param[in] pkgname The name of the designated package
 * @param[in] writable_pkg The name of package which is authorized to change the badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;

	err = badge_create("org.tizen.sms", "org.tizen.sms2");
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_create(const char *pkgname, const char *writable_pkg);

/**
 * @brief This function sets badge property for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] property name
 * @param[in] property value
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 */
int badge_setting_property_set(const char *pkgname, const char *property, const char *value);

/**
 * @brief This function gets badge property for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] property name
 * @param[in] pointer which can save the getting value
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 */
int badge_setting_property_get(const char *pkgname, const char *property, char **value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BADGE_INTERNAL_DEF_H__ */

