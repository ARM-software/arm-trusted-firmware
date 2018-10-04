/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>
#include <platform_def.h>

	.weak	plat_arm_calc_core_pos
	.weak	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_arm_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	ldcopr	r0, MPIDR
	b	plat_arm_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_arm_calc_core_pos(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ClusterId * 4) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_arm_calc_core_pos
	and	r1, r0, #MPIDR_CPU_MASK
	and	r0, r0, #MPIDR_CLUSTER_MASK
	add	r0, r1, r0, LSR #6
	bx	lr
endfunc plat_arm_calc_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : r0 - r3
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	ldr	r0, =PLAT_ARM_CRASH_UART_BASE
	ldr	r1, =PLAT_ARM_CRASH_UART_CLK_IN_HZ
	ldr	r2, =ARM_CONSOLE_BAUDRATE
	b	console_pl011_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : r1 - r2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	ldr	r1, =PLAT_ARM_CRASH_UART_BASE
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * int plat_crash_console_flush()
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	ldr	r0, =PLAT_ARM_CRASH_UART_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush
