/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <errno.h>
#include <mce.h>
#include <memctrl.h>
#include <common/runtime_svc.h>
#include <tegra_private.h>
#include <tegra_platform.h>
#include <stdbool.h>

/*******************************************************************************
 * Tegra194 SiP SMCs
 ******************************************************************************/

/*******************************************************************************
 * This function is responsible for handling all T194 SiP calls
 ******************************************************************************/
int32_t plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     const void *cookie,
		     void *handle,
		     uint64_t flags)
{
	int32_t ret = -ENOTSUP;

	(void)smc_fid;
	(void)x1;
	(void)x4;
	(void)cookie;
	(void)flags;

	return ret;
}
