/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <context.h>

/* -----------------------------------------------------------------------------
 * Very simple stackless exception handlers used by the spm shim layer.
 * -----------------------------------------------------------------------------
 */
	.globl	spm_shim_exceptions_ptr

vector_base spm_shim_exceptions_ptr, .spm_shim_exceptions

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0, .spm_shim_exceptions
	b	.
	check_vector_size SynchronousExceptionSP0

vector_entry IrqSP0, .spm_shim_exceptions
	b	.
	check_vector_size IrqSP0

vector_entry FiqSP0, .spm_shim_exceptions
	b	.
	check_vector_size FiqSP0

vector_entry SErrorSP0, .spm_shim_exceptions
	b	.
	check_vector_size SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx, .spm_shim_exceptions
	b	.
	check_vector_size SynchronousExceptionSPx

vector_entry IrqSPx, .spm_shim_exceptions
	b	.
	check_vector_size IrqSPx

vector_entry FiqSPx, .spm_shim_exceptions
	b	.
	check_vector_size FiqSPx

vector_entry SErrorSPx, .spm_shim_exceptions
	b	.
	check_vector_size SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600. No exceptions
	 * are handled since secure_partition does not implement
	 * a lower EL
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64, .spm_shim_exceptions
	msr	tpidr_el1, x30
	mrs	x30, esr_el1
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH

	cmp	x30, #EC_AARCH64_SVC
	b.eq 	do_smc

	cmp	x30, #EC_AARCH32_SVC
	b.eq	do_smc

	cmp	x30, #EC_AARCH64_SYS
	b.eq	handle_sys_trap

	/* Fail in all the other cases */
	b	panic

	/* ---------------------------------------------
	 * Tell SPM that we are done initialising
	 * ---------------------------------------------
	 */
do_smc:
	mrs	x30, tpidr_el1
	smc	#0
	eret

	/* AArch64 system instructions trap are handled as a panic for now */
handle_sys_trap:
panic:
	b	panic
	check_vector_size SynchronousExceptionA64

vector_entry IrqA64, .spm_shim_exceptions
	b	.
	check_vector_size IrqA64

vector_entry FiqA64, .spm_shim_exceptions
	b	.
	check_vector_size FiqA64

vector_entry SErrorA64, .spm_shim_exceptions
	b	.
	check_vector_size SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA32, .spm_shim_exceptions
	b	.
	check_vector_size SynchronousExceptionA32

vector_entry IrqA32, .spm_shim_exceptions
	b	.
	check_vector_size IrqA32

vector_entry FiqA32, .spm_shim_exceptions
	b	.
	check_vector_size FiqA32

vector_entry SErrorA32, .spm_shim_exceptions
	b	.
	check_vector_size SErrorA32
