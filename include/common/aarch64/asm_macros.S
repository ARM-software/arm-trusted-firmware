/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#ifndef __ASM_MACROS_S__
#define __ASM_MACROS_S__

#include <arch.h>
#include <asm_macros_common.S>


	.macro	func_prologue
	stp	x29, x30, [sp, #-0x10]!
	mov	x29,sp
	.endm

	.macro	func_epilogue
	ldp	x29, x30, [sp], #0x10
	.endm


	.macro	dcache_line_size  reg, tmp
	mrs	\tmp, ctr_el0
	ubfx	\tmp, \tmp, #16, #4
	mov	\reg, #4
	lsl	\reg, \reg, \tmp
	.endm


	.macro	icache_line_size  reg, tmp
	mrs	\tmp, ctr_el0
	and	\tmp, \tmp, #0xf
	mov	\reg, #4
	lsl	\reg, \reg, \tmp
	.endm


	.macro	smc_check  label
	mrs	x0, esr_el3
	ubfx	x0, x0, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x0, #EC_AARCH64_SMC
	b.ne	$label
	.endm

	/*
	 * Declare the exception vector table, enforcing it is aligned on a
	 * 2KB boundary, as required by the ARMv8 architecture.
	 * Use zero bytes as the fill value to be stored in the padding bytes
	 * so that it inserts illegal AArch64 instructions. This increases
	 * security, robustness and potentially facilitates debugging.
	 */
	.macro vector_base  label
	.section .vectors, "ax"
	.align 11, 0
	\label:
	.endm

	/*
	 * Create an entry in the exception vector table, enforcing it is
	 * aligned on a 128-byte boundary, as required by the ARMv8 architecture.
	 * Use zero bytes as the fill value to be stored in the padding bytes
	 * so that it inserts illegal AArch64 instructions. This increases
	 * security, robustness and potentially facilitates debugging.
	 */
	.macro vector_entry  label
	.section .vectors, "ax"
	.align 7, 0
	\label:
	.endm

	/*
	 * This macro verifies that the given vector doesn't exceed the
	 * architectural limit of 32 instructions. This is meant to be placed
	 * immediately after the last instruction in the vector. It takes the
	 * vector entry as the parameter
	 */
	.macro check_vector_size since
	  .if (. - \since) > (32 * 4)
	    .error "Vector exceeds 32 instructions"
	  .endif
	.endm

#if ENABLE_PLAT_COMPAT
	/*
	 * This macro calculates the base address of an MP stack using the
	 * platform_get_core_pos() index, the name of the stack storage and
	 * the size of each stack
	 * In: X0 = MPIDR of CPU whose stack is wanted
	 * Out: X0 = physical address of stack base
	 * Clobber: X30, X1, X2
	 */
	.macro get_mp_stack _name, _size
	bl  platform_get_core_pos
	ldr x2, =(\_name + \_size)
	mov x1, #\_size
	madd x0, x0, x1, x2
	.endm
#endif

	/*
	 * This macro calculates the base address of the current CPU's MP stack
	 * using the plat_my_core_pos() index, the name of the stack storage
	 * and the size of each stack
	 * Out: X0 = physical address of stack base
	 * Clobber: X30, X1, X2
	 */
	.macro get_my_mp_stack _name, _size
	bl  plat_my_core_pos
	ldr x2, =(\_name + \_size)
	mov x1, #\_size
	madd x0, x0, x1, x2
	.endm

	/*
	 * This macro calculates the base address of a UP stack using the
	 * name of the stack storage and the size of the stack
	 * Out: X0 = physical address of stack base
	 */
	.macro get_up_stack _name, _size
	ldr x0, =(\_name + \_size)
	.endm

	/*
	 * Helper macro to generate the best mov/movk combinations according
	 * the value to be moved. The 16 bits from '_shift' are tested and
	 * if not zero, they are moved into '_reg' without affecting
	 * other bits.
	 */
	.macro _mov_imm16 _reg, _val, _shift
		.if (\_val >> \_shift) & 0xffff
			.if (\_val & (1 << \_shift - 1))
				movk	\_reg, (\_val >> \_shift) & 0xffff, LSL \_shift
			.else
				mov	\_reg, \_val & (0xffff << \_shift)
			.endif
		.endif
	.endm

	/*
	 * Helper macro to load arbitrary values into 32 or 64-bit registers
	 * which generates the best mov/movk combinations. Many base addresses
	 * are 64KB aligned the macro will eliminate updating bits 15:0 in
	 * that case
	 */
	.macro mov_imm _reg, _val
		.if (\_val) == 0
			mov	\_reg, #0
		.else
			_mov_imm16	\_reg, (\_val), 0
			_mov_imm16	\_reg, (\_val), 16
			_mov_imm16	\_reg, (\_val), 32
			_mov_imm16	\_reg, (\_val), 48
		.endif
	.endm

#endif /* __ASM_MACROS_S__ */
