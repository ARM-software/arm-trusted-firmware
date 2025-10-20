/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <asm_macros.S>
#include <cpu_macros.S>
#include <context.h>

	.globl	handle_pending_async_ea

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
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	str	xzr, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	exception_return
endfunc handle_pending_async_ea
