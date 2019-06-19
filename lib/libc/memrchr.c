/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#undef memrchr

void *memrchr(const void *src, int c, size_t len)
{
	const unsigned char *s = src + (len - 1);

	while (len--) {
		if (*s == (unsigned char)c) {
			return (void*) s;
		}

		s--;
	}

	return NULL;
}
