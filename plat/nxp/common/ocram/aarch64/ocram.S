/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

#include <soc_default_base_addr.h>
#include <soc_default_helper_macros.h>

.global ocram_init

/*
 * void ocram_init(uintptr_t start_addr, size_t size)
 *
 * This function will do OCRAM ECC.
 * OCRAM is initialized with 64-bit writes and then a write
 * performed to address 0x0010_0534 with the value 0x0000_0008.
 *
 * x0: start_addr
 * x1: size in bytes
 * Called from C
 */

func ocram_init
	/* save the aarch32/64 non-volatile registers */
	stp	x4,  x5,  [sp, #-16]!
	stp	x6,  x7,  [sp, #-16]!
	stp	x8,  x9,  [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x18, x30, [sp, #-16]!

	/* convert bytes to 64-byte chunks */
	lsr	x1, x1, #6
1:
	/* for each location, read and write-back */
	dc	ivac, x0
	dsb	sy
	ldp	x4, x5, [x0]
	ldp	x6, x7, [x0, #16]
	ldp	x8, x9, [x0, #32]
	ldp	x10, x11, [x0, #48]
	stp	x4, x5, [x0]
	stp	x6, x7, [x0, #16]
	stp	x8, x9, [x0, #32]
	stp	x10, x11, [x0, #48]
	dc	cvac, x0

	sub	x1, x1, #1
	cbz	x1, 2f
	add	x0, x0, #64
	b	1b
2:
	/* Clear OCRAM ECC status bit in SBEESR2 and MBEESR2 */
	ldr	w1, =OCRAM_EESR_MASK
	ldr	x0, =DCFG_SBEESR2_ADDR
	str	w1, [x0]
	ldr	x0, =DCFG_MBEESR2_ADDR
	str	w1, [x0]

	/* restore the aarch32/64 non-volatile registers */
	ldp	x18, x30, [sp], #16
	ldp	x12, x13, [sp], #16
	ldp	x10, x11, [sp], #16
	ldp	x8,  x9,  [sp], #16
	ldp	x6,  x7,  [sp], #16
	ldp	x4,  x5,  [sp], #16
	ret
endfunc ocram_init
