/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/gicv3.h>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	versal_calc_core_pos
	.globl	platform_mem_init
	.globl	plat_my_core_pos

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * TODO: Should we read the PSYS register to make sure
	 * that the request has gone through.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	mrs	x0, mpidr_el1

	/*
	 * There is no sane reason to come out of this wfi. This
	 * cpu will be powered on and reset by the cpu_on pm api
	 */
	dsb	sy
	bl	plat_panic_handler
endfunc plat_secondary_cold_boot_setup

func plat_is_my_cpu_primary
	mov	x9, x30
	bl	plat_my_core_pos
	cmp	x0, #VERSAL_PRIMARY_CPU
	cset	x0, eq
	ret	x9
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the versal_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	versal_calc_core_pos
endfunc plat_my_core_pos

func versal_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc versal_calc_core_pos

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on VERSAL
	 * platform. The Secure RAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init
