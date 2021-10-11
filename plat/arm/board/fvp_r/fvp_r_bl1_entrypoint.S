/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <el2_common_macros.S>
#include <lib/xlat_mpu/xlat_mpu.h>

	.globl	bl1_entrypoint
	.globl	bl1_run_next_image


	/* -----------------------------------------------------
	 * bl1_entrypoint() is the entry point into the trusted
	 * firmware code when a cpu is released from warm or
	 * cold reset.
	 * -----------------------------------------------------
	 */

func bl1_entrypoint
	/* ---------------------------------------------------------------------
	 * If the reset address is programmable then bl1_entrypoint() is
	 * executed only on the cold boot path. Therefore, we can skip the warm
	 * boot mailbox mechanism.
	 * ---------------------------------------------------------------------
	 */
	el2_entrypoint_common					\
		_init_sctlr=1					\
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS	\
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU	\
		_init_memory=1					\
		_init_c_runtime=1				\
		_exception_vectors=bl1_exceptions		\
		_pie_fixup_size=0

	/* --------------------------------------------------------------------
	 * Perform BL1 setup
	 * --------------------------------------------------------------------
	 */
	bl	bl1_setup

	/* --------------------------------------------------------------------
	 * Initialize platform and jump to our c-entry point
	 * for this type of reset.
	 * --------------------------------------------------------------------
	 */
	bl	bl1_main

	/* ---------------------------------------------
	 * Should never reach this point.
	 * ---------------------------------------------
	 */
	no_ret	plat_panic_handler
endfunc bl1_entrypoint

func bl1_run_next_image
	mov	x20,x0

	/* ---------------------------------------------
	 * MPU needs to be disabled because both BL1 and BL33 execute
	 * in EL2, and therefore share the same address space.
	 * BL33 will initialize the address space according to its
	 * own requirement.
	 * ---------------------------------------------
	 */
	bl	disable_mpu_icache_el2

	/* ---------------------------------------------
	 * Wipe clean and disable all MPU regions.  This function expects
	 * that the MPU has already been turned off, and caching concerns
	 * addressed, but it also explicitly turns off the MPU.
	 * ---------------------------------------------
	 */
	bl	clear_all_mpu_regions

	/* --------------------------------------------------
	 * Do the transition to next boot image.
	 * --------------------------------------------------
	 */
	ldp	x0, x1, [x20, #ENTRY_POINT_INFO_PC_OFFSET]
	msr	elr_el2, x0
	msr	spsr_el2, x1

	ldp	x6, x7, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x30)]
	ldp	x4, x5, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x20)]
	ldp	x2, x3, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x10)]
	ldp	x0, x1, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x0)]
	exception_return
endfunc bl1_run_next_image
