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

#ifndef __BADGE_INTERNAL_DEF_H__
#define __BADGE_INTERNAL_DEF_H__

#include <stdbool.h>
#include <stdarg.h>

#include "badge_error.h"
#include "badge.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _badge_h badge_h;


/**
 * @internal
 * @brief This function checks whether badge service is ready
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @remarks The specific error code can be obtained using the get_last_result() method. Error codes are described in Exception section.
 * @return 1 if badge service is ready, other value if badge service isn't ready
 * @exception BADGE_ERROR_NONE Success
 * @exception BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 */
int badge_is_service_ready(void);

/**
 * @internal
 * @brief This function adds deferred task. the registered task will be executed when badge service become ready
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] badge_add_deferred_task The callback function
 * @param[in] user_data The user data to be passed to the callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_OUT_OF_MEMORY Out of memory
 * @see #badge_error_e
 * @see badge_is_service_ready()
 */
int badge_add_deferred_task(
		void (*badge_add_deferred_task)(void *data), void *user_data);

/**
 * @internal
 * @brief This function removes deferred task.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] badge_add_deferred_task The callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @see #badge_error_e
 * @see badge_is_service_ready()
 */
int badge_del_deferred_task(
		void (*badge_add_deferred_task)(void *data));

/**
 * @internal
 * @brief Tests if the badge for the designated application exists or not.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] app_id The name of the designated application
 * @param[out] existing The bool value of badge existence status
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_FROM_DB Error from DB
 * @retval BADGE_ERROR_NOT_EXIST Not exist
 * @retval BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 * @see #badge_error_e
 * @see badge_new()
 * @see badge_remove()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;
	bool exist;

	err = badge_is_existing(app_id, &exist);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_is_existing(const char *app_id, bool *existing);

char *_badge_get_pkgname_by_pid(void);

int _badge_is_existing(const char *pkgname, bool *existing);

int _badge_foreach_existed(badge_foreach_cb callback, void *data);

int _badge_insert(badge_h *badge);

int _badge_remove(const char *caller, const char *pkgname);

int _badget_set_count(const char *caller, const char *pkgname,
			unsigned int count);

int _badget_get_count(const char *pkgname, unsigned int *count);

int _badget_set_display(const char *pkgname,
			unsigned int is_display);

int _badget_get_display(const char *pkgname, unsigned int *is_display);

int _badge_register_changed_cb(badge_change_cb callback, void *data);

int _badge_unregister_changed_cb(badge_change_cb callback);

int _badge_free(badge_h *badge);

badge_h *_badge_new(const char *pkgname, const char *writable_pkgs,
		int *err);

char *_badge_pkgs_new(int *err, const char *pkg1, ...);

char *_badge_pkgs_new_valist(int *err,
			const char *pkg1, va_list args);

void badge_changed_cb_call(unsigned int action, const char *pkgname,
			unsigned int count);

/**
 * @internal
 * @brief Creates a badge for the designated package.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @details Creates new badge to display.
 * @param[in] pkgname The name of the designated package
 * @param[in] writable_pkg The name of package which is authorized to change the badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;

	err = badge_create("org.tizen.sms", "org.tizen.sms2");
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_create(const char *pkgname, const char *writable_pkg);

/**
 * @brief This function sets badge property for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] property name
 * @param[in] property value
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 */
int badge_setting_property_set(const char *pkgname, const char *property, const char *value);

/**
 * @brief This function gets badge property for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] property name
 * @param[in] pointer which can save the getting value
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 */
int badge_setting_property_get(const char *pkgname, const char *property, char **value);

/**
 * @internal
 * @brief Called to retrieve the badge existence.
 * @since_tizen 2.4
 * @param[in] app_id The name of the application
 * @param[in] count The count of the badge
 * @param[in] user_data The user data passed from the foreach function
 * @pre badge_foreach_existed() will invoke this callback.
 * @see badge_foreach_existed()
 */
typedef void (*badge_cb)(const char *app_id, unsigned int count, void *user_data) DEPRECATED;


/**
 * @internal
 * @brief Retrieves all existing badges.
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] callback The callback function
 * @param[in] user_data The user data to be passed to the callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_FROM_DB Error form DB
 * @retval BADGE_ERROR_OUT_OF_MEMORY Out of memory
 * @retval BADGE_ERROR_NOT_EXIST Not exist
 * @see #badge_error_e
 * @see badge_get_count()
 * @see badge_is_existing()
 */
int badge_foreach_existed(badge_cb callback, void *user_data) DEPRECATED;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BADGE_INTERNAL_DEF_H__ */

