/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <plat_imx8.h>

static uintptr_t secure_entrypoint;

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define RVBARADDRx(c)		(IMX_SIM1_BASE + 0x5c + 0x4 * (c))
#define WKPUx(c)		(IMX_SIM1_BASE + 0x3c + 0x4 * (c))
#define AD_COREx_LPMODE(c)	(IMX_CMC1_BASE + 0x50 + 0x4 * (c))

static int imx_pwr_set_cpu_entry(unsigned int cpu, unsigned int entry)
{
	mmio_write_32(RVBARADDRx(cpu), entry);

	/* set update bit */
	mmio_write_32(IMX_SIM1_BASE + 0x8, mmio_read_32(IMX_SIM1_BASE + 0x8) | BIT_32(24 + cpu));
	/* wait for ack */
	while (!(mmio_read_32(IMX_SIM1_BASE + 0x8) & BIT_32(26 + cpu))) {
	}

	/* clear update bit */
	mmio_write_32(IMX_SIM1_BASE + 0x8, mmio_read_32(IMX_SIM1_BASE + 0x8) & ~BIT_32(24 + cpu));
	/* clear ack bit */
	mmio_write_32(IMX_SIM1_BASE + 0x8, mmio_read_32(IMX_SIM1_BASE + 0x8) | BIT_32(26 + cpu));

	return 0;
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu = MPIDR_AFFLVL0_VAL(mpidr);

	imx_pwr_set_cpu_entry(cpu, secure_entrypoint);

	mmio_write_32(IMX_CMC1_BASE + 0x18, 0x3f);
	mmio_write_32(IMX_CMC1_BASE + 0x50 + 0x4 * cpu, 0);

	/* enable wku wakeup for idle */
	mmio_write_32(IMX_SIM1_BASE + 0x3c + 0x4 * cpu, 0xffffffff);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	imx_pwr_set_cpu_entry(0, IMX_ROM_ENTRY);
	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned int cpu = MPIDR_AFFLVL0_VAL(read_mpidr_el1());

	plat_gic_cpuif_disable();

	/* disable wakeup */
	mmio_write_32(WKPUx(cpu), 0);

	mmio_write_32(AD_COREx_LPMODE(cpu), 0x3);
}

void __dead2 imx8ulp_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1) {
		wfi();
	}
}

void __dead2 imx8ulp_system_reset(void)
{
	imx_pwr_set_cpu_entry(0, IMX_ROM_ENTRY);

	/* Write invalid command to WDOG CNT to trigger reset */
	mmio_write_32(IMX_WDOG3_BASE + 0x4, 0x12345678);

	while (true) {
		wfi();
	}
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.system_reset = imx8ulp_system_reset,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_pwr_down_wfi = imx8ulp_pwr_domain_pwr_down_wfi,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	secure_entrypoint = sec_entrypoint;
	imx_pwr_set_cpu_entry(0, sec_entrypoint);
	*psci_ops = &imx_plat_psci_ops;

	mmio_write_32(IMX_CMC1_BASE + 0x18, 0x3f);
	mmio_write_32(IMX_SIM1_BASE + 0x3c, 0xffffffff);

	return 0;
}
