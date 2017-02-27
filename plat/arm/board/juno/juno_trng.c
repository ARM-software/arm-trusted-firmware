/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <mmio.h>
#include <string.h>
#include <utils.h>
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
