/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CppUTest/TestHarness.h"
extern "C" {
#include "lib/libc/string.h"
}

#define BUFFER_SIZE	16
#define CANARY		0x55
#define TESTCHAR	0xaa

/* Check that a buffer consists only of bytes with the same content. */
static int check_buffer(const uint8_t *buffer, unsigned long int length,
			uint8_t content)
{
	unsigned long int i;

	for (i = 0; i < length; i++)
		if (buffer[i] != content)
			return 0;

	return 1;
}

TEST_GROUP(memset) {
	TEST_SETUP() {
		for (int i = 0; i < BUFFER_SIZE; i++) {
			buf[i] = CANARY;
		}
	}

	uint8_t buf[BUFFER_SIZE];
};

/* memset with a zero length should not touch the buffer. */
TEST(memset, zero_length) {
	POINTERS_EQUAL(buf, memset(buf, TESTCHAR, 0));
	CHECK_EQUAL(check_buffer(buf, sizeof(buf), CANARY), 1);
}

/* memset with an unaligned length. buffer must be original after that. */
TEST(memset, set_inital) {
	POINTERS_EQUAL(buf, memset(buf, TESTCHAR, 5));
	CHECK_EQUAL(check_buffer(buf, 5, TESTCHAR), 1);
	CHECK_EQUAL(check_buffer(buf+ 5, BUFFER_SIZE - 5, CANARY), 1);
}

/* memset with an unaligned start and length. buffer must be original outside.*/
TEST(memset, set_middle) {
	POINTERS_EQUAL(buf + 5, memset(buf + 5, TESTCHAR, 7));
	CHECK_EQUAL(check_buffer(buf, 5, CANARY), 1);
	CHECK_EQUAL(check_buffer(buf + 5, 7, TESTCHAR), 1);
	CHECK_EQUAL(check_buffer(buf + 5 + 7, BUFFER_SIZE - 5 - 7, CANARY), 1);
}

/* memset the whole buffer. */
TEST(memset, set_all) {
	POINTERS_EQUAL(buf, memset(buf, TESTCHAR, sizeof(buf)));
	CHECK_EQUAL(check_buffer(buf, sizeof(buf), TESTCHAR), 1);
}
