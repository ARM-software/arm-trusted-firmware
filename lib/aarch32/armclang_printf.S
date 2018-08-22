/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

/* Symbols needed by armclang */

	.globl __0printf
	.globl __1printf
	.globl __2printf

func __0printf
__1printf:
__2printf:
        b	printf
endfunc __0printf
