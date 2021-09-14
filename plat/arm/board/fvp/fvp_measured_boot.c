/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>
#include <plat/arm/common/plat_arm.h>

/* Event Log data */
static uint8_t event_log[PLAT_ARM_EVENT_LOG_MAX_SIZE];

/* FVP table with platform specific image IDs, names and PCRs */
const event_log_metadata_t fvp_event_log_metadata[] = {
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

const event_log_metadata_t *plat_event_log_get_metadata(void)
{
	return fvp_event_log_metadata;
}

void bl2_plat_mboot_init(void)
{
	event_log_init(event_log, event_log + sizeof(event_log));
	event_log_write_header();
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	/* Event Log address in Non-Secure memory */
	uintptr_t ns_log_addr;

	/* Event Log filled size */
	size_t event_log_cur_size;

	event_log_cur_size = event_log_get_cur_size(event_log);

	rc = arm_set_nt_fw_info(
#ifdef SPD_opteed
			    (uintptr_t)event_log,
#endif
			    event_log_cur_size, &ns_log_addr);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "NT");
		/*
		 * It is a fatal error because on FVP secure world software
		 * assumes that a valid event log exists and will use it to
		 * record the measurements into the fTPM.
		 * Note: In FVP platform, OP-TEE uses nt_fw_config to get the
		 * secure Event Log buffer address.
		 */
		panic();
	}

	/* Copy Event Log to Non-secure memory */
	(void)memcpy((void *)ns_log_addr, (const void *)event_log,
		     event_log_cur_size);

	/* Ensure that the Event Log is visible in Non-secure memory */
	flush_dcache_range(ns_log_addr, event_log_cur_size);

#if defined(SPD_tspd) || defined(SPD_spmd)
	/* Set Event Log data in TOS_FW_CONFIG */
	rc = arm_set_tos_fw_info((uintptr_t)event_log,
				 event_log_cur_size);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "TOS");
		panic();
	}
#endif

	dump_event_log(event_log, event_log_cur_size);
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
