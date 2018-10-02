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
	.cfi_startproc
	\_name:
	.endm

pmusram_entry_func pmu_cpuson_entrypoint

#if PSRAM_CHECK_WAKEUP_CPU
check_wake_cpus:
	mrs	x0, MPIDR_EL1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	orr	x0, x0, x1

	/* primary_cpu */
	ldr	w1, boot_mpidr
	cmp	w0, w1
	b.eq	sys_wakeup

	/*
	 * If the core is not the primary cpu,
	 * force the core into wfe.
	 */
wfe_loop:
	wfe
	b	wfe_loop
sys_wakeup:
#endif

#if PSRAM_DO_DDR_RESUME
ddr_resume:
	ldr	x2, =__bl31_sram_stack_end
	mov     sp, x2
	bl	dmc_resume
#endif
	bl	sram_restore
sys_resume:
	bl	bl31_warm_entrypoint
endfunc pmu_cpuson_entrypoint
