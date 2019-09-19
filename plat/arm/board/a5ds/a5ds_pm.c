/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <drivers/arm/gicv2.h>

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
static int a5ds_pwr_domain_on(u_register_t mpidr)
{
	unsigned int pos = plat_core_pos_by_mpidr(mpidr);
	uint64_t *hold_base = (uint64_t *)A5DS_HOLD_BASE;

	hold_base[pos] = A5DS_HOLD_STATE_GO;
	dsbish();
	sev();

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void a5ds_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* TODO: This setup is needed only after a cold boot*/
	gicv2_pcpu_distif_init();

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
}

/*******************************************************************************
 * Export the platform handlers via a5ds_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
plat_psci_ops_t a5ds_psci_pm_ops = {
	/* dummy struct */
	.validate_ns_entrypoint = NULL,
	.pwr_domain_on = a5ds_pwr_domain_on,
	.pwr_domain_on_finish = a5ds_pwr_domain_on_finish
};

int __init plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	uintptr_t *mailbox = (void *)A5DS_TRUSTED_MAILBOX_BASE;
	*mailbox = sec_entrypoint;

	*psci_ops = &a5ds_psci_pm_ops;

	return 0;
}
