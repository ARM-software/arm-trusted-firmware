/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
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

	.globl	handle_lower_el_ea_esb
	.globl	enter_lower_el_sync_ea
	.globl	enter_lower_el_async_ea


/*
 * Function to delegate External Aborts synchronized by ESB instruction at EL3
 * vector entry. This function assumes GP registers x0-x29 have been saved, and
 * are available for use. It delegates the handling of the EA to platform
 * handler, and returns only upon successfully handling the EA; otherwise
 * panics. On return from this function, the original exception handler is
 * expected to resume.
 */
func handle_lower_el_ea_esb
	mov	x0, #ERROR_EA_ESB
	mrs	x1, DISR_EL1
	b	ea_proceed
endfunc handle_lower_el_ea_esb


/*
 * This function forms the tail end of Synchronous Exception entry from lower
 * EL, and expects to handle Synchronous External Aborts from lower EL and CPU
 * Implementation Defined Exceptions. If any other kind of exception is detected,
 * then this function reports unhandled exception.
 *
 * Since it's part of exception vector, this function doesn't expect any GP
 * registers to have been saved. It delegates the handling of the EA to platform
 * handler, and upon successfully handling the EA, exits EL3; otherwise panics.
 */
func enter_lower_el_sync_ea
	/*
	 * Explicitly save x30 so as to free up a register and to enable
	 * branching.
	 */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH

	/* Check for I/D aborts from lower EL */
	cmp	x30, #EC_IABORT_LOWER_EL
	b.eq	1f

	cmp	x30, #EC_DABORT_LOWER_EL
	b.eq	1f

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
	cbz	x0, 2f
	mrs	x1, esr_el3
	ubfx	x1, x1, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	blr	x0
	b	2f

1:
	/* Test for EA bit in the instruction syndrome */
	mrs	x30, esr_el3
	tbz	x30, #ESR_ISS_EABORT_EA_BIT, 3f

	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * If Secure Cycle Counter is not disabled in MDCR_EL3 when
	 * ARMv8.5-PMU is implemented, save PMCR_EL0 and disable Cycle Counter.
	 */
	bl	save_gp_pmcr_pauth_regs

#if ENABLE_PAUTH
	/* Load and program APIAKey firmware key */
	bl	pauth_load_bl31_apiakey
#endif

	/* Setup exception class and syndrome arguments for platform handler */
	mov	x0, #ERROR_EA_SYNC
	mrs	x1, esr_el3
	adr	x30, el3_exit
	b	delegate_sync_ea

2:
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]

3:
	/* Synchronous exceptions other than the above are assumed to be EA */
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	no_ret	report_unhandled_exception
endfunc enter_lower_el_sync_ea


/*
 * This function handles SErrors from lower ELs.
 *
 * Since it's part of exception vector, this function doesn't expect any GP
 * registers to have been saved. It delegates the handling of the EA to platform
 * handler, and upon successfully handling the EA, exits EL3; otherwise panics.
 */
func enter_lower_el_async_ea
	/*
	 * Explicitly save x30 so as to free up a register and to enable
	 * branching
	 */
	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	/*
	 * Save general purpose and ARMv8.3-PAuth registers (if enabled).
	 * If Secure Cycle Counter is not disabled in MDCR_EL3 when
	 * ARMv8.5-PMU is implemented, save PMCR_EL0 and disable Cycle Counter.
	 */
	bl	save_gp_pmcr_pauth_regs

#if ENABLE_PAUTH
	/* Load and program APIAKey firmware key */
	bl	pauth_load_bl31_apiakey
#endif

	/* Setup exception class and syndrome arguments for platform handler */
	mov	x0, #ERROR_EA_ASYNC
	mrs	x1, esr_el3
	adr	x30, el3_exit
	b	delegate_async_ea
endfunc enter_lower_el_async_ea


/*
 * Prelude for Synchronous External Abort handling. This function assumes that
 * all GP registers have been saved by the caller.
 *
 * x0: EA reason
 * x1: EA syndrome
 */
func delegate_sync_ea
#if RAS_EXTENSION
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
#if RAS_EXTENSION
	/*
	 * Check for Implementation Defined Syndrome. If so, skip checking
	 * Uncontainable error type from the syndrome as the format is unknown.
	 */
	tbnz	x1, #SERROR_IDS_BIT, 1f

	/*
	 * Check for Uncontainable error type. If so, route to the platform
	 * fatal error handler rather than the generic EA one.
	 */
	ubfx	x2, x1, #EABORT_AET_SHIFT, #EABORT_AET_WIDTH
	cmp	x2, #ERROR_STATUS_UET_UC
	b.ne	1f

	/* Check DFSC for SError type */
	ubfx	x3, x1, #EABORT_DFSC_SHIFT, #EABORT_DFSC_WIDTH
	cmp	x3, #DFSC_SERROR
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
