/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <spi.h>

#define BITS_PER_BYTE		8
#define CMD_LEN1		1

static int spi_flash_read_write(const uint8_t *cmd,
				size_t cmd_len,
				const uint8_t *data_out,
				uint8_t *data_in,
				size_t data_len)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if (data_len == 0)
		flags |= SPI_XFER_END;

	ret = spi_xfer(cmd_len * BITS_PER_BYTE, cmd, NULL, flags);
	if (ret) {
		ERROR("SF: Failed to send command (%zu bytes): %d\n",
		      cmd_len, ret);
	} else if (data_len != 0) {
		ret = spi_xfer(data_len * BITS_PER_BYTE, data_out,
			       data_in, SPI_XFER_END);
		if (ret)
			ERROR("SF: Failed to transfer %zu bytes of data: %d\n",
			      data_len, ret);
	}

	return ret;
}

int spi_flash_cmd_read(const uint8_t *cmd,
		       size_t cmd_len,
		       void *data,
		       size_t data_len)
{
	return spi_flash_read_write(cmd, cmd_len, NULL, data, data_len);
}

int spi_flash_cmd(uint8_t cmd, void *response, size_t len)
{
	return spi_flash_cmd_read(&cmd, CMD_LEN1, response, len);
}

int spi_flash_cmd_write(const uint8_t *cmd,
			size_t cmd_len,
			const void *data,
			size_t data_len)
{
	return spi_flash_read_write(cmd, cmd_len, data, NULL, data_len);
}
