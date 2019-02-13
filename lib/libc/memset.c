/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>

void *memset(void *dst, int val, size_t count)
{
	char *ptr = dst;

	while (count--)
		*ptr++ = val;

	return dst;
}
