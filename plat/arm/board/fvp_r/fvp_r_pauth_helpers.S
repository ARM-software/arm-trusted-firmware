/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <lib/el3_runtime/cpu_data.h>

	.global	pauth_init_enable_el2
	.global	pauth_disable_el2

/* -------------------------------------------------------------
 * File contains EL2 versions of EL3 funcs in:
 * 	.../lib/extensions/pauth/pauth_helpers.S
 * -------------------------------------------------------------
 */

/* -------------------------------------------------------------
 * Program APIAKey_EL1 and enable pointer authentication in EL2
 * -------------------------------------------------------------
 */
func pauth_init_enable_el2
	stp	x29, x30, [sp, #-16]!

	/* Initialize platform key */
	bl	plat_init_apkey

	/* Program instruction key A used by the Trusted Firmware */
	msr	APIAKeyLo_EL1, x0
	msr	APIAKeyHi_EL1, x1

	/* Enable pointer authentication */
	mrs	x0, sctlr_el2
	orr	x0, x0, #SCTLR_EnIA_BIT

#if ENABLE_BTI
	 /* Enable PAC branch type compatibility */
	bic	x0, x0, #SCTLR_BT_BIT
#endif
	msr	sctlr_el2, x0
	isb

	ldp	x29, x30, [sp], #16
	ret
endfunc pauth_init_enable_el2

/* -------------------------------------------------------------
 * Disable pointer authentication in EL2
 * -------------------------------------------------------------
 */
func pauth_disable_el2
	mrs	x0, sctlr_el2
	bic	x0, x0, #SCTLR_EnIA_BIT
	msr	sctlr_el2, x0
	isb
	ret
endfunc pauth_disable_el2
