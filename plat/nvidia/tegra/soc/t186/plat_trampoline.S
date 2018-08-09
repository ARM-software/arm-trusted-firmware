/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <memctrl_v2.h>
#include <plat/common/common_def.h>
#include <tegra_def.h>

#define TEGRA186_MC_CTX_SIZE		0x93

	.globl	tegra186_cpu_reset_handler

/* CPU reset handler routine */
func tegra186_cpu_reset_handler _align=4
	/* prepare to relocate to TZSRAM */
	mov	x0, #BL31_BASE
	adr	x1, __tegra186_cpu_reset_handler_end
	adr	x2, __tegra186_cpu_reset_handler_data
	ldr	x2, [x2, #8]

	/* memcpy16 */
m_loop16:
	cmp	x2, #16
	b.lt	m_loop1
	ldp	x3, x4, [x1], #16
	stp	x3, x4, [x0], #16
	sub	x2, x2, #16
	b	m_loop16
	/* copy byte per byte */
m_loop1:
	cbz	x2, boot_cpu
	ldrb	w3, [x1], #1
	strb	w3, [x0], #1
	subs	x2, x2, #1
	b.ne	m_loop1

boot_cpu:
	adr	x0, __tegra186_cpu_reset_handler_data
	ldr	x0, [x0]
	br	x0
endfunc tegra186_cpu_reset_handler

	/*
	 * Tegra186 reset data (offset 0x0 - 0x430)
	 *
	 * 0x000: secure world's entrypoint
	 * 0x008: BL31 size (RO + RW)
	 * 0x00C: MC context start
	 * 0x42C: MC context end
	 */

	.align 4
	.type	__tegra186_cpu_reset_handler_data, %object
	.globl	__tegra186_cpu_reset_handler_data
__tegra186_cpu_reset_handler_data:
	.quad	tegra_secure_entrypoint
	.quad	__BL31_END__ - BL31_BASE

	.globl	__tegra186_system_suspend_state
__tegra186_system_suspend_state:
	.quad	0

	.align 4
	.globl	__tegra186_mc_context
__tegra186_mc_context:
	.rept	TEGRA186_MC_CTX_SIZE
	.quad	0
	.endr
	.size	__tegra186_cpu_reset_handler_data, \
		. - __tegra186_cpu_reset_handler_data

	.align 4
	.globl	__tegra186_cpu_reset_handler_end
__tegra186_cpu_reset_handler_end:

	.globl tegra186_get_cpu_reset_handler_size
	.globl tegra186_get_cpu_reset_handler_base
	.globl tegra186_get_mc_ctx_offset

/* return size of the CPU reset handler */
func tegra186_get_cpu_reset_handler_size
	adr	x0, __tegra186_cpu_reset_handler_end
	adr	x1, tegra186_cpu_reset_handler
	sub	x0, x0, x1
	ret
endfunc tegra186_get_cpu_reset_handler_size

/* return the start address of the CPU reset handler */
func tegra186_get_cpu_reset_handler_base
	adr	x0, tegra186_cpu_reset_handler
	ret
endfunc tegra186_get_cpu_reset_handler_base

/* return the size of the MC context */
func tegra186_get_mc_ctx_offset
	adr	x0, __tegra186_mc_context
	adr	x1, tegra186_cpu_reset_handler
	sub	x0, x0, x1
	ret
endfunc tegra186_get_mc_ctx_offset
