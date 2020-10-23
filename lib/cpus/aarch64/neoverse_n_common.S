/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <neoverse_n_common.h>

	.global is_scu_present_in_dsu

/*
 * Check if the SCU L3 Unit is present on the DSU
 * 1-> SCU present
 * 0-> SCU not present
 *
 * This function is implemented as weak on dsu_helpers.S and must be
 * overwritten for Neoverse Nx cores.
 */

func is_scu_present_in_dsu
	mrs	x0, CPUCFR_EL1
	ubfx	x0, x0, #SCU_SHIFT, #1
	eor	x0, x0, #1
	ret
endfunc is_scu_present_in_dsu
