/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

	.globl	plat_crash_console_flush
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	platform_mem_init
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_reset_handler
	.globl	plat_calc_core_pos

	/* -----------------------------------------------------
	 * unsigned int plat_my_core_pos(void);
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_calc_core_pos(u_register_t mpidr);
	 * -----------------------------------------------------
	 */
func plat_calc_core_pos
	and	x0, x0, #MPIDR_CPU_MASK
	ret
endfunc plat_calc_core_pos

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #AML_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ---------------------------------------------
	 * void platform_mem_init(void);
	 * ---------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, AML_UART0_AO_BASE
	mov_imm	x1, AML_UART0_AO_CLK_IN_HZ
	mov_imm	x2, AML_UART_BAUDRATE
	b	console_meson_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, AML_UART0_AO_BASE
	b	console_meson_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * void plat_crash_console_flush()
	 * Out : void.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, AML_UART0_AO_BASE
	b	console_meson_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------
	 * void plat_reset_handler(void);
	 * ---------------------------------------------
	 */
func plat_reset_handler
	ret
endfunc plat_reset_handler
