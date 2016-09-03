/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	plat_get_my_entrypoint

func plat_get_my_entrypoint
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint
