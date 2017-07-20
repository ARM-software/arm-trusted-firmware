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
