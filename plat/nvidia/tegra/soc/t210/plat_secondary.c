/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <pmc.h>
#include <tegra_def.h>

#define SB_CSR				0x0
#define  SB_CSR_NS_RST_VEC_WR_DIS	(1 << 1)

/* CPU reset vector */
#define SB_AA64_RESET_LOW		0x30	/* width = 31:0 */
#define SB_AA64_RESET_HI		0x34	/* width = 11:0 */

extern void tegra_secure_entrypoint(void);

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t val;
	uint64_t reset_addr = (uint64_t)tegra_secure_entrypoint;

	INFO("Setting up secondary CPU boot\n");

	/* setup secondary CPU vector */
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_LOW,
			(reset_addr & 0xFFFFFFFF) | 1);
	val = reset_addr >> 32;
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_HI, val & 0x7FF);

	/* configure PMC */
	tegra_pmc_cpu_setup(reset_addr);
	tegra_pmc_lock_cpu_vectors();
}
