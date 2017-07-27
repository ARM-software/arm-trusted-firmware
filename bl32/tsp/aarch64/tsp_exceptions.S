/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <tsp.h>


	/* ----------------------------------------------------
	 * The caller-saved registers x0-x18 and LR are saved
	 * here.
	 * ----------------------------------------------------
	 */

#define SCRATCH_REG_SIZE #(20 * 8)

	.macro save_caller_regs_and_lr
	sub	sp, sp, SCRATCH_REG_SIZE
	stp	x0, x1, [sp]
	stp	x2, x3, [sp, #0x10]
	stp	x4, x5, [sp, #0x20]
	stp	x6, x7, [sp, #0x30]
	stp	x8, x9, [sp, #0x40]
	stp	x10, x11, [sp, #0x50]
	stp	x12, x13, [sp, #0x60]
	stp	x14, x15, [sp, #0x70]
	stp	x16, x17, [sp, #0x80]
	stp	x18, x30, [sp, #0x90]
	.endm

	.macro restore_caller_regs_and_lr
	ldp	x0, x1, [sp]
	ldp	x2, x3, [sp, #0x10]
	ldp	x4, x5, [sp, #0x20]
	ldp	x6, x7, [sp, #0x30]
	ldp	x8, x9, [sp, #0x40]
	ldp	x10, x11, [sp, #0x50]
	ldp	x12, x13, [sp, #0x60]
	ldp	x14, x15, [sp, #0x70]
	ldp	x16, x17, [sp, #0x80]
	ldp	x18, x30, [sp, #0x90]
	add	sp, sp, SCRATCH_REG_SIZE
	.endm

	/* ----------------------------------------------------
	 * Common TSP interrupt handling routine
	 * ----------------------------------------------------
	 */
	.macro	handle_tsp_interrupt label
	/* Enable the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	save_caller_regs_and_lr
	bl	tsp_common_int_handler
	cbz	x0, interrupt_exit_\label

	/*
	 * This interrupt was not targetted to S-EL1 so send it to
	 * the monitor and wait for execution to resume.
	 */
	smc	#0
interrupt_exit_\label:
	restore_caller_regs_and_lr
	eret
	.endm

	.globl	tsp_exceptions

	/* -----------------------------------------------------
	 * TSP exception handlers.
	 * -----------------------------------------------------
	 */
vector_base tsp_exceptions
	/* -----------------------------------------------------
	 * Current EL with _sp_el0 : 0x0 - 0x200. No exceptions
	 * are expected and treated as irrecoverable errors.
	 * -----------------------------------------------------
	 */
vector_entry sync_exception_sp_el0
	b	plat_panic_handler
	check_vector_size sync_exception_sp_el0

vector_entry irq_sp_el0
	b	plat_panic_handler
	check_vector_size irq_sp_el0

vector_entry fiq_sp_el0
	b	plat_panic_handler
	check_vector_size fiq_sp_el0

vector_entry serror_sp_el0
	b	plat_panic_handler
	check_vector_size serror_sp_el0


	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400. Only IRQs/FIQs
	 * are expected and handled
	 * -----------------------------------------------------
	 */
vector_entry sync_exception_sp_elx
	b	plat_panic_handler
	check_vector_size sync_exception_sp_elx

vector_entry irq_sp_elx
	handle_tsp_interrupt irq_sp_elx
	check_vector_size irq_sp_elx

vector_entry fiq_sp_elx
	handle_tsp_interrupt fiq_sp_elx
	check_vector_size fiq_sp_elx

vector_entry serror_sp_elx
	b	plat_panic_handler
	check_vector_size serror_sp_elx


	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600. No exceptions
	 * are handled since TSP does not implement a lower EL
	 * -----------------------------------------------------
	 */
vector_entry sync_exception_aarch64
	b	plat_panic_handler
	check_vector_size sync_exception_aarch64

vector_entry irq_aarch64
	b	plat_panic_handler
	check_vector_size irq_aarch64

vector_entry fiq_aarch64
	b	plat_panic_handler
	check_vector_size fiq_aarch64

vector_entry serror_aarch64
	b	plat_panic_handler
	check_vector_size serror_aarch64


	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800. No exceptions
	 * handled since the TSP does not implement a lower EL.
	 * -----------------------------------------------------
	 */
vector_entry sync_exception_aarch32
	b	plat_panic_handler
	check_vector_size sync_exception_aarch32

vector_entry irq_aarch32
	b	plat_panic_handler
	check_vector_size irq_aarch32

vector_entry fiq_aarch32
	b	plat_panic_handler
	check_vector_size fiq_aarch32

vector_entry serror_aarch32
	b	plat_panic_handler
	check_vector_size serror_aarch32
