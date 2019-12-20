/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMF_ASM_MACROS_S
#define PMF_ASM_MACROS_S

#define PMF_TS_SIZE	8

	/*
	 * This macro calculates the address of the per-cpu timestamp
	 * for the given service name and local timestamp id.
	 * Clobbers: r0 - r4
	 */
	.macro pmf_calc_timestamp_addr _name, _tid
	mov	r4, lr
	bl	plat_my_core_pos
	mov	lr, r4
	ldr	r1, =__PERCPU_TIMESTAMP_SIZE__
	mov	r2, #(\_tid * PMF_TS_SIZE)
	mla	r0, r0, r1, r2
	ldr	r1, =pmf_ts_mem_\_name
	add	r0, r0, r1
	.endm

#endif /* PMF_ASM_MACROS_S */
