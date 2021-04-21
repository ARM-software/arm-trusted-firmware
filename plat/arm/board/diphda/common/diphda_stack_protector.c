/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <plat/common/platform.h>

static uint32_t plat_generate_random_number(void)
{
	uintptr_t return_addr = (uintptr_t)__builtin_return_address(0U);
	uintptr_t frame_addr = (uintptr_t)__builtin_frame_address(0U);
	uint64_t cntpct = read_cntpct_el0();

	/* Generate 32-bit pattern: saving the 2 least significant bytes
	 * in random_lo and random_hi
	 */
	uint16_t random_lo = (uint16_t)(
			(((uint64_t)return_addr) << 13) ^ frame_addr ^ cntpct
			);

	uint16_t random_hi = (uint16_t)(
			(((uint64_t)frame_addr) << 15) ^ return_addr ^ cntpct
			);

	return (((uint32_t)random_hi) << 16) | random_lo;
}

u_register_t plat_get_stack_protector_canary(void)
{
	return  plat_generate_random_number(); /* a 32-bit pattern returned */
}
