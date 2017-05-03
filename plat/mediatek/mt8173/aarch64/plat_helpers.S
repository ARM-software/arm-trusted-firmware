/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <mt8173_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_report_exception
	.globl	platform_is_primary_cpu
	.globl  plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* MT8173 Oak does not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

func platform_is_primary_cpu
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #MT8173_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc platform_is_primary_cpu

#if !ENABLE_PLAT_COMPAT
	/* -----------------------------------------------------
	 * unsigned int plat_my_core_pos(void);
	 *
	 * result: CorePos = CoreId + (ClusterId << 2)
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs     x0, mpidr_el1
	and     x1, x0, #MPIDR_CPU_MASK
	and     x0, x0, #MPIDR_CLUSTER_MASK
	add     x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos
#endif

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, MT8173_UART0_BASE
	mov_imm	x1, MT8173_UART_CLOCK
	mov_imm	x2, MT8173_BAUDRATE
	b	console_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm x1, MT8173_UART0_BASE
	b	console_core_putc
endfunc plat_crash_console_putc
