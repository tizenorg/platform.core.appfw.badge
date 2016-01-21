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
#include <openssl/md5.h>

#include <vconf.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"
#include "badge_ipc.h"

#define BADGE_SERVICE_BUS_NAME "org.tizen.badge_service"
#define BADGE_SERVICE_INTERFACE_NAME "org.tizen.badge_service"
#define BADGE_SERVICE_OBJECT_PATH "/org/tizen/badge_service"

#define PROVIDER_BUS_NAME "org.tizen.data_provider_service"
#define PROVIDER_OBJECT_PATH "/org/tizen/data_provider_service"
#define PROVIDER_BADGE_INTERFACE_NAME "org.tizen.data_provider_badge_service"

#define BADGE_IPC_DBUS_PREFIX "org.tizen.badge_ipc_"
#define BADGE_IPC_OBJECT_PATH "/org/tizen/badge_service"

#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"

#define BADGE_IPC_TIMEOUT 1.0

#if !defined(VCONFKEY_MASTER_STARTED)
#define VCONFKEY_MASTER_STARTED "memory/data-provider-master/started"
#endif

typedef struct _task_list task_list;
struct _task_list {
	task_list *prev;
	task_list *next;

	void (*task_cb) (void *data);
	void *data;
};
static task_list *g_task_list;

static char *_bus_name = NULL;
static GDBusConnection *_gdbus_conn = NULL;
static int monitor_id = 0;

static void _do_deferred_task(void);
static void _master_started_cb_task(keynode_t *node, void *data);
static int is_started_cb_set_task = 0;

/*!
 * functions to check state of master
 */
static inline void _set_master_started_cb(vconf_callback_fn cb)
{
	int ret = -1;

	ret = vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, cb, NULL);
	if (ret != 0)
		ERR("failed to notify key(%s) : %d", VCONFKEY_MASTER_STARTED, ret);
}

static inline void _unset_master_started_cb(vconf_callback_fn cb)
{
	int ret = -1;

	ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, cb);
	if (ret != 0)
		ERR("failed to notify key(%s) : %d", VCONFKEY_MASTER_STARTED, ret);
}

int badge_ipc_is_master_ready(void)
{
	int ret = -1, is_master_started = 0;

	ret = vconf_get_bool(VCONFKEY_MASTER_STARTED, &is_master_started);
	if (ret == 0 && is_master_started == 1) {
		ERR("the master has been started");
	} else {
		is_master_started = 0;
		ERR("the master has been stopped");
	}

	return is_master_started;
}

int badge_ipc_add_deferred_task(
		void (*badge_add_deferred_task)(void *data),
		void *user_data)
{
	task_list *list = NULL;
	task_list *list_new = NULL;

	list_new = (task_list *) malloc(sizeof(task_list));

	if (list_new == NULL)
		return BADGE_ERROR_OUT_OF_MEMORY;

	if (is_started_cb_set_task == 0) {
		_set_master_started_cb(_master_started_cb_task);
		is_started_cb_set_task = 1;
	}

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
	task_list *list_del = NULL;
	task_list *list_prev = NULL;
	task_list *list_next = NULL;

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

			if (g_task_list == NULL) {
				if (is_started_cb_set_task == 1) {
					_unset_master_started_cb(_master_started_cb_task);
					is_started_cb_set_task = 0;
				}
			}

			return BADGE_ERROR_NONE;
		}
		list_del = list_del->next;
	} while (list_del != NULL);

	return BADGE_ERROR_INVALID_PARAMETER;
}

static void _do_deferred_task(void)
{
	task_list *list_do = NULL;
	task_list *list_temp = NULL;

	if (g_task_list == NULL)
		return;

	list_do = g_task_list;
	g_task_list = NULL;
	if (is_started_cb_set_task == 1) {
		_unset_master_started_cb(_master_started_cb_task);
		is_started_cb_set_task = 0;
	}

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

static void _master_started_cb_task(keynode_t *node, void *data)
{
	if (badge_ipc_is_master_ready())
		_do_deferred_task();
}

/*
 * dbus handler implementation
 */
static void _insert_badge_notify(GVariant *parameters, GDBusMethodInvocation *invocation)
{
	int ret = 0;
	char *pkgname = NULL;

	g_variant_get(parameters, "(is)", &ret, &pkgname);
	if (ret == BADGE_ERROR_NONE)
		badge_changed_cb_call(BADGE_ACTION_CREATE, pkgname, 0);
	else
		ERR("failed to insert a new badge:%d", ret);
}

static void _delete_badge_notify(GVariant *parameters, GDBusMethodInvocation *invocation)
{
	int ret = 0;
	char *pkgname = NULL;

	g_variant_get(parameters, "(is)", &ret, &pkgname);
	if (ret == BADGE_ERROR_NONE)
		badge_changed_cb_call(BADGE_ACTION_REMOVE, pkgname, 0);
	else
		ERR("failed to remove a badge:%d", ret);
}

static void _set_badge_notify(GVariant *parameters, GDBusMethodInvocation *invocation)
{
	int ret = 0;
	char *pkgname = NULL;
	int count = 0;

	g_variant_get(parameters, "(isi)", &ret, &pkgname, &count);
	if (ret == BADGE_ERROR_NONE)
		badge_changed_cb_call(BADGE_ACTION_UPDATE, pkgname, count);
	else
		ERR("failed to remove a badge:%d", ret);
}

static void _set_disp_option_notify(GVariant *parameters, GDBusMethodInvocation *invocation)
{
	int ret = 0;
	char *pkgname = NULL;
	int is_display = 0;

	g_variant_get(parameters, "(isi)", &ret, &pkgname, &is_display);
	if (ret == BADGE_ERROR_NONE)
		badge_changed_cb_call(BADGE_ACTION_CHANGED_DISPLAY, pkgname, is_display);
	else
		ERR("failed to remove a badge:%d", ret);
}

static void _dbus_method_call_handler(GDBusConnection *conn,
		const gchar *sender, const gchar *object_path, const gchar *iface_name,
		const gchar *method_name, GVariant *parameters,
		GDBusMethodInvocation *invocation, gpointer user_data)
{
	DBG("method_name: %s", method_name);
	if (g_strcmp0(method_name, "insert_badge_notify") == 0)
		_insert_badge_notify(parameters, invocation);
	else if (g_strcmp0(method_name, "delete_badge_notify") == 0)
		_delete_badge_notify(parameters, invocation);
	else if (g_strcmp0(method_name, "set_badge_notify") == 0)
		_set_badge_notify(parameters, invocation);
	else if (g_strcmp0(method_name, "set_disp_option_notify") == 0)
		_set_disp_option_notify(parameters, invocation);
}

static char *_get_encoded_name(const char *appid)
{
	int prefix_len = strlen(BADGE_IPC_DBUS_PREFIX);

	unsigned char c[MD5_DIGEST_LENGTH] = { 0 };
	char *md5_interface = NULL;
	char *temp;
	int index = 0;
	MD5_CTX mdContext;
	int encoded_name_len = prefix_len + (MD5_DIGEST_LENGTH * 2) + 2;
	int appid_len = strlen(appid) + 1;

	MD5_Init(&mdContext);
	MD5_Update(&mdContext, appid, appid_len);
	MD5_Final(c, &mdContext);

	md5_interface = (char *) calloc(encoded_name_len, sizeof(char));
	if (md5_interface == NULL) {
		ERR("md5_interface calloc failed!!");
		return 0;
	}

	snprintf(md5_interface, encoded_name_len, "%s", BADGE_IPC_DBUS_PREFIX);
	temp = md5_interface;
	temp += prefix_len;

	for (index = 0; index < MD5_DIGEST_LENGTH; index++) {
		snprintf(temp, 3, "%02x", c[index]);
		temp += 2;
	}

	DBG("encoded_name : %s ", md5_interface);
	return md5_interface;
}

static const GDBusInterfaceVTable interface_vtable = {
		_dbus_method_call_handler,
		NULL,
		NULL };

static void _badge_on_bus_acquired(GDBusConnection *connection,
		const gchar *name, gpointer user_data) {
	ERR("_badge_on_bus_acquired : %s", name);
}

static void _badge_on_name_acquired(GDBusConnection *connection,
		const gchar *name, gpointer user_data) {
	ERR("_badge_on_name_acquired : %s", name);
}

static void _badge_on_name_lost(GDBusConnection *connection, const gchar *name,
		gpointer user_data) {
	ERR("_badge_on_name_lost : %s", name);
}

int _register_badge_dbus_interface(const char *appid) {
	GDBusNodeInfo *introspection_data = NULL;
	int registration_id = 0;
	static gchar introspection_prefix[] = "<node>"
			"  <interface name='";
	static gchar introspection_postfix[] =
			"'>"
			"        <method name='insert_badge_notify'>"
			"          <arg type='i' name='ret' direction='in'/>"
			"          <arg type='s' name='pkgname' direction='in'/>"
			"        </method>"
			"        <method name='delete_badge_notify'>"
			"          <arg type='i' name='ret' direction='in'/>"
			"          <arg type='s' name='pkgname' direction='in'/>"
			"        </method>"
			"        <method name='set_badge_notify'>"
			"          <arg type='i' name='ret' direction='in'/>"
			"          <arg type='s' name='pkgname' direction='in'/>"
			"          <arg type='i' name='count' direction='in'/>"
			"        </method>"
			"        <method name='set_disp_option_notify'>"
			"          <arg type='i' name='ret' direction='in'/>"
			"          <arg type='s' name='pkgname' direction='in'/>"
			"          <arg type='i' name='is_display' direction='in'/>"
			"        </method>"
			"        <method name='set_noti_property_notify'>"
			"          <arg type='i' name='ret' direction='in'/>"
			"          <arg type='s' name='pkgname' direction='in'/>"
			"          <arg type='i' name='is_display' direction='in'/>"
			"        </method>"
			"  </interface>"
			"</node>";
	char *introspection_xml = NULL;
	int introspection_xml_len = 0;
	int owner_id = 0;
	GVariant *result = NULL;
	char *interface_name = NULL;

	_bus_name = _get_encoded_name(appid);
	interface_name = _bus_name;

	introspection_xml_len = strlen(introspection_prefix)
			+ strlen(interface_name) + strlen(introspection_postfix) + 1;

	introspection_xml = (char *) calloc(introspection_xml_len, sizeof(char));
	if (!introspection_xml) {
		ERR("out of memory");
		goto out;
	}

	owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM, _bus_name,
			G_BUS_NAME_OWNER_FLAGS_NONE, _badge_on_bus_acquired,
			_badge_on_name_acquired, _badge_on_name_lost,
			NULL, NULL);
	if (!owner_id) {
		ERR("g_bus_own_name error");
		g_dbus_node_info_unref(introspection_data);
		goto out;
	}

	DBG("Acquiring the own name : %d", owner_id);

	snprintf(introspection_xml, introspection_xml_len, "%s%s%s",
			introspection_prefix, interface_name, introspection_postfix);

	introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	if (!introspection_data) {
		ERR("g_dbus_node_info_new_for_xml() is failed.");
		goto out;
	}

	registration_id = g_dbus_connection_register_object(
			_gdbus_conn,
			BADGE_SERVICE_OBJECT_PATH,
			introspection_data->interfaces[0],
			&interface_vtable, NULL, NULL, NULL);

	DBG("registration_id %d", registration_id);
	if (registration_id == 0) {
		ERR("Failed to g_dbus_connection_register_object");
		goto out;
	}
	out: if (introspection_data)
		g_dbus_node_info_unref(introspection_data);
	if (introspection_xml)
		free(introspection_xml);
	if (result)
		g_variant_unref(result);

	return registration_id;
}

int _dbus_init(void) {
	bool ret = false;
	GError *error = NULL;

	_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (_gdbus_conn == NULL) {
		if (error != NULL) {
			ERR("Failed to get dbus [%s]", error->message);
			g_error_free(error);
		}
		goto out;
	}
	ret = true;
	out: if (!_gdbus_conn)
		g_object_unref(_gdbus_conn);

	return ret;

}

int badge_dbus_init() {
	int ret = BADGE_ERROR_NONE;
	int id = 0;
	if (_gdbus_conn == NULL) {
		_dbus_init();
		id = _register_badge_dbus_interface(_badge_get_pkgname_by_pid());
		if (id < 1) {
			ret = BADGE_ERROR_IO_ERROR;
			ERR("Failed to _register_badge_dbus_interface");
		} else {
			monitor_id = id;
		}
	}
	return ret;
}


/*
 * implement user request
 */
int _send_sync_noti(GVariant *body, GDBusMessage **reply, char *cmd) {
	GError *err = NULL;
	GDBusMessage *msg = NULL;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_BADGE_INTERFACE_NAME,
			cmd);
	if (!msg) {
		ERR("Can't allocate new method call");
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

	if (!*reply) {
		if (err != NULL) {
			ERR("No reply. error = %s", err->message);
			g_error_free(err);
		}
		//		if (notification_ipc_is_master_ready() == 1)
		//			return NOTIFICATION_ERROR_PERMISSION_DENIED;
		//		else
		return BADGE_ERROR_SERVICE_NOT_READY;
	}
	ERR("_send_sync_noti done !!");
	return BADGE_ERROR_NONE;

}

int _send_service_register()
{
	GVariant *body = NULL;
	GDBusMessage *reply = NULL;
	int result = BADGE_ERROR_NONE;

	body = g_variant_new("(s)", _bus_name);
	result = _send_sync_noti(body, &reply, "service_register");
	if (result == BADGE_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &result);
	}

	ERR("_send_service_register done = %s", _bus_name);
	return result;
}

int badge_ipc_monitor_init(void)
{
	DBG("register a service\n");
	int ret = BADGE_ERROR_NONE;
	ret = badge_dbus_init();
	if (ret != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", ret);
		return ret;
	}
	ret = _send_service_register();
	return ret;
}

int badge_ipc_monitor_fini(void)
{
	g_bus_unown_name(monitor_id);
	return BADGE_ERROR_NONE;
}

int badge_ipc_request_insert(const char *pkgname, const char *writable_pkg, const char *caller)
{
	int result = BADGE_ERROR_NONE;
	GDBusMessage *reply = NULL;
	GVariant *body = NULL;

	result = badge_dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(sss)", pkgname, writable_pkg, caller);

	result = _send_sync_noti(body, &reply, "insert_badge");
	if (result == BADGE_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &result);
	}
	DBG("badge_ipc_request_insert done [result: %d]", result);
	return result;
}

int badge_ipc_request_delete(const char *pkgname, const char *caller)
{
	int result = BADGE_ERROR_NONE;
	GDBusMessage *reply = NULL;
	GVariant *body = NULL;

	result = badge_dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ss)", pkgname, caller);

	result = _send_sync_noti(body, &reply, "delete_badge");
	if (result == BADGE_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &result);
	}
	DBG("badge_ipc_request_insert done [result: %d]", result);
	return result;
}

int badge_ipc_request_set_count(const char *pkgname, const char *caller, int count)
{
	int result = BADGE_ERROR_NONE;
	GDBusMessage *reply = NULL;
	GVariant *body = NULL;

	result = badge_dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ssi)", pkgname, caller, count);

	result = _send_sync_noti(body, &reply, "set_badge_count");
	if (result == BADGE_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &result);
	}
	DBG("badge_ipc_request_set_count done [result: %d]", result);
	return result;
}

int badge_ipc_request_set_display(const char *pkgname, const char *caller, int display_option)
{
	int result = BADGE_ERROR_NONE;
	GDBusMessage *reply = NULL;
	GVariant *body = NULL;

	result = badge_dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ssi)", pkgname, caller, display_option);

	result = _send_sync_noti(body, &reply, "set_disp_option");
	if (result == BADGE_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &result);
	}
	DBG("badge_ipc_request_set_display done [result: %d]", result);
	return result;
}

int badge_ipc_setting_property_set(const char *pkgname, const char *property, const char *value)
{
	int result = BADGE_ERROR_NONE;
	GDBusMessage *reply = NULL;
	GVariant *body = NULL;

	result = badge_dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(sss)", pkgname, property, value);

	result = _send_sync_noti(body, &reply, "set_noti_property");
	if (result == BADGE_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &result);
	}
	DBG("badge_ipc_setting_property_set done [result: %d]", result);
	return result;
}

int badge_ipc_setting_property_get(const char *pkgname, const char *property, char **value)
{
	int result = BADGE_ERROR_NONE;
	GDBusMessage *reply = NULL;
	GVariant *body = NULL;
	char *ret_val = NULL;

	result = badge_dbus_init();
	if (result != BADGE_ERROR_NONE) {
		ERR("Can't init dbus %d", result);
		return result;
	}
	body = g_variant_new("(ss)", pkgname, property);

	result = _send_sync_noti(body, &reply, "get_noti_property");
	if (result == BADGE_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(is)", &result, ret_val);
		if (result == BADGE_ERROR_NONE && ret_val != NULL)
			*value = strdup(ret_val);
	}
	DBG("badge_ipc_setting_property_get done [result: %d]", result);
	return result;
}


