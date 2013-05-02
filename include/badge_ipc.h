/*
 *  libnotification
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Seungtaek Chung <seungtaek.chung@samsung.com>, Mi-Ju Lee <miju52.lee@samsung.com>, Xi Zhichan <zhichan.xi@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
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

badge_error_e badge_ipc_monitor_init(void);
badge_error_e badge_ipc_monitor_fini(void);

badge_error_e badge_ipc_request_insert(const char *pkgname, const char *writable_pkg, const char *caller);
badge_error_e badge_ipc_request_delete(const char *pkgname, const char *caller);
badge_error_e badge_ipc_request_set_count(const char *pkgname, const char *caller, int count);
badge_error_e badge_ipc_request_set_display(const char *pkgname, const char *caller, int display_option);

int badge_ipc_is_master_ready(void);
badge_error_e badge_ipc_add_deffered_task(void (*deffered_task_cb)(void *data), void *user_data);
badge_error_e badge_ipc_del_deffered_task(void (*deffered_task_cb)(void *data));

#ifdef __cplusplus
}
#endif
#endif
