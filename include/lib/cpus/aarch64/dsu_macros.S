/*
 * Copyright (c) 2019-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DSU_MACROS_S
#define DSU_MACROS_S

#include <asm_macros.S>
#include <dsu_def.h>
#include <lib/cpus/errata.h>

.macro check_errata_dsu_798953_impl
	mov	x2, #ERRATA_APPLIES
	mov	x3, #ERRATA_NOT_APPLIES

	/* Check if DSU is equal to r0p0 */
	mrs	x1, CLUSTERIDR_EL1

	/* DSU variant and revision bitfields in CLUSTERIDR are adjacent */
	ubfx	x0, x1, #CLUSTERIDR_REV_SHIFT,\
			#(CLUSTERIDR_REV_BITS + CLUSTERIDR_VAR_BITS)
	mov	x1, #(0x0 << CLUSTERIDR_REV_SHIFT)
	cmp	x0, x1
	csel	x0, x2, x3, EQ
.endm

.macro errata_dsu_798953_wa_impl
	/* If erratum applies, disable high-level clock gating */
	mrs	x0, CLUSTERACTLR_EL1
	orr	x0, x0, #CLUSTERACTLR_EL1_DISABLE_CLOCK_GATING
	msr	CLUSTERACTLR_EL1, x0
.endm

.macro branch_if_scu_not_present _target:req
	/* Check if the SCU L3 Unit is present on the DSU */
	mrs	x0, CPUCFR_EL1
	ubfx	x0, x0, #SCU_SHIFT, #1
	eor	x0, x0, #1
	/* If SCU is not present, return without applying patch */
	cmp	x0, xzr
	mov	x0, #ERRATA_NOT_APPLIES
	b.eq	\_target
.endm

.macro check_errata_dsu_936184_impl
	mov	x0, #ERRATA_NOT_APPLIES
	/* Erratum applies only if DSU has the ACP interface */
	mrs	x1, CLUSTERCFR_EL1
	ubfx	x1, x1, #CLUSTERCFR_ACP_SHIFT, #1
	cbz	x1, 1f

	/* If ACP is present, check if DSU is older than r2p0 */
	mrs	x1, CLUSTERIDR_EL1

	/* DSU variant and revision bitfields in CLUSTERIDR are adjacent */
	ubfx	x2, x1, #CLUSTERIDR_REV_SHIFT,\
			#(CLUSTERIDR_REV_BITS + CLUSTERIDR_VAR_BITS)
	cmp x2, #(0x2 << CLUSTERIDR_VAR_SHIFT)
	b.hs	1f
	mov	x0, #ERRATA_APPLIES
1:
.endm

.macro errata_dsu_936184_wa_impl
	/* If erratum applies, we set a mask to a DSU control register */
	mrs	x0, CLUSTERACTLR_EL1
	ldr	x1, =DSU_ERRATA_936184_MASK
	orr	x0, x0, x1
	msr	CLUSTERACTLR_EL1, x0
.endm

.macro check_errata_dsu_2313941_impl
	mov	x2, #ERRATA_APPLIES
	mov	x3, #ERRATA_NOT_APPLIES

	/* Check if DSU version is less than or equal to r3p1 */
	mrs	x1, CLUSTERIDR_EL1

	mov	x0, #ERRATA_NOT_APPLIES
	/* DSU variant and revision bitfields in CLUSTERIDR are adjacent */
	ubfx	x0, x1, #CLUSTERIDR_REV_SHIFT,\
			#(CLUSTERIDR_REV_BITS + CLUSTERIDR_VAR_BITS)
	mov	x1, #(0x31 << CLUSTERIDR_REV_SHIFT)
	cmp	x0, x1
	csel	x0, x2, x3, LS
1:
.endm

.macro errata_dsu_2313941_wa_impl
	/* If erratum applies, disable high-level clock gating */
	mrs	x0, CLUSTERACTLR_EL1
	orr	x0, x0, #CLUSTERACTLR_EL1_DISABLE_SCLK_GATING
	msr	CLUSTERACTLR_EL1, x0
.endm

/*
 * Check if erratum is fixed via CLUSTERREVIDR_EL1 bit (\bitpos).
 * If not fixed (bit is clear), set x0 = ERRATA_APPLIES (from x3).
 * If fixed (bit is set), keep x0 = ERRATA_NOT_APPLIES.
 */
.macro check_revidr_bit bitpos:req
	mrs	x4, CLUSTERREVIDR_EL1
	mov	x1, #1
	lsl	x1, x1, #\bitpos
	tst	x1, x4
	csel	x0, x0, x3, NE
.endm

.macro check_errata_dsu_2900952_applies
	mov	x0, #ERRATA_NOT_APPLIES
	mov	x3, #ERRATA_APPLIES

	/* Check if DSU revision is equal to r2p0 */
	mrs	x1, CLUSTERIDR_EL1

	/* DSU variant and revision bitfields in CLUSTERIDR are adjacent */
	ubfx	x2, x1, #CLUSTERIDR_REV_SHIFT,\
			#(CLUSTERIDR_REV_BITS + CLUSTERIDR_VAR_BITS)
	cmp x2, #(0x2 << CLUSTERIDR_VAR_SHIFT)
	b.ne	1f
	check_revidr_bit 1
1:
.endm

.macro errata_dsu_2900952_wa_apply

	ldr	x1, =((CLUSTERACTLR_EL1_IGNORE_INTERCONNECT_CBUSY | \
				CLUSTERACTLR_EL1_ASSERT_CBUSY))

	mrs	x0, CLUSTERACTLR_EL1
	orr	x0, x0, x1
	msr	CLUSTERACTLR_EL1, x0
.endm

#endif /* DSU_MACROS_S */
