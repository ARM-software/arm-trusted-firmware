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
	/* ---------------------------------------------
	 * Zero out the callee saved register to prevent
	 * leakage of secure state into the normal world
	 * during the first ERET after a cold/warm boot.
	 * ---------------------------------------------
	 */
	.macro	zero_callee_saved_regs
	mov	x19, xzr
	mov	x20, xzr
	mov	x21, xzr
	mov	x22, xzr
	mov	x23, xzr
	mov	x24, xzr
	mov	x25, xzr
	mov	x26, xzr
	mov	x27, xzr
	mov	x28, xzr
	mov	x29, xzr
	.endm

	.macro	switch_to_exception_stack reg1 reg2
	mov     \reg1 , sp
	ldr	\reg2, [\reg1, #CTX_EL3STATE_OFFSET + CTX_EXCEPTION_SP]
	mov	sp, \reg2
	.endm

	/* -----------------------------------------------------
	 * Handle SMC exceptions seperately from other sync.
	 * exceptions.
	 * -----------------------------------------------------
	 */
	.macro	handle_sync_exception
	stp	x30, xzr, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH

	cmp	x30, #EC_AARCH32_SMC
	b.eq	smc_handler32

	cmp	x30, #EC_AARCH64_SMC
	b.eq	smc_handler64

	/* -----------------------------------------------------
	 * The following code handles any synchronous exception
	 * that is not an SMC. SP_EL3 is pointing to a context
	 * structure where all the scratch registers are saved.
	 * An exception stack is also retrieved from the context
	 * Currently, a register dump is printed since BL31 does
	 * not expect any such exceptions.
	 * -----------------------------------------------------
	 */
	bl	save_scratch_registers
	switch_to_exception_stack x0 x1

	/* Save the core_context pointer for handled faults */
	stp	x0, xzr, [sp, #-0x10]!
	bl	fault_handler
	ldp	x0, xzr, [sp], #0x10

	mov	sp, x0
	bl	restore_scratch_registers
	ldp	x30, xzr, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	eret
	.endm

	/* -----------------------------------------------------
	 * Use a platform defined mechanism to report an async.
	 * exception.
	 * -----------------------------------------------------
	 */
	.macro	handle_async_exception type
	stp	x30, xzr, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	bl	save_scratch_registers
	switch_to_exception_stack x0 x1

	/* Save the core_context pointer */
	stp	x0, xzr, [sp, #-0x10]!
	mov	x0, \type
	bl	plat_report_exception
	ldp	x0, xzr, [sp], #0x10

	mov	sp, x0
	bl	restore_scratch_registers
	ldp	x30, xzr, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	.endm

