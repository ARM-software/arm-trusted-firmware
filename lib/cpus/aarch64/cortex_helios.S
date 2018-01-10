/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cortex_helios.h>
#include <cpu_macros.S>
#include <debug.h>
#include <plat_macros.S>

func cortex_helios_cpu_pwr_dwn
	mrs	x0, CORTEX_HELIOS_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_HELIOS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_HELIOS_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_helios_cpu_pwr_dwn

.section .rodata.cortex_helios_regs, "aS"
cortex_helios_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_helios_cpu_reg_dump
	adr	x6, cortex_helios_regs
	mrs	x8, CORTEX_HELIOS_ECTLR_EL1
	ret
endfunc cortex_helios_cpu_reg_dump

declare_cpu_ops cortex_helios, CORTEX_HELIOS_MIDR, \
	CPU_NO_RESET_FUNC, \
	cortex_helios_cpu_pwr_dwn
