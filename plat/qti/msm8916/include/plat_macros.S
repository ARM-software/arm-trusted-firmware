/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

#include <arm_macros.S>

#include <msm8916_mmap.h>

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant GIC registers whenever
	 * an unhandled exception is taken in BL31.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	mov_imm	x16, APCS_QGIC2_GICD
	mov_imm	x17, APCS_QGIC2_GICC
	arm_print_gic_regs
	.endm

#endif /* PLAT_MACROS_S */
