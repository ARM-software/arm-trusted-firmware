/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <drivers/console.h>
#include <platform_def.h>

	.weak	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.weak	platform_mem_init
	.globl	plat_ls_calc_core_pos


	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_ls_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_ls_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_ls_calc_core_pos(u_register_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ClusterId * 4) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_ls_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_ls_calc_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */

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

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on LS
	 * platforms. The Secure SRAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init
