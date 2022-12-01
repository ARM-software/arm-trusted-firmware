/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <services/rmm_core_manifest.h>
#include <services/rmmd_svc.h>
#include <services/trp/platform_trp.h>
#include <trp_helpers.h>

#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*******************************************************************************
 * Received from boot manifest and populated here
 ******************************************************************************/
extern uint32_t trp_boot_manifest_version;

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
static console_t arm_trp_runtime_console;

static int arm_trp_process_manifest(rmm_manifest_t *manifest)
{
	/* padding field on the manifest must be RES0 */
	assert(manifest->padding == 0U);

	/* Verify the Boot Manifest Version. Only the Major is considered */
	if (RMMD_MANIFEST_VERSION_MAJOR !=
		RMMD_GET_MANIFEST_VERSION_MAJOR(manifest->version)) {
		return E_RMM_BOOT_MANIFEST_VERSION_NOT_SUPPORTED;
	}

	trp_boot_manifest_version = manifest->version;
	flush_dcache_range((uintptr_t)manifest, sizeof(rmm_manifest_t));

	return 0;
}

void arm_trp_early_platform_setup(rmm_manifest_t *manifest)
{
	int rc;

	rc = arm_trp_process_manifest(manifest);
	if (rc != 0) {
		trp_boot_abort(rc);
	}

	/*
	 * Initialize a different console than already in use to display
	 * messages from trp
	 */
	rc = console_pl011_register(PLAT_ARM_TRP_UART_BASE,
				    PLAT_ARM_TRP_UART_CLK_IN_HZ,
				    ARM_CONSOLE_BAUDRATE,
				    &arm_trp_runtime_console);
	if (rc == 0) {
		panic();
	}

	console_set_scope(&arm_trp_runtime_console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);

}

void trp_early_platform_setup(rmm_manifest_t *manifest)
{
	arm_trp_early_platform_setup(manifest);
}
