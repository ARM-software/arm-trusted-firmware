/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <context.h>
#include <runtime_svc.h>
#include <smcc_helpers.h>
#include <smcc_macros.S>
#include <xlat_tables.h>

	.globl	sp_min_vector_table
	.globl	sp_min_entrypoint
	.globl	sp_min_warm_entrypoint

func sp_min_vector_table
	b	sp_min_entrypoint
	b	plat_panic_handler	/* Undef */
	b	handle_smc		/* Syscall */
	b	plat_panic_handler	/* Prefetch abort */
	b	plat_panic_handler	/* Data abort */
	b	plat_panic_handler	/* Reserved */
	b	plat_panic_handler	/* IRQ */
	b	plat_panic_handler	/* FIQ */
endfunc sp_min_vector_table

func handle_smc
	smcc_save_gp_mode_regs

	/* r0 points to smc_context */
	mov	r2, r0				/* handle */
	ldcopr	r0, SCR

	/* Save SCR in stack */
	push	{r0}
	and	r3, r0, #SCR_NS_BIT		/* flags */

	/* Switch to Secure Mode*/
	bic	r0, #SCR_NS_BIT
	stcopr	r0, SCR
	isb
	ldr	r0, [r2, #SMC_CTX_GPREG_R0]	/* smc_fid */
	/* Check whether an SMC64 is issued */
	tst	r0, #(FUNCID_CC_MASK << FUNCID_CC_SHIFT)
	beq	1f	/* SMC32 is detected */
	mov	r0, #SMC_UNK
	str	r0, [r2, #SMC_CTX_GPREG_R0]
	mov	r0, r2
	b	2f	/* Skip handling the SMC */
1:
	mov	r1, #0				/* cookie */
	bl	handle_runtime_svc
2:
	/* r0 points to smc context */

	/* Restore SCR from stack */
	pop	{r1}
	stcopr	r1, SCR
	isb

	b	sp_min_exit
endfunc handle_smc

/*
 * The Cold boot/Reset entrypoint for SP_MIN
 */
func sp_min_entrypoint

	/*
	 * The caches and TLBs are disabled at reset. If any implementation
	 * allows the caches/TLB to be hit while they are disabled, ensure
	 * that they are invalidated here
	 */

	/* Make sure we are in Secure Mode*/
	ldcopr	r0, SCR
	bic	r0, #SCR_NS_BIT
	stcopr	r0, SCR
	isb

	/* Switch to monitor mode */
	cps	#MODE32_mon
	isb

	/*
	 * Set sane values for NS SCTLR as well.
	 * Switch to non secure mode for this.
	 */
	ldr	r0, =(SCTLR_RES1)
	ldcopr	r1, SCR
	orr	r2, r1, #SCR_NS_BIT
	stcopr	r2, SCR
	isb

	ldcopr	r2, SCTLR
	orr	r0, r0, r2
	stcopr	r0, SCTLR
	isb

	stcopr	r1, SCR
	isb

	/*
	 * Set the CPU endianness before doing anything that might involve
	 * memory reads or writes.
	 */
	ldcopr	r0, SCTLR
	bic	r0, r0, #SCTLR_EE_BIT
	stcopr	r0, SCTLR
	isb

	/* Run the CPU Specific Reset handler */
	bl	reset_handler

	/*
	 * Enable the instruction cache and data access
	 * alignment checks
	 */
	ldcopr	r0, SCTLR
	ldr	r1, =(SCTLR_RES1 | SCTLR_A_BIT | SCTLR_I_BIT)
	orr	r0, r0, r1
	stcopr	r0, SCTLR
	isb

	/* Set the vector tables */
	ldr	r0, =sp_min_vector_table
	stcopr	r0, VBAR
	stcopr	r0, MVBAR
	isb

	/*
	 * Enable the SIF bit to disable instruction fetches
	 * from Non-secure memory.
	 */
	ldcopr	r0, SCR
	orr	r0, r0, #SCR_SIF_BIT
	stcopr	r0, SCR

	/*
	 * Enable the SError interrupt now that the exception vectors have been
	 * setup.
	 */
	cpsie   a
	isb

	/* Enable access to Advanced SIMD registers */
	ldcopr	r0, NSACR
	bic	r0, r0, #NSASEDIS_BIT
	orr	r0, r0, #(NASCR_CP10_BIT | NASCR_CP11_BIT)
	stcopr	r0, NSACR
	isb

	/*
	 * Enable access to Advanced SIMD, Floating point and to the Trace
	 * functionality as well.
	 */
	ldcopr	r0, CPACR
	bic	r0, r0, #ASEDIS_BIT
	bic	r0, r0, #TRCDIS_BIT
	orr	r0, r0, #CPACR_ENABLE_FP_ACCESS
	stcopr	r0, CPACR
	isb

	vmrs	r0, FPEXC
	orr	r0, r0, #FPEXC_EN_BIT
	vmsr	FPEXC, r0

	/* Detect whether Warm or Cold boot */
	bl	plat_get_my_entrypoint
	cmp	r0, #0
	/* If warm boot detected, jump to warm boot entry */
	bxne	r0

	/* Setup C runtime stack */
	bl	plat_set_my_stack

	/* Perform platform specific memory initialization */
	bl	platform_mem_init

	/* Initialize the C Runtime Environment */

	/*
	 * Invalidate the RW memory used by SP_MIN image. This includes
	 * the data and NOBITS sections. This is done to safeguard against
	 * possible corruption of this memory by dirty cache lines in a system
	 * cache as a result of use by an earlier boot loader stage.
	 */
	ldr	r0, =__RW_START__
	ldr	r1, =__RW_END__
	sub	r1, r1, r0
	bl	inv_dcache_range

	ldr	r0, =__BSS_START__
	ldr	r1, =__BSS_SIZE__
	bl	zeromem

#if USE_COHERENT_MEM
	ldr	r0, =__COHERENT_RAM_START__
	ldr	r1, =__COHERENT_RAM_UNALIGNED_SIZE__
	bl	zeromem
#endif

	/* Perform platform specific early arch. setup */
	bl	sp_min_early_platform_setup
	bl	sp_min_plat_arch_setup

	/* Jump to the main function */
	bl	sp_min_main

	/* -------------------------------------------------------------
	 * Clean the .data & .bss sections to main memory. This ensures
	 * that any global data which was initialised by the primary CPU
	 * is visible to secondary CPUs before they enable their data
	 * caches and participate in coherency.
	 * -------------------------------------------------------------
	 */
	ldr	r0, =__DATA_START__
	ldr	r1, =__DATA_END__
	sub	r1, r1, r0
	bl	clean_dcache_range

	ldr	r0, =__BSS_START__
	ldr	r1, =__BSS_END__
	sub	r1, r1, r0
	bl	clean_dcache_range

	/* Program the registers in cpu_context and exit monitor mode */
	mov	r0, #NON_SECURE
	bl	cm_get_context

	/* Restore the SCR */
	ldr	r2, [r0, #CTX_REGS_OFFSET + CTX_SCR]
	stcopr	r2, SCR
	isb

	/* Restore the SCTLR  */
	ldr	r2, [r0, #CTX_REGS_OFFSET + CTX_NS_SCTLR]
	stcopr	r2, SCTLR

	bl	smc_get_next_ctx
	/* The other cpu_context registers have been copied to smc context */
	b	sp_min_exit
endfunc sp_min_entrypoint

/*
 * The Warm boot entrypoint for SP_MIN.
 */
func sp_min_warm_entrypoint

	/* Setup C runtime stack */
	bl	plat_set_my_stack

	/* --------------------------------------------
	 * Enable the MMU with the DCache disabled. It
	 * is safe to use stacks allocated in normal
	 * memory as a result. All memory accesses are
	 * marked nGnRnE when the MMU is disabled. So
	 * all the stack writes will make it to memory.
	 * All memory accesses are marked Non-cacheable
	 * when the MMU is enabled but D$ is disabled.
	 * So used stack memory is guaranteed to be
	 * visible immediately after the MMU is enabled
	 * Enabling the DCache at the same time as the
	 * MMU can lead to speculatively fetched and
	 * possibly stale stack memory being read from
	 * other caches. This can lead to coherency
	 * issues.
	 * --------------------------------------------
	 */
	mov	r0, #DISABLE_DCACHE
	bl	bl32_plat_enable_mmu

	bl	sp_min_warm_boot

	/* Program the registers in cpu_context and exit monitor mode */
	mov	r0, #NON_SECURE
	bl	cm_get_context

	/* Restore the SCR */
	ldr	r2, [r0, #CTX_REGS_OFFSET + CTX_SCR]
	stcopr	r2, SCR
	isb

	/* Restore the SCTLR  */
	ldr	r2, [r0, #CTX_REGS_OFFSET + CTX_NS_SCTLR]
	stcopr	r2, SCTLR

	bl	smc_get_next_ctx

	/* The other cpu_context registers have been copied to smc context */
	b	sp_min_exit
endfunc sp_min_warm_entrypoint

/*
 * The function to restore the registers from SMC context and return
 * to the mode restored to SPSR.
 *
 * Arguments : r0 must point to the SMC context to restore from.
 */
func sp_min_exit
	smcc_restore_gp_mode_regs
	eret
endfunc sp_min_exit
