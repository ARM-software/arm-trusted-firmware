/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <mce.h>
#include <tegra_def.h>
#include <tegra_private.h>

#define SCRATCH_SECURE_RSV1_SCRATCH_0	0x658U
#define SCRATCH_SECURE_RSV1_SCRATCH_1	0x65CU

#define CPU_RESET_MODE_AA64		1U

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t addr_low, addr_high;

	INFO("Setting up secondary CPU boot\n");

	/* TZDRAM base will be used as the "resume" address */
	addr_low = (uintptr_t)&tegra_secure_entrypoint | CPU_RESET_MODE_AA64;
	addr_high = (uintptr_t)(((uintptr_t)&tegra_secure_entrypoint >> 32U) & 0x7ffU);

	/* save reset vector to be used during SYSTEM_SUSPEND exit */
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_LO,
			addr_low);
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_RESET_VECTOR_HI,
			addr_high);
}
