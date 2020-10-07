/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <platform_def.h>

#include "juno_decl.h"

#define NSAMPLE_CLOCKS	1 /* min 1 cycle, max 231 cycles */
#define NRETRIES	5

/* initialised to false */
static bool juno_trng_initialized;

static bool output_valid(void)
{
	int i;

	for (i = 0; i < NRETRIES; i++) {
		uint32_t val;

		val = mmio_read_32(TRNG_BASE + TRNG_STATUS);
		if (val & 1U)
			return true;
	}
	return false; /* No output data available. */
}

/*
 * This function fills `buf` with 8 bytes of entropy.
 * It uses the Trusted Entropy Source peripheral on Juno.
 * Returns 'true' when the buffer has been filled with entropy
 * successfully, or 'false' otherwise.
 */
bool juno_getentropy(uint64_t *buf)
{
	uint64_t ret;

	assert(buf);
	assert(!check_uptr_overflow((uintptr_t)buf, sizeof(*buf)));

	if (!juno_trng_initialized) {
		/* Disable interrupt mode. */
		mmio_write_32(TRNG_BASE + TRNG_INTMASK, 0);
		/* Program TRNG to sample for `NSAMPLE_CLOCKS`. */
		mmio_write_32(TRNG_BASE + TRNG_CONFIG, NSAMPLE_CLOCKS);
		/* Abort any potentially pending sampling. */
		mmio_write_32(TRNG_BASE + TRNG_CONTROL, 2);
		/* Reset TRNG outputs. */
		mmio_write_32(TRNG_BASE + TRNG_STATUS, 1);

		juno_trng_initialized = true;
	}

	if (!output_valid()) {
		/* Start TRNG. */
		mmio_write_32(TRNG_BASE + TRNG_CONTROL, 1);

		if (!output_valid())
			return false;
	}

	/* XOR each two 32-bit registers together, combine the pairs */
	ret = mmio_read_32(TRNG_BASE + 0);
	ret ^= mmio_read_32(TRNG_BASE + 4);
	ret <<= 32;

	ret |= mmio_read_32(TRNG_BASE + 8);
	ret ^= mmio_read_32(TRNG_BASE + 12);
	*buf = ret;

	/* Acknowledge current cycle, clear output registers. */
	mmio_write_32(TRNG_BASE + TRNG_STATUS, 1);
	/* Trigger next TRNG cycle. */
	mmio_write_32(TRNG_BASE + TRNG_CONTROL, 1);

	return true;
}
