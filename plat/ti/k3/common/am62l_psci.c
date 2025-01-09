/*
 * Copyright (c) 2025, Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <k3_gicv3.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <stdbool.h>
#include <ti_sci.h>
#include <ti_sci_protocol.h>
#include <plat_scmi_def.h>
#include <device_wrapper.h>
#include <devices.h>

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define WKUP_CTRL_MMR0_DEVICE_MANAGEMENT_BASE	(0x43050000UL)
#define WKUP_CTRL_MMR0_DEVICE_RESET_OFFSET	(0x4000)

uintptr_t k3_sec_entrypoint;
uintptr_t k3_sec_entrypoint_glob;

static int k3_pwr_domain_on(u_register_t mpidr)
{
	int core, proc_id, ret;

	core = plat_core_pos_by_mpidr(mpidr);
	if (core < 0) {
		ERROR("Could not get target core id: %d\n", core);
		return PSCI_E_INTERN_FAIL;
	}

	proc_id = PLAT_PROC_START_ID + core;	// should be 0x21

	VERBOSE("proc_id = 0x%x\n", proc_id);

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

	/* TODO: Add the actual pm operation call to turn on the core */

	return PSCI_E_SUCCESS;
}

static void __dead2 k3_pwr_domain_off_wfi(const psci_power_state_t *target_state)
{
	/* At very least the local core should be powering down */
	assert(CORE_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE);

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();

	/* If our cluster is not going down we stop here */
	if (CLUSTER_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE) {
		/* TODO: Add the actual pm operation call to turn off the core */
	}

	while (true)
		wfi();
}

void k3_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	k3_gic_pcpu_init();
	k3_gic_cpuif_enable();
}

static void __dead2 k3_system_reset(void)
{
	mmio_write_32(WKUP_CTRL_MMR0_DEVICE_MANAGEMENT_BASE + WKUP_CTRL_MMR0_DEVICE_RESET_OFFSET,
		      0x6);

	ERROR("%s: Failed to reset device\n", __func__);
	while (true)
		wfi();
}

static int k3_validate_power_state(unsigned int power_state,
				   psci_power_state_t *req_state)
{
	/* TODO: perform the proper validation */

	return PSCI_E_SUCCESS;
}

static void k3_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	/* CPU & cluster off, system in retention */
	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}

static plat_psci_ops_t k3_plat_psci_ops = {
	.pwr_domain_on = k3_pwr_domain_on,
	.pwr_domain_pwr_down_wfi = k3_pwr_domain_off_wfi,
	.pwr_domain_on_finish = k3_pwr_domain_on_finish,
	.get_sys_suspend_power_state = k3_get_sys_suspend_power_state,
	.system_reset = k3_system_reset,
	.validate_power_state = k3_validate_power_state,
};

void  __aligned(16) jump_to_atf_func(void)
{
	void (*bl31_loc_warm_entry)(void) = (void *)k3_sec_entrypoint_glob; // bl31_warm_entrypoint

	bl31_loc_warm_entry();
}

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	k3_sec_entrypoint_glob = sec_entrypoint;
	k3_sec_entrypoint = (unsigned long)(void *)&jump_to_atf_func;
	VERBOSE("k3_sec_entrypoint = 0x%lx\n", k3_sec_entrypoint);

	*psci_ops = &k3_plat_psci_ops;

	return 0;
}
