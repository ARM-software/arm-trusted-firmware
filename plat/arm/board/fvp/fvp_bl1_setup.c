/*
 * Copyright (c) 2013-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <bl1/bl1.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/smmu_v3.h>
#include <drivers/arm/sp805.h>
#include <lib/mmio.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/common/arm_def.h>
#include <plat/common/platform.h>
#include "fvp_private.h"

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	arm_bl1_early_platform_setup();

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();
	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	fvp_interconnect_enable();
}

void plat_arm_secure_wdt_start(void)
{
	sp805_start(ARM_SP805_TWDG_BASE, ARM_TWDG_LOAD_VAL);
}

void plat_arm_secure_wdt_stop(void)
{
	sp805_stop(ARM_SP805_TWDG_BASE);
}

void bl1_platform_setup(void)
{
	arm_bl1_platform_setup();

	/* Initialize System level generic or SP804 timer */
	fvp_timer_init();

	/* On FVP RevC, initialize SMMUv3 */
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) != 0U)
		smmuv3_security_init(PLAT_FVP_SMMUV3_BASE);
}

__dead2 void bl1_plat_fwu_done(void *client_cookie, void *reserved)
{
	uint32_t nv_flags = mmio_read_32(V2M_SYS_NVFLAGS_ADDR);

	/* Clear the NV flags register. */
	mmio_write_32((V2M_SYSREGS_BASE + V2M_SYS_NVFLAGSCLR),
		      nv_flags);

	/* Setup the watchdog to reset the system as soon as possible */
	sp805_refresh(ARM_SP805_TWDG_BASE, 1U);

	while (true)
		wfi();
}

/*******************************************************************************
 * The following function checks if Firmware update is needed by checking error
 * reported in NV flag.
 ******************************************************************************/
bool plat_arm_bl1_fwu_needed(void)
{
	int32_t nv_flags = (int32_t)mmio_read_32(V2M_SYS_NVFLAGS_ADDR);

	/* if image load/authentication failed */
	return ((nv_flags == -EAUTH) || (nv_flags == -ENOENT));
}
