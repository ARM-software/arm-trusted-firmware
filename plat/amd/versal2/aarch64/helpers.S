/*
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/gicv3.h>

#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	platform_mem_init
	.globl	plat_my_core_pos
	.globl	plat_core_pos_by_mpidr
	.extern plat_primary_cpu_core

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

	/* Load address of var into x2. */
	ldr	x2, =plat_primary_cpu_core
	/* Load var value into x3 */
	ldr	w3, [x2]
	mov	w4, #PLAT_INVALID_CPU_CORE
	cmp	w3, w4
	/* On first instance (as it equals) primary core pos is updated. */
	b.ne	skip_var_update

	str     w0, [x2]
	mov	x3, x0

skip_var_update:
	cmp	x0, x3
	cset	x0, eq
	ret	x9
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_core_pos_by_mpidr()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_core_pos_by_mpidr
endfunc plat_my_core_pos

       /*----------------------------------------------------------------------
	* unsigned int plat_core_pos_by_mpidr(u_register_t mpid)
	*
	* Function to calculate the core position
	*
	* clobbers: x0 - x3
	* ---------------------------------------------------------------------
	*/
func plat_core_pos_by_mpidr

	/* Extract individual affinity fields from MPIDR */
	ubfx	x1, x0, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x2, x0, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS

	/* check if cpu_id valid */
	cmp     x2, #PLATFORM_CORE_COUNT_PER_CLUSTER
	b.hi    error_invalid_core

	/* check if cluster valid */
	cmp	x1, #PLATFORM_CLUSTER_COUNT
	b.hi    error_invalid_cluster

	/* Compute linear position */
	mov	x3, #PLATFORM_CORE_COUNT_PER_CLUSTER
	madd	x0, x1, x3, x2
	ret
error_invalid_cluster:
	mov     x0, #E_INVALID_CLUSTER_COUNT
	ret
error_invalid_core:
        mov     x0, #E_INVALID_CORE_COUNT
        ret
endfunc plat_core_pos_by_mpidr

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on platform
	 * The Secure RAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init
