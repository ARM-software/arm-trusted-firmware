/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert_macros.S>
#include <asm_macros.S>

	.globl	amu_group0_cnt_read_internal
	.globl	amu_group0_cnt_write_internal
	.globl	amu_group1_cnt_read_internal
	.globl	amu_group1_cnt_write_internal
	.globl	amu_group1_set_evtype_internal

/*
 * uint64_t amu_group0_cnt_read_internal(int idx);
 *
 * Given `idx`, read the corresponding AMU counter
 * and return it in `r0` and `r1`.
 */
func amu_group0_cnt_read_internal
#if ENABLE_ASSERTIONS
	/* `idx` should be between [0, 3] */
	mov	r1, r0
	lsr	r1, r1, #2
	cmp	r1, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of ldcopr16/bx lr instruction pair
	 * in the table below.
	 */
	adr	r1, 1f
	lsl	r0, r0, #3	/* each ldcopr16/bx lr sequence is 8 bytes */
	add	r1, r1, r0
	bx	r1
1:
	ldcopr16	r0, r1, AMEVCNTR00	/* index 0 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR01	/* index 1 */
	bx 		lr
	ldcopr16	r0, r1, AMEVCNTR02	/* index 2 */
	bx 		lr
	ldcopr16	r0, r1, AMEVCNTR03	/* index 3 */
	bx 		lr
endfunc amu_group0_cnt_read_internal

/*
 * void amu_group0_cnt_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 * `idx` is passed in `r0` and `val` is passed in `r2` and `r3`.
 * `r1` is used as a scratch register.
 */
func amu_group0_cnt_write_internal
#if ENABLE_ASSERTIONS
	/* `idx` should be between [0, 3] */
	mov	r1, r0
	lsr	r1, r1, #2
	cmp	r1, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of stcopr16/bx lr instruction pair
	 * in the table below.
	 */
	adr	r1, 1f
	lsl	r0, r0, #3	/* each stcopr16/bx lr sequence is 8 bytes */
	add	r1, r1, r0
	bx	r1

1:
	stcopr16	r2, r3, AMEVCNTR00	/* index 0 */
	bx 		lr
	stcopr16	r2, r3, AMEVCNTR01	/* index 1 */
	bx 		lr
	stcopr16	r2, r3, AMEVCNTR02	/* index 2 */
	bx 		lr
	stcopr16	r2, r3, AMEVCNTR03	/* index 3 */
	bx 		lr
endfunc amu_group0_cnt_write_internal

/*
 * uint64_t amu_group1_cnt_read_internal(int idx);
 *
 * Given `idx`, read the corresponding AMU counter
 * and return it in `r0` and `r1`.
 */
func amu_group1_cnt_read_internal
#if ENABLE_ASSERTIONS
	/* `idx` should be between [0, 15] */
	mov	r1, r0
	lsr	r1, r1, #4
	cmp	r1, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of ldcopr16/bx lr instruction pair
	 * in the table below.
	 */
	adr	r1, 1f
	lsl	r0, r0, #3	/* each ldcopr16/bx lr sequence is 8 bytes */
	add	r1, r1, r0
	bx	r1

1:
	ldcopr16	r0, r1, AMEVCNTR10	/* index 0 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR11	/* index 1 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR12	/* index 2 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR13	/* index 3 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR14	/* index 4 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR15	/* index 5 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR16	/* index 6 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR17	/* index 7 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR18	/* index 8 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR19	/* index 9 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR1A	/* index 10 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR1B	/* index 11 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR1C	/* index 12 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR1D	/* index 13 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR1E	/* index 14 */
	bx		lr
	ldcopr16	r0, r1, AMEVCNTR1F	/* index 15 */
	bx		lr
endfunc amu_group1_cnt_read_internal

/*
 * void amu_group1_cnt_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 * `idx` is passed in `r0` and `val` is passed in `r2` and `r3`.
 * `r1` is used as a scratch register.
 */
func amu_group1_cnt_write_internal
#if ENABLE_ASSERTIONS
	/* `idx` should be between [0, 15] */
	mov	r1, r0
	lsr	r1, r1, #4
	cmp	r1, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of ldcopr16/bx lr instruction pair
	 * in the table below.
	 */
	adr	r1, 1f
	lsl	r0, r0, #3	/* each stcopr16/bx lr sequence is 8 bytes */
	add	r1, r1, r0
	bx	r1

1:
	stcopr16	r2, r3,	AMEVCNTR10	/* index 0 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR11	/* index 1 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR12	/* index 2 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR13	/* index 3 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR14	/* index 4 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR15	/* index 5 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR16	/* index 6 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR17	/* index 7 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR18	/* index 8 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR19	/* index 9 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR1A	/* index 10 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR1B	/* index 11 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR1C	/* index 12 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR1D	/* index 13 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR1E	/* index 14 */
	bx		lr
	stcopr16	r2, r3,	AMEVCNTR1F	/* index 15 */
	bx		lr
endfunc amu_group1_cnt_write_internal

/*
 * void amu_group1_set_evtype_internal(int idx, unsigned int val);
 *
 * Program the AMU event type register indexed by `idx`
 * with the value `val`.
 */
func amu_group1_set_evtype_internal
#if ENABLE_ASSERTIONS
	/* `idx` should be between [0, 15] */
	mov	r2, r0
	lsr	r2, r2, #4
	cmp	r2, #0
	ASM_ASSERT(eq)

	/* val should be between [0, 65535] */
	mov	r2, r1
	lsr	r2, r2, #16
	cmp	r2, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of stcopr/bx lr instruction pair
	 * in the table below.
	 */
	adr	r2, 1f
	lsl	r0, r0, #3	/* each stcopr/bx lr sequence is 8 bytes */
	add	r2, r2, r0
	bx	r2

1:
	stcopr	r1,	AMEVTYPER10 /* index 0 */
	bx	lr
	stcopr	r1,	AMEVTYPER11 /* index 1 */
	bx	lr
	stcopr	r1,	AMEVTYPER12 /* index 2 */
	bx	lr
	stcopr	r1,	AMEVTYPER13 /* index 3 */
	bx	lr
	stcopr	r1,	AMEVTYPER14 /* index 4 */
	bx	lr
	stcopr	r1,	AMEVTYPER15 /* index 5 */
	bx	lr
	stcopr	r1,	AMEVTYPER16 /* index 6 */
	bx	lr
	stcopr	r1,	AMEVTYPER17 /* index 7 */
	bx	lr
	stcopr	r1,	AMEVTYPER18 /* index 8 */
	bx	lr
	stcopr	r1,	AMEVTYPER19 /* index 9 */
	bx	lr
	stcopr	r1,	AMEVTYPER1A /* index 10 */
	bx	lr
	stcopr	r1,	AMEVTYPER1B /* index 11 */
	bx	lr
	stcopr	r1,	AMEVTYPER1C /* index 12 */
	bx	lr
	stcopr	r1,	AMEVTYPER1D /* index 13 */
	bx	lr
	stcopr	r1,	AMEVTYPER1E /* index 14 */
	bx	lr
	stcopr	r1,	AMEVTYPER1F /* index 15 */
	bx	lr
endfunc amu_group1_set_evtype_internal
