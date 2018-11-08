/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMF_ASM_MACROS_S
#define PMF_ASM_MACROS_S

#define PMF_TS_SIZE	8

	/*
	 * This macro calculates the address of the per-cpu timestamp
	 * for the given service name and local timestamp id.
	 * Clobbers: x0 - x9
	 */
	.macro pmf_calc_timestamp_addr _name, _tid
	mov	x9, x30
	bl	plat_my_core_pos
	mov	x30, x9
	adr	x2, __PMF_PERCPU_TIMESTAMP_END__
	adr	x1, __PMF_TIMESTAMP_START__
	sub	x1, x2, x1
	mov	x2, #(\_tid * PMF_TS_SIZE)
	madd	x0, x0, x1, x2
	adr	x1, pmf_ts_mem_\_name
	add	x0, x0, x1
	.endm

#endif /* PMF_ASM_MACROS_S */
