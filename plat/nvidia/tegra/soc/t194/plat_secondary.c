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

#define MISCREG_CPU_RESET_VECTOR	0x2000
#define MISCREG_AA64_RST_LOW		0x2004
#define MISCREG_AA64_RST_HIGH		0x2008

#define CPU_RESET_MODE_AA64		1

extern void tegra_secure_entrypoint(void);

#if ENABLE_SYSTEM_SUSPEND_CTX_SAVE_TZDRAM
extern void tegra186_cpu_reset_handler(void);
extern uint64_t __tegra186_smmu_ctx_start;
#endif

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t addr_low, addr_high;
#if ENABLE_SYSTEM_SUSPEND_CTX_SAVE_TZDRAM
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t cpu_reset_handler_base = params_from_bl2->tzdram_base;
#else
	uint64_t cpu_reset_handler_base = (uintptr_t)tegra_secure_entrypoint;
#endif

	INFO("Setting up secondary CPU boot\n");

#if ENABLE_SYSTEM_SUSPEND_CTX_SAVE_TZDRAM
	memcpy((void *)((uintptr_t)cpu_reset_handler_base),
		 (void *)(uintptr_t)tegra186_cpu_reset_handler,
		 (uintptr_t)&__tegra186_smmu_ctx_start -
		 (uintptr_t)tegra186_cpu_reset_handler);
#endif

	addr_low = (uint32_t)cpu_reset_handler_base | CPU_RESET_MODE_AA64;
	addr_high = (uint32_t)((cpu_reset_handler_base >> 32) & 0x7ff);

	/* write lower 32 bits first, then the upper 11 bits */
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_LOW, addr_low);
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_HIGH, addr_high);

	/* save reset vector to be used during SYSTEM_SUSPEND exit */
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV1_LO,
			addr_low);
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV1_HI,
			addr_high);

	/* update reset vector address to the CCPLEX */
	mce_update_reset_vector();
}
