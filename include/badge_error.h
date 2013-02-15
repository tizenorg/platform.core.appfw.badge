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


#ifndef __BADGE_ERROR_DEF_H__
#define __BADGE_ERROR_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

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
 * @breief Enumeration for Badge error
 */
typedef enum _badge_error_e {
	BADGE_ERROR_NONE = 0,			/**< Success */
	BADGE_ERROR_INVALID_DATA = -1,		/**< Invalid parameter */
	BADGE_ERROR_NO_MEMORY = -2,		/**< No memory */
	BADGE_ERROR_FROM_DB = -3,		/**< Error from DB */
	BADGE_ERROR_ALREADY_EXIST = -4,		/**< Already exist */
	BADGE_ERROR_FROM_DBUS = -5,		/**< Error from DBus */
	BADGE_ERROR_NOT_EXIST = -6,		/**< Not exist */
	BADGE_ERROR_PERMISSION_DENIED = -7,	/**< Permission denied */
} badge_error_e;

#ifdef __cplusplus
	}
#endif

#endif /* __BADGE_ERROR_DEF_H__ */

