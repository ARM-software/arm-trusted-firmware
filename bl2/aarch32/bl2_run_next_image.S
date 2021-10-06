/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>

	.globl	bl2_run_next_image


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
