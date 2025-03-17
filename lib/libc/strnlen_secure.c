/*
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

size_t strnlen_secure(const char *str, size_t maxlen)
{
	size_t len = 0;

	if (str == NULL) {
		return 0;
	}

	while ((len < maxlen) && (str[len] != '\0')) {
		len++;
	}

	return len;
}
