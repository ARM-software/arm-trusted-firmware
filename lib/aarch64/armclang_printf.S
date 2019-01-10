/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

/* Symbols needed by armclang */

	.globl __0printf
	.globl __1printf
	.globl __2printf

func __0printf
	b	printf
endfunc __0printf

func __1printf
	b	printf
endfunc __1printf

func __2printf
	b	printf
endfunc __2printf
