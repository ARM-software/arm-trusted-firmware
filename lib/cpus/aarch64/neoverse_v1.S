/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_v1.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse V1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-V1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table NEOVERSE_V1_BHB_LOOP_COUNT, neoverse_v1
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start neoverse_v1, ERRATUM(1618635), ERRATA_V1_1618635
	/* Inserts a DMB SY before and after MRS PAR_EL1 */
	ldr	x0, =0x0
	msr	NEOVERSE_V1_CPUPSELR_EL3, x0
	ldr	x0, = 0xEE070F14
	msr	NEOVERSE_V1_CPUPOR_EL3, x0
	ldr	x0, = 0xFFFF0FFF
	msr	NEOVERSE_V1_CPUPMR_EL3, x0
	ldr	x0, =0x4005027FF
	msr	NEOVERSE_V1_CPUPCR_EL3, x0

	/* Inserts a DMB SY before STREX imm offset */
	ldr	x0, =0x1
	msr	NEOVERSE_V1_CPUPSELR_EL3, x0
	ldr	x0, =0x00e8400000
	msr	NEOVERSE_V1_CPUPOR_EL3, x0
	ldr	x0, =0x00fff00000
	msr	NEOVERSE_V1_CPUPMR_EL3, x0
	ldr	x0, = 0x4001027FF
	msr	NEOVERSE_V1_CPUPCR_EL3, x0

	/* Inserts a DMB SY before STREX[BHD}/STLEX* */
	ldr	x0, =0x2
	msr	NEOVERSE_V1_CPUPSELR_EL3, x0
	ldr	x0, =0x00e8c00040
	msr	NEOVERSE_V1_CPUPOR_EL3, x0
	ldr	x0, =0x00fff00040
	msr	NEOVERSE_V1_CPUPMR_EL3, x0
	ldr	x0, = 0x4001027FF
	msr	NEOVERSE_V1_CPUPCR_EL3, x0

	/* Inserts a DMB SY after STREX imm offset */
	ldr	x0, =0x3
	msr	NEOVERSE_V1_CPUPSELR_EL3, x0
	ldr	x0, =0x00e8400000
	msr	NEOVERSE_V1_CPUPOR_EL3, x0
	ldr	x0, =0x00fff00000
	msr	NEOVERSE_V1_CPUPMR_EL3, x0
	ldr	x0, = 0x4004027FF
	msr	NEOVERSE_V1_CPUPCR_EL3, x0

	/* Inserts a DMB SY after STREX[BHD}/STLEX* */
	ldr	x0, =0x4
	msr	NEOVERSE_V1_CPUPSELR_EL3, x0
	ldr	x0, =0x00e8c00040
	msr	NEOVERSE_V1_CPUPOR_EL3, x0
	ldr	x0, =0x00fff00040
	msr	NEOVERSE_V1_CPUPMR_EL3, x0
	ldr	x0, = 0x4004027FF
	msr	NEOVERSE_V1_CPUPCR_EL3, x0

workaround_reset_end neoverse_v1, ERRATUM(1618635)

check_erratum_ls neoverse_v1, ERRATUM(1618635), CPU_REV(0, 0)

workaround_reset_start neoverse_v1, ERRATUM(1774420), ERRATA_V1_1774420
	/* Set bit 53 in CPUECTLR_EL1 */
	sysreg_bit_set NEOVERSE_V1_CPUECTLR_EL1, NEOVERSE_V1_CPUECTLR_EL1_BIT_53
workaround_reset_end neoverse_v1, ERRATUM(1774420)

check_erratum_ls neoverse_v1, ERRATUM(1774420), CPU_REV(1, 0)

workaround_reset_start neoverse_v1, ERRATUM(1791573), ERRATA_V1_1791573
	/* Set bit 2 in ACTLR2_EL1 */
	sysreg_bit_set NEOVERSE_V1_ACTLR2_EL1, NEOVERSE_V1_ACTLR2_EL1_BIT_2
workaround_reset_end neoverse_v1, ERRATUM(1791573)

check_erratum_ls neoverse_v1, ERRATUM(1791573), CPU_REV(1, 0)

workaround_reset_start neoverse_v1, ERRATUM(1852267), ERRATA_V1_1852267
	/* Set bit 28 in ACTLR2_EL1 */
	sysreg_bit_set NEOVERSE_V1_ACTLR2_EL1, NEOVERSE_V1_ACTLR2_EL1_BIT_28
workaround_reset_end neoverse_v1, ERRATUM(1852267)

check_erratum_ls neoverse_v1, ERRATUM(1852267), CPU_REV(1, 0)

workaround_reset_start neoverse_v1, ERRATUM(1925756), ERRATA_V1_1925756
	/* Set bit 8 in CPUECTLR_EL1 */
	sysreg_bit_set NEOVERSE_V1_CPUECTLR_EL1, NEOVERSE_V1_CPUECTLR_EL1_BIT_8
workaround_reset_end neoverse_v1, ERRATUM(1925756)

check_erratum_ls neoverse_v1, ERRATUM(1925756), CPU_REV(1, 1)

workaround_reset_start neoverse_v1, ERRATUM(1940577), ERRATA_V1_1940577
	mov	x0, #0
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3900002
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0

	mov	x0, #1
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3800082
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0

	mov	x0, #2
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3800200
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF003E0
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0

workaround_reset_end neoverse_v1, ERRATUM(1940577)

check_erratum_range neoverse_v1, ERRATUM(1940577), CPU_REV(1, 0), CPU_REV(1, 1)

workaround_reset_start neoverse_v1, ERRATUM(1966096), ERRATA_V1_1966096
	mov	x0, #0x3
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0xEE010F12
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0xFFFF0FFF
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x80000000003FF
	msr	S3_6_C15_C8_1, x0
workaround_reset_end neoverse_v1, ERRATUM(1966096)

check_erratum_range neoverse_v1, ERRATUM(1966096), CPU_REV(1, 0), CPU_REV(1, 1)

workaround_reset_start neoverse_v1, ERRATUM(2108267), ERRATA_V1_2108267
	mrs	x1, NEOVERSE_V1_CPUECTLR_EL1
	mov	x0, #NEOVERSE_V1_CPUECTLR_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CPUECTLR_EL1_PF_MODE_LSB, #CPUECTLR_EL1_PF_MODE_WIDTH
	msr	NEOVERSE_V1_CPUECTLR_EL1, x1
workaround_reset_end neoverse_v1, ERRATUM(2108267)

check_erratum_ls neoverse_v1, ERRATUM(2108267), CPU_REV(1, 2)

workaround_reset_start neoverse_v1, ERRATUM(2139242), ERRATA_V1_2139242
	mov	x0, #0x3
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0xEE720F14
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0xFFFF0FDF
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x40000005003FF
	msr	S3_6_C15_C8_1, x0
workaround_reset_end neoverse_v1, ERRATUM(2139242)

check_erratum_ls neoverse_v1, ERRATUM(2139242), CPU_REV(1, 1)

workaround_reset_start neoverse_v1, ERRATUM(2216392), ERRATA_V1_2216392
	ldr	x0, =0x5
	msr	S3_6_c15_c8_0, x0 /* CPUPSELR_EL3 */
	ldr	x0, =0x10F600E000
	msr	S3_6_c15_c8_2, x0 /* CPUPOR_EL3 */
	ldr	x0, =0x10FF80E000
	msr	S3_6_c15_c8_3, x0 /* CPUPMR_EL3 */
	ldr	x0, =0x80000000003FF
	msr	S3_6_c15_c8_1, x0 /* CPUPCR_EL3 */
workaround_reset_end neoverse_v1, ERRATUM(2216392)

check_erratum_range neoverse_v1, ERRATUM(2216392), CPU_REV(1, 0), CPU_REV(1, 1)

workaround_reset_start neoverse_v1, ERRATUM(2294912), ERRATA_V1_2294912
	/* Set bit 0 in ACTLR2_EL1 */
	sysreg_bit_set NEOVERSE_V1_ACTLR2_EL1, NEOVERSE_V1_ACTLR2_EL1_BIT_0
workaround_reset_end neoverse_v1, ERRATUM(2294912)

check_erratum_ls neoverse_v1, ERRATUM(2294912), CPU_REV(1, 2)

workaround_reset_start neoverse_v1, ERRATUM(2372203), ERRATA_V1_2372203
	/* Set bit 40 in ACTLR2_EL1 */
	sysreg_bit_set NEOVERSE_V1_ACTLR2_EL1, NEOVERSE_V1_ACTLR2_EL1_BIT_40
workaround_reset_end neoverse_v1, ERRATUM(2372203)

check_erratum_ls neoverse_v1, ERRATUM(2372203), CPU_REV(1, 1)

workaround_runtime_start neoverse_v1, ERRATUM(2743093), ERRATA_V1_2743093
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end neoverse_v1, ERRATUM(2743093)

check_erratum_ls neoverse_v1, ERRATUM(2743093), CPU_REV(1, 2)

workaround_reset_start neoverse_v1, ERRATUM(2743233), ERRATA_V1_2743233
	sysreg_bit_clear NEOVERSE_V1_ACTLR5_EL1, NEOVERSE_V1_ACTLR5_EL1_BIT_56
	sysreg_bit_set NEOVERSE_V1_ACTLR5_EL1, NEOVERSE_V1_ACTLR5_EL1_BIT_55
workaround_reset_end neoverse_v1, ERRATUM(2743233)

check_erratum_ls neoverse_v1, ERRATUM(2743233), CPU_REV(1, 2)

workaround_reset_start neoverse_v1, ERRATUM(2779461), ERRATA_V1_2779461
	sysreg_bit_set NEOVERSE_V1_ACTLR3_EL1, NEOVERSE_V1_ACTLR3_EL1_BIT_47
workaround_reset_end neoverse_v1, ERRATUM(2779461)

check_erratum_ls neoverse_v1, ERRATUM(2779461), CPU_REV(1, 2)


workaround_reset_start neoverse_v1, CVE(2022,23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Neoverse-V1 generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_neoverse_v1
#endif /* IMAGE_BL31 */
workaround_reset_end neoverse_v1, CVE(2022,23960)

check_erratum_chosen neoverse_v1, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_v1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_V1_CPUPWRCTLR_EL1, NEOVERSE_V1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	apply_erratum neoverse_v1, ERRATUM(2743093), ERRATA_V1_2743093

	isb
	ret
endfunc neoverse_v1_core_pwr_dwn

errata_report_shim neoverse_v1

cpu_reset_func_start neoverse_v1
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end neoverse_v1

	/* ---------------------------------------------
	 * This function provides Neoverse-V1 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_v1_regs, "aS"
neoverse_v1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_v1_cpu_reg_dump
	adr	x6, neoverse_v1_regs
	mrs	x8, NEOVERSE_V1_CPUECTLR_EL1
	ret
endfunc neoverse_v1_cpu_reg_dump

declare_cpu_ops neoverse_v1, NEOVERSE_V1_MIDR, \
	neoverse_v1_reset_func, \
	neoverse_v1_core_pwr_dwn
