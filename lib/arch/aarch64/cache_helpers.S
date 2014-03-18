/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <asm_macros.S>

	.globl	dcisw
	.globl	dccisw
	.globl	dccsw
	.globl	dccvac
	.globl	dcivac
	.globl	dccivac
	.globl	dccvau
	.globl	dczva
	.globl	flush_dcache_range
	.globl	inv_dcache_range
	.globl	dcsw_op_louis
	.globl	dcsw_op_all

func dcisw
	dc	isw, x0
	dsb	sy
	isb
	ret


func dccisw
	dc	cisw, x0
	dsb	sy
	isb
	ret


func dccsw
	dc	csw, x0
	dsb	sy
	isb
	ret


func dccvac
	dc	cvac, x0
	dsb	sy
	isb
	ret


func dcivac
	dc	ivac, x0
	dsb	sy
	isb
	ret


func dccivac
	dc	civac, x0
	dsb	sy
	isb
	ret


func dccvau
	dc	cvau, x0
	dsb	sy
	isb
	ret


func dczva
	dc	zva, x0
	dsb	sy
	isb
	ret


	/* ------------------------------------------
	 * Clean+Invalidate from base address till
	 * size. 'x0' = addr, 'x1' = size
	 * ------------------------------------------
	 */
func flush_dcache_range
	dcache_line_size x2, x3
	add	x1, x0, x1
	sub	x3, x2, #1
	bic	x0, x0, x3
flush_loop:
	dc	civac, x0
	add	x0, x0, x2
	cmp	x0, x1
	b.lo    flush_loop
	dsb	sy
	ret


	/* ------------------------------------------
	 * Invalidate from base address till
	 * size. 'x0' = addr, 'x1' = size
	 * ------------------------------------------
	 */
func inv_dcache_range
	dcache_line_size x2, x3
	add	x1, x0, x1
	sub	x3, x2, #1
	bic	x0, x0, x3
inv_loop:
	dc	ivac, x0
	add	x0, x0, x2
	cmp	x0, x1
	b.lo    inv_loop
	dsb	sy
	ret


	/* ------------------------------------------
	 * Data cache operations by set/way to the
	 * level specified
	 * ------------------------------------------
	 * ----------------------------------
	 * Call this func with the clidr in
	 * x0, starting cache level in x10,
	 * last cache level in x3 & cm op in
	 * x14
	 * ----------------------------------
	 */
func dcsw_op
all_start_at_level:
	add	x2, x10, x10, lsr #1            // work out 3x current cache level
	lsr	x1, x0, x2                      // extract cache type bits from clidr
	and	x1, x1, #7                      // mask of the bits for current cache only
	cmp	x1, #2                          // see what cache we have at this level
	b.lt	skip                            // skip if no cache, or just i-cache
	msr	csselr_el1, x10                 // select current cache level in csselr
	isb                                     // isb to sych the new cssr&csidr
	mrs	x1, ccsidr_el1                  // read the new ccsidr
	and	x2, x1, #7                      // extract the length of the cache lines
	add	x2, x2, #4                      // add 4 (line length offset)
	mov	x4, #0x3ff
	and	x4, x4, x1, lsr #3              // find maximum number on the way size
	clz	w5, w4                          // find bit position of way size increment
	mov	x7, #0x7fff
	and	x7, x7, x1, lsr #13             // extract max number of the index size
loop2:
	mov	x9, x4                          // create working copy of max way size
loop3:
	lsl	x6, x9, x5
	orr	x11, x10, x6                    // factor way and cache number into x11
	lsl	x6, x7, x2
	orr	x11, x11, x6                    // factor index number into x11
	mov	x12, x0
	mov	x13, x30 // lr
	mov	x0, x11
	blr	x14
	mov	x0, x12
	mov	x30, x13 // lr
	subs	x9, x9, #1                      // decrement the way
	b.ge    loop3
	subs	x7, x7, #1                      // decrement the index
	b.ge    loop2
skip:
	add	x10, x10, #2                    // increment cache number
	cmp	x3, x10
	b.gt    all_start_at_level
finished:
	mov	x10, #0                         // swith back to cache level 0
	msr	csselr_el1, x10                 // select current cache level in csselr
	dsb	sy
	isb
	ret


func do_dcsw_op
	cbz	x3, exit
	cmp	x0, #DCISW
	b.eq	dc_isw
	cmp	x0, #DCCISW
	b.eq	dc_cisw
	cmp	x0, #DCCSW
	b.eq	dc_csw
dc_isw:
	mov	x0, x9
	adr	x14, dcisw
	b	dcsw_op
dc_cisw:
	mov	x0, x9
	adr	x14, dccisw
	b	dcsw_op
dc_csw:
	mov	x0, x9
	adr	x14, dccsw
	b	dcsw_op
exit:
	ret


func dcsw_op_louis
	dsb	sy
	setup_dcsw_op_args x10, x3, x9, #LOUIS_SHIFT, #CLIDR_FIELD_WIDTH, #LEVEL_SHIFT
	b	do_dcsw_op


func dcsw_op_all
	dsb	sy
	setup_dcsw_op_args x10, x3, x9, #LOC_SHIFT, #CLIDR_FIELD_WIDTH, #LEVEL_SHIFT
	b	do_dcsw_op
