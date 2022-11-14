/*
 * Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	flush_dcache_range
	.globl	flush_dcache_to_popa_range
	.globl	clean_dcache_range
	.globl	inv_dcache_range
	.globl	dcsw_op_louis
	.globl	dcsw_op_all
	.globl	dcsw_op_level1
	.globl	dcsw_op_level2
	.globl	dcsw_op_level3

/*
 * This macro can be used for implementing various data cache operations `op`
 */
.macro do_dcache_maintenance_by_mva op
	/* Exit early if size is zero */
	cbz	x1, exit_loop_\op
	dcache_line_size x2, x3
	add	x1, x0, x1
	sub	x3, x2, #1
	bic	x0, x0, x3
loop_\op:
	dc	\op, x0
	add	x0, x0, x2
	cmp	x0, x1
	b.lo	loop_\op
	dsb	sy
exit_loop_\op:
	ret
.endm

.macro check_plat_can_cmo
#if CONDITIONAL_CMO
	mov	x3, x30
	mov	x2, x0
	bl	plat_can_cmo
	mov	x30, x3
	cbnz	x0, 1f
	ret
1:
	mov	 x0, x2
#endif
.endm
	/* ------------------------------------------
	 * Clean+Invalidate from base address till
	 * size. 'x0' = addr, 'x1' = size
	 * ------------------------------------------
	 */
func flush_dcache_range
	check_plat_can_cmo
	do_dcache_maintenance_by_mva civac
endfunc flush_dcache_range

	/* ------------------------------------------
	 * Clean from base address till size.
	 * 'x0' = addr, 'x1' = size
	 * ------------------------------------------
	 */
func clean_dcache_range
	check_plat_can_cmo
	do_dcache_maintenance_by_mva cvac
endfunc clean_dcache_range

	/* ------------------------------------------
	 * Invalidate from base address till
	 * size. 'x0' = addr, 'x1' = size
	 * ------------------------------------------
	 */
func inv_dcache_range
	check_plat_can_cmo
	do_dcache_maintenance_by_mva ivac
endfunc inv_dcache_range


	/*
	 * On implementations with FEAT_MTE2,
	 * Root firmware must issue DC_CIGDPAPA instead of DC_CIPAPA ,
	 * in order to additionally clean and invalidate Allocation Tags
	 * associated with the affected locations.
	 *
	 * ------------------------------------------
	 * Clean+Invalidate by PA to POPA
	 * from base address till size.
	 * 'x0' = addr, 'x1' = size
	 * ------------------------------------------
	 */
func flush_dcache_to_popa_range
	/* Exit early if size is zero */
	cbz	x1, exit_loop_dc_cipapa
	check_plat_can_cmo
	dcache_line_size x2, x3
	sub	x3, x2, #1
	bic	x0, x0, x3
	add	x1, x1, x0
loop_dc_cipapa:
	sys	#6, c7, c14, #1, x0 /* DC CIPAPA,<Xt> */
	add	x0, x0, x2
	cmp	x0, x1
	b.lo	loop_dc_cipapa
	dsb	osh
exit_loop_dc_cipapa:
	ret
endfunc	flush_dcache_to_popa_range

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way to the level specified
	 *
	 * The main function, do_dcsw_op requires:
	 * x0: The operation type (0-2), as defined in arch.h
	 * x3: The last cache level to operate on
	 * x9: clidr_el1
	 * x10: The cache level to begin operation from
	 * and will carry out the operation on each data cache from level 0
	 * to the level in x3 in sequence
	 *
	 * The dcsw_op macro sets up the x3 and x9 parameters based on
	 * clidr_el1 cache information before invoking the main function
	 * ---------------------------------------------------------------
	 */

	.macro	dcsw_op shift, fw, ls
	mrs	x9, clidr_el1
	ubfx	x3, x9, \shift, \fw
	lsl	x3, x3, \ls
	mov	x10, xzr
	b	do_dcsw_op
	.endm

func do_dcsw_op
	cbz	x3, exit
	mrs	x12, ID_AA64MMFR2_EL1	// stash FEAT_CCIDX identifier in x12
	ubfx	x12, x12, #ID_AA64MMFR2_EL1_CCIDX_SHIFT, #ID_AA64MMFR2_EL1_CCIDX_LENGTH
	adr	x14, dcsw_loop_table	// compute inner loop address
	add	x14, x14, x0, lsl #5	// inner loop is 8x32-bit instructions
#if ENABLE_BTI
	add	x14, x14, x0, lsl #2	// inner loop is + "bti j" instruction
#endif
	mov	x0, x9
	mov	w8, #1
loop1:
	add	x2, x10, x10, lsr #1	// work out 3x current cache level
	lsr	x1, x0, x2		// extract cache type bits from clidr
	and	x1, x1, #7		// mask the bits for current cache only
	cmp	x1, #2			// see what cache we have at this level
	b.lo	level_done		// nothing to do if no cache or icache

	msr	csselr_el1, x10		// select current cache level in csselr
	isb				// isb to sych the new cssr&csidr
	mrs	x1, ccsidr_el1		// read the new ccsidr
	and	x2, x1, #7		// extract the length of the cache lines
	add	x2, x2, #4		// add 4 (line length offset)

	cbz	x12, 1f			// check for FEAT_CCIDX for Associativity
	ubfx	x4, x1, #3, #21 	// x4 = associativity CCSIDR_EL1[23:3]
	b 	2f
1:
	ubfx	x4, x1, #3, #10 	// x4 = associativity CCSIDR_EL1[12:3]
2:
	clz	w5, w4			// bit position of way size increment
	lsl	w9, w4, w5		// w9 = aligned max way number
	lsl	w16, w8, w5		// w16 = way number loop decrement
	orr	w9, w10, w9		// w9 = combine way and cache number

	cbz	x12, 3f			// check for FEAT_CCIDX for NumSets
	ubfx	x6, x1, #32, #24	// x6 (w6) = numsets CCSIDR_EL1[55:32]
					// ISA will not allow x->w ubfx
	b	4f
3:
	ubfx	w6, w1, #13, #15	// w6 = numsets CCSIDR_EL1[27:13]
4:
	lsl	w17, w8, w2		// w17 = set number loop decrement
	dsb	sy			// barrier before we start this level
	br	x14			// jump to DC operation specific loop

	.macro	dcsw_loop _op
#if ENABLE_BTI
	bti	j
#endif
loop2_\_op:
	lsl	w7, w6, w2		// w7 = aligned max set number

loop3_\_op:
	orr	w11, w9, w7		// combine cache, way and set number
	dc	\_op, x11
	subs	w7, w7, w17		// decrement set number
	b.hs	loop3_\_op

	subs	x9, x9, x16		// decrement way number
	b.hs	loop2_\_op

	b	level_done
	.endm

level_done:
	add	x10, x10, #2		// increment cache number
	cmp	x3, x10
	b.hi	loop1
	msr	csselr_el1, xzr		// select cache level 0 in csselr
	dsb	sy			// barrier to complete final cache operation
	isb
exit:
	ret
endfunc do_dcsw_op

dcsw_loop_table:
	dcsw_loop isw
	dcsw_loop cisw
	dcsw_loop csw


func dcsw_op_louis
	check_plat_can_cmo
	dcsw_op #LOUIS_SHIFT, #CLIDR_FIELD_WIDTH, #LEVEL_SHIFT
endfunc dcsw_op_louis


func dcsw_op_all
	check_plat_can_cmo
	dcsw_op #LOC_SHIFT, #CLIDR_FIELD_WIDTH, #LEVEL_SHIFT
endfunc dcsw_op_all

	/* ---------------------------------------------------------------
	 *  Helper macro for data cache operations by set/way for the
	 *  level specified
	 * ---------------------------------------------------------------
	 */
	.macro dcsw_op_level level
	mrs	x9, clidr_el1
	mov	x3, \level
	sub	x10, x3, #2
	b	do_dcsw_op
	.endm

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way for level 1 cache
	 *
	 * The main function, do_dcsw_op requires:
	 * x0: The operation type (0-2), as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_level1
	check_plat_can_cmo
	dcsw_op_level #(1 << LEVEL_SHIFT)
endfunc dcsw_op_level1

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way for level 2 cache
	 *
	 * The main function, do_dcsw_op requires:
	 * x0: The operation type (0-2), as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_level2
	check_plat_can_cmo
	dcsw_op_level #(2 << LEVEL_SHIFT)
endfunc dcsw_op_level2

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way for level 3 cache
	 *
	 * The main function, do_dcsw_op requires:
	 * x0: The operation type (0-2), as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_level3
	check_plat_can_cmo
	dcsw_op_level #(3 << LEVEL_SHIFT)
endfunc dcsw_op_level3
