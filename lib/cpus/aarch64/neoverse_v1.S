/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
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

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #1618635.
	 * This applies to revision r0p0 and is fixed in
	 * r1p0.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0, x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1618635_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_1618635
	cbz	x0, 1f

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

	/* Synchronize to enable patches */
	isb
1:
	ret x17
endfunc errata_neoverse_v1_1618635_wa

func check_errata_1618635
	/* Applies to revision r0p0. */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_1618635

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #1774420.
	 * This applies to revisions r0p0 and r1p0, fixed in r1p1.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1774420_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_1774420
	cbz	x0, 1f

	/* Set bit 53 in CPUECTLR_EL1 */
	mrs     x1, NEOVERSE_V1_CPUECTLR_EL1
	orr	x1, x1, #NEOVERSE_V1_CPUECTLR_EL1_BIT_53
	msr     NEOVERSE_V1_CPUECTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_neoverse_v1_1774420_wa

func check_errata_1774420
	/* Applies to r0p0 and r1p0. */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1774420

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #1791573.
	 * This applies to revisions r0p0 and r1p0, fixed in r1p1.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1791573_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_1791573
	cbz	x0, 1f

	/* Set bit 2 in ACTLR2_EL1 */
	mrs	x1, NEOVERSE_V1_ACTLR2_EL1
	orr	x1, x1, #NEOVERSE_V1_ACTLR2_EL1_BIT_2
	msr	NEOVERSE_V1_ACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_neoverse_v1_1791573_wa

func check_errata_1791573
	/* Applies to r0p0 and r1p0. */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1791573

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #1852267.
	 * This applies to revisions r0p0 and r1p0, fixed in r1p1.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1852267_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_1852267
	cbz	x0, 1f

	/* Set bit 28 in ACTLR2_EL1 */
	mrs	x1, NEOVERSE_V1_ACTLR2_EL1
	orr	x1, x1, #NEOVERSE_V1_ACTLR2_EL1_BIT_28
	msr	NEOVERSE_V1_ACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_neoverse_v1_1852267_wa

func check_errata_1852267
	/* Applies to r0p0 and r1p0. */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1852267

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #1925756.
	 * This applies to revisions <= r1p1.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1925756_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_1925756
	cbz	x0, 1f

	/* Set bit 8 in CPUECTLR_EL1 */
	mrs	x1, NEOVERSE_V1_CPUECTLR_EL1
	orr	x1, x1, #NEOVERSE_V1_CPUECTLR_EL1_BIT_8
	msr	NEOVERSE_V1_CPUECTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_neoverse_v1_1925756_wa

func check_errata_1925756
	/* Applies to <= r1p1. */
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_1925756

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Erratum #1940577
	 * This applies to revisions r1p0 - r1p1 and is open.
	 * It also exists in r0p0 but there is no fix in that
	 * revision.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1940577_wa
	/* Compare x0 against revisions r1p0 - r1p1 */
	mov	x17, x30
	bl	check_errata_1940577
	cbz	x0, 1f

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

	isb
1:
	ret	x17
endfunc errata_neoverse_v1_1940577_wa

func check_errata_1940577
	/* Applies to revisions r1p0 - r1p1. */
	mov	x1, #0x10
	mov	x2, #0x11
	b	cpu_rev_var_range
endfunc check_errata_1940577

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #1966096
	 * This applies to revisions r1p0 - r1p1 and is open.
	 * It also exists in r0p0 but there is no workaround
	 * for that revision.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1966096_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_1966096
	cbz	x0, 1f

	/* Apply the workaround. */
	mov	x0, #0x3
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0xEE010F12
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0xFFFF0FFF
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x80000000003FF
	msr	S3_6_C15_C8_1, x0
	isb

1:
	ret	x17
endfunc errata_neoverse_v1_1966096_wa

func check_errata_1966096
	mov	x1, #0x10
	mov	x2, #0x11
	b	cpu_rev_var_range
endfunc check_errata_1966096

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #2139242.
	 * This applies to revisions r0p0, r1p0, and r1p1, it
	 * is still open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_2139242_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2139242
	cbz	x0, 1f

	/* Apply the workaround. */
	mov	x0, #0x3
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0xEE720F14
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0xFFFF0FDF
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x40000005003FF
	msr	S3_6_C15_C8_1, x0
	isb

1:
	ret	x17
endfunc errata_neoverse_v1_2139242_wa

func check_errata_2139242
	/* Applies to r0p0, r1p0, r1p1 */
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_2139242

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #2108267.
	 * This applies to revisions r0p0, r1p0, and r1p1, it
	 * is still open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x1, x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_2108267_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2108267
	cbz	x0, 1f

	/* Apply the workaround. */
	mrs	x1, NEOVERSE_V1_CPUECTLR_EL1
	mov	x0, #NEOVERSE_V1_CPUECTLR_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CPUECTLR_EL1_PF_MODE_LSB, #CPUECTLR_EL1_PF_MODE_WIDTH
	msr	NEOVERSE_V1_CPUECTLR_EL1, x1
1:
	ret	x17
endfunc errata_neoverse_v1_2108267_wa

func check_errata_2108267
	/* Applies to r0p0, r1p0, r1p1 */
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_2108267

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #2216392.
	 * This applies to revisions r1p0 and r1p1 and is
	 * still open.
	 * This issue is also present in r0p0 but there is no
	 * workaround in that revision.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_2216392_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2216392
	cbz	x0, 1f

	ldr	x0, =0x5
	msr	S3_6_c15_c8_0, x0 /* CPUPSELR_EL3 */
	ldr	x0, =0x10F600E000
	msr	S3_6_c15_c8_2, x0 /* CPUPOR_EL3 */
	ldr	x0, =0x10FF80E000
	msr	S3_6_c15_c8_3, x0 /* CPUPMR_EL3 */
	ldr	x0, =0x80000000003FF
	msr	S3_6_c15_c8_1, x0 /* CPUPCR_EL3 */

	isb
1:
	ret	x17
endfunc errata_neoverse_v1_2216392_wa

func check_errata_2216392
	/* Applies to revisions r1p0 and r1p1. */
	mov	x1, #CPU_REV(1, 0)
	mov	x2, #CPU_REV(1, 1)
	b	cpu_rev_var_range
endfunc check_errata_2216392

	/* -----------------------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #2294912.
	 * This applies to revisions r0p0, r1p0, and r1p1 and is still open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * -----------------------------------------------------------------
	 */
func errata_neoverse_v1_2294912_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2294912
	cbz	x0, 1f

	/* Set bit 0 in ACTLR2_EL1 */
	mrs     x1, NEOVERSE_V1_ACTLR2_EL1
	orr	x1, x1, #NEOVERSE_V1_ACTLR2_EL1_BIT_0
	msr     NEOVERSE_V1_ACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_neoverse_v1_2294912_wa

func check_errata_2294912
	/* Applies to r0p0, r1p0, and r1p1 right now */
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_2294912

	/* ---------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #2372203.
	 * This applies to revisions <= r1p1 and is still open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ----------------------------------------------------
	 */
func errata_neoverse_v1_2372203_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2372203
	cbz	x0, 1f

	/* Set bit 40 in ACTLR2_EL1 */
	mrs	x1, NEOVERSE_V1_ACTLR2_EL1
	orr	x1, x1, #NEOVERSE_V1_ACTLR2_EL1_BIT_40
	msr	NEOVERSE_V1_ACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_neoverse_v1_2372203_wa

func check_errata_2372203
	/* Applies to <= r1p1. */
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_2372203

	/* ----------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #2743093.
	 * This applies to revisions <= r1p2 and is still open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ----------------------------------------------------
	 */
func errata_neoverse_v1_2743093_wa
	mov	x17, x30
	bl	check_errata_2743093
	cbz	x0, 1f

	/* dsb before isb of power down sequence */
	dsb	sy
1:
	ret	x17
endfunc errata_neoverse_v1_2743093_wa

func check_errata_2743093
	/* Applies to all revisions <= r1p2 */
	mov	x1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_2743093

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_v1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, NEOVERSE_V1_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_V1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	NEOVERSE_V1_CPUPWRCTLR_EL1, x0
#if ERRATA_V1_2743093
	mov	x15, x30
	bl	cpu_get_rev_var
	bl	errata_neoverse_v1_2743093_wa
	mov	x30, x15
#endif /* ERRATA_V1_2743093 */
	isb
	ret
endfunc neoverse_v1_core_pwr_dwn

	/*
	 * Errata printing function for Neoverse V1. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func neoverse_v1_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_V1_1618635, neoverse_v1, 1618635
	report_errata ERRATA_V1_1774420, neoverse_v1, 1774420
	report_errata ERRATA_V1_1791573, neoverse_v1, 1791573
	report_errata ERRATA_V1_1852267, neoverse_v1, 1852267
	report_errata ERRATA_V1_1925756, neoverse_v1, 1925756
	report_errata ERRATA_V1_1940577, neoverse_v1, 1940577
	report_errata ERRATA_V1_1966096, neoverse_v1, 1966096
	report_errata ERRATA_V1_2108267, neoverse_v1, 2108267
	report_errata ERRATA_V1_2139242, neoverse_v1, 2139242
	report_errata ERRATA_V1_2216392, neoverse_v1, 2216392
	report_errata ERRATA_V1_2294912, neoverse_v1, 2294912
	report_errata ERRATA_V1_2372203, neoverse_v1, 2372203
	report_errata ERRATA_V1_2743093, neoverse_v1, 2743093
	report_errata WORKAROUND_CVE_2022_23960, neoverse_v1, cve_2022_23960

	ldp	x8, x30, [sp], #16
	ret
endfunc neoverse_v1_errata_report
#endif

func neoverse_v1_reset_func
	mov	x19, x30

	/* Disable speculative loads */
	msr	SSBS, xzr
	isb

	/* Get the CPU revision and stash it in x18. */
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_V1_1618635
	mov x0, x18
	bl errata_neoverse_v1_1618635_wa
#endif

#if ERRATA_V1_1774420
	mov	x0, x18
	bl	errata_neoverse_v1_1774420_wa
#endif

#if ERRATA_V1_1791573
	mov	x0, x18
	bl	errata_neoverse_v1_1791573_wa
#endif

#if ERRATA_V1_1852267
	mov	x0, x18
	bl	errata_neoverse_v1_1852267_wa
#endif

#if ERRATA_V1_1925756
	mov	x0, x18
	bl	errata_neoverse_v1_1925756_wa
#endif

#if ERRATA_V1_1940577
	mov	x0, x18
	bl	errata_neoverse_v1_1940577_wa
#endif

#if ERRATA_V1_1966096
	mov	x0, x18
	bl	errata_neoverse_v1_1966096_wa
#endif

#if ERRATA_V1_2139242
	mov	x0, x18
	bl	errata_neoverse_v1_2139242_wa
#endif

#if ERRATA_V1_2108267
	mov	x0, x18
	bl	errata_neoverse_v1_2108267_wa
#endif

#if ERRATA_V1_2216392
	mov	x0, x18
	bl	errata_neoverse_v1_2216392_wa
#endif

#if ERRATA_V1_2294912
	mov	x0, x18
	bl	errata_neoverse_v1_2294912_wa
#endif

#if ERRATA_V1_2372203
	mov	x0, x18
	bl	errata_neoverse_v1_2372203_wa
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Neoverse-V1 generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_neoverse_v1
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */

	isb
	ret	x19
endfunc neoverse_v1_reset_func

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
