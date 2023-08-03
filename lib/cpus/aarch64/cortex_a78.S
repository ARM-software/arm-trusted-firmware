/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a78.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_a78 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

.globl cortex_a78_reset_func
.globl cortex_a78_core_pwr_dwn

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A78_BHB_LOOP_COUNT, cortex_a78
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_a78, ERRATUM(1688305), ERRATA_A78_1688305
	sysreg_bit_set CORTEX_A78_ACTLR2_EL1, CORTEX_A78_ACTLR2_EL1_BIT_1
workaround_reset_end cortex_a78, ERRATUM(1688305)

check_erratum_ls cortex_a78, ERRATUM(1688305), CPU_REV(1, 0)

workaround_reset_start cortex_a78, ERRATUM(1821534), ERRATA_A78_1821534
	sysreg_bit_set CORTEX_A78_ACTLR2_EL1, CORTEX_A78_ACTLR2_EL1_BIT_2
workaround_reset_end cortex_a78, ERRATUM(1821534)

check_erratum_ls cortex_a78, ERRATUM(1821534), CPU_REV(1, 0)

workaround_reset_start cortex_a78, ERRATUM(1941498), ERRATA_A78_1941498
	sysreg_bit_set CORTEX_A78_CPUECTLR_EL1, CORTEX_A78_CPUECTLR_EL1_BIT_8
workaround_reset_end cortex_a78, ERRATUM(1941498)

check_erratum_ls cortex_a78, ERRATUM(1941498), CPU_REV(1, 1)

workaround_reset_start cortex_a78, ERRATUM(1951500), ERRATA_A78_1951500
	msr	S3_6_c15_c8_0, xzr
	ldr	x0, =0x10E3900002
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0

	mov	x0, #1
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10E3800082
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0

	mov	x0, #2
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10E3800200
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF003E0
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0
workaround_reset_end cortex_a78, ERRATUM(1951500)

check_erratum_range cortex_a78, ERRATUM(1951500), CPU_REV(1, 0), CPU_REV(1, 1)

workaround_reset_start cortex_a78, ERRATUM(1952683), ERRATA_A78_1952683
	ldr	x0,=0x5
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xEEE10A10
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFEF0FFF
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x0010F000
	msr	S3_6_c15_c8_4,x0
	ldr	x0,=0x0010F000
	msr	S3_6_c15_c8_5,x0
	ldr	x0,=0x40000080023ff
	msr	S3_6_c15_c8_1,x0
	ldr	x0,=0x6
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xEE640F34
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFEF0FFF
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x40000080023ff
	msr	S3_6_c15_c8_1,x0
workaround_reset_end cortex_a78, ERRATUM(1952683)

check_erratum_ls cortex_a78, ERRATUM(1952683), CPU_REV(0, 0)

workaround_reset_start cortex_a78, ERRATUM(2132060), ERRATA_A78_2132060
	/* Apply the workaround. */
	mrs	x1, CORTEX_A78_CPUECTLR_EL1
	mov	x0, #CORTEX_A78_CPUECTLR_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CPUECTLR_EL1_PF_MODE_LSB, #CPUECTLR_EL1_PF_MODE_WIDTH
	msr	CORTEX_A78_CPUECTLR_EL1, x1
workaround_reset_end cortex_a78, ERRATUM(2132060)

check_erratum_ls cortex_a78, ERRATUM(2132060), CPU_REV(1, 2)

workaround_reset_start cortex_a78, ERRATUM(2242635), ERRATA_A78_2242635
	ldr	x0, =0x5
	msr	S3_6_c15_c8_0, x0 /* CPUPSELR_EL3 */
	ldr	x0, =0x10F600E000
	msr	S3_6_c15_c8_2, x0 /* CPUPOR_EL3 */
	ldr	x0, =0x10FF80E000
	msr	S3_6_c15_c8_3, x0 /* CPUPMR_EL3 */
	ldr	x0, =0x80000000003FF
	msr	S3_6_c15_c8_1, x0 /* CPUPCR_EL3 */
workaround_reset_end cortex_a78, ERRATUM(2242635)

check_erratum_range cortex_a78, ERRATUM(2242635), CPU_REV(1, 0), CPU_REV(1, 2)

workaround_reset_start cortex_a78, ERRATUM(2376745), ERRATA_A78_2376745
	sysreg_bit_set CORTEX_A78_ACTLR2_EL1, BIT(0)
workaround_reset_end cortex_a78, ERRATUM(2376745)

check_erratum_ls cortex_a78, ERRATUM(2376745), CPU_REV(1, 2)

workaround_reset_start cortex_a78, ERRATUM(2395406), ERRATA_A78_2395406
	sysreg_bit_set CORTEX_A78_ACTLR2_EL1, BIT(40)
workaround_reset_end cortex_a78, ERRATUM(2395406)

check_erratum_ls cortex_a78, ERRATUM(2395406), CPU_REV(1, 2)

workaround_reset_start cortex_a78, ERRATUM(2742426), ERRATA_A78_2742426
	/* Apply the workaround */
	mrs	x1, CORTEX_A78_ACTLR5_EL1
	bic	x1, x1, #BIT(56)
	orr	x1, x1, #BIT(55)
	msr	CORTEX_A78_ACTLR5_EL1, x1
workaround_reset_end cortex_a78, ERRATUM(2742426)

check_erratum_ls cortex_a78, ERRATUM(2742426), CPU_REV(1, 2)

workaround_runtime_start cortex_a78, ERRATUM(2772019), ERRATA_A78_2772019
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end cortex_a78, ERRATUM(2772019)

check_erratum_ls cortex_a78, ERRATUM(2772019), CPU_REV(1, 2)

workaround_reset_start cortex_a78, ERRATUM(2779479), ERRATA_A78_2779479
	sysreg_bit_set CORTEX_A78_ACTLR3_EL1, BIT(47)
workaround_reset_end cortex_a78, ERRATUM(2779479)

check_erratum_ls cortex_a78, ERRATUM(2779479), CPU_REV(1, 2)

workaround_reset_start cortex_a78, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-X1 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_a78
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a78, CVE(2022, 23960)

check_erratum_chosen cortex_a78, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

cpu_reset_func_start cortex_a78
#if ENABLE_FEAT_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	sysreg_bit_clear actlr_el3, CORTEX_A78_ACTLR_TAM_BIT

	/* Make sure accesses from non-secure EL0/EL1 are not trapped to EL2 */
	sysreg_bit_clear actlr_el2, CORTEX_A78_ACTLR_TAM_BIT

	/* Enable group0 counters */
	mov	x0, #CORTEX_A78_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET0_EL0, x0

	/* Enable group1 counters */
	mov	x0, #CORTEX_A78_AMU_GROUP1_MASK
	msr	CPUAMCNTENSET1_EL0, x0
#endif
cpu_reset_func_end cortex_a78

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a78_core_pwr_dwn
	sysreg_bit_set CORTEX_A78_CPUPWRCTLR_EL1, CORTEX_A78_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT

	apply_erratum cortex_a78, ERRATUM(2772019), ERRATA_A78_2772019

	isb
	ret
endfunc cortex_a78_core_pwr_dwn

errata_report_shim cortex_a78

	/* ---------------------------------------------
	 * This function provides cortex_a78 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a78_regs, "aS"
cortex_a78_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a78_cpu_reg_dump
	adr	x6, cortex_a78_regs
	mrs	x8, CORTEX_A78_CPUECTLR_EL1
	ret
endfunc cortex_a78_cpu_reg_dump

declare_cpu_ops cortex_a78, CORTEX_A78_MIDR, \
	cortex_a78_reset_func, \
	cortex_a78_core_pwr_dwn
