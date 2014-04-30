/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <cm_macros.S>
#include <context.h>
#include <platform.h>
#include <runtime_svc.h>

	.globl	runtime_exceptions
	.globl	el3_exit
	.globl	get_exception_stack

	.macro save_x18_to_x29_sp_el0
	stp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	stp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	stp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	stp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	stp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	stp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	mrs	x18, sp_el0
	str	x18, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_SP_EL0]
	.endm

	.section	.vectors, "ax"; .align 11

	.align	7
runtime_exceptions:
	/* -----------------------------------------------------
	 * Current EL with _sp_el0 : 0x0 - 0x180
	 * -----------------------------------------------------
	 */
sync_exception_sp_el0:
	/* -----------------------------------------------------
	 * We don't expect any synchronous exceptions from EL3
	 * -----------------------------------------------------
	 */
	wfi
	b	sync_exception_sp_el0
	check_vector_size sync_exception_sp_el0

	.align	7
	/* -----------------------------------------------------
	 * EL3 code is non-reentrant. Any asynchronous exception
	 * is a serious error. Loop infinitely.
	 * -----------------------------------------------------
	 */
irq_sp_el0:
	handle_async_exception IRQ_SP_EL0
	b	irq_sp_el0
	check_vector_size irq_sp_el0

	.align	7
fiq_sp_el0:
	handle_async_exception FIQ_SP_EL0
	b	fiq_sp_el0
	check_vector_size fiq_sp_el0

	.align	7
serror_sp_el0:
	handle_async_exception SERROR_SP_EL0
	b	serror_sp_el0
	check_vector_size serror_sp_el0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x380
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_sp_elx:
	/* -----------------------------------------------------
	 * This exception will trigger if anything went wrong
	 * during a previous exception entry or exit or while
	 * handling an earlier unexpected synchronous exception.
	 * In any case we cannot rely on SP_EL3. Switching to a
	 * known safe area of memory will corrupt at least a
	 * single register. It is best to enter wfi in loop as
	 * that will preserve the system state for analysis
	 * through a debugger later.
	 * -----------------------------------------------------
	 */
	wfi
	b	sync_exception_sp_elx
	check_vector_size sync_exception_sp_elx

	/* -----------------------------------------------------
	 * As mentioned in the previous comment, all bets are
	 * off if SP_EL3 cannot be relied upon. Report their
	 * occurrence.
	 * -----------------------------------------------------
	 */
	.align	7
irq_sp_elx:
	b	irq_sp_elx
	check_vector_size irq_sp_elx

	.align	7
fiq_sp_elx:
	b	fiq_sp_elx
	check_vector_size fiq_sp_elx

	.align	7
serror_sp_elx:
	b	serror_sp_elx
	check_vector_size serror_sp_elx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x580
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_aarch64:
	/* -----------------------------------------------------
	 * This exception vector will be the entry point for
	 * SMCs and traps that are unhandled at lower ELs most
	 * commonly. SP_EL3 should point to a valid cpu context
	 * where the general purpose and system register state
	 * can be saved.
	 * -----------------------------------------------------
	 */
	handle_sync_exception
	check_vector_size sync_exception_aarch64

	.align	7
	/* -----------------------------------------------------
	 * Asynchronous exceptions from lower ELs are not
	 * currently supported. Report their occurrence.
	 * -----------------------------------------------------
	 */
irq_aarch64:
	handle_async_exception IRQ_AARCH64
	b	irq_aarch64
	check_vector_size irq_aarch64

	.align	7
fiq_aarch64:
	handle_async_exception FIQ_AARCH64
	b	fiq_aarch64
	check_vector_size fiq_aarch64

	.align	7
serror_aarch64:
	handle_async_exception SERROR_AARCH64
	b	serror_aarch64
	check_vector_size serror_aarch64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x780
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_aarch32:
	/* -----------------------------------------------------
	 * This exception vector will be the entry point for
	 * SMCs and traps that are unhandled at lower ELs most
	 * commonly. SP_EL3 should point to a valid cpu context
	 * where the general purpose and system register state
	 * can be saved.
	 * -----------------------------------------------------
	 */
	handle_sync_exception
	check_vector_size sync_exception_aarch32

	.align	7
	/* -----------------------------------------------------
	 * Asynchronous exceptions from lower ELs are not
	 * currently supported. Report their occurrence.
	 * -----------------------------------------------------
	 */
irq_aarch32:
	handle_async_exception IRQ_AARCH32
	b	irq_aarch32
	check_vector_size irq_aarch32

	.align	7
fiq_aarch32:
	handle_async_exception FIQ_AARCH32
	b	fiq_aarch32
	check_vector_size fiq_aarch32

	.align	7
serror_aarch32:
	handle_async_exception SERROR_AARCH32
	b	serror_aarch32
	check_vector_size serror_aarch32

	.align	7

	/* -----------------------------------------------------
	 * The following code handles secure monitor calls.
	 * Depending upon the execution state from where the SMC
	 * has been invoked, it frees some general purpose
	 * registers to perform the remaining tasks. They
	 * involve finding the runtime service handler that is
	 * the target of the SMC & switching to runtime stacks
	 * (SP_EL0) before calling the handler.
	 *
	 * Note that x30 has been explicitly saved and can be
	 * used here
	 * -----------------------------------------------------
	 */
func smc_handler
smc_handler32:
	/* Check whether aarch32 issued an SMC64 */
	tbnz	x0, #FUNCID_CC_SHIFT, smc_prohibited

	/* -----------------------------------------------------
	 * Since we're are coming from aarch32, x8-x18 need to
	 * be saved as per SMC32 calling convention. If a lower
	 * EL in aarch64 is making an SMC32 call then it must
	 * have saved x8-x17 already therein.
	 * -----------------------------------------------------
	 */
	stp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	stp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	stp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	stp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	stp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]

	/* x4-x7, x18, sp_el0 are saved below */

smc_handler64:
	/* -----------------------------------------------------
	 * Populate the parameters for the SMC handler. We
	 * already have x0-x4 in place. x5 will point to a
	 * cookie (not used now). x6 will point to the context
	 * structure (SP_EL3) and x7 will contain flags we need
	 * to pass to the handler Hence save x5-x7. Note that x4
	 * only needs to be preserved for AArch32 callers but we
	 * do it for AArch64 callers as well for convenience
	 * -----------------------------------------------------
	 */
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	stp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]

	/* Save rest of the gpregs and sp_el0*/
	save_x18_to_x29_sp_el0

	mov	x5, xzr
	mov	x6, sp

	/* Get the unique owning entity number */
	ubfx	x16, x0, #FUNCID_OEN_SHIFT, #FUNCID_OEN_WIDTH
	ubfx	x15, x0, #FUNCID_TYPE_SHIFT, #FUNCID_TYPE_WIDTH
	orr	x16, x16, x15, lsl #FUNCID_OEN_WIDTH

	adr	x11, (__RT_SVC_DESCS_START__ + RT_SVC_DESC_HANDLE)

	/* Load descriptor index from array of indices */
	adr	x14, rt_svc_descs_indices
	ldrb	w15, [x14, x16]

	/* -----------------------------------------------------
	 * Restore the saved C runtime stack value which will
	 * become the new SP_EL0 i.e. EL3 runtime stack. It was
	 * saved in the 'cpu_context' structure prior to the last
	 * ERET from EL3.
	 * -----------------------------------------------------
	 */
	ldr	x12, [x6, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]

	/*
	 * Any index greater than 127 is invalid. Check bit 7 for
	 * a valid index
	 */
	tbnz	w15, 7, smc_unknown

	/* Switch to SP_EL0 */
	msr	spsel, #0

	/* -----------------------------------------------------
	 * Get the descriptor using the index
	 * x11 = (base + off), x15 = index
	 *
	 * handler = (base + off) + (index << log2(size))
	 * -----------------------------------------------------
	 */
	lsl	w10, w15, #RT_SVC_SIZE_LOG2
	ldr	x15, [x11, w10, uxtw]

	/* -----------------------------------------------------
	 * Save the SPSR_EL3, ELR_EL3, & SCR_EL3 in case there
	 * is a world switch during SMC handling.
	 * TODO: Revisit if all system registers can be saved
	 * later.
	 * -----------------------------------------------------
	 */
	mrs	x16, spsr_el3
	mrs	x17, elr_el3
	mrs	x18, scr_el3
	stp	x16, x17, [x6, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	stp	x18, xzr, [x6, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]

	/* Copy SCR_EL3.NS bit to the flag to indicate caller's security */
	bfi	x7, x18, #0, #1

	mov	sp, x12

	/* -----------------------------------------------------
	 * Call the Secure Monitor Call handler and then drop
	 * directly into el3_exit() which will program any
	 * remaining architectural state prior to issuing the
	 * ERET to the desired lower EL.
	 * -----------------------------------------------------
	 */
#if DEBUG
	cbz	x15, rt_svc_fw_critical_error
#endif
	blr	x15

	/* -----------------------------------------------------
	 * This routine assumes that the SP_EL3 is pointing to
	 * a valid context structure from where the gp regs and
	 * other special registers can be retrieved.
	 *
	 * Keep it in the same section as smc_handler as this
	 * function uses a fall-through to el3_exit
	 * -----------------------------------------------------
	 */
el3_exit: ; .type el3_exit, %function
	/* -----------------------------------------------------
	 * Save the current SP_EL0 i.e. the EL3 runtime stack
	 * which will be used for handling the next SMC. Then
	 * switch to SP_EL3
	 * -----------------------------------------------------
	 */
	mov	x17, sp
	msr	spsel, #1
	str	x17, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]

	/* -----------------------------------------------------
	 * Restore SPSR_EL3, ELR_EL3 and SCR_EL3 prior to ERET
	 * -----------------------------------------------------
	 */
	ldp	x18, xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]
	ldp	x16, x17, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	msr	scr_el3, x18
	msr	spsr_el3, x16
	msr	elr_el3, x17

	/* Restore saved general purpose registers and return */
	bl	restore_gp_registers
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	eret

smc_unknown:
	/*
	 * Here we restore x4-x18 regardless of where we came from. AArch32
	 * callers will find the registers contents unchanged, but AArch64
	 * callers will find the registers modified (with stale earlier NS
	 * content). Either way, we aren't leaking any secure information
	 * through them
	 */
	bl	restore_gp_registers_callee

smc_prohibited:
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	mov	w0, #SMC_UNK
	eret

rt_svc_fw_critical_error:
	b	rt_svc_fw_critical_error

	/* -----------------------------------------------------
	 * The following functions are used to saved and restore
	 * all the general pupose registers. Ideally we would
	 * only save and restore the callee saved registers when
	 * a world switch occurs but that type of implementation
	 * is more complex. So currently we will always save and
	 * restore these registers on entry and exit of EL3.
	 * These are not macros to ensure their invocation fits
	 * within the 32 instructions per exception vector.
	 * -----------------------------------------------------
	 */
func save_gp_registers
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	stp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]
	stp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	stp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	stp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	stp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	stp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	save_x18_to_x29_sp_el0
	ret

func restore_gp_registers
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]

restore_gp_registers_callee:
	ldr	x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_SP_EL0]

	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	ldp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]
	ldp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	ldp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	ldp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	ldp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	msr	sp_el0, x17
	ldp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	ldp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	ldp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	ldp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	ldp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	ldp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	ldp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	ret

	/* -----------------------------------------------------
	 * 256 bytes of exception stack for each cpu
	 * -----------------------------------------------------
	 */
#if DEBUG
#define PCPU_EXCEPTION_STACK_SIZE	0x300
#else
#define PCPU_EXCEPTION_STACK_SIZE	0x100
#endif
	/* -----------------------------------------------------
	 * void get_exception_stack (uint64_t mpidr) : This
	 * function is used to allocate a small stack for
	 * reporting unhandled exceptions
	 * -----------------------------------------------------
	 */
func get_exception_stack
	mov	x10, x30 // lr
	get_mp_stack pcpu_exception_stack, PCPU_EXCEPTION_STACK_SIZE
	ret	x10

	/* -----------------------------------------------------
	 * Per-cpu exception stacks in normal memory.
	 * -----------------------------------------------------
	 */
declare_stack pcpu_exception_stack, tzfw_normal_stacks, \
		PCPU_EXCEPTION_STACK_SIZE, PLATFORM_CORE_COUNT
