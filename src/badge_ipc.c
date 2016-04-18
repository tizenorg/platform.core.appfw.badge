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
#include <gio/gio.h>

#include <vconf.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"
#include "badge_ipc.h"


#define PROVIDER_BUS_NAME "org.tizen.data_provider_service"
#define PROVIDER_OBJECT_PATH "/org/tizen/data_provider_service"
#define PROVIDER_BADGE_INTERFACE_NAME "org.tizen.data_provider_badge_service"


#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"


typedef struct _task_list task_list;
struct _task_list {
	task_list *prev;
	task_list *next;

	void (*task_cb) (void *data);
	void *data;
};
static task_list *g_task_list;

static GDBusConnection *_gdbus_conn = NULL;
static int monitor_id = 0;
static int provider_monitor_id = 0;
static int is_master_started = 0;

static void _do_deferred_task(void);

int badge_ipc_is_master_ready(void)
{
	GVariant *result = NULL;
	GError *err = NULL;
	gboolean name_exist;

	result = g_dbus_connection_call_sync(
			_gdbus_conn,
			DBUS_SERVICE_DBUS,
			DBUS_PATH_DBUS,
			DBUS_INTERFACE_DBUS,
			"NameHasOwner",
			g_variant_new("(s)", PROVIDER_BUS_NAME),
			G_VARIANT_TYPE("(b)"),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&err);

	if (err || (result == NULL)) {
		if (err) {
			ERR("No reply. error = %s", err->message);
			g_error_free(err);
		}
		is_master_started = 0;
	} else {
		g_variant_get(result, "(b)", &name_exist);

		if (!name_exist) {
			ERR("Name not exist %s", PROVIDER_BUS_NAME);
			ERR("the master has been stopped");
			is_master_started = 0;
		} else {
			DBG("the master has been started");
			is_master_started = 1;
		}
	}

	if (result)
		g_variant_unref(result);

	return is_master_started;
}

int badge_ipc_add_deferred_task(
		void (*badge_add_deferred_task)(void *data),
		void *user_data)
{
	task_list *list;
	task_list *list_new;

	list_new = (task_list *) malloc(sizeof(task_list));

	if (list_new == NULL)
		return BADGE_ERROR_OUT_OF_MEMORY;

	list_new->next = NULL;
	list_new->prev = NULL;

	list_new->task_cb = badge_add_deferred_task;
	list_new->data = user_data;

	if (g_task_list == NULL) {
		g_task_list = list_new;
	} else {
		list = g_task_list;

		while (list->next != NULL)
			list = list->next;

		list->next = list_new;
		list_new->prev = list;
	}
	return BADGE_ERROR_NONE;
}

int badge_ipc_del_deferred_task(
		void (*badge_add_deferred_task)(void *data))
{
	task_list *list_del;
	task_list *list_prev;
	task_list *list_next;

	list_del = g_task_list;

	if (list_del == NULL)
		return BADGE_ERROR_INVALID_PARAMETER;

	while (list_del->prev != NULL)
		list_del = list_del->prev;

	do {
		if (list_del->task_cb == badge_add_deferred_task) {
			list_prev = list_del->prev;
			list_next = list_del->next;

			if (list_prev == NULL)
				g_task_list = list_next;
			else
				list_prev->next = list_next;

			if (list_next == NULL) {
				if (list_prev != NULL)
					list_prev->next = NULL;

			} else {
				list_next->prev = list_prev;
			}

			free(list_del);
			return BADGE_ERROR_NONE;
		}
		list_del = list_del->next;
	} while (list_del != NULL);

	return BADGE_ERROR_INVALID_PARAMETER;
}

static void _do_deferred_task(void)
{
	task_list *list_do;
	task_list *list_temp;

	if (g_task_list == NULL)
		return;

	list_do = g_task_list;
	g_task_list = NULL;

	while (list_do->prev != NULL)
		list_do = list_do->prev;

	while (list_do != NULL) {
		if (list_do->task_cb != NULL) {
			list_do->task_cb(list_do->data);
			DBG("called:%p", list_do->task_cb);
		}
		list_temp = list_do->next;
		free(list_do);
		list_do = list_temp;
	}
}

/*
 * dbus handler implementation
 */
static void _insert_badge_notify(GVariant *parameters)
{
	char *pkgname = NULL;

	g_variant_get(parameters, "(&s)", &pkgname);
	badge_changed_cb_call(BADGE_ACTION_CREATE, pkgname, 0);
}

static void _delete_badge_notify(GVariant *parameters)
{
	char *pkgname = NULL;

	g_variant_get(parameters, "(&s)", &pkgname);
	badge_changed_cb_call(BADGE_ACTION_REMOVE, pkgname, 0);
}

static void _set_badge_count_notify(GVariant *parameters)
{
	char *pkgname = NULL;
	int count = 0;

	g_variant_get(parameters, "(&si)", &pkgname, &count);
	badge_changed_cb_call(BADGE_ACTION_UPDATE, pkgname, count);
}

static void _set_disp_option_notify(GVariant *parameters)
{
	char *pkgname = NULL;
	int is_display = 0;

	g_variant_get(parameters, "(&si)", &pkgname, &is_display);
	badge_changed_cb_call(BADGE_ACTION_CHANGED_DISPLAY, pkgname, is_display);
}

static void _handle_badge_notify(GDBusConnection *connection,
		const gchar     *sender_name,
		const gchar     *object_path,
		const gchar     *interface_name,
		const gchar     *signal_name,
		GVariant        *parameters,
		gpointer         user_data)
{
	DBG("signal_name: %s", signal_name);
	if (g_strcmp0(signal_name, "insert_badge_notify") == 0)
		_insert_badge_notify(parameters);
	else if (g_strcmp0(signal_name, "delete_badge_notify") == 0)
		_delete_badge_notify(parameters);
	else if (g_strcmp0(signal_name, "set_badge_count_notify") == 0)
		_set_badge_count_notify(parameters);
	else if (g_strcmp0(signal_name, "set_disp_option_notify") == 0)
		_set_disp_option_notify(parameters);
}

static int _dbus_init(void)
{
	GError *error = NULL;

	if (_gdbus_conn == NULL) {
		_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if (_gdbus_conn == NULL) {
			if (error != NULL) {
				ERR("Failed to get dbus [%s]", error->message);
				g_error_free(error);
			}
			return BADGE_ERROR_IO_ERROR;
		}
		badge_error_quark();
	}

	return BADGE_ERROR_NONE;
}

static int _dbus_signal_init()
{
	int ret = BADGE_ERROR_NONE;
	int id;

	if (monitor_id == 0) {
		DBG("get dbus connection success");
		id = g_dbus_connection_signal_subscribe(_gdbus_conn,
				PROVIDER_BUS_NAME,
				PROVIDER_BADGE_INTERFACE_NAME,	/*   interface */
				NULL,				/*   member */
				PROVIDER_OBJECT_PATH,		/*   path */
				NULL,				/*   arg0 */
				G_DBUS_SIGNAL_FLAGS_NONE,
				_handle_badge_notify,
				NULL,
				NULL);

		DBG("subscribe id : %d", id);
		if (id == 0) {
			ret = BADGE_ERROR_IO_ERROR;
			ERR("Failed to _register_noti_dbus_interface");
		} else {
			monitor_id = id;
		}
	}

	return ret;
}

/*
 * implement user request
 */
static int _send_sync_badge(GVariant *body, GDBusMessage **reply, char *cmd)
{
	GError *err = NULL;
	GDBusMessage *msg;
	int ret = BADGE_ERROR_NONE;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_BADGE_INTERFACE_NAME,
			cmd);
	if (!msg) {
		ERR("Can't allocate new method call");
		if (body)
			g_variant_unref(body);
		return BADGE_ERROR_OUT_OF_MEMORY;
	}

	if (body != NULL)
		g_dbus_message_set_body(msg, body);

	*reply = g_dbus_connection_send_message_with_reply_sync(
			_gdbus_conn,
			msg,
			G_DBUS_SEND_MESSAGE_FLAGS_NONE,
			-1,
			NULL,
			NULL,
			&err);

	g_object_unref(msg);

	if (!*reply) {
		ret = BADGE_ERROR_SERVICE_NOT_READY;
		if (err != NULL) {
			ERR("No reply. cmd = %s,  error = %s", cmd, err->message);
			if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
				ret = BADGE_ERROR_PERMISSION_DENIED;
			g_error_free(err);
		}
		return ret;
	}

	if (g_dbus_message_to_gerror(*reply, &err)) {
		ret = err->code;
		ERR("_send_sync_badge cmd = %s, error %s", cmd, err->message);
		if (err->code == G_DBUS_ERROR_ACCESS_DENIED)
			ret = BADGE_ERROR_PERMISSION_DENIED;
		g_error_free(err);
		return ret;
	}
	ERR("_send_sync_badge done !!");
	return BADGE_ERROR_NONE;

}

static int _send_service_register()
{
	GDBusMessage *reply = NULL;
	int result;

	result = _send_sync_badge(NULL, &reply, "badge_service_register");

	if (reply)
		g_object_unref(reply);

	badge_changed_cb_call(BADGE_ACTION_SERVICE_READY, NULL, 0);
	DBG("_send_service_register dones");
	return result;
}

static int _ipc_monitor_register(void)
{
	DBG("register a service\n");

	return  _send_service_register();
}

static void _on_name_appeared(GDBusConnection *connection,
		const gchar     *name,
		const gchar     *name_owner,
		gpointer         user_data)
{
	DBG("name appeared : %s", name);
	is_master_started = 1;
	_ipc_monitor_register();

	_do_deferred_task();
}

static void _on_name_vanished(GDBusConnection *connection,
		const gchar     *name,
		gpointer         user_data)
{
	DBG("name vanished : %s", name);
	is_master_started = 0;
}

int badge_ipc_monitor_init(void)
{
	DBG("register a service\n");
	int ret = BADGE_ERROR_NONE;

	ret = _dbus_init();
	if (ret != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", ret);
		return ret;
	}

	ret = _dbus_signal_init();
	if (ret != BADGE_ERROR_NONE) {
		ERR("Can't init dbus signal %d", ret);
		return ret;
	}

	ret = _ipc_monitor_register();
	if (ret != BADGE_ERROR_NONE) {
		ERR("Can't init ipc_monitor_register %d", ret);
		return ret;
	}

	if (provider_monitor_id == 0) {
		provider_monitor_id = g_bus_watch_name_on_connection(
				_gdbus_conn,
				PROVIDER_BUS_NAME,
				G_BUS_NAME_WATCHER_FLAGS_NONE,
				_on_name_appeared,
				_on_name_vanished,
				NULL,
				NULL);

		if (provider_monitor_id == 0) {
			ERR("watch on name fail");
			g_dbus_connection_signal_unsubscribe(_gdbus_conn, monitor_id);
			monitor_id = 0;
			return BADGE_ERROR_IO_ERROR;
		}
	}

	return ret;
}

int badge_ipc_monitor_fini(void)
{
	if (provider_monitor_id) {
		g_bus_unwatch_name(provider_monitor_id);
		provider_monitor_id = 0;
	}

	if (monitor_id) {
		g_dbus_connection_signal_unsubscribe(_gdbus_conn, monitor_id);
		monitor_id = 0;
	}
	return BADGE_ERROR_NONE;
}

int badge_ipc_request_insert(const char *pkgname, const char *writable_pkg, const char *caller)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}

	body = g_variant_new("(sss)", pkgname, writable_pkg, caller);
	if (!body) {
		ERR("Cannot create gvariant. Out of memory.");
		return BADGE_ERROR_OUT_OF_MEMORY;
	}

	result = _send_sync_badge(body, &reply, "insert_badge");

	if (reply)
		g_object_unref(reply);

	DBG("badge_ipc_request_insert done [result: %d]", result);
	return result;
}

int badge_ipc_request_delete(const char *pkgname, const char *caller)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ss)", pkgname, caller);

	result = _send_sync_badge(body, &reply, "delete_badge");

	if (reply)
		g_object_unref(reply);

	DBG("badge_ipc_request_delete done [result: %d]", result);
	return result;
}

int badge_ipc_request_set_count(const char *pkgname, const char *caller, int count)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ssi)", pkgname, caller, count);

	result = _send_sync_badge(body, &reply, "set_badge_count");

	if (reply)
	    g_object_unref(reply);

	DBG("badge_ipc_request_set_count done [result: %d]", result);
	return result;
}

int badge_ipc_request_get_count(const char *pkgname, unsigned int *count)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	int ret_count;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(s)", pkgname);

	result = _send_sync_badge(body, &reply, "get_badge_count");
	if (result == BADGE_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &ret_count);
		*count = ret_count;
	}

	if (reply)
		g_object_unref(reply);

	DBG("badge_ipc_request_get_count done [result: %d]", result);
	return result;
}

int badge_ipc_request_set_display(const char *pkgname, const char *caller, unsigned int display_option)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ssi)", pkgname, caller, display_option);

	result = _send_sync_badge(body, &reply, "set_disp_option");
	if (reply)
		g_object_unref(reply);

	DBG("badge_ipc_request_set_display done [result: %d]", result);
	return result;
}

int badge_ipc_request_get_display(const char *pkgname, unsigned int *is_display)
{
	int result ;
	GDBusMessage *reply = NULL;
	GVariant *body ;
	GVariant *reply_body;
	unsigned int ret_is_display;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(s)", pkgname);

	result = _send_sync_badge(body, &reply, "get_disp_option");
	if (result == BADGE_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(i)", &ret_is_display);
		*is_display = ret_is_display;
	}

	if (reply)
		g_object_unref(reply);

	DBG("badge_ipc_request_get_display done [result: %d]", result);
	return result;
}

int badge_ipc_setting_property_set(const char *pkgname, const char *property, const char *value)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(sss)", pkgname, property, value);

	result = _send_sync_badge(body, &reply, "set_noti_property");
	if (reply)
		g_object_unref(reply);

	DBG("badge_ipc_setting_property_set done [result: %d]", result);
	return result;
}

int badge_ipc_setting_property_get(const char *pkgname, const char *property, char **value)
{
	int result;
	GDBusMessage *reply = NULL;
	GVariant *body;
	GVariant *reply_body;
	char *ret_val = NULL;

	result = _dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ss)", pkgname, property);

	result = _send_sync_badge(body, &reply, "get_noti_property");
	if (result == BADGE_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(&s)", ret_val);

		if (ret_val != NULL)
			*value = strdup(ret_val);
		else
			result = BADGE_ERROR_IO_ERROR;
	}
	if (reply)
		g_object_unref(reply);

	DBG("badge_ipc_setting_property_get done [result: %d]", result);
	return result;
}


