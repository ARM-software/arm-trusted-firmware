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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <denver.h>
#include <debug.h>
#include <flowctrl.h>
#include <mmio.h>
#include <platform_def.h>
#include <pmc.h>
#include <psci.h>
#include <tegra_def.h>
#include <tegra_private.h>

/*
 * Register used to clear CPU reset signals. Each CPU has two reset
 * signals: CPU reset (3:0) and Core reset (19:16)
 */
#define CPU_CMPLX_RESET_CLR		0x344
#define CPU_CORE_RESET_MASK		0x10001

static int cpu_powergate_mask[PLATFORM_MAX_CPUS_PER_CLUSTER];

int32_t tegra_soc_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

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
	if ((pwr_lvl != PLAT_MAX_PWR_LVL) ||
	    (state_id != PLAT_SYS_SUSPEND_STATE_ID)) {
		ERROR("unsupported state id @ power level\n");
		return PSCI_E_INVALID_PARAMS;
	}

	/* all power domain in 'OFF' state */
	for (int i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
		req_state->pwr_domain_state[i] = power_state;

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

int tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	tegra_fc_cpu_off(read_mpidr() & MPIDR_CPU_MASK);
	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/* Allow restarting CPU #1 using PMC on suspend exit */
	cpu_powergate_mask[1] = 0;

	/* Program FC to enter suspend state */
	tegra_fc_cpu_idle(read_mpidr());

	/* Suspend DCO operations */
	write_actlr_el1(target_state->pwr_domain_state[PLAT_MAX_PWR_LVL]);

	return PSCI_E_SUCCESS;
}
