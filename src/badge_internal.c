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

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <aul.h>
#include <sqlite3.h>
#include <db-util.h>

#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"

#define BADGE_PKGNAME_LEN 512
#define BADGE_TABLE_NAME "badge_data"
#define BADGE_OPTION_TABLE_NAME "badge_option"

#define BADGE_CHANGED_NOTI	"badge_changed"
#define BADGE_DBUS_BUS_NAME	"org.tizen.libbadge"
#define BADGE_DBUS_PATH		"/org/tizen/libbadge"
#define BADGE_DBUS_INTERFACE	"org.tizen.libbadge.signal"

struct _badge_h {
	char *pkgname;
	char *writable_pkgs;
};

struct _badge_cb_data {
	badge_change_cb callback;
	void *data;
};

static GList *g_badge_cb_list;
static DBusConnection *g_badge_cb_handle;

static inline long _get_max_len(void)
{
	long max = 0;
	long path_max_len = 4096;

#ifdef _PC_PATH_MAX
	max = (pathconf("/", _PC_PATH_MAX) < 1 ? path_max_len
				: pathconf("/", _PC_PATH_MAX));
#else /* _PC_PATH_MAX */
	max = path_max_len;
#endif /* _PC_PATH_MAX */
	return max;
}

char *_badge_get_pkgname_by_pid(void)
{
	char *pkgname = NULL;
	int pid = 0;
	int ret = AUL_R_OK;
	int fd = 0;
	long max = 0;

	pid = getpid();
	max = _get_max_len();
	pkgname = malloc(max);
	if (!pkgname) {
		ERR("fail to alloc memory");
		return NULL;
	}
	memset(pkgname, 0x00, max);

	ret = aul_app_get_pkgname_bypid(pid, pkgname, max);
	if (ret != AUL_R_OK) {
		fd = open("/proc/self/cmdline", O_RDONLY);
		if (fd < 0) {
			free(pkgname);
			return NULL;
		}

		ret = read(fd, pkgname, max - 1);
		if (ret <= 0) {
			close(fd);
			free(pkgname);
			return NULL;
		}

		close(fd);
	}

	if (pkgname[0] == '\0') {
		free(pkgname);
		return NULL;
	} else
		return pkgname;
}


static void _badge_changed(unsigned int action, const char *pkgname,
			unsigned int count)
{
	DBusConnection *connection = NULL;
	DBusMessage *message = NULL;
	DBusError err;
	dbus_bool_t ret;

	if (!pkgname) {
		ERR("pkgname is NULL");
		return;
	}

	dbus_error_init(&err);
	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!connection) {
		ERR("Fail to dbus_bus_get : %s", err.message);
		dbus_error_free(&err);
		return;
	}

	message = dbus_message_new_signal(BADGE_DBUS_PATH,
				BADGE_DBUS_INTERFACE,
				BADGE_CHANGED_NOTI);

	if (!message) {
		ERR("fail to create dbus message");
		goto release_n_return;
	}

	dbus_message_append_args(message,
				DBUS_TYPE_UINT32, &action,
				DBUS_TYPE_STRING, &pkgname,
				DBUS_TYPE_UINT32, &count,
				DBUS_TYPE_INVALID);

	ret = dbus_connection_send(connection, message, NULL);
	if (!ret) {
		ERR("fail to send dbus message : [%u][%s][%u]",
				action, pkgname, count);
		goto release_n_return;
	}

	dbus_connection_flush(connection);

	DBG("success to emit signal [%u][%s][%u]",
			action, pkgname, count);

release_n_return:
	dbus_error_free(&err);

	if (message)
		dbus_message_unref(message);

	if (connection)
		dbus_connection_unref(connection);
}

static badge_error_e _badge_check_data_inserted(const char *pkgname,
					sqlite3 *db)
{
	sqlite3_stmt *stmt = NULL;
	int count = 0;
	int result = BADGE_ERROR_NONE;
	char *sqlbuf = NULL;
	int sqlret;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	if (!db)
		return BADGE_ERROR_INVALID_DATA;

	sqlbuf = sqlite3_mprintf("SELECT count(*) FROM %s WHERE " \
			 "pkgname = %Q",
			 BADGE_TABLE_NAME, pkgname);

	if (!sqlbuf) {
		ERR("fail to alloc sql query");
		return BADGE_ERROR_NO_MEMORY;
	}

	sqlret = sqlite3_prepare_v2(db, sqlbuf, -1, &stmt, NULL);
	if (sqlret != SQLITE_OK) {
		ERR("DB err [%s]", sqlite3_errmsg(db));
		ERR("query[%s]", sqlbuf);
		result = BADGE_ERROR_FROM_DB;
		goto free_and_return;
	}

	sqlret = sqlite3_step(stmt);
	if (sqlret == SQLITE_ROW)
		count = sqlite3_column_int(stmt, 0);
	else
		count = 0;

	DBG("[%s], count[%d]", sqlbuf, count);

	if (count > 0)
		result = BADGE_ERROR_ALREADY_EXIST;
	else
		result = BADGE_ERROR_NOT_EXIST;

free_and_return:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (stmt)
		sqlite3_finalize(stmt);

	return result;
}

static badge_error_e _badge_check_option_inserted(const char *pkgname,
					sqlite3 *db)
{
	sqlite3_stmt *stmt = NULL;
	int count = 0;
	int result = BADGE_ERROR_NONE;
	char *sqlbuf = NULL;
	int sqlret;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	if (!db)
		return BADGE_ERROR_INVALID_DATA;

	sqlbuf = sqlite3_mprintf("SELECT count(*) FROM %s WHERE " \
			 "pkgname = %Q",
			 BADGE_OPTION_TABLE_NAME, pkgname);

	if (!sqlbuf) {
		ERR("fail to alloc sql query");
		return BADGE_ERROR_NO_MEMORY;
	}

	sqlret = sqlite3_prepare_v2(db, sqlbuf, -1, &stmt, NULL);
	if (sqlret != SQLITE_OK) {
		ERR("DB err [%s]", sqlite3_errmsg(db));
		ERR("query[%s]", sqlbuf);
		result = BADGE_ERROR_FROM_DB;
		goto free_and_return;
	}

	sqlret = sqlite3_step(stmt);
	if (sqlret == SQLITE_ROW)
		count = sqlite3_column_int(stmt, 0);
	else
		count = 0;

	DBG("[%s], count[%d]", sqlbuf, count);

	if (count > 0)
		result = BADGE_ERROR_ALREADY_EXIST;
	else
		result = BADGE_ERROR_NOT_EXIST;

free_and_return:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (stmt)
		sqlite3_finalize(stmt);

	return result;
}

static badge_error_e _badge_check_writable(const char *caller,
			const char *pkgname, sqlite3 *db)
{
	sqlite3_stmt *stmt = NULL;
	int count = 0;
	badge_error_e result = BADGE_ERROR_NONE;
	char *sqlbuf = NULL;
	int sqlret;

	if (!db)
		return BADGE_ERROR_INVALID_DATA;

	if (!caller)
		return BADGE_ERROR_INVALID_DATA;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	if (g_strcmp0(caller, pkgname) == 0)
		return BADGE_ERROR_NONE;

	sqlbuf = sqlite3_mprintf("SELECT COUNT(*) FROM %s WHERE " \
			 "pkgname = %Q AND writable_pkgs LIKE '%%%q%%'",
			 BADGE_TABLE_NAME,
			 pkgname, caller);
	if (!sqlbuf) {
		ERR("fail to alloc sql query");
		return BADGE_ERROR_NO_MEMORY;
	}

	sqlret = sqlite3_prepare_v2(db, sqlbuf, -1, &stmt, NULL);
	if (sqlret != SQLITE_OK) {
		ERR("DB err [%s]", sqlite3_errmsg(db));
		ERR("query[%s]", sqlbuf);
		result = BADGE_ERROR_FROM_DB;
		goto free_and_return;
	}

	sqlret = sqlite3_step(stmt);
	if (sqlret == SQLITE_ROW)
		count = sqlite3_column_int(stmt, 0);
	else
		count = 0;

	DBG("[%s], count[%d]", sqlbuf, count);

	if (count <= 0)
		result = BADGE_ERROR_PERMISSION_DENIED;

free_and_return:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (stmt)
		sqlite3_finalize(stmt);

	return result;
}


badge_error_e _badge_is_existing(const char *pkgname, bool *existing)
{
	sqlite3 *db = NULL;
	int sqlret;
	badge_error_e ret = BADGE_ERROR_NONE;
	badge_error_e result = BADGE_ERROR_NONE;

	if (!pkgname || !existing) {
		ERR("pkgname : %s, existing : %p", pkgname, existing);
		return BADGE_ERROR_INVALID_DATA;
	}

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%d]", sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	result = _badge_check_data_inserted(pkgname, db);
	if (result == BADGE_ERROR_ALREADY_EXIST)
		*existing = TRUE;
	else if (result == BADGE_ERROR_NOT_EXIST)
		*existing = FALSE;
	else
		ret = result;

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return BADGE_ERROR_NONE;
}

badge_error_e _badge_foreach_existed(badge_cb callback, void *data)
{
	sqlite3 *db = NULL;
	badge_error_e result = BADGE_ERROR_NONE;
	char *sqlbuf = NULL;
	sqlite3_stmt *stmt = NULL;
	int sqlret;

	if (!callback)
		return BADGE_ERROR_INVALID_DATA;

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%d]", sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	sqlbuf = sqlite3_mprintf("SELECT pkgname, badge FROM %s",
				BADGE_TABLE_NAME);
	if (!sqlbuf) {
		ERR("fail to alloc sql query");
		result = BADGE_ERROR_NO_MEMORY;
		goto free_and_return;
	}

	sqlret = sqlite3_prepare_v2(db, sqlbuf, -1, &stmt, NULL);
	if (sqlret != SQLITE_OK) {
		ERR("fail to sqlite3_prepare_v2 - [%s]", sqlite3_errmsg(db));
		ERR("query[%s]", sqlbuf);
		result = BADGE_ERROR_FROM_DB;
		goto free_and_return;
	}

	sqlret = sqlite3_step(stmt);
	if (sqlret == SQLITE_DONE) {
		INFO("badge db has no data");
		result = BADGE_ERROR_NOT_EXIST;
		goto free_and_return;
	} else if (sqlret == SQLITE_ROW) {
		const char *pkg = NULL;
		unsigned int badge_count = 0;

		pkg = (const char *)sqlite3_column_text(stmt, 0);
		badge_count = (unsigned int)sqlite3_column_int(stmt, 1);

		if (pkg)
			callback(pkg, badge_count, data);
		else
			ERR("db has invaild data");
	} else {
		ERR("fail to sqlite3_step : %d", sqlret);
		result = BADGE_ERROR_FROM_DB;
		goto free_and_return;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const char *pkg = NULL;
		unsigned int badge_count = 0;

		pkg = (const char *)sqlite3_column_text(stmt, 0);
		badge_count = (unsigned int)sqlite3_column_int(stmt, 1);


		if (pkg)
			callback(pkg, badge_count, data);
		else
			ERR("db has invaild data");
	}

free_and_return:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (stmt)
		sqlite3_finalize(stmt);

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return result;
}

badge_error_e _badge_insert(badge_h *badge)
{
	sqlite3 *db = NULL;
	int sqlret;
	badge_error_e ret = BADGE_ERROR_NONE;
	badge_error_e result = BADGE_ERROR_NONE;
	char *sqlbuf = NULL;
	char *err_msg = NULL;

	if (!badge || !badge->pkgname || !badge->writable_pkgs)
		return BADGE_ERROR_INVALID_DATA;

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%s][%d]", BADGE_DB_PATH, sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	/* Check pkgname & id */
	ret = _badge_check_data_inserted(badge->pkgname, db);
	if (ret != BADGE_ERROR_NOT_EXIST) {
		result = ret;
		goto return_close_db;
	}

	sqlbuf = sqlite3_mprintf("INSERT INTO %s " \
			"(pkgname, " \
			"writable_pkgs) " \
			"VALUES "
			"(%Q, %Q);",
			 BADGE_TABLE_NAME,
			 badge->pkgname, badge->writable_pkgs);
	if (!sqlbuf) {
		ERR("fail to alloc query");
		result = BADGE_ERROR_NO_MEMORY;
		goto return_close_db;
	}

	sqlret = sqlite3_exec(db, sqlbuf, NULL, NULL, &err_msg);
	if (sqlret != SQLITE_OK) {
		ERR("fail to insert badge[%s], err[%d - %s]",
					badge->pkgname, sqlret, err_msg);
		result = BADGE_ERROR_FROM_DB;
		goto return_close_db;
	}

	_badge_changed(BADGE_ACTION_CREATE, badge->pkgname, 0);

	/* inserting badge options */
	ret = _badge_check_option_inserted(badge->pkgname, db);
	if (ret != BADGE_ERROR_NOT_EXIST) {
		result = ret;
		goto return_close_db;
	}

	sqlbuf = sqlite3_mprintf("INSERT INTO %s " \
			"(pkgname) " \
			"VALUES "
			"(%Q);",
			BADGE_OPTION_TABLE_NAME,
			 badge->pkgname);
	if (!sqlbuf) {
		ERR("fail to alloc query");
		result = BADGE_ERROR_NO_MEMORY;
		goto return_close_db;
	}

	sqlret = sqlite3_exec(db, sqlbuf, NULL, NULL, &err_msg);
	if (sqlret != SQLITE_OK) {
		ERR("fail to insert badge option[%s], err[%d - %s]",
					badge->pkgname, sqlret, err_msg);
		result = BADGE_ERROR_FROM_DB;
		goto return_close_db;
	}

return_close_db:
	if (err_msg)
		free(err_msg);

	if (sqlbuf)
		sqlite3_free(sqlbuf);

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return result;
}

badge_error_e _badge_remove(const char *caller, const char *pkgname)
{
	badge_error_e ret = BADGE_ERROR_NONE;
	badge_error_e result = BADGE_ERROR_NONE;
	sqlite3 *db = NULL;
	int sqlret;
	char *sqlbuf = NULL;
	char *err_msg = NULL;

	if (!caller)
		return BADGE_ERROR_INVALID_DATA;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%d]", sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	ret = _badge_check_data_inserted(pkgname, db);
	if (ret != BADGE_ERROR_ALREADY_EXIST) {
		result = ret;
		goto return_close_db;
	}

	ret = _badge_check_writable(caller, pkgname, db);
	if (ret != BADGE_ERROR_NONE) {
		result = ret;
		goto return_close_db;
	}

	sqlbuf = sqlite3_mprintf("DELETE FROM %s WHERE pkgname = %Q",
			 BADGE_TABLE_NAME, pkgname);
	if (!sqlbuf) {
		ERR("fail to alloc query");
		result = BADGE_ERROR_NO_MEMORY;
		goto return_close_db;
	}

	sqlret = sqlite3_exec(db, sqlbuf, NULL, NULL, &err_msg);
	if (sqlret != SQLITE_OK) {
		ERR("fail to remove badge[%s], err[%d - %s]",
					pkgname, sqlret, err_msg);
		result = BADGE_ERROR_FROM_DB;
		goto return_close_db;
	}

	_badge_changed(BADGE_ACTION_REMOVE, pkgname, 0);

	/* treating option table */
	ret = _badge_check_option_inserted(pkgname, db);
	if (ret != BADGE_ERROR_ALREADY_EXIST) {
		result = ret;
		goto return_close_db;
	}

	sqlbuf = sqlite3_mprintf("DELETE FROM %s WHERE pkgname = %Q",
			BADGE_OPTION_TABLE_NAME, pkgname);
	if (!sqlbuf) {
		ERR("fail to alloc query");
		result = BADGE_ERROR_NO_MEMORY;
		goto return_close_db;
	}

	sqlret = sqlite3_exec(db, sqlbuf, NULL, NULL, &err_msg);
	if (sqlret != SQLITE_OK) {
		ERR("fail to remove badge option[%s], err[%d - %s]",
					pkgname, sqlret, err_msg);
		result = BADGE_ERROR_FROM_DB;
		goto return_close_db;
	}

return_close_db:
	if (err_msg)
		free(err_msg);

	if (sqlbuf)
		sqlite3_free(sqlbuf);

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return result;
}

badge_error_e _badget_set_count(const char *caller, const char *pkgname,
			unsigned int count)
{
	badge_error_e ret = BADGE_ERROR_NONE;
	badge_error_e result = BADGE_ERROR_NONE;
	sqlite3 *db = NULL;
	char *sqlbuf = NULL;
	int sqlret;
	char *err_msg = NULL;

	if (!caller)
		return BADGE_ERROR_INVALID_DATA;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%d]", sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	ret = _badge_check_data_inserted(pkgname, db);
	if (ret != BADGE_ERROR_ALREADY_EXIST) {
		result = ret;
		goto return_close_db;
	}

	ret = _badge_check_writable(caller, pkgname, db);
	if (ret != BADGE_ERROR_NONE) {
		result = ret;
		goto return_close_db;
	}

	sqlbuf = sqlite3_mprintf("UPDATE %s SET badge = %d " \
			"WHERE pkgname = %Q",
			 BADGE_TABLE_NAME, count, pkgname);
	if (!sqlbuf) {
		ERR("fail to alloc query");
		result = BADGE_ERROR_NO_MEMORY;
		goto return_close_db;
	}

	sqlret = sqlite3_exec(db, sqlbuf, NULL, NULL, &err_msg);
	if (sqlret != SQLITE_OK) {
		ERR("fail to set badge[%s] count[%d], err[%d - %s]",
				pkgname, count, sqlret, err_msg);
		result = BADGE_ERROR_FROM_DB;
		goto return_close_db;
	}

	_badge_changed(BADGE_ACTION_UPDATE, pkgname, count);

return_close_db:
	if (err_msg)
		free(err_msg);

	if (sqlbuf)
		sqlite3_free(sqlbuf);

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return result;
}

badge_error_e _badget_get_count(const char *pkgname, unsigned int *count)
{
	badge_error_e ret = BADGE_ERROR_NONE;
	badge_error_e result = BADGE_ERROR_NONE;
	sqlite3 *db = NULL;
	char *sqlbuf = NULL;
	sqlite3_stmt *stmt = NULL;
	int sqlret;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	if (!count)
		return BADGE_ERROR_INVALID_DATA;

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%d]", sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	ret = _badge_check_data_inserted(pkgname, db);
	if (ret != BADGE_ERROR_ALREADY_EXIST) {
		result = ret;
		goto return_close_db;
	}

	sqlbuf = sqlite3_mprintf("SELECT badge FROM %s " \
			"WHERE pkgname = %Q",
			 BADGE_TABLE_NAME, pkgname);
	if (!sqlbuf) {
		ERR("fail to alloc query");
		result = BADGE_ERROR_NO_MEMORY;
		goto return_close_db;
	}

	sqlret = sqlite3_prepare_v2(db, sqlbuf, -1, &stmt, NULL);
	if (sqlret != SQLITE_OK) {
		ERR("fail to prepare %s - [%s]",
				sqlbuf, sqlite3_errmsg(db));
		result = BADGE_ERROR_FROM_DB;
		goto return_close_db;
	}

	sqlret = sqlite3_step(stmt);
	if (sqlret == SQLITE_ROW)
		*count = (unsigned int)sqlite3_column_int(stmt, 0);
	else
		*count = (unsigned int)0;

return_close_db:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (stmt)
		sqlite3_finalize(stmt);

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return result;
}

badge_error_e _badget_set_display(const char *pkgname,
			unsigned int is_display)
{
	badge_error_e ret = BADGE_ERROR_NONE;
	badge_error_e result = BADGE_ERROR_NONE;
	sqlite3 *db = NULL;
	char *sqlbuf = NULL;
	int sqlret;
	char *err_msg = NULL;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	if (is_display != 0 && is_display != 1)
		return BADGE_ERROR_INVALID_DATA;

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%d]", sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	ret = _badge_check_option_inserted(pkgname, db);
	if (ret == BADGE_ERROR_ALREADY_EXIST) {
		sqlbuf = sqlite3_mprintf("UPDATE %s SET display = %d " \
				"WHERE pkgname = %Q",
				BADGE_OPTION_TABLE_NAME, is_display, pkgname);
		if (!sqlbuf) {
			ERR("fail to alloc query");
			result = BADGE_ERROR_NO_MEMORY;
			goto return_close_db;
		}

		sqlret = sqlite3_exec(db, sqlbuf, NULL, NULL, &err_msg);
		if (sqlret != SQLITE_OK) {
			ERR("fail to set badge[%s] option[%d], err[%d - %s]",
					pkgname, is_display, sqlret, err_msg);
			result = BADGE_ERROR_FROM_DB;
			goto return_close_db;
		}
	} else if (ret == BADGE_ERROR_NOT_EXIST) {
		sqlbuf = sqlite3_mprintf("INSERT INTO %s " \
				"(pkgname, " \
				"display) " \
				"VALUES "
				"(%Q, %d);",
				BADGE_OPTION_TABLE_NAME,
				pkgname, is_display);
		if (!sqlbuf) {
			ERR("fail to alloc query");
			result = BADGE_ERROR_NO_MEMORY;
			goto return_close_db;
		}

		sqlret = sqlite3_exec(db, sqlbuf, NULL, NULL, &err_msg);
		if (sqlret != SQLITE_OK) {
			ERR("fail to set badge[%s] option[%d], err[%d - %s]",
					pkgname, is_display, sqlret, err_msg);
			result = BADGE_ERROR_FROM_DB;
			goto return_close_db;
		}
	} else {
		result = ret;
		goto return_close_db;
	}

	_badge_changed(BADGE_ACTION_CHANGED_DISPLAY, pkgname, is_display);

return_close_db:
	if (err_msg)
		free(err_msg);

	if (sqlbuf)
		sqlite3_free(sqlbuf);

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return result;
}

badge_error_e _badget_get_display(const char *pkgname, unsigned int *is_display)
{
	badge_error_e ret = BADGE_ERROR_NONE;
	badge_error_e result = BADGE_ERROR_NONE;
	sqlite3 *db = NULL;
	char *sqlbuf = NULL;
	sqlite3_stmt *stmt = NULL;
	int sqlret;

	if (!pkgname)
		return BADGE_ERROR_INVALID_DATA;

	if (!is_display)
		return BADGE_ERROR_INVALID_DATA;

	sqlret = db_util_open(BADGE_DB_PATH, &db, 0);
	if (sqlret != SQLITE_OK || !db) {
		ERR("fail to db_util_open - [%d]", sqlret);
		return BADGE_ERROR_FROM_DB;
	}

	ret = _badge_check_option_inserted(pkgname, db);
	if (ret != BADGE_ERROR_ALREADY_EXIST) {
		result = ret;
		goto return_close_db;
	}

	sqlbuf = sqlite3_mprintf("SELECT display FROM %s " \
			"WHERE pkgname = %Q",
			BADGE_OPTION_TABLE_NAME, pkgname);
	if (!sqlbuf) {
		ERR("fail to alloc query");
		result = BADGE_ERROR_NO_MEMORY;
		goto return_close_db;
	}

	sqlret = sqlite3_prepare_v2(db, sqlbuf, -1, &stmt, NULL);
	if (sqlret != SQLITE_OK) {
		ERR("fail to prepare %s - [%s]",
				sqlbuf, sqlite3_errmsg(db));
		result = BADGE_ERROR_FROM_DB;
		goto return_close_db;
	}

	sqlret = sqlite3_step(stmt);
	if (sqlret == SQLITE_ROW)
		*is_display = (unsigned int)sqlite3_column_int(stmt, 0);
	else
		*is_display = (unsigned int)1;

return_close_db:
	if (sqlbuf)
		sqlite3_free(sqlbuf);

	if (stmt)
		sqlite3_finalize(stmt);

	sqlret = db_util_close(db);
	if (sqlret != SQLITE_OK)
		WARN("fail to db_util_close - [%d]", sqlret);

	return result;
}

static void _badge_changed_cb_call(unsigned int action, const char *pkgname,
			unsigned int count)
{
	GList *list = g_badge_cb_list;

	while (list) {
		struct _badge_cb_data *bd = g_list_nth_data(list, 0);
		if (!bd)
			continue;

		if (bd->callback)
			bd->callback(action, pkgname, count, bd->data);

		list = g_list_next(list);
	}
}

static DBusHandlerResult _badge_signal_filter(DBusConnection *conn,
		DBusMessage *msg, void *user_data)
{
	const char *interface;
	DBusError error;
	dbus_bool_t ret;
	unsigned int action = 0;
	const char *pkgname = NULL;
	unsigned int count = 0;

	dbus_error_init(&error);

	interface = dbus_message_get_interface(msg);
	DBG("path : %s", dbus_message_get_path(msg));
	DBG("interface : %s", interface);

	if (g_strcmp0(BADGE_DBUS_INTERFACE, interface))
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	ret = dbus_message_is_signal(msg, interface, BADGE_CHANGED_NOTI);
	if (!ret) {
		DBG("this msg is not signal");
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	ret = dbus_message_get_args(msg, &error,
				DBUS_TYPE_UINT32, &action,
				DBUS_TYPE_STRING, &pkgname,
				DBUS_TYPE_UINT32, &count,
				DBUS_TYPE_INVALID);
	if (!ret) {
		ERR("fail to get args : %s", error.message);
		dbus_error_free(&error);
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	_badge_changed_cb_call(action, pkgname, count);

	return DBUS_HANDLER_RESULT_HANDLED;
}

static inline void __bus_rule_get(char *buf, int buf_len)
{
	if (!buf)
		return;
	if (buf_len <= 1)
		return;

	snprintf(buf, buf_len,
		"path='%s',type='signal',interface='%s',member='%s'",
		BADGE_DBUS_PATH,
		BADGE_DBUS_INTERFACE,
		BADGE_CHANGED_NOTI);
}

static void _badge_changed_monitor_init()
{
	DBusError err;
	DBusConnection *conn = NULL;
	char rule[1024] = {'\0', };

	if (g_badge_cb_handle)
		return;

	dbus_error_init(&err);
	conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
	if (!conn) {
		ERR("fail to get bus : %s", err.message);
		dbus_error_free(&err);
		return;
	}
	dbus_connection_setup_with_g_main(conn, NULL);

	__bus_rule_get(rule, sizeof(rule));

	dbus_bus_add_match(conn, rule, &err);
	if (dbus_error_is_set(&err)) {
		ERR("fail to dbus_bus_remove_match : %s",
				err.message);
		dbus_error_free(&err);
		dbus_connection_close(conn);
		return;
	}

	if (dbus_connection_add_filter(conn, _badge_signal_filter,
					NULL, NULL) == FALSE) {
		ERR("fail to dbus_connection_add_filter : %s",
				err.message);
		dbus_error_free(&err);
		dbus_connection_close(conn);
		return;
	}

	dbus_connection_set_exit_on_disconnect(conn, FALSE);

	g_badge_cb_handle = conn;

	return;
}

static void _badge_chanaged_monitor_fini()
{
	DBusConnection *conn = g_badge_cb_handle;
	char rule[1024] = {'\0', };
	DBusError err;

	if (!conn)
		return;

	dbus_error_init(&err);

	dbus_connection_remove_filter(conn, _badge_signal_filter, NULL);

	__bus_rule_get(rule, sizeof(rule));

	dbus_bus_remove_match(conn, rule, &err);
	if (dbus_error_is_set(&err)) {
		ERR("fail to dbus_bus_remove_match : %s",
				err.message);
		dbus_error_free(&err);
	}

	dbus_connection_close(conn);

	g_badge_cb_handle = NULL;
}

static gint _badge_data_compare(gconstpointer a, gconstpointer b)
{
	const struct _badge_cb_data *bd = NULL;

	if (!a)
		return -1;
	bd = a;

	if (bd->callback == b)
		return 0;

	return 1;
}

badge_error_e _badge_register_changed_cb(badge_change_cb callback, void *data)
{
	struct _badge_cb_data *bd = NULL;
	GList *found = NULL;

	if (!g_badge_cb_handle)
		_badge_changed_monitor_init();

	found = g_list_find_custom(g_badge_cb_list, (gconstpointer)callback,
			_badge_data_compare);

	if (found) {
		bd = g_list_nth_data(found, 0);
		bd->data = data;
	} else {

		bd = malloc(sizeof(struct _badge_cb_data));
		if (!bd)
			return BADGE_ERROR_NO_MEMORY;


		bd->callback = callback;
		bd->data = data;

		g_badge_cb_list = g_list_append(g_badge_cb_list, bd);
	}
	return BADGE_ERROR_NONE;
}

badge_error_e _badge_unregister_changed_cb(badge_change_cb callback)
{
	GList *found = NULL;

	found = g_list_find_custom(g_badge_cb_list, (gconstpointer)callback,
				_badge_data_compare);

	if (found) {
		struct _badge_cb_data *bd = g_list_nth_data(found, 0);
		g_badge_cb_list = g_list_delete_link(g_badge_cb_list, found);
		free(bd);
	}

	if (!g_badge_cb_list)
		_badge_chanaged_monitor_fini();

	return BADGE_ERROR_NONE;
}

badge_error_e _badge_free(badge_h *badge)
{
	if (!badge)
		return BADGE_ERROR_INVALID_DATA;

	if (badge->pkgname)
		free(badge->pkgname);

	if (badge->writable_pkgs)
		free(badge->writable_pkgs);

	free(badge);

	return BADGE_ERROR_NONE;
}

badge_h *_badge_new(const char *pkgname, const char *writable_pkgs,
		badge_error_e *err)
{
	badge_h *badge = NULL;

	if (!pkgname) {
		ERR("PKGNAME is NULL");
		if (err)
			*err = BADGE_ERROR_INVALID_DATA;
		return NULL;
	}

	if (!writable_pkgs) {
		ERR("writable_pkgs is NULL");
		if (err)
			*err = BADGE_ERROR_INVALID_DATA;
		return NULL;
	}


	badge = (badge_h *)malloc(sizeof(struct _badge_h));
	if (!badge) {
		ERR("fail to alloc handle");
		if (err)
			*err = BADGE_ERROR_NO_MEMORY;
		return NULL;
	}

	badge->pkgname = strdup(pkgname);
	badge->writable_pkgs = strdup(writable_pkgs);
	if (err)
		*err = BADGE_ERROR_NONE;

	return badge;
}

char *_badge_pkgs_new(badge_error_e *err, const char *pkg1, ...)
{
	char *caller_pkgname = NULL;
	char *s = NULL;
	char *result = NULL;
	char *ptr = NULL;
	gsize length;
	va_list args;


	if (err)
		*err = BADGE_ERROR_NONE;

	caller_pkgname = _badge_get_pkgname_by_pid();
	if (!caller_pkgname) {
		ERR("fail to get caller pkgname");
		if (err)
			*err = BADGE_ERROR_PERMISSION_DENIED;
		return NULL;
	}

	if (!pkg1) {
		WARN("pkg1 is NULL");
		return caller_pkgname;
	}

	length = strlen(pkg1);
	va_start(args, pkg1);
	s = va_arg(args, char *);
	while (s) {
		length += strlen(s) ;
		s = va_arg(args, char *);
	}
	va_end(args);

	if (length <= 0) {
		WARN("length is %d", length);
		return caller_pkgname;
	}

	result = g_new0(char, length + 1); /* 1 for null terminate */
	if (!result) {
		ERR("fail to alloc memory");
		if (err)
			*err = BADGE_ERROR_NO_MEMORY;
		free(caller_pkgname);
		return NULL;
	}

	ptr = result;
	ptr = g_stpcpy(ptr, pkg1);
	va_start(args, pkg1);
	s = va_arg(args, char *);
	while (s) {
		ptr = g_stpcpy(ptr, s);
		s = va_arg(args, char *);
	}
	va_end(args);

	if (g_strstr_len(result, -1, caller_pkgname) == NULL) {
		char *new_pkgs = NULL;
		new_pkgs = g_strdup_printf("%s%s", caller_pkgname, result);
		if (!new_pkgs) {
			ERR("fail to alloc memory");
			if (err)
				*err = BADGE_ERROR_NO_MEMORY;

			free(result);
			free(caller_pkgname);
			return NULL;
		}
		free(result);
		result = new_pkgs;
	}

	free(caller_pkgname);

	return result;
}

char *_badge_pkgs_new_valist(badge_error_e *err, const char *pkg1, va_list args)
{
	char *caller_pkgname = NULL;
	char *s = NULL;
	char *result = NULL;
	char *ptr = NULL;
	gsize length;
	va_list args2;

	if (err)
		*err = BADGE_ERROR_NONE;

	caller_pkgname = _badge_get_pkgname_by_pid();
	if (!caller_pkgname) {
		ERR("fail to get caller pkgname");
		if (err)
			*err = BADGE_ERROR_PERMISSION_DENIED;
		return NULL;
	}

	if (!pkg1) {
		WARN("pkg1 is NULL");
		return caller_pkgname;
	}

	G_VA_COPY(args2, args);

	length = strlen(pkg1);
	s = va_arg(args, char *);
	while (s) {
		length += strlen(s) ;
		s = va_arg(args, char *);
	}

	if (length <= 0) {
		WARN("length is %d", length);
		va_end(args2);
		return caller_pkgname;
	}

	result = g_new0(char, length + 1); /* 1 for null terminate */
	if (!result) {
		ERR("fail to alloc memory");
		if (err)
			*err = BADGE_ERROR_NO_MEMORY;
		free(caller_pkgname);
		va_end(args2);
		return NULL;
	}

	ptr = result;
	ptr = g_stpcpy(ptr, pkg1);
	s = va_arg(args2, char *);
	while (s) {
		ptr = g_stpcpy(ptr, s);
		s = va_arg(args2, char *);
	}
	va_end(args2);

	if (g_strstr_len(result, -1, caller_pkgname) == NULL) {
		char *new_pkgs = NULL;
		new_pkgs = g_strdup_printf("%s%s", caller_pkgname, result);
		if (!new_pkgs) {
			ERR("fail to alloc memory");
			if (err)
				*err = BADGE_ERROR_NO_MEMORY;

			free(result);
			free(caller_pkgname);
			return NULL;
		}
		free(result);
		result = new_pkgs;
	}

	free(caller_pkgname);

	return result;
}

