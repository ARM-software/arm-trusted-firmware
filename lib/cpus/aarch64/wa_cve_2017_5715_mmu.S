/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <services/arm_arch_svc.h>

	.globl	wa_cve_2017_5715_mmu_vbar

#define ESR_EL3_A64_SMC0	0x5e000000
#define ESR_EL3_A32_SMC0	0x4e000000

vector_base wa_cve_2017_5715_mmu_vbar

	.macro	apply_cve_2017_5715_wa _is_sync_exception _esr_el3_val
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	mrs	x1, sctlr_el3
	/* Disable MMU */
	bic	x1, x1, #SCTLR_M_BIT
	msr	sctlr_el3, x1
	isb
	/* Enable MMU */
	orr	x1, x1, #SCTLR_M_BIT
	msr	sctlr_el3, x1
	/*
	 * Defer ISB to avoid synchronizing twice in case we hit
	 * the workaround SMC call which will implicitly synchronize
	 * because of the ERET instruction.
	 */

	/*
	 * Ensure SMC is coming from A64/A32 state on #0
	 * with W0 = SMCCC_ARCH_WORKAROUND_1 or W0 = SMCCC_ARCH_WORKAROUND_3
	 *
	 * This sequence evaluates as:
	 *    (W0==SMCCC_ARCH_WORKAROUND_1) || (W0==SMCCC_ARCH_WORKAROUND_3) ?
	 *    (ESR_EL3==SMC#0) : (NE)
	 * allowing use of a single branch operation
	 */
	.if \_is_sync_exception
		orr	w1, wzr, #SMCCC_ARCH_WORKAROUND_1
		cmp	w0, w1
		orr	w1, wzr, #SMCCC_ARCH_WORKAROUND_3
		ccmp	w0, w1, #4, ne
		mrs	x0, esr_el3
		mov_imm	w1, \_esr_el3_val
		ccmp	w0, w1, #0, eq
		/* Static predictor will predict a fall through */
		bne	1f
		exception_return
1:
	.endif

	/*
	 * Synchronize now to enable the MMU.  This is required
	 * to ensure the load pair below reads the data stored earlier.
	 */
	isb
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	.endm

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry mmu_sync_exception_sp_el0
	b	sync_exception_sp_el0
end_vector_entry mmu_sync_exception_sp_el0

vector_entry mmu_irq_sp_el0
	b	irq_sp_el0
end_vector_entry mmu_irq_sp_el0

vector_entry mmu_fiq_sp_el0
	b	fiq_sp_el0
end_vector_entry mmu_fiq_sp_el0

vector_entry mmu_serror_sp_el0
	b	serror_sp_el0
end_vector_entry mmu_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry mmu_sync_exception_sp_elx
	b	sync_exception_sp_elx
end_vector_entry mmu_sync_exception_sp_elx

vector_entry mmu_irq_sp_elx
	b	irq_sp_elx
end_vector_entry mmu_irq_sp_elx

vector_entry mmu_fiq_sp_elx
	b	fiq_sp_elx
end_vector_entry mmu_fiq_sp_elx

vector_entry mmu_serror_sp_elx
	b	serror_sp_elx
end_vector_entry mmu_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry mmu_sync_exception_aarch64
	apply_cve_2017_5715_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A64_SMC0
	b	sync_exception_aarch64
end_vector_entry mmu_sync_exception_aarch64

vector_entry mmu_irq_aarch64
	apply_cve_2017_5715_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	irq_aarch64
end_vector_entry mmu_irq_aarch64

vector_entry mmu_fiq_aarch64
	apply_cve_2017_5715_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	fiq_aarch64
end_vector_entry mmu_fiq_aarch64

vector_entry mmu_serror_aarch64
	apply_cve_2017_5715_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	serror_aarch64
end_vector_entry mmu_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry mmu_sync_exception_aarch32
	apply_cve_2017_5715_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A32_SMC0
	b	sync_exception_aarch32
end_vector_entry mmu_sync_exception_aarch32

vector_entry mmu_irq_aarch32
	apply_cve_2017_5715_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	irq_aarch32
end_vector_entry mmu_irq_aarch32

vector_entry mmu_fiq_aarch32
	apply_cve_2017_5715_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	fiq_aarch32
end_vector_entry mmu_fiq_aarch32

vector_entry mmu_serror_aarch32
	apply_cve_2017_5715_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	serror_aarch32
end_vector_entry mmu_serror_aarch32
