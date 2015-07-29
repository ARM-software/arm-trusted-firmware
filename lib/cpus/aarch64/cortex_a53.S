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
#include <bl_common.h>
#include <cortex_a53.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a53_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a53_disable_dcache

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a53_disable_smp
	mrs	x0, CPUECTLR_EL1
	bic	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	isb
	dsb	sy
	ret
endfunc cortex_a53_disable_smp

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A53 Errata #826319.
	 * This applies only to revision <= r0p2 of Cortex A53.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Clobbers : x0 - x5
	 * --------------------------------------------------
	 */
func errata_a53_826319_wa
	/*
	 * Compare x0 against revision r0p2
	 */
	cmp	x0, #2
	b.ls	apply_826319
#if DEBUG
	b	print_revision_warning
#else
	ret
#endif
apply_826319:
	mrs	x1, L2ACTLR_EL1
	bic	x1, x1, #L2ACTLR_ENABLE_UNIQUECLEAN
	orr	x1, x1, #L2ACTLR_DISABLE_CLEAN_PUSH
	msr	L2ACTLR_EL1, x1
	ret
endfunc errata_a53_826319_wa

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A53 Errata #836870.
	 * This applies only to revision <= r0p3 of Cortex A53.
	 * From r0p4 and onwards, this errata is enabled by
	 * default.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Clobbers : x0 - x5
	 * --------------------------------------------------
	 */
func errata_a53_836870_wa
	/*
	 * Compare x0 against revision r0p3
	 */
	cmp	x0, #3
	b.ls	apply_836870
#if DEBUG
	b	print_revision_warning
#else
	ret
#endif
apply_836870:
	mrs	x1, CPUACTLR_EL1
	orr	x1, x1, #CPUACTLR_DTAH
	msr	CPUACTLR_EL1, x1
	ret
endfunc errata_a53_836870_wa

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A53.
	 * Clobbers: x0-x5, x15, x19, x30
	 * -------------------------------------------------
	 */
func cortex_a53_reset_func
	mov	x19, x30
	mrs	x0, midr_el1

	/*
	 * Extract the variant[20:23] and revision[0:3] from x0
	 * and pack it in x15[0:7] as variant[4:7] and revision[0:3].
	 * First extract x0[16:23] to x15[0:7] and zero fill the rest.
	 * Then extract x0[0:3] into x15[0:3] retaining other bits.
	 */
	ubfx	x15, x0, #(MIDR_VAR_SHIFT - MIDR_REV_BITS), \
			#(MIDR_REV_BITS + MIDR_VAR_BITS)
	bfxil	x15, x0, #MIDR_REV_SHIFT, #MIDR_REV_BITS

#if ERRATA_A53_826319
	mov	x0, x15
	bl	errata_a53_826319_wa
#endif

#if ERRATA_A53_836870
	mov	x0, x15
	bl	errata_a53_836870_wa
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
endfunc cortex_a53_reset_func

func cortex_a53_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a53_disable_dcache

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
	mov	x30, x18
	b	cortex_a53_disable_smp
endfunc cortex_a53_core_pwr_dwn

func cortex_a53_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a53_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* ---------------------------------------------
	 * Flush L2 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a53_disable_smp
endfunc cortex_a53_cluster_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a53 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a53_regs, "aS"
cortex_a53_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a53_cpu_reg_dump
	adr	x6, cortex_a53_regs
	mrs	x8, CPUECTLR_EL1
	ret
endfunc cortex_a53_cpu_reg_dump

declare_cpu_ops cortex_a53, CORTEX_A53_MIDR
