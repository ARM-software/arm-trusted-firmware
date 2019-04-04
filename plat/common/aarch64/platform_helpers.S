/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/console.h>
#include <platform_def.h>

	.weak	plat_report_exception
	.weak	plat_reset_handler
	.weak	plat_disable_acp
	.weak	bl1_plat_prepare_exit
	.weak	plat_panic_handler
	.weak	bl31_plat_enable_mmu
	.weak	bl32_plat_enable_mmu

	.weak	plat_handle_uncontainable_ea
	.weak	plat_handle_double_fault
	.weak	plat_handle_el3_ea

#define MPIDR_RES_BIT_MASK	0xff000000

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
