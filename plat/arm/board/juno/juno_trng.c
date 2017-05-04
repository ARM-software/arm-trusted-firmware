/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <mmio.h>
#include <string.h>
#include <utils_def.h>
#include "juno_def.h"

#define NSAMPLE_CLOCKS	1 /* min 1 cycle, max 231 cycles */
#define NRETRIES	5

static inline int output_valid(void)
{
	int i;

	for (i = 0; i < NRETRIES; i++) {
		uint32_t val;

		val = mmio_read_32(TRNG_BASE + TRNG_STATUS);
		if (val & 1U)
			break;
	}
	if (i >= NRETRIES)
		return 0; /* No output data available. */
	return 1;
}

/*
 * This function fills `buf` with `len` bytes of entropy.
 * It uses the Trusted Entropy Source peripheral on Juno.
 * Returns 0 when the buffer has been filled with entropy
 * successfully and -1 otherwise.
 */
int juno_getentropy(void *buf, size_t len)
{
	uint8_t *bp = buf;

	assert(buf);
	assert(len);
	assert(!check_uptr_overflow((uintptr_t)bp, len));

	/* Disable interrupt mode. */
	mmio_write_32(TRNG_BASE + TRNG_INTMASK, 0);
	/* Program TRNG to sample for `NSAMPLE_CLOCKS`. */
	mmio_write_32(TRNG_BASE + TRNG_CONFIG, NSAMPLE_CLOCKS);

	while (len > 0) {
		int i;

		/* Start TRNG. */
		mmio_write_32(TRNG_BASE + TRNG_CONTROL, 1);

		/* Check if output is valid. */
		if (!output_valid())
			return -1;

		/* Fill entropy buffer. */
		for (i = 0; i < TRNG_NOUTPUTS; i++) {
			size_t n;
			uint32_t val;

			val = mmio_read_32(TRNG_BASE + i * sizeof(uint32_t));
			n = MIN(len, sizeof(uint32_t));
			memcpy(bp, &val, n);
			bp += n;
			len -= n;
			if (len == 0)
				break;
		}

		/* Reset TRNG outputs. */
		mmio_write_32(TRNG_BASE + TRNG_STATUS, 1);
	}

	return 0;
}
