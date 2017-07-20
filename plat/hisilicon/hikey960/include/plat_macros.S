/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_MACROS_S__
#define __PLAT_MACROS_S__

#include <cci.h>
#include <gic_v2.h>
#include <hi3660.h>
#include <platform_def.h>

.section .rodata.gic_reg_name, "aS"
gicc_regs:
	.asciz "gicc_hppir", "gicc_ahppir", "gicc_ctlr", ""
gicd_pend_reg:
	.asciz "gicd_ispendr regs (Offsets 0x200 - 0x278)\n"    \
                " Offset:\t\t\tvalue\n"
newline:
	.asciz "\n"
spacer:
	.asciz ":\t\t0x"

.section .rodata.cci_reg_name, "aS"
cci_iface_regs:
	.asciz "cci_snoop_ctrl_cluster0", "cci_snoop_ctrl_cluster1" , ""

/* ---------------------------------------------
 * The below macro prints out relevant GIC
 * registers whenever an unhandled exception is
 * taken in BL31.
 * ---------------------------------------------
 */
.macro plat_crash_print_regs
	mov_imm	x16, GICD_REG_BASE
	mov_imm	x17, GICC_REG_BASE

	/* Load the gicc reg list to x6 */
	adr	x6, gicc_regs
	/* Load the gicc regs to gp regs used by str_in_crash_buf_print */
	ldr	w8, [x17, #GICC_HPPIR]
	ldr	w9, [x17, #GICC_AHPPIR]
	ldr	w10, [x17, #GICC_CTLR]
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
	adr	x6, cci_iface_regs
	/* Store in x7 the base address of the first interface */
	mov_imm	x7, (CCI400_REG_BASE + SLAVE_IFACE_OFFSET(	\
	                CCI400_SL_IFACE3_CLUSTER_IX))
	ldr	w8, [x7, #SNOOP_CTRL_REG]
	/* Store in x7 the base address of the second interface */
	mov_imm	x7, (CCI400_REG_BASE + SLAVE_IFACE_OFFSET(	\
	                CCI400_SL_IFACE4_CLUSTER_IX))
	ldr	w9, [x7, #SNOOP_CTRL_REG]
	/* Store to the crash buf and print to console */
	bl	str_in_crash_buf_print
.endm

#endif /* __PLAT_MACROS_S__ */
