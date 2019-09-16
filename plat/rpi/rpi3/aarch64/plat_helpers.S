/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

#include "../rpi3_hw.h"

	.globl	plat_crash_console_flush
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	platform_mem_init
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_reset_handler
	.globl	plat_rpi3_calc_core_pos
	.globl	plat_secondary_cold_boot_setup

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *
	 *  This function uses the plat_rpi3_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_rpi3_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_rpi3_calc_core_pos(u_register_t mpidr);
	 *
	 *  CorePos = (ClusterId * 4) + CoreId
	 * -----------------------------------------------------
	 */
func plat_rpi3_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_rpi3_calc_core_pos

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #RPI3_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

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
	/* Calculate address of our hold entry */
	bl	plat_my_core_pos
	lsl	x0, x0, #3
	mov_imm	x2, PLAT_RPI3_TM_HOLD_BASE
	add	x0, x0, x2

	/*
	 * This code runs way before requesting the warmboot of this core,
	 * so it is possible to clear the mailbox before getting a request
	 * to boot.
	 */
	mov	x1, PLAT_RPI3_TM_HOLD_STATE_WAIT
	str	x1,[x0]

	/* Wait until we have a go */
poll_mailbox:
	wfe
	ldr	x1, [x0]
	cmp	x1, PLAT_RPI3_TM_HOLD_STATE_GO
	bne	poll_mailbox

	/* Jump to the provided entrypoint */
	mov_imm	x0, PLAT_RPI3_TM_ENTRYPOINT
	ldr	x1, [x0]
	br	x1
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------------------------------
	 * uintptr_t plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and a warm
	 * boot.
	 *
	 * This functions returns:
	 *  - 0 for a cold boot.
	 *  - Any other value for a warm boot.
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	/* TODO: support warm boot */
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint

	/* ---------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * No need to carry out any memory initialization.
	 * ---------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x3
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, PLAT_RPI3_UART_BASE
	mov_imm	x1, PLAT_RPI3_UART_CLK_IN_HZ
	mov_imm	x2, PLAT_RPI3_UART_BAUDRATE
	b	console_16550_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, PLAT_RPI3_UART_BASE
	b	console_16550_core_putc
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
	mov_imm	x0, PLAT_RPI3_UART_BASE
	b	console_16550_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------
	 * void plat_reset_handler(void);
	 * ---------------------------------------------
	 */
func plat_reset_handler
	/* use the 19.2 MHz clock for the architected timer */
	mov	x0, #RPI3_INTC_BASE_ADDRESS
	mov	w1, #0x80000000
	str	wzr, [x0, #RPI3_INTC_CONTROL_OFFSET]
	str	w1, [x0, #RPI3_INTC_PRESCALER_OFFSET]
	ret
endfunc plat_reset_handler
