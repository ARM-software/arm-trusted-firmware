/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
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
	ldcopr	r0, MPIDR
	and	r1, r0, #MPIDR_CPU_MASK
#ifdef PLAT_RK_MPIDR_CLUSTER_MASK
	and	r0, r0, #PLAT_RK_MPIDR_CLUSTER_MASK
#else
	and	r0, r0, #MPIDR_CLUSTER_MASK
#endif
	orr	r0, r0, r1

	/* primary_cpu */
	ldr	r1, boot_mpidr
	cmp	r0, r1
	beq	sys_wakeup

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
	ldr	r2, =__bl32_sram_stack_end
	mov     sp, r2
	bl	dmc_resume
#endif
	bl	sram_restore
sys_resume:
	bl	sp_min_warm_entrypoint
endfunc pmu_cpuson_entrypoint
