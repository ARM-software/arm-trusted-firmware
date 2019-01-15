/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <platform_def.h>

	.weak	plat_secondary_cold_boot_setup
	.weak	plat_get_my_entrypoint
	.globl	css_calc_core_pos_swap_cluster
	.weak	plat_is_my_cpu_primary

	/* ---------------------------------------------------------------------
	 * void plat_secondary_cold_boot_setup(void);
	 * In the normal boot flow, cold-booting secondary
	 * CPUs is not yet implemented and they panic.
	 * ---------------------------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* TODO: Implement secondary CPU cold boot setup on CSS platforms */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------------------------------
	 * uintptr_t plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and a warm
	 * boot. On CSS platforms, this distinction is based on the contents of
	 * the Trusted Mailbox. It is initialised to zero by the SCP before the
	 * AP cores are released from reset. Therefore, a zero mailbox means
	 * it's a cold reset.
	 *
	 * This functions returns the contents of the mailbox, i.e.:
	 *  - 0 for a cold boot;
	 *  - the warm boot entrypoint for a warm boot.
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	ldr	r0, =PLAT_ARM_TRUSTED_MAILBOX_BASE
	ldr	r0, [r0]
	bx	lr
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------------
	 * unsigned int css_calc_core_pos_swap_cluster(u_register_t mpidr)
	 * Utility function to calculate the core position by
	 * swapping the cluster order. This is necessary in order to
	 * match the format of the boot information passed by the SCP
	 * and read in plat_is_my_cpu_primary below.
	 * -----------------------------------------------------------
	 */
func css_calc_core_pos_swap_cluster
	and	r1, r0, #MPIDR_CPU_MASK
	and	r0, r0, #MPIDR_CLUSTER_MASK
	eor	r0, r0, #(1 << MPIDR_AFFINITY_BITS)  // swap cluster order
	add	r0, r1, r0, LSR #6
	bx	lr
endfunc css_calc_core_pos_swap_cluster

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu (applicable ony after a cold boot)
	 * -----------------------------------------------------
	 */
#if CSS_USE_SCMI_SDS_DRIVER
func plat_is_my_cpu_primary
	mov	r10, lr
	bl	plat_my_core_pos
	mov	r4, r0
	bl	sds_get_primary_cpu_id
	/* Check for error */
	mov	r1, #0xffffffff
	cmp	r0, r1
	beq	1f
	cmp	r0, r4
	moveq	r0, #1
	movne	r0, #0
	bx	r10
1:
	no_ret	plat_panic_handler
endfunc plat_is_my_cpu_primary
#else
func plat_is_my_cpu_primary
	mov	r10, lr
	bl	plat_my_core_pos
	ldr	r1, =SCP_BOOT_CFG_ADDR
	ldr	r1, [r1]
	ubfx	r1, r1, #PLAT_CSS_PRIMARY_CPU_SHIFT, \
			#PLAT_CSS_PRIMARY_CPU_BIT_WIDTH
	cmp	r0, r1
	moveq	r0, #1
	movne	r0, #0
	bx	r10
endfunc plat_is_my_cpu_primary
#endif
