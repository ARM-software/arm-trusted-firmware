/*
 * Copyright (c) 2026, BayLibre SAS
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_HOLD_PEN_S
#define PLAT_HOLD_PEN_S

#include <plat/common/plat_hold_pen.h>

/*
 * plat_hold_pen_wait_and_jump base, idx, tmp
 *
 * Compute the hold pen slot address from a base address and core
 * index, then poll until both magic tags are valid and the entry
 * field contains a real entrypoint (not HOLD_STATE_WAIT), then
 * branch to it. This macro does not return.
 *
 * Before branching, writes HOLD_STATE_WAIT back to prevent stale
 * reuse on warm boot. The dcache is not enabled when this macro is
 * used so no flush is needed.
 *
 *   base: register holding the hold pen base address (clobbered)
 *   idx:  register holding the core index (clobbered)
 *   tmp:  scratch register
 */

#ifdef __aarch64__
	.macro plat_hold_pen_wait_and_jump base, idx, tmp
	mov	\tmp, #HOLD_SLOT_SIZE
	madd	\base, \idx, \tmp, \base
poll_\@:
	ldr	\idx, [\base, #HOLD_SLOT_MAGIC1]
	mov_imm	\tmp, HOLD_BOOT_MAGIC1
	cmp	\idx, \tmp
	b.ne	wait_\@
	ldr	\idx, [\base, #HOLD_SLOT_MAGIC2]
	mov_imm	\tmp, HOLD_BOOT_MAGIC2
	cmp	\idx, \tmp
	b.ne	wait_\@
	ldr	\idx, [\base, #HOLD_SLOT_ENTRY]
	cmp	\idx, #HOLD_STATE_WAIT
	b.eq	wait_\@
	/* Prevent reuse of stale entry on warm boot */
	mov	\tmp, #HOLD_STATE_WAIT
	str	\tmp, [\base, #HOLD_SLOT_ENTRY]
	br	\idx
wait_\@:
	wfe
	b	poll_\@
	.endm
#else /* __aarch64__ */
	.macro plat_hold_pen_wait_and_jump base, idx, tmp
	mov	\tmp, #HOLD_SLOT_SIZE
	mla	\base, \idx, \tmp, \base
poll_\@:
	ldr	\idx, [\base, #HOLD_SLOT_MAGIC1]
	mov_imm	\tmp, HOLD_BOOT_MAGIC1
	cmp	\idx, \tmp
	bne	wait_\@
	ldr	\idx, [\base, #HOLD_SLOT_MAGIC2]
	mov_imm	\tmp, HOLD_BOOT_MAGIC2
	cmp	\idx, \tmp
	bne	wait_\@
	ldr	\idx, [\base, #HOLD_SLOT_ENTRY]
	cmp	\idx, #HOLD_STATE_WAIT
	beq	wait_\@
	/* Prevent reuse of stale entry on warm boot */
	mov	\tmp, #HOLD_STATE_WAIT
	str	\tmp, [\base, #HOLD_SLOT_ENTRY]
	bx	\idx
wait_\@:
	wfe
	b	poll_\@
	.endm
#endif /* __aarch64__ */

#endif /* PLAT_HOLD_PEN_S */
