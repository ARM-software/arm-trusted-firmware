/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.weak	plat_report_exception
	.weak	plat_crash_console_init
	.weak	plat_crash_console_putc
	.weak	plat_crash_console_flush
	.weak	plat_reset_handler
	.weak	plat_disable_acp
	.weak	bl1_plat_prepare_exit
	.weak	platform_mem_init
	.weak	plat_error_handler
	.weak	plat_panic_handler
	.weak	bl2_plat_preload_setup
	.weak	plat_try_next_boot_source

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_report_exception
	bx	lr
endfunc plat_report_exception

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
	 * void bl1_plat_prepare_exit(entry_point_info_t *ep_info);
	 * Called before exiting BL1. Default: do nothing
	 * -----------------------------------------------------
	 */
func bl1_plat_prepare_exit
	bx	lr
endfunc bl1_plat_prepare_exit

	/* -----------------------------------------------------
	 * void plat_error_handler(int err) __dead2;
	 * Endless loop by default.
	 * -----------------------------------------------------
	 */
func plat_error_handler
	wfi
	b	plat_error_handler
endfunc plat_error_handler

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
