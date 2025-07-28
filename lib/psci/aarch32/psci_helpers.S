/*
 * Copyright (c) 2016-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
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
	push	{r4, lr}
	bl	plat_get_my_stack

	/* Turn off the D-cache */
	ldcopr	r1, SCTLR
	bic	r1, #SCTLR_C_BIT
	stcopr	r1, SCTLR
	isb

	/* ---------------------------------------------
	 * Calculate and store the size of the used
	 * stack memory in r1.
	 * ---------------------------------------------
	 */
	mov	r4, r0
	mov	r1, sp
	sub	r1, r0, r1
	mov	r0, sp
	bl	flush_dcache_range

	/* ---------------------------------------------
	 * Calculate and store the size of the unused
	 * stack memory in r1. Calculate and store the
	 * stack base address in r0.
	 * ---------------------------------------------
	 */
	sub	r0, r4, #PLATFORM_STACK_SIZE
	sub	r1, sp, r0
	bl	inv_dcache_range

	pop	{r4, pc}
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
	/* r12 is pushed to meet the 8 byte stack alignment requirement */
	push	{r12, lr}

	/* ---------------------------------------------
	 * Ensure any inflight stack writes have made it
	 * to main memory.
	 * ---------------------------------------------
	 */
	dmb	st

	/* ---------------------------------------------
	 * Calculate and store the size of the used
	 * stack memory in r1. Calculate and store the
	 * stack base address in r0.
	 * ---------------------------------------------
	 */
	bl	plat_get_my_stack
	mov	r1, sp
	sub	r1, r0, r1
	mov	r0, sp
	bl	inv_dcache_range

	/* ---------------------------------------------
	 * Enable the data cache.
	 * ---------------------------------------------
	 */
	ldcopr	r0, SCTLR
	orr	r0, r0, #SCTLR_C_BIT
	stcopr	r0, SCTLR
	isb

	pop	{r12, pc}
endfunc psci_do_pwrup_cache_maintenance
