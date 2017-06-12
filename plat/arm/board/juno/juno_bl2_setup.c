/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl_common.h>
#include <desc_image_load.h>
#include <plat_arm.h>

#if JUNO_AARCH32_EL3_RUNTIME
/*******************************************************************************
 * This function changes the spsr for BL32 image to bypass
 * the check in BL1 AArch64 exception handler. This is needed in the aarch32
 * boot flow as the core comes up in aarch64 and to enter the BL32 image a warm
 * reset in aarch32 state is required.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = arm_bl2_handle_post_image_load(image_id);

	if (!err && (image_id == BL32_IMAGE_ID)) {
		bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
		assert(bl_mem_params);
		bl_mem_params->ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS);
	}

	return err;
}

#if !CSS_USE_SCMI_SDS_DRIVER
/*
 * We need to override some of the platform functions when booting SP_MIN
 * on Juno AArch32. These needs to be done only for SCPI/BOM SCP systems as
 * in case of SDS, the structures remain in memory and doesn't need to be
 * overwritten.
 */

static unsigned int scp_boot_config;

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	arm_bl2_early_platform_setup(mem_layout);

	/* Save SCP Boot config before it gets overwritten by SCP_BL2 loading */
	VERBOSE("BL2: Saving SCP Boot config = 0x%x\n", scp_boot_config);
	scp_boot_config = mmio_read_32(SCP_BOOT_CFG_ADDR);
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

	mmio_write_32(SCP_BOOT_CFG_ADDR, scp_boot_config);
	VERBOSE("BL2: Restored SCP Boot config = 0x%x\n", scp_boot_config);
}
#endif

#endif /* JUNO_AARCH32_EL3_RUNTIME */
