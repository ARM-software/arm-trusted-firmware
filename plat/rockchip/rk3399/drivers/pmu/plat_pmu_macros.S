/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>
#include <pmu_regs.h>

	.globl	clst_warmboot_data

	.macro sram_func _name
	.cfi_sections .debug_frame
	.section .sram.text, "ax"
	.type \_name, %function
	.cfi_startproc
	\_name:
	.endm

#define CRU_CLKSEL_CON6	0x118

#define DDRCTL0_C_SYSREQ_CFG 0x0100
#define DDRCTL1_C_SYSREQ_CFG 0x1000

#define DDRC0_SREF_DONE_EXT 0x01
#define DDRC1_SREF_DONE_EXT 0x04

#define PLL_MODE_SHIFT	(0x8)
#define PLL_NORMAL_MODE	((0x3 << (PLL_MODE_SHIFT + 16)) | \
						 (0x1 << PLL_MODE_SHIFT))
#define MPIDR_CLST_L_BITS 0x0
	/*
	 * For different socs, if we want to speed up warmboot,
	 * we need to config some regs here.
	 * If scu was suspend, we must resume related clk
	 * from slow (24M) mode to normal mode first.
	 * X0: MPIDR_EL1 & MPIDR_CLUSTER_MASK
	 */
.macro	func_rockchip_clst_warmboot
	adr	x4, clst_warmboot_data
	lsr	x5, x0, #6
	ldr	w3, [x4, x5]
	str	wzr, [x4, x5]
	cmp	w3, #PMU_CLST_RET
	b.ne	clst_warmboot_end
	ldr	w6, =(PLL_NORMAL_MODE)
	/*
	 * core_l offset is CRU_BASE + 0xc,
	 * core_b offset is CRU_BASE + 0x2c
	 */
	ldr	x7, =(CRU_BASE + 0xc)
	lsr	x2, x0, #3
	str	w6, [x7, x2]
clst_warmboot_end:
.endm

.macro rockchip_clst_warmboot_data
clst_warmboot_data:
	.rept	PLATFORM_CLUSTER_COUNT
	.word	0
	.endr
.endm

	/* -----------------------------------------------
	 * void sram_func_set_ddrctl_pll(uint32_t pll_src)
	 * Function to switch the PLL source for ddrctrl
	 * In: x0 - The PLL of the clk_ddrc clock source
	 * out: None
	 * Clobber list : x0 - x3, x5, x8 - x10
	 * -----------------------------------------------
	 */

	.globl	sram_func_set_ddrctl_pll

sram_func sram_func_set_ddrctl_pll
	/* backup parameter */
	mov	x8, x0

	/* disable the MMU at EL3 */
	mrs 	x9, sctlr_el3
	bic	x10, x9, #(SCTLR_M_BIT)
	msr 	sctlr_el3, x10
	isb
	dsb 	sy

	/* enable ddrctl0_1 idle request */
	mov	x5, PMU_BASE
	ldr	w0, [x5, #PMU_SFT_CON]
	orr	w0, w0, #DDRCTL0_C_SYSREQ_CFG
	orr	w0, w0, #DDRCTL1_C_SYSREQ_CFG
	str	w0, [x5, #PMU_SFT_CON]

check_ddrc0_1_sref_enter:
	ldr	w1, [x5, #PMU_DDR_SREF_ST]
	and	w2, w1, #DDRC0_SREF_DONE_EXT
	and	w3, w1, #DDRC1_SREF_DONE_EXT
	orr	w2, w2, w3
	cmp	w2, #(DDRC0_SREF_DONE_EXT | DDRC1_SREF_DONE_EXT)
	b.eq	check_ddrc0_1_sref_enter

	/*
	 * select a PLL for ddrctrl:
	 * x0 = 0: ALPLL
	 * x0 = 1: ABPLL
	 * x0 = 2: DPLL
	 * x0 = 3: GPLLL
	 */
	mov     x5, CRU_BASE
	lsl	w0, w8, #4
	orr	w0, w0, #0x00300000
	str 	w0, [x5, #CRU_CLKSEL_CON6]

	/* disable ddrctl0_1 idle request */
	mov	x5, PMU_BASE
	ldr	w0, [x5, #PMU_SFT_CON]
	bic	w0, w0, #DDRCTL0_C_SYSREQ_CFG
	bic	w0, w0, #DDRCTL1_C_SYSREQ_CFG
	str	w0, [x5, #PMU_SFT_CON]

check_ddrc0_1_sref_exit:
	ldr	w1, [x5, #PMU_DDR_SREF_ST]
	and	w2, w1, #DDRC0_SREF_DONE_EXT
	and	w3, w1, #DDRC1_SREF_DONE_EXT
	orr	w2, w2, w3
	cmp	w2, #0x0
	b.eq	check_ddrc0_1_sref_exit

	/* reenable the MMU at EL3 */
	msr 	sctlr_el3, x9
	isb
	dsb 	sy

	ret
endfunc sram_func_set_ddrctl_pll
