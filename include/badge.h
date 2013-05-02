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

#ifndef __BADGE_DEF_H__
#define __BADGE_DEF_H__

#include <stdbool.h>
#include <badge_error.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BADGE_LIBRARY Badge Library
 * @brief This badge library to store badge information of applications.
 */

/**
 * @ingroup BADGE_LIBRARY
 * @defgroup BADGE_TYPE badge type
 * @brief Badge type
 */

/**
 * @addtogroup BADGE_TYPE
 * @{
 */

/**
 * @breief Enumeration for Badge action
 */
enum _badge_action {
	BADGE_ACTION_CREATE = 0,
	BADGE_ACTION_REMOVE,
	BADGE_ACTION_UPDATE,
	BADGE_ACTION_CHANGED_DISPLAY,
	BADGE_ACTION_SERVICE_READY,
};


/**
 * @brief Called to retrieve the badge existed.
 * @param[in] pkgname The name of package
 * @param[in] count The count of badge
 * @param[in] user_data The user data passed from the foreach function
 * @pre badge_foreach_existed() will invoke this callback.
 * @see badge_foreach_existed()
 */
typedef void (*badge_cb)(const char *pkgname, unsigned int count, void *data);


/**
 * @brief Called when badge information is changed.
 * @param[in] action The type of changing
 * @param[in] pkgname The name of package
 * @param[in] count The count of badge
 * @param[in] user_data The user data passed from the callback register function
 * @pre badge_register_changed_cb() will invoke this callback.
 * @see badge_unregister_changed_cb()
 */
typedef void (*badge_change_cb)(unsigned int action, const char *pkgname,
			unsigned int count, void *data);


/**
 * @ingroup BADGE_LIBRARY
 * @defgroup BADGE badge core API
 * @brief badge core API
 */

/**
 * @addtogroup BADGE
 * @{
 */


/**
 * @brief This function creates badge for designated package.
 * @details Creates new badge to display.
 * @param[in] pkgname The name of designated package
 * @param[in] writable_pkg The name of package which is authorized to change badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	badge_error_e err = BADGE_ERROR_NONE;

	err = badge_create("org.tizen.sms", "org.tizen.sms2");
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
badge_error_e badge_create(const char *pkgname, const char *writable_pkg);


/**
 * @brief This function removes badge for designated package.
 * @param[in] pkgname The name of designated package
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	badge_error_e err = BADGE_ERROR_NONE;

	err = badge_remove("org.tizen.sms");
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
badge_error_e badge_remove(const char *pkgname);

/**
 * @brief This function sets badge count for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] count The count of badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_create()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	badge_error_e err = BADGE_ERROR_NONE;

	err = badge_set_count("org.tizen.sms", 1);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
badge_error_e badge_set_count(const char *pkgname, unsigned int count);

/**
 * @brief This function gets badge count for designated package.
 * @param[in] pkgname The name of designated package
 * @param[out] count The count of badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_create()
 * @see badge_set_count()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	badge_error_e err = BADGE_ERROR_NONE;

	err = badge_get_count("org.tizen.sms", 1);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
badge_error_e badge_get_count(const char *pkgname, unsigned int *count);

/**
 * @brief This function sets displaying option for designated package.
 * @param[in] pkgname The name of designated package
 * @param[in] is_display The displaying option, 1 = display 0 = not display
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_create()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	badge_error_e err = BADGE_ERROR_NONE;

	err = badge_set_display("org.tizen.sms", 1);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
badge_error_e badge_set_display(const char *pkgname, unsigned int is_display);

/**
 * @brief This function gets displaying option for designated package.
 * @param[in] pkgname The name of designated package
 * @param[out]  is_display The displaying option, 1 = display 0 = not display
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_create()
 * @see badge_set_count()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int is_display = 0;
	badge_error_e err = BADGE_ERROR_NONE;

	err = badge_get_display("org.tizen.sms", &is_display);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
badge_error_e badge_get_display(const char *pkgname, unsigned int *is_display);

/**
 * @brief This function tests badge for designated package is existed or not.
 * @param[in] pkgname The name of designated package
 * @param[out] existing The bool value of badge existing status
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_create()
 * @see badge_remove()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	badge_error_e err = BADGE_ERROR_NONE;
	bool exist;

	err = badge_is_existing("org.tizen.sms", &exist);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
badge_error_e badge_is_existing(const char *pkgname, bool *existing);

/**
 * @brief This function retrieves all badges which are existed.
 * @param[in] callback The callback function
 * @param[in] data The user data to be passed to the callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_get_count()
 * @see badge_is_existing()
 */
badge_error_e badge_foreach_existed(badge_cb callback, void *data);

/**
 * @brief This function registers callback function to receive badge changed event.
 * @param[in] callback The callback function
 * @param[in] data The user data to be passed to the callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_create()
 * @see badge_remove()
 * @see badge_set_count()
 */
badge_error_e badge_register_changed_cb(badge_change_cb callback, void *data);

/**
 * @brief This function unregisters callback function to receive badge changed event.
 * @param[in] callback The callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @see #badge_error_e
 * @see badge_register_changed_cb()
 */
badge_error_e badge_unregister_changed_cb(badge_change_cb callback);

int badge_is_service_ready(void);

badge_error_e badge_add_deffered_task(
		void (*deffered_task_cb)(void *data), void *user_data);

badge_error_e badge_del_deffered_task(
		void (*deffered_task_cb)(void *data));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BADGE_DEF_H__ */


