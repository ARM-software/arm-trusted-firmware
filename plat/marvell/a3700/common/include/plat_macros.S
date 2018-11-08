/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

#include <marvell_macros.S>

/* ---------------------------------------------
 * The below macro prints out relevant GIC and
 * CCI registers registers whenever an unhandled
 * exception is taken in BL31.
 * ---------------------------------------------
 */
.macro plat_crash_print_regs
	mov_imm	x17, MVEBU_GICC_BASE
	mov_imm	x16, MVEBU_GICD_BASE
	marvell_print_gic_regs
	print_cci_regs
.endm

#endif /* PLAT_MACROS_S */
