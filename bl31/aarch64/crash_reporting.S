/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_macros.S>
#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/utils_def.h>

	.globl	report_unhandled_exception
	.globl	report_unhandled_interrupt
	.globl	el3_panic

#if CRASH_REPORTING

	/* ------------------------------------------------------
	 * The below section deals with dumping the system state
	 * when an unhandled exception is taken in EL3.
	 * The layout and the names of the registers which will
	 * be dumped during a unhandled exception is given below.
	 * ------------------------------------------------------
	 */
.section .rodata.crash_prints, "aS"
print_spacer:
	.asciz	"             = 0x"

gp_regs:
	.asciz	"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",\
		"x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",\
		"x16", "x17", "x18", "x19", "x20", "x21", "x22",\
		"x23", "x24", "x25", "x26", "x27", "x28", "x29", ""
el3_sys_regs:
	.asciz	"scr_el3", "sctlr_el3", "cptr_el3", "tcr_el3",\
		"daif", "mair_el3", "spsr_el3", "elr_el3", "ttbr0_el3",\
		"esr_el3", "far_el3", ""

non_el3_sys_regs:
	.asciz	"spsr_el1", "elr_el1", "spsr_abt", "spsr_und",\
		"spsr_irq", "spsr_fiq", "sctlr_el1", "actlr_el1", "cpacr_el1",\
		"csselr_el1", "sp_el1", "esr_el1", "ttbr0_el1", "ttbr1_el1",\
		"mair_el1", "amair_el1", "tcr_el1", "tpidr_el1", "tpidr_el0",\
		"tpidrro_el0",  "par_el1", "mpidr_el1", "afsr0_el1", "afsr1_el1",\
		"contextidr_el1", "vbar_el1", "cntp_ctl_el0", "cntp_cval_el0",\
		"cntv_ctl_el0", "cntv_cval_el0", "cntkctl_el1", "sp_el0", "isr_el1", ""

#if CTX_INCLUDE_AARCH32_REGS
aarch32_regs:
	.asciz	"dacr32_el2", "ifsr32_el2", ""
#endif /* CTX_INCLUDE_AARCH32_REGS */

panic_msg:
	.asciz "PANIC in EL3.\nx30"
excpt_msg:
	.asciz "Unhandled Exception in EL3.\nx30"
intr_excpt_msg:
	.asciz "Unhandled Interrupt Exception in EL3.\nx30"

	/*
	 * Helper function to print from crash buf.
	 * The print loop is controlled by the buf size and
	 * ascii reg name list which is passed in x6. The
	 * function returns the crash buf address in x0.
	 * Clobbers : x0 - x7, sp
	 */
func size_controlled_print
	/* Save the lr */
	mov	sp, x30
	/* load the crash buf address */
	mrs	x7, tpidr_el3
test_size_list:
	/* Calculate x5 always as it will be clobbered by asm_print_hex */
	mrs	x5, tpidr_el3
	add	x5, x5, #CPU_DATA_CRASH_BUF_SIZE
	/* Test whether we have reached end of crash buf */
	cmp	x7, x5
	b.eq	exit_size_print
	ldrb	w4, [x6]
	/* Test whether we are at end of list */
	cbz	w4, exit_size_print
	mov	x4, x6
	/* asm_print_str updates x4 to point to next entry in list */
	bl	asm_print_str
	/* x0 = number of symbols printed + 1 */
	sub	x0, x4, x6
	/* update x6 with the updated list pointer */
	mov	x6, x4
	bl	print_alignment
	ldr	x4, [x7], #REGSZ
	bl	asm_print_hex
	bl	asm_print_newline
	b	test_size_list
exit_size_print:
	mov	x30, sp
	ret
endfunc size_controlled_print

	/* -----------------------------------------------------
	 * This function calculates and prints required number
	 * of space characters followed by "= 0x", based on the
	 * length of ascii register name.
 	 * x0: length of ascii register name + 1
	 * ------------------------------------------------------
 	 */
func print_alignment
	/* The minimum ascii length is 3, e.g. for "x0" */
	adr	x4, print_spacer - 3
	add	x4, x4, x0
	b	asm_print_str
endfunc print_alignment

	/*
	 * Helper function to store x8 - x15 registers to
	 * the crash buf. The system registers values are
	 * copied to x8 to x15 by the caller which are then
	 * copied to the crash buf by this function.
	 * x0 points to the crash buf. It then calls
	 * size_controlled_print to print to console.
	 * Clobbers : x0 - x7, sp
	 */
func str_in_crash_buf_print
	/* restore the crash buf address in x0 */
	mrs	x0, tpidr_el3
	stp	x8, x9, [x0]
	stp	x10, x11, [x0, #REGSZ * 2]
	stp	x12, x13, [x0, #REGSZ * 4]
	stp	x14, x15, [x0, #REGSZ * 6]
	b	size_controlled_print
endfunc str_in_crash_buf_print

	/* ------------------------------------------------------
	 * This macro calculates the offset to crash buf from
	 * cpu_data and stores it in tpidr_el3. It also saves x0
	 * and x1 in the crash buf by using sp as a temporary
	 * register.
	 * ------------------------------------------------------
	 */
	.macro prepare_crash_buf_save_x0_x1
	/* we can corrupt this reg to free up x0 */
	mov	sp, x0
	/* tpidr_el3 contains the address to cpu_data structure */
	mrs	x0, tpidr_el3
	/* Calculate the Crash buffer offset in cpu_data */
	add	x0, x0, #CPU_DATA_CRASH_BUF_OFFSET
	/* Store crash buffer address in tpidr_el3 */
	msr	tpidr_el3, x0
	str	x1, [x0, #REGSZ]
	mov	x1, sp
	str	x1, [x0]
	.endm

	/* -----------------------------------------------------
	 * This function allows to report a crash (if crash
	 * reporting is enabled) when an unhandled exception
	 * occurs. It prints the CPU state via the crash console
	 * making use of the crash buf. This function will
	 * not return.
	 * -----------------------------------------------------
	 */
func report_unhandled_exception
	prepare_crash_buf_save_x0_x1
	adr	x0, excpt_msg
	mov	sp, x0
	/* This call will not return */
	b	do_crash_reporting
endfunc report_unhandled_exception


	/* -----------------------------------------------------
	 * This function allows to report a crash (if crash
	 * reporting is enabled) when an unhandled interrupt
	 * occurs. It prints the CPU state via the crash console
	 * making use of the crash buf. This function will
	 * not return.
	 * -----------------------------------------------------
	 */
func report_unhandled_interrupt
	prepare_crash_buf_save_x0_x1
	adr	x0, intr_excpt_msg
	mov	sp, x0
	/* This call will not return */
	b	do_crash_reporting
endfunc report_unhandled_interrupt

	/* -----------------------------------------------------
	 * This function allows to report a crash (if crash
	 * reporting is enabled) when panic() is invoked from
	 * C Runtime. It prints the CPU state via the crash
	 * console making use of the crash buf. This function
	 * will not return.
	 * -----------------------------------------------------
	 */
func el3_panic
	msr	spsel, #MODE_SP_ELX
	prepare_crash_buf_save_x0_x1
	adr	x0, panic_msg
	mov	sp, x0
	/* This call will not return */
	b	do_crash_reporting
endfunc el3_panic

	/* ------------------------------------------------------------
	 * The common crash reporting functionality. It requires x0
	 * and x1 has already been stored in crash buf, sp points to
	 * crash message and tpidr_el3 contains the crash buf address.
	 * The function does the following:
	 *   - Retrieve the crash buffer from tpidr_el3
	 *   - Store x2 to x6 in the crash buffer
	 *   - Initialise the crash console.
	 *   - Print the crash message by using the address in sp.
	 *   - Print x30 value to the crash console.
	 *   - Print x0 - x7 from the crash buf to the crash console.
	 *   - Print x8 - x29 (in groups of 8 registers) using the
	 *     crash buf to the crash console.
	 *   - Print el3 sys regs (in groups of 8 registers) using the
	 *     crash buf to the crash console.
	 *   - Print non el3 sys regs (in groups of 8 registers) using
	 *     the crash buf to the crash console.
	 * ------------------------------------------------------------
	 */
func do_crash_reporting
	/* Retrieve the crash buf from tpidr_el3 */
	mrs	x0, tpidr_el3
	/* Store x2 - x6, x30 in the crash buffer */
	stp	x2, x3, [x0, #REGSZ * 2]
	stp	x4, x5, [x0, #REGSZ * 4]
	stp	x6, x30, [x0, #REGSZ * 6]
	/* Initialize the crash console */
	bl	plat_crash_console_init
	/* Verify the console is initialized */
	cbz	x0, crash_panic
	/* Print the crash message. sp points to the crash message */
	mov	x4, sp
	bl	asm_print_str
	/* Print spaces to align "x30" string */
	mov	x0, #4
	bl	print_alignment
	/* load the crash buf address */
	mrs	x0, tpidr_el3
	/* report x30 first from the crash buf */
	ldr	x4, [x0, #REGSZ * 7]

#if ENABLE_PAUTH
	/* Demangle address */
	xpaci	x4
#endif
	bl	asm_print_hex
	bl	asm_print_newline
	/* Load the crash buf address */
	mrs	x0, tpidr_el3
	/* Now mov x7 into crash buf */
	str	x7, [x0, #REGSZ * 7]

	/* Report x0 - x29 values stored in crash buf*/
	/* Store the ascii list pointer in x6 */
	adr	x6, gp_regs
	/* Print x0 to x7 from the crash buf */
	bl	size_controlled_print
	/* Store x8 - x15 in crash buf and print */
	bl	str_in_crash_buf_print
	/* Load the crash buf address */
	mrs	x0, tpidr_el3
	/* Store the rest of gp regs and print */
	stp	x16, x17, [x0]
	stp	x18, x19, [x0, #REGSZ * 2]
	stp	x20, x21, [x0, #REGSZ * 4]
	stp	x22, x23, [x0, #REGSZ * 6]
	bl	size_controlled_print
	/* Load the crash buf address */
	mrs	x0, tpidr_el3
	stp	x24, x25, [x0]
	stp	x26, x27, [x0, #REGSZ * 2]
	stp	x28, x29, [x0, #REGSZ * 4]
	bl	size_controlled_print

	/* Print the el3 sys registers */
	adr	x6, el3_sys_regs
	mrs	x8, scr_el3
	mrs	x9, sctlr_el3
	mrs	x10, cptr_el3
	mrs	x11, tcr_el3
	mrs	x12, daif
	mrs	x13, mair_el3
	mrs	x14, spsr_el3
	mrs	x15, elr_el3
	bl	str_in_crash_buf_print
	mrs	x8, ttbr0_el3
	mrs	x9, esr_el3
	mrs	x10, far_el3
	bl	str_in_crash_buf_print

	/* Print the non el3 sys registers */
	adr	x6, non_el3_sys_regs
	mrs	x8, spsr_el1
	mrs	x9, elr_el1
	mrs	x10, spsr_abt
	mrs	x11, spsr_und
	mrs	x12, spsr_irq
	mrs	x13, spsr_fiq
	mrs	x14, sctlr_el1
	mrs	x15, actlr_el1
	bl	str_in_crash_buf_print
	mrs	x8, cpacr_el1
	mrs	x9, csselr_el1
	mrs	x10, sp_el1
	mrs	x11, esr_el1
	mrs	x12, ttbr0_el1
	mrs	x13, ttbr1_el1
	mrs	x14, mair_el1
	mrs	x15, amair_el1
	bl	str_in_crash_buf_print
	mrs	x8, tcr_el1
	mrs	x9, tpidr_el1
	mrs	x10, tpidr_el0
	mrs	x11, tpidrro_el0
	mrs	x12, par_el1
	mrs	x13, mpidr_el1
	mrs	x14, afsr0_el1
	mrs	x15, afsr1_el1
	bl	str_in_crash_buf_print
	mrs	x8, contextidr_el1
	mrs	x9, vbar_el1
	mrs	x10, cntp_ctl_el0
	mrs	x11, cntp_cval_el0
	mrs	x12, cntv_ctl_el0
	mrs	x13, cntv_cval_el0
	mrs	x14, cntkctl_el1
	mrs	x15, sp_el0
	bl	str_in_crash_buf_print
	mrs	x8, isr_el1
	bl	str_in_crash_buf_print

#if CTX_INCLUDE_AARCH32_REGS
	/* Print the AArch32 registers */
	adr	x6, aarch32_regs
	mrs	x8, dacr32_el2
	mrs	x9, ifsr32_el2
	bl	str_in_crash_buf_print
#endif /* CTX_INCLUDE_AARCH32_REGS */

	/* Get the cpu specific registers to report */
	bl	do_cpu_reg_dump
	bl	str_in_crash_buf_print

	/* Print some platform registers */
	plat_crash_print_regs

	bl	plat_crash_console_flush

	/* Done reporting */
	no_ret	plat_panic_handler
endfunc do_crash_reporting

#else	/* CRASH_REPORTING */
func report_unhandled_exception
report_unhandled_interrupt:
	no_ret	plat_panic_handler
endfunc report_unhandled_exception
#endif	/* CRASH_REPORTING */


func crash_panic
	no_ret	plat_panic_handler
endfunc crash_panic
