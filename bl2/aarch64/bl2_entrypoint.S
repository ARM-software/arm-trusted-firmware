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
#include <asm_macros.S>
#include <bl_common.h>


	.globl	bl2_entrypoint



func bl2_entrypoint
	/*---------------------------------------------
	 * Store the extents of the tzram available to
	 * BL2 for future use. Use the opcode param to
	 * allow implement other functions if needed.
	 * ---------------------------------------------
	 */
	mov	x20, x0
	mov	x21, x1

	/* ---------------------------------------------
	 * This is BL2 which is expected to be executed
	 * only by the primary cpu (at least for now).
	 * So, make sure no secondary has lost its way.
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_is_primary_cpu
	cbz	x0, _panic

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	adr	x0, early_exceptions
	msr	vbar_el1, x0

	/* ---------------------------------------------
	 * Enable the instruction cache.
	 * ---------------------------------------------
	 */
	mrs	x0, sctlr_el1
	orr	x0, x0, #SCTLR_I_BIT
	msr	sctlr_el1, x0
	isb

	/* ---------------------------------------------
	 * Check the opcodes out of paranoia.
	 * ---------------------------------------------
	 */
	mov	x0, #RUN_IMAGE
	cmp	x0, x20
	b.ne	_panic

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

	/* --------------------------------------------
	 * Give ourselves a small coherent stack to
	 * ease the pain of initializing the MMU
	 * --------------------------------------------
	 */
	mrs	x0, mpidr_el1
	bl	platform_set_coherent_stack

	/* ---------------------------------------------
	 * Perform early platform setup & platform
	 * specific early arch. setup e.g. mmu setup
	 * ---------------------------------------------
	 */
	mov	x0, x21
	bl	bl2_early_platform_setup
	bl	bl2_plat_arch_setup

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
	bl	bl2_main
_panic:
	b	_panic
