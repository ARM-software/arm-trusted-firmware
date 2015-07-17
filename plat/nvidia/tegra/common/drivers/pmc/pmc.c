/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <pmc.h>
#include <tegra_def.h>

/* Module IDs used during power ungate procedure */
static const int pmc_cpu_powergate_id[4] = {
	0, /* CPU 0 */
	9, /* CPU 1 */
	10, /* CPU 2 */
	11 /* CPU 3 */
};

/*******************************************************************************
 * Power ungate CPU to start the boot process. CPU reset vectors must be
 * populated before calling this function.
 ******************************************************************************/
void tegra_pmc_cpu_on(int cpu)
{
	uint32_t val;

	/*
	 * Check if CPU is already power ungated
	 */
	val = tegra_pmc_read_32(PMC_PWRGATE_STATUS);
	if (val & (1 << pmc_cpu_powergate_id[cpu]))
		return;

	/*
	 * The PMC deasserts the START bit when it starts the power
	 * ungate process. Loop till no power toggle is in progress.
	 */
	do {
		val = tegra_pmc_read_32(PMC_PWRGATE_TOGGLE);
	} while (val & PMC_TOGGLE_START);

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
	} while (val & (1 << 8));

	/* loop till the CPU is power ungated */
	do {
		val = tegra_pmc_read_32(PMC_PWRGATE_STATUS);
	} while ((val & (1 << pmc_cpu_powergate_id[cpu])) == 0);
}

/*******************************************************************************
 * Setup CPU vectors for resume from deep sleep
 ******************************************************************************/
void tegra_pmc_cpu_setup(uint64_t reset_addr)
{
	uint32_t val;

	tegra_pmc_write_32(PMC_SECURE_SCRATCH34, (reset_addr & 0xFFFFFFFF) | 1);
	val = reset_addr >> 32;
	tegra_pmc_write_32(PMC_SECURE_SCRATCH35, val & 0x7FF);
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
	reg |= 0x10;		/* restart */
	tegra_pmc_write_32(PMC_CONFIG, reg);
	wfi();

	ERROR("Tegra System Reset: operation not handled.\n");
	panic();
}
