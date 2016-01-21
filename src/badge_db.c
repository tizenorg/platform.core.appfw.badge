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
#include <sqlite3.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"

EXPORT_API
int badge_db_insert(const char *pkgname, const char *writable_pkg, const char *caller)
{
	int err = BADGE_ERROR_NONE;
	badge_h *badge = NULL;
	char *pkgs = NULL;

	if (!pkgname) {
		WARN("package name is NULL");
		return BADGE_ERROR_INVALID_PARAMETER;
	}

	pkgs = _badge_pkgs_new(&err, writable_pkg, caller, NULL);
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
int badge_db_delete(const char *pkgname, const char *caller)
{
	int result = BADGE_ERROR_NONE;

	result = _badge_remove(caller, pkgname);

	return result;
}

EXPORT_API
int badge_db_set_count(const char *pkgname, const char *caller, int count)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_set_count(caller, pkgname, count);

	return result;
}

EXPORT_API
int badge_db_get_count(const char *pkgname, int *count)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_get_count(pkgname, count);

	return result;
}

EXPORT_API
int badge_db_set_display_option(const char *pkgname, const char *caller, int is_display)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_set_display(pkgname, is_display);

	return result;
}

EXPORT_API
int badge_db_get_display_option(const char *pkgname, int *is_display)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_set_display(pkgname, is_display);

	return result;
}

EXPORT_API
int badge_db_exec(sqlite3 * db, const char *query, int *num_changes)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	if (db == NULL)
		return BADGE_ERROR_INVALID_PARAMETER;

	if (query == NULL)
		return BADGE_ERROR_INVALID_PARAMETER;

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		ERR("DB err(%d) : %s", ret,
				 sqlite3_errmsg(db));
		return BADGE_ERROR_FROM_DB;
	}

	if (stmt != NULL) {
		ret = sqlite3_step(stmt);
		if (ret == SQLITE_OK || ret == SQLITE_DONE) {
			if (num_changes != NULL)
				*num_changes = sqlite3_changes(db);

			sqlite3_finalize(stmt);
		} else {
			ERR("DB err(%d) : %s", ret,
					 sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			return BADGE_ERROR_FROM_DB;
		}
	} else {
			return BADGE_ERROR_FROM_DB;
	}

	return BADGE_ERROR_NONE;
}
