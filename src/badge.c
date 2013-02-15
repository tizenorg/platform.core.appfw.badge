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

#include <stdlib.h>
#include <stdarg.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"

EXPORT_API
badge_error_e badge_create(const char *pkgname, const char *writable_pkg)
{
	badge_error_e err = BADGE_ERROR_NONE;
	badge_h *badge = NULL;
	char *pkgs = NULL;

	if (!pkgname) {
		WARN("package name is NULL");
		return BADGE_ERROR_INVALID_DATA;
	}

	pkgs = _badge_pkgs_new(&err, writable_pkg, NULL);
	if (!pkgs) {
		ERR("fail to _badge_pkgs_new : %d", err);
		return err;
	}

	INFO("pkgs : %s", pkgs);

	badge = _badge_new(pkgname, pkgs, &err);
	if (!badge) {
		ERR("fail to _badge_new : %d", err);
		free(pkgs);
		return err;
	}
	free(pkgs);

	err = _badge_insert(badge);
	if (err != BADGE_ERROR_NONE) {
		ERR("fail to _badge_insert : %d", err);
		_badge_free(badge);
		return err;
	}

	_badge_free(badge);

	return BADGE_ERROR_NONE;
}

EXPORT_API
badge_error_e badge_remove(const char *pkgname)
{
	char *caller = NULL;
	badge_error_e result = BADGE_ERROR_NONE;

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;

	}

	result = _badge_remove(caller, pkgname);
	free(caller);

	return result;
}

EXPORT_API
badge_error_e badge_is_existing(const char *pkgname, bool *existing)
{
	return _badge_is_existing(pkgname, existing);
}


EXPORT_API
badge_error_e badge_foreach_existed(badge_cb callback, void *data)
{
	return _badge_foreach_existed(callback, data);
}

EXPORT_API
badge_error_e badge_set_count(const char *pkgname, unsigned int count)
{
	char *caller = NULL;
	badge_error_e result = BADGE_ERROR_NONE;

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	result = _badget_set_count(caller, pkgname, count);

	free(caller);

	return result;
}

EXPORT_API
badge_error_e badge_get_count(const char *pkgname, unsigned int *count)
{
	return _badget_get_count(pkgname, count);
}

EXPORT_API
badge_error_e badge_set_display(const char *pkgname, unsigned int is_display)
{
	badge_error_e result = BADGE_ERROR_NONE;

	result = _badget_set_display(pkgname, is_display);

	return result;
}

EXPORT_API
badge_error_e badge_get_display(const char *pkgname, unsigned int *is_display)
{
	return _badget_get_display(pkgname, is_display);
}

EXPORT_API
badge_error_e badge_register_changed_cb(badge_change_cb callback, void *data)
{
	return _badge_register_changed_cb(callback, data);
}

EXPORT_API
badge_error_e badge_unregister_changed_cb(badge_change_cb callback)
{
	return _badge_unregister_changed_cb(callback);
}

