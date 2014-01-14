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
#include <runtime_svc.h>

	.globl	runtime_exceptions


#include <asm_macros.S>


	.section	.text, "ax"; .align 11
	
	.align	7
runtime_exceptions:
	/* -----------------------------------------------------
	 * Current EL with _sp_el0 : 0x0 - 0x180
	 * -----------------------------------------------------
	 */
sync_exception_sp_el0:
	exception_entry save_regs
	mov	x0, #SYNC_EXCEPTION_SP_EL0
	mov	x1, sp
	bl	sync_exception_handler
	exception_exit restore_regs
	eret

	.align	7
irq_sp_el0:
	exception_entry save_regs
	mov	x0, #IRQ_SP_EL0
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
fiq_sp_el0:
	exception_entry save_regs
	mov	x0, #FIQ_SP_EL0
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
serror_sp_el0:
	exception_entry save_regs
	mov	x0, #SERROR_SP_EL0
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x380
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_sp_elx:
	exception_entry save_regs
	mov	x0, #SYNC_EXCEPTION_SP_ELX
	mov	x1, sp
	bl	sync_exception_handler
	exception_exit restore_regs
	eret

	.align	7
irq_sp_elx:
	exception_entry save_regs
	mov	x0, #IRQ_SP_ELX
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
fiq_sp_elx:
	exception_entry save_regs
	mov	x0, #FIQ_SP_ELX
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
serror_sp_elx:
	exception_entry save_regs
	mov	x0, #SERROR_SP_ELX
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x580
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_aarch64:
	exception_entry save_regs
	mov	x0, #SYNC_EXCEPTION_AARCH64
	mov	x1, sp
	bl	sync_exception_handler
	exception_exit restore_regs
	eret

	.align	7
irq_aarch64:
	exception_entry save_regs
	mov	x0, #IRQ_AARCH64
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
fiq_aarch64:
	exception_entry save_regs
	mov	x0, #FIQ_AARCH64
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
serror_aarch64:
	exception_entry save_regs
	mov	x0, #IRQ_AARCH32
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x780
	 * -----------------------------------------------------
	 */
	.align	7
sync_exception_aarch32:
	exception_entry save_regs
	mov	x0, #SYNC_EXCEPTION_AARCH32
	mov	x1, sp
	bl	sync_exception_handler
	exception_exit restore_regs
	eret

	.align	7
irq_aarch32:
	exception_entry save_regs
	mov	x0, #IRQ_AARCH32
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
fiq_aarch32:
	exception_entry save_regs
	mov	x0, #FIQ_AARCH32
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7
serror_aarch32:
	exception_entry save_regs
	mov	x0, #SERROR_AARCH32
	mov	x1, sp
	bl	async_exception_handler
	exception_exit restore_regs
	eret

	.align	7

