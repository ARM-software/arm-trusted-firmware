/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
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

#if K3_CLUSTER1_MSMC_PORT != UNUSED
	cmp	x1, #K3_CLUSTER0_MSMC_PORT
	b.eq out
	add	x0, x0, #K3_CLUSTER0_CORE_COUNT
#if K3_CLUSTER2_MSMC_PORT != UNUSED
	cmp	x1, #K3_CLUSTER1_MSMC_PORT
	b.eq out
	add	x0, x0, #K3_CLUSTER1_CORE_COUNT
#if K3_CLUSTER3_MSMC_PORT != UNUSED
	cmp	x1, #K3_CLUSTER2_MSMC_PORT
	b.eq out
	add	x0, x0, #K3_CLUSTER2_CORE_COUNT
#endif /* K3_CLUSTER3_MSMC_PORT != UNUSED */
#endif /* K3_CLUSTER2_MSMC_PORT != UNUSED */
#endif /* K3_CLUSTER1_MSMC_PORT != UNUSED */

out:
	ret
endfunc plat_my_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, CRASH_CONSOLE_BASE
	mov_imm	x1, CRASH_CONSOLE_CLK
	mov_imm	x2, CRASH_CONSOLE_BAUD_RATE
	mov w3, #0x0
	b	console_core_init

endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, CRASH_CONSOLE_BASE
	b	console_core_putc
endfunc plat_crash_console_putc
