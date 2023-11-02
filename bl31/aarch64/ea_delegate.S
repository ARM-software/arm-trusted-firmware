/*
 * Copyright (c) 2018-2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <assert_macros.S>
#include <asm_macros.S>
#include <assert_macros.S>
#include <bl31/ea_handle.h>
#include <context.h>
#include <lib/extensions/ras_arch.h>
#include <cpu_macros.S>
#include <context.h>

	.globl	handle_lower_el_sync_ea
	.globl	handle_lower_el_async_ea
	.globl	handle_pending_async_ea
/*
 * This function handles Synchronous External Aborts from lower EL.
 *
 * It delegates the handling of the EA to platform handler, and upon successfully
 * handling the EA, exits EL3; otherwise panics.
 *
 * This function assumes x30 has been saved.
 */
func handle_lower_el_sync_ea
	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH

	/* Check for I/D aborts from lower EL */
	cmp	x30, #EC_IABORT_LOWER_EL
	b.eq	1f

	cmp	x30, #EC_DABORT_LOWER_EL
	b.eq	1f

	/* EA other than above are unhandled exceptions */
	no_ret	report_unhandled_exception
1:
	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * Also save PMCR_EL0 and set the PSTATE to a known state.
	 */
	bl	prepare_el3_entry

#if ENABLE_PAUTH
	/* Load and program APIAKey firmware key */
	bl	pauth_load_bl31_apiakey
#endif

	/* Setup exception class and syndrome arguments for platform handler */
	mov	x0, #ERROR_EA_SYNC
	mrs	x1, esr_el3
	bl	delegate_sync_ea

	/* el3_exit assumes SP_EL0 on entry */
	msr	spsel, #MODE_SP_EL0
	b	el3_exit
endfunc handle_lower_el_sync_ea


/*
 * This function handles SErrors from lower ELs.
 *
 * It delegates the handling of the EA to platform handler, and upon successfully
 * handling the EA, exits EL3; otherwise panics.
 *
 * This function assumes x30 has been saved.
 */
func handle_lower_el_async_ea

	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * Also save PMCR_EL0 and set the PSTATE to a known state.
	 */
	bl	prepare_el3_entry

#if ENABLE_PAUTH
	/* Load and program APIAKey firmware key */
	bl	pauth_load_bl31_apiakey
#endif

	/* Setup exception class and syndrome arguments for platform handler */
	mov	x0, #ERROR_EA_ASYNC
	mrs	x1, esr_el3
	bl	delegate_async_ea

	/* el3_exit assumes SP_EL0 on entry */
	msr	spsel, #MODE_SP_EL0
	b	el3_exit
endfunc handle_lower_el_async_ea

/*
 * Handler for async EA from lower EL synchronized at EL3 entry in FFH mode.
 *
 * This scenario may arise when there is an error (EA) in the system which is not
 * yet signaled to PE while executing in lower EL. During entry into EL3, the errors
 * are synchronized either implicitly or explicitly causing async EA to pend at EL3.
 *
 * On detecting the pending EA (via ISR_EL1.A), if the EA routing model is Firmware
 * First handling (FFH, SCR_EL3.EA = 1) this handler first handles the pending EA
 * and then handles the original exception.
 *
 * This function assumes x30 has been saved.
 */
func handle_pending_async_ea
	/*
	 * Prepare for nested handling of EA. Stash sysregs clobbered by nested
	 * exception and handler
	 */
	str	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_GPREG_LR]
	mrs	x30, esr_el3
	str	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ESR_EL3]
	mrs	x30, spsr_el3
	str	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_SPSR_EL3]
	mrs	x30, elr_el3
	str	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ELR_EL3]

	mov	x30, #1
	str	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_NESTED_EA_FLAG]
	/*
	 * Restore the original x30 saved as part of entering EL3. This is not
	 * required for the current function but for EL3 SError vector entry
	 * once PSTATE.A bit is unmasked. We restore x30 and then the same
	 * value is stored in EL3 SError vector entry.
	 */
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	/*
	 * After clearing PSTATE.A bit pending SError will trigger at current EL.
	 * Put explicit synchronization event to ensure newly unmasked interrupt
	 * is taken immediately.
	 */
	unmask_async_ea

	/* Restore the original exception information along with zeroing the storage */
	ldr	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ELR_EL3]
	msr	elr_el3, x30
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ELR_EL3]
	ldr	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_SPSR_EL3]
	msr	spsr_el3, x30
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_SPSR_EL3]
	ldr	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ESR_EL3]
	msr	esr_el3, x30
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_ESR_EL3]

	/*
	 * If the original exception corresponds to SError from lower El, eret back
	 * to lower EL, otherwise return to vector table for original exception handling.
	 */
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x30, #EC_SERROR
	ldr	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_GPREG_LR]
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_SAVED_GPREG_LR]
	b.eq	1f
	ret
1:
	exception_return
endfunc handle_pending_async_ea

/*
 * Prelude for Synchronous External Abort handling. This function assumes that
 * all GP registers have been saved by the caller.
 *
 * x0: EA reason
 * x1: EA syndrome
 */
func delegate_sync_ea
#if ENABLE_FEAT_RAS
	/*
	 * Check for Uncontainable error type. If so, route to the platform
	 * fatal error handler rather than the generic EA one.
	 */
	ubfx    x2, x1, #EABORT_SET_SHIFT, #EABORT_SET_WIDTH
	cmp     x2, #ERROR_STATUS_SET_UC
	b.ne    1f

	/* Check fault status code */
	ubfx    x3, x1, #EABORT_DFSC_SHIFT, #EABORT_DFSC_WIDTH
	cmp     x3, #SYNC_EA_FSC
	b.ne    1f

	no_ret  plat_handle_uncontainable_ea
1:
#endif

	b       ea_proceed
endfunc delegate_sync_ea


/*
 * Prelude for Asynchronous External Abort handling. This function assumes that
 * all GP registers have been saved by the caller.
 *
 * x0: EA reason
 * x1: EA syndrome
 */
func delegate_async_ea
#if ENABLE_FEAT_RAS
	/* Check Exception Class to ensure SError, as this function should
	 * only be invoked for SError. If that is not the case, which implies
	 * either an HW error or programming error, panic.
	 */
	ubfx	x2, x1, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x2, EC_SERROR
	b.ne	el3_panic
	/*
	 * Check for Implementation Defined Syndrome. If so, skip checking
	 * Uncontainable error type from the syndrome as the format is unknown.
	 */
	tbnz	x1, #SERROR_IDS_BIT, 1f

	/* AET only valid when DFSC is 0x11 */
	ubfx	x2, x1, #EABORT_DFSC_SHIFT, #EABORT_DFSC_WIDTH
	cmp	x2, #DFSC_SERROR
	b.ne	1f

	/*
	 * Check for Uncontainable error type. If so, route to the platform
	 * fatal error handler rather than the generic EA one.
	 */
	ubfx	x3, x1, #EABORT_AET_SHIFT, #EABORT_AET_WIDTH
	cmp	x3, #ERROR_STATUS_UET_UC
	b.ne	1f

	no_ret	plat_handle_uncontainable_ea
1:
#endif

	b	ea_proceed
endfunc delegate_async_ea


/*
 * Delegate External Abort handling to platform's EA handler. This function
 * assumes that all GP registers have been saved by the caller.
 *
 * x0: EA reason
 * x1: EA syndrome
 */
func ea_proceed
	/*
	 * If the ESR loaded earlier is not zero, we were processing an EA
	 * already, and this is a double fault.
	 */
	ldr	x5, [sp, #CTX_EL3STATE_OFFSET + CTX_ESR_EL3]
	cbz	x5, 1f
	no_ret	plat_handle_double_fault

1:
	/* Save EL3 state */
	mrs	x2, spsr_el3
	mrs	x3, elr_el3
	stp	x2, x3, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]

	/*
	 * Save ESR as handling might involve lower ELs, and returning back to
	 * EL3 from there would trample the original ESR.
	 */
	mrs	x4, scr_el3
	mrs	x5, esr_el3
	stp	x4, x5, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]

	/*
	 * Setup rest of arguments, and call platform External Abort handler.
	 *
	 * x0: EA reason (already in place)
	 * x1: Exception syndrome (already in place).
	 * x2: Cookie (unused for now).
	 * x3: Context pointer.
	 * x4: Flags (security state from SCR for now).
	 */
	mov	x2, xzr
	mov	x3, sp
	ubfx	x4, x4, #0, #1

	/* Switch to runtime stack */
	ldr	x5, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]
	msr	spsel, #MODE_SP_EL0
	mov	sp, x5

	mov	x29, x30
#if ENABLE_ASSERTIONS
	/* Stash the stack pointer */
	mov	x28, sp
#endif
	bl	plat_ea_handler

#if ENABLE_ASSERTIONS
	/*
	 * Error handling flows might involve long jumps; so upon returning from
	 * the platform error handler, validate that the we've completely
	 * unwound the stack.
	 */
	mov	x27, sp
	cmp	x28, x27
	ASM_ASSERT(eq)
#endif

	/* Make SP point to context */
	msr	spsel, #MODE_SP_ELX

	/* Restore EL3 state and ESR */
	ldp	x1, x2, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	msr	spsr_el3, x1
	msr	elr_el3, x2

	/* Restore ESR_EL3 and SCR_EL3 */
	ldp	x3, x4, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]
	msr	scr_el3, x3
	msr	esr_el3, x4

#if ENABLE_ASSERTIONS
	cmp	x4, xzr
	ASM_ASSERT(ne)
#endif

	/* Clear ESR storage */
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_ESR_EL3]

	ret	x29
endfunc ea_proceed
