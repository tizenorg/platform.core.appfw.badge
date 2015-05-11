/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libintl.h>
#include <tet_api.h>
#include <badge.h>

#define TEST_PKG "org.tizen.tetware"

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

static void startup(void);
static void cleanup(void);

static void utc_badge_setting_property_set_n(void);
static void utc_badge_setting_property_set_p(void);
static void utc_badge_setting_property_get_n(void);
static void utc_badge_setting_property_get_p(void);

#define TEST_PKG "org.tizen.tetware"

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

struct tet_testlist tet_testlist[] = {
	{utc_badge_setting_property_set_n, NEGATIVE_TC_IDX},
	{utc_badge_setting_property_set_p, POSITIVE_TC_IDX},
	{utc_badge_setting_property_get_n, NEGATIVE_TC_IDX},
	{utc_badge_setting_property_get_p, POSITIVE_TC_IDX},
	{ NULL, 0 },
};

static void startup(void)
{
	/* start of TC */
	tet_printf("\n TC start");
}


static void cleanup(void)
{
	/* end of TC */
	tet_printf("\n TC end");
}

/**
 * @brief Negative test case of badge_setting_property_set()
 */
static void utc_badge_setting_property_set_n(void)
{
	int ret;

	ret = badge_setting_property_set(NULL, NULL, NULL);
	dts_check_eq("badge_setting_property_set", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_setting_property_set()
 */
static void utc_badge_setting_property_set_p(void)
{
	int ret;

	ret = badge_setting_property_set(TEST_PKG, "OPT_BADGE", "ON");
	/*Invalid parameter test*/
	dts_check_ne("badge_setting_property_set", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_NONE in case of valid parameter");
}

/**
 * @brief Negative test case of badge_setting_property_get()
 */
static void utc_badge_setting_property_get_n(void)
{
	int ret;

	ret = badge_setting_property_get(NULL, NULL, NULL);
	dts_check_eq("badge_setting_property_get", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_setting_property_get()
 */
static void utc_badge_setting_property_get_p(void)
{
	int ret;
	char *value = NULL;

	ret = badge_setting_property_get(TEST_PKG, "OPT_BADGE", &value);
	dts_check_ne("badge_setting_property_get", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_NONE in case of valid parameter");
}
