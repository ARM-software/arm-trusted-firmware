/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl pmu_cpuson_entrypoint
	.macro pmusram_entry_func _name
	.section .pmusram.entry, "ax"
	.type \_name, %function
	.func \_name
	.cfi_startproc
	\_name:
	.endm

pmusram_entry_func pmu_cpuson_entrypoint

#if PSRAM_DO_DDR_RESUME
ddr_resume:
	ldr	x2, =__bl31_sram_stack_end
	mov     sp, x2
	bl	dmc_restore
#endif
	bl	sram_restore
sys_resume:
	bl	psci_entrypoint
endfunc pmu_cpuson_entrypoint
