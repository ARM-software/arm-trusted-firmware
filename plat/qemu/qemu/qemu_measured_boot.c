/*
 * Copyright (c) 2022-2025, Arm Limited. All rights reserved.
 * Copyright (c) 2022-2023, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <plat/common/common_def.h>
#include <plat/common/platform.h>

#include <common/debug.h>
#include <common/measured_boot.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/metadata.h>
#include <event_measure.h>
#include <event_print.h>
#include <tcg.h>
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
	int rc;
	tpm_alg_id algos[] = {
#ifdef TPM_ALG_ID
		TPM_ALG_ID,
#else
		/*
		 * TODO: with MEASURED_BOOT=1 several algorithms are now compiled into
		 * Mbed-TLS, we ought to query the backend to figure out what algorithms
		 * to use.
		 */
		TPM_ALG_SHA256,
		TPM_ALG_SHA384,
		TPM_ALG_SHA512,
#endif
	};

	/*
	 * Here we assume that BL1/ROM code doesn't have the driver
	 * to measure the BL2 code which is a common case for
	 * already existing platforms
	 */
	rc = event_log_init_and_reg(event_log, event_log + sizeof(event_log),
				    0U, crypto_mod_tcg_hash);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}

	rc = event_log_write_header(algos, ARRAY_SIZE(algos), 0, NULL, 0);
	if (rc < 0) {
		ERROR("Failed to write event log header (%d).\n", rc);
		panic();
	}

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

	event_log_dump((uint8_t *)event_log_base, event_log_cur_size);

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
		 * It is a non-fatal error because on QEMU secure world software
		 * assumes that a valid event log exists and will use it to
		 * record the measurements into the fTPM or sw-tpm, but the boot
		 * can also happen without TPM on the platform. It's up to
		 * higher layer in boot sequence to decide if this is fatal or
		 * not, e.g. by not providing access to TPM encrypted storage.
		 * Note: In QEMU platform, OP-TEE uses nt_fw_config to get the
		 * secure Event Log buffer address.
		 */
		WARN("Ignoring TPM errors, continuing without\n");
		return;
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
	const event_log_metadata_t *metadata_ptr;
	int err;

	metadata_ptr = mboot_find_event_log_metadata(qemu_event_log_metadata,
						     image_id);
	if (metadata_ptr == NULL) {
		ERROR("Unable to find metadata for image %u.\n", image_id);
		return -1;
	}

	/* Calculate image hash and record data in Event Log */
	err = event_log_measure_and_record(metadata_ptr->pcr,
					   image_data->image_base,
					   image_data->image_size,
					   metadata_ptr->name,
					   strlen(metadata_ptr->name) + 1U);
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
