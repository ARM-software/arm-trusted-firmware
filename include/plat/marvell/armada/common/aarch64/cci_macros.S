/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef CCI_MACROS_S
#define CCI_MACROS_S

#include <drivers/arm/cci.h>
#include <platform_def.h>

.section .rodata.cci_reg_name, "aS"
cci_iface_regs:
	.asciz "cci_snoop_ctrl_cluster0", "cci_snoop_ctrl_cluster1" , ""

	/* ------------------------------------------------
	 * The below required platform porting macro prints
	 * out relevant interconnect registers whenever an
	 * unhandled exception is taken in BL31.
	 * Clobbers: x0 - x9, sp
	 * ------------------------------------------------
	 */
	.macro print_cci_regs
	adr	x6, cci_iface_regs
	/* Store in x7 the base address of the first interface */
	mov_imm	x7, (PLAT_MARVELL_CCI_BASE + SLAVE_IFACE_OFFSET(	\
			PLAT_MARVELL_CCI_CLUSTER0_SL_IFACE_IX))
	ldr	w8, [x7, #SNOOP_CTRL_REG]
	/* Store in x7 the base address of the second interface */
	mov_imm	x7, (PLAT_MARVELL_CCI_BASE + SLAVE_IFACE_OFFSET(	\
			PLAT_MARVELL_CCI_CLUSTER1_SL_IFACE_IX))
	ldr	w9, [x7, #SNOOP_CTRL_REG]
	/* Store to the crash buf and print to console */
	bl	str_in_crash_buf_print
	.endm

#endif /* CCI_MACROS_S */
