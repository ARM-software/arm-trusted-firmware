/*
 * Copyright (c) 2013-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>

void *memmove(void *dst, const void *src, size_t len)
{
	if (dst == src || len == 0)
		return dst;

	/*
	 * Assuming pointers convert to linear byte addresses, this unsigned
	 * difference is less than `len` only when `dst` starts inside the source
	 * range at a higher address. Copy backwards in that case, and forwards
	 * otherwise, including when the ranges overlap with `dst` below `src`.
	 */
	if (((uintptr_t)dst - (uintptr_t)src) >= len)  {
		/* copy forward... */
		const char *end = dst + len;
		const char *s = (const char *)src;
		char *d = (char *)dst;
		while (d != end) {
			*d++ = *s++;
		}
	} else {
		/* copy backwards... */
		const char *end = dst;
		const char *s = (const char *)src + len;
		char *d = (char *)dst + len;
		while (d != end) {
			*--d = *--s;
		}
	}

	return dst;
}
