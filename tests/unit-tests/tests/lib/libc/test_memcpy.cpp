/*
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CppUTest/TestHarness.h"
extern "C" {
#include "lib/libc/string.h"
}

#define BUFFER_SIZE	(16)
#define CANARY		0x55
#define SOURCE		0xaa

static int check_buffer(const uint8_t *buffer, unsigned long int length,
			uint8_t content)
{
	unsigned long int i;

	for (i = 0; i < length; i++)
		if (buffer[i] != content)
			return 0;

	return 1;
}

TEST_GROUP(memcpy) {
	TEST_SETUP() {
		for (int i = 0; i < BUFFER_SIZE; i++) {
			dst[i] = CANARY;
			src[i] = SOURCE;
		}
	}

	uint8_t dst[BUFFER_SIZE];
	uint8_t src[BUFFER_SIZE];
};

TEST(memcpy, zero_length) {
	POINTERS_EQUAL(dst, memcpy(dst, src, 0));
	CHECK_EQUAL(check_buffer(src, sizeof(src), SOURCE), 1);
	CHECK_EQUAL(check_buffer(dst, sizeof(dst), CANARY), 1);
}

TEST(memcpy, copy_inital) {
	POINTERS_EQUAL(dst, memcpy(dst, src, 5));
	CHECK_EQUAL(check_buffer(src, sizeof(src), SOURCE), 1);
	CHECK_EQUAL(check_buffer(dst, 5, SOURCE), 1);
	CHECK_EQUAL(check_buffer(dst + 5, BUFFER_SIZE - 5, CANARY), 1);
}

TEST(memcpy, copy_middle) {
	POINTERS_EQUAL(dst, memcpy(dst, src + 5, 7));
	CHECK_EQUAL(check_buffer(src, sizeof(src), SOURCE), 1);
	CHECK_EQUAL(check_buffer(dst, 7, SOURCE), 1);
	MEMCMP_EQUAL(dst, src + 5, 7);
	CHECK_EQUAL(check_buffer(dst + 7, BUFFER_SIZE - 7, CANARY), 1);
}

TEST(memcpy, copy_all) {
	POINTERS_EQUAL(dst, memcpy(dst, src, sizeof(dst)));
	CHECK_EQUAL(check_buffer(src, sizeof(src), SOURCE), 1);
	CHECK_EQUAL(check_buffer(dst, sizeof(dst), SOURCE), 1);
	MEMCMP_EQUAL(src, dst, sizeof(dst));
}
