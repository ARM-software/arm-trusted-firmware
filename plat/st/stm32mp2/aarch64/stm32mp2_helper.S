/*
 * Copyright (c) 2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

#include <platform_def.h>

	.globl	platform_mem_init
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	plat_crash_console_init
	.globl	plat_crash_console_flush
	.globl	plat_crash_console_putc

func platform_mem_init
	/* Nothing to do, don't need to init SYSRAM */
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * Set secondary core in WFI waiting for core reset.
	 * ---------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	dsb	sy
	wfi
	/* This shouldn't be reached */
	b	.
endfunc plat_secondary_cold_boot_setup

	/* ----------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * This function checks if this is the primary CPU
	 * ----------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CPU_MASK)
	cmp	x0, #STM32MP_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 *
	 * Initialize the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_init
endfunc plat_crash_console_init

func plat_crash_console_flush
endfunc plat_crash_console_flush

func plat_crash_console_putc
endfunc plat_crash_console_putc

