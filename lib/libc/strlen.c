/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string_private.h>

size_t strlen(const char *s)
{
	const char *cursor = s;

	while (*cursor)
		cursor++;

	return cursor - s;
}
