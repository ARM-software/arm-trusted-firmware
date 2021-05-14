/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <asm_macros.S>
#include <platform_def.h>

/*
 * Below address in used only for reading, therefore no problem with concurrent
 * Linux access.
 */
#define MVEBU_TEST_PIN_LATCH_N (MVEBU_NB_GPIO_REG_BASE + 0x8)
 #define MVEBU_XTAL_MODE_MASK		BIT(9)

	/* -----------------------------------------------------
	 * uint32_t get_ref_clk (void);
	 *
	 * returns reference clock in MHz (25 or 40)
	 * -----------------------------------------------------
	 */
.globl	get_ref_clk
func get_ref_clk
	mov_imm	x0, MVEBU_TEST_PIN_LATCH_N
	ldr	w0, [x0]
	tst	w0, #MVEBU_XTAL_MODE_MASK
	bne	40
	mov	w0, #25
	ret
40:
	mov	w0, #40
	ret
endfunc get_ref_clk
