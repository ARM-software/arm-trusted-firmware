/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>

#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a65ae.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if !HW_ASSISTED_COHERENCY
#error "Cortex-A65AE must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS
#error "Cortex-A65AE supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

 /*
  * ERRATA_DSU_936184 :
  * The errata is defined in dsu_helpers.S but applies to cortex_a65ae
  * as well. Henceforth creating symbolic names to the already existing errata
  * workaround functions to get them registered under the Errata Framework.
  */
.equ check_erratum_cortex_a65ae_936184, check_errata_dsu_936184
.equ erratum_cortex_a65ae_936184_wa, errata_dsu_936184_wa
add_erratum_entry cortex_a65ae, ERRATUM(936184), ERRATA_DSU_936184, APPLY_AT_RESET

cpu_reset_func_start cortex_a65ae
cpu_reset_func_end cortex_a65ae

func cortex_a65ae_cpu_pwr_dwn
	sysreg_bit_set CORTEX_A65AE_CPUPWRCTLR_EL1, CORTEX_A65AE_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_a65ae_cpu_pwr_dwn

errata_report_shim cortex_a65ae

.section .rodata.cortex_a65ae_regs, "aS"
cortex_a65ae_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a65ae_cpu_reg_dump
	adr	x6, cortex_a65ae_regs
	mrs	x8, CORTEX_A65AE_ECTLR_EL1
	ret
endfunc cortex_a65ae_cpu_reg_dump

declare_cpu_ops cortex_a65ae, CORTEX_A65AE_MIDR, \
	cortex_a65ae_reset_func, \
	cortex_a65ae_cpu_pwr_dwn
