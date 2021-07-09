/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <el3_common_macros.S>

	.globl	bl2_entrypoint


func bl2_entrypoint
	/* Save arguments x0-x3 from previous Boot loader */
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
	mov	x23, x3

	el3_entrypoint_common                                   \
		_init_sctlr=0                                   \
		_warm_boot_mailbox=0                            \
		_secondary_cold_boot=0                          \
		_init_memory=0                                  \
		_init_c_runtime=1                               \
		_exception_vectors=bl2_el3_exceptions           \
		_pie_fixup_size=0

	/* ---------------------------------------------
	 * Restore parameters of boot rom
	 * ---------------------------------------------
	 */
	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23

	/* ---------------------------------------------
	 * Perform BL2 setup
	 * ---------------------------------------------
	 */
	bl	bl2_setup

#if ENABLE_PAUTH
	/* ---------------------------------------------
	 * Program APIAKey_EL1 and enable pointer authentication.
	 * ---------------------------------------------
	 */
	bl	pauth_init_enable_el3
#endif /* ENABLE_PAUTH */

	/* ---------------------------------------------
	 * Jump to main function.
	 * ---------------------------------------------
	 */
	bl	bl2_main

	/* ---------------------------------------------
	 * Should never reach this point.
	 * ---------------------------------------------
	 */
	no_ret	plat_panic_handler
endfunc bl2_entrypoint
