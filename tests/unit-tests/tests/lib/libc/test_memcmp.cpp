/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CppUTest/TestHarness.h"
extern "C" {
#include "lib/libc/string.h"
}

#define BUFFER_SIZE	(16)

TEST_GROUP(memcmp) {
	TEST_SETUP() {
		for (int i = 0; i < BUFFER_SIZE; i++) {
			buffer1[i] = 0;
			buffer2[i] = 0;
		}
	}

	uint8_t buffer1[BUFFER_SIZE];
	uint8_t buffer2[BUFFER_SIZE];
};

TEST(memcmp, zero_length) {
	LONGS_EQUAL(0, memcmp(buffer1, buffer2, 0));
}

TEST(memcmp, same) {
	LONGS_EQUAL(0, memcmp(buffer1, buffer2, sizeof(buffer1)));
}

TEST(memcmp, first_diff_positive) {
	buffer1[0] = 1;
	LONGS_EQUAL(1, memcmp(buffer1, buffer2, sizeof(buffer1)));
}

TEST(memcmp, first_diff_negative) {
	buffer2[0] = 1;
	LONGS_EQUAL(-1, memcmp(buffer1, buffer2, sizeof(buffer1)));
}

TEST(memcmp, second_diff_positive) {
	buffer1[1] = 1;
	LONGS_EQUAL(1, memcmp(buffer1, buffer2, sizeof(buffer1)));
}

TEST(memcmp, second_diff_negative) {
	buffer2[1] = 1;
	LONGS_EQUAL(-1, memcmp(buffer1, buffer2, sizeof(buffer1)));
}

TEST(memcmp, last_diff_positive) {
	buffer1[sizeof(buffer1) - 1] = 1;
	LONGS_EQUAL(1, memcmp(buffer1, buffer2, sizeof(buffer1)));
}

TEST(memcmp, last_diff_negative) {
	buffer2[sizeof(buffer2) - 1] = 1;
	LONGS_EQUAL(-1, memcmp(buffer1, buffer2, sizeof(buffer1)));
}
