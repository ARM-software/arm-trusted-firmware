/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mce.h>
#include <string.h>
#include <tegra_def.h>
#include <tegra_private.h>

#define MISCREG_CPU_RESET_VECTOR	0x2000U
#define MISCREG_AA64_RST_LOW		0x2004U
#define MISCREG_AA64_RST_HIGH		0x2008U

#define CPU_RESET_MODE_AA64		1U

extern void tegra194_cpu_reset_handler(void);
extern uint64_t __tegra194_smmu_ctx_start;

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t addr_low, addr_high;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t cpu_reset_handler_base = params_from_bl2->tzdram_base;

	INFO("Setting up secondary CPU boot\n");

	memcpy((void *)((uintptr_t)cpu_reset_handler_base),
		 (void *)(uintptr_t)tegra194_cpu_reset_handler,
		 (uintptr_t)&__tegra194_smmu_ctx_start -
		 (uintptr_t)&tegra194_cpu_reset_handler);

	addr_low = (uint32_t)cpu_reset_handler_base | CPU_RESET_MODE_AA64;
	addr_high = (uint32_t)((cpu_reset_handler_base >> 32U) & 0x7ffU);

	/* write lower 32 bits first, then the upper 11 bits */
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_LOW, addr_low);
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_HIGH, addr_high);

	/* save reset vector to be used during SYSTEM_SUSPEND exit */
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_LO,
			addr_low);
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_HI,
			addr_high);
}
