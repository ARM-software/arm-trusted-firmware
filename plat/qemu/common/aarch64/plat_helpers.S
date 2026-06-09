/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2026, BayLibre SAS
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <plat/common/plat_hold_pen.S>
#include <platform_def.h>

	.globl	plat_my_core_pos
	.globl	plat_get_my_entrypoint
	.globl	platform_mem_init
	.globl	plat_qemu_calc_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl  plat_secondary_cold_boot_setup
	.globl  plat_get_my_entrypoint
	.globl  plat_is_my_cpu_primary

func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_qemu_calc_core_pos
endfunc plat_my_core_pos

/*
 *  unsigned int plat_qemu_calc_core_pos(u_register_t mpidr);
 *  With this function: CorePos = (ClusterId * 4) + CoreId
 */
func plat_qemu_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #(MPIDR_AFFINITY_BITS -\
				  PLATFORM_CPU_PER_CLUSTER_SHIFT)
	ret
endfunc plat_qemu_calc_core_pos

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
	cmp	x0, #QEMU_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * Helper macro to gather the core position into x0, the
	 * hold pen base address into x1, and the hold pen slot
	 * size into x2.
	 *
	 * This macro uses bl, so lr will be clobbered.
	 * -----------------------------------------------------
	 */
.macro	gather_platinum_trio
	bl	plat_my_core_pos
	mov_imm	x1, PLAT_QEMU_HOLD_BASE
	mov	x2, HOLD_SLOT_SIZE
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
	gather_platinum_trio
	/* first check to see if we are about to be suspended */
	madd	x1, x0, x2, x1
	ldr	x0, [x1, HOLD_SLOT_MAGIC1]
	mov_imm x2, HOLD_SUSPEND_MAGIC1
	cmp x2, x0
	/* if not, normal mailbox */
	bne	1f
	ldr	x0, [x1, HOLD_SLOT_MAGIC2]
	mov_imm x2, HOLD_SUSPEND_MAGIC2
	cmp	x2, x0
	/* if the magic numbers match, we enter the suspend state */
	beq	2f
	/* if not, normal mailbox */
1:
	gather_platinum_trio
	plat_hold_pen_wait_and_jump x1, x0, x2
2:
	wfi
	/* if we wake up from wfi, the suspend is considered over.
	 * We will now clear the magic numbers in the hold pen slot
	 * to prevent any stale entry from being reused on a future
	 * warm boot.
	 */
	gather_platinum_trio
	madd	x1, x0, x2, x1
	mov_imm x2, HOLD_BOOT_MAGIC1
	str x2, [x1, HOLD_SLOT_MAGIC1]
	mov_imm x2, HOLD_BOOT_MAGIC2
	str x2, [x1, HOLD_SLOT_MAGIC2]
	b	1b
endfunc plat_secondary_cold_boot_setup

func plat_get_my_entrypoint
	/* TODO support warm boot */
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint

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
	mov_imm x0, PLAT_QEMU_CRASH_UART_BASE
	mov_imm x1, PLAT_QEMU_CRASH_UART_CLK_IN_HZ
	mov_imm x2, PLAT_QEMU_CONSOLE_BAUDRATE
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
	mov_imm	x1, PLAT_QEMU_CRASH_UART_BASE
	b	console_pl011_core_putc
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
	mov_imm	x0, PLAT_QEMU_CRASH_UART_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush

