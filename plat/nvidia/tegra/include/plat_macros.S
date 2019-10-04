/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015-2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

#include <drivers/arm/gicv2.h>
#include <tegra_def.h>

.section .rodata.gic_reg_name, "aS"
gicc_regs:
	.asciz "gicc_hppir", "gicc_ahppir", "gicc_ctlr", ""
gicd_pend_reg:
	.asciz "gicd_ispendr regs (Offsets 0x200 - 0x278)\n Offset:\t\t\tvalue\n"
newline:
	.asciz "\n"
spacer:
	.asciz ":\t\t0x"

/* ---------------------------------------------
 * The below macro prints out relevant GIC
 * registers whenever an unhandled exception is
 * taken in BL31.
 * ---------------------------------------------
 */
.macro plat_crash_print_regs
#ifdef TEGRA_GICC_BASE
	mov_imm	x16, TEGRA_GICC_BASE

	/* gicc base address is now in x16 */
	adr	x6, gicc_regs	/* Load the gicc reg list to x6 */
	/* Load the gicc regs to gp regs used by str_in_crash_buf_print */
	ldr	w8, [x16, #GICC_HPPIR]
	ldr	w9, [x16, #GICC_AHPPIR]
	ldr	w10, [x16, #GICC_CTLR]
	/* Store to the crash buf and print to cosole */
	bl	str_in_crash_buf_print
#endif
	/* Print the GICD_ISPENDR regs */
	mov_imm	x16, TEGRA_GICD_BASE
	add	x7, x16, #GICD_ISPENDR
	adr	x4, gicd_pend_reg
	bl	asm_print_str
2:
	sub	x4, x7, x16
	cmp	x4, #0x280
	b.eq	1f
	bl	asm_print_hex
	adr	x4, spacer
	bl	asm_print_str
	ldr	w4, [x7], #4
	bl	asm_print_hex
	adr	x4, newline
	bl	asm_print_str
	b	2b
1:
.endm

#endif /* PLAT_MACROS_S */
