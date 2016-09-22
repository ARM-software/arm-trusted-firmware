/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <cpu_data.h>
#include <cpu_macros.S>

#if IMAGE_BL1 || IMAGE_BL32
	/*
	 * The reset handler common to all platforms.  After a matching
	 * cpu_ops structure entry is found, the correponding reset_handler
	 * in the cpu_ops is invoked. The reset handler is invoked very early
	 * in the boot sequence and it is assumed that we can clobber r0 - r10
	 * without the need to follow AAPCS.
	 * Clobbers: r0 - r10
	 */
	.globl	reset_handler
func reset_handler
	mov	r10, lr

	/* The plat_reset_handler can clobber r0 - r9 */
	bl	plat_reset_handler

	/* Get the matching cpu_ops pointer (clobbers: r0 - r5) */
	bl	get_cpu_ops_ptr

#if ASM_ASSERTION
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif

	/* Get the cpu_ops reset handler */
	ldr	r1, [r0, #CPU_RESET_FUNC]
	cmp	r1, #0
	mov	lr, r10
	bxne	r1
	bx	lr
endfunc reset_handler

#endif /* IMAGE_BL1 || IMAGE_BL32 */

#if IMAGE_BL32 /* The power down core and cluster is needed only in  BL32 */
	/*
	 * The prepare core power down function for all platforms.  After
	 * the cpu_ops pointer is retrieved from cpu_data, the corresponding
	 * pwr_dwn_core in the cpu_ops is invoked. Follows AAPCS.
	 */
	.globl	prepare_core_pwr_dwn
func prepare_core_pwr_dwn
	push	{lr}
	bl	_cpu_data
	pop	{lr}

	ldr	r1, [r0, #CPU_DATA_CPU_OPS_PTR]
#if ASM_ASSERTION
	cmp	r1, #0
	ASM_ASSERT(ne)
#endif

	/* Get the cpu_ops core_pwr_dwn handler */
	ldr	r0, [r1, #CPU_PWR_DWN_CORE]
	bx	r0
endfunc prepare_core_pwr_dwn

	/*
	 * The prepare cluster power down function for all platforms.  After
	 * the cpu_ops pointer is retrieved from cpu_data, the corresponding
	 * pwr_dwn_cluster in the cpu_ops is invoked. Follows AAPCS.
	 */
	.globl	prepare_cluster_pwr_dwn
func prepare_cluster_pwr_dwn
	push	{lr}
	bl	_cpu_data
	pop	{lr}

	ldr	r1, [r0, #CPU_DATA_CPU_OPS_PTR]
#if ASM_ASSERTION
	cmp	r1, #0
	ASM_ASSERT(ne)
#endif

	/* Get the cpu_ops cluster_pwr_dwn handler */
	ldr	r0, [r1, #CPU_PWR_DWN_CLUSTER]
	bx	r0
endfunc prepare_cluster_pwr_dwn

	/*
	 * Initializes the cpu_ops_ptr if not already initialized
	 * in cpu_data. This must only be called after the data cache
	 * is enabled. AAPCS is followed.
	 */
	.globl	init_cpu_ops
func init_cpu_ops
	push	{r4 - r6, lr}
	bl	_cpu_data
	mov	r6, r0
	ldr	r1, [r0, #CPU_DATA_CPU_OPS_PTR]
	cmp	r1, #0
	bne	1f
	bl	get_cpu_ops_ptr
#if ASM_ASSERTION
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif
	str	r0, [r6, #CPU_DATA_CPU_OPS_PTR]!
1:
	pop	{r4 - r6, pc}
endfunc init_cpu_ops

#endif /* IMAGE_BL32 */

	/*
	 * The below function returns the cpu_ops structure matching the
	 * midr of the core. It reads the MIDR and finds the matching
	 * entry in cpu_ops entries. Only the implementation and part number
	 * are used to match the entries.
	 * Return :
	 *     r0 - The matching cpu_ops pointer on Success
	 *     r0 - 0 on failure.
	 * Clobbers: r0 - r5
	 */
	.globl	get_cpu_ops_ptr
func get_cpu_ops_ptr
	/* Get the cpu_ops start and end locations */
	ldr	r4, =(__CPU_OPS_START__ + CPU_MIDR)
	ldr	r5, =(__CPU_OPS_END__ + CPU_MIDR)

	/* Initialize the return parameter */
	mov	r0, #0

	/* Read the MIDR_EL1 */
	ldcopr	r2, MIDR
	ldr	r3, =CPU_IMPL_PN_MASK

	/* Retain only the implementation and part number using mask */
	and	r2, r2, r3
1:
	/* Check if we have reached end of list */
	cmp	r4, r5
	bge	error_exit

	/* load the midr from the cpu_ops */
	ldr	r1, [r4], #CPU_OPS_SIZE
	and	r1, r1, r3

	/* Check if midr matches to midr of this core */
	cmp	r1, r2
	bne	1b

	/* Subtract the increment and offset to get the cpu-ops pointer */
	sub	r0, r4, #(CPU_OPS_SIZE + CPU_MIDR)
error_exit:
	bx	lr
endfunc get_cpu_ops_ptr
