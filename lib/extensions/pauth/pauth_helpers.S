/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <lib/el3_runtime/cpu_data.h>

	.globl	pauth_load_bl31_apiakey
	.globl	pauth_load_bl1_apiakey_enable

/* -------------------------------------------------------------
 * The following functions strictly follow the AArch64 PCS
 * to use x9-x17 (temporary caller-saved registers) to load
 * the APIAKey_EL1 and enable pointer authentication.
 * -------------------------------------------------------------
 */
func pauth_load_bl31_apiakey
	/* tpidr_el3 contains the address of cpu_data structure */
	mrs	x9, tpidr_el3

	/* Load apiakey from cpu_data */
	ldp	x10, x11, [x9, #CPU_DATA_APIAKEY_OFFSET]

	/* Program instruction key A */
	msr	APIAKeyLo_EL1, x10
	msr	APIAKeyHi_EL1, x11
	isb
	ret
endfunc pauth_load_bl31_apiakey

func pauth_load_bl1_apiakey_enable
	/* Load instruction key A used by the Trusted Firmware */
	adrp	x9, bl1_apiakey
	add	x9, x9, :lo12:bl1_apiakey
	ldp	x10, x11, [x9]

	/* Program instruction key A */
	msr	APIAKeyLo_EL1, x10
	msr	APIAKeyHi_EL1, x11

	/* Enable pointer authentication */
	mrs	x9, sctlr_el3
	orr	x9, x9, #SCTLR_EnIA_BIT
	msr	sctlr_el3, x9
	isb
	ret
endfunc pauth_load_bl1_apiakey_enable
