/*
* Copyright (c) 2019, Arm Limited. All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>

/*******************************************************************************
 * Export the platform handlers via fvp_ve_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
plat_psci_ops_t fvp_ve_psci_pm_ops = {
	/* dummy struct */
	.validate_ns_entrypoint = NULL,
};

int __init plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &fvp_ve_psci_pm_ops;

	return 0;
}
