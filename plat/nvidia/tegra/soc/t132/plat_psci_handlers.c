/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <denver.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <flowctrl.h>
#include <pmc.h>
#include <tegra_def.h>
#include <tegra_private.h>

/*
 * Register used to clear CPU reset signals. Each CPU has two reset
 * signals: CPU reset (3:0) and Core reset (19:16)
 */
#define CPU_CMPLX_RESET_CLR		0x344
#define CPU_CORE_RESET_MASK		0x10001

/* Clock and Reset controller registers for system clock's settings */
#define SCLK_RATE			0x30
#define SCLK_BURST_POLICY		0x28
#define SCLK_BURST_POLICY_DEFAULT	0x10000000

static int cpu_powergate_mask[PLATFORM_MAX_CPUS_PER_CLUSTER];

int32_t tegra_soc_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int state_id = psci_get_pstate_id(power_state);
	int cpu = read_mpidr() & MPIDR_CPU_MASK;

	/*
	 * Sanity check the requested state id, power level and CPU number.
	 * Currently T132 only supports SYSTEM_SUSPEND on last standing CPU
	 * i.e. CPU 0
	 */
	if ((state_id != PSTATE_ID_SOC_POWERDN) || (cpu != 0)) {
		ERROR("unsupported state id @ power level\n");
		return PSCI_E_INVALID_PARAMS;
	}

	/* Set lower power states to PLAT_MAX_OFF_STATE */
	for (uint32_t i = MPIDR_AFFLVL0; i < PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;

	/* Set the SYSTEM_SUSPEND state-id */
	req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] =
		PSTATE_ID_SOC_POWERDN;

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t mask = CPU_CORE_RESET_MASK << cpu;

	if (cpu_powergate_mask[cpu] == 0) {

		/* Deassert CPU reset signals */
		mmio_write_32(TEGRA_CAR_RESET_BASE + CPU_CMPLX_RESET_CLR, mask);

		/* Power on CPU using PMC */
		tegra_pmc_cpu_on(cpu);

		/* Fill in the CPU powergate mask */
		cpu_powergate_mask[cpu] = 1;

	} else {
		/* Power on CPU using Flow Controller */
		tegra_fc_cpu_on(cpu);
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/*
	 * Lock scratch registers which hold the CPU vectors
	 */
	tegra_pmc_lock_cpu_vectors();

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t val;

	tegra_fc_cpu_off(read_mpidr() & MPIDR_CPU_MASK);

	/* Disable DCO operations */
	denver_disable_dco();

	/* Power down the CPU */
	val = read_actlr_el1() & ~ACTLR_EL1_PMSTATE_MASK;
	write_actlr_el1(val | DENVER_CPU_STATE_POWER_DOWN);

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t val;

#if ENABLE_ASSERTIONS
	int cpu = read_mpidr() & MPIDR_CPU_MASK;

	/* SYSTEM_SUSPEND only on CPU0 */
	assert(cpu == 0);
#endif

	/* Allow restarting CPU #1 using PMC on suspend exit */
	cpu_powergate_mask[1] = 0;

	/* Program FC to enter suspend state */
	tegra_fc_cpu_powerdn(read_mpidr());

	/* Disable DCO operations */
	denver_disable_dco();

	/* Program the suspend state ID */
	val = read_actlr_el1() & ~ACTLR_EL1_PMSTATE_MASK;
	write_actlr_el1(val | target_state->pwr_domain_state[PLAT_MAX_PWR_LVL]);

	return PSCI_E_SUCCESS;
}

int tegra_soc_prepare_system_reset(void)
{
	/*
	 * Set System Clock (SCLK) to POR default so that the clock source
	 * for the PMC APB clock would not be changed due to system reset.
	 */
	mmio_write_32((uintptr_t)TEGRA_CAR_RESET_BASE + SCLK_BURST_POLICY,
		       SCLK_BURST_POLICY_DEFAULT);
	mmio_write_32((uintptr_t)TEGRA_CAR_RESET_BASE + SCLK_RATE, 0);

	/* Wait 1 ms to make sure clock source/device logic is stabilized. */
	mdelay(1);

	return PSCI_E_SUCCESS;
}
