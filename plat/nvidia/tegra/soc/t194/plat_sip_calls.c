/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
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

extern bool tegra_fake_system_suspend;

/*******************************************************************************
 * Tegra194 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND		0xC2FFFE03U

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

	(void)x1;
	(void)x4;
	(void)cookie;
	(void)flags;

	if (smc_fid == TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND) {
		/*
		 * System suspend mode is set if the platform ATF is
		 * running on VDK and there is a debug SIP call. This mode
		 * ensures that the debug path is exercised, instead of
		 * regular code path to suit the pre-silicon platform needs.
		 * This includes replacing the call to WFI, with calls to
		 * system suspend exit procedures.
		 */
		if (tegra_platform_is_virt_dev_kit()) {
			tegra_fake_system_suspend = true;
			ret = 0;
		}
	}

	return ret;
}
