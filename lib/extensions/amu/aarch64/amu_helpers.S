/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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
 * and return it in `x0`.
 */
func amu_group0_cnt_read_internal
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	mov	x1, x0
	lsr	x1, x1, #2
	cmp	x1, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	adr	x1, 1f
	lsl	x0, x0, #3		/* each mrs/ret sequence is 8 bytes */
	add	x1, x1, x0
	br	x1

1:
	mrs	x0, AMEVCNTR00_EL0	/* index 0 */
	ret
	mrs	x0, AMEVCNTR01_EL0	/* index 1 */
	ret
	mrs	x0, AMEVCNTR02_EL0	/* index 2 */
	ret
	mrs	x0, AMEVCNTR03_EL0	/* index 3 */
	ret
endfunc amu_group0_cnt_read_internal

/*
 * void amu_group0_cnt_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 */
func amu_group0_cnt_write_internal
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	mov	x2, x0
	lsr	x2, x2, #2
	cmp	x2, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	adr	x2, 1f
	lsl	x0, x0, #3		/* each msr/ret sequence is 8 bytes */
	add	x2, x2, x0
	br	x2

1:
	msr	AMEVCNTR00_EL0, x1	/* index 0 */
	ret
	msr	AMEVCNTR01_EL0, x1	/* index 1 */
	ret
	msr	AMEVCNTR02_EL0, x1	/* index 2 */
	ret
	msr	AMEVCNTR03_EL0, x1	/* index 3 */
	ret
endfunc amu_group0_cnt_write_internal

/*
 * uint64_t amu_group1_cnt_read_internal(int idx);
 *
 * Given `idx`, read the corresponding AMU counter
 * and return it in `x0`.
 */
func amu_group1_cnt_read_internal
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	mov	x1, x0
	lsr	x1, x1, #4
	cmp	x1, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	adr	x1, 1f
	lsl	x0, x0, #3		/* each mrs/ret sequence is 8 bytes */
	add	x1, x1, x0
	br	x1

1:
	mrs	x0, AMEVCNTR10_EL0	/* index 0 */
	ret
	mrs	x0, AMEVCNTR11_EL0	/* index 1 */
	ret
	mrs	x0, AMEVCNTR12_EL0	/* index 2 */
	ret
	mrs	x0, AMEVCNTR13_EL0	/* index 3 */
	ret
	mrs	x0, AMEVCNTR14_EL0	/* index 4 */
	ret
	mrs	x0, AMEVCNTR15_EL0	/* index 5 */
	ret
	mrs	x0, AMEVCNTR16_EL0	/* index 6 */
	ret
	mrs	x0, AMEVCNTR17_EL0	/* index 7 */
	ret
	mrs	x0, AMEVCNTR18_EL0	/* index 8 */
	ret
	mrs	x0, AMEVCNTR19_EL0	/* index 9 */
	ret
	mrs	x0, AMEVCNTR1A_EL0	/* index 10 */
	ret
	mrs	x0, AMEVCNTR1B_EL0	/* index 11 */
	ret
	mrs	x0, AMEVCNTR1C_EL0	/* index 12 */
	ret
	mrs	x0, AMEVCNTR1D_EL0	/* index 13 */
	ret
	mrs	x0, AMEVCNTR1E_EL0	/* index 14 */
	ret
	mrs	x0, AMEVCNTR1F_EL0	/* index 15 */
	ret
endfunc amu_group1_cnt_read_internal

/*
 * void amu_group1_cnt_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 */
func amu_group1_cnt_write_internal
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	mov	x2, x0
	lsr	x2, x2, #4
	cmp	x2, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	adr	x2, 1f
	lsl	x0, x0, #3		/* each msr/ret sequence is 8 bytes */
	add	x2, x2, x0
	br	x2

1:
	msr	AMEVCNTR10_EL0, x1	/* index 0 */
	ret
	msr	AMEVCNTR11_EL0, x1	/* index 1 */
	ret
	msr	AMEVCNTR12_EL0, x1	/* index 2 */
	ret
	msr	AMEVCNTR13_EL0, x1	/* index 3 */
	ret
	msr	AMEVCNTR14_EL0, x1	/* index 4 */
	ret
	msr	AMEVCNTR15_EL0, x1	/* index 5 */
	ret
	msr	AMEVCNTR16_EL0, x1	/* index 6 */
	ret
	msr	AMEVCNTR17_EL0, x1	/* index 7 */
	ret
	msr	AMEVCNTR18_EL0, x1	/* index 8 */
	ret
	msr	AMEVCNTR19_EL0, x1	/* index 9 */
	ret
	msr	AMEVCNTR1A_EL0, x1	/* index 10 */
	ret
	msr	AMEVCNTR1B_EL0, x1	/* index 11 */
	ret
	msr	AMEVCNTR1C_EL0, x1	/* index 12 */
	ret
	msr	AMEVCNTR1D_EL0, x1	/* index 13 */
	ret
	msr	AMEVCNTR1E_EL0, x1	/* index 14 */
	ret
	msr	AMEVCNTR1F_EL0, x1	/* index 15 */
	ret
endfunc amu_group1_cnt_write_internal

/*
 * void amu_group1_set_evtype_internal(int idx, unsigned int val);
 *
 * Program the AMU event type register indexed by `idx`
 * with the value `val`.
 */
func amu_group1_set_evtype_internal
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	mov	x2, x0
	lsr	x2, x2, #4
	cmp	x2, #0
	ASM_ASSERT(eq)

	/* val should be between [0, 65535] */
	mov	x2, x1
	lsr	x2, x2, #16
	cmp	x2, #0
	ASM_ASSERT(eq)
#endif

	/*
	 * Given `idx` calculate address of msr/ret instruction pair
	 * in the table below.
	 */
	adr	x2, 1f
	lsl	x0, x0, #3		/* each msr/ret sequence is 8 bytes */
	add	x2, x2, x0
	br	x2

1:
	msr	AMEVTYPER10_EL0, x1	/* index 0 */
	ret
	msr	AMEVTYPER11_EL0, x1	/* index 1 */
	ret
	msr	AMEVTYPER12_EL0, x1	/* index 2 */
	ret
	msr	AMEVTYPER13_EL0, x1	/* index 3 */
	ret
	msr	AMEVTYPER14_EL0, x1	/* index 4 */
	ret
	msr	AMEVTYPER15_EL0, x1	/* index 5 */
	ret
	msr	AMEVTYPER16_EL0, x1	/* index 6 */
	ret
	msr	AMEVTYPER17_EL0, x1	/* index 7 */
	ret
	msr	AMEVTYPER18_EL0, x1	/* index 8 */
	ret
	msr	AMEVTYPER19_EL0, x1	/* index 9 */
	ret
	msr	AMEVTYPER1A_EL0, x1	/* index 10 */
	ret
	msr	AMEVTYPER1B_EL0, x1	/* index 11 */
	ret
	msr	AMEVTYPER1C_EL0, x1	/* index 12 */
	ret
	msr	AMEVTYPER1D_EL0, x1	/* index 13 */
	ret
	msr	AMEVTYPER1E_EL0, x1	/* index 14 */
	ret
	msr	AMEVTYPER1F_EL0, x1	/* index 15 */
	ret
endfunc amu_group1_set_evtype_internal
