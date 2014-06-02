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

#include <asm_macros.S>
#include <cpu_data.h>

.globl	init_cpu_data_ptr
.globl	_cpu_data_by_mpidr
.globl	_cpu_data_by_index

/* -----------------------------------------------------------------
 * void init_cpu_data_ptr(void)
 *
 * Initialise the TPIDR_EL3 register to refer to the cpu_data_t
 * for the calling CPU. This must be called before cm_get_cpu_data()
 *
 * This can be called without a valid stack.
 * clobbers: x0, x1, x9, x10
 * -----------------------------------------------------------------
 */
func init_cpu_data_ptr
	mov	x10, x30
	mrs	x0, mpidr_el1
	bl	_cpu_data_by_mpidr
	msr	tpidr_el3, x0
	ret	x10


/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data_by_mpidr(uint64_t mpidr)
 *
 * Return the cpu_data structure for the CPU with given MPIDR
 *
 * This can be called without a valid stack. It assumes that
 * platform_get_core_pos() does not clobber register x9.
 * clobbers: x0, x1, x9
 * -----------------------------------------------------------------
 */
func _cpu_data_by_mpidr
	mov	x9, x30
	bl	platform_get_core_pos
	mov	x30, x9
	b	_cpu_data_by_index


/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data_by_index(uint32_t cpu_index)
 *
 * Return the cpu_data structure for the CPU with given linear index
 *
 * This can be called without a valid stack.
 * clobbers: x0, x1
 * -----------------------------------------------------------------
 */
func _cpu_data_by_index
	adr	x1, percpu_data
	add	x0, x1, x0, LSL #CPU_DATA_LOG2SIZE
	ret
