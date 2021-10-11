/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/debug.h>

	.globl el2_panic

	/***********************************************************
	 * The common implementation of do_panic for all BL stages
	 ***********************************************************/

.section .rodata.panic_str, "aS"
	panic_msg: .asciz "PANIC at PC : 0x"

/*
 * el2_panic will be redefined by the
 * crash reporting mechanism (if enabled)
 */
el2_panic:
	mov	x6, x30
	bl	plat_crash_console_init

	/* Check if the console is initialized */
	cbz	x0, _panic_handler

	/* The console is initialized */
	adr	x4, panic_msg
	bl	asm_print_str
	mov	x4, x6

	/* The panic location is lr -4 */
	sub	x4, x4, #4
	bl	asm_print_hex

	bl	plat_crash_console_flush

_panic_handler:
	/* Pass to plat_panic_handler the address from where el2_panic was
	 * called, not the address of the call from el2_panic.
	 */
	mov	x30, x6
	b	plat_panic_handler
