/*
 * Copyright (c) 2015-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

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
 * The below required platform porting macro
 * prints out relevant registers whenever an
 * unhandled exception is taken in BL31.
 * ---------------------------------------------
 */
.macro plat_crash_print_regs
	nop
.endm

/* ---------------------------------------------
 * The below macro prints out relevant GIC
 * registers whenever an unhandled exception is
 * taken in BL31.
 * ---------------------------------------------
 */
.macro plat_print_gic_regs
	nop
	/*TBD-STINGRAY*/
.endm

/* ------------------------------------------------
 * The below required platform porting macro prints
 * out relevant interconnect registers whenever an
 * unhandled exception is taken in BL3-1.
  * ------------------------------------------------
 */
.macro plat_print_interconnect_regs
	nop
	/*TBD-STINGRAY*/
.endm

#endif /* PLAT_MACROS_S */
