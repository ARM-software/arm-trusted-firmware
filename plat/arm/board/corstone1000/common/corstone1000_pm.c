/*
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>
#include <plat/common/platform.h>
#include <drivers/arm/gicv2.h>
/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/

static void __dead2 corstone1000_system_reset(void)
{

	uint32_t volatile * const watchdog_ctrl_reg = (uint32_t *) SECURE_WATCHDOG_ADDR_CTRL_REG;
	uint32_t volatile * const watchdog_val_reg = (uint32_t *) SECURE_WATCHDOG_ADDR_VAL_REG;

	/*
	 * Disable GIC CPU interface to prevent pending interrupt
	 * from waking up the AP from WFI.
	 */
	gicv2_cpuif_disable();

	/* Flush and invalidate data cache */
	dcsw_op_all(DCCISW);

	*(watchdog_val_reg) = SECURE_WATCHDOG_COUNTDOWN_VAL;
	*watchdog_ctrl_reg = SECURE_WATCHDOG_MASK_ENABLE;
	while (1) {
		wfi();
	}
}

#if defined(CORSTONE1000_FVP_MULTICORE)
int corstone1000_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= ARM_NS_DRAM1_BASE) && (entrypoint < (ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE))) {
		return PSCI_E_SUCCESS;
	}
	return PSCI_E_INVALID_ADDRESS;
}

int corstone1000_pwr_domain_on(u_register_t mpidr)
{
	int core_index = plat_core_pos_by_mpidr(mpidr);
	uint64_t *secondary_core_hold_base = (uint64_t *)CORSTONE1000_SECONDARY_CORE_HOLD_BASE;

	/* Validate the core index */
	if (core_index < 0 || core_index > PLATFORM_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}
	secondary_core_hold_base[core_index] = CORSTONE1000_SECONDARY_CORE_STATE_GO;
	dsbish();
	sev();

	return PSCI_E_SUCCESS;
}

void corstone1000_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	(void)target_state;
	plat_arm_gic_init();
}
#endif

plat_psci_ops_t plat_arm_psci_pm_ops = {
#if defined(CORSTONE1000_FVP_MULTICORE)
	.pwr_domain_on = corstone1000_pwr_domain_on,
	.pwr_domain_on_finish = corstone1000_pwr_domain_on_finish,
	.validate_ns_entrypoint = corstone1000_validate_ns_entrypoint,
	.system_reset = corstone1000_system_reset,
#else
	.validate_ns_entrypoint = NULL,
	.system_reset = corstone1000_system_reset,
#endif
};

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	ops = &plat_arm_psci_pm_ops;
	return ops;
}
