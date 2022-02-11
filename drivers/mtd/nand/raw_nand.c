/*
 * Copyright (c) 2019-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/raw_nand.h>
#include <lib/utils.h>

#include <platform_def.h>

#define ONFI_SIGNATURE_ADDR	0x20U

/* CRC calculation */
#define CRC_POLYNOM		0x8005U
#define CRC_INIT_VALUE		0x4F4EU

/* Status register */
#define NAND_STATUS_READY	BIT(6)

static struct rawnand_device rawnand_dev;

#pragma weak plat_get_raw_nand_data
int plat_get_raw_nand_data(struct rawnand_device *device)
{
	return 0;
}

static int nand_send_cmd(uint8_t cmd, unsigned int tim)
{
	struct nand_req req;

	zeromem(&req, sizeof(struct nand_req));
	req.nand = rawnand_dev.nand_dev;
	req.type = NAND_REQ_CMD | cmd;
	req.inst_delay = tim;

	return rawnand_dev.ops->exec(&req);
}

static int nand_send_addr(uint8_t addr, unsigned int tim)
{
	struct nand_req req;

	zeromem(&req, sizeof(struct nand_req));
	req.nand = rawnand_dev.nand_dev;
	req.type = NAND_REQ_ADDR;
	req.addr = &addr;
	req.inst_delay = tim;

	return rawnand_dev.ops->exec(&req);
}

static int nand_send_wait(unsigned int delay, unsigned int tim)
{
	struct nand_req req;

	zeromem(&req, sizeof(struct nand_req));
	req.nand = rawnand_dev.nand_dev;
	req.type = NAND_REQ_WAIT;
	req.inst_delay = tim;
	req.delay_ms = delay;

	return rawnand_dev.ops->exec(&req);
}


static int nand_read_data(uint8_t *data, unsigned int length, bool use_8bit)
{
	struct nand_req req;

	zeromem(&req, sizeof(struct nand_req));
	req.nand = rawnand_dev.nand_dev;
	req.type = NAND_REQ_DATAIN | (use_8bit ? NAND_REQ_BUS_WIDTH_8 : 0U);
	req.addr = data;
	req.length = length;

	return rawnand_dev.ops->exec(&req);
}

int nand_change_read_column_cmd(unsigned int offset, uintptr_t buffer,
				unsigned int len)
{
	int ret;
	uint8_t addr[2];
	unsigned int i;

	ret = nand_send_cmd(NAND_CMD_CHANGE_1ST, 0U);
	if (ret !=  0) {
		return ret;
	}

	if (rawnand_dev.nand_dev->buswidth == NAND_BUS_WIDTH_16) {
		offset /= 2U;
	}

	addr[0] = offset;
	addr[1] = offset >> 8;

	for (i = 0; i < 2U; i++) {
		ret = nand_send_addr(addr[i], 0U);
		if (ret !=  0) {
			return ret;
		}
	}

	ret = nand_send_cmd(NAND_CMD_CHANGE_2ND, NAND_TCCS_MIN);
	if (ret !=  0) {
		return ret;
	}

	return nand_read_data((uint8_t *)buffer, len, false);
}

int nand_read_page_cmd(unsigned int page, unsigned int offset,
		       uintptr_t buffer, unsigned int len)
{
	uint8_t addr[5];
	uint8_t i = 0U;
	uint8_t j;
	int ret;

	VERBOSE(">%s page %u offset %u buffer 0x%lx\n", __func__, page, offset,
		buffer);

	if (rawnand_dev.nand_dev->buswidth == NAND_BUS_WIDTH_16) {
		offset /= 2U;
	}

	addr[i++] = offset;
	addr[i++] = offset >> 8;

	addr[i++] = page;
	addr[i++] = page >> 8;
	if (rawnand_dev.nand_dev->size > SZ_128M) {
		addr[i++] = page >> 16;
	}

	ret = nand_send_cmd(NAND_CMD_READ_1ST, 0U);
	if (ret != 0) {
		return ret;
	}

	for (j = 0U; j < i; j++) {
		ret = nand_send_addr(addr[j], 0U);
		if (ret != 0) {
			return ret;
		}
	}

	ret = nand_send_cmd(NAND_CMD_READ_2ND, NAND_TWB_MAX);
	if (ret != 0) {
		return ret;
	}

	ret = nand_send_wait(PSEC_TO_MSEC(NAND_TR_MAX), NAND_TRR_MIN);
	if (ret != 0) {
		return ret;
	}

	if (buffer != 0U) {
		ret = nand_read_data((uint8_t *)buffer, len, false);
	}

	return ret;
}

static int nand_status(uint8_t *status)
{
	int ret;

	ret = nand_send_cmd(NAND_CMD_STATUS, NAND_TWHR_MIN);
	if (ret != 0) {
		return ret;
	}

	if (status != NULL) {
		ret = nand_read_data(status, 1U, true);
	}

	return ret;
}

int nand_wait_ready(unsigned int delay_ms)
{
	uint8_t status;
	int ret;
	uint64_t timeout;

	/* Wait before reading status */
	udelay(1);

	ret = nand_status(NULL);
	if (ret != 0) {
		return ret;
	}

	timeout = timeout_init_us(delay_ms * 1000U);
	while (!timeout_elapsed(timeout)) {
		ret = nand_read_data(&status, 1U, true);
		if (ret != 0) {
			return ret;
		}

		if ((status & NAND_STATUS_READY) != 0U) {
			return nand_send_cmd(NAND_CMD_READ_1ST, 0U);
		}

		udelay(10);
	}

	return -ETIMEDOUT;
}

#if NAND_ONFI_DETECT
static uint16_t nand_check_crc(uint16_t crc, uint8_t *data_in,
			       unsigned int data_len)
{
	uint32_t i;
	uint32_t j;
	uint32_t bit;

	for (i = 0U; i < data_len; i++) {
		uint8_t cur_param = *data_in++;

		for (j = BIT(7); j != 0U; j >>= 1) {
			bit = crc & BIT(15);
			crc <<= 1;

			if ((cur_param & j) != 0U) {
				bit ^= BIT(15);
			}

			if (bit != 0U) {
				crc ^= CRC_POLYNOM;
			}
		}

		crc &= GENMASK(15, 0);
	}

	return crc;
}

static int nand_read_id(uint8_t addr, uint8_t *id, unsigned int size)
{
	int ret;

	ret = nand_send_cmd(NAND_CMD_READID, 0U);
	if (ret !=  0) {
		return ret;
	}

	ret = nand_send_addr(addr, NAND_TWHR_MIN);
	if (ret !=  0) {
		return ret;
	}

	return nand_read_data(id, size, true);
}

static int nand_reset(void)
{
	int ret;

	ret = nand_send_cmd(NAND_CMD_RESET, NAND_TWB_MAX);
	if (ret != 0) {
		return ret;
	}

	return nand_send_wait(PSEC_TO_MSEC(NAND_TRST_MAX), 0U);
}

static int nand_read_param_page(void)
{
	struct nand_param_page page;
	uint8_t addr = 0U;
	int ret;

	ret = nand_send_cmd(NAND_CMD_READ_PARAM_PAGE, 0U);
	if (ret != 0) {
		return ret;
	}

	ret = nand_send_addr(addr, NAND_TWB_MAX);
	if (ret != 0) {
		return ret;
	}

	ret = nand_send_wait(PSEC_TO_MSEC(NAND_TR_MAX), NAND_TRR_MIN);
	if (ret != 0) {
		return ret;
	}

	ret = nand_read_data((uint8_t *)&page, sizeof(page), true);
	if (ret != 0) {
		return ret;
	}

	if (strncmp((char *)&page.page_sig, "ONFI", 4) != 0) {
		WARN("Error ONFI detection\n");
		return -EINVAL;
	}

	if (nand_check_crc(CRC_INIT_VALUE, (uint8_t *)&page, 254U) !=
	    page.crc16) {
		WARN("Error reading param\n");
		return -EINVAL;
	}

	if ((page.features & ONFI_FEAT_BUS_WIDTH_16) != 0U) {
		rawnand_dev.nand_dev->buswidth = NAND_BUS_WIDTH_16;
	} else {
		rawnand_dev.nand_dev->buswidth = NAND_BUS_WIDTH_8;
	}

	rawnand_dev.nand_dev->block_size = page.num_pages_per_blk *
					   page.bytes_per_page;
	rawnand_dev.nand_dev->page_size = page.bytes_per_page;
	rawnand_dev.nand_dev->size = page.num_pages_per_blk *
				     page.bytes_per_page *
				     page.num_blk_in_lun * page.num_lun;

	if (page.nb_ecc_bits != GENMASK_32(7, 0)) {
		rawnand_dev.nand_dev->ecc.max_bit_corr = page.nb_ecc_bits;
		rawnand_dev.nand_dev->ecc.size = SZ_512;
	}

	VERBOSE("Page size %u, block_size %u, Size %llu, ecc %u, buswidth %u\n",
		rawnand_dev.nand_dev->page_size,
		rawnand_dev.nand_dev->block_size, rawnand_dev.nand_dev->size,
		rawnand_dev.nand_dev->ecc.max_bit_corr,
		rawnand_dev.nand_dev->buswidth);

	return 0;
}

static int detect_onfi(void)
{
	int ret;
	char id[4];

	ret = nand_reset();
	if (ret != 0) {
		return ret;
	}

	ret = nand_read_id(ONFI_SIGNATURE_ADDR, (uint8_t *)id, sizeof(id));
	if (ret != 0) {
		return ret;
	}

	if (strncmp(id, "ONFI", sizeof(id)) != 0) {
		WARN("NAND Non ONFI detected\n");
		return -ENODEV;
	}

	return nand_read_param_page();
}
#endif

static int nand_mtd_block_is_bad(unsigned int block)
{
	unsigned int nbpages_per_block = rawnand_dev.nand_dev->block_size /
					 rawnand_dev.nand_dev->page_size;
	uint8_t bbm_marker[2];
	uint8_t page;
	int ret;

	for (page = 0U; page < 2U; page++) {
		ret = nand_read_page_cmd(block * nbpages_per_block,
					 rawnand_dev.nand_dev->page_size,
					 (uintptr_t)bbm_marker,
					 sizeof(bbm_marker));
		if (ret != 0) {
			return ret;
		}

		if ((bbm_marker[0] != GENMASK_32(7, 0)) ||
		    (bbm_marker[1] != GENMASK_32(7, 0))) {
			WARN("Block %u is bad\n", block);
			return 1;
		}
	}

	return 0;
}

static int nand_mtd_read_page_raw(struct nand_device *nand, unsigned int page,
				  uintptr_t buffer)
{
	return nand_read_page_cmd(page, 0U, buffer,
				  rawnand_dev.nand_dev->page_size);
}

void nand_raw_ctrl_init(const struct nand_ctrl_ops *ops)
{
	rawnand_dev.ops = ops;
}

int nand_raw_init(unsigned long long *size, unsigned int *erase_size)
{
	rawnand_dev.nand_dev = get_nand_device();
	if (rawnand_dev.nand_dev == NULL) {
		return -EINVAL;
	}

	rawnand_dev.nand_dev->mtd_block_is_bad = nand_mtd_block_is_bad;
	rawnand_dev.nand_dev->mtd_read_page = nand_mtd_read_page_raw;
	rawnand_dev.nand_dev->ecc.mode = NAND_ECC_NONE;

	if ((rawnand_dev.ops->setup == NULL) ||
	    (rawnand_dev.ops->exec == NULL)) {
		return -ENODEV;
	}

#if NAND_ONFI_DETECT
	if (detect_onfi() != 0) {
		WARN("Detect ONFI failed\n");
	}
#endif

	if (plat_get_raw_nand_data(&rawnand_dev) != 0) {
		return -EINVAL;
	}

	assert((rawnand_dev.nand_dev->page_size != 0U) &&
	       (rawnand_dev.nand_dev->block_size != 0U) &&
	       (rawnand_dev.nand_dev->size != 0U));

	*size = rawnand_dev.nand_dev->size;
	*erase_size = rawnand_dev.nand_dev->block_size;

	rawnand_dev.ops->setup(rawnand_dev.nand_dev);

	return 0;
}
