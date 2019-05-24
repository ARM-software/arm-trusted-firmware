/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
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
	adr	x1, 1f
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	tst	x0, #~3
	ASM_ASSERT(eq)
#endif
	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	add	x1, x1, x0, lsl #3	/* each mrs/ret sequence is 8 bytes */
#if ENABLE_BTI
	add	x1, x1, x0, lsl #2	/* + "bti j" instruction */
#endif
	br	x1

1:	read	AMEVCNTR00_EL0		/* index 0 */
	read	AMEVCNTR01_EL0		/* index 1 */
	read	AMEVCNTR02_EL0		/* index 2 */
	read	AMEVCNTR03_EL0		/* index 3 */
endfunc amu_group0_cnt_read_internal

/*
 * void amu_group0_cnt_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 */
func amu_group0_cnt_write_internal
	adr	x2, 1f
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	tst	x0, #~3
	ASM_ASSERT(eq)
#endif
	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	add	x2, x2, x0, lsl #3	/* each msr/ret sequence is 8 bytes */
#if ENABLE_BTI
	add	x2, x2, x0, lsl #2	/* + "bti j" instruction */
#endif
	br	x2

1:	write	AMEVCNTR00_EL0		/* index 0 */
	write	AMEVCNTR01_EL0		/* index 1 */
	write	AMEVCNTR02_EL0		/* index 2 */
	write	AMEVCNTR03_EL0		/* index 3 */
endfunc amu_group0_cnt_write_internal

/*
 * uint64_t amu_group1_cnt_read_internal(int idx);
 *
 * Given `idx`, read the corresponding AMU counter
 * and return it in `x0`.
 */
func amu_group1_cnt_read_internal
	adr	x1, 1f
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	tst	x0, #~0xF
	ASM_ASSERT(eq)
#endif
	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	add	x1, x1, x0, lsl #3	/* each mrs/ret sequence is 8 bytes */
#if ENABLE_BTI
	add	x1, x1, x0, lsl #2	/* + "bti j" instruction */
#endif
	br	x1

1:	read	AMEVCNTR10_EL0		/* index 0 */
	read	AMEVCNTR11_EL0		/* index 1 */
	read	AMEVCNTR12_EL0		/* index 2 */
	read	AMEVCNTR13_EL0		/* index 3 */
	read	AMEVCNTR14_EL0		/* index 4 */
	read	AMEVCNTR15_EL0		/* index 5 */
	read	AMEVCNTR16_EL0		/* index 6 */
	read	AMEVCNTR17_EL0		/* index 7 */
	read	AMEVCNTR18_EL0		/* index 8 */
	read	AMEVCNTR19_EL0		/* index 9 */
	read	AMEVCNTR1A_EL0		/* index 10 */
	read	AMEVCNTR1B_EL0		/* index 11 */
	read	AMEVCNTR1C_EL0		/* index 12 */
	read	AMEVCNTR1D_EL0		/* index 13 */
	read	AMEVCNTR1E_EL0		/* index 14 */
	read	AMEVCNTR1F_EL0		/* index 15 */
endfunc amu_group1_cnt_read_internal

/*
 * void amu_group1_cnt_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 */
func amu_group1_cnt_write_internal
	adr	x2, 1f
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	tst	x0, #~0xF
	ASM_ASSERT(eq)
#endif
	/*
	 * Given `idx` calculate address of mrs/ret instruction pair
	 * in the table below.
	 */
	add	x2, x2, x0, lsl #3	/* each msr/ret sequence is 8 bytes */
#if ENABLE_BTI
	add	x2, x2, x0, lsl #2	/* + "bti j" instruction */
#endif
	br	x2

1:	write	AMEVCNTR10_EL0		/* index 0 */
	write	AMEVCNTR11_EL0		/* index 1 */
	write	AMEVCNTR12_EL0		/* index 2 */
	write	AMEVCNTR13_EL0		/* index 3 */
	write	AMEVCNTR14_EL0		/* index 4 */
	write	AMEVCNTR15_EL0		/* index 5 */
	write	AMEVCNTR16_EL0		/* index 6 */
	write	AMEVCNTR17_EL0		/* index 7 */
	write	AMEVCNTR18_EL0		/* index 8 */
	write	AMEVCNTR19_EL0		/* index 9 */
	write	AMEVCNTR1A_EL0		/* index 10 */
	write	AMEVCNTR1B_EL0		/* index 11 */
	write	AMEVCNTR1C_EL0		/* index 12 */
	write	AMEVCNTR1D_EL0		/* index 13 */
	write	AMEVCNTR1E_EL0		/* index 14 */
	write	AMEVCNTR1F_EL0		/* index 15 */
endfunc amu_group1_cnt_write_internal

/*
 * void amu_group1_set_evtype_internal(int idx, unsigned int val);
 *
 * Program the AMU event type register indexed by `idx`
 * with the value `val`.
 */
func amu_group1_set_evtype_internal
	adr	x2, 1f
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	tst	x0, #~0xF
	ASM_ASSERT(eq)

	/* val should be between [0, 65535] */
	tst	x1, #~0xFFFF
	ASM_ASSERT(eq)
#endif
	/*
	 * Given `idx` calculate address of msr/ret instruction pair
	 * in the table below.
	 */
	add	x2, x2, x0, lsl #3	/* each msr/ret sequence is 8 bytes */
#if ENABLE_BTI
	add	x2, x2, x0, lsl #2	/* + "bti j" instruction */
#endif
	br	x2

1:	write	AMEVTYPER10_EL0		/* index 0 */
	write	AMEVTYPER11_EL0		/* index 1 */
	write	AMEVTYPER12_EL0		/* index 2 */
	write	AMEVTYPER13_EL0		/* index 3 */
	write	AMEVTYPER14_EL0		/* index 4 */
	write	AMEVTYPER15_EL0		/* index 5 */
	write	AMEVTYPER16_EL0		/* index 6 */
	write	AMEVTYPER17_EL0		/* index 7 */
	write	AMEVTYPER18_EL0		/* index 8 */
	write	AMEVTYPER19_EL0		/* index 9 */
	write	AMEVTYPER1A_EL0		/* index 10 */
	write	AMEVTYPER1B_EL0		/* index 11 */
	write	AMEVTYPER1C_EL0		/* index 12 */
	write	AMEVTYPER1D_EL0		/* index 13 */
	write	AMEVTYPER1E_EL0		/* index 14 */
	write	AMEVTYPER1F_EL0		/* index 15 */
endfunc amu_group1_set_evtype_internal
