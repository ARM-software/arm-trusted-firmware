/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/desc_image_load.h>
#include <drivers/measured_boot/event_log/event_log.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

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
	uint8_t bl2_hash[TCG_DIGEST_SIZE];

	event_log_init();

	/* Get BL2 hash from DTB */
	/* TODO: Avoid the extra copy of the hash buffer */
	bl2_plat_get_hash(bl2_hash);

	/* Add BL2 event */
	event_log_record(bl2_hash, &fvp_images_data[0]);
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

int plat_mboot_measure_image(unsigned int image_id)
{
	const bl_mem_params_node_t *bl_mem_params =
		get_bl_mem_params_node(image_id);

	assert(bl_mem_params != NULL);

	image_info_t info = bl_mem_params->image_info;
	int err;

	if ((info.h.attr & IMAGE_ATTRIB_SKIP_LOADING) == 0U) {
		/* Calculate image hash and record data in Event Log */
		err = event_log_measure_record(info.image_base,
					       info.image_size, image_id);
		if (err != 0) {
			ERROR("%s%s image id %u (%i)\n",
			      "BL2: Failed to ", "record", image_id, err);
			return err;
		}
	}

	return 0;
}
