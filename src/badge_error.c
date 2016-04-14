/*
 *  libbadge
 *
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <gio/gio.h>
#include "badge_error.h"

static const GDBusErrorEntry dbus_error_entries[] = {
	{BADGE_ERROR_INVALID_PARAMETER, "org.freedesktop.Badge.Error.INVALID_PARAMETER"},
	{BADGE_ERROR_OUT_OF_MEMORY,     "org.freedesktop.Badge.Error.OUT_OF_MEMORY"},
	{BADGE_ERROR_IO_ERROR,          "org.freedesktop.Badge.Error.IO_ERROR"},
	{BADGE_ERROR_PERMISSION_DENIED, "org.freedesktop.Badge.Error.PERMISSION_DENIED"},
	{BADGE_ERROR_FROM_DB,           "org.freedesktop.Badge.Error.FROM_DB"},
	{BADGE_ERROR_ALREADY_EXIST,  "org.freedesktop.Badge.Error.ALREADY_EXIST"},
	{BADGE_ERROR_FROM_DBUS,         "org.freedesktop.Badge.Error.FROM_DBUS"},
	{BADGE_ERROR_NOT_EXIST,      "org.freedesktop.Badge.Error.NOT_EXIST"},
	{BADGE_ERROR_SERVICE_NOT_READY, "org.freedesktop.Badge.Error.SERVICE_NOT_READY"},
	{BADGE_ERROR_INVALID_PACKAGE, "org.freedesktop.Badge.Error.INVALID_PACKAGE"},
};

EXPORT_API GQuark badge_error_quark(void)
{
	static volatile gsize quark_volatile = 0;
	g_dbus_error_register_error_domain("badge-error-quark",
			&quark_volatile,
			dbus_error_entries,
			G_N_ELEMENTS(dbus_error_entries));
	return (GQuark) quark_volatile;
}

