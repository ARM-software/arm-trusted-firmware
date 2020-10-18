/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <bl32/tsp/tsp.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#include "../tsp_private.h"


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

func tsp_entrypoint _align=3

#if ENABLE_PIE
		/*
		 * ------------------------------------------------------------
		 * If PIE is enabled fixup the Global descriptor Table only
		 * once during primary core cold boot path.
		 *
		 * Compile time base address, required for fixup, is calculated
		 * using "pie_fixup" label present within first page.
		 * ------------------------------------------------------------
		 */
	pie_fixup:
		ldr	x0, =pie_fixup
		and	x0, x0, #~(PAGE_SIZE_MASK)
		mov_imm	x1, (BL32_LIMIT - BL32_BASE)
		add	x1, x1, x0
		bl	fixup_gdt_reloc
#endif /* ENABLE_PIE */

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	adr	x0, tsp_exceptions
	msr	vbar_el1, x0
	isb

	/* ---------------------------------------------
	 * Enable the SError interrupt now that the
	 * exception vectors have been setup.
	 * ---------------------------------------------
	 */
	msr	daifclr, #DAIF_ABT_BIT

	/* ---------------------------------------------
	 * Enable the instruction cache, stack pointer
	 * and data access alignment checks and disable
	 * speculative loads.
	 * ---------------------------------------------
	 */
	mov	x1, #(SCTLR_I_BIT | SCTLR_A_BIT | SCTLR_SA_BIT)
	mrs	x0, sctlr_el1
	orr	x0, x0, x1
	bic	x0, x0, #SCTLR_DSSBS_BIT
	msr	sctlr_el1, x0
	isb

	/* ---------------------------------------------
	 * Invalidate the RW memory used by the BL32
	 * image. This includes the data and NOBITS
	 * sections. This is done to safeguard against
	 * possible corruption of this memory by dirty
	 * cache lines in a system cache as a result of
	 * use by an earlier boot loader stage.
	 * ---------------------------------------------
	 */
	adr	x0, __RW_START__
	adr	x1, __RW_END__
	sub	x1, x1, x0
	bl	inv_dcache_range

	/* ---------------------------------------------
	 * Zero out NOBITS sections. There are 2 of them:
	 *   - the .bss section;
	 *   - the coherent memory section.
	 * ---------------------------------------------
	 */
	ldr	x0, =__BSS_START__
	ldr	x1, =__BSS_SIZE__
	bl	zeromem

#if USE_COHERENT_MEM
	ldr	x0, =__COHERENT_RAM_START__
	ldr	x1, =__COHERENT_RAM_UNALIGNED_SIZE__
	bl	zeromem
#endif

	/* --------------------------------------------
	 * Allocate a stack whose memory will be marked
	 * as Normal-IS-WBWA when the MMU is enabled.
	 * There is no risk of reading stale stack
	 * memory after enabling the MMU as only the
	 * primary cpu is running at the moment.
	 * --------------------------------------------
	 */
	bl	plat_set_my_stack

	/* ---------------------------------------------
	 * Initialize the stack protector canary before
	 * any C code is called.
	 * ---------------------------------------------
	 */
#if STACK_PROTECTOR_ENABLED
	bl	update_stack_protector_canary
#endif

	/* ---------------------------------------------
	 * Perform TSP setup
	 * ---------------------------------------------
	 */
	bl	tsp_setup

#if ENABLE_PAUTH
	/* ---------------------------------------------
	 * Program APIAKey_EL1
	 * and enable pointer authentication
	 * ---------------------------------------------
	 */
	bl	pauth_init_enable_el1
#endif /* ENABLE_PAUTH */

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
endfunc tsp_entrypoint


	/* -------------------------------------------
	 * Table of entrypoint vectors provided to the
	 * TSPD for the various entrypoints
	 * -------------------------------------------
	 */
vector_base tsp_vector_table
	b	tsp_yield_smc_entry
	b	tsp_fast_smc_entry
	b	tsp_cpu_on_entry
	b	tsp_cpu_off_entry
	b	tsp_cpu_resume_entry
	b	tsp_cpu_suspend_entry
	b	tsp_sel1_intr_entry
	b	tsp_system_off_entry
	b	tsp_system_reset_entry
	b	tsp_abort_yield_smc_entry

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
endfunc tsp_cpu_off_entry

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD when the
	 * system is about to be switched off (through
	 * a SYSTEM_OFF psci call) to ask the TSP to
	 * perform any necessary bookkeeping.
	 * ---------------------------------------------
	 */
func tsp_system_off_entry
	bl	tsp_system_off_main
	restore_args_call_smc
endfunc tsp_system_off_entry

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD when the
	 * system is about to be reset (through a
	 * SYSTEM_RESET psci call) to ask the TSP to
	 * perform any necessary bookkeeping.
	 * ---------------------------------------------
	 */
func tsp_system_reset_entry
	bl	tsp_system_reset_main
	restore_args_call_smc
endfunc tsp_system_reset_entry

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
	isb

	/* Enable the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	/* ---------------------------------------------
	 * Enable the instruction cache, stack pointer
	 * and data access alignment checks
	 * ---------------------------------------------
	 */
	mov	x1, #(SCTLR_I_BIT | SCTLR_A_BIT | SCTLR_SA_BIT)
	mrs	x0, sctlr_el1
	orr	x0, x0, x1
	msr	sctlr_el1, x0
	isb

	/* --------------------------------------------
	 * Give ourselves a stack whose memory will be
	 * marked as Normal-IS-WBWA when the MMU is
	 * enabled.
	 * --------------------------------------------
	 */
	bl	plat_set_my_stack

	/* --------------------------------------------
	 * Enable MMU and D-caches together.
	 * --------------------------------------------
	 */
	mov	x0, #0
	bl	bl32_plat_enable_mmu

#if ENABLE_PAUTH
	/* ---------------------------------------------
	 * Program APIAKey_EL1
	 * and enable pointer authentication
	 * ---------------------------------------------
	 */
	bl	pauth_init_enable_el1
#endif /* ENABLE_PAUTH */

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
endfunc tsp_cpu_on_entry

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
endfunc tsp_cpu_suspend_entry

	/*-------------------------------------------------
	 * This entrypoint is used by the TSPD to pass
	 * control for `synchronously` handling a S-EL1
	 * Interrupt which was triggered while executing
	 * in normal world. 'x0' contains a magic number
	 * which indicates this. TSPD expects control to
	 * be handed back at the end of interrupt
	 * processing. This is done through an SMC.
	 * The handover agreement is:
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
	 *    'x0' = TSP_HANDLED_S_EL1_INTR
	 * ------------------------------------------------
	 */
func	tsp_sel1_intr_entry
#if DEBUG
	mov_imm	x2, TSP_HANDLE_SEL1_INTR_AND_RETURN
	cmp	x0, x2
	b.ne	tsp_sel1_int_entry_panic
#endif
	/*-------------------------------------------------
	 * Save any previous context needed to perform
	 * an exception return from S-EL1 e.g. context
	 * from a previous Non secure Interrupt.
	 * Update statistics and handle the S-EL1
	 * interrupt before returning to the TSPD.
	 * IRQ/FIQs are not enabled since that will
	 * complicate the implementation. Execution
	 * will be transferred back to the normal world
	 * in any case. The handler can return 0
	 * if the interrupt was handled or TSP_PREEMPTED
	 * if the expected interrupt was preempted
	 * by an interrupt that should be handled in EL3
	 * e.g. Group 0 interrupt in GICv3. In both
	 * the cases switch to EL3 using SMC with id
	 * TSP_HANDLED_S_EL1_INTR. Any other return value
	 * from the handler will result in panic.
	 * ------------------------------------------------
	 */
	save_eret_context x2 x3
	bl	tsp_update_sync_sel1_intr_stats
	bl	tsp_common_int_handler
	/* Check if the S-EL1 interrupt has been handled */
	cbnz	x0, tsp_sel1_intr_check_preemption
	b	tsp_sel1_intr_return
tsp_sel1_intr_check_preemption:
	/* Check if the S-EL1 interrupt has been preempted */
	mov_imm	x1, TSP_PREEMPTED
	cmp	x0, x1
	b.ne	tsp_sel1_int_entry_panic
tsp_sel1_intr_return:
	mov_imm	x0, TSP_HANDLED_S_EL1_INTR
	restore_eret_context x2 x3
	smc	#0

	/* Should never reach here */
tsp_sel1_int_entry_panic:
	no_ret	plat_panic_handler
endfunc tsp_sel1_intr_entry

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

	/* Should never reach here */
	no_ret	plat_panic_handler
endfunc tsp_cpu_resume_entry

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD to ask
	 * the TSP to service a fast smc request.
	 * ---------------------------------------------
	 */
func tsp_fast_smc_entry
	bl	tsp_smc_handler
	restore_args_call_smc

	/* Should never reach here */
	no_ret	plat_panic_handler
endfunc tsp_fast_smc_entry

	/*---------------------------------------------
	 * This entrypoint is used by the TSPD to ask
	 * the TSP to service a Yielding SMC request.
	 * We will enable preemption during execution
	 * of tsp_smc_handler.
	 * ---------------------------------------------
	 */
func tsp_yield_smc_entry
	msr	daifclr, #DAIF_FIQ_BIT | DAIF_IRQ_BIT
	bl	tsp_smc_handler
	msr	daifset, #DAIF_FIQ_BIT | DAIF_IRQ_BIT
	restore_args_call_smc

	/* Should never reach here */
	no_ret	plat_panic_handler
endfunc tsp_yield_smc_entry

	/*---------------------------------------------------------------------
	 * This entrypoint is used by the TSPD to abort a pre-empted Yielding
	 * SMC. It could be on behalf of non-secure world or because a CPU
	 * suspend/CPU off request needs to abort the preempted SMC.
	 * --------------------------------------------------------------------
	 */
func tsp_abort_yield_smc_entry

	/*
	 * Exceptions masking is already done by the TSPD when entering this
	 * hook so there is no need to do it here.
	 */

	/* Reset the stack used by the pre-empted SMC */
	bl	plat_set_my_stack

	/*
	 * Allow some cleanup such as releasing locks.
	 */
	bl	tsp_abort_smc_handler

	restore_args_call_smc

	/* Should never reach here */
	bl	plat_panic_handler
endfunc tsp_abort_yield_smc_entry
