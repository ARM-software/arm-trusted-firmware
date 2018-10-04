/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <asm_macros.S>
#include <marvell_def.h>
#include <platform_def.h>

#define PTE_NON_EXEC_OFF	54	/* XN - eXecute Never bit offset - see VMSAv8-64 */

	.globl marvell_ble_prepare_exit

func marvell_ble_prepare_exit
	/*
	 * Read the page table base and set the first page to be executable.
	 * This is required for jumping to DRAM for further execution.
	 */
	mrs	x0, ttbr0_el3
	ldr     x1, [x0]
	mov	x2, #1
	bic	x1, x1, x2, lsl #PTE_NON_EXEC_OFF
	str	x1, [x0]
	tlbi	alle3
	dsb	sy
	isb
	ret
endfunc marvell_ble_prepare_exit
