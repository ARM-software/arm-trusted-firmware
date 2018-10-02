/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <console.h>
#include <platform_def.h>

	.weak	plat_report_exception
	.weak	plat_crash_console_init
	.weak	plat_crash_console_putc
	.weak	plat_crash_console_flush
	.weak	plat_reset_handler
	.weak	plat_disable_acp
	.weak	bl1_plat_prepare_exit
	.weak	plat_panic_handler
	.weak	bl31_plat_enable_mmu
	.weak	bl32_plat_enable_mmu

	.weak	plat_handle_uncontainable_ea
	.weak	plat_handle_double_fault
	.weak	plat_handle_el3_ea

	.globl	platform_get_core_pos

#define MPIDR_RES_BIT_MASK	0xff000000

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_report_exception
	ret
endfunc plat_report_exception

#if MULTI_CONSOLE_API
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
	mov	x0, #0
	ret
#else	/* IMAGE_BL1 */
	mov	x3, x30
	mov	x0, #CONSOLE_FLAG_CRASH
	bl	console_switch_state
	mov	x0, #1
	ret	x3
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

#else	/* MULTI_CONSOLE_API */

	/* -----------------------------------------------------
	 * In the old API these are all no-op stubs that need to
	 * be overridden by the platform to be useful.
	 * -----------------------------------------------------
	 */
func plat_crash_console_init
	mov	x0, #0
	ret
endfunc plat_crash_console_init

func plat_crash_console_putc
	ret
endfunc plat_crash_console_putc

func plat_crash_console_flush
	ret
endfunc plat_crash_console_flush
#endif

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform. This function should preserve x19 - x29.
	 * -----------------------------------------------------
	 */
func plat_reset_handler
	ret
endfunc plat_reset_handler

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform. This function is allowed to use
	 * registers x0 - x17.
	 * -----------------------------------------------------
	 */
func plat_disable_acp
	ret
endfunc plat_disable_acp

	/* -----------------------------------------------------
	 * void bl1_plat_prepare_exit(entry_point_info_t *ep_info);
	 * Called before exiting BL1. Default: do nothing
	 * -----------------------------------------------------
	 */
func bl1_plat_prepare_exit
	ret
endfunc bl1_plat_prepare_exit

	/* -----------------------------------------------------
	 * void plat_panic_handler(void) __dead2;
	 * Endless loop by default.
	 * -----------------------------------------------------
	 */
func plat_panic_handler
	wfi
	b	plat_panic_handler
endfunc plat_panic_handler

	/* -----------------------------------------------------
	 * void bl31_plat_enable_mmu(uint32_t flags);
	 *
	 * Enable MMU in BL31.
	 * -----------------------------------------------------
	 */
func bl31_plat_enable_mmu
	b	enable_mmu_direct_el3
endfunc bl31_plat_enable_mmu

	/* -----------------------------------------------------
	 * void bl32_plat_enable_mmu(uint32_t flags);
	 *
	 * Enable MMU in BL32.
	 * -----------------------------------------------------
	 */
func bl32_plat_enable_mmu
	b	enable_mmu_direct_el1
endfunc bl32_plat_enable_mmu


	/* -----------------------------------------------------
	 * Platform handler for Uncontainable External Abort.
	 *
	 * x0: EA reason
	 * x1: EA syndrome
	 * -----------------------------------------------------
	 */
func plat_handle_uncontainable_ea
	b	report_unhandled_exception
endfunc plat_handle_uncontainable_ea

	/* -----------------------------------------------------
	 * Platform handler for Double Fault.
	 *
	 * x0: EA reason
	 * x1: EA syndrome
	 * -----------------------------------------------------
	 */
func plat_handle_double_fault
	b	report_unhandled_exception
endfunc plat_handle_double_fault

	/* -----------------------------------------------------
	 * Platform handler for EL3 External Abort.
	 * -----------------------------------------------------
	 */
func plat_handle_el3_ea
	b	report_unhandled_exception
endfunc plat_handle_el3_ea
