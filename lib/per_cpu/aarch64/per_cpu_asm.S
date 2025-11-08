/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_ASM_S
#define PER_CPU_ASM_S

#include <arch.h>
#include <asm_macros.S>
#include <lib/per_cpu/per_cpu_defs.h>

.globl per_cpu_base

/* -----------------------------------------------------------------
 * Gets the per cpu base address for particular cpu. When NUMA awareness is
 * enabled, it is the platforms responsibility to implement
 * plat_per_cpu_base.
 *
 * This function would be called by asm and C routines. If NUMA awareness is
 * enabled, care must be taken to preserve the clobber list.
 *
 * args - cpu in x0
 * ret  - per cpu base address in x0.
 * -----------------------------------------------------------------
 */
func per_cpu_base
#if PLATFORM_NODE_COUNT == 1
	adr_l	x1, __PER_CPU_START__
	/* x0 += r * __PER_CPU_UNIT_SECTION_SIZE__ */
	adr_l	x2, __PER_CPU_UNIT_END__
	sub	x2, x2, x1
	madd	x0, x2, x0, x1
	ret
#else
	b	plat_per_cpu_base
	/* Intentionally using 'b' instead of 'bl' to avoid creating
	 * a return address. This saves the link register (LR) from being
	 * clobbered and reduces the clobber list in the calling context.
	 * Any future updates to append code after the branch would mean
	 * moving from the branch from "b" to "bl".
	 */
#endif
endfunc per_cpu_base

#endif /* PER_CPU_ASM_S*/
