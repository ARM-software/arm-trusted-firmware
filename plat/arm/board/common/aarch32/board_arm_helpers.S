/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <common/bl_common.h>
#include <platform_def.h>

	.globl  plat_report_exception


	/* -------------------------------------------------------
	 * void plat_report_exception(unsigned int type)
	 * Function to report an unhandled exception
	 * with platform-specific means.
	 * On FVP platform, it updates the LEDs
	 * to indicate where we are.
	 * SYS_LED[0]   - 0x0
	 * SYS_LED[2:1] - 0x0
	 * SYS_LED[7:3] - Exception Mode.
	 * Clobbers: r0-r1
	 * -------------------------------------------------------
	 */
func plat_report_exception
	lsl	r0, r0, #V2M_SYS_LED_EC_SHIFT
	ldr	r1, =V2M_SYSREGS_BASE
	add	r1, r1, #V2M_SYS_LED
	str	r0, [r1]
	bx	lr
endfunc plat_report_exception
