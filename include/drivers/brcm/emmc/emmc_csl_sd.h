/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef	CSL_SD_H
#define	CSL_SD_H

#define SD_CLOCK_BASE				104000000
#define SD_CLOCK_52MHZ				52000000
#define SD_CLOCK_26MHZ				26000000
#define SD_CLOCK_17MHZ				17330000
#define SD_CLOCK_13MHZ				13000000
#define SD_CLOCK_10MHZ				10000000
#define SD_CLOCK_9MHZ				9000000
#define SD_CLOCK_7MHZ				7000000
#define SD_CLOCK_5MHZ				5000000
#define SD_CLOCK_1MHZ				1000000
#define SD_CLOCK_400KHZ				400000

#define SD_DRIVE_STRENGTH_MASK			0x38000000
#if defined(_BCM213x1_) || defined(_BCM21551_) || defined(_ATHENA_)
#define SD_DRIVE_STRENGTH			0x28000000
#elif defined(_BCM2153_)
#define SD_DRIVE_STRENGTH			0x38000000
#else
#define SD_DRIVE_STRENGTH			0x00000000
#endif

#define SD_NUM_HOST				2

#define SD_CARD_UNLOCK				0
#define SD_CARD_LOCK				0x4
#define SD_CARD_CLEAR_PWD			0x2
#define SD_CARD_SET_PWD				0x1
#define	SD_CARD_ERASE_PWD			0x8

#define SD_CARD_LOCK_STATUS			0x02000000
#define SD_CARD_UNLOCK_STATUS			0x01000000

#define SD_CMD_ERROR_FLAGS			(0x18F << 16)
#define SD_DATA_ERROR_FLAGS			(0x70  << 16)
#define SD_AUTO_CMD12_ERROR_FLAGS		(0x9F)
#define SD_CARD_STATUS_ERROR			0x10000000
#define SD_CMD_MISSING				0x80000000

#define SD_TRAN_HIGH_SPEED			0x32
#define SD_CARD_HIGH_CAPACITY			0x40000000
#define SD_CARD_POWER_UP_STATUS			0x80000000

struct sd_dev_info {
	uint32_t mode; /* interrupt or polling */
	uint32_t dma; /* dma enabled or disabled */
	uint32_t voltage; /* voltage level */
	uint32_t slot; /* if the HC is locatd at slot 0 or slot 1 */
	uint32_t version; /* 1.0 or 2.0 */
	uint32_t curSystemAddr; /* system address */
	uint32_t dataWidth; /* data width for the controller */
	uint32_t clock; /* clock rate */
	uint32_t status; /* if device is active on transfer or not */
};

void data_xfer_setup(struct sd_handle *handle, uint8_t *data,
		     uint32_t length, int dir);
int reset_card(struct sd_handle *handle);
int reset_host_ctrl(struct sd_handle *handle);
int init_card(struct sd_handle *handle, int detection);
int init_mmc_card(struct sd_handle *handle);
int write_buffer(struct sd_handle *handle, uint32_t len, uint8_t *buffer);
int read_buffer(struct sd_handle *handle, uint32_t len, uint8_t *buffer);
int select_blk_sz(struct sd_handle *handle, uint16_t size);
int check_error(struct sd_handle *handle, uint32_t ints);

int process_data_xfer(struct sd_handle *handle, uint8_t *buffer,
			  uint32_t addr, uint32_t length, int dir);
int read_block(struct sd_handle *handle, uint8_t *dst, uint32_t addr,
		uint32_t len);
#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
int erase_card(struct sd_handle *handle, uint32_t addr, uint32_t blocks);
#endif
int write_block(struct sd_handle *handle, uint8_t *src, uint32_t addr,
		 uint32_t len);
int process_cmd_response(struct sd_handle *handle, uint32_t cmdIndex,
			 uint32_t rsp0, uint32_t rsp1, uint32_t rsp2,
			 uint32_t rsp3, struct sd_resp *resp);
int32_t set_config(struct sd_handle *handle, uint32_t speed,
		   uint32_t retry, uint32_t dma, uint32_t dmaBound,
		   uint32_t blkSize, uint32_t wfe_retry);

uint32_t wait_for_event(struct sd_handle *handle, uint32_t mask,
			uint32_t retry);
int set_boot_config(struct sd_handle *handle, uint32_t config);

int mmc_cmd1(struct sd_handle *handle);
#endif /* CSL_SD_H */
