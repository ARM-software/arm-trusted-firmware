/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <pmc.h>
#include <tegra_def.h>

#define RESET_ENABLE	0x10U

/* Module IDs used during power ungate procedure */
static const uint32_t pmc_cpu_powergate_id[4] = {
	0, /* CPU 0 */
	9, /* CPU 1 */
	10, /* CPU 2 */
	11 /* CPU 3 */
};

/*******************************************************************************
 * Power ungate CPU to start the boot process. CPU reset vectors must be
 * populated before calling this function.
 ******************************************************************************/
void tegra_pmc_cpu_on(int32_t cpu)
{
	uint32_t val;

	/*
	 * Check if CPU is already power ungated
	 */
	val = tegra_pmc_read_32(PMC_PWRGATE_STATUS);
	if ((val & (1UL << pmc_cpu_powergate_id[cpu])) == 0U) {
		/*
		 * The PMC deasserts the START bit when it starts the power
		 * ungate process. Loop till no power toggle is in progress.
		 */
		do {
			val = tegra_pmc_read_32(PMC_PWRGATE_TOGGLE);
		} while ((val & PMC_TOGGLE_START) != 0U);

		/*
		 * Start the power ungate procedure
		 */
		val = pmc_cpu_powergate_id[cpu] | PMC_TOGGLE_START;
		tegra_pmc_write_32(PMC_PWRGATE_TOGGLE, val);

		/*
		 * The PMC deasserts the START bit when it starts the power
		 * ungate process. Loop till powergate START bit is asserted.
		 */
		do {
			val = tegra_pmc_read_32(PMC_PWRGATE_TOGGLE);
		} while ((val & (1UL << 8)) != 0U);

		/* loop till the CPU is power ungated */
		do {
			val = tegra_pmc_read_32(PMC_PWRGATE_STATUS);
		} while ((val & (1UL << pmc_cpu_powergate_id[cpu])) == 0U);
	}
}

/*******************************************************************************
 * Setup CPU vectors for resume from deep sleep
 ******************************************************************************/
void tegra_pmc_cpu_setup(uint64_t reset_addr)
{
	uint32_t val;

	tegra_pmc_write_32(PMC_SECURE_SCRATCH34,
			   ((uint32_t)reset_addr & 0xFFFFFFFFU) | 1U);
	val = (uint32_t)(reset_addr >> 32U);
	tegra_pmc_write_32(PMC_SECURE_SCRATCH35, val & 0x7FFU);
}

/*******************************************************************************
 * Lock CPU vectors to restrict further writes
 ******************************************************************************/
void tegra_pmc_lock_cpu_vectors(void)
{
	uint32_t val;

	/* lock PMC_SECURE_SCRATCH22 */
	val = tegra_pmc_read_32(PMC_SECURE_DISABLE2);
	val |= PMC_SECURE_DISABLE2_WRITE22_ON;
	tegra_pmc_write_32(PMC_SECURE_DISABLE2, val);

	/* lock PMC_SECURE_SCRATCH34/35 */
	val = tegra_pmc_read_32(PMC_SECURE_DISABLE3);
	val |= (PMC_SECURE_DISABLE3_WRITE34_ON |
		PMC_SECURE_DISABLE3_WRITE35_ON);
	tegra_pmc_write_32(PMC_SECURE_DISABLE3, val);
}

/*******************************************************************************
 * Restart the system
 ******************************************************************************/
__dead2 void tegra_pmc_system_reset(void)
{
	uint32_t reg;

	reg = tegra_pmc_read_32(PMC_CONFIG);
	reg |= RESET_ENABLE;		/* restart */
	tegra_pmc_write_32(PMC_CONFIG, reg);
	wfi();

	ERROR("Tegra System Reset: operation not handled.\n");
	panic();
}
