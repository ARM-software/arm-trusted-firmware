/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CDN_MMC_H
#define CDN_MMC_H

#include <drivers/cadence/cdns_combo_phy.h>
#include <drivers/mmc.h>
#include "socfpga_plat_def.h"

#if MMC_DEVICE_TYPE == 0
#define CONFIG_DMA_ADDR_T_64BIT		0
#endif

#define MMC_REG_BASE			SOCFPGA_MMC_REG_BASE
#define COMBO_PHY_REG		0x0
#define SDHC_EXTENDED_WR_MODE_MASK	0xFFFFFFF7
#define SDHC_DLL_RESET_MASK	0x00000001
/* HRS09 */
#define SDHC_PHY_SW_RESET			BIT(0)
#define SDHC_PHY_INIT_COMPLETE		BIT(1)
#define SDHC_EXTENDED_RD_MODE(x)	((x) << 2)
#define EXTENDED_WR_MODE			3
#define SDHC_EXTENDED_WR_MODE(x)	((x) << 3)
#define RDCMD_EN					15
#define SDHC_RDCMD_EN(x)			((x) << 15)
#define SDHC_RDDATA_EN(x)			((x) << 16)

/* CMD_DATA_OUTPUT */
#define SDHC_CDNS_HRS16				0x40

/* This value determines the interval by which DAT line timeouts are detected */
/* The interval can be computed as below: */
/* • 1111b - Reserved */
/* • 1110b - t_sdmclk*2(27+2) */
/* • 1101b - t_sdmclk*2(26+2) */
#define READ_CLK					0xa << 16
#define WRITE_CLK					0xe << 16
#define DTC_VAL						0xE

/* SRS00 */
/* System Address / Argument 2 / 32-bit block count
 * This field is used as:
 * • 32-bit Block Count register
 * • SDMA system memory address
 * • Auto CMD23 Argument
 */
#define SAAR						(1)

/* SRS01 */
/* Transfer Block Size
 * This field defines block size for block data transfers
 */
#define BLOCK_SIZE					0

/* SDMA Buffer Boundary
 * System address boundary can be set for SDMA engine.
 */
#define SDMA_BUF					7 << 12

/* Block Count For Current Transfer
 * To set the number of data blocks can be defined for next transfer
 */
#define BLK_COUNT_CT				16

/* SRS03 */
#define CMD_START					(U(1) << 31)
#define CMD_USE_HOLD_REG			(1 << 29)
#define CMD_UPDATE_CLK_ONLY			(1 << 21)
#define CMD_SEND_INIT				(1 << 15)
#define CMD_STOP_ABORT_CMD			(4 << 22)
#define CMD_RESUME_CMD				(2 << 22)
#define CMD_SUSPEND_CMD				(1 << 22)
#define DATA_PRESENT				(1 << 21)
#define CMD_IDX_CHK_ENABLE			(1 << 20)
#define CMD_WRITE					(0 << 4)
#define CMD_READ					(1 << 4)
#define	MULTI_BLK_READ				(1 << 5)
#define RESP_ERR					(1 << 7)
#define CMD_CHECK_RESP_CRC			(1 << 19)
#define RES_TYPE_SEL_48				(2 << 16)
#define RES_TYPE_SEL_136			(1 << 16)
#define RES_TYPE_SEL_48_B			(3 << 16)
#define RES_TYPE_SEL_NO				(0 << 16)
#define DMA_ENABLED					(1 << 0)
#define BLK_CNT_EN					(1 << 1)
#define AUTO_CMD_EN					(2 << 2)
#define COM_IDX						24
#define ERROR_INT					(1 << 15)
#define INT_SBE						(1 << 13)
#define INT_HLE						(1 << 12)
#define INT_FRUN					(1 << 11)
#define INT_DRT						(1 << 9)
#define INT_RTO						(1 << 8)
#define INT_DCRC					(1 << 7)
#define INT_RCRC					(1 << 6)
#define INT_RXDR					(1 << 5)
#define INT_TXDR					(1 << 4)
#define INT_DTO						(1 << 3)
#define INT_CMD_DONE				(1 << 0)
#define TRAN_COMP					(1 << 1)

/* SRS09 */
#define STATUS_DATA_BUSY			BIT(2)

/* SRS10 */
/* LED Control
 * State of this bit directly drives led port of the host
 * in order to control the external LED diode
 * Default value 0 << 1
 */
#define LEDC						BIT(0)
#define LEDC_OFF					0 << 1

/* Data Transfer Width
 * Bit used to configure DAT bus width to 1 or 4
 * Default value 1 << 1
 */
#define DT_WIDTH					BIT(1)
#define DTW_4BIT					1 << 1

/* Extended Data Transfer Width
 * This bit is to enable/disable 8-bit DAT bus width mode
 * Default value 1 << 5
 */
#define EDTW_8BIT					1 << 5

/* High Speed Enable
 * Selects operating mode to Default Speed (HSE=0) or High Speed (HSE=1)
 */
#define HS_EN						BIT(2)

/* here 0 defines the 64 Kb size */
#define MAX_64KB_PAGE				0
#define EMMC_DESC_SIZE		(1<<20)

/* SRS11 */
/* Software Reset For All
 * When set to 1, the entire slot is reset
 * After completing the reset operation, SRFA bit is automatically cleared
 */
#define SRFA						BIT(24)

/* Software Reset For CMD Line
 * When set to 1, resets the logic related to the command generation and response checking
 */
#define SRCMD						BIT(25)

/* Software Reset For DAT Line
 * When set to 1, resets the logic related to the data path,
 * including data buffers and the DMA logic
 */
#define SRDAT						BIT(26)

/* SRS15 */
/* UHS Mode Select
 * Used to select one of UHS-I modes.
 * • 000b - SDR12
 * • 001b - SDR25
 * • 010b - SDR50
 * • 011b - SDR104
 * • 100b - DDR50
 */
#define SDR12_MODE					0 << 16
#define SDR25_MODE					1 << 16
#define SDR50_MODE					2 << 16
#define SDR104_MODE					3 << 16
#define DDR50_MODE					4 << 16
/* 1.8V Signaling Enable
 * • 0 - for Default Speed, High Speed mode
 * • 1 - for UHS-I mode
 */
#define V18SE						BIT(19)

/* CMD23 Enable
 * In result of Card Identification process,
 * Host Driver set this bit to 1 if Card supports CMD23
 */
#define CMD23_EN					BIT(27)

/* Host Version 4.00 Enable
 * • 0 - Version 3.00
 * • 1 - Version 4.00
 */
#define HV4E						BIT(28)
/* Conf depends on SRS15.HV4E */
#define SDMA						0 << 3
#define ADMA2_32					2 << 3
#define ADMA2_64					3 << 3

/* Preset Value Enable
 * Setting this bit to 1 triggers an automatically update of SRS11
 */
#define PVE							BIT(31)

#define BIT_AD_32					0 << 29
#define BIT_AD_64					1 << 29

/* SW RESET REG*/
#define SDHC_CDNS_HRS00				(0x00)
#define SDHC_CDNS_HRS00_SWR			BIT(0)

/* PHY access port */
#define SDHC_CDNS_HRS04				0x10
#define SDHC_CDNS_HRS04_ADDR		GENMASK(5, 0)

/* PHY data access port */
#define SDHC_CDNS_HRS05				0x14

/* eMMC control registers */
#define SDHC_CDNS_HRS06				0x18

/* SRS */
#define SDHC_CDNS_SRS_BASE			0x200
#define SDHC_CDNS_SRS00				0x200
#define SDHC_CDNS_SRS01				0x204
#define SDHC_CDNS_SRS02				0x208
#define SDHC_CDNS_SRS03				0x20c
#define SDHC_CDNS_SRS04				0x210
#define SDHC_CDNS_SRS05				0x214
#define SDHC_CDNS_SRS06				0x218
#define SDHC_CDNS_SRS07				0x21C
#define SDHC_CDNS_SRS08				0x220
#define SDHC_CDNS_SRS09				0x224
#define SDHC_CDNS_SRS09_CI			BIT(16)
#define SDHC_CDNS_SRS10				0x228
#define SDHC_CDNS_SRS11				0x22C
#define SDHC_CDNS_SRS12				0x230
#define SDHC_CDNS_SRS13				0x234
#define SDHC_CDNS_SRS14				0x238
#define SDHC_CDNS_SRS15				0x23c
#define SDHC_CDNS_SRS21				0x254
#define SDHC_CDNS_SRS22				0x258
#define SDHC_CDNS_SRS23				0x25c

/* HRS07 */
#define SDHC_CDNS_HRS07				0x1c
#define SDHC_IDELAY_VAL(x)			((x) << 0)
#define SDHC_RW_COMPENSATE(x)		((x) << 16)

/* PHY reset port */
#define SDHC_CDNS_HRS09				0x24

/* HRS10 */
/* PHY reset port */
#define SDHC_CDNS_HRS10				0x28

/* HCSDCLKADJ DATA; DDR Mode */
#define SDHC_HCSDCLKADJ(x)			((x) << 16)

/* Pinmux headers will reomove after ATF driver implementation */
#define PINMUX_SDMMC_SEL			0x0
#define PIN0SEL						0x00
#define PIN1SEL						0x04
#define PIN2SEL						0x08
#define PIN3SEL						0x0C
#define PIN4SEL						0x10
#define PIN5SEL						0x14
#define PIN6SEL						0x18
#define PIN7SEL						0x1C
#define PIN8SEL						0x20
#define PIN9SEL						0x24
#define PIN10SEL					0x28

/* HRS16 */
#define SDHC_WRCMD0_DLY(x)			((x) << 0)
#define SDHC_WRCMD1_DLY(x)			((x) << 4)
#define SDHC_WRDATA0_DLY(x)			((x) << 8)
#define SDHC_WRDATA1_DLY(x)			((x) << 12)
#define SDHC_WRCMD0_SDCLK_DLY(x)	((x) << 16)
#define SDHC_WRCMD1_SDCLK_DLY(x)	((x) << 20)
#define SDHC_WRDATA0_SDCLK_DLY(x)	((x) << 24)
#define SDHC_WRDATA1_SDCLK_DLY(x)	((x) << 28)

/* Shared Macros */
#define SDMMC_CDN(_reg)				(SDMMC_CDN_REG_BASE + \
								(SDMMC_CDN_##_reg))

/* Refer to atf/tools/cert_create/include/debug.h */
#define BIT_32(nr)					(U(1) << (nr))

/* MMC Peripheral Definition */
#define SOCFPGA_MMC_BLOCK_SIZE		U(8192)
#define SOCFPGA_MMC_BLOCK_MASK		(SOCFPGA_MMC_BLOCK_SIZE - U(1))
#define SOCFPGA_MMC_BOOT_CLK_RATE	(400 * 1000)
#define MMC_RESPONSE_NONE			0
#define SDHC_CDNS_SRS03_VALUE		0x01020013

/* Value randomly chosen for eMMC RCA, it should be > 1 */
#define MMC_FIX_RCA					6
#define RCA_SHIFT_OFFSET			16

#define CMD_EXTCSD_PARTITION_CONFIG	179
#define CMD_EXTCSD_BUS_WIDTH		183
#define CMD_EXTCSD_HS_TIMING		185
#define CMD_EXTCSD_SEC_CNT			212

#define PART_CFG_BOOT_PARTITION1_ENABLE	(U(1) << 3)
#define PART_CFG_PARTITION1_ACCESS	(U(1) << 0)

/* Values in EXT CSD register */
#define MMC_BUS_WIDTH_1				U(0)
#define MMC_BUS_WIDTH_4				U(1)
#define MMC_BUS_WIDTH_8				U(2)
#define MMC_BUS_WIDTH_DDR_4			U(5)
#define MMC_BUS_WIDTH_DDR_8			U(6)
#define MMC_BOOT_MODE_BACKWARD		(U(0) << 3)
#define MMC_BOOT_MODE_HS_TIMING		(U(1) << 3)
#define MMC_BOOT_MODE_DDR			(U(2) << 3)

#define EXTCSD_SET_CMD				(U(0) << 24)
#define EXTCSD_SET_BITS				(U(1) << 24)
#define EXTCSD_CLR_BITS				(U(2) << 24)
#define EXTCSD_WRITE_BYTES			(U(3) << 24)
#define EXTCSD_CMD(x)				(((x) & 0xff) << 16)
#define EXTCSD_VALUE(x)				(((x) & 0xff) << 8)
#define EXTCSD_CMD_SET_NORMAL		U(1)

#define CSD_TRAN_SPEED_UNIT_MASK	GENMASK(2, 0)
#define CSD_TRAN_SPEED_MULT_MASK	GENMASK(6, 3)
#define CSD_TRAN_SPEED_MULT_SHIFT	3

#define STATUS_CURRENT_STATE(x)		(((x) & 0xf) << 9)
#define STATUS_READY_FOR_DATA		BIT(8)
#define STATUS_SWITCH_ERROR			BIT(7)
#define MMC_GET_STATE(x)			(((x) >> 9) & 0xf)
#define MMC_STATE_IDLE				0
#define MMC_STATE_READY				1
#define MMC_STATE_IDENT				2
#define MMC_STATE_STBY				3
#define MMC_STATE_TRAN				4
#define MMC_STATE_DATA				5
#define MMC_STATE_RCV				6
#define MMC_STATE_PRG				7
#define MMC_STATE_DIS				8
#define MMC_STATE_BTST				9
#define MMC_STATE_SLP				10

#define MMC_FLAG_CMD23				(U(1) << 0)

#define CMD8_CHECK_PATTERN			U(0xAA)
#define VHS_2_7_3_6_V				BIT(8)

/*ADMA table component*/
#define ADMA_DESC_ATTR_VALID		BIT(0)
#define ADMA_DESC_ATTR_END			BIT(1)
#define ADMA_DESC_ATTR_INT			BIT(2)
#define ADMA_DESC_ATTR_ACT1			BIT(4)
#define ADMA_DESC_ATTR_ACT2			BIT(5)
#define ADMA_DESC_TRANSFER_DATA		ADMA_DESC_ATTR_ACT2

enum sd_opcode {
	SD_GO_IDLE_STATE = 0,
	SD_ALL_SEND_CID = 2,
	SD_SEND_RELATIVE_ADDR = 3,
	SDIO_SEND_OP_COND = 5, /* SDIO cards only */
	SD_SWITCH = 6,
	SD_SELECT_CARD = 7,
	SD_SEND_IF_COND = 8,
	SD_SEND_CSD = 9,
	SD_SEND_CID = 10,
	SD_VOL_SWITCH = 11,
	SD_STOP_TRANSMISSION = 12,
	SD_SEND_STATUS = 13,
	SD_GO_INACTIVE_STATE = 15,
	SD_SET_BLOCK_SIZE = 16,
	SD_READ_SINGLE_BLOCK = 17,
	SD_READ_MULTIPLE_BLOCK = 18,
	SD_SEND_TUNING_BLOCK = 19,
	SD_SET_BLOCK_COUNT = 23,
	SD_WRITE_SINGLE_BLOCK = 24,
	SD_WRITE_MULTIPLE_BLOCK = 25,
	SD_ERASE_BLOCK_START = 32,
	SD_ERASE_BLOCK_END = 33,
	SD_ERASE_BLOCK_OPERATION = 38,
	SD_APP_CMD = 55,
	SD_SPI_READ_OCR = 58, /* SPI mode only */
	SD_SPI_CRC_ON_OFF = 59, /* SPI mode only */
};

enum sd_app_cmd {
	SD_APP_SET_BUS_WIDTH = 6,
	SD_APP_SEND_STATUS = 13,
	SD_APP_SEND_NUM_WRITTEN_BLK = 22,
	SD_APP_SET_WRITE_BLK_ERASE_CNT = 23,
	SD_APP_SEND_OP_COND = 41,
	SD_APP_CLEAR_CARD_DETECT = 42,
	SD_APP_SEND_SCR = 51,
};

struct cdns_sdmmc_sdhc {
	uint32_t	sdhc_extended_rd_mode;
	uint32_t	sdhc_extended_wr_mode;
	uint32_t	sdhc_hcsdclkadj;
	uint32_t	sdhc_idelay_val;
	uint32_t	sdhc_rdcmd_en;
	uint32_t	sdhc_rddata_en;
	uint32_t	sdhc_rw_compensate;
	uint32_t	sdhc_sdcfsh;
	uint32_t	sdhc_sdcfsl;
	uint32_t	sdhc_wrcmd0_dly;
	uint32_t	sdhc_wrcmd0_sdclk_dly;
	uint32_t	sdhc_wrcmd1_dly;
	uint32_t	sdhc_wrcmd1_sdclk_dly;
	uint32_t	sdhc_wrdata0_dly;
	uint32_t	sdhc_wrdata0_sdclk_dly;
	uint32_t	sdhc_wrdata1_dly;
	uint32_t	sdhc_wrdata1_sdclk_dly;
};

enum sdmmc_device_mode {
	SD_DS_ID, /* Identification */
	SD_DS, /* Default speed */
	SD_HS, /* High speed */
	SD_UHS_SDR12, /* Ultra high speed SDR12 */
	SD_UHS_SDR25, /* Ultra high speed SDR25 */
	SD_UHS_SDR50, /* Ultra high speed SDR`50 */
	SD_UHS_SDR104, /* Ultra high speed SDR104 */
	SD_UHS_DDR50, /* Ultra high speed DDR50 */
	EMMC_SDR_BC, /* SDR backward compatible */
	EMMC_SDR, /* SDR */
	EMMC_DDR, /* DDR */
	EMMC_HS200, /* High speed 200Mhz in SDR */
	EMMC_HS400, /* High speed 200Mhz in DDR */
	EMMC_HS400es, /* High speed 200Mhz in SDR with enhanced strobe*/
};

struct cdns_sdmmc_params {
	uintptr_t	reg_base;
	uintptr_t	reg_pinmux;
	uintptr_t	reg_phy;
	uintptr_t	desc_base;
	size_t		desc_size;
	int		clk_rate;
	int		bus_width;
	unsigned int	flags;
	enum sdmmc_device_mode	cdn_sdmmc_dev_mode;
	enum mmc_device_type	cdn_sdmmc_dev_type;
	uint32_t	combophy;
};

/* read and write API */
size_t sdmmc_read_blocks(int lba, uintptr_t buf, size_t size);
size_t sdmmc_write_blocks(int lba, const uintptr_t buf, size_t size);

struct cdns_idmac_desc {
	/*8 bit attribute*/
	uint8_t attr;
	/*reserved bits in desc*/
	uint8_t reserved;
	/*page length for the descriptor*/
	uint16_t len;
	/*lower 32 bits for buffer (64 bit addressing)*/
	uint32_t addr_lo;
#if CONFIG_DMA_ADDR_T_64BIT == 1
	/*higher 32 bits for buffer (64 bit addressing)*/
	uint32_t addr_hi;
} __aligned(8);
#else
} __packed;
#endif



/* Function Prototype */
int cdns_sd_host_init(struct cdns_sdmmc_combo_phy *mmc_combo_phy_reg,
struct cdns_sdmmc_sdhc *mmc_sdhc_reg);
void cdns_set_sdmmc_var(struct cdns_sdmmc_combo_phy *combo_phy_reg,
struct cdns_sdmmc_sdhc *sdhc_reg);
#endif
