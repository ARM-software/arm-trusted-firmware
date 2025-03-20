/*
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

int strcpy_secure(char *restrict dest, size_t dest_size, const char *restrict src)
{
	/* Check for null pointers */
	if ((dest == NULL) || (src == NULL)) {
		return -EINVAL;
	}

	/* Check the destination size valid range */
	if (dest_size == 0) {
		return -ERANGE;
	}

	/* Calculate the length of the source string */
	size_t src_len = strnlen_secure(src, dest_size);

	/* Check if the source string fits in the destination buffer */
	if (src_len >= dest_size) {
		/* Set destination to an empty string */
		dest[0] = '\0';
		return -ERANGE;
	}

	/* Copy the source string to the destination */
	for (dest[src_len] = '\0'; src_len > 0; src_len--) {
		dest[src_len - 1] = src[src_len - 1];
	}

	return 0;
}
