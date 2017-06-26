/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	flush_dcache_range
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
.macro do_dcache_maintenance_by_mva op, coproc, opc1, CRn, CRm, opc2
	/* Exit early if size is zero */
	cmp	r1, #0
	beq	exit_loop_\op
	dcache_line_size r2, r3
	add	r1, r0, r1
	sub	r3, r2, #1
	bic	r0, r0, r3
loop_\op:
	stcopr	r0, \coproc, \opc1, \CRn, \CRm, \opc2
	add	r0, r0, r2
	cmp	r0, r1
	blo	loop_\op
	dsb	sy
exit_loop_\op:
	bx	lr
.endm

	/* ------------------------------------------
	 * Clean+Invalidate from base address till
	 * size. 'r0' = addr, 'r1' = size
	 * ------------------------------------------
	 */
func flush_dcache_range
	do_dcache_maintenance_by_mva cimvac, DCCIMVAC
endfunc flush_dcache_range

	/* ------------------------------------------
	 * Clean from base address till size.
	 * 'r0' = addr, 'r1' = size
	 * ------------------------------------------
	 */
func clean_dcache_range
	do_dcache_maintenance_by_mva cmvac, DCCMVAC
endfunc clean_dcache_range

	/* ------------------------------------------
	 * Invalidate from base address till
	 * size. 'r0' = addr, 'r1' = size
	 * ------------------------------------------
	 */
func inv_dcache_range
	do_dcache_maintenance_by_mva imvac, DCIMVAC
endfunc inv_dcache_range

	/* ----------------------------------------------------------------
	 * Data cache operations by set/way to the level specified
	 *
	 * The main function, do_dcsw_op requires:
	 * r0: The operation type (DC_OP_ISW, DC_OP_CISW, DC_OP_CSW),
	 * as defined in arch.h
	 * r1: The cache level to begin operation from
	 * r2: clidr_el1
	 * r3: The last cache level to operate on
	 * and will carry out the operation on each data cache from level 0
	 * to the level in r3 in sequence
	 *
	 * The dcsw_op macro sets up the r2 and r3 parameters based on
	 * clidr_el1 cache information before invoking the main function
	 * ----------------------------------------------------------------
	 */

	.macro	dcsw_op shift, fw, ls
	ldcopr	r2, CLIDR
	ubfx	r3, r2, \shift, \fw
	lsl	r3, r3, \ls
	mov	r1, #0
	b	do_dcsw_op
	.endm

func do_dcsw_op
	push	{r4-r12,lr}
	adr	r11, dcsw_loop_table	// compute cache op based on the operation type
	add	r6, r11, r0, lsl #3	// cache op is 2x32-bit instructions
loop1:
	add	r10, r1, r1, LSR #1	// Work out 3x current cache level
	mov	r12, r2, LSR r10	// extract cache type bits from clidr
	and	r12, r12, #7   		// mask the bits for current cache only
	cmp	r12, #2			// see what cache we have at this level
	blo	level_done      	// no cache or only instruction cache at this level

	stcopr	r1, CSSELR		// select current cache level in csselr
	isb				// isb to sych the new cssr&csidr
	ldcopr	r12, CCSIDR		// read the new ccsidr
	and	r10, r12, #7   		// extract the length of the cache lines
	add	r10, r10, #4        	// add 4 (r10 = line length offset)
	ubfx	r4, r12, #3, #10	// r4 = maximum way number (right aligned)
	clz	r5, r4            	// r5 = the bit position of the way size increment
	mov	r9, r4			// r9 working copy of the aligned max way number

loop2:
	ubfx	r7, r12, #13, #15	// r7 = max set number (right aligned)

loop3:
	orr	r0, r1, r9, LSL r5	// factor in the way number and cache level into r0
	orr	r0, r0, r7, LSL r10	// factor in the set number

	blx	r6
	subs	r7, r7, #1              // decrement the set number
	bhs	loop3
	subs	r9, r9, #1              // decrement the way number
	bhs	loop2
level_done:
	add	r1, r1, #2		// increment the cache number
	cmp	r3, r1
	dsb	sy			// ensure completion of previous cache maintenance instruction
	bhi	loop1

	mov	r6, #0
	stcopr	r6, CSSELR		//select cache level 0 in csselr
	dsb	sy
	isb
	pop	{r4-r12,pc}

dcsw_loop_table:
	stcopr	r0, DCISW
	bx	lr
	stcopr	r0, DCCISW
	bx	lr
	stcopr	r0, DCCSW
	bx	lr

endfunc do_dcsw_op

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way till PoU.
	 *
	 * The function requires :
	 * r0: The operation type (DC_OP_ISW, DC_OP_CISW, DC_OP_CSW),
	 * as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_louis
	dcsw_op #LOUIS_SHIFT, #CLIDR_FIELD_WIDTH, #LEVEL_SHIFT
endfunc	dcsw_op_louis

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way till PoC.
	 *
	 * The function requires :
	 * r0: The operation type (DC_OP_ISW, DC_OP_CISW, DC_OP_CSW),
	 * as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_all
	dcsw_op #LOC_SHIFT, #CLIDR_FIELD_WIDTH, #LEVEL_SHIFT
endfunc	dcsw_op_all


	/* ---------------------------------------------------------------
	 *  Helper macro for data cache operations by set/way for the
	 *  level specified
	 * ---------------------------------------------------------------
	 */
	.macro	dcsw_op_level level
	ldcopr	r2, CLIDR
	mov	r3, \level
	sub	r1, r3, #2
	b	do_dcsw_op
	.endm

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way for level 1 cache
	 *
	 * The main function, do_dcsw_op requires:
	 * r0: The operation type (DC_OP_ISW, DC_OP_CISW, DC_OP_CSW),
	 * as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_level1
	dcsw_op_level #(1 << LEVEL_SHIFT)
endfunc dcsw_op_level1

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way for level 2 cache
	 *
	 * The main function, do_dcsw_op requires:
	 * r0: The operation type (DC_OP_ISW, DC_OP_CISW, DC_OP_CSW),
	 * as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_level2
	dcsw_op_level #(2 << LEVEL_SHIFT)
endfunc dcsw_op_level2

	/* ---------------------------------------------------------------
	 * Data cache operations by set/way for level 3 cache
	 *
	 * The main function, do_dcsw_op requires:
	 * r0: The operation type (DC_OP_ISW, DC_OP_CISW, DC_OP_CSW),
	 * as defined in arch.h
	 * ---------------------------------------------------------------
	 */
func dcsw_op_level3
	dcsw_op_level #(3 << LEVEL_SHIFT)
endfunc dcsw_op_level3
