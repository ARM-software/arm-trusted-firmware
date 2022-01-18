/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
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

/* Minimum measurement value size that can be requested to store */
#define MEASUREMENT_VALUE_MIN_SIZE	32U
/* Maximum measurement value size that can be requested to store */
#define MEASUREMENT_VALUE_MAX_SIZE	64U
/* Minimum signer id size that can be requested to store */
#define SIGNER_ID_MIN_SIZE		MEASUREMENT_VALUE_MIN_SIZE
/* Maximum signer id size that can be requested to store */
#define SIGNER_ID_MAX_SIZE		MEASUREMENT_VALUE_MAX_SIZE
/* The theoretical maximum image version is: "255.255.65535\0" */
#define VERSION_MAX_SIZE		14U
/* Example sw_type: "BL_2, BL_33, etc." */
#define SW_TYPE_MAX_SIZE		20U
#define NUM_OF_MEASUREMENT_SLOTS	32U


/**
 * Extends and stores a measurement to the requested slot.
 *
 * index			Slot number in which measurement is to be stored
 * signer_id			Pointer to signer_id buffer.
 * signer_id_size		Size of the signer_id buffer in bytes.
 * version			Pointer to version buffer.
 * version_size			Size of the version buffer in bytes.
 * measurement_algo		Algorithm identifier used for measurement.
 * sw_type			Pointer to sw_type buffer.
 * sw_type_size			Size of the sw_type buffer in bytes.
 * measurement_value		Pointer to measurement_value buffer.
 * measurement_value_size	Size of the measurement_value buffer in bytes.
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

/* Not a standard PSA API, just an extension therefore use the 'rss_' prefix
 * rather than the usual 'psa_'.
 */
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
				     bool lock_measurement);

#endif /* PSA_MEASURED_BOOT_H */
