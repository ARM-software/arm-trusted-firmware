/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <context.h>

/* -----------------------------------------------------------------------------
 * File contains an EL2 equivalent of the EL3 vector table from:
 * 	.../bl1/aarch64/bl1_exceptions.S
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * Very simple stackless exception handlers used by BL1.
 * -----------------------------------------------------------------------------
 */
	.globl	bl1_exceptions

vector_base bl1_exceptions

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0
	mov	x0, #SYNC_EXCEPTION_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SynchronousExceptionSP0

vector_entry IrqSP0
	mov	x0, #IRQ_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry IrqSP0

vector_entry FiqSP0
	mov	x0, #FIQ_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry FiqSP0

vector_entry SErrorSP0
	mov	x0, #SERROR_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx
	mov	x0, #SYNC_EXCEPTION_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SynchronousExceptionSPx

vector_entry IrqSPx
	mov	x0, #IRQ_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry IrqSPx

vector_entry FiqSPx
	mov	x0, #FIQ_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry FiqSPx

vector_entry SErrorSPx
	mov	x0, #SERROR_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64
	/* The current v8-R64 implementation does not support conduit calls */
	b	el2_panic
end_vector_entry SynchronousExceptionA64

vector_entry IrqA64
	mov	x0, #IRQ_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry IrqA64

vector_entry FiqA64
	mov	x0, #FIQ_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry FiqA64

vector_entry SErrorA64
	mov	x0, #SERROR_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SErrorA64


unexpected_sync_exception:
	mov	x0, #SYNC_EXCEPTION_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler

	/* -----------------------------------------------------
	 * Save Secure/Normal world context and jump to
	 * BL1 SMC handler.
	 * -----------------------------------------------------
	 */
