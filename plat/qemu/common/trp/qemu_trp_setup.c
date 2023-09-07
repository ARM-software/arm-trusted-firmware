/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <platform_def.h>
#include <services/rmm_core_manifest.h>
#include <services/rmmd_svc.h>
#include <services/trp/platform_trp.h>
#include <trp_helpers.h>

#include "../qemu_private.h"

/*******************************************************************************
 * Received from boot manifest and populated here
 ******************************************************************************/
extern uint32_t trp_boot_manifest_version;

static int qemu_trp_process_manifest(struct rmm_manifest *manifest)
{
	/* padding field on the manifest must be RES0 */
	assert(manifest->padding == 0U);

	/* Verify the Boot Manifest Version. Only the Major is considered */
	if (RMMD_MANIFEST_VERSION_MAJOR !=
		RMMD_GET_MANIFEST_VERSION_MAJOR(manifest->version)) {
		return E_RMM_BOOT_MANIFEST_VERSION_NOT_SUPPORTED;
	}

	trp_boot_manifest_version = manifest->version;
	flush_dcache_range((uintptr_t)manifest, sizeof(struct rmm_manifest));

	return 0;
}

void trp_early_platform_setup(struct rmm_manifest *manifest)
{
	int rc;

	rc = qemu_trp_process_manifest(manifest);
	if (rc != 0) {
		trp_boot_abort(rc);
	}

	qemu_console_init();
}
