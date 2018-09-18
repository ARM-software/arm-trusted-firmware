/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <gicv2.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>

#include "gxbb_private.h"

#define SCPI_POWER_ON		0
#define SCPI_POWER_RETENTION	1
#define SCPI_POWER_OFF		3

#define SCPI_SYSTEM_SHUTDOWN	0
#define SCPI_SYSTEM_REBOOT	1

static uintptr_t gxbb_sec_entrypoint;

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
	gxbb_program_mailbox(mpidr, gxbb_sec_entrypoint);
	scpi_set_css_power_state(mpidr,
				 SCPI_POWER_ON, SCPI_POWER_ON, SCPI_POWER_ON);
	dmbsy();
	sev();

	return PSCI_E_SUCCESS;
}

static void gxbb_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/*******************************************************************************
 * Platform handlers and setup function.
 ******************************************************************************/
static const plat_psci_ops_t gxbb_ops = {
	.pwr_domain_on			= gxbb_pwr_domain_on,
	.pwr_domain_on_finish		= gxbb_pwr_domain_on_finish,
	.system_off			= gxbb_system_off,
	.system_reset			= gxbb_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	gxbb_sec_entrypoint = sec_entrypoint;
	*psci_ops = &gxbb_ops;
	return 0;
}
