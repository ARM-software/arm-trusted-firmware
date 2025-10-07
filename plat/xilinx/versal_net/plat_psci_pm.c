/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
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

#include <drivers/delay_timer.h>
#include <plat_private.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include <pm_common.h>
#include "pm_ipi.h"
#include "pm_svc_main.h"
#include "versal_net_def.h"

static uintptr_t versal_net_sec_entry;

static int32_t versal_net_pwr_domain_on(u_register_t mpidr)
{
	int32_t cpu_id = plat_core_pos_by_mpidr(mpidr);
	const struct pm_proc *proc;
	int32_t ret = PSCI_E_INTERN_FAIL;

	VERBOSE("%s: mpidr: 0x%lx, cpuid: %x\n",
		__func__, mpidr, cpu_id);

	if (cpu_id == -1) {
		goto exit_label;
	}

	proc = pm_get_proc(cpu_id);
	if (proc == NULL) {
		goto exit_label;
	}

	(void)pm_req_wakeup(proc->node_id, (versal_net_sec_entry & 0xFFFFFFFFU) | 0x1U,
		      versal_net_sec_entry >> 32, 0, 0);

	/* Clear power down request */
	pm_client_wakeup(proc);

	ret = PSCI_E_SUCCESS;

exit_label:
	return ret;
}

/**
 * versal_net_pwr_domain_off() - This function performs actions to turn off
 *                               core.
 * @target_state: Targeted state.
 *
 */
static void versal_net_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint32_t ret, fw_api_version, version_type[RET_PAYLOAD_ARG_CNT] = {0U};
	uint32_t cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	if (proc == NULL) {
		goto exit_label;
	}

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	/*
	 * Send request to PMC to power down the appropriate APU CPU
	 * core.
	 * According to PSCI specification, CPU_off function does not
	 * have resume address and CPU core can only be woken up
	 * invoking CPU_on function, during which resume address will
	 * be set.
	 */
	ret = pm_feature_check((uint32_t)PM_SELF_SUSPEND, &version_type[0], NON_SECURE);
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

exit_label:
	return;
}

static int32_t versal_net_validate_ns_entrypoint(uint64_t ns_entrypoint)
{
	int32_t ret = PSCI_E_SUCCESS;

	if (((ns_entrypoint >= PLAT_DDR_LOWMEM_MAX) && (ns_entrypoint <= PLAT_DDR_HIGHMEM_MAX)) ||
		((ns_entrypoint >= BL31_BASE) && (ns_entrypoint <= BL31_LIMIT))) {
		ret = PSCI_E_INVALID_ADDRESS;
	}

	return ret;
}

/**
 * versal_net_system_reset_scope() - Sends the reset request to firmware for
 * the system to reset.
 * @scope : scope of reset which could be SYSTEM/SUBSYSTEM/PS-ONLY
 *
 * Return:
 *     Does not return if system resets, none if there is a failure.
 */
static void __dead2 versal_net_system_reset_scope(uint32_t scope)
{
	uint32_t ret, timeout = 10000U;

	request_cpu_pwrdwn();

	/*
	 * Send the system reset request to the firmware if power down request
	 * is not received from firmware.
	 */
	if (!pm_pwrdwn_req_status()) {
		(void)pm_system_shutdown(XPM_SHUTDOWN_TYPE_RESET,
					 scope, NON_SECURE);

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

/**
 * versal_net_system_reset() - This function sends the reset request to firmware
 * for the system to reset in response to SYSTEM_RESET call
 *
 * Return:
 *     Does not return if system resets, none if there is a failure.
 */
static void __dead2 versal_net_system_reset(void)
{
	/*
	 * Any platform-specific actions for handling a cold reset
	 * should be performed here before invoking
	 * versal_net_system_reset_scope.
	 */
	versal_net_system_reset_scope(XPM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM);
}

/**
 * versal_net_system_reset2() - Handles warm / vendor-specific system reset
 * in response to SYSTEM_RESET2 call.
 * @is_vendor: Flag indicating if this is a vendor-specific reset
 * @reset_type: Type of reset requested
 * @cookie: Additional reset data
 *
 * This function initiates a controlled system reset by requesting it
 * through the PM firmware.
 *
 * Return:
 *	Does not return if system resets, PSCI_E_INTERN_FAIL
 *	if there is a failure.
 */
static int versal_net_system_reset2(int is_vendor, int reset_type, u_register_t cookie)
{
	if (is_vendor == 0 && reset_type == PSCI_RESET2_SYSTEM_WARM_RESET) {
		/*
		 * Any platform-specific actions for handling a warm reset
		 * should be performed here before invoking
		 * versal_net_system_reset_scope.
		 */
		versal_net_system_reset_scope(XPM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM);
	} else {
		/* Vendor specific reset */
		versal_net_system_reset_scope(pm_get_shutdown_scope());
	}

	return PSCI_E_INTERN_FAIL;
}

/**
 * versal_net_pwr_domain_suspend() - This function sends request to PMC to suspend
 *                                   core.
 * @target_state: Targeted state.
 *
 */
static void versal_net_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint32_t state;
	uint32_t cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	if (proc == NULL) {
		goto exit_label;
	}

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		gic_save();
	}

	state = (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) ?
		PM_STATE_SUSPEND_TO_RAM : PM_STATE_CPU_IDLE;

	/* Send request to PMC to suspend this core */
	(void)pm_self_suspend(proc->node_id, MAX_LATENCY, state, versal_net_sec_entry,
			      NON_SECURE);

	/* TODO: disable coherency */

exit_label:
	return;
}

static void versal_net_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	(void)target_state;
}

/**
 * versal_net_pwr_domain_suspend_finish() - This function performs actions to finish
 *                                          suspend procedure.
 * @target_state: Targeted state.
 *
 */
static void versal_net_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint32_t cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	if (proc == NULL) {
		goto exit_label;
	}

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	/* Clear the APU power control register for this cpu */
	pm_client_wakeup(proc);

	/* TODO: enable coherency */

	/* APU was turned off, so restore GIC context */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		gic_resume();
	}

exit_label:
	return;
}

/**
 * versal_net_system_off() - This function sends the system off request
 *                           to firmware. This function does not return.
 *
 */
static void __dead2 versal_net_system_off(void)
{
	/* Send the power down request to the PMC */
	(void)pm_system_shutdown(XPM_SHUTDOWN_TYPE_SHUTDOWN,
				 pm_get_shutdown_scope(), NON_SECURE);

	while (true) {
		wfi();
	}
}

/**
 * versal_net_validate_power_state() - This function ensures that the power state
 *                                     parameter in request is valid.
 * @power_state: Power state of core.
 * @req_state: Requested state.
 *
 * Return: Returns status, either PSCI_E_SUCCESS or reason.
 *
 */
static int32_t versal_net_validate_power_state(unsigned int power_state,
					       psci_power_state_t *req_state)
{
	int32_t ret = PSCI_E_INVALID_PARAMS;

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
	if (psci_get_pstate_id(power_state) == 0U) {
		ret = PSCI_E_SUCCESS;
	}

	return ret;
}

/**
 * versal_net_get_sys_suspend_power_state() - Get power state for system
 *                                            suspend.
 * @req_state: Requested state.
 *
 */
static void versal_net_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	uint64_t i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}

static const struct plat_psci_ops versal_net_nopmc_psci_ops = {
	.pwr_domain_on                  = versal_net_pwr_domain_on,
	.pwr_domain_off                 = versal_net_pwr_domain_off,
	.pwr_domain_on_finish           = versal_net_pwr_domain_on_finish,
	.pwr_domain_suspend             = versal_net_pwr_domain_suspend,
	.pwr_domain_suspend_finish      = versal_net_pwr_domain_suspend_finish,
	.system_off                     = versal_net_system_off,
	.system_reset                   = versal_net_system_reset,
	.system_reset2                  = versal_net_system_reset2,
	.validate_ns_entrypoint		= versal_net_validate_ns_entrypoint,
	.validate_power_state           = versal_net_validate_power_state,
	.get_sys_suspend_power_state    = versal_net_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int32_t plat_setup_psci_ops(uintptr_t sec_entrypoint,
			    const struct plat_psci_ops **psci_ops)
{
	versal_net_sec_entry = sec_entrypoint;

	VERBOSE("Setting up entry point %lx\n", versal_net_sec_entry);

	*psci_ops = &versal_net_nopmc_psci_ops;

	return 0;
}

int32_t sip_svc_setup_init(void)
{
	return pm_setup();
}

uint64_t smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
		     void *cookie, void *handle, uint64_t flags)
{
	return pm_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
}
