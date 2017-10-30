/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl1.h>
#include <bl_common.h>
#include <context.h>

/* -----------------------------------------------------------------------------
 * Very simple stackless exception handlers used by BL2.
 * -----------------------------------------------------------------------------
 */
	.globl	bl2_el3_exceptions

vector_base bl2_el3_exceptions

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0
	mov	x0, #SYNC_EXCEPTION_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SynchronousExceptionSP0

vector_entry IrqSP0
	mov	x0, #IRQ_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size IrqSP0

vector_entry FiqSP0
	mov	x0, #FIQ_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size FiqSP0

vector_entry SErrorSP0
	mov	x0, #SERROR_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx
	mov	x0, #SYNC_EXCEPTION_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SynchronousExceptionSPx

vector_entry IrqSPx
	mov	x0, #IRQ_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size IrqSPx

vector_entry FiqSPx
	mov	x0, #FIQ_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size FiqSPx

vector_entry SErrorSPx
	mov	x0, #SERROR_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64
	mov	x0, #SYNC_EXCEPTION_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SynchronousExceptionA64

vector_entry IrqA64
	mov	x0, #IRQ_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size IrqA64

vector_entry FiqA64
	mov	x0, #FIQ_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size FiqA64

vector_entry SErrorA64
	mov	x0, #SERROR_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA32
	mov	x0, #SYNC_EXCEPTION_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SynchronousExceptionA32

vector_entry IrqA32
	mov	x0, #IRQ_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size IrqA32

vector_entry FiqA32
	mov	x0, #FIQ_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size FiqA32

vector_entry SErrorA32
	mov	x0, #SERROR_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
	check_vector_size SErrorA32
