/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include "emmc_config.h"
#include "emmc_hal.h"
#include "emmc_std.h"
#include "emmc_registers.h"
#include "emmc_def.h"

static const uint32_t cmd_reg_hw[EMMC_CMD_MAX + 1] = {
	0x00000000,		/* CMD0 */
	0x00000701,		/* CMD1 */
	0x00000002,		/* CMD2 */
	0x00000003,		/* CMD3 */
	0x00000004,		/* CMD4 */
	0x00000505,		/* CMD5 */
	0x00000406,		/* CMD6 */
	0x00000007,		/* CMD7 */
	0x00001C08,		/* CMD8 */
	0x00000009,		/* CMD9 */
	0x0000000A,		/* CMD10 */
	0x00000000,		/* reserved */
	0x0000000C,		/* CMD12 */
	0x0000000D,		/* CMD13 */
	0x00001C0E,		/* CMD14 */
	0x0000000F,		/* CMD15 */
	0x00000010,		/* CMD16 */
	0x00000011,		/* CMD17 */
	0x00007C12,		/* CMD18 */
	0x00000C13,		/* CMD19 */
	0x00000000,
	0x00001C15,		/* CMD21 */
	0x00000000,
	0x00000017,		/* CMD23 */
	0x00000018,		/* CMD24 */
	0x00006C19,		/* CMD25 */
	0x00000C1A,		/* CMD26 */
	0x0000001B,		/* CMD27 */
	0x0000001C,		/* CMD28 */
	0x0000001D,		/* CMD29 */
	0x0000001E,		/* CMD30 */
	0x00001C1F,		/* CMD31 */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000423,		/* CMD35 */
	0x00000424,		/* CMD36 */
	0x00000000,
	0x00000026,		/* CMD38 */
	0x00000427,		/* CMD39 */
	0x00000428,		/* CMD40(send cmd) */
	0x00000000,
	0x0000002A,		/* CMD42 */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000C31,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00007C35,
	0x00006C36,
	0x00000037,		/* CMD55 */
	0x00000038,		/* CMD56(Read) */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000
};

uint32_t emmc_bit_field(uint8_t *data, uint32_t top, uint32_t bottom)
{
	uint32_t value;

	uint32_t index_top = (uint32_t) (15 - (top >> 3));
	uint32_t index_bottom = (uint32_t) (15 - (bottom >> 3));

	if (index_top == index_bottom) {
		value = data[index_top];
	} else if ((index_top + 1) == index_bottom) {
		value =
		    (uint32_t) ((data[index_top] << 8) | data[index_bottom]);
	} else if ((index_top + 2) == index_bottom) {
		value =
		    (uint32_t) ((data[index_top] << 16) |
				(data[index_top + 1] << 8) | data[index_top +
								  2]);
	} else {
		value =
		    (uint32_t) ((data[index_top] << 24) |
				(data[index_top + 1] << 16) |
				(data[index_top + 2] << 8) | data[index_top +
								  3]);
	}

	value = ((value >> (bottom & 0x07)) & ((1 << (top - bottom + 1)) - 1));

	return value;
}

void emmc_write_error_info(uint16_t func_no, EMMC_ERROR_CODE error_code)
{

	mmc_drv_obj.error_info.num = func_no;
	mmc_drv_obj.error_info.code = (uint16_t) error_code;

	ERROR("BL2: emmc err:func_no=0x%x code=0x%x\n", func_no, error_code);
}

void emmc_write_error_info_func_no(uint16_t func_no)
{

	mmc_drv_obj.error_info.num = func_no;

	ERROR("BL2: emmc err:func_no=0x%x\n", func_no);
}

void emmc_make_nontrans_cmd(HAL_MEMCARD_COMMAND cmd, uint32_t arg)
{
	/* command information */
	mmc_drv_obj.cmd_info.cmd = cmd;
	mmc_drv_obj.cmd_info.arg = arg;
	mmc_drv_obj.cmd_info.dir = HAL_MEMCARD_READ;
	mmc_drv_obj.cmd_info.hw =
	    cmd_reg_hw[cmd & HAL_MEMCARD_COMMAND_INDEX_MASK];

	/* clear data transfer information */
	mmc_drv_obj.trans_size = 0;
	mmc_drv_obj.remain_size = 0;
	mmc_drv_obj.buff_address_virtual = NULL;
	mmc_drv_obj.buff_address_physical = NULL;

	/* response information */
	mmc_drv_obj.response_length = 6;

	switch (mmc_drv_obj.cmd_info.cmd & HAL_MEMCARD_RESPONSE_TYPE_MASK) {
	case HAL_MEMCARD_RESPONSE_NONE:
		mmc_drv_obj.response = (uint32_t *) mmc_drv_obj.response_data;
		mmc_drv_obj.response_length = 0;
		break;
	case HAL_MEMCARD_RESPONSE_R1:
		mmc_drv_obj.response = &mmc_drv_obj.r1_card_status;
		break;
	case HAL_MEMCARD_RESPONSE_R1b:
		mmc_drv_obj.cmd_info.hw |= BIT10;	/* bit10 = R1 busy bit */
		mmc_drv_obj.response = &mmc_drv_obj.r1_card_status;
		break;
	case HAL_MEMCARD_RESPONSE_R2:
		mmc_drv_obj.response = (uint32_t *) mmc_drv_obj.response_data;
		mmc_drv_obj.response_length = 17;
		break;
	case HAL_MEMCARD_RESPONSE_R3:
		mmc_drv_obj.response = &mmc_drv_obj.r3_ocr;
		break;
	case HAL_MEMCARD_RESPONSE_R4:
		mmc_drv_obj.response = &mmc_drv_obj.r4_resp;
		break;
	case HAL_MEMCARD_RESPONSE_R5:
		mmc_drv_obj.response = &mmc_drv_obj.r5_resp;
		break;
	default:
		mmc_drv_obj.response = (uint32_t *) mmc_drv_obj.response_data;
		break;
	}
}

void emmc_make_trans_cmd(HAL_MEMCARD_COMMAND cmd, uint32_t arg,
			 uint32_t *buff_address_virtual,
			 uint32_t len,
			 HAL_MEMCARD_OPERATION dir,
			 HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode)
{
	emmc_make_nontrans_cmd(cmd, arg);	/* update common information */

	/* for data transfer command */
	mmc_drv_obj.cmd_info.dir = dir;
	mmc_drv_obj.buff_address_virtual = buff_address_virtual;
	mmc_drv_obj.buff_address_physical = buff_address_virtual;
	mmc_drv_obj.trans_size = len;
	mmc_drv_obj.remain_size = len;
	mmc_drv_obj.transfer_mode = transfer_mode;
}

EMMC_ERROR_CODE emmc_send_idle_cmd(uint32_t arg)
{
	EMMC_ERROR_CODE result;
	uint32_t freq;

	/* initialize state */
	mmc_drv_obj.mount = FALSE;
	mmc_drv_obj.selected = FALSE;
	mmc_drv_obj.during_transfer = FALSE;
	mmc_drv_obj.during_cmd_processing = FALSE;
	mmc_drv_obj.during_dma_transfer = FALSE;
	mmc_drv_obj.dma_error_flag = FALSE;
	mmc_drv_obj.force_terminate = FALSE;
	mmc_drv_obj.state_machine_blocking = FALSE;

	mmc_drv_obj.bus_width = HAL_MEMCARD_DATA_WIDTH_1_BIT;
	mmc_drv_obj.max_freq = MMC_20MHZ;	/* 20MHz */
	mmc_drv_obj.current_state = EMMC_R1_STATE_IDLE;

	/* CMD0 (MMC clock is current frequency. if Data transfer mode, 20MHz or higher.) */
	emmc_make_nontrans_cmd(CMD0_GO_IDLE_STATE, arg);	/* CMD0 */
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		return result;
	}

	/* change MMC clock(400KHz) */
	freq = MMC_400KHZ;
	result = emmc_set_request_mmc_clock(&freq);
	if (result != EMMC_SUCCESS) {
		return result;
	}

	return EMMC_SUCCESS;
}
