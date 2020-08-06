/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
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
vector_base workaround_bpflush_runtime_exceptions

	.macro	apply_workaround
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]

	/* Disable cycle counter when event counting is prohibited */
	mrs	x1, pmcr_el0
	orr	x0, x1, #PMCR_EL0_DP_BIT
	msr	pmcr_el0, x0
	isb

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
	/* DCO is not supported on PN5 and later */
	mrs	x1, midr_el1
	mov_imm	x2, DENVER_MIDR_PN4
	cmp	x1, x2
	b.hi	1f

	mov	x18, x30
	bl	plat_my_core_pos
	mov	x1, #1
	lsl	x1, x1, x0
	msr	s3_0_c15_c0_2, x1
	mov	x30, x18
1:	ret
endfunc denver_enable_dco

	/* ----------------------------------------------------
	 * Disable dynamic code optimizer (DCO)
	 * ----------------------------------------------------
	 */
func denver_disable_dco
	/* DCO is not supported on PN5 and later */
	mrs	x1, midr_el1
	mov_imm	x2, DENVER_MIDR_PN4
	cmp	x1, x2
	b.hi	2f

	/* turn off background work */
	mov	x18, x30
	bl	plat_my_core_pos
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

	mov	x30, x18
2:	ret
endfunc denver_disable_dco

func check_errata_cve_2017_5715
	mov	x0, #ERRATA_MISSING
#if WORKAROUND_CVE_2017_5715
	/*
	 * Check if the CPU supports the special instruction
	 * required to flush the indirect branch predictor and
	 * RSB. Support for this operation can be determined by
	 * comparing bits 19:16 of ID_AFR0_EL1 with 0b0001.
	 */
	mrs	x1, id_afr0_el1
	mov	x2, #0x10000
	and	x1, x1, x2
	cbz	x1, 1f
	mov	x0, #ERRATA_APPLIES
1:
#endif
	ret
endfunc check_errata_cve_2017_5715

func check_errata_cve_2018_3639
#if WORKAROUND_CVE_2018_3639
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2018_3639

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

#if WORKAROUND_CVE_2018_3639
	/*
	 * Denver CPUs with DENVER_MIDR_PN3 or earlier, use different
	 * bits in the ACTLR_EL3 register to disable speculative
	 * store buffer and memory disambiguation.
	 */
	mrs	x0, midr_el1
	mov_imm	x1, DENVER_MIDR_PN4
	cmp	x0, x1
	mrs	x0, actlr_el3
	mov	x1, #(DENVER_CPU_DIS_MD_EL3 | DENVER_CPU_DIS_SSB_EL3)
	mov	x2, #(DENVER_PN4_CPU_DIS_MD_EL3 | DENVER_PN4_CPU_DIS_SSB_EL3)
	csel	x3, x1, x2, ne
	orr	x0, x0, x3
	msr	actlr_el3, x0
	isb
	dsb	sy
#endif

	/* ----------------------------------------------------
	 * Reset ACTLR.PMSTATE to C1 state
	 * ----------------------------------------------------
	 */
	mrs	x0, actlr_el1
	bic	x0, x0, #DENVER_CPU_PMSTATE_MASK
	orr	x0, x0, #DENVER_CPU_PMSTATE_C1
	msr	actlr_el1, x0

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

#if REPORT_ERRATA
	/*
	 * Errata printing function for Denver. Must follow AAPCS.
	 */
func denver_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata WORKAROUND_CVE_2017_5715, denver, cve_2017_5715
	report_errata WORKAROUND_CVE_2018_3639, denver, cve_2018_3639

	ldp	x8, x30, [sp], #16
	ret
endfunc denver_errata_report
#endif

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

/* macro to declare cpu_ops for Denver SKUs */
.macro	denver_cpu_ops_wa midr
	declare_cpu_ops_wa denver, \midr, \
		denver_reset_func, \
		check_errata_cve_2017_5715, \
		CPU_NO_EXTRA2_FUNC, \
		denver_core_pwr_dwn, \
		denver_cluster_pwr_dwn
.endm

denver_cpu_ops_wa DENVER_MIDR_PN0
denver_cpu_ops_wa DENVER_MIDR_PN1
denver_cpu_ops_wa DENVER_MIDR_PN2
denver_cpu_ops_wa DENVER_MIDR_PN3
denver_cpu_ops_wa DENVER_MIDR_PN4
denver_cpu_ops_wa DENVER_MIDR_PN5
denver_cpu_ops_wa DENVER_MIDR_PN6
denver_cpu_ops_wa DENVER_MIDR_PN7
denver_cpu_ops_wa DENVER_MIDR_PN8
denver_cpu_ops_wa DENVER_MIDR_PN9
