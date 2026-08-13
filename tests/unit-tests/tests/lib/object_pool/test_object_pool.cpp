/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

extern "C" {
#include "lib/object_pool.h"
}
#include "object_pool_allocator_wrapper.h"
#include "common/mock_debug.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <string.h>

#define POOL_TEST_VALUE0	(0x01234567)
#define POOL_TEST_VALUE1	(0xf0123456)

TEST_GROUP(object_pool) {
	TEST_SETUP() {
		memset(array, 0x00, sizeof(array));
		object_pool_allocate(&pool); // Invoking C wrapper
	}

	TEST_TEARDOWN() {
		mock().checkExpectations();
		mock().clear();
	}

	void check_remaining_fields(size_t start) {
		for (size_t i = start; i < POOL_SIZE; i++) {
			UNSIGNED_LONGS_EQUAL(0, array[i]);
		}
	}

	struct object_pool pool = {0};
};

TEST(object_pool, allocate_one) {
	uint32_t *first = (uint32_t *)pool_alloc(&pool);
	*first = POOL_TEST_VALUE0;

	UNSIGNED_LONGS_EQUAL(POOL_TEST_VALUE0, array[0]);
	check_remaining_fields(1);
}

TEST(object_pool, allocate_two) {
	uint32_t *first = (uint32_t *)pool_alloc(&pool);
	*first = POOL_TEST_VALUE0;

	uint32_t *second = (uint32_t *)pool_alloc(&pool);
	*second = POOL_TEST_VALUE1;

	UNSIGNED_LONGS_EQUAL(POOL_TEST_VALUE0, array[0]);
	UNSIGNED_LONGS_EQUAL(POOL_TEST_VALUE1, array[1]);
	check_remaining_fields(2);
}

TEST(object_pool, allocate_two_at_once) {
	uint32_t *items = (uint32_t *)pool_alloc_n(&pool, 2);
	items[0] = POOL_TEST_VALUE0;
	items[1] = POOL_TEST_VALUE1;

	UNSIGNED_LONGS_EQUAL(POOL_TEST_VALUE0, array[0]);
	UNSIGNED_LONGS_EQUAL(POOL_TEST_VALUE1, array[1]);
	check_remaining_fields(2);
}

TEST(object_pool, allocate_one_then_overflow) {
	panic_environment_t env;

	uint32_t *first = (uint32_t*) pool_alloc(&pool);
	*first = POOL_TEST_VALUE0;

	if (SETUP_PANIC_ENVIRONMENT(env)) {
		expect_tf_log(
				"Cannot allocate 16 objects out of pool (15 objects left).\n");
		pool_alloc_n(&pool, POOL_SIZE);
	}
}

TEST(object_pool, overflow) {
	panic_environment_t env;

	if (SETUP_PANIC_ENVIRONMENT(env)) {
		expect_tf_log("Cannot allocate 17 objects out of pool (16 objects left).\n");
		pool_alloc_n(&pool, POOL_SIZE + 1);
	}
}
