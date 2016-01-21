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


#ifndef __BADGE_DB_DEF_H__
#define __BADGE_DB_DEF_H__

#include <stdbool.h>
#include <sqlite3.h>
#include <badge_error.h>

#ifdef __cplusplus
extern "C" {
#endif

int badge_db_insert(const char *pkgname, const char *writable_pkg, const char *caller);
int badge_db_delete(const char *pkgname, const char *caller_pkg);
int badge_db_set_count(const char *pkgname, const char *caller_pkg, int count);
int badge_db_get_count(const char *pkgname, int *count);
int badge_db_set_display_option(const char *pkgname, const char *caller_pkg, int is_display);
int badge_db_get_display_option(const char *pkgname, int *is_display);
int badge_db_exec(sqlite3 * db, const char *query, int *num_changes);

#ifdef __cplusplus
	}
#endif

#endif /* __BADGE_DB_DEF_H__ */

