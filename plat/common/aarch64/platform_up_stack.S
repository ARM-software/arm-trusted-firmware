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
#include <platform_def.h>


	.local	pcpu_dv_mem_stack
	.local	platform_normal_stacks
	.globl	platform_set_stack
	.globl	platform_get_stack
	.globl	platform_set_coherent_stack


	/* -----------------------------------------------------
	 * void platform_set_coherent_stack (unsigned long)
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function sets the stack pointer to a stack
	 * allocated in device memory.
	 * -----------------------------------------------------
	 */
func platform_set_coherent_stack
	get_up_stack pcpu_dv_mem_stack, PCPU_DV_MEM_STACK_SIZE
	mov sp, x0
	ret


	/* -----------------------------------------------------
	 * unsigned long platform_get_stack (unsigned long)
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function returns the stack pointer for a
	 * stack allocated in device memory.
	 * -----------------------------------------------------
	 */
func platform_get_stack
	get_up_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	ret

	/* -----------------------------------------------------
	 * void platform_set_stack (unsigned long)
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function sets the stack pointer to a stack
	 * allocated in normal memory.
	 * -----------------------------------------------------
	 */
func platform_set_stack
	get_up_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	mov sp, x0
	ret

	/* -----------------------------------------------------
	 * Single cpu stack in normal memory.
	 * Used for C code during boot, PLATFORM_STACK_SIZE bytes
	 * are allocated
	 * -----------------------------------------------------
	 */
declare_stack platform_normal_stacks, tzfw_normal_stacks, \
		PLATFORM_STACK_SIZE, 1

	/* -----------------------------------------------------
	 * Single cpu stack in device/coherent memory.
	 * PCPU_DV_MEM_STACK_SIZE bytes are allocated.
	 * -----------------------------------------------------
	 */
declare_stack pcpu_dv_mem_stack, tzfw_coherent_mem, \
		PCPU_DV_MEM_STACK_SIZE, 1
