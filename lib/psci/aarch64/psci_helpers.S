/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#include <lib/psci/psci.h>
#include <platform_def.h>

	.globl	psci_do_pwrdown_cache_maintenance
	.globl	psci_do_pwrup_cache_maintenance

/* -----------------------------------------------------------------------
 * void psci_do_pwrdown_cache_maintenance(void);
 *
 * This function turns off data caches and also ensures that stack memory
 * is correctly flushed out to avoid coherency issues due to a change in
 * its memory attributes.
 * -----------------------------------------------------------------------
 */
func psci_do_pwrdown_cache_maintenance
	stp     x29, x30, [sp,#-16]!
	stp     x19, x20, [sp,#-16]!

	/* Disable L1 data cache and unified L2 cache */
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb

	/* ---------------------------------------------
	 * Do stack maintenance by flushing the used
	 * stack to the main memory and invalidating the
	 * remainder.
	 * ---------------------------------------------
	 */
	bl	plat_get_my_stack

	/* ---------------------------------------------
	 * Calculate and store the size of the used
	 * stack memory in x1.
	 * ---------------------------------------------
	 */
	mov	x19, x0
	mov	x1, sp
	sub	x1, x0, x1
	mov	x0, sp
	bl	flush_dcache_range

	/* ---------------------------------------------
	 * Calculate and store the size of the unused
	 * stack memory in x1. Calculate and store the
	 * stack base address in x0.
	 * ---------------------------------------------
	 */
	sub	x0, x19, #PLATFORM_STACK_SIZE
	sub	x1, sp, x0
	bl	inv_dcache_range

	ldp	x19, x20, [sp], #16
	ldp	x29, x30, [sp], #16
	ret
endfunc psci_do_pwrdown_cache_maintenance


/* -----------------------------------------------------------------------
 * void psci_do_pwrup_cache_maintenance(void);
 *
 * This function performs cache maintenance after this cpu is powered up.
 * Currently, this involves managing the used stack memory before turning
 * on the data cache.
 * -----------------------------------------------------------------------
 */
func psci_do_pwrup_cache_maintenance
	stp	x29, x30, [sp,#-16]!

	/* ---------------------------------------------
	 * Ensure any inflight stack writes have made it
	 * to main memory.
	 * ---------------------------------------------
	 */
	dmb	st

	/* ---------------------------------------------
	 * Calculate and store the size of the used
	 * stack memory in x1. Calculate and store the
	 * stack base address in x0.
	 * ---------------------------------------------
	 */
	bl	plat_get_my_stack
	mov	x1, sp
	sub	x1, x0, x1
	mov	x0, sp
	bl	inv_dcache_range

	/* ---------------------------------------------
	 * Enable the data cache.
	 * ---------------------------------------------
	 */
	mrs	x0, sctlr_el3
	orr	x0, x0, #SCTLR_C_BIT
	msr	sctlr_el3, x0
	isb

	ldp	x29, x30, [sp], #16
	ret
endfunc psci_do_pwrup_cache_maintenance
