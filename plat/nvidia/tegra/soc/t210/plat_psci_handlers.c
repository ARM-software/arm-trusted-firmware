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

static int cpu_powergate_mask[PLATFORM_MAX_CPUS_PER_CLUSTER];

int32_t tegra_soc_validate_power_state(unsigned int power_state)
{
	/* Sanity check the requested afflvl */
	if (psci_get_pstate_type(power_state) == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on affinity level 0 i.e.
		 * a cpu on Tegra. Ignore any other affinity level.
		 */
		if (psci_get_pstate_afflvl(power_state) != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;
	}

	/* Sanity check the requested state id */
	switch (psci_get_pstate_id(power_state)) {
	case PSTATE_ID_CORE_POWERDN:
	case PSTATE_ID_CLUSTER_IDLE:
	case PSTATE_ID_CLUSTER_POWERDN:
	case PSTATE_ID_SOC_POWERDN:
		break;

	default:
		ERROR("unsupported state id\n");
		return PSCI_E_NOT_SUPPORTED;
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_prepare_cpu_suspend(unsigned int id, unsigned int afflvl)
{
	/* There's nothing to be done for affinity level 1 */
	if (afflvl == MPIDR_AFFLVL1)
		return PSCI_E_SUCCESS;

	switch (id) {
	/* Prepare for cpu idle */
	case PSTATE_ID_CORE_POWERDN:
		tegra_fc_cpu_idle(read_mpidr());
		return PSCI_E_SUCCESS;

	/* Prepare for cluster idle */
	case PSTATE_ID_CLUSTER_IDLE:
		tegra_fc_cluster_idle(read_mpidr());
		return PSCI_E_SUCCESS;

	/* Prepare for cluster powerdn */
	case PSTATE_ID_CLUSTER_POWERDN:
		tegra_fc_cluster_powerdn(read_mpidr());
		return PSCI_E_SUCCESS;

	/* Prepare for system idle */
	case PSTATE_ID_SOC_POWERDN:

		/* Enter system suspend state */
		tegra_pm_system_suspend_entry();

		/* suspend the entire soc */
		tegra_fc_soc_powerdn(read_mpidr());

		return PSCI_E_SUCCESS;

	default:
		ERROR("Unknown state id (%d)\n", id);
		break;
	}

	return PSCI_E_NOT_SUPPORTED;
}

int tegra_soc_prepare_cpu_on_finish(unsigned long mpidr)
{
	uint32_t val;

	/*
	 * Check if we are exiting from SOC_POWERDN.
	 */
	if (tegra_system_suspended()) {

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

		/*
		 * System resume complete.
		 */
		tegra_pm_system_suspend_exit();
	}

	/*
	 * T210 has a dedicated ARMv7 boot and power mgmt processor, BPMP. It's
	 * used for power management and boot purposes. Inform the BPMP that
	 * we have completed the cluster power up.
	 */
	if (psci_get_max_phys_off_afflvl() == MPIDR_AFFLVL1)
		tegra_fc_lock_active_cluster();

	return PSCI_E_SUCCESS;
}

int tegra_soc_prepare_cpu_on(unsigned long mpidr)
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

int tegra_soc_prepare_cpu_off(unsigned long mpidr)
{
	tegra_fc_cpu_off(mpidr & MPIDR_CPU_MASK);
	return PSCI_E_SUCCESS;
}
