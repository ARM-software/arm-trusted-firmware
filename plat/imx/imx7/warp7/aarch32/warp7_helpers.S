/*
 * Copyright (c) Linaro 2018 Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

	.globl	platform_mem_init
	.globl	plat_get_my_entrypoint

	/* ---------------------------------------------
	 * int plat_mem_init(void)
	 * Function to initialize memory.
	 * The HAB hands off the DDR controller already
	 * setup and ready to use.
	 * Implement the mandatory function as a NOP
	 * ---------------------------------------------
	 */
func platform_mem_init
	bx	lr
endfunc platform_mem_init

func plat_get_my_entrypoint
	mov	r0, #0
	bx	lr
endfunc plat_get_my_entrypoint

