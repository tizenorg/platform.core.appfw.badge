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
#include <unistd.h>
#include <package_manager.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"
#include "badge_ipc.h"

EXPORT_API
int badge_create(const char *pkgname, const char *writable_pkg)
{
	return badge_create_for_uid(pkgname, writable_pkg, getuid());
}

EXPORT_API
int badge_new(const char *writable_app_id)
{
	return badge_new_for_uid(writable_app_id, getuid());
}

EXPORT_API
int badge_add(const char *badge_app_id)
{
	return badge_add_for_uid(badge_app_id, getuid());
}

EXPORT_API
int badge_remove(const char *app_id)
{
	return badge_remove_for_uid(app_id, getuid());
}

EXPORT_API
int badge_is_existing(const char *app_id, bool *existing)
{
	return badge_is_existing_for_uid(app_id, existing, getuid());
}

EXPORT_API
int badge_foreach(badge_foreach_cb callback, void *user_data)
{
	return badge_foreach_for_uid(callback, user_data, getuid());
}

EXPORT_API
int badge_set_count(const char *app_id, unsigned int count)
{
	return badge_set_count_for_uid(app_id, count, getuid());
}

EXPORT_API
int badge_get_count(const char *app_id, unsigned int *count)
{
	return badge_get_count_for_uid(app_id, count, getuid());
}

EXPORT_API
int badge_set_display(const char *app_id, unsigned int is_display)
{
	return badge_set_display_for_uid(app_id, is_display, getuid());
}

EXPORT_API
int badge_get_display(const char *app_id, unsigned int *is_display)
{
	return badge_get_display_for_uid(app_id, is_display, getuid());
}


EXPORT_API
int badge_register_changed_cb(badge_change_cb callback, void *data)
{
	return badge_register_changed_cb_for_uid(callback, data, getuid());
}

EXPORT_API
int badge_unregister_changed_cb(badge_change_cb callback)
{
	return badge_unregister_changed_cb_for_uid(callback, getuid());
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
