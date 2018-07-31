/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <console.h>

	.globl	console_register
	.globl	console_unregister
	.globl	console_is_registered
	.globl	console_set_scope
	.globl	console_switch_state
	.globl	console_putc
	.globl	console_getc
	.globl	console_flush

	/*
	 *  The console list pointer is in the data section and not in
	 *  .bss even though it is zero-init. In particular, this allows
	 *  the console functions to start using this variable before
	 *  the runtime memory is initialized for images which do not
	 *  need to copy the .data section from ROM to RAM.
	 */
.section .data.console_list ; .align 3
	console_list: .quad 0x0
.section .data.console_state ; .align 0
	console_state: .byte CONSOLE_FLAG_BOOT

	/* -----------------------------------------------
	 * int console_register(console_t *console)
	 * Function to insert a new console structure into
	 * the console list. Should usually be called by
	 * console_<driver>_register implementations. The
	 * data structure passed will be taken over by the
	 * console framework and *MUST* be allocated in
	 * persistent memory (e.g. the data section).
	 * In : x0 - address of console_t structure
	 * Out: x0 - Always 1 (for easier tail calling)
	 * Clobber list: x0, x1
	 * -----------------------------------------------
	 */
func console_register
	stp	x21, x30, [sp, #-16]!
#if ENABLE_ASSERTIONS
	/* Assert that x0 isn't a NULL pointer */
	cmp	x0, #0
	ASM_ASSERT(ne)
	/* Assert that the struct isn't in the stack */
	adrp	x1, __STACKS_START__
	add	x1, x1, :lo12:__STACKS_START__
	cmp	x0, x1
	b.lo	not_on_stack
	adrp	x1, __STACKS_END__
	add	x1, x1, :lo12:__STACKS_END__
	cmp	x0, x1
	ASM_ASSERT(hs)
not_on_stack:
	/* Assert that this struct isn't in the list */
	mov	x1, x0 /* Preserve x0 and x30 */
	bl	console_is_registered
	cmp	x0, #0
	ASM_ASSERT(eq)
	mov	x0, x1
#endif /* ENABLE_ASSERTIONS */
	adrp	x21, console_list
	ldr	x1, [x21, :lo12:console_list]	/* X1 = first struct in list */
	str	x0, [x21, :lo12:console_list]	/* list head = new console */
	str	x1, [x0, #CONSOLE_T_NEXT]	/* new console next ptr = X1 */
	mov	x0, #1
	ldp	x21, x30, [sp], #16
	ret
endfunc console_register

	/* -----------------------------------------------
	 * int console_unregister(console_t *console)
	 * Function to find a specific console in the list
	 * of currently active consoles and remove it.
	 * In: x0 - address of console_t struct to remove
	 * Out: x0 - removed address, or NULL if not found
	 * Clobber list: x0, x1
	 * -----------------------------------------------
	 */
func console_unregister
#if ENABLE_ASSERTIONS
	/* Assert that x0 isn't a NULL pointer */
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	stp	x21, xzr, [sp, #-16]!
	adrp	x21, console_list
	add	x21, x21, :lo12:console_list	/* X21 = ptr to first struct */
	ldr	x1, [x21]			/* X1 = first struct */

unregister_loop:
	cbz	x1, unregister_not_found
	cmp	x0, x1
	b.eq	unregister_found
	ldr	x21, [x21]			/* X21 = next ptr of struct */
	ldr	x1, [x21]			/* X1 = next struct */
	b	unregister_loop

unregister_found:
	ldr	x1, [x1]			/* X1 = next struct */
	str	x1, [x21]			/* prev->next = cur->next */
	ldp	x21, xzr, [sp], #16
	ret

unregister_not_found:
	mov	x0, #0				/* return NULL if not found */
	ldp	x21, xzr, [sp], #16
	ret
endfunc console_unregister

	/* -----------------------------------------------
	 * int console_is_registered(console_t *console)
	 * Function to detect if a specific console is
	 * registered or not.
	 * In: x0 - address of console_t struct to remove
	 * Out: x0 - 1 if it is registered, 0 if not.
	 * Clobber list: x0
	 * -----------------------------------------------
	 */
func console_is_registered
#if ENABLE_ASSERTIONS
	/* Assert that x0 isn't a NULL pointer */
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	stp	x21, xzr, [sp, #-16]!
	adrp	x21, console_list
	ldr	x21, [x21, :lo12:console_list]	/* X21 = first console struct */
check_registered_loop:
	cbz	x21, console_not_registered /* Check if end of list */
	cmp	x0, x21		/* Check if the pointers are different */
	b.eq	console_registered
	ldr	x21, [x21, #CONSOLE_T_NEXT]	/* Get pointer to next struct */
	b	check_registered_loop
console_not_registered:
	mov	x0, #0
	ldp	x21, xzr, [sp], #16
	ret
console_registered:
	mov	x0, #1
	ldp	x21, xzr, [sp], #16
	ret
endfunc console_is_registered

	/* -----------------------------------------------
	 * void console_switch_state(unsigned int new_state)
	 * Function to switch the current console state.
	 * The console state determines which of the
	 * registered consoles are actually used at a time.
	 * In : w0 - global console state to move to
	 * Clobber list: x0, x1
	 * -----------------------------------------------
	 */
func console_switch_state
	adrp	x1, console_state
	strb	w0, [x1, :lo12:console_state]
	ret
endfunc console_switch_state

	/* -----------------------------------------------
	 * void console_set_scope(console_t *console,
	 *                       unsigned int scope)
	 * Function to update the states that a given console
	 * may be active in.
	 * In : x0 - pointer to console_t struct
	 *    : w1 - new active state mask
	 * Clobber list: x0, x1, x2
	 * -----------------------------------------------
	 */
func console_set_scope
#if ENABLE_ASSERTIONS
	tst	w1, #~CONSOLE_FLAG_SCOPE_MASK
	ASM_ASSERT(eq)
#endif /* ENABLE_ASSERTIONS */
	ldr	w2, [x0, #CONSOLE_T_FLAGS]
	and	w2, w2, #~CONSOLE_FLAG_SCOPE_MASK
	orr	w2, w2, w1
	str	w2, [x0, #CONSOLE_T_FLAGS]
	ret
endfunc console_set_scope

	/* ---------------------------------------------
	 * int console_putc(int c)
	 * Function to output a character. Calls all
	 * active console's putc() handlers in succession.
	 * In : x0 - character to be printed
	 * Out: x0 - printed character on success, or < 0
	             if at least one console had an error
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func console_putc
	stp	x21, x30, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	mov	w20, #ERROR_NO_VALID_CONSOLE	/* W20 = current return value */
	mov	w19, w0				/* W19 = character to print */
	adrp	x21, console_list
	ldr	x21, [x21, :lo12:console_list]	/* X21 = first console struct */

putc_loop:
	cbz	x21, putc_done
	adrp	x1, console_state
	ldrb	w1, [x1, :lo12:console_state]
	ldr	w2, [x21, #CONSOLE_T_FLAGS]
	tst	w1, w2
	b.eq	putc_continue
	ldr	x2, [x21, #CONSOLE_T_PUTC]
	cbz	x2, putc_continue
	mov	w0, w19
	mov	x1, x21
	blr	x2
	cmp	w20, #ERROR_NO_VALID_CONSOLE	/* update W20 if it's NOVALID */
	ccmp	w0, #0, #0x8, ne		/* else update it if W0 < 0 */
	csel	w20, w0, w20, lt
putc_continue:
	ldr	x21, [x21]			/* X21 = next struct */
	b	putc_loop

putc_done:
	mov	w0, w20
	ldp	x19, x20, [sp], #16
	ldp	x21, x30, [sp], #16
	ret
endfunc console_putc

	/* ---------------------------------------------
	 * int console_getc(void)
	 * Function to get a character from any console.
	 * Keeps looping through all consoles' getc()
	 * handlers until one of them returns a
	 * character, then stops iterating and returns
	 * that character to the caller. Will stop looping
	 * if all active consoles report real errors
	 * (other than just not having a char available).
	 * Out : x0 - read character, or < 0 on error
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_getc
	stp	x30, xzr, [sp, #-16]!
	stp	x20, x21, [sp, #-16]!
getc_try_again:
	mov	w20, #ERROR_NO_VALID_CONSOLE	/* W20 = current return value */
	adrp	x21, console_list
	ldr	x21, [x21, :lo12:console_list]	/* X21 = first console struct */
	cbnz	x21, getc_loop
	mov	w0, w20				/* If no consoles registered */
	ldp	x20, x21, [sp], #16
	ldp	x30, xzr, [sp], #16
	ret					/* return immediately. */

getc_loop:
	adrp	x0, console_state
	ldrb	w0, [x0, :lo12:console_state]
	ldr	w1, [x21, #CONSOLE_T_FLAGS]
	tst	w0, w1
	b.eq	getc_continue
	ldr	x1, [x21, #CONSOLE_T_GETC]
	cbz	x1, getc_continue
	mov	x0, x21
	blr	x1
	cmp	w0, #0				/* if X0 >= 0: return */
	b.ge	getc_found
	cmp	w20, #ERROR_NO_PENDING_CHAR	/* may update W20 (NOCHAR has */
	csel	w20, w20, w0, eq		/* precedence vs real errors) */
getc_continue:
	ldr	x21, [x21]			/* X21 = next struct */
	cbnz	x21, getc_loop
	cmp	w20, #ERROR_NO_PENDING_CHAR	/* Keep scanning if at least */
	b.eq	getc_try_again			/* one console returns NOCHAR */
	mov	w0, w20

getc_found:
	ldp	x20, x21, [sp], #16
	ldp	x30, xzr, [sp], #16
	ret
endfunc console_getc

	/* ---------------------------------------------
	 * int console_flush(void)
	 * Function to force a write of all buffered
	 * data that hasn't been output. Calls all
	 * console's flush() handlers in succession.
	 * Out: x0 - 0 on success, < 0 if at least one error
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func console_flush
	stp	x30, xzr, [sp, #-16]!
	stp	x20, x21, [sp, #-16]!
	mov	w20, #ERROR_NO_VALID_CONSOLE	/* W20 = current return value */
	adrp	x21, console_list
	ldr	x21, [x21, :lo12:console_list]	/* X21 = first console struct */

flush_loop:
	cbz	x21, flush_done
	adrp	x1, console_state
	ldrb	w1, [x1, :lo12:console_state]
	ldr	w2, [x21, #CONSOLE_T_FLAGS]
	tst	w1, w2
	b.eq	flush_continue
	ldr	x1, [x21, #CONSOLE_T_FLUSH]
	cbz	x1, flush_continue
	mov	x0, x21
	blr	x1
	cmp	w20, #ERROR_NO_VALID_CONSOLE	/* update W20 if it's NOVALID */
	ccmp	w0, #0, #0x8, ne		/* else update it if W0 < 0 */
	csel	w20, w0, w20, lt
flush_continue:
	ldr	x21, [x21]			/* X21 = next struct */
	b	flush_loop

flush_done:
	mov	w0, w20
	ldp	x20, x21, [sp], #16
	ldp	x30, xzr, [sp], #16
	ret
endfunc console_flush
