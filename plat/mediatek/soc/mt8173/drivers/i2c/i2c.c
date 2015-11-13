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
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <i2c_private.h>
#include <mmio.h>
#include <mt8173_def.h>
#include <platform_def.h>
#include <string.h>

struct i2c_cmd_buf {
	uint8_t addr;
	uint8_t *write_buf;
	uint8_t *read_buf;
	int write_buf_len;
	int read_buf_len;
};

static uint8_t * const dma_buf = (void *)DMA_BUF_BASE;
static const uint32_t dma_buf_size = DMA_BUF_SIZE;

#define PERI_GLOBALCON_PDN0_SET	(PERI_CON_BASE + 0x08)
#define PERI_GLOBALCON_PDN0_CLR	(PERI_CON_BASE + 0x10)
#define AP_DMA_PDN		(1U << 12)
#define I2C0_PDN		(1U << 23)
#define I2C1_PDN		(1U << 24)
#define I2C2_PDN		(1U << 25)
#define I2C3_PDN		(1U << 26)
#define I2C4_PDN		(1U << 27)

#define PERI_GLOBALCON_PDN1_SET	(PERI_CON_BASE + 0x0c)
#define PERI_GLOBALCON_PDN1_CLR	(PERI_CON_BASE + 0x14)
#define I2C6_PDN		(1U << 13)

#define MAX_MT8173_I2C_BUS	7

#ifndef KHz
#define KHz	(1000)
#endif
#ifndef MHz
#define MHz	(1000 * KHz)
#endif

#define I2C_CLK_HZ (273 * MHz / 16)

static const struct mtk_i2c i2c[MAX_MT8173_I2C_BUS] = {
	/* i2c0 setting */
	{
		.i2c_regs = (void *)I2C_BASE,
		.i2c_dma_regs = (void *)(I2C_DMA_BASE + 0x80),
	},

	/* i2c1 setting */
	{
		.i2c_regs = (void *)(I2C_BASE + 0x1000),
		.i2c_dma_regs = (void *)(I2C_DMA_BASE + 0x100),
	},

	/* i2c2 setting */
	{
		.i2c_regs = (void *)(I2C_BASE + 0x2000),
		.i2c_dma_regs = (void *)(I2C_DMA_BASE + 0x180),
	},

	/* i2c3 setting */
	{
		.i2c_regs = (void *)(I2C_BASE + 0x9000),
		.i2c_dma_regs = (void *)(I2C_DMA_BASE + 0x200),
	},

	 /* i2c4 setting */
	{
		.i2c_regs = (void *)(I2C_BASE + 0xa000),
		.i2c_dma_regs = (void *)(I2C_DMA_BASE + 0x280),
	},

	/* i2c5 is reserved for internal use. */
	{
		.i2c_regs = NULL,
		.i2c_dma_regs = NULL,
	},

	/* i2c6 setting */
	{
		.i2c_regs = (void *)(I2C_BASE + 0xc000),
		.i2c_dma_regs = (void *)I2C_DMA_BASE,
	}
};

static inline void i2c_dma_reset(struct mt8173_i2c_dma_regs *dma_regs)
{
	mmio_write_32((uintptr_t)&dma_regs->dma_rst, 0x1);
	udelay(50);
	mmio_write_32((uintptr_t)&dma_regs->dma_rst, 0x2);
	udelay(50);
	mmio_write_32((uintptr_t)&dma_regs->dma_rst, 0x0);
	udelay(50);
}

static inline void mtk_i2c_dump_info(uint8_t bus)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	struct mt8173_i2c_regs *regs = i2c[bus].i2c_regs;

	INFO("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\n"
	     "CONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\n"
	     "TIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\n"
	     "DEBUGSTAT %x\nEXT_CONF %x\n",
		mmio_read_32((uintptr_t)&regs->slave_addr),
		mmio_read_32((uintptr_t)&regs->intr_mask),
		mmio_read_32((uintptr_t)&regs->intr_stat),
		mmio_read_32((uintptr_t)&regs->control),
		mmio_read_32((uintptr_t)&regs->transfer_len),
		mmio_read_32((uintptr_t)&regs->transac_len),
		mmio_read_32((uintptr_t)&regs->delay_len),
		mmio_read_32((uintptr_t)&regs->timing),
		mmio_read_32((uintptr_t)&regs->start),
		mmio_read_32((uintptr_t)&regs->fifo_stat),
		mmio_read_32((uintptr_t)&regs->io_config),
		mmio_read_32((uintptr_t)&regs->hs),
		mmio_read_32((uintptr_t)&regs->debug_stat),
		mmio_read_32((uintptr_t)&regs->ext_conf));

	INFO("addr address %x\n", (uint32_t)(uintptr_t)regs);
#endif
}

static uint32_t mtk_i2c_transfer(uint8_t bus, struct i2c_cmd_buf *cmd_buf,
				 enum i2c_modes read)
{
	uint32_t ret_code = I2C_OK;
	uint16_t status;
	uint8_t  addr;
	uint32_t write_len = 0;
	uint32_t read_len = 0;
	uint8_t *write_buffer = NULL;
	uint8_t *read_buffer = NULL;
	uint8_t sample_div;
	uint8_t step_div;
	uint32_t retry;
	struct mt8173_i2c_regs *regs;
	struct mt8173_i2c_dma_regs *dma_regs;

	regs = i2c[bus].i2c_regs;
	dma_regs = i2c[bus].i2c_dma_regs;

	addr = cmd_buf->addr;

	switch (read) {
	case I2C_WRITE_MODE:
		write_len = cmd_buf->write_buf_len;
		write_buffer = cmd_buf->write_buf;
		assert(write_len > 0 && write_len <= 255);
		break;

	case I2C_READ_MODE:
		read_len = cmd_buf->read_buf_len;
		read_buffer = cmd_buf->read_buf;
		assert(read_len > 0 && read_len <= 255);
		break;

	/* Must use special write-then-read mode for repeated starts. */
	case I2C_WRITE_READ_MODE:
		write_len = cmd_buf->write_buf_len;
		read_len = cmd_buf->read_buf_len;
		write_buffer = cmd_buf->write_buf;
		read_buffer = cmd_buf->read_buf;
		assert(write_len > 0 && write_len <= 255);
		assert(read_len > 0 && read_len <= 255);
		break;
	}

	/* Calculate i2c frequency */
	sample_div = 2;
	step_div = I2C_CLK_HZ / (400 * KHz * sample_div * 2);
	if (I2C_CLK_HZ % (400 * KHz * sample_div * 2) > 0)
		step_div++;
	assert(sample_div < 8);
	mmio_write_32((uintptr_t)&regs->timing,
		(sample_div - 1) << 8 | (step_div - 1));

	mmio_write_32((uintptr_t)&regs->hs, I2C_HS_DEFAULT_VALUE);

	/* Clear interrupt status */
	mmio_write_32((uintptr_t)&regs->intr_stat,
		I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

	mmio_write_32((uintptr_t)&regs->fifo_addr_clr, 0x1);

	/* Enable interrupt */
	mmio_write_32((uintptr_t)&regs->intr_mask,
		I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

	switch (read) {
	case I2C_WRITE_MODE:
		assert(write_len < dma_buf_size);
		memcpy(dma_buf, write_buffer, write_len);

		/* control registers */
		mmio_write_32((uintptr_t)&regs->control, ACK_ERR_DET_EN |
			DMA_EN | CLK_EXT | REPEATED_START_FLAG);

		/* Set transfer and transaction len */
		mmio_write_32((uintptr_t)&regs->transac_len, 1);
		mmio_write_32((uintptr_t)&regs->transfer_len, write_len);

		/* set i2c write slave address*/
		mmio_write_32((uintptr_t)&regs->slave_addr, addr << 1);

		/* Prepare buffer data to start transfer */
		mmio_write_32((uintptr_t)&dma_regs->dma_con, I2C_DMA_CON_TX);
		mmio_write_32((uintptr_t)&dma_regs->dma_tx_mem_addr,
			(uintptr_t)dma_buf);
		mmio_write_32((uintptr_t)&dma_regs->dma_tx_len, write_len);
		break;

	case I2C_READ_MODE:
		/* control registers */
		mmio_write_32((uintptr_t)&regs->control, ACK_ERR_DET_EN |
			DMA_EN | CLK_EXT | REPEATED_START_FLAG);

		/* Set transfer and transaction len */
		mmio_write_32((uintptr_t)&regs->transac_len, 1);
		mmio_write_32((uintptr_t)&regs->transfer_len, read_len);

		/* set i2c read slave address*/
		mmio_write_32((uintptr_t)&regs->slave_addr, (addr << 1 | 0x1));

		/* Prepare buffer data to start transfer */
		mmio_write_32((uintptr_t)&dma_regs->dma_con, I2C_DMA_CON_RX);
		mmio_write_32((uintptr_t)&dma_regs->dma_rx_mem_addr,
			(uintptr_t)dma_buf);
		mmio_write_32((uintptr_t)&dma_regs->dma_rx_len, read_len);
		break;

	case I2C_WRITE_READ_MODE:
		assert(write_len < dma_buf_size);
		memcpy(dma_buf, write_buffer, write_len);

		/* control registers */
		mmio_write_32((uintptr_t)&regs->control,
			DIR_CHG | ACK_ERR_DET_EN | DMA_EN | CLK_EXT |
			REPEATED_START_FLAG);

		/* Set transfer and transaction len */
		mmio_write_32((uintptr_t)&regs->transfer_len, write_len);
		mmio_write_32((uintptr_t)&regs->transfer_aux_len, read_len);
		mmio_write_32((uintptr_t)&regs->transac_len, 2);

		/* set i2c write slave address*/
		mmio_write_32((uintptr_t)&regs->slave_addr, addr << 1);

		/* Prepare buffer data to start transfer */
		mmio_write_32((uintptr_t)&dma_regs->dma_con, I2C_DMA_CLR_FLAG);
		mmio_write_32((uintptr_t)&dma_regs->dma_tx_mem_addr,
			(uintptr_t)dma_buf);
		mmio_write_32((uintptr_t)&dma_regs->dma_tx_len, write_len);
		mmio_write_32((uintptr_t)&dma_regs->dma_rx_mem_addr,
			(uintptr_t)dma_buf);
		mmio_write_32((uintptr_t)&dma_regs->dma_rx_len, read_len);
		break;
	}

	mmio_write_32((uintptr_t)&dma_regs->dma_int_flag, I2C_DMA_CLR_FLAG);
	mmio_write_32((uintptr_t)&dma_regs->dma_en, I2C_DMA_START_EN);

	/* start transfer transaction */
	mmio_write_32((uintptr_t)&regs->start, 0x1);

	/* polling mode : see if transaction complete */
	retry = 10; /* timeout: 100ms (polling 10 times in 10ms interval) */
	while (1) {
		mdelay(10);
		status = mmio_read_32((uintptr_t)&regs->intr_stat);
		if (status & I2C_HS_NACKERR) {
			ret_code = I2C_TRANSFER_FAIL_HS_NACKERR;
			ERROR("[i2c%d transfer] transaction NACK error\n", bus);
			mtk_i2c_dump_info(bus);
			break;
		} else if (status & I2C_ACKERR) {
			ret_code = I2C_TRANSFER_FAIL_ACKERR;
			ERROR("[i2c%d transfer] transaction ACK error\n", bus);
			mtk_i2c_dump_info(bus);
			break;
		} else if (status & I2C_TRANSAC_COMP) {
			ret_code = I2C_OK;
			assert(read_len < dma_buf_size);
			memcpy(read_buffer, dma_buf, read_len);
			break;
		}

		retry--;
		if (retry == 0) {
			ret_code = I2C_TRANSFER_FAIL_TIMEOUT;
			ERROR("[i2c%d transfer] transaction timeout\n", bus);
			mtk_i2c_dump_info(bus);
			break;
		}
	}

	mmio_write_32((uintptr_t)&regs->intr_stat,
		I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

	/* clear bit mask */
	mmio_write_32((uintptr_t)&regs->intr_mask,
		I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

	/* reset the i2c controller for next i2c transfer. */
	mmio_write_32((uintptr_t)&regs->softreset, 0x1);

	i2c_dma_reset(dma_regs);

	return ret_code;
}

int i2c_readb(uint8_t bus, uint8_t addr, uint8_t reg, uint8_t *data)
{
	struct i2c_cmd_buf cmd_buf;

	cmd_buf.addr = addr;
	cmd_buf.write_buf = &reg;
	cmd_buf.write_buf_len = 1;
	cmd_buf.read_buf = data;
	cmd_buf.read_buf_len = 1;

	return mtk_i2c_transfer(bus, &cmd_buf, I2C_WRITE_READ_MODE);
}

int i2c_writeb(uint8_t bus, uint8_t addr, uint8_t reg, uint8_t data)
{
	struct i2c_cmd_buf cmd_buf;
	uint8_t buf[] = { reg, data };

	cmd_buf.addr = addr;
	cmd_buf.write_buf = buf;
	cmd_buf.write_buf_len = 2;

	return mtk_i2c_transfer(bus, &cmd_buf, I2C_WRITE_MODE);
}

void i2c_clock_ctrl(uint32_t i2c_bus, int enable)
{
	assert((i2c_bus < MAX_MT8173_I2C_BUS) && (i2c_bus != 5));

	/* i2c bus clock */
	switch (i2c_bus) {
	case 6:
		if (enable)
			mmio_write_32(PERI_GLOBALCON_PDN1_CLR, I2C6_PDN);
		else
			mmio_setbits_32(PERI_GLOBALCON_PDN1_SET, I2C6_PDN);
		break;
	default:
		if (enable)
			mmio_write_32(PERI_GLOBALCON_PDN0_CLR,
				I2C0_PDN << i2c_bus);
		else
			mmio_setbits_32(PERI_GLOBALCON_PDN0_SET,
				I2C0_PDN << i2c_bus);
		break;
	}

	/* ap dma clock */
	if (enable)
		mmio_setbits_32(PERI_GLOBALCON_PDN0_CLR, AP_DMA_PDN);
	else
		mmio_setbits_32(PERI_GLOBALCON_PDN0_SET, AP_DMA_PDN);
}
