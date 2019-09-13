/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <cdefs.h>
#include <stdint.h>

/*
 * This is only a toy implementation to generate a seemingly random
 * 128-bit key from sp, x30 and cntpct_el0 values.
 * A production system must re-implement this function to generate
 * keys from a reliable randomness source.
 */
uint128_t plat_init_apkey(void)
{
	uint64_t return_addr = (uint64_t)__builtin_return_address(0U);
	uint64_t frame_addr = (uint64_t)__builtin_frame_address(0U);
	uint64_t cntpct = read_cntpct_el0();

	/* Generate 128-bit key */
	uint64_t key_lo = (return_addr << 13) ^ frame_addr ^ cntpct;
	uint64_t key_hi = (frame_addr << 15) ^ return_addr ^ cntpct;

	return ((uint128_t)(key_hi) << 64) | key_lo;
}
