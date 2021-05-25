/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
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

	/* FEAT_AMUv1p1 virtualisation offset register functions */
	.globl	amu_group0_voffset_read_internal
	.globl	amu_group0_voffset_write_internal
	.globl	amu_group1_voffset_read_internal
	.globl	amu_group1_voffset_write_internal

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

#if ENABLE_AMU_AUXILIARY_COUNTERS
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
#endif

/*
 * Accessor functions for virtual offset registers added with FEAT_AMUv1p1
 */

/*
 * uint64_t amu_group0_voffset_read_internal(int idx);
 *
 * Given `idx`, read the corresponding AMU virtual offset register
 * and return it in `x0`.
 */
func amu_group0_voffset_read_internal
	adr	x1, 1f
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	tst	x0, #~3
	ASM_ASSERT(eq)
	/* Make sure idx != 1 since AMEVCNTVOFF01_EL2 does not exist */
	cmp	x0, #1
	ASM_ASSERT(ne)
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

1:	read	AMEVCNTVOFF00_EL2	/* index 0 */
	.skip	8			/* AMEVCNTVOFF01_EL2 does not exist */
#if ENABLE_BTI
	.skip	4
#endif
	read	AMEVCNTVOFF02_EL2	/* index 2 */
	read	AMEVCNTVOFF03_EL2	/* index 3 */
endfunc amu_group0_voffset_read_internal

/*
 * void amu_group0_voffset_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU virtual offset register.
 */
func amu_group0_voffset_write_internal
	adr	x2, 1f
#if ENABLE_ASSERTIONS
	/*
	 * It can be dangerous to call this function with an
	 * out of bounds index.  Ensure `idx` is valid.
	 */
	tst	x0, #~3
	ASM_ASSERT(eq)
	/* Make sure idx != 1 since AMEVCNTVOFF01_EL2 does not exist */
	cmp	x0, #1
	ASM_ASSERT(ne)
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

1:	write	AMEVCNTVOFF00_EL2	/* index 0 */
	.skip	8			/* AMEVCNTVOFF01_EL2 does not exist */
#if ENABLE_BTI
	.skip	4
#endif
	write	AMEVCNTVOFF02_EL2	/* index 2 */
	write	AMEVCNTVOFF03_EL2	/* index 3 */
endfunc amu_group0_voffset_write_internal

#if ENABLE_AMU_AUXILIARY_COUNTERS
/*
 * uint64_t amu_group1_voffset_read_internal(int idx);
 *
 * Given `idx`, read the corresponding AMU virtual offset register
 * and return it in `x0`.
 */
func amu_group1_voffset_read_internal
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

1:	read	AMEVCNTVOFF10_EL2	/* index 0 */
	read	AMEVCNTVOFF11_EL2	/* index 1 */
	read	AMEVCNTVOFF12_EL2	/* index 2 */
	read	AMEVCNTVOFF13_EL2	/* index 3 */
	read	AMEVCNTVOFF14_EL2	/* index 4 */
	read	AMEVCNTVOFF15_EL2	/* index 5 */
	read	AMEVCNTVOFF16_EL2	/* index 6 */
	read	AMEVCNTVOFF17_EL2	/* index 7 */
	read	AMEVCNTVOFF18_EL2	/* index 8 */
	read	AMEVCNTVOFF19_EL2	/* index 9 */
	read	AMEVCNTVOFF1A_EL2	/* index 10 */
	read	AMEVCNTVOFF1B_EL2	/* index 11 */
	read	AMEVCNTVOFF1C_EL2	/* index 12 */
	read	AMEVCNTVOFF1D_EL2	/* index 13 */
	read	AMEVCNTVOFF1E_EL2	/* index 14 */
	read	AMEVCNTVOFF1F_EL2	/* index 15 */
endfunc amu_group1_voffset_read_internal

/*
 * void amu_group1_voffset_write_internal(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU virtual offset register.
 */
func amu_group1_voffset_write_internal
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

1:	write	AMEVCNTVOFF10_EL2	/* index 0 */
	write	AMEVCNTVOFF11_EL2	/* index 1 */
	write	AMEVCNTVOFF12_EL2	/* index 2 */
	write	AMEVCNTVOFF13_EL2	/* index 3 */
	write	AMEVCNTVOFF14_EL2	/* index 4 */
	write	AMEVCNTVOFF15_EL2	/* index 5 */
	write	AMEVCNTVOFF16_EL2	/* index 6 */
	write	AMEVCNTVOFF17_EL2	/* index 7 */
	write	AMEVCNTVOFF18_EL2	/* index 8 */
	write	AMEVCNTVOFF19_EL2	/* index 9 */
	write	AMEVCNTVOFF1A_EL2	/* index 10 */
	write	AMEVCNTVOFF1B_EL2	/* index 11 */
	write	AMEVCNTVOFF1C_EL2	/* index 12 */
	write	AMEVCNTVOFF1D_EL2	/* index 13 */
	write	AMEVCNTVOFF1E_EL2	/* index 14 */
	write	AMEVCNTVOFF1F_EL2	/* index 15 */
endfunc amu_group1_voffset_write_internal
#endif
