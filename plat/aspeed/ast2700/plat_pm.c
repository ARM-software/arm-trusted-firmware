/*
 * Copyright (c) 2023, Aspeed Technology Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

static uintptr_t sec_ep;

static int plat_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu = plat_core_pos_by_mpidr(mpidr);
	uintptr_t ep_reg;

	switch (cpu) {
	case 1U:
		ep_reg = SCU_CPU_SMP_EP1;
		break;
	case 2U:
		ep_reg = SCU_CPU_SMP_EP2;
		break;
	case 3U:
		ep_reg = SCU_CPU_SMP_EP3;
		break;
	default:
		return PSCI_E_INVALID_PARAMS;
	}

	mmio_write_64(ep_reg, sec_ep);

	dsbsy();

	sev();

	return PSCI_E_SUCCESS;
}

static void plat_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

static const plat_psci_ops_t plat_psci_ops = {
	.pwr_domain_on = plat_pwr_domain_on,
	.pwr_domain_on_finish = plat_pwr_domain_on_finish,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	sec_ep = sec_entrypoint;
	*psci_ops = &plat_psci_ops;

	return 0;
}
