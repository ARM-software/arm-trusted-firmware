/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMMC_H
#define EMMC_H

#include <stdint.h>

#include <common/debug.h>

#include <platform_def.h>

#include "emmc_chal_types.h"
#include "emmc_chal_sd.h"
#include "emmc_csl_sdprot.h"
#include "emmc_csl_sdcmd.h"
#include "emmc_pboot_hal_memory_drv.h"

/* ------------------------------------------------------------------- */
#define EXT_CSD_SIZE		512

#ifdef PLAT_SD_MAX_READ_LENGTH
#define SD_MAX_READ_LENGTH PLAT_SD_MAX_READ_LENGTH
#ifdef USE_EMMC_LARGE_BLK_TRANSFER_LENGTH
#define SD_MAX_BLK_TRANSFER_LENGTH	0x10000000
#else
#define SD_MAX_BLK_TRANSFER_LENGTH	0x1000
#endif
#else
#define SD_MAX_READ_LENGTH EMMC_BLOCK_SIZE
#define SD_MAX_BLK_TRANSFER_LENGTH EMMC_BLOCK_SIZE
#endif

struct emmc_global_buffer {
	union {
		uint8_t Ext_CSD_storage[EXT_CSD_SIZE];
		uint8_t tempbuf[SD_MAX_READ_LENGTH];
	} u;
};

struct emmc_global_vars {
	struct sd_card_data cardData;
	struct sd_handle sdHandle;
	struct sd_dev sdDevice;
	struct sd_card_info sdCard;
	unsigned int init_done;
};

#define ICFG_SDIO0_CAP0__SLOT_TYPE_R 27
#define ICFG_SDIO0_CAP0__INT_MODE_R 26
#define ICFG_SDIO0_CAP0__SYS_BUS_64BIT_R 25
#define ICFG_SDIO0_CAP0__VOLTAGE_1P8V_R 24
#define ICFG_SDIO0_CAP0__VOLTAGE_3P0V_R 23
#define ICFG_SDIO0_CAP0__VOLTAGE_3P3V_R 22
#define ICFG_SDIO0_CAP0__SUSPEND_RESUME_R 21
#define ICFG_SDIO0_CAP0__SDMA_R 20
#define ICFG_SDIO0_CAP0__HIGH_SPEED_R 19
#define ICFG_SDIO0_CAP0__ADMA2_R 18
#define ICFG_SDIO0_CAP0__EXTENDED_MEDIA_R 17
#define ICFG_SDIO0_CAP0__MAX_BLOCK_LEN_R 15
#define ICFG_SDIO0_CAP0__BASE_CLK_FREQ_R 7
#define ICFG_SDIO0_CAP0__TIMEOUT_UNIT_R 6
#define ICFG_SDIO0_CAP0__TIMEOUT_CLK_FREQ_R 0
#define ICFG_SDIO0_CAP1__SPI_BLOCK_MODE_R 22
#define ICFG_SDIO0_CAP1__SPI_MODE_R 21
#define ICFG_SDIO0_CAP1__CLK_MULT_R 13
#define ICFG_SDIO0_CAP1__RETUNING_MODE_R 11
#define ICFG_SDIO0_CAP1__TUNE_SDR50_R 10
#define ICFG_SDIO0_CAP1__TIME_RETUNE_R 6
#define ICFG_SDIO0_CAP1__DRIVER_D_R 5
#define ICFG_SDIO0_CAP1__DRIVER_C_R 4
#define ICFG_SDIO0_CAP1__DRIVER_A_R 3
#define ICFG_SDIO0_CAP1__DDR50_R 2
#define ICFG_SDIO0_CAP1__SDR104_R 1
#define ICFG_SDIO0_CAP1__SDR50_R 0

#define SDIO0_CTRL_REGS_BASE_ADDR	   (SDIO0_EMMCSDXC_SYSADDR)
#define SDIO0_IDM_RESET_CTRL_ADDR	   (SDIO_IDM0_IDM_RESET_CONTROL)

#define EMMC_CTRL_REGS_BASE_ADDR	    SDIO0_CTRL_REGS_BASE_ADDR
#define EMMC_IDM_RESET_CTRL_ADDR	    SDIO0_IDM_RESET_CTRL_ADDR
#define EMMC_IDM_IO_CTRL_DIRECT_ADDR	SDIO_IDM0_IO_CONTROL_DIRECT

extern struct emmc_global_buffer *emmc_global_buf_ptr;

extern struct emmc_global_vars *emmc_global_vars_ptr;

#define EMMC_CARD_DETECT_TIMEOUT_MS 1200
#define EMMC_CMD_TIMEOUT_MS 200
#define EMMC_BUSY_CMD_TIMEOUT_MS 200
#define EMMC_CLOCK_SETTING_TIMEOUT_MS 100
#define EMMC_WFE_RETRY 40000
#define EMMC_WFE_RETRY_DELAY_US 10

#ifdef EMMC_DEBUG
#define EMMC_TRACE INFO
#else
#define EMMC_TRACE(...)
#endif

#endif /* EMMC_H */
