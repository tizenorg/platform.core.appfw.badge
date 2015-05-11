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

#include <stdlib.h>
#include <stdarg.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"
#include "badge_ipc.h"

EXPORT_API
int badge_create(const char *pkgname, const char *writable_pkg)
{
	char *caller = NULL;
	int err = BADGE_ERROR_NONE;

	if (pkgname == NULL) {
		return BADGE_ERROR_INVALID_PARAMETER;
	}

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	err = badge_ipc_request_insert(pkgname, writable_pkg, caller);

	free(caller);
	return err;
}

EXPORT_API
int badge_new(const char *writable_app_id)
{
	char *caller = NULL;
	int err = BADGE_ERROR_NONE;

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	err = badge_ipc_request_insert(caller, writable_app_id, caller);

	free(caller);
	return err;
}

EXPORT_API
int badge_remove(const char *app_id)
{
	char *caller = NULL;
	int result = BADGE_ERROR_NONE;

	if (app_id == NULL) {
		return BADGE_ERROR_INVALID_PARAMETER;
	}

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	result = badge_ipc_request_delete(app_id, caller);

	free(caller);
	return result;
}

EXPORT_API
int badge_is_existing(const char *app_id, bool *existing)
{
	return _badge_is_existing(app_id, existing);
}


EXPORT_API
int badge_foreach_existed(badge_cb callback, void *data)
{
	return _badge_foreach_existed(callback, data);
}

EXPORT_API
int badge_set_count(const char *app_id, unsigned int count)
{
	char *caller = NULL;
	int result = BADGE_ERROR_NONE;

	if (app_id == NULL) {
		return BADGE_ERROR_INVALID_PARAMETER;
	}

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	result = badge_ipc_request_set_count(app_id, caller, count);

	free(caller);
	return result;
}

EXPORT_API
int badge_get_count(const char *app_id, unsigned int *count)
{
	return _badget_get_count(app_id, count);
}

EXPORT_API
int badge_set_display(const char *app_id, unsigned int is_display)
{
	char *caller = NULL;
	int result = BADGE_ERROR_NONE;

	if (app_id == NULL) {
		return BADGE_ERROR_INVALID_PARAMETER;
	}

	caller = _badge_get_pkgname_by_pid();
	if (!caller) {
		ERR("fail to get caller pkgname");
		return BADGE_ERROR_PERMISSION_DENIED;
	}

	result = badge_ipc_request_set_display(app_id, caller, is_display);

	free(caller);
	return result;
}

EXPORT_API
int badge_get_display(const char *app_id, unsigned int *is_display)
{
	return _badget_get_display(app_id, is_display);
}

EXPORT_API
int badge_register_changed_cb(badge_change_cb callback, void *data)
{
	if (callback == NULL) {
		return BADGE_ERROR_INVALID_PARAMETER;
	}

	return _badge_register_changed_cb(callback, data);
}

EXPORT_API
int badge_unregister_changed_cb(badge_change_cb callback)
{
	if (callback == NULL) {
		return BADGE_ERROR_INVALID_PARAMETER;
	}

	return _badge_unregister_changed_cb(callback);
}

EXPORT_API
int badge_is_service_ready(void)
{
	return badge_ipc_is_master_ready();
}

EXPORT_API
int badge_add_deferred_task(
		void (*badge_add_deferred_task)(void *data), void *user_data)
{
	return badge_ipc_add_deferred_task(badge_add_deferred_task, user_data);
}

EXPORT_API
int badge_del_deferred_task(
		void (*badge_add_deferred_task)(void *data))
{
	return badge_ipc_del_deferred_task(badge_add_deferred_task);
}
