/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.weak	plat_crash_console_init
	.weak	plat_crash_console_putc
	.weak	plat_crash_console_flush
	.weak	plat_reset_handler
	.weak	plat_disable_acp
	.weak	platform_mem_init
	.weak	plat_panic_handler
	.weak	bl2_plat_preload_setup
	.weak	plat_try_next_boot_source

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_crash_console_init
	mov	r0, #0
	bx	lr
endfunc plat_crash_console_init

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_crash_console_putc
	bx	lr
endfunc plat_crash_console_putc

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_crash_console_flush
	mov	r0, #0
	bx	lr
endfunc plat_crash_console_flush

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_reset_handler
	bx	lr
endfunc plat_reset_handler

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_disable_acp
	bx	lr
endfunc plat_disable_acp

	/* ---------------------------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	bx	lr
endfunc platform_mem_init

	/* -----------------------------------------------------
	 * void plat_panic_handler(void) __dead2;
	 * Endless loop by default.
	 * -----------------------------------------------------
	 */
func plat_panic_handler
	b	plat_panic_handler
endfunc plat_panic_handler


	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platfrom.
	 * -----------------------------------------------------
	 */
func bl2_plat_preload_setup
	bx	lr
endfunc bl2_plat_preload_setup

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platfrom.
	 * -----------------------------------------------------
	 */
func plat_try_next_boot_source
	mov	r0, #0
	bx	lr
endfunc plat_try_next_boot_source
