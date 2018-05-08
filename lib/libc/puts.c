/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

int puts(const char *s)
{
	int count = 0;
	while(*s) {
		if (putchar(*s++) == EOF)
			return EOF;
		count++;
	}

	/* According to the puts(3) manpage, the function should write a
	 * trailing newline.
	 */
	if (putchar('\n') == EOF)
		return EOF;

	return count + 1;
}
