/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <context.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <smccc_helpers.h>
#include <smccc_macros.S>

	.globl	bl1_aarch32_smc_handler


func bl1_aarch32_smc_handler
	/* On SMC entry, `sp` points to `smc_ctx_t`. Save `lr`. */
	str	lr, [sp, #SMC_CTX_LR_MON]

	/* ------------------------------------------------
	 * SMC in BL1 is handled assuming that the MMU is
	 * turned off by BL2.
	 * ------------------------------------------------
	 */

	/* ----------------------------------------------
	 * Detect if this is a RUN_IMAGE or other SMC.
	 * ----------------------------------------------
	 */
	mov	lr, #BL1_SMC_RUN_IMAGE
	cmp	lr, r0
	bne	smc_handler

	/* ------------------------------------------------
	 * Make sure only Secure world reaches here.
	 * ------------------------------------------------
	 */
	ldcopr  r8, SCR
	tst	r8, #SCR_NS_BIT
	blne	report_exception

	/* ---------------------------------------------------------------------
	 * Pass control to next secure image.
	 * Here it expects r1 to contain the address of a entry_point_info_t
	 * structure describing the BL entrypoint.
	 * ---------------------------------------------------------------------
	 */
	mov	r8, r1
	mov	r0, r1
	bl	bl1_print_next_bl_ep_info

#if SPIN_ON_BL1_EXIT
	bl	print_debug_loop_message
debug_loop:
	b	debug_loop
#endif

	mov	r0, r8
	bl	bl1_plat_prepare_exit

	stcopr	r0, TLBIALL
	dsb	sy
	isb

	/*
	 * Extract PC and SPSR based on struct `entry_point_info_t`
	 * and load it in LR and SPSR registers respectively.
	 */
	ldr	lr, [r8, #ENTRY_POINT_INFO_PC_OFFSET]
	ldr	r1, [r8, #(ENTRY_POINT_INFO_PC_OFFSET + 4)]
	msr	spsr_xc, r1

	/* Some BL32 stages expect lr_svc to provide the BL33 entry address */
	cps	#MODE32_svc
	ldr	lr, [r8, #ENTRY_POINT_INFO_LR_SVC_OFFSET]
	cps	#MODE32_mon

	add	r8, r8, #ENTRY_POINT_INFO_ARGS_OFFSET
	ldm	r8, {r0, r1, r2, r3}
	exception_return
endfunc bl1_aarch32_smc_handler

	/* -----------------------------------------------------
	 * Save Secure/Normal world context and jump to
	 * BL1 SMC handler.
	 * -----------------------------------------------------
	 */
func smc_handler
	/* -----------------------------------------------------
	 * Save the GP registers.
	 * -----------------------------------------------------
	 */
	smccc_save_gp_mode_regs

	/*
	 * `sp` still points to `smc_ctx_t`. Save it to a register
	 * and restore the C runtime stack pointer to `sp`.
	 */
	mov	r6, sp
	ldr	sp, [r6, #SMC_CTX_SP_MON]

	ldr	r0, [r6, #SMC_CTX_SCR]
	and	r7, r0, #SCR_NS_BIT		/* flags */

	/* Switch to Secure Mode */
	bic	r0, #SCR_NS_BIT
	stcopr	r0, SCR
	isb

	/* If caller is from Secure world then turn on the MMU */
	tst	r7, #SCR_NS_BIT
	bne	skip_mmu_on

	/* Turn on the MMU */
	mov	r0, #DISABLE_DCACHE
	bl	enable_mmu_svc_mon

	/* Enable the data cache. */
	ldcopr	r9, SCTLR
	orr	r9, r9, #SCTLR_C_BIT
	stcopr	r9, SCTLR
	isb

skip_mmu_on:
	/* Prepare arguments for BL1 SMC wrapper. */
	ldr	r0, [r6, #SMC_CTX_GPREG_R0]	/* smc_fid */
	mov	r1, #0				/* cookie */
	mov	r2, r6				/* handle */
	mov	r3, r7				/* flags */
	bl	bl1_smc_wrapper

	/* Get the smc_context for next BL image */
	bl	smc_get_next_ctx
	mov	r4, r0

	/* Only turn-off MMU if going to secure world */
	ldr	r5, [r4, #SMC_CTX_SCR]
	tst	r5, #SCR_NS_BIT
	bne	skip_mmu_off

	/* Disable the MMU */
	bl	disable_mmu_icache_secure
	stcopr	r0, TLBIALL
	dsb	sy
	isb

skip_mmu_off:
	/* -----------------------------------------------------
	 * Do the transition to next BL image.
	 * -----------------------------------------------------
	 */
	mov	r0, r4
	monitor_exit
endfunc smc_handler
