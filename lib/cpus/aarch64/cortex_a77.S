/*
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a77.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A77 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A77 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A77_BHB_LOOP_COUNT, cortex_a77
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_a77, ERRATUM(1508412), ERRATA_A77_1508412
	/* move cpu revision in again and compare against r0p0 */
	mov	x0, x7
	mov	x1, #CPU_REV(0, 0)
	bl	cpu_rev_var_ls
	cbz	x0, 1f

	ldr	x0, =0x0
	msr	CORTEX_A77_CPUPSELR_EL3, x0
	ldr 	x0, =0x00E8400000
	msr	CORTEX_A77_CPUPOR_EL3, x0
	ldr	x0, =0x00FFE00000
	msr	CORTEX_A77_CPUPMR_EL3, x0
	ldr	x0, =0x4004003FF
	msr	CORTEX_A77_CPUPCR_EL3, x0
	ldr	x0, =0x1
	msr	CORTEX_A77_CPUPSELR_EL3, x0
	ldr	x0, =0x00E8C00040
	msr	CORTEX_A77_CPUPOR_EL3, x0
	ldr	x0, =0x00FFE00040
	msr	CORTEX_A77_CPUPMR_EL3, x0
	b	2f
1:
	ldr	x0, =0x0
	msr	CORTEX_A77_CPUPSELR_EL3, x0
	ldr	x0, =0x00E8400000
	msr	CORTEX_A77_CPUPOR_EL3, x0
	ldr	x0, =0x00FF600000
	msr	CORTEX_A77_CPUPMR_EL3, x0
	ldr	x0, =0x00E8E00080
	msr	CORTEX_A77_CPUPOR2_EL3, x0
	ldr	x0, =0x00FFE000C0
	msr	CORTEX_A77_CPUPMR2_EL3, x0
2:
	ldr	x0, =0x04004003FF
	msr	CORTEX_A77_CPUPCR_EL3, x0
workaround_reset_end cortex_a77, ERRATUM(1508412)

check_erratum_ls cortex_a77, ERRATUM(1508412), CPU_REV(1, 0)

workaround_reset_start cortex_a77, ERRATUM(1791578), ERRATA_A77_1791578
	sysreg_bit_set CORTEX_A77_ACTLR2_EL1, CORTEX_A77_ACTLR2_EL1_BIT_2
workaround_reset_end cortex_a77, ERRATUM(1791578)

check_erratum_ls cortex_a77, ERRATUM(1791578), CPU_REV(1, 1)

workaround_reset_start cortex_a77, ERRATUM(1800714), ERRATA_A77_1800714
	/* Disable allocation of splintered pages in the L2 TLB */
	sysreg_bit_set CORTEX_A77_CPUECTLR_EL1, CORTEX_A77_CPUECTLR_EL1_BIT_53
workaround_reset_end cortex_a77, ERRATUM(1800714)

check_erratum_ls cortex_a77, ERRATUM(1800714), CPU_REV(1, 1)

workaround_reset_start cortex_a77, ERRATUM(1925769), ERRATA_A77_1925769
	sysreg_bit_set CORTEX_A77_CPUECTLR_EL1, CORTEX_A77_CPUECTLR_EL1_BIT_8
workaround_reset_end cortex_a77, ERRATUM(1925769)

check_erratum_ls cortex_a77, ERRATUM(1925769), CPU_REV(1, 1)

workaround_reset_start cortex_a77, ERRATUM(1946167), ERRATA_A77_1946167
	ldr	x0,=0x4
	msr	CORTEX_A77_CPUPSELR_EL3,x0
	ldr	x0,=0x10E3900002
	msr	CORTEX_A77_CPUPOR_EL3,x0
	ldr	x0,=0x10FFF00083
	msr	CORTEX_A77_CPUPMR_EL3,x0
	ldr	x0,=0x2001003FF
	msr	CORTEX_A77_CPUPCR_EL3,x0

	ldr	x0,=0x5
	msr	CORTEX_A77_CPUPSELR_EL3,x0
	ldr	x0,=0x10E3800082
	msr	CORTEX_A77_CPUPOR_EL3,x0
	ldr	x0,=0x10FFF00083
	msr	CORTEX_A77_CPUPMR_EL3,x0
	ldr	x0,=0x2001003FF
	msr	CORTEX_A77_CPUPCR_EL3,x0

	ldr	x0,=0x6
	msr	CORTEX_A77_CPUPSELR_EL3,x0
	ldr	x0,=0x10E3800200
	msr	CORTEX_A77_CPUPOR_EL3,x0
	ldr	x0,=0x10FFF003E0
	msr	CORTEX_A77_CPUPMR_EL3,x0
	ldr	x0,=0x2001003FF
	msr	CORTEX_A77_CPUPCR_EL3,x0
workaround_reset_end cortex_a77, ERRATUM(1946167)

check_erratum_ls cortex_a77, ERRATUM(1946167), CPU_REV(1, 1)

workaround_reset_start cortex_a77, ERRATUM(2356587), ERRATA_A77_2356587
	sysreg_bit_set CORTEX_A77_ACTLR2_EL1, CORTEX_A77_ACTLR2_EL1_BIT_0
workaround_reset_end cortex_a77, ERRATUM(2356587)

check_erratum_ls cortex_a77, ERRATUM(2356587), CPU_REV(1, 1)

workaround_runtime_start cortex_a77, ERRATUM(2743100), ERRATA_A77_2743100
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end cortex_a77, ERRATUM(2743100), NO_ISB

check_erratum_ls cortex_a77, ERRATUM(2743100), CPU_REV(1, 1)

workaround_reset_start cortex_a77, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-A77 generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_a77
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a77, CVE(2022, 23960)

check_erratum_chosen cortex_a77, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A77. Must follow AAPCS.
	 * -------------------------------------------------
	 */
cpu_reset_func_start cortex_a77
cpu_reset_func_end cortex_a77

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a77_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set CORTEX_A77_CPUPWRCTLR_EL1, \
		CORTEX_A77_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	apply_erratum cortex_a77, ERRATUM(2743100), ERRATA_A77_2743100

	isb
	ret
endfunc cortex_a77_core_pwr_dwn

errata_report_shim cortex_a77
	/* ---------------------------------------------
	 * This function provides Cortex-A77 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a77_regs, "aS"
cortex_a77_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a77_cpu_reg_dump
	adr	x6, cortex_a77_regs
	mrs	x8, CORTEX_A77_CPUECTLR_EL1
	ret
endfunc cortex_a77_cpu_reg_dump

declare_cpu_ops cortex_a77, CORTEX_A77_MIDR, \
	cortex_a77_reset_func, \
	cortex_a77_core_pwr_dwn
