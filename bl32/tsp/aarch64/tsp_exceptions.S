/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <bl_common.h>
#include <arch.h>
#include <tsp.h>
#include <asm_macros.S>


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
	.section	.vectors, "ax"; .align 11

	.align	7
tsp_exceptions:
	/* -----------------------------------------------------
	 * Current EL with _sp_el0 : 0x0 - 0x180. No exceptions
	 * are expected and treated as irrecoverable errors.
	 * -----------------------------------------------------
	 */
sync_exception_sp_el0:
	wfi
	b	sync_exception_sp_el0
	check_vector_size sync_exception_sp_el0

	.align	7

irq_sp_el0:
	b	irq_sp_el0
	check_vector_size irq_sp_el0

	.align	7
fiq_sp_el0:
	b	fiq_sp_el0
	check_vector_size fiq_sp_el0

	.align	7
serror_sp_el0:
	b	serror_sp_el0
	check_vector_size serror_sp_el0


	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x380. Only IRQs/FIQs
	 * are expected and handled
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_sp_elx:
	wfi
	b	sync_exception_sp_elx
	check_vector_size sync_exception_sp_elx

	.align	7
irq_sp_elx:
	handle_tsp_interrupt irq_sp_elx
	check_vector_size irq_sp_elx

	.align	7
fiq_sp_elx:
	handle_tsp_interrupt fiq_sp_elx
	check_vector_size fiq_sp_elx

	.align	7
serror_sp_elx:
	b	serror_sp_elx
	check_vector_size serror_sp_elx


	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x580. No exceptions
	 * are handled since TSP does not implement a lower EL
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_aarch64:
	wfi
	b	sync_exception_aarch64
	check_vector_size sync_exception_aarch64

	.align	7
irq_aarch64:
	b	irq_aarch64
	check_vector_size irq_aarch64

	.align	7
fiq_aarch64:
	b	fiq_aarch64
	check_vector_size fiq_aarch64

	.align	7
serror_aarch64:
	b	serror_aarch64
	check_vector_size serror_aarch64


	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x780. No exceptions
	 * handled since the TSP does not implement a lower EL.
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_aarch32:
	wfi
	b	sync_exception_aarch32
	check_vector_size sync_exception_aarch32

	.align	7
irq_aarch32:
	b	irq_aarch32
	check_vector_size irq_aarch32

	.align	7
fiq_aarch32:
	b	fiq_aarch32
	check_vector_size fiq_aarch32

	.align	7
serror_aarch32:
	b	serror_aarch32
	check_vector_size serror_aarch32
	.align	7
