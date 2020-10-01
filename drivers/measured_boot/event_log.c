/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <arch_helpers.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/event_log.h>
#include <mbedtls/md.h>

#include <plat/common/platform.h>

/* Event Log data */
static uint8_t event_log[EVENT_LOG_SIZE];

/* End of Event Log */
#define	EVENT_LOG_END	((uintptr_t)event_log + sizeof(event_log) - 1U)

CASSERT(sizeof(event_log) >= LOG_MIN_SIZE, assert_event_log_size);

/* Pointer in event_log[] */
static uint8_t *log_ptr = event_log;

/* Pointer to measured_boot_data_t */
const static measured_boot_data_t *plat_data_ptr;

static uintptr_t tos_fw_config_base;
static uintptr_t nt_fw_config_base;

/* TCG_EfiSpecIdEvent */
static const id_event_headers_t id_event_header = {
	.header = {
		.pcr_index = PCR_0,
		.event_type = EV_NO_ACTION,
		.digest = {0},
		.event_size = (uint32_t)(sizeof(id_event_struct_t) +
				(sizeof(id_event_algorithm_size_t) *
				HASH_ALG_COUNT))
	},

	.struct_header = {
		.signature = TCG_ID_EVENT_SIGNATURE_03,
		.platform_class = PLATFORM_CLASS_CLIENT,
		.spec_version_minor = TCG_SPEC_VERSION_MINOR_TPM2,
		.spec_version_major = TCG_SPEC_VERSION_MAJOR_TPM2,
		.spec_errata = TCG_SPEC_ERRATA_TPM2,
		.uintn_size = (uint8_t)(sizeof(unsigned int) /
					sizeof(uint32_t)),
		.number_of_algorithms = HASH_ALG_COUNT
	}
};

static const event2_header_t locality_event_header = {
		/*
		 * All EV_NO_ACTION events SHALL set
		 * TCG_PCR_EVENT2.pcrIndex = 0, unless otherwise specified
		 */
		.pcr_index = PCR_0,

		/*
		 * All EV_NO_ACTION events SHALL set
		 * TCG_PCR_EVENT2.eventType = 03h
		 */
		.event_type = EV_NO_ACTION,

		/*
		 * All EV_NO_ACTION events SHALL set
		 * TCG_PCR_EVENT2.digests to all
		 * 0x00's for each allocated Hash algorithm
		 */
		.digests = {
			.count = HASH_ALG_COUNT
		}
};

/* Platform's table with platform specific image IDs, names and PCRs */
static const image_data_t plat_images_data[] = {
	{ BL2_IMAGE_ID, BL2_STRING, PCR_0 },		/* Reserved for BL2 */
	{ INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

static const measured_boot_data_t plat_measured_boot_data = {
	plat_images_data,
	NULL,	/* platform_set_nt_fw_info */
	NULL	/* platform_set_tos_fw_info */
};

/*
 * Function retuns pointer to platform's measured_boot_data_t structure
 *
 * Must be overridden in the platform code
 */
#pragma weak plat_get_measured_boot_data

const measured_boot_data_t *plat_get_measured_boot_data(void)
{
	return &plat_measured_boot_data;
}

/*
 * Add TCG_PCR_EVENT2 event
 *
 * @param[in] hash	Pointer to hash data of TCG_DIGEST_SIZE bytes
 * @param[in] image_ptr	Pointer to image_data_t structure
 * @return:
 *	0 = success
 *    < 0 = error code
 */
static int add_event2(const uint8_t *hash, const image_data_t *image_ptr)
{
	void *ptr = log_ptr;
	uint32_t name_len;
	uint32_t size_of_event;

	assert(image_ptr != NULL);
	assert(image_ptr->name != NULL);

	name_len = (uint32_t)strlen(image_ptr->name) + 1U;
	size_of_event = name_len + (uint32_t)EVENT2_HDR_SIZE;

	/* Check for space in Event Log buffer */
	if (((uintptr_t)ptr + size_of_event) > EVENT_LOG_END) {
		ERROR("%s(): Event Log is short of memory", __func__);
		return -ENOMEM;
	}

	/*
	 * As per TCG specifications, firmware components that are measured
	 * into PCR[0] must be logged in the event log using the event type
	 * EV_POST_CODE.
	 */
	/* TCG_PCR_EVENT2.PCRIndex */
	((event2_header_t *)ptr)->pcr_index = image_ptr->pcr;

	/* TCG_PCR_EVENT2.EventType */
	((event2_header_t *)ptr)->event_type = EV_POST_CODE;

	/* TCG_PCR_EVENT2.Digests.Count */
	ptr = (uint8_t *)ptr + offsetof(event2_header_t, digests);
	((tpml_digest_values *)ptr)->count = HASH_ALG_COUNT;

	/* TCG_PCR_EVENT2.Digests[] */
	ptr = (uint8_t *)((uintptr_t)ptr +
			offsetof(tpml_digest_values, digests));

	/* TCG_PCR_EVENT2.Digests[].AlgorithmId */
	((tpmt_ha *)ptr)->algorithm_id = TPM_ALG_ID;

	/* TCG_PCR_EVENT2.Digests[].Digest[] */
	ptr = (uint8_t *)((uintptr_t)ptr + offsetof(tpmt_ha, digest));

	/* Check for space in Event Log buffer */
	if (((uintptr_t)ptr + TCG_DIGEST_SIZE) > EVENT_LOG_END) {
		ERROR("%s(): Event Log is short of memory", __func__);
		return -ENOMEM;
	}

	if (hash == NULL) {
		/* Get BL2 hash from DTB */
		bl2_plat_get_hash(ptr);
	} else {
		/* Copy digest */
		(void)memcpy(ptr, (const void *)hash, TCG_DIGEST_SIZE);
	}

	/* TCG_PCR_EVENT2.EventSize */
	ptr = (uint8_t *)((uintptr_t)ptr + TCG_DIGEST_SIZE);
	((event2_data_t *)ptr)->event_size = name_len;

	/* Copy event data to TCG_PCR_EVENT2.Event */
	(void)memcpy((void *)(((event2_data_t *)ptr)->event),
			(const void *)image_ptr->name, name_len);

	/* End of event data */
	log_ptr = (uint8_t *)((uintptr_t)ptr +
			offsetof(event2_data_t, event) + name_len);

	return 0;
}

/*
 * Init Event Log
 *
 * Initialises Event Log by writing Specification ID and
 * Startup Locality events.
 */
void event_log_init(void)
{
	const char locality_signature[] = TCG_STARTUP_LOCALITY_SIGNATURE;
	const uint8_t *start_ptr;
	void *ptr = event_log;

	/* Get pointer to platform's measured_boot_data_t structure */
	plat_data_ptr = plat_get_measured_boot_data();

	/*
	 * Add Specification ID Event first
	 *
	 * Copy TCG_EfiSpecIDEventStruct structure header
	 */
	(void)memcpy(ptr, (const void *)&id_event_header,
			sizeof(id_event_header));
	ptr = (uint8_t *)((uintptr_t)ptr + sizeof(id_event_header));

	/* TCG_EfiSpecIdEventAlgorithmSize structure */
	((id_event_algorithm_size_t *)ptr)->algorithm_id = TPM_ALG_ID;
	((id_event_algorithm_size_t *)ptr)->digest_size = TCG_DIGEST_SIZE;
	ptr = (uint8_t *)((uintptr_t)ptr + sizeof(id_event_algorithm_size_t));

	/*
	 * TCG_EfiSpecIDEventStruct.vendorInfoSize
	 * No vendor data
	 */
	((id_event_struct_data_t *)ptr)->vendor_info_size = 0;
	ptr = (uint8_t *)((uintptr_t)ptr +
			offsetof(id_event_struct_data_t, vendor_info));
	if ((uintptr_t)ptr != ((uintptr_t)event_log + ID_EVENT_SIZE)) {
		panic();
	}

	start_ptr = (uint8_t *)ptr;

	/*
	 * The Startup Locality event should be placed in the log before
	 * any event which extends PCR[0].
	 *
	 * Ref. TCG PC Client Platform Firmware Profile 9.4.5.3
	 */

	/* Copy Startup Locality Event Header */
	(void)memcpy(ptr, (const void *)&locality_event_header,
			sizeof(locality_event_header));
	ptr = (uint8_t *)((uintptr_t)ptr + sizeof(locality_event_header));

	/* TCG_PCR_EVENT2.Digests[].AlgorithmId */
	((tpmt_ha *)ptr)->algorithm_id = TPM_ALG_ID;

	/* TCG_PCR_EVENT2.Digests[].Digest[] */
	(void)memset(&((tpmt_ha *)ptr)->digest, 0, TPM_ALG_ID);
	ptr = (uint8_t *)((uintptr_t)ptr +
			offsetof(tpmt_ha, digest) + TCG_DIGEST_SIZE);

	/* TCG_PCR_EVENT2.EventSize */
	((event2_data_t *)ptr)->event_size =
		(uint32_t)sizeof(startup_locality_event_t);
	ptr = (uint8_t *)((uintptr_t)ptr + offsetof(event2_data_t, event));

	/* TCG_EfiStartupLocalityEvent.Signature */
	(void)memcpy(ptr, (const void *)locality_signature,
		sizeof(TCG_STARTUP_LOCALITY_SIGNATURE));

	/*
	 * TCG_EfiStartupLocalityEvent.StartupLocality = 0:
	 * the platform's boot firmware
	 */
	((startup_locality_event_t *)ptr)->startup_locality = 0U;
	ptr = (uint8_t *)((uintptr_t)ptr + sizeof(startup_locality_event_t));
	if ((uintptr_t)ptr != ((uintptr_t)start_ptr + LOC_EVENT_SIZE)) {
		panic();
	}

	log_ptr = (uint8_t *)ptr;

	/* Add BL2 event */
	if (add_event2(NULL, plat_data_ptr->images_data) != 0) {
		panic();
	}
}

/*
 * Calculate and write hash of image, configuration data, etc.
 * to Event Log.
 *
 * @param[in] data_base		Address of data
 * @param[in] data_size		Size of data
 * @param[in] data_id		Data ID
 * @return:
 *	0 = success
 *    < 0 = error
 */
int tpm_record_measurement(uintptr_t data_base, uint32_t data_size,
			   uint32_t data_id)
{
	const image_data_t *data_ptr = plat_data_ptr->images_data;
	unsigned char hash_data[MBEDTLS_MD_MAX_SIZE];
	int rc;

	/* Check if image_id is supported */
	while (data_ptr->id != data_id) {
		if ((data_ptr++)->id == INVALID_ID) {
			ERROR("%s(): image_id %u not supported\n",
				__func__, data_id);
			return -EINVAL;
		}
	}

	if (data_id == TOS_FW_CONFIG_ID) {
		tos_fw_config_base = data_base;
	} else if (data_id == NT_FW_CONFIG_ID) {
		nt_fw_config_base = data_base;
	} else {
		/* No action */
	}

	/* Calculate hash */
	rc = crypto_mod_calc_hash((unsigned int)MBEDTLS_MD_ID,
				(void *)data_base, data_size, hash_data);
	if (rc != 0) {
		return rc;
	}

	return add_event2(hash_data, data_ptr);
}

/*
 * Finalise Event Log
 *
 * @param[out] log_addr	Pointer to return Event Log address
 * @param[out] log_size	Pointer to return Event Log size
 * @return:
 *	0 = success
 *    < 0 = error code
 */
int event_log_finalise(uint8_t **log_addr, size_t *log_size)
{
	/* Event Log size */
	size_t num_bytes = (uintptr_t)log_ptr - (uintptr_t)event_log;
	int rc;

	assert(log_addr != NULL);
	assert(log_size != NULL);

	if (nt_fw_config_base == 0UL) {
		ERROR("%s(): %s_FW_CONFIG not loaded\n", __func__, "NT");
		return -ENOENT;
	}

	/*
	 * Set Event Log data in NT_FW_CONFIG and
	 * get Event Log address in Non-Secure memory
	 */
	if (plat_data_ptr->set_nt_fw_info != NULL) {

		/* Event Log address in Non-Secure memory */
		uintptr_t ns_log_addr;

		rc = plat_data_ptr->set_nt_fw_info(
				nt_fw_config_base,
#ifdef SPD_opteed
				(uintptr_t)event_log,
#endif
				num_bytes, &ns_log_addr);
		if (rc != 0) {
			ERROR("%s(): Unable to update %s_FW_CONFIG\n",
						__func__, "NT");
			return rc;
		}

		/* Copy Event Log to Non-secure memory */
		(void)memcpy((void *)ns_log_addr, (const void *)event_log,
				num_bytes);

		/* Ensure that the Event Log is visible in Non-secure memory */
		flush_dcache_range(ns_log_addr, num_bytes);

		/* Return Event Log address in Non-Secure memory */
		*log_addr = (uint8_t *)ns_log_addr;

	} else {
		INFO("%s(): set_%s_fw_info not set\n", __func__, "nt");

		/* Return Event Log address in Secure memory */
		*log_addr = event_log;
	}

	if (tos_fw_config_base != 0UL) {
		if (plat_data_ptr->set_tos_fw_info != NULL) {

			/* Set Event Log data in TOS_FW_CONFIG */
			rc = plat_data_ptr->set_tos_fw_info(
						tos_fw_config_base,
						(uintptr_t)event_log,
						num_bytes);
			if (rc != 0) {
				ERROR("%s(): Unable to update %s_FW_CONFIG\n",
						__func__, "TOS");
				return rc;
			}
		} else {
			INFO("%s(): set_%s_fw_info not set\n", __func__, "tos");
		}
	} else {
		INFO("%s(): %s_FW_CONFIG not loaded\n", __func__, "TOS");
	}

	/* Ensure that the Event Log is visible in Secure memory */
	flush_dcache_range((uintptr_t)event_log, num_bytes);

	/* Return Event Log size */
	*log_size = num_bytes;

	return 0;
}
