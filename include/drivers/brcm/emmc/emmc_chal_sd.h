/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef	CHAL_SD_H
#define	CHAL_SD_H

#include <stddef.h>

#define BASE_CLK_FREQ   (200 * 1000 * 1000)
#define INIT_CLK_FREQ   (400 * 1000)

#define SD_ERROR_RECOVERABLE                   0
#define SD_ERROR_NON_RECOVERABLE               1

#define SD_OK                                  0
#define SD_FAIL                                (-1)
#define SD_INVALID_HANDLE                      (-2)
#define SD_CEATA_INIT_ERROR                    (-3)
#define SD_RESET_ERROR                         (-4)
#define SD_CARD_INIT_ERROR                     (-5)
#define SD_INV_DATA_WIDTH                      (-6)
#define SD_SET_BUS_WIDTH_ERROR                 (-7)
#define SD_DMA_NOT_SUPPORT                     (-8)
#define SD_SDIO_READ_ERROR                     (-9)
#define SD_SDIO_WRITE_ERROR                    (-10)
#define SD_WRITE_ERROR                         (-11)
#define SD_READ_ERROR                          (-12)
#define SD_READ_SIZE_ERROR                     (-13)
#define SD_RW_ADDRESS_ERROR                    (-14)
#define SD_XFER_ADDRESS_ERROR                  (-15)
#define SD_DATA_XFER_ADDR_ERROR                (-16)
#define SD_DATA_XFER_ERROR                     (-17)
#define SD_WRITE_SIZE_ERROR                    (-18)
#define SD_CMD_STATUS_UPDATE_ERR               (-19)
#define SD_CMD12_ERROR                         (-20)
#define SD_CMD_DATA_ERROR                      (-21)
#define SD_CMD_TIMEOUT                         (-22)
#define SD_CMD_NO_RESPONSE                     (-22)
#define SD_CMD_ABORT_ERROR                     (-23)
#define SD_CMD_INVALID                         (-24)
#define SD_CMD_RESUME_ERROR                    (-25)
#define SD_CMD_ERR_INVALID_RESPONSE            (-26)
#define SD_WAIT_TIMEOUT                        (-27)
#define SD_READ_TIMEOUT                        (-28)
#define SD_CEATA_REST_ERROR                    (-29)
#define SD_INIT_CAED_FAILED                    (-30)
#define SD_ERROR_CLOCK_OFFLIMIT                (-31)
#define SD_INV_SLOT                            (-32)

#define SD_NOR_INTERRUPTS                      0x000000FF
#define SD_ERR_INTERRUPTS                      0x03FF0000
#define SD_CMD_ERROR_INT                       0x010F0000
#define SD_DAT_ERROR_INT                       0x02F00000
#define SD_DAT_TIMEOUT                         0x00100000

/* Operation modes */
#define SD_PIO_MODE		               0
#define SD_INT_MODE		               1

/* Support both ADMA and SDMA (for version 2.0 and above) */
#define SD_DMA_OFF                             0
#define SD_DMA_SDMA                            1
#define SD_DMA_ADMA                            2

#define SD_NORMAL_SPEED                        0
#define SD_HIGH_SPEED                          1

#define SD_XFER_CARD_TO_HOST                   3
#define SD_XFER_HOST_TO_CARD                   4

#define SD_CARD_DETECT_AUTO                    0
#define SD_CARD_DETECT_SD                      1
#define SD_CARD_DETECT_SDIO                    2
#define SD_CARD_DETECT_MMC                     3
#define SD_CARD_DETECT_CEATA                   4

#define SD_ABORT_SYNC_MODE                     0
#define SD_ABORT_ASYNC_MODE                    1

#define SD_CMD_ERROR_FLAGS                     (0x18F << 16)
#define SD_DATA_ERROR_FLAGS                    (0x70  << 16)
#define SD_AUTO_CMD12_ERROR_FLAGS              (0x9F)

#define SD_CARD_STATUS_ERROR                   0x10000000
#define SD_CMD_MISSING                         0x80000000
#define SD_ERROR_INT                           0x8000

#define SD_TRAN_HIGH_SPEED                     0x32
#define SD_CARD_HIGH_CAPACITY                  0x40000000
#define SD_CARD_POWER_UP_STATUS                0x80000000

#define	SD_HOST_CORE_TIMEOUT                   0x0E

/* SD CARD and Host Controllers bus width */
#define	SD_BUS_DATA_WIDTH_1BIT                 0x00
#define	SD_BUS_DATA_WIDTH_4BIT                 0x02
#define	SD_BUS_DATA_WIDTH_8BIT                 0x20

/* dma boundary settings */
#define SD_DMA_BOUNDARY_4K                     0
#define SD_DMA_BOUNDARY_8K                     (1 << 12)
#define SD_DMA_BOUNDARY_16K                    (2 << 12)
#define SD_DMA_BOUNDARY_32K                    (3 << 12)
#define SD_DMA_BOUNDARY_64K                    (4 << 12)
#define SD_DMA_BOUNDARY_128K                   (5 << 12)
#define SD_DMA_BOUNDARY_256K                   (6 << 12)
#define SD_DMA_BOUNDARY_512K                   (7 << 12)

#define SD_CMDR_CMD_NORMAL                     0x00000000
#define SD_CMDR_CMD_SUSPEND                    0x00400000
#define SD_CMDR_CMD_RESUME                     0x00800000
#define SD_CMDR_CMD_ABORT                      0x00c00000

#define SD_CMDR_RSP_TYPE_NONE                  0x0
#define SD_CMDR_RSP_TYPE_R2                    0x1
#define SD_CMDR_RSP_TYPE_R3_4                  0x2
#define SD_CMDR_RSP_TYPE_R1_5_6                0x2
#define SD_CMDR_RSP_TYPE_R1b_5b                0x3
#define SD_CMDR_RSP_TYPE_S                     16

struct sd_ctrl_info {
	uint32_t blkReg;	/* current block register cache value */
	uint32_t cmdReg;	/* current command register cache value */
	uint32_t argReg;	/* current argument register cache value */
	uint32_t cmdIndex;	/* current command index */
	uint32_t cmdStatus;	/* current command status, cmd/data compelete */
	uint16_t rca;	/* relative card address */
	uint32_t ocr;	/* operation codition */
	uint32_t eventList;	/* events list */
	uint32_t blkGapEnable;

	uint32_t capability;	/* controller's capbilities */
	uint32_t maxCurrent;	/* maximum current supported */
	uint32_t present;	/* if card is inserted or removed */
	uint32_t version;	/* SD spec version 1.0 or 2.0 */
	uint32_t vendor;	/* vendor number */

	uintptr_t sdRegBaseAddr;	/* sdio control registers */
	uintptr_t hostRegBaseAddr;	/* SD Host control registers */
};

struct sd_cfg {
	uint32_t mode;	/* interrupt or polling */
	uint32_t dma;	/* dma enabled or disabled */
	uint32_t retryLimit;	/* command retry limit */
	uint32_t speedMode;	/* speed mode, 0 standard, 1 high speed */
	uint32_t voltage;	/* voltage level */
	uint32_t blockSize;	/* access block size (512 for HC card) */
	uint32_t dmaBoundary;	/* dma address boundary */
	uint32_t detSignal;	/* card det signal src, for test purpose only */
	uint32_t rdWaiting;
	uint32_t wakeupOut;
	uint32_t wakeupIn;
	uint32_t wakeupInt;
	uint32_t wfe_retry;
	uint32_t gapInt;
	uint32_t readWait;
	uint32_t led;
};

struct sd_dev {
	struct sd_cfg cfg;		/* SD configuration */
	struct sd_ctrl_info ctrl;	/* SD info */
};

int32_t chal_sd_start(CHAL_HANDLE *sdHandle, uint32_t mode,
		      uint32_t sdBase, uint32_t hostBase);
int32_t chal_sd_config(CHAL_HANDLE *sdHandle, uint32_t speed,
		       uint32_t retry, uint32_t boundary,
		       uint32_t blkSize, uint32_t dma);
int32_t chal_sd_stop(void);
int32_t chal_sd_set_dma(CHAL_HANDLE *sdHandle, uint32_t mode);
uintptr_t chal_sd_get_dma_addr(CHAL_HANDLE *handle);
int32_t chal_sd_config_bus_width(CHAL_HANDLE *sdHandle, int32_t width);
int32_t chal_sd_send_cmd(CHAL_HANDLE *sdHandle, uint32_t cmdIndex,
			 uint32_t arg, uint32_t options);
int32_t chal_sd_set_dma_addr(CHAL_HANDLE *sdHandle, uintptr_t address);
int32_t chal_sd_set_clock(CHAL_HANDLE *sdHandle,
			  uint32_t div_ctrl_setting, uint32_t on);
uint32_t chal_sd_freq_2_div_ctrl_setting(uint32_t desired_freq);
int32_t chal_sd_setup_xfer(CHAL_HANDLE *sdHandle, uint8_t *data,
			   uint32_t length, int32_t dir);
int32_t chal_sd_write_buffer(CHAL_HANDLE *sdHandle, uint32_t length,
			     uint8_t *data);
int32_t chal_sd_read_buffer(CHAL_HANDLE *sdHandle, uint32_t length,
			    uint8_t *data);
int32_t chal_sd_reset_line(CHAL_HANDLE *sdHandle, uint32_t line);
int32_t chal_sd_get_response(CHAL_HANDLE *sdHandle, uint32_t *resp);
int32_t chal_sd_clear_pending_irq(CHAL_HANDLE *sdHandle);
int32_t chal_sd_get_irq_status(CHAL_HANDLE *sdHandle);
int32_t chal_sd_clear_irq(CHAL_HANDLE *sdHandle, uint32_t mask);
uint32_t chal_sd_get_present_status(CHAL_HANDLE *sdHandle);
int32_t chal_sd_get_atuo12_error(CHAL_HANDLE *sdHandle);
void chal_sd_set_speed(CHAL_HANDLE *sdHandle, uint32_t speed);
int32_t chal_sd_check_cap(CHAL_HANDLE *sdHandle, uint32_t cap);
void chal_sd_set_irq_signal(CHAL_HANDLE *sdHandle, uint32_t mask,
			    uint32_t state);
void chal_sd_dump_fifo(CHAL_HANDLE *sdHandle);
#endif /* CHAL_SD_H */
