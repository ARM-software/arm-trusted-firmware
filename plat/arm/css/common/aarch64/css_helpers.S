/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
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
	 *
	 * In the normal boot flow, cold-booting secondary CPUs is not yet
	 * implemented and they panic.
	 *
	 * When booting an EL3 payload, secondary CPUs are placed in a holding
	 * pen, waiting for their mailbox to be populated. Note that all CPUs
	 * share the same mailbox ; therefore, populating it will release all
	 * CPUs from their holding pen. If finer-grained control is needed then
	 * this should be handled in the code that secondary CPUs jump to.
	 * ---------------------------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
#ifndef EL3_PAYLOAD_BASE
	/* TODO: Implement secondary CPU cold boot setup on CSS platforms */
cb_panic:
	b	cb_panic
#else
	mov_imm	x0, PLAT_ARM_TRUSTED_MAILBOX_BASE

	/* Wait until the mailbox gets populated */
poll_mailbox:
	ldr	x1, [x0]
	cbz	x1, 1f
	br	x1
1:
	wfe
	b	poll_mailbox
#endif /* EL3_PAYLOAD_BASE */
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
	mov_imm	x0, PLAT_ARM_TRUSTED_MAILBOX_BASE
	ldr	x0, [x0]
	ret
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
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	eor	x0, x0, #(1 << MPIDR_AFFINITY_BITS)  // swap cluster order
	add	x0, x1, x0, LSR #6
	ret
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
	mov	x9, x30
	bl	plat_my_core_pos
	mov	x4, x0
	bl	sds_get_primary_cpu_id
	/* Check for error */
	mov	x1, #0xffffffff
	cmp	x0, x1
	b.eq	1f
	cmp	x0, x4
	cset	w0, eq
	ret	x9
1:
	no_ret	plat_panic_handler
endfunc plat_is_my_cpu_primary
#else
func plat_is_my_cpu_primary
	mov	x9, x30
	bl	plat_my_core_pos
	mov_imm	x1, SCP_BOOT_CFG_ADDR
	ldr	x1, [x1]
	ubfx	x1, x1, #PLAT_CSS_PRIMARY_CPU_SHIFT, \
			#PLAT_CSS_PRIMARY_CPU_BIT_WIDTH
	cmp	x0, x1
	cset	w0, eq
	ret	x9
endfunc plat_is_my_cpu_primary
#endif
