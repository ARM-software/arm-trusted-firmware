/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __PLAT_DRIVER_I2C_PRIVATE_H__
#define __PLAT_DRIVER_I2C_PRIVATE_H__

/* I2C Configuration */
enum {
	I2C_HS_DEFAULT_VALUE	 = 0x0102,
};

enum i2c_modes {
	I2C_WRITE_MODE		= 0,
	I2C_READ_MODE		= 1,
	I2C_WRITE_READ_MODE	= 2,
};

enum {
	I2C_DMA_CON_TX          = 0x0,
	I2C_DMA_CON_RX          = 0x1,
	I2C_DMA_START_EN        = 0x1,
	I2C_DMA_INT_FLAG_NONE   = 0x0,
	I2C_DMA_CLR_FLAG        = 0x0,
	I2C_DMA_FLUSH_FLAG      = 0x1,
};

/* I2C DMA Registers */
struct mt8173_i2c_dma_regs {
	uint32_t dma_int_flag;
	uint32_t dma_int_en;
	uint32_t dma_en;
	uint32_t dma_rst;
	uint32_t reserved1;
	uint32_t dma_flush;
	uint32_t dma_con;
	uint32_t dma_tx_mem_addr;
	uint32_t dma_rx_mem_addr;
	uint32_t dma_tx_len;
	uint32_t dma_rx_len;
};

/* I2C Register */
struct mt8173_i2c_regs {
	uint32_t data_port;
	uint32_t slave_addr;
	uint32_t intr_mask;
	uint32_t intr_stat;
	uint32_t control;
	uint32_t transfer_len;
	uint32_t transac_len;
	uint32_t delay_len;
	uint32_t timing;
	uint32_t start;
	uint32_t ext_conf;
	uint32_t reserved1;
	uint32_t fifo_stat;
	uint32_t fifo_thresh;
	uint32_t fifo_addr_clr;
	uint32_t reserved2;
	uint32_t io_config;
	uint32_t debug;
	uint32_t hs;
	uint32_t reserved3;
	uint32_t softreset;
	uint32_t dcm;
	uint32_t reserved4[3];
	uint32_t debug_stat;
	uint32_t debug_ctrl;
	uint32_t transfer_aux_len;
};

struct mtk_i2c {
	struct mt8173_i2c_regs *i2c_regs;
	struct mt8173_i2c_dma_regs *i2c_dma_regs;
};

enum {
	I2C_TRANS_LEN_MASK = (0xff),
	I2C_TRANS_AUX_LEN_MASK = (0x1f << 8),
	I2C_CONTROL_MASK = (0x3f << 1)
};

/* Register mask */
enum {
	I2C_HS_NACKERR = (1 << 2),
	I2C_ACKERR = (1 << 1),
	I2C_TRANSAC_COMP = (1 << 0),
};

/* i2c control bits */
enum {
	ACK_ERR_DET_EN = (1 << 5),
	DIR_CHG = (1 << 4),
	CLK_EXT = (1 << 3),
	DMA_EN = (1 << 2),
	REPEATED_START_FLAG = (1 << 1),
	STOP_FLAG = (0 << 1)
};

/* I2C Status Code */

enum {
	I2C_OK = 0x0000,
	I2C_SET_SPEED_FAIL_OVER_SPEED = 0xA001,
	I2C_TRANSFER_INVALID_LENGTH = 0xA002,
	I2C_TRANSFER_FAIL_HS_NACKERR = 0xA003,
	I2C_TRANSFER_FAIL_ACKERR = 0xA004,
	I2C_TRANSFER_FAIL_TIMEOUT = 0xA005,
	I2C_TRANSFER_INVALID_ARGUMENT = 0xA006
};

#endif /* __PLAT_DRIVER_I2C_PRIVATE_H__ */
