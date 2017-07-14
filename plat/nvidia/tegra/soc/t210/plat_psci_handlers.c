/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <flowctrl.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <pmc.h>
#include <psci.h>
#include <tegra_def.h>
#include <tegra_private.h>

/*
 * Register used to clear CPU reset signals. Each CPU has two reset
 * signals: CPU reset (3:0) and Core reset (19:16).
 */
#define CPU_CMPLX_RESET_CLR		0x454
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

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_POWERDN:
		/*
		 * Core powerdown request only for afflvl 0
		 */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id & 0xff;

		break;

	case PSTATE_ID_CLUSTER_IDLE:
	case PSTATE_ID_CLUSTER_POWERDN:
		/*
		 * Cluster powerdown/idle request only for afflvl 1
		 */
		req_state->pwr_domain_state[MPIDR_AFFLVL1] = state_id;
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id;

		break;

	case PSTATE_ID_SOC_POWERDN:
		/*
		 * System powerdown request only for afflvl 2
		 */
		for (uint32_t i = MPIDR_AFFLVL0; i < PLAT_MAX_PWR_LVL; i++)
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;

		req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] =
			PLAT_SYS_SUSPEND_STATE_ID;

		break;

	default:
		ERROR("%s: unsupported state id (%d)\n", __func__, state_id);
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler to calculate the proper target power level at the
 * specified affinity level
 ******************************************************************************/
plat_local_state_t tegra_soc_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = *states;
	int cpu = plat_my_core_pos();
	int core_pos = read_mpidr() & MPIDR_CPU_MASK;

	/* get the power state at this level */
	if (lvl == MPIDR_AFFLVL1)
		target = *(states + core_pos);
	if (lvl == MPIDR_AFFLVL2)
		target = *(states + cpu);

	/* Cluster idle/power-down */
	if ((lvl == MPIDR_AFFLVL1) && ((target == PSTATE_ID_CLUSTER_IDLE) ||
	    (target == PSTATE_ID_CLUSTER_POWERDN))) {
		return target;
	}

	/* System Suspend */
	if (((lvl == MPIDR_AFFLVL2) || (lvl == MPIDR_AFFLVL1)) &&
	    (target == PSTATE_ID_SOC_POWERDN))
		return PSTATE_ID_SOC_POWERDN;

	/* default state */
	return PSCI_LOCAL_STATE_RUN;
}

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr();
	const plat_local_state_t *pwr_domain_state =
		target_state->pwr_domain_state;
	unsigned int stateid_afflvl2 = pwr_domain_state[MPIDR_AFFLVL2];
	unsigned int stateid_afflvl1 = pwr_domain_state[MPIDR_AFFLVL1];
	unsigned int stateid_afflvl0 = pwr_domain_state[MPIDR_AFFLVL0];

	if (stateid_afflvl2 == PSTATE_ID_SOC_POWERDN) {

		assert((stateid_afflvl0 == PLAT_MAX_OFF_STATE) ||
		       (stateid_afflvl0 == PSTATE_ID_SOC_POWERDN));
		assert((stateid_afflvl1 == PLAT_MAX_OFF_STATE) ||
		       (stateid_afflvl1 == PSTATE_ID_SOC_POWERDN));

		/* suspend the entire soc */
		tegra_fc_soc_powerdn(mpidr);

	} else if (stateid_afflvl1 == PSTATE_ID_CLUSTER_IDLE) {

		assert(stateid_afflvl0 == PSTATE_ID_CLUSTER_IDLE);

		/* Prepare for cluster idle */
		tegra_fc_cluster_idle(mpidr);

	} else if (stateid_afflvl1 == PSTATE_ID_CLUSTER_POWERDN) {

		assert(stateid_afflvl0 == PSTATE_ID_CLUSTER_POWERDN);

		/* Prepare for cluster powerdn */
		tegra_fc_cluster_powerdn(mpidr);

	} else if (stateid_afflvl0 == PSTATE_ID_CORE_POWERDN) {

		/* Prepare for cpu powerdn */
		tegra_fc_cpu_powerdn(mpidr);

	} else {
		ERROR("%s: Unknown state id\n", __func__);
		return PSCI_E_NOT_SUPPORTED;
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint32_t val;

	/*
	 * Check if we are exiting from SOC_POWERDN.
	 */
	if (target_state->pwr_domain_state[PLAT_MAX_PWR_LVL] ==
			PLAT_SYS_SUSPEND_STATE_ID) {

		/*
		 * Lock scratch registers which hold the CPU vectors
		 */
		tegra_pmc_lock_cpu_vectors();

		/*
		 * Enable WRAP to INCR burst type conversions for
		 * incoming requests on the AXI slave ports.
		 */
		val = mmio_read_32(TEGRA_MSELECT_BASE + MSELECT_CONFIG);
		val &= ~ENABLE_UNSUP_TX_ERRORS;
		val |= ENABLE_WRAP_TO_INCR_BURSTS;
		mmio_write_32(TEGRA_MSELECT_BASE + MSELECT_CONFIG, val);

		/*
		 * Restore Boot and Power Management Processor (BPMP) reset
		 * address and reset it.
		 */
		tegra_fc_reset_bpmp();
	}

	/*
	 * T210 has a dedicated ARMv7 boot and power mgmt processor, BPMP. It's
	 * used for power management and boot purposes. Inform the BPMP that
	 * we have completed the cluster power up.
	 */
	tegra_fc_lock_active_cluster();

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t mask = CPU_CORE_RESET_MASK << cpu;

	/* Deassert CPU reset signals */
	mmio_write_32(TEGRA_CAR_RESET_BASE + CPU_CMPLX_RESET_CLR, mask);

	/* Turn on CPU using flow controller or PMC */
	if (cpu_powergate_mask[cpu] == 0) {
		tegra_pmc_cpu_on(cpu);
		cpu_powergate_mask[cpu] = 1;
	} else {
		tegra_fc_cpu_on(cpu);
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	tegra_fc_cpu_off(read_mpidr() & MPIDR_CPU_MASK);
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
