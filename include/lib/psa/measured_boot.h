/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PSA_MEASURED_BOOT_H
#define PSA_MEASURED_BOOT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "psa/error.h"

/**
 * Extends and stores a measurement to the requested slot.
 *
 * index			Slot number in which measurement is to be stored
 * signer_id			Pointer to signer_id buffer.
 * signer_id_size		Size of the signer_id in bytes.
 * version			Pointer to version buffer.
 * version_size			Size of the version string in bytes.
 * measurement_algo		Algorithm identifier used for measurement.
 * sw_type			Pointer to sw_type buffer.
 * sw_type_size			Size of the sw_type string in bytes.
 * measurement_value		Pointer to measurement_value buffer.
 * measurement_value_size	Size of the measurement_value in bytes.
 * lock_measurement		Boolean flag requesting whether the measurement
 *				is to be locked.
 *
 * PSA_SUCCESS:
 *	- Success.
 * PSA_ERROR_INVALID_ARGUMENT:
 *	- The size of any argument is invalid OR
 *	- Input Measurement value is NULL OR
 *	- Input Signer ID is NULL OR
 *	- Requested slot index is invalid.
 * PSA_ERROR_BAD_STATE:
 *	- Request to lock, when slot is already locked.
 * PSA_ERROR_NOT_PERMITTED:
 *	- When the requested slot is not accessible to the caller.
 */

/* Not a standard PSA API, just an extension therefore use the 'rse_' prefix
 * rather than the usual 'psa_'.
 */
psa_status_t
rse_measured_boot_extend_measurement(uint8_t index,
				     const uint8_t *signer_id,
				     size_t signer_id_size,
				     const uint8_t *version,
				     size_t version_size,
				     uint32_t measurement_algo,
				     const uint8_t *sw_type,
				     size_t sw_type_size,
				     const uint8_t *measurement_value,
				     size_t measurement_value_size,
				     bool lock_measurement);

/**
 * Retrieves a measurement from the requested slot.
 *
 * index			Slot number from which measurement is to be
 *				retrieved.
 * signer_id			Pointer to signer_id buffer.
 * signer_id_size		Size of the signer_id buffer in bytes.
 * signer_id_len		On success, number of bytes that make up
 * 				signer_id.
 * version			Pointer to version buffer.
 * version_size			Size of the version buffer in bytes.
 * version_len			On success, number of bytes that makeup the
 * 				version.
 * measurement_algo		Pointer to measurement_algo.
 * sw_type			Pointer to sw_type buffer.
 * sw_type_size			Size of the sw_type buffer in bytes.
 * sw_type_len			On success, number of bytes that makeup the
 * 				sw_type.
 * measurement_value		Pointer to measurement_value buffer.
 * measurement_value_size	Size of the measurement_value buffer in bytes.
 * measurement_value_len	On success, number of bytes that make up the
 * 				measurement_value.
 * is_locked			Pointer to lock status of requested measurement
 * 				slot.
 *
 * PSA_SUCCESS
 *	- Success.
 * PSA_ERROR_INVALID_ARGUMENT
 *	- The size of at least one of the output buffers is incorrect or the
 *	  requested slot index is invalid.
 * PSA_ERROR_DOES_NOT_EXIST
 *	- The requested slot is empty, does not contain a measurement.
 */
psa_status_t rse_measured_boot_read_measurement(uint8_t index,
					uint8_t *signer_id,
					size_t signer_id_size,
					size_t *signer_id_len,
					uint8_t *version,
					size_t version_size,
					size_t *version_len,
					uint32_t *measurement_algo,
					uint8_t *sw_type,
					size_t sw_type_size,
					size_t *sw_type_len,
					uint8_t *measurement_value,
					size_t measurement_value_size,
					size_t *measurement_value_len,
					bool *is_locked);

#endif /* PSA_MEASURED_BOOT_H */
