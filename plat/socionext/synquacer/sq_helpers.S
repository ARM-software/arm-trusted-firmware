/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

	.global	sq_calc_core_pos
	.global	plat_my_core_pos
	.global	platform_mem_init
	.global	plat_is_my_cpu_primary
	.global plat_secondary_cold_boot_setup
	.global	plat_crash_console_init
	.global	plat_crash_console_putc
	.global	plat_crash_console_flush

/*
 * unsigned int sq_calc_core_pos(u_register_t mpidr)
 * core_pos = (cluster_id * max_cpus_per_cluster) + core_id
 */
func sq_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, lsr #7
	ret
endfunc sq_calc_core_pos

func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	sq_calc_core_pos
endfunc plat_my_core_pos

func platform_mem_init
	ret
endfunc platform_mem_init

/*
 * Secondary CPUs are placed in a holding pen, waiting for their mailbox
 * to be populated. Note that all CPUs share the same mailbox ; therefore,
 * populating it will release all CPUs from their holding pen. If
 * finer-grained control is needed then this should be handled in the
 * code that secondary CPUs jump to.
 */
func plat_secondary_cold_boot_setup
	ldr	x0, sq_sec_entrypoint

	/* Wait until the mailbox gets populated */
poll_mailbox:
	cbz	x0, 1f
	br	x0
1:
	wfe
	b	poll_mailbox
endfunc plat_secondary_cold_boot_setup

/*
 * Find out whether the current cpu is the primary
 * cpu (applicable only after a cold boot)
 */
func plat_is_my_cpu_primary
	mov	x9, x30
	bl	plat_my_core_pos
	ldr	x1, =SQ_BOOT_CFG_ADDR
	ldr	x1, [x1]
	ubfx	x1, x1, #PLAT_SQ_PRIMARY_CPU_SHIFT, \
			#PLAT_SQ_PRIMARY_CPU_BIT_WIDTH
	cmp	x0, x1
	cset	w0, eq
	ret	x9
endfunc plat_is_my_cpu_primary

/*
 * int plat_crash_console_init(void)
 * Function to initialize the crash console
 * without a C Runtime to print crash report.
 * Clobber list : x0, x1, x2
 */
func plat_crash_console_init
	mov_imm x0, PLAT_SQ_BOOT_UART_BASE
	mov_imm x1, PLAT_SQ_BOOT_UART_CLK_IN_HZ
	mov_imm x2, SQ_CONSOLE_BAUDRATE
	b	console_pl011_core_init
endfunc plat_crash_console_init

/*
 * int plat_crash_console_putc(int c)
 * Function to print a character on the crash
 * console without a C Runtime.
 * Clobber list : x1, x2
 */
func plat_crash_console_putc
	mov_imm	x1, PLAT_SQ_BOOT_UART_BASE
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

/*
 * int plat_crash_console_flush(int c)
 * Function to force a write of all buffered
 * data that hasn't been output.
 * Out : return -1 on error else return 0.
 * Clobber list : x0, x1
 */
func plat_crash_console_flush
	mov_imm	x0, PLAT_SQ_BOOT_UART_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush
