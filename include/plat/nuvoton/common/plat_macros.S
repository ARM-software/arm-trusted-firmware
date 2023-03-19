/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (c) 2017-2023 Nuvoton Technology Corp.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

#include <arm_macros.S>
#include <cci_macros.S>
#include <platform_def.h>

/*
 * The below macro prints out relevant GIC
 * registers whenever an unhandled exception is
 * taken in BL3-1.
 * Clobbers: x0 - x10, x16, x17, sp
 */
.macro plat_print_gic_regs
mov_imm	x17, BASE_GICC_BASE
mov_imm	x16, BASE_GICD_BASE
arm_print_gic_regs
.endm

/*
 * the below macros print out relevant interconnect
 * registers whenever an unhandled exception is
 * taken in BL3-1
 */
.macro plat_print_interconnect_regs
	/* TODO */
.endm

/*
 * The below required platform porting macro
 * prints out relevant platform registers
 * whenever an unhandled exception is taken in
 * BL31.
 */
.macro plat_crash_print_regs
	/* TODO */
.endm

#endif /* PLAT_MACROS_S */
