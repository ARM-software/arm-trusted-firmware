/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * the below macros print out relevant GIC
 * registers whenever an unhandled exception is
 * taken in BL3-1
 */
.macro plat_print_gic_regs
	/* TODO */
.endm

/*
 * the below macros print out relevant interconnect
 * registers whenever an unhandled exception is
 * taken in BL3-1
 */
.macro plat_print_interconnect_regs
	/* TODO */
.endm

/* ---------------------------------------------
 * The below required platform porting macro
 * prints out relevant platform registers
 * whenever an unhandled exception is taken in
 * BL31.
 * ---------------------------------------------
 */
.macro plat_crash_print_regs
      /* TODO */
.endm
