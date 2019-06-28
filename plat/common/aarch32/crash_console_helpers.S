/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
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

	/* -----------------------------------------------------
	 * int plat_crash_console_init(void)
	 * Use normal console by default. Switch it to crash
	 * mode so serial consoles become active again.
	 * NOTE: This default implementation will only work for
	 * crashes that occur after a normal console (marked
	 * valid for the crash state) has been registered with
	 * the console framework. To debug crashes that occur
	 * earlier, the platform has to override these functions
	 * with an implementation that initializes a console
	 * driver with hardcoded parameters. See
	 * docs/porting-guide.rst for more information.
	 * -----------------------------------------------------
	 */
func plat_crash_console_init
#if defined(IMAGE_BL1)
	/*
	 * BL1 code can possibly crash so early that the data segment is not yet
	 * accessible. Don't risk undefined behavior by trying to run the normal
	 * console framework. Platforms that want to debug BL1 will need to
	 * override this with custom functions that can run from registers only.
	 */
	mov	r0, #0
	bx	lr
#else	/* IMAGE_BL1 */
	mov	r3, lr
	mov	r0, #CONSOLE_FLAG_CRASH
	bl	console_switch_state
	mov	r0, #1
	bx	r3
#endif
endfunc plat_crash_console_init

	/* -----------------------------------------------------
	 * void plat_crash_console_putc(int character)
	 * Output through the normal console by default.
	 * -----------------------------------------------------
	 */
func plat_crash_console_putc
	b	console_putc
endfunc plat_crash_console_putc

	/* -----------------------------------------------------
	 * void plat_crash_console_flush(void)
	 * Flush normal console by default.
	 * -----------------------------------------------------
	 */
func plat_crash_console_flush
	b	console_flush
endfunc plat_crash_console_flush
