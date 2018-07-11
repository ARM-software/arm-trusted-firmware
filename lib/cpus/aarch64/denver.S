/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <context.h>
#include <denver.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* -------------------------------------------------
	 * CVE-2017-5715 mitigation
	 *
	 * Flush the indirect branch predictor and RSB on
	 * entry to EL3 by issuing a newly added instruction
	 * for Denver CPUs.
	 *
	 * To achieve this without performing any branch
	 * instruction, a per-cpu vbar is installed which
	 * executes the workaround and then branches off to
	 * the corresponding vector entry in the main vector
	 * table.
	 * -------------------------------------------------
	 */
	.globl	workaround_bpflush_runtime_exceptions

vector_base workaround_bpflush_runtime_exceptions

	.macro	apply_workaround
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]

	/* -------------------------------------------------
	 * A new write-only system register where a write of
	 * 1 to bit 0 will cause the indirect branch predictor
	 * and RSB to be flushed.
	 *
	 * A write of 0 to bit 0 will be ignored. A write of
	 * 1 to any other bit will cause an MCA.
	 * -------------------------------------------------
	 */
	mov	x0, #1
	msr	s3_0_c15_c0_6, x0
	isb

	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	.endm

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpflush_sync_exception_sp_el0
	b	sync_exception_sp_el0
end_vector_entry workaround_bpflush_sync_exception_sp_el0

vector_entry workaround_bpflush_irq_sp_el0
	b	irq_sp_el0
end_vector_entry workaround_bpflush_irq_sp_el0

vector_entry workaround_bpflush_fiq_sp_el0
	b	fiq_sp_el0
end_vector_entry workaround_bpflush_fiq_sp_el0

vector_entry workaround_bpflush_serror_sp_el0
	b	serror_sp_el0
end_vector_entry workaround_bpflush_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpflush_sync_exception_sp_elx
	b	sync_exception_sp_elx
end_vector_entry workaround_bpflush_sync_exception_sp_elx

vector_entry workaround_bpflush_irq_sp_elx
	b	irq_sp_elx
end_vector_entry workaround_bpflush_irq_sp_elx

vector_entry workaround_bpflush_fiq_sp_elx
	b	fiq_sp_elx
end_vector_entry workaround_bpflush_fiq_sp_elx

vector_entry workaround_bpflush_serror_sp_elx
	b	serror_sp_elx
end_vector_entry workaround_bpflush_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpflush_sync_exception_aarch64
	apply_workaround
	b	sync_exception_aarch64
end_vector_entry workaround_bpflush_sync_exception_aarch64

vector_entry workaround_bpflush_irq_aarch64
	apply_workaround
	b	irq_aarch64
end_vector_entry workaround_bpflush_irq_aarch64

vector_entry workaround_bpflush_fiq_aarch64
	apply_workaround
	b	fiq_aarch64
end_vector_entry workaround_bpflush_fiq_aarch64

vector_entry workaround_bpflush_serror_aarch64
	apply_workaround
	b	serror_aarch64
end_vector_entry workaround_bpflush_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpflush_sync_exception_aarch32
	apply_workaround
	b	sync_exception_aarch32
end_vector_entry workaround_bpflush_sync_exception_aarch32

vector_entry workaround_bpflush_irq_aarch32
	apply_workaround
	b	irq_aarch32
end_vector_entry workaround_bpflush_irq_aarch32

vector_entry workaround_bpflush_fiq_aarch32
	apply_workaround
	b	fiq_aarch32
end_vector_entry workaround_bpflush_fiq_aarch32

vector_entry workaround_bpflush_serror_aarch32
	apply_workaround
	b	serror_aarch32
end_vector_entry workaround_bpflush_serror_aarch32

	.global	denver_disable_dco

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func denver_disable_ext_debug
	mov	x0, #1
	msr	osdlr_el1, x0
	isb
	dsb	sy
	ret
endfunc denver_disable_ext_debug

	/* ----------------------------------------------------
	 * Enable dynamic code optimizer (DCO)
	 * ----------------------------------------------------
	 */
func denver_enable_dco
	mrs	x0, mpidr_el1
	and	x0, x0, #0xF
	mov	x1, #1
	lsl	x1, x1, x0
	msr	s3_0_c15_c0_2, x1
	ret
endfunc denver_enable_dco

	/* ----------------------------------------------------
	 * Disable dynamic code optimizer (DCO)
	 * ----------------------------------------------------
	 */
func denver_disable_dco

	/* turn off background work */
	mrs	x0, mpidr_el1
	and	x0, x0, #0xF
	mov	x1, #1
	lsl	x1, x1, x0
	lsl	x2, x1, #16
	msr	s3_0_c15_c0_2, x2
	isb

	/* wait till the background work turns off */
1:	mrs	x2, s3_0_c15_c0_2
	lsr	x2, x2, #32
	and	w2, w2, 0xFFFF
	and	x2, x2, x1
	cbnz	x2, 1b

	ret
endfunc denver_disable_dco

	/* -------------------------------------------------
	 * The CPU Ops reset function for Denver.
	 * -------------------------------------------------
	 */
func denver_reset_func

	mov	x19, x30

#if IMAGE_BL31 && WORKAROUND_CVE_2017_5715
	/*
	 * Check if the CPU supports the special instruction
	 * required to flush the indirect branch predictor and
	 * RSB. Support for this operation can be determined by
	 * comparing bits 19:16 of ID_AFR0_EL1 with 0b0001.
	 */
	mrs	x0, id_afr0_el1
	mov	x1, #0x10000
	and	x0, x0, x1
	cmp	x0, #0
	adr	x1, workaround_bpflush_runtime_exceptions
	mrs	x2, vbar_el3
	csel	x0, x1, x2, ne
	msr	vbar_el3, x0
#endif

	/* ----------------------------------------------------
	 * Enable dynamic code optimizer (DCO)
	 * ----------------------------------------------------
	 */
	bl	denver_enable_dco

	ret	x19
endfunc denver_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Denver.
	 * ----------------------------------------------------
	 */
func denver_core_pwr_dwn

	mov	x19, x30

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	bl	denver_disable_ext_debug

	ret	x19
endfunc denver_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Denver.
	 * -------------------------------------------------------
	 */
func denver_cluster_pwr_dwn
	ret
endfunc denver_cluster_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Denver specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.denver_regs, "aS"
denver_regs:  /* The ascii list of register names to be reported */
	.asciz	"actlr_el1", ""

func denver_cpu_reg_dump
	adr	x6, denver_regs
	mrs	x8, ACTLR_EL1
	ret
endfunc denver_cpu_reg_dump

declare_cpu_ops denver, DENVER_MIDR_PN0, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN1, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN2, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN3, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN4, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn
