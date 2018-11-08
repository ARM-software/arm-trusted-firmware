/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CSS_MACROS_S
#define CSS_MACROS_S

#include <arm_macros.S>
#include <platform_def.h>

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant GIC registers whenever an
	 * unhandled exception is taken in BL31.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro css_print_gic_regs
	mov_imm	x16, PLAT_ARM_GICD_BASE
	mov_imm	x17, PLAT_ARM_GICC_BASE
	arm_print_gic_regs
	.endm

#endif /* CSS_MACROS_S */
