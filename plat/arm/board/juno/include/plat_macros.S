/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __PLAT_MACROS_S__
#define __PLAT_MACROS_S__

#include <cci_macros.S>
#include <css_macros.S>

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant platform registers
	 * whenever an unhandled exception is taken in
	 * BL31.
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	css_print_gic_regs
	print_cci_regs
	.endm

#endif /* __PLAT_MACROS_S__ */
