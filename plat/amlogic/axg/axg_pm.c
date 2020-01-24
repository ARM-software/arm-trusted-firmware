/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/console.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "aml_private.h"

#define SCPI_POWER_ON		0
#define SCPI_POWER_RETENTION	1
#define SCPI_POWER_OFF		3

#define SCPI_SYSTEM_SHUTDOWN	0
#define SCPI_SYSTEM_REBOOT	1

static uintptr_t axg_sec_entrypoint;

static void axg_pm_set_reset_addr(u_register_t mpidr, uint64_t value)
{
	unsigned int core = plat_calc_core_pos(mpidr);
	uintptr_t cpu_mailbox_addr = AML_PSCI_MAILBOX_BASE + (core << 4);

	mmio_write_64(cpu_mailbox_addr, value);
}

static void axg_pm_reset(u_register_t mpidr, uint32_t value)
{
	unsigned int core = plat_calc_core_pos(mpidr);
	uintptr_t cpu_mailbox_addr = AML_PSCI_MAILBOX_BASE + (core << 4) + 8;

	mmio_write_32(cpu_mailbox_addr, value);
}

static void __dead2 axg_system_reset(void)
{
	u_register_t mpidr = read_mpidr_el1();
	int ret;

	INFO("BL31: PSCI_SYSTEM_RESET\n");

	ret = aml_scpi_sys_power_state(SCPI_SYSTEM_REBOOT);
	if (ret != 0) {
		ERROR("BL31: PSCI_SYSTEM_RESET: SCP error: %i\n", ret);
		panic();
	}

	axg_pm_reset(mpidr, 0);

	wfi();

	ERROR("BL31: PSCI_SYSTEM_RESET: Operation not handled\n");
	panic();
}

static void __dead2 axg_system_off(void)
{
	u_register_t mpidr = read_mpidr_el1();
	int ret;

	INFO("BL31: PSCI_SYSTEM_OFF\n");

	ret = aml_scpi_sys_power_state(SCPI_SYSTEM_SHUTDOWN);
	if (ret != 0) {
		ERROR("BL31: PSCI_SYSTEM_OFF: SCP error %i\n", ret);
		panic();
	}

	axg_pm_set_reset_addr(mpidr, 0);
	axg_pm_reset(mpidr, 0);

	dmbsy();
	wfi();

	ERROR("BL31: PSCI_SYSTEM_OFF: Operation not handled\n");
	panic();
}

static int32_t axg_pwr_domain_on(u_register_t mpidr)
{
	axg_pm_set_reset_addr(mpidr, axg_sec_entrypoint);
	aml_scpi_set_css_power_state(mpidr,
				     SCPI_POWER_ON, SCPI_POWER_ON, SCPI_POWER_ON);
	dmbsy();
	sev();

	return PSCI_E_SUCCESS;
}

static void axg_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	axg_pm_set_reset_addr(read_mpidr_el1(), 0);
}

static void axg_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	uint32_t system_state = SCPI_POWER_ON;
	uint32_t cluster_state = SCPI_POWER_ON;

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

	axg_pm_reset(mpidr, -1);

	gicv2_cpuif_disable();

	if (target_state->pwr_domain_state[MPIDR_AFFLVL2] ==
					PLAT_LOCAL_STATE_OFF)
		system_state = SCPI_POWER_OFF;

	if (target_state->pwr_domain_state[MPIDR_AFFLVL1] ==
					PLAT_LOCAL_STATE_OFF)
		cluster_state = SCPI_POWER_OFF;


	aml_scpi_set_css_power_state(mpidr,
				     SCPI_POWER_OFF, cluster_state,
				     system_state);
}

static void __dead2 axg_pwr_domain_pwr_down_wfi(const psci_power_state_t
						 *target_state)
{
	dsbsy();
	axg_pm_reset(read_mpidr_el1(), 0);

	for (;;)
		wfi();
}

/*******************************************************************************
 * Platform handlers and setup function.
 ******************************************************************************/
static const plat_psci_ops_t axg_ops = {
	.pwr_domain_on			= axg_pwr_domain_on,
	.pwr_domain_on_finish		= axg_pwr_domain_on_finish,
	.pwr_domain_off			= axg_pwr_domain_off,
	.pwr_domain_pwr_down_wfi	= axg_pwr_domain_pwr_down_wfi,
	.system_off			= axg_system_off,
	.system_reset			= axg_system_reset
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	axg_sec_entrypoint = sec_entrypoint;
	*psci_ops = &axg_ops;
	return 0;
}
