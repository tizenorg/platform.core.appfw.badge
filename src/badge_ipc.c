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

#include <vconf.h>

#include <packet.h>
#include <com-core.h>
#include <com-core_packet.h>

#include "badge.h"
#include "badge_log.h"
#include "badge_error.h"
#include "badge_internal.h"
#include "badge_ipc.h"

#define BADGE_IPC_TIMEOUT 1.0

#if !defined(VCONFKEY_MASTER_STARTED)
#define VCONFKEY_MASTER_STARTED "memory/data-provider-master/started"
#endif

static struct info {
	int server_fd;
	int client_fd;
	const char *socket_file;
	struct {
		int (*request_cb)(const char *appid, const char *name, int type, const char *content, const char *icon, pid_t pid, double period, int allow_duplicate, void *data);
		void *data;
	} server_cb;
	int initialized;
	int is_started_cb_set_svc;
	int is_started_cb_set_task;
} s_info = {
	.server_fd = -1,
	.client_fd = -1,
	.socket_file = BADGE_ADDR,
	.initialized = 0,
	.is_started_cb_set_svc = 0,
	.is_started_cb_set_task = 0,
};

typedef struct _task_list task_list;
struct _task_list {
	task_list *prev;
	task_list *next;

	void (*task_cb) (void *data);
	void *data;
};

static task_list *g_task_list;

static badge_error_e badge_ipc_monitor_register(void);
static badge_error_e badge_ipc_monitor_deregister(void);
static void _do_deffered_task(void);
static void _master_started_cb_task(keynode_t *node, void *data);

/*!
 * functions to check state of master
 */
static inline void _set_master_started_cb(vconf_callback_fn cb) {
	int ret = -1;

	ret = vconf_notify_key_changed(VCONFKEY_MASTER_STARTED,
			cb, NULL);
	if (ret != 0) {
		ERR("failed to notify key(%s) : %d",
				VCONFKEY_MASTER_STARTED, ret);
	}
}

static inline void _unset_master_started_cb(vconf_callback_fn cb) {
	int ret = -1;

	ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED,
			cb);
	if (ret != 0) {
		ERR("failed to notify key(%s) : %d",
				VCONFKEY_MASTER_STARTED, ret);
	}
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

badge_error_e
badge_ipc_add_deffered_task(
		void (*deffered_task_cb)(void *data),
		void *user_data)
{
	task_list *list = NULL;
	task_list *list_new = NULL;

	list_new =
	    (task_list *) malloc(sizeof(task_list));

	if (list_new == NULL) {
		return BADGE_ERROR_NO_MEMORY;
	}

	if (s_info.is_started_cb_set_task == 0) {
		_set_master_started_cb(_master_started_cb_task);
		s_info.is_started_cb_set_task = 1;
	}

	list_new->next = NULL;
	list_new->prev = NULL;

	list_new->task_cb = deffered_task_cb;
	list_new->data = user_data;

	if (g_task_list == NULL) {
		g_task_list = list_new;
	} else {
		list = g_task_list;

		while (list->next != NULL) {
			list = list->next;
		}

		list->next = list_new;
		list_new->prev = list;
	}
	return BADGE_ERROR_NONE;
}

badge_error_e
badge_ipc_del_deffered_task(
		void (*deffered_task_cb)(void *data))
{
	task_list *list_del = NULL;
	task_list *list_prev = NULL;
	task_list *list_next = NULL;

	list_del = g_task_list;

	if (list_del == NULL) {
		return BADGE_ERROR_INVALID_DATA;
	}

	while (list_del->prev != NULL) {
		list_del = list_del->prev;
	}

	do {
		if (list_del->task_cb == deffered_task_cb) {
			list_prev = list_del->prev;
			list_next = list_del->next;

			if (list_prev == NULL) {
				g_task_list = list_next;
			} else {
				list_prev->next = list_next;
			}

			if (list_next == NULL) {
				if (list_prev != NULL) {
					list_prev->next = NULL;
				}
			} else {
				list_next->prev = list_prev;
			}

			free(list_del);

			if (g_task_list == NULL) {
				if (s_info.is_started_cb_set_task == 1) {
					_unset_master_started_cb(_master_started_cb_task);
					s_info.is_started_cb_set_task = 0;
				}
			}

			return BADGE_ERROR_NONE;
		}
		list_del = list_del->next;
	} while (list_del != NULL);

	return BADGE_ERROR_INVALID_DATA;
}

static void _do_deffered_task(void) {
	task_list *list_do = NULL;
	task_list *list_temp = NULL;

	if (g_task_list == NULL) {
		return;
	}

	list_do = g_task_list;
	g_task_list = NULL;
	if (s_info.is_started_cb_set_task == 1) {
		_unset_master_started_cb(_master_started_cb_task);
		s_info.is_started_cb_set_task = 0;
	}

	while (list_do->prev != NULL) {
		list_do = list_do->prev;
	}

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

static void _master_started_cb_service(keynode_t *node,
		void *data) {
	int ret = BADGE_ERROR_NONE;

	if (badge_ipc_is_master_ready()) {
		ERR("try to register a badge service");
		ret = badge_ipc_monitor_deregister();
		if (ret != BADGE_ERROR_NONE) {
			ERR("failed to deregister a monitor");
		}
		ret = badge_ipc_monitor_register();
		if (ret != BADGE_ERROR_NONE) {
			ERR("failed to register a monitor");
		}
	} else {
		ERR("try to unregister a badge service");
		ret = badge_ipc_monitor_deregister();
		if (ret != BADGE_ERROR_NONE) {
			ERR("failed to deregister a monitor");
		}
	}
}

static void _master_started_cb_task(keynode_t *node,
		void *data) {

	if (badge_ipc_is_master_ready()) {
		_do_deffered_task();
	}
}

/*!
 * functions to handler services
 */
static struct packet *_handler_insert_badge(pid_t pid, int handle, const struct packet *packet)
{
	int ret = 0;
	char *pkgname = NULL;

	if (!packet) {
		ERR("a packet is null");
		return NULL;
	}

	DBG("");

	//return code, pkgname
	if (packet_get(packet, "is", &ret, &pkgname) == 2) {
		if (ret == BADGE_ERROR_NONE) {
			badge_changed_cb_call(BADGE_ACTION_CREATE, pkgname, 0);
		} else {
			ERR("failed to insert a new badge:%d", ret);
		}
	} else {
		ERR("failed to get data from a packet");
	}

	return NULL;
}

static struct packet *_handler_delete_badge(pid_t pid, int handle, const struct packet *packet)
{
	int ret = 0;
	char *pkgname = NULL;

	if (!packet) {
		ERR("a packet is null");
		return NULL;
	}

	DBG("");

	if (packet_get(packet, "is", &ret, &pkgname) == 2) {
		if (ret == BADGE_ERROR_NONE) {
			badge_changed_cb_call(BADGE_ACTION_REMOVE, pkgname, 0);
		} else {
			ERR("failed to remove a badge:%d", ret);
		}
	} else {
		ERR("failed to get data from a packet");
	}

	return NULL;
}

static struct packet *_handler_set_badge_count(pid_t pid, int handle, const struct packet *packet)
{
	int ret = 0;
	char *pkgname = NULL;
	int count = 0;

	if (!packet) {
		ERR("a packet is null");
		return NULL;
	}

	DBG("");

	if (packet_get(packet, "isi", &ret, &pkgname, &count) == 3) {
		if (ret == BADGE_ERROR_NONE) {
			badge_changed_cb_call(BADGE_ACTION_UPDATE, pkgname, count);
		} else {
			ERR("failed to update count of badge:%d", ret);
		}
	} else {
		ERR("failed to get data from a packet");
	}

	return NULL;
}

static struct packet *_handler_set_display_option(pid_t pid, int handle, const struct packet *packet)
{
	int ret = 0;
	char *pkgname = NULL;
	int is_display = 0;

	if (!packet) {
		ERR("a packet is null");
		return NULL;
	}

	DBG("");

	if (packet_get(packet, "isi", &ret, &pkgname, &is_display) == 3) {
		if (ret == BADGE_ERROR_NONE) {
			badge_changed_cb_call(BADGE_ACTION_CHANGED_DISPLAY, pkgname, is_display);
		} else {
			ERR("failed to update the display option of badge:%d, %d", ret, is_display);
		}
	} else {
		ERR("failed to get data from a packet");
	}

	return NULL;
}

static int _handler_service_register(pid_t pid, int handle, const struct packet *packet, void *data)
{
	int ret;

	DBG("");

	if (!packet) {
		ERR("Packet is not valid\n");
		ret = BADGE_ERROR_INVALID_DATA;
	} else if (packet_get(packet, "i", &ret) != 1) {
		ERR("Packet is not valid\n");
		ret = BADGE_ERROR_INVALID_DATA;
	} else {
		if (ret == BADGE_ERROR_NONE) {
			badge_changed_cb_call(BADGE_ACTION_SERVICE_READY, NULL, 0);
		}
	}
	return ret;
}

/*!
 * functions to initialize and register a monitor
 */
static badge_error_e badge_ipc_monitor_register(void)
{
	int ret;
	struct packet *packet;
	static struct method service_table[] = {
		{
			.cmd = "insert_badge",
			.handler = _handler_insert_badge,
		},
		{
			.cmd = "delete_badge",
			.handler = _handler_delete_badge,
		},
		{
			.cmd = "set_badge_count",
			.handler = _handler_set_badge_count,
		},
		{
			.cmd = "set_disp_option",
			.handler = _handler_set_display_option,
		},
		{
			.cmd = NULL,
			.handler = NULL,
		},
	};

	if (s_info.initialized == 1) {
		return BADGE_ERROR_NONE;
	} else {
		s_info.initialized = 1;
	}

	ERR("register a service\n");

	s_info.server_fd = com_core_packet_client_init(s_info.socket_file, 0, service_table);
	if (s_info.server_fd < 0) {
		ERR("Failed to make a connection to the master\n");
		return BADGE_ERROR_IO;
	}

	packet = packet_create("service_register", "");
	if (!packet) {
		ERR("Failed to build a packet\n");
		return BADGE_ERROR_IO;
	}

	ret = com_core_packet_async_send(s_info.server_fd, packet, 1.0, _handler_service_register, NULL);
	DBG("Service register sent: %d\n", ret);
	packet_destroy(packet);
	if (ret != 0) {
		com_core_packet_client_fini(s_info.server_fd);
		s_info.server_fd = BADGE_ERROR_INVALID_DATA;
		ret = BADGE_ERROR_IO;
	} else {
		ret = BADGE_ERROR_NONE;
	}

	DBG("Server FD: %d\n", s_info.server_fd);
	return ret;
}

badge_error_e badge_ipc_monitor_deregister(void)
{
	if (s_info.initialized == 0) {
		return BADGE_ERROR_NONE;
	}

	com_core_packet_client_fini(s_info.server_fd);
	s_info.server_fd = BADGE_ERROR_INVALID_DATA;

	s_info.initialized = 0;

	return BADGE_ERROR_NONE;
}

badge_error_e badge_ipc_monitor_init(void)
{
	int ret = BADGE_ERROR_NONE;

	if (badge_ipc_is_master_ready()) {
		ret = badge_ipc_monitor_register();
	}

	if (s_info.is_started_cb_set_svc == 0) {
		_set_master_started_cb(_master_started_cb_service);
		s_info.is_started_cb_set_svc = 1;
	}

	return ret;
}

badge_error_e badge_ipc_monitor_fini(void)
{
	int ret = BADGE_ERROR_NONE;

	if (s_info.is_started_cb_set_svc == 1) {
		_unset_master_started_cb(_master_started_cb_service);
		s_info.is_started_cb_set_svc = 0;
	}

	ret = badge_ipc_monitor_deregister();

	return ret;
}


badge_error_e badge_ipc_request_insert(const char *pkgname, const char *writable_pkg, const char *caller) {
	int ret = 0;
	struct packet *packet;
	struct packet *result;

	packet = packet_create("insert_badge", "sss", pkgname, writable_pkg, caller);
	result = com_core_packet_oneshot_send(BADGE_ADDR,
			packet,
			BADGE_IPC_TIMEOUT);
	packet_destroy(packet);

	if (result != NULL) {
		if (packet_get(result, "i", &ret) != 1) {
			ERR("Failed to get a result packet");
			packet_unref(result);
			return BADGE_ERROR_IO;
		}

		if (ret != BADGE_ERROR_NONE) {
			packet_unref(result);
			return ret;
		}
		packet_unref(result);
	} else {
		badge_ipc_is_master_ready();
		return BADGE_ERROR_SERVICE_NOT_READY;
	}

	return BADGE_ERROR_NONE;
}

badge_error_e badge_ipc_request_delete(const char *pkgname, const char *caller) {
	int ret = 0;
	struct packet *packet;
	struct packet *result;

	packet = packet_create("delete_badge", "ss", pkgname, caller);
	result = com_core_packet_oneshot_send(BADGE_ADDR,
			packet,
			BADGE_IPC_TIMEOUT);
	packet_destroy(packet);

	if (result != NULL) {
		if (packet_get(result, "i", &ret) != 1) {
			ERR("Failed to get a result packet");
			packet_unref(result);
			return BADGE_ERROR_IO;
		}

		if (ret != BADGE_ERROR_NONE) {
			packet_unref(result);
			return ret;
		}
		packet_unref(result);
	} else {
		badge_ipc_is_master_ready();
		return BADGE_ERROR_SERVICE_NOT_READY;
	}

	return BADGE_ERROR_NONE;
}

badge_error_e badge_ipc_request_set_count(const char *pkgname, const char *caller, int count) {
	int ret = 0;
	struct packet *packet;
	struct packet *result;

	packet = packet_create("set_badge_count", "ssi", pkgname, caller, count);
	result = com_core_packet_oneshot_send(BADGE_ADDR,
			packet,
			BADGE_IPC_TIMEOUT);
	packet_destroy(packet);

	if (result != NULL) {
		if (packet_get(result, "i", &ret) != 1) {
			ERR("Failed to get a result packet");
			packet_unref(result);
			return BADGE_ERROR_IO;
		}

		if (ret != BADGE_ERROR_NONE) {
			packet_unref(result);
			return ret;
		}
		packet_unref(result);
	} else {
		badge_ipc_is_master_ready();
		return BADGE_ERROR_SERVICE_NOT_READY;
	}

	return BADGE_ERROR_NONE;
}

badge_error_e badge_ipc_request_set_display(const char *pkgname, const char *caller, int display_option) {
	int ret = 0;
	struct packet *packet;
	struct packet *result;

	packet = packet_create("set_disp_option", "ssi", pkgname, caller, display_option);
	result = com_core_packet_oneshot_send(BADGE_ADDR,
			packet,
			BADGE_IPC_TIMEOUT);
	packet_destroy(packet);

	if (result != NULL) {
		if (packet_get(result, "i", &ret) != 1) {
			ERR("Failed to get a result packet");
			packet_unref(result);
			return BADGE_ERROR_IO;
		}

		if (ret != BADGE_ERROR_NONE) {
			packet_unref(result);
			return ret;
		}
		packet_unref(result);
	} else {
		badge_ipc_is_master_ready();
		return BADGE_ERROR_SERVICE_NOT_READY;
	}

	return BADGE_ERROR_NONE;
}

badge_error_e badge_ipc_setting_property_set(const char *pkgname, const char *property, const char *value)
{
	int status = 0;
	int ret = 0;
	struct packet *packet;
	struct packet *result;

	packet = packet_create("set_noti_property", "sss", pkgname, property, value);
	result = com_core_packet_oneshot_send(BADGE_ADDR,
			packet,
			BADGE_IPC_TIMEOUT);
	packet_destroy(packet);

	if (result != NULL) {
		if (packet_get(result, "ii", &status, &ret) != 2) {
			ERR("Failed to get a result packet");
			packet_unref(result);
			return BADGE_ERROR_IO;
		}
		packet_unref(result);
	} else {
		ERR("failed to receive answer(delete)");
		return BADGE_ERROR_SERVICE_NOT_READY;
	}

	return status;
}

badge_error_e badge_ipc_setting_property_get(const char *pkgname, const char *property, char **value)
{
	int status = 0;
	char *ret = NULL;
	struct packet *packet;
	struct packet *result;

	packet = packet_create("get_noti_property", "ss", pkgname, property);
	result = com_core_packet_oneshot_send(BADGE_ADDR,
			packet,
			BADGE_IPC_TIMEOUT);
	packet_destroy(packet);

	if (result != NULL) {
		if (packet_get(result, "is", &status, &ret) != 2) {
			ERR("Failed to get a result packet");
			packet_unref(result);
			return BADGE_ERROR_IO;
		}
		if (status == BADGE_ERROR_NONE && ret != NULL) {
			*value = strdup(ret);
		}
		packet_unref(result);
	} else {
		ERR("failed to receive answer(delete)");
		return BADGE_ERROR_SERVICE_NOT_READY;
	}

	return status;
}
