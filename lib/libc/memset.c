/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>
#include <stdint.h>

void *memset(void *dst, int val, size_t count)
{
	uint8_t *ptr = dst;
	uint64_t *ptr64;
	uint64_t fill = (unsigned char)val;

	/* Simplify code below by making sure we write at least one byte. */
	if (count == 0U) {
		return dst;
	}

	/* Handle the first part, until the pointer becomes 64-bit aligned. */
	while (((uintptr_t)ptr & 7U) != 0U) {
		*ptr = (uint8_t)val;
		ptr++;
		if (--count == 0U) {
			return dst;
		}
	}

	/* Duplicate the fill byte to the rest of the 64-bit word. */
	fill |= fill << 8;
	fill |= fill << 16;
	fill |= fill << 32;

	/* Use 64-bit writes for as long as possible. */
	ptr64 = (uint64_t *)ptr;
	for (; count >= 8U; count -= 8) {
		*ptr64 = fill;
		ptr64++;
	}

	/* Handle the remaining part byte-per-byte. */
	ptr = (uint8_t *)ptr64;
	while (count-- > 0U)  {
		*ptr = (uint8_t)val;
		ptr++;
	}

	return dst;
}
