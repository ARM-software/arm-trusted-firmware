/*
 * Copyright 2019-2020 Broadcom.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>

#include <ddr_init.h>
#include <scp_cmd.h>
#include <scp_utils.h>
#include <platform_def.h>

#include "bcm_elog_ddr.h"
#include "m0_cfg.h"
#include "m0_ipc.h"

void elog_init_ddr_log(void)
{
	struct elog_setup setup = {0};
	struct elog_global_header global;
	struct elog_meta_record rec;
	unsigned int rec_idx = 0;
	uint32_t log_offset;
	uintptr_t metadata;
	char *rec_desc[ELOG_SUPPORTED_REC_CNT] = {"SYSRESET", "THERMAL",
						  "DDR_ECC", "APBOOTLG",
						  "IDM"};

	/*
	 * If this is warm boot, return immediately.
	 * We expect metadata to be initialized already
	 */
	if (is_warmboot()) {
		WARN("Warmboot detected, skip ELOG metadata initialization\n");
		return;
	}

	memset(&global, 0, sizeof(global));

	global.sector_size = ELOG_SECTOR_SIZE;
	global.signature = ELOG_GLOBAL_META_HDR_SIG;
	global.rec_count = ELOG_SUPPORTED_REC_CNT;

	/* Start of logging area in DDR memory */
	log_offset = ELOG_STORE_OFFSET;

	/* Shift to the first RECORD header */
	log_offset += 2 * global.sector_size;

	/* Temporary place to hold metadata */
	metadata = TMP_ELOG_METADATA_BASE;

	memcpy((void *)metadata, &global, sizeof(global));
	metadata += sizeof(global);

	while (rec_idx < global.rec_count) {
		memset(&rec, 0, sizeof(rec));

		rec.type = rec_idx;
		if (rec_idx == ELOG_REC_UART_LOG) {
			rec.format =  ELOG_REC_FMT_ASCII;
			rec.src_mem_type =  ELOG_SRC_MEM_TYPE_DDR;
			rec.alt_src_mem_type =  ELOG_SRC_MEM_TYPE_FS4_SCRATCH;
			rec.src_mem_addr =  BCM_ELOG_BL31_BASE;
			rec.alt_src_mem_addr =  BCM_ELOG_BL2_BASE;
			rec.rec_size =  ELOG_APBOOTLG_REC_SIZE;
		} else if (rec_idx == ELOG_REC_IDM_LOG) {
			rec.type = IDM_ELOG_REC_TYPE;
			rec.format = ELOG_REC_FMT_CUSTOM;
			rec.src_mem_type =  ELOG_SRC_MEM_TYPE_DDR;
			rec.alt_src_mem_type = ELOG_SRC_MEM_TYPE_CRMU_SCRATCH;
			rec.src_mem_addr =  ELOG_IDM_SRC_MEM_ADDR;
			rec.alt_src_mem_addr =  0x0;
			rec.rec_size =  ELOG_DEFAULT_REC_SIZE;
		} else {
			rec.format = ELOG_REC_FMT_CUSTOM;
			rec.src_mem_type = ELOG_SRC_MEM_TYPE_CRMU_SCRATCH;
			rec.alt_src_mem_type = ELOG_SRC_MEM_TYPE_CRMU_SCRATCH;
			rec.src_mem_addr = ELOG_USE_DEFAULT_MEM_ADDR;
			rec.alt_src_mem_addr = ELOG_USE_DEFAULT_MEM_ADDR;
			rec.rec_size = ELOG_DEFAULT_REC_SIZE;
		}

		rec.nvm_type = LOG_MEDIA_DDR;
		rec.sector_size = ELOG_SECTOR_SIZE;

		rec.rec_addr = (uint64_t)log_offset;
		log_offset += rec.rec_size;

		/* Sanity checks */
		if (rec.type > ELOG_MAX_REC_COUNT ||
		    rec.format > ELOG_MAX_REC_FORMAT ||
		    (rec.nvm_type > ELOG_MAX_NVM_TYPE &&
		     rec.nvm_type != ELOG_NVM_DEFAULT) ||
		    !rec.rec_size ||
		    !rec.sector_size ||
		    rec_idx >= ELOG_SUPPORTED_REC_CNT) {
			ERROR("Invalid ELOG record(%u) detected\n", rec_idx);
			return;
		}

		memset(rec.rec_desc, ' ', sizeof(rec.rec_desc));

		memcpy(rec.rec_desc, rec_desc[rec_idx],
		       strlen(rec_desc[rec_idx]));

		memcpy((void *)metadata, &rec, sizeof(rec));
		metadata += sizeof(rec);

		rec_idx++;
	}

	setup.params[0] = TMP_ELOG_METADATA_BASE;
	setup.params[1] = (sizeof(global) + global.rec_count * sizeof(rec));
	setup.cmd = ELOG_SETUP_CMD_WRITE_META;

	flush_dcache_range((uintptr_t)&setup, sizeof(struct elog_setup));
	flush_dcache_range((uintptr_t)setup.params[0], setup.params[1]);

	/* initialize DDR Logging METADATA if this is NOT warmboot */
	if (!is_warmboot()) {
		if (scp_send_cmd(MCU_IPC_MCU_CMD_ELOG_SETUP,
				 (uint32_t)(uintptr_t)(&setup),
				 SCP_CMD_DEFAULT_TIMEOUT_US)) {
			ERROR("scp_send_cmd: timeout/error for elog setup\n");
			return;
		}
	}

	NOTICE("MCU Error logging initialized\n");
}
