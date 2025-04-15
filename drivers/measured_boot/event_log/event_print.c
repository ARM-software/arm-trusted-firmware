/*
 * Copyright (c) 2020-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include "event_log.h"

/**
 * Print a TCG_EfiSpecIDEventStruct entry from the event log.
 *
 * This function extracts and prints a TCG_EfiSpecIDEventStruct
 * entry from the event log for debugging or auditing purposes.
 *
 * @param[in,out] log_addr  Pointer to the current position in the Event Log.
 *                          Updated to the next entry after processing.
 * @param[in,out] log_size  Pointer to the remaining Event Log size.
 *                          Updated to reflect the remaining bytes.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int event_log_print_id_event(uint8_t **log_addr, size_t *log_size)
{
	unsigned int i;
	uint8_t info_size, *info_size_ptr;
	void *ptr = *log_addr;
	id_event_headers_t *event = (id_event_headers_t *)ptr;
	id_event_algorithm_size_t *alg_ptr;
	uint32_t event_size, number_of_algorithms;
	size_t digest_len;
	const uint8_t *end_ptr = (uint8_t *)((uintptr_t)*log_addr + *log_size);

	if (*log_size < sizeof(id_event_headers_t)) {
		return -EINVAL;
	}

	/* The fields of the event log header are defined to be PCRIndex of 0,
	 * EventType of EV_NO_ACTION, Digest of 20 bytes of 0, and
	 * Event content defined as TCG_EfiSpecIDEventStruct.
	 */
	LOG_EVENT("TCG_EfiSpecIDEvent:\n");
	LOG_EVENT("  PCRIndex           : %u\n", event->header.pcr_index);
	if (event->header.pcr_index != (uint32_t)PCR_0) {
		return -EINVAL;
	}

	LOG_EVENT("  EventType          : %u\n", event->header.event_type);
	if (event->header.event_type != EV_NO_ACTION) {
		return -EINVAL;
	}

	LOG_EVENT("  Digest             :");
	for (i = 0U; i < sizeof(event->header.digest); ++i) {
		uint8_t val = event->header.digest[i];

		(void)printf(" %02x", val);
		if ((i & U(0xF)) == 0U) {
			(void)printf("\n");
			LOG_EVENT("\t\t      :");
		}
	}

	if ((i & U(0xF)) != 0U) {
		(void)printf("\n");
	}

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
	if (digest_len > (uintptr_t)end_ptr - (uintptr_t)alg_ptr) {
		return -EFAULT;
	}

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
			return -ENOENT;
		}

		LOG_EVENT("       DigestSize    : %u\n",
					alg_ptr[i].digest_size);
	}

	/* Address of VendorInfoSize */
	info_size_ptr = (uint8_t *)((uintptr_t)alg_ptr + digest_len);
	if ((uintptr_t)info_size_ptr > (uintptr_t)end_ptr) {
		return -EFAULT;
	}

	info_size = *info_size_ptr++;
	LOG_EVENT("  VendorInfoSize     : %u\n", info_size);

	/* Check VendorInfo end address */
	if (((uintptr_t)info_size_ptr + info_size) > (uintptr_t)end_ptr) {
		return -EFAULT;
	}

	/* Check EventSize */
	if (event_size !=
	    (sizeof(id_event_struct_t) + digest_len + info_size)) {
		return -EFAULT;
	}

	if (info_size != 0U) {
		LOG_EVENT("  VendorInfo         :");
		for (i = 0U; i < info_size; ++i) {
			(void)printf(" %02x", *info_size_ptr++);
		}
		(void)printf("\n");
	}

	*log_size -= (uintptr_t)info_size_ptr - (uintptr_t)*log_addr;
	*log_addr = info_size_ptr;

	return 0;
}

/**
 * Print a TCG_PCR_EVENT2 entry from the event log.
 *
 * This function extracts and prints a TCG_PCR_EVENT2 structure
 * from the event log for debugging or auditing purposes.
 *
 * @param[in,out] log_addr  Pointer to the current position in the Event Log.
 *                          Updated to the next entry after processing.
 * @param[in,out] log_size  Pointer to the remaining Event Log size.
 *                          Updated to reflect the remaining bytes.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int event_log_print_pcr_event2(uint8_t **log_addr, size_t *log_size)
{
	uint32_t event_size, count;
	size_t sha_size, digests_size = 0U;
	void *ptr = *log_addr;
	const uint8_t *end_ptr = (uint8_t *)((uintptr_t)*log_addr + *log_size);

	if (*log_size < sizeof(event2_header_t)) {
		return -EINVAL;
	}

	LOG_EVENT("PCR_Event2:\n");
	LOG_EVENT("  PCRIndex           : %u\n",
			((event2_header_t *)ptr)->pcr_index);
	LOG_EVENT("  EventType          : %u\n",
			((event2_header_t *)ptr)->event_type);

	count = ((event2_header_t *)ptr)->digests.count;
	if (count < 1U) {
		LOG_EVENT("Invalid Digests Count      : %u\n", count);
		return -EINVAL;
	}

	LOG_EVENT("  Digests Count      : %u\n", count);

	/* Address of TCG_PCR_EVENT2.Digests[] */
	ptr = (uint8_t *)ptr + sizeof(event2_header_t);
	if ((uintptr_t)ptr > (uintptr_t)end_ptr) {
		return -EFAULT;
	}

	for (unsigned int i = 0U; i < count; ++i) {
		/* Check AlgorithmId address */
		if (((uintptr_t)ptr + offsetof(tpmt_ha, digest)) >
		    (uintptr_t)end_ptr) {
			return -EFAULT;
		}

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
			printf("Algorithm 0x%x not found\n",
				((tpmt_ha *)ptr)->algorithm_id);
			return -ENOENT;
		}

		/* End of Digest[] */
		ptr = (uint8_t *)((uintptr_t)ptr + offsetof(tpmt_ha, digest));
		if (((uintptr_t)ptr + sha_size) > (uintptr_t)end_ptr) {
			return -EFAULT;
		}

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
	if (((uintptr_t)ptr + offsetof(event2_data_t, event)) >
	    (uintptr_t)end_ptr) {
		return -EFAULT;
	}

	event_size = ((event2_data_t *)ptr)->event_size;
	LOG_EVENT("  EventSize          : %u\n", event_size);

	/* Address of TCG_PCR_EVENT2.Event[EventSize] */
	ptr = (uint8_t *)((uintptr_t)ptr + offsetof(event2_data_t, event));

	/* End of TCG_PCR_EVENT2.Event[EventSize] */
	if (((uintptr_t)ptr + event_size) > (uintptr_t)end_ptr) {
		return -EFAULT;
	}

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

	return 0;
}

int event_log_dump(uint8_t *log_addr, size_t log_size)
{
	int rc;

	if (log_addr == NULL) {
		return -EINVAL;
	}

	/* Print TCG_EfiSpecIDEvent */
	rc = event_log_print_id_event(&log_addr, &log_size);

	if (rc < 0) {
		return rc;
	}

	while (log_size != 0U) {
		rc = event_log_print_pcr_event2(&log_addr, &log_size);
		if (rc < 0) {
			return rc;
		}
	}
	return 0;
}
