/*
 * Copyright (c) 2014-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_macros.S>
#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <lib/utils_def.h>

	.globl	report_unhandled_exception
	.globl	report_unhandled_interrupt
	.globl	report_el3_panic

#if CRASH_REPORTING
/* need enough space in crash buffer to save 8 registers */
#define CRASH_BUF_SIZE		(8 * CPU_WORD_SIZE)
#define CRASH_BUF_SPACE		(CRASH_BUF_SIZE * PLATFORM_CORE_COUNT)

.section .data.crash_buf
crash_buf:
	.align	3 /* log2 of CPU_WORD_SIZE */
	.space  CRASH_BUF_SPACE

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
	.asciz	"elr_el3", "esr_el3", "mpidr_el1", "scr_el3", "tcr_el3",\
		"daif", "mair_el3", "spsr_el3", "cptr_el3", "ttbr0_el3",\
		"sctlr_el3", "far_el3", "sp_el0", "isr_el1", ""

#if CTX_INCLUDE_AARCH32_REGS
aarch32_regs:
	.asciz	"dacr32_el2", "ifsr32_el2", ""
#endif /* CTX_INCLUDE_AARCH32_REGS */

panic_msg:
	.asciz "PANIC in EL3.\nx30"
excpt_msg:
	.asciz "Unhandled Exception in EL3.\nx30"
intr_excpt_msg:
	.ascii "Unhandled Interrupt Exception in EL3.\n"
x30_msg:
	.asciz "x30"

	/*
	 * Helper function to print from crash buf.
	 * The print loop is controlled by the buf size and
	 * ascii reg name list which is passed in x6. The
	 * function returns the crash buf address in x0.
	 * Clobbers : x0 - x7, x20, sp
	 */
func size_controlled_print_helper
#if ENABLE_FEAT_D128
size_controlled_print_128:
	/* Set flag to print 128-bit registers */
	mov	x20, #1
	b	1f

size_controlled_print:
	/* Set flag to print 64-bit registers */
	mov	x20, #0
1:
#else
size_controlled_print:
#endif
	/* Save the lr */
	mov	sp, x30
	/* load the crash buf address */
	mrs	x7, tpidr_el3
test_size_list:
	/* Calculate x5 always as it will be clobbered by asm_print_hex */
	mrs	x5, tpidr_el3
	add	x5, x5, #CRASH_BUF_SIZE
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
	/* Print the high 64 bits (or whole 64-bit register) */
	ldr	x4, [x7], #REGSZ
	bl	asm_print_hex
#if ENABLE_FEAT_D128
	cbz	x20, 2f
	/* Print the low 64 bits in case of a 128-bit register */
	ldr	x4, [x7], #REGSZ
	bl	asm_print_hex
2:
#endif
	bl	asm_print_newline
	b	test_size_list
exit_size_print:
	mov	x30, sp
	ret
endfunc size_controlled_print_helper

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
	 * Clobbers : x0 - x7, x20, sp
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

	/*
	 * An equivalent helper function for storing x8 - x15
	 * registers in a different order inside the crash buf.
	 * In the end the function size_controlled_print_128 is
	 * called to print the registers to the console.
	 * Clobbers : x0 - x7, x20, sp
	 */
func str_in_crash_buf_print_128
	/* restore the crash buf address in x0 */
	mrs	x0, tpidr_el3
	stp	x8, x9, [x0]
	stp	x10, x11, [x0, #REGSZ * 2]
	stp	x12, x13, [x0, #REGSZ * 4]
	stp	x14, x15, [x0, #REGSZ * 6]
	b	size_controlled_print_128
endfunc str_in_crash_buf_print_128

	/* ---------------------------------------------------------------------
	 * This macro calculates the offset to crash buf and stores it in
	 * tpidr_el3. It also saves x0 to x6 and x30 in the crash buf by using
	 * system registers as temporary registers. These are selected to be
	 * present in a minimal ARMv8.0 implementation and not overwrite context
	 * we want to report.
	 * ---------------------------------------------------------------------
	 */
	.macro prepare_crash_buf_and_save_regs
	msr	tpidr_el3,	x0

	/* PMUv3 is presumed to be always present. Disable counting so
	 * pmccntr_el0 can be used */
	mrs	x0, pmcr_el0
	bic	x0, x0, #PMCR_EL0_E_BIT
	msr	pmcr_el0, x0

	msr	far_el1,	x1
	msr	elr_el1,	x2
	msr	tpidr_el1,	x3
	msr	mair_el1,	x4
	msr	sp_el1,		x5
	msr	vbar_el1,	x6
	msr	tpidr_el0,	x7
	msr	tpidrro_el0,	x8
	msr	pmccntr_el0,	x30

	/* calculate crash_buf[core_pos] */
	bl	plat_my_core_pos
	mov_imm	x1, CRASH_BUF_SIZE
	mul	x0, x0, x1
	adr_l	x1, crash_buf
	add	x0, x0, x1

	/* put x0 in the crash buffer */
	mrs	x1, tpidr_el3
	str	x1, [x0]

	/* Store crash buffer address in tpidr_el3 */
	msr	tpidr_el3, x0

	/* put x1 - x7 in the crash buffer */
	mrs	x1, far_el1
	mrs	x2, elr_el1
	mrs	x3, tpidr_el1
	mrs	x4, mair_el1
	mrs	x5, sp_el1
	mrs	x6, vbar_el1
	mrs	x30, pmccntr_el0

	str	x1, [x0, #REGSZ]
	stp	x2, x3, [x0, #REGSZ * 2]
	stp	x4, x5, [x0, #REGSZ * 4]
	stp	x6, x30, [x0, #REGSZ * 6]

	/* put these back as there's no space in the buffer */
	mrs	x7, tpidr_el0
	mrs	x8, tpidrro_el0
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
	/* Switch to SP_ELx */
	msr     spsel, #MODE_SP_ELX
	prepare_crash_buf_and_save_regs
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
	prepare_crash_buf_and_save_regs
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
func report_el3_panic
	msr	spsel, #MODE_SP_ELX
	prepare_crash_buf_and_save_regs
	adr	x0, panic_msg
	mov	sp, x0
	/* Fall through to 'do_crash_reporting' */

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
	 *     the crash buf to the crash console. A group may be
	 *     interrupted in case a potential group of 128-bit
	 *     sys regs needs to be printed.
	 * ------------------------------------------------------------
	 */
do_crash_reporting:
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
	/* Load the crash buf address */
	mrs	x0, tpidr_el3
	/* Report x30 first from the crash buf */
	ldr	x4, [x0, #REGSZ * 7]

#if ENABLE_PAUTH
#if ENABLE_PAUTH == 2
	/* Skip if not present in hardware */
	is_feat_pauth_present_asm x0, x1
	beq	1f
#endif
	/*
	 * The assembler must see support for xpaci. So turn the compiler
	 * extension on. GCC prior to 10 doesn't understand the PAuth extension
	 * but it does understand armv8.3-a in general. Avoid using 8.3 if
	 * the compiler understands "pauth" so we don't downgrade a higher
	 * -march that was specified on the commandline.
	 */
#if __GNUC__ < 10
	.arch armv8.3-a
#else
	.arch_extension pauth
#endif
	/* Demangle address */
	xpaci	x4
1:
#endif
	bl	asm_print_hex
	bl	asm_print_newline
	/* Load the crash buf address */
	mrs	x0, tpidr_el3
	/* Now mov x7 into crash buf */
	str	x7, [x0, #REGSZ * 7]

	/* Report x0 - x29 values stored in crash buf */
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

	/* visually seperate GP and sys regs */
	bl	asm_print_newline

	/* Print the el3 sys registers */
print_el3_sys_regs:
	adr	x6, el3_sys_regs
	mrs	x8, elr_el3
	mrs	x9, esr_el3
	mrs	x10, mpidr_el1
	mrs	x11, scr_el3
	mrs	x12, daif
	mrs	x13, mair_el3
	mrs	x14, spsr_el3
	mrs	x15, cptr_el3
	bl	str_in_crash_buf_print
	mrs	x8, ttbr0_el3
	mrs	x9, sctlr_el3
	mrs	x10, far_el3
	mrs	x11, tcr_el3
	mrs	x12, sp_el0
	mrs	x13, isr_el1
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
endfunc report_el3_panic

#else	/* CRASH_REPORTING */
func report_unhandled_exception
report_unhandled_interrupt:
	no_ret	plat_panic_handler
endfunc report_unhandled_exception
#endif	/* CRASH_REPORTING */

func crash_panic
	no_ret	plat_panic_handler
endfunc crash_panic
