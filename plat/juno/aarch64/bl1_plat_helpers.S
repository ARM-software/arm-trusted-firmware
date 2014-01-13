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

	.globl	platform_get_entrypoint
	.globl	platform_cold_boot_init
	.globl	plat_secondary_cold_boot_setup


	.section	.text, "ax"; .align 3


	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * -----------------------------------------------------
	 */
plat_secondary_cold_boot_setup: ; .type plat_secondary_cold_boot_setup, %function
	/* Juno todo: Implement secondary CPU cold boot setup on Juno */
cb_panic:
	b	cb_panic


	/* -----------------------------------------------------
	 * void platform_get_entrypoint (unsigned int mpid);
	 *
	 * Main job of this routine is to distinguish between
	 * a cold and warm boot.
	 * On a cold boot the secondaries first wait for the
	 * platform to be initialized after which they are
	 * hotplugged in. The primary proceeds to perform the
	 * platform initialization.
	 * On a warm boot, each cpu jumps to the address in its
	 * mailbox.
	 *
	 * TODO: Not a good idea to save lr in a temp reg
	 * TODO: PSYSR is a common register and should be
	 * 	accessed using locks. Since its not possible
	 * 	to use locks immediately after a cold reset
	 * 	we are relying on the fact that after a cold
	 * 	reset all cpus will read the same WK field
	 * -----------------------------------------------------
	 */
platform_get_entrypoint: ; .type platform_get_entrypoint, %function
	mov	x9, x30 // lr
	mov	x2, x0
	ldr	x1, =PWRC_BASE
	str	w2, [x1, #PSYSR_OFF]
	ldr	w2, [x1, #PSYSR_OFF]
	ubfx	w2, w2, #PSYSR_WK_SHIFT, #PSYSR_WK_MASK
	cbnz	w2, warm_reset
	mov	x0, x2
	b	exit
warm_reset:
	/* ---------------------------------------------
	 * A per-cpu mailbox is maintained in the tru-
	 * sted DRAM. Its flushed out of the caches
	 * after every update using normal memory so
	 * its safe to read it here with SO attributes
	 * ---------------------------------------------
	 */
	ldr	x10, =TZDRAM_BASE + MBOX_OFF
	bl	platform_get_core_pos
	lsl	x0, x0, #CACHE_WRITEBACK_SHIFT
	ldr	x0, [x10, x0]
	cbz	x0, _panic
exit:
	ret	x9
_panic:	b	_panic


	/* -----------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Zero out the mailbox registers in the TZDRAM. The
	 * mmu is turned off right now and only the primary can
	 * ever execute this code. Secondaries will read the
	 * mailboxes using SO accesses. In short, BL31 will
	 * update the mailboxes after mapping the tzdram as
	 * normal memory. It will flush its copy after update.
	 * BL1 will always read the mailboxes with the MMU off
	 * -----------------------------------------------------
	 */
platform_mem_init: ; .type platform_mem_init, %function
	ldr	x0, =TZDRAM_BASE + MBOX_OFF
	stp	xzr, xzr, [x0, #0]
	stp	xzr, xzr, [x0, #0x10]
	stp	xzr, xzr, [x0, #0x20]
	stp	xzr, xzr, [x0, #0x30]
	ret


	/* -----------------------------------------------------
	 * void platform_cold_boot_init (bl1_main function);
	 *
	 * Routine called only by the primary cpu after a cold
	 * boot to perform early platform initialization
	 * -----------------------------------------------------
	 */
platform_cold_boot_init: ; .type platform_cold_boot_init, %function
	mov	x20, x0
	bl	platform_mem_init
	bl	read_mpidr
	mov	x19, x0

	/* ---------------------------------------------
	 * Give ourselves a small coherent stack to
	 * ease the pain of initializing the MMU and
	 * CCI in assembler
	 * ---------------------------------------------
	 */
	bl	platform_set_coherent_stack

	/* ---------------------------------------------
	 * Architectural init. can be generic e.g.
	 * enabling stack alignment and platform spec-
	 * ific e.g. MMU & page table setup as per the
	 * platform memory map. Perform the latter here
	 * and the former in bl1_main.
	 * ---------------------------------------------
	 */
	bl	bl1_early_platform_setup
	bl	bl1_plat_arch_setup

	/* ---------------------------------------------
	 * Give ourselves a stack allocated in Normal
	 * -IS-WBWA memory
	 * ---------------------------------------------
	 */
	mov	x0, x19
	bl	platform_set_stack

	/* ---------------------------------------------
	 * Jump to the main function. Returning from it
	 * is a terminal error.
	 * ---------------------------------------------
	 */
	blr	x20

cb_init_panic:
	b	cb_init_panic
