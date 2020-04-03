/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <sf.h>

int spi_flash_probe(struct spi_flash *flash);
int spi_flash_erase(struct spi_flash *flash, uint32_t offset, uint32_t len);
int spi_flash_write(struct spi_flash *flash, uint32_t offset,
		    uint32_t len, void *buf);
int spi_flash_read(struct spi_flash *flash, uint32_t offset,
		   uint32_t len, void *data);
#endif /* _SPI_FLASH_H_ */
