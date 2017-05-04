/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl1.h>
#include <bl_common.h>

	.globl	bl1_aarch32_smc_handler


func bl1_aarch32_smc_handler
	/* ------------------------------------------------
	 * SMC in BL1 is handled assuming that the MMU is
	 * turned off by BL2.
	 * ------------------------------------------------
	 */

	/* ----------------------------------------------
	 * Only RUN_IMAGE SMC is supported.
	 * ----------------------------------------------
	 */
	mov	r8, #BL1_SMC_RUN_IMAGE
	cmp	r8, r0
	blne	report_exception

	/* ------------------------------------------------
	 * Make sure only Secure world reaches here.
	 * ------------------------------------------------
	 */
	ldcopr  r8, SCR
	tst	r8, #SCR_NS_BIT
	blne	report_exception

	/* ---------------------------------------------------------------------
	 * Pass control to next secure image.
	 * Here it expects r1 to contain the address of a entry_point_info_t
	 * structure describing the BL entrypoint.
	 * ---------------------------------------------------------------------
	 */
	mov	r8, r1
	mov	r0, r1
	bl	bl1_print_next_bl_ep_info

#if SPIN_ON_BL1_EXIT
	bl	print_debug_loop_message
debug_loop:
	b	debug_loop
#endif

	mov	r0, r8
	bl	bl1_plat_prepare_exit

	stcopr	r0, TLBIALL
	dsb	sy
	isb

	/*
	 * Extract PC and SPSR based on struct `entry_point_info_t`
	 * and load it in LR and SPSR registers respectively.
	 */
	ldr	lr, [r8, #ENTRY_POINT_INFO_PC_OFFSET]
	ldr	r1, [r8, #(ENTRY_POINT_INFO_PC_OFFSET + 4)]
	msr	spsr, r1

	add	r8, r8, #ENTRY_POINT_INFO_ARGS_OFFSET
	ldm	r8, {r0, r1, r2, r3}
	eret
endfunc bl1_aarch32_smc_handler
