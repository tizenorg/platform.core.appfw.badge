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

#ifndef __BADGE_DEF_H__
#define __BADGE_DEF_H__

#include <stdbool.h>
#include <badge_error.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file badge.h
 * @brief This file contains the badge APIs.
 */

/**
 * @addtogroup BADGE_MODULE
 * @{
 */

/**
 * @deprecated Deprecated since 2.4 Use badge_add instead.
 * @brief Creates a badge for the application itself.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @details Creates new badge to display.
 * @param[in] writable_app_id The id of application which is authorized to change the badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_IO_ERROR Error from I/O
 * @retval BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 * @see #badge_error_e
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;

	err = badge_new(app_id);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_new(const char *writable_app_id);



/**
 * @brief Creates a badge for the application specified by the badge_app_id.
 * @remarks Creating and updating a badge of the other application is allowed only when both applications are signed with the same certificate.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] badge_app_id The id of the application for which the badge will be created. This parameter can be null when creating a badge for itself.
 * @return #BADGE_ERROR_NONE If success, other value if failure
 * @retval #BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval #BADGE_ERROR_IO_ERROR Error from I/O
 * @retval #BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 * @retval #BADGE_ERROR_INVALID_PACKAGE The caller application is not signed with the certificate of the badge_app_id
 * @see #badge_error_e
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;

	err = badge_add("org.tizen.email");
	if(err != BADGE_ERROR_NONE) {
		return;
	}
}
 * @endcode
 */
int badge_add(const char *badge_app_id);


/**
 * @brief Removes the badge for the designated application.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] app_id The name of the designated application
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_IO_ERROR Error from I/O
 * @retval BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 * @see #badge_error_e
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;

	err = badge_remove(app_id);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_remove(const char *app_id);

/**
 * @brief Sets badge count for the designated application.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] app_id The name of the designated application
 * @param[in] count The count of the badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_IO_ERROR Error from I/O
 * @retval BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 * @see #badge_error_e
 * @see badge_new()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;

	err = badge_set_count(app_id, 1);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_set_count(const char *app_id, unsigned int count);

/**
 * @brief Gets badge count for the designated application.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] app_id The name of the designated application
 * @param[out] count The count of the badge
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_FROM_DB Error from DB
 * @retval BADGE_ERROR_ALREADY_EXIST Already exist
 * @retval BADGE_ERROR_OUT_OF_MEMORY Out of memory
 * @see #badge_error_e
 * @see badge_new()
 * @see badge_set_count()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;
	int count = 0;

	err = badge_get_count(app_id, &count);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_get_count(const char *app_id, unsigned int *count);

/**
 * @brief Sets the display option for the designated application.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] app_id The name of the designated application
 * @param[in] is_display The display option (1 = display, 0 = not display)
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_IO_ERROR Error from I/O
 * @retval BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 * @see #badge_error_e
 * @see badge_new()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int err = BADGE_ERROR_NONE;

	err = badge_set_display(app_id, 1);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_set_display(const char *app_id, unsigned int is_display);

/**
 * @brief Gets the display option for the designated application.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] app_id The name of the designated application
 * @param[out]  is_display The display option (1 = display, 0 = not display)
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_FROM_DB Error from DB
 * @retval BADGE_ERROR_NOT_EXIST Not exist
 * @retval BADGE_ERROR_SERVICE_NOT_READY Service is not ready
 * @retval BADGE_ERROR_OUT_OF_MEMORY Out of memory
 * @see #badge_error_e
 * @see badge_new()
 * @see badge_set_count()
 * @par Sample code:
 * @code
#include <badge.h>
...
{
	int is_display = 0;
	int err = BADGE_ERROR_NONE;

	err = badge_get_display(app_id, &is_display);
	if(err != BADGE_ERROR_NONE) {
		return;
	}

}
 * @endcode
 */
int badge_get_display(const char *app_id, unsigned int *is_display);

/**
 * @brief Callback function for getting result of badge_foreach.
 * @since_tizen 2.4
 * @param[in] app_id The id of the application
 * @param[in] count The count of the badge
 * @param[in] user_data The user data passed from the badge_foreach function
 * @return true to continue with the next iteration of the loop, false to break out of the loop
 * @pre badge_foreach() will invoke this callback.
 * @see badge_foreach()
 */
typedef bool (*badge_foreach_cb)(const char *app_id, unsigned int count, void *user_data);


/**
 * @brief Retrieves all existing badges.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
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
 * @see badge_foreach_cb()
 */
int badge_foreach(badge_foreach_cb callback, void *user_data);

/**
 * @brief Enumeration for Badge Action.
 * @since_tizen 2.4
 */
enum badge_action {
	BADGE_ACTION_CREATE = 0,	/**< Badge created */
	BADGE_ACTION_REMOVE,	/**< Badge removed */
	BADGE_ACTION_UPDATE,	/**< Badge updated */
	BADGE_ACTION_CHANGED_DISPLAY,	/**< The display option of the badge changed  */
	BADGE_ACTION_SERVICE_READY,	/**< The badge service is ready */
};

/**
 * @brief Called when the badge information is changed.
 * @since_tizen 2.4
 * @param[in] action The type of the change. Refer #badge_action
 * @param[in] app_id The name of the application
 * @param[in] count The count of the badge
 * @param[in] user_data The user data passed from the callback register function
 * @pre badge_register_changed_cb() will invoke this callback.
 * @see badge_unregister_changed_cb()
 */
typedef void (*badge_change_cb)(unsigned int action, const char *app_id,
			unsigned int count, void *user_data);
/**
 * @brief Registers a callback function to receive badge change event.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] callback The callback function
 * @param[in] user_data The user data to be passed to the callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_OUT_OF_MEMORY Out of memory
 * @see #badge_error_e
 * @see badge_new()
 * @see badge_remove()
 * @see badge_set_count()
 */
int badge_register_changed_cb(badge_change_cb callback, void *user_data);

/**
 * @brief Unregisters a callback function to receive badge change event.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] callback The callback function
 * @return #BADGE_ERROR_NONE if success, other value if failure
 * @retval BADGE_ERROR_NONE Success
 * @retval BADGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval BADGE_ERROR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval BADGE_ERROR_NOT_EXIST Not exist
 * @see #badge_error_e
 * @see badge_register_changed_cb()
 */
int badge_unregister_changed_cb(badge_change_cb callback);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BADGE_DEF_H__ */


