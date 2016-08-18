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
#ifndef __SMCC_MACROS_S__
#define __SMCC_MACROS_S__

#include <arch.h>

/*
 * Macro to save the General purpose registers including the banked
 * registers to the SMC context on entry due a SMC call. On return, r0
 * contains the pointer to the `smc_context_t`.
 */
	.macro smcc_save_gp_mode_regs
	push	{r0-r3, lr}

	ldcopr	r0, SCR
	and	r0, r0, #SCR_NS_BIT
	bl	smc_get_ctx

	/* Save r4 - r12 in the SMC context */
	add	r1, r0, #SMC_CTX_GPREG_R4
	stm	r1!, {r4-r12}

	/*
	 * Pop r0 - r3, lr to r4 - r7, lr from stack and then save
	 * it to SMC context.
	 */
	pop	{r4-r7, lr}
	stm	r0, {r4-r7}

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
	stm	r1!, {r4-r12}

	mrs	r4, sp_svc
	mrs	r5, lr_svc
	mrs	r6, spsr_abt
	mrs	r7, sp_abt
	mrs	r8, lr_abt
	mrs	r9, spsr_und
	mrs	r10, sp_und
	mrs	r11, lr_und
	mrs	r12, spsr
	stm	r1!, {r4-r12, lr}

	.endm

/*
 * Macro to restore the General purpose registers including the banked
 * registers from the SMC context prior to exit from the SMC call.
 * r0 must point to the `smc_context_t` to restore from.
 */
	.macro smcc_restore_gp_mode_regs

	/* Restore the banked registers including the current SPSR and LR */
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

	ldm	r1!, {r4-r12, lr}
	msr	sp_svc, r4
	msr	lr_svc, r5
	msr	spsr_abt, r6
	msr	sp_abt, r7
	msr	lr_abt, r8
	msr	spsr_und, r9
	msr	sp_und, r10
	msr	lr_und, r11
	msr	spsr, r12

	/* Restore the rest of the general purpose registers */
	ldm	r0, {r0-r12}
	.endm

#endif /* __SMCC_MACROS_S__ */
