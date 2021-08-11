/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

#include <arm_macros.S>

/* ---------------------------------------------
 * The below required platform porting macro
 * prints out relevant platform registers
 * whenever an unhandled exception is taken in
 * BL31.
 *
 * There are currently no platform specific regs
 * to print.
 * ---------------------------------------------
 */
	.macro plat_crash_print_regs
	.endm

#endif /* PLAT_MACROS_S */
