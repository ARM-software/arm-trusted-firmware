/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <context.h>
#include <el3_common_macros.S>
#include <smccc_helpers.h>
#include <smccc_macros.S>

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
		_init_sctlr=1					\
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS	\
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU	\
		_init_memory=1					\
		_init_c_runtime=1				\
		_exception_vectors=bl1_vector_table

	/* -----------------------------------------------------
	 * Perform BL1 setup
	 * -----------------------------------------------------
	 */
	bl	bl1_setup

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
	 * Get the smc_context for next BL image,
	 * program the gp/system registers and save it in `r4`.
	 */
	bl	smc_get_next_ctx
	mov	r4, r0

	/* Only turn-off MMU if going to secure world */
	ldr	r5, [r4, #SMC_CTX_SCR]
	tst	r5, #SCR_NS_BIT
	bne	skip_mmu_off

	/*
	 * MMU needs to be disabled because both BL1 and BL2/BL2U execute
	 * in PL1, and therefore share the same address space.
	 * BL2/BL2U will initialize the address space according to its
	 * own requirement.
	 */
	bl	disable_mmu_icache_secure
	stcopr	r0, TLBIALL
	dsb	sy
	isb

skip_mmu_off:
	/* Restore smc_context from `r4` and exit secure monitor mode. */
	mov	r0, r4
	monitor_exit
endfunc bl1_entrypoint
