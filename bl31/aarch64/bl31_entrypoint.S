/*
 * Copyright (c) 2013, ARM Limited. All rights reserved.
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

#include <bl1.h>
#include <bl_common.h>
#include <platform.h>
#include <arch.h>


	.globl	bl31_entrypoint


	.section	entry_code, "ax"; .align 3

	/* -----------------------------------------------------
	 * bl31_entrypoint() is the cold boot entrypoint,
	 * executed only by the primary cpu.
	 * -----------------------------------------------------
	 */

bl31_entrypoint:; .type bl31_entrypoint, %function
	/* ---------------------------------------------
	 * BL2 has populated x0,x3,x4 with the opcode
	 * indicating BL31 should be run, memory layout
	 * of the trusted SRAM available to BL31 and
	 * information about running the non-trusted
	 * software already loaded by BL2.
	 * ---------------------------------------------
	 */

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	adr	x1, runtime_exceptions
	msr	vbar_el3, x1

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
	bl	read_mpidr
	mov	x19, x0
	bl	platform_is_primary_cpu
	cbz	x0, _panic

	/* --------------------------------------------
	 * Give ourselves a small coherent stack to
	 * ease the pain of initializing the MMU
	 * --------------------------------------------
	 */
	mov	x0, x19
	bl	platform_set_coherent_stack

	/* ---------------------------------------------
	 * Perform platform specific early arch. setup
	 * ---------------------------------------------
	 */
	mov	x0, x20
	mov	x1, x21
	mov	x2, x19
	bl	bl31_early_platform_setup
	bl	bl31_plat_arch_setup

	/* ---------------------------------------------
	 * Give ourselves a stack allocated in Normal
	 * -IS-WBWA memory
	 * ---------------------------------------------
	 */
	mov	x0, x19
	bl	platform_set_stack

	/* ---------------------------------------------
	 * Use SP_EL0 to initialize BL31. It allows us
	 * to jump to the next image without having to
	 * come back here to ensure all of the stack's
	 * been popped out. run_image() is not nice
	 * enough to reset the stack pointer before
	 * handing control to the next stage.
	 * ---------------------------------------------
	 */
	mov	x0, sp
	msr	sp_el0, x0
	msr	spsel, #0
	isb

	/* ---------------------------------------------
	 * Jump to main function.
	 * ---------------------------------------------
	 */
	bl	bl31_main

_panic:
	b	_panic
