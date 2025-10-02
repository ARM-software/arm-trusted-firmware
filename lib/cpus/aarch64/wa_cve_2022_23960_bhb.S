/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <context.h>

#if WORKAROUND_CVE_2022_23960
	/*
	 * This macro applies the mitigation for CVE-2022-23960.
         * The macro saves x2 to the CPU context.
         * SP should point to the CPU context.
	 */
	.macro	apply_cve_2022_23960_bhb_wa _bhb_loop_count

#if ENABLE_FEAT_CLRBHB
	/*
	 * Platforms with FEAT_CLRBHB can omit the loop.
	 */
	clrbhb
	isb
#endif /* ENABLE_FEAT_CLRBHB */

#if ENABLE_FEAT_CLRBHB != 1
	str	x2, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]

	/* CVE-BHB-NUM loop count */
	mov	x2, \_bhb_loop_count

1:
	/* b pc+4 part of the workaround */
	b	2f
2:
	subs	x2, x2, #1
	bne	1b
	speculation_barrier
	ldr	x2, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
3:
#endif	/* ENABLE_FEAT_CLRBHB != 1 */
	.endm
#endif /* WORKAROUND_CVE_2022_23960 */
