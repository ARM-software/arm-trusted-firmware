/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
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
extern uint64_t tegra_console_base;

/*
 * tegra_fake_system_suspend acts as a boolean var controlling whether
 * we are going to take fake system suspend code or normal system suspend code
 * path. This variable is set inside the sip call handlers,when the kernel
 * requests a SIP call to set the suspend debug flags.
 */
uint8_t tegra_fake_system_suspend;

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that will be overridden by a SoC.
 */
#pragma weak tegra_soc_pwr_domain_suspend_pwrdown_early
#pragma weak tegra_soc_pwr_domain_suspend
#pragma weak tegra_soc_pwr_domain_on
#pragma weak tegra_soc_pwr_domain_off
#pragma weak tegra_soc_pwr_domain_on_finish
#pragma weak tegra_soc_pwr_domain_power_down_wfi
#pragma weak tegra_soc_prepare_system_reset
#pragma weak tegra_soc_prepare_system_off
#pragma weak tegra_soc_get_target_pwr_state

int tegra_soc_pwr_domain_suspend_pwrdown_early(const psci_power_state_t *target_state)
{
	return PSCI_E_NOT_SUPPORTED;
}

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

int tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state)
{
	return PSCI_E_SUCCESS;
}

int tegra_soc_prepare_system_reset(void)
{
	return PSCI_E_SUCCESS;
}

__dead2 void tegra_soc_prepare_system_off(void)
{
	ERROR("Tegra System Off: operation not handled.\n");
	panic();
}

plat_local_state_t tegra_soc_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = PLAT_MAX_OFF_STATE, temp;

	assert(ncpu);

	do {
		temp = *states++;
		if ((temp < target))
			target = temp;
	} while (--ncpu);

	return target;
}

/*******************************************************************************
 * This handler is called by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call to get the `power_state` parameter. This allows the platform to encode
 * the appropriate State-ID field within the `power_state` parameter which can
 * be utilized in `pwr_domain_suspend()` to suspend to system affinity level.
******************************************************************************/
void tegra_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	/* all affinities use system suspend state id */
	for (uint32_t i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PSTATE_ID_SOC_POWERDN;
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
 * Handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 * This handler is called with SMP and data cache enabled, when
 * HW_ASSISTED_COHERENCY = 0
 ******************************************************************************/
void tegra_pwr_domain_suspend_pwrdown_early(const psci_power_state_t *target_state)
{
	tegra_soc_pwr_domain_suspend_pwrdown_early(target_state);
}

/*******************************************************************************
 * Handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void tegra_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	tegra_soc_pwr_domain_suspend(target_state);

	/* Disable console if we are entering deep sleep. */
	if (target_state->pwr_domain_state[PLAT_MAX_PWR_LVL] ==
			PSTATE_ID_SOC_POWERDN)
		console_uninit();

	/* disable GICC */
	tegra_gic_cpuif_deactivate();
}

/*******************************************************************************
 * Handler called at the end of the power domain suspend sequence. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
__dead2 void tegra_pwr_domain_power_down_wfi(const psci_power_state_t
					     *target_state)
{
	uint8_t pwr_state = target_state->pwr_domain_state[PLAT_MAX_PWR_LVL];
	uint64_t rmr_el3 = 0;

	/* call the chip's power down handler */
	tegra_soc_pwr_domain_power_down_wfi(target_state);

	/*
	 * If we are in fake system suspend mode, ensure we start doing
	 * procedures that help in looping back towards system suspend exit
	 * instead of calling WFI by requesting a warm reset.
	 * Else, just call WFI to enter low power state.
	 */
	if ((tegra_fake_system_suspend != 0U) &&
	    (pwr_state == (uint8_t)PSTATE_ID_SOC_POWERDN)) {

		/* warm reboot */
		rmr_el3 = read_rmr_el3();
		write_rmr_el3(rmr_el3 | RMR_WARM_RESET_CPU);

	} else {
		/* enter power down state */
		wfi();
	}

	/* we can never reach here */
	panic();
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
	plat_gic_setup();

	/*
	 * Check if we are exiting from deep sleep.
	 */
	if (target_state->pwr_domain_state[PLAT_MAX_PWR_LVL] ==
			PSTATE_ID_SOC_POWERDN) {

		/* Initialize the runtime console */
		if (tegra_console_base != (uint64_t)0) {
			console_init(tegra_console_base, TEGRA_BOOT_UART_CLK_IN_HZ,
				TEGRA_CONSOLE_BAUDRATE);
		}

		/*
		 * Restore Memory Controller settings as it loses state
		 * during system suspend.
		 */
		tegra_memctrl_restore_settings();

		/*
		 * Security configuration to allow DRAM/device access.
		 */
		plat_params = bl31_get_plat_params();
		tegra_memctrl_tzdram_setup(plat_params->tzdram_base,
			plat_params->tzdram_size);

		/*
		 * Set up the TZRAM memory aperture to allow only secure world
		 * access
		 */
		tegra_memctrl_tzram_setup(TEGRA_TZRAM_BASE, TEGRA_TZRAM_SIZE);
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
	INFO("Powering down system...\n");

	tegra_soc_prepare_system_off();
}

/*******************************************************************************
 * Handler called when the system wants to be restarted.
 ******************************************************************************/
__dead2 void tegra_system_reset(void)
{
	INFO("Restarting system...\n");

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
	assert(req_state);

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
	.pwr_domain_suspend_pwrdown_early = tegra_pwr_domain_suspend_pwrdown_early,
	.pwr_domain_suspend		= tegra_pwr_domain_suspend,
	.pwr_domain_on_finish		= tegra_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= tegra_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi	= tegra_pwr_domain_power_down_wfi,
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

/*******************************************************************************
 * Platform handler to calculate the proper target power level at the
 * specified affinity level
 ******************************************************************************/
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	return tegra_soc_get_target_pwr_state(lvl, states, ncpu);
}
