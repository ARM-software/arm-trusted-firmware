/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <stdint.h>

/*
 * Instruction pointer authentication key A. The low 64-bit are at [0], and the
 * high bits at [1].
 */
uint64_t plat_apiakey[2];

/*
 * This is only a toy implementation to generate a seemingly random 128-bit key
 * from sp and x30 values. A production system must re-implement this function
 * to generate keys from a reliable randomness source.
 */
uint64_t *plat_init_apiakey(void)
{
	uintptr_t return_addr = (uintptr_t)__builtin_return_address(0U);
	uintptr_t frame_addr = (uintptr_t)__builtin_frame_address(0U);

	plat_apiakey[0] = (return_addr << 13) ^ frame_addr;
	plat_apiakey[1] = (frame_addr << 15) ^ return_addr;

	return plat_apiakey;
}
