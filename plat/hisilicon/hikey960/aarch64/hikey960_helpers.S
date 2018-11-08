/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_a53.h>
#include <cortex_a73.h>
#include "../hikey960_def.h"

	.globl	plat_my_core_pos
	.globl	platform_mem_init
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_report_exception
	.globl	plat_reset_handler
	.globl	clr_ex
	.globl	nop

func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * void platform_mem_init(void);
	 *
	 * We don't need to carry out any memory initialization
	 * on HIKEY. The Secure RAM is accessible straight away.
	 * -----------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, CRASH_CONSOLE_BASE
	mov_imm	x1, PL011_UART_CLK_IN_HZ
	mov_imm	x2, PL011_BAUDRATE
	b	console_pl011_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, CRASH_CONSOLE_BASE
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * int plat_crash_console_flush()
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, CRASH_CONSOLE_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------
	 * void plat_report_exception(unsigned int type)
	 * Function to report an unhandled exception
	 * with platform-specific means.
	 * On HIKEY platform, it updates the LEDs
	 * to indicate where we are
	 * ---------------------------------------------
	 */
func plat_report_exception
	mov	x8, x30

	/* Turn on LED according to x0 (0 -- f) */
	ldr	x2, =0xf7020000
	and	x1, x0, #1
	str	w1, [x2, #4]
	and	x1, x0, #2
	str	w1, [x2, #8]
	and	x1, x0, #4
	str	w1, [x2, #16]
	and	x1, x0, #8
	str	w1, [x2, #32]

	mrs	x2, currentel
	and	x2, x2, #0x0c
	/* Check EL1 */
	cmp	x2, #0x04
	beq	plat_report_el1

	adr	x4, plat_err_str
	bl	asm_print_str

	adr	x4, esr_el3_str
	bl	asm_print_str

	mrs	x4, esr_el3
	bl	asm_print_hex

	adr	x4, elr_el3_str
	bl	asm_print_str

	mrs	x4, elr_el3
	bl	asm_print_hex
	b	plat_report_end

plat_report_el1:
	adr	x4, plat_err_str
	bl	asm_print_str

	adr	x4, esr_el1_str
	bl	asm_print_str

	mrs	x4, esr_el1
	bl	asm_print_hex

	adr	x4, elr_el1_str
	bl	asm_print_str

	mrs	x4, elr_el1
	bl	asm_print_hex
plat_report_end:
	mov	x30, x8
	ret
endfunc plat_report_exception

	/* -----------------------------------------------------
	 * void plat_reset_handler(void);
	 * -----------------------------------------------------
	 */
func plat_reset_handler
	ret
endfunc plat_reset_handler

	/* -----------------------------------------------------
	 * void clrex(void);
	 * -----------------------------------------------------
	 */
func clr_ex
	clrex
	ret
endfunc clr_ex

	/* -----------------------------------------------------
	 * void nop(void);
	 * -----------------------------------------------------
	 */
func nop
	nop
	ret
endfunc nop

.section .rodata.rev_err_str, "aS"
plat_err_str:
	.asciz "\nPlatform exception reporting:"
esr_el3_str:
	.asciz "\nESR_EL3: "
elr_el3_str:
	.asciz "\nELR_EL3: "
esr_el1_str:
	.asciz "\nESR_EL1: "
elr_el1_str:
	.asciz "\nELR_EL1: "
