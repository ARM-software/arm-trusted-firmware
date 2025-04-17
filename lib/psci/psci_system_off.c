/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gic.h>
#include <drivers/console.h>
#include <plat/common/platform.h>

#include "psci_private.h"

void __dead2 psci_system_off(void)
{
	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_off != NULL);

	/* Notify the Secure Payload Dispatcher */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_system_off != NULL)) {
		psci_spd_pm->svc_system_off();
	}

	console_flush();

#if USE_GIC_DRIVER
	/* turn the GIC off before we hand off to the platform */
	gic_cpuif_disable(plat_my_core_pos());
#endif /* USE_GIC_DRIVER */

	/* Call the platform specific hook */
	psci_plat_pm_ops->system_off();

	psci_pwrdown_cpu_end_terminal();
}

void __dead2 psci_system_reset(void)
{
	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_reset != NULL);

	/* Notify the Secure Payload Dispatcher */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_system_reset != NULL)) {
		psci_spd_pm->svc_system_reset();
	}

	console_flush();

#if USE_GIC_DRIVER
	/* turn the GIC off before we hand off to the platform */
	gic_cpuif_disable(plat_my_core_pos());
#endif /* USE_GIC_DRIVER */

	/* Call the platform specific hook */
	psci_plat_pm_ops->system_reset();

	psci_pwrdown_cpu_end_terminal();
}

u_register_t psci_system_reset2(uint32_t reset_type, u_register_t cookie)
{
	unsigned int is_vendor;

	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_reset2 != NULL);

	is_vendor = (reset_type >> PSCI_RESET2_TYPE_VENDOR_SHIFT) & 1U;
	if (is_vendor == 0U) {
		/*
		 * Only WARM_RESET is allowed for architectural type resets.
		 */
		if (reset_type != PSCI_RESET2_SYSTEM_WARM_RESET) {
			return (u_register_t) PSCI_E_INVALID_PARAMS;
		}
		if ((psci_plat_pm_ops->write_mem_protect != NULL) &&
		    (psci_plat_pm_ops->write_mem_protect(0) < 0)) {
			return (u_register_t) PSCI_E_NOT_SUPPORTED;
		}
	}

	/* Notify the Secure Payload Dispatcher */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_system_reset != NULL)) {
		psci_spd_pm->svc_system_reset();
	}
	console_flush();

#if USE_GIC_DRIVER
	/* turn the GIC off before we hand off to the platform */
	gic_cpuif_disable(plat_my_core_pos());
#endif /* USE_GIC_DRIVER */

	u_register_t ret =
		(u_register_t) psci_plat_pm_ops->system_reset2((int) is_vendor, reset_type, cookie);
	if (ret != PSCI_E_SUCCESS)
		return ret;

	psci_pwrdown_cpu_end_terminal();
}
