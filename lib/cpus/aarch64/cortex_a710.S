/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a710.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex A710 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex A710 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A710_BHB_LOOP_COUNT, cortex_a710
#endif /* WORKAROUND_CVE_2022_23960 */

/* --------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 1987031.
 * This applies to revision r0p0, r1p0 and r2p0 of Cortex-A710. It is still
 * open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a710_1987031_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_1987031
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	ldr x0,=0x6
	msr S3_6_c15_c8_0,x0
	ldr x0,=0xF3A08002
	msr S3_6_c15_c8_2,x0
	ldr x0,=0xFFF0F7FE
	msr S3_6_c15_c8_3,x0
	ldr x0,=0x40000001003ff
	msr S3_6_c15_c8_1,x0
	ldr x0,=0x7
	msr S3_6_c15_c8_0,x0
	ldr x0,=0xBF200000
	msr S3_6_c15_c8_2,x0
	ldr x0,=0xFFEF0000
	msr S3_6_c15_c8_3,x0
	ldr x0,=0x40000001003f3
	msr S3_6_c15_c8_1,x0
	isb
1:
	ret	x17
endfunc errata_a710_1987031_wa

func check_errata_1987031
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1987031

/* --------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2081180.
 * This applies to revision r0p0, r1p0 and r2p0 of Cortex-A710.
 * It is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a710_2081180_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2081180
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	ldr	x0,=0x3
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xF3A08002
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFF0F7FE
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x10002001003FF
	msr	S3_6_c15_c8_1,x0
	ldr	x0,=0x4
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xBF200000
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFEF0000
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x10002001003F3
	msr	S3_6_c15_c8_1,x0
	isb
1:
	ret	x17
endfunc errata_a710_2081180_wa

func check_errata_2081180
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2081180

/* ---------------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2055002.
 * This applies to revision r1p0, r2p0 of Cortex-A710 and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------------
 */
func errata_a710_2055002_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2055002
	cbz	x0, 1f
	mrs	x1, CORTEX_A710_CPUACTLR_EL1
	orr	x1, x1, CORTEX_A710_CPUACTLR_EL1_BIT_46
	msr	CORTEX_A710_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a710_2055002_wa

func check_errata_2055002
	/* Applies to r1p0, r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2055002

/* -------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2017096.
 * This applies to revisions r0p0, r1p0 and r2p0 of Cortex-A710.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * -------------------------------------------------------------
 */
func errata_a710_2017096_wa
	/* Compare x0 against revision r0p0 to r2p0 */
	mov     x17, x30
	bl      check_errata_2017096
	cbz     x0, 1f
	mrs     x1, CORTEX_A710_CPUECTLR_EL1
	orr     x1, x1, CORTEX_A710_CPUECTLR_EL1_PFSTIDIS_BIT
	msr     CORTEX_A710_CPUECTLR_EL1, x1

1:
	ret     x17
endfunc errata_a710_2017096_wa

func check_errata_2017096
	/* Applies to r0p0, r1p0, r2p0 */
	mov     x1, #0x20
	b       cpu_rev_var_ls
endfunc check_errata_2017096


/* ---------------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2083908.
 * This applies to revision r2p0 of Cortex-A710 and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------------
 */
func errata_a710_2083908_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2083908
	cbz	x0, 1f
	mrs	x1, CORTEX_A710_CPUACTLR5_EL1
	orr	x1, x1, CORTEX_A710_CPUACTLR5_EL1_BIT_13
	msr	CORTEX_A710_CPUACTLR5_EL1, x1
1:
	ret	x17
endfunc errata_a710_2083908_wa

func check_errata_2083908
	/* Applies to r2p0 */
	mov	x1, #CPU_REV(2, 0)
	mov	x2, #CPU_REV(2, 0)
	b	cpu_rev_var_range
endfunc check_errata_2083908

/* ---------------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2058056.
 * This applies to revisions r0p0, r1p0 and r2p0 of Cortex-A710 and is still
 * open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------------
 */
func errata_a710_2058056_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2058056
	cbz	x0, 1f
	mrs	x1, CORTEX_A710_CPUECTLR2_EL1
	mov	x0, #CORTEX_A710_CPUECTLR2_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CPUECTLR2_EL1_PF_MODE_LSB, #CPUECTLR2_EL1_PF_MODE_WIDTH
	msr	CORTEX_A710_CPUECTLR2_EL1, x1
1:
	ret	x17
endfunc errata_a710_2058056_wa

func check_errata_2058056
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2058056

/* --------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2267065.
 * This applies to revisions r0p0, r1p0 and r2p0.
 * It is fixed in r2p1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x1, x17
 * --------------------------------------------------
 */
func errata_a710_2267065_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2267065
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	mrs	x1, CORTEX_A710_CPUACTLR_EL1
	orr	x1, x1, CORTEX_A710_CPUACTLR_EL1_BIT_22
	msr	CORTEX_A710_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a710_2267065_wa

func check_errata_2267065
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2267065

/* ---------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2136059.
 * This applies to revision r0p0, r1p0 and r2p0.
 * It is fixed in r2p1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------
 */
func errata_a710_2136059_wa
	/* Compare x0 against revision r2p0 */
	mov     x17, x30
	bl      check_errata_2136059
	cbz     x0, 1f

	/* Apply the workaround */
	mrs     x1, CORTEX_A710_CPUACTLR5_EL1
	orr     x1, x1, CORTEX_A710_CPUACTLR5_EL1_BIT_44
	msr     CORTEX_A710_CPUACTLR5_EL1, x1

1:
	ret     x17
endfunc errata_a710_2136059_wa

func check_errata_2136059
	/* Applies to r0p0, r1p0 and r2p0 */
	mov     x1, #0x20
	b       cpu_rev_var_ls
endfunc check_errata_2136059

/* ----------------------------------------------------------------
 * Errata workaround for Cortex-A710 Erratum 2147715.
 * This applies to revision r2p0, and is fixed in r2p1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0, x1, x17
 * ----------------------------------------------------------------
 */
func errata_a710_2147715_wa
	mov 	x17, x30
	bl 	check_errata_2147715
	cbz	x0, 1f

	/* Apply workaround; set CPUACTLR_EL1[22]
	 * to 1, which will cause the CFP instruction
	 * to invalidate all branch predictor resources
	 * regardless of context.
	 */
	mrs 	x1, CORTEX_A710_CPUACTLR_EL1
	orr	x1, x1, CORTEX_A710_CPUACTLR_EL1_BIT_22
	msr 	CORTEX_A710_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a710_2147715_wa

func check_errata_2147715
	mov 	x1, #0x20
	mov 	x2, #0x20
	b 	cpu_rev_var_range
endfunc check_errata_2147715

/* ---------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2216384.
 * This applies to revision r0p0, r1p0 and r2p0.
 * It is fixed in r2p1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------
 */
func errata_a710_2216384_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2216384
	cbz	x0, 1f

	/* Apply workaround: set CPUACTLR5_EL1[17]
	 * to 1 and the following instruction
	 * patching sequence.
	 */
	mrs	x1, CORTEX_A710_CPUACTLR5_EL1
	orr	x1, x1, CORTEX_A710_CPUACTLR5_EL1_BIT_17
	msr	CORTEX_A710_CPUACTLR5_EL1, x1

	ldr	x0,=0x5
	msr	CORTEX_A710_CPUPSELR_EL3, x0
	ldr	x0,=0x10F600E000
	msr	CORTEX_A710_CPUPOR_EL3, x0
	ldr	x0,=0x10FF80E000
	msr	CORTEX_A710_CPUPMR_EL3, x0
	ldr	x0,=0x80000000003FF
	msr	CORTEX_A710_CPUPCR_EL3, x0
	isb
1:
	ret 	x17
endfunc errata_a710_2216384_wa

func check_errata_2216384
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2216384

/* ---------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2282622.
 * This applies to revision r0p0, r1p0, r2p0 and r2p1.
 * It is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0, x1, x17
 * ---------------------------------------------------------------
 */
func errata_a710_2282622_wa
	/* Compare x0 against revision r2p1 */
	mov     x17, x30
	bl      check_errata_2282622
	cbz     x0, 1f

	/* Apply the workaround */
	mrs     x1, CORTEX_A710_CPUACTLR2_EL1
	orr     x1, x1, #BIT(0)
	msr     CORTEX_A710_CPUACTLR2_EL1, x1

1:
	ret     x17
endfunc errata_a710_2282622_wa

func check_errata_2282622
	/* Applies to r0p0, r1p0, r2p0 and r2p1 */
	mov     x1, #0x21
	b       cpu_rev_var_ls
endfunc check_errata_2282622

/* ------------------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2291219 on power down request.
 * This applies to revision <= r2p0 and is fixed in r2p1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x1, x17
 * ------------------------------------------------------------------------
 */
func errata_a710_2291219_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2291219
	cbz	x0, 1f

	/* Set bit 36 in ACTLR2_EL1 */
	mrs	x1, CORTEX_A710_CPUACTLR2_EL1
	orr	x1, x1, #CORTEX_A710_CPUACTLR2_EL1_BIT_36
	msr	CORTEX_A710_CPUACTLR2_EL1, x1
1:
	ret	x17
endfunc errata_a710_2291219_wa

func check_errata_2291219
	/* Applies to <= r2p0. */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2291219

/* ---------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2008768.
 * This applies to revision r0p0, r1p0 and r2p0.
 * It is fixed in r2p1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0, x1, x2, x17
 * ---------------------------------------------------------------
 */
func errata_a710_2008768_wa
	mov     x17, x30
	bl      check_errata_2008768
	cbz     x0, 1f

	/* Stash ERRSELR_EL1 in x2 */
	mrs	x2, ERRSELR_EL1

	/* Select error record 0 and clear ED bit */
	msr	ERRSELR_EL1, xzr
	mrs	x1, ERXCTLR_EL1
	bfi	x1, xzr, #ERXCTLR_ED_SHIFT, #1
	msr	ERXCTLR_EL1, x1

	/* Select error record 1 and clear ED bit */
	mov	x0, #1
	msr	ERRSELR_EL1, x0
	mrs	x1, ERXCTLR_EL1
	bfi	x1, xzr, #ERXCTLR_ED_SHIFT, #1
	msr	ERXCTLR_EL1, x1

	/* Restore ERRSELR_EL1 from x2 */
	msr	ERRSELR_EL1, x2

1:
	ret     x17
endfunc errata_a710_2008768_wa

func check_errata_2008768
	/* Applies to r0p0, r1p0 and r2p0 */
	mov     x1, #0x20
	b       cpu_rev_var_ls
endfunc check_errata_2008768

/* -------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2371105.
 * This applies to revisions <= r2p0 and is fixed in r2p1.
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * -------------------------------------------------------
 */
func errata_a710_2371105_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_2371105
	cbz	x0, 1f

	/* Set bit 40 in CPUACTLR2_EL1 */
	mrs	x1, CORTEX_A710_CPUACTLR2_EL1
	orr	x1, x1, #CORTEX_A710_CPUACTLR2_EL1_BIT_40
	msr	CORTEX_A710_CPUACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a710_2371105_wa

func check_errata_2371105
	/* Applies to <= r2p0. */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2371105

/* ----------------------------------------------------
 * Errata Workaround for Cortex-A710 Errata #2768515
 * This applies to revisions <= r2p1 and is still open.
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ----------------------------------------------------
 */
func errata_a710_2768515_wa
	mov	x17, x30
	bl	check_errata_2768515
	cbz	x0, 1f

	/* dsb before isb of power down sequence */
	dsb	sy
1:
	ret	x17
endfunc errata_a710_2768515_wa

func check_errata_2768515
	/* Applies to all revisions <= r2p1 */
	mov	x1, #0x21
	b	cpu_rev_var_ls
endfunc check_errata_2768515

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a710_core_pwr_dwn

#if ERRATA_A710_2008768
	mov	x4, x30
	bl	cpu_get_rev_var
	bl	errata_a710_2008768_wa
	mov	x30, x4
#endif

#if ERRATA_A710_2291219
	mov	x15, x30
	bl	cpu_get_rev_var
	bl	errata_a710_2291219_wa
	mov	x30, x15
#endif /* ERRATA_A710_2291219 */

	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_A710_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A710_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_A710_CPUPWRCTLR_EL1, x0
#if ERRATA_A710_2768515
	mov	x15, x30
	bl	cpu_get_rev_var
	bl	errata_a710_2768515_wa
	mov	x30, x15
#endif /* ERRATA_A710_2768515 */
	isb
	ret
endfunc cortex_a710_core_pwr_dwn

#if REPORT_ERRATA
	/*
	 * Errata printing function for Cortex-A710. Must follow AAPCS.
	 */
func cortex_a710_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A710_1987031, cortex_a710, 1987031
	report_errata ERRATA_A710_2081180, cortex_a710, 2081180
	report_errata ERRATA_A710_2055002, cortex_a710, 2055002
	report_errata ERRATA_A710_2017096, cortex_a710, 2017096
	report_errata ERRATA_A710_2083908, cortex_a710, 2083908
	report_errata ERRATA_A710_2058056, cortex_a710, 2058056
	report_errata ERRATA_A710_2267065, cortex_a710, 2267065
	report_errata ERRATA_A710_2136059, cortex_a710, 2136059
	report_errata ERRATA_A710_2282622, cortex_a710, 2282622
	report_errata ERRATA_A710_2008768, cortex_a710, 2008768
	report_errata ERRATA_A710_2147715, cortex_a710, 2147715
	report_errata ERRATA_A710_2216384, cortex_a710, 2216384
	report_errata ERRATA_A710_2291219, cortex_a710, 2291219
	report_errata ERRATA_A710_2371105, cortex_a710, 2371105
	report_errata ERRATA_A710_2768515, cortex_a710, 2768515
	report_errata WORKAROUND_CVE_2022_23960, cortex_a710, cve_2022_23960
	report_errata ERRATA_DSU_2313941, cortex_a710, dsu_2313941

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a710_errata_report
#endif

func cortex_a710_reset_func
	mov	x19, x30

	/* Disable speculative loads */
	msr	SSBS, xzr

	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_DSU_2313941
	bl	errata_dsu_2313941_wa
#endif

#if ERRATA_A710_1987031
	mov	x0, x18
	bl	errata_a710_1987031_wa
#endif

#if ERRATA_A710_2081180
	mov	x0, x18
	bl	errata_a710_2081180_wa
#endif

#if ERRATA_A710_2055002
	mov	x0, x18
	bl	errata_a710_2055002_wa
#endif

#if ERRATA_A710_2017096
	mov	x0, x18
	bl	errata_a710_2017096_wa
#endif

#if ERRATA_A710_2083908
	mov	x0, x18
	bl	errata_a710_2083908_wa
#endif

#if ERRATA_A710_2058056
	mov	x0, x18
	bl	errata_a710_2058056_wa
#endif

#if ERRATA_A710_2267065
	mov	x0, x18
	bl	errata_a710_2267065_wa
#endif

#if ERRATA_A710_2136059
	mov	x0, x18
	bl	errata_a710_2136059_wa
#endif

#if ERRATA_A710_2147715
	mov	x0, x18
	bl 	errata_a710_2147715_wa
#endif

#if ERRATA_A710_2216384
	mov	x0, x18
	bl 	errata_a710_2216384_wa
#endif /* ERRATA_A710_2216384 */

#if ERRATA_A710_2282622
	mov	x0, x18
	bl	errata_a710_2282622_wa
#endif

#if ERRATA_A710_2371105
	mov	x0, x18
	bl	errata_a710_2371105_wa
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-A710 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_a710
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */

	isb
	ret	x19
endfunc cortex_a710_reset_func

	/* ---------------------------------------------
	 * This function provides Cortex-A710 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a710_regs, "aS"
cortex_a710_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a710_cpu_reg_dump
	adr	x6, cortex_a710_regs
	mrs	x8, CORTEX_A710_CPUECTLR_EL1
	ret
endfunc cortex_a710_cpu_reg_dump

declare_cpu_ops cortex_a710, CORTEX_A710_MIDR, \
	cortex_a710_reset_func, \
	cortex_a710_core_pwr_dwn
