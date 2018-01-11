/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <context.h>

	.globl	workaround_mmu_runtime_exceptions

vector_base workaround_mmu_runtime_exceptions

	.macro	apply_workaround
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	mrs	x0, sctlr_el3
	/* Disable MMU */
	bic	x1, x0, #SCTLR_M_BIT
	msr	sctlr_el3, x1
	isb
	/* Restore MMU config */
	msr	sctlr_el3, x0
	isb
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	.endm

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_mmu_sync_exception_sp_el0
	b	sync_exception_sp_el0
	check_vector_size workaround_mmu_sync_exception_sp_el0

vector_entry workaround_mmu_irq_sp_el0
	b	irq_sp_el0
	check_vector_size workaround_mmu_irq_sp_el0

vector_entry workaround_mmu_fiq_sp_el0
	b	fiq_sp_el0
	check_vector_size workaround_mmu_fiq_sp_el0

vector_entry workaround_mmu_serror_sp_el0
	b	serror_sp_el0
	check_vector_size workaround_mmu_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_mmu_sync_exception_sp_elx
	b	sync_exception_sp_elx
	check_vector_size workaround_mmu_sync_exception_sp_elx

vector_entry workaround_mmu_irq_sp_elx
	b	irq_sp_elx
	check_vector_size workaround_mmu_irq_sp_elx

vector_entry workaround_mmu_fiq_sp_elx
	b	fiq_sp_elx
	check_vector_size workaround_mmu_fiq_sp_elx

vector_entry workaround_mmu_serror_sp_elx
	b	serror_sp_elx
	check_vector_size workaround_mmu_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_mmu_sync_exception_aarch64
	apply_workaround
	b	sync_exception_aarch64
	check_vector_size workaround_mmu_sync_exception_aarch64

vector_entry workaround_mmu_irq_aarch64
	apply_workaround
	b	irq_aarch64
	check_vector_size workaround_mmu_irq_aarch64

vector_entry workaround_mmu_fiq_aarch64
	apply_workaround
	b	fiq_aarch64
	check_vector_size workaround_mmu_fiq_aarch64

vector_entry workaround_mmu_serror_aarch64
	apply_workaround
	b	serror_aarch64
	check_vector_size workaround_mmu_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_mmu_sync_exception_aarch32
	apply_workaround
	b	sync_exception_aarch32
	check_vector_size workaround_mmu_sync_exception_aarch32

vector_entry workaround_mmu_irq_aarch32
	apply_workaround
	b	irq_aarch32
	check_vector_size workaround_mmu_irq_aarch32

vector_entry workaround_mmu_fiq_aarch32
	apply_workaround
	b	fiq_aarch32
	check_vector_size workaround_mmu_fiq_aarch32

vector_entry workaround_mmu_serror_aarch32
	apply_workaround
	b	serror_aarch32
	check_vector_size workaround_mmu_serror_aarch32
