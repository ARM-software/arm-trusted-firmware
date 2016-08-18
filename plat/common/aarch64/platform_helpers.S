/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.weak	plat_report_exception
	.weak	plat_crash_console_init
	.weak	plat_crash_console_putc
	.weak	plat_reset_handler
	.weak	plat_disable_acp
	.weak	bl1_plat_prepare_exit
	.weak	plat_error_handler
	.weak	plat_panic_handler

#if !ENABLE_PLAT_COMPAT
	.globl	platform_get_core_pos

#define MPIDR_RES_BIT_MASK	0xff000000

	/* ------------------------------------------------------------------
	 *  int platform_get_core_pos(int mpidr)
	 *  Returns the CPU index of the CPU specified by mpidr. This is
	 *  defined when platform compatibility is disabled to enable Trusted
	 *  Firmware components like SPD using the old  platform API to work.
	 *  This API is deprecated and it assumes that the mpidr specified is
	 *  that of a valid and present CPU. Instead, plat_my_core_pos()
	 *  should be used for CPU index of the current CPU and
	 *  plat_core_pos_by_mpidr() should be used for CPU index of a
	 *  CPU specified by its mpidr.
	 * ------------------------------------------------------------------
	 */
func_deprecated platform_get_core_pos
	bic	x0, x0, #MPIDR_RES_BIT_MASK
	mrs	x1, mpidr_el1
	bic	x1, x1, #MPIDR_RES_BIT_MASK
	cmp	x0, x1
	beq	plat_my_core_pos
	b	platform_core_pos_helper
endfunc_deprecated platform_get_core_pos
#endif

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_report_exception
	ret
endfunc plat_report_exception

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_crash_console_init
	mov	x0, #0
	ret
endfunc plat_crash_console_init

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func plat_crash_console_putc
	ret
endfunc plat_crash_console_putc

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform. This function should preserve x10.
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
	wfi
	b	plat_panic_handler
endfunc plat_panic_handler
