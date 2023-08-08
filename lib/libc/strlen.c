/*
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

size_t strlen(const char *s)
{
	const char *cursor = s;

	while (*cursor)
		cursor++;

	return cursor - s;
}
