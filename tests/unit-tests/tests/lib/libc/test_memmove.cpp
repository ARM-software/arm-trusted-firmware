/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "CppUTest/TestHarness.h"
extern "C" {
#include "lib/libc/string.h"
}

#define BUFFER_SIZE	10

static unsigned int memcpy_call_count;

/* Need to intercept memcpy calls from code-under-test.
 * The current implementation delegates to memcpy if
 * the buffers do not overlap. We need to make sure
 * that this only happens if and only if they are
 * separate, as memcpy behaviour is undefined if
 * the buffers overlap. */
extern "C" void *mock_memcpy(void *dst, const void *src, size_t len)
{
	memcpy_call_count++;

	uint8_t *d = (uint8_t *)dst;
	const uint8_t *s = (const uint8_t *)src;

	for (size_t i = 0; i < len; i++) {
		d[i] = s[i];
	}

	return dst;
}

TEST_GROUP(memmove) {
	TEST_SETUP() {
		memcpy_call_count = 0;

		for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
			buffer[i] = (uint8_t)i;
			src[i] = (uint8_t)i;
			dst[i] = 0xaa;
		}
	}

	uint8_t buffer[BUFFER_SIZE];
	uint8_t src[BUFFER_SIZE];
	uint8_t dst[BUFFER_SIZE];
};

TEST(memmove, non_overlapping_buffers) {
	const uint8_t expected_src[BUFFER_SIZE] = { 0, 1, 2, 3, 4,
						    5, 6, 7, 8, 9 };

	POINTERS_EQUAL(dst, memmove(dst, src, sizeof(src)));
	MEMCMP_EQUAL(src, dst, sizeof(dst));
	MEMCMP_EQUAL(expected_src, src, sizeof(src));
}

TEST(memmove, zero_length) {
	const uint8_t expected_src[BUFFER_SIZE] = { 0, 1, 2, 3, 4,
						    5, 6, 7, 8, 9 };
	const uint8_t expected_dst[BUFFER_SIZE] = { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
						    0xaa, 0xaa, 0xaa, 0xaa, 0xaa };

	POINTERS_EQUAL(dst, memmove(dst, src, 0));
	MEMCMP_EQUAL(expected_src, src, sizeof(src));
	MEMCMP_EQUAL(expected_dst, dst, sizeof(dst));
}

TEST(memmove, destination_equals_source) {
	const uint8_t expected[BUFFER_SIZE] = { 0, 1, 2, 3, 4,
						5, 6, 7, 8, 9 };

	POINTERS_EQUAL(buffer, memmove(buffer, buffer, sizeof(buffer)));
	MEMCMP_EQUAL(expected, buffer, sizeof(buffer));
	UNSIGNED_LONGS_EQUAL_TEXT(0, memcpy_call_count,
				  "memcpy called with overlapping buffers!");
}

TEST(memmove, overlapping_source_after_destination) {
	const uint8_t expected[BUFFER_SIZE] = { 2, 3, 4, 5, 6,
						5, 6, 7, 8, 9 };

	POINTERS_EQUAL(buffer, memmove(buffer, buffer + 2, 5));
	MEMCMP_EQUAL(expected, buffer, sizeof(buffer));
	UNSIGNED_LONGS_EQUAL_TEXT(0, memcpy_call_count, "memcpy called with overlapping buffers!");
}

TEST(memmove, overlapping_destination_after_source) {
	const uint8_t expected[BUFFER_SIZE] = { 0, 1, 0, 1, 2,
						3, 4, 7, 8, 9 };

	POINTERS_EQUAL(buffer + 2, memmove(buffer + 2, buffer, 5));
	MEMCMP_EQUAL(expected, buffer, sizeof(buffer));
	UNSIGNED_LONGS_EQUAL_TEXT(0, memcpy_call_count, "memcpy called with overlapping buffers!");
}
