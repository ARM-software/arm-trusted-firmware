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
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <memctrl.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <pmc.h>
#include <psci.h>
#include <tegra_def.h>
#include <tegra_private.h>

extern uint64_t tegra_bl31_phys_base;
extern uint64_t sec_entry_point[PLATFORM_CORE_COUNT];
static int system_suspended;

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that will be overridden by a SoC.
 */
#pragma weak tegra_prepare_cpu_suspend
#pragma weak tegra_prepare_cpu_on
#pragma weak tegra_prepare_cpu_off
#pragma weak tegra_prepare_cpu_on_finish

int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl)
{
	return PSCI_E_NOT_SUPPORTED;
}

int tegra_prepare_cpu_on(unsigned long mpidr)
{
	return PSCI_E_SUCCESS;
}

int tegra_prepare_cpu_off(unsigned long mpidr)
{
	return PSCI_E_SUCCESS;
}

int tegra_prepare_cpu_on_finish(unsigned long mpidr)
{
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Track system suspend entry.
 ******************************************************************************/
void tegra_pm_system_suspend_entry(void)
{
	system_suspended = 1;
}

/*******************************************************************************
 * Track system suspend exit.
 ******************************************************************************/
void tegra_pm_system_suspend_exit(void)
{
	system_suspended = 0;
}

/*******************************************************************************
 * Get the system suspend state.
 ******************************************************************************/
int tegra_system_suspended(void)
{
	return system_suspended;
}

/*******************************************************************************
 * Handler called when an affinity instance is about to enter standby.
 ******************************************************************************/
void tegra_affinst_standby(unsigned int power_state)
{
	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

/*******************************************************************************
 * Handler called to check the validity of the power state parameter.
 ******************************************************************************/
int32_t tegra_validate_power_state(unsigned int power_state)
{
	/* Sanity check the requested state */
	if (psci_get_pstate_type(power_state) == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on affinity level 0 i.e.
		 * a cpu on Tegra. Ignore any other affinity level.
		 */
		if (psci_get_pstate_afflvl(power_state) != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int tegra_affinst_on(unsigned long mpidr,
		   unsigned long sec_entrypoint,
		   unsigned int afflvl,
		   unsigned int state)
{
	int cpu = mpidr & MPIDR_CPU_MASK;

	/*
	 * Support individual CPU power on only.
	 */
	if (afflvl > MPIDR_AFFLVL0)
		return PSCI_E_SUCCESS;

	/*
	 * Flush entrypoint variable to PoC since it will be
	 * accessed after a reset with the caches turned off.
	 */
	sec_entry_point[cpu] = sec_entrypoint;
	flush_dcache_range((uint64_t)&sec_entry_point[cpu], sizeof(uint64_t));

	return tegra_prepare_cpu_on(mpidr);
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be turned off. The
 * level determines the affinity instance. The 'state' arg. allows the
 * platform to decide whether the cluster is being turned off and take apt
 * actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
void tegra_affinst_off(unsigned int afflvl, unsigned int state)
{
	/*
	 * Support individual CPU power off only.
	 */
	if (afflvl > MPIDR_AFFLVL0)
		return;

	tegra_prepare_cpu_off(read_mpidr());
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be suspended. The
 * level and mpidr determine the affinity instance. The 'state' arg. allows the
 * platform to decide whether the cluster is being turned off and take apt
 * actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to flush a write to the global variable, to prevent unpredictable
 * results.
 ******************************************************************************/
void tegra_affinst_suspend(unsigned long sec_entrypoint,
			unsigned int afflvl,
			unsigned int state)
{
	int id = psci_get_suspend_stateid();
	int cpu = read_mpidr() & MPIDR_CPU_MASK;

	if (afflvl > PLATFORM_MAX_AFFLVL)
		return;

	/*
	 * Flush entrypoint variable to PoC since it will be
	 * accessed after a reset with the caches turned off.
	 */
	sec_entry_point[cpu] = sec_entrypoint;
	flush_dcache_range((uint64_t)&sec_entry_point[cpu], sizeof(uint64_t));

	tegra_prepare_cpu_suspend(id, afflvl);

	/* disable GICC */
	tegra_gic_cpuif_deactivate();
}

/*******************************************************************************
 * Handler called when an affinity instance has just been powered on after
 * being turned off earlier. The level determines the affinity instance.
 * The 'state' arg. allows the platform to decide whether the cluster was
 * turned off prior to wakeup and do what's necessary to set it up.
 ******************************************************************************/
void tegra_affinst_on_finish(unsigned int afflvl, unsigned int state)
{
	plat_params_from_bl2_t *plat_params;

	/*
	 * Support individual CPU power on only.
	 */
	if (afflvl > MPIDR_AFFLVL0)
		return;

	/*
	 * Initialize the GIC cpu and distributor interfaces
	 */
	tegra_gic_setup();

	/*
	 * Check if we are exiting from deep sleep.
	 */
	if (tegra_system_suspended()) {

		/*
		 * Lock scratch registers which hold the CPU vectors.
		 */
		tegra_pmc_lock_cpu_vectors();

		/*
		 * SMMU configuration.
		 */
		tegra_memctrl_setup();

		/*
		 * Security configuration to allow DRAM/device access.
		 */
		plat_params = bl31_get_plat_params();
		tegra_memctrl_tzdram_setup(tegra_bl31_phys_base,
			plat_params->tzdram_size);
	}

	/*
	 * Reset hardware settings.
	 */
	tegra_prepare_cpu_on_finish(read_mpidr());
}

/*******************************************************************************
 * Handler called when an affinity instance has just been powered on after
 * having been suspended earlier. The level and mpidr determine the affinity
 * instance.
 ******************************************************************************/
void tegra_affinst_suspend_finish(unsigned int afflvl, unsigned int state)
{
	if (afflvl == MPIDR_AFFLVL0)
		tegra_affinst_on_finish(afflvl, state);
}

/*******************************************************************************
 * Handler called when the system wants to be powered off
 ******************************************************************************/
__dead2 void tegra_system_off(void)
{
	ERROR("Tegra System Off: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * Handler called when the system wants to be restarted.
 ******************************************************************************/
__dead2 void tegra_system_reset(void)
{
	/*
	 * Program the PMC in order to restart the system.
	 */
	tegra_pmc_system_reset();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t tegra_plat_pm_ops = {
	.affinst_standby	= tegra_affinst_standby,
	.affinst_on		= tegra_affinst_on,
	.affinst_off		= tegra_affinst_off,
	.affinst_suspend	= tegra_affinst_suspend,
	.affinst_on_finish	= tegra_affinst_on_finish,
	.affinst_suspend_finish	= tegra_affinst_suspend_finish,
	.system_off		= tegra_system_off,
	.system_reset		= tegra_system_reset,
	.validate_power_state	= tegra_validate_power_state
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the fvp power controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	/*
	 * Reset hardware settings.
	 */
	tegra_prepare_cpu_on_finish(read_mpidr());

	/*
	 * Initialize PM ops struct
	 */
	*plat_ops = &tegra_plat_pm_ops;

	return 0;
}
