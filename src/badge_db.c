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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sqlite3.h>
#include <tzplatform_config.h>
#include <db-util.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"

#define BADGE_DB_NAME ".badge.db"
#define CREATE_BADGE_TABLE " \
PRAGMA journal_mode = PERSIST; \
create table if not exists badge_data ( \
	pkgname TEXT NOT NULL, \
	writable_pkgs TEXT, \
	badge INTEGER default 0, \
	rowid INTEGER PRIMARY KEY AUTOINCREMENT, \
	uid INTEGER, \
	UNIQUE (pkgname, uid) \
); \
create table if not exists badge_option ( \
	pkgname TEXT NOT NULL, \
	display INTEGER default 1, \
	uid INTEGER, \
	UNIQUE (pkgname, uid) \
); "

EXPORT_API
int badge_db_init()
{
	int r;
	sqlite3 *db = NULL;
	char *errmsg = NULL;
	char defname[FILENAME_MAX];
	const char *db_path = tzplatform_getenv(TZ_SYS_DB);
	if (db_path == NULL) {
		ERR("fail to get db_path");
		return BADGE_ERROR_OUT_OF_MEMORY;
	}
	snprintf(defname, sizeof(defname), "%s/%s", db_path, BADGE_DB_NAME);

	DBG("db path : %s", defname);
	r = sqlite3_open_v2(defname, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	if (r) {
		db_util_close(db);
		ERR("fail to open notification db %d", r);
		return BADGE_ERROR_IO_ERROR;
	}

	r = sqlite3_exec(db, CREATE_BADGE_TABLE, NULL, NULL, &errmsg);
	if (r != SQLITE_OK) {
		ERR("query error(%d)(%s)", r, errmsg);
		sqlite3_free(errmsg);
		db_util_close(db);
		return BADGE_ERROR_IO_ERROR;
	}

	db_util_close(db);
	return BADGE_ERROR_NONE;
}

EXPORT_API
int badge_db_is_existing(const char *pkgname, bool *existing)
{
	int result = BADGE_ERROR_NONE;
	result = _badge_is_existing(pkgname, existing);
	return result;
}

EXPORT_API
int badge_db_get_list(GList **badge_list, uid_t uid)
{
	int result = BADGE_ERROR_NONE;
	result = _badge_get_list(badge_list, uid);
	return result;
}

EXPORT_API
int badge_db_insert(const char *pkgname, const char *writable_pkg, const char *caller, uid_t uid)
{
	int err = BADGE_ERROR_NONE;
	badge_h *badge = NULL;
	char *pkgs = NULL;

	if (!pkgname) {
		ERR("package name is NULL");
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

	err = _badge_insert(badge, uid);
	if (err != BADGE_ERROR_NONE) {
		ERR("fail to _badge_insert : %d", err);
		_badge_free(badge);
		return err;
	}

	_badge_free(badge);

	return BADGE_ERROR_NONE;
}

EXPORT_API
int badge_db_delete(const char *pkgname, const char *caller, uid_t uid)
{
	int result = BADGE_ERROR_NONE;

	result = _badge_remove(caller, pkgname, uid);

	return result;
}

EXPORT_API
int badge_db_set_count(const char *pkgname, const char *caller, unsigned int count, uid_t uid)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_set_count(caller, pkgname, count, uid);

	return result;
}

EXPORT_API
int badge_db_get_count(const char *pkgname, unsigned int *count, uid_t uid)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_get_count(pkgname, count, uid);

	return result;
}

EXPORT_API
int badge_db_set_display_option(const char *pkgname, const char *caller, unsigned int is_display, uid_t uid)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_set_display(pkgname, is_display, uid);

	return result;
}

EXPORT_API
int badge_db_get_display_option(const char *pkgname, unsigned int *is_display, uid_t uid)
{
	int result = BADGE_ERROR_NONE;

	result = _badget_get_display(pkgname, is_display, uid);

	return result;
}

EXPORT_API
int badge_db_exec(sqlite3 *db, const char *query, int *num_changes)
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
