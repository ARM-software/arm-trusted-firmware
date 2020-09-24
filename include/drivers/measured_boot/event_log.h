/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdint.h>

#include <common/debug.h>
#include <drivers/measured_boot/tcg.h>

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
#define HASH_ALG_COUNT	1U

#define	INVALID_ID	MAX_NUMBER_IDS

#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

#define BL2_STRING			"BL_2"
#define BL31_STRING			"BL_31"
#define BL32_STRING			"BL_32"
#define	BL32_EXTRA1_IMAGE_STRING	"BL32_EXTRA1_IMAGE"
#define	BL32_EXTRA2_IMAGE_STRING	"BL32_EXTRA2_IMAGE"
#define BL33_STRING			"BL_33"
#define GPT_IMAGE_STRING		"GPT"
#define HW_CONFIG_STRING		"HW_CONFIG"
#define NT_FW_CONFIG_STRING		"NT_FW_CONFIG"
#define SCP_BL2_IMAGE_STRING		"SCP_BL2_IMAGE"
#define SOC_FW_CONFIG_STRING		"SOC_FW_CONFIG"
#define STM32_IMAGE_STRING		"STM32"
#define	TOS_FW_CONFIG_STRING		"TOS_FW_CONFIG"

typedef struct {
	unsigned int id;
	const char *name;
	unsigned int pcr;
} image_data_t;

typedef struct {
	const image_data_t *images_data;
	int (*set_nt_fw_info)(uintptr_t config_base,
#ifdef SPD_opteed
				uintptr_t log_addr,
#endif
				size_t log_size, uintptr_t *ns_log_addr);
	int (*set_tos_fw_info)(uintptr_t config_base, uintptr_t log_addr,
				size_t log_size);
} measured_boot_data_t;

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
void event_log_init(void);
int event_log_finalise(uint8_t **log_addr, size_t *log_size);
void dump_event_log(uint8_t *log_addr, size_t log_size);
const measured_boot_data_t *plat_get_measured_boot_data(void);
int tpm_record_measurement(uintptr_t data_base, uint32_t data_size,
			   uint32_t data_id);
#endif /* EVENT_LOG_H */
