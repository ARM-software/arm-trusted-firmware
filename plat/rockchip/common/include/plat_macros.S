/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ROCKCHIP_PLAT_MACROS_S__
#define __ROCKCHIP_PLAT_MACROS_S__

#include <cci.h>
#include <gic_common.h>
#include <gicv2.h>
#include <gicv3.h>
#include <platform_def.h>

.section .rodata.gic_reg_name, "aS"
/* Applicable only to GICv2 and GICv3 with SRE disabled (legacy mode) */
gicc_regs:
	.asciz "gicc_hppir", "gicc_ahppir", "gicc_ctlr", ""

/* Applicable only to GICv3 with SRE enabled */
icc_regs:
	.asciz "icc_hppir0_el1", "icc_hppir1_el1", "icc_ctlr_el3", ""

/* Registers common to both GICv2 and GICv3 */
gicd_pend_reg:
	.asciz "gicd_ispendr regs (Offsets 0x200 - 0x278)\n"	\
		" Offset:\t\t\tvalue\n"
newline:
	.asciz "\n"
spacer:
	.asciz ":\t\t0x"

.section .rodata.cci_reg_name, "aS"
cci_iface_regs:
	.asciz "cci_snoop_ctrl_cluster0", "cci_snoop_ctrl_cluster1" , ""

	/* ---------------------------------------------
	 * The below utility macro prints out relevant GIC
	 * and CCI registers whenever an unhandled
	 * exception is taken in BL31.
	 * Expects: GICD base in x16, GICC base in x17
	 * Clobbers: x0 - x10, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs

	mov_imm	x16, PLAT_RK_GICD_BASE
	mov_imm	x17, PLAT_RK_GICC_BASE

	/* Check for GICv3 system register access */
	mrs	x7, id_aa64pfr0_el1
	ubfx	x7, x7, #ID_AA64PFR0_GIC_SHIFT, #ID_AA64PFR0_GIC_WIDTH
	cmp	x7, #1
	b.ne	print_gicv2

	/* Check for SRE enable */
	mrs	x8, ICC_SRE_EL3
	tst	x8, #ICC_SRE_SRE_BIT
	b.eq	print_gicv2

	/* Load the icc reg list to x6 */
	adr	x6, icc_regs
	/* Load the icc regs to gp regs used by str_in_crash_buf_print */
	mrs	x8, ICC_HPPIR0_EL1
	mrs	x9, ICC_HPPIR1_EL1
	mrs	x10, ICC_CTLR_EL3
	/* Store to the crash buf and print to console */
	bl	str_in_crash_buf_print
	b	print_gic_common

print_gicv2:
	/* Load the gicc reg list to x6 */
	adr	x6, gicc_regs
	/* Load the gicc regs to gp regs used by str_in_crash_buf_print */
	ldr	w8, [x17, #GICC_HPPIR]
	ldr	w9, [x17, #GICC_AHPPIR]
	ldr	w10, [x17, #GICC_CTLR]
	/* Store to the crash buf and print to console */
	bl	str_in_crash_buf_print

print_gic_common:
	/* Print the GICD_ISPENDR regs */
	add	x7, x16, #GICD_ISPENDR
	adr	x4, gicd_pend_reg
	bl	asm_print_str
gicd_ispendr_loop:
	sub	x4, x7, x16
	cmp	x4, #0x280
	b.eq	exit_print_gic_regs
	bl	asm_print_hex

	adr	x4, spacer
	bl	asm_print_str

	ldr	x4, [x7], #8
	bl	asm_print_hex

	adr	x4, newline
	bl	asm_print_str
	b	gicd_ispendr_loop
exit_print_gic_regs:

#if PLATFORM_CLUSTER_COUNT > 1
	adr	x6, cci_iface_regs
	/* Store in x7 the base address of the first interface */
	mov_imm	x7, (PLAT_RK_CCI_BASE + SLAVE_IFACE_OFFSET(	\
			PLAT_RK_CCI_CLUSTER0_SL_IFACE_IX))
	ldr	w8, [x7, #SNOOP_CTRL_REG]
	/* Store in x7 the base address of the second interface */
	mov_imm	x7, (PLAT_RK_CCI_BASE + SLAVE_IFACE_OFFSET(	\
			PLAT_RK_CCI_CLUSTER1_SL_IFACE_IX))
	ldr	w9, [x7, #SNOOP_CTRL_REG]
	/* Store to the crash buf and print to console */
	bl	str_in_crash_buf_print
#endif
	.endm

#endif /* __ROCKCHIP_PLAT_MACROS_S__ */
