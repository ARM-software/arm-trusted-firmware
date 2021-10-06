/*
 * Copyright (c) 2013-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <common/bl_common.h>
#include <el3_common_macros.S>

	.globl	bl1_entrypoint
	.globl	bl1_run_bl2_in_root


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
	el3_entrypoint_common					\
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

#if ENABLE_PAUTH
	/* --------------------------------------------------------------------
	 * Program APIAKey_EL1 and enable pointer authentication.
	 * --------------------------------------------------------------------
	 */
	bl	pauth_init_enable_el3
#endif /* ENABLE_PAUTH */

	/* --------------------------------------------------------------------
	 * Initialize platform and jump to our c-entry point
	 * for this type of reset.
	 * --------------------------------------------------------------------
	 */
	bl	bl1_main

#if ENABLE_PAUTH
	/* --------------------------------------------------------------------
	 * Disable pointer authentication before jumping to next boot image.
	 * --------------------------------------------------------------------
	 */
	bl	pauth_disable_el3
#endif /* ENABLE_PAUTH */

	/* --------------------------------------------------
	 * Do the transition to next boot image.
	 * --------------------------------------------------
	 */
#if ENABLE_RME
	b	bl1_run_bl2_in_root
#else
	b	el3_exit
#endif
endfunc bl1_entrypoint

	/* -----------------------------------------------------
	 * void bl1_run_bl2_in_root();
	 * This function runs BL2 in root/EL3 when RME is enabled.
	 * -----------------------------------------------------
	 */

func bl1_run_bl2_in_root
	/* read bl2_ep_info */
	adrp	x20, bl2_ep_info
	add	x20, x20, :lo12:bl2_ep_info
	ldr	x20, [x20]

	/* ---------------------------------------------
	 * MMU needs to be disabled because BL2 executes
	 * in EL3. It will initialize the address space
	 * according to its own requirements.
	 * ---------------------------------------------
	 */
	bl	disable_mmu_icache_el3
	tlbi	alle3

	ldp	x0, x1, [x20, #ENTRY_POINT_INFO_PC_OFFSET]
	msr	elr_el3, x0
	msr	spsr_el3, x1

	ldp	x6, x7, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x30)]
	ldp	x4, x5, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x20)]
	ldp	x2, x3, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x10)]
	ldp	x0, x1, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x0)]
	exception_return
endfunc bl1_run_bl2_in_root
