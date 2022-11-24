/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * If a platform wishes to use the functions in this file it has to be added to
 * the Makefile of the platform. It is not included in the common Makefile.
 */

#include <asm_macros.S>
#include <drivers/console.h>

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush

	/*
	 * Spinlock to syncronize access to crash_console_triggered. We cannot
	 * acquire spinlocks when the cache is disabled, so in some cases (like
	 * late during CPU suspend) some risk remains.
	 */
.section .data.crash_console_spinlock
	define_asm_spinlock crash_console_spinlock

	/*
	 * Flag to make sure that only one CPU can write a crash dump even if
	 * multiple crash at the same time. Interleaving crash dumps on the same
	 * console would just make the output unreadable, so it's better to only
	 * get a single but uncorrupted dump. This also means that we don't have
	 * to duplicate the reg_stash below for each CPU.
	 */
.section .data.crash_console_triggered
	crash_console_triggered: .byte 0

	/*
	 * Space to stash away some register values while we're calling into
	 * console drivers and don't have a real stack available. We need x14,
	 * x15 and x30 for bookkeeping within the plat_crash_console functions
	 * themselves, and some console drivers use x16 and x17 as additional
	 * scratch space that is not preserved by the main crash reporting
	 * framework. (Note that x16 and x17 should really never be expected to
	 * retain their values across any function call, even between carefully
	 * designed assembly functions, since the linker is always free to
	 * insert a function call veneer that uses these registers as scratch
	 * space at any time. The current crash reporting framework doesn't
	 * really respect that, but since TF is usually linked as a single
	 * contiguous binary of less than 128MB, it seems to work in practice.)
	 */
.section .data.crash_console_reg_stash
	.align 3
	crash_console_reg_stash: .quad 0, 0, 0, 0, 0

	/* --------------------------------------------------------------------
	 * int plat_crash_console_init(void)
	 * Takes the crash console spinlock (if possible) and checks the trigger
	 * flag to make sure we're the first CPU to dump. If not, return an
	 * error (so crash dumping will fail but the CPU will still call
	 * plat_panic_handler() which may do important platform-specific tasks
	 * that may be needed on all crashing CPUs). In either case, the lock
	 * will be released so other CPUs can make forward progress on this.
	 * Clobbers: x0 - x4, x30
	 * --------------------------------------------------------------------
	 */
func plat_crash_console_init
#if defined(IMAGE_BL31)
	mov	x4, x30		/* x3 and x4 are not clobbered by spin_lock() */
	mov	x3, #0		/* return value */

	adrp	x0, crash_console_spinlock
	add	x0, x0, :lo12:crash_console_spinlock

	mrs	x1, sctlr_el3
	tst	x1, #SCTLR_C_BIT
	beq	skip_spinlock	/* can't synchronize when cache disabled */
	bl	spin_lock

skip_spinlock:
	adrp	x1, crash_console_triggered
	add	x1, x1, :lo12:crash_console_triggered
	ldarb	w2, [x1]
	cmp	w2, #0
	bne	init_error

	mov	x3, #1		/* set return value to success */
	stlrb	w3, [x1]

init_error:
	bl	spin_unlock	/* harmless if we didn't acquire the lock */
	mov	x0, x3
	ret	x4
#else	/* Only one CPU in BL1/BL2, no need to synchronize anything */
	mov	x0, #1
	ret
#endif
endfunc plat_crash_console_init

	/* --------------------------------------------------------------------
	 * int plat_crash_console_putc(char c)
	 * Prints the character on all consoles registered with the console
	 * framework that have CONSOLE_FLAG_CRASH set. Note that this is only
	 * helpful for crashes that occur after the platform intialization code
	 * has registered a console. Platforms using this implementation need to
	 * ensure that all console drivers they use that have the CRASH flag set
	 * support this (i.e. are written in assembly and comply to the register
	 * clobber requirements of plat_crash_console_putc().
	 * --------------------------------------------------------------------
	 */
func plat_crash_console_putc
	adrp	x1, crash_console_reg_stash
	add	x1, x1, :lo12:crash_console_reg_stash
	stp	x14, x15, [x1]
	stp	x16, x17, [x1, #16]
	str	x30, [x1, #32]

	mov	w14, w0				/* W14 = character to print */
	adrp	x15, console_list
	ldr	x15, [x15, :lo12:console_list]	/* X15 = first console struct */

putc_loop:
	cbz	x15, putc_done
	ldr	w1, [x15, #CONSOLE_T_FLAGS]
	tst	w1, #CONSOLE_FLAG_CRASH
	b.eq	putc_continue
	ldr	x2, [x15, #CONSOLE_T_PUTC]
	cbz	x2, putc_continue
	cmp	w14, #'\n'
	b.ne	putc
	tst	w1, #CONSOLE_FLAG_TRANSLATE_CRLF
	b.eq	putc
	mov	x1, x15
	mov	w0, #'\r'
	blr	x2
	ldr	x2, [x15, #CONSOLE_T_PUTC]
putc:
	mov	x1, x15
	mov	w0, w14
	blr	x2
putc_continue:
	ldr	x15, [x15]			/* X15 = next struct */
	b	putc_loop

putc_done:
	adrp	x1, crash_console_reg_stash
	add	x1, x1, :lo12:crash_console_reg_stash
	ldp	x14, x15, [x1]
	ldp	x16, x17, [x1, #16]
	ldr	x30, [x1, #32]
	ret
endfunc plat_crash_console_putc

	/* --------------------------------------------------------------------
	 * int plat_crash_console_flush(char c)
	 * Flushes all consoles registered with the console framework that have
	 * CONSOLE_FLAG_CRASH set. Same requirements as putc().
	 * --------------------------------------------------------------------
	 */
func plat_crash_console_flush
	adrp	x1, crash_console_reg_stash
	add	x1, x1, :lo12:crash_console_reg_stash
	stp	x30, x15, [x1]
	stp	x16, x17, [x1, #16]

	adrp	x15, console_list
	ldr	x15, [x15, :lo12:console_list]	/* X15 = first console struct */

flush_loop:
	cbz	x15, flush_done
	ldr	w1, [x15, #CONSOLE_T_FLAGS]
	tst	w1, #CONSOLE_FLAG_CRASH
	b.eq	flush_continue
	ldr	x2, [x15, #CONSOLE_T_FLUSH]
	cbz	x2, flush_continue
	mov	x0, x15
	blr	x2
flush_continue:
	ldr	x15, [x15]			/* X15 = next struct */
	b	flush_loop

flush_done:
	adrp	x1, crash_console_reg_stash
	add	x1, x1, :lo12:crash_console_reg_stash
	ldp	x30, x15, [x1]
	ldp	x16, x17, [x1, #16]
	ret
endfunc plat_crash_console_flush
