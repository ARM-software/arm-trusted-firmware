/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/ep_info.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <plat_arm.h>
#include <plat_fdt.h>

#include "def.h"
#include <ipi.h>
#include <plat_private.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include <pm_common.h>
#include "pm_defs.h"
#include "pm_svc_main.h"

static uintptr_t sec_entry;

/* 1 sec of wait timeout for receiving idle callback */
#define IDLE_CB_WAIT_TIMEOUT	(1000000U)

static int32_t versal2_pwr_domain_on(u_register_t mpidr)
{
	int32_t cpu_id = plat_core_pos_by_mpidr(mpidr);
	int32_t ret = (int32_t) PSCI_E_INTERN_FAIL;
	enum pm_ret_status pm_ret;
	const struct pm_proc *proc;

	if (cpu_id != -1) {
		proc = pm_get_proc((uint32_t)cpu_id);
		if (proc != NULL) {
			pm_ret = pm_req_wakeup(proc->node_id,
					       (uint32_t)
					       ((sec_entry & 0xFFFFFFFFU) | 0x1U),
					       sec_entry >> 32, 0, 0);

			if (pm_ret == PM_RET_SUCCESS) {
				/* Clear power down request */
				pm_client_wakeup(proc);
				ret = (int32_t) PSCI_E_SUCCESS;
			}
		}
	}

	return ret;
}

/**
 * versal2_pwr_domain_off() - Turn off core.
 * @target_state: Targeted state.
 */
static void versal2_pwr_domain_off(const psci_power_state_t *target_state)
{
	const struct pm_proc *proc;
	uint32_t cpu_id = plat_my_core_pos();
	enum pm_ret_status pm_ret;
	size_t i;

	proc = pm_get_proc(cpu_id);
	if (proc == NULL) {
		ERROR("Failed to get proc %d\n", cpu_id);
		goto err;
	}

	for (i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	plat_gic_cpuif_disable();
	/*
	 * Send request to PMC to power down the appropriate APU CPU
	 * core.
	 * According to PSCI specification, CPU_off function does not
	 * have resume address and CPU core can only be woken up
	 * invoking CPU_on function, during which resume address will
	 * be set.
	 */
	pm_ret = pm_self_suspend(proc->node_id, MAX_LATENCY, PM_STATE_CPU_OFF, 0,
				 NON_SECURE);

	if (pm_ret != PM_RET_SUCCESS) {
		ERROR("Failed to power down CPU %d\n", cpu_id);
	}
err:
	return;
}

/**
 * versal2_system_reset() - Send the reset request to firmware for the
 *                          system to reset. This function does not
 *                          return as it resets system.
 */
static void __dead2 versal2_system_reset(void)
{
	uint32_t timeout = 10000U;
	enum pm_ret_status pm_ret;
	int32_t ret;

	request_cpu_pwrdwn();

	/*
	 * Send the system reset request to the firmware if power down request
	 * is not received from firmware.
	 */
	if (pm_pwrdwn_req_status() == false) {
		/*
		 * TODO: shutdown scope for this reset needs be revised once
		 * we have a clearer understanding of the overall reset scoping
		 * including the implementation of SYSTEM_RESET2.
		 */
		pm_ret = pm_system_shutdown(XPM_SHUTDOWN_TYPE_RESET,
					 pm_get_shutdown_scope(), NON_SECURE);

		if (pm_ret != PM_RET_SUCCESS) {
			WARN("System shutdown failed\n");
		}

		/*
		 * Wait for system shutdown request completed and idle callback
		 * not received.
		 */
		do {
			ret = ipi_mb_enquire_status(primary_proc->ipi->local_ipi_id,
						    primary_proc->ipi->remote_ipi_id);
			udelay(100);
			timeout--;
		} while ((ret != (int32_t)IPI_MB_STATUS_RECV_PENDING) && (timeout > 0U));
	}

	(void)psci_cpu_off();

	while (true) {
		wfi();
	}
}

/**
 * versal2_pwr_domain_suspend() - Send request to PMC to suspend core.
 * @target_state: Targeted state.
 */
static void versal2_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	const struct pm_proc *proc;
	uint32_t cpu_id = plat_my_core_pos();
	uint32_t state;
	enum pm_ret_status ret;
	size_t i;

	proc = pm_get_proc(cpu_id);
	if (proc == NULL) {
		ERROR("Failed to get proc %d\n", cpu_id);
		goto err;
	}

	for (i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	plat_gic_cpuif_disable();

	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		plat_gic_save();
	}

	state = (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) ?
		PM_STATE_SUSPEND_TO_RAM : PM_STATE_CPU_IDLE;

	/* Send request to PMC to suspend this core */
	ret = pm_self_suspend(proc->node_id, MAX_LATENCY, state, sec_entry,
			      NON_SECURE);

	if (ret != PM_RET_SUCCESS) {
		ERROR("Failed to power down CPU %d\n", cpu_id);
	}

err:
	return;
}

static int32_t versal2_validate_ns_entrypoint(uint64_t ns_entrypoint)
{
	int32_t ret = PSCI_E_SUCCESS;
	struct reserve_mem_range *rmr;
	uint32_t index = 0, counter = 0;

	rmr = get_reserved_entries_fdt(&counter);

	VERBOSE("Validate ns_entry point %lx\n", ns_entrypoint);

	if (counter != 0) {
		while (index < counter) {
			if ((ns_entrypoint >= rmr[index].base) &&
				       (ns_entrypoint <= rmr[index].size)) {
				ret = PSCI_E_INVALID_ADDRESS;
				break;
			}
			index++;
		}
	} else {
		if ((ns_entrypoint >= BL31_BASE) && (ns_entrypoint <= BL31_LIMIT)) {
			ret = PSCI_E_INVALID_ADDRESS;
		}
	}

	return ret;
}

static void versal2_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	(void)target_state;

	/* Enable the gic cpu interface */
	plat_gic_pcpu_init();

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_gic_cpuif_enable();
}

/**
 * versal2_pwr_domain_suspend_finish() - Performs actions to finish
 *                                       suspend procedure.
 * @target_state: Targeted state.
 */
static void versal2_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	const struct pm_proc *proc;
	uint32_t cpu_id = plat_my_core_pos();
	size_t i;

	proc = pm_get_proc(cpu_id);
	if (proc == NULL) {
		ERROR("Failed to get proc %d\n", cpu_id);
		goto err;
	}

	for (i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	/* Clear the APU power control register for this cpu */
	pm_client_wakeup(proc);

	/* APU was turned off, so restore GIC context */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		plat_gic_resume();
	}

	plat_gic_cpuif_enable();

err:
	return;
}

/**
 * versal2_system_off() - Send the system off request to firmware.
 *                        This function does not return as it puts core into WFI
 */
static void __dead2 versal2_system_off(void)
{
	uint64_t timeout;
	enum pm_ret_status ret;

	request_cpu_pwrdwn();

	/* Send the power down request to the PMC */
	ret = pm_system_shutdown(XPM_SHUTDOWN_TYPE_SHUTDOWN,
				 pm_get_shutdown_scope(), NON_SECURE);

	if (ret != PM_RET_SUCCESS) {
		ERROR("System shutdown failed\n");
	}

	/*
	 * Wait for system shutdown request completed and idle callback
	 * not received.
	 */
	timeout = timeout_init_us(IDLE_CB_WAIT_TIMEOUT);
	do {
		ret = ipi_mb_enquire_status(primary_proc->ipi->local_ipi_id,
					    primary_proc->ipi->remote_ipi_id);
		udelay(100);
	} while ((ret != (int32_t)IPI_MB_STATUS_RECV_PENDING) && !timeout_elapsed(timeout));

	(void)psci_cpu_off();

	while (true) {
		wfi();
	}
}

/**
 * versal2_validate_power_state() - Ensure that the power state
 *                                  parameter in request is valid.
 * @power_state: Power state of core.
 * @req_state: Requested state.
 *
 * Return: Returns status, either PSCI_E_SUCCESS or reason.
 */
static int32_t versal2_validate_power_state(unsigned int power_state,
					       psci_power_state_t *req_state)
{
	uint32_t pstate = psci_get_pstate_type(power_state);
	int32_t ret = PSCI_E_SUCCESS;

	VERBOSE("%s: power_state: 0x%x\n", __func__, power_state);

	assert(req_state);

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_RET_STATE;
	} else {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_OFF_STATE;
	}

	/* The 'state_id' is expected to be zero */
	if (psci_get_pstate_id(power_state) != 0U) {
		ret = PSCI_E_INVALID_PARAMS;
	}

	return ret;
}

/**
 * versal2_get_sys_suspend_power_state() - Get power state for system
 *                                            suspend.
 * @req_state: Requested state.
 */
static void versal2_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	uint64_t i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}

/**
 * Export the platform specific power ops.
 */
static const struct plat_psci_ops versal2_nopmc_psci_ops = {
	.pwr_domain_on                  = versal2_pwr_domain_on,
	.pwr_domain_off                 = versal2_pwr_domain_off,
	.pwr_domain_on_finish           = versal2_pwr_domain_on_finish,
	.pwr_domain_suspend             = versal2_pwr_domain_suspend,
	.pwr_domain_suspend_finish      = versal2_pwr_domain_suspend_finish,
	.system_off                     = versal2_system_off,
	.system_reset                   = versal2_system_reset,
	.validate_ns_entrypoint		= versal2_validate_ns_entrypoint,
	.validate_power_state           = versal2_validate_power_state,
	.get_sys_suspend_power_state    = versal2_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			    const struct plat_psci_ops **psci_ops)
{
	sec_entry = sec_entrypoint;

	VERBOSE("Setting up entry point %lx\n", sec_entry);

	*psci_ops = &versal2_nopmc_psci_ops;

	return 0;
}

int32_t sip_svc_setup_init(void)
{
	return pm_setup();
}

uint64_t smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4,
		     const void *cookie, void *handle, uint64_t flags)
{
	return pm_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
}
