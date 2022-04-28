/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <ti_sci_protocol.h>
#include <k3_gicv3.h>
#include <ti_sci.h>

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

uintptr_t k3_sec_entrypoint;

static void k3_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr;

	scr = read_scr_el3();
	/* Enable the Non secure interrupt to wake the CPU */
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);
	isb();
	/* dsb is good practice before using wfi to enter low power states */
	dsb();
	/* Enter standby state */
	wfi();
	/* Restore SCR */
	write_scr_el3(scr);
}

static int k3_pwr_domain_on(u_register_t mpidr)
{
	int core, proc_id, device_id, ret;

	core = plat_core_pos_by_mpidr(mpidr);
	if (core < 0) {
		ERROR("Could not get target core id: %d\n", core);
		return PSCI_E_INTERN_FAIL;
	}

	proc_id = PLAT_PROC_START_ID + core;
	device_id = PLAT_PROC_DEVICE_START_ID + core;

	ret = ti_sci_proc_request(proc_id);
	if (ret) {
		ERROR("Request for processor failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	ret = ti_sci_proc_set_boot_cfg(proc_id, k3_sec_entrypoint, 0, 0);
	if (ret) {
		ERROR("Request to set core boot address failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	/* sanity check these are off before starting a core */
	ret = ti_sci_proc_set_boot_ctrl(proc_id,
			0, PROC_BOOT_CTRL_FLAG_ARMV8_L2FLUSHREQ |
			   PROC_BOOT_CTRL_FLAG_ARMV8_AINACTS |
			   PROC_BOOT_CTRL_FLAG_ARMV8_ACINACTM);
	if (ret) {
		ERROR("Request to clear boot configuration failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	ret = ti_sci_device_get(device_id);
	if (ret) {
		ERROR("Request to start core failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	return PSCI_E_SUCCESS;
}

void k3_pwr_domain_off(const psci_power_state_t *target_state)
{
	int core, cluster, proc_id, device_id, cluster_id, ret;

	/* At very least the local core should be powering down */
	assert(CORE_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE);

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();

	core = plat_my_core_pos();
	cluster = MPIDR_AFFLVL1_VAL(read_mpidr_el1());
	proc_id = PLAT_PROC_START_ID + core;
	device_id = PLAT_PROC_DEVICE_START_ID + core;
	cluster_id = PLAT_CLUSTER_DEVICE_START_ID + (cluster * 2);

	/*
	 * If we are the last core in the cluster then we take a reference to
	 * the cluster device so that it does not get shutdown before we
	 * execute the entire cluster L2 cleaning sequence below.
	 */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		ret = ti_sci_device_get(cluster_id);
		if (ret) {
			ERROR("Request to get cluster failed: %d\n", ret);
			return;
		}
	}

	/* Start by sending wait for WFI command */
	ret = ti_sci_proc_wait_boot_status_no_wait(proc_id,
			/*
			 * Wait maximum time to give us the best chance to get
			 * to WFI before this command timeouts
			 */
			UINT8_MAX, 100, UINT8_MAX, UINT8_MAX,
			/* Wait for WFI */
			PROC_BOOT_STATUS_FLAG_ARMV8_WFI, 0, 0, 0);
	if (ret) {
		ERROR("Sending wait for WFI failed (%d)\n", ret);
		return;
	}

	/* Now queue up the core shutdown request */
	ret = ti_sci_device_put_no_wait(device_id);
	if (ret) {
		ERROR("Sending core shutdown message failed (%d)\n", ret);
		return;
	}

	/* If our cluster is not going down we stop here */
	if (CLUSTER_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	/* set AINACTS */
	ret = ti_sci_proc_set_boot_ctrl_no_wait(proc_id,
			PROC_BOOT_CTRL_FLAG_ARMV8_AINACTS, 0);
	if (ret) {
		ERROR("Sending set control message failed (%d)\n", ret);
		return;
	}

	/* set L2FLUSHREQ */
	ret = ti_sci_proc_set_boot_ctrl_no_wait(proc_id,
			PROC_BOOT_CTRL_FLAG_ARMV8_L2FLUSHREQ, 0);
	if (ret) {
		ERROR("Sending set control message failed (%d)\n", ret);
		return;
	}

	/* wait for L2FLUSHDONE*/
	ret = ti_sci_proc_wait_boot_status_no_wait(proc_id,
			UINT8_MAX, 2, UINT8_MAX, UINT8_MAX,
			PROC_BOOT_STATUS_FLAG_ARMV8_L2F_DONE, 0, 0, 0);
	if (ret) {
		ERROR("Sending wait message failed (%d)\n", ret);
		return;
	}

	/* clear L2FLUSHREQ */
	ret = ti_sci_proc_set_boot_ctrl_no_wait(proc_id,
			0, PROC_BOOT_CTRL_FLAG_ARMV8_L2FLUSHREQ);
	if (ret) {
		ERROR("Sending set control message failed (%d)\n", ret);
		return;
	}

	/* set ACINACTM */
	ret = ti_sci_proc_set_boot_ctrl_no_wait(proc_id,
			PROC_BOOT_CTRL_FLAG_ARMV8_ACINACTM, 0);
	if (ret) {
		ERROR("Sending set control message failed (%d)\n", ret);
		return;
	}

	/* wait for STANDBYWFIL2 */
	ret = ti_sci_proc_wait_boot_status_no_wait(proc_id,
			UINT8_MAX, 2, UINT8_MAX, UINT8_MAX,
			PROC_BOOT_STATUS_FLAG_ARMV8_STANDBYWFIL2, 0, 0, 0);
	if (ret) {
		ERROR("Sending wait message failed (%d)\n", ret);
		return;
	}

	/* Now queue up the cluster shutdown request */
	ret = ti_sci_device_put_no_wait(cluster_id);
	if (ret) {
		ERROR("Sending cluster shutdown message failed (%d)\n", ret);
		return;
	}
}

void k3_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* TODO: Indicate to System firmware about completion */

	k3_gic_pcpu_init();
	k3_gic_cpuif_enable();
}

static void __dead2 k3_system_off(void)
{
	ERROR("System Off: operation not handled.\n");
	while (true)
		wfi();
}

static void __dead2 k3_system_reset(void)
{
	/* Send the system reset request to system firmware */
	ti_sci_core_reboot();

	while (true)
		wfi();
}

static int k3_validate_power_state(unsigned int power_state,
				   psci_power_state_t *req_state)
{
	/* TODO: perform the proper validation */

	return PSCI_E_SUCCESS;
}

static int k3_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/* TODO: perform the proper validation */

	return PSCI_E_SUCCESS;
}

#if K3_PM_SYSTEM_SUSPEND
static void k3_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int core, proc_id;

	core = plat_my_core_pos();
	proc_id = PLAT_PROC_START_ID + core;

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();
	k3_gic_save_context();

	k3_pwr_domain_off(target_state);

	ti_sci_enter_sleep(proc_id, 0, k3_sec_entrypoint);
}

static void k3_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	k3_gic_restore_context();
	k3_gic_cpuif_enable();
}

static void k3_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	/* CPU & cluster off, system in retention */
	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}
#endif

static const plat_psci_ops_t k3_plat_psci_ops = {
	.cpu_standby = k3_cpu_standby,
	.pwr_domain_on = k3_pwr_domain_on,
	.pwr_domain_off = k3_pwr_domain_off,
	.pwr_domain_on_finish = k3_pwr_domain_on_finish,
#if K3_PM_SYSTEM_SUSPEND
	.pwr_domain_suspend = k3_pwr_domain_suspend,
	.pwr_domain_suspend_finish = k3_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = k3_get_sys_suspend_power_state,
#endif
	.system_off = k3_system_off,
	.system_reset = k3_system_reset,
	.validate_power_state = k3_validate_power_state,
	.validate_ns_entrypoint = k3_validate_ns_entrypoint
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	k3_sec_entrypoint = sec_entrypoint;

	*psci_ops = &k3_plat_psci_ops;

	return 0;
}
