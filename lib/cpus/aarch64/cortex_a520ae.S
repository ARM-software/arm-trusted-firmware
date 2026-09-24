/*
 * Copyright (c) 2021-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TRM: https://support.arm.com/documentation/107726/latest
 * SDEN: https://support.arm.com/documentation/SDEN-2343003/latest
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a520.h>
#include <cpu_macros.S>
#include <dsu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex A520 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex A520 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a520ae

workaround_reset_start cortex_a520ae, ERRATUM(3674306), ERRATA_A520_AE_3674306
	sysreg_bit_set CORTEX_A520_CPUACTLR_EL1, BIT(27)
workaround_reset_end cortex_a520ae, ERRATUM(3674306)

check_erratum_chosen cortex_a520ae, ERRATUM(3674306), ERRATA_A520_AE_3674306

workaround_reset_start cortex_a520ae, ERRATUM(3711577), ERRATA_A520_AE_3711577
	sysreg_bit_set CORTEX_A520_CPUACTLR_EL1, BIT(9)
workaround_reset_end cortex_a520ae, ERRATUM(3711577)

check_erratum_chosen cortex_a520ae, ERRATUM(3711577), ERRATA_A520_AE_3711577

func cortex_a520ae_core_pwr_dwn
	sysreg_bit_set CORTEX_A520_CPUPWRCTLR_EL1, CORTEX_A520_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_a520ae_core_pwr_dwn

cpu_reset_func_start cortex_a520ae
	/* Disable speculative loads */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end cortex_a520ae


declare_cpu_ops cortex_a520ae, CORTEX_A520_MIDR, \
	cortex_a520ae_reset_func, \
	cortex_a520ae_core_pwr_dwn
