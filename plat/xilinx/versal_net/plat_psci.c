/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <plat_arm.h>

#include <plat_private.h>

static uintptr_t versal_net_sec_entry;

static const struct plat_psci_ops versal_net_nopmc_psci_ops = {
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int32_t plat_setup_psci_ops(uintptr_t sec_entrypoint,
			    const struct plat_psci_ops **psci_ops)
{
	versal_net_sec_entry = sec_entrypoint;

	VERBOSE("Setting up entry point %lx\n", versal_net_sec_entry);

	*psci_ops = &versal_net_nopmc_psci_ops;

	return 0;
}
