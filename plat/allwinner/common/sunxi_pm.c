/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_scpi.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <sunxi_cpucfg.h>
#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define SUNXI_WDOG0_CTRL_REG		(SUNXI_R_WDOG_BASE + 0x0010)
#define SUNXI_WDOG0_CFG_REG		(SUNXI_R_WDOG_BASE + 0x0014)
#define SUNXI_WDOG0_MODE_REG		(SUNXI_R_WDOG_BASE + 0x0018)

#define CPU_PWR_LVL			MPIDR_AFFLVL0
#define CLUSTER_PWR_LVL			MPIDR_AFFLVL1
#define SYSTEM_PWR_LVL			MPIDR_AFFLVL2

#define CPU_PWR_STATE(state) \
	((state)->pwr_domain_state[CPU_PWR_LVL])
#define CLUSTER_PWR_STATE(state) \
	((state)->pwr_domain_state[CLUSTER_PWR_LVL])
#define SYSTEM_PWR_STATE(state) \
	((state)->pwr_domain_state[SYSTEM_PWR_LVL])

#define mpidr_is_valid(mpidr) (plat_core_pos_by_mpidr(mpidr) >= 0)

/*
 * The addresses for the SCP exception vectors are defined in the or1k
 * architecture specification.
 */
#define OR1K_VEC_FIRST			0x01
#define OR1K_VEC_LAST			0x0e
#define OR1K_VEC_ADDR(n)		(0x100 * (n))

/*
 * This magic value is the little-endian representation of the or1k
 * instruction "l.mfspr r2, r0, 0x12", which is guaranteed to be the
 * first instruction in the SCP firmware.
 */
#define SCP_FIRMWARE_MAGIC		0xb4400012

static bool scpi_available;

static inline scpi_power_state_t scpi_map_state(plat_local_state_t psci_state)
{
	if (is_local_state_run(psci_state))
		return scpi_power_on;
	if (is_local_state_retn(psci_state))
		return scpi_power_retention;
	return scpi_power_off;
}

static void sunxi_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr = read_scr_el3();

	assert(is_local_state_retn(cpu_state));

	write_scr_el3(scr | SCR_IRQ_BIT);
	wfi();
	write_scr_el3(scr);
}

static int sunxi_pwr_domain_on(u_register_t mpidr)
{
	if (mpidr_is_valid(mpidr) == 0)
		return PSCI_E_INTERN_FAIL;

	if (scpi_available) {
		scpi_set_css_power_state(mpidr,
					 scpi_power_on,
					 scpi_power_on,
					 scpi_power_on);
	} else {
		sunxi_cpu_on(mpidr);
	}

	return PSCI_E_SUCCESS;
}

static void sunxi_pwr_domain_off(const psci_power_state_t *target_state)
{
	plat_local_state_t cpu_pwr_state     = CPU_PWR_STATE(target_state);
	plat_local_state_t cluster_pwr_state = CLUSTER_PWR_STATE(target_state);
	plat_local_state_t system_pwr_state  = SYSTEM_PWR_STATE(target_state);

	if (is_local_state_off(cpu_pwr_state))
		gicv2_cpuif_disable();

	if (scpi_available) {
		scpi_set_css_power_state(read_mpidr(),
					 scpi_map_state(cpu_pwr_state),
					 scpi_map_state(cluster_pwr_state),
					 scpi_map_state(system_pwr_state));
	}
}

static void __dead2 sunxi_pwr_down_wfi(const psci_power_state_t *target_state)
{
	sunxi_cpu_off(read_mpidr());

	while (1)
		wfi();
}

static void sunxi_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	if (is_local_state_off(SYSTEM_PWR_STATE(target_state)))
		gicv2_distif_init();
	if (is_local_state_off(CPU_PWR_STATE(target_state))) {
		gicv2_pcpu_distif_init();
		gicv2_cpuif_enable();
	}
}

static void __dead2 sunxi_system_off(void)
{
	gicv2_cpuif_disable();

	if (scpi_available) {
		/* Send the power down request to the SCP */
		uint32_t ret = scpi_sys_power_state(scpi_system_shutdown);

		if (ret != SCP_OK)
			ERROR("PSCI: SCPI %s failed: %d\n", "shutdown", ret);
	}

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
	gicv2_cpuif_disable();

	if (scpi_available) {
		/* Send the system reset request to the SCP */
		uint32_t ret = scpi_sys_power_state(scpi_system_reboot);

		if (ret != SCP_OK)
			ERROR("PSCI: SCPI %s failed: %d\n", "reboot", ret);
	}

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

static int sunxi_validate_power_state(unsigned int power_state,
				      psci_power_state_t *req_state)
{
	unsigned int power_level = psci_get_pstate_pwrlvl(power_state);
	unsigned int type = psci_get_pstate_type(power_state);

	assert(req_state != NULL);

	if (power_level > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	if (type == PSTATE_TYPE_STANDBY) {
		/* Only one retention power state is supported. */
		if (psci_get_pstate_id(power_state) > 0)
			return PSCI_E_INVALID_PARAMS;
		/* The SoC cannot be suspended without losing state */
		if (power_level == SYSTEM_PWR_LVL)
			return PSCI_E_INVALID_PARAMS;
		for (unsigned int i = 0; i <= power_level; ++i)
			req_state->pwr_domain_state[i] = PLAT_MAX_RET_STATE;
	} else {
		/* Only one off power state is supported. */
		if (psci_get_pstate_id(power_state) > 0)
			return PSCI_E_INVALID_PARAMS;
		for (unsigned int i = 0; i <= power_level; ++i)
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
	/* Higher power domain levels should all remain running */
	for (unsigned int i = power_level + 1; i <= PLAT_MAX_PWR_LVL; ++i)
		req_state->pwr_domain_state[i] = PSCI_LOCAL_STATE_RUN;

	return PSCI_E_SUCCESS;
}

static int sunxi_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entry point must be in DRAM */
	if (ns_entrypoint >= SUNXI_DRAM_BASE)
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

static void sunxi_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	assert(req_state);

	for (unsigned int i = 0; i <= PLAT_MAX_PWR_LVL; ++i)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

static int sunxi_get_node_hw_state(u_register_t mpidr,
				   unsigned int power_level)
{
	unsigned int cluster_state, cpu_state;
	unsigned int cpu = MPIDR_AFFLVL0_VAL(mpidr);

	/* SoC power level (always on if PSCI works). */
	if (power_level == SYSTEM_PWR_LVL)
		return HW_ON;
	if (scpi_get_css_power_state(mpidr, &cpu_state, &cluster_state))
		return PSCI_E_NOT_SUPPORTED;
	/* Cluster power level (full power state available). */
	if (power_level == CLUSTER_PWR_LVL) {
		if (cluster_state == scpi_power_on)
			return HW_ON;
		if (cluster_state == scpi_power_retention)
			return HW_STANDBY;
		return HW_OFF;
	}
	/* CPU power level (one bit boolean for on or off). */
	return ((cpu_state & BIT(cpu)) != 0) ? HW_ON : HW_OFF;
}

static plat_psci_ops_t sunxi_psci_ops = {
	.cpu_standby			= sunxi_cpu_standby,
	.pwr_domain_on			= sunxi_pwr_domain_on,
	.pwr_domain_off			= sunxi_pwr_domain_off,
	.pwr_domain_on_finish		= sunxi_pwr_domain_on_finish,
	.system_off			= sunxi_system_off,
	.system_reset			= sunxi_system_reset,
	.validate_power_state		= sunxi_validate_power_state,
	.validate_ns_entrypoint		= sunxi_validate_ns_entrypoint,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	assert(psci_ops);

	/* Program all CPU entry points. */
	for (unsigned int cpu = 0; cpu < PLATFORM_CORE_COUNT; ++cpu) {
		mmio_write_32(SUNXI_CPUCFG_RVBAR_LO_REG(cpu),
			      sec_entrypoint & 0xffffffff);
		mmio_write_32(SUNXI_CPUCFG_RVBAR_HI_REG(cpu),
			      sec_entrypoint >> 32);
	}

	/* Check for a valid SCP firmware, and boot the SCP if found. */
	if (mmio_read_32(SUNXI_SCP_BASE) == SCP_FIRMWARE_MAGIC) {
		/* Program SCP exception vectors to the firmware entrypoint. */
		for (unsigned int i = OR1K_VEC_FIRST; i <= OR1K_VEC_LAST; ++i) {
			uint32_t vector = SUNXI_SRAM_A2_BASE + OR1K_VEC_ADDR(i);
			uint32_t offset = SUNXI_SCP_BASE - vector;

			mmio_write_32(vector, offset >> 2);
			clean_dcache_range(vector, sizeof(uint32_t));
		}
		/* Take the SCP out of reset. */
		mmio_setbits_32(SUNXI_R_CPUCFG_BASE, BIT(0));
		/* Wait for the SCP firmware to boot. */
		if (scpi_wait_ready() == 0)
			scpi_available = true;
	}

	NOTICE("PSCI: System suspend is %s\n",
	       scpi_available ? "available via SCPI" : "unavailable");
	if (scpi_available) {
		/* Suspend is only available via SCPI. */
		sunxi_psci_ops.pwr_domain_suspend = sunxi_pwr_domain_off;
		sunxi_psci_ops.pwr_domain_suspend_finish = sunxi_pwr_domain_on_finish;
		sunxi_psci_ops.get_sys_suspend_power_state = sunxi_get_sys_suspend_power_state;
		sunxi_psci_ops.get_node_hw_state = sunxi_get_node_hw_state;
	} else {
		/* This is only needed when SCPI is unavailable. */
		sunxi_psci_ops.pwr_domain_pwr_down_wfi = sunxi_pwr_down_wfi;
	}

	*psci_ops = &sunxi_psci_ops;

	return 0;
}
