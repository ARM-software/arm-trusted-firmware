/*
 * Copyright (c) 2020-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdint.h>

#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/event_log/tcg.h>

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
#error "Not supported EVENT_LOG_LEVEL"
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
void event_log_buf_init(uint8_t *event_log_start, uint8_t *event_log_finish);
void event_log_init(uint8_t *event_log_start, uint8_t *event_log_finish);
void event_log_write_specid_event(void);
void event_log_write_header(void);
void dump_event_log(uint8_t *log_addr, size_t log_size);
int event_log_measure(uintptr_t data_base, uint32_t data_size,
		      unsigned char hash_data[CRYPTO_MD_MAX_SIZE]);
void event_log_record(const uint8_t *hash, uint32_t event_type,
		      const event_log_metadata_t *metadata_ptr);
int event_log_measure_and_record(uintptr_t data_base, uint32_t data_size,
				 uint32_t data_id,
				 const event_log_metadata_t *metadata_ptr);
size_t event_log_get_cur_size(uint8_t *event_log_start);

#endif /* EVENT_LOG_H */
