/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <asm_macros.S>
#include <cortex_a72.h>
#ifndef PLAT_a3700
#include <drivers/marvell/ccu.h>
#include <drivers/marvell/cache_llc.h>
#endif
#include <marvell_def.h>
#include <platform_def.h>

	.weak	plat_marvell_calc_core_pos
	.weak	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	platform_mem_init
	.globl	disable_mmu_dcache
	.globl	invalidate_tlb_all
	.globl	platform_unmap_sram
	.globl	disable_sram
	.globl	disable_icache
	.globl	invalidate_icache_all
	.globl	marvell_exit_bootrom
	.globl	ca72_l2_enable_unique_clean

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_marvell_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_marvell_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_marvell_calc_core_pos(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ClusterId * 2) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_marvell_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #7
	ret
endfunc plat_marvell_calc_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
#ifdef PLAT_a3700
	mov	x1, x30
	bl	get_ref_clk
	mov	x30, x1
	mov_imm	x1, 1000000
	mul	x1, x0, x1
#else
	mov_imm	x1, PLAT_MARVELL_UART_CLK_IN_HZ
#endif
	mov_imm	x0, PLAT_MARVELL_UART_BASE
	mov_imm	x2, MARVELL_CONSOLE_BAUDRATE
#ifdef PLAT_a3700
	b	console_a3700_core_init
#else
	b	console_16550_core_init
#endif
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, PLAT_MARVELL_UART_BASE
#ifdef PLAT_a3700

	b	console_a3700_core_putc
#else
	b	console_16550_core_putc
#endif
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * void plat_crash_console_flush()
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : void.
	 * Clobber list : r0
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, PLAT_MARVELL_UART_BASE
#ifdef PLAT_a3700
	b	console_a3700_core_flush
#else
	b	console_16550_core_flush
#endif
endfunc plat_crash_console_flush

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on ARM
	 * platforms. The Secure RAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

	/* -----------------------------------------------------
	 * Disable icache, dcache, and MMU
	 * -----------------------------------------------------
	 */
func disable_mmu_dcache
	mrs	x0, sctlr_el3
	bic	x0, x0, 0x1		/* M bit - MMU */
	bic	x0, x0, 0x4		/* C bit - Dcache L1 & L2 */
	msr	sctlr_el3, x0
	isb
	b	mmu_off
mmu_off:
	ret
endfunc disable_mmu_dcache

	/* -----------------------------------------------------
	 * Disable all TLB entries
	 * -----------------------------------------------------
	 */
func invalidate_tlb_all
	tlbi	alle3
	dsb	sy
	isb
	ret
endfunc invalidate_tlb_all

	/* -----------------------------------------------------
	 * Disable the i cache
	 * -----------------------------------------------------
	 */
func disable_icache
	mrs 	x0, sctlr_el3
	bic	x0, x0, 0x1000	/* I bit - Icache L1 & L2 */
	msr	sctlr_el3, x0
	isb
	ret
endfunc disable_icache

	/* -----------------------------------------------------
	 * Disable all of the i caches
	 * -----------------------------------------------------
	 */
func invalidate_icache_all
	ic	ialluis
	isb	sy
	ret
endfunc invalidate_icache_all

	/* -----------------------------------------------------
	 * Clear the SRAM enabling bit to unmap SRAM
	 * -----------------------------------------------------
	 */
func platform_unmap_sram
	ldr	x0, =CCU_SRAM_WIN_CR
	str	wzr, [x0]
	ret
endfunc platform_unmap_sram

	/* -----------------------------------------------------
	 * Disable the SRAM
	 * -----------------------------------------------------
	 */
func disable_sram
	/* Disable the line lockings. They must be disabled expictly
	 * or the OS will have problems using the cache */
	ldr	x1, =MASTER_LLC_TC0_LOCK
	str	wzr, [x1]

	/* Invalidate all ways */
	ldr	w1, =LLC_WAY_MASK
	ldr	x0, =MASTER_LLC_INV_WAY
	str	w1, [x0]

	/* Finally disable LLC */
	ldr	x0, =MASTER_LLC_CTRL
	str	wzr, [x0]

	ret
endfunc disable_sram

	/* -----------------------------------------------------
	 * Operation when exit bootROM:
	 * Disable the MMU
	 * Disable and invalidate the dcache
	 * Unmap and disable the SRAM
	 * Disable and invalidate the icache
	 * -----------------------------------------------------
	 */
func marvell_exit_bootrom
	/* Save the system restore address */
	mov	x28, x0

	/* Close the caches and MMU */
	bl	disable_mmu_dcache

	/*
	 * There is nothing important in the caches now,
	 * so invalidate them instead of cleaning.
	 */
	adr	x0, __RW_START__
	adr	x1, __RW_END__
	sub	x1, x1, x0
	bl	inv_dcache_range
	bl	invalidate_tlb_all

	/*
	 * Clean the memory mapping of SRAM
	 * the DDR mapping will remain to enable boot image to execute
	 */
	bl	platform_unmap_sram

	/* Disable the SRAM */
	bl	disable_sram

	/* Disable and invalidate icache */
	bl	disable_icache
	bl	invalidate_icache_all

	mov	x0, x28
	br	x0
endfunc marvell_exit_bootrom

	/*
	 * Enable L2 UniqueClean evictions with data
	 */
func ca72_l2_enable_unique_clean

	mrs	x0, CORTEX_A72_L2ACTLR_EL1
	orr	x0, x0, #CORTEX_A72_L2ACTLR_ENABLE_UNIQUE_CLEAN
	msr	CORTEX_A72_L2ACTLR_EL1, x0

	ret
endfunc ca72_l2_enable_unique_clean
