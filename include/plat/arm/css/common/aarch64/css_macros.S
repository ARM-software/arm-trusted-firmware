/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
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
	/*
	 * The ID registers will show GIC version, so it can safely be assumed
	 * that no v2 operation will be attempted on v3. Don't require the
	 * macro.
	 */
#if USE_GIC_DRIVER == 2
	mov_imm	x17, PLAT_ARM_GICC_BASE
#endif
	arm_print_gic_regs
	.endm

#endif /* CSS_MACROS_S */
