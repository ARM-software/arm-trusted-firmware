/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <arch_helpers.h>
#include <lib/mmio.h>

#include "bcm_emmc.h"
#include "emmc_chal_types.h"
#include "emmc_csl_sdprot.h"
#include "emmc_chal_sd.h"
#include "emmc_csl_sdcmd.h"
#include "emmc_csl_sd.h"
#include "emmc_pboot_hal_memory_drv.h"

#define SD_CARD_BUSY                    0x80000000
#define SD_CARD_RETRY_LIMIT             1000
#define SD_CARD_HIGH_SPEED_PS           13
#define SD_CHK_HIGH_SPEED_MODE          0x00FFFFF1
#define SD_SET_HIGH_SPEED_MODE          0x80FFFFF1
#define SD_MMC_ENABLE_HIGH_SPEED        0x03b90100	//0x03b90103
#define SD_MMC_8BIT_MODE                0x03b70200
#define SD_MMC_4BIT_MODE                0x03b70100
#define SD_MMC_1BIT_MODE                0x03b70000

#define SD_MMC_BOOT_8BIT_MODE           0x03b10200
#define SD_MMC_BOOT_4BIT_MODE           0x03b10100
#define SD_MMC_BOOT_1BIT_MODE           0x03b10000
#define SDIO_HW_EMMC_EXT_CSD_BOOT_CNF   0X03B30000

#ifdef USE_EMMC_FIP_TOC_CACHE
/*
 * Cache size mirrors the size of the global eMMC temp buffer
 * which is used for non-image body reads such as headers, ToC etc.
 */
#define CACHE_SIZE           ((EMMC_BLOCK_SIZE) * 2)
#define PARTITION_BLOCK_ADDR ((PLAT_FIP_ATTEMPT_OFFSET)/(EMMC_BLOCK_SIZE))

static uint32_t cached_partition_block;
static uint8_t cached_block[CACHE_SIZE];
#endif

static int set_card_data_width(struct sd_handle *handle, int width);
static int abort_err(struct sd_handle *handle);
static int err_recovery(struct sd_handle *handle, uint32_t errors);
static int xfer_data(struct sd_handle *handle, uint32_t mode, uint32_t addr,
		     uint32_t length, uint8_t *base);

int set_boot_config(struct sd_handle *handle, uint32_t config)
{
	return mmc_cmd6(handle, SDIO_HW_EMMC_EXT_CSD_BOOT_CNF | config);
}

void process_csd_mmc_speed(struct sd_handle *handle, uint32_t csd_mmc_speed)
{
	uint32_t div_ctrl_setting;

	/* CSD field TRAN_SPEED:
	 * Bits [2:0] 0 = 100 KHz
	 *            1 = 1 MHz
	 *            2 = 10 MHz
	 *            3 = 100 MHz
	 *            4...7 Reserved.
	 * Bits [6:3] 0 = Reserved
	 *            1 = 1.0
	 *            2 = 1.2
	 *            3 = 1.3
	 *            4 = 1.5
	 *            5 = 2.0
	 *            6 = 2.6
	 *            7 = 3.0
	 *            8 = 3.5
	 *            9 = 4.0
	 *            A = 4.5
	 *            B = 5.2
	 *            C = 5.5
	 *            D = 6.0
	 *            E = 7.0
	 *            F = 8.0
	 * For cards supporting version 4.0, 4.1, and 4.2 of the standard,
	 * the value shall be 20 MHz (0x2A).
	 * For cards supporting version 4.3 , the value shall be 26 MHz (0x32)
	 */

	switch (csd_mmc_speed & 0x7F) {
	case 0x2A:
		EMMC_TRACE("Speeding up eMMC clock to 20MHz\n");
		div_ctrl_setting =
		    chal_sd_freq_2_div_ctrl_setting(20 * 1000 * 1000);
		break;
	case 0x32:
		EMMC_TRACE("Speeding up eMMC clock to 26MHz\n");
		div_ctrl_setting =
		    chal_sd_freq_2_div_ctrl_setting(26 * 1000 * 1000);
		break;
	default:
		/* Unknown */
		return;
	}

	chal_sd_set_clock((CHAL_HANDLE *) handle->device, div_ctrl_setting, 0);

	chal_sd_set_clock((CHAL_HANDLE *) handle->device, div_ctrl_setting, 1);

	SD_US_DELAY(1000);
}


/*
 * The function changes SD/SDIO/MMC card data width if
 * the card support configurable data width. The host controller
 * and the card has to be in the same bus data width.
 */
int set_card_data_width(struct sd_handle *handle, int width)
{
	uint32_t data_width = 0;
	int is_valid_arg = 1;
	int rc = SD_FAIL;
	char *bitwidth_str = " ";
	char *result_str = "failed";

	switch (width) {
#ifdef DRIVER_EMMC_ENABLE_DATA_WIDTH_8BIT
	case SD_BUS_DATA_WIDTH_8BIT:
		data_width = SD_MMC_8BIT_MODE;
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
		bitwidth_str = "8_BIT";
#endif
		break;
#endif
	case SD_BUS_DATA_WIDTH_4BIT:
		data_width = SD_MMC_4BIT_MODE;
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
		bitwidth_str = "4_BIT";
#endif
		break;

	case SD_BUS_DATA_WIDTH_1BIT:
		data_width = SD_MMC_1BIT_MODE;
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
		bitwidth_str = "1_BIT";
#endif
		break;

	default:
		is_valid_arg = 0;
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
		bitwidth_str = "unknown";
#endif
		break;
	}

	if (is_valid_arg) {
		rc = mmc_cmd6(handle, data_width);
		if (rc == SD_OK) {
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
			result_str = "succeeded";
#endif
			chal_sd_config_bus_width((CHAL_HANDLE *) handle->device,
						 width);
		} else {
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
			result_str = "failed";
#endif
		}
	} else {
		rc = SD_FAIL;
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
		result_str = "ignored";
#endif
	}

	VERBOSE("SDIO Data Width(%s) %s.\n", bitwidth_str, result_str);

	return rc;
}


/*
 * Error handling routine. Does abort data
 * transmission if error is found.
 */
static int abort_err(struct sd_handle *handle)
{
	uint32_t present, options, event, rel = 0;
	struct sd_resp cmdRsp;

	handle->device->ctrl.argReg = 0;
	handle->device->ctrl.cmdIndex = SD_CMD_STOP_TRANSMISSION;

	options = (SD_CMD_STOP_TRANSMISSION << 24) |
		  (SD_CMDR_RSP_TYPE_R1b_5b << SD_CMDR_RSP_TYPE_S) |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK;

	chal_sd_send_cmd((CHAL_HANDLE *) handle->device,
			 handle->device->ctrl.cmdIndex,
			 handle->device->ctrl.argReg, options);

	event = wait_for_event(handle,
			       SD4_EMMC_TOP_INTR_CMDDONE_MASK |
			       SD_ERR_INTERRUPTS,
			       handle->device->cfg.wfe_retry);

	if (event & SD_CMD_ERROR_INT) {
		rel = SD_ERROR_NON_RECOVERABLE;
	} else {
		if (event & SD_DAT_TIMEOUT) {
			return SD_ERROR_NON_RECOVERABLE;
		}

		chal_sd_get_response((CHAL_HANDLE *) handle->device,
				     (uint32_t *)&cmdRsp);

		process_cmd_response(handle, handle->device->ctrl.cmdIndex,
				     cmdRsp.data.r2.rsp1, cmdRsp.data.r2.rsp2,
				     cmdRsp.data.r2.rsp3, cmdRsp.data.r2.rsp4,
				     &cmdRsp);

		SD_US_DELAY(2000);

		present =
		    chal_sd_get_present_status((CHAL_HANDLE *) handle->device);

		if ((present & 0x00F00000) == 0x00F00000)
			rel = SD_ERROR_RECOVERABLE;
		else
			rel = SD_ERROR_NON_RECOVERABLE;
	}

	return rel;
}


/*
 * The function handles real data transmission on both DMA and
 * none DMA mode, In None DMA mode the data transfer starts
 * when the command is sent to the card, data has to be written
 * into the host contollers buffer at this time one block
 * at a time.
 * In DMA mode, the real data transfer is done by the DMA engine
 * and this functions just waits for the data transfer to complete.
 *
 */
int process_data_xfer(struct sd_handle *handle, uint8_t *buffer, uint32_t addr,
		      uint32_t length, int dir)
{
	if (dir == SD_XFER_HOST_TO_CARD) {
#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
		if (handle->device->cfg.dma == SD_DMA_OFF) {
			/*
			 * In NON DMA mode, the real data xfer starts from here
			 */
			if (write_buffer(handle, length, buffer))
				return SD_WRITE_ERROR;
		} else {
			wait_for_event(handle,
				       SD4_EMMC_TOP_INTR_TXDONE_MASK |
				       SD_ERR_INTERRUPTS,
				       handle->device->cfg.wfe_retry);

			if (handle->device->ctrl.cmdStatus == SD_OK)
				return SD_OK;

			check_error(handle, handle->device->ctrl.cmdStatus);
			return SD_WRITE_ERROR;
		}
#else
		return SD_WRITE_ERROR;
#endif
	} else {		/* SD_XFER_CARD_TO_HOST */

		if (handle->device->cfg.dma == SD_DMA_OFF) {
			/* In NON DMA mode, the real data
			 * transfer starts from here
			 */
			if (read_buffer(handle, length, buffer))
				return SD_READ_ERROR;

		} else {	/* for DMA mode */

			/*
			 * once the data transmission is done
			 * copy data to the host buffer.
			 */
			wait_for_event(handle,
				       SD4_EMMC_TOP_INTR_TXDONE_MASK |
				       SD_ERR_INTERRUPTS,
				       handle->device->cfg.wfe_retry);

			if (handle->device->ctrl.cmdStatus == SD_OK)
				return SD_OK;

			check_error(handle, handle->device->ctrl.cmdStatus);
			return SD_READ_ERROR;
		}
	}
	return SD_OK;
}


/*
 * The function sets block size for the next SD/SDIO/MMC
 * card read/write command.
 */
int select_blk_sz(struct sd_handle *handle, uint16_t size)
{
	return sd_cmd16(handle, size);
}


/*
 * The function initalizes the SD/SDIO/MMC/CEATA and detects
 * the card according to the flag of detection.
 * Once this function is called, the card is put into ready state
 * so application can do data transfer to and from the card.
 */
int init_card(struct sd_handle *handle, int detection)
{
	/*
	 * After Reset, eMMC comes up in 1 Bit Data Width by default.
	 * Set host side to match.
	 */
	chal_sd_config_bus_width((CHAL_HANDLE *) handle->device,
				 SD_BUS_DATA_WIDTH_1BIT);

#ifdef USE_EMMC_FIP_TOC_CACHE
	cached_partition_block = 0;
#endif
	handle->device->ctrl.present = 0; /* init card present to be no card */

	init_mmc_card(handle);

	handle->device->ctrl.present = 1; /* card is detected */

	/* switch the data width back */
	if (handle->card->type != SD_CARD_MMC)
		return SD_FAIL;

	/*
	 * Dynamically set Data Width to highest supported value.
	 * Try different data width settings (highest to lowest).
	 * Verify each setting by reading EXT_CSD and comparing
	 * against the EXT_CSD contents previously read in call to
	 * init_mmc_card() earlier. Stop at first verified data width
	 * setting.
	 */
	{
#define EXT_CSD_PROPERTIES_SECTION_START_INDEX	192
#define EXT_CSD_PROPERTIES_SECTION_END_INDEX	511
		uint8_t buffer[EXT_CSD_SIZE];
#ifdef DRIVER_EMMC_ENABLE_DATA_WIDTH_8BIT
		/* Try 8 Bit Data Width */
		chal_sd_config_bus_width((CHAL_HANDLE *) handle->device,
					 SD_BUS_DATA_WIDTH_8BIT);
		if ((!set_card_data_width(handle, SD_BUS_DATA_WIDTH_8BIT)) &&
		    (!mmc_cmd8(handle, buffer)) &&
		    (!memcmp(&buffer[EXT_CSD_PROPERTIES_SECTION_START_INDEX],
			     &(emmc_global_buf_ptr->u.Ext_CSD_storage[EXT_CSD_PROPERTIES_SECTION_START_INDEX]),
			     EXT_CSD_PROPERTIES_SECTION_END_INDEX - EXT_CSD_PROPERTIES_SECTION_START_INDEX + 1)))

			return SD_OK;
#endif
		/* Fall back to 4 Bit Data Width */
		chal_sd_config_bus_width((CHAL_HANDLE *) handle->device,
					 SD_BUS_DATA_WIDTH_4BIT);
		if ((!set_card_data_width(handle, SD_BUS_DATA_WIDTH_4BIT)) &&
		    (!mmc_cmd8(handle, buffer)) &&
		    (!memcmp(&buffer[EXT_CSD_PROPERTIES_SECTION_START_INDEX],
			     &(emmc_global_buf_ptr->u.Ext_CSD_storage[EXT_CSD_PROPERTIES_SECTION_START_INDEX]),
			     EXT_CSD_PROPERTIES_SECTION_END_INDEX - EXT_CSD_PROPERTIES_SECTION_START_INDEX + 1)))

			return SD_OK;

		/* Fall back to 1 Bit Data Width */
		chal_sd_config_bus_width((CHAL_HANDLE *) handle->device,
					 SD_BUS_DATA_WIDTH_1BIT);
		/* Just use 1 Bit Data Width then. */
		if (!set_card_data_width(handle, SD_BUS_DATA_WIDTH_1BIT))
			return SD_OK;

	}
	return SD_CARD_INIT_ERROR;
}


/*
 * The function handles MMC/CEATA card initalization.
 */
int init_mmc_card(struct sd_handle *handle)
{
	uint32_t ocr = 0, newOcr, rc, limit = 0;
	uint32_t cmd1_option = 0x40300000;
	uint32_t sec_count;

	handle->card->type = SD_CARD_MMC;

	do {
		SD_US_DELAY(1000);
		newOcr = 0;
		ocr = 0;
		rc = sd_cmd1(handle, cmd1_option, &newOcr);
		limit++;

		if (rc == SD_OK)
			ocr = newOcr;

	} while (((ocr & SD_CARD_BUSY) == 0) && (limit < SD_CARD_RETRY_LIMIT));

	if (limit >= SD_CARD_RETRY_LIMIT) {
		handle->card->type = SD_CARD_UNKNOWN;
		EMMC_TRACE("CMD1 Timeout: Device is not ready\n");
		return SD_CARD_UNKNOWN;
	}

	/* Save the ocr register */
	handle->device->ctrl.ocr = ocr;

	/* Ready State */
	rc = sd_cmd2(handle);
	if (rc != SD_OK) {
		handle->card->type = SD_CARD_UNKNOWN;
		return SD_CARD_UNKNOWN;
	}

	rc = sd_cmd3(handle);
	if (rc != SD_OK) {
		handle->card->type = SD_CARD_UNKNOWN;
		return SD_CARD_UNKNOWN;
	}
	/* read CSD */
	rc = sd_cmd9(handle, &emmc_global_vars_ptr->cardData);
	if (rc != SD_OK) {
		handle->card->type = SD_CARD_UNKNOWN;
		return SD_CARD_UNKNOWN;
	}

	/* Increase clock frequency according to what the card advertises */
	EMMC_TRACE("From CSD...  cardData.csd.mmc.speed = 0x%X\n",
		   emmc_global_vars_ptr->cardData.csd.mmc.speed);
	process_csd_mmc_speed(handle,
			      emmc_global_vars_ptr->cardData.csd.mmc.speed);

	/* goto transfer mode */
	rc = sd_cmd7(handle, handle->device->ctrl.rca);
	if (rc != SD_OK) {
		handle->card->type = SD_CARD_UNKNOWN;
		return SD_CARD_UNKNOWN;
	}

	rc = mmc_cmd8(handle, emmc_global_buf_ptr->u.Ext_CSD_storage);
	if (rc == SD_OK) {
		/* calcul real capacity */
		sec_count = emmc_global_buf_ptr->u.Ext_CSD_storage[212] |
			    emmc_global_buf_ptr->u.Ext_CSD_storage[213] << 8 |
			    emmc_global_buf_ptr->u.Ext_CSD_storage[214] << 16 |
			    emmc_global_buf_ptr->u.Ext_CSD_storage[215] << 24;

		EMMC_TRACE("Device density = %ldMBytes\n",
			   handle->card->size / (1024 * 1024));

		if (sec_count > 0) {
			handle->card->size = (uint64_t)sec_count * 512;

			EMMC_TRACE("Updated Device density = %ldMBytes\n",
				   handle->card->size / (1024 * 1024));
		}

		if (sec_count > (2u * 1024 * 1024 * 1024) / 512) {
			handle->device->ctrl.ocr |= SD_CARD_HIGH_CAPACITY;
			handle->device->cfg.blockSize = 512;
		}

		if (handle->device->ctrl.ocr & SD_CARD_HIGH_CAPACITY)
			EMMC_TRACE("Sector addressing\n");
		else
			EMMC_TRACE("Byte addressing\n");

		EMMC_TRACE("Ext_CSD_storage[162]: 0x%02X  Ext_CSD_storage[179]: 0x%02X\n",
			   emmc_global_buf_ptr->u.Ext_CSD_storage[162],
			   emmc_global_buf_ptr->u.Ext_CSD_storage[179]);
	}

	return handle->card->type;
}


/*
 * The function send reset command to the card.
 * The card will be in ready status after the reset.
 */
int reset_card(struct sd_handle *handle)
{
	int res = SD_OK;

	/* on reset, card's RCA should return to 0 */
	handle->device->ctrl.rca = 0;

	res = sd_cmd0(handle);

	if (res != SD_OK)
		return SD_RESET_ERROR;

	return res;
}


/*
 * The function sends command to the card and starts
 * data transmission.
 */
static int xfer_data(struct sd_handle *handle,
		     uint32_t mode,
		     uint32_t addr, uint32_t length, uint8_t *base)
{
	int rc = SD_OK;

	VERBOSE("XFER: dest: 0x%llx, addr: 0x%x, size: 0x%x bytes\n",
		(uint64_t)base, addr, length);

	if ((length / handle->device->cfg.blockSize) > 1) {
		if (mode == SD_OP_READ) {
			inv_dcache_range((uintptr_t)base, (uint64_t)length);
			rc = sd_cmd18(handle, addr, length, base);
		} else {
#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
			flush_dcache_range((uintptr_t)base, (uint64_t)length);
			rc = sd_cmd25(handle, addr, length, base);
#else
			rc = SD_DATA_XFER_ERROR;
#endif
		}
	} else {
		if (mode == SD_OP_READ) {
			inv_dcache_range((uintptr_t)base, (uint64_t)length);
			rc = sd_cmd17(handle, addr,
				      handle->device->cfg.blockSize, base);
		} else {
#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
			flush_dcache_range((uintptr_t)base, (uint64_t)length);
			rc = sd_cmd24(handle, addr,
				      handle->device->cfg.blockSize, base);
#else
			rc = SD_DATA_XFER_ERROR;
#endif
		}
	}

	if (rc != SD_OK)
		return SD_DATA_XFER_ERROR;

	return SD_OK;
}

#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
int erase_card(struct sd_handle *handle, uint32_t addr, uint32_t blocks)
{
	uint32_t end_addr;

	INFO("ERASE: addr: 0x%x, num of sectors: 0x%x\n", addr, blocks);

	if (sd_cmd35(handle, addr) != SD_OK)
		return SD_FAIL;

	end_addr = addr + blocks - 1;
	if (sd_cmd36(handle, end_addr) != SD_OK)
		return SD_FAIL;

	if (sd_cmd38(handle) != SD_OK)
		return SD_FAIL;

	return SD_OK;
}
#endif

/*
 * The function reads block data from a card.
 */
#ifdef USE_EMMC_FIP_TOC_CACHE
int read_block(struct sd_handle *handle,
	       uint8_t *dst, uint32_t addr, uint32_t len)
{
	int rel = SD_OK;

	/*
	 * Avoid doing repeated reads of the partition block
	 * by caching.
	 */
	if (cached_partition_block &&
	    addr == PARTITION_BLOCK_ADDR &&
	    len == CACHE_SIZE) {
		memcpy(dst, cached_block, len);
	} else {
		rel = xfer_data(handle, SD_OP_READ, addr, len, dst);

		if (len == CACHE_SIZE && addr == PARTITION_BLOCK_ADDR) {
			cached_partition_block = 1;
			memcpy(cached_block, dst, len);
		}
	}

	return rel;
}
#else
int read_block(struct sd_handle *handle,
	       uint8_t *dst, uint32_t addr, uint32_t len)
{
	return xfer_data(handle, SD_OP_READ, addr, len, dst);
}
#endif

#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE

/*
 * The function writes block data to a card.
 */
int write_block(struct sd_handle *handle,
		uint8_t *src, uint32_t addr, uint32_t len)
{
	int rel = SD_OK;

	/*
	 * Current HC has problem to get response of cmd16 after cmd12,
	 * the delay is necessary to sure the next cmd16 will not be timed out.
	 * The delay has to be at least 4 ms.
	 * The code removed cmd16 and use cmd13 to get card status before
	 * sending cmd18 or cmd25 to make sure the card is ready and thus
	 * no need to have delay here.
	 */

	rel = xfer_data(handle, SD_OP_WRITE, addr, len, src);

	EMMC_TRACE("wr_blk addr:0x%08X src:0x%08X len:0x%08X result:%d\n",
		   addr, src, len, rel);

	return rel;
}


/*
 * The function is called to write one block data directly to
 * a card's data buffer.
 * it is used in Non-DMA mode for card data transmission.
 */
int write_buffer(struct sd_handle *handle, uint32_t length, uint8_t *data)
{
	uint32_t rem, blockSize, event;
	uint8_t *pData = data;

	blockSize = handle->device->cfg.blockSize;
	rem = length;

	if (rem == 0)
		return SD_OK;

	while (rem > 0) {

		event = wait_for_event(handle,
				       SD4_EMMC_TOP_INTR_BWRDY_MASK |
				       SD_ERR_INTERRUPTS,
				       handle->device->cfg.wfe_retry);

		if (handle->device->ctrl.cmdStatus) {
			check_error(handle, handle->device->ctrl.cmdStatus);
			return SD_WRITE_ERROR;
		}

		if (rem >= blockSize)
			chal_sd_write_buffer((CHAL_HANDLE *) handle->device,
					     blockSize, pData);
		else
			chal_sd_write_buffer((CHAL_HANDLE *) handle->device,
					     rem, pData);

		if (rem > blockSize) {
			rem -= blockSize;
			pData += blockSize;
		} else {
			pData += rem;
			rem = 0;
		}
	}

	if ((event & SD4_EMMC_TOP_INTR_TXDONE_MASK) !=
	    SD4_EMMC_TOP_INTR_TXDONE_MASK) {
		event = wait_for_event(handle,
				       SD4_EMMC_TOP_INTR_TXDONE_MASK |
				       SD_ERR_INTERRUPTS,
				       handle->device->cfg.wfe_retry);

		if (handle->device->ctrl.cmdStatus != SD_OK) {
			check_error(handle, handle->device->ctrl.cmdStatus);
			return SD_WRITE_ERROR;
		}
	} else {
		handle->device->ctrl.eventList &= ~SD4_EMMC_TOP_INTR_TXDONE_MASK;
	}

	return SD_OK;
}
#endif /* INCLUDE_EMMC_DRIVER_WRITE_CODE */


/*
 * The function is called to read maximal one block data
 * directly from a card
 * It is used in Non-DMA mode for card data transmission.
 */
int read_buffer(struct sd_handle *handle, uint32_t length, uint8_t *data)
{
	uint32_t rem, blockSize, event = 0;
	uint8_t *pData = data;

	blockSize = handle->device->cfg.blockSize;
	rem = length;

	if (rem == 0)
		return SD_OK;

	while (rem > 0) {
		event = wait_for_event(handle,
				       SD4_EMMC_TOP_INTR_BRRDY_MASK |
				       SD_ERR_INTERRUPTS,
				       handle->device->cfg.wfe_retry);

		if (handle->device->ctrl.cmdStatus) {
			check_error(handle, handle->device->ctrl.cmdStatus);
			return SD_READ_ERROR;
		}

		if (rem >= blockSize)
			chal_sd_read_buffer((CHAL_HANDLE *) handle->device,
					    blockSize, pData);
		else
			chal_sd_read_buffer((CHAL_HANDLE *) handle->device, rem,
					    pData);

		if (rem > blockSize) {
			rem -= blockSize;
			pData += blockSize;
		} else {
			pData += rem;
			rem = 0;
		}
	}

	/* In case, there are extra data in the SD FIFO, just dump them. */
	chal_sd_dump_fifo((CHAL_HANDLE *) handle->device);

	if ((event & SD4_EMMC_TOP_INTR_TXDONE_MASK) !=
	    SD4_EMMC_TOP_INTR_TXDONE_MASK) {
		event = wait_for_event(handle, SD4_EMMC_TOP_INTR_TXDONE_MASK,
				       handle->device->cfg.wfe_retry);

		if (handle->device->ctrl.cmdStatus) {
			check_error(handle, handle->device->ctrl.cmdStatus);
			return SD_READ_ERROR;
		}
	} else {
		handle->device->ctrl.eventList &= ~SD4_EMMC_TOP_INTR_TXDONE_MASK;
	}

	return SD_OK;
}


/*
 * Error handling routine.
 * The function just reset the DAT
 * and CMD line if an error occures during data transmission.
 */
int check_error(struct sd_handle *handle, uint32_t ints)
{
	uint32_t rel;

	chal_sd_set_irq_signal((CHAL_HANDLE *) handle->device,
			       SD_ERR_INTERRUPTS, 0);

	if (ints & SD4_EMMC_TOP_INTR_CMDERROR_MASK) {

		chal_sd_reset_line((CHAL_HANDLE *) handle->device,
				   SD4_EMMC_TOP_CTRL1_CMDRST_MASK);
		rel = abort_err(handle);

		chal_sd_reset_line((CHAL_HANDLE *) handle->device,
				   SD4_EMMC_TOP_CTRL1_DATRST_MASK);
		chal_sd_set_irq_signal((CHAL_HANDLE *) handle->device,
				       SD_ERR_INTERRUPTS, 1);

		return (rel == SD_ERROR_NON_RECOVERABLE) ?
				SD_ERROR_NON_RECOVERABLE : SD_ERROR_RECOVERABLE;
	} else {
		rel = err_recovery(handle, ints);
	}

	chal_sd_set_irq_signal((CHAL_HANDLE *) handle->device,
			       SD_ERR_INTERRUPTS, 1);

	return rel;
}


/*
 * Error recovery routine.
 * Try to recover from the error.
 */
static int err_recovery(struct sd_handle *handle, uint32_t errors)
{
	uint32_t rel = 0;

	/*
	 * In case of timeout error, the cmd line and data line maybe
	 * still active or stuck at atcitve so it is needed to reset
	 * either data line or cmd line to make sure a new cmd can be sent.
	 */

	if (errors & SD_CMD_ERROR_INT)
		chal_sd_reset_line((CHAL_HANDLE *) handle->device,
				   SD4_EMMC_TOP_CTRL1_CMDRST_MASK);

	if (errors & SD_DAT_ERROR_INT)
		chal_sd_reset_line((CHAL_HANDLE *) handle->device,
				   SD4_EMMC_TOP_CTRL1_DATRST_MASK);

	/* Abort transaction by sending out stop command */
	if ((handle->device->ctrl.cmdIndex == 18) ||
	    (handle->device->ctrl.cmdIndex == 25))
		rel = abort_err(handle);

	return rel;
}


/*
 * The function is called to read one block data directly from a card.
 * It is used in Non-DMA mode for card data transmission.
 */
int process_cmd_response(struct sd_handle *handle,
			 uint32_t cmdIndex,
			 uint32_t rsp0,
			 uint32_t rsp1,
			 uint32_t rsp2, uint32_t rsp3, struct sd_resp *resp)
{
	int result = SD_OK;

	/* R6 */
	uint32_t rca = (rsp0 >> 16) & 0xffff;
	uint32_t cardStatus = rsp0;

	/* R4 */
	uint32_t cBit = (rsp0 >> 31) & 0x1;
	uint32_t funcs = (rsp0 >> 28) & 0x7;
	uint32_t memPresent = (rsp0 >> 27) & 0x1;

	resp->r1 = 0x3f;
	resp->cardStatus = cardStatus;

	if (cmdIndex == SD_CMD_IO_SEND_OP_COND) {
		resp->data.r4.cardReady = cBit;
		resp->data.r4.funcs = funcs;
		resp->data.r4.memPresent = memPresent;
		resp->data.r4.ocr = cardStatus;
	}

	if (cmdIndex == SD_CMD_MMC_SET_RCA) {
		resp->data.r6.rca = rca;
		resp->data.r6.cardStatus = cardStatus & 0xFFFF;
	}

	if (cmdIndex == SD_CMD_SELECT_DESELECT_CARD) {
		resp->data.r7.rca = rca;
	}

	if (cmdIndex == SD_CMD_IO_RW_DIRECT) {
		if (((rsp0 >> 16) & 0xffff) != 0)
			result = SD_CMD_ERR_INVALID_RESPONSE;

		resp->data.r5.data = rsp0 & 0xff;
	}

	if (cmdIndex == SD_CMD_IO_RW_EXTENDED) {
		if (((rsp0 >> 16) & 0xffff) != 0)
			result = SD_CMD_ERR_INVALID_RESPONSE;

		resp->data.r5.data = rsp0 & 0xff;
	}

	if (cmdIndex == SD_ACMD_SD_SEND_OP_COND ||
	    cmdIndex == SD_CMD_SEND_OPCOND)
		resp->data.r3.ocr = cardStatus;

	if (cmdIndex == SD_CMD_SEND_CSD ||
	    cmdIndex == SD_CMD_SEND_CID ||
	    cmdIndex == SD_CMD_ALL_SEND_CID) {
		resp->data.r2.rsp4 = rsp3;
		resp->data.r2.rsp3 = rsp2;
		resp->data.r2.rsp2 = rsp1;
		resp->data.r2.rsp1 = rsp0;
	}

	if ((cmdIndex == SD_CMD_READ_EXT_CSD) &&
	    (handle->card->type == SD_CARD_SD)) {
		if ((resp->cardStatus & 0xAA) != 0xAA) {
			result = SD_CMD_ERR_INVALID_RESPONSE;
		}
	}

	return result;
}


/*
 * The function sets DMA buffer and data length, process
 * block size and the number of blocks to be transferred.
 * It returns the DMA buffer address.
 * It copies dma data from user buffer to the DMA buffer
 * if the operation is to write data to the SD card.
 */
void data_xfer_setup(struct sd_handle *handle, uint8_t *data, uint32_t length,
		     int dir)
{
	chal_sd_setup_xfer((CHAL_HANDLE *)handle->device, data, length, dir);
}


/*
 * The function does soft reset the host SD controller. After
 * the function call all host controller's register are reset
 * to default vallue;
 *
 * Note    This function only resets the host controller it does not
 *          reset the controller's handler.
 */
int reset_host_ctrl(struct sd_handle *handle)
{
	chal_sd_stop();

	return SD_OK;
}

static void pstate_log(struct sd_handle *handle)
{
	ERROR("PSTATE: 0x%x\n", mmio_read_32
		(handle->device->ctrl.sdRegBaseAddr +
			SD4_EMMC_TOP_PSTATE_SD4_OFFSET));
	ERROR("ERRSTAT: 0x%x\n", mmio_read_32
		(handle->device->ctrl.sdRegBaseAddr +
			SD4_EMMC_TOP_ERRSTAT_OFFSET));
}

/*
 * The function waits for one or a group of interrupts specified
 * by mask. The function returns if any one the interrupt status
 * is set. If interrupt mode is not enabled then it will poll
 * the interrupt status register until a interrupt status is set
 * an error interrupt happens. If interrupt mode is enabled then
 * this function should be called after the interrupt
 * is received by ISR routine.
 */
uint32_t wait_for_event(struct sd_handle *handle,
			uint32_t mask, uint32_t retry)
{
	uint32_t regval, cmd12, time = 0;

	handle->device->ctrl.cmdStatus = 0;	/* no error */
	EMMC_TRACE("%s %d mask:0x%x timeout:%d irq_status:0x%x\n",
		   __func__, __LINE__, mask, retry,
		   chal_sd_get_irq_status((CHAL_HANDLE *)handle->device));

	/* Polling mode */
	do {
		regval = chal_sd_get_irq_status((CHAL_HANDLE *)handle->device);

		if (regval & SD4_EMMC_TOP_INTR_DMAIRQ_MASK) {
			chal_sd_set_dma_addr((CHAL_HANDLE *)handle->device,
					(uintptr_t)
				chal_sd_get_dma_addr((CHAL_HANDLE *)
						handle->device));
			chal_sd_clear_irq((CHAL_HANDLE *)handle->device,
					  SD4_EMMC_TOP_INTR_DMAIRQ_MASK);
		}

		if (time++ > retry) {
			ERROR("EMMC: No response (cmd%d) after %dus.\n",
			      handle->device->ctrl.cmdIndex,
			      time * EMMC_WFE_RETRY_DELAY_US);
			handle->device->ctrl.cmdStatus = SD_CMD_MISSING;
			pstate_log(handle);
			ERROR("EMMC: INT[0x%x]\n", regval);
			break;
		}

		if (regval & SD4_EMMC_TOP_INTR_CTOERR_MASK) {
			ERROR("EMMC: Cmd%d timeout INT[0x%x]\n",
			      handle->device->ctrl.cmdIndex, regval);
			handle->device->ctrl.cmdStatus =
			    SD4_EMMC_TOP_INTR_CTOERR_MASK;
			pstate_log(handle);
			break;
		}
		if (regval & SD_CMD_ERROR_FLAGS) {
			ERROR("EMMC: Cmd%d error INT[0x%x]\n",
			      handle->device->ctrl.cmdIndex, regval);
			handle->device->ctrl.cmdStatus = SD_CMD_ERROR_FLAGS;
			pstate_log(handle);
			break;
		}

		cmd12 = chal_sd_get_atuo12_error((CHAL_HANDLE *)handle->device);
		if (cmd12) {
			ERROR("EMMC: Cmd%d auto cmd12 err:0x%x\n",
			      handle->device->ctrl.cmdIndex, cmd12);
			handle->device->ctrl.cmdStatus = cmd12;
			pstate_log(handle);
			break;
		}

		if (SD_DATA_ERROR_FLAGS & regval) {
			ERROR("EMMC: Data for cmd%d error, INT[0x%x]\n",
			      handle->device->ctrl.cmdIndex, regval);
			handle->device->ctrl.cmdStatus =
			    (SD_DATA_ERROR_FLAGS & regval);
			pstate_log(handle);
			break;
		}

		if ((regval & mask) == 0)
			udelay(EMMC_WFE_RETRY_DELAY_US);

	} while ((regval & mask) == 0);

	/* clear the interrupt since it is processed */
	chal_sd_clear_irq((CHAL_HANDLE *)handle->device, (regval & mask));

	return (regval & mask);
}

int32_t set_config(struct sd_handle *handle, uint32_t speed, uint32_t retry,
		    uint32_t dma, uint32_t dmaBound, uint32_t blkSize,
		    uint32_t wfe_retry)
{
	int32_t rel = 0;

	if (handle == NULL)
		return SD_FAIL;

	handle->device->cfg.wfe_retry = wfe_retry;

	rel = chal_sd_config((CHAL_HANDLE *)handle->device, speed, retry,
			     dmaBound, blkSize, dma);
	return rel;

}

int mmc_cmd1(struct sd_handle *handle)
{
	uint32_t newOcr, res;
	uint32_t cmd1_option = MMC_OCR_OP_VOLT | MMC_OCR_SECTOR_ACCESS_MODE;

	/*
	 * After Reset, eMMC comes up in 1 Bit Data Width by default.
	 * Set host side to match.
	 */
	chal_sd_config_bus_width((CHAL_HANDLE *) handle->device,
				 SD_BUS_DATA_WIDTH_1BIT);

#ifdef USE_EMMC_FIP_TOC_CACHE
	cached_partition_block = 0;
#endif
	handle->device->ctrl.present = 0; /* init card present to be no card */

	handle->card->type = SD_CARD_MMC;

	res = sd_cmd1(handle, cmd1_option, &newOcr);

	if (res != SD_OK) {
		EMMC_TRACE("CMD1 Timeout: Device is not ready\n");
		res = SD_CARD_UNKNOWN;
	}
	return res;
}
