/*
 * Copyright (c) 2023, Aspeed Technology Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <arch.h>
#include <cortex_a35.h>
#include <platform_def.h>

	.globl	platform_mem_init
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_get_syscnt_freq2
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush

/* void platform_mem_init(void); */
func platform_mem_init
	/* DRAM init. is done by preceding MCU */
	ret
endfunc platform_mem_init

/* unsigned int plat_is_my_cpu_primary(void); */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLATFORM_CORE_PRIMARY
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

/* unsigned int plat_my_core_pos(void); */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	mov	x2, #PLATFORM_CORE_COUNT_PER_CLUSTER
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	madd	x0, x0, x2, x1
	ret
endfunc plat_my_core_pos

/* void plat_secondary_cold_boot_setup (void); */
func plat_secondary_cold_boot_setup
	mov	x0, xzr
	bl	plat_my_core_pos
	mov_imm	x1, SCU_CPU_SMP_EP0
	add	x1, x1, x0, lsl #3

poll_smp_mbox_go:
	wfe
	ldr	x0, [x1]
	cmp	x0, xzr
	beq	poll_smp_mbox_go
	br	x0
endfunc plat_secondary_cold_boot_setup

/* unsigned int plat_get_syscnt_freq2(void); */
func plat_get_syscnt_freq2
	mov_imm	w0, PLAT_SYSCNT_CLKIN_HZ
	ret
endfunc plat_get_syscnt_freq2

/* int plat_crash_console_init(void); */
func plat_crash_console_init
	mov_imm	x0, CONSOLE_UART_BASE
	mov_imm	x1, CONSOLE_UART_CLKIN_HZ
	mov_imm	x2, CONSOLE_UART_BAUDRATE
	b	console_16550_core_init
endfunc plat_crash_console_init

/* int plat_crash_console_putc(int); */
func plat_crash_console_putc
	mov_imm	x1, CONSOLE_UART_BASE
	b	console_16550_core_putc
endfunc plat_crash_console_putc

/* void plat_crash_console_flush(void); */
func plat_crash_console_flush
	mov_imm	x0, CONSOLE_UART_BASE
	b	console_16550_core_flush
endfunc plat_crash_console_flush
