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


#ifndef __BADGE_ERROR_DEF_H__
#define __BADGE_ERROR_DEF_H__

#include <tizen.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file badge_error.h
 * @brief This file contains the badge APIs
 */

/**
 * @addtogroup BADGE_MODULE
 * @{
 */

/**
 * @brief Enumeration for Badge Error
 */
typedef enum _badge_error_e {
	BADGE_ERROR_NONE = TIZEN_ERROR_NONE,			/**< Success */
	BADGE_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,		/**< Invalid parameter */
	BADGE_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY,		/**< Out of memory */
	BADGE_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED,	/**< Permission denied */
	BADGE_ERROR_IO_ERROR = TIZEN_ERROR_IO_ERROR,	/**< Error from I/O */
	BADGE_ERROR_FROM_DB = TIZEN_ERROR_BADGE | 0x01,		/**< Error from DB */
	BADGE_ERROR_ALREADY_EXIST = TIZEN_ERROR_BADGE | 0x02,		/**< Already exist */
	BADGE_ERROR_FROM_DBUS = TIZEN_ERROR_BADGE | 0x03,		/**< Error from DBus */
	BADGE_ERROR_NOT_EXIST = TIZEN_ERROR_BADGE | 0x04,		/**< Not exist */
	BADGE_ERROR_SERVICE_NOT_READY = TIZEN_ERROR_BADGE | 0x05,	/**< Error service not ready */
	BADGE_ERROR_INVALID_PACKAGE = TIZEN_ERROR_BADGE | 0x06,	/**< The caller application is not signed with the certificate of the badge owner*/
} badge_error_e;

#define BADGE_ERROR (badge_error_quark ())
GQuark badge_error_quark(void);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif

#endif /* __BADGE_ERROR_DEF_H__ */

