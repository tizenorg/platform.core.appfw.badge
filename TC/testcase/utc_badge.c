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

static void utc_badge_create_n(void);
static void utc_badge_create_p(void);
static void utc_badge_remove_n(void);
static void utc_badge_remove_p(void);
static void utc_badge_set_count_n(void);
static void utc_badge_set_count_p(void);
static void utc_badge_get_count_n(void);
static void utc_badge_get_count_p(void);
static void utc_badge_set_display_n(void);
static void utc_badge_set_display_p(void);
static void utc_badge_get_display_n(void);
static void utc_badge_get_display_p(void);
static void utc_badge_is_existing_n(void);
static void utc_badge_is_existing_p(void);
static void utc_badge_foreach_existed_n(void);
static void utc_badge_foreach_existed_p(void);
static void utc_badge_register_changed_cb_n(void);
static void utc_badge_register_changed_cb_p(void);
static void utc_badge_unregister_changed_n(void);
static void utc_badge_unregister_changed_p(void);

#define TEST_PKG "org.tizen.tetware"

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

struct tet_testlist tet_testlist[] = {
	{utc_badge_create_n , NEGATIVE_TC_IDX},
	{utc_badge_create_p, POSITIVE_TC_IDX},
	{utc_badge_remove_n , NEGATIVE_TC_IDX},
	{utc_badge_remove_p, POSITIVE_TC_IDX},
	{utc_badge_set_count_n , NEGATIVE_TC_IDX},
	{utc_badge_set_count_p, POSITIVE_TC_IDX},
	{utc_badge_get_count_n , NEGATIVE_TC_IDX},
	{utc_badge_get_count_p, POSITIVE_TC_IDX},
	{utc_badge_set_display_n , NEGATIVE_TC_IDX},
	{utc_badge_set_display_p, POSITIVE_TC_IDX},
	{utc_badge_get_display_n , NEGATIVE_TC_IDX},
	{utc_badge_get_display_p, POSITIVE_TC_IDX},
	{utc_badge_is_existing_n , NEGATIVE_TC_IDX},
	{utc_badge_is_existing_p, POSITIVE_TC_IDX},
	{utc_badge_foreach_existed_n , NEGATIVE_TC_IDX},
	{utc_badge_foreach_existed_p, POSITIVE_TC_IDX},
	{utc_badge_register_changed_cb_n , NEGATIVE_TC_IDX},
	{utc_badge_register_changed_cb_p, POSITIVE_TC_IDX},
	{utc_badge_unregister_changed_n , NEGATIVE_TC_IDX},
	{utc_badge_unregister_changed_p, POSITIVE_TC_IDX},
	{ NULL, 0 },
};

static void _badge_foreach_cb(const char *pkgname, unsigned int count, void *data) {

}

static void _badge_changed_cb(unsigned int action, const char *pkgname,
			unsigned int count, void *data) {

}

static void startup(void)
{
	/* start of TC */
	tet_printf("\n TC start");
	badge_remove(TEST_PKG);
}


static void cleanup(void)
{
	/* end of TC */
	tet_printf("\n TC end");
}

/**
 * @brief Negative test case of badge_create()
 */
static void utc_badge_create_n(void)
{
	int ret;

	ret = badge_create(NULL, NULL);

	dts_check_eq("badge_create", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_create()
 */
static void utc_badge_create_p(void)
{
	int ret;

	ret = badge_create(TEST_PKG, TEST_PKG);

	dts_check_eq("badge_create", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}


/**
 * @brief Negative test case of badge_remove()
 */
static void utc_badge_remove_n(void)
{
	int ret;

	ret = badge_remove(NULL);

	dts_check_eq("badge_remove", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_remove()
 */
static void utc_badge_remove_p(void)
{
	int ret;

	ret = badge_create(TEST_PKG, TEST_PKG);
	ret = badge_remove(TEST_PKG);

	dts_check_eq("badge_create", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}

/**
 * @brief Negative test case of badge_set_count()
 */
static void utc_badge_set_count_n(void)
{
	int ret;

	ret = badge_set_count(NULL, 0);

	dts_check_eq("badge_set_count", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_set_count()
 */
static void utc_badge_set_count_p(void)
{
	int ret;

	ret = badge_create(TEST_PKG, TEST_PKG);
	ret = badge_set_count(TEST_PKG, 1);

	dts_check_eq("badge_set_count", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}


/**
 * @brief Negative test case of badge_get_count()
 */
static void utc_badge_get_count_n(void)
{
	int ret;

	ret = badge_get_count(NULL, NULL);

	dts_check_eq("badge_get_count", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_get_count()
 */
static void utc_badge_get_count_p(void)
{
	int ret;
	int count = 0;

	ret = badge_create(TEST_PKG, TEST_PKG);
	ret = badge_set_count(TEST_PKG, 1);
	ret = badge_get_count(TEST_PKG, &count);

	dts_check_eq("badge_get_count", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}

/**
 * @brief Negative test case of badge_set_display()
 */
static void utc_badge_set_display_n(void)
{
	int ret;

	ret = badge_set_display(NULL, 0);

	dts_check_eq("badge_set_display", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_set_display()
 */
static void utc_badge_set_display_p(void)
{
	int ret;
	int count = 0;

	ret = badge_create(TEST_PKG, TEST_PKG);
	ret = badge_set_count(TEST_PKG, 1);
	ret = badge_set_display(TEST_PKG, 1);

	dts_check_eq("badge_set_display", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}

/**
 * @brief Negative test case of badge_get_display()
 */
static void utc_badge_get_display_n(void)
{
	int ret;

	ret = badge_get_display(NULL, NULL);

	dts_check_eq("badge_get_display", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_get_display()
 */
static void utc_badge_get_display_p(void)
{
	int ret;
	int count = 0;

	ret = badge_create(TEST_PKG, TEST_PKG);
	ret = badge_set_count(TEST_PKG, 1);
	ret = badge_get_display(TEST_PKG, &count);

	dts_check_eq("badge_get_display", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}

/**
 * @brief Negative test case of badge_is_existing()
 */
static void utc_badge_is_existing_n(void)
{
	int ret;

	ret = badge_is_existing(NULL, NULL);

	dts_check_eq("badge_is_existing", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_is_existing()
 */
static void utc_badge_is_existing_p(void)
{
	int ret;
	int is_existing;

	ret = badge_create(TEST_PKG, TEST_PKG);
	ret = badge_is_existing(TEST_PKG, &is_existing);

	dts_check_eq("badge_is_existing", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}

/**
 * @brief Negative test case of badge_foreach_existed()
 */
static void utc_badge_foreach_existed_n(void)
{
	int ret;

	ret = badge_foreach_existed(NULL, NULL);

	dts_check_eq("badge_foreach_existed", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_foreach_existed()
 */
static void utc_badge_foreach_existed_p(void)
{
	int ret;

	ret = badge_create(TEST_PKG, TEST_PKG);
	ret = badge_foreach_existed(_badge_foreach_cb, NULL);

	dts_check_eq("badge_foreach_existed", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}

/**
 * @brief Negative test case of badge_register_changed_cb()
 */
static void utc_badge_register_changed_cb_n(void)
{
	int ret;

	ret = badge_register_changed_cb(NULL, NULL);

	dts_check_eq("badge_register_changed_cb", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_register_changed_cb()
 */
static void utc_badge_register_changed_cb_p(void)
{
	int ret;

	ret = badge_register_changed_cb(_badge_changed_cb, NULL);

	dts_check_eq("badge_register_changed_cb", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}

/**
 * @brief Negative test case of badge_unregister_changed()
 */
static void utc_badge_unregister_changed_n(void)
{
	int ret;

	ret = badge_unregister_changed_cb(NULL);

	dts_check_eq("badge_unregister_changed_cb", ret, BADGE_ERROR_INVALID_DATA,
		"Must return BADGE_ERROR_INVALID_DATA in case of invalid parameter");
}

/**
 * @brief Positive test case of badge_unregister_changed()
 */
static void utc_badge_unregister_changed_p(void)
{
	int ret;

	ret = badge_register_changed_cb(_badge_changed_cb, NULL);
	ret = badge_unregister_changed_cb(_badge_changed_cb);

	dts_check_eq("badge_unregister_changed_cb", ret, BADGE_ERROR_NONE,
		"Must return BADGE_ERROR_NONE in case of invalid parameter");
}
