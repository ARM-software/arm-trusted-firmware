/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SDIO_H
#define SDIO_H

#include <stdbool.h>

#define SR_IPROC_SDIO0_CFG_BASE      0x689006e4
#define SR_IPROC_SDIO0_SID_BASE      0x68900b00
#define SR_IPROC_SDIO0_PAD_BASE      0x68a4017c
#define SR_IPROC_SDIO0_IOCTRL_BASE   0x68e02408

#define SR_IPROC_SDIO1_CFG_BASE      0x68900734
#define SR_IPROC_SDIO1_SID_BASE      0x68900b08
#define SR_IPROC_SDIO1_PAD_BASE      0x68a401b4
#define SR_IPROC_SDIO1_IOCTRL_BASE   0x68e03408

#define NS3Z_IPROC_SDIO0_CFG_BASE    0x68a20540
#define NS3Z_IPROC_SDIO0_SID_BASE    0x68900b00
#define NS3Z_IPROC_SDIO0_TP_OUT_SEL  0x68a20308
#define NS3Z_IPROC_SDIO0_PAD_BASE    0x68a20500
#define NS3Z_IPROC_SDIO0_IOCTRL_BASE 0x68e02408

#define PHY_BYPASS      BIT(14)
#define LEGACY_EN       BIT(31)
#define PHY_DISABLE     (LEGACY_EN | PHY_BYPASS)

#define NS3Z_IPROC_SDIO1_CFG_BASE    0x68a30540
#define NS3Z_IPROC_SDIO1_SID_BASE    0x68900b08
#define NS3Z_IPROC_SDIO1_PAD_BASE    0x68a30500
#define NS3Z_IPROC_SDIO1_IOCTRL_BASE 0x68e03408

#define ICFG_SDIO_CAP0          0x10
#define ICFG_SDIO_CAP1          0x14
#define ICFG_SDIO_STRAPSTATUS_0 0x0
#define ICFG_SDIO_STRAPSTATUS_1 0x4
#define ICFG_SDIO_STRAPSTATUS_2 0x8
#define ICFG_SDIO_STRAPSTATUS_3 0xc
#define ICFG_SDIO_STRAPSTATUS_4 0x18

#define ICFG_SDIO_SID_ARADDR    0x0
#define ICFG_SDIO_SID_AWADDR    0x4

#define ICFG_SDIOx_CAP0__SLOT_TYPE_MASK         0x3
#define ICFG_SDIOx_CAP0__SLOT_TYPE_SHIFT        27
#define ICFG_SDIOx_CAP0__INT_MODE_SHIFT         26
#define ICFG_SDIOx_CAP0__SYS_BUS_64BIT_SHIFT    25
#define ICFG_SDIOx_CAP0__VOLTAGE_1P8V_SHIFT     24
#define ICFG_SDIOx_CAP0__VOLTAGE_3P0V_SHIFT     23
#define ICFG_SDIOx_CAP0__VOLTAGE_3P3V_SHIFT     22
#define ICFG_SDIOx_CAP0__SUSPEND_RESUME_SHIFT   21
#define ICFG_SDIOx_CAP0__SDMA_SHIFT             20
#define ICFG_SDIOx_CAP0__HIGH_SPEED_SHIFT       19
#define ICFG_SDIOx_CAP0__ADMA2_SHIFT            18
#define ICFG_SDIOx_CAP0__EXTENDED_MEDIA_SHIFT   17
#define ICFG_SDIOx_CAP0__MAX_BLOCK_LEN_MASK     0x3
#define ICFG_SDIOx_CAP0__MAX_BLOCK_LEN_SHIFT    15
#define ICFG_SDIOx_CAP0__BASE_CLK_FREQ_MASK     0xff
#define ICFG_SDIOx_CAP0__BASE_CLK_FREQ_SHIFT    7
#define ICFG_SDIOx_CAP0__TIMEOUT_UNIT_SHIFT     6
#define ICFG_SDIOx_CAP0__TIMEOUT_CLK_FREQ_MASK  0x3f
#define ICFG_SDIOx_CAP0__TIMEOUT_CLK_FREQ_SHIFT 0

#define ICFG_SDIOx_CAP1__SPI_BLOCK_MODE_SHIFT   22
#define ICFG_SDIOx_CAP1__SPI_MODE_SHIFT         21
#define ICFG_SDIOx_CAP1__CLK_MULT_MASK          0xff
#define ICFG_SDIOx_CAP1__CLK_MULT_SHIFT         13
#define ICFG_SDIOx_CAP1__RETUNING_MODE_MASK     0x3
#define ICFG_SDIOx_CAP1__RETUNING_MODE_SHIFT    11
#define ICFG_SDIOx_CAP1__TUNE_SDR50_SHIFT       10
#define ICFG_SDIOx_CAP1__TIME_RETUNE_MASK       0xf
#define ICFG_SDIOx_CAP1__TIME_RETUNE_SHIFT      6
#define ICFG_SDIOx_CAP1__DRIVER_D_SHIFT         5
#define ICFG_SDIOx_CAP1__DRIVER_C_SHIFT         4
#define ICFG_SDIOx_CAP1__DRIVER_A_SHIFT         3
#define ICFG_SDIOx_CAP1__DDR50_SHIFT            2
#define ICFG_SDIOx_CAP1__SDR104_SHIFT           1
#define ICFG_SDIOx_CAP1__SDR50_SHIFT            0

#ifdef USE_DDR
#define SDIO_DMA  1
#else
#define SDIO_DMA  0
#endif

#define SDIO0_CAP0_CFG  \
	(0x1 << ICFG_SDIOx_CAP0__SLOT_TYPE_SHIFT) \
	| (0x0 << ICFG_SDIOx_CAP0__INT_MODE_SHIFT) \
	| (0x0 << ICFG_SDIOx_CAP0__SYS_BUS_64BIT_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__VOLTAGE_1P8V_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__VOLTAGE_3P0V_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__VOLTAGE_3P3V_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__SUSPEND_RESUME_SHIFT) \
	| (SDIO_DMA << ICFG_SDIOx_CAP0__SDMA_SHIFT) \
	| (SDIO_DMA << ICFG_SDIOx_CAP0__ADMA2_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__HIGH_SPEED_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__EXTENDED_MEDIA_SHIFT) \
	| (0x2 << ICFG_SDIOx_CAP0__MAX_BLOCK_LEN_SHIFT) \
	| (0xc8 << ICFG_SDIOx_CAP0__BASE_CLK_FREQ_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__TIMEOUT_UNIT_SHIFT) \
	| (0x30 << ICFG_SDIOx_CAP0__TIMEOUT_CLK_FREQ_SHIFT)

#define SDIO0_CAP1_CFG  \
	(0x1 << ICFG_SDIOx_CAP1__SPI_BLOCK_MODE_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__SPI_MODE_SHIFT)\
	| (0x0 << ICFG_SDIOx_CAP1__CLK_MULT_SHIFT)\
	| (0x2 << ICFG_SDIOx_CAP1__RETUNING_MODE_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__TUNE_SDR50_SHIFT)\
	| (0x0 << ICFG_SDIOx_CAP1__DRIVER_D_SHIFT)\
	| (0x0 << ICFG_SDIOx_CAP1__DRIVER_C_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__DRIVER_A_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__DDR50_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__SDR104_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__SDR50_SHIFT)

#define SDIO1_CAP0_CFG  \
	(0x0 << ICFG_SDIOx_CAP0__SLOT_TYPE_SHIFT) \
	| (0x0 << ICFG_SDIOx_CAP0__INT_MODE_SHIFT) \
	| (0x0 << ICFG_SDIOx_CAP0__SYS_BUS_64BIT_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__VOLTAGE_1P8V_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__VOLTAGE_3P0V_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__VOLTAGE_3P3V_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__SUSPEND_RESUME_SHIFT) \
	| (SDIO_DMA << ICFG_SDIOx_CAP0__SDMA_SHIFT) \
	| (SDIO_DMA << ICFG_SDIOx_CAP0__ADMA2_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__HIGH_SPEED_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__EXTENDED_MEDIA_SHIFT) \
	| (0x2 << ICFG_SDIOx_CAP0__MAX_BLOCK_LEN_SHIFT) \
	| (0xc8 << ICFG_SDIOx_CAP0__BASE_CLK_FREQ_SHIFT) \
	| (0x1 << ICFG_SDIOx_CAP0__TIMEOUT_UNIT_SHIFT) \
	| (0x30 << ICFG_SDIOx_CAP0__TIMEOUT_CLK_FREQ_SHIFT)

#define SDIO1_CAP1_CFG  \
	(0x1 << ICFG_SDIOx_CAP1__SPI_BLOCK_MODE_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__SPI_MODE_SHIFT)\
	| (0x0 << ICFG_SDIOx_CAP1__CLK_MULT_SHIFT)\
	| (0x2 << ICFG_SDIOx_CAP1__RETUNING_MODE_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__TUNE_SDR50_SHIFT)\
	| (0x0 << ICFG_SDIOx_CAP1__DRIVER_D_SHIFT)\
	| (0x0 << ICFG_SDIOx_CAP1__DRIVER_C_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__DRIVER_A_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__DDR50_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__SDR104_SHIFT)\
	| (0x1 << ICFG_SDIOx_CAP1__SDR50_SHIFT)

#define PAD_SDIO_CLK      0x4
#define PAD_SDIO_DATA0    0x8
#define PAD_SDIO_DATA1    0xc
#define PAD_SDIO_DATA2    0x10
#define PAD_SDIO_DATA3    0x14
#define PAD_SDIO_DATA4    0x18
#define PAD_SDIO_DATA5    0x1c
#define PAD_SDIO_DATA6    0x20
#define PAD_SDIO_DATA7    0x24
#define PAD_SDIO_CMD      0x28

/* 12mA Drive strength*/
#define PAD_SDIO_SELX     (0x5 << 1)
#define PAD_SDIO_SRC      (1 << 0)
#define PAD_SDIO_MASK     (0xF << 0)
#define PAD_SDIO_VALUE    (PAD_SDIO_SELX | PAD_SDIO_SRC)

/*
 * SDIO_PRESETVAL0
 *
 * Each 13 Bit filed consists:
 * drivestrength - 12:11
 * clkgensel - b10
 * sdkclkfreqsel - 9:0
 * Field		Bit(s)		Description
 * ============================================================
 * SDR25_PRESET		25:13		Preset Value for SDR25
 * SDR50_PRESET		12:0		Preset Value for SDR50
 */
#define SDIO_PRESETVAL0		0x01005001

/*
 * SDIO_PRESETVAL1
 *
 * Each 13 Bit filed consists:
 * drivestrength - 12:11
 * clkgensel - b10
 * sdkclkfreqsel - 9:0
 * Field		Bit(s)		Description
 * ============================================================
 * SDR104_PRESET		25:13		Preset Value for SDR104
 * SDR12_PRESET		12:0		Preset Value for SDR12
 */
#define SDIO_PRESETVAL1		0x03000004

/*
 * SDIO_PRESETVAL2
 *
 * Each 13 Bit filed consists:
 * drivestrength - 12:11
 * clkgensel - b10
 * sdkclkfreqsel - 9:0
 * Field		Bit(s)		Description
 * ============================================================
 * HIGH_SPEED_PRESET	25:13		Preset Value for High Speed
 * INIT_PRESET		12:0		Preset Value for Initialization
 */
#define SDIO_PRESETVAL2		0x010040FA

/*
 * SDIO_PRESETVAL3
 *
 * Each 13 Bit filed consists:
 * drivestrength - 12:11
 * clkgensel - b10
 * sdkclkfreqsel - 9:0
 * Field		Bit(s)		Description
 * ============================================================
 * DDR50_PRESET		25:13		Preset Value for DDR50
 * DEFAULT_PRESET	12:0		Preset Value for Default Speed
 */
#define SDIO_PRESETVAL3		0x01004004

/*
 * SDIO_PRESETVAL4
 *
 * Field			Bit(s)		Description
 * ============================================================
 * FORCE_USE_IP_TUNE_CLK	30		Force use IP clock
 * TUNING_COUNT			29:24		Tuning count
 * OVERRIDE_1P8V		23:16
 * OVERRIDE_3P3V		15:8
 * OVERRIDE_3P0V		7:0
 */
#define SDIO_PRESETVAL4		0x20010101

#define SDIO_SID_SHIFT		5

typedef struct {
	uintptr_t cfg_base;
	uintptr_t sid_base;
	uintptr_t io_ctrl_base;
	uintptr_t pad_base;
} SDIO_CFG;

void brcm_stingray_sdio_init(void);

#endif /* SDIO_H */
