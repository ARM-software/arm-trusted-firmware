/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	uniphier_warmboot_entrypoint
	.globl	uniphier_fake_pwr_down

func uniphier_warmboot_entrypoint
	mrs	x0, mpidr_el1
	mov_imm	x1, MPIDR_AFFINITY_MASK
	and	x0, x0, x1
	b	1f
0:	wfe
1:	ldr	x1, uniphier_holding_pen_release
	cmp	x1, x0
	b.ne	0b
	ldr	x0, uniphier_sec_entrypoint
	br	x0
endfunc uniphier_warmboot_entrypoint

func uniphier_fake_pwr_down
	bl	disable_mmu_icache_el3
	b	uniphier_warmboot_entrypoint
endfunc uniphier_fake_pwr_down
