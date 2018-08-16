/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <cci.h>
#include <debug.h>
#include <gicv3.h>
#include <mmio.h>
#include <plat_imx8.h>
#include <psci.h>
#include <sci/sci.h>
#include <stdbool.h>

#define CORE_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) \
	((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

const static int ap_core_index[PLATFORM_CORE_COUNT] = {
	SC_R_A53_0, SC_R_A53_1, SC_R_A53_2,
	SC_R_A53_3, SC_R_A72_0, SC_R_A72_1,
};

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cluster_id, cpu_id;

	cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	printf("imx_pwr_domain_on cluster_id %d, cpu_id %d\n", cluster_id, cpu_id);

	if (cluster_id == 0) {
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_A53,
			SC_PM_PW_MODE_ON);
		if (sc_pm_set_resource_power_mode(ipc_handle, ap_core_index[cpu_id],
			SC_PM_PW_MODE_ON) != SC_ERR_NONE) {
			ERROR("cluster0 core %d power on failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}

		if (sc_pm_cpu_start(ipc_handle, ap_core_index[cpu_id],
			true, BL31_BASE) != SC_ERR_NONE) {
			ERROR("boot cluster0 core %d failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}
	} else {
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_A72,
			SC_PM_PW_MODE_ON);
		if (sc_pm_set_resource_power_mode(ipc_handle, ap_core_index[cpu_id + 4],
			SC_PM_PW_MODE_ON) != SC_ERR_NONE) {
			ERROR(" cluster1 core %d power on failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}

		if (sc_pm_cpu_start(ipc_handle, ap_core_index[cpu_id + 4],
			true, BL31_BASE) != SC_ERR_NONE) {
			ERROR("boot cluster1 core %d failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}
	}

	return ret;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();

	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();
	sc_pm_req_cpu_low_power_mode(ipc_handle,
		ap_core_index[cpu_id + cluster_id * 4],
		SC_PM_PW_MODE_OFF,
		SC_PM_WAKE_SRC_NONE);
	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));
	printf("turn off cluster:%d core:%d\n", cluster_id, cpu_id);
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();

	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	sc_pm_set_cpu_resume_addr(ipc_handle,
		ap_core_index[cpu_id + cluster_id * 4], BL31_BASE);
	sc_pm_req_cpu_low_power_mode(ipc_handle,
		ap_core_index[cpu_id + cluster_id * 4],
		SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_GIC);
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();

	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	plat_gic_cpuif_enable();
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.validate_power_state = imx_validate_power_state,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.system_off = imx_system_off,
	.system_reset = imx_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	imx_mailbox_init(sec_entrypoint);
	*psci_ops = &imx_plat_psci_ops;

	/* Request low power mode for cluster/cci, only need to do once */
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_OFF);
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A53, SC_PM_PW_MODE_OFF);
	sc_pm_req_low_power_mode(ipc_handle, SC_R_CCI, SC_PM_PW_MODE_OFF);

	/* Request RUN and LP modes for DDR, system interconnect etc. */
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53,
		SC_PM_SYS_IF_DDR, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72,
		SC_PM_SYS_IF_DDR, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53,
		SC_PM_SYS_IF_MU, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72,
		SC_PM_SYS_IF_MU, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53,
		SC_PM_SYS_IF_INTERCONNECT, SC_PM_PW_MODE_ON,
		SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72,
		SC_PM_SYS_IF_INTERCONNECT, SC_PM_PW_MODE_ON,
		SC_PM_PW_MODE_STBY);

	return 0;
}
