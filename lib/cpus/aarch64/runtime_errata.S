/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_a510.h>
#include <cpu_macros.S>

/*
 * void apply_cpu_pwr_dwn_errata(void);
 *
 * This function applies various CPU errata during power down.
 */
	.globl apply_cpu_pwr_dwn_errata
func apply_cpu_pwr_dwn_errata
	mov	x19, x30
	bl      cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A510_2684597
	bl erratum_cortex_a510_2684597_wa
#endif

	ret	x19
endfunc apply_cpu_pwr_dwn_errata
