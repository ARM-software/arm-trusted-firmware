/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <lib/xlat_tables/xlat_tables_v2.h>

	.global	enable_mmu_direct_svc_mon
	.global	enable_mmu_direct_hyp

	/* void enable_mmu_direct_svc_mon(unsigned int flags) */
func enable_mmu_direct_svc_mon
	/* Assert that MMU is turned off */
#if ENABLE_ASSERTIONS
	ldcopr  r1, SCTLR
	tst	r1, #SCTLR_M_BIT
	ASM_ASSERT(eq)
#endif

	/* Invalidate TLB entries */
	TLB_INVALIDATE(r0, TLBIALL)

	mov	r3, r0
	ldr	r0, =mmu_cfg_params

	/* MAIR0. Only the lower 32 bits are used. */
	ldr	r1, [r0, #(MMU_CFG_MAIR << 3)]
	stcopr	r1, MAIR0

	/* TTBCR. Only the lower 32 bits are used. */
	ldr	r2, [r0, #(MMU_CFG_TCR << 3)]
	stcopr	r2, TTBCR

	/* TTBR0 */
	ldr	r1, [r0, #(MMU_CFG_TTBR0 << 3)]
	ldr	r2, [r0, #((MMU_CFG_TTBR0 << 3) + 4)]
	stcopr16	r1, r2, TTBR0_64

	/* TTBR1 is unused right now; set it to 0. */
	mov	r1, #0
	mov	r2, #0
	stcopr16	r1, r2, TTBR1_64

	/*
	 * Ensure all translation table writes have drained into memory, the TLB
	 * invalidation is complete, and translation register writes are
	 * committed before enabling the MMU
	 */
	dsb	ish
	isb

	/* Enable enable MMU by honoring flags */
	ldcopr  r1, SCTLR
	ldr	r2, =(SCTLR_WXN_BIT | SCTLR_C_BIT | SCTLR_M_BIT)
	orr	r1, r1, r2

	/* Clear C bit if requested */
	tst	r3, #DISABLE_DCACHE
	bicne	r1, r1, #SCTLR_C_BIT

	stcopr	r1, SCTLR
	isb

	bx	lr
endfunc enable_mmu_direct_svc_mon


	/* void enable_mmu_direct_hyp(unsigned int flags) */
func enable_mmu_direct_hyp
	/* Assert that MMU is turned off */
#if ENABLE_ASSERTIONS
	ldcopr  r1, HSCTLR
	tst	r1, #HSCTLR_M_BIT
	ASM_ASSERT(eq)
#endif

	/* Invalidate TLB entries */
	TLB_INVALIDATE(r0, TLBIALL)

	mov	r3, r0
	ldr	r0, =mmu_cfg_params

	/* HMAIR0 */
	ldr	r1, [r0, #(MMU_CFG_MAIR << 3)]
	stcopr	r1, HMAIR0

	/* HTCR */
	ldr	r2, [r0, #(MMU_CFG_TCR << 3)]
	stcopr	r2, HTCR

	/* HTTBR */
	ldr	r1, [r0, #(MMU_CFG_TTBR0 << 3)]
	ldr	r2, [r0, #((MMU_CFG_TTBR0 << 3) + 4)]
	stcopr16	r1, r2, HTTBR_64

	/*
	 * Ensure all translation table writes have drained into memory, the TLB
	 * invalidation is complete, and translation register writes are
	 * committed before enabling the MMU
	 */
	dsb	ish
	isb

	/* Enable enable MMU by honoring flags */
	ldcopr  r1, HSCTLR
	ldr	r2, =(HSCTLR_WXN_BIT | HSCTLR_C_BIT | HSCTLR_M_BIT)
	orr	r1, r1, r2

	/* Clear C bit if requested */
	tst	r3, #DISABLE_DCACHE
	bicne	r1, r1, #HSCTLR_C_BIT

	stcopr	r1, HSCTLR
	isb

	bx	lr
endfunc enable_mmu_direct_hyp
