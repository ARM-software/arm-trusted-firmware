/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant GIC and CCI registers
	 * whenever an unhandled exception is taken in
	 * BL31.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	.endm

#endif /* PLAT_MACROS_S */
