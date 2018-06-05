/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <gicv3.h>
#include <mmio.h>
#include <plat_imx8.h>
#include <psci.h>
#include <sci/sci.h>
#include <stdbool.h>

const static int ap_core_index[PLATFORM_CORE_COUNT] = {
	SC_R_A35_0, SC_R_A35_1, SC_R_A35_2, SC_R_A35_3
};

plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *target_state,
					     unsigned int ncpu)
{
	return 0;
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cpu_id;

	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	tf_printf("imx_pwr_domain_on cpu_id %d\n", cpu_id);

	if (sc_pm_set_resource_power_mode(ipc_handle, ap_core_index[cpu_id],
	    SC_PM_PW_MODE_ON) != SC_ERR_NONE) {
		ERROR("core %d power on failed!\n", cpu_id);
		ret = PSCI_E_INTERN_FAIL;
	}

	if (sc_pm_cpu_start(ipc_handle, ap_core_index[cpu_id],
	    true, BL31_BASE) != SC_ERR_NONE) {
		ERROR("boot core %d failed!\n", cpu_id);
		ret = PSCI_E_INTERN_FAIL;
	}

	return ret;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	imx_mailbox_init(sec_entrypoint);
	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
