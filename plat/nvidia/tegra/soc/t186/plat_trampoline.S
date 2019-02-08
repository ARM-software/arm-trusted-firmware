/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <memctrl_v2.h>
#include <plat/common/common_def.h>
#include <tegra_def.h>

#define TEGRA186_STATE_SYSTEM_SUSPEND	0x5C7
#define TEGRA186_STATE_SYSTEM_RESUME	0x600D
#define TEGRA186_SMMU_CTX_SIZE		0x420

	.globl	tegra186_cpu_reset_handler

/* CPU reset handler routine */
func tegra186_cpu_reset_handler _align=4
	/* check if we are exiting system suspend state */
	adr	x0, __tegra186_system_suspend_state
	ldr	x1, [x0]
	mov	x2, #TEGRA186_STATE_SYSTEM_SUSPEND
	lsl	x2, x2, #16
	add	x2, x2, #TEGRA186_STATE_SYSTEM_SUSPEND
	cmp	x1, x2
	bne	boot_cpu

	/* set system resume state */
	mov	x1, #TEGRA186_STATE_SYSTEM_RESUME
	lsl	x1, x1, #16
	mov	x2, #TEGRA186_STATE_SYSTEM_RESUME
	add	x1, x1, x2
	str	x1, [x0]
	dsb	sy

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
	 * 0x00C: SMMU context start
	 * 0x42C: SMMU context end
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
	.globl	__tegra186_smmu_context
__tegra186_smmu_context:
	.rept	TEGRA186_SMMU_CTX_SIZE
	.quad	0
	.endr
	.size	__tegra186_cpu_reset_handler_data, \
		. - __tegra186_cpu_reset_handler_data

	.align 4
	.globl	__tegra186_cpu_reset_handler_end
__tegra186_cpu_reset_handler_end:

	.globl tegra186_get_cpu_reset_handler_size
	.globl tegra186_get_cpu_reset_handler_base
	.globl tegra186_get_smmu_ctx_offset
	.globl tegra186_set_system_suspend_entry

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

/* return the size of the SMMU context */
func tegra186_get_smmu_ctx_offset
	adr	x0, __tegra186_smmu_context
	adr	x1, tegra186_cpu_reset_handler
	sub	x0, x0, x1
	ret
endfunc tegra186_get_smmu_ctx_offset

/* set system suspend state before SC7 entry */
func tegra186_set_system_suspend_entry
	mov	x0, #TEGRA_MC_BASE
	mov	x3, #MC_SECURITY_CFG3_0
	ldr	w1, [x0, x3]
	lsl	x1, x1, #32
	mov	x3, #MC_SECURITY_CFG0_0
	ldr	w2, [x0, x3]
	orr	x3, x1, x2			/* TZDRAM base */
	adr	x0, __tegra186_system_suspend_state
	adr	x1, tegra186_cpu_reset_handler
	sub	x2, x0, x1			/* offset in TZDRAM */
	mov	x0, #TEGRA186_STATE_SYSTEM_SUSPEND
	lsl	x0, x0, #16
	add	x0, x0, #TEGRA186_STATE_SYSTEM_SUSPEND
	str	x0, [x3, x2]			/* set value in TZDRAM */
	dsb	sy
	ret
endfunc tegra186_set_system_suspend_entry
