/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.global	pauth_load_bl_apiakey

/* -----------------------------------------------------
 * The following function strictly follows the AArch64
 * PCS to use x9-x17 (temporary caller-saved registers)
 * to load the APIA key used by the firmware.
 * -----------------------------------------------------
 */
func pauth_load_bl_apiakey
	/* Load instruction key A used by the Trusted Firmware. */
	adrp	x11, plat_apiakey
	add	x11, x11, :lo12:plat_apiakey
	ldp	x9, x10, [x11]

	msr	APIAKeyLo_EL1, x9
	msr	APIAKeyHi_EL1, x10
	isb

	ret
endfunc pauth_load_bl_apiakey

