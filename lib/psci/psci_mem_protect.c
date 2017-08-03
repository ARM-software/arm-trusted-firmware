/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <limits.h>
#include <utils.h>
#include "psci_private.h"

int psci_mem_protect(unsigned int enable)
{
	int val;

	assert(psci_plat_pm_ops->read_mem_protect);
	assert(psci_plat_pm_ops->write_mem_protect);

	if (psci_plat_pm_ops->read_mem_protect(&val) < 0)
		return PSCI_E_NOT_SUPPORTED;
	if (psci_plat_pm_ops->write_mem_protect(enable) < 0)
		return PSCI_E_NOT_SUPPORTED;

	return val != 0;
}

int psci_mem_chk_range(uintptr_t base, u_register_t length)
{
	int ret;

	assert(psci_plat_pm_ops->mem_protect_chk);

	if (length == 0 || check_uptr_overflow(base, length-1))
		return PSCI_E_DENIED;

	ret = psci_plat_pm_ops->mem_protect_chk(base, length);
	return (ret < 0) ? PSCI_E_DENIED : PSCI_E_SUCCESS;
}
