/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include "emmc_config.h"
#include "emmc_hal.h"
#include "emmc_std.h"
#include "emmc_registers.h"
#include "emmc_def.h"
#include "micro_delay.h"
#include "rcar_def.h"

static EMMC_ERROR_CODE emmc_clock_ctrl(uint8_t mode);
static EMMC_ERROR_CODE emmc_card_init(void);
static EMMC_ERROR_CODE emmc_high_speed(void);
static EMMC_ERROR_CODE emmc_bus_width(uint32_t width);
static uint32_t emmc_set_timeout_register_value(uint32_t freq);
static void set_sd_clk(uint32_t clkDiv);
static uint32_t emmc_calc_tran_speed(uint32_t *freq);
static void emmc_get_partition_access(void);
static void emmc_set_bootpartition(void);

static void emmc_set_bootpartition(void)
{
	uint32_t reg;

	reg = mmio_read_32(RCAR_PRR) & (PRR_PRODUCT_MASK | PRR_CUT_MASK);
	if (reg == PRR_PRODUCT_M3_CUT10) {
		mmc_drv_obj.boot_partition_en =
		    (EMMC_PARTITION_ID) ((mmc_drv_obj.ext_csd_data[179] &
					  EMMC_BOOT_PARTITION_EN_MASK) >>
					 EMMC_BOOT_PARTITION_EN_SHIFT);
	} else if ((reg == PRR_PRODUCT_H3_CUT20)
		   || (reg == PRR_PRODUCT_M3_CUT11)) {
		mmc_drv_obj.boot_partition_en = mmc_drv_obj.partition_access;
	} else {
		if ((mmio_read_32(MFISBTSTSR) & MFISBTSTSR_BOOT_PARTITION) !=
		    0U) {
			mmc_drv_obj.boot_partition_en = PARTITION_ID_BOOT_2;
		} else {
			mmc_drv_obj.boot_partition_en = PARTITION_ID_BOOT_1;
		}
	}
}

static EMMC_ERROR_CODE emmc_card_init(void)
{
	int32_t retry;
	uint32_t freq = MMC_400KHZ;	/* 390KHz */
	EMMC_ERROR_CODE result;
	uint32_t resultCalc;

	/* state check */
	if ((mmc_drv_obj.initialize != TRUE)
	    || (mmc_drv_obj.card_power_enable != TRUE)
	    || ((GETR_32(SD_INFO2) & SD_INFO2_CBSY) != 0)
	    ) {
		emmc_write_error_info(EMMC_FUNCNO_CARD_INIT, EMMC_ERR_STATE);
		return EMMC_ERR_STATE;
	}

	/* clock on (force change) */
	mmc_drv_obj.current_freq = 0;
	mmc_drv_obj.max_freq = MMC_20MHZ;
	result = emmc_set_request_mmc_clock(&freq);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return EMMC_ERR;
	}

	rcar_micro_delay(1000U);	/* wait 1ms */

	/* Get current access partition */
	emmc_get_partition_access();

	/* CMD0, arg=0x00000000 */
	result = emmc_send_idle_cmd(0x00000000);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return result;
	}

	rcar_micro_delay(200U);	/* wait 74clock 390kHz(189.74us) */

	/* CMD1 */
	emmc_make_nontrans_cmd(CMD1_SEND_OP_COND, EMMC_HOST_OCR_VALUE);
	for (retry = 300; retry > 0; retry--) {
		result =
		    emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
		if (result != EMMC_SUCCESS) {
			emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
			return result;
		}

		if ((mmc_drv_obj.r3_ocr & EMMC_OCR_STATUS_BIT) != 0) {
			break;	/* card is ready. exit loop */
		}
		rcar_micro_delay(1000U);	/* wait 1ms */
	}

	if (retry == 0) {
		emmc_write_error_info(EMMC_FUNCNO_CARD_INIT, EMMC_ERR_TIMEOUT);
		return EMMC_ERR_TIMEOUT;
	}

	switch (mmc_drv_obj.r3_ocr & EMMC_OCR_ACCESS_MODE_MASK) {
	case EMMC_OCR_ACCESS_MODE_SECT:
		mmc_drv_obj.access_mode = TRUE;	/* sector mode */
		break;
	default:
		/* unknown value */
		emmc_write_error_info(EMMC_FUNCNO_CARD_INIT, EMMC_ERR);
		return EMMC_ERR;
	}

	/* CMD2 */
	emmc_make_nontrans_cmd(CMD2_ALL_SEND_CID_MMC, 0x00000000);
	mmc_drv_obj.response = (uint32_t *) (&mmc_drv_obj.cid_data[0]);	/* use CID special buffer */
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return result;
	}

	/* CMD3 */
	emmc_make_nontrans_cmd(CMD3_SET_RELATIVE_ADDR, EMMC_RCA << 16);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return result;
	}

	/* CMD9 (CSD) */
	emmc_make_nontrans_cmd(CMD9_SEND_CSD, EMMC_RCA << 16);
	mmc_drv_obj.response = (uint32_t *) (&mmc_drv_obj.csd_data[0]);	/* use CSD special buffer */
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return result;
	}

	/* card version check */
	if (EMMC_CSD_SPEC_VARS() < 4) {
		emmc_write_error_info(EMMC_FUNCNO_CARD_INIT,
				      EMMC_ERR_ILLEGAL_CARD);
		return EMMC_ERR_ILLEGAL_CARD;
	}

	/* CMD7 (select card) */
	emmc_make_nontrans_cmd(CMD7_SELECT_CARD, EMMC_RCA << 16);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return result;
	}

	mmc_drv_obj.selected = TRUE;

	/* card speed check */
	resultCalc = emmc_calc_tran_speed(&freq);	/* Card spec is calculated from TRAN_SPEED(CSD).  */
	if (resultCalc == 0) {
		emmc_write_error_info(EMMC_FUNCNO_CARD_INIT,
				      EMMC_ERR_ILLEGAL_CARD);
		return EMMC_ERR_ILLEGAL_CARD;
	}
	mmc_drv_obj.max_freq = freq;	/* max frequency (card spec) */

	result = emmc_set_request_mmc_clock(&freq);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return EMMC_ERR;
	}

	/* set read/write timeout */
	mmc_drv_obj.data_timeout = emmc_set_timeout_register_value(freq);
	SETR_32(SD_OPTION,
		((GETR_32(SD_OPTION) & ~(SD_OPTION_TIMEOUT_CNT_MASK)) |
		 mmc_drv_obj.data_timeout));

	/* SET_BLOCKLEN(512byte) */
	/* CMD16 */
	emmc_make_nontrans_cmd(CMD16_SET_BLOCKLEN, EMMC_BLOCK_LENGTH);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return result;
	}

	/* Transfer Data Length */
	SETR_32(SD_SIZE, EMMC_BLOCK_LENGTH);

	/* CMD8 (EXT_CSD) */
	emmc_make_trans_cmd(CMD8_SEND_EXT_CSD, 0x00000000,
			    (uint32_t *) (&mmc_drv_obj.ext_csd_data[0]),
			    EMMC_MAX_EXT_CSD_LENGTH, HAL_MEMCARD_READ,
			    HAL_MEMCARD_NOT_DMA);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		/* CMD12 is not send.
		 * If BUS initialization is failed, user must be execute Bus initialization again.
		 * Bus initialization is start CMD0(soft reset command).
		 */
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		return result;
	}

	/* Set boot partition */
	emmc_set_bootpartition();

	return EMMC_SUCCESS;
}

static EMMC_ERROR_CODE emmc_high_speed(void)
{
	uint32_t freq;	      /**< High speed mode clock frequency */
	EMMC_ERROR_CODE result;
	uint8_t cardType;

	/* state check */
	if (mmc_drv_obj.selected != TRUE) {
		emmc_write_error_info(EMMC_FUNCNO_HIGH_SPEED, EMMC_ERR_STATE);
		return EMMC_ERR_STATE;
	}

	/* max frequency */
	cardType = (uint8_t) mmc_drv_obj.ext_csd_data[EMMC_EXT_CSD_CARD_TYPE];
	if ((cardType & EMMC_EXT_CSD_CARD_TYPE_52MHZ) != 0)
		freq = MMC_52MHZ;
	else if ((cardType & EMMC_EXT_CSD_CARD_TYPE_26MHZ) != 0)
		freq = MMC_26MHZ;
	else
		freq = MMC_20MHZ;

	/* Hi-Speed-mode selction */
	if ((MMC_52MHZ == freq) || (MMC_26MHZ == freq)) {
		/* CMD6 */
		emmc_make_nontrans_cmd(CMD6_SWITCH, EMMC_SWITCH_HS_TIMING);
		result =
		    emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
		if (result != EMMC_SUCCESS) {
			emmc_write_error_info_func_no(EMMC_FUNCNO_HIGH_SPEED);
			return result;
		}

		mmc_drv_obj.hs_timing = TIMING_HIGH_SPEED;	/* High-Speed */
	}

	/* set mmc clock */
	mmc_drv_obj.max_freq = freq;
	result = emmc_set_request_mmc_clock(&freq);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_HIGH_SPEED);
		return EMMC_ERR;
	}

	/* set read/write timeout */
	mmc_drv_obj.data_timeout = emmc_set_timeout_register_value(freq);
	SETR_32(SD_OPTION,
		((GETR_32(SD_OPTION) & ~(SD_OPTION_TIMEOUT_CNT_MASK)) |
		 mmc_drv_obj.data_timeout));

	/* CMD13 */
	emmc_make_nontrans_cmd(CMD13_SEND_STATUS, EMMC_RCA << 16);
	result =
	    emmc_exec_cmd(EMMC_R1_ERROR_MASK_WITHOUT_CRC, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_HIGH_SPEED);
		return result;
	}

	return EMMC_SUCCESS;
}

static EMMC_ERROR_CODE emmc_clock_ctrl(uint8_t mode)
{
	uint32_t value;

	/* busy check */
	if ((GETR_32(SD_INFO2) & SD_INFO2_CBSY) != 0) {
		emmc_write_error_info(EMMC_FUNCNO_SET_CLOCK,
				      EMMC_ERR_CARD_BUSY);
		return EMMC_ERR;
	}

	if (mode == TRUE) {
		/* clock ON */
		value =
		    ((GETR_32(SD_CLK_CTRL) | MMC_SD_CLK_START) &
		     SD_CLK_WRITE_MASK);
		SETR_32(SD_CLK_CTRL, value);	/* on  */
		mmc_drv_obj.clock_enable = TRUE;
	} else {
		/* clock OFF */
		value =
		    ((GETR_32(SD_CLK_CTRL) & MMC_SD_CLK_STOP) &
		     SD_CLK_WRITE_MASK);
		SETR_32(SD_CLK_CTRL, value);	/* off */
		mmc_drv_obj.clock_enable = FALSE;
	}

	return EMMC_SUCCESS;
}

static EMMC_ERROR_CODE emmc_bus_width(uint32_t width)
{
	EMMC_ERROR_CODE result = EMMC_ERR;

	/* parameter check */
	if ((width != 8) && (width != 4) && (width != 1)) {
		emmc_write_error_info(EMMC_FUNCNO_BUS_WIDTH, EMMC_ERR_PARAM);
		return EMMC_ERR_PARAM;
	}

	/* state check */
	if (mmc_drv_obj.selected != TRUE) {
		emmc_write_error_info(EMMC_FUNCNO_BUS_WIDTH, EMMC_ERR_STATE);
		return EMMC_ERR_STATE;
	}

	mmc_drv_obj.bus_width = (HAL_MEMCARD_DATA_WIDTH) (width >> 2);	/* 2 = 8bit, 1 = 4bit, 0 =1bit */

	/* CMD6 */
	emmc_make_nontrans_cmd(CMD6_SWITCH,
			       (EMMC_SWITCH_BUS_WIDTH_1 |
				(mmc_drv_obj.bus_width << 8)));
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		/* occurred error */
		mmc_drv_obj.bus_width = HAL_MEMCARD_DATA_WIDTH_1_BIT;
		goto EXIT;
	}

	switch (mmc_drv_obj.bus_width) {
	case HAL_MEMCARD_DATA_WIDTH_1_BIT:
		SETR_32(SD_OPTION,
			((GETR_32(SD_OPTION) & ~(BIT15 | BIT13)) | BIT15));
		break;
	case HAL_MEMCARD_DATA_WIDTH_4_BIT:
		SETR_32(SD_OPTION, (GETR_32(SD_OPTION) & ~(BIT15 | BIT13)));
		break;
	case HAL_MEMCARD_DATA_WIDTH_8_BIT:
		SETR_32(SD_OPTION,
			((GETR_32(SD_OPTION) & ~(BIT15 | BIT13)) | BIT13));
		break;
	default:
		goto EXIT;
	}

	/* CMD13 */
	emmc_make_nontrans_cmd(CMD13_SEND_STATUS, EMMC_RCA << 16);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		goto EXIT;
	}

	/* CMD8 (EXT_CSD) */
	emmc_make_trans_cmd(CMD8_SEND_EXT_CSD, 0x00000000,
			    (uint32_t *) (&mmc_drv_obj.ext_csd_data[0]),
			    EMMC_MAX_EXT_CSD_LENGTH, HAL_MEMCARD_READ,
			    HAL_MEMCARD_NOT_DMA);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		goto EXIT;
	}

	return EMMC_SUCCESS;

EXIT:

	emmc_write_error_info(EMMC_FUNCNO_BUS_WIDTH, result);
	ERROR("BL2: emmc bus_width error end\n");
	return result;
}

EMMC_ERROR_CODE emmc_select_partition(EMMC_PARTITION_ID id)
{
	EMMC_ERROR_CODE result;
	uint32_t arg;
	uint32_t partition_config;

	/* state check */
	if (mmc_drv_obj.mount != TRUE) {
		emmc_write_error_info(EMMC_FUNCNO_NONE, EMMC_ERR_STATE);
		return EMMC_ERR_STATE;
	}

	/* id = PARTITION_ACCESS(Bit[2:0]) */
	if ((id & ~PARTITION_ID_MASK) != 0) {
		emmc_write_error_info(EMMC_FUNCNO_NONE, EMMC_ERR_PARAM);
		return EMMC_ERR_PARAM;
	}

	/* EXT_CSD[179] value */
	partition_config =
	    (uint32_t) mmc_drv_obj.ext_csd_data[EMMC_EXT_CSD_PARTITION_CONFIG];
	if ((partition_config & PARTITION_ID_MASK) == id) {
		result = EMMC_SUCCESS;
	} else {

		partition_config =
		    (uint32_t) ((partition_config & ~PARTITION_ID_MASK) | id);
		arg = EMMC_SWITCH_PARTITION_CONFIG | (partition_config << 8);

		result = emmc_set_ext_csd(arg);
	}

	return result;
}

static void set_sd_clk(uint32_t clkDiv)
{
	uint32_t dataL;

	dataL = (GETR_32(SD_CLK_CTRL) & (~SD_CLK_CTRL_CLKDIV_MASK));

	switch (clkDiv) {
	case 1:
		dataL |= 0x000000FFU;
		break;		/* 1/1   */
	case 2:
		dataL |= 0x00000000U;
		break;		/* 1/2   */
	case 4:
		dataL |= 0x00000001U;
		break;		/* 1/4   */
	case 8:
		dataL |= 0x00000002U;
		break;		/* 1/8   */
	case 16:
		dataL |= 0x00000004U;
		break;		/* 1/16  */
	case 32:
		dataL |= 0x00000008U;
		break;		/* 1/32  */
	case 64:
		dataL |= 0x00000010U;
		break;		/* 1/64  */
	case 128:
		dataL |= 0x00000020U;
		break;		/* 1/128 */
	case 256:
		dataL |= 0x00000040U;
		break;		/* 1/256 */
	case 512:
		dataL |= 0x00000080U;
		break;		/* 1/512 */
	}

	SETR_32(SD_CLK_CTRL, dataL);
	mmc_drv_obj.current_freq = (uint32_t) clkDiv;
}

static void emmc_get_partition_access(void)
{
	uint32_t reg;
	EMMC_ERROR_CODE result;

	reg = mmio_read_32(RCAR_PRR) & (PRR_PRODUCT_MASK | PRR_CUT_MASK);
	if ((reg == PRR_PRODUCT_H3_CUT20) || (reg == PRR_PRODUCT_M3_CUT11)) {
		SETR_32(SD_OPTION, 0x000060EEU);	/* 8 bits width */
		/* CMD8 (EXT_CSD) */
		emmc_make_trans_cmd(CMD8_SEND_EXT_CSD, 0x00000000U,
				    (uint32_t *) (&mmc_drv_obj.ext_csd_data[0]),
				    EMMC_MAX_EXT_CSD_LENGTH,
				    HAL_MEMCARD_READ, HAL_MEMCARD_NOT_DMA);
		mmc_drv_obj.get_partition_access_flag = TRUE;
		result =
		    emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
		mmc_drv_obj.get_partition_access_flag = FALSE;
		if (result == EMMC_SUCCESS) {
			mmc_drv_obj.partition_access =
			    (EMMC_PARTITION_ID) (mmc_drv_obj.ext_csd_data[179]
						 & PARTITION_ID_MASK);
		} else if (result == EMMC_ERR_CMD_TIMEOUT) {
			mmc_drv_obj.partition_access = PARTITION_ID_BOOT_1;
		} else {
			emmc_write_error_info(EMMC_FUNCNO_GET_PERTITION_ACCESS,
					      result);
			panic();
		}
		SETR_32(SD_OPTION, 0x0000C0EEU);	/* Initialize */
	}
}

static uint32_t emmc_calc_tran_speed(uint32_t *freq)
{
	const uint32_t unit[8] = { 10000, 100000, 1000000, 10000000,
				0, 0, 0, 0 };   /**< frequency unit (1/10) */
	const uint32_t mult[16] = { 0, 10, 12, 13, 15, 20, 26, 30, 35, 40, 45,
				52, 55, 60, 70, 80 };

	uint32_t maxFreq;
	uint32_t result;
	uint32_t tran_speed = EMMC_CSD_TRAN_SPEED();

	/* tran_speed = 0x32
	 * unit[tran_speed&0x7] = uint[0x2] = 1000000
	 * mult[(tran_speed&0x78)>>3] = mult[0x30>>3] = mult[6] = 26
	 * 1000000 * 26 = 26000000 (26MHz)
	 */

	result = 1;
	maxFreq =
	    unit[tran_speed & EMMC_TRANSPEED_FREQ_UNIT_MASK] *
	    mult[(tran_speed & EMMC_TRANSPEED_MULT_MASK) >>
		 EMMC_TRANSPEED_MULT_SHIFT];

	if (maxFreq == 0) {
		result = 0;
	} else if (MMC_FREQ_52MHZ <= maxFreq)
		*freq = MMC_52MHZ;
	else if (MMC_FREQ_26MHZ <= maxFreq)
		*freq = MMC_26MHZ;
	else if (MMC_FREQ_20MHZ <= maxFreq)
		*freq = MMC_20MHZ;
	else
		*freq = MMC_400KHZ;

	return result;
}

static uint32_t emmc_set_timeout_register_value(uint32_t freq)
{
	uint32_t timeoutCnt;	/* SD_OPTION   - Timeout Counter  */

	switch (freq) {
	case 1U:
		timeoutCnt = 0xE0U;
		break;		/* SDCLK * 2^27 */
	case 2U:
		timeoutCnt = 0xE0U;
		break;		/* SDCLK * 2^27 */
	case 4U:
		timeoutCnt = 0xD0U;
		break;		/* SDCLK * 2^26 */
	case 8U:
		timeoutCnt = 0xC0U;
		break;		/* SDCLK * 2^25 */
	case 16U:
		timeoutCnt = 0xB0U;
		break;		/* SDCLK * 2^24 */
	case 32U:
		timeoutCnt = 0xA0U;
		break;		/* SDCLK * 2^23 */
	case 64U:
		timeoutCnt = 0x90U;
		break;		/* SDCLK * 2^22 */
	case 128U:
		timeoutCnt = 0x80U;
		break;		/* SDCLK * 2^21 */
	case 256U:
		timeoutCnt = 0x70U;
		break;		/* SDCLK * 2^20 */
	case 512U:
		timeoutCnt = 0x70U;
		break;		/* SDCLK * 2^20 */
	default:
		timeoutCnt = 0xE0U;
		break;		/* SDCLK * 2^27 */
	}

	return timeoutCnt;
}

EMMC_ERROR_CODE emmc_set_ext_csd(uint32_t arg)
{
	EMMC_ERROR_CODE result;

	/* CMD6 */
	emmc_make_nontrans_cmd(CMD6_SWITCH, arg);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		return result;
	}

	/* CMD13 */
	emmc_make_nontrans_cmd(CMD13_SEND_STATUS, EMMC_RCA << 16);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		return result;
	}

	/* CMD8 (EXT_CSD) */
	emmc_make_trans_cmd(CMD8_SEND_EXT_CSD, 0x00000000,
			    (uint32_t *) (&mmc_drv_obj.ext_csd_data[0]),
			    EMMC_MAX_EXT_CSD_LENGTH, HAL_MEMCARD_READ,
			    HAL_MEMCARD_NOT_DMA);
	result = emmc_exec_cmd(EMMC_R1_ERROR_MASK, mmc_drv_obj.response);
	if (result != EMMC_SUCCESS) {
		return result;
	}
	return EMMC_SUCCESS;
}

EMMC_ERROR_CODE emmc_set_request_mmc_clock(uint32_t *freq)
{
	/* parameter check */
	if (freq == NULL) {
		emmc_write_error_info(EMMC_FUNCNO_SET_CLOCK, EMMC_ERR_PARAM);
		return EMMC_ERR_PARAM;
	}

	/* state check */
	if ((mmc_drv_obj.initialize != TRUE)
	    || (mmc_drv_obj.card_power_enable != TRUE)) {
		emmc_write_error_info(EMMC_FUNCNO_SET_CLOCK, EMMC_ERR_STATE);
		return EMMC_ERR_STATE;
	}

	/* clock is already running in the desired frequency. */
	if ((mmc_drv_obj.clock_enable == TRUE)
	    && (mmc_drv_obj.current_freq == *freq)) {
		return EMMC_SUCCESS;
	}

	/* busy check */
	if ((GETR_32(SD_INFO2) & SD_INFO2_CBSY) != 0) {
		emmc_write_error_info(EMMC_FUNCNO_SET_CLOCK,
				      EMMC_ERR_CARD_BUSY);
		return EMMC_ERR;
	}

	set_sd_clk(*freq);
	mmc_drv_obj.clock_enable = FALSE;

	return emmc_clock_ctrl(TRUE);	/* clock on */
}

EMMC_ERROR_CODE rcar_emmc_mount(void)
{
	EMMC_ERROR_CODE result;

	/* state check */
	if ((mmc_drv_obj.initialize != TRUE)
	    || (mmc_drv_obj.card_power_enable != TRUE)
	    || ((GETR_32(SD_INFO2) & SD_INFO2_CBSY) != 0)
	    ) {
		emmc_write_error_info(EMMC_FUNCNO_MOUNT, EMMC_ERR_STATE);
		return EMMC_ERR_STATE;
	}

	/* initialize card (IDLE state --> Transfer state) */
	result = emmc_card_init();
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_CARD_INIT);
		if (emmc_clock_ctrl(FALSE) != EMMC_SUCCESS) {
			/* nothing to do. */
		}
		return result;
	}

	/* Switching high speed mode */
	result = emmc_high_speed();
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_HIGH_SPEED);
		if (emmc_clock_ctrl(FALSE) != EMMC_SUCCESS) {
			/* nothing to do. */
		}
		return result;
	}

	/* Changing the data bus width */
	result = emmc_bus_width(8);
	if (result != EMMC_SUCCESS) {
		emmc_write_error_info_func_no(EMMC_FUNCNO_BUS_WIDTH);
		if (emmc_clock_ctrl(FALSE) != EMMC_SUCCESS) {
			/* nothing to do. */
		}
		return result;
	}

	/* mount complete */
	mmc_drv_obj.mount = TRUE;

	return EMMC_SUCCESS;
}
