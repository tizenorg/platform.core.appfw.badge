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

#ifndef __BADGE_INTERNAL_DEF_H__
#define __BADGE_INTERNAL_DEF_H__

#include <stdbool.h>
#include <stdarg.h>

#include "badge_error.h"
#include "badge.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

typedef struct _badge_h badge_h;

char *_badge_get_pkgname_by_pid(void);

badge_error_e _badge_is_existing(const char *pkgname, bool *existing);

badge_error_e _badge_foreach_existed(badge_cb callback, void *data);

badge_error_e _badge_insert(badge_h *badge);

badge_error_e _badge_remove(const char *caller, const char *pkgname);

badge_error_e _badget_set_count(const char *caller, const char *pkgname,
			unsigned int count);

badge_error_e _badget_get_count(const char *pkgname, unsigned int *count);

badge_error_e _badget_set_display(const char *pkgname,
			unsigned int is_display);

badge_error_e _badget_get_display(const char *pkgname, unsigned int *is_display);

badge_error_e _badge_register_changed_cb(badge_change_cb callback, void *data);

badge_error_e _badge_unregister_changed_cb(badge_change_cb callback);

badge_error_e _badge_free(badge_h *badge);

badge_h *_badge_new(const char *pkgname, const char *writable_pkgs,
		badge_error_e *err);

char *_badge_pkgs_new(badge_error_e *err, const char *pkg1, ...);

char *_badge_pkgs_new_valist(badge_error_e *err,
			const char *pkg1, va_list args);

void badge_changed_cb_call(unsigned int action, const char *pkgname,
			unsigned int count);

#endif /* __BADGE_INTERNAL_DEF_H__ */

