/*
 * Copyright (c) 2019-2021,  STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/spi_nand.h>
#include <lib/utils.h>

#define SPI_NAND_MAX_ID_LEN		4U
#define DELAY_US_400MS			400000U
#define MACRONIX_ID			0xC2U

static struct spinand_device spinand_dev;

#pragma weak plat_get_spi_nand_data
int plat_get_spi_nand_data(struct spinand_device *device)
{
	return 0;
}

static int spi_nand_reg(bool read_reg, uint8_t reg, uint8_t *val,
			enum spi_mem_data_dir dir)
{
	struct spi_mem_op op;

	zeromem(&op, sizeof(struct spi_mem_op));
	if (read_reg) {
		op.cmd.opcode = SPI_NAND_OP_GET_FEATURE;
	} else {
		op.cmd.opcode = SPI_NAND_OP_SET_FEATURE;
	}

	op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	op.addr.val = reg;
	op.addr.nbytes = 1U;
	op.addr.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	op.data.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	op.data.dir = dir;
	op.data.nbytes = 1U;
	op.data.buf = val;

	return spi_mem_exec_op(&op);
}

static int spi_nand_read_reg(uint8_t reg, uint8_t *val)
{
	return spi_nand_reg(true, reg, val, SPI_MEM_DATA_IN);
}

static int spi_nand_write_reg(uint8_t reg, uint8_t val)
{
	return spi_nand_reg(false, reg, &val, SPI_MEM_DATA_OUT);
}

static int spi_nand_update_cfg(uint8_t mask, uint8_t val)
{
	int ret;
	uint8_t cfg = spinand_dev.cfg_cache;

	cfg &= ~mask;
	cfg |= val;

	if (cfg == spinand_dev.cfg_cache) {
		return 0;
	}

	ret = spi_nand_write_reg(SPI_NAND_REG_CFG, cfg);
	if (ret == 0) {
		spinand_dev.cfg_cache = cfg;
	}

	return ret;
}

static int spi_nand_ecc_enable(bool enable)
{
	return spi_nand_update_cfg(SPI_NAND_CFG_ECC_EN,
				   enable ? SPI_NAND_CFG_ECC_EN : 0U);
}

static int spi_nand_quad_enable(uint8_t manufacturer_id)
{
	bool enable = false;

	if (manufacturer_id != MACRONIX_ID) {
		return 0;
	}

	if (spinand_dev.spi_read_cache_op.data.buswidth ==
	    SPI_MEM_BUSWIDTH_4_LINE) {
		enable = true;
	}

	return spi_nand_update_cfg(SPI_NAND_CFG_QE,
				   enable ? SPI_NAND_CFG_QE : 0U);
}

static int spi_nand_wait_ready(uint8_t *status)
{
	int ret;
	uint64_t timeout = timeout_init_us(DELAY_US_400MS);

	while (!timeout_elapsed(timeout)) {
		ret = spi_nand_read_reg(SPI_NAND_REG_STATUS, status);
		if (ret != 0) {
			return ret;
		}

		VERBOSE("%s Status %x\n", __func__, *status);
		if ((*status & SPI_NAND_STATUS_BUSY) == 0U) {
			return 0;
		}
	}

	return -ETIMEDOUT;
}

static int spi_nand_reset(void)
{
	struct spi_mem_op op;
	uint8_t status;
	int ret;

	zeromem(&op, sizeof(struct spi_mem_op));
	op.cmd.opcode = SPI_NAND_OP_RESET;
	op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;

	ret = spi_mem_exec_op(&op);
	if (ret != 0) {
		return ret;
	}

	return spi_nand_wait_ready(&status);
}

static int spi_nand_read_id(uint8_t *id)
{
	struct spi_mem_op op;

	zeromem(&op, sizeof(struct spi_mem_op));
	op.cmd.opcode = SPI_NAND_OP_READ_ID;
	op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	op.data.dir = SPI_MEM_DATA_IN;
	op.data.nbytes = SPI_NAND_MAX_ID_LEN;
	op.data.buf = id;
	op.data.buswidth = SPI_MEM_BUSWIDTH_1_LINE;

	return spi_mem_exec_op(&op);
}

static int spi_nand_load_page(unsigned int page)
{
	struct spi_mem_op op;
	uint32_t block_nb = page / spinand_dev.nand_dev->block_size;
	uint32_t page_nb = page - (block_nb * spinand_dev.nand_dev->page_size);
	uint32_t nbpages_per_block = spinand_dev.nand_dev->block_size /
				     spinand_dev.nand_dev->page_size;
	uint32_t block_sh = __builtin_ctz(nbpages_per_block) + 1U;

	zeromem(&op, sizeof(struct spi_mem_op));
	op.cmd.opcode = SPI_NAND_OP_LOAD_PAGE;
	op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	op.addr.val = (block_nb << block_sh) | page_nb;
	op.addr.nbytes = 3U;
	op.addr.buswidth = SPI_MEM_BUSWIDTH_1_LINE;

	return spi_mem_exec_op(&op);
}

static int spi_nand_read_from_cache(unsigned int page, unsigned int offset,
				    uint8_t *buffer, unsigned int len)
{
	uint32_t nbpages_per_block = spinand_dev.nand_dev->block_size /
				     spinand_dev.nand_dev->page_size;
	uint32_t block_nb = page / nbpages_per_block;
	uint32_t page_sh = __builtin_ctz(spinand_dev.nand_dev->page_size) + 1U;

	spinand_dev.spi_read_cache_op.addr.val = offset;

	if ((spinand_dev.nand_dev->nb_planes > 1U) && ((block_nb % 2U) == 1U)) {
		spinand_dev.spi_read_cache_op.addr.val |= 1U << page_sh;
	}

	spinand_dev.spi_read_cache_op.data.buf = buffer;
	spinand_dev.spi_read_cache_op.data.nbytes = len;

	return spi_mem_exec_op(&spinand_dev.spi_read_cache_op);
}

static int spi_nand_read_page(unsigned int page, unsigned int offset,
			      uint8_t *buffer, unsigned int len,
			      bool ecc_enabled)
{
	uint8_t status;
	int ret;

	ret = spi_nand_ecc_enable(ecc_enabled);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nand_load_page(page);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nand_wait_ready(&status);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nand_read_from_cache(page, offset, buffer, len);
	if (ret != 0) {
		return ret;
	}

	if (ecc_enabled && ((status & SPI_NAND_STATUS_ECC_UNCOR) != 0U)) {
		return -EBADMSG;
	}

	return 0;
}

static int spi_nand_mtd_block_is_bad(unsigned int block)
{
	unsigned int nbpages_per_block = spinand_dev.nand_dev->block_size /
					 spinand_dev.nand_dev->page_size;
	uint8_t bbm_marker[2];
	int ret;

	ret = spi_nand_read_page(block * nbpages_per_block,
				 spinand_dev.nand_dev->page_size,
				 bbm_marker, sizeof(bbm_marker), false);
	if (ret != 0) {
		return ret;
	}

	if ((bbm_marker[0] != GENMASK_32(7, 0)) ||
	    (bbm_marker[1] != GENMASK_32(7, 0))) {
		WARN("Block %i is bad\n", block);
		return 1;
	}

	return 0;
}

static int spi_nand_mtd_read_page(struct nand_device *nand, unsigned int page,
				  uintptr_t buffer)
{
	return spi_nand_read_page(page, 0, (uint8_t *)buffer,
				  spinand_dev.nand_dev->page_size, true);
}

int spi_nand_init(unsigned long long *size, unsigned int *erase_size)
{
	uint8_t id[SPI_NAND_MAX_ID_LEN];
	int ret;

	spinand_dev.nand_dev = get_nand_device();
	if (spinand_dev.nand_dev == NULL) {
		return -EINVAL;
	}

	spinand_dev.nand_dev->mtd_block_is_bad = spi_nand_mtd_block_is_bad;
	spinand_dev.nand_dev->mtd_read_page = spi_nand_mtd_read_page;
	spinand_dev.nand_dev->nb_planes = 1;

	spinand_dev.spi_read_cache_op.cmd.opcode = SPI_NAND_OP_READ_FROM_CACHE;
	spinand_dev.spi_read_cache_op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	spinand_dev.spi_read_cache_op.addr.nbytes = 2U;
	spinand_dev.spi_read_cache_op.addr.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	spinand_dev.spi_read_cache_op.dummy.nbytes = 1U;
	spinand_dev.spi_read_cache_op.dummy.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	spinand_dev.spi_read_cache_op.data.buswidth = SPI_MEM_BUSWIDTH_1_LINE;

	if (plat_get_spi_nand_data(&spinand_dev) != 0) {
		return -EINVAL;
	}

	assert((spinand_dev.nand_dev->page_size != 0U) &&
	       (spinand_dev.nand_dev->block_size != 0U) &&
	       (spinand_dev.nand_dev->size != 0U));

	ret = spi_nand_reset();
	if (ret != 0) {
		return ret;
	}

	ret = spi_nand_read_id(id);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nand_read_reg(SPI_NAND_REG_CFG, &spinand_dev.cfg_cache);
	if (ret != 0) {
		return ret;
	}

	ret = spi_nand_quad_enable(id[1]);
	if (ret != 0) {
		return ret;
	}

	VERBOSE("SPI_NAND Detected ID 0x%x\n", id[1]);

	VERBOSE("Page size %i, Block size %i, size %lli\n",
		spinand_dev.nand_dev->page_size,
		spinand_dev.nand_dev->block_size,
		spinand_dev.nand_dev->size);

	*size = spinand_dev.nand_dev->size;
	*erase_size = spinand_dev.nand_dev->block_size;

	return 0;
}
