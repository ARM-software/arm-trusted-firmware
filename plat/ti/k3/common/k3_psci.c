/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <k3_gicv3.h>
#include <psci.h>
#include <stdbool.h>

#define STUB() ERROR("stub %s called\n", __func__)

uintptr_t k3_sec_entrypoint;

static void k3_cpu_standby(plat_local_state_t cpu_state)
{
	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

static int k3_pwr_domain_on(u_register_t mpidr)
{
	sev();

	/* TODO: Indicate to System firmware about powering up */

	return PSCI_E_SUCCESS;
}

void k3_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();

	/* TODO: Indicate to System firmware about powering down */
}

void k3_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* TODO: Indicate to System firmware about completion */

	k3_gic_pcpu_init();
	k3_gic_cpuif_enable();
}

static void __dead2 k3_system_reset(void)
{
	/* TODO: Indicate to System firmware about system reset */
	STUB();

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
