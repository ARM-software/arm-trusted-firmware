/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cortex_a72.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a72_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a72_disable_dcache

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * ---------------------------------------------
	 */
func cortex_a72_disable_l2_prefetch
	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_DIS_TWD_ACC_PFTCH_BIT
	mov	x1, #CPUECTLR_L2_IPFTCH_DIST_MASK
	orr	x1, x1, #CPUECTLR_L2_DPFTCH_DIST_MASK
	bic	x0, x0, x1
	msr	CPUECTLR_EL1, x0
	isb
	ret
endfunc cortex_a72_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable the load-store hardware prefetcher.
	 * ---------------------------------------------
	 */
func cortex_a72_disable_hw_prefetcher
	mrs	x0, CPUACTLR_EL1
	orr	x0, x0, #CPUACTLR_DISABLE_L1_DCACHE_HW_PFTCH
	msr	CPUACTLR_EL1, x0
	isb
	dsb	ish
	ret
endfunc cortex_a72_disable_hw_prefetcher

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a72_disable_smp
	mrs	x0, CPUECTLR_EL1
	bic	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	ret
endfunc cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a72_disable_ext_debug
	mov	x0, #1
	msr	osdlr_el1, x0
	isb
	dsb	sy
	ret
endfunc cortex_a72_disable_ext_debug

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A72.
	 * -------------------------------------------------
	 */
func cortex_a72_reset_func
	/* ---------------------------------------------
	 * As a bare minimum enable the SMP bit.
	 * ---------------------------------------------
	 */
	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	isb
	ret
endfunc cortex_a72_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A72.
	 * ----------------------------------------------------
	 */
func cortex_a72_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable the load-store hardware prefetcher.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_hw_prefetcher

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a72_disable_ext_debug
endfunc cortex_a72_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A72.
	 * -------------------------------------------------------
	 */
func cortex_a72_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable the load-store hardware prefetcher.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_hw_prefetcher

#if !SKIP_A72_L1_FLUSH_PWR_DWN
	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1
#endif

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* -------------------------------------------------
	 * Flush the L2 caches.
	 * -------------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a72_disable_ext_debug
endfunc cortex_a72_cluster_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a72 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a72_regs, "aS"
cortex_a72_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a72_cpu_reg_dump
	adr	x6, cortex_a72_regs
	mrs	x8, CPUECTLR_EL1
	ret
endfunc cortex_a72_cpu_reg_dump


declare_cpu_ops cortex_a72, CORTEX_A72_MIDR
