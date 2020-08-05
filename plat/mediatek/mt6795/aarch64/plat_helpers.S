/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_report_exception
	.globl	platform_is_primary_cpu
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	platform_mem_init


	.macro crash_ram_log
	 /*
	 * Check teearg->atf_log_buf_size.
	 * Exit if atf_log_buf_size equals 0
	 */
	adr	x2, ptr_atf_crash_flag
	ldr	x2, [x2]
	/* exit if ptr_atf_crash_flag equals NULL */
	cbz x2, exit_putc

	/*
	 * set atf crash magic number
	 */
1:
	adr	x2, ptr_atf_crash_flag
	ldr	x2, [x2]
	mov_imm x1, 0xdead1abf
	/* p_atf_log_ctrl->atf_crash_flag = 0xdead1abf */
	str	w1, [x2]
	/* can't use w3 return addr, w4, start of buffer addr */
	ldr	w2, [x2]
	cmp	w2, w1
	b.ne	1b

	/*
	 * get cpu id
	 */
	mrs	x1, mpidr_el1
	/* refer to platform_get_core_pos */
	and	x2, x1, #MPIDR_CPU_MASK
	and	x1, x1, #MPIDR_CLUSTER_MASK
	/* x1 = cpu id (cpu id = aff0 + aff1*4 ) */
	add	x1, x2, x1, LSR #6

	adr	x2, ptr_atf_except_write_pos_per_cpu
	ldr	x2, [x2]
	/*
	 * plus (cpu_id * 8)-->
	 * &p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id]
	 * x2 = &p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id];
	 */
	add x2, x2, x1, LSL # 3
	/* log write */
	/* w1 = p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id] */
	ldr	x1, [x2]
	/* *x1 = w0-->
	 *  *(p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id]) = c)
	 */
	strb	w0, [x1]
	/* w1++ */
	add	x1, x1, #1
	/* p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id] = w1 */
	str	x1, [x2]
exit_putc:
	.endm

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
	/* Do not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

func platform_is_primary_cpu
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc platform_is_primary_cpu

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, UART0_BASE
	mov_imm	x1, UART_CLOCK
	mov_imm	x2, UART_BAUDRATE
	b	console_init
	ret
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm x1, UART0_BASE
	b	console_core_putc
	ret
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * void plat_crash_console_flush(int c)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : void.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, UART0_BASE
	b	console_core_flush
endfunc plat_crash_console_flush

	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Any memory init, relocation to be done before the
	 * platform boots. Called very early in the boot process.
	 * --------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

