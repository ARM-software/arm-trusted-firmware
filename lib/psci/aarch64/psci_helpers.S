/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>
#include <psci.h>

	.globl	psci_do_pwrdown_cache_maintenance
	.globl	psci_do_pwrup_cache_maintenance
	.globl	psci_power_down_wfi
#if !ERROR_DEPRECATED
	.globl psci_entrypoint
#endif

/* -----------------------------------------------------------------------
 * void psci_do_pwrdown_cache_maintenance(unsigned int power level);
 *
 * This function performs cache maintenance for the specified power
 * level. The levels of cache affected are determined by the power
 * level which is passed as the argument i.e. level 0 results
 * in a flush of the L1 cache. Both the L1 and L2 caches are flushed
 * for a higher power level.
 *
 * Additionally, this function also ensures that stack memory is correctly
 * flushed out to avoid coherency issues due to a change in its memory
 * attributes after the data cache is disabled.
 * -----------------------------------------------------------------------
 */
func psci_do_pwrdown_cache_maintenance
	stp     x29, x30, [sp,#-16]!
	stp     x19, x20, [sp,#-16]!

	/* ---------------------------------------------
	 * Determine to how many levels of cache will be
	 * subject to cache maintenance. Power level
	 * 0 implies that only the cpu is being powered
	 * down. Only the L1 data cache needs to be
	 * flushed to the PoU in this case. For a higher
	 * power level we are assuming that a flush
	 * of L1 data and L2 unified cache is enough.
	 * This information should be provided by the
	 * platform.
	 * ---------------------------------------------
	 */
	cmp	w0, #PSCI_CPU_PWR_LVL
	b.eq	do_core_pwr_dwn
	bl	prepare_cluster_pwr_dwn
	b	do_stack_maintenance

do_core_pwr_dwn:
	bl	prepare_core_pwr_dwn

	/* ---------------------------------------------
	 * Do stack maintenance by flushing the used
	 * stack to the main memory and invalidating the
	 * remainder.
	 * ---------------------------------------------
	 */
do_stack_maintenance:
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

/* -----------------------------------------------------------------------
 * void psci_power_down_wfi(void);
 * This function is called to indicate to the power controller that it
 * is safe to power down this cpu. It should not exit the wfi and will
 * be released from reset upon power up.
 * -----------------------------------------------------------------------
 */
func psci_power_down_wfi
	dsb	sy		// ensure write buffer empty
	wfi
	bl	plat_panic_handler
endfunc psci_power_down_wfi

/* -----------------------------------------------------------------------
 * void psci_entrypoint(void);
 * The deprecated entry point for PSCI on warm boot for AArch64.
 * -----------------------------------------------------------------------
 */
func_deprecated psci_entrypoint
	b	bl31_warm_entrypoint
endfunc_deprecated psci_entrypoint
