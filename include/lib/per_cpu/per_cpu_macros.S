/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_MACROS_S
#define PER_CPU_MACROS_S

#include <arch.h>
#include <asm_macros.S>
#include <lib/per_cpu/per_cpu_defs.h>

/* -----------------------------------------------------------------
 * per_cpu_cur <label>, <dst>, <clobber>
 * Returns: <dst> = per-CPU address of <label>
 * Clobbers: <dst> and <clobber>
 *
 * Defaults: dst=x0, clobber=x1
 * ----------------------------------------------------------------- */
.macro  per_cpu_cur label, dst=x0, clobber=x1
	/* Safety checks */
	.ifc \dst,\clobber
	.error "per_cpu_cur: dst and clobber must be different"
	.endif

	/* dst = absolute address of label */
	adr_l	\dst, \label

	/* clobber = absolute address of __PER_CPU_START__ */
	adr_l	\clobber, __PER_CPU_START__

	/* dst = (label - __PER_CPU_START__) */
	sub	\dst, \dst, \clobber

	/* clobber = per-cpu base (TPIDR_EL3) */
	mrs	\clobber, tpidr_el3

	/* dst = base + offset */
	add	\dst, \clobber, \dst
.endm

/* -----------------------------------------------------------------
 *	Populates tpidr_el3  with cpu specific per cpu section's
 *	offset address.
 *	Clobbers: x0 - x3
 * -----------------------------------------------------------------
 */
.macro per_cpu_init
	bl	plat_my_core_pos
	bl	per_cpu_base
	/* Update tpidr_el3 with offset */
	msr	tpidr_el3, x0
.endm

#endif /* PER_CPU_MACROS_S */
