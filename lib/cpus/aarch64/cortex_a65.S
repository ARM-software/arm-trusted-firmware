/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>

#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a65.h>
#include <cpu_macros.S>
#include <dsu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if !HW_ASSISTED_COHERENCY
#error "Cortex-A65 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS
#error "Cortex-A65 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a65

workaround_reset_start cortex_a65, ERRATUM(936184), ERRATA_DSU_936184
	errata_dsu_936184_wa_impl
workaround_reset_end cortex_a65, ERRATUM(936184)

check_erratum_custom_start cortex_a65, ERRATUM(936184)
	check_errata_dsu_936184_impl
	ret
check_erratum_custom_end cortex_a65, ERRATUM(936184)

workaround_reset_start cortex_a65, ERRATUM(1179935), ERRATA_A65_1179935
	sysreg_bit_set CORTEX_A65_CPUACTLR_EL1, BIT(49)
workaround_reset_end cortex_a65, ERRATUM(1179935)

check_erratum_ls cortex_a65, ERRATUM(1179935), CPU_REV(0, 0)

workaround_reset_start cortex_a65, ERRATUM(1227419), ERRATA_A65_1227419
	sysreg_bit_set CORTEX_A65_CPUACTLR_EL1, BIT(51)
workaround_reset_end cortex_a65, ERRATUM(1227419)

check_erratum_ls cortex_a65, ERRATUM(1227419), CPU_REV(1, 0)

/* Due to the nature of the errata it is applied unconditionally when chosen */
check_erratum_chosen cortex_a65, ERRATUM(1541130), ERRATA_A65_1541130
/* erratum workaround is interleaved with generic code */
add_erratum_entry cortex_a65, ERRATUM(1541130), ERRATA_A65_1541130

cpu_reset_func_start cortex_a65
cpu_reset_func_end cortex_a65

func cortex_a65_cpu_pwr_dwn
	mrs	x0, CORTEX_A65_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A65_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_A65_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a65_cpu_pwr_dwn

.section .rodata.cortex_a65_regs, "aS"
cortex_a65_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a65_cpu_reg_dump
	adr	x6, cortex_a65_regs
	mrs	x8, CORTEX_A65_ECTLR_EL1
	ret
endfunc cortex_a65_cpu_reg_dump

declare_cpu_ops cortex_a65, CORTEX_A65_MIDR, \
	cortex_a65_reset_func, \
	cortex_a65_cpu_pwr_dwn
