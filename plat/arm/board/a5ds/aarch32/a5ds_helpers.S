/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary

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
	/* Calculate address of our hold entry */
	bl	plat_my_core_pos
	lsl	r0, r0, #A5DS_HOLD_ENTRY_SHIFT
	mov_imm	r2, A5DS_HOLD_BASE
	/* Clear the value stored in the hold address for the specific core */
	mov_imm	r3, A5DS_HOLD_STATE_WAIT
	str	r3, [r2, r0]
	dmb	ish

	/* Wait until we have a go */
poll_mailbox:
	ldr	r1, [r2, r0]
	cmp	r1, #A5DS_HOLD_STATE_WAIT
	beq	1f
	mov_imm	r0, A5DS_TRUSTED_MAILBOX_BASE
	ldr	r1, [r0]
	bx	r1
1:
	wfe
	b	poll_mailbox
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot.
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	/* TODO support warm boot */
	/* Cold reset */
	mov	r0, #0
	bx	lr

endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	ldcopr	r0, MPIDR
	ldr	r1, =MPIDR_AFFINITY_MASK
	and	r0, r1
	cmp	r0, #0
	moveq	r0, #1
	movne	r0, #0
	bx	lr
endfunc plat_is_my_cpu_primary

	/* ---------------------------------------------------------------------
	 * Loads MPIDR in r0 and calls plat_arm_calc_core_pos
	 * ---------------------------------------------------------------------
	 */
func plat_my_core_pos
	ldcopr	r0, MPIDR
	b	plat_arm_calc_core_pos

endfunc plat_my_core_pos

	/* ---------------------------------------------------------------------
	 * unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *
	 * Function to calculate the core position on A5DS.
	 *
	 * (ClusterId * A5DS_MAX_CPUS_PER_CLUSTER * A5DS_MAX_PE_PER_CPU) +
	 * (CPUId * A5DS_MAX_PE_PER_CPU) +
	 * ThreadId
	 *
	 * which can be simplified as:
	 *
	 * ((ClusterId * A5DS_MAX_CPUS_PER_CLUSTER + CPUId) * A5DS_MAX_PE_PER_CPU)
	 * + ThreadId
	 * ---------------------------------------------------------------------
	 */
func plat_arm_calc_core_pos
	mov	r3, r0

	/*
	 * Check for MT bit in MPIDR. If not set, shift MPIDR to left to make it
	 * look as if in a multi-threaded implementation
	 */
	tst	r0, #MPIDR_MT_MASK
	lsleq	r3, r0, #MPIDR_AFFINITY_BITS

	/* Extract individual affinity fields from MPIDR */
	ubfx	r0, r3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	r1, r3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	r2, r3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	/* Compute linear position */
	mov	r3, #A5DS_MAX_CPUS_PER_CLUSTER
	mla	r1, r2, r3, r1
	mov	r3, #A5DS_MAX_PE_PER_CPU
	mla	r0, r1, r3, r0

	bx	lr
endfunc plat_arm_calc_core_pos
