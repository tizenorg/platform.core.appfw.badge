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
 * @file badge.h
 * @brief This file contains the badge APIs.
 */

/**
 * @addtogroup BADGE_MODULE
 * @{
 */


/**
 * @internal
 * @brief Enumeration for Badge Action.
 * @since_tizen 2.3
 */
enum badge_action {
	BADGE_ACTION_CREATE = 0,	/**< @internal Badge created */
	BADGE_ACTION_REMOVE,	/**< @internal Badge removed */
	BADGE_ACTION_UPDATE,	/**< @internal Badge updated */
	BADGE_ACTION_CHANGED_DISPLAY,	/**< @internal The display option of the badge changed  */
	BADGE_ACTION_SERVICE_READY,	/**< @internal The badge service is ready */
};


/**
 * @internal
 * @brief Called to retrieve the badge existence.
 * @since_tizen 2.3
 * @param[in] app_id The name of the application
 * @param[in] count The count of the badge
 * @param[in] data The user data passed from the foreach function
 * @pre badge_foreach_existed() will invoke this callback.
 * @see badge_foreach_existed()
 */
typedef void (*badge_cb)(const char *app_id, unsigned int count, void *data);


/**
 * @internal
 * @brief Called when the badge information is changed.
 * @since_tizen 2.3
 * @param[in] action The type of the change
 * @param[in] app_id The name of the application
 * @param[in] count The count of the badge
 * @param[in] data The user data passed from the callback register function
 * @pre badge_register_changed_cb() will invoke this callback.
 * @see badge_unregister_changed_cb()
 */
typedef void (*badge_change_cb)(unsigned int action, const char *app_id,
			unsigned int count, void *data);

/**
 * @internal
 * @brief Retrieves all existing badges.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] callback The callback function
 * @param[in] data The user data to be passed to the callback function
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
int badge_foreach_existed(badge_cb callback, void *data);

/**
 * @internal
 * @brief Registers a callback function to receive badge change event.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/notification
 * @param[in] callback The callback function
 * @param[in] data The user data to be passed to the callback function
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
int badge_register_changed_cb(badge_change_cb callback, void *data);

/**
 * @internal
 * @brief Unregisters a callback function to receive badge change event.
 * @since_tizen 2.3
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



/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BADGE_DEF_H__ */


