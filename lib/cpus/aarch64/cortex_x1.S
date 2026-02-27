/*
 * Copyright (c) 2022-2024, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_x1.h>
#include <cpu_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-X1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-X1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_X1_BHB_LOOP_COUNT, cortex_x1
#endif /* WORKAROUND_CVE_2022_23960 */

cpu_reset_prologue cortex_x1

workaround_reset_start cortex_x1, ERRATUM(1467580), ERRATA_X1_1467580
	ldr x0, =0x7
	msr s3_6_c15_c8_0, x0
	ldr x0, =0xf3d08000
	msr s3_6_c15_c8_2, x0
	ldr x0, =0xfff0f0ff
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x80000002003ff
	msr s3_6_c15_c8_1, x0
workaround_reset_end cortex_x1, ERRATUM(1467580)

check_erratum_ls cortex_x1, ERRATUM(1467580), CPU_REV(0, 0)

workaround_reset_start cortex_x1, ERRATUM(1492189), ERRATA_X1_1492189
	sysreg_bit_set CORTEX_X1_ACTLR5_EL1, BIT(8)
workaround_reset_end cortex_x1, ERRATUM(1492189)

check_erratum_ls cortex_x1, ERRATUM(1492189), CPU_REV(0, 0)

workaround_reset_start cortex_x1, ERRATUM(1503072), ERRATA_X1_1503072
	ldr x0, =0x0
	msr s3_6_c15_c8_0, x0
	ldr x0, =0xee070f14
	msr s3_6_c15_c8_2,x0
	ldr x0, =0xffff0fff
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x4005027ff
	msr s3_6_c15_c8_1, x0

	ldr x0, =0x1
	msr s3_6_c15_c8_0, x0
	ldr x0, =0x00e8400000
	msr s3_6_c15_c8_2, x0
	ldr x0, =0x00fff00000
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x4001027ff
	msr s3_6_c15_c8_1, x0

	ldr x0, =0x2
	msr s3_6_c15_c8_0, x0
	ldr x0, =0x00e8c00040
	msr s3_6_c15_c8_2, x0
	ldr x0, =0x00fff00040
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x4001027ff
	msr s3_6_c15_c8_1, x0

	ldr x0, =0x3
	msr s3_6_c15_c8_0, x0
	ldr x0, =0x00e8400000
	msr s3_6_c15_c8_2, x0
	ldr x0, =0x00fff00000
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x4004027ff
	msr s3_6_c15_c8_1, x0

	ldr x0, =0x4
	msr s3_6_c15_c8_0, x0
	ldr x0, =0x00e8c00040
	msr s3_6_c15_c8_2, x0
	ldr x0, =0x00fff00040
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x4004027ff
	msr s3_6_c15_c8_1, x0
workaround_reset_end cortex_x1, ERRATUM(1503072)

check_erratum_ls cortex_x1, ERRATUM(1503072), CPU_REV(0, 0)

workaround_reset_start cortex_x1, ERRATUM(1515634), ERRATA_X1_1515634
	sysreg_bit_set CORTEX_X1_CPUACTLR_EL1, BIT(11)
workaround_reset_end cortex_x1, ERRATUM(1515634)

check_erratum_ls cortex_x1, ERRATUM(1515634), CPU_REV(0, 0)

workaround_reset_start cortex_x1, ERRATUM(1688305), ERRATA_X1_1688305
	sysreg_bit_set CORTEX_X1_ACTLR2_EL1, BIT(1)
workaround_reset_end cortex_x1, ERRATUM(1688305)

check_erratum_ls cortex_x1, ERRATUM(1688305), CPU_REV(1, 0)

workaround_reset_start cortex_x1, ERRATUM(1821534), ERRATA_X1_1821534
	sysreg_bit_set CORTEX_X1_ACTLR2_EL1, BIT(2)
workaround_reset_end cortex_x1, ERRATUM(1821534)

check_erratum_ls cortex_x1, ERRATUM(1821534), CPU_REV(1, 0)

workaround_reset_start cortex_x1, ERRATUM(1827429), ERRATA_X1_1827429
	sysreg_bit_set CORTEX_X1_CPUECTLR_EL1, BIT(53)
workaround_reset_end cortex_x1, ERRATUM(1827429)

check_erratum_ls cortex_x1, ERRATUM(1827429), CPU_REV(1, 0)

workaround_reset_start cortex_x1, ERRATUM(1941498), ERRATA_X1_1941498
	sysreg_bit_set CORTEX_X1_CPUECTLR_EL1, BIT(8)
workaround_reset_end cortex_x1, ERRATUM(1941498)

check_erratum_ls cortex_x1, ERRATUM(1941498), CPU_REV(1, 1)

workaround_reset_start cortex_x1, ERRATUM(1951500), ERRATA_X1_1951500
	ldr x0, =0x0
	msr s3_6_c15_c8_0, x0
	ldr x0, =0x10e3900002
	msr s3_6_c15_c8_2, x0
	ldr x0, =0x10fff00083
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x2001003ff
	msr s3_6_c15_c8_1, x0
	ldr x0, =0x1
	msr s3_6_c15_c8_0, x0
	ldr x0, =0x10e3800082
	msr s3_6_c15_c8_2, x0
	ldr x0, =0x10fff00083
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x2001003ff
	msr s3_6_c15_c8_1, x0
	ldr x0, =0x2
	msr s3_6_c15_c8_0, x0
	ldr x0, =0x10e3800200
	msr s3_6_c15_c8_2, x0
	ldr x0, =0x10fff003e0
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x2001003ff
	msr s3_6_c15_c8_1, x0
workaround_reset_end cortex_x1, ERRATUM(1951500)

check_erratum_range cortex_x1, ERRATUM(1951500), CPU_REV(1, 0), CPU_REV(1, 1)

workaround_reset_start cortex_x1, ERRATUM(1952683), ERRATA_X1_1952683
	ldr x0, =0x5
	msr s3_6_c15_c8_0, x0
	ldr x0, =0xeee10a10
	msr s3_6_c15_c8_2, x0
	ldr x0, =0xffef0fff
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x0010f000
	msr s3_6_c15_c8_4, x0
	ldr x0, =0x0010f000
	msr s3_6_c15_c8_5, x0
	ldr x0, =0x40000080023ff
	msr s3_6_c15_c8_1, x0
	ldr x0, =0x6
	msr s3_6_c15_c8_0, x0
	ldr x0, =0xee640f34
	msr s3_6_c15_c8_2, x0
	ldr x0, =0xffef0fff
	msr s3_6_c15_c8_3, x0
	ldr x0, =0x40000080023ff
	msr s3_6_c15_c8_1, x0
workaround_reset_end cortex_x1, ERRATUM(1952683)

check_erratum_ls cortex_x1, ERRATUM(1952683), CPU_REV(0, 0)

workaround_reset_start cortex_x1, ERRATUM(2376745), ERRATA_X1_2376745
	sysreg_bit_set CORTEX_X1_ACTLR2_EL1, BIT(0)
workaround_reset_end cortex_x1, ERRATUM(2376745)

check_erratum_chosen cortex_x1, ERRATUM(2376745), ERRATA_X1_2376745

workaround_reset_start cortex_x1, ERRATUM(2395406), ERRATA_X1_2395406
	sysreg_bit_set CORTEX_X1_ACTLR2_EL1, BIT(40)
workaround_reset_end cortex_x1, ERRATUM(2395406)

check_erratum_chosen cortex_x1, ERRATUM(2395406), ERRATA_X1_2395406

workaround_reset_start cortex_x1, ERRATUM(2742426), ERRATA_X1_2742426
	sysreg_bit_set CORTEX_X1_ACTLR5_EL1, BIT(55)
	sysreg_bit_clear CORTEX_X1_ACTLR5_EL1, BIT(56)
workaround_reset_end cortex_x1, ERRATUM(2742426)

check_erratum_chosen cortex_x1, ERRATUM(2742426), ERRATA_X1_2742426

workaround_reset_start cortex_x1, ERRATUM(2779479), ERRATA_X1_2779479
	sysreg_bit_set CORTEX_X1_ACTLR3_EL1, BIT(47)
workaround_reset_end cortex_x1, ERRATUM(2779479)

check_erratum_chosen cortex_x1, ERRATUM(2779479), ERRATA_X1_2779479

check_erratum_chosen cortex_x1, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

workaround_reset_start cortex_x1, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-X1 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_x1
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_x1, CVE(2022, 23960)

/* Disable hardware page aggregation. Enables mitigation for `CVE-2024-5660` */
workaround_reset_start cortex_x1, CVE(2024, 5660), WORKAROUND_CVE_2024_5660
	sysreg_bit_set CORTEX_X1_CPUECTLR_EL1, BIT(46)
workaround_reset_end cortex_x1, CVE(2024, 5660)

check_erratum_ls cortex_x1, CVE(2024, 5660), CPU_REV(1, 2)

cpu_reset_func_start cortex_x1
cpu_reset_func_end cortex_x1

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_x1_core_pwr_dwn
	sysreg_bit_set CORTEX_X1_CPUPWRCTLR_EL1, CORTEX_X1_CORE_PWRDN_EN_MASK
	isb
	ret
endfunc cortex_x1_core_pwr_dwn

       /* ---------------------------------------------
	* This function provides Cortex X1 specific
	* register information for crash reporting.
	* It needs to return with x6 pointing to
	* a list of register names in ascii and
	* x8 - x15 having values of registers to be
	* reported.
	* ---------------------------------------------
	*/
.section .rodata.cortex_x1_regs, "aS"
cortex_x1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x1_cpu_reg_dump
	adr	x6, cortex_x1_regs
	mrs	x8, CORTEX_X1_CPUECTLR_EL1
	ret
endfunc cortex_x1_cpu_reg_dump

declare_cpu_ops cortex_x1, CORTEX_X1_MIDR, \
	cortex_x1_reset_func, \
	cortex_x1_core_pwr_dwn
