/*
 * Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <plat_private.h>
#include "pm_api_sys.h"
#include "pm_client.h"

static uintptr_t zynqmp_sec_entry;

static void zynqmp_cpu_standby(plat_local_state_t cpu_state)
{
	VERBOSE("%s: cpu_state: 0x%x\n", __func__, cpu_state);

	dsb();
	wfi();
}

static int zynqmp_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu_id = plat_core_pos_by_mpidr(mpidr);
	const struct pm_proc *proc;
	uint32_t buff[3];
	enum pm_ret_status ret;

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);

	if (cpu_id == -1) {
		return PSCI_E_INTERN_FAIL;
	}
	proc = pm_get_proc(cpu_id);

	/* Check the APU proc status before wakeup */
	ret = pm_get_node_status(proc->node_id, buff);
	if ((ret != PM_RET_SUCCESS) || (buff[0] == PM_PROC_STATE_SUSPENDING)) {
		return PSCI_E_INTERN_FAIL;
	}

	/* Clear power down request */
	pm_client_wakeup(proc);

	/* Send request to PMU to wake up selected APU CPU core */
	pm_req_wakeup(proc->node_id, 1, zynqmp_sec_entry, REQ_ACK_BLOCKING);

	return PSCI_E_SUCCESS;
}

static void zynqmp_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	/* Prevent interrupts from spuriously waking up this cpu */
	gicv2_cpuif_disable();

	/*
	 * Send request to PMU to power down the appropriate APU CPU
	 * core.
	 * According to PSCI specification, CPU_off function does not
	 * have resume address and CPU core can only be woken up
	 * invoking CPU_on function, during which resume address will
	 * be set.
	 */
	pm_self_suspend(proc->node_id, MAX_LATENCY, PM_STATE_CPU_IDLE, 0);
}

static void zynqmp_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int state;
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	state = target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE ?
		PM_STATE_SUSPEND_TO_RAM : PM_STATE_CPU_IDLE;

	/* Send request to PMU to suspend this core */
	pm_self_suspend(proc->node_id, MAX_LATENCY, state, zynqmp_sec_entry);

	/* APU is to be turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		/* disable coherency */
		plat_arm_interconnect_exit_coherency();
	}
}

static void zynqmp_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}
	plat_arm_gic_pcpu_init();
	gicv2_cpuif_enable();
}

static void zynqmp_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	/* Clear the APU power control register for this cpu */
	pm_client_wakeup(proc);

	/* enable coherency */
	plat_arm_interconnect_enter_coherency();
	/* APU was turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		plat_arm_gic_init();
	} else {
		gicv2_cpuif_enable();
		gicv2_pcpu_distif_init();
	}
}

/*******************************************************************************
 * ZynqMP handlers to shutdown/reboot the system
 ******************************************************************************/

static void __dead2 zynqmp_system_off(void)
{
	/* disable coherency */
	plat_arm_interconnect_exit_coherency();

	/* Send the power down request to the PMU */
	pm_system_shutdown(PMF_SHUTDOWN_TYPE_SHUTDOWN,
			   pm_get_shutdown_scope());

	while (1) {
		wfi();
	}
}

static void __dead2 zynqmp_system_reset(void)
{
	/* disable coherency */
	plat_arm_interconnect_exit_coherency();

	/* Send the system reset request to the PMU */
	pm_system_shutdown(PMF_SHUTDOWN_TYPE_RESET,
			   pm_get_shutdown_scope());

	while (1) {
		wfi();
	}
}

static int zynqmp_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	VERBOSE("%s: power_state: 0x%x\n", __func__, power_state);

	int pstate = psci_get_pstate_type(power_state);

	assert(req_state);

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_RET_STATE;
	} else {
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_OFF_STATE;
	}
	/* We expect the 'state id' to be zero */
	if (psci_get_pstate_id(power_state)) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

static void zynqmp_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	req_state->pwr_domain_state[PSCI_CPU_PWR_LVL] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[1] = PLAT_MAX_OFF_STATE;
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const struct plat_psci_ops zynqmp_psci_ops = {
	.cpu_standby			= zynqmp_cpu_standby,
	.pwr_domain_on			= zynqmp_pwr_domain_on,
	.pwr_domain_off			= zynqmp_pwr_domain_off,
	.pwr_domain_suspend		= zynqmp_pwr_domain_suspend,
	.pwr_domain_on_finish		= zynqmp_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= zynqmp_pwr_domain_suspend_finish,
	.system_off			= zynqmp_system_off,
	.system_reset			= zynqmp_system_reset,
	.validate_power_state		= zynqmp_validate_power_state,
	.get_sys_suspend_power_state	= zynqmp_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	zynqmp_sec_entry = sec_entrypoint;

	*psci_ops = &zynqmp_psci_ops;

	return 0;
}
