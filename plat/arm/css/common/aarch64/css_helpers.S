/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <cpu_macros.S>
#include <css_def.h>

	.weak	plat_secondary_cold_boot_setup
	.weak	platform_get_entrypoint
	.weak	platform_mem_init
	.globl	platform_get_core_pos
	.weak	platform_is_primary_cpu


	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* todo: Implement secondary CPU cold boot setup on CSS platforms */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

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
	 * -----------------------------------------------------
	 */
func platform_get_entrypoint
	mov	x9, x30 // lr
	bl	platform_get_core_pos
	ldr	x1, =TRUSTED_MAILBOXES_BASE
	lsl	x0, x0, #TRUSTED_MAILBOX_SHIFT
	ldr	x0, [x1, x0]
	ret	x9
endfunc platform_get_entrypoint

	/*
	 * Override the default implementation to swap the cluster order.
	 * This is necessary in order to match the format of the boot
	 * information passed by the SCP and read in platform_is_primary_cpu
	 * below.
	 */
func platform_get_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	eor	x0, x0, #(1 << MPIDR_AFFINITY_BITS)  // swap cluster order
	add	x0, x1, x0, LSR #6
	ret
endfunc platform_get_core_pos

	/* -----------------------------------------------------
	 * void platform_mem_init(void);
	 *
	 * We don't need to carry out any memory initialization
	 * on CSS platforms. The Secure RAM is accessible straight away.
	 * -----------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

	/* -----------------------------------------------------
	 * unsigned int platform_is_primary_cpu (unsigned int mpid);
	 *
	 * Given the mpidr say whether this cpu is the primary
	 * cpu (applicable ony after a cold boot)
	 * -----------------------------------------------------
	 */
func platform_is_primary_cpu
	mov	x9, x30
	bl	platform_get_core_pos
	ldr	x1, =SCP_BOOT_CFG_ADDR
	ldr	x1, [x1]
	ubfx	x1, x1, #PRIMARY_CPU_SHIFT, #PRIMARY_CPU_BIT_WIDTH
	cmp	x0, x1
	cset	x0, eq
	ret	x9
endfunc platform_is_primary_cpu
