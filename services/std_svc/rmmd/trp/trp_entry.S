/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <services/rmmd_svc.h>
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

	bl	trp_setup

	bl	trp_main
warm_boot:
	mov_imm	x0, RMMD_RMI_REQ_COMPLETE
	mov	x1, xzr
	smc	#0
	b	trp_handler

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
	bl	trp_rmi_handler
	restore_args_call_smc
	b	trp_handler
endfunc trp_handler
