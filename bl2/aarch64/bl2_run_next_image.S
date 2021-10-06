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
	mov	x20,x0
	/* ---------------------------------------------
	 * MMU needs to be disabled because both BL2 and BL31 execute
	 * in EL3, and therefore share the same address space.
	 * BL31 will initialize the address space according to its
	 * own requirement.
	 * ---------------------------------------------
	 */
	bl	disable_mmu_icache_el3
	tlbi	alle3
	bl	bl2_el3_plat_prepare_exit

#if ENABLE_PAUTH
	/* ---------------------------------------------
	 * Disable pointer authentication before jumping
	 * to next boot image.
	 * ---------------------------------------------
	 */
	bl	pauth_disable_el3
#endif /* ENABLE_PAUTH */

	ldp	x0, x1, [x20, #ENTRY_POINT_INFO_PC_OFFSET]
	msr	elr_el3, x0
	msr	spsr_el3, x1

	ldp	x6, x7, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x30)]
	ldp	x4, x5, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x20)]
	ldp	x2, x3, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x10)]
	ldp	x0, x1, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x0)]
	exception_return
endfunc bl2_run_next_image
