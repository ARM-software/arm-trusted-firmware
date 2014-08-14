/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#if IMAGE_BL31
#include <cpu_data.h>
#endif

 /* Reset fn is needed in BL at reset vector */
#if IMAGE_BL1 || (IMAGE_BL31 && RESET_TO_BL31)
	/*
	 * The reset handler common to all platforms.  After a matching
	 * cpu_ops structure entry is found, the correponding reset_handler
	 * in the cpu_ops is invoked.
	 */
	.globl	reset_handler
func reset_handler
	mov	x10, x30

	/* Get the matching cpu_ops pointer */
	bl	get_cpu_ops_ptr
#if ASM_ASSERTION
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif

	/* Get the cpu_ops reset handler */
	ldr	x2, [x0, #CPU_RESET_FUNC]
	cbz	x2, 1f
	blr	x2
1:
	ret	x10
#endif /* IMAGE_BL1 || (IMAGE_BL31 && RESET_TO_BL31) */

	/*
	 * The below function returns the cpu_ops structure matching the
	 * midr of the core. It reads the MIDR_EL1 and finds the matching
	 * entry in cpu_ops entries. Only the implementation and part number
	 * are used to match the entries.
	 * Return :
	 *     x0 - The matching cpu_ops pointer on Success
	 *     x0 - 0 on failure.
	 * Clobbers : x0 - x5
	 */
	.globl	get_cpu_ops_ptr
func get_cpu_ops_ptr
	/* Get the cpu_ops start and end locations */
	adr	x4, (__CPU_OPS_START__ + CPU_MIDR)
	adr	x5, (__CPU_OPS_END__ + CPU_MIDR)

	/* Initialize the return parameter */
	mov	x0, #0

	/* Read the MIDR_EL1 */
	mrs	x2, midr_el1
	mov_imm	x3, CPU_IMPL_PN_MASK

	/* Retain only the implementation and part number using mask */
	and	w2, w2, w3
1:
	/* Check if we have reached end of list */
	cmp	x4, x5
	b.eq	error_exit

	/* load the midr from the cpu_ops */
	ldr	x1, [x4], #CPU_OPS_SIZE
	and	w1, w1, w3

	/* Check if midr matches to midr of this core */
	cmp	w1, w2
	b.ne	1b

	/* Subtract the increment and offset to get the cpu-ops pointer */
	sub	x0, x4, #(CPU_OPS_SIZE + CPU_MIDR)
error_exit:
	ret
