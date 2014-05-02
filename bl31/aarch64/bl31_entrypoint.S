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

#include <bl_common.h>
#include <platform.h>
#include <arch.h>
#include "cm_macros.S"
#include <asm_macros.S>


	.globl	bl31_entrypoint


	/* -----------------------------------------------------
	 * bl31_entrypoint() is the cold boot entrypoint,
	 * executed only by the primary cpu.
	 * -----------------------------------------------------
	 */

func bl31_entrypoint
	/* ---------------------------------------------
	 * BL2 has populated x0 with the opcode
	 * indicating BL31 should be run, x3 with
	 * a pointer to a 'bl31_args' structure & x4
 	 * with any other optional information
	 * ---------------------------------------------
	 */

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	adr	x1, early_exceptions
	msr	vbar_el3, x1

	/* ---------------------------------------------------------------------
	 * The initial state of the Architectural feature trap register
	 * (CPTR_EL3) is unknown and it must be set to a known state. All
	 * feature traps are disabled. Some bits in this register are marked as
	 * Reserved and should not be modified.
	 *
	 * CPTR_EL3.TCPAC: This causes a direct access to the CPACR_EL1 from EL1
	 *  or the CPTR_EL2 from EL2 to trap to EL3 unless it is trapped at EL2.
	 * CPTR_EL3.TTA: This causes access to the Trace functionality to trap
	 *  to EL3 when executed from EL0, EL1, EL2, or EL3. If system register
	 *  access to trace functionality is not supported, this bit is RES0.
	 * CPTR_EL3.TFP: This causes instructions that access the registers
	 *  associated with Floating Point and Advanced SIMD execution to trap
	 *  to EL3 when executed from any exception level, unless trapped to EL1
	 *  or EL2.
	 * ---------------------------------------------------------------------
	 */
	mrs	x1, cptr_el3
	bic	w1, w1, #TCPAC_BIT
	bic	w1, w1, #TTA_BIT
	bic	w1, w1, #TFP_BIT
	msr	cptr_el3, x1

	/* ---------------------------------------------
	 * Enable the instruction cache.
	 * ---------------------------------------------
	 */
	mrs	x1, sctlr_el3
	orr	x1, x1, #SCTLR_I_BIT
	msr	sctlr_el3, x1
	isb

	/* ---------------------------------------------
	 * Check the opcodes out of paranoia.
	 * ---------------------------------------------
	 */
	mov	x19, #RUN_IMAGE
	cmp	x0, x19
	b.ne	_panic
	mov	x20, x3
	mov	x21, x4

	/* ---------------------------------------------
	 * This is BL31 which is expected to be executed
	 * only by the primary cpu (at least for now).
	 * So, make sure no secondary has lost its way.
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_is_primary_cpu
	cbz	x0, _panic

	/* ---------------------------------------------
	 * Zero out NOBITS sections. There are 2 of them:
	 *   - the .bss section;
	 *   - the coherent memory section.
	 * ---------------------------------------------
	 */
	ldr	x0, =__BSS_START__
	ldr	x1, =__BSS_SIZE__
	bl	zeromem16

	ldr	x0, =__COHERENT_RAM_START__
	ldr	x1, =__COHERENT_RAM_UNALIGNED_SIZE__
	bl	zeromem16

	/* ---------------------------------------------
	 * Use SP_EL0 for the C runtime stack.
	 * ---------------------------------------------
	 */
	msr	spsel, #0

	/* --------------------------------------------
	 * Give ourselves a small coherent stack to
	 * ease the pain of initializing the MMU
	 * --------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_coherent_stack

	/* ---------------------------------------------
	 * Perform platform specific early arch. setup
	 * ---------------------------------------------
	 */
	mov	x0, x20
	mov	x1, x21
	bl	bl31_early_platform_setup
	bl	bl31_plat_arch_setup

	/* ---------------------------------------------
	 * Give ourselves a stack allocated in Normal
	 * -IS-WBWA memory
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_stack

	/* ---------------------------------------------
	 * Jump to main function.
	 * ---------------------------------------------
	 */
	bl	bl31_main

	zero_callee_saved_regs
	b	el3_exit

_panic:
	wfi
	b	_panic
