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
.section .data.console_list ; .align 2
	console_list: .word 0x0
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
	 * In : r0 - address of console_t structure
	 * Out: r0 - Always 1 (for easier tail calling)
	 * Clobber list: r0, r1
	 * -----------------------------------------------
	 */
func console_register
	push	{r6,  lr}
#if ENABLE_ASSERTIONS
	/* Assert that r0 isn't a NULL pointer */
	cmp	r0, #0
	ASM_ASSERT(ne)
	/* Assert that the struct isn't in the stack */
	ldr	r1, =__STACKS_START__
	cmp	r0, r1
	blo	not_on_stack
	ldr	r1, =__STACKS_END__
	cmp	r0, r1
	ASM_ASSERT(hs)
not_on_stack:
	/* Assert that this struct isn't in the list */
	mov	r1, r0 /* Preserve r0 and lr */
	bl	console_is_registered
	cmp	r0, #0
	ASM_ASSERT(eq)
	mov	r0, r1
#endif /* ENABLE_ASSERTIONS */
	ldr	r6, =console_list
	ldr	r1, [r6]	/* R1 = first struct in list */
	str	r0, [r6]	/* list head = new console */
	str	r1, [r0, #CONSOLE_T_NEXT]	/* new console next ptr = R1 */
	mov	r0, #1
	pop	{r6, pc}
endfunc console_register

	/* -----------------------------------------------
	 * int console_unregister(console_t *console)
	 * Function to find a specific console in the list
	 * of currently active consoles and remove it.
	 * In: r0 - address of console_t struct to remove
	 * Out: r0 - removed address, or NULL if not found
	 * Clobber list: r0, r1
	 * -----------------------------------------------
	 */
func console_unregister
#if ENABLE_ASSERTIONS
	/* Assert that r0 isn't a NULL pointer */
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	push	{r6}
	ldr	r6, =console_list		/* R6 = ptr to first struct */
	ldr	r1, [r6]			/* R1 = first struct */

unregister_loop:
	cmp	r1, #0
	beq	unregister_not_found
	cmp	r0, r1
	beq	unregister_found
	ldr	r6, [r6]			/* R6 = next ptr of struct */
	ldr	r1, [r6]			/* R1 = next struct */
	b	unregister_loop

unregister_found:
	ldr	r1, [r1]			/* R1 = next struct */
	str	r1, [r6]			/* prev->next = cur->next */
	pop	{r6}
	bx	lr

unregister_not_found:
	mov	r0, #0				/* return NULL if not found */
	pop	{r6}
	bx	lr
endfunc console_unregister

	/* -----------------------------------------------
	 * int console_is_registered(console_t *console)
	 * Function to detect if a specific console is
	 * registered or not.
	 * In: r0 - address of console_t struct to remove
	 * Out: r0 - 1 if it is registered, 0 if not.
	 * Clobber list: r0
	 * -----------------------------------------------
	 */
func console_is_registered
#if ENABLE_ASSERTIONS
	/* Assert that r0 isn't a NULL pointer */
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	push	{r6}
	ldr	r6, =console_list
	ldr	r6, [r6]	/* R6 = first console struct */
check_registered_loop:
	cmp	r6, #0			/* Check if end of list */
	beq	console_not_registered
	cmp	r0, r6		/* Check if the pointers are different */
	beq	console_registered
	ldr	r6, [r6, #CONSOLE_T_NEXT]	/* Get pointer to next struct */
	b	check_registered_loop
console_not_registered:
	mov	r0, #0
	pop	{r6}
	bx	lr
console_registered:
	mov	r0, #1
	pop	{r6}
	bx	lr
endfunc console_is_registered

	/* -----------------------------------------------
	 * void console_switch_state(unsigned int new_state)
	 * Function to switch the current console state.
	 * The console state determines which of the
	 * registered consoles are actually used at a time.
	 * In : r0 - global console state to move to
	 * Clobber list: r0, r1
	 * -----------------------------------------------
	 */
func console_switch_state
	ldr	r1, =console_state
	strb	r0, [r1]
	bx	lr
endfunc console_switch_state

	/* -----------------------------------------------
	 * void console_set_scope(console_t *console,
	 *                       unsigned int scope)
	 * Function to update the states that a given console
	 * may be active in.
	 * In : r0 - pointer to console_t struct
	 *    : r1 - new active state mask
	 * Clobber list: r0, r1, r2
	 * -----------------------------------------------
	 */
func console_set_scope
#if ENABLE_ASSERTIONS
	ands	r2, r1, #~CONSOLE_FLAG_SCOPE_MASK
	ASM_ASSERT(eq)
#endif /* ENABLE_ASSERTIONS */
	ldr	r2, [r0, #CONSOLE_T_FLAGS]
	and	r2, r2, #~CONSOLE_FLAG_SCOPE_MASK
	orr	r2, r2, r1
	str	r2, [r0, #CONSOLE_T_FLAGS]
	bx	lr
endfunc console_set_scope

	/* ---------------------------------------------
	 * int console_putc(int c)
	 * Function to output a character. Calls all
	 * active console's putc() handlers in succession.
	 * In : r0 - character to be printed
	 * Out: r0 - printed character on success, or < 0
	             if at least one console had an error
	 * Clobber list : r0, r1, r2
	 * ---------------------------------------------
	 */
func console_putc
	push	{r4-r6, lr}
	mov	r5, #ERROR_NO_VALID_CONSOLE	/* R5 = current return value */
	mov	r4, r0				/* R4 = character to print */
	ldr	r6, =console_list
	ldr	r6, [r6]	/* R6 = first console struct */

putc_loop:
	cmp	r6, #0
	beq	putc_done
	ldr	r1, =console_state
	ldrb	r1, [r1]
	ldr	r2, [r6, #CONSOLE_T_FLAGS]
	tst	r1, r2
	beq	putc_continue
	ldr	r2, [r6, #CONSOLE_T_PUTC]
	cmp	r2, #0
	beq	putc_continue
	mov	r0, r4
	mov	r1, r6
	blx	r2
	cmp	r5, #ERROR_NO_VALID_CONSOLE	/* update R5 if it's NOVALID */
	cmpne	r0, #0				/* else update it if R0 < 0 */
	movlt	r5, r0
putc_continue:
	ldr	r6, [r6]			/* R6 = next struct */
	b	putc_loop

putc_done:
	mov	r0, r5
	pop	{r4-r6, pc}
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
	 * Out : r0 - read character, or < 0 on error
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_getc
	push	{r5-r6, lr}
getc_try_again:
	mov	r5, #ERROR_NO_VALID_CONSOLE	/* R5 = current return value */
	ldr	r6, =console_list
	ldr	r6, [r6]			/* R6 = first console struct */
	cmp	r6, #0
	bne	getc_loop
	mov	r0, r5				/* If no consoles registered */
	pop	{r5-r6, pc}			/* return immediately. */

getc_loop:
	ldr	r0, =console_state
	ldrb	r0, [r0]
	ldr	r1, [r6, #CONSOLE_T_FLAGS]
	tst	r0, r1
	beq	getc_continue
	ldr	r1, [r6, #CONSOLE_T_GETC]
	cmp	r1, #0
	beq	getc_continue
	mov	r0, r6
	blx	r1
	cmp	r0, #0				/* if R0 >= 0: return */
	bge	getc_found
	cmp	r5, #ERROR_NO_PENDING_CHAR	/* may update R5 (NOCHAR has */
	movne	r5, r0				/* precedence vs real errors) */
getc_continue:
	ldr	r6, [r6]			/* R6 = next struct */
	cmp	r6, #0
	bne	getc_loop
	cmp	r5, #ERROR_NO_PENDING_CHAR	/* Keep scanning if at least */
	beq	getc_try_again			/* one console returns NOCHAR */
	mov	r0, r5

getc_found:
	pop	{r5-r6, pc}
endfunc console_getc

	/* ---------------------------------------------
	 * int console_flush(void)
	 * Function to force a write of all buffered
	 * data that hasn't been output. Calls all
	 * console's flush() handlers in succession.
	 * Out: r0 - 0 on success, < 0 if at least one error
	 * Clobber list : r0, r1, r2
	 * ---------------------------------------------
	 */
func console_flush
	push	{r5-r6, lr}
	mov	r5, #ERROR_NO_VALID_CONSOLE	/* R5 = current return value */
	ldr	r6, =console_list
	ldr	r6, [r6]			/* R6 = first console struct */

flush_loop:
	cmp	r6, #0
	beq	flush_done
	ldr	r1, =console_state
	ldrb	r1, [r1]
	ldr	r2, [r6, #CONSOLE_T_FLAGS]
	tst	r1, r2
	beq	flush_continue
	ldr	r1, [r6, #CONSOLE_T_FLUSH]
	cmp	r1, #0
	beq	flush_continue
	mov	r0, r6
	blx	r1
	cmp	r5, #ERROR_NO_VALID_CONSOLE	/* update R5 if it's NOVALID */
	cmpne	r0, #0				/* else update it if R0 < 0 */
	movlt	r5, r0
flush_continue:
	ldr	r6, [r6]			/* R6 = next struct */
	b	flush_loop

flush_done:
	mov	r0, r5
	pop	{r5-r6, pc}
endfunc console_flush
