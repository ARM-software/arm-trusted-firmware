/*
 * Copyright 2022-2026 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_a55.h>

#include <platform_def.h>

	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_calc_core_pos
	.globl	plat_arm_calc_core_pos
	.globl	platform_mem_init
	.globl	plat_reset_handler

	/* ----------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * This function checks if this is the primary CPU
	 * ----------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	mov_imm x1, MPIDR_AFFINITY_MASK
	and	x0, x0, x1
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ----------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * This Function uses the plat_calc_core_pos()
	 * to get the index of the calling CPU.
	 * ----------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	mov	x1, #MPIDR_AFFLVL_MASK
	and	x0, x1, x0, lsr #MPIDR_AFF1_SHIFT
	ret
endfunc plat_my_core_pos

	/*
	 * unsigned int plat_calc_core_pos(uint64_t mpidr)
	 * helper function to calculate the core position.
	 * With this function.
	 */
func plat_calc_core_pos
	mov	x1, #MPIDR_AFFLVL_MASK
	and	x0, x1, x0, lsr #MPIDR_AFF1_SHIFT
	ret
endfunc plat_calc_core_pos

func plat_arm_calc_core_pos
	b	plat_core_pos_by_mpidr
endfunc plat_arm_calc_core_pos

func platform_mem_init
	ret
endfunc platform_mem_init

func plat_reset_handler
#if defined(PLAT_imx93)
	/* Check if IRQ22 (s401 halt_ack) is pending in GPC IMR6 */
	mov	x0, #GPC_BASE
	ldr	w1, [x0, #0x968]
	tbnz	w1, #28, suspend
	ret
suspend:
	/* Clear the s401 halt_ack req via BLK_CTRL_S LPCG_A55 */
	mov	x0, #BLK_CTRL_S_BASE
	mov	w1, #0x100
	str	w1, [x0, #0x134]
	/* Delay for halt_ack deassert propagation */
	nop
	nop
	str	wzr, [x0, #0x134]

	/* Enable core power down */
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb

	/* Enable cluster power down */
	mrs	x0, CLUSTERPWRDN_EL1
	orr	x0, x0, #DSU_CLUSTER_PWR_OFF | BIT(1)
	msr	CLUSTERPWRDN_EL1, x0
	isb

	wfi
#else
	ret
#endif
endfunc plat_reset_handler
