/*
 * Copyright (c) 2020-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdint.h>

#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
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

/*
 * Each event log entry has some metadata (i.e. a string) that identifies
 * what is measured.These macros define these strings.
 * Note that these strings follow the standardization recommendations
 * defined in the Arm Server Base Security Guide (a.k.a. SBSG, Arm DEN 0086),
 * where applicable. They should not be changed in the code.
 * Where the SBSG does not make recommendations, we are free to choose any
 * naming convention.
 * The key thing is to choose meaningful strings so that when the TPM event
 * log is used in attestation, the different components can be identified.
 */
#define EVLOG_BL2_STRING		"BL_2"
#define EVLOG_BL31_STRING		"SECURE_RT_EL3"
#if defined(SPD_opteed)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_OPTEE"
#elif defined(SPD_tspd)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_TSPD"
#elif defined(SPD_tlkd)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_TLKD"
#elif defined(SPD_trusty)
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_TRUSTY"
#else
#define EVLOG_BL32_STRING		"SECURE_RT_EL1_UNKNOWN"
#endif
#define	EVLOG_BL32_EXTRA1_STRING	"SECURE_RT_EL1_OPTEE_EXTRA1"
#define	EVLOG_BL32_EXTRA2_STRING	"SECURE_RT_EL1_OPTEE_EXTRA2"
#define EVLOG_BL33_STRING		"BL_33"
#define EVLOG_FW_CONFIG_STRING		"FW_CONFIG"
#define EVLOG_HW_CONFIG_STRING		"HW_CONFIG"
#define EVLOG_NT_FW_CONFIG_STRING	"NT_FW_CONFIG"
#define EVLOG_SCP_BL2_STRING		"SYS_CTRL_2"
#define EVLOG_SOC_FW_CONFIG_STRING	"SOC_FW_CONFIG"
#define EVLOG_STM32_STRING		"STM32"
#define EVLOG_TB_FW_CONFIG_STRING	"TB_FW_CONFIG"
#define	EVLOG_TOS_FW_CONFIG_STRING	"TOS_FW_CONFIG"
#define EVLOG_RMM_STRING 		"RMM"
#define EVLOG_SP1_STRING		"SP1"
#define EVLOG_SP2_STRING		"SP2"
#define EVLOG_SP3_STRING		"SP3"
#define EVLOG_SP4_STRING		"SP4"
#define EVLOG_SP5_STRING		"SP5"
#define EVLOG_SP6_STRING		"SP6"
#define EVLOG_SP7_STRING		"SP7"
#define EVLOG_SP8_STRING		"SP8"

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
void event_log_init(uint8_t *event_log_start, uint8_t *event_log_finish);
void event_log_write_header(void);
void dump_event_log(uint8_t *log_addr, size_t log_size);
const event_log_metadata_t *plat_event_log_get_metadata(void);
int event_log_measure_and_record(uintptr_t data_base, uint32_t data_size,
				 uint32_t data_id);
size_t event_log_get_cur_size(uint8_t *event_log_start);

#endif /* EVENT_LOG_H */
