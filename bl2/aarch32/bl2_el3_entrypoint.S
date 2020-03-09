/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <el3_common_macros.S>

	.globl	bl2_entrypoint
	.globl	bl2_run_next_image


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
                _exception_vectors=bl2_vector_table

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

func bl2_run_next_image
	mov	r8,r0

	/*
	 * MMU needs to be disabled because both BL2 and BL32 execute
	 * in PL1, and therefore share the same address space.
	 * BL32 will initialize the address space according to its
	 * own requirement.
	 */
	bl	disable_mmu_icache_secure
	stcopr	r0, TLBIALL
	dsb	sy
	isb
	mov	r0, r8
	bl	bl2_el3_plat_prepare_exit

	/*
	 * Extract PC and SPSR based on struct `entry_point_info_t`
	 * and load it in LR and SPSR registers respectively.
	 */
	ldr	lr, [r8, #ENTRY_POINT_INFO_PC_OFFSET]
	ldr	r1, [r8, #(ENTRY_POINT_INFO_PC_OFFSET + 4)]
	msr	spsr_xc, r1

	/* Some BL32 stages expect lr_svc to provide the BL33 entry address */
	cps	#MODE32_svc
	ldr	lr, [r8, #ENTRY_POINT_INFO_LR_SVC_OFFSET]
	cps	#MODE32_mon

	add	r8, r8, #ENTRY_POINT_INFO_ARGS_OFFSET
	ldm	r8, {r0, r1, r2, r3}
	exception_return
endfunc bl2_run_next_image
