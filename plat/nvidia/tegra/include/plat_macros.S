/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PLAT_MACROS_S__
#define __PLAT_MACROS_S__

#include <gic_v2.h>
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
.macro plat_print_gic_regs
	mov_imm	x16, TEGRA_GICC_BASE
	cbz	x16, 1f
	/* gicc base address is now in x16 */
	adr	x6, gicc_regs	/* Load the gicc reg list to x6 */
	/* Load the gicc regs to gp regs used by str_in_crash_buf_print */
	ldr	w8, [x16, #GICC_HPPIR]
	ldr	w9, [x16, #GICC_AHPPIR]
	ldr	w10, [x16, #GICC_CTLR]
	/* Store to the crash buf and print to cosole */
	bl	str_in_crash_buf_print

	/* Print the GICD_ISPENDR regs */
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
	ldr	x4, [x7], #8
	bl	asm_print_hex
	adr	x4, newline
	bl	asm_print_str
	b	2b
1:
.endm

/* ------------------------------------------------
 * The below required platform porting macro prints
 * out relevant interconnect registers whenever an
 * unhandled exception is taken in BL3-1.
  * ------------------------------------------------
 */
.macro plat_print_interconnect_regs
	nop
.endm

#endif /* __PLAT_MACROS_S__ */
