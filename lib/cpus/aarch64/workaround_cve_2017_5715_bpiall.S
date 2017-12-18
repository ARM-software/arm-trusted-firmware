/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <context.h>

	.globl	workaround_bpiall_vbar0_runtime_exceptions

#define EMIT_BPIALL		0xee070fd5
#define EMIT_MOV_R0_IMM(v)	0xe3a0000##v
#define EMIT_SMC		0xe1600070

	.macro	enter_workaround _stub_name
	/* Save GP regs */
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	stp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]
	stp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	stp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	stp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	stp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	stp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	stp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	stp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	stp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	stp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	stp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	stp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]

	adr	x4, \_stub_name

	/*
	 * Load SPSR_EL3 and VBAR_EL3.  SPSR_EL3 is set up to have
	 * all interrupts masked in preparation to running the workaround
	 * stub in S-EL1.  VBAR_EL3 points to the vector table that
	 * will handle the SMC back from the workaround stub.
	 */
	ldp	x0, x1, [x4, #0]

	/*
	 * Load SCTLR_EL1 and ELR_EL3.  SCTLR_EL1 is configured to disable
	 * the MMU in S-EL1.  ELR_EL3 points to the appropriate stub in S-EL1.
	 */
	ldp	x2, x3, [x4, #16]

	mrs	x4, scr_el3
	mrs	x5, spsr_el3
	mrs	x6, elr_el3
	mrs	x7, sctlr_el1
	mrs	x8, esr_el3

	/* Preserve system registers in the workaround context */
	stp	x4, x5, [sp, #CTX_CVE_2017_5715_OFFSET + CTX_CVE_2017_5715_QUAD0]
	stp	x6, x7, [sp, #CTX_CVE_2017_5715_OFFSET + CTX_CVE_2017_5715_QUAD2]
	stp	x8, x30, [sp, #CTX_CVE_2017_5715_OFFSET + CTX_CVE_2017_5715_QUAD4]

	/*
	 * Setting SCR_EL3 to all zeroes means that the NS, RW
	 * and SMD bits are configured as expected.
	 */
	msr	scr_el3, xzr

	/*
	 * Reload system registers with the crafted values
	 * in preparation for entry in S-EL1.
	 */
	msr	spsr_el3, x0
	msr	vbar_el3, x1
	msr	sctlr_el1, x2
	msr	elr_el3, x3

	eret
	.endm

	/* ---------------------------------------------------------------------
	 * This vector table is used at runtime to enter the workaround at
	 * AArch32 S-EL1 for Sync/IRQ/FIQ/SError exceptions.  If the workaround
	 * is not enabled, the existing runtime exception vector table is used.
	 * ---------------------------------------------------------------------
	 */
vector_base workaround_bpiall_vbar0_runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_sp_el0
	b	sync_exception_sp_el0
	/*
	 * Since each vector table entry is 128 bytes, we can store the
	 * stub context in the unused space to minimize memory footprint.
	 */
aarch32_stub_smc:
	.word	EMIT_BPIALL
	.word	EMIT_MOV_R0_IMM(1)
	.word	EMIT_SMC
aarch32_stub_ctx_smc:
	/* Mask all interrupts and set AArch32 Supervisor mode */
	.quad	(SPSR_AIF_MASK << SPSR_AIF_SHIFT | \
	         SPSR_M_AARCH32 << SPSR_M_SHIFT | \
	         MODE32_svc << MODE32_SHIFT)

	/*
	 * VBAR_EL3 points to vbar1 which is the vector table
	 * used while the workaround is executing.
	 */
	.quad	workaround_bpiall_vbar1_runtime_exceptions

	/* Setup SCTLR_EL1 with MMU off and I$ on */
	.quad	SCTLR_AARCH32_EL1_RES1 | SCTLR_I_BIT

	/* ELR_EL3 is setup to point to the sync exception stub in AArch32 */
	.quad	aarch32_stub_smc
	check_vector_size workaround_bpiall_vbar0_sync_exception_sp_el0

vector_entry workaround_bpiall_vbar0_irq_sp_el0
	b	irq_sp_el0
aarch32_stub_irq:
	.word	EMIT_BPIALL
	.word	EMIT_MOV_R0_IMM(2)
	.word	EMIT_SMC
aarch32_stub_ctx_irq:
	.quad	(SPSR_AIF_MASK << SPSR_AIF_SHIFT | \
	         SPSR_M_AARCH32 << SPSR_M_SHIFT | \
	         MODE32_svc << MODE32_SHIFT)
	.quad	workaround_bpiall_vbar1_runtime_exceptions
	.quad	SCTLR_AARCH32_EL1_RES1 | SCTLR_I_BIT
	.quad	aarch32_stub_irq
	check_vector_size workaround_bpiall_vbar0_irq_sp_el0

vector_entry workaround_bpiall_vbar0_fiq_sp_el0
	b	fiq_sp_el0
aarch32_stub_fiq:
	.word	EMIT_BPIALL
	.word	EMIT_MOV_R0_IMM(4)
	.word	EMIT_SMC
aarch32_stub_ctx_fiq:
	.quad	(SPSR_AIF_MASK << SPSR_AIF_SHIFT | \
	         SPSR_M_AARCH32 << SPSR_M_SHIFT | \
	         MODE32_svc << MODE32_SHIFT)
	.quad	workaround_bpiall_vbar1_runtime_exceptions
	.quad	SCTLR_AARCH32_EL1_RES1 | SCTLR_I_BIT
	.quad	aarch32_stub_fiq
	check_vector_size workaround_bpiall_vbar0_fiq_sp_el0

vector_entry workaround_bpiall_vbar0_serror_sp_el0
	b	serror_sp_el0
aarch32_stub_serror:
	.word	EMIT_BPIALL
	.word	EMIT_MOV_R0_IMM(8)
	.word	EMIT_SMC
aarch32_stub_ctx_serror:
	.quad	(SPSR_AIF_MASK << SPSR_AIF_SHIFT | \
	         SPSR_M_AARCH32 << SPSR_M_SHIFT | \
	         MODE32_svc << MODE32_SHIFT)
	.quad	workaround_bpiall_vbar1_runtime_exceptions
	.quad	SCTLR_AARCH32_EL1_RES1 | SCTLR_I_BIT
	.quad	aarch32_stub_serror
	check_vector_size workaround_bpiall_vbar0_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_sp_elx
	b	sync_exception_sp_elx
	check_vector_size workaround_bpiall_vbar0_sync_exception_sp_elx

vector_entry workaround_bpiall_vbar0_irq_sp_elx
	b	irq_sp_elx
	check_vector_size workaround_bpiall_vbar0_irq_sp_elx

vector_entry workaround_bpiall_vbar0_fiq_sp_elx
	b	fiq_sp_elx
	check_vector_size workaround_bpiall_vbar0_fiq_sp_elx

vector_entry workaround_bpiall_vbar0_serror_sp_elx
	b	serror_sp_elx
	check_vector_size workaround_bpiall_vbar0_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_aarch64
	enter_workaround aarch32_stub_ctx_smc
	check_vector_size workaround_bpiall_vbar0_sync_exception_aarch64

vector_entry workaround_bpiall_vbar0_irq_aarch64
	enter_workaround aarch32_stub_ctx_irq
	check_vector_size workaround_bpiall_vbar0_irq_aarch64

vector_entry workaround_bpiall_vbar0_fiq_aarch64
	enter_workaround aarch32_stub_ctx_fiq
	check_vector_size workaround_bpiall_vbar0_fiq_aarch64

vector_entry workaround_bpiall_vbar0_serror_aarch64
	enter_workaround aarch32_stub_ctx_serror
	check_vector_size workaround_bpiall_vbar0_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_aarch32
	enter_workaround aarch32_stub_ctx_smc
	check_vector_size workaround_bpiall_vbar0_sync_exception_aarch32

vector_entry workaround_bpiall_vbar0_irq_aarch32
	enter_workaround aarch32_stub_ctx_irq
	check_vector_size workaround_bpiall_vbar0_irq_aarch32

vector_entry workaround_bpiall_vbar0_fiq_aarch32
	enter_workaround aarch32_stub_ctx_fiq
	check_vector_size workaround_bpiall_vbar0_fiq_aarch32

vector_entry workaround_bpiall_vbar0_serror_aarch32
	enter_workaround aarch32_stub_ctx_serror
	check_vector_size workaround_bpiall_vbar0_serror_aarch32

	/* ---------------------------------------------------------------------
	 * This vector table is used while the workaround is executing.  It
	 * installs a simple SMC handler to allow the Sync/IRQ/FIQ/SError
	 * workaround stubs to enter EL3 from S-EL1.  It restores the previous
	 * EL3 state before proceeding with the normal runtime exception vector.
	 * ---------------------------------------------------------------------
	 */
vector_base workaround_bpiall_vbar1_runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200 (UNUSED)
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_sp_el0
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_sync_exception_sp_el0

vector_entry workaround_bpiall_vbar1_irq_sp_el0
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_irq_sp_el0

vector_entry workaround_bpiall_vbar1_fiq_sp_el0
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_fiq_sp_el0

vector_entry workaround_bpiall_vbar1_serror_sp_el0
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400 (UNUSED)
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_sp_elx
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_sync_exception_sp_elx

vector_entry workaround_bpiall_vbar1_irq_sp_elx
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_irq_sp_elx

vector_entry workaround_bpiall_vbar1_fiq_sp_elx
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_fiq_sp_elx

vector_entry workaround_bpiall_vbar1_serror_sp_elx
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600 (UNUSED)
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_aarch64
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_sync_exception_aarch64

vector_entry workaround_bpiall_vbar1_irq_aarch64
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_irq_aarch64

vector_entry workaround_bpiall_vbar1_fiq_aarch64
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_fiq_aarch64

vector_entry workaround_bpiall_vbar1_serror_aarch64
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_aarch32
	/* Restore register state from the workaround context */
	ldp	x2, x3, [sp, #CTX_CVE_2017_5715_OFFSET + CTX_CVE_2017_5715_QUAD0]
	ldp	x4, x5, [sp, #CTX_CVE_2017_5715_OFFSET + CTX_CVE_2017_5715_QUAD2]
	ldp	x6, x30, [sp, #CTX_CVE_2017_5715_OFFSET + CTX_CVE_2017_5715_QUAD4]

	/* Apply the restored system register state */
	msr	scr_el3, x2
	msr	spsr_el3, x3
	msr	elr_el3, x4
	msr	sctlr_el1, x5
	msr	esr_el3, x6

	/*
	 * Workaround is complete, so swap VBAR_EL3 to point
	 * to workaround entry table in preparation for subsequent
	 * Sync/IRQ/FIQ/SError exceptions.
	 */
	adr	x2, workaround_bpiall_vbar0_runtime_exceptions
	msr	vbar_el3, x2

	/*
	 * Restore all GP regs except x0 and x1.  The value in x0
	 * indicates the type of the original exception.
	 */
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	ldp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]
	ldp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	ldp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	ldp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	ldp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	ldp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	ldp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	ldp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	ldp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	ldp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	ldp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	ldp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]

	/*
	 * Each of these handlers will first restore x0 and x1 from
	 * the context and the branch to the common implementation for
	 * each of the exception types.
	 */
	tbnz	x0, #1, workaround_bpiall_vbar1_irq
	tbnz	x0, #2, workaround_bpiall_vbar1_fiq
	tbnz	x0, #3, workaround_bpiall_vbar1_serror

	/* Fallthrough case for Sync exception */
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	b	sync_exception_aarch64
	check_vector_size workaround_bpiall_vbar1_sync_exception_aarch32

vector_entry workaround_bpiall_vbar1_irq_aarch32
	b	report_unhandled_interrupt
workaround_bpiall_vbar1_irq:
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	b	irq_aarch64
	check_vector_size workaround_bpiall_vbar1_irq_aarch32

vector_entry workaround_bpiall_vbar1_fiq_aarch32
	b	report_unhandled_interrupt
workaround_bpiall_vbar1_fiq:
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	b	fiq_aarch64
	check_vector_size workaround_bpiall_vbar1_fiq_aarch32

vector_entry workaround_bpiall_vbar1_serror_aarch32
	b	report_unhandled_exception
workaround_bpiall_vbar1_serror:
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	b	serror_aarch64
	check_vector_size workaround_bpiall_vbar1_serror_aarch32
