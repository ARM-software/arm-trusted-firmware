/*
 * Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <bl31/ea_handle.h>
#include <bl31/interrupt_mgmt.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <el3_common_macros.S>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/smccc.h>

	.globl	runtime_exceptions

	.globl	sync_exception_sp_el0
	.globl	irq_sp_el0
	.globl	fiq_sp_el0
	.globl	serror_sp_el0

	.globl	sync_exception_sp_elx
	.globl	irq_sp_elx
	.globl	fiq_sp_elx
	.globl	serror_sp_elx

	.globl	sync_exception_aarch64
	.globl	irq_aarch64
	.globl	fiq_aarch64
	.globl	serror_aarch64

	.globl	sync_exception_aarch32
	.globl	irq_aarch32
	.globl	fiq_aarch32
	.globl	serror_aarch32

	/*
	 * Macro that prepares entry to EL3 upon taking an exception.
	 *
	 * With RAS_EXTENSION, this macro synchronizes pending errors with an ESB
	 * instruction. When an error is thus synchronized, the handling is
	 * delegated to platform EA handler.
	 *
	 * Without RAS_EXTENSION, this macro synchronizes pending errors using
         * a DSB, unmasks Asynchronous External Aborts and saves X30 before
	 * setting the flag CTX_IS_IN_EL3.
	 */
	.macro check_and_unmask_ea
#if RAS_EXTENSION
	/* Synchronize pending External Aborts */
	esb

	/* Unmask the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	/*
	 * Explicitly save x30 so as to free up a register and to enable
	 * branching
	 */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	/* Check for SErrors synchronized by the ESB instruction */
	mrs	x30, DISR_EL1
	tbz	x30, #DISR_A_BIT, 1f

	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * If Secure Cycle Counter is not disabled in MDCR_EL3 when
	 * ARMv8.5-PMU is implemented, save PMCR_EL0 and disable Cycle Counter.
	 * Also set the PSTATE to a known state.
	 */
	bl	prepare_el3_entry

	bl	handle_lower_el_ea_esb

	/* Restore general purpose, PMCR_EL0 and ARMv8.3-PAuth registers */
	bl	restore_gp_pmcr_pauth_regs
1:
#else
	/*
	 * For SoCs which do not implement RAS, use DSB as a barrier to
	 * synchronize pending external aborts.
	 */
	dsb	sy

	/* Unmask the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	/* Use ISB for the above unmask operation to take effect immediately */
	isb

	/*
	 * Refer Note 1. No need to restore X30 as both handle_sync_exception
	 * and handle_interrupt_exception macro which follow this macro modify
	 * X30 anyway.
	 */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	mov 	x30, #1
	str	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_IS_IN_EL3]
	dmb	sy
#endif
	.endm

#if !RAS_EXTENSION
	/*
	 * Note 1: The explicit DSB at the entry of various exception vectors
	 * for handling exceptions from lower ELs can inadvertently trigger an
	 * SError exception in EL3 due to pending asynchronous aborts in lower
	 * ELs. This will end up being handled by serror_sp_elx which will
	 * ultimately panic and die.
	 * The way to workaround is to update a flag to indicate if the exception
	 * truly came from EL3. This flag is allocated in the cpu_context
	 * structure and located at offset "CTX_EL3STATE_OFFSET + CTX_IS_IN_EL3"
	 * This is not a bullet proof solution to the problem at hand because
	 * we assume the instructions following "isb" that help to update the
	 * flag execute without causing further exceptions.
	 */

	/* ---------------------------------------------------------------------
	 * This macro handles Asynchronous External Aborts.
	 * ---------------------------------------------------------------------
	 */
	.macro	handle_async_ea
	/*
	 * Use a barrier to synchronize pending external aborts.
	 */
	dsb	sy

	/* Unmask the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	/* Use ISB for the above unmask operation to take effect immediately */
	isb

	/* Refer Note 1 */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	mov 	x30, #1
	str	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_IS_IN_EL3]
	dmb	sy

	b	handle_lower_el_async_ea
	.endm

	/*
	 * This macro checks if the exception was taken due to SError in EL3 or
	 * because of pending asynchronous external aborts from lower EL that got
	 * triggered due to explicit synchronization in EL3. Refer Note 1.
	 */
	.macro check_if_serror_from_EL3
	/* Assumes SP_EL3 on entry */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	ldr	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_IS_IN_EL3]
	cbnz	x30, exp_from_EL3

	/* Handle asynchronous external abort from lower EL */
	b	handle_lower_el_async_ea

exp_from_EL3:
	/* Jump to plat_handle_el3_ea which does not return */
	.endm
#endif

	/* ---------------------------------------------------------------------
	 * This macro handles Synchronous exceptions.
	 * Only SMC exceptions are supported.
	 * ---------------------------------------------------------------------
	 */
	.macro	handle_sync_exception
#if ENABLE_RUNTIME_INSTRUMENTATION
	/*
	 * Read the timestamp value and store it in per-cpu data. The value
	 * will be extracted from per-cpu data by the C level SMC handler and
	 * saved to the PMF timestamp region.
	 */
	mrs	x30, cntpct_el0
	str	x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
	mrs	x29, tpidr_el3
	str	x30, [x29, #CPU_DATA_PMF_TS0_OFFSET]
	ldr	x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
#endif

	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH

	/* Handle SMC exceptions separately from other synchronous exceptions */
	cmp	x30, #EC_AARCH32_SMC
	b.eq	smc_handler32

	cmp	x30, #EC_AARCH64_SMC
	b.eq	smc_handler64

	/* Synchronous exceptions other than the above are assumed to be EA */
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	b	enter_lower_el_sync_ea
	.endm


	/* ---------------------------------------------------------------------
	 * This macro handles FIQ or IRQ interrupts i.e. EL3, S-EL1 and NS
	 * interrupts.
	 * ---------------------------------------------------------------------
	 */
	.macro	handle_interrupt_exception label

	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * If Secure Cycle Counter is not disabled in MDCR_EL3 when
	 * ARMv8.5-PMU is implemented, save PMCR_EL0 and disable Cycle Counter.
	 * Also set the PSTATE to a known state.
	 */
	bl	prepare_el3_entry

#if ENABLE_PAUTH
	/* Load and program APIAKey firmware key */
	bl	pauth_load_bl31_apiakey
#endif

	/* Save the EL3 system registers needed to return from this exception */
	mrs	x0, spsr_el3
	mrs	x1, elr_el3
	stp	x0, x1, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]

	/* Switch to the runtime stack i.e. SP_EL0 */
	ldr	x2, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]
	mov	x20, sp
	msr	spsel, #MODE_SP_EL0
	mov	sp, x2

	/*
	 * Find out whether this is a valid interrupt type.
	 * If the interrupt controller reports a spurious interrupt then return
	 * to where we came from.
	 */
	bl	plat_ic_get_pending_interrupt_type
	cmp	x0, #INTR_TYPE_INVAL
	b.eq	interrupt_exit_\label

	/*
	 * Get the registered handler for this interrupt type.
	 * A NULL return value could be 'cause of the following conditions:
	 *
	 * a. An interrupt of a type was routed correctly but a handler for its
	 *    type was not registered.
	 *
	 * b. An interrupt of a type was not routed correctly so a handler for
	 *    its type was not registered.
	 *
	 * c. An interrupt of a type was routed correctly to EL3, but was
	 *    deasserted before its pending state could be read. Another
	 *    interrupt of a different type pended at the same time and its
	 *    type was reported as pending instead. However, a handler for this
	 *    type was not registered.
	 *
	 * a. and b. can only happen due to a programming error. The
	 * occurrence of c. could be beyond the control of Trusted Firmware.
	 * It makes sense to return from this exception instead of reporting an
	 * error.
	 */
	bl	get_interrupt_type_handler
	cbz	x0, interrupt_exit_\label
	mov	x21, x0

	mov	x0, #INTR_ID_UNAVAILABLE

	/* Set the current security state in the 'flags' parameter */
	mrs	x2, scr_el3
	ubfx	x1, x2, #0, #1

	/* Restore the reference to the 'handle' i.e. SP_EL3 */
	mov	x2, x20

	/* x3 will point to a cookie (not used now) */
	mov	x3, xzr

	/* Call the interrupt type handler */
	blr	x21

interrupt_exit_\label:
	/* Return from exception, possibly in a different security state */
	b	el3_exit

	.endm


vector_base runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry sync_exception_sp_el0
#ifdef MONITOR_TRAPS
	stp x29, x30, [sp, #-16]!

	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH

	/* Check for BRK */
	cmp	x30, #EC_BRK
	b.eq	brk_handler

	ldp x29, x30, [sp], #16
#endif /* MONITOR_TRAPS */

	/* We don't expect any synchronous exceptions from EL3 */
	b	report_unhandled_exception
end_vector_entry sync_exception_sp_el0

vector_entry irq_sp_el0
	/*
	 * EL3 code is non-reentrant. Any asynchronous exception is a serious
	 * error. Loop infinitely.
	 */
	b	report_unhandled_interrupt
end_vector_entry irq_sp_el0


vector_entry fiq_sp_el0
	b	report_unhandled_interrupt
end_vector_entry fiq_sp_el0


vector_entry serror_sp_el0
	no_ret	plat_handle_el3_ea
end_vector_entry serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry sync_exception_sp_elx
	/*
	 * This exception will trigger if anything went wrong during a previous
	 * exception entry or exit or while handling an earlier unexpected
	 * synchronous exception. There is a high probability that SP_EL3 is
	 * corrupted.
	 */
	b	report_unhandled_exception
end_vector_entry sync_exception_sp_elx

vector_entry irq_sp_elx
	b	report_unhandled_interrupt
end_vector_entry irq_sp_elx

vector_entry fiq_sp_elx
	b	report_unhandled_interrupt
end_vector_entry fiq_sp_elx

vector_entry serror_sp_elx
#if !RAS_EXTENSION
	check_if_serror_from_EL3
#endif
	no_ret	plat_handle_el3_ea
end_vector_entry serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry sync_exception_aarch64
	/*
	 * This exception vector will be the entry point for SMCs and traps
	 * that are unhandled at lower ELs most commonly. SP_EL3 should point
	 * to a valid cpu context where the general purpose and system register
	 * state can be saved.
	 */
	apply_at_speculative_wa
	check_and_unmask_ea
	handle_sync_exception
end_vector_entry sync_exception_aarch64

vector_entry irq_aarch64
	apply_at_speculative_wa
	check_and_unmask_ea
	handle_interrupt_exception irq_aarch64
end_vector_entry irq_aarch64

vector_entry fiq_aarch64
	apply_at_speculative_wa
	check_and_unmask_ea
	handle_interrupt_exception fiq_aarch64
end_vector_entry fiq_aarch64

vector_entry serror_aarch64
	apply_at_speculative_wa
#if RAS_EXTENSION
	msr	daifclr, #DAIF_ABT_BIT
	b	enter_lower_el_async_ea
#else
	handle_async_ea
#endif
end_vector_entry serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry sync_exception_aarch32
	/*
	 * This exception vector will be the entry point for SMCs and traps
	 * that are unhandled at lower ELs most commonly. SP_EL3 should point
	 * to a valid cpu context where the general purpose and system register
	 * state can be saved.
	 */
	apply_at_speculative_wa
	check_and_unmask_ea
	handle_sync_exception
end_vector_entry sync_exception_aarch32

vector_entry irq_aarch32
	apply_at_speculative_wa
	check_and_unmask_ea
	handle_interrupt_exception irq_aarch32
end_vector_entry irq_aarch32

vector_entry fiq_aarch32
	apply_at_speculative_wa
	check_and_unmask_ea
	handle_interrupt_exception fiq_aarch32
end_vector_entry fiq_aarch32

vector_entry serror_aarch32
	apply_at_speculative_wa
#if RAS_EXTENSION
	msr	daifclr, #DAIF_ABT_BIT
	b	enter_lower_el_async_ea
#else
	handle_async_ea
#endif
end_vector_entry serror_aarch32

#ifdef MONITOR_TRAPS
	.section .rodata.brk_string, "aS"
brk_location:
	.asciz "Error at instruction 0x"
brk_message:
	.asciz "Unexpected BRK instruction with value 0x"
#endif /* MONITOR_TRAPS */

	/* ---------------------------------------------------------------------
	 * The following code handles secure monitor calls.
	 * Depending upon the execution state from where the SMC has been
	 * invoked, it frees some general purpose registers to perform the
	 * remaining tasks. They involve finding the runtime service handler
	 * that is the target of the SMC & switching to runtime stacks (SP_EL0)
	 * before calling the handler.
	 *
	 * Note that x30 has been explicitly saved and can be used here
	 * ---------------------------------------------------------------------
	 */
func smc_handler
smc_handler32:
	/* Check whether aarch32 issued an SMC64 */
	tbnz	x0, #FUNCID_CC_SHIFT, smc_prohibited

smc_handler64:
	/* NOTE: The code below must preserve x0-x4 */

	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * If Secure Cycle Counter is not disabled in MDCR_EL3 when
	 * ARMv8.5-PMU is implemented, save PMCR_EL0 and disable Cycle Counter.
	 * Also set the PSTATE to a known state.
	 */
	bl	prepare_el3_entry

#if ENABLE_PAUTH
	/* Load and program APIAKey firmware key */
	bl	pauth_load_bl31_apiakey
#endif

	/*
	 * Populate the parameters for the SMC handler.
	 * We already have x0-x4 in place. x5 will point to a cookie (not used
	 * now). x6 will point to the context structure (SP_EL3) and x7 will
	 * contain flags we need to pass to the handler.
	 */
	mov	x5, xzr
	mov	x6, sp

	/*
	 * Restore the saved C runtime stack value which will become the new
	 * SP_EL0 i.e. EL3 runtime stack. It was saved in the 'cpu_context'
	 * structure prior to the last ERET from EL3.
	 */
	ldr	x12, [x6, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]

	/* Switch to SP_EL0 */
	msr	spsel, #MODE_SP_EL0

	/*
	 * Save the SPSR_EL3, ELR_EL3, & SCR_EL3 in case there is a world
	 * switch during SMC handling.
	 * TODO: Revisit if all system registers can be saved later.
	 */
	mrs	x16, spsr_el3
	mrs	x17, elr_el3
	mrs	x18, scr_el3
	stp	x16, x17, [x6, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	str	x18, [x6, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]

	/* Clear flag register */
	mov	x7, xzr

#if ENABLE_RME
	/* Copy SCR_EL3.NSE bit to the flag to indicate caller's security */
	ubfx	x7, x18, #SCR_NSE_SHIFT, 1

	/*
	 * Shift copied SCR_EL3.NSE bit by 5 to create space for
	 * SCR_EL3.NS bit. Bit 5 of the flag correspondes to
	 * the SCR_EL3.NSE bit.
	 */
	lsl	x7, x7, #5
#endif /* ENABLE_RME */

	/* Copy SCR_EL3.NS bit to the flag to indicate caller's security */
	bfi	x7, x18, #0, #1

	mov	sp, x12

	/* Get the unique owning entity number */
	ubfx	x16, x0, #FUNCID_OEN_SHIFT, #FUNCID_OEN_WIDTH
	ubfx	x15, x0, #FUNCID_TYPE_SHIFT, #FUNCID_TYPE_WIDTH
	orr	x16, x16, x15, lsl #FUNCID_OEN_WIDTH

	/* Load descriptor index from array of indices */
	adrp	x14, rt_svc_descs_indices
	add	x14, x14, :lo12:rt_svc_descs_indices
	ldrb	w15, [x14, x16]

	/* Any index greater than 127 is invalid. Check bit 7. */
	tbnz	w15, 7, smc_unknown

	/*
	 * Get the descriptor using the index
	 * x11 = (base + off), w15 = index
	 *
	 * handler = (base + off) + (index << log2(size))
	 */
	adr	x11, (__RT_SVC_DESCS_START__ + RT_SVC_DESC_HANDLE)
	lsl	w10, w15, #RT_SVC_SIZE_LOG2
	ldr	x15, [x11, w10, uxtw]

	/*
	 * Call the Secure Monitor Call handler and then drop directly into
	 * el3_exit() which will program any remaining architectural state
	 * prior to issuing the ERET to the desired lower EL.
	 */
#if DEBUG
	cbz	x15, rt_svc_fw_critical_error
#endif
	blr	x15

	b	el3_exit

smc_unknown:
	/*
	 * Unknown SMC call. Populate return value with SMC_UNK and call
	 * el3_exit() which will restore the remaining architectural state
	 * i.e., SYS, GP and PAuth registers(if any) prior to issuing the ERET
         * to the desired lower EL.
	 */
	mov	x0, #SMC_UNK
	str	x0, [x6, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	b	el3_exit

smc_prohibited:
	restore_ptw_el1_sys_regs
	ldp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	mov	x0, #SMC_UNK
	exception_return

#if DEBUG
rt_svc_fw_critical_error:
	/* Switch to SP_ELx */
	msr	spsel, #MODE_SP_ELX
	no_ret	report_unhandled_exception
#endif
endfunc smc_handler

	/* ---------------------------------------------------------------------
	 * The following code handles exceptions caused by BRK instructions.
	 * Following a BRK instruction, the only real valid cause of action is
	 * to print some information and panic, as the code that caused it is
	 * likely in an inconsistent internal state.
	 *
	 * This is initially intended to be used in conjunction with
	 * __builtin_trap.
	 * ---------------------------------------------------------------------
	 */
#ifdef MONITOR_TRAPS
func brk_handler
	/* Extract the ISS */
	mrs	x10, esr_el3
	ubfx	x10, x10, #ESR_ISS_SHIFT, #ESR_ISS_LENGTH

	/* Ensure the console is initialized */
	bl	plat_crash_console_init

	adr	x4, brk_location
	bl	asm_print_str
	mrs	x4, elr_el3
	bl	asm_print_hex
	bl	asm_print_newline

	adr	x4, brk_message
	bl	asm_print_str
	mov	x4, x10
	mov	x5, #28
	bl	asm_print_hex_bits
	bl	asm_print_newline

	no_ret	plat_panic_handler
endfunc brk_handler
#endif /* MONITOR_TRAPS */
