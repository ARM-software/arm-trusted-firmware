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

const static int ap_core_index[PLATFORM_CORE_COUNT] = {
	SC_R_A53_0, SC_R_A53_1, SC_R_A53_2,
	SC_R_A53_3, SC_R_A72_0, SC_R_A72_1,
};

/* need to enable USE_COHERENT_MEM to avoid coherence issue */
#if USE_COHERENT_MEM
static unsigned int a53_cpu_on_number __section("tzfw_coherent_mem");
static unsigned int a72_cpu_on_number __section("tzfw_coherent_mem");
#endif

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cluster_id, cpu_id;

	cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	tf_printf("imx_pwr_domain_on cluster_id %d, cpu_id %d\n", cluster_id, cpu_id);

	if (cluster_id == 0) {
		if (a53_cpu_on_number == 0)
			sc_pm_set_resource_power_mode(ipc_handle, SC_R_A53, SC_PM_PW_MODE_ON);

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
		if (a72_cpu_on_number == 0)
			sc_pm_set_resource_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_ON);

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
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);

	if (cluster_id == 0 && a53_cpu_on_number++ == 0)
		cci_enable_snoop_dvm_reqs(0);
	if (cluster_id == 1 && a72_cpu_on_number++ == 0)
		cci_enable_snoop_dvm_reqs(1);

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

void __attribute__((noreturn)) imx_system_off(void)
{
	sc_pm_set_sys_power_mode(ipc_handle, SC_PM_PW_MODE_OFF);
	wfi();
	ERROR("power off failed.\n");
	panic();
}

void  __attribute__((noreturn)) imx_system_reset(void)
{
	sc_pm_reset(ipc_handle, SC_PM_RESET_TYPE_BOARD);

	while (1)
		;
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.system_off = imx_system_off,
	.system_reset = imx_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);

	imx_mailbox_init(sec_entrypoint);
	*psci_ops = &imx_plat_psci_ops;

	if (cluster_id == 0)
		a53_cpu_on_number++;
	else
		a72_cpu_on_number++;

	return 0;
}
