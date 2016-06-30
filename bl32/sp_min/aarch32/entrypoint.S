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
#include <el3_common_macros.S>
#include <runtime_svc.h>
#include <smcc_helpers.h>
#include <smcc_macros.S>
#include <xlat_tables.h>

	.globl	sp_min_vector_table
	.globl	sp_min_entrypoint
	.globl	sp_min_warm_entrypoint


vector_base sp_min_vector_table
	b	sp_min_entrypoint
	b	plat_panic_handler	/* Undef */
	b	handle_smc		/* Syscall */
	b	plat_panic_handler	/* Prefetch abort */
	b	plat_panic_handler	/* Data abort */
	b	plat_panic_handler	/* Reserved */
	b	plat_panic_handler	/* IRQ */
	b	plat_panic_handler	/* FIQ */


/*
 * The Cold boot/Reset entrypoint for SP_MIN
 */
func sp_min_entrypoint
#if !RESET_TO_SP_MIN
	/* ---------------------------------------------------------------
	 * Preceding bootloader has populated r0 with a pointer to a
	 * 'bl_params_t' structure & r1 with a pointer to platform
	 * specific structure
	 * ---------------------------------------------------------------
	 */
	mov	r11, r0
	mov	r12, r1

	/* ---------------------------------------------------------------------
	 * For !RESET_TO_SP_MIN systems, only the primary CPU ever reaches
	 * sp_min_entrypoint() during the cold boot flow, so the cold/warm boot
	 * and primary/secondary CPU logic should not be executed in this case.
	 *
	 * Also, assume that the previous bootloader has already set up the CPU
	 * endianness and has initialised the memory.
	 * ---------------------------------------------------------------------
	 */
	el3_entrypoint_common					\
		_set_endian=0					\
		_warm_boot_mailbox=0				\
		_secondary_cold_boot=0				\
		_init_memory=0					\
		_init_c_runtime=1				\
		_exception_vectors=sp_min_vector_table

	/* ---------------------------------------------------------------------
	 * Relay the previous bootloader's arguments to the platform layer
	 * ---------------------------------------------------------------------
	 */
	mov	r0, r11
	mov	r1, r12
#else
	/* ---------------------------------------------------------------------
	 * For RESET_TO_SP_MIN systems which have a programmable reset address,
	 * sp_min_entrypoint() is executed only on the cold boot path so we can
	 * skip the warm boot mailbox mechanism.
	 * ---------------------------------------------------------------------
	 */
	el3_entrypoint_common					\
		_set_endian=1					\
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS	\
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU	\
		_init_memory=1					\
		_init_c_runtime=1				\
		_exception_vectors=sp_min_vector_table

	/* ---------------------------------------------------------------------
	 * For RESET_TO_SP_MIN systems, BL32 (SP_MIN) is the first bootloader
	 * to run so there's no argument to relay from a previous bootloader.
	 * Zero the arguments passed to the platform layer to reflect that.
	 * ---------------------------------------------------------------------
	 */
	mov	r0, #0
	mov	r1, #0
#endif /* RESET_TO_SP_MIN */

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
 * SMC handling function for SP_MIN.
 */
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
 * The Warm boot entrypoint for SP_MIN.
 */
func sp_min_warm_entrypoint
	/*
	 * On the warm boot path, most of the EL3 initialisations performed by
	 * 'el3_entrypoint_common' must be skipped:
	 *
	 *  - Only when the platform bypasses the BL1/BL32 (SP_MIN) entrypoint by
	 *    programming the reset address do we need to set the CPU endianness.
	 *    In other cases, we assume this has been taken care by the
	 *    entrypoint code.
	 *
	 *  - No need to determine the type of boot, we know it is a warm boot.
	 *
	 *  - Do not try to distinguish between primary and secondary CPUs, this
	 *    notion only exists for a cold boot.
	 *
	 *  - No need to initialise the memory or the C runtime environment,
	 *    it has been done once and for all on the cold boot path.
	 */
	el3_entrypoint_common					\
		_set_endian=PROGRAMMABLE_RESET_ADDRESS		\
		_warm_boot_mailbox=0				\
		_secondary_cold_boot=0				\
		_init_memory=0					\
		_init_c_runtime=0				\
		_exception_vectors=sp_min_vector_table

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
