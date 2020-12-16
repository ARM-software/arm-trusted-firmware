/*
 * Copyright (c) 2015-2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include "emmc_config.h"
#include "emmc_def.h"
#include "emmc_hal.h"
#include "emmc_registers.h"
#include "emmc_std.h"

#define MIN_EMMC(a, b)	(((a) < (b)) ? (a) : (b))
#define EMMC_RW_SECTOR_COUNT_MAX	0x0000ffffU

static EMMC_ERROR_CODE emmc_multiple_block_read(uint32_t *buff_address_virtual,
		uint32_t sector_number, uint32_t count,
		HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode)
{
	EMMC_ERROR_CODE result;

	/* parameter check */
	if ((count > EMMC_RW_SECTOR_COUNT_MAX)
	    || (count == 0)
	    || ((transfer_mode != HAL_MEMCARD_DMA)
		&& (transfer_mode != HAL_MEMCARD_NOT_DMA))
	    ) {
		emmc_write_error_info(EMMC_FUNCNO_READ_SECTOR, EMMC_ERR_PARAM);
		return EMMC_ERR_PARAM;
	}

	/* CMD23 */
	emmc_make_nontrans_cmd(CMD23_SET_BLOCK_COUNT, count);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		return result;
	}
	SETR_32(SD_SECCNT, count);
	SETR_32(SD_STOP, 0x00000100);
	/* SD_BUF Read/Write DMA Transfer enable */
	SETR_32(CC_EXT_MODE, (CC_EXT_MODE_CLEAR | CC_EXT_MODE_DMASDRW_ENABLE));

	/* CMD18 */
	emmc_make_trans_cmd(CMD18_READ_MULTIPLE_BLOCK, sector_number,
			    buff_address_virtual,
			    count << EMMC_SECTOR_SIZE_SHIFT, HAL_MEMCARD_READ,
			    transfer_mode);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		return result;	/* CMD18 error code */
	}

	/* CMD13 */
	emmc_make_nontrans_cmd(CMD13_SEND_STATUS, EMMC_RCA << 16);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		return result;
	}
#if RCAR_BL2_DCACHE == 1
	if (transfer_mode == HAL_MEMCARD_NOT_DMA) {
		flush_dcache_range((uint64_t) buff_address_virtual,
				   ((size_t) count << EMMC_SECTOR_SIZE_SHIFT));
	}
#endif /* RCAR_BL2_DCACHE == 1 */

	/* ready status check */
	if ((mmc_drv_obj.r1_card_status & EMMC_R1_READY) == 0) {
		emmc_write_error_info(EMMC_FUNCNO_READ_SECTOR,
				      EMMC_ERR_CARD_BUSY);
		return EMMC_ERR_CARD_BUSY;
	}

	/* state check */
	if (mmc_drv_obj.current_state != EMMC_R1_STATE_TRAN) {
		emmc_write_error_info(EMMC_FUNCNO_READ_SECTOR,
				      EMMC_ERR_CARD_STATE);
		return EMMC_ERR_CARD_STATE;
	}

	return EMMC_SUCCESS;
}

EMMC_ERROR_CODE emmc_read_sector(uint32_t *buff_address_virtual,
				 uint32_t sector_number,
				 uint32_t count, uint32_t feature_flags)
{
	uint32_t trans_count;
	uint32_t remain;
	EMMC_ERROR_CODE result;
	HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode;

	/* parameter check */
	if (count == 0) {
		emmc_write_error_info(EMMC_FUNCNO_READ_SECTOR, EMMC_ERR_PARAM);
		return EMMC_ERR_PARAM;
	}

	/* state check */
	if (mmc_drv_obj.mount != TRUE) {
		emmc_write_error_info(EMMC_FUNCNO_READ_SECTOR, EMMC_ERR_STATE);
		return EMMC_ERR_STATE;
	}

	/* DMA? */
	if ((feature_flags & LOADIMAGE_FLAGS_DMA_ENABLE) != 0) {
		transfer_mode = HAL_MEMCARD_DMA;
	} else {
		transfer_mode = HAL_MEMCARD_NOT_DMA;
	}

	remain = count;
	while (remain != 0) {
		trans_count = MIN_EMMC(remain, EMMC_RW_SECTOR_COUNT_MAX);
		result =
		    emmc_multiple_block_read(buff_address_virtual,
					     sector_number, trans_count,
					     transfer_mode);
		if (result != EMMC_SUCCESS) {
			return result;
		}

		buff_address_virtual += (EMMC_BLOCK_LENGTH_DW * trans_count);
		sector_number += trans_count;
		remain -= trans_count;
	}

	return EMMC_SUCCESS;
}
