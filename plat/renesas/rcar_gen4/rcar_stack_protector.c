/*
 * Copyright (c) 2021-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>

#define RANDOM_CANARY_VALUE	((u_register_t)0xDFF5FC8A720E205EULL)

u_register_t plat_get_stack_protector_canary(void)
{
	uintptr_t val1 = (uintptr_t)__builtin_return_address(0U);
	uintptr_t val2 = (uintptr_t)__builtin_frame_address(0U);
	u_register_t cnt;
	u_register_t seed;
	u_register_t mul;
	u_register_t ret;

	cnt = read_cntpct_el0();
	seed = (cnt ^ RANDOM_CANARY_VALUE) & ULONG_MAX;
	ret = seed;

	INFO("seed value: 0x%16lx    cnt: 0x%16lx\n", seed, cnt);

	if ((ULONG_MAX / val1) > seed) {
		mul = (u_register_t)(val1 * seed);
		if ((ULONG_MAX - mul) > val2) {
			ret = mul + val2;
		}
	}

	INFO("canary value: 0x%lx    cnt: 0x%16lx\n", ret, read_cntpct_el0());

	return ret;
}
