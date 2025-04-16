/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
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
	.globl	report_el3_panic
	.globl	report_elx_panic

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

non_el3_sys_regs_1:
	.asciz	"spsr_el1", "elr_el1", "spsr_abt", "spsr_und",\
		"spsr_irq", "spsr_fiq", "sctlr_el1", "actlr_el1", "cpacr_el1",\
		"csselr_el1", "sp_el1", "esr_el1", ""

ttbr_regs:
	.asciz	"ttbr0_el1", "ttbr0_el2", "ttbr1_el1", "vttbr_el2", ""

non_el3_sys_regs_2:
	.asciz	"mair_el1", "amair_el1", "tcr_el1", "tpidr_el1",\
		"tpidr_el0", "tpidrro_el0", ""

par_reg:
	.asciz	"par_el1", ""

non_el3_sys_regs_3:
	.asciz	"mpidr_el1", "afsr0_el1", "afsr1_el1", "contextidr_el1",\
		"vbar_el1", "cntp_ctl_el0", "cntp_cval_el0", "cntv_ctl_el0",\
		"cntv_cval_el0", "cntkctl_el1", "sp_el0", "isr_el1", ""

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
excpt_msg_el:
	.asciz "Unhandled Exception from lower EL.\n"

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
	 * This function allows to report a crash from the lower
	 * exception level (if crash reporting is enabled) when
	 * lower_el_panic() is invoked from C Runtime.
	 * It prints the CPU state via the crash console making
	 * use of 'cpu_context' structure where general purpose
	 * registers are saved and the crash buf.
	 * This function will not return.
	 * -----------------------------------------------------
	 */
func report_elx_panic
	msr	spsel, #MODE_SP_ELX

	/* Print the crash message */
	adr	x4, excpt_msg_el
	bl	asm_print_str

	/* Report x0 - x29 values stored in 'gpregs_ctx' structure */
	/* Store the ascii list pointer in x6 */
	adr	x6, gp_regs
	add	x7, sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0

print_next:
	ldrb	w4, [x6]
	/* Test whether we are at end of list */
	cbz	w4, print_x30
	mov	x4, x6
	/* asm_print_str updates x4 to point to next entry in list */
	bl	asm_print_str
	/* x0 = number of symbols printed + 1 */
	sub	x0, x4, x6
	/* Update x6 with the updated list pointer */
	mov	x6, x4
	bl	print_alignment
	ldr	x4, [x7], #REGSZ
	bl	asm_print_hex
	bl	asm_print_newline
	b	print_next

print_x30:
	adr	x4, x30_msg
	bl	asm_print_str

	/* Print spaces to align "x30" string */
	mov	x0, #4
	bl	print_alignment

	/* Report x30 */
	ldr	x4, [x7]

	/* ----------------------------------------------------------------
	 * Different virtual address space size can be defined for each EL.
	 * Ensure that we use the proper one by reading the corresponding
	 * TCR_ELx register.
	 * ----------------------------------------------------------------
	 */
	cmp	x8, #MODE_EL2
	b.lt	from_el1	/* EL1 */
	mrs	x2, sctlr_el2
	mrs	x1, tcr_el2

	/* ----------------------------------------------------------------
	 * Check if pointer authentication is enabled at the specified EL.
	 * If it isn't, we can then skip stripping a PAC code.
	 * ----------------------------------------------------------------
	 */
test_pauth:
	tst	x2, #(SCTLR_EnIA_BIT | SCTLR_EnIB_BIT)
	b.eq	no_pauth

	/* Demangle address */
	and	x1, x1, #0x3F	/* T0SZ = TCR_ELx[5:0] */
	sub	x1, x1, #64
	neg	x1, x1		/* bottom_pac_bit = 64 - T0SZ */
	mov	x2, #-1
	lsl	x2, x2, x1
	bic	x4, x4, x2

no_pauth:
	bl	asm_print_hex
	bl	asm_print_newline

	/* tpidr_el3 contains the address to cpu_data structure */
	mrs	x0, tpidr_el3
	/* Calculate the Crash buffer offset in cpu_data */
	add	x0, x0, #CPU_DATA_CRASH_BUF_OFFSET
	/* Store crash buffer address in tpidr_el3 */
	msr	tpidr_el3, x0

	/* Print the rest of crash dump */
	b	print_el3_sys_regs

from_el1:
	mrs	x2, sctlr_el1
	mrs	x1, tcr_el1
	b	test_pauth
endfunc	report_elx_panic

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
	prepare_crash_buf_save_x0_x1
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

	/* Print the el3 sys registers */
print_el3_sys_regs:
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
	adr	x6, non_el3_sys_regs_1
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
	bl	str_in_crash_buf_print

	adr	x6, ttbr_regs
#if ENABLE_FEAT_D128
	is_feat_sysreg128_present_asm	x19
	/* Fallback to 64-bit if FEAT_SYSREG128 is disabled */
	cbz	x19, ttbr_regs_64_bit
	bl	read_ttbr0_el1
	mov	x8, x1
	mov	x9, x0
	bl	read_ttbr0_el2
	mov	x10, x1
	mov	x11, x0
	bl	read_ttbr1_el1
	mov	x12, x1
	mov	x13, x0
	bl	read_vttbr_el2
	mov	x14, x1
	mov	x15, x0
	bl	str_in_crash_buf_print_128
	b	1f

ttbr_regs_64_bit:
#endif
	mrs	x8, ttbr0_el1
	mrs	x9, ttbr0_el2
	mrs	x10, ttbr1_el1
	mrs	x11, vttbr_el2
	bl	str_in_crash_buf_print
1:
	adr	x6, non_el3_sys_regs_2
	mrs	x8, mair_el1
	mrs	x9, amair_el1
	mrs	x10, tcr_el1
	mrs	x11, tpidr_el1
	mrs	x12, tpidr_el0
	mrs	x13, tpidrro_el0
	bl	str_in_crash_buf_print

	adr	x6, par_reg
#if ENABLE_FEAT_D128
	/* Fallback to 64-bit if FEAT_SYSREG128 is disabled */
	cbz	x19, par_reg_64_bit
	bl	read_par_el1
	mov	x8, x1
	mov	x9, x0
	bl	str_in_crash_buf_print_128
	b	2f

par_reg_64_bit:
#endif
	mrs	x8, par_el1
	bl	str_in_crash_buf_print
2:
	adr	x6, non_el3_sys_regs_3
	mrs	x8, mpidr_el1
	mrs	x9, afsr0_el1
	mrs	x10, afsr1_el1
	mrs	x11, contextidr_el1
	mrs	x12, vbar_el1
	mrs	x13, cntp_ctl_el0
	mrs	x14, cntp_cval_el0
	mrs	x15, cntv_ctl_el0
	bl	str_in_crash_buf_print
	mrs	x8, cntv_cval_el0
	mrs	x9, cntkctl_el1
	mrs	x10, sp_el0
	mrs	x11, isr_el1
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
