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
#include <delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include <pmc.h>
#include <flowctrl.h>
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
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL) {
		ERROR("%s: unsupported power_state (0x%x)\n", __func__,
			power_state);
		return PSCI_E_INVALID_PARAMS;
	}

	/* Sanity check the requested afflvl */
	if (psci_get_pstate_type(power_state) == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on affinity level 0 i.e.
		 * a cpu on Tegra. Ignore any other affinity level.
		 */
		if (pwr_lvl != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;

		/* power domain in standby state */
		req_state->pwr_domain_state[pwr_lvl] = PLAT_MAX_RET_STATE;

		return PSCI_E_SUCCESS;
	}

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_POWERDN:
		/*
		 * Core powerdown request only for afflvl 0
		 */
		if (pwr_lvl != MPIDR_AFFLVL0)
			goto error;

		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id & 0xff;

		break;

	case PSTATE_ID_CLUSTER_IDLE:
	case PSTATE_ID_CLUSTER_POWERDN:
		/*
		 * Cluster powerdown/idle request only for afflvl 1
		 */
		if (pwr_lvl != MPIDR_AFFLVL1)
			goto error;

		req_state->pwr_domain_state[MPIDR_AFFLVL1] = state_id;
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_OFF_STATE;

		break;

	case PSTATE_ID_SOC_POWERDN:
		/*
		 * System powerdown request only for afflvl 2
		 */
		if (pwr_lvl != PLAT_MAX_PWR_LVL)
			goto error;

		for (int i = MPIDR_AFFLVL0; i < PLAT_MAX_PWR_LVL; i++)
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;

		req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] =
			PLAT_SYS_SUSPEND_STATE_ID;

		break;

	default:
		ERROR("%s: unsupported state id (%d)\n", __func__, state_id);
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;

error:
	ERROR("%s: unsupported state id (%d)\n", __func__, state_id);
	return PSCI_E_INVALID_PARAMS;
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

		assert(stateid_afflvl0 == PLAT_MAX_OFF_STATE);
		assert(stateid_afflvl1 == PLAT_MAX_OFF_STATE);

		/* suspend the entire soc */
		tegra_fc_soc_powerdn(mpidr);

	} else if (stateid_afflvl1 == PSTATE_ID_CLUSTER_IDLE) {

		assert(stateid_afflvl0 == PLAT_MAX_OFF_STATE);

		/* Prepare for cluster idle */
		tegra_fc_cluster_idle(mpidr);

	} else if (stateid_afflvl1 == PSTATE_ID_CLUSTER_POWERDN) {

		assert(stateid_afflvl0 == PLAT_MAX_OFF_STATE);

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
