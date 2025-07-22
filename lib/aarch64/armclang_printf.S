/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

/* Symbols needed by armclang */

	.globl __0printf
	.globl __1printf
	.globl __2printf
	.globl __2snprintf

func __0printf
	b	printf
endfunc __0printf

func __1printf
	b	printf
endfunc __1printf

func __2printf
	b	printf
endfunc __2printf

func __2snprintf
	b	snprintf
endfunc	__2snprintf
