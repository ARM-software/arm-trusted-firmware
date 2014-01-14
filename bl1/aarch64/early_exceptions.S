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
#include <bl_common.h>
#include <bl1.h>
#include <platform.h>
#include <runtime_svc.h>

	.globl	early_exceptions


	.section	.text, "ax"; .align 11

	/* -----------------------------------------------------
	 * Very simple exception handlers used by BL1 and BL2.
	 * Apart from one SMC exception all other traps loop
	 * endlessly.
	 * -----------------------------------------------------
	 */
	.align	7
early_exceptions:
	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x180
	 * -----------------------------------------------------
	 */
SynchronousExceptionSP0:
	mov	x0, #SYNC_EXCEPTION_SP_EL0
	bl	plat_report_exception
	b	SynchronousExceptionSP0

	.align	7
IrqSP0:
	mov	x0, #IRQ_SP_EL0
	bl	plat_report_exception
	b	IrqSP0

	.align	7
FiqSP0:
	mov	x0, #FIQ_SP_EL0
	bl	plat_report_exception
	b	FiqSP0

	.align	7
SErrorSP0:
	mov	x0, #SERROR_SP_EL0
	bl	plat_report_exception
	b	SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x380
	 * -----------------------------------------------------
	 */
	.align	7
SynchronousExceptionSPx:
	mov	x0, #SYNC_EXCEPTION_SP_ELX
	bl	plat_report_exception
	b	SynchronousExceptionSPx

	.align	7
IrqSPx:
	mov	x0, #IRQ_SP_ELX
	bl	plat_report_exception
	b	IrqSPx

	.align	7
FiqSPx:
	mov	x0, #FIQ_SP_ELX
	bl	plat_report_exception
	b	FiqSPx

	.align	7
SErrorSPx:
	mov	x0, #SERROR_SP_ELX
	bl	plat_report_exception
	b	SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x580
	 * -----------------------------------------------------
	 */
	.align	7
SynchronousExceptionA64:
	/* ---------------------------------------------
	 * Only a single SMC exception from BL2 to ask
	 * BL1 to pass EL3 control to BL31 is expected
	 * here.
	 * ---------------------------------------------
	 */
	sub	sp, sp, #0x40
	stp	x0, x1, [sp, #0x0]
	stp	x2, x3, [sp, #0x10]
	stp	x4, x5, [sp, #0x20]
	stp	x6, x7, [sp, #0x30]
	mov	x19, x0
	mov	x20, x1
	mov	x21, x2

	mov	x0, #SYNC_EXCEPTION_AARCH64
	bl	plat_report_exception

	bl	read_esr
	ubfx	x1, x0, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x1, #EC_AARCH64_SMC
	b.ne	panic
	mov	x1, #RUN_IMAGE
	cmp	x19, x1
	b.ne	panic
	mov	x0, x20
	mov	x1, x21
	mov	x2, x3
	mov	x3, x4
	bl	display_boot_progress
	mov	x0, x20
	bl	write_elr
	mov	x0, x21
	bl	write_spsr
	ubfx	x0, x21, #MODE_EL_SHIFT, #2
	cmp	x0, #MODE_EL3
	b.ne	skip_mmu_teardown
	/* ---------------------------------------------
	 * If BL31 is to be executed in EL3 as well
	 * then turn off the MMU so that it can perform
	 * its own setup. TODO: Assuming flat mapped
	 * translations here. Also all should go into a
	 * separate MMU teardown function
	 * ---------------------------------------------
	 */
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	bl	read_sctlr
	bic	x0, x0, x1
	bl	write_sctlr
	mov	x0, #DCCISW
	bl	dcsw_op_all
	bl	tlbialle3
skip_mmu_teardown:
	ldp     x6, x7, [sp, #0x30]
	ldp     x4, x5, [sp, #0x20]
	ldp     x2, x3, [sp, #0x10]
	ldp     x0, x1, [sp, #0x0]
	add     sp, sp, #0x40
	eret
panic:
	b	panic
	.align	7
IrqA64:
	mov	x0, #IRQ_AARCH64
	bl	plat_report_exception
	b	IrqA64

	.align	7
FiqA64:
	mov	x0, #FIQ_AARCH64
	bl	plat_report_exception
	b	FiqA64

	.align	7
SErrorA64:
	mov	x0, #SERROR_AARCH64
	bl	plat_report_exception
	b   	SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x0 - 0x180
	 * -----------------------------------------------------
	 */
	.align	7
SynchronousExceptionA32:
	mov	x0, #SYNC_EXCEPTION_AARCH32
	bl	plat_report_exception
	b	SynchronousExceptionA32

	.align	7
IrqA32:
	mov	x0, #IRQ_AARCH32
	bl	plat_report_exception
	b	IrqA32

	.align	7
FiqA32:
	mov	x0, #FIQ_AARCH32
	bl	plat_report_exception
	b	FiqA32

	.align	7
SErrorA32:
	mov	x0, #SERROR_AARCH32
	bl	plat_report_exception
	b	SErrorA32
