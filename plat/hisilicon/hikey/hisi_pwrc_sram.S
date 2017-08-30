/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_a53.h>
#include <hi6220.h>
#include <hisi_sram_map.h>

	.global pm_asm_code
	.global pm_asm_code_end
	.global v7_asm
	.global v7_asm_end

func pm_asm_code _align=3
	mov	x0, 0
	msr	oslar_el1, x0

	mrs	x0, CORTEX_A53_CPUACTLR_EL1
	bic	x0, x0, #(CORTEX_A53_CPUACTLR_EL1_RADIS | \
				CORTEX_A53_CPUACTLR_EL1_L1RADIS)
	orr	x0, x0, #0x180000
	orr	x0, x0, #0xe000
	msr	CORTEX_A53_CPUACTLR_EL1, x0

	mrs	x3, actlr_el3
	orr	x3, x3, #ACTLR_EL3_L2ECTLR_BIT
	msr	actlr_el3, x3

	mrs	x3, actlr_el2
	orr	x3, x3, #ACTLR_EL2_L2ECTLR_BIT
	msr	actlr_el2, x3

	ldr	x3, =PWRCTRL_ACPU_ASM_D_ARM_PARA_AD
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
pen:	ldr	x4, [x3, x0, LSL #3]
	cbz	x4, pen

	mov	x0, #0x0
	mov	x1, #0x0
	mov	x2, #0x0
	mov	x3, #0x0
	br	x4

	.ltorg

pm_asm_code_end:
endfunc pm_asm_code

	/*
	 * By default, all cores in Hi6220 reset with aarch32 mode.
	 * Now hardcode ARMv7 instructions to execute warm reset for
	 * switching aarch64 mode.
	 */
	.align	3
	.section .rodata.v7_asm, "aS"
v7_asm:
	.word	0xE1A00000	// nop
	.word	0xE3A02003	// mov r2, #3
	.word	0xEE0C2F50	// mcr 15, 0, r2, cr12, cr0, {2}
	.word	0xE320F003	// wfi

	.ltorg
v7_asm_end:
