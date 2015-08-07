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
extern uint64_t tegra_sec_entry_point;

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that will be overridden by a SoC.
 */
#pragma weak tegra_soc_pwr_domain_suspend
#pragma weak tegra_soc_pwr_domain_on
#pragma weak tegra_soc_pwr_domain_off
#pragma weak tegra_soc_pwr_domain_on_finish
#pragma weak tegra_soc_prepare_system_reset

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	return PSCI_E_NOT_SUPPORTED;
}

int tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	return PSCI_E_SUCCESS;
}

int tegra_soc_prepare_system_reset(void)
{
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This handler is called by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call to get the `power_state` parameter. This allows the platform to encode
 * the appropriate State-ID field within the `power_state` parameter which can
 * be utilized in `pwr_domain_suspend()` to suspend to system affinity level.
******************************************************************************/
void tegra_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	/* lower affinities use PLAT_MAX_OFF_STATE */
	for (int i = MPIDR_AFFLVL0; i < PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;

	/* max affinity uses system suspend state id */
	req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] = PSTATE_ID_SOC_POWERDN;
}

/*******************************************************************************
 * Handler called when an affinity instance is about to enter standby.
 ******************************************************************************/
void tegra_cpu_standby(plat_local_state_t cpu_state)
{
	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int tegra_pwr_domain_on(u_register_t mpidr)
{
	return tegra_soc_pwr_domain_on(mpidr);
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void tegra_pwr_domain_off(const psci_power_state_t *target_state)
{
	tegra_soc_pwr_domain_off(target_state);
}

/*******************************************************************************
 * Handler called when called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void tegra_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	tegra_soc_pwr_domain_suspend(target_state);

	/* disable GICC */
	tegra_gic_cpuif_deactivate();
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void tegra_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	plat_params_from_bl2_t *plat_params;

	/*
	 * Initialize the GIC cpu and distributor interfaces
	 */
	tegra_gic_setup();

	/*
	 * Check if we are exiting from deep sleep.
	 */
	if (target_state->pwr_domain_state[PLAT_MAX_PWR_LVL] ==
			PSTATE_ID_SOC_POWERDN) {

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
	tegra_soc_pwr_domain_on_finish(target_state);
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
void tegra_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	tegra_pwr_domain_on_finish(target_state);
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
	/* per-SoC system reset handler */
	tegra_soc_prepare_system_reset();

	/*
	 * Program the PMC in order to restart the system.
	 */
	tegra_pmc_system_reset();
}

/*******************************************************************************
 * Handler called to check the validity of the power state parameter.
 ******************************************************************************/
int32_t tegra_validate_power_state(unsigned int power_state,
				   psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);

	assert(req_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	return tegra_soc_validate_power_state(power_state, req_state);
}

/*******************************************************************************
 * Platform handler called to check the validity of the non secure entrypoint.
 ******************************************************************************/
int tegra_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= TEGRA_DRAM_BASE) && (entrypoint <= TEGRA_DRAM_END))
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_psci_ops_t tegra_plat_psci_ops = {
	.cpu_standby			= tegra_cpu_standby,
	.pwr_domain_on			= tegra_pwr_domain_on,
	.pwr_domain_off			= tegra_pwr_domain_off,
	.pwr_domain_suspend		= tegra_pwr_domain_suspend,
	.pwr_domain_on_finish		= tegra_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= tegra_pwr_domain_suspend_finish,
	.system_off			= tegra_system_off,
	.system_reset			= tegra_system_reset,
	.validate_power_state		= tegra_validate_power_state,
	.validate_ns_entrypoint		= tegra_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= tegra_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops and initialize Power Controller
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	psci_power_state_t target_state = { { PSCI_LOCAL_STATE_RUN } };

	/*
	 * Flush entrypoint variable to PoC since it will be
	 * accessed after a reset with the caches turned off.
	 */
	tegra_sec_entry_point = sec_entrypoint;
	flush_dcache_range((uint64_t)&tegra_sec_entry_point, sizeof(uint64_t));

	/*
	 * Reset hardware settings.
	 */
	tegra_soc_pwr_domain_on_finish(&target_state);

	/*
	 * Initialize PSCI ops struct
	 */
	*psci_ops = &tegra_plat_psci_ops;

	return 0;
}
