/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
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

func l2_mem_init
	/* Initialize the L2 RAM latency */
	mrs	x1, S3_1_c11_c0_2
	mov	x0, #0x1C7
	/* Clear L2 Tag RAM latency and L2 Data RAM latency */
	bic	x1, x1, x0
	/* Set L2 data ram latency bits [2:0] */
	orr	x1, x1, #0x2
	/* set L2 tag ram latency bits [8:6] */
	orr	x1,  x1, #0x80
	msr	S3_1_c11_c0_2, x1
	isb
	ret
endfunc	l2_mem_init

func apply_platform_errata
	ret
endfunc apply_platform_errata

func plat_reset_handler
	mov	x29, x30
#if (defined(IMAGE_BL2) && BL2_AT_EL3)
	bl	l2_mem_init
#endif
	bl	apply_platform_errata

#if defined(IMAGE_BL31)
	ldr	x0, =POLICY_SMMU_PAGESZ_64K
	cbz	x0, 1f
	/* Set the SMMU page size in the SACR register */
	bl	_set_smmu_pagesz_64
#endif
1:
	/*
	 * May be cntfrq_el0 needs to be assigned
	 * the value COUNTER_FREQUENCY
	 */
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
	/* ls1046a does not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

/*
 * unsigned int plat_is_my_cpu_primary (void);
 *
 * Find out whether the current cpu is the primary cpu.
 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, 0x0
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary
