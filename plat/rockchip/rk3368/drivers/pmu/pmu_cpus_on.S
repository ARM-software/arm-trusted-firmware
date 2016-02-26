/*
 * Copyright (C) 2016, Fuzhou Rockchip Electronics Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
	 * If the core is not the primary cpu,
	 * force the core into wfe.
	 */
wfe_loop:
	wfe
	b	wfe_loop
sys_wakeup:
	/* check ddr flag for resume ddr */
	ldr	w2, [x5, #PSRAM_DT_DDRFLAG]
	.align	3
psram_data:
	.quad	PSRAM_DT_BASE
warm_boot_func:
	.quad	platform_cpu_warmboot
pmu_cpuson_entrypoint_end:
	.word	0
endfunc pmu_cpuson_entrypoint
