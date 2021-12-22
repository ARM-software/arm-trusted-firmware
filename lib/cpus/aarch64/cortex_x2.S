/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_x2.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex X2 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex X2 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_X2_BHB_LOOP_COUNT, cortex_x2
#endif /* WORKAROUND_CVE_2022_23960 */

	/* --------------------------------------------------
	 * Errata Workaround for Cortex X2 Errata #2002765.
	 * This applies to revisions r0p0, r1p0, and r2p0 and
	 * is open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0, x1, x17
	 * --------------------------------------------------
	 */
func errata_cortex_x2_2002765_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2002765
	cbz	x0, 1f

	ldr	x0, =0x6
	msr	S3_6_C15_C8_0, x0 /* CPUPSELR_EL3 */
	ldr	x0, =0xF3A08002
	msr	S3_6_C15_C8_2, x0 /* CPUPOR_EL3 */
	ldr	x0, =0xFFF0F7FE
	msr	S3_6_C15_C8_3, x0 /* CPUPMR_EL3 */
	ldr	x0, =0x40000001003ff
	msr	S3_6_C15_C8_1, x0 /* CPUPCR_EL3 */
	isb

1:
	ret	x17
endfunc errata_cortex_x2_2002765_wa

func check_errata_2002765
	/* Applies to r0p0 - r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2002765

	/* --------------------------------------------------
	 * Errata Workaround for Cortex X2 Errata #2058056.
	 * This applies to revisions r0p0, r1p0, and r2p0 and
	 * is open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0, x1, x17
	 * --------------------------------------------------
	 */
func errata_cortex_x2_2058056_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2058056
	cbz	x0, 1f

	mrs	x1, CORTEX_X2_CPUECTLR2_EL1
	mov	x0, #CORTEX_X2_CPUECTLR2_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CORTEX_X2_CPUECTLR2_EL1_PF_MODE_SHIFT, #CORTEX_X2_CPUECTLR2_EL1_PF_MODE_WIDTH
	msr	CORTEX_X2_CPUECTLR2_EL1, x1

1:
	ret	x17
endfunc errata_cortex_x2_2058056_wa

func check_errata_2058056
	/* Applies to r0p0 - r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2058056

	/* --------------------------------------------------
	 * Errata Workaround for Cortex X2 Errata #2083908.
	 * This applies to revision r2p0 and is open.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x2, x17
	 * --------------------------------------------------
	 */
func errata_cortex_x2_2083908_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2083908
	cbz	x0, 1f

	/* Apply the workaround by setting bit 13 in CPUACTLR5_EL1. */
	mrs	x1, CORTEX_X2_CPUACTLR5_EL1
	orr	x1, x1, #BIT(13)
	msr	CORTEX_X2_CPUACTLR5_EL1, x1

1:
	ret	x17
endfunc errata_cortex_x2_2083908_wa

func check_errata_2083908
	/* Applies to r2p0 */
	mov	x1, #0x20
	mov	x2, #0x20
	b	cpu_rev_var_range
endfunc check_errata_2083908

	/* --------------------------------------------------
	 * Errata Workaround for Cortex-X2 Errata 2017096.
	 * This applies only to revisions r0p0, r1p0 and r2p0
	 * and is fixed in r2p1.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0, x1, x17
	 * --------------------------------------------------
	 */
func errata_x2_2017096_wa
	/* Compare x0 against revision r0p0 to r2p0 */
	mov     x17, x30
	bl      check_errata_2017096
	cbz     x0, 1f
	mrs     x1, CORTEX_X2_CPUECTLR_EL1
	orr     x1, x1, CORTEX_X2_CPUECTLR_EL1_PFSTIDIS_BIT
	msr     CORTEX_X2_CPUECTLR_EL1, x1

1:
	ret     x17
endfunc errata_x2_2017096_wa

func check_errata_2017096
	/* Applies to r0p0, r1p0, r2p0 */
	mov     x1, #0x20
	b       cpu_rev_var_ls
endfunc check_errata_2017096

	/* --------------------------------------------------
	 * Errata Workaround for Cortex-X2 Errata 2081180.
	 * This applies to revision r0p0, r1p0 and r2p0
	 * and is fixed in r2p1.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0, x1, x17
	 * --------------------------------------------------
	 */
func errata_x2_2081180_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2081180
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	ldr	x0, =0x3
	msr	CORTEX_X2_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0xF3A08002
	msr	CORTEX_X2_IMP_CPUPOR_EL3, x0
	ldr	x0, =0xFFF0F7FE
	msr	CORTEX_X2_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x10002001003FF
	msr	CORTEX_X2_IMP_CPUPCR_EL3, x0
	ldr	x0, =0x4
	msr	CORTEX_X2_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0xBF200000
	msr	CORTEX_X2_IMP_CPUPOR_EL3, x0
	ldr	x0, =0xFFEF0000
	msr	CORTEX_X2_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x10002001003F3
	msr	CORTEX_X2_IMP_CPUPCR_EL3, x0
	isb
1:
	ret	x17
endfunc errata_x2_2081180_wa

func check_errata_2081180
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2081180

	/* --------------------------------------------------
	 * Errata Workaround for Cortex X2 Errata 2216384.
	 * This applies to revisions r0p0, r1p0, and r2p0
	 * and is fixed in r2p1.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0, x1, x17
	 * --------------------------------------------------
	 */
func errata_x2_2216384_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2216384
	cbz	x0, 1f

	mrs	x1, CORTEX_X2_CPUACTLR5_EL1
	orr	x1, x1, CORTEX_X2_CPUACTLR5_EL1_BIT_17
	msr	CORTEX_X2_CPUACTLR5_EL1, x1

	/* Apply instruction patching sequence */
	ldr	x0, =0x5
	msr	CORTEX_X2_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0x10F600E000
	msr	CORTEX_X2_IMP_CPUPOR_EL3, x0
	ldr	x0, =0x10FF80E000
	msr	CORTEX_X2_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x80000000003FF
	msr	CORTEX_X2_IMP_CPUPCR_EL3, x0
	isb

1:
	ret	x17
endfunc errata_x2_2216384_wa

func check_errata_2216384
	/* Applies to r0p0 - r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2216384

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

	/* ---------------------------------------------------------
	 * Errata Workaround for Cortex-X2 Errata 2147715.
	 * This applies only to revisions r2p0 and is fixed in r2p1.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0, x1, x17
	 * ---------------------------------------------------------
	 */
func errata_x2_2147715_wa
	/* Compare x0 against revision r2p0 */
	mov     x17, x30
	bl      check_errata_2147715
	cbz     x0, 1f

	/* Apply the workaround by setting bit 22 in CPUACTLR_EL1. */
	mrs     x1, CORTEX_X2_CPUACTLR_EL1
	orr     x1, x1, CORTEX_X2_CPUACTLR_EL1_BIT_22
	msr     CORTEX_X2_CPUACTLR_EL1, x1

1:
	ret     x17
endfunc errata_x2_2147715_wa

func check_errata_2147715
	/* Applies to r2p0 */
	mov	x1, #0x20
	mov	x2, #0x20
	b	cpu_rev_var_range
endfunc check_errata_2147715

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_x2_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_X2_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_X2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_X2_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_x2_core_pwr_dwn

	/*
	 * Errata printing function for Cortex X2. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func cortex_x2_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_X2_2002765, cortex_x2, 2002765
	report_errata ERRATA_X2_2017096, cortex_x2, 2017096
	report_errata ERRATA_X2_2058056, cortex_x2, 2058056
	report_errata ERRATA_X2_2081180, cortex_x2, 2081180
	report_errata ERRATA_X2_2083908, cortex_x2, 2083908
	report_errata ERRATA_X2_2147715, cortex_x2, 2147715
	report_errata ERRATA_X2_2216384, cortex_x2, 2216384
	report_errata WORKAROUND_CVE_2022_23960, cortex_x2, cve_2022_23960
	report_errata ERRATA_DSU_2313941, cortex_x2, dsu_2313941

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_x2_errata_report
#endif

func cortex_x2_reset_func
	mov	x19, x30

	/* Disable speculative loads */
	msr	SSBS, xzr

	/* Get the CPU revision and stash it in x18. */
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_DSU_2313941
	bl	errata_dsu_2313941_wa
#endif

#if ERRATA_X2_2002765
	mov	x0, x18
	bl	errata_cortex_x2_2002765_wa
#endif

#if ERRATA_X2_2058056
	mov	x0, x18
	bl	errata_cortex_x2_2058056_wa
#endif

#if ERRATA_X2_2083908
	mov	x0, x18
	bl	errata_cortex_x2_2083908_wa
#endif

#if ERRATA_X2_2017096
	mov	x0, x18
	bl	errata_x2_2017096_wa
#endif

#if ERRATA_X2_2081180
	mov	x0, x18
	bl	errata_x2_2081180_wa
#endif

#if ERRATA_X2_2216384
	mov	x0, x18
	bl	errata_x2_2216384_wa
#endif

#if ERRATA_X2_2147715
	mov	x0, x18
	bl	errata_x2_2147715_wa
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-X2 generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
         */
	adr	x0, wa_cve_vbar_cortex_x2
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */

	isb
	ret	x19
endfunc cortex_x2_reset_func

	/* ---------------------------------------------
	 * This function provides Cortex X2 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_x2_regs, "aS"
cortex_x2_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x2_cpu_reg_dump
	adr	x6, cortex_x2_regs
	mrs	x8, CORTEX_X2_CPUECTLR_EL1
	ret
endfunc cortex_x2_cpu_reg_dump

declare_cpu_ops cortex_x2, CORTEX_X2_MIDR, \
	cortex_x2_reset_func, \
	cortex_x2_core_pwr_dwn
