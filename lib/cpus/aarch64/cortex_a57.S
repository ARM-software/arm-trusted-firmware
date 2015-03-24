/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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
#include <bl_common.h>
#include <cortex_a57.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a57_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * ---------------------------------------------
	 */
func cortex_a57_disable_l2_prefetch
	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_DIS_TWD_ACC_PFTCH_BIT
	mov	x1, #CPUECTLR_L2_IPFTCH_DIST_MASK
	orr	x1, x1, #CPUECTLR_L2_DPFTCH_DIST_MASK
	bic	x0, x0, x1
	msr	CPUECTLR_EL1, x0
	isb
	dsb	ish
	ret
endfunc cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a57_disable_smp
	mrs	x0, CPUECTLR_EL1
	bic	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	ret
endfunc cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a57_disable_ext_debug
	mov	x0, #1
	msr	osdlr_el1, x0
	isb
	dsb	sy
	ret
endfunc cortex_a57_disable_ext_debug

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #806969.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Clobbers : x0 - x5
	 * --------------------------------------------------
	 */
func errata_a57_806969_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	cbz	x0, apply_806969
#if DEBUG
	b	print_revision_warning
#else
	ret
#endif
apply_806969:
	/*
	 * Test if errata has already been applied in an earlier
	 * invocation of the reset handler and does not need to
	 * be applied again.
	 */
	mrs	x1, CPUACTLR_EL1
	tst	x1, #CPUACTLR_NO_ALLOC_WBWA
	b.ne	skip_806969
	orr	x1, x1, #CPUACTLR_NO_ALLOC_WBWA
	msr	CPUACTLR_EL1, x1
skip_806969:
	ret
endfunc errata_a57_806969_wa


	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #813420.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Clobbers : x0 - x5
	 * ---------------------------------------------------
	 */
func errata_a57_813420_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	cbz	x0, apply_813420
#if DEBUG
	b	print_revision_warning
#else
	ret
#endif
apply_813420:
	/*
	 * Test if errata has already been applied in an earlier
	 * invocation of the reset handler and does not need to
	 * be applied again.
	 */
	mrs	x1, CPUACTLR_EL1
	tst	x1, #CPUACTLR_DCC_AS_DCCI
	b.ne	skip_813420
	orr	x1, x1, #CPUACTLR_DCC_AS_DCCI
	msr	CPUACTLR_EL1, x1
skip_813420:
	ret
endfunc errata_a57_813420_wa

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A57.
	 * Clobbers: x0-x5, x15, x19, x30
	 * -------------------------------------------------
	 */
func cortex_a57_reset_func
	mov	x19, x30
	mrs	x0, midr_el1

	/*
	 * Extract the variant[20:23] and revision[0:3] from x0
	 * and pack it in x15[0:7] as variant[4:7] and revision[0:3].
	 * First extract x0[16:23] to x15[0:7] and zero fill the rest.
	 * Then extract x0[0:3] into x15[0:3] retaining other bits.
	 */
	ubfx	x15, x0, #(MIDR_VAR_SHIFT - MIDR_REV_BITS), #(MIDR_REV_BITS + MIDR_VAR_BITS)
	bfxil	x15, x0, #MIDR_REV_SHIFT, #MIDR_REV_BITS

#if ERRATA_A57_806969
	mov	x0, x15
	bl	errata_a57_806969_wa
#endif

#if ERRATA_A57_813420
	mov	x0, x15
	bl	errata_a57_813420_wa
#endif

	/* ---------------------------------------------
	 * As a bare minimum enable the SMP bit if it is
	 * not already set.
	 * ---------------------------------------------
	 */
	mrs	x0, CPUECTLR_EL1
	tst	x0, #CPUECTLR_SMP_BIT
	b.ne	skip_smp_setup
	orr	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
skip_smp_setup:
	isb
	ret	x19
endfunc cortex_a57_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A57.
	 * ----------------------------------------------------
	 */
func cortex_a57_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

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
	bl	cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A57.
	 * -------------------------------------------------------
	 */
func cortex_a57_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

#if !SKIP_A57_L1_FLUSH_PWR_DWN
	/* -------------------------------------------------
	 * Flush the L1 caches.
	 * -------------------------------------------------
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
	bl	cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_cluster_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a57 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a57_regs, "aS"
cortex_a57_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a57_cpu_reg_dump
	adr	x6, cortex_a57_regs
	mrs	x8, CPUECTLR_EL1
	ret
endfunc cortex_a57_cpu_reg_dump


declare_cpu_ops cortex_a57, CORTEX_A57_MIDR
