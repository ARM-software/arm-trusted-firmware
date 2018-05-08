/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdarg.h>

/* Choose max of 128 chars for now. */
#define PRINT_BUFFER_SIZE 128
int printf(const char *fmt, ...)
{
	va_list args;
	char buf[PRINT_BUFFER_SIZE];
	int count;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	va_end(args);

	/* Use putchar directly as 'puts()' adds a newline. */
	buf[PRINT_BUFFER_SIZE - 1] = '\0';
	count = 0;
	while (buf[count])
	{
		if (putchar(buf[count]) != EOF) {
			count++;
		} else {
			count = EOF;
			break;
		}
	}

	return count;
}
