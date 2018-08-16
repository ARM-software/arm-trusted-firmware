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

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cpu_id;

	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	printf("imx_pwr_domain_on cpu_id %d\n", cpu_id);

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

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();
	sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
		SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_NONE);
	printf("turn off core:%d\n", cpu_id);
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();

	sc_pm_set_cpu_resume_addr(ipc_handle, ap_core_index[cpu_id], BL31_BASE);
	sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
		SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_GIC);
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	sc_pm_req_low_power_mode(ipc_handle, ap_core_index[cpu_id],
		SC_PM_PW_MODE_ON);

	plat_gic_cpuif_enable();
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.system_off = imx_system_off,
	.system_reset = imx_system_reset,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.validate_power_state = imx_validate_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	imx_mailbox_init(sec_entrypoint);
	*psci_ops = &imx_plat_psci_ops;

	/* Request low power mode for A35 cluster, only need to do once */
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A35, SC_PM_PW_MODE_OFF);

	/* Request RUN and LP modes for DDR, system interconnect etc. */
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35,
		SC_PM_SYS_IF_DDR, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35,
		SC_PM_SYS_IF_MU, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35,
		SC_PM_SYS_IF_INTERCONNECT, SC_PM_PW_MODE_ON,
		SC_PM_PW_MODE_STBY);

	return 0;
}
