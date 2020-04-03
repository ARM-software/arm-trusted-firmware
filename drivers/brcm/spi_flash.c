/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>

#include <sf.h>
#include <spi.h>

#define SPI_FLASH_CMD_LEN	4
#define QSPI_WAIT_TIMEOUT_US	200000U /* usec */

#define FINFO(jedec_id, ext_id, _sector_size, _n_sectors, _page_size, _flags) \
		.id = {							\
			((jedec_id) >> 16) & 0xff,			\
			((jedec_id) >> 8) & 0xff,			\
			(jedec_id) & 0xff,				\
			((ext_id) >> 8) & 0xff,				\
			(ext_id) & 0xff,				\
			},						\
		.id_len = (!(jedec_id) ? 0 : (3 + ((ext_id) ? 2 : 0))),	\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = _page_size,				\
		.flags = (_flags),

/* SPI/QSPI flash device params structure */
const struct spi_flash_info spi_flash_ids[] = {
	{"W25Q64CV", FINFO(0xef4017, 0x0, 64 * 1024, 128, 256, WR_QPP | SECT_4K)},
	{"W25Q64DW", FINFO(0xef6017, 0x0, 64 * 1024, 128, 256, WR_QPP | SECT_4K)},
	{"W25Q32",   FINFO(0xef4016, 0x0, 64 * 1024, 64,  256, SECT_4K)},
	{"MX25l3205D",  FINFO(0xc22016, 0x0, 64 * 1024, 64, 256, SECT_4K)},
};

static void spi_flash_addr(uint32_t addr, uint8_t *cmd)
{
	/*
	 * cmd[0] holds a SPI Flash command, stored earlier
	 * cmd[1/2/3] holds 24bit flash address
	 */
	cmd[1] = addr >> 16;
	cmd[2] = addr >> 8;
	cmd[3] = addr >> 0;
}

static const struct spi_flash_info *spi_flash_read_id(void)
{
	const struct spi_flash_info *info;
	uint8_t id[SPI_FLASH_MAX_ID_LEN];
	int ret;

	ret = spi_flash_cmd(CMD_READ_ID, id, SPI_FLASH_MAX_ID_LEN);
	if (ret < 0) {
		ERROR("SF: Error %d reading JEDEC ID\n", ret);
		return NULL;
	}

	for (info = spi_flash_ids; info->name != NULL; info++) {
		if (info->id_len) {
			if (!memcmp(info->id, id, info->id_len))
				return info;
		}
	}

	printf("SF: unrecognized JEDEC id bytes: %02x, %02x, %02x\n",
	       id[0], id[1], id[2]);
	return NULL;
}

/* Enable writing on the SPI flash */
static inline int spi_flash_cmd_write_enable(struct spi_flash *flash)
{
	return spi_flash_cmd(CMD_WRITE_ENABLE, NULL, 0);
}

static int spi_flash_cmd_wait(struct spi_flash *flash)
{
	uint8_t cmd;
	uint32_t i;
	uint8_t status;
	int ret;

	i = 0;
	while (1) {
		cmd = CMD_RDSR;
		ret = spi_flash_cmd_read(&cmd, 1, &status, 1);
		if (ret < 0) {
			ERROR("SF: cmd wait failed\n");
			break;
		}
		if (!(status & STATUS_WIP))
			break;

		i++;
		if (i >= QSPI_WAIT_TIMEOUT_US) {
			ERROR("SF: cmd wait timeout\n");
			ret = -1;
			break;
		}
		udelay(1);
	}

	return ret;
}

static int spi_flash_write_common(struct spi_flash *flash, const uint8_t *cmd,
				  size_t cmd_len, const void *buf,
				  size_t buf_len)
{
	int ret;

	ret = spi_flash_cmd_write_enable(flash);
	if (ret < 0) {
		ERROR("SF: enabling write failed\n");
		return ret;
	}

	ret = spi_flash_cmd_write(cmd, cmd_len, buf, buf_len);
	if (ret < 0) {
		ERROR("SF: write cmd failed\n");
		return ret;
	}

	ret = spi_flash_cmd_wait(flash);
	if (ret < 0) {
		ERROR("SF: write timed out\n");
		return ret;
	}

	return ret;
}

static int spi_flash_read_common(const uint8_t *cmd, size_t cmd_len,
				 void *data, size_t data_len)
{
	int ret;

	ret = spi_flash_cmd_read(cmd, cmd_len, data, data_len);
	if (ret < 0) {
		ERROR("SF: read cmd failed\n");
		return ret;
	}

	return ret;
}

int spi_flash_read(struct spi_flash *flash, uint32_t offset,
		   uint32_t len, void *data)
{
	uint32_t read_len = 0, read_addr;
	uint8_t cmd[SPI_FLASH_CMD_LEN];
	int ret;

	ret = spi_claim_bus();
	if (ret) {
		ERROR("SF: unable to claim SPI bus\n");
		return ret;
	}

	cmd[0] = CMD_READ_NORMAL;
	while (len) {
		read_addr = offset;
		read_len = MIN(flash->page_size, (len - read_len));
		spi_flash_addr(read_addr, cmd);

		ret = spi_flash_read_common(cmd, sizeof(cmd), data, read_len);
		if (ret < 0) {
			ERROR("SF: read failed\n");
			break;
		}

		offset += read_len;
		len -= read_len;
		data += read_len;
	}
	SPI_DEBUG("SF read done\n");

	spi_release_bus();
	return ret;
}

int spi_flash_write(struct spi_flash *flash, uint32_t offset,
		    uint32_t len, void *buf)
{
	unsigned long byte_addr, page_size;
	uint8_t cmd[SPI_FLASH_CMD_LEN];
	uint32_t chunk_len, actual;
	uint32_t write_addr;
	int ret;

	ret = spi_claim_bus();
	if (ret) {
		ERROR("SF: unable to claim SPI bus\n");
		return ret;
	}

	page_size = flash->page_size;

	cmd[0] = flash->write_cmd;
	for (actual = 0; actual < len; actual += chunk_len) {
		write_addr = offset;
		byte_addr = offset % page_size;
		chunk_len = MIN(len - actual,
				(uint32_t)(page_size - byte_addr));
		spi_flash_addr(write_addr, cmd);

		SPI_DEBUG("SF:0x%p=>cmd:{0x%02x 0x%02x%02x%02x} chunk_len:%d\n",
			  buf + actual, cmd[0], cmd[1],
			  cmd[2], cmd[3], chunk_len);

		ret = spi_flash_write_common(flash, cmd, sizeof(cmd),
					     buf + actual, chunk_len);
		if (ret < 0) {
			ERROR("SF: write cmd failed\n");
			break;
		}

		offset += chunk_len;
	}
	SPI_DEBUG("SF write done\n");

	spi_release_bus();
	return ret;
}

int spi_flash_erase(struct spi_flash *flash, uint32_t offset, uint32_t len)
{
	uint8_t cmd[SPI_FLASH_CMD_LEN];
	uint32_t erase_size, erase_addr;
	int ret;

	erase_size = flash->erase_size;

	if (offset % erase_size || len % erase_size) {
		ERROR("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	ret = spi_claim_bus();
	if (ret) {
		ERROR("SF: unable to claim SPI bus\n");
		return ret;
	}

	cmd[0] = flash->erase_cmd;
	while (len) {
		erase_addr = offset;
		spi_flash_addr(erase_addr, cmd);

		SPI_DEBUG("SF: erase %2x %2x %2x %2x (%x)\n", cmd[0], cmd[1],
			cmd[2], cmd[3], erase_addr);

		ret = spi_flash_write_common(flash, cmd, sizeof(cmd), NULL, 0);
		if (ret < 0) {
			ERROR("SF: erase failed\n");
			break;
		}

		offset += erase_size;
		len -= erase_size;
	}
	SPI_DEBUG("sf erase done\n");

	spi_release_bus();
	return ret;
}

int spi_flash_probe(struct spi_flash *flash)
{
	const struct spi_flash_info *info = NULL;
	int ret;

	ret = spi_claim_bus();
	if (ret) {
		ERROR("SF: Unable to claim SPI bus\n");
		ERROR("SF: probe failed\n");
		return ret;
	}

	info = spi_flash_read_id();
	if (!info)
		goto probe_fail;

	INFO("Flash Name: %s sectors %x, sec size %x\n",
	     info->name, info->n_sectors,
	     info->sector_size);
	flash->size = info->n_sectors * info->sector_size;
	flash->sector_size = info->sector_size;
	flash->page_size = info->page_size;
	flash->flags = info->flags;

	flash->read_cmd = CMD_READ_NORMAL;
	flash->write_cmd = CMD_PAGE_PROGRAM;
	flash->erase_cmd = CMD_ERASE_64K;
	flash->erase_size = ERASE_SIZE_64K;

probe_fail:
	spi_release_bus();
	return ret;
}
