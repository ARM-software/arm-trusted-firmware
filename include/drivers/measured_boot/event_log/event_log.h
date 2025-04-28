/*
 * Copyright (c) 2020-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stddef.h>
#include <stdint.h>

#include <drivers/auth/crypto_mod.h>
#include "event_handoff.h"
#include "tcg.h"

/*
 * Set Event Log debug level to one of:
 *
 * LOG_LEVEL_ERROR
 * LOG_LEVEL_INFO
 * LOG_LEVEL_WARNING
 * LOG_LEVEL_VERBOSE
 */
#if EVENT_LOG_LEVEL   == LOG_LEVEL_ERROR
#define	LOG_EVENT	ERROR
#elif EVENT_LOG_LEVEL == LOG_LEVEL_NOTICE
#define	LOG_EVENT	NOTICE
#elif EVENT_LOG_LEVEL == LOG_LEVEL_WARNING
#define	LOG_EVENT	WARN
#elif EVENT_LOG_LEVEL == LOG_LEVEL_INFO
#define	LOG_EVENT	INFO
#elif EVENT_LOG_LEVEL == LOG_LEVEL_VERBOSE
#define	LOG_EVENT	VERBOSE
#else
#define LOG_EVENT printf
#endif

/* Number of hashing algorithms supported */
#define HASH_ALG_COUNT		1U

#define EVLOG_INVALID_ID	UINT32_MAX

#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

typedef struct {
	unsigned int id;
	const char *name;
	unsigned int pcr;
} event_log_metadata_t;

#define	ID_EVENT_SIZE	(sizeof(id_event_headers_t) + \
			(sizeof(id_event_algorithm_size_t) * HASH_ALG_COUNT) + \
			sizeof(id_event_struct_data_t))

#define	LOC_EVENT_SIZE	(sizeof(event2_header_t) + \
			sizeof(tpmt_ha) + TCG_DIGEST_SIZE + \
			sizeof(event2_data_t) + \
			sizeof(startup_locality_event_t))

#define	LOG_MIN_SIZE	(ID_EVENT_SIZE + LOC_EVENT_SIZE)

#define EVENT2_HDR_SIZE	(sizeof(event2_header_t) + \
			sizeof(tpmt_ha) + TCG_DIGEST_SIZE + \
			sizeof(event2_data_t))

/* Functions' declarations */

/**
 * Initialize the Event Log buffer.
 *
 * Sets global pointers to manage the Event Log memory region,
 * allowing subsequent log operations to write into the buffer.
 *
 * @param[in] event_log_start  Pointer to the start of the Event Log buffer.
 * @param[in] event_log_finish Pointer to the end of the buffer
 *                             (i.e., one byte past the last valid address).
 *
 * @return 0 on success, or -EINVAL if the input range is invalid.
 */
int event_log_buf_init(uint8_t *event_log_start, uint8_t *event_log_finish);

/**
 * Dump the contents of the Event Log.
 *
 * Outputs the raw contents of the Event Log buffer, typically
 * for debugging or audit purposes.
 *
 * @param[in] log_addr Pointer to the start of the Event Log buffer.
 * @param[in] log_size Size of the Event Log buffer in bytes.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int event_log_dump(uint8_t *log_addr, size_t log_size);

/**
 * Initialize the Event Log subsystem.
 *
 * Wrapper around `event_log_buf_init()` to configure the memory range
 * for the Event Log buffer.
 *
 * @param[in] event_log_start  Pointer to the start of the Event Log buffer.
 * @param[in] event_log_finish Pointer to the end of the buffer
 *                             (i.e., one byte past the last valid address).
 *
 * @return 0 on success, or a negative error code on failure.
 */
int event_log_init(uint8_t *event_log_start, uint8_t *event_log_finish);

/**
 * Measure input data and log its hash to the Event Log.
 *
 * Computes the cryptographic hash of the specified data and records it
 * in the Event Log as a TCG_PCR_EVENT2 structure using event type EV_POST_CODE.
 * Useful for firmware or image attestation.
 *
 * @param[in] data_base     Pointer to the base of the data to be measured.
 * @param[in] data_size     Size of the data in bytes.
 * @param[in] data_id       Identifier used to match against metadata.
 * @param[in] metadata_ptr  Pointer to an array of event_log_metadata_t.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int event_log_measure_and_record(uintptr_t data_base, uint32_t data_size,
				 uint32_t data_id,
				 const event_log_metadata_t *metadata_ptr);

/**
 * Measure the input data and return its hash.
 *
 * Computes the cryptographic hash of the specified memory region using
 * the default hashing algorithm configured in the Event Log subsystem.
 *
 * @param[in]  data_base  Pointer to the base of the data to be measured.
 * @param[in]  data_size  Size of the data in bytes.
 * @param[out] hash_data  Buffer to hold the resulting hash output
 *                        (must be at least CRYPTO_MD_MAX_SIZE bytes).
 *
 * @return 0 on success, or an error code on failure.
 */
int event_log_measure(uintptr_t data_base, uint32_t data_size,
		      unsigned char hash_data[CRYPTO_MD_MAX_SIZE]);

/**
 * Record a measurement event in the Event Log.
 *
 * Writes a TCG_PCR_EVENT2 structure to the Event Log using the
 * provided hash and metadata. This function assumes the buffer
 * has enough space and that `event_log_buf_init()` has been called.
 *
 * @param[in] hash         Pointer to the digest (TCG_DIGEST_SIZE bytes).
 * @param[in] event_type   Type of the event, as defined in tcg.h.
 * @param[in] metadata_ptr Pointer to an event_log_metadata_t structure
 *                         providing event-specific context (e.g., PCR index, name).
 *
 * @return 0 on success, or -ENOMEM if the buffer has insufficient space.
 */
int event_log_record(const uint8_t *hash, uint32_t event_type,
		     const event_log_metadata_t *metadata_ptr);

/**
 * Initialize the Event Log with mandatory header events.
 *
 * Writes the Specification ID (SpecID) and Startup Locality events
 * as required by the TCG PC Client Platform Firmware Profile.
 * These must be the first entries in the Event Log.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int event_log_write_header(void);

/**
 * Write the SpecID event to the Event Log.
 *
 * Records the TCG_EfiSpecIDEventStruct to declare the structure
 * and supported algorithms of the Event Log format.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int event_log_write_specid_event(void);

/**
 * Get the current size of the Event Log.
 *
 * Calculates how many bytes of the Event Log buffer have been used,
 * based on the current log pointer and the start of the buffer.
 *
 * @param[in] event_log_start Pointer to the start of the Event Log buffer.
 *
 * @return The number of bytes currently used in the Event Log.
 */
size_t event_log_get_cur_size(uint8_t *event_log_start);

#endif /* EVENT_LOG_H */
