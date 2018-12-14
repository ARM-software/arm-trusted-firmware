/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <plat/common/platform.h>

/*
 * Canary value used by the compiler runtime checks to detect stack corruption.
 *
 * Force the canary to be in .data to allow predictable memory layout relatively
 * to the stacks.
 */
u_register_t  __attribute__((section(".data.stack_protector_canary")))
	__stack_chk_guard = (u_register_t) 3288484550995823360ULL;

/*
 * Function called when the stack's canary check fails, which means the stack
 * was corrupted. It must not return.
 */
void __dead2 __stack_chk_fail(void)
{
#if DEBUG
	ERROR("Stack corruption detected\n");
#endif
	panic();
}

