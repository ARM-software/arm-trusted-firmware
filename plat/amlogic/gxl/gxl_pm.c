/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
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

static uintptr_t gxl_sec_entrypoint;
static volatile uint32_t gxl_cpu0_go;

static void gxl_pm_set_reset_addr(u_register_t mpidr, uint64_t value)
{
	unsigned int core = plat_calc_core_pos(mpidr);
	uintptr_t cpu_mailbox_addr = AML_PSCI_MAILBOX_BASE + (core << 4);

	mmio_write_64(cpu_mailbox_addr, value);
}

static void gxl_pm_reset(u_register_t mpidr)
{
	unsigned int core = plat_calc_core_pos(mpidr);
	uintptr_t cpu_mailbox_addr = AML_PSCI_MAILBOX_BASE + (core << 4) + 8;

	mmio_write_32(cpu_mailbox_addr, 0);
}

static void __dead2 gxl_system_reset(void)
{
	INFO("BL31: PSCI_SYSTEM_RESET\n");

	u_register_t mpidr = read_mpidr_el1();
	uint32_t status = mmio_read_32(AML_AO_RTI_STATUS_REG3);
	int ret;

	NOTICE("BL31: Reboot reason: 0x%x\n", status);

	status &= 0xFFFF0FF0;

	console_flush();

	mmio_write_32(AML_AO_RTI_STATUS_REG3, status);

	ret = aml_scpi_sys_power_state(SCPI_SYSTEM_REBOOT);

	if (ret != 0) {
		ERROR("BL31: PSCI_SYSTEM_RESET: SCP error: %i\n", ret);
		panic();
	}

	gxl_pm_reset(mpidr);

	wfi();

	ERROR("BL31: PSCI_SYSTEM_RESET: Operation not handled\n");
	panic();
}

static void __dead2 gxl_system_off(void)
{
	INFO("BL31: PSCI_SYSTEM_OFF\n");

	u_register_t mpidr = read_mpidr_el1();
	int ret;

	ret = aml_scpi_sys_power_state(SCPI_SYSTEM_SHUTDOWN);

	if (ret != 0) {
		ERROR("BL31: PSCI_SYSTEM_OFF: SCP error %i\n", ret);
		panic();
	}

	gxl_pm_set_reset_addr(mpidr, 0);
	gxl_pm_reset(mpidr);

	wfi();

	ERROR("BL31: PSCI_SYSTEM_OFF: Operation not handled\n");
	panic();
}

static int32_t gxl_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core = plat_calc_core_pos(mpidr);

	/* CPU0 can't be turned OFF, emulate it with a WFE loop */
	if (core == AML_PRIMARY_CPU) {
		VERBOSE("BL31: Releasing CPU0 from wait loop...\n");

		gxl_cpu0_go = 1;
		flush_dcache_range((uintptr_t)&gxl_cpu0_go,
				sizeof(gxl_cpu0_go));
		dsb();
		isb();

		sev();

		return PSCI_E_SUCCESS;
	}

	gxl_pm_set_reset_addr(mpidr, gxl_sec_entrypoint);
	aml_scpi_set_css_power_state(mpidr,
				     SCPI_POWER_ON, SCPI_POWER_ON, SCPI_POWER_ON);
	dmbsy();
	sev();

	return PSCI_E_SUCCESS;
}

static void gxl_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned int core = plat_calc_core_pos(read_mpidr_el1());

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

	if (core == AML_PRIMARY_CPU) {
		gxl_cpu0_go = 0;
		flush_dcache_range((uintptr_t)&gxl_cpu0_go,
				sizeof(gxl_cpu0_go));
		dsb();
		isb();
	}

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static void gxl_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int core = plat_calc_core_pos(mpidr);

	gicv2_cpuif_disable();

	/* CPU0 can't be turned OFF, emulate it with a WFE loop */
	if (core == AML_PRIMARY_CPU)
		return;

	aml_scpi_set_css_power_state(mpidr,
				     SCPI_POWER_OFF, SCPI_POWER_ON, SCPI_POWER_ON);
}

static void __dead2 gxl_pwr_domain_pwr_down_wfi(const psci_power_state_t
						 *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int core = plat_calc_core_pos(mpidr);

	/* CPU0 can't be turned OFF, emulate it with a WFE loop */
	if (core == AML_PRIMARY_CPU) {
		VERBOSE("BL31: CPU0 entering wait loop...\n");

		while (gxl_cpu0_go == 0)
			wfe();

		VERBOSE("BL31: CPU0 resumed.\n");

		/*
		 * Because setting CPU0's warm reset entrypoint through PSCI
		 * mailbox and/or mmio mapped RVBAR (0xda834650) does not seem
		 * to work, jump to it manually.
		 * In order to avoid an assert, mmu has to be disabled.
		 */
		disable_mmu_el3();
		((void(*)(void))gxl_sec_entrypoint)();
	}

	dsbsy();
	gxl_pm_set_reset_addr(mpidr, 0);
	gxl_pm_reset(mpidr);

	for (;;)
		wfi();
}

/*******************************************************************************
 * Platform handlers and setup function.
 ******************************************************************************/
static const plat_psci_ops_t gxl_ops = {
	.pwr_domain_on			= gxl_pwr_domain_on,
	.pwr_domain_on_finish		= gxl_pwr_domain_on_finish,
	.pwr_domain_off			= gxl_pwr_domain_off,
	.pwr_domain_pwr_down_wfi	= gxl_pwr_domain_pwr_down_wfi,
	.system_off			= gxl_system_off,
	.system_reset			= gxl_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	gxl_sec_entrypoint = sec_entrypoint;
	*psci_ops = &gxl_ops;
	gxl_cpu0_go = 0;
	return 0;
}
