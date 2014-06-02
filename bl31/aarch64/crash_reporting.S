/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <cpu_data.h>
#include <plat_macros.S>
#include <platform_def.h>

	.globl	dump_state_and_die
	.globl	dump_intr_state_and_die
	.globl  init_crash_reporting

#if CRASH_REPORTING
	/* ------------------------------------------------------
	 * The below section deals with dumping the system state
	 * when an unhandled exception is taken in EL3.
	 * The layout and the names of the registers which will
	 * be dumped during a unhandled exception is given below.
	 * ------------------------------------------------------
	 */
.section .rodata.dump_reg_name, "aS"
caller_saved_regs:	.asciz	"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",\
	 "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16",\
	 "x17", "x18", ""

callee_saved_regs: .asciz	"x19", "x20", "x21", "x22", "x23", "x24",\
	 "x25", "x26", "x27", "x28", "x29", "x30", ""

el3_sys_regs: .asciz	"scr_el3", "sctlr_el3", "cptr_el3", "tcr_el3",\
	 "daif", "mair_el3", "spsr_el3", "elr_el3", "ttbr0_el3", "esr_el3",\
	 "sp_el3", "far_el3", ""

non_el3_sys_0_regs: .asciz "spsr_el1", "elr_el1", "spsr_abt", "spsr_und",\
	"spsr_irq", "spsr_fiq", "sctlr_el1", "actlr_el1", "cpacr_el1",\
	"csselr_el1", "sp_el1", "esr_el1", "ttbr0_el1", "ttbr1_el1",\
	"mair_el1", "amair_el1", "tcr_el1", "tpidr_el1", ""

non_el3_sys_1_regs: .asciz "tpidr_el0", "tpidrro_el0", "dacr32_el2",\
	"ifsr32_el2", "par_el1", "far_el1", "afsr0_el1", "afsr1_el1",\
	"contextidr_el1", "vbar_el1", "cntp_ctl_el0", "cntp_cval_el0",\
	"cntv_ctl_el0", "cntv_cval_el0", "cntkctl_el1", "fpexc32_el2",\
	"sp_el0", ""

	/* -----------------------------------------------------
	 * Currently we are stack limited. Hence make sure that
	 * we dont try to dump more than 20 registers using the
	 * stack.
	 * -----------------------------------------------------
	 */

#define REG_SIZE 0x8

/* The caller saved registers are X0 to X18 */
#define CALLER_SAVED_REG_SIZE 		(20 * REG_SIZE)
/* The caller saved registers are X19 to X30 */
#define CALLEE_SAVED_REG_SIZE 		(12 * REG_SIZE)
/* The EL3 sys regs*/
#define EL3_SYS_REG_SIZE 			(12 * REG_SIZE)
/* The non EL3 sys regs set-0 */
#define NON_EL3_SYS_0_REG_SIZE 		(18 * REG_SIZE)
/* The non EL3 sys regs set-1 */
#define NON_EL3_SYS_1_REG_SIZE 		(18 * REG_SIZE)

	.macro print_caller_saved_regs
	sub	sp, sp, #CALLER_SAVED_REG_SIZE
	stp	x0, x1, [sp]
	stp	x2, x3, [sp, #(REG_SIZE * 2)]
	stp	x4, x5, [sp, #(REG_SIZE * 4)]
	stp	x6, x7, [sp, #(REG_SIZE * 6)]
	stp	x8, x9, [sp, #(REG_SIZE * 8)]
	stp	x10, x11, [sp, #(REG_SIZE * 10)]
	stp	x12, x13, [sp, #(REG_SIZE * 12)]
	stp	x14, x15, [sp, #(REG_SIZE * 14)]
	stp	x16, x17, [sp, #(REG_SIZE * 16)]
	stp	x18, xzr, [sp, #(REG_SIZE * 18)]
	adr	x0, caller_saved_regs
	mov	x1, sp
	bl	print_string_value
	add	sp, sp, #CALLER_SAVED_REG_SIZE
	.endm

	.macro print_callee_saved_regs
	sub	sp, sp, CALLEE_SAVED_REG_SIZE
	stp	x19, x20, [sp]
	stp	x21, x22, [sp, #(REG_SIZE * 2)]
	stp	x23, x24, [sp, #(REG_SIZE * 4)]
	stp	x25, x26, [sp, #(REG_SIZE * 6)]
	stp	x27, x28, [sp, #(REG_SIZE * 8)]
	stp	x29, x30, [sp, #(REG_SIZE * 10)]
	adr	x0, callee_saved_regs
	mov	x1, sp
	bl	print_string_value
	add	sp, sp, #CALLEE_SAVED_REG_SIZE
	.endm

	.macro print_el3_sys_regs
	sub	sp, sp, #EL3_SYS_REG_SIZE
	mrs	x9, scr_el3
	mrs	x10, sctlr_el3
	mrs	x11, cptr_el3
	mrs	x12, tcr_el3
	mrs	x13, daif
	mrs	x14, mair_el3
	mrs	x15, spsr_el3 /*save the elr and spsr regs seperately*/
	mrs	x16, elr_el3
	mrs	x17, ttbr0_el3
	mrs	x8, esr_el3
	mrs	x7, far_el3

	stp	x9, x10, [sp]
	stp	x11, x12, [sp, #(REG_SIZE * 2)]
	stp	x13, x14, [sp, #(REG_SIZE * 4)]
	stp	x15, x16, [sp, #(REG_SIZE * 6)]
	stp	x17, x8, [sp, #(REG_SIZE * 8)]
	stp	x0, x7, [sp, #(REG_SIZE * 10)] /* sp_el3 is in x0 */

	adr	x0, el3_sys_regs
	mov	x1, sp
	bl	print_string_value
	add	sp, sp, #EL3_SYS_REG_SIZE
	.endm

	.macro print_non_el3_sys_0_regs
	sub	sp, sp, #NON_EL3_SYS_0_REG_SIZE
	mrs	x9, spsr_el1
	mrs	x10, elr_el1
	mrs	x11, spsr_abt
	mrs	x12, spsr_und
	mrs	x13, spsr_irq
	mrs	x14, spsr_fiq
	mrs	x15, sctlr_el1
	mrs	x16, actlr_el1
	mrs	x17, cpacr_el1
	mrs	x8, csselr_el1

	stp	x9, x10, [sp]
	stp	x11, x12, [sp, #(REG_SIZE * 2)]
	stp	x13, x14, [sp, #(REG_SIZE * 4)]
	stp	x15, x16, [sp, #(REG_SIZE * 6)]
	stp	x17, x8, [sp, #(REG_SIZE * 8)]

	mrs	x10, sp_el1
	mrs	x11, esr_el1
	mrs	x12, ttbr0_el1
	mrs	x13, ttbr1_el1
	mrs	x14, mair_el1
	mrs	x15, amair_el1
	mrs	x16, tcr_el1
	mrs	x17, tpidr_el1

	stp	x10, x11, [sp, #(REG_SIZE * 10)]
	stp	x12, x13, [sp, #(REG_SIZE * 12)]
	stp	x14, x15, [sp, #(REG_SIZE * 14)]
	stp	x16, x17, [sp, #(REG_SIZE * 16)]

	adr	x0, non_el3_sys_0_regs
	mov	x1, sp
	bl	print_string_value
	add	sp, sp, #NON_EL3_SYS_0_REG_SIZE
	.endm

	.macro print_non_el3_sys_1_regs
	sub	sp, sp, #NON_EL3_SYS_1_REG_SIZE

	mrs	x9, tpidr_el0
	mrs	x10, tpidrro_el0
	mrs	x11, dacr32_el2
	mrs	x12, ifsr32_el2
	mrs	x13, par_el1
	mrs	x14, far_el1
	mrs	x15, afsr0_el1
	mrs	x16, afsr1_el1
	mrs	x17, contextidr_el1
	mrs	x8, vbar_el1

	stp	x9, x10, [sp]
	stp	x11, x12, [sp, #(REG_SIZE * 2)]
	stp	x13, x14, [sp, #(REG_SIZE * 4)]
	stp	x15, x16, [sp, #(REG_SIZE * 6)]
	stp	x17, x8, [sp, #(REG_SIZE * 8)]

	mrs	x10, cntp_ctl_el0
	mrs	x11, cntp_cval_el0
	mrs	x12, cntv_ctl_el0
	mrs	x13, cntv_cval_el0
	mrs	x14, cntkctl_el1
	mrs	x15, fpexc32_el2
	mrs	x8, sp_el0

	stp	x10, x11, [sp, #(REG_SIZE *10)]
	stp	x12, x13, [sp, #(REG_SIZE * 12)]
	stp	x14, x15, [sp, #(REG_SIZE * 14)]
	stp	x8, xzr, [sp, #(REG_SIZE * 16)]

	adr	x0, non_el3_sys_1_regs
	mov	x1, sp
	bl	print_string_value
	add	sp, sp, #NON_EL3_SYS_1_REG_SIZE
	.endm

	.macro init_crash_stack
	msr	cntfrq_el0, x0 /* we can corrupt this reg to free up x0 */
	mrs	x0, tpidr_el3

	/* Check if tpidr is initialized */
	cbz	x0, infinite_loop

	ldr	x0, [x0, #CPU_DATA_CRASH_STACK_OFFSET]
	/* store the x30 and sp to stack */
	str	x30, [x0, #-(REG_SIZE)]!
	mov	x30, sp
	str	x30, [x0, #-(REG_SIZE)]!
	mov	sp, x0
	mrs	x0, cntfrq_el0
	.endm

	/* ---------------------------------------------------
	 * The below function initializes the crash dump stack ,
	 * and prints the system state. This function
	 * will not return.
	 * ---------------------------------------------------
	 */
func dump_state_and_die
	init_crash_stack
	print_caller_saved_regs
	b	print_state

func dump_intr_state_and_die
	init_crash_stack
	print_caller_saved_regs
	plat_print_gic_regs /* fall through to print_state */

print_state:
	/* copy the original x30 from stack */
	ldr	x30, [sp, #REG_SIZE]
	print_callee_saved_regs
	/* copy the original SP_EL3 from stack to x0 and rewind stack */
	ldr x0, [sp], #(REG_SIZE * 2)
	print_el3_sys_regs
	print_non_el3_sys_0_regs
	print_non_el3_sys_1_regs

#else	/* CRASH_REPORING */

func dump_state_and_die
dump_intr_state_and_die:

#endif	/* CRASH_REPORING */

infinite_loop:
	b	infinite_loop


#define PCPU_CRASH_STACK_SIZE	0x140

	/* -----------------------------------------------------
	 * Per-cpu crash stacks in normal memory.
	 * -----------------------------------------------------
	 */
declare_stack pcpu_crash_stack, tzfw_normal_stacks, \
		PCPU_CRASH_STACK_SIZE, PLATFORM_CORE_COUNT

	/* -----------------------------------------------------
	 * Provides each CPU with a small stacks for reporting
	 * unhandled exceptions, and stores the stack address
	 * in cpu_data
	 *
	 * This can be called without a runtime stack
	 * clobbers: x0 - x4
	 * -----------------------------------------------------
	 */
func init_crash_reporting
	mov	x4, x30
	mov	x2, #0
	adr	x3, pcpu_crash_stack
init_crash_loop:
	mov	x0, x2
	bl	_cpu_data_by_index
	add	x3, x3, #PCPU_CRASH_STACK_SIZE
	str	x3, [x0, #CPU_DATA_CRASH_STACK_OFFSET]
	add	x2, x2, #1
	cmp	x2, #PLATFORM_CORE_COUNT
	b.lo	init_crash_loop
	ret	x4
