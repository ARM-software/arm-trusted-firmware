/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#if CONFIG_MTK_MCUSYS
#include <mcucfg.h>
#endif
#include <platform_def.h>
	/*
	 * Declare as weak function so that can be
	 * overwritten by platform helpers
	 */
	.weak platform_mem_init
	.weak plat_core_pos_by_mpidr
	.weak plat_my_core_pos
	.weak plat_mediatek_calc_core_pos
	.global plat_mpidr_by_core_pos
	.global plat_reset_handler

	/* -----------------------------------------------------
	 * unsigned long plat_mpidr_by_core_pos(uint32_t cpuid)
	 * This function calcuate mpidr by cpu pos if cpu
	 * topology is linear.
	 *
	 * Clobbers: x0-x1
	 * -----------------------------------------------------
	 */
func plat_mpidr_by_core_pos
	lsl x0, x0, #MPIDR_AFF1_SHIFT
	mrs x1, mpidr_el1
	and x1, x1, #MPIDR_MT_MASK
	orr x0, x0, x1
	ret
endfunc plat_mpidr_by_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_arm_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b plat_mediatek_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * int plat_mediatek_calc_core_pos(u_register_t mpidr);
	 *
	 * In ARMv8.2, AFF2 is cluster id, AFF1 is core id and
	 * AFF0 is thread id. There is only one cluster in ARMv8.2
	 * and one thread in current implementation.
	 *
	 * With this function: CorePos = CoreID (AFF1)
	 * we do it with x0 = (x0 >> 8) & 0xff
	 * -----------------------------------------------------
	 */
func plat_mediatek_calc_core_pos
	b plat_core_pos_by_mpidr
endfunc plat_mediatek_calc_core_pos

	/* ------------------------------------------------------
	 * int32_t plat_core_pos_by_mpidr(u_register_t mpidr)
	 *
	 * This function implements a part of the critical
	 * interface between the psci generic layer and the
	 * platform that allows the former to query the platform
	 * to convert an MPIDR to a unique linear index.
	 *
	 * Clobbers: x0-x1
	 * ------------------------------------------------------
	 */
func plat_core_pos_by_mpidr
	mov	x1, #MPIDR_AFFLVL_MASK
	and	x0, x1, x0, lsr #MPIDR_AFF1_SHIFT
	ret
endfunc plat_core_pos_by_mpidr

	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Any memory init, relocation to be done before the
	 * platform boots. Called very early in the boot process.
	 * --------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

func plat_reset_handler
#if CONFIG_MTK_MCUSYS
	mov	x10, x30
	bl	plat_my_core_pos
	mov	x30, x10
	mov     w1, #0x1
	lsl     w1, w1, w0
	ldr	x0, =CPC_MCUSYS_CPU_ON_SW_HINT_SET
	str     w1, [x0]
	dsb     sy
#endif

#if CONFIG_MTK_ECC
	mov x10, x30
	/* enable sequence of ecc for cpus */
	bl disable_core_ecc
	bl ft_ecc_clear_per_core
	bl enable_core_ecc
	mov x30, x10
#endif

	ret
endfunc plat_reset_handler
