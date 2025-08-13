/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <el3_common_macros.S>

	.globl	bl2_entrypoint

#if BL2_IN_XIP_MEM || ENABLE_RME
#define FIXUP_SIZE	0
#else
#define FIXUP_SIZE	((BL2_LIMIT) - (BL2_BASE))
#endif

func bl2_entrypoint
	/* Save arguments x0-x3 from previous Boot loader */
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
	mov	x23, x3

	el3_entrypoint_common							\
		_init_sctlr=RESET_TO_BL2					\
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS && RESET_TO_BL2	\
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU && RESET_TO_BL2	\
		_init_memory=RESET_TO_BL2					\
		_init_c_runtime=1						\
		_exception_vectors=bl2_el3_exceptions				\
		_pie_fixup_size=FIXUP_SIZE

	/* ---------------------------------------------
	 * Restore parameters of boot rom
	 * ---------------------------------------------
	 */
	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23

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
