/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.global	el2_exit

/* ------------------------------------------------------------------
 * The mechanism, from el3_exit, is not used in this v8-R64 implementation.
 * ------------------------------------------------------------------
 */
func el2_exit
	exception_return
endfunc el2_exit
