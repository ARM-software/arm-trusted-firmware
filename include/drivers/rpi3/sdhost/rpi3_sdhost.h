/*
 * Copyright (c) 2019, Linaro Limited
 * Copyright (c) 2019, Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI3_SDHOST_H
#define	RPI3_SDHOST_H

#include <drivers/mmc.h>
#include <stdint.h>
#include <platform_def.h>

struct rpi3_sdhost_params {
	uintptr_t	reg_base;
	uint32_t	clk_rate;
	uint32_t	bus_width;
	uint32_t        flags;
	uint32_t	current_cmd;
	uint8_t		cmdbusy;
	uint8_t		mmc_app_cmd;
	uint32_t	ns_per_fifo_word;

	uint32_t	sdcard_rca;
	uint32_t	gpio48_pinselect[6];
};

void rpi3_sdhost_init(struct rpi3_sdhost_params *params,
		      struct mmc_device_info *mmc_dev_info);
void rpi3_sdhost_stop(void);

/* Registers */
#define HC_COMMAND			0x00        /* Command and flags */
#define HC_ARGUMENT			0x04
#define HC_TIMEOUTCOUNTER		0x08
#define HC_CLOCKDIVISOR			0x0c
#define HC_RESPONSE_0			0x10
#define HC_RESPONSE_1			0x14
#define HC_RESPONSE_2			0x18
#define HC_RESPONSE_3			0x1c
#define HC_HOSTSTATUS			0x20
#define HC_POWER			0x30
#define HC_DEBUG			0x34
#define HC_HOSTCONFIG			0x38
#define HC_BLOCKSIZE			0x3c
#define HC_DATAPORT			0x40
#define HC_BLOCKCOUNT			0x50

/* Flags for HC_COMMAND register */
#define HC_CMD_ENABLE			0x8000
#define HC_CMD_FAILED			0x4000
#define HC_CMD_BUSY			0x0800
#define HC_CMD_RESPONSE_NONE		0x0400
#define HC_CMD_RESPONSE_LONG		0x0200
#define HC_CMD_WRITE			0x0080
#define HC_CMD_READ			0x0040
#define HC_CMD_COMMAND_MASK		0x003f

#define HC_CLOCKDIVISOR_MAXVAL		0x07ff
#define HC_CLOCKDIVISOR_PREFERVAL	0x027b
#define HC_CLOCKDIVISOR_SLOWVAL		0x0148
#define HC_CLOCKDIVISOR_STOPVAL		0x01fb

/* Flags for HC_HOSTSTATUS register */
#define HC_HSTST_HAVEDATA		0x0001
#define HC_HSTST_ERROR_FIFO		0x0008
#define HC_HSTST_ERROR_CRC7		0x0010
#define HC_HSTST_ERROR_CRC16		0x0020
#define HC_HSTST_TIMEOUT_CMD		0x0040
#define HC_HSTST_TIMEOUT_DATA		0x0080
#define HC_HSTST_INT_BLOCK		0x0200
#define HC_HSTST_INT_BUSY		0x0400

#define HC_HSTST_RESET			0xffff

#define HC_HSTST_MASK_ERROR_DATA	(HC_HSTST_ERROR_FIFO | \
					 HC_HSTST_ERROR_CRC7 | \
					 HC_HSTST_ERROR_CRC16 | \
					 HC_HSTST_TIMEOUT_DATA)

#define HC_HSTST_MASK_ERROR_ALL		(HC_HSTST_MASK_ERROR_DATA | \
					 HC_HSTST_TIMEOUT_CMD)

/* Flags for HC_HOSTCONFIG register */
#define HC_HSTCF_INTBUS_WIDE		0x0002
#define HC_HSTCF_EXTBUS_4BIT		0x0004
#define HC_HSTCF_SLOW_CARD		0x0008
#define HC_HSTCF_INT_DATA		0x0010
#define HC_HSTCF_INT_BLOCK		0x0100
#define HC_HSTCF_INT_BUSY		0x0400

/* Flags for HC_DEBUG register */
#define HC_DBG_FIFO_THRESH_WRITE_SHIFT	9
#define HC_DBG_FIFO_THRESH_READ_SHIFT	14
#define HC_DBG_FIFO_THRESH_MASK		0x001f
#define HC_DBG_FSM_MASK			0xf
#define HC_DBG_FSM_IDENTMODE		0x0
#define HC_DBG_FSM_DATAMODE		0x1
#define HC_DBG_FSM_READDATA		0x2
#define HC_DBG_FSM_WRITEDATA		0x3
#define HC_DBG_FSM_READWAIT		0x4
#define HC_DBG_FSM_READCRC		0x5
#define HC_DBG_FSM_WRITECRC		0x6
#define HC_DBG_FSM_WRITEWAIT1		0x7
#define HC_DBG_FSM_POWERDOWN		0x8
#define HC_DBG_FSM_POWERUP		0x9
#define HC_DBG_FSM_WRITESTART1		0xa
#define HC_DBG_FSM_WRITESTART2		0xb
#define HC_DBG_FSM_GENPULSES		0xc
#define HC_DBG_FSM_WRITEWAIT2		0xd
#define HC_DBG_FSM_STARTPOWDOWN		0xf
#define HC_DBG_FORCE_DATA_MODE		0x40000

/* Settings */
#define HC_FIFO_SIZE			16
#define HC_FIFO_THRESH_READ		4
#define HC_FIFO_THRESH_WRITE		4

#define HC_TIMEOUT_DEFAULT		0x00f00000
#define HC_TIMEOUT_IDLE			0x00a00000

#endif  /* RPI3_SDHOST_H */
