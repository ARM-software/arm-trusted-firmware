/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/cassert.h>
#include <lib/psci/psci.h>

#include <sq_common.h>
#include "sq_scpi.h"

uintptr_t sq_sec_entrypoint;

int sq_pwr_domain_on(u_register_t mpidr)
{
#if SQ_USE_SCMI_DRIVER
	sq_scmi_on(mpidr);
#else
	/*
	 * SCP takes care of powering up parent power domains so we
	 * only need to care about level 0
	 */
	scpi_set_sq_power_state(mpidr, scpi_power_on, scpi_power_on,
				 scpi_power_on);
#endif

	return PSCI_E_SUCCESS;
}

static void sq_pwr_domain_on_finisher_common(
		const psci_power_state_t *target_state)
{
	assert(SQ_CORE_PWR_STATE(target_state) == SQ_LOCAL_STATE_OFF);

	/*
	 * Perform the common cluster specific operations i.e enable coherency
	 * if this cluster was off.
	 */
	if (SQ_CLUSTER_PWR_STATE(target_state) == SQ_LOCAL_STATE_OFF)
		plat_sq_interconnect_enter_coherency();
}

void sq_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* Assert that the system power domain need not be initialized */
	assert(SQ_SYSTEM_PWR_STATE(target_state) == SQ_LOCAL_STATE_RUN);

	sq_pwr_domain_on_finisher_common(target_state);

	/* Program the gic per-cpu distributor or re-distributor interface */
	sq_gic_pcpu_init();

	/* Enable the gic cpu interface */
	sq_gic_cpuif_enable();
}

#if !SQ_USE_SCMI_DRIVER
static void sq_power_down_common(const psci_power_state_t *target_state)
{
	uint32_t cluster_state = scpi_power_on;
	uint32_t system_state = scpi_power_on;

	/* Prevent interrupts from spuriously waking up this cpu */
	sq_gic_cpuif_disable();

	/* Check if power down at system power domain level is requested */
	if (SQ_SYSTEM_PWR_STATE(target_state) == SQ_LOCAL_STATE_OFF)
		system_state = scpi_power_retention;

	/* Cluster is to be turned off, so disable coherency */
	if (SQ_CLUSTER_PWR_STATE(target_state) == SQ_LOCAL_STATE_OFF) {
		plat_sq_interconnect_exit_coherency();
		cluster_state = scpi_power_off;
	}

	/*
	 * Ask the SCP to power down the appropriate components depending upon
	 * their state.
	 */
	scpi_set_sq_power_state(read_mpidr_el1(),
				 scpi_power_off,
				 cluster_state,
				 system_state);
}
#endif

void sq_pwr_domain_off(const psci_power_state_t *target_state)
{
#if SQ_USE_SCMI_DRIVER
	/* Prevent interrupts from spuriously waking up this cpu */
	sq_gic_cpuif_disable();

	/* Cluster is to be turned off, so disable coherency */
	if (SQ_CLUSTER_PWR_STATE(target_state) == SQ_LOCAL_STATE_OFF) {
		plat_sq_interconnect_exit_coherency();
	}

	sq_scmi_off(target_state);
#else
	sq_power_down_common(target_state);
#endif
}

void __dead2 sq_system_off(void)
{
#if SQ_USE_SCMI_DRIVER
	sq_scmi_sys_shutdown();
#else
	volatile uint32_t *gpio = (uint32_t *)PLAT_SQ_GPIO_BASE;

	/* set PD[9] high to power off the system */
	gpio[5] |= 0x2;		/* set output */
	gpio[1] |= 0x2;		/* set high */
	dmbst();

	generic_delay_timer_init();

	mdelay(1);

	while (1) {
		gpio[1] &= ~0x2;	/* set low */
		dmbst();

		mdelay(1);

		gpio[1] |= 0x2;		/* set high */
		dmbst();

		mdelay(100);
	}

	wfi();
	ERROR("SQ System Off: operation not handled.\n");
	panic();
#endif
}

void __dead2 sq_system_reset(void)
{
#if SQ_USE_SCMI_DRIVER
	sq_scmi_sys_reboot();
#else
	uint32_t response;

	/* Send the system reset request to the SCP */
	response = scpi_sys_power_state(scpi_system_reboot);

	if (response != SCP_OK) {
		ERROR("SQ System Reset: SCP error %u.\n", response);
		panic();
	}
	wfi();
	ERROR("SQ System Reset: operation not handled.\n");
	panic();
#endif
}

void sq_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr;

	assert(cpu_state == SQ_LOCAL_STATE_RET);

	scr = read_scr_el3();
	/* Enable PhysicalIRQ bit for NS world to wake the CPU */
	write_scr_el3(scr | SCR_IRQ_BIT);
	isb();
	dsb();
	wfi();

	/*
	 * Restore SCR to the original value, synchronisation of scr_el3 is
	 * done by eret while el3_exit to save some execution cycles.
	 */
	write_scr_el3(scr);
}

const plat_psci_ops_t sq_psci_ops = {
	.pwr_domain_on		= sq_pwr_domain_on,
	.pwr_domain_off		= sq_pwr_domain_off,
	.pwr_domain_on_finish	= sq_pwr_domain_on_finish,
	.cpu_standby		= sq_cpu_standby,
	.system_off		= sq_system_off,
	.system_reset		= sq_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	sq_sec_entrypoint = sec_entrypoint;
	flush_dcache_range((uint64_t)&sq_sec_entrypoint,
			   sizeof(sq_sec_entrypoint));

	*psci_ops = &sq_psci_ops;

	return 0;
}
