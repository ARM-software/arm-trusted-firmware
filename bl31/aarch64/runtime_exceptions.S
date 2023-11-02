/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <bl31/ea_handle.h>
#include <bl31/interrupt_mgmt.h>
#include <bl31/sync_handle.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <cpu_macros.S>
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
	 * Save LR and make x30 available as most of the routines in vector entry
	 * need a free register
	 */
	.macro save_x30
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	.endm

	.macro restore_x30
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	.endm

	/*
	 * Macro that synchronizes errors (EA) and checks for pending SError.
	 * On detecting a pending SError it either reflects it back to lower
	 * EL (KFH) or handles it in EL3 (FFH) based on EA routing model.
	 */
	.macro	sync_and_handle_pending_serror
	synchronize_errors
	mrs	x30, ISR_EL1
	tbz	x30, #ISR_A_SHIFT, 2f
#if FFH_SUPPORT
	mrs	x30, scr_el3
	tst	x30, #SCR_EA_BIT
	b.eq	1f
	bl	handle_pending_async_ea
	b	2f
#endif
1:
	/* This function never returns, but need LR for decision making */
	bl	reflect_pending_async_ea_to_lower_el
2:
	.endm

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
	b.eq	sync_handler64

	cmp	x30, #EC_AARCH64_SYS
	b.eq	sync_handler64

	cmp	x30, #EC_IMP_DEF_EL3
	b.eq	imp_def_el3_handler

	/* If FFH Support then try to handle lower EL EA exceptions. */
#if FFH_SUPPORT
	mrs	x30, scr_el3
	tst	x30, #SCR_EA_BIT
	b.eq	1f
	b	handle_lower_el_sync_ea
#endif
1:
	/* Synchronous exceptions other than the above are unhandled */
	b	report_unhandled_exception
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
#if FFH_SUPPORT
	/*
	 * This will trigger if the exception was taken due to SError in EL3 or
	 * because of pending asynchronous external aborts from lower EL that got
	 * triggered due to implicit/explicit synchronization in EL3 (SCR_EL3.EA=1)
	 * during EL3 entry. For the former case we continue with "plat_handle_el3_ea".
	 * The later case will occur when PSTATE.A bit is cleared in
	 * "handle_pending_async_ea". This means we are doing a nested
	 * exception in EL3. Call the handler for async EA which will eret back to
	 * original el3 handler if it is nested exception. Also, unmask EA so that we
	 * catch any further EA arise when handling this nested exception at EL3.
	 */
	save_x30
	ldr	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_NESTED_EA_FLAG]
	cbz	x30, 1f
	/*
	 * This is nested exception handling, clear the flag to avoid taking this
	 * path for further exceptions caused by EA handling
	 */
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_NESTED_EA_FLAG]
	unmask_async_ea
	b	handle_lower_el_async_ea
1:
	restore_x30
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
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	handle_sync_exception
end_vector_entry sync_exception_aarch64

vector_entry irq_aarch64
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	b	handle_interrupt_exception
end_vector_entry irq_aarch64

vector_entry fiq_aarch64
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	b 	handle_interrupt_exception
end_vector_entry fiq_aarch64

	/*
	 * Need to synchronize any outstanding SError since we can get a burst of errors.
	 * So reuse the sync mechanism to catch any further errors which are pending.
	 */
vector_entry serror_aarch64
#if FFH_SUPPORT
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	b	handle_lower_el_async_ea
#else
	b	report_unhandled_exception
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
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	handle_sync_exception
end_vector_entry sync_exception_aarch32

vector_entry irq_aarch32
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	b	handle_interrupt_exception
end_vector_entry irq_aarch32

vector_entry fiq_aarch32
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	b	handle_interrupt_exception
end_vector_entry fiq_aarch32

	/*
	 * Need to synchronize any outstanding SError since we can get a burst of errors.
	 * So reuse the sync mechanism to catch any further errors which are pending.
	 */
vector_entry serror_aarch32
#if FFH_SUPPORT
	save_x30
	apply_at_speculative_wa
	sync_and_handle_pending_serror
	unmask_async_ea
	b	handle_lower_el_async_ea
#else
	b	report_unhandled_exception
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
func sync_exception_handler
smc_handler32:
	/* Check whether aarch32 issued an SMC64 */
	tbnz	x0, #FUNCID_CC_SHIFT, smc_prohibited

sync_handler64:
	/* NOTE: The code below must preserve x0-x4 */

	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * Also save PMCR_EL0 and  set the PSTATE to a known state.
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
	 * Save the SPSR_EL3 and ELR_EL3 in case there is a world
	 * switch during SMC handling.
	 * TODO: Revisit if all system registers can be saved later.
	 */
	mrs	x16, spsr_el3
	mrs	x17, elr_el3
	stp	x16, x17, [x6, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]

	/* Load SCR_EL3 */
	mrs	x18, scr_el3

	/* check for system register traps */
	mrs	x16, esr_el3
	ubfx	x17, x16, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x17, #EC_AARCH64_SYS
	b.eq	sysreg_handler64

	/* Clear flag register */
	mov	x7, xzr

#if ENABLE_RME
	/* Copy SCR_EL3.NSE bit to the flag to indicate caller's security */
	ubfx	x7, x18, #SCR_NSE_SHIFT, #1

	/*
	 * Shift copied SCR_EL3.NSE bit by 5 to create space for
	 * SCR_EL3.NS bit. Bit 5 of the flag corresponds to
	 * the SCR_EL3.NSE bit.
	 */
	lsl	x7, x7, #5
#endif /* ENABLE_RME */

	/* Copy SCR_EL3.NS bit to the flag to indicate caller's security */
	bfi	x7, x18, #0, #1

	mov	sp, x12

	/*
	 * Per SMCCC documentation, bits [23:17] must be zero for Fast
	 * SMCs. Other values are reserved for future use. Ensure that
	 * these bits are zeroes, if not report as unknown SMC.
	 */
	tbz	x0, #FUNCID_TYPE_SHIFT, 2f  /* Skip check if its a Yield Call*/
	tst	x0, #(FUNCID_FC_RESERVED_MASK << FUNCID_FC_RESERVED_SHIFT)
	b.ne	smc_unknown

	/*
	 * Per SMCCCv1.3 a caller can set the SVE hint bit in the SMC FID
	 * passed through x0. Copy the SVE hint bit to flags and mask the
	 * bit in smc_fid passed to the standard service dispatcher.
	 * A service/dispatcher can retrieve the SVE hint bit state from
	 * flags using the appropriate helper.
	 */
2:
	and	x16, x0, #(FUNCID_SVE_HINT_MASK << FUNCID_SVE_HINT_SHIFT)
	orr	x7, x7, x16
	bic	x0, x0, #(FUNCID_SVE_HINT_MASK << FUNCID_SVE_HINT_SHIFT)

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

sysreg_handler64:
	mov	x0, x16		/* ESR_EL3, containing syndrome information */
	mov	x1, x6		/* lower EL's context */
	mov	x19, x6		/* save context pointer for after the call */
	mov	sp, x12		/* EL3 runtime stack, as loaded above */

	/* int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx); */
	bl	handle_sysreg_trap
	/*
	 * returns:
	 *   -1: unhandled trap, panic
	 *    0: handled trap, return to the trapping instruction (repeating it)
	 *    1: handled trap, return to the next instruction
	 */

	tst	w0, w0
	b.mi	elx_panic	/* negative return value: panic */
	b.eq	1f		/* zero: do not change ELR_EL3 */

	/* advance the PC to continue after the instruction */
	ldr	x1, [x19, #CTX_EL3STATE_OFFSET + CTX_ELR_EL3]
	add	x1, x1, #4
	str	x1, [x19, #CTX_EL3STATE_OFFSET + CTX_ELR_EL3]
1:
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
endfunc sync_exception_handler

	/* ---------------------------------------------------------------------
	 * This function handles FIQ or IRQ interrupts i.e. EL3, S-EL1 and NS
	 * interrupts.
	 *
	 * Note that x30 has been explicitly saved and can be used here
	 * ---------------------------------------------------------------------
	 */
func handle_interrupt_exception
	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * Also save PMCR_EL0 and  set the PSTATE to a known state.
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
	b.eq	interrupt_exit

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
	cbz	x0, interrupt_exit
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

interrupt_exit:
	/* Return from exception, possibly in a different security state */
	b	el3_exit
endfunc handle_interrupt_exception

func imp_def_el3_handler
	/* Save GP registers */
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]

	/* Get the cpu_ops pointer */
	bl	get_cpu_ops_ptr

	/* Get the cpu_ops exception handler */
	ldr	x0, [x0, #CPU_E_HANDLER_FUNC]

	/*
	 * If the reserved function pointer is NULL, this CPU does not have an
	 * implementation defined exception handler function
	 */
	cbz	x0, el3_handler_exit
	mrs	x1, esr_el3
	ubfx	x1, x1, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	blr	x0
el3_handler_exit:
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	restore_x30
	no_ret	report_unhandled_exception
endfunc imp_def_el3_handler

/*
 * Handler for async EA from lower EL synchronized at EL3 entry in KFH mode.
 *
 * This scenario may arise when there is an error (EA) in the system which is not
 * yet signaled to PE while executing in lower EL. During entry into EL3, the errors
 * are synchronized either implicitly or explicitly causing async EA to pend at EL3.
 *
 * On detecting the pending EA (via ISR_EL1.A) and if the EA routing model is
 * KFH (SCR_EL3.EA = 1) this handler reflects ther error back to lower EL.
 *
 * This function assumes x30 has been saved.
 */
func reflect_pending_async_ea_to_lower_el
	/*
	 * As the original exception was not handled we need to ensure that we return
	 * back to the instruction which caused the exception. To acheive that, eret
	 * to "elr-4" (Label "subtract_elr_el3") for SMC or simply eret otherwise
	 * (Label "skip_smc_check").
	 *
	 * LIMITATION: It could be that async EA is masked at the target exception level
	 * or the priority of async EA wrt to the EL3/secure interrupt is lower, which
	 * causes back and forth between lower EL and EL3. In case of back and forth between
	 * lower EL and EL3, we can track the loop count in "CTX_NESTED_EA_FLAG" and leverage
	 * previous ELR in "CTX_SAVED_ELR_EL3" to detect this cycle and further panic
	 * to indicate a problem here (Label "check_loop_ctr"). If we are in this cycle, loop
	 * counter retains its value but if we do a normal el3_exit this flag gets cleared.
	 * However, setting SCR_EL3.IESB = 1, should give priority to SError handling
	 * as per AArch64.TakeException pseudo code in Arm ARM.
	 *
	 * TODO: In future if EL3 gets a capability to inject a virtual SError to lower
	 * ELs, we can remove the el3_panic and handle the original exception first and
	 * inject SError to lower EL before ereting back.
	 */
	stp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	ldr	x29, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ELR_EL3]
	mrs	x28, elr_el3
	cmp	x29, x28
	b.eq	check_loop_ctr
	str	x28, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ELR_EL3]
	/* Zero the loop counter */
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_NESTED_EA_FLAG]
	b	skip_loop_ctr
check_loop_ctr:
	ldr	x29, [sp, #CTX_EL3STATE_OFFSET + CTX_NESTED_EA_FLAG]
	add	x29, x29, #1
	str	x29, [sp, #CTX_EL3STATE_OFFSET + CTX_NESTED_EA_FLAG]
	cmp	x29, #ASYNC_EA_REPLAY_COUNTER
	b.ge	el3_panic
skip_loop_ctr:
	/*
	 * Logic to distinguish if we came from SMC or any other exception.
	 * Use offsets in vector entry to get which exception we are handling.
	 * In each vector entry of size 0x200, address "0x0-0x80" is for sync
	 * exception and "0x80-0x200" is for async exceptions.
	 * Use vector base address (vbar_el3) and exception offset (LR) to
	 * calculate whether the address we came from is any of the following
	 * "0x0-0x80", "0x200-0x280", "0x400-0x480" or "0x600-0x680"
	 */
	mrs	x29, vbar_el3
	sub	x30, x30, x29
	and	x30, x30, #0x1ff
	cmp	x30, #0x80
	b.ge	skip_smc_check
	/* Its a synchronous exception, Now check if it is SMC or not? */
	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x30, #EC_AARCH32_SMC
	b.eq	subtract_elr_el3
	cmp	x30, #EC_AARCH64_SMC
	b.eq	subtract_elr_el3
	b	skip_smc_check
subtract_elr_el3:
	sub	x28, x28, #4
skip_smc_check:
	msr	elr_el3, x28
	ldp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	exception_return
endfunc reflect_pending_async_ea_to_lower_el

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
