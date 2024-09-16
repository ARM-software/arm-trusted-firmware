/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <platform_def.h>

static uintptr_t stm32_sec_entrypoint;

static void stm32_cpu_standby(plat_local_state_t cpu_state)
{
}

static int stm32_pwr_domain_on(u_register_t mpidr)
{
	return PSCI_E_INTERN_FAIL;
}

static void stm32_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* Nothing to do */
}

static void stm32_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/* Nothing to do, power domain is not disabled */
}

static void stm32_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
}

/*******************************************************************************
 * STM32MP2 handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
static void stm32_pwr_domain_suspend_finish(const psci_power_state_t
					    *target_state)
{
	/* Nothing to do, power domain is not disabled */
}

static void __dead2 stm32_pwr_domain_pwr_down_wfi(const psci_power_state_t
						  *target_state)
{
	ERROR("stm32mp2 Power Down WFI: operation not handled.\n");
	panic();
}

static void __dead2 stm32_system_off(void)
{
	ERROR("stm32mp2 System Off: operation not handled.\n");
	panic();
}

static void __dead2 stm32_system_reset(void)
{
	stm32mp_system_reset();
}

static int stm32_validate_power_state(unsigned int power_state,
				      psci_power_state_t *req_state)
{
	return PSCI_E_INVALID_PARAMS;
}

static int stm32_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/* The non-secure entry point must be in DDR */
	if (entrypoint < STM32MP_DDR_BASE) {
		return PSCI_E_INVALID_ADDRESS;
	}

	return PSCI_E_SUCCESS;
}

static void stm32_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
}

/*******************************************************************************
 * Export the platform handlers. The ARM Standard platform layer will take care
 * of registering the handlers with PSCI.
 ******************************************************************************/
static const plat_psci_ops_t stm32_psci_ops = {
	.cpu_standby = stm32_cpu_standby,
	.pwr_domain_on = stm32_pwr_domain_on,
	.pwr_domain_off = stm32_pwr_domain_off,
	.pwr_domain_suspend = stm32_pwr_domain_suspend,
	.pwr_domain_on_finish = stm32_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = stm32_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = stm32_pwr_domain_pwr_down_wfi,
	.system_off = stm32_system_off,
	.system_reset = stm32_system_reset,
	.validate_power_state = stm32_validate_power_state,
	.validate_ns_entrypoint = stm32_validate_ns_entrypoint,
	.get_sys_suspend_power_state = stm32_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	stm32_sec_entrypoint = sec_entrypoint;
	*psci_ops = &stm32_psci_ops;

	return 0;
}
