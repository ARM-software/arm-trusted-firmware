/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <services/rmmd_svc.h>

#include <platform_def.h>
#include "trp_private.h"

.global trp_head
.global trp_smc

.section ".head.text", "ax"

	/* ---------------------------------------------
	 * Populate the params in x0-x7 from the pointer
	 * to the smc args structure in x0.
	 * ---------------------------------------------
	 */
	.macro restore_args_call_smc
	ldp	x6, x7, [x0, #TRP_ARG6]
	ldp	x4, x5, [x0, #TRP_ARG4]
	ldp	x2, x3, [x0, #TRP_ARG2]
	ldp	x0, x1, [x0, #TRP_ARG0]
	smc	#0
	.endm

	/* ---------------------------------------------
	 * Entry point for TRP
	 * ---------------------------------------------
	 */
trp_head:
	/*
	 * Stash arguments from previous boot stage
	 */
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
	mov	x23, x3

	/*
	 * Validate CPUId before allocating a stack.
	 */
	cmp	x20, #PLATFORM_CORE_COUNT
	b.lo	1f

	mov_imm	x0, RMM_BOOT_COMPLETE
	mov_imm	x1, E_RMM_BOOT_CPU_ID_OUT_OF_RANGE
	smc	#0

	/* EL3 should never return back here, so panic if it does */
	b	trp_panic

1:
	bl	plat_set_my_stack

	/*
	 * Find out whether this is a cold or warm boot
	 */
	ldr	x1, cold_boot_flag
	cbz	x1, warm_boot

	/*
	 * Update cold boot flag to indicate cold boot is done
	 */
	adr	x2, cold_boot_flag
	str	xzr, [x2]

	/* ---------------------------------------------
	 * Zero out BSS section
	 * ---------------------------------------------
	 */
	ldr	x0, =__BSS_START__
	ldr	x1, =__BSS_SIZE__
	bl	zeromem

	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23
	bl	trp_setup
	bl	trp_main
	b	1f

warm_boot:
	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23
	bl	trp_validate_warmboot_args
	cbnz	x0, trp_panic /* Failed to validate warmboot args */

1:
	mov_imm	x0, RMM_BOOT_COMPLETE
	mov	x1, xzr /* RMM_BOOT_SUCCESS */
	smc	#0
	b	trp_handler

trp_panic:
	no_ret plat_panic_handler

	/*
	 * Flag to mark if it is a cold boot.
	 * 1: cold boot, 0: warmboot.
	 */
.align 3
cold_boot_flag:
	.dword		1

	/* ---------------------------------------------
	 *   Direct SMC call to BL31 service provided by
	 *   RMM Dispatcher
	 * ---------------------------------------------
	 */
func trp_smc
	restore_args_call_smc
	ret
endfunc trp_smc

	/* ---------------------------------------------
	 * RMI call handler
	 * ---------------------------------------------
	 */
func trp_handler
	/*
	 * Save Link Register and X4, as per SMCCC v1.2 its value
	 * must be preserved unless it contains result, as specified
	 * in the function definition.
	 */
	stp	x4, lr, [sp, #-16]!

	/*
	 * Zero the space for X0-X3 in trp_smc_result structure
	 * and pass its address as the last argument.
	 */
	stp	xzr, xzr, [sp, #-16]!
	stp	xzr, xzr, [sp, #-16]!
	mov	x7, sp

	bl	trp_rmi_handler

	ldp	x1, x2, [sp], #16
	ldp	x3, x4, [sp], #16
	ldp	x5, lr, [sp], #16

	ldr	x0, =RMM_RMI_REQ_COMPLETE
	smc	#0

	b	trp_handler
endfunc trp_handler
