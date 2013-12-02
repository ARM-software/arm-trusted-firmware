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

#include <arch.h>

	.globl	reset_handler


	.section	.text, "ax"; .align 3

	/* -----------------------------------------------------
	 * reset_handler() is the entry point into the trusted
	 * firmware code when a cpu is released from warm or
	 * cold reset.
	 * -----------------------------------------------------
	 */

reset_handler:; .type reset_handler, %function
	/* ---------------------------------------------
	 * Perform any processor specific actions upon
	 * reset e.g. cache, tlb invalidations etc.
	 * ---------------------------------------------
	 */
	bl	cpu_reset_handler

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	adr	x0, early_exceptions
	msr	vbar_el3, x0

	/* ---------------------------------------------
	 * Enable the instruction cache.
	 * ---------------------------------------------
	 */
	mrs	x0, sctlr_el3
	orr	x0, x0, #SCTLR_I_BIT
	msr	sctlr_el3, x0

	isb

_wait_for_entrypoint:
	/* ---------------------------------------------
	 * Find the type of reset and jump to handler
	 * if present. If the handler is null then it is
	 * a cold boot. The primary cpu will set up the
	 * platform while the secondaries wait for
	 * their turn to be woken up
	 * ---------------------------------------------
	 */
	bl	read_mpidr
	bl	platform_get_entrypoint
	cbnz	x0, _do_warm_boot
	bl	read_mpidr
	bl	platform_is_primary_cpu
	cbnz	x0, _do_cold_boot

	/* ---------------------------------------------
	 * Perform any platform specific secondary cpu
	 * actions
	 * ---------------------------------------------
	 */
	bl	plat_secondary_cold_boot_setup
	b	_wait_for_entrypoint

_do_cold_boot:
	/* ---------------------------------------------
	 * Init C runtime environment.
	 *   - Zero-initialise the NOBITS sections.
	 *     There are 2 of them:
	 *       - the .bss section;
	 *       - the coherent memory section.
	 *   - Copy the data section from BL1 image
	 *     (stored in ROM) to the correct location
	 *     in RAM.
	 * ---------------------------------------------
	 */
	ldr	x0, =__BSS_START__
	ldr	x1, =__BSS_SIZE__
	bl	zeromem16

	ldr	x0, =__COHERENT_RAM_START__
	ldr	x1, =__COHERENT_RAM_UNALIGNED_SIZE__
	bl	zeromem16

	ldr	x0, =__DATA_RAM_START__
	ldr	x1, =__DATA_ROM_START__
	ldr	x2, =__DATA_SIZE__
	bl	memcpy16

	/* ---------------------------------------------
	 * Initialize platform and jump to our c-entry
	 * point for this type of reset
	 * ---------------------------------------------
	 */
	adr	x0, bl1_main
	bl	platform_cold_boot_init
	b	_panic

_do_warm_boot:
	/* ---------------------------------------------
	 * Jump to BL31 for all warm boot init.
	 * ---------------------------------------------
	 */
	blr	x0
_panic:
	b	_panic
