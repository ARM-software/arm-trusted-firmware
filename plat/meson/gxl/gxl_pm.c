/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <drivers/console.h>
#include <common/debug.h>
#include <errno.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <lib/psci/psci.h>

#include "gxl_private.h"

#define SCPI_POWER_ON		0
#define SCPI_POWER_RETENTION	1
#define SCPI_POWER_OFF		3

#define SCPI_SYSTEM_SHUTDOWN	0
#define SCPI_SYSTEM_REBOOT	1

static uintptr_t gxbb_sec_entrypoint;
static volatile uint32_t gxbb_cpu0_go;

static void gxbb_program_mailbox(u_register_t mpidr, uint64_t value)
{
	unsigned int core = plat_gxbb_calc_core_pos(mpidr);
	uintptr_t cpu_mailbox_addr = GXBB_PSCI_MAILBOX_BASE + (core << 4);

	mmio_write_64(cpu_mailbox_addr, value);
	flush_dcache_range(cpu_mailbox_addr, sizeof(uint64_t));
}

static void __dead2 gxbb_system_reset(void)
{
	INFO("BL31: PSCI_SYSTEM_RESET\n");

	uint32_t status = mmio_read_32(GXBB_AO_RTI_STATUS_REG3);

	NOTICE("BL31: Reboot reason: 0x%x\n", status);

	status &= 0xFFFF0FF0;

	console_flush();

	mmio_write_32(GXBB_AO_RTI_STATUS_REG3, status);

	int ret = scpi_sys_power_state(SCPI_SYSTEM_REBOOT);

	if (ret != 0) {
		ERROR("BL31: PSCI_SYSTEM_RESET: SCP error: %u\n", ret);
		panic();
	}

	wfi();

	ERROR("BL31: PSCI_SYSTEM_RESET: Operation not handled\n");
	panic();
}

static void __dead2 gxbb_system_off(void)
{
	INFO("BL31: PSCI_SYSTEM_OFF\n");

	unsigned int ret = scpi_sys_power_state(SCPI_SYSTEM_SHUTDOWN);

	if (ret != 0) {
		ERROR("BL31: PSCI_SYSTEM_OFF: SCP error %u\n", ret);
		panic();
	}

	gxbb_program_mailbox(read_mpidr_el1(), 0);

	wfi();

	ERROR("BL31: PSCI_SYSTEM_OFF: Operation not handled\n");
	panic();
}

static int32_t gxbb_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core = plat_gxbb_calc_core_pos(mpidr);

	/* CPU0 can't be turned OFF, emulate it with a WFE loop */
	if (core == GXBB_PRIMARY_CPU) {
		VERBOSE("BL31: Releasing CPU0 from wait loop...\n");

		gxbb_cpu0_go = 1;
		flush_dcache_range((uintptr_t)&gxbb_cpu0_go,
				sizeof(gxbb_cpu0_go));
		dsb();
		isb();

		sev();

		return PSCI_E_SUCCESS;
	}

	gxbb_program_mailbox(mpidr, gxbb_sec_entrypoint);
	scpi_set_css_power_state(mpidr,
				 SCPI_POWER_ON, SCPI_POWER_ON, SCPI_POWER_ON);
	dmbsy();
	sev();

	return PSCI_E_SUCCESS;
}

static void gxbb_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned int core = plat_gxbb_calc_core_pos(read_mpidr_el1());

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

	if (core == GXBB_PRIMARY_CPU) {
		gxbb_cpu0_go = 0;
		flush_dcache_range((uintptr_t)&gxbb_cpu0_go,
				sizeof(gxbb_cpu0_go));
		dsb();
		isb();
	}

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static void gxbb_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int core = plat_gxbb_calc_core_pos(mpidr);
	uintptr_t addr = GXBB_PSCI_MAILBOX_BASE + 8 + (core << 4);

	mmio_write_32(addr, 0xFFFFFFFF);
	flush_dcache_range(addr, sizeof(uint32_t));

	gicv2_cpuif_disable();

	/* CPU0 can't be turned OFF, emulate it with a WFE loop */
	if (core == GXBB_PRIMARY_CPU)
		return;

	scpi_set_css_power_state(mpidr,
				 SCPI_POWER_OFF, SCPI_POWER_ON, SCPI_POWER_ON);
}

static void __dead2 gxbb_pwr_domain_pwr_down_wfi(const psci_power_state_t
						 *target_state)
{
	unsigned int core = plat_gxbb_calc_core_pos(read_mpidr_el1());

	/* CPU0 can't be turned OFF, emulate it with a WFE loop */
	if (core == GXBB_PRIMARY_CPU) {
		VERBOSE("BL31: CPU0 entering wait loop...\n");

		while (gxbb_cpu0_go == 0)
			wfe();

		VERBOSE("BL31: CPU0 resumed.\n");

		write_rmr_el3(RMR_EL3_RR_BIT | RMR_EL3_AA64_BIT);
	}

	dsbsy();

	for (;;)
		wfi();
}

/*******************************************************************************
 * Platform handlers and setup function.
 ******************************************************************************/
static const plat_psci_ops_t gxbb_ops = {
	.pwr_domain_on			= gxbb_pwr_domain_on,
	.pwr_domain_on_finish		= gxbb_pwr_domain_on_finish,
	.pwr_domain_off			= gxbb_pwr_domain_off,
	.pwr_domain_pwr_down_wfi	= gxbb_pwr_domain_pwr_down_wfi,
	.system_off			= gxbb_system_off,
	.system_reset			= gxbb_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	gxbb_sec_entrypoint = sec_entrypoint;
	*psci_ops = &gxbb_ops;
	gxbb_cpu0_go = 0;
	return 0;
}
