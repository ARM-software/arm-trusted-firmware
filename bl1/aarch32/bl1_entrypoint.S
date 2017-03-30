/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <context.h>
#include <el3_common_macros.S>
#include <smcc_helpers.h>
#include <smcc_macros.S>

	.globl	bl1_vector_table
	.globl	bl1_entrypoint

	/* -----------------------------------------------------
	 * Setup the vector table to support SVC & MON mode.
	 * -----------------------------------------------------
	 */
vector_base bl1_vector_table
	b	bl1_entrypoint
	b	report_exception	/* Undef */
	b	bl1_aarch32_smc_handler	/* SMC call */
	b	report_exception	/* Prefetch abort */
	b	report_exception	/* Data abort */
	b	report_exception	/* Reserved */
	b	report_exception	/* IRQ */
	b	report_exception	/* FIQ */

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
		_set_endian=1					\
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS	\
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU	\
		_init_memory=1					\
		_init_c_runtime=1				\
		_exception_vectors=bl1_vector_table

	/* -----------------------------------------------------
	 * Perform early platform setup & platform
	 * specific early arch. setup e.g. mmu setup
	 * -----------------------------------------------------
	 */
	bl	bl1_early_platform_setup
	bl	bl1_plat_arch_setup

	/* -----------------------------------------------------
	 * Jump to main function.
	 * -----------------------------------------------------
	 */
	bl	bl1_main

	/* -----------------------------------------------------
	 * Jump to next image.
	 * -----------------------------------------------------
	 */

	/*
	 * MMU needs to be disabled because both BL1 and BL2 execute
	 * in PL1, and therefore share the same address space.
	 * BL2 will initialize the address space according to its
	 * own requirement.
	 */
	bl	disable_mmu_icache_secure
	stcopr	r0, TLBIALL
	dsb	sy
	isb

	/*
	 * Get the smc_context for next BL image,
	 * program the gp/system registers and exit
	 * secure monitor mode
	 */
	bl	smc_get_next_ctx
	monitor_exit
endfunc bl1_entrypoint
