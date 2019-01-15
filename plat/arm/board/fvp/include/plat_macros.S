/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

#include <arm_macros.S>
#include <platform_def.h>

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant GIC registers whenever an
	 * unhandled exception is taken in BL31.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	/*
	 * Detect if we're using the base memory map or
	 * the legacy VE memory map
	 */
	mov_imm	x0, (V2M_SYSREGS_BASE + V2M_SYS_ID)
	ldr	w16, [x0]
	/* Extract BLD (12th - 15th bits) from the SYS_ID */
	ubfx	x16, x16, #V2M_SYS_ID_BLD_SHIFT, #4
	/* Check if VE mmap */
	cmp	w16, #BLD_GIC_VE_MMAP
	b.eq	use_ve_mmap
	/* Assume Base Cortex mmap */
	mov_imm	x17, BASE_GICC_BASE
	mov_imm	x16, BASE_GICD_BASE
	b	print_gic_regs
use_ve_mmap:
	mov_imm	x17, VE_GICC_BASE
	mov_imm	x16, VE_GICD_BASE
print_gic_regs:
	arm_print_gic_regs
	.endm

#endif /* PLAT_MACROS_S */
