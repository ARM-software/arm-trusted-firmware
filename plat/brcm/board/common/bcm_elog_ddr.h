/*
 * Copyright 2019-2020 Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BCM_ELOG_DDR_H
#define BCM_ELOG_DDR_H

#define ELOG_GLOBAL_META_HDR_SIG	0x45524c47
#define ELOG_MAX_REC_COUNT		13
#define ELOG_MAX_REC_FORMAT		1
#define ELOG_MAX_NVM_TYPE		4
/* Use a default NVM, set by m0 configuration */
#define ELOG_NVM_DEFAULT		0xff

/* Max. number of cmd parameters per elog spec */
#define ELOG_PARAM_COUNT		3
/*
 * Number of supported RECORD Types-
 * SYSRESET, THERMAL, DDR_ECC, APBOOTLG, IDM
 */
#define ELOG_SUPPORTED_REC_CNT		5

#define ELOG_REC_DESC_LENGTH		8

#define ELOG_SECTOR_SIZE		0x1000

/* Default Record size for all record types except APBOOTLOG */
#define ELOG_DEFAULT_REC_SIZE		0x10000

/* Default record size for APBOOTLOG record */
#define ELOG_APBOOTLG_REC_SIZE		0x60000

/* Use default CRMU provided mem address */
#define ELOG_USE_DEFAULT_MEM_ADDR	0x0

/* Temporary place to hold metadata */
#define TMP_ELOG_METADATA_BASE		(ELOG_AP_UART_LOG_BASE + \
					 BCM_ELOG_BL2_SIZE)
/* IDM ELOG source memory address */
#define ELOG_IDM_SRC_MEM_ADDR		0x8f213000

#define IDM_ELOG_REC_TYPE		5

enum elog_record_type {
	ELOG_REC_SYS_RESET_EVT = 0,
	ELOG_REC_THERMAL_EVT,
	ELOG_REC_DDR_ECC,
	ELOG_REC_UART_LOG,
	ELOG_REC_IDM_LOG,
	ELOG_REC_MAX
};

enum elog_record_format {
	ELOG_REC_FMT_ASCII = 0,
	ELOG_REC_FMT_CUSTOM
};

enum elog_src_memory_type {
	ELOG_SRC_MEM_TYPE_CRMU_SCRATCH = 0,
	ELOG_SRC_MEM_TYPE_FS4_SCRATCH,
	ELOG_SRC_MEM_TYPE_DDR,
	ELOG_SRC_MEM_TYPE_CHIMP_SCRATCH
};

enum elog_setup_cmd {
	ELOG_SETUP_CMD_VALIDATE_META,
	ELOG_SETUP_CMD_WRITE_META,
	ELOG_SETUP_CMD_ERASE,
	ELOG_SETUP_CMD_READ,
	ELOG_SETUP_CMD_CHECK
};

struct elog_setup {
	uint32_t cmd;
	uint32_t params[ELOG_PARAM_COUNT];
	uint32_t result;
	uint32_t ret_code;
};

struct elog_meta_record {
	uint8_t type;
	uint8_t format;
	uint8_t src_mem_type;
	uint8_t alt_src_mem_type;
	uint8_t nvm_type;
	char rec_desc[ELOG_REC_DESC_LENGTH];
	uint64_t src_mem_addr;
	uint64_t alt_src_mem_addr;
	uint64_t rec_addr;
	uint32_t rec_size;
	uint32_t sector_size;
	uint8_t padding[3];
} __packed;

struct elog_global_header {
	uint32_t signature;
	uint32_t sector_size;
	uint8_t revision;
	uint8_t rec_count;
	uint16_t padding;
} __packed;

void elog_init_ddr_log(void);

#endif /* BCM_ELOG_DDR_H */
