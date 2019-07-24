/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>
#include <cpus_on_fixed_addr.h>

	.globl sys_sleep_flag_sram
	.globl pmu_cpuson_entrypoint

	.macro pmusram_entry_func _name
	.section .pmusram.entry, "ax"
	.type \_name, %function
	.cfi_startproc
	\_name:
	.endm

pmusram_entry_func pmu_cpuson_entrypoint
	adr	x5, sys_sleep_flag_sram
	ldr	w2, [x5, #PSRAM_DT_PM_FLAG]

	tbz	w2, #PM_WARM_BOOT_SHT, sys_resume_sp
	ldr	x1, =platform_cpu_warmboot
	br	x1
sys_resume_sp:
	adr	x5, sys_sleep_flag_sram
	ldr	x1, [x5, #PSRAM_DT_SP]
	mov	sp, x1
ddr_resume:
	ldr	x1, [x5, #PSRAM_DT_DDR_FUNC]
	cmp	x1, #0
	b.eq	sys_resume
	blr	x1
sys_resume:
	ldr	x1, =bl31_warm_entrypoint
	br	x1
endfunc pmu_cpuson_entrypoint

	.section .pmusram.data, "a"
	.align  3
sys_sleep_flag_sram:
	.rept	PSRAM_DT_SIZE_WORDS
	.word	0
	.endr
