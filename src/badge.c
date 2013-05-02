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
#include "badge_ipc.h"

EXPORT_API
badge_error_e badge_create(const char *pkgname, const char *writable_pkg)
{
	char *caller = NULL;
	badge_error_e err = BADGE_ERROR_NONE;

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	err = badge_ipc_request_insert(pkgname, writable_pkg, caller);

	return err;
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

	result = badge_ipc_request_delete(pkgname, caller);

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

	result = badge_ipc_request_set_count(pkgname, caller, count);

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
	char *caller = NULL;
	badge_error_e result = BADGE_ERROR_NONE;

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	result = badge_ipc_request_set_display(pkgname, caller, is_display);

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

EXPORT_API
int badge_is_service_ready(void)
{
	return badge_ipc_is_master_ready();
}

EXPORT_API
badge_error_e badge_add_deffered_task(
		void (*deffered_task_cb)(void *data), void *user_data)
{
	return badge_ipc_add_deffered_task(deffered_task_cb, user_data);
}

EXPORT_API
badge_error_e badge_del_deffered_task(
		void (*deffered_task_cb)(void *data))
{
	return badge_ipc_del_deffered_task(deffered_task_cb);
}
