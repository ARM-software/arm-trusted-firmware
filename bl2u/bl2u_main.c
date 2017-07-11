/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <auth_mod.h>
#include <bl1.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>

/*******************************************************************************
 * This function is responsible to:
 * Load SCP_BL2U if platform has defined SCP_BL2U_BASE
 * Perform platform setup.
 * Go back to EL3.
 ******************************************************************************/
void bl2u_main(void)
{
	NOTICE("BL2U: %s\n", version_string);
	NOTICE("BL2U: %s\n", build_message);

#if SCP_BL2U_BASE
	int rc;
	/* Load the subsequent bootloader images */
	rc = bl2u_plat_handle_scp_bl2u();
	if (rc) {
		ERROR("Failed to load SCP_BL2U (%i)\n", rc);
		panic();
	}
#endif

	/* Perform platform setup in BL2U after loading SCP_BL2U */
	bl2u_platform_setup();

	console_flush();

#ifdef AARCH32
	/*
	 * For AArch32 state BL1 and BL2U share the MMU setup.
	 * Given that BL2U does not map BL1 regions, MMU needs
	 * to be disabled in order to go back to BL1.
	 */
	disable_mmu_icache_secure();
#endif /* AARCH32 */

	/*
	 * Indicate that BL2U is done and resume back to
	 * normal world via an SMC to BL1.
	 * x1 could be passed to Normal world,
	 * so DO NOT pass any secret information.
	 */
	smc(FWU_SMC_SEC_IMAGE_DONE, 0, 0, 0, 0, 0, 0, 0);
	wfi();
}
