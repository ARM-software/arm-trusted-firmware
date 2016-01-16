/*
 * Copyright (c) 2014, Fuzhou Rockchip Electronics Co., Ltd
 * Author: Tony Xie <tony.xie@rock-chips.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <arch.h>
#include <asm_macros.S>
#include <rk3368_def.h>
#include <pmu.h>

	.globl pmu_cpuson_entrypoint_start
	.globl pmu_cpuson_entrypoint_end

func pmu_cpuson_entrypoint
pmu_cpuson_entrypoint_start:
	ldr	x5, psram_data
	ldr	w0, [x5, #PSRAM_DT_SYS_MODE]
	cmp	x0, #PMU_SYS_SLP_MODE
	b.eq	check_wake_cpus
	ldr	x6, warm_boot_func
	br	x6
check_wake_cpus:
	mrs	x0, MPIDR_EL1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	orr	x0, x0, x1
	/* primary_cpu */
	ldr	x1, [x3, #PSRAM_DT_MPIDR]
	cmp	x0, x1
	b.eq	sys_wakeup
	/*
	 * If the core  is not the primary cpu.
	 * force the core into wfe.
	 */
wfe_loop:
	wfe
	b	wfe_loop
sys_wakeup:
	/* check ddr flag for resume ddr */
	ldr	w2, [x5, #PSRAM_DT_DDRFLAG]
	.align 3
psram_data:
	.quad	PSRAM_DT_BASE
warm_boot_func:
	.quad	platform_cpu_warmboot
pmu_cpuson_entrypoint_end:
	.word	0
endfunc pmu_cpuson_entrypoint
