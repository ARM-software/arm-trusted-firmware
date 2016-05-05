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

#include <asm_macros.S>
#include <cpu_data.h>

	.globl	_cpu_data
	.globl	_cpu_data_by_index

/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data(void)
 *
 * Return the cpu_data structure for the current CPU.
 * -----------------------------------------------------------------
 */
func _cpu_data
	push	{lr}
	bl	plat_my_core_pos
	pop	{lr}
	b	_cpu_data_by_index
endfunc _cpu_data

/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data_by_index(uint32_t cpu_index)
 *
 * Return the cpu_data structure for the CPU with given linear index
 *
 * This can be called without a valid stack.
 * clobbers: r0, r1
 * -----------------------------------------------------------------
 */
func _cpu_data_by_index
	ldr	r1, =percpu_data
	add	r0, r1, r0, LSL #CPU_DATA_LOG2SIZE
	bx	lr
endfunc _cpu_data_by_index
