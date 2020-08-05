/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
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
	.globl	platform_mem_init


	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_arm_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_arm_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ClusterId * 4) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_arm_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_arm_calc_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, PLAT_ARM_CRASH_UART_BASE
	mov_imm	x1, PLAT_ARM_CRASH_UART_CLK_IN_HZ
	mov_imm	x2, ARM_CONSOLE_BAUDRATE
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
	mov_imm	x1, PLAT_ARM_CRASH_UART_BASE
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * void plat_crash_console_flush()
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : void.
	 * Clobber list : r0
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, PLAT_ARM_CRASH_UART_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on ARM
	 * platforms. The Secure RAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

/*
 * Need to use coherent stack when ARM Cryptocell is used to autheticate images
 * since Cryptocell uses DMA to transfer data and it is not coherent with the
 * AP CPU.
 */
#if ARM_CRYPTOCELL_INTEG
#if defined(IMAGE_BL1) || defined(IMAGE_BL2)
	.globl	plat_get_my_stack
	.globl	plat_set_my_stack
	.local	platform_coherent_stacks

	/* -------------------------------------------------------
	 * uintptr_t plat_get_my_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function returns the stack pointer for a
	 * stack allocated in coherent memory.
	 * -------------------------------------------------------
	 */
func plat_get_my_stack
	get_up_stack platform_coherent_stacks, PLATFORM_STACK_SIZE
	ret
endfunc plat_get_my_stack

	/* -------------------------------------------------------
	 * void plat_set_my_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function sets the stack pointer to a stack
	 * allocated in coherent memory.
	 * -------------------------------------------------------
	 */
func plat_set_my_stack
	get_up_stack platform_coherent_stacks, PLATFORM_STACK_SIZE
	mov sp, x0
	ret
endfunc plat_set_my_stack

	/* ----------------------------------------------------
	 * Single cpu stack in coherent memory.
	 * ----------------------------------------------------
	 */
declare_stack platform_coherent_stacks, tzfw_coherent_mem, \
		PLATFORM_STACK_SIZE, 1, CACHE_WRITEBACK_GRANULE

#endif	/* defined(IMAGE_BL1) || defined(IMAGE_BL2) */
#endif	/* ARM_CRYPTOCELL_INTEG */
