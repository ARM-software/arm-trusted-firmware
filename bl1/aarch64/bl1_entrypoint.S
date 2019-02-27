/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <el3_common_macros.S>

	.globl	bl1_entrypoint


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
		_exception_vectors=bl1_exceptions

	/* --------------------------------------------------------------------
	 * Perform BL1 setup
	 * --------------------------------------------------------------------
	 */
	bl	bl1_setup

	/* --------------------------------------------------------------------
	 * Enable pointer authentication
	 * --------------------------------------------------------------------
	 */
#if ENABLE_PAUTH
	mrs	x0, sctlr_el3
	orr	x0, x0, #SCTLR_EnIA_BIT
	msr	sctlr_el3, x0
	isb
#endif /* ENABLE_PAUTH */

	/* --------------------------------------------------------------------
	 * Initialize platform and jump to our c-entry point
	 * for this type of reset.
	 * --------------------------------------------------------------------
	 */
	bl	bl1_main

	/* --------------------------------------------------------------------
	 * Disable pointer authentication before jumping to BL31 or that will
	 * cause an authentication failure during the early platform init.
	 * --------------------------------------------------------------------
	 */
#if ENABLE_PAUTH
	mrs	x0, sctlr_el3
	bic	x0, x0, #SCTLR_EnIA_BIT
	msr	sctlr_el3, x0
	isb
#endif /* ENABLE_PAUTH */

	/* --------------------------------------------------
	 * Do the transition to next boot image.
	 * --------------------------------------------------
	 */
	b	el3_exit
endfunc bl1_entrypoint
