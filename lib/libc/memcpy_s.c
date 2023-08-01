/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>

int memcpy_s(void *dst, size_t dsize, void *src, size_t ssize)
{
	unsigned int *s = (unsigned int *)src;
	unsigned int *d = (unsigned int *)dst;

	/*
	 * Check source and destination size is NULL
	 */
	if ((dst == NULL) || (src == NULL)) {
		return -ENOMEM;
	}

	/*
	 * Check source and destination size validity
	 */
	if ((dsize == 0) || (ssize == 0)) {
		return -ERANGE;
	}

	/*
	 * Check both source and destination size range
	 */
	if ((ssize > dsize) || (dsize > ssize)) {
		return -EINVAL;
	}

	/*
	 * Check both source and destination address overlapping
	 * When (s > d < s + ssize)
	 * Or (d > s < d + dsize)
	 */

	if (d > s) {
		if ((d) < (s + ssize)) {
			return -EOPNOTSUPP;
		}
	}

	if (s > d) {
		if ((s) < (d + dsize)) {
			return -EOPNOTSUPP;
		}
	}

	/*
	 * Start copy process when there is no error
	 */
	while (ssize--) {
		d[ssize] = s[ssize];
	}

	return 0;
}
