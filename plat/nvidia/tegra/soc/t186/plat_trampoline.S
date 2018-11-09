/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <memctrl_v2.h>
#include <plat/common/common_def.h>
#include <tegra_def.h>

#define TEGRA186_MC_CTX_SIZE		0x93

	.globl tegra186_get_mc_ctx_size

	/*
	 * Tegra186 reset data (offset 0x0 - 0x420)
	 *
	 * 0x000: MC context start
	 * 0x420: MC context end
	 */

	.align 4
__tegra186_mc_context:
	.rept	TEGRA186_MC_CTX_SIZE
	.quad	0
	.endr

	.align 4
__tegra186_mc_context_end:

/* return the size of the MC context */
func tegra186_get_mc_ctx_size
	adr	x0, __tegra186_mc_context_end
	adr	x1, __tegra186_mc_context
	sub	x0, x0, x1
	ret
endfunc tegra186_get_mc_ctx_size
