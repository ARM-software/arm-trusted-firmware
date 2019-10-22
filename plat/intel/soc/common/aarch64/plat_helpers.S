/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <platform_def.h>
#include <el3_common_macros.S>

	.globl	plat_secondary_cold_boot_setup
	.globl	platform_is_primary_cpu
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl  plat_crash_console_flush
	.globl	platform_mem_init
	.globl	plat_secondary_cpus_bl31_entry

	.globl plat_get_my_entrypoint

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
	/* Wait until the it gets reset signal from rstmgr gets populated */
poll_mailbox:
	wfi
	mov_imm	x0, PLAT_SEC_ENTRY
	ldr	x1, [x0]
	mov_imm	x2, PLAT_CPUID_RELEASE
	ldr	x3, [x2]
	mrs	x4, mpidr_el1
	and	x4, x4, #0xff
	cmp	x3, x4
	b.ne	poll_mailbox
	br	x1
endfunc plat_secondary_cold_boot_setup

func platform_is_primary_cpu
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc platform_is_primary_cpu

func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	b   platform_is_primary_cpu
endfunc plat_is_my_cpu_primary

func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

func warm_reset_req
	str	xzr, [x4]
	bl	plat_is_my_cpu_primary
	cbz	x0, cpu_in_wfi
	mov_imm x1, PLAT_SEC_ENTRY
	str	xzr, [x1]
	mrs	x1, rmr_el3
	orr	x1, x1, #0x02
	msr	rmr_el3, x1
	isb
	dsb	sy
cpu_in_wfi:
	wfi
	b	cpu_in_wfi
endfunc warm_reset_req

func plat_get_my_entrypoint
	ldr	x4, =L2_RESET_DONE_REG
	ldr	x5, [x4]
	ldr	x1, =L2_RESET_DONE_STATUS
	cmp	x1, x5
	b.eq	warm_reset_req
	mov_imm	x1, PLAT_SEC_ENTRY
	ldr	x0, [x1]
	ret
endfunc plat_get_my_entrypoint


	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, PLAT_UART0_BASE
	mov_imm	x1, PLAT_UART_CLOCK
	mov_imm	x2, PLAT_BAUDRATE
	b	console_16550_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm x1, PLAT_UART0_BASE
	b	console_16550_core_putc
endfunc plat_crash_console_putc

func plat_crash_console_flush
	mov_imm x0, CRASH_CONSOLE_BASE
	b	console_16550_core_flush
endfunc plat_crash_console_flush


	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Any memory init, relocation to be done before the
	 * platform boots. Called very early in the boot process.
	 * --------------------------------------------------------
	 */
func platform_mem_init
	mov	x0, #0
	ret
endfunc platform_mem_init

func plat_secondary_cpus_bl31_entry
	el3_entrypoint_common                                   \
		_init_sctlr=0                                   \
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS  \
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU      \
		_init_memory=1                                  \
		_init_c_runtime=1                               \
		_exception_vectors=runtime_exceptions		\
		_pie_fixup_size=BL31_LIMIT - BL31_BASE
endfunc plat_secondary_cpus_bl31_entry
