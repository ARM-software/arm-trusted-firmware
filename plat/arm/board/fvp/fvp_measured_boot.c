/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log.h>
#include <plat/arm/common/plat_arm.h>

/* FVP table with platform specific image IDs, names and PCRs */
static const image_data_t fvp_images_data[] = {
	{ BL2_IMAGE_ID, BL2_STRING, PCR_0 },		/* Reserved for BL2 */
	{ BL31_IMAGE_ID, BL31_STRING, PCR_0 },
	{ BL32_IMAGE_ID, BL32_STRING, PCR_0 },
	{ BL32_EXTRA1_IMAGE_ID, BL32_EXTRA1_IMAGE_STRING, PCR_0 },
	{ BL32_EXTRA2_IMAGE_ID, BL32_EXTRA2_IMAGE_STRING, PCR_0 },
	{ BL33_IMAGE_ID, BL33_STRING, PCR_0 },
	{ HW_CONFIG_ID, HW_CONFIG_STRING, PCR_0 },
	{ NT_FW_CONFIG_ID, NT_FW_CONFIG_STRING, PCR_0 },
	{ SCP_BL2_IMAGE_ID, SCP_BL2_IMAGE_STRING, PCR_0 },
	{ SOC_FW_CONFIG_ID, SOC_FW_CONFIG_STRING, PCR_0 },
	{ TOS_FW_CONFIG_ID, TOS_FW_CONFIG_STRING, PCR_0 },
	{ INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

static const measured_boot_data_t fvp_measured_boot_data = {
	fvp_images_data,
	arm_set_nt_fw_info,
	arm_set_tos_fw_info
};

/*
 * Function retuns pointer to FVP plat_measured_boot_data_t structure
 */
const measured_boot_data_t *plat_get_measured_boot_data(void)
{
	return &fvp_measured_boot_data;
}

void bl2_plat_mboot_init(void)
{
	event_log_init();
}

void bl2_plat_mboot_finish(void)
{
	uint8_t *log_addr;
	size_t log_size;
	int rc;

	rc = event_log_finalise(&log_addr, &log_size);
	if (rc != 0) {
		/*
		 * It is a fatal error because on FVP secure world software
		 * assumes that a valid event log exists and will use it to
		 * record the measurements into the fTPM
		 */
		panic();
	}

	dump_event_log(log_addr, log_size);
}
