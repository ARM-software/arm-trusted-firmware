/*
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <el3_common_macros.S>

	.globl	bl2_entrypoint


func bl2_entrypoint
	/* Save arguments x0-x3 from previous Boot loader */
	mov	r9, r0
	mov	r10, r1
	mov	r11, r2
	mov	r12, r3

	el3_entrypoint_common                                   \
                _init_sctlr=1                                   \
                _warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS  \
                _secondary_cold_boot=!COLD_BOOT_SINGLE_CPU      \
                _init_memory=1                                  \
                _init_c_runtime=1                               \
                _exception_vectors=bl2_vector_table		\
		_pie_fixup_size=0

	/*
	 * Restore parameters of boot rom
	 */
	mov	r0, r9
	mov	r1, r10
	mov	r2, r11
	mov	r3, r12

	/* ---------------------------------------------
	 * Perform BL2 setup
	 * ---------------------------------------------
	 */
	bl	bl2_el3_setup

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
