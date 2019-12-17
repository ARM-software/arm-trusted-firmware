/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <sunxi_cpucfg.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define SUNXI_WDOG0_CTRL_REG		(SUNXI_R_WDOG_BASE + 0x0010)
#define SUNXI_WDOG0_CFG_REG		(SUNXI_R_WDOG_BASE + 0x0014)
#define SUNXI_WDOG0_MODE_REG		(SUNXI_R_WDOG_BASE + 0x0018)

#define mpidr_is_valid(mpidr) ( \
	MPIDR_AFFLVL3_VAL(mpidr) == 0 && \
	MPIDR_AFFLVL2_VAL(mpidr) == 0 && \
	MPIDR_AFFLVL1_VAL(mpidr) < PLATFORM_CLUSTER_COUNT && \
	MPIDR_AFFLVL0_VAL(mpidr) < PLATFORM_MAX_CPUS_PER_CLUSTER)

static int sunxi_pwr_domain_on(u_register_t mpidr)
{
	if (mpidr_is_valid(mpidr) == 0)
		return PSCI_E_INTERN_FAIL;

	sunxi_cpu_on(mpidr);

	return PSCI_E_SUCCESS;
}

static void sunxi_pwr_domain_off(const psci_power_state_t *target_state)
{
	gicv2_cpuif_disable();
}

static void __dead2 sunxi_pwr_down_wfi(const psci_power_state_t *target_state)
{
	sunxi_cpu_off(read_mpidr());

	while (1)
		wfi();
}

static void sunxi_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static void __dead2 sunxi_system_off(void)
{
	/* Turn off all secondary CPUs */
	sunxi_disable_secondary_cpus(read_mpidr());

	sunxi_power_down();

	udelay(1000);
	ERROR("PSCI: Cannot turn off system, halting\n");
	wfi();
	panic();
}

static void __dead2 sunxi_system_reset(void)
{
	/* Reset the whole system when the watchdog times out */
	mmio_write_32(SUNXI_WDOG0_CFG_REG, 1);
	/* Enable the watchdog with the shortest timeout (0.5 seconds) */
	mmio_write_32(SUNXI_WDOG0_MODE_REG, (0 << 4) | 1);
	/* Wait for twice the watchdog timeout before panicking */
	mdelay(1000);

	ERROR("PSCI: System reset failed\n");
	wfi();
	panic();
}

static int sunxi_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entry point must be in DRAM */
	if (ns_entrypoint >= SUNXI_DRAM_BASE)
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

static plat_psci_ops_t sunxi_psci_ops = {
	.pwr_domain_on			= sunxi_pwr_domain_on,
	.pwr_domain_off			= sunxi_pwr_domain_off,
	.pwr_domain_pwr_down_wfi	= sunxi_pwr_down_wfi,
	.pwr_domain_on_finish		= sunxi_pwr_domain_on_finish,
	.system_off			= sunxi_system_off,
	.system_reset			= sunxi_system_reset,
	.validate_ns_entrypoint		= sunxi_validate_ns_entrypoint,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	assert(psci_ops);

	for (int cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu += 1) {
		mmio_write_32(SUNXI_CPUCFG_RVBAR_LO_REG(cpu),
			      sec_entrypoint & 0xffffffff);
		mmio_write_32(SUNXI_CPUCFG_RVBAR_HI_REG(cpu),
			      sec_entrypoint >> 32);
	}

	*psci_ops = &sunxi_psci_ops;

	return 0;
}
