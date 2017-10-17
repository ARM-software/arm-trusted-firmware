/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SMCC_MACROS_S__
#define __SMCC_MACROS_S__

#include <arch.h>

/*
 * Macro to save the General purpose registers (r0 - r12), the banked
 * spsr, lr, sp registers and the `scr` register to the SMC context on entry
 * due a SMC call. The `lr` of the current mode (monitor) is expected to be
 * already saved. The `sp` must point to the `smc_ctx_t` to save to.
 * Additionally, also save the 'pmcr' register as this is updated whilst
 * executing in the secure world.
 */
	.macro smcc_save_gp_mode_regs
	/* Save r0 - r12 in the SMC context */
	stm	sp, {r0-r12}
	mov	r0, sp
	add	r0, r0, #SMC_CTX_SP_USR

	/* Save the banked registers including the current SPSR and LR */
	mrs	r4, sp_usr
	mrs	r5, lr_usr
	mrs	r6, spsr_irq
	mrs	r7, sp_irq
	mrs	r8, lr_irq
	mrs	r9, spsr_fiq
	mrs	r10, sp_fiq
	mrs	r11, lr_fiq
	mrs	r12, spsr_svc
	stm	r0!, {r4-r12}

	mrs	r4, sp_svc
	mrs	r5, lr_svc
	mrs	r6, spsr_abt
	mrs	r7, sp_abt
	mrs	r8, lr_abt
	mrs	r9, spsr_und
	mrs	r10, sp_und
	mrs	r11, lr_und
	mrs	r12, spsr
	stm	r0!, {r4-r12}

	/* lr_mon is already saved by caller */
	ldcopr	r4, SCR
	str	r4, [sp, #SMC_CTX_SCR]
	ldcopr	r4, PMCR
	str	r4, [sp, #SMC_CTX_PMCR]
	.endm

/*
 * Macro to restore the `smc_ctx_t`, which includes the General purpose
 * registers and banked mode registers, and exit from the monitor mode.
 * r0 must point to the `smc_ctx_t` to restore from.
 */
	.macro monitor_exit
	/*
	 * Save the current sp and restore the smc context
	 * pointer to sp which will be used for handling the
	 * next SMC.
	 */
	str	sp, [r0, #SMC_CTX_SP_MON]
	mov	sp, r0

	/*
	 * Restore SCR first so that we access the right banked register
	 * when the other mode registers are restored.
	 */
	ldr	r1, [r0, #SMC_CTX_SCR]
	stcopr	r1, SCR
	isb

	/*
	 * Restore the PMCR register.
	 */
	ldr	r1, [r0, #SMC_CTX_PMCR]
	stcopr	r1, PMCR

	/* Restore the banked registers including the current SPSR */
	add	r1, r0, #SMC_CTX_SP_USR
	ldm	r1!, {r4-r12}
	msr	sp_usr, r4
	msr	lr_usr, r5
	msr	spsr_irq, r6
	msr	sp_irq, r7
	msr	lr_irq, r8
	msr	spsr_fiq, r9
	msr	sp_fiq, r10
	msr	lr_fiq, r11
	msr	spsr_svc, r12

	ldm	r1!, {r4-r12}
	msr	sp_svc, r4
	msr	lr_svc, r5
	msr	spsr_abt, r6
	msr	sp_abt, r7
	msr	lr_abt, r8
	msr	spsr_und, r9
	msr	sp_und, r10
	msr	lr_und, r11
	/*
	 * Use the `_fsxc` suffix explicitly to instruct the assembler
	 * to update all the 32 bits of SPSR. Else, by default, the
	 * assembler assumes `_fc` suffix which only modifies
	 * f->[31:24] and c->[7:0] bits of SPSR.
	 */
	msr	spsr_fsxc, r12

	/* Restore the LR */
	ldr	lr, [r0, #SMC_CTX_LR_MON]

	/* Restore the rest of the general purpose registers */
	ldm	r0, {r0-r12}
	eret
	.endm

#endif /* __SMCC_MACROS_S__ */
