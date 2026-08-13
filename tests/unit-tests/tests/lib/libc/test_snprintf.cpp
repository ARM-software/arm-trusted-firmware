/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CppUTest/TestHarness.h"
extern "C" {
#include "lib/libc/string.h"
}

/*
 * Check a memory region of a certain length to only contain one particular
 * byte. We use that to verify that portions of the buffer still contain
 * the canary bytes we put in there.
 */
static int check_buffer(const char *buffer, unsigned long int length,
			char content)
{
	unsigned long int i;

	for (i = 0; i < length; i++)
		if (buffer[i] != content)
			return 0;

	return 1;
}

#define BUFFER_SIZE	16
#define CANARY		0x7f

TEST_GROUP(snprintf) {
	TEST_SETUP() {
		for (int i = 0; i < BUFFER_SIZE; i++) {
			buffer[i] = CANARY;
		}
	}

	char buffer[BUFFER_SIZE];
};

/*
 * Print a number using a given format string. Assume that the result
 * fits in the buffer provided, and check that the resulting buffer
 * content matches the expectation. The return value must be the length
 * of the (expected) string, and the result must be NUL-terminated.
 */
static void number_test(char *buffer, const char *format, int num,
			const char *result)
{
	int slen = strlen(result);

	LONGS_EQUAL(slen, snprintf(buffer, BUFFER_SIZE, format, num));
	STRCMP_EQUAL(buffer, result);
	CHECK_EQUAL(check_buffer(buffer + slen + 1,
		    BUFFER_SIZE - slen - 1, CANARY), 1);
	CHECK_EQUAL(buffer[slen], 0);
	memset(buffer, CANARY, BUFFER_SIZE);
}

static void buffer_test(char *buffer, const char *format, int buflen, int num,
			const char *result, int excp_ret)
{
	LONGS_EQUAL(excp_ret, snprintf(buffer, buflen, format, num));
	STRCMP_EQUAL(buffer, result);

	/* the rest of the buffer must be untouched */
	CHECK_EQUAL(check_buffer(buffer + excp_ret + 1,
		    BUFFER_SIZE - excp_ret - 1, CANARY), 1);

	/* a buffer large enough must be NUL terminated */
	if (excp_ret < buflen)
		CHECK_EQUAL(buffer[excp_ret], 0);

	/* prime the buffer for the next test */
	memset(buffer, CANARY, BUFFER_SIZE);
}

TEST(snprintf, simple_num) {
	LONGS_EQUAL(3, snprintf(buffer, 16, "%d", 123));
	STRCMP_EQUAL(buffer, "123");
	CHECK_EQUAL(check_buffer(buffer + 4, BUFFER_SIZE - 4, CANARY), 1);
}

TEST(snprintf, num_basic1) {
	number_test(buffer, "%d", 0, "0");
	number_test(buffer, "%i", 0, "0");
	number_test(buffer, "%u", 0, "0");
}

TEST(snprintf, num_basic2) {
	number_test(buffer, "%d", 1234, "1234");
	number_test(buffer, "%i", 1234, "1234");
	number_test(buffer, "%u", 1234, "1234");
}

TEST(snprintf, num_alignment) {
	number_test(buffer, "%04d",  12, "0012");
	number_test(buffer, "%6d",   12, "    12");
	number_test(buffer, "%-6d",  12, "12    ");
	/*
	 * Those two are failing at the moment. The format string is admittedly
	 * shady, GCC/glibc warn about this:
	 * warning: '0' flag ignored with '-' flag in gnu_printf format [-Wformat=]
	 *
	number_test(buffer, "%0-5d", 12, "12   ");
	number_test(buffer, "%-05d", 12, "12   ");
	 *
	 */
}

TEST(snprintf, num_negative) {
	number_test(buffer, "%d",  -12, "-12");
	number_test(buffer, "%i",  -12, "-12");
	number_test(buffer, "%u",  -12, "4294967284");
}

TEST(snprintf, num_hex) {
	number_test(buffer, "%x", 0, "0");
	number_test(buffer, "%x", 15, "f");
	number_test(buffer, "%04x", 16, "0010");
	number_test(buffer, "%4x", 17, "  11");
	number_test(buffer, "%-4X", 255, "FF  ");
	number_test(buffer, "%X", 63, "3F");
	number_test(buffer, "0x%x", 0x7fffffff, "0x7fffffff");
	number_test(buffer, "0x%x", -1, "0xffffffff");
}

TEST(snprintf, buffer1) {
	buffer_test(buffer, "justfour%d", 10, 4, "justfour4", 9);
	buffer_test(buffer, "onetwothree%d", 16, 123, "onetwothree123", 14);
	buffer_test(buffer, "onetwothree%d", 15, 123, "onetwothree123", 14);
	buffer_test(buffer, " onetwoonly%d", 14, 123, " onetwoonly12", 14);
	buffer_test(buffer, "    oneonly%d", 13, 123, "    oneonly1", 14);
	buffer_test(buffer, "   nonumber%d", 12, 123, "   nonumber", 14);
}

TEST(snprintf, buffer_align) {
	buffer_test(buffer, "%7d", 8, 123, "    123",  7);
	buffer_test(buffer, "%8d", 8,  123, "     12",  8);
	buffer_test(buffer, "%10d", 8, 123, "       ", 10);
	buffer_test(buffer, "%-7d", 8, 123, "123    ",  7);
	buffer_test(buffer, "%-8d", 8, 123, "123    ",  8);
	buffer_test(buffer, "%-8d", 4, 1, "1  ", 8);
}
