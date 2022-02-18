/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	plat_reset_handler
	.globl  platform_mem_init

func platform_mem1_init
	ret
endfunc platform_mem1_init

func platform_mem_init
	ret
endfunc	platform_mem_init

func apply_platform_errata
	ret
endfunc apply_platform_errata

func plat_reset_handler
	mov	x29, x30
	bl	apply_platform_errata

#if defined(IMAGE_BL31)
	ldr	x0, =POLICY_SMMU_PAGESZ_64K
	cbz	x0, 1f
	/* Set the SMMU page size in the sACR register */
	bl	_set_smmu_pagesz_64
#endif
1:
	mov	x30, x29
	ret
endfunc plat_reset_handler

	/*
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 */
func plat_secondary_cold_boot_setup
	/* ls1088a does not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

	/*
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, 0x0
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary
