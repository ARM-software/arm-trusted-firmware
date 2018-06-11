/** @file

  Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S
#include <arch.h>
#include <platform_def.h>

.section .rodata.gic_reg_name, "aS"
gic_regs: .asciz "icc_ctlr", "icc_hppir0", "icc_hppir1", ""

	/* ---------------------------------------------
	 * The below macro prints out relevant GIC
	 * registers whenever an unhandled exception is
	 * taken in BL31.
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	mrs	x8, ICC_CTLR_EL3
	mrs	x9, ICC_HPPIR0_EL1
	mrs	x10, ICC_HPPIR1_EL1
	adr	x6, gic_regs
	bl	str_in_crash_buf_print
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
#endif /* PLAT_MACROS_S */
