/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
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
#include <drivers/measured_boot/event_log/event_log.h>

#include <plat/common/platform.h>

#if TPM_ALG_ID == TPM_ALG_SHA512
#define	CRYPTO_MD_ID	CRYPTO_MD_SHA512
#elif TPM_ALG_ID == TPM_ALG_SHA384
#define	CRYPTO_MD_ID	CRYPTO_MD_SHA384
#elif TPM_ALG_ID == TPM_ALG_SHA256
#define	CRYPTO_MD_ID	CRYPTO_MD_SHA256
#else
#  error Invalid TPM algorithm.
#endif /* TPM_ALG_ID */

/* Running Event Log Pointer */
static uint8_t *log_ptr;

/* Pointer to the first byte past end of the Event Log buffer */
static uintptr_t log_end;

/* Pointer to event_log_metadata_t */
static const event_log_metadata_t *plat_metadata_ptr;

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
	 * All EV_NO_ACTION events SHALL set TCG_PCR_EVENT2.digests to all
	 * 0x00's for each allocated Hash algorithm
	 */
	.digests = {
		.count = HASH_ALG_COUNT
	}
};

/*
 * Record a measurement as a TCG_PCR_EVENT2 event
 *
 * @param[in] hash		Pointer to hash data of TCG_DIGEST_SIZE bytes
 * @param[in] metadata_ptr	Pointer to event_log_metadata_t structure
 *
 * There must be room for storing this new event into the event log buffer.
 */
static void event_log_record(const uint8_t *hash,
			     const event_log_metadata_t *metadata_ptr)
{
	void *ptr = log_ptr;
	uint32_t name_len;

	assert(hash != NULL);
	assert(metadata_ptr != NULL);
	assert(metadata_ptr->name != NULL);
	/* event_log_init() must have been called prior to this. */
	assert(log_ptr != NULL);

	name_len = (uint32_t)strlen(metadata_ptr->name) + 1U;

	/* Check for space in Event Log buffer */
	assert(((uintptr_t)ptr + (uint32_t)EVENT2_HDR_SIZE + name_len) <
	       log_end);

	/*
	 * As per TCG specifications, firmware components that are measured
	 * into PCR[0] must be logged in the event log using the event type
	 * EV_POST_CODE.
	 */
	/* TCG_PCR_EVENT2.PCRIndex */
	((event2_header_t *)ptr)->pcr_index = metadata_ptr->pcr;

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

	/* Copy digest */
	(void)memcpy(ptr, (const void *)hash, TCG_DIGEST_SIZE);

	/* TCG_PCR_EVENT2.EventSize */
	ptr = (uint8_t *)((uintptr_t)ptr + TCG_DIGEST_SIZE);
	((event2_data_t *)ptr)->event_size = name_len;

	/* Copy event data to TCG_PCR_EVENT2.Event */
	(void)memcpy((void *)(((event2_data_t *)ptr)->event),
			(const void *)metadata_ptr->name, name_len);

	/* End of event data */
	log_ptr = (uint8_t *)((uintptr_t)ptr +
			offsetof(event2_data_t, event) + name_len);
}

/*
 * Initialise Event Log global variables, used during the recording
 * of various payload measurements into the Event Log buffer
 *
 * @param[in] event_log_start		Base address of Event Log buffer
 * @param[in] event_log_finish		End address of Event Log buffer,
 * 					it is a first byte past end of the
 * 					buffer
 */
void event_log_init(uint8_t *event_log_start, uint8_t *event_log_finish)
{
	assert(event_log_start != NULL);
	assert(event_log_finish > event_log_start);

	log_ptr = event_log_start;
	log_end = (uintptr_t)event_log_finish;

	/* Get pointer to platform's event_log_metadata_t structure */
	plat_metadata_ptr = plat_event_log_get_metadata();
	assert(plat_metadata_ptr != NULL);
}

/*
 * Initialises Event Log by writing Specification ID and
 * Startup Locality events
 */
void event_log_write_header(void)
{
	const char locality_signature[] = TCG_STARTUP_LOCALITY_SIGNATURE;
	void *ptr = log_ptr;

	/* event_log_init() must have been called prior to this. */
	assert(log_ptr != NULL);

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
	log_ptr = (uint8_t *)((uintptr_t)ptr + sizeof(startup_locality_event_t));
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
int event_log_measure_and_record(uintptr_t data_base, uint32_t data_size,
				 uint32_t data_id)
{
	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	int rc;
	const event_log_metadata_t *metadata_ptr = plat_metadata_ptr;

	/* Get the metadata associated with this image. */
	while ((metadata_ptr->id != EVLOG_INVALID_ID) &&
		(metadata_ptr->id != data_id)) {
		metadata_ptr++;
	}
	assert(metadata_ptr->id != EVLOG_INVALID_ID);

	/* Calculate hash */
	rc = crypto_mod_calc_hash(CRYPTO_MD_ID,
				  (void *)data_base, data_size, hash_data);
	if (rc != 0) {
		return rc;
	}

	event_log_record(hash_data, metadata_ptr);

	return 0;
}

/*
 * Get current Event Log buffer size i.e. used space of Event Log buffer
 *
 * @param[in]  event_log_start		Base Pointer to Event Log buffer
 *
 * @return: current Size of Event Log buffer
 */
size_t event_log_get_cur_size(uint8_t *event_log_start)
{
	assert(event_log_start != NULL);
	assert(log_ptr >= event_log_start);

	return (size_t)((uintptr_t)log_ptr - (uintptr_t)event_log_start);
}
