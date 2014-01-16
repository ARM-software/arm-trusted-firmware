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

#include <arch.h>
#include <platform.h>


	.globl	pcpu_dv_mem_stack
	.weak	platform_get_core_pos
	.weak	platform_set_stack
	.weak	platform_get_stack
	.weak	platform_is_primary_cpu
	.weak	platform_set_coherent_stack
	.weak	platform_check_mpidr
	.weak	plat_report_exception

	/* -----------------------------------------------------
	 * 512 bytes of coherent stack for each cpu
	 * -----------------------------------------------------
	 */
#define PCPU_DV_MEM_STACK_SIZE	0x200


	.section	.text, "ax"; .align 3

	/* -----------------------------------------------------
	 * unsigned long long platform_set_coherent_stack
	 *                                    (unsigned mpidr);
	 * For a given mpidr, this function returns the stack
	 * pointer allocated in device memory. This stack can
	 * be used by C code which enables/disables the SCTLR.M
	 * SCTLR.C bit e.g. while powering down a cpu
	 * -----------------------------------------------------
	 */
platform_set_coherent_stack: ; .type platform_set_coherent_stack, %function
	mov	x5, x30 // lr
	bl	platform_get_core_pos
	add	x0, x0, #1
	mov	x1, #PCPU_DV_MEM_STACK_SIZE
	mul	x0, x0, x1
	ldr	x1, =pcpu_dv_mem_stack
	add	sp, x1, x0
	ret	x5


	/* -----------------------------------------------------
	 *  int platform_get_core_pos(int mpidr);
	 *  With this function: CorePos = (ClusterId * 4) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
platform_get_core_pos: ; .type platform_get_core_pos, %function
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret


	/* -----------------------------------------------------
	 * void platform_is_primary_cpu (unsigned int mpid);
	 *
	 * Given the mpidr say whether this cpu is the primary
	 * cpu (applicable ony after a cold boot)
	 * -----------------------------------------------------
	 */
platform_is_primary_cpu: ; .type platform_is_primary_cpu, %function
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PRIMARY_CPU
	cset	x0, eq
	ret

	/* -----------------------------------------------------
	 * void platform_get_stack (unsigned long mpidr)
	 * -----------------------------------------------------
	 */
platform_get_stack: ; .type platform_get_stack, %function
	mov	x10, x30 // lr
	bl	platform_get_core_pos
	add	x0, x0, #1
	mov	x1, #PLATFORM_STACK_SIZE
	mul	x0, x0, x1
	ldr	x1, =platform_normal_stacks
	add	x0, x1, x0
	ret	x10

	/* -----------------------------------------------------
	 * void platform_set_stack (unsigned long mpidr)
	 * -----------------------------------------------------
	 */
platform_set_stack: ; .type platform_set_stack, %function
	mov	x9, x30 // lr
	bl	platform_get_stack
	mov	sp, x0
	ret	x9

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
platform_check_mpidr: ; .type platform_check_mpidr, %function
	mov	x0, xzr
	ret

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
plat_report_exception:
	ret

	/* -----------------------------------------------------
	 * Per-cpu stacks in device memory.
	 * Used for C code just before power down or right after
	 * power up when the MMU or caches need to be turned on
	 * or off. Each cpu gets a stack of 512 bytes.
	 * -----------------------------------------------------
	 */
	.section	tzfw_coherent_mem, "aw", %nobits; .align 6

pcpu_dv_mem_stack:
	/* Zero fill */
	.space (PLATFORM_CORE_COUNT * PCPU_DV_MEM_STACK_SIZE), 0
