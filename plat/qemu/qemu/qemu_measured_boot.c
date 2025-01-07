/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2022-2023, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/metadata.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <tools_share/tbbr_oid.h>

#include "../common/qemu_private.h"

/* Event Log data */
static uint8_t event_log[PLAT_EVENT_LOG_MAX_SIZE];
static uint64_t event_log_base;

/* QEMU table with platform specific image IDs, names and PCRs */
static const event_log_metadata_t qemu_event_log_metadata[] = {
	{ BL31_IMAGE_ID, MBOOT_BL31_IMAGE_STRING, PCR_0 },
	{ BL32_IMAGE_ID, MBOOT_BL32_IMAGE_STRING, PCR_0 },
	{ BL32_EXTRA1_IMAGE_ID, MBOOT_BL32_EXTRA1_IMAGE_STRING, PCR_0 },
	{ BL32_EXTRA2_IMAGE_ID, MBOOT_BL32_EXTRA2_IMAGE_STRING, PCR_0 },
	{ BL33_IMAGE_ID, MBOOT_BL33_IMAGE_STRING, PCR_0 },
	{ HW_CONFIG_ID, MBOOT_HW_CONFIG_STRING, PCR_0 },
	{ NT_FW_CONFIG_ID, MBOOT_NT_FW_CONFIG_STRING, PCR_0 },
	{ SCP_BL2_IMAGE_ID, MBOOT_SCP_BL2_IMAGE_STRING, PCR_0 },
	{ SOC_FW_CONFIG_ID, MBOOT_SOC_FW_CONFIG_STRING, PCR_0 },
	{ TOS_FW_CONFIG_ID, MBOOT_TOS_FW_CONFIG_STRING, PCR_0 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

void bl2_plat_mboot_init(void)
{
	/*
	 * Here we assume that BL1/ROM code doesn't have the driver
	 * to measure the BL2 code which is a common case for
	 * already existing platforms
	 */
	event_log_init(event_log, event_log + sizeof(event_log));
	event_log_write_header();

	/*
	 * TBD - Add code to do self measurement of BL2 code and add an
	 * event for BL2 measurement
	 */

	event_log_base = (uintptr_t)event_log;
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	/* Event Log address in Non-Secure memory */
	uintptr_t ns_log_addr;

	/* Event Log filled size */
	size_t event_log_cur_size;

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_base);

	dump_event_log((uint8_t *)event_log_base, event_log_cur_size);

#if TRANSFER_LIST
	if (!plat_handoff_mboot((void *)event_log_base, event_log_cur_size,
				(void *)(uintptr_t)FW_HANDOFF_BASE))
		return;
#endif

	rc = qemu_set_nt_fw_info(
#ifdef SPD_opteed
			    (uintptr_t)event_log_base,
#endif
			    event_log_cur_size, &ns_log_addr);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "NT");
		/*
		 * It is a fatal error because on QEMU secure world software
		 * assumes that a valid event log exists and will use it to
		 * record the measurements into the fTPM or sw-tpm.
		 * Note: In QEMU platform, OP-TEE uses nt_fw_config to get the
		 * secure Event Log buffer address.
		 */
		panic();
	}

	/* Copy Event Log to Non-secure memory */
	(void)memcpy((void *)ns_log_addr, (const void *)event_log_base,
		     event_log_cur_size);

	/* Ensure that the Event Log is visible in Non-secure memory */
	flush_dcache_range(ns_log_addr, event_log_cur_size);

#if defined(SPD_tspd) || defined(SPD_spmd)
	/* Set Event Log data in TOS_FW_CONFIG */
	rc = qemu_set_tos_fw_info((uintptr_t)event_log_base,
				 event_log_cur_size);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "TOS");
		panic();
	}
#endif /* defined(SPD_tspd) || defined(SPD_spmd) */

}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	/* Calculate image hash and record data in Event Log */
	int err = event_log_measure_and_record(image_data->image_base,
					       image_data->image_size,
					       image_id,
					       qemu_event_log_metadata);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record", image_id, err);
		return err;
	}

	return 0;
}

int plat_mboot_measure_key(const void *pk_oid, const void *pk_ptr,
			   size_t pk_len)
{
	return 0;
}
