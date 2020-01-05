/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SF_H
#define SF_H

#include <stdint.h>
#include <stddef.h>

#ifdef SPI_DEBUG
#define SPI_DEBUG(fmt, ...)	INFO(fmt, ##__VA_ARGS__)
#else
#define SPI_DEBUG(fmt, ...)
#endif

#define SPI_FLASH_MAX_ID_LEN	6

#define CMD_WRSR		0x01 /* Write status register */
#define CMD_PAGE_PROGRAM	0x02
#define CMD_READ_NORMAL		0x03
#define CMD_RDSR		0x05
#define CMD_WRITE_ENABLE	0x06
#define CMD_RDFSR		0x70
#define CMD_READ_ID		0x9f
#define CMD_ERASE_4K		0x20
#define CMD_ERASE_64K		0xd8
#define ERASE_SIZE_64K		(64 * 1024)

/* Common status */
#define STATUS_WIP		BIT(0)

struct spi_flash {
	struct spi_slave *spi;
	uint32_t size;
	uint32_t page_size;
	uint32_t sector_size;
	uint32_t erase_size;
	uint8_t erase_cmd;
	uint8_t read_cmd;
	uint8_t write_cmd;
	uint8_t flags;
};

struct spi_flash_info {
	const char *name;

	/*
	 * This array stores the ID bytes.
	 * The first three bytes are the JEDIC ID.
	 * JEDEC ID zero means "no ID" (mostly older chips).
	 */
	uint8_t id[SPI_FLASH_MAX_ID_LEN];
	uint8_t id_len;

	uint32_t sector_size;
	uint32_t n_sectors;
	uint16_t page_size;

	uint8_t flags;
};

/* Enum list - Full read commands */
enum spi_read_cmds {
	ARRAY_SLOW              = BIT(0),
	ARRAY_FAST              = BIT(1),
	DUAL_OUTPUT_FAST        = BIT(2),
	DUAL_IO_FAST            = BIT(3),
	QUAD_OUTPUT_FAST        = BIT(4),
	QUAD_IO_FAST            = BIT(5),
};

/* sf param flags */
enum spi_param_flag {
	SECT_4K         = BIT(0),
	SECT_32K        = BIT(1),
	E_FSR           = BIT(2),
	SST_BP          = BIT(3),
	SST_WP          = BIT(4),
	WR_QPP          = BIT(5),
};

int spi_flash_cmd_read(const uint8_t *cmd, size_t cmd_len,
		       void *data, size_t data_len);
int spi_flash_cmd(uint8_t cmd, void *response, size_t len);
int spi_flash_cmd_write(const uint8_t *cmd, size_t cmd_len,
			const void *data, size_t data_len);
#endif
