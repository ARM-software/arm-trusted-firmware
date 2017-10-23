/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <mce.h>
#include <tegra186_private.h>
#include <tegra_def.h>
#include <tegra_private.h>

#define MISCREG_AA64_RST_LOW		0x2004U
#define MISCREG_AA64_RST_HIGH		0x2008U

#define SCRATCH_SECURE_RSV1_SCRATCH_0	0x658U
#define SCRATCH_SECURE_RSV1_SCRATCH_1	0x65CU

#define CPU_RESET_MODE_AA64		1U

extern void memcpy16(void *dest, const void *src, unsigned int length);

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t addr_low, addr_high;
	const plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t cpu_reset_handler_base, cpu_reset_handler_size;

	INFO("Setting up secondary CPU boot\n");

	/*
	 * The BL31 code resides in the TZSRAM which loses state
	 * when we enter System Suspend. Copy the wakeup trampoline
	 * code to TZDRAM to help us exit from System Suspend.
	 */
	cpu_reset_handler_base = tegra186_get_cpu_reset_handler_base();
	cpu_reset_handler_size = tegra186_get_cpu_reset_handler_size();
	(void)memcpy16((void *)(uintptr_t)params_from_bl2->tzdram_base,
			(const void *)(uintptr_t)cpu_reset_handler_base,
			cpu_reset_handler_size);

	/* TZDRAM base will be used as the "resume" address */
	addr_low = (uint32_t)params_from_bl2->tzdram_base | CPU_RESET_MODE_AA64;
	addr_high = (uint32_t)((params_from_bl2->tzdram_base >> 32U) & 0x7ffU);

	/* write lower 32 bits first, then the upper 11 bits */
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_LOW, addr_low);
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_HIGH, addr_high);

	/* save reset vector to be used during SYSTEM_SUSPEND exit */
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_LO,
			addr_low);
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_HI,
			addr_high);

	/* update reset vector address to the CCPLEX */
	(void)mce_update_reset_vector();
}
