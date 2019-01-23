/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/css_scp.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Weak definition may be overridden in specific CSS based platform */
#pragma weak bl2u_plat_handle_scp_bl2u

/* Data structure which holds the SCP_BL2U image info for BL2U */
static image_info_t scp_bl2u_image_info;

/*******************************************************************************
 * BL1 can pass platform dependent information to BL2U in x1.
 * In case of ARM CSS platforms x1 contains SCP_BL2U image info.
 * In case of ARM FVP platforms x1 is not used.
 * In both cases, x0 contains the extents of the memory available to BL2U
 ******************************************************************************/
void bl2u_early_platform_setup(meminfo_t *mem_layout, void *plat_info)
{
	if (!plat_info)
		panic();

	arm_bl2u_early_platform_setup(mem_layout, plat_info);

	scp_bl2u_image_info = *(image_info_t *)plat_info;
}

/*******************************************************************************
 * Transfer SCP_BL2U from Trusted RAM using the SCP Download protocol.
 ******************************************************************************/
int bl2u_plat_handle_scp_bl2u(void)
{
	int ret;

	INFO("BL2U: Initiating SCP_BL2U transfer to SCP\n");

	ret = css_scp_boot_image_xfer((void *)scp_bl2u_image_info.image_base,
		scp_bl2u_image_info.image_size);

	if (ret == 0)
		ret = css_scp_boot_ready();

	if (ret == 0)
		INFO("BL2U: SCP_BL2U transferred to SCP\n");
	else
		ERROR("BL2U: SCP_BL2U transfer failure\n");

	return ret;
}
