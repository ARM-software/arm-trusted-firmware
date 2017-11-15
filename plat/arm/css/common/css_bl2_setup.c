/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <css_def.h>
#include <debug.h>
#include <mmio.h>
#include <plat_arm.h>
#include <string.h>
#include <utils.h>
#include "../drivers/scp/css_scp.h"

/* Weak definition may be overridden in specific CSS based platform */
#if LOAD_IMAGE_V2
#pragma weak plat_arm_bl2_handle_scp_bl2
#else
#pragma weak bl2_plat_handle_scp_bl2
#endif

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
#if LOAD_IMAGE_V2
int plat_arm_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
#else
int bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info)
#endif
{
	int ret;

	INFO("BL2: Initiating SCP_BL2 transfer to SCP\n");

	ret = css_scp_boot_image_xfer((void *)scp_bl2_image_info->image_base,
		scp_bl2_image_info->image_size);

	if (ret == 0)
		ret = css_scp_boot_ready();

	if (ret == 0)
		INFO("BL2: SCP_BL2 transferred to SCP\n");
	else
		ERROR("BL2: SCP_BL2 transfer failure\n");

	return ret;
}

#if !CSS_USE_SCMI_SDS_DRIVER
# if defined(EL3_PAYLOAD_BASE) || JUNO_AARCH32_EL3_RUNTIME

/*
 * We need to override some of the platform functions when booting an EL3
 * payload or SP_MIN on Juno AArch32. This needs to be done only for
 * SCPI/BOM SCP systems as in case of SDS, the structures remain in memory and
 * don't need to be overwritten.
 */

static unsigned int scp_boot_config;

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	arm_bl2_early_platform_setup(mem_layout);

	/* Save SCP Boot config before it gets overwritten by SCP_BL2 loading */
	scp_boot_config = mmio_read_32(SCP_BOOT_CFG_ADDR);
	VERBOSE("BL2: Saved SCP Boot config = 0x%x\n", scp_boot_config);
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

	/*
	 * Before releasing the AP cores out of reset, the SCP writes some data
	 * at the beginning of the Trusted SRAM. It is is overwritten before
	 * reaching this function. We need to restore this data, as if the
	 * target had just come out of reset. This implies:
	 *  - zeroing the first 128 bytes of Trusted SRAM using zeromem instead
	 *    of zero_normalmem since this is device memory.
	 *  - restoring the SCP boot configuration.
	 */
	VERBOSE("BL2: Restoring SCP reset data in Trusted SRAM\n");
	zeromem((void *) ARM_SHARED_RAM_BASE, 128);
	mmio_write_32(SCP_BOOT_CFG_ADDR, scp_boot_config);
}

# endif /* EL3_PAYLOAD_BASE */

#endif /* CSS_USE_SCMI_SDS_DRIVER */
