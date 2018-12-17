/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SMCCC_MACROS_S
#define SMCCC_MACROS_S

#include <arch.h>

/*
 * Macro to save the General purpose registers (r0 - r12), the banked
 * spsr, lr, sp registers and the `scr` register to the SMC context on entry
 * due a SMC call. The `lr` of the current mode (monitor) is expected to be
 * already saved. The `sp` must point to the `smc_ctx_t` to save to.
 * Additionally, also save the 'pmcr' register as this is updated whilst
 * executing in the secure world.
 */
	.macro smccc_save_gp_mode_regs
	/* Save r0 - r12 in the SMC context */
	stm	sp, {r0-r12}
	mov	r0, sp
	add	r0, r0, #SMC_CTX_SP_USR

#if ARM_ARCH_MAJOR == 7 && !defined(ARMV7_SUPPORTS_VIRTUALIZATION)
	/* Must be in secure state to restore Monitor mode */
	ldcopr	r4, SCR
	bic	r2, r4, #SCR_NS_BIT
	stcopr	r2, SCR
	isb

	cps	#MODE32_sys
	stm	r0!, {sp, lr}

	cps	#MODE32_irq
	mrs	r2, spsr
	stm	r0!, {r2, sp, lr}

	cps	#MODE32_fiq
	mrs	r2, spsr
	stm	r0!, {r2, sp, lr}

	cps	#MODE32_svc
	mrs	r2, spsr
	stm	r0!, {r2, sp, lr}

	cps	#MODE32_abt
	mrs	r2, spsr
	stm	r0!, {r2, sp, lr}

	cps	#MODE32_und
	mrs	r2, spsr
	stm	r0!, {r2, sp, lr}

	/* lr_mon is already saved by caller */
	cps	#MODE32_mon
	mrs	r2, spsr
	stm	r0!, {r2}

	stcopr	r4, SCR
	isb
#else
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
#endif
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

#if ARM_ARCH_MAJOR == 7 && !defined(ARMV7_SUPPORTS_VIRTUALIZATION)
	/* Must be in secure state to restore Monitor mode */
	ldcopr	r4, SCR
	bic	r2, r4, #SCR_NS_BIT
	stcopr	r2, SCR
	isb

	cps	#MODE32_sys
	ldm	r1!, {sp, lr}

	cps	#MODE32_irq
	ldm	r1!, {r2, sp, lr}
	msr	spsr_fsxc, r2

	cps	#MODE32_fiq
	ldm	r1!, {r2, sp, lr}
	msr	spsr_fsxc, r2

	cps	#MODE32_svc
	ldm	r1!, {r2, sp, lr}
	msr	spsr_fsxc, r2

	cps	#MODE32_abt
	ldm	r1!, {r2, sp, lr}
	msr	spsr_fsxc, r2

	cps	#MODE32_und
	ldm	r1!, {r2, sp, lr}
	msr	spsr_fsxc, r2

	cps	#MODE32_mon
	ldm	r1!, {r2}
	msr	spsr_fsxc, r2

	stcopr	r4, SCR
	isb
#else
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
#endif

	/* Restore the LR */
	ldr	lr, [r0, #SMC_CTX_LR_MON]

	/* Restore the rest of the general purpose registers */
	ldm	r0, {r0-r12}
	eret
	.endm

#endif /* SMCCC_MACROS_S */
