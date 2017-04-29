/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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
#include <cortex_a57.h>
#include <cpu_macros.S>
#include <debug.h>

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * Clobbers: r0-r1
	 * ---------------------------------------------
	 */
func cortex_a57_disable_smp
	ldcopr16	r0, r1, CPUECTLR
	bic64_imm	r0, r1, CPUECTLR_SMP_BIT
	stcopr16	r0, r1, CPUECTLR
	bx	lr
endfunc cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * Clobbers: r0-r2
	 * ---------------------------------------------
	 */
func cortex_a57_disable_l2_prefetch
	ldcopr16	r0, r1, CPUECTLR
	orr64_imm	r0, r1, CPUECTLR_DIS_TWD_ACC_PFTCH_BIT
	bic64_imm	r0, r1, (CPUECTLR_L2_IPFTCH_DIST_MASK | \
				CPUECTLR_L2_DPFTCH_DIST_MASK)
	stcopr16	r0, r1, CPUECTLR
	isb
	dsb	ish
	bx	lr
endfunc cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a57_disable_ext_debug
	mov	r0, #1
	stcopr	r0, DBGOSDLR
	isb
	dsb	sy
	bx	lr
endfunc cortex_a57_disable_ext_debug

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A57.
	 * -------------------------------------------------
	 */
func cortex_a57_reset_func
	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	ldcopr16	r0, r1, CPUECTLR
	orr64_imm	r0, r1, CPUECTLR_SMP_BIT
	stcopr16	r0, r1,	CPUECTLR
	isb
	bx	lr
endfunc cortex_a57_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A57.
	 * ----------------------------------------------------
	 */
func cortex_a57_core_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ASM_ASSERTION
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
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
	pop	{r12, lr}
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A57.
	 * Clobbers: r0-r3
	 * -------------------------------------------------------
	 */
func cortex_a57_cluster_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ASM_ASSERTION
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
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
	mov	r0, #DC_OP_CISW
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
	pop	{r12, lr}
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_cluster_pwr_dwn

declare_cpu_ops cortex_a57, CORTEX_A57_MIDR, \
	cortex_a57_reset_func, \
	cortex_a57_core_pwr_dwn, \
	cortex_a57_cluster_pwr_dwn
