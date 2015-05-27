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

/* Power down state IDs */
#define PSTATE_ID_CORE_POWERDN		7
#define PSTATE_ID_CLUSTER_IDLE		16
#define PSTATE_ID_CLUSTER_POWERDN	17
#define PSTATE_ID_SOC_POWERDN		27

static int cpu_powergate_mask[PLATFORM_MAX_CPUS_PER_CLUSTER];

int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl)
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

int tegra_prepare_cpu_on_finish(unsigned long mpidr)
{
	/*
	 * Check if we are exiting from SOC_POWERDN.
	 */
	if (tegra_system_suspended()) {

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

int tegra_prepare_cpu_on(unsigned long mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;

	/* Turn on CPU using flow controller or PMC */
	if (cpu_powergate_mask[cpu] == 0) {
		tegra_pmc_cpu_on(cpu);
		cpu_powergate_mask[cpu] = 1;
	} else {
		tegra_fc_cpu_on(cpu);
	}

	return PSCI_E_SUCCESS;
}

int tegra_prepare_cpu_off(unsigned long mpidr)
{
	tegra_fc_cpu_off(mpidr & MPIDR_CPU_MASK);
	return PSCI_E_SUCCESS;
}
