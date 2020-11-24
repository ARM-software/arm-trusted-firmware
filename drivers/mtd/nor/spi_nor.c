/*
 * Copyright (c) 2019-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/spi_nor.h>
#include <lib/utils.h>

#define SR_WIP			BIT(0)	/* Write in progress */
#define CR_QUAD_EN_SPAN		BIT(1)	/* Spansion Quad I/O */
#define SR_QUAD_EN_MX		BIT(6)	/* Macronix Quad I/O */
#define FSR_READY		BIT(7)	/* Device status, 0 = Busy, 1 = Ready */

/* Defined IDs for supported memories */
#define SPANSION_ID		0x01U
#define MACRONIX_ID		0xC2U
#define MICRON_ID		0x2CU

#define BANK_SIZE		0x1000000U

#define SPI_READY_TIMEOUT_US	40000U

static struct nor_device nor_dev;

#pragma weak plat_get_nor_data
int plat_get_nor_data(struct nor_device *device)
{
	return 0;
}

static int spi_nor_reg(uint8_t reg, uint8_t *buf, size_t len,
		       enum spi_mem_data_dir dir)
{
	struct spi_mem_op op;

	zeromem(&op, sizeof(struct spi_mem_op));
	op.cmd.opcode = reg;
	op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	op.data.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	op.data.dir = dir;
	op.data.nbytes = len;
	op.data.buf = buf;

	return spi_mem_exec_op(&op);
}

static inline int spi_nor_read_id(uint8_t *id)
{
	return spi_nor_reg(SPI_NOR_OP_READ_ID, id, 1U, SPI_MEM_DATA_IN);
}

static inline int spi_nor_read_cr(uint8_t *cr)
{
	return spi_nor_reg(SPI_NOR_OP_READ_CR, cr, 1U, SPI_MEM_DATA_IN);
}

static inline int spi_nor_read_sr(uint8_t *sr)
{
	return spi_nor_reg(SPI_NOR_OP_READ_SR, sr, 1U, SPI_MEM_DATA_IN);
}

static inline int spi_nor_read_fsr(uint8_t *fsr)
{
	return spi_nor_reg(SPI_NOR_OP_READ_FSR, fsr, 1U, SPI_MEM_DATA_IN);
}

static inline int spi_nor_write_en(void)
{
	return spi_nor_reg(SPI_NOR_OP_WREN, NULL, 0U, SPI_MEM_DATA_OUT);
}

/*
 * Check if device is ready.
 *
 * Return 0 if ready, 1 if busy or a negative error code otherwise
 */
static int spi_nor_ready(void)
{
	uint8_t sr;
	int ret;

	ret = spi_nor_read_sr(&sr);
	if (ret != 0) {
		return ret;
	}

	if ((nor_dev.flags & SPI_NOR_USE_FSR) != 0U) {
		uint8_t fsr;

		ret = spi_nor_read_fsr(&fsr);
		if (ret != 0) {
			return ret;
		}

		return (((fsr & FSR_READY) != 0U) && ((sr & SR_WIP) == 0U)) ?
			0 : 1;
	}

	return (((sr & SR_WIP) == 0U) ? 0 : 1);
}

static int spi_nor_wait_ready(void)
{
	int ret;
	uint64_t timeout = timeout_init_us(SPI_READY_TIMEOUT_US);

	while (!timeout_elapsed(timeout)) {
		ret = spi_nor_ready();
		if (ret <= 0) {
			return ret;
		}
	}

	return -ETIMEDOUT;
}

static int spi_nor_macronix_quad_enable(void)
{
	uint8_t sr;
	int ret;

	ret = spi_nor_read_sr(&sr);
	if (ret != 0) {
		return ret;
	}

	if ((sr & SR_QUAD_EN_MX) != 0U) {
		return 0;
	}

	ret = spi_nor_write_en();
	if (ret != 0) {
		return ret;
	}

	sr |= SR_QUAD_EN_MX;
	ret = spi_nor_reg(SPI_NOR_OP_WRSR, &sr, 1U, SPI_MEM_DATA_OUT);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nor_wait_ready();
	if (ret != 0) {
		return ret;
	}

	ret = spi_nor_read_sr(&sr);
	if ((ret != 0) || ((sr & SR_QUAD_EN_MX) == 0U)) {
		return -EINVAL;
	}

	return 0;
}

static int spi_nor_write_sr_cr(uint8_t *sr_cr)
{
	int ret;

	ret = spi_nor_write_en();
	if (ret != 0) {
		return ret;
	}

	ret = spi_nor_reg(SPI_NOR_OP_WRSR, sr_cr, 2U, SPI_MEM_DATA_OUT);
	if (ret != 0) {
		return -EINVAL;
	}

	ret = spi_nor_wait_ready();
	if (ret != 0) {
		return ret;
	}

	return 0;
}

static int spi_nor_quad_enable(void)
{
	uint8_t sr_cr[2];
	int ret;

	ret = spi_nor_read_cr(&sr_cr[1]);
	if (ret != 0) {
		return ret;
	}

	if ((sr_cr[1] & CR_QUAD_EN_SPAN) != 0U) {
		return 0;
	}

	sr_cr[1] |= CR_QUAD_EN_SPAN;
	ret = spi_nor_read_sr(&sr_cr[0]);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nor_write_sr_cr(sr_cr);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nor_read_cr(&sr_cr[1]);
	if ((ret != 0) || ((sr_cr[1] & CR_QUAD_EN_SPAN) == 0U)) {
		return -EINVAL;
	}

	return 0;
}

static int spi_nor_clean_bar(void)
{
	int ret;

	if (nor_dev.selected_bank == 0U) {
		return 0;
	}

	nor_dev.selected_bank = 0U;

	ret = spi_nor_write_en();
	if (ret != 0) {
		return ret;
	}

	return spi_nor_reg(nor_dev.bank_write_cmd, &nor_dev.selected_bank,
			   1U, SPI_MEM_DATA_OUT);
}

static int spi_nor_write_bar(uint32_t offset)
{
	uint8_t selected_bank = offset / BANK_SIZE;
	int ret;

	if (selected_bank == nor_dev.selected_bank) {
		return 0;
	}

	ret = spi_nor_write_en();
	if (ret != 0) {
		return ret;
	}

	ret = spi_nor_reg(nor_dev.bank_write_cmd, &selected_bank,
			  1U, SPI_MEM_DATA_OUT);
	if (ret != 0) {
		return ret;
	}

	nor_dev.selected_bank = selected_bank;

	return 0;
}

static int spi_nor_read_bar(void)
{
	uint8_t selected_bank = 0U;
	int ret;

	ret = spi_nor_reg(nor_dev.bank_read_cmd, &selected_bank,
			  1U, SPI_MEM_DATA_IN);
	if (ret != 0) {
		return ret;
	}

	nor_dev.selected_bank = selected_bank;

	return 0;
}

int spi_nor_read(unsigned int offset, uintptr_t buffer, size_t length,
		 size_t *length_read)
{
	size_t remain_len;
	int ret;

	*length_read = 0U;
	nor_dev.read_op.addr.val = offset;
	nor_dev.read_op.data.buf = (void *)buffer;

	VERBOSE("%s offset %i length %zu\n", __func__, offset, length);

	while (length != 0U) {
		if ((nor_dev.flags & SPI_NOR_USE_BANK) != 0U) {
			ret = spi_nor_write_bar(nor_dev.read_op.addr.val);
			if (ret != 0) {
				return ret;
			}

			remain_len = (BANK_SIZE * (nor_dev.selected_bank + 1)) -
				nor_dev.read_op.addr.val;
			nor_dev.read_op.data.nbytes = MIN(length, remain_len);
		} else {
			nor_dev.read_op.data.nbytes = length;
		}

		ret = spi_mem_exec_op(&nor_dev.read_op);
		if (ret != 0) {
			spi_nor_clean_bar();
			return ret;
		}

		length -= nor_dev.read_op.data.nbytes;
		nor_dev.read_op.addr.val += nor_dev.read_op.data.nbytes;
		nor_dev.read_op.data.buf += nor_dev.read_op.data.nbytes;
		*length_read += nor_dev.read_op.data.nbytes;
	}

	if ((nor_dev.flags & SPI_NOR_USE_BANK) != 0U) {
		ret = spi_nor_clean_bar();
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

int spi_nor_init(unsigned long long *size, unsigned int *erase_size)
{
	int ret;
	uint8_t id;

	/* Default read command used */
	nor_dev.read_op.cmd.opcode = SPI_NOR_OP_READ;
	nor_dev.read_op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	nor_dev.read_op.addr.nbytes = 3U;
	nor_dev.read_op.addr.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	nor_dev.read_op.data.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	nor_dev.read_op.data.dir = SPI_MEM_DATA_IN;

	if (plat_get_nor_data(&nor_dev) != 0) {
		return -EINVAL;
	}

	assert(nor_dev.size != 0U);

	if (nor_dev.size > BANK_SIZE) {
		nor_dev.flags |= SPI_NOR_USE_BANK;
	}

	*size = nor_dev.size;

	ret = spi_nor_read_id(&id);
	if (ret != 0) {
		return ret;
	}

	if ((nor_dev.flags & SPI_NOR_USE_BANK) != 0U) {
		switch (id) {
		case SPANSION_ID:
			nor_dev.bank_read_cmd = SPINOR_OP_BRRD;
			nor_dev.bank_write_cmd = SPINOR_OP_BRWR;
			break;
		default:
			nor_dev.bank_read_cmd = SPINOR_OP_RDEAR;
			nor_dev.bank_write_cmd = SPINOR_OP_WREAR;
			break;
		}
	}

	if (nor_dev.read_op.data.buswidth == 4U) {
		switch (id) {
		case MACRONIX_ID:
			INFO("Enable Macronix quad support\n");
			ret = spi_nor_macronix_quad_enable();
			break;
		case MICRON_ID:
			break;
		default:
			ret = spi_nor_quad_enable();
			break;
		}
	}

	if ((ret == 0) && ((nor_dev.flags & SPI_NOR_USE_BANK) != 0U)) {
		ret = spi_nor_read_bar();
	}

	return ret;
}
