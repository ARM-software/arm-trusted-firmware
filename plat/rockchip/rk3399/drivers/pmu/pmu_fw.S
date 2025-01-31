/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* convoluted way to make sure that the define is pasted just the right way */
.macro INCBIN file sym sec
	.section \sec, "a"
	.global \sym
	.type \sym, @object
	.align 4
\sym :
	.incbin "\file"
	.size \sym , .-\sym
	.global \sym\()_end
\sym\()_end :
.endm

INCBIN RK3399M0FW, "rk3399m0_bin", ".sram.incbin"
INCBIN RK3399M0PMUFW, "rk3399m0pmu_bin", ".pmusram.incbin"
