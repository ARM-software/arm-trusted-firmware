/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <platform.h>
#include <stddef.h>
#include "psci_private.h"

void __dead2 psci_system_off(void)
{
	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_off);

	/* Notify the Secure Payload Dispatcher */
	if (psci_spd_pm && psci_spd_pm->svc_system_off) {
		psci_spd_pm->svc_system_off();
	}

	console_flush();

	/* Call the platform specific hook */
	psci_plat_pm_ops->system_off();

	/* This function does not return. We should never get here */
}

void __dead2 psci_system_reset(void)
{
	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_reset);

	/* Notify the Secure Payload Dispatcher */
	if (psci_spd_pm && psci_spd_pm->svc_system_reset) {
		psci_spd_pm->svc_system_reset();
	}

	console_flush();

	/* Call the platform specific hook */
	psci_plat_pm_ops->system_reset();

	/* This function does not return. We should never get here */
}

int psci_system_reset2(uint32_t reset_type, u_register_t cookie)
{
	int is_vendor;

	psci_print_power_domain_map();

	assert(psci_plat_pm_ops->system_reset2);

	is_vendor = (reset_type >> PSCI_RESET2_TYPE_VENDOR_SHIFT) & 1;
	if (!is_vendor) {
		/*
		 * Only WARM_RESET is allowed for architectural type resets.
		 */
		if (reset_type != PSCI_RESET2_SYSTEM_WARM_RESET)
			return PSCI_E_INVALID_PARAMS;
		if (psci_plat_pm_ops->write_mem_protect &&
		    psci_plat_pm_ops->write_mem_protect(0) < 0) {
			return PSCI_E_NOT_SUPPORTED;
		}
	}

	/* Notify the Secure Payload Dispatcher */
	if (psci_spd_pm && psci_spd_pm->svc_system_reset) {
		psci_spd_pm->svc_system_reset();
	}
	console_flush();

	return psci_plat_pm_ops->system_reset2(is_vendor, reset_type, cookie);
}
