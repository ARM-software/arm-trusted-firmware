/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>

	.globl	bl2_vector_table

vector_base bl2_vector_table
	b	bl2_entrypoint
	b	report_exception	/* Undef */
	b	report_exception	/* SVC call */
	b	report_prefetch_abort	/* Prefetch abort */
	b	report_data_abort	/* Data abort */
	b	report_exception	/* Reserved */
	b	report_exception	/* IRQ */
	b	report_exception	/* FIQ */
