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

#ifndef __BADGE_IPC_H__
#define __BADGE_IPC_H__

#include <badge.h>

#define BADGE_ADDR "/tmp/.badge.service"

#ifdef __cplusplus
extern "C" {
#endif

struct packet;

int badge_ipc_monitor_init(void);
int badge_ipc_monitor_fini(void);

int badge_ipc_request_insert(const char *pkgname, const char *writable_pkg, const char *caller);
int badge_ipc_request_delete(const char *pkgname, const char *caller);
int badge_ipc_request_set_count(const char *pkgname, const char *caller, int count);
int badge_ipc_request_get_count(const char *pkgname, int *count);
int badge_ipc_request_set_display(const char *pkgname, const char *caller, int display_option);
int badge_ipc_request_get_display(const char *pkgname, int *is_display);

int badge_ipc_is_master_ready(void);
int badge_ipc_add_deferred_task(void (*badge_add_deferred_task)(void *data), void *user_data);
int badge_ipc_del_deferred_task(void (*badge_add_deferred_task)(void *data));

int badge_ipc_setting_property_set(const char *pkgname, const char *property, const char *value);
int badge_ipc_setting_property_get(const char *pkgname, const char *property, char **value);

#ifdef __cplusplus
}
#endif
#endif
