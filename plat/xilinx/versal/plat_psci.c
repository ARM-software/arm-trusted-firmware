/*
 * Copyright (c) 2018-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <plat_arm.h>
#include <plat_private.h>
#include <pm_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>
#include <plat/arm/common/plat_arm.h>

#include "pm_api_sys.h"
#include "pm_client.h"

static uintptr_t versal_sec_entry;

static int versal_pwr_domain_on(u_register_t mpidr)
{
	int cpu_id = plat_core_pos_by_mpidr(mpidr);
	const struct pm_proc *proc;

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);

	if (cpu_id == -1) {
		return PSCI_E_INTERN_FAIL;
	}

	proc = pm_get_proc((unsigned int)cpu_id);

	/* Send request to PMC to wake up selected ACPU core */
	(void)pm_req_wakeup(proc->node_id, (versal_sec_entry & 0xFFFFFFFFU) | 0x1U,
			    versal_sec_entry >> 32, 0, SECURE_FLAG);

	/* Clear power down request */
	pm_client_wakeup(proc);

	return PSCI_E_SUCCESS;
}

/**
 * versal_pwr_domain_suspend() - This function sends request to PMC to suspend
 * core.
 *
 * @target_state	Targated state
 */
static void versal_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int state;
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0U; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	plat_versal_gic_cpuif_disable();

	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		plat_versal_gic_save();
	}

	state = target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE ?
		PM_STATE_SUSPEND_TO_RAM : PM_STATE_CPU_IDLE;

	/* Send request to PMC to suspend this core */
	(void)pm_self_suspend(proc->node_id, MAX_LATENCY, state, versal_sec_entry,
			      SECURE_FLAG);

	/* APU is to be turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		/* disable coherency */
		plat_arm_interconnect_exit_coherency();
	}
}

/**
 * versal_pwr_domain_suspend_finish() - This function performs actions to finish
 * suspend procedure.
 *
 * @target_state	Targated state
 */
static void versal_pwr_domain_suspend_finish(
					const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0U; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	/* Clear the APU power control register for this cpu */
	pm_client_wakeup(proc);

	/* enable coherency */
	plat_arm_interconnect_enter_coherency();

	/* APU was turned off, so restore GIC context */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		plat_versal_gic_resume();
	}

	plat_versal_gic_cpuif_enable();
}

void versal_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* Enable the gic cpu interface */
	plat_versal_gic_pcpu_init();

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_versal_gic_cpuif_enable();
}

/**
 * versal_system_off() - This function sends the system off request
 * to firmware.  This function does not return.
 */
static void __dead2 versal_system_off(void)
{
	/* Send the power down request to the PMC */
	(void)pm_system_shutdown(XPM_SHUTDOWN_TYPE_SHUTDOWN,
				 pm_get_shutdown_scope(), SECURE_FLAG);

	while (1) {
		wfi();
	}
}

/**
 * versal_system_reset() - This function sends the reset request
 * to firmware for the system to reset.  This function does not return.
 */
static void __dead2 versal_system_reset(void)
{
	/* Send the system reset request to the PMC */
	(void)pm_system_shutdown(XPM_SHUTDOWN_TYPE_RESET,
				 pm_get_shutdown_scope(), SECURE_FLAG);

	while (1) {
		wfi();
	}
}

/**
 * versal_pwr_domain_off() - This function performs actions to turn off core
 *
 * @target_state	Targated state
 */
static void versal_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0U; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_versal_gic_cpuif_disable();

	/*
	 * Send request to PMC to power down the appropriate APU CPU
	 * core.
	 * According to PSCI specification, CPU_off function does not
	 * have resume address and CPU core can only be woken up
	 * invoking CPU_on function, during which resume address will
	 * be set.
	 */
	(void)pm_self_suspend(proc->node_id, MAX_LATENCY, PM_STATE_CPU_IDLE, 0,
			      SECURE_FLAG);
}

/**
 * versal_validate_power_state() - This function ensures that the power state
 * parameter in request is valid.
 *
 * @power_state		Power state of core
 * @req_state		Requested state
 *
 * @return	Returns status, either success or reason
 */
static int versal_validate_power_state(unsigned int power_state,
				       psci_power_state_t *req_state)
{
	VERBOSE("%s: power_state: 0x%x\n", __func__, power_state);

	unsigned int pstate = psci_get_pstate_type(power_state);

	assert(req_state);

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_RET_STATE;
	} else {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_OFF_STATE;
	}

	/* We expect the 'state id' to be zero */
	if (psci_get_pstate_id(power_state) != 0U) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

/**
 * versal_get_sys_suspend_power_state() -  Get power state for system suspend
 *
 * @req_state	Requested state
 */
static void versal_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	req_state->pwr_domain_state[PSCI_CPU_PWR_LVL] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[1] = PLAT_MAX_OFF_STATE;
}

static const struct plat_psci_ops versal_nopmc_psci_ops = {
	.pwr_domain_on			= versal_pwr_domain_on,
	.pwr_domain_off			= versal_pwr_domain_off,
	.pwr_domain_on_finish		= versal_pwr_domain_on_finish,
	.pwr_domain_suspend		= versal_pwr_domain_suspend,
	.pwr_domain_suspend_finish	= versal_pwr_domain_suspend_finish,
	.system_off			= versal_system_off,
	.system_reset			= versal_system_reset,
	.validate_power_state		= versal_validate_power_state,
	.get_sys_suspend_power_state	= versal_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	versal_sec_entry = sec_entrypoint;

	*psci_ops = &versal_nopmc_psci_ops;

	return 0;
}
