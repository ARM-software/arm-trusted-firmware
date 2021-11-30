/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <services/spm_core_manifest.h>

#include <plat/common/platform.h>
#include <platform_def.h>

int plat_spm_core_manifest_load(spmc_manifest_attribute_t *manifest,
				const void *pm_addr)
{
	entry_point_info_t *ep_info = bl31_plat_get_next_image_ep_info(SECURE);

	assert(ep_info != NULL);
	assert(manifest != NULL);

	manifest->major_version = 1;
	manifest->minor_version = 0;
	manifest->exec_state = ep_info->args.arg2;
	manifest->load_address = BL32_BASE;
	manifest->entrypoint = BL32_BASE;
	manifest->binary_size = BL32_LIMIT - BL32_BASE;
	manifest->spmc_id = 0x8000;

	return 0;
}
