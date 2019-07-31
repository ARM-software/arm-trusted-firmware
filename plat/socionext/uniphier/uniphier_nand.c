/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include "uniphier.h"

#define NAND_CMD_READ0		0
#define NAND_CMD_READSTART	0x30

#define DENALI_ECC_ENABLE			0x0e0
#define DENALI_PAGES_PER_BLOCK			0x150
#define DENALI_DEVICE_MAIN_AREA_SIZE		0x170
#define DENALI_DEVICE_SPARE_AREA_SIZE		0x180
#define DENALI_TWO_ROW_ADDR_CYCLES		0x190
#define DENALI_INTR_STATUS0			0x410
#define   DENALI_INTR_ECC_UNCOR_ERR			BIT(1)
#define   DENALI_INTR_DMA_CMD_COMP			BIT(2)
#define   DENALI_INTR_INT_ACT				BIT(12)

#define DENALI_DMA_ENABLE			0x700

#define DENALI_HOST_ADDR			0x00
#define DENALI_HOST_DATA			0x10

#define DENALI_MAP01				(1 << 26)
#define DENALI_MAP10				(2 << 26)
#define DENALI_MAP11				(3 << 26)

#define DENALI_MAP11_CMD			((DENALI_MAP11) | 0)
#define DENALI_MAP11_ADDR			((DENALI_MAP11) | 1)
#define DENALI_MAP11_DATA			((DENALI_MAP11) | 2)

#define DENALI_ACCESS_DEFAULT_AREA		0x42

#define UNIPHIER_NAND_BBT_UNKNOWN		0xff

struct uniphier_nand {
	uintptr_t host_base;
	uintptr_t reg_base;
	int pages_per_block;
	int page_size;
	int two_row_addr_cycles;
	uint8_t bbt[16];
};

struct uniphier_nand uniphier_nand;

static void uniphier_nand_host_write(struct uniphier_nand *nand,
				     uint32_t addr, uint32_t data)
{
	mmio_write_32(nand->host_base + DENALI_HOST_ADDR, addr);
	mmio_write_32(nand->host_base + DENALI_HOST_DATA, data);
}

static uint32_t uniphier_nand_host_read(struct uniphier_nand *nand,
					uint32_t addr)
{
	mmio_write_32(nand->host_base + DENALI_HOST_ADDR, addr);
	return mmio_read_32(nand->host_base + DENALI_HOST_DATA);
}

static int uniphier_nand_block_isbad(struct uniphier_nand *nand, int block)
{
	int page = nand->pages_per_block * block;
	int column = nand->page_size;
	uint8_t bbm;
	uint32_t status;
	int is_bad;

	/* use cache if available */
	if (block < ARRAY_SIZE(nand->bbt) &&
	    nand->bbt[block] != UNIPHIER_NAND_BBT_UNKNOWN)
		return nand->bbt[block];

	mmio_write_32(nand->reg_base + DENALI_ECC_ENABLE, 0);

	mmio_write_32(nand->reg_base + DENALI_INTR_STATUS0, -1);

	uniphier_nand_host_write(nand, DENALI_MAP11_CMD, NAND_CMD_READ0);
	uniphier_nand_host_write(nand, DENALI_MAP11_ADDR, column & 0xff);
	uniphier_nand_host_write(nand, DENALI_MAP11_ADDR, (column >> 8) & 0xff);
	uniphier_nand_host_write(nand, DENALI_MAP11_ADDR, page & 0xff);
	uniphier_nand_host_write(nand, DENALI_MAP11_ADDR, (page >> 8) & 0xff);
	if (!nand->two_row_addr_cycles)
		uniphier_nand_host_write(nand, DENALI_MAP11_ADDR,
					 (page >> 16) & 0xff);
	uniphier_nand_host_write(nand, DENALI_MAP11_CMD, NAND_CMD_READSTART);

	do {
		status = mmio_read_32(nand->reg_base + DENALI_INTR_STATUS0);
	} while (!(status & DENALI_INTR_INT_ACT));

	bbm = uniphier_nand_host_read(nand, DENALI_MAP11_DATA);

	is_bad = bbm != 0xff;

	/* if possible, save the result for future re-use */
	if (block < ARRAY_SIZE(nand->bbt))
		nand->bbt[block] = is_bad;

	if (is_bad)
		WARN("found bad block at %d. skip.\n", block);

	return is_bad;
}

static int uniphier_nand_read_pages(struct uniphier_nand *nand, uintptr_t buf,
				    int page_start, int page_count)
{
	uint32_t status;

	mmio_write_32(nand->reg_base + DENALI_ECC_ENABLE, 1);
	mmio_write_32(nand->reg_base + DENALI_DMA_ENABLE, 1);

	mmio_write_32(nand->reg_base + DENALI_INTR_STATUS0, -1);

	/* use Data DMA (64bit) */
	mmio_write_32(nand->host_base + DENALI_HOST_ADDR,
		      DENALI_MAP10 | page_start);

	/*
	 * 1. setup transfer type, interrupt when complete,
	 *    burst len = 64 bytes, the number of pages
	 */
	mmio_write_32(nand->host_base + DENALI_HOST_DATA,
		      0x01002000 | (64 << 16) | page_count);

	/* 2. set memory low address */
	mmio_write_32(nand->host_base + DENALI_HOST_DATA, buf);

	/* 3. set memory high address */
	mmio_write_32(nand->host_base + DENALI_HOST_DATA, buf >> 32);

	do {
		status = mmio_read_32(nand->reg_base + DENALI_INTR_STATUS0);
	} while (!(status & DENALI_INTR_DMA_CMD_COMP));

	mmio_write_32(nand->reg_base + DENALI_DMA_ENABLE, 0);

	if (status & DENALI_INTR_ECC_UNCOR_ERR) {
		ERROR("uncorrectable error in page range %d-%d",
		      page_start, page_start + page_count - 1);
		return -EBADMSG;
	}

	return 0;
}

static size_t __uniphier_nand_read(struct uniphier_nand *nand, int lba,
				   uintptr_t buf, size_t size)
{
	int pages_per_block = nand->pages_per_block;
	int page_size = nand->page_size;
	int blocks_to_skip = lba / pages_per_block;
	int pages_to_read = div_round_up(size, page_size);
	int page = lba % pages_per_block;
	int block = 0;
	uintptr_t p = buf;
	int page_count, ret;

	while (blocks_to_skip) {
		ret = uniphier_nand_block_isbad(nand, block);
		if (ret < 0)
			goto out;

		if (!ret)
			blocks_to_skip--;

		block++;
	}

	while (pages_to_read) {
		ret = uniphier_nand_block_isbad(nand, block);
		if (ret < 0)
			goto out;

		if (ret) {
			block++;
			continue;
		}

		page_count = MIN(pages_per_block - page, pages_to_read);

		ret = uniphier_nand_read_pages(nand, p,
					       block * pages_per_block + page,
					       page_count);
		if (ret)
			goto out;

		block++;
		page = 0;
		p += page_size * page_count;
		pages_to_read -= page_count;
	}

out:
	/* number of read bytes */
	return MIN(size, p - buf);
}

static size_t uniphier_nand_read(int lba, uintptr_t buf, size_t size)
{
	size_t count;

	inv_dcache_range(buf, size);

	count = __uniphier_nand_read(&uniphier_nand, lba, buf, size);

	inv_dcache_range(buf, size);

	return count;
}

static struct io_block_dev_spec uniphier_nand_dev_spec = {
	.buffer = {
		.offset = UNIPHIER_BLOCK_BUF_BASE,
		.length = UNIPHIER_BLOCK_BUF_SIZE,
	},
	.ops = {
		.read = uniphier_nand_read,
	},
	/* fill .block_size at run-time */
};

static int uniphier_nand_hw_init(struct uniphier_nand *nand)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nand->bbt); i++)
		nand->bbt[i] = UNIPHIER_NAND_BBT_UNKNOWN;

	nand->host_base = 0x68000000;
	nand->reg_base = 0x68100000;

	nand->pages_per_block =
			mmio_read_32(nand->reg_base + DENALI_PAGES_PER_BLOCK);

	nand->page_size =
		mmio_read_32(nand->reg_base + DENALI_DEVICE_MAIN_AREA_SIZE);

	if (mmio_read_32(nand->reg_base + DENALI_TWO_ROW_ADDR_CYCLES) & BIT(0))
		nand->two_row_addr_cycles = 1;

	uniphier_nand_host_write(nand, DENALI_MAP10,
				 DENALI_ACCESS_DEFAULT_AREA);

	return 0;
}

int uniphier_nand_init(uintptr_t *block_dev_spec)
{
	int ret;

	ret = uniphier_nand_hw_init(&uniphier_nand);
	if (ret)
		return ret;

	uniphier_nand_dev_spec.block_size = uniphier_nand.page_size;

	*block_dev_spec = (uintptr_t)&uniphier_nand_dev_spec;

	return 0;
}
