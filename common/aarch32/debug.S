/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	asm_assert
	.globl	do_panic
	.globl	report_exception

/* Since the max decimal input number is 65536 */
#define MAX_DEC_DIVISOR		10000

/* The offset to add to get ascii for numerals '0 - 9' */
#define ASCII_OFFSET_NUM	'0'

	.section .rodata.panic_str, "aS"
panic_msg:
	.asciz "PANIC at PC : 0x"
panic_end:
	.asciz "\r\n"

	/***********************************************************
	 * The common implementation of do_panic for all BL stages
	 ***********************************************************/
func do_panic
	/* Have LR copy point to PC at the time of panic */
	sub	r6, lr, #4

	/* Initialize crash console and verify success */
	bl	plat_crash_console_init
	cmp	r0, #0
	beq	1f

	/* Print panic message */
	ldr	r4, =panic_msg
	bl	asm_print_str

	/* Print LR in hex */
	mov	r4, r6
	bl	asm_print_hex

	/* Print new line */
	ldr	r4, =panic_end
	bl	asm_print_str

	bl	plat_crash_console_flush

1:
	mov	lr, r6
	b	plat_panic_handler
endfunc do_panic

	/***********************************************************
	 * This function is called from the vector table for
	 * unhandled exceptions. It reads the current mode and
	 * passes it to platform.
	 ***********************************************************/
func report_exception
	mrs	r0, cpsr
	and	r0, #MODE32_MASK
	bl	plat_report_exception
	no_ret	plat_panic_handler
endfunc report_exception

#if ENABLE_ASSERTIONS
.section .rodata.assert_str, "aS"
assert_msg1:
	.asciz "ASSERT: File "
assert_msg2:
#if ARM_ARCH_MAJOR == 7 && !defined(ARMV7_SUPPORTS_VIRTUALIZATION)
	/******************************************************************
	 * Virtualization comes with the UDIV/SDIV instructions. If missing
	 * write file line number in hexadecimal format.
	 ******************************************************************/
	.asciz " Line 0x"
#else
	.asciz " Line "
#endif

/* ---------------------------------------------------------------------------
 * Assertion support in assembly.
 * The below function helps to support assertions in assembly where we do not
 * have a C runtime stack. Arguments to the function are :
 * r0 - File name
 * r1 - Line no
 * Clobber list : lr, r0 - r6
 * ---------------------------------------------------------------------------
 */
func asm_assert
#if LOG_LEVEL >= LOG_LEVEL_INFO
	/*
	 * Only print the output if LOG_LEVEL is higher or equal to
	 * LOG_LEVEL_INFO, which is the default value for builds with DEBUG=1.
	 */
	/* Stash the parameters already in r0 and r1 */
	mov	r5, r0
	mov	r6, r1

	/* Initialize crash console and verify success */
	bl	plat_crash_console_init
	cmp	r0, #0
	beq	1f

	/* Print file name */
	ldr	r4, =assert_msg1
	bl	asm_print_str
	mov	r4, r5
	bl	asm_print_str

	/* Print line number string */
	ldr	r4, =assert_msg2
	bl	asm_print_str

	/* Test for maximum supported line number */
	ldr	r4, =~0xffff
	tst	r6, r4
	bne	1f
	mov	r4, r6

#if ARM_ARCH_MAJOR == 7 && !defined(ARMV7_SUPPORTS_VIRTUALIZATION)
	/******************************************************************
	 * Virtualization comes with the UDIV/SDIV instructions. If missing
	 * write file line number in hexadecimal format.
	 ******************************************************************/
	bl	asm_print_hex
#else
	/* Print line number in decimal */
	mov	r6, #10			/* Divide by 10 after every loop iteration */
	ldr	r5, =MAX_DEC_DIVISOR
dec_print_loop:
	udiv	r0, r4, r5			/* Quotient */
	mls	r4, r0, r5, r4			/* Remainder */
	add	r0, r0, #ASCII_OFFSET_NUM	/* Convert to ASCII */
	bl	plat_crash_console_putc
	udiv	r5, r5, r6			/* Reduce divisor */
	cmp	r5, #0
	bne	dec_print_loop
#endif

	bl	plat_crash_console_flush

1:
#endif /* LOG_LEVEL >= LOG_LEVEL_INFO */
	no_ret	plat_panic_handler
endfunc asm_assert
#endif /* ENABLE_ASSERTIONS */

/*
 * This function prints a string from address in r4
 * Clobber: lr, r0 - r4
 */
func asm_print_str
	mov	r3, lr
1:
	ldrb	r0, [r4], #0x1
	cmp	r0, #0
	beq	2f
	bl	plat_crash_console_putc
	b	1b
2:
	bx	r3
endfunc asm_print_str

/*
 * This function prints a hexadecimal number in r4.
 * In: r4 = the hexadecimal to print.
 * Clobber: lr, r0 - r3, r5
 */
func asm_print_hex
	mov	r3, lr
	mov	r5, #32  /* No of bits to convert to ascii */
1:
	sub	r5, r5, #4
	lsr	r0, r4, r5
	and	r0, r0, #0xf
	cmp	r0, #0xa
	blo	2f
	/* Add by 0x27 in addition to ASCII_OFFSET_NUM
	 * to get ascii for characters 'a - f'.
	 */
	add	r0, r0, #0x27
2:
	add	r0, r0, #ASCII_OFFSET_NUM
	bl	plat_crash_console_putc
	cmp	r5, #0
	bne	1b
	bx	r3
endfunc asm_print_hex
