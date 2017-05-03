/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>
#include <pmu_sram.h>

	.globl pmu_cpuson_entrypoint_start
	.globl pmu_cpuson_entrypoint_end

func pmu_cpuson_entrypoint
pmu_cpuson_entrypoint_start:
	ldr	x5, psram_data
check_wake_cpus:
	mrs	x0, MPIDR_EL1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	orr	x0, x0, x1
	/* primary_cpu */
	ldr	w1, [x5, #PSRAM_DT_MPIDR]
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
	/* check ddr flag for resume ddr */
	ldr	w2, [x5, #PSRAM_DT_DDRFLAG]
	cmp	w2, #0x0
	b.eq	sys_resume
ddr_resume:
	ldr	x2, [x5, #PSRAM_DT_SP]
	mov	sp, x2
	ldr	x1, [x5, #PSRAM_DT_DDR_FUNC]
	ldr	x0, [x5, #PSRAM_DT_DDR_DATA]
	blr	x1
sys_resume:
	ldr	x1, sys_wakeup_entry
	br	x1

	.align	3
psram_data:
	.quad	PSRAM_DT_BASE
sys_wakeup_entry:
	.quad	psci_entrypoint
pmu_cpuson_entrypoint_end:
	.word	0
endfunc pmu_cpuson_entrypoint
