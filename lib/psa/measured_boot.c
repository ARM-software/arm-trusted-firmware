/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>

#include <common/debug.h>
#include <measured_boot.h>
#include <psa/client.h>
#include <psa_manifest/sid.h>

#include "measured_boot_private.h"

static void print_byte_array(const uint8_t *array __unused, size_t len __unused)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	size_t i;

	if (array == NULL || len == 0U) {
		(void)printf("\n");
	} else {
		for (i = 0U; i < len; ++i) {
			(void)printf(" %02x", array[i]);
			if ((i & U(0xF)) == U(0xF)) {
				(void)printf("\n");
				if (i < (len - 1U)) {
					INFO("\t\t:");
				}
			}
		}
	}
#endif
}

static void log_measurement(uint8_t index,
			    const uint8_t *signer_id,
			    size_t signer_id_size,
			    const uint8_t *version,     /* string */
			    size_t version_size,
			    const uint8_t *sw_type,     /* string */
			    size_t sw_type_size,
			    uint32_t measurement_algo,
			    const uint8_t *measurement_value,
			    size_t measurement_value_size,
			    bool lock_measurement)
{
	INFO("Measured boot extend measurement:\n");
	INFO(" - slot        : %u\n", index);
	INFO(" - signer_id   :");
	print_byte_array(signer_id, signer_id_size);
	INFO(" - version     : %s\n",  version);
	INFO(" - version_size: %zu\n", version_size);
	INFO(" - sw_type     : %s\n",  sw_type);
	INFO(" - sw_type_size: %zu\n", sw_type_size);
	INFO(" - algorithm   : %x\n", measurement_algo);
	INFO(" - measurement :");
	print_byte_array(measurement_value, measurement_value_size);
	INFO(" - locking     : %s\n", lock_measurement ? "true" : "false");
}

#if !PLAT_RSS_NOT_SUPPORTED
psa_status_t
rss_measured_boot_extend_measurement(uint8_t index,
				     const uint8_t *signer_id,
				     size_t signer_id_size,
				     const uint8_t *version,
				     size_t version_size,
				     uint32_t measurement_algo,
				     const uint8_t *sw_type,
				     size_t sw_type_size,
				     const uint8_t *measurement_value,
				     size_t measurement_value_size,
				     bool lock_measurement)
{
	struct measured_boot_extend_iovec_t extend_iov = {
		.index = index,
		.lock_measurement = lock_measurement,
		.measurement_algo = measurement_algo,
		.sw_type = {0},
		.sw_type_size = sw_type_size,
	};

	psa_invec in_vec[] = {
		{.base = &extend_iov,
			.len = sizeof(struct measured_boot_extend_iovec_t)},
		{.base = signer_id, .len = signer_id_size},
		{.base = version, .len = version_size},
		{.base = measurement_value, .len = measurement_value_size}
	};

	if (sw_type != NULL) {
		if (sw_type_size > SW_TYPE_MAX_SIZE) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}
		memcpy(extend_iov.sw_type, sw_type, sw_type_size);
	}

	log_measurement(index, signer_id, signer_id_size,
			version, version_size, sw_type, sw_type_size,
			measurement_algo, measurement_value,
			measurement_value_size, lock_measurement);

	return psa_call(RSS_MEASURED_BOOT_HANDLE,
			RSS_MEASURED_BOOT_EXTEND,
			in_vec, IOVEC_LEN(in_vec),
			NULL, 0);
}

#else /* !PLAT_RSS_NOT_SUPPORTED */

psa_status_t
rss_measured_boot_extend_measurement(uint8_t index,
				     const uint8_t *signer_id,
				     size_t signer_id_size,
				     const uint8_t *version,
				     size_t version_size,
				     uint32_t measurement_algo,
				     const uint8_t *sw_type,
				     size_t sw_type_size,
				     const uint8_t *measurement_value,
				     size_t measurement_value_size,
				     bool lock_measurement)
{
	log_measurement(index, signer_id, signer_id_size,
			version, version_size, sw_type, sw_type_size,
			measurement_algo, measurement_value,
			measurement_value_size, lock_measurement);

	return PSA_SUCCESS;
}
#endif /* !PLAT_RSS_NOT_SUPPORTED */
