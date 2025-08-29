/*
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/ep_info.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <plat_arm.h>

#include "drivers/delay_timer.h"
#include <plat_private.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include <pm_common.h>
#include "pm_ipi.h"
#include "pm_svc_main.h"

#define SEC_ENTRY_ADDRESS_MASK		0xFFFFFFFFUL
#define RESUME_ADDR_SET			0x1UL

static uintptr_t versal_sec_entry;

static int32_t versal_pwr_domain_on(u_register_t mpidr)
{
	int32_t cpu_id = plat_core_pos_by_mpidr(mpidr);
	const struct pm_proc *proc;
	int32_t ret = PSCI_E_INTERN_FAIL;

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);

	if (cpu_id == -1) {
		goto exit_label;
	}

	proc = pm_get_proc((uint32_t)cpu_id);
	if (proc == NULL) {
		goto exit_label;
	}

	/* Send request to PMC to wake up selected ACPU core */
	(void)pm_req_wakeup(proc->node_id,
			    (uint32_t)((versal_sec_entry & SEC_ENTRY_ADDRESS_MASK) |
			    RESUME_ADDR_SET), versal_sec_entry >> 32, 0, NON_SECURE);

	/* Clear power down request */
	pm_client_wakeup(proc);

	ret = PSCI_E_SUCCESS;

exit_label:
	return ret;
}

/**
 * versal_pwr_domain_suspend() - This function sends request to PMC to suspend
 *                               core.
 * @target_state: Targated state.
 *
 */
static void versal_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint32_t state;
	uint32_t cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	if (proc == NULL) {
		return;
	}

	for (size_t i = 0U; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	plat_versal_gic_cpuif_disable();

	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		plat_versal_gic_save();
	}

	state = (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) ?
		PM_STATE_SUSPEND_TO_RAM : PM_STATE_CPU_IDLE;

	/* Send request to PMC to suspend this core */
	(void)pm_self_suspend(proc->node_id, MAX_LATENCY, state, versal_sec_entry,
			      NON_SECURE);

	/* APU is to be turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		/* disable coherency */
		plat_arm_interconnect_exit_coherency();
	}
}

/**
 * versal_pwr_domain_suspend_finish() - This function performs actions to finish
 *                                      suspend procedure.
 * @target_state: Targated state.
 *
 */
static void versal_pwr_domain_suspend_finish(
					const psci_power_state_t *target_state)
{
	uint32_t cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	if (proc == NULL) {
		return;
	}

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

static void versal_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/*
	 * Typecasting to void to intentionally retain the variable and avoid
	 * MISRA violation for unused parameters. This may be used in the
	 * future if specific action is required based on CPU power state.
	 */
	(void)target_state;

	/* Enable the gic cpu interface */
	plat_versal_gic_pcpu_init();

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_versal_gic_cpuif_enable();
}

/**
 * versal_system_off() - This function sends the system off request to firmware.
 *                       This function does not return.
 *
 */
static void __dead2 versal_system_off(void)
{
	/* Send the power down request to the PMC */
	(void)pm_system_shutdown(XPM_SHUTDOWN_TYPE_SHUTDOWN,
				 pm_get_shutdown_scope(), NON_SECURE);

	while (true) {
		wfi();
	}
}

/**
 * versal_system_reset() - This function sends the reset request to firmware
 *                         for the system to reset.  This function does not
 *			   return.
 *
 */
static void __dead2 versal_system_reset(void)
{
	uint32_t ret, timeout = 10000U;

	request_cpu_pwrdwn();

	/*
	 * Send the system reset request to the firmware if power down request
	 * is not received from firmware.
	 */
	if (!pm_pwrdwn_req_status()) {
		(void)pm_system_shutdown(XPM_SHUTDOWN_TYPE_RESET,
					 pm_get_shutdown_scope(), NON_SECURE);

		/*
		 * Wait for system shutdown request completed and idle callback
		 * not received.
		 */
		do {
			ret = ipi_mb_enquire_status(primary_proc->ipi->local_ipi_id,
						    primary_proc->ipi->remote_ipi_id);
			udelay(100);
			timeout--;
		} while ((ret != IPI_MB_STATUS_RECV_PENDING) && (timeout > 0U));
	}

	(void)psci_cpu_off();

	while (true) {
		wfi();
	}
}

static int32_t versal_validate_ns_entrypoint(uint64_t ns_entrypoint)
{
	int32_t ret = PSCI_E_SUCCESS;

	if (((ns_entrypoint >= PLAT_DDR_LOWMEM_MAX) && (ns_entrypoint <= PLAT_DDR_HIGHMEM_MAX)) ||
		((ns_entrypoint >= BL31_BASE) && (ns_entrypoint <= BL31_LIMIT))) {
		ret = PSCI_E_INVALID_ADDRESS;
	}

	return ret;
}

/**
 * versal_pwr_domain_off() - This function performs actions to turn off core.
 * @target_state: Targated state.
 *
 */
static void versal_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint32_t ret, fw_api_version, version_type[RET_PAYLOAD_ARG_CNT] = {0U};
	uint32_t cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	if (proc == NULL) {
		return;
	}

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
	ret = (uint32_t)pm_feature_check((uint32_t)PM_SELF_SUSPEND,
					 &version_type[0], NON_SECURE);
	if (ret == (uint32_t)PM_RET_SUCCESS) {
		fw_api_version = version_type[0] & 0xFFFFU;
		if (fw_api_version >= 3U) {
			(void)pm_self_suspend(proc->node_id, MAX_LATENCY, PM_STATE_CPU_OFF, 0,
					      NON_SECURE);
		} else {
			(void)pm_self_suspend(proc->node_id, MAX_LATENCY, PM_STATE_CPU_IDLE, 0,
					      NON_SECURE);
		}
	}
}

/**
 * versal_validate_power_state() - This function ensures that the power state
 *                                 parameter in request is valid.
 * @power_state: Power state of core.
 * @req_state: Requested state.
 *
 * Return: Returns status, either success or reason.
 *
 */
static int32_t versal_validate_power_state(uint32_t power_state,
				       psci_power_state_t *req_state)
{
	int32_t ret = PSCI_E_SUCCESS;
	VERBOSE("%s: power_state: 0x%x\n", __func__, power_state);

	uint32_t pstate = psci_get_pstate_type(power_state);

	assert(req_state != NULL);

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_RET_STATE;
	} else {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_OFF_STATE;
	}

	/* We expect the 'state id' to be zero */
	if (psci_get_pstate_id(power_state) != 0U) {
		ret = PSCI_E_INVALID_PARAMS;
	}

	return ret;
}

/**
 * versal_get_sys_suspend_power_state() - Get power state for system suspend.
 * @req_state: Requested state.
 *
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
	.validate_ns_entrypoint		= versal_validate_ns_entrypoint,
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
