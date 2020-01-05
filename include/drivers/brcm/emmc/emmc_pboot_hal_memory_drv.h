/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PBOOT_HAL_MEMORY_EMMC_DRV_H
#define PBOOT_HAL_MEMORY_EMMC_DRV_H

#include <drivers/delay_timer.h>

#include "emmc_chal_types.h"
#include "emmc_chal_sd.h"
#include "emmc_csl_sdprot.h"
#include "emmc_csl_sdcmd.h"
#include "emmc_csl_sd.h"
#include "emmc_brcm_rdb_sd4_top.h"

#define CLK_SDIO_DIV_52MHZ      0x0
#define SYSCFG_IOCR4_PAD_10MA   0x38000000

#define SDCLK_CNT_PER_MS  52000
#define BOOT_ACK_TIMEOUT  (50 * SDCLK_CNT_PER_MS)
#define BOOT_DATA_TIMEOUT (1000 * SDCLK_CNT_PER_MS)

#define EMMC_BOOT_OK             0
#define EMMC_BOOT_ERROR          1
#define EMMC_BOOT_TIMEOUT        2
#define EMMC_BOOT_INVALIDIMAGE   3
#define EMMC_BOOT_NO_CARD        4

#define EMMC_USER_AREA             0
#define EMMC_BOOT_PARTITION1       1
#define EMMC_BOOT_PARTITION2       2
#define EMMC_USE_CURRENT_PARTITION 3

#define EMMC_BOOT_PARTITION_SIZE (128*1024)
#define EMMC_BLOCK_SIZE          512
#define EMMC_DMA_SIZE            (4*1024)

/*
 * EMMC4.3 definitions
 * Table 6 EXT_CSD access mode
 * Access
 * Bits Access Name Operation
 * 00 Command Set The command set is changed according to the Cmd Set field of
 * the argument
 * 01 Set Bits The bits in the pointed uint8_t are set,
 * according to the 1 bits in the Value field.
 * 10 Clear Bits The bits in the pointed uint8_t are cleared,
 * according to the 1 bits in the Value field.
 * 11 Write Byte The Value field is written into the pointed uint8_t.
 */

#define  SDIO_HW_EMMC_EXT_CSD_WRITE_BYTE             0X03000000

/* Boot bus width1 BOOT_BUS_WIDTH 1 R/W [177] */
#define SDIO_HW_EMMC_EXT_CSD_BOOT_BUS_WIDTH_OFFSET   0X00B10000

/* Boot configuration BOOT_CONFIG 1 R/W [179] */
#define SDIO_HW_EMMC_EXT_CSD_BOOT_CONFIG_OFFSET      0X00B30000

/* Bus width mode BUS_WIDTH 1 WO [183] */
#define SDIO_HW_EMMC_EXT_CSD_BUS_WIDTH_OFFSET        0X00B70000

/*
 * Bit 6: BOOT_ACK (non-volatile)
 * 0x0 : No boot acknowledge sent (default)
 * 0x1 : Boot acknowledge sent during boot operation
 * Bit[5:3] : BOOT_PARTITION_ENABLE (non-volatile)
 * User selects boot data that will be sent to master
 * 0x0 : Device not boot enabled (default)
 * 0x1 : Boot partition 1 enabled for boot
 * 0x2 : Boot partition 2 enabled for boot
 * 0x3-0x6 : Reserved
 * 0x7 : User area enabled for boot
 * Bit[2:0] : BOOT_PARTITION_ACCESS
 * User selects boot partition for read and write operation
 * 0x0 : No access to boot partition (default)
 * 0x1 : R/W boot partition 1
 * 0x2 : R/W boot partition 2
 * 0x3-0x7 : Reserved
 */

#define SDIO_HW_EMMC_EXT_CSD_BOOT_ACC_BOOT1     0X00000100
#define SDIO_HW_EMMC_EXT_CSD_BOOT_ACC_BOOT2     0X00000200
#define SDIO_HW_EMMC_EXT_CSD_BOOT_ACC_USER      0X00000000
#define SDIO_HW_EMMC_EXT_CSD_BOOT_EN_BOOT1      0X00004800
#define SDIO_HW_EMMC_EXT_CSD_BOOT_EN_BOOT2      0X00005000
#define SDIO_HW_EMMC_EXT_CSD_BOOT_EN_USER       0X00007800

#define SD_US_DELAY(x) udelay(x)

#endif
