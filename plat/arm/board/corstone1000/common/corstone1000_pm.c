/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>
/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/

static void __dead2 corstone1000_system_reset(void)
{

	uint32_t volatile * const watchdog_ctrl_reg = (uint32_t *) SECURE_WATCHDOG_ADDR_CTRL_REG;
	uint32_t volatile * const watchdog_val_reg = (uint32_t *) SECURE_WATCHDOG_ADDR_VAL_REG;

	*(watchdog_val_reg) = SECURE_WATCHDOG_COUNTDOWN_VAL;
	*watchdog_ctrl_reg = SECURE_WATCHDOG_MASK_ENABLE;
	while (1) {
		wfi();
	}
}

plat_psci_ops_t plat_arm_psci_pm_ops = {
	.system_reset = corstone1000_system_reset,
	.validate_ns_entrypoint = NULL
};

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	ops = &plat_arm_psci_pm_ops;
	return ops;
}
