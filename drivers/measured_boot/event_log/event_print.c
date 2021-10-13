/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/measured_boot/event_log/event_log.h>

#if LOG_LEVEL >= EVENT_LOG_LEVEL

/*
 * Print TCG_EfiSpecIDEventStruct
 *
 * @param[in/out] log_addr	Pointer to Event Log
 * @param[in/out] log_size	Pointer to Event Log size
 */
static void id_event_print(uint8_t **log_addr, size_t *log_size)
{
	unsigned int i;
	uint8_t info_size, *info_size_ptr;
	void *ptr = *log_addr;
	id_event_headers_t *event = (id_event_headers_t *)ptr;
	id_event_algorithm_size_t *alg_ptr;
	uint32_t event_size, number_of_algorithms;
	size_t digest_len;
#if ENABLE_ASSERTIONS
	const uint8_t *end_ptr = (uint8_t *)((uintptr_t)*log_addr + *log_size);
	bool valid = true;
#endif

	assert(*log_size >= sizeof(id_event_headers_t));

	/* The fields of the event log header are defined to be PCRIndex of 0,
	 * EventType of EV_NO_ACTION, Digest of 20 bytes of 0, and
	 * Event content defined as TCG_EfiSpecIDEventStruct.
	 */
	LOG_EVENT("TCG_EfiSpecIDEvent:\n");
	LOG_EVENT("  PCRIndex           : %u\n", event->header.pcr_index);
	assert(event->header.pcr_index == (uint32_t)PCR_0);

	LOG_EVENT("  EventType          : %u\n", event->header.event_type);
	assert(event->header.event_type == EV_NO_ACTION);

	LOG_EVENT("  Digest             :");
	for (i = 0U; i < sizeof(event->header.digest); ++i) {
		uint8_t val = event->header.digest[i];

		(void)printf(" %02x", val);
		if ((i & U(0xF)) == 0U) {
			(void)printf("\n");
			LOG_EVENT("\t\t      :");
		}
#if ENABLE_ASSERTIONS
		if (val != 0U) {
			valid = false;
		}
#endif
	}
	if ((i & U(0xF)) != 0U) {
		(void)printf("\n");
	}

	assert(valid);

	/* EventSize */
	event_size = event->header.event_size;
	LOG_EVENT("  EventSize          : %u\n", event_size);

	LOG_EVENT("  Signature          : %s\n",
			event->struct_header.signature);
	LOG_EVENT("  PlatformClass      : %u\n",
			event->struct_header.platform_class);
	LOG_EVENT("  SpecVersion        : %u.%u.%u\n",
			event->struct_header.spec_version_major,
			event->struct_header.spec_version_minor,
			event->struct_header.spec_errata);
	LOG_EVENT("  UintnSize          : %u\n",
			event->struct_header.uintn_size);

	/* NumberOfAlgorithms */
	number_of_algorithms = event->struct_header.number_of_algorithms;
	LOG_EVENT("  NumberOfAlgorithms : %u\n", number_of_algorithms);

	/* Address of DigestSizes[] */
	alg_ptr = event->struct_header.digest_size;

	/* Size of DigestSizes[] */
	digest_len = number_of_algorithms * sizeof(id_event_algorithm_size_t);
	assert(((uintptr_t)alg_ptr + digest_len) <= (uintptr_t)end_ptr);

	LOG_EVENT("  DigestSizes        :\n");
	for (i = 0U; i < number_of_algorithms; ++i) {
		LOG_EVENT("    #%u AlgorithmId   : SHA", i);
		uint16_t algorithm_id = alg_ptr[i].algorithm_id;

		switch (algorithm_id) {
		case TPM_ALG_SHA256:
			(void)printf("256\n");
			break;
		case TPM_ALG_SHA384:
			(void)printf("384\n");
			break;
		case TPM_ALG_SHA512:
			(void)printf("512\n");
			break;
		default:
			(void)printf("?\n");
			ERROR("Algorithm 0x%x not found\n", algorithm_id);
			assert(false);
		}

		LOG_EVENT("       DigestSize    : %u\n",
					alg_ptr[i].digest_size);
	}

	/* Address of VendorInfoSize */
	info_size_ptr = (uint8_t *)((uintptr_t)alg_ptr + digest_len);
	assert((uintptr_t)info_size_ptr <= (uintptr_t)end_ptr);

	info_size = *info_size_ptr++;
	LOG_EVENT("  VendorInfoSize     : %u\n", info_size);

	/* Check VendorInfo end address */
	assert(((uintptr_t)info_size_ptr + info_size) <= (uintptr_t)end_ptr);

	/* Check EventSize */
	assert(event_size == (sizeof(id_event_struct_t) +
				digest_len + info_size));
	if (info_size != 0U) {
		LOG_EVENT("  VendorInfo         :");
		for (i = 0U; i < info_size; ++i) {
			(void)printf(" %02x", *info_size_ptr++);
		}
		(void)printf("\n");
	}

	*log_size -= (uintptr_t)info_size_ptr - (uintptr_t)*log_addr;
	*log_addr = info_size_ptr;
}

/*
 * Print TCG_PCR_EVENT2
 *
 * @param[in/out] log_addr	Pointer to Event Log
 * @param[in/out] log_size	Pointer to Event Log size
 */
static void event2_print(uint8_t **log_addr, size_t *log_size)
{
	uint32_t event_size, count;
	size_t sha_size, digests_size = 0U;
	void *ptr = *log_addr;
#if ENABLE_ASSERTIONS
	const uint8_t *end_ptr = (uint8_t *)((uintptr_t)*log_addr + *log_size);
#endif

	assert(*log_size >= sizeof(event2_header_t));

	LOG_EVENT("PCR_Event2:\n");
	LOG_EVENT("  PCRIndex           : %u\n",
			((event2_header_t *)ptr)->pcr_index);
	LOG_EVENT("  EventType          : %u\n",
			((event2_header_t *)ptr)->event_type);

	count = ((event2_header_t *)ptr)->digests.count;
	LOG_EVENT("  Digests Count      : %u\n", count);

	/* Address of TCG_PCR_EVENT2.Digests[] */
	ptr = (uint8_t *)ptr + sizeof(event2_header_t);
	assert(((uintptr_t)ptr <= (uintptr_t)end_ptr) && (count != 0U));

	for (unsigned int i = 0U; i < count; ++i) {
		/* Check AlgorithmId address */
		assert(((uintptr_t)ptr +
			offsetof(tpmt_ha, digest)) <= (uintptr_t)end_ptr);

		LOG_EVENT("    #%u AlgorithmId   : SHA", i);
		switch (((tpmt_ha *)ptr)->algorithm_id) {
		case TPM_ALG_SHA256:
			sha_size = SHA256_DIGEST_SIZE;
			(void)printf("256\n");
			break;
		case TPM_ALG_SHA384:
			sha_size = SHA384_DIGEST_SIZE;
			(void)printf("384\n");
			break;
		case TPM_ALG_SHA512:
			sha_size = SHA512_DIGEST_SIZE;
			(void)printf("512\n");
			break;
		default:
			(void)printf("?\n");
			ERROR("Algorithm 0x%x not found\n",
				((tpmt_ha *)ptr)->algorithm_id);
			panic();
		}

		/* End of Digest[] */
		ptr = (uint8_t *)((uintptr_t)ptr + offsetof(tpmt_ha, digest));
		assert(((uintptr_t)ptr + sha_size) <= (uintptr_t)end_ptr);

		/* Total size of all digests */
		digests_size += sha_size;

		LOG_EVENT("       Digest        :");
		for (unsigned int j = 0U; j < sha_size; ++j) {
			(void)printf(" %02x", *(uint8_t *)ptr++);
			if ((j & U(0xF)) == U(0xF)) {
				(void)printf("\n");
				if (j < (sha_size - 1U)) {
					LOG_EVENT("\t\t      :");
				}
			}
		}
	}

	/* TCG_PCR_EVENT2.EventSize */
	assert(((uintptr_t)ptr + offsetof(event2_data_t, event)) <= (uintptr_t)end_ptr);

	event_size = ((event2_data_t *)ptr)->event_size;
	LOG_EVENT("  EventSize          : %u\n", event_size);

	/* Address of TCG_PCR_EVENT2.Event[EventSize] */
	ptr = (uint8_t *)((uintptr_t)ptr + offsetof(event2_data_t, event));

	/* End of TCG_PCR_EVENT2.Event[EventSize] */
	assert(((uintptr_t)ptr + event_size) <= (uintptr_t)end_ptr);

	if ((event_size == sizeof(startup_locality_event_t)) &&
	     (strcmp((const char *)ptr, TCG_STARTUP_LOCALITY_SIGNATURE) == 0)) {
		LOG_EVENT("  Signature          : %s\n",
			((startup_locality_event_t *)ptr)->signature);
		LOG_EVENT("  StartupLocality    : %u\n",
			((startup_locality_event_t *)ptr)->startup_locality);
	} else {
		LOG_EVENT("  Event              : %s\n", (uint8_t *)ptr);
	}

	*log_size -= (uintptr_t)ptr + event_size - (uintptr_t)*log_addr;
	*log_addr = (uint8_t *)ptr + event_size;
}
#endif	/* LOG_LEVEL >= EVENT_LOG_LEVEL */

/*
 * Print Event Log
 *
 * @param[in]	log_addr	Pointer to Event Log
 * @param[in]	log_size	Event Log size
 */
void dump_event_log(uint8_t *log_addr, size_t log_size)
{
#if LOG_LEVEL >= EVENT_LOG_LEVEL
	assert(log_addr != NULL);

	/* Print TCG_EfiSpecIDEvent */
	id_event_print(&log_addr, &log_size);

	while (log_size != 0U) {
		event2_print(&log_addr, &log_size);
	}
#endif
}
