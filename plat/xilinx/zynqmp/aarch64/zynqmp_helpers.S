/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <drivers/arm/gicv2.h>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	zynqmp_calc_core_pos
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	platform_mem_init

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * TODO: Should we read the PSYS register to make sure
	 * that the request has gone through.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	mrs	x0, mpidr_el1

	/* Deactivate the gic cpu interface */
	ldr	x1, =BASE_GICC_BASE
	mov	w0, #(IRQ_BYP_DIS_GRP1 | FIQ_BYP_DIS_GRP1)
	orr	w0, w0, #(IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP0)
	str	w0, [x1, #GICC_CTLR]

	/*
	 * There is no sane reason to come out of this wfi. This
	 * cpu will be powered on and reset by the cpu_on pm api
	 */
	dsb	sy
1:
	no_ret	plat_panic_handler
endfunc plat_secondary_cold_boot_setup

func plat_is_my_cpu_primary
	mov	x9, x30
	bl	plat_my_core_pos
	cmp	x0, #ZYNQMP_PRIMARY_CPU
	cset	x0, eq
	ret	x9
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the zynqmp_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	zynqmp_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int zynqmp_calc_core_pos(u_register_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ClusterId * 4) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func zynqmp_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc zynqmp_calc_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, ZYNQMP_CRASH_UART_BASE
	mov_imm	x1, ZYNQMP_CRASH_UART_CLK_IN_HZ
	mov_imm	x2, ZYNQMP_UART_BAUDRATE
	b	console_cdns_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, ZYNQMP_CRASH_UART_BASE
	b	console_cdns_core_putc
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
	mov_imm	x0, ZYNQMP_CRASH_UART_BASE
	b	console_cdns_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on ARM
	 * platforms. The Secure RAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init
