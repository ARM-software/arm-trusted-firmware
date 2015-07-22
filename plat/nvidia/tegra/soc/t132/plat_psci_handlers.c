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

int tegra_prepare_cpu_on(unsigned long mpidr)
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

int tegra_prepare_cpu_off(unsigned long mpidr)
{
	tegra_fc_cpu_off(mpidr & MPIDR_CPU_MASK);
	return PSCI_E_SUCCESS;
}

int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl)
{
	/* Nothing to be done for lower affinity levels */
	if (afflvl < MPIDR_AFFLVL2)
		return PSCI_E_SUCCESS;

	/* Support for system suspend only */
	if (id != PLAT_SYS_SUSPEND_STATE_ID) {
		ERROR("%s: state-id not supported (%d)\n", __func__, id);
		return PSCI_E_NOT_SUPPORTED;
	}

	/* Enter system suspend state */
	tegra_pm_system_suspend_entry();

	/* Allow restarting CPU #1 using PMC on suspend exit */
	cpu_powergate_mask[1] = 0;

	/* Program FC to enter suspend state */
	tegra_fc_cpu_idle(read_mpidr());

	/* Suspend DCO operations */
	write_actlr_el1(id);

	return PSCI_E_SUCCESS;
}
