/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
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

#ifdef TI_AM65X_WORKAROUND
/* Need to flush psci internal locks before shutdown or their values are lost */
#include "../../../../lib/psci/psci_private.h"
#endif

uintptr_t k3_sec_entrypoint;

static void k3_cpu_standby(plat_local_state_t cpu_state)
{
	unsigned int scr;

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
	int core_id, proc, device, ret;

	core_id = plat_core_pos_by_mpidr(mpidr);
	if (core_id < 0) {
		ERROR("Could not get target core id: %d\n", core_id);
		return PSCI_E_INTERN_FAIL;
	}

	proc = PLAT_PROC_START_ID + core_id;
	device = PLAT_PROC_DEVICE_START_ID + core_id;

	ret = ti_sci_proc_request(proc);
	if (ret) {
		ERROR("Request for processor failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	ret = ti_sci_proc_set_boot_cfg(proc, k3_sec_entrypoint, 0, 0);
	if (ret) {
		ERROR("Request to set core boot address failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	ret = ti_sci_device_get(device);
	if (ret) {
		ERROR("Request to start core failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	return PSCI_E_SUCCESS;
}

void k3_pwr_domain_off(const psci_power_state_t *target_state)
{
	int core_id, proc, device, ret;

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();

	core_id = plat_my_core_pos();
	proc = PLAT_PROC_START_ID + core_id;
	device = PLAT_PROC_DEVICE_START_ID + core_id;

	/* Start by sending wait for WFI command */
	ret = ti_sci_proc_wait_boot_status_no_wait(proc,
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
	ret = ti_sci_device_put_no_wait(device);
	if (ret) {
		ERROR("Sending core shutdown message failed (%d)\n", ret);
		return;
	}
}

void k3_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* TODO: Indicate to System firmware about completion */

	k3_gic_pcpu_init();
	k3_gic_cpuif_enable();
}

#ifdef TI_AM65X_WORKAROUND
static void  __dead2 k3_pwr_domain_pwr_down_wfi(const psci_power_state_t
						  *target_state)
{
	flush_cpu_data(psci_svc_cpu_data);
	flush_dcache_range((uintptr_t) psci_locks, sizeof(psci_locks));
	psci_power_down_wfi();
}
#endif

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

static const plat_psci_ops_t k3_plat_psci_ops = {
	.cpu_standby = k3_cpu_standby,
	.pwr_domain_on = k3_pwr_domain_on,
	.pwr_domain_off = k3_pwr_domain_off,
	.pwr_domain_on_finish = k3_pwr_domain_on_finish,
#ifdef TI_AM65X_WORKAROUND
	.pwr_domain_pwr_down_wfi = k3_pwr_domain_pwr_down_wfi,
#endif
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
