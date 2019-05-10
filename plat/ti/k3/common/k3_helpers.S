/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_a72.h>
#include <cpu_macros.S>
#include <platform_def.h>

#define K3_BOOT_REASON_COLD_RESET 0x1

	/* ------------------------------------------------------------------
	 *  uintptr_t plat_get_my_entrypoint(void)
	 * ------------------------------------------------------------------
	 *
	 * This function is called with the called with the MMU and caches
	 * disabled (SCTLR_EL3.M = 0 and SCTLR_EL3.C = 0). The function is
	 * responsible for distinguishing between a warm and cold reset for the
	 * current CPU using platform-specific means. If it's a warm reset,
	 * then it returns the warm reset entrypoint point provided to
	 * plat_setup_psci_ops() during BL31 initialization. If it's a cold
	 * reset then this function must return zero.
	 *
	 * This function does not follow the Procedure Call Standard used by
	 * the Application Binary Interface for the ARM 64-bit architecture.
	 * The caller should not assume that callee saved registers are
	 * preserved across a call to this function.
	 */
	.globl	plat_get_my_entrypoint
func plat_get_my_entrypoint
	ldr x0, k3_boot_reason_data_store
	cmp  x0, #K3_BOOT_REASON_COLD_RESET

	/* We ONLY support cold boot at this point */
	bne plat_unsupported_boot
	mov	x0, #0
	ret

	/*
	 * We self manage our boot reason.
	 * At load time, we have just a default reason - which is cold reset
	 */
k3_boot_reason_data_store:
	.word	K3_BOOT_REASON_COLD_RESET

plat_unsupported_boot:
	b plat_unsupported_boot

endfunc plat_get_my_entrypoint

	/* ------------------------------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * ------------------------------------------------------------------
	 *
	 * This function returns the index of the calling CPU which is used as a
	 * CPU-specific linear index into blocks of memory (for example while
	 * allocating per-CPU stacks). This function will be invoked very early
	 * in the initialization sequence which mandates that this function
	 * should be implemented in assembly and should not rely on the
	 * avalability of a C runtime environment. This function can clobber x0
	 * - x8 and must preserve x9 - x29.
	 *
	 * This function plays a crucial role in the power domain topology
	 * framework in PSCI and details of this can be found in Power Domain
	 * Topology Design.
	 */
	.globl plat_my_core_pos
func plat_my_core_pos
	mrs	x0, MPIDR_EL1

	and	x1, x0, #MPIDR_CLUSTER_MASK
	lsr	x1, x1, #MPIDR_AFF1_SHIFT
	and	x0, x0, #MPIDR_CPU_MASK

	cmp	x1, 0
	b.eq out
	add	x0, x0, #K3_CLUSTER0_CORE_COUNT

	cmp	x1, 1
	b.eq out
	add	x0, x0, #K3_CLUSTER1_CORE_COUNT

	cmp	x1, 2
	b.eq out
	add	x0, x0, #K3_CLUSTER2_CORE_COUNT

out:
	ret
endfunc plat_my_core_pos

	/* --------------------------------------------------------------------
	 * This handler does the following:
	 * - Set the L2 Data RAM latency to 2 (i.e. 3 cycles) for Cortex-A72
	 * --------------------------------------------------------------------
	 */
	.globl plat_reset_handler
func plat_reset_handler
	/* Only on Cortex-A72 */
	jump_if_cpu_midr CORTEX_A72_MIDR, a72
	ret

	/* Cortex-A72 specific settings */
a72:
	mrs x0, CORTEX_A72_L2CTLR_EL1
	orr x0, x0, #(CORTEX_A72_L2_DATA_RAM_LATENCY_3_CYCLES << CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT)
	msr CORTEX_A72_L2CTLR_EL1, x0
	isb
	ret
endfunc plat_reset_handler

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
	.globl plat_crash_console_init
func plat_crash_console_init
	mov_imm	x0, CRASH_CONSOLE_BASE
	mov_imm	x1, CRASH_CONSOLE_CLK
	mov_imm	x2, CRASH_CONSOLE_BAUD_RATE
	mov w3, #0x0
	b	console_16550_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
	.globl plat_crash_console_putc
func plat_crash_console_putc
	mov_imm	x1, CRASH_CONSOLE_BASE
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
	.globl plat_crash_console_flush
func plat_crash_console_flush
	mov_imm	x0, CRASH_CONSOLE_BASE
	b	console_16550_core_flush
endfunc plat_crash_console_flush
