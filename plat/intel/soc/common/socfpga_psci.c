/*
 * Copyright (c) 2019-2023, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>

#ifndef GICV3_SUPPORT_GIC600
#include <drivers/arm/gicv2.h>
#else
#include <drivers/arm/gicv3.h>
#endif
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>
#include "socfpga_mailbox.h"
#include "socfpga_plat_def.h"
#include "socfpga_reset_manager.h"
#include "socfpga_sip_svc.h"
#include "socfpga_system_manager.h"

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
void socfpga_wakeup_secondary_cpu(unsigned int cpu_id);
extern void plat_secondary_cold_boot_setup(void);
#endif

/*******************************************************************************
 * plat handler called when a CPU is about to enter standby.
 ******************************************************************************/
void socfpga_cpu_standby(plat_local_state_t cpu_state)
{
	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	VERBOSE("%s: cpu_state: 0x%x\n", __func__, cpu_state);
	dsb();
	wfi();
}

/*******************************************************************************
 * plat handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
int socfpga_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu_id = plat_core_pos_by_mpidr(mpidr);
#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	/* TODO: Add in CPU FUSE from SDM */
#else
	uint32_t psci_boot = 0x00;

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);
#endif

	if (cpu_id == -1)
		return PSCI_E_INTERN_FAIL;

#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
	if (cpu_id == 0x00) {
		psci_boot = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_8));
		psci_boot |= 0x20000; /* bit 17 */
		mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_8), psci_boot);
	}

	mmio_write_64(PLAT_CPUID_RELEASE, cpu_id);
#endif

	/* release core reset */
#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	bl31_plat_set_secondary_cpu_entrypoint(cpu_id);
#else
	mmio_setbits_32(SOCFPGA_RSTMGR(MPUMODRST), 1 << cpu_id);
	mmio_write_64(PLAT_CPUID_RELEASE, cpu_id);
#endif

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * plat handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void socfpga_pwr_domain_off(const psci_power_state_t *target_state)
{
	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* Prevent interrupts from spuriously waking up this cpu */
#ifdef GICV3_SUPPORT_GIC600
	gicv3_cpuif_disable(plat_my_core_pos());
#else
	gicv2_cpuif_disable();
#endif

}

/*******************************************************************************
 * plat handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void socfpga_pwr_domain_suspend(const psci_power_state_t *target_state)
{
#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
	unsigned int cpu_id = plat_my_core_pos();
#endif

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
	/* assert core reset */
	mmio_setbits_32(SOCFPGA_RSTMGR(MPUMODRST), 1 << cpu_id);
#endif
}

/*******************************************************************************
 * plat handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void socfpga_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* Enable the gic cpu interface */
#ifdef GICV3_SUPPORT_GIC600
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
#else
	/* Program the gic per-cpu distributor or re-distributor interface */
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
#endif
}

/*******************************************************************************
 * plat handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 ******************************************************************************/
void socfpga_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
	unsigned int cpu_id = plat_my_core_pos();
#endif

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
	/* release core reset */
	mmio_clrbits_32(SOCFPGA_RSTMGR(MPUMODRST), 1 << cpu_id);
#endif
}

/*******************************************************************************
 * plat handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 socfpga_system_off(void)
{
	wfi();
	ERROR("System Off: operation not handled.\n");
	panic();
}

extern uint64_t intel_rsu_update_address;

static void __dead2 socfpga_system_reset(void)
{
	uint32_t addr_buf[2];

	memcpy(addr_buf, &intel_rsu_update_address,
			sizeof(intel_rsu_update_address));
	if (intel_rsu_update_address) {
		mailbox_rsu_update(addr_buf);
	} else {
		mailbox_reset_cold();
	}

	while (1)
		wfi();
}

static int socfpga_system_reset2(int is_vendor, int reset_type,
					u_register_t cookie)
{
#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	mailbox_reset_warm(reset_type);
#else
	if (cold_reset_for_ecc_dbe()) {
		mailbox_reset_cold();
	}
#endif

	/* disable cpuif */
#ifdef GICV3_SUPPORT_GIC600
	gicv3_cpuif_disable(plat_my_core_pos());
#else
	gicv2_cpuif_disable();
#endif

	/* Store magic number */
	mmio_write_32(L2_RESET_DONE_REG, L2_RESET_DONE_STATUS);

	/* Increase timeout */
	mmio_write_32(SOCFPGA_RSTMGR(HDSKTIMEOUT), 0xffffff);

	/* Enable handshakes */
	mmio_setbits_32(SOCFPGA_RSTMGR(HDSKEN), RSTMGR_HDSKEN_SET);

#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
	/* Reset L2 module */
	mmio_setbits_32(SOCFPGA_RSTMGR(COLDMODRST), 0x100);
#endif

	while (1)
		wfi();

	/* Should not reach here */
	return 0;
}

int socfpga_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	VERBOSE("%s: power_state: 0x%x\n", __func__, power_state);

	return PSCI_E_SUCCESS;
}

int socfpga_validate_ns_entrypoint(unsigned long ns_entrypoint)
{
	VERBOSE("%s: ns_entrypoint: 0x%lx\n", __func__, ns_entrypoint);
	return PSCI_E_SUCCESS;
}

void socfpga_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	req_state->pwr_domain_state[PSCI_CPU_PWR_LVL] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[1] = PLAT_MAX_OFF_STATE;
}

/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
const plat_psci_ops_t socfpga_psci_pm_ops = {
	.cpu_standby = socfpga_cpu_standby,
	.pwr_domain_on = socfpga_pwr_domain_on,
	.pwr_domain_off = socfpga_pwr_domain_off,
	.pwr_domain_suspend = socfpga_pwr_domain_suspend,
	.pwr_domain_on_finish = socfpga_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = socfpga_pwr_domain_suspend_finish,
	.system_off = socfpga_system_off,
	.system_reset = socfpga_system_reset,
	.system_reset2 = socfpga_system_reset2,
	.validate_power_state = socfpga_validate_power_state,
	.validate_ns_entrypoint = socfpga_validate_ns_entrypoint,
	.get_sys_suspend_power_state = socfpga_get_sys_suspend_power_state
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	/* Save warm boot entrypoint.*/
	mmio_write_64(PLAT_SEC_ENTRY, sec_entrypoint);
	*psci_ops = &socfpga_psci_pm_ops;

	return 0;
}
