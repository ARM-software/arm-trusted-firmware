/*
 * Copyright (c) 2021-2025 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary
	.globl	plat_arm_calc_core_pos

#ifdef CORSTONE1000_CORTEX_A320
	.globl	plat_my_core_pos

func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_arm_calc_core_pos
endfunc plat_my_core_pos

func plat_arm_calc_core_pos
	/*
	 * Aff0 is always 0 for Cortex-A320 MPIDR format:
	 * https://developer.arm.com/documentation/109551/0001/AArch64-registers/AArch64-Identification-registers-summary/MPIDR-EL1--Multiprocessor-Affinity-Register?lang=en
	 */
	/* Extract Aff1 (core ID) */
	ubfx    x1, x0, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS

	/* Extract Aff2 (cluster lower bits) */
	ubfx    x2, x0, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	/* Extract Aff3 (cluster upper bits) */
	ubfx    x3, x0, #MPIDR_AFF3_SHIFT, #MPIDR_AFFINITY_BITS

	/* cluster_id = (Aff3 << 8) | Aff2 */
	lsl     x3, x3, #MPIDR_AFFINITY_BITS
	orr     x3, x3, x2

	/* core_pos = core_id + (cluster_id * FVP_MAX_CPUS_PER_CLUSTER) */
	mov	x4, #CORSTONE1000_MAX_CPUS_PER_CLUSTER
	madd	x0, x3, x4, x1

	ret
endfunc plat_arm_calc_core_pos
#endif

	/* --------------------------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * For AArch32, cold-booting secondary CPUs is not yet
	 * implemented and they panic.
	 * --------------------------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
#if defined(CORSTONE1000_FVP_MULTICORE)

	/* Calculate the address of our hold entry */
	bl	plat_my_core_pos
	lsl	x0, x0, #CORSTONE1000_SECONDARY_CORE_HOLD_SHIFT
	mov_imm	x2, CORSTONE1000_SECONDARY_CORE_HOLD_BASE

	/* Set the wait state for the secondary core */
	mov_imm	x3, CORSTONE1000_SECONDARY_CORE_STATE_WAIT
	str	x3, [x2, x0]
	dmb	ish

	/* Poll until the primary core signals to go  */
poll_mailbox:
	ldr	x1, [x2, x0]
	cmp	x1, #CORSTONE1000_SECONDARY_CORE_STATE_WAIT
	beq	1f
	mov_imm	x0, PLAT_ARM_TRUSTED_MAILBOX_BASE
	ldr	x1, [x0]
	br	x1
1:
	wfe
	b	poll_mailbox
#else
cb_panic:
	b	cb_panic
#endif

endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot. On corstone1000, this information can be queried from the power
	 * controller. The Power Control SYS Status Register (PSYSR) indicates
	 * the wake-up reason for the CPU.
	 *
	 * For a cold boot, return 0.
	 * For a warm boot, Not yet supported.
	 *
	 * TODO: PSYSR is a common register and should be
	 * 	accessed using locks. Since it is not possible
	 * 	to use locks immediately after a cold reset
	 * 	we are relying on the fact that after a cold
	 * 	reset all cpus will read the same WK field
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	/* TODO support warm boot */
	/* Cold reset */
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current CPU is the primary
	 * CPU.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	mov_imm	x1, MPIDR_AFFINITY_MASK
	and	x0, x0, x1
	cmp	x0, #CORSTONE1000_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary
