/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_scp.h>
#include <drivers/arm/css/sds.h>
#include <drivers/delay_timer.h>
#include <plat/common/platform.h>
#include <platform_def.h>

int css_scp_boot_image_xfer(void *image, unsigned int image_size)
{
	int ret;
	unsigned int image_offset, image_flags;

	ret = sds_init();
	if (ret != SDS_OK) {
		ERROR("SCP SDS initialization failed\n");
		panic();
	}

	VERBOSE("Writing SCP image metadata\n");
	image_offset = (uintptr_t) image - ARM_TRUSTED_SRAM_BASE;
	ret = sds_struct_write(SDS_SCP_IMG_STRUCT_ID, SDS_SCP_IMG_ADDR_OFFSET,
			&image_offset, SDS_SCP_IMG_ADDR_SIZE,
			SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK)
		goto sds_fail;

	ret = sds_struct_write(SDS_SCP_IMG_STRUCT_ID, SDS_SCP_IMG_SIZE_OFFSET,
			&image_size, SDS_SCP_IMG_SIZE_SIZE,
			SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK)
		goto sds_fail;

	VERBOSE("Marking SCP image metadata as valid\n");
	image_flags = SDS_SCP_IMG_VALID_FLAG_BIT;
	ret = sds_struct_write(SDS_SCP_IMG_STRUCT_ID, SDS_SCP_IMG_FLAG_OFFSET,
			&image_flags, SDS_SCP_IMG_FLAG_SIZE,
			SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK)
		goto sds_fail;

	return 0;
sds_fail:
	ERROR("SCP SDS write to SCP IMG struct failed\n");
	panic();
}

/*
 * API to wait for SCP to signal till it's ready after booting the transferred
 * image.
 */
int css_scp_boot_ready(void)
{
	uint32_t scp_feature_availability_flags;
	int ret, retry = CSS_SCP_READY_10US_RETRIES;


	VERBOSE("Waiting for SCP RAM to complete its initialization process\n");

	/* Wait for the SCP RAM Firmware to complete its initialization process */
	while (retry > 0) {
		ret = sds_struct_read(SDS_FEATURE_AVAIL_STRUCT_ID, 0,
				&scp_feature_availability_flags,
				SDS_FEATURE_AVAIL_SIZE,
				SDS_ACCESS_MODE_NON_CACHED);
		if (ret == SDS_ERR_STRUCT_NOT_FINALIZED)
			continue;

		if (ret != SDS_OK) {
			ERROR(" sds_struct_read failed\n");
			panic();
		}

		if (scp_feature_availability_flags &
				SDS_FEATURE_AVAIL_SCP_RAM_READY_BIT)
			return 0;

		udelay(10);
		retry--;
	}

	ERROR("Timeout of %d ms expired waiting for SCP RAM Ready flag\n",
			CSS_SCP_READY_10US_RETRIES/100);

	plat_panic_handler();
}
