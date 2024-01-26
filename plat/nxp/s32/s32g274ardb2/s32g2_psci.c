/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>
#include <plat/common/platform.h>

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	static const plat_psci_ops_t s32g2_psci_ops = {
	};

	*psci_ops = &s32g2_psci_ops;

	return 0;
}

