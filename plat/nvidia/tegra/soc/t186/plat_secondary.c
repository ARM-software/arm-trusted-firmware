/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <debug.h>
#include <mce.h>
#include <mmio.h>
#include <string.h>
#include <tegra_def.h>
#include <tegra_private.h>

#define MISCREG_CPU_RESET_VECTOR	0x2000
#define MISCREG_AA64_RST_LOW		0x2004
#define MISCREG_AA64_RST_HIGH		0x2008

#define SCRATCH_SECURE_RSV1_SCRATCH_0	0x658
#define SCRATCH_SECURE_RSV1_SCRATCH_1	0x65C

#define CPU_RESET_MODE_AA64		1

extern uint64_t tegra_bl31_phys_base;
extern uint64_t __tegra186_cpu_reset_handler_end;

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t addr_low, addr_high;
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t cpu_reset_handler_base;

	INFO("Setting up secondary CPU boot\n");

	if ((tegra_bl31_phys_base >= TEGRA_TZRAM_BASE) &&
	    (tegra_bl31_phys_base <= (TEGRA_TZRAM_BASE + TEGRA_TZRAM_SIZE))) {

		/*
		 * The BL31 code resides in the TZSRAM which loses state
		 * when we enter System Suspend. Copy the wakeup trampoline
		 * code to TZDRAM to help us exit from System Suspend.
		 */
		cpu_reset_handler_base = params_from_bl2->tzdram_base;
		memcpy16((void *)((uintptr_t)cpu_reset_handler_base),
			 (void *)(uintptr_t)tegra186_cpu_reset_handler,
			 (uintptr_t)&__tegra186_cpu_reset_handler_end -
			 (uintptr_t)tegra186_cpu_reset_handler);

	} else {
		cpu_reset_handler_base = (uintptr_t)tegra_secure_entrypoint;
	}

	addr_low = (uint32_t)cpu_reset_handler_base | CPU_RESET_MODE_AA64;
	addr_high = (uint32_t)((cpu_reset_handler_base >> 32) & 0x7ff);

	/* write lower 32 bits first, then the upper 11 bits */
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_LOW, addr_low);
	mmio_write_32(TEGRA_MISC_BASE + MISCREG_AA64_RST_HIGH, addr_high);

	/* save reset vector to be used during SYSTEM_SUSPEND exit */
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_SECURE_RSV1_SCRATCH_0,
			addr_low);
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_SECURE_RSV1_SCRATCH_1,
			addr_high);

	/* update reset vector address to the CCPLEX */
	mce_update_reset_vector();
}
