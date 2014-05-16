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

#include <arch.h>
#include <asm_macros.S>
#include <tsp.h>


	.globl	tsp_entrypoint
	.globl  tsp_vector_table



	/* ---------------------------------------------
	 * Populate the params in x0-x7 from the pointer
	 * to the smc args structure in x0.
	 * ---------------------------------------------
	 */
	.macro restore_args_call_smc
	ldp	x6, x7, [x0, #TSP_ARG6]
	ldp	x4, x5, [x0, #TSP_ARG4]
	ldp	x2, x3, [x0, #TSP_ARG2]
	ldp	x0, x1, [x0, #TSP_ARG0]
	smc	#0
	.endm

	.macro	save_eret_context reg1 reg2
	mrs	\reg1, elr_el1
	mrs	\reg2, spsr_el1
	stp	\reg1, \reg2, [sp, #-0x10]!
	stp	x30, x18, [sp, #-0x10]!
	.endm

	.macro restore_eret_context reg1 reg2
	ldp	x30, x18, [sp], #0x10
	ldp	\reg1, \reg2, [sp], #0x10
	msr	elr_el1, \reg1
	msr	spsr_el1, \reg2
	.endm

	.section	.text, "ax"
	.align 3

func tsp_entrypoint

	/* ---------------------------------------------
	 * The entrypoint is expected to be executed
	 * only by the primary cpu (at least for now).
	 * So, make sure no secondary has lost its way.
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_is_primary_cpu
	cbz	x0, tsp_entrypoint_panic

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	adr	x0, tsp_exceptions
	msr	vbar_el1, x0

	/* ---------------------------------------------
	 * Enable the instruction cache.
	 * ---------------------------------------------
	 */
	mrs	x0, sctlr_el1
	orr	x0, x0, #SCTLR_I_BIT
	msr	sctlr_el1, x0
	isb

	/* ---------------------------------------------
	 * Zero out NOBITS sections. There are 2 of them:
	 *   - the .bss section;
	 *   - the coherent memory section.
	 * ---------------------------------------------
	 */
	ldr	x0, =__BSS_START__
	ldr	x1, =__BSS_SIZE__
	bl	zeromem16

	ldr	x0, =__COHERENT_RAM_START__
	ldr	x1, =__COHERENT_RAM_UNALIGNED_SIZE__
	bl	zeromem16

	/* --------------------------------------------
	 * Give ourselves a small coherent stack to
	 * ease the pain of initializing the MMU
	 * --------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_coherent_stack

	/* ---------------------------------------------
	 * Perform early platform setup & platform
	 * specific early arch. setup e.g. mmu setup
	 * ---------------------------------------------
	 */
	bl	bl32_early_platform_setup
	bl	bl32_plat_arch_setup

	/* ---------------------------------------------
	 * Give ourselves a stack allocated in Normal
	 * -IS-WBWA memory
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_stack

	/* ---------------------------------------------
	 * Jump to main function.
	 * ---------------------------------------------
	 */
	bl	tsp_main

	/* ---------------------------------------------
	 * Tell TSPD that we are done initialising
	 * ---------------------------------------------
	 */
	mov	x1, x0
	mov	x0, #TSP_ENTRY_DONE
	smc	#0

tsp_entrypoint_panic:
	b	tsp_entrypoint_panic


	/* -------------------------------------------
	 * Table of entrypoint vectors provided to the
	 * TSPD for the various entrypoints
	 * -------------------------------------------
	 */
func tsp_vector_table
	b	tsp_std_smc_entry
	b	tsp_fast_smc_entry
	b	tsp_cpu_on_entry
	b	tsp_cpu_off_entry
	b	tsp_cpu_resume_entry
	b	tsp_cpu_suspend_entry
	b	tsp_fiq_entry

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD when this
	 * cpu is to be turned off through a CPU_OFF
	 * psci call to ask the TSP to perform any
	 * bookeeping necessary. In the current
	 * implementation, the TSPD expects the TSP to
	 * re-initialise its state so nothing is done
	 * here except for acknowledging the request.
	 * ---------------------------------------------
	 */
func tsp_cpu_off_entry
	bl	tsp_cpu_off_main
	restore_args_call_smc

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD when this
	 * cpu is turned on using a CPU_ON psci call to
	 * ask the TSP to initialise itself i.e. setup
	 * the mmu, stacks etc. Minimal architectural
	 * state will be initialised by the TSPD when
	 * this function is entered i.e. Caches and MMU
	 * will be turned off, the execution state
	 * will be aarch64 and exceptions masked.
	 * ---------------------------------------------
	 */
func tsp_cpu_on_entry
	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	adr	x0, tsp_exceptions
	msr	vbar_el1, x0

	/* ---------------------------------------------
	 * Enable the instruction cache.
	 * ---------------------------------------------
	 */
	mrs	x0, sctlr_el1
	orr	x0, x0, #SCTLR_I_BIT
	msr	sctlr_el1, x0
	isb

	/* --------------------------------------------
	 * Give ourselves a small coherent stack to
	 * ease the pain of initializing the MMU
	 * --------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_coherent_stack

	/* ---------------------------------------------
	 * Initialise the MMU
	 * ---------------------------------------------
	 */
	bl	bl32_plat_enable_mmu

	/* ---------------------------------------------
	 * Give ourselves a stack allocated in Normal
	 * -IS-WBWA memory
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_stack

	/* ---------------------------------------------
	 * Enter C runtime to perform any remaining
	 * book keeping
	 * ---------------------------------------------
	 */
	bl	tsp_cpu_on_main
	restore_args_call_smc

	/* Should never reach here */
tsp_cpu_on_entry_panic:
	b	tsp_cpu_on_entry_panic

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD when this
	 * cpu is to be suspended through a CPU_SUSPEND
	 * psci call to ask the TSP to perform any
	 * bookeeping necessary. In the current
	 * implementation, the TSPD saves and restores
	 * the EL1 state.
	 * ---------------------------------------------
	 */
func tsp_cpu_suspend_entry
	bl	tsp_cpu_suspend_main
	restore_args_call_smc

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD to pass
	 * control for handling a pending S-EL1 FIQ.
	 * 'x0' contains a magic number which indicates
	 * this. TSPD expects control to be handed back
	 * at the end of FIQ processing. This is done
	 * through an SMC. The handover agreement is:
	 *
	 * 1. PSTATE.DAIF are set upon entry. 'x1' has
	 *    the ELR_EL3 from the non-secure state.
	 * 2. TSP has to preserve the callee saved
	 *    general purpose registers, SP_EL1/EL0 and
	 *    LR.
	 * 3. TSP has to preserve the system and vfp
	 *    registers (if applicable).
	 * 4. TSP can use 'x0-x18' to enable its C
	 *    runtime.
	 * 5. TSP returns to TSPD using an SMC with
	 *    'x0' = TSP_HANDLED_S_EL1_FIQ
	 * ---------------------------------------------
	 */
func	tsp_fiq_entry
#if DEBUG
	mov	x2, #(TSP_HANDLE_FIQ_AND_RETURN & ~0xffff)
	movk	x2, #(TSP_HANDLE_FIQ_AND_RETURN &  0xffff)
	cmp	x0, x2
	b.ne	tsp_fiq_entry_panic
#endif
	/*---------------------------------------------
	 * Save any previous context needed to perform
	 * an exception return from S-EL1 e.g. context
	 * from a previous IRQ. Update statistics and
	 * handle the FIQ before returning to the TSPD.
	 * IRQ/FIQs are not enabled since that will
	 * complicate the implementation. Execution
	 * will be transferred back to the normal world
	 * in any case. A non-zero return value from the
	 * fiq handler is an error.
	 * ---------------------------------------------
	 */
	save_eret_context x2 x3
	bl	tsp_update_sync_fiq_stats
	bl	tsp_fiq_handler
	cbnz	x0, tsp_fiq_entry_panic
	restore_eret_context x2 x3
	mov	x0, #(TSP_HANDLED_S_EL1_FIQ & ~0xffff)
	movk	x0, #(TSP_HANDLED_S_EL1_FIQ &  0xffff)
	smc	#0

tsp_fiq_entry_panic:
	b	tsp_fiq_entry_panic

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD when this
	 * cpu resumes execution after an earlier
	 * CPU_SUSPEND psci call to ask the TSP to
	 * restore its saved context. In the current
	 * implementation, the TSPD saves and restores
	 * EL1 state so nothing is done here apart from
	 * acknowledging the request.
	 * ---------------------------------------------
	 */
func tsp_cpu_resume_entry
	bl	tsp_cpu_resume_main
	restore_args_call_smc
tsp_cpu_resume_panic:
	b	tsp_cpu_resume_panic

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD to ask
	 * the TSP to service a fast smc request.
	 * ---------------------------------------------
	 */
func tsp_fast_smc_entry
	bl	tsp_smc_handler
	restore_args_call_smc
tsp_fast_smc_entry_panic:
	b	tsp_fast_smc_entry_panic

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD to ask
	 * the TSP to service a std smc request.
	 * We will enable preemption during execution
	 * of tsp_smc_handler.
	 * ---------------------------------------------
	 */
func tsp_std_smc_entry
	msr	daifclr, #DAIF_FIQ_BIT | DAIF_IRQ_BIT
	bl	tsp_smc_handler
	msr	daifset, #DAIF_FIQ_BIT | DAIF_IRQ_BIT
	restore_args_call_smc
tsp_std_smc_entry_panic:
	b	tsp_std_smc_entry_panic
