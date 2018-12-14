/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <limits.h>

#include <lib/utils.h>

#include "psci_private.h"

u_register_t psci_mem_protect(unsigned int enable)
{
	int val;

	assert(psci_plat_pm_ops->read_mem_protect != NULL);
	assert(psci_plat_pm_ops->write_mem_protect != NULL);

	if (psci_plat_pm_ops->read_mem_protect(&val) < 0)
		return (u_register_t) PSCI_E_NOT_SUPPORTED;
	if (psci_plat_pm_ops->write_mem_protect(enable) < 0)
		return (u_register_t) PSCI_E_NOT_SUPPORTED;

	return (val != 0) ? 1U : 0U;
}

u_register_t psci_mem_chk_range(uintptr_t base, u_register_t length)
{
	int ret;

	assert(psci_plat_pm_ops->mem_protect_chk != NULL);

	if ((length == 0U) || check_uptr_overflow(base, length - 1U))
		return (u_register_t) PSCI_E_DENIED;

	ret = psci_plat_pm_ops->mem_protect_chk(base, length);
	return (ret < 0) ?
	       (u_register_t) PSCI_E_DENIED : (u_register_t) PSCI_E_SUCCESS;
}
