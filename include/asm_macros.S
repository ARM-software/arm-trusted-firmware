/*
 * Copyright (c) 2013, ARM Limited and Contributors. All rights reserved.
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

	.macro	func_prologue
	stp	x29, x30, [sp, #-0x10]!
	mov	x29,sp
	.endm

	.macro	func_epilogue
	ldp	x29, x30, [sp], #0x10
	.endm


	.macro	dcache_line_size  reg, tmp
	mrs     \tmp, ctr_el0
	ubfx	\tmp, \tmp, #16, #4
	mov     \reg, #4
	lsl     \reg, \reg, \tmp
	.endm


	.macro	icache_line_size  reg, tmp
	mrs     \tmp, ctr_el0
	and     \tmp, \tmp, #0xf
	mov     \reg, #4
	lsl     \reg, \reg, \tmp
	.endm


	.macro	exception_entry  func
	stp	x29, x30, [sp, #-(SIZEOF_GPREGS - GPREGS_FP_OFF)]!
	bl	\func
	.endm


	.macro	exception_exit  func
	bl	\func
	ldp	x29, x30, [sp], #(SIZEOF_GPREGS - GPREGS_FP_OFF)
	.endm


	.macro	smc_check  label
	bl	read_esr
	ubfx	x0, x0, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x0, #EC_AARCH64_SMC
	b.ne	$label
	.endm


	.macro	setup_dcsw_op_args  start_level, end_level, clidr, shift, fw, ls
	mrs	\clidr, clidr_el1
	mov	\start_level, xzr
	ubfx	\end_level, \clidr, \shift, \fw
	lsl	\end_level, \end_level, \ls
	.endm
