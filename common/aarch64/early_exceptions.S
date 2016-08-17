/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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

#include <asm_macros.S>
#include <bl_common.h>

/* -----------------------------------------------------------------------------
 * Very simple stackless exception handlers used by BL2 and BL31 stages.
 * BL31 uses them before stacks are setup. BL2 uses them throughout.
 * -----------------------------------------------------------------------------
 */
	.globl	early_exceptions

vector_base early_exceptions

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0
	mov	x0, #SYNC_EXCEPTION_SP_EL0
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SynchronousExceptionSP0

vector_entry IrqSP0
	mov	x0, #IRQ_SP_EL0
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size IrqSP0

vector_entry FiqSP0
	mov	x0, #FIQ_SP_EL0
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size FiqSP0

vector_entry SErrorSP0
	mov	x0, #SERROR_SP_EL0
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx
	mov	x0, #SYNC_EXCEPTION_SP_ELX
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SynchronousExceptionSPx

vector_entry IrqSPx
	mov	x0, #IRQ_SP_ELX
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size IrqSPx

vector_entry FiqSPx
	mov	x0, #FIQ_SP_ELX
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size FiqSPx

vector_entry SErrorSPx
	mov	x0, #SERROR_SP_ELX
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64
	mov	x0, #SYNC_EXCEPTION_AARCH64
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SynchronousExceptionA64

vector_entry IrqA64
	mov	x0, #IRQ_AARCH64
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size IrqA64

vector_entry FiqA64
	mov	x0, #FIQ_AARCH64
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size FiqA64

vector_entry SErrorA64
	mov	x0, #SERROR_AARCH64
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA32
	mov	x0, #SYNC_EXCEPTION_AARCH32
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SynchronousExceptionA32

vector_entry IrqA32
	mov	x0, #IRQ_AARCH32
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size IrqA32

vector_entry FiqA32
	mov	x0, #FIQ_AARCH32
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size FiqA32

vector_entry SErrorA32
	mov	x0, #SERROR_AARCH32
	bl	plat_report_exception
	bl	plat_panic_handler
	check_vector_size SErrorA32
