/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

void *memchr(const void *src, int c, size_t len)
{
	const char *s = src;

	while (len--) {
		if (*s == c)
			return (void *) s;
		s++;
	}

	return NULL;
}
