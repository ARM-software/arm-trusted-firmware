/*
 * Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string_private.h>

void *memchr(const void *src, int c, size_t len)
{
	const unsigned char *s = src;

	while (len--) {
		if (*s == (unsigned char)c)
			return (void *) s;
		s++;
	}

	return NULL;
}
