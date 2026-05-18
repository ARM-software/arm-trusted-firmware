/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <platform_def.h>

/* Relocatable code section */

	.globl  lfa_r_holding_lock
	.globl  lfa_r_holding_wait
	.globl  lfa_r_holding_unlock
	.globl  lfa_r_holding_wait_warm_reset

/*
 * The 'rfunc' macro is a modified version of the 'func' macro in
 * "include/common/asm_macros_common.S". It differs in that it does not change
 * the section to ".text.asm". See the original version for comments.
 */
	.macro rfunc _name, _align=2
	.cfi_sections .debug_frame
	.section .lfa_relocatable_code.\_name, "ax"
	.type \_name, %function
	.cfi_startproc
	.align \_align
	\_name:
#if ENABLE_BTI
	 bti	jc
#endif
	.endm

/*
 * Enable holding lock.
 *
 * void lfa_r_holding_lock(spinlock_t *lock);
 */
rfunc lfa_r_holding_lock
	mov	w1, #1
	stlr	w1, [x0]
	ret
endfunc lfa_r_holding_lock

/*
 * Release hold.
 *
 * Use store-release to unconditionally clear the spinlock variable.
 * Store operation generates an event to all cores waiting in WFE
 * when address is monitored by the global monitor.
 *
 * void lfa_r_holding_unlock(spinlock_t *lock);
 */
rfunc lfa_r_holding_unlock
	mov	w1, #0
	stlr	w1, [x0]
	ret
endfunc lfa_r_holding_unlock

/*
 * Check lock using load-exclusive instruction.
 *
 * void lfa_r_holding_wait(spinlock_t *lock);
 */
rfunc lfa_r_holding_wait
	sevl
l1:	wfe
	ldaxr	w1, [x0]
	cbnz	w1, l1
	ret
endfunc lfa_r_holding_wait

/*
 * Check lock using load-exclusive instruction.
 * Issue RMR_EL3 for warm reset if lock is unlocked
 *
 * void lfa_r_holding_wait(spinlock_t *lock);
 */
rfunc lfa_r_holding_wait_warm_reset
	sevl
l2:
	wfe
	ldaxr	w1, [x0]
	cbnz	w1, l2
l3:
	wfi
	b l3
endfunc lfa_r_holding_wait_warm_reset

/* Relocatable data section */

	.globl  lfa_r_holding_lock_var
	.globl  lfa_r_ep_addresses
	.globl  lfa_r_context_ids

.align 8
.section .lfa_relocatable_data.lfa_r_holding_lock_var, "aw"
lfa_r_holding_lock_var: .quad 0x0

.align 8
.section .lfa_relocatable_data.lfa_r_ep_addresses, "aw"
lfa_r_ep_addresses:
	.fill	PLATFORM_CORE_COUNT, 8, 0x0

.align 8
.section .lfa_relocatable_data.lfa_r_context_ids, "aw"
lfa_r_context_ids:
	.fill	PLATFORM_CORE_COUNT, 8, 0x0
